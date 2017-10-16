/*********************************************************************************
* watch_task.c
*
* Implementation of watch task for check task close is or not
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
#include <nonstdlib.h>
#include <libmid_isp/snx_mid_isp.h>
#include <libmid_nvram/snx_mid_nvram.h>
#include <string.h>
#include <sys_clock.h>
#include <sys/time.h>
#include "watch_task.h"
#include "rec_schedule.h"
#include "../video/video_main.h"
#include "../video/snapshot.h"
#include "../audio/audio_main.h"
#include <libmid_automount/automount.h>
#include <libmid_rtsp_server/rtsp_server.h>
#include <generated/snx_sdk_conf.h>
#include "rec_common.h"

#include <libmid_usbd/mid_usbd.h>
#include <usb_device/usb_device.h>

#define WT_PRINT(level, fmt, args...) print_q(level, "[watchtask]%s: "fmt, __func__,##args)


static taskisrunning task_is_running;
static taskisclosing task_is_closing;
static taskisrestarting  task_is_restarting;
static settasktoclose set_task_to_close;
static int waitflag = 0;

extern int preview_use_isp0dup;
xSemaphoreHandle watchmutex;


static int chk_protect_function_enable(void)
{
	int intbuf;
	snx_nvram_integer_get(NVRAM_SPACE, NVRAM_PROTECT_UPBD, &intbuf);
	if (intbuf <= 0)
		return 0;
	return 1;
}

static int chk_timelapse_function_enable(void)
{
	int intbuf;
	snx_nvram_integer_get(NVRAM_SPACE, NVRAM_TIMELAPSE_UPBD, &intbuf);
	if (intbuf <= 0)
		return 0;
	return 1;
}

///////////////////////close/////////////////////////////
//protect
void set_protect_closeflag(void)
{
	if (chk_protect_function_enable() == 0)
		return;
	set_task_to_close.settaskclose |= setprotectcloseflag;
}

static int chk_protect_closeflag(void)
{
	return !!(set_task_to_close.settaskclose & setprotectcloseflag);
}

static void reset_protect_closeflag(void)
{
	set_task_to_close.settaskclose &= (~setprotectcloseflag);
}

//record
void set_record_closeflag(void)
{
	set_task_to_close.settaskclose |= setrecordcloseflag;
}

int chk_record_closeflag(void)
{
	return !!(set_task_to_close.settaskclose & setrecordcloseflag);
}

void reset_record_closeflag(void)
{
	set_task_to_close.settaskclose &= (~setrecordcloseflag);
}

//timelapse
void set_lapse_record_closeflag(void)
{
	if (chk_timelapse_function_enable() == 0)
		return ;
	set_task_to_close.settaskclose |= setlapsecloseflag;
}

int chk_lapse_record_closeflag(void)
{
	return !!(set_task_to_close.settaskclose & setlapsecloseflag);
}

void reset_lapse_record_closeflag(void)
{
	set_task_to_close.settaskclose &= (~setlapsecloseflag);
}

//isp0dup
void set_isp0dup_closeflag(void)
{
	set_task_to_close.settaskclose |= setisp0dupcloseflag;
}

int chk_isp0dup_closeflag(void)
{
	return !!(set_task_to_close.settaskclose & setisp0dupcloseflag);
}

void reset_isp0dup_closeflag(void)
{
	set_task_to_close.settaskclose &= (~setisp0dupcloseflag);
}

//isp0task
void set_isp0_closeflag(void)
{
	set_task_to_close.settaskclose |= setisp0closeflag;
}

int chk_isp0_closeflag(void)
{
	return !!(set_task_to_close.settaskclose & setisp0closeflag);
}

void reset_isp0_closeflag(void)
{
	set_task_to_close.settaskclose &= (~setisp0closeflag);
}

//isp1task
void set_isp1_closeflag(void)
{
	set_task_to_close.settaskclose |= setisp1closeflag;
}

int chk_isp1_closeflag(void)
{
	return !!(set_task_to_close.settaskclose & setisp1closeflag);
}

void reset_isp1_closeflag(void)
{
	set_task_to_close.settaskclose &= (~setisp1closeflag);
}

///////////////////////restart/////////////////////////////
//protect restart
void set_protect_restartflag(void)
{
	if (chk_protect_function_enable() == 0)
		return ;
	task_is_restarting.task_restart |= protectrestartflag;
}

int chk_protect_restartflag(void)
{
	return !!(task_is_restarting.task_restart & protectrestartflag);
}

void reset_protect_restartflag()
{
	task_is_restarting.task_restart &= (~protectrestartflag);
}

//record restart
void set_record_restartflag(void)
{
	task_is_restarting.task_restart |= recordrestartflag;
}

int chk_record_restartflag(void)
{
	return !!(task_is_restarting.task_restart & recordrestartflag);
}

void reset_record_restartflag(void)
{
	task_is_restarting.task_restart &= (~recordrestartflag);
}

//lapse restart
void set_lapse_record_restartflag(void)
{
	if (chk_timelapse_function_enable() == 0)
		return ;
	task_is_restarting.task_restart |= lapserestartflag;
}

int chk_lapse_record_restartflag(void)
{
	return !!(task_is_restarting.task_restart & lapserestartflag);
}

void reset_lapse_record_restartflag(void)
{
	task_is_restarting.task_restart &= (~lapserestartflag);
}

//dup restart
void set_isp0dup_restartflag(void)
{
	task_is_restarting.task_restart |= isp0duprestartflag;
}
int chk_isp0dup_restartflag(void)
{
	return !!(task_is_restarting.task_restart & isp0duprestartflag);
}

void reset_isp0dup_restartflag(void)
{
	task_is_restarting.task_restart &= (~isp0duprestartflag);
}


//isp0 restart
void set_isp0_restartflag(void)
{
	task_is_restarting.task_restart |= isp0restartflag;
}

int chk_isp0_restartflag(void)
{
	return !!(task_is_restarting.task_restart & isp0restartflag);
}

void reset_isp0_restartflag(void)
{
	task_is_restarting.task_restart &= (~isp0restartflag);
}

//isp1 restart

void set_isp1_restartflag(void)
{
	task_is_restarting.task_restart |= isp1restartflag;
}


int chk_isp1_restartflag(void)
{
	return !!(task_is_restarting.task_restart & isp1restartflag);
}

void reset_isp1_restartflag(void)
{
	task_is_restarting.task_restart &= (~isp1restartflag);
}

/**
* @brief interface function - set protect task running
* @param value 1:running 0:not running
*/
void protect_is_running(bool value)
{
	if (chk_protect_function_enable() == 0)
		return;
	task_is_running.bits.protectrunning = value;
	return;
}

