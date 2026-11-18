/*
  MAVLink FTP (FILE_TRANSFER_PROTOCOL) for the SD card, served by the
  camera component to GCSes on WiFi, so logs, videos and photos can
  be fetched (and files put, renamed or removed) with a GCS's MAVLink
  FTP as well as through the web interface.

  It follows ArduPilot's implementation (GCS_FTP.cpp): the same
  opcodes, errors, directory listing format, burst reads and
  resending of a reply when its request is repeated, with a few
  sessions.  Paths are on the card, from its root.

  Requests are handed to our own task, which does all the file work
  and queues its replies for the MAVLink thread to pack and send,
  each only to the GCS that asked.  The MAVLink thread owns the UDP
  socket (lwIP 1.4 doesn't allow two tasks to send on one) and the
  camera component's sequence numbers, and must never wait on the
  card: it also has to keep up with the flight controller's UART.

  Files being written by the remote logger, or a video being
  recorded, can be read but not removed, renamed or written.
 */

#include <FreeRTOS.h>
#include <bsp.h>
#include <task.h>
#include <queue.h>
#include <nonstdlib.h>
#include <stdio.h>
#include <string.h>
#include <libmid_fatfs/ff.h>
#include "mavlink_ftp.h"
#include "mavlink_remote_log.h"
#include "dev_console.h"
#include "record/rec_schedule.h"

#define FTP_DATA_LEN (MAVLINK_FTP_PAYLOAD_LEN - 12)
#define FTP_MAX_SESSIONS 3
// a session idle this long may be taken over by another GCS
#define FTP_SESSION_TIMEOUT_MS 3000
// and one idle this long is closed
#define FTP_SESSION_KILL_TIMEOUT_MS 20000
// how long to wait for room in the reply queue
#define FTP_SEND_TIMEOUT_MS 3000
// packets in one burst read; the GCS asks for another to go on.
// ArduPilot sends 2000, enough for a whole parameter file; ours are
// shorter so each takes well under a second on WiFi: pymavlink's
// mavftp gives up on a burst it hasn't finished within its idle time
// (3.7 s), as it has sent nothing since asking for it
#define FTP_BURST_PACKETS 250
#define FTP_REQUEST_QUEUE_LEN 8
#define FTP_REPLY_QUEUE_LEN 16
// upstream, but not in our mavlink headers yet
#define FTP_OPCODE_LISTDIRECTORYWITHTIME 16

/*
  Every FatFs call here goes through a queue to FatFs's own task,
  which also serves the recorder and remote logger, so each costs
  several milliseconds; reading a file 239 bytes at a time, or
  skipping a directory from its start for every page of a listing,
  was slow.  So files are read a block at a time, and a listing
  carries on from where the last page stopped.

  Blocks are read at their own offset into a buffer aligned like the
  web server's (whose 2k reads come out right): read into an
  unaligned buffer, this FatFs's direct multi-sector reads came out
  wrong
 */
#define FTP_BLOCK_LEN 2048

struct ftp_txn {
    struct sockaddr_in addr;
    uint32_t sysid;
    uint8_t compid;
    uint16_t seq;
    uint8_t session;
    uint8_t opcode;
    uint8_t size;
    uint8_t req_opcode;
    uint8_t burst_complete;
    uint32_t offset;
    uint8_t data[FTP_DATA_LEN];
};

struct ftp_session {
    uint8_t block[FTP_BLOCK_LEN] __attribute__((aligned(32)));
    uint32_t block_ofs;         // the part of the file in block[]
    uint32_t block_len;
    FIL fh;
    bool open;
    bool writing;
    uint32_t last_send_ms;      // 0 when idle
    int16_t id;                 // -1 until first used
    struct sockaddr_in addr;
    uint32_t sysid;
    uint8_t compid;
    // a listing in progress: the next page carries on from here
    DIR dir;
    bool dir_open;
    bool dir_with_time;
    uint32_t dir_next;          // the offset the next page asks for
    char dir_path[FTP_DATA_LEN + 2];
    char dir_pending[FTP_DATA_LEN + 1]; // the entry that didn't fit
    unsigned dir_pending_len;
};

static xQueueHandle request_queue;
static xQueueHandle reply_queue;
static bool started;
static volatile uint32_t last_activity_ms;
static struct {
    uint32_t requests;
    uint32_t dropped;           // request queue full: lost, and the GCS retries
    uint32_t replies;
    uint32_t send_timeouts;
} stats;

// everything below here is only used by the FTP task
static struct ftp_session sessions[FTP_MAX_SESSIONS];
static struct ftp_txn last_reply;
static bool last_reply_resendable;
static uint32_t crc_table[256];
static uint8_t file_buf[FTP_BLOCK_LEN] __attribute__((aligned(32)));
static char path1[FTP_DATA_LEN + 2], path2[FTP_DATA_LEN + 2];
static char lfn[_MAX_LFN + 1];
static char entry_buf[_MAX_LFN + 40];

