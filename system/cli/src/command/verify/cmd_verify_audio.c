#include <FreeRTOS.h>
#include <bsp.h>
#include <task.h>
#include <nonstdlib.h>
#include <sys/time.h>
#include <cmd_verify.h>
#include <audio/audio_dri.h>
#include <audio/audio_error.h>
#include <audio.h>
#include <libmid_fatfs/ff.h>
#include <audio_codec.h>
#include "generated/snx_sdk_conf.h"
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AAC
#include <snx_aac.h>
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_ALAW
#include <snx_alaw.h>
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_MULAW
#include <snx_mulaw.h>
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_G726
#include <snx_g726.h>
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AUD32
#include <snx_aud32.h>
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_PROCESS_AGC
#include <snx_agc.h>
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_PROCESS_AEC
#include <snx_aec.h>
#endif

typedef struct audio_param {
	int format;
	int rate;
	int buf_size;
	int buf_threshold;
	int second;
#ifdef CONFIG_MIDDLEWARE_AUDIO_PROCESS_AGC
	struct snx_agc_params_st *agc_t;
#endif
} audio_param_t;
audio_param_t aud_t;

#ifdef CONFIG_MIDDLEWARE_AUDIO_PROCESS_AGC
static struct snx_agc_params_st g_agc_t =
{
	.gain_max = AGC_GAIN_MAX,
	.gain_min = AGC_GAIN_MIN,
	.gain_default = AGC_GAIN_DEFAULT,
	.dyn_gain_max = AGC_DYN_GAIN_MAX,
	.dyn_gain_min = AGC_DYN_GAIN_MIN,
	.dyn_targetlevel_high = AGC_DYN_TARGET_LEVEL_HIGH,
	.dyn_targetlevel_low = AGC_DYN_TARGET_LEVEL_LOW,
	.dyn_updatespeed = AGC_DYN_UPDATE_SPEED,
	.peakamp_thres = AGC_PEAKAMP_THD,
	.peakcnt_thres = AGC_PEAKCNT_THD,
	.frame_bufsize = 800, //bytes
	.samplerate = 8000,
	.upstep = AGC_UPSTEP,
	.downstep = AGC_DOWNSTEP,
};
#endif

int check_sine_wave(unsigned char *addr, int size)
{
	int i = 0 ;
	int ret = 0;
	unsigned short *data;

	data = (unsigned short *)addr;

	for (i = 0 ; i < (size - 16) ; i += 16) {
		if ((data[0] == data[8]) && (data[1] == data[7]) &&
				(data[2] == data[6]) && (data[3] == data[5])) {
			data += 8;
		} else {
			print_msg_queue("Sine wave error!!");
			ret = -1;
			break;
		}
	}

	return ret;
}
//param_t mem_param;

void audio_play_task(void *pvParameters)
{
	audio_param_t *param = (audio_param_t *)pvParameters;
	struct audio_stream_st *audio_stream;
	struct params_st *params;
	FIL MyFile;
	int retval = 0, status = 0;
	uint8_t *buf, *pbuf;
	unsigned int br = 0;

	print_msg_queue("format = %d\n", param->format);
	print_msg_queue("rate = %d\n", param->rate);
	print_msg_queue("buf size = %d\n", param->buf_size);
	print_msg_queue("hold = %d\n", param->buf_threshold);
	//print_msg_queue("second = %d\n", param->second);

	if (f_open(&MyFile, "audio.pcm", FA_READ) == FR_OK) {
		print_msg_queue("File open ok\n");
	} else {
		print_msg_queue("File open fail\n");
		goto out;
	}

	buf = (uint8_t *)pvPortMalloc(1024, GFP_KERNEL, MODULE_CLI);
	if (buf == NULL) {
		print_msg_queue("Failed to allocate memory. --- %d\n");
		goto out;
	}

	retval = audio_open(&audio_stream, AUD_PLY_STREAM);
	if (retval < 0) {
		print_msg_queue("Failed to open device.\n");
		goto err1;
	}
	print_msg("audio open ok.\n");

	retval = audio_params_alloca(&params);
	if (retval < 0) {
		print_msg_queue("Failed to allocate memory.\n");
		goto err2;
	}

	print_msg("Allcate paramters ok\n");
	audio_params_set_rate(audio_stream, params, param->rate);
	audio_params_set_format(audio_stream, params, param->format);
	retval = audio_set_params(audio_stream, params);

	if (retval < 0) {
		print_msg_queue("Failed to set paramters.\n");
		goto err2;
	}
	print_msg_queue("Set paramters ok\n");

	print_msg_queue("Start audio play....\n");

	pbuf = buf;
	while(1) {
		f_read(&MyFile, buf, 1024, &br);
rewrite:
		retval = audio_write(audio_stream, buf, 1024, 0);

		if(retval > 0) {
			//print_msg("Process:%d Bytes\n", retval);
			//size -= retval;
			//pbuf += retval;
		} else {
			if(retval == -EAGAIN) {
				vTaskDelay(10 / portTICK_RATE_MS);
				goto rewrite;
			}

			audio_status(audio_stream, &status);
			if(status == AUD_STATE_XRUN) {
				print_msg_queue("overrun\n");
				audio_resume(audio_stream);
			} else {
				print_msg_queue("Error capture status\n");
				retval = -6;
				goto err3;
			}
		}

		if (br == 0)
			break;
	}

	print_msg_queue("Audio play done..\n");

err3:
	audio_drain(audio_stream);
err2:
	audio_close(audio_stream);
err1:
	vPortFree(buf);

	f_close(&MyFile);
out:
	vTaskDelete(NULL);

}

int cmd_verify_audio_play(int argc, char* argv[])
{
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_ALAW
	aud_t.format = AUD_FORMAT_A_LAW;
	aud_t.rate = 48000;
	aud_t.buf_size = 8192;
	aud_t.buf_threshold = 1024;
	aud_t.second = 0;

	if (pdPASS != xTaskCreate(audio_play_task, "audio_play", STACK_SIZE_4K, (void*) &aud_t,
			99, NULL))
		print_msg("Could not create audio_play_task\r\n");
#endif
	return 0;
}

void audio_record_task(void *pvParameters)
{
	audio_param_t *param = (audio_param_t *)pvParameters;
	struct audio_stream_st *audio_stream;
	struct params_st *params;
	FIL MyFile;
	int retval = 0, status = 0;
	int second_cnt = 0;
	int wbytes = 0;
	uint8_t *buf, *pbuf;
	uint32_t size, osize;
	int i = 0;

	print_msg_queue("format = %d\n", param->format);
	print_msg_queue("rate = %d\n", param->rate);
	print_msg_queue("buf size = %d\n", param->buf_size);
	print_msg_queue("hold = %d\n", param->buf_threshold);
	print_msg_queue("second = %d\n", param->second);

	if (f_open(&MyFile, "audio.pcm", FA_CREATE_ALWAYS | FA_WRITE) == FR_OK) {
		print_msg_queue("File open ok\n");
	} else {
		print_msg_queue("File open fail\n");
		goto out;
	}

#ifdef CONFIG_MIDDLEWARE_AUDIO_PROCESS_AGC
	if (param->agc_t)
		size = param->agc_t->frame_bufsize; //bytes
	else
#endif
		size = 160;
	osize = size;

	if ( (param->format == AUD_FORMAT_S16_LE) || (param->format == AUD_FORMAT_U16_LE) )
		second_cnt = ((param->second * param->rate) / osize) << 1;
	else
		second_cnt = (param->second * param->rate) / osize;
	second_cnt++;

	buf = (uint8_t *)pvPortMalloc(size, GFP_KERNEL, MODULE_CLI);
	if (buf == NULL) {
		print_msg_queue("Failed to allocate memory. --- %d\n", size);
		goto out;
	}

	retval = audio_open(&audio_stream, AUD_CAP_STREAM);
	if (retval < 0) {
		print_msg_queue("Failed to open device.\n");
		goto err1;
	}
	print_msg("audio open ok.\n");

	retval = audio_params_alloca(&params);
	if (retval < 0) {
		print_msg_queue("Failed to allocate memory.\n");
		goto err2;
	}

	print_msg("Allcate paramters ok\n");
	audio_params_set_rate(audio_stream, params, param->rate);
	audio_params_set_format(audio_stream, params, param->format);
#ifdef CONFIG_MIDDLEWARE_AUDIO_PROCESS_AGC
	if (param->agc_t)
		audio_params_set_agc(audio_stream, params, &(param->agc_t));
#endif
	retval = audio_set_params(audio_stream, params);

	if (retval < 0) {
		print_msg_queue("Failed to set paramters.\n");
		goto err2;
	}
	print_msg_queue("Set paramters ok\n");

	print_msg_queue("Start audio record....\n");
	for (i = 0 ; i < second_cnt ; i++) {
		pbuf = buf;
		size = osize;
		while(size > 0) {
				retval = audio_read(audio_stream, pbuf, size, 0);

				//print_msg("Process:%d Bytes\n", retval);
			if(retval > 0) {
				//print_msg("Process:%d Bytes\n", retval);
				size -= retval;
				pbuf += retval;
			} else {
				if(retval == -EAGAIN) {
					vTaskDelay(10 / portTICK_RATE_MS);
					continue;
				}

				audio_status(audio_stream, &status);
				if(status == AUD_STATE_XRUN) {
					print_msg_queue("overrun\n");
					audio_resume(audio_stream);
				} else {
					print_msg_queue("Error capture status %d\n", status);
					retval = -6;
					goto err3;
				}
			}
		}

		if (f_write(&MyFile, buf, osize, (void *)&wbytes) != FR_OK) {
			print_msg_queue("audio save to sd error!!!, cnt = %d\n", wbytes);
		}
	}

	print_msg_queue("Stop audio record....\n");

err3:
	audio_drain(audio_stream);
err2:
	audio_close(audio_stream);
err1:
	vPortFree(buf);
#ifdef CONFIG_MIDDLEWARE_AUDIO_PROCESS_AGC
	if (param->agc_t)
		param->agc_t = NULL;
#endif
	f_close(&MyFile);
out:
	vTaskDelete(NULL);
}

