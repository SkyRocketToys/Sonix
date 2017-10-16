/**
 * @file
 * this is application file for snapshot
 * @author Algorithm Dept Sonix.
 */
///must modify function snapshot and thumbnail to one function
#include <FreeRTOS.h>
#include <task.h>
#include <bsp.h>
#include <timers.h>
#include <nonstdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <queue.h>
#include <semphr.h>
#include <sys/time.h>
#include <libmid_fatfs/ff.h>
#include <libmid_automount/automount.h>
#include "video_main.h"
#include "rec_common.h"
#include "snapshot.h"
#include "upload1.h"
#include "rec_seed.h"
#include <generated/snx_sdk_conf.h>
#include <libmid_nvram/snx_mid_nvram.h>

#define PIC_PRINT(level, fmt, args...) print_q(level, "[snapshot]%s: "fmt, __func__,##args)


#define mutex_lock(xMutex) xSemaphoreTake( xMutex, portMAX_DELAY )
#define mutex_unlock(xMutex) xSemaphoreGive( xMutex )

static void task_save_pic( void *pvParameters );
static void task_save_thumbnail( void *pvParameters );

//static xSemaphoreHandle thumbnail_takepic_mtx;
/**
* @brief global variable for snapshot infomation
*/
static snapshot_info_t snapshot_info;
static snapshot_info_t thumbnail_info;

/**
* @brief global variable for snapshot infomation
*/
static int snapshot_start = 0;			/**<  task of snapshot is start or not*/
static int snapshot_ongoing = 0;		/**<  record picture is on processing or not */
static int snapshot_is_full = 0;		/**<  space for saving picture is full or not*/
static int thumbnail_start = 0;		/**<  task of snapshot is start or not*/
static int thumbnail_ongoing = 0;		/**<  record picture is on processing or not */
//static int thumbnail_is_full = 0;		/**<  space for saving picture is full or not*/

long long snapshot_free_space = 0;		/**<  remaind space for picture(byte unit)*/
long long thumbnail_free_space = 0;		/**<  remaind space for picture(byte unit)*/

//static xTimerHandle SmalSnapshoTimer = NULL;

static int set_snapshot_count(snapshot_info_t *shapshot_info)
{
	FIL  file;
	FRESULT ret;
	char filelistpath[LEN_FILENAME];
	int rval = 0;
	
	uint32_t bufsize = 0, uiWritedSize;
	memset(filelistpath, 0x00, sizeof(filelistpath));
	snprintf(filelistpath, sizeof(filelistpath), "%s/%s", shapshot_info->snapshotpath, RECORD_INDEX);
	if ((ret = f_open(&file, filelistpath, FA_CREATE_ALWAYS | FA_WRITE)) != FR_OK) {
		PIC_PRINT(SYS_ERR, "file open fail(ret=%d)\n", ret);
		return -1;
	}
	PIC_PRINT(SYS_DBG, "write record_count=%d\n", shapshot_info->snapshot_count);
	if (((ret = f_write(&file, &shapshot_info->snapshot_count, sizeof(unsigned int), &uiWritedSize)) != FR_OK) || ((sizeof(unsigned int)) != uiWritedSize)) {
		PIC_PRINT(SYS_ERR, "write data fail(ret = %d), %x, %x\n", ret, bufsize, uiWritedSize);
	}
	f_close(&file);

	rval = f_chmod(filelistpath,  AM_HID, AM_HID | AM_ARC);
	if (rval)
		PIC_PRINT(SYS_ERR, "hide %s file fail(%d)\n", filelistpath, rval);

	return pdPASS;
}

