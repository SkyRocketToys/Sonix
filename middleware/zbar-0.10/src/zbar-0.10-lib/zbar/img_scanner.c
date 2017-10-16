/*------------------------------------------------------------------------
 *  Copyright 2007-2009 (c) Jeff Brown <spadix@users.sourceforge.net>
 *
 *  This file is part of the ZBar Bar Code Reader.
 *
 *  The ZBar Bar Code Reader is free software; you can redistribute it
 *  and/or modify it under the terms of the GNU Lesser Public License as
 *  published by the Free Software Foundation; either version 2.1 of
 *  the License, or (at your option) any later version.
 *
 *  The ZBar Bar Code Reader is distributed in the hope that it will be
 *  useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 *  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser Public License
 *  along with the ZBar Bar Code Reader; if not, write to the Free
 *  Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 *  Boston, MA  02110-1301  USA
 *
 *  http://sourceforge.net/projects/zbar
 *------------------------------------------------------------------------*/

#include <config.h>
#include <unistd.h>
#ifdef HAVE_INTTYPES_H
# include <inttypes.h>
#endif
#include <stdlib.h>     /* malloc, free */
#include <time.h>       /* clock_gettime */
#include <sys/time.h>   /* gettimeofday */
#include <string.h>     /* memcmp, memset, memcpy */
#include <assert.h>

#include <zbar.h>
#include "error.h"
#include "image.h"
#ifdef ENABLE_QRCODE
# include "qrcode.h"
#endif
#include "img_scanner.h"
#include "svg.h"

#if 1
# define ASSERT_POS \
    assert(p == data + x + y * (intptr_t)w)
#else
# define ASSERT_POS
#endif

/* FIXME cache setting configurability */

/* number of times the same result must be detected
 * in "nearby" images before being reported
 */
#define CACHE_CONSISTENCY    3 /* images */

/* time interval for which two images are considered "nearby"
 */
#define CACHE_PROXIMITY   1000 /* ms */

/* time that a result must *not* be detected before
 * it will be reported again
 */
#define CACHE_HYSTERESIS  2000 /* ms */

/* time after which cache entries are invalidated
 */
#define CACHE_TIMEOUT     (CACHE_HYSTERESIS * 2) /* ms */

#define NUM_SCN_CFGS (ZBAR_CFG_Y_DENSITY - ZBAR_CFG_X_DENSITY + 1)

#define CFG(iscn, cfg) ((iscn)->configs[(cfg) - ZBAR_CFG_X_DENSITY])
#define TEST_CFG(iscn, cfg) (((iscn)->config >> ((cfg) - ZBAR_CFG_POSITION)) & 1)

#ifndef NO_STATS
# define STAT(x) iscn->stat_##x++
#else
# define STAT(...)
# define dump_stats(...)
#endif

#define RECYCLE_BUCKETS     5

typedef struct recycle_bucket_s {
    int nsyms;
    zbar_symbol_t *head;
} recycle_bucket_t;

/* image scanner state */
struct zbar_image_scanner_s {
    zbar_scanner_t *scn;        /* associated linear intensity scanner */
    zbar_decoder_t *dcode;      /* associated symbol decoder */
#ifdef ENABLE_QRCODE
    qr_reader *qr;              /* QR Code 2D reader */
#endif

    const void *userdata;       /* application data */
    /* user result callback */
    zbar_image_data_handler_t *handler;

    unsigned long time;         /* scan start time */
    zbar_image_t *img;          /* currently scanning image *root* */
    int dx, dy, du, umin, v;    /* current scan direction */
    zbar_symbol_set_t *syms;    /* previous decode results */
    /* recycled symbols in 4^n size buckets */
    recycle_bucket_t recycle[RECYCLE_BUCKETS];

    int enable_cache;           /* current result cache state */
    zbar_symbol_t *cache;       /* inter-image result cache entries */

    /* configuration settings */
    unsigned config;            /* config flags */
    int configs[NUM_SCN_CFGS];  /* int valued configurations */

#ifndef NO_STATS
    int stat_syms_new;
    int stat_iscn_syms_inuse, stat_iscn_syms_recycle;
    int stat_img_syms_inuse, stat_img_syms_recycle;
    int stat_sym_new;
    int stat_sym_recycle[RECYCLE_BUCKETS];
#endif
};

