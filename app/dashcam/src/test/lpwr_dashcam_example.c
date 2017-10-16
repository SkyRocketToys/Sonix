#include <FreeRTOS.h>
#include <bsp.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include <nonstdlib.h>
#include <string.h>
#include "debug.h"
#include <generated/snx_sdk_conf.h>
#include "../main_flow.h"
#ifndef CONFIG_APP_DRONE
#include "../mcu_v2/mcu.h"
#endif

#include <../record/rec_schedule.h>
#include <../record/rec_common.h>
#include <bootinfo/bootinfo.h>

#define USB_RET_SUCCESS			0
#define USB_RET_FAIL			1
#define VIDEO_SUSPEND			0
#define VIDEO_RESUME			1
#define SENSOR_CLK_DISABLE		0
#define SENSOR_CLK_ENABLE		1

xTaskHandle lpwr_dashcam_example = NULL;

unsigned int GetTime()
{
	unsigned int i,Tmpts;
	struct timeval ts;
	gettimeofday(&ts, NULL);
	Tmpts=(unsigned int)ts.tv_sec*1000000+(unsigned int)ts.tv_usec;
	return Tmpts;
}

void lpwr_dashcam_example_task (void *pvParameters)
{
	for ( ;; ) {
		vTaskDelay( 1000/portTICK_PERIOD_MS );
        struct timeval LastTs,CurrTs;
        uint32_t Ret;
        
        timing_uninit();
        
        //print_msg("%u\t wifi suspend\n",GetTime());
   		//wifi_plugout_cb();    >>> cause Assert             

        //print_msg("%u\t USBH suspend\n",GetTime());
        if(wifi_init(1)||msc_init(1)||uvc_init(1)||hub_init(1)){
            while(usbh_root_one_suspend()==USB_RET_FAIL)
            	print_msg("USBH suspend fail\n");
		}
		
        //print_msg("%u\t sensor suspend\n",GetTime());
        snx_isp_sensor_suspend_get(&Ret);
		if(Ret==SENSOR_CLK_ENABLE){
            snx_isp_sensor_suspend_set(VIDEO_SUSPEND);
            gettimeofday(&LastTs, NULL);
			while(Ret==SENSOR_CLK_ENABLE){
				gettimeofday(&CurrTs, NULL);
				if((CurrTs.tv_usec - LastTs.tv_usec) >= 60000){
					print_msg("VIDEO suspend fail\n");
					snx_isp_sensor_suspend_set(VIDEO_SUSPEND);
					gettimeofday(&LastTs, NULL);
				}
				vTaskDelay( 2/portTICK_PERIOD_MS );
				snx_isp_sensor_suspend_get(&Ret);
			}	
		}
		
        //print_msg("%u\t audio suspend\n",GetTime());
		audio_suspend();

        //print_msg("%u\t Enter enter_lpwr_critical_section\n",GetTime()); 
        enter_lpwr_critical_section(0);

       	//print_msg("%u\t Exit enter_lpwr_critical_section - audio_restart\n",GetTime());
		audio_restart();

        //print_msg("%u\t sensor resume\n",GetTime()); 
		snx_isp_sensor_suspend_set(VIDEO_RESUME);
        snx_isp_sensor_suspend_get(&Ret);
		while(Ret==SENSOR_CLK_DISABLE){
			snx_isp_sensor_suspend_set(VIDEO_RESUME);
        	print_msg("VIDEO restart fail\n");
			snx_isp_sensor_suspend_get(&Ret);
		}
		
        //print_msg("%u\t wifi resume\n",GetTime()); 
     	//wifi_plugin_cb();  >>> cause Assert                   
                        
        //print_msg("%u\t USBH resume\n",GetTime()); 
        if(wifi_init(1)||msc_init(1)||uvc_init(1)||hub_init(1)){
	        while(usbh_root_one_resume()==USB_RET_FAIL)
				print_msg("USBH resume fail\n");
		}
		
        //print_msg("%u\t timing reinit\n",GetTime()); 
        timing_reinit();
        //print_msg("%u\t system normal\n",GetTime()); 
	}
}

int lpwr_dashcam_example_init(void)
{
	if (pdPASS != xTaskCreate(lpwr_dashcam_example_task, "lpwr_dashcam_example", STACK_SIZE_8K  , NULL,  80, &lpwr_dashcam_example)) {
		print_msg_queue("Could not create lpwr_dashcam_example task\r\n");
		return pdFAIL;
	} else
		return pdPASS;

}
