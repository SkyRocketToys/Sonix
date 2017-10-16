/**
 * @file
 * this is application file for audio data capture
 * @author Algorithm Dept Sonix.
 */
#include <FreeRTOS.h>
#include <task.h>
#include <bsp.h>
#include <nonstdlib.h>
#include <string.h>
#include <stdio.h>
#include <queue.h>
#include <semphr.h>
#include <sys/time.h>
#include <libmid_nvram/snx_mid_nvram.h>
#include "audio_main.h"


#define ADO_PRINT(fmt, args...) print_msg("[mf_audio]%s "fmt, __func__,  ##args)
#define ADO_PRINT_QUEUE(fmt, args...) print_msg_queue("[mf_audio]%s: "fmt, __func__,##args)

#if 0
typedef struct _AAC_INFO_ST
{
	unsigned int sample_rate;
	unsigned int aot;
	unsigned int afterburner;
	unsigned int eld_sbr;
	unsigned int vbr;
	unsigned int bitrate;
	unsigned int bits_per_sample;
	unsigned int channels;
	//unsigned int high_quality;
	AACENC_InfoStruct enc_info;
}AAC_INFO_ST;
#endif

typedef struct _finish_buf
{
	uint8_t * pbuf;
	int size;

}finish_buf_t;



audio_info_t audio_info;
/*
audio_info_t audio_info = {
	.type = AUD_CAP_STREAM,
	.format = AUD_FORMAT_A_LAW,
	.BitsPerSample = 8,
	.SampleRate = 8000,
	.PacketSize = 160,
	.BufThreshold = 256,	
	.BufSize = 8192,
};*/

void task_audio_get( void *pvParameters );
void task_audio_notice( void *pvParameters );

/**
* @brief interface function - audio capture initialization
* @return return pdPASS if success
*/
int mf_audio_init(void)
{

	memset(&audio_info, 0, sizeof(audio_info_t));
	audio_info.type = AUD_CAP_STREAM;
#if AAC_ENABLE
	audio_info.format = AUD_FORMAT_AAC;//
	audio_info.BitsPerSample = 16;
	audio_info.SampleRate = 11025;
	audio_info.BitRate = 15999;
	audio_info.PacketSize = 2048;
	audio_info.BufThreshold = 256;	
	audio_info.BufSize = 8192<<1;
#else
	audio_info.format = AUD_FORMAT_A_LAW;//
	audio_info.BitsPerSample = 8;
	audio_info.SampleRate = 8000;
	audio_info.BitRate = audio_info.SampleRate*audio_info.BitsPerSample;
	audio_info.PacketSize = 800;//160;	//20ms
	audio_info.BufThreshold = 256;	
	audio_info.BufSize = 8192;
#endif
	//audio_info.notice = recordaudio;
	audio_info.notice = NULL;
	ADO_PRINT_QUEUE("sample rate = %d, bps = %d, packet = %d, buf = %d, thr = %d\n", 
		audio_info.SampleRate, audio_info.BitsPerSample, audio_info.PacketSize, 
		audio_info.BufSize, audio_info.BufThreshold);

	//get queue
	audio_info.queue_empty = xQueueCreate(MAX_AUDIO_PACKET, sizeof(uint8_t *));
	if(NULL == audio_info.queue_empty)
	{
		ADO_PRINT_QUEUE("queue queue_empty create fail\n");
		goto fail4;
	}

	audio_info.queue_ready = xQueueCreate(MAX_AUDIO_PACKET, sizeof(finish_buf_t));
	if(NULL == audio_info.queue_ready)
	{
		ADO_PRINT_QUEUE("queue queue_ready create fail\n");
		goto fail3;
	}
	//start task to get audio frame
	if (pdPASS != xTaskCreate(task_audio_get, "ado_get", STACK_SIZE_16K, NULL,
			PRIORITY_TASK_APP_AUDIO_GET, &audio_info.task_get))
	{
		ADO_PRINT_QUEUE("Could not create task audio get\n");
		goto fail2;
	}	

	//start task to notice audio finish
	if (pdPASS != xTaskCreate(task_audio_notice, "ado_notice", STACK_SIZE_1K, NULL,
			PRIORITY_TASK_APP_AUDIO_GET, &audio_info.task_notic))
	{
		ADO_PRINT_QUEUE("Could not create task audio notice\n");
		goto fail1;
	}	


	return pdPASS;


	//vTaskDelete(audio_info.task_get);
fail1:	
	vTaskDelete(audio_info.task_notic);	
fail2:	
	vQueueDelete(audio_info.queue_ready);
fail3:	
	vQueueDelete(audio_info.queue_empty);
fail4:	
	return pdFAIL;
}

