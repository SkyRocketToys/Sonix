/**
 * @file
 * this is application header file for audio capture, include this file before use
 * @author Algorithm Dept Sonix.
 */

#ifndef __MF_AUDIO_H__
#define __MF_AUDIO_H__


#include <libmid_audio/audio.h>
#include <libmid_audio/audio_codec.h>
#include <sys/time.h>
#include <libmid_nvram/snx_mid_nvram.h>

#define AAC_ENABLE	1
#define MSG_TONE_ENABLE   0

#if AAC_ENABLE
#include <libmid_audio/snx_aac.h>
#else
#include <libmid_audio/snx_alaw.h>
#endif

#define ADO_PRINT(level, fmt, args...) print_q(level, "[audio]%s: "fmt, __func__,##args)


#define NVRAM_PKG_AUDIO_ISP         	"App_ISP"
#define NVRAM_CFG_AUDIO_VOICE      	    "record_voice"
#define NVRAM_AUDIO_TONE				"MSG_TONE"

#define MAX_AUDIO_PACKET 4	/**<  max buffer number for saving audio frame*/
#define MAX_AUDIO_PLYA_QUEUE 5

//#define AAC_SAMPLE_BYTE     2
#define AAC_BLOCK			0
#define AAC_SMAPLE_RATE 	11025
//#define AAC_BUF_SIZE		8192
//#define AAC_BUF_THRESHOLD	1024
#define AAC_BITRATE			15999



typedef void (*audio_notice_t)(unsigned char *, unsigned int, struct timeval);

/**
* @brief structure for audio infomation
*/
typedef struct _audio_info {
	uint8_t type;			/**<  set AUD_CAP_STREAM for audio capture*/
	int32_t format;			/**<  audio format AUD_FORMAT_S16_LE, AUD_FORMAT_A_LAW or AUD_FORMAT_MU_LAW...*/
	int32_t BitsPerSample;	/**<  bits per sample */
	int32_t SampleRate;		/**<  audio sample rate*/
	int32_t BitRate;		/**<  bit rate*/
	int32_t BufSize;		/**<  buffer size which is parameter to middleware*/ //must larger tahn BufThreshold in block mode to avoid buf overflow(=8192*2^n)
	int32_t BufThreshold;	/**<  threshold which is parameter to middleware*/ //must larger than PacketSize in block mode(=256*2^n)
	int32_t PacketSize;		/**<  size of audio frame*/
	uint8_t *pPacketSpace;	/**<  pointer to dynamic allocated audio buffer*/
	uint8_t *Packet[MAX_AUDIO_PACKET];	/**<  pointer for each buffer*/

	unsigned int time_interval;		/**<  audio info time interval */

	audio_notice_t notice;			/**<  function pointer to get each audio frame*/
	struct audio_stream_st *pAudioStream;	/**<  structure for middleware audio infomation*/
	void *pCodec;					/**< support AAC (struct snx_aac_st *) or Alaw (struct snx_alaw_st *)*/
	xQueueHandle queue_empty;		/**<  queue for empty buffer ready to saving audio frame*/
	xQueueHandle queue_ready;		/**<  queue for buffer has audio frame*/
	xTaskHandle task_get;			/**<  task to get audio data*/
	xTaskHandle	task_notic;			/**<  task to notic audio frame complete*/
	int32_t		aac_used;			/**<  1:means using AAC; 0:means using ALAW */
} audio_info_t;

/**
* @brief structure to report current audio infomation
*/
typedef struct _audio_param {
	unsigned int uiFormat;
	unsigned int uiSampleRate;
	unsigned char ucBitsPerSample;
	unsigned int uiPacketSize;
	unsigned int  uiBitRate;
} audio_param_t;


/**
* @brief structure for audio out information
*/
typedef struct audio_out_info_st {
	int32_t type;
	int32_t format;
	int32_t codec;
	int32_t is_block;
	uint32_t rate;
	uint32_t size;
	uint32_t bitrate;
	uint8_t *buf_dec;
	uint8_t *buf_aac;
	char filename[32];
	struct params_st *aud_dev_param;
	struct audio_stream_st *audio_stream;
	struct snx_aac_params_st aac_para;
	nvram_file_t *fp;
	QueueHandle_t play_queue;
	xTaskHandle task_out;
} audio_out_info_st;

typedef struct _finish_buf {
	uint8_t *pbuf;
	int size;
} finish_buf_t;


int mf_audio_init(void);
void mf_audio_uninit(void);
void mf_audio_get_param(audio_param_t *pv_param);
void mf_audio_set_record_cb(audio_notice_t notice_function);
void mf_audio_set_record_voice(int level);
int mf_audio_out_init(void);
void mf_audio_out_unint(void);
void aac_tone_play(char *filename);

void task_audio_get(void *pvParameters);
void task_audio_out(void *pvParameters);



extern struct snx_aac_st *p_aac_st;
extern uint8_t ucFlashAudioBuf;

#endif
