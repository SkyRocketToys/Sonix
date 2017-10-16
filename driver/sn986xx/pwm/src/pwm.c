/*
 * pwm.c
 *
 *  Created on: Apr 15, 2015
 *      Author: spark_lee
 */


#include "FreeRTOS.h"

#include <bsp.h>
#include <pwm.h>



void pwm_enable(uint32_t base, uint32_t en)
{
	PWM_CTRL_t ctrl;

	ctrl.r = inl((base+PWM_CTRL_REG));
	ctrl.b.PWM_EN = en;

	outl((base+PWM_CTRL_REG), ctrl.r);
}

void pwm_set_inverse(uint32_t base, uint32_t val)
{
	PWM_CTRL_t ctrl;

	ctrl.r = inl((base+PWM_CTRL_REG));
	ctrl.b.PWM_INV = val;

	outl((base+PWM_CTRL_REG), ctrl.r);
}


uint32_t pwm_gpio_in(uint32_t base)
{
	PWM_CTRL_t ctrl;

	ctrl.r = inl((base+PWM_CTRL_REG));

	return ctrl.b.PWM_IO_I;
}

void pwm_gpio_out(uint32_t base, uint32_t out)
{
	PWM_CTRL_t ctrl;

	ctrl.r = inl((base+PWM_CTRL_REG));
	ctrl.b.PWM_IO_O = out;

	outl((base+PWM_CTRL_REG), ctrl.r);

}

void pwm_gpio_outputen(uint32_t base, uint32_t dir)
{
	PWM_CTRL_t ctrl;

	ctrl.r = inl((base+PWM_CTRL_REG));
	ctrl.b.PWM_IO_OE = dir;

	outl((base+PWM_CTRL_REG), ctrl.r);

	/*fix HW Bug*/
	if(base == PWM2 && dir == 1){
		ctrl.r = inl((PWM1+PWM_CTRL_REG));
		ctrl.b.PWM_IO_OE = 1;
		outl((PWM1+PWM_CTRL_REG), ctrl.r);
	}
	/************/
}

uint32_t pwm_get_clkdiv(uint32_t base)
{
	PWM_CTRL_t ctrl;

	ctrl.r = inl((base+PWM_CTRL_REG));

	return ctrl.b.PWM_CK_DIV + 1;

}

void pwm_set_clkdiv(uint32_t base, uint32_t div)
{
	PWM_CTRL_t ctrl;

	if(div<1){
		div = 1;
	}

	ctrl.r = inl((base+PWM_CTRL_REG));
	ctrl.b.PWM_CK_DIV = div-1;

	outl((base+PWM_CTRL_REG), ctrl.r);
}

uint32_t pwm_get_period(uint32_t base)
{
	return inl((base+PWM_PERIOD_REG))+1;
}

void pwm_set_period(uint32_t base, uint32_t val)
{
	if(val<1)
	{
		val = 1;
	}

	outl((base+PWM_PERIOD_REG), val-1);
}

uint32_t pwm_get_duty(uint32_t base)
{
	return inl((base+PWM_DUTY_REG));
}

void pwm_set_duty(uint32_t base, uint32_t val)
{
	outl((base+PWM_DUTY_REG), val);
}


uint32_t pwm_get_ctrl(uint32_t base)
{
	return inl((base+PWM_CTRL_REG));
}


/*************************/
void pwm_init(PWM_INIT_t set)
{
	pwm_set_inverse(set.pwm, set.inv);
	pwm_set_clkdiv(set.pwm, set.clk_div);
	pwm_set_period(set.pwm, set.period);
	pwm_set_duty(set.pwm, set.duty);

}
