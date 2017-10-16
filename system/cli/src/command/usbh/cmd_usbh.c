#include <FreeRTOS.h>
#include <task.h>
#include <bsp.h>
#include <timers.h>
#include <nonstdlib.h>
#include <string.h>
#include <stdio.h>
#include <queue.h>
#include <semphr.h>
#include <sys_clock.h>
#include <sys/time.h>
#include "rec_common.h"
#include "debug.h"
#include "rec_schedule.h"
#include "../video/video_main.h"
#include "../audio/audio_main.h"
//#include "rec_query.h"
#include <libmid_automount/automount.h>
//#include "../file_protection/file_protection.h"
#include <libmid_nvram/snx_mid_nvram.h>
#include "../daemon/json_cmd.h"

//#include "sdcard_sensorcapility.h"
#include "usbh/USBH.h"
#include "usbh/UVC.h"
//#include "watch_task.h"
//#include "user_cmd_bit.h"
//#include "upload1.h"
#include "avcutils.h"
#include "cmd_usbh.h"
#include "../sensor_cap/sensor_capability.h"


#ifdef CONFIG_CLI_CMD_USBH

UVC_APP_STRUCTURE	UVC_APP[5];

recordinfo_t UVC_SDRECORD_INFO[UVC_STREAMID_NUM_MAX];

int isWaitIFrame = 0;
int isIFrame = 0;
int currentFrameType = 0;

void uvc_sd_safe_free(void **pp)
{
  if((pp != NULL) && (*pp != NULL))
  {
     free(*pp);
	 *pp = NULL;
  }
}

static void uvc_sd_info_uninit(struct SDRecrodInfo** info)
{
	uvc_sd_safe_free(*info);
}


static recordinfo_t* uvc_sd_info_init(void)
{
	recordinfo_t *info = NULL;
	
	if(!(info = (recordinfo_t *)pvPortMalloc(sizeof(recordinfo_t), GFP_KERNEL, MODULE_APP)))
	{
       goto finally;
	}

	memset(info, 0, sizeof(recordinfo_t));

	info->pRecord_info = NULL;
	memset(&info->RecParam, 0, sizeof(RecParam_t));
	info->sched_p = NULL;
	info->readfilelistok = 0;
	info->recordusedsize = 0;
	info->recordstatus = 0;
	info->lastsecond = 0;
	info->recordclose = 0;
	info->sdcardisfull = 0;
	info->sdcardseed = 0;
	memset(info->schedpath, 0, sizeof(info->schedpath));
	info->gpscount = 0;
	info->gsensorcount = 0;
	info->fileformat = 0;
	info->rm_queue = NULL;
	
	return info;

finally:
    if(info != NULL)
		uvc_sd_info_uninit(&info);
	return NULL;
}


int cmd_uvc_get_info(int argc, char* argv[])
{
	USBH_UVC_INFO_Struct			*uvc_info;	
		
	uvc_info	=	(USBH_UVC_INFO_Struct*) uvc_get_info();
	uvc_print_info_data(uvc_info);		

	return 0;
}

