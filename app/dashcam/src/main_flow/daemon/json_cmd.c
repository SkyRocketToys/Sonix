/**
 * @file
 * this is application file for paring cmd
 * @author Algorithm Dept Sonix.
 */

#include <FreeRTOS.h>
#include <bsp.h>
#include <task.h>
#include <queue.h>
#include <nonstdlib.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys_clock.h>
#include <sglib.h>
#include <libmid_json/snx_json.h>
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
#include "json_cmd.h"
#include "json_cmd_internal.h"
#include "socket_ctrl.h"
#include "snapshot.h"
#include "audio_main.h"
#include "lwip/inet.h"
#include <wifi/wifi_api.h>
#include "../sensor_cap/sensor_capability.h"
#include "online_fw_upgrade.h"
#include "user_param.h"
//nclude "../file_protection/file_protection.h"
//#ifndef CONFIG_APP_DRONE
#include "mcu.h"
//#endif
#include "fwversion.h"
#include "main_flow.h"
#include "watch_task.h"
#include "rec_seed.h"
#include "utility.h"
#include "../errno.h"
#include "video_main.h"
#include <libmid_mcu/mcu_ctrl.h>
#if CONFIG_MAVLINK_WIFI
#include "../ArduPilot/mavlink_wifi.h"
#endif

#ifdef NOT_DEFINED_ELSWHERE_ADDR_COMPARATOR
#define ADDR_COMPARATOR(e1, e2) (e1->ip_addr - e2->ip_addr)
#endif

#ifndef CONFIG_APP_DRONE
#include "mcu.h"
#endif

snx_jtoken_t *tokener = NULL;
snx_json_t *parse_json;
uint8_t format_status = 0;
ConnItem_t *playback_list=NULL, *preview_list=NULL; 
char g_str_timezone[10];
buffer_t *g_resp_buf = NULL;

int factory_mode = 0;

#if LIMIT_PLAYBACK_AND_DOWNLOAD_CONNECTION
int dwnload_and_playback_max_conn_number = 0;
int dwnlod_and_pb_conn_num = 0;
dwnload_pb_conn_t dwnload_pb_conn[MAX_CONNECT_NUM] = {0};
#endif

json_command_source_t json_cmd_ctrl_src = JSON_CMD_SRC_ETH;		/* chkuo add, add json commands main control source */

#define X(str, var, num, fun) const char *Json_##var##_CMD = #str;
	CMD_LIST
#ifdef CMD_LIST_EXT
	CMD_LIST_EXT
#endif
#undef X

#define X(str, var, num, fun) static buffer_t *js_##fun(snx_json_t *parse_json, void* param);
	CMD_LIST
#ifdef CMD_LIST_EXT
	CMD_LIST_EXT
#endif
#undef X

ActionHandler_t ActHandler[] = 
{
#define X(str, var, num, fun) {#str, js_##fun},
	CMD_LIST
#ifdef CMD_LIST_EXT
	CMD_LIST_EXT
#endif
#undef X
	{ NULL, NULL }
};
#define ACT_HANDLER_NUM (sizeof(ActHandler)/sizeof(ActHandler[0]))

static int conn_list_add(ConnItem_t **plist, uint32_t addr, char *mac);
static int conn_list_del(ConnItem_t **plist, char *mac);
int conn_list_len(ConnItem_t **plist);

#if LIMIT_PLAYBACK_AND_DOWNLOAD_CONNECTION
void set_max_dwnlod_and_pb_max_conn_num(int num)
{
	dwnload_and_playback_max_conn_number = num;
	JSON_PRINT_QUEUE("Download and PlayBack Max conn = %d\n", dwnload_and_playback_max_conn_number);
	return;
}

void del_dwnlod_and_pb_conn_num(void)
{
	dwnlod_and_pb_conn_num--;
	JSON_PRINT_QUEUE("Download and Playback conn = %d\n", dwnlod_and_pb_conn_num);
	return;
}

void set_dwnlod_pb_conn_status(char mac, int status)
{
	int index = 0;

	for(index; index < dwnload_and_playback_max_conn_number; index++) {
		if (strcmp(mac, dwnload_pb_conn[index].mac) == 0) {
			JSON_PRINT(SYS_DBG, "Set MAC %s status to %d\n", mac, status);
			dwnload_pb_conn[index].status = status;
			break;
		}
	}
}


static int is_addr_exist_in_dwnlod_pb_conn(char *mac)
{
	int index = 0;

	for (index; index < dwnload_and_playback_max_conn_number; index++) {
		if (strcmp(mac, dwnload_pb_conn[index].mac) == 0) {
			JSON_PRINT(SYS_DBG, "MAC addr %s found\n", mac);
			return 1;
		}
	}

	if (index >= dwnload_and_playback_max_conn_number) {
		JSON_PRINT(SYS_ERR, "MAC addr %s no found\n", mac);
		return 0;
	}
}

static void add_conn_to_pb_and_dwnlod(int socket_addr, char *mac, int status)
{
	int index = 0;
	
	for (index; index < dwnload_and_playback_max_conn_number; index++) {
		if (dwnload_pb_conn[index].ip == 0 || dwnload_pb_conn[index].status == 0)
			break;
	}

	if (index >= dwnload_and_playback_max_conn_number) {
		JSON_PRINT(SYS_ERR, "DownloadFile and Playback connections is up limit.\n");
		JSON_PRINT(SYS_ERR, "Max Conn = %d\n", dwnload_and_playback_max_conn_number);
	} else {
		JSON_PRINT(SYS_DBG, "add Socket ip %d MAC %sto dwnload_pb_conn.\n", socket_addr, mac);
		dwnload_pb_conn[index].ip = socket_addr;
		sprintf(dwnload_pb_conn[index].mac, "%s", mac);
		dwnload_pb_conn[index].status = status;
	}

	/***list all addr for debug
	for(index=0; index < MAX_CONNECT_NUM; index++)
	{
		 JSON_PRINT(SYS_DBG, "dwnload_pb_conn[%d].ip = %d\n", index, dwnload_pb_conn[index].ip);
	}
	***/
}

void delete_conn_from_pb_and_dwnlod(char *mac)
{
	int index = 0;
	
	for (index; index < MAX_CONNECT_NUM; index++) {
		if (strcmp(mac, dwnload_pb_conn[index].mac) == 0) {
			JSON_PRINT(SYS_DBG, "delete Socket MAC=%d from dwnload_pb_conn.\n", mac);
			dwnload_pb_conn[index].ip = 0;
			memset(dwnload_pb_conn[index].mac, 0x0, sizeof(dwnload_pb_conn[index].mac));
			dwnload_pb_conn[index].status = 0;
			break;
		}
	}
}
#endif

#if 0
static void reboot_task(void *args)
{
	JSON_PRINT_QUEUE("wait for 3 seconds, the system will reboot\n");
	vTaskDelay(3000/portTICK_PERIOD_MS);
	all_task_uinit(0);
	if(snx_nvram_reset_to_default(NVRAM_RTD_ALL, NULL, NULL) != NVRAM_SUCCESS)
	{
		JSON_PRINT_QUEUE_ERR("NVRAM Reset to default fail!!!!! \n");
	}
	reboot();
	vTaskDelete(NULL);
}
#endif

/**
 * @brief show ip address in list
 * @param plist pointer for list
 */
static void conn_list_show(ConnItem_t **plist)
{
	ConnItem_t *pItem;
	
	pItem = *plist;
	while (pItem) {
		JSON_PRINT(SYS_DBG, "\t %s(%x)\n", pItem->mac, pItem->ip_addr);
		pItem = pItem->next;
	}
}

/**
 * @brief add ip address to list
 * @param plist pointer for list
 * @return return pdPASS if add to list success
 */
static int conn_list_add(ConnItem_t **plist, uint32_t addr, char *mac)
{
	ConnItem_t *item, *ret;
	
	item = pvPortMalloc(sizeof(ConnItem_t), GFP_KERNEL, MODULE_APP);
	if (item == NULL) {
		JSON_PRINT(SYS_ERR, "ConnItem_t allocate fail (size = %d)\n", sizeof(ConnItem_t));
		return pdFAIL;
	}
	
	item->ip_addr = addr;
	sprintf(item->mac, "%s", mac);
	SGLIB_LIST_ADD_IF_NOT_MEMBER(ConnItem_t, *plist, item, ADDR_COMPARATOR, next, ret);
	if (ret) {	//has the same member in list
		vPortFree(item);
		return pdFAIL;
	}
	conn_list_show(plist);//test
	return pdPASS;
}

/**
 * @brief delete ip address from list
 * @param plist pointer for list
 * @param deleted ip addr
 * @return return pdPASS if success
 */
static int conn_list_del(ConnItem_t **plist, char *mac)
{
	ConnItem_t *ret, del_item;
	sprintf(del_item.mac, "%s", mac);

	SGLIB_LIST_DELETE_IF_MEMBER(ConnItem_t, *plist, &del_item, ADDR_COMPARATOR, next, ret);
	//JSON_PRINT(SYS_DBG, "ret = %x, delete mac = %s\n", (int)ret, mac);
	if (ret)
		vPortFree(ret);

	return pdPASS;
}

/**
 * @brief report num of element in list
 * @param plist pointer for list
 * @return return length of list
 */
int conn_list_len(ConnItem_t **plist)
{
	int len;

	SGLIB_LIST_LEN(ConnItem_t, *plist, next, len) 

		return len;
}

/**
 * @brief interface function - report sd format is on processing or not
 * @return return 1 if format is on processing
 */
int json_get_format_status(void)
{
	return format_status;
}

/**
 * @brief interface function - notice ip address to disconnect
 * @param rtsp_fname filename to playback in this ip addr
 * @param ip_addr ip addr to disconnect
 */
void json_disconnect(char *rtsp_fname, char *mac, int type)
{
	//check to notice rtsp to stop video stream once socet disconnect
	pb_stop(mac);

	//stop uploading file
	stopfileupload(mac, UPLOAD_FG);
	stopfileupload(mac, UPLOAD_BG);

	if (type != SOCKETDEL_EXCEPT_RTSP) {
		//stop rtsp
		close_rtsp_stream_by_mac(mac);
		
		conn_list_del(&playback_list, mac);
		if (conn_list_len(&playback_list)) {
			//show playback list
			JSON_PRINT(SYS_DBG, "playback ip list(%d):\n", conn_list_len(&playback_list));
			conn_list_show(&playback_list);
		}
		//restart record when all user leave playback
		if (conn_list_len(&playback_list) == 0) {
			schedrec_suspend_restart(0);
		#ifndef CONFIG_APP_DRONE
			reclapse_suspend_restart(0);
		#endif
		}
		conn_list_del(&preview_list, mac);
		JSON_PRINT(SYS_DBG, "del preview ip list(%d):\n", conn_list_len(&preview_list));
		if (conn_list_len(&preview_list)) {
			//show preview list
			JSON_PRINT(SYS_DBG, "preview ip list(%d):\n", conn_list_len(&preview_list));
			conn_list_show(&preview_list);
		}

		#if defined (CONFIG_SYSTEM_PLATFORM_SN98293)
		//TODO: if wifi and uvc exist at the same time ??
		#else			
		if (conn_list_len(&preview_list) == 0)
			mf_set_preview(0);
		#endif //defined (CONFIG_SYSTEM_PLATFORM_SN98293) end
		
	}
}

int js_stream_start(int live_view, char *filename, int filetype, char *mac, unsigned long clientip)
{
	int ret = OK;
	char abs_path[128] = {0};
	char frename[128] = {0};

	if (live_view) {
		//enable preview encoder when no other user set preview
		if (conn_list_len(&preview_list) == 0)
			mf_set_preview(1);
		
		//restart record when all user leave playback
		if (conn_list_len(&playback_list) == 0) {
			/*restore wb mem creation*/
			schedrec_set_wb_mem(1);

			schedrec_suspend_restart(0);
#ifndef CONFIG_APP_DRONE
			reclapse_suspend_restart(0);
#endif
		}
		conn_list_add(&preview_list, clientip, mac);
#if MSG_TONE_ENABLE
		aac_tone_play(NVRAM_START_PREVIEW_TONE);
#endif
	} else {
#if 0//LIMIT_PLAYBACK_AND_DOWNLOAD_CONNECTION
		if (is_addr_exist_in_dwnlod_pb_conn(psocket->mac)) {
			dwnlod_and_pb_conn_num++;
			JSON_PRINT(SYS_DBG, "Download and Playback conn = %d\n", dwnlod_and_pb_conn_num);
			psocket->status = DL_PB_PLAYBACK;
		} else {
			if (dwnlod_and_pb_conn_num >= dwnload_and_playback_max_conn_number) {
				JSON_PRINT(SYS_ERR, "DownloadFile and Playback connections is up limit.\n");
				JSON_PRINT(SYS_ERR, "Max Conn = %d, Current Conn = %d\n", dwnload_and_playback_max_conn_number, dwnlod_and_pb_conn_num);
				ret = OVER_DOWLOD_AND_PB_MAX_CONN;
				goto resp_json;
			} else {
				psocket->status = DL_PB_PLAYBACK;
				add_conn_to_pb_and_dwnlod(psocket->addr.sin_addr.s_addr, psocket->mac, DL_PB_PLAYBACK);
				dwnlod_and_pb_conn_num++;
				JSON_PRINT(SYS_DBG, "Download and Playback conn = %d\n", dwnlod_and_pb_conn_num);
			}
		}
#endif

		schedrec_suspend();	//close record
#ifndef CONFIG_APP_DRONE
		reclapse_suspend();
#endif
		switch (filetype) {
			case 0:	//record file
				sprintf(abs_path, "%s/%s/%s", SD_ROOT, SD_SCHED_PATH, filename);
				break;
			case 1:	//protect file
				sprintf(abs_path, "%s/%s/%s", SD_ROOT, SD_PROTECT_PATH, filename);
				break;
		}

		memset(frename, 0, sizeof(frename));
		ret = pb_receive_cmd(abs_path, clientip, mac, frename);
		if (ret == 0) {
			conn_list_add(&playback_list, clientip, mac);
		}
		JSON_PRINT(SYS_DBG, "abs_path = %s\n", abs_path);
		JSON_PRINT(SYS_DBG, "frename = %s\n", frename);
	}
	return ret;

}

void js_stream_finish(int live_view, char *mac)
{
	if (live_view) {
	#if MSG_TONE_ENABLE
		aac_tone_play(NVRAM_END_PREVIEW_TONE);
	#endif
		conn_list_del(&preview_list, mac);
	
	#if defined (CONFIG_SYSTEM_PLATFORM_SN98293)
		//TODO: if wifi and uvc exist at the same time ??
	#else
		if (conn_list_len(&preview_list) == 0)
			mf_set_preview(0);
	#endif //defined (CONFIG_SYSTEM_PLATFORM_SN98293) end
	
	} else {
		pb_stop(mac);
		conn_list_del(&playback_list, mac);

#if 0//LIMIT_PLAYBACK_AND_DOWNLOAD_CONNECTION
		if (is_addr_exist_in_dwnlod_pb_conn(psocket->mac)) {
			psocket->status = DL_PB_INIT;
			set_dwnlod_pb_conn_status(psocket->mac, DL_PB_INIT);
			dwnlod_and_pb_conn_num--;
			JSON_PRINT(SYS_DBG, "Download and Playback conn = %d\n", dwnlod_and_pb_conn_num);
		} else {
			psocket->status = DL_PB_INIT;
			JSON_PRINT(SYS_DBG, "Download and Playback conn = %d\n", dwnlod_and_pb_conn_num);
		}
#endif
	}
}


/**
 * @brief interface function - json cmd control initialization
 * @return return pdPASS if success
 */
int json_init(void)
{
	if (tokener == NULL) {
		if (!(tokener = snx_json_token_new())) {
			JSON_PRINT(SYS_ERR, "Couldn't create test tokener\n");
			return pdFAIL;
		}
	}
	
	if(g_resp_buf == NULL) {
		g_resp_buf = buffer_new();
		if (!g_resp_buf) {
			JSON_PRINT(SYS_ERR, "ERROR: Couldn't create resp buf\n");
			return pdFAIL;
		}
		
	}
	
	rtsp_reg_stream_start(js_stream_start);
	rtsp_reg_stream_finish(js_stream_finish);

	return pdPASS;
}

/**
 * @brief interface function - json cmd control uninitialization
 */
void json_uninit(void)
{
	if (tokener) {
		snx_json_tokener_free(tokener);
		tokener = NULL;
	}
	
	if(g_resp_buf != NULL) {
		buffer_free(g_resp_buf);
		g_resp_buf = NULL;
	}
}

/**
 * @brief interface function - parsing json cmd and to execute corresponding API
 * @param pbuf cmd to parse
 * @param param pointer for socket info
 * @return pointer for data to response to APP
 */
buffer_t *parse_json_cmd(buffer_t *pbuf, void *param)
{
	char *cmd;
	int i;
	buffer_t *resp_buf = NULL;
	SocketItem_t *psocket = (SocketItem_t *)param;
	
	if (tokener == NULL) {
		JSON_PRINT(SYS_ERR, "tokener is invalid\n");
		goto end;
	}

	JSON_PRINT(SYS_DBG, "data = \n%s size=%d, fd=%d\n",
	                 pbuf->data, pbuf->use_size, psocket->fd);

	snx_json_tokener_reset(tokener);

	if (!(parse_json =  snx_json_token_parse(tokener, pbuf->data, pbuf->use_size))) {
		JSON_PRINT(SYS_ERR, "Couldn't create test parse json object: %s\n", pbuf->data);
		goto end;
	}

	if (snx_json_obj_get_str(parse_json, "type", &cmd)) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON string obj \n");
		goto release;
	}

	/* chkuo add, add json command control source object */
	if (snx_json_obj_add_int(parse_json, Json_MainCtrl_SRC_CMD, (int)json_cmd_ctrl_src) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't create json command main control source object\n");
	}

	for (i = 0; i < ACT_HANDLER_NUM; i++) {
		if (strcmp(cmd, ActHandler[i].cmd) == 0) {
			resp_buf = ActHandler[i].handler(parse_json, psocket);
			break;
		}
	}

	if (i == ACT_HANDLER_NUM)
		JSON_PRINT(SYS_ERR, "cmd = %s not found\n", cmd);

release:
	snx_json_obj_free(parse_json);
end:
	return resp_buf;
}