static void ftp_task(void *pvParameters);

static bool ftp_start(void)
{
    if (started) {
        return true;
    }
    request_queue = xQueueCreate(FTP_REQUEST_QUEUE_LEN, sizeof(struct ftp_txn));
    reply_queue = xQueueCreate(FTP_REPLY_QUEUE_LEN, sizeof(struct mavlink_ftp_reply));
    if (request_queue == NULL || reply_queue == NULL ||
        xTaskCreate(ftp_task, "mavlink_ftp", STACK_SIZE_8K, NULL, 20, NULL) != pdPASS) {
        // nothing more to be done; and don't try again on every request
        console_printf("ftp: unable to start\n");
        if (request_queue != NULL) {
            vQueueDelete(request_queue);
            request_queue = NULL;
        }
        if (reply_queue != NULL) {
            vQueueDelete(reply_queue);
            reply_queue = NULL;
        }
    }
    started = true;
    return request_queue != NULL;
}

/*
  a FILE_TRANSFER_PROTOCOL message for us, from the MAVLink thread
 */
void mavlink_ftp_request(const mavlink_message_t *msg, const struct sockaddr_in *from)
{
    if (!ftp_start() || request_queue == NULL) {
        return;
    }
    mavlink_file_transfer_protocol_t pkt;
    mavlink_msg_file_transfer_protocol_decode(msg, &pkt);
    const uint8_t *p = pkt.payload;

    struct ftp_txn req;
    req.addr = *from;
    req.sysid = msg->sysid;
    req.compid = msg->compid;
    req.seq = p[0] | (p[1] << 8);
    req.session = p[2];
    req.opcode = p[3];
    req.size = p[4];
    req.req_opcode = p[5];
    req.burst_complete = p[6];
    req.offset = p[8] | (p[9] << 8) | (p[10] << 16) | ((uint32_t)p[11] << 24);
    memcpy(req.data, &p[12], FTP_DATA_LEN);

    stats.requests++;
    last_activity_ms = get_time_boot_ms();
    if (xQueueSend(request_queue, &req, 0) != pdTRUE) {
        // as ArduPilot does: treated like a lost packet, which the
        // GCS will send again, rather than a NAK that may confuse it
        stats.dropped++;
    }
}

/*
  the next reply to send, for the MAVLink thread
 */
bool mavlink_ftp_get_reply(struct mavlink_ftp_reply *reply)
{
    return reply_queue != NULL && xQueueReceive(reply_queue, reply, 0) == pdTRUE;
}

/*
  is a transfer going on?  The MAVLink thread looks for replies more
  often while one is
 */
bool mavlink_ftp_busy(void)
{
    return reply_queue != NULL &&
        (uxQueueMessagesWaiting(reply_queue) > 0 ||
         get_time_boot_ms() - last_activity_ms < 1000);
}

void mavlink_ftp_status(void)
{
    unsigned i, open = 0;
    for (i = 0; i < FTP_MAX_SESSIONS; i++) {
        if (sessions[i].open) {
            open++;
        }
    }
    console_printf("ftp: %s, %u requests (%u dropped), %u replies (%u timed out), %u files open\n",
                   !started ? "not started" : request_queue == NULL ? "failed to start" : "running",
                   (unsigned)stats.requests, (unsigned)stats.dropped,
                   (unsigned)stats.replies, (unsigned)stats.send_timeouts, open);
}

static bool same_gcs(const struct sockaddr_in *addr, uint32_t sysid, uint8_t compid,
                     const struct sockaddr_in *addr2, uint32_t sysid2, uint8_t compid2)
{
    return addr->sin_addr.s_addr == addr2->sin_addr.s_addr &&
        addr->sin_port == addr2->sin_port &&
        sysid == sysid2 && compid == compid2;
}

/*
  queue a reply for the MAVLink thread; false if there was no room
 */
static bool push_reply(const struct ftp_txn *reply)
{
    struct mavlink_ftp_reply r;
    r.addr = reply->addr;
    r.sysid = reply->sysid;
    r.compid = reply->compid;
    uint8_t *p = r.payload;
    memset(p, 0, sizeof(r.payload));
    p[0] = reply->seq & 0xFF;
    p[1] = reply->seq >> 8;
    p[2] = reply->session;
    p[3] = reply->opcode;
    p[4] = reply->size;
    p[5] = reply->req_opcode;
    p[6] = reply->burst_complete;
    p[8] = reply->offset & 0xFF;
    p[9] = (reply->offset >> 8) & 0xFF;
    p[10] = (reply->offset >> 16) & 0xFF;
    p[11] = reply->offset >> 24;
    // only the reply's own bytes, so the rest is zero and trimmed off
    memcpy(&p[12], reply->data, reply->size < FTP_DATA_LEN ? reply->size : FTP_DATA_LEN);

    last_activity_ms = get_time_boot_ms();
    if (xQueueSend(reply_queue, &r, FTP_SEND_TIMEOUT_MS / portTICK_PERIOD_MS) != pdTRUE) {
        stats.send_timeouts++;
        return false;
    }
    stats.replies++;
    return true;
}