void _zbar_image_scanner_recycle_syms (zbar_image_scanner_t *iscn,
                                       zbar_symbol_t *sym)
{
    zbar_symbol_t *next = NULL;
    for(; sym; sym = next) {
        next = sym->next;
        if(sym->refcnt && _zbar_refcnt(&sym->refcnt, -1)) {
            /* unlink referenced symbol */
            /* FIXME handle outstanding component refs (currently unsupported)
             */
            assert(sym->data_alloc);
            sym->next = NULL;
        }
        else {
            /* recycle unreferenced symbol */
            if(!sym->data_alloc) {
                sym->data = NULL;
                sym->datalen = 0;
            }
            if(sym->syms) {
                if(_zbar_refcnt(&sym->syms->refcnt, -1))
                    assert(0);
                _zbar_image_scanner_recycle_syms(iscn, sym->syms->head);
                sym->syms->head = NULL;
                _zbar_symbol_set_free(sym->syms);
                sym->syms = NULL;
            }
            int i;
            for(i = 0; i < RECYCLE_BUCKETS; i++)
                if(sym->data_alloc < 1 << (i * 2))
                    break;
            if(i == RECYCLE_BUCKETS) {
                assert(sym->data);
                free(sym->data);
                sym->data = NULL;
                sym->data_alloc = 0;
                i = 0;
            }
            recycle_bucket_t *bucket = &iscn->recycle[i];
            /* FIXME cap bucket fill */
            bucket->nsyms++;
            sym->next = bucket->head;
            bucket->head = sym;
        }
    }
}

static inline int recycle_syms (zbar_image_scanner_t *iscn,
                                zbar_symbol_set_t *syms)
{
    if(_zbar_refcnt(&syms->refcnt, -1))
        return(1);

    _zbar_image_scanner_recycle_syms(iscn, syms->head);
    syms->head = syms->tail = NULL;
    syms->nsyms = 0;
    return(0);
}

inline void zbar_image_scanner_recycle_image (zbar_image_scanner_t *iscn,
                                              zbar_image_t *img)
{
    zbar_symbol_set_t *syms = iscn->syms;
    if(syms && syms->refcnt) {
        if(recycle_syms(iscn, syms)) {
            STAT(iscn_syms_inuse);
            iscn->syms = NULL;
        }
        else
            STAT(iscn_syms_recycle);
    }

    syms = img->syms;
    img->syms = NULL;
    if(syms && recycle_syms(iscn, syms)) {
        STAT(img_syms_inuse);
        syms = iscn->syms;
    }
    else if(syms) {
        STAT(img_syms_recycle);

        /* select one set to resurrect, destroy the other */
        if(iscn->syms) {
            _zbar_symbol_set_free(syms);
            syms = iscn->syms;
        }
        else
            iscn->syms = syms;
    }
}

inline zbar_symbol_t*
_zbar_image_scanner_alloc_sym (zbar_image_scanner_t *iscn,
                               zbar_symbol_type_t type,
                               int datalen)
{
    /* recycle old or alloc new symbol */
    int i;
    for(i = 0; i < RECYCLE_BUCKETS - 1; i++)
        if(datalen <= 1 << (i * 2))
            break;

    zbar_symbol_t *sym = NULL;
    for(; i > 0; i--)
        if((sym = iscn->recycle[i].head)) {
            STAT(sym_recycle[i]);
            break;
        }

    if(sym) {
        iscn->recycle[i].head = sym->next;
        sym->next = NULL;
        assert(iscn->recycle[i].nsyms);
        iscn->recycle[i].nsyms--;
    }
    else {
        sym = calloc(1, sizeof(zbar_symbol_t));
        STAT(sym_new);
    }

    /* init new symbol */
    sym->type = type;
    sym->quality = 1;
    sym->npts = 0;
    sym->cache_count = 0;
    sym->time = iscn->time;
    assert(!sym->syms);

    if(datalen > 0) {
        sym->datalen = datalen - 1;
        if(sym->data_alloc < datalen) {
            if(sym->data)
                free(sym->data);
            sym->data_alloc = datalen;
            sym->data = malloc(datalen);
        }
    }
    else {
        if(sym->data)
            free(sym->data);
        sym->data = NULL;
        sym->datalen = sym->data_alloc = 0;
    }
    return(sym);
}

static inline zbar_symbol_t *cache_lookup (zbar_image_scanner_t *iscn,
                                           zbar_symbol_t *sym)
{
    /* search for matching entry in cache */
    zbar_symbol_t **entry = &iscn->cache;
    while(*entry) {
        if((*entry)->type == sym->type &&
           (*entry)->datalen == sym->datalen &&
           !memcmp((*entry)->data, sym->data, sym->datalen))
            break;
        if((sym->time - (*entry)->time) > CACHE_TIMEOUT) {
            /* recycle stale cache entry */
            zbar_symbol_t *next = (*entry)->next;
            (*entry)->next = NULL;
            _zbar_image_scanner_recycle_syms(iscn, *entry);
            *entry = next;
        }
        else
            entry = &(*entry)->next;
    }
    return(*entry);
}

