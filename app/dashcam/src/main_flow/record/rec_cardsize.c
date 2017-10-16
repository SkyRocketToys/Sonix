
/*********************************************************************************
* sdcard_sizecontrol.c
*
* Implementation of  memory card size control
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
#include "debug.h"
#include "rec_common.h"
#include "rec_fatfs.h"
#include "rec_cardsize.h"
#include "snapshot.h"
#include <generated/snx_sdk_conf.h>
#include <libmid_nvram/snx_mid_nvram.h>
#include "rec_seed.h"
struct card_size_s sizecontrol;

/**
* @brief   check sd size is too small or not
* @param szName: file name
* @return return 1: sd size is too small,so remind to format sd  0:normal
*/

int cardsize_toosmall(void)
{
	if ((sizecontrol.readcard_finish == 1) && (sizecontrol.remind_format == 1))
		return 1;
	else
		return 0;
}

/**
* @brief   check nvram for each record folder  size
*/
static void chk_nvram_spacesize(void)
{
	int intbuf;
	
	snx_nvram_integer_get(NVRAM_SPACE, NVRAM_RECORD_UPBD, &intbuf);
	sizecontrol.dashcamschedfolder_canusesize = (sizecontrol.dashcamfolder_canusesize * intbuf * 0.01);
#ifndef CONFIG_APP_DRONE
	snx_nvram_integer_get(NVRAM_SPACE, NVRAM_TIMELAPSE_UPBD, &intbuf);
	sizecontrol.dashcamtimelapsefolder_canusesize = (sizecontrol.dashcamfolder_canusesize * intbuf * 0.01);
	snx_nvram_integer_get(NVRAM_SPACE, NVRAM_PROTECT_UPBD, &intbuf);
	sizecontrol.dashcamprotectfolder_canusesize = (sizecontrol.dashcamfolder_canusesize * intbuf * 0.01);
#endif
	snx_nvram_integer_get(NVRAM_SPACE, NVRAM_PICTURE_UPBD, &intbuf);
	sizecontrol.dashcampicturefolder_canusesize = (sizecontrol.dashcamfolder_canusesize * intbuf * 0.01);
}

/**
* @brief   check memory  card  space size and allocate space size for each  folder size
*
*/

void chk_cardsize(void)
{
	card_info_space(&sizecontrol.total_sd_size, &sizecontrol.total_sd_free_size);
	chk_card_allfolder();
	sizecontrol.schedfolder_usedsize = get_curfolder_usedspace(SD_REC_SCHED);
#ifndef CONFIG_APP_DRONE
	sizecontrol.timelapsefolder_usedsize = get_curfolder_usedspace(SD_REC_TIMELAPSE);
	sizecontrol.protectfolder_usedsize = get_curfolder_usedspace(SD_REC_PROTECT);
#endif
	sizecontrol.picturefolder_usedsize = get_curfolder_usedspace(SD_REC_PICTURE);

#ifndef CONFIG_APP_DRONE
	if (sizecontrol.total_sd_free_size + sizecontrol.timelapsefolder_usedsize + sizecontrol.schedfolder_usedsize < SDCARDSIZESMALLLIMIT) {
		REC_PRINT(SYS_WARN, "Remind APP format SDCard\n");
		sizecontrol.remind_format = 1;
	}
	sizecontrol.otherfolder_usedsize = sizecontrol.total_sd_size - sizecontrol.total_sd_free_size - sizecontrol.schedfolder_usedsize - sizecontrol.timelapsefolder_usedsize - sizecontrol.protectfolder_usedsize - sizecontrol.picturefolder_usedsize;
#else
	if (sizecontrol.total_sd_free_size + sizecontrol.schedfolder_usedsize < SDCARDSIZESMALLLIMIT) {
		REC_PRINT(SYS_WARN, "Remind APP format SDCard\n");
		sizecontrol.remind_format = 1;
	}
	sizecontrol.otherfolder_usedsize = sizecontrol.total_sd_size - sizecontrol.total_sd_free_size - sizecontrol.schedfolder_usedsize - sizecontrol.picturefolder_usedsize;
#endif

	if (sizecontrol.otherfolder_usedsize < 0)
		sizecontrol.otherfolder_usedsize = 0;
	sizecontrol.dashcamfolder_canusesize = sizecontrol.total_sd_size - sizecontrol.otherfolder_usedsize;

#if 0
	sizecontrol.dashcamschedfolder_canusesize = (int)(sizecontrol.dashcamfolder_canusesize * SD_RECORD_UPBD);
	sizecontrol.dashcamtimelapsefolder_canusesize = (int)(sizecontrol.dashcamfolder_canusesize * SD_TIMELAPSE_UPBD);
	sizecontrol.dashcamprotectfolder_canusesize = (int)(sizecontrol.dashcamfolder_canusesize * SD_PROTECT_UPBD);
	sizecontrol.dashcampicturefolder_canusesize = (int)(sizecontrol.dashcamfolder_canusesize * SD_PICTURE_UPBD);
	sizecontrol.dashcamthumbnailfolder_canusesize = (int)(sizecontrol.dashcamfolder_canusesize * SD_THUMBNAIL_UPBD);
#endif
	chk_nvram_spacesize();
	mf_snapshot_update_size(sizecontrol.picturefolder_usedsize, sizecontrol.dashcampicturefolder_canusesize);
	REC_PRINT(SYS_INFO, "SD (sz:%u /free:%u MB)\n", 
				(int)(sizecontrol.total_sd_size >> 20), (int)(sizecontrol.total_sd_free_size >> 20));
	REC_PRINT(SYS_INFO, "Sched (used: %u /available: %u MB)\n", 
				(int)(sizecontrol.schedfolder_usedsize >> 20), (int)(sizecontrol.dashcamschedfolder_canusesize >> 20));
	REC_PRINT(SYS_INFO, "Picture (used: %u /available: %u MB)\n", 
				(int)(sizecontrol.picturefolder_usedsize >> 20), (int)(sizecontrol.dashcampicturefolder_canusesize >> 20));
#ifndef CONFIG_APP_DRONE
	if (sizecontrol.dashcamtimelapsefolder_canusesize)
		REC_PRINT(SYS_INFO, "Timelapse (used: %u /available: %u MB)\n", 
					(int)(sizecontrol.timelapsefolder_usedsize >> 20), (int)(sizecontrol.dashcamtimelapsefolder_canusesize >> 20));
	REC_PRINT(SYS_INFO, "Protect (used: %u /available: %u MB)\n", 
				(int)(sizecontrol.protectfolder_usedsize >> 20), (int)(sizecontrol.dashcamprotectfolder_canusesize >> 20));
#endif
	if ((sizecontrol.total_sd_size < 0) || (sizecontrol.total_sd_free_size < 0)) {
		sizecontrol.readcard_finish = -1;
	} else {
		sizecontrol.readcard_finish = 1;
	}
}