/*
   static buffer_t* test_cmd(snx_json_t *parse_json, void* param)
   {
   int count;
   snx_json_t *resp_json;
   buffer_t * resp_buf = NULL;
   char *output;
//JSON_PRINT_QUEUE("\n");
if ( snx_json_obj_get_int(parse_json, "Count", &count) != 0)
{
JSON_PRINT_QUEUE_ERR("Couldn't get JSON int obj \n");
}

if (!(resp_json = snx_json_obj_new()))
{
JSON_PRINT_QUEUE_ERR("Couldn't create JSON obj \n");
return NULL;
}

if ( snx_json_obj_add_str(resp_json, "type", "resp") != 0)
{
JSON_PRINT_QUEUE_ERR("Couldn't add JSON string obj \n");
goto release_json;
}

if (  snx_json_obj_add_int(resp_json, "status", 12345) != 0)
{
JSON_PRINT_QUEUE_ERR("Couldn't add JSON integer obj \n");
goto release_json;
}
resp_buf = buffer_new();
output = snx_json_object_to_json_string(resp_json);
buffer_add(resp_buf, output,  strlen((char*)output));


release_json:
snx_json_obj_free(resp_json);
return resp_buf;


}*/

#if 0
static  SDSTATE chk_sd_state()
{
	int result;
	if(get_sd_umount_err())
	{
		result=get_sd_status();
		if(result==FR_NO_FILESYSTEM)//have sdcard but not fat32
		{
			return CARD_FORMAT_ERROR;

		}else
		{			
			return CARD_NOT_EXIST;
		}	 
	}
	return  CARD_OK;
}
#endif

static buffer_t* js_heartbeat(snx_json_t *parse_json, void* param)
{
	socket_update_tick((SocketItem_t *)param);
	return NULL;
}

void set_wifi_param(void)
{
	int setting = 0;
	
	if (check_wifi_param_bit() == 1) {
		WiFi_QueryAndSet(SET_SEND_DISCONNECT, NULL, NULL);
		WiFi_QueryAndSet(SET_BEACON_OFF, NULL, NULL);
		setting = 1;
	}
	if (chk_ssid_bit() == 1) {
		char ssid[32] = {'\0'};
		unsigned short unsignedssidlen;
		if (snx_nvram_string_get("WIFI_DEV", "AP_SSID_INFO", ssid) == NVRAM_SUCCESS) {
			unsignedssidlen = strlen(ssid);
			JSON_PRINT(SYS_DBG, "ssid=%s\n", ssid);
			JSON_PRINT(SYS_DBG, "ssidlen=%d\n", unsignedssidlen);
			WiFi_QueryAndSet(SET_BEACON_SSID, (unsigned char *)ssid, &unsignedssidlen);
		}
		set_ssid_bit(0);
	}
	if (chk_pwd_bit() == 1) {
#if RTSP_CHECK_LEGAL
		/* For customer's request. */
		reboot();
#else
		char pwd[64] = {'\0'};
		unsigned short unsignedpwdlen = 0;
		enc_auth_t enc = AUTH_NONE;
		if( snx_nvram_integer_get("WIFI_DEV", "AP_AUTH_MODE", &enc) != NVRAM_SUCCESS) {
			JSON_PRINT(SYS_ERR, "cannot get ap auth mode !\n");
		}
		else if(AUTH_NONE == enc) {
			WiFi_QueryAndSet(SET_SECURITY_NONE, NULL, NULL);
			JSON_PRINT(SYS_DBG, "Wifi authentication set to open mode\n");
		}
		else if (snx_nvram_string_get("WIFI_DEV", "AP_KEY_INFO", pwd) == NVRAM_SUCCESS) {
			unsignedpwdlen = strlen(pwd);
			JSON_PRINT(SYS_DBG, "pwd=%s\n", pwd);
			JSON_PRINT(SYS_DBG, "pwdlen=%d\n", unsignedpwdlen);
			if (enc == AUTH_WEP) {
				WiFi_QueryAndSet(SET_SECURITY_WEP, (unsigned char *)pwd, &unsignedpwdlen);
			} else if (enc == AUTH_WPA || enc == AUTH_WPA2) {
				WiFi_QueryAndSet((enc == AUTH_WPA)?(SET_SECURITY_WPA):(SET_SECURITY_WPA2), (unsigned char *)pwd, &unsignedpwdlen);
			} else {
				JSON_PRINT(SYS_ERR, "WIFI Pwd Setting ERROR\n");
			}
		}
		
		set_pwd_bit(0);
#endif
	}
	if (chk_channel_bit() == 1) {
		int channel_idx;
		unsigned char unsgined_channel_idx;
		unsigned short unsignedlen;
		if (snx_nvram_integer_get("WIFI_DEV", "AP_CHANNEL_INFO", &channel_idx) == NVRAM_SUCCESS) {
			unsignedlen = 4;
			unsgined_channel_idx = channel_idx;
			JSON_PRINT(SYS_DBG, "channel_idx=%d\n", unsgined_channel_idx);
			WiFi_QueryAndSet(SET_HW_CHANNEL, &unsgined_channel_idx, &unsignedlen);
		}
		set_channel_bit(0);
	}
	if (setting == 1) {
		WiFi_QueryAndSet(SET_BEACON_ON, NULL, NULL);
		WiFi_QueryAndSet(SET_SEND_DISCONNECT, NULL, NULL);
		socket_delete_all();
	}
}

static buffer_t *js_set_channel(snx_json_t *parse_json, void *param)
{
	int channel_idx = -1;
	snx_json_t *resp_json = NULL;
	char *output = NULL;
	int ret = 0;
	int nvram_err = 0;

	if (snx_json_obj_get_int(parse_json, "channel", &channel_idx)) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	}

	if (snx_nvram_integer_set("WIFI_DEV", "AP_CHANNEL_INFO", channel_idx) != NVRAM_SUCCESS) {
		JSON_PRINT(SYS_ERR, "Save Flash Fail(%d)", nvram_err);
		ret = NVRAM_ERR;
		goto response_json;
	}
	// setting wifi
	set_channel_bit(1);

response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "setchannel_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status", DAEMON_ERRNO(CMD_SETCHANNEL, ret)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	
	
	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;

}
static buffer_t *js_set_pwd(snx_json_t *parse_json, void *param)
{
	int ret = 0;
	snx_json_t *resp_json = NULL;
	char *pwd = NULL, *output = NULL;
	int nvram_err = 0;

	if (snx_json_obj_get_str(parse_json, "pwd", &pwd)) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	}

	JSON_PRINT(SYS_ERR, "PWD: %s\n", pwd);

	if (strlen(pwd) == WIFI_PWD_LENTH) {
		
		enc_auth_t auth_mode;
		
		if( snx_nvram_integer_get("WIFI_DEV", "AP_AUTH_MODE", &auth_mode) != NVRAM_SUCCESS) {
			JSON_PRINT(SYS_ERR, "cannot get ap auth mode !\n");
			ret = NVRAM_ERR;
			goto response_json;
		}

		if(AUTH_WPA2 != auth_mode) {
			/*
			**	Force use of WPA2 authentication...
			*/
			if( snx_nvram_integer_set("WIFI_DEV", "AP_AUTH_MODE", AUTH_WPA2) != NVRAM_SUCCESS) {
				JSON_PRINT(SYS_ERR, "Save Flash Fail(%d)", nvram_err);
				ret = NVRAM_ERR;
				goto response_json;
			}
		}

		if (snx_nvram_integer_set("WIFI_DEV", "AP_KEY_LEN", strlen(pwd)) != NVRAM_SUCCESS) {
			JSON_PRINT(SYS_ERR, "Save Flash Fail(%d)", nvram_err);
			ret = NVRAM_ERR;
			goto response_json;
		}

		if (snx_nvram_string_set("WIFI_DEV", "AP_KEY_INFO", pwd) != NVRAM_SUCCESS) {
			JSON_PRINT(SYS_ERR, "Save Flash Fail(%d)", nvram_err);
			ret = NVRAM_ERR;
			goto response_json;
		}
	}
	else {
		JSON_PRINT(SYS_ERR, "PWD wrong length: %i\n", strlen(pwd));
		ret = INVALID_PARAM;
		goto response_json;
	}

	set_pwd_bit(1);

response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "setpwd_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status", DAEMON_ERRNO(CMD_SETPWD, ret)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}


	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output,  strlen((char *)output));


release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}

static buffer_t *js_set_ssid(snx_json_t *parse_json, void *param)
{
	int ret = 0;
	snx_json_t *resp_json = NULL;
	char *ssid = NULL, *output = NULL;
	int nvram_err = 0;

	if (snx_json_obj_get_str(parse_json, "ssid", &ssid)) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	}

	if (strlen(ssid) <= WIFI_SSID_MAX_LENTH) {
		if (snx_nvram_string_set("WIFI_DEV", "AP_SSID_INFO", ssid) != NVRAM_SUCCESS) {
			JSON_PRINT(SYS_ERR, "Save Flash Fail(%d)", nvram_err);
			ret = NVRAM_ERR;
			goto response_json;
		}
	} else {
		JSON_PRINT(SYS_ERR, "SSID lenth is largest 32\n");
		ret = INVALID_PARAM;
		goto response_json;
	}

	set_ssid_bit(1);

response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "setssid_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status", DAEMON_ERRNO(CMD_SETSSID, ret)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}


	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}

static buffer_t *js_set_wifiparameters(snx_json_t *parse_json, void *param)
{
	int ret = 0;
	snx_json_t *resp_json = NULL;
	char *ssid = NULL, *output = NULL, *pwd = NULL;
	int channel_idx = -1;
	enc_auth_t enc = AUTH_NONE;
#ifdef ENGINNER_MODE
	int retry_count = 0;
	int udp_type = 0;
#endif

	if (!snx_json_obj_get_str(parse_json, "ssid", &ssid)) {
		if (strlen(ssid) <= WIFI_SSID_MAX_LENTH) {
			if (snx_nvram_string_set("WIFI_DEV", "AP_SSID_INFO", ssid) != NVRAM_SUCCESS) {
				JSON_PRINT(SYS_ERR, "set wifi ssid nvram fail \n");
				ret = NVRAM_ERR;
				goto response_json;
			}
		} else {
			JSON_PRINT(SYS_ERR, "SSID lenth is largest 32\n");
			ret = INVALID_PARAM;
			goto response_json;
		}
		set_ssid_bit(1);
	}

	if (!snx_json_obj_get_str(parse_json, "pwd", &pwd)) {

		if( snx_nvram_integer_get("WIFI_DEV", "AP_AUTH_MODE", (int *)&enc) != NVRAM_SUCCESS) {
			JSON_PRINT(SYS_ERR, "cannot get ap auth mode !\n");
			ret = NVRAM_ERR;
			goto response_json;
		}
		JSON_PRINT(SYS_DBG, "WIFI Auth ENC %d\n", enc);
		if (enc == AUTH_WEP) {
			if (strlen(pwd) == WIFI_PWD_LENTH) {
				if (snx_nvram_string_set("WIFI_DEV", "AP_KEY_INFO", pwd) != NVRAM_SUCCESS) {
					JSON_PRINT(SYS_ERR, "set wifi pwd nvram fail\n");
					ret = NVRAM_ERR;
					goto response_json;
				}
			} else {
				JSON_PRINT(SYS_ERR, "PWD lenth isn't equal 5\n");
				ret = INVALID_PARAM;
				goto response_json;
			}
		} else if ((enc == AUTH_WPA) || (enc == AUTH_WPA2)) {
			if ((strlen(pwd) >= 8) && (strlen(pwd) <= 63)) {
				if (snx_nvram_string_set("WIFI_DEV", "AP_KEY_INFO", pwd) != NVRAM_SUCCESS) {
					JSON_PRINT(SYS_ERR, "set wifi pwd nvram fail\n");
					ret = NVRAM_ERR;
					goto response_json;
				}
			} else {
				JSON_PRINT(SYS_ERR, "PWD lenth is out of range(8~63 bytes)\n");
				ret = INVALID_PARAM;
				goto response_json;
			}
			
		} else {
			JSON_PRINT(SYS_ERR, "Unknow encrypt protocol\n");
			ret = INVALID_PARAM;
			goto response_json;
		}
		set_pwd_bit(1);
	}

	if (!snx_json_obj_get_int(parse_json, "channel", &channel_idx)) {
		if (snx_nvram_integer_set("WIFI_DEV", "AP_CHANNEL_INFO", channel_idx) != NVRAM_SUCCESS) {
			JSON_PRINT(SYS_ERR, "set wifi channel nvram fail\n");
			ret = NVRAM_ERR;
			goto response_json;
		}
		set_channel_bit(1);
	}

#ifdef ENGINNER_MODE
	if (!snx_json_obj_get_int(parse_json, "tx_retry", &retry_count)) {
		if (snx_nvram_integer_set("WIFI_DEV", "TX_RETRY", retry_count) != NVRAM_SUCCESS) {
			JSON_PRINT(SYS_ERR, "Save Flash Fail when setting tx retry count");
			ret = NVRAM_ERR;
		} else {
			unsigned char unsgined_retry_count = retry_count;
			JSON_PRINT(SYS_DBG, "tx retry_count=%d\n", unsgined_retry_count);
			WiFi_QueryAndSet(SET_TX_RETRY_COUNT, &unsgined_retry_count, NULL);
		}
	}
	
	if (!snx_json_obj_get_int(parse_json, "udp_type", &udp_type))
	{
		if (snx_nvram_integer_set("WIFI_DEV", "UDP_TYPE", udp_type) != NVRAM_SUCCESS) 
		{
			JSON_PRINT(SYS_ERR, "Save Flash Fail when setting udp type");
			ret = 1;
		}else {		
			//unsigned char unsgined_retry_count = retry_count;
			//JSON_PRINT(SYS_DEBG, "tx retry_count=%d\n", unsgined_retry_count);
			//WiFi_QueryAndSet(SET_TX_RETRY_COUNT, &unsgined_retry_count, NULL); 
		}
		
	}	
#endif

response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "setwifiparameters_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status", DAEMON_ERRNO(CMD_SETWIFIPARAMETERS, ret)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}

static buffer_t *js_set_wdr(snx_json_t *parse_json, void *param)
{
	int wdr_en = -1;
	int ret = 0;
	snx_json_t *resp_json = NULL;
	char *output = NULL;

	if (snx_json_obj_get_int(parse_json, "status", &wdr_en)) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	}

	ret = app_uti_set_video_wdr(wdr_en);

response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "setwdr_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status",  DAEMON_ERRNO(CMD_SETWDR, ret)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}
	
	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}

static buffer_t *js_set_mirror(snx_json_t *parse_json, void *param)
{
	int mirror = -1;
	int ret = 0;
	snx_json_t *resp_json = NULL;
	char *output = NULL;

	if (snx_json_obj_get_int(parse_json, "status", &mirror)) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	}

	ret = app_uti_set_video_mirror(mirror);

response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "setmirror_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status",	DAEMON_ERRNO(CMD_SETMIRROR, ret)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}
	
	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}

static buffer_t *js_set_flip(snx_json_t *parse_json, void *param)
{
	int flip = -1;
	int ret = 0;
	snx_json_t *resp_json = NULL;
	char *output = NULL;

	if (snx_json_obj_get_int(parse_json, "status", &flip)) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	}

	ret = app_uti_set_video_flip(flip);

response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "setflip_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status",	DAEMON_ERRNO(CMD_SETFLIP, ret)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}
	
	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output,  strlen((char *)output));


release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}


static buffer_t *js_get_videostatus(snx_json_t *parse_json, void *param)
{
	snx_json_t *resp_json = NULL;
	char *output = NULL;
	int wdr_status = -1, mirror_status = -1, flip_status = -1, resolution = -1;
	unsigned int uibps = 0, width = 0, height = 0, ucfps = 0, ucgop = 0;
	int capability = 0;
	int usbdclassmode = 0;
	int ret = 0;
#ifdef ENGINNER_MODE
#if CONFIG_APP_DRONE
	int resume = -1, suspend = -1;
#endif
	int pre_ext_pframe_num = -1, pre_ext_qp_range = -1, pre_ext_qp_max = -1, pre_ext_upper_pframe = -1, pre_ext_upper_pframe_dup1 = -1, pre_qp_max = -1, pre_qp_min = -1;
#endif

	snx_isp_drc_status_get(&wdr_status);
	snx_isp_sensor_mirror_get(&mirror_status);
	snx_isp_sensor_flip_get(&flip_status);
	usbdclassmode = usbd_mid_get_class_mode();

	capability = ((mf_video_resmap_get_record_capability() &
	               REC_CAPABILITY_MASK) |
	              (mf_video_resmap_get_preview_capability() &
	               VIEW_CAPABILITY_MASK));

	mf_video_resmap_get_preview_params(&width, &height,
	                                   &ucfps, &ucgop, &uibps);

	if ((width == HD_WIDTH) && (height == HD_HEIGHT))
		resolution = 1;
	else if ((width == VGA_WIDTH) && (height == VGA_HEIGHT))
		resolution = 0;
	else {
		JSON_PRINT(SYS_ERR, "resolution size no OK !");
		goto end;
	}

#ifdef ENGINNER_MODE
	if( (ret = app_uti_get_video_ext_qp(&pre_ext_pframe_num, &pre_ext_qp_range, &pre_ext_qp_max, &pre_ext_upper_pframe, &pre_ext_upper_pframe_dup1, &pre_qp_max, &pre_qp_min) ) != 0 ) {
		JSON_PRINT(SYS_ERR, "get ext qp failed !\n");
		goto end;
	}

#if CONFIG_APP_DRONE
	if( (ret = app_uti_get_video_preview_time(&resume, &suspend) ) != 0 ) {
		JSON_PRINT(SYS_ERR, "get video preview time error !\n");
		goto end;
	}
#endif
#endif

	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "getvideostatus_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "wdr", wdr_status) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "mirror", mirror_status) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "flip", flip_status) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}
	
	if (snx_json_obj_add_int(resp_json, "fps", ucfps) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}
	
	if (snx_json_obj_add_int(resp_json, "bitrate", uibps) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}
	
	if (snx_json_obj_add_int(resp_json, "resolution", resolution) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "capability", capability) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "usbdclassmode", usbdclassmode) != 0) { //0:mass mode 1:uvc mode
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "gop", ucgop) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

