/**
 * @file
 * MAVLink WiFi bridge to uart2
 * @author tridge
 */

#include <FreeRTOS.h>
#include <bsp.h>
#include <task.h>
#include <nonstdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <libmid_fwupgrade/fwupgrade.h>
#include "socket_ctrl.h"
#include "main_flow.h"
#include <libmid_fatfs/ff.h>
#include <uart/uart.h>
#include "dev_console.h"
#include "ublox.h"
#include "mavlink_wifi.h"
#include "mavlink_remote_log.h"
#include "bootloader_client.h"
#include "tx_upload.h"
#include <mavlink_core.h>
#include "telemetry_format.h"
#include "talloc.h"
#include "record/rec_schedule.h"
#include <web_server.h>
#include <libmid_nvram/snx_mid_nvram.h>
#include "util/print_vprintf.h"
#include "video_main.h"
#include <libmid_automount/automount.h>
#include "snapshot.h"

#define MAVLINK_LOCAL_PORT 14550
#define MAVLINK_DEST_PORT  14550

static xTaskHandle telem_process;
static uint8_t debug_level;
static bool mavlink_sitl_input = true; // we will disable this for production
static int sitl_listen_sock = -1;
static int sitl_sock = -1;
static char stm32_id[40];
static bool rc_ok;
static bool vehicle_armed;

#define QUEUE_SIZE 50*1024

// don't record for less than 1s, to prevent corrupted files
#define MIN_RECORDING_MS   1000

// timestamp when recording started
static uint32_t recording_start_ms;
static bool recording_end_pending;

// queue for data from uart2 ISR to mavlink thread
static QueueHandle_t isr_queue;

// queue for mavlink packets to send to FC
static QueueHandle_t mavlink_queue;

static struct {
    unsigned sent_pkts;
    unsigned recv_pkts;
    unsigned sent_bytes;
    unsigned recv_bytes;
} stats;

static struct telem_gps_data gps_data;
static struct telem_gps_data home_data;
static struct telem_attitude_data attitude_data;
static struct telem_uart_data uart_data;
static struct telem_nav_data nav_data;

static struct mga_position gps_pos;

static uint32_t last_heartbeat_ms;
static uint32_t last_attitude_ms;
static uint32_t last_autopilot_version_ms;

gps_fix_type_t gps_fix_type;
long long fc_unix_time; // flight controller's concept of the time

static const uint32_t telem_baudrates[] = { 625000, 115200 };
static uint8_t current_baudrate;
#define NUM_BAUDRATES (sizeof(telem_baudrates)/sizeof(telem_baudrates[0]))

static telem_mode_t telem_mode;
static telem_mode_t telem_mode_requested;

static bool mavlink_handle_msg(const mavlink_message_t *msg);

/*
  list of packet types received
 */
struct mavlink_packet {
    struct mavlink_packet *next;
    const char *name;
    mavlink_message_t msg;
    uint32_t receive_ms;
};


static struct mavlink_packet *mavlink_packets;

/*
  list of parameters received
 */
struct param_packet {
    struct param_packet *next;
    char name[17];
    float value;
};


// we have a param list per letter, to reduce list traversal cost
static struct param_packet *param_packets[26];
static uint32_t param_count;
static uint32_t param_expected_count;
static uint32_t param_last_value_sec;

/*
  list of command acks received
 */
struct mavlink_command_ack {
    struct mavlink_command_ack *next;
    uint32_t receive_ms;
    uint16_t command;
    uint8_t result;
};

static struct mavlink_command_ack *command_acks;

/*
  telemetry data sent over wifi to app is big-endian. ABI on Sonix is
  little-endian, so we need to swap bytes
 */
static int32_t swap_int32(int32_t x)
{
    return (int32_t)htonl(x);
}

static int16_t swap_int16(int16_t x)
{
    return (int16_t)htons(x);
}

#define DEG_TO_RAD      (M_PI / 180.0f)
#define RAD_TO_DEG      (180.0f / M_PI)

#define degrees(x) ((x) * RAD_TO_DEG)
#define radians(x) ((x) * DEG_TO_RAD)

/*
  handle uart2 ISR (for incoming bytes)
*/
static void uart2_isr_handler(int irq)
{
        uint8_t i;
        uint32_t nbytes;

        // get FIFO length
        nbytes = inl((unsigned*)(BSP_UART2_BASE_ADDRESS + FIFO_THD));
        nbytes = (nbytes >> 24) & 0x3F;
        if (nbytes > 30) {
            // FIFO is full
            for (i = 0 ; i < 32 ; i++) {
                inl((unsigned*)(BSP_UART2_BASE_ADDRESS + RS_DATA));
            }
            return;
        }

        /* Get the received mcu command from the UART */
        for (i=0; i<nbytes; i++) {
            uint8_t b = (uint8_t)inl((unsigned*)(BSP_UART2_BASE_ADDRESS + RS_DATA));
            xQueueSendFromISR(isr_queue, (void*)&b, 0);
        }

        uart2_clear_rx_interrupt();
}

/*
  write bytes to the flight controller, either over TCP SITL link, or
  over uart2
 */
void mavlink_fc_write(const uint8_t *buf, uint32_t len)
{
    if (mavlink_sitl_input && sitl_sock != -1) {
        write(sitl_sock, buf, len);
    } else {
        uart2_write(buf, len);
    }
}

/*
  write bytes to the RC controller, which is flight board when SITL is active
 */
void mavlink_rc_write(const uint8_t *buf, uint32_t len)
{
    if (sitl_sock != -1) {
        uart2_write(buf, len);
    }
}

/*
  process a UDP packet
 */
static void process_udp_packet(uint8_t *buf, ssize_t len)
{
    mavlink_fc_write(buf, len);
    stats.recv_pkts++;
    stats.recv_bytes += len;
}

int uart2_readbyte(uint8_t *b, const ssize_t timeout)
{
    if (!xQueueReceive(isr_queue, b, timeout)) {
	return 0;
    }
    return 1;
}


/*
  send a mavlink msg over WiFi
 */
static void send_mavlink_wifi(int fd, mavlink_message_t *msg)
{
    uint8_t buf[300];
    uint16_t len = mavlink_msg_to_send_buffer(buf, msg);
    if (len > 0) {
        struct sockaddr_in addr;
        memset(&addr, 0x0, sizeof(addr));

        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
        addr.sin_port = htons(MAVLINK_DEST_PORT);
        
        //console_printf("sending pkt of len %u\n", len);
        sendto(fd, buf, len, 0, (struct sockaddr*)&addr, sizeof(addr));

        stats.sent_pkts++;
        stats.sent_bytes += len;
    }

}

