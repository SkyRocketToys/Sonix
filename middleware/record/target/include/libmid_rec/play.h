/**
 * @file
 * this is middleware play header file, include this file before use
 * @author Algorithm Dept Sonix. (CJ porting to RTOS)
 */

#ifndef _PLAY_H_
#define _PLAY_H_

#include "avi.h"
#include "mp4.h"

/**
* @brief Avi file codec format.
*/
enum AV_CODEC{
	NONE_FORMAT,
	PCM_FORMAT,
	ALAW_FORMAT,
	MULAW_FORMAT,
	G726_FORMAT,
	AAC_FORMAT,
	MJPEG_FORMAT,
	H264_FORMAT,
	SNX420_FORMAT,
};

enum PLAY_TYPE{
	PLAY_AVI,
	PLAY_MP4,
};

enum PLAY_AV_TYPE{
	PLAY_AV_TYPE_NONE,
	PLAY_AV_TYPE_VIDEO,
	PLAY_AV_TYPE_AUDIO,
};

enum CHECK_IFAME{
	NOT_CHECK_IFAME,
	IS_CHECK_IFAME,
};



/**
* @brief structure for play media infomation.
*/
typedef struct _play_source{
	enum PLAY_TYPE type;
	AVI_read_Info_t AviReadInfo;		/**<  avi file header pointer */
	MP4_read_Info_t Mp4ReadInfo;
	uint32_t SetTimeFlag;				/**<  do set time flag */
}play_source_t;

int play_check_keyframe(uint8_t *data, void *pData, uint32_t size);
play_source_t* play_create_recording_file(const char *filename, void *idx_ptr,enum CHECK_IFAME);
play_source_t* play_create(const char *filename,enum CHECK_IFAME);
void play_destory(play_source_t* play);
enum AV_CODEC play_read_av_type(play_source_t* play, enum AV_TYPE type);
int play_read_video_resolution(play_source_t* play, uint32_t *width, uint32_t *height);
int play_read_video_fps(play_source_t* play, uint32_t *fps);
int play_read_video_frames_num(play_source_t* play, uint32_t *frames_num);
int play_read_audio_sample_rate(play_source_t* play, uint32_t *sample_rate);
int play_read_audio_bitrate(play_source_t* play, uint32_t *bitrate);
int play_check_keyframe(uint8_t *data, void *pData, uint32_t size);
enum PLAY_AV_TYPE play_read_data(play_source_t* play, void **data, uint32_t *size);
int play_reset_read_data(play_source_t* play, void *buf);
int play_set_time(play_source_t* play, uint32_t timestamp,enum CHECK_IFAME);

#endif		//_PLAY_H_
