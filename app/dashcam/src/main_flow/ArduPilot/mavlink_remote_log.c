/**
 * @file
 * MAVLink Remote Logging
 * @author pbarker
 */

#include <stdlib.h>
#include <libmid_fatfs/ff.h>
#include <sys_clock.h>
#include "dev_console.h"

#include "mavlink_remote_log.h"

static FIL log_fh;
static const uint8_t start_interval = 1; // seconds
static long long last_start_attempt = 0; // time (seconds)
static const uint8_t stop_interval = 1; // seconds
static long long last_stop_attempt = 0; // time (seconds)
static uint32_t last_message_received_ms;
static bool need_sync;

static long long last_lastlog_filepath_warning = 0;
static long long last_lastlog_logfh_warning = 0;

static bool setup_done = false;
static bool enabled = true;
static char log_filepath[300];
static uint32_t ack_counter;

static void mavlink_remote_log_setup()
{
    if (setup_done) {
	return;
    }
    setup_done = true;
}

static void ensure_log_basedir_exists()
{
    // ensure the logging directory exists:
    if (!exists(REMOTE_LOG_BASEDIR)) {
	if (!mkdir(REMOTE_LOG_BASEDIR)) {
            static long long last_report;
            long long now = get_sys_seconds_boot();
            if (now - last_report > 10) {
                console_printf("rl: failed to mkdir " REMOTE_LOG_BASEDIR "\n");
                last_report = now;
            }
	}
    }
}

/*
 * return a filename into which to write data for a new log
 */
static void new_log_filepath(char *path, const int len)
{
    ensure_log_basedir_exists();

    const char *lastlog_filepath = REMOTE_LOG_BASEDIR "/LASTLOG.TXT";

    uint16_t new_log_number = 1;

    /* attempt to read number of last log created */
    FIL lastlog_fptr;
    if (f_open(&lastlog_fptr, lastlog_filepath, FA_READ) == FR_OK) {
	char buf[16];
        memset(buf, '\0', sizeof(buf));
	unsigned int read_count;
	if (f_read(&lastlog_fptr, buf, sizeof(buf), &read_count) == FR_OK) {
	    const uint16_t last_log_number = atoi(buf);
	    new_log_number = last_log_number + 1;
	}
	f_close(&lastlog_fptr);
    }

    /* write out new last log created number */
    if (f_open(&lastlog_fptr, lastlog_filepath, FA_CREATE_ALWAYS|FA_WRITE) == FR_OK) {
	char buf[16];
	memset(buf, '\0', sizeof(buf));
	int used_bytes = snprintf(buf, sizeof(buf), "%u", new_log_number);
	uint32_t bytes_written;
	if (f_write(&lastlog_fptr, buf, used_bytes, &bytes_written) == FR_OK) {
	    if (bytes_written != used_bytes) {
		// whinge about short write
		console_printf("rl: short write\n");
	    }
	} else {
	    // whinge about failed write
	    console_printf("rl: failed write\n");
	}
        f_sync(&lastlog_fptr);
	f_close(&lastlog_fptr);
    } else {
	// moan about not being able to open log file
        const long long now = get_sys_seconds_boot();
        if (last_lastlog_filepath_warning == 0 ||
            now - last_lastlog_filepath_warning > 10) {
            console_printf("rl: failed to open (%s)\n", lastlog_filepath);
            last_lastlog_filepath_warning = now;
        }
    }

    memset(path, '\0', len);
    const char *fmt = REMOTE_LOG_BASEDIR "/%06u.BIN";
    snprintf(path, len, fmt, new_log_number);
}

/*
 * open a file to write blocks of remote dataflash log into
 * returns -1 on failure
 */
static int log_fh_open()
{
    new_log_filepath(log_filepath, sizeof(log_filepath));
    if (f_open(&log_fh, log_filepath, FA_CREATE_ALWAYS|FA_WRITE) != FR_OK) {
        const long long now = get_sys_seconds_boot();
        if (last_lastlog_logfh_warning == 0 ||
            now - last_lastlog_logfh_warning > 10) {
            console_printf("rl: failed to open (%s)\n", log_filepath);
            last_lastlog_logfh_warning = now;
        }
	return -1;
    }
    console_printf("rl: new dataflash log (%s)\n", log_filepath);
    return 0;
}

/*
 * close dataflash log file
 */
static void log_fh_close()
{
    unsigned int file_size = (unsigned)f_size(&log_fh);
    f_close(&log_fh);
    log_fh.fs = 0; // OS doesn't do this if it fails to close...

    // remove any very small log files.  This can happen when we
    // timeout after receiving a critical message
    if (file_size < 42000) {
        console_printf("rl: small log (%u bytes).  Removing.\n", (unsigned)file_size);
        f_unlink(log_filepath);
    }
}

/*
 * indicate whether log is open for writing
 */
static const int log_fh_is_open()
{
    return (log_fh.fs != 0);
}


/*
 * send an acknowledgement to the server that we have received a block
 * acks are also used to start/stop logging sessions
 */
static void send_ack(const uint32_t seqno)
{
    mavlink_msg_remote_log_block_status_send(MAVLINK_COMM_FC,
					     MAVLINK_TARGET_SYSTEM_ID,
					     MAV_COMP_ID_LOG,
					     seqno,
					     1);
    ack_counter++;
}

