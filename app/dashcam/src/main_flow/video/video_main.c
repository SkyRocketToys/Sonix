/**
 * @file
 * this is application file for video data capture
 */



#include <FreeRTOS.h>
#include <task.h>
#include <bsp.h>
#include <nonstdlib.h>
#include <string.h>
#include <stdio.h>
#include <queue.h>
#include <semphr.h>
#include <sys/time.h>
#include "video_main.h"
#include "rec_schedule.h"
#include "../sensor_cap/sensor_capability.h"
#include <libmid_rtsp_server/rtsp_server.h>
#include "snapshot.h"
#include <libmid_nvram/snx_mid_nvram.h>
#include <libmid_isp/snx_mid_isp.h>
#include <libmid_automount/automount.h>
#include <wifi/wifi_api.h>
#include "main_flow.h"
#include "json_cmd.h"
#include "watch_task.h"
#include "user_param.h"
#include <usb_device/usbd_uvc.h>
#include "usbd_ext.h"
#include <generated/snx_sdk_conf.h>
#include "utility.h"

//#define PREVIEW_ALWAYS_ENABLE	0	/**<  1:always enable preview path even if no user uses preview*/

#if RTSP_CHECK_LEGAL
int legal_preview = 0;		/**< if legal_preview is 0, then I frame will be atttach illegal information bit. */ 
#endif


static int video_hack = 0;          /**< Hack video resolution to be 640 X 360 15fps 512bps **/
/**
* @brief variable for record infomation.
*/
video_user_param_t  PreviewUserParam;

/**
 * @brief variable for isp channel identify to replace id by feature.
*/
//static video_info_t ispch0_info, ispch1_info;
video_info_t ispch0_info, ispch1_info;

struct snx_m2m *snx_m2m_channel0 = &ispch0_info.m2m;;
struct snx_m2m *snx_m2m_channel1 = &ispch1_info.m2m;

int Livestream_id = -1;
int preview_use_isp0dup = 0;

extern int takepic_num;
extern int snapshot_onepic;

void osd_preview_is_dup1(int enable)
{
	osd_ds_init(0, &ispch0_info.ds_info, &ispch0_info.m2m.vc); //isp0dup0 for record
	osd_ds_set_en(&ispch0_info.ds_info, enable);

	osd_ds_init(1, &ispch0_info.ds_info, &ispch0_info.m2m.vc); //isp0dup0 for record
	osd_ds_set_en(&ispch0_info.ds_info, enable);
}


void osd_preview_is_dup1_setting()
{
	int osdenable = 0;
	if (snx_nvram_integer_get(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_VIDEO_OSD, &osdenable) != NVRAM_SUCCESS) {
		VDO_PRINT(SYS_ERR, "Get OSDStatus from NVRAM failed!\n");
	}

	if (osdenable == 1)
		osd_preview_is_dup1(1);
	else
		osd_preview_is_dup1(0);
}

void osd_preview_is_isp1(int enable)
{
	osd_ds_init(0, &ispch0_info.ds_info, &ispch0_info.m2m.vc); //isp0dup0 for record
	osd_ds_set_en(&ispch0_info.ds_info, enable);

	osd_ds_init(0, &ispch1_info.ds_info, &ispch1_info.m2m.vc); //isp1dup0 for preview
	osd_ds_set_en(&ispch1_info.ds_info, enable);
}

void osd_preview_is_isp1_setting()
{
	int osdenable = 0;
	
	if (snx_nvram_integer_get(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_VIDEO_OSD, &osdenable) != NVRAM_SUCCESS) {
		VDO_PRINT(SYS_ERR, "Get OSDStatus from NVRAM failed!\n");
	}

	if (osdenable == 1)
		osd_preview_is_isp1(1);
	else
		osd_preview_is_isp1(0);
}


void mf_video_h264_set_iframe(void)
{
	snx_video_set_keyframe(&ispch1_info.m2m, MASTER_PATH);
}

void mf_video_h264_set_iframe_ispdup(void)
{
	snx_video_set_keyframe(&ispch0_info.m2m, DUP_PATH);
}

void mf_video_h264_set_iframe_for_record(void)
{
	snx_video_set_keyframe(&ispch0_info.m2m, MASTER_PATH);
}

void set_takepic_num(int num)
{
	takepic_num = num;
}

int  preview_is_isp0dup(void)    //for save memory
{
	unsigned rec_w, rec_h, rec_fps;
	unsigned prev_w, prev_h, prev_fps;

	mf_video_resmap_get_record_params(&rec_w, &rec_h, &rec_fps, NULL, NULL);
	mf_video_resmap_get_preview_params(&prev_w, &prev_h, &prev_fps, NULL, NULL);
	preview_use_isp0dup = ((rec_w == prev_w) && (rec_h == prev_h)
	                       && (prev_fps <= rec_fps));

    VDO_PRINT(SYS_INFO, ": rec_w = %u; prev_w = %u; rec_h = %u; prev_h = %u; rec_fps = %u; prev_fps = %u\n",
        rec_w, prev_w, rec_h, prev_h, rec_fps, prev_fps);

    return !!(preview_use_isp0dup);
}

int chk_preview_use_isp0dup()
{
	return !!preview_use_isp0dup;
}


