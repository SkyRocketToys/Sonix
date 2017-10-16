/*********************************************************************************
* sdcad_playback.h
*
* Header file of  playback file
*
* History:
*    2016/04/26 - [Allen_Chang] created file
*
* Copyright (C) 1996-2015, Sonix, Inc.
*
* All rights reserved. No Part of this file may be reproduced, stored
* in a retrieval system, or transmitted, in any form, or by any means,
* electronic, mechanical, photocopying, recording, or otherwise,
* without the prior consent of Sonix, Inc.

*********************************************************************************/

#include <libmid_rec/play.h>
#include <libmid_rtsp_server/rtsp_server.h>
#include "rec_common.h"

#define NOTIFY_APP_READEND_TIMES 20        /**<notify app playback end */

struct infotortsp {
	audio_attr  audioattr;
	video_attr  videoattr;
	int rtsp_videoid;
	int rtsp_audioid;
};

struct mediainfo {
	int audioformat;                   	  /**<audioformat */
	int videoformat;	     			  /**<video format  h264/mjepg */
	uint32_t width;         			  /**<video resolution width */
	uint32_t height;        		      /**<video resolution height */
	uint32_t fps;                         /**<video fps */
	uint32_t frames_num;                  /**<file total frames number*/
	uint32_t sample_rate;                 /**<audio sample rate*/
	uint32_t bitrate;                     /**<audio bitrate*/
	uint32_t file_len;                    /**<file lengh*/
};

struct timestampinfo {
	int video_cnt;
	struct timeval video_cur_tv;
	struct timeval audio_cur_tv;
};

struct playbackinfo_s {
	play_source_t *play;                  		/**<middleware playsource structure*/
	char play_filename[LEN_FULLPATH];			/**<plyaback file name  */
	char play_filename_rename[LEN_FULLPATH];	/**<plyaback file name  + clinet ip addr */
	DWORD clinetipaddr;                   		/**<clinet ip addr */
	char *clinetmac;
	struct mediainfo   media_info;        		/**<avi or mp4  information */
	struct infotortsp  rtspinfo;          		/**<avi or mp4  information */
	int play_start;                       		/**<playback file start */
	int play_close;                       		/**<playback task to close */
	int numbertask;                       		/**<number of playback task create */
	int has_audio;                        		/**<check if the stream has audio data*/
	int cntframe;                         		/**<frame count*/
	int time_interval;                    		/**<the time interval between video frames*/
	int first_video_packet;               		/**<check if the packet is the first video packet*/
	int first_audio_packet;               		/**<check if the packet is the first audio packet*/
	int total_delay_time_count;           		/**<total delay time of receive frame*/
	int pb_stop_read_data;                		/**<stop read data in demux flow*/
	unsigned int preview_rec_frame_time;  		/**< receive time of preview frame */
	struct timestampinfo timstmp_info;
	xSemaphoreHandle pbseek_mutex;
};

typedef struct playbackinfo_s playbackinfo_t;

int  pb_get_filelen(const char *filename, int *filelen);
void pb_init_array(void);
int pb_receive_cmd(const char *play_filename, DWORD clinetip, char *clinetmac, char *return_rename);
void pb_start(char *mac, int strat_point);
void pb_stop(char *mac);
void pb_all_stop(void);
void pb_pause(char *mac);
void pb_seek(char *mac, uint32_t timestamp);
int  pb_state(void);
int pb_get_filelen(const char *filename, int *filelen);