// Auto Pilot Modes enumeration
enum control_mode_t {
    STABILIZE =     0,  // manual airframe angle with manual throttle
    ACRO =          1,  // manual body-frame angular rate with manual throttle
    ALT_HOLD =      2,  // manual airframe angle with automatic throttle
    AUTO =          3,  // fully automatic waypoint control using mission commands
    GUIDED =        4,  // fully automatic fly to coordinate or fly at velocity/direction using GCS immediate commands
    LOITER =        5,  // automatic horizontal acceleration with automatic throttle
    RTL =           6,  // automatic return to launching point
    CIRCLE =        7,  // automatic circular flight with automatic throttle
    LAND =          9,  // automatic landing with horizontal position control
    DRIFT =        11,  // semi-automous position, yaw and throttle control
    SPORT =        13,  // manual earth-frame angular rate control with manual throttle
    FLIP =         14,  // automatically flip the vehicle on the roll axis
    AUTOTUNE =     15,  // automatically tune the vehicle's roll and pitch gains
    POSHOLD =      16,  // automatic position hold with manual override, with automatic throttle
    BRAKE =        17,  // full-brake using inertial/GPS system, no pilot input
    THROW =        18,  // throw to launch mode using inertial/GPS system, no pilot input
    AVOID_ADSB =   19,  // automatic avoidance of obstacles in the macro scale - e.g. full-sized aircraft
    GUIDED_NOGPS = 20,  // guided mode but only accepts attitude and altitude
};


/*
  send a request to set stream rates
 */
static void send_stream_rates_request(uint8_t rate)
{
    mavlink_msg_request_data_stream_send(MAVLINK_COMM_FC,
                                         MAVLINK_TARGET_SYSTEM_ID,
                                         0,
                                         MAV_DATA_STREAM_ALL,
                                         rate, 1);
}

/*
  send a mavlink COMMAND_LONG
 */
static void send_command_long(unsigned command,
                              float p1, float p2, float p3, float p4, float p5, float p6, float p7)
{
    mavlink_msg_command_long_send(MAVLINK_COMM_FC,
                                  MAVLINK_TARGET_SYSTEM_ID,
                                  0,
                                  command,
                                  0,
                                  p1, p2, p3, p4, p5, p6, p7);
}

/*
  ask for home position
 */
static void send_get_home_request(void)
{
    send_command_long(MAV_CMD_GET_HOME_POSITION,
                      0, 0, 0, 0, 0, 0, 0);
}

/*
  ask for home position
 */
static void send_set_flight_mode(enum control_mode_t mode)
{
    mavlink_msg_set_mode_send(MAVLINK_COMM_FC,
                              MAVLINK_TARGET_SYSTEM_ID,
                              MAV_MODE_FLAG_CUSTOM_MODE_ENABLED,
                              mode);
}

/*
  send a named int to flight controller
 */
static void send_named_int(const char *name, int32_t value)
{
    mavlink_msg_named_value_int_send(MAVLINK_COMM_FC, get_sys_seconds_boot()*1000, name, value);
    if (sitl_sock != -1) {
        // also send to real FC so TX can get the info
        mavlink_msg_named_value_int_send(MAVLINK_COMM_RC, get_sys_seconds_boot()*1000, name, value);
    }
}

/*
  periodic mavlink tasks - run on each HEARTBEAT from fc
 */
static void mavlink_periodic(void)
{
    static uint32_t last_periodic_ms;
    static uint32_t last_send_stream_ms;
    uint32_t now = get_time_boot_ms();
    if (now - last_periodic_ms < 250) {
        return;
    }
    last_periodic_ms = now;
    
    if (now - last_send_stream_ms > 15000 &&
        now - last_attitude_ms > 500) {
        send_stream_rates_request(4);
        last_send_stream_ms = now;
        if (debug_level > 0) {
            console_printf("sending stream request\n");
        }
    }

    if (schedrec_state() == 1 || (disable_sd_save && uart_data.is_recording_video)) {
        // keep LEDs blinking when recording video
        send_named_int("VNOTIFY", 1);
    }
    
    static uint32_t last_home_request_ms;
    
    if (home_data.latitude == 0 &&
        home_data.longitude == 0 &&
        now - last_home_request_ms > 10000) {
        last_home_request_ms = now;
        if (debug_level > 1) {
            console_printf("requesting home\n");
        }
        send_get_home_request();
    }

    static uint32_t last_param_request_ms;
    if (now - last_param_request_ms > 3000 &&
        (param_count == 0 ||
         (param_expected_count > param_count &&
          get_sys_seconds_boot() - param_last_value_sec > 20))) {
        last_param_request_ms = now;
        console_printf("requesting parameters param_count=%u param_expected_count=%u\n",
                       param_count, param_expected_count);
        mavlink_msg_param_request_list_send(MAVLINK_COMM_FC,
                                            MAVLINK_TARGET_SYSTEM_ID,
                                            0);
    }

    static uint32_t last_wifi_chan_ms;
    if (now - last_wifi_chan_ms > 30000) {
        last_wifi_chan_ms = now;
        int channel = 0;
        snx_nvram_integer_get("WIFI_DEV", "AP_CHANNEL_INFO", &channel);
        send_named_int("WIFICHAN", channel);        
    }

    static uint32_t last_capabilities_ms;
    if (now - last_capabilities_ms > 3000 &&
        (last_autopilot_version_ms == 0 || (now - last_capabilities_ms) > 30000)) {
        last_capabilities_ms = now;
        send_command_long(MAV_CMD_REQUEST_AUTOPILOT_CAPABILITIES,
                          1, 0, 0, 0, 0, 0, 0);
    }

    mavlink_remote_log_sync(false);
}

extern const char *mavlink_message_name(const mavlink_message_t *msg);

/*
  save a param value
 */