void set_ispch0_params()
{
	unsigned width, height, fps, gop, bps;

	memset(&ispch0_info, 0, sizeof(video_info_t));
	mf_video_resmap_get_record_params(&width, &height, &fps, &gop, &bps);

	if (1 == video_hack) {
		ispch0_info.width = VGA_WIDTH;
		ispch0_info.height = VGA_HEIGHT;
	} else {
        ispch0_info.width = width;
        ispch0_info.height = height;
    }
	ispch0_info.ucFps = (unsigned char) fps;
	ispch0_info.ucGop = (unsigned short) gop;
	ispch0_info.uiBps = (unsigned int) bps;
	ispch0_info.ucScale = 0;
	ispch0_info.ucStreamMode = FMT_H264 | FMT_MJPEG;
	ispch0_info.ucStreamModeusedup1 = FMT_H264; // use dup1 for preview
	ispch0_info.ucIspChannel = CHANNEL_RECORD;
	ispch0_info.ucDupEnable = !!(width < FHD_WIDTH && height < FHD_HEIGHT); // use dup1 for preview
	snapshot_onepic = 0;
	ispch0_info.h264_notice = recordvideo;

	if (chk_preview_use_isp0dup()) {
#if RTSP_CHECK_LEGAL
        if(1 == legal_preview)
        {
            ispch0_info.h264_notice_dup1 = send_preview_to_rtp;
        }
        else
        {
            ispch0_info.h264_notice_dup1 = NULL;
        }
#endif
	} else {
		ispch0_info.h264_notice_dup1 = NULL;
	}

#ifndef CONFIG_APP_DRONE
	ispch0_info.h264_timelapse = reclapse_video;
#endif

	mf_video_resmap_get_sensor_max_capability((int *)&width, (int *)&height);

	memset(ispch0_info.schedthumbnailname, 0x00, sizeof(ispch0_info.schedthumbnailname));
	memset(ispch0_info.timelapsethumbnailname, 0x00, sizeof(ispch0_info.timelapsethumbnailname));
	memset(ispch0_info.protectthumbnailname, 0x00, sizeof(ispch0_info.protectthumbnailname));
}

void set_ispch1_params()
{
	unsigned width, height, fps, gop, bps;

	memset(&ispch1_info, 0, sizeof(video_info_t));
	mf_video_resmap_get_preview_params(&width, &height, &fps, &gop, &bps);

    if (chk_preview_use_isp0dup()) {
		ispch1_info.width = QVGA_WIDTH;
		ispch1_info.height = QVGA_HEIGHT;
		ispch1_info.ucFps = 30;
		ispch1_info.ucGop = 0;
		ispch1_info.uiBps = VGA_H264_BPS;
		ispch1_info.ucStreamMode = FMT_MJPEG;
		ispch1_info.ucDupEnable = 0;
		// replace preview params
		width = ispch1_info.width;
		height = ispch1_info.height;
	} else {
		ispch1_info.width = width;
		ispch1_info.height = height;
		ispch1_info.ucFps = (unsigned char) fps;
		ispch1_info.ucGop = (unsigned short) gop;
		ispch1_info.uiBps = (unsigned int) bps;
#ifndef CONFIG_APP_DRONE
#if defined(USBD_OUTFORMAT_MJPEG) && USBD_OUTFORMAT_MJPEG
		ispch1_info.ucStreamMode = FMT_H264 | FMT_MJPEG;
#else
		ispch1_info.ucStreamMode = FMT_H264;
#endif
#else
		ispch1_info.ucStreamMode = FMT_H264;
#endif
		ispch1_info.ucStreamModeusedup1 = FMT_MJPEG;
#if RTSP_CHECK_LEGAL
        if (1 == legal_preview) {
#endif
            ispch1_info.h264_notice = send_preview_to_rtp;
#if RTSP_CHECK_LEGAL
        } else {
            ispch1_info.h264_notice = NULL;
        }
#endif
		ispch1_info.ucDupEnable = 1; // MJPEG scale 1/4
	}

	if ((width == HD_WIDTH) && (height == HD_HEIGHT))
		ispch1_info.ucScale = 2;
	else if ((width == VGA_WIDTH) && (height == VGA_HEIGHT))
		ispch1_info.ucScale = 1;
	else if ((width == QVGA_WIDTH) && (height == QVGA_HEIGHT))
		ispch1_info.ucScale = 0;

	ispch1_info.ucIspChannel = CHANNEL_PREVIEW;

	memset(ispch1_info.schedthumbnailname, 0x00, sizeof(ispch1_info.schedthumbnailname));
	memset(ispch1_info.timelapsethumbnailname, 0x00, sizeof(ispch1_info.timelapsethumbnailname));
	memset(ispch1_info.protectthumbnailname, 0x00, sizeof(ispch1_info.protectthumbnailname));
}