static bool session_push_reply(struct ftp_session *s, const struct ftp_txn *reply)
{
    s->last_send_ms = get_time_boot_ms();
    const bool sent = push_reply(reply);
    if (reply->req_opcode == MAV_FTP_OPCODE_TERMINATESESSION) {
        s->last_send_ms = 0;
    }
    return sent;
}

static void session_close_dir(struct ftp_session *s)
{
    if (s->dir_open) {
        f_closedir(&s->dir);
        s->dir_open = false;
    }
}

static FRESULT session_close(struct ftp_session *s)
{
    FRESULT fr = FR_OK;
    if (s->open) {
        fr = f_close(&s->fh);
        s->open = false;
    }
    s->block_len = 0;
    session_close_dir(s);
    s->last_send_ms = 0;
    return fr;
}

/*
  read from the session's file at offset, through its block buffer;
  fewer bytes than asked for at the end of the file
 */
static FRESULT session_read(struct ftp_session *s, uint32_t offset, uint8_t *buf, unsigned len, unsigned *got)
{
    *got = 0;
    while (len > 0) {
        if (offset < s->block_ofs || offset >= s->block_ofs + s->block_len) {
            const uint32_t start = offset & ~(uint32_t)(FTP_BLOCK_LEN - 1);
            if (start >= f_size(&s->fh)) {
                break;
            }
            FRESULT fr = FR_OK;
            if (s->fh.fptr != start) {
                fr = f_lseek(&s->fh, start);
            }
            UINT br = 0;
            if (fr == FR_OK) {
                fr = f_read(&s->fh, s->block, FTP_BLOCK_LEN, &br);
            }
            if (fr != FR_OK) {
                s->block_len = 0;
                return fr;
            }
            s->block_ofs = start;
            s->block_len = br;
            if (offset >= start + br) {
                break;
            }
        }
        unsigned n = s->block_ofs + s->block_len - offset;
        if (n > len) {
            n = len;
        }
        memcpy(buf, &s->block[offset - s->block_ofs], n);
        buf += n;
        offset += n;
        len -= n;
        *got += n;
    }
    return FR_OK;
}

static void ftp_error(struct ftp_txn *reply, uint8_t error)
{
    reply->opcode = MAV_FTP_OPCODE_NAK;
    reply->data[0] = error;
    reply->size = 1;
}

// an errno-style code for the GCS to show
static void ftp_error_errno(struct ftp_txn *reply, uint8_t err)
{
    reply->opcode = MAV_FTP_OPCODE_NAK;
    reply->data[0] = MAV_FTP_ERR_FAILERRNO;
    reply->data[1] = err;
    reply->size = 2;
}

#define FTP_EIO 5
#define FTP_EBADF 9
#define FTP_ENOMEM 12
#define FTP_EACCES 13
#define FTP_EBUSY 16
#define FTP_ENODEV 19
#define FTP_ENOTDIR 20
#define FTP_EISDIR 21
#define FTP_EINVAL 22
#define FTP_EMFILE 24
#define FTP_ENOSPC 28
#define FTP_ENOTEMPTY 39

static void ftp_error_fr(struct ftp_txn *reply, FRESULT fr)
{
    switch (fr) {
    case FR_NO_FILE:
    case FR_NO_PATH:
        ftp_error(reply, MAV_FTP_ERR_FILENOTFOUND);
        return;
    case FR_EXIST:
        ftp_error(reply, MAV_FTP_ERR_FILEEXISTS);
        return;
    case FR_WRITE_PROTECTED:
        ftp_error(reply, MAV_FTP_ERR_FILEPROTECTED);
        return;
    case FR_DENIED:
        ftp_error_errno(reply, FTP_EACCES);
        return;
    case FR_INVALID_NAME:
    case FR_INVALID_PARAMETER:
    case FR_IS_ROOT_DIR:
        ftp_error_errno(reply, FTP_EINVAL);
        return;
    case FR_INVALID_OBJECT:
        ftp_error_errno(reply, FTP_EBADF);
        return;
    case FR_INVALID_DRIVE:
    case FR_NOT_ENABLED:
    case FR_NO_FILESYSTEM:
        ftp_error_errno(reply, FTP_ENODEV);
        return;
    case FR_TIMEOUT:
    case FR_LOCKED:
        ftp_error_errno(reply, FTP_EBUSY);
        return;
    case FR_NOT_ENOUGH_CORE:
    case FR_NOT_ENOUGH_BUF:
        ftp_error_errno(reply, FTP_ENOMEM);
        return;
    case FR_TOO_MANY_OPEN_FILES:
        ftp_error_errno(reply, FTP_EMFILE);
        return;
    default:
        ftp_error_errno(reply, FTP_EIO);
        return;
    }
}

