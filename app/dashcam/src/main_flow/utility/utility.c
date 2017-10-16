/*********************************************************************************
* /Cstreamer_main.c
*
* Implementation of Utility Apis
*
* History:
*    2016/07/06 - [FuKuei Chang] created file
*
*
* Copyright (C) 1996-2016, Sonix, Inc.
*
* All rights reserved. No Part of this file may be reproduced, stored
* in a retrieval system, or transmitted, in any form, or by any means,
* electronic, mechanical, photocopying, recording, or otherwise,
* without the prior consent of Sonix, Inc.
*
*********************************************************************************/
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <nonstdlib.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys_clock.h>
#include <sglib.h>
#include <libmid_fatfs/ff.h>
#include <libmid_isp/snx_mid_isp.h>
#include <libmid_nvram/snx_mid_nvram.h>
#include <libmid_automount/automount.h>
#include <libmid_rtsp_server/rtsp_server.h>
#include <generated/snx_sdk_conf.h>
#include <libmid_fwupgrade/fwupgrade.h>
#include <libmid_usbd/mid_usbd.h>
#ifdef CONFIG_MODULE_RTC_SUPPORT
#include <rtc/rtc.h>
#endif
#include "../streammgr/upload1.h"
#include "rec_common.h"
#include "rec_schedule.h"
#include "../playback/play_back.h"
#include "video_main.h"
//#include "socket_ctrl.h"
#include "snapshot.h"
#include "audio_main.h"
#include "lwip/inet.h"
#include <wifi/wifi_api.h>
#include "../sensor_cap/sensor_capability.h"
//#include "online_fw_upgrade.h"
#include "user_param.h"
#include "main_flow.h"
#include "watch_task.h"
#include "rec_seed.h"
#include "utility.h"
#include "../errno.h"
#include "json_cmd.h"

#ifndef CONFIG_APP_DRONE
#include "mcu.h"
#endif

#include <libmid_sd/mid_sd.h>

/**
 * Macro or defines
 */

#define UTIL_PRINT(level, fmt, args...) print_q(level, "[utility]%s: "fmt, __func__,##args)


extern int conn_list_len(ConnItem_t **plist);
extern ConnItem_t *playback_list;
//extern ConnItem_t *preview_list;
extern char g_str_timezone[10];

//task manager
int g_reboot_task_running = 0;
int g_sd_format_running = 0;
int g_format_status = 0;
int g_sd_test_running = 0;
int g_test_status = 0;
int g_fw_upgrade_status = 0;
xSemaphoreHandle g_util_mutex = NULL;

time_zone_tbl_t timezone_tbl[] = { {"GMT-12", 0},   {"GMT-11", 1},  {"GMT-10", 2}, 	{"GMT-9.5", 3}, {"GMT-9", 4},
	{"GMT-8", 5},    {"GMT-7", 6},   {"GMT-6", 7}, 	{"GMT-5", 8},  	{"GMT-4", 9},
	{"GMT-3.5", 10},  {"GMT-3", 11},   {"GMT-2", 12}, 	{"GMT-1", 13}, 	{"GMT+0", 14},
	{"GMT+1", 15},    {"GMT+2", 16},   {"GMT+3", 17}, 	{"GMT+3.5", 18},
	{"GMT+4", 19},    {"GMT+4.5", 20}, {"GMT+5", 21},     {"GMT+5.5", 22}, {"GMT+5.75", 23},
	{"GMT+6", 24},    {"GMT+6.5", 25}, {"GMT+7", 26},     {"GMT+8", 27},	{"GMT+8.5", 28},
	{"GMT+8.75", 29}, {"GMT+9", 30},   {"GMT+9.5", 31},   {"GMT+10", 32},	{"GMT+10.5", 33},
	{"GMT+11", 34},  {"GMT+12", 35},   {"GMT+12.75", 36}, {"GMT+13", 37},	{"GMT+14", 38}
};

static void reboot_task(void *args)
{

	UTIL_PRINT(SYS_INFO, "wait for 3 seconds, the system will reboot\n");
	vTaskDelay(3000 / portTICK_PERIOD_MS);
	all_task_uinit(0);
	if(snx_nvram_reset_to_default(NVRAM_RTD_ALL, NULL, NULL) != NVRAM_SUCCESS) {
		UTIL_PRINT(SYS_ERR, "NVRAM Reset to default fail!!!!! \n");
	}
	reboot();

	g_reboot_task_running = 0;
	vTaskDelete(NULL);
}

static void start_access_sdcard_fun(void)
{
	mid_sd_remount();
	mf_snapshot_init();
	mf_thumbnail_init();
	init_record_task();
#ifndef CONFIG_APP_DRONE
	reclapse_init_task();
	init_protect_task();
#endif
	mf_video_set_record_cb(recordvideo);
#ifndef CONFIG_APP_DRONE
	mf_audio_set_record_cb(recordaudio);
#endif
}

static void close_access_sdcard_fun(void)
{
	//mf_video_set_record_cb(NULL);   //where is  frame disable ,  record cannot recevie i-fame , and record_start can not OK,sometimes
	//mf_audio_set_record_cb(NULL);
#ifndef CONFIG_APP_DRONE
	set_protect_to_closing(1);
	if(get_sd_umount_err() == 1) {
		UTIL_PRINT(SYS_ERR, "ProtectMUTEX\n");
		unlock_protect_mutex();
	}
	wait_protect_task_closed();
#endif
	set_record_to_closing(1);
	if(get_sd_umount_err() == 1) {
		UTIL_PRINT(SYS_ERR, "RecordMUTEX\n");
		unlock_record_mutex();
	}
	wait_record_task_closed();
#ifndef CONFIG_APP_DRONE
	set_lapse_to_closing(1);
	if(get_sd_umount_err() == 1) {
		UTIL_PRINT(SYS_ERR, "TimelapseRecordMUTEX\n");
		unlock_timelapse_mutex();
	}
	wait_lapse_task_closed();
#endif
	mf_snapshot_uninit();
	mf_thumbnail_uninit();

}

int app_uti_chk_sd_state(void)
{
	int result = 0;
	
	if (get_sd_umount_err()) {
		result = get_sd_status();
		if( result == FR_NO_FILESYSTEM ) { //have sdcard but not fat32
			UTIL_PRINT(SYS_ERR, "SD Card Format Error\n");
			return SD_TYPE_ERR;
		} else {
			UTIL_PRINT(SYS_ERR, "SD Card Doesn't Exist\n");
			return SD_NOT_EXIST;
		}
	}

	if (g_sd_format_running == 1) {
		UTIL_PRINT(SYS_ERR, "SD Card Is Formatting Now.\n");
		return SD_IS_FORMATTING;
	}
	else if (g_sd_test_running == 1) {
		UTIL_PRINT(SYS_ERR, "SD Card Is Testing Now.\n");
		return SD_IS_TESTING;
	}
	else {}

	return SD_OK;
}

/***
static int get_filesize(const char* fname)
{
	FILINFO finfo;
	char lfn[ _MAX_LFN + 1 ];
	//char szTmp[10]={0};
	char *path0 = (char *)fname;
	finfo.lfname = lfn;
	finfo.lfsize = _MAX_LFN + 1;
	if(f_stat( path0, & finfo ) == FR_OK)
	{
		UTIL_PRINT(SYS_INFO, "(finfo.fsize)===%d byte \n",(finfo.fsize));
		return (finfo.fsize);
	}
	else
	{
		UTIL_PRINT(SYS_ERR, "read file size error\n");
		return GET_FILE_SIZE_ERR;
	}
}
***/

int app_util_init(void)
{

	if( g_util_mutex == NULL ) {
		if( !(g_util_mutex = xSemaphoreCreateMutex()) ) {
			UTIL_PRINT(SYS_ERR, "could not create mutex \n");
			return pdFAIL;
		}
	}

	return pdPASS;
}

int app_util_uninit(void)
{
	if( g_util_mutex != NULL ) {
		vSemaphoreDelete(g_util_mutex);
		g_util_mutex = NULL;
	}

	return pdPASS;
}

int app_uti_chk_folder_path_len(int cur_name_len)
{
	int ret = 0;
	struct usr_config *pUserCfg = NULL;

	if ((ret = get_usr_config(&pUserCfg)) != 0) {
		UTIL_PRINT(SYS_ERR, "get usr config failed\n");
		return (-1);
	}
	
	if (((pUserCfg->root_folder_len + cur_name_len + 3) > FILE_DIR_LEN) || //picutre folder len 
		((pUserCfg->root_folder_len + pUserCfg->rec_folder_len + cur_name_len + 4) > FILE_DIR_LEN) ||
		((pUserCfg->root_folder_len + pUserCfg->protect_folder_len + cur_name_len + 4) > FILE_DIR_LEN) ||
		((pUserCfg->root_folder_len + pUserCfg->timelapse_folder_len + cur_name_len + 4) > FILE_DIR_LEN)) 
	{
		UTIL_PRINT(SYS_ERR, "folder path is over thr max path length.\n");
		return (-1);
	}
	
	return ret;

}


int app_uti_set_sd_dir_info(int type, char *name)
{
	int ret = 0;
	int nvram_err = NVRAM_SUCCESS;
	char dir_name[FILE_DIR_LEN] = {0};
	char dir_type[32] = {0};
	int setting_name_len = 0;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	setting_name_len = strlen(name);

	if ((setting_name_len >= 0) && (setting_name_len <= FILE_DIR_LEN)) 
	{
		if (app_uti_chk_folder_path_len(setting_name_len) != 0) {
			ret = OVER_MAX_FOLDER_LEN;
			goto finally;
		}

		switch(type)
		{
			case U_SD_ROOT:
				snprintf(dir_type, sizeof(dir_type), "%s", NVRAM_SD_ROOT);
				break;
			case U_SD_RECORD:
				snprintf(dir_type, sizeof(dir_type), "%s", NVRAM_SD_RECORD);
				break;
			case U_SD_PROTECT:
				snprintf(dir_type, sizeof(dir_type), "%s", NVRAM_SD_PROTECT);
				break;
			case U_SD_PICTURE:
				snprintf(dir_type, sizeof(dir_type), "%s", NVRAM_SD_PICTURE);
				break;
			case U_SD_TIMELAPSE:
				snprintf(dir_type, sizeof(dir_type), "%s", NVRAM_SD_TIMELAPSE);
				break;
			case U_SD_THUMBNAIL:
				snprintf(dir_type, sizeof(dir_type), "%s", NVRAM_SD_THUMBNAIL);
				break;
			default:
				UTIL_PRINT(SYS_ERR, "Invalid type(%d)\n", type);
				ret = INVALID_PARAM;
				goto finally;
		}
		
		memset(dir_name, 0x0, sizeof(dir_name));
		snprintf(dir_name, strlen(name), name);
		
		if ( (nvram_err = snx_nvram_string_set(NVRAM_PATH_INFO, dir_type, dir_name) ) != NVRAM_SUCCESS)
		{
			UTIL_PRINT(SYS_ERR, "SET Flash Fail(%d)\n", nvram_err);
			ret = NVRAM_ERR;
			goto finally;
		}

	}
	else
	{
		UTIL_PRINT(SYS_ERR, "Invalid Parameters\n");
		ret = INVALID_PARAM;
		goto finally;
	}

finally:
	xSemaphoreGive(g_util_mutex);
	return ret;
}

int app_uti_get_root_dir_name(char *name)
{
	int ret = 0;
	int nvram_err = NVRAM_SUCCESS;
	char sd_root[FILE_DIR_LEN] = {0};

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if ( (nvram_err = snx_nvram_string_get(NVRAM_PATH_INFO, NVRAM_SD_ROOT, sd_root) ) != NVRAM_SUCCESS)
	{
		UTIL_PRINT(SYS_ERR, "GET Flash Fail(%d)\n", nvram_err);
		ret = NVRAM_ERR;
		goto finally;
	}
	snprintf(name, sizeof(sd_root), sd_root);
	UTIL_PRINT(SYS_DBG, "root: %s\n", name);

finally:
	xSemaphoreGive(g_util_mutex);
	return ret;
}

int app_uti_get_record_dir_name(char *name)
{
	int ret = 0;
	int nvram_err = NVRAM_SUCCESS;
	char sd_record[FILE_DIR_LEN] = {0};

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if ( (nvram_err = snx_nvram_string_get(NVRAM_PATH_INFO, NVRAM_SD_RECORD, sd_record) ) != NVRAM_SUCCESS)
	{
		UTIL_PRINT(SYS_ERR, "GET Flash Fail(%d)\n", nvram_err);
		ret = NVRAM_ERR;
		goto finally;
	}
	snprintf(name, sizeof(sd_record), sd_record);
	UTIL_PRINT(SYS_DBG, "record: %s\n", name);

finally:
	xSemaphoreGive(g_util_mutex);
	return ret;
}

int app_uti_get_protect_dir_name(char *name)
{
	int ret = 0;
	int nvram_err = NVRAM_SUCCESS;
	char sd_protect[FILE_DIR_LEN] = {0};

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if ( (nvram_err = snx_nvram_string_get(NVRAM_PATH_INFO, NVRAM_SD_PROTECT, sd_protect) ) != NVRAM_SUCCESS)
	{
		UTIL_PRINT(SYS_ERR, "GET Flash Fail(%d)\n", nvram_err);
		ret = NVRAM_ERR;
		goto finally;
	}
	snprintf(name, sizeof(sd_protect), sd_protect);
	UTIL_PRINT(SYS_ERR, "protect: %s\n", name);

finally:
	xSemaphoreGive(g_util_mutex);
	return ret;
}