int set_rtsp_param(video_info_t *pinfo)
{
	video_attr rtsp_video_stream;
#if RTSP_PREVIEW_AUDIO
	int audio_flag = app_uti_get_preview_audio_mode();
	audio_attr rtsp_audio_stream;
#endif

	memset(&rtsp_video_stream, 0, sizeof(video_attr));
	snprintf(rtsp_video_stream.video_codec, sizeof(rtsp_video_stream.video_codec), "h264");
	rtsp_video_stream.height = pinfo->height;
	rtsp_video_stream.width = pinfo->width;
	rtsp_video_stream.fps = pinfo->ucFps;

#if RTSP_PREVIEW_AUDIO
	if(audio_flag == PREVIEW_AUDIO_ON){
		memset(&rtsp_audio_stream, 0, sizeof(audio_attr));
		snprintf(rtsp_audio_stream.audio_codec, sizeof(rtsp_audio_stream.audio_codec), "aac");
		rtsp_audio_stream.bitrate = 16000;
		rtsp_audio_stream.channels = 1;
		rtsp_audio_stream.samplerate = 11025;
		add_rtsp_stream(LIVE_PREVIEW_STREAM_NAME, 0, &rtsp_video_stream, &rtsp_audio_stream);
	}else
		add_rtsp_stream(LIVE_PREVIEW_STREAM_NAME, 0, &rtsp_video_stream, NULL);
#else
	add_rtsp_stream(LIVE_PREVIEW_STREAM_NAME, 0, &rtsp_video_stream, NULL);
#endif
	
	Livestream_id = get_media_id_in_content(LIVE_PREVIEW_STREAM_NAME);
	VDO_PRINT(SYS_DBG, "Livestream %s ID: %x\n", LIVE_PREVIEW_STREAM_NAME, Livestream_id);
	if (Livestream_id < 0) {
		VDO_PRINT(SYS_ERR, "Could not Find the Livstream ID\n");
		return pdFAIL;
	}
	return pdPASS;
}

int mf_video_stream_ispch0_init()
{
	set_ispch0_params();

	if (video_set_init(&ispch0_info) == pdFAIL)
		return pdFAIL;

	ispch0_info.previewdataqueue = xQueueCreate(1, sizeof(uint8_t));
	if (ispch0_info.previewdataqueue == NULL)
		return pdFAIL;

	if (chk_preview_use_isp0dup()) {
		VDO_PRINT(SYS_INFO, "PREVIEW USE ISP0DUP\n");

		if (set_rtsp_param(&ispch0_info) == pdFAIL)
			return pdFAIL;

		rtsp_reg_force_iframe(mf_video_h264_set_iframe_ispdup);
		osd_preview_is_dup1_setting();
	}

	isp0_is_running(1);

	if (xTaskCreate(task_video_get, "vdo_get_ispch0_stream", STACK_SIZE_4K,
	                &ispch0_info, PRIORITY_TASK_APP_VIDEO_GET,
	                &ispch0_info.task_get) != pdPASS) {
		VDO_PRINT(SYS_ERR, "Could not create task video get\n");
		return pdFAIL;
	}

	if (xTaskCreate(task_video_duppreview, "vdo_get_rec_duppreviewstream",
	                STACK_SIZE_4K, &ispch0_info, PRIORITY_TASK_APP_VIDEO_GET,
	                &ispch0_info.task_handlepreview) != pdPASS) {
		VDO_PRINT(SYS_ERR, "Could not create task video get\n");
		return pdFAIL;
	}

#ifndef CONFIG_APP_DRONE
	/* chkuo add, for UVC device re-connect, use new descriptor */
	usbd_app_uvc_reconnect();
#endif

	//VDO_PRINT(SYS_DBG, "ispch0_info.task_get = %x\n", ispch0_info.task_get);
	return pdPASS;
}

static void mf_video_stream_ispch0_uninit(void)  //no use
{
	video_set_uninit(&ispch0_info);
	vTaskDelete(ispch0_info.task_get);
}

int mf_video_stream_ispch1_init()
{
	set_ispch1_params();
	//printvideoparam(&ispch1_info, 1);

	if (video_set_init(&ispch1_info) == pdFAIL)
		return pdFAIL;

	if (!chk_preview_use_isp0dup()) {
		VDO_PRINT(SYS_INFO, "PREVIEW USE ISP1DUP\n");
		if (set_rtsp_param(&ispch1_info) == pdFAIL)
			return pdFAIL;
		rtsp_reg_force_iframe(mf_video_h264_set_iframe);
		osd_preview_is_isp1_setting();
	}

	//start task to get video frame
	isp1_is_running(1);

	if (chk_preview_use_isp0dup()) {
		if (xTaskCreate(task_video_get_thumbnail, "vdo_get_thumbnail_stream", STACK_SIZE_4K, &ispch1_info, PRIORITY_TASK_APP_VIDEO_GET, &ispch1_info.task_get) != pdPASS) {
			VDO_PRINT(SYS_ERR, "Could not create task video get\n");
			return pdFAIL;
		}
	} else {
		if (xTaskCreate(task_video_get, "vdo_get_ispch1_stream", STACK_SIZE_8K,
		                &ispch1_info, PRIORITY_TASK_APP_VIDEO_GET,
		                &ispch1_info.task_get) != pdPASS) {
			VDO_PRINT(SYS_ERR, "Could not create task video get\n");
			return pdFAIL;
		}
	}

	//VDO_PRINT(SYS_DBG, "ispch1_info.task_get = %x\n", ispch1_info.task_get);
	return pdPASS;
}

