/*
*	File: usbd_uvc_vc.c
*	Version: 1.0
*	Author: chkuo
*	Date: 2016-01-14
*	Descriptions: SONiX USB UVC Device Video Capture functions
*/

#include <FreeRTOS.h>
#include <task.h>
#include <bsp.h>
#include <nonstdlib.h>
#include <string.h>
#include <usb_device/usb_device.h>
#include <usb_device/usbd_uvc.h>
#include <libmid_usbd/mid_usbd.h>
#include "usbd_uvc_vc.h"
#include "snx_xu_mpt.h"

#include <vc/snx_vc.h>
#include <isp/isp.h>
#include <libmid_vc/snx_mid_vc.h>


typedef struct uvc_video_info{
	unsigned int width;
	unsigned int height;
	unsigned char ucIspChannel;
	unsigned char ucFps;
	unsigned char ucScale;
	unsigned char ucDup;
	unsigned char ucStreamMode;
	unsigned char ucStrameOn;
	unsigned short ucGop;
	unsigned int uiBps;	//bits per second
	struct snx_m2m m2m;
} uvc_video_info_t;

enum uvc_stream_state {
  UVC_STREAM_OFF = 0,
  UVC_STREAM_ON,
  UVC_STREAM_CLOSING,
};

static uvc_video_info_t VideoINFO = {0};


void snx_usbd_uvc_vc_task(void *pvParameters)
{
	uint32_t g_imgsize = 0;
	uint8_t *g_imgbuf = NULL;

	while (VideoINFO.ucStrameOn == UVC_STREAM_ON) {
		if(VideoINFO.ucStreamMode & FMT_H264) {
			snx_video_read(&VideoINFO.m2m);
			g_imgsize = snx_video_h264_stream(&VideoINFO.m2m, VideoINFO.ucDup, (unsigned int *)&g_imgbuf);
		} else if(VideoINFO.ucStreamMode & FMT_MJPEG) {
			snx_video_read(&VideoINFO.m2m);
			g_imgsize = snx_video_jpeg_stream(&VideoINFO.m2m, VideoINFO.ucDup, (unsigned int *)&g_imgbuf);
		} else if(VideoINFO.ucStreamMode & FMT_SNX420) {
			snx_isp_read(&VideoINFO.m2m);
			g_imgbuf = VideoINFO.m2m.ctx[VideoINFO.m2m.channel].userptr;
			g_imgsize = VideoINFO.m2m.ctx[VideoINFO.m2m.channel].size;
		}

		if (g_imgsize != 0) {
			usbd_uvc_drv_send_image(g_imgbuf, g_imgsize);
		}
	}
	VideoINFO.ucStrameOn = UVC_STREAM_OFF;
	vTaskDelete(NULL);
}


