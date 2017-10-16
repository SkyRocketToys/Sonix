#ifndef __MP4_LIB_H__
#define __MP4_LIB_H__

#define MP4_PRINT(level, fmt, args...) print_q(level, "[MP4]%s: "fmt, __func__,##args)


//#define MP4_HAS_AUDIO	1 									/**< MP4 file contain audio infomation*/

#define MP4_INDEX_TABLE_NUM		2
#define MKTAG(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))
#define MKBTAG(a,b,c,d) (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

#define MP4_FRAME_BUFFER_SIZE 600*1024

#define MP4_FILENAME_LENGTH	300

#include <libmid_fatfs/ff.h>
#include "writebuf.h"
#include "dyn_buf.h"
#include "avi_header.h"

typedef void (*mp4_cb_t)(void *, CMDStatus_t);
typedef uint32_t FOURC;

enum
{
	MP4_VIDEO=0,
		
	MP4_AUDIO,	

	MAX_MP4_STREAM_NUM
};

enum
{
	RETURN_MP4_VIDEO=0,		
	RETURN_MP4_AUDIO,	
	RETURN_MP4_NONE
};

typedef struct _Mp4VdoInfo{
	unsigned int width, height;	/**<  video resolution*/
	unsigned char ucFps;		/**<  frame rate*/
	unsigned char ucScale;	 
	unsigned char ucStreamMode; /**<  steam mode: 2=H.264*/
	//int iBps;
	//int iGop;

}Mp4VdoInfo_t;



typedef struct _Mp4AdoInfo{
	unsigned int uiFormat;			/**<  audio format AUD_FORMAT_S16_LE, AUD_FORMAT_A_LAW or AUD_FORMAT_MU_LAW*/
	unsigned int uiSampleRate; 		/**<  audio sample rate*/
	unsigned char ucChannel;		/**<  audio channel number*/
	unsigned char ucBitsPerSample;	/**<  bits per sample*/	 
	//unsigned long ulChunkSize;
	unsigned int uiFps;				/**<  packet number per second*/
	unsigned int uiBitRate;
}Mp4AdoInfo_t;

/**
* @brief structure for wirte buffer init 
*/
typedef struct _Mp4WirteBufferInit{
	uint32_t write_buf_size;
	int write_unit_to_file;
}Mp4WirteBufferInitInfo_t;


typedef struct FileBox{
	FOURC type;
	FOURC major_brand;
	uint32_t minor_version;
	FOURC compatible_brand[3];
}FileBox_t;

typedef struct MvHdBox{
	uint32_t size;
	FOURC type;
	uint8_t versizon;
	uint8_t flag[3];
	uint32_t create_time;
	uint32_t md_time;
	uint32_t t_scale;
	uint32_t duration;
	uint32_t rate;
	uint16_t vol;
	uint8_t reserved[10];
	uint8_t matrix[36];
	uint8_t pre_def[24];
	uint32_t next_track;
}MvHdBox_t;

typedef struct TrakHdBox{
	uint32_t size;
	FOURC type;
	uint8_t versizon;
	uint8_t flag[3];
	uint32_t create_time;
	uint32_t md_time;
	uint32_t track_id;
	uint32_t reserved;
	uint32_t duration;
	uint32_t reserved2[2];
	uint8_t layer[2];
	uint16_t alternate_g;
	uint8_t vol[2];
	uint8_t reserved3[2];
	uint8_t matrix[36];
	uint32_t width;
	uint32_t height;
}TrakHdBox_t;

typedef struct MdiaHdBox{
	uint32_t size;
	FOURC type;
	uint8_t versizon;
	uint8_t flag[3];
	uint32_t create_time;
	uint32_t md_time;
	uint32_t t_scale;
	uint32_t duration;
	uint8_t language[2];
	uint8_t pre_def[2];
}MdiaHdBox_t;

typedef struct HdRefBox{
	uint32_t size;
	FOURC type;
	uint8_t versizon;
	uint8_t flag[3];
	uint32_t pre_def;
	uint32_t hdlr_type;
	uint8_t  reserved[12];
	uint8_t name[31];// not sure, but with end of 0
	uint8_t end;
}HdRefBox_t;

