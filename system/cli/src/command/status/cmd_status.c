#include <stdio.h>
#include <FreeRTOS.h>
#include <task.h>
#include <interrupt.h>
#include <bsp.h>
#include "cmd_status.h"
#include <nonstdlib.h>
#ifndef AUTOCONF_INCLUDED
#include <generated/snx_sdk_conf.h>
#endif

/** \defgroup system_cli Command Line Interface modules
 * \n
 * @{
 */
/** @} */
/** \defgroup cmd_status Status commands
 *  \ingroup system_cli
 * @{
 */

extern intr_counter_t intr_cnt;

/**
* @brief Display system information that about bus frequency
* @param None
* @details Example: sysinfo
*/
int cmd_stat_sysinfo(int argc, char* argv[])
{
	int n = 0;
	system_bus_freq_t *freq;
	TickType_t tick = 0;

	tick = xTaskGetTickCount();

	freq = clk_get();

	n = uxTaskGetNumberOfTasks();
	print_msg_queue("Number of Task : %d\n", n);
	print_msg_queue("Bootup time : %ds\n", tick / configTICK_RATE_HZ);
	print_msg_queue("CPU	: %d Hz\n", freq->fcpu);
	print_msg_queue("DDR	: %d Hz\n", freq->fddr);
	print_msg_queue("AHB	: %d Hz\n", freq->fahb);
	print_msg_queue("APB	: %d Hz\n", freq->fapb);
	print_msg_queue("AHB2	: %d Hz\n", freq->fahb2);
	print_msg_queue("ISP	: %d Hz\n", freq->fisp);

	return 0;
}

int cmd_stat_cpu(int argc, char* argv[])
{
	print_msg_queue("status cpu command %d %s\n", argc, argv[1]);

	return 0;
}

/**
* @brief Display interrupt received counter
* @param None
* @details Example: intr
*/
int cmd_stat_intr(int argc, char* argv[])
{
	int i = 0;

	print_msg_queue("Interrupt counter :\n");
	for (i = 0 ; i < 32 ; i++) {
		if (intr_cnt.cnt[i] > 0) {
			print_msg_queue("%02d : %d\n", i, intr_cnt.cnt[i]);
		}
	}

	return 0;
}

/** @} */
