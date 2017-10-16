/*********************************************************************************
* sdcard_record.c
*
* Implementation of  recording  internal APIs
*
* History:
*    2015/10/26 - [Allen_Chang] created file
*
*
* Copyright (C) 1996-2015, Sonix, Inc.
*
* All rights reserved. No Part of this file may be reproduced, stored
* in a retrieval system, or transmitted, in any form, or by any means,
* electronic, mechanical, photocopying, recording, or otherwise,
* without the prior consent of Sonix, Inc.
*
*********************************************************************************/
#include <FreeRTOS.h>
#include <task.h>
#include <bsp.h>
#include <timers.h>
#include <nonstdlib.h>
#include <string.h>
#include <stdio.h>
#include <queue.h>
#include <semphr.h>
#include <sys_clock.h>
#include <sys/time.h>
#include <libmid_automount/automount.h>
#include <libmid_nvram/snx_mid_nvram.h>
#include <generated/snx_sdk_conf.h>
#include "rec_common.h"
#include "debug.h"
#include "rec_schedule.h"
#include "../video/video_main.h"
#include "../audio/audio_main.h"
#include "../daemon/json_cmd.h"
#include "watch_task.h"
#include "user_param.h"
#include "../sensor_cap/sensor_capability.h"
#include "upload1.h"
#include "../playback/play_back.h"
#ifndef CONFIG_APP_DRONE
#include "mcu.h"
#endif
#include "rec_seed.h"


extern char **g_Filelist;
extern int  g_Filelistsize;
extern int userdisablerecord;

recordinfo_t *sdrecord_info = NULL;
automount_info_t *sdaumt_record_info;
#ifndef CONFIG_APP_DRONE
static int firststarttask = 0;
#endif
static system_date_t dev_time;
static chk_card_file_info_t card_info;
static bool force_next_file_flag = FALSE;

static void record_flow( void *pvParameters );
static void rm_file_task(void *pvParameters);

void modify_dev_time(void)
{
	get_date(&dev_time);
	REC_PRINT(SYS_DBG, "modify dev_time dev_year=%d,dev_month=%d,dev_day=%d,dev_hour=%d,dev_minute=%d,dev_second=%d\n", (dev_time).year, (dev_time).month, (dev_time).day, (dev_time).hour, (dev_time).minute, (dev_time).second);
}

/**
 * @brief interface function - enable recording
 */
void enable_rec(void)
{
	REC_PRINT(SYS_DBG, "start record access sdcard\n");
	if ((sdrecord_info != NULL) && (sdrecord_info->sched_p != NULL))
		sdrecord_info->sched_p->sd_sched_internal_en = 1;
}

/**
 * @brief interface function - disalbe recording but stream continue
 */
void diable_rec(void)
{
	REC_PRINT(SYS_DBG, "stop record access sdcard\n");
	if ((sdrecord_info != NULL) && (sdrecord_info->sched_p != NULL))
		sdrecord_info->sched_p->sd_sched_internal_en = 0;
}

/**
 * @brief interface function - add record status
 * @param sdrecord info structure
 * @param flag  status set
 */
void add_rec_status(recordinfo_t *info, int flag)
{
	info->recordstatus |= flag;
}

/**
 * @brief interface function - delrecord status
 * @param sdrecord info structure
 * @param flag  status del
 */
void del_rec_status(recordinfo_t *info, int flag)
{
	info->recordstatus &= ~flag;
}

/**
 * @brief interface function - check status
 * @param sdrecord info structure
 * @param flag
 * @return 1 if status on , else 0 if status off
 */
int chk_rec_status(const recordinfo_t *info, int flag)
{
	return (info->recordstatus & flag);
}

/**
 * @brief interface function - check status
 * @param 
 * @return -1 if sdrecord_info is not created. 1 if recording is working ; 0 if recording is finished
 */
int get_schedrec_status(void)
{
	if(sdrecord_info == NULL)
		return -1;
	else
		return sdrecord_info->recordstatus;
}


/**
* @brief   init schedule record time structure
* @param
* @return return struct SchedTime if success
*/

struct SchedTime *rec_nvraminit(void)
{
	struct SchedTime *sched_p;
	
	if (!(sched_p = (SchedTime_t *)pvPortMalloc(sizeof(SchedTime_t), GFP_KERNEL, MODULE_APP))) {
		REC_PRINT(SYS_ERR, "recrodinfo_t init fail\n");
		goto finally;
	}
	
	memset(sched_p, 0, sizeof(SchedTime_t));
	return sched_p;
	
finally:
	return NULL;
}


/**
* @brief   unit schedule time structure
* @param  double pointer to  sched_p
* @return
*/

void rec_nvram_uninit(struct SchedTime **sched_p)
{
	safeFree(*sched_p);
}


