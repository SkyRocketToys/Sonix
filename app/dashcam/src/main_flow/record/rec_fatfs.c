/*********************************************************************************
* rec_fatfs.c
*
* Implementation file of  reading memory card space
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
#include <sys_clock.h>
#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "rec_common.h"
#include "rec_fatfs.h"
#include <libmid_nvram/snx_mid_nvram.h>


int card_info_space(long long *total_space, long long *free_space)
{
	int ret;
	FATFS *fs;
	unsigned long fre_clust, tot_clust;
	long long size = 0;
	
	ret = f_getfree("0:", &fre_clust, &fs);
	if (ret == FR_OK) {
		/* Get total sectors and free sectors */
		tot_clust = (fs->n_fatent - 2);
		tot_clust = (tot_clust * fs->csize) >> 11;
		size = (long long)tot_clust << 20;
		*total_space = size;

		/* Get total sectors and free sectors */
		tot_clust = (fs->n_fatent - 2);
		fre_clust = (fre_clust * fs->csize) >> 11;
		size = (long long)fre_clust << 20;
		*free_space = size;
		
		return 0;
	} else
		return (-1);
}

/**
* @brief   card total space
* @return totalspace (B)
*/

long long card_total_space(void)
{
	int ret;
	FATFS *fs;
	unsigned long fre_clust, tot_clust;
	long long size = 0;
	
	ret = f_getfree("0:", &fre_clust, &fs);
	if (ret == FR_OK) {
		/* Get total sectors and free sectors */
		tot_clust = (fs->n_fatent - 2);
		tot_clust = (tot_clust * fs->csize) >> 11;
		size = (long long)tot_clust << 20;
		return size;
	} else
		return 0;
}

/**
* @brief   card free space
* @return freespace (B)
*/

long long card_free_space(void)
{
	int ret;
	FATFS *fs;
	unsigned long fre_clust, tot_clust;
	long long size = 0;
	
	ret = f_getfree("0:", &fre_clust, &fs);
	if (ret == FR_OK) {
		/* Get total sectors and free sectors */
		tot_clust = (fs->n_fatent - 2);
		fre_clust = (fre_clust * fs->csize) >> 11;
		size = (long long)fre_clust << 20;
		return size;
	} else
		return -1;
}

/**
* @brief   check  memory card root dir is exist or not
* @return  0: not exist  1:exist
*/

int chk_root_folder(void)
{
	char path[LEN_FILENAME];
	struct usr_config *pUserCfg;
	int ret = 0;
	
	if ((ret = get_usr_config(&pUserCfg)) != 0) {
		REC_PRINT(SYS_ERR, "get usr config failed.\n");
		return 0;
	}
	
	memset(path, 0x00, sizeof(path));
	snprintf(path, sizeof(path), "%s", pUserCfg->root_path);
	if (exists(path) == 0)
		return 0;
	else
		return 1;
}


