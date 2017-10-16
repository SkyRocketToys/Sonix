/*********************************************************************************
* sdcad_common.c
*
* Header file of  sccard common function
*
* History:
*    2016/04/21 - [Allen_Chang] created file
*
* Copyright (C) 1996-2015, Sonix, Inc.
*
* All rights reserved. No Part of this file may be reproduced, stored
* in a retrieval system, or transmitted, in any form, or by any means,
* electronic, mechanical, photocopying, recording, or otherwise,
* without the prior consent of Sonix, Inc.

*********************************************************************************/

#include <FreeRTOS.h>
#include <task.h>
#include <bsp.h>
#include <timers.h>
#include <nonstdlib.h>
#include <generated/snx_sdk_conf.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "rec_common.h"
#include "debug.h"
#include "upload1.h"
#include "../playback/play_back.h"
#include "video_main.h"
#include <libmid_nvram/snx_mid_nvram.h>
#include "rec_schedule.h"
#include "rec_common.h"


struct usr_config rec_config;

#ifndef CONFIG_APP_DRONE
static int SdRecordFormat = 0;	/* timeformat */
#else
static int SdRecordFormat = 1;	/* countformat */
#endif


/**
* @brief interface function -get_rec_fileformat
* @param
* @return  0:RECORD_TIMEFORMAT 1:RECORD_COUNTFORMAT
*/
int rec_fileformat_get(void)
{
	return SdRecordFormat;
}

int get_usr_config(struct usr_config **pusr_config)
{
	int ret = 0;
	struct usr_config *pUserCfg = &rec_config;
	
	if (pUserCfg->set_config_ready == 0)
		ret = -1;
	else
		*pusr_config = &rec_config;
	
	return ret;
}


static int rec_regularoffset_get(const char * filename)
{
	char recordcurrentname[LEN_FILENAME] = {0};
	char *px, *delim = "/";
	int offset = 0;
	int len = strlen(filename);

	strncpy(recordcurrentname, filename, len);
	
	/* Parsing related key words */
	px = strtok(recordcurrentname, delim);
	offset += (strlen(px) + 1);
	while (px = strtok(NULL, delim))
	{
		offset += (strlen(px) + 1);
		if (offset >= len) {
			// end of tail, so break and return.
			offset -= strlen(px);
			break;
		}
	}			

	return offset;
}


