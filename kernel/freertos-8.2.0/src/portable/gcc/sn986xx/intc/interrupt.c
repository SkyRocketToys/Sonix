#include <FreeRTOS.h>
#include <bsp.h>
#include "interrupt.h"

intr_counter_t intr_cnt;
volatile unsigned int isInIRQ = 0;

#define UL1                    ( 0x00000001 )
#define UL0                    ( 0x00000000 )
#define ULFF                   ( 0xFFFFFFFF )
#define BM_IRQ_PART            ( 0x0000001F )
#define BM_VECT_ENABLE_BIT     ( 0x00000020 )

#define NR_VECTORS             ( 16 )
#define NR_INTERRUPTS          ( 32 )

/*
 * A table with IRQs serviced by each VICVECTADDRn.
 * If a table's field is negative, its corresponding VICVECTADDRn presumably
 * does not serve any IRQ. In this case, the corresponding VICVECTCNTLn is
 * supposed to be set to 0 and its VICVECTADDRn should be set to __irq_dummyISR.
 */
typedef struct _isrVectRecord
{
    int8_t irq;                   /* IRQ handled by this record */
    pVectoredIsrPrototype isr;    /* address of the ISR */
    int8_t priority;              /* priority of this IRQ */
} isrVectRecord;

static isrVectRecord __irqVect[NR_INTERRUPTS];

/**
 * Enable CPU's IRQ mode that handles IRQ interrupt requests.
 */
void irq_enableIrqMode(void)
{
    /*
     * To enable IRQ mode, bit 7 of the Program Status Register (CSPR)
     * must be cleared to 0. See pp. 2-15 to 2-17 of the DDI0222 for more details.
     * The CSPR can only be accessed using assembler.
     */

    __asm volatile("MRS r0, cpsr");        /* Read in the CPSR register. */
    __asm volatile("BIC r0, r0, #0x80");   /* Clear bit 8, (0x80) -- Causes IRQs to be enabled. */
    __asm volatile("MSR cpsr_c, r0");      /* Write it back to the CPSR register */
}


/**
 * Disable CPU's IRQ and FIQ mode that handle IRQ interrupt requests.
 */
void irq_disableIrqMode(void)
{
    /*
     * To disable IRQ mode, bit 7 of the Program Status Register (CSPR)
     * must be set t1 0. See pp. 2-15 to 2-17 of the DDI0222 for more details.
     * The CSPR can only be accessed using assembler.
     */

    __asm volatile("MRS r0, cpsr");       /* Read in the CPSR register. */
    __asm volatile("ORR r0, r0, #0xC0");  /* Disable IRQ and FIQ exceptions. */
    __asm volatile("MSR cpsr_c, r0");     /* Write it back to the CPSR register. */
}


/* a prototype required for __irq_dummyISR() */
//extern void uart_print(uint8_t nr, char* str);

/*
 * A dummy ISR routine for servicing vectored IRQs.
 *
 * It is supposed to be set as a default address of all vectored IRQ requests. If an "unconfigured"
 * IRQ is triggered, it is still better to be serviced by this dummy function instead of
 * being directed to an arbitrary address with possibly dangerous effects.
 */
static void __irq_dummyISR(void)
{
    /*
     * An "empty" function.
     * As this is a test application, it emits a warning to the UART0.
     */
     //uart_print(0, "<WARNING, A DUMMY ISR ROUTINE!!!>\r\n");
}

void pic_clearIrq(int irq)
{
	outl(IRQ_CLR, (0x1 << irq));
}

/*
 * IRQ handler routine, called directly from the IRQ vector, implemented in exception.c
 * Prototype of this function is not public and should not be exposed in a .h file. Instead,
 * its prototype must be declared as 'extern' where required (typically in exception.c only).
 *
 * NOTE:
 * There is no check that provided addresses are correct! It is up to developers
 * that valid ISR addresses are assigned before the IRQ mode is enabled!
 *
 * It supports two modes of IRQ handling, vectored and nonvectored mode. They are implemented
 * for testing purposes only, in a real world application, only one mode should be selected
 * and implemented.
 */
