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
#include <audio/audio_dri.h>
#include <libmid_audio/audio.h>
#include <generated/snx_sdk_conf.h>
#include <libmid_nvram/snx_mid_nvram.h>
#include <libmid_fatfs/ff.h>
#include "mcu/mcu.h"
#ifdef CONFIG_SYSTEM_TRACE_SELECT
#include <trcUser.h>
#endif


static serialflash_instr_t Sf_instr[] = {
	{
		.WREN		= 0x06,
		.WRDI		= 0x04,
		.RDID		= 0x9f,
		.RDSR		= 0x05,
		.WRSR		= 0x01,
		.READ		= 0x03,
		.PP 		= 0x02,
		.SE 		= 0x20,
		.BE 		= 0xd8,
		.CE 		= 0x60,
	}
};

static serialflash_int_t sf_init_struct[] = {
	{
		.sf_inst		= &Sf_instr[0],
		.serial_flash	= {
			.name		= "MXIC",				//MX25L12805
			.id0		= 0xC2,
			.id1		= 0x20,
			.id2		= 0x88,
			.page_size	= 256,					//256 byte
			.sector_size= 256 * 16,				//4K byte
			.block_size	= 256 * 16 * 16,		//64K byte
			.chip_size	= 256 * 16 * 16 * 256,	//16M byte
		},
		.max_hz 		= 33 * 1000 * 1000, 	//33Mhz
		.readID 		= NULL,
		.chip_erase 	= NULL,
		.sector_erase	= NULL,
		.block_erase	= NULL,
		.sf_write		= NULL,
		.sf_read		= NULL,
		.clearSR		= NULL,
		.protectSR		= NULL,
	},
};


#ifdef CONFIG_MODULE_RTC_SUPPORT
// #include <rtc/rtc.h>

// #define RTC_KEY_ADDR		0x6 // reserve addr 0-5
// #define RTC_KEY			0xA55A

// system_date_t g_sys_default_time = {2016, 1, 1, 0, 0, 0, 0};


// void rtc_init()
// {
// 	system_date_t sys_date = g_sys_default_time;
// 	long long rtc_sec = 0;
// 	volatile unsigned key = 0;

// 	rtc_set_databufaddr(RTC_KEY_ADDR);
// 	key = (rtc_get_databuf() & 0xFF) << 8;
// 	rtc_set_databufaddr(RTC_KEY_ADDR + 1);
// 	key |= rtc_get_databuf() & 0xFF;

// 	rtc_reset();

// 	if (key != RTC_KEY) {
// 		print_msg("set rtc key!!!\n");
// 		// first time boot-up
// 		rtc_sec = tm_to_time(&sys_date, 0);
// 		rtc_set_rtctimer(rtc_sec);

// 		set_date(&sys_date, 0);

// 		rtc_set_databufaddr(RTC_KEY_ADDR);
// 		rtc_set_databuf((RTC_KEY & 0xFF00) >> 8);
// 		rtc_set_databufaddr(RTC_KEY_ADDR + 1);
// 		rtc_set_databuf(RTC_KEY & 0xFF);
// 	} else {
// 		// Init System Time from RTC
// 		rtc_sec = (long long)rtc_get_rtctimer();
// 		time_to_tm(rtc_sec, 0, &sys_date);
// 		set_date(&sys_date, 0);
// 	}
// }
#endif // defined CONFIG_MODULE_RTC_SUPPORT


 /*
  * Performs initialization of all supported hardware.
  * All peripherals are stopped, their interrupt triggering is disabled, etc.
  */
void _init(void)
{
	gpio_pin_info info;
	unsigned char drv;
	
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
	// vTraceInitTraceData();
	// if (! uiTraceStart())
	// 	print_msg("Error: trace start failed!!! \n");
#endif

	uart_init();

#ifndef CONFIG_APP_DRONE
	// /* Init MCU Task */
	// mcu_init();
	// mcu_connect_notask();
#endif
	/* Init timing task */
	timing_init();

	if (get_rtos_run_status() == 0) {
		print_msg("rtos run in NORMAL \n");
	}else if (get_rtos_run_status() == 1){
		clear_forced_to_rescue_flag();
		print_msg("rtos run in RESCUE \n");
	}else {
		print_msg("rtos run in UNKNOW \n");
	}

#if defined(CONFIG_MODULE_RTC_SUPPORT) && !defined(CONFIG_APP_DRONE)
	// rtc_init();
#endif

	// i2c_init();
	// {
	// 	// Pull high GPIO1 for Sensor
	// 	info.pinumber = 1;
	// 	info.mode = 1;
	// 	info.value = 1;
	// 	snx_gpio_open();
	// 	snx_gpio_write(info);
	// 	snx_gpio_close();
	// }
// 	if(snx_isp_init() != pdPASS){
// #ifndef CONFIG_APP_DRONE
// 		mcu_set_err_flag(ERR_SENSOR_ISP);
// #endif
// 	}

	// audio_dri_init();
	// audio_init();
	
	sf_license(&sf_init_struct[0],sizeof(sf_init_struct)/sizeof(serialflash_int_t)); 
	if(sf_init() != SF_ERR_PASS){
#ifndef CONFIG_APP_DRONE
		// mcu_set_err_flag(ERR_SF);
#endif
	}

#if (CONFIG_RESCUE_SD_UPGRADE == 1)
	if(sd_init(MS_SD_MODE) != SD_RTN_PASS){
#ifndef CONFIG_APP_DRONE
		// mcu_set_err_flag(ERR_SDCARD);
#endif
	}
	mid_sd_init();
#endif

	mid_sf_init();


	// if(snx_nvram_init() != NVRAM_SUCCESS){	/* snx_nvram_init function call, must be placed after the sf_init and mid_sf_init functions */
// #ifndef CONFIG_APP_DRONE
// 		mcu_set_err_flag(ERR_NVRAM);
// #endif
// 	}


#ifdef CONFIG_USBH_STORAGE_SUPPORT
#if (CONFIG_RESCUE_SD_UPGRADE == 1)	
	f_set_drv(2);
	drv=f_get_drv();
	print_msg ("usbh storage support drv = %x\n", drv);
#endif
#endif

#if (CONFIG_RESCUE_SD_UPGRADE == 1)	
	init_ff();
#endif
	{
#if (CONFIG_SYSTEM_PLATFORM_SN98670 == 1) || (CONFIG_SYSTEM_PLATFORM_SN98671 == 1) || (CONFIG_SYSTEM_PLATFORM_SN98672 == 1)
#ifndef CONFIG_USBH_STORAGE_SUPPORT
		// Turn on GPIO0 WiFi power
		// info.pinumber = 0;
		// info.mode = 1;
		// info.value = 1;
		// snx_gpio_open();
		// snx_gpio_write(info);
		// snx_gpio_close();
#endif
#endif
	}
	return;
}