/**
* @brief search directory in memory card and read content and write all file node list  to file
* @param type:0:schedule file 1:protected file  2:picture file 3:thumbnail file
* @param path:pointer to absolute path
* @param fileformat: 0:filename by time format  1:filename by count format
* @return return 0 if success
*/
int chkdir_writetofile(int type, char *path)
{
	DIR dir;
	FILINFO fno;
	FIL  file;
	char filename[LEN_FILENAME];
	char filelistpath[LEN_FILENAME];
	char filelistdir[LEN_FILENAME];
	char target[LEN_FILENAME];
	char lfn[_MAX_LFN + 1];
	uint32_t bufsize = 0, uiWritedSize;
	struct usr_config *pUserCfg = &rec_config;
	FRESULT ret, retopen;

	fno.lfname = lfn;
	fno.lfsize = sizeof(lfn);
	memset(filelistpath, 0x00, sizeof(filelistpath));
	memset(filelistdir, 0x00, sizeof(filelistdir));
	if (type == SD_REC_SCHED) {
		snprintf(filelistdir, sizeof(filelistdir), "%s", pUserCfg->rec_path);
		snprintf(filelistpath, sizeof(filelistpath), "%s/%s", pUserCfg->rec_path, SD_RECORD_FILELIST);
		strncpy(path, filelistpath, strlen(filelistpath));
	} else if (type == SD_REC_PROTECT) {
		snprintf(filelistdir, sizeof(filelistdir), "%s", pUserCfg->protect_path);
		snprintf(filelistpath, sizeof(filelistpath), "%s/%s", pUserCfg->protect_path, SD_PROTECT_FILELIST);
		strncpy(path, filelistpath, strlen(filelistpath));
	} else if (type == SD_REC_PICTURE) {
		snprintf(filelistdir, sizeof(filelistdir), "%s", pUserCfg->pic_path);
		snprintf(filelistpath, sizeof(filelistpath), "%s/%s", pUserCfg->pic_path, SD_PICTURE_FILELIST);
		strncpy(path, filelistpath, strlen(filelistpath));
	} else if (type == SD_REC_TIMELAPSE) {
		snprintf(filelistdir, sizeof(filelistdir), "%s", pUserCfg->timelapse_path);
		snprintf(filelistpath, sizeof(filelistpath), "%s/%s", pUserCfg->timelapse_path, SD_TIMELAPSE_FILELIST);
		strncpy(path, filelistpath, strlen(filelistpath));
	} else {
		REC_PRINT(SYS_ERR, "type error\n");
		return LIST_FILE_TYPE_ERROR;
	}

	if (0 != is_current_upload_file(filelistpath, UPLOAD_FG)) {
		REC_PRINT(SYS_INFO, "file (%s) is downloading \n", filelistpath);
		return LIST_FILE_DOWNLOAD;
	}

	if ((ret = f_open(&file, filelistpath, FA_CREATE_ALWAYS | FA_WRITE)) != FR_OK) {
		REC_PRINT(SYS_ERR, "file %s open fail(ret = %d)\n", filelistpath, ret);
		return LIST_FILE_OPEN_FAIL;
	}

	retopen = f_opendir(&dir, filelistdir);
	while ((ret = f_readdir(&dir, &fno)) == FR_OK) {
		if (ret != FR_OK || fno.fname[0] == 0) break;
		if ((fno.fname[0] == '.') || (snx_fm_check_file_legal(GET_FN(fno)) != 1)) continue;
		if (ret == FR_OK) {
			if (fno.fattrib & AM_DIR) { //dir

			} else {
				memset(filename, 0, sizeof(filename));
				memset(target, 0x00, sizeof(target));
				snprintf(target, sizeof(target), "%s/%s", filelistdir, GET_FN(fno));
				REC_PRINT(SYS_DBG, "target=%s\n", target);
				int filelen;
				if (type == SD_REC_SCHED) {
					pb_get_filelen(target, &filelen);
					snprintf(filename, sizeof(filename), "%s,%04d%02d%02d%02d%02d%02d,%04d,%c\n", GET_FN(fno), FF_YEAR(fno.fdate), FF_MONTH(fno.fdate), FF_DATE(fno.fdate),
					        FF_HOUR(fno.ftime), FF_MINUTE(fno.ftime), FF_SECOND(fno.ftime), filelen , (fno.fattrib & AM_RDO) ? '1' : '0');
				} else if (type == SD_REC_PROTECT) {
					pb_get_filelen(target, &filelen);
					snprintf(filename, sizeof(filename), "%s,%04d%02d%02d%02d%02d%02d,%04d,%c\n", GET_FN(fno), FF_YEAR(fno.fdate), FF_MONTH(fno.fdate), FF_DATE(fno.fdate),
					        FF_HOUR(fno.ftime), FF_MINUTE(fno.ftime), FF_SECOND(fno.ftime), filelen , (fno.fattrib & AM_RDO) ? '1' : '0');

				} else if (type == SD_REC_PICTURE) {
					snprintf(filename, sizeof(filename), "%s,%04d%02d%02d%02d%02d%02d\n", GET_FN(fno), FF_YEAR(fno.fdate), FF_MONTH(fno.fdate), FF_DATE(fno.fdate),
					        FF_HOUR(fno.ftime), FF_MINUTE(fno.ftime), FF_SECOND(fno.ftime));

				} else if (type == SD_THUMBNAIL) {
					snprintf(filename, sizeof(filename), "%s,%04d%02d%02d%02d%02d%02d\n", GET_FN(fno), FF_YEAR(fno.fdate), FF_MONTH(fno.fdate), FF_DATE(fno.fdate),
					        FF_HOUR(fno.ftime), FF_MINUTE(fno.ftime), FF_SECOND(fno.ftime));
				} else if (type == SD_REC_SCHED) {
					pb_get_filelen(target, &filelen);
					snprintf(filename, sizeof(filename), "%s,%04d%02d%02d%02d%02d%02d,%04d,%c\n", GET_FN(fno), FF_YEAR(fno.fdate), FF_MONTH(fno.fdate), FF_DATE(fno.fdate),
					        FF_HOUR(fno.ftime), FF_MINUTE(fno.ftime), FF_SECOND(fno.ftime), filelen , (fno.fattrib & AM_RDO) ? '1' : '0');
				} else if (type == SD_REC_TIMELAPSE) {
					pb_get_filelen(target, &filelen);
					snprintf(filename, sizeof(filename), "%s,%04d%02d%02d%02d%02d%02d,%04d,%c\n", GET_FN(fno), FF_YEAR(fno.fdate), FF_MONTH(fno.fdate), FF_DATE(fno.fdate),
					        FF_HOUR(fno.ftime), FF_MINUTE(fno.ftime), FF_SECOND(fno.ftime), filelen , (fno.fattrib & AM_RDO) ? '1' : '0');
				}
				
				if (((ret = f_write(&file, filename, strlen(filename), &uiWritedSize)) != FR_OK) || ((strlen(filename)) != uiWritedSize)) {
					REC_PRINT(SYS_ERR, "write data fail(ret = %d), %x, %x\n", ret, bufsize, uiWritedSize);
				}
			}
		}
	}
	REC_PRINT(SYS_DBG, "write end \n");
	if (retopen == FR_OK)
		f_closedir(&dir);

	f_close(&file);

	return LIST_FILE_OK;
}