static void param_save_packet(const mavlink_message_t *msg)
{
    mavlink_param_value_t param_pkt;
    mavlink_msg_param_value_decode(msg, &param_pkt);
    if (param_pkt.param_id[0] < 'A' || param_pkt.param_id[0] > 'Z') {
        // invalid name
        return;
    }
    struct param_packet *p0 = param_packets[param_pkt.param_id[0] - 'A'];
    struct param_packet *p;
    for (p=p0; p; p=p->next) {
        if (strncmp(p->name, param_pkt.param_id, 16) == 0) {
            p->value = param_pkt.param_value;
            param_last_value_sec = get_sys_seconds_boot();
            if (param_pkt.param_count < 30000 &&
                param_pkt.param_count > 0 &&
                param_pkt.param_count > param_expected_count) {
                param_expected_count = param_pkt.param_count-1;
            }
            return;
        }
    }
    p = talloc_size(NULL, sizeof(*p));
    if (p) {
        strncpy(p->name, param_pkt.param_id, 16);
        p->name[16] = 0;
        p->value = param_pkt.param_value;
        p->next = p0;
        param_packets[param_pkt.param_id[0] - 'A'] = p;
        param_count++;
        param_last_value_sec = get_sys_seconds_boot();
        if (param_pkt.param_count < 30000 &&
            param_pkt.param_count > 0 &&
            param_pkt.param_count > param_expected_count) {
            param_expected_count = param_pkt.param_count-1;
        }
    }
}

/*
  get a parameter value
 */
bool mavlink_param_get(const char *name, float *value)
{
    if (name[0] < 'A' || name[0] > 'Z') {
        return false;
    }
    struct param_packet *p0 = param_packets[name[0] - 'A'];
    struct param_packet *p;
    for (p=p0; p; p=p->next) {
        if (strcmp(p->name, name) == 0) {
            *value = p->value;
            return true;
        }
    }
    return false;
}

/*
  list all parameters as json
 */
void mavlink_param_list_json(struct sock_buf *sock, const char *prefix, bool *first)
{
    uint8_t c;
    uint8_t plen = strlen(prefix);
    
    for (c=0; c<26; c++) {
        struct param_packet *p0 = param_packets[c];
        struct param_packet *p;
        for (p=p0; p; p=p->next) {
            if (strncmp(p->name, prefix, plen) != 0) {
                continue;
            }
            char *vstr = print_printf(sock, "%f ", p->value);
            if (vstr == NULL) {
                continue;
            }
            // ensure it is null terminated
            vstr[talloc_get_size(vstr)-1] = 0;
            if (vstr[strlen(vstr)-1] == '.') {
                // can't have trailing . in javascript float for json
                vstr[strlen(vstr)-1] = 0;
            }
            if (!*first) {
                sock_printf(sock, ",\r\n");
            }
            *first = false;
            sock_printf(sock, "{ \"name\" : \"%s\", \"value\" : %s }",
                        p->name, vstr);
            talloc_free(vstr);
        }
    }
}

/*
  save last instance of each packet type
 */
static void mavlink_save_packet(const mavlink_message_t *msg)
{
    if (msg->msgid == MAVLINK_MSG_ID_PARAM_VALUE) {
        param_save_packet(msg);
    }
    struct mavlink_packet *p;
    for (p=mavlink_packets; p; p=p->next) {
        if (p->msg.msgid == msg->msgid) {
            memcpy(&p->msg, msg, sizeof(mavlink_message_t));
            p->receive_ms = get_time_boot_ms();
            return;
        }
    }
    p = talloc_size(NULL, sizeof(*p));
    if (p == NULL) {
        return;
    }
    p->next = mavlink_packets;
    p->name = mavlink_message_name(msg);
    memcpy(&p->msg, msg, sizeof(mavlink_message_t));
    p->receive_ms = get_time_boot_ms();
    mavlink_packets = p;
}


/*
  save last instance of each COMMAND_ACK
 */
static void command_ack_save(const mavlink_command_ack_t *m)
{
    struct mavlink_command_ack *p;
    for (p=command_acks; p; p=p->next) {
        if (p->command == m->command) {
            p->result = m->result;
            p->receive_ms = get_time_boot_ms();
            return;
        }
    }
    p = talloc(NULL, struct mavlink_command_ack);
    if (p) {
        p->next = command_acks;
        p->command = m->command;
        p->result = m->result;
        p->receive_ms = get_time_boot_ms();
        command_acks = p;
    }
}

/*
  give last command ack result as json
 */
bool command_ack_get(uint16_t command, uint8_t *result, uint32_t *receive_ms)
{
    struct mavlink_command_ack *p;
    for (p=command_acks; p; p=p->next) {
        if (p->command == command) {
            *result = p->result;
            *receive_ms = p->receive_ms;
            return true;
        }
    }
    return false;
}

/*
  get last message of a specified type
 */
const mavlink_message_t *mavlink_get_message_by_msgid(uint32_t msgid, uint32_t *receive_ms)
{
    struct mavlink_packet *p;
    for (p=mavlink_packets; p; p=p->next) {
        if (p->msg.msgid == msgid) {
            *receive_ms = p->receive_ms;
            return &p->msg;
        }
    }
    return NULL;
}

/*
  get last message of a specified type
 */
const mavlink_message_t *mavlink_get_message_by_name(const char *name, uint32_t *receive_ms)
{
    struct mavlink_packet *p;
    for (p=mavlink_packets; p; p=p->next) {
        if (p->name && strcmp(name, p->name) == 0) {
            *receive_ms = p->receive_ms;
            return &p->msg;
        }
    }
    return NULL;
}

/*
  get list of available mavlink packets as JSON
 */
void mavlink_message_list_json(struct sock_buf *sock)
{
    sock_printf(sock, "[");
    bool first = true;
    struct mavlink_packet *p;
    for (p=mavlink_packets; p; p=p->next) {
        sock_printf(sock, "%s\"%s\"", first?"":", ", p->name);
        first = false;
    }
    sock_printf(sock, "]");
}

/*
 * handle an (as yet undecoded) mavlink message
 */
