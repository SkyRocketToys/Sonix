/**
 * @file
 * this is udp command for uart2 Tx/Rx use.
 * @author 
 */
#include <FreeRTOS.h>
#include <bsp.h>
#include <task.h>
#include <queue.h>
#include <nonstdlib.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <libmid_rtsp_server/rtsp_server.h>
#include <libmid_nvram/snx_mid_nvram.h>
#include "json_cmd.h"
#include "socket_ctrl.h"
#include "online_fw_upgrade.h"
#include "video_main.h"
#include <uart/uart.h>
#include "udp_cmd.h"
#include <gpio/gpio.h>

#include <libmid_fatfs/ff.h>
#include "../main_flow/record/rec_schedule.h"
#include "../main_flow/playback/play_back.h"

#include "../../../../middleware/rtsp/src/rtsp_common.h"


#define UDP_PRINT(level, fmt, args...) print_q(level, "[udp]%s: "fmt, __func__,##args)


#define    PRIORITY_TASK_UDP_CMD    38//PRIORITY_TASK_APP_CMDDAEMON/////4
#define    PRIORITY_TASK_RX_CMD      3

#define    PRIORITY_TASK_WIFI_LED      2
#define    PRIORITY_TASK_REC_LED       2


#define GPIO_WRITE    1
#define GPIO_READ      1
#ifdef EVB_LED_GPIO
#define LED_POWER     2
#define LED_RECORD    0
#else
#define LED_POWER     0
#define LED_RECORD    1
#endif
static	int g_udp_sock = 0;
static	xTaskHandle udp_recv_process;


#define STATUS_CAP     		0x00
#define STATUS_REC    		0x01
#define STATUS_REC_STOP      0x02
#define STATUS_FAIL               0x03

static	uint8_t g_rec_status = 0x01;

extern volatile int sd_card_remove;
extern RtspServer server;

#ifdef SUPPORT_UART2_RX_CMD
static 	xTaskHandle uart2_rx_process;
static	QueueHandle_t uart2RxQueue;
static  	udp_cmd_data_t recv_cmd;


static  SDSTATE check_sd_state()
{
	int result;
	if (get_sd_umount_err())
	{
		result=get_sd_status();
		if (result==FR_NO_FILESYSTEM)//have sdcard but not fat32
		{
			return CARD_FORMAT_ERROR;

		}else
		{			
			return CARD_NOT_EXIST;
		}	 
	}
	return  CARD_OK;
}
/**
* @brief interface function - Rx record start.
* @param flag error number.
*/
unsigned char udp_recstart(void)
{
	uint8_t stats= 0 ;

	UDP_PRINT(SYS_DBG, "UDP : Rec Start\n");

	SDSTATE sdstate;
	sdstate=check_sd_state();
	if (sdstate!=CARD_OK)
	{
		if (sdstate==CARD_NOT_EXIST)
		{
			UDP_PRINT(SYS_WARN, "SD Card doesn't exist\n");
			stats = 0x02;
		}
		else if (sdstate==CARD_FORMAT_ERROR)
			UDP_PRINT(SYS_ERR, "SD Format error exist\n");
			stats = 0x10;		
	}
	if (schedrec_get_isfull() == 1) {
		UDP_PRINT(SYS_INFO, "record folder is full.\n");
		stats = 0x03;

	}	

	if ((sd_card_remove) ||(json_get_format_status() == 1)) {
		stats = 0x01;	// Record start fail
	} else if (pb_state()) {
		stats = 0x01;	// Record start fail
	} else {
		schedrec_suspend_restart(1);
#if 0//ndef CONFIG_APP_DRONE
		reclapse_suspend_restart(1);
#endif
		stats = 0x00;	// Record start OK
	}

	return stats;
}

