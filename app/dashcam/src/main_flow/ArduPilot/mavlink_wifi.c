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
#include <lwip/netif.h>
#include "record/rec_schedule.h"
#include <web_server.h>
#include <libmid_nvram/snx_mid_nvram.h>
#include "util/print_vprintf.h"
#include "video_main.h"
#include <../sensor_cap/sensor_capability.h>
#include "../utility/utility.h"
#include "../errno.h"
#include <libmid_isp/snx_mid_isp.h>
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

/*
  GCSes that have sent us MAVLink.  Telemetry goes to each of them by
  unicast, which WiFi acknowledges and retries; broadcasts are sent
  once and lost if they don't arrive, which cost 5-10% of messages.
  HEARTBEAT is still broadcast, so a GCS that hasn't sent anything yet
  can find us (and sent to any GCS on another port, which the broadcast
  doesn't reach), and everything is broadcast while no GCS is known.
 */
#define WIFI_MAX_CLIENTS 4
#define WIFI_CLIENT_TIMEOUT_MS 5000
extern struct netif EMAC_if;  // lwIP's WiFi interface, for our own address
static bool wifi_unicast = true;
static struct wifi_client {
    struct sockaddr_in addr;
    uint32_t last_ms;
    bool active;
} wifi_clients[WIFI_MAX_CLIENTS];

static struct telem_gps_data gps_data;
static struct telem_gps_data home_data;
static struct telem_attitude_data attitude_data;
static struct telem_uart_data uart_data;
static struct telem_nav_data nav_data;

static struct mga_position gps_pos;

static uint32_t last_heartbeat_ms;
static uint8_t fc_sysid; // from the flight controller's HEARTBEAT; 0 until we hear one
static uint32_t last_sys_status_ms;
static uint32_t last_autopilot_version_ms;

/*
  AUTOPILOT_VERSION can be requested two ways.  Old flight controller
  firmware (e.g. ArduPilot 3.x) only understands
  MAV_CMD_REQUEST_AUTOPILOT_CAPABILITIES; ArduPilot 4.8 and later only
  understand MAV_CMD_REQUEST_MESSAGE.  We send both and stop sending
  whichever one the flight controller rejects.
 */
static bool request_message_unsupported;
static bool request_message_accepted;
static bool request_capabilities_unsupported;

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
static bool camera_udp_input(const uint8_t *buf, ssize_t len, const struct sockaddr_in *from);
static void wifi_client_seen(const struct sockaddr_in *from);
static void message_intervals_gcs_msg(const mavlink_message_t *msg, bool from_web);

