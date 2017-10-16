/**
 * @file
 *
 * Declaration of public functions that handle
 * the board's UART controllers.
 *
 * @author Jernej Kovacic
 */

#ifndef _UART_H_
#define _UART_H_

#define	UART_CONFIG				0x0
#define	TX_MODE_UART			1		//BIT0
#define	TX_STOP_BIT_2			1<<1	//BIT1
#define	TX_RDY					1<<2	//BIT2
#define	RX_MODE_UART			1<<4	//BIT4
#define	RX_RDY					1<<5	//BIT5
#define	RS485_EN				1<<7	//BIT7
#define	RS232_DMA_TX_EN			1<<8	//BIT8
#define	RS232_DMA_RX_EN			1<<9	//BIT9
#define	DMA_TX_BURST_MODE		1<<10	//BIT10
#define	DMA_RX_BURST_MODE		1<<11	//BIT11
#define	RS232_TX_INT_EN_BIT		1<<16
#define	RS232_RX_INT_EN_BIT		1<<17
#define	RS232_TX_INT_CLR_BIT	1<<18
#define	RS232_RX_INT_CLR_BIT	1<<19

#define	UART_CLOCK				0xc
#define	FIFO_THD				0x18
//bit definition
#define	RS_RATE					0x0
#define	RS_RATE_MASK			0x7f
#define	TRX_BASE				0x7
#define	TRX_BASE_MASK			0xf80

#define	RS_DATA					0x10
#define	RS_DATA_MASK			0xff

typedef struct uart_status_counter {
	unsigned int tx_byte_cnt;
	unsigned int rx_byte_cnt;
}uart_status_counter_t;

//void uart_init(uint8_t nr);
void uart_init(void);
void uart2_init(unsigned int baudrate);

//void uart_printChar(uint8_t nr, char ch);
void uart_print_char(char ch);

//void uart_print(uint8_t nr, const char* str);
void uart_print(const char* str);

void uart_enable_rx_interrupt();
void uart2_enable_rx_interrupt();

void uart_clear_rx_interrupt();
void uart2_clear_rx_interrupt(void);
void uart2_register_irq(int irq, void *isr_handler);
void uart2_unregister_irq(int irq);

void uart2_disable_rx_interrupt(void);

void uart2_write(const unsigned char *data, unsigned len);

char uart_read_char();

void uart_isr_handler(int irq);

uart_status_counter_t *get_uart_status_cnt(void);

void uart2_set_baudrate(unsigned int reg);
unsigned int uart2_get_baudrate(void);

unsigned int uart2_get_fifo_threshold_reg(void);
void uart2_set_fifo_threshold_reg(unsigned int reg);

#endif  /* _UART_H_ */
