#include <stddef.h>

#include <FreeRTOS.h>
#include <task.h>

#include "app_config.h"
#include <cli/receive.h>
#include <cli/printlog.h>
#include <bsp.h>
#include <nonstdlib.h>
#include <sys_clock.h>
#include <usbh/USBH.h>
#include <lwip/tcpip.h>
#include <wifi/wifi_api.h>
#include "../usbd_ext/usbd_ext.h"
#include <libmid_isp/snx_mid_isp.h>
#include <libmid_automount/automount.h>
#include <lwip/tcpip.h>
#include <mac/mac.h>

//extern void WiFi_Task_Init(void);
//extern void usbh_freertos_init(void);
extern void LwIPConfig(void *pvParameters);

/*
 * This diagnostic pragma will suppress the -Wmain warning,
 * raised when main() does not return an int
 * (which is perfectly OK in bare metal programming!).
 *
 * More details about the GCC diagnostic pragmas:
 * https://gcc.gnu.org/onlinedocs/gcc/Diagnostic-Pragmas.html
 */
#pragma GCC diagnostic ignored "-Wmain"

/* Default parameters if no parameter struct is available */
static const portCHAR defaultText[] = "<NO TEXT>\r\n";
static const UBaseType_t defaultDelay = 1000;

unsigned int ulIdleCycleCount = 0;

void vApplicationIdleHook(void)
{
	ulIdleCycleCount++;
	//vPrintMsg("Idle....\n");
}

/*
 * A convenience function that is called when a FreeRTOS API call fails
 * and a program cannot continue. It prints a message (if provided) and
 * ends in an infinite loop.
 */
static void FreeRTOS_Error(const portCHAR* msg)
{
    if ( NULL != msg )
    {
    	print_msg("%s", msg);
        //vDirectPrintMsg(msg);
    }

    for ( ; ; );
}



xTaskHandle task_automount_version = NULL;

int sd_firmware_version(void)
{       
    FILINFO finfo;  
    int     _max_lfn = 255;
    char    lfn[ _max_lfn + 1 ];    // Buffer to store the LFN  
    char *  path0 = "FIRMWARE_660R.bin";

    FIL  fd;
    int ret;
    unsigned int fir_size = 0;
    unsigned int fir_ver_size_offset = 0;
    unsigned int fir_ver_offset = 0;
    unsigned int br = 0;
    char *version = NULL;
    unsigned int version_size = 0;

    finfo.lfname = lfn;
    finfo.lfsize = _max_lfn + 1;
    // wait for all task init
//    vTaskDelay(20000/portTICK_PERIOD_MS);
    
    // check fir file 
    if (f_stat( path0, & finfo ) == FR_OK) {
        print_msg("FIRMWARE_660R.bin found\n");
    }
    else{
        print_msg("FIRMWARE_660R.bin not found\n");
        goto notexit;
    }


    // check fir version
    if((ret=f_open(&fd,path0,FA_OPEN_EXISTING|FA_READ))!=FR_OK)
    {
        print_msg("FIRMWARE_660R.bin open fail\n");
        goto notexit;
    }

    fir_size = f_size(&fd);
    print_msg("FIRMWARE_660R.bin Size = %d \n",fir_size);

    // read version size
    fir_ver_size_offset = fir_size - 16 - 4;
    if(f_lseek(&fd, fir_ver_size_offset) != FR_OK){
       print_msg("FIRMWARE_660R.bin get version offset fail!\n");
       goto ffail;
    }

    if ((f_read(&fd, &version_size, sizeof(unsigned int), &br))!=FR_OK) {
        print_msg("read version size failed!\n");
        goto ffail;
    }

    // read version
    if(!(version = (char *) pvPortMalloc(version_size, GFP_KERNEL, MODULE_APP))){
          print_msg("pvPortMalloc failed!\n");
          goto ffail;
    }

    // read version
    fir_ver_offset = fir_size - 16 - 4 - version_size;
    if(f_lseek(&fd, fir_ver_offset) != FR_OK){
       print_msg("FIRMWARE_660R.bin get version offset fail!\n");
       goto out;
    }

    if ((f_read(&fd, version, version_size, &br))!=FR_OK) {
        print_msg("read version size failed!\n");
        goto out;
    }

    print_msg("firmware_660r.bin version = %s \n", version);

out:
    vPortFree(version);  
ffail:
    f_close(&fd);

notexit:

    return 0;
}

void sd_version_task (void *pvParameters) 
{
    automount_info_t*   sdaumt_info;

    sdaumt_info = get_automount_info();

    for( ;; ) 
    {
        if( xSemaphoreTake(sdaumt_info->SdFwupgradeMutex, portMAX_DELAY ) == pdTRUE ) 
        {
            print_msg_queue("sd_firmware_upgrade check...\n");
            sd_firmware_version();
            break;
        }
    }
	vTaskDelete(NULL);

}

int sd_version_init(void)
{   
    if (pdPASS != xTaskCreate(sd_version_task, "AutoMount fw version", STACK_SIZE_8K  , NULL,  80, &task_automount_version))
    {
        print_msg_queue("Could not create Task1 task\r\n");
        return pdFAIL;
    }
    else
        return pdPASS;
    
}

/* Startup function that creates and runs two FreeRTOS tasks */
void main(void)
{
	/* Init of print related tasks: */
	//if (pdFAIL == printInit(PRINT_UART_NR))
		//FreeRTOS_Error("Initialization of print failed\r\n");

	/*
	 * I M P O R T A N T :
	 * Make sure (in startup.s) that main is entered in Supervisor mode.
	 * When vTaskStartScheduler launches the first task, it will switch
	 * to System mode and enable interrupt exceptions.
	 */
	print_msg("====================================\n");
	print_msg("=========  FreeRTOS v8.2.0  ========\n");
	print_msg("===========    mptool    ==========\n");
	print_msg("====================================\n");

#ifdef CONFIG_MODULE_MAC_SUPPORT
	eth_init();
#endif
	tcpip_init(LwIPConfig, NULL);

	/* Init of receiver related tasks: */
	if (pdFAIL == recvInit())
		FreeRTOS_Error("Initialization of receiver failed\r\n");

	init_recv_task();

	/* Init printlog task */
	Init_PrintLog_Task();

	/* Set eraly stage finish */
	set_eraly_stage_flag (0);

	//init_sys_clock_task();

	// USBH init
	usbh_freertos_init();

	//mcu_connect_notask();

	// Initial WIFI TASK
#if CONFIG_MODULE_WIFI_SUPPORT
#if CONFIG_WIFI_MODE_AP
	WiFi_Task_Init(NULL, WIFI_RUN_MODE_AP);
#elif CONFIG_WIFI_MODE_STA
	WiFi_Task_Init(NULL, WIFI_RUN_MODE_DEV);
#endif
#endif

	// init usbd_ext
	usbd_ext_init();
	
	automount_init();

	sd_version_init();


	print_msg("Start OS\n");

	/* Start the FreeRTOS scheduler */
	vTaskStartScheduler();

	/*
	 * If all goes well, vTaskStartScheduler should never return.
	 * If it does return, typically not enough heap memory is reserved.
	 */
	FreeRTOS_Error("Could not start the scheduler!!!\r\n");

	/* just in case if an infinite loop is somehow omitted in FreeRTOS_Error */
	for ( ; ; );
}

void vApplicationStackOverflowHook( TaskHandle_t xTask,
                                    signed char *pcTaskName )
{
	print_msg("%s task stack overflow\n", pcTaskName);

}