typedef struct VmhdBox{
	uint32_t size;
	FOURC type;
	uint32_t flag;
	uint32_t graphics_mode;
	uint32_t res[2];
}VmhdBox_t;

typedef struct SmhdBox{
	uint32_t size;
	FOURC type;
	uint32_t flag;
	uint16_t balance;
	uint16_t reserved;
}SmhdBox_t;


typedef struct MinfHdBox{
	union{
		VmhdBox_t vmhd;
		SmhdBox_t smhd;
	}vshd;
}MinfHdBox_t;

typedef struct DrefBox{
	uint32_t size;
	FOURC type;
	uint32_t flag;
	uint32_t entrycount;
	uint32_t usize;
	FOURC utype;
	uint32_t uflag;
	//url
}DrefBox_t;

typedef struct DinfBox{
	uint32_t size;
	FOURC type;
	DrefBox_t dref;
	//url
}DinfBox_t;


typedef struct StsdBox{
	uint32_t size;
	FOURC type;
	uint32_t flag;
	uint32_t entrycount;
	
}StsdBox_t;


typedef struct SttsBox{
	uint32_t size;
	FOURC type;
	
}SttsBox_t;

typedef struct StscBox{
	uint32_t size;
	FOURC type;
	
}StscBox_t;

typedef struct StszBox{
	uint32_t size;
	FOURC type;
	
}StszBox_t;

typedef struct StcoBox{
	uint32_t size;
	FOURC type;
	
}StcoBox_t;

typedef struct StblBox{
	uint32_t size;
	FOURC type;
	StsdBox_t stsd;
	SttsBox_t stts;
	StscBox_t stsc;
	StszBox_t stsz;
	StcoBox_t stco;
}StblBox_t;

typedef struct MinfBox{
	uint32_t size;
	FOURC type;
	MinfHdBox_t minf_hd;
	DinfBox_t dinf;
	StblBox_t stbl;
}MinfBox_t;

typedef struct MediaBox{
	uint32_t size;
	FOURC type;
	MdiaHdBox_t mdia_hd;
	HdRefBox_t hd_ref;
	MinfBox_t minf;
}MediaBox_t;

typedef struct TrackBox{
	uint32_t size;
	FOURC type;
	TrakHdBox_t track_hd;
	MediaBox_t media;
}TrackBox_t;

typedef struct HdlrBox{
	uint32_t size;
	FOURC type;
	uint32_t reserved;
	uint32_t reserved2;
	FOURC dir;
	FOURC app;
	uint32_t reserved3;
	uint32_t reserved4;
	uint8_t reserved5;
}HdlrBox_t;

typedef struct DataBox{
	uint32_t size;
	FOURC type;
	uint32_t ver;
	uint32_t reserved;
	uint8_t lib[10];
}DataBox_t;

typedef struct TooBox{
	uint32_t size;
	FOURC type;
	DataBox_t data;
}TooBox_t;

typedef struct IlistBox{
	uint32_t size;
	FOURC type;
	TooBox_t too;
}IlistBox_t;

typedef struct MetaBox{
	uint32_t size;
	FOURC type;
	uint32_t reserved;
	HdlrBox_t hdlr;
	IlistBox_t ilist;
}MetaBox_t;

typedef struct UdtaBox{
	uint32_t size;
	FOURC type;
	TooBox_t too;
	MetaBox_t meta;
}UdtaBox_t;

typedef struct MovieBox{
	uint32_t size;
	FOURC type;
	MvHdBox_t mvhd;
	TrackBox_t video_track;

	TrackBox_t audio_track;

	UdtaBox_t udta;
}MovieBox_t;

typedef struct MdatBox{
	uint32_t extend_size;
	//FOURC wf;
	//uint32_t size;
	//FOURC type;
	FOURC exten_type;
	uint32_t size;
	FOURC type;
}MdatBox_t;

typedef struct _Mp4FileHeader{
	uint32_t size;
	FileBox_t file_box;	
	MdatBox_t mdat_box;
	//MovieBox_t movie_box;
	
} Mp4FileHeader_t;

typedef struct _Mp4FileTrailer{
	MovieBox_t movie_box;
} Mp4FileTrailer_t;

#define MOV_INDEX_SIZE 36
#define MOV_SYNC_SAMPLE         0x0001
#define MOV_PARTIAL_SYNC_SAMPLE 0x0002

