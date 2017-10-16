/*********************************************************************************
* /Cstreamer_main.c
*
* Implementation of schedule for recording internal APIs
*
* History:
*    2015/08/11 - [yiling] created file
*
*
* Copyright (C) 1996-2015, Sonix, Inc.
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
#include <timers.h>
#include <semphr.h>
#include <bsp.h>
#include <nonstdlib.h>
#include <libmid_nvram/snx_mid_nvram.h>
#include <libmid_automount/automount.h>
#include <stdio.h>
#include <string.h>
#include "audio/audio_main.h"
#include "video/video_main.h"
#include "main_flow.h"
#include "rec_schedule.h"
#include "socket_ctrl.h"
#include "../playback/play_back.h"
#include "timestamp_osd/timestamp_osd.h"
#include <libmid_rtsp_server/rtsp_server.h>
#include "../usbd_ext/usbd_ext.h"
#include <wifi/wifi_api.h>
#include <generated/snx_sdk_conf.h>
#include <usbh/USBH.h>
#include <gpio/gpio.h>
#include "watch_task.h"
#include "sd_fw_upgrade.h"
#include "../test/lpwr_dashcam_example.h"
#include <../sensor_cap/sensor_capability.h>
#if CONFIG_UDP_FLOW
#include "udp_cmd.h"
#endif
#include "../mcu_v2/mcu.h"
#include "rec_common.h"
#include "utility.h"
#include "audio_pushtalk.h"


#define MAIN_PRINT(level, fmt, args...) print_q(level, "[main_flow]%s: "fmt,##args)

#if CONFIG_EDCCA_FLOW
#include <wifi/wifi_api.h>

// void edcca_init()
// {
// //====EDCCA TEST

// //Check period ..in unit of ms
//     unsigned long val=30;
//     WiFi_QueryAndSet(SET_EDCCA_PERIOD, &val, 0);

// //threshold of turning off¡K. In unit of percentage 
//     val=90;
//     WiFi_QueryAndSet(SET_EDCCA_ED_TH, &val, 0);

// //To prevent bouncing¡K.. unit of times
//     val=1;
//     WiFi_QueryAndSet(SET_EDCCA_BLOCK_CHECK_TH, &val, 0);

// //EDCCA On and off¡K 1=on, 0=off.
//     val=1;
//     WiFi_QueryAndSet(SET_EDCCA_ONOFF, &val, 0);
// }
#endif


bool bWifiInsert = FALSE;

/* Integrating wifi plug in/out cb operation */
// void wifi_plugin_cb(void)
// {
// 	MAIN_PRINT(SYS_INFO, "============ Plug in Wi-Fi device ============\n");
// 	bWifiInsert = TRUE;
// 	WiFi_Task_Init(NULL, WIFI_RUN_MODE_AP);
// }

// void wifi_plugout_cb(void)
// {
// 	MAIN_PRINT(SYS_INFO, "============ Plug out Wi-Fi device ============\n");
// 	bWifiInsert = FALSE;
// 	WiFi_Task_UnInit();
// }

void task_main_flow( void *pvParameters )
{
	print_msg_queue("===== main flow start =====\n");

#if (CONFIG_RESCUE_USBDEV_UPGRADE == 1)	
//	usbd_mid_uvc_set_max_payload_size(CONFIG_MODULE_USB_DEVICE_UVC_PAYLOAD_SIZE);
	usbd_ext_init();	/* USB Device init */
#endif

#if (CONFIG_RESCUE_SD_UPGRADE == 1)	
	automount_init();
	sd_fwupgrade_init();
#endif


#if 0	
	int mp_mode_enable;
	if (snx_nvram_integer_get(NVRAM_MP_MODE_PKG_NAME, NVRAM_MP_MODE_CFG_ENABLE_NAME, &mp_mode_enable) != NVRAM_SUCCESS) {
		snx_nvram_integer_set(NVRAM_MP_MODE_PKG_NAME, NVRAM_MP_MODE_CFG_ENABLE_NAME, NVRAM_MP_MODE_DISABLE);
	} else {
		if (mp_mode_enable != NVRAM_MP_MODE_DISABLE)
			snx_nvram_integer_set(NVRAM_MP_MODE_PKG_NAME, NVRAM_MP_MODE_CFG_ENABLE_NAME, NVRAM_MP_MODE_DISABLE);
	}
	automount_init();
	mf_video_resmap_init();
	rec_filemanage_init();
#if 1
	if (app_util_init() != pdPASS)
		MAIN_PRINT(SYS_ERR, "app utility init fail\n");
	if (mf_video_init() != pdPASS)
		MAIN_PRINT(SYS_ERR, "video init fail\n");

#ifdef CONFIG_APP_DRONE
#if RTSP_PREVIEW_AUDIO
	if (mf_audio_init() != pdPASS)
		MAIN_PRINT(SYS_ERR, "audio init fail\r\n");	
#endif
#else
	if (timestamp_osd_init() != pdPASS)
		MAIN_PRINT(SYS_ERR, "timestamp osd init fail\r\n");

	if (mf_audio_init() != pdPASS)
		MAIN_PRINT(SYS_ERR, "audio init fail\r\n");
	
//	if (mf_audio_out_init() != pdPASS)
//		MAIN_PRINT(SYS_ERR, "audio out init fail\r\n");
#endif
	init_record_task();
	pb_init_array();
#ifndef CONFIG_APP_DRONE
	reclapse_init_task();
	init_protect_task();
#endif /* CONFIG_APP_DRONE */
#else   //HD60
	if (mf_videohd60_init() != pdPASS)
		MAIN_PRINT(SYS_ERR, "video init fail\r\n");
#ifndef CONFIG_APP_DRONE
	if (timestamp_osd_init() != pdPASS)
		MAIN_PRINT(SYS_ERR, "timestamp osd init fail\r\n");
#endif /* CONFIG_APP_DRONE */
#endif
	socket_init();
	socket_server_create(8080);
#if AUDIO_PUSH_TALK
	pt_audio_init(8828);
#endif
#if CONFIG_UDP_FLOW
	udp_cmd_socket_create();
#endif
	sd_fwupgrade_init();
//	lpwr_dashcam_example_init();
	check_task_restart_process();
	check_task_close_process();

	usbd_mid_uvc_set_max_payload_size(CONFIG_MODULE_USB_DEVICE_UVC_PAYLOAD_SIZE);
	usbd_ext_init();	/* USB Device init */

#if CONFIG_EDCCA_FLOW
	while(1){
		int res = 0;
		res = WIFI_AP_Initial_Done();
		if(res == 1)
			break;
		
		vTaskDelay(300/portTICK_PERIOD_MS);
	}
    edcca_init();
#endif
#endif

	print_msg_queue("===== main flow end =====\n");

	vTaskDelete(NULL);
}