/**
* @brief interface function - Rx record stop.
* @param flag error number.
*/
unsigned char udp_recstop(void)
{
	uint8_t stats = 0;

	UDP_PRINT(SYS_DBG, "UDP : Rec Stop\n");

	if ((sd_card_remove) || (json_get_format_status() == 1)) {
		stats = 0x01;	// Record start fail
	} else {
		user_diable_rec();
#if 0//ndef CONFIG_APP_DRONE
		user_disable_reclapse();
#endif
		stats = 0x00;	// Record start OK
	}

	return stats;
}

/**
* @brief interface function - Rx snapshot.
* @param flag error number.
*/
unsigned char udp_snapshot(void)
{
	uint8_t stats = 0;

	UDP_PRINT(SYS_DBG, "UDP : Snap Shot\n");

	SDSTATE sdstate;
	sdstate=check_sd_state();
	if (sdstate!=CARD_OK)
	{
		if (sdstate==CARD_NOT_EXIST)
		{
			UDP_PRINT(SYS_WARN, "SD Card doesn't exist\n");
			stats = 0x02;
		}
		else if (sdstate==CARD_FORMAT_ERROR)
			stats = 0x10;
		//goto resp_json;
	}
	
	if (mf_snapshot_get_isfull() == 1) {
		UDP_PRINT(SYS_INFO, "photo folder is full.\n");
		stats = 0x04;
		//goto resp_json;
	}
	
	if (check_takepic_task() != 0)
	{
		UDP_PRINT(SYS_WARN, "Continuous shooting is already running.\n");
		stats = 0x03;
		//goto resp_json; 
	}


	if ((sd_card_remove) || (json_get_format_status() == 1)) {
		stats = 0x01;
	} else {
		if (pb_state()) {
			UDP_PRINT(SYS_ERR, "Now in playback. Snapshot fail....\n");
		} else {
			set_takepic_num(1);
			mf_set_snapshot(1);
		}

		stats = 0x00;
	}

	return stats;

}

/**
* @brief interface function - Get Rx data test vidoe function. 
*/
void uart2_rx_cmd(void)
{
	//xQueueReceive(uart2CmdQueue, &recv_cmd, portMAX_DELAY);
	if (recv_cmd.Data10 == 0x80) {		
		if (udp_snapshot()==0){
			g_rec_status = STATUS_CAP;
		}else {
		      g_rec_status = STATUS_FAIL;
		   UDP_PRINT(SYS_ERR, "========udp_snapshot fail\n");
		}   
		//UDP_PRINT(SYS_DBG, "tran_cmd.Data10 = [%x]\n", tran_cmd.Data10);
		//udp_info.uart2_write = 0;
		
	} else if (recv_cmd.Data10 == 0x81) {		
		if (udp_recstart() == 0){
			g_rec_status = STATUS_REC;
		}else{
		      g_rec_status = STATUS_FAIL;
		   UDP_PRINT(SYS_ERR, "==========udp_recstart fail\n");	
		//UDP_PRINT(SYS_DBG, "tran_cmd.Data10 = [%x]\n", tran_cmd.Data10);
		//udp_info.uart2_write = 0;
		}
		
	} else if (recv_cmd.Data10 == 0x82) {		
		if (udp_recstop()==0){
			g_rec_status = STATUS_REC_STOP;
		}else{
		      g_rec_status = STATUS_FAIL;
		   UDP_PRINT(SYS_ERR, "===========udp_recstop fail\n");	
		//UDP_PRINT(SYS_DBG, "tran_cmd.Data10 = [%x]\n", tran_cmd.Data10);
		//udp_info.uart2_write = 0;
		}
	}		
		
}

