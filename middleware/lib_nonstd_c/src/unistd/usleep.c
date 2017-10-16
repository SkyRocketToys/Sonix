#define _GNU_SOURCE
#include <FreeRTOS.h>
#include <bsp.h>
#include <unistd.h>
#include <time.h>

#include <stdint.h>
//#include <timer/timer.h>

int usleep(unsigned useconds)
{
	unsigned int curr_counter = 0;
	unsigned int us_tick = 0, delay_match = 0;

	//vPortEnterCritical();

	us_tick = (useconds - 2) * 10;
	curr_counter = inl(0x98600020);

	if (us_tick < curr_counter) {
		delay_match = curr_counter - us_tick;
	} else {
		delay_match = (0x80000000 - 1) - us_tick - curr_counter;
	}

	while (delay_match < inl(0x98600020));

	//vPortExitCritical();

	return 0;
}

void udelay(unsigned useconds)
{
	unsigned int curr_counter = 0;
	unsigned int us_tick = 0, delay_match = 0;

	vPortEnterCritical();

	us_tick = (useconds - 2) * 10;
	curr_counter = inl(0x98600020);

	if (us_tick < curr_counter) {
		delay_match = curr_counter - us_tick;
	} else {
		delay_match = (0x80000000 - 1) - us_tick - curr_counter;
	}


	while (delay_match < inl(0x98600020));

	vPortExitCritical();
}
