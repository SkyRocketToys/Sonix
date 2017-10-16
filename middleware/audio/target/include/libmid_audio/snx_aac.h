#ifndef _SNX_AUDIO_AAC_H_
#define _SNX_AUDIO_AAC_H_
//RBK AAC for RTOS +++
//RBK for RTOS typedef conflict -- _FF_INTEGER__
/*
#include "aacenc_lib.h"
#include "aacdecoder_lib.h"
*/
#define DEVICE			"hw:0,0"
#define CHANNEL			1
#define FORMAT			SND_PCM_FORMAT_S16_LE
#define PER_SAMPLE_BYTE		2
#define CHANNEL_MODE		MODE_1
/*
CHANNEL_ORDER:
0: MPEG channel ordering
1: WAVE file format channel ordering
*/
#define CHANNEL_ORDER		1
/*
TRANSPORT_TYPE:
TT_MP4_RAW: raw access units
TT_MP4_ADIF: ADIF bitstream format
TT_MP4_ADTS: ADTS bitstream format
TT_MP4_LATM_MCP1: Audio Mux Elements (LATM) with muxConfigPresent = 1
TT_MP4_LATM_MCP0: Audio Mux Elements (LATM) with muxConfigPresent = 0, out of band StreamMuxConfig
TT_MP4_LOAS: Audio Sync Stream (LOAS)
*/
#define TRANSPORT_TYPE		TT_MP4_ADTS


/*
ERR_CONCEAL: Error concealment method
0: Spectral muting.
1: Noise substitution (see CONCEAL_NOISE).
2: Energy interpolation (adds additional signal delay of one frame, see CONCEAL_INTER).
*/
#define ERR_CONCEAL			2

/*
DRC_BOOST: Dynamic Range Control: boost, where [0] is none and [127] is max boost
*/
#define DRC_BOOST			0

/*
DRC_CUT: Dynamic Range Control: attenuation factor, where [0] is none and [127] is max compression
*/
#define DRC_CUT				0

/*
DRC_LEVEL: Dynamic Range Control: reference level, quantized to 0.25dB steps where [0] is 0dB and [127] is -31.75dB
*/
#define DRC_LEVEL			0

/*
DRC_HEAVY: Dynamic Range Control: heavy compression, where [1] is on (RF mode) and [0] is off
*/
#define DRC_HEAVY			0

/*
LIMIT_ENABLE: Signal level limiting enable
-1: Auto-config. Enable limiter for all non-lowdelay configurations by default.
0: Disable limiter in general.
1: Enable limiter always. It is recommended to call the decoder with a AACDEC_CLRHIST flag to reset all states when the limiter switch is changed explicitly.
*/
#define LIMIT_ENABLE			-1

/*git
1024 or 960 for AAC-LC
2048 or 1920 for HE-AAC (v2)
512 or 480 for AAC-LD and AAC-ELD
*/
#define PCM_BUF_SIZE			(2048 * CHANNEL * PER_SAMPLE_BYTE)

#define AAC_BUF_SIZE			768


//typedef struct AACENC_InfoStruct AACENC_InfoStruct;
/*
struct AAC_INFO_ST
{
	unsigned int sample_rate;
	unsigned int aot;
	unsigned int afterburner;
	unsigned int eld_sbr;
	unsigned int vbr;
	unsigned int bitrate;
	unsigned int bits_per_sample;
	unsigned int channels;
	struct AACENC_InfoStruct *enc_info;
};
*/
//RBK start here
struct snx_aac_info_st
{
	int32_t  type;
	uint32_t samplerate;
	uint32_t bitrate;
	uint32_t bytes_per_frame;
	uint32_t pcm_bytes_per_frame;
	uint32_t samples_per_frame;
    uint32_t aot;
    uint32_t afterburner;
    uint32_t eld_sbr;
    uint32_t vbr;
    uint32_t bits_per_sample;
    uint32_t channels;
    //struct AACENC_InfoStruct *enc_info;
    uint32_t encMaxOutBufBytes;
	uint32_t encFrameLength;
};

struct snx_aac_params_st
{
	int32_t  type;
    uint32_t samplerate;
    uint32_t bitrate;
    uint32_t aot;
    uint32_t afterburner;
    uint32_t eld_sbr;
    uint32_t vbr;
    uint32_t bits_per_sample;
    uint32_t channels;
};

struct snx_aac_st;


#define IN
#define OUT
#define INOUT

int32_t snx_aac_open     (IN struct snx_aac_params_st *aac_params, OUT struct snx_aac_st **aac_ctl );
int32_t snx_aac_close    (IN struct snx_aac_st *aac_ctl);
int32_t snx_aac_encode   (IN struct snx_aac_st *aac_ctl, INOUT uint8_t *p_src, INOUT uint8_t *p_dst, IN  int32_t src_bytes, INOUT int32_t *p_dst_bytes);
int32_t snx_aac_decode   (IN struct snx_aac_st *aac_ctl, INOUT uint8_t *p_src, INOUT uint8_t *p_dst, IN  int32_t src_bytes, INOUT int32_t *p_dst_bytes);
int32_t snx_aac_get_info (IN struct snx_aac_st *aac_ctl, OUT   struct snx_aac_info_st *aac_info);

#endif //_SNX_AUDIO_AAC_H_
