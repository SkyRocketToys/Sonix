
/*********************************************************************************
* sdcad_record.h
*
* Header file of  schedule record
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



#ifndef __SD_CARDRECORD_H__
#define __SD_CARDRECORD_H__
#include <libmid_rec/record.h>
#include "rec_fatfs.h"
#include "rec_cardsize.h"
#include "rec_common.h"

#define RM_MAX_QUEUE_ITEM 5
#define NVRAM_SD_EJECT	"SD_Card.aac"
#define NVRAM_SD_INJECT	"SD_pull_Card.aac"
#define NVRAM_START_REC	"Start_recoeding.aac"
#define NVRAM_END_REC	"Stop_Recording.aac"


#define IMPORT_CARD_INACTIVE 0
#define IMPORT_CARD_ACTIVE 1
#define IMPORT_CARD_FINISHED 2

typedef struct rm_info {
	char rm_name_path[LEN_FILENAME];
	char rn_name[LEN_FILENAME];
} rm_info_t;

typedef struct _chk_card_file_info{
	int task_state;
	xQueueHandle chk_card_queue;
	xTaskHandle task_chk_card;
}chk_card_file_info_t;


struct recordinfo_s {
	record_info_t *pRecord_info;		/**<middleware record info structure*/
	enum FILE_TYPE type;				/**<file category type*/
	RecParam_t RecParam;             	/**<record video and audio info*/
	SchedTime_t  *sched_p;	         	/**<schedule time */
	int  readfilelistok;             	/**<if read memory card file have been readed , set to 1 */
	long long  recordusedsize;       	/**<schedule record  folder size that has been used */
	int  recordstatus;               	/**<schedule record  status  */
	int  lastsecond;
	int  recordclose;                	/**<if record task close , set to 1 */
	int  sdcardisfull;               	/**<if sdcard is full , set to 1 */
	int  sdcardseed;
	int  wb_release;					/**<to release wb mem , the value is 1; to create mem, the value is 0*/
	char schedpath[LEN_FILENAME];		/**<schedule record path */
	unsigned short  gpscount;        	/**<gps sn */
	unsigned short  gsensorcount;    	/**<gsensor sn */
	enum RECORDFILE_TYPE fileformat; 	/**<0:timebase 1:count base*/
	xQueueHandle rm_queue;
	xQueueHandle chk_card_queue;
	xTaskHandle rm_file_task;
};

typedef struct recordinfo_s recordinfo_t;

int  init_record_task(void);
void recordaudio(unsigned char *pframe, unsigned int uiframeSize, struct timeval tval);
void recordvideo(unsigned char iframe, unsigned char *pframe, unsigned int uiframeSize, struct timeval tval);
recordinfo_t *get_rec_pointer(void);
void schedrec_suspend_restart(int Forcerecord);
void schedrec_suspend(void);
void schedrec_close(void);
int  schedrec_state(void);
char **schedrec_listtobuffer(int *filesize);
int  del_rec_recordfile(const char *delname);
int  chk_rec_status(const recordinfo_t *info, int flag);
int  read_card_state(void);
void user_diable_rec(void);
void user_enable_rec(void);
void diable_rec(void);
void enable_rec(void);
int  schedrec_get_isfull(void);
int  chk_rec_enable(void);
void modify_dev_time(void);
int  reclist_to_file(char *path);
void rec_sched_incompletefile_hndl(char *filename);

void schedrec_set_wb_status(int flag);
int get_schedrec_status(void);
void chk_card_task_init(void);
void chk_card_task_uninit(void);
void rec_sched_next_file();
void chk_userparam(recordinfo_t *sdrecord_info);
void chk_rec_start_changefile(recordinfo_t *sdrecord_info);
void rec_stop_changestate(recordinfo_t *sdrecord_info);
void rec_full_deloldestfile(recordinfo_t *sdrecord_info);	

extern int userdisablerecord;
void add_rec_status(recordinfo_t *info, int flag);
void del_rec_status(recordinfo_t *info, int flag);

void sched_rec_flow(recordinfo_t *sdrecord_info);

#endif