int app_uti_get_picture_dir_name(char *name)
{
	int ret = 0;
	int nvram_err = NVRAM_SUCCESS;
	char sd_picture[FILE_DIR_LEN] = {0};

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if ( (nvram_err = snx_nvram_string_get(NVRAM_PATH_INFO, NVRAM_SD_PICTURE, sd_picture) ) != NVRAM_SUCCESS)
	{
		UTIL_PRINT(SYS_ERR, "GET Flash Fail(%d)\n", nvram_err);
		ret = NVRAM_ERR;
		goto finally;
	}
	snprintf(name, sizeof(sd_picture), sd_picture);
	UTIL_PRINT(SYS_DBG, "pic: %s\n", name);

finally:
	xSemaphoreGive(g_util_mutex);
	return ret;
}

int app_uti_get_timelapse_dir_name(char *name)
{
	int ret = 0;
	int nvram_err = NVRAM_SUCCESS;
	char sd_timelapse[FILE_DIR_LEN] = {0};

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if ( (nvram_err = snx_nvram_string_get(NVRAM_PATH_INFO, NVRAM_SD_TIMELAPSE, sd_timelapse) ) != NVRAM_SUCCESS)
	{
		UTIL_PRINT(SYS_ERR, "GET Flash Fail(%d)\n", nvram_err);
		ret = NVRAM_ERR;
		goto finally;
	}
	snprintf(name, sizeof(sd_timelapse), sd_timelapse);
	UTIL_PRINT(SYS_DBG, "timelapse: %s\n", name);

finally:
	xSemaphoreGive(g_util_mutex);
	return ret;
}

int app_uti_get_thumbnail_dir_name(char *name)
{
	int ret = 0;
	int nvram_err = NVRAM_SUCCESS;
	char sd_thumbnail[FILE_DIR_LEN] = {0};

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if ( (nvram_err = snx_nvram_string_get(NVRAM_PATH_INFO, NVRAM_SD_THUMBNAIL, sd_thumbnail) ) != NVRAM_SUCCESS)
	{
		UTIL_PRINT(SYS_ERR, "GET Flash Fail(%d)\n", nvram_err);
		ret = NVRAM_ERR;
		goto finally;
	}
	snprintf(name, sizeof(sd_thumbnail), sd_thumbnail);
	UTIL_PRINT(SYS_DBG, "thumbnail: %s\n", name);

finally:
	xSemaphoreGive(g_util_mutex);
	return ret;
}

//************************************************
//			WI-FI APIs
//************************************************
/***
int app_uti_set_wifi_channel(int channel)
{
	int ret = 0;
	int nvram_err = NVRAM_SUCCESS;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if( (channel >= 1) && (channel <= 11) )
	{
		if ( (nvram_err = snx_nvram_integer_set("WIFI_DEV", "CHANNEL_INFO", channel) ) != NVRAM_SUCCESS)
		{
			UTIL_PRINT(SYS_ERR, "Save Flash Fail(%d)\n", nvram_err);
			ret = NVRAM_ERR;
			goto finally;
		}

		set_channel_bit(1);
	}
	else
	{
		UTIL_PRINT(SYS_ERR, "Invalid Parameters\n");
		ret = INVALID_PARAM;
		goto finally;
	}

finally:
	xSemaphoreGive(g_util_mutex);
	return ret;
}

int app_uti_set_wifi_pwd(char* pwd)
{
	int ret = 0;
	int nvram_err = NVRAM_SUCCESS;
	int keylen = 0;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if( strlen(pwd) == WIFI_PWD_LENTH )
	{
		if( (nvram_err = snx_nvram_string_set("WIFI_DEV", "KEY_INFO", pwd) ) != NVRAM_SUCCESS )
		{
			 UTIL_PRINT(SYS_ERR, "Save Flash Fail(%d)\n", nvram_err);
			 ret = NVRAM_ERR;
			 goto finally;
		}

		keylen = WIFI_PWD_LENTH;

		if( (nvram_err = snx_nvram_integer_set("WIFI_DEV", "KEY_LEN", keylen) ) != NVRAM_SUCCESS )
		{
			UTIL_PRINT(SYS_ERR, "Save Flash Fail(%d)\n", nvram_err);
			ret = NVRAM_ERR;
			goto finally;
		}
		set_pwd_bit(1);
	}
	else
	{
		UTIL_PRINT(SYS_ERR, "PWD lenth isn't equal 5\n");
		ret = INVALID_PARAM;
		goto finally;
	}

finally:

	xSemaphoreGive(g_util_mutex);

	return ret;
}

int app_uti_set_wifi_ssid(char* ssid)
{
	int ret = 0;
	int nvram_err = NVRAM_SUCCESS;
	int ssidlen = 0;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	ssidlen = strlen(ssid);
	if( ssidlen <= WIFI_SSID_MAX_LENTH )
	{
		if ( (nvram_err = snx_nvram_string_set("WIFI_DEV", "SSID_INFO", ssid) ) != NVRAM_SUCCESS )
		{
			UTIL_PRINT(SYS_ERR, "Save Flash Fail(%d)\n", nvram_err);
			ret = NVRAM_ERR;
			goto finally;
		}

		if ( (nvram_err = snx_nvram_integer_set("WIFI_DEV", "SSID_LEN", ssidlen) )  != NVRAM_SUCCESS )
		{
			UTIL_PRINT(SYS_ERR, "Save Flash Fail(%d)\n", nvram_err);
			ret = NVRAM_ERR;
			goto finally;
		}
		set_ssid_bit(1);
	}
	else
	{
		UTIL_PRINT(SYS_ERR, "SSID lenth is largest 32\n");
		ret = INVALID_PARAM;
		goto finally;
	}

finally:

	xSemaphoreGive(g_util_mutex);

	return ret;
}

#ifdef ENGINNER_MODE
int app_uti_set_wifi_param(char *ssid, char *pwd, int channel, int retry_count)
#else
int app_uti_set_wifi_param(char *ssid, char *pwd, int channel)
#endif
{
	int ret = 0;
	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if( ssid != NULL ) {
		int ssidlen = strlen(ssid);
		if( (ssidlen <= WIFI_SSID_MAX_LENTH) ) {
			if ( snx_nvram_string_set("WIFI_DEV", "SSID_INFO", ssid) != NVRAM_SUCCESS ) {
				UTIL_PRINT(SYS_ERR, "set wifi ssid nvram fail \n");
				ret = NVRAM_ERR;
				goto finally;
			}

			if ( snx_nvram_integer_set("WIFI_DEV", "SSID_LEN", ssidlen) != NVRAM_SUCCESS ) {
				UTIL_PRINT(SYS_ERR, "set wifi ssidlen nvram fail\n");
				ret = NVRAM_ERR;
				goto finally;
			}
			set_ssid_bit(1);
		}else {
			UTIL_PRINT(SYS_ERR, "SSID lenth is largest 32\n");
			ret = INVALID_PARAM;
			goto finally;
		}
	}

	if( pwd != NULL ) {
		if( strlen(pwd) == WIFI_PWD_LENTH ) {
			if ( snx_nvram_string_set("WIFI_DEV", "KEY_INFO", pwd) != NVRAM_SUCCESS ) {
				UTIL_PRINT(SYS_ERR, "set wifi pwd nvram fail\n");
				ret = NVRAM_ERR;
				set_ssid_bit(0);
				goto finally;
			}
			int keylen = WIFI_PWD_LENTH;
			if ( snx_nvram_integer_set("WIFI_DEV", "KEY_LEN", keylen) != NVRAM_SUCCESS ) {
				UTIL_PRINT(SYS_ERR, "set wifi keylen nvram fail\n");
				ret = NVRAM_ERR;
				set_ssid_bit(0);
				goto finally;
			}
			set_pwd_bit(1);
		}else	{
			UTIL_PRINT(SYS_ERR, "PWD lenth isn't equal 5\n");
			ret = INVALID_PARAM;
			set_ssid_bit(0);
			goto finally;
		}
	}

	if( channel != -1 ) {
		if( snx_nvram_integer_set("WIFI_DEV", "CHANNEL_INFO", channel) != NVRAM_SUCCESS ) {
			UTIL_PRINT(SYS_ERR, "set wifi channel nvram fail\n");
			ret = NVRAM_ERR;
			set_pwd_bit(0);
			set_ssid_bit(0);
			goto finally;
		}
		set_channel_bit(1);
	}

	#ifdef ENGINNER_MODE
	if( retry_count != -1 ) {
		if( snx_nvram_integer_set("WIFI_DEV", "TX_RETRY", retry_count) != NVRAM_SUCCESS ) {
			UTIL_PRINT(SYS_ERR, "Save Flash Fail when setting tx retry count");
			ret = NVRAM_ERR;
			goto finally;
		}else {
			unsigned char unsgined_retry_count = retry_count;
			UTIL_PRINT(SYS_DBG, "tx retry_count=%d\n", unsgined_retry_count);
			WiFi_QueryAndSet(SET_TX_RETRY_COUNT, &unsgined_retry_count, NULL);
		}
	}
	#endif

finally:

	xSemaphoreGive(g_util_mutex);

	return ret;
}
***/
//************************************************
//			Video APIs
//************************************************

/***
void app_uti_get_preview_params(unsigned int *width, unsigned int *height, unsigned int *ucfps, unsigned int *ucgop, unsigned int *uibps)
{
	mf_video_resmap_get_preview_params( width, height, ucfps, ucgop, uibps );
}
***/

int app_uti_wid_hight_to_resolution(int width, int height)
{
	int resolution = 0;

	if( (width == FHD_WIDTH) && (height == FHD_HEIGHT) )
		resolution = 2;
	else if( (width == HD_WIDTH) && (height == HD_HEIGHT) )
		resolution = 1;
	else if( (width == VGA_WIDTH) && (height == VGA_HEIGHT) )
		resolution = 0;
	else {
		UTIL_PRINT(SYS_ERR, "The resolution is invalid\n");
		resolution = -1;
	}
	return resolution;
}

/***
int app_uti_check_all_task_is_running(void)
{
	int ret = OK;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if( chk_record_is_running() == 0 )
	{
		UTIL_PRINT(SYS_ERR, "Record Task have not init success\n");
		ret = TASK_HAVE_NOT_INIT_OK;
		goto finally;
	}
	if( chk_isp0_is_running() == 0 )
	{
		UTIL_PRINT(SYS_ERR, "ISP0 Task have not init success\n");
		ret = TASK_HAVE_NOT_INIT_OK;
		goto finally;
	}
	if( chk_isp1_is_running() == 0 )
	{
		UTIL_PRINT(SYS_ERR, "ISP1 Task have not init success\n");
		ret = TASK_HAVE_NOT_INIT_OK;
		goto finally;
	}
#ifndef CONFIG_APP_DRONE
	if( chk_protect_is_running() == 0 )
	{
		UTIL_PRINT(SYS_ERR, "Protect Task have not init success\n");
		ret = TASK_HAVE_NOT_INIT_OK;
		goto finally;
	}
	if( chk_lapse_is_running() == 0 )
	{
		UTIL_PRINT(SYS_ERR, "Timelapse Task have not init success\n");
		ret  = TASK_HAVE_NOT_INIT_OK;
		goto finally;
	}
#endif

finally:

	xSemaphoreGive(g_util_mutex);

	return ret;

}
***/

/***
int app_uti_check_resolution_change(int resolution, int *width, int *height)
{
	int resolution_chage = 0;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if( resolution == 0 ) { //vga
		if( (*width != VGA_WIDTH) || (*height != VGA_HEIGHT) ) {
			*width = (unsigned) VGA_WIDTH;
			*height = (unsigned) VGA_HEIGHT;
			resolution_chage = 1;
		}
	} else if (resolution == 1) { //hd
		if((*width != HD_WIDTH) || (*height != HD_HEIGHT) ) {
			*width = (unsigned) HD_WIDTH;
			*height = (unsigned) HD_HEIGHT;
			resolution_chage = 1;
		}
	} else if (resolution == 2) { //fhd
		if( (*width != FHD_WIDTH) || (*height != FHD_HEIGHT) ) {
			*width = (unsigned) FHD_WIDTH;
			*height = (unsigned) FHD_HEIGHT;
			resolution_chage = 1;
		}
	}

	xSemaphoreGive(g_util_mutex);

	return resolution_chage;
}
***/

