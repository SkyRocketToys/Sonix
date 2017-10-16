#ifndef __MID_USBD_H__
#define __MID_USBD_H__


#include <usb_device/usbd_msc.h>
#include <usb_device/usbd_uvc.h>
#include <usb_device/usbd_hid.h>


#if 0
#define USBD_MID_PRINT(fmt, args...) print_msg("[usbd-mid] "fmt, ##args)
#define USBD_MID_PRINT_QUEUE(fmt, args...) print_msg_queue("[usbd-mid] "fmt, ##args)
#else
#define USBD_MID_PRINT(fmt, args...)
#define USBD_MID_PRINT_QUEUE(fmt, args...)
#endif


/* SONiX USB Device Options */
#define SNX_USBD_OPTION_NONE                    (0)
#define SNX_USBD_OPTION_ISPTUNINGTOOL           (1<<0)
#define SNX_USBD_OPTION_MPTOOL                  (1<<1)
#define SNX_USBD_OPTION_DOWNLOADTOOL            (1<<2)
#define SNX_USBD_OPTION_TOOL_CLASS            	(SNX_USBD_OPTION_ISPTUNINGTOOL |  SNX_USBD_OPTION_MPTOOL | SNX_USBD_OPTION_DOWNLOADTOOL)
#define SNX_USBD_OPTION_UVC_RES_CHANGE      	(1<<7)

typedef enum{
	USBD_MID_STATE_INIT = 0,
	USBD_MID_STATE_CONNECT,
	USBD_MID_STATE_DISCONNECT,
	USBD_MID_STATE_RESET,
	USBD_MID_STATE_SUSPEND,
	USBD_MID_STATE_RESUME,
} usbd_mid_state_t;

typedef enum{
	USBD_MID_STATE_PLUG_OUT = 0,
	USBD_MID_STATE_PLUG_IN_ADAPTER,
	USBD_MID_STATE_PLUG_IN_HOST,
} usbd_mid_hotplug_state_t;


typedef enum{
	USBD_MID_MAINFLOW_NOTYET = 0,
	USBD_MID_MAINFLOW_RUNNING,
	USBD_MID_MAINFLOW_DONE,
} usbd_mid_mainflow_state_t;


typedef struct usb_device_desc_info {
	struct uvc_device_desc_info *usbd_uvc_info;
	struct msc_device_desc_info *usbd_msc_info;
	struct hid_device_desc_info *usbd_hid_info;
} usb_device_desc_info_t;


typedef struct usbd_class_switch_param {
	int cs_mode;
	unsigned int options;
} usbd_class_switch_param_t;



/* type define */
typedef void (*usbd_mid_connect_cb_t)(void);
typedef void (*usbd_mid_disconnect_cb_t)(void);
typedef void (*usbd_mid_reset_cb_t)(void);
typedef void (*usbd_mid_suspend_cb_t)(void);
typedef void (*usbd_mid_resume_cb_t)(void);
typedef void (*usbd_mid_plugin_cb_t)(void);
typedef void (*usbd_mid_plugout_cb_t)(void);
typedef void (*usbd_mid_remove_maintask_cb_t)(void);
typedef void (*usbd_mid_restore_maintask_cb_t)(void);
typedef uint8_t (*usbd_mid_uvc_sys_xu_mpt_cb_t)(uint8_t uvc_req);
typedef uint8_t (*usbd_mid_uvc_usr_xu_cb_t)(uint8_t uvc_cs, uint8_t uvc_req);
typedef void (*usbd_mid_class_switch_prev_notice_cb_t)(int32_t cs_mode, uint32_t options);
typedef void (*usbd_mid_uvc_start_preview_cb_t)(struct preview_image_info *img);
typedef void (*usbd_mid_uvc_stop_preview_cb_t)(void);
typedef void (*usbd_mid_uvc_switch_preview_resource_cb_t)(void);
typedef void (*usbd_mid_msc_ready_to_upgrade_cb_t)(void);
typedef void (*usbd_mid_hid_data_err_cb_t)(void);


/* USB device callback functions */
void usbd_event_connect_reg_cb(usbd_mid_connect_cb_t cb);
void usbd_event_disconnect_reg_cb(usbd_mid_disconnect_cb_t cb);
void usbd_event_reset_reg_cb(usbd_mid_reset_cb_t cb);
void usbd_event_suspend_reg_cb(usbd_mid_suspend_cb_t cb);
void usbd_event_resume_reg_cb(usbd_mid_resume_cb_t cb);
void usbd_event_plugin_reg_cb(usbd_mid_plugin_cb_t cb);
void usbd_event_plugout_reg_cb(usbd_mid_plugout_cb_t cb);
void usbd_event_remove_maintask_reg_cb(usbd_mid_remove_maintask_cb_t cb);
void usbd_event_restore_maintask_reg_cb(usbd_mid_restore_maintask_cb_t cb);
void usbd_mid_uvc_sys_xu_mpt_reg_cb(usbd_mid_uvc_sys_xu_mpt_cb_t cb);
void usbd_mid_uvc_usr_xu_reg_cb(usbd_mid_uvc_usr_xu_cb_t cb);
void usbd_event_class_switch_prev_notice_reg_cb(usbd_mid_class_switch_prev_notice_cb_t cb);
void usbd_mid_uvc_start_preview_reg_cb(usbd_mid_uvc_start_preview_cb_t cb);
void usbd_mid_uvc_stop_preview_reg_cb(usbd_mid_uvc_stop_preview_cb_t cb);
void usbd_mid_msc_ready_to_upgrade_cb(usbd_mid_msc_ready_to_upgrade_cb_t cb);
void usbd_mid_hid_data_err_event_reg_cb(usbd_mid_hid_data_err_cb_t cb);


/* usb device function */
void usbd_mid_set_class_mode(int mode, unsigned int opt);
int usbd_mid_get_class_mode(void);
void usbd_mid_switch_cs_task(void *pvParameters);
usbd_mid_hotplug_state_t usbd_mid_get_hotplug_state(void);
int usbd_mid_class_code2mode(unsigned char code);
void usbd_mid_init(usb_device_desc_info_t *usbd_desc_info);
void usbd_mid_uninit(void);


/* UVC functions */
uint32_t usbd_mid_uvc_read_data(uint8_t* buf, uint32_t* len);
uint32_t usbd_mid_uvc_write_data(uint8_t* buf, uint32_t len);
void usbd_mid_uvc_events_start_preview(struct preview_image_info *img);
void usbd_mid_uvc_events_stop_preview(void);
void usbd_mid_uvc_switch_preview_resource(void);
void usbd_mid_uvc_vc_entry(void);
void usbd_mid_uvc_set_max_payload_size(unsigned int size);



/* MSC functions */


/* HID functions */
void usbd_mid_hid_set_report_param(uint16_t rp_time, uint8_t bypass_lut);
void usbd_mid_hid_send_string(uint8_t* buf, uint32_t len);

#endif	