#ifdef ENGINNER_MODE
	if (snx_json_obj_add_int(resp_json, "pre_ext_pframe_num", pre_ext_pframe_num) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj for pre_ext_pframe_num\n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "pre_ext_qp_range", pre_ext_qp_range) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj for pre_ext_qp_range \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "pre_ext_qp_max", pre_ext_qp_max) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj for pre_ext_qp_max \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "pre_ext_upper_pframe", pre_ext_upper_pframe) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj for pre_ext_upper_pframe\n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "pre_ext_upper_pframe_dup1", pre_ext_upper_pframe_dup1) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj for pre_ext_upper_pframe_dup1 \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "pre_qp_max", pre_qp_max) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj for pre_qp_max\n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "pre_qp_min", pre_qp_min) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj for pre_qp_min\n");
		goto release_json;
	}

#if CONFIG_APP_DRONE
	if (snx_json_obj_add_int(resp_json, "resume", resume) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj for resume\n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "suspend", suspend) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj for suspend\n");
		goto release_json;
	}
#endif // CONFIG_APP_DRONE
#endif // ENGINNER_MODE

	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}

#if 0
int CheckIsUseOnlyISP(int resolution, int fps)
{
	int recordfps, recordresolution, recordwidth, recordheight;
	if (snx_nvram_integer_get(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_RECORD_VIDEO_FPS, &recordfps) != NVRAM_SUCCESS) {

		JSON_PRINT(SYS_ERR, "Get Preview FPS Video from NVRAM failed!");
		goto end;
	}
	if (snx_nvram_integer_get(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_RECORD_WIDTH, &recordwidth) != NVRAM_SUCCESS) {

		JSON_PRINT(SYS_ERR, "Get Preview Video WIDTH from NVRAM failed!");
		goto end;
	}
	if (snx_nvram_integer_get(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_RECORD_HEIGHT, &recordheight) != NVRAM_SUCCESS) {

		JSON_PRINT(SYS_ERR, "Get Preview Video HEIGHT from NVRAM failed!");
		goto end;
	}
	if ((recordwidth == FHD_WIDTH) && (recordheight = FHD_HEIGHT))
		recordresolution = 2; //FHD
	else if ((recordwidth == HD_WIDTH) && (recordheight = HD_HEIGHT))
		recordresolution = 1; //HD
	else if ((recordwidth == VGA_WIDTH) && (recordheight = VGA_HEIGHT))
		recordresolution = 0; //VGA
	else {
		JSON_PRINT(SYS_ERR, "resolution size no OK !");
		goto end;
	}
	if ((resolution == recordresolution) && (fps <= recordfps) && (recordfps <= 30))
		return 1;
	else
		return 0;

end:
	return 0;
}
#endif

static buffer_t *js_set_videoparameters(snx_json_t *parse_json, void *param)
{
	int bitrate = 0, fps = 0, resolution = 0, gop = 0, rc = 0;
	snx_json_t *resp_json = NULL;
	char *output = NULL;
	int width = 0, height = 0;
	int resolutionchange = 0;
#ifdef ENGINNER_MODE
#if CONFIG_APP_DRONE
	int resume = -1, suspend = -1;
#endif
	int preview_setting = 0;
	int pre_ext_pframe_num = -1, pre_ext_qp_range = -1, pre_ext_qp_max = -1, pre_ext_upper_pframe = -1, pre_ext_upper_pframe_dup1 = -1, pre_qp_max = -1, pre_qp_min = -1;
#endif

	if (snx_json_obj_get_int(parse_json, "bitrate", &bitrate)) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		rc = GET_OBJECT_FAIL;
		goto response_json;
	}

	if (snx_json_obj_get_int(parse_json, "fps", &fps)) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		rc = GET_OBJECT_FAIL;
		goto response_json;
	}

	if (snx_json_obj_get_int(parse_json, "resolution", &resolution)) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		rc = GET_OBJECT_FAIL;
		goto response_json;
	}

	if (snx_json_obj_get_int(parse_json, "gop", &gop)) {
		//JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		//goto end;
		gop = 0;
	}
	
    JSON_PRINT(SYS_INFO, "bitrate: %u; fps: %u; resolution: %u\n", bitrate, fps, resolution);

	if(
#if 0
        (chk_record_is_running() == 0)
        ||
#endif
#ifndef CONFIG_APP_DRONE
        (chk_protect_is_running() == 0)
        ||
        (chk_lapse_is_running() == 0) 
        ||
#endif
        (chk_isp0_is_running() == 0)
        ||
        (chk_isp1_is_running() == 0)
    ) {
#if 0
		if (chk_record_is_running() == 0)
			JSON_PRINT(SYS_ERR, "Record Task have not init success\n");
#endif
		if (chk_isp0_is_running() == 0)
			JSON_PRINT(SYS_ERR, "ISP0 Task have not init success\n");
		if (chk_isp1_is_running() == 0)
			JSON_PRINT(SYS_ERR, "ISP1 Task have not init success\n");
#ifndef CONFIG_APP_DRONE
		if (chk_protect_is_running() == 0)
			JSON_PRINT(SYS_ERR, "Protect Task have not init success\n");
		if (chk_lapse_is_running() == 0)
			JSON_PRINT(SYS_ERR, "Timelapse Task have not init success\n");
#endif
		rc = TASK_HAVE_NOT_INIT_OK;  //reboot task , wait
		goto response_json;
	}

#ifdef ENGINNER_MODE
	snx_json_obj_get_int(parse_json, "pre_ext_pframe_num", &pre_ext_pframe_num);
	snx_json_obj_get_int(parse_json, "pre_ext_qp_range", &pre_ext_qp_range);
	snx_json_obj_get_int(parse_json, "pre_ext_qp_max", &pre_ext_qp_max);
	snx_json_obj_get_int(parse_json, "pre_ext_qp_max", &pre_ext_qp_max);
	snx_json_obj_get_int(parse_json, "pre_ext_upper_pframe", &pre_ext_upper_pframe);
	snx_json_obj_get_int(parse_json, "pre_ext_upper_pframe_dup1", &pre_ext_upper_pframe_dup1);
	snx_json_obj_get_int(parse_json, "pre_qp_max", &pre_qp_max);
	snx_json_obj_get_int(parse_json, "pre_qp_min", &pre_qp_min);

	rc = app_uti_set_video_ext_qp(pre_ext_pframe_num, pre_ext_qp_range, pre_ext_qp_max, pre_ext_upper_pframe, pre_ext_upper_pframe_dup1, pre_qp_max, pre_qp_min);
	if(rc == OK)
		preview_setting = 1;
		
	

#if CONFIG_APP_DRONE
	snx_json_obj_get_int(parse_json, "resume", &resume);
	snx_json_obj_get_int(parse_json, "suspend", &suspend);

	rc = app_uti_set_video_preview_time(resume,  suspend);
	if(rc == OK)
		preview_setting = 1;
#endif
	
	if(preview_setting == 1){
		int sdstate;
		sdstate = app_uti_chk_sd_state();
		if(sdstate != SD_OK) {
			rc = sdstate;
			goto response_json;
		}
	}


#endif // ENGINNER_MODE 

	mf_video_resmap_get_preview_params(
	    (unsigned *) &width,
	    (unsigned *) &height,
	    NULL, NULL, NULL);

	if (resolution == 0) { //vga
		if ((width != VGA_WIDTH) || (height != VGA_HEIGHT)) {
			width = (unsigned) VGA_WIDTH;
			height = (unsigned) VGA_HEIGHT;
			resolutionchange = 1;
		}
	} else if (resolution == 1) { //hd
		if ((width != HD_WIDTH) || (height != HD_HEIGHT)) {
			width = (unsigned) HD_WIDTH;
			height = (unsigned) HD_HEIGHT;
			resolutionchange = 1;
		}
	} else if (resolution == 2) { //fhd
		if ((width != FHD_WIDTH) || (height != FHD_HEIGHT)) {
			width = (unsigned) FHD_WIDTH;
			height = (unsigned) FHD_HEIGHT;
			resolutionchange = 1;
		}
	}

	mf_video_resmap_update_preview_params(
	    (unsigned) width,
	    (unsigned) height,
	    (unsigned) fps,
	    (unsigned) gop,
	    (unsigned) bitrate, UPDATESEL_ALL);

	if ((chk_record_is_running() == 1)
#ifndef CONFIG_APP_DRONE
	        && (chk_protect_is_running() == 1)
	        && (chk_lapse_is_running() == 1)
#endif
	        && (chk_isp0_is_running() == 1)
	        && (chk_isp1_is_running() == 1)) {

#ifdef ENGINNER_MODE
		if ( ((chk_preview_use_isp0dup() == 1) && (resolutionchange == 1) ) || //resolution change
		        ((chk_preview_use_isp0dup() == 1) && (preview_setting == 1) )) {
						 if(((chk_preview_use_isp0dup() == 1) && (preview_setting == 1) )) {
#ifndef CONFIG_APP_DRONE
                                set_protect_closeflag();
#endif
                                set_record_closeflag();
#ifndef CONFIG_APP_DRONE
                                set_lapse_record_closeflag();
#endif
                                set_isp0_closeflag();
                                set_isp1_closeflag();       
                        }else {
                                set_isp0dup_closeflag();
                                set_isp1_closeflag();
                        }


		} else if ( ((chk_preview_use_isp0dup() == 0) && (resolutionchange == 1) ) || //resolution change
		            ((chk_preview_use_isp0dup() == 0) && (preview_setting == 1) ) ) {
						if(((chk_preview_use_isp0dup() == 0) && (preview_setting == 1) )) {
#ifndef CONFIG_APP_DRONE
                                set_protect_closeflag();
#endif
                                set_record_closeflag();
#ifndef CONFIG_APP_DRONE
                                set_lapse_record_closeflag();
#endif
                                set_isp0_closeflag();
                                set_isp1_closeflag();       
                        }else
                                set_isp1_closeflag();

		} else { //bps fps change
			set_preview_fps_bit(1);
			set_preview_bps_bit(1);
			//if(qp_change == 1)
			//set_preview_ext_qp_bit(1);
		}
#else
		if ((chk_preview_use_isp0dup() == 1) && (resolutionchange == 1)) {	//resolution change
			set_isp0dup_closeflag();
			set_isp1_closeflag();

		} else if ((chk_preview_use_isp0dup() == 0) && (resolutionchange == 1)) {	//resolution change
			set_isp1_closeflag();
		} else { //bps fps change
			set_preview_fps_bit(1);
			set_preview_bps_bit(1);

		}
#endif // ENGINNER_MODE
	} else {
		JSON_PRINT(SYS_ERR, "ALL Task have not init success\n");
		rc = TASK_HAVE_NOT_INIT_OK;  //reboot task , wait
	}

response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "setvideoparameters_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status",	DAEMON_ERRNO(CMD_SETVIDEOPARMETER, rc == 0 ? 0 : 1)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}

static buffer_t *js_set_videobitrate(snx_json_t *parse_json, void *param)
{
	int bitrate = -1;
	int rc = 0;
	snx_json_t *resp_json = NULL;
	char *output = NULL;

	if (snx_json_obj_get_int(parse_json, "bitrate", &bitrate)) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		rc = GET_OBJECT_FAIL;
		goto response_json;
	}

	rc = app_uti_set_preview_bitrate(bitrate);

response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "setvideobitrate_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status",	DAEMON_ERRNO(CMD_SETVIDEOBITRATE, rc == 0 ? 0 : 1)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}

static buffer_t *js_set_videofps(snx_json_t *parse_json, void *param)
{
	int fps = -1;
	int rc = 0;
	snx_json_t *resp_json = NULL;
	char *output = NULL;

	if (snx_json_obj_get_int(parse_json, "fps", &fps )) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		rc = GET_OBJECT_FAIL;
		goto response_json;
	}

	rc = app_uti_set_preview_fps(fps);

response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", " setvideofps_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status",	DAEMON_ERRNO(CMD_SETVIDEOFPS, rc == 0 ? 0 : 1)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}

static buffer_t *js_set_videoresolution(snx_json_t *parse_json, void *param)    //now no use
{
	int resolution = -1;
	int rc = 0;
	snx_json_t *resp_json = NULL;
	char *output = NULL;

	if (snx_json_obj_get_int(parse_json, "resolution", &resolution )) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		rc = GET_OBJECT_FAIL;
		goto response_json;
	}

	rc = app_uti_set_preview_resolution(resolution);
	vTaskDelay(1000 / portTICK_RATE_MS );

response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", " setvideoresolution_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status",	DAEMON_ERRNO(CMD_SETVIDEORESOLUTION, rc == 0 ? 0 : 1)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}

static buffer_t *js_takepicture(snx_json_t *parse_json, void *param)
{
	snx_json_t *resp_json = NULL;
	char *output = NULL;
	int ret = 0;
	int pic_num = 0;
	int queue_num = 0;
	int sdstate = 0;

	if (snx_json_obj_get_int(parse_json, "pic_num", &pic_num) != 0) {
            // if we aren't told how many pictures, then use 1
            pic_num = 1;
	}

    JSON_PRINT(SYS_INFO, "; pic_num: %i\n", pic_num);

	sdstate = app_uti_chk_sd_state();
	if(sdstate != SD_OK) {
		ret = sdstate;
		goto response_json;
	}

	if(mf_snapshot_get_isfull() == 1) {
		JSON_PRINT(SYS_ERR, "photo folder is full.\n");
		ret = PHOTO_FOLDER_IS_FULL;
		goto response_json;
	}

	if (check_takepic_task() != 0) {
		JSON_PRINT(SYS_ERR, "Continuous shooting is already running.\n");
		ret = CONTINUE_SHOOTING_IS_RUNNING;
		goto response_json;
	}

	if ((chk_record_is_running() == 0)
#ifndef CONFIG_APP_DRONE
	        || (chk_protect_is_running() == 0)
	        || (chk_lapse_is_running() == 0)
#endif
	        || (chk_isp0_is_running() == 0)
	        || (chk_isp1_is_running() == 0) ) {
		JSON_PRINT(SYS_ERR, "ALL Task have not init success\n");
		ret = TASK_HAVE_NOT_INIT_OK;
		goto response_json;
	}

	queue_num = mf_snapshot_get_queue_num();
	if ( (queue_num + pic_num) > SHAPSHOT_MAX_QUEUE_ITEM) {
		JSON_PRINT(SYS_ERR, "Queue number is full.\n");
		ret = OVER_SNAPSHOT_MAX_QUEUE_NUM;
		goto response_json;
	}
	ret =  app_uti_take_picture(pic_num);
	
response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "takepicture_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status", DAEMON_ERRNO(CMD_TAKEPICTURE, ret)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}
	
	output = snx_json_object_to_json_string(resp_json);
	JSON_PRINT(SYS_DBG, "response_json:\n%s\n", output);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}

static buffer_t *js_set_powerfrequency(snx_json_t *parse_json, void *param)
{
	int powerfrequency = -1;
	int ret = 0;
	snx_json_t *resp_json = NULL;
	char *output = NULL;

	if (snx_json_obj_get_int(parse_json, "frequency", &powerfrequency )) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	}

	ret = app_uti_set_power_frequency(powerfrequency);

response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "setpowerfrequency_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status",	DAEMON_ERRNO(CMD_SETPOWERFREQUENCY, ret)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}


static buffer_t *js_set_gsensorparamter(snx_json_t *parse_json, void *param)
{
	int gsensor_sensitivity = -1;
	int ret = 0;
	snx_json_t *resp_json = NULL;
	char *output = NULL;

	if (snx_json_obj_get_int(parse_json, "sensitivity", &gsensor_sensitivity)) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	}

	ret = app_uti_set_gsensor_sensitivity(gsensor_sensitivity);

response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "setgsensorparameter_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status",  DAEMON_ERRNO(CMD_SETGSENSORPARAMTER, ret)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}
	
	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output,  strlen((char *)output));
release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;

}

