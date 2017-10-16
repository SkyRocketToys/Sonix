#ifndef SNX_AEC_H
#define SNX_AEC_H


#ifdef __cplusplus
extern "C" {
#endif

#include <FreeRTOS.h>
#include <semphr.h>

/* Return error code */
#define AEC_SUCCESS		0
#define AEC_ERR_BASE	100
#define AEC_ERR_BASE_INVALID_FORMAT			(-(AEC_ERR_BASE + 1)) //-101
#define AEC_ERR_BASE_INVALID_PARAMETER		(-(AEC_ERR_BASE + 2)) //-102
#define AEC_ERR_BASE_INSUFFICIENT_RESOUCE	(-(AEC_ERR_BASE + 3)) //-103
#define AEC_ERR_BASE_MEM_ALIGN				(-(AEC_ERR_BASE + 4)) //-104
#define AEC_ERR_BASE_CODEC_FAIL				(-(AEC_ERR_BASE + 5)) //-105
#define AEC_ERR_BASE_ALREADY_OPEN			(-(AEC_ERR_BASE + 6)) //-106

#define AEC_BLOCK_SAMPLES		256 //AEC algo. limitation
#define AAC_PCM_FRAME_SAMPLES	1024 //AAC algo. limitation
#define	AEC_PIECE_FRAME_NUM		5	//normal frames in one piece
#define	AAC_PIECE_FRAME_NUM		1	//AAC just one frame(2048bytes) in one piece

/* AEC Rate control default parameters */
#define AUDIO_AEC_RATECTRL_BASETIME			850000 //us

/*	8000/256=31.25 loops, 
	AUDIO_AEC_RATECTRL_BASETIME / 31.25 = 27200 us/per frame */
#define AUDIO_AEC_TXCTRL					27200	//default, depend on codec

/* (16000 * 2 / 16 compress_ratio ) / 40 bytes_per_frame = 50 loops, 
	AUDIO_AEC_RATECTRL_BASETIME / 50 	= 17000 us/per frame */
#define AUDIO_AEC_RXCTRL					17000	//default, depend on codec
#define AUDIO_AEC_RX_INIT_DELAY				0		//playback initial delay, us
#define AUDIO_AEC_TX_INIT_DELAY				100000	//capture initial delay, us
#define AUDIO_AEC_TX_RESET_DELAY			0 		//us, not be used
#define AUDIO_AEC_RX_RESET_PAD_ZERO_FRAMES	5		//when AEC reset occur, ref data will how many pad zero frames

#define AEC_CODEC_DISABLE					NULL

#define AEC_DEBUG				0
/* AEC_DEBUG will dump mic/reference pcm raw data before AEC and output pcm raw data after AEC */
#if AEC_DEBUG
#define	AEC_DEBUG_MIC_PATH 	"testMic.pcm"
#define	AEC_DEBUG_REF_PATH 	"testRef.pcm"
#define	AEC_DEBUG_OUT_PATH 	"testOut.pcm"
#endif

enum AECSwitch {
	AEC_DISABLE = 0,
	AEC_ENABLE
};

enum AECTaskStopHandle {
	AEC_TASK_RELEASE = -1,
	AEC_TASK_HOLD = 0
};

typedef void (*aec_audio_cb)(const struct timeval *tstamp, void *data, size_t len,void *cbarg);

struct snx_aec_params_st {
/***************** Required ******************/	
	/* Process type: 
		AUD_CAP_STREAM: 
		 output size = 2560 bytes / compress_ratio
		 (2560 = AEC_BLOCK_SAMPLES * format_size * AEC_PIECE_FRAME_NUM)
		AUD_PLY_STREAM:
		 required encoded input size = 40 bytes * N
	*/
	uint32_t type;
	
	/* Sample rate */
	uint32_t sample_rate;
	
	/* bit rate */
	uint32_t bit_rate;
	
	/* Data format in encoded buffer */
	/*	AUD_FORMAT_AUD32, compress_ratio = 16
		AUD_FORMAT_G726, compress_ratio = 8
		AUD_FORMAT_A_LAW, compress_ratio = 2
		AUD_FORMAT_MU_LAW, compress_ratio = 2
		AUD_FORMAT_AAC, compress_ratio = ~16
		AEC_CODEC_DISABLE, compress_ratio = 1 */
	uint32_t codec;
	
	/* How many bytes per sample */
	uint32_t format_size; //16bit:2, 8bit:1
	
	/* audio stream interface */
	struct audio_stream_st *audio_stream;
	
/*************** Capture Only ****************/	
	/* app callback function in capture process */
	aec_audio_cb cb;
	void *cbarg;
	
/***************** Optional ******************/
	/* Delay parameters auto setting or user define */
	uint8_t delay_userdef; //AEC_ENABLE or AEC_DISABLE
	
	/* Run time delay */
	uint32_t delay_ctrl;

	/* Init delay */
	uint32_t delay_init;

	/* Reset delay (pad zero frames) */
	uint32_t delay_reset;
	
	/* AGC arguments */
	struct snx_agc_params_st *agc_arg;
};

struct snx_aec_st {
	uint32_t type;
	uint32_t codec;
	void *codec_st;
	uint32_t format_size;
	struct audio_stream_st *audio_stream;
	uint32_t compress_ratio;
	/* Encoded data buffer */
	uint8_t *buffer;
	uint32_t buffer_len;
	uint32_t data_len;
	/* RAW data buffer */
	uint8_t *pcm_buf;
	uint32_t pcm_buf_len;
	uint32_t pcm_data_len;
/*************** Capture Only ****************/	
	aec_audio_cb cb;
	void *cbarg;
	/* AGC arguments */
	uint32_t agc_enable;
};

#define IN
#define OUT
#define INOUT

int32_t snx_aec_open	(IN struct snx_aec_params_st *params, OUT struct snx_aec_st **aec );
int32_t snx_aec_start	(IN struct snx_aec_st *aec);
void	snx_aec_process	(IN struct snx_aec_st *aec, IN enum AECSwitch *aec_enable);
void 	snx_aec_stop	(IN struct snx_aec_st *aec, IN enum AECTaskStopHandle stopHandle);
void 	snx_aec_close	(IN struct snx_aec_st *aec);
/* snx_aec_fill_in_data suits for Playback fill in encoded data and decode to raw data for play */
int32_t snx_aec_fill_in_data	(IN struct snx_aec_st *aec, IN uint8_t* data, IN uint32_t len);

#ifdef __cplusplus 
}
#endif

#endif /* SNX_AEC_H */