/**
* @brief interface function - check protecttask is running
* @param value 1:running 0:not running
*/
int chk_protect_is_running(void)
{
	if (chk_protect_function_enable() == 0)
		return 1;
	return !!(task_is_running.task_running & protectrunningflag);
}

/**
* @brief interface function - set record task closing
* @param value 1:to closing  0:closing finish
*/
void set_protect_to_closing(bool value)
{
	if (chk_protect_function_enable() == 0)
		return ;
	task_is_closing.bits.protectclosing = value;
	return;
}

/**
* @brief interface function - check protecttask close ok
* @return 1 : to closing  0: close protecttask finish
*/
int chk_protect_task_close(void)
{
	//if(chk_protect_function_enable()==0)
	//	return 1;
	if (task_is_closing.task_closing & protectclosingflag) {
#ifndef CONFIG_APP_DRONE
		set_protect_to_closing(0);
#endif
		return 1;
	}
	return 0;
}

/**
* @brief interface function - wait  protecttask close finish
* @param value 0:close finish  -1:timeout
*/
int wait_protect_task_closed(void)
{
	struct timeval cur_tv;
	struct timeval las_tv;
	if (chk_protect_function_enable() == 0)
		return 0 ;
	gettimeofday(&las_tv, NULL);
	while (chk_protect_is_running() == 1) {
		gettimeofday(&cur_tv, NULL);
		if ((cur_tv.tv_sec - las_tv.tv_sec) >= 30) {
			return -1;
		}
		vTaskDelay(50 / portTICK_RATE_MS);
	}
	WT_PRINT(SYS_INFO, "protect task close\n");
	return 0;
}



