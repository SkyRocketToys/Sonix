/*
 * timer.h
 *
 *  Created on: Apr 20, 2015
 *      Author: spark_lee
 */

#ifndef DRIVER_SN986XX_TIMER_SRC_TIMER_H_
#define DRIVER_SN986XX_TIMER_SRC_TIMER_H_

#define BSP_TIMER_BASE_ADDRESS		(0x98600000)

#define TIMER_1		(1)
#define TIMER_2		(2)
#define TIMER_3		(3)


#define TM1_COUNTER_REG		(0x00)
#define TM1_LOAD_REG		(0x04)
#define TM1_MATCH1_REG		(0x08)
#define TM1_MATCH2_REG		(0x0c)

#define TM2_COUNTER_REG		(0x10)
#define TM2_LOAD_REG		(0x14)
#define TM2_MATCH1_REG		(0x18)
#define TM2_MATCH2_REG		(0x1c)

#define TM3_COUNTER_REG		(0x20)
#define TM3_LOAD_REG		(0x24)
#define TM3_MATCH1_REG		(0x28)
#define TM3_MATCH2_REG		(0x2c)


#define TMx_CTRL_REG		(0x30)
#define TMx_FLAG_REG		(0x34)
#define TMx_MASK_REG		(0x38)

#define TMx_PCLK			(0)
#define TMx_EXTCLK			(1)

#define TMx_DOWNCOUNT		(0)
#define TMx_UPCOUNT			(1)

typedef struct
{
	uint32_t timer;
	uint32_t counter;
	uint32_t match1;
	uint32_t match1_mask;
	uint32_t match2;
	uint32_t match2_mask;
	uint32_t load;
	uint32_t updown;
	uint32_t clk_src;

}TIMER_INIT_t;



typedef union
{
		uint32_t r;
		struct
		{
			uint32_t TM1_EN:1;
			uint32_t TM1_CLOCK:1;
			uint32_t TM1_OF_EN:1;

			uint32_t TM2_EN:1;
			uint32_t TM2_CLOCK:1;
			uint32_t TM2_OF_EN:1;

			uint32_t TM3_EN:1;
			uint32_t TM3_CLOCK:1;
			uint32_t TM3_OF_EN:1;

			uint32_t TM1_UPDOWN:1;
			uint32_t TM2_UPDOWN:1;
			uint32_t TM3_UPDOWN:1;

			uint32_t Res:20;
		}b;
}TMx_CTRL_t;

typedef union
{
	uint32_t r;
	struct
	{
		//read only
		uint32_t TM1_MATCH1_FLAG:1;
		uint32_t TM1_MATCH2_FLAG:1;
		uint32_t TM1_OF_FLAG:1;

		uint32_t TM2_MATCH1_FLAG:1;
		uint32_t TM2_MATCH2_FLAG:1;
		uint32_t TM2_OF_FLAG:1;

		uint32_t TM3_MATCH1_FLAG:1;
		uint32_t TM3_MATCH2_FLAG:1;
		uint32_t TM3_OF_FLAG:1;

		//write only
		uint32_t CLR_TM1_MATCH1_FLAG:1;	//bit 9
		uint32_t CLR_TM1_MATCH2_FLAG:1;
		uint32_t CLR_TM1_OF_FLAG:1;

		uint32_t CLR_TM2_MATCH1_FLAG:1;	//bit 12
		uint32_t CLR_TM2_MATCH2_FLAG:1;
		uint32_t CLR_TM2_OF_FLAG:1;

		uint32_t CLR_TM3_MATCH1_FLAG:1;	//bit 15
		uint32_t CLR_TM3_MATCH2_FLAG:1;
		uint32_t CLR_TM3_OF_FLAG:1;

		uint32_t Res:14;
	}b;
}TMx_FLAG_t;

typedef union
{
	uint32_t r;
	struct
	{

		uint32_t TM1_MATCH1_MSK:1;
		uint32_t TM1_MATCH2_MSK:1;
		uint32_t TM1_OF_MSK:1;

		uint32_t TM2_MATCH1_MSK:1;
		uint32_t TM2_MATCH2_MSK:1;
		uint32_t TM2_OF_MSK:1;

		uint32_t TM3_MATCH1_MSK:1;
		uint32_t TM3_MATCH2_MSK:1;
		uint32_t TM3_OF_MSK:1;

		uint32_t Res:23;
	}b;
}TMx_MASK_t;

void timer_set_counter(uint32_t timer, uint32_t val);
void timer_set_load(uint32_t timer, uint32_t val);
void timer_set_match1(uint32_t timer, uint32_t match1);
void timer_set_match2(uint32_t timer, uint32_t match2);

uint32_t timer_get_counter(uint32_t timer);
uint32_t timer_get_load(uint32_t timer);
uint32_t timer_get_match1(uint32_t timer);
uint32_t timer_get_match2(uint32_t timer);

void timer_set_enable(uint32_t timer, uint32_t en);
void timer_set_clksrc(uint32_t timer, uint32_t val);
void timer_set_intr_enable(uint32_t timer, uint32_t en);
void timer_set_updown(uint32_t timer, uint32_t val);

void timer_clr_match1_flag(uint32_t timer);
void timer_clr_match2_flag(uint32_t timer);
void timer_clr_overflow_flag(uint32_t timer);

void timer_set_match1_mask(uint32_t timer, uint32_t val);
void timer_set_match2_mask(uint32_t timer, uint32_t val);
void timer_set_overflow_mask(uint32_t timer, uint32_t val);

uint32_t timer_get_ctrl();
uint32_t timer_get_flag();
uint32_t timer_get_mask();

void timer_gen_init(TIMER_INIT_t set);
void timer_test();

#endif /* DRIVER_SN986XX_TIMER_SRC_TIMER_H_ */
