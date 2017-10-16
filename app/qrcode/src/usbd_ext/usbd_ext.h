#ifndef __USBD_EXT_H__
#define __USBD_EXT_H__


#if 1
#define USBD_EXT_PRINT(fmt, args...) print_msg("[usbd-ext] "fmt, ##args)
#else
#define USBD_EXT_PRINT(fmt, args...)
#endif


#define USBD_OUTFORMAT_MJPEG             1        //0:H264 1:MJPEG
#define USBD_CLASS_ISPTUNINGTOOL        (1<<0)
#define USBD_CLASS_MPTOOL               (1<<1)
#define USBD_CLASS_DOWNLOADTOOL         (1<<2)

#define PREV_SUPPORT_MAX_WIDTH         1280      //for uvc preview support max       
#define PREV_SUPPORT_MAX_HEIGHT        720
#define PREV_SUPPORT_MAX_FPS           30

#define USBD_UVC_XU_USR_ENABLE			0
enum{
	CHANNEL_RECORD = 0,		/**<  isp channel for record*/
	CHANNEL_PREVIEW = 1,	/**<  isp channel for preview*/
};

/* This USBD_UVC_MCTRL macro define for car camera project. 2016/07/20
 * 0: WiFi(Mobile) is key control, If WiFi application change video format or resolution.
 * Then UVC will be follow it. But USB host can't control change format and resolution.
 * 1:  USB host is key control, If USB host application change video format or resolution.
 * Then WiFi will be follow it.
 * */
#define USBD_UVC_MCTRL				0

uint8_t usbd_app_IsPlugin(void);
void usbd_app_uvc_reconnect(void);

void usbd_ext_init(void);
void usbd_ext_uninit(void);


#endif