void saferFree(void **pp)
{
	if (pp != NULL && *pp != NULL) {
		free(*pp);
		*pp = NULL;
	}
}

/**
* @brief   get record name to make snapshot name
* @param  rec_info : record info structure
* @param  type schudue record :0  timelapse record:1 protected record :2
*/

void rec_snapshot_query(enum RECORDKIND type, const char *name)
{
	char recordcurrentname[LEN_FILENAME] = {0};
	char snapshotname[LEN_FILENAME] = {0};
	char *px;
	char *px2;
	char *px3;
	char *px4;
	char *delim = "/";
	char *delim2 = ".";
	struct usr_config *pUserCfg = &rec_config;
	
	strncpy(recordcurrentname, name, strlen(name));
	px = strtok(recordcurrentname, delim);
	px2 = strtok(NULL, delim);
	px3 = strtok(NULL, delim);
	px4 = strtok(px3, delim2);

	if (type == SCHED_RECORD) 
		strcat (snapshotname, pUserCfg->rec_tn_path);
	else if (type == TIMELAPSE_RECORD)
		strcat (snapshotname, pUserCfg->ts_tn_path);
	else if (type == PROTECT_RECORD)
		strcat (snapshotname, pUserCfg->protect_tn_path);
	
	strcat (snapshotname, "/");
	strcat (snapshotname, px4);
	strcat (snapshotname, ".jpg");
	mf_set_thumbnail_fordup(snapshotname, type);
}


/**
* @brief   check record file and add filelen
* @param  oldname:oldstyle name
* @param  newname:newstyle name
* @param  filelen: file len
*/
void rename_recfile_addlen(const char *oldname, char *newname, int filelen)
{
	char *p = (char *)oldname;
	char szTmp[LEN_FILENAME] = {0};
	char szTmp2[LEN_FILENAME] = {0};
	int len;
	len = strlen(oldname);
	strncpy(szTmp, p, len - 8);
	strncpy(szTmp2, p + len - 4, 4);
	sprintf(newname, "%s%04d%s", szTmp, filelen, szTmp2);
}

