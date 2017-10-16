/**
 * @file
 * this is middleware avi header file, include this file before use
 * @author Algorithm Dept Sonix. (yiling porting to RTOS)
 */

#ifndef __AVI_LIB_H__
#define __AVI_LIB_H__

//#define AVI_HAS_AUDIO	1 									/**< avi file contain audio infomation*/
//#define MAX_STREAM_LENTH	200								/**< max stream length in one avi file(unit:second) */
#define MAX_STREAM_LENTH	pAviInfo->MaxStreamLength
#define MAX_FRAME_NUM(v,a)	(MAX_STREAM_LENTH*((v)+(a+1)))	/**< max frame num in one avi file(video+audio), audio+1 for upper bound*/
#define AVI_INDEX_TABLE_NUM		2
#define	fast_seek				1 							/**< avi read file at fast seek mode*/
#define	SZ_TBL					20*1024						/**< cluster link map table size for fast seek mode*/

#include <libmid_fatfs/ff.h>
#include "avi_header.h"
#include "writebuf.h"

typedef void (*avi_cb_t)(void *, CMDStatus_t);

#define AVI_FILENAME_LENGTH	300

enum
{
	AVI_VIDEO=0,
	
	AVI_AUDIO,	

	MAX_STREAM_NUM
};

enum AV_TYPE{
	AV_TYPE_NONE,
	AV_TYPE_VIDEO,
	AV_TYPE_AUDIO,
};

/**
* @brief structure for video infomation
*/
typedef struct _AviVdoInfo{
	unsigned int width, height;	/**<  video resolution*/
	unsigned char ucFps;		/**<  frame rate*/
	unsigned int  uiBps;	
	unsigned char ucScale;	 
	unsigned char ucStreamMode; /**<  steam mode: 2=H.264*/
	//int iBps;
	//int iGop;

}AviVdoInfo_t;


/**
* @brief structure for audio infomation
*/
typedef struct _AviAdoInfo{
	unsigned int uiFormat;			/**<  audio format AUD_FORMAT_S16_LE, AUD_FORMAT_A_LAW or AUD_FORMAT_MU_LAW*/
	unsigned int uiSampleRate; 		/**<  audio sample rate*/
	unsigned char ucChannel;		/**<  audio channel number*/
	unsigned char ucBitsPerSample;	/**<  bits per sample*/	 
	//unsigned long ulChunkSize;
	unsigned int uiFps;				/**<  packet number per second*/
	unsigned int uiBitRate;
}AviAdoInfo_t;



/**
* @brief structure for wirte buffer init 
*/
typedef struct _AviWirteBufferInit{
	uint32_t write_buf_size;
	int write_unit_to_file;
}AviWirteBufferInitInfo_t;





typedef struct _AVI_Info{
	AVIFileHeader_t *pAviHeader;	//avi file header 
	AVIIndex_t AviIndex[AVI_INDEX_TABLE_NUM];			//avi index data 
	int CurIndex; 
	int MaxStreamLength;
	AviVdoInfo_t AviVideoInfo;		//video parameter
	AviAdoInfo_t AviAudioInfo;		//audio parameter	//error:audio

	char Filename[AVI_FILENAME_LENGTH];
	char IntevalFilename[AVI_FILENAME_LENGTH];
	unsigned char *SkipFrame;
	int SkipFrameLen;
	FIL AviFile;					//avi file
	TickType_t StartTick[MAX_STREAM_NUM]; //tick of stream start	
	int NumFrame[MAX_STREAM_NUM];		//
	int has_audio;
	int has_gps;
	int has_gsensor;


	unsigned int IndexOffset;
	WriteBufInfo_t WBInfo;
	BufInitInfo_t WBInitInfo;

	//for skip frame and av sync
	uint64_t pre_vdo_reach_time;
	unsigned int vdo_accumlation_duration;
//	int cur_ubAVI_H264_PIC, cur_ubAVI_H264_POC;
	unsigned char cur_ubAVI_H264_PIC, cur_ubAVI_H264_POC;
	uint64_t pre_ado_reach_time;
	unsigned int ado_accumlation_duration;

	//call back function	
	avi_cb_t hdr_cb;
	avi_cb_t tra_cb;
	void *cb_parm;
	avi_cb_t media_start_cb;
	signed char rec_start_ret; 
	xTaskHandle FinishCmdtask;	

	int NumData[MAX_DATA_NUM];		//

	AVIGPSINFO_t		*GPS_info;
	AVIGPS_t			GPS;			//avi GPSinfo data 

	AVIGSENSORINFO_t	*GSENSOR_info;
	AVIGSENSOR_t		GSENSOR;		//avi G-Acceleration info data 

	
}AVI_Info_t;	

