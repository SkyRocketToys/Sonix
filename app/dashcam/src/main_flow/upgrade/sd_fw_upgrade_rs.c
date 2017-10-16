#include <FreeRTOS.h>
#include <bsp.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include <nonstdlib.h>
#include <string.h>
#include "debug.h"
#include "rec_common.h"
#include "rec_fatfs.h"

#include <libmid_nvram/snx_mid_nvram.h>
#include <libmid_fwupgrade/fwupgrade.h>
#include <libmid_automount/automount.h>
#include <generated/snx_sdk_conf.h>
#include "../main_flow.h"
#ifndef CONFIG_APP_DRONE
#include "../mcu_v2/mcu.h"
#endif

#define FW_PRINT(level, fmt, args...) print_q(level, "[fwupgr]%s: "fmt, __func__,##args)


xTaskHandle task_automount_fwupgrade = NULL;

int sd_firmware_upgrade(void)
{
	FILINFO finfo;
	int     _max_lfn = 255;
	char    lfn[ _max_lfn + 1 ];    // Buffer to store the LFN
	char   *path0 = "FIRMWARE_660R.bin";
	FIL  fir;
	int ret;
	unsigned int fir_size = 0;
	unsigned int br = 0;
	unsigned char *fir_image = NULL;
	fwburning_info_t   *sfwb_info;
#if (CONFIG_USBH_STORAGE_SUPPORT==1)
	unsigned int PtrOffset=0;
	unsigned char *Tmpfirimage = NULL;
#endif
	finfo.lfname = lfn;
	finfo.lfsize = _max_lfn + 1;

	// check fir file
	if (f_stat( path0, & finfo ) == FR_OK) {
		FW_PRINT(SYS_INFO, "FIRMWARE_660R.bin found\n");
	} else {
		FW_PRINT(SYS_DBG, "FIRMWARE_660R.bin not found\n");
		goto notexit;
	}

	if ((ret = f_open(&fir, path0, FA_OPEN_EXISTING | FA_READ)) != FR_OK) {
		FW_PRINT(SYS_ERR, "FIRMWARE_660R.bin open fail\n");
		goto notexit;
	}

	fir_size = f_size(&fir);
	FW_PRINT(SYS_DBG, "FIRMWARE_660R.bin Size = %d \n", fir_size);

	// read firmware image
#if (CONFIG_USBH_STORAGE_SUPPORT==1)
	PtrOffset = 512 - (fir.fptr%512);
	Tmpfirimage = (unsigned char *)pvPortMalloc((fir_size + 512 + 512), GFP_DMA, MODULE_APP);
	fir_image = (unsigned char *)((((unsigned int)Tmpfirimage + 512) & (0xFFFFFE00)) + (512 - PtrOffset));
	if(!fir_image){
		FW_PRINT(SYS_ERR, "pvPortMalloc fir_image failed (size = %d)!\n", fir_size);
		goto out;
	}
#else
	if (!(fir_image = (unsigned char *) pvPortMalloc(fir_size, GFP_DMA, MODULE_APP))) {
		FW_PRINT(SYS_ERR, "pvPortMalloc fir_image failed (size = %d)!\n", fir_size);
		goto out;
	}
#endif
	
	if ((f_read(&fir, fir_image, fir_size, &br)) != FR_OK) {
		FW_PRINT(SYS_ERR, "read fir_image failed!\n");
		goto imgfail;
	}

// 	if (check_bypass_mode(fir_image, fir_size) == 1) {
// 		FW_PRINT(SYS_INFO, "SD FIRMWARE_660R.BIN BYPASS MODE\n");
// 		snx_nvram_integer_set(NVRAM_MP_MODE_PKG_NAME, NVRAM_MP_MODE_CFG_ENABLE_NAME, NVRAM_MP_MODE_ENABLE);
// 		fir_size = 0xFFFFFFFF;
// 	} else {
// 		if (fw_upgrade_precheck(fir_image, fir_size) != 0)
// //			goto imgfail;
// 		FW_PRINT(SYS_INFO, "SD FIRMWARE_660R.BIN UPGRADE MODE\n");
// 	}


	// all_task_uinit(TASK_KEEP_NO_KEEP);
#ifndef CONFIG_APP_DRONE
	// mcu_set_err_flag(FIRMWARE_UPGRADE);
#endif

	fw_upgrade(fir_image, fir_size);

	sfwb_info = get_fwburning_info();

	FW_PRINT(SYS_DBG, "wait upgrading ...\n");
	if ( xSemaphoreTake(sfwb_info->FwburningMutex, portMAX_DELAY ) == pdTRUE ) {
		FW_PRINT(SYS_INFO, "reboot !!!\n");
// #ifndef CONFIG_APP_DRONE
// 		mcu_clear_err_flag(FIRMWARE_UPGRADE);
// 		mcu_set_err_flag(ALL_RESET);
// #endif
	}

imgfail:
#if (CONFIG_USBH_STORAGE_SUPPORT==1)
	if (Tmpfirimage) {
		vPortFree(Tmpfirimage);
		Tmpfirimage = NULL;
	}
#else
	if (fir_image) {
		vPortFree(fir_image);
		fir_image = NULL;
	}
#endif
	
out:
	f_close(&fir);
notexit:
	return 0;
}

void sd_fwupgrade_task (void *pvParameters)
{
	automount_info_t   *sdaumt_info;

	sdaumt_info = get_automount_info();

	for ( ;; ) {
		if ( xSemaphoreTake(sdaumt_info->SdFwupgradeMutex, portMAX_DELAY ) == pdTRUE ) {
			FW_PRINT(SYS_INFO, "sd_firmware_upgrade check...\n");
			sd_firmware_upgrade();
		}
	}
}

int sd_fwupgrade_init(void)
{
	if (pdPASS != xTaskCreate(sd_fwupgrade_task, "AutoMount fw upgrade", STACK_SIZE_8K  , NULL,  80, &task_automount_fwupgrade)) {
		FW_PRINT(SYS_ERR, "Could not create Task1 task\r\n");
		return pdFAIL;
	} else
		return pdPASS;

}