/**
* @brief if recording file have been compeleted,add len to this file
* @param finishedfile : a finished recording file.
* @param renameoffile : return rename of file.
*/
void rec_finishedfile_addlen(char *finishedfile, char *renameoffile)
{
	FILINFO finfo;
	int filelen = 0;
	char lfn[ _MAX_LFN + 1 ];

	finfo.lfname = lfn;
	finfo.lfsize = _MAX_LFN + 1;
	if (f_stat(finishedfile, &finfo) == FR_OK) {
		pb_get_filelen(finishedfile, &filelen);
		rename_recfile_addlen(finishedfile, renameoffile, filelen);
		REC_PRINT(SYS_INFO, "rename %s to %s \n", finishedfile, renameoffile);
		f_rename(finishedfile, renameoffile);
	}
}

void rec_size_del(const char *filename, long long *size)
{
	FILINFO finfo;
	char lfn[ _MAX_LFN + 1 ];
	char *path0 = (char *)filename;

	finfo.lfname = lfn;
	finfo.lfsize = _MAX_LFN + 1;
	if (f_stat(path0, & finfo) == FR_OK) {
		*size -= finfo.fsize;
		if (*size < 0)
			*size = 0;
	}
}


static int rec_loadcfg(struct usr_config *pUserCfg)
{
	char sd_root[LEN_FILENAME] = {0};
	char sd_record[LEN_FILENAME] = {0};
#ifndef CONFIG_APP_DRONE 
	char sd_protect[LEN_FILENAME] = {0};
	char sd_timelapse[LEN_FILENAME] = {0};
#endif
	char sd_picture[LEN_FILENAME] = {0};
	char sd_thumbnail[LEN_FILENAME] = {0};

	int record_type = 0;
	
	if (!pUserCfg) {
		REC_PRINT(SYS_ERR, "Invalid argument\n");
		return (-1);
	}

	memset(pUserCfg, 0x00, sizeof(struct usr_config));

	// get directory name from nvram
	if (snx_nvram_string_get(NVRAM_PATH_INFO, NVRAM_SD_ROOT, sd_root) != NVRAM_SUCCESS) {
		REC_PRINT(SYS_ERR, "get root directory from nvram failed!\n");
		snprintf(sd_root, sizeof(sd_root), "%s", SD_ROOT);
	}
	if (snx_nvram_string_get(NVRAM_PATH_INFO, NVRAM_SD_RECORD, sd_record) != NVRAM_SUCCESS) {
		REC_PRINT(SYS_ERR, "get record directory from nvram failed!\n");
		snprintf(sd_record, sizeof(sd_record), "%s", SD_SCHED_PATH);
	}
#ifndef CONFIG_APP_DRONE
	if (snx_nvram_string_get(NVRAM_PATH_INFO, NVRAM_SD_PROTECT, sd_protect) != NVRAM_SUCCESS) {
		REC_PRINT(SYS_ERR, "get protect directory from nvram failed!\n");
		snprintf(sd_protect, sizeof(sd_protect), "%s", SD_PROTECT_PATH);
	}

	if (snx_nvram_string_get(NVRAM_PATH_INFO, NVRAM_SD_TIMELAPSE, sd_timelapse) != NVRAM_SUCCESS) {
		REC_PRINT(SYS_ERR, "get timelapse directory from nvram failed!\n");
		snprintf(sd_timelapse, sizeof(sd_timelapse), "%s", SD_TIMELAPSE_PATH);
	}
#endif
	if (snx_nvram_string_get(NVRAM_PATH_INFO, NVRAM_SD_PICTURE, sd_picture) != NVRAM_SUCCESS) {
		REC_PRINT(SYS_ERR, "get picture directory from nvram failed!\n");
		snprintf(sd_picture, sizeof(sd_picture), "%s", SD_PICTURE_PATH);
	}

	if (snx_nvram_string_get(NVRAM_PATH_INFO, NVRAM_SD_THUMBNAIL, sd_thumbnail) != NVRAM_SUCCESS) {
		REC_PRINT(SYS_ERR, "get thumbnail directory from nvram failed!\n");
		snprintf(sd_thumbnail, sizeof(sd_thumbnail), "%s", SD_THUMBNAIL_PATH);
	}

	// set foleder len
	pUserCfg->root_folder_len = strlen(sd_root);
	pUserCfg->rec_folder_len = strlen(sd_record);
#ifndef CONFIG_APP_DRONE
	pUserCfg->protect_folder_len = strlen(sd_protect);
	pUserCfg->timelapse_folder_len = strlen(sd_timelapse);
#endif
	pUserCfg->pic_folder_len = strlen(sd_picture);
	pUserCfg->thumbnail_folder_len = strlen(sd_thumbnail);

	// set file path
	snprintf(pUserCfg->root_path, sizeof(pUserCfg->root_path), "/%s", sd_root);
	snprintf(pUserCfg->rec_path, sizeof(pUserCfg->rec_path), "/%s/%s", sd_root, sd_record);
#ifndef CONFIG_APP_DRONE
	snprintf(pUserCfg->timelapse_path, sizeof(pUserCfg->timelapse_path), "/%s/%s", sd_root, sd_timelapse);
	snprintf(pUserCfg->protect_path, sizeof(pUserCfg->protect_path), "/%s/%s", sd_root, sd_protect);
#endif
	snprintf(pUserCfg->pic_path, sizeof(pUserCfg->pic_path), "/%s/%s", sd_root, sd_picture);
	snprintf(pUserCfg->rec_tn_path, sizeof(pUserCfg->rec_tn_path), "/%s/%s/%s", sd_root, sd_record, sd_thumbnail);
#ifndef CONFIG_APP_DRONE
	snprintf(pUserCfg->protect_tn_path, sizeof(pUserCfg->ts_tn_path), "/%s/%s/%s", sd_root, sd_protect, sd_thumbnail);
	snprintf(pUserCfg->ts_tn_path, sizeof(pUserCfg->ts_tn_path), "/%s/%s/%s", sd_root, sd_timelapse, sd_thumbnail);
	snprintf(pUserCfg->timelapse_prefix, sizeof(pUserCfg->timelapse_prefix), "%s", SD_TIMELAPSE_PRENAME);
	snprintf(pUserCfg->protect_prefix, sizeof(pUserCfg->protect_prefix), "%s", SD_PROTECT_PRENAME);
#endif
	snprintf(pUserCfg->rec_prefix, sizeof(pUserCfg->rec_prefix), "%s", SD_SCHED_PRENAME);
	snprintf(pUserCfg->pic_prefix, sizeof(pUserCfg->pic_prefix), "%s", SD_PICTURE_PRENAME);
	snprintf(pUserCfg->thumbnail_prefix, sizeof(pUserCfg->thumbnail_prefix), "%s", SD_THUMBNAIL_PRENAME);
	
	if (snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TYPE, &record_type) != NVRAM_SUCCESS) {
		REC_PRINT(SYS_ERR, "get record type from nvram failed!\n");
		record_type = RECORD_AVI;
	}

	if (record_type == RECORD_AVI) {
		snprintf(pUserCfg->media_type, sizeof(pUserCfg->media_type), "%s", MEDIA_TYPE_AVI);
	} else if (record_type == RECORD_MP4) {
		snprintf(pUserCfg->media_type, sizeof(pUserCfg->media_type), "%s", MEDIA_TYPE_MP4);
#ifdef CONFIG_SKIP_INCOMPLETE_FILE
		/* skip and remove incompleted files when skip option is enable. */
		pUserCfg->skip = 1;
		REC_PRINT(SYS_INFO, "Enable skip incompleted file option!\n");
#else
		pUserCfg->skip = 0;
#endif
	} else {
		/* error, using default value:AVI */
		snprintf(pUserCfg->media_type, sizeof(pUserCfg->media_type), "%s", MEDIA_TYPE_AVI);
	}

	snprintf(pUserCfg->count_index, sizeof(pUserCfg->count_index), "%s", COUNT_INDEX);

	pUserCfg->set_config_ready = 1;
	
	return 0;
}


