#ifndef _MCU_CTRL_H_
#define _MCU_CTRL_H_

#include <task.h>
#include <generated/snx_sdk_conf.h>

#include "stm32_ctrl.h"

typedef struct mcu_commands_func {
	uint8_t (*cmd_sync)(uint8_t*);
	uint8_t (*cmd_record_start)(void);
	uint8_t (*cmd_set_setting)();
	uint8_t (*cmd_record_stop)(void);
	uint8_t (*cmd_snap_shot)(void);
	uint8_t (*cmd_file_lock)(void);
	uint8_t (*cmd_wifi_on)(void);
	uint8_t (*cmd_wifi_off)(void);
	uint8_t (*cmd_reset_status)(void);
	uint8_t (*cmd_usb_detection)(uint8_t);
	uint8_t (*cmd_emergency_lock)(uint8_t, uint8_t);
	uint8_t (*cmd_gps_location)(uint8_t, double, uint8_t, double);
	uint8_t (*cmd_gps_date)(long long, uint16_t);
	uint8_t (*cmd_g_sensor_data)(short, short, short, uint8_t);
	int (*cmd_read_wifichannel)(void);							//CSL
	uint8_t (*cmd_fbgps_info)(uint8_t*);							//CSL FJW
	uint8_t (*cmd_navigation_status)(uint8_t*);						//CSL FJW
	
} mcu_commands_func_t;

typedef struct mcu_cmd_counter {
	uint32_t cnt_sync;
	uint32_t cnt_recstart;
	uint32_t cnt_recstop;
	uint32_t cnt_snapshot;
	uint32_t cnt_filelock;
	uint32_t cnt_wifion;
	uint32_t cnt_wifioff;
	uint32_t cnt_resetstatus;
	uint32_t cnt_usbdevice;
	uint32_t cnt_emergencylock;
	uint32_t cnt_gpslocation;
	uint32_t cnt_gps_date;
	uint32_t cnd_g_sensor_data;
} mcu_cmd_counter_t;

#define CMD_SYNC			0x01
#define CMD_SETTING			0x02
#define CMD_TESTING			0x03

#define CMD_RECORD_START	0x80
#define CMD_RECORD_STOP		0x81
#define CMD_SNAP_SHOT		0x82
#define CMD_FILE_LOCK		0x83
#define CMD_WIFI_ON			0x84
#define CMD_WIFI_OFF		0x85
#define CMD_PARA_RESET		0x86
#define CMD_RESET_STATUS	0x87
#define CMD_USB_DETECTION	0x88
#define CMD_EMERGENCY_LOCK	0x89

#define CMD_FBGPS_INFO     	0x20				//CSL FJW
#define CMD_NAVIGATION_STA  0x21				//CSL FJW
#define CMD_FLIGHT_PLAN     0x35				//CSL FJW

#define CMD_GPS_LOCATION	0x90
#define CMD_GPS_DATE		0x91
#define CMD_G_SENSOR_DATA	0x92

#define CMD_FLIGHT_INFO		0xB0				//CSL
#define CMD_RESET_PASSWORD	0xB1				//CSL
#define CMD_RESET_DRONE     0xB2	// CPM extra command to reset the drone: 0xB3 0x7B 0xEA
#define CMD_BOOTLOADER_DRONE 0xB3	// CPM extra command to go to drone bootloader mode: 0xB4 0x79 0xE8

#ifdef CONFIG_UART2_BAUD_115200
#define MCU_UART2_BAUD 115200
#else
#define MCU_UART2_BAUD 9600
#endif

#if defined(CONFIG_MCU_MSG_PLD_16)
#define MCU_MSG_PLD_SIZ 16
#elif defined(CONFIG_MCU_MSG_PLD_14)
#define MCU_MSG_PLD_SIZ 14
#elif defined(CONFIG_MCU_MSG_PLD_12)
#define MCU_MSG_PLD_SIZ 12
#else
#define MCU_MSG_PLD_SIZ 10
#endif

#define MCU_FB_FW_IMAGE_FILE_NAME "fbfwimage.hex"

typedef struct _mcu_command{
	uint8_t Cmd;
	uint8_t Data[MCU_MSG_PLD_SIZ];
	uint8_t CheckSum;
}mcu_command_t;

void mcu_cmds_register(mcu_commands_func_t *cmdsrc);
mcu_cmd_counter_t *get_mcu_conter(void);
xTaskHandle get_mcu_recv_handle(void);
void mcu_boot_load(void);
void mcu_ctrl_init(void);
void mcu_ctrl_restart(void);

void test_pattern(void);
void mcu_recv_task(void *pvParameters);			//CSL
int sendflightplan(uint8_t* plandata);          //FJW
unsigned short mcu_get_flight_fwversion(void);

#endif	// _MCU_CTRL_H_