static void mf_video_stream_ispch1_uninit(void)  //no use
{
	video_set_uninit(&ispch1_info);
	vTaskDelete(ispch1_info.task_get);
}

extern int factory_mode;
 
/**
* @brief interface function - video capture initialization
* @return return pdPASS if success
*/
int mf_video_init(void)
{
#if RTSP_PREVIEW_AUDIO
	init_rtsp_server(1);
#else
	init_rtsp_server(0);
#endif

#if RTSP_CHECK_LEGAL // customer's special request to ensure security.
    legal_preview = 1;
	if(0 == factory_mode)
	{
		enc_auth_t auth_mode = AUTH_NONE;

		if( snx_nvram_integer_get("WIFI_DEV", "AP_AUTH_MODE", (int *)&auth_mode) != NVRAM_SUCCESS)
		{
			VDO_PRINT(SYS_ERR, "Could not get authentication mode - rtsp use disabled!\n");
            legal_preview = 0;
		}
		else if(AUTH_NONE == auth_mode)
		{
#ifdef CONFIG_DISABLE_VIDEO_WIFI_OPEN
			VDO_PRINT(SYS_WARN, "Wi-Fi authentication mode open - rtsp use disallowed!\n");
			legal_preview = 0;
#else
			VDO_PRINT(SYS_WARN, "Wi-Fi authentication mode open - rtsp use allowed\n");
			legal_preview = 1;
#endif
		}
	}
#endif

    preview_is_isp0dup();
    if (mf_video_stream_ispch0_init() != pdPASS)
    {
        return pdFAIL;
    }

    if (mf_video_stream_ispch1_init() != pdPASS)
    {
        return pdFAIL;
    }

	mf_snapshot_init();
	mf_thumbnail_init();

	return pdPASS;
}

/**
* @brief interface function - video capture  uninitialization  //no use
*/
void mf_video_uninit(void)
{
	mf_snapshot_uninit();
	mf_thumbnail_uninit();
	mf_video_stream_ispch1_uninit();
	mf_video_stream_ispch0_uninit();
}

void printvideoparam(video_info_t *pinfo, int type)
{
	if (type == 0) {
		VDO_PRINT(SYS_DBG, "ispch0_info.width=%d, ispch0_info.height=%d, ispch0_info.ucFps=%d\n", pinfo->width, pinfo->height);
		VDO_PRINT(SYS_DBG, "ispch0_info.ucFps=%d, ispch0_info.ucGop=%d\n", pinfo->ucFps, pinfo->ucGop);
		VDO_PRINT(SYS_DBG, "ispch0_info.uiBps=%ld\n", pinfo->uiBps);
	} else if (type == 1) {
		VDO_PRINT(SYS_DBG, "ispch1_info.width=%d, ispch1_info.height=%d, ispch1_info.ucFps=%d\n", pinfo->width, pinfo->height);
		VDO_PRINT(SYS_DBG, "ispch1_info.ucFps=%d, ispch1_info.ucGop=%d\n", pinfo->ucFps, pinfo->ucGop);
		VDO_PRINT(SYS_DBG, "ispch1_info.uiBps=%ld\n", pinfo->uiBps);
	}
}


void set_thumbnail_pic(video_info_t *pVInfo, unsigned char *pFrameaddr, unsigned int uiFrameSize)
{
	if (pVInfo->mj_notice) {
//		VDO_PRINT(SYS_DBG, "(%d)get thumbnail mj frame\n", pVInfo->ucIspChannel);
		if (*pVInfo->schedthumbnailname) {
			(pVInfo->mj_notice)(pFrameaddr, uiFrameSize, pVInfo->schedthumbnailname, SCHED_RECORD);
			memset(pVInfo->schedthumbnailname, 0x00, sizeof(pVInfo->schedthumbnailname));
		}
		if (*pVInfo->timelapsethumbnailname) {
			(pVInfo->mj_notice)(pFrameaddr, uiFrameSize, pVInfo->timelapsethumbnailname, TIMELAPSE_RECORD);
			memset(pVInfo->timelapsethumbnailname, 0x00, sizeof(pVInfo->timelapsethumbnailname));
		}
		if (*pVInfo->protectthumbnailname) {
			(pVInfo->mj_notice)(pFrameaddr, uiFrameSize, pVInfo->protectthumbnailname, PROTECT_RECORD);
			memset(pVInfo->protectthumbnailname, 0x00, sizeof(pVInfo->protectthumbnailname));
		}
	}
}

void check_sdcard_exist(app_frameadddata *info, bool value)
{
	info->bits.sdcardexist = value;
}
void check_record_running(app_frameadddata *info, bool value)
{
	info->bits.recordruning = value;
}

void check_sdcard_sizetoosmall(app_frameadddata *info, bool value)
{
	info->bits.sdsizeistoosmall = value;
}

void check_sdcard_isFat32(app_frameadddata *info, bool value)  //1:is fat32 0:not fat32
{
	info->bits.sdformatisfat32 = value;
}

#if RTSP_CHECK_LEGAL
/* Assgin additional byte in bit-4 for notifi legal preview or not. */
void check_legal_preview(app_frameadddata *info, bool value)
{
	info->bits.legal_preview = value;
}
#endif

