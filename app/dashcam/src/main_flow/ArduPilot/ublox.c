#include "ublox.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <libmid_fatfs/ff.h>
#include <utility.h> // for get_sys_seconds
#include <semphr.h>
#include <queue.h>

#include "dev_console.h"
#include "mavlink_wifi.h"
#include <mavlink_core.h>
#include "talloc.h"
#include <web_server.h>
#include "util/print_vprintf.h"
#include <libmid_nvram/snx_mid_nvram.h>

#define LEAPSECS_SINCE_1980 0x80

#define UBLOX_MIN_DATE 1494314686

// semaphore protecting both access to the file on disk
// and the mga_offline_data variables storing it in memory
static xSemaphoreHandle mga_offline_sem;

static uint8_t *mga_offline_data;
static uint32_t mga_offline_data_len;
static uint32_t mga_upload_utc;
static uint32_t mga_highest_cached_utc;

// time since boot when we last supplied offline assistance data
static long long offline_assistance_data_supplied_sec;

// utc time of the data we uploaded
static uint32_t offline_assistance_data_supplied_utc;

static const char *ublox_mga_offline_filepath = UBLOX_BASEDIR "/OFFLINE.UBX";
static const char *ublox_gps_pos_filepath = UBLOX_BASEDIR "/GPSPOS.TXT";

extern QueueHandle_t mavlink_msg_to_fc_queue;

static struct mga_position mga_pos_estimate;

// record time to first fix
static uint32_t fix_wait_start;
static uint32_t fix_time;

// where did mga_pos_estimate come from?
static enum {
    POS_SOURCE_NONE=0,
    POS_SOURCE_MICROSD,
    POS_SOURCE_APP,
    POS_SOURCE_GPS
} pos_source, last_pos_source;

static enum {
    TIME_SOURCE_NONE=0,
    TIME_SOURCE_MICROSD,
    TIME_SOURCE_APP,
    TIME_SOURCE_FC
} time_source, last_time_source;

#define MAX_DATA_SIZE 200000 // typical is 130k

static void initialise_mga_offline_mutex();
static int mga_offline_read_file(uint8_t **ret, uint32_t *retlen);
static int mga_offline_lock();
static void mga_offline_unlock();

/*
 * initialise ublox handling, including reading of data from SD card
 */
void ublox_init()
{
    ublox_debug("ublox_init\n");

    initialise_mga_offline_mutex();
    if (mga_offline_lock() == 0) {
        mga_offline_read_file(&mga_offline_data, &mga_offline_data_len); // may fail
        mga_offline_unlock();
    }
}

/*
 * create mutex protecting mga-offline data held in memory
 */
static void initialise_mga_offline_mutex()
{
    if (mga_offline_sem == NULL) {
	mga_offline_sem = xSemaphoreCreateBinary();
	if (mga_offline_sem == NULL) {
	    ublox_debug("Failed to allocate semaphore\n");
	    return;
	}
	xSemaphoreGive(mga_offline_sem);
	ublox_debug("initialised mutex: %x\n", mga_offline_sem);
    }
}

/*
 * ublox message definitions
 */
const uint8_t PREAMBLE1 = 0xb5;
const uint8_t PREAMBLE2 = 0x62;

#define PACKED __attribute__((__packed__))
struct PACKED ubx_header {
    uint8_t preamble1;
    uint8_t preamble2;
    uint8_t class_id;
    uint8_t msg_id;
    uint16_t length;
};
typedef struct PACKED {
    uint8_t type;
    uint8_t version;
    uint8_t svID;
    uint8_t gnssId;
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t reserved1;
    uint8_t data[64];
    uint8_t reserved2[4];
} ubx_mga_ano_t;
typedef struct PACKED {
    uint8_t type;
    uint8_t version;
    uint8_t ref;
    int8_t leapSecs;
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t reserved1;
    uint32_t ns;
    uint16_t tAccS;
    uint8_t reserved2[2];
    uint32_t tAccNs;
} ubx_ini_time_utc_t;

typedef struct PACKED {
    uint16_t version;
    uint16_t mask1;
    uint32_t mask2;
    uint8_t  reserved1[2];
    uint8_t minSVs;
    uint8_t maxSVs;
    uint8_t minCNO;
    uint8_t reserved2;
    uint8_t iniFix3D;
    uint8_t reserved3[2];
    uint8_t ackAiding;
    uint16_t wknRollover;
    uint8_t reserved4[6];
    uint8_t usePPP;
    uint8_t aopCfg;
    uint8_t reserved5[2];
    uint16_t aopOrbMaxErr;
    uint8_t reserved6[4];
    uint8_t reserved7[3];
    uint8_t useAdr;
} ubx_navx5_t;

typedef struct PACKED {
    uint8_t type;
    uint8_t reserved[3];
    int32_t lat; // 1e7 degrees
    int32_t lon; // 1e7 degrees
    int32_t alt; // cm WGS84
    uint32_t posAcc; // stddev cm
} ubx_mga_ini_pos_llh;

/*
 * parse a stream of characters supplied by getchar(), calling
 * msghandler for each ublox message which is found.
 * TODO: put a comment here explaining why we don't just pass in a buffer
 *
 * data is held in a buffer containing a sliding window hopefully
 * containing a message.  If a parse failure occurs the window is
 * moved forward one byte and parsing is re-attempted
 */
