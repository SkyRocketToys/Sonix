#pragma once

#define UBLOX_BASEDIR "/UBLOX"

#include <stdint.h>
#include <time.h>

void ublox_init();

// main for taking data from a network port
void ublox_assist_upload_task_process(void *pvParameters);

// set the time on the GPS module:
void ublox_send_ini_time_utc(const time_t epoch_time_seconds);

// main loop for handling sending assistence to ublox
void ublox_assist_task_process(void *pvParameters);

#ifdef __STDC__
void ublox_debug(const char  *format, ...);
#else
void ublox_debug(const char  *format, arg...);
#endif

struct mga_position {
    int32_t latitude;
    int32_t longitude;
    int32_t altitude_cm;
    uint32_t utc_time;
    int32_t time_base;
};

void cmd_ublox_status(unsigned argc, const char *argv[]);
void cmd_ublox_upload(void);
struct sock_buf;
void ublox_send_status_json(struct sock_buf *sock);
void handle_ublox_data(const uint8_t *data, const uint32_t data_len);
void ublox_set_position(const struct mga_position *pos);
void ublox_set_time(uint32_t time_utc);

#define UBLOX_MAX_MESSAGE_SIZE 200

typedef enum {
    CLASS_CFG = 0x06,
    CLASS_MGA = 0x13,
} ublox_class_t;

typedef enum {
    MSG_MGA_ANO = 0x20,
    MSG_MGA_INI_TIME_UTC = 0x40,
    MSG_MGA_INI_POS_LLH = 0x40,
    MSG_CFG_NAVX5 = 0x23,
} ublox_msg_t;