static bool mavlink_handle_msg(const mavlink_message_t *msg)
{
    if (debug_level > 2 && msg->msgid != MAVLINK_MSG_ID_REMOTE_LOG_DATA_BLOCK) {
        console_printf("mavlink msg %u\n", msg->msgid);
    }

    mavlink_save_packet(msg);
    mavlink_periodic();

    uint32_t now = get_time_boot_ms();
    
    switch(msg->msgid) {
    case MAVLINK_MSG_ID_REMOTE_LOG_DATA_BLOCK: {
	mavlink_remote_log_data_block_t decoded_message;
	mavlink_msg_remote_log_data_block_decode(msg, &decoded_message);
	mavlink_handle_remote_log_data_block(&decoded_message);
        return true;
    }

    /*
      translate telemetry data to json app format
    */
    case MAVLINK_MSG_ID_GPS_RAW_INT: {
	mavlink_gps_raw_int_t m;
	mavlink_msg_gps_raw_int_decode(msg, &m);
        gps_data.num_sats = m.satellites_visible;
        gps_fix_type = m.fix_type;
        if (m.fix_type > 0) {
            gps_pos.latitude = m.lat;
            gps_pos.longitude = m.lon;
            gps_pos.altitude_cm = m.alt/10;
        }
        break;
    }
    case MAVLINK_MSG_ID_HOME_POSITION: {
	mavlink_home_position_t m;
	mavlink_msg_home_position_decode(msg, &m);
        home_data.latitude = swap_int32(m.latitude);
        home_data.longitude = swap_int32(m.longitude);
        break;
    }
    case MAVLINK_MSG_ID_ATTITUDE: {
        last_attitude_ms = now;
	mavlink_attitude_t m;
	mavlink_msg_attitude_decode(msg, &m);
        attitude_data.roll = swap_int16(degrees(m.roll)*100);
        attitude_data.pitch = swap_int16(degrees(m.pitch)*100);
        attitude_data.yaw = swap_int16(degrees(m.yaw)*100);
        int16_t yaw = degrees(m.yaw);
        if (yaw < 0) {
            yaw += 360;
        }
        uart_data.mag_y = swap_int16(yaw);
        break;
    }
    case MAVLINK_MSG_ID_VFR_HUD: {
	mavlink_vfr_hud_t m;
	mavlink_msg_vfr_hud_decode(msg, &m);
        attitude_data.throttle = swap_int16(m.throttle);
        uart_data.altitude = swap_int16(m.alt*100);
        break;
    }
    case MAVLINK_MSG_ID_GLOBAL_POSITION_INT: {
	mavlink_global_position_int_t m;
	mavlink_msg_global_position_int_decode(msg, &m);
        uart_data.altitude = swap_int16(m.relative_alt/10); // in cm
        gps_data.latitude = swap_int32(m.lat);
        gps_data.longitude = swap_int32(m.lon);
        break;
    }
    case MAVLINK_MSG_ID_SYS_STATUS: {
	mavlink_sys_status_t m;
	mavlink_msg_sys_status_decode(msg, &m);
        uart_data.telemetry_power = m.voltage_battery/100; // in 0.1V units
        rc_ok = (m.onboard_control_sensors_health & MAV_SYS_STATUS_SENSOR_RC_RECEIVER) != 0;
        break;
    }
    case MAVLINK_MSG_ID_SYSTEM_TIME: {
	mavlink_system_time_t m;
	mavlink_msg_system_time_decode(msg, &m);
        fc_unix_time = m.time_unix_usec/1000000; // microseconds->seconds
        gps_pos.utc_time = fc_unix_time;
        gps_pos.time_base = get_sys_seconds_boot();
        /* console_printf("system_time_msg: us=%lu s=%lu\n", m.time_unix_usec, fc_unix_time); */
        break;
    }
    case MAVLINK_MSG_ID_HEARTBEAT: {
        if (last_heartbeat_ms == 0 ||
            now - last_heartbeat_ms > 10000) {
            console_printf("heartbeat ok\n");
        }
        last_heartbeat_ms = now;

	mavlink_heartbeat_t m;
	mavlink_msg_heartbeat_decode(msg, &m);
        bool armed = m.base_mode & MAV_MODE_FLAG_SAFETY_ARMED;
        if (m.autopilot == MAV_AUTOPILOT_ARDUPILOTMEGA) {
            switch (m.custom_mode) {
            case AUTO:
            case GUIDED:
            case LOITER:
            case POSHOLD:
            case RTL:
                uart_data.pos_hold_mode = 1;
                uart_data.takeoff_state = armed?2:0;
                break;
            case LAND:
                uart_data.pos_hold_mode = 0;
                uart_data.takeoff_state = armed?3:0;
                break;
            default:
                uart_data.pos_hold_mode = 0;
                uart_data.takeoff_state = armed?1:0;
                break;
            }
        }

        if (!armed && vehicle_armed && schedrec_state() == 1) {
            console_printf("disarmed: cycling video file\n");
            rec_sched_next_file();
        }
        vehicle_armed = armed;
        break;
    }

    case MAVLINK_MSG_ID_DATA16: {
        // data16 is used for ACKs for TX firmware upload
	mavlink_data16_t m;
	mavlink_msg_data16_decode(msg, &m);
        tx_upload_handle_data16(&m);
        break;
    }

    case MAVLINK_MSG_ID_COMMAND_ACK: {
	mavlink_command_ack_t m;
	mavlink_msg_command_ack_decode(msg, &m);
        command_ack_save(&m);
        break;
    }

    case MAVLINK_MSG_ID_NAMED_VALUE_INT: {
	mavlink_named_value_int_t m;
	mavlink_msg_named_value_int_decode(msg, &m);
        if (strncmp(m.name, "SNAPSHOT", 10) == 0) {
            if (!disable_sd_save) {
                take_snapshot();
            }
            send_named_int("VNOTIFY", 1);
        } else if (strncmp(m.name, "VIDEOTOG", 10) == 0) {
            if (toggle_recording()) {
                send_named_int("VNOTIFY", 1);
            }
        } else if (strncmp(m.name, "WIFIRESET", 9) == 0) {
            wifi_reset();
        }
        break;
    }

    case MAVLINK_MSG_ID_AUTOPILOT_VERSION: {
        last_autopilot_version_ms = now;
        break;
    }

    case MAVLINK_MSG_ID_STATUSTEXT: {
	mavlink_statustext_t m;
	mavlink_msg_statustext_decode(msg, &m);
        if (strncmp(m.text, "PX4v3 ", 6) == 0) {
            strncpy(stm32_id, m.text+6, sizeof(stm32_id)-1);
            stm32_id[sizeof(stm32_id)-1] = 0;
        }
        // sync log on any STATUSTEXT. This helps ensure we get
        // critical messages
        mavlink_remote_log_sync(true);
        break;
    }

    case MAVLINK_MSG_ID_RC_CHANNELS: {
	mavlink_rc_channels_t m;
        mavlink_msg_rc_channels_decode(msg, &m);
        if (rc_ok) {
            uart_data.rf_strength = m.chan9_raw;
        } else {
            uart_data.rf_strength = 0;
        }
        if (mavlink_sitl_input && sitl_sock != -1) {
            // always have RC input with SITL
            if (uart_data.rf_strength < 20) {
                uart_data.rf_strength = 20;
            }
        }
        break;
    }
        
    case MAVLINK_MSG_ID_CAMERA_FEEDBACK: {
      // Piggyback on GCS message to trigger camera from ardupilot mission
      if (!disable_sd_save) {
	take_snapshot();
      }
      send_named_int("VNOTIFY", 1);
        break;
    }
        
    default:
	break;
    }
    return false;
}