/*
Audio Codecs from the Microsoft WAVE Registry
0x0000	WAVE_FORMAT_UNKNOWN
0x0001	WAVE_FORMAT_PCM
0x0002	WAVE_FORMAT_ADPCM
0x0006	WAVE_FORMAT_ALAW
0x0007	WAVE_FORMAT_MULAW
0x0014	WAVE_FORMAT_G723_ADPCM
0x0040	WAVE_FORMAT_G721_ADPCM
0x0050	WAVE_FORMAT_MPEG
0x0064	WAVE_FORMAT_G726_ADPCM
0x00FF	WAVE_FORMAT_AAC

Video Codecs from the Microsoft AVI Registry
H264	H264
MJPG	Motion JPEG DIB Format
*/

/**
* @brief structure for read avi file information
*/
typedef struct _AviReadInfo{
	FIL AviFile;						/**<  file structure for avi */
	char Filename[300];					/**<  avi file name */
	AVIFileHeader_t *pAviHeader;		/**<  avi file header pointer */
	AVIIndexEntry_t *pAviIndex;			/**<  avi file index pointer */
	uint32_t FrmIdx;					/**<  Current frame index */
	uint32_t FrmIdxNum;					/**<  Total frame index number */
	uint32_t avi_fb_size;				/**<  estimated frame size */
	uint8_t *pFrmbuf;					/**<  Frame ping pong buffer pointer */
	uint8_t IdxRdy;						/**<  Avi Index load finished and ready to use */
	uint8_t ReadRecordingFlag;			/**<  Set flag when file is recording */
#if fast_seek
	uint32_t *clmt;						/**<  cluster link map table buffer when fast seek mode enable*/
#endif
	uint8_t *OriginAviHeaderAddr;
	uint8_t *OriginAviIndexAddr;
	uint32_t dev;	 					/**<  current storage, 1:mmc, 2:usbh*/
}AVI_read_Info_t;

int avi_init(AVI_Info_t *pAviInfo,AviWirteBufferInitInfo_t* WriteBufferInit,AviVdoInfo_t* AviVdoInfo,  unsigned int cap);
int avi_uninit(AVI_Info_t *pAviInfo);
int avi_set_stream_info(AVI_Info_t *pAviInfo, AviVdoInfo_t *pVdoInfo, AviAdoInfo_t *pAdoInfo, int max_stream_len);
int avi_set_file_name(AVI_Info_t *pAviInfo, char *filename);
char* avi_get_file_name(AVI_Info_t *pAviInfo);
char* avi_get_inteval_file_name(AVI_Info_t *pAviInfo);
void avi_clear_inteval_file_name(AVI_Info_t *pAviInfo);
int avi_write_header(AVI_Info_t *pAviInfo);
int avi_write_packet(AVI_Info_t *pAviInfo, uint8_t ucMode, uint8_t ucKeyFrame, unsigned char* pFrame, int iFrameSize, uint64_t uiTimeStamp);

int avi_write_trailer(AVI_Info_t *pAviInfo);
void avi_set_cb(AVI_Info_t *pAviInfo, avi_cb_t hdr_cb, avi_cb_t tra_cb, avi_cb_t media_cb,void *param);	

//----- for playback middleware -----
uint32_t avi_get_movi_frmcnt(AVIFileHeader_t AviHeader, uint8_t ubStreamType);
uint32_t avi_get_rate(AVIFileHeader_t AviHeader, uint8_t ubStreamType);
uint32_t avi_get_ado_sample_rate(AVIFileHeader_t AviHeader);
uint32_t avi_get_ado_bits_per_sample(AVIFileHeader_t AviHeader);
int avi_get_vdo_resolution(AVIFileHeader_t AviHeader, uint32_t *width, uint32_t *height);
int avi_set_recording_index_ptr(AVI_read_Info_t *pAviReadInfo, AVIIndex_t *pAviIndex);
int avi_read_init(AVI_read_Info_t *pAviReadInfo);
int avi_read_uninit(AVI_read_Info_t *pAviReadInfo);
int avi_read_frame(AVI_read_Info_t *pAviReadInfo, void **data, uint32_t *size, uint8_t *frm_type);
int avi_reset_frmidx(AVI_read_Info_t *pAviReadInfo);
int avi_reset_data_buf(AVI_read_Info_t *pAviReadInfo, void *buf);
int avi_seek_frame(AVI_read_Info_t *pAviReadInfo, uint32_t time);
int avi_check_fourcc(uint32_t Val, char *pPtr);
void avi_start_finish(AVI_Info_t *pAviInfo, CMDStatus_t status);
char avi_get_wb_status(AVI_Info_t *pAviInfo);

#endif //__AVI_LIB_H__