static inline void cache_sym (zbar_image_scanner_t *iscn,
                              zbar_symbol_t *sym)
{
    if(iscn->enable_cache) {
        zbar_symbol_t *entry = cache_lookup(iscn, sym);
        if(!entry) {
            /* FIXME reuse sym */
            entry = _zbar_image_scanner_alloc_sym(iscn, sym->type,
                                                  sym->datalen + 1);
            memcpy(entry->data, sym->data, sym->datalen);
            entry->time = sym->time - CACHE_HYSTERESIS;
            entry->cache_count = -CACHE_CONSISTENCY;
            /* add to cache */
            entry->next = iscn->cache;
            iscn->cache = entry;
        }

        /* consistency check and hysteresis */
        uint32_t age = sym->time - entry->time;
        entry->time = sym->time;
        int near_thresh = (age < CACHE_PROXIMITY);
        int far_thresh = (age >= CACHE_HYSTERESIS);
        int dup = (entry->cache_count >= 0);
        if((!dup && !near_thresh) || far_thresh)
            entry->cache_count = -CACHE_CONSISTENCY;
        else if(dup || near_thresh)
            entry->cache_count++;

        sym->cache_count = entry->cache_count;
    }
    else
        sym->cache_count = 0;
}

void _zbar_image_scanner_add_sym(zbar_image_scanner_t *iscn,
                                 zbar_symbol_t *sym)
{
    cache_sym(iscn, sym);

    zbar_symbol_set_t *syms = iscn->syms;
    if(sym->cache_count || !syms->tail) {
        sym->next = syms->head;
        syms->head = sym;
    }
    else {
        sym->next = syms->tail->next;
        syms->tail->next = sym;
    }

    if(!sym->cache_count)
        syms->nsyms++;
    else if(!syms->tail)
        syms->tail = sym;

    _zbar_symbol_refcnt(sym, 1);
}

#ifdef ENABLE_QRCODE
extern qr_finder_line *_zbar_decoder_get_qr_finder_line(zbar_decoder_t*);

# define QR_FIXED(v, rnd) ((((v) << 1) + (rnd)) << (QR_FINDER_SUBPREC - 1))
# define PRINT_FIXED(val, prec) \
    ((val) >> (prec)),         \
        (1000 * ((val) & ((1 << (prec)) - 1)) / (1 << (prec)))

static inline void qr_handler (zbar_image_scanner_t *iscn)
{
    qr_finder_line *line = _zbar_decoder_get_qr_finder_line(iscn->dcode);
    assert(line);
    unsigned u = zbar_scanner_get_edge(iscn->scn, line->pos[0],
                                       QR_FINDER_SUBPREC);
    line->boffs = u - zbar_scanner_get_edge(iscn->scn, line->boffs,
                                            QR_FINDER_SUBPREC);
    line->len = zbar_scanner_get_edge(iscn->scn, line->len,
                                      QR_FINDER_SUBPREC);
    line->eoffs = zbar_scanner_get_edge(iscn->scn, line->eoffs,
                                        QR_FINDER_SUBPREC) - line->len;
    line->len -= u;

    u = QR_FIXED(iscn->umin, 0) + iscn->du * u;
    if(iscn->du < 0) {
        u -= line->len;
        int tmp = line->boffs;
        line->boffs = line->eoffs;
        line->eoffs = tmp;
    }
    int vert = !iscn->dx;
    line->pos[vert] = u;
    line->pos[!vert] = QR_FIXED(iscn->v, 1);

    _zbar_qr_found_line(iscn->qr, vert, line);
}
#endif

