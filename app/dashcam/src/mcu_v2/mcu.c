/**
 * @file
 * this is mcu file mcu.c
 * @author CJ
 */
 
#include <FreeRTOS.h>
#include <task.h>
#include <bsp.h>
#include <string.h>
#include <nonstdlib.h>
#include <queue.h>
#include <semphr.h>
#include <wifi/wifi_api.h>
#include <gpio/gpio.h>
#include <wdt/wdt.h>
#include <string.h>
#include <stdio.h>
#include <libmid_rtsp_server/rtsp_server.h>
#include <libmid_nvram/snx_mid_nvram.h>
#include <libmid_rec/avi_header.h>
#include <generated/snx_sdk_conf.h>
#include <libmid_mcu/mcu_ctrl.h>
#include <usb_device/usb_device.h>
#include <uart/uart.h>
#include "../main_flow/record/rec_schedule.h"
#include "../main_flow/playback/play_back.h"
#include "../main_flow/video/video_main.h"
#include "../main_flow/daemon/json_cmd.h"
#include "../main_flow/daemon/socket_ctrl.h"
#include "../main_flow/main_flow.h"
#include "mcu.h"
#include "utility.h"

#define MCU_DEBUG	0
#define APP_MCU_PRINT(fmt, args...) print_msg("[mcu]%s: "fmt, __func__,  ##args)
#define APP_MCU_PRINT_QUEUE(fmt, args...) print_msg_queue("[mcu]%s: "fmt, __func__,##args)
#define APP_MCU_DEBUG_PRINT(fmt, args...) if(MCU_DEBUG) \
	print_msg_queue("[mcu]%s: "fmt, __func__,##args)

#if 0
#define	TX_MODE_UART				1
#define	TX_STOP_BIT_2				1<<1
#define	TX_RDY						1<<2
#define	RX_MODE_UART				1<<4
#define	RX_RDY						1<<5
#define	RS485_EN					1<<7
#define	RS232_DMA_TX_EN				1<<8
#define	RS232_DMA_RX_EN				1<<9
#define	DMA_TX_BURST_MODE			1<<10
#define	DMA_RX_BURST_MODE			1<<11
#define	RS232_TX_INT_EN_BIT			1<<16
#define	RS232_RX_INT_EN_BIT			1<<17
#define	RS232_TX_INT_CLR_BIT		1<<18
#define	RS232_RX_INT_CLR_BIT		1<<19

#define	UART_CONFIG					0x0
#define	UART_CLOCK					0x0C
#define	RS_DATA						0x10
#define	FIFO_THD					0x18
#endif

#define SP_OFFSET_WIFI				0
#define SP_OFFSET_RECORD			2
#define SP_OFFSET_98660				4
#define SP_OFFSET_SENSOR			6

#define KS_WIFI_RST_FINISH			2
#define KS_SENSOR_RST_FINISH		4

#define	MCU_QUEUE_BUF				6
#define UART2_TIME_OUT				2000000

static gps_info_t gpsinfo;
static g_sensor_info_t g_sensorinfo;
static int g_sensor_sensitivity = 1000;
static uint8_t wifi_channelinfo = 0;			//CSL
static uint8_t fb_gpsinfo[10];				//CSL FJW
static uint8_t fb_navistatus[4];			//CSL FJW

QueueHandle_t McuCmdPlanDataQueue;			// FJW

enum SYNC_STATUS{
	SS_NORMAL = 0,
	SS_SYS_ABNORMAL,
	SS_SENSOR_FAIL,
	SS_WIFI_FAIL,
	SS_FIRMWARE_UPGRADE = 0x10,
};

enum SYNC_PARA_WIFI{
	SP_WIFI_OFF = 0,
	SP_WIFI_ON_NONLINK,
	SP_WIFI_ON_LINK,
	SP_WIFI_ON,
};

enum WIFI_STATUS{
	WIFI_OFF = 0,
	WIFI_ON,
};

enum SYNC_PARA_RECORD{
	SP_RECORD_WORKING = 0,
	SP_RECORD_NO_CARD,
	SP_RECORD_IN_PLAYBACK,
	SP_RECORD_STOP,
};

enum SYNC_PARA_98660{
	SP_660_NORMAL = 0,
	SP_660_SEN_ABNORMAL,
	SP_660_WIFI_ABNORMAL,
};

enum SYNC_PARA_SENSOR{
	SP_SEN_WORKING = 0,
	SP_SEN_STOP,
};

enum KEY_STATUS_RECORD_START{
	KS_RECORD_START_OK = 0,
	KS_RECORD_START_NO_CARD,
};

enum KEY_STATUS_RECORD_STOP{
	KS_RECORD_STOP_OK = 0,
	KS_RECORD_STOP_NO_CARD,
};

enum KEY_STATUS_SNAP_SHOT{
	KS_SNAP_SHOT_OK = 0,
	KS_SNAP_SHOT_FAIL,
};

enum KEY_STATUS_FILE_LOCK{
	KS_FILE_LOCK_OK = 0,
	KS_FILE_LOCK_FAIL,
};

enum KEY_STATUS_WIFI_ON{
	KS_WIFI_ON_OK = 0,
	KS_WIFI_ON_FAIL,
};

enum KEY_STATUS_WIFI_OFF{
	KS_WIFI_OFF_OK = 0,
	KS_WIFI_OFF_FAIL,
};

extern volatile int sd_card_remove;
//static QueueHandle_t McuCmdQueue;
uint32_t SysErrFlag = 0;
uint32_t WifiStatus = WIFI_ON;
uint32_t TaskRunFlag = 0;

//mcu_cmd_counter_t *get_mcu_command_counter()
//{
//	return cmd_cnt_t;
//}

/**
* @brief interface function - set error information to mcu.
* @param flag error number.
*/
void mcu_set_err_flag(enum SYSTEM_ERROR_FLAG flag)
{
	SysErrFlag |= (1 << flag);
	APP_MCU_PRINT_QUEUE("set mcu flag 0x%x....\n", flag);
}

/**
* @brief interface function - clear error flag.
* @param flag error number.
*/
void mcu_clear_err_flag(enum SYSTEM_ERROR_FLAG flag)
{
	SysErrFlag &= ~(1 << flag);
}

/**
* @brief interface function - reset flag to zero.
*/
void mcu_reset_err_flag(void)
{
	SysErrFlag = 0;
}


#define DETECT_SECOND			2000	//2000ms
#define RESET_DEFAULT_DELAY		100		//100ms

/**
* @brief Check gpio2 and do reset to default.
* @param pvParameters rtos task parameter.
*/
void reset_to_defult_task(void *pvParameters)
{
	gpio_pin_info info;
	int push_cnt = 0;

	info.pinumber = 2;
	info.mode = 0;
	info.value = 0;

	for(;;){
		snx_gpio_read(&info);

		if(info.value == 0){
			push_cnt++;
		}
		else{
			push_cnt = 0;
		}

		if(push_cnt == (DETECT_SECOND / RESET_DEFAULT_DELAY)){
			push_cnt = 0;
			APP_MCU_PRINT("\n\rStarting reset to default.\n");
			
			all_task_uinit(TASK_KEEP_NO_KEEP);
			
			//reset parameter
			if(snx_nvram_reset_to_default(NVRAM_RTD_ALL, NULL, NULL) != NVRAM_SUCCESS){
				APP_MCU_PRINT_QUEUE("Reset to default fail!!!!!\n");
			}
			
			APP_MCU_PRINT("\n\rStarting system reset.\n");
			//reset system by watch dog
			/*
			wdt_disable();
			wdt_intr_enable(0);
			wdt_clr_flag();
			wdt_setload(0);
			wdt_reset_enable(1);
			wdt_enable();
			*/
			
			//reset system by mcu
			mcu_set_err_flag(ALL_RESET);

			while(1);
		}

		vTaskDelay(RESET_DEFAULT_DELAY / portTICK_RATE_MS);
	}
	
	vTaskDelete(NULL);
}

unsigned char sync(unsigned char *syspara)
{
	uint8_t stats;

	*syspara = 0x00;
	stats = SS_NORMAL;

	//check error flag & 98660 status & sensor status
	if (SysErrFlag != 0) {
		if ((SysErrFlag & (1 << ERR_SENSOR_ISP)) != 0) {
			stats = SS_SENSOR_FAIL;
			*syspara |= (SP_660_SEN_ABNORMAL << SP_OFFSET_98660);
			*syspara |= (SP_SEN_STOP << SP_OFFSET_SENSOR);
		} else if ((SysErrFlag & (1 << ERR_WIFI)) != 0) {
				if (stats == SS_NORMAL) {
					stats = SS_WIFI_FAIL;
				} else {
					stats = SS_SYS_ABNORMAL;
				}

				*syspara |= (SP_660_WIFI_ABNORMAL << SP_OFFSET_98660);
				WifiStatus = WIFI_OFF;
		} else {
			stats = SS_SYS_ABNORMAL;
		}

		if ((SysErrFlag & (1 << FIRMWARE_UPGRADE)) != 0) {
			stats = SS_FIRMWARE_UPGRADE;
		}
	}

		//check wifi status
	if(WifiStatus != WIFI_OFF){
		if(socket_get_cur_connect_num() == 0){
			*syspara |= (SP_WIFI_ON_NONLINK << SP_OFFSET_WIFI);
		} else {
			*syspara |= (SP_WIFI_ON_LINK << SP_OFFSET_WIFI);
		}
	}

	//check record status
	if (sd_card_remove) {
		*syspara |= (SP_RECORD_NO_CARD << SP_OFFSET_RECORD);
	} else if(pb_state()) {
		*syspara |= (SP_RECORD_IN_PLAYBACK << SP_OFFSET_RECORD);
	} else if(schedrec_state() == 0) {
		*syspara |= (SP_RECORD_STOP << SP_OFFSET_RECORD);
	}

	return stats;
}

unsigned char recstart(void)
{
	uint8_t stats= 0 ;

	print_msg_queue("MCU : Rec Start\n");

	if ((sd_card_remove) || (json_get_format_status() == 1)) {
		stats = 0x01;	// Record start fail
	} else if (pb_state()) {
		stats = 0x01;	// Record start fail
	} else {
		schedrec_suspend_restart(1);
#ifndef CONFIG_APP_DRONE
		reclapse_suspend_restart(1);
#endif
		stats = 0x00;	// Record start OK
	}

	return stats;
}

unsigned char recstop(void)
{
	uint8_t stats = 0;

	print_msg_queue("MCU : Rec Stop\n");

	if ((sd_card_remove) || (json_get_format_status() == 1)) {
		stats = 0x01;	// Record start fail
	} else {
		user_diable_rec();
#ifndef CONFIG_APP_DRONE
		user_disable_reclapse();
#endif
		stats = 0x00;	// Record start OK
	}

	return stats;
}

unsigned char snapshot(void)
{
	uint8_t stats = 0;

	print_msg_queue("MCU : Snap Shot\n");

	if ((sd_card_remove) || (json_get_format_status() == 1)) {
		stats = KS_SNAP_SHOT_FAIL;
	} else {
		if (pb_state()) {
			print_msg_queue("Now in playback. Snapshot fail....\n");
		} else {
			set_takepic_num(1);
			mf_set_snapshot(1);
		}

		stats = KS_SNAP_SHOT_OK;
	}

	return stats;

}

unsigned char filelock(void)
{
	uint8_t stats = 0;

	print_msg_queue("MCU : File Lock\n");

	if ((sd_card_remove) || (json_get_format_status() == 1)) {
		stats = KS_FILE_LOCK_FAIL;
	} else {
		if (pb_state()) {
			print_msg_queue("Now in playback. File protect fail....\n");
		} else {
//CSL			set_trigger(1);
		}

		stats = KS_FILE_LOCK_OK;
	}

	return stats;
}

unsigned char wifion(void)
{
	uint8_t stats = 0;

	if (WifiStatus != WIFI_ON) {
		print_msg_queue("MCU : Wifi ON\n");
		WifiStatus = WIFI_ON;
	}
	stats = KS_WIFI_ON_OK;

	return stats;
}

unsigned char wifioff(void)
{
	uint8_t stats = 0;


	if(WifiStatus != WIFI_OFF){
		print_msg_queue("MCU : Wifi OFF\n");
		WifiStatus = WIFI_OFF;

		WiFi_Task_UnInit();
		vTaskDelay(1800 / portTICK_RATE_MS);
		print_msg("Uninit Wifi finish\n");
	}
	stats = KS_WIFI_OFF_OK;

	return stats;
}

unsigned char usb_detection(unsigned char a)
{
	uint8_t stats = 0;

	if (a == 0x02) { // High
		print_msg_queue("USB DETECTION High\n");
		usbd_set_ext_hotplug_state(1);
		stats = 0x00;
	} else if (a == 0x04) { // Low
		print_msg_queue("USB DETECTION Low\n");
		usbd_set_ext_hotplug_state(0);
		stats = 0x00;
	}

	return stats;
}

unsigned char emergency_lock(unsigned char a, unsigned char acceleration)
{
	uint8_t stats = 0;

	print_msg_queue("MCU : Emergency Lock\n");

	if ((sd_card_remove) || (json_get_format_status() == 1)) {
		stats = KS_FILE_LOCK_FAIL;
	} else {
		if (pb_state()) {
			print_msg_queue("Now in playback. File protect fail....\n");
		} else {
//CSL			set_trigger(1);
		}

		stats = KS_FILE_LOCK_OK;
	}

	return stats;
}

unsigned char gps_date(long long seconds, unsigned short speed)
{
	uint8_t stats = 0;
	system_date_t time;

	time_to_tm(seconds, 0, &time);

	gpsinfo.avi_gps->usGpsYear = time.year;
	gpsinfo.avi_gps->ucGpsMonth = time.month;
	gpsinfo.avi_gps->ucGpsDay = time.day;

	gpsinfo.avi_gps->ucGpsHour = time.hour;
	gpsinfo.avi_gps->ucGpsMinute = time.minute;
	gpsinfo.avi_gps->ucGpsSecond = time.second;

	print_msg_queue("==== GPS Date message ====\n");
	print_msg("year = %d\n", gpsinfo.avi_gps->usGpsYear);
	print_msg("month = %d\n", gpsinfo.avi_gps->ucGpsMonth);
	print_msg("day = %d\n", gpsinfo.avi_gps->ucGpsDay);
	print_msg("hour = %d\n", gpsinfo.avi_gps->ucGpsHour);
	print_msg("min = %d\n", gpsinfo.avi_gps->ucGpsMinute);
	print_msg("sec = %d\n", gpsinfo.avi_gps->ucGpsSecond);

	print_msg("speed = %d km\n", gpsinfo.avi_gps->usSpeed);

	gpsinfo.avi_gps->usSpeed = speed;

	return stats;
}

unsigned char gps_location(unsigned char dir_ns, double latitude, unsigned char dir_we, double longitude)
{
	uint8_t stats = 0;
	unsigned int lat = 0, lon = 0;
#if 0
	system_date_t date;
#endif
	gpsinfo.avi_gps->ucGPSStatus = 1;

	gpsinfo.avi_gps->ssLatDegInt = (short)(latitude / 100);
	lat = latitude * 10000;
	lat = lat - (gpsinfo.avi_gps->ssLatDegInt *  1000000);
	lat = lat *  1000;
	lat = lat / 6;
	if (dir_ns == 1) {	// S
		gpsinfo.avi_gps->ssLatDegInt = (-1) * gpsinfo.avi_gps->ssLatDegInt;
	}
	gpsinfo.avi_gps->ulLatDegDec = lat;

	gpsinfo.avi_gps->ssLonDegInt = (short)(longitude / 100);
	lon = longitude * 10000;
	lon = lon - (gpsinfo.avi_gps->ssLonDegInt * 1000000);
	lon = lon * 1000;
	lon = lon / 6;
	if (dir_we == 0) {	// W
		gpsinfo.avi_gps->ssLonDegInt = (-1) * gpsinfo.avi_gps->ssLonDegInt;
	}
	gpsinfo.avi_gps->ulLonDegDec = lon;

	gpsinfo.avi_gps->usAltitude = 0;
	//gpsinfo.avi_gps->usSpeed = speed;
#if 0
	get_date(&date);

	gpsinfo.avi_gps->usGpsYear = date.year;
	gpsinfo.avi_gps->ucGpsMonth = date.month;
	gpsinfo.avi_gps->ucGpsDay = date.day;
	gpsinfo.avi_gps->ucGpsHour = date.hour;
	gpsinfo.avi_gps->ucGpsMinute = date.minute;
	gpsinfo.avi_gps->ucGpsSecond = date.second;
#endif
	gpsinfo.sn++;
	if (gpsinfo.sn == 256)
		gpsinfo.sn = 0;
#if 1
	print_msg_queue("==== GPS Debug message ====\n");
	print_msg_queue("ssLatDegInt = %d\n", gpsinfo.avi_gps->ssLatDegInt);
	print_msg_queue("ulLatDegDec = %d\n", gpsinfo.avi_gps->ulLatDegDec);
	print_msg_queue("ssLonDegInt = %d\n", gpsinfo.avi_gps->ssLonDegInt);
	print_msg_queue("ulLonDegDec = %d\n", gpsinfo.avi_gps->ulLonDegDec);
	print_msg_queue("SN = %d\n", gpsinfo.sn);
	print_msg_queue("==== Debug message ===-\n");
#endif
	return stats;
}

void set_gsensor_sensitivity(int val)
{
	g_sensor_sensitivity = val;
}

unsigned char g_sensor_data(short x, short y, short z, unsigned char config)
{
	uint8_t stats = 0;
	int base_bit = 0, base_g = 0, quotient = 0;
	int tmp_x = 0, tmp_y = 0, tmp_z = 0;
#if 0
	int sensitivity = 0;

	if (snx_nvram_integer_get("App_ISP", "gsensorsensitivity", &sensitivity) != NVRAM_SUCCESS) {
		print_msg_queue("Get gsensorsensitivity faile\n");
		stats = 1;
	}
#endif
	//print_msg_queue("gsensorsensitivity = %d\n", g_sensor_sensitivity);

	switch (config & 0x3) {
	case 0:
		base_g = 2;
		g_sensorinfo.avi_g_sensor->ucAcceRange = 0;

		break;
	case 1:
		base_g = 4;
		g_sensorinfo.avi_g_sensor->ucAcceRange = 1;

		break;
	case 2:
		base_g = 8;
		g_sensorinfo.avi_g_sensor->ucAcceRange = 2;

		break;
	default:
		base_g = 2;
		g_sensorinfo.avi_g_sensor->ucAcceRange = 0;
		break;
	}

	switch ((config >> 2) & 0x3) {
	case 0:
		base_bit = 128;

		break;
	case 1:
		base_bit = 2048;

		break;
	case 2:
		base_bit = 65536;

		break;

	default:
		base_bit = 2048;
		break;
	}

	switch ((config >> 4) & 0x3) {
	case 0:
		quotient = 100;
		break;
	case 1:
		quotient = 1000;
		break;
	case 2:
		quotient = 10000;
		break;
	case 3:
		quotient = 100000;
		break;
	default:
		quotient = 1000;
		break;
	}

	tmp_x = abs(x);
	tmp_y = abs(y);
	tmp_z = abs(z);


	if ((tmp_x * quotient * base_g) / base_bit >= g_sensor_sensitivity ||
			(tmp_y * quotient * base_g) / base_bit >= g_sensor_sensitivity ||
			(tmp_z * quotient * base_g) / base_bit >= g_sensor_sensitivity) {
		if (pb_state()) {
			print_msg_queue("Now in playback. File protect fail....\n");
		} else {
			print_msg_queue("G sensor sensitivity trigger\n");
//CSL			set_trigger(1);
		}
	}

	g_sensorinfo.avi_g_sensor->ucGVInt_X = (tmp_x * base_g) / base_bit;
	g_sensorinfo.avi_g_sensor->ulGVDec_X = (tmp_x * quotient * base_g) / base_bit;
	g_sensorinfo.avi_g_sensor->ulGVDec_X -=  (g_sensorinfo.avi_g_sensor->ucGVInt_X) * quotient;
	g_sensorinfo.avi_g_sensor->ulGVDec_X *= 100;
	if (x < 0) { // negative
		g_sensorinfo.avi_g_sensor->ucGVInt_X |= (0x1 << 7);
	}


	g_sensorinfo.avi_g_sensor->ucGVInt_Y = (tmp_y * base_g) / base_bit;
	g_sensorinfo.avi_g_sensor->ulGVDec_Y = (tmp_y * quotient * base_g) / base_bit;
	g_sensorinfo.avi_g_sensor->ulGVDec_Y -=  (g_sensorinfo.avi_g_sensor->ucGVInt_Y) * quotient;
	g_sensorinfo.avi_g_sensor->ulGVDec_Y *= 100;
	if (y < 0) {
		g_sensorinfo.avi_g_sensor->ucGVInt_Y |= (0x1 << 7);
	}


	g_sensorinfo.avi_g_sensor->ucGVInt_Z = (tmp_z * base_g)/ base_bit;
	g_sensorinfo.avi_g_sensor->ulGVDec_Z = (tmp_z * quotient * base_g) / base_bit;
	g_sensorinfo.avi_g_sensor->ulGVDec_Z -=  (g_sensorinfo.avi_g_sensor->ucGVInt_Z) * quotient;
	g_sensorinfo.avi_g_sensor->ulGVDec_Z *= 100;
	if (z < 0) {
		g_sensorinfo.avi_g_sensor->ucGVInt_Z |= (0x1 << 7);
	}

#if 0
	// X
	tmp_x = abs(x) * (base_g / base_bit);
	g_sensorinfo.avi_g_sensor->ucGVInt_X = tmp_x / 1000000;
	g_sensorinfo.avi_g_sensor->ulGVDec_X = tmp_x % 1000000;
	if (x < 0) { // negative
		g_sensorinfo.avi_g_sensor->ucGVInt_X |= (0x1 << 7);
	}

	// Y
	tmp_y = abs(y) * (base_g / base_bit);
	g_sensorinfo.avi_g_sensor->ucGVInt_Y = tmp_y / 1000000;
	g_sensorinfo.avi_g_sensor->ulGVDec_Y = tmp_y % 1000000;
	if (y < 0) {
		g_sensorinfo.avi_g_sensor->ucGVInt_Y |= (0x1 << 7);
	}

	// Z
	tmp_z = abs(z) * (base_g / base_bit);
	g_sensorinfo.avi_g_sensor->ucGVInt_Z = tmp_z / 1000000;
	g_sensorinfo.avi_g_sensor->ulGVDec_Z = tmp_z % 1000000;
	if (z < 0) {
		g_sensorinfo.avi_g_sensor->ucGVInt_Z |= (0x1 << 7);
	}
#endif
	g_sensorinfo.sn++;
	if (g_sensorinfo.sn == 256)
		g_sensorinfo.sn = 0;
#if 0
	print_msg_queue("==== G Sensor Debug message ===-\n");
	print_msg_queue("ucGVInt_X = %d\n", g_sensorinfo.avi_g_sensor->ucGVInt_X);
	print_msg_queue("ulGVDec_X = %d\n", g_sensorinfo.avi_g_sensor->ulGVDec_X);

	print_msg_queue("ucGVInt_Y = %d\n", g_sensorinfo.avi_g_sensor->ucGVInt_Y);
	print_msg_queue("ulGVDec_Y = %d\n", g_sensorinfo.avi_g_sensor->ulGVDec_Y);

	print_msg_queue("ucGVInt_Z = %d\n", g_sensorinfo.avi_g_sensor->ucGVInt_Z);
	print_msg_queue("ulGVDec_Z = %d\n", g_sensorinfo.avi_g_sensor->ulGVDec_Z);
#endif
	return stats;
}

mcu_commands_func_t cmd_t = {
	.cmd_sync = sync,
	.cmd_record_start = recstart,
	.cmd_record_stop = recstop,
	.cmd_snap_shot = snapshot,
	.cmd_file_lock = filelock,
	.cmd_wifi_off = wifioff,
	.cmd_wifi_on = wifion,
	.cmd_usb_detection = usb_detection,
	.cmd_emergency_lock =  emergency_lock,
	.cmd_gps_location = gps_location,
	.cmd_gps_date = gps_date,
	.cmd_g_sensor_data = g_sensor_data,
	.cmd_read_wifichannel = wifi_readchannelinfo,			//CSL
	.cmd_fbgps_info = fbgps_getinof,						//CSL FJW
	.cmd_navigation_status = fbnavi_getstatus,				//CSL FJW
};

//CSL start
void wifi_channelinfoset(int val)
{
	wifi_channelinfo = val;
}

int wifi_readchannelinfo(void)
{
	return wifi_channelinfo;
}

uint8_t fbgps_getinof(uint8_t* pfbgps)
{
	//todo 
	//print_msg("%s %s %d\n",__FILE__,__FUNCTION__,__LINE__);
	memcpy(fb_gpsinfo,pfbgps,sizeof(fb_gpsinfo));
	return 0;
}

uint8_t fbnavi_getstatus(uint8_t* pfbnavi)
{
	//todo 
	
	//print_msg("%s %s %d\n",__FILE__,__FUNCTION__,__LINE__);
	memcpy(fb_navistatus,pfbnavi,sizeof(fb_navistatus));
	return 0;
}

uint8_t* app_getfbgps(void)
{
	
	//print_msg("%s %s %d\n",__FILE__,__FUNCTION__,__LINE__);
	return fb_gpsinfo;
}

uint8_t* app_getfbnavista(void)
{
	
	//print_msg("%s %s %d\n",__FILE__,__FUNCTION__,__LINE__);
	return fb_navistatus;
}

uint8_t* app_sendflightplan_tofb(uint8_t* plandata,int datalen)
{
	//todo
	int index = 0;
	uint8_t data[10];
	int len = 0;
	int rescode;
	
	//print_msg("%s %s %d\n",__FILE__,__FUNCTION__,__LINE__);
	while(index<datalen)
	{
		memset(data,0,sizeof(data));
		len = ((index+10)<datalen)?10:(datalen-index);
		memcpy(data,plandata+index,len);
		index += len;
		sendflightplan(data);
		xQueueReceive(McuCmdPlanDataQueue, &rescode, portMAX_DELAY);
		
	}
	
	return plandata;
}


//CSL end


unsigned char empty_func()
{
	uint8_t stats;
	stats = SS_NORMAL;
	return stats;
}

unsigned char empty_usb_detection(unsigned char s)
{
	uint8_t stats;
	stats = SS_NORMAL;
	return stats;
}

unsigned char empty_emergency_lock(unsigned char a, unsigned char acceleration)
{
	uint8_t stats;
	stats = SS_NORMAL;
	return stats;
}

unsigned char empty_gps_location(unsigned char dir_ns, double latitude, unsigned char dir_we, double longitude)
{
	uint8_t stats;
	stats = SS_NORMAL;
	return stats;
}

unsigned char empty_gps_date(long long seconds, unsigned short speed)
{
	uint8_t stats;
	stats = SS_NORMAL;
	return stats;
}

unsigned char empty_g_sensor_data(short x, short y, short z, unsigned char config)
{
	uint8_t stats;
	stats = SS_NORMAL;
	return stats;
}

mcu_commands_func_t empty_cmd_t = {
	.cmd_sync = sync,
	.cmd_record_start = empty_func,
	.cmd_record_stop = empty_func,
	.cmd_snap_shot = empty_func,
	.cmd_file_lock = empty_func,
	.cmd_wifi_off = empty_func,
	.cmd_wifi_on = empty_func,
	.cmd_usb_detection = empty_usb_detection,
	.cmd_emergency_lock =  empty_emergency_lock,
	.cmd_gps_location = empty_gps_location,
	.cmd_gps_date = empty_gps_date,
	.cmd_g_sensor_data = empty_g_sensor_data,
};

void app_mcu_cmds_register_empty(void)
{
	mcu_cmds_register(&empty_cmd_t);
}

g_sensor_info_t *get_g_sensor_info_table()
{
	return &g_sensorinfo;
}

AVIGSENSORINFO_t *get_avi_g_sensor_info()
{
	return g_sensorinfo.avi_g_sensor;
}

unsigned short get_g_sensor_sn()
{
	return g_sensorinfo.sn;
}

gps_info_t *get_gps_info_table()
{
	return &gpsinfo;
}

AVIGPSINFO_t *get_avi_gps_info()
{
	return gpsinfo.avi_gps;
}

unsigned short get_gps_sn()
{
	return gpsinfo.sn;
}

//CSL start
void task_read_info( void *pvParameters )
{
	while(1)
	{
		int channel_idx;
		
//		vTaskDelay(300/portTICK_PERIOD_MS);
		if (snx_nvram_integer_get("WIFI_DEV", "AP_CHANNEL_INFO", &channel_idx) == NVRAM_SUCCESS)	
		{
			APP_MCU_PRINT_QUEUE("nvram read %u\n", channel_idx);
			
			wifi_channelinfoset(channel_idx);

			vTaskDelete(NULL);
		}
		
		APP_MCU_PRINT_QUEUE("nvram read fail\n");
	}
}
//CSL end
/**
* @brief initial mcu task and queue.
*/
void mcu_init(void)
{
	SysErrFlag = 0;
	WifiStatus = WIFI_ON;
	TaskRunFlag = 0;
	//mcu_commands_func_t cmd_t;

	// Init GPS information table
	gpsinfo.avi_gps = pvPortMalloc(sizeof(AVIGPSINFO_t), GFP_KERNEL, MODULE_APP);
	if(gpsinfo.avi_gps == NULL) {
		APP_MCU_PRINT_QUEUE("alloc gps memory failed (size = %d)\n", sizeof(AVIGPSINFO_t));	
	}
	gpsinfo.sn = 0;

	g_sensorinfo.avi_g_sensor = pvPortMalloc(sizeof(AVIGSENSORINFO_t), GFP_KERNEL, MODULE_APP);
	if(g_sensorinfo.avi_g_sensor == NULL) {
		APP_MCU_PRINT_QUEUE("alloc gsensor memory failed (size = %d)\n", sizeof(AVIGPSINFO_t));	
	}
	g_sensorinfo.sn = 0;

	// Moved into mcu_ctrl_init();
	//uart2_init(MCU_UART2_BAUD);
	mcu_ctrl_init();

	mcu_cmds_register(&cmd_t);
	// FJW start
	McuCmdPlanDataQueue = xQueueCreate(1, sizeof(int));
	if(McuCmdPlanDataQueue == NULL){
		APP_MCU_PRINT_QUEUE("McuCmdPlanDataQueue create fail\n");
		return;
	}
	// FJW end
	/*if (snx_nvram_integer_get("App_ISP", "gsensorsensitivity", &g_sensor_sensitivity) != NVRAM_SUCCESS) {
		print_msg_queue("Get gsensorsensitivity faile\n");
	}*/

	if(pdPASS != xTaskCreate(reset_to_defult_task, "reset_default", STACK_SIZE_1K, (void*)NULL, PRIORITY_TASK_SYS_RESET_DEFAULT, NULL)){
		APP_MCU_PRINT_QUEUE("Could not create reset_default.\n");
	}
	//CSL start
	if (pdPASS != xTaskCreate(task_read_info, "task_read_info", STACK_SIZE_1K, (void*)NULL,PRIORITY_TASK_SYS_RESET_DEFAULT, NULL)){
		APP_MCU_PRINT_QUEUE("Could not create task_read_info\r\n");
	}
	//CSL end
}

/**
* @brief uninitial mcu task and queue.
*/
void mcu_uninit(void)
{
//	vTaskDelete(reset_to_defult_task);		//CSL
//	mcu_cmds_register(NULL);				//CSL
}