void main_flow_init(void)
{
#ifndef CONFIG_USBH_STORAGE_SUPPORT
	/* Register Wifi plug in/out cb */
	// usbh_plug_cb_reg(USBH_WIFI_CLASS, wifi_plugin_cb, wifi_plugout_cb);
#endif

	if (pdPASS != xTaskCreate(task_main_flow, "task_main_flow", STACK_SIZE_4K, NULL,
	                          PRIORITY_TASK_APP_MAINFLOW, NULL))
		MAIN_PRINT(SYS_ERR, "Could not create TaskMainFlow\r\n");
}


#if 0
void all_task_uinit(unsigned int task_keep)
{
	MAIN_PRINT(SYS_INFO, "all_task_uinit\n");

	if ((task_keep & TASK_KEEP_USBD) == 0) {
		usbd_ext_uninit();
	}

	disable_diskio();

#ifndef CONFIG_APP_DRONE
	//uart2_disable_rx_interrupt();
	app_mcu_cmds_register_empty();
#endif
#if CONFIG_UDP_FLOW
    udp_cmd_socket_uninit();
#endif
	socket_uninit();
#if 1
#ifndef CONFIG_APP_DRONE
	//close Protect
	set_protect_to_closing(1);
	if (get_sd_umount_err() == 1)
		unlock_protect_mutex();
	wait_protect_task_closed();
	//close TimeLapseRecord
	set_lapse_to_closing(1);
	if (get_sd_umount_err() == 1)
		unlock_timelapse_mutex();
	wait_lapse_task_closed();
#endif
	//close Record
	set_record_to_closing(1);
	if (get_sd_umount_err() == 1)
		unlock_record_mutex();
	wait_record_task_closed();

#ifdef CONFIG_APP_DRONE
#if RTSP_PREVIEW_AUDIO
	mf_audio_uninit();
#endif
#else
	mf_audio_uninit();
//	mf_audio_out_unint();
	timestamp_osd_uninit();
#endif

#else //HD60
#ifndef CONFIG_APP_DRONE
	timestamp_osd_uninit();
#endif
#endif
	//close Isp
	set_isp0_to_closing(1);
	wait_isp0_task_closed();
	set_isp1_to_closing(1);
	wait_isp1_task_closed();

	destroy_rtsp_server();
	app_util_uninit();
	rec_filemanage_uninit();

#ifndef CONFIG_USBH_STORAGE_SUPPORT
#if (CONFIG_SYSTEM_PLATFORM_SN98670 == 1) || (CONFIG_SYSTEM_PLATFORM_SN98671 == 1) || (CONFIG_SYSTEM_PLATFORM_SN98672 == 1)
	// Turn off GPIO0 WiFi power and trigger usb host to do plug out.
	gpio_pin_info info;
	info.pinumber = 0;
	info.mode = 1;
	info.value = 0;
	snx_gpio_open();
	snx_gpio_write(info);
	snx_gpio_close();
	vTaskDelay(500 / portTICK_PERIOD_MS); //waiting for usbh detect plug out
#else
	WiFi_Task_UnInit();
#endif
#endif

#if (CONFIG_SYSTEM_PLATFORM_SN98660 == 1) && (!defined CONFIG_APP_DRONE)
		mcu_set_err_flag(ERR_WIFI);
		vTaskDelay( 500 / portTICK_RATE_MS );
		mcu_reset_err_flag();
#endif

	usbh_freertos_uninit();
	
	vTaskDelay(300 / portTICK_PERIOD_MS); //waiting for video audio uninit ok
	return;
}

#endif