/*
  telemetry format packed structures. These are encoded as JSON for
  sending to the app
 */

#pragma once

#define PACKED __attribute__((__packed__))

/*
  telemetry data structures for app
 */
struct PACKED telem_gps_data {
    uint8_t num_sats;
    uint8_t gps_strength;
    int32_t latitude;
    int32_t longitude;
};

struct PACKED telem_attitude_data {
    uint8_t m1; // 0xFF
    uint8_t m2; // 0xFF
    int16_t roll;
    int16_t pitch;
    int16_t throttle;
    int16_t yaw;
};

struct PACKED telem_uart_data {
    uint8_t unknown1;
    uint8_t head_free_mode:1;
    uint8_t pos_hold_mode:1;
    uint8_t mag_cal_stage:2;
    uint8_t flags_modified_data:4;
    uint8_t buttons; // 2
    int16_t mag_y; // 3,4
    uint8_t drone_version; // 5
    uint8_t tx_version; // 6
    uint8_t telemetry_power:6;
    uint8_t rtl_mode:1;
    uint8_t spare_bit:1;
    int16_t altitude; // 8,9
    uint8_t takeoff_state:2;
    uint8_t is_recording_video:1;
    uint8_t rf_strength:5;
    uint8_t checksum; // 11
};

struct PACKED telem_nav_data {
    uint8_t unknown[4];
};