/**
* @brief   get record param from nvram data
* @param  sched_p: save nvram to sched_p structure
* @return return 0 if success
*/
int get_record_param(struct SchedTime *sched_p)
{
	int intbuf;
	unsigned int unsignintbuf;
	
	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_0_BEG_HOUR, &intbuf);
	sched_p->day[0].hour = intbuf;
	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_0_BEG_MINUTE, &intbuf);
	sched_p->day[0].minute = intbuf;
	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_0_BEG_SECOND, &intbuf);
	sched_p->day[0].second = intbuf;
	snx_nvram_unsign_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_0_DURATION, &unsignintbuf);
	sched_p->day[0].duration = unsignintbuf;

	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_1_BEG_HOUR, &intbuf);
	sched_p->day[1].hour = intbuf;
	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_1_BEG_MINUTE, &intbuf);
	sched_p->day[1].minute = intbuf;
	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_1_BEG_SECOND, &intbuf);
	sched_p->day[1].second = intbuf;
	snx_nvram_unsign_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_1_DURATION, &unsignintbuf);
	sched_p->day[1].duration = unsignintbuf;

	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_2_BEG_HOUR, &intbuf);
	sched_p->day[2].hour = intbuf;
	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_2_BEG_MINUTE, &intbuf);
	sched_p->day[2].minute = intbuf;
	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_2_BEG_SECOND, &intbuf);
	sched_p->day[2].second = intbuf;
	snx_nvram_unsign_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_2_DURATION, &unsignintbuf);
	sched_p->day[2].duration = unsignintbuf;

	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_3_BEG_HOUR, &intbuf);
	sched_p->day[3].hour = intbuf;
	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_3_BEG_MINUTE, &intbuf);
	sched_p->day[3].minute = intbuf;
	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_3_BEG_SECOND, &intbuf);
	sched_p->day[3].second = intbuf;
	snx_nvram_unsign_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_3_DURATION, &unsignintbuf);
	sched_p->day[3].duration = unsignintbuf;

	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_4_BEG_HOUR, &intbuf);
	sched_p->day[4].hour = intbuf;
	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_4_BEG_MINUTE, &intbuf);
	sched_p->day[4].minute = intbuf;
	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_4_BEG_SECOND, &intbuf);
	sched_p->day[4].second = intbuf;
	snx_nvram_unsign_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_4_DURATION, &unsignintbuf);
	sched_p->day[4].duration = unsignintbuf;

	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_5_BEG_HOUR, &intbuf);
	sched_p->day[5].hour = intbuf;
	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_5_BEG_MINUTE, &intbuf);
	sched_p->day[5].minute = intbuf;
	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_5_BEG_SECOND, &intbuf);
	sched_p->day[5].second = intbuf;
	snx_nvram_unsign_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_5_DURATION, &unsignintbuf);
	sched_p->day[5].duration = unsignintbuf;

	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_6_BEG_HOUR, &intbuf);
	sched_p->day[6].hour = intbuf;
	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_6_BEG_MINUTE, &intbuf);
	sched_p->day[6].minute = intbuf;
	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_6_BEG_SECOND, &intbuf);
	sched_p->day[6].second = intbuf;
	snx_nvram_unsign_integer_get(NVRAM_RECORD, NVRAM_RECORD_TIME_6_DURATION, &unsignintbuf);
	sched_p->day[6].duration = unsignintbuf;

	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_SCHED_INTERVAL, &intbuf); //20sec
	sched_p->sd_sched_record_interval = intbuf;

	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_MD_INTERVAL, &intbuf); //5sec
	sched_p->sd_alarm_record_interval = intbuf;

	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_SCHED_CYCLE, &intbuf);    //0:not cycle 1:cycle
	sched_p->sd_sched_record_cycle = intbuf;

	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_SCHED_ENABLE, &intbuf); //enable
	sched_p->sd_sched_record_en = intbuf;
	sched_p->sd_sched_internal_en = 1;
	snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_MD_ENABLE, &intbuf); //enable
	sched_p->sd_alarm_record_en = intbuf;

	return 0;
}


#ifndef CONFIG_APP_DRONE
/**
 * @brief interface function - read mcu GPS data and send to avi file
 * @param sdrecord info structure
 */
static void chk_gpsinfo(recordinfo_t *info)
{
	unsigned short sn;
	
	sn = get_gps_sn();
	if ((sn >= 1) && (info->gpscount != sn)) {
		info->gpscount = sn;
		AVIGPSINFO_t *gpsinfo;
		gpsinfo = get_avi_gps_info();
		info->pRecord_info->GPS_info.ulChunkID = 1;
		info->pRecord_info->GPS_info.ucGPSStatus = 1;
		info->pRecord_info->GPS_info.ssLonDegInt = gpsinfo->ssLonDegInt;
		info->pRecord_info->GPS_info.ulLonDegDec = gpsinfo->ulLonDegDec;
		info->pRecord_info->GPS_info.ssLatDegInt = gpsinfo->ssLatDegInt;
		info->pRecord_info->GPS_info.ulLatDegDec = gpsinfo->ulLatDegDec;
		info->pRecord_info->GPS_info.usAltitude = gpsinfo->usAltitude;
		info->pRecord_info->GPS_info.usSpeed = gpsinfo->usSpeed;
		info->pRecord_info->GPS_info.ucGpsHour = gpsinfo->ucGpsHour;
		info->pRecord_info->GPS_info.ucGpsMinute = gpsinfo->ucGpsMinute;
		info->pRecord_info->GPS_info.ucGpsSecond = gpsinfo->ucGpsSecond;
		info->pRecord_info->GPS_info.usGpsYear = gpsinfo->usGpsYear;
		info->pRecord_info->GPS_info.ucGpsMonth = gpsinfo->ucGpsMonth;
		info->pRecord_info->GPS_info.ucGpsDay = gpsinfo->ucGpsDay;
	} else {
		info->pRecord_info->GPS_info.ulChunkID = 0;
		info->pRecord_info->GPS_info.ucGPSStatus = 0;
	}
}

/**
 * @brief interface function - read mcu GSensor data and send to avi file
 * @param sdrecord info structure
 */