/**
* @brief interface function - Get uart2 data and send Queue from ISR. 
*/
void udp_uart2_isr_handler(int irq)
{
	uint8_t i;
	uint32_t val;
	
    cpu_udelay(3000);
	val = inl((unsigned*)(BSP_UART2_BASE_ADDRESS + FIFO_THD));
	val = (val >> 24) & 0x3F;
	if (val > 30) {
		for (i = 0 ; i < 32 ; i++) {
			val = inl((unsigned*)(BSP_UART2_BASE_ADDRESS + RS_DATA));
		}
//		UDP_PRINT(SYS_DBG, "UART2 FIFO FULL!!!\n\r");
		return;
	}
	/* Get the received mcu command from the UART */
	while(inl((unsigned*)(BSP_UART2_BASE_ADDRESS + UART_CONFIG))&RX_RDY){
		for(i=0; i<sizeof(udp_cmd_data_t); i++){
			*(((char*)&recv_cmd) + i)  = (char)inl((unsigned*)(BSP_UART2_BASE_ADDRESS + RS_DATA));
		}
	}

	xQueueSendFromISR(uart2RxQueue, (void*)&recv_cmd, 0);//test rx to tx
	
	uart2_clear_rx_interrupt();
	
	return;
}

/**
* @brief interface function - Get uart2 data and run video function. 
*/
void uart2_rx_task_process(void)
{
	memset(&recv_cmd, 0, sizeof(udp_cmd_data_t));	
			
	while (1) {
		xQueueReceive(uart2RxQueue, &recv_cmd, portMAX_DELAY);
    	uart2_rx_cmd(); 
		vTaskDelay(100/portTICK_RATE_MS );		
	}

	vTaskDelete(NULL);
}
#endif

/**
* @brief interface function - Get udp byte type from nvram.
*/
int get_udp_type_nvram(void)
{
   int udp_val;

    if (snx_nvram_integer_get("WIFI_DEV", "UDP_TYPE", &udp_val) != NVRAM_SUCCESS) {
        udp_val = 0; 
		snx_nvram_integer_set("WIFI_DEV", "UDP_TYPE", udp_val);
		UDP_PRINT(SYS_DBG, "udp type default = [%d]\n",udp_val);
    } else {		
		UDP_PRINT(SYS_DBG, "udp type success = [%d]\n",udp_val);
    }

	return udp_val;
	
}

/**
* @brief interface function - Checksum compare UDP data. 
*/
uint8_t udp_checksum(udp_cmd_data_t* tran_cmd, udp_info_t* udp_info)
{	
	uint8_t checksum;

    //check sum
    if (udp_info->type == 1) {
		checksum = (tran_cmd->Data1+tran_cmd->Data2+tran_cmd->Data3+tran_cmd->Data4\
		+tran_cmd->Data5+tran_cmd->Data6+tran_cmd->Data7+tran_cmd->Data8+tran_cmd->Data9+tran_cmd->Data10)&0xff;
        if (tran_cmd->Data11== checksum)
			udp_info->uart2_write = 1;  
		else
			udp_info->uart2_write = 0;		
    } else if (udp_info->type == 2) {
		checksum = (tran_cmd->Data1^tran_cmd->Data2^tran_cmd->Data3^tran_cmd->Data4\
		^tran_cmd->Data5)&0xff;  
        if (tran_cmd->Data6 == checksum)
			udp_info->uart2_write = 1;  
		else
		   	udp_info->uart2_write = 0;		
    } else if (udp_info->type == 3){
		checksum = (tran_cmd->Data2+tran_cmd->Data3+tran_cmd->Data4\
		+tran_cmd->Data5+tran_cmd->Data6+tran_cmd->Data7+tran_cmd->Data8+tran_cmd->Data9+tran_cmd->Data10)&0xff; 
        if (tran_cmd->Data11 == checksum)
			udp_info->uart2_write = 1;  
		else
		   	udp_info->uart2_write = 0;
    }else{//udp_info->type == 0
		checksum = (tran_cmd->Data2+tran_cmd->Data3+tran_cmd->Data4\
		+tran_cmd->Data5+tran_cmd->Data6+tran_cmd->Data7+tran_cmd->Data8+tran_cmd->Data9+tran_cmd->Data10)&0xff; 
        if (tran_cmd->Data11 == checksum)
			udp_info->uart2_write = 1;  
		else
		   	udp_info->uart2_write = 0;

    }	

	return checksum;

}