void rec_filemanage_init(void)
{
	rec_loadcfg(&rec_config);
	
#ifndef CONFIG_APP_DRONE
	snx_fm_init(FORMAT_TIME, &rec_config);
#else
	snx_fm_init(FORMAT_COUNT, &rec_config);
#endif

	/* create related file node */
	rec_filenode_init(T_RECORD, TRUE, rec_sched_incompletefile_hndl);
#ifndef CONFIG_APP_DRONE
#if defined(CONFIG_SYSTEM_PLATFORM_SN98660) || defined(CONFIG_SYSTEM_PLATFORM_SN98661) || defined(CONFIG_SYSTEM_PLATFORM_SN98671) || defined (CONFIG_SYSTEM_PLATFORM_SN98293)
	rec_filenode_init(T_PROTECT, TRUE, rec_protect_incompletefile_hndl);
#endif
	rec_filenode_init(T_TIMELAPSE, TRUE, rec_lapse_incompletefile_hndl);
#endif
#if defined(CONFIG_SYSTEM_PLATFORM_SN98660) || defined(CONFIG_SYSTEM_PLATFORM_SN98661) || defined(CONFIG_SYSTEM_PLATFORM_SN98671) || defined (CONFIG_SYSTEM_PLATFORM_SN98293)
	rec_filenode_init(T_SNAPSHOT, FALSE, NULL);
#endif

}

