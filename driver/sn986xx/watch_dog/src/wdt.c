/*
 * wdt.c
 *
 *  Created on: Apr 15, 2015
 *      Author: spark_lee
 */


#include "FreeRTOS.h"

#include <bsp.h>
#include <wdt.h>


uint32_t wdt_getcount()
{
	uint32_t val ;
	val = inl((BSP_WDT_BASE_ADDRESS+WDOG_COUNTER_REG));
	return val;
}

void wdt_restart()
{
	outl((BSP_WDT_BASE_ADDRESS+WDOG_RESTART_REG), WDT_RESTART_CODE);
}

uint32_t wdt_getload()
{
	uint32_t val ;
	val = inl((BSP_WDT_BASE_ADDRESS+WDOG_LOAD_REG));
	return val;
}

void wdt_setload(uint32_t count)
{
	outl((BSP_WDT_BASE_ADDRESS+WDOG_LOAD_REG), count);
	wdt_restart();
}



void wdt_enable()
{
	WDOG_EN_REG_t reg;

	reg.r = inl((BSP_WDT_BASE_ADDRESS+WDOG_EN_REG));
	reg.b.WDOG_EN = WDT_ENABLE;

	outl((BSP_WDT_BASE_ADDRESS+WDOG_EN_REG), reg.r);
}

void wdt_disable()
{
	WDOG_EN_REG_t reg;

	reg.r = inl((BSP_WDT_BASE_ADDRESS+WDOG_EN_REG));
	reg.b.WDOG_EN = WDT_DISABLE;

	outl((BSP_WDT_BASE_ADDRESS+WDOG_EN_REG), reg.r);
}

void wdt_reset_enable(uint32_t en)
{
	WDOG_EN_REG_t reg;

	reg.r = inl((BSP_WDT_BASE_ADDRESS+WDOG_EN_REG));
	reg.b.WDOG_RST_EN = en;

	outl((BSP_WDT_BASE_ADDRESS+WDOG_EN_REG), reg.r);
}


void wdt_intr_enable(uint32_t en)
{
	WDOG_EN_REG_t reg;

	reg.r = inl((BSP_WDT_BASE_ADDRESS+WDOG_EN_REG));
	reg.b.WDOG_INTR_EN = en;

	outl((BSP_WDT_BASE_ADDRESS+WDOG_EN_REG), reg.r);
}

void wdt_ext_enable(uint32_t en)
{
	WDOG_EN_REG_t reg;

	reg.r = inl((BSP_WDT_BASE_ADDRESS+WDOG_EN_REG));
	reg.b.WDOG_EXT_EN = en;

	outl((BSP_WDT_BASE_ADDRESS+WDOG_EN_REG), reg.r);
}

//void wdt_ext_disable()
//{
//	WDOG_EN_REG_t reg;
//
//	reg.r = inl((BSP_WDT_BASE_ADDRESS+WDOG_EN_REG));
//	reg.b.WDOG_EXT_EN = WDT_DISABLE;
//
//	outl((BSP_WDT_BASE_ADDRESS+WDOG_EN_REG), reg.r);
//}

uint32_t wdt_getclksrc()
{
	WDOG_EN_REG_t reg;

	reg.r = inl((BSP_WDT_BASE_ADDRESS+WDOG_EN_REG));
	return reg.b.WDOG_CLOCK;
}

/*
* Setting Watchdog clock source
* void wdt_setclksrc(uint32_t clk)
* @param clk : WDT_CLKSRC_PCLK / WDT_CLKSRC_10MHZ
* @return :void
*/
void wdt_setclksrc(uint32_t clk)
{
	WDOG_EN_REG_t reg;

	reg.r = inl((BSP_WDT_BASE_ADDRESS+WDOG_EN_REG));
	reg.b.WDOG_CLOCK = clk;

	outl((BSP_WDT_BASE_ADDRESS+WDOG_EN_REG), reg.r);
}

uint32_t wdt_getflag()
{
	return inl((BSP_WDT_BASE_ADDRESS+WDOG_FLAG_REG));
}

void wdt_clr_flag()
{
	outl((BSP_WDT_BASE_ADDRESS+WDOG_CLR_FLAG_REG), 1);
}

void wdt_setlen(uint32_t len)
{
	outl((BSP_WDT_BASE_ADDRESS+WD_LENGTH_REG), len);
}

uint32_t wdt_getLen()
{
	uint32_t val ;
	val = inl((BSP_WDT_BASE_ADDRESS+WD_LENGTH_REG));
	return val;
}

void system_reboot()
{
	wdt_disable();
	wdt_intr_enable(0);
	wdt_clr_flag();
	wdt_setload(0);
	wdt_reset_enable(1);
	wdt_enable();
}

/*********************/
void wdt_init(WDT_INIT_t set)
{
	wdt_clr_flag();

	wdt_setclksrc(set.clk_src);
	wdt_setload(set.reload);

	wdt_reset_enable(set.reset);
	wdt_intr_enable(set.intr);

}
