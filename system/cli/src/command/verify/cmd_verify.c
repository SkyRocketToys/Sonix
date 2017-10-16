#include <generated/snx_sdk_conf.h>
#include "cmd_verify.h"
#include "cmd_net.h"
#include <printlog.h>
#include <FreeRTOS.h>
#include <bsp.h>
#include <task.h>
#include <nonstdlib.h>
#include <stdint.h>


#include <libmid_fatfs/ff.h>
#include <netinet/in.h>
#include <libmid_cyassl/cyassl/ctaocrypt/aes.h>
#include <vc/snx_vc.h>
#include <isp/isp.h>
#include <libmid_vc/snx_mid_vc.h>

#include <libmid_fatfs/ff.h>
#include <usbh/USBH.h>
#include <usb_device/usb_device.h>
#include <libmid_usbd/mid_usbd.h>

#include <libmid_automount/automount.h>
#include <event_groups.h>

/** \defgroup cmd_verify Verify commands
 *  \ingroup system_cli
 * @{
 */

int cmd_verify_net_lwip_loopback(int argc, char* argv[])
{
	return 0;
}

int cmd_verify_net_mac_loopback(int argc, char* argv[])
{

	return 0;
}

int cmd_verify_net_iperf(int argc, char* argv[])
{
	print_msg_queue("client\n");

	return 0;
}

int cmd_verify_net_ping(int argc, char* argv[])
{
	print_msg_queue("ping\n");
	ping(argv[1],5);

	return 0;
}

void SNXAPP_SOCKET_TCP_CLIENT_TASK(void *id);
void SNXAPP_SOCKET_UDP_CLIENT_TASK(void *id);

paramtpStruct tpparam;

int cmd_verify_net_throughput(int argc, char* argv[])
{
	strcpy(tpparam.ip_addr,argv[3]);
	tpparam.port = atoi(argv[4]);
	if(!strcmp(argv[1], "tcp")){
		if(!strcmp(argv[1],"svr")){
			print_msg("server\n");
		}else if(!strcmp(argv[2], "cli")){
			
			if (pdPASS != xTaskCreate(SNXAPP_SOCKET_TCP_CLIENT_TASK, "TCPc task", 2048  , (void *) &tpparam,	255, NULL))
				print_msg("Could not create TCPc task\r\n");
		}
	}else if(!strcmp(argv[1], "udp")){
		print_msg("udp\n");
		if(!strcmp(argv[2],"svr")){
			print_msg("server\n");
		}else if(!strcmp(argv[2], "cli")){
			if (pdPASS != xTaskCreate(SNXAPP_SOCKET_UDP_CLIENT_TASK, "UDPc task", 2048  ,(void *)  &tpparam,	60, NULL ))
				print_msg("Could not create UDPc task\r\n");
		}
	}
		


	return 0;
}

//=======================================================



typedef struct usbh_msc_param{
	uint32_t counter;
	uint32_t pattern_size;
}usbh_msc_param_t;

usbh_msc_param_t usbh_msc_t;

uint8_t usbh_msc_regular_test(usbh_msc_param_t usbh_msc_t, uint32_t capacity)
{
	uint32_t addr = 0;
	uint32_t i = 1;
	size_t pattern_size = (size_t)usbh_msc_t.pattern_size;
	uint8_t  result = 0;
	uint8_t  status = 0;

	uint8_t *pattern_buf = NULL;
	uint8_t *compare_buf = NULL;

	pattern_buf = pvPortMalloc((pattern_size + 1024), GFP_DMA, MODULE_CLI);
	pattern_buf = (uint8_t*)(((uint32_t)pattern_buf & 0xFFFFFC00) + 0x400); //do 1k alignment

	compare_buf = pvPortMalloc((pattern_size + 1024), GFP_DMA, MODULE_CLI);
	compare_buf = (uint8_t*)(((uint32_t)compare_buf & 0xFFFFFC00) + 0x400); //do 1k alignment

	while(i <= usbh_msc_t.counter){
		memset(pattern_buf, i, pattern_size);
		memset(compare_buf, 0, pattern_size);

		status = msc_write(addr, (uint8_t*)pattern_buf, (pattern_size/512));
		status = msc_read((uint8_t*)compare_buf, addr, (pattern_size/512));
	
		result = memcmp(pattern_buf, compare_buf, pattern_size);
		if(result != 0){
			break;
		}
		addr += (pattern_size/512);
		if(addr >= capacity)
			break;
		i++;
	}
	vPortFree(pattern_buf);
	vPortFree(compare_buf);
	return result;
}

