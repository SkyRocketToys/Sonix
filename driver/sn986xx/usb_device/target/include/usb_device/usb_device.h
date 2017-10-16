/** 
* @file 
* this is usb device driver file 
* usb_device.h 
* @author IP2/Luka 
*/
#ifndef __USB_DEVICE_H
#define __USB_DEVICE_H


#include <FreeRTOS.h>
#include <bsp.h>
#include <task.h>
#include <interrupt.h>
#include <semphr.h>
#include "ch9.h"

#ifdef __cplusplus
extern "C"{
#endif


//#define USBD_PRINT(fmt, args...) print_msg_queue("[usbd] "fmt, ##args)
//#define USBD_PRINT(fmt, args...) print_msg("[usbd] "fmt, ##args)
#define USBD_PRINT(fmt, args...)


typedef enum{
	USBD_MODE_MSC = 0,
	USBD_MODE_UVC,
	USBD_MODE_HID,	
	USBD_MODE_UNKNOWN = 255,
} usbd_class_mode_t;


typedef enum{
	USBD_POWER_OFF = 0,
	USBD_POWER_ON,
} usbd_power_mode_t;


#define USBD_DEF_CLASS_MODE					USBD_MODE_MSC

#define USBD_PKG							"USB_Device"
#define USBD_CFG_CLASS_MODE					"class_mode"
#define USBD_CFG_HOTPLUG_MODE				"hotplug_mode"
#define USBD_CFG_HOTPLUG_GPIO_NUM			"hotplug_gpio_num"
#define USBD_CFG_HOTPLUG_GPIO_TRIG_LEV		"hotplug_gpio_trig_lev"


#define true 	1
#define false	0

/* type define */
typedef void (*usbd_drv_connect_cb_t)(void);
typedef void (*usbd_drv_disconnect_cb_t)(void);
typedef void (*usbd_drv_reset_cb_t)(void);
typedef void (*usbd_drv_suspend_cb_t)(void);
typedef void (*usbd_drv_resume_cb_t)(void);
typedef int (*usbd_drv_vendor_cmd_cb_t)(struct usb_ctrlrequest *ctrl);
typedef void (*usbd_drv_plugin_cb_t)(void);
typedef void (*usbd_drv_plugout_cb_t)(void);

/* USB device callback functions */
void usbd_drv_connect_reg_cb(usbd_drv_connect_cb_t cb);
void usbd_drv_disconnect_reg_cb(usbd_drv_disconnect_cb_t cb);
void usbd_drv_reset_reg_cb(usbd_drv_reset_cb_t cb);
void usbd_drv_suspend_reg_cb(usbd_drv_suspend_cb_t cb);
void usbd_drv_resume_reg_cb(usbd_drv_resume_cb_t cb);
void usbd_drv_vendor_cmd_reg_cb(usbd_drv_vendor_cmd_cb_t cb);
void usbd_drv_plugin_reg_cb(usbd_drv_plugin_cb_t cb);
void usbd_drv_plugout_reg_cb(usbd_drv_plugout_cb_t cb);


/* USB device functions */
void USBDClassDrvInit(void);
void usbd_drv_task_init(void);
void usbd_drv_task_uninit(void);
int usbd_get_class_mode(void);
void usbd_set_class_mode(int mode);
inline void usbd_set_ext_hotplug_state(int state);
void usbd_set_power(int mode);
int usbd_vendor_cmd_queue(uint32_t length, uint8_t *buf);


//#define cpu_to_le16(x) ((__force __le16)(__u16)(x))
#define ESUCCESS        0 		/* success */
#define EDOM            33      /* Math argument out of domain of func */
#define ENOMEM          12      /* Out of Memory */
#define EINVAL          22      /* Invalid argument */
#define ENOSPC          28      /* No space left on device */
#define EOPNOTSUPP      95      /* Operation not supported on transport endpoint */
#define EBUSY           16      /* Device or resource busy */
#define ENODEV          19      /* No such device */
#define EALREADY        114     /* Operation already in progress */
#define EINPROGRESS     115     /* Operation now in progress */
#define ENOTSUPP        524     /* Operation is not supported */
#define EPROTO          71      /* Protocol error */
#define ECONNABORTED    103     /* Software caused connection abort */
#define ESHUTDOWN       108     /* Cannot send after transport endpoint shutdown */
#define ECONNRESET      104     /* Connection reset by peer */

#define	EIO				5	/* I/O error */
#define EOVERFLOW       75      /* Value too large for defined data type */
#define	EAGAIN			11	/* Try again */

#define	EINTR			4	/* Interrupted system call */
	

#ifdef __cplusplus
}
#endif


#endif

