#include <stdint.h>

#include <bsp.h>

#include <interrupt.h>
#include <timer.h>
#include <uart/uart.h>
#include <i2c/i2c.h>
#include <isp/isp.h>
#include <sf/sf.h>
#include <sd/sd_ctrl.h>
#include <sd/sdv2_sd.h>
#include <libmid_sd/mid_sd.h>
#include <libmid_sf/mid_sf.h>
#include <audio/audio_dri.h>
#include <libmid_audio/audio.h>
#include <generated/snx_sdk_conf.h>
#include <libmid_nvram/snx_mid_nvram.h>
#ifdef CONFIG_SYSTEM_TRACE_SELECT
#include <trcUser.h>
#endif

 /*
  * Performs initialization of all supported hardware.
  * All peripherals are stopped, their interrupt triggering is disabled, etc.
  */
void _init(void)
{
	/* Disable IRQ triggering (may be reenabled after ISRs are properly set) */
	irq_disableIrqMode();

	// Init bus freq table
	init_freq_table();

	/* Init the vectored interrupt controller */
	pic_init();

	/* Init all counters of all available timers */
	timer_init_prepare();

	uart_init();

#ifdef CONFIG_SYSTEM_TRACE_SELECT
	vTraceInitTraceData();
	if (! uiTraceStart())
		print_msg("Error: trace start failed!!! \n");
#endif

	i2c_init();
	//snx_i2c_init(0, I2C_STANDARD_CLK);
	//snx_i2c_init(1, I2C_STANDARD_CLK);

	snx_isp_init();
	audio_dri_init();
	audio_init();

	sf_init();
	sd_init(MS_SD_MODE);	 
	mid_sd_init();
	mid_sf_init();
	snx_nvram_init();	/* snx_nvram_init function call, must be placed after the sf_init and mid_sf_init functions */

	return;
}