int app_uti_set_video_ext_qp(int pre_ext_pframe_num, int pre_ext_qp_range, int pre_ext_qp_max, int pre_ext_upper_pframe, int pre_ext_upper_pframe_dup1, int pre_qp_max, int pre_qp_min)
{
	int ret = -1;
	int nvram_err = NVRAM_SUCCESS;

	if (pre_ext_pframe_num == -1 ||
		pre_ext_qp_range == -1 ||
		pre_ext_qp_max == -1 ||
		pre_ext_upper_pframe == -1 ||
		pre_ext_upper_pframe_dup1 == -1 ||
		pre_qp_max == -1 ||
		pre_qp_min == -1)
		return ret;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if(pre_ext_pframe_num != -1) {
		if ( (nvram_err = snx_nvram_integer_set(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_PREVIEW_EXT_PFRAME_NUM, pre_ext_pframe_num) ) != NVRAM_SUCCESS) {
			UTIL_PRINT(SYS_ERR, "Save Flash Fail(%d)\n", nvram_err);
			ret = NVRAM_ERR;
			goto finally;
		}
	}

	if(pre_ext_qp_range != -1) {
		if ( (nvram_err = snx_nvram_integer_set(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_PREVIEW_EXT_QP_RANGE, pre_ext_qp_range) ) != NVRAM_SUCCESS) {
			UTIL_PRINT(SYS_ERR, "Save Flash Fail(%d)\n", nvram_err);
			ret = NVRAM_ERR;
			goto finally;
		}
	}

	if(pre_ext_qp_max != -1) {
		if ( (nvram_err = snx_nvram_integer_set(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_PREVIEW_EXT_QP_MAX, pre_ext_qp_max) ) != NVRAM_SUCCESS) {
			UTIL_PRINT(SYS_ERR, "Save Flash Fail(%d)\n", nvram_err);
			ret = NVRAM_ERR;
			goto finally;
		}
	}

	if(pre_ext_upper_pframe != -1) {
		if ( (nvram_err = snx_nvram_integer_set(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_PREVIEW_EXT_UPPER_PFRAME, pre_ext_upper_pframe) ) != NVRAM_SUCCESS) {
			UTIL_PRINT(SYS_ERR, "Save Flash Fail(%d)\n", nvram_err);
			ret = NVRAM_ERR;
			goto finally;
		}
	}

	if(pre_ext_upper_pframe_dup1 != -1) {
		if ( (nvram_err = snx_nvram_integer_set(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_PREVIEW_EXT_UPPER_PFRAME_DUP1, pre_ext_upper_pframe_dup1) ) != NVRAM_SUCCESS) {
			UTIL_PRINT(SYS_ERR, "Save Flash Fail(%d)\n", nvram_err);
			ret = NVRAM_ERR;
			goto finally;
		}
	}

	if(pre_qp_max != -1) {
		if( pre_qp_max >= 50 ) {
			UTIL_PRINT(SYS_ERR, "max qp must smaller than 50(%d)\n", pre_qp_max);
			//pre_qp_max = 49;
			ret = INVALID_PARAM;
			goto finally;
		}
		if ( (nvram_err = snx_nvram_integer_set(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_PREVIEW_QP_MAX, pre_qp_max) ) != NVRAM_SUCCESS) {
			UTIL_PRINT(SYS_ERR, "Save Flash Fail(%d)\n", nvram_err);
			ret = NVRAM_ERR;
			goto finally;
		}
	}

	if(pre_qp_min != -1) {
		if( pre_qp_min <= 10 ) {
			UTIL_PRINT(SYS_ERR, "min qp must larger than 10(%d)\n", pre_qp_min);
			//pre_qp_min = 11;
			ret = INVALID_PARAM;
			goto finally;
		}

		if ( (nvram_err = snx_nvram_integer_set(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_PREVIEW_QP_MIN, pre_qp_min) ) != NVRAM_SUCCESS) {
			UTIL_PRINT(SYS_ERR, "Save Flash Fail(%d)\n", nvram_err);
			ret = NVRAM_ERR;
			goto finally;
		}
	}
	
	ret = 0;

finally:

	xSemaphoreGive(g_util_mutex);

	return ret;

}


int app_uti_set_video_preview_time(int resume,  int suspend)
{
	int ret = -1;
	int nvram_err = NVRAM_SUCCESS;

	if (resume == -1 || suspend == -1)
		return ret;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if(resume != -1) {
		if ( (nvram_err = snx_nvram_integer_set(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_PREVIEW_RESUME, resume) ) != NVRAM_SUCCESS) {
			UTIL_PRINT(SYS_ERR, "Save Flash Fail(%d)\n", nvram_err);
			ret = NVRAM_ERR;
			goto finally;
		}
	}

	if(suspend != -1) {
		if ( (nvram_err = snx_nvram_integer_set(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_PREVIEW_SUSPEND, suspend) ) != NVRAM_SUCCESS) {
			UTIL_PRINT(SYS_ERR, "Save Flash Fail(%d)\n", nvram_err);
			ret = NVRAM_ERR;
			goto finally;
		}
	}

	ret = 0;

finally:

	xSemaphoreGive(g_util_mutex);

	return ret;
}

int app_uti_set_video_wdr(int wdr_en)
{
	int ret = 0;
	int nvram_err = NVRAM_SUCCESS;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if( (wdr_en == 1) || (wdr_en == 0) ) {
		if( (nvram_err = snx_nvram_integer_set(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_VIDEO_WDR, wdr_en) ) != NVRAM_SUCCESS ) {
			UTIL_PRINT(SYS_ERR, "Save Flash Fail(%d)\n", nvram_err);
			ret = NVRAM_ERR;
			goto finally;
		}

		if( (ret = snx_isp_drc_status_set(wdr_en) ) != 0 ) {
			UTIL_PRINT(SYS_ERR, "Set Video WDR Data Error\n");
			ret = SET_WDR_ERR;
			goto finally;
		}
	} else {
		UTIL_PRINT(SYS_ERR, "Invalid Parameter\n");
		ret = INVALID_PARAM;
		goto finally;
	}

finally:
	xSemaphoreGive(g_util_mutex);
	return ret;

}

int app_uti_set_video_mirror(int mirror)
{
	int ret = 0;
	int nvram_err = NVRAM_SUCCESS;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if( (mirror == 1) || (mirror == 0) ) {
		if( (nvram_err = snx_nvram_integer_set(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_VIDEO_MIRROR, mirror) ) != NVRAM_SUCCESS ) {
			UTIL_PRINT(SYS_ERR, "Save Flash Fail(%d)\n", nvram_err);
			ret = NVRAM_ERR;
			goto finally;
		}

		if( (ret = snx_isp_sensor_mirror_set(mirror) ) != 0 ) {
			UTIL_PRINT(SYS_ERR, "Set Video Mirror Error\n");
			ret = SET_MIRROR_ERR;
			goto finally;
		}

	} else {
		UTIL_PRINT(SYS_ERR, "Invalid Parameter\n");
		ret = INVALID_PARAM;
		goto finally;
	}

finally:

	xSemaphoreGive(g_util_mutex);

	return ret;

}

int app_uti_set_video_flip(int flip)
{
	int ret = 0;
	int nvram_err = NVRAM_SUCCESS;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if( (flip == 0) || (flip == 1) ) {
		if( (nvram_err = snx_nvram_integer_set(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_VIDEO_FLIP, flip) ) != NVRAM_SUCCESS ) {
			UTIL_PRINT(SYS_ERR, "Save Flash Fail(%d)\n", nvram_err);
			ret = NVRAM_ERR;
			goto finally;
		}

		if( (ret = snx_isp_sensor_flip_set(flip) ) != 0 ) {
			UTIL_PRINT(SYS_ERR, "Set Video Flip Error\n");
			ret = SET_FLIP_ERR;
			goto finally;
		}
	} else {
		UTIL_PRINT(SYS_ERR, "Invalid Parameter\n");
		ret = INVALID_PARAM;
		goto finally;
	}

finally:

	xSemaphoreGive(g_util_mutex);

	return ret;

}

/***
int app_uti_get_video_status(int *wdr_status, int *mirror_status, int *flip_status, int *usbdclassmode,
							 int *capability, unsigned int *width, unsigned int *height, unsigned int *ucfps,
							 unsigned int *ucgop, unsigned int *uibps, int *resolution)
{
	int ret = 0;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	snx_isp_drc_status_get(wdr_status);
	snx_isp_sensor_mirror_get(mirror_status);
	snx_isp_sensor_flip_get(flip_status);
	*usbdclassmode = usbd_mid_get_class_mode();
	*capability = app_uti_get_capability();
	app_uti_get_preview_params(width, height, ucfps, ucgop, uibps);
	if( (*resolution = app_uti_wid_hight_to_resolution(*width, *height) ) == -1 )
	{
		UTIL_PRINT(SYS_ERR, "get resolution failed !\n");
		ret = INVALID_PARAM;
		goto finally;
	}

finally:

	xSemaphoreGive(g_util_mutex);

	return ret;
}
***/

int app_uti_set_preview_bitrate(unsigned int bit_rate)
{
	int ret = 0;
	int nvram_err = NVRAM_SUCCESS;
	unsigned int bitrate = bit_rate;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if( bitrate > 0 ) {
		if( (nvram_err = snx_nvram_unsign_integer_set(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_PREVIEWVIDEO_BPS, bitrate) ) != NVRAM_SUCCESS ) {
			UTIL_PRINT(SYS_ERR, "Save Bitrate To Flash Fail(%d)\n", nvram_err);
			ret = NVRAM_ERR;
			goto finally;
		}

		set_preview_bps_bit(1);
	} else {
		UTIL_PRINT(SYS_ERR, "Invalid Parameter\n");
		ret = INVALID_PARAM;
		goto finally;
	}

finally:

	xSemaphoreGive(g_util_mutex);

	return ret;
}

int app_uti_set_preview_fps(int fps)
{
	int ret = 0;
	int nvram_err = NVRAM_SUCCESS;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if( fps > 0 ) {
		if( (nvram_err = snx_nvram_integer_set(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_PREVIEW_VIDEO_FPS, fps) ) != NVRAM_SUCCESS ) {
			UTIL_PRINT(SYS_ERR, "Save FPS To Flash Fail(%d)\n", nvram_err);
			ret = NVRAM_ERR;
			goto finally;
		}

		if( (nvram_err = snx_nvram_integer_set(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_PREVIEW_VIDEO_GOP, fps) ) != NVRAM_SUCCESS ) {
			UTIL_PRINT(SYS_ERR, "Save GOP To Flash Fail(%d)\n", nvram_err);
			ret = NVRAM_ERR;
			goto finally;
		}
		set_preview_fps_bit(1);
	} else {
		UTIL_PRINT(SYS_ERR, "Invalid Parameter\n");
		ret = INVALID_PARAM;
		goto finally;
	}

finally:

	xSemaphoreGive(g_util_mutex);

	return ret;

}

int app_uti_set_preview_resolution(int resolution)
{
	int ret = 0;
	int nvram_err = NVRAM_SUCCESS;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if( (resolution >= 0) && (resolution <= 2) ) {
		if( resolution = 0) { //vga
			if( (nvram_err = snx_nvram_integer_set(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_PREVIEWVIDEO_WIDTH, VGA_WIDTH) ) != NVRAM_SUCCESS ) {
				UTIL_PRINT(SYS_ERR, "Save VGA Width To Flash Fail(%d)\n", nvram_err);
				ret = NVRAM_ERR;
				goto finally;
			}

			if( (nvram_err = snx_nvram_integer_set(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_PREVIEWVIDEO_HEIGHT, VGA_HEIGHT) ) != NVRAM_SUCCESS ) {
				UTIL_PRINT(SYS_ERR, "Save VGA Height To Flash Fail(%d)\n", nvram_err);
				ret = NVRAM_ERR;
				goto finally;
			}
		}

		if( resolution = 1) { //hd
			if( (nvram_err = snx_nvram_integer_set(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_PREVIEWVIDEO_WIDTH, HD_WIDTH) ) != NVRAM_SUCCESS ) {
				UTIL_PRINT(SYS_ERR, "Save HD Width To Flash Fail(%d)\n", nvram_err);
				ret = NVRAM_ERR;
				goto finally;
			}

			if( (nvram_err = snx_nvram_integer_set(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_PREVIEWVIDEO_HEIGHT, HD_HEIGHT) ) != NVRAM_SUCCESS ) {
				UTIL_PRINT(SYS_ERR, "Save HD Height To Flash Fail(%d)\n", nvram_err);
				ret = NVRAM_ERR;
				goto finally;
			}
		}

		set_preview_reso_bit(1);
	} else {
		UTIL_PRINT(SYS_ERR, "Invalid Parameter\n");
		ret = INVALID_PARAM;
		goto finally;
	}

finally:

	xSemaphoreGive(g_util_mutex);

	return ret;

}

int app_uti_set_power_frequency(int power_frequency)
{
	int ret = 0;
	int nvram_err = NVRAM_SUCCESS;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if( (power_frequency == 50) || (power_frequency == 60) ) {
		if( (nvram_err = snx_nvram_integer_set(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_VIDEO_POWERFREQUENCY, power_frequency) ) != NVRAM_SUCCESS ) {
			UTIL_PRINT(SYS_ERR, "Save Power Frequency To Flash Fail(%d)\n", nvram_err);
			ret = NVRAM_ERR;
			goto finally;
		}
		if( (ret = snx_isp_light_frequency_set(power_frequency) ) != 0 ) {
			UTIL_PRINT(SYS_ERR, "Set Power Frequency Error\n");
			ret = SET_POWER_FREQ_ERR;
			goto finally;
		}
	} else {
		UTIL_PRINT(SYS_ERR, "Invalid Parameter\n");
		ret = INVALID_PARAM;
		goto finally;
	}

finally:

	xSemaphoreGive(g_util_mutex);

	return ret;

}

int app_uti_get_video_ext_qp(int *pre_ext_pframe_num, int *pre_ext_qp_range, int *pre_ext_qp_max, int *pre_ext_upper_pframe, int *pre_ext_upper_pframe_dup1, int *pre_qp_max, int *pre_qp_min)
{
	int ret = 0;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if( snx_nvram_integer_get(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_PREVIEW_EXT_PFRAME_NUM, pre_ext_pframe_num) != NVRAM_SUCCESS ) {
		UTIL_PRINT(SYS_ERR, "cannot get ext qp pframe number !\n");
		ret = NVRAM_ERR;
		goto finally;
	}

	if( snx_nvram_integer_get(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_PREVIEW_EXT_QP_RANGE, pre_ext_qp_range) != NVRAM_SUCCESS ) {
		UTIL_PRINT(SYS_ERR, "cannot get ext qp range !\n");
		ret = NVRAM_ERR;
		goto finally;
	}

	if( snx_nvram_integer_get(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_PREVIEW_EXT_QP_MAX, pre_ext_qp_max) != NVRAM_SUCCESS ) {
		UTIL_PRINT(SYS_ERR, "cannot get ext qp max value !\n");
		ret = NVRAM_ERR;
		goto finally;
	}

	if( snx_nvram_integer_get(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_PREVIEW_EXT_UPPER_PFRAME, pre_ext_upper_pframe) != NVRAM_SUCCESS) {
		UTIL_PRINT(SYS_ERR, "cannot get ext qp upper pframe size !\n");
		ret = NVRAM_ERR;
		goto finally;
	}

	if( snx_nvram_integer_get(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_PREVIEW_EXT_UPPER_PFRAME_DUP1, pre_ext_upper_pframe_dup1) != NVRAM_SUCCESS) {
		UTIL_PRINT(SYS_ERR, "cannot get ext qp upper pframe size for dup1 !\n");
		ret = NVRAM_ERR;
		goto finally;
	}

	if( snx_nvram_integer_get(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_PREVIEW_QP_MAX, pre_qp_max) != NVRAM_SUCCESS) {
		UTIL_PRINT(SYS_ERR, "cannot get qp max value !\n");
		ret = NVRAM_ERR;
		goto finally;
	}

	if( snx_nvram_integer_get(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_PREVIEW_QP_MIN, pre_qp_min) != NVRAM_SUCCESS) {
		UTIL_PRINT(SYS_ERR, "cannot get qp min value !\n");
		ret = NVRAM_ERR;
		goto finally;
	}

finally:

	xSemaphoreGive(g_util_mutex);

	return ret;
}

