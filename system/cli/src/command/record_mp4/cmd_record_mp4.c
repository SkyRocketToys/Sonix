#include "FreeRTOS.h"
#include <bsp.h>
#include <stdio.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include "cmd_rtsp.h"
#include <vc/snx_vc.h>
#include <isp/isp.h>
#include "printlog.h"
#include <libmid_vc/snx_mid_vc.h>
#include <nonstdlib.h>
#include "generated/snx_sdk_conf.h"
#include <string.h>
#include <libmid_fatfs/ff.h>
#include <sys_clock.h>
#include "mp4.h"

Mp4_Info_t Mp4Info;
struct snx_m2m m2m;
int run_task = 0;
int video_record_over = 0;
int audio_record_over = 0;


#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AAC
/***************************************************aac*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <libmid_audio/audio.h>
#include <audio/audio_dri.h>
#include <audio/audio_error.h>
//#include <fdk-aac/aacenc_lib.h>
#define _FF_INTEGER__  //RBK for RTOS typedef conflict -- _FF_INTEGER__
#include "fdk-aac/aacenc_lib.h"
#include "fdk-aac/aacdecoder_lib.h"


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
SIGNALING_MODE:
SIG_IMPLICIT: Implicit backward compatible signaling (default for non-MPEG-4 based AOT¡¯s and for the transport formats ADIF and ADTS)
SIG_EXPLICIT_BW_COMPATIBLE: Explicit backward compatible signaling
SIG_EXPLICIT_HIERARCHICAL: Explicit hierarchical signaling (default for MPEG-4 based AOT¡¯s and for all transport formats excluding ADIF and ADTS)
*/
//#define SIGNALING_MODE		SIG_IMPLICIT

struct aac_info_st1
{
	unsigned int aot;
	unsigned int eld_sbr;
	unsigned int sample_rate;
	unsigned int vbr;
	unsigned int bitrate;
	unsigned int high_quality;
	AACENC_InfoStruct enc_info;
};

static void usage_aac(void)
{
	print_msg( "usage:\n");
	print_msg( "./aac_encode -f aot eld_sbr sample_rate vbr bitrate high_quality infile outfile\n");
	print_msg( "./aac_encode -d aot eld_sbr sample_rate vbr bitrate high_quality capture_time(in second) outfile\n");
	print_msg( "\naot: audio object type\n");
	print_msg( "2: MPEG-4 AAC Low Complexity.\n");
	print_msg( "5: MPEG-4 AAC Low Complexity with Spectral Band Replication (HE-AAC).\n");
	print_msg( "23: MPEG-4 AAC Low-Delay.\n");
	print_msg( "39: MPEG-4 AAC Enhanced Low-Delay.\n");
	print_msg( "\neld_sbr: MPEG-4 AAC Enhanced Low-Delay with Spectral Band Replication.\n");
	print_msg( "1: enable SBR, 0: disable SBR\n");
	print_msg( "\nvbr: variable bitrate\n");
	print_msg( "0:cbr, 1~5:vbr\n");
	print_msg( "\nbitrate: bitrate in cbr mode, this parameter is useless in vbr mode.\n");
	print_msg( "\nhigh_quality: 1: high quality, 0:normal quality\n");
	print_msg( "\n\n");
}

static const char *aac_get_error1(AACENC_ERROR err)
{
	switch (err)
	{
		case AACENC_OK:
			return "No error";
		case AACENC_INVALID_HANDLE:
			return "Invalid handle";
		case AACENC_MEMORY_ERROR:
			return "Memory allocation error";
		case AACENC_UNSUPPORTED_PARAMETER:
			return "Unsupported parameter";
		case AACENC_INVALID_CONFIG:
			return "Invalid config";
		case AACENC_INIT_ERROR:
			return "Initialization error";
		case AACENC_INIT_AAC_ERROR:
			return "AAC library initialization error";
		case AACENC_INIT_SBR_ERROR:
			return "SBR library initialization error";
		case AACENC_INIT_TP_ERROR:
			return "Transport library initialization error";
		case AACENC_INIT_META_ERROR:
			return "Metadata library initialization error";
		case AACENC_ENCODE_ERROR:
			return "Encoding error";
		case AACENC_ENCODE_EOF:
			return "End of file";
		default:
			return "Unknown error";
	}
}


