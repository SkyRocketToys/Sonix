/*
  interface for MAVLink <-> WiFi bridge on UART2
 */
#pragma once

#include "mavlink_core.h"
#include "ublox.h"

void mavlink_wifi_socket_create(void);
void mavlink_show_stats(void);
void mavlink_set_debug(uint8_t debug_level);
int mavlink_set_flight_response(int index, int value);
void mavlink_set_sitl(bool enable);
void mavlink_fc_write(const uint8_t *buf, uint32_t len);
void mavlink_rc_write(const uint8_t *buf, uint32_t len);
int mavlink_fc_send(mavlink_message_t *msg);
bool toggle_recording(void);
bool take_snapshot(void);
void mavlink_param_set(const char *name, float value);

typedef enum {
    TELEM_MODE_NONE,
    TELEM_MODE_MAVLINK,
    TELEM_MODE_DEVCONSOLE,
    TELEM_MODE_INITIAL_BOOTLOADER_LOOP,
}  telem_mode_t;

void telem_main_request_mode(telem_mode_t mode);
telem_mode_t telem_main_get_mode();

// get data for app from MAVLink data
void telem_get_gps_data(uint8_t gps_data[10]);
void telem_get_uart_data(uint8_t uart_data[12]);
void telem_get_nav_data(uint8_t nav_data[4]);


// turn fake data on or off:
void mavlink_fakedata(uint8_t argc, const char *argv[]);
const mavlink_message_t *mavlink_get_message_by_msgid(uint32_t msgid, uint32_t *receive_ms);
const mavlink_message_t *mavlink_get_message_by_name(const char *name, uint32_t *receive_ms);
void mavlink_message_list_json(struct sock_buf *sock);
bool command_ack_get(uint16_t command, uint8_t *result, uint32_t *age_ms);

struct sock_buf;

bool mavlink_param_get(const char *name, float *value);
void mavlink_param_list_json(struct sock_buf *sock, const char *prefix, bool *first);
unsigned mavlink_fc_pkt_count(void);

typedef uint8_t gps_fix_type_t;
gps_fix_type_t get_gps_fix_type(void);

extern long long fc_unix_time;

// return size of files in bytes, -1 on error
int mw_get_filesize(const char *fname);

void get_fc_gps_lat_lon(struct mga_position *pos);

// get STM32 ID as string
const char *mavlink_get_stm32_id(void);

// access disable of sd save from app
extern int disable_sd_save;

// get vehicle armed state
bool get_vehicle_armed(void);
