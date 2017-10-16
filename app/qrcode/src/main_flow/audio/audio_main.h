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

#define AAC_ENABLE 1

#if AAC_ENABLE
#include <libmid_audio/snx_aac.h>
#else
#include <libmid_audio/snx_alaw.h>
#endif

#define NVRAM_PKG_AUDIO_ISP         	"App_ISP"
#define NVRAM_CFG_AUDIO_VOICE      	    "record_voice"

#define MAX_AUDIO_PACKET 4	/**<  max buffer number for saving audio frame*/

typedef void (*audio_notice_t)(unsigned char*, unsigned int, struct timeval);

/**
* @brief structure for audio infomation
*/
typedef struct _audio_info{
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
	audio_notice_t notice;	/**<  function pointer to get each audio frame*/
	struct audio_stream_st *pAudioStream;	/**<  structure for middleware audio infomation*/
#if AAC_ENABLE
	struct snx_aac_st *pCodec;
#else
	struct snx_alaw_st *pCodec;
#endif
	xQueueHandle queue_empty;		/**<  queue for empty buffer ready to saving audio frame*/	
	xQueueHandle queue_ready;		/**<  queue for buffer has audio frame*/	
	xTaskHandle task_get;			/**<  task to get audio data*/	
	xTaskHandle	task_notic;			/**<  task to notic audio frame complete*/		
}audio_info_t;

/**
* @brief structure to report current audio infomation 
*/
typedef struct _audio_param{
	unsigned int uiFormat;			
	unsigned int uiSampleRate;		
	unsigned char ucBitsPerSample;	
	unsigned int uiPacketSize;	
	unsigned int  uiBitRate;
}audio_param_t;


int mf_audio_init(void);
void mf_audio_uninit(void);
void mf_audio_get_param(audio_param_t* pv_param);
void mf_audio_set_record_cb(audio_notice_t notice_function);
void mf_audio_set_record_voice(int level);

#endif