uint8_t usbh_msc_random_test(usbh_msc_param_t usbh_msc_t, uint32_t capacity)
{
	uint32_t addr = 0;
	uint32_t i = 1, j = 0;
	size_t pattern_size = (size_t)usbh_msc_t.pattern_size;
	uint8_t  result = 0;
	uint8_t  status = 0;

	uint8_t *pattern_buf = NULL;
	uint8_t *compare_buf = NULL;

	pattern_buf = pvPortMalloc((pattern_size + 1024), GFP_DMA, MODULE_CLI);
	pattern_buf = (uint8_t*)(((uint32_t)pattern_buf & 0xFFFFFC00) + 0x400); //do 1k alignment

	compare_buf = pvPortMalloc((pattern_size + 1024), GFP_DMA, MODULE_CLI);
	compare_buf = (uint8_t*)(((uint32_t)compare_buf & 0xFFFFFC00) + 0x400); //do 1k alignment

	while(i <= usbh_msc_t.counter){		
		memset(compare_buf, 0, pattern_size);

		for(j=0;j<pattern_size;j++){
			pattern_buf[j] = rand() % 256;
		}
		status = msc_write(addr, (uint8_t*)pattern_buf, (pattern_size/512));
		status = msc_read((uint8_t*)compare_buf, addr, (pattern_size/512));

		result = memcmp(pattern_buf, compare_buf, pattern_size);
		if(result != 0){
			break;
		}
		addr += (pattern_size/512);
		if(addr >= capacity)
			break;
		i++;
	}
	vPortFree(pattern_buf);
	vPortFree(compare_buf);
	return result;
}

void usbh_msc_rw_task(void *pvParameters)
{
	usbh_msc_param_t usbh_msc_param;
	uint32_t capacity = 0;
	uint8_t  regular_test = 0;
	uint8_t  random_test = 0;
	uint8_t  result = 0;	

	memcpy((uint8_t*)&usbh_msc_param, (uint8_t*)pvParameters, sizeof(usbh_msc_param));
	for(;;){
		if(msc_ready() == SUCCESS){
			capacity = msc_capacity();
			if(regular_test == 0){
				print_msg_queue("\ntest regular pattern.. \n");
				result = usbh_msc_regular_test(usbh_msc_param, capacity);
				if(result == 0){
					print_msg_queue("\nregular pattern test PASS \n");
					regular_test = 1;
				}
				else{
					print_msg_queue("\nregular pattern test FAIL \n");
					goto done;
				}
			}

			if(random_test == 0){
				print_msg_queue("\ntest random pattern.. \n");
				result = usbh_msc_random_test(usbh_msc_param, capacity);
				if(result == 0){
					print_msg_queue("\nrandom pattern test PASS \n");
					random_test = 1;
				}
				else{
					print_msg_queue("\nrandom pattern test FAIL \n");
					goto done;
				}
			}

			if((regular_test == 1) && (random_test == 1)){
				goto done;
			}
		}
	}
done:	
	vTaskDelete(NULL);
}

int cmd_verify_usbh_msc(int argc, char* argv[])
{
	usbh_msc_param_t usbh_msc_t;
	uint32_t timeout_wait = 0;

	memset(&usbh_msc_t, 0, sizeof(usbh_msc_t));
	usbh_msc_t.pattern_size = 1024 * 4;
	usbh_msc_t.counter = 100;

	print_msg_queue(" please plug in usb key..... \n");

	while(msc_ready() == FAIL){
		print_msg_queue(".");
		vTaskDelay(10);
		if(timeout_wait > 100){
			print_msg_queue("\nwait usb key timeout\n");
			goto out;
		}
		timeout_wait++;
	}
	
	if(pdPASS != xTaskCreate(usbh_msc_rw_task, "usbh_msc_rw_task", STACK_SIZE_4K, (void*)&usbh_msc_t, 59, NULL)){
		print_msg_queue("Could not create task usbh_msc_rw_task\n");
	}

out:
	return pdPASS;
}