/**
* @brief interface function - set record task running
* @param value 1:running 0:not running
*/
void record_is_running(bool value)
{
	task_is_running.bits.recordrunning = value;
	return;
}

/**
* @brief interface function - check recordtask is running
* @param value 1:running 0:not running
*/
int chk_record_is_running(void)
{
	return !!(task_is_running.task_running & recordrunningflag);
}

/**
* @brief interface function - set record task closing
* @param value 1:to closing  0:closing finish
*/
void set_record_to_closing(bool value)
{
	task_is_closing.bits.recordclosing = value;
	return;
}

/**
* @brief interface function - check recordttask close ok
* @return 1 : to closing  0: close recordtask finish
*/
int chk_record_task_close(void)
{
	if (task_is_closing.task_closing & recordclosingflag) {
		set_record_to_closing(0);
		return 1;
	}
	return 0;
}

/**
* @brief interface function - wait  Recordtask close finish
* @param value 0:close finish  -1:timeout
*/
int wait_record_task_closed(void)
{
	struct timeval cur_tv;
	struct timeval las_tv;

	gettimeofday(&las_tv, NULL);
	while (chk_record_is_running() == 1) {
		gettimeofday(&cur_tv, NULL);
		if ((cur_tv.tv_sec - las_tv.tv_sec) >= 30) {
			return -1;
		}
		vTaskDelay(50 / portTICK_RATE_MS);
	}
	WT_PRINT(SYS_INFO, "record task close\n");
	return 0;
}

/**
* @brief interface function - set  lapse record task running
* @param value 1:running 0:not running
*/
void lapse_is_running(bool value)
{
	if (chk_timelapse_function_enable() == 0)
		return ;

	task_is_running.bits.lapserunning = value;
	return;
}

/**
* @brief interface function - check lapse record task is running
* @param value 1:running 0:not running
*/
int chk_lapse_is_running(void)
{
	if (chk_timelapse_function_enable() == 0)
		return 1;

	return !!(task_is_running.task_running & lapserunningflag);
}

/**
* @brief interface function - set lapse record task closing
* @param value 1:to closing  0:closing finish
*/
void set_lapse_to_closing(bool value)
{
	if (chk_timelapse_function_enable() == 0)
		return ;

	task_is_closing.bits.lapseclosing = value;
	return;
}

/**
* @brief interface function - check lapse record ttask close ok
* @return 1 : to closing  0: close recordtask finish
*/
int chk_lapse_task_close(void)
{
	if (task_is_closing.task_closing & lapseclosingflag) {
		set_lapse_to_closing(0);
		return 1;
	}
	return 0;
}

/**
* @brief interface function - wait  Recordtask close finish
* @param value 0:close finish  -1:timeout
*/
int wait_lapse_task_closed(void)
{
	struct timeval cur_tv;
	struct timeval las_tv;
	if (chk_timelapse_function_enable() == 0)
		return 0;
	gettimeofday(&las_tv, NULL);
	while (chk_lapse_is_running() == 1) {
		gettimeofday(&cur_tv, NULL);
		if ((cur_tv.tv_sec - las_tv.tv_sec) >= 30) {
			return -1;
		}
		vTaskDelay(50 / portTICK_RATE_MS);
	}
	WT_PRINT(SYS_INFO, "lapse record task close\n");
	return 0;
}

/**
* @brief interface function - set isp0 task running
* @param value 1:running 0:not running
*/
void isp0_is_running(bool value)
{
	task_is_running.bits.isp0running = value;
	return;
}