char * rec_get_dbg_string(enum FILE_TYPE type)
{
	switch (type) {
		case T_AUDIO:
			return "Audio";
		case T_RECORD:
			return "Record";
		case T_TIMELAPSE:
			return "Timelapse";
		case T_PROTECT:
			return "Protect";
		case T_SNAPSHOT:
			return "Snapshot";
		case T_THUMBNAIL:
			return "Thumbnail";
		default:
			return "Unknown type";
	}
}

void rec_filemanage_uninit(void)
{
	snx_fm_uinit();
}

void rec_filenode_init(enum FILE_TYPE type, bool bUseThumbnail, void *cb)
{
	snx_fm_add_filetype(type, bUseThumbnail, cb);
}

void rec_filenode_update(enum FILE_TYPE type)
{
#ifdef CONFIG_APP_DRONE
	snx_fm_update_count(type, NULL);	
#endif	
}


int rec_filenode_add(enum FILE_TYPE type, const char *filename, long long *used_size)
{
	char *path0;
	FILINFO finfo;
	char lfn[ _MAX_LFN + 1 ];

	path0 = (char *)filename;
	finfo.lfname = lfn;
	finfo.lfsize = _MAX_LFN + 1;

	REC_PRINT(SYS_DBG, "Add filenode %s\n", filename);

	if (f_stat(path0, & finfo) != FR_OK) {
		return (-1);
	}
	
	if (used_size != NULL) {
		*used_size += finfo.fsize;
		REC_PRINT(SYS_DBG, "%s used = %u\n", rec_get_dbg_string(type), (int)(*used_size >> 10));
	}
	
	snx_fm_add_filenode(type, finfo, NULL);

	if (type == T_PROTECT)
		f_chmod(path0, AM_RDO, AM_RDO | AM_ARC);

	return 0;
}