/**
* @brief  init  sizecontrol structure
*
*/
void cardsize_init(void)
{
	sizecontrol.total_sd_size = 0;
	sizecontrol.total_sd_free_size = 0;
	sizecontrol.schedfolder_usedsize = 0;
#ifndef CONFIG_APP_DRONE
	sizecontrol.timelapsefolder_usedsize = 0;
	sizecontrol.protectfolder_usedsize = 0;
#endif
	sizecontrol.picturefolder_usedsize = 0;
	sizecontrol.otherfolder_usedsize = 0;
	sizecontrol.dashcamfolder_canusesize = 0;
	sizecontrol.dashcamschedfolder_canusesize = 0;
#ifndef CONFIG_APP_DRONE
	sizecontrol.dashcamtimelapsefolder_canusesize = 0;
	sizecontrol.dashcamprotectfolder_canusesize = 0;
#endif
	sizecontrol.dashcampicturefolder_canusesize = 0;
	sizecontrol.readcard_finish = 0;
	sizecontrol.remind_format = 0;
}

/**
* @brief   unit  sizecontrol structure
*
*/
void cardsize_uinit(void)
{
	sizecontrol.readcard_finish = 0;
}

/**
* @brief  if memory card insert, read memory card information
* @return return 0 if reading memory card information has not finished,  return 1: has finished
*/

int get_readcard_finish(void)
{
	return sizecontrol.readcard_finish;
}

/**
* @brief  get schedule record folder size that  has been used
* @return  schedule record size that has been used
*/

long long get_schedrec_usedsize(void)
{
	return sizecontrol.schedfolder_usedsize;
}

/**
* @brief  get schedule record folder size that  can use for recording
* @return  schedule record size that can use for recording
*/

long long get_schedfolder_canusesize(void)
{
	return sizecontrol.dashcamschedfolder_canusesize;
}


#ifndef CONFIG_APP_DRONE
/**
* @brief  get protected record folder size that  has been used
* @return  protected record size that has been used
*/

long long get_protectrec_usedsize(void)
{
	return sizecontrol.protectfolder_usedsize;
}

/**
* @brief  get protect record folder size that  can use for recording
* @return  protect record size that can use for recording
*/

long long get_protectfolder_canusesize(void)
{
	return sizecontrol.dashcamprotectfolder_canusesize;
}

/**
* @brief  get timelpase record folder size that  has been used
* @return  timelpase record size that has been used
*/

long long get_timelapserec_usedsize(void)
{
	return sizecontrol.timelapsefolder_usedsize;
}

/**
* @brief  get timelpase record folder size that  can use for recording
* @return  timelpase record size that can use for recording
*/

long long get_timelapsefolder_canusesize(void)
{
	return sizecontrol.dashcamtimelapsefolder_canusesize;
}

#endif