int app_uti_get_video_preview_time(int *resume, int *suspend) {
	int ret = 0;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if( snx_nvram_integer_get(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_PREVIEW_RESUME, resume) != NVRAM_SUCCESS) {
		UTIL_PRINT(SYS_ERR, "cannot get resume time !\n");
		ret = NVRAM_ERR;
		goto finally;
	}

	if( snx_nvram_integer_get(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_PREVIEW_SUSPEND, suspend) != NVRAM_SUCCESS) {
		UTIL_PRINT(SYS_ERR, "cannot get suspend time !\n");
		ret = NVRAM_ERR;
		goto finally;
	}

finally:
	xSemaphoreGive(g_util_mutex);

	return ret;
}


//************************************************
//			SD Manager APIs
//************************************************


int app_uti_get_sd_status(void)
{
	int sdstate = 0;

	sdstate = app_uti_chk_sd_state();

	return sdstate;
}

/***
int app_uti_get_sd_space(int *fre_percent, int *tot_GByte)
{
	int ret = 0;
	unsigned long fre_clust = 0, tot_clust = 0;
	FATFS *fs = NULL;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if( (ret = app_uti_chk_sd_state() ) != SD_OK )
	{
		goto finally;
	}

	if( (ret = f_getfree("0:", &fre_clust, &fs) ) == FR_OK )
	{
		// Get total sectors and free sectors
		tot_clust = (fs->n_fatent - 2);
		*fre_percent = (int)(fre_clust * 100 + (tot_clust >>1 ) ) / tot_clust;
		*tot_GByte = (tot_clust * fs->csize + (1 << 21) -1) >> (30 - 9);
		UTIL_PRINT(SYS_INFO, "total = %d, free = %d, csize = %d\n", tot_clust, fre_clust, fs->csize);
	}
	else
	{
		UTIL_PRINT(SYS_ERR, "get free fail\n");
		ret = SD_GET_SPACE_ERR;
	}

finally:

	xSemaphoreGive(g_util_mutex);

	return ret;
}
***/
int app_uti_is_format_running(void)
{
	return g_sd_format_running;
}

int app_uti_get_sd_format_status(void)
{
	return g_format_status;
}

void app_uti_set_sd_format(void *args)
{
	int ret = 0;
	g_format_status = SD_FORMAT_WAITING;
	unsigned char *polling_status_byte = (unsigned char *)args;

	if (polling_status_byte)
		*polling_status_byte = SD_FORMAT_WAITING - SD_ERR_BASE;
	
	close_access_sdcard_fun();

	if(mf_thumbnail_status() != 0)
		g_format_status = SD_THUMBNAIL_ON_WORKING_ERR;
	else {
		ret = f_mkfs ("0:", 0, 0);
		if( ret == 0 ) {
			UTIL_PRINT(SYS_INFO, "sd format success\n");
			g_format_status = OK;
		} else {
			UTIL_PRINT(SYS_ERR, "sd format fail\n");
			g_format_status = SD_FAILED;
		}
	}

	if (polling_status_byte) { //snx_xu_usr.c polling status BIT 0~3
		if (g_format_status)
			*polling_status_byte = g_format_status - SD_ERR_BASE;
		else
			*polling_status_byte = 0xF;
	}

	UTIL_PRINT(SYS_INFO, "Format Result=%d \n", g_format_status);
	start_access_sdcard_fun();
	g_sd_format_running = 0;
	vTaskDelete(NULL);
}

int app_uti_create_sd_format_task(void *args)
{
	int ret = 0;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	//check if sdcard is working
	if((getfileuploadstatus(UPLOAD_BG) == UPLOAD_START) || (getfileuploadstatus(UPLOAD_FG) == UPLOAD_START))
	{
		ret = SD_FILE_UPLOADING_ERR;
		goto finally;
	}
	else if(conn_list_len(&playback_list) != 0)
	{
		ret = SD_PLAYBAK_ON_WORKING_ERR;
		goto finally;
	}
	else if(mf_snapshot_status() != 0)
	{
		ret = SD_SNAPSHOT_ON_WORKING_ERR;
		goto finally;
	}

	if( g_sd_format_running == 0) {
		if(pdPASS != xTaskCreate(app_uti_set_sd_format, "sd_format_task", STACK_SIZE_2K, args, PRIORITY_TASK_APP_CMDDAEMON, NULL)) {
			UTIL_PRINT(SYS_ERR, "Create SD Format Task Failed!!!\n");
			ret = CREATE_TASK_ERR;
			goto finally;
		}
		g_sd_format_running = 1;
	} else {
		UTIL_PRINT(SYS_ERR, "SD Card Is Formatting Now.\n");
		ret = SD_IS_FORMATTING;
	}
finally:

	xSemaphoreGive(g_util_mutex);

	return ret;
}

#define SD_TEST_PATTERN_SZ		(1024*1024)		/* 1M byte */
#define SD_TEST_BLK					1
#define SD_TEST_LOOP_CNT			2
#define LEN_REPORT					16

typedef struct sd_test_report
{
	char avg_write[LEN_REPORT];
	char avg_read[LEN_REPORT];
	int reserved;
} SD_Report_t, * pSD_Report_t;

SD_Report_t sd_test_report;


static void sd_speed_transfer_to_string(char *output, unsigned long long speed)
{
	unsigned long deci;

	if (speed > (1024*1024)) {
		speed *= 100;
		speed /= (1024*1024);
		deci = speed % 100;
		speed /= 100;
		snprintf(output, LEN_REPORT, "%lu.%lu %s", (unsigned long)speed, deci, "MB/s");
	}
	else if (speed > 1024) {
		speed /= 1024;
		snprintf(output, LEN_REPORT, "%lu %s", (unsigned long)speed, "KB/s");
	}
	else {
		snprintf(output, LEN_REPORT, "%lu %s", (unsigned long)speed, "B/s");
	}
}

static int sd_rwtest(pSD_Report_t pReport)
{
	uint64_t sd_size;
	unsigned int cmp_size = SD_TEST_PATTERN_SZ, block_cnt;
	uint8_t *sd_buf1 = NULL, *sd_buf2 = NULL;
	int i, ret;
	struct timeval start_tt, end_tt;
	unsigned long long speed, cost_time;
	int test_cnt = 1, cnt, bs;
	unsigned long long total_speed, total_time_read = 0, total_time_write = 0;
	uint32_t wbytes = 0, rbytes = 0;
	FRESULT res;
	FIL rFile, wFile;

	block_cnt = SD_TEST_BLK;
	test_cnt = SD_TEST_LOOP_CNT;

	if ((ret = mid_sd_identify(MID_SD_BLOCK, NULL)) != MID_SD_QUEUE_FINISH) {
		UTIL_PRINT(SYS_ERR, "identify fail\n");
		ret = pdFAIL;
		goto fail;
	}

	if ((ret = mid_sd_get_capacity(&sd_size , MID_SD_BLOCK, NULL)) == MID_SD_QUEUE_FINISH) {
		UTIL_PRINT(SYS_DBG, "sd_size = %d MBytes\n", sd_size / 2048);
	}

	UTIL_PRINT(SYS_DBG, "rwtest size = 0x%x bytes\n",	(uint32_t)cmp_size);

	if (!(sd_buf1 = pvPortMalloc(cmp_size, GFP_KERNEL, MODULE_APP))) {
		UTIL_PRINT(SYS_ERR, "buffer1 allocate error\n");
		ret = pdFAIL;
		goto fail;
	}

	if (!(sd_buf2 = pvPortMalloc(cmp_size, GFP_KERNEL, MODULE_APP))) {
		UTIL_PRINT(SYS_ERR, "buffer2 allocate error\n");
		ret = pdFAIL;
		goto fail;
	}

	memset(sd_buf2, 0xff, cmp_size);

	for (cnt = 0; cnt < test_cnt; cnt++) {
		if (f_open(&wFile, ".rwtestfile", FA_CREATE_ALWAYS | FA_WRITE))
			goto fail;
		gettimeofday(&start_tt, NULL);
		for (bs = 0; bs < block_cnt; bs++) {
			res = f_write(&wFile, sd_buf1, cmp_size, (void *)&wbytes);
		}
		
		f_sync(&wFile);
		gettimeofday(&end_tt, NULL);
		if (end_tt.tv_usec < start_tt.tv_usec)
			cost_time = (end_tt.tv_sec - 1 - start_tt.tv_sec)*1000000 + (end_tt.tv_usec +1000000 - start_tt.tv_usec);
		else
			cost_time = (end_tt.tv_sec - start_tt.tv_sec)*1000000 + (end_tt.tv_usec - start_tt.tv_usec);

		total_time_write += cost_time;
		speed = cmp_size * block_cnt; //cast first
		speed = speed * 1000000 / cost_time;
		f_close(&wFile);

		if (f_open(&rFile, ".rwtestfile", FA_OPEN_EXISTING | FA_READ))
			goto fail;
		gettimeofday(&start_tt, NULL);
		for (bs = 0; bs < block_cnt; bs++) {
			f_read(&rFile, sd_buf2, cmp_size, &rbytes);
		}
		
		gettimeofday(&end_tt, NULL);
		if (end_tt.tv_usec < start_tt.tv_usec)
			cost_time = (end_tt.tv_sec - 1 - start_tt.tv_sec)*1000000 + (end_tt.tv_usec +1000000 - start_tt.tv_usec);
		else
			cost_time = (end_tt.tv_sec - start_tt.tv_sec)*1000000 + (end_tt.tv_usec - start_tt.tv_usec);
		
		total_time_read += cost_time;
		speed = cmp_size * block_cnt; //cast first
		speed = speed * 1000000 / cost_time;
		f_close(&rFile);

		if (f_open(&rFile, ".rwtestfile", FA_OPEN_EXISTING | FA_READ))
			goto fail;
		for (bs = 0; bs < block_cnt; bs++) {
			f_read(&rFile, sd_buf2, cmp_size, &rbytes);
			for (i = 0; i < cmp_size; i++ ) {
				if (sd_buf1[i] != sd_buf2[i]) {
					UTIL_PRINT(SYS_ERR, "data compare error (%x != %x) from %d\n", sd_buf1[i], sd_buf2[i], i);
					ret = pdFAIL;
					goto fail;
				}
			}
		}
		
		f_close(&rFile);

		if (i == cmp_size)
			UTIL_PRINT(SYS_INFO, "data compare success\n");
		
		f_unlink(".rwtestfile");
	}

	if (test_cnt > 1) {
		//Avg write
		total_speed = test_cnt * cmp_size * block_cnt;
		total_speed = total_speed * 1000000 / total_time_write;
		total_time_write /= test_cnt;
		sd_speed_transfer_to_string(pReport->avg_write, total_speed);
		UTIL_PRINT(SYS_INFO, "avg_write: %s\n", pReport->avg_write);

		//Avg read
		total_speed = test_cnt * cmp_size * block_cnt;
		total_speed = total_speed * 1000000 / total_time_read;
		total_time_read /= test_cnt;
		sd_speed_transfer_to_string(pReport->avg_read, total_speed);
		UTIL_PRINT(SYS_INFO, "avg_read: %s\n", pReport->avg_read);
	}

	ret = pdPASS;
	
fail:
	if (sd_buf1) { vPortFree(sd_buf1); sd_buf1 = NULL; }
	if (sd_buf2) { vPortFree(sd_buf2); sd_buf2 = NULL; }
	if (ret == pdFAIL) {
		UTIL_PRINT(SYS_ERR, "sd rwtest fail\n");
	}
	else {
		UTIL_PRINT(SYS_INFO, "sd rwtest pass\n");
	}
	fs_cmd_umount(0);

	return ret;
}	


int app_uti_is_test_running(void)
{
	return g_sd_test_running;
}

int app_uti_get_sd_test_status(void)
{
	return g_test_status;
}

void app_uti_get_test_report(char *avg_write, char *avg_read)
{
	snprintf(avg_write, strlen(sd_test_report.avg_write), "%s", sd_test_report.avg_write);
	snprintf(avg_read, strlen(sd_test_report.avg_read), "%s", sd_test_report.avg_read);
}