int cmd_verify_usbd_msc(int argc, char* argv[])
{
	print_msg_queue("\n usb device msc test\n");

	print_msg_queue(" Please connect to the host or platform with usb cable,PC,Notebook,Linux OS,Windows OS,etc. \n");
	print_msg_queue(" Check to see if there are flash/pen drive device on system, if successful. \n");

	
	usbd_set_class_mode(USBD_MODE_MSC);
	
	return pdPASS;
}

int cmd_verify_usbd_uvc(int argc, char* argv[])
{
	print_msg_queue("\n usb device uvc test\n");

	print_msg_queue(" Please connect to the host or platform with usb cable,PC,Notebook,Linux OS,Windows OS,etc. \n");
	print_msg_queue(" Check to see if there are sensor image data through the application program, if successful. \n");

	
	usbd_set_class_mode(USBD_MODE_UVC);
	
	return pdPASS;
}


//RBK usb host iad test code **********************************************************************************

#define HOSTVERIFY_SUCCESS 0
#define HOSTVERIFY_FAIL -1
#define HOSTVERIFY_INVALID_PARAM -2

#define EVENT_INIT_DONE	 ( 1 << 0 )
#define EVENT_WORK_DONE  ( 1 << 1 )
#define EVENT_WORK_ABORT ( 1 << 2 )

#define tQUEUE_LENGTH 3
#define tQUEUE_DATASIZE 1280*720*2
#define tTOTAL_TEST_FRAME 256
#define tFRAMERATE 10

void deinitbuf(QueueHandle_t queue);
int initbuf(QueueHandle_t * queue, unsigned int uiSize, unsigned int uiHowManybuffer);

/* RBK Struct with settings for task4 */
typedef struct _paramDataGenTask

{
    portCHAR* text;                  	  /* text to be printed by the task */
    UBaseType_t  delay;              	  /* delay in milliseconds */
    unsigned int uiStreamIdx;        	  /* stream idx */
    QueueHandle_t queueBufEmpty;          /* queue for empty data buffers */
    QueueHandle_t queueBufFilled;         /* queue for filled data buffers */
     unsigned int  queueLength;
    unsigned int  frameSize;
    unsigned int  totalTestFrame;
    EventGroupHandle_t  jobEvent;         /* Event  */

} paramDataGenTask;

paramDataGenTask tParamDataGenTask[1] = {
		(paramDataGenTask) { .text="*** Generate ***", .delay=tFRAMERATE , .uiStreamIdx=1, 0, 0, 0, 0, 0, 0}
};



void vDataGenAPP( void *pvParameters )
{
	FIL fdst;      /* File objects */
	FRESULT fr;
	uint32_t bw;
	byte bData = 0;
	int i = 0;	EventBits_t uxBits = 0;
	paramDataGenTask* params = (paramDataGenTask*) pvParameters;
	UBaseType_t waitTimeOut = params->delay;
	QueueHandle_t queueBufFilled = 0;
	QueueHandle_t queueBufEmpty = 0;
	unsigned char *pbuf = NULL;

	while (!(uxBits & (EVENT_INIT_DONE|EVENT_WORK_ABORT)))
	{uxBits = xEventGroupWaitBits(params->jobEvent,EVENT_INIT_DONE,pdFALSE,pdFALSE,3000/portTICK_PERIOD_MS);}

	if (uxBits & EVENT_WORK_ABORT){
		goto out;
	}

	queueBufFilled = params->queueBufFilled;
	queueBufEmpty  = params->queueBufEmpty;


	//open record file
	fr = f_open(&fdst, "USBH_DATA", FA_CREATE_ALWAYS | FA_WRITE);

	if (fr)
	{
		print_msg("Open file for Recording fail\n");
		goto out;
	}

	for( ; ; )
	{
		pbuf = NULL;
		if (xQueueReceive(queueBufFilled,&pbuf,waitTimeOut / portTICK_RATE_MS *10))
		{
			if (pbuf)
			{
				print_msg ("vDataGenAPP: start write %X.\n",pbuf);


				//RBK check data
				for ( i=0;i<params->frameSize;i++){
					if (bData != pbuf[i])
					{
						print_msg ("vDataGenAPP: !!!!!!!!!!!!! Data error !!!!!!!!!!!!!.\n");
						goto out;
					}
				}

				bData++;

				//fr = f_write(&fdst, pbuf, params->queueLength, &bw);

				print_msg ("vDataGenAPP: stop write.\n");

				if (xQueueSend(queueBufEmpty,&pbuf,0))
				{
					print_msg_queue ("vDataGenAPP: requeue.\n");
				}else
				{
					print_msg_queue ("vDataGenAPP: requeue buffer error.\n");
					goto out;
				}
			}
		}else{
			print_msg_queue ("vDataGenAPP: dequeue buffer error.\n");
			goto out;
		}

	}
out:

	print_msg_queue ("vDataGenAPP: work finish\n");

	f_close(&fdst);
	//xSemaphoreGive(params->queueReadyDone);
	xEventGroupSetBits(params->jobEvent,EVENT_WORK_DONE);

	vTaskDelete(NULL);
}