/*
  is the request's name the right length, as ArduPilot checks it?
  It may or may not count a terminating NUL
 */
static bool check_name_len(const struct ftp_txn *req)
{
    const size_t len = strnlen((const char *)req->data, sizeof(req->data));
    if (req->size == 0) {
        return false;
    }
    if (len == req->size) {
        return true;
    }
    return req->size - len == 1 && req->data[sizeof(req->data) - 1] == 0;
}

/*
  a path from the GCS, made absolute (FatFs has a current directory,
  which the console's cd changes) and without a trailing /
 */
static void make_path(const char *name, size_t max, char *path)
{
    const size_t n = strnlen(name, max);
    size_t o = 0;
    if (n == 0 || name[0] != '/') {
        path[o++] = '/';
    }
    memcpy(&path[o], name, n);
    o += n;
    path[o] = 0;
    while (o > 1 && path[o - 1] == '/') {
        path[--o] = 0;
    }
}

static bool request_path(struct ftp_txn *req, struct ftp_txn *reply, char *path)
{
    if (!check_name_len(req)) {
        ftp_error(reply, MAV_FTP_ERR_INVALIDDATASIZE);
        return false;
    }
    make_path((const char *)req->data, sizeof(req->data), path);
    return true;
}

static bool ends_with_nocase(const char *s, const char *suffix)
{
    const size_t n = strlen(s), m = strlen(suffix);
    if (n < m) {
        return false;
    }
    s += n - m;
    while (*s) {
        char a = *s++, b = *suffix++;
        if (a >= 'A' && a <= 'Z') {
            a += 'a' - 'A';
        }
        if (b >= 'A' && b <= 'Z') {
            b += 'a' - 'A';
        }
        if (a != b) {
            return false;
        }
    }
    return true;
}

/*
  is this file being written by someone else?  The remote log being
  written, and the video being recorded (named _0000 until it is
  closed and renamed with its length).  FatFs here has no locking,
  so changing one of these under its writer would corrupt the card
 */
static bool path_busy(const char *path)
{
    const char *log = mavlink_remote_log_current_file();
    if (log != NULL && ends_with_nocase(path, log) && strlen(path) == strlen(log)) {
        return true;
    }
    return schedrec_state() == 1 && ends_with_nocase(path, "_0000.mp4");
}

/*
  a FAT date and time as seconds since 1970, or 0 if there's none.
  Cards written without a clock carry 1980, FAT's own epoch
 */