void ublox_parser(void *caller_state,
                  int (*getchar)(void *),
                  void (*msghandler)(void*, ublox_class_t, ublox_msg_t, const uint8_t*, const uint32_t))
{
    uint8_t buffer[250]; // contains sliding window
    uint8_t *window_start = buffer;
    uint8_t *window_end = buffer;
    uint8_t *window_feed = buffer;

    typedef enum {
        id_state_waiting_preamble1,
        id_state_waiting_preamble2,
        id_state_waiting_class,
        id_state_waiting_msg_id,
        id_state_waiting_len1,
        id_state_waiting_len2,
        id_state_waiting_mga_ano_body,
        id_state_waiting_cksum_a,
        id_state_waiting_cksum_b
    } id_state_t;
    id_state_t id_state;

    uint8_t mga_ano_body_count = 0;  // initialised to shut compiler up

    id_state = id_state_waiting_preamble1;

    uint8_t mga_len = 0;  // initialised to shut compiler up

    uint8_t class_id = 0; // initialised to shut compiler up
    uint8_t msg_id = 0; // initialised to shut compiler up

    uint8_t cksum_a = 0;  // initialised to shut compiler up
    uint8_t cksum_b = 0;  // initialised to shut compiler up

    /* ublox_debug("size is %u\n", sizeof(struct ubx_mga_ano)); */
    while (1) {
        uint8_t data;
        if (window_feed < window_end) {
            data = *window_feed++; // love this
        } else {
            // new data needed!
            // this could be read(...) rather than getchar for efficiency
            int x = getchar(caller_state);
            if (x == -1) {
                break;
            }
            data = (uint8_t)x;
            /* printf("window_end=%u size=%u\n", window_end, sizeof(buffer)); */
            /* printf("delta=%u\n", window_end-buffer); */
            if (window_end-buffer == sizeof(buffer)) {
                // shift data back in the buffer
                /* printf("Window shift! start=%u\n", window_start); */
                memcpy(buffer, window_start, window_end-window_start);
                window_end = buffer+(window_end-window_start);
                window_start = buffer;
            }
            *window_end++ = data;
            window_feed = window_end;
        }

        /* ublox_debug("state=%lu char=%02x\n", id_state, data); */

        switch(id_state) {
        case id_state_waiting_preamble1:
            if (data != PREAMBLE1) {
                goto parse_failure;
            }
            id_state = id_state_waiting_preamble2;
            break;
        case id_state_waiting_preamble2:
            if (data != PREAMBLE2) {
                goto parse_failure;
            }
            id_state = id_state_waiting_class;
            break;
        case id_state_waiting_class:
            class_id = data;
            cksum_a = cksum_b = data;
            /* if (data != CLASS_MGA) { */
            /*     goto parse_failure; */
            /* } */
            id_state = id_state_waiting_msg_id;
            break;
        case id_state_waiting_msg_id:
            msg_id = data;
            cksum_b += (cksum_a += data);
            /* if (data != MSG_MGA_ANO) { */
            /*     goto parse_failure; */
            /* } */
            mga_ano_body_count = 0;
            id_state = id_state_waiting_len1;
            break;
        case id_state_waiting_len1:
            cksum_b += (cksum_a += data);
            mga_len = data;
            /* ublox_debug("Got len1 %u\n", mga_len); */
            id_state = id_state_waiting_len2;
            break;
        case id_state_waiting_len2:
            cksum_b += (cksum_a += data);
            mga_len += data<<16;
            /* ublox_debug("Got length %u\n", mga_len); */
            id_state = id_state_waiting_mga_ano_body;
            break;
        case id_state_waiting_mga_ano_body:
            cksum_b += (cksum_a += data);
            if (++mga_ano_body_count < mga_len) {
                break;
            }
            id_state = id_state_waiting_cksum_a;
            break;
        case id_state_waiting_cksum_a:
            if (data != cksum_a) {
                ublox_debug("checksum_a mismatch\n");
                goto parse_failure;
            } else {
                /* ublox_debug("checksum_a match\n"); */
            }
            id_state = id_state_waiting_cksum_b;
            break;
        case id_state_waiting_cksum_b:
            if (data != cksum_b) {
                ublox_debug("checksum_b mismatch\n");
                goto parse_failure;
            } else {
                /* ublox_debug("checksum_b match\n"); */
            }
            msghandler(caller_state, class_id, msg_id, window_start, window_feed-window_start);
            window_start = window_feed;
            id_state = id_state_waiting_preamble1;
            continue;
        parse_failure:
            ublox_debug("Parse failure in state (%u)\n", id_state);
            // try parsing again with the first byte being the next
            // one along from where we thought we had a message the
            // first time
            window_start++;
            window_feed = window_start;
            id_state = id_state_waiting_preamble1;
        }
    }
}


/*
 * ensures the logging directory exists:
 */
static void ensure_basedir()
{
    if (exists(UBLOX_BASEDIR)) {
        return;
    }
    if (!mkdir(UBLOX_BASEDIR)) {
        ublox_debug("failed to mkdir " UBLOX_BASEDIR "\n");
    }
}