static buffer_t *js_get_sdspace(snx_json_t *parse_json, void *param)
{
	int ret = 0;
	snx_json_t *resp_json = NULL;
	char *output = NULL;
	int fre_percent = -1, tot_GByte = -1;//, count=0;
	int err_num = 0;
	FATFS *fs = NULL;
	unsigned long fre_clust = 0, tot_clust = 0;
	int sdstate = 0;

	sdstate = app_uti_chk_sd_state();
	if(sdstate != SD_OK) {
		err_num = sdstate;
		goto response_json;
	}

	/* Get volume information and free clusters of drive 1 */
	ret = f_getfree("0:", &fre_clust, &fs);
	if (ret == FR_OK) {
		/* Get total sectors and free sectors */
		tot_clust = (fs->n_fatent - 2);
		fre_percent = (int)(fre_clust * 100 + (tot_clust >> 1)) / tot_clust;
		tot_GByte = (tot_clust * fs->csize + (1 << 21) - 1) >> (30 - 9); //
		//while(tot_GByte)
		//{
		//	count++;
		//	tot_GByte >>= 1;
		//}
		//tot_GByte = 1<<count;
		JSON_PRINT(SYS_DBG, "total = %d, free = %d, csize = %d\n", tot_clust, fre_clust, fs->csize);
	} else {
		fre_percent = -1;
		tot_GByte = -1;
		JSON_PRINT(SYS_ERR, "get free fail\n");
	}

response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_int(resp_json, "errorcode", err_num) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_str(resp_json, "type", "getsdspace_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "sdspace", fre_percent) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "totalspace", tot_GByte) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	output = snx_json_object_to_json_string(resp_json);
	JSON_PRINT(SYS_DBG, "response_json:\n%s\n", output);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}


static buffer_t *js_get_format_status(snx_json_t *parse_json, void *param)
{
	int ret = 0;
	snx_json_t *resp_json = NULL;
	char *output = NULL;
	FATFS *fs = NULL;
	int fre_percent = -1, tot_GByte = -1;//, count=0;
	unsigned long fre_clust = 0, tot_clust = 0;

	ret = app_uti_get_sd_format_status();

	if (ret == OK) {
		/* Get volume information and free clusters of drive 1 */
		ret = f_getfree("0:", &fre_clust, &fs);
		if (ret == FR_OK) {
			/* Get total sectors and free sectors */
			tot_clust = (fs->n_fatent - 2);
			fre_percent = (int)(fre_clust * 100 + (tot_clust >> 1)) / tot_clust;
			tot_GByte = (tot_clust * fs->csize + (1 << 21) - 1) >> (30 - 9); //
			JSON_PRINT(SYS_DBG, "total = %d, free = %d, csize = %d\n", tot_clust, fre_clust, fs->csize);
		}
	}
	
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "getsdformat_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "sdspace", fre_percent) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "totalspace", tot_GByte) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status", DAEMON_ERRNO(CMD_GETSDFORMAT, ret)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}

static buffer_t *js_set_sdformat(snx_json_t *parse_json, void *param)
{
	int fmt_ret = 0;
	snx_json_t *resp_json = NULL;
	char *output = NULL;

	int sdstate;
	sdstate = app_uti_chk_sd_state();
	if(sdstate == SD_NOT_EXIST) {
		JSON_PRINT(SYS_ERR, "SD Card doesn't exist\n");
		fmt_ret = sdstate;
		goto response_json;
	} else if(sdstate == SD_IS_FORMATTING) {
		JSON_PRINT(SYS_ERR, "SD Card format is running\n");
		fmt_ret = sdstate;
		goto response_json;
	}

	JSON_PRINT(SYS_DBG, "SD Card sdstate=%d\n", sdstate);
	//set format start
	fmt_ret = app_uti_create_sd_format_task(NULL);

response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "setsdformat_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status", DAEMON_ERRNO(CMD_SETSDFORMAT, fmt_ret)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	output = snx_json_object_to_json_string(resp_json);
	JSON_PRINT(SYS_DBG, "response_json:\n%s\n", output);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}

static buffer_t *js_get_sdtest_status(snx_json_t *parse_json, void *param)
{
	int ret = 0;
	snx_json_t *resp_json = NULL;
	char *output = NULL;
	char avg_write[16] = {0}, avg_read[16] = {0};

	ret = app_uti_get_sd_test_status();
	if (ret == OK) {
		app_uti_get_test_report(avg_write, avg_read);
	} else {
		snprintf(avg_write, 16, "%s", "NG");
		snprintf(avg_read, 16, "%s", "NG");
	}
	
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "getsdtest_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_str(resp_json, "avg_write", avg_write) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_str(resp_json, "avg_read", avg_read) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status", DAEMON_ERRNO(CMD_GETSDTEST, ret)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}

static buffer_t *js_set_sdtest(snx_json_t *parse_json, void *param)
{
	int ret = 0;
	snx_json_t *resp_json = NULL;
	char *output = NULL;
	int sdstate;
	
	sdstate = app_uti_chk_sd_state();
	if(sdstate == SD_NOT_EXIST) {
		JSON_PRINT(SYS_ERR, "SD Card doesn't exist\n");
		ret = sdstate;
		goto response_json;
	} else if(sdstate == SD_IS_FORMATTING) {
		JSON_PRINT(SYS_ERR, "SD Card format is running\n");
		ret = sdstate;
		goto response_json;
	}
	else if (sdstate == SD_IS_TESTING) {
		JSON_PRINT(SYS_ERR, "SD Card test is running\n");
		ret = sdstate;
		goto response_json;
	}

	JSON_PRINT(SYS_DBG, "SD Card sdstate=%d\n", sdstate);

	ret = app_uti_create_sd_test_task(NULL);

response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "setsdtest_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status", DAEMON_ERRNO(CMD_SETSDTEST, ret)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	output = snx_json_object_to_json_string(resp_json);
	JSON_PRINT(SYS_DBG, "response_json:\n%s\n", output);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}

static buffer_t *js_set_recordstatus(snx_json_t *parse_json, void *param)
{
	snx_json_t *resp_json = NULL;
	char *output = NULL;
	int rec_status = -1;
	int ret = 0;

	if (snx_json_obj_get_int(parse_json, "status", &rec_status)) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	}

	int sdstate = 0;
	sdstate = app_uti_chk_sd_state();
	if( sdstate != SD_OK) {
		ret = sdstate;
		goto response_json;
	}

	if (schedrec_get_isfull() == 1) {
		JSON_PRINT(SYS_ERR, "record folder is full.\n");
		ret = RECORD_FOLDER_IS_FULL;
		goto response_json;
	}
	
#ifndef CONFIG_APP_DRONE
	if (reclapse_get_isfull() == 1) {
		JSON_PRINT(SYS_ERR, "timelapse record folder is full.\n");
		ret = TIMELAPSE_FOLDER_IS_FULL;
		goto response_json;
	}
#endif

	if(rec_status == 1) {
		//check playback is working or not
		if(conn_list_len(&playback_list) == 0) {
			ret = app_uti_enable_record();
		} else {
			JSON_PRINT(SYS_DBG, "playback is still working \n");
			ret = PLAYBACK_IS_ON_WORKING;
		}
	} else if(rec_status == 0) {
		ret = app_uti_disable_record();
	} else {
		JSON_PRINT(SYS_ERR, "Invalid Parameters\n");
		ret = INVALID_PARAM;
	}

response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "setrecordstatus_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status", DAEMON_ERRNO(CMD_SETRECORDSTATUS, ret)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	output = snx_json_object_to_json_string(resp_json);
	JSON_PRINT(SYS_DBG, "response_json:\n%s\n", output);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}

static buffer_t *js_get_recordstatus(snx_json_t *parse_json, void *param)
{
	snx_json_t *resp_json = NULL;
	char *output = NULL;
	int rec_status = 0, rec_running = 0;
	int level = -1, recordlength = -1, protectlength = -1, cycle = -1, capability = -1;
	int recordresolution = 0;
	unsigned int recordbps = 0, recordgop = 0, recordfps = 0;
	int ret = 0;

#ifdef ENGINNER_MODE
	int rec_ext_pframe_num = -1, rec_ext_qp_range = -1, rec_ext_qp_max = -1, rec_ext_upper_pframe = -1, rec_qp_max = -1, rec_qp_min = -1;
#endif
	ret = app_uti_get_record_status(&rec_status, &rec_running, &recordfps, &recordgop, &recordbps, &recordresolution, &capability, &level, &recordlength, &cycle, &protectlength);
	if (ret != OK) {
		JSON_PRINT(SYS_ERR, "ret = %d\n", ret);
		goto response_json;
	}

#ifdef ENGINNER_MODE

	if( (ret = app_uti_get_record_ext_qp(&rec_ext_pframe_num, &rec_ext_qp_range, &rec_ext_qp_max, &rec_ext_upper_pframe, &rec_qp_max, &rec_qp_min) ) != 0 ) {
		JSON_PRINT(SYS_ERR, "get ext qp failed !\n");
		goto response_json;
	}
#endif // ENGINNER_MODE

response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "getrecordstatus_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status",	DAEMON_ERRNO(CMD_GETRECORDSTATUS, ret)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "recstatus", (rec_status == 1) ? 1 : 0) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "recrunning", (rec_running == 1) ? 1 : 0) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "volume", level) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}
	
	if (snx_json_obj_add_int(resp_json, "length", recordlength) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}
	
	if (snx_json_obj_add_int(resp_json, "fps", recordfps) != 0) { //not implement
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "bitrate", recordbps) != 0) { //not implement
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}
	
	if (snx_json_obj_add_int(resp_json, "resolution", recordresolution) != 0) { //not implement
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}
	
	if (snx_json_obj_add_int(resp_json, "loop", cycle) != 0) { //not implement
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "capability", capability) != 0) { //not implement
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}
	
	if (snx_json_obj_add_int(resp_json, "protectlength", protectlength) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}
	
	if (snx_json_obj_add_int(resp_json, "gop", recordgop) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

#ifdef ENGINNER_MODE
	if (snx_json_obj_add_int(resp_json, "rec_ext_pframe_num", rec_ext_pframe_num) != 0) { //not implement
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj for rec_ext_pframe_num\n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "rec_ext_qp_range", rec_ext_qp_range) != 0) { //not implement
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj for rec_ext_qp_range\n");
		goto release_json;
	}
	
	if (snx_json_obj_add_int(resp_json, "rec_ext_qp_max", rec_ext_qp_max) != 0) { //not implement
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj for rec_ext_qp_max\n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "rec_ext_upper_pframe", rec_ext_upper_pframe) != 0) { //not implement
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj for rec_ext_upper_pframe\n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "rec_qp_max", rec_qp_max) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj for rec_qp_max\n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "rec_qp_min", rec_qp_min) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj for rec_qp_min\n");
		goto release_json;
	}
#endif

	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output, strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}


static buffer_t *js_set_recordaudiostatus(snx_json_t *parse_json, void *param)
{
	int level = -1;
	int rc = 0;
	snx_json_t *resp_json = NULL;
	char *output = NULL;
	int sdstate = 0;

	if (snx_json_obj_get_int(parse_json, "level", &level)) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		rc = GET_OBJECT_FAIL;
		goto response_json;
	}
	JSON_PRINT(SYS_DBG, "voice level=%d \n", level);

	sdstate = app_uti_chk_sd_state();
	if(sdstate != SD_OK) {
		rc = sdstate;
		goto response_json;
	}
	
#ifndef CONFIG_APP_DRONE
	if (snx_nvram_integer_set(NVRAM_PKG_AUDIO_ISP, NVRAM_CFG_AUDIO_VOICE, level) != NVRAM_SUCCESS) {
		JSON_PRINT(SYS_ERR, "Set AUDIO VOICE to NVRAM failed!");
		rc = NVRAM_ERR;
	} else {
		set_record_audio_voice_bit(1);
	}
#endif // CONFIG_APP_DRONE

response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "setrecordaudiostatus_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status",	DAEMON_ERRNO(CMD_SETRECORDAUDIOSTATUS, rc)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	output = snx_json_object_to_json_string(resp_json);
	JSON_PRINT(SYS_DBG, "response_json:\n%s\n", output);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}

#define MAX_RECORD_LEN_SIG	(9999)
static buffer_t *js_set_recordlength(snx_json_t *parse_json, void *param)
{
	int minutes = -1;
	int rc = 0;
	snx_json_t *resp_json = NULL;
	char *output = NULL;
	int sec = 0;
	int sdstate = 0;

	if (snx_json_obj_get_int(parse_json, "minutes", &minutes)) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		rc = GET_OBJECT_FAIL;
		goto response_json;
	}

	sdstate = app_uti_chk_sd_state();
	if(sdstate != SD_OK) {
		rc = sdstate;
		goto response_json;
	}

	// when users setting record length is max (9999), caculate max sec.
#ifdef CONFIG_APP_DRONE
	if (minutes == MAX_RECORD_LEN_SIG)
		minutes = 10;		// setting max record length to 10 minutes for mem issue
#else
	if (minutes == MAX_RECORD_LEN_SIG)
		minutes = 30;		// setting max record length to 30 minutes
#endif

	sec = minutes * 60;

	if (snx_nvram_integer_set(NVRAM_RECORD, NVRAM_RECORD_SCHED_INTERVAL, sec) != NVRAM_SUCCESS) {
		JSON_PRINT(SYS_ERR, "Set Record Length data to NVRAM failed!");
		rc = NVRAM_ERR;
		goto response_json;
	}

	if ((chk_record_is_running() == 1)
#ifndef CONFIG_APP_DRONE
	        && (chk_protect_is_running() == 1)
	        && (chk_lapse_is_running() == 1)
#endif
	        && (chk_isp0_is_running() == 1)
	        && (chk_isp1_is_running() == 1)) {
		set_protect_closeflag();
		set_record_closeflag();
#ifndef CONFIG_APP_DRONE
		set_lapse_record_closeflag();
#endif
	} else {
		JSON_PRINT(SYS_ERR, "ALL Task have not init success\n");
		rc = TASK_HAVE_NOT_INIT_OK;  //reboot task , wait
	}

response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "setrecordlength_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status",	DAEMON_ERRNO(CMD_SETRECORDLENGTH, rc)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}

static buffer_t *js_set_protectlength(snx_json_t *parse_json, void *param)
{
	int second = -1;
	int rc = 0;
	snx_json_t *resp_json = NULL;
	char *output = NULL;
	int sdstate = 0;

	if (snx_json_obj_get_int(parse_json, "second", &second)) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		rc = GET_OBJECT_FAIL;
		goto response_json;
	}

	sdstate = app_uti_chk_sd_state();
	if(sdstate != SD_OK) {
		rc = sdstate;
		goto response_json;
	}

	if (snx_nvram_integer_set(NVRAM_RECORD, NVRAM_PROTECTRECORD_LENGTH, second) != NVRAM_SUCCESS) {
		JSON_PRINT(SYS_ERR, "Set Protect Length data to NVRAM failed!");
		rc = NVRAM_ERR;
		goto response_json;
	}

response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "setprotectlength_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status",	DAEMON_ERRNO(CMD_SETPROTECTLENGTH, rc)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	output = snx_json_object_to_json_string(resp_json);
	JSON_PRINT(SYS_DBG, "response_json:\n%s\n", output);
	buffer_add(g_resp_buf, output,  strlen((char *)output));
release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;

}

static buffer_t *js_set_looprecordstatus(snx_json_t *parse_json, void *param)
{
	int status = -1;
	int rc = 0;
	snx_json_t *resp_json = NULL;
	char *output = NULL;
	int sdstate = 0;

	if (snx_json_obj_get_int(parse_json, "status", &status)) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		rc = GET_OBJECT_FAIL;
		goto response_json;
	}

	sdstate = app_uti_chk_sd_state();
	if(sdstate != SD_OK) {
		rc = sdstate;
		goto response_json;
	}

	if (snx_nvram_integer_set(NVRAM_RECORD, NVRAM_RECORD_SCHED_CYCLE, status) != NVRAM_SUCCESS) {
		JSON_PRINT(SYS_ERR, "Set Record Length data to NVRAM failed!");
		rc = NVRAM_ERR;
		goto response_json;
	}
	
	set_record_cycle_bit(1);

response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "setlooprecordstatus_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status",	DAEMON_ERRNO(CMD_SETLOOPRECORDSTATUS, rc)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	output = snx_json_object_to_json_string(resp_json);
	JSON_PRINT(SYS_DBG, "response_json:\n%s\n", output);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}

static buffer_t *js_get_recordcapability(snx_json_t *parse_json, void *param)
{
	snx_json_t *resp_json;
	char *output = NULL;
	int capability = 0;

	capability = ((mf_video_resmap_get_record_capability() & REC_CAPABILITY_MASK) |
	              (mf_video_resmap_get_preview_capability() & VIEW_CAPABILITY_MASK) );

	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}
	
	if (snx_json_obj_add_str(resp_json, "type", "getrecordcapability_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}
	
	if (snx_json_obj_add_int(resp_json, "capability", capability) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}
	
	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output,  strlen((char *)output));
release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}

static buffer_t *js_set_recordparameters(snx_json_t *parse_json, void *param)
{
	int rc = 0;
	snx_json_t *resp_json = NULL;
	char *output = NULL;
	int fps = -1, resolution = -1, gop = -1, bitrate = -1;
	unsigned int unsignedbitrate = 0;
	int width = 0, height = 0;
	int resolutionchange = 0;
	int sdstate = 0;
#ifdef ENGINNER_MODE
	int rec_ext_pframe_num = -1, rec_ext_qp_range = -1, rec_ext_qp_max = -1, rec_ext_upper_pframe = -1, rec_qp_max = -1, rec_qp_min = -1;
#endif

	if (snx_json_obj_get_int(parse_json, "bitrate", &bitrate)) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		rc = GET_OBJECT_FAIL;
		goto response_json;
	}
	
	unsignedbitrate = bitrate;
	if (snx_json_obj_get_int(parse_json, "fps", &fps)) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		rc = GET_OBJECT_FAIL;
		goto response_json;
	}

	if (snx_json_obj_get_int(parse_json, "resolution", &resolution)) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		rc = GET_OBJECT_FAIL;
		goto response_json;
	}

	if (snx_json_obj_get_int(parse_json, "gop", &gop)) {
		gop = 0;
	}


	sdstate = app_uti_chk_sd_state();
	if(sdstate != SD_OK) {
		rc = sdstate;
		goto response_json;
	}

	//check preview is working or not
	if(conn_list_len(&preview_list) != 0) {
		JSON_PRINT(SYS_WARN, "someone is watching preview, so device can't change record param. \n");
		rc = PREVIEW_IS_ON_WORKING;
		goto response_json;
	}

	if ((chk_record_is_running() == 0)
#ifndef CONFIG_APP_DRONE
	        || (chk_protect_is_running() == 0)
	        || (chk_lapse_is_running() == 0)
#endif
	        || (chk_isp0_is_running() == 0)
	        || (chk_isp1_is_running() == 0)) {
		if (chk_record_is_running() == 0)
			JSON_PRINT(SYS_ERR, "Record Task have not init success\n");
		if (chk_isp0_is_running() == 0)
			JSON_PRINT(SYS_ERR, "ISP0 Task have not init success\n");
		if (chk_isp1_is_running() == 0)
			JSON_PRINT(SYS_ERR, "ISP1 Task have not init success\n");
#ifndef CONFIG_APP_DRONE
		if (chk_protect_is_running() == 0)
			JSON_PRINT(SYS_ERR, "Protect Task have not init success\n");
		if (chk_lapse_is_running() == 0)
			JSON_PRINT(SYS_ERR, "Timelapse Task have not init success\n");
#endif
		rc = TASK_HAVE_NOT_INIT_OK;  //reboot task , wait
		goto response_json;
	}

#ifdef ENGINNER_MODE
	snx_json_obj_get_int(parse_json, "rec_ext_pframe_num", &rec_ext_pframe_num);
	snx_json_obj_get_int(parse_json, "rec_ext_qp_range", &rec_ext_qp_range);
	snx_json_obj_get_int(parse_json, "rec_ext_qp_max", &rec_ext_qp_max);
	snx_json_obj_get_int(parse_json, "rec_ext_upper_pframe", &rec_ext_upper_pframe);
	snx_json_obj_get_int(parse_json, "rec_qp_max", &rec_qp_max);
	snx_json_obj_get_int(parse_json, "rec_qp_min", &rec_qp_min);
	rc = app_uti_set_record_ext_qp(rec_ext_pframe_num, rec_ext_qp_range, rec_ext_qp_max, rec_ext_upper_pframe, rec_qp_max, rec_qp_min);
