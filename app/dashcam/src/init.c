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
#include "generated/snx_flashlayout_conf.h"


#ifdef CONFIG_SYSTEM_TRACE_SELECT
#include <trcUser.h>
#endif

/**
 * @defgroup app_dashcam Dashcam Application modules
 * @brief Dashcam Application system
 * @{
 */
 
 
#ifdef CONFIG_MODULE_RTC_SUPPORT
#include <rtc/rtc.h>


#define RTC_KEY_ADDR		0x6 /**< the addr of rtc key.(reserve addr 0-5) */ 
#define RTC_KEY			0xA55A	/**< rtc key. */

system_date_t g_sys_default_time = {2016, 1, 1, 0, 0, 0, 0};

void ext_set_nvram_addr(void);

/**
 * @brief initialize RTC.
 */
void rtc_init()
{
	system_date_t sys_date = g_sys_default_time;
	long long rtc_sec = 0;
	volatile unsigned key = 0;

	rtc_set_databufaddr(RTC_KEY_ADDR);
	key = (rtc_get_databuf() & 0xFF) << 8;
	rtc_set_databufaddr(RTC_KEY_ADDR + 1);
	key |= rtc_get_databuf() & 0xFF;

	rtc_reset();

	if (key != RTC_KEY) {
		print_msg("set rtc key!!!\n");
		// first time boot-up
		rtc_sec = tm_to_time(&sys_date, 0);
		rtc_set_rtctimer(rtc_sec);

		set_date(&sys_date, 0);

		rtc_set_databufaddr(RTC_KEY_ADDR);
		rtc_set_databuf((RTC_KEY & 0xFF00) >> 8);
		rtc_set_databufaddr(RTC_KEY_ADDR + 1);
		rtc_set_databuf(RTC_KEY & 0xFF);
	} else {
		// Init System Time from RTC
		rtc_sec = (long long)rtc_get_rtctimer();
		time_to_tm(rtc_sec, 0, &sys_date);
		set_date(&sys_date, 0);
	}
}
#endif // defined CONFIG_MODULE_RTC_SUPPORT

static serialflash_instr_t Sf_instr[] = {
	{
		.WREN		 = 0x06,
		.WRDI		 = 0x04,
		.RDID		 = 0x9f,
		.RDSR		 = 0x05,
		.WRSR		 = 0x01,
		.READ		 = 0x03,
		.PP		 = 0x02,
		.SE		 = 0x20,
		.BE		 = 0xd8,
		.CE		 = 0x60,
	}
};

static serialflash_int_t sf_init_struct[] = {
	{
		.sf_inst		 = &Sf_instr[0],
		.serial_flash	 = {
			.name		 = "MXIC",				 //MX25L12805
			.id0		 = 0xC2,
			.id1		 = 0x20,
			.id2		 = 0x88,
			.page_size  = 256, 				 //256 byte
			.sector_size= 256 * 16,			 //4K byte
			.block_size = 256 * 16 * 16,		 //64K byte
			.chip_size  = 256 * 16 * 16 * 256,  //16M byte
		},
		.max_hz		 = 33 * 1000 * 1000,	 //33Mhz
		.readID		 = NULL,
		.chip_erase	 = NULL,
		.sector_erase	 = NULL,
		.block_erase	 = NULL,
		.sf_write		 = NULL,
		.sf_read		 = NULL,
		.clearSR		 = NULL,
		.protectSR 	 = NULL,
	},
};

static bool tried_isp_reinit;

/*
  set a GPIO pin
 */
static void set_gpio(unsigned pin, unsigned value)
{
    gpio_pin_info info;
    info.pinumber = pin;
    info.mode = 1;
    info.value = value;
    snx_gpio_open();
    snx_gpio_write(info);
    snx_gpio_close();
}

/*
  this hook is used to detect isp initialisation failure 
 */
