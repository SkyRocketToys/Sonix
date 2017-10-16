#ifndef __AVI_HEADER_LIB_H__
#define __AVI_HEADER_LIB_H__


#define ATOM_HEAD_SIZE		8	//ulList+ulSize or ulFourCC+ulSize
#define INDEX_SIZE(x)		((x)<<4)
#define MAX_INDEX_SIZE(v,a) 		INDEX_SIZE(MAX_FRAME_NUM(v,a))
#define INDEX_BLOCK_SIZE(x)	(ATOM_HEAD_SIZE+INDEX_SIZE(x))



#define AVI_HASINDEX		0x00000010 // Index at end of file?
#define AVI_MUSTUSEINDEX	0x00000020
#define AVI_ISINTERLEAVED	0x00000100
#define AVI_TRUSTCKTYPE 	0x00000800 // Use CKType to find key frames
#define AVI_WASCAPTUREFILE	0x00010000
#define AVI_COPYRIGHTED 	0x00020000 

#define  AVI_KEYFRAME		0x00000010

#define AVI_WAVE_FORMAT_PCM			0x0001 /* PCM */
#define AVI_WAVE_FORMAT_ALAW		0x0006 /* ALAW */
#define AVI_WAVE_FORMAT_MULAW		0x0007 /* MULAW */
#define AVI_WAVE_FORMAT_AAC			0x00FF 


enum
{
	AVI_GPS=0,
	AVI_GSENSOR,
	MAX_DATA_NUM
};


enum
{
	AVI_P_FRAME = 0,	
	AVI_I_FRAME,
	AVI_SKIP_FRAME,
};


typedef uint32_t FOURCC;

typedef  struct  LIST{
	FOURCC		ulList;	//"LIST"
	uint32_t	ulSize;
	FOURCC		ulFourCC;
	//BYTE	data[ulSize-4]   //  contains  Lists  and  Chunks
}LIST_t;

typedef struct  CHUNK{
	FOURCC	ulFourCC;
	uint32_t	ulSize;
	//BYTE	data[ulSize]       //  contains  headers  or  video/audio  data
}CHUNK_t;

typedef struct RECT
{
	uint16_t left;
	uint16_t top;
	uint16_t right;
	uint16_t bottom;
}RECT_t;


typedef struct MainAVIHeader
{
	//CHUNK_t chunk;
	FOURCC		ulFourCC;
	uint32_t	ulSize;
	struct {	
		uint32_t  ulMicroSecPerFrame;  //  frame  display  rate  (or  0)
		uint32_t  ulMaxBytesPerSec;  //  max.  transfer  rate
		uint32_t  ulPaddingGranularity;  //  pad  to  multiples  of  this size;
		uint32_t  ulFlags;  //  the  ever-present  flags
		uint32_t  ulTotalFrames;  //  #  frames  in  file
		uint32_t  ulInitialFrames;
		uint32_t  ulStreams;
		uint32_t  ulSuggestedBufferSize;
		uint32_t  ulWidth;
		uint32_t  ulHeight;
		uint32_t  ulReserved[4];
	}info;
}MainAVIHeader_t;


typedef  struct AVIStreamHeader
{
	//CHUNK_t chunk;
	FOURCC		ulFourCC;
	uint32_t	ulSize;
	struct {
		FOURCC	fccType;	// "vids" for video,  "auds" for audio
		FOURCC	fccHandler;	//"h264" for h.264
		uint32_t	ulFlags;
		uint16_t	usPriority;
		uint16_t	usanguage;
		uint32_t	ulInitialFrames;
		uint32_t	ulScale;
		uint32_t	ulRate; /*  ulRate  /  ulScale  ==  samples/second  */
		uint32_t	ulStart;
		uint32_t	ulLength; /*  In  units  above...  */
		uint32_t	ulSuggestedBufferSize;
		uint32_t	ulQuality;
		uint32_t	ulSampleSize;
		RECT_t	rcFrame;
	}info;
} AVIStreamHeader_t;

//video format
typedef struct BitMapInfoHeader
{
	//CHUNK_t chunk;
	FOURCC		ulFourCC;
	uint32_t	ulSize;
	struct {
		uint32_t	ulSize;
		uint32_t	ulWidth;
		uint32_t	ulHeight;
		uint16_t	usPlanes;
		uint16_t	usBitCount;
		uint32_t 	ulCompression;
		uint32_t 	ulSizeImage;
		uint32_t 	ulXPelsPerMeter;
		uint32_t 	ulYPelsPerMeter;
		uint32_t 	ulClrUsed;		
		uint32_t	ulClrImportant;
		//BYTE 	extradata[24];	//???
	}info;
} BitMapInfoHeader_t;

typedef struct WaveForMatex
{
	//CHUNK_t chunk;
	FOURCC		ulFourCC;
	uint32_t	ulSize;
	struct {
		uint16_t	usFormatTag;
		uint16_t	usChannels;
		uint32_t	ulSamplesPerSec;
		uint32_t	ulAvgBytesPerSec;
		uint16_t	usBlockAlign;
		uint16_t	usBitsPerSample;
		//uint16_t	usSamplesPerBlock;	
	}info;	
} WaveForMatex_t;


typedef struct AVIIndexEntry{
	uint32_t	ulckid;
	uint32_t	ulFlags;
	uint32_t	ulChunkOffset;
	uint32_t	ulChunkLength;
}AVIIndexEntry_t;

typedef struct AVIGPSINFO{ 			// Total 28 bytes.
	uint32_t	ulChunkID; 		// Video frame index. 
	
	uint8_t		ucGPSStatus;
	int16_t		ssLonDegInt;
	uint32_t	ulLonDegDec;
	int16_t		ssLatDegInt;
	uint32_t	ulLatDegDec;
	uint16_t	usAltitude;
	uint16_t	usSpeed;
	uint8_t		ucGpsHour;
	uint8_t		ucGpsMinute;
	uint8_t		ucGpsSecond;
	uint16_t	usGpsYear;
	uint8_t		ucGpsMonth;
	uint8_t		ucGpsDay;
}__attribute__((packed))AVIGPSINFO_t;

typedef struct AVIGSENSORINFO //Total 20 bytes.
{
	uint32_t	ulChunkID; 		// Video frame index. 

	uint8_t		ucAcceRange;
	uint8_t		ucGVInt_X;
	uint32_t	ulGVDec_X;
	uint8_t		ucGVInt_Y;
	uint32_t	ulGVDec_Y;
	uint8_t		ucGVInt_Z;
	uint32_t	ulGVDec_Z;
}__attribute__((packed))AVIGSENSORINFO_t;

typedef struct _AVIFileHeader{
	LIST_t RIFF;
	struct{
		LIST_t hdrl;
		MainAVIHeader_t avih;
		struct{
			LIST_t strl;
			AVIStreamHeader_t strh;
			BitMapInfoHeader_t strf;	//video froamt
		} s1;			
		struct{
			LIST_t strl;
			AVIStreamHeader_t strh;
			WaveForMatex_t strf;

		} s2;
	}head;
	LIST_t movi;
//	AVIOLDINDEX idx1;
} AVIFileHeader_t;


typedef struct{
	CHUNK_t idx1;
	AVIIndexEntry_t *idxentry;
}AVIIndex_t;


typedef struct{
	CHUNK_t chunk_header;
	AVIGPSINFO_t *entry;
}AVIGPS_t;

typedef struct{
	CHUNK_t chunk_header;
	AVIGSENSORINFO_t *entry;
}AVIGSENSOR_t;

#endif //__AVI_HEADER_LIB_H__