int uvc_get_para(char* argv[],uint32_t* devid, uint32_t* fmt,uint32_t* res,uint32_t* fps){
	
	//assigment devid
	*devid = simple_strtoul(argv[1], NULL, 10);
	//parsing format 
	if((strcmp(argv[2], "H264") == 0) || (strcmp(argv[2], "h264") == 0)){
		*fmt = 1;
	}else if((strcmp(argv[2], "MJPEG") == 0) || (strcmp(argv[2], "mjpeg") == 0)){
		*fmt = 2;
	}else if((strcmp(argv[2], "YUV") == 0) || (strcmp(argv[2], "yuv") == 0)){
		*fmt = 3;
	}else{
		return pdFAIL;
	}
	
	//parsing res 	
	if((strcmp(argv[3], "1920") == 0) && (strcmp(argv[4], "1080") == 0)){	
		*res = 11;
	}else if((strcmp(argv[3], "1280") == 0) && (strcmp(argv[4], "800") == 0)){			
		*res = 1;	
	}else if((strcmp(argv[3], "1280") == 0) && (strcmp(argv[4], "720") == 0)){		
		*res = 2;
	}else if((strcmp(argv[3], "960") == 0) && (strcmp(argv[4], "540") == 0)){	
		*res = 3;		
	}else if((strcmp(argv[3], "848") == 0) && (strcmp(argv[4], "480") == 0)){	
		*res = 4;			
	}else if((strcmp(argv[3], "640") == 0) && (strcmp(argv[4], "480") == 0)){			
		*res = 5;	
	}else if((strcmp(argv[3], "640") == 0) && (strcmp(argv[4], "360") == 0)){			
		*res = 6;		
	}else if((strcmp(argv[3], "424") == 0) && (strcmp(argv[4], "240") == 0)){			
		*res = 7;			
	}else if((strcmp(argv[3], "320") == 0) && (strcmp(argv[4], "240") == 0)){			
		*res = 8;			
	}else if((strcmp(argv[3], "320") == 0) && (strcmp(argv[4], "180") == 0)){			
		*res = 9;			
	}else if((strcmp(argv[3], "160") == 0) && (strcmp(argv[4], "120") == 0)){			
		*res = 10;			
	}else{
		return pdFAIL;
	}
	
	//parsing fps		
	if(strcmp(argv[5], "5") == 0){
		*fps = 0x001E8480;	
	}else if(strcmp(argv[5], "10") == 0){
		*fps = 0x000F4240;			
	}else if(strcmp(argv[5], "15") == 0){
		*fps = 0x000A2C2A;	
	}else if(strcmp(argv[5], "20") == 0){
		*fps = 0x0007A120;	
	}else if(strcmp(argv[5], "25") == 0){
		*fps = 0x00061a80;
	}else if(strcmp(argv[5], "30") == 0){
		*fps = 0x00051615;	
	}else{
		return pdFAIL;
	}							
	return pdPASS;
}

void uvc_sd_set_rec_writebuffer(RecWirteBufferInitInfo_t* writebufferparam,video_param_t *pv_param)
{
	if((pv_param->width >= FHD_WIDTH) && (pv_param->height >= FHD_HEIGHT))
#if defined(CONFIG_SYSTEM_PLATFORM_SN98660) || defined(CONFIG_SYSTEM_PLATFORM_SN98661)
		writebufferparam->write_buf_size = 5 * 1024 * 1024;
#else
#if defined(CONFIG_SYSTEM_PLATFORM_SN98672)
		writebufferparam->write_buf_size = 3 * 1024 * 1024;
#else
		writebufferparam->write_buf_size = 2.5 * 1024 * 1024;
#endif
#endif
	else if((pv_param->width >= HD_WIDTH) && (pv_param->height >= HD_HEIGHT))
		writebufferparam->write_buf_size = 1.5 * 1024 * 1024;
	else if((pv_param->width >= VGA_WIDTH) && (pv_param->height >= VGA_HEIGHT))
		writebufferparam->write_buf_size = 1.2 * 1024 * 1024;
	else
		writebufferparam->write_buf_size = 1.2 * 1024 * 1024;

	writebufferparam->write_unit_to_file = 0x20000;
}


unsigned int uvc_get_bps(uint32_t fmt, uint32_t res)
{
	unsigned int bps = 0;
	if(fmt == 1)
	{
		switch(res)
		{
		case 2: // HD
			bps = HD_H264_BPS;
			break;

		case 5: // VGA
		case 6:
			bps = VGA_H264_BPS;
			break;

		case 11: // FHD
			bps = FHD_H264_BPS;
			break;

		default:
			bps = 0;
			break;
		}
	}
	else
	{
		return 0;
	}

	return bps;
}

