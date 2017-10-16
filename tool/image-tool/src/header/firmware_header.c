#include "common.h"
#include "generated/snx_sdk_conf.h"

u32 FLASH_INFO_START;	
u32 HW_SETTING_START;   
u32 U_BOOT_START;       
u32 U_ENV_START;        
u32 FLASH_LAYOUT_START; 
u32 FACTORY_START;      
u32 U_LOGO_START;       
u32 RESCUE_START;
u32 AHB2_CLOCK;

u32 IMAGE_TABLE_START;
u32 IMAGE_TABLE_SIZE;

u32 ONLY_BRINGUP_RTOS = 0;


#define ITCM_ZI 0xFFFF6000
#define SB_FWIMG_LOAD_ADDR 	(ITCM_ZI + 0x1c)
#define SB_FWIMG_SIZE 	0x00010000

#define SNX_SYS_BASE			0x98000000

#define UPDATE_STATUS_ADDR				0x3fffff0
/* cp done status */
#define	CP_DONE_FAIL					0x40000000
#define CP_DONE_OK					0x80000000
#define CP_DONE_BUSY					0x0


static int get_clock ()
{
	u32 pll800=0, ahb2_rate=0;

	pll800 = ((* (volatile unsigned int *) SNX_SYS_BASE) & 0x3f8000) >> 15;
	ahb2_rate = ((* (volatile unsigned int *) (SNX_SYS_BASE + 4)) & 0xf8000) >> 15;
		
	AHB2_CLOCK = (pll800 * 12 * 1000000) / ahb2_rate;
	
	return (0);
}

/*
  implement watchdog disable so the watchdog doesn't fire while we are
  writing a new firmware to serial flash
 */
#define BSP_WDT_BASE_ADDRESS		(0x98700000)
#define WDOG_EN_REG				(0xc)
#ifndef outl
#define outl(addr, value)       (*((volatile unsigned int *)(addr)) = value)
#endif

static void wdt_disable()
{
	outl((BSP_WDT_BASE_ADDRESS+WDOG_EN_REG), 0);
}

//int main(unsigned int addr)
int main(unsigned int addr, unsigned int size)
{
/* check id */
	u32 platform_id, firmware_size=0;


	platform_id = (* (volatile unsigned int *) (SNX_SYS_BASE + 0x10)) & 0xfffff;
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660) || defined(CONFIG_SYSTEM_PLATFORM_SN98661) || defined (CONFIG_SYSTEM_PLATFORM_SN98670) || defined (CONFIG_SYSTEM_PLATFORM_SN98671) || defined (CONFIG_SYSTEM_PLATFORM_SN98672) || defined (CONFIG_SYSTEM_PLATFORM_SN98293)
	if (platform_id && (platform_id != 0x58660)) {
		serial_printf ("ERROR: id is not match, hardware platform id = %x, but image id is 58660\n", platform_id);
		* (volatile unsigned int *) (UPDATE_STATUS_ADDR) = CP_DONE_FAIL;
		return -1;
	}

	if (size == 0xFFFFFFFF) {
		* (volatile unsigned int *) (0x98200004) = 0;
		* (volatile unsigned int *) (0x98200024) = 0;
		* (volatile unsigned int *) (0x9870000c) = 0;
		* (volatile unsigned int *) (0x98600030) = 0;

		u32 clock_reg;

		clock_reg = (* (volatile unsigned int *) (0x90600000));
		clock_reg &= ~(0x1 << 22); //IMG_TX_EN
		* (volatile unsigned int *) (0x90600000) = clock_reg;
		clock_reg = (* (volatile unsigned int *) (0x90600e00));
		clock_reg &= ~(0x1 << 0); //RAW_3DNR_EN
		* (volatile unsigned int *) (0x90600e00) = clock_reg;
		
		serial_printf ("======== FW Only Bring up RTOS =========== : %x\n", size);

		if (CONFIG_DDR_TOTAL_SIZE <= 16) {
			serial_printf ("======== Warning!!! Bring up RTOS STOP! STOP! ========\n");
			while(1);
		}

		if (addr < 0x1000000) {
			memcpy(0x1800000,addr,0x300000);
			addr = 0x1800000;
		}

		ONLY_BRINGUP_RTOS = 1;
	}
#else
	if (platform_id && (platform_id != 0x58600)) {
		serial_printf ("ERROR: id is not match, hardware platform id = %x, but image id is 58600\n", platform_id);
		* (volatile unsigned int *) (UPDATE_STATUS_ADDR) = CP_DONE_FAIL;
		return -1;
	}
#endif

	//IMAGE_TABLE_START = readl(SB_FWIMG_LOAD_ADDR) + SB_FWIMG_SIZE;
	//IMAGE_TABLE_SIZE = readl(IMAGE_TABLE_START);
#if 1
	IMAGE_TABLE_START = addr;
	IMAGE_TABLE_SIZE = *((u32*)IMAGE_TABLE_START);
#else
	IMAGE_TABLE_START = 0x2010000;
	IMAGE_TABLE_SIZE = *((u32*)IMAGE_TABLE_START);
#endif
	
	serial_puts("----DISABLE WDT----\n");
	wdt_disable();
        
	serial_puts("----UPDATE FIRMWARE----\n");
	serial_printf("IMAGE_TABLE_START = 0x%x\n", IMAGE_TABLE_START);
	//serial_printf("FIRMWARE_SIZE = 0x%x\n", firmware_size);

	get_clock();
	serial_init();

	if (ONLY_BRINGUP_RTOS == 0) {
		if(ms_serial_flash_init()) {
			* (volatile unsigned int *) (UPDATE_STATUS_ADDR) = CP_DONE_FAIL;
			return -1;
		}
	}
		
	if(ms_serial_flash_update(addr,size)) {
		* (volatile unsigned int *) (UPDATE_STATUS_ADDR) = CP_DONE_FAIL;
		serial_puts("serial flash update failed\n");
		return -1;
	}

	serial_puts("serial flash update success\n");
	* (volatile unsigned int *) (UPDATE_STATUS_ADDR) = CP_DONE_OK;
	return 0;
}
