/**
 * @file
 * TCP based development console
 * @author tridge, January 2017
 */

#include <FreeRTOS.h>
#include <bsp.h>
#include <ctype.h>
#include <task.h>
#include <nonstdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <generated/snx_sdk_conf.h>
#include <libmid_fwupgrade/fwupgrade.h>
#include <libmid_nvram/snx_mid_nvram.h>
#include <libmid_automount/automount.h>
#include "../../../../../system/cli/src/include/cmd_fs.h"
#include "../../../../../system/cli/src/include/cmd_status.h"
#include "../../../../../system/cli/src/include/cmd_system.h"
#include "../../../../../system/cli/src/rtoscli.h"
#include <../sensor_cap/sensor_capability.h>
#include "socket_ctrl.h"
#include "main_flow.h"
#include "mavlink_wifi.h"
#include "dev_console.h"
#include "tx_upload.h"
#include "mavlink_remote_log.h"
#include "dev_console_json.h"
#include <web_server.h>
#include "bootloader_client.h"
#include "ublox.h"

#include <libmid_fatfs/ff.h>
#include <i2c/i2c.h>
#include "util/print_vprintf.h"
#include <wifi/wifi_api.h>
#include <uart/uart.h>
#include "talloc.h"
#include "video_main.h"
#include "snapshot.h"
#include "record/rec_schedule.h"
#include <libmid_isp/snx_mid_isp.h>
#include "lwip/dhcps.h"

static xTaskHandle console_recv_process;
static xTaskHandle console_upload_process;
static xTaskHandle fc_console_upload_process;
static xTaskHandle tx_console_upload_process;
static xTaskHandle web_server_process;

// task responsible for taking assistance data from a socket:
static xTaskHandle ublox_assist_upload_process;
// task responsible for feeding assistance data to the GPS unit:
static xTaskHandle ublox_assist_process;

#define streq(a,b) (strcmp(a, b) == 0)

#define MAX_SOCKETS 4

static volatile int connected_sock[MAX_SOCKETS] = { -1, -1, -1, -1 };
static volatile bool started;

static xSemaphoreHandle console_mutex;

xSemaphoreHandle any_fw_upgrade_mutex;

// copy of ov9732_adap i2c handle
static struct i2c_adapter *ov9732_adap;

// config params from WIFI.TXT
static struct config_var {
    struct config_var *next;
    const char *name;
    const char *value;
} *config_vars = NULL;

extern char g_str_timezone[10];

#define MAX(a,b) (((a) > (b)) ? (a) : (b))

static uint8_t num_sockets(void)
{
    uint8_t count=0;
    uint8_t i;
    for (i=0; i<MAX_SOCKETS; i++) {
        if (connected_sock[i] != -1) {
            count++;
        }
    }
    return count;
}

static void console_print(const char *buf)
{
    uint8_t i;
    for (i=0; i<MAX_SOCKETS; i++) {
        if (connected_sock[i] != -1) {
            write(connected_sock[i], buf, strlen(buf));
        }
    }
}

static void console_prompt(void)
{
    console_print("dev> ");
}


void console_write(const uint8_t *data, uint32_t len)
{
    uint8_t i;
    if (!console_mutex ||
        xSemaphoreTake(console_mutex, 100) != pdTRUE) {
        return;
    }
    for (i=0; i<MAX_SOCKETS; i++) {
        if (connected_sock[i] != -1) {
            write(connected_sock[i], data, len);
        }
    }
    xSemaphoreGive(console_mutex);
}

void console_printf(const char *fmt, ...)
{
    if (!num_sockets()) {
        return;
    }
    if (strchr(fmt, '%') == NULL) {
        // simple string
        console_write((const uint8_t *)fmt, strlen(fmt));
        return;
    }

    va_list ap;
    va_start(ap, fmt);
    console_vprintf(fmt, ap);
    va_end(ap);
}

void console_vprintf(const char *fmt, va_list ap)
{
    if (!num_sockets()) {
        return;
    }
    if (strchr(fmt, '%') == NULL) {
        // simple string
        console_write((const uint8_t *)fmt, strlen(fmt));
        return;
    }

    char *buf = print_vprintf(NULL, fmt, ap);
    if (buf) {
        console_write((const uint8_t *)buf, talloc_get_size(buf));
        talloc_free(buf);
    }
}

/*
  echo output
 */
static void cmd_echo(unsigned argc, const char *argv[])
{
    console_printf("echo[%u]: ", argc);
    unsigned i;
    
    for (i=1; i<argc; i++) {
        console_printf("%s ", argv[i]);
    }
    console_printf("\n");
}

/*
  reboot board
 */
static void cmd_reboot(unsigned argc, const char *argv[])
{
    console_printf("rebooting ...\n");
    reboot();
}

/*
  start threads
 */
static void cmd_start(unsigned argc, const char *argv[])
{
    console_printf("starting threads\n");
    started = true;
}

/*
  set SSID
 */
static void cmd_ssid(unsigned argc, const char *argv[])
{
    console_printf("Setting SSID to '%s'\n", argv[1]);
    snx_nvram_string_set("WIFI_DEV", "AP_SSID_INFO", __DECONST(char *,argv[1]));
}

/*
  set temporary SSID
 */
static void cmd_ssid_temporary(unsigned argc, const char *argv[])
{
    const char *ssid = argv[1];
    unsigned short length = strlen(ssid);
    console_printf("Setting temporary SSID to '%s'\n", argv[1]);
    WiFi_QueryAndSet(SET_BEACON_OFF, NULL, NULL);
    WiFi_QueryAndSet(SET_BEACON_SSID, (unsigned char *)ssid, &length);
    WiFi_QueryAndSet(SET_BEACON_ON, NULL, NULL);
}

/*
  set wifi password
 */
static void cmd_password(unsigned argc, const char *argv[])
{
    console_printf("Setting password to '%s'\n", argv[1]);
    snx_nvram_string_set("WIFI_DEV", "AP_KEY_INFO", __DECONST(char *,argv[1]));
}

/*
  show memory size and availability
 */
static void cmd_mem(unsigned argc, const char *argv[])
{
    console_printf("Kernel: total %u free %u\n",
                xPortGetTotalHeapSize(GFP_KERNEL),
                xPortGetFreeHeapSize(GFP_KERNEL));
    console_printf("DMA:    total %u free %u\n",
                xPortGetTotalHeapSize(GFP_DMA),
                xPortGetFreeHeapSize(GFP_DMA));
}

/*
  show mavlink statistics
 */
static void cmd_mavlink(unsigned argc, const char *argv[])
{
#if defined(CONFIG_MAVLINK_WIFI)
    if (argc == 1 || streq(argv[1], "status")) {
        mavlink_show_stats();
    } else if (streq(argv[1], "fakedata")) {
        mavlink_fakedata(argc-2, &argv[2]);
    } else {
        console_printf("Unknown mavlink command '%s'\n", argv[1]);
    }
#endif
}

/*
  set mavlink debug
 */
static void cmd_mavdebug(unsigned argc, const char *argv[])
{
#if defined(CONFIG_MAVLINK_WIFI)
    mavlink_set_debug(atoi(argv[1]));
#endif
}

/*
  set web debug
 */
static void cmd_webdebug(unsigned argc, const char *argv[])
{
    web_server_set_debug(atoi(argv[1]));
}

/*
  set mavlink sitl input
 */
static void cmd_mavsitl(unsigned argc, const char *argv[])
{
#if defined(CONFIG_MAVLINK_WIFI)
    mavlink_set_sitl(atoi(argv[1]));
#endif
}

/*
  show update
 */
