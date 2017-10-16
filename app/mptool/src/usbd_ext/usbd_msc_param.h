/*
*	File: usbd_msc_param.h
*	Version: 1.0
*	Author: chkuo
*	Date: 2016-01-11
*	Descriptions: SONiX MSC Device parameter
*/

#ifndef __USBD_MSC_PARAM_H__
#define __USBD_MSC_PARAM_H__

#include <generated/snx_sdk_conf.h>
#include <usb_device/usb_device.h>
#include <usb_device/usbd_msc.h>


#ifdef __cplusplus
extern "C"{
#endif


/* ------------------------------------------------------------------------
 * UVC Parameter
 */
#define USBD_MSC_VENDOR_ID              0x0C45	/* Sonix Technology Co. */
#define USBD_MSC_PRODUCT_ID             0x8220	/* SN98660 MSC */
#define USBD_MSC_DEVICE_BCD             0x0100	/* 1.00 */

static unsigned short msc_vendor_id = USBD_MSC_VENDOR_ID;
static unsigned short msc_product_id = USBD_MSC_PRODUCT_ID;
static unsigned short msc_bcdDevice = USBD_MSC_DEVICE_BCD;

static char msc_vendor_str[]    = "Sonix Technology Co., Ltd.";
static char msc_product_str[]   = "USB2.0 Mass Storage";
static char msc_config_str[]    = "Sonix Storage";

static char msc_vendor_name[8]      = "SONiX   ";
static char msc_product_name[16]    = "USB MASS STORAGE";
static char msc_release[4]          = "0.01";

#ifdef CONFIG_MODULE_USB_DEVICE_SELFPOWER
	static unsigned char msc_attributes = 0x40;
#else
	static unsigned char msc_attributes = 0x00;
#endif

#ifdef CONFIG_MODULE_USB_DEVICE_VBUS_DRAW
	static unsigned char msc_maxpower   = (CONFIG_MODULE_USB_DEVICE_VBUS_DRAW/2);
#endif


static struct msc_device_desc_info msc_desc_info = {
	.idVendor = &msc_vendor_id,
	.idProduct = &msc_product_id,
	.bcdDevice = &msc_bcdDevice,
	.strVendor = msc_vendor_str,
	.StrProduct = msc_product_str,
	.StrConfig = msc_config_str,

	.bmAttributes = &msc_attributes,
	.bMaxPower = &msc_maxpower,

	.msc_VendorName = msc_vendor_name,
	.msc_ProductName = msc_product_name,
	.msc_Release = msc_release,
};


#ifdef __cplusplus
}
#endif


#endif /*__USBD_MSC_PARAM_H__*/