static int aac_encoder_init_2(HANDLE_AACENCODER *phandle, struct aac_info_st1 *paac_info)
{
	HANDLE_AACENCODER handle;
	AACENC_ERROR err;
	int retval = 0;

	err = aacEncOpen(&handle, 0x01|0x02|0x10, CHANNEL);
	if(err != AACENC_OK)
	{
		print_msg( "Unable to open the encoder: %s\n", aac_get_error1(err));
		retval = -11;
		goto out;
	}

	err = aacEncoder_SetParam(handle, AACENC_AOT, paac_info->aot);
	if(err != AACENC_OK)
	{
		print_msg( "Unable to set the AOT %d: %s\n", paac_info->aot, aac_get_error1(err));
		retval = -12;
		goto err;
	}

	if(
		(paac_info->aot == 39)
		&&
		(paac_info->eld_sbr == 1)
	  )
	{
		err = aacEncoder_SetParam(handle, AACENC_SBR_MODE, 1);
		if(err != AACENC_OK)
		{
			print_msg( "Unable to enable SBR for ELD: %s\n", aac_get_error1(err));
			retval = -13;
			goto err;
		}
	}

	err = aacEncoder_SetParam(handle, AACENC_SAMPLERATE, paac_info->sample_rate);
	if(err != AACENC_OK)
	{
		print_msg( "Unable to set the sample rate %d: %s\n", paac_info->sample_rate, aac_get_error1(err));
		retval = -14;
		goto err;
	}

	err = aacEncoder_SetParam(handle, AACENC_CHANNELMODE, CHANNEL_MODE);
	if(err != AACENC_OK)
	{
		print_msg( "Unable to set channel mode %d: %s\n", CHANNEL_MODE, aac_get_error1(err));
		retval = -15;
		goto err;
	}

	err = aacEncoder_SetParam(handle, AACENC_CHANNELORDER, CHANNEL_ORDER);
	if(err != AACENC_OK)
	{
		print_msg( "Unable to set wav channel order %d: %s\n", CHANNEL_ORDER, aac_get_error1(err));
		retval = -16;
		goto err;
	}

	if(paac_info->vbr)
	{
		err = aacEncoder_SetParam(handle, AACENC_BITRATEMODE, paac_info->vbr);
		if(err != AACENC_OK)
		{
			print_msg( "Unable to set the VBR bitrate mode %d: %s\n", paac_info->vbr, aac_get_error1(err));
			retval = -17;
			goto err;
		}
	}
	else
	{
		err = aacEncoder_SetParam(handle, AACENC_BITRATE, paac_info->bitrate);
		if(err != AACENC_OK)
		{
			print_msg( "Unable to set the bitrate %d: %s\n", paac_info->bitrate, aac_get_error1(err));
			retval = -18;
			goto err;
		}
	}

	err = aacEncoder_SetParam(handle, AACENC_TRANSMUX, TRANSPORT_TYPE);
	if(err != AACENC_OK)
	{
		print_msg( "Unable to set the transmux format: %s\n", aac_get_error1(err));
		retval = -19;
		goto err;
	}

	err = aacEncoder_SetParam(handle, AACENC_AFTERBURNER, paac_info->high_quality);
	if(err != AACENC_OK)
	{
		print_msg( "Unable to set afterburner to %d: %s\n", paac_info->high_quality, aac_get_error1(err));
		retval = -21;
		goto err;
	}

	err = aacEncEncode(handle, NULL, NULL, NULL, NULL);
	if(err != AACENC_OK)
	{
		print_msg( "Unable to initialize the encoder: %s\n", aac_get_error1(err));
		retval = -23;
		goto err;
	}

	err = aacEncInfo(handle, &(paac_info->enc_info));
	if(err != AACENC_OK)
	{
		print_msg( "Unable to get encoder info: %s\n", aac_get_error1(err));
		retval = -24;
		goto err;
	}



	*phandle = handle;

out:
	return retval;

err:
	aacEncClose(&handle);
	return retval;
}