void isp_print_msg_hook(const char *fmt)
{
    const char *match_str = "[isp] sensor load module failed";
    if (strncmp(fmt, match_str, strlen(match_str)) != 0) {
        return;
    }
    /*
      the OV9732 sensors needs GPIO1 high, the H62 needs it low. We
      use this hook to allow us to detect sensor failure and change
      GPIO1. This allows for a single firmware that supports both
      sensors.
     */
    if (!tried_isp_reinit) {
        tried_isp_reinit = true;
        print_msg("trying GPIO1=0\n");
        set_gpio(1, 0);
        snx_isp_init();
    }
}

 /*
  * Performs initialization of all supported hardware.
  * All peripherals are stopped, their interrupt triggering is disabled, etc.
  */

/**
 * @brief initialize RTOS system.
 */
void _init(void)
{
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
	vTraceInitTraceData();
	if (! uiTraceStart())
		print_msg("Error: trace start failed!!! \n");
#endif

	uart_init();

#ifndef CONFIG_APP_DRONE
	/* Init MCU Task */
	mcu_init();
	mcu_connect_notask();
#endif
	/* Init timing task */
	timing_init();

	if (get_rtos_run_status() == 0) {
		print_msg("rtos run in NORMAL \n");
	}else if (get_rtos_run_status() == 1){
		print_msg("rtos run in RESCUE \n");
	}else {
		print_msg("rtos run in UNKNOW \n");
	}

#if defined(CONFIG_MODULE_RTC_SUPPORT) && !defined(CONFIG_APP_DRONE)
	rtc_init();
#endif

	i2c_init();

        print_msg("trying GPIO1=1\n");
        set_gpio(1, 1);
        
	if(snx_isp_init() != pdPASS){
#ifndef CONFIG_APP_DRONE
            mcu_set_err_flag(ERR_SENSOR_ISP);
#endif
        }
       
	audio_dri_init();
	audio_init();
	
	sf_license(&sf_init_struct[0],sizeof(sf_init_struct)/sizeof(serialflash_int_t)); 
	if(sf_init() != SF_ERR_PASS){
#ifndef CONFIG_APP_DRONE
		mcu_set_err_flag(ERR_SF);
#endif
	}

	if(sd_init(MS_SD_MODE) != SD_RTN_PASS){
#ifndef CONFIG_APP_DRONE
		mcu_set_err_flag(ERR_SDCARD);
#endif
	}
	mid_sd_init();
	mid_sf_init();
	ext_set_nvram_addr(); 
	if(snx_nvram_init() != NVRAM_SUCCESS){	/* snx_nvram_init function call, must be placed after the sf_init and mid_sf_init functions */
#ifndef CONFIG_APP_DRONE
		mcu_set_err_flag(ERR_NVRAM);
#endif
	   print_msg ("nvram fail !!! forced to rescue !!!\n");
	   forced_to_rescue();
	   while(1);
	}


#ifdef CONFIG_USBH_STORAGE_SUPPORT
	f_set_drv(2);
	drv=f_get_drv();
	print_msg ("usbh storage support drv = %x\n", drv);
#endif

	init_ff();
	{
#if (CONFIG_SYSTEM_PLATFORM_SN98670 == 1) || (CONFIG_SYSTEM_PLATFORM_SN98671 == 1) || (CONFIG_SYSTEM_PLATFORM_SN98672 == 1)
#ifndef CONFIG_USBH_STORAGE_SUPPORT
		// Turn on GPIO0 WiFi power
            set_gpio(0, 1);
#endif
#endif
	}
	return;
}
/** @} */
void ext_set_nvram_addr(void) 
{
	set_nvram_addr (factory_st, 0);
	set_nvram_addr (factory_nd, 1);
	set_nvram_addr (nvram_st, 2);
	set_nvram_addr (nvram_nd, 3);
	set_nvram_addr (data_st, 4);
	set_nvram_addr (data_nd, 5);
#if defined(CONFIG_MODULE_USB_DEVICE_MSC_SF)	
	set_nvram_addr (storage_st, 6);
	set_nvram_addr (storage_nd, 7);
#endif
}