/**
* @brief interface function - video capture  uninitialization
*/
void mf_audio_uninit(void)
{
	ADO_PRINT_QUEUE("\n");
#if AAC_ENABLE
	if(audio_info.pCodec)
	{
		snx_aac_close(audio_info.pCodec);
		audio_info.pCodec=NULL;

	}
#else
	if(audio_info.pCodec)
	{
		snx_alaw_close(audio_info.pCodec);
		audio_info.pCodec=NULL;
	}
#endif
    if(audio_info.pAudioStream)
    {
		audio_drain(audio_info.pAudioStream);
		audio_close(audio_info.pAudioStream);
		audio_info.pAudioStream=NULL;

    }
	if(audio_info.pPacketSpace)
	{
		vPortFree(audio_info.pPacketSpace);
		audio_info.pPacketSpace=NULL;

	}
	if(audio_info.task_get)
	{
		vTaskDelete(audio_info.task_get);
		audio_info.task_get=NULL;

	}
	if(audio_info.task_notic)
	{
		vTaskDelete(audio_info.task_notic);
		audio_info.task_notic=NULL;

	}
	if(audio_info.queue_ready)
	{
		vQueueDelete(audio_info.queue_ready);
        audio_info.queue_ready=NULL;
	}
	if(audio_info.queue_empty)
	{
		vQueueDelete(audio_info.queue_empty);
		audio_info.queue_empty=NULL;

	}
}

