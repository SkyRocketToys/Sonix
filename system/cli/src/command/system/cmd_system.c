#include <FreeRTOS.h>
#include <sys_clock.h>
#include <nonstdlib.h>


/** \defgroup cmd_system System commands
 *  \ingroup system_cli
 * @{
 */

#define KB_SIZE 1024
#define KB_SHIFT 10
#define MB_SHIFT 20

typedef struct ModuleNameMap
{
	size_t xModuleID;
	char xModuleName[15];
}ModuleNameMap;


ModuleNameMap DDRModMap[MODULE_DEF_MAX] = 
{
	{MODULE_UNKNOW, "(nonstd_c)"},
	{MODULE_KERNEL, "KERNEL"}, 
	{MODULE_CLI, "CLI"},
	{MODULE_TRACE, "TRACE"},
	{MODULE_DRI_MAC, "DRI_MAC"},
	{MODULE_DRI_VIDEO, "DRI_VIDEO"},
	{MODULE_DRI_UART, "DRI_UART"},
	{MODULE_DRI_I2C, "DRI_I2C"},
	{MODULE_DRI_SENSOR, "DRI_SENSOR"},
	{MODULE_DRI_ISP, "DRI_ISP"},
	{MODULE_DRI_AUDIO, "DRI_AUDIO"},
	{MODULE_DRI_SF, "DRI_SF"},
	{MODULE_DRI_SD, "DRI_SD"},
	{MODULE_DRI_SDIO, "DRI_SDIO"},
	{MODULE_DRI_GPIO, "DRI_GPIO"},
	{MODULE_DRI_PWM, "DRI_PWM"},
	{MODULE_DRI_RTC, "DRI_RTC"},
	{MODULE_DRI_SPI, "DRI_SPI"},	
	{MODULE_DRI_TIMER, "DRI_TIMER"},
	{MODULE_DRI_WATCHDOG, "DRI_WATCHDOG"},
	{MODULE_DRI_AHBDMA, "DRI_AHBDMA"},
	{MODULE_DRI_USBH, "DRI_USBH"},	
	{MODULE_DRI_USBD, "DRI_USBD"},
	{MODULE_DRI_WIFI, "DRI_WIFI"},	
	{MODULE_DRI_BOOTINFO, "DRI_BOOTINFO"},
	{MODULE_MID_VIDEO, "MID_VIDEO"},
	{MODULE_MID_AUDIO, "MID_AUDIO"},
	{MODULE_MID_SD, "MID_SD"},
	{MODULE_MID_SF, "MID_SF"},
	{MODULE_MID_FATFS, "MID_FATFS"},
	{MODULE_MID_RECORD, "MID_RECORD"},
	{MODULE_MID_NVRAM, "MID_NVRAM"},
	{MODULE_MID_LWIP, "MID_LWIP"},
	{MODULE_MID_RTSP, "MID_RTSP"},
	{MODULE_MID_CYASSL, "MID_CYASSL"},
	{MODULE_MID_LIB_CLOUD, "MID_LIB_CLOUD"},
	{MODULE_MID_JSON, "MID_JSON"},	
	{MODULE_MID_TD, "MID_TD"},
	{MODULE_MID_WEBSOCKETS, "MID_WEBSOCKETS"},
	{MODULE_MID_USBD, "MID_USBD"},
	{MODULE_MID_ZBAR, "MID_ZBAR"},
	{MODULE_MID_FWUPGRADE, "MID_FWUPGRADE"},
	{MODULE_MID_AUTOMOUNT, "MID_AUTOMOUNT"},
	{MODULE_MID_ZLIB, "MID_ZLIB"},
	{MODULE_APP, "APP"},


};