void addtional_info(app_frameadddata *info)
{
	int result;
	if (get_sd_umount_err()) {
		result = get_sd_status();
		if (result == FR_NO_FILESYSTEM) {
			check_sdcard_exist(info, TRUE);     //have sdcard but not fat32
			check_sdcard_isFat32(info, FALSE);
		} else {
			check_sdcard_exist(info, FALSE);     //no sdcard
			check_sdcard_isFat32(info, FALSE);
		}
	} else { //have sdcard and fat32
		check_sdcard_exist(info, TRUE);
		check_sdcard_isFat32(info, TRUE);
	}

	if (schedrec_state() == 1)
		check_record_running(info, TRUE);
	else
		check_record_running(info, FALSE);

	if (cardsize_toosmall())
		check_sdcard_sizetoosmall(info, TRUE);
	else
		check_sdcard_sizetoosmall(info, FALSE);
#if RTSP_CHECK_LEGAL
	if (legal_preview == 0) {
		VDO_PRINT(SYS_DBG, "Illeagle preview...\n");
		check_legal_preview(info, FALSE);
	}
	else
		check_legal_preview(info, TRUE);
#endif
	//VDO_PRINT(SYS_DBG, "info.frameadddata=%d\n",info->frameadddata);

}


void send_preview_to_rtp(unsigned char IFrame, unsigned char *pFrame,
                         unsigned int uiFrameSize, struct timeval tval)
{
	unsigned fps = 0;

	mf_video_resmap_get_preview_params(NULL, NULL, &fps, NULL, NULL);

#if 0
	static unsigned int  pre_time = 0;
	unsigned int cur_time = 0;
	static int time_count = 0;
	static frame_count = 1;
	cur_time = xTaskGetTickCount() * portTICK_RATE_MS;
	unsigned int timediff = cur_time - pre_time;
	int time_interval = 1000 / fps;


	if ((timediff > time_interval) && (pre_time != 0)) {
		time_count += (timediff - time_interval);
	}

	if ((timediff < time_interval) && (pre_time != 0)) {
		time_count -= (time_interval - timediff);
	}

	if ((timediff < time_interval) && (pre_time != 0) && (time_count < 0)) {
		vTaskDelay( (0 - time_count) / portTICK_RATE_MS);
		time_count = 0;
		cur_time = xTaskGetTickCount() * portTICK_RATE_MS;
		timediff = cur_time - pre_time;
	}

	//VDO_PRINT(SYS_DBG, "Pre_View timediff=%d  pre_time=%u  cur_time=%u\n",cur_time-pre_time, pre_time, cur_time);
	pre_time = cur_time;
#endif

	if (IFrame == pdTRUE) {
		app_frameadddata frame_addtional_information;
		addtional_info(&frame_addtional_information);      //0x53 0x4E(tag)
		*(pFrame + uiFrameSize)   =  0x53;
		*(pFrame + uiFrameSize + 1) =  0x4E;
		*(pFrame + uiFrameSize + 2) =  frame_addtional_information.frameadddata;
		*(pFrame + uiFrameSize + 3) =  (unsigned char) fps;
		*(pFrame + uiFrameSize + 4) =  0x00;
		*(pFrame + uiFrameSize + 5) =  0x00;
		*(pFrame + uiFrameSize + 6) =  0x00;
		*(pFrame + uiFrameSize + 7) =  0x00;
		send_rtp_data(Livestream_id & 0xFFFF, (char *)pFrame, uiFrameSize + 8, NULL, PREVIEW_STREAM);
	} else
		send_rtp_data(Livestream_id & 0xFFFF, (char *)pFrame, uiFrameSize, NULL, PREVIEW_STREAM);
	/*
	video_id = id & 0xFFFF;
	audio_id = id >> 16;
	*/
}


/**
* @brief interface function - set function pointer to get record frame
* @param notice_function function pointer to notice frame data ready
*/
void mf_video_set_record_cb(h264_notice_t notice_function)
{
	ispch0_info.h264_notice = notice_function;
}

/**
* @brief interface function - set function pointer to get record frame
* @param notice_function function pointer to notice frame data ready
*/
void mf_video_set_record_dup1_cb(h264_notice_t notice_function)
{
	ispch0_info.h264_notice_dup1 = notice_function;
}

/**
* @brief interface function - set function pointer to get record frame
* @param notice_function function pointer to notice frame data ready
*/
void mf_video_set_timelapse_cb(h264_notice_t notice_function)
{
	ispch0_info.h264_timelapse = notice_function;
}

int check_takepic_task(void)
{
	VDO_PRINT(SYS_DBG, "check_takepic_task = %d\n", snapshot_onepic);
	return snapshot_onepic;
}

/**
* @brief interface function - set snapshot to caputre one image
*/
int mf_set_snapshot(unsigned char ucEnable)
{
	if (ucEnable) {
		//prevent users from doing the same task many times
		snapshot_onepic = 1;
	}

	if (!ispch0_info.ucStrameOn) {
		VDO_PRINT(SYS_DBG, "stream off\n");
		return pdFAIL;
	}
	if (ucEnable)
		ispch0_info.ucStreamMode |= FMT_MJPEG;
	else
		ispch0_info.ucStreamMode &= (~FMT_MJPEG);

	return pdPASS;
}