#if 0
static int mf_aac_encoder_init(HANDLE_AACENCODER *phandle, AAC_INFO_ST *paac_info)
{
	int retval = -1;

	unsigned int sample_rate = paac_info->sample_rate;
	unsigned int aot = paac_info->aot;
	unsigned int afterburner = paac_info->afterburner;
	unsigned int eld_sbr = paac_info->eld_sbr;
	unsigned int vbr = paac_info->vbr;
	unsigned int bitrate = paac_info->bitrate;
//	unsigned int bits_per_sample = paac_info->bits_per_sample;
	unsigned int channels = paac_info->channels;

	unsigned int mode = 0;

	switch (channels) {
	case 1: mode = MODE_1;       break;
	case 2: mode = MODE_2;       break;
	case 3: mode = MODE_1_2;     break;
	case 4: mode = MODE_1_2_1;   break;
	case 5: mode = MODE_1_2_2;   break;
	case 6: mode = MODE_1_2_2_1; break;
	default:
		print_msg( "Unsupported channels %d\n", channels);
		return retval;
	}
	//RBK test code
/*
	if (aacEncOpen(phandle, 0x01|0x02|0x10, channels) != AACENC_OK) {
		print_msg( "Unable to open encoder\n");
		return retval;
	}
*/
	if (aacEncOpen(phandle, 0x01, channels) != AACENC_OK) {
		ADO_PRINT_QUEUE( "Unable to open encoder\n");
		return retval;
	}
	if (aacEncoder_SetParam(*phandle, AACENC_AOT, aot) != AACENC_OK) {
		ADO_PRINT_QUEUE( "Unable to set the AOT\n");
		goto err;
	}
	if (aot == 39 && eld_sbr) {
		if (aacEncoder_SetParam(*phandle, AACENC_SBR_MODE, eld_sbr) != AACENC_OK) {
			ADO_PRINT_QUEUE( "Unable to set SBR mode for ELD\n");
			goto err;
		}
	}
	if (aacEncoder_SetParam(*phandle, AACENC_SAMPLERATE, sample_rate) != AACENC_OK) {
		ADO_PRINT_QUEUE( "Unable to set the AOT\n");
		goto err;
	}
	if (aacEncoder_SetParam(*phandle, AACENC_CHANNELMODE, mode) != AACENC_OK) {
		ADO_PRINT_QUEUE( "Unable to set the channel mode\n");
		goto err;
	}
	if (aacEncoder_SetParam(*phandle, AACENC_CHANNELORDER, 1) != AACENC_OK) {
		ADO_PRINT_QUEUE( "Unable to set the wav channel order\n");
		goto err;
	}
	if (vbr) {
		if (aacEncoder_SetParam(*phandle, AACENC_BITRATEMODE, vbr) != AACENC_OK) {
			ADO_PRINT_QUEUE( "Unable to set the VBR bitrate mode\n");
			goto err;
		}
	} else {
		if (aacEncoder_SetParam(*phandle, AACENC_BITRATE, bitrate) != AACENC_OK) {
			ADO_PRINT_QUEUE( "Unable to set the bitrate\n");
			goto err;
		}
	}
	if (aacEncoder_SetParam(*phandle, AACENC_TRANSMUX, 2) != AACENC_OK) {
		ADO_PRINT_QUEUE( "Unable to set the ADTS transmux\n");
		goto err;
	}
	if (aacEncoder_SetParam(*phandle, AACENC_AFTERBURNER, afterburner) != AACENC_OK) {
		ADO_PRINT_QUEUE( "Unable to set the afterburner mode\n");
		goto err;
	}
	if (aacEncEncode(*phandle, NULL, NULL, NULL, NULL) != AACENC_OK) {
		ADO_PRINT_QUEUE( "Unable to initialize the encoder\n");
		goto err;
	}

	if (aacEncInfo(*phandle, &(paac_info->enc_info)) != AACENC_OK) {
		ADO_PRINT_QUEUE( "Unable to get the encoder info\n");
		goto err;
	}

	retval = 0;
	return retval;

err:
	aacEncClose(phandle);
	return retval;

}
#endif