int cmd_system_date(int argc, char* argv[])
{
	system_date_t date;

	if (argc == 1) {
		get_date(&date);

		print_msg_queue("%d/%d/%d - %d:%d:%d", date.year, date.month, date.day,
												date.hour, date.minute, date.second);

		switch (date.week) {
		case 1:
			print_msg_queue(" Mon\n");
			break;
		case 2:
			print_msg_queue(" Tue\n");
			break;
		case 3:
			print_msg_queue(" Wed\n");
			break;
		case 4:
			print_msg_queue(" Thu\n");
			break;
		case 5:
			print_msg_queue(" Fri\n");
			break;
		case 6:
			print_msg_queue(" Sat\n");
			break;
		case 7:
			print_msg_queue(" Sun\n");
			break;
		}
	} else if (argc == 7) {
		date.year = simple_strtoul(argv[1], 0, 10);
		date.month = simple_strtoul(argv[2], 0, 10);
		date.day = simple_strtoul(argv[3], 0, 10);
		date.hour = simple_strtoul(argv[4], 0, 10);
		date.minute = simple_strtoul(argv[5], 0, 10);
		date.second = simple_strtoul(argv[6], 0, 10);

		set_date(&date, 0);

	} else {
		print_msg_queue("Argument error!!!\n");
	}

	return 0;
}


