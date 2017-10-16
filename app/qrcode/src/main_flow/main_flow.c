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
#include <isp/isp.h>
#include <vc/snx_vc.h>
#include <libmid_isp/snx_mid_isp.h>
#include <libmid_vc/snx_mid_vc.h>
#include <libmid_automount/automount.h>
#include <usb_device/usbd_uvc.h>
#include <libmid_usbd/mid_usbd.h>
#include <stdio.h>
#include <string.h>
#include "audio/audio_main.h"
#include "main_flow.h"
//#include <wifi/WiFi.h>
#include <generated/snx_sdk_conf.h>
#include <usbh/USBH.h>
#include <gpio/gpio.h>
#include "sd_fw_upgrade.h"
#include <isp/isp.h>
#include <zbar.h>
#include "../usbd_ext/usbd_ext.h"


//static usb_device_desc_info_t usbd_desc_info = {};


SemaphoreHandle_t UsbfwupdateMutex;
//static int usbupdateflag = 0;

void task_main_flow( void *pvParameters )
{
	print_msg_queue("===== main flow start =====\n");
//	automount_init();
	usbd_ext_init();
	//usbd_mid_init(&usbd_desc_info);
	
	UsbfwupdateMutex = xSemaphoreCreateBinary();

	print_msg_queue(" HID initial \n");
//	usbd_mid_set_class_mode(2, 0);

	if (pdPASS != xTaskCreate(task_qrscan_flow, "task_qrscan_flow", STACK_SIZE_4K, NULL,PRIORITY_TASK_APP_VIDEO_GET, NULL))
		print_msg_queue("Could not create TaskMainFlow\r\n");
	
	print_msg_queue("===== main flow end =====\n");

	vTaskDelete(NULL);
}
void main_flow_init(void)
{
	if (pdPASS != xTaskCreate(task_main_flow, "task_main_flow", STACK_SIZE_4K, NULL,10, NULL))
		print_msg_queue("Could not create TaskMainFlow\r\n");

}
static int usbupdatefwflag = 0;
void task_qrscan_flow( void *pvParameters )
{

	int ret;
	int i, width, height, rate;
	static int counts;
	static int isp_buffer_num;
	struct snx_frame_ctx ctx[2];

	int   yuv_out_size;
	unsigned char *yuv_out;
//	uint8_t buf[255];
	zbar_image_scanner_t *scanner = NULL;
	int datalen = 0;

	int md_enable=1, md_threshold=300, status;
//	unsigned int report[6] = {0};
	unsigned int mask[6] = {0};

	print_msg_queue("start QR Scan \r\n");
	vTaskDelay(100/portTICK_PERIOD_MS);

	print_msg("\n\n");

	width = 320;
	height = 240;
	rate = 30;

	print_msg("Resolution : %d x %d \n",width,height);
	print_msg("Frame Rate : %d fps \n",rate);
		
	// zbar init
	scanner = zbar_image_scanner_create();
	zbar_image_scanner_set_config(scanner, 0, ZBAR_CFG_ENABLE, 1);
	yuv_out_size=((width)*(height)*3)/2;
	print_msg("yuv_out_size=%d=0x%x\n",yuv_out_size,yuv_out_size);
	yuv_out = (unsigned char *)pvPortMalloc(yuv_out_size, GFP_KERNEL, MODULE_MID_VIDEO);
	print_msg("yuv_out=0x%08x\n",yuv_out);

	if(width == 0 || height == 0 || rate == 0){
		print_msg("Video args error!!\n");
		print_msg("width=%d\n",width);
		print_msg("height=%d\n",height);
		print_msg("rate=%d\n",rate);
		goto __error;
	}
		
	if((ret = snx_isp_open(0, width, height,rate , VIDEO_PIX_FMT_SNX420)) == pdFAIL){
		print_msg("%s:%d:open video device error!\n",__func__,__LINE__);
		goto __error;
	}

	isp_buffer_num = 2;
	if((ret = snx_isp_reqbufs(0, &isp_buffer_num)) == pdFAIL){
		print_msg("request buffers error!\n");
		goto __close_video_device;
	}

	for(i = 0; i < isp_buffer_num; i++){
		ctx[i].index = i;
		if((ret = snx_isp_querybuf(0, &ctx[i])) == pdFAIL){
			print_msg("query buffers error!\n");
			goto __close_video_device;
		}
	}

	for(i = 0; i < isp_buffer_num; i++){
		ctx[i].index = i;
		if((ret =snx_isp_qbuf(0, &ctx[i])) == pdFAIL){
			print_msg("enqueue buffer error!\n");
			goto __close_video_device;
		}
	}

	if((ret = snx_isp_streamon(0)) == pdFAIL){
		print_msg("video stream on error!\n");
		goto __close_video_device;
	}

	snx_isp_print_drop_frame(0);

	/* wrap image data */
	zbar_image_t *image = zbar_image_create();
	zbar_image_set_format(image, *(int*)"Y800");
	zbar_image_set_size(image, width, height);
	zbar_image_set_data(image,yuv_out,yuv_out_size, zbar_image_free_data);

	print_msg("QR Code Scan start decoding!\n");


	snx_isp_md_threshold_set(md_threshold);//0x0~0xffff
	snx_isp_md_int_threshold_set(1);
	snx_isp_md_block_mask_set(mask);
	snx_isp_md_int_timeout_set(1000);
	snx_isp_md_enable_set(md_enable);

	counts = 0;

	while(1) {	
		struct snx_frame_ctx vb;

//		vTaskDelay(30/portTICK_PERIOD_MS);
		if(usbupdatefwflag == 1)
		{
			usbupdatefwflag = 0;
			print_msg("start update fw,break out qr scan flow \n");
			break;
		}
		if((ret = snx_isp_dqbuf(0, &vb)) == pdFAIL)
			goto __close_video_stream;

		if(status == 1) {
			// Motion detect start QR scan

			// YUV converter
			snx_420line_to_420((char *)vb.userptr, (char *)yuv_out, width, height );

//			print_msg_queue("<<test>><%s><%d> %d %d %d cnt=%d\n",__func__, __LINE__
//							, md_enable, md_threshold, status, counts);
#if 1 // Scan QR code
			zbar_scan_image(scanner, image);//cost much time if the picture's size is very big

			// extract results
			const zbar_symbol_t *symbol = zbar_image_first_symbol(image);
			
			for(; symbol; symbol = zbar_symbol_next(symbol)) {
				/* do something useful with results */
				zbar_symbol_type_t typ = zbar_symbol_get_type(symbol);
				const char *data = zbar_symbol_get_data(symbol);
				datalen = zbar_symbol_get_data_length(symbol);
				print_msg("decoded %s symbol \"%s\"\n",zbar_get_symbol_name(typ), data);
//				strcpy((char*)buf, (char*)data);
//				sprintf((char*)buf, "%s\n", (char*)data);
//				print_msg("%d len \n",datalen);
//				usbd_mid_hid_send_string(buf,datalen);
				usbd_mid_hid_send_string((unsigned char*)data,datalen);
				status = 0;
			}
#endif	
			counts++;
			if(counts > (rate*1)) {
				status = 0;
				counts =0;
			}
		} //if(status == 1)
		else { // status == 0
			if(counts > (rate*1)) {
				//scan
//			print_msg_queue("<<test>><%s><%d> %d %d %d cnt=%d\n",__func__, __LINE__
//							, md_enable, md_threshold, status, counts);

				snx_isp_md_int_get(&status); /* interrupt. if 0 timeout, else have motion */	
				if(status == 1)
					counts = 0;
			}
			counts++;
		}

		if((ret =snx_isp_qbuf(0, &vb)) == pdFAIL)
			goto __close_video_device;
	} //while(1)

		
	// clean up 
	zbar_image_destroy(image);
	zbar_image_scanner_destroy(scanner);

	

	xSemaphoreGive(UsbfwupdateMutex);
		
__close_video_stream:
	snx_isp_streamoff(0);
__close_video_device:
	snx_isp_close(0);
	snx_isp_print_drop_frame(1);
__error:
	print_msg("error  \n");

	vTaskDelete(NULL);	
}

void all_task_uinit(unsigned int task_keep)
{
	print_msg_queue("[main_flow]all_task_uinit\n");

	if ((task_keep & TASK_KEEP_USBD) == 0) {
		usbd_ext_uninit();
	}
	usbupdatefwflag = 1;
	xSemaphoreTake(UsbfwupdateMutex,portMAX_DELAY);

	vTaskDelay(300/portTICK_PERIOD_MS);
	return;
}