#endif // ENGINNER_MODE

	mf_video_resmap_get_record_params(
	    (unsigned *) &width,
	    (unsigned *) &height,
	    NULL, NULL, NULL);

	JSON_PRINT(SYS_DBG, "width=%d, height=%d \n", width, height);
	if (resolution == 0) { //vga
		if ((width != VGA_WIDTH) || (height != VGA_HEIGHT)) {
			JSON_PRINT(SYS_DBG, "resolution == 0");
			width = (unsigned) VGA_WIDTH;
			height = (unsigned) VGA_HEIGHT;
			resolutionchange = 1;
		}
	} else if (resolution == 1) { //hd
		if ((width != HD_WIDTH) || (height != HD_HEIGHT)) {
			JSON_PRINT(SYS_DBG, "resolution == 1");
			width = (unsigned) HD_WIDTH;
			height = (unsigned) HD_HEIGHT;
			resolutionchange = 1;
		}
	} else if (resolution == 2) { //fhd
		if ((width != FHD_WIDTH) || (height != FHD_HEIGHT)) {
			JSON_PRINT(SYS_DBG, "resolution == 2");
			width = (unsigned) FHD_WIDTH;
			height = (unsigned) FHD_HEIGHT;
			resolutionchange = 1;
		}
	}

	if (mf_video_resmap_update_record_params(
	            (unsigned) width,
	            (unsigned) height,
	            (unsigned) fps,
	            (unsigned) gop,
	            unsignedbitrate, UPDATESEL_ALL) < 0) {
		// TODO: Need 2 add new return code when fail case in setting
		// record params.
		resolutionchange = 0;
		rc = SET_RECORD_PARAM_ERR;
		JSON_PRINT(SYS_ERR, "Record params lower than preview\n");
		goto response_json;
	}

	if ((chk_record_is_running() == 1)
#ifndef CONFIG_APP_DRONE
	        && (chk_protect_is_running() == 1)
	        && (chk_lapse_is_running() == 1)
#endif
	        && (chk_isp0_is_running() == 1)
	        && (chk_isp1_is_running() == 1)) {
#ifndef CONFIG_APP_DRONE
		set_protect_closeflag();
#endif
		set_record_closeflag();
#ifndef CONFIG_APP_DRONE
		set_lapse_record_closeflag();
#endif
		set_isp0_closeflag();
		set_isp1_closeflag();
	} else {

	}

response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "setrecordparameters_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status",	DAEMON_ERRNO(CMD_SETRECORDPARAMETERS, rc)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	output = snx_json_object_to_json_string(resp_json);
	JSON_PRINT(SYS_DBG, "response_json:\n%s\n", output);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}

static buffer_t *js_synctime(snx_json_t *parse_json, void *param)
{
	snx_json_t *resp_json = NULL;
	char *output = NULL, *timezone = NULL;
	int year = 0, month = 0, day = 0, hour = 0, minute = 0, second = 0;
	int ret = 0;
	system_date_t  time = {0};

	if (snx_json_obj_get_int(parse_json, "year", &year)) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	}

	if (snx_json_obj_get_int(parse_json, "month", &month)) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	}

	if (snx_json_obj_get_int(parse_json, "day", &day)) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	}

	if (snx_json_obj_get_int(parse_json, "hour", &hour)) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	}

	if (snx_json_obj_get_int(parse_json, "min", &minute)) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	}

	if (snx_json_obj_get_int(parse_json, "sec", &second)) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	}

	if (snx_json_obj_get_str(parse_json, "timezone", &timezone) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON string obj \n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	}

	time.year = year;
	time.month = month;
	time.day = day;
	time.hour = hour;
	time.minute = minute;
	time.second = second;
	memset(g_str_timezone, 0x0, sizeof(g_str_timezone));
	memcpy(g_str_timezone, timezone, ( (strlen(timezone) < 10) ? strlen(timezone) : 10 ) );
	ret = app_uti_sync_time(&time);
	
response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "synctime_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status",  DAEMON_ERRNO(CMD_SYNCTIME, 0) ) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}
	
	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output,  strlen((char *)output));
release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}

static buffer_t *js_get_time(snx_json_t *parse_json, void *param)
{
	snx_json_t *resp_json = NULL;
	char *output = NULL;
	system_date_t time = {0};
	get_date(&time);

	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "gettime_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "year", time.year) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "month", time.month) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "day", time.day) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "hour", time.hour) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "min", time.minute) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "sec", time.second) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_str(resp_json, "timezone", g_str_timezone) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;

}



static buffer_t *js_get_iqversion(snx_json_t *parse_json, void *param)
{
	snx_json_t *resp_json = NULL;
	char *output = NULL;
	int ret = 0;
	int version = 0;

	ret = app_uti_get_iq_version(&version);

	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}
	
	if (snx_json_obj_add_str(resp_json, "type", "getiqversion_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}
	
	if (snx_json_obj_add_int(resp_json, "resp_status", DAEMON_ERRNO(CMD_GETIQVERSION, (ret == 0) ? 0 : 1)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}
	
	if (snx_json_obj_add_int(resp_json, "iqversion", version) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}
	
	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}

static buffer_t *js_get_batterystatus(snx_json_t *parse_json, void *param)
{
	int battery_level = 0;
	snx_json_t *resp_json = NULL;
	char *output = NULL;

	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "getbatterystatus_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "batterylevel", battery_level) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}

static buffer_t *js_get_deviceparameter(snx_json_t *parse_json, void *param)
{
	int battery_level = 0;
	snx_json_t *resp_json = NULL;
	char *output = NULL;
	system_date_t time = {0};
	unsigned char channel = 0;
	unsigned char ssid[64] = {'\0'};
	unsigned char pwd[64] = {'\0'};
	char dev_ver[128] = {'\0'};
	int powerfrequency = 0;
	int gsensor_sensitivity = 0;
	int ret = 0;
	int version = 0;
	int wifi_mode = 0;

#ifdef ENGINNER_MODE
	int tx_retry = 0;
	int udp_type = 0;
#endif

	ret = app_uti_get_device_params(&version, &gsensor_sensitivity, &channel, ssid, pwd, &powerfrequency, dev_ver, &wifi_mode);
	if (ret != OK) {
		JSON_PRINT(SYS_ERR, "ret = %d\n", ret);
		goto response_json;
	}

#ifdef ENGINNER_MODE
	app_uti_get_device_engmode_params(&tx_retry, &udp_type);
	if (ret != OK) {
		JSON_PRINT(SYS_ERR, "ret = %d\n", ret);
		goto response_json;
	}
#endif

	// Get System date
	get_date(&time);

response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}
	
	if (snx_json_obj_add_str(resp_json, "type", "getdeviceparameter_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status",	DAEMON_ERRNO(CMD_GETDEVICEPARAMETER, ret)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}
	
	if (snx_json_obj_add_int(resp_json, "year", (int)time.year) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}
	
	if (snx_json_obj_add_int(resp_json, "month", (int)time.month) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}
	
	if (snx_json_obj_add_int(resp_json, "day", (int)time.day) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}
	
	if (snx_json_obj_add_int(resp_json, "hour", (int)time.hour) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}
	
	if (snx_json_obj_add_int(resp_json, "min", (int)time.minute) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}
	
	if (snx_json_obj_add_int(resp_json, "sec", (int)time.second) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}
	
	if (snx_json_obj_add_str(resp_json, "timezone", g_str_timezone) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}
	
	if (snx_json_obj_add_int(resp_json, "batterylevel", battery_level) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}
	
	if (snx_json_obj_add_int(resp_json, "wifichannel", (int)channel) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}
	
	if (snx_json_obj_add_str(resp_json, "ssid", (const char *)ssid) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}
	
	if (snx_json_obj_add_str(resp_json, "pwd", (const char *)pwd) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "wifimode", wifi_mode) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}
#ifdef ENGINNER_MODE
	if (snx_json_obj_add_int(resp_json, "tx_retry", tx_retry) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}
	
	if (snx_json_obj_add_int(resp_json, "udp_type", udp_type) != 0)
	{
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}
#endif

	if (snx_json_obj_add_int(resp_json, "powerfrequency", powerfrequency) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}
	
	if (snx_json_obj_add_int(resp_json, "gsensor_sensitivity", gsensor_sensitivity) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}
	
	if (snx_json_obj_add_str(resp_json, "fwversion", (const char *)dev_ver) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}
	
	if (snx_json_obj_add_int(resp_json, "iqversion", version) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}
	
	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}

static buffer_t *js_get_videolist(snx_json_t *parse_json, void *param)
{
	int listnum, i, ret = 0;
	char **file_list;
	uint32_t list_len = 0;
	char message[128] = {0};
	const char *message1 = "error:sd card doesn't exist\n";
	const char *message2 = "error:sd card is reading\n";
	char seedfirststring[20] = {0};
	char seedsecondstring[20] = {0};

	ret = read_card_state();
	
	if ((ret == 0) || (ret == -1)) {
		if (get_sd_umount_err() == 1)
			strncpy(message, message1, strlen(message1));
		else
			strncpy(message, message2, strlen(message2));

		list_len += strlen(message);
		list_len = htonl(list_len);
		buffer_add(g_resp_buf, &list_len, 4);
		buffer_add(g_resp_buf, message, strlen(message));
		goto end;
	}
	
	file_list = rec_filelist_CreateBuf(rec_recordtype_get(SCHED_RECORD), &listnum);
	JSON_PRINT(SYS_DBG, "listnum = %d, point = %d\n", listnum, file_list);
	seed_inttostring(seedfirststring, seedsecondstring);

	if (file_list) {
		for (i = 0; i < listnum; i++) {
			if (file_list[i] != NULL) {
				list_len += strlen(file_list[i]);
			}
		}
	}

	list_len += strlen(seedfirststring) + strlen(seedsecondstring);
	list_len = htonl(list_len);
	buffer_add(g_resp_buf, &list_len, 4);

	if (file_list) {
		for (i = 0; i < listnum; i++) {
			if (file_list[i] != NULL) {
				buffer_add(g_resp_buf, file_list[i], strlen(file_list[i]));
			}
		}
		rec_filelist_ReleaseBuf(rec_recordtype_get(SCHED_RECORD), file_list, listnum);
	}
	buffer_add(g_resp_buf, seedfirststring, strlen(seedfirststring));    //add rec_seed with two string for iOS
	buffer_add(g_resp_buf, seedsecondstring, strlen(seedsecondstring));
end:
	return g_resp_buf;
}

static buffer_t *js_get_timelapsevideolist(snx_json_t *parse_json, void *param)
{
	uint32_t list_len = 0;
#ifndef CONFIG_APP_DRONE
	int listnum, i;
	char **file_list;
	char message[128] = {0};
	const char *message1 = "error:sd card doesn't exist\n";
	const char *message2 = "error:sd card is reading\n";
#endif

	JSON_PRINT(SYS_ERR, "GetTimeLapseVideoList\n");
	
#ifndef CONFIG_APP_DRONE
	if (!reclapse_read_card_state()) {
		if (get_sd_umount_err() == 1)
			strncpy(message, message1, strlen(message1));
		else
			strncpy(message, message2, strlen(message2));

		list_len += strlen(message);
		list_len = htonl(list_len);
		buffer_add(g_resp_buf, &list_len, 4);
		buffer_add(g_resp_buf, message, strlen(message));
		goto end;
	}

	file_list = rec_filelist_CreateBuf(rec_recordtype_get(TIMELAPSE_RECORD), &listnum);

	JSON_PRINT(SYS_DBG, "listnum = %d, point = %d\n", listnum, file_list);

	if (file_list) {
		for (i = 0; i < listnum; i++) {
			if (file_list[i] != NULL) {
				list_len += strlen(file_list[i]);
			}
		}
	}
#endif
	list_len = htonl(list_len);
	buffer_add(g_resp_buf, &list_len, 4);

#ifndef CONFIG_APP_DRONE
	if (file_list) {
		for (i = 0; i < listnum; i++) {
			if (file_list[i] != NULL) {
				buffer_add(g_resp_buf, file_list[i], strlen(file_list[i]));
				JSON_PRINT(SYS_DBG, "str=%s, len = %d\n", file_list[i], strlen(file_list[i]));
			}
		}
		rec_filelist_ReleaseBuf(rec_recordtype_get(TIMELAPSE_RECORD), file_list, listnum);
	}
end:
#endif
	return g_resp_buf;
}

static buffer_t *js_get_indexfile(snx_json_t *parse_json, void *param)
{
	int list_src = 0, ret = 0, port = 0, is_full = 0, get_seed = 0;
	snx_json_t *resp_json = NULL;
	SocketItem_t *psocket = (SocketItem_t *)param;
	char *output = NULL;
	char list_path[LEN_FILENAME] = {0};
	int sdstate = 0;
	int list_state = 0;
	int upload_status = 0;

	if(snx_json_obj_get_int(parse_json, "list", &list_src) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	}

    JSON_PRINT(SYS_INFO, "; list_src: %i\n", list_src);

    if(PROTECT_LIST == list_src)
    {
        list_src = PICTURE_LIST;
    }

	sdstate = app_uti_chk_sd_state();
	if(sdstate != SD_OK) {
		ret = sdstate;
		goto response_json;
	}

#if defined(CONFIG_SYSTEM_PLATFORM_SN98660) || defined(CONFIG_SYSTEM_PLATFORM_SN98661) || defined(CONFIG_SYSTEM_PLATFORM_SN98671) || defined (CONFIG_SYSTEM_PLATFORM_SN98293)
	if ((list_src >= RECORD_LIST) && (list_src <= TIMELAPSE_LIST)) { //list_src range: 0~4
#ifndef CONFIG_APP_DRONE
		if (list_src == PROTECT_LIST) {
			list_state = protectlist_to_file(list_path);
			if ((list_state == LIST_FILE_OK) || (list_state == LIST_FILE_IS_NONE)) {
				JSON_PRINT(SYS_DBG, "list_path===%s\n", list_path);
			} else if (list_state == LIST_FILE_DOWNLOAD) {
				ret = FILELIST_IS_UPLOADING;
				goto response_json;
			} else {
				JSON_PRINT(SYS_DBG, "list_state===%d\n", list_state);
				ret = GET_INDEXFILE_ERR;
				goto response_json;
			}

		} else if (list_src == PICTURE_LIST) {
			list_state = piclist_to_file(list_path);
			if ((list_state == LIST_FILE_OK) || (list_state == LIST_FILE_IS_NONE)) {
				JSON_PRINT(SYS_DBG, "list_path===%s\n", list_path);
			} else if (list_state == LIST_FILE_DOWNLOAD) {
				ret = FILELIST_IS_UPLOADING;
				goto response_json;
			} else {
				JSON_PRINT(SYS_DBG, "list_state===%d\n", list_state);
				ret = GET_INDEXFILE_ERR;
				goto response_json;
			}
		} else if (list_src == RECORD_LIST) {
			list_state = reclist_to_file(list_path);
			if ((list_state == LIST_FILE_OK) || (list_state == LIST_FILE_IS_NONE)) {
				JSON_PRINT(SYS_DBG, "list_path===%s\n", list_path);
			} else if (list_state == LIST_FILE_DOWNLOAD) {
				ret = FILELIST_IS_UPLOADING;
				goto response_json;
			} else {
				JSON_PRINT(SYS_DBG, "list_state===%d\n", list_state);
				ret = GET_INDEXFILE_ERR;
				goto response_json;
			}
		} else if (list_src == TIMELAPSE_LIST) {
			list_state = reclapselist_to_file(list_path);
			if ((list_state == LIST_FILE_OK) || (list_state == LIST_FILE_IS_NONE)) {
				JSON_PRINT(SYS_DBG, "list_path===%s\n", list_path);
			} else if (list_state == LIST_FILE_DOWNLOAD) {
				ret = FILELIST_IS_UPLOADING;
				goto response_json;
			} else {
				JSON_PRINT(SYS_DBG, "list_state===%d\n", list_state);
				ret = GET_INDEXFILE_ERR;
				goto response_json;
			}
		}
#else
		if ((list_src == PICTURE_LIST) || (list_src == RECORD_LIST)) {
			if (list_src == PICTURE_LIST) { 
				list_state = piclist_to_file(list_path);
				if ((list_state == LIST_FILE_OK) || (list_state == LIST_FILE_IS_NONE)) {
					JSON_PRINT(SYS_INFO, "list_path===%s\n", list_path);
				} else if (list_state == LIST_FILE_DOWNLOAD) {
					ret = FILELIST_IS_UPLOADING;
					goto response_json;
				} else {
					JSON_PRINT(SYS_DBG, "list_state===%d\n", list_state);
					ret = GET_INDEXFILE_ERR;
					goto response_json;
				}
			} else if (list_src == RECORD_LIST) {
				list_state = reclist_to_file(list_path);
				if ((list_state == LIST_FILE_OK) || (list_state == LIST_FILE_IS_NONE)) {
					JSON_PRINT(SYS_INFO, "list_path===%s\n", list_path);
				} else if (list_state == LIST_FILE_DOWNLOAD) {
					ret = FILELIST_IS_UPLOADING;
					goto response_json;
				} else {
					JSON_PRINT(SYS_DBG, "list_state===%d\n", list_state);
					ret = GET_INDEXFILE_ERR;
					goto response_json;
				}
			}
		}
#endif //CONFIG_APP_DRONE
	}
#else // In 672 case
	if (list_src == RECORD_LIST) {
            list_state = reclist_to_file(list_path);
            if ((list_state == LIST_FILE_OK) || (list_state == LIST_FILE_IS_NONE)) {
                JSON_PRINT(SYS_DBG, "list_path===%s\n", list_path);
            } else if (list_state == LIST_FILE_DOWNLOAD) {
                ret = FILELIST_IS_UPLOADING;
                goto response_json;
            } else {
                JSON_PRINT(SYS_DBG, "list_state===%d\n", list_state);
                ret = GET_INDEXFILE_ERR;
                goto response_json;
            }
	}
	else if ((list_src > RECORD_LIST) && (list_src <= TIMELAPSE_LIST)) { //list_src range: 0~4
		list_state = chkdir_writetofile(list_src, list_path);
		if ((list_state == LIST_FILE_OK) || (list_state == LIST_FILE_IS_NONE)) {
			JSON_PRINT(SYS_DBG, "list_path===%s\n", list_path);
		} else if (list_state == LIST_FILE_DOWNLOAD) {
			ret = FILELIST_IS_UPLOADING;
			goto response_json;
		} else {
			JSON_PRINT(SYS_DBG, "list_state===%d\n", list_state);
			ret = 1;
			goto response_json;
		}
	}
#endif
	upload_status = startfileupload(psocket->addr.sin_addr.s_addr, psocket->mac, UPLOAD_FG, (uint16_t*)&port) ;
	if(upload_status != 0){
		JSON_PRINT(SYS_ERR, "startfileupload failed error:%d\n", upload_status);
		ret = 1;
		goto response_json;
	}
	
	upload_status = add_file_upload(list_path, psocket->addr.sin_addr.s_addr, psocket->mac, 0, UPLOAD_FG);
	if(upload_status != 0){
		JSON_PRINT(SYS_ERR, "add_file_upload failed error:%d\n", upload_status);
		ret = 1;
		goto response_json;
	}

#ifndef CONFIG_APP_DRONE
	if (list_src == PROTECT_LIST)
		is_full = get_protectfolder_full();
	else if (list_src == PICTURE_LIST)
		is_full = mf_snapshot_get_isfull();
	else if (list_src == RECORD_LIST)
		is_full = schedrec_get_isfull();
	else if (list_src == TIMELAPSE_LIST)
		is_full = reclapse_get_isfull();
	else
		JSON_PRINT(SYS_ERR, "parameter is not correct\n");
#else
	if (list_src == PICTURE_LIST)	//pic
		is_full = mf_snapshot_get_isfull();
	else if (list_src == RECORD_LIST)
		is_full = schedrec_get_isfull();
	else
		JSON_PRINT(SYS_ERR, "parameter is not correct\n");
#endif
	get_seed_from_file(&get_seed);
	JSON_PRINT(SYS_DBG, "get_seed=%d\n", get_seed);
	
response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "getindexfile_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status", DAEMON_ERRNO(CMD_GETINDEXFILE, ret)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "port", port) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "seed", get_seed) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "sdisfull", is_full) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}


