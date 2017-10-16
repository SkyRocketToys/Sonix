
#ifndef __MF_UDP_CMD_H__ 
#define __MF_UDP_CMD_H__ 


#define	UDP_CMD_QUEUE_BUF				2
#define UDP_CMD_PORT					30864
//#define SUPPORT_UART2_RX_CMD 
//#define EVB_LED_GPIO
#define SUPPORT_LED_GPIO


typedef struct _udp_cmd_data{
	uint8_t Data0;
	uint8_t Data1;
	uint8_t Data2;
	uint8_t Data3;
	uint8_t Data4;
	uint8_t Data5;
	uint8_t Data6;
	uint8_t Data7;
	uint8_t Data8;
	uint8_t Data9;
	uint8_t Data10;
	uint8_t Data11;	
	uint8_t Data12;
}udp_cmd_data_t;

typedef struct _udp_info{
	uint8_t type;//0:13byte, 1:11byte, 2:8byte 
	uint8_t size;//13byte, 11byte, 8byte
	uint8_t uart2_write;
}udp_info_t;

//void udp_recv_task_process(void *pvParameters);
//void udp_uart2_task_process(void *pvParameters);
void udp_cmd_socket_uninit(void);
int udp_cmd_socket_create(void);
void led_gpio_ctrl(int num, int mode, int val);

#endif