int mf_set_thumbnail()
{
	if (chk_preview_use_isp0dup() == 1) {
		if (!ispch1_info.ucStrameOn) {
			VDO_PRINT(SYS_DBG, "stream off\n");
			return pdFAIL;
		}
		ispch1_info.ucStreamMode |= FMT_MJPEG;
	} else {
		if (!ispch1_info.ucStrameOn) {
			VDO_PRINT(SYS_DBG, "stream off\n");
			return pdFAIL;
		}
		ispch1_info.ucStreamModeusedup1 |= FMT_MJPEG;
	}
	return pdPASS;
}


int mf_set_thumbnail_fordup(const char *thumbnailname, enum RECORDKIND type)
{
	if (!ispch1_info.ucStrameOn) {
		VDO_PRINT(SYS_DBG, "stream off\n");
		return pdFAIL;
	}

	if (chk_preview_use_isp0dup() == 1) {
		ispch1_info.ucStreamMode |= FMT_MJPEG;
	} else {
		ispch1_info.ucStreamModeusedup1 |= FMT_MJPEG;
	}

	if (type == SCHED_RECORD)
		strcpy(ispch1_info.schedthumbnailname, thumbnailname);
	else if (type == TIMELAPSE_RECORD)
		strcpy(ispch1_info.timelapsethumbnailname, thumbnailname);
	else if (type == PROTECT_RECORD)
		strcpy(ispch1_info.protectthumbnailname, thumbnailname);

	return pdPASS;
}



/**
* @brief interface function - set preview start or stop
* @param ucEnable 0:preview stop 1:preview start
*/
void mf_set_preview(unsigned char ucEnable)
{
//#if !PREVIEW_ALWAYS_ENABLE
	if (chk_preview_use_isp0dup() == 1) {
		VDO_PRINT(SYS_DBG, "isp0dup1 preview %s\n", ucEnable ? "ON" : "OFF");

		if (ucEnable)
			ispch0_info.ucStreamModeusedup1 |= FMT_H264;
		else
			ispch0_info.ucStreamModeusedup1 &= (~FMT_H264);

	} else {
		VDO_PRINT(SYS_DBG, "isp1dup0 preview %s\n", ucEnable ? "ON" : "OFF");

		if (ucEnable)
			ispch1_info.ucStreamMode |= FMT_H264;
		else
			ispch1_info.ucStreamMode &= (~FMT_H264);
	}
//#endif
}

/**
* @brief interface function - set uvc view start or stop
* @param ucEnable 0:uvc view stop 1:uvc view start
*/

void mf_set_uvc_view(unsigned char ucEnable)
{
	if (chk_preview_use_isp0dup() == 1) {
#if defined(USBD_OUTFORMAT_MJPEG) && USBD_OUTFORMAT_MJPEG
		VDO_PRINT(SYS_DBG, "isp0dup1 mjpeg preview %s\n", ucEnable ? "ON" : "OFF");
		if (ucEnable)
			ispch0_info.ucStreamMode |= FMT_MJPEG;
		else
			ispch0_info.ucStreamMode &= (~FMT_MJPEG);
#else
		VDO_PRINT(SYS_DBG, "isp0dup1 h264 preview %s\n", ucEnable ? "ON" : "OFF");
		if (ucEnable)
			ispch0_info.ucStreamModeusedup1 |= FMT_H264;
		else
			ispch0_info.ucStreamModeusedup1 &= (~FMT_H264);
#endif
		if (usbd_app_IsPlugin() == 1) {
			if (ucEnable == 1)
				mf_video_set_usbd_uvc_image_cb(usbd_uvc_drv_send_image, CHANNEL_RECORD);
			else if (ucEnable == 0)
				mf_video_set_usbd_uvc_image_cb(NULL, CHANNEL_RECORD);
			else {
				mf_video_set_usbd_uvc_image_cb(NULL, CHANNEL_RECORD);
				VDO_PRINT(SYS_ERR, "Parameter is not correct\n");
			}
		}
	} else {

#if defined(USBD_OUTFORMAT_MJPEG) && USBD_OUTFORMAT_MJPEG
		VDO_PRINT(SYS_DBG, "isp1dup0 mjpeg preview %s\n", ucEnable ? "ON" : "OFF");
		if (ucEnable)
			ispch1_info.ucStreamMode |= FMT_MJPEG;
		else
			ispch1_info.ucStreamMode &= (~FMT_MJPEG);
#else
		VDO_PRINT(SYS_DBG, "isp1dup0 h264 preview %s\n", ucEnable ? "ON" : "OFF");
		if (ucEnable)
			ispch1_info.ucStreamMode |= FMT_H264;
		else
			ispch1_info.ucStreamMode &= (~FMT_H264);
#endif
		if (usbd_app_IsPlugin() == 1) {
			if (ucEnable == 1)
				mf_video_set_usbd_uvc_image_cb(usbd_uvc_drv_send_image, CHANNEL_PREVIEW);
			else if (ucEnable == 0)
				mf_video_set_usbd_uvc_image_cb(NULL, CHANNEL_PREVIEW);
			else {
				mf_video_set_usbd_uvc_image_cb(NULL, CHANNEL_PREVIEW);
				VDO_PRINT(SYS_ERR, "Parameter is not correct\n");
			}
		}
	}
}



