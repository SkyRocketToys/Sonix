/**
 * @file
 * this is application header file for video data capture, include this file before use
 * @author Algorithm Dept Sonix.
 */
#ifndef __MF_VIDEO_H__
#define __MF_VIDEO_H__


#include <isp/isp.h>
#include <vc/snx_vc.h>
#include <libmid_vc/snx_mid_vc.h>
#include <timestamp_osd/timestamp_osd.h>
#include <generated/snx_sdk_conf.h>
#include "rec_common.h"

#if 0
#define VDO_PRINT(level, fmt, args...) print_q(level, "[video]%s(%u): "fmt, __func__,__LINE__,##args)
#else
#define __MODULE__ "video_main"
#define PRINT_Q(level, fmt, args...) print_q(level, "[%s]: %s(%u): "fmt, __MODULE__, __func__, __LINE__, ##args)
#define VDO_PRINT(level, fmt, args...) PRINT_Q(level, fmt, ##args)
#endif


typedef void (*h264_notice_t)(unsigned char, unsigned char *, unsigned int, struct timeval);
typedef void (*mj_notice_t)(unsigned char *, unsigned int, const char *, unsigned int);
typedef void (*usbd_video_t)(unsigned char *image, unsigned int img_size);

#define LIVE_PREVIEW_STREAM_NAME	"media/stream2"
#define LIVE_PREVIEW_STREAM_Head	"media/"
#define LIVE_PREVIEW_STREAM_Body	"stream2"

#define NVRAM_PKG_VIDEO_ISP         	"App_ISP"
#define NVRAM_CFG_VIDEO_MIRROR      	"mirror"
#define NVRAM_CFG_VIDEO_FLIP        	"flip"
#define NVRAM_CFG_VIDEO_WDR         	"wdr"
#define NVRAM_CFG_VIDEO_POWERFREQUENCY  "powerfrequency"
#define NVRAM_CFG_VIDEO_GSENSORSENSITIVITY    "gsensorsensitivity"
#define NVRAM_CFG_VIDEO_OSD             "osd"
#define NVRAM_CFG_PREVIEW_VIDEO_FPS  	"preview_fps"
#define NVRAM_CFG_PREVIEW_VIDEO_GOP  	"preview_gop"
#define NVRAM_CFG_PREVIEWVIDEO_BPS   	"preview_bps"
#define NVRAM_CFG_PREVIEWVIDEO_WIDTH  	"preview_width"
#define NVRAM_CFG_PREVIEWVIDEO_HEIGHT  	"preview_height"
#define NVRAM_CFG_PREVIEW_EXT_PFRAME_NUM  	"preview_ext_pframe_num"
#define NVRAM_CFG_PREVIEW_EXT_QP_RANGE  	"preview_ext_qp_range"
#define NVRAM_CFG_PREVIEW_EXT_QP_MAX  	"preview_ext_qp_max"
#define NVRAM_CFG_PREVIEW_EXT_UPPER_PFRAME  	"preview_ext_upper_pframe"
#define NVRAM_CFG_PREVIEW_EXT_UPPER_PFRAME_DUP1  	"preview_ext_upper_pframe_dup1"
#define NVRAM_CFG_PREVIEW_QP_MAX  	"preview_qp_max"
#define NVRAM_CFG_PREVIEW_QP_MIN  	"preview_qp_min"
#define NVRAM_CFG_PREVIEW_RESUME  	"preview_resume"
#define NVRAM_CFG_PREVIEW_SUSPEND  	"preview_suspend"


#define NVRAM_CFG_RECORD_VIDEO_FPS      "record_fps"
#define NVRAM_CFG_RECORD_VIDEO_GOP      "record_gop"
#define NVRAM_CFG_RECORD_BPS            "record_bps"
#define NVRAM_CFG_RECORD_WIDTH          "record_width"
#define NVRAM_CFG_RECORD_HEIGHT         "record_height"
#define NVRAM_CFG_RECORD_EXT_PFRAME_NUM  	"record_ext_pframe_num"
#define NVRAM_CFG_RECORD_EXT_QP_RANGE  	"record_ext_qp_range"
#define NVRAM_CFG_RECORD_EXT_QP_MAX  	"record_ext_qp_max"
#define NVRAM_CFG_RECORD_EXT_UPPER_PFRAME  	"record_ext_upper_pframe"
#define NVRAM_CFG_RECORD_QP_MAX  	"record_qp_max"
#define NVRAM_CFG_RECORD_QP_MIN  	"record_qp_min"

