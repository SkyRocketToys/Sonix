#include <FreeRTOS.h>
#include <task.h>
#include <time.h>
#include <sys/time.h>
#include <bsp.h>
#include <sys_clock.h>

int gettimeofday(struct timeval *tv, void *tz)
{
	TickType_t tick = 0;
	unsigned int usec = 0;

	tick = xTaskGetTickCount();

	//tv->tv_sec = tick / 100;
	tv->tv_sec = get_sys_seconds();
	usec = (configTIMER_CLOCK_HZ / configTICK_RATE_HZ) - inl(0x98600000);
	tv->tv_usec = (((tick % configTICK_RATE_HZ) * (portTICK_PERIOD_MS * 1000)) + (usec / (configTIMER_CLOCK_HZ / 1000000)));

	return 0;
}

int gettimeofdayFromISR(struct timeval *tv, void *tz)
{
	TickType_t tick = 0;
	unsigned int usec = 0;

	tick = xTaskGetTickCountFromISR();

	//tv->tv_sec = tick / 100;
	tv->tv_sec = get_sys_seconds();
	usec = (configTIMER_CLOCK_HZ / configTICK_RATE_HZ) - inl(0x98600000);
	tv->tv_usec = (((tick % configTICK_RATE_HZ) * (portTICK_PERIOD_MS * 1000)) + (usec / (configTIMER_CLOCK_HZ / 1000000)));

	return 0;
}