void _pic_IrqHandler(void)
{
	int irq_n = 0 ;
	pVectoredIsrPrototype isrAddr;
	volatile unsigned int irqflag = 0;



	irqflag = inl(IRQ_FLAG);
	while ((irqflag = inl(IRQ_FLAG)) == 0);

	for (irq_n = 0 ; irq_n < NR_INTERRUPTS ; irq_n++) {
        	//if (inl(IRQ_FLAG) & (0x1 << irq_n) && (__irqVect[irq_n].irq == irq_n))
		if (irqflag & (0x1 << irq_n) && (__irqVect[irq_n].irq == irq_n))
			break;
	}

        /*
         * Reads the Vector Address Register with the ISR address of the currently active interrupt.
         * Reading this register also indicates to the priority hardware that the interrupt
         * is being serviced.
         */
        isrAddr = __irqVect[irq_n].isr;

#if (configUSE_TRACE_FACILITY == 1)
        vTraceStoreISRBegin(irq_n);
#endif
    	isInIRQ = 1;
        /* Execute the routine at the vector address */
        (*isrAddr)(irq_n);
        isInIRQ = 0;
#if (configUSE_TRACE_FACILITY == 1)
        vTraceStoreISREnd(0);
#endif
        intr_cnt.cnt[irq_n]++;

        // Clear Irq signal
        pic_clearIrq(irq_n);
}


/**
 * Initializes the primary interrupt controller to default settings.
 *
 * All interrupt request lines are set to generate IRQ interrupts and all
 * interrupt request lines are disabled by default. Additionally, all vector
 * and other registers are cleared.
 */
void pic_init(void)
{
	uint8_t i;

	isInIRQ = 1;

	//Initialize the IRQ interrupt controller
	outl(IRQ_EN,0x0);			// disable all IRQs
	outl(IRQ_CLR, 0xffffffff);	// clear all IRQs
	outl(IRQ_MODE,0x0);		// level trigger
	outl(IRQ_TRG,0x0);			// high level

	//Initialize the FIQ interrupt controller
	outl(FIQ_EN,0x0);			// disable all FIQs
	outl(FIQ_CLR, 0xffffffff);	// clear all FIQs
	outl(FIQ_MODE,0x0);		// level trigger
	outl(FIQ_TRG,0x0);			// high level

	for (i = 0 ; i < NR_INTERRUPTS ; ++i ) {
		/* clear its entry in the table */
		__irqVect[i].irq = -1;                 /* no IRQ assigned */
		__irqVect[i].isr = (pVectoredIsrPrototype)&__irq_dummyISR;    /* dummy ISR routine */
		__irqVect[i].priority = -1;            /* lowest priority */

		// Init interrupt counter
		intr_cnt.cnt[i] = 0;
	}
}


/**
 * Enable the the interrupt request line on the PIC for the specified interrupt number.
 *
 * Nothing is done if 'irq' is invalid, i.e. equal or greater than 32.
 *
 * @param irq - interrupt number (must be smaller than 32)
 */
void pic_enableInterrupt(uint8_t irq)
{
    /* TODO check for valid (unreserved) interrupt numbers? Applies also for other functions */
	unsigned int en = 0;

	en = inl(IRQ_EN);
	en |= (0x1 << irq);
	outl(IRQ_EN, en);
}


/**
 * Disable the the interrupt request line on the PIC for the specified interrupt number.
 *
 * Nothing is done if 'irq' is invalid, i.e. equal or greater than 32.
 *
 * @param irq - interrupt number (must be smaller than 32)
 */
void pic_disableInterrupt(uint8_t irq)
{
	volatile unsigned value;

	value = inl(IRQ_EN);
	value &= ~(0x1 << irq);
	outl(IRQ_EN, value);	// dsiable IRQ
}

