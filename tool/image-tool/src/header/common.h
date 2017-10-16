#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdarg.h>

#include "io.h"
#include "ctype.h"
#include "errno.h"
#include "types.h"
#include "string.h"
#include "stddef.h"

#define NONE 0

#define min(a, b) (((a) < (b)) ? (a) : (b))

#define BIT(x)	(1<<(x))


/**
 * UART
 */
#define SN926_UART_BASE 0x98a00000

typedef struct 
{
	volatile u32	CONFIG;
	volatile u32	DMA_TX_SIZE;
	volatile u32	DMA_RX_SIZE;
	volatile u32	CLOCK;
	volatile u32	RSDATA;
	volatile u32	GPIO;
	volatile u32 	FIFO;
} SN926_UART_NEW;

int serial_init (void);
void serial_puts(const char *s);

int ms_serial_flash_init(void);
int ms_serial_flash_update(unsigned int addr, unsigned int size);

extern u32 _TEXT_BASE;
extern u32 _bss_start;
extern u32 _bss_end;


extern u32 AHB2_CLOCK;

extern u32 IMAGE_TABLE_START;
extern u32 IMAGE_TABLE_SIZE;


int vsprintf(char *buf, const char *fmt, va_list args);
int serial_printf(const char *fmt, ...);


#endif
