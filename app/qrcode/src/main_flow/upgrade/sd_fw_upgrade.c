#include <FreeRTOS.h>
#include <bsp.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include <nonstdlib.h>
#include <string.h>
#include "debug.h"

#include <libmid_nvram/snx_mid_nvram.h>
#include <libmid_fwupgrade/fwupgrade.h>
#include <libmid_automount/automount.h>
#include <generated/snx_sdk_conf.h>
#include "../main_flow.h"
#ifndef CONFIG_APP_DRONE
#endif

xTaskHandle task_automount_fwupgrade = NULL;

int sd_firmware_upgrade(void)
{       
    FILINFO finfo;  
    int     _max_lfn = 255;
    char    lfn[ _max_lfn + 1 ];    // Buffer to store the LFN  
    char *  path0 = "FIRMWARE_660R.bin";

    FIL  fir;
    int ret;
    unsigned int fir_size = 0;
    unsigned int fir_ver_size_offset = 0;
    unsigned int fir_ver_offset = 0;
    unsigned int br = 0;
    char *version = NULL;
    unsigned int version_size = 0;
    unsigned char *fir_image = NULL;

    unsigned int dev_ver_len = 0;
    char *dev_ver = NULL;

    uchar hash[16];
    MD5_CTX ctx;

    char enc_hash[16];
    int i;

    fwburning_info_t*   sfwb_info;


    finfo.lfname = lfn;
    finfo.lfsize = _max_lfn + 1;

    // wait for all task init
    vTaskDelay(6000/portTICK_PERIOD_MS);
    
    // check fir file 
    if (f_stat( path0, & finfo ) == FR_OK) {
        CSTREAMER_DEBUGF(CS_RECORD_DEBUG|CSTREAMER_DBG_TRACE,("FIRMWARE_660R.bin found\n"));
    }
    else{
        CSTREAMER_DEBUGF(CS_RECORD_DEBUG|CSTREAMER_DBG_TRACE,("FIRMWARE_660R.bin not found\n"));
        goto notexit;
    }


    // check fir version
    if((ret=f_open(&fir,path0,FA_OPEN_EXISTING|FA_READ))!=FR_OK)
    {
        CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("FIRMWARE_660R.bin open fail\n"));
        goto notexit;
    }

    fir_size = f_size(&fir);
    CSTREAMER_DEBUGF(CS_RECORD_DEBUG|CSTREAMER_DBG_TRACE,("FIRMWARE_660R.bin Size = %d \n",fir_size));

    // read version size
    fir_ver_size_offset = fir_size - 16 - 4;
    if(f_lseek(&fir, fir_ver_size_offset) != FR_OK){
       CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("FIRMWARE_660R.bin get version offset fail!\n"));
       goto ffail;
    }

    if ((f_read(&fir, &version_size, sizeof(unsigned int), &br))!=FR_OK) {
        CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("read version size failed!\n"));
        goto ffail;
    }

    CSTREAMER_DEBUGF(CS_RECORD_DEBUG|CSTREAMER_DBG_TRACE,("version Size = %x \n", version_size));

    // read version
    if(!(version = (char *) pvPortMalloc(version_size, GFP_KERNEL, MODULE_APP))){
          CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("pvPortMalloc failed!\n"));
          goto ffail;
    }

    // read version
    fir_ver_offset = fir_size - 16 - 4 - version_size;
    if(f_lseek(&fir, fir_ver_offset) != FR_OK){
       CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("FIRMWARE_660R.bin get version offset fail!\n"));
       goto out;
    }

    if ((f_read(&fir, version, version_size, &br))!=FR_OK) {
        CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("read version size failed!\n"));
        goto out;
    }

    CSTREAMER_DEBUGF(CS_RECORD_DEBUG|CSTREAMER_DBG_TRACE,("firmware_660r.bin version = %s \n", version));


    snx_nvram_get_data_len("SNX_NVRAM", "version", &dev_ver_len);
    dev_ver = (char *) pvPortMalloc(dev_ver_len, GFP_KERNEL, MODULE_APP);
    snx_nvram_string_get("SNX_NVRAM", "version", dev_ver);
    

    CSTREAMER_DEBUGF(CS_RECORD_DEBUG|CSTREAMER_DBG_TRACE,("dev ver size = %x \n", dev_ver_len));
    CSTREAMER_DEBUGF(CS_RECORD_DEBUG|CSTREAMER_DBG_TRACE,("dev version = %s \n", dev_ver));

    // compare version
    if ((version_size != (dev_ver_len -1)) || (strncmp(version,dev_ver,version_size)!=0)) {
        CSTREAMER_DEBUGF(CS_RECORD_DEBUG|CSTREAMER_DBG_TRACE,("version or size is not the same\n"));
    }
    else{
        CSTREAMER_DEBUGF(CS_RECORD_DEBUG|CSTREAMER_DBG_TRACE,("version and size is the same\n"));
        goto out;
    }


    // read firmware image
    if(!(fir_image = (unsigned char *) pvPortMalloc(fir_size, GFP_DMA, MODULE_APP))){
          CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("pvPortMalloc fir_image failed! fir_size = %d\n",fir_size));
          goto out;
    }

    if(f_lseek(&fir, 0) != FR_OK){
       CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("FIRMWARE_660R.bin get version offset fail!\n"));
       goto imgfail;
    }

    if ((f_read(&fir, fir_image, fir_size, &br))!=FR_OK) {
        CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("read fir_image failed!\n"));
        goto imgfail;
    }

    CSTREAMER_DEBUGF(CS_RECORD_DEBUG|CSTREAMER_DBG_TRACE,("fir_image addr=%x, fir_size= %x\n",fir_image,fir_size));

    // check platform
    if (check_platform_match(fir_image, fir_size) == 1) {
        CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("FIRMWARE_660R.bin PLATFORM MISMATCH\n"));
        goto imgfail;
    }

    // Calculate MD5 and endecrypt
    md5_init(&ctx); 
    md5_update(&ctx,fir_image,(fir_size-16)); 
    md5_final(&ctx,hash);

        CSTREAMER_DEBUGF(CS_RECORD_DEBUG|CSTREAMER_DBG_TRACE,("MD5###%x%x%x%x%x%x%x%x###\n",hash[0],hash[1],hash[2],hash[3],hash[4],hash[5],hash[6],hash[7]));
        CSTREAMER_DEBUGF(CS_RECORD_DEBUG|CSTREAMER_DBG_TRACE,("MD5###%x%x%x%x%x%x%x%x###\n",hash[8],hash[9],hash[10],hash[11],hash[12],hash[13],hash[14],hash[15]));

    snx_endecrypt((char *)hash, enc_hash);

    // compare MD5 endecrypt
    CSTREAMER_DEBUGF(CS_RECORD_DEBUG|CSTREAMER_DBG_TRACE,("ENCRY_MD5="));
    for (i=0;i<16;i++) {
        if (fir_image[fir_size - 16 + i] != enc_hash[i]) {
             CSTREAMER_DEBUGF(CS_RECORD_DEBUG|CSTREAMER_DBG_TRACE,("FIRMWARE_660R.BIN CHECK MD5 ERROR!!!\n"));
             goto imgfail;
        }
        CSTREAMER_DEBUGF(CS_RECORD_DEBUG|CSTREAMER_DBG_TRACE,("%x,",enc_hash[i]));
    }
    
        CSTREAMER_DEBUGF(CS_RECORD_DEBUG|CSTREAMER_DBG_TRACE,("\n"));
        CSTREAMER_DEBUGF(CS_RECORD_DEBUG|CSTREAMER_DBG_TRACE,("FIRMWARE_660R.BIN CHECK MD5 SUCCESS!!!\n"));

  //  all_task_uinit(TASK_KEEP_NO_KEEP);
#ifndef CONFIG_APP_DRONE
#endif
    if (check_bypass_mode(fir_image, fir_size) == 1) {
        print_msg("SD FIRMWARE_660R.BIN BYPASS MODE\n");
		snx_nvram_integer_set(NVRAM_MP_MODE_PKG_NAME, NVRAM_MP_MODE_CFG_ENABLE_NAME, NVRAM_MP_MODE_ENABLE);
        fw_upgrade(fir_image, 0xFFFFFFFF);
    }else {
        print_msg("SD FIRMWARE_660R.BIN UPGRADE MODE\n");
        fw_upgrade(fir_image, fir_size);
    }


    sfwb_info = get_fwburning_info();

    print_msg_queue("wait upgrading ...\n");
    if( xSemaphoreTake(sfwb_info->FwburningMutex, portMAX_DELAY ) == pdTRUE ) 
    {
        print_msg_queue("reboot !!!\n");
#ifndef CONFIG_APP_DRONE
#endif
    }

imgfail:
    vPortFree(fir_image);
out:
    vPortFree(dev_ver);
    vPortFree(version);  
ffail:
    f_close(&fir);
notexit:
    return 0;
}

void sd_fwupgrade_task (void *pvParameters) 
{
    automount_info_t*   sdaumt_info;

    sdaumt_info = get_automount_info();

    for( ;; ) 
    {
        if( xSemaphoreTake(sdaumt_info->SdFwupgradeMutex, portMAX_DELAY ) == pdTRUE ) 
        {
            print_msg_queue("sd_firmware_upgrade check...\n");
            sd_firmware_upgrade();
        }
    }
}

int sd_fwupgrade_init(void)
{   
    if (pdPASS != xTaskCreate(sd_fwupgrade_task, "AutoMount fw upgrade", STACK_SIZE_8K  , NULL,  80, &task_automount_fwupgrade))
    {
        print_msg_queue("Could not create Task1 task\r\n");
        return pdFAIL;
    }
    else
        return pdPASS;
    
}
