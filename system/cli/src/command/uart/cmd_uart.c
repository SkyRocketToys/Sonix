#include <stdint.h>
#include <bsp.h>
#include <uart/uart.h>
#include <nonstdlib.h>
#include <string.h>
#include "cmd_uart.h"

#define DEBUG		1
#define printf(fmt, args...) if(DEBUG) print_msg_queue((fmt), ##args)

int cmd_uart_status(int argc, char* argv[])
{
	uart_status_counter_t *status;

	status =  get_uart_status_cnt();

	print_msg_queue("TX bytes = %d\n", status->tx_byte_cnt);
	print_msg_queue("RX bytes = %d\n", status->rx_byte_cnt);

	return 0;
}

typedef struct _verify_info {
	char msg[32];
	unsigned szmsg;
} uart2_verify_info;

static uart2_verify_info definfo = {{0}};

void uart2_verify_isr_handler(int irq)
{
	volatile unsigned val;
	char recv_msg[32] = {0};
	int i;

	cpu_udelay(3000);
	val = inl((unsigned*)(BSP_UART2_BASE_ADDRESS + FIFO_THD));
	val = (val >> 24) & 0x3F;
	if (val > 32) {
		for (i = 0 ; i < 32 ; i++) {
			val = inl((unsigned*)(BSP_UART2_BASE_ADDRESS + RS_DATA));
		}
		print_msg("UART2 FIFO FULL!!!\n\r");
		return;
	}

	if (definfo.szmsg != val) {
		print_msg("UART2 data size of Tx(%u), Rx(%u)\n", definfo.szmsg, val);
	}

	while(inl((unsigned*)(BSP_UART2_BASE_ADDRESS + UART_CONFIG))&RX_RDY){
		for(i=0; i < val; i++){
			*(((char*)&recv_msg) + i) = (char)inl((unsigned*)(BSP_UART2_BASE_ADDRESS + RS_DATA));
		}
	}

	if (strncmp(definfo.msg, recv_msg, val) == 0) {
		print_msg("UART2 Data Compare Success, msg [%s]\n", recv_msg);
	} else {
		print_msg("UART2 Data Compare Fail, msg [%s]\n", recv_msg);
	}

	uart2_clear_rx_interrupt();
	uart2_disable_rx_interrupt();
	uart2_unregister_irq(UART2_IRQ_NUM);

	return;
}

int cmd_uart2_verify(int argc, char* argv[])
{
	int i;

	if (argc < 2) {
		print_msg("Usage: uart2_verify [msg]\n");
		print_msg("msg:\t\tMessage to transmit, l.e. 32 bytes.\n");
		return -1;
	}

	uart2_init(115200);
	uart2_register_irq(UART2_IRQ_NUM, &uart2_verify_isr_handler);
	outl((unsigned int*)(BSP_UART2_BASE_ADDRESS + FIFO_THD), 0x0101L);

	definfo.szmsg = strlen(argv[1]);
	memset(definfo.msg, 0, sizeof(char) * 32);
	strncpy(definfo.msg, argv[1], definfo.szmsg);

	//print_msg("%s info msg:%s, msg size:%u\n", __func__, definfo.msg, definfo.szmsg);
	// transmit
	while ((inl((unsigned*)(BSP_UART2_BASE_ADDRESS + UART_CONFIG))&TX_RDY) == 0);

	for (i = 0; i < definfo.szmsg; ++i) {
		outl((unsigned*)((BSP_UART2_BASE_ADDRESS + RS_DATA)),
				*(((char*)&definfo.msg) + i));
	}

	return 0;
}

