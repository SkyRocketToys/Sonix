/*********************************************************************************
* watch_task.h
*
* Header file of  watch task for check task close is or not
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


#ifndef __WATCH_TASK_H__
#define __WATCH_TASK_H__
#include <stdbool.h>


#define protectrunningflag          	(0x1<<0)
#define recordrunningflag     			(0x1<<1)
#define isp0runningflag             	(0x1<<2)
#define isp1runningflag             	(0x1<<3)
#define lapserunningflag                (0x1<<4)
#define task5                           (0x1<<5)
#define task6                           (0x1<<6)
#define task7                           (0x1<<7)

typedef union {
	struct {
		unsigned char protectrunning: 1;                 /**<if protect task init ok and run  set to 1*/
		unsigned char recordrunning: 1;                  /**<if record task init ok  and run  set to 1*/
		unsigned char isp0running: 1;                    /**<if isp0    task init ok  and run   set to 1*/
		unsigned char isp1running : 1;                   /**<if isp1    task init ok  and run   set to 1*/
		unsigned char lapserunning : 1;                  /**<if lapse  task init ok  and run   set to 1*/
		unsigned char bit5 : 1;
		unsigned char bit6 : 1;
		unsigned char bit7 : 1;
	} bits;
	unsigned char task_running;
} taskisrunning;




#define setprotectcloseflag          	    (0x1<<0)
#define setrecordcloseflag     			    (0x1<<1)
#define setisp0closeflag             	    (0x1<<2)
#define setisp1closeflag             	    (0x1<<3)
#define setisp0dupcloseflag                 (0x1<<4)
#define setlapsecloseflag               (0x1<<5)
#define task6                           (0x1<<6)
#define task7                           (0x1<<7)

typedef union {
	struct {
		unsigned char setprotectclose: 1;          /**<if process want to close protect task , set to 1*/
		unsigned char setrecordclose: 1;           /**<if process  want to close recordtask , set to 1*/
		unsigned char setisp0close: 1;             /**<if process  want to close isp0 task , set to 1*/
		unsigned char setisp1close : 1;            /**<if process  want to close isp1 task , set to 1*/
		unsigned char setisp0dupclose : 1;         /**<if process  want to close isp0dup task , set to 1*/
		unsigned char setlapseclose : 1;           /**<if process  want to close  lapse task , set to 1*/
		unsigned char bit6 : 1;
		unsigned char bit7 : 1;
	} bits;
	unsigned char settaskclose;
} settasktoclose ;


#define protectclosingflag          	(0x1<<0)
#define recordclosingflag     			(0x1<<1)
#define isp0closingflag             	(0x1<<2)
#define isp1closingflag             	(0x1<<3)
#define isp0dupclosingflag              (0x1<<4)
#define lapseclosingflag                (0x1<<5)
#define task6                           (0x1<<6)
#define task7                           (0x1<<7)

typedef union {
	struct {
		unsigned char protectclosing: 1;              /**<if set to 1 ,protect task will close*/
		unsigned char recordclosing: 1;               /**<if set to 1 ,record  task will close*/
		unsigned char isp0closing: 1;                 /**<if set to 1 ,isp0     task will close*/
		unsigned char isp1closing : 1;                /**<if set to 1 ,isp1     task will close*/
		unsigned char isp0dup1closing : 1;            /**<if set to 1 ,isp0dup  task will close*/
		unsigned char lapseclosing : 1;               /**<if set to 1 ,timelapse  task will close*/
		unsigned char bit6 : 1;
		unsigned char bit7 : 1;
	} bits;
	unsigned char task_closing;
} taskisclosing;





#define protectrestartflag          	    (0x1<<0)
#define recordrestartflag     			    (0x1<<1)
#define isp0restartflag             	    (0x1<<2)
#define isp1restartflag             	    (0x1<<3)
#define isp0duprestartflag                  (0x1<<4)
#define lapserestartflag                    (0x1<<5)
#define task6                               (0x1<<6)
#define task7                               (0x1<<7)

typedef union {
	struct {
		unsigned char protectrestart: 1;       		/**<if set to 1 ,protect task has closed, it must re-start*/
		unsigned char recordrestart: 1;             /**<if set to 1 ,record task has closed, it must re-start*/
		unsigned char isp0restart: 1;               /**<if set to 1 ,isp0 task has closed, it must re-start*/
		unsigned char isp1restart : 1;              /**<if set to 1 ,isp1 task has closed, it must re-start*/
		unsigned char isp0duprestart : 1;           /**<if set to 1 ,isp0dup task has closed, it must re-start*/
		unsigned char lapserestart : 1;             /**<if set to 1 ,lapse  task has closed, it must re-start*/
		unsigned char bit6 : 1;
		unsigned char bit7 : 1;
	} bits;
	unsigned char task_restart;
} taskisrestarting ;



int watch_task_init(void);
int chk_ispclose_restart(void);     /**<for uvc   0:not ok 1: init ok*/
void protect_is_running(bool value);
int chk_protect_task_close(void);
int chk_record_is_running(void);
int chk_protect_is_running(void);
int chk_lapse_is_running(void);
int chk_isp0_is_running(void);
int chk_isp1_is_running(void);
void set_isp0dup_closeflag(void);
void set_protect_to_closing(bool value);
int wait_protect_task_closed(void);
void set_record_to_closing(bool value);
int wait_record_task_closed(void);
void set_lapse_to_closing(bool value);
int wait_lapse_task_closed(void);
void set_protect_closeflag(void);
void set_record_closeflag(void);
void set_lapse_record_closeflag(void);
void set_isp0_closeflag(void);
void set_isp1_closeflag(void);
void lapse_is_running(bool value);
int chk_record_task_close(void);
void isp0_is_running(bool value);
void isp1_is_running(bool value);
int chk_isp1_task_close(void);
int chk_isp0_task_close(void);
int chk_isp0dup_task_close(void);
int check_task_restart_process(void);
int check_task_close_process(void);
void set_isp0_to_closing(bool value);
int wait_isp0_task_closed(void);
void set_isp1_to_closing(bool value);
int wait_isp1_task_closed(void);
int chk_lapse_task_close(void);
void record_is_running(bool value);

#endif