static struct audio_stream_st *audio_device_init_1( unsigned int sample_rate, int cap_format)
{
	int retval = 0;
	struct audio_stream_st *audio_stream;
	int32_t type, format, format_size, is_block;
	uint32_t rate, buf_size, buf_threshold;
	struct params_st *params;
//	uint8_t *buf, *pbuf;
//	uint32_t size,size2;
//	int32_t i;

	print_msg("\naudio test task run.\n");

	
	type = AUD_CAP_STREAM;
	print_msg("Capture\n");
	

	//format = AUD_FORMAT_A_LAW;
	format = cap_format;
	format_size = 2;
	
	rate = sample_rate;
	buf_size = 8192;
	buf_threshold = 256;	
	is_block = 1;
/*
	//size = 5 * rate * format_size;
	size2 = size = cap_size;
	buf = (uint8_t *)pvPortMalloc(size, GFP_KERNEL, MODULE_CLI);
	if(buf == NULL)
	{
		print_msg("Failed to allocate memory. --- %d\n", size);
		retval = -2;
		goto out;
	}
	print_msg("Buffer address:0x%08x\n", buf);
	print_msg("Buffer size:0x%08x\n", size);
*/

	retval = audio_open(&audio_stream, type);
	if(retval < 0)
	{
		print_msg("Failed to open device.\n");
		retval = -3;
		goto cmd_err;
	}
	print_msg("audio open ok.\n");

	retval = audio_params_alloca(&params);
	if(retval < 0)
	{
		print_msg("Failed to allocate memory.\n");
		retval = -4;
		goto cmd_err;
	}	

	print_msg("Allcate paramters ok\n");
	audio_params_set_rate(audio_stream, params, rate);
	audio_params_set_format(audio_stream, params, format);
	retval = audio_set_params(audio_stream, params);
	if(retval < 0)
	{
		print_msg("Failed to set paramters.\n");
		retval = -5;
		goto cmd_err;
	}	
	print_msg("Set paramters ok\n");
	return audio_stream;

	return NULL;

cmd_err:
	print_msg("init audio device error.\n");
	return NULL;
}