static void chk_gsensorinfo(recordinfo_t *info)
{
	unsigned short sn;
	
	sn = get_g_sensor_sn();
	if ((sn >= 1) && (info->gsensorcount != sn)) {
		info->gsensorcount = sn;
		AVIGSENSORINFO_t *gsensorinfo;
		gsensorinfo = get_avi_g_sensor_info();
		info->pRecord_info->GSENSOR_info.ulChunkID = 1;
		info->pRecord_info->GSENSOR_info.ucAcceRange = gsensorinfo->ucAcceRange;
		info->pRecord_info->GSENSOR_info.ucGVInt_X = gsensorinfo->ucGVInt_X;
		info->pRecord_info->GSENSOR_info.ulGVDec_X = gsensorinfo->ulGVDec_X;
		info->pRecord_info->GSENSOR_info.ucGVInt_Y = gsensorinfo->ucGVInt_Y;
		info->pRecord_info->GSENSOR_info.ulGVDec_Y = gsensorinfo->ulGVDec_Y;
		info->pRecord_info->GSENSOR_info.ucGVInt_Z = gsensorinfo->ucGVInt_Z;
		info->pRecord_info->GSENSOR_info.ulGVDec_Z = gsensorinfo->ulGVDec_Z;
	} else {
		info->pRecord_info->GSENSOR_info.ulChunkID = 0;
	}
}
#endif

/**
 * @brief interface function - send video data to middleware recording
 * @param IFrame is I frame or not
 * @param pFrame pointer for frame data
 * @param uiFrameSize frame size
 */
void recordvideo(unsigned char iframe, unsigned char *pframe, unsigned int uiframeSize, struct timeval tval)
{
	if (((sdrecord_info != NULL) && (sdrecord_info->pRecord_info != NULL)) && (sdrecord_info->recordclose != 1)) {
		if (iframe == 1) {
#ifndef CONFIG_APP_DRONE
			chk_gpsinfo(sdrecord_info);
			chk_gsensorinfo(sdrecord_info);
#endif
		}
		record_video(sdrecord_info->pRecord_info, iframe, pframe, uiframeSize, tval);
	}
}

/**
 * @brief interface function - send audio data to middleware recording
 * @param pFrame pointer for each frame data
 * @param uiFrameSize frame size
 */
void recordaudio(unsigned char *pframe, unsigned int uiframeSize, struct timeval tval)
{
	if (((sdrecord_info != NULL) && (sdrecord_info->pRecord_info != NULL)) && (sdrecord_info->recordclose != 1))
		record_audio(sdrecord_info->pRecord_info, pframe, uiframeSize, tval);
}

/**
 * @brief interface function - get recordinfo pointer for protect function use
 * @return if have,return recordinfo_t pointer
 */
recordinfo_t *get_rec_pointer(void)
{
	if ((sdrecord_info != NULL) && (sdrecord_info->pRecord_info != NULL))
		return sdrecord_info;
	else
		return NULL;
}

/**
 *@brief interface function - init_record_task
 * @return 0 :success , -1:Taskcreate Fail
 */
int init_record_task(void)
{
	int intbuf;
	
	snx_nvram_integer_get(NVRAM_SPACE, NVRAM_RECORD_UPBD, &intbuf);
	if (intbuf <= 0)
		return 0;
	
	if (pdPASS != xTaskCreate(record_flow, "record_flow", STACK_SIZE_6K, NULL, PRIORITY_TASK_APP_REC_FLOW, NULL)) {
		REC_PRINT(SYS_ERR, "could not create record task\n");
		return (-1);
	}
	
	return 0;
}

/**
 *@brief interface function - Uninit SDRecordInfo structure
 *
 */
static void rec_info_uninit(recordinfo_t **info)
{
	safeFree(*info);
}

/**
 *@brief interface function - init SDRecordInfo structure
 * @return recordinfo_t pointer  if success
 */
static recordinfo_t *rec_info_init(void)
{
	recordinfo_t *info;
	int ret = 0;
	struct usr_config *pUserCfg = NULL;

	if ((ret = get_usr_config(&pUserCfg)) != 0) {
		REC_PRINT(SYS_ERR, "get usr config failed\n");
		goto finally;
	}

	if (!(info = (recordinfo_t *)pvPortMalloc(sizeof(recordinfo_t), GFP_KERNEL, MODULE_APP))) {
		REC_PRINT(SYS_ERR, "sd recrod info init fail\n");
		goto finally;
	}
	
	memset(info, 0, sizeof(recordinfo_t));

	info->pRecord_info = NULL;
	info->sched_p = NULL;
	info->lastsecond = 0;
	info->recordstatus = 0;
	info->recordclose = 0;
	info->gpscount = 0;
	info->gsensorcount = 0;
	info->sdcardisfull = 0;
	info->sdcardseed = 0;
	info->rm_queue = xQueueCreate(RM_MAX_QUEUE_ITEM, sizeof(rm_info_t));
	if (NULL == info->rm_queue) {
		REC_PRINT(SYS_ERR, "queue rm_queue create fail\n");
		goto finally;
	}
		
	if (pdPASS != xTaskCreate(rm_file_task, "rm_file_task", STACK_SIZE_4K, NULL, PRIORITY_TASK_APP_REC_FLOW, &(info->rm_file_task))) {
		REC_PRINT(SYS_ERR, "cannot create rm_file_task\n");
		goto finally1;
	}
	
	memset(info->schedpath, 0x00, sizeof(info->schedpath));
	snprintf(info->schedpath, sizeof(info->schedpath), "%s", pUserCfg->rec_path);
	return info;

finally1:
	if (info->rm_queue != NULL)
		vQueueDelete(info->rm_queue);
finally:
	if (info != NULL)
		rec_info_uninit(&info);
	return NULL;
}

/**
 *@brief interface function - setting middleware wribuffer size by resolution
 * @param RecWirteBufferInitInfo_t write_buf_size:middleware writebuffer setting ,write_unit_to_file: write unit size to file once;
 * @param pv_param videorecord information
 */