int cmd_verify_audio_agc_info(int argc, char* argv[])
{
#ifdef CONFIG_MIDDLEWARE_AUDIO_PROCESS_AGC
	print_msg("## Current setting ##\n");
	print_msg("Record length: %d secs\n",(aud_t.second)?aud_t.second:300);
	print_msg("Sample rate: %d\n",g_agc_t.samplerate);
	print_msg("Gain max: %d\n",g_agc_t.gain_max);
	print_msg("Gain min: %d\n",g_agc_t.gain_min);
	print_msg("Gain default: %d\n",g_agc_t.gain_default);
	print_msg("Dynamic Gain max: %d\n",g_agc_t.dyn_gain_max);
	print_msg("Dynamic Gain min: %d\n",g_agc_t.dyn_gain_min);
	print_msg("Dynamic target level high bound: %d\n",g_agc_t.dyn_targetlevel_high);
	print_msg("Dynamic target level low bound: %d\n",g_agc_t.dyn_targetlevel_low);
	print_msg("Dynamic update speed: %d\n",g_agc_t.dyn_updatespeed);
	print_msg("Buffer size: %d(samples)\n",g_agc_t.frame_bufsize >> 1); //bytes to samples
	print_msg("Peak energy threshold: %d\n",g_agc_t.peakamp_thres);
	print_msg("Peak counts threshold: %d\n",g_agc_t.peakcnt_thres);
	print_msg("Gain increase speed: %d\n",g_agc_t.upstep);
	print_msg("Gain decrease speed: %d\n",g_agc_t.downstep);
#endif
	return 0;
}

int cmd_verify_audio_set_agc(int argc, char* argv[])
{
#ifdef CONFIG_MIDDLEWARE_AUDIO_PROCESS_AGC
	if (argc == 2) {
		if (strcmp(argv[0], "agcrsec") == 0)
			aud_t.second = atoi(argv[1]);
		else if (strcmp(argv[0], "agcgmax") == 0)
			g_agc_t.gain_max = atoi(argv[1]);
		else if(strcmp(argv[0], "agcgmin") == 0)
			g_agc_t.gain_min = atoi(argv[1]);
		else if(strcmp(argv[0], "agcgdef") == 0)
			g_agc_t.gain_default = atoi(argv[1]);
		else if(strcmp(argv[0], "agcdgmax") == 0)
			g_agc_t.dyn_gain_max = atoi(argv[1]);
		else if(strcmp(argv[0], "agcdgmin") == 0)
			g_agc_t.dyn_gain_min = atoi(argv[1]);
		else if(strcmp(argv[0], "agcdthigh") == 0)
			g_agc_t.dyn_targetlevel_high = atoi(argv[1]);
		else if(strcmp(argv[0], "agcdtlow") == 0)
			g_agc_t.dyn_targetlevel_low = atoi(argv[1]);
		else if(strcmp(argv[0], "agcduspeed") == 0)
			g_agc_t.dyn_updatespeed = atoi(argv[1]);
		else if(strcmp(argv[0], "agcpth") == 0)
			g_agc_t.peakamp_thres = atoi(argv[1]);
		else if(strcmp(argv[0], "agcpcnt") == 0)
			g_agc_t.peakcnt_thres = atoi(argv[1]);
		else if(strcmp(argv[0], "agcbsize") == 0)
			g_agc_t.frame_bufsize = atoi(argv[1]) << 1; //PCM S16 LE, samples to bytes
		else if(strcmp(argv[0], "agcrate") == 0)
			g_agc_t.samplerate = atoi(argv[1]);
		else if(strcmp(argv[0], "agcupstep") == 0)
			g_agc_t.upstep = atoi(argv[1]);
		else if(strcmp(argv[0], "agcdnstep") == 0)
			g_agc_t.downstep = atoi(argv[1]);
	} else {
		print_msg("parameter number error -- %d\n", argc);
		goto cmd_err;
	}

#endif
	return 0;
cmd_err:
	print_msg("Command error.\n");
	return -1;
}

#ifdef CONFIG_MIDDLEWARE_AUDIO_PROCESS_AEC
struct snx_aec_st *aec_cap = NULL,*aec_play = NULL;
uint32_t aec_enable = AEC_ENABLE; //AEC_ENABLE / AEC_DISABLE
uint32_t is_aec_running;
FIL CbF;
#endif

int cmd_verify_audio_aec_stop(int argc, char* argv[])
{
#ifdef CONFIG_MIDDLEWARE_AUDIO_PROCESS_AEC
	is_aec_running = 0;
	
	if (aec_cap) {
		snx_aec_stop(aec_cap, AEC_TASK_RELEASE);
		vTaskDelay(100 / portTICK_RATE_MS);
		snx_aec_close(aec_cap);
	}
	
	if (aec_play) {
		snx_aec_stop(aec_play, AEC_TASK_RELEASE);
		vTaskDelay(100 / portTICK_RATE_MS);
		snx_aec_close(aec_play);
	}
#endif
	return 0;
}

void audio_aec_callback(const struct timeval *tstamp, void *data, size_t len,void *cbarg)
{
#ifdef CONFIG_MIDDLEWARE_AUDIO_PROCESS_AEC
	FRESULT fr;
	if (!data || !len || !is_aec_running) 
		return;
	
	int32_t wbytes = 0;
	fr = f_write(&CbF, (uint8_t *)data, len, (void *)&wbytes);
	if (fr) {
		print_msg_queue("Write data is fail\n");
		cmd_verify_audio_aec_stop(0,NULL);
	}
#endif
}

void audio_aec_task_capture(void)
{
#if defined(CONFIG_MIDDLEWARE_AUDIO_PROCESS_AEC) && defined(CONFIG_MIDDLEWARE_AUDIO_CODEC_AUD32)
	int ret = 0;
	int32_t format_size = 2; //AUD_FORMAT_S16_LE
	int32_t type = AUD_CAP_STREAM;
	struct audio_stream_st *audio_stream = NULL;
	FRESULT fr;

	if ((ret = audio_open(&audio_stream, type)) < 0)
	{
        print_msg_queue("open device failed\n");	
		goto cap_out;
	}
	
	struct params_st *params;
	if ((ret = audio_params_alloca(&params)) != 0) {
		print_msg_queue("audio params alloca failed\n");
		goto cap_out;
	}
	
	/* setup audio information */
	audio_params_set_rate(audio_stream, params, 8000);
	audio_params_set_format(audio_stream, params, AUD_FORMAT_S16_LE);
	if ((ret = audio_set_params(audio_stream, params)) < 0) {
		print_msg_queue("Failed to set paramters.\n");	
		goto cap_out;
	}
	
	/* MIC volume max: 0x14 */
	audio_high_ctrl(audio_stream, AUD_SET_CAP_VOL, 0x14);
	
	
	fr = f_open(&CbF, "AEC.out", FA_CREATE_ALWAYS | FA_WRITE);
	if (fr) {
		print_msg_queue("Create file failed.\n");
		goto cap_out;
	}
	
	/* AEC initial */
	struct snx_aec_params_st *aec_params = NULL;	
	aec_params = (struct snx_aec_params_st*)pvPortMalloc(sizeof(struct snx_aec_params_st),GFP_KERNEL, MODULE_CLI);
	if (aec_params)
	{
		memset(aec_params,0x0,sizeof(struct snx_aec_params_st));
		struct snx_aec_st **aec_st;
		aec_st = &aec_cap;
		aec_params->type = type;
		aec_params->sample_rate = 8000;
		aec_params->bit_rate = 8000;
		aec_params->codec = AUD_FORMAT_AUD32;
		aec_params->cb = audio_aec_callback;
		aec_params->cbarg = NULL; //User define, callback arguments
		/* Output encoded data size = 256 * format_size * 5 / compress_ratio */
		aec_params->delay_userdef = AEC_DISABLE;
		aec_params->format_size = format_size;
		aec_params->audio_stream = audio_stream;
		
		ret = snx_aec_open(aec_params, aec_st);
		if (ret != AEC_SUCCESS) {
			print_msg_queue("snx_aec_open failed.\n");
			goto cap_out;
		}
	}
	else
		print_msg_queue("snx_aec_params_st pvPortMalloc failed.\n");	
	
	/* while loop in process, exit by snx_aec_stop */
	snx_aec_process(aec_cap, &aec_enable);
	
cap_out:
	if (is_aec_running)
		is_aec_running = 0;
	f_close(&CbF);
	if (aec_params) {
		vPortFree(aec_params);
		aec_params = NULL;
	}
	audio_params_free(params);
	if (audio_stream) {
		audio_drain(audio_stream);
		audio_close(audio_stream);
		audio_stream = NULL;
	}
	print_msg_queue("AEC capture task exit.\n");
	vTaskDelete(NULL);
#endif
}

