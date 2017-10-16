#include "FreeRTOS.h"
#include <stddef.h>
#include <bsp.h>
#include <nonstdlib.h>
#include <string.h>
#include <libmid_usbd/mid_usbd.h>
#include <libmid_nvram/snx_mid_nvram.h>
#include "usbd_ext.h"
#include "usbd_msc_param.h"
#include "main_flow.h"
#include <wdt/wdt.h>
#include "../main_flow/main_flow.h"


#define PREVIEW_DELAY_TIME	(50)
#define LOOP_TIMEOUT_CNT	(2000/PREVIEW_DELAY_TIME)


static usb_device_desc_info_t usbd_desc_info = {
	.usbd_msc_info = &msc_desc_info,
};


static unsigned char usbd_plugin_flag = 0;
static int usbd_class_mode = USBD_MODE_UNKNOWN;
static unsigned int usbd_tool_options = 0;
static unsigned char usbd_isp_tuning_mode = 0;

uint8_t usbd_app_IsPlugin(void)
{
	return usbd_plugin_flag;
}

void usbd_app_task_uinit(void)
{
 	all_task_uinit(TASK_KEEP_USBD);	
}

#if 1
void snx_reboot(void)
{
	WDT_INIT_t set;

	set.reload = 1;
	set.clk_src = 1;
	set.reset = 1;
	set.intr = 0;

	wdt_init(set);
	wdt_reset_enable(1);
	wdt_enable();
}
#endif

void usbd_ext_class_switch(int32_t cs_mode, uint32_t options)
{
	usbd_tool_options = options;

	switch (cs_mode) {
		
	case USBD_MODE_MSC:
		usbd_isp_tuning_mode = 0;
		break;

	case USBD_MODE_HID:
		break;

	default:
		break;
	}

	if (cs_mode != usbd_class_mode) {
		usbd_class_mode = cs_mode;
	}

}

void usbd_ext_plugin_to_usbh(void)
{
	int cs_mode = usbd_get_class_mode();
	usbd_ext_class_switch(cs_mode, 0);
	usbd_plugin_flag = 1;
}

void usbd_ext_plugout_from_usbh(void)
{
	usbd_plugin_flag = 0;
}

void usbd_ext_init(void)
{
	USBD_EXT_PRINT("USBD_app init\n");

	usbd_event_remove_maintask_reg_cb(usbd_ext_plugin_to_usbh);
	usbd_event_restore_maintask_reg_cb(usbd_ext_plugout_from_usbh);
	usbd_event_class_switch_prev_notice_reg_cb(usbd_ext_class_switch);

	usbd_mid_msc_ready_to_upgrade_cb(usbd_app_task_uinit);

	usbd_mid_init(&usbd_desc_info);

	usbd_plugin_flag = 0;
	usbd_isp_tuning_mode = 0;
}

void usbd_ext_uninit(void)
{
	USBD_EXT_PRINT("USBD_app uninit\n");

	usbd_mid_uninit();

	usbd_event_class_switch_prev_notice_reg_cb(NULL);
	usbd_event_remove_maintask_reg_cb(NULL);
	usbd_event_restore_maintask_reg_cb(NULL);

	usbd_plugin_flag = 0;
	usbd_isp_tuning_mode = 0;
}

