#include <stdint.h>
#include <stddef.h>

#include <bsp.h>

#define CTL_ENABLE          ( 0x00000080 )
#define CTL_MODE            ( 0x00000040 )
#define CTL_INTR            ( 0x00000020 )
#define CTL_PRESCALE_1      ( 0x00000008 )
#define CTL_PRESCALE_2      ( 0x00000004 )
#define CTL_CTRLEN          ( 0x00000002 )
#define CTL_ONESHOT         ( 0x00000001 )

#define TM1_COUNTER			0x00
#define TM1_LOAD			0x04
#define TM1_MATCH1			0x08
#define TM1_MATCH2			0x0c
#define TM2_COUNTER			0x10
#define TM2_LOAD			0x14
#define TM2_MATCH1			0x18
#define TM2_MATCH2			0x1c
#define TM3_COUNTER			0x20
#define TM3_LOAD			0x24
#define TM3_MATCH1			0x28
#define TM3_MATCH2			0x2c
#define	TMCR				0x30
#define INTR_STATE			0x34

typedef struct
{
	uint32_t Tm1En:1;		// Timer1 enable bit
	uint32_t Tm1Clock:1;		// Timer1 clock source  (0: PCLK, 1: EXT1CLK)
	uint32_t Tm1OfEn:1;		// Timer1 over flow interrupt enable bit
	uint32_t Tm2En:1;
	uint32_t Tm2Clock:1;
	uint32_t Tm2OfEn:1;
	uint32_t Tm3En:1;
	uint32_t Tm3Clock:1;
	uint32_t Tm3OfEn:1;
	uint32_t Tm1UpDown:1;
	uint32_t Tm2UpDown:1;
	uint32_t Tm3UpDown:1;
//hiyan temp remove UINT32 CountDownEn:3;
	uint32_t Reserved:23;
}TimerControl;

typedef struct
{
	uint32_t TimerValue;
	uint32_t TimerLoad;
	uint32_t TimerMatch1;
	uint32_t TimerMatch2;
}TimerReg;

/**
 * Initializes the specified timer's counter controller.
 */
void timer_init_prepare(void)
{
	volatile TimerControl *ctrl = (TimerControl *)(BSP_TIMER_BASE_ADDRESS + TMCR);
	volatile TimerReg *reg = (TimerReg *)(BSP_TIMER_BASE_ADDRESS + TM1_COUNTER);
	volatile TimerReg *reg3 = (TimerReg *)(BSP_TIMER_BASE_ADDRESS + TM3_COUNTER);

	ctrl->Tm1En = 0;

	reg->TimerValue = 0x80000000;
	reg->TimerLoad = 0x0;
	//reg->TimerValue = 0x16387a0;
	//reg->TimerLoad = 0x16387a0;
	//ctrl->Tm1En = 1;	// enable timer 1
	// Enable fixed timer clock at 10MHZ
	ctrl->Tm1Clock = 1;	// timer clock source: 10Mhz
	ctrl->Tm1OfEn = 0;	// over flow interrupt disable

	ctrl->Tm2En = 0;	// disable timer 2
	ctrl->Tm3En = 0;	// disable timer 3

	// Timer3
	outl(0x98600038, 0x1c0);
	ctrl->Tm3Clock = 1;
	ctrl->Tm3OfEn = 0;
	reg3->TimerValue= 0x80000000 - 0x1;
	reg3->TimerLoad = 0x80000000 - 0x1;
	ctrl->Tm3En = 1;


	//Set_Timer_AutoReloadValue (1, TIMER_LOAD_VAL);
}

/**
 * Starts the specified timer's counter.
 */
void timer_start_prepare(void)
{
	volatile TimerControl *ctrl = (TimerControl *)(BSP_TIMER_BASE_ADDRESS + TMCR);

	ctrl->Tm1En = 1;
}

/**
 * Enables the timer's interrupt triggering (when the counter reaches 0).
 */
void timer_enable_intr_prepare(void)
{
	volatile TimerControl *ctrl = (TimerControl *)(BSP_TIMER_BASE_ADDRESS + TMCR);

	ctrl->Tm1OfEn = 1;	// enable timer 1
}

/**
 * Clears the interrupt output from the specified timer.
 */
void timer_clear_intr_prepare(void)
{
	//volatile TimerReg *reg = (TimerReg *)(0x98600000);

	outl((BSP_TIMER_BASE_ADDRESS + INTR_STATE), 0xe00);
}


/**
 * Sets the value of the specified counter's Load Register.
 *
 * When the timer runs in periodic mode and its counter reaches 0,
 * the counter is reloaded to this value.
 */
void timer_set_load_prepare(uint32_t counter)
{
	volatile TimerReg *reg = (TimerReg *)(BSP_TIMER_BASE_ADDRESS + TM1_COUNTER);

	reg->TimerValue = counter;
	reg->TimerLoad = counter;
}