static int get_snapshot_count(snapshot_info_t *shapshot_info)
{
	FIL  file;
	FRESULT ret;
	char filelistpath[LEN_FILENAME];
	uint32_t bufsize = 0, uiReaddSize;

	memset(filelistpath, 0x00, sizeof(filelistpath));
	snprintf(filelistpath, sizeof(filelistpath), "%s/%s", shapshot_info->snapshotpath, RECORD_INDEX);
	if (exists(filelistpath) == 1) {
		if ((ret = f_open(&file, filelistpath, FA_OPEN_EXISTING | FA_READ)) != FR_OK) {
			PIC_PRINT(SYS_ERR, "file %s open fail(ret=%d)\n", file, ret);
			return (-1);
		}
		if (((ret = f_read(&file, &shapshot_info->snapshot_count, sizeof(unsigned int), &uiReaddSize)) != FR_OK) || ((sizeof(unsigned int)) != uiReaddSize)) {
			PIC_PRINT(SYS_ERR, "read data fail(ret = %d), %x, %x\n", ret, bufsize, uiReaddSize);
		}
		f_close(&file);
	} else
		PIC_PRINT(SYS_ERR, "filelistpath is not exit\n");

	PIC_PRINT(SYS_DBG, "read read read record_count=%d\n", shapshot_info->snapshot_count);

	return pdPASS;
}

int get_filepath(char *szFilePath, int len)
{
	system_date_t date;
	char cur_path[LEN_FILENAME] = {0};
	static char pre_path[LEN_FILENAME] = {0};
	static int pic_index = 1;
	int ret = 0;
	struct usr_config *pUserCfg = NULL;

	if ((ret = get_usr_config(&pUserCfg)) != 0) {
		PIC_PRINT(SYS_ERR, "get usr config failed\n");
		return pdFAIL;
	}

	if (szFilePath == NULL) {
		PIC_PRINT(SYS_ERR, "pointer to file path is null\n");
		return pdFAIL;
	}
	
	memset(szFilePath, 0, len);
	if (snapshot_info.fileformat == RECORD_TIMEFORMAT) {
		get_date(&date);
		snprintf(cur_path, len, "%s/%04d_%02d_%02d_%02d_%02d_%02d.jpg", pUserCfg->pic_path, date.year, date.month, date.day, date.hour, date.minute, date.second);
		if (strncmp(pre_path, cur_path, SNAPSHOT_PATH_LEN) != 0)
			pic_index = 1;

		snprintf(szFilePath, len, "%s/%04d_%02d_%02d_%02d_%02d_%02d-%02d.jpg", pUserCfg->pic_path, date.year, date.month, date.day, date.hour, date.minute, date.second, pic_index);
		strcpy(pre_path, cur_path);
		pic_index++;
	} else if (snapshot_info.fileformat == RECORD_COUNTFORMAT) {
		if (snapshot_info.snapshot_count >= 10000) {
			snapshot_info.snapshot_count = 0;
			update_seed();
		}
		snprintf(szFilePath, len, "%s/%s%04d.jpg", pUserCfg->pic_path, pUserCfg->pic_prefix, snapshot_info.snapshot_count++);
		set_snapshot_count(&snapshot_info);
	}

	return pdPASS;
}

/**
 * @brief interface function - initialization for shapshot
 * @return return pdPASS if success
 */