/**
* @brief check each folder is exist or not
*/
int chk_card_allfolder(void)
{
	int rec;
	int intbuf;
	char path[LEN_FILENAME];
	struct usr_config *pUserCfg;
	
	if (get_usr_config(&pUserCfg) != 0) {
		REC_PRINT(SYS_ERR, "get usr config failed.\n");
		return 0;
	}
	
	memset(path, 0x00, sizeof(path));
	snprintf(path, sizeof(path), "%s", pUserCfg->root_path);
	if (exists(path) == 0) {
		rec = mkdir(path);
		REC_PRINT(SYS_DBG, "check root folder =%d \n", rec);
		//return 0;
	}
	snx_nvram_integer_get(NVRAM_SPACE, NVRAM_RECORD_UPBD, &intbuf);
	if (intbuf != 0) {
		memset(path, 0x00, sizeof(path));
		snprintf(path, sizeof(path), "%s", pUserCfg->rec_path);
		if (exists(path) == 0) {
			rec = mkdir(path);
			REC_PRINT(SYS_DBG, "check schedule folder =%d \n", rec);
			//return 0;
		}
		memset(path, 0x00, sizeof(path));
		snprintf(path, sizeof(path), "%s", pUserCfg->rec_tn_path);
		if (exists(path) == 0) {
			rec = mkdir(path);
			REC_PRINT(SYS_DBG, "check schedule thumbnail folder =%d \n", rec);
			//return 0;
		}
	}
#ifndef CONFIG_APP_DRONE
	snx_nvram_integer_get(NVRAM_SPACE, NVRAM_PROTECT_UPBD, &intbuf);
	if (intbuf != 0) {
		memset(path, 0x00, sizeof(path));
		snprintf(path, sizeof(path), "%s", pUserCfg->protect_path);
		if (exists(path) == 0) {
			rec = mkdir(path);
			REC_PRINT(SYS_DBG, "check protected folder =%d \n", rec);
			//return 0;
		}
		memset(path, 0x00, sizeof(path));
		snprintf(path, sizeof(path), "%s", pUserCfg->protect_tn_path);
		if (exists(path) == 0) {
			rec = mkdir(path);
			REC_PRINT(SYS_DBG, "check protected thumbnail folder =%d \n", rec);
			//return 0;
		}
	}
#endif
	snx_nvram_integer_get(NVRAM_SPACE, NVRAM_PICTURE_UPBD, &intbuf);
	if (intbuf != 0) {
		memset(path, 0x00, sizeof(path));
		snprintf(path, sizeof(path), "%s", pUserCfg->pic_path);
		if (exists(path) == 0) {
			rec = mkdir(path);
			REC_PRINT(SYS_DBG, "check picture folder =%d \n", rec);
			//return 0;
		}
	}
#ifndef CONFIG_APP_DRONE
	snx_nvram_integer_get(NVRAM_SPACE, NVRAM_TIMELAPSE_UPBD, &intbuf);
	if (intbuf != 0) {
		memset(path, 0x00, sizeof(path));
		snprintf(path, sizeof(path), "%s", pUserCfg->timelapse_path);
		if (exists(path) == 0) {
			rec = mkdir(path);
			REC_PRINT(SYS_DBG, "check timelapse folder =%d \n", rec);
			//return 0;
		}
		memset(path, 0x00, sizeof(path));
		snprintf(path, sizeof(path), "%s", pUserCfg->ts_tn_path);
		if (exists(path) == 0) {
			rec = mkdir(path);
			REC_PRINT(SYS_DBG, "ccheck timelapse thumbnail folder  =%d \n", rec);
			//return 0;
		}
	}
#endif
	return 0;
}


/**
* @brief  get current foler used space
* @return size  (B)
*/
long long get_curfolder_usedspace(int record_type)
{
	long long used_size_mb = 0;
	char target[LEN_FILENAME];
	long long size = 0;
	struct usr_config *pUserCfg;
	
	if(get_usr_config(&pUserCfg) != 0) {
		REC_PRINT(SYS_ERR, "get usr config failed.\n");
		return -1;
	}

	memset(target, 0x00, sizeof(target));
	if (record_type == SD_REC_SCHED) {
		snprintf(target, sizeof(target), "%s", pUserCfg->rec_path);
	} else if (record_type == SD_REC_PROTECT) {
		snprintf(target, sizeof(target), "%s", pUserCfg->protect_path);
	} else if (record_type == SD_REC_PICTURE) {
		snprintf(target, sizeof(target), "%s",pUserCfg->pic_path);
	} else if (record_type == SD_REC_TIMELAPSE) {
		snprintf(target, sizeof(target), "%s", pUserCfg->timelapse_path);
	}
	else {
		REC_PRINT(SYS_ERR, "Unsupport type =%d \n", record_type);
		return (-1);
	}

	used_size_mb = fs_cmd_ffdu(target);
	size = (long long)used_size_mb << 20;
	return size;
}