static void process_udp_packet(uint8_t *buf, ssize_t len, const struct sockaddr_in *from)
{
    if (camera_udp_input(buf, len, from)) {
        return;
    }
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
  note a GCS we've had a UDP packet from
 */
static void wifi_client_seen(const struct sockaddr_in *from)
{
    const uint32_t now_ms = get_time_boot_ms();
    struct wifi_client *slot = NULL;
    unsigned i;
    for (i = 0; i < WIFI_MAX_CLIENTS; i++) {
        struct wifi_client *c = &wifi_clients[i];
        if (c->active && c->addr.sin_addr.s_addr == from->sin_addr.s_addr &&
            c->addr.sin_port == from->sin_port) {
            c->last_ms = now_ms;
            return;
        }
    }
    // a new one: not ourselves, should we hear our own broadcasts
    if (from->sin_addr.s_addr == EMAC_if.ip_addr.addr) {
        return;
    }
    // take a free or timed-out slot, or else the one heard from longest ago
    for (i = 0; i < WIFI_MAX_CLIENTS; i++) {
        struct wifi_client *c = &wifi_clients[i];
        if (!c->active || now_ms - c->last_ms > WIFI_CLIENT_TIMEOUT_MS) {
            slot = c;
            break;
        }
        if (slot == NULL || now_ms - c->last_ms > now_ms - slot->last_ms) {
            slot = c;
        }
    }
    slot->addr = *from;
    slot->last_ms = now_ms;
    slot->active = true;
    char ip[16];
    console_printf("mavlink: GCS at %s:%u\n", inet_ntoa_r(from->sin_addr, ip, sizeof(ip)),
                   ntohs(from->sin_port));
}

static void sendto_wifi(int fd, const uint8_t *buf, uint16_t len, const struct sockaddr_in *addr)
{
    sendto(fd, buf, len, 0, (const struct sockaddr*)addr, sizeof(*addr));
}

/*
  send a mavlink msg over WiFi
 */
static void send_mavlink_wifi(int fd, mavlink_message_t *msg)
{
    uint8_t buf[300];
    uint16_t len = mavlink_msg_to_send_buffer(buf, msg);
    if (len == 0) {
        return;
    }
    // counted once per message, however many GCSes it goes to
    stats.sent_pkts++;
    stats.sent_bytes += len;
    const bool heartbeat = msg->msgid == MAVLINK_MSG_ID_HEARTBEAT;
    bool unicast_sent = false;
    if (wifi_unicast) {
        const uint32_t now_ms = get_time_boot_ms();
        unsigned i;
        for (i = 0; i < WIFI_MAX_CLIENTS; i++) {
            struct wifi_client *c = &wifi_clients[i];
            if (!c->active || now_ms - c->last_ms > WIFI_CLIENT_TIMEOUT_MS) {
                continue;
            }
            if (heartbeat) {
                // the broadcast reaches those on our port; the rest
                // need their own copy
                if (c->addr.sin_port != htons(MAVLINK_DEST_PORT)) {
                    sendto_wifi(fd, buf, len, &c->addr);
                }
                continue;
            }
            sendto_wifi(fd, buf, len, &c->addr);
            unicast_sent = true;
        }
    }
    if (!unicast_sent) {
        struct sockaddr_in addr;
        memset(&addr, 0x0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
        addr.sin_port = htons(MAVLINK_DEST_PORT);
        sendto_wifi(fd, buf, len, &addr);
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
  A MAVLink camera component on WiFi, so ground stations can find the
  video and use the camera.

  QGroundControl wants a HEARTBEAT from a camera component of the
  vehicle's system, then CAMERA_INFORMATION, then
  VIDEO_STREAM_INFORMATION; it uses the last two whether or not it
  asked for them, so we broadcast all three once a second.  We also
  answer the camera commands the GCS sends us: still capture, video
  recording, and requests for the camera messages.

  Our own broadcasts are never passed on to the flight controller, so
  it doesn't learn about the camera component and route its traffic
  onto other links.  Without that route it would act on messages for
  the camera itself, so packets holding nothing but messages for the
  camera aren't passed on either; everything else from the GCS goes
  to the flight controller untouched, and we only look at a copy.
 */
#define CAMERA_COMPONENT_ID MAV_COMP_ID_CAMERA
#define CAMERA_RTSP_URI "rtsp://192.168.99.1:554/" LIVE_PREVIEW_STREAM_NAME

/*
  The camera definition file (APWeb's files/data/camera.xml) lists our
  settings.  Ground stations cache it by vendor, model and version, so
  bump the version whenever the file changes.
 */
#define CAMERA_DEFINITION_URI "http://192.168.99.1/data/camera.xml"
#define CAMERA_DEFINITION_VERSION 3

static int camera_udp_sock = -1;
static mavlink_status_t camera_status; // our own sequence numbers
static int32_t camera_image_count;     // photos taken since boot

/*
  Photos asked for while the previous one is still being saved are
  queued rather than refused.  A photo is reported as captured once
  the snapshot code has written it to the card, or failed to.
 */
#define CAMERA_MAX_QUEUED_PHOTOS 3
#define CAMERA_PHOTO_TIMEOUT_MS 10000
static uint8_t camera_photos_queued;
// the last photo queued with a capture sequence number (param4)
static uint32_t camera_capture_seq;
static uint8_t camera_capture_sysid, camera_capture_compid;
static uint32_t camera_capture_ms;
static bool camera_photo_in_progress;
static uint32_t camera_photo_start_ms;
static unsigned camera_photo_saved_before, camera_photo_failed_before;
static bool camera_queue_blocked;      // waiting on a snapshot started elsewhere
static uint32_t camera_queue_blocked_ms;
static float camera_free_mib;          // refreshed every few seconds

// a video started from here, the phone app or the transmitter
static bool camera_video_on;
static uint32_t camera_video_start_ms;
static uint32_t camera_own_packets;    // our broadcasts heard back, and dropped

// where the vehicle is, for CAMERA_IMAGE_CAPTURED
static mavlink_global_position_int_t camera_position;

static void camera_send(mavlink_message_t *msg)
{
    if (camera_udp_sock != -1) {
        send_mavlink_wifi(camera_udp_sock, msg);
    }
}

static void camera_send_heartbeat(void)
{
    mavlink_message_t msg;
    mavlink_msg_heartbeat_pack_status(fc_sysid, CAMERA_COMPONENT_ID, &camera_status, &msg,
                                      MAV_TYPE_CAMERA, MAV_AUTOPILOT_INVALID,
                                      0, 0, MAV_STATE_ACTIVE);
    camera_send(&msg);
}

static void camera_preview_params(unsigned *width, unsigned *height, unsigned *fps)
{
    if (mf_video_resmap_get_preview_params(width, height, fps, NULL, NULL) < 0) {
        *width = *height = *fps = 0;
    }
}

static void camera_send_information(void)
{
    unsigned width, height, fps;
    camera_preview_params(&width, &height, &fps);

    mavlink_camera_information_t info;
    memset(&info, 0, sizeof(info));
    info.time_boot_ms = get_time_boot_ms();
    info.focal_length = NAN;
    info.sensor_size_h = NAN;
    info.sensor_size_v = NAN;
    info.flags = CAMERA_CAP_FLAGS_CAPTURE_VIDEO |
                 CAMERA_CAP_FLAGS_CAPTURE_IMAGE |
                 CAMERA_CAP_FLAGS_HAS_VIDEO_STREAM;
    info.resolution_h = width;
    info.resolution_v = height;
    strncpy((char *)info.vendor_name, "SkyRocket", sizeof(info.vendor_name));
    strncpy((char *)info.model_name, "SkyViper", sizeof(info.model_name));
    info.cam_definition_version = CAMERA_DEFINITION_VERSION;
    strncpy(info.cam_definition_uri, CAMERA_DEFINITION_URI, sizeof(info.cam_definition_uri));

    mavlink_message_t msg;
    mavlink_msg_camera_information_encode_status(fc_sysid, CAMERA_COMPONENT_ID, &camera_status,
                                                 &msg, &info);
    camera_send(&msg);
}

static void camera_send_stream_information(void)
{
    unsigned width, height, fps;
    camera_preview_params(&width, &height, &fps);

    // QGC ignores later copies for a stream it already knows, so the
    // URI must not change
    mavlink_message_t msg;
    mavlink_msg_video_stream_information_pack_status(fc_sysid, CAMERA_COMPONENT_ID, &camera_status, &msg,
                                                     1, 1, // stream 1 of 1
                                                     VIDEO_STREAM_TYPE_RTSP,
                                                     VIDEO_STREAM_STATUS_FLAGS_RUNNING,
                                                     fps, width, height,
                                                     0, 0, 0, // bitrate, rotation, hfov unknown
                                                     "SkyViper", CAMERA_RTSP_URI,
                                                     VIDEO_STREAM_ENCODING_H264,
                                                     0, // a MAVLink camera, not one attached to the autopilot
                                                     0); // vfov unknown
    camera_send(&msg);
}

/*
  are we recording video?  The phone app starts and stops recording
  without going through toggle_recording(), so ask the recorder
 */
static bool camera_is_recording(void)
{
    if (recording_end_pending) {
        return false; // stopping, once it has run for MIN_RECORDING_MS
    }
    return schedrec_state() == 1 || (disable_sd_save && uart_data.is_recording_video);
}

static void camera_update_video_state(void)
{
    const bool on = camera_is_recording();
    if (on && !camera_video_on) {
        camera_video_start_ms = uart_data.is_recording_video ? recording_start_ms : get_time_boot_ms();
    }
    camera_video_on = on;
}

static void camera_send_capture_status(void)
{
    camera_update_video_state();

    mavlink_camera_capture_status_t status;
    memset(&status, 0, sizeof(status));
    status.time_boot_ms = get_time_boot_ms();
    status.image_status = (camera_photo_in_progress || camera_photos_queued > 0) ? 1 : 0;
    status.video_status = camera_video_on ? 1 : 0;
    if (camera_video_on) {
        status.recording_time_ms = get_time_boot_ms() - camera_video_start_ms;
    }
    status.available_capacity = camera_free_mib;
    status.image_count = camera_image_count;

    mavlink_message_t msg;
    mavlink_msg_camera_capture_status_encode_status(fc_sysid, CAMERA_COMPONENT_ID, &camera_status,
                                                    &msg, &status);
    camera_send(&msg);
}

// fills in storage, and remembers the free space for CAMERA_CAPTURE_STATUS
static void camera_read_storage(mavlink_storage_information_t *s)
{
    mavlink_storage_information_t storage;
    memset(&storage, 0, sizeof(storage));
    storage.time_boot_ms = get_time_boot_ms();
    storage.storage_id = 1;
    storage.storage_count = 1;
    storage.type = STORAGE_TYPE_MICROSD;
    strncpy(storage.name, "microSD", sizeof(storage.name));
    storage.read_speed = NAN;
    storage.write_speed = NAN;

    FATFS *fs = NULL;
    DWORD free_clusters = 0;
    if (f_getfree("0:", &free_clusters, &fs) == FR_OK && fs != NULL) {
        const float mib_per_cluster = fs->csize * 512.0f / (1024 * 1024);
        storage.status = STORAGE_STATUS_READY;
        storage.total_capacity = (fs->n_fatent - 2) * mib_per_cluster;
        storage.available_capacity = free_clusters * mib_per_cluster;
        storage.used_capacity = storage.total_capacity - storage.available_capacity;
        camera_free_mib = storage.available_capacity;
    } else {
        storage.status = STORAGE_STATUS_EMPTY;
        camera_free_mib = 0;
    }
    *s = storage;
}

static void camera_send_storage_information(void)
{
    mavlink_storage_information_t storage;
    camera_read_storage(&storage);

    mavlink_message_t msg;
    mavlink_msg_storage_information_encode_status(fc_sysid, CAMERA_COMPONENT_ID, &camera_status,
                                                  &msg, &storage);
    camera_send(&msg);
}

static void camera_send_image_captured(int32_t index, bool success)
{
    mavlink_camera_image_captured_t captured;
    memset(&captured, 0, sizeof(captured));
    captured.time_boot_ms = get_time_boot_ms();
    captured.time_utc = (uint64_t)fc_unix_time * 1000000ULL;
    captured.lat = camera_position.lat;
    captured.lon = camera_position.lon;
    captured.alt = camera_position.alt;
    captured.relative_alt = camera_position.relative_alt;
    captured.q[0] = NAN; // orientation unknown
    captured.image_index = index;
    captured.capture_result = success ? 1 : 0;

    mavlink_message_t msg;
    mavlink_msg_camera_image_captured_encode_status(fc_sysid, CAMERA_COMPONENT_ID, &camera_status,
                                                    &msg, &captured);
    camera_send(&msg);
}

static void camera_send_ack(const mavlink_message_t *request, uint16_t command, uint8_t result)
{
    mavlink_message_t msg;
    mavlink_msg_command_ack_pack_status(fc_sysid, CAMERA_COMPONENT_ID, &camera_status, &msg,
                                        command, result,
                                        0, 0, // progress, result_param2
                                        request->sysid, request->compid);
    camera_send(&msg);
}

/*
  Camera settings, by the names used in the camera definition file.
  WDR is changed the same way as from the phone app, so it's saved and
  the two stay in step.
 */
static int camera_get_wdr(int *value)
{
    if (snx_nvram_integer_get(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_VIDEO_WDR, value) != NVRAM_SUCCESS) {
        *value = 1; // factory setting
    }
    return OK;
}

/*
  The image filters aren't saved: they go back to the factory tuning
  (the defaults here) when the camera restarts.  Ranges are what was
  measured on the camera: the library accepts values outside them, but
  brightness, contrast and saturation stop changing at the top, and
  negative contrast turns the picture grey.
 */
static const struct camera_param {
    const char *name;      // at most 15 characters, for PARAM_EXT
    uint8_t type;          // MAV_PARAM_EXT_TYPE_UINT8 (switches) or _INT32
    int32_t min, max;      // as in the camera definition file
    int32_t default_value; // restored by RESET_CAMERA_SETTINGS
    int (*get)(int *value);
    int (*set)(int value);
} camera_params[] = {
    { "CAM_WDR",        MAV_PARAM_EXT_TYPE_UINT8, 0, 1,   1,  camera_get_wdr,                app_uti_set_video_wdr },
    { "CAM_BRIGHTNESS", MAV_PARAM_EXT_TYPE_INT32, 0, 127, 64, snx_isp_filter_brightness_get, snx_isp_filter_brightness_set },
    { "CAM_CONTRAST",   MAV_PARAM_EXT_TYPE_INT32, 0, 64,  32, snx_isp_filter_contrast_get,   snx_isp_filter_contrast_set },
    { "CAM_SATURATION", MAV_PARAM_EXT_TYPE_INT32, 0, 127, 64, snx_isp_filter_saturation_get, snx_isp_filter_saturation_set },
};
#define CAMERA_NUM_PARAMS (sizeof(camera_params)/sizeof(camera_params[0]))

static int32_t camera_param_get(const struct camera_param *p)
{
    int value = 0;
    if (p->get(&value) != OK) {
        console_printf("camera: can't read %s\n", p->name);
        return 0;
    }
    return value;
}

static const struct camera_param *camera_param_find(const char *id, int16_t index)
{
    if (index >= 0) {
        return (unsigned)index < CAMERA_NUM_PARAMS ? &camera_params[index] : NULL;
    }
    unsigned i;
    for (i = 0; i < CAMERA_NUM_PARAMS; i++) {
        // param_id is not NUL-terminated when all 16 characters are used
        if (strncmp(id, camera_params[i].name, 16) == 0) {
            return &camera_params[i];
        }
    }
    return NULL;
}

// PARAM_EXT values are little-endian at the start of the field
static void camera_param_encode(char *buf, const struct camera_param *p, int32_t value)
{
    memset(buf, 0, MAVLINK_MSG_PARAM_EXT_VALUE_FIELD_PARAM_VALUE_LEN);
    if (p->type == MAV_PARAM_EXT_TYPE_UINT8) {
        buf[0] = (uint8_t)value;
    } else {
        memcpy(buf, &value, sizeof(value));
    }
}

static int32_t camera_param_decode(const char *buf, const struct camera_param *p)
{
    if (p->type == MAV_PARAM_EXT_TYPE_UINT8) {
        return (uint8_t)buf[0];
    }
    int32_t value;
    memcpy(&value, buf, sizeof(value));
    return value;
}

static void camera_send_param(const struct camera_param *p)
{
    char value[MAVLINK_MSG_PARAM_EXT_VALUE_FIELD_PARAM_VALUE_LEN];
    camera_param_encode(value, p, camera_param_get(p));
    mavlink_message_t msg;
    mavlink_msg_param_ext_value_pack_status(fc_sysid, CAMERA_COMPONENT_ID, &camera_status, &msg,
                                            p->name, value, p->type,
                                            CAMERA_NUM_PARAMS, p - camera_params);
    camera_send(&msg);
}

/*
  PARAM_EXT_SET: apply it, and ACK with the value now in force, which
  QGC shows in its settings panel
 */
static void camera_handle_param_set(const mavlink_message_t *msg)
{
    mavlink_param_ext_set_t set;
    mavlink_msg_param_ext_set_decode(msg, &set);
    if (set.target_system != fc_sysid || set.target_component != CAMERA_COMPONENT_ID) {
        return;
    }
    const struct camera_param *p = camera_param_find(set.param_id, -1);
    if (p == NULL) {
        return; // not one of ours; leave the GCS to time out
    }

    uint8_t result;
    const int32_t v = camera_param_decode(set.param_value, p);
    if (set.param_type != p->type || v < p->min || v > p->max) {
        result = PARAM_ACK_VALUE_UNSUPPORTED;
    } else if (p->set(v) != OK) {
        result = PARAM_ACK_FAILED;
    } else {
        result = PARAM_ACK_ACCEPTED;
    }

    char value[MAVLINK_MSG_PARAM_EXT_ACK_FIELD_PARAM_VALUE_LEN];
    camera_param_encode(value, p, camera_param_get(p));
    mavlink_message_t ack;
    mavlink_msg_param_ext_ack_pack_status(fc_sysid, CAMERA_COMPONENT_ID, &camera_status, &ack,
                                          p->name, value, p->type, result);
    camera_send(&ack);
}

static void camera_handle_param_read(const mavlink_message_t *msg)
{
    mavlink_param_ext_request_read_t req;
    mavlink_msg_param_ext_request_read_decode(msg, &req);
    if (req.target_system != fc_sysid || req.target_component != CAMERA_COMPONENT_ID) {
        return;
    }
    const struct camera_param *p = camera_param_find(req.param_id, req.param_index);
    if (p != NULL) {
        camera_send_param(p);
    }
}

static void camera_handle_param_list(const mavlink_message_t *msg)
{
    mavlink_param_ext_request_list_t req;
    mavlink_msg_param_ext_request_list_decode(msg, &req);
    if (req.target_system != fc_sysid || req.target_component != CAMERA_COMPONENT_ID) {
        return;
    }
    unsigned i;
    for (i = 0; i < CAMERA_NUM_PARAMS; i++) {
        camera_send_param(&camera_params[i]);
    }
}

// back to defaults
static uint8_t camera_reset_settings(void)
{
    uint8_t result = MAV_RESULT_ACCEPTED;
    unsigned i;
    for (i = 0; i < CAMERA_NUM_PARAMS; i++) {
        if (camera_params[i].set(camera_params[i].default_value) != OK) {
            result = MAV_RESULT_FAILED;
        }
    }
    return result;
}

/*
  send one of the camera messages; false if we don't do that message
 */
static bool camera_send_message(uint32_t msgid)
{
    switch (msgid) {
    case MAVLINK_MSG_ID_HEARTBEAT:
        camera_send_heartbeat();
        return true;
    case MAVLINK_MSG_ID_CAMERA_INFORMATION:
        camera_send_information();
        return true;
    case MAVLINK_MSG_ID_VIDEO_STREAM_INFORMATION:
        camera_send_stream_information();
        return true;
    case MAVLINK_MSG_ID_CAMERA_CAPTURE_STATUS:
        camera_send_capture_status();
        return true;
    case MAVLINK_MSG_ID_STORAGE_INFORMATION:
        camera_send_storage_information();
        return true;
    }
    return false;
}

static uint8_t camera_take_photo(const mavlink_message_t *msg, const mavlink_command_long_t *c)
{
    // single shots only; no intervals or bursts
    if (c->param2 > 0 || (int)c->param3 != 1) {
        return MAV_RESULT_UNSUPPORTED;
    }
    if (disable_sd_save || mf_snapshot_get_isfull() == 1) {
        return MAV_RESULT_DENIED;
    }
    // the GCS numbers its captures so that a command sent again (as
    // when our ACK was lost) doesn't take a second photo
    const uint32_t seq = (uint32_t)c->param4;
    const uint32_t now = get_time_boot_ms();
    if (seq != 0 && seq == camera_capture_seq &&
        msg->sysid == camera_capture_sysid && msg->compid == camera_capture_compid &&
        now - camera_capture_ms < CAMERA_PHOTO_TIMEOUT_MS) {
        return MAV_RESULT_ACCEPTED;
    }
    if (camera_photos_queued >= CAMERA_MAX_QUEUED_PHOTOS) {
        return MAV_RESULT_TEMPORARILY_REJECTED;
    }
    camera_photos_queued++;
    camera_capture_seq = seq;
    camera_capture_sysid = msg->sysid;
    camera_capture_compid = msg->compid;
    camera_capture_ms = now;
    return MAV_RESULT_ACCEPTED;
}

// give up on the queued photos, telling the GCS each one failed
static void camera_drop_queued_photos(void)
{
    while (camera_photos_queued > 0) {
        camera_photos_queued--;
        camera_send_image_captured(camera_image_count, false);
    }
}

/*
  start queued photos and report finished ones; called every time
  round the main loop
 */
static void camera_photo_update(void)
{
    static uint32_t last_update_ms;
    const uint32_t now = get_time_boot_ms();

    // check_takepic_task() logs each call, so don't ask too often
    if (now - last_update_ms < 100) {
        return;
    }
    last_update_ms = now;

    if (camera_photo_in_progress) {
        unsigned saved, failed, pending;
        mf_snapshot_get_results(&saved, &failed, &pending);
        const bool written = saved != camera_photo_saved_before;
        const bool done = written || failed != camera_photo_failed_before;
        if (!done && now - camera_photo_start_ms < CAMERA_PHOTO_TIMEOUT_MS) {
            return;
        }
        camera_photo_in_progress = false;
        camera_send_image_captured(camera_image_count, written);
        if (written) {
            camera_image_count++;
        }
        if (!done) {
            // the snapshot code may be stuck; don't leave the GCS
            // waiting on photos behind it
            console_printf("camera: photo timed out\n");
            camera_drop_queued_photos();
        }
        camera_send_capture_status();
    }

    if (camera_photos_queued == 0) {
        return;
    }

    // a snapshot started some other way (e.g. the transmitter's
    // button) has to be taken and written before we start ours, so
    // that its result isn't taken for ours
    unsigned saved_before, failed_before, pending;
    mf_snapshot_get_results(&saved_before, &failed_before, &pending);
    if (check_takepic_task() != 0 || pending != 0) {
        if (!camera_queue_blocked) {
            camera_queue_blocked = true;
            camera_queue_blocked_ms = now;
        } else if (now - camera_queue_blocked_ms > CAMERA_PHOTO_TIMEOUT_MS) {
            console_printf("camera: snapshot stuck; dropping queued photos\n");
            camera_drop_queued_photos();
            camera_queue_blocked = false;
            camera_send_capture_status();
        }
        return;
    }
    camera_queue_blocked = false;

    camera_photos_queued--;
    camera_photo_saved_before = saved_before;
    camera_photo_failed_before = failed_before;
    if (take_snapshot()) {
        camera_photo_in_progress = true;
        camera_photo_start_ms = now;
    } else {
        // e.g. the photo folder has filled up
        camera_send_image_captured(camera_image_count, false);
        camera_send_capture_status();
    }
}

/*
  a command from the GCS addressed to the camera
 */
static void camera_handle_command(const mavlink_message_t *msg)
{
    mavlink_command_long_t c;
    mavlink_msg_command_long_decode(msg, &c);
    if (c.target_system != fc_sysid || c.target_component != CAMERA_COMPONENT_ID) {
        return;
    }

    uint8_t result = MAV_RESULT_UNSUPPORTED;
    // message to send once ACKed, if any (not 0: that's HEARTBEAT)
    uint32_t send_after_ack = UINT32_MAX;

    switch (c.command) {
    case MAV_CMD_REQUEST_MESSAGE:
        // ACK every one promptly: QGC holds back further requests to
        // us while one is outstanding
        if ((int)c.param1 == MAVLINK_MSG_ID_VIDEO_STREAM_INFORMATION && c.param2 > 1) {
            break; // we only have stream 1
        }
        switch ((uint32_t)c.param1) {
        case MAVLINK_MSG_ID_HEARTBEAT:
        case MAVLINK_MSG_ID_CAMERA_INFORMATION:
        case MAVLINK_MSG_ID_VIDEO_STREAM_INFORMATION:
        case MAVLINK_MSG_ID_CAMERA_CAPTURE_STATUS:
        case MAVLINK_MSG_ID_STORAGE_INFORMATION:
            result = MAV_RESULT_ACCEPTED;
            send_after_ack = (uint32_t)c.param1;
            break;
        }
        break;

    // the older way of asking for the same messages
    case MAV_CMD_REQUEST_CAMERA_INFORMATION:
        result = MAV_RESULT_ACCEPTED;
        send_after_ack = MAVLINK_MSG_ID_CAMERA_INFORMATION;
        break;
    case MAV_CMD_REQUEST_VIDEO_STREAM_INFORMATION:
        result = MAV_RESULT_ACCEPTED;
        send_after_ack = MAVLINK_MSG_ID_VIDEO_STREAM_INFORMATION;
        break;
    case MAV_CMD_REQUEST_CAMERA_CAPTURE_STATUS:
        result = MAV_RESULT_ACCEPTED;
        send_after_ack = MAVLINK_MSG_ID_CAMERA_CAPTURE_STATUS;
        break;
    case MAV_CMD_REQUEST_STORAGE_INFORMATION:
        result = MAV_RESULT_ACCEPTED;
        send_after_ack = MAVLINK_MSG_ID_STORAGE_INFORMATION;
        break;

    case MAV_CMD_IMAGE_START_CAPTURE:
        result = camera_take_photo(msg, &c);
        send_after_ack = MAVLINK_MSG_ID_CAMERA_CAPTURE_STATUS;
        break;
    case MAV_CMD_RESET_CAMERA_SETTINGS:
        // never UNSUPPORTED: that leaves QGC's capture buttons locked
        result = camera_reset_settings();
        break;

    case MAV_CMD_IMAGE_STOP_CAPTURE:
        // nothing runs on after a single shot
        result = MAV_RESULT_ACCEPTED;
        break;

    case MAV_CMD_VIDEO_START_CAPTURE:
        if (!camera_is_recording()) {
            // the flag can be stale, e.g. if the card filled up
            uart_data.is_recording_video = false;
            toggle_recording();
        }
        result = MAV_RESULT_ACCEPTED;
        send_after_ack = MAVLINK_MSG_ID_CAMERA_CAPTURE_STATUS;
        break;
    case MAV_CMD_VIDEO_STOP_CAPTURE:
        if (camera_is_recording()) {
            // it may have been started from the phone app
            uart_data.is_recording_video = true;
            toggle_recording();
        }
        result = MAV_RESULT_ACCEPTED;
        send_after_ack = MAVLINK_MSG_ID_CAMERA_CAPTURE_STATUS;
        break;
    }

    camera_send_ack(msg, c.command, result);
    if (send_after_ack != UINT32_MAX) {
        camera_send_message(send_after_ack);
    }
}

// is this message addressed to the camera?
static bool camera_is_target(const mavlink_message_t *msg)
{
    const mavlink_msg_entry_t *e = mavlink_get_msg_entry(msg->msgid);
    if (e == NULL || !(e->flags & MAV_MSG_ENTRY_FLAG_HAVE_TARGET_COMPONENT)) {
        return false;
    }
    const uint8_t *payload = (const uint8_t *)_MAV_PAYLOAD(msg);
    return payload[e->target_system_ofs] == fc_sysid &&
        payload[e->target_component_ofs] == CAMERA_COMPONENT_ID;
}

/*
  look at a UDP packet from the GCS before it goes to the flight
  controller; returns true if it must not go there:

  - one of our own broadcasts heard back.  Each of those is a single
    message, so the header tells us.

  - nothing but messages addressed to the camera.  The flight
    controller has no route to us (we never send it anything), so it
    would act on them itself: ArduPilot handles a message for its own
    system ID with an unknown component as if it were addressed to it.
 */
static bool camera_udp_input(const uint8_t *buf, ssize_t len, const struct sockaddr_in *from)
{
    if (fc_sysid != 0) {
        if ((len >= 7 && buf[0] == MAVLINK_STX && buf[5] == fc_sysid && buf[6] == CAMERA_COMPONENT_ID) ||
            (len >= 5 && buf[0] == MAVLINK_STX_MAVLINK1 && buf[3] == fc_sysid && buf[4] == CAMERA_COMPONENT_ID)) {
            if (camera_own_packets++ == 0) {
                console_printf("camera: dropping our own broadcasts heard back\n");
            }
            return true;
        }
    }

    // our own parser state, so as not to need another MAVLink channel.
    // Each datagram is parsed afresh: one sender's partial frame
    // mustn't swallow the start of another's
    static mavlink_message_t rxmsg;
    static mavlink_status_t rxstatus;
    memset(&rxstatus, 0, sizeof(rxstatus));
    bool for_camera = false, for_others = false, framed = false;
    ssize_t i;
    for (i = 0; i < len; i++) {
        mavlink_message_t msg;
        mavlink_status_t status;
        if (mavlink_frame_char_buffer(&rxmsg, &rxstatus, buf[i], &msg, &status) != MAVLINK_FRAMING_OK) {
            continue;
        }
        if (!framed) {
            // only a sender of MAVLink counts as a GCS, and it must be
            // known before we answer it
            framed = true;
            wifi_client_seen(from);
        }
        if (fc_sysid == 0) {
            continue;
        }
        if (!camera_is_target(&msg)) {
            message_intervals_gcs_msg(&msg, false);
            for_others = true;
            continue;
        }
        for_camera = true;
        switch (msg.msgid) {
        case MAVLINK_MSG_ID_COMMAND_LONG:
            camera_handle_command(&msg);
            break;
        case MAVLINK_MSG_ID_PARAM_EXT_REQUEST_LIST:
            camera_handle_param_list(&msg);
            break;
        case MAVLINK_MSG_ID_PARAM_EXT_REQUEST_READ:
            camera_handle_param_read(&msg);
            break;
        case MAVLINK_MSG_ID_PARAM_EXT_SET:
            camera_handle_param_set(&msg);
            break;
        }
    }
    // only whole messages, all for us; anything else goes on as it was
    return for_camera && !for_others && rxstatus.parse_state == MAVLINK_PARSE_STATE_IDLE;
}

static void camera_periodic(void)
{
    static uint32_t last_camera_ms;
    uint32_t now = get_time_boot_ms();

    // only while there's a vehicle for the camera to be part of
    if (fc_sysid == 0 || now - last_heartbeat_ms > 3000) {
        return;
    }

    camera_photo_update();
    if (now - last_camera_ms < 1000) {
        return;
    }
    last_camera_ms = now;

    // free space for CAMERA_CAPTURE_STATUS; FatFs keeps the free
    // cluster count once it has counted them, so this is cheap
    static uint32_t last_storage_ms;
    if (last_storage_ms == 0 || now - last_storage_ms > 10000) {
        mavlink_storage_information_t storage;
        camera_read_storage(&storage);
        last_storage_ms = now;
    }

    camera_send_heartbeat();
    camera_send_information();
    camera_send_stream_information();
    // QGC leaves its buttons showing "busy" until it hears this
    camera_send_capture_status();
}

/*
  Message rates we ask the flight controller for on our link, over
  the stream rates.  A REQUEST_DATA_STREAM (ours, or a GCS's passing
  through from WiFi) resets the interval of each message in the
  streams it names, which for MAV_DATA_STREAM_ALL includes both of
  these, unless the link's MAVn_OPTIONS NOSTREAMOVERRIDE bit is set,
  which older ArduPilots don't have; so they are sent again after
  each one, and whenever the flight controller restarts, since
  intervals don't survive a reboot.
  A message a GCS sets itself with SET_MESSAGE_INTERVAL is left to it
  until the flight controller restarts, and we give up if the flight
  controller doesn't support SET_MESSAGE_INTERVAL.
 */
static const struct {
    uint32_t msgid;
    int32_t interval_us;
    uint8_t stream;             // the MAV_DATA_STREAM it is in on ArduPilot
} fc_message_intervals[] = {
    // a fresh attitude for every video frame's telemetry
    { MAVLINK_MSG_ID_ATTITUDE, 20000, MAV_DATA_STREAM_EXTRA1 },
    // position and velocity for the video telemetry
    { MAVLINK_MSG_ID_GLOBAL_POSITION_INT, 100000, MAV_DATA_STREAM_POSITION },
};
#define FC_MESSAGE_INTERVAL_COUNT (sizeof(fc_message_intervals)/sizeof(fc_message_intervals[0]))
static bool msg_interval_pending[FC_MESSAGE_INTERVAL_COUNT];  // to be sent (again)
static bool msg_interval_gcs[FC_MESSAGE_INTERVAL_COUNT];      // set by a GCS: leave it alone
static bool msg_interval_refused[FC_MESSAGE_INTERVAL_COUNT];  // the FC said no
static bool msg_interval_unsupported;   // the FC can't do SET_MESSAGE_INTERVAL
/*
  One request at a time: COMMAND_ACK doesn't say which message it is
  for, so an ack is only taken for the request outstanding, and after
  one sent more than once is acked, further acks are waited out
  before the next is sent.
 */
static int8_t msg_interval_current = -1;  // the one awaiting its ack
static bool msg_interval_current_reset;   // reset again since it was sent
static uint8_t msg_interval_tries;
static uint32_t msg_interval_sent_ms;
static bool msg_interval_quiet;           // waiting out late acks...
static uint32_t msg_interval_quiet_ms;    // ...since then
// how many of each have arrived since msg_interval_check_ms
static uint16_t msg_interval_seen[FC_MESSAGE_INTERVAL_COUNT];
static uint32_t msg_interval_check_ms;
static uint32_t msg_interval_resent_ms;
static uint32_t msg_interval_fc_heartbeat_ms;  // the FC's own, not another component's
static uint32_t msg_interval_gcs_cmd_ms;       // a GCS's SET_MESSAGE_INTERVAL passed through
static uint32_t msg_interval_web_cmd_ms;       // a web client's, which goes out as us
static bool msg_interval_sys_status_gcs;       // a GCS set SYS_STATUS's interval

// mark ours for sending again: those in a stream, or all of them
static void message_intervals_resend(uint8_t stream)
{
    unsigned i;
    for (i = 0; i < FC_MESSAGE_INTERVAL_COUNT; i++) {
        if (stream == MAV_DATA_STREAM_ALL || fc_message_intervals[i].stream == stream) {
            msg_interval_pending[i] = true;
            if ((int)i == msg_interval_current) {
                // reset after (or as) we sent it, so send it once more
                msg_interval_current_reset = true;
            }
        }
    }
}

// wait a second for late acks before sending the next request
static void message_intervals_quiet(uint32_t now)
{
    msg_interval_quiet = true;
    msg_interval_quiet_ms = now;
}

// start counting arrivals afresh
static void message_intervals_check_reset(uint32_t now)
{
    memset(msg_interval_seen, 0, sizeof(msg_interval_seen));
    msg_interval_check_ms = now;
}

// a message from the flight controller
static void message_intervals_count(uint32_t msgid)
{
    unsigned i;
    for (i = 0; i < FC_MESSAGE_INTERVAL_COUNT; i++) {
        if (fc_message_intervals[i].msgid == msgid && msg_interval_seen[i] < UINT16_MAX) {
            msg_interval_seen[i]++;
        }
    }
}

/*
  check our messages are arriving at about the rates we asked for.
  An acked interval can still be lost unseen: ArduPilot sets every
  interval from its stream rates the first time it sends, which can
  come after it has acked us at boot.
 */
static void message_intervals_check(uint32_t now)
{
    const uint32_t window_ms = now - msg_interval_check_ms;
    if (window_ms < 2000) {
        return;
    }
    bool slow = false;
    unsigned i;
    for (i = 0; i < FC_MESSAGE_INTERVAL_COUNT; i++) {
        const uint32_t expected = window_ms * 1000U / (uint32_t)fc_message_intervals[i].interval_us;
        if (!msg_interval_gcs[i] && !msg_interval_refused[i] &&
            msg_interval_seen[i] * 2U < expected) {
            slow = true;
        }
    }
    message_intervals_check_reset(now);
    if (slow && now - msg_interval_resent_ms > 5000) {
        console_printf("message intervals not kept; sending them again\n");
        msg_interval_resent_ms = now;
        message_intervals_resend(MAV_DATA_STREAM_ALL);
    }
}

// the flight controller restarted: it has forgotten every interval,
// including any a GCS set, and may be different firmware
static void message_intervals_restart(void)
{
    memset(msg_interval_gcs, 0, sizeof(msg_interval_gcs));
    memset(msg_interval_refused, 0, sizeof(msg_interval_refused));
    msg_interval_sys_status_gcs = false;
    msg_interval_unsupported = false;
    // an ack for what we sent before won't come now
    msg_interval_current = -1;
    msg_interval_current_reset = false;
    message_intervals_resend(MAV_DATA_STREAM_ALL);
}

/*
  look at what a GCS on WiFi, or a web client, sends the flight
  controller, for anything that resets or takes over our message
  intervals
 */
static void message_intervals_gcs_msg(const mavlink_message_t *msg, bool from_web)
{
    uint8_t target_system, target_component;
    uint32_t msgid;
    switch (msg->msgid) {
    case MAVLINK_MSG_ID_REQUEST_DATA_STREAM: {
        mavlink_request_data_stream_t m;
        mavlink_msg_request_data_stream_decode(msg, &m);
        // ArduPilot ignores start_stop values other than 0 and 1
        if ((m.target_system == fc_sysid || m.target_system == 0) && m.start_stop <= 1) {
            message_intervals_resend(m.req_stream_id);
        }
        return;
    }
    case MAVLINK_MSG_ID_COMMAND_LONG: {
        mavlink_command_long_t m;
        mavlink_msg_command_long_decode(msg, &m);
        if (m.command != MAV_CMD_SET_MESSAGE_INTERVAL) {
            return;
        }
        target_system = m.target_system;
        target_component = m.target_component;
        msgid = (uint32_t)m.param1;
        break;
    }
    case MAVLINK_MSG_ID_COMMAND_INT: {
        mavlink_command_int_t m;
        mavlink_msg_command_int_decode(msg, &m);
        if (m.command != MAV_CMD_SET_MESSAGE_INTERVAL) {
            return;
        }
        target_system = m.target_system;
        target_component = m.target_component;
        msgid = (uint32_t)m.param1;
        break;
    }
    default:
        return;
    }
    if ((target_system != fc_sysid && target_system != 0) ||
        (target_component != MAV_COMP_ID_AUTOPILOT1 && target_component != 0)) {
        return;
    }
    // ArduPilot before 4.2 (and MAVLink 1) acks it without a target,
    // the same as it acks ours; and a web client's goes out as us, so
    // later ArduPilots ack it to us
    msg_interval_gcs_cmd_ms = get_time_boot_ms();
    if (from_web) {
        msg_interval_web_cmd_ms = msg_interval_gcs_cmd_ms;
    }
    if (msgid == MAVLINK_MSG_ID_SYS_STATUS) {
        // it may have turned it off: no longer a sign the streams stopped
        msg_interval_sys_status_gcs = true;
    }
    unsigned i;
    for (i = 0; i < FC_MESSAGE_INTERVAL_COUNT; i++) {
        if (fc_message_intervals[i].msgid == msgid && !msg_interval_gcs[i]) {
            msg_interval_gcs[i] = true;
            msg_interval_pending[i] = false;
            console_printf("message interval for %u now set by a GCS\n", (unsigned)msgid);
        }
    }
}

static void message_intervals_send(uint8_t i, uint32_t now)
{
    send_command_long(MAV_CMD_SET_MESSAGE_INTERVAL,
                      fc_message_intervals[i].msgid,
                      fc_message_intervals[i].interval_us,
                      0, 0, 0, 0, 0);
    msg_interval_sent_ms = now;
    msg_interval_tries++;
}

// send what needs sending, one at a time, each until it is acked
static void message_intervals_update(uint32_t now)
{
    if (msg_interval_unsupported) {
        return;
    }
    if (fc_sysid == 0 || now - msg_interval_fc_heartbeat_ms > 3000) {
        return;
    }
    if (msg_interval_current >= 0) {
        if (now - msg_interval_sent_ms < 1000) {
            // waiting for its ack
            return;
        }
        if (msg_interval_gcs[msg_interval_current]) {
            // a GCS has set it since we sent ours: sending ours again
            // would undo that
            msg_interval_current = -1;
            message_intervals_quiet(now);
            message_intervals_check_reset(now);
            return;
        }
        if (msg_interval_tries < 5) {
            message_intervals_send(msg_interval_current, now);
            return;
        }
        console_printf("message interval for %u not acked\n",
                       (unsigned)fc_message_intervals[msg_interval_current].msgid);
        // the rate check sends it again if it didn't take
        msg_interval_pending[msg_interval_current] = false;
        msg_interval_current = -1;
        message_intervals_quiet(now);
        message_intervals_check_reset(now);
        return;
    }
    if (msg_interval_quiet) {
        if (now - msg_interval_quiet_ms < 1000) {
            // late acks for one we sent more than once may still come
            return;
        }
        msg_interval_quiet = false;
    }
    unsigned i;
    for (i = 0; i < FC_MESSAGE_INTERVAL_COUNT; i++) {
        if (msg_interval_pending[i] && !msg_interval_gcs[i] && !msg_interval_refused[i]) {
            msg_interval_current = i;
            msg_interval_current_reset = false;
            msg_interval_tries = 0;
            message_intervals_send(i, now);
            return;
        }
    }
    message_intervals_check(now);
}

/*
  can an ack be ours?  Only while we are waiting for one, and not if
  someone else's SET_MESSAGE_INTERVAL in the last 2 seconds may be the
  one being acked; our retry covers that.  ArduPilot before 4.2 (and
  MAVLink 1) don't fill in COMMAND_ACK's target, so an ack without one
  may be any GCS's; one targeted at us may be a web client's, which
  goes out as us
 */
static bool message_intervals_take_ack(bool targeted)
{
    const uint32_t cmd_ms = targeted ? msg_interval_web_cmd_ms : msg_interval_gcs_cmd_ms;
    return msg_interval_current >= 0 &&
           (cmd_ms == 0 || get_time_boot_ms() - cmd_ms > 2000);
}

// a heartbeat from the flight controller itself
static void message_intervals_fc_heartbeat(uint32_t now)
{
    if (msg_interval_fc_heartbeat_ms == 0 || now - msg_interval_fc_heartbeat_ms > 3000) {
        // it may have restarted, which loses message intervals
        message_intervals_restart();
    }
    msg_interval_fc_heartbeat_ms = now;
}

// an ack for one of our SET_MESSAGE_INTERVALs
static void message_intervals_handle_ack(const mavlink_command_ack_t *m)
{
    if (msg_interval_current < 0) {
        // a duplicate, or for one we gave up on
        return;
    }
    const uint32_t now = get_time_boot_ms();
    const uint8_t i = msg_interval_current;
    if (m->result == MAV_RESULT_UNSUPPORTED) {
        console_printf("SET_MESSAGE_INTERVAL unsupported; using stream rates\n");
        msg_interval_unsupported = true;
    } else if (m->result != MAV_RESULT_ACCEPTED) {
        msg_interval_refused[i] = true;
        msg_interval_pending[i] = false;
        console_printf("message interval for %u refused (%u)\n",
                       (unsigned)fc_message_intervals[i].msgid, (unsigned)m->result);
    } else if (!msg_interval_current_reset) {
        msg_interval_pending[i] = false;
    }
    if (msg_interval_tries > 1) {
        message_intervals_quiet(now);
    }
    msg_interval_current = -1;
    message_intervals_check_reset(now);
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

    // ask for the streams when they stop: SYS_STATUS stands for them,
    // since ATTITUDE has a message interval of its own, unless a GCS
    // has set SYS_STATUS's interval itself.  The flight controller can
    // lose a request made while it is still starting
    if (!msg_interval_sys_status_gcs &&
        now - last_send_stream_ms > 5000 &&
        now - last_sys_status_ms > 2000) {
        send_stream_rates_request(4);
        last_send_stream_ms = now;
        // which resets our message intervals
        message_intervals_resend(MAV_DATA_STREAM_ALL);
        if (debug_level > 0) {
            console_printf("sending stream request\n");
        }
    }

    message_intervals_update(now);

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
        if (!request_message_unsupported) {
            send_command_long(MAV_CMD_REQUEST_MESSAGE,
                              MAVLINK_MSG_ID_AUTOPILOT_VERSION, 0, 0, 0, 0, 0, 0);
        }
        if (!request_capabilities_unsupported && !request_message_accepted) {
            send_command_long(MAV_CMD_REQUEST_AUTOPILOT_CAPABILITIES,
                              1, 0, 0, 0, 0, 0, 0);
        }
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
    if (msg->sysid == fc_sysid && msg->compid == MAV_COMP_ID_AUTOPILOT1) {
        message_intervals_count(msg->msgid);
    }
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
        camera_position = m;
        break;
    }
    case MAVLINK_MSG_ID_SYS_STATUS: {
	mavlink_sys_status_t m;
	mavlink_msg_sys_status_decode(msg, &m);
        last_sys_status_ms = now;
        uart_data.telemetry_power = m.voltage_battery/100; // in 0.1V units
        rc_ok = (m.onboard_control_sensors_health & MAV_SYS_STATUS_SENSOR_RC_RECEIVER) != 0;
        break;
    }
    case MAVLINK_MSG_ID_SYSTEM_TIME: {
	mavlink_system_time_t m;
	mavlink_msg_system_time_decode(msg, &m);
        // the flight controller restarted (too quickly for the
        // heartbeat to notice): its message intervals are gone.  Only
        // its own SYSTEM_TIME tells us that
        if (msg->sysid == fc_sysid && msg->compid == MAV_COMP_ID_AUTOPILOT1) {
            static uint32_t last_fc_boot_ms;
            if (m.time_boot_ms + 2000 < last_fc_boot_ms) {
                console_printf("flight controller restarted\n");
                message_intervals_restart();
            }
            last_fc_boot_ms = m.time_boot_ms;
        }
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
            // the flight controller may have been reflashed while we
            // weren't hearing from it; re-probe how to get its version
            request_message_unsupported = false;
            request_message_accepted = false;
            request_capabilities_unsupported = false;
            last_autopilot_version_ms = 0;
        }
        last_heartbeat_ms = now;

	mavlink_heartbeat_t m;
	mavlink_msg_heartbeat_decode(msg, &m);
        if (msg->compid == MAV_COMP_ID_AUTOPILOT1 && m.autopilot != MAV_AUTOPILOT_INVALID) {
            fc_sysid = msg->sysid;
            message_intervals_fc_heartbeat(now);
        }
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
        // only the flight controller's own ACKs tell us what its
        // firmware supports; ignore ACKs from other components
        if (msg->sysid != MAVLINK_TARGET_SYSTEM_ID ||
            msg->compid != MAV_COMP_ID_AUTOPILOT1) {
            break;
        }
        // our own message intervals; a GCS's acks are its own business.
        // Older ArduPilots' acks carry no target: see
        // message_intervals_take_ack()
        if (m.command == MAV_CMD_SET_MESSAGE_INTERVAL &&
            ((m.target_system == mavlink_system.sysid &&
              m.target_component == mavlink_system.compid &&
              message_intervals_take_ack(true)) ||
             (m.target_system == 0 && m.target_component == 0 &&
              message_intervals_take_ack(false)))) {
            message_intervals_handle_ack(&m);
            break;
        }
        // which GCS asked doesn't matter
        if (m.command == MAV_CMD_REQUEST_MESSAGE) {
            if (m.result == MAV_RESULT_ACCEPTED) {
                request_message_accepted = true;
            } else if (m.result == MAV_RESULT_UNSUPPORTED) {
                request_message_unsupported = true;
            }
        } else if (m.command == MAV_CMD_REQUEST_AUTOPILOT_CAPABILITIES &&
                   m.result == MAV_RESULT_UNSUPPORTED) {
            request_capabilities_unsupported = true;
        }
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
                current_baudrate = 0;
                uart2_set_baudrate(telem_baudrates[current_baudrate]);
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
  auto-baud on main port.  Until we hear a heartbeat, try each rate in
  turn, starting once the flight controller has had time to boot;
  once we have heard one, only look again after a long silence, so a
  flight controller restarting or a busy link doesn't make us leave
  the right rate
 */
#define AUTOBAUD_FIRST_MS 20000     // at the first rate before searching
#define AUTOBAUD_LOST_MS 30000      // silence before searching again
#define AUTOBAUD_TRY_MS 4000        // at each rate while searching
static void mavlink_check_baudrate(void)
{
    static uint32_t last_change_ms;
    const uint32_t now = get_time_boot_ms();
    if (last_heartbeat_ms == 0 ? now < AUTOBAUD_FIRST_MS :
        now - last_heartbeat_ms < AUTOBAUD_LOST_MS) {
        return;
    }
    if (last_change_ms != 0 && now - last_change_ms < AUTOBAUD_TRY_MS) {
        return;
    }
    last_change_ms = now;
    current_baudrate = (current_baudrate+1) % NUM_BAUDRATES;
    uart2_set_baudrate(telem_baudrates[current_baudrate]);
    console_printf("no heartbeat; trying baudrate %u\n", (unsigned)telem_baudrates[current_baudrate]);
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
                                              m.chan8_raw,
                                              // channels 9 to 18: 0 means ignore
                                              0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
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
    camera_udp_sock = udp_sock;

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
            struct sockaddr_in from;
            socklen_t fromlen = sizeof(from);
            ssize_t nread = recvfrom(udp_sock, buf, sizeof(buf), 0,
                                     (struct sockaddr *)&from, &fromlen);
            if (nread > 0) {
                process_udp_packet(buf, nread, &from);
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
                // the web server's mavlink_message_send queues here too
                message_intervals_gcs_msg(msg, true);
                _mavlink_resend_uart(MAVLINK_COMM_FC, msg);
                talloc_free(msg);
            }
        }
        
	mavlink_remote_log_periodic();
	mavlink_check_baudrate();
	tx_upload_periodic();
        check_video_recording_end();
        camera_periodic();
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
    {
        const uint32_t now_ms = get_time_boot_ms();
        unsigned i;
        console_printf("unicast: %s\n", wifi_unicast ? "on" : "off");
        for (i = 0; i < WIFI_MAX_CLIENTS; i++) {
            const struct wifi_client *c = &wifi_clients[i];
            if (c->active) {
                char ip[16];
                console_printf("  GCS %s:%u last heard %u ms ago%s\n",
                               inet_ntoa_r(c->addr.sin_addr, ip, sizeof(ip)), ntohs(c->addr.sin_port),
                               (unsigned)(now_ms - c->last_ms),
                               now_ms - c->last_ms > WIFI_CLIENT_TIMEOUT_MS ? " (timed out)" : "");
            }
        }
    }
    console_printf("gps: %s %s  home: %s %s  nsats: %u\n",
                   latlon_str(gps_data.latitude), latlon_str(gps_data.longitude),
                   latlon_str(home_data.latitude), latlon_str(home_data.longitude),
                   gps_data.num_sats);
    console_printf("throttle: %u  alt: %d  bat: %u tstate: %u rf: %u\n",
                   swap_int16(attitude_data.throttle), swap_int16(uart_data.altitude),
                   uart_data.telemetry_power, uart_data.takeoff_state,
                   uart_data.rf_strength);
    console_printf("fakedata: %u  mavsitl: %s\n", dummy_data, sitl_sock!=-1?"true":"false");
    {
        unsigned i;
        console_printf("message intervals:%s\n",
                       msg_interval_unsupported ? " unsupported by the FC" : "");
        for (i = 0; i < FC_MESSAGE_INTERVAL_COUNT; i++) {
            console_printf("  %u every %d us: %s\n",
                           (unsigned)fc_message_intervals[i].msgid,
                           (int)fc_message_intervals[i].interval_us,
                           msg_interval_gcs[i] ? "set by a GCS" :
                           msg_interval_refused[i] ? "refused" :
                           (int)i == msg_interval_current ? "sending" :
                           msg_interval_pending[i] ? "to send" : "sent");
        }
    }
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

void mavlink_set_unicast(bool enable)
{
    wifi_unicast = enable;
    console_printf("mavlink: unicast to GCSes %s\n", enable ? "on" : "off");
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