// read file data from path into *ret; caller to free using talloc_free
// returns -1 on failure, 0 on success
static int read_file(const char *path, uint8_t **data, uint32_t *datalen)
{
    int ret = -1;
    if (!exists(__DECONST(char *, path))) {
        goto out;
    }
    int size = mw_get_filesize(path);
    if (size < 0) {
        //ublox_debug("File (%s): Failed to get size\n", path);
        goto out;
    }
    if (size > MAX_DATA_SIZE) { // that's 1 megabyte
        ublox_debug("File (%s): too large for read_fileing (%d)\n", path, size);
        goto out;
    }
    FIL fh;
    if (f_open(&fh, path, FA_READ) != FR_OK) {
        //ublox_debug("File (%s): failed to open\n", path);
        goto out;
    }

    ublox_debug("read_file: (%d) (%s)\n", size, path);

    if (!(*data = (unsigned char *)talloc_zero_size(NULL, size+1))) {
        ublox_debug("Failed to allocate (%d) bytes for (%s)\n", size, path);
        goto out_close;
    }

    uint32_t bytes_read = 0;
    while (bytes_read < size) {
        uint32_t read_count;
        if (f_read(&fh, &((*data)[bytes_read]), size-bytes_read, &read_count) != FR_OK) {
            console_printf("nc_file: fread failed\n");
            break;
        }
        if (read_count == -1) {
            console_printf("nc_file: -1 in read_count\n"); /* ?! */
            break;
        }
        if (read_count == 0) {
            console_printf("nc_file: fread EOF\n"); /* I hope */
            break;
        }
        bytes_read += read_count;
    }

    if (bytes_read == size) {
        ret = 0;
        *datalen = size;
    }

out_close:
    f_close(&fh);

out:
    return ret;
}

/*
  save position in nvram
 */
static void save_gps_pos_nvram(struct mga_position *pos)
{
    snx_nvram_integer_set("SkyViper", __DECONST(char *, "POS_LAT"), pos->latitude);    
    snx_nvram_integer_set("SkyViper", __DECONST(char *, "POS_LON"), pos->longitude);    
    snx_nvram_integer_set("SkyViper", __DECONST(char *, "POS_ALT"), pos->altitude_cm);
    snx_nvram_integer_set("SkyViper", __DECONST(char *, "POS_TIME"), pos->utc_time);
}

/*
  save position on microSD or NVRAM
 */
static void save_gps_pos(struct mga_position *pos)
{
    char *buf = print_printf(NULL, "%d %d %d %u\r\n", pos->latitude, pos->longitude, pos->altitude_cm, pos->utc_time);
    if (buf) {
        if (dev_write_file(ublox_gps_pos_filepath, (const uint8_t *)buf, talloc_get_size(buf)) == 0) {
            if (pos_source != POS_SOURCE_GPS) {
                console_printf("Saved GPS position %d %d %d\n", pos->latitude, pos->longitude, pos->altitude_cm);
            }
        } else {
            save_gps_pos_nvram(pos);
        }
        talloc_free(buf);
    }
}

/*
  load position from nvram
 */
static void load_gps_pos_nvram()
{
    int lat, lon, alt_cm, utc_time;

    if (snx_nvram_integer_get("SkyViper", __DECONST(char *, "POS_LAT"), &lat) == NVRAM_SUCCESS &&
        snx_nvram_integer_get("SkyViper", __DECONST(char *, "POS_LON"), &lon) == NVRAM_SUCCESS &&
        snx_nvram_integer_get("SkyViper", __DECONST(char *, "POS_ALT"), &alt_cm) == NVRAM_SUCCESS &&
        snx_nvram_integer_get("SkyViper", __DECONST(char *, "POS_TIME"), &utc_time) == NVRAM_SUCCESS) {
        mga_pos_estimate.latitude = lat;
        mga_pos_estimate.longitude = lon;
        mga_pos_estimate.altitude_cm = alt_cm;
        pos_source = POS_SOURCE_MICROSD;
        if (utc_time > UBLOX_MIN_DATE) {
            mga_pos_estimate.utc_time = utc_time;
            time_source = TIME_SOURCE_MICROSD;
        }
    }
}


/*
  load position from nvram
 */
static void load_gps_pos()
{
    char *buf = NULL;
    uint32_t size = 0;
    if (read_file(ublox_gps_pos_filepath, (uint8_t **)&buf, &size) == 0 && buf) {
        int lat, lon, alt;
        int utc_time = 0;
        sscanf(buf, "%d %d %d %d", &lat, &lon, &alt, &utc_time);
        talloc_free(buf);
        if (lat != 0 || lon != 0) {
            mga_pos_estimate.latitude = lat;
            mga_pos_estimate.longitude = lon;
            mga_pos_estimate.altitude_cm = alt;
            console_printf("Pos source MicroSD\n");
            pos_source = POS_SOURCE_MICROSD;
        }
        if (utc_time > UBLOX_MIN_DATE) {
            mga_pos_estimate.utc_time = utc_time;
            // assume we're doing a 2 minute reboot
            mga_pos_estimate.time_base = -120;
            time_source = TIME_SOURCE_MICROSD;
        }
        console_printf("Loaded position %d %d %d T=%u\n", lat, lon, alt, utc_time);
    } else {
        load_gps_pos_nvram();
    }
}