static void cmd_uptime(unsigned argc, const char *argv[])
{
    console_printf("sys_seconds: %u  ticks: %lu  ticks/ms: %u\n",
                   (unsigned)get_sys_seconds_boot(),
                   (unsigned long)xTaskGetTickCount(),
                   (unsigned)portTICK_PERIOD_MS);
}

/*
  start/stop video preview
 */
static void cmd_preview(unsigned argc, const char *argv[])
{
    if (strcmp(argv[1], "0") == 0) {
        console_printf("stopping preview\n");
        mf_set_preview(0);
    } else if (strcmp(argv[1], "1") == 0) {
        console_printf("starting preview\n");
        mf_set_preview(1);
    } else if (strcmp(argv[1], "help") == 0) {
        console_printf("preview 1|0\n");
    } else {
        console_printf("unknown preview command '%s'\n", argv[1]);
    }
}

/*
  start/stop video record
 */
static void cmd_record(unsigned argc, const char *argv[])
{
    if (argc < 2) {
	console_printf("enable: %d state: %d\n", chk_rec_enable(), schedrec_state());
    } else if (strcmp(argv[1], "0") == 0) {
        console_printf("stopping record\n");
        user_diable_rec();
    } else if (strcmp(argv[1], "1") == 0) {
        console_printf("starting record\n");
        schedrec_suspend_restart(1);
    } else {
        console_printf("unknown record command '%s'\n", argv[1]);
    }
}



/*
  take a snapshot
 */
static void cmd_snapshot(unsigned argc, const char *argv[])
{
    take_snapshot();
}

/*
  list files
 */
static void cmd_fs(unsigned argc, const char *argv[])
{
    print_msg_hook_t old_hook = print_msg_queue_set_hook(console_print);
    
    const char *cmd = argv[0];

    // cope with const breakage in libs
    char **argv2 = __DECONST(char **, argv);
    if (strcmp(cmd, "ls") == 0) {
        cmd_fs_ls(argc, argv2);
    } else if (strcmp(cmd, "rm") == 0) {
        cmd_fs_rm(argc, argv2);
    } else if (strcmp(cmd, "pwd") == 0) {
        cmd_fs_pwd(argc, argv2);
    } else if (strcmp(cmd, "cd") == 0) {
        cmd_fs_cd(argc, argv2);
    } else if (strcmp(cmd, "mkdir") == 0) {
        cmd_fs_mkdir(argc, argv2);
    } else if (strcmp(cmd, "du") == 0) {
        cmd_fs_du(argc, argv2);
    } else {
        console_printf("Unknown command '%s'\n", cmd);
    }
    print_msg_queue_set_hook(old_hook);
}

/*
  show system info
 */
static void cmd_sysinfo(unsigned argc, const char *argv[])
{
    print_msg_hook_t old_hook = print_msg_queue_set_hook(console_print);
    cmd_stat_sysinfo(argc, __DECONST(char **, argv));
    print_msg_queue_set_hook(old_hook);    
}

/*
  manipulate ublox data stuff
 */
void cmd_ublox(unsigned argc, const char *argv[])
{
    if (argc == 1 || streq(argv[1], "status")) {
	cmd_ublox_status(argc-1, &argv[1]);
    } else if (argc > 1 && streq(argv[1], "upload")) {
        cmd_ublox_upload();
    } else {
	console_printf("Unknown ublox command: %s\n", argv[1]);
    }
}

/*
  manipulate remote logging
 */
void cmd_rl(unsigned argc, const char *argv[])
{
    if (argc == 1 || streq(argv[1], "status")) {
	cmd_rl_status(argc-1, &argv[1]);
    } else if (streq(argv[1], "enable")) {
	cmd_rl_enable(argc-1, &argv[1]);
    } else if (streq(argv[1], "disable")) {
	cmd_rl_disable(argc-1, &argv[1]);
    } else if (streq(argv[1], "help")) {
	cmd_rl_help(argc-1, &argv[1]);
    } else {
	console_printf("Unknown remote-log command: %s\n", argv[1]);
    }
}

/*
  manipulate json-from-app
 */
void cmd_json(unsigned argc, const char *argv[])
{
    if (argc == 1 || streq(argv[1], "help")) {
        cmd_json_help(argc-2, &argv[2]);
    } else if (streq(argv[1], "sync")) {
        cmd_json_sync(argc-2, &argv[2]);
    } else if (streq(argv[1], "dump")) {
        cmd_json_dump(argc-2, &argv[2]);
    } else if (streq(argv[1], "status")) {
        cmd_json_status(argc-2, &argv[2]);
    } else {
        console_printf("Unknown json command: %s\n", argv[1]);
    }
}

/*
  control nvram
 */
static void cmd_nvramreset(unsigned argc, const char *argv[])
{
    print_msg_queue("RESETTING NVRAM\n");
    snx_nvram_reset_to_default(NVRAM_RTD_ALL, NULL, NULL);
}

/*
  set uart2 baudrate
 */
static void cmd_baudrate(unsigned argc, const char *argv[])
{
    unsigned rate = strtol(argv[1], NULL, 0);
    uart2_set_baudrate(rate);
    console_printf("Set uart2 baudrate to %u\n", rate);
}

/*
  show video info
 */
static void cmd_vidinfo(unsigned argc, const char *argv[])
{
    unsigned width, height, fps, gop, bps;
    mf_video_resmap_get_record_params(&width, &height, &fps, &gop, &bps);
    console_printf("res=%ux%u fps=%u bps=%u gop=%u\n", width, height, fps, bps, gop);

    mf_video_resmap_get_preview_params(&width, &height, &fps, &gop, &bps);
    console_printf("preview: res=%ux%u fps=%u bps=%u gop=%u\n", width, height, fps, bps, gop);
}

/*
  read or write physical memory
 */
static void cmd_phymem_rw(unsigned argc, const char *argv[])
{
    print_msg_hook_t old_hook = print_msg_queue_set_hook(console_print);
    cmd_system_phymem_rw(argc, __DECONST(char **, argv));
    print_msg_queue_set_hook(old_hook);
}

/*
  write to OV9732 registers
 */
int ov9732_write(uint16_t regnum, uint8_t count, uint8_t *data)
{
    if (ov9732_adap == NULL) {
        return -1;
    }
    struct i2c_msg msg;
    uint8_t buf[2+count];
    uint8_t i;
    
    msg.addr = 0x20;
    msg.flags = 0;
    msg.buf = buf;
    msg.len = 2+count;

    buf[0] = regnum >> 8;
    buf[1] = regnum & 0xFF;

    for (i=0; i<count; i++) {
        buf[2+i] = data[i];
    }
        
    return I2C_transfer(ov9732_adap, &msg, 1);
}

/*
  read or write registers on the oc9732
 */
static void cmd_ov9732(unsigned argc, const char *argv[])
{
    if (ov9732_adap == NULL) {
        console_printf("No ov9732_adap found\n");
        return;
    }
    if (strcmp(argv[1], "read") == 0) {
        if (argc <= 3) {
            console_printf("usage: ov9732 read REG COUNT\n");
            return;
        }
        unsigned regnum = strtol(argv[2], NULL, 0);
        unsigned count = strtol(argv[3], NULL, 0);
        struct i2c_msg msgs[2];
        uint8_t buf1[2];
        uint8_t buf2[count];
        int8_t i;

        memset(msgs, 0, sizeof(msgs));
        memset(buf1, 0, sizeof(buf1));
        memset(buf2, 0, sizeof(buf2));


        msgs[0].addr = 0x20;
        msgs[0].flags = 0;
        msgs[0].buf = buf1;
        msgs[0].len = 2;
        buf1[0] = regnum >> 8;
        buf1[1] = regnum & 0xFF;

        msgs[1].addr = 0x20;
        msgs[1].flags = I2C_M_RD;
        msgs[1].buf = buf2;
        msgs[1].len = count;
        
        int ret = I2C_transfer(ov9732_adap, msgs, 2);
        console_printf("read 0x%04x ret=%d [", regnum, ret);
        for (i=0; i<count-1; i++) {
            console_printf("%02x ", buf2[i]);
        }
        console_printf("%02x]\n", buf2[count-1]);
    }

    if (strcmp(argv[1], "write") == 0) {
        if (argc <= 3) {
            console_printf("usage: ov9732 write REG COUNT [data]\n");
            return;
        }
        unsigned regnum = strtol(argv[2], NULL, 0);
        unsigned count = strtol(argv[3], NULL, 0);
        if (argc < 4+count) {
            console_printf("usage: ov9732 write REG COUNT [data]\n");
            return;
        }
        uint8_t data[count];
        uint8_t i;
        for (i=0; i<count; i++) {
            data[i] = strtol(argv[4+i], NULL, 0);
        }
        int ret = ov9732_write(regnum, count, data);
        console_printf("write 0x%04x ret=%d\n", regnum, ret);
    }
}