typedef struct MOVIentry {
    unsigned int size;
    uint32_t     pos;
    unsigned int samplesInChunk;
    unsigned int entries;
    int32_t      dts;
    uint32_t     flags;
} MOVIentry;

typedef struct MOVIndex {
    int         mode;
    int         entry;
    unsigned    timescale;
    uint32_t    time;
    int32_t     trackDuration;
    long        sampleCount;
    long        sampleSize;
   // int         hasKeyframes;
#define MOV_TRACK_CTTS         0x0001
#define MOV_TRACK_STPS         0x0002
    uint32_t    flags;
    int         language;
    int         trackID;
    int         tag; ///< stsd fourcc

    uint32_t         vosLen;
    uint8_t     *vosData;
    MOVIentry   *cluster;
    int		bitrate;
    //int         audio_vbr;
    int         height; ///< active picture (w/o VBI) height for D-10/IMX
    //uint32_t    tref_tag;
    //int         tref_id; ///< trackID of the referenced track

    //int         hint_track;   ///< the track that hints this track, -1 if no hint track is set
    //int         src_track;    ///< the track that this hint track describe
    //uint32_t    prev_rtp_ts;
    //int64_t     cur_rtp_ts_unwrapped;
    //uint32_t    max_packet_size;
} MOVTrack;

typedef struct Mp4HdPos_t{
	int64_t mdat_pos;
}Mp4HdPos_t;

typedef struct _Mp4_Info{
	Mp4FileHeader_t *pMp4Header;	
	//Mp4FileTrailer_t *pMp4Trailer;
	//Mp4Index_t Mp4Index[MP4_INDEX_TABLE_NUM];
	int CurIndex; 
	Mp4VdoInfo_t Mp4VideoInfo;
	Mp4AdoInfo_t Mp4AudioInfo;
	MOVTrack Mp4Videotrk;
	MOVTrack Mp4Audiotrk;
	Mp4HdPos_t hd_pos;

	int has_audio;
	int has_gps;
	int has_gsensor;
	char Filename[MP4_FILENAME_LENGTH];
	char IntevalFilename[MP4_FILENAME_LENGTH];
	unsigned char *SkipFrame;
	int SkipFrameLen;
	FIL Mp4File;
	FIL Mp4File_read;
	TickType_t StartTick[MAX_MP4_STREAM_NUM];
	int NumFrame[MAX_MP4_STREAM_NUM];
	uint32_t IndexOffset;
	uint64_t Timestamp;
	WriteBufInfo_t WBInfo;
	BufInitInfo_t WBInitInfo;
	//for skip frame and av sync
	uint64_t pre_vdo_reach_time, vdo_accumlation_duration;
	int cur_ubMP4_H264_PIC, cur_ubMP4_H264_POC;
	uint64_t pre_ado_reach_time, ado_accumlation_duration;
	MP4AVIOContext *pb;
	FIL	*pf;
	//call back function	
	mp4_cb_t hdr_cb;
	mp4_cb_t tra_cb;
	void *cb_parm;
	mp4_cb_t media_start_cb;
	signed char rec_start_ret;
	uint32_t FrmVdoIdx;					/**<  Current frame index */
	uint32_t FrmAdoIdx;					/**<  Current frame index */
	uint32_t FrmIdxNum;					/**<  Total frame index number */
	uint32_t mp4_fb_size;				/**<  estimated frame size */
	uint8_t *pFrmbuf;					/**<  Frame ping pong buffer pointer */
	xTaskHandle FinishCmdtask;	
	xSemaphoreHandle OngoingMutex;	


	int NumData[MAX_DATA_NUM];		//
	AVIGPSINFO_t		*GPS_info;
	AVIGPS_t			GPS;			//avi GPSinfo data 
	AVIGSENSORINFO_t	*GSENSOR_info;
	AVIGSENSOR_t		GSENSOR;		//avi G-Acceleration info data 


}Mp4_Info_t;

typedef struct _MP4_AV_Index{
	int pos;
	int size;
	int type;
	int flag;
}MP4_AV_Index;