int mf_snapshot_init(void)
{
	int intbuf;
	int ret = 0;
	struct usr_config *pUserCfg = NULL;

	if ((ret = get_usr_config(&pUserCfg)) != 0) {
		PIC_PRINT(SYS_ERR, "get usr config failed\n");
		goto fail1;
	}
	
	snx_nvram_integer_get(NVRAM_SPACE, NVRAM_PICTURE_UPBD, &intbuf);
	if (intbuf <= 0)
		return pdPASS;
	
	snapshot_info.queue_pic = xQueueCreate(SHAPSHOT_MAX_QUEUE_ITEM, sizeof(pic_info_t));
	if (NULL == snapshot_info.queue_pic) {
		PIC_PRINT(SYS_ERR, "queue queue_pic create fail\n");
		goto fail1;
	}
	
	//register call back functio to get pic
	mf_video_set_snapshot_cb(mf_snapshot_get_pic);
	snapshot_info.fileformat = rec_fileformat_get();
	snapshot_info.snapshot_count = 0;
	memset(snapshot_info.snapshotpath, 0x00, sizeof(snapshot_info.snapshotpath));
	snprintf(snapshot_info.snapshotpath, sizeof(snapshot_info.snapshotpath), "%s", pUserCfg->pic_path);
	if (snapshot_info.fileformat == RECORD_COUNTFORMAT)
		get_snapshot_count(&snapshot_info);
//	PIC_PRINT(SYS_DBG, "snapshot_info.snapshot_count==%d\n", snapshot_info.snapshot_count);

#if defined(CONFIG_SYSTEM_PLATFORM_SN98660) || defined(CONFIG_SYSTEM_PLATFORM_SN98661) || defined(CONFIG_SYSTEM_PLATFORM_SN98671) || defined (CONFIG_SYSTEM_PLATFORM_SN98293)
//	rec_filenode_init(T_SNAPSHOT, FALSE);
	rec_filenode_update(T_SNAPSHOT);
#endif

	if (pdPASS != xTaskCreate(task_save_pic, "snapshot", STACK_SIZE_4K, NULL,
	                          PRIORITY_TASK_APP_TEST03, &snapshot_info.task_save_pic)) {
		PIC_PRINT(SYS_ERR, "Could not create task snapshot\n");
		goto fail2;
	}
	snapshot_start = 1;
	return pdPASS;

fail2:
	mf_video_set_snapshot_cb(NULL);
	vQueueDelete(snapshot_info.queue_pic);
fail1:
	return pdFAIL;
}

/**
 * @brief interface function - enable thumbnail
 */

void stop_thumbnail_access_sdcard()
{
	//PIC_PRINT(SYS_DBG, "StopThumbnailAccessSDCard\n");
	//xTimerStop( SmalSnapshoTimer, portMAX_DELAY );
}

/**
 * @brief interface function - disable thumbnail
 */

void start_thumbnail_access_sdcard()
{
	//PIC_PRINT(SYS_DBG, "StartThumbnailAccessSDCard\n");
	//xTimerStart( SmalSnapshoTimer, portMAX_DELAY );
}


/**
 * @brief interface function - initialization for shapshot
 * @return return pdPASS if success
 */
int mf_thumbnail_init(void)
{
	int intbuf;
	
	snx_nvram_integer_get(NVRAM_SPACE, NVRAM_THUMBNAIL_UPBD, &intbuf);
	if (intbuf <= 0)
		return pdPASS;

	thumbnail_info.queue_pic = xQueueCreate(SHAPSHOT_MAX_QUEUE_ITEM, sizeof(pic_info_t));

	if (NULL == thumbnail_info.queue_pic) {
		PIC_PRINT(SYS_ERR, "small queue queue_pic create fail\n");
		goto fail1;
	}

	/* set cb function for video jpeg */
	mf_video_set_thumbnail_cb(mf_thumbnail_get_pic);
	if (pdPASS != xTaskCreate(task_save_thumbnail, "thumbnail", STACK_SIZE_4K, NULL,
	                          PRIORITY_TASK_APP_TEST03, &thumbnail_info.task_save_pic)) {
		PIC_PRINT(SYS_ERR, "Could not create task snapshot\n");
		goto fail2;
	}
	
	thumbnail_start = 1;
	return pdPASS;

fail2:
	mf_video_set_thumbnail_cb(NULL);
	vQueueDelete(thumbnail_info.queue_pic);
fail1:
	return pdFAIL;
}

/**
 * @brief interface function - Uninitialization for shapshot
 */