void audio_aec_task_playback(void)
{
#if defined(CONFIG_MIDDLEWARE_AUDIO_PROCESS_AEC) && defined(CONFIG_MIDDLEWARE_AUDIO_CODEC_AUD32)
	int ret = 0;
	int32_t format_size = 2; //AUD_FORMAT_S16_LE
	int32_t type = AUD_PLY_STREAM;
	struct audio_stream_st *audio_stream = NULL;
	
	if ((ret = audio_open(&audio_stream, type)) < 0)
	{
        print_msg_queue("open device failed\n");	
		goto play_out;
	}
	
	struct params_st *params;
	if ((ret = audio_params_alloca(&params)) != 0) {
		print_msg_queue("audio params alloca failed\n");
		goto play_out;
	}
	
	/* setup audio information */
	audio_params_set_rate(audio_stream, params, 16000);
	audio_params_set_format(audio_stream, params, AUD_FORMAT_S16_LE);
	if ((ret = audio_set_params(audio_stream, params)) < 0) {
		print_msg_queue("Failed to set paramters.\n");	
		goto play_out;
	}
	
	/* Speaker volume max: 0x0F */
	audio_high_ctrl(audio_stream, AUD_SET_PLY_VOL, 0x07);
	
	/* AEC initial */
	struct snx_aec_params_st *aec_params = NULL;
	aec_params = (struct snx_aec_params_st*)pvPortMalloc(sizeof(struct snx_aec_params_st),GFP_KERNEL, MODULE_CLI);
	if (aec_params)
	{
		memset(aec_params,0x0,sizeof(struct snx_aec_params_st));
		struct snx_aec_st **aec_st;
		aec_st = &aec_play;
		aec_params->type = type;
		aec_params->sample_rate = 16000;
		aec_params->bit_rate = 16000;
		aec_params->codec = AUD_FORMAT_AUD32;
		aec_params->delay_userdef = AEC_DISABLE;
		aec_params->format_size = format_size;
		aec_params->audio_stream = audio_stream;
		
		ret = snx_aec_open(aec_params, aec_st);
		if (ret != AEC_SUCCESS) {
			print_msg_queue("snx_aec_open failed.\n");
			goto play_out;
		}
	}
	else
		print_msg_queue("snx_aec_params_st pvPortMalloc failed.\n");
	
	/* while loop in process, exit by snx_aec_stop */
	snx_aec_process(aec_play, &aec_enable);
	
play_out:
	if (is_aec_running)
		is_aec_running = 0;
	if (aec_params) {
		vPortFree(aec_params);
		aec_params = NULL;
	}
	audio_params_free(params);
	if (audio_stream) {
		audio_drain(audio_stream);
		audio_close(audio_stream);
		audio_stream = NULL;
	}
	print_msg_queue("AEC playback task exit.\n");
	vTaskDelete(NULL);
#endif
}

void audio_aec_task_fill_data(void)
{
#ifdef CONFIG_MIDDLEWARE_AUDIO_PROCESS_AEC
	int ret = 0;
	FIL fsrc;      /* File objects */
	FRESULT fr;          /* FatFs function common result code */
	uint32_t rbytes;
	uint32_t enc_size;
	uint8_t *file_buffer = NULL;
	uint32_t domiso_position = 0;
	uint32_t enc_frame_len = 40;
	uint8_t frame_len[4];
	
	fr = f_open(&fsrc, "domiso_16K.aud32", FA_OPEN_EXISTING | FA_READ);
	if (fr)
	{
		print_msg_queue("Open file of play is fail\n");
		goto fill_out;
	}
	enc_size = (uint32_t)(f_size(&fsrc));
	if (!(file_buffer = (uint8_t *)pvPortMalloc(enc_size,GFP_KERNEL, MODULE_CLI)))
	{
		print_msg_queue("Buffer malloc failed.\n");
		f_close(&fsrc);
		goto fill_out;
	}
	f_read(&fsrc, file_buffer, enc_size, &rbytes);
	f_close(&fsrc);
			
	while (is_aec_running)
	{
		if (aec_play) {
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AAC
			if (aec_play->codec == AUD_FORMAT_AAC) {
				//get AAC frame size from header bit 0...30-42
				frame_len[0] = ((file_buffer[4+domiso_position] & 0x1F) << 3) | 
								((file_buffer[5+domiso_position] & 0xE0) >> 5) ;
				frame_len[1] = ((file_buffer[3+domiso_position] & 0x3) << 3) | 
								((file_buffer[4+domiso_position] & 0xE0) >> 5);
				frame_len[2] = 0x0;
				frame_len[3] = 0x0;
				enc_frame_len = *((uint32_t*)frame_len);
			}
#endif
			
			/* Wait for consume */
			while (aec_play->data_len >= (10240-enc_frame_len) )
				vTaskDelay(10 / portTICK_RATE_MS);
			
			/* behavior choice: 
				1. Wait for consume or 
				2. Fill in directly, buffer will be auto-overwrited */
			snx_aec_fill_in_data(aec_play, file_buffer + domiso_position, enc_frame_len);
		}
		domiso_position = (domiso_position + enc_frame_len) % enc_size;
	}
	
fill_out:	
	if (file_buffer) {
		vPortFree(file_buffer);
		file_buffer = NULL;
	}
	print_msg_queue("AEC fill task exit.\n");
	vTaskDelete(NULL);
#endif
}

int cmd_verify_audio_aec_start(int argc, char* argv[])
{
#ifdef CONFIG_MIDDLEWARE_AUDIO_PROCESS_AEC
	int ret = AEC_SUCCESS;
	
	if (is_aec_running) {
		print_msg_queue("AEC already opened.\r\n");
		return -1;
	}
	else
		is_aec_running = 1;
		
	if (pdPASS != xTaskCreate(audio_aec_task_capture, "audio_aec_cap", STACK_SIZE_4K*8, NULL,
			39, NULL))
		print_msg_queue("Could not create audio_aec_cap task\r\n");

	if (pdPASS != xTaskCreate(audio_aec_task_playback, "audio_aec_play", STACK_SIZE_4K*8, NULL,
			39, NULL))
		print_msg_queue("Could not create audio_aec_play task\r\n");
		
	if (pdPASS != xTaskCreate(audio_aec_task_fill_data, "audio_aec_fill", STACK_SIZE_4K, NULL,
			39, NULL))
		print_msg_queue("Could not create audio_aec_fill task\r\n");
		
	vTaskDelay(1000 / portTICK_RATE_MS);
	
	ret = (snx_aec_start(aec_cap) || snx_aec_start(aec_play) );
	if (ret != AEC_SUCCESS)
		print_msg_queue("snx_aec_start failed(%d)\r\n",ret);
#endif	
	return 0;
}

int cmd_verify_audio_record_agc(int argc, char* argv[])
{
#ifdef CONFIG_MIDDLEWARE_AUDIO_PROCESS_AGC
	aud_t.format = AUD_FORMAT_S16_LE;
	aud_t.rate = g_agc_t.samplerate;
	aud_t.buf_size = 8192;
	aud_t.buf_threshold = 1024;
	if (aud_t.second == 0)
		aud_t.second = 300;

	aud_t.agc_t = &g_agc_t;

	if (pdPASS != xTaskCreate(audio_record_task, "audio_rec_agc", STACK_SIZE_4K, (void*) &aud_t,
			59, NULL))
		print_msg("Could not create audio_record_agc_task\n");
#endif
	return 0;

cmd_err:
	print_msg("Command error.\n");
	return -1;
}

int cmd_verify_audio_record(int argc, char* argv[])
{
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_ALAW
	aud_t.format = AUD_FORMAT_A_LAW;
	aud_t.rate = 8000;
	aud_t.buf_size = 8192;
	aud_t.buf_threshold = 1024;
	aud_t.second = 3600;
#ifdef CONFIG_MIDDLEWARE_AUDIO_PROCESS_AGC
	aud_t.agc_t = NULL;
#endif

	if (pdPASS != xTaskCreate(audio_record_task, "audio_record", STACK_SIZE_4K, (void*) &aud_t,
			59, NULL))
		print_msg("Could not create audio_record_task\n");
#endif
	return 0;
}

typedef struct audio_codec_st {
	int32_t type;
	int32_t format;
	int32_t codec;
	int32_t format_size;
	int32_t is_block;
	uint32_t rate;
	uint32_t buf_size;
	uint32_t buf_threshold;
	uint32_t size;
	uint32_t sec;
	uint32_t bitrate;
	int8_t * filename;
}audio_codec_st;
audio_codec_st audio_codec_para;

#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AAC
static struct snx_aac_params_st g_aac_params;
static struct snx_aac_st *g_aac;
static struct snx_aac_info_st g_aac_info;
#endif

#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AUD32
static struct snx_aud32_params_st g_aud32_params;
static struct snx_aud32_st *g_aud32;
static struct snx_aud32_info_st g_aud32_info;
#endif

#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_ALAW
static struct snx_alaw_params_st g_alaw_params;
static struct snx_alaw_st *g_alaw;
static struct snx_alaw_info_st g_alaw_info;
#endif