/* for test purpose, generate data pattern. play as the data from usb host*/
void vDataGenUSBTask( void *pvParameters )
{
	const portCHAR* taskName;
	UBaseType_t  delay;
	paramDataGenTask* params = (paramDataGenTask*) pvParameters;
	static const portCHAR defaultText[] = "<NO TEXT>\r\n";
	static const UBaseType_t defaultDelay = 1000;
	unsigned char bdata = 0;
	unsigned char *pbuf = NULL;
	int i = 0;

	params->queueBufEmpty = NULL;
	params->queueBufFilled = NULL;

	QueueHandle_t queueBufEmpty = 0;
	QueueHandle_t queueBufFilled = 0;

	taskName = ( NULL==params || NULL==params->text ? defaultText : params->text );
	delay = ( NULL==params ? defaultDelay : params->delay);

	print_msg_queue("hello %s  \n", taskName);

	queueBufFilled = xQueueCreate(params->queueLength,sizeof(unsigned char*));

	if ((initbuf(&queueBufEmpty,params->frameSize, params->queueLength) == HOSTVERIFY_SUCCESS) &&
		(queueBufFilled !=0))
	{
		print_msg_queue("vDataGenUSBTask  %s  init queue done\n",taskName);

		params->queueBufEmpty = queueBufEmpty;
		params->queueBufFilled = queueBufFilled;

		xEventGroupSetBits(params->jobEvent,EVENT_INIT_DONE);

		for(i=0;i<params->totalTestFrame;i++)
		{
			/* Print out the name of this task. */

			vTaskDelay( delay / portTICK_RATE_MS );

			if (xQueueReceive(queueBufEmpty,&pbuf,0))
			{
				print_msg_queue("vDataGenUSBTask  %s  prepare data buffer %X\n",taskName,pbuf);
				if (pbuf){
					memset(pbuf,bdata,params->frameSize);
					bdata = bdata + 1;

					//add into data queue
					if (xQueueSend(queueBufFilled,(void **)&pbuf,0))
					{
						print_msg_queue ("vDataGenUSBTask: new frame is generated.\n");
					}else
					{
						//goto error leave
						goto err_leave;
					}

				}else
					print_msg_queue("vDataGenUSBTask dequeue null buffer\n");
			}else
			{
				print_msg("vDataGenUSBTask dequeue fail\n");
				xEventGroupSetBits(params->jobEvent,EVENT_WORK_ABORT);
				goto err_leave;
			}

		}
	}else
	{
		print_msg_queue("vDataGenUSBTask initbuf error.\n");
		xEventGroupSetBits(params->jobEvent,EVENT_WORK_ABORT);
		goto err_leave;
	}
	vTaskDelete(NULL);
	return ;


err_leave:

	//xSemaphoreGive(params->queueReadyDone);
	xEventGroupSetBits(params->jobEvent,EVENT_WORK_ABORT);


	/*
	 * If the task implementation ever manages to break out of the
	 * infinite loop above, it must be deleted before reaching the
	 * end of the function!
	 */


	vTaskDelete(NULL);
}

