#ifndef __MAIN_FLOW_H__
#define __MAIN_FLOW_H__


#define TASK_KEEP_NO_KEEP		(0)
#define TASK_KEEP_USBD			(1<<0)	// BIT0
//#define TASK_KEEP_USBH			(1<<1)	// BIT1
//#define TASK_KEEP_WIFI_DRV		(1<<2)	// BIT2

void main_flow_init(void);
void all_task_uinit(unsigned int task_keep);
void task_qrscan_flow( void *pvParameters );
int SetMotionAttr(
		 int index,
		 int enable,
		 int x,
		 int y,
		 int w,
		 int h,
		 int threshold
		);

void InitMonitorParameter(void);

#endif
