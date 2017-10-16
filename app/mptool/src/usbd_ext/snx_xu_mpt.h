#ifndef __SNX_XU_MPT_H__
#define __SNX_XU_MPT_H__

#include <task.h>
#include <stddef.h>
#include <queue.h>
#include <semphr.h>
#include <usb_device/usbd_uvc.h>


#define XU_SYS_MPT_LEN                          0x40
// ----------------------------- XU_SYS_MPT_CTRL Sub-Command ID ------------------------------------
#define MPT_GET_VERSION                         0x00
#define MPT_DIR_SET_CMD                         0x01
#define MPT_PROG_PROCESS                        0x02
#define MPT_OSD_AF_REPORT						0x03
#define MPT_ADO_RECORD                          0x10
#define MPT_ADO_RECORD_GET_RESULT              	0x11
#define MPT_ADO_PLAYBACK                        0x12
#define MPT_ADO_PLAYBACK_GET_RESULT				0x13
#define MPT_SD_INSERT_STATE                   	0x20
#define MPT_SD_RW_TEST                          0x21
#define MPT_SD_RW_GET_RESULT                   	0x22
#define MPT_GPIO_OUT                            0x30
#define MPT_GPIO_IN                             0x31
#define MPT_IR_CUT_SET_MODE                     0x32
#define MPT_NET_STA_SSID                        0x40
#define MPT_NET_STA_PASSWD                      0x41
#define MPT_NET_STA_IPADDR                    	0x42
#define MPT_NET_AP_SSID                    	    0x43
#define MPT_NET_AP_PASSWD                       0x44
#define MPT_NET_AP_IPADDR                       0x45
#define MPT_NET_MACADDR                         0x46
#define MPT_NET_ACTIVE                          0x47
#define MPT_NET_VDO_GET_ALL_RESOLUTION          0x48
#define MPT_NET_VDO_FMT                         0x49
#define MPT_NET_VDO_RESOLUTION                  0x4A
#define MPT_NET_VDO_FR                          0x4B
#define MPT_NET_VDO_BR                          0x4C
#define MPT_NET_SUPPORT_MODE					0x4D
#define MPT_VDO_MIRROR							0x50
#define MPT_VDO_FLIP							0x51
#define MPT_VDO_BR								0x52


#define MPT_IVALID_MODE		0xff
#define MPT_XU_CMD_ERR		UNKNOWN_ERR

/******GPIO*******/
#define MPT_GPIO_GPIO0	0
#define MPT_GPIO_GPIO1	1
#define MPT_GPIO_GPIO2	2
#define MPT_GPIO_GPIO3	3
#define MPT_GPIO_GPIO4	4
#define MPT_GPIO_GPIO6	5

#define MPT_GPIO_PWM1	6
#define MPT_GPIO_PWM2	7

#define MPT_GPIO_MS1_IO0	8
#define MPT_GPIO_MS1_IO1	9
#define MPT_GPIO_MS1_IO2	10
#define MPT_GPIO_MS1_IO3	11
#define MPT_GPIO_MS1_IO4	12
#define MPT_GPIO_MS1_IO5	13

#define MPT_GPIO_AUD_IO0	14
#define MPT_GPIO_AUD_IO1	15
#define MPT_GPIO_AUD_IO2	16
#define MPT_GPIO_AUD_IO3	17
#define MPT_GPIO_AUD_IO4	18

#define MPT_GPIO_SCL2	19
#define MPT_GPIO_SDA2	20

#define MPT_GPIO_SSP1_CLK	21
#define MPT_GPIO_SSP1_TX	22
#define MPT_GPIO_SSP1_FS	23
#define MPT_GPIO_SSP1_RX	24

enum ENC_AUTH_TYPE {
	OPEN_MODE = 0,
	WEP_MODE,
	WPA_MODE,
	WPA2_MODE,
};

typedef enum record_complete_state {
	RECORD_COMPLETED = 0,
	RECORD_FAILED,
	RECORD_NOT_COMPLETED = 0xff,
} mpt_complete_state_t;

typedef struct mpt_module_manager_t{
	TaskHandle_t 		task;
	QueueHandle_t 		queue;
	SemaphoreHandle_t 	sem;
	char *				data;
}mpt_module_manager;

struct mpt_status_t
{
	int mpt_cur_cmd;

	/* gpio in number */
	unsigned char gpio_num;

	/* net work */
	unsigned char ip_mode;
	char sWiFiSSID[32];
    char sWiFiPWD[32];

	/* sd rw status*/
	unsigned char sd_rw_status;
	unsigned int sd_r_speed;
	unsigned int sd_w_speed;
	
	unsigned char audio_rec_status;
	unsigned char audio_pb_status;
	
	mpt_module_manager common;
	mpt_module_manager sd;
	mpt_module_manager audio;
	
	/* AF report to OSD task */
	TaskHandle_t af2osd_task;

	unsigned int data_length;
};

int initUsbdUvcXuMpt(void);
int uninitUsbdUvcXuMpt(void);
uint8_t usbd_uvc_events_sys_xu_mpt(uint8_t uvc_req);
void mpt_osd_af_report_set_position(void);

#endif //__SNX_XU_MPT_H__