int mf_snapshot_uninit(void)
{
	int intbuf;
	
	snx_nvram_integer_get(NVRAM_SPACE, NVRAM_PICTURE_UPBD, &intbuf);
	if (intbuf <= 0)
		return pdPASS;
	
	snapshot_start = 0;
	mf_video_set_snapshot_cb(NULL);
	vTaskDelete(snapshot_info.task_save_pic);
	vQueueDelete(snapshot_info.queue_pic);
#if defined(CONFIG_SYSTEM_PLATFORM_SN98660) || defined(CONFIG_SYSTEM_PLATFORM_SN98661) || defined(CONFIG_SYSTEM_PLATFORM_SN98671) || defined (CONFIG_SYSTEM_PLATFORM_SN98293)
	rec_filelist_clear(T_SNAPSHOT);
#endif

	return pdPASS;
}

/**
 * @brief interface function - Uninitialization for shapshot
 */
int mf_thumbnail_uninit(void)
{
	int intbuf;
	
	snx_nvram_integer_get(NVRAM_SPACE, NVRAM_THUMBNAIL_UPBD, &intbuf);
	if (intbuf <= 0)
		return pdPASS;
	
	thumbnail_start = 0;
	mf_video_set_thumbnail_cb(NULL);
	vTaskDelete(thumbnail_info.task_save_pic);
	vQueueDelete(thumbnail_info.queue_pic);

	return pdPASS;
}


/**
* @brief interface function - snapshot list buffer to file
* @param path : picture file folder
*
*/
#if defined(CONFIG_SYSTEM_PLATFORM_SN98660) || defined(CONFIG_SYSTEM_PLATFORM_SN98661) || defined(CONFIG_SYSTEM_PLATFORM_SN98671) || defined (CONFIG_SYSTEM_PLATFORM_SN98293)
int piclist_to_file(char *path)
{
	char target[64] = {0};
	int len = 0;
	int ret = 0;
	struct usr_config *pUserCfg = NULL;

	if ((ret = get_usr_config(&pUserCfg)) != 0) {
		PIC_PRINT(SYS_ERR, "get usr config failed\n");
		return LIST_FILE_CONFIG_ERR;
	}
	
	memset(target, 0x00, sizeof(target));
	snprintf(target, sizeof(target), "%s/%s", pUserCfg->pic_path, SD_PICTURE_FILELIST);
	len = strlen(target);
	strncpy(path, target, len);
	if (is_current_upload_file(target, UPLOAD_FG) != 0) {
		PIC_PRINT(SYS_ERR, "picture file txt is downloading \n");
		strncpy(path, target, strlen(target));
		return LIST_FILE_DOWNLOAD;
	}
	return rec_filelist_Createfile(T_SNAPSHOT, target, len);
}
#endif