static void symbol_handler (zbar_decoder_t *dcode)
{
    zbar_image_scanner_t *iscn = zbar_decoder_get_userdata(dcode);
    zbar_symbol_type_t type = zbar_decoder_get_type(dcode);
    /* FIXME assert(type == ZBAR_PARTIAL) */
    /* FIXME debug flag to save/display all PARTIALs */
    if(type <= ZBAR_PARTIAL)
        return;

#ifdef ENABLE_QRCODE
    if(type == ZBAR_QRCODE) {
        qr_handler(iscn);
        return;
    }
#else
    assert(type != ZBAR_QRCODE);
#endif

    const char *data = zbar_decoder_get_data(dcode);
    unsigned datalen = zbar_decoder_get_data_length(dcode);

    int x = 0, y = 0;
    if(TEST_CFG(iscn, ZBAR_CFG_POSITION)) {
        /* tmp position fixup */
        int w = zbar_scanner_get_width(iscn->scn);
        int u = iscn->umin + iscn->du * zbar_scanner_get_edge(iscn->scn, w, 0);
        if(iscn->dx) {
            x = u;
            y = iscn->v;
        }
        else {
            x = iscn->v;
            y = u;
        }
    }

    /* FIXME need better symbol matching */
    zbar_symbol_t *sym;
    for(sym = iscn->syms->head; sym; sym = sym->next)
        if(sym->type == type &&
           sym->datalen == datalen &&
           !memcmp(sym->data, data, datalen)) {
            sym->quality++;
            if(TEST_CFG(iscn, ZBAR_CFG_POSITION))
                /* add new point to existing set */
                /* FIXME should be polygon */
                sym_add_point(sym, x, y);
            return;
        }

    sym = _zbar_image_scanner_alloc_sym(iscn, type, datalen + 1);
    /* FIXME grab decoder buffer */
    memcpy(sym->data, data, datalen + 1);

    /* initialize first point */
    if(TEST_CFG(iscn, ZBAR_CFG_POSITION))
        sym_add_point(sym, x, y);

    _zbar_image_scanner_add_sym(iscn, sym);
}

zbar_image_scanner_t *zbar_image_scanner_create ()
{
    zbar_image_scanner_t *iscn = calloc(1, sizeof(zbar_image_scanner_t));
    if(!iscn)
        return(NULL);
    iscn->dcode = zbar_decoder_create();
    iscn->scn = zbar_scanner_create(iscn->dcode);
    if(!iscn->dcode || !iscn->scn) {
        zbar_image_scanner_destroy(iscn);
        return(NULL);
    }
    zbar_decoder_set_userdata(iscn->dcode, iscn);
    zbar_decoder_set_handler(iscn->dcode, symbol_handler);

#ifdef ENABLE_QRCODE
    iscn->qr = _zbar_qr_create();
#endif

    /* apply default configuration */
    CFG(iscn, ZBAR_CFG_X_DENSITY) = 1;
    CFG(iscn, ZBAR_CFG_Y_DENSITY) = 1;
    zbar_image_scanner_set_config(iscn, 0, ZBAR_CFG_POSITION, 1);
    return(iscn);
}

#ifndef NO_STATS
static inline void dump_stats (const zbar_image_scanner_t *iscn)
{
    zprintf(1, "symbol sets allocated   = %-4d\n", iscn->stat_syms_new);
    zprintf(1, "    scanner syms in use = %-4d\trecycled  = %-4d\n",
            iscn->stat_iscn_syms_inuse, iscn->stat_iscn_syms_recycle);
    zprintf(1, "    image syms in use   = %-4d\trecycled  = %-4d\n",
            iscn->stat_img_syms_inuse, iscn->stat_img_syms_recycle);
    zprintf(1, "symbols allocated       = %-4d\n", iscn->stat_sym_new);
    int i;
    for(i = 0; i < RECYCLE_BUCKETS; i++)
        zprintf(1, "     recycled[%d]        = %-4d\n",
                i, iscn->stat_sym_recycle[i]);
}
#endif

void zbar_image_scanner_destroy (zbar_image_scanner_t *iscn)
{
    dump_stats(iscn);
    if(iscn->syms) {
        if(iscn->syms->refcnt)
            zbar_symbol_set_ref(iscn->syms, -1);
        else
            _zbar_symbol_set_free(iscn->syms);
        iscn->syms = NULL;
    }
    if(iscn->scn)
        zbar_scanner_destroy(iscn->scn);
    iscn->scn = NULL;
    if(iscn->dcode)
        zbar_decoder_destroy(iscn->dcode);
    iscn->dcode = NULL;
    int i;
    for(i = 0; i < RECYCLE_BUCKETS; i++) {
        zbar_symbol_t *sym, *next;
        for(sym = iscn->recycle[i].head; sym; sym = next) {
            next = sym->next;
            _zbar_symbol_free(sym);
        }
    }
#ifdef ENABLE_QRCODE
    if(iscn->qr) {
        _zbar_qr_destroy(iscn->qr);
        iscn->qr = NULL;
    }
#endif
    free(iscn);
}

