/*
 * rtc.h
 *
 *  Created on: Apr 15, 2015
 *      Author: spark_lee
 */

#ifndef DRIVER_SN986XX_RTC_SRC_RTC_H_
#define DRIVER_SN986XX_RTC_SRC_RTC_H_


#define BSP_RTC_BASE_ADDRESS		(0x98500000)

typedef struct
{
	uint32_t val1:	8;
	uint32_t rev1:	24;

	uint32_t val2:	8;
	uint32_t rev2:	24;

	uint32_t val3:	8;
	uint32_t rev3:	24;
}RTC_VAL_t;

typedef struct
{
	uint32_t password1:	8;
	uint32_t rev1:		24;

	uint32_t password2:	8;
	uint32_t rev2:		24;
}RTC_PASSWORD_t;

typedef union
{
	uint32_t r;
	struct
	{
		uint32_t RTC_WR_TRG:1;
		uint32_t RTC_RD_TRG:1;
		uint32_t ALARM_EN:1;
		uint32_t WAKEUP_EN:1;
		uint32_t ALARM_FLAG:1;
		uint32_t WAKEUP_FLAG:1;
		uint32_t Res1:1;
		uint32_t ST_RST:1;
		uint32_t Res2:24;
	}b;
}RTC_CTRL_t;

typedef union
{
	uint32_t r;
	struct
	{
		uint32_t RTC_RD_RDY:1;
		uint32_t RTC_WR_RDY:1;
		uint32_t RTC_CTL:1;
		uint32_t RTC_PG_RST:1;
		uint32_t KEY:1;
		uint32_t Res:24;
	}b;
}RTC_STAT_t;


#define RTC_GET_VAL(addr)	((inl(addr)&0xff)|((inl(addr+4)&0xff)<<8)|((inl(addr+8)&0xff)<<16))

#define RTC_PASSWD1_CODE	(0x56)
#define RTC_PASSWD2_CODE	(0xa9)

#define RTC_WR_TRG_FLAG		(1)

/*----------- Register Offset -----------*/
#define RTC_TIMER_REG		(0x0)
#define ALARM_TIMER_REG		(0x10)
#define WAKEUP_TIMER_REG	(0x20)

#define RTC_CTRL_REG		(0x2c)
#define DATA_BUF_ADDR_REG	(0x30)
#define DATA_BUF_REG		(0x34)
#define RTC_PASSWORD_REG	(0x38)
#define RTC_PASSWORD1_REG	(0x38)
#define RTC_PASSWORD2_REG	(0x3c)
#define RTC_STAT_REG		(0x40)

#define RTC_BELOW_FLAG_BUF_ADDR		0x5 //0x6-7 for key in init.c
#define DEFAULT_RTC_BASE_UNIX_TIME	0x3FFFFFFF
#define DEFAULT_RTC_BASE_YEAR	2004
#define DEFAULT_RTC_BASE_MONTH	1
#define DEFAULT_RTC_BASE_DAY	10
#define DEFAULT_RTC_BASE_HOUR	13
#define DEFAULT_RTC_BASE_MIN	37
#define DEFAULT_RTC_BASE_SEC	03

int rtc_wait_rd_ready();
int rtc_wait_wr_ready();
void rtc_setpasswd();
void rtc_reset();
uint32_t rtc_getstat();
long long rtc_get_rtctimer();
void rtc_set_rtctimer(long long val);
uint32_t rtc_get_alarmtimer();
void rtc_set_alarmtimer(uint32_t val);
void rtc_alerme_enable(uint32_t en);
uint32_t rtc_get_wakeuptimer();
void rtc_set_wakeuptimer(uint32_t val);
void rtc_wakeup_enable(uint32_t en);
uint32_t rtc_get_databufaddr();
void rtc_set_databufaddr(uint32_t addr);
uint32_t rtc_get_databuf();
void rtc_set_databuf(uint32_t addr);



#endif /* DRIVER_SN986XX_RTC_SRC_RTC_H_ */