static int get_filesize(const char *fname)
{
	FILINFO finfo;
	char lfn[ _MAX_LFN + 1 ];
	char *path0 = (char *)fname;
	
	finfo.lfname = lfn;
	finfo.lfsize = _MAX_LFN + 1;
	if (f_stat( path0, & finfo) == FR_OK) {
		JSON_PRINT(SYS_INFO, "(finfo.fsize)===%d byte \n", (finfo.fsize));
		return (finfo.fsize);
	} else {
		JSON_PRINT(SYS_ERR, "read file size error\n");
		return (-1);
	}
}

static buffer_t* js_downloadfilestart(snx_json_t *parse_json, void* param)
{
	buffer_t * resp_buf = NULL;
	snx_json_t *resp_json = NULL;
	SocketItem_t *psocket = (SocketItem_t*)param;
	char *output;
	int port = 0, ret = 0, download_method = UPLOAD_FG;
	int tag = 0;
	

	if (snx_json_obj_get_int(parse_json, "download_method", &download_method) != 0)
	{
		JSON_PRINT(SYS_ERR, "Couldn't get download_method\n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	}
	
	snx_json_obj_get_int(parse_json, "tag", &tag);
	
	ret = startfileupload(psocket->addr.sin_addr.s_addr, psocket->mac, download_method, (uint16_t*)&port) ;

	if (ret == 0)
	{
		JSON_PRINT(SYS_INFO, "Downloadfile start ip %d, method %s\n", psocket->addr.sin_addr.s_addr, (download_method ? "UPLOAD_FG" : "UPLOAD_BG"));
	}
	
response_json:
	if (!(resp_json = snx_json_obj_new()))
	{
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		return NULL;
	}

	if ( snx_json_obj_add_str(resp_json, "type", "downloadfilestart_res") != 0)
	{
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if ( snx_json_obj_add_int(resp_json, "resp_status",DAEMON_ERRNO(CMD_DOWNLOADFILESTART, ret)) != 0)
	{
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if ( snx_json_obj_add_int(resp_json, "port", port) != 0)
	{
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if ( snx_json_obj_add_int(resp_json, "download_method", download_method) != 0)
	{
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}
	
	if (snx_json_obj_add_int(resp_json, "tag", tag) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	resp_buf = buffer_new();
	if(!resp_buf)
	{
		JSON_PRINT(SYS_ERR, "resp buf allocate fail\n");
		goto release_json;
	}		
	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output,  strlen((char*)output));

release_json:
	snx_json_obj_free(resp_json);
	return g_resp_buf;
}


static buffer_t *js_downloadfile(snx_json_t *parse_json, void *param)
{
	snx_json_t *resp_json = NULL;
	SocketItem_t *psocket = (SocketItem_t *)param;
	char *output = NULL;
	char *fname = NULL, abs_path[LEN_FULLPATH] = {0};
	int pos = 0, ret = 0, filetype = 0, tag = 0, number = 0, file_size = 0;
	int download_method = UPLOAD_FG;
	int sdstate = 0;
	char newname[LEN_FILENAME] = {0};
	int upload_status = 0, port = 0;

	if (snx_json_obj_get_str(parse_json, "filename", &fname) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON string obj \n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	}

	if (snx_json_obj_get_int(parse_json, "pos", &pos) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	}

	if (snx_json_obj_get_int(parse_json, "number", &number) != 0) {
		//for support old version
		number = 0;
	}
	
	if (snx_json_obj_get_int(parse_json, "filetype", &filetype) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	}

	if (snx_json_obj_get_int(parse_json, "download_method", &download_method) != 0)
	{
		JSON_PRINT(SYS_ERR, "Couldn't get download_method\n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	}
	
	snx_json_obj_get_int(parse_json, "tag", &tag);

    JSON_PRINT(SYS_INFO, "; fname: %s; filetype: %i\n", fname, filetype);

	sdstate = app_uti_chk_sd_state();
	if(sdstate != SD_OK) {
		ret = sdstate;
		goto response_json;
	}

#if LIMIT_PLAYBACK_AND_DOWNLOAD_CONNECTION
	//check connection number of download file and playback.
	if ((filetype == 0) || (filetype == 1) || (filetype == 3)) { //filetype 0:record file. 1:protect file. 3:timelapse file
		if (is_addr_exist_in_dwnlod_pb_conn(psocket->mac)) {
			dwnlod_and_pb_conn_num++;
			JSON_PRINT(SYS_DBG, "Download and Playback conn = %d\n", dwnlod_and_pb_conn_num);
			psocket->status = 1;
		} else {
			if (dwnlod_and_pb_conn_num >= dwnload_and_playback_max_conn_number) {
				JSON_PRINT(SYS_ERR, "DownloadFile and Playback connections is up limit.\n");
				JSON_PRINT(SYS_ERR, "Max Conn = %d, Current Conn = %d\n", dwnload_and_playback_max_conn_number, dwnlod_and_pb_conn_num);
				ret = OVER_DOWLOD_AND_PB_MAX_CONN;
				goto response_json;
			} else {
				psocket->status = 1;
				add_conn_to_pb_and_dwnlod(psocket->addr.sin_addr.s_addr, psocket->mac, 1);
				dwnlod_and_pb_conn_num++;
				JSON_PRINT(SYS_DBG, "Download and Playback conn = %d\n", dwnlod_and_pb_conn_num);
			}
		}
	}
#endif
	if ((number == 0) && ((filetype >= QUERY_RECORD_THUMBNAIL_FILE) && (filetype <= QUERY_TIMELAPSE_THUMBNAIL_FILE))) {
		rename_recfile_addlen(fname, newname, 0);
	} else {
		snprintf(newname, sizeof(newname), "%s", fname);
	}

	rec_query_abspath(filetype, abs_path, sizeof(abs_path), newname);

	JSON_PRINT(SYS_DBG, "path = %s\n", abs_path);
	if ((file_size = get_filesize(abs_path) ) == -1) {
		ret = GET_FILE_SIZE_ERR;
		goto response_json;
	}

	upload_status = startfileupload(psocket->addr.sin_addr.s_addr, psocket->mac, download_method, (uint16_t*)&port) ;
	if(upload_status != 0){
		JSON_PRINT(SYS_ERR, "startfileupload failed error:%d\n", upload_status);
		ret = 1;
		goto response_json;
	}
	
	upload_status = add_file_upload(abs_path, psocket->addr.sin_addr.s_addr, psocket->mac, pos, download_method);
	if(upload_status != 0){
		JSON_PRINT(SYS_ERR, "add_file_upload failed error:%d\n", upload_status);
		ret = 1;
		goto response_json;
	}

response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "downloadfile_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status", DAEMON_ERRNO(CMD_DOWNLOADFILE, ret)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "port", port) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "filesize", file_size) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "tag", tag) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}

static buffer_t *js_downloadfilefinish(snx_json_t *parse_json, void *param)
{
	SocketItem_t *psocket = (SocketItem_t *)param;
	snx_json_t *resp_json = NULL;
	int ret = 0, tag = 0, download_method = UPLOAD_FG;
	char *output = NULL;

	if (snx_json_obj_get_int(parse_json, "download_method", &download_method) != 0)	{
		JSON_PRINT(SYS_ERR, "Couldn't get download_method\n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	 }

	snx_json_obj_get_int(parse_json, "tag", &tag);
#if LIMIT_PLAYBACK_AND_DOWNLOAD_CONNECTION
	if ((filetype == 0) || (filetype == 1) || (filetype == 3)) { //filetype 0:record file. 1:protect file. 3:timelapse file
		if (is_addr_exist_in_dwnlod_pb_conn(psocket->mac)) {
			psocket->status = 0;
			set_dwnlod_pb_conn_status(psocket->mac, 0);
			dwnlod_and_pb_conn_num--;
			JSON_PRINT(SYS_DBG, "Download and Playback conn = %d\n", dwnlod_and_pb_conn_num);
		} else {
			psocket->status = 0;
			JSON_PRINT(SYS_DBG, "Download and Playback conn = %d\n", dwnlod_and_pb_conn_num);
		}
	}
#endif

	if (download_method == UPLOAD_BG)
	        ret = stopfileupload(psocket->mac, UPLOAD_BG);
	else //UPLOAD_FG
	        ret = stopfileupload(psocket->mac, UPLOAD_FG);
	
	if (ret == OK) {
		JSON_PRINT(SYS_INFO, "Downloadfile finish ip %d, method %s\n", psocket->addr.sin_addr.s_addr, (download_method ? "UPLOAD_FG" : "UPLOAD_BG"));
	}

response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "downloadfilefinish_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status", DAEMON_ERRNO(CMD_DOWNLOADFILEFINISH, ret)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "tag", tag) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if ( snx_json_obj_add_int(resp_json, "download_method", download_method) != 0)
	{
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}
	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}

static buffer_t *js_deletefile(snx_json_t *parse_json, void *param)
{
	snx_json_t *resp_json = NULL;
	char *fname = NULL;
	int ret = 0, filetype = -1;
	char *output = NULL;
	char abs_path[LEN_FULLPATH] = {0};
	int sdstate = 0;

	if (snx_json_obj_get_str(parse_json, "filename", &fname) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON string obj \n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	}

	if (snx_json_obj_get_int(parse_json, "filetype", &filetype) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	}

	sdstate = app_uti_chk_sd_state();
	if(sdstate != SD_OK) {
		ret = sdstate;
		goto response_json;
	}

	if (filetype != QUERY_RECORD_FILE && filetype != QUERY_PROTECT_FILE && filetype != QUERY_PIC_FILE) {
		JSON_PRINT(SYS_ERR, "Unsupport type (%d) in delete file\n", filetype);
	}

	rec_query_abspath(filetype, abs_path, sizeof(abs_path), fname);
	JSON_PRINT(SYS_DBG, "path = %s\n", abs_path);

	//notice record to update file list
	if(filetype == QUERY_RECORD_FILE) { //record
		ret = app_uti_del_record_file(abs_path, fname);
	}
#ifndef CONFIG_APP_DRONE
	else if(filetype == QUERY_PROTECT_FILE) { //protect
		ret = app_uti_del_protect_file(abs_path, fname);
	}
#endif
	else if(filetype == QUERY_PIC_FILE) { //pic
		ret = app_uti_del_picture_file(abs_path, fname);
	}
	else {
		JSON_PRINT(SYS_ERR, "Unsupport delete type (%d) = %s\n", filetype, abs_path);
	}

response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "deletefile_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status", DAEMON_ERRNO(CMD_DELETEFILE, ret)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	output = snx_json_object_to_json_string(resp_json);
	JSON_PRINT(SYS_DBG, "response_json:\n%s\n", output);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}


static buffer_t *js_streamuvc(snx_json_t *parse_json, void *param)
{
	snx_json_t *resp_json = NULL;
	int view_enable = -1;
	char *output = NULL;
	int ret = 0;

	if (snx_json_obj_get_int(parse_json, "viewenable", &view_enable) != 0)
	{
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	}

	ret = app_uti_set_stream_uvc(view_enable);

response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "streamuvc_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}
	
	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;

}


static buffer_t *js_streamvideo(snx_json_t *parse_json, void *param)
{
#if 0
	JSON_PRINT(SYS_ERR, "NO Support this command\n");
	JSON_PRINT(SYS_ERR, "NO Support this command\n");
	JSON_PRINT(SYS_ERR, "NO Support this command\n");
	JSON_PRINT(SYS_ERR, "NO Support this command\n");
	JSON_PRINT(SYS_ERR, "NO Support this command\n");
#endif

#if 0
	buffer_t *resp_buf = NULL;
	snx_json_t *resp_json;
	SocketItem_t *psocket = (SocketItem_t *)param;
	char *output;
	char *fname, frename[LEN_FILENAME];
	char *mask, fnamemask[8];
	char abs_path[LEN_FULLPATH];
	int live_view, filetype, ret = 0;

	//1: live preview, 0: sd playback
	if (snx_json_obj_get_int(parse_json, "live", &live_view) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	}

	if (snx_json_obj_get_str(parse_json, "filename", &fname) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON string obj \n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	}

	// 0 : record file. 	1 : protect file.
	if (snx_json_obj_get_int(parse_json, "filetype", &filetype) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	}

	if (live_view) {
		//enable preview encoder when no other user set preview
		if (conn_list_len(&preview_list) == 0)
			mf_set_preview(1);

		//restart record when all user leave playback
		if (conn_list_len(&playback_list) == 0) {
			schedrec_suspend_restart(0);
#ifndef CONFIG_APP_DRONE
			reclapse_suspend_restart(0);
#endif
		}
		conn_list_add(&preview_list, psocket->addr.sin_addr.s_addr, psocket->mac);
		if (snx_json_obj_get_str(parse_json, "mask", &mask) != 0) {
			strcpy(frename, LIVE_PREVIEW_STREAM_NAME);

		} else {
			strcpy(fnamemask, LIVE_PREVIEW_STREAM_Body);
			int i = 0;
			int length = strlen(fnamemask);
			for (i = 0; i < length; i++) {
				fnamemask[i] = (char)(fnamemask[i] ^ (mask[i]));
			}
			strcpy(frename, LIVE_PREVIEW_STREAM_Head);
			strcat(frename, fnamemask);
		}
	} else {
#if LIMIT_PLAYBACK_AND_DOWNLOAD_CONNECTION
		if (is_addr_exist_in_dwnlod_pb_conn(psocket->mac)) {
			dwnlod_and_pb_conn_num++;
			JSON_PRINT(SYS_INFO, "Download and Playback conn = %d\n", dwnlod_and_pb_conn_num);
			psocket->status = 2;
		} else {
			if (dwnlod_and_pb_conn_num >= dwnload_and_playback_max_conn_number) {
				JSON_PRINT(SYS_ERR, "DownloadFile and Playback connections is up limit.\n");
				JSON_PRINT(SYS_ERR, "Max Conn = %d, Current Conn = %d\n", dwnload_and_playback_max_conn_number, dwnlod_and_pb_conn_num);
				ret = OVER_DOWLOD_AND_PB_MAX_CONN;
				goto response_json;
			} else {
				psocket->status = 2;
				add_conn_to_pb_and_dwnlod(psocket->addr.sin_addr.s_addr, psocket->mac, 2);
				dwnlod_and_pb_conn_num++;
				JSON_PRINT(SYS_INFO, "Download and Playback conn = %d\n", dwnlod_and_pb_conn_num);
			}
		}
#endif
		schedrec_suspend();	//close record
#ifndef CONFIG_APP_DRONE
		reclapse_suspend();
#endif

		rec_query_abspath(filetype, abs_path, sizeof(abs_path), fname);
		if (filetype != QUERY_RECORD_FILE && filetype != QUERY_PROTECT_FILE) {
			JSON_PRINT(SYS_ERR, "Unsupport type (%d) in streamvideo\n", filetype);
		}

		if (strlen(psocket->rtsp_fname) > 0)
			pb_stop(psocket->mac);

		memset(frename, 0, sizeof(frename));
		ret = pb_receive_cmd(abs_path, psocket->addr.sin_addr.s_addr, psocket->mac, frename);
		if (ret == 0) {
			memcpy(psocket->rtsp_fname, frename, sizeof(psocket->rtsp_fname));
			conn_list_add(&playback_list, psocket->addr.sin_addr.s_addr, psocket->mac);
		}
		JSON_PRINT(SYS_DBG, "abs_path = %s\n", abs_path);
		JSON_PRINT(SYS_DBG, "frename = %s\n", frename);
	}

response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "streamvideo_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "live", live_view) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status", DAEMON_ERRNO(CMD_STREAMVIDEO, abs(ret))) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_str(resp_json, "rtspname", frename) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON str obj \n");
		goto release_json;
	}

	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
end:
#endif

	buffer_t * resp_buf = NULL;
	snx_json_t *resp_json;
	SocketItem_t *psocket = (SocketItem_t*)param;
	char *output;
	char *fname, frename[100];
	char *mask,fnamemask[8];
	char abs_path[300];
	int live_view, filetype, ret=0;
	//JSON_PRINT_QUEUE("\n");

	//1: live preview, 0: sd playback
	if ( snx_json_obj_get_int(parse_json, "live", &live_view) != 0)
	{
		JSON_PRINT_QUEUE_ERR("Couldn't get JSON int obj \n");
	}

	if ( snx_json_obj_get_str(parse_json, "filename", &fname) != 0)
	{
		JSON_PRINT_QUEUE_ERR("Couldn't get JSON string obj \n");
		goto end;
	}

	// 0 : record file. 	1 : protect file.
	if (snx_json_obj_get_int(parse_json, "filetype", &filetype) != 0)
	{
		JSON_PRINT_QUEUE_ERR("Couldn't get JSON int obj \n");
		goto end;
	}

	if(live_view)
	{
		//enable preview encoder when no other user set preview
		if(conn_list_len(&preview_list)==0)
			mf_set_preview(1);			

		//restart record when all user leave playback
		if(conn_list_len(&playback_list)==0)
		{
			schedrec_suspend_restart(0);
#ifndef CONFIG_APP_DRONE
			reclapse_suspend_restart(0);
#endif
		}
		conn_list_add(&preview_list,psocket->addr.sin_addr.s_addr,psocket->mac);
		if ( snx_json_obj_get_str(parse_json, "mask", &mask) != 0)
		{
			strcpy(frename, LIVE_PREVIEW_STREAM_NAME);

		}
		else
		{
			strcpy(fnamemask, LIVE_PREVIEW_STREAM_Body);
			int i=0;
			int length=strlen(fnamemask);
			for (i = 0; i <length;i++) 
			{
				fnamemask[i] =(char)(fnamemask[i]^(mask[i]));
			}	
			strcpy (frename,LIVE_PREVIEW_STREAM_Head);   
			strcat (frename,fnamemask);     
		} 

		// ----- HotGen -----
		// Trying to stop auto-record!!!
		user_diable_rec();
#ifndef CONFIG_APP_DRONE
		user_disable_reclapse();
#endif
		// ----- HotGen -----

	}
	else
	{
#if LIMIT_PLAYBACK_AND_DOWNLOAD_CONNECTION
		if (is_ip_exist_in_dwnlod_pb_conn(psocket->addr.sin_addr.s_addr))
		{
			dwnlod_and_pb_conn_num++;	
			JSON_PRINT_QUEUE("Download and Playback conn = %d\n", dwnlod_and_pb_conn_num);
			psocket->status = 2;
		}
		else
		{
			if (dwnlod_and_pb_conn_num >= dwnload_and_playback_max_conn_number)
			{
				JSON_PRINT_QUEUE_ERR("DownloadFile and Playback connections is up limit.\n");
				JSON_PRINT_QUEUE_ERR("Max Conn = %d, Current Conn = %d\n", dwnload_and_playback_max_conn_number, dwnlod_and_pb_conn_num);
				ret = 3;	
				goto resp_json;
			}
			else
			{
				psocket->status = 2;
				add_conn_to_pb_and_dwnlod(psocket->addr.sin_addr.s_addr, 2);
				dwnlod_and_pb_conn_num++;	
				JSON_PRINT_QUEUE("Download and Playback conn = %d\n", dwnlod_and_pb_conn_num);
			}
		}
#endif
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
		ret = pb_receive_cmd(abs_path,psocket->addr.sin_addr.s_addr, psocket->mac, frename);
		if(ret==0)
		{
			memcpy(psocket->rtsp_fname, frename, sizeof(psocket->rtsp_fname));
			conn_list_add(&playback_list,psocket->addr.sin_addr.s_addr,psocket->mac);
		}
		JSON_PRINT_QUEUE("abs_path = %s\n", abs_path);
		JSON_PRINT_QUEUE("frename = %s\n", frename);
	}

#if LIMIT_PLAYBACK_AND_DOWNLOAD_CONNECTION
resp_json:
#endif
	if (!(resp_json = snx_json_obj_new()))
	{
		JSON_PRINT_QUEUE_ERR("Couldn't create JSON obj \n");
		return NULL;
	}

	if ( snx_json_obj_add_str(resp_json, "type", "streamvideo_res") != 0)
	{
		JSON_PRINT_QUEUE_ERR("Couldn't add JSON string obj \n");
		goto release_json;
	}

	if ( snx_json_obj_add_int(resp_json, "live", live_view) != 0)
	{
		JSON_PRINT_QUEUE_ERR("Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if ( snx_json_obj_add_int(resp_json, "status", DAEMON_ERRNO(CMD_STREAMVIDEO, abs(ret))) != 0)
	{
		JSON_PRINT_QUEUE_ERR("Couldn't add JSON integer obj \n");
		goto release_json;
	}

	//if ( snx_json_obj_add_str(resp_json, "rtspname", frename) != 0)
	if ( snx_json_obj_add_str(resp_json, "rtspname", frename) != 0)
	{
		JSON_PRINT_QUEUE_ERR("Couldn't add JSON str obj \n");
		goto release_json;
	}

	resp_buf = buffer_new();
	if(!resp_buf)
	{
		JSON_PRINT_QUEUE_ERR("resp buf allocate fail\n");
		goto release_json;
	}		
	output = snx_json_object_to_json_string(resp_json);
	buffer_add(resp_buf, output,  strlen((char*)output));

release_json:
	snx_json_obj_free(resp_json);
end:	
	return resp_buf;
}


static buffer_t *js_streamvideofinish(snx_json_t *parse_json, void *param)
{
#if 0
	JSON_PRINT(SYS_ERR, "NO Support this command\n");
	JSON_PRINT(SYS_ERR, "NO Support this command\n");
	JSON_PRINT(SYS_ERR, "NO Support this command\n");
	JSON_PRINT(SYS_ERR, "NO Support this command\n");
	JSON_PRINT(SYS_ERR, "NO Support this command\n");
#endif

#if 0
	buffer_t *resp_buf = NULL;
	snx_json_t *resp_json;
	SocketItem_t *psocket = (SocketItem_t *)param;
	char *output;
	char *fname;
	char *tag;
	char  frename[100] = {0};
	int ret = 0;

	if (snx_json_obj_get_str(parse_json, "filename", &fname) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON string obj \n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	}
	
	if (snx_json_obj_get_str(parse_json, "mask", &tag) != 0) {
		strcpy(frename, fname);
		JSON_PRINT(SYS_DBG, "frename==%s\n", frename);
	} else {
		strcpy(frename, LIVE_PREVIEW_STREAM_NAME);
		JSON_PRINT("SYS_DBG, frename==%s\n", frename);
	}

	if (strcmp(frename, LIVE_PREVIEW_STREAM_NAME) == 0) {
		conn_list_del(&preview_list, psocket->mac);
		if (conn_list_len(&preview_list) == 0)
			mf_set_preview(0);
	} else {
		pb_stop(psocket->mac);
		conn_list_del(&playback_list, psocket->mac);
		//clear rtsp filename
		memset(psocket->rtsp_fname, 0, sizeof(psocket->rtsp_fname));

#if LIMIT_PLAYBACK_AND_DOWNLOAD_CONNECTION
		if (is_addr_exist_in_dwnlod_pb_conn(psocket->mac)) {
			psocket->status = 0;
			set_dwnlod_pb_conn_status(psocket->mac, 0);
			dwnlod_and_pb_conn_num--;
			JSON_PRINT(SYS_INFO, "Download and Playback conn = %d\n", dwnlod_and_pb_conn_num);
		} else {
			psocket->status = 0;
			JSON_PRINT(SYS_INFO, "Download and Playback conn = %d\n", dwnlod_and_pb_conn_num);
		}
#endif
	}

response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "streamvideofinish_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status", DAEMON_ERRNO(CMD_STREAMVIDEOFINISH, 0)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
end:
#endif

	buffer_t * resp_buf = NULL;
	snx_json_t *resp_json;
	SocketItem_t *psocket = (SocketItem_t*)param;
	char *output;
	char *fname;
	char *tag;
	char  frename[100]={0};
	//JSON_PRINT_QUEUE("\n");

	if (snx_json_obj_get_str(parse_json, "filename", &fname) != 0)
	{
		JSON_PRINT_QUEUE_ERR("Couldn't get JSON string obj \n");
		goto end;
	}
	if ( snx_json_obj_get_str(parse_json, "mask", &tag) != 0)
	{
		strcpy(frename, fname);
		JSON_PRINT_QUEUE("frename==%s\n",frename);
	}
	else
	{
		strcpy(frename, LIVE_PREVIEW_STREAM_NAME);
		JSON_PRINT_QUEUE("frename==%s\n",frename);
	}

	if(strcmp(frename,LIVE_PREVIEW_STREAM_NAME)==0)
	{
//		conn_list_del(&preview_list,psocket->addr.sin_addr.s_addr);
		conn_list_del(&preview_list,psocket->mac);

		if(conn_list_len(&preview_list)==0)
			mf_set_preview(0);
	}
	else
	{
		pb_stop(psocket->addr.sin_addr.s_addr); 		
//		conn_list_del(&playback_list,psocket->addr.sin_addr.s_addr);
		conn_list_del(&playback_list,psocket->mac);
		//clear rtsp filename
		memset(psocket->rtsp_fname, 0, sizeof(psocket->rtsp_fname));

#if LIMIT_PLAYBACK_AND_DOWNLOAD_CONNECTION
		if (is_ip_exist_in_dwnlod_pb_conn(psocket->addr.sin_addr.s_addr))
		{
			psocket->status = 0;
			set_dwnlod_pb_conn_status(psocket->addr.sin_addr.s_addr, 0);
			dwnlod_and_pb_conn_num--;
			JSON_PRINT_QUEUE("Download and Playback conn = %d\n", dwnlod_and_pb_conn_num);
		}
		else
		{
			psocket->status = 0;
			JSON_PRINT_QUEUE("Download and Playback conn = %d\n", dwnlod_and_pb_conn_num);
		}
#endif
	}

	if (!(resp_json = snx_json_obj_new()))
	{
		JSON_PRINT_QUEUE_ERR("Couldn't create JSON obj \n");
		return NULL;
	}

	if ( snx_json_obj_add_str(resp_json, "type", "streamvideofinish_res") != 0)
	{
		JSON_PRINT_QUEUE_ERR("Couldn't add JSON string obj \n");
		goto release_json;
	}

	if ( snx_json_obj_add_int(resp_json, "status", DAEMON_ERRNO(CMD_STREAMVIDEOFINISH, 0)) != 0)
	{
		JSON_PRINT_QUEUE_ERR("Couldn't add JSON integer obj \n");
		goto release_json;
	}

	resp_buf = buffer_new();
	if(!resp_buf)
	{
		JSON_PRINT_QUEUE_ERR("resp buf allocate fail\n");
		goto release_json;
	}		
	output = snx_json_object_to_json_string(resp_json);
	buffer_add(resp_buf, output,  strlen((char*)output));

release_json:
	snx_json_obj_free(resp_json);
end:	
	return resp_buf;
}

static buffer_t *js_set_osdonoff(snx_json_t *parse_json, void *param)
{
	snx_json_t *resp_json = NULL;
	char *output = NULL;
	int osdenable = -1;
	int ret = 0;

	if (snx_json_obj_get_int(parse_json, "osd", &osdenable)) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	}

	if ((ret = app_uti_set_osd_onoff(osdenable) ) != OK ) {
		// No set OSD string, show a warning message...
		if(ret ==  GET_OSD_STRING_ERR)
			ret = OK;
		goto response_json;
	}

	if (chk_preview_use_isp0dup() == 1) {
		JSON_PRINT(SYS_DBG, "osd:chk_preview_use_isp0dup\n");
		osd_preview_is_dup1_setting();
	} else {
		JSON_PRINT(SYS_DBG, "osd:CheckPreviewUseIsp1\n");
		osd_preview_is_isp1_setting();
	}


response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "setosdonoff_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status", DAEMON_ERRNO(CMD_SETOSDONOFF, ret)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	output = snx_json_object_to_json_string(resp_json);
	JSON_PRINT(SYS_DBG, "response_json:\n%s\n", output);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}

static buffer_t *js_get_osdstatus(snx_json_t *parse_json, void *param)
{
	snx_json_t *resp_json = NULL, *jobj_arr_int = NULL;
	char *output = NULL;
	int osdenable = 0;
	unsigned int len = 0;
	int ret = 0;
	unsigned int *val = NULL;
	int i = 0;
	int nvramnodata = 0;
	unsigned char *osd_str = NULL;

	ret = app_uti_get_osd_status(&osdenable, &osd_str, &nvramnodata, &len);
	if( ret != 0) {
		JSON_PRINT(SYS_ERR, "Get OSD Status from NVRAM failed!");
		goto end;
	}

	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		vPortFree(osd_str);
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "getosdstatus_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "osd", osdenable) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}
	
	if (nvramnodata == 0) {
		if ((snx_json_obj_add_array(resp_json, "unicode", &jobj_arr_int)) != 0) {
			goto release_json;
		}
		val = (unsigned int *)osd_str;
		for (i = 0 ; i < (len / 4) ; i++, val++) {
			if (snx_json_obj_add_array_int(jobj_arr_int, *val) != 0) {
				JSON_PRINT(SYS_ERR, "Couldn't add JSON int obj \n");
				snx_json_obj_free(jobj_arr_int);
				goto release_json;
			}
		}
	}

	if (snx_json_obj_add_int(resp_json, "resp_status",	DAEMON_ERRNO(CMD_GETOSDSTATUS, ret)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output,  strlen((char *)output));
release_json:
	vPortFree(osd_str);
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}



static buffer_t *js_sendfontfile(snx_json_t *parse_json, void *param)
{
	snx_json_t *resp_json;
	char *output;
	uint16_t port = 0;
	int ret = 0;
	snx_json_t *font_arr = NULL;
	snx_json_t *unicode_arr = NULL;
	int i, unicode_cnt = 0;
	int filesize = 0;
	int unicode_data = 0;
	int *nvram_unicode_data = NULL;

	if (snx_json_obj_get_array(parse_json, "showString", &font_arr, &unicode_cnt) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON string obj \n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	}

	if ((ret = snx_json_obj_get_int(parse_json, "fileSize", &filesize))) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	}


	if (check_dwnlodFont_task() != 0) {
		JSON_PRINT(SYS_ERR, "Download_FontFile task is already running.\n");
		ret = DOWNLOD_FONE_TASK_IS_RUNNGIN;
		goto response_json;
	}


	if (unicode_cnt > 0) {
		if (!(nvram_unicode_data = pvPortMalloc(sizeof(int) * unicode_cnt, GFP_KERNEL, MODULE_APP))) {
			JSON_PRINT(SYS_ERR, "Create unicode data buffer failed (size = %d)!\n", sizeof(int) * unicode_cnt);
			ret = MEMORY_IS_NOT_ENOUGH;
			goto end;
		}

		for (i = 0; i < unicode_cnt; i++) {
			snx_json_obj_get_array_obj(font_arr, i, &unicode_arr);
			snx_json_obj_get_int(unicode_arr, "unicode", &unicode_data);
			nvram_unicode_data[i] = unicode_data;
		}

		if (nvram_unicode_data != NULL) {
			snx_nvram_uchar_hex_set("app_osd_string", "osd_string", ( unsigned char *)&nvram_unicode_data[0], sizeof(int)*unicode_cnt);
			vPortFree(nvram_unicode_data);
		}
	}

	if (socket_download_font_create(&port, filesize) == pdFAIL) {
		JSON_PRINT(SYS_ERR, "create download socket fail\n");
		ret = CREATE_DOWNLOD_SOCKET_ERR;	//create socket fail
	}

response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "sendfontfile_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status", DAEMON_ERRNO(CMD_SENDFONTFILE, ret)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "port", (int)port) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}

#undef UPGRADE_HACK
#ifdef UPGRADE_HACK
unsigned int fb_version = 0;
#endif

static buffer_t *js_sendfwbin(snx_json_t *parse_json, void *param)
{
	snx_json_t *resp_json = NULL;
	SocketItem_t *psocket = (SocketItem_t *)param;
	int ret = 0;
	char *type = NULL;
	int filesize = 0;
	uint16_t port = 0;
	char *output;
    fwupgrade_mode_t mode = FBFWMODE;

	JSON_PRINT(SYS_DBG, " - trace\n");

	if ((ret = snx_json_obj_get_str(parse_json, "type", &type))) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON string obj \n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	}

	JSON_PRINT(SYS_DBG, " - type: %s\n", type);

	if ((ret = snx_json_obj_get_int(parse_json, "fileSize", &filesize))) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	}

	if ((ret = snx_json_obj_get_int(parse_json, "mode", &mode))) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
#ifdef CONFIG_SUPPORT_MULTIPLE_UPGRADE_MODES
		ret = GET_OBJECT_FAIL;
		goto response_json;
#endif
	}

	JSON_PRINT(SYS_DBG, " - filesize: %u\n", filesize);

	if (check_fwupgrad_task() == FWUPGRAD_TASK_RUNNING) {
		JSON_PRINT(SYS_ERR, "FwUpgrade task is already running.\n");
		ret = UPGRADE_IS_RUNNING;
		goto response_json;
	}

	JSON_PRINT(SYS_DBG, " - trace\n");

	//download firmware process
	if (socket_download_fw_create(&port, filesize, psocket, mode) == pdFAIL) {
		JSON_PRINT(SYS_ERR, "create download socket fail\n");
		ret = CREATE_DOWNLOAD_SOCKET_ERR;
	}

response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "sendfwbin_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status", DAEMON_ERRNO(CMD_SENDFWBIN, ret)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "port", (int)port) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output,  strlen((char *)output));
	
