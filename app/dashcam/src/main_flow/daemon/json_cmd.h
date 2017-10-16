/**
 * @file
 * this is application header file for paring cmd, include this file before use
 * @author Algorithm Dept Sonix.
 */

#ifndef __MF_JSON_CMD_H__
#define __MF_JSON_CMD_H__
#include "socket_ctrl.h"
#include "json_cmd_list.h"

#define JSON_CMD_EXT_BASE 0x3F

enum {
#define X(str, var, num, fun) CMD_##num,
	CMD_LIST
	CMD_STD_END,
	CMD_EXT_BASE = JSON_CMD_EXT_BASE,
#ifdef CMD_LIST_EXT
	CMD_LIST_EXT
#endif
	CMD_EXT_END
#undef X
};

#define X(str, var, num, fun) extern const char *Json_##var##_CMD;
	CMD_LIST
#ifdef CMD_LIST_EXT
	CMD_LIST_EXT
#endif
#undef X

#define ADDR_COMPARATOR(e1, e2) (strcmp(e1->mac, e2->mac))
#define NVRAM_START_PREVIEW_TONE  "Start_the_preview.aac"
#define NVRAM_END_PREVIEW_TONE	"End_the_preview.aac"
#define LIMIT_PLAYBACK_AND_DOWNLOAD_CONNECTION  0  /**< Enable or Disable limit playback and download file connections*/

/* chkuo add, Json source control command */
#define Json_MainCtrl_SRC_CMD               "json_cmd_mctrl_src"

/**
* @brief enum for command source
*/
typedef enum json_command_source {
	JSON_CMD_SRC_ETH = 0,
	JSON_CMD_SRC_USBD,
} json_command_source_t;

enum FILELIST_TYPE {
	RECORD_LIST = 0,
	PROTECT_LIST,
	PICTURE_LIST,
	THUMBNAIL_LIST,
	TIMELAPSE_LIST,
};

typedef enum {
	CARD_OK = 0,
	CARD_NOT_EXIST = 1,
	CARD_FORMAT_ERROR = 2,
}SDSTATE;

#if LIMIT_PLAYBACK_AND_DOWNLOAD_CONNECTION
typedef struct _dwnload_pb_conn {
	int ip;
	char mac[20];
	int status;
} dwnload_pb_conn_t;
#endif

int json_init(void);
void json_uninit(void);
buffer_t *parse_json_cmd(buffer_t *pbuf, void *param);
void set_wifi_param(void);
void json_disconnect(char *rtsp_fname, char *mac, int type);
int json_get_format_status(void);
void json_cmd_mctrl_src_change(json_command_source_t cmd_src);
#if LIMIT_PLAYBACK_AND_DOWNLOAD_CONNECTION
void set_max_dwnlod_and_pb_max_conn_num(int num);
void del_dwnlod_and_pb_conn_num(void);
void delete_conn_from_pb_and_dwnlod(char *mac);
#endif

#endif
