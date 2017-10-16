#ifndef __SNX_XU_USR_H__
#define __SNX_XU_USR_H__

#include <task.h>
#include <stddef.h>
#include <queue.h>
#include <semphr.h>
#include <usb_device/usbd_uvc.h>

#define XU_USR_ERR				UNKNOWN_ERR
#define XU_USR_CMD_TIMEOUT		5000 //ms
#define XU_USR_CS_LOCK_TIMEOUT	500 //ms
#define XU_USR_LEN				0x40
// ----------------------------- XU_USR_DEV_MGMT Control Selectors ID ------------------------------------
enum XU_USR_CS_ID {
	XU_USR_CS_DEV_MGMT = 0x01,
	XU_USR_CS_VID_MGMT,
	XU_USR_CS_REC_MGMT,
	XU_USR_CS_SD_MGMT,
	XU_USR_CS_POLLING_STATUS,
	XU_USR_CS_RESERVED,
//...New Control Selector
	XU_USR_CS_MAX
};

// ----------------------------- XU_USR_DEV_MGMT Sub-Command ID ------------------------------------
enum XU_USR_DEV_MGMT_SUB_CMD {
	XU_USR_DEV_SUB_GETTIME = 0x00,
	XU_USR_DEV_SUB_SYNCTIME,
	XU_USR_DEV_SUB_GETBATTERY,
	XU_USR_DEV_SUB_GETDEVPARAM,
	XU_USR_DEV_SUB_GETDEVVERSION,
	XU_USR_DEV_SUB_SETPWRFREQ,
	XU_USR_DEV_SUB_SETGSENSORPARAM,
	XU_USR_DEV_SUB_GETGSENSORINFO,
	XU_USR_DEV_SUB_NVRAMRESETDEF,
	XU_USR_DEV_SUB_REBOOT,
	XU_USR_DEV_SUB_MAX
};

// ----------------------------- XU_USR_VID_MGMT Sub-Command ID ------------------------------------
enum XU_USR_VID_MGMT_SUB_CMD {
	XU_USR_VID_SUB_GETVIDPARAM = 0x00,
	XU_USR_VID_SUB_SETVIDPARAM,
	XU_USR_VID_SUB_SETWDR,
	XU_USR_VID_SUB_SETMIRROR,
	XU_USR_VID_SUB_SETFLIP,
	XU_USR_VID_SUB_MAX
};

// ----------------------------- XU_USR_REC_MGMT Sub-Command ID ------------------------------------
enum XU_USR_REC_MGMT_SUB_CMD {
	XU_USR_REC_SUB_TAKEPIC = 0x00,
	XU_USR_REC_SUB_GETRECSTA,
	XU_USR_REC_SUB_SETRECSTA,
	XU_USR_REC_SUB_SETRECPARAM,
	XU_USR_REC_SUB_SETRECAUDSTA,
	XU_USR_REC_SUB_SETRECLEN,
	XU_USR_REC_SUB_SETPRTLEN,
	XU_USR_REC_SUB_SETLRECSTA,
	XU_USR_REC_SUB_GETRECCAP,
	XU_USR_REC_SUB_MAX
};

// ----------------------------- XU_USR_SD_MGMT Sub-Command ID ------------------------------------
enum XU_USR_SD_MGMT_SUB_CMD {
	XU_USR_SD_SUB_GETSPACE = 0x00,
	XU_USR_SD_SUB_SETFORMAT,
	XU_USR_SD_SUB_SETSDDIRINFO,
	XU_USR_SD_SUB_GETSDROOTDIRINFO,
	XU_USR_SD_SUB_GETSDRECDIRINFO,
	XU_USR_SD_SUB_GETSDPROTDIRINFO,
	XU_USR_SD_SUB_GETSDPICDIRINFO,
	XU_USR_SD_SUB_GETSDTIMLAPDIRINFO,
	XU_USR_SD_SUB_GETSDTHUMBDIRINFO,
	XU_USR_SD_SUB_MAX
};

// ----------------------------- UVC XU USR COMMAND DIRECTION ------------------------------------
enum XU_USR_DIRECTION {
	XU_USR_DIRECTION_NONE = 0x00,
	XU_USR_DIRECTION_SET,
	XU_USR_DIRECTION_GET,
};

typedef struct uvc_xu_usr_subcmd_st_t
{
	uint8_t subcmd_id;
	uint8_t direction;
	uint8_t (*cb)(uint8_t *data, uint32_t len);
}uvc_xu_usr_subcmd_st;

typedef struct uvc_xu_usr_cs_st_t
{
	uint8_t cs_id;
	uint8_t direction;
	uint8_t (*cb)(uint8_t *data, uint32_t len);
	uint8_t subcmd_num;
	uvc_xu_usr_subcmd_st *subcmd_st;
}uvc_xu_usr_cs_st;

typedef struct video_enginemode_t{
	int pre_ext_pframe_num;
    int pre_ext_qp_range;
    int pre_ext_qp_max;
	int pre_ext_upper_pframe;
	int pre_ext_upper_pframe_dup1;
	int pre_qp_max;
	int pre_qp_min;
	int resume;
	int suspend;
}video_enginemode;

typedef struct record_enginemode_t{
	int rec_ext_pframe_num;
    int rec_ext_qp_range;
    int rec_ext_qp_max;
	int rec_ext_upper_pframe;
	int rec_qp_max;
	int rec_qp_min;
}record_enginemode;

typedef struct polling_stat_t{
	struct {
		uint8_t sd_format	:4;
		uint8_t reserved	:4;
	}sd;
	//...max 64bytes supported
}polling_st;

int initUsbdUvcXuUsr(usb_device_desc_info_t *desc_ptr, uint8_t *GUID);
int uninitUsbdUvcXuUsr(usb_device_desc_info_t *desc_ptr);
uint8_t usbd_ext_uvc_usr_xu_process(uint8_t uvc_cs, uint8_t uvc_req);

#endif //__SNX_XU_USR_H__
