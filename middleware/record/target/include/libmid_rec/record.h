/**
 * @file
 * this is middleware record header file, include this file before use
 * @author Algorithm Dept Sonix. (yiling and chienjen porting to RTOS)
 */
#ifndef __RECORD_LIB_H__ 
#define __RECORD_LIB_H__ 
#include <sys/time.h>
#include <sys_clock.h>
#include "record_common.h"

//#define REC_AVI				1	/**< record stream as avi file*/
//#define REC_MP4				0	/**< record stream as avi file*/
//#if REC_AVI
#include "avi.h"
//#elif REC_MP4
#include "mp4.h"
//#else
#include <libmid_fatfs/ff.h>
#include "writebuf.h"
#include <libmid_filemanager/usr_ctrl.h>
//#endif

//#define REC_HAS_AUDIO		AVI_HAS_AUDIO
#define CMD_QUEUE_BUF		5	
#define MAX_SEMA_NUM		(CMD_QUEUE_BUF+1)
#define RECORD_INDEX        "index"

#define PRERECORD			0	/**< enable prerecord function*/
#define RECORD_FILENAME_LENGTH	300

#include "buflist.h"
#if PRERECORD
#include "buflist.h"

#define	TYPE_VIDEO			0	/**< define as video*/
#define	TYPE_AUDIO			1	/**< define as video*/
#endif

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif


/**
* @brief structure for video infomation
*/
typedef struct _RecVdoInfo{
	unsigned int width, height;	/**<  video resolution*/
	unsigned char ucFps;		/**<  frame rate*/
	unsigned int  uiBps;
	unsigned char ucScale;	 	/**<  video scale*/
	unsigned char ucStreamMode;	/**<  steam mode: 2=H.264*/ 
	//int iBps;
	//int iGop;
}RecVdoInfo_t;


/**
* @brief structure for audio infomation
*/
typedef struct _RecAdoInfo{
	unsigned int uiFormat;			/**<  audio format AUD_FORMAT_S16_LE, AUD_FORMAT_A_LAW or AUD_FORMAT_MU_LAW...*/
	unsigned int uiSampleRate;		/**<  audio sample rate*/
	unsigned char ucBitsPerSample;	/**<  bits per sample*/
	unsigned int uiPacketSize;		/**<  size for each record unit*/
	unsigned int  uiBitRate;
}RecAdoInfo_t;

/**
* @brief structure for wirte buffer init 
*/
typedef struct _RecWriteBufferInit{
	uint32_t write_buf_size;
	int write_unit_to_file;
}RecWirteBufferInitInfo_t;


enum RECORD_TYPE{
	RECORD_AVI=0,
	RECORD_MP4,
	RECORD_STREAM_FILE,
};

enum RECORDFILE_TYPE{
	RECORD_TIMEFORMAT=0,
    RECORD_COUNTFORMAT,
};

typedef void (*finishedfile_cb_t)(char* finishedfile,char* renameoffile);
typedef void (*update_seed_cb_t)();

/**
* @brief structure for record parameters
*/

typedef struct _RecParam{
	RecVdoInfo_t vdo;
	RecAdoInfo_t ado;
	RecWirteBufferInitInfo_t writebufferparam;
	int max_record_len;
}RecParam_t;