/*
 * take lock protecting mga-offline data in memory
 * returns 0 if lock was taken, -1 otherwise
 */
static int mga_offline_lock()
{
    if (mga_offline_sem == NULL) {
        return -1;
    }
    if (xSemaphoreTake(mga_offline_sem, 100) != pdTRUE ) {
        console_printf("Failed to take mga-offline-semaphore\n");
        return -1;
    }
    return 0;
}
/*
 * returns 0 if the lock could be taken, -1 otherwise
 */
static int mga_offline_poll()
{
    if (mga_offline_sem == NULL) {
        return -1;
    }
    if (xSemaphoreTake(mga_offline_sem, 0) != pdTRUE ) {
        return -1;
    }
    return 0;
}

/*
 * release lock protecting mga-offline data in memory
 */
static void mga_offline_unlock()
{
    xSemaphoreGive(mga_offline_sem);
}

/*
 * read mga-offline data from nvram
 */
static int mga_offline_read_nvram(uint8_t **data, uint32_t *retlen)
{
    unsigned int len = 0;
    int ret = snx_nvram_get_data_len("UBLOX", "MGA_DATA", &len);
    if (ret != NVRAM_SUCCESS || len == 0 || len > 200*1000) {
        return -1;
    }
    *data = talloc_zero_size(NULL, len);
    if (*data == NULL) {
        return -1;
    }
    ret = snx_nvram_binary_get("UBLOX", "MGA_DATA", *data);
    if (ret != NVRAM_SUCCESS) {
        console_printf("failed to load mga of len %u from NVRAM\n", len);
        talloc_free(*data);
        *data = NULL;
        return -1;
    }
    console_printf("loaded mga of len %u from NVRAM\n", *retlen);
    *retlen = len;
    return 0;
}


/*
 * read mga-offline data from set path in storage;
 * caller to free vMalloced *ret
 */
static int mga_offline_read_file(uint8_t **data, uint32_t *retlen)
{
    int ret = read_file(ublox_mga_offline_filepath, data, retlen);
    if (ret != 0) {
        ret = mga_offline_read_nvram(data, retlen);
    }
    return ret;
}

/*
 * write mga-offline data to nvram
 */
static int mga_offline_write_nvram(const uint8_t *data, uint32_t datalen)
{
    if (snx_nvram_binary_set("UBLOX", "MGA_DATA", __DECONST(uint8_t *,data), datalen) != NVRAM_SUCCESS) {
        console_printf("failed to save mga of len %u to NVRAM\n", datalen);
        return -1;
    }
    console_printf("saved mga of len %u to NVRAM\n", datalen);
    return 0;
}

/*
 * write mga-offline data to set path in storage;
 */
static int mga_offline_write_file(const uint8_t *data, uint32_t datalen)
{
    int ret = dev_write_file(ublox_mga_offline_filepath, data, datalen);
    if (ret != 0) {
        ret = mga_offline_write_nvram(data, datalen);
    }
    return ret;
}

/*
 * pack payload into a UBX message
 */
static uint16_t ublox_make_ubx(uint8_t *ubx,
                               const ublox_class_t class_id,
                               const ublox_msg_t msg_id,
                               const uint8_t *payload, const uint16_t len)
{
    // page 125 / 126
    uint16_t o = 0;
    ubx[o++] = PREAMBLE1;
    ubx[o++] = PREAMBLE2;
    ubx[o++] = class_id;
    ubx[o++] = msg_id;
    ubx[o++] = len & 0xff;
    ubx[o++] = len >> 8;
    memcpy(&ubx[o], payload, len);
    o += len;

    uint8_t cksum_a = 0;
    uint8_t cksum_b = 0;
    // calculate checksum:
    uint16_t i;
    for(i=2; i<o; i++) {
        cksum_b += (cksum_a += ubx[i]);
    }
    ubx[o++] = cksum_a;
    ubx[o++] = cksum_b;
    return o;
}

/*
  return utc time as a string
 */
static const char *utc_time_string(uint32_t utc_time)
{
    static char ret[100];
    system_date_t result = {};
    break_date(&result, utc_time);
    snprintf(ret, sizeof(ret), "%04u-%02u-%02u %02u:%02u:%02u",
             result.year, result.month, result.day,
             result.hour, result.minute, result.second);
    return &ret[0];
}

/*
 * create and pack an mga-ini-time corresponding to epoch_time_seconds
 */
static void ublox_pack_mga_ini_time_utc(ubx_ini_time_utc_t *msg,
                                        const time_t epoch_time_seconds)
{
    system_date_t result;
    break_date(&result, epoch_time_seconds);

    msg->type = 0x10; // magic number from datasheet
    msg->version = 0x0;
    msg->ref = 0;
    msg->leapSecs = LEAPSECS_SINCE_1980;;
    msg->year = result.year;
    msg->month = result.month;
    msg->day = result.day;
    msg->hour = result.hour;
    msg->minute = result.minute;
    msg->second = (result.second == 60) ? 59 : result.second; // FIXME!
    msg->reserved1 = 0;
    msg->ns = 0;
    // set better time accuracy if using app for time
    msg->tAccS = time_source>=TIME_SOURCE_APP?120:3000;
    msg->reserved2[0] = 0;
    msg->reserved2[1] = 0;
    msg->tAccNs = 999999999;

#if 0
    console_printf("Setting ublox date: %04u-%02u-%02u %02u:%02u:%02u\n",
                   result.year, result.month, result.day, result.hour, result.minute, result.second);
#endif
}


