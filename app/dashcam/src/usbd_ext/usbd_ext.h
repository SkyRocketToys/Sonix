#ifndef __USBD_EXT_H__
#define __USBD_EXT_H__


#if 1
#define USBD_EXT_PRINT(level, fmt, args...) print_q(level, "[video]%s: "fmt, __func__,##args)
#else
#define USBD_EXT_PRINT(fmt, args...)
#endif


#define USBD_OUTFORMAT_MJPEG             0        //0:H264 1:MJPEG
#define USBD_CLASS_ISPTUNINGTOOL        (1<<0)
#define USBD_CLASS_MPTOOL               (1<<1)
#define USBD_CLASS_DOWNLOADTOOL         (1<<2)

#define PREV_SUPPORT_MAX_WIDTH         1280      //for uvc preview support max       
#define PREV_SUPPORT_MAX_HEIGHT        720
#define PREV_SUPPORT_MAX_FPS           30

#define USBD_UVC_XU_USR_ENABLE			1
#define UVC_USBH_TO_USBD_SERIES_CONNECTION_ENABLE	0

uint8_t usbd_app_IsPlugin(void);
void usbd_app_uvc_reconnect(void);

void usbd_ext_init(void);
void usbd_ext_uninit(void);


#endif