static void app_uti_sdtest_task(void *args)
{
	int ret = 0;
	
	memset(&sd_test_report, 0, sizeof(SD_Report_t));
	
	g_test_status = SD_TEST_WAITING;
	unsigned char *polling_status_byte = (unsigned char *)args;

	if (polling_status_byte)
		*polling_status_byte = SD_TEST_WAITING - SD_ERR_BASE;
	
	close_access_sdcard_fun();

	if ((ret = sd_rwtest(&sd_test_report)) != pdPASS) {
		g_test_status = SD_TEST_FAILED_ERR;
	}
	else {
		g_test_status = OK;
	}

	if (polling_status_byte) { //snx_xu_usr.c polling status BIT 0~3
		if (g_test_status)
			*polling_status_byte = g_test_status - SD_ERR_BASE;
		else
			*polling_status_byte = 0xF;
	}

	UTIL_PRINT(SYS_DBG, "Test Result=%d \n", g_test_status);
	start_access_sdcard_fun();
	g_sd_test_running = 0;
	vTaskDelete(NULL);
}


int app_uti_create_sd_test_task(void *args)
{
	int ret = 0;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	//check if sdcard is working
	if ((getfileuploadstatus(UPLOAD_BG) == UPLOAD_START) || (getfileuploadstatus(UPLOAD_FG) == UPLOAD_START))
	{
		ret = SD_FILE_UPLOADING_ERR;
		goto finally;
	}
	else if(conn_list_len(&playback_list) != 0)
	{
		ret = SD_PLAYBAK_ON_WORKING_ERR;
		goto finally;
	}
	else if(mf_snapshot_status() != 0)
	{
		ret = SD_SNAPSHOT_ON_WORKING_ERR;
		goto finally;
	}

	if (g_sd_test_running == 0) {
		if(pdPASS != xTaskCreate(app_uti_sdtest_task, "sd_test_task", STACK_SIZE_4K, args, PRIORITY_TASK_APP_CMDDAEMON, NULL)) {
			UTIL_PRINT(SYS_ERR, "Create SD Format Task Failed!!!\n");
			ret = CREATE_TASK_ERR;
			goto finally;
		}
		g_sd_test_running = 1;
	} else {
		UTIL_PRINT(SYS_ERR, "SD Card Is Formatting Now.\n");
		ret = SD_IS_TESTING;
	}
	
finally:
	xSemaphoreGive(g_util_mutex);
	return ret;
}


//************************************************
//			Device Manager APIs
//************************************************

int app_uti_sync_time(system_date_t *time)
{
	int ret = 0;
	system_date_t dev_time = {0};
	long long rtc_sec = 0;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

#if 0
	// no RTC hardware will sync everytime.
	rtc_sec = (long long)rtc_get_rtctimer();
	time_to_tm(rtc_sec, 0, &dev_time);
#else
	get_date( &dev_time );
#endif
	//UTIL_PRINT(SYS_DBG, "#### JSON set time :%04d.%02d.%02d\n", time->year, time->month, time->day);
	//UTIL_PRINT(SYS_DBG, "#### JSON set day  :  %02d:%02d:%02d\n", time->hour, time->minute, time->second);
	//UTIL_PRINT(SYS_DBG, "#### SYS get time :%04d.%02d.%02d\n", dev_time.year, dev_time.month, dev_time.day);
	//UTIL_PRINT(SYS_DBG, "#### SYS get day  :	%02d:%02d:%02d\n", dev_time.hour, dev_time.minute, dev_time.second);
	if( (time->year != dev_time.year) || (time->month != dev_time.month) ||
	        (time->day != dev_time.day)	 || (time->hour != dev_time.hour)   ||
	        (time->minute != dev_time.minute) ||
	        ( (time->second - dev_time.second) > 10 ) ||
	        ( (time->second - dev_time.second) < -10) ) {
		schedrec_suspend(); //close record
#ifndef CONFIG_APP_DRONE
		reclapse_suspend();
#endif
		vTaskDelay(1000 / portTICK_RATE_MS );
		set_date(time, 0);
#ifdef CONFIG_MODULE_RTC_SUPPORT
		rtc_sec = tm_to_time(time, 0);
		rtc_set_rtctimer( (uint32_t) rtc_sec);
#endif
		modify_dev_time();
		schedrec_suspend_restart(0);
#ifndef CONFIG_APP_DRONE
		reclapse_suspend_restart(0);
#endif
	}
//finally:

	xSemaphoreGive(g_util_mutex);

	return ret;

}

int app_uti_get_iq_version(int *version)
{
	isp_iq_ver_t iqversion = {{0}};
	int ret = 0;

	if(snx_isp_iq_version_get(&iqversion) == 0) { //ok
		*version = (iqversion.date.y3 << 28) | (iqversion.date.y2 << 24) | (iqversion.date.y1 << 20) |
		           (iqversion.date.y0 << 16) | (iqversion.date.m1 << 12) | (iqversion.date.m0 << 8 ) |
		           (iqversion.date.d1 << 4 ) | (iqversion.date.d0);
		ret = OK;
	} else {
		*version = 0;
		ret = GET_IQ_VERSION_ERR;
	}

	return ret;
}

int app_uti_get_device_engmode_params(int *tx_retry, int *udp_type) {
	int ret = 0;
	int nvram_err = NVRAM_SUCCESS;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if( snx_nvram_integer_get("WIFI_DEV", "TX_RETRY", tx_retry) != NVRAM_SUCCESS )
	{
		UTIL_PRINT(SYS_ERR, "Get tx retry count from NVRAM failed!\n");
		ret = NVRAM_ERR;
		goto finally;
	}
	if( snx_nvram_integer_get("WIFI_DEV", "UDP_TYPE", udp_type) != NVRAM_SUCCESS )
	{
		UTIL_PRINT(SYS_ERR, "Get udp type from NVRAM failed!\n");
		ret = NVRAM_ERR;
		goto finally;
	}
finally:

	xSemaphoreGive(g_util_mutex);

	return ret;


}

int app_uti_get_device_params(int *version, int *gsensor_sensitivity, unsigned char *channel, unsigned char *ssid, unsigned char *pwd, int *powerfrequency, char *dev_ver, int *wifi_mode)
{
	int ret = 0;
	int nvram_err = NVRAM_SUCCESS;
	unsigned short ssid_len = 0;
	unsigned short pwd_len = 0;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	ret = app_uti_get_iq_version(version);
	if (ret != OK) {
		goto finally;
	}

	if( (nvram_err = snx_nvram_integer_get(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_VIDEO_GSENSORSENSITIVITY, gsensor_sensitivity) ) != NVRAM_SUCCESS )
	{
		UTIL_PRINT(SYS_ERR, "Get Gsensor_Paramter From Flash Fail(%d)\n", nvram_err);
		ret = NVRAM_ERR;
		goto finally;
	}

	if( (nvram_err = snx_nvram_integer_get("WIFI_DEV", "AP_AUTH_MODE", wifi_mode)) != NVRAM_SUCCESS)
	{
		UTIL_PRINT(SYS_ERR, "Get AP AUTH MODE From NVRAM Fail!(%d)\n", nvram_err);
		ret = NVRAM_ERR;
		goto finally;
	}
	WiFi_QueryAndSet(QID_HW_CHANNEL, channel, 0);
	WiFi_QueryAndSet(QID_BEACON_SSID, ssid, &ssid_len);

	if( (nvram_err = snx_nvram_string_get("WIFI_DEV", "AP_KEY_INFO", pwd)) != NVRAM_SUCCESS)
	{
		UTIL_PRINT(SYS_ERR, "Get AP AUTH MODE From NVRAM Fail!(%d)\n", nvram_err);
		ret = NVRAM_ERR;
		goto finally;
	}

	if( (nvram_err = snx_nvram_string_get("SNX_NVRAM", "version", dev_ver) ) != NVRAM_SUCCESS )
	{
		UTIL_PRINT(SYS_ERR, "get fw_version from flash fail(%d)\n", nvram_err);
		ret = NVRAM_ERR;
		goto finally;
	}
	if( (nvram_err = snx_isp_light_frequency_get(powerfrequency) ) != NVRAM_SUCCESS )
	{
		UTIL_PRINT(SYS_ERR, "Get Power Frequency From Flash Fail(%d)\n", nvram_err);
		ret = NVRAM_ERR;
		goto finally;
	}
finally:

	xSemaphoreGive(g_util_mutex);

	return ret;

}

int app_uti_set_nvram_to_default(void)
{
	int ret = 0;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if( g_reboot_task_running == 0) {
		if(pdPASS != xTaskCreate(reboot_task, "wait_to_reboot", STACK_SIZE_1K, NULL, PRIORITY_TASK_APP_CMDDAEMON, NULL)) {
			UTIL_PRINT(SYS_ERR, "Create reboot_task failed!!!\n");
			ret = CREATE_TASK_ERR;
			goto finally;
		}
		g_reboot_task_running = 1;
	} else {
		ret = SYSTEM_IS_REBOOTING;
		UTIL_PRINT(SYS_ERR, "Reboot task is already running!!!\n");
	}
finally:

	xSemaphoreGive(g_util_mutex);

	return ret;
}

int app_uti_set_class_mode(int usbdclassmode)
{
	int ret = 0;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if( ( (usbdclassmode >= 0) && (usbdclassmode <= 2) ) ) { //0:mass mode 1:uvc mode 2:hid mode
		usbd_mid_set_class_mode(usbdclassmode, 0);
	} else {
		UTIL_PRINT(SYS_ERR, "Invalid Parameters\n");
		ret = INVALID_PARAM;
		goto finally;
	}
finally:

	xSemaphoreGive(g_util_mutex);

	return ret;
}

int app_uti_set_osd_onoff(int osd_enable)
{
	int ret = 0;
	unsigned int len = 0;
	int nvram_err = NVRAM_SUCCESS;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if( (osd_enable == 0) || (osd_enable == 1) ) {
		if( (nvram_err = snx_nvram_integer_set(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_VIDEO_OSD, osd_enable) ) != NVRAM_SUCCESS ) {
			UTIL_PRINT(SYS_ERR, "Save OSD To Flash Fail(%d)", nvram_err);
			ret = NVRAM_ERR;
			goto finally;
		}

		if( (nvram_err = snx_nvram_get_data_len("app_osd_string", "osd_string", &len) ) != NVRAM_SUCCESS ) {
			UTIL_PRINT(SYS_ERR, "Get OSD String Length Frome Flash Fail(%d)", nvram_err);
			ret = NVRAM_ERR;
			goto finally;
		}

		if( len < 4) {
			UTIL_PRINT(SYS_ERR, "Nvram get osd str error. size=%d !!!!!!\n", len);
			ret =  GET_OSD_STRING_ERR;
			goto finally;
		}

	} else {
		UTIL_PRINT(SYS_ERR, "Invalid Parameters\n");
		ret = INVALID_PARAM;
		goto finally;
	}

finally:

	xSemaphoreGive(g_util_mutex);

	return ret;
}

int app_uti_send_fone_file(char *data, int size)
{
	int ret = 0;
	char *pdata;
	int nvram_err = NVRAM_SUCCESS;
	static char *nvram_pack = "app_osd_ctrl_font";
	static char *font_config_16 = "FontFile_16.bin";
	static char *font_config_48 = "FontFile_48.bin";

	nvram_data_info_t nvram;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	//save font 16
	pdata = data;
	nvram.data = pdata + HDR_LEN;
	nvram.data_len = (*(char *)(pdata + 13) << 24) | (*(char *)(pdata + 2) << 16) | (*(char *)(pdata + 11) << 8) | (*(char *)(pdata + 10));
	nvram.data_type = NVRAM_DT_BIN_RAW;
	if(((*(char *)(pdata + 1)) == 16) && (nvram.data_len + HDR_LEN) <= size) {
		UTIL_PRINT(SYS_DBG, "save font 16 size = %d\n", nvram.data_len);
		if((nvram_err = snx_nvram_set_immediately(nvram_pack, font_config_16, &nvram))) {
			UTIL_PRINT(SYS_ERR, "Save osd string to Flash Fail(%d)\n", nvram_err);
			ret = NVRAM_ERR;
		}
	}

	//save font 48
	pdata = data + HDR_LEN + nvram.data_len;
	nvram.data = pdata + HDR_LEN;
	nvram.data_len = (*(char *)(pdata + 13) << 24) | (*(char *)(pdata + 2) << 16) | (*(char *)(pdata + 11) << 8) | (*(char *)(pdata + 10));
	nvram.data_type = NVRAM_DT_BIN_RAW;
	if((*(char *)(pdata + 1)) == 48  && ((pdata - data)  + nvram.data_len + HDR_LEN) <= size) {
		UTIL_PRINT(SYS_DBG, "save font 48 size = %d\n", nvram.data_len);
		if((nvram_err = snx_nvram_set_immediately(nvram_pack, font_config_48, &nvram))) {
			UTIL_PRINT(SYS_ERR, "Save osd string to Flash Fail(%d)\n", nvram_err);
			ret = NVRAM_ERR;
		}
	}

	xSemaphoreGive(g_util_mutex);

	return ret;
}

int app_uti_get_osd_status(int *osd_enable, unsigned char **osd_str, int *nvramnodata, unsigned int *len)
{
	int ret = 0;
	int nvram_err = NVRAM_SUCCESS;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if( (nvram_err = snx_nvram_integer_get(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_VIDEO_OSD, osd_enable) ) != NVRAM_SUCCESS ) {
		UTIL_PRINT(SYS_ERR, "Get OSD Status From Flash Fail(%d)", nvram_err);
		ret = NVRAM_ERR;
		goto finally;
	}

	if( (nvram_err = snx_nvram_get_data_len("app_osd_string", "osd_string", len) ) != NVRAM_SUCCESS ) {
		UTIL_PRINT(SYS_ERR, "Get OSD String Length Frome Flash Fail(%d)", nvram_err);
		*nvramnodata = 1;
	}

	*osd_str = (unsigned char *)pvPortMalloc(*len, GFP_KERNEL, MODULE_APP);
	
	if(*osd_str == NULL) {
		UTIL_PRINT(SYS_ERR, "Could not allocation osd string memory space!!!!!!\n");
		ret = MEMORY_NOT_ENOUGH;
		goto finally;
	}

	if( (nvram_err = snx_nvram_uchar_hex_get("app_osd_string", "osd_string", *osd_str) ) != NVRAM_SUCCESS ) {
		UTIL_PRINT(SYS_ERR, "Get OSD String Length Frome Flash Fail(%d)", nvram_err);
		*nvramnodata = 2;
	}
finally:

	xSemaphoreGive(g_util_mutex);
	
	return ret;
}

