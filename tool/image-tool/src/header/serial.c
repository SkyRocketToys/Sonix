#include "common.h"
#include "generated/snx_sdk_conf.h"

//#define CONFIG_HZ       (25000000 /2)		//FPGA

#define CONFIG_HZ       (25000000)		//ASIC
#define CONFIG_BAUDRATE	115200

#define UART_CONFIG		0x0
#define TX_MODE_UART	0x0
#define	TX_STOP_BIT_2	0x1
#define	TX_RDY			0x2
#define	RX_MODE_UART	0x4
#define	RX_RDY			0x5
#define	UART_CLOCK		0xc
#define	RS_RATE			0x0
#define	RS_RATE_MASK	0x7f
#define	TRX_BASE		0x7
#define	TRX_BASE_MASK	0xf80
#define	DATA			0x10
#define	DATA_MASK		0xff


static SN926_UART_NEW *sn926_uart = 
	(SN926_UART_NEW *) SN926_UART_BASE;

int serial_init (void)
{
	#if 0
	unsigned int val;

	val = BIT(TX_MODE_UART)|BIT(RX_MODE_UART);
	writel(val, &sn926_uart->CONFIG);

	val = (115200/CONFIG_BAUDRATE)&RS_RATE_MASK;
	val |= ((CONFIG_HZ/8/115200)<<TRX_BASE)&TRX_BASE_MASK;
	writel(val, &sn926_uart->CLOCK);

	val = readl(&sn926_uart->FIFO);
	val &= ~((0x1F<<0)|(0x1F<<8));
	val |= ((32<<0)|(32<<8));
	writel(val, &sn926_uart->FIFO);
#endif
	return 0;
}

#ifdef CONFIG_ENABLE_BURNBIN_LOG
static void serial_putc(const char c)
{
	unsigned int val;

	if (c == '\n')
		serial_putc ('\r');

	do{
		val = readl(&sn926_uart->FIFO);
		val >>= 16;
		val &= 0x3f;
	}while (val == 32);
	
	writel((unsigned char) c, &sn926_uart->RSDATA);
}
#else
static void serial_putc(const char c)
{
}
#endif


void serial_puts(const char *s)
{
	while(*s) {
		serial_putc (*s++);
	}
}

int serial_printf(const char *fmt, ...)
{
    int i;
    va_list args;
    char printbuffer[256];
   
    va_start(args, fmt);
    i = vsprintf(printbuffer, fmt, args);
    va_end(args);
        
    serial_puts(printbuffer);

    return i;
}