static void set_rec_writebuffer(RecWirteBufferInitInfo_t *writebufferparam, video_param_t *pv_param)
{
	if ((pv_param->width >= FHD_WIDTH) && (pv_param->height >= FHD_HEIGHT))
#if defined(CONFIG_SYSTEM_PLATFORM_SN98660) || defined(CONFIG_SYSTEM_PLATFORM_SN98661)
		writebufferparam->write_buf_size = 128 * 40 * 1024;
#else
#if defined(CONFIG_SYSTEM_PLATFORM_SN98672)
		writebufferparam->write_buf_size = 128 * 20 * 1024;
#else
		writebufferparam->write_buf_size = 128 * 20 * 1024;
#endif
#endif
	else if ((pv_param->width >= HD_WIDTH) && (pv_param->height >= HD_HEIGHT))
		writebufferparam->write_buf_size = 128 * 12 * 1024;
	else if ((pv_param->width >= VGA_WIDTH) && (pv_param->height >= VGA_HEIGHT))
		writebufferparam->write_buf_size = 128 * 10 * 1024;
	else
		writebufferparam->write_buf_size = 128 * 10 * 1024;

	writebufferparam->write_unit_to_file = 0x20000;
}

/**
 * @brief interface function - sched record init
 * @param recordinfo_t  structure
 * @return 1 if record init success
 * @note  get video & audio param and init middleware record
 */
static int rec_record_init(recordinfo_t *info)
{
	int record_type;
	int ret = 0;
	struct usr_config *pUserCfg = NULL;

	if ((ret = get_usr_config(&pUserCfg)) != 0) {
		REC_PRINT(SYS_ERR, "get usr config failed\n");
		goto finish;
	}
	
	video_param_t vparam;
#ifndef CONFIG_APP_DRONE
	audio_param_t aparam;
#endif

	mf_video_get_rec_param(&vparam);
	info->RecParam.vdo.width = vparam.width;
	info->RecParam.vdo.height = vparam.height;
	info->RecParam.vdo.ucFps = vparam.ucFps;
	info->RecParam.vdo.uiBps = vparam.uiBps;
	info->RecParam.vdo.ucScale = vparam.ucScale;
	info->RecParam.vdo.ucStreamMode = vparam.ucStreamMode;
#ifndef CONFIG_APP_DRONE
	mf_audio_get_param(&aparam);
	info->RecParam.ado.uiFormat = aparam.uiFormat;
	info->RecParam.ado.uiSampleRate = aparam.uiSampleRate;
	info->RecParam.ado.ucBitsPerSample = aparam.ucBitsPerSample;
	info->RecParam.ado.uiPacketSize = aparam.uiPacketSize;
	info->RecParam.ado.uiBitRate = aparam.uiBitRate;
#else
	info->RecParam.ado.uiFormat = 1 << 20;		// AUD_FORMAT_AAC;
	info->RecParam.ado.uiSampleRate = 11025;
	info->RecParam.ado.ucBitsPerSample = 16;
	info->RecParam.ado.uiPacketSize = 2048;
	info->RecParam.ado.uiBitRate = 15999;
#endif

	info->RecParam.max_record_len = info->sched_p->sd_sched_record_interval;
	info->type = T_RECORD;
	set_rec_writebuffer(&(info->RecParam.writebufferparam), &vparam);

#if LIMIT_PLAYBACK_AND_DOWNLOAD_CONNECTION
	if ((info->RecParam.vdo.width == FHD_WIDTH) && (info->RecParam.vdo.height == FHD_HEIGHT))
		set_max_dwnlod_and_pb_max_conn_num(1);
	else if ((info->RecParam.vdo.width == HD_WIDTH) && (info->RecParam.vdo.height == HD_HEIGHT))
		set_max_dwnlod_and_pb_max_conn_num(2);
	else
		set_max_dwnlod_and_pb_max_conn_num(4);
#endif

	if (snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_TYPE, &record_type) != NVRAM_SUCCESS) {
		REC_PRINT(SYS_ERR, "get record type from nvram failed!\n");
		goto finish;
	}

	REC_PRINT(SYS_DBG, "record_type=%d\n", record_type);

#ifndef CONFIG_APP_DRONE
	ret = record_init(&(info->pRecord_info), &info->RecParam, 1, record_type, (AUDIO_CAP|GPS_CAP|GSENSOR_CAP), rec_finishedfile_addlen, update_seed, MAX_PRE_TIME, T_RECORD);
#else
	ret = record_init(&(info->pRecord_info), &info->RecParam, 1, record_type, 0, rec_finishedfile_addlen, update_seed, MAX_PRE_TIME, T_RECORD);
#endif
	if (ret == pdPASS) {
		record_set_prefix_name(info->pRecord_info, pUserCfg->rec_prefix);
		if (info->fileformat == RECORD_TIMEFORMAT) {
			record_set_filename_format(info->pRecord_info, RECORD_TIMEFORMAT);
		} else if (info->fileformat == RECORD_COUNTFORMAT) {
			record_set_filename_format(info->pRecord_info, RECORD_COUNTFORMAT);
		}
	} else
		goto finish;

finish:
	return ret;
}

/**
 * @brief interface function - read record param for nvram
 * @param recordinfo_t  structure
 * @return 0 if readnvram OK
 */
static int rec_nvram(recordinfo_t *sdrecord_info)
{
	sdrecord_info->sched_p = rec_nvraminit();
	if (sdrecord_info->sched_p == NULL) {
		REC_PRINT(SYS_ERR, "sd nvram init fail\n");
		goto finally;
	}
	
	get_record_param(sdrecord_info->sched_p);
	sdrecord_info->fileformat = rec_fileformat_get();
	
	return 0;
finally:
	if (sdrecord_info->sched_p != NULL)
		rec_nvram_uninit(&sdrecord_info->sched_p);
	return (-1);
}