typedef struct _record_info{
	xSemaphoreHandle CmdQueueSema[MAX_SEMA_NUM];	//set sema larger than queue num to avoid sema still used by previous cmd
	xSemaphoreHandle mutex;
	xSemaphoreHandle FileSyncMutex;	
	QueueHandle_t  RecordActQueue; 
	xTaskHandle WriteFiletask;
	//unsigned char ucCmdQueueStatus[CMD_QUEUE_BUF];
	char	index;
	unsigned char ucRecStatus;
	unsigned char ucRecStart;
	unsigned char ucRecNext;
	unsigned char ucRecStop;
	unsigned char ucRecWaitFileClose;
	unsigned char ucSemaphoreIndex;
	volatile unsigned char ucIsRecording;
	unsigned char ucPrerecordTime;					//unit : 1 sec
	char prefix_name[20];
	char path[200];
	char rec_start_ret, rec_next_ret, rec_stop_ret;
	unsigned int record_count;
	enum RECORDFILE_TYPE fileformat;
	enum FILE_TYPE record_file_type;
	volatile unsigned char ucRecCache;
	buflist_t AVBufList;
	xSemaphoreHandle AVListMutex;
	volatile system_date_t date;
	//int max_pre_time;
	int max_av_length;
#if PRERECORD	
	volatile unsigned char ucFileOpen;				/**< file opened and do record */
	unsigned char uclast_cmd;						/**< last command for record */
	unsigned char ucVdoFps;							/**< video frame rate information */
	unsigned int uiAdoFps;							/**< audio frame rate information */
	volatile int iVdoListReadPosi;					/**< which node recording with video list */
	volatile int iAdoListReadPosi;					/**< which node recording with audio list */
	buflist_t VdoList;								/**< video buffer list */
	buflist_t AdoList;								/**< audio buffer list */
	xTaskHandle WriteCmdtask;						/**< task for get date and write command to  WriteFiletash */
	xSemaphoreHandle VdoListMutex;					/**< mutex for video buffer list */
	xSemaphoreHandle AdoListMutex;					/**< mutex for audio buffer list */
#endif
	enum RECORD_TYPE rec_type;

	//char FileName[30] = "stream_000.avi";
	AVI_Info_t AviInfo;	

	Mp4_Info_t Mp4Info;

	FIL fstream;
	char FileName[RECORD_FILENAME_LENGTH];
	char IntevalFileName[RECORD_FILENAME_LENGTH];
	//WriteBufInfo_t WBInfo;
	//BufInitInfo_t WBInitInfo;
    char LastFileNameClosing[300];
	char LastFileName[300];
	
	unsigned int rec_capability;
	
	finishedfile_cb_t finishedfile_cb;
	update_seed_cb_t  update_seed_cb;
	AVIGPSINFO_t		GPS_info;
	AVIGSENSORINFO_t	GSENSOR_info;
	
}record_info_t;


typedef struct _RecFileInfo{
	char filename[300];
	AVIIndex_t *pAviIndex;
	int VdoFrameNum;
	int AdoFrameNum;
	unsigned char stop_status;	// 1:on working 0:finish
}RecFileInfo_t;


const char *get_lost_data_path();
void set_lost_data_flag(int val);
int record_init(record_info_t** prec_info, RecParam_t *pRecParam, unsigned char index, enum RECORD_TYPE record_type, unsigned int cap,finishedfile_cb_t cb, update_seed_cb_t update_seed, int max_pre_time, enum FILE_TYPE file_type);
int record_uninit(record_info_t* rec_info);
void record_writebufreset(record_info_t* rec_info);
void record_video(record_info_t* rec_info, unsigned char IFrame, unsigned char *pFrame, unsigned int uiFrameSize, struct timeval tval);
void record_audio(record_info_t* rec_info, unsigned char *pFrame, unsigned int uiFrameSize, struct timeval tval);
int record_set_start(record_info_t* rec_info, int is_block);
int record_set_next(record_info_t* rec_info, int is_block);
int record_set_stop(record_info_t* rec_info, int is_block);
int record_get_state(record_info_t* rec_info);
int prerecord_set_time(record_info_t* rec_info, unsigned char time);
unsigned char prerecord_get_time(record_info_t* rec_info);
char* record_get_last_file_name(const record_info_t* rec_info);
char* record_get_cur_file_name(const record_info_t* rec_info);
int record_get_file_info(const record_info_t* rec_info, RecFileInfo_t *pCurFileInfo, RecFileInfo_t *pLastFileInfo);
void record_set_prefix_name(record_info_t* rec_info, char* prefix_name);
void record_set_filename_format(record_info_t* rec_info,enum RECORDFILE_TYPE format);
int record_set_path(record_info_t* rec_info, char* path);
char record_get_wb_status(record_info_t* rec_info);
int record_writebuf_release(record_info_t* rec_info);
int record_writebuf_create(record_info_t* rec_info);
#endif	//__RECORD_LIB_H__