#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_MULAW
static struct snx_mulaw_params_st g_mulaw_params;
static struct snx_mulaw_st *g_mulaw;
static struct snx_mulaw_info_st g_mulaw_info;
#endif

#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_G726
static struct snx_g726_params_st g_g726_params;
static struct snx_g726_st *g_g726;
static struct snx_g726_info_st g_g726_info;
#endif

void codec_help(void)
{
	print_msg("1. argument 0 is acodec\n");
	print_msg("2. argument 1 is operation type:\n\tcap : record\n\tply : playback\n");
	print_msg("3. argument 2 is the codec type:\n");
	print_msg("\tA-LAW means a-law codec\n");
	print_msg("\tMU-LAW means mu-law codec\n");
	print_msg("\tG726 means G.726 codec\n");
	print_msg("\tAUD32 means audio32 codec\n");
	print_msg("\tAAC means AAC codec\n");
	print_msg("4. argument 3 is seconds\n");
	print_msg("5. argument 4 is sample rate\n");
	print_msg("6. argument 5 is file name\n");
}

void codec_get_info(audio_codec_st *para)
{
	int32_t ret;
	switch (para->codec)
	{
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_ALAW
	case AUD_FORMAT_A_LAW:
		ret = snx_alaw_get_info (g_alaw, &g_alaw_info);
		print_msg("the bit rate is %u\n",g_alaw_info.bitrate);
		print_msg("the sample rate is %u\n",g_alaw_info.samplerate);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_MULAW
	case AUD_FORMAT_MU_LAW:
		ret = snx_mulaw_get_info (g_mulaw, &g_mulaw_info);
		print_msg("the bit rate is %u\n",g_mulaw_info.bitrate);
		print_msg("the sample rate is %u\n",g_mulaw_info.samplerate);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_G726
	case AUD_FORMAT_G726:
		ret = snx_g726_get_info (g_g726, &g_g726_info);
		print_msg("the bit rate is %u\n",g_g726_info.bitrate);
		print_msg("the sample rate is %u\n",g_g726_info.samplerate);
		print_msg("the compress ratio is %u\n",g_g726_info.compress_ratio);
		print_msg("the PCM bytes per frame is %u\n",g_g726_info.pcm_bytes_per_frame);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AUD32
	case AUD_FORMAT_AUD32:
		ret = snx_aud32_get_info (g_aud32, &g_aud32_info);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AAC
	case AUD_FORMAT_AAC:
		ret = snx_aac_get_info (g_aac, &g_aac_info);
		break;
#endif
	default:
		break;
	}
	return ret;
}
int32_t codec_open(audio_codec_st *para)
{
	int32_t ret;
	switch (para->codec)
	{
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_ALAW
	case AUD_FORMAT_A_LAW:
		ret = snx_alaw_open (&g_alaw_params, &g_alaw);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_MULAW
	case AUD_FORMAT_MU_LAW:
		ret = snx_mulaw_open (&g_mulaw_params, &g_mulaw);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_G726
	case AUD_FORMAT_G726:
		ret = snx_g726_open (&g_g726_params, &g_g726);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AUD32
	case AUD_FORMAT_AUD32:
		ret = snx_aud32_open (&g_aud32_params, &g_aud32);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AAC
	case AUD_FORMAT_AAC:
		ret = snx_aac_open (&g_aac_params, &g_aac);
		break;
#endif
	default:
		break;
	}
	return ret;
}

void codec_close(audio_codec_st *para)
{
	switch (para->codec)
	{
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_ALAW
	case AUD_FORMAT_A_LAW:
		snx_alaw_close(g_alaw);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_MULAW
	case AUD_FORMAT_MU_LAW:
		snx_mulaw_close(g_mulaw);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_G726
	case AUD_FORMAT_G726:
		snx_g726_close(g_g726);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AUD32
	case AUD_FORMAT_AUD32:
		snx_aud32_close(g_aud32);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AAC
	case AUD_FORMAT_AAC:
		snx_aac_close(g_aac);
		break;
#endif
	default:
		break;
	}
}

int32_t codec_encode(audio_codec_st *para, uint8_t *p_src, uint8_t *p_dst, int32_t src_bytes, int32_t *p_dst_bytes)
{
	int32_t ret;
	switch (para->codec)
	{
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_ALAW
	case AUD_FORMAT_A_LAW:
		ret = snx_alaw_encode(g_alaw, p_src, p_dst, src_bytes, p_dst_bytes);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_MULAW
	case AUD_FORMAT_MU_LAW:
		ret = snx_mulaw_encode(g_mulaw, p_src, p_dst, src_bytes, p_dst_bytes);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_G726
	case AUD_FORMAT_G726:
		ret = snx_g726_encode(g_g726, p_src, p_dst, src_bytes, p_dst_bytes);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AUD32
	case AUD_FORMAT_AUD32:
		ret = snx_aud32_encode(g_aud32, p_src, p_dst, src_bytes, p_dst_bytes);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AAC
	case AUD_FORMAT_AAC:
		ret = snx_aac_encode(g_aac, p_src, p_dst, src_bytes, p_dst_bytes);
		break;
#endif
	default:
		break;
	}
	return ret;
}

int32_t codec_decode(audio_codec_st *para, uint8_t *p_src, uint8_t *p_dst, int32_t src_bytes, int32_t *p_dst_bytes)
{
	int32_t ret;
	switch (para->codec)
	{
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_ALAW
	case AUD_FORMAT_A_LAW:
		ret = snx_alaw_decode(g_alaw, p_src, p_dst, src_bytes, p_dst_bytes);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_MULAW
	case AUD_FORMAT_MU_LAW:
		ret = snx_mulaw_decode(g_mulaw, p_src, p_dst, src_bytes, p_dst_bytes);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_G726
	case AUD_FORMAT_G726:
		ret = snx_g726_decode(g_g726, p_src, p_dst, src_bytes, p_dst_bytes);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AUD32
	case AUD_FORMAT_AUD32:
		ret = snx_aud32_decode(g_aud32, p_src, p_dst, src_bytes, p_dst_bytes);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AAC
	case AUD_FORMAT_AAC:
		ret = snx_aac_decode(g_aac, p_src, p_dst, src_bytes, p_dst_bytes);
		break;
#endif
	default:
		break;
	}
	return ret;
}


