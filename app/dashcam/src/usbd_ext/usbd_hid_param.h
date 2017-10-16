/*
*	File: usbd_hid_param.h
*	Version: 1.0
*	Author: luka
*	Date: 2016-07-28
*	Descriptions: SONiX HID Device parameter
*/

#ifndef __USBD_HID_PARAM_H__
#define __USBD_HID_PARAM_H__

#include <generated/snx_sdk_conf.h>
#include <usb_device/usb_device.h>
#include <usb_device/usbd_hidh>


#ifdef __cplusplus
extern "C"{
#endif


/* ------------------------------------------------------------------------
 * UVC Parameter
 */
#define USBD_HID_VENDOR_ID              0x0C45	/* Sonix Technology Co. */
#define USBD_HID_PRODUCT_ID             0x8220	/* SN98660 HID */
#define USBD_HID_DEVICE_BCD             0x0100	/* 1.00 */

static unsigned short hid_vendor_id = USBD_HID_VENDOR_ID;
static unsigned short hid_product_id = USBD_HID_PRODUCT_ID;
static unsigned short hid_bcdDevice = USBD_HID_DEVICE_BCD;

static char hid_vendor_str[]    = "Sonix Technology Co., Ltd.";
static char hid_product_str[]   = "USB2.0 HID class";
static char hid_config_str[]    = "Sonix HID";


#ifdef CONFIG_MODULE_USB_DEVICE_SELFPOWER
	static unsigned char hid_attributes = 0x40;
#else
	static unsigned char hid_attributes = 0x00;
#endif

#ifdef CONFIG_MODULE_USB_DEVICE_VBUS_DRAW
	static unsigned char hid_maxpower   = (CONFIG_MODULE_USB_DEVICE_VBUS_DRAW/2);
#endif


static struct hid_device_desc_info hid_desc_info = {
	.idVendor = &hid_vendor_id,
	.idProduct = &hid_product_id,
	.bcdDevice = &hid_bcdDevice,
	.strVendor = hid_vendor_str,
	.StrProduct = hid_product_str,
	.StrConfig = hid_config_str,

	.bmAttributes = &hid_attributes,
	.bMaxPower = &hid_maxpower,

};


#ifdef __cplusplus
}
#endif


#endif /*__USBD_HID_PARAM_H__*/