int rec_filenode_del(enum FILE_TYPE type, const char * filename)
{
	REC_PRINT(SYS_DBG, "delete filenode %s\n", filename);
	snx_fm_del_file_node(type, filename, NULL);

	return 0;
}

int rec_filelist_amount(enum FILE_TYPE type)
{
	int total_num = 0;
	
	if (snx_fm_get_file_amount(type, &total_num, NULL))
		return (-1);

	return total_num;
}


int rec_thumbnail_add(enum FILE_TYPE type, const char * filename)
{	
	int offset = 0;
	char *ptr;

	ptr = (char *)filename;
	offset = rec_regularoffset_get(filename);
	ptr += offset;

	return (snx_fm_add_thumbnail_node(type, ptr, NULL));
}

/**
* @brief   if delete record file , and must delete thumbnail file
* @param filename :record file name
* Ex: 
* time based - /sonix/record/2015_09_24_14_01_05_0180.avi
* 	thumbnail - /sonix/record/thumbnail/2015_09_24_14_01_05_0000.jpg
* count based - /sonix/record/record_1836_0180.avi
* 	thumbnail - /sonix/record/thumbnail/record_1836_0000.jpg
*/
void rec_thumbnail_del(enum FILE_TYPE type, const char *filename)
{
	char recordcurrentname[LEN_FILENAME] = {0};
	char snapshotname[LEN_FILENAME] = {0};
	char thumbnailname[LEN_FILENAME] = {0};
	char target[LEN_FILENAME] = {0};
	int  i, numb_thumbnail, ret;
	char *px, *px2, *px3, *ptr;
	char *delim = "/";
	int offset = 0;
	struct usr_config *pUserCfg = &rec_config;

	strncpy(recordcurrentname, filename, strlen(filename));
	/* Parsing related key words */
	px = strtok(recordcurrentname, delim);
	px2 = strtok(NULL, delim);
	px3 = strtok(NULL, delim);
	
	/* Concate related path*/
	if (type == T_RECORD) 
		strcat (snapshotname, pUserCfg->rec_tn_path);
	else if (type == T_TIMELAPSE)
		strcat (snapshotname, pUserCfg->ts_tn_path);
	else if (type == T_PROTECT)
		strcat (snapshotname, pUserCfg->protect_tn_path);
	strcat(snapshotname, "/");

	ptr = (char *)filename;
	offset = rec_regularoffset_get(filename);
	ptr += offset;
	
	/* Get the total thumbnail belong to the specified video */
	if ((ret = snx_fm_get_cor_num(type, ptr, &numb_thumbnail, NULL)) != FM_OK) {
		REC_PRINT(SYS_WARN, "get thumbnail failed for parent name %s\n", ptr);
		return;
	}

	REC_PRINT(SYS_DBG, "delete thunmbnail:%s #. of thumbnail %d\n", ptr, numb_thumbnail);
	
	/* Rm related thumbnail from SD */
	for (i = 0; i < numb_thumbnail; i++) {
		memset(target, 0, sizeof(target));
		memset(thumbnailname, 0, sizeof(thumbnailname));
		snx_fm_get_cor_file(type, ptr, thumbnailname, sizeof(thumbnailname), i, NULL);
		strcat(target, snapshotname);
		strcat(target, thumbnailname);
		fs_cmd_rm(target);
		vTaskDelay(20 / portTICK_RATE_MS );
	}
	/* Release thumbnail summerization */
	snx_fm_del_thumbnail_node(type, ptr, NULL);
}


int rec_recordtype_get(unsigned int type)
{
	if (type == SCHED_RECORD)
		return T_RECORD;
	else if (type == TIMELAPSE_RECORD)
		return T_TIMELAPSE;
	else if (type == PROTECT_RECORD)
		return T_PROTECT;
	else if (type == SNAPSHOT_RECORD)
		return T_SNAPSHOT;
	else {
		REC_PRINT(SYS_ERR, ":Unsupport type %d\n", type);
		return (-1);
	}
}