/**
 * Checks whether the interrupt request line for the requested interrupt is enabled.
 *
 * 0 is returned if 'irq' is invalid, i.e. equal or greater than 32.
 *
 * @param irq - interrupt number (must be smaller than 32)
 *
 * @return 0 if disabled, a nonzero value (typically 1) if the interrupt request line is enabled
 */
int8_t pic_isInterruptEnabled(uint8_t irq)
{
    /* See description of VICINTENCLEAR, page 3-7 of DDI0181: */

    //return ( irq<NR_INTERRUPTS && (pPicReg->VICINTENABLE & (UL1<<irq)) ? 1 : 0 );
	return 0;
}

#if (configUSE_TRACE_FACILITY == 1)
char *irq_str[32] = {"0_Gpio", "1_I2c1", "2_I2c2", "3_Rtc_alarm", "4_Rtc_wake", "5_Timer", "6_Wdt", "7_SSP1",
		"8_Uart1", "9_Usb_dev", "10_Uart2", "11", "12_AHBC", "13_APB_DMA", "14_DMAC", "15_MS1",
		"16_H264", "17_MAC", "18_Aud_Ply", "19_Aud_Rec", "20", "21_Vsync", "22_HW_End", "23_HSync",
		"24_USB_Host", "25_AES", "26_MS2", "27_SD_Det", "28_Jpeg", "29", "30_MIPI", "31",
};
#endif
/**
 * Registers a vector interrupt ISR for the requested interrupt request line.
 * The vectored interrupt is enabled by default.
 *
 * Nothing is done and -1 is returned if either 'irq' is invalid (must be less than 32)
 * or ISR's address is NULL.
 *
 * Entries are internally sorted in descending order by priority.
 * Entries with the same priority are additionally sorted by the time of registration
 * (entries registered earlier are ranked higher).
 * If 'irq' has already been registered, its internal entry will be overridden with
 * new values and resorted by priority.
 * The first 16 entries, sorted by priority, are automatically entered into appropriate vector
 * registers of the primary interrupt controller.
 *
 * @note IRQ handling should be completely disabled prior to calling this function!
 *
 * @param irq - interrupt number (must be smaller than 32)
 * @param addr - address of the ISR that services the interrupt 'irq'
 * @param priority - priority of handling this IRQ (higher value means higher priority), the actual priority
 *                   will be silently truncated to 127 if this value is exceeded.
 *
 * @return position of the IRQ handling entry within an internal table, a negative value if registration was unsuccessful
 */
int8_t pic_registerIrq(
                        uint8_t irq,
                        pVectoredIsrPrototype addr,
                        uint8_t priority )
{
	__irqVect[irq].irq = irq;
	__irqVect[irq].isr = addr;
	__irqVect[irq].priority = priority & PIC_MAX_PRIORITY;

#if (configUSE_TRACE_FACILITY == 1)
	vTraceSetISRProperties(irq, irq_str[irq], 0);
#endif
	return irq;
}

/**
 * Unregisters a vector interrupt ISR for the requested interrupt request line.
 *
 * Nothing is done if 'irq' is invalid, i.e. equal or greater than 32 or
 * no vector for the 'irq' exists.
 *
 * @note IRQ handling should be completely disabled prior to calling this function!
 *
 * @param irq - interrupt number (must be smaller than 32)
 */
void pic_unregisterIrq(uint8_t irq)
{
	/* clear its entry in the table */
	__irqVect[irq].irq = -1;	/* no IRQ assigned */
	__irqVect[irq].isr = (pVectoredIsrPrototype)&__irq_dummyISR;    /* dummy ISR routine */
	__irqVect[irq].priority = -1;	/* lowest priority */

	// Re-Init interrupt counter
	intr_cnt.cnt[irq] = 0;
}

/**
 * Unregisters all vector interrupts.
 */
void pic_unregisterAllIrqs(void)
{

}

unsigned int inIRQ()
{
	return isInIRQ;
}

void ClearinIRQ()
{
	isInIRQ = 0;
}
