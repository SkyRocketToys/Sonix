/*
*	File: usbd_uvc_param.h
*	Version: 1.0
*	Author: chkuo
*	Date: 2015-12-30
*	Descriptions: SONiX UVC Device parameter
*/

#ifndef __USBD_UVC_PARAM_H__
#define __USBD_UVC_PARAM_H__

#include <generated/snx_sdk_conf.h>
#include <usb_device/usb_device.h>
#include <usb_device/usbd_uvc.h>
#include "usbd_ext.h"

#include "../sensor_cap/sensor_capability.h"

#ifdef __cplusplus
extern "C"{
#endif

/* ------------------------------------------------------------------------
 * UVC Parameter
 */

#define USBD_UVC_VENDOR_ID          0x0C45	/* Sonix Technology Co. */
#define USBD_UVC_PRODUCT_ID         0x8200	/* SN98660 UVC */
#define USBD_UVC_DEVICE_BCD         0x0100	/* 1.00 */

static unsigned short uvc_vendor_id = USBD_UVC_VENDOR_ID;
static unsigned short uvc_product_id = USBD_UVC_PRODUCT_ID;
static unsigned short uvc_bcdDevice = USBD_UVC_DEVICE_BCD;

static char uvc_vendor_str[] 	= "Sonix Technology Co., Ltd.";
static char uvc_product_str[]	= "DashCam";
static char uvc_config_str[]	= "USB Video";

static unsigned char UsrXUbmControls[4] = {0x00, 0x00, 0x00, 0x00};
static unsigned char UsrXUGUID[16] = { 0x37, 0x1E, 0xA3, 0x8D, 0xC1, 0xC7, 0xF2, 0x4A,
										0xB2, 0xA5, 0xE4, 0xAA, 0xB1, 0x86, 0x75, 0xF0 };

#ifdef CONFIG_MODULE_USB_DEVICE_SELFPOWER
	static unsigned char uvc_attributes = 0x40;
#else
	static unsigned char uvc_attributes = 0x00;
#endif

#ifdef CONFIG_MODULE_USB_DEVICE_VBUS_DRAW
	static unsigned char uvc_maxpower   = (CONFIG_MODULE_USB_DEVICE_VBUS_DRAW/2);
#endif

#if defined(USBD_OUTFORMAT_MJPEG) && USBD_OUTFORMAT_MJPEG
#define SNX_MJPG_HEADER_LEN				589
struct image_info uvc_mjpg_fhd = {
	.type                               = UVC_VS_FORMAT_MJPEG ,
	.width                              = FHD_WIDTH,
	.height                             = FHD_HEIGHT,
	.imagesize                          = (FHD_WIDTH * FHD_HEIGHT) + SNX_MJPG_HEADER_LEN,
	.fps_num                            = 5,
	.fps                                = { 30, 20, 15, 10, 5 },
};
struct image_info uvc_mjpg_hd = {
	.type                               = UVC_VS_FORMAT_MJPEG ,
	.width                              = HD_WIDTH,
	.height                             = HD_HEIGHT,
	.imagesize                          = (HD_WIDTH * HD_HEIGHT) + SNX_MJPG_HEADER_LEN,
	.fps_num                            = 6,
	.fps                                = { 60, 30, 20, 15, 10, 5 },
};
struct image_info uvc_mjpg_vga = {
	.type                               = UVC_VS_FORMAT_MJPEG ,
	.width                              = VGA_WIDTH,
	.height                             = VGA_HEIGHT,
	.imagesize                          = (VGA_WIDTH * VGA_HEIGHT) + SNX_MJPG_HEADER_LEN,
	.fps_num                            = 6,
	.fps                                = { 60, 30, 20, 15, 10, 5 },
};
#endif

struct image_info uvc_h264_fhd = {
	.type                               = UVC_VS_FORMAT_FRAME_BASED ,
	.width                              = FHD_WIDTH,
	.height                             = FHD_HEIGHT,
	.imagesize                          = FHD_WIDTH * FHD_HEIGHT,
	.fps_num                            = 5,
	.fps                                = { 30, 20, 15, 10, 5 },
};
struct image_info uvc_h264_hd = {
	.type                               = UVC_VS_FORMAT_FRAME_BASED ,
	.width                              = HD_WIDTH,
	.height                             = HD_HEIGHT,
	.imagesize                          = HD_WIDTH * HD_HEIGHT,
	.fps_num                            = 6,
	.fps                                = { 60, 30, 20, 15, 10, 5 },
};
struct image_info uvc_h264_vga = {
	.type                               = UVC_VS_FORMAT_FRAME_BASED ,
	.width                              = VGA_WIDTH,
	.height                             = VGA_HEIGHT,
	.imagesize                          = VGA_WIDTH * VGA_HEIGHT,
	.fps_num                            = 6,
	.fps                                = { 60, 30, 20, 15, 10, 5 },
};


struct image_info *uvc_img_info_list[] = {
#if defined(USBD_OUTFORMAT_MJPEG) && USBD_OUTFORMAT_MJPEG
	// MJPG list
	&uvc_mjpg_fhd,
	&uvc_mjpg_hd,
	&uvc_mjpg_vga,
#endif
	// H264 list
	&uvc_h264_fhd,
	&uvc_h264_hd,
	&uvc_h264_vga,
	NULL
};


static struct uvc_device_desc_info uvc_desc_info = {
	.idVendor = &uvc_vendor_id,
	.idProduct = &uvc_product_id,
	.bcdDevice = &uvc_bcdDevice,
	.strVendor = uvc_vendor_str,
	.StrProduct = uvc_product_str,
	.StrConfig = uvc_config_str,

	.bmAttributes = &uvc_attributes,
#ifdef CONFIG_MODULE_USB_DEVICE_VBUS_DRAW
	.bMaxPower = &uvc_maxpower,
#endif

	.img_info = uvc_img_info_list,
	.UsrXUbmControls = UsrXUbmControls,
	.UsrXUGUID = UsrXUGUID,
};


#ifdef __cplusplus
}
#endif


#endif /*__USBD_UVC_PARAM_H__*/