void codec_init(audio_codec_st *para, uint32_t *psize, uint32_t *pblock_count, uint32_t *pbuf_dst_size)
{
	switch (para->codec)
	{
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_ALAW
	case AUD_FORMAT_A_LAW:
		*psize = 80;
		*pblock_count = para->size / (*psize);
		*pbuf_dst_size = 40;
		g_alaw_params.samplerate = para->rate;
		g_alaw_params.type = para->type;
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_MULAW
	case AUD_FORMAT_MU_LAW:
		*psize = 80;
		*pblock_count = para->size / (*psize);
		*pbuf_dst_size = 40;
		g_mulaw_params.samplerate = para->rate;
		g_mulaw_params.type = para->type;
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_G726
	case AUD_FORMAT_G726:
		g_g726_params.pcm_bytes_per_frame = 512;
		g_g726_params.compress_ratio = 2;
		g_g726_params.bytes_per_frame = 64;
		g_g726_params.samplerate = para->rate;
		g_g726_params.type = para->type;
		*psize = g_g726_params.pcm_bytes_per_frame * 10;
		*pbuf_dst_size =  g_g726_params.bytes_per_frame * 10;
		*pblock_count = para->size / (*psize);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AUD32
	case AUD_FORMAT_AUD32:
		g_aud32_params.bitrate = para->bitrate;
		g_aud32_params.samplerate = para->rate;
		if (para->type == AUD_CAP_STREAM)
			g_aud32_params.type = AUD_CODEC_ENCODER;
		else
			g_aud32_params.type = AUD_CODEC_DECODER;
		if (para->rate == 8000)
			*psize = 320;
		else if (para->rate == 16000)
			*psize = 640;
		else if (para->rate == 32000)
			*psize = 1280;
		*pblock_count = para->size / (*psize);
		*pbuf_dst_size = (*psize) >> 4;
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AAC
	case AUD_FORMAT_AAC:
		g_aac_params.samplerate = para->rate;
		g_aac_params.bitrate = para->bitrate;
		g_aac_params.afterburner = 1;
		g_aac_params.aot = 2;
		g_aac_params.bits_per_sample = 16;
		g_aac_params.channels = 1;
		g_aac_params.eld_sbr = 0;
		g_aac_params.vbr = 0;
		if (para->type == AUD_CAP_STREAM)
			g_aac_params.type = AUD_CODEC_ENCODER;
		else
			g_aac_params.type = AUD_CODEC_DECODER;
		*psize = 2048;
		*pblock_count = para->size / (*psize);
		*pbuf_dst_size = 128;
		break;
#endif
	default:
		para->codec = 0;
		*psize = 160;
		*pblock_count = para->size / (*psize);
		break;
	}
}
void vAudioCodecTask(void* pvParameters)
{
	audio_codec_st *para = (audio_codec_st *)pvParameters;
	struct audio_stream_st *audio_stream;
	struct params_st *params;
	uint8_t *buf, *pbuf;
	uint8_t *buf_dst, *pbuf_dst;
	uint32_t buf_dst_size;
	uint32_t size = 0;
	uint32_t block_count;
	uint32_t cur_blk = 0;
	int32_t retval = 0;
	int32_t status;
	int32_t type;
	int32_t is_block;
	FIL fsrc, fdst;      /* File objects */
	FRESULT fr;          /* FatFs function common result code */
	uint32_t br, bw;     /* File read/write count */

	codec_init(para, &size, &block_count, &buf_dst_size);

	buf = (uint8_t *)pvPortMalloc(size, GFP_KERNEL, MODULE_CLI);
	if(buf == NULL)
	{
		print_msg("Failed to allocate memory. --- %d\n",size);
		goto out;
	}

	if (para->codec)
	{
		retval = codec_open(para);
		if (retval != AUD_CODEC_SUCCESS)
		{
			print_msg("codec open is fail\n");
			goto out;
		}
	}

	if (para->codec)
		codec_get_info(para);

	if (para->codec)
	{
		buf_dst = (uint8_t *)pvPortMalloc(size, GFP_KERNEL, MODULE_CLI);
		if (buf_dst == NULL)
		{
			print_msg("Failed to allocate memory. --- %d\n",size);
			goto out;
		}
		pbuf_dst = buf_dst;
	}

	if (para->type == AUD_CAP_STREAM)
	{
		 fr = f_open(&fdst, para->filename, FA_CREATE_ALWAYS | FA_WRITE);
		 if (fr)
		 {
			 print_msg("Open file of Record is fail\n");
			 goto out;
		 }
	}
	else
	{
		fr = f_open(&fsrc, para->filename, FA_OPEN_EXISTING | FA_READ);
		if (fr)
		{
			print_msg("Open file of play is fail\n");
			goto out;
		}
	}

	if(para->type == AUD_PLY_STREAM)
		vTaskDelay(1000 / portTICK_RATE_MS);

	retval = audio_open(&audio_stream, para->type);
	if(retval < 0)
	{
		print_msg("Failed to open device.\n");
		goto err1;
	}
	print_msg("audio open ok.\n");

	retval = audio_params_alloca(&params);
	if(retval < 0)
	{
		print_msg("Failed to allocate memory.\n");
		goto err2;
	}
	print_msg("Allcate paramters ok\n");
	audio_params_set_rate(audio_stream, params, para->rate);
	audio_params_set_format(audio_stream, params, para->format);
	retval = audio_set_params(audio_stream, params);

	if(retval < 0)
	{
		print_msg("Failed to set paramters.\n");
		goto err2;
	}
	print_msg("Set paramters ok\n");

	pbuf = buf;
	type = para->type;
	is_block = para->is_block;
	for (cur_blk = 0; cur_blk < block_count; cur_blk ++)
	{
			if(type == AUD_CAP_STREAM)
			{
reread:
				retval = audio_read(audio_stream, pbuf, size, is_block);
				if (retval <= 0)
				{
					if(retval == -EAGAIN) {
						vTaskDelay(10 / portTICK_RATE_MS);
						goto reread;
					}

					audio_status(audio_stream, &status);
					if(status == AUD_STATE_XRUN)
					{
						print_msg("overrun\n");
						audio_resume(audio_stream);
					}
					else
					{
						print_msg("Error capture status\n");
						goto err3;
					}
				}

				// audio codec encode
				if (para->codec)
				{
					retval = codec_encode(para, pbuf, pbuf_dst, size, &buf_dst_size);
					if (retval == AUD_CODEC_SUCCESS)
					{
						fr = f_write(&fdst, pbuf_dst, buf_dst_size, &bw);
					}
					else
					{
						print_msg("encode(%d) is fail\n",cur_blk);
						break;
					}
				}
				else
					fr = f_write(&fdst, pbuf, size, &bw);

				if (fr)
				{
					print_msg("Write data is fail\n");
					goto err1;
				}
			}
			else
			{
readmore:
				if (para->codec)
					fr = f_read(&fsrc, buf_dst, buf_dst_size, &br);
				else
					fr = f_read(&fsrc, pbuf, size, &br);
				if (fr)
				{
					print_msg("Read data is fail\n");
					goto err1;
				}

				// audio codec decode
				if (para->codec)
				{
					retval = codec_decode(para, buf_dst, pbuf, buf_dst_size, &size);
					if (retval != AUD_CODEC_SUCCESS)
					{
						if (AUD_CODEC_SUCCESS == retval && size>0){
							//print_msg("aac codec done\n");
						}else{
							if ( AUD_CODEC_ERR_MOREDATA == retval){
								print_msg_queue("aac decoding needs more data %d \n",retval);
								goto readmore;
							}
						}
						print_msg("decode(%d) is fail\n",cur_blk);
						break;
					}
				}
rewrite:
				retval = audio_write(audio_stream, pbuf, size, is_block);
				if (retval <= 0)
				{
					if(retval == -EAGAIN) {
						vTaskDelay(10 / portTICK_RATE_MS);
						goto rewrite;
					}

					audio_status(audio_stream, &status);
					if(status == AUD_STATE_XRUN)
					{
						print_msg("underrun\n");
						audio_resume(audio_stream);
					}
					else
					{
						print_msg("Error playback status\n");
						goto err3;
					}
				}
			}
		}


err3:
	audio_drain(audio_stream);
err2:
	audio_close(audio_stream);
	if (para->codec)
		codec_close(para);
err1:
	if (type == AUD_CAP_STREAM)
		f_close(&fdst);
	else
		f_close(&fsrc);
out:
	vPortFree(buf);
	if(para->codec)
		vPortFree(buf_dst);
	print_msg("audio codec task complete!!!\n");
	vTaskDelete(NULL);
}


int cmd_verify_audio_codec(int argc, char* argv[])
{
	// call help for user
	if (argc == 2)
	{
		if(strcmp(argv[1], "--help") == 0)
		{
			codec_help();
			return 0;
		}
	}

	if(argc != 6)
	{
		print_msg("parameter number error -- %d\n", argc);
		print_msg("please key the command of 'acodec --help'\n");
		return -1;
	}

	if(strcmp(argv[1], "cap") == 0)
	{
		audio_codec_para.type = AUD_CAP_STREAM;
		print_msg("Capture\n");
	}
	else if(strcmp(argv[1], "ply") == 0)
	{
		audio_codec_para.type = AUD_PLY_STREAM;
		print_msg("Playback\n");
	}
	else
	{
		print_msg("type parameter(%s) error.\n", argv[1]);
		return -1;
	}

#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_ALAW
	if(strcmp(argv[2], "A-LAW") == 0)
	{
		audio_codec_para.codec = AUD_FORMAT_A_LAW;
		audio_codec_para.format = AUD_FORMAT_S16_LE;
		audio_codec_para.format_size = 2;
	}
	else
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_MULAW
	if(strcmp(argv[2], "MU-LAW") == 0)
	{
		audio_codec_para.codec = AUD_FORMAT_MU_LAW;
		audio_codec_para.format = AUD_FORMAT_S16_LE;
		audio_codec_para.format_size = 2;
	}
	else
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_G726
	if(strcmp(argv[2], "G726") == 0)
	{
		audio_codec_para.codec = AUD_FORMAT_G726;
		audio_codec_para.format = AUD_FORMAT_S16_LE;
		audio_codec_para.format_size = 2;
	}
	else
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AUD32
	if(strcmp(argv[2], "AUD32") == 0)
	{
		audio_codec_para.codec = AUD_FORMAT_AUD32;
		audio_codec_para.format = AUD_FORMAT_S16_LE;
		audio_codec_para.format_size = 2;
	}
	else 
#endif
	if(strcmp(argv[2], "S8") == 0)
	{
		audio_codec_para.format = AUD_FORMAT_S8;
		audio_codec_para.format_size = 1;
	}
	else 
	if(strcmp(argv[2], "U8") == 0)
	{
		audio_codec_para.format = AUD_FORMAT_U8;
		audio_codec_para.format_size = 1;
	}
	else 
	if(strcmp(argv[2], "S16_LE") == 0)
	{
		audio_codec_para.format = AUD_FORMAT_S16_LE;
		audio_codec_para.format_size = 2;
	}
	else 
	if(strcmp(argv[2], "U16_LE") == 0)
	{
		audio_codec_para.format = AUD_FORMAT_U16_LE;
		audio_codec_para.format_size = 2;
	}
	else 
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AAC
	if(strcmp(argv[2], "AAC") == 0)
	{
		audio_codec_para.codec = AUD_FORMAT_AAC;
		audio_codec_para.format = AUD_FORMAT_S16_LE;
		audio_codec_para.format_size = 2;
	}
	else
#endif
	{
		print_msg("format parameter(%s) error.\n", argv[2]);
		return -1;
	}

	audio_codec_para.sec = simple_strtoul(argv[3], NULL, 0);
	audio_codec_para.rate = simple_strtoul(argv[4], NULL, 0);
	audio_codec_para.buf_size = 32768;
	audio_codec_para.buf_threshold = 8192;
	audio_codec_para.is_block = 0;
	audio_codec_para.bitrate = audio_codec_para.rate;
	//audio_codec_para.filename = "test.pcm";
	audio_codec_para.filename = argv[5];

	audio_codec_para.size = audio_codec_para.sec * audio_codec_para.rate * audio_codec_para.format_size;

	if (pdPASS != xTaskCreate(vAudioCodecTask, "audio codec test", STACK_SIZE_8K *4, (void*) &audio_codec_para, 2, NULL))
	{
		print_msg("Could not create task of audio codec\n");
	}
	return 0;
}