/**
 * @brief interface function - user app diable recording
 */
void user_diable_rec(void)
{
	REC_PRINT(SYS_DBG, "user diable record\n");
#if MSG_TONE_ENABLE
	aac_tone_play(NVRAM_END_REC);
#endif
	userdisablerecord = 1;
	mf_set_record(0);	// H.264 encode disable
}

/**
 * @brief interface function - user app enable recording
 */
void user_enable_rec(void)
{
	REC_PRINT(SYS_DBG, "user enable record\n");
	userdisablerecord = 0;
	mf_set_record(1);	// H.264 encode enable
}

/**
 * @brief interface function - check record enable
 @param Forcerecord : 0:daible 1:enable
 */
int chk_rec_enable(void)
{
	if (userdisablerecord == 1)
		return 0;
	else
		return 1;
}

/**
 * @brief interface function - suspend recording to start
 @param Forcerecord : 1:force record for mcu  0:not force record
 */
void schedrec_suspend_restart(int force_record)
{
	if ((sdrecord_info != NULL) && (sdrecord_info->sched_p != NULL))
		sdrecord_info->sched_p->sd_sched_internal_en = 1;

	if (force_record == 0) {
		if (userdisablerecord == 0) {
			mf_set_record(1);	// H.264 encode enable
		}
	} else if (force_record == 1) {
		mf_set_record(1);	// H.264 encode enable
		userdisablerecord = 0;
	}
}

/**
 * @brief interface function - suspend recording and H.264 encode disable
 */
void schedrec_suspend(void)
{
	REC_PRINT(SYS_DBG, "sched record suspend\n");
	if ((sdrecord_info != NULL) && (sdrecord_info->sched_p != NULL))
		sdrecord_info->sched_p->sd_sched_internal_en = 0;
	mf_set_record(0);	// H.264 encode disable
}

/**
 * @brief interface function - set flag to create or release wb mem
 */

void schedrec_set_wb_mem(int flag)
{
	if(sdrecord_info != NULL) {
		if(flag == 1)
			record_writebuf_create(sdrecord_info->pRecord_info);
		else
			record_writebuf_release(sdrecord_info->pRecord_info);
			
	}
}

/**
 * @brief interface function - checking now record is running
 *@ return  1:recording is running
 *@            0:recording is not running
 *@          -1:reocrding task not init
 */
int schedrec_state(void)
{
	if ((sdrecord_info == NULL) || (sdrecord_info->sched_p == NULL))
		return (-1);
	else if ((sdrecord_info->sched_p->sd_sched_internal_en == 0) || (userdisablerecord == 1) || (get_sd_umount_err() == 1) || (sdrecord_info->sdcardisfull == 1))
		return 0;
	else if (sdrecord_info->sched_p->sd_sched_internal_en == 1)
		return 1;
	else
		return (-1);
}

/**
 * @brief interface function - close sched record task
 */
void schedrec_close(void)
{
	schedrec_suspend();
	if (sdrecord_info != NULL)
		sdrecord_info->recordclose = 1;
}

/**
 * @brief interface function - report space for schedule record is full or not
 * @return 1:is full
 */
int schedrec_get_isfull(void)
{
	return sdrecord_info->sdcardisfull;
}


/**
 * @brief interface function -ReadSDCardDataState
 *@ return  -1:reocrding task not init
 *                0: sdcard inset , and read "/sonix/record/..." is running
 *@             1:sdcard insert , and reading "/sonix/record/..."  total file is finished
 *@
 */
int read_card_state(void)
{
	if ((sdrecord_info == NULL) || (sdrecord_info->sched_p == NULL))
		return (-1);
	else
		return sdrecord_info->readfilelistok;
}


/**
 *@brief interface function -del record file  for app user use delete
 @ return  0: can delete file  -1: not can delete file

*/
int del_rec_recordfile(const char *delname)
{
	char target[LEN_FILENAME];
	int ret = 0;
	struct usr_config *pUserCfg = NULL;

	if ((ret = get_usr_config(&pUserCfg)) != 0) {
		REC_PRINT(SYS_ERR, "get usr config failed\n");
		return (-1);
	}

	memset(target, 0x00, sizeof(target));
	snprintf(target, sizeof(target), "%s/%s", pUserCfg->rec_path, delname);
	if ((sdrecord_info != NULL)) {
		ret = schedrec_state();
		if (ret == 0) {
			rec_filenode_del(sdrecord_info->type, delname);
			rec_size_del(target, &sdrecord_info->recordusedsize);
			rec_thumbnail_del(sdrecord_info->type, target);
			return 0;
		} else if (ret == 1) {
			if (strcmp(target, record_get_cur_file_name(sdrecord_info->pRecord_info)) != 0) {
				rec_filenode_del(sdrecord_info->type, delname);
				rec_size_del(target, &sdrecord_info->recordusedsize);
				rec_thumbnail_del(sdrecord_info->type, target);
				return 0;
			} else
				return (-1);
		} else
			return (-1);
	}
	return (-1);
}


/**
 * @brief interface function - checking now record is running
 *@ , if runnig , stop record and  add file to list  and  add size
 *@
 */