static uint32_t fat_mtime(WORD fdate, WORD ftime)
{
    int y = 1980 + (fdate >> 9);
    const unsigned m = (fdate >> 5) & 15, d = fdate & 31;
    if (y <= 1980 || m < 1 || m > 12 || d < 1) {
        return 0;
    }
    // days from civil, after Howard Hinnant
    y -= m <= 2;
    const int era = y / 400;
    const unsigned yoe = y - era * 400;
    const unsigned mp = m > 2 ? m - 3 : m + 9;
    const unsigned doy = (153 * mp + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    const uint32_t days = era * 146097 + doe - 719468;
    return days * 86400UL + (ftime >> 11) * 3600UL + ((ftime >> 5) & 63) * 60UL + (ftime & 31) * 2;
}

/*
  a listing entry, with its terminating NUL; its length, counting the
  NUL, or -1 to leave it out
 */
static int gen_dir_entry(const FILINFO *fno, const char *name, bool with_time)
{
    int n;
    if (with_time && strchr(name, '\t') != NULL) {
        // a tab separates the fields, so the spec has this sent as a skip
        n = snprintf(entry_buf, sizeof(entry_buf), "S");
    } else if (fno->fattrib & AM_DIR) {
        if (with_time) {
            n = snprintf(entry_buf, sizeof(entry_buf), "D%s\t0\t%u", name,
                         (unsigned)fat_mtime(fno->fdate, fno->ftime));
        } else {
            n = snprintf(entry_buf, sizeof(entry_buf), "D%s", name);
        }
    } else if (with_time) {
        n = snprintf(entry_buf, sizeof(entry_buf), "F%s\t%u\t%u", name, (unsigned)fno->fsize,
                     (unsigned)fat_mtime(fno->fdate, fno->ftime));
    } else {
        n = snprintf(entry_buf, sizeof(entry_buf), "F%s\t%u", name, (unsigned)fno->fsize);
    }
    if (n < 0 || n >= (int)sizeof(entry_buf)) {
        return -1;
    }
    return n + 1;
}

/*
  list a directory, from the request's offset entry on, as many
  entries as fit.  A GCS asks for one page after another, so the
  session keeps the directory open, and the next page goes on from
  where this one stopped rather than skipping from the start
 */
static void list_dir(struct ftp_session *s, struct ftp_txn *req, struct ftp_txn *reply, bool with_time)
{
    reply->offset = req->offset;
    if (!request_path(req, reply, path1)) {
        return;
    }
    FRESULT fr = FR_OK;
    unsigned index = 0;
    const bool carry_on = s->dir_open && s->dir_with_time == with_time &&
        req->offset == s->dir_next && strcmp(path1, s->dir_path) == 0;
    if (!carry_on) {
        session_close_dir(s);
        fr = f_opendir(&s->dir, path1);
        if (fr != FR_OK) {
            ftp_error_fr(reply, fr);
            return;
        }
        s->dir_open = true;
        s->dir_with_time = with_time;
        strcpy(s->dir_path, path1);
        s->dir_pending_len = 0;
        s->dir_next = 0;
    } else if (s->dir_pending_len > 0) {
        memcpy(reply->data, s->dir_pending, s->dir_pending_len);
        index = s->dir_pending_len;
        s->dir_pending_len = 0;
        s->dir_next++;
    }

    FILINFO fno;
    fno.lfname = lfn;
    fno.lfsize = sizeof(lfn);
    for (;;) {
        fr = f_readdir(&s->dir, &fno);
        if (fr != FR_OK || fno.fname[0] == 0) {
            break;
        }
        const char *name = lfn[0] ? lfn : fno.fname;
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
            continue;
        }
        const int n = gen_dir_entry(&fno, name, with_time);
        if (n < 0 || n > FTP_DATA_LEN) {
            // could never be sent; both the skipping and the packing
            // must leave it out, or the offsets drift apart
            continue;
        }
        if (s->dir_next < req->offset) {
            s->dir_next++;
            continue;
        }
        if (index + n > FTP_DATA_LEN) {
            // the first of the next page
            memcpy(s->dir_pending, entry_buf, n);
            s->dir_pending_len = n;
            break;
        }
        memcpy(&reply->data[index], entry_buf, n);
        index += n;
        s->dir_next++;
    }

    if (index == 0 || fr != FR_OK) {
        session_close_dir(s);
    }
    if (index == 0) {
        if (fr != FR_OK) {
            ftp_error_fr(reply, fr);
        } else {
            ftp_error(reply, MAV_FTP_ERR_EOF);
        }
        return;
    }
    reply->opcode = MAV_FTP_OPCODE_ACK;
    reply->size = index;
}

static void crc_init(void)
{
    uint32_t i;
    for (i = 0; i < 256; i++) {
        uint32_t c = i;
        unsigned k;
        for (k = 0; k < 8; k++) {
            c = (c & 1) ? 0xEDB88320UL ^ (c >> 1) : c >> 1;
        }
        crc_table[i] = c;
    }
}

/*
  the file's CRC32 as ArduPilot computes it: the usual table, from
  0, and not inverted at the end
 */
static FRESULT file_crc32(const char *path, uint32_t *crc)
{
    FIL fh;
    FRESULT fr = f_open(&fh, path, FA_READ | FA_OPEN_EXISTING);
    if (fr != FR_OK) {
        return fr;
    }
    uint32_t c = 0;
    for (;;) {
        UINT br = 0;
        fr = f_read(&fh, file_buf, sizeof(file_buf), &br);
        if (fr != FR_OK || br == 0) {
            break;
        }
        UINT i;
        for (i = 0; i < br; i++) {
            c = crc_table[(c ^ file_buf[i]) & 0xFF] ^ (c >> 8);
        }
    }
    f_close(&fh);
    *crc = c;
    return fr;
}

static FRESULT stat_path(const char *path, FILINFO *fno)
{
    fno->lfname = lfn;
    fno->lfsize = sizeof(lfn);
    return f_stat(path, fno);
}

/*
  handle one request on a session; true if the reply has been sent
  already (a burst read sends its own)
 */