/*
  show system time
 */
static void cmd_time(unsigned argc, const char *argv[])
{
	system_date_t t = {0};
	get_date(&t);
        console_printf("%02u-%02u-%02u : %02u:%02u:%02u Z:%s sys_seconds:%lu\n",
                       t.year, t.month, t.day, t.hour, t.minute, t.second,
                       g_str_timezone, (unsigned long)get_sys_seconds());
}

/*
  setup for station mode
 */
static void setup_wifi_station(const char *ssid, const char *password)
{
    xWifiStackEvent_t xTestEvent;

    const char *printpass = password ? password : "(No password)";
    const char *fmt = "Setting up for WiFi station SSID='%s' PASS='%s'\n";
    print_msg_queue(fmt, ssid, printpass);
    console_printf(fmt, ssid, printpass);

    WiFi_Task_UnInit();
    dhcps_deinit();
    vTaskDelay(2000);
    
    WiFi_Task_Init(NULL, WIFI_RUN_MODE_DEV);
    vTaskDelay(2000);

    memset(&xTestEvent, 0x0, sizeof(xTestEvent));
    xTestEvent.eEventType = eWifiSetAPInfo;
    strcpy((char *)xTestEvent.sAPSSID, ssid);
    strcpy((char *)xTestEvent.sAPPassWd, password);
    prvSendEventToWiFiTask(&xTestEvent);
}

/*
  setup in WiFi station mode
 */
static void cmd_wifi_station(unsigned argc, const char *argv[])
{
    if (argc < 3) {
        console_printf("Usage: wifistation SSID [PASSWORD]\n");
        return;
    }
    if (argc < 4) {
        setup_wifi_station(argv[1], NULL);
        return;
    }
    setup_wifi_station(argv[1], argv[2]);
}

/*
  run a system CLI command
 */
static void cmd_sys(char *line)
{
    print_msg_hook_t old_hook = print_msg_queue_set_hook(console_print);
    parse_cmd(line, 0);
    print_msg_queue_set_hook(old_hook);
}

/*
  list all tasks
 */
static void cmd_tasks(unsigned argc, const char *argv[])
{
#if configUSE_TRACE_FACILITY == 1
    unsigned numtasks = uxTaskGetNumberOfTasks();
    char *buf = talloc_size(NULL, numtasks * 80);
    if (buf == NULL) {
        console_printf("unable to allocate for %u tasks\n", numtasks);
        return;
    }
    *buf = 0;
    vTaskList(buf);
    console_printf("Showing %u tasks\n", numtasks);
    console_print(buf);
    console_printf("\n");
    //vTaskGetRunTimeStats(buf);
    //console_printf("%s\n", buf);
    talloc_free(buf);
#else
    console_printf("trace not enabled\n");
#endif
}

typedef void (*cmd_fn)(unsigned , const char *[]);

static const struct {
    char *command;
    cmd_fn function;
    unsigned minargs;
    const char *help;
} commands[] = {
    { "echo", cmd_echo, 1, "usage: echo <ARGS>" },
    { "start", cmd_start, 0, "start main threads" },
    { "reboot", cmd_reboot, 0, "reboot system" },
    { "ssid", cmd_ssid, 1, "usage: ssid <ESSID>" },
    { "ssidtmp", cmd_ssid_temporary, 1, "usage: ssidtmp <ESSID>" },
    { "password", cmd_password, 1, "usage: password <PASSWORD>" },
    { "mem", cmd_mem, 0, "show system memory" },
    { "mavlink", cmd_mavlink, 0, "show mavlink stats" },
    { "mavdebug", cmd_mavdebug, 1, "set mavlink debug level" },
    { "webdebug", cmd_webdebug, 1, "set web server debug level" },
    { "mavsitl", cmd_mavsitl, 1, "enable/disable mavlink input for simulation" },
    { "uptime", cmd_uptime, 0, "show uptime" },
    { "preview", cmd_preview, 1, "start/stop video preview" },
    { "record", cmd_record, 0, "start/stop video record" },
    { "snapshot", cmd_snapshot, 0, "take a snapshot" },
    { "tasks", cmd_tasks, 0, "show tasks" },
    { "ls", cmd_fs, 0, "list files" },
    { "pwd", cmd_fs, 0, "show working directory" },
    { "cd", cmd_fs, 0, "change working directory" },
    { "rm", cmd_fs, 0, "remove file" },
    { "mkdir", cmd_fs, 0, "make directory" },
    { "du", cmd_fs, 0, "show disk usage" },
    { "nvramreset", cmd_nvramreset, 0, "reset NVRAM" },
    { "baudrate", cmd_baudrate, 0, "set uart2 baudrate" },
    { "rl", cmd_rl, 0, "remote logging commands" },
    { "ublox", cmd_ublox, 0, "ublox related commands" },
    { "json", cmd_json, 0, "app json commands" },
    { "sysinfo", cmd_sysinfo, 0, "show system info" },
    { "vidinfo", cmd_vidinfo, 0, "show video info" },
    { "phymem_rw", cmd_phymem_rw, 1, "read or write physical memory" },
    { "ov9732", cmd_ov9732, 1, "read/write ov9732 bus" },
    { "time", cmd_time, 0, "show system time" },
    { "wifistation", cmd_wifi_station, 0, "setup in WiFi station mode" },
    { "sys", NULL, 0, "run a system command" },
    { NULL, NULL, 0, NULL }
};



static void process_line(char *line)
{
    int len = strlen(line);
    // trim trailing whitespace
    while (len > 0 && strchr("\r\n\t",line[len-1])) {
        len--;
        line[len] = 0;
    }

    // trim leading whitespace
    while (len > 0 && strchr("\r\n\t",line[0])) {
        len--;
        line++;
    }

    const unsigned max_args = 20;
    const char *delim = " \t";
    const char *args[max_args];
    char *tok, *savep=NULL;
    unsigned nargs = 0;

    // special case for 'sys' command
    if (strncmp(line, "sys", 3) == 0 && (line[3]<'a' || line[3]>'z')) {
        cmd_sys(line+3);
        console_prompt();
        return;
    }
    
    for (tok = strtok_r(line, delim, &savep);
         tok;
         tok = strtok_r(NULL, delim, &savep)) {
        args[nargs++] = tok;
    }

    if (nargs == 0 || strlen(args[0]) == 0) {
        // no command
        console_prompt();
        return;
    }

    unsigned i;
    if (strcmp(args[0], "help") == 0) {
        for (i=0; commands[i].command != NULL; i++) {
            console_printf("%s: %s\n", commands[i].command, commands[i].help);
        }
        console_prompt();
        return;
    }
    
    for (i=0; commands[i].command != NULL; i++) {
        if (strcmp(commands[i].command, args[0]) == 0 && commands[i].function != NULL) {
            if (nargs-1 < commands[i].minargs) {
                console_printf("%s\n", commands[i].help);
            } else {
                commands[i].function(nargs, args);
            }
            console_prompt();
            return;
        }
    }
    console_printf("Unknown command: '%s'\n", args[0]);
    console_prompt();
}