int cmd_verify_audio_flyplay(int argc, char* argv[])
{
	print_msg_queue("fly play\n");

	return 0;
}

void vAudioTwowayTask(void* pvParameters)
{
	audio_codec_st *para = (audio_codec_st *)pvParameters;
	struct audio_stream_st *audio_stream_cap;
	struct audio_stream_st *audio_stream_ply;
	struct params_st *params;
	int32_t retval;
	uint8_t *buf,*buf_dst;
	uint8_t *pbuf;
	uint32_t size, buf_dst_size;
	uint32_t block_count;
	uint32_t cur_blk = 0;
	int32_t status;

	bool bWriteFile = false;
	uint32_t bw;

	FIL fdst;      /* File objects */
	FRESULT fr;          /* FatFs function common result code */

#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AAC
	struct snx_aac_params_st aac_params;
	struct snx_aac_st *aac_enc,*aac_dec;
	int32_t iaacdecode = AUD_CODEC_SUCCESS;
#endif

#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AUD32
	struct snx_aud32_params_st aud32_params;
	struct snx_aud32_st *aud32_enc,*aud32_dec;
	int32_t iaud32decode = AUD_CODEC_SUCCESS;
#endif

#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_ALAW
	struct snx_alaw_params_st alaw_params;
	struct snx_alaw_st *alaw_enc,*alaw_dec;
#endif

#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_MULAW
	struct snx_mulaw_params_st mulaw_params;
	struct snx_mulaw_st *mulaw_enc,*mulaw_dec;
#endif

#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_G726
	struct snx_g726_params_st g726_params;
	struct snx_g726_st *g726_enc,*g726_dec;
#endif

#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_ALAW
	if (para->codec == AUD_FORMAT_A_LAW)
		size = 80;
	else
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_MULAW
	if (para->codec == AUD_FORMAT_MU_LAW)
		size = 80;
	else
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_G726
	if (para->codec == AUD_FORMAT_G726)
		size = 40;
	else 
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AUD32
	if (para->codec == AUD_FORMAT_AUD32)
	{
		if (para->rate == 8000)
			size = 320;
		else if (para->rate == 16000)
			size = 640;
		else if (para->rate == 32000)
			size = 1280;
	}
	else 
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AAC
	if (para->codec == AUD_FORMAT_AAC)
		size = 2048;
	else
#endif
		size = 160; // raw mode - AUD_FORMAT_S16_LE AUD_FORMAT_U16_LE


	buf = (uint8_t *)pvPortMalloc(para->size, GFP_KERNEL, MODULE_CLI);
	if(buf == NULL)
	{
		print_msg("Failed to allocate memory. --- %d\n", para->size);
		goto out;
	}
	print_msg("Buffer address:0x%08x\n", buf);
	print_msg("Buffer size:0x%08x\n", para->size);

	para->type = AUD_CAP_STREAM;
	retval = audio_open(&audio_stream_cap, para->type);
	if(retval < 0)
	{
		print_msg("Failed to open device for capture.\n");
		goto err_cap;
	}
	print_msg("audio open of capture is OK.\n");

	para->type = AUD_PLY_STREAM;
	retval = audio_open(&audio_stream_ply, para->type);
	if(retval < 0)
	{
		print_msg("Failed to open device for play.\n");
		goto err_ply;
	}
	print_msg("audio open of play is OK.\n");

	retval = audio_params_alloca(&params);
	if(retval < 0)
	{
		print_msg("Failed to allocate memory.\n");
		goto err_ply;
	}
	print_msg("Allocate parameters of capture OK\n");

	audio_params_set_rate(audio_stream_cap, params, para->rate);
	audio_params_set_format(audio_stream_cap, params, para->format);
	retval = audio_set_params(audio_stream_cap, params);
	if(retval < 0)
	{
		print_msg("Failed to set parameters of capture .\n");
		goto err_ply;
	}
	print_msg("Set parameters of capture OK\n");

	audio_params_set_rate(audio_stream_ply, params, para->rate);
	audio_params_set_format(audio_stream_ply, params, para->format);
	retval = audio_set_params(audio_stream_ply, params);
	if(retval < 0)
	{
		print_msg("Failed to set parameters of play .\n");
		goto err_ply;
	}
	print_msg("Set parameters of play OK\n");

	if (para->codec){
		buf_dst = (uint8_t *)pvPortMalloc(para->size, GFP_KERNEL, MODULE_CLI);
		if(buf_dst == NULL)
		{
			print_msg("Failed to allocate memory. --- %d\n", para->size);
			goto out;
		}

		buf_dst_size = size;
	}

    // codec init

	switch (para->codec){
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AAC
	case AUD_FORMAT_AAC:

		aac_params.samplerate = para->rate;
		aac_params.bitrate = para->bitrate;
		aac_params.afterburner = 1;
		aac_params.aot = 2;
		aac_params.bits_per_sample = 16;
		aac_params.channels = 1;
		aac_params.eld_sbr = 0;
		aac_params.vbr = 0;
		aac_params.type = AUD_CODEC_ENCODER;

		if (AUD_CODEC_SUCCESS != snx_aac_open (&aac_params, &aac_enc)){
			print_msg_queue("aac encoder open fail\n");
		}

		struct snx_aac_info_st info;
		snx_aac_get_info(aac_enc,&info);

		print_msg_queue("aac encoder return maxOutputBytes %d\n",info.encMaxOutBufBytes);
		print_msg_queue("aac encoder return frameLength %d\n",info.encFrameLength);

		//RBK reassign block size

		size = info.encFrameLength * para->format_size;

		aac_params.type = AUD_CODEC_DECODER;

		if (AUD_CODEC_SUCCESS != snx_aac_open (&aac_params, &aac_dec)){
			print_msg_queue("aac decoder open fail\n");
		}

		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AUD32
	case AUD_FORMAT_AUD32:

		aud32_params.samplerate = para->rate;
		aud32_params.bitrate    = para->bitrate;
		aud32_params.type = AUD_CODEC_ENCODER;

		if (AUD_CODEC_SUCCESS != snx_aud32_open (&aud32_params, &aud32_enc)){
			print_msg_queue("aac encoder open fail\n");
		}
		aud32_params.type = AUD_CODEC_DECODER;
		if (AUD_CODEC_SUCCESS != snx_aud32_open (&aud32_params, &aud32_dec)){
			print_msg_queue("aac decoder open fail\n");
		}

		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_ALAW
	case AUD_FORMAT_A_LAW:

		alaw_params.samplerate = para->rate;
		alaw_params.type = AUD_CODEC_ENCODER;
		if (AUD_CODEC_SUCCESS != snx_alaw_open (&alaw_params, &alaw_enc)){
			print_msg_queue("aud32 encoder open fail\n");
		}

		alaw_params.type = AUD_CODEC_DECODER;
		if (AUD_CODEC_SUCCESS != snx_alaw_open (&alaw_params, &alaw_dec)){
			print_msg_queue("aud32 decoder open fail\n");
		}
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_MULAW
	case AUD_FORMAT_MU_LAW:

		mulaw_params.samplerate = para->rate;
		mulaw_params.type = AUD_CODEC_ENCODER;
		if (AUD_CODEC_SUCCESS != snx_mulaw_open (&mulaw_params, &mulaw_enc)){
			print_msg_queue("mulaw encoder open fail\n");
		}

		mulaw_params.type = AUD_CODEC_DECODER;
		if (AUD_CODEC_SUCCESS != snx_mulaw_open (&mulaw_params, &mulaw_dec)){
			print_msg_queue("mulaw decoder open fail\n");
		}
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_G726
	case AUD_FORMAT_G726:

		size = 512;
		g726_params.samplerate = para->rate;
		g726_params.compress_ratio = 2;
		g726_params.pcm_bytes_per_frame = 512;
		g726_params.bytes_per_frame = 64;
		g726_params.type = AUD_CODEC_ENCODER;

		if (AUD_CODEC_SUCCESS != snx_g726_open (&g726_params, &g726_enc)){
			print_msg_queue("g726 encoder open fail\n");
		}

		g726_params.type = AUD_CODEC_DECODER;

		if (AUD_CODEC_SUCCESS != snx_g726_open (&g726_params, &g726_dec)){
			print_msg_queue("g726 decoder open fail\n");
		}

		break;
#endif
		default:
		break;
	}

	block_count = para->size / size;
	pbuf = buf;


	if (para->filename != NULL)
		bWriteFile = true;

	if (bWriteFile){
		fr = f_open(&fdst, para->filename, FA_CREATE_ALWAYS | FA_WRITE);
		if (fr)
		{
			print_msg("Open file of Record is fail\n");
			goto err_run;
		}
	}


	for (cur_blk = 0; cur_blk < block_count; cur_blk ++)
	{
reread:
		retval = audio_read(audio_stream_cap, pbuf, size, para->is_block);
		if (retval <= 0)
		{
			if(retval == -EAGAIN)
			{
				vTaskDelay(10 / portTICK_RATE_MS);
				goto reread;
			}

			audio_status(audio_stream_cap, &status);
			if(status == AUD_STATE_XRUN)
			{
				print_msg("overrun\n");
				audio_resume(audio_stream_cap);
				continue;
			}
			else
			{
				print_msg("Error capture status\n");
				goto err_run;
			}
		}

		if (para->codec){

			buf_dst_size = size;

			switch (para->codec){
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AAC
			case AUD_FORMAT_AAC:

				if (AUD_CODEC_SUCCESS == snx_aac_encode (aac_enc,pbuf,buf_dst,size,&buf_dst_size) && buf_dst_size>0){

					if (bWriteFile){
						fr = f_write(&fdst, buf_dst, buf_dst_size, &bw);
						if (fr)
						{
							print_msg("Write data is fail\n");
							goto err_run;
						}
					}

					memset(pbuf,0,size);
					iaacdecode =  snx_aac_decode (aac_dec,buf_dst,pbuf,buf_dst_size,&size);
					if (AUD_CODEC_SUCCESS == iaacdecode && size>0){
						//print_msg("aac codec done\n");
					}else{
						if ( AUD_CODEC_ERR_MOREDATA == iaacdecode){
							print_msg_queue("aac decoding needs more data %d \n",iaacdecode);
							goto reread;
						}

						print_msg_queue("aac decoding fail %d \n",iaacdecode);
					}
				}else{
					print_msg_queue("aac encoding fail \n");
				}
				//print_msg_queue("aac encoding feed %d  , return %d \n",size,buf_dst_size);

				break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AUD32
			case AUD_FORMAT_AUD32:
				if (AUD_CODEC_SUCCESS == snx_aud32_encode (aud32_enc,pbuf,buf_dst,size,&buf_dst_size) && buf_dst_size>0){

					if (bWriteFile){
						fr = f_write(&fdst, buf_dst, buf_dst_size, &bw);
						if (fr)
						{
							print_msg("Write data is fail\n");
							goto err_run;
						}
					}

					memset(pbuf,0,size);
					size = 0;
					iaud32decode = snx_aud32_decode (aud32_dec,buf_dst,pbuf,buf_dst_size,&size);
					if (AUD_CODEC_SUCCESS==iaud32decode && size>0){
						//print_msg_queue("aud32 codec done\n");
					}else{
						print_msg_queue("aud32 decoding fail\n");
					}
				}else
					print_msg_queue("aud32 encoding fail\n");
				break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_ALAW
			case AUD_FORMAT_A_LAW:
				if (AUD_CODEC_SUCCESS == snx_alaw_encode (alaw_enc,pbuf,buf_dst,size,&buf_dst_size) && buf_dst_size>0){

					if (bWriteFile){
						fr = f_write(&fdst, buf_dst, buf_dst_size, &bw);
						if (fr)
						{
							print_msg("Write data is fail\n");
							goto err_run;
						}
					}

					memset(pbuf,0,size);
					size = 0;
					if (AUD_CODEC_SUCCESS == snx_alaw_decode (alaw_dec,buf_dst,pbuf,buf_dst_size,&size) && size>0){
						//print_msg_queue("alaw codec done\n");
					}else
						print_msg_queue("alaw decoding fail\n");
				}else
					print_msg_queue("aac encoding fail\n");
				break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_MULAW
			case AUD_FORMAT_MU_LAW:
				if (AUD_CODEC_SUCCESS == snx_mulaw_encode (mulaw_enc,pbuf,buf_dst,size,&buf_dst_size) && buf_dst_size>0){

					if (bWriteFile){
						fr = f_write(&fdst, buf_dst, buf_dst_size, &bw);
						if (fr)
						{
							print_msg("Write data is fail\n");
							goto err_run;
						}
					}

					memset(pbuf,0,size);
					size = 0;
					if (AUD_CODEC_SUCCESS == snx_mulaw_decode (mulaw_dec,buf_dst,pbuf,buf_dst_size,&size) && size>0){
						//print_msg_queue("mulaw codec done\n");
					}else
						print_msg_queue("mulaw decoding fail\n");
				}else
					print_msg_queue("aac encoding fail\n");
				break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_G726
			case AUD_FORMAT_G726:
				if (AUD_CODEC_SUCCESS == snx_g726_encode (g726_enc,pbuf,buf_dst,size,&buf_dst_size) && buf_dst_size>0){

					if (bWriteFile){
						fr = f_write(&fdst, buf_dst, buf_dst_size, &bw);
						if (fr)
						{
							print_msg("Write data is fail\n");
							goto err_run;
						}
					}

					memset(pbuf,0,size);
					size = 0;
					if (AUD_CODEC_SUCCESS == snx_g726_decode (g726_dec,buf_dst,pbuf,buf_dst_size,&size) && size>0){
						print_msg_queue("g726 codec done\n");
					}else
						print_msg_queue("g726 decoding fail\n");
				}else
					print_msg_queue("g726 encoding fail\n");
				break;
#endif
			default:
				break;
			}
		}
rewrite:

		retval = audio_write(audio_stream_ply, pbuf, size, para->is_block);
		if (retval <= 0)
		{
			if(retval == -EAGAIN)
			{
				vTaskDelay(10 / portTICK_RATE_MS);
				goto rewrite;
			}

			audio_status(audio_stream_ply, &status);
			if(status == AUD_STATE_XRUN)
			{
				print_msg("overrun\n");
				audio_resume(audio_stream_ply);
				continue;
			}
			else
			{
				print_msg("Error play status\n");
				goto err_run;
			}
		}
			pbuf = pbuf + size;
	}


	switch (para->codec){
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AAC
	case AUD_FORMAT_AAC:
		snx_aac_close(aac_enc);
		snx_aac_close(aac_dec);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AUD32
	case AUD_FORMAT_AUD32:
		snx_aud32_close(aud32_enc);
		snx_aud32_close(aud32_dec);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_ALAW
	case AUD_FORMAT_A_LAW:
		snx_alaw_close(alaw_enc);
		snx_alaw_close(alaw_dec);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_MULAW
	case AUD_FORMAT_MU_LAW:
		snx_mulaw_close(mulaw_enc);
		snx_mulaw_close(mulaw_dec);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_G726
	case AUD_FORMAT_G726:
		snx_g726_close(g726_enc);
		snx_g726_close(g726_dec);
		break;
#endif
	default:
		break;
	}


err_run:
	if (bWriteFile)
		f_close(&fdst);
	audio_drain(audio_stream_ply);
	audio_drain(audio_stream_cap);
err_ply:
	audio_close(audio_stream_ply);
err_cap:
	audio_close(audio_stream_cap);
out:
	if (buf)
		vPortFree(buf);
	if (para->codec)
		vPortFree(buf_dst);

	vTaskDelete(NULL);
}