static void task_save_pic(void *pvParameters)
{
	pic_info_t pic_info;
	FIL pic_file;
	int ret;
	unsigned int uiWritedSize;
	int first_insert = 0;
	BaseType_t xStatus;

	while (1) {
		xStatus = xQueueReceive(snapshot_info.queue_pic, &pic_info, 500 / portTICK_RATE_MS);
		if (pdPASS == xStatus) {

			if ((get_sd_umount_err() == 0) && (first_insert == 0)) {
				first_insert = 1;
#if defined(CONFIG_SYSTEM_PLATFORM_SN98660) || defined(CONFIG_SYSTEM_PLATFORM_SN98661) || defined(CONFIG_SYSTEM_PLATFORM_SN98671) || defined (CONFIG_SYSTEM_PLATFORM_SN98293)
				rec_filenode_update(T_SNAPSHOT);
				rec_import_files(T_SNAPSHOT);
				if (snapshot_info.fileformat == RECORD_COUNTFORMAT)
					get_snapshot_count(&snapshot_info);
#endif
			}
			if ((snapshot_start == 1) && (get_sd_umount_err() == 0) && (first_insert == 1)) {
				if ((snapshot_free_space >= SD_PICTURE_RESERVED) &&  (snapshot_free_space >= (long long)pic_info.size) && !snapshot_is_full) {
					snapshot_ongoing = 1;
					//get_filepath(pic_path, sizeof(pic_path));
					PIC_PRINT(SYS_DBG, "save pic path = %s\n", pic_info.picname);
					if ((ret = f_open(&pic_file, pic_info.picname, FA_WRITE | FA_CREATE_ALWAYS)) != FR_OK) {
						PIC_PRINT(SYS_ERR, "open file fail(ret = %d)\n", ret);
						snapshot_ongoing = 0;
						if(pic_info.dev==REC_DEV_USBH)
							vPortFree(pic_info.OriginAddr);
						else
							vPortFree(pic_info.addr);

						vPortFree(pic_info.picname);
						continue;
					}

					if ((ret = f_write(&pic_file, pic_info.addr, pic_info.size, &uiWritedSize)) != FR_OK) {
						PIC_PRINT(SYS_ERR, "write file fail(ret = %d)\n", ret);
						snapshot_ongoing = 0;
						if(pic_info.dev==REC_DEV_USBH)
							vPortFree(pic_info.OriginAddr);
						else
							vPortFree(pic_info.addr);

						vPortFree(pic_info.picname);
						f_close(&pic_file);
						continue;
					}

					f_close(&pic_file);
#if defined(CONFIG_SYSTEM_PLATFORM_SN98660) || defined(CONFIG_SYSTEM_PLATFORM_SN98661) || defined(CONFIG_SYSTEM_PLATFORM_SN98671) || defined (CONFIG_SYSTEM_PLATFORM_SN98293)
					rec_filenode_add(rec_recordtype_get(pic_info.type), pic_info.picname, NULL);			
#endif
					snapshot_ongoing = 0;
					snapshot_free_space -= pic_info.size;
				} else {
					snapshot_is_full = 1;
					PIC_PRINT(SYS_WARN, "pic folder is full\n");
				}
			} else {
				if ((get_sd_umount_err() == 1) && (first_insert == 1)) {
#if defined(CONFIG_SYSTEM_PLATFORM_SN98660) || defined(CONFIG_SYSTEM_PLATFORM_SN98661) || defined(CONFIG_SYSTEM_PLATFORM_SN98671) || defined (CONFIG_SYSTEM_PLATFORM_SN98293)
					rec_filelist_clear(rec_recordtype_get(pic_info.type));
#endif
					first_insert = 0;
					PIC_PRINT(SYS_ERR, "sd card doesn't exist\n");
				}
			}
			if(pic_info.dev==REC_DEV_USBH)
				vPortFree(pic_info.OriginAddr);
			else
				vPortFree(pic_info.addr);

			vPortFree(pic_info.picname);
		} else {
			if ((get_sd_umount_err() == 0) && (first_insert == 0)) {
#if defined(CONFIG_SYSTEM_PLATFORM_SN98660) || defined(CONFIG_SYSTEM_PLATFORM_SN98661) || defined(CONFIG_SYSTEM_PLATFORM_SN98671) || defined (CONFIG_SYSTEM_PLATFORM_SN98293)
				rec_filenode_update(T_SNAPSHOT);
				rec_import_files(T_SNAPSHOT);
				if(snapshot_info.fileformat==RECORD_COUNTFORMAT)
					get_snapshot_count(&snapshot_info);
#endif
				first_insert = 1;
			} else if ((get_sd_umount_err() == 1) && (first_insert == 1)) {
#if defined(CONFIG_SYSTEM_PLATFORM_SN98660) || defined(CONFIG_SYSTEM_PLATFORM_SN98661) || defined(CONFIG_SYSTEM_PLATFORM_SN98671) || defined (CONFIG_SYSTEM_PLATFORM_SN98293)
				rec_filelist_clear(T_SNAPSHOT);
#endif
				first_insert = 0;
			} else {
				/* do notihing */
			}
		}
	}
	vTaskDelete(NULL);
}

/**
 * @brief task for snapshot
 */