/* typedef int (*bootloader_dump_info_fprintf_t) (void * data, const char * format, ...); */
/* bootloader_dump_info_fprintf_t */
static void bootloader_dump_info()
{
    uint32_t rev;
    if  (bootloader_get_bl_rev(&rev) == -1) {
        print_msg_queue("[ibl] Failed to get bootloader revision\n");
    } else {
        print_msg_queue("[ibl] Bootloader revision: %u\n", rev);
    }

    uint32_t board_id;
    if  (bootloader_get_board_id(&board_id) == -1) {
        print_msg_queue("[ibl] Failed to get board id\n");
    } else {
        print_msg_queue("[ibl] Board ID: %u\n", board_id);
    }

    uint32_t board_rev;
    if  (bootloader_get_board_rev(&board_rev) == -1) {
        print_msg_queue("[ibl] Failed to get board rev\n");
    } else {
        print_msg_queue("[ibl] Board revision: %u\n", board_rev);
    }

    uint32_t fw_size;
    if  (bootloader_get_fw_size(&fw_size) == -1) {
        print_msg_queue("[ibl] Failed to get fw size\n");
    } else {

        print_msg_queue("[ibl] FW Size: %u\n", fw_size);
    }

    uint32_t chip_id;
    if  (bootloader_get_chip_id(&chip_id) == -1) {
        print_msg_queue("[ibl] Failed to get chip id\n");
    } else {
        print_msg_queue("[ibl] Chip ID: %u\n", chip_id);
    }

    uint32_t sn[3];
    if  (bootloader_get_sn(sn) == -1) {
        print_msg_queue("[ibl] Failed to get serial number\n");
    } else {
        print_msg_queue("[ibl] Serial Number: %x%x%x\n", sn[0],sn[1],sn[2]);
    }

    uint32_t vec_area[4];
    if  (bootloader_get_vec_area(vec_area) == -1) {
        print_msg_queue("[ibl] Failed to get vectors\n");
    } else {
        print_msg_queue("[ibl] Vectors: %x %x %x %x\n", vec_area[0],vec_area[1],vec_area[2],vec_area[3]) ;
    }

    uint32_t crc;
    if (bootloader_get_crc(&crc) == -1) {
        print_msg_queue("[ibl] Failed to get crc\n");
        return;
    }
    print_msg_queue("[ibl] STM32 Flash CRC: %08x\n", crc);

}

static void initial_bootloader_loop(void *pvParameters, int udp_sock)
{
    print_msg_queue("[ibl] Bootloader loop starting\n");

    pic_disableInterrupt(UART2_IRQ_NUM);

    const unsigned int old_baudrate = uart2_get_baudrate();
    if (!autobaud_bootloader()) {
        print_msg_queue("[ibl] Failed to autobaud bootloader\n");
        goto out;
    }

    if (bootloader_get_sync() == -1) {
        print_msg_queue("[ibl] Failed to get bootloader sync\n");
        goto out;
    }
    print_msg_queue("[ibl] Found bootloader\n");

    bootloader_dump_info();

    if (bootloader_boot() == -1) {
        print_msg_queue("[ibl] Failed to boot\n");
        goto out;
    }

out:
    print_msg_queue("[ibl] Setting baudrate to %u\n", old_baudrate);
    uart2_set_baudrate(old_baudrate);
    pic_enableInterrupt(UART2_IRQ_NUM);
    telem_main_request_mode(TELEM_MODE_MAVLINK);
}

/* start functions required for ublox */
void ublox_debug(const char  *format, ...)
{
    va_list ap;
    va_start(ap, format);
    console_vprintf(format, ap);
    //vprint_msg_queue(format, ap);
    va_end(ap);
}
/* end functions required for ublox */

/* start functions required for bootloader_client */
void bootloader_debug(const char  *format, ...)
{
    va_list ap;
    va_start(ap, format);
    console_vprintf(format, ap);
    va_end(ap);
}
/*
 * returns number of bytes written
 */
ssize_t bootloader_write(const uint8_t *bytes, const size_t bytecount, const bl_timeout_t timeout)
{
    /* timeout currently ignored */
    uart2_write(bytes, bytecount);
    return bytecount;
}

ssize_t bootloader_readbyte(uint8_t *byte, const bl_timeout_t timeout)
{
    uint32_t nbytes;

    TickType_t t_start = xTaskGetTickCount();
    
    while (true) {
        nbytes = inl((unsigned*)(BSP_UART2_BASE_ADDRESS + FIFO_THD));
        nbytes = (nbytes >> 24) & 0x3F;
        if (nbytes) {
            *byte = (uint8_t)inl((unsigned*)(BSP_UART2_BASE_ADDRESS + RS_DATA));
            return 1;
        }

        // treat timeout as ticks
        TickType_t t_now = xTaskGetTickCount();
        uint32_t tdiff = t_now - t_start;
        if (tdiff > timeout) {
            break;
        }
    }
    return 0;
    /* const ssize_t ret = uart2_readbyte(byte, timeout); */
    /* if (ret == 1) { */
    /*     console_printf("Got byte (0x%02x)\n", *byte); */
    /* } */
    /* return ret; */
}
/* end functions required for bootloader_client */

static void mavlink_main(void *pvParameters, int udp_sock);
static void telem_devconsole_main(void *pvParameters);

void telem_main_request_mode(telem_mode_t mode)
{
    telem_mode_requested = mode;
}

telem_mode_t telem_main_get_mode()
{
    return telem_mode;
}