/**
* @brief interface function - set record start or stop
* @param ucEnable 0:preview stop 1:preview start
*/
void mf_set_record(unsigned char ucEnable)
{
	VDO_PRINT(SYS_DBG, "H.264 Record %s\n", ucEnable ? "ON" : "OFF");
	if (ucEnable)
		ispch0_info.ucStreamMode |= FMT_H264;
	else
		ispch0_info.ucStreamMode &= (~FMT_H264);
}

/**
* @brief interface function - get parameter for video record
* @param pv_param struct for video parameter
*/
void mf_video_get_rec_param(video_param_t *pv_param)
{
	unsigned w, h, fps, bps;
	mf_video_resmap_get_record_params(&w, &h, &fps, NULL, &bps);

	pv_param->width = w;
	pv_param->height = h;
	pv_param->ucFps = (unsigned char) fps;
	pv_param->uiBps = bps;
	pv_param->ucScale = ispch0_info.ucScale;
	//pv_param->ucStreamMode = ispch0_info.ucStreamMode;
	pv_param->ucStreamMode = FMT_H264 | FMT_MJPEG; //if streamtask init ok, then apporlock mf_set_record stop, that record task  ucStreamMode is error
}


/**
* @brief interface function - set function pointer to get preview frame
* @param notice_function function pointer to notice frame data ready
*/
void mf_video_set_preview_cb(h264_notice_t notice_function)
{
	ispch1_info.h264_notice = notice_function;
#if 0
	// TODO: check automatically in setting function
	if (chk_preview_use_isp0dup() == 1) {
		ispch0_info.h264_notice_dup1 = notice_function;
	} else {
		ispch1_info.h264_notice = notice_function;
	}
#endif
}

/**
* @brief interface function - set function pointer to get snapshot image
* @param notice_function function pointer to notice image data ready
*/
void mf_video_set_snapshot_cb(mj_notice_t notice_function)
{
	ispch0_info.mj_notice = notice_function;
}

mj_notice_t mf_video_get_snapshot_cb(void)
{
    return ispch0_info.mj_notice;
}

/**
* @brief interface function - set function pointer to get snapshot image
* @param notice_function function pointer to notice image data ready
*/
void mf_video_set_thumbnail_cb(mj_notice_t notice_function)
{
	ispch1_info.mj_notice = notice_function;
}

/**
* @brief interface function - set function pointer to set uvc image function
* @param cb function pointer to usb device set uvc image function
* @param ch video channel
*/
void mf_video_set_usbd_uvc_image_cb(usbd_video_t cb, int ch)
{
	if (ch == CHANNEL_RECORD) {
		ispch0_info.usbd_vdo2uvc = cb;
	} else if (ch == CHANNEL_PREVIEW) {
		ispch1_info.usbd_vdo2uvc = cb;
	}
}

void check_preview_video_isp_userparam(video_info_t *pVInfo)
{
	int fps;
	int gop;
	unsigned int bitrate;
	mf_video_resmap_get_preview_params(NULL, NULL, (unsigned *) &fps,
	                                   (unsigned *) &gop, &bitrate);

	if (chk_preview_fps_bit() == 1) { //fps
		VDO_PRINT(SYS_DBG, "channel1 dup0 fps=%d\n", fps);
		snx_video_set_fps(&pVInfo->m2m, MASTER_PATH, fps, FMT_H264);
#ifndef CONFIG_APP_DRONE
#if defined(USBD_OUTFORMAT_MJPEG) && USBD_OUTFORMAT_MJPEG
		snx_video_set_fps(&pVInfo->m2m, MASTER_PATH, fps, FMT_MJPEG);
#endif
#endif
		pVInfo->ucFps = fps;	//for rtsp limit fps

		VDO_PRINT(SYS_DBG, "channel1 dup0 gop=%d\n", gop);
		snx_video_set_gop(&pVInfo->m2m, MASTER_PATH, gop);

		set_preview_fps_bit(0);
	}
	if (chk_preview_bps_bit() == 1) { //bps
		VDO_PRINT(SYS_DBG, "channel1 dup0 bitrate=%ld\n", bitrate);
		snx_video_set_bps(&pVInfo->m2m, MASTER_PATH, bitrate);
		set_preview_bps_bit(0);
	}
	if (chk_isp1_task_close() == 1) {
		pVInfo->taskclose_reboottask = 1;
	}

}

void enable_isp0dup1_preview()
{
	if (chk_preview_use_isp0dup() == 0) { // now preview use isp0dup1
		if (ispch0_info.ucIspChannel == CHANNEL_RECORD) {
			VDO_PRINT(SYS_DBG, "channel(%d) dup(%d) enable H264(mode = %d)\n", ispch0_info.ucIspChannel, DUP_PATH, ispch0_info.ucStreamModeusedup1);
			ispch0_info.ucStreamModeusedup1 |= FMT_H264;
			snx_video_set_mode(&ispch0_info.m2m, DUP_PATH, ispch0_info.ucStreamModeusedup1);
#if RTSP_CHECK_LEGAL
            if(1 == legal_preview)
            {
#endif
                mf_video_set_record_dup1_cb(send_preview_to_rtp);
#if RTSP_CHECK_LEGAL
            }
            else
            {
                mf_video_set_record_dup1_cb(NULL);
            }
#endif
			set_preview_fps_bit(1);
			set_preview_bps_bit(1);
		}
		preview_use_isp0dup = 1;
	}
}

