#include "FreeRTOS.h"
#include <stddef.h>
#include <bsp.h>
#include <nonstdlib.h>
#include <string.h>
#include <usb_device/usbd_uvc.h>
#include <libmid_usbd/mid_usbd.h>
#include <libmid_nvram/snx_mid_nvram.h>
#include "usbd_ext.h"
#include "usbd_uvc_param.h"
#include "usbd_msc_param.h"
#include "main_flow.h"
#include <wdt/wdt.h>
#include "watch_task.h"
#include <libmid_json/snx_json.h>
#include "video_main.h"
#include "json_cmd.h"
#include "tunninguse.h"
#include "snx_json_usbd.h"
#include "../main_flow/main_flow.h"
#include "snx_xu_usr.h"
#include <usbh/USBH.h>
#include <usbh/UVC.h>
#include <libmid_vc/avcutils.h>

#define PREVIEW_DELAY_TIME	(50)
#define LOOP_TIMEOUT_CNT	(2000/PREVIEW_DELAY_TIME)

static usb_device_desc_info_t usbd_desc_info = {
	.usbd_msc_info = &msc_desc_info,
	.usbd_uvc_info = &uvc_desc_info,
};

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


void usbd_ext_init(void)
{
	USBD_EXT_PRINT(SYS_INFO, "USBD_app init\n");
	usbd_mid_init(&usbd_desc_info);
}

void usbd_ext_uninit(void)
{
	USBD_EXT_PRINT(SYS_INFO, "USBD_app uninit\n");
	usbd_mid_uninit();
}