zbar_image_data_handler_t*
zbar_image_scanner_set_data_handler (zbar_image_scanner_t *iscn,
                                     zbar_image_data_handler_t *handler,
                                     const void *userdata)
{
    zbar_image_data_handler_t *result = iscn->handler;
    iscn->handler = handler;
    iscn->userdata = userdata;
    return(result);
}

int zbar_image_scanner_set_config (zbar_image_scanner_t *iscn,
                                   zbar_symbol_type_t sym,
                                   zbar_config_t cfg,
                                   int val)
{
    if(cfg < ZBAR_CFG_POSITION)
        return(zbar_decoder_set_config(iscn->dcode, sym, cfg, val));

    if(sym > ZBAR_PARTIAL)
        return(1);

    if(cfg >= ZBAR_CFG_X_DENSITY && cfg <= ZBAR_CFG_Y_DENSITY) {

        CFG(iscn, cfg) = val;
        return(0);
    }

    if(cfg > ZBAR_CFG_POSITION)
        return(1);
    cfg -= ZBAR_CFG_POSITION;

    if(!val)
        iscn->config &= ~(1 << cfg);
    else if(val == 1)
        iscn->config |= (1 << cfg);
    else
        return(1);

    return(0);
}

void zbar_image_scanner_enable_cache (zbar_image_scanner_t *iscn,
                                      int enable)
{
    if(iscn->cache) {
        /* recycle all cached syms */
        _zbar_image_scanner_recycle_syms(iscn, iscn->cache);
        iscn->cache = NULL;
    }
    iscn->enable_cache = (enable) ? 1 : 0;
}

const zbar_symbol_set_t *
zbar_image_scanner_get_results (const zbar_image_scanner_t *iscn)
{
    return(iscn->syms);
}

static inline void quiet_border (zbar_image_scanner_t *iscn)
{
    /* flush scanner pipeline */
    zbar_scanner_t *scn = iscn->scn;
    zbar_scanner_flush(scn);
    zbar_scanner_flush(scn);
    zbar_scanner_new_scan(scn);
}

#define movedelta(dx, dy) do {                  \
        x += (dx);                              \
        y += (dy);                              \
        p += (dx) + ((intptr_t)(dy) * w);       \
    } while(0);

