#ifndef _BSP_H_
#define _BSP_H_


/*
 * Base address and IRQs of both timer controllers
 * (see pp. 4-21  and 4-67 of DUI0225D):
 */

#define BSP_NR_TIMERS       ( 2 )

#define BSP_TIMER_IRQS      { ( 4 ), ( 5 ) }

/*
 * IRQ, reserved for software generated interrupts.
 * See pp.4-46 to 4-48 of the DUI0225D.
 */

#define BSP_SOFTWARE_IRQ            ( 1 )

#define outl(addr, value)       (*((volatile unsigned int *)(addr)) = value)
#define inl(addr)                (*((volatile unsigned int *)(addr)))


#include <register.h>

#define PRIORITY_IRQ_GPIO			0
#define PRIORITY_IRQ_I2C1			0
#define PRIORITY_IRQ_I2C2			0
#define PRIORITY_IRQ_RTC_ALARM		0
#define PRIORITY_IRQ_RTC_WAKEUP		0
#define PRIORITY_IRQ_TIMER			0
#define PRIORITY_IRQ_WATCHDOG		0
#define PRIORITY_IRQ_SSP1			0
#define PRIORITY_IRQ_UART1			0
#define PRIORITY_IRQ_USB_DEVICE		0
#define PRIORITY_IRQ_UART2			0
#define PRIORITY_IRQ_AHBC			0
#define PRIORITY_IRQ_APBC			0
#define PRIORITY_IRQ_DMAC			0
#define PRIORITY_IRQ_MS1			0
#define PRIORITY_IRQ_H264			0
#define PRIORITY_IRQ_MAC			0
#define PRIORITY_IRQ_AUDIO_PLAYBACK	0
#define PRIORITY_IRQ_AUDIO_RECORD	0
#define PRIORITY_IRQ_SEN_VSYNC		0
#define PRIORITY_IRQ_HW_END			0
#define PRIORITY_IRQ_SEN_HSYNC		0
#define PRIORITY_IRQ_USB_HOST		0
#define PRIORITY_IRQ_AES			0
#define PRIORITY_IRQ_MS2			0
#define PRIORITY_IRQ_SD_CARD_DETECT	0
#define PRIORITY_IRQ_JPEG_DECODER	0
#define PRIORITY_IRQ_MIPI			0

#include <priority.h>

/*
 * Stack size define
 */
#define STACK_SIZE_512			128
#define STACK_SIZE_768			192
#define STACK_SIZE_1K			256
#define STACK_SIZE_2K			512
#define STACK_SIZE_4K			1024
#define STACK_SIZE_5K			1280
#define STACK_SIZE_6K			1536
#define STACK_SIZE_7K			1792
#define STACK_SIZE_8K			2048
#define STACK_SIZE_16K			4096
#define STACK_SIZE_32K			8192
#define STACK_SIZE_64K			16384


#define GLOBAL_SETTING_BASE_ARRD	0x98000000
#define GLOBAL_CTRL					(GLOBAL_SETTING_BASE_ARRD + 0x0)
#define GLOBAL_RATE					(GLOBAL_SETTING_BASE_ARRD + 0x4)
#define GLOBAL_GATING				(GLOBAL_SETTING_BASE_ARRD + 0x8)
#define GLOBAL_FPGA_VER				(GLOBAL_SETTING_BASE_ARRD + 0xc)
#define GLOBAL_ASIC_ID				(GLOBAL_SETTING_BASE_ARRD + 0x10)

typedef struct pll_clock {
	uint32_t pll800_clk;
	uint32_t pll800_div;
	uint32_t pll300_clk;
	uint32_t pll300_div;
}pll_clock_t;

typedef struct global_rate {
	uint32_t ddr_rate:4;
	uint32_t cpu_rate:4;
	uint32_t ahb_rate:4;
	uint32_t apb_rate:3;
	uint32_t ahb2_rate:5;
	uint32_t isp_rate:4;
	uint32_t lcd_rate:4;
	uint32_t jpgdec_rate:4;
}global_setting_t;

typedef struct system_bus_freq {
	uint32_t fddr;
	uint32_t fcpu;
	uint32_t fahb;
    union {
            uint32_t fapb;
            uint32_t freq_i2c1;
            uint32_t freq_i2c2;
            uint32_t freq_uart1;
            uint32_t freq_uart2;
    };
	uint32_t fahb2;
	uint32_t fisp;
}system_bus_freq_t;

void init_freq_table(void);
system_bus_freq_t *clk_get(void);
void cpu_udelay(int us);

typedef void (*pVectoredIsrPrototype)(int irq);
void pic_enableInterrupt(uint8_t irq);
int8_t pic_registerIrq(uint8_t irq, pVectoredIsrPrototype addr, uint8_t priority );

#endif   /* _BSP_H_ */