#define DASGCAM_DEF_ISP_MIRROR		1
#define DASGCAM_DEF_ISP_FLIP		1
#define DASGCAM_DEF_ISP_WDR			1
#define DASGCAM_DEF_ISP_POWERFREQUENCY			50


#define FHD_H264_BPS                 (10 * 1024 * 1024)
#define HD_H264_BPS                  (4.5 * 1024 * 1024)
#define VGA_H264_BPS                 (1 * 1024 * 1024)
#define PREV_HD_H264_BPS             (2 * 1024 * 1024)
#define PREV_VGA_H264_BPS            (1 * 500 * 1024)

#ifdef CONFIG_DISABLE_VIDEO_WIFI_OPEN
#define RTSP_CHECK_LEGAL CONFIG_DISABLE_VIDEO_WIFI_OPEN
#else
#define RTSP_CHECK_LEGAL       		1       /**<  0:disable wifi password check. */
#endif       								/**<  1:enable wifi password check. */


#define FMT_H264				(0x1 << 1)
#define FMT_MJPEG				(0x1 << 2)
#define MASTER_PATH				0
#define DUP_PATH				1


/**
* @brief enum for isp channel
*/
enum {
	CHANNEL_RECORD = 0,		/**<  isp channel for record*/
	CHANNEL_PREVIEW = 1,	/**<  isp channel for preview*/
};


/**
* @brief structure for video infomation
*/
typedef struct _video_info {
	unsigned int width, height;		/**<  video resolution*/
	unsigned char ucIspChannel;		/**<  isp channel(start from 0)*/
	unsigned char ucFps;			/**<  frame rate*/
	unsigned char ucScale;			/**<  scale: X1=0, X2=1, X4=2 for duplicate path*/
	unsigned char ucDupEnable; 			/**<  master path=0, duplicat path=1*/
	unsigned char ucStreamMode; 	/**<  H.264=2, MJPG=4*/
	unsigned char ucStreamModeusedup1;
	unsigned char ucStrameOn;		/**<  video task on working or not*/
	unsigned short ucGop;			/**<  GOP: 0~255*/
	unsigned int uiBps;				/**<  bit rate*/

	unsigned int time_interval;				/**<  video info time interval */

	char schedthumbnailname[LEN_FILENAME];
	char timelapsethumbnailname[LEN_FILENAME];
	char protectthumbnailname[LEN_FILENAME];
	//unsigned int useSensorHD60mode;
	//int iGop;
	xTaskHandle task_get;  			/**<  task to capture video*/
	xTaskHandle task_handlepreview;
	xQueueHandle previewdataqueue;
	struct snx_m2m m2m;				/**<  structure for video middleware*/
	h264_notice_t h264_notice;		/**<  function pointer to get h.264 frame data*/
	h264_notice_t h264_notice_dup1; /**<  function pointer to get h.264 frame data*/
	h264_notice_t h264_timelapse;   /**<  function pointer to get h.264 frame data*/
	mj_notice_t mj_notice;			/**<  function pointer to get mjpg data*/
	osd_ds_info_t ds_info;			/**<  strcuture for OSD information*/
	unsigned int taskclose_reboottask; /**taskclose and reboot task*/
	usbd_video_t usbd_vdo2uvc;		/**<  callback function pointer to get video data to UVC device */
} video_info_t;

/**
* @brief structure to report current video infomation
*/
typedef struct _video_param {
	unsigned int width, height;
	unsigned char ucFps;
	unsigned int  uiBps;
	unsigned char ucScale;
	unsigned char ucStreamMode;
} video_param_t;