int app_uti_take_picture(int pic_num)
{
	int ret = 0;
	int rc = 0;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	//UTIL_PRINT("\n");
	set_takepic_num(pic_num);
	if( (rc = mf_set_snapshot(1) ) != pdPASS ) {
		UTIL_PRINT(SYS_ERR, "Take Picture Failed.\n");
		ret = TAKE_PICTURE_ERR;
	} else {
		ret = OK;
	}

	xSemaphoreGive(g_util_mutex);

	return ret;

}

int app_uti_set_gsensor_sensitivity(int sensitivity)
{
	//high 1600, med 1000, low 500
	int ret = 0;
	int nvram_err = NVRAM_SUCCESS;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	UTIL_PRINT(SYS_DBG, "sensitivity %d\n", sensitivity);
	if(sensitivity > 0) {
		if( (nvram_err = snx_nvram_integer_set(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_VIDEO_GSENSORSENSITIVITY, sensitivity) ) != NVRAM_SUCCESS ) {
			UTIL_PRINT(SYS_ERR, "Save Flash Fail(%d)\n", nvram_err);
			ret = NVRAM_ERR;
			goto finally;
		}
#ifndef CONFIG_APP_DRONE
	set_gsensor_sensitivity(sensitivity);
#endif
	} else {
		UTIL_PRINT(SYS_ERR, "Invalid Parameters\n");
		ret = INVALID_PARAM;
		goto finally;
	}

finally:

	xSemaphoreGive(g_util_mutex);

	return ret;
}

//************************************************
//			Record APIs
//************************************************

int app_uti_enable_record(void)
{
	int ret = 0;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	user_enable_rec();

#ifndef CONFIG_APP_DRONE
	user_enable_reclapse();
#endif

	xSemaphoreGive(g_util_mutex);

	return ret;

}

int app_uti_disable_record(void)
{
	int ret = 0;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	user_diable_rec();

#ifndef CONFIG_APP_DRONE
	user_disable_reclapse();
#endif

	xSemaphoreGive(g_util_mutex);

	return ret;
}

/***
int app_uti_set_record_status(int rec_status)
{
	int ret = 0;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if( (ret = app_uti_chk_sd_state() ) != SD_OK )
	{
		goto finally;
	}

	if ( (rec_status != 1) || (rec_status != 0) )
	{
		if(schedrec_get_isfull() == 1)
		{
			UTIL_PRINT(SYS_ERR, "record folder is full.\n");
			ret = RECORD_FOLDER_IS_FULL;
			goto finally;
		}
	#ifndef CONFIG_APP_DRONE
		if(reclapse_get_isfull() == 1)
		{
			UTIL_PRINT(SYS_ERR, "timelapse record folder is full.\n");
			ret = TIMELAPSE_FOLDER_IS_FULL;
			goto finally;
		}
	#endif
		if(rec_status == 1)
		{
			user_enable_rec();
			//#ifdef CONFIG_APP_DRONE  //save to nvarm
			//			if (snx_nvram_integer_set(NVRAM_RECORD,NVRAM_RECORD_SCHED_ENABLE,&rec_status)!= NVRAM_SUCCESS)
			//			{
			//				UTIL_PRINT(SYS_ERR, "Save sched enable Fail(%d)", rec_status);
			//				goto resp_json;
			//			}
			//#endif
		#ifndef CONFIG_APP_DRONE
			user_enable_reclapse();
		#endif
		}
		else if(rec_status == 0)
		{
			user_diable_rec();
			//#ifdef CONFIG_APP_DRONE  //save to nvarm
			//		if (snx_nvram_integer_set(NVRAM_RECORD,NVRAM_RECORD_SCHED_ENABLE,&rec_status)!= NVRAM_SUCCESS)
			//		{
			//			UTIL_PRINT(SYS_ERR, "Save sched enable Fail(%d)", rec_status);
			//			goto resp_json;
			//		}
			//#endif
		#ifndef CONFIG_APP_DRONE
			user_disable_reclapse();
		#endif
		}
	}
	else
	{
		UTIL_PRINT(SYS_ERR, "Invalid Parameters\n");
		ret = INVALID_PARAM;
		goto finally;
	}
finally:

	xSemaphoreGive(g_util_mutex);

	return ret;
}
***/

int app_uti_get_record_status(int *rec_status, int *rec_running, unsigned int *recordfps, unsigned int *recordgop, unsigned int *recordbps, int *recordresolution, int *capability, int *level, int *recordlength, int *cycle, int *protectlength)
{
	int ret = 0;
	unsigned int recordwidth = 0, recordheight = 0;
	//int nvram_err = NVRAM_SUCCESS;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	*rec_status = chk_rec_enable();
	*rec_running= schedrec_state();

	mf_video_resmap_get_record_params(&recordwidth, &recordheight, recordfps, recordgop, recordbps);
	
	if( (*recordresolution = app_uti_wid_hight_to_resolution(recordwidth, recordheight) ) == -1 )
	{
		UTIL_PRINT(SYS_ERR, "get resolution failed !\n");
		ret = GET_RESOLUTION_ERR ;
		goto finally;
	}

	*capability = ( (mf_video_resmap_get_record_capability() & REC_CAPABILITY_MASK) |
				   (mf_video_resmap_get_preview_capability() & VIEW_CAPABILITY_MASK) );

#ifndef CONFIG_APP_DRONE
	if (snx_nvram_integer_get(NVRAM_PKG_AUDIO_ISP, NVRAM_CFG_AUDIO_VOICE, level) != NVRAM_SUCCESS)
	{
		UTIL_PRINT(SYS_ERR, "Get Record voice level from NVRAM failed!");
		ret = NVRAM_ERR;
		goto finally;
	}
#endif
	if( snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_SCHED_INTERVAL, recordlength) != NVRAM_SUCCESS )
	{
		UTIL_PRINT(SYS_ERR, "Get Record Length from NVRAM failed!");
		ret = NVRAM_ERR;
		goto finally;

	}

	*recordlength = *recordlength / 60;

	if( snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_SCHED_CYCLE, cycle) != NVRAM_SUCCESS )
	{
		UTIL_PRINT(SYS_ERR, "Get Record Length from NVRAM failed!");
		ret = NVRAM_ERR;
		goto finally;
	}

	if( snx_nvram_integer_get(NVRAM_RECORD, NVRAM_PROTECTRECORD_LENGTH, protectlength) != NVRAM_SUCCESS )
	{
		UTIL_PRINT(SYS_ERR, "Get Protect Length from NVRAM failed!");
		ret = NVRAM_ERR;
		goto finally;
	}

finally:

	xSemaphoreGive(g_util_mutex);

	return ret;

}

int app_uti_get_record_ext_qp(int *rec_ext_pframe_num, int *rec_ext_qp_range, int *rec_ext_qp_max, int *rec_ext_upper_pframe, int *rec_qp_max, int *rec_qp_min)
{
	int ret = 0;
	// int nvram_err = NVRAM_SUCCESS;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if( snx_nvram_integer_get(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_RECORD_EXT_PFRAME_NUM, rec_ext_pframe_num) != NVRAM_SUCCESS ) {
		UTIL_PRINT(SYS_ERR, "cannot get ext qp pframe number !\n");
		ret = NVRAM_ERR;
		goto finally;
	}

	if( snx_nvram_integer_get(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_RECORD_EXT_QP_RANGE, rec_ext_qp_range) != NVRAM_SUCCESS ) {
		UTIL_PRINT(SYS_ERR, "cannot get ext qp range !\n");
		ret = NVRAM_ERR;
		goto finally;
	}

	if( snx_nvram_integer_get(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_RECORD_EXT_QP_MAX, rec_ext_qp_max) != NVRAM_SUCCESS ) {
		UTIL_PRINT(SYS_ERR, "cannot get ext qp max value !\n");
		ret = NVRAM_ERR;
		goto finally;
	}

	if( snx_nvram_integer_get(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_RECORD_EXT_UPPER_PFRAME, rec_ext_upper_pframe) != NVRAM_SUCCESS) {
		UTIL_PRINT(SYS_ERR, "cannot get ext qp upper pframe size !\n");
		ret = NVRAM_ERR;
		goto finally;
	}

	if( snx_nvram_integer_get(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_RECORD_QP_MAX, rec_qp_max) != NVRAM_SUCCESS) {
		UTIL_PRINT(SYS_ERR, "cannot get qp max value !\n");
		ret = NVRAM_ERR;
		goto finally;
	}

	if( snx_nvram_integer_get(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_RECORD_QP_MIN, rec_qp_min) != NVRAM_SUCCESS) {
		UTIL_PRINT(SYS_ERR, "cannot get qp min value !\n");
		ret = NVRAM_ERR;
		goto finally;
	}

finally:

	xSemaphoreGive(g_util_mutex);

	return ret;
}

/***
int app_uti_set_record_length(int minutes)
{
	int ret = 0;
	int nvram_err = NVRAM_SUCCESS;
	int sec = 0;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if( (ret = app_uti_chk_sd_state() ) != SD_OK )
	{
		goto finally;
	}

	if(minutes > 0)
	{
		// when users setting record length is max (9999), caculate max sec.
		if (minutes == MAX_RECORD_LEN_SIG)
			minutes = 30;		// setting max record length to 30 minutes

		sec = minutes * 60;

		if( (nvram_err = snx_nvram_integer_set(NVRAM_RECORD, NVRAM_RECORD_SCHED_INTERVAL, sec) ) != NVRAM_SUCCESS )
		{
			UTIL_PRINT(SYS_ERR, "Save Record Length Flash Fail(%d)", nvram_err);
			ret = NVRAM_ERR;
			goto finally;
		}

		if( app_uti_check_all_task_is_running() != OK)
		{
			UTIL_PRINT(SYS_ERR, "ALL Task have not init success\n");
			ret = TASK_HAVE_NOT_INIT_OK;  //reboot task , wait
		}

		set_protect_closeflag();
		set_record_closeflag();
	#ifndef CONFIG_APP_DRONE
		set_lapse_record_closeflag();
	#endif
	}
	else
	{
		UTIL_PRINT(SYS_ERR, "Invalid Parameters\n");
		ret = INVALID_PARAM;
		goto finally;
	}

finally:

	xSemaphoreGive(g_util_mutex);

	return ret;

}
***/

/***
int app_uti_set_protect_length(int second)
{
	int ret = 0;
	int nvram_err = NVRAM_SUCCESS;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if( (ret = app_uti_chk_sd_state() ) != SD_OK )
	{
		goto finally;
	}

	if(second > 0)
	{
		if( (nvram_err = snx_nvram_integer_set(NVRAM_RECORD, NVRAM_PROTECTRECORD_LENGTH, second) ) != NVRAM_SUCCESS )
		{
			UTIL_PRINT(SYS_ERR, "Save Protect Length Flash Fail(%d)", nvram_err);
			ret = NVRAM_ERR;
			goto finally;
		}

	}
	else
	{
		UTIL_PRINT(SYS_ERR, "Invalid Parameters\n");
		ret = INVALID_PARAM;
		goto finally;
	}
finally:

	xSemaphoreGive(g_util_mutex);

	return ret;

}
***/

/***
int app_uti_set_record_loop(int status)
{
	int ret = 0;
	int nvram_err = NVRAM_SUCCESS;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if( (ret = app_uti_chk_sd_state() ) != SD_OK )
	{
		goto finally;
	}

	if( (status != 1) || (status != 0) )
	{
		if( (nvram_err = snx_nvram_integer_set(NVRAM_RECORD, NVRAM_RECORD_SCHED_CYCLE, status) ) != NVRAM_SUCCESS )
		{
			UTIL_PRINT(SYS_ERR, "Save Record Cycle Flash Fail(%d)", nvram_err);
			ret = NVRAM_ERR;
			goto finally;
		}
		set_record_cycle_bit(1);

	}
	else
	{
		UTIL_PRINT(SYS_ERR, "Invalid Parameters\n");
		ret = INVALID_PARAM;
		goto finally;
	}
finally:

	xSemaphoreGive(g_util_mutex);

	return ret;

}
***/
/***
int app_uti_set_record_audio_volume(int vol_level)
{
	int ret = 0;
	int nvram_err = NVRAM_SUCCESS;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if( (ret = app_uti_chk_sd_state() ) != SD_OK )
	{
		goto finally;
	}

	if( (vol_level >=  0) && (vol_level >= 100) )
	{
		UTIL_PRINT("voice level=%d \n",vol_level);

		if( (nvram_err = snx_nvram_integer_set(NVRAM_PKG_AUDIO_ISP, NVRAM_CFG_AUDIO_VOICE, vol_level) ) != NVRAM_SUCCESS )
		{
			UTIL_PRINT(SYS_ERR, "Save Audio Voice Flash Fail(%d)", nvram_err);
			ret = NVRAM_ERR;
			goto finally;
		}
		set_record_audio_voice_bit(1);
	}
	else
	{
		UTIL_PRINT(SYS_ERR, "Invalid Parameters\n");
		ret = INVALID_PARAM;
		goto finally;
	}

finally:

	xSemaphoreGive(g_util_mutex);

	return ret;

}
***/