/**
* @brief Display system free memory
* @param None
* @details Example: free
*/
int cmd_system_free(int argc, char* argv[])
{
	int i, id;
	size_t *UsedSizeCB = xPortGetHeapUsedSizeByModule(GFP_KERNEL);
	size_t *UsedSizeNCNB = xPortGetHeapUsedSizeByModule(GFP_DMA);
	size_t *UsedSizePreCB = xPortGetHeapUsedSizeByModule(GFP_KERNEL|GFP_PREALLOCATE);
	size_t *UsedSizePreNCNB = xPortGetHeapUsedSizeByModule(GFP_DMA|GFP_PREALLOCATE);

	size_t *MaxUsedSizeCB = xPortGetMaximumHeapUsedSizeByModule(GFP_KERNEL);
	size_t *MaxUsedSizeNCNB = xPortGetMaximumHeapUsedSizeByModule(GFP_DMA);
	size_t *MaxUsedSizePreCB = xPortGetMaximumHeapUsedSizeByModule(GFP_KERNEL|GFP_PREALLOCATE);
	size_t *MaxUsedSizePreNCNB = xPortGetMaximumHeapUsedSizeByModule(GFP_DMA|GFP_PREALLOCATE);
	size_t total_cb = 0, total_ncnb =0, total_pre_cb = 0, total_pre_ncnb =0;
	
	int cb_size = xPortGetTotalHeapSize(GFP_KERNEL);
	int cb_free_size = xPortGetFreeHeapSize(GFP_KERNEL);

	int ncb_size = xPortGetTotalHeapSize(GFP_DMA);
	int ncb_free_size = xPortGetFreeHeapSize(GFP_DMA);

	int pre_cb_size = xPortGetTotalHeapSize(GFP_KERNEL|GFP_PREALLOCATE);
	int pre_cb_free_size = xPortGetFreeHeapSize(GFP_KERNEL|GFP_PREALLOCATE);

	int pre_ncb_size = xPortGetTotalHeapSize(GFP_DMA|GFP_PREALLOCATE);
	int pre_ncb_free_size = xPortGetFreeHeapSize(GFP_DMA|GFP_PREALLOCATE);
	int value;
	
	print_msg_queue("start addr = %d\n", xPortGetHeapStart());
	print_msg_queue("\tTotal\t\tUsed\tFree\n");
	print_msg_queue("------------------------------------------------\n");
	print_msg_queue("CB\t%5d KB\t%5d KB\t%5d KB\n"
			, cb_size >> KB_SHIFT
			, (cb_size - cb_free_size) >> KB_SHIFT
			, cb_free_size >> KB_SHIFT);
	print_msg_queue("NCNB\t%5d KB\t%5d KB\t%5d KB\n"
			, ncb_size >> KB_SHIFT
			, (ncb_size - ncb_free_size) >> KB_SHIFT
			, ncb_free_size >> KB_SHIFT);

	print_msg_queue("PreCB\t%5d KB\t%5d KB\t%5d KB\n"
			, pre_cb_size >> KB_SHIFT
			, (pre_cb_size - pre_cb_free_size) >> KB_SHIFT
			, pre_cb_free_size >> KB_SHIFT);

	print_msg_queue("PreNCNB\t%5d KB\t%5d KB\t%5d KB\n"
			, pre_ncb_size >> KB_SHIFT
			, (pre_ncb_size - pre_ncb_free_size) >> KB_SHIFT
			, pre_ncb_free_size >> KB_SHIFT);

	print_msg_queue("------------------------------------------------\n");
/*
	print_msg_queue("\n\tMax usage\n");
	print_msg_queue("------------------------------------------------\n");
	print_msg_queue("CB\t%d Mbytes\n", (cb_size - xPortGetMinimumEverFreeHeapSize(GFP_KERNEL))>> MB_SHIFT);
	print_msg_queue("NCNB\t%d Mbytes\n", (ncb_size - xPortGetMinimumEverFreeHeapSize(GFP_DMA))>> MB_SHIFT);
	print_msg_queue("------------------------------------------------\n");
*/
	if(argc < 2){
		print_msg_queue(" Usage: free [value]\n");
		print_msg_queue(" value:\n");
		print_msg_queue("      1: Show used size \n");
		print_msg_queue("      2: Show max ever used size\n");
		print_msg_queue("      3: Show all size info\n");
	} else {
		value = simple_strtoul(argv[1], NULL, 10);

		if(value&0x1) {
			total_cb = 0; total_ncnb = 0; total_pre_cb = 0; total_pre_ncnb = 0;
			
			print_msg_queue("\nModule\t\t     cb      ncnb   pre_cb    pre_ncnb  <used size>\n");
			print_msg_queue("------------------------------------------------\n");

			for(i = 0; i<MODULE_DEF_MAX;i++)
			{
				id = DDRModMap[i].xModuleID;
				total_cb += UsedSizeCB[id];
				total_ncnb += UsedSizeNCNB[id];
				total_pre_cb += UsedSizePreCB[id];
				total_pre_ncnb += UsedSizePreNCNB[id];
				if((UsedSizeCB[id]+UsedSizeNCNB[id]
					+UsedSizePreCB[id]+UsedSizePreNCNB[id])>0) {
					print_msg_queue("%-15s %5d %s %5d %s %5d %s %5d %s\n"
								, DDRModMap[i].xModuleName
								, UsedSizeCB[id]>>(UsedSizeCB[id]>KB_SIZE?KB_SHIFT:0) 
								, (UsedSizeCB[id]>KB_SIZE?"KB":" B")
								, UsedSizeNCNB[id]>>(UsedSizeNCNB[id]>KB_SIZE?KB_SHIFT:0) 
								, (UsedSizeNCNB[id]>KB_SIZE?"KB":" B")
								, UsedSizePreCB[id]>>(UsedSizePreCB[id]>KB_SIZE?KB_SHIFT:0) 
								, (UsedSizePreCB[id]>KB_SIZE?"KB":" B")
								, UsedSizePreNCNB[id]>>(UsedSizePreNCNB[id]>KB_SIZE?KB_SHIFT:0) 
								, (UsedSizePreNCNB[id]>KB_SIZE?"KB":" B")
								
								);
				}
			}

		} // if(value&0x1)
			print_msg_queue("------------------------------------------------\n");
		print_msg_queue("total\t\t%5d MB %5d MB %5d MB %5d MB\n"
						, total_cb >> MB_SHIFT, total_ncnb >> MB_SHIFT
						, total_pre_cb >> MB_SHIFT, total_pre_ncnb >> MB_SHIFT);

		//	max used size
		if(value&0x2) {
			total_cb = 0; total_ncnb = 0; total_pre_cb = 0; total_pre_ncnb = 0;


			print_msg_queue("\nModule\t\t     cb      ncnb   pre_cb    pre_ncnb  <used size>\n");
			print_msg_queue("------------------------------------------------\n");

			for(i = 0; i<MODULE_DEF_MAX;i++) {
				id = DDRModMap[i].xModuleID;
				total_cb += MaxUsedSizeCB[id];
				total_ncnb += MaxUsedSizeNCNB[id];
				total_pre_cb += MaxUsedSizePreCB[id];
				total_pre_ncnb += MaxUsedSizePreNCNB[id];
				
				if((UsedSizeCB[id]+UsedSizeNCNB[id])>0) {
					print_msg_queue("%-15s %5d %s %5d %s %5d %s %5d %s\n"
								, DDRModMap[i].xModuleName
								, MaxUsedSizeCB[id]>>(MaxUsedSizeCB[id]>KB_SIZE?KB_SHIFT:0) 
								, (MaxUsedSizeCB[id]>KB_SIZE?"KB":" B")
								, MaxUsedSizeNCNB[id]>>(MaxUsedSizeNCNB[id]>KB_SIZE?KB_SHIFT:0) 
								, (MaxUsedSizeNCNB[id]>KB_SIZE?"KB":" B")
								, MaxUsedSizePreCB[id]>>(MaxUsedSizePreCB[id]>KB_SIZE?KB_SHIFT:0) 
								, (MaxUsedSizePreCB[id]>KB_SIZE?"KB":" B")
								, MaxUsedSizePreNCNB[id]>>(MaxUsedSizePreNCNB[id]>KB_SIZE?KB_SHIFT:0) 
								, (MaxUsedSizePreNCNB[id]>KB_SIZE?"KB":" B")

								);
				}
			}
			print_msg_queue("------------------------------------------------\n");
			print_msg_queue("total\t\t%5d MB %5d MB %5d MB %5d MB\n"
							, total_cb >> MB_SHIFT, total_ncnb >> MB_SHIFT
							, total_pre_cb >> MB_SHIFT, total_pre_ncnb >> MB_SHIFT);
		} //if(value&0x2)
	} // if(argc < 2)

	
	return 0;
}