/*
 * attempt to stop logging - but not too often
 */
static void attempt_stop()
{
    const long long now = get_sys_seconds_boot();
    if (now - last_stop_attempt < stop_interval) {
	return;
    }
    last_stop_attempt = now;
    send_ack(MAV_REMOTE_LOG_DATA_BLOCK_STOP);
}

/*
 * attempt to start logging - but not too often
 */
static void attempt_start()
{
    const long long now = get_sys_seconds_boot();

    if (now - last_start_attempt < start_interval) {
	return;
    }
    //console_printf("rl: Sending remote-log start request\n");
    last_start_attempt = now;
    send_ack(MAV_REMOTE_LOG_DATA_BLOCK_START);
}

/*
 * write received data to disk
 */
void mavlink_handle_remote_log_data_block(mavlink_remote_log_data_block_t *msg)
{
    if (msg->target_system != MAVLINK_SYSTEM_ID ||
	msg->target_component != MAVLINK_COMPONENT_ID_REMOTE_LOG) {
	/* This shouldn't ever happen. */
	return;
    }

    if (!enabled) {
	// we could attempt_stop() here.
	return;
    }

    if (!log_fh_is_open()) {
	if (msg->seqno == 0) {
	    /* first block received - start a new block */
	    if (log_fh_open() == -1) {
		return;
	    }
	} else if (msg->seqno > 20) {
	    /* assume old logging session is in progress, useless from
	       our perspective.  Try to close that one off so we can
	       start another: */
	    attempt_stop();
            return;
	} else {
            // low-numbered block received; perhaps we missed the first one...
            return;
        }
    }

    last_message_received_ms = get_time_boot_ms();

    const uint32_t offset = MAVLINK_MSG_REMOTE_LOG_DATA_BLOCK_FIELD_DATA_LEN*msg->seqno;
    if (f_lseek(&log_fh, offset) != FR_OK) {
	/* seek failed?  Close this file and try opening another one. */
	console_printf("rl: seek (%u) failed\n", offset);
	log_fh_close();
	return;
    }
    unsigned int written;
    if (f_write(&log_fh, msg->data, MAVLINK_MSG_REMOTE_LOG_DATA_BLOCK_FIELD_DATA_LEN, &written) != FR_OK) {
	/* write failure; close this file and try opening another one */
	console_printf("rl: write failed\n");
	log_fh_close();
	return;
    }

    if (written != MAVLINK_MSG_REMOTE_LOG_DATA_BLOCK_FIELD_DATA_LEN) {
	/* short write?  Could have been interrrupted, drop block and
	   let retries fix it. */
	return;
    }

    need_sync = true;

    // ack this block
    send_ack(msg->seqno);

    // make sure we get data to disk, write at multiple of block size
    // of fs and remote logging
    if (offset % (MAVLINK_MSG_REMOTE_LOG_DATA_BLOCK_FIELD_DATA_LEN * 512) == 0) {
        f_sync(&log_fh);
        need_sync = false;
    }

}

/*
  flush to disk if we haven't written anything for 1 second. This is
  to ensure we flush on disarm
 */
void mavlink_remote_log_sync(bool force)
{
    if (log_fh_is_open() && need_sync &&
        (force || get_time_boot_ms() - last_message_received_ms > 1000)) {
        f_sync(&log_fh);        
        need_sync = false;
    }
}

void mavlink_remote_log_periodic()
{
    // filesystem isn't necessarily ready at boot, so we always call
    // this (and it does nothing once things are set up)
    mavlink_remote_log_setup();

    if (enabled) {
        if (log_fh_is_open()) {
            const uint32_t now = get_time_boot_ms();
            const uint32_t delta = now - last_message_received_ms;
            if (delta > 10000) {
                console_printf("rl: no messages received in %u seconds; closing output\n", (unsigned)(delta/1000));
                log_fh_close();
            }
        } else {
            attempt_start();
        }
    } else {
        if (log_fh_is_open()) {
            log_fh_close();
        }
    }
}


/*
  command-line support
*/
void cmd_rl_status(unsigned argc, const char *argv[])
{
    console_printf("rl: ack_counter %u\n", (unsigned)ack_counter);
    if (strlen(log_filepath)) {
	console_printf("rl: log_filepath: %s\n", log_filepath);
    }
    if (!enabled) {
	console_printf("rl: Disabled\n");
	return;
    }
    if (log_fh_is_open()) {
	console_printf("rl: Output file size: %u\n", (unsigned)f_size(&log_fh));
    } else {
	console_printf("rl: Output not opened\n");
    }
}

void cmd_rl_disable(unsigned argc, const char *argv[])
{
    if (!enabled ) {
	console_printf("rl: Already disabled\n");
	return;
    }
    if (log_fh_is_open()) {
	log_fh_close();
    }
    console_printf("rl: Disabled\n");
    enabled = false;
}

void cmd_rl_enable(unsigned argc, const char *argv[])
{
    if (enabled) {
	console_printf("rl: Already enabled\n");
	return;
    }
    console_printf("rl: Enabled\n");
    enabled = true;
}

void cmd_rl_help(unsigned argc, const char *argv[])
{
    console_printf("rl: Usage: rl help|enable|disable|status\n");
}
