/*
 * wdt.h
 *
 *  Created on: Apr 15, 2015
 *      Author: spark_lee
 */

#ifndef DRIVER_SN986XX_WATCH_DOG_WDT_H_
#define DRIVER_SN986XX_WATCH_DOG_WDT_H_


#define BSP_WDT_BASE_ADDRESS		(0x98700000)


#define WDT_ENABLE	(1)
#define WDT_DISABLE	(0)

#define WDT_RESTART_CODE	(0x5AB9)

#define WDT_CLKSRC_PCLK		(0)
#define WDT_CLKSRC_10MHZ	(1)

/*------- Register Offset ---------*/
#define WDOG_COUNTER_REG		(0x0)
#define WDOG_LOAD_REG			(0x4)
#define WDOG_RESTART_REG		(0x8)
#define WDOG_EN_REG				(0xc)
#define WDOG_FLAG_REG			(0x10)
#define WDOG_CLR_FLAG_REG		(0x14)
#define WD_LENGTH_REG			(0x18)

typedef struct
{
	uint32_t reload;
	uint32_t clk_src;

	uint32_t reset;
	uint32_t intr;
}WDT_INIT_t;

typedef union
{
	uint32_t r;
	struct
	{
		uint32_t WDOG_EN		:1;
		uint32_t WDOG_RST_EN	:1;
		uint32_t WDOG_INTR_EN	:1;
		uint32_t WDOG_EXT_EN	:1;
		uint32_t WDOG_CLOCK		:1;
		uint32_t Reserved		:27;
	}b;
}WDOG_EN_REG_t;



uint32_t wdt_getcount();
void wdt_restart();
uint32_t wdt_getload();
void wdt_setload(uint32_t count);
void wdt_enable();
void wdt_disable();
void wdt_reset_enable(uint32_t en);
void wdt_intr_enable(uint32_t en);
void wdt_ext_enable(uint32_t en);
uint32_t wdt_getclksrc();
void wdt_setclksrc(uint32_t clk);
uint32_t wdt_getflag();
void wdt_clr_flag();
void wdt_setlen(uint32_t len);
uint32_t wdt_getLen();
void wdt_init(WDT_INIT_t set);

void system_reboot(void);


#endif /* DRIVER_SN986XX_WATCH_DOG_WDT_H_ */