#ifdef UPGRADE_HACK
	JSON_PRINT(SYS_DBG, "Pretend FB version is 270\n");
	fb_version = 270;
#endif

release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}

static buffer_t *js_upgradefw(snx_json_t *parse_json, void *param)
{
	snx_json_t *resp_json = NULL;
	SocketItem_t *psocket = (SocketItem_t *)param;
	int ret = 0;
	int fw_download_status ;
	char *type = NULL;
	char *output = NULL;

	JSON_PRINT(SYS_DBG, " - trace\n");

	if ((ret = snx_json_obj_get_str(parse_json, "type", &type))) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON string obj \n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	}

	fw_download_status = get_download_fw_status(psocket);
	if (fw_download_status != -1)
		ret = fw_download_status;
	else
		ret = FWUPGRADE_INIT_ERR;
	JSON_PRINT(SYS_INFO, "fw_download_status: %d\n", ret);

response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "upgradefw_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status", DAEMON_ERRNO(CMD_UPGRADEFW, ret)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}

/**
 * @brief interface function - change json command main control source
 * @return none
 */
void json_cmd_mctrl_src_change(json_command_source_t cmd_src)
{
	json_cmd_ctrl_src = cmd_src;
}

static buffer_t *js_nvramresettodefault(snx_json_t *parse_json, void *param)
{
	snx_json_t *resp_json = NULL;
	int rc = 0;
	char *output;

	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "nvramresettodefault_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status",	DAEMON_ERRNO(CMD_NVRAMRESETTODEFAULT, rc)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}
	
	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

	rc = app_uti_set_nvram_to_default();