/*
  periodic tasks for dev_console. Called every 2s
 */
static void dev_console_periodic(void)
{
    int ae_offset = 0;
    int ret = snx_nvram_integer_get("SkyViper", __DECONST(char *, "AE_OFFSET"), &ae_offset);
    if (ret == NVRAM_SUCCESS) {
        int current_ae_offset = 0;
        if (snx_isp_offset_get(&current_ae_offset) == 0 && current_ae_offset != ae_offset && ae_offset >= 0) {
            console_printf("Setting AE_OFFSET to %d\n", ae_offset);
            snx_isp_offset_set(&ae_offset);
        }
    }
    
    /*
      give a way to disable the ov9732 chip via a NVRAM setting
     */
    int video_mode = -1;
    const char *video_enable = "video_enable";
    static int last_mode = -1;

    ret = snx_nvram_integer_get("SkyViper", __DECONST(char *, video_enable), &video_mode);
    if (ret != NVRAM_SUCCESS) {
        // default to best video
        video_mode = 2;
        snx_nvram_integer_set("SkyViper", __DECONST(char *, video_enable), video_mode);
    }

    if (last_mode == video_mode) {
        return;
    }

#if 0
    // disabled for H62 support
    console_printf("Setting video mode %d\n", video_mode);
    
    switch (video_mode) {
    case 0:
        ov9732_set_video(OV9732_VIDEO_OFF);
        break;
    case 1:
    case 2:
        ov9732_set_video(OV9732_NORMAL);
        break;
    case 3:
        ov9732_set_video(OV9732_VIDEO_SLOW);
        break;
    }
#endif

    last_mode = video_mode;
}


/*
  check for FC firmware update at boot from arducopter.abin on microSD
 */
static void check_fc_fw_microSD(void)
{
    const char *filename = "flight_fw.abin";
    print_msg_queue("Checking for FC fw in %s\n", filename);
    
    int size = mw_get_filesize(filename);
    if (size < 200000 || size > 2000000) {
        print_msg_queue("invalid FC FW size %d\n", size);
        // can't be a flight fw
        return;
    }
    FIL fh;
    if (f_open(&fh, filename, FA_READ) != FR_OK) {
        print_msg_queue("failed to open %s\n", filename);
        return;
    }

    static uint8_t *fw;
    if (fw) {
        free(fw);
    }
    fw = malloc(size);
    if (!fw) {
        print_msg_queue("failed to allocate %d bytes\n", size);
        return;
    }

    unsigned int read_count;
    if (f_read(&fh, fw, size, &read_count) != FR_OK ||
        read_count != size) {
        print_msg_queue("failed to read %d bytes\n", size);
        free(fw);
        return;
    }

    f_close(&fh);
    
    uint32_t fw_offset;
    if (check_fc_fw_md5(fw, size, &fw_offset)) {
        print_msg_queue("Good MD5 - updating\n");
        upgrade_fc_firmware(fw, size, fw_offset);
    } else {
        print_msg_queue("Bad MD5 - not updating\n");
    }
}

/*
  get config vars as JSON
 */
void get_config_vars_json(struct sock_buf *sock)
{
    struct config_var *var;
    sock_printf(sock, "{");
    for (var = config_vars; var; var=var->next) {
        sock_printf(sock, "%s\"%s\" : \"%s\"", var==config_vars?"":",", var->name, var->value);
    }
    sock_printf(sock, "}");
}

/*
  get a single WIFI.TXT config variable
 */
const char *get_config_var(const char *varname)
{
    struct config_var *var;
    for (var = config_vars; var; var=var->next) {
        if (strcmp(varname, var->name) == 0) {
            return var->value;
        }
    }
    return NULL;
}

/*
  check for a WIFI.TXT file on microSD insert
  This is used so the factory can have fixed SSID per build station
 */
static void check_wifi_txt(void)
{
    const char *filename = "WIFI.TXT";
    int size = mw_get_filesize(filename);
    char *contents;
    if (size < 5 || size > 1000) {
        print_msg_queue("invalid WIFI.TXT size %d\n", size);
        // can't be a valid WIFI.TXT
        return;
    }
    print_msg_queue("Checking WIFI.TXT\n");
    FIL fh;
    if (f_open(&fh, filename, FA_READ) != FR_OK) {
        return;
    }
    
    contents = malloc(size);
    if (!contents) {
        print_msg_queue("failed to allocate %d bytes\n", size);
        return;
    }

    unsigned int read_count;
    if (f_read(&fh, contents, size, &read_count) != FR_OK ||
        read_count != size) {
        print_msg_queue("failed to read %d bytes\n", size);
        free(contents);
        return;
    }

    f_close(&fh);

    const char *ssid=NULL, *password=NULL;
    int channel = 0;

    // free previous list
    while (config_vars) {
        struct config_var *var = config_vars;
        config_vars = config_vars->next;
        talloc_free(var);
    }
    
    char *tok, *savep=NULL;
    for (tok = strtok_r(contents, "\r\n", &savep);
         tok;
         tok = strtok_r(NULL, "\r\n \t", &savep)) {
        char *p = strchr(tok, '=');
        if (p == NULL) {
            continue;
        }
        *p = 0;
        struct config_var *var = talloc_zero(config_vars, struct config_var);
        if (var == NULL) {
            continue;
        }
        const char *name = tok;
        const char *value = p+1;

        // add to linked list
        var->name = talloc_strdup(var, name);
        var->value = talloc_strdup(var, value);
        var->next = config_vars;
        config_vars = var;
        
        if (strcmp(name, "SSID") == 0) {
            ssid = value;
        }
        if (strcmp(name, "PASSWORD") == 0) {
            password = value;
        }
        if (strcmp(name, "CHANNEL") == 0) {
            channel = atoi(value);
        }
    }

    WiFi_QueryAndSet(SET_BEACON_OFF, NULL, NULL);
    if (ssid) {
        unsigned short length = strlen(ssid);
        print_msg_queue("Set SSID %s\n", ssid);
        WiFi_QueryAndSet(SET_BEACON_SSID, (unsigned char *)ssid, &length);
    }
    if (password) {
        unsigned short length = strlen(password);
        print_msg_queue("Set password %s\n", password);
        WiFi_QueryAndSet(SET_SECURITY_WPA2, (unsigned char *)password, &length);
    }
    if (channel != 0) {
        unsigned short length = 4;
        print_msg_queue("Set channel %d\n", channel);
        WiFi_QueryAndSet(SET_HW_CHANNEL, (unsigned char *)&channel, &length);
    }

    if (get_config_var("STATION_SSID")) {
        setup_wifi_station(get_config_var("STATION_SSID"),
                           get_config_var("STATION_PASS"));
    }
    WiFi_QueryAndSet(SET_BEACON_ON, NULL, NULL);
    
    free(contents);
}

/*
  check for special files on microSD when inserted
 */
void check_microSD_ArduPilot(void)
{
    // delete any left-over upgrade firmware
    f_unlink("FIRMWARE_660R.bin");
    
    check_fc_fw_microSD();
    check_wifi_txt();
}