static uint16_t ublox_make_ubx_mga_ini_time_utc(uint8_t *ubx, const time_t epoch_time_seconds)
{
    ubx_ini_time_utc_t msg;
    ublox_pack_mga_ini_time_utc(&msg, epoch_time_seconds);
    return ublox_make_ubx(ubx, CLASS_MGA, MSG_MGA_INI_TIME_UTC, (const uint8_t*)&msg, sizeof(msg));
}

static uint16_t ublox_make_ubx_navx5_ack_aiding(uint8_t *ubx)
{
    ubx_navx5_t msg;
    memset(&msg, 0, sizeof(msg));
    msg.ackAiding = 1;
    msg.mask1 = 1U<<10;
    return ublox_make_ubx(ubx, CLASS_CFG, MSG_CFG_NAVX5, (const uint8_t*)&msg, sizeof(msg));
}


static uint16_t ublox_make_ubx_mga_ini_pos_llh(uint8_t *ubx, const struct mga_position *pos)
{
    ubx_mga_ini_pos_llh msg = {};
    msg.type = 1; // pos-llh
    msg.lat = pos->latitude;
    msg.lon = pos->longitude;
    msg.alt = pos->altitude_cm;
    msg.posAcc = 10000; // assume 100m
    return ublox_make_ubx(ubx, CLASS_MGA, MSG_MGA_INI_POS_LLH, (const uint8_t*)&msg, sizeof(msg));
}

/*
 * handle data which has been uploaded to the Sonix via a listened-on port
 * Unconditionally stores the data to the SD card (if present)
 * parses the offline data and may 
 */
void handle_ublox_data(const uint8_t *data, const uint32_t data_len)
{
    ublox_debug("[uatp] handle_ublox_data starting\n");
    if (mga_offline_sem == NULL) {
        ublox_debug("ublox not initialised\n");
        return;
    }

    // cache the data in memory:
    // TODO: don't do this if we have a fix already
    // NOTE: we may not have an SD card, so we need to do this caching here
    ublox_debug("[uatp] caching in memory\n");
    if (mga_offline_lock() == -1) {
        return;
    }

    if (mga_offline_data != NULL) {
        if (mga_offline_data_len == data_len &&
            memcmp(data, mga_offline_data, data_len) == 0) {
            // its the same as what we have
            ublox_debug("[uatp] no change in data\n");
            mga_offline_unlock();
            return;
        }
        talloc_free(mga_offline_data);
        mga_offline_data = NULL;
    }

    mga_offline_data = (uint8_t *)talloc_size(NULL, data_len);
    if (mga_offline_data == NULL) {
        ublox_debug("Failed to allocate (%u) bytes\n", data_len);
        mga_offline_unlock();
        return;
    }
    memcpy(mga_offline_data, data, data_len);
    mga_offline_data_len = data_len;
    offline_assistance_data_supplied_sec = 0;

    mga_highest_cached_utc = 0;    

    char *md5 = md5_string(NULL, data, data_len);
    console_printf("MGA MD5: %s\n", md5);
    talloc_free(md5);
    
    // than what we've previously
    // sent

    mga_offline_unlock();

    /*
     * store it to the filesystem.  This is done second because if the
     * SD card is misbehaving we'd rather have the other thread have
     * access to the data sooner...
     */
    ublox_debug("[uatp] considering caching to SD card\n");
    ensure_basedir();
    if (mga_offline_lock() == 0) {
        mga_offline_write_file(data, data_len); // this may fail (e.g. no SD card!)
        mga_offline_unlock();
    } else {
        ublox_debug("[uatp] failed to lock offline data\n");
    }

    return;
}

/*
  set a position estimate for the GPS from the app or web browser
 */
void ublox_set_position(const struct mga_position *pos)
{
    if (pos && pos_source <= POS_SOURCE_APP) {
        mga_pos_estimate = *pos;
        pos_source = POS_SOURCE_APP;
        time_source = TIME_SOURCE_APP;
        mga_pos_estimate.time_base = get_sys_seconds_boot();
        console_printf("Pos source app\n");
    }
}

void ublox_set_time(uint32_t utc_time)
{
    if (utc_time > UBLOX_MIN_DATE &&
        time_source < TIME_SOURCE_APP) {
        mga_pos_estimate.utc_time = utc_time;
        mga_pos_estimate.time_base = get_sys_seconds_boot();
        time_source = TIME_SOURCE_APP;
    }
}

/*
* receive stream of ublox messages to cache and pass on to uBlox
*/
void ublox_assist_upload_task_process(void *pvParameters)
{
    port_read_then_call("uBlox Assist data", UBLOX_ASSIST_PORT, MAX_DATA_SIZE, handle_ublox_data);
}


/*
 * encapsulate data (of length len) in a mavlink GPS_INJECT_DATA and
 * enqueue that message to be sent by the mavlink thread
 * returns 0 on success, -1 on failure
 */
