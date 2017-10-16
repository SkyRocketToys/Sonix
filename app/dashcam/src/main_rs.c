#include <stddef.h>

#include <FreeRTOS.h>
#include <task.h>

#include <cli/receive.h>
#include <cli/printlog.h>
#include <bsp.h>
#include <nonstdlib.h>
#include <sys_clock.h>
#include <generated/snx_sdk_conf.h>
#include <usbh/USBH.h>
#include <lwip/tcpip.h>
#include <wifi/wifi_api.h>
#include <libmid_fwupgrade/fwupgrade.h>
#include <mac/mac.h>
#include "watch_task.h"
//#include "../../../driver/sn986xx/wifi/src/WiFi.h"
#include "main_flow/main_flow.h"
//#include "wifi_sta_mode.h"

//extern void WiFi_Task_Init(void);
extern void usbh_freertos_init(void);
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
	print_msg("===========    Dashcam    ==========\n");
	print_msg("====================================\n");

//#ifdef CONFIG_MODULE_MAC_SUPPORT
//	eth_init();
//#endif

	// tcpip_init(LwIPConfig, NULL);


	/* Init of receiver related tasks: */
	if (pdFAIL == recvInit())
		FreeRTOS_Error("Initialization of receiver failed\r\n");

	init_recv_task();

	/* Init printlog task */
	Init_PrintLog_Task();

	/* Set eraly stage finish */
	set_eraly_stage_flag (0);

	//init_sys_clock_task();

#ifndef CONFIG_ENABLE_DEBUG_MODE
//	wdt_keepalive_init(10); /* 10 sec feed dog*/
#endif

// FOR TEST
#if 1
	// USBH init
	// usbh_freertos_init();
#endif

	main_flow_init();

#ifdef CONFIG_SYSTEM_TRACE_SELECT
	print_msg("Trace recorder addr = 0x%x, recorder buf size = 0x%x\n", (unsigned int)vTraceGetTraceBuffer(), (unsigned int)uiTraceGetTraceBufferSize());
#endif

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
