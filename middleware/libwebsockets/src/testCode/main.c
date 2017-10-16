/*
Copyright 2013, Jernej Kovacic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/


/**
 * @file
 * A simple demo application.
 *
 * @author Jernej Kovacic
 */



#include <stddef.h>

#include <FreeRTOS.h>
#include <task.h>

#include "app_config.h"
#include <cli/receive.h>
#include <cli/printlog.h>
#include <bsp.h>
#include <nonstdlib.h>
#include <sys_clock.h>
#include "../../../driver/sn986xx/usb_host/src/USBH.h"
//#include "../../../driver/sn986xx/wifi/src/WiFi.h"
#include <usb_device/usb_device.h>
#include "libmid_websockets/client.h"
extern void WiFi_Task_Init(void);
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


//websocket callback
cwebsocket_client srv_websocket_client;
#define APPSRVIP "ws://192.168.1.2:8080"
void srv_cwebsocket_subprotocol_echo_client_onopen(void *websocket) {
        cwebsocket_client *client = (cwebsocket_client *)websocket;
        print_msg("onopen: fd=%i\n", client->fd);
}

void srv_cwebsocket_subprotocol_echo_client_onmessage(void *websocket, cwebsocket_message *message) {
        cwebsocket_client *client = (cwebsocket_client *)websocket;

        //print_msg("%s\n",message->payload);
        print_msg("onmessage: fd=%i, opcode=%#04x, payload_len=%d, ----- actual_payload_len=%d\n",
        			client->fd, message->opcode, message->payload_len, strlen(message->payload));

        struct timeval time;
        gettimeofday(&time, NULL);

        print_msg("OnMessage, time difference = %d\n", time.tv_usec);

        char *ack = message->payload;




}

void srv_cwebsocket_subprotocol_echo_client_onclose(void *websocket, int code, const char *reason) {
        cwebsocket_client *client = (cwebsocket_client *)websocket;
        print_msg("onclose: fd=%i, code=%i, reason=%s\n", client->fd, code, reason);
}

void srv_cwebsocket_subprotocol_echo_client_onerror(void *websocket, const char *message) {
        cwebsocket_client *client = (cwebsocket_client *)websocket;
        print_msg("onerr: fd=%i, message=%s\n", client->fd, message);
}

cwebsocket_subprotocol* srv_cwebsocket_subprotocol_echo_client_new() {
        cwebsocket_subprotocol *protocol = malloc(sizeof(cwebsocket_subprotocol));
        memset(protocol, 0, sizeof(cwebsocket_subprotocol));
        protocol->name = "echo.cwebsocket\0";
        protocol->onopen = &srv_cwebsocket_subprotocol_echo_client_onopen;
        protocol->onmessage = &srv_cwebsocket_subprotocol_echo_client_onmessage;
        protocol->onclose = &srv_cwebsocket_subprotocol_echo_client_onclose;
        protocol->onerror = &srv_cwebsocket_subprotocol_echo_client_onerror;
        return protocol;
}

int RunWebSocketConnection(void)
{
         cwebsocket_client_init(&srv_websocket_client, NULL, 0);
          srv_websocket_client.subprotocol = srv_cwebsocket_subprotocol_echo_client_new();
    //srv_websocket_client.uri = "ws://smaudio.dev.sonixcloud.com:80/deviceStreamChan";//"ws://smapp.dev.sonixcloud.com/v2/deviceRequest:80";
          srv_websocket_client.uri = APPSRVIP;//"ws://smapp.dev.sonixcloud.com:80/v2/deviceRequest";
          srv_websocket_client.hostname = "192.168.119.2";
          srv_websocket_client.port = "8080";
          srv_websocket_client.querystring = "relay";
          print_msg("----> into RunWebSocketConnection ...");
          if(cwebsocket_client_connect(&srv_websocket_client) == -1) {
                   print_msg("websocket connect failed!\n");
                         return 1;
           }


    return 0;
}

void RunWebSocketClose(void)
{
        //6. Colse websocket connection
        print_msg("----- Close websocket -----\n");
        cwebsocket_client_close(&srv_websocket_client, 1000, "main: run loop complete");
}








//





void vApplicationIdleHook(void)
{
	ulIdleCycleCount++;
	//vPrintMsg("Idle....\n");
}


/* Task function - may be instantiated in multiple tasks */
void vTaskFunction( void *pvParameters )
{
	const portCHAR* taskName;
	UBaseType_t  delay;
	paramStruct* params = (paramStruct*) pvParameters;

	taskName = ( NULL==params || NULL==params->text ? defaultText : params->text );
	delay = ( NULL==params ? defaultDelay : params->delay);


	        print_msg("\n--------> Dhcp wait !! ...%d\n", delay);
	        dhcp_wait();
	        print_msg("\n++++++++ RunWebSocketConnection ...\n");
	        vTaskDelay( 2000/portTICK_RATE_MS );
	        RunWebSocketConnection();
	        vTaskDelay( 1000/portTICK_RATE_MS );


	         int bytesWrite = 0, readsByte = 0;
	         int length = 1024;
	         unsigned char cmd[length];
	         memset(cmd, 0x88, length);

	        cwebsocket_client_start_read(&srv_websocket_client);

	         int count = 0;
	        for( ; ; )
	        {
	                bytesWrite = cwebsocket_client_write_data(&srv_websocket_client, cmd, length, BINARY_FRAME);


	                //print_msg("main : read byte length is : ", readsByte);
	                vTaskDelay( 10 );

	                count++;

	                if(count==50){

	                	break;
	                }
	        }


	        vTaskDelay( 3000 );
	        print_msg("\n-------- RunWebSocketClose ...\n");
	       	RunWebSocketClose();







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
    	//print_msg_queue("hi %s\n", taskName);

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
	print_msg("====================================\n");

	eth_init();
	tcpip_init(LwIPConfig, NULL);
#if 0
	if (pdPASS != xTaskCreate(vTaskUSBD, "USBD task", STACK_SIZE_2K  , 0,	PRIORITY_TASK_APP_TEST01, NULL))
		FreeRTOS_Error("Could not create USBD task\r\n");
#endif

	if (pdPASS != xTaskCreate(vTaskFunction, "task1", STACK_SIZE_8K, (void*) &tParam[0],
			PRIORITY_TASK_APP_TEST01, NULL))
		FreeRTOS_Error("Could not create task1\r\n");

	if (pdPASS != xTaskCreate(vPeriodicTaskFunction, "task2", STACK_SIZE_512, (void*) &tParam[1],
			PRIORITY_TASK_APP_TEST02, NULL))
		FreeRTOS_Error("Could not create task2\r\n");

	if (pdPASS != xTaskCreate(vTaskFunction3, "task3", STACK_SIZE_512, (void*) &tParam[2],
			PRIORITY_TASK_APP_TEST03, NULL))
		FreeRTOS_Error("Could not create task3\r\n");

	/* Init of receiver related tasks: */
	if (pdFAIL == recvInit())
		FreeRTOS_Error("Initialization of receiver failed\r\n");

	init_recv_task();

	/* Init printlog task */
	Init_PrintLog_Task();

	init_sys_clock_task();

	// USBH init
	usbh_freertos_init();

	// Initial WIFI TASK
	WiFi_Task_Init();
    //CstreamerInit();
    //snx_lib_cloud_test_main();
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