int cmd_uvc_start(int argc, char* argv[])
{
	USBH_Device_Structure 			*uvc_dev;
	USBH_UVC_INFO_Struct			*uvc_info;	
	UVC_APP_STRUCTURE				TEMP_UVC_APP;
	int 							ret;
	uint32_t 		uvc_devid=0,uvc_fmt=0,uvc_res=0,uvc_fps=0;
	recordinfo_t* 		uvc_sdrecord_info=NULL;

	// check basic parameter 
	if(argc <5 ) goto hint;

	// assign basic parameter 
	if(uvc_get_para(argv,&uvc_devid,&uvc_fmt,&uvc_res,&uvc_fps) == pdFAIL) goto hint;

	// assign option 
	memset(&TEMP_UVC_APP,0,sizeof(UVC_APP_STRUCTURE));	

	if(argc >= 7){
		if((strcmp(argv[6], "SD") == 0) || (strcmp(argv[6], "sd") == 0)){
			TEMP_UVC_APP.SDReocrdEnable = 1;
		}
		else if (strcmp(argv[6], "preview") == 0) {
			TEMP_UVC_APP.previewEnable = 1;
		}
	}

	if(argc >= 8){
		TEMP_UVC_APP.DebugEnable = 1;
	}	

	TEMP_UVC_APP.width		=	simple_strtoul(argv[3], NULL, 10);
	TEMP_UVC_APP.height		=	simple_strtoul(argv[4], NULL, 10);
	TEMP_UVC_APP.fps		=	simple_strtoul(argv[5], NULL, 10);

	if(TEMP_UVC_APP.SDReocrdEnable){		
		// Check SD Card is Ready
		if(get_sd_umount_err() == 1) goto err;
	
		//Init record parameter
		uvc_sdrecord_info = uvc_sd_info_init();
	
		//init sd recoder
		video_param_t vparam;
		vparam.width 			= TEMP_UVC_APP.width;
		vparam.height 			= TEMP_UVC_APP.height;
		vparam.ucFps			= (unsigned char)TEMP_UVC_APP.fps;
		vparam.uiBps			= uvc_get_bps(uvc_fmt, uvc_res);
		vparam.ucScale			= 0;
		vparam.ucStreamMode		= FMT_H264 | FMT_MJPEG;

		if(vparam.uiBps == 0)
			goto vdo_hint;

		uvc_sdrecord_info->RecParam.vdo.width 			= vparam.width;
		uvc_sdrecord_info->RecParam.vdo.height 			= vparam.height;
		uvc_sdrecord_info->RecParam.vdo.ucFps 			= vparam.ucFps;
		uvc_sdrecord_info->RecParam.vdo.uiBps			= vparam.uiBps;
		uvc_sdrecord_info->RecParam.vdo.ucScale 		= vparam.ucScale;
		uvc_sdrecord_info->RecParam.vdo.ucStreamMode 	= vparam.ucStreamMode;
		uvc_sdrecord_info->RecParam.max_record_len 		= 180;

//		uvc_sdrecord_info->RecParam.ado.uiFormat = 1048576;
//		uvc_sdrecord_info->RecParam.ado.uiSampleRate = 11025;
//		uvc_sdrecord_info->RecParam.ado.ucBitsPerSample = 16;
//		uvc_sdrecord_info->RecParam.ado.uiPacketSize = 2048;
//		uvc_sdrecord_info->RecParam.ado.uiBitRate = 16000;

		uvc_sd_set_rec_writebuffer(&(uvc_sdrecord_info->RecParam.writebufferparam),&vparam);

		ret = record_init(&(uvc_sdrecord_info->pRecord_info), &uvc_sdrecord_info->RecParam, 1, RECORD_AVI, 0, 0, 0, 0);

		if(ret == pdPASS)
			record_set_prefix_name(uvc_sdrecord_info->pRecord_info, "UVC");
		else
			goto err;		
	
		//Start Record 
		record_set_start(uvc_sdrecord_info->pRecord_info, 0);
	}
	else if (TEMP_UVC_APP.previewEnable) {

		isWaitIFrame = 0;
		isIFrame = 0;
		currentFrameType = 0;

		if (uvc_fmt == 1) {
			isWaitIFrame = 1;
		}
	}
	//get usb device
	uvc_dev 	=	(USBH_Device_Structure*)uvc_init(uvc_devid);
	if (uvc_dev == NULL) {
		print_msg_queue("uvc_init(%d) failed\n",uvc_devid);
		goto err;
	}

	if (xTASK_HDL_UVC_APP == NULL) {
		// Create get video stream Task
		xTaskCreate(
			uvc_app_task,
			(const char * )"USBH_UVC_APP_TASK",
			8192,
			NULL,
			PRIORITY_TASK_DRV_USBH,
			&xTASK_HDL_UVC_APP);
	}

	//allocate memory 
	if (uvc_dev->CLASS_DRV == USBH_UVC_ISO_CLASS) {
#ifdef 	Sonix_ISO_Accelerator_Enable		
		TEMP_UVC_APP.size = MaxFrameSize;
		do {
			TEMP_UVC_APP.ptr = (uint8_t*) pvPortMalloc(TEMP_UVC_APP.size, GFP_DMA, MODULE_DRI_USBH);
		} while (TEMP_UVC_APP.ptr == NULL);
#else
		TEMP_UVC_APP.size = Standard_iTD_interval*3072*8;
		do {
			TEMP_UVC_APP.ptr = (uint8_t*)pvPortMalloc(TEMP_UVC_APP.size, GFP_DMA, MODULE_DRI_USBH);
		}while(TEMP_UVC_APP.ptr == NULL);
#endif
	} else if (uvc_dev->CLASS_DRV == USBH_UVC_BULK_CLASS) {
		TEMP_UVC_APP.size = USBH_UVC_BK_STREAM_BUF_SIZE * 2; // ping/pong buf, erick modify
		do {
			TEMP_UVC_APP.ptr = (uint8_t*) pvPortMalloc(TEMP_UVC_APP.size, GFP_DMA, MODULE_DRI_USBH);
		} while (TEMP_UVC_APP.ptr == NULL);
	}
	memset(TEMP_UVC_APP.ptr, 0, TEMP_UVC_APP.size);

	UVC_INFO("UVC_APP.ptr = 0x%x",(uint32_t)TEMP_UVC_APP.ptr);

	//UVC START 
	TEMP_UVC_APP.StreamID	=	uvc_start(uvc_dev, uvc_fmt, uvc_res, uvc_fps, TEMP_UVC_APP.ptr, TEMP_UVC_APP.size);

	UVC_INFO("\r\n The Stream ID is : 0x%x",TEMP_UVC_APP.StreamID);	

	//Copy UVC_APP to global with stream ID 
	memcpy(&UVC_APP[TEMP_UVC_APP.StreamID], &TEMP_UVC_APP, sizeof(UVC_APP_STRUCTURE));
	memcpy(&UVC_SDRECORD_INFO[TEMP_UVC_APP.StreamID], uvc_sdrecord_info, sizeof(recordinfo_t));
	return NULL;	

dev_hint:
	UVC_INFO("\r\n device not exist!");
	return NULL;

vdo_hint:
	UVC_INFO("\r\n this fmt or res is not supported!");
	return NULL;
	
hint:
	UVC_INFO("\r\n usage : uvc_start devid fmt resX resY fps RecEn/PreEn");
	UVC_INFO("\r\n    ex : uvc_start 1 h264 1920 1080 30 sd/preview");
	return NULL;		
	
err:	
	UVC_INFO("\r\nUVC Control Error occur!");
	return NULL;	 	
	
}

