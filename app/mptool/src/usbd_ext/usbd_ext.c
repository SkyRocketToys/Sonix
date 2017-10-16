#include "FreeRTOS.h"
#include <stddef.h>
#include <bsp.h>
#include <nonstdlib.h>
#include <string.h>
#include <libmid_usbd/mid_usbd.h>
#include "usbd_ext.h"
#include "usbd_uvc_param.h"
#include "usbd_msc_param.h"
#include "snx_xu_mpt.h"
#include "usbd_uvc_vc.h"

static usb_device_desc_info_t usbd_desc_info = {
	.usbd_msc_info = &msc_desc_info,
	.usbd_uvc_info = &uvc_desc_info,
};

static int usbd_class_mode = USBD_MODE_UNKNOWN;
void usbd_ext_uninit(void);

void usbd_ext_class_switch(int32_t cs_mode, uint32_t options)
{
	if ( (usbd_class_mode == USBD_MODE_UVC) || (usbd_class_mode == USBD_MODE_UNKNOWN) ){
		snx_usbd_uvc_stop_preview();
	}

	if (cs_mode != usbd_class_mode) {
		usbd_class_mode = cs_mode;
	}

}

void snx_mptool_init_task(void *pvParameters)
{
	int key_res_width, key_res_height, key_fps;

	// chkuo add, waitting for isp ready. Temporary use snx_isp_sensor_mode_get method.
	snx_isp_sensor_mode_get(&key_res_width, &key_res_height, &key_fps);

	usbd_mid_uvc_sys_xu_mpt_reg_cb(usbd_uvc_events_sys_xu_mpt);
	snx_usbd_uvc_vc_init();
	usbd_event_class_switch_prev_notice_reg_cb(usbd_ext_class_switch);
	usbd_mid_msc_ready_to_upgrade_cb(usbd_ext_uninit); //for bypass reboot

	usbd_mid_init(&usbd_desc_info);
	initUsbdUvcXuMpt();

	vTaskDelete(NULL);
}


void usbd_ext_init(void)
{
	USBD_EXT_PRINT("=== MP Tool init ===\n");
	// init MPTool task
	if (xTaskCreate(snx_mptool_init_task, (const char *)"MPTOOL_INIT_TASK", STACK_SIZE_1K, NULL, PRIORITY_TASK_DRV_USBD_CLASS, NULL) != pdPASS) {
		USBD_EXT_PRINT("Create MPTool init task fail\n");
	}
}

void usbd_ext_uninit(void)
{
	USBD_EXT_PRINT("=== MP Tool uninit ===\n");
	uninitUsbdUvcXuMpt();
	usbd_mid_uninit();
	usbd_drv_uvc_set_desc_info(NULL);
	usbd_mid_uvc_sys_xu_mpt_reg_cb(NULL);
	vTaskDelay(500/portTICK_PERIOD_MS);//for bypass reboot
}