int zbar_scan_image (zbar_image_scanner_t *iscn,
                     zbar_image_t *img)
{
    /* timestamp image
     * FIXME prefer video timestamp
     */
#if _POSIX_TIMERS > 0
    struct timespec abstime;
    clock_gettime(CLOCK_REALTIME, &abstime);
    iscn->time = (abstime.tv_sec * 1000) + ((abstime.tv_nsec / 500000) + 1) / 2;
#else
    struct timeval abstime;
    gettimeofday(&abstime, NULL);
    iscn->time = (abstime.tv_sec * 1000) + ((abstime.tv_usec / 500) + 1) / 2;
#endif

#ifdef ENABLE_QRCODE
    _zbar_qr_reset(iscn->qr);
#endif

    /* get grayscale image, convert if necessary */
    if(img->format != fourcc('Y','8','0','0') &&
       img->format != fourcc('G','R','E','Y'))
        return(-1);
    iscn->img = img;

    /* recycle previous scanner and image results */
    zbar_image_scanner_recycle_image(iscn, img);
    zbar_symbol_set_t *syms = iscn->syms;
    if(!syms) {
        syms = iscn->syms = _zbar_symbol_set_create();
        STAT(syms_new);
        zbar_symbol_set_ref(syms, 1);
    }
    else
        zbar_symbol_set_ref(syms, 2);
    img->syms = syms;

    unsigned w = img->width;
    unsigned h = img->height;
    const uint8_t *data = img->data;

    zbar_image_write_png(img, "debug.png");
    svg_open("debug.svg", 0, 0, w, h);
    svg_image("debug.png", w, h);

    zbar_scanner_t *scn = iscn->scn;

    int density = CFG(iscn, ZBAR_CFG_Y_DENSITY);
    if(density > 0) {
        svg_group_start("scanner", 0, 1, 1, 0, 0);
        const uint8_t *p = data;
        int x = 0, y = 0;
        iscn->dy = 0;

        int border = (((h - 1) % density) + 1) / 2;
        if(border > h / 2)
            border = h / 2;
        movedelta(0, border);
        iscn->v = y;

        zbar_scanner_new_scan(scn);

        while(y < h) {
            zprintf(128, "img_x+: %04d,%04d @%p\n", x, y, p);
            svg_path_start("vedge", 1. / 32, 0, y + 0.5);
            iscn->dx = iscn->du = 1;
            iscn->umin = 0;
            while(x < w) {
                uint8_t d = *p;
                movedelta(1, 0);
                zbar_scan_y(scn, d);
            }
            ASSERT_POS;
            quiet_border(iscn);
            svg_path_end();

            movedelta(-1, density);
            iscn->v = y;
            if(y >= h)
                break;

            zprintf(128, "img_x-: %04d,%04d @%p\n", x, y, p);
            svg_path_start("vedge", -1. / 32, w, y + 0.5);
            iscn->dx = iscn->du = -1;
            iscn->umin = w;
            while(x >= 0) {
                uint8_t d = *p;
                movedelta(-1, 0);
                zbar_scan_y(scn, d);
            }
            ASSERT_POS;
            quiet_border(iscn);
            svg_path_end();

            movedelta(1, density);
            iscn->v = y;
        }
        svg_group_end();
    }
    iscn->dx = 0;

    density = CFG(iscn, ZBAR_CFG_X_DENSITY);
    if(density > 0) {
        svg_group_start("scanner", 90, 1, -1, 0, 0);
        const uint8_t *p = data;
        int x = 0, y = 0;

        int border = (((w - 1) % density) + 1) / 2;
        if(border > w / 2)
            border = w / 2;
        movedelta(border, 0);
        iscn->v = x;

        while(x < w) {
            zprintf(128, "img_y+: %04d,%04d @%p\n", x, y, p);
            svg_path_start("vedge", 1. / 32, 0, x + 0.5);
            iscn->dy = iscn->du = 1;
            iscn->umin = 0;
            while(y < h) {
                uint8_t d = *p;
                movedelta(0, 1);
                zbar_scan_y(scn, d);
            }
            ASSERT_POS;
            quiet_border(iscn);
            svg_path_end();

            movedelta(density, -1);
            iscn->v = x;
            if(x >= w)
                break;

            zprintf(128, "img_y-: %04d,%04d @%p\n", x, y, p);
            svg_path_start("vedge", -1. / 32, h, x + 0.5);
            iscn->dy = iscn->du = -1;
            iscn->umin = h;
            while(y >= 0) {
                uint8_t d = *p;
                movedelta(0, -1);
                zbar_scan_y(scn, d);
            }
            ASSERT_POS;
            quiet_border(iscn);
            svg_path_end();

            movedelta(density, 1);
            iscn->v = x;
        }
        svg_group_end();
    }
    iscn->dy = 0;
    iscn->img = NULL;

#ifdef ENABLE_QRCODE
    _zbar_qr_decode(iscn->qr, iscn, img);
#endif

    /* FIXME tmp hack to filter bad EAN results */
    if(syms->nsyms && !iscn->enable_cache &&
       (density == 1 || CFG(iscn, ZBAR_CFG_Y_DENSITY) == 1)) {
        zbar_symbol_t **symp = &syms->head, *sym;
        while((sym = *symp)) {
            if(sym->type < ZBAR_I25 && sym->type > ZBAR_PARTIAL &&
               sym->quality < 3) {
                /* recycle */
                *symp = sym->next;
                syms->nsyms--;
                sym->next = NULL;
                _zbar_image_scanner_recycle_syms(iscn, sym);
            }
            else
                symp = &sym->next;
        }
    }

    if(syms->nsyms && iscn->handler)
        iscn->handler(img, iscn->userdata);

    svg_close();
    return(syms->nsyms);
}

#ifdef DEBUG_SVG
/* FIXME lame...*/
# include "svg.c"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <FreeRTOS.h>
#include <portable.h>

#if 0
void* malloc(size_t size) {
	return pvPortMalloc(size, GFP_KERNEL, MODULE_MID_ZBAR);
}
#endif

#if 0
void* calloc(size_t nmemb, size_t size) {
	uint64_t len = nmemb * size;
	void *ptr = NULL;

	if (len == 0) return NULL;

	ptr = pvPortMalloc(len, GFP_KERNEL, MODULE_MID_ZBAR);
	if (!ptr) return NULL;

	memset(ptr, 0, len);
	return ptr;
}
#endif

#if 0
void* realloc(void *ptr, size_t size) {
	void *memb = NULL;

	if (ptr)
	{
		vPortFree(ptr);
	}
	else
	//if (ptr==NULL)
	{
		//return NULL;
	}

	memb = pvPortMalloc(size, GFP_KERNEL, MODULE_MID_ZBAR);

	if (!memb) return NULL;

	return memb;
}
#endif


#if 0
void free(void *ptr) {
	if (ptr) vPortFree(ptr);
}
#endif