static int mavlink_inject(const uint8_t *data, uint16_t len)
{
    mavlink_message_t *msg;
    msg = talloc_zero(NULL, mavlink_message_t);
    if (msg == NULL) {
        return -1;
    }
    mavlink_msg_gps_inject_data_pack_chan(MAVLINK_SYSTEM_ID,
                                          1, // source component id
                                          MAVLINK_COMM_FC,
                                          msg,
                                          MAVLINK_TARGET_SYSTEM_ID,
                                          0,
                                          len,
                                          data);
    return mavlink_fc_send(msg);
}

/*
 * task responsible for ensuring the flight controller has a good time
 */
static void ublox_assist_task_process_set_ublox_time(void)
{
    if (mga_pos_estimate.utc_time < UBLOX_MIN_DATE) {
        // this looks like a time-since-boot; Sonix has no concept of current time
        return;
    }
    // ublox_debug("[uatp] enqueueing ublox time: %lu\n", (unsigned long)now);
    // we've seen a jump in the system time to something
    // looking more like a real-clock time.  Trust it and set
    // the GPS time appropriately.  We should NOT do this if
    // the GPS already has a time that looks reasonable, but
    // AP doesn't currently bare that.
    uint8_t ubx[UBLOX_MAX_MESSAGE_SIZE];
    uint32_t tnow = mga_pos_estimate.utc_time + (get_sys_seconds_boot() - mga_pos_estimate.time_base);
    uint16_t len = ublox_make_ubx_mga_ini_time_utc(ubx, tnow);
    mavlink_inject(ubx, len);
}

/*
  enable ack aiding in NAVX5
 */
static void ublox_assist_enable_ack_aiding(void)
{
    uint8_t ubx[UBLOX_MAX_MESSAGE_SIZE];
    uint16_t len = ublox_make_ubx_navx5_ack_aiding(ubx);
    mavlink_inject(ubx, len);
}

struct source_data_state {
    const uint8_t *data; // data to search for ubx messages
    const uint32_t data_len; // length of data to search
    uint32_t pos; // current position in search

    uint16_t best_message_start; // offset of message in source
    uint16_t best_message_len;
    int32_t best_message_timedelta;
    long long best_message_time;
    system_date_t result;
    uint32_t inject_count;
};

/*
 * provide a byte of ublox message data
 */
static int ublox_mga_offline_getchar(void *state)
{
    struct source_data_state *mystate = (struct source_data_state *)state;
    if (mystate->pos >= mystate->data_len) {
        return -1;
    }
    return mystate->data[mystate->pos++];
}

/*
 * Handle a ublox message found in uploaded data Is passed the entire
 * message, checksum and all compares found message against the best
 * message found so far, may take this in preference.
 */
static void ublox_mga_offline_msghandler(void *state, ublox_class_t class_id, ublox_msg_t msg_id, const uint8_t *msg, uint32_t msglen)
{
    if (class_id != CLASS_MGA || msg_id != MSG_MGA_ANO) {
        ublox_debug("Received unexpected message (%u:%u)\n", class_id, msg_id);
        return;
    }
    struct source_data_state *mystate = (struct source_data_state *)state;
    const ubx_mga_ano_t *fields = (ubx_mga_ano_t*)&msg[sizeof(struct ubx_header)];
    //ublox_debug("year: %u\n", fields->year);
    //ublox_debug("month: %u\n", fields->month);
    //ublox_debug("day: %u\n", fields->day);

    system_date_t pdate = {};
    pdate.year = fields->year + 2000;
    pdate.month = fields->month;
    pdate.day = fields->day;
    const long long ubx_time = tm_to_time(&pdate, 0);

    if (ubx_time > mga_highest_cached_utc) {
        // remember the highest date we have
        mga_highest_cached_utc = ubx_time;
    }
    
    long long now = get_sys_seconds_utc();
    int32_t delta = now - ubx_time;

    // ublox_debug("ubxtime=%lu now=%lu delta=%ld\n", (unsigned long)ubx_time, (unsigned long)now, (long)delta);
    if (mystate->best_message_timedelta == 0 ||
        (delta >= 0 &&
         delta < mystate->best_message_timedelta)) {
        mystate->best_message_start = mystate->pos;
        mystate->best_message_len = msglen;
        mystate->best_message_timedelta = delta;
        mystate->best_message_time = ubx_time;
    }
    return;
}

static bool mga_logging = false;
static FIL mga_log;

/*
 * Handle a ublox message found in uploaded data Is passed the entire
 * message, checksum and all compares found message against the best
 * message found so far, may take this in preference.
 */
static void ublox_mga_offline_msgsender(void *state, ublox_class_t class_id, ublox_msg_t msg_id, const uint8_t *msg, uint32_t msglen)
{
    if (class_id != CLASS_MGA || msg_id != MSG_MGA_ANO) {
        ublox_debug("Received unexpected message (%u:%u)\n", class_id, msg_id);
        return;
    }
    struct source_data_state *mystate = (struct source_data_state *)state;
    const ubx_mga_ano_t *fields = (ubx_mga_ano_t*)&msg[sizeof(struct ubx_header)];

    system_date_t pdate = {};
    pdate.year = fields->year + 2000;
    pdate.month = fields->month;
    pdate.day = fields->day;

    if (pdate.year != mystate->result.year ||
        pdate.month != mystate->result.month ||
        pdate.day != mystate->result.day) {
        // not right date
        return;
    }

    mystate->inject_count++;
    mavlink_inject(msg, msglen);

    if (mga_logging) {
        uint32_t bytes_written = 0;
        f_write(&mga_log, msg, msglen, &bytes_written);
    }
}