static bool handle_request(struct ftp_session *s, struct ftp_txn *req, struct ftp_txn *reply)
{
    FRESULT fr;
    const uint32_t now = get_time_boot_ms();

    if (req->size > sizeof(req->data)) {
        ftp_error(reply, MAV_FTP_ERR_INVALIDDATASIZE);
        return false;
    }

    switch (req->opcode) {
    case MAV_FTP_OPCODE_NONE:
        reply->opcode = MAV_FTP_OPCODE_ACK;
        break;

    case MAV_FTP_OPCODE_TERMINATESESSION:
        fr = session_close(s);
        if (fr != FR_OK) {
            ftp_error_fr(reply, fr);
        } else {
            reply->opcode = MAV_FTP_OPCODE_ACK;
        }
        break;

    case MAV_FTP_OPCODE_LISTDIRECTORY:
        list_dir(s, req, reply, false);
        break;

    case FTP_OPCODE_LISTDIRECTORYWITHTIME:
        list_dir(s, req, reply, true);
        break;

    case MAV_FTP_OPCODE_OPENFILERO:
        if (s->open && now - s->last_send_ms > FTP_SESSION_TIMEOUT_MS) {
            // no activity for a while: the GCS missed our reply to
            // its open, and is opening again
            session_close(s);
        }
        if (s->open) {
            ftp_error(reply, MAV_FTP_ERR_FAIL);
            break;
        }
        if (!request_path(req, reply, path1)) {
            break;
        }
        fr = f_open(&s->fh, path1, FA_READ | FA_OPEN_EXISTING);
        if (fr != FR_OK) {
            ftp_error_fr(reply, fr);
            break;
        }
        s->open = true;
        s->writing = false;
        s->block_len = 0;
        reply->opcode = MAV_FTP_OPCODE_ACK;
        reply->size = 4;
        {
            const uint32_t size = f_size(&s->fh);
            reply->data[0] = size & 0xFF;
            reply->data[1] = (size >> 8) & 0xFF;
            reply->data[2] = (size >> 16) & 0xFF;
            reply->data[3] = size >> 24;
        }
        break;

    case MAV_FTP_OPCODE_READFILE: {
        if (!s->open) {
            ftp_error(reply, MAV_FTP_ERR_FILENOTFOUND);
            break;
        }
        if (s->writing) {
            ftp_error(reply, MAV_FTP_ERR_FAIL);
            break;
        }
        unsigned br = 0;
        fr = session_read(s, req->offset, reply->data, req->size < FTP_DATA_LEN ? req->size : FTP_DATA_LEN, &br);
        if (fr != FR_OK) {
            ftp_error_fr(reply, fr);
            break;
        }
        if (br == 0) {
            ftp_error(reply, MAV_FTP_ERR_EOF);
            break;
        }
        reply->opcode = MAV_FTP_OPCODE_ACK;
        reply->offset = req->offset;
        reply->size = br;
        break;
    }

    case MAV_FTP_OPCODE_BURSTREADFILE: {
        const unsigned max_read = (req->size == 0 || req->size > FTP_DATA_LEN) ? FTP_DATA_LEN : req->size;
        if (!s->open) {
            ftp_error(reply, MAV_FTP_ERR_FILENOTFOUND);
            break;
        }
        if (s->writing) {
            ftp_error(reply, MAV_FTP_ERR_FAIL);
            break;
        }
        reply->offset = req->offset;
        unsigned i;
        for (i = 0; i < FTP_BURST_PACKETS; i++) {
            unsigned br = 0;
            fr = session_read(s, reply->offset, reply->data, max_read, &br);
            if (fr != FR_OK) {
                reply->burst_complete = 1;
                ftp_error_fr(reply, fr);
                break;
            }
            if (br == 0) {
                reply->burst_complete = 1;
                ftp_error(reply, MAV_FTP_ERR_EOF);
                break;
            }
            reply->opcode = MAV_FTP_OPCODE_ACK;
            // the GCS asks for another burst to go on
            reply->burst_complete = (i == FTP_BURST_PACKETS - 1);
            reply->size = br;
            if (!session_push_reply(s, reply)) {
                // nothing is taking our replies; stop, and the GCS
                // will ask again.  That must start a new burst rather
                // than be answered with this unsent packet
                last_reply_resendable = false;
                return true;
            }
            reply->offset += br;
            reply->seq++;
        }
        // only an error (including the end of the file) is still to send
        return reply->opcode != MAV_FTP_OPCODE_NAK;
    }

    case MAV_FTP_OPCODE_ACK:
    case MAV_FTP_OPCODE_NAK:
        // not expected; ignore them, and with no reply sent, there's
        // none to send again
        last_reply_resendable = false;
        return true;

    case MAV_FTP_OPCODE_OPENFILEWO:
    case MAV_FTP_OPCODE_CREATEFILE:
        if (s->open) {
            ftp_error(reply, MAV_FTP_ERR_FAIL);
            break;
        }
        if (!request_path(req, reply, path1)) {
            break;
        }
        if (path_busy(path1)) {
            ftp_error(reply, MAV_FTP_ERR_FILEPROTECTED);
            break;
        }
        if (req->opcode == MAV_FTP_OPCODE_CREATEFILE) {
            fr = f_open(&s->fh, path1, FA_WRITE | FA_CREATE_ALWAYS);
        } else {
            // this FatFs refuses FA_OPEN_ALWAYS for a file that exists
            fr = f_open(&s->fh, path1, FA_WRITE | FA_OPEN_EXISTING);
            if (fr == FR_NO_FILE) {
                fr = f_open(&s->fh, path1, FA_WRITE | FA_CREATE_NEW);
            }
        }
        if (fr != FR_OK) {
            ftp_error_fr(reply, fr);
            break;
        }
        s->open = true;
        s->writing = true;
        reply->opcode = MAV_FTP_OPCODE_ACK;
        break;

    case MAV_FTP_OPCODE_WRITEFILE: {
        if (!s->open) {
            ftp_error(reply, MAV_FTP_ERR_FILENOTFOUND);
            break;
        }
        if (!s->writing) {
            ftp_error(reply, MAV_FTP_ERR_FAIL);
            break;
        }
        fr = f_lseek(&s->fh, req->offset);
        if (fr != FR_OK) {
            ftp_error_fr(reply, fr);
            break;
        }
        UINT bw = 0;
        fr = f_write(&s->fh, req->data, req->size, &bw);
        if (fr != FR_OK) {
            ftp_error_fr(reply, fr);
            break;
        }
        if (bw != req->size) {
            ftp_error_errno(reply, FTP_ENOSPC);
            break;
        }
        reply->opcode = MAV_FTP_OPCODE_ACK;
        reply->offset = req->offset;
        break;
    }

    case MAV_FTP_OPCODE_CREATEDIRECTORY:
        if (!request_path(req, reply, path1)) {
            break;
        }
        fr = f_mkdir(path1);
        if (fr != FR_OK) {
            ftp_error_fr(reply, fr);
            break;
        }
        reply->opcode = MAV_FTP_OPCODE_ACK;
        break;

    case MAV_FTP_OPCODE_REMOVEFILE:
    case MAV_FTP_OPCODE_REMOVEDIRECTORY: {
        if (!request_path(req, reply, path1)) {
            break;
        }
        FILINFO fno;
        fr = stat_path(path1, &fno);
        if (fr != FR_OK) {
            ftp_error_fr(reply, fr);
            break;
        }
        // f_unlink removes either, so check it's the one asked for
        const bool is_dir = (fno.fattrib & AM_DIR) != 0;
        if (req->opcode == MAV_FTP_OPCODE_REMOVEFILE && is_dir) {
            ftp_error_errno(reply, FTP_EISDIR);
            break;
        }
        if (req->opcode == MAV_FTP_OPCODE_REMOVEDIRECTORY && !is_dir) {
            ftp_error_errno(reply, FTP_ENOTDIR);
            break;
        }
        if (path_busy(path1)) {
            ftp_error(reply, MAV_FTP_ERR_FILEPROTECTED);
            break;
        }
        fr = f_unlink(path1);
        if (fr == FR_DENIED && is_dir) {
            ftp_error_errno(reply, FTP_ENOTEMPTY);
            break;
        }
        if (fr != FR_OK) {
            ftp_error_fr(reply, fr);
            break;
        }
        reply->opcode = MAV_FTP_OPCODE_ACK;
        break;
    }

    case MAV_FTP_OPCODE_TRUNCATEFILE: {
        if (!request_path(req, reply, path1)) {
            break;
        }
        if (path_busy(path1)) {
            ftp_error(reply, MAV_FTP_ERR_FILEPROTECTED);
            break;
        }
        FIL fh;
        fr = f_open(&fh, path1, FA_WRITE | FA_OPEN_EXISTING);
        if (fr != FR_OK) {
            ftp_error_fr(reply, fr);
            break;
        }
        if (req->offset > f_size(&fh)) {
            // seeking past the end would make it longer
            f_close(&fh);
            ftp_error_errno(reply, FTP_EINVAL);
            break;
        }
        fr = f_lseek(&fh, req->offset);
        if (fr == FR_OK) {
            fr = f_truncate(&fh);
        }
        const FRESULT fr2 = f_close(&fh);
        if (fr == FR_OK) {
            fr = fr2;
        }
        if (fr != FR_OK) {
            ftp_error_fr(reply, fr);
            break;
        }
        reply->opcode = MAV_FTP_OPCODE_ACK;
        break;
    }

    case MAV_FTP_OPCODE_RENAME: {
        const char *name1 = (const char *)req->data;
        const size_t len1 = strnlen(name1, sizeof(req->data) - 2);
        const char *name2 = (const char *)&req->data[len1 + 1];
        const size_t len2 = strnlen(name2, sizeof(req->data) - (len1 + 1));
        const bool counts_nul = req->size - (len1 + len2) == 2 &&
            req->data[sizeof(req->data) - 1] == 0;
        if (req->size == 0 || name1[len1] != 0 ||
            (len1 + len2 + 1 != req->size && !counts_nul)) {
            ftp_error(reply, MAV_FTP_ERR_INVALIDDATASIZE);
            break;
        }
        make_path(name1, len1, path1);
        make_path(name2, len2, path2);
        if (path_busy(path1) || path_busy(path2)) {
            ftp_error(reply, MAV_FTP_ERR_FILEPROTECTED);
            break;
        }
        fr = f_rename(path1, path2);
        if (fr != FR_OK) {
            ftp_error_fr(reply, fr);
            break;
        }
        reply->opcode = MAV_FTP_OPCODE_ACK;
        break;
    }

    case MAV_FTP_OPCODE_CALCFILECRC: {
        if (!request_path(req, reply, path1)) {
            break;
        }
        uint32_t crc = 0;
        fr = file_crc32(path1, &crc);
        if (fr != FR_OK) {
            ftp_error_fr(reply, fr);
            break;
        }
        memset(reply->data, 0, sizeof(reply->data));
        reply->data[0] = crc & 0xFF;
        reply->data[1] = (crc >> 8) & 0xFF;
        reply->data[2] = (crc >> 16) & 0xFF;
        reply->data[3] = crc >> 24;
        reply->size = 4;
        reply->opcode = MAV_FTP_OPCODE_ACK;
        break;
    }

    default:
        // say so specifically, so a GCS trying a newer opcode can
        // fall back to an older one
        ftp_error(reply, MAV_FTP_ERR_UNKNOWNCOMMAND);
        break;
    }
    return false;
}