static void check_video_recording_end()
{
    if (recording_end_pending && get_time_boot_ms() - recording_start_ms > MIN_RECORDING_MS) {
        console_printf("stopping recording (delayed)\n");
        user_diable_rec();
        uart_data.is_recording_video = false;
        recording_end_pending = false;
    }
}

/*
 * thread main for handling telemetry to and from flight controller
 */
static void telem_main(void *pvParameters)
{
    telem_mode = TELEM_MODE_NONE;
    telem_mode_requested = TELEM_MODE_MAVLINK;

    int udp_sock;

    isr_queue = xQueueCreate(QUEUE_SIZE, 1);
    if (isr_queue == NULL) {
        console_printf("Failed to create mavlink queue\n");
        goto end;
    }

    mavlink_queue = xQueueCreate(300, sizeof(void*));
    
    // setup handler for uart2
    uart2_init(625000); 

    uart2_register_irq(UART2_IRQ_NUM, &uart2_isr_handler);

    // setup data structures for mavlink_main:
    if ((udp_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        goto end;
    }

    struct sockaddr_in addr;
    memset(&addr, 0x0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(MAVLINK_LOCAL_PORT);

    if (bind(udp_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        goto end;
    }
    /* end setup data structures for mavlink_main: */

    /* TODO: just arbitrate access to serial port for separate
     * processes (split mavlink_main into a separate process)
     */
    while (1) {
        if (telem_mode_requested != telem_mode) {
            /* any transition logic calls go here e.g. serial flush */
            switch(telem_mode_requested) {
            case TELEM_MODE_INITIAL_BOOTLOADER_LOOP:
                break;
            case TELEM_MODE_MAVLINK:
                // and change to 625000
                uart2_set_baudrate(625000);
                break;
            default:
                break;
            }
            uint8_t b;
            uint32_t draincount = 0;
            while (uart2_readbyte(&b, 1) == 1) {
                draincount++;
            }
            console_printf("serial switching drained %lu bytes\n", (unsigned long)draincount);
            telem_mode = telem_mode_requested;
        }
        switch (telem_mode) {
        case TELEM_MODE_INITIAL_BOOTLOADER_LOOP:
            initial_bootloader_loop(pvParameters, udp_sock);
            break;
        case TELEM_MODE_MAVLINK:
            mavlink_main(pvParameters, udp_sock);
            break;
        case TELEM_MODE_DEVCONSOLE:
            telem_devconsole_main(pvParameters);
            break;
        case TELEM_MODE_NONE:
            break;
        default:
            console_printf("serial switching bad mode %u from %u\n", telem_mode_requested, telem_mode);
            telem_mode = TELEM_MODE_MAVLINK;
            break;
        }
    }

end:
    vTaskDelete(NULL);
}

/*
  auto-baud on main port
 */
static void mavlink_check_baudrate(void)
{
    static long long last_check;
    long long now = get_sys_seconds_boot();
    if (now - last_check > 3) {
        last_check = now;
        if (now - last_heartbeat_ms > 3000) {
            current_baudrate = (current_baudrate+1) % NUM_BAUDRATES;
            uart2_set_baudrate(telem_baudrates[current_baudrate]);
            if (debug_level > 0) {
                console_printf("trying baudrate %u\n", telem_baudrates[current_baudrate]);
            }
        }
    }
}


/*
  start a socket for SITL input
 */
static void start_sitl_sock(void)
{
    struct sockaddr_in addr;

    if ((sitl_listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        goto failed;
    }

    memset(&addr, 0x0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(MAVLINK_LOCAL_PORT);

    if (bind(sitl_listen_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        goto failed;
    }

    if (listen(sitl_listen_sock, 2) < 0) {
        goto failed;
    }

    console_printf("Opened SITL socket\n");
    return;

failed:
    if (sitl_listen_sock != -1) {
        close(sitl_listen_sock);
        sitl_listen_sock = -1;
    }
}

/*
  handle RC_CHANNELS from UART while in SITL, so we can use real TX
 */
static void handle_rc_input(mavlink_message_t *msg)
{
    switch (msg->msgid) {
    case MAVLINK_MSG_ID_RC_CHANNELS: {
	mavlink_rc_channels_t m;
        mavlink_msg_rc_channels_decode(msg, &m);
        //console_printf("chancount=%u\n", m.chancount);
        // we need to send override as system ID 255
        uint8_t saved_id = mavlink_system.sysid;
        mavlink_system.sysid = 255;
        mavlink_msg_rc_channels_override_send(MAVLINK_COMM_FC,
                                              MAVLINK_TARGET_SYSTEM_ID,
                                              0,
                                              m.chan1_raw,
                                              m.chan2_raw,
                                              m.chan3_raw,
                                              m.chan4_raw,
                                              m.chan5_raw,
                                              m.chan6_raw,
                                              m.chan7_raw,
                                              m.chan8_raw);
        mavlink_system.sysid = saved_id;        
        break;
    }
    case MAVLINK_MSG_ID_NAMED_VALUE_INT:
    case MAVLINK_MSG_ID_NAMED_VALUE_FLOAT:
        _mavlink_resend_uart(MAVLINK_COMM_FC, msg);
        mavlink_handle_msg(msg);
        break;
    default:
        break;
    }
}

/*
   thread main for mavlink processing
*/
static void mavlink_main(void *pvParameters, int udp_sock)
{
    console_printf("Starting mavlink_wifi main\n");

    while (telem_mode == telem_mode_requested) {
        fd_set fds;
        struct timeval tv;
        int numfd = udp_sock+1;
        
        FD_ZERO(&fds);
        FD_SET(udp_sock, &fds);

        if (mavlink_sitl_input && sitl_listen_sock != -1 && sitl_sock == -1) {
            // listen for a new SITL connection
            FD_SET(sitl_listen_sock, &fds);
            if (sitl_listen_sock >= numfd) {
                numfd = sitl_listen_sock+1;
            }
        }

        if (mavlink_sitl_input && sitl_sock != -1) {
            // listen for SITL data
            FD_SET(sitl_sock, &fds);
            if (sitl_sock >= numfd) {
                numfd = sitl_sock+1;
            }
        }
        
        tv.tv_sec = 0;
        tv.tv_usec = 10000;
        
        int res = select(numfd, &fds, NULL, NULL, &tv);
        if (res > 0 && FD_ISSET(udp_sock, &fds)) {
            // we have data pending
            uint8_t buf[300];
            ssize_t nread = read(udp_sock, buf, sizeof(buf));
            if (nread > 0) {
                process_udp_packet(buf, nread);
            }
        }

        /*
          check for new SITL connection
         */
        if (res > 0 && sitl_listen_sock != -1 && FD_ISSET(sitl_listen_sock, &fds)) {
            struct sockaddr_in addr;
            int len = sizeof(struct sockaddr_in);
            sitl_sock = accept(sitl_listen_sock, (struct sockaddr *)&addr, (socklen_t *)&len);
            if (sitl_sock != -1) {
                console_printf("New SITL connection\n");
            }
        }

        /*
          check for SITL data
         */
        if (res > 0 && sitl_sock != -1 && FD_ISSET(sitl_sock, &fds)) {
            uint8_t buf[200];
            ssize_t nread = read(sitl_sock, buf, sizeof(buf));
            uint8_t i;
            if (nread <= 0) {
                close(sitl_sock);
                sitl_sock = -1;
            } else {
                mavlink_message_t msg;
                mavlink_status_t status;
                for (i=0; i<nread; i++) {
                    if (mavlink_parse_char(MAVLINK_COMM_FC, buf[i], &msg, &status)) {
                        if (!mavlink_handle_msg(&msg)) {
                            // forward to WiFi connection as a udp packet:
                            send_mavlink_wifi(udp_sock, &msg);
                        }
                    }
                }
            }
        }

        uint8_t uart_comm_channel = MAVLINK_COMM_FC;
        if (mavlink_sitl_input && sitl_sock != -1) {
            // when SITL is active we parse the UART to FC, but only for RC_CHANNELS
            uart_comm_channel = MAVLINK_COMM_RC;
        }
        // check for MAVLink bytes from uart2
        uint8_t b;
            
        while (xQueueReceive(isr_queue, &b, 0)) {
            mavlink_message_t msg;
            mavlink_status_t status;
            uint8_t count = 0;
            if (mavlink_parse_char(uart_comm_channel, b, &msg, &status)) {
                if (uart_comm_channel == MAVLINK_COMM_RC) {
                    handle_rc_input(&msg);
                } else {
                    // possibly handle packet locally:
                    if (!mavlink_handle_msg(&msg)) {
                        // forward to WiFi connection as a udp packet:
                        send_mavlink_wifi(udp_sock, &msg);
                    }
                }
                if (count++ == 10) {
                    // ensure we don't starve the queue
                    break;
                }
            }
        }

        if (mavlink_sitl_input && sitl_listen_sock == -1) {
            start_sitl_sock();
        }

        /*
          send a queued mavlink message, if any
         */
        if (last_heartbeat_ms != 0 && get_time_boot_ms() - last_heartbeat_ms < 2000) {
            mavlink_message_t *msg = NULL;
            if (xQueueReceive(mavlink_queue, &msg, 0)) {
                _mavlink_resend_uart(MAVLINK_COMM_FC, msg);
                talloc_free(msg);
            }
        }
        
	mavlink_remote_log_periodic();
	mavlink_check_baudrate();
	tx_upload_periodic();
        check_video_recording_end();
    }
}

static void telem_devconsole_main(void *pvParameters)
{
    while (telem_mode_requested == TELEM_MODE_DEVCONSOLE) {
        /* all activity is undertaken by the dev_console thread */
        mdelay(100);
    }
}

/*
  amazingly, %f doesn't work in printf
 */
static const char *latlon_str(int32_t v)
{
    v = swap_int32(v);
    float d = v * 1.0e-7;
    static char ret[4][30];
    static uint8_t c;
    c = (c+1) % 4;
    snprintf(ret[c], sizeof(ret[c]), "%d.%u",
             (int)d, (unsigned)labs(((d-(int)d) * 1.0e7)));
    return ret[c];
}

uint8_t dummy_data = 0;

/*
  show packet stats
 */
void mavlink_show_stats(void)
{
    console_printf("Send: %u pkts %u bytes at %u\n", stats.sent_pkts, stats.sent_bytes,
                   uart2_get_baudrate());
    console_printf("Recv: %u pkts %u bytes\n", stats.recv_pkts, stats.recv_bytes);
    console_printf("gps: %s %s  home: %s %s  nsats: %u\n",
                   latlon_str(gps_data.latitude), latlon_str(gps_data.longitude),
                   latlon_str(home_data.latitude), latlon_str(home_data.longitude),
                   gps_data.num_sats);
    console_printf("throttle: %u  alt: %d  bat: %u tstate: %u rf: %u\n",
                   swap_int16(attitude_data.throttle), swap_int16(uart_data.altitude),
                   uart_data.telemetry_power, uart_data.takeoff_state,
                   uart_data.rf_strength);
    console_printf("fakedata: %u  mavsitl: %s\n", dummy_data, sitl_sock!=-1?"true":"false");
}

unsigned mavlink_fc_pkt_count(void)
{
    return stats.sent_pkts;
}

/**
* create sockets for MAVLink WiFi bridge
*/
void mavlink_wifi_socket_create(void)
{
    print_msg_queue("Creating telem_main task\n");
    if (pdPASS != xTaskCreate(telem_main, "telem_main", STACK_SIZE_16K, NULL, 40,&telem_process)) {
        print_msg_queue("Unable to create telem_main task\n");
    }
}

void mavlink_fakedata(uint8_t argc, const char *argv[])
{
    if (argc < 1) {
        console_printf("mavlink fakedata 0|1");
        return;
    }
    dummy_data = atoi(argv[0]);
}

// return GPS lat/lon if available
void get_fc_gps_lat_lon(struct mga_position *pos)
{
    *pos = gps_pos;
}

// get data for app from MAVLink data
void telem_get_gps_data(uint8_t data[10])
{
    static uint8_t send_gps_type;

    if (dummy_data) {
        gps_data.num_sats = 13;
        gps_data.gps_strength = 17;
        gps_data.latitude = swap_int32((int32_t)(-35.3312430 * 1e7));
        gps_data.longitude = swap_int32((int32_t)(149.132115 * 1e7));
    }

    // home position is sent with num_sats=254 and gps_strength=255
    home_data.num_sats = 254;
    home_data.gps_strength = 255;
    if (dummy_data) {
        home_data.latitude = swap_int32((int32_t)(-35.331350 * 1e7));
        home_data.longitude = swap_int32((int32_t)(149.131846 * 1e7));
    }
    
    attitude_data.m1 = 0xFF;
    attitude_data.m2 = 0xFF;
    if (dummy_data) {
        attitude_data.roll = swap_int16(3);
        attitude_data.pitch = swap_int16(2);
        attitude_data.throttle = swap_int16(35);
        attitude_data.yaw = swap_int16(4500);
    }
    
    switch (send_gps_type) {
    case 0:
        memcpy(data, &gps_data, sizeof(gps_data));
        break;

    case 1:
        // send GPS location if home not set
        if (home_data.latitude != 0 || home_data.longitude != 0) {
            memcpy(data, &home_data, sizeof(home_data));
        } else {
            memcpy(data, &gps_data, sizeof(gps_data));
        }
        break;

    case 2:
        memcpy(data, &attitude_data, sizeof(attitude_data));
        break;
    }

    if (debug_level > 1) {
        console_printf("request gps_data %u\n", send_gps_type);
    }
    send_gps_type = (send_gps_type + 1) % 3;
}

void telem_get_uart_data(uint8_t data[12])
{
    uart_data.drone_version = 12;
    uart_data.tx_version = 2;
    uart_data.head_free_mode = 0;
    uart_data.flags_modified_data = uart_data.flags_modified_data==1?2:1;
    uart_data.mag_cal_stage++;
    uart_data.rtl_mode = 0;

    memcpy(data, &uart_data, sizeof(uart_data));
    if (debug_level > 1) {
        console_printf("request uart_data\n");
    }
}

void telem_get_nav_data(uint8_t data[4])
{
    memcpy(data, &nav_data, sizeof(nav_data));
    if (debug_level > 1) {
        console_printf("request nav_data\n");
    }
}

void mavlink_set_debug(uint8_t level)
{
    debug_level = level;
}

void mavlink_set_sitl(bool enable)
{
    mavlink_sitl_input = enable;
    if (!mavlink_sitl_input) {
        if (sitl_sock != -1) {
            close(sitl_sock);
            sitl_sock = -1;
        }
        if (sitl_listen_sock != -1) {
            close(sitl_listen_sock);
            sitl_listen_sock = -1;
        }
    }
    console_printf("mavlink_sitl=%s\n", enable?"true":"false");
}

enum FlightCommand {
    flightControllerTakeOff = 1,
    flightControllerLand = 2,
    flightControllerCalibrateMagnetometer = 3,
    flightControllerEnterManual = 4, // Not implemented
    flightControllerEnterAuto = 5, // Not implemented
    flightControllerEnterHeadfreeMode = 6,
    flightControllerLeaveHeadfreeMode = 7,
    flightControllerEnterReturnToHomeMode = 8,
    flightControllerToggleVideoRecording = 9
};

/*
  toggle video recording to microSD
 */
bool toggle_recording(void)
{
    uart_data.is_recording_video = !uart_data.is_recording_video;
    if (!uart_data.is_recording_video) {
        if (get_time_boot_ms() - recording_start_ms > MIN_RECORDING_MS) {
            console_printf("stopping recording\n");
            user_diable_rec();
            recording_end_pending = false;
        } else {
            console_printf("stopping recording (pending)\n");
            recording_end_pending = true;
        }
    } else {
        recording_start_ms = get_time_boot_ms();
        recording_end_pending = false;
        console_printf("starting recording\n");
        if (!disable_sd_save) {
            schedrec_suspend_restart(1);
        }
    }
    return uart_data.is_recording_video != 0;
}

/*
  take a photo
 */
bool take_snapshot(void)
{
    int ret;
    
    int sdstate = get_sd_status();
    console_printf("SDCard status: %d\n", (int)sdstate);

    if (mf_snapshot_get_isfull() == 1) {
        console_printf("photo folder is full.\n");
        return false;
    }
	
    if (check_takepic_task() != 0) {
        console_printf("Continuous shooting is already running.\n");
        return false;
    }

    set_takepic_num(1);
    ret = mf_set_snapshot(1);
    console_printf("snapshot: ret=%d\n", ret);
    return true;
}

/*
  handle flight control commands
 */
int mavlink_set_flight_response(int index, int value)
{
    enum FlightCommand cmd = value;
    switch (cmd) {
    case flightControllerEnterReturnToHomeMode:
        console_printf("flight_command: RTL\n");
        send_set_flight_mode(RTL);
        return 0;

    case flightControllerToggleVideoRecording:
        toggle_recording();
        return 0;

    default:
        console_printf("unknown flight_command: %d %d\n", index, value);
        break;
    }
    return 1;
}

/*
  set a parameter
 */
void mavlink_param_set(const char *name, float value)
{
    console_printf("Setting parameter %s to %f\n", name, value);
    mavlink_msg_param_set_send(MAVLINK_COMM_FC, MAVLINK_TARGET_SYSTEM_ID, 0, name, value, 0);
}

/*
  queue a MAVLink packet to be sent to FC. Must be allocated with talloc
 */
int mavlink_fc_send(mavlink_message_t *msg)
{
    // we must steal to NULL so the memory doesn't get freed when the task exits
    talloc_steal(NULL, msg);
    if (xQueueSend(mavlink_queue, (const void *)&msg, 0) != pdTRUE) {
        talloc_free(msg);
        return -1;
    }
    return 0;
}

int mw_get_filesize(const char *fname)
{
	FILINFO finfo;
	char lfn[ _MAX_LFN + 1 ];
	char *path0 = (char *)fname;

	finfo.lfname = lfn;
	finfo.lfsize = _MAX_LFN + 1;
	if (f_stat( path0, & finfo) == FR_OK) {
		/* console_printf("(finfo.fsize)===%d byte \n", (finfo.fsize)); */
		return (finfo.fsize);
	} else {
                /* console_printf("read file size (%s) error\n", fname); */
		return (-1);
	}
}

// get GPS fix type
gps_fix_type_t get_gps_fix_type(void)
{
    return gps_fix_type;
}

// get STM32 ID as string
const char *mavlink_get_stm32_id(void)
{
    return stm32_id;
}

/*
  get the vehicle armed state
 */
bool get_vehicle_armed(void)
{
    uint32_t now = get_time_boot_ms();
    // if we have not heard from the flight controller for 20s then
    // assume disarmed
    if (vehicle_armed && now - last_heartbeat_ms < 20000) {
        return true;
    }
    return false;
}