int cmd_system_reboot(int argc, char* argv[])
{

	print_msg_queue("Reboot\n");
	wdt_setload(1);
	wdt_enable();
	wdt_reset_enable(1);

	return 0;
}

int cmd_system_phymem_rw(int argc, char* argv[])
{
	int i;
	unsigned int addr = 0, value = 0;

	addr = simple_strtoul(argv[1], NULL, 10) & (~3);

	// READ: phymem_rw address
	if (argc == 2) {
		for (i=0; i<8; i+=1) {
			print_msg_queue("0x%08X : %08X  %08X  %08X  %08X\n", addr + i*4*4,
				*((unsigned int*)(addr + i*4*4)),
				*((unsigned int*)(addr + (i*4 + 1)*4)),
				*((unsigned int*)(addr + (i*4 + 2)*4)),
				*((unsigned int*)(addr + (i*4 + 3)*4)));
		}
		return 0;
	}

	// WRITE: phymem_rw address value
	if (argc == 3) {
		value = simple_strtoul(argv[2], NULL, 10);
		print_msg_queue("old value:%08X : %08X\n", addr, *((unsigned int*)addr));
		*(unsigned int*)addr = value;
		print_msg_queue("new value:%08X : %08X\n", addr, *((unsigned int*)addr));
		return 0;
	}
	return 0;
}

static char *get_debug_level_string(int level)
{
	switch (level)
	{
		case SYS_ERR:
			return ("ERROR(0)");
		case SYS_WARN:
			return ("WARN(1)");
		case SYS_INFO:
			return ("INFO(2)");
		case SYS_DBG:
			return ("DBG(3)");
		default:
			return ("Undefined");
	}
}

int cmd_system_dbg_cfg(int argc, char* argv[])
{
	if (argc < 2 || argc > 3) {
		goto error;
	}
	
	if (!(strncasecmp(argv[1], "get", 3))) {
		int level = get_system_dbg_level();
		print_msg_queue("Current system debug level (%s)\n", get_debug_level_string(level));
	}
	else if (!(strncasecmp(argv[1], "set", 3))) {
		int level;
		if (argc < 3) {
			goto error;
		}
		level = atoi (argv[2]);
		if (set_system_dbg_level(level) == -1) {
			print_msg_queue("Out of level range (%d)\n", level);
			goto error;
		}
		print_msg_queue("Configure system debug level (%s)\n", get_debug_level_string(level));
		
	}
	else {
		/* undefined reference */
		goto error;	
	}

	return 0;
	
error:
	print_msg_queue("dbg_cfg [operator] [paramter]\n");
	print_msg_queue("[operator]: set / get\n");
	print_msg_queue("[paramter]: if operator is \"set\", then paramter is required.\n");
	print_msg_queue("[paramter]: ERROR(0), WARN(1), INFO(2), DBG(3)\n");
	return 0;
}
/** @} */