/*
  parse MGA data, setting up mga_highest_cached_utc
 */
static void ublox_mga_parse(void)
{
    struct source_data_state state = {
        mga_offline_data, // data to search
        mga_offline_data_len, // length of data to search
        0, // current getchar offset

        0, // best message start
        0, // best message length
        0, // best message time-delta
    };

    ublox_parser(&state, ublox_mga_offline_getchar, ublox_mga_offline_msghandler);
}

/*
 * Search ublox offline data to find the most appropriate
 * ublox-offline-assist message to send to the ublox unit
 * MUST be called with mga_offline_sem held
 * returns 0 for success and 1 for failure
 */
static int ublox_mga_offline_find_ubx(system_date_t date)
{
    /* ublox_debug("Looking for data for date: %04u-%02u-%02u\n", date.year, date.month, date.day); */
    if (mga_offline_data == NULL) {
        // attempt to read from SD card:
        uint8_t *data;
        uint32_t datalen;
        if (mga_offline_read_file(&data, &datalen) == -1) {
            return -1;
        }
        mga_offline_data = data;
        mga_offline_data_len = datalen;
        char *md5 = md5_string(NULL, data, datalen);
        console_printf("Loaded MGA MD5: %s\n", md5);
        talloc_free(md5);
    }

    // search the UBX data for a good message to send; that's the one
    // closest to today....

    struct source_data_state state = {
        mga_offline_data, // data to search
        mga_offline_data_len, // length of data to search
        0, // current getchar offset

        0, // best message start
        0, // best message length
        0, // best message time-delta
    };

    ublox_parser(&state, ublox_mga_offline_getchar, ublox_mga_offline_msghandler);

    // rewind buffer
    state.pos = 0;
    
    if (state.best_message_len == 0) {
        // no suitable message found
        return -1;
    }

    time_to_tm(state.best_message_time, 0, &state.result);

    mga_upload_utc = state.best_message_time;

    if (mga_logging) {
        f_open(&mga_log, "mga.log", FA_CREATE_ALWAYS|FA_WRITE);
    }
    
    ublox_assist_enable_ack_aiding();
    ublox_assist_task_process_set_ublox_time();

    if (mga_pos_estimate.latitude ||
        mga_pos_estimate.longitude ||
        mga_pos_estimate.altitude_cm) {
        uint8_t ubx[UBLOX_MAX_MESSAGE_SIZE];
        console_printf("Sending MGA_INI_POS %ld %ld %ld\n",
                       (long)mga_pos_estimate.latitude, 
                       (long)mga_pos_estimate.longitude,
                       (long)mga_pos_estimate.altitude_cm);
        uint16_t len = ublox_make_ubx_mga_ini_pos_llh(ubx, &mga_pos_estimate);
        mavlink_inject(ubx, len);
    }
    
    ublox_parser(&state, ublox_mga_offline_getchar, ublox_mga_offline_msgsender);

    if (mga_logging) {
        f_close(&mga_log);
    }
    
    console_printf("injected %u messages for %04u-%02u-%02u (age=%ld s)\n",
                   (unsigned)state.inject_count,
                   (unsigned)state.result.year,
                   (unsigned)state.result.month,
                   (unsigned)state.result.day,
                   (long)state.best_message_timedelta);
    
    return 0;
}

/*
 * find and queue-for-injection a UBlox mga-offline packet
 */
static int upload_mga_offline()
{
    system_date_t broken_date = {};
    break_date(&broken_date, fc_unix_time);

    // ensure the app doesn't update the offline data on us:
    if (mga_offline_lock() == -1) {
        return -1;
    }

    if (ublox_mga_offline_find_ubx(broken_date) == -1) {
        mga_offline_unlock();
        return -1;
    }

    offline_assistance_data_supplied_utc = mga_upload_utc;
    offline_assistance_data_supplied_sec = get_sys_seconds_boot();
    
    mga_offline_unlock();

    return 0;
}

