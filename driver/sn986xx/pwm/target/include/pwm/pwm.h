/*
 * pwm.h
 *
 *  Created on: Apr 15, 2015
 *      Author: spark_lee
 */

#ifndef DRIVER_SN986XX_PWM_SRC_PWM_H_
#define DRIVER_SN986XX_PWM_SRC_PWM_H_

#define MAX_PWM_CLK_DIV				(255)			/*!< max clk divide value (1<<8 - 1)        */
#define MAX_PWM_PERIOD				(0xfffff)		/*!< max clk divide value (1<<20 - 1) */

#define BSP_PWM1_BASE_ADDRESS		(0x99000000)
#define BSP_PWM2_BASE_ADDRESS		(0x99100000)
#define BSP_PWM3_BASE_ADDRESS		(0x99300000)

#define PWM1		(BSP_PWM1_BASE_ADDRESS)
#define PWM2		(BSP_PWM2_BASE_ADDRESS)
#define PWM3		(BSP_PWM3_BASE_ADDRESS)

#define PWM_CTRL_REG		(0x00)
#define PWM_PERIOD_REG		(0x04)
#define PWM_DUTY_REG		(0x08)

typedef struct
{
	uint32_t pwm;
	uint32_t inv;
	uint32_t clk_div;
	uint32_t period;
	uint32_t duty;
}PWM_INIT_t;

typedef union
{
	uint32_t r;
	struct
	{
		uint32_t PWM_EN:1;
		uint32_t PWM_INV:1;
		uint32_t PWM_IO_I:1;
		uint32_t PWM_IO_O:1;
		uint32_t PWM_IO_OE:1;
		uint32_t Res1:3;
		uint32_t PWM_CK_DIV:8;
		uint32_t Res2:16;
	}b;
}PWM_CTRL_t;


void pwm_enable(uint32_t base, uint32_t en);
void pwm_set_inverse(uint32_t base, uint32_t val);

uint32_t pwm_gpio_in(uint32_t base);
void pwm_gpio_out(uint32_t base, uint32_t out);
void pwm_gpio_outputen(uint32_t base, uint32_t dir);

uint32_t pwm_get_clkdiv(uint32_t base);
void pwm_set_clkdiv(uint32_t base, uint32_t div);

uint32_t pwm_get_period(uint32_t base);
void pwm_set_period(uint32_t base, uint32_t val);

uint32_t pwm_get_duty(uint32_t base);
void pwm_set_duty(uint32_t base, uint32_t val);

uint32_t pwm_get_ctrl(uint32_t base);

void pwm_init(PWM_INIT_t set);


#endif /* DRIVER_SN986XX_PWM_SRC_PWM_H_ */
