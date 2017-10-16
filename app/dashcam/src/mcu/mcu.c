/**
 * @file
 * this is mcu file mcu.c
 * @author CJ
 */
 
#include <FreeRTOS.h>
#include <task.h>
#include <bsp.h>
#include <string.h>
#include <nonstdlib.h>
#include <queue.h>
#include <semphr.h>
#include <wifi/wifi_api.h>
#include <gpio/gpio.h>
#include <wdt/wdt.h>
#include <string.h>
#include <stdio.h>
#include <libmid_rtsp_server/rtsp_server.h>
#include <libmid_nvram/snx_mid_nvram.h>
#include "../main_flow/record/rec_schedule.h"
#include "../main_flow/playback/play_back.h"
#include "../main_flow/video/video_main.h"
#include "../main_flow/daemon/json_cmd.h"
#include "../main_flow/daemon/socket_ctrl.h"
#include "../main_flow/record/rec_protect.h"
#include "../main_flow/main_flow.h"
#include "mcu.h"

#define MCU_DEBUG	0
#define APP_MCU_PRINT(fmt, args...) print_msg("[mcu]%s: "fmt, __func__,  ##args)
#define APP_MCU_PRINT_QUEUE(fmt, args...) print_msg_queue("[mcu]%s: "fmt, __func__,##args)
#define APP_MCU_DEBUG_PRINT(fmt, args...) if(MCU_DEBUG) \
	print_msg_queue("[mcu]%s: "fmt, __func__,##args)

#define	TX_MODE_UART				1
#define	TX_STOP_BIT_2				1<<1
#define	TX_RDY						1<<2
#define	RX_MODE_UART				1<<4
#define	RX_RDY						1<<5
#define	RS485_EN					1<<7
#define	RS232_DMA_TX_EN				1<<8
#define	RS232_DMA_RX_EN				1<<9
#define	DMA_TX_BURST_MODE			1<<10
#define	DMA_RX_BURST_MODE			1<<11
#define	RS232_TX_INT_EN_BIT			1<<16
#define	RS232_RX_INT_EN_BIT			1<<17
#define	RS232_TX_INT_CLR_BIT		1<<18
#define	RS232_RX_INT_CLR_BIT		1<<19

#define	UART_CONFIG					0x0
#define	UART_CLOCK					0x0C
#define	RS_DATA						0x10
#define	FIFO_THD					0x18

#define SP_OFFSET_WIFI				0
#define SP_OFFSET_RECORD			2
#define SP_OFFSET_98660				4
#define SP_OFFSET_SENSOR			6

#define KS_WIFI_RST_FINISH			2
#define KS_SENSOR_RST_FINISH		4

#define	MCU_QUEUE_BUF				6
#define UART2_TIME_OUT				2000000

enum MCU_CMD_COMMON{
	CMD_SYNC = 0,
	CMD_KEY,
};

enum MCU_CMD_KEY{
	CMD_RECORD_START = 0x80,
	CMD_RECORD_STOP,
	CMD_SNAP_SHOT,
	CMD_FILE_LOCK,
	CMD_WIFI_ON,
	CMD_WIFI_OFF,
	CMD_PARA_RESET,
	CMD_RESET_STATUS,
	CMD_USB_DETECTION,
	CMD_EMERGENCY_LOCK,
};

enum SYNC_STATUS{
	SS_NORMAL = 0,
	SS_SYS_ABNORMAL,
	SS_SENSOR_FAIL,
	SS_WIFI_FAIL,
	SS_FIRMWARE_UPGRADE = 0x10,
};

enum SYNC_PARA_WIFI{
	SP_WIFI_OFF = 0,
	SP_WIFI_ON_NONLINK,
	SP_WIFI_ON_LINK,
	SP_WIFI_ON,
};

enum WIFI_STATUS{
	WIFI_OFF = 0,
	WIFI_ON,
};

enum SYNC_PARA_RECORD{
	SP_RECORD_WORKING = 0,
	SP_RECORD_NO_CARD,
	SP_RECORD_IN_PLAYBACK,
	SP_RECORD_STOP,
};

enum SYNC_PARA_98660{
	SP_660_NORMAL = 0,
	SP_660_SEN_ABNORMAL,
	SP_660_WIFI_ABNORMAL,
};

enum SYNC_PARA_SENSOR{
	SP_SEN_WORKING = 0,
	SP_SEN_STOP,
};

enum KEY_STATUS_RECORD_START{
	KS_RECORD_START_OK = 0,
	KS_RECORD_START_NO_CARD,
	KS_RECORD_START_IN_PLAYBACK,
};

enum KEY_STATUS_RECORD_STOP{
	KS_RECORD_STOP_OK = 0,
	KS_RECORD_STOP_NO_CARD,
};

enum KEY_STATUS_SNAP_SHOT{
	KS_SNAP_SHOT_OK = 0,
	KS_SNAP_SHOT_FAIL,
	KS_SNAP_SHOT_NO_CARD,
};

enum KEY_STATUS_FILE_LOCK{
	KS_FILE_LOCK_OK = 0,
	KS_FILE_LOCK_FAIL,
	KS_FILE_LOCK_NO_CARD,
};

enum KEY_STATUS_WIFI_ON{
	KS_WIFI_ON_OK = 0,
};

enum KEY_STATUS_WIFI_OFF{
	KS_WIFI_OFF_OK = 0,
};


typedef struct _mcu_command{
	uint8_t Cmd;							/**<  Avi Index load finished and ready to use */
	uint8_t CheckSum;
	uint8_t Data0;
	uint8_t Data1;
	uint8_t Data2;
}mcu_command_t;

typedef struct mcu_cmd_counter {
	unsigned int cnt_sync;
	unsigned int cnt_recstart;
	unsigned int cnt_recstop;
	unsigned int cnt_snapshot;
	unsigned int cnt_filelock;
	unsigned int cnt_wifion;
	unsigned int cnt_wifioff;
	unsigned int cnt_usbdevice;
} mcu_cmd_counter_t;

mcu_cmd_counter_t cmd_cnt_t;

extern volatile int sd_card_remove;
static QueueHandle_t McuCmdQueue;
static xTaskHandle McuRecvTask;
uint32_t SysErrFlag = 0;
uint32_t WifiStatus = WIFI_ON;
uint32_t TaskRunFlag = 0;

//mcu_cmd_counter_t *get_mcu_command_counter()
//{
//	return cmd_cnt_t;
//}

void show_mcu_command_counter()
{
	print_msg_queue("sync = %d\n", cmd_cnt_t.cnt_sync);
	print_msg_queue("recstart = %d\n", cmd_cnt_t.cnt_recstart);
	print_msg_queue("recstop = %d\n", cmd_cnt_t.cnt_recstop);
	print_msg_queue("snapshot = %d\n", cmd_cnt_t.cnt_snapshot);
	print_msg_queue("filelock = %d\n", cmd_cnt_t.cnt_filelock);
	print_msg_queue("wifion = %d\n", cmd_cnt_t.cnt_wifion);
	print_msg_queue("wifioff = %d\n", cmd_cnt_t.cnt_wifioff);
	print_msg_queue("usbdevice = %d\n", cmd_cnt_t.cnt_usbdevice);
}

/**
* @brief interface function - set error information to mcu.
* @param flag error number.
*/
void mcu_set_err_flag(enum SYSTEM_ERROR_FLAG flag)
{
	SysErrFlag |= (1 << flag);
	APP_MCU_PRINT_QUEUE("set mcu flag 0x%x....\n", flag);
}

/**
* @brief interface function - clear error flag.
* @param flag error number.
*/
void mcu_clear_err_flag(enum SYSTEM_ERROR_FLAG flag)
{
	SysErrFlag &= ~(1 << flag);
}

/**
* @brief interface function - reset flag to zero.
*/
void mcu_reset_err_flag(void)
{
	SysErrFlag = 0;
}

void mcu_uart2_clear_rx_interrupt(void)
{
	unsigned int tmp = 0x0;

	tmp = inl(BSP_UART2_BASE_ADDRESS + UART_CONFIG);
	tmp |= RS232_RX_INT_CLR_BIT;
	outl(BSP_UART2_BASE_ADDRESS + UART_CONFIG, tmp);
}

/**
* @brief interrupt function - receive and send command to queue when TaskRunFlag is 1.
* @param irq interrupt number.
*/
void mcu_uart2_isr_handler(int irq)
{
	uint8_t i;
	uint32_t val;
	uint32_t cnt = 0;
	mcu_command_t recv_cmd;
	mcu_command_t tran_cmd;

	mcu_uart2_clear_rx_interrupt();
	
	val = inl((unsigned int*)(BSP_UART2_BASE_ADDRESS + FIFO_THD));
	val = (val >> 24) & 0x3F;
	if(val > 30){
		for(i=0; i<32; i++){
			val = inl((unsigned int*)(BSP_UART2_BASE_ADDRESS + RS_DATA));
		}
		APP_MCU_PRINT("MCU FIFO FULL!!!\n\r");
		return;
	}

	/* Get the received mcu command from the UART */
	while(inl((unsigned int*)(BSP_UART2_BASE_ADDRESS + UART_CONFIG))&RX_RDY){
		for(i=0; i<sizeof(mcu_command_t); i++){
			*(((uint8_t*)&recv_cmd) + i)  = (uint8_t)inl((unsigned int*)(BSP_UART2_BASE_ADDRESS + RS_DATA));
		}
	}
	
	//APP_MCU_PRINT("MCU isr running..... %x\n\r", TaskRunFlag);

	if(TaskRunFlag == 1){
		xQueueSendToBackFromISR(McuCmdQueue, (void*)&recv_cmd, 0);
	}
	else{
		if(recv_cmd.Cmd == 0x80){
			tran_cmd.Cmd = recv_cmd.Cmd;
			tran_cmd.Data0 = (SysErrFlag == 0) ? 0 : 1;
			tran_cmd.Data1 = recv_cmd.Data0;
			tran_cmd.Data2 = 13;		//wifi on and record stop
			tran_cmd.CheckSum = tran_cmd.Cmd + tran_cmd.Data0 + tran_cmd.Data1 + tran_cmd.Data2;
		}
		else if(recv_cmd.Cmd == 0x81){
			tran_cmd.Cmd = recv_cmd.Cmd;
			tran_cmd.Data0 = 0;
			tran_cmd.Data1 = 0xFF - tran_cmd.Data0;
			tran_cmd.Data2 = recv_cmd.Data0;
			tran_cmd.CheckSum = tran_cmd.Cmd + tran_cmd.Data0 + tran_cmd.Data1 + tran_cmd.Data2;
		}
		else{
			return;
		}

		cnt = 0;
		while(!((inl((unsigned int*)(BSP_UART2_BASE_ADDRESS + UART_CONFIG))&TX_RDY) && (cnt > 2000))){
			cnt++;
			
			if(cnt > UART2_TIME_OUT){
				APP_MCU_PRINT("MCU set command timeout.....\n");
				return;
			}
		}
		
		for(i=0; i<5; i++){
			outl((unsigned int*)((BSP_UART2_BASE_ADDRESS + RS_DATA)), *(((char*)&tran_cmd) + i));
		}
		
		//APP_MCU_PRINT("Set sync to MCU\n\r");
	}
}

#pragma GCC diagnostic ignored "-Wuninitialized"
/**
* @brief connect mcu with no task when system boot up.
*/
void mcu_connect_notask(void)
{
	uint8_t i;
	uint32_t val;
	uint32_t cnt = 0;
	mcu_command_t recv_cmd;
	mcu_command_t tran_cmd;
	recv_cmd.CheckSum=0;recv_cmd.Cmd=0;recv_cmd.Data0=0;recv_cmd.Data1=0;recv_cmd.Data2=0;
    tran_cmd.CheckSum=0;tran_cmd.Cmd=0;tran_cmd.Data0=0;tran_cmd.Data1=0;tran_cmd.Data2=0;
	
	val = inl((unsigned int*)(BSP_UART2_BASE_ADDRESS + FIFO_THD));
	val = (val >> 24) & 0x3F;
	if(val > 30){
		for(i=0; i<32; i++){
			val = inl((unsigned int*)(BSP_UART2_BASE_ADDRESS + RS_DATA));
		}
		APP_MCU_PRINT("MCU FIFO FULL!!!\n\r");
		return;
	}

	//wait mcu command
	cnt = 0;
	while((inl((unsigned int*)(BSP_UART2_BASE_ADDRESS + UART_CONFIG))&RX_RDY) == 0){
		cnt++;
		if(cnt > UART2_TIME_OUT){
			APP_MCU_PRINT("MCU get command timeout....\n");
			return;
		}
	}

	cnt = 0;
	while(inl((unsigned int*)(BSP_UART2_BASE_ADDRESS + UART_CONFIG))&RX_RDY){
		for(i=0; i<sizeof(mcu_command_t); i++){
			*(((uint8_t*)&recv_cmd) + i)  = (uint8_t)inl((unsigned int*)(BSP_UART2_BASE_ADDRESS + RS_DATA));
		}
		
		cnt++;
		if(cnt > UART2_TIME_OUT){
			APP_MCU_PRINT("MCU get command timeout....\n");
			return;
		}
	}
	
	//APP_MCU_PRINT("get mcu command...........%x \n\r",cnt);

	if(recv_cmd.Cmd == 0x80){
		tran_cmd.Cmd = recv_cmd.Cmd;
		tran_cmd.Data0 = (SysErrFlag == 0) ? 0 : 1;
		tran_cmd.Data1 = recv_cmd.Data0;
		tran_cmd.Data2 = 13;		//wifi on and record stop
		tran_cmd.CheckSum = tran_cmd.Cmd + tran_cmd.Data0 + tran_cmd.Data1 + tran_cmd.Data2;
	}
	else if(recv_cmd.Cmd == 0x81){
		tran_cmd.Cmd = recv_cmd.Cmd;
		tran_cmd.Data0 = 0;
		tran_cmd.Data1 = 0xFF - tran_cmd.Data0;
		tran_cmd.Data2 = recv_cmd.Data0;
		tran_cmd.CheckSum = tran_cmd.Cmd + tran_cmd.Data0 + tran_cmd.Data1 + tran_cmd.Data2;
	}
	else{
		APP_MCU_PRINT("command fail...........%x %x %x %x %x \n\r", (uint32_t)recv_cmd.Cmd, (uint32_t)recv_cmd.CheckSum, (uint32_t)recv_cmd.Data0, (uint32_t)recv_cmd.Data1, (uint32_t)recv_cmd.Data2);
		return;
	}

	cnt = 0;
	while(!((inl((unsigned int*)(BSP_UART2_BASE_ADDRESS + UART_CONFIG))&TX_RDY) && (cnt > 2000))){
		cnt++;
		
		if(cnt > UART2_TIME_OUT){
			APP_MCU_PRINT("MCU set command timeout.....\n");
			return;
		}
	}
	
	for(i=0; i<5; i++){
		outl((unsigned int*)((BSP_UART2_BASE_ADDRESS + RS_DATA)), *(((char*)&tran_cmd) + i));
	}
}

/**
* @brief initial uart2 baud rate and fifo.
*/
void mcu_uart2_init(void)
{
	unsigned int tmp = 0;
	
	/* Attempt to register UART2's IRQ on VIC */
	pic_registerIrq(UART2_IRQ_NUM, &mcu_uart2_isr_handler, PRIORITY_IRQ_UART2);

	/* Enable the UART2's IRQ on VIC */
	pic_enableInterrupt(UART2_IRQ_NUM);

	//config UART2 clock/baudrate 115200/12=9600
	tmp = 0x58C;
	outl((unsigned int*)(BSP_UART2_BASE_ADDRESS + UART_CLOCK), tmp);
	
	//set FIFO threshold
	tmp = 0x0505;
	outl((unsigned int*)(BSP_UART2_BASE_ADDRESS + FIFO_THD), tmp);

	//config UART2 mode
	tmp = TX_MODE_UART|RX_MODE_UART|RS232_RX_INT_EN_BIT;

	//set UART2 config
	outl((unsigned int*)(BSP_UART2_BASE_ADDRESS + UART_CONFIG), tmp);
}

void mcu_uart2_reset(void)
{
	//set UART2 config
	outl((unsigned int*)(BSP_UART2_BASE_ADDRESS + UART_CONFIG), 0);

	mcu_uart2_init();
	
	APP_MCU_PRINT_QUEUE("uart2 reset finish.\n");
}

/**
* @brief parse mcu command and do it.
* @param pvParameters rtos task parameter.
*/
void mcu_recv_task(void *pvParameters)
{
	uint8_t i;
	uint8_t stats;
	uint8_t sys_para;
	uint8_t check_sum;
	uint8_t tran_flag;
	mcu_command_t recv_cmd;
	mcu_command_t tran_cmd;
	
	memset(&recv_cmd, 0, sizeof(mcu_command_t));
	memset(&tran_cmd, 0, sizeof(mcu_command_t));
	
	vTaskDelay(1000 / portTICK_RATE_MS);
	TaskRunFlag = 1;
	
	while(1){
		xQueueReceive(McuCmdQueue, &recv_cmd, portMAX_DELAY);

		//get lastest command 
		while(uxQueueMessagesWaiting(McuCmdQueue) != 0){
			xQueueReceive(McuCmdQueue, &recv_cmd, 0);
		}
		
		APP_MCU_DEBUG_PRINT("=======================================\n");
		
		tran_flag = (recv_cmd.Cmd & 0x80) >> 7;
		check_sum = recv_cmd.Cmd + recv_cmd.Data0 + recv_cmd.Data1 + recv_cmd.Data2;
		
		if(check_sum != recv_cmd.CheckSum){
			APP_MCU_PRINT_QUEUE("check sum error!!!!!!!  recv= 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x.\n", recv_cmd.Cmd, recv_cmd.CheckSum, recv_cmd.Data0, recv_cmd.Data1, recv_cmd.Data2);
			//mcu_uart2_reset();
		}
		else{
			APP_MCU_DEBUG_PRINT("Recv from Mcu = 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x.\n", recv_cmd.Cmd, recv_cmd.CheckSum, recv_cmd.Data0, recv_cmd.Data1, recv_cmd.Data2);
		
			switch(recv_cmd.Cmd & 0x7F){
				case CMD_SYNC:
					cmd_cnt_t.cnt_sync++;
					if(tran_flag){
						stats = SS_NORMAL;
						sys_para = 0;
						
						//check error flag & 98660 status & sensor status
						if(SysErrFlag != 0){
							if((SysErrFlag & (1 << ERR_SENSOR_ISP)) != 0){
								stats = SS_SENSOR_FAIL;
								sys_para |= (SP_660_SEN_ABNORMAL << SP_OFFSET_98660);
								sys_para |= (SP_SEN_STOP << SP_OFFSET_SENSOR);
							}
							else if((SysErrFlag & (1 << ERR_WIFI)) != 0){
								if(stats == SS_NORMAL){
									stats = SS_WIFI_FAIL;
								}
								else{
									stats = SS_SYS_ABNORMAL;
								}
								
								sys_para |= (SP_660_WIFI_ABNORMAL << SP_OFFSET_98660);
								WifiStatus = WIFI_OFF;
							}
							else{
								stats = SS_SYS_ABNORMAL;
							}

							if((SysErrFlag & (1 << FIRMWARE_UPGRADE)) != 0){
								stats = SS_FIRMWARE_UPGRADE;
							}
						}
						
						//check wifi status
						if(WifiStatus != WIFI_OFF){
							if(socket_get_cur_connect_num() == 0){
								sys_para |= (SP_WIFI_ON_NONLINK << SP_OFFSET_WIFI);
							}
							else{
								sys_para |= (SP_WIFI_ON_LINK << SP_OFFSET_WIFI);
							}
						}
						
						//check record status
						if(sd_card_remove){
							sys_para |= (SP_RECORD_NO_CARD << SP_OFFSET_RECORD);
						}
						else if(pb_state()){
							sys_para |= (SP_RECORD_IN_PLAYBACK << SP_OFFSET_RECORD);
						}
						else if(schedrec_state() == 0){
							sys_para |= (SP_RECORD_STOP << SP_OFFSET_RECORD);
						}
						
						tran_cmd.Cmd = recv_cmd.Cmd;
						tran_cmd.Data0 = stats;
						tran_cmd.Data1 = recv_cmd.Data0;
						tran_cmd.Data2 = sys_para;
						tran_cmd.CheckSum = tran_cmd.Cmd + tran_cmd.Data0 + tran_cmd.Data1 + tran_cmd.Data2;
					}
				
					break;
				case CMD_KEY:
					/*
					 * Because it is very strange protocol that using different element to store checksum value.
					 * The other strange point that has two checksum value in total 5 bytes data.
					 */
					//if((recv_cmd.Data0 != CMD_RESET_STATUS) && (recv_cmd.Data1 != (0xFF - recv_cmd.Data0))){
					//	tran_flag = 0;
					//	APP_MCU_PRINT_QUEUE("Data 0 check sum error!!!!!\n");
					//	break;
					//}
					
					stats = 0;
					//APP_MCU_DEBUG_PRINT("Recv command from Mcu = 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x.\n", recv_cmd.Cmd, recv_cmd.CheckSum, recv_cmd.Data0, recv_cmd.Data1, recv_cmd.Data2);

					switch(recv_cmd.Data0){
						case CMD_RECORD_START:
							cmd_cnt_t.cnt_recstart++;
							APP_MCU_PRINT_QUEUE("Do record start.\n");
							if((sd_card_remove) || (json_get_format_status() == 1)){
								stats = KS_RECORD_START_NO_CARD;
							}
							else if(pb_state()){
								stats = KS_RECORD_START_IN_PLAYBACK;
							}
							else{
								schedrec_suspend_restart(1);
								reclapse_suspend_restart(1);
								stats = KS_RECORD_START_OK;
							}
							break;
						case CMD_RECORD_STOP:
							cmd_cnt_t.cnt_recstop++;
							APP_MCU_PRINT_QUEUE("Do record stop.\n");
							if((sd_card_remove) || (json_get_format_status() == 1)){
								stats = KS_RECORD_STOP_NO_CARD;
							}
							else{
								user_diable_rec();
								user_disable_reclapse();
								stats = KS_RECORD_STOP_OK;
							}
							break;
						case CMD_SNAP_SHOT:
							cmd_cnt_t.cnt_snapshot++;
							APP_MCU_PRINT_QUEUE("Do Snapshot.\n");
							if((sd_card_remove) || (json_get_format_status() == 1)){
								stats = KS_SNAP_SHOT_NO_CARD;
							}
							else{
								if(pb_state()){
									APP_MCU_PRINT_QUEUE("Now in playback. Snapshot fail....\n");
								}
								else{
									set_takepic_num(1);
									mf_set_snapshot(1);
								}
								
								stats = KS_SNAP_SHOT_OK;
							}
							break;
						case CMD_FILE_LOCK:
							cmd_cnt_t.cnt_filelock++;
							APP_MCU_PRINT_QUEUE("Do file protection.\n");
							if((sd_card_remove) || (json_get_format_status() == 1)){
								stats = KS_FILE_LOCK_NO_CARD;
							}
							else{
								if(pb_state()){
									APP_MCU_PRINT_QUEUE("Now in playback. File protect fail....\n");
								}
								else{
									set_trigger(1);
								}
								
								stats = KS_FILE_LOCK_OK;
							}
							break;
						case CMD_WIFI_ON:
							cmd_cnt_t.cnt_wifion++;
							APP_MCU_PRINT_QUEUE("Do wifi on-------!!!!!!\n");
							if(WifiStatus != WIFI_ON){
								WifiStatus = WIFI_ON;
								//WiFi_Task_Init();		//wifi on call this function????
								/*init_rtsp_server();
								mf_video_set_preview_cb(send_preview_to_rtp);
								socket_init();
								socket_server_create(8080);*/
							}
							stats = KS_WIFI_ON_OK;
							break;
						case CMD_WIFI_OFF:
							cmd_cnt_t.cnt_wifioff++;
							APP_MCU_PRINT_QUEUE("Do wifi off-------!!!!!!\n");
							if(WifiStatus != WIFI_OFF){
								WifiStatus = WIFI_OFF;
								//wifi off call this function????
								/*mf_video_set_preview_cb(NULL);
								APP_MCU_PRINT("\n\r 11111\n\r");
								socket_uninit();
								APP_MCU_PRINT("\n\r 22222\n\r");
								pb_all_stop();
								APP_MCU_PRINT("\n\r 33333\n\r");
								destroy_rtsp_server();
								APP_MCU_PRINT("\n\r 44444\n\r");*/
								WiFi_Task_UnInit();
								vTaskDelay(1800 / portTICK_RATE_MS);
								print_msg("Uninit Wifi finish\n");
								//APP_MCU_PRINT("\n\r 55555\n\r");
							}
							stats = KS_WIFI_OFF_OK;
							break;
						case CMD_RESET_STATUS:
							APP_MCU_PRINT_QUEUE("Return reset status.\n");
							if(recv_cmd.Data2 != (0xFF - recv_cmd.Data0 - recv_cmd.Data1)){
								tran_flag = 0;
								APP_MCU_PRINT_QUEUE("CMD_RESET_STATUS Data 0+1 check sum error!!!!!\n");
								break;
							}
							
							if(recv_cmd.Data1 == KS_WIFI_RST_FINISH){
								mcu_clear_err_flag(ERR_WIFI);
								if(WifiStatus != WIFI_ON){
									WifiStatus = WIFI_ON;
									WiFi_Task_Init(NULL, WIFI_RUN_MODE_AP);		//wifi on call this function????
									/*init_rtsp_server();
									mf_video_set_preview_cb(send_preview_to_rtp);
									socket_init();
									socket_server_create(8080);*/
								}
							}
							else if(recv_cmd.Data1 == KS_SENSOR_RST_FINISH){
								mcu_clear_err_flag(ERR_SENSOR_ISP);
								if(snx_isp_init() != pdPASS){
									mcu_set_err_flag(ERR_SENSOR_ISP);
								}
							}

							stats = recv_cmd.Data1;
							break;

						case CMD_USB_DETECTION:
							if(recv_cmd.Data2 != (0xFF - recv_cmd.Data0 - recv_cmd.Data1)){
								tran_flag = 0;
								APP_MCU_PRINT_QUEUE("CMD_USB_DETECTION Data 0+1 check sum error!!!!!\n");
								break;
							}
							APP_MCU_PRINT_QUEUE("Do USB detect command-------!!!!!!\n");
							cmd_cnt_t.cnt_usbdevice++;
							if (recv_cmd.Data1 == 0x02) { // High
								APP_MCU_PRINT_QUEUE("USB DETECTION High\n");
								usbd_set_ext_hotplug_state(1);
								stats = 0x02;
							} else if (recv_cmd.Data1 == 0x04) { // Low
								APP_MCU_PRINT_QUEUE("USB DETECTION Low\n");
								usbd_set_ext_hotplug_state(0);
								stats = 0x04;
							}

							break;

						case CMD_EMERGENCY_LOCK:
							APP_MCU_PRINT_QUEUE("Do emergency lock.\n");
							if((sd_card_remove) || (json_get_format_status() == 1)){
								stats = 0x0;
							} else {
								if (pb_state()) {
									APP_MCU_PRINT_QUEUE("Now in playback. File protect fail....\n");
									stats = 0x0;
								} else {
									set_trigger(1);
									stats = 0x1;
								}
							}
							break;
						default:
							tran_flag = 0;
							APP_MCU_PRINT_QUEUE("No this key command!!!!!\n");
							break;
					}
					
					if(tran_flag){
						tran_cmd.Cmd = recv_cmd.Cmd;
						tran_cmd.Data0 = stats;
						tran_cmd.Data1 = 0xFF - tran_cmd.Data0;
						tran_cmd.Data2 = recv_cmd.Data0;
						tran_cmd.CheckSum = tran_cmd.Cmd + tran_cmd.Data0 + tran_cmd.Data1 + tran_cmd.Data2;
					}
					break;
				default:
					tran_flag = 0;
					APP_MCU_PRINT_QUEUE("No this command!!!!!\n");
					break;
			}
			
			if(tran_flag){
				vTaskDelay(2 / portTICK_RATE_MS);		//must delay to wait mcu
				while((inl((unsigned int*)(BSP_UART2_BASE_ADDRESS + UART_CONFIG))&TX_RDY) == 0);
				
				for(i=0; i<sizeof(mcu_command_t); i++){
					outl((unsigned int*)((BSP_UART2_BASE_ADDRESS + RS_DATA)), *(((char*)&tran_cmd) + i));
				}
				
				APP_MCU_DEBUG_PRINT("Tran to Mcu   = 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x.\n", tran_cmd.Cmd, tran_cmd.CheckSum, tran_cmd.Data0, tran_cmd.Data1, tran_cmd.Data2);
			}
		}
	}

	vTaskDelete(NULL);
}

#define DETECT_SECOND			2000	//2000ms
#define RESET_DEFAULT_DELAY		100		//100ms

/**
* @brief Check gpio2 and do reset to default.
* @param pvParameters rtos task parameter.
*/
void reset_to_defult_task(void *pvParameters)
{
	gpio_pin_info info;
	int push_cnt = 0;

	info.pinumber = 2;
	info.mode = 0;
	info.value = 0;

	for(;;){
		snx_gpio_read(&info);

		if(info.value == 0){
			push_cnt++;
		}
		else{
			push_cnt = 0;
		}

		if(push_cnt == (DETECT_SECOND / RESET_DEFAULT_DELAY)){
			push_cnt = 0;
			APP_MCU_PRINT("\n\rStarting reset to default.\n");
			
			all_task_uinit(TASK_KEEP_NO_KEEP);
			
			//reset parameter
			if(snx_nvram_reset_to_default(NVRAM_RTD_ALL, NULL, NULL) != NVRAM_SUCCESS){
				APP_MCU_PRINT_QUEUE("Reset to default fail!!!!!\n");
			}
			
			APP_MCU_PRINT("\n\rStarting system reset.\n");
			//reset system by watch dog
			/*
			wdt_disable();
			wdt_intr_enable(0);
			wdt_clr_flag();
			wdt_setload(0);
			wdt_reset_enable(1);
			wdt_enable();
			*/
			
			//reset system by mcu
			mcu_set_err_flag(ALL_RESET);

			while(1);
		}

		vTaskDelay(RESET_DEFAULT_DELAY / portTICK_RATE_MS);
	}
	
	vTaskDelete(NULL);
}

/**
* @brief initial mcu task and queue.
*/
void mcu_init(void)
{
	SysErrFlag = 0;
	WifiStatus = WIFI_ON;
	TaskRunFlag = 0;

	//memset(cmd_cnt_t, 0, sizeof(mcu_cmd_counter_t));

	mcu_uart2_init();
	
	McuCmdQueue = xQueueCreate(MCU_QUEUE_BUF, sizeof(mcu_command_t));
	if(McuCmdQueue == NULL){
		APP_MCU_PRINT_QUEUE("McuCmdQueue create fail\n");
		return;
	}

	if(pdPASS != xTaskCreate(mcu_recv_task, "mcu_recv", STACK_SIZE_1K, NULL, PRIORITY_TASK_APP_MCU_RECV, &McuRecvTask)){
		APP_MCU_PRINT_QUEUE("Could not create mcu connect task.\n");
	}

	if(pdPASS != xTaskCreate(reset_to_defult_task, "reset_default", STACK_SIZE_1K, (void*)NULL, PRIORITY_TASK_SYS_RESET_DEFAULT, NULL)){
		APP_MCU_PRINT_QUEUE("Could not create reset_default.\n");
	}
}

/**
* @brief uninitial mcu task and queue.
*/
void mcu_uninit(void)
{
	vTaskDelete(McuRecvTask);
	vQueueDelete(McuCmdQueue);
	vTaskDelete(reset_to_defult_task);	
}