#if 0
int printf(const char *format, ...) {
	return -ENOSYS;
}

FILE *stderr;

//FILE* fopen(const char *path, const char *mode) {
//}

int fprintf(FILE *stream, const char *format, ...) {
	return -ENOSYS;
}
#endif

#if 0
int sprintf(char *str, const char *format, ...) {
	return -ENOSYS;
}
int snprintf(char *str, size_t size, const char *format, ...) {
	return -ENOSYS;
}
#endif

#if 0
void __assert(const char *a, const char *b, unsigned int c, const char *d) {
	return;
}
#endif

#if 0
int clock_gettime (clockid_t __clock_id, struct timespec *__tp) {
	return 0;
}
#endif

#if 0
/* Copyright (C) 2011 by Valentin Ochs
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/* Minor changes by Rich Felker for integration in musl, 2011-04-27. */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

//#include "atomic.h"
#define ntz(x) a_ctz_l((x))

static inline int a_ctz_l(unsigned long x)
{
        static const char debruijn32[32] = {
                0, 1, 23, 2, 29, 24, 19, 3, 30, 27, 25, 11, 20, 8, 4, 13,
                31, 22, 28, 18, 26, 10, 7, 12, 21, 17, 9, 6, 16, 5, 15, 14
        };
        return debruijn32[(x&-x)*0x076be629 >> 27];
}

typedef int (*cmpfun)(const void *, const void *);

static inline int pntz(size_t p[2]) {
	int r = ntz(p[0] - 1);
	if(r != 0 || (r = 8*sizeof(size_t) + ntz(p[1])) != 8*sizeof(size_t)) {
		return r;
	}
	return 0;
}

static void cycle(size_t width, unsigned char* ar[], int n)
{
	unsigned char tmp[256];
	size_t l;
	int i;

	if(n < 2) {
		return;
	}

	ar[n] = tmp;
	while(width) {
		l = sizeof(tmp) < width ? sizeof(tmp) : width;
		memcpy(ar[n], ar[0], l);
		for(i = 0; i < n; i++) {
			memcpy(ar[i], ar[i + 1], l);
			ar[i] += l;
		}
		width -= l;
	}
}

/* shl() and shr() need n > 0 */
static inline void shl(size_t p[2], int n)
{
	if(n >= 8 * sizeof(size_t)) {
		n -= 8 * sizeof(size_t);
		p[1] = p[0];
		p[0] = 0;
	}
	p[1] <<= n;
	p[1] |= p[0] >> (sizeof(size_t) * 8 - n);
	p[0] <<= n;
}

static inline void shr(size_t p[2], int n)
{
	if(n >= 8 * sizeof(size_t)) {
		n -= 8 * sizeof(size_t);
		p[0] = p[1];
		p[1] = 0;
	}
	p[0] >>= n;
	p[0] |= p[1] << (sizeof(size_t) * 8 - n);
	p[1] >>= n;
}

static void sift(unsigned char *head, size_t width, cmpfun cmp, int pshift, size_t lp[])
{
	unsigned char *rt, *lf;
	unsigned char *ar[14 * sizeof(size_t) + 1];
	int i = 1;

	ar[0] = head;
	while(pshift > 1) {
		rt = head - width;
		lf = head - width - lp[pshift - 2];

		if((*cmp)(ar[0], lf) >= 0 && (*cmp)(ar[0], rt) >= 0) {
			break;
		}
		if((*cmp)(lf, rt) >= 0) {
			ar[i++] = lf;
			head = lf;
			pshift -= 1;
		} else {
			ar[i++] = rt;
			head = rt;
			pshift -= 2;
		}
	}
	cycle(width, ar, i);
}

static void trinkle(unsigned char *head, size_t width, cmpfun cmp, size_t pp[2], int pshift, int trusty, size_t lp[])
{
	unsigned char *stepson,
	              *rt, *lf;
	size_t p[2];
	unsigned char *ar[14 * sizeof(size_t) + 1];
	int i = 1;
	int trail;

	p[0] = pp[0];
	p[1] = pp[1];

	ar[0] = head;
	while(p[0] != 1 || p[1] != 0) {
		stepson = head - lp[pshift];
		if((*cmp)(stepson, ar[0]) <= 0) {
			break;
		}
		if(!trusty && pshift > 1) {
			rt = head - width;
			lf = head - width - lp[pshift - 2];
			if((*cmp)(rt, stepson) >= 0 || (*cmp)(lf, stepson) >= 0) {
				break;
			}
		}

		ar[i++] = stepson;
		head = stepson;
		trail = pntz(p);
		shr(p, trail);
		pshift += trail;
		trusty = 0;
	}
	if(!trusty) {
		cycle(width, ar, i);
		sift(head, width, cmp, pshift, lp);
	}
}