/**
* @brief interface function - check isp0task is running
* @param value 1:running 0:not running
*/
int chk_isp0_is_running(void)
{
	return !!(task_is_running.task_running & isp0runningflag);
}

/**
* @brief interface function - set isp0 task closing
* @param value 1: to closing  0:closing finish
*/
void set_isp0_to_closing(bool value)
{
	task_is_closing.bits.isp0closing = value;
	return;
}

/**
* @brief interface function - check isp0 task  close ok
* @return 1 : to closing  0: close recordtask finish
*/
int chk_isp0_task_close(void)
{
	if (task_is_closing.task_closing & isp0closingflag) {
		set_isp0_to_closing(0);
		return 1;
	}
	return 0;
}


/**
* @brief interface function - set isp1 dup closing
* @param value 1: to closing  0:closing finish
*/
void set_isp0dup_to_closing(bool value)
{
	task_is_closing.bits.isp0dup1closing = value;
	return;
}

/**
* @brief interface function - check isp0 dup  close ok
* @return 1 : to closing  0: close recordtask finish
*/
int chk_isp0dup_task_close(void)
{
	if (task_is_closing.task_closing & isp0dupclosingflag) {
		set_isp0dup_to_closing(0);
		return 1;
	}
	return 0;
}

/**
* @brief interface function - wait  isptask0 close finish
* @param value 0:close finish  -1:timeout
*/

int wait_isp0_task_closed(void)
{
	struct timeval cur_tv;
	struct timeval las_tv;
	gettimeofday(&las_tv, NULL);
	while (chk_isp0_is_running() == 1) {
		gettimeofday(&cur_tv, NULL);
		if ((cur_tv.tv_sec - las_tv.tv_sec) >= 30) {
			return -1;
		}
		vTaskDelay(50 / portTICK_RATE_MS);
	}
	WT_PRINT(SYS_INFO, "isp0 task close\n");
	return 0;
}

/**
* @brief interface function - wait  protecttask close finish
* @param value 0:close finish  -1:timeout
*/
int wait_isp0dup_task_closed(void)
{
	struct timeval cur_tv;
	struct timeval las_tv;
	gettimeofday(&las_tv, NULL);
	while (chk_preview_use_isp0dup() == 1) {
		gettimeofday(&cur_tv, NULL);
		if ((cur_tv.tv_sec - las_tv.tv_sec) >= 30) {
			return -1;
		}
		vTaskDelay(50 / portTICK_RATE_MS);
	}
	WT_PRINT(SYS_INFO, "isp0dup close\n");
	return 0;
}

/**
* @brief interface function - set isp1 task running
* @param value 1:running 0:not running
*/

void isp1_is_running(bool value)
{
	task_is_running.bits.isp1running = value;
	return;
}

/**
* @brief interface function - check isp1task is running
* @param value 1:running 0:not running
*/

int chk_isp1_is_running(void)
{
	return !!(task_is_running.task_running & isp1runningflag);
}

/**
* @brief interface function - set isp1 task closing
* @param value 1: to closing  0:closing finish
*/
void set_isp1_to_closing(bool value)
{
	task_is_closing.bits.isp1closing = value;
	return;
}

/**
* @brief interface function - check isp0 task  close ok
* @return 1 : to closing  0: close recordtask finish
*/
int chk_isp1_task_close(void)
{
	if (task_is_closing.task_closing & isp1closingflag) {
		set_isp1_to_closing(0);
		return 1;
	}
	return 0;
}

/**
* @brief interface function - wait  isptask1 close finish
* @param value 0:close finish  -1:timeout
*/

int wait_isp1_task_closed(void)
{
	struct timeval cur_tv;
	struct timeval las_tv;
	gettimeofday(&las_tv, NULL);
	while (chk_isp1_is_running() == 1) {
		gettimeofday(&cur_tv, NULL);
		if ((cur_tv.tv_sec - las_tv.tv_sec) >= 30) {
			return -1;
		}
		vTaskDelay(50 / portTICK_RATE_MS);
	}
	WT_PRINT(SYS_INFO, "isp1 task close\n");
	return 0;
}