/**
* @brief interface function - Get UDP byte size and switch baudrate. 
*/
void udp_get_type(udp_info_t* udp_info)
{	
	udp_info->type = get_udp_type_nvram();
	UDP_PRINT(SYS_DBG, "============udp_info.type ========= [%d]\n",udp_info->type);
    if (udp_info->type == 1) {//11byte
        udp_info->size = 13;
		uart2_init(19200);
    } else if (udp_info->type == 2) {//8byte
        udp_info->size = 8;
		uart2_init(19200);
    } else if (udp_info->type == 3){//13byte
		udp_info->size = 13;
		uart2_init(9600);    
    }else {//udp_info->type == 0
		udp_info->size = 13;
		uart2_init(115200); 
    }	
}

#define MAX_RETRY_CNT	200
#define MAX_DELAY_UDP_TIME	400
#define MIN_DELAY_UDP_TIME	10

/**
* @brief interface function - Get udp byte type from nvram. 
*/
void udp_recv_task_process(void *pvParameters)
{
	int len;
   	int i;
   	int cnt= 0;
	udp_info_t 	udp_info;
	udp_cmd_data_t tran_cmd;	
	memset(&tran_cmd, 0, sizeof(udp_cmd_data_t));
	memset(&udp_info, 0, sizeof(udp_info_t));
       udp_get_type(&udp_info);
	  
	#ifdef SUPPORT_UART2_RX_CMD
	uart2_register_irq(UART2_IRQ_NUM, &udp_uart2_isr_handler);//test RX 
	#endif
	outl((unsigned int*)(BSP_UART2_BASE_ADDRESS + FIFO_THD), 0x0101L);

	
	struct sockaddr_in addr;
	int addr_len = sizeof(struct sockaddr_in);
	
	UDP_PRINT(SYS_DBG, "hello! %s\n", "UDP cmd task");

	if ((g_udp_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		UDP_PRINT(SYS_ERR, "create socket failed\n");
		goto end;
	}
	
	memset(&addr, 0x0, sizeof(addr));
	
	{
		int flags;
		if ((flags = fcntl(g_udp_sock, F_GETFL, 0)) < 0) {
			UDP_PRINT(SYS_ERR, "fcntl(%d, F_GETFL)", g_udp_sock);
			goto end;
		}

		if (fcntl(g_udp_sock, F_SETFL, flags | O_NONBLOCK) == -1) {
			UDP_PRINT(SYS_ERR, "fcntl(%d, F_SETFL)", g_udp_sock);
			goto end;
		}
   }

	addr.sin_family = AF_INET;
	addr.sin_port = htons(UDP_CMD_PORT);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(g_udp_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		UDP_PRINT(SYS_ERR, "binding failed!!\n");
		closesocket(g_udp_sock);
		g_udp_sock = 0;
		goto end;
	}
	
	while(1)
	{
		memset(&tran_cmd, 0x0, sizeof(udp_cmd_data_t));
	
		if ((len = recvfrom(g_udp_sock, &tran_cmd, sizeof(tran_cmd), 0, (struct sockaddr*)&addr, (socklen_t *)&addr_len)) <0) {
			//UDP_PRINT("could not read datagram!!");
		
			if (cnt >= MAX_RETRY_CNT){
				cnt = MAX_RETRY_CNT-1;
				vTaskDelay(MAX_DELAY_UDP_TIME/portTICK_RATE_MS );
				UDP_PRINT(SYS_DBG, "sleep %d \n", MAX_DELAY_UDP_TIME);	
			}else{
				vTaskDelay(MIN_DELAY_UDP_TIME/portTICK_RATE_MS );
				cnt++;
				UDP_PRINT(SYS_DBG, "sleep %d \n", MIN_DELAY_UDP_TIME);	
			}
				
			continue;
		} else {
			cnt = 0;
			//UDP_PRINT("recv packets %d from %s\n", len, inet_ntoa(addr.sin_addr));
			//UDP_PRINT(SYS_DBG, "buffer = 0X%02x\n",buffer);
			//xQueueSendToBack(uart2CmdQueue, (uint8_t*) tran_cmd, 50);
		}

		udp_checksum(&tran_cmd, &udp_info);

		if (udp_info.uart2_write == 1) {			
			while((inl((uint32_t*)(BSP_UART2_BASE_ADDRESS + UART_CONFIG))&TX_RDY) == 0);

			for(i=0; i < udp_info.size; i++) {
				outl((uint32_t*)((BSP_UART2_BASE_ADDRESS + RS_DATA)), *(((char*)&tran_cmd) + i));
			}
		} else {
			UDP_PRINT(SYS_ERR, "UDP data checksum error\n");
		}
				
		/*//Send	
		memset(buffer, 0x0, sizeof(buffer));
		len = snprintf(buffer, 100, "Hello, This is a test!!");
		sendto(g_udp_sock, buffer, len, 0, &addr, addr_len);
		*/
		//vTaskDelay(1/portTICK_RATE_MS );////////test
	}
	/*
	 * If the task implementation ever manages to break out of the
	 * infinite loop above, it must be deleted before reaching the
	 * end of the function!
	 */
end:
	vTaskDelete(NULL);

}

/**
* @brief interface function - release all memory. 
*/
void udp_cmd_socket_uninit(void)
{
	vTaskDelete(udp_recv_process);

#ifdef SUPPORT_UART2_RX_CMD
	vTaskDelete(uart2_rx_process);
	vQueueDelete(uart2RxQueue);
#endif	
	if (g_udp_sock != 0)
		closesocket(g_udp_sock);
}

#ifdef SUPPORT_LED_GPIO

void led_gpio_ctrl(int num, int mode, int val)
{

	gpio_pin_info info;

	/*
		Start to Control GPIO:
		1. OPEN GPIO
		2. config its info structure
		3. write opt for output mode
		4. read opt for input mode
		5. close GPIO
	*/
#ifdef EVB_LED_GPIO
      snx_gpio_open();
#else
	snx_pwm_gpio_open();
#endif
	info.pinumber = num;
	info.mode = mode;
#ifdef EVB_LED_GPIO	
	info.value = val;
#else
      info.value = ~(val);
#endif


#if 1//def EVB_LED_GPIO
	{
		uint32_t reg;
		uint32_t bypass = 0;
		reg = gpio_get_bypass();
		reg &= ~(1 << info.pinumber);
		reg |= (bypass << info.pinumber);
		gpio_set_bypass(reg);
	}
#endif
	if (mode == 1){
#ifdef EVB_LED_GPIO		
		snx_gpio_write(info);
#else
		snx_pwm_gpio_write(info);
#endif

	}
	if (mode == 0)
	{
#ifdef EVB_LED_GPIO	
		snx_gpio_read(&info);
#else
		snx_pwm_gpio_read(&info);
#endif
	}

#ifdef EVB_LED_GPIO		
	snx_gpio_close();
#else
      snx_pwm_gpio_close();
#endif


	return pdPASS;   
}

void led_wifi_process(void)
{
	bool bval=0;	
			
	while (1) 
	{
		bval = (bval == 1)? (0): (1);

		if (WiFiAPLinkNum() == 0){
			led_gpio_ctrl(LED_POWER,GPIO_WRITE,bval);
			
		vTaskDelay(900/portTICK_RATE_MS );
		}else {//get IP link
			if (server.nb_connections == 0){//if (socket_get_cur_connect_num() == 0){
				led_gpio_ctrl(LED_POWER,GPIO_WRITE,bval);
				vTaskDelay(400/portTICK_RATE_MS );
			}else {
				led_gpio_ctrl(LED_POWER,GPIO_WRITE,0);
			}
		}
		 	
		vTaskDelay(100/portTICK_RATE_MS );
	}
	vTaskDelete(NULL);
}	

int led_rec_flash(bool val)
{
    if (g_rec_status == STATUS_CAP) {
		led_gpio_ctrl(LED_RECORD,GPIO_WRITE,0);
		vTaskDelay(900/portTICK_RATE_MS );
    } else if (g_rec_status == STATUS_REC) {
		led_gpio_ctrl(LED_RECORD,GPIO_WRITE,val);
		vTaskDelay(900/portTICK_RATE_MS );
    } else if (g_rec_status == STATUS_REC_STOP) {
		led_gpio_ctrl(LED_RECORD,GPIO_WRITE,1);
    }	else {//STATUS_FAIL
		led_gpio_ctrl(LED_RECORD,GPIO_WRITE,1);	 
    }
}

void led_rec_process(void)
{
	bool bval=0;	
			
	while (1) 
	{
		//rec_status = chk_rec_enable();
		//rec_running= schedrec_state();	
		if ( (sd_card_remove) || (app_uti_is_format_running() == 1) ){
			g_rec_status = STATUS_REC_STOP;//stats = KS_RECORD_START_NO_CARD;
		}
		else if (chk_rec_enable() == 0){
			if (mf_snapshot_status() == 1)
				g_rec_status = STATUS_CAP;
			else
				g_rec_status = STATUS_REC_STOP;		     
		}else if (pb_state()){
			g_rec_status = STATUS_REC_STOP;//stats = KS_RECORD_START_IN_PLAYBACK;
		}
		else{
			if (mf_snapshot_status() == 1){
				g_rec_status = STATUS_CAP;		
			}else{
				g_rec_status = STATUS_REC;//stats = KS_RECORD_START_OK;
			}
		}

		bval = (bval == 1)?(0):(1);

		led_rec_flash(bval);
				 	
		vTaskDelay(100/portTICK_RATE_MS );
	}
	vTaskDelete(NULL);
}	
#endif

/**
* @brief interface function - udp command socket and UART2 task create. 
*/
int udp_cmd_socket_create(void)
{
#ifdef SUPPORT_LED_GPIO
	if (pdPASS != xTaskCreate(led_wifi_process, "led_wifi", STACK_SIZE_1K, NULL, PRIORITY_TASK_WIFI_LED,NULL)) {
		UDP_PRINT(SYS_ERR, "Could not create task led_wifi\n");
	}

	if (pdPASS != xTaskCreate(led_rec_process, "led_rec", STACK_SIZE_1K, NULL, PRIORITY_TASK_REC_LED,NULL)) {
		UDP_PRINT(SYS_ERR, "Could not create task led_rec\n");
	}	
#endif
	if (pdPASS != xTaskCreate(udp_recv_task_process, "udp_recv", STACK_SIZE_2K, NULL, PRIORITY_TASK_UDP_CMD,&udp_recv_process)) {
		UDP_PRINT(SYS_ERR, "Could not create task socket event\n");
		goto fail1;
	}

#ifdef SUPPORT_UART2_RX_CMD
{

    uart2RxQueue = xQueueCreate(UDP_CMD_QUEUE_BUF, sizeof(udp_cmd_data_t)); 

	if (uart2RxQueue == NULL) {
		UDP_PRINT(SYS_ERR, "FlyCmdQueue create fail\n");
		goto fail3;
	}
	if (pdPASS != xTaskCreate(uart2_rx_task_process, "uart2_rx", STACK_SIZE_1K, NULL,
		PRIORITY_TASK_RX_CMD, &uart2_rx_process)) {
		UDP_PRINT(SYS_ERR, "Could not create client task socket event\n");
		goto fail4;
	}
}	
#endif

	return pdPASS;

#ifdef SUPPORT_UART2_RX_CMD
fail4:
vTaskDelete(uart2_rx_process);

fail3:
vQueueDelete(uart2RxQueue);
#endif

fail1:
vTaskDelete(udp_recv_process);
	return pdFAIL;	
}