void disable_isp0dup1_preview(video_info_t *pinfo)
{
	if (chk_preview_use_isp0dup() == 1) { // now preview use isp0dup1
		if (pinfo->ucIspChannel == CHANNEL_RECORD) {
			VDO_PRINT(SYS_DBG, "channel(%d) dup(%d) close H264(mode = %d)\n", pinfo->ucIspChannel, DUP_PATH, pinfo->ucStreamModeusedup1);
			pinfo->ucStreamModeusedup1 &= (~FMT_H264);
			snx_video_set_mode(&pinfo->m2m, DUP_PATH, pinfo->ucStreamModeusedup1);
			mf_video_set_record_dup1_cb(NULL);
		}
		preview_use_isp0dup = 0;
	}
}

void check_record_video_isp_userparam(video_info_t *pVInfo)
{
	int fps;
	int gop;
	unsigned int bitrate;
	
	if (chk_isp0_task_close() == 1) {
		pVInfo->taskclose_reboottask = 1;
	}
	if (chk_preview_use_isp0dup() == 1) {
		mf_video_resmap_get_preview_params(NULL, NULL, (unsigned *) &fps,
		                                   (unsigned *) &gop, &bitrate);
		
		if (chk_isp0dup_task_close() == 1) {
			VDO_PRINT(SYS_DBG, "chk_isp0dup_task_close\n");
			disable_isp0dup1_preview(pVInfo);
		}
		if (chk_preview_fps_bit() == 1) { //fps
			VDO_PRINT(SYS_DBG, "channel0 dup1 fps=%d\n", fps);
			snx_video_set_fps(&pVInfo->m2m, DUP_PATH, fps, FMT_H264);
#ifndef CONFIG_APP_DRONE
#if defined(USBD_OUTFORMAT_MJPEG) && USBD_OUTFORMAT_MJPEG
			snx_video_set_fps(&pVInfo->m2m, MASTER_PATH, fps, FMT_MJPEG);
#endif
#endif

			VDO_PRINT(SYS_DBG, "channel0 dup1 gop=%d\n", gop);
			snx_video_set_gop(&pVInfo->m2m, DUP_PATH, gop);

			set_preview_fps_bit(0);
		}
		if (chk_preview_bps_bit() == 1) { //bps
			VDO_PRINT(SYS_DBG, "channel0 dup1 bitrate=%ld\n", bitrate);
			snx_video_set_bps(&pVInfo->m2m, DUP_PATH, bitrate);
			set_preview_bps_bit(0);
		}
	}
}

void check_thumbnail_isp1_userparam(video_info_t *pVInfo)
{
	if (chk_isp1_task_close() == 1) {
		pVInfo->taskclose_reboottask = 1;
	}
}


////for HD60 preview Task
int mf_videohd60_init(void)
{
	int result;
#if RTSP_PREVIEW_AUDIO
	init_rtsp_server(1);
#else
	init_rtsp_server(0);
#endif
	memset(&ispch0_info, 0, sizeof(video_info_t));
	ispch0_info.width = HD_WIDTH;
	ispch0_info.height = HD_HEIGHT;
	ispch0_info.ucFps = 60;
	ispch0_info.ucGop = 0; //use gop=fps
	ispch0_info.uiBps = HD_H264_BPS;
	ispch0_info.ucScale = 0;
	ispch0_info.ucStreamMode = FMT_H264;
	ispch0_info.ucIspChannel = CHANNEL_RECORD;   //use isp0
	//ispch0_info.ucIspChannel = CHANNEL_PREVIEW;   //use isp1
	ispch0_info.ucDupEnable = 0;
#if RTSP_CHECK_LEGAL
    if(1 == legal_preview)
    {
#endif
        ispch0_info.h264_notice = send_preview_to_rtp;
#if RTSP_CHECK_LEGAL
    }
    else
    {
        ispch0_info.h264_notice = NULL;
    }
#endif
	if ((result = video_set_init(&ispch0_info)) == pdFAIL)
		return pdFAIL;
	if ((result = set_rtsp_param(&ispch0_info)) == pdFAIL)
		return pdFAIL;
	rtsp_reg_force_iframe(mf_video_h264_set_iframe);
	mf_set_preview(1);
	if (pdPASS != xTaskCreate(task_video_get, "vdo_get_preview_stream", STACK_SIZE_8K, &ispch0_info,
	                          PRIORITY_TASK_APP_VIDEO_GET, &ispch0_info.task_get)) {
		VDO_PRINT(SYS_ERR, "Could not create task video get\n");
		return pdFAIL;
	}
	osd_ds_init(0, &ispch0_info.ds_info, &ispch0_info.m2m.vc);
	osd_ds_set_en(&ispch0_info.ds_info, 1);
	return pdPASS;
}
///////