void ublox_assist_task_process(void *pvParameters)
{
    /* vTaskDelete(NULL); */
    /* return; */

    ensure_basedir();

    load_gps_pos();
    
    while (1) {
        uint32_t now = get_sys_seconds_boot();
        
        if (time_source < TIME_SOURCE_FC &&
            fc_unix_time > UBLOX_MIN_DATE) {
            mga_pos_estimate.utc_time = fc_unix_time;
            mga_pos_estimate.time_base = get_sys_seconds_boot();
            time_source = TIME_SOURCE_FC;
        }

        if (pos_source == POS_SOURCE_NONE) {
            load_gps_pos();
        }

        // record time to fix
        if (fix_wait_start == 0 && get_gps_fix_type() == 1) {
            fix_wait_start = now;
        }
        if (fix_time == 0 && get_gps_fix_type() >= 3) {
            fix_time = now - fix_wait_start;
        }
        
        uint32_t time_estimate = mga_pos_estimate.utc_time;
        
        if (time_estimate < UBLOX_MIN_DATE) {
            // can't do anything till we have the time
            mdelay(1000);
            continue;
        }

        // save GPS lock every 30 seconds when we have lock
        static uint32_t last_gps_pos_save_s;
        if (get_gps_fix_type() >= 3 &&
            now - last_gps_pos_save_s > 30) {
            if (!last_gps_pos_save_s && 
        	mga_pos_estimate.utc_time > UBLOX_MIN_DATE) {
              console_printf("setting time to %u\n", mga_pos_estimate.utc_time);
              set_date_utc(mga_pos_estimate.utc_time);
            }

            get_fc_gps_lat_lon(&mga_pos_estimate);
            save_gps_pos(&mga_pos_estimate);
            pos_source = POS_SOURCE_GPS;
            last_gps_pos_save_s = now;
        }

        if (get_gps_fix_type() != 1) {
            // need to wait for GPS state 1
            mdelay(1000);
            continue;
        }

        // load offline cache if not loaded already
        if (mga_offline_data_len == 0 &&
            mga_offline_lock() == 0) {
            mga_offline_read_file(&mga_offline_data, &mga_offline_data_len);
            mga_offline_unlock();
        }

        if (mga_offline_data_len == 0) {
            // no data to send
            mdelay(1000);
            continue;
        }

        if (time_source == 0) {
            // we need the time
            mdelay(1000);
            continue;                        
        }
        
        if (last_time_source >= time_source &&
            last_pos_source >= pos_source &&
            offline_assistance_data_supplied_utc != 0 &&
            offline_assistance_data_supplied_sec != 0) {
            // already done the best we can
            mdelay(1000);
            continue;            
        }
        
        console_printf("MGA upload pos-source=%u time-source=%u\n",
                       pos_source, time_source);
        
        // upload offline data
        if (upload_mga_offline() == 0) {
            last_time_source = time_source;
            last_pos_source = pos_source;
        }

        mdelay(1000); // run at 1Hz
    }
    ublox_debug("ublox_assist_task_process exiting\n");
    vTaskDelete(NULL);
}

/*
  send JSON encoded ublox status
 */
void ublox_send_status_json(struct sock_buf *sock)
{
    if (mga_offline_data_len > 0 && mga_highest_cached_utc == 0) {
        ublox_mga_parse();
    }
    sock_printf(sock, "{ \"fc_time\" : %lu, \"uploaded_offline\" : %u, \"offline_cache_size\" : %u, \"fix_type\" : %u, \"upload_utc\" : %lu, \"highest_utc\" : %lu, \"fix_wait_start\" : %u, \"fix_time\" : %u, \"pos_source\" : %u, \"time_source\" : %u }",
                (unsigned long)fc_unix_time,
                offline_assistance_data_supplied_sec?1:0,
                (unsigned)mga_offline_data_len,
                get_gps_fix_type(),
                (unsigned long)offline_assistance_data_supplied_utc,
                (unsigned long)mga_highest_cached_utc,
                fix_wait_start,
                fix_time,
                last_pos_source,
                last_time_source);
}


/*
  command-line support
*/
void cmd_ublox_status(unsigned argc, const char *argv[])
{
    if (mga_offline_data_len > 0 && mga_highest_cached_utc == 0) {
        ublox_mga_parse();
    }
    
    console_printf("ublox: pos=(%d %d %d) t=%s\n", mga_pos_estimate.latitude, mga_pos_estimate.longitude, mga_pos_estimate.altitude_cm,
                   utc_time_string(mga_pos_estimate.utc_time + (get_sys_seconds_boot() - mga_pos_estimate.time_base)));
    console_printf("ublox: time-source=%u pos-source=%u fix-type=%u\n",
                   time_source, pos_source, get_gps_fix_type());
    console_printf("ublox: highest_utc=%u\n",
                   mga_highest_cached_utc);
    console_printf("ublox: uploaded-time=%u uploaded-pos=%u\n",
                   last_time_source, last_pos_source);
    console_printf("ublox: fc-time=%s\n", utc_time_string(fc_unix_time));
    console_printf("ublox: port=%u \n", UBLOX_ASSIST_PORT);
    console_printf("ublox: time=%s\n", utc_time_string(get_sys_seconds_utc()));
    console_printf("ublox: fix_wait_start=%u fix_time=%u\n", fix_wait_start, fix_time);
    
    if (exists(__DECONST(char*,ublox_mga_offline_filepath))) {
        int size = mw_get_filesize(ublox_mga_offline_filepath);
        console_printf("ublox: %s=%d bytes\n", ublox_mga_offline_filepath, size);
    } else {
        console_printf("ublox: %s=missing\n", ublox_mga_offline_filepath);
    }
    if (mga_offline_data == NULL) {
        console_printf("ublox: offline-cache=empty\n");
    } else {
        console_printf("ublox: offline-cache=%u bytes\n", mga_offline_data_len);
    }
    if (mga_offline_poll() == 0) {
        console_printf("ublox: offline-semaphore=untaken\n");
        mga_offline_unlock();
    } else {
        console_printf("ublox: offline-semaphore=taken\n");
    }
}

/*
  force re-upload
*/
void cmd_ublox_upload(void)
{
    upload_mga_offline();
}

/*
  end command-line support
*/