void rec_stop_changestate(recordinfo_t *sdrecord_info)
{
	if (chk_rec_status(sdrecord_info, RECORD_SCHED_ACTIVE)) {
		REC_PRINT(SYS_DBG, "RECORD_SCHED_ACTIVE\n");
		record_set_stop(sdrecord_info->pRecord_info, 1);
		rec_filenode_add(sdrecord_info->type, record_get_last_file_name(sdrecord_info->pRecord_info), &sdrecord_info->recordusedsize);
		del_rec_status(sdrecord_info, RECORD_SCHED_ACTIVE);
	}
	
	del_rec_status(sdrecord_info, RECORD_START_RUNNING);
}

/**
 * @brief interface function - checking app jsoncmd setting value for record
 *@ param sdrecord info structure
 *@
 */
void chk_userparam(recordinfo_t *sdrecord_info)
{
	int cycle;
	int level;

	if (chk_record_task_close() == 1) {
		sdrecord_info->recordclose = 1;
		sdrecord_info->sched_p->sd_sched_internal_en = 0; //disable record
	}
	if (chk_record_cycle_bit()) {
		if (snx_nvram_integer_get(NVRAM_RECORD, NVRAM_RECORD_SCHED_CYCLE, &cycle) != NVRAM_SUCCESS) {
			REC_PRINT(SYS_ERR, "get record schedule cycle from nvram failed!\n");
			return;
		}
		REC_PRINT(SYS_DBG, "cycle=%d\n", cycle);
		sdrecord_info->sched_p->sd_sched_record_cycle = cycle;
		set_record_cycle_bit(0);
	}
#ifdef CONFIG_MODULE_AUDIO_SUPPORT
	if (chk_record_audio_voice_bit()) {
		int levelchangetoset;
		if (snx_nvram_integer_get(NVRAM_PKG_AUDIO_ISP, NVRAM_CFG_AUDIO_VOICE, &level) != NVRAM_SUCCESS) {
			REC_PRINT(SYS_ERR, "get record audio voice from nvram failed!\n");
			return;
		}
		if ((level < 0) || (level > 100))
			levelchangetoset = 20; //default
		if (level == 0)
			levelchangetoset = 0;
		else if ((0 < level) && (level <= 3))
			levelchangetoset = 1;
		else if (level > 3) {
			levelchangetoset = (int)level / 3;
			if (levelchangetoset > 31)
				levelchangetoset = 31;
		}
		REC_PRINT(SYS_DBG, "level change to set=%d\n", levelchangetoset);
		mf_audio_set_record_voice(levelchangetoset);
		set_record_audio_voice_bit(0);
	}
#endif
}

/**
 * @brief interface function - checking record full , delete file process
 *@ param sdrecord info structure
 *@
 */
void rec_full_deloldestfile(recordinfo_t *sdrecord_info)
{
	char removefile[LEN_FILENAME] = {0};
	char target[LEN_FILENAME] = {0};
	int ret;
	struct usr_config *pUserCfg = NULL;

	if ((ret = get_usr_config(&pUserCfg)) != 0) {
		REC_PRINT(SYS_ERR, "get usr config failed\n");
	}
	
	memset(target, 0x00, sizeof(target));
	memset(removefile, 0x00, sizeof(removefile));

	/* get the oldest file name from file manager */
	ret = snx_fm_get_first_file(sdrecord_info->type, removefile, sizeof(removefile), NULL);
	/***
	if (ret == -1) {
		if (chk_rec_status(sdrecord_info, RECORD_SCHED_ACTIVE)) {
			record_set_stop(sdrecord_info->pRecord_info, 1);
			rec_filenode_add(sdrecord_info->type, record_get_last_file_name(sdrecord_info->pRecord_info), &sdrecord_info->recordusedsize);
			REC_PRINT(SYS_WARN, "sdcard is full but no oldest file\n");
			del_rec_status(sdrecord_info, RECORD_SCHED_ACTIVE);
			sdrecord_info->sdcardisfull = 1;
		}
	}***/ 
	if (ret == 0) {
		snprintf(target, sizeof(target), "%s/%s", pUserCfg->rec_path, removefile);
		if((0 == is_current_upload_file(target, UPLOAD_BG)) || (0 == is_current_upload_file(target, UPLOAD_FG))) {
			rm_info_t rm_info;
			memset(&rm_info, 0, sizeof(rm_info_t));
			strcpy(rm_info.rm_name_path, target);
			strcpy(rm_info.rn_name, removefile);
			xQueueSendToBack(sdrecord_info->rm_queue, &rm_info , portMAX_DELAY);
		}
	} else if (ret == FM_LISTWAIT) {
		//do nothing..
	} else {
		REC_PRINT(SYS_ERR, "ret is not correct value=%d\n", ret);
	}
}


static bool force_next_file_flag;

/*
  move to a new recording file now. This is called on disarm, to
  prevent corrupt files on landing when user removes battery
 */
void rec_sched_next_file(void)
{
    force_next_file_flag = true;
}

/**
 * @brief interface function - checking record start now ,and when to change record file
 *@ param sdrecord info structure
 *@
 */