static void task_save_thumbnail(void *pvParameters)
{
	pic_info_t pic_info;
	FIL pic_file;
	int ret;
	unsigned int uiWritedSize;
	int readsdcardok = 0;
	
	while (1) {
		xQueueReceive(thumbnail_info.queue_pic, &pic_info, portMAX_DELAY );
		if (thumbnail_start == 1 && get_sd_umount_err() == 0) {
//			PIC_PRINT(SYS_DBG, "thumbnail size = %x\n", pic_info.size);
			thumbnail_ongoing = 1;
			PIC_PRINT(SYS_DBG, "save %s\n", pic_info.picname);
			if ((ret = f_open(&pic_file, pic_info.picname, FA_WRITE | FA_CREATE_ALWAYS)) != FR_OK) {
				PIC_PRINT(SYS_ERR, "open file fail(ret = %d)\n", ret);
				thumbnail_ongoing = 0;
				goto end;
			}

			if ((ret = f_write(&pic_file, pic_info.addr, pic_info.size, &uiWritedSize)) != FR_OK) {
				PIC_PRINT(SYS_ERR, "write file fail(ret = %d)\n", ret);
				thumbnail_ongoing = 0;
				f_close(&pic_file);
				goto end;
			}

			f_close(&pic_file);
//			PIC_PRINT(SYS_DBG, "write thumbnail success\n");
			rec_thumbnail_add(rec_recordtype_get(pic_info.type), pic_info.picname);
			thumbnail_ongoing = 0;
		} else {
			if (get_sd_umount_err() == 1) {
				PIC_PRINT(SYS_ERR, "sd card doesn't exist\n");
				readsdcardok = 0;
			}
		}
end:
		if(pic_info.dev==REC_DEV_USBH)
			vPortFree(pic_info.OriginAddr);
		else
			vPortFree(pic_info.addr);
		
		vPortFree(pic_info.picname);
	}
	vTaskDelete(NULL);
}

/**
 * @brief interface function - get picture from video task
 * @param pPic pointer for picture start address
 * @param size size fo picture
 * @param name no use set NULL
 */
void mf_snapshot_get_pic(unsigned char *pPic, unsigned int size, const char *name, enum RECORDKIND type)
{
	pic_info_t pic_info;
	if(f_get_drv()==REC_DEV_USBH){
		pic_info.OriginAddr = pvPortMalloc(size+0x1ff, GFP_KERNEL, MODULE_APP);
		pic_info.addr = ((unsigned int)pic_info.OriginAddr+0x1ff) & ~0x1ff;
		pic_info.dev = REC_DEV_USBH;
	}else{
		pic_info.addr = pvPortMalloc(size, GFP_KERNEL, MODULE_APP);
		pic_info.dev = REC_DEV_MMC;
	}
	
	PIC_PRINT(SYS_INFO, "mf_snapshot_get_pic-Malloc size=%d\n", size);
	if (!pic_info.addr) {
		PIC_PRINT(SYS_ERR, "picture buffer allocate fail (size = %d)\n", size);
		return;
	}
	
	memcpy(pic_info.addr, pPic, size);
	pic_info.size = size;
	pic_info.type = type;
	pic_info.picname = strdup(name);
	//**check queue is full or not
	xQueueSendToBack(snapshot_info.queue_pic, &pic_info , portMAX_DELAY);
}

/**
 * @brief interface function - get picture from video task
 * @param pPic pointer for picture start address
 * @param size size fo picture
 */
void mf_thumbnail_get_pic(unsigned char *pPic, unsigned int size, const char *name, unsigned int type)
{
	pic_info_t pic_info;
	if(f_get_drv()==REC_DEV_USBH){
		pic_info.OriginAddr = pvPortMalloc(size+0x1ff, GFP_KERNEL, MODULE_APP);
		pic_info.addr = ((unsigned int)pic_info.OriginAddr+0x1ff) & ~0x1ff;
		pic_info.dev = REC_DEV_USBH;
	}else{
		pic_info.addr = pvPortMalloc(size, GFP_KERNEL, MODULE_APP);
		pic_info.dev = REC_DEV_MMC;
	}
	
	if (!pic_info.addr) {
		PIC_PRINT(SYS_ERR, "thumbnail buffer allocate fail (size = %d)\n", size);
		return;
	}
	
	memcpy(pic_info.addr, pPic, size);
	pic_info.size = size;
	pic_info.picname = strdup(name);
	pic_info.type = type;
	//**check queue is full or not
	xQueueSendToBack(thumbnail_info.queue_pic, &pic_info , portMAX_DELAY);
}