release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}


static buffer_t *js_usbdclassmode(snx_json_t *parse_json, void *param)
{
	int usbdclassmode = -1, ret = 0;
	snx_json_t *resp_json = NULL;
	char *output = NULL;

	if (snx_json_obj_get_int(parse_json, "usbdclassmode", &usbdclassmode)) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	}

	ret = app_uti_set_class_mode(usbdclassmode);

response_json:
	if (!(resp_json = snx_json_obj_new())) {
		JSON_PRINT(SYS_ERR, "Couldn't create JSON obj \n");
		goto end;
	}

	if (snx_json_obj_add_str(resp_json, "type", "usbdclassmode_res") != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON string obj \n");
		goto release_json;
	}

	if (snx_json_obj_add_int(resp_json, "resp_status", DAEMON_ERRNO(CMD_USBDCLASSMODE, ret)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}
	
	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output,  strlen((char *)output));


release_json:
	snx_json_obj_free(resp_json);
end:
	return g_resp_buf;
}

static buffer_t* js_set_playbackseek(snx_json_t *parse_json, void* param)
{
	char *client_ip;
	int timestamp;
	snx_json_t *resp_json;
	buffer_t * resp_buf = NULL;
	int rc = 0;
	struct in_addr addr;
	DWORD ip;
	char *output;

	if (snx_json_obj_get_str(parse_json, "client_ip", &client_ip))
	{
		JSON_PRINT_QUEUE_ERR("Couldn't get JSON string obj \n");
		goto end;
	}

	if (snx_json_obj_get_int(parse_json, "timestamp", &timestamp))
	{
		JSON_PRINT_QUEUE_ERR("Couldn't get JSON int obj \n");
		goto end;
	}

	if (!inet_aton(client_ip, &addr))
	{
		JSON_PRINT_QUEUE_ERR("ip address transvert failed \n");
		goto end;
	}

	ip = ntohl(addr.s_addr);
	JSON_PRINT(SYS_INFO, ": ip = %lu\n", ip);

	pb_seek((char *)ip, (uint32_t)timestamp);

	if (!(resp_json = snx_json_obj_new()))
	{
		JSON_PRINT_QUEUE_ERR("Couldn't create JSON obj \n");
		return NULL;
	}

	if ( snx_json_obj_add_int(resp_json, "status",	DAEMON_ERRNO(CMD_SETPLAYBACKSEEK,rc==0?0:1)) != 0)
	{
		JSON_PRINT_QUEUE_ERR("Couldn't add JSON integer obj \n");
		goto release_json;
	}
	resp_buf = buffer_new();
	if(!resp_buf)
	{
		JSON_PRINT_QUEUE_ERR("resp buf allocate fail\n");
		goto release_json;
	}		
	output = snx_json_object_to_json_string(resp_json);
	buffer_add(resp_buf, output,  strlen((char*)output));

release_json:
	snx_json_obj_free(resp_json);
end:	
	return resp_buf;
}

static buffer_t* js_notifyrtspstop(snx_json_t *parse_json, void* param)
{
	buffer_t * resp_buf = NULL;
	/***
	snx_json_t *resp_json;
	SocketItem_t *psocket = (SocketItem_t*)param;
	char *output;
	int index;
	int ret;

	if (snx_json_obj_get_int(parse_json, "index", &index))
	{
		JSON_PRINT_QUEUE_ERR("Couldn't get JSON int obj \n");
		goto end;
	}

	JSON_PRINT(SYS_INFO, "ip:%s\n", inet_ntoa(psocket->addr.sin_addr.s_addr));
	ret =  close_rtsp_stream_in_different_stage(inet_ntoa(psocket->addr.sin_addr.s_addr));
	JSON_PRINT(SYS_INFO, "ret:%d\n", ret);

	if (!(resp_json = snx_json_obj_new()))
	{
		JSON_PRINT_QUEUE_ERR("Couldn't create JSON obj \n");
		return NULL;
	}

	if ( snx_json_obj_add_str(resp_json, "type", "notifyrtspstop_res") != 0)
	{
		JSON_PRINT_QUEUE_ERR("Couldn't add JSON string obj \n");
		goto release_json;
	}

	if ( snx_json_obj_add_int(resp_json, "index", index) != 0)
	{
		JSON_PRINT_QUEUE_ERR("Couldn't add JSON integer obj \n");
		goto release_json;
	}

	if ( snx_json_obj_add_int(resp_json, "status", DAEMON_ERRNO(CMD_NOTIFYRTSPSTOP, ret)) != 0)
	{
		JSON_PRINT_QUEUE_ERR("Couldn't add JSON integer obj \n");
		goto release_json;
	}

	resp_buf = buffer_new();
	if(!resp_buf)
	{
		JSON_PRINT_QUEUE_ERR("resp buf allocate fail\n");
		goto release_json;
	}		
	output = snx_json_object_to_json_string(resp_json);
	buffer_add(resp_buf, output,  strlen((char*)output));
release_json:
	snx_json_obj_free(resp_json);
end:
	***/
	return resp_buf;
}

// ----- HotGen -----
static buffer_t* js_set_flight_response(snx_json_t *parse_json, void* param)
{
	int index = 0;
	int value = 0;
	int ret = 0;
	snx_json_t *resp_json;
	buffer_t * resp_buf = NULL;
	char *output;

	if (snx_json_obj_get_int(parse_json, "index", &index))
	{
		JSON_PRINT_QUEUE_ERR("Couldn't get JSON int obj \n");
		goto end;
	}

	if (snx_json_obj_get_int(parse_json, "value", &value))
	{
		JSON_PRINT_QUEUE_ERR("Couldn't get JSON int obj \n");
		goto end;
	}
	
#if CONFIG_MAVLINK_WIFI
        ret = mavlink_set_flight_response(index, value);
#else
        extern int Set_UART_Response_Data(int index, int value);
	ret = Set_UART_Response_Data(index, value);
#endif

	if (!(resp_json = snx_json_obj_new()))
	{
		JSON_PRINT_QUEUE_ERR("Couldn't create JSON obj \n");
		return NULL;
	}

	if( snx_json_obj_add_str(resp_json, "type", "setflightresponse_res") != 0)
	{
		JSON_PRINT_QUEUE_ERR("Couldn't add JSON string obj \n");
		goto release_json;
	}

	if ( snx_json_obj_add_int(resp_json, "status", DAEMON_ERRNO(CMD_SETFLIGHTRESPONSE, ret)) != 0)
	{
		JSON_PRINT_QUEUE_ERR("Couldn't add JSON integer obj \n");
		goto release_json;
	}

	resp_buf = buffer_new();
	if(!resp_buf)
	{
		JSON_PRINT_QUEUE_ERR("resp buf allocate fail\n");
		goto release_json;
	}		
	output = snx_json_object_to_json_string(resp_json);
	buffer_add(resp_buf, output,  strlen((char*)output));

release_json:
	snx_json_obj_free(resp_json);
end:	
	return resp_buf;
}

static buffer_t* js_disable_sd_save(snx_json_t *parse_json, void* param)
{
	int value = 0;
	snx_json_t *resp_json;
	buffer_t * resp_buf = NULL;
	char *output;

	if (snx_json_obj_get_int(parse_json, "value", &value))
	{
		JSON_PRINT_QUEUE_ERR("Couldn't get JSON int obj \n");
		goto end;
	}
	
	extern int disable_sd_save;
	disable_sd_save = value;

	if (!(resp_json = snx_json_obj_new()))
	{
		JSON_PRINT_QUEUE_ERR("Couldn't create JSON obj \n");
		return NULL;
	}

	if( snx_json_obj_add_str(resp_json, "type", "js_disable_sd_save_res") != 0)
	{
		JSON_PRINT_QUEUE_ERR("Couldn't add JSON string obj \n");
		goto release_json;
	}

	if ( snx_json_obj_add_int(resp_json, "status", 0) != 0)
	{
		JSON_PRINT_QUEUE_ERR("Couldn't add JSON integer obj \n");
		goto release_json;
	}

	resp_buf = buffer_new();
	if(!resp_buf)
	{
		JSON_PRINT_QUEUE_ERR("resp buf allocate fail\n");
		goto release_json;
	}		
	output = snx_json_object_to_json_string(resp_json);
	buffer_add(resp_buf, output,  strlen((char*)output));

release_json:
	snx_json_obj_free(resp_json);
end:	
	return resp_buf;
}

static buffer_t* js_get_flight_data(snx_json_t *parse_json, void* param)
{
    int val, i;
	snx_json_t *resp_json, *jobj_arr_int, *gps_array_int, *nav_array_int; 
	buffer_t * resp_buf = NULL;
	char *output;

	if (!(resp_json = snx_json_obj_new()))
	{
		JSON_PRINT_QUEUE_ERR("Couldn't create JSON obj \n");
		return NULL;
	}

	if( snx_json_obj_add_str(resp_json, "type", "getflightdata_res") != 0)
	{
		JSON_PRINT_QUEUE_ERR("Couldn't add JSON string obj \n");
		goto release_json;
	}

	if ((snx_json_obj_add_array(resp_json, "data", &jobj_arr_int)) != 0)
	{
		goto release_json;
	}

#if CONFIG_MAVLINK_WIFI
        uint8_t uart_data[12];
        telem_get_uart_data(uart_data);
	for (i = 0 ; i < sizeof(uart_data); i++)
	{
            uint8_t val = uart_data[i];
            if (snx_json_obj_add_array_int(jobj_arr_int, val) != 0)
		{
			print_msg("Couldn't add JSON int obj \n");
			snx_json_obj_free(jobj_arr_int);
			goto release_json;
		}
	}
#else
	unsigned dsize = Get_Receive_UART_Size();
	for (i = 0 ; i < dsize; i++)
	{
		val = Get_Receive_UART_Data(i);
#ifdef UPGRADE_HACK
		if(0 != fb_version)
		{
			if(i == dsize - 3)
			{
				JSON_PRINT(SYS_INFO, "Reporting a version of lo: %u\n", fb_version & 0xff);

				val = fb_version & 0xff;
			}
			else if(i == dsize - 2)
			{
				JSON_PRINT(SYS_INFO, "Reporting a version of hi: %u\n", (fb_version >> 8) & 0xff);

				val = (fb_version >> 8) & 0xff;
			}
		}
#endif
		if (snx_json_obj_add_array_int(jobj_arr_int, val) != 0)
		{
			print_msg("Couldn't add JSON int obj \n");
			snx_json_obj_free(jobj_arr_int);
			goto release_json;
		}
	}
#endif

	mcu_cmd_counter_t *counter = get_mcu_conter();
	if ((snx_json_obj_add_int(resp_json, "photo_count", counter->cnt_snapshot)) != 0)
	{
		goto release_json;
	}
	if ((snx_json_obj_add_int(resp_json, "video_count", counter->cnt_recstart + counter->cnt_recstop)) != 0)
	{
		goto release_json;
	}
	
	// GPS from flight board.
#if CONFIG_MAVLINK_WIFI
        uint8_t gps_data[10];
        telem_get_gps_data(gps_data);
#else
	uint8_t *gps_data = app_getfbgps();
#endif
	if ((snx_json_obj_add_array(resp_json, "gps", &gps_array_int)) != 0)
	{
		goto release_json;
	}
	for (i = 0 ; i < 10; i++)
	{
		val = gps_data[i];
		if (snx_json_obj_add_array_int(gps_array_int, val) != 0)
		{
			print_msg("Couldn't add JSON int obj \n");
			snx_json_obj_free(gps_array_int);
			goto release_json;
		}
	}

	// Navigation state from flight board.
#if CONFIG_MAVLINK_WIFI
        uint8_t nav_data[4];
        telem_get_nav_data(nav_data);
#else
	uint8_t *nav_data = app_getfbnavista();
#endif
	if ((snx_json_obj_add_array(resp_json, "nav", &nav_array_int)) != 0)
	{
		goto release_json;
	}
	for (i = 0 ; i < 4; i++)
	{
		val = nav_data[i];
		if (snx_json_obj_add_array_int(nav_array_int, val) != 0)
		{
			print_msg("Couldn't add JSON int obj \n");
			snx_json_obj_free(nav_array_int);
			goto release_json;
		}
	}
	
	resp_buf = buffer_new();
	if(!resp_buf)
	{
		JSON_PRINT_QUEUE_ERR("resp buf allocate fail\n");
		goto release_json;
	}		
	output = snx_json_object_to_json_string(resp_json);
	buffer_add(resp_buf, output,  strlen((char*)output));

release_json:
	snx_json_obj_free(resp_json);
	return resp_buf;
}

static buffer_t* js_get_wifistatus(snx_json_t *parse_json, void* param)
{
	buffer_t * resp_buf = NULL;
	snx_json_t *resp_json;
	char *output;
	unsigned char mac_address_client[6];
	unsigned char strength = 0;

#if 0
	unsigned char mac_address_host[6];
	unsigned char debug_string[256];
#endif

	if (!(resp_json = snx_json_obj_new()))
	{
		JSON_PRINT_QUEUE_ERR("Couldn't create JSON obj \n");
		return NULL;
	}

	if ( snx_json_obj_add_str(resp_json, "type", "getwifistatus_res") != 0)
	{
		JSON_PRINT_QUEUE_ERR("Couldn't add JSON string obj \n");
		goto release_json;
	}

	dhcps_get_client_mac(mac_address_client);

#if 0
	memcpy(mac_address_host, wlan_get_get_mac_addr(), 6);

	sprintf(debug_string, "HOST: %02x:%02x:%02x:%02x:%02x:%02x CLIENT: %02x:%02x:%02x:%02x:%02x:%02x",
		mac_address_host[0], mac_address_host[1], mac_address_host[2], mac_address_host[3], mac_address_host[4], mac_address_host[5],
		mac_address_client[0], mac_address_client[1], mac_address_client[2], mac_address_client[3], mac_address_client[4], mac_address_client[5]);

	if ( snx_json_obj_add_str(resp_json, "debug", debug_string) != 0)
	{
		JSON_PRINT_QUEUE_ERR("Couldn't add JSON string obj \n");
		goto release_json;
	}
#endif

	WiFi_QueryAndSet(QID_GET_RX_STREGTH, mac_address_client, 0);
	strength = mac_address_client[0];

	if ( snx_json_obj_add_int(resp_json, "strength", strength) != 0)
	{
		JSON_PRINT_QUEUE_ERR("Couldn't add JSON integer obj \n");
		goto release_json;
	}

	resp_buf = buffer_new();
	if(!resp_buf)
	{
		JSON_PRINT_QUEUE_ERR("resp buf allocate fail\n");
		goto release_json;
	}		
	output = snx_json_object_to_json_string(resp_json);
	buffer_add(resp_buf, output,  strlen((char*)output));

release_json:
	snx_json_obj_free(resp_json);
	return resp_buf;
}

static buffer_t* js_get_flight_fwversion(snx_json_t *parse_json, void* param)
{
	buffer_t *resp_buf = NULL;
	snx_json_t *resp_json = NULL;
	char *output;
	unsigned short version;

	if(!(resp_json = snx_json_obj_new()))
	{
		JSON_PRINT_QUEUE_ERR("Couldn't create JSON obj \n");
		return NULL;
	}

	if(snx_json_obj_add_str(resp_json, "type", "getflightfwversion_res") != 0)
	{
		JSON_PRINT_QUEUE_ERR("Couldn't add JSON string obj \n");
		goto release_json;
	}

	version = mcu_get_flight_fwversion();
	
	if(snx_json_obj_add_int(resp_json, "flightfwversion", version) != 0)
	{
		JSON_PRINT_QUEUE_ERR("Couldn't add JSON integer obj \n");
		goto release_json;
	}

	resp_buf = buffer_new();
	if(!resp_buf)
	{
		JSON_PRINT_QUEUE_ERR("resp buf allocate fail\n");
		goto release_json;
	}		

	output = snx_json_object_to_json_string(resp_json);
	buffer_add(resp_buf, output, strlen(output));

release_json:
	snx_json_obj_free(resp_json);
	return resp_buf;
}

#ifdef CONFIG_APP_STREAMING
#if CONFIG_APP_STREAMING
extern void send_preview_to_rtp(unsigned char IFrame, unsigned char *pFrame,
                         unsigned int uiFrameSize, struct timeval tval);
extern video_info_t ispch0_info, ispch1_info;
#endif
#endif

static buffer_t* js_set_factory_mode(snx_json_t *parse_json, void* param)
{
	buffer_t *resp_buf = NULL;
	snx_json_t *resp_json = NULL;
	char *output;
	int ret = 0;
	int mode;

	if(!(resp_json = snx_json_obj_new()))
	{
		JSON_PRINT_QUEUE_ERR("Couldn't create JSON obj \n");
		return NULL;
	}

#ifdef CONFIG_APP_STREAMING
#if CONFIG_APP_STREAMING
	if ((ret = snx_json_obj_get_int(parse_json, "mode", &mode))) {
		JSON_PRINT(SYS_ERR, "Couldn't get JSON int obj \n");
		ret = GET_OBJECT_FAIL;
		goto response_json;
	}

    JSON_PRINT(SYS_INFO, "Factory mode %sabled...\n", mode? "en": "dis");
    if(0 != mode)
    {
        ispch0_info.h264_notice_dup1 = send_preview_to_rtp;
        ispch1_info.h264_notice = send_preview_to_rtp;
    }
#endif
#endif

	if(snx_json_obj_add_str(resp_json, "type", "setfactorymode_res") != 0)
	{
		JSON_PRINT_QUEUE_ERR("Couldn't add JSON string obj \n");
		goto release_json;
	}

	resp_buf = buffer_new();
	if(!resp_buf)
	{
		JSON_PRINT_QUEUE_ERR("resp buf allocate fail\n");
		goto release_json;
	}		

response_json:
	if (snx_json_obj_add_int(resp_json, "resp_status", DAEMON_ERRNO(CMD_SETFACTORYMODE, ret)) != 0) {
		JSON_PRINT(SYS_ERR, "Couldn't add JSON integer obj \n");
		goto release_json;
	}

	output = snx_json_object_to_json_string(resp_json);
	buffer_add(g_resp_buf, output,  strlen((char *)output));

release_json:
	snx_json_obj_free(resp_json);
	return resp_buf;
}
