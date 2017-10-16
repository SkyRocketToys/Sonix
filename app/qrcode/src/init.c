#include <FreeRTOS.h>
#include <bsp.h>

#include <interrupt.h>
#include <timer.h>
#include <uart/uart.h>
#include <bootinfo/bootinfo.h>
#include <i2c/i2c.h>
#include <isp/isp.h>
#include <gpio/gpio.h>
#include <sf/sf.h>
#include <sd/sd_ctrl.h>
#include <sd/sdv2_sd.h>
#include <libmid_sd/mid_sd.h>
#include <libmid_sf/mid_sf.h>
#include <libmid_fatfs/ff.h>
#include <audio/audio_dri.h>
#include <libmid_audio/audio.h>
#include <generated/snx_sdk_conf.h>
#include <libmid_nvram/snx_mid_nvram.h>
#ifdef CONFIG_SYSTEM_TRACE_SELECT
#include <trcUser.h>
#endif
//#include "mcu.h"


 /*
  * Performs initialization of all supported hardware.
  * All peripherals are stopped, their interrupt triggering is disabled, etc.
  */
void _init(void)
{
	/* get run in normal or rescue info */
	get_bootsel_info();

	/* Disable IRQ triggering (may be reenabled after ISRs are properly set) */
	irq_disableIrqMode();

	// Init bus freq table
	init_freq_table();

	/* Init the vectored interrupt controller */
	pic_init();

	/* Init all counters of all available timers */
	timer_init_prepare();

#ifdef CONFIG_SYSTEM_TRACE_SELECT
	vTraceInitTraceData();
	if (! uiTraceStart())
		print_msg("Error: trace start failed!!! \n");
#endif

	uart_init();

//	MCU_APP_INIT();

	/* Init timing task */
	timing_init();

	if (get_rtos_run_status() == 0) {
		print_msg("rtos run in NORMAL \n");
	}else if (get_rtos_run_status() == 1){
		print_msg("rtos run in RESCUE \n");
	}else {
		print_msg("rtos run in UNKNOW \n");
	}

	i2c_init();

	{
		gpio_pin_info info;

#if (CONFIG_SYSTEM_PLATFORM_SN98670 == 1) || (CONFIG_SYSTEM_PLATFORM_SN98671 == 1) || (CONFIG_SYSTEM_PLATFORM_SN98672 == 1)
		// Turn on GPIO0 WiFi power
		info.pinumber = 0;
		info.mode = 1;
		info.value = 1;
		snx_gpio_open();
		snx_gpio_write(info);
		snx_gpio_close();
#endif
		// pull high GPIO1 for Sensor
		info.pinumber = 1;
		info.mode = 1;
		info.value = 1;
		snx_gpio_open();
		snx_gpio_write(info);
		snx_gpio_close();
	}
	if(snx_isp_init() != pdPASS){
		//mcu_set_err_flag(ERR_SENSOR_ISP);
	}

//	audio_dri_init();
//	audio_init();

	if(sf_init() != SF_ERR_PASS){
//		mcu_set_err_flag(ERR_SF);
	}

	if(sd_init(MS_SD_MODE) != SD_RTN_PASS){
	//	mcu_set_err_flag(ERR_SDCARD);
	}
	mid_sd_init();
	mid_sf_init();
	if(snx_nvram_init() != NVRAM_SUCCESS){	/* snx_nvram_init function call, must be placed after the sf_init and mid_sf_init functions */
//		mcu_set_err_flag(ERR_NVRAM);
	}

	init_ff();
	return;
}