/**
* @brief interface function - for uvc use
* @return 0: isp1 close and restart  0:not ok , 1: ok
*/


int  chk_ispclose_restart(void)
{
	return !!((set_task_to_close.settaskclose & setisp1closeflag) ||
	          (chk_isp0_is_running() == 0) ||
	          (chk_isp1_is_running() == 0));
}


/**
* @brief interface function - check task is closing and set restart flag
* @note: close task sequence : protect ->record ->lapse -> isp0dup -> isp0 ->isp1
*/

static void check_task_close(void *pvParameters)
{
	int close_task;
	while (1) {

		if (waitflag == 0) {
			close_task = 0;
			if (chk_protect_closeflag()) {           /**< check protect task will close */
#ifndef CONFIG_APP_DRONE
				set_protect_to_closing(1);           /**< set close flag to use for protect task */
				wait_protect_task_closed();          /**< wait protect task has closed */
				set_protect_restartflag();           /**< set restart protect task flag */
#endif
				reset_protect_closeflag();           /**< clear close */
				close_task = 1;
			}
			if (chk_record_closeflag()) {            /**< check record task will close */
				set_record_to_closing(1);            /**< set close flag to use for record task */
				wait_record_task_closed();           /**< wait record task has closed */
				set_record_restartflag();            /**< set restart record task flag */
				reset_record_closeflag();            /**< clear close */
				close_task = 1;
			}
			if (chk_lapse_record_closeflag()) {      /**< the same flow*/
#ifndef CONFIG_APP_DRONE
				set_lapse_to_closing(1);
				wait_lapse_task_closed();
				set_lapse_record_restartflag();
#endif
				reset_lapse_record_closeflag();
				close_task = 1;
			}
			if (chk_isp0dup_closeflag()) {           /**< the same flow*/
				set_isp0dup_to_closing(1);
				wait_isp0dup_task_closed();
				set_isp0dup_restartflag();
				reset_isp0dup_closeflag();
				close_task = 1;
			}
			if (chk_isp0_closeflag()) {              /**< the same flow*/
				set_isp0_to_closing(1);
				wait_isp0_task_closed();
				set_isp0_restartflag();
				reset_isp0_closeflag();
				close_task = 1;
			}
			if (chk_isp1_closeflag()) {              /**< the same flow*/
				set_isp1_to_closing(1);
				wait_isp1_task_closed();
				set_isp1_restartflag();
				reset_isp1_closeflag();
				close_task = 1;
			}
			if (close_task == 1) {
#ifndef CONFIG_APP_DRONE
				timestamp_osd_uninit();
#endif
				waitflag = 1;
			}
		}
		vTaskDelay(500 / portTICK_RATE_MS );
	}
	vTaskDelete(NULL);
}


/**
* @brief interface function -  task restart
* @note: restart task sequence : isp0->isp0dup->isp1->record->lapse->protect
*/