int cmd_verify_audio_twoway(int argc, char* argv[])
{
	uint32_t ubuf_size = 32768;
	uint32_t ubuf_threshold = 8192;
	int32_t  iis_block = 0;

	print_msg_queue("command parameters:\n");
	print_msg_queue("cli>verify>audio>twoway [AUDIO_TYPE][SEC][SAMPLE_RATE][BITRATE][FILENAME-optional]\n");

	if(argc != 5 && argc != 6)
	{
		print_msg_queue("parameter number error -- %d\n", argc);
		return -1;
	}

	memset(&audio_codec_para,'0',sizeof(struct audio_codec_st));

#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_ALAW
	if(strcmp(argv[1], "A-LAW") == 0)
	{
		audio_codec_para.codec = AUD_FORMAT_A_LAW;
		audio_codec_para.format = AUD_FORMAT_S16_LE;
		audio_codec_para.format_size = 2;
	}
	else
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_MULAW
	if(strcmp(argv[1], "MU-LAW") == 0)
	{
		audio_codec_para.codec = AUD_FORMAT_MU_LAW;
		audio_codec_para.format = AUD_FORMAT_S16_LE;
		audio_codec_para.format_size = 2;
	}
	else 
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_G726
	if(strcmp(argv[1], "G726") == 0)
	{
		audio_codec_para.codec = AUD_FORMAT_G726;
		audio_codec_para.format = AUD_FORMAT_S16_LE;
		audio_codec_para.format_size = 2;
	}
	else 
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AUD32
	if(strcmp(argv[1], "AUD32") == 0)
	{
		audio_codec_para.codec = AUD_FORMAT_AUD32;
		audio_codec_para.format = AUD_FORMAT_S16_LE;
		audio_codec_para.format_size = 2;
	}
	else 
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AAC
	if(strcmp(argv[1], "AAC") == 0)
	{
		audio_codec_para.codec = AUD_FORMAT_AAC;
		audio_codec_para.format = AUD_FORMAT_S16_LE;
		audio_codec_para.format_size = 2;
	}
	else 
#endif
	if(strcmp(argv[1], "S16_LE") == 0)
	{
		audio_codec_para.codec = NULL;
		audio_codec_para.format = AUD_FORMAT_S16_LE;
		audio_codec_para.format_size = 2;
	}
	else if(strcmp(argv[1], "U16_LE") == 0)
	{
		audio_codec_para.codec = NULL;
		audio_codec_para.format = AUD_FORMAT_U16_LE;
		audio_codec_para.format_size = 2;
	}
	else
	{
		print_msg("format parameter(%s) error.\n", argv[2]);
		return -1;
	}

	audio_codec_para.sec = simple_strtoul(argv[2], NULL, 0);
	audio_codec_para.rate = simple_strtoul(argv[3], NULL, 0);
	audio_codec_para.buf_size = ubuf_size;
	audio_codec_para.buf_threshold = ubuf_threshold;
	audio_codec_para.is_block = iis_block;
	audio_codec_para.bitrate = simple_strtoul(argv[4], NULL, 0);
	if (argc >=5)
		audio_codec_para.filename = argv[5];
	audio_codec_para.size = audio_codec_para.sec * audio_codec_para.rate * audio_codec_para.format_size;


	if (pdPASS != xTaskCreate(vAudioTwowayTask, "audio two way test", STACK_SIZE_8K *4, (void*) &audio_codec_para, 2, NULL))
	{
		print_msg("Could not create task of audio two way\n");
	}



	return 0;
}