typedef struct _Mp4ReadInfo{
	FIL Mp4File;						/**<  file structure for mp4 */
	char Filename[300];					/**<  mp4 file name */

	uint32_t FrmIdx;					/**<  Current frame index */
	uint32_t FrmIdxNum;					/**<  Total frame index number */
	uint32_t mp4_fb_size;				/**<  estimated frame size */
	uint8_t *pFrmbuf;					/**<  Frame ping pong buffer pointer */
	uint8_t IdxRdy;						/**<  mp4 Index load finished and ready to use */
	uint8_t ReadRecordingFlag;			/**<  Set flag when file is recording */
	uint8_t HasAudio;
	MOVTrack Mp4Videotrk;
	MOVTrack Mp4Audiotrk;
	uint8_t *sps;
	uint8_t *pps;
	int sps_len;
	int pps_len;
	MP4_AV_Index *index;
}MP4_read_Info_t;

char* mp4_get_file_name(Mp4_Info_t *pMp4Info);
char* mp4_get_inteval_file_name(Mp4_Info_t *pMp4Info);
void mp4_clear_inteval_file_name(Mp4_Info_t *pMp4Info);
int mp4_set_stream_info(Mp4_Info_t *pMp4Info, Mp4VdoInfo_t *pVdoInfo, Mp4AdoInfo_t *pAdoInfo);
int mp4_write_header(Mp4_Info_t *pMp4Info);
int mp4_write_packet(Mp4_Info_t *pMp4Info ,uint8_t ucMode,uint8_t ucKeyFrame, unsigned char* pFrame, int iFrameSize, uint64_t uiTimeStamp);
int mp4_write_trailer(Mp4_Info_t *pMp4Info);
int mp4_init(Mp4_Info_t *pMp4Info,Mp4WirteBufferInitInfo_t* WriteBufferInit,  unsigned int cap);
void mp4_set_cb(Mp4_Info_t *pMp4Info, mp4_cb_t hdr_cb, mp4_cb_t tra_cb,mp4_cb_t media_cb, void *param);
void mp4_start_finish(Mp4_Info_t *pAviInfo, CMDStatus_t status);
int mp4_uninit(Mp4_Info_t *pMp4Info);

int mp4_ongoing_read_init(Mp4_Info_t *pMp4Info);
int mp4_ongoing_read_video_fps(Mp4_Info_t *pMp4Info, int *fps);
int mp4_ongoing_read_video_framenum(Mp4_Info_t *pMp4Info, int *framenum);
int mp4_ongoing_reset_data_buf(Mp4_Info_t *pMp4Info, void *buf);
int mp4_ongoing_read_frame(Mp4_Info_t *pMp4Info, void **data, uint32_t *size, uint8_t *frm_type);
int mp4_ongoing_seek_frame(Mp4_Info_t *pMp4Info, int time);
int mp4_ongoing_read_uninit(Mp4_Info_t *pMp4Info);

int mp4_read_init(MP4_read_Info_t *pMp4ReadInfo);
int mp4_read_uninit(MP4_read_Info_t *pMp4ReadInfo);
int mp4_read_frame(MP4_read_Info_t *pMp4ReadInfo, void **data, uint32_t *size, uint8_t *frm_type);
int mp4_seek_frame(MP4_read_Info_t *pMp4ReadInfo, int time);
int mp4_read_video_codec(MP4_read_Info_t *pMp4ReadInfo, char *codec);
int mp4_read_video_resolution(MP4_read_Info_t *pMp4ReadInfo, uint32_t *width, uint32_t *height);
int mp4_read_audio_codec(MP4_read_Info_t *pMp4ReadInfo, char *codec);
int mp4_read_audio_simplerate(MP4_read_Info_t *pMp4ReadInfo, uint32_t *simplerate);
int mp4_read_video_fps(MP4_read_Info_t *pMp4ReadInfo, uint32_t *fps);
int mp4_read_video_framenum(MP4_read_Info_t *pMp4ReadInfo, uint32_t *framenum);
int mp4_read_audio_bitrate(MP4_read_Info_t *pMp4ReadInfo, uint32_t *bitrate);
int mp4_reset_data_buf(MP4_read_Info_t *pMp4ReadInfo, void *buf);
char mp4_get_wb_status(Mp4_Info_t *pMp4Info);

#endif