/**
* @brief task for aduio capture and write infomation to queue_ready
*/
void task_audio_get( void *pvParameters )
{

	uint8_t *pbuf, *pPacketBuf,*PCMPacketBuf=NULL;
	int32_t retval=0, status, size;
	struct params_st *params;	
	int i, count=0, enc_size;
	unsigned int CurTime, lastTime;
	int err;
	finish_buf_t finish_buf;
	int voicelevel;
#if AAC_ENABLE
	struct snx_aac_params_st CodecParams;
#else
	struct snx_alaw_params_st CodecParams;
#endif

	audio_info.pPacketSpace = pvPortMalloc(audio_info.PacketSize*MAX_AUDIO_PACKET, GFP_DMA, MODULE_APP);
	//ADO_PRINT_QUEUE("Packet = 0x%x  PacketSize = %d\n", audio_info.Packet, audio_info.PacketSize);
	if(audio_info.pPacketSpace==NULL)
	{
		ADO_PRINT_QUEUE("alloc audio_info.Packet fail");
		goto fail;
	}

	PCMPacketBuf = pvPortMalloc(audio_info.PacketSize, GFP_DMA, MODULE_APP);
	if(PCMPacketBuf ==NULL)
	{
		ADO_PRINT_QUEUE("alloc PCMPacketBuf Packet fail");
		vPortFree(audio_info.pPacketSpace);
		goto fail;
	}

	
	for(i=0;i<MAX_AUDIO_PACKET;i++)
		audio_info.Packet[i] = audio_info.pPacketSpace + i * audio_info.PacketSize;

	//send index of pack num to empty queue
	for(i=0;i<MAX_AUDIO_PACKET;i++)
	{
		
		xQueueSendToBack( audio_info.queue_empty , &audio_info.Packet[i], 0 ); 
		ADO_PRINT_QUEUE("pack[%d] = 0x%x\n",i, audio_info.Packet[i]);
	}


	retval = audio_open(&audio_info.pAudioStream, audio_info.type);
	if(retval < 0)
	{
		ADO_PRINT_QUEUE("Failed to open device.\n");
		goto fail;
	}

	retval = audio_params_alloca(&params);
	if(retval < 0)
	{
		ADO_PRINT_QUEUE("Failed to allocate memory.\n");
		retval = -4;
		goto fail;
	}	

	ADO_PRINT_QUEUE("set rate = %d, fmt = %d, buf size = %d, threshold = %d\n", audio_info.SampleRate, audio_info.format, 
							audio_info.BufSize, audio_info.BufThreshold);
	audio_params_set_rate(audio_info.pAudioStream, params, audio_info.SampleRate);
	audio_params_set_format(audio_info.pAudioStream, params, AUD_FORMAT_S16_LE);
	audio_params_set_buf_size(audio_info.pAudioStream, params, audio_info.BufSize);
	audio_params_set_buf_threshold(audio_info.pAudioStream, params, audio_info.BufThreshold);
	retval = audio_set_params(audio_info.pAudioStream, params);
	if(retval < 0)
	{
		ADO_PRINT_QUEUE("Failed to set paramters.\n");
		goto fail;
	}	
	ADO_PRINT_QUEUE("Set paramters ok\n");

	//get digital gain range
	audio_high_ctrl(audio_info.pAudioStream, AUD_SET_CAP_VOL, 0);
	//get analog gain range
	//audio_high_ctrl(audio_info.pAudioStream, AUD_SET_SIG_VOL, 20);
	
	if (snx_nvram_integer_get(NVRAM_PKG_AUDIO_ISP, NVRAM_CFG_AUDIO_VOICE, &voicelevel) != NVRAM_SUCCESS) 
	{
		audio_high_ctrl(audio_info.pAudioStream, AUD_SET_SIG_VOL, 20);	
	}
	else
	{
		int levelchangetoset;
		if((voicelevel < 0) || (voicelevel > 100))			
			levelchangetoset = 20; //default
		if(voicelevel == 0)
			levelchangetoset = 0;
		else if((0 < voicelevel) && (voicelevel <= 3))
			levelchangetoset = 1;
		else if(voicelevel > 3)
		{
			levelchangetoset = (int)voicelevel / 3;
			if(levelchangetoset > 31)
				levelchangetoset = 31;		 
		}
		ADO_PRINT_QUEUE("audio voice level =%d\n",levelchangetoset);
        audio_high_ctrl(audio_info.pAudioStream, AUD_SET_SIG_VOL, levelchangetoset);	
	}
	
#if AAC_ENABLE
	CodecParams.type = AUD_CODEC_ENCODER;
	CodecParams.samplerate = audio_info.SampleRate;
	CodecParams.bitrate = audio_info.BitRate;
	CodecParams.aot = 2;
	CodecParams.afterburner = 1;
	CodecParams.eld_sbr = 0;
	CodecParams.vbr = 0;
	CodecParams.bits_per_sample = audio_info.BitsPerSample;
	CodecParams.channels = 1;

	err = snx_aac_open(&CodecParams, &(audio_info.pCodec));
	if(err < 0)
	{
		ADO_PRINT_QUEUE("aac encoder init fail\n");
		goto fail;
	}
#else
	CodecParams.type = AUD_CODEC_ENCODER;
	CodecParams.samplerate = audio_info.SampleRate;

	err = snx_alaw_open(&CodecParams, &(audio_info.pCodec));
	if(err < 0)
	{
		ADO_PRINT_QUEUE("aac encoder init fail\n");
		goto fail;
	}
#endif

	lastTime = xTaskGetTickCount();
	CurTime = lastTime;


	while(1)
	{	
		//get empty pack 
		xQueueReceive( audio_info.queue_empty, &pPacketBuf, portMAX_DELAY );
		pbuf = PCMPacketBuf;
		size = audio_info.PacketSize;


		
		while(size > 0)
		{
			retval = audio_read(audio_info.pAudioStream, pbuf, size, 0);

			if(retval > 0)
			{
				//ADO_PRINT_QUEUE("Process:%d Bytes\n", retval);
				size -= retval;
				pbuf += retval;
			}
			else
			{
				if(retval == -EAGAIN)
				{
					vTaskDelay(10 / portTICK_RATE_MS);
					continue;
				}
				audio_status(audio_info.pAudioStream, &status);
				if(status == AUD_STATE_XRUN)
				{
					ADO_PRINT_QUEUE("overrun\n");
					audio_resume(audio_info.pAudioStream); 

				}
				else
				{
					ADO_PRINT_QUEUE("Error capture status\n");
					goto fail;
				}
			}
		}

#if AAC_ENABLE
		err = snx_aac_encode(audio_info.pCodec, PCMPacketBuf, pPacketBuf, audio_info.PacketSize, &enc_size);
		if(err < 0) {
			ADO_PRINT_QUEUE("snx_aac_encode error %d\n",err);
			goto fail;
		}
#else
		err = snx_alaw_encode(audio_info.pCodec, PCMPacketBuf, pPacketBuf, audio_info.PacketSize, &enc_size);
		if(err < 0) {
			ADO_PRINT_QUEUE("snx_alaw_encode error %d\n",err);
			goto fail;
		}
#endif
		
		CurTime = xTaskGetTickCount();
		count ++;
		if(CurTime>lastTime)
		{
			if((CurTime-lastTime)>(10000/portTICK_RATE_MS))
			{
				ADO_PRINT_QUEUE("count = %d, time = %d(ms)\n", count, (CurTime-lastTime)*portTICK_RATE_MS);
				count = 0;
				lastTime = CurTime;
			}
		}
		else
		{
			lastTime = xTaskGetTickCount();
			CurTime = lastTime;
		}
		
		finish_buf.pbuf = pPacketBuf;
		finish_buf.size= enc_size;
		xQueueSendToBack( audio_info.queue_ready, &finish_buf , portMAX_DELAY); 
	
	}

fail:
#if AAC_ENABLE
	if(audio_info.pCodec)
		snx_aac_close(audio_info.pCodec);
#else
	if(audio_info.pCodec)
		snx_alaw_close(audio_info.pCodec);
#endif
audio_drain(audio_info.pAudioStream);
audio_close(audio_info.pAudioStream);
vPortFree(audio_info.pPacketSpace);
vTaskDelete(NULL);


}

