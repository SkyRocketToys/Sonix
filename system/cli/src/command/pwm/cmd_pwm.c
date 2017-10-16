#include <FreeRTOS.h>
#include <stdio.h>
#include "cmd_pwm.h"
#include "cmd_debug.h"
#include "printlog.h"
#include <nonstdlib.h>
#include <pwm/pwm.h>
#include <string.h>
#include <bsp.h>
#include <nonstdlib.h>

#define ONE_MSEC   1000000
#define printf(fmt, args...) if(1) print_msg_queue((fmt), ##args)
#define DEBUG 0
#define DEBUG_PRINT(fmt, args...) if(DEBUG) print_msg_queue((fmt), ##args)

typedef struct {
	PWM_INIT_t pwm_set;
	uint32_t pwm_clk_ns;
	uint32_t clk_div_val;
	unsigned long clk_div_ns;
	unsigned long period_ns;
	unsigned long duty_ns;
} PWM_INFO_t;

/** \fn void calculate_clk_div(struct pwm_device *pwm, unsigned int div_start_val, unsigned int div_end_val,  unsigned long period_ns, unsigned int *div_val, unsigned long *div_ns)
  * \brief calculate clock divide time and value
  * \param pwm :The device of pwm
  * \param div_start_val :the begin value of divide clock
  * \param div_end_val :the end value of divide clock
  * \param period_ns :period time
  * \param *div_val :get clock divide value
  * \param *div_ns :get clock divide time
  * 
  */
void calculate_clk_div(PWM_INFO_t *pwm_param, unsigned int div_start_val, unsigned int div_end_val,  unsigned long period_ns, unsigned int *div_val, unsigned long *div_ns)
{
	unsigned int mid;
	unsigned long temp_clk_ns;
	unsigned long temp_period_val;

	DEBUG_PRINT("%s:%d div_start_val = %d  div_end_val=%d\n\n", __func__,  __LINE__, div_start_val,           div_end_val);

	/* period_ns = (div_val + 1) * clk_ns * (period_val + 1) */
	mid = (div_start_val + div_end_val + 1) / 2;
	temp_clk_ns = (mid + 1) * pwm_param->pwm_clk_ns;
	temp_period_val = period_ns / temp_clk_ns;
	if(temp_period_val)
		temp_period_val -= 1;

	if(div_start_val == div_end_val){
		if(MAX_PWM_PERIOD < temp_period_val)
			*div_val = div_start_val + 1;
		else
			*div_val = div_start_val;
		*div_ns = (*div_val + 1) * pwm_param->pwm_clk_ns;
		DEBUG_PRINT("%s:%d div_start_val = %d  div_end_val=%d\n\n", __func__,  __LINE__, div_start_val,       div_end_val);

		return;
	}

	/* judge mid if it is too little */
	if(MAX_PWM_PERIOD < temp_period_val)
		calculate_clk_div(pwm_param, mid + 1, div_end_val, period_ns, div_val, div_ns);
    else if(MAX_PWM_PERIOD > temp_period_val)
        calculate_clk_div(pwm_param, div_start_val, mid -1, period_ns, div_val, div_ns);
    else{
		*div_val = mid;
        *div_ns = mid * pwm_param->pwm_clk_ns;
		DEBUG_PRINT("%s:%d mid=%d\n\n", __func__,  __LINE__, mid);

    }
	return;
}

static void pwm_ctrl_usage() {   
	printf(" Usage: pwm_ctrl [PWM_ID] [Duty Time] [Period Time]\n");
	printf(" PWM_ID: 0: PWM1, 1: PWM2, 2: PWM3\n");
	printf(" Duty Time: duty keep time(millisecond ) must > 0\n");
	printf(" Period Time: period time (millisecond ) must > 0\n");
	printf(" Example: \n");
	printf("      pwm_ctrl 0 500 1000\n");
}  

int cmd_pwm_ctrl(int argc, char* argv[])
{                          

	int device = 0;
	int duty_time = 0;
	int period_time = 0;
	PWM_INFO_t pwm_param;
	PWM_INIT_t *pwm;

	pwm = &pwm_param.pwm_set;

	if (argc < 4) {
		pwm_ctrl_usage();
		return pdFAIL;
	}

	device = simple_strtoul(argv[1], NULL, 10);
	if ((device < 0) || (device > 2))
	{
		printf("Wrong device number (%d)\n\n", device);
		pwm_ctrl_usage();
		return pdFAIL;
	}

	duty_time = simple_strtoul(argv[2], NULL, 10);
	if (duty_time <= 0)
	{
		printf("Duty Time MUST > 0 (%d)\n\n", duty_time);
		pwm_ctrl_usage();
		return pdFAIL;
	}

	period_time = simple_strtoul(argv[3], NULL, 10);
	if (period_time <= 0)
	{
		printf("Period Time MUST > 0 (%d)\n\n", period_time);
		pwm_ctrl_usage();
		return pdFAIL;
	}

	if(device == 0) {
		pwm->pwm = PWM1;
	} else if(device == 1) {
		pwm->pwm = PWM2;
	} else if(device == 2) {
		pwm->pwm = PWM3;
	} else {
		pwm_ctrl_usage();
		return pdFAIL;
	}

    if (duty_time >= period_time)
    {
		pwm_param.duty_ns= period_time * ONE_MSEC;
		pwm_param.period_ns= duty_time * ONE_MSEC;
    }
    else
    {
		pwm_param.duty_ns= duty_time * ONE_MSEC;
		pwm_param.period_ns= period_time * ONE_MSEC;
    }

	pwm_param.pwm_clk_ns = 1000000000 / clk_get()->fapb;
	pwm_param.clk_div_val = pwm_get_clkdiv(pwm->pwm);
 	pwm->inv = 0;

 	/* 
 		1. Disable PWM 
 		2. Calculate PWM DIV
 		3. CONFIG Period and Duty 
 		4. Enable PWM
 	*/
 	pwm_enable(pwm->pwm, 0);

 	pwm_set_inverse(pwm->pwm, pwm->inv);

 	calculate_clk_div(&pwm_param, 1, MAX_PWM_CLK_DIV, pwm_param.period_ns, &pwm->clk_div, &pwm_param.clk_div_ns);
	if (pwm_param.clk_div_val != pwm->clk_div)
		pwm_set_clkdiv(pwm->pwm, pwm->clk_div);

	pwm->period = pwm_param.period_ns / pwm_param.clk_div_ns;
	pwm->duty = pwm_param.duty_ns / pwm_param.clk_div_ns;

	pwm_set_period(pwm->pwm, pwm->period);
	pwm_set_duty(pwm->pwm, pwm->duty);
	
	pwm_enable(pwm->pwm, 1);

	printf("PWM%d Config Period time: %d ms Duty time: %d ms\n", device, pwm_param.period_ns / ONE_MSEC, pwm_param.duty_ns / ONE_MSEC);

	return pdPASS;   
}