void chk_rec_start_changefile(recordinfo_t *sdrecord_info)
{
	if (chk_rec_status(sdrecord_info, RECORD_SCHED_ACTIVE)) {
		int currentfilesize = 0;
		RecFileInfo_t pCurFileInfo;
		RecFileInfo_t pLastFileInfo;
		
		record_get_file_info(sdrecord_info->pRecord_info, &pCurFileInfo, &pLastFileInfo);
		if (sdrecord_info->sched_p->sd_sched_record_interval > SD_MAX_RECORD_LENGTH) {
			currentfilesize = rec_filesize_get(record_get_cur_file_name(sdrecord_info->pRecord_info));
		}
				
		if ((pCurFileInfo.VdoFrameNum >= ((sdrecord_info->sched_p->sd_sched_record_interval) * (sdrecord_info->RecParam.vdo.ucFps))) || 
			(currentfilesize > MAXFILESIZE) || force_next_file_flag) {
                        force_next_file_flag = false;
			record_set_next(sdrecord_info->pRecord_info, 1);
			mf_video_h264_set_iframe_for_record();
			rec_filenode_add(sdrecord_info->type, record_get_last_file_name(sdrecord_info->pRecord_info), &sdrecord_info->recordusedsize);
			rec_snapshot_query(SCHED_RECORD, record_get_cur_file_name(sdrecord_info->pRecord_info));
		}
	} else {
		if (exists(sdrecord_info->schedpath) == 0) {
			mkdir(sdrecord_info->schedpath);
		}
		record_set_path(sdrecord_info->pRecord_info, sdrecord_info->schedpath);
		record_set_start(sdrecord_info->pRecord_info, 1);
		rec_snapshot_query(SCHED_RECORD, record_get_cur_file_name(sdrecord_info->pRecord_info));
		add_rec_status(sdrecord_info, RECORD_SCHED_ACTIVE);
#if MSG_TONE_ENABLE
		aac_tone_play(NVRAM_START_REC);
#endif
		REC_PRINT(SYS_DBG, "Schedule record start\n");
	}
	add_rec_status(sdrecord_info, RECORD_START_RUNNING);
}


void set_card_task_state(int state){
	card_info.task_state = state;
}

int chk_card_task_state(void){
	return card_info.task_state;
}


/**
 * @brief interface function - SDCardCheck
 * @note check sdcard folder data for add to list
 */
void import_record_task(void *pvParameters)
{
	int result = 0;
	char c;
	
	while(card_info.task_state) {
		xQueueReceive(card_info.chk_card_queue, (void *)&c, portMAX_DELAY);
		if(card_info.task_state == IMPORT_CARD_INACTIVE )
			break;
		
		if(sdrecord_info) {
			result = rec_import_files(sdrecord_info->type);
			sdrecord_info->readfilelistok = 1;
		}
	}
	
	card_info.task_state = IMPORT_CARD_FINISHED;
	vTaskDelete(NULL);
}


/**
 *@brief interface function - SDCardCheck Task
 */
void chk_card_task_init(void)
{
	memset(&card_info, 0, sizeof(chk_card_file_info_t));
	card_info.chk_card_queue = xQueueCreate(1, sizeof(char));
	if(NULL == card_info.chk_card_queue) {
		REC_PRINT(SYS_ERR, "Create SDcard chk Queue Failed\n");	
		goto fail1;
	}
	card_info.task_state = IMPORT_CARD_ACTIVE;
	if (pdPASS != xTaskCreate(import_record_task, "import_files_task", STACK_SIZE_6K, NULL, PRIORITY_TASK_APP_REC_FLOW, &card_info.task_chk_card)) {
		
		REC_PRINT(SYS_ERR, "SDCardCheckList task create fail\n");
		card_info.task_state = IMPORT_CARD_INACTIVE;
		goto fail2;
	}
	
	REC_PRINT(SYS_DBG, "Init chk_card_task Success\n");
	return;

fail2:
	if(card_info.chk_card_queue) {
		vQueueDelete(card_info.chk_card_queue);
		card_info.chk_card_queue = NULL;
	}	
fail1:
	return;
}

void chk_card_task_uninit(void)
{
	set_card_task_state(IMPORT_CARD_INACTIVE);
	{
		int w = 10; //anydata;
		xQueueSendToBack(card_info.chk_card_queue, (void *)&w, 0);
	}
	
	while(1){
		if(chk_card_task_state() == IMPORT_CARD_FINISHED)
			break;
		vTaskDelay(20 / portTICK_RATE_MS);
	}
	
	if(card_info.chk_card_queue) {
		vQueueDelete(card_info.chk_card_queue);
		card_info.chk_card_queue = NULL;
	}
	REC_PRINT(SYS_DBG, "Uninit chk_card_task Success\n");
}
/**
 * @brief interface function-rm oldest file
 * @note it is too long time to rm file , so move to task to handle
 */

static void rm_file_task(void *pvParameters)
{
	rm_info_t rm_info;
	
	while (1) {
		xQueueReceive(sdrecord_info->rm_queue, &rm_info, portMAX_DELAY );
		REC_PRINT(SYS_DBG, "remove %s\n", rm_info.rm_name_path);
		rec_filenode_del(sdrecord_info->type, rm_info.rn_name);
		rec_size_del(rm_info.rm_name_path, &sdrecord_info->recordusedsize);
		fs_cmd_rm(rm_info.rm_name_path);
		rec_thumbnail_del(sdrecord_info->type, rm_info.rm_name_path);
	}
}

/**
 *@brief interface function - RecordTaskFlow
 *@ note init sdinfo,init record, init nvram, and check sdcard insert
 */