void deinitbuf(QueueHandle_t queue)
{
	unsigned char *pbuf = NULL;
	while (xQueueReceive(queue,&pbuf,0))
	{
		if (pbuf)
				vPortFree (pbuf);
	}
	vQueueDelete(queue);
}

int initbuf(QueueHandle_t * queue, unsigned int uiSize, unsigned int uiHowManybuffer)
{
	unsigned char *pbuf = NULL;
	int i=0;

	if (queue == 0)
		return HOSTVERIFY_INVALID_PARAM;

	if (uiSize==0 || uiHowManybuffer==0)
		return HOSTVERIFY_INVALID_PARAM;

	*queue = 0;
	*queue = xQueueCreate(uiHowManybuffer,sizeof(unsigned char*));

	if (*queue!=0)
	{

		for(i=0;i<uiHowManybuffer;i++)
		{
			pbuf = (unsigned char *)pvPortMalloc(uiSize,GFP_KERNEL, MODULE_APP);
			if (!pbuf) {
				print_msg_queue ("initbuf: pvPortMalloc buffer fail.\n");
				//goto error leave
				goto err_leve;
			}

			print_msg_queue ("initbuf: %X\n",pbuf);

			if (xQueueSend(*queue,(void **)&pbuf,0))
			{
				print_msg_queue ("initbuf: kick new buffer into queue.\n");
			}else
			{
				//goto error leave
				goto err_leve;
			}
		}
	}else
	{
		//goto error leave
		return HOSTVERIFY_FAIL;
	}

	return HOSTVERIFY_SUCCESS;

err_leve:

print_msg_queue ("initbuf: queue init fail.\n");

return HOSTVERIFY_FAIL;
}


int cmd_verify_usbh_iad(int argc, char* argv[])
{

	EventBits_t uxBits = 0;
	tParamDataGenTask[0].totalTestFrame = tTOTAL_TEST_FRAME;
	tParamDataGenTask[0].queueLength    = tQUEUE_LENGTH;
	tParamDataGenTask[0].frameSize      = tQUEUE_DATASIZE;


    tParamDataGenTask[0].jobEvent = xEventGroupCreate();

    /* Was the event group created successfully? */
    if( tParamDataGenTask[0].jobEvent == NULL )
    {
        /* The event group was not created because there was insufficient
        FreeRTOS heap available. */
    	goto out;
    }

    xEventGroupClearBits( tParamDataGenTask[0].jobEvent ,EVENT_INIT_DONE|EVENT_WORK_DONE|EVENT_WORK_ABORT);

	if (pdPASS != xTaskCreate(vDataGenUSBTask, "task4", STACK_SIZE_4K, (void*) &tParamDataGenTask[0],
			5, NULL))
		print_msg_queue("Could not create task4\r\n");

	if (pdPASS != xTaskCreate(vDataGenAPP, "task5", STACK_SIZE_4K, (void*) &tParamDataGenTask[0],
			5, NULL))
		print_msg_queue("Could not create task5\r\n");


	while (!(uxBits & (EVENT_WORK_DONE|EVENT_WORK_ABORT)))
	{	uxBits = xEventGroupWaitBits(tParamDataGenTask[0].jobEvent,EVENT_WORK_DONE|EVENT_WORK_ABORT,pdFALSE,pdFALSE,3000/portTICK_PERIOD_MS);
	}

	vEventGroupDelete(tParamDataGenTask[0].jobEvent);

	//Release queue
	if (tParamDataGenTask[0].queueBufEmpty == NULL)
		deinitbuf(tParamDataGenTask[0].queueBufEmpty);
	if (tParamDataGenTask[0].queueBufFilled == NULL)
		deinitbuf(tParamDataGenTask[0].queueBufFilled);

	tParamDataGenTask[0].queueBufEmpty = NULL;
	tParamDataGenTask[0].queueBufFilled = NULL;

out:
	return pdPASS;
}

/** @} */