int rec_import_files(enum FILE_TYPE type)
{
	int result, total_num = 0;
	snx_fm_create_filelist(type, NULL);
	while(1) {
		
		result = snx_fm_check_list_is_finish(type);
		if ((result == LIST_FINISH) || (result == LIST_FAIL)|| (get_sd_umount_err() == 1)) {
			break;
		}
		
		vTaskDelay(100 / portTICK_RATE_MS );
	}

	if (result == LIST_FINISH)
		total_num = rec_filelist_amount(type);

	REC_PRINT(SYS_ERR, "(%s) with num (%d) res (%d)\n", rec_get_dbg_string(type), total_num, result);
	
	return ((result == LIST_FINISH)?(1):(0));
}

void rec_filelist_clear(enum FILE_TYPE type)
{
	REC_PRINT(SYS_DBG, "clear filelist %s\n", rec_get_dbg_string(type));
	snx_fm_del_all_file_node(type, NULL);
}


char ** rec_filelist_CreateBuf(enum FILE_TYPE type, int *listsize)
{
	char **listbuffer = NULL;
	snx_fm_node_to_buffer_create(type, &listbuffer, listsize, NULL);
	return listbuffer;
}

void rec_filelist_ReleaseBuf(enum FILE_TYPE type, char **listbuffer, int listsize)
{
	snx_fm_node_to_buffer_delete(type, listbuffer, listsize, NULL);
}

int rec_filelist_Createfile(enum FILE_TYPE type, const char *target_path, int len)
{
	int ret;
	
	ret = snx_fm_node_to_file(type, (char *)target_path, len, NULL);
	if (ret != FM_OK) {
		if (ret == FM_FILETYPE) {
			ret = LIST_FILE_TYPE_ERROR;
		}
		else if (ret == FM_OPENFILE) {
			ret = LIST_FILE_OPEN_FAIL;
		}
		else if (ret = FM_LISTWAIT)
			ret = FM_OK;
		else
			ret = LIST_FILE_UNKNOWN;
	}

	return ret;
}

int rec_filesize_get(const char *name)
{
	char *path0;
	FILINFO finfo;
	char lfn[ _MAX_LFN + 1 ];
	
	path0 = (char *)name;
	finfo.lfname = lfn;
	finfo.lfsize = _MAX_LFN + 1;
	if (f_stat(path0, & finfo) == FR_OK) {
		return (finfo.fsize >> 20);
	}
	
	return (-1);
}


int rec_query_abspath(int filetype, char *output, int output_size, const char *filename)
{
	struct usr_config *pUserCfg = &rec_config;

	switch (filetype) {
		case QUERY_RECORD_FILE:
			snprintf(output, output_size, "%s/%s", pUserCfg->rec_path, filename);
			break;
		case QUERY_PROTECT_FILE:
			snprintf(output, output_size, "%s/%s", pUserCfg->protect_path, filename);
			break;
		case QUERY_PIC_FILE:
			snprintf(output, output_size, "%s/%s", pUserCfg->pic_path, filename);
			break;
		case QUERY_TIMELAPSE_FILE:
			snprintf(output, output_size, "%s/%s", pUserCfg->timelapse_path, filename);
			break;
		case QUERY_RECORD_THUMBNAIL_FILE:
			snprintf(output, output_size, "%s/%s", pUserCfg->rec_tn_path, filename);
			break;
		case QUERY_PROTECT_THUMBNAIL_FILE:
			snprintf(output, output_size, "%s/%s", pUserCfg->protect_tn_path, filename);
			break;
		case QUERY_TIMELAPSE_THUMBNAIL_FILE:
			snprintf(output, output_size, "%s/%s", pUserCfg->ts_tn_path, filename);
			break;
		case QUERY_FILELIST:
			snprintf(output, output_size, "FileList.txt");
			break;
	}

	return 0;
}