static void record_flow(void *pvParameters)
{
	int  ret;
	unsigned int sensor_cap;
	
	sdrecord_info = rec_info_init();
	sensor_cap = mf_video_resmap_get_record_capability();
	/*****init sdinfo*****/
	if (sdrecord_info == NULL) {
		REC_PRINT(SYS_ERR, "sd_info_init fail\n");
		cardsize_uinit();
		goto end;
	}

	/*****nvramcheck*****/
	if (rec_nvram(sdrecord_info)) {
		REC_PRINT(SYS_ERR, "nvram_init fail\n");
		goto finally;
	}

	/*****init rec*****/
	ret = rec_record_init(sdrecord_info);
	if (ret == pdFAIL) {
		REC_PRINT(SYS_ERR, "record_init fail\n");
		goto finally;
	}

//	rec_filenode_init(sdrecord_info->type, TRUE);
	rec_filenode_update(sdrecord_info->type);

	mf_video_set_record_cb(recordvideo);
#ifndef CONFIG_APP_DRONE
	mf_audio_set_record_cb(recordaudio);
	if (firststarttask == 0) {
		firststarttask = 1;
		get_date(&dev_time);
	}
#endif

#if CONFIG_APP_DRONE
	if (sdrecord_info->sched_p->sd_sched_record_en == 1)
		user_enable_rec();
	else
		user_diable_rec();
#endif

	REC_PRINT(SYS_DBG, "TaskRecMainFlow INIT OK\n");
	sdaumt_record_info = get_automount_info();
	record_is_running(1);
	for (;;) {
		if (sdrecord_info->recordclose == 1) {
			goto finally;
		}
		
		cardsize_init();
		sdrecord_info->readfilelistok = 0;
		sdrecord_info->sdcardseed = 0;
		if (xSemaphoreTake(sdaumt_record_info->SdRecordMutex, portMAX_DELAY) == pdTRUE) {
#if MSG_TONE_ENABLE
			aac_tone_play(NVRAM_SD_INJECT);
#endif
			add_rec_status(sdrecord_info, RECORD_START_RUNNING);
#ifndef CONFIG_APP_DRONE
			int updateseed;
#endif
			if (chk_record_task_close() == 1) {
				del_rec_status(sdrecord_info, RECORD_START_RUNNING);
				goto finally;
			}
			
			record_writebufreset(sdrecord_info->pRecord_info);
			sdrecord_info->sdcardisfull = 0;
			chk_cardsize();
			sdrecord_info->recordusedsize = get_schedrec_usedsize();
			//parsing card and import filenode
			int w = 10; //anydata;
			xQueueSendToBack(card_info.chk_card_queue, (void *)&w, 0);


#if defined(CONFIG_MODULE_RTC_SUPPORT) && !defined(CONFIG_APP_DRONE)
			updateseed = check_update_seed(&dev_time);
			if (updateseed == 1)
				update_seed();
#else
			start_new_seed();
#endif
			if (get_readcard_finish() == 1) {
				long long space;
				space = get_schedfolder_canusesize();
				if ((space - sdrecord_info->recordusedsize < SD_RECORD_RESERVED) && 
					(sdrecord_info->sched_p->sd_sched_record_cycle == 0)) {
					REC_PRINT(SYS_INFO, "SD Card Record is Full & No Cycle\n");
					sdrecord_info->sdcardisfull = 1;
				}
				sched_rec_flow(sdrecord_info);
			}
		}
	}

finally:
	mf_video_set_record_cb(NULL);
#ifndef CONFIG_APP_DRONE
	mf_audio_set_record_cb(NULL);
#endif
	if (sdrecord_info->pRecord_info != NULL) {
		record_uninit(sdrecord_info->pRecord_info);
		sdrecord_info->pRecord_info = NULL;
	}
	cardsize_uinit();
	snx_fm_release_filelist(sdrecord_info->type, NULL);
	if (sdrecord_info->sched_p != NULL)
		rec_nvram_uninit(&sdrecord_info->sched_p);
	if (sdrecord_info != NULL) {
		if (sdrecord_info->rm_file_task != NULL) {
			vTaskDelete(sdrecord_info->rm_file_task);
			sdrecord_info->rm_file_task = NULL;
		}
		if (sdrecord_info->rm_queue != NULL) {
			vQueueDelete(sdrecord_info->rm_queue);
			sdrecord_info->rm_queue = NULL;
		}
		rec_info_uninit(&sdrecord_info);
	}

end:
	record_is_running(0);
	vTaskDelete(NULL);
}

int reclist_to_file(char *path)
{
	char target[LEN_FILENAME] = {0};
	int len = 0;
	int ret;
	struct usr_config *pUserCfg = NULL;

	if ((ret = get_usr_config(&pUserCfg)) != 0) {
		REC_PRINT(SYS_ERR, "get usr config failed\n");
		return (-1);
	}
	
	memset(target, 0x00, sizeof(target));
	snprintf(target, sizeof(target), "%s/%s", pUserCfg->rec_path, SD_RECORD_FILELIST);
	len = strlen(target);
	strncpy(path, target, len);
	if (sdrecord_info == NULL) {
		return (-1);
	} else if (is_current_upload_file(target, UPLOAD_FG) != 0) {
		REC_PRINT(SYS_ERR, "record filelist is downloading\n");
		return LIST_FILE_DOWNLOAD;
	} else {
		return rec_filelist_Createfile(T_RECORD, target, len);
	}
}

void rec_sched_incompletefile_hndl(char *filename)
{
	struct usr_config *pUserCfg;
	char target[128] = {0};
	int ret = 0;
	
	if ((ret = get_usr_config(&pUserCfg)) != 0) {
		REC_PRINT(SYS_ERR, "get usr config failed.\n");
		return;
	}
	/* remove incomplete file in MP4 format */
	snprintf(target, sizeof(target), "%s/%s", pUserCfg->rec_path, filename);
	REC_PRINT(SYS_DBG, "sdrecord_info->recordusedsize=%u KB\n",(int)(sdrecord_info->recordusedsize >> 10));
	rec_size_del(target, &sdrecord_info->recordusedsize);
	REC_PRINT(SYS_DBG, "after delete ,sdrecord_info->recordusedsize=%u KB\n",(int)(sdrecord_info->recordusedsize >> 10));
	fs_rm(target);
	rec_thumbnail_del(T_RECORD, target);
}