static void check_task_restart(void *pvParameters)
{
	//int result;
	int recordtask_restart = 0;
	while (1) {
		if (waitflag == 1) {

			if (chk_isp0dup_restartflag()) {                                     /**< check ispdup is restart*/
				if (chk_isdup_set_osd_gain() == 1) {                              /**< preview use isp0dup*/
					enable_isp0dup1_preview();                                   /**< enable  isp0dup preview and send data to rtp */
#ifdef CONFIG_APP_DASHCAM
					usbd_mid_set_class_mode(USBD_MODE_UVC, SNX_USBD_OPTION_UVC_RES_CHANGE);
#endif
				}
				reset_isp0dup_restartflag();                               		 /**< clear isp0dup restart flag*/
			}

			if (chk_isp0_restartflag()) {                                        /**< check isp0 is restart*/

				if (chk_isdup_set_osd_gain() == 1) {                              /**< preview use isp0dup*/
					preview_use_isp0dup = 1;
					if (mf_video_stream_ispch0_init() != pdPASS)                        /**< record  use isp0, preview use isp0dup*/
						return ;
					rtsp_reg_force_iframe(mf_video_h264_set_iframe_ispdup);
				} else {                                                          /**< preview use isp1*/
					preview_use_isp0dup = 0;
					if (mf_video_stream_ispch0_init() != pdPASS)                        /**< record  use isp0, preview use isp1*/
						return ;
				}
				mf_snapshot_init();
				reset_isp0_restartflag();                                         /**< clear isp0 restart flag*/
			}
			if (chk_isp1_restartflag()) {                                         /**< check isp1 is restart*/
				if (chk_isdup_set_osd_gain() == 1) {                              /**< preview use isp0dup*/
					enable_isp0dup1_preview();                                    /**< enable  isp0dup preview and send data to rtp */
#ifndef CONFIG_APP_DRONE
					//usbd_mid_set_class_mode(USBD_MODE_UVC, SNX_USBD_OPTION_UVC_RES_CHANGE);
#endif
					mf_set_preview(1);
					if (mf_video_stream_ispch1_init() != pdPASS)
						return ;

				} else {
					if (mf_video_stream_ispch1_init() == pdFAIL) {          /**< preview use isp1*/
						WT_PRINT(SYS_ERR, "video preview_init fail\n");
						mf_video_set_preview_cb(NULL);
					} else {
						rtsp_reg_force_iframe(mf_video_h264_set_iframe);

#ifndef CONFIG_APP_DRONE
						//usbd_mid_set_class_mode(USBD_MODE_UVC, SNX_USBD_OPTION_UVC_RES_CHANGE);
						mf_set_snapshot(0);
#endif
						mf_set_preview(1);
					}
				}
				mf_thumbnail_init();
				reset_isp1_restartflag();                                         /**< clear isp1 restart flag*/
			}
			if (preview_use_isp0dup == 1)
				osd_preview_is_dup1_setting();                       /**< setting osd */
			else
				osd_preview_is_isp1_setting();
#ifndef CONFIG_APP_DRONE
			timestamp_osd_init();
#endif

			if (chk_record_restartflag()) {                                       /**< check record is restart*/
				init_record_task();                                               /**< restart record task*/
				recordtask_restart = 1;
				reset_record_restartflag();                                       /**< clear record restart flag*/
			}
			if (chk_lapse_record_restartflag()) {                                 /**< check lapse is restart*/
#ifndef CONFIG_APP_DRONE
				reclapse_init_task();                                             /**< restart lapse task*/
#endif
				recordtask_restart = 1;
				reset_lapse_record_restartflag();                                 /**< clear lapse restart flag*/
			}
			if (chk_protect_restartflag()) {                                      /**< check protect is restart*/
#ifndef CONFIG_APP_DRONE
				init_protect_task();                                              /**< restart protect task*/
#endif
				recordtask_restart = 1;
				reset_protect_restartflag();                                      /**< clear protect  restart flag*/
			}
			if (recordtask_restart == 1) {

				mid_sd_refresh_mutex();                                                 /**<memory card remount for record,lapse,protect task*/
				recordtask_restart = 0;
			}
			waitflag = 0;
		}
		vTaskDelay(500 / portTICK_RATE_MS );
	}
	vTaskDelete(NULL);
}


int check_task_close_process(void)
{
	if (pdPASS != xTaskCreate(check_task_close, "check_task_close", STACK_SIZE_4K, NULL, PRIORITY_TASK_APP_VIDEO_GET, NULL)) {
		WT_PRINT(SYS_ERR, "could not create watch_task\n");
		return pdFAIL;
	}
	return pdPASS;
}

int check_task_restart_process(void)
{
	if (pdPASS != xTaskCreate(check_task_restart, "check_task_restart", STACK_SIZE_4K, NULL, PRIORITY_TASK_APP_VIDEO_GET, NULL)) {
		WT_PRINT(SYS_ERR, "Could not create watch_task\n");
		return pdFAIL;
	}
	return pdPASS;
}