uint8_t stop_streamid = 0;
uint8_t stop_flag = 0;
int cmd_uvc_stop(int argc, char* argv[])
{
	USBH_Device_Structure 			*uvc_dev;
	uint32_t						temp_id;
	uint8_t				dev_id;

	if(argc != 2) goto hint;
	temp_id = simple_strtoul(argv[1], NULL, 10);
	if(UVC_APP[temp_id].SDReocrdEnable){
		record_set_stop(UVC_SDRECORD_INFO[temp_id].pRecord_info, 0);
		record_uninit(UVC_SDRECORD_INFO[temp_id].pRecord_info);
		//record_set_stop(uvc_sdrecord_info->pRecord_info, 0);
		//record_uninit(uvc_sdrecord_info->pRecord_info);
	}
	else if (UVC_APP[temp_id].previewEnable) {
		isWaitIFrame = 0;
		isIFrame = 0;
		currentFrameType = 0;
	}
	dev_id = uvc_streamid_to_devid(temp_id);
	uvc_dev	= (USBH_Device_Structure*)uvc_init(dev_id);

	if(uvc_dev!=0){
		uvc_stop(uvc_dev, temp_id);
	}
	vPortFree(UVC_APP[temp_id].ptr);
	UVC_INFO("\r\nStream ID = 0x%d , Total Frame count = %d",UVC_APP[temp_id].StreamID,UVC_APP[temp_id].framecnt);

	stop_streamid = temp_id;
	stop_flag = 1;
	return 0;
	
hint:
	UVC_INFO("\r\n usage : uvc_stop StreamID");
	UVC_INFO("  ex : uvc_stop 1");
	return 0;

}

