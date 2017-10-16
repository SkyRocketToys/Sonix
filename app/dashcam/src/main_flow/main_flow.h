#ifndef __MAIN_FLOW_H__
#define __MAIN_FLOW_H__


#define TASK_KEEP_NO_KEEP		(0)
#define TASK_KEEP_USBD			(1<<0)	// BIT0
#define TASK_KEEP_USBH			(1<<1)	// BIT1
#define TASK_KEEP_WIFI_DRV		(1<<2)	// BIT2
#define TASK_KEEP_RTSP_SEV		(1<<3)	// BIT3

#define RTSP_PREVIEW_AUDIO		0
#define AUDIO_PUSH_TALK			0

void main_flow_init(void);
void all_task_uinit(unsigned int task_keep);
#endif
