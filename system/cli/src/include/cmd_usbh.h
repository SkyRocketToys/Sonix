#ifndef _CMD_USBH_H_
#define _CMD_USBH_H_

#ifdef CONFIG_CLI_CMD_USBH
#include <FreeRTOS.h>
#include <task.h>
#include <bsp.h>
#include <timers.h>
#include <nonstdlib.h>
#include <string.h>
#include <stdio.h>
#include <queue.h>
#include <semphr.h>

int cmd_uvc_get_info(int argc, char* argv[]);		
int cmd_uvc_start(int argc, char* argv[]);		
int cmd_uvc_stop(int argc, char* argv[]);		

#define CMD_TBL_USBH		CMD_TBL_ENTRY(		\
	"usbh",		4,      NULL,			\
	"+usbh		- USB Host command table",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_usbh_tbl,		cmd_main_tbl			\
),

#define CMD_TBL_UVC_GET_INFO	CMD_TBL_ENTRY(		\
	"uvc_get_info",	12,      cmd_uvc_get_info,	\
	"uvc_get_info	- UVC GET Stream Infomation ",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_UVC_START	CMD_TBL_ENTRY(		\
	"uvc_start",	9,      cmd_uvc_start,	\
	"uvc_start	- UVC START Stream ",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_UVC_STOP	CMD_TBL_ENTRY(		\
	"uvc_stop",	8,      	cmd_uvc_stop,	\
	"uvc_stop	- UVC STOP Stream ",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

extern void uvc_app_task(void * pvParameters);


typedef struct
{  

	uint32_t	StreamID;
	uint32_t 	SDReocrdEnable;
	uint32_t 	previewEnable;
	uint32_t 	DebugEnable;	
	uint32_t 	*ptr;
	uint32_t 	size;
	uint32_t 	width;
	uint32_t 	height;
	uint32_t 	fps;
	uint32_t 	framecnt;
	
}UVC_APP_STRUCTURE;
#endif

#endif /* _CMD_USBH_H_ */