#define FRAM_CNT 500
void uvc_app_task(void * pvParameters)
{
	USBH_UVC_STREAM_Structure		uvc_stream;		
	struct timeval 					tval;
	uint8_t 						iframe;
	uint32_t 			prv_cnt[6] = {0};
	uint8_t 			cur_streamid = 1;
	uint8_t 			chg_frame = 0;

	memset(&uvc_stream, 0x00, sizeof(uvc_stream));
	for(;;){
		xQueueReceive(USBH_QUEUE_STREAM_DATA, &uvc_stream, portMAX_DELAY);		
		UVC_APP[uvc_stream.id].framecnt ++;		
		if((uvc_stream.id != 0) && ((uvc_stream.id <= 10))){
			//if((uvc_stream.size > 12) && (((uint32_t)uvc_stream.ptr)+uvc_stream.size) <= uvc_stream.ring_buff_end ){
			if((uvc_stream.size > 12) && (uvc_stream.size < USBH_UVC_BK_STREAM_BUF_SIZE)){
#if 0				
				tval.tv_usec = ((framecnt*33*1000)%999999);
				tval.tv_sec = (((framecnt*33)-(tval.tv_usec/1000))/1000);					
#else
			    gettimeofday (&tval,NULL);
#endif			
				if(UVC_APP[uvc_stream.id].SDReocrdEnable){
					((currentFrameType = snx_avc_get_slice_type(uvc_stream.ptr, uvc_stream.size)) == TYPE_I) ? (iframe = 1) : (iframe = 0);
					if((currentFrameType == TYPE_I) || (currentFrameType == TYPE_P)) {
						record_video(UVC_SDRECORD_INFO[uvc_stream.id].pRecord_info
									, iframe
									, (uint32_t *)uvc_stream.ptr
									, uvc_stream.size, tval);
					}
				}
				else if(UVC_APP[uvc_stream.id].previewEnable) {

					if (isWaitIFrame) { // H264
						if (!isIFrame) {
							currentFrameType = snx_avc_get_slice_type(uvc_stream.ptr, uvc_stream.size);
							if (currentFrameType == TYPE_I) {
								isIFrame = 1;
							}
						}
						if (isIFrame) {
							usbd_uvc_drv_send_image(uvc_stream.ptr + 3, uvc_stream.size - 12);
						}
					} else {
						if(chg_frame){
							if(uvc_stream.id != cur_streamid){
								cur_streamid = uvc_stream.id;
								chg_frame = 0;
								print_msg("streamid:%d\n", uvc_stream.id);
							}
						}
						if(stop_flag) {
							if(cur_streamid == stop_streamid){
								cur_streamid = uvc_stream.id;
								stop_flag = 0;
							}
						}		
						if(uvc_stream.id == cur_streamid){
							if(prv_cnt[cur_streamid] <= FRAM_CNT){
								usbd_uvc_drv_send_image(uvc_stream.ptr+3, uvc_stream.size-12);
								prv_cnt[uvc_stream.id]++;
							}
							else{
								prv_cnt[cur_streamid] = 0;
								chg_frame = 1;							
							}
						}						
						//usbd_uvc_drv_send_image(uvc_stream.ptr + 3, uvc_stream.size - 12);
					}
				}

				if(UVC_APP[uvc_stream.id].DebugEnable){
					UVC_INFO("\r\n =================================");
					UVC_INFO("\r\n uvc_stream.id = %d",uvc_stream.id);
					UVC_INFO("\r\n uvc_stream.ptr = %x",uvc_stream.ptr);
					UVC_INFO("\r\n uvc_stream.size = %d",uvc_stream.size);
					UVC_INFO("\r\n uvc_stream.ring_buff_end = %x",uvc_stream.ring_buff_end);
					UVC_INFO("\r\n uvc_stream timeval = %x",tval.tv_sec*1000+(tval.tv_usec+500)/1000);												
					UVC_INFO("\r\n uvc_stream iframe = %x",iframe);
					UVC_INFO("\r\n =================================");								
				}
			}else{
				if(UVC_APP[uvc_stream.id].DebugEnable){
					UVC_INFO(" =================================");
					UVC_INFO(" uvc_stream.id = %d",uvc_stream.id);
					UVC_INFO(" uvc_stream.ptr = %x",uvc_stream.ptr);
					UVC_INFO(" uvc_stream.size = %d",uvc_stream.size);
					UVC_INFO(" uvc_stream.ring_buff_end = %x",uvc_stream.ring_buff_end);
					UVC_INFO(" uvc_stream timeval = %x",tval.tv_sec*1000+(tval.tv_usec+500)/1000);
					UVC_INFO(" uvc_stream iframe = %x",iframe);
					UVC_INFO(" =================================");
				}
				UVC_INFO("\r\n SKIP FRAME \n");
			}
			uvc_stream_complete(&uvc_stream);
		}else{	
			UVC_INFO("\r\n =================================");
			UVC_INFO("\r\n =================================");				
			UVC_INFO("\r\n ID Err or Buffer under run");
			UVC_INFO("\r\n =================================");
			UVC_INFO("\r\n =================================");																				
		}
	}	
}

#endif