int app_uti_set_record_ext_qp(int rec_ext_pframe_num, int rec_ext_qp_range, int rec_ext_qp_max, int rec_ext_upper_pframe, int rec_qp_max, int rec_qp_min)
{
	int ret = 0;
	int nvram_err = NVRAM_SUCCESS;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if( rec_ext_pframe_num != -1 ) {
		if ( (nvram_err = snx_nvram_integer_set(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_RECORD_EXT_PFRAME_NUM, rec_ext_pframe_num) ) != NVRAM_SUCCESS ) {
			UTIL_PRINT(SYS_ERR, "Save Flash Fail(%d)", nvram_err);
			ret = NVRAM_ERR;
			goto finally;
		}
		//qp_change = 1;
	}

	if( rec_ext_qp_range != -1 ) {
		if ( (nvram_err = snx_nvram_integer_set(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_RECORD_EXT_QP_RANGE, rec_ext_qp_range) ) != NVRAM_SUCCESS ) {
			UTIL_PRINT(SYS_ERR, "Save Flash Fail(%d)", nvram_err);
			ret = NVRAM_ERR;
			goto finally;
		}
		//qp_change = 1;
	}

	if( rec_ext_qp_max != -1 ) {
		if( (nvram_err = snx_nvram_integer_set(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_RECORD_EXT_QP_MAX, rec_ext_qp_max) ) != NVRAM_SUCCESS ) {
			UTIL_PRINT(SYS_ERR, "Save Flash Fail(%d)", nvram_err);
			ret = NVRAM_ERR;
			goto finally;
		}
		//qp_change = 1;
	}

	if( rec_ext_upper_pframe != -1 ) {
		if( (nvram_err = snx_nvram_integer_set(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_RECORD_EXT_UPPER_PFRAME, rec_ext_upper_pframe) ) != NVRAM_SUCCESS ) {
			UTIL_PRINT(SYS_ERR, "Save Flash Fail(%d)", nvram_err);
			ret = NVRAM_ERR;
			goto finally;
		}
		//qp_change = 1;
	}

	if( rec_qp_max != -1 ) {
		if( rec_qp_max >= 50 ) {
			UTIL_PRINT(SYS_ERR, "max qp must smaller than 50(%d)", rec_qp_max);
			rec_qp_max = 49;
			ret = INVALID_PARAM;
			goto finally;
		}
		if( (nvram_err = snx_nvram_integer_set(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_RECORD_QP_MAX, rec_qp_max) ) != NVRAM_SUCCESS ) {
			UTIL_PRINT(SYS_ERR, "Save Flash Fail(%d)", nvram_err);
			ret = NVRAM_ERR;
			goto finally;
		}
		//qp_change = 1;
	}

	if( rec_qp_min != -1 ) {
		if( rec_qp_min <= 10 ) {
			UTIL_PRINT(SYS_ERR, "min qp must larger than 10(%d)", rec_qp_min);
			rec_qp_min = 11;
			ret = INVALID_PARAM;
			goto finally;
		}
		if( (nvram_err = snx_nvram_integer_set(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_RECORD_QP_MIN, rec_qp_min) ) != NVRAM_SUCCESS ) {
			UTIL_PRINT(SYS_ERR, "Save Flash Fail(%d)", nvram_err);
			ret = NVRAM_ERR;
			goto finally;
		}
		//qp_change = 1;
	}

finally:

	xSemaphoreGive(g_util_mutex);

	return ret;

}

//************************************************
//			Transmit APIs
//************************************************

/***
int app_uti_get_indexfile(int list_src, int *port, int *get_seed, int *is_full, SocketItem_t *psocket)
{
	int ret = 0;
	int intbuf = 0;
	int list_state = 0;
	char list_path[100] = {0};

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if( (ret = app_uti_chk_sd_state() ) != SD_OK )
	{
		goto finally;
	}

	intbuf = get_rec_fileformat();

#if defined(CONFIG_SYSTEM_PLATFORM_SN98660) || defined(CONFIG_SYSTEM_PLATFORM_SN98661) || defined(CONFIG_SYSTEM_PLATFORM_SN98671) || defined (CONFIG_SYSTEM_PLATFORM_SN98293)
	if( (list_src == 0) || (list_src == 1) ) //list_src 0:protect ,1:pic 2:smallpic
	{
#ifndef CONFIG_APP_DRONE
	    if( list_src == 0 ) //protect
		{
			list_state = protectlist_to_file(list_path);
			if( (list_state == LIST_FILE_OK) || (list_state == LIST_FILE_IS_NONE) )
			{
				UTIL_PRINT(SYS_DBG, "list_path===%s\n",list_path);
			}
			else if( list_state == LIST_FILE_DOWNLOAD )
			{
				//useing the same filelist.txt
			}
			else
			{
				UTIL_PRINT(SYS_DBG, "list_state===%d\n", list_state);
				ret = GET_INDEX_ERR;
				goto finally;
			}

			*is_full = get_protectfolder_full();
		}
		else if( list_src == 1 ) //pic
		{
			list_state = piclist_to_file(list_path);
			if( (list_state == LIST_FILE_OK) || (list_state == LIST_FILE_IS_NONE) )
			{
				UTIL_PRINT(SYS_DBG, "list_path===%s\n",list_path);
			}
			else if( list_state == LIST_FILE_DOWNLOAD )
			{

			}
			else
			{
				UTIL_PRINT(SYS_DBG, "list_state===%d\n", list_state);
				ret = GET_INDEX_ERR;
				goto finally;
			}

			*is_full = mf_snapshot_get_isfull();
		}
#else
		if( list_src == 1 )
		{
			list_state = piclist_to_file(list_path);
			if ( (list_state == LIST_FILE_OK) || (list_state == LIST_FILE_IS_NONE) )
			{
				UTIL_PRINT(SYS_DBG, "list_path===%s\n", list_path);
			}
			else if( list_state == LIST_FILE_DOWNLOAD )
			{
				//useing the same filelist.txt
			}
			else
			{
				UTIL_PRINT(SYS_DBG, "list_state===%d\n", list_state);
				ret = GET_INDEX_ERR;
				goto finally;
			}

			*is_full = mf_snapshot_get_isfull();
		}
#endif
	}
	else
	{
		UTIL_PRINT(SYS_ERR, "Invalid Parameters\n");
		ret = INVALID_PARAM;
		goto finally;
	}
#else
	if( (list_src == 0) || (list_src == 1) )//list_src 0:protect ,1:pic 2:smallpic
	{
		chkdir_writetofile(list_src+1, list_path, intbuf);
	}
	else
	{
		UTIL_PRINT(SYS_ERR, "Invalid Parameters\n");
		ret = INVALID_PARAM;
		goto finally;
	}
#endif

	if( add_file_upload_bg(list_path, psocket->addr.sin_addr.s_addr, (uint16_t*)port, 0) != 0 )
	{
		UTIL_PRINT(SYS_ERR, "Upload File Failed..\n");
		ret = UPLOAD_FILE_ERR;
		goto finally;
	}

	get_seed_from_file(get_seed);
	UTIL_PRINT(SYS_DBG, "get_seed=%d\n", *get_seed);
finally:

	xSemaphoreGive(g_util_mutex);

	return ret;
}
***/

#if 0
#if LIMIT_PLAYBACK_AND_DOWNLOAD_CONNECTION
static int is_ip_exist_in_dwnlod_pb_conn(int socket_addr)
{
	int index = 0;

	for(index; index < dwnload_and_playback_max_conn_number; index++) {
		if (socket_addr == dwnload_pb_conn[index].ip) {
			UTIL_PRINT(SYS_DBG, "Socket ip %d found\n", socket_addr);
			return 1;
		}
	}

	if(index >= dwnload_and_playback_max_conn_number) {

		UTIL_PRINT(SYS_DBG, "Socket ip %d no found\n", socket_addr);
		return 0;
	}
}

int app_uti_chk_and_add_connection(int filetype, SocketItem_t *psocket, int conn_state)
{
	int ret = 0;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if( filetype < 0) {
		UTIL_PRINT(SYS_ERR, "Invalid Parameters\n");
		ret = 1;
		goto finally;
	}
	//check connection number of download file and playback.
	if ((filetype == 0) || (filetype == 1) || (filetype == 3)) { //filetype 0:record file. 1:protect file. 3:timelapse file
		if (is_ip_exist_in_dwnlod_pb_conn(psocket->addr.sin_addr.s_addr)) {
			dwnlod_and_pb_conn_num++;
			UTIL_PRINT(SYS_DBG, "Download and Playback conn = %d\n", dwnlod_and_pb_conn_num);
			psocket->status = 1;
		} else {
			if (dwnlod_and_pb_conn_num >= dwnload_and_playback_max_conn_number) {
				UTIL_PRINT(SYS_ERR, "DownloadFile and Playback connections is up limit.\n");
				UTIL_PRINT(SYS_ERR, "Max Conn = %d, Current Conn = %d\n", dwnload_and_playback_max_conn_number, dwnlod_and_pb_conn_num);
				ret = -1;
				goto finally;
			} else {
				psocket->status = 1;
				add_conn_to_pb_and_dwnlod(psocket->addr.sin_addr.s_addr, 1);
				dwnlod_and_pb_conn_num++;
				UTIL_PRINT(SYS_DBG, "Download and Playback conn = %d\n", dwnlod_and_pb_conn_num);
			}
		}
	}

finally:

	xSemaphoreGive(g_util_mutex);

	return ret;
}

void app_uti_chk_and_delete_connection(int filetype, SocketItem_t *psocket)
{
	int ret = 0;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if( filetype < 0) {
		UTIL_PRINT(SYS_ERR, "Invalid Parameters\n");
		ret = 1;
		goto finally;
	}

	if( (filetype == 0) || (filetype == 1) || (filetype == 3) ) { //filetype 0:record file. 1:protect file. 3:timelapse file
		if( is_ip_exist_in_dwnlod_pb_conn(psocket->addr.sin_addr.s_addr) ) {
			psocket->status = 0;
			set_dwnlod_pb_conn_status(psocket->addr.sin_addr.s_addr, 0);
			dwnlod_and_pb_conn_num--;
			UTIL_PRINT(SYS_DBG, "Download and Playback conn = %d\n", dwnlod_and_pb_conn_num);
		} else {
			psocket->status = 0;
			UTIL_PRINT(SYS_DBG, "Download and Playback conn = %d\n", dwnlod_and_pb_conn_num);
		}
	}

finally:

	xSemaphoreGive(g_util_mutex);

	return ret;
}
#endif
#endif

