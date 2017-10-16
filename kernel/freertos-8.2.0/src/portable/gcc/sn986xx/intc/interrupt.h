#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_
#include <bsp.h>

#define PIC_MAX_PRIORITY     ( 127 )

/**
 * Required prototype for vectored ISR servicing routines
 */
void irq_enableIrqMode(void);
void irq_disableIrqMode(void);
void pic_init(void);
void pic_disableInterrupt(uint8_t irq);

void pic_unregisterIrq(uint8_t irq);
void pic_unregisterAllIrqs(void);
void pic_clearIrq(int irq);
unsigned int inIRQ(void);
void ClearinIRQ(void);

/* Interrupt Controller */
#define IRQ_SCR		((volatile unsigned *)(BSP_INTC_BASE_ADDRESS + 0x00))       // IRQ Source Register (R)
#define IRQ_EN		((volatile unsigned *)(BSP_INTC_BASE_ADDRESS + 0x04))       // IRQ Enable Register
#define IRQ_CLR		((volatile unsigned *)(BSP_INTC_BASE_ADDRESS + 0x08))       // IRQ Interrupt Clear Register (W)
#define IRQ_MODE	((volatile unsigned *)(BSP_INTC_BASE_ADDRESS + 0x0c))       // IRQ Trig Mode Register
#define IRQ_TRG		((volatile unsigned *)(BSP_INTC_BASE_ADDRESS + 0x10))       // IRQ Trig Level Register
#define IRQ_FLAG	((volatile unsigned *)(BSP_INTC_BASE_ADDRESS + 0x14))       // IRQ Status Register (R)
#define FIQ_SCR		((volatile unsigned *)(BSP_INTC_BASE_ADDRESS + 0x20))       // FIQ Source Register (R)
#define FIQ_EN		((volatile unsigned *)(BSP_INTC_BASE_ADDRESS + 0x24))       // FIQ Enable Register
#define FIQ_CLR		((volatile unsigned *)(BSP_INTC_BASE_ADDRESS + 0x28))       // FIQ Interrupt Clear Register (W)
#define FIQ_MODE	((volatile unsigned *)(BSP_INTC_BASE_ADDRESS + 0x2c))       // FIQ Trig Mode Register
#define FIQ_TRG		((volatile unsigned *)(BSP_INTC_BASE_ADDRESS + 0x30))       // FIQ Trig Level Register
#define FIQ_FLAG	((volatile unsigned *)(BSP_INTC_BASE_ADDRESS + 0x34))       // FIQ Status Register (R)

#define INT_REV		((volatile unsigned *)(BSP_INTC_BASE_ADDRESS + 0x50))       // INT Revision Register (R)
#define INT_NUM		((volatile unsigned *)(BSP_INTC_BASE_ADDRESS + 0x54))       // Feature Register for Input Number (R)
#define IRQ_DEBOUNCE	((volatile unsigned *)(BSP_INTC_BASE_ADDRESS + 0x58))       // Feature Register for IRQ De-bounce Location (R)
#define FIQ_DEBOUNCE	((volatile unsigned *)(BSP_INTC_BASE_ADDRESS + 0x5c))       // Feature Register for FIQ De-bounce Location (R)

typedef struct intr_counter {
	unsigned long long cnt[32];
}intr_counter_t;

#endif  /* _INTERRUPT_H_ */