void snx_usbd_uvc_start_preview(struct preview_image_info* img)
{
	print_msg("UVC preview start\n");
	if(img->type == UVC_VS_FORMAT_MJPEG) {
		VideoINFO.ucStreamMode = FMT_MJPEG;
	} else if (img->type == UVC_VS_FORMAT_UNCOMPRESSED) {
		VideoINFO.ucStreamMode = FMT_SNX420;
	} else {
		VideoINFO.ucStreamMode = FMT_H264;
	}
	VideoINFO.width = img->width;
	VideoINFO.height = img->height;
	VideoINFO.ucFps = img->fps;
	VideoINFO.ucScale = 1;
	VideoINFO.ucIspChannel = 0;
	VideoINFO.ucGop = 0;
	VideoINFO.ucDup = 0;

	// Set H264 bit-rate
	if ((VideoINFO.width >= UVC_FHD_WIDTH) && (VideoINFO.height >= UVC_FHD_HEIGHT)) {
		VideoINFO.uiBps = UVC_FHD_H264_BPS;
	} else if ((VideoINFO.width >= UVC_HD_WIDTH) && (VideoINFO.height >= UVC_HD_HEIGHT)) {
		VideoINFO.uiBps = UVC_HD_H264_BPS;
	} else {
		VideoINFO.uiBps = UVC_VGA_H264_BPS;
	}

	memset(&VideoINFO.m2m, 0x0, sizeof(struct snx_m2m));

	if (VideoINFO.ucStreamMode & FMT_SNX420) {
		VideoINFO.m2m.isp_mode = VIDEO_PIX_FMT_SBGGR10;	// 10-bits raw data.
		VideoINFO.m2m.vc.mode = 0;
		VideoINFO.m2m.isp_bufs = 2;
		VideoINFO.m2m.channel = VideoINFO.ucIspChannel;
		snx_video_set_resolution(&VideoINFO.m2m, VideoINFO.width, VideoINFO.height);
		snx_isp_set_fps(&VideoINFO.m2m, VideoINFO.ucFps);

		snx_isp_start(&VideoINFO.m2m);
	} else {
		snx_video_init(&VideoINFO.m2m);
		// set video
		VideoINFO.m2m.channel = VideoINFO.ucIspChannel; //isp channel
		snx_video_set_resolution(&VideoINFO.m2m, VideoINFO.width, VideoINFO.height);
		snx_isp_set_fps(&VideoINFO.m2m, VideoINFO.ucFps);
		snx_video_set_scale(&VideoINFO.m2m, VideoINFO.ucScale);
		snx_set_mdrc_en(&VideoINFO.m2m, VideoINFO.ucDup, 0);	// disable motion detection rate control
		snx_set_mdcnt_en(&VideoINFO.m2m, VideoINFO.ucDup, 0);	// disable auto low bitrate control
		snx_video_set_mode(&VideoINFO.m2m, VideoINFO.ucDup, VideoINFO.ucStreamMode);
		if(VideoINFO.ucStreamMode & FMT_H264) {
			snx_video_set_fps(&VideoINFO.m2m, VideoINFO.ucDup, VideoINFO.ucFps, FMT_H264);
			snx_video_set_bps(&VideoINFO.m2m, VideoINFO.ucDup, VideoINFO.uiBps);
			snx_set_mbrc_en(&VideoINFO.m2m, VideoINFO.ucDup, 0);
			snx_video_set_percent(&VideoINFO.m2m, VideoINFO.ucDup, FMT_H264, (VideoINFO.uiBps * 100) / (VideoINFO.width * VideoINFO.height * 12));
		} else if(VideoINFO.ucStreamMode & FMT_MJPEG) {
			snx_video_set_fps(&VideoINFO.m2m, VideoINFO.ucDup, VideoINFO.ucFps, FMT_MJPEG);
			snx_video_set_qp(&VideoINFO.m2m, VideoINFO.ucDup, 32, FMT_MJPEG);
			snx_video_set_percent(&VideoINFO.m2m, VideoINFO.ucDup, FMT_MJPEG, 25);	// suppose QP > 10
		}

		snx_video_set_gop(&VideoINFO.m2m, VideoINFO.ucDup, 0); // fps == gop
		snx_video_start(&VideoINFO.m2m);
	}
	VideoINFO.ucStrameOn = UVC_STREAM_ON;

	/* Reset MP Tool AF report OSD position */
	mpt_osd_af_report_set_position();

	// Task Init
	if (xTaskCreate(snx_usbd_uvc_vc_task, (const char *)"MPTool_UVC_VC_TASK", STACK_SIZE_1K, NULL, PRIORITY_TASK_DRV_USBD_PROCESS, NULL) != pdPASS) {
		print_msg("Create MPTool UVC Video Capture task fail\n");
	}
}

void snx_usbd_uvc_stop_preview()
{
	print_msg("UVC preview stop\n");
	if(VideoINFO.ucStrameOn == UVC_STREAM_ON){
		VideoINFO.ucStrameOn = UVC_STREAM_CLOSING;
		while (VideoINFO.ucStrameOn != UVC_STREAM_OFF) {
			vTaskDelay(10/portTICK_PERIOD_MS);
		}
		if (VideoINFO.ucStreamMode == FMT_SNX420) {
			snx_isp_stop(&VideoINFO.m2m);
		} else {
			snx_video_stop(&VideoINFO.m2m);
			snx_video_uninit(&VideoINFO.m2m);
		}
	}
	VideoINFO.ucStrameOn = UVC_STREAM_OFF;
}

void snx_usbd_uvc_vc_init(void)
{
	memset(&VideoINFO, 0, sizeof(uvc_video_info_t));
	usbd_mid_uvc_start_preview_reg_cb(snx_usbd_uvc_start_preview);
	usbd_mid_uvc_stop_preview_reg_cb(snx_usbd_uvc_stop_preview);
}

// ==== Other control functions ===== //
void snx_usbd_uvc_set_bitrate(int bps)
{
	snx_video_set_bps(&VideoINFO.m2m, VideoINFO.ucDup, bps);
}

int snx_usbd_uvc_get_bitrate(void)
{
	return snx_video_get_bps(&VideoINFO.m2m, VideoINFO.ucDup);
}

void snx_usbd_uvc_get_cur_img_info(int *width, int *height)
{
	if (VideoINFO.ucStrameOn == UVC_STREAM_ON) {
		*width = VideoINFO.width;
		*height = VideoINFO.height;
	}
}