/***
int app_uti_add_file_upload(char *fname, int number, int pos, int filetype, int *port, int *file_size, SocketItem_t *psocket)
{
	char abs_path[300] = {0};
	char newname[32] = {0};
	int ret = 0;
	int rc = 0;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if( (fname == NULL) || (pos < 0) || (filetype < 0))
	{
		UTIL_PRINT(SYS_ERR, "Invalid Parameters\n");
		ret = INVALID_PARAM;
		goto finally;
	}

	if((number==0)&&((filetype==4)||(filetype==5)||(filetype==6)))
	{
		rename_recfile_addlen(fname,newname,0);
	}else
	{
		sprintf(newname,"%s",fname);
	}

	switch(filetype)
	{
		case 0:	//record file
			sprintf(abs_path, "%s/%s/%s", SD_ROOT,SD_SCHED_PATH, newname);
			break;
		case 1:	//protect file
			sprintf(abs_path, "%s/%s/%s", SD_ROOT,SD_PROTECT_PATH, newname);
			break;
		case 2:	//pic
			sprintf(abs_path, "%s/%s/%s", SD_ROOT,SD_PICTURE_PATH, newname);
			break;
		case 3:	//timelapse
			sprintf(abs_path, "%s/%s/%s", SD_ROOT,SD_TIMELAPSE_PATH, newname);
			break;
		case 4: //record thumbnail
			sprintf(abs_path, "%s/%s/%s/%s", SD_ROOT,SD_SCHED_PATH,SD_THUMBNAIL_PATH, newname);
			break;
		case 5: //protect thumbnail
			sprintf(abs_path, "%s/%s/%s/%s", SD_ROOT,SD_PROTECT_PATH,SD_THUMBNAIL_PATH, newname);
			break;
		case 6: //timelapse thumbnail
			sprintf(abs_path, "%s/%s/%s/%s", SD_ROOT,SD_TIMELAPSE_PATH,SD_THUMBNAIL_PATH, newname);
			break;
		case 7: //filelist
			sprintf(abs_path, "FileList.txt");
			break;
	}
	UTIL_PRINT(SYS_DBG, "path = %s\n", abs_path);

	if( (rc = get_filesize(abs_path) ) != 0 )
	{
		ret = GET_FILE_SIZE_ERR;
		goto finally;
	}
	else
	{
		*file_size = rc;
	}

	rc = add_file_upload_bg(abs_path, psocket->addr.sin_addr.s_addr, (uint16_t*)port, pos);
	if(rc == UPLOAD_FILE_INVALID_PARAM)
		ret = INVALID_PARAM;
	else if(rc == UPLOAD_FILE_NOT_EXIST)
		ret = UPLOAD_FILE_NOT_EXIST;
	else if(rc == UPLOAD_FILE_USER_IS_FULL)
		ret = UPLOAD_FILE_USER_IS_FULL;
	else if(rc == UPLOAD_CREATE_TASK_FAILED)
		ret = UPLOAD_CREATE_TASK_FAILED;
	else if(rc == UPLOAD_WAIT_PRE_TASK_CLOSE)
		ret = UPLOAD_WAIT_PRE_TASK_CLOSE;

finally:
	xSemaphoreGive(g_util_mutex);

	return ret;
}

int app_uti_stop_file_upload(int filetype, char *fname, SocketItem_t *psocket)
{
	char abs_path[300] = {0};
	int ret = 0;
	int rc = 0;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if( (filetype < 0 ) || (fname == NULL) )
	{
		UTIL_PRINT(SYS_ERR, "Invalid Parameters\n");
		ret = INVALID_PARAM;
		goto finally;
	}

	switch(filetype)
	{
		case 0:	//record file
			sprintf(abs_path, "%s/%s/%s", SD_ROOT,SD_SCHED_PATH, fname);
			break;
		case 1:	//protect file
			sprintf(abs_path, "%s/%s/%s", SD_ROOT,SD_PROTECT_PATH, fname);
			break;
		case 2:	//pic
			sprintf(abs_path, "%s/%s/%s", SD_ROOT,SD_PICTURE_PATH, fname);
			break;
		case 3: //timelapse
			sprintf(abs_path, "%s/%s/%s", SD_ROOT,SD_TIMELAPSE_PATH, fname);
			break;
		case 4: //record thumbnail
			sprintf(abs_path, "%s/%s/%s/%s", SD_ROOT,SD_SCHED_PATH,SD_THUMBNAIL_PATH, fname);
			break;
		case 5: //protect thumbnail
			sprintf(abs_path, "%s/%s/%s/%s", SD_ROOT,SD_PROTECT_PATH,SD_THUMBNAIL_PATH, fname);
			break;
		case 6: //timelapse thumbnail
			sprintf(abs_path, "%s/%s/%s/%s", SD_ROOT,SD_TIMELAPSE_PATH,SD_THUMBNAIL_PATH, fname);
			break;
		case 7:
			sprintf(abs_path, "FileList.txt");
			break;
	}

	UTIL_PRINT(SYS_DBG, "path = %s\n", abs_path);
	rc = stopfileupload(psocket->addr.sin_addr.s_addr, UPLOAD_BG);
	if(rc == -1)
	{
		ret = STOP_UPLOAD_FILE_ERR; //stopfile error.
	}
	else if(rc = -2)
	{
		ret = INVALID_PARAM; //Invalid Parameters, Only Accept UPLOAD_BG OR UPLOAD_FG
	}

finally:

	xSemaphoreGive(g_util_mutex);

	return ret;
}
***/
/***
int app_uti_delete_file(int filetype, char *fname)
{
	int ret = 0;
	char abs_path[300] = {0};

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if( (ret = app_uti_chk_sd_state() ) != SD_OK )
	{
		goto finally;
	}

	if( (filetype < 0) || (fname == NULL) )
	{
		UTIL_PRINT(SYS_ERR, "Invalid Parameters\n");
		ret = INVALID_PARAM;
		goto finally;
	}

	switch(filetype)
	{
		case 0:	//record file
			sprintf(abs_path, "%s/%s/%s", SD_ROOT,SD_SCHED_PATH, fname);
			break;
		case 1:	//protect file
			sprintf(abs_path, "%s/%s/%s", SD_ROOT,SD_PROTECT_PATH, fname);
			break;
		case 2:	//pic
			sprintf(abs_path, "%s/%s/%s", SD_ROOT,SD_PICTURE_PATH, fname);
			break;
	}

	UTIL_PRINT(SYS_DBG, "path = %s\n", abs_path);

	//notice record to update file list
	if(filetype==0)   //record
	{
		int result;
		result=del_rec_filenode(fname);
		if(result==0)
		{
			if (fs_cmd_rm(abs_path) == FR_OK)
				ret = OK;
			else
				ret = DELETE_FILE_ERR;
			del_thumbnail(abs_path);
		}

	}
#ifndef CONFIG_APP_DRONE
	else if(filetype==1)  //protect
	{
		int result;
		result=chk_del_rec_protectfile(fname);
		if(result==0)
		{
			f_chmod(abs_path, AM_SYS | AM_ARC, AM_ARC | AM_RDO | AM_HID | AM_SYS);
			if (fs_cmd_rm(abs_path) == FR_OK)
				ret = OK;
			else
				ret = DELETE_FILE_ERR;
			del_thumbnail(abs_path);
		}
	}
#endif
	else if(filetype == 2)  //pic
	{
		del_snapshot_file(fname);

		if (fs_cmd_rm(abs_path) == FR_OK)
			ret = OK;
		else
			ret = DELETE_FILE_ERR;

	}

finally:

	xSemaphoreGive(g_util_mutex);

	return ret;
}
***/
int app_uti_del_record_file(char *abs_path, char *fname)
{
	int ret = 0;
	int result = 0;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	result = rec_filenode_del(T_RECORD, fname);
	if(result == 0) {
		if (fs_cmd_rm(abs_path) == FR_OK)
			ret = OK;
		else
			ret = DELETE_FILE_ERR;
		rec_thumbnail_del(T_RECORD, abs_path);
	}

	xSemaphoreGive(g_util_mutex);

	return ret;
}

int app_uti_del_protect_file(char *abs_path, char *fname)
{
	int ret = 0;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);
#ifndef CONFIG_APP_DRONE
	int result = 0;

	result = rec_filenode_del(T_PROTECT, fname);
	if(result == 0) {
		f_chmod(abs_path, AM_SYS | AM_ARC, AM_ARC | AM_RDO | AM_HID | AM_SYS);
		if (fs_cmd_rm(abs_path) == FR_OK)
			ret = OK;
		else
			ret = DELETE_FILE_ERR;
		rec_thumbnail_del(T_PROTECT, abs_path);
	}

#endif
	xSemaphoreGive(g_util_mutex);
	return ret;
}

int app_uti_del_picture_file(char *abs_path, char *fname)
{
	int ret = 0;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	del_snapshot_file(fname);

	if (fs_cmd_rm(abs_path) == FR_OK)
		ret = OK;
	else
		ret = DELETE_FILE_ERR;

	xSemaphoreGive(g_util_mutex);

	return ret;
}

int app_uti_set_stream_uvc(int view_enable)
{
	int ret = 0;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if( view_enable == 1 ) {
		UTIL_PRINT(SYS_INFO, "uvc:view_enable\n");
		mf_set_uvc_view(1);
	} else if( view_enable == 0 ) {
		UTIL_PRINT(SYS_INFO, "uvc:view_disable\n");
		mf_set_uvc_view(0);
	} else {
		UTIL_PRINT(SYS_ERR, "Invalid parameters\n");
		ret = INVALID_PARAM;
		goto finally;
	}


finally:

	xSemaphoreGive(g_util_mutex);

	return ret;
}

void app_uti_set_stream_preview(int enable)
{

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	//enable preview encoder when no other user set preview
	mf_set_preview(enable);

	xSemaphoreGive(g_util_mutex);

	return;
}

void app_uti_restart_playback(int force_record)
{
	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	schedrec_suspend_restart(force_record);
#ifndef CONFIG_APP_DRONE
	reclapse_suspend_restart(force_record);
#endif

	xSemaphoreGive(g_util_mutex);

	return;
}

/***
int app_uti_set_stream_playback(char *fname, char *frename, int filetype, SocketItem_t *psocket)
{
	char abs_path[300] = {0};
	int ret = 0;
	int rc = 0;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	schedrec_suspend();	//close record
#ifndef CONFIG_APP_DRONE
	reclapse_suspend();
#endif
	switch(filetype)
	{
		case 0:	//record file
			sprintf(abs_path, "%s/%s/%s", SD_ROOT,SD_SCHED_PATH, fname);
			break;
		case 1:	//protect file
			sprintf(abs_path, "%s/%s/%s", SD_ROOT,SD_PROTECT_PATH, fname);
			break;
	}

	if(strlen(psocket->rtsp_fname)>0)
		pb_stop(psocket->addr.sin_addr.s_addr);

	memset(frename, 0, sizeof(frename));
	rc = pb_receive_cmd(abs_path,psocket->addr.sin_addr.s_addr, frename);

	if(rc == PB_SD_NOT_EXIST)
		ret = SD_NOT_EXIST;
	else if(rc == PB_INIT_FAIL)
		ret = PB_INIT_FAIL;
	else if(rc == PB_USER_IS_FULL)
		ret = PB_USER_IS_FULL;
	else if(rc == PB_CHECK_FILE_ERR)
		ret = PB_CHECK_FILE_ERR;


	UTIL_PRINT(SYS_DBG, "abs_path = %s\n", abs_path);
	UTIL_PRINT(SYS_DBG, "frename = %s\n", frename);
	xSemaphoreGive(g_util_mutex);

	return ret;
}

# if 0
int app_uti_del_stream_preview(SocketItem_t *psocket)
{
	int ret = 0;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	conn_list_del(&preview_list, psocket->addr.sin_addr.s_addr);
	if(conn_list_len(&preview_list)==0)
		mf_set_preview(0);

	xSemaphoreGive(g_util_mutex);

	return ret;
}
#endif

int app_uti_del_stream_playback(SocketItem_t *psocket)
{
	int ret = 0;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	pb_stop(psocket->addr.sin_addr.s_addr);

	xSemaphoreGive(g_util_mutex);

	return ret;
}
***/


int modify_fw_upgrade_status(int val)
{
	int ret = 0;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	if(g_fw_upgrade_status == val)
		return UPGRADE_IS_RUNNING;
	else
		g_fw_upgrade_status = val;

	xSemaphoreGive(g_util_mutex);

	return ret;
}

/***
int app_uti_dwnlod_fw_create_socket(uint16_t *port, int filesize, SocketItem_t *psocket)
{
	int ret = 0;

	if( filesize > 0 )
	{
		if (check_fwupgrad_task() != 0)
		{
			UTIL_PRINT(SYS_ERR, "fwupgrade task is already running.\n");
			ret = FWUPGRADE_IS_RUNNING ;
			goto finally;
		}

		//set fw_upgrade flag for heart beat
		//psocket->fw_upgrade = 1;

		//download firmware process
		if(socket_download_fw_create(port, filesize, psocket)==pdFAIL)
		{
			UTIL_PRINT(SYS_ERR, "create download socket fail\n");
			ret = CREATE_DOWNLOAD_SOCKET_ERR;	//create socket fail
			goto finally;
		}
	}
	else
	{
		UTIL_PRINT(SYS_ERR, "Invalid Parameters\n");
		ret = INVALID_PARAM;
		goto finally;
	}
finally:

	xSemaphoreGive(g_util_mutex);

	return ret;
}

int app_uti_get_fw_dwnlod_stat(SocketItem_t *psocket)
{
	int ret = 0;
	int fw_download_status = -1;

	xSemaphoreTake(g_util_mutex, portMAX_DELAY);

	fw_download_status = get_download_fw_status(psocket);
	if (fw_download_status == UPGRADE_RECEIVE_SIZE_ERR)
		ret = UPGRADE_RECEIVE_SIZE_ERR;
	else if(fw_download_status == UPGRADE_VERSION_ERR)
		ret = UPGRADE_VERSION_ERR;
	else if(fw_download_status == UPGRADE_MD5_ERR)
		ret = UPGRADE_MD5_ERR;
	else if(fw_download_status == UPGRADE_FW_AND_PLATFORM_DISMATCH)
		ret = UPGRADE_FW_AND_PLATFORM_DISMATCH;
	else if(fw_download_status == -1)
		ret = FWUPGRADE_INIT_ERR; //UpgradeFW Init failed.
	UTIL_PRINT(SYS_INFO, "fw_download_status: %d\n", ret);

//finally:

	xSemaphoreGive(g_util_mutex);

	return ret;
}
***/



int app_uti_get_timezone_num(char *time_zone)
{
	uint8_t time_zone_num = -1;
	int timezone_tbl_size = ARRAY_SIZE(timezone_tbl);
	int index = 0;

	UTIL_PRINT(SYS_DBG, "time_zone = %s\n", time_zone);
	xSemaphoreTake(g_util_mutex, portMAX_DELAY);
	for(index = 0; index < timezone_tbl_size; index++) {
		if(!strcmp( time_zone, timezone_tbl[index].time_zone) ) {
			time_zone_num = timezone_tbl[index].num;
			break;
		}
	}

	if(time_zone_num == -1) {
		UTIL_PRINT(SYS_ERR, "Invalid Parameters\n");
		time_zone_num = INVALID_PARAM;
	}

	UTIL_PRINT(SYS_DBG, "num = %d\n", timezone_tbl[index].num);
	xSemaphoreGive(g_util_mutex);

	return time_zone_num;
}


static int G_preview_audio = PREVIEW_AUDIO_ON;

int app_uti_set_preview_audio_mode(int flag)
{
#if RTSP_PREVIEW_AUDIO
	xSemaphoreTake(g_util_mutex, portMAX_DELAY);
	if(flag == PREVIEW_AUDIO_ON)
		UTIL_PRINT(SYS_DBG, "Enable preview audio\n");
	else if(flag == PREVIEW_AUDIO_OFF)
		UTIL_PRINT(SYS_DBG, "Disable preview audio\n");
	else{
		UTIL_PRINT(SYS_ERR, "ERROR: wrong setting\n");
		xSemaphoreGive(g_util_mutex);
		return 0;
	}
	G_preview_audio = flag;
	
	xSemaphoreGive(g_util_mutex);
	return 0;
#else

	UTIL_PRINT(SYS_ERR, "ERROR: NO SUPPORT\n");
	return -1;
#endif
}

int app_uti_get_preview_audio_mode(void)
{
#if RTSP_PREVIEW_AUDIO
	int tmp = 0;
	xSemaphoreTake(g_util_mutex, portMAX_DELAY);
	tmp = G_preview_audio;
	xSemaphoreGive(g_util_mutex);
	return tmp;
#else

	UTIL_PRINT(SYS_ERR, "ERROR: NO SUPPORT\n");
	return -1;
#endif
}