struct cmd_info_st
{
	int32_t type;
	int32_t format;
	uint32_t format_size;
	uint32_t rate;
	int32_t time;
	int32_t is_block;
	uint32_t min_avail_size;
	uint32_t align_size;
};

struct aud_info_st
{
	struct cmd_info_st cmd_info;
	struct audio_stream_st *stream;
	xSemaphoreHandle xMutex;
	int32_t stream_finish;
	int32_t test_finish;
	char task_name[16];
};

#define TMSG(format,...)                print_msg_queue("%s: "format, pcTaskGetTaskName(xTaskGetCurrentTaskHandle()), ##__VA_ARGS__)

void audio_multi_help(void)
{
	print_msg("Usage:\n");
	print_msg("multi <type> <format> <sample rate> <time> [is_block] [min avail size] [align size]\n");
	print_msg("type: cap --- capture, ply --- playback\n");
	print_msg("format: S8, U8, S16_LE U16_LE\n");
	print_msg("sample rate: 8000, 16000, 32000, 48000, 11025, 22050, 44100\n");
	print_msg("time: capture or playback time in second.\n");
	print_msg("is_block: 0 --- Non block, 1 --- block\n");
}

int parse_audio_cmd(int argc, char *argv[], struct cmd_info_st *pcmd_info)
{
	print_msg("\n");

	if(pcmd_info == NULL)
	{
		print_msg("parameter 'pcmd_info' is NULL.\n");
		return -1;
	}

	if((argc < 5) || (argc > 8))
	{
		print_msg("parameter number error -- %d\n", argc);
		goto cmd_err;
	}

	if(strcmp(argv[1], "cap") == 0)
	{
		pcmd_info->type = AUD_CAP_STREAM;
		print_msg("Capture\n");
	}
	else if(strcmp(argv[1], "ply") == 0)
	{
		pcmd_info->type = AUD_PLY_STREAM;
		print_msg("Playback\n");
	}
	else
	{
		print_msg("type parameter(%s) error.\n", argv[1]);
		goto cmd_err;
	}

	if(strcmp(argv[2], "S8") == 0)
	{
		pcmd_info->format = AUD_FORMAT_S8;
		pcmd_info->format_size = 8;
	}
	else if(strcmp(argv[2], "U8") == 0)
	{
		pcmd_info->format = AUD_FORMAT_U8;
		pcmd_info->format_size = 8;
	}
	else if(strcmp(argv[2], "S16_LE") == 0)
	{
		pcmd_info->format = AUD_FORMAT_S16_LE;
		pcmd_info->format_size = 16;
	}
	else if(strcmp(argv[2], "U16_LE") == 0)
	{
		pcmd_info->format = AUD_FORMAT_U16_LE;
		pcmd_info->format_size = 16;
	}
	else
	{
		print_msg("format parameter(%s) error.\n", argv[2]);
		goto cmd_err;
	}

	pcmd_info->rate = simple_strtoul(argv[3], NULL, 0);
	pcmd_info->time = simple_strtoul(argv[4], NULL, 0);
	if(argc >= 6)
		pcmd_info->is_block = simple_strtoul(argv[5], NULL, 0);
	else
		pcmd_info->is_block = 0;

	if(argc >= 7)
		pcmd_info->min_avail_size = simple_strtoul(argv[6], NULL, 0);
	else
		pcmd_info->min_avail_size = 0;

	if(argc == 8)
		pcmd_info->align_size = simple_strtoul(argv[7], NULL, 0);
	else
		pcmd_info->align_size = 0;

	return 0;

cmd_err:
	print_msg("Command error.\n\n\n");
	audio_multi_help();
	return -1;
}

int audio_stream_init(struct aud_info_st *paud_info)
{
	int32_t retval = 0;
	struct audio_stream_st *audio_stream;
	struct params_st *params;
	struct cmd_info_st *pcmd_info = &(paud_info->cmd_info);

	retval = audio_open(&audio_stream, pcmd_info->type);
	if(retval < 0)
	{
		TMSG("Failed to open device.\n");
		retval = -3;
		goto out;
	}
	TMSG("audio open ok.\n");

	retval = audio_params_alloca(&params);
	if(retval < 0)
	{
		TMSG("Failed to allocate parameters.\n");
		retval = -4;
		goto err1;
	}	

	TMSG("Allcate paramters ok\n");
	audio_params_set_rate(audio_stream, params, pcmd_info->rate);
	audio_params_set_format(audio_stream, params, pcmd_info->format);
	if(pcmd_info->min_avail_size)
		audio_params_set_min_avail(audio_stream, params, pcmd_info->min_avail_size);
	if(pcmd_info->align_size)
		audio_params_set_align_size(audio_stream, params, pcmd_info->align_size);
	retval = audio_set_params(audio_stream, params);
	if(retval < 0)
	{
		TMSG("Failed to set paramters.\n");
		retval = -5;
		goto err2;
	}	
	audio_params_free(params);
	TMSG("Set paramters ok\n");

	paud_info->stream = audio_stream;
	goto out;

err2:
	audio_params_free(params);
err1:
	audio_close(audio_stream);
out:
	return retval;
}

void audio_stream_uninit(struct aud_info_st *paud_info)
{
	audio_drain(paud_info->stream);
	audio_close(paud_info->stream);
}

int32_t process_audio_stream(struct aud_info_st *paud_info)
{
	uint32_t size;
	uint8_t *buf, *pbuf;
	int32_t status;
	int32_t retval = 0;
	struct cmd_info_st *pcmd_info = &(paud_info->cmd_info);

	size = pcmd_info->time * pcmd_info->rate * pcmd_info->format_size / 8;
	buf = (uint8_t *)pvPortMalloc(size, GFP_KERNEL, MODULE_CLI);
	if(buf == NULL)
	{
		TMSG("Failed to allocate memory. --- %d\n", size);
		retval = -1;
		goto out;
	}
	TMSG("Buffer address:0x%08x\n", buf);
	TMSG("Buffer size:0x%08x\n", size);
	if(pcmd_info->type == AUD_PLY_STREAM)
		vTaskDelay(1000 / portTICK_RATE_MS);

	TMSG("run\n");
	pbuf = buf;
	while(size > 0)
	{
		if(pcmd_info->type == AUD_CAP_STREAM)
			retval = audio_read(paud_info->stream, pbuf, size, pcmd_info->is_block);
		else
			retval = audio_write(paud_info->stream, pbuf, size, pcmd_info->is_block);

		if(retval > 0)
		{
	//		TMSG("Process:%d Bytes\n", retval);
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

			audio_status(paud_info->stream, &status);
			if(status == AUD_STATE_XRUN)
			{
				if(pcmd_info->type == AUD_CAP_STREAM)
					TMSG("overrun\n");
				else
					TMSG("underrun\n");
				audio_resume(paud_info->stream);
			}
			else
			{
				TMSG("Error status(%d,%d)\n", retval, status);
				goto err;
			}
		}
	}

err:
	vPortFree(buf);
out:
	return retval;
}

void audio_multi_test_task(void *param)
{
	int32_t retval;
	struct aud_info_st *paud_info = (struct aud_info_st *)param;
	
	retval = audio_stream_init(paud_info);
	if(retval < 0)
	{
		TMSG("Failed to call 'audio_stream_init'\n");
		goto out;
	}
	
	retval = process_audio_stream(paud_info);
	if(retval < 0)
	{
		TMSG("Failed to call 'process_audio_stream'\n");
		goto err1;
	}

err1:
	audio_stream_uninit(paud_info);
out:
	TMSG("end\n");
	vPortFree(paud_info);
	vTaskDelete(NULL);
}

static uint32_t g_stream_index = 1;
#define PRIORITY_TASK_AUDIO_MULTI_TEST_STREAM	39
int cmd_verify_audio_multi(int argc, char* argv[])
{
	struct aud_info_st *paud_info;
	char task_name[16];
	int32_t retval = 0;

	paud_info = (struct aud_info_st *)pvPortMalloc(sizeof(struct aud_info_st), GFP_KERNEL, MODULE_CLI);
	if(paud_info == NULL)
	{
		print_msg("Failed to allocate 'struct aud_info_st' memory.\n");
		retval = -1;
		goto out;
	}

	retval = parse_audio_cmd(argc, argv, &(paud_info->cmd_info));
	if(retval < 0)
		goto err1;

	snprintf(task_name, 16, "audio stream%d", g_stream_index);
	g_stream_index++;

	if(pdPASS != xTaskCreate(audio_multi_test_task, task_name, STACK_SIZE_1K, paud_info,
				PRIORITY_TASK_AUDIO_MULTI_TEST_STREAM, NULL))
	{
		print_msg("Could not create '%s' task\n", task_name);
		retval = -2;
		goto err1;
	}

	goto out;

err1:
	vPortFree(paud_info);
out:
	return retval;
}

