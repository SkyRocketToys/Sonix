/*
*	File: usbd_uvc_vc.h
*	Version: 1.0
*	Author: chkuo
*	Date: 2016-01-14
*	Descriptions: SONiX USB UVC Device Video Capture header file.
*/

#ifndef __USBD_UVC_VC_H__
#define __USBD_UVC_VC_H__



#ifdef __cplusplus
extern "C"{
#endif


#define UVC_FHD_WIDTH                   1920
#define UVC_FHD_HEIGHT                  1080
#define UVC_HD_WIDTH                    1280
#define UVC_HD_HEIGHT                   720
#define UVC_VGA_WIDTH                   640
#define UVC_VGA_HEIGHT                  480

#define UVC_FHD_H264_BPS                 (10 * 1024 * 1024)
#define UVC_HD_H264_BPS                  (4.5 * 1024 * 1024)
#define UVC_VGA_H264_BPS                 (1 * 1024 * 1024)


// Variable



// Functions
void snx_usbd_uvc_vc_init(void);
void snx_usbd_uvc_set_bitrate(int bps);
int snx_usbd_uvc_get_bitrate(void);
void snx_usbd_uvc_get_cur_img_info(int *width, int *height);
void snx_usbd_uvc_start_preview(struct preview_image_info* img);
void snx_usbd_uvc_stop_preview();


#ifdef __cplusplus
}
#endif


#endif /*__USBD_UVC_VC_H__*/