/**
*@brief structure to set app(user)setting param
*/
typedef struct _video_user_param {
	int user_setting_flag;
	unsigned int current_width;
	unsigned int current_height;
	unsigned char current_ucFps;
	unsigned int current_uiBps;
	unsigned int current_ucGop;
	unsigned int usersetting_width;
	unsigned int usersetting_height;
	unsigned char usersetting_ucFps;
	unsigned int usersetting_uiBps;
	unsigned int usersetting_ucGop;
} video_user_param_t;


typedef union {
	struct {
		unsigned char sdcardexist:       1;
		unsigned char recordruning:      1;
		unsigned char sdsizeistoosmall:  1;
		unsigned char sdformatisfat32 : 1;
#if RTSP_CHECK_LEGAL
		unsigned char legal_preview:	1;
#else
		unsigned char bit4 : 1;
#endif
		unsigned char bit5 : 1;
		unsigned char bit6 : 1;
		unsigned char bit7 : 1;
	} bits;
	unsigned char frameadddata;
} app_frameadddata ;


int mf_video_init(void);
void mf_video_uninit(void);
int mf_set_snapshot(unsigned char ucEnable);
void mf_set_preview(unsigned char ucEnable);
void mf_set_record(unsigned char ucEnable);
void mf_video_get_rec_param(video_param_t *pv_param);
void mf_video_set_record_cb(h264_notice_t notice_function);
void mf_video_set_preview_cb(h264_notice_t notice_function);
void mf_video_set_shapshot_cb(mj_notice_t notice_function);
void mf_video_set_usbd_uvc_image_cb(usbd_video_t cb, int ch);
void osd_preview_is_dup1_setting();
void osd_preview_is_isp1_setting();
void mf_video_h264_set_iframe(void);
void mf_video_h264_set_iframe_ispdup(void);
void mf_video_h264_set_iframe_for_record(void);
void send_preview_to_rtp(unsigned char IFrame, unsigned char *pFrame, unsigned int uiFrameSize, struct timeval tval);
void preview_video_set_bps(unsigned int usersetting_uiBps);
void preview_video_set_fps(unsigned char usersetting_ucFps);
void preview_video_set_resolution(int width, int height);
void preview_video_set_allparam(int width, int height, unsigned char usersetting_ucFps, unsigned int usersetting_uiBps);
void set_takepic_num(int num);

int check_takepic_task(void);
int video_stream_preview_init(void);
int video_stream_rec_init(void);
int video_stream_thumbnail_init(void);
int chk_preview_use_isp0dup(void);
void enable_isp0dup1_preview(void);
void getnvram_videopreview_param(void);
void check_width_height(video_info_t *pinfo);
void mf_set_uvc_view(unsigned char ucEnable);
void mf_video_set_timelapse_cb(h264_notice_t notice_function);
void mf_video_set_snapshot_cb(mj_notice_t notice_function);
mj_notice_t mf_video_get_snapshot_cb(void);
void mf_video_set_thumbnail_cb(mj_notice_t notice_function);
int mf_set_thumbnail_fordup(const char *thumbnailname, enum RECORDKIND type);
int mf_video_stream_ispch0_init(void);
int mf_video_stream_ispch1_init(void);


void mf_video_set_preview_cb(h264_notice_t notice_function);
void mf_video_set_record_cb(h264_notice_t notice_function);
void mf_video_set_record_dup1_cb(h264_notice_t notice_function);
void mf_video_set_timelapse_cb(h264_notice_t notice_function);


void disable_isp0dup1_preview(video_info_t *pinfo);

void set_thumbnail_pic(video_info_t *pVInfo, unsigned char *pFrameaddr, unsigned int uiFrameSize);
void check_thumbnail_isp1_userparam(video_info_t *pVInfo);
void check_record_video_isp_userparam(video_info_t *pVInfo);
void check_preview_video_isp_userparam(video_info_t *pVInfo);



extern int takepic_num;
extern int snapshot_onepic;

void task_video_get(void *pvParameters);
void task_video_duppreview(void *pvParameters);
void task_video_get_thumbnail(void *pvParameters);
int video_set_init(video_info_t *pinfo);
void video_set_uninit(video_info_t *pinfo);

#endif