static void setup_reply(const struct ftp_txn *req, struct ftp_txn *reply)
{
    memset(reply, 0, sizeof(*reply));
    reply->addr = req->addr;
    reply->sysid = req->sysid;
    reply->compid = req->compid;
    reply->req_opcode = req->opcode;
    reply->session = req->session;
    reply->seq = req->seq + 1;
    last_reply_resendable = true;
}

static void ftp_task(void *pvParameters)
{
    unsigned i;
    for (i = 0; i < FTP_MAX_SESSIONS; i++) {
        sessions[i].id = -1;
    }
    crc_init();

    static struct ftp_txn req;
    struct ftp_txn *reply = &last_reply;
    last_reply_resendable = false;

    for (;;) {
        if (xQueueReceive(request_queue, &req, 100 / portTICK_PERIOD_MS) != pdTRUE) {
            // close sessions left idle
            const uint32_t now = get_time_boot_ms();
            for (i = 0; i < FTP_MAX_SESSIONS; i++) {
                struct ftp_session *s = &sessions[i];
                if (s->last_send_ms != 0 && now - s->last_send_ms > FTP_SESSION_KILL_TIMEOUT_MS) {
                    session_close(s);
                }
            }
            continue;
        }

        if (req.opcode == MAV_FTP_OPCODE_RESETSESSION) {
            // close all of this GCS's sessions; always ACKed
            for (i = 0; i < FTP_MAX_SESSIONS; i++) {
                struct ftp_session *s = &sessions[i];
                if (same_gcs(&req.addr, req.sysid, req.compid, &s->addr, s->sysid, s->compid)) {
                    session_close(s);
                }
            }
            setup_reply(&req, reply);
            reply->opcode = MAV_FTP_OPCODE_ACK;
            push_reply(reply);
            continue;
        }

        struct ftp_session *session = NULL;
        for (i = 0; i < FTP_MAX_SESSIONS; i++) {
            struct ftp_session *s = &sessions[i];
            if (s->id == req.session &&
                same_gcs(&req.addr, req.sysid, req.compid, &s->addr, s->sysid, s->compid)) {
                session = s;
                break;
            }
        }
        if (session == NULL) {
            // take over a session with no file open, or else the one
            // idle longest, if it's idle.  Each GCS (and each run of
            // a command line tool) has its own port, so is a new GCS
            // here; one that only listed a directory shouldn't keep
            // the next one waiting
            const uint32_t now = get_time_boot_ms();
            session = NULL;
            for (i = 0; i < FTP_MAX_SESSIONS; i++) {
                struct ftp_session *s = &sessions[i];
                if (!s->open && (session == NULL || now - s->last_send_ms > now - session->last_send_ms)) {
                    session = s;
                }
            }
            if (session == NULL) {
                session = &sessions[0];
                for (i = 1; i < FTP_MAX_SESSIONS; i++) {
                    if (now - sessions[i].last_send_ms > now - session->last_send_ms) {
                        session = &sessions[i];
                    }
                }
            }
            if (session->open && session->last_send_ms != 0 &&
                now - session->last_send_ms < FTP_SESSION_TIMEOUT_MS) {
                setup_reply(&req, reply);
                ftp_error(reply, MAV_FTP_ERR_NOSESSIONSAVAILABLE);
                push_reply(reply);
                continue;
            }
            session_close(session);
            session->id = req.session;
            session->addr = req.addr;
            session->sysid = req.sysid;
            session->compid = req.compid;
        }

        // a request again whose reply we still have: send that again
        if (last_reply_resendable &&
            same_gcs(&req.addr, req.sysid, req.compid, &reply->addr, reply->sysid, reply->compid) &&
            req.session == reply->session && (uint16_t)(req.seq + 1) == reply->seq &&
            !(reply->opcode == MAV_FTP_OPCODE_NAK && reply->data[0] == MAV_FTP_ERR_NOSESSIONSAVAILABLE)) {
            session_push_reply(session, reply);
            continue;
        }

        setup_reply(&req, reply);
        if (!handle_request(session, &req, reply)) {
            session_push_reply(session, reply);
        }
    }
}