static void run_aac_1()
{
	struct audio_stream_st *audio_stream=NULL;
	unsigned int frames, remain_frame, frame_size;
	char *pcm_buf, *aac_buf, *buf;
	int pcm_buf_size, aac_buf_size, size;
	int retval = 0;
	int res;
	HANDLE_AACENCODER handle;
	AACENC_ERROR err;
	AACENC_BufDesc in_buf   = { 0 }, out_buf = { 0 };
	AACENC_InArgs  in_args  = { 0 };
	AACENC_OutArgs out_args = { 0 };
	int pcm_buf_identifier = IN_AUDIO_DATA, aac_buf_identifier = OUT_BITSTREAM_DATA;
	int pcm_element_size = 2, aac_element_size = 1;
	struct aac_info_st1 aac_info;
	int is_file_mode = 0;
	unsigned int cap_time, cap_frames;
	//char *infile_name, *outfile_name;
	FIL out_fp;
	int ret;

	is_file_mode = 0;
	aac_info.aot = 2;

	aac_info.eld_sbr = 0;

	//aac_info.sample_rate = 8000;
	aac_info.sample_rate = 11025;
	aac_info.vbr = 0;

	aac_info.bitrate = 16000;
	
	aac_info.high_quality = 1;
	cap_time = 1000;
	if(is_file_mode)
		print_msg("File mode:\n");
	else
	{
		print_msg("Device mode:\n");
		print_msg("Capture time: %d second\n", cap_time);
	}
	print_msg("audio object type:%d\n", aac_info.aot);
	print_msg("sbr on eld:%d\n", aac_info.eld_sbr);
	print_msg("sample rate:%d\n", aac_info.sample_rate);
	print_msg("VBR:%d\n", aac_info.vbr);
	print_msg("bit rate:%d\n", aac_info.bitrate);
	print_msg("high quality:%d\n", aac_info.high_quality);
	print_msg("\n\n");
	
/* 
	res = f_open(&out_fp, "test.aac", FA_WRITE|FA_CREATE_ALWAYS);
	if(res != FR_OK)
	{
		print_msg( "Open output file  failed...   %d\n", res);
		retval = -3;
		goto end;
	}
 
	if(is_file_mode)
	{
		in_fp = fopen(infile_name, "rb");
		if(in_fp == NULL)
		{
			print_msg( "open input file %s failed\n", infile_name);
			retval = -4;
			goto end;
		}
	}
	else
	{
		retval = audio_device_init_1(aac_info.sample_rate, &pcm);
		if(retval < 0)
			goto end;
	}*/

	retval = aac_encoder_init_2(&handle, &aac_info);
	if(retval < 0)
		goto end;
	
	

	print_msg("frame size:%d\n", aac_info.enc_info.frameLength);	
	print_msg("max output bytes:%d\n\n", aac_info.enc_info.maxOutBufBytes);
	frame_size = aac_info.enc_info.frameLength;
	pcm_buf_size = frame_size * PER_SAMPLE_BYTE;
	pcm_buf = (char *) malloc(pcm_buf_size);
	if(pcm_buf == NULL)
	{
		print_msg( "malloc pcm buffer failed\n");
		retval = -5;
		goto end;
	}

	aac_buf_size = aac_info.enc_info.maxOutBufBytes;
	aac_buf = (char *) malloc(aac_buf_size);
	if(aac_buf == NULL)
	{
		print_msg( "malloc opus buffer failed\n");
		retval = -6;
		goto end;
	}

	{
		audio_stream = audio_device_init_1(aac_info.sample_rate, AUD_FORMAT_S16_LE);
		if(audio_stream == NULL)
			goto end;
	}


	print_msg("start capture\n");
	cap_frames = 0;
	remain_frame = 0;
	while(run_task)
	{
		if(!is_file_mode)
		{
			if(cap_frames >= (aac_info.sample_rate * cap_time))
			{
				print_msg("end capture\n");
				f_close(&out_fp);
				while(1)
					vTaskDelay( 100000/portTICK_PERIOD_MS );
				goto end;
			}
		}
		frames = frame_size - remain_frame;
		buf = pcm_buf + remain_frame * PER_SAMPLE_BYTE;
		while(frames > 0)
		{
		
			{
				res = audio_read(audio_stream, buf, frames*PER_SAMPLE_BYTE, 1);
				if(res < 0)
				{
					if (res == -EAGAIN)
					{
						print_msg("-----------------------retval:%d Bytes\n", retval);
						vTaskDelay(50 / portTICK_RATE_MS);
						continue;
					}
					else
					{
						print_msg("error from readi: %d\n",res); 
						retval = -52;
						goto end;
					}
				}
			}

			buf += res;
			frames -= res/PER_SAMPLE_BYTE;
			cap_frames += res/PER_SAMPLE_BYTE;
		}

		frames = frame_size - frames;
		size = frames * PER_SAMPLE_BYTE;

		if(frames < frame_size)
			in_args.numInSamples = -1;
		else
		{
			in_args.numInSamples = frames;
			in_buf.numBufs = 1;
			in_buf.bufs = (void**)&pcm_buf;
			in_buf.bufferIdentifiers = &pcm_buf_identifier;
			in_buf.bufSizes = &size;
			in_buf.bufElSizes = &pcm_element_size;
		}

		out_buf.numBufs = 1;
		out_buf.bufs = (void**)&aac_buf;
		out_buf.bufferIdentifiers = &aac_buf_identifier;
		out_buf.bufSizes = &aac_buf_size;
		out_buf.bufElSizes = &aac_element_size;

		err = aacEncEncode(handle, &in_buf, &out_buf, &in_args, &out_args);
		if(err != AACENC_OK)
		{
			if((frames < frame_size) && (err == AACENC_ENCODE_EOF))
			{
				print_msg("end capture\n");
				goto end;
			}
			print_msg( "Unable to encode frame: %s\n", aac_get_error1(err));
			retval = -53;
			goto end;
		}

		remain_frame = frames - out_args.numInSamples;
		if(remain_frame)
			memcpy(pcm_buf, pcm_buf + out_args.numInSamples * PER_SAMPLE_BYTE, remain_frame * PER_SAMPLE_BYTE);

		/* write file */
	/*	if(out_args.numOutBytes)
			fwrite(aac_buf, 1, out_args.numOutBytes, out_fp);*/

		if(out_args.numOutBytes){
			struct timeval tval;
			unsigned int time;
			
			//print_msg("Encoder over. aac size is %d\n",out_args.numOutBytes);
			gettimeofday(&tval, NULL);;
			time = tval.tv_sec*1000+(tval.tv_usec+500)/1000;
			ret = mp4_write_packet(&Mp4Info, MP4_AUDIO, 0, (unsigned char*)aac_buf,out_args.numOutBytes,time);
			if(ret != pdPASS)
			{
				print_msg("%s:mp4 write video packet fail\n", __func__);
				break;
			}
			/*f_write(&out_fp, aac_buf, out_args.numOutBytes, &write_size);
			if(write_size != out_args.numOutBytes){
				print_msg("capture size is %d, write size is %d\n", out_args.numOutBytes, write_size);
			}*/
		}
	}	


end:
/*	if(in_fp != NULL)
		fclose(in_fp);

	if(out_fp != NULL)
		fclose(out_fp);
*/
	if(pcm_buf != NULL)
		free(pcm_buf);

	if(aac_buf != NULL)
		free(aac_buf);

	if(audio_stream != NULL)
	{
		audio_drain(audio_stream);
		audio_close(audio_stream);
	}
	if(handle != NULL)
		aacEncClose(&handle);

	print_msg("stop audio\n");	
	audio_record_over = 1;
	//vTaskDelay( 3000/portTICK_PERIOD_MS );
	//print_msg("stop audio over\n");	
	vTaskDelete(NULL);

	print_msg( "args error\n");
	usage_aac();

	vTaskDelete(NULL);
}