/**
* listen for debug connection
*/
static void dev_console_task_process(void *pvParameters)
{
    int tcp_sock;
    char buf[128];
    int buflen = 0;
    uint32_t last_periodic = 0;

    struct sockaddr_in addr;

    if ((tcp_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        goto end;
    }

    memset(&addr, 0x0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(CONSOLE_PORT);

    if (bind(tcp_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        goto end;
    }

    if (listen(tcp_sock, 2) < 0) {
        goto end;
    }

    // check for new firmware at startup
    check_fc_fw_microSD();
    
    while (1)
    {
        fd_set fds;
        struct timeval tv;
        unsigned numfd = tcp_sock+1;
        uint8_t i;

        // call periodic tasks every 2s
        if (get_sys_seconds_boot() - last_periodic > 2) {
            last_periodic = get_sys_seconds_boot();
            dev_console_periodic();
        }
        
        FD_ZERO(&fds);
        if (num_sockets() < MAX_SOCKETS) {
            FD_SET(tcp_sock, &fds);
        }

        for (i=0; i<MAX_SOCKETS; i++) {
            if (connected_sock[i] != -1) {
                numfd = MAX(numfd, connected_sock[i] + 1);
                FD_SET(connected_sock[i], &fds);
            }
        }
        
        tv.tv_sec = 10;
        tv.tv_usec = 0;
        int res = select(numfd, &fds, NULL, NULL, &tv);

        if (res >= 1) {
            if (FD_ISSET(tcp_sock, &fds)) {
                // try and accept an incoming connection
                int len = sizeof(struct sockaddr_in);
                for (i=0; i<MAX_SOCKETS; i++) {
                    if (connected_sock[i] == -1) {
                        connected_sock[i] = accept(tcp_sock, (struct sockaddr *)&addr, (socklen_t *)&len);
                        if (connected_sock[i] != -1) {
                            console_printf("Dev console start (compiled " __DATE__ " " __TIME__ ") fd=%d\n", connected_sock[i]);
                            console_prompt();
                        }
                        break;
                    }
                }
            }
            
            for (i=0; i<MAX_SOCKETS; i++) {
                if (connected_sock[i] != -1 && FD_ISSET(connected_sock[i], &fds)) {
                    int n = read(connected_sock[i], &buf[buflen], sizeof(buf)-buflen);
                    if (n <= 0) {
                        int fd = connected_sock[i];
                        if (xSemaphoreTake(console_mutex, 1000) == pdTRUE) {
                            connected_sock[i] = -1;
                            xSemaphoreGive(console_mutex);
                        }
                        close(fd);
                        // assume end of input and a blocking socket
                        console_printf("disconnect %d\n", n);
                        print_msg_queue("closed dev_console %d ret=%d errno=%d\n", fd, n, errno);
                        continue;
                    }
                    buflen += n;
                }
            }
        }

        if (buflen > 0) {
            char *p = memchr(buf, '\n', buflen);
            if (p == NULL) {
                p = memchr(buf, '\r', buflen);
            }
            if (p == NULL) {
                continue;
            }
            int linelen = (int)(p-buf);
            *p = 0;
            process_line(buf);
            buflen = (buflen-linelen)-1;
            memmove(buf, p+1, buflen);
        }

        if (buflen == sizeof(buf)) {
            buflen = 0;
        }
    }

end:
    vTaskDelete(NULL);
}


/*
  check md5 of firmware before upgrading. a firmware should have the
  encoded md5 in last 16 bytes of the image
 */
bool check_fw_md5(const unsigned char *fw, unsigned fwlen)
{
	MD5_CTX ctx;
        uint8_t hash[16];
        unsigned char enc_hash[16];
        uint8_t i;
        
        if (fwlen < 16) {
            return false;
        }
        
	// Calculate MD5
	md5_init(&ctx);
	md5_update(&ctx, __DECONST(void *, fw), (fwlen - 16));
	md5_final(&ctx, hash);

	snx_endecrypt((char *)hash, (char *)enc_hash);

	for (i = 0; i < 16; i++) {
            if (fw[fwlen - 16 + i] != enc_hash[i]) {
                console_printf("Bad MD5 on fw image\n");
                return false;
            }
	}
        console_printf("Good MD5 on fw image\n");
        return true;
}

/**
* accept firmware uploads via TCP
*/
static void fw_upload_task_process(void *pvParameters)
{
    int tcp_sock;

    struct sockaddr_in addr;

    if ((tcp_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        goto end;
    }

    memset(&addr, 0x0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(UPLOAD_PORT);

    if (bind(tcp_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        goto end;
    }

    if (listen(tcp_sock, 2) < 0) {
        goto end;
    }

    while (1)
    {
        mdelay(100);

        int len = sizeof(struct sockaddr_in);
        int fd = accept(tcp_sock, (struct sockaddr *)&addr, (socklen_t *)&len);
        if (fd == -1) {
            continue;
        }

        /* NOTE: we do not release this mutex before rebooting; it is
         * assumed the reboot is successful.  Releasing the mutex may
         * allow someone time to upload e.g. the transmitter firmware
         * between release and our reboot actually happening.  This is
         * taken early to avoid allocating memory if anyone else is in
         * the middle of an upgrade */
        if (!any_fw_upgrade_mutex ||
            xSemaphoreTake(any_fw_upgrade_mutex, 100) != pdTRUE) {
            console_printf("Failed to acquire any_fw_upgrade_mutex\n");
            close(fd);
            continue;
        }

        // read firmware image
        unsigned char *fw;
        
        if (!(fw = (unsigned char *)talloc_size(NULL, MAX_SN_FW_SIZE))) {
            console_printf("Failed to allocate %u bytes\n", MAX_SN_FW_SIZE);
            xSemaphoreGive(any_fw_upgrade_mutex);
            close(fd);
            continue;
        }
        unsigned fwlen = 0;
        
        while (1) {
            unsigned n;
            n = read(fd, fw+fwlen, MAX_SN_FW_SIZE-fwlen);
            if (n <= 0) {
                break;
            }
            fwlen += n;
        }
        close(fd);

        console_printf("Uploaded %u bytes\n", fwlen);

        if (!check_fw_md5(fw, fwlen)) {
            talloc_free(fw);
            xSemaphoreGive(any_fw_upgrade_mutex);
            continue;
        }

        if (dev_write_file("FIRMWARE_660R.bin", (const uint8_t *)fw, fwlen) != 0) {
            set_upload_message("Failed to write image to microSD - please check SD card");
            talloc_free(fw);
            xSemaphoreGive(any_fw_upgrade_mutex);
            continue;
        }        

        console_printf("Upgrading ...\n");
        mdelay(100);

        //all_task_uinit(0);
        fw_upgrade(fw, fwlen);

	fwburning_info_t* sfwb_info = get_fwburning_info();

        print_msg_queue("wait upgrading ...\n");
        if (xSemaphoreTake(sfwb_info->FwburningMutex, portMAX_DELAY ) == pdTRUE ) {
            print_msg_queue("reboot !!!\n");
        }
        reboot();
    }

end:
    vTaskDelete(NULL);
}

/**
* accept fc firwmare uploads via TCP
*/
static uint8_t fc_fw_upload_task_process_upload_progress_last_percent = 101;
static void fc_fw_upload_task_process_upload_progress(const uint8_t percentage)
{
    if (fc_fw_upload_task_process_upload_progress_last_percent != percentage) {
        set_upload_progress(percentage);
        console_printf("Sent %u%%\n", percentage);
        fc_fw_upload_task_process_upload_progress_last_percent = percentage;
    }
}

/*
 * returns true if a sync was achieved
 */
static int fc_fw_draining_sync()
{
    set_upload_message("Getting bootloader sync");
    uint8_t get_sync_done = 0;
    uint8_t i;
    const uint8_t max_attempts = 10;
    for (i=0; i<max_attempts; i++) {
        uint8_t b;
        uint32_t draincount = 0;
        while (bootloader_readbyte(&b,1) == 1) {
            draincount++;
        }
        if (bootloader_get_sync() == -1) {
            console_printf("Still awaiting sync (%i/%i) (%lu bytes drained)\n", i, max_attempts, (unsigned long)draincount);
            mdelay(100);
            continue;
        }
        set_upload_message("got sync");
        get_sync_done = 1;
        break;
    }
    if (!get_sync_done) {
        console_printf("get_sync timeout\n");
    }
    return get_sync_done;
}

/*
  convert a hex digit to a number
 */
static uint8_t hex_digit(char c)
{
    switch (c) {
    case '0' ... '9':
        return c - '0';
    case 'a' ... 'f':
        return 10 + (c - 'a');
    case 'A' ... 'F':
        return 10 + (c - 'A');
    }
    return 0;
}

/*
  check md5 of FC firmware before upgrading. a firmware should have
  the encoded md5 as a text header
  if this passes, then set fw_offset to the offset of the binary firmware
  in the data

  The file format this uses is created by Tools/scripts/make_abin.sh
  in the ArduPilot tree
 */
bool check_fc_fw_md5(unsigned char *fw, unsigned fwlen, uint32_t *fw_offset)
{
	MD5_CTX ctx;
        uint8_t hash[16];
        uint8_t fw_md5[16];
        uint16_t i, j;
        char *md5_str;
        
        if (fwlen < 1000) {
            console_printf("Firmware too small %u\n", fwlen);
            return false;
        }

        // find the MD5: header
        for (i=0; i<1000; i++) {
            if (strncmp((char *)&fw[i], "MD5: ", 5) == 0) {
                break;
            }
        }
        if (i == 1000 || (fwlen - i) < 37) {
            console_printf("No MD5 header found\n");
            return false;
        }
        md5_str = (char *)&fw[i+5];

        console_printf("MD5: %32.32s\n", md5_str);
        
        for (j=0; j<16; j++) {
            if (!isxdigit(md5_str[j*2]) || !isxdigit(md5_str[j*2+1])) {
                console_printf("bad MD5 string\n");
                return false;
            }
            fw_md5[j] = hex_digit(md5_str[j*2]) << 4 | hex_digit(md5_str[j*2+1]);
        }

        // find the -- end of header marker
        for (; i<1000; i++) {
            if (strncmp((char *)&fw[i], "--\n", 3) == 0) {
                break;
            }
        }
        if (i == 1000) {
            console_printf("No firmware body found\n");
            return false;
        }
        (*fw_offset) = i+3;
        
	// Calculate MD5
	md5_init(&ctx);
	md5_update(&ctx, fw+(*fw_offset), (fwlen - (*fw_offset)));
	md5_final(&ctx, hash);

        if (memcmp(fw_md5, hash, 16) != 0) {
            console_printf("Bad MD5 on fc fw image fw_offset=%u\n", *fw_offset);
            return false;
        }
        console_printf("Good MD5 on fc fw image\n");
        return true;
}

static int upload_task_upload_firmware(const unsigned char *fw, const unsigned fwlen, const uint32_t desired_baud);


/*
  perform fc firmware upgrade task
 */
void upgrade_fc_firmware(const unsigned char *fw, unsigned fwlen, unsigned fw_offset)
{
        /* attempt to control the mavlink port */
        console_printf("Attempting to get telemetry control\n");
        uint8_t i;
        for (i=0; i<10; i++) {
            if (telem_main_get_mode() == TELEM_MODE_DEVCONSOLE) {
                break;
            }
            telem_main_request_mode(TELEM_MODE_DEVCONSOLE);
            mdelay(100);
        }
        if (telem_main_get_mode() != TELEM_MODE_DEVCONSOLE) {
            console_printf("Failed to gain telemetry control\n");
            return;
        }

        pic_disableInterrupt(UART2_IRQ_NUM);

        // try a range of baudrates
        uint32_t baudrates[] = { 625000, 115200 };
        uint8_t nbaud = sizeof(baudrates) / sizeof(baudrates[0]);
        for (i=0; i<10; i++) {
            uint32_t rate = baudrates[i % nbaud];
            if (upload_task_upload_firmware(fw+fw_offset, fwlen-fw_offset, rate) == 0) {
                break;
            }
        }

        set_upload_message("Enabling mavlink");
        telem_main_request_mode(TELEM_MODE_MAVLINK);

        set_upload_message("Reenabling interrupts");
        pic_enableInterrupt(UART2_IRQ_NUM);

        set_upload_message("upgrade finished");
        set_upload_progress(100);
}


/*
  task for uploading flight control firmware over TCP
 */
static void fc_fw_upload_task_process(void *pvParameters)
{
    int tcp_sock;

    struct sockaddr_in addr;

    if ((tcp_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        goto end;
    }

    memset(&addr, 0x0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(FC_UPLOAD_PORT);

    if (bind(tcp_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        goto end;
    }

    if (listen(tcp_sock, 2) < 0) {
        goto end;
    }

    while (1)
    {
        mdelay(100);

        int len = sizeof(struct sockaddr_in);
        int fd = accept(tcp_sock, (struct sockaddr *)&addr, (socklen_t *)&len);
        if (fd == -1) {
            continue;
        }

        // attempt to acquire mutex early so we don't allocate memory
        // if someone else is in the middle of an upgrade:
        if (!any_fw_upgrade_mutex ||
            xSemaphoreTake(any_fw_upgrade_mutex, 100) != pdTRUE) {
            console_printf("Failed to acquire any_fw_upgrade_mutex\n");
            close(fd);
            continue;
        }

        // read firmware image
        unsigned char *fw;

        console_printf("Receiving new FC firmware\n");

        if (!(fw = (unsigned char *)talloc_size(NULL, MAX_FC_FW_SIZE))) {
            console_printf("Failed to allocate %u bytes\n", MAX_FC_FW_SIZE);
            xSemaphoreGive(any_fw_upgrade_mutex);
            close(fd);
            continue;
        }
        unsigned fwlen = 0;

        memset(fw, '\xff', MAX_FC_FW_SIZE);

        while (1) {
            unsigned n;
            n = read(fd, fw+fwlen, MAX_FC_FW_SIZE-fwlen);
            if (n <= 0) {
                break;
            }
            fwlen += n;
        }
        close(fd);

        console_printf("Received %u bytes (thanks!)\n", fwlen);
        mdelay(100);

        uint32_t fw_offset=0;
        if (!check_fc_fw_md5(fw, fwlen, &fw_offset)) {
            talloc_free(fw);
            xSemaphoreGive(any_fw_upgrade_mutex);
            continue;
        }

        upgrade_fc_firmware(fw, fwlen, fw_offset);

        talloc_free(fw);

        xSemaphoreGive(any_fw_upgrade_mutex);

    } // end while(1)

end:
    vTaskDelete(NULL);
}

const uint32_t initial_bauds[] = { 115200, 625000 };

int autobaud_bootloader()
{
        uint8_t i=0;
        for (i=0; i<sizeof(initial_bauds)/sizeof(initial_bauds[0]); i++) {
            const uint32_t initial_baud = initial_bauds[i];
            console_printf("Setting local baud to %u\n", initial_baud);
            uart2_set_baudrate(initial_baud);

            if (!fc_fw_draining_sync()) {
                console_printf("Failed to sync\n");
            } else {
                return 1;
            }
        }
        return 0;
}

/* must be called with interrupts disabled! */
static int get_bootloader()
{
        uint8_t i=0;
        for (i=0; i<sizeof(initial_bauds)/sizeof(initial_bauds[0]); i++) {
            const uint32_t initial_baud = initial_bauds[i];;
            console_printf("Setting local baud to %u\n", initial_baud);
            uart2_set_baudrate(initial_baud);

            set_upload_message("Sending mavlink reboot");
            if (bootloader_send_mavlink_reboot() == -1) {
                set_upload_message("Failed to send mavlink reboot");
                // rumble on....
            } else {
                // pause a while to let that stuff go out....
                mdelay(50);
            }
        }

        return autobaud_bootloader();
}

static int upload_task_upload_firmware(const unsigned char *fw, const unsigned fwlen, const uint32_t desired_baud)
{
        int ret = -1;

        unsigned int old_baudrate = uart2_get_baudrate();

        console_printf("Sending mavlink reboot\n");
        if (bootloader_send_mavlink_reboot() == -1) {
            console_printf("Failed to send mavlink reboot\n");
            // rumble on....
        } else {
            // pause a while to let that stuff go out....
            mdelay(100);
        }

        int synced = get_bootloader();
        if (!synced) {
                console_printf("Failed to do initial sync\n");
                goto out_no_sync;
        }

        console_printf("Setting remote baudrate to %u\n", desired_baud);

        if (bootloader_set_baudrate(desired_baud) == -1) {
            console_printf("Set baudrate failed\n");
        } else {
            console_printf("Setting local baud to %u\n", desired_baud);
            uart2_set_baudrate(desired_baud);

            if (!fc_fw_draining_sync()) {
                goto out_bootloader_synced;
            }
        }

        set_upload_message("Erasing flash");
        if (bootloader_erase_send() == -1) {
            console_printf("Failed to send erase\n");
            goto out_bootloader_synced;
        }
        console_printf("Waiting to recv sync\n");
        uint8_t sync_received = 0;
        uint8_t i;
        for (i=0;i<15;i++) {
            bl_reply_t status;
            if (bootloader_recv_sync(&status, 1000) == -1) {
                console_printf("No sync yet\n");
                mdelay(1000);
                continue;
            }
            console_printf("Sync received\n");
            sync_received = 1;
            break;
        }
        if (!sync_received) {
            console_printf("No sync received after erase\n");
            goto out_bootloader_synced;
        }
        set_upload_message("Uploading firmware");
        long long start = get_sys_seconds_boot();
        if (bootloader_program(__DECONST(uint8_t*,fw), fwlen+(fwlen%4), fc_fw_upload_task_process_upload_progress) == -1) {
            console_printf("Upload failed\n");
            goto out_bootloader_synced;
        }
        console_printf("Upload took (%lld) seconds\n", get_sys_seconds_boot()-start);
        uint32_t fc_crc;
        if (bootloader_get_crc(&fc_crc) == -1) {
            console_printf("get_crc failed\n");
            goto out_bootloader_synced;
        }
        uint32_t flashed_fw_size;
        if (bootloader_get_fw_size(&flashed_fw_size) == -1) {
            console_printf("get_fw_size failed\n");
            goto out_bootloader_synced;
        }
        uint32_t my_fw_crc = bootloader_crc32_firmware(__DECONST(uint8_t*,fw), fwlen+(fwlen%4), flashed_fw_size);
        if (my_fw_crc != fc_crc) {
            console_printf("get_fw_size failed\n");
            goto out_bootloader_synced;
        }
        set_upload_message("CRC match - success");
        console_printf("CRC match; flash successful (crc=0x%08x)\n", fc_crc);
        if (bootloader_boot() != -1) {
            console_printf("Sent boot\n");
            f_unlink("flight_fw.abin");
        } else {
            console_printf("Failed to send boot command\n");
        }
        // avoid talking mavlink for a little while:
        mdelay(1000);
        ret = 0;

    out_bootloader_synced:
        /* reset serial port */
        console_printf("Setting baudrate to %x\n", old_baudrate);
        uart2_set_baudrate(old_baudrate);

    out_no_sync:

        return ret;
}

/**
 * binds and listens on supplied port.  When a connection is accepted,
 * reads all of the data (up to a maximum size of maxsize) from that
 * connection until EOF.  Then calls munch with the data and its
 * length.  The memory containing the data is freed upon return from
 * munch, so take a copy if you need it.
 */
void port_read_then_call(const char *description, int port, uint32_t maxsize, void (*munch)(const uint8_t*, const uint32_t))
{
    struct sockaddr_in addr;

    int tcp_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (tcp_sock == -1) {
        print_msg_queue("pstc: Failed to create socket %u\n", port);
        goto end;
    }

    memset(&addr, 0x0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    if (bind(tcp_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        print_msg_queue("pstc: Failed to bind %u\n", port);
        goto end;
    }

    if (listen(tcp_sock, 2) < 0) {
        print_msg_queue("pstc: Failed to listen \n");
        goto end;
    }

    print_msg_queue("pstc: %s listening on port %u sock=%u\n", description, port, tcp_sock);

    while (1)
    {
        mdelay(100);

        int fd = accept(tcp_sock, NULL, NULL);
        if (fd == -1) {
            continue;
        }

        // read data
        console_printf("Receiving new %s (maxsize=%u)\n", description, maxsize);

        unsigned char *fw = (unsigned char *)talloc_size(NULL, maxsize);
        if (!fw) {
            console_printf("Failed to allocate %u bytes\n", maxsize);
            close(fd);
        }
        unsigned fwlen = 0;

        while (1) {
            unsigned n;
            n = read(fd, fw+fwlen, maxsize-fwlen);
            if (n <= 0) {
                break;
            }
            fwlen += n;
        }
        close(fd);

        console_printf("Received %u bytes (thanks!)\n", fwlen);
        if (fwlen >= maxsize) {
            console_printf("Probably too-small a buffer; aborting");
            goto err_free;
        }
        mdelay(100);

        munch(fw, fwlen);

    err_free:
        talloc_free(fw);

    } // end while(1)

end:
    vTaskDelete(NULL);
}

/**
* @brief interface function - udp command socket task create.
*/
int dev_console_socket_create()
{
    console_mutex = xSemaphoreCreateMutex();
    any_fw_upgrade_mutex = xSemaphoreCreateMutex();
    
    print_msg_queue("Waiting one second ...\n");
    
    mdelay(1000);

    print_msg_queue("starting dev_console\n");
    
    if (pdPASS != xTaskCreate(dev_console_task_process, "dev_console", STACK_SIZE_16K, NULL, 30, &console_recv_process)) {
        goto fail1;
    }

    if (pdPASS != xTaskCreate(fw_upload_task_process, "fw_upload", STACK_SIZE_16K, NULL, 20, &console_upload_process)) {
        goto fail1;
    }

    if (pdPASS != xTaskCreate(fc_fw_upload_task_process, "fc_fw_upload", STACK_SIZE_16K, NULL, 20, &fc_console_upload_process)) {
        goto fail1;
    }

    if (pdPASS != xTaskCreate(tx_fw_upload_task_process, "tx_fw_upload", STACK_SIZE_16K, NULL, 20, &tx_console_upload_process)) {
        goto fail1;
    }

    if (pdPASS != xTaskCreate(web_server_task_process, "web_server", STACK_SIZE_16K, NULL, 20, &web_server_process)) {
        goto fail1;
    }

    ublox_init();
    if (pdPASS != xTaskCreate(ublox_assist_upload_task_process, "ublox_assist_upload", STACK_SIZE_4K, NULL, 20 , &ublox_assist_upload_process)) {
        goto fail1;
    }

    if (pdPASS != xTaskCreate(ublox_assist_task_process, "ublox_assist", STACK_SIZE_16K, NULL, 20 , &ublox_assist_process)) {
        goto fail1;
    }

#ifdef CONFIG_DEV_CONSOLE_PAUSE
    print_msg_queue("Waiting for 'start' command\n");
    while (!started) {
        // sleep 0.1 second
        mdelay(100);
    }
#endif

    return pdPASS;

fail1:
    vTaskDelete(console_recv_process);
    vTaskDelete(console_upload_process);
    vTaskDelete(fc_console_upload_process);
    vTaskDelete(tx_console_upload_process);
    vTaskDelete(ublox_assist_upload_process);
    vTaskDelete(ublox_assist_process);
    print_msg_queue("dev_console failed\n");
    return pdFAIL;
}

#if 1
/*
  intercept i2c transfer operations. This relies on changing i2c_transfer to I2C_transfer in libi2c.a
 */
int i2c_transfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num)
{
    if (ov9732_adap == NULL && num > 0 && msgs && msgs[0].addr == 0x20) {
        // snag the ov9732 handle. We could probaby create our own,
        // but I don't know all the expected arguments to the i2c
        // handle creation functions
        ov9732_adap = adap;
    }
#if 0
    // intercept and print i2c operations
    if (num > 0) {
        console_printf("I2C %s num=%u 0x%02x flags=0x%x len=%u [%02x %02x %02x]\n",
                       adap->name, num, msgs[0].addr, msgs[0].flags, msgs[0].len,
                       msgs[0].buf[0], msgs[0].buf[1], msgs[0].buf[2]);
    }
    int ret = I2C_transfer(adap, msgs, num);
    console_printf("I2O %s num=%u 0x%02x flags=0x%x len=%u [%02x %02x %02x]\n",
                   adap->name, num, msgs[0].addr, msgs[0].flags, msgs[0].len,
                   msgs[0].buf[0], msgs[0].buf[1], msgs[0].buf[2]);
#else
    // just pass them through
    int ret = I2C_transfer(adap, msgs, num);
#endif
    return ret;
}
#endif


// support web progress bars
static uint8_t upload_progess;
static const char *upload_message;
static long long last_progress_time;

void set_upload_progress(uint8_t percent)
{
    last_progress_time = get_sys_seconds_boot();
    upload_progess = percent;
}

uint8_t get_upload_progress(void)
{
    if (get_sys_seconds_boot() - last_progress_time > 20) {
        return 0;
    }
    return upload_progess;
}

void set_upload_message(const char *message)
{
    if (message != upload_message) {
        console_printf("%s\n", message);
        upload_message = message;
    }
}

const char *get_upload_message(void)
{
    if (get_sys_seconds_boot() - last_progress_time > 20) {
        return "";
    }
    return upload_message;
}

/*
  see if a file exists on the microSD card
 */
bool file_exists(const char *filename)
{
    FILLIST f = {};
    f.finfo.lfname = f.lfname;
    f.finfo.lfsize = sizeof(f.lfname);
    return f_stat(filename, &f.finfo) == FR_OK;
}

/*
  setup defaults for ArduPilot NVRAM
 */
void ardupilot_nvram_setup(void)
{
    char var[100];
    if (snx_nvram_string_get("WIFI_DEV", "AP_SSID_PREFIX", var) != NVRAM_SUCCESS) {
        print_msg_queue("Setting ArduPilot WiFi defaults\n");
        snx_nvram_string_set("WIFI_DEV", "AP_SSID_PREFIX", "SKYVIPER_");
        snx_nvram_integer_set("WIFI_DEV", "AP_AUTH_MODE", AUTH_WPA2);
        snx_nvram_string_set("WIFI_DEV", "AP_KEY_INFO", "skyviper");
        snx_nvram_integer_set("WIFI_DEV", "AP_CHANNEL_INFO", 9);
    } else {
        print_msg_queue("AP_SSID_PREFIX is '%s'\n", var);
    }

    // default recording interval to 1200 from 180
    int interval = 0;
    if (snx_nvram_integer_get("RECORD", __DECONST(char *, "RECORDSCHEDINTERVAL"), &interval) != NVRAM_SUCCESS ||
        interval == 180) {
        snx_nvram_integer_set("RECORD", __DECONST(char *, "RECORDSCHEDINTERVAL"), 1200);
    }
}

/*
  sleep for ms milliseconds
 */
void mdelay(uint32_t ms)
{
    vTaskDelay(ms/portTICK_PERIOD_MS);
}

/*
  get time since boot in milliseconds
 */
uint32_t get_time_boot_ms(void)
{
    return xTaskGetTickCount() * portTICK_PERIOD_MS;
}


void ov9732_set_video(enum ov9732_setting setting)
{
    switch (setting) {
    case OV9732_NORMAL: {
        uint8_t v = 4;
        ov9732_write(0x3082, 1, &v);
        v = 0x3c;
        ov9732_write(0x3081, 1, &v);
        break;
    }
    case OV9732_VIDEO_SLOW: {
        uint8_t v = 6;
        ov9732_write(0x3082, 1, &v);
        v = 0x3c;
        ov9732_write(0x3081, 1, &v);
        break;
    }
    case OV9732_VIDEO_OFF: {
        uint8_t v = 4;
        ov9732_write(0x3081, 1, &v);
        ov9732_write(0x3082, 1, &v);
        break;
    }
    }
}

/*
  return MD5 of a blob as a talloc string
 */
char *md5_string(void *memctx, const uint8_t *data, uint32_t len)
{
    MD5_CTX ctx;
    uint8_t hash[16];
    md5_init(&ctx);
    md5_update(&ctx, __DECONST(void *, data), len);
    md5_final(&ctx, hash);
    char *s = talloc_zero_size(memctx, 33);
    if (s == NULL) {
        return s;
    }
    uint8_t i;
    for (i=0; i<16; i++) {
        sprintf(&s[i*2], "%02X", hash[i]);
    }
    s[32] = 0;
    return s;
}


/*
 * write data_len bytes from data to path, creating/truncating that file if required
 # returns 0 on success, -1 on failure
 */
int dev_write_file(const char *path, const uint8_t *data, const uint32_t data_len)
{
    FIL fptr;
    int ret = -1;

    if (f_open(&fptr, path, FA_CREATE_ALWAYS|FA_WRITE) != FR_OK) {
        console_printf("Failed to open (%s)\n", path);
        goto out;
    }

    uint32_t total_bytes_written = 0;
    uint32_t bytes_written;
    while (total_bytes_written < data_len) {
        if (f_write(&fptr, &data[total_bytes_written], data_len-total_bytes_written, &bytes_written) != FR_OK) {
            console_printf("Failed to write to (%s)\n", path);
            goto out_file_open;
        }
        if (bytes_written == 0) {
            console_printf("Zero bytes written to (%s)\n", path);
            goto out_file_open;
        }
        if (bytes_written < data_len) {
            console_printf("short write to (%s)\n", path);
        }
        total_bytes_written += bytes_written;
    }
    if (total_bytes_written == data_len) {
        ret = 0;
    }
    f_sync(&fptr);
out_file_open:
    f_close(&fptr);
out:
    return ret;
}

/*
  reset WiFi settings - triggered on WIFIRESET message from flight board
 */
void wifi_reset(void)
{
    console_printf("Reset WiFi settings\n");
    const unsigned char * mac = wlan_get_get_mac_addr();
    char *ssid = talloc_asprintf(NULL, "SKYVIPER_%02X%02X%02X", mac[3], mac[4], mac[5]);

    snx_nvram_string_set("WIFI_DEV", "AP_SSID_INFO", ssid);
    snx_nvram_integer_set("WIFI_DEV", "AP_AUTH_MODE", AUTH_WPA2);
    snx_nvram_string_set("WIFI_DEV", "AP_KEY_INFO", "skyviper");
    snx_nvram_integer_set("WIFI_DEV", "AP_CHANNEL_INFO", 9);

    talloc_free(ssid);
    
    WiFi_Task_UnInit();
    dhcps_deinit();
    vTaskDelay(2000);
    
    WiFi_Task_Init(NULL, WIFI_RUN_MODE_AP);
    vTaskDelay(2000);
    dhcps_init();
}