void qsort(void *base, size_t nel, size_t width, cmpfun cmp)
{
	size_t lp[12*sizeof(size_t)];
	size_t i, size = width * nel;
	unsigned char *head, *high;
	size_t p[2] = {1, 0};
	int pshift = 1;
	int trail;

	if (!size) return;

	head = base;
	high = head + size - width;

	/* Precompute Leonardo numbers, scaled by element width */
	for(lp[0]=lp[1]=width, i=2; (lp[i]=lp[i-2]+lp[i-1]+width) < size; i++);

	while(head < high) {
		if((p[0] & 3) == 3) {
			sift(head, width, cmp, pshift, lp);
			shr(p, 2);
			pshift += 2;
		} else {
			if(lp[pshift - 1] >= high - head) {
				trinkle(head, width, cmp, p, pshift, 0, lp);
			} else {
				sift(head, width, cmp, pshift, lp);
			}
			
			if(pshift == 1) {
				shl(p, 1);
				pshift = 0;
			} else {
				shl(p, pshift - 1);
				pshift = 1;
			}
		}
		
		p[0] |= 1;
		head += width;
	}

	trinkle(head, width, cmp, p, pshift, 0, lp);

	while(pshift != 1 || p[0] != 1 || p[1] != 0) {
		if(pshift <= 1) {
			trail = pntz(p);
			shr(p, trail);
			pshift += trail;
		} else {
			shl(p, 2);
			pshift -= 2;
			p[0] ^= 7;
			shr(p, 1);
			trinkle(head - lp[pshift] - width, width, cmp, p, pshift + 1, 1, lp);
			shl(p, 1);
			p[0] |= 1;
			trinkle(head - width, width, cmp, p, pshift, 1, lp);
		}
		head -= width;
	}
}
#endif


#if 0
static void swap(void *x, void *y, size_t l) {
	char *a = x, *b = y, c;
	while(l--) {
		c = *a;
		*a++ = *b;
		*b++ = c;
	}
}

static void sort(char *array, size_t size, int (*cmp)(void*,void*), int begin, int end) {
	if (end > begin) {
		void *pivot = array + begin;
		int l = begin + size;
		int r = end;
		while(l < r) {
			if (cmp(array+l,pivot) <= 0) {
				l += size;
			} else if ( cmp(array+r, pivot) > 0 )  {
				r -= size;
			} else if ( l < r ) {
				swap(array+l, array+r, size);
			}
		}
		l -= size;
		swap(array+begin, array+l, size);
		sort(array, size, cmp, begin, l);
		sort(array, size, cmp, r, end);
	}
}

void qsort(void *__base, size_t __nmemb, size_t __size, __compar_fn_t __compar) {
	sort(__base, __size, __compar, 0, __nmemb * __size);
}
#endif

#if 0
void *memchr(const void *s, int c, size_t n)
{
	unsigned char *p = (unsigned char*)s;
	while( n-- )
		if( *p != (unsigned char)c )
			p++;
		else
			return p;
	return 0;
}
#endif

#if 0
void *memmove(void *dest, const void *src, size_t n)
{
	unsigned char tmp[n];

	print_msg("%s:%d:n=%d=%x\n", __func__, __LINE__,n,n);

	
	memcpy(tmp,src,n);
	memcpy(dest,tmp,n);
	return dest;
}
#endif

#if 0
char* strstr(const char *str, const char *target) {
	if (!*target) return str;
	char *p1 = (char*)str, *p2 = (char*)target;
	char *p1Adv = (char*)str;
	while (*++p2)
		p1Adv++;
	while (*p1Adv) {
		char *p1Begin = p1;
		p2 = (char*)target;
		while (*p1 && *p2 && *p1 == *p2) {
			p1++;
			p2++;
		}
		if (!*p2)
			return p1Begin;
		p1 = p1Begin + 1;
		p1Adv++;
	}
	return NULL;
}
#endif

char* strerror(int __errnum) {
	return __errnum ? "error occur" : "no error";
}

#if 0
char* strdup(const char *s) {
	size_t str_sz = strlen(s);
	char *str = malloc(str_sz+1);
	memcpy(str, s, str_sz);
	str[str_sz] = '0';
	return str;
}
#endif

#if 0
int fputs(const char *s, FILE *stream) {
	print_msg(s);
	return strlen(s);
}
#endif