/**
 * @brief interface function - report snapshot is no working or not
 * @return 1:snapshot is on working 0:snapshot isn't on woring
 */
int mf_snapshot_status(void)
{
	if (uxQueueMessagesWaiting(snapshot_info.queue_pic)) {
		return 1;	//still working
	} else {
		if (snapshot_ongoing) {
			return 1;
		}
		return 0;
	}
}

int mf_snapshot_get_queue_num(void)
{
	return uxQueueMessagesWaiting(snapshot_info.queue_pic);
}

/**
 * @brief interface function - report snapshot is no working or not
 * @return 1:snapshot is on working 0:snapshot isn't on woring
 */
int mf_thumbnail_status(void)
{
	if (uxQueueMessagesWaiting(thumbnail_info.queue_pic))
		return 1;	//still working
	else {
		return !!(thumbnail_ongoing);
	}
}

/**
 * @brief interface function - notice to update space for doing snapshot(when sd mount)
 * @param iUsedSize size of folder for snapshot already used(Byte unit)
 * @param iCanuseSize size of folder for snapshot remainds(Byte unit)
 */
void mf_snapshot_update_size(long long iUsedSize, long long iCanuseSize)	//Byte
{
	//=====test start=======
	long long size = 0; //test
	char target[] = "/sonix/picture";

	//fs_cmd_du("/sonix/picture", &size);
	size = (long long)fs_cmd_ffdu(target);

	iUsedSize = (long long)size << 20;
	//=====test start=======

	if (iCanuseSize >= iUsedSize) {
		snapshot_free_space = (iCanuseSize - iUsedSize);
	} else
		snapshot_free_space = 0;

	if (snapshot_free_space < SD_PICTURE_RESERVED)
		snapshot_is_full = 1;
	else
		snapshot_is_full = 0;
}

/**
 * @brief interface function - report space for snapshot is full or not
 * @return 1:is full
 */
int mf_snapshot_get_isfull(void)
{
	return snapshot_is_full;
}

int del_snapshot_file(char *fn)
{
	FILINFO finfo;
	char lfn[ _MAX_LFN + 1 ];
	char target[LEN_FILENAME];
	char  *path0;
	int ret = 0;
	struct usr_config *pUserCfg = NULL;

	if ((ret = get_usr_config(&pUserCfg)) != 0) {
		PIC_PRINT(SYS_ERR, "get usr config failed\n");
		return (-1);
	}
	
	memset(target, 0x00, sizeof(target));
	snprintf(target, sizeof(target), "%s/%s", pUserCfg->pic_path, fn);
	path0 = target;
	finfo.lfname = lfn;
	finfo.lfsize = _MAX_LFN + 1;

	if (f_stat(path0, & finfo) != FR_OK)
		return (-1);
	
	{
		PIC_PRINT(SYS_DBG, "snapshot_free_space=%u KB\n", (int)(snapshot_free_space >> 10));
		snapshot_free_space = snapshot_free_space + finfo.fsize;
		if (snapshot_free_space >= SD_PICTURE_RESERVED)
			snapshot_is_full = 0;
#if defined(CONFIG_SYSTEM_PLATFORM_SN98660) || defined(CONFIG_SYSTEM_PLATFORM_SN98661) || defined(CONFIG_SYSTEM_PLATFORM_SN98671) || defined (CONFIG_SYSTEM_PLATFORM_SN98293)
		rec_filenode_del(T_SNAPSHOT, fn);
#endif
		PIC_PRINT(SYS_DBG, "after delete ,snapshot_free_space=%u KB\n", (int)(snapshot_free_space >> 10));
	}
	return 0;
}
