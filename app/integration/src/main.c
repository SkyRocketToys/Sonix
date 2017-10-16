#include <stddef.h>

#include <FreeRTOS.h>
#include <task.h>

#include <cli/receive.h>
#include <cli/printlog.h>
#include <bsp.h>
#include <nonstdlib.h>
#include <sys_clock.h>
#include "mcu.h"
#include <libmid_usbd/mid_usbd.h>

/*
 * This diagnostic pragma will suppress the -Wmain warning,
 * raised when main() does not return an int
 * (which is perfectly OK in bare metal programming!).
 *
 * More details about the GCC diagnostic pragmas:
 * https://gcc.gnu.org/onlinedocs/gcc/Diagnostic-Pragmas.html
 */
#pragma GCC diagnostic ignored "-Wmain"


/* Struct with settings for each task */
typedef struct _paramStruct
{
    portCHAR* text;                  /* text to be printed by the task */
    UBaseType_t  delay;              /* delay in milliseconds */
} paramStruct;

/* Default parameters if no parameter struct is available */
static const portCHAR defaultText[] = "<NO TEXT>\r\n";
static const UBaseType_t defaultDelay = 1000;

unsigned int ulIdleCycleCount = 0;

void vApplicationIdleHook(void)
{
	cpu_udelay(10);
	ulIdleCycleCount++;
	//vPrintMsg("Idle....\n");
}

void max_mem_usage()
{
	print_msg("CB\t%dk bytes\n", (xPortGetTotalHeapSize(GFP_KERNEL) - xPortGetMinimumEverFreeHeapSize(GFP_KERNEL)) / 1024);
	print_msg("NCNB\t%dk bytes\n", (xPortGetTotalHeapSize(GFP_DMA) - xPortGetMinimumEverFreeHeapSize(GFP_DMA)) / 1024);
}



/* Task function - may be instantiated in multiple tasks */
void vTaskFunction( void *pvParameters )
{
	const portCHAR* taskName;
	UBaseType_t  delay;
	paramStruct* params = (paramStruct*) pvParameters;

	taskName = ( NULL==params || NULL==params->text ? defaultText : params->text );
	delay = ( NULL==params ? defaultDelay : params->delay);

	for( ; ; )
	{
		/* Print out the name of this task. */
		print_msg_queue("hello %s\n", taskName);

		portYIELD();

		vTaskDelay( delay / portTICK_RATE_MS );
	}

	/*
	 * If the task implementation ever manages to break out of the
	 * infinite loop above, it must be deleted before reaching the
	 * end of the function!
	 */
	vTaskDelete(NULL);
}

void vTaskFunction3(void *pvParameters)
{
	const portCHAR* taskName;
	UBaseType_t  delay;
	paramStruct* params = (paramStruct*) pvParameters;

	taskName = ( NULL==params || NULL==params->text ? defaultText : params->text );
	delay = ( NULL==params ? defaultDelay : params->delay);

	for (;;) {
		/* Print out the name of this task. */
		//print_msg_queue("Haha %s\n", taskName);

		vTaskDelay( delay / portTICK_RATE_MS );
	}

	/*
	 * If the task implementation ever manages to break out of the
	 * infinite loop above, it must be deleted before reaching the
	 * end of the function!
	 */
	vTaskDelete(NULL);
}


/* Fixed frequency periodic task function - may be instantiated in multiple tasks */
void vPeriodicTaskFunction(void* pvParameters)
{
    const portCHAR* taskName;
    UBaseType_t delay;
    paramStruct* params = (paramStruct*) pvParameters;

    taskName = ( NULL==params || NULL==params->text ? defaultText : params->text );
    delay = ( NULL==params ? defaultDelay : params->delay);

    for( ; ; )
    {
        /* Print out the name of this task. */
    	print_msg_queue("hi integration %s\n", taskName);

        /*
         * The task will unblock exactly after 'delay' milliseconds (actually
         * after the appropriate number of ticks), relative from the moment
         * it was last unblocked.
         */
	    vTaskDelay( delay / portTICK_RATE_MS );
    }

    /*
     * If the task implementation ever manages to break out of the
     * infinite loop above, it must be deleted before reaching the
     * end of the function!
     */
    vTaskDelete(NULL);
}


/* Parameters for two tasks */
static const paramStruct tParam[3] = {
		(paramStruct) { .text="Task1", .delay=5000 },
		(paramStruct) { .text="==== Task2 ====", .delay=5000 },
		(paramStruct) { .text="### Task3 ###", .delay=3000 }
};


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
	print_msg("=========    Integration    ========\n");
	print_msg("====================================\n");

	//eth_init();

	//if (pdPASS != xTaskCreate(vTaskFunction, "task1", STACK_SIZE_512, (void*) &tParam[0],
	//		PRIORITY_TASK_APP_TEST01, NULL))
	//	FreeRTOS_Error("Could not create task1\r\n");

	//if (pdPASS != xTaskCreate(vPeriodicTaskFunction, "task2", STACK_SIZE_512, (void*) &tParam[1],
	//		PRIORITY_TASK_APP_TEST02, NULL))
	//	FreeRTOS_Error("Could not create task2\r\n");

	//if (pdPASS != xTaskCreate(vTaskFunction3, "task3", STACK_SIZE_512, (void*) &tParam[2],
	//		PRIORITY_TASK_APP_TEST03, NULL))
	//	FreeRTOS_Error("Could not create task3\r\n");
	MCU_APP_INIT();

	/* init USB Device */
	usbd_mid_init(NULL);

	/* Init of receiver related tasks: */
	if (pdFAIL == recvInit())
		FreeRTOS_Error("Initialization of receiver failed\r\n");

	init_recv_task();

	/* Init printlog task */
	Init_PrintLog_Task();

	init_systemlog_task();

	//init_sys_clock_task()
	print_msg("\nStartup memory max usage\n");
	max_mem_usage();

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