/**
* @brief task to get audio data from queue_ready and executing notice function
*/
void task_audio_notice( void *pvParameters )
{
	finish_buf_t finish_buf;
	struct timeval cur_tv;
	
	while(1)
	{
		xQueueReceive(audio_info.queue_ready, &finish_buf, portMAX_DELAY );
		gettimeofday(&cur_tv, NULL);
		if(audio_info.notice)
			audio_info.notice(finish_buf.pbuf, finish_buf.size, cur_tv);
		xQueueSendToBack( audio_info.queue_empty, &finish_buf.pbuf, portMAX_DELAY); 
	}
	vTaskDelete(NULL);

}


/**
* @brief interface function - get parameter for audio
* @param pv_param struct for audio parameter
*/
void mf_audio_get_param(audio_param_t* pv_param)
{
	pv_param->uiFormat = audio_info.format;
	pv_param->uiSampleRate = audio_info.SampleRate; 
	pv_param->ucBitsPerSample = audio_info.BitsPerSample; 
	pv_param->uiPacketSize = audio_info.PacketSize;
	pv_param->uiBitRate = audio_info.BitRate;
#if AAC_ENABLE
	pv_param->uiBitRate++;
#endif
}

/**
* @brief interface function - set function pointer to get audio frame
* @param notice_function function pointer to notice frame data ready
*/
void mf_audio_set_record_cb(audio_notice_t notice_function)
{
	audio_info.notice = notice_function;

}


void mf_audio_set_record_voice(int level)
{  
	audio_high_ctrl(audio_info.pAudioStream, AUD_SET_SIG_VOL, level);	
}
