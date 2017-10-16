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
#include "rec_schedule.h"
#include <libmid_nvram/snx_mid_nvram.h>
#include "audio_main.h"
#include "main_flow.h"
#include "utility.h"

#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AAC
#include <libmid_audio/snx_aac.h>
#endif


#if RTSP_PREVIEW_AUDIO
#include <libmid_rtsp_server/rtsp_server.h>
extern int Livestream_id;
#endif

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

audio_info_t audio_info;
audio_out_info_st audio_out_info;
extern struct snx_aac_st *p_aac_st;
extern uint8_t ucFlashAudioBuf;

/***Test***/
#if 0
void send_cmd(void* pvParameters)
{
	ADO_PRINT(SYS_DBG, "send_cmd_task .......\n");
	char *filename = "sd_ok.aac";
	xQueueSendToBack(audio_out_info.play_queue, (uint8_t*)filename, 0);
	vTaskDelete(NULL);
}
#endif

void aac_tone_play(char *filename)
{
	ADO_PRINT(SYS_DBG, "send to file %s\n", filename);
	xQueueSendToBack(audio_out_info.play_queue, (uint8_t*)filename, 0);
}


/**
* @brief interface function - audio capture initialization
* @return return pdPASS if success
*/
int mf_audio_init(void)
{
	uint16_t stackDepth;
	memset(&audio_info, 0, sizeof(audio_info_t));
	audio_info.type = AUD_CAP_STREAM;
#if AAC_ENABLE
	audio_info.format = AUD_FORMAT_AAC;//
	audio_info.BitsPerSample = 16;
	audio_info.SampleRate = 11025;
	audio_info.BitRate = 15999;
	audio_info.PacketSize = 2048;
	audio_info.BufThreshold = 256;
	audio_info.BufSize = 8192 << 1;
#else
	audio_info.format = AUD_FORMAT_A_LAW;//
	audio_info.BitsPerSample = 8;
	audio_info.SampleRate = 8000;
	audio_info.BitRate = audio_info.SampleRate * audio_info.BitsPerSample;
	audio_info.PacketSize = 800;//160;	//20ms
	audio_info.BufThreshold = 256;
	audio_info.BufSize = 8192;
#endif
	audio_info.time_interval = 10000;

	audio_info.notice = recordaudio;
	ADO_PRINT(SYS_INFO, "sample rate = %d, bps = %d, packet = %d, buf = %d, thr = %d\n",
	                audio_info.SampleRate, audio_info.BitsPerSample, audio_info.PacketSize,
	                audio_info.BufSize, audio_info.BufThreshold);

	//get queue
	audio_info.queue_empty = xQueueCreate(MAX_AUDIO_PACKET, sizeof(uint8_t *));
	if (NULL == audio_info.queue_empty) {
		ADO_PRINT(SYS_ERR, "queue queue_empty create fail\n");
		goto fail4;
	}

	audio_info.queue_ready = xQueueCreate(MAX_AUDIO_PACKET, sizeof(finish_buf_t));
	if (NULL == audio_info.queue_ready) {
		ADO_PRINT(SYS_ERR, "queue queue_ready create fail\n");
		goto fail3;
	}
	//start task to get audio frame
	if (pdPASS != xTaskCreate(task_audio_get, "ado_get", STACK_SIZE_16K, NULL,
	                          PRIORITY_TASK_APP_AUDIO_GET, &audio_info.task_get)) {
		ADO_PRINT(SYS_ERR, "Could not create task audio get\n");
		goto fail2;
	}

	//start task to notice audio finish
#if RTSP_PREVIEW_AUDIO
	stackDepth = STACK_SIZE_4K;	/* increase for rtp push data */
#else
	stackDepth = STACK_SIZE_1K;
#endif

	if (pdPASS != xTaskCreate(task_audio_notice, "ado_notice", stackDepth, NULL,
							  PRIORITY_TASK_APP_AUDIO_GET, &audio_info.task_notic)) {
		ADO_PRINT(SYS_ERR, "Could not create task audio notice\n");
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
	if (audio_info.aac_used) {
		if (audio_info.pCodec) {
			snx_aac_close((struct snx_aac_st *)audio_info.pCodec);
			audio_info.pCodec = NULL;
		}
	}
	else {
		if (audio_info.pCodec) {
			snx_alaw_close((struct snx_alaw_st *)audio_info.pCodec);
			audio_info.pCodec = NULL;
		}
	}
	
	if (audio_info.pAudioStream) {
		audio_drain(audio_info.pAudioStream);
		audio_close(audio_info.pAudioStream);
		audio_info.pAudioStream = NULL;
	}
	if (audio_info.pPacketSpace) {
		vPortFree(audio_info.pPacketSpace);
		audio_info.pPacketSpace = NULL;
	}
	if (audio_info.task_get) {
		vTaskDelete(audio_info.task_get);
		audio_info.task_get = NULL;
	}
	if (audio_info.task_notic) {
		vTaskDelete(audio_info.task_notic);
		audio_info.task_notic = NULL;
	}
	if (audio_info.queue_ready) {
		vQueueDelete(audio_info.queue_ready);
		audio_info.queue_ready = NULL;
	}
	if (audio_info.queue_empty) {
		vQueueDelete(audio_info.queue_empty);
		audio_info.queue_empty = NULL;
	}
}



/**
* @brief task to get audio data from queue_ready and executing notice function
*/
void task_audio_notice( void *pvParameters )
{
	finish_buf_t finish_buf;
	struct timeval cur_tv;
#if RTSP_PREVIEW_AUDIO
	int audio_flag = app_uti_get_preview_audio_mode();
#endif	
	while (1) {
		xQueueReceive(audio_info.queue_ready, &finish_buf, portMAX_DELAY );
		gettimeofday(&cur_tv, NULL);
		if (audio_info.notice)
			audio_info.notice(finish_buf.pbuf, finish_buf.size, cur_tv);

#if RTSP_PREVIEW_AUDIO
		if( audio_flag == PREVIEW_AUDIO_ON)
			send_rtp_data((Livestream_id & 0xFFFF0000) >> 16, (char*)finish_buf.pbuf, finish_buf.size, NULL, PREVIEW_STREAM);	
#endif
		xQueueSendToBack( audio_info.queue_empty, &finish_buf.pbuf, portMAX_DELAY);
	}
	vTaskDelete(NULL);

}


/**
* @brief interface function - get parameter for audio
* @param pv_param struct for audio parameter
*/
void mf_audio_get_param(audio_param_t *pv_param)
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

int mf_audio_out_init(void)
{
	audio_out_info.type = AUD_PLY_STREAM;
	audio_out_info.format = AUD_FORMAT_S16_LE;
	audio_out_info.codec = AUD_FORMAT_AAC;
	audio_out_info.is_block = AAC_BLOCK;
	audio_out_info.rate = AAC_SMAPLE_RATE;
	audio_out_info.bitrate = AAC_BITRATE;

	//get queue
	audio_out_info.play_queue = xQueueCreate(MAX_AUDIO_PLYA_QUEUE, sizeof(audio_out_info.filename));
	if (NULL == audio_out_info.play_queue) {
		ADO_PRINT(SYS_ERR, "queue queue_empty create fail\n");
		goto fail1;
	}

	if (pdPASS != xTaskCreate(task_audio_out, "audio out task", STACK_SIZE_8K * 2, (void *) &audio_out_info, 30, &audio_out_info.task_out)) {
		ADO_PRINT(SYS_ERR, "Could not create task of audio play\n");
		goto fail2;
	}

	//Test code
	#if 0
	if (pdPASS != xTaskCreate(send_cmd, "sned_cmd", STACK_SIZE_1K, (void*) NULL, 10, NULL))
	{
		ADO_PRINT(SYS_ERR, "Could not create task of audio play\n");
		goto fail2;
	}
	#endif
	return pdPASS;
fail2:
	vQueueDelete(audio_out_info.play_queue);
fail1:
	return pdFAIL;
}

void mf_audio_out_unint(void)
{
	if (audio_out_info.audio_stream != NULL) {
		audio_drain(audio_out_info.audio_stream);
		audio_close(audio_out_info.audio_stream);
		audio_out_info.audio_stream = NULL;
	}

	if (p_aac_st != NULL) {
		snx_aac_close(p_aac_st);
		p_aac_st = NULL;
	}

	if (audio_out_info.aud_dev_param != NULL) {
		audio_params_free(audio_out_info.aud_dev_param);
		audio_out_info.aud_dev_param = NULL;
	}

	if (audio_out_info.fp != NULL) {
		snx_nvram_binary_close(audio_out_info.fp);
		audio_out_info.fp = NULL;
	}

	if (audio_out_info.buf_dec != NULL) {
		vPortFree(audio_out_info.buf_dec);
		audio_out_info.buf_dec = NULL;
	}

	if (audio_out_info.buf_aac != NULL) {
		vPortFree(audio_out_info.buf_aac);
		audio_out_info.buf_aac = NULL;
	}

	if (audio_out_info.play_queue) {
		vQueueDelete(audio_out_info.play_queue);
		audio_out_info.play_queue = NULL;
	}

	if (audio_out_info.task_out) {
		vTaskDelete(audio_out_info.task_out);
		audio_out_info.task_out = NULL;
	}
}

void audio_suspend()
{
	audio_drain(audio_info.pAudioStream);
}

void audio_restart()
{
	uint8_t *TmpPCMPacketBuf = NULL;
	audio_resume(audio_info.pAudioStream);
	
	TmpPCMPacketBuf = pvPortMalloc(audio_info.PacketSize, GFP_DMA, MODULE_APP);
	if (TmpPCMPacketBuf == NULL) {
		ADO_PRINT(SYS_ERR,"alloc PCMPacketBuf Packet fail (size = %d)\n", audio_info.PacketSize);
		vPortFree(TmpPCMPacketBuf);
	}
	//just for resume audio stream...
	audio_read(audio_info.pAudioStream, TmpPCMPacketBuf, audio_info.PacketSize, 0);
	vPortFree(TmpPCMPacketBuf);
	ucFlashAudioBuf=1;
}