/***************************************************aac*********************************************************************/
#endif //if AAC defined



static int video_set_init1()
{
	int res;
	struct snx_m2m *psnx_m2m = &m2m;
	snx_video_init (psnx_m2m);
	// set video
	psnx_m2m->channel = 0; //isp channel
	snx_video_set_resolution(psnx_m2m, 1280, 720);
	
	snx_isp_set_fps(psnx_m2m, 30);

	snx_video_set_scale(psnx_m2m, 0);

	
	snx_set_mdrc_en(psnx_m2m,0, 0);	// disable motion detection rate control
	snx_set_mdcnt_en(psnx_m2m, 0, 0);	// disable auto low bitrate control    
	snx_video_set_mode(psnx_m2m, 0, FMT_H264);
	

		
	snx_video_set_fps(psnx_m2m, 0, 30, FMT_H264);
	snx_video_set_bps(psnx_m2m, 0, 3 * 1024 *1024);
	snx_set_mbrc_en(psnx_m2m, 0, 0);		// disable macro block based rate control
	snx_set_qp_boundary(psnx_m2m, 0, 35, 20);	// set h264 QP range between 20~35
	snx_set_delta_qp(psnx_m2m, 0, 5, 5);	// delta QP up bound (default 8, dashcam suggest set 5)
		// 1 = I(non-refP)(non-refP)(non-refP)
		// 2 = I(refP)(non-refP)(refP)(non-refP)
		// 3 = I(refP)(refP)(non-refP)(refP)(refP)(non-refP)
	snx_video_set_refp(psnx_m2m, 0, 2);
	snx_video_set_gop(psnx_m2m, 0,0);


	
	if((res=snx_video_start(psnx_m2m))==pdFAIL)
	{
        snx_video_uninit (psnx_m2m);
		return pdFAIL;
	}


   return pdPASS;
}


static void task_video_get1( void *pvParameters )
{
	unsigned char *pFrame;
	unsigned int uiFrameSize;
	int ret;
	uint8_t IFrame;
	unsigned int time;
	struct timeval tval;
	struct snx_m2m *p_m2m = &m2m;
	video_set_init1();

	while(run_task)
	{
		snx_video_read(p_m2m);
		uiFrameSize = snx_video_h264_stream(p_m2m, 0, (unsigned int *)&pFrame);
		if(uiFrameSize)
		{
			IFrame = snx_video_is_keyframe(p_m2m, 0);
			tval = snx_video_get_timestamp(p_m2m);
			time = tval.tv_sec*1000+(tval.tv_usec+500)/1000;
			ret = mp4_write_packet(&Mp4Info, MP4_VIDEO, IFrame, pFrame,uiFrameSize,time);
			if(ret != pdPASS)
			{
				print_msg("%s:mp4 write video packet fail\n", __func__);
				break;
			}
				
		}
		

	}


	snx_video_stop(p_m2m);
	snx_video_uninit(p_m2m);
	print_msg("stop video\n");	
	video_record_over = 1;
	//vTaskDelay( 3000/portTICK_PERIOD_MS );
	//print_msg("stop video over\n");	
	vTaskDelete(NULL);
}

int cmd_record_mp4_start(int argc, char* argv[])
{
	int ret;

	Mp4VdoInfo_t VdoInfo;
	Mp4AdoInfo_t AdoInfo;
	Mp4WirteBufferInitInfo_t WriteBufferInitParam;
	VdoInfo.width = 1280;
	VdoInfo.height= 720;
	VdoInfo.ucFps = 30; 
	VdoInfo.ucScale= 0;
	VdoInfo.ucStreamMode = 2;
        memset(&AdoInfo, 0, sizeof(AdoInfo));
#ifdef AUD_FORMAT_AAC
	AdoInfo.uiFormat = AUD_FORMAT_AAC;
	AdoInfo.uiSampleRate = 11025;
	AdoInfo.ucChannel = 1;
	AdoInfo.ucBitsPerSample = 16;
	AdoInfo.uiFps = 10;
	AdoInfo.uiBitRate = 16000;
#endif
	WriteBufferInitParam.write_buf_size= 1048576;
	WriteBufferInitParam.write_unit_to_file= 65536;


	memset(&Mp4Info, 0, sizeof(Mp4_Info_t));
	if(mp4_init(&Mp4Info,&WriteBufferInitParam, 1) != pdPASS){
		print_msg("<<test>><%s><%d> Could not init mp4\n", __func__, __LINE__);
		return pdFAIL;
	}


	if(mp4_set_stream_info(&Mp4Info, &VdoInfo, NULL)!=pdPASS){
		print_msg("<<test>><%s><%d> Could not set mp4 info\n", __func__, __LINE__);
		mp4_uninit(&Mp4Info);
		return pdFAIL;
	}
	{
		system_date_t date;
		get_date(&date);
		sprintf(Mp4Info.Filename,"test%04d_%02d_%02d_%02d_%02d_%02d.mp4",date.year,date.month,date.day,date.hour,date.minute,date.second);      
	}
	if((ret = mp4_write_header(&Mp4Info)) != pdPASS)
	{
		print_msg("%s:mp4 write header fail(%d!=%d)\n",  __func__,ret, pdPASS);
		mp4_uninit(&Mp4Info);
		return pdFAIL;
	}

	run_task = 1;
	video_record_over = 0;
	audio_record_over = 0;
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AAC
	if (pdPASS != xTaskCreate(run_aac_1, "run_aac_1", 2048 * 4, NULL,
							PRIORITY_TASK_APP_TEST01, NULL))
		print_msg("<<test>><%s><%d> Could not create task\n", __func__, __LINE__);
#endif




	if (pdPASS != xTaskCreate(task_video_get1, "vdo_get_rec_stream", STACK_SIZE_4K, NULL,
			PRIORITY_TASK_APP_TEST01, NULL))
	{
		print_msg("Could not create task video get\n");
		return pdFAIL;
	}


	
	return pdPASS;
}



int cmd_record_mp4_stop(int argc, char* argv[])
{
	int ret;
	
	run_task = 0;
	//vTaskDelay( 1000/portTICK_PERIOD_MS );
	while((video_record_over == 0) || (audio_record_over == 0))
		vTaskDelay( 100/portTICK_PERIOD_MS );
	print_msg(" stop record\n");
	ret = mp4_write_trailer(&Mp4Info);
	if(ret != pdPASS)
	{
		print_msg("mp4 write trailer fail\n");
	}
	vTaskDelay( 3000/portTICK_PERIOD_MS );
	print_msg(" stop record trailer %d\n",ret);
	mp4_uninit(&Mp4Info);
	

	print_msg(" (task  stop)\n");
	return pdPASS;
	
}



