#include <FreeRTOS.h>
#include <bsp.h>
#include <task.h>
#include <cmd_verify.h>
#include <string.h>
#include <audio/audio_dri.h>
#include <audio/audio_error.h>
#include <audio.h>
#include <libmid_fatfs/ff.h>
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
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AMRNB
#include <snx_amrnb.h>
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AMRWB
#include <snx_amrwb.h>
#endif
#include <libmid_audio/audio_codec.h>

typedef struct audio_cmd_codec_st {
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
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AAC
	struct snx_aac_params_st aac_para;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AUD32
	struct snx_aud32_params_st aud32_para;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_ALAW
	struct snx_alaw_params_st alaw_para;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_MULAW
	struct snx_mulaw_params_st mulaw_para;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_G726
	struct snx_g726_params_st g726_para;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AMRNB
	struct snx_amrnb_params_st amrnb_para;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AMRWB
	struct snx_amrwb_params_st amrwb_para;
#endif
}audio_cmd_codec_st;

audio_cmd_codec_st audio_codec_para;

#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AAC
static struct snx_aac_st *g_aac;
static struct snx_aac_info_st g_aac_info;
#endif

#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AUD32
static struct snx_aud32_st *g_aud32;
static struct snx_aud32_info_st g_aud32_info;
#endif

#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_ALAW
static struct snx_alaw_st *g_alaw;
static struct snx_alaw_info_st g_alaw_info;
#endif

#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_MULAW
static struct snx_mulaw_st *g_mulaw;
static struct snx_mulaw_info_st g_mulaw_info;
#endif

#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_G726
static struct snx_g726_st *g_g726;
static struct snx_g726_info_st g_g726_info;
#endif

#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AMRNB
static struct snx_amrnb_st *g_amrnb;
static struct snx_amrnb_info_st *g_amrnb_info;
static int amrnb_frame = 0;
#endif

#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AMRWB
static struct snx_amrwb_st *g_amrwb;
static struct snx_amrwb_info_st *g_amrwb_info;
// static int amrwb_frame = 0;
#endif

void cmd_codec_help(void)
{
	print_msg_queue("1. argument 0 is acodec\n");
	print_msg_queue("2. argument 1 is operation type:\n\tcap : record\n\tply : playback\n");
	print_msg_queue("3. argument 2 is the codec type:\n");
	print_msg_queue("\tA-LAW means a-law codec\n");
	print_msg_queue("\tMU-LAW means mu-law codec\n");
	print_msg_queue("\tG726 means G.726 codec\n");
	print_msg_queue("\tAUD32 means audio32 codec\n");
	print_msg_queue("\tAAC means AAC codec\n");
	print_msg_queue("4. argument 3 is seconds\n");
	print_msg_queue("5. argument 4 is sample rate\n");
	print_msg_queue("6. argument 5 is file name\n");
}

void cmd_codec_get_info(audio_cmd_codec_st *para)
{
	int32_t ret;
	switch (para->codec)
	{
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_ALAW
	case AUD_FORMAT_A_LAW:
		ret = snx_alaw_get_info (g_alaw, &g_alaw_info);
		print_msg_queue("the bit rate is %u\n",g_alaw_info.bitrate);
		print_msg_queue("the sample rate is %u\n",g_alaw_info.samplerate);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_MULAW
	case AUD_FORMAT_MU_LAW:
		ret = snx_mulaw_get_info (g_mulaw, &g_mulaw_info);
		print_msg_queue("the bit rate is %u\n",g_mulaw_info.bitrate);
		print_msg_queue("the sample rate is %u\n",g_mulaw_info.samplerate);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_G726
	case AUD_FORMAT_G726:
		ret = snx_g726_get_info (g_g726, &g_g726_info);
		print_msg_queue("the bit rate is %u\n",g_g726_info.bitrate);
		print_msg_queue("the sample rate is %u\n",g_g726_info.samplerate);
		print_msg_queue("the compress ratio is %u\n",g_g726_info.compress_ratio);
		print_msg_queue("the PCM bytes per frame is %u\n",g_g726_info.pcm_bytes_per_frame);
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
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AMRNB
	case AUD_FORMAT_AMRNB:
		ret = snx_amrnb_get_info (g_amrnb, &g_amrnb_info);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AMRWB
	case AUD_FORMAT_AMRWB:
		ret = snx_amrwb_get_info (g_amrwb, &g_amrwb_info);
		break;
#endif
	default:
		break;
	}
	return ret;
}
int32_t cmd_codec_open(audio_cmd_codec_st *para)
{
	int32_t ret;
	switch (para->codec)
	{
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_ALAW
	case AUD_FORMAT_A_LAW:
		ret = snx_alaw_open (&para->alaw_para, &g_alaw);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_MULAW
	case AUD_FORMAT_MU_LAW:
		ret = snx_mulaw_open (&para->mulaw_para, &g_mulaw);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_G726
	case AUD_FORMAT_G726:
		ret = snx_g726_open (&para->g726_para, &g_g726);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AUD32
	case AUD_FORMAT_AUD32:
		ret = snx_aud32_open (&para->aud32_para, &g_aud32);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AAC
	case AUD_FORMAT_AAC:
		ret = snx_aac_open (&para->aac_para, &g_aac);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AMRNB
	case AUD_FORMAT_AMRNB:
		ret = snx_amrnb_open (&para->amrnb_para, &g_amrnb);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AMRWB
	case AUD_FORMAT_AMRWB:
		ret = snx_amrwb_open (&para->amrwb_para, &g_amrwb);
		break;
#endif
	default:
		break;
	}
	return ret;
}

void cmd_codec_close(audio_cmd_codec_st *para)
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
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AMRNB
	case AUD_FORMAT_AMRNB:
		snx_amrnb_close(g_amrnb);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AMRWB
	case AUD_FORMAT_AMRWB:
		snx_amrwb_close(g_amrwb);
		break;
#endif
	default:
		break;
	}
}

int32_t cmd_codec_encode(audio_cmd_codec_st *para, uint8_t *p_src, uint8_t *p_dst, int32_t src_bytes, int32_t *p_dst_bytes)
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
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AMRNB
	case AUD_FORMAT_AMRNB:
		ret = snx_amrnb_encode(g_amrnb, p_src, p_dst, src_bytes, p_dst_bytes);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AMRWB
	case AUD_FORMAT_AMRWB:
		ret = snx_amrwb_encode(g_amrwb, p_src, p_dst, src_bytes, p_dst_bytes);
		break;
#endif
	default:
		break;
	}
	return ret;
}

int32_t cmd_codec_decode(audio_cmd_codec_st *para, uint8_t *p_src, uint8_t *p_dst, int32_t src_bytes, int32_t *p_dst_bytes)
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
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AMRNB
	case AUD_FORMAT_AMRNB:
		ret = snx_amrnb_decode(g_amrnb, p_src, p_dst, src_bytes, p_dst_bytes);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AMRWB
	case AUD_FORMAT_AMRWB:
		ret = snx_amrwb_decode(g_amrwb, p_src, p_dst, src_bytes, p_dst_bytes);
		break;
#endif
	default:
		break;
	}
	return ret;
}


void cmd_codec_init(audio_cmd_codec_st *para, uint32_t *psize, uint32_t *pblock_count, uint32_t *pbuf_dst_size)
{
	para->is_block = 0;
	para->buf_size = 8192;
	para->buf_threshold = 1024;
	para->size = para->rate * para->sec * para->format_size;
	switch (para->codec)
	{
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_ALAW
	case AUD_FORMAT_A_LAW:
		*psize = 80;
		*pblock_count = para->size / (*psize);
		*pbuf_dst_size = 40;
		para->alaw_para.samplerate = para->rate;
		para->alaw_para.type = para->type;
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_MULAW
	case AUD_FORMAT_MU_LAW:
		*psize = 80;
		*pblock_count = para->size / (*psize);
		*pbuf_dst_size = 40;
		para->mulaw_para.samplerate = para->rate;
		para->mulaw_para.type = para->type;
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_G726
	case AUD_FORMAT_G726:
		para->g726_para.samplerate = para->rate;
		para->g726_para.type = para->type;
		*psize = para->g726_para.pcm_bytes_per_frame * 10;
		*pbuf_dst_size = para->g726_para.bytes_per_frame * 10;
		*pblock_count = para->size / (*psize);
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AUD32
	case AUD_FORMAT_AUD32:
		para->aud32_para.bitrate = para->bitrate;
		para->aud32_para.samplerate = para->rate;
		if (para->type == AUD_CAP_STREAM)
			para->aud32_para.type = AUD_CODEC_ENCODER;
		else
			para->aud32_para.type = AUD_CODEC_DECODER;
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
		para->aac_para.samplerate = para->rate;
		para->aac_para.bitrate = para->bitrate;
		if (para->type == AUD_CAP_STREAM)
			para->aac_para.type = AUD_CODEC_ENCODER;
		else
			para->aac_para.type = AUD_CODEC_DECODER;
		*psize = 2048;
		*pblock_count = para->size / (*psize);
		*pbuf_dst_size = 128;
		break;
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AMRNB
	case AUD_FORMAT_AMRNB:
		para->amrnb_para.samplerate = para->rate;
		para->amrnb_para.bitrate = BR475;
		para->amrnb_para.dtx = 0;
		if (para->type == AUD_CAP_STREAM)
			para->amrnb_para.type = AMRNB_CAP;
		else
			para->amrnb_para.type = AMRNB_PLY;
		*psize = 320;
		*pblock_count = para->size / (*psize);
		amrnb_frame = *pblock_count;
		break;
#endif
	default:
		para->codec = 0;
		*psize = 160;
		*pblock_count = para->size / (*psize);
		break;
	}
}

void vAudioCodec_cmd_Task(void* pvParameters)
{
	audio_cmd_codec_st *para = (audio_cmd_codec_st *)pvParameters;
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
	uint32_t file_size;

	// for AMR-NB decode
	uint8_t toc, ft;
	int16_t packed_size[16] = {12, 13, 15, 17, 19, 20, 26, 31, 5, 0, 0, 0, 0, 0, 0, 0};

	cmd_codec_init(para, &size, &block_count, &buf_dst_size);

	buf = (uint8_t *)pvPortMalloc(size, GFP_KERNEL, MODULE_CLI);
	if(buf == NULL)
	{
		print_msg_queue("Failed to allocate memory. --- %d\n",size);
		goto out;
	}

	if (para->codec)
	{
		retval = cmd_codec_open(para);
		if (retval != AUD_CODEC_SUCCESS)
		{
			print_msg_queue("codec open is fail\n");
			goto out;
		}
	}

	if (para->codec)
		cmd_codec_get_info(para);

	if (para->codec)
	{
		buf_dst = (uint8_t *)pvPortMalloc(size, GFP_KERNEL, MODULE_CLI);
		if (buf_dst == NULL)
		{
			print_msg_queue("Failed to allocate memory. --- %d\n",size);
			goto out;
		}
		pbuf_dst = buf_dst;
	}

	if (para->type == AUD_CAP_STREAM)
	{
		 fr = f_open(&fdst, (char*)para->filename, FA_CREATE_ALWAYS | FA_WRITE);
		 if (fr)
		 {
			 print_msg_queue("Open file of Record is fail\n");
			 goto out;
		 }
	}
	else
	{
		fr = f_open(&fsrc, (char*)para->filename, FA_OPEN_EXISTING | FA_READ);
		if (fr)
		{
			print_msg_queue("Open file of play is fail\n");
			goto out;
		}
	}

	if(para->type == AUD_PLY_STREAM)
		vTaskDelay(1000 / portTICK_RATE_MS);

	retval = audio_open(&audio_stream, para->type);
	if(retval < 0)
	{
		print_msg_queue("Failed to open device.\n");
		goto err1;
	}
	print_msg_queue("audio open ok.\n");

	retval = audio_params_alloca(&params);
	if(retval < 0)
	{
		print_msg_queue("Failed to allocate memory.\n");
		goto err2;
	}
	print_msg_queue("Allcate paramters ok\n");
	audio_params_set_rate(audio_stream, params, para->rate);
	audio_params_set_format(audio_stream, params, para->format);
	retval = audio_set_params(audio_stream, params);

	if(retval < 0)
	{
		print_msg_queue("Failed to set paramters.\n");
		goto err2;
	}
	print_msg_queue("Set paramters ok\n");

	pbuf = buf;
	type = para->type;
	is_block = para->is_block;
	if ( (para->codec == AUD_FORMAT_AAC) && (para->type == AUD_PLY_STREAM) ) //Dynamic compress ratio
	{
		uint8_t frame_len[4];
		uint32_t len,dec_len;
		file_size = (uint32_t)(f_size(&fsrc));
		fr = f_read(&fsrc, buf_dst, size, &br);
		while (file_size)
		{
			pbuf = buf;
			//parse header get frame len
			frame_len[0] = ((pbuf_dst[4] & 0x1F) << 3) |
							((pbuf_dst[5] & 0xE0) >> 5) ;
			frame_len[1] = ((pbuf_dst[3] & 0x3) << 3) |
							((pbuf_dst[4] & 0xE0) >> 5);
			frame_len[2] = 0x0;
			frame_len[3] = 0x0;
			len = *((uint32_t*)frame_len);
			dec_len=2048;
			//print_msg_queue("Decode frame %d to %d\n",len, dec_len);
			//decode aac
			retval = snx_aac_decode(g_aac, (uint8_t *)pbuf_dst, (uint8_t *)(pbuf), len, &dec_len);
			if (retval != AUD_CODEC_SUCCESS)
			{
				print_msg_queue("decode is fail\n");
				break;
			}
			uint32_t write_size= dec_len;
aac_rewrite:
			//playback
			retval = audio_write(audio_stream, pbuf, write_size, is_block);
			if (retval <= 0)
			{
				if(retval == -EAGAIN) {
					vTaskDelay(100 / portTICK_RATE_MS);
					goto aac_rewrite;
				}

				audio_status(audio_stream, &status);
				if(status == AUD_STATE_XRUN)
				{
					print_msg_queue("underrun\n");
					audio_resume(audio_stream);
				}
				else
				{
					print_msg_queue("Error playback status\n");
					goto err3;
				}
			}
			else if (retval != write_size)
			{
				vTaskDelay(10 / portTICK_RATE_MS);
				write_size -= retval;
				pbuf += retval;
				goto aac_rewrite;
			}
			else
			{
				//print_msg_queue("Write frame done!!\n");
			}
			//shift data and read left file data
			memcpy(buf_dst, buf_dst+len, size-len);
			file_size -= len;
			if (file_size)
				fr = f_read(&fsrc, buf_dst+(size-len), len, &br);
		}
	}
	else //static compress ratio
	{
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
						print_msg_queue("overrun\n");
						audio_resume(audio_stream);
					}
					else
					{
						print_msg_queue("Error capture status\n");
						goto err3;
					}
				}

				// audio codec encode
				if (para->codec)
				{
					retval = cmd_codec_encode(para, pbuf, pbuf_dst, size, (int32_t*)&buf_dst_size);
					if (retval == AUD_CODEC_SUCCESS)
					{
						fr = f_write(&fdst, pbuf_dst, buf_dst_size, &bw);
					}
					else
					{
						print_msg_queue("encode(%d) is fail\n",cur_blk);
						break;
					}
				}
				else
					fr = f_write(&fdst, pbuf, size, &bw);

				if (fr)
				{
					print_msg_queue("Write data is fail\n");
					goto err1;
				}
			}
			else
			{
readmore:
				if (para->codec)
				{
					if (para->codec == AUD_FORMAT_AMRNB)
					{
						fr = f_read(&fsrc, buf_dst, 1, &br);
						toc = *buf_dst;
						ft = (toc >> 3) & 0x0F;
						fr = f_read(&fsrc, buf_dst + 1, packed_size[ft], &br);
						buf_dst_size = packed_size[ft] + 1;
					}
					else
					{
						fr = f_read(&fsrc, buf_dst, buf_dst_size, &br);
					}
				}
				else
					fr = f_read(&fsrc, pbuf, size, &br);
				if (fr)
				{
					print_msg_queue("Read data is fail\n");
					goto err1;
				}

				// audio codec decode
				if (para->codec)
				{
					retval = cmd_codec_decode(para, buf_dst, pbuf, buf_dst_size, (int32_t*)&size);
					if (retval != AUD_CODEC_SUCCESS)
					{
						if (AUD_CODEC_SUCCESS == retval && size>0){
							//print_msg_queue("aac codec done\n");
						}else{
							if ( AUD_CODEC_ERR_MOREDATA == retval){
								print_msg_queue("aac decoding needs more data %d \n",retval);
								goto readmore;
							}
						}
						print_msg_queue("decode(%d) is fail\n",cur_blk);
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
						print_msg_queue("underrun\n");
						audio_resume(audio_stream);
					}
					else
					{
						print_msg_queue("Error playback status\n");
						goto err3;
					}
				}
			}
		}
	}


err3:
	audio_drain(audio_stream);
err2:
	audio_close(audio_stream);
	if (para->codec)
		cmd_codec_close(para);
err1:
	audio_params_free(params);
	if (type == AUD_CAP_STREAM)
		f_close(&fdst);
	else
		f_close(&fsrc);
out:
	vPortFree(buf);
	if(para->codec)
		vPortFree(buf_dst);
	print_msg_queue("audio codec task complete!!!\n");
	vTaskDelete(NULL);
}

int cmd_audio_play(int argc, char* argv[])
{
	audio_codec_para.type = AUD_PLY_STREAM;
	audio_codec_para.codec = 0;
	audio_codec_para.format = AUD_FORMAT_S16_LE;
	audio_codec_para.format_size = 2;
	audio_codec_para.sec = 10;
	audio_codec_para.rate = 16000;
	audio_codec_para.bitrate = audio_codec_para.rate;
	audio_codec_para.filename = "audio.pcm";

	if (pdPASS != xTaskCreate(vAudioCodec_cmd_Task, "audio play test", STACK_SIZE_8K, (void*) &audio_codec_para, 2, NULL))
	{
		print_msg_queue("Could not create task of audio play\n");
	}
	return 0;
}

int cmd_audio_record(int argc, char* argv[])
{
	audio_codec_para.type = AUD_CAP_STREAM;
	audio_codec_para.codec = 0;
	audio_codec_para.format = AUD_FORMAT_S16_LE;
	audio_codec_para.format_size = 2;
	audio_codec_para.sec = 10;
	audio_codec_para.rate = 16000;
	audio_codec_para.bitrate = audio_codec_para.rate;
	audio_codec_para.filename = "audio.pcm";

	if (pdPASS != xTaskCreate(vAudioCodec_cmd_Task, "audio record test", STACK_SIZE_8K, (void*) &audio_codec_para, 2, NULL))
	{
		print_msg_queue("Could not create task of audio record\n");
	}

	return 0;
}

int cmd_audio_codec(int argc, char* argv[])
{
	// call help for user
	if (argc == 2)
	{
		if(strcmp(argv[1], "--help") == 0)
		{
			cmd_codec_help();
			return 0;
		}
	}

	if(argc != 6)
	{
		print_msg_queue("parameter number error -- %d\n", argc);
		print_msg_queue("please key the command of 'acodec --help'\n");
		return -1;
	}

	if(strcmp(argv[1], "cap") == 0)
	{
		audio_codec_para.type = AUD_CAP_STREAM;
		print_msg_queue("Capture\n");
	}
	else if(strcmp(argv[1], "ply") == 0)
	{
		audio_codec_para.type = AUD_PLY_STREAM;
		print_msg_queue("Playback\n");
	}
	else
	{
		print_msg_queue("type parameter(%s) error.\n", argv[1]);
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
		audio_codec_para.g726_para.pcm_bytes_per_frame = 512;
		audio_codec_para.g726_para.compress_ratio = 2;
		audio_codec_para.g726_para.bytes_per_frame = 64;
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
		audio_codec_para.codec = 0;
		audio_codec_para.format = AUD_FORMAT_S8;
		audio_codec_para.format_size = 1;
	}
	else if(strcmp(argv[2], "U8") == 0)
	{
		audio_codec_para.codec = 0;
		audio_codec_para.format = AUD_FORMAT_U8;
		audio_codec_para.format_size = 1;
	}
	else if(strcmp(argv[2], "S16_LE") == 0)
	{
		audio_codec_para.codec = 0;
		audio_codec_para.format = AUD_FORMAT_S16_LE;
		audio_codec_para.format_size = 2;
	}
	else if(strcmp(argv[2], "U16_LE") == 0)
	{
		audio_codec_para.codec = 0;
		audio_codec_para.format = AUD_FORMAT_U16_LE;
		audio_codec_para.format_size = 2;
	}
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AAC
	else if(strcmp(argv[2], "AAC") == 0)
	{
		audio_codec_para.codec = AUD_FORMAT_AAC;
		audio_codec_para.format = AUD_FORMAT_S16_LE;
		audio_codec_para.format_size = 2;
		audio_codec_para.aac_para.afterburner = 1;
		audio_codec_para.aac_para.aot = 2;
		audio_codec_para.aac_para.bits_per_sample = 16;
		audio_codec_para.aac_para.channels = 1;
		audio_codec_para.aac_para.eld_sbr = 0;
		audio_codec_para.aac_para.vbr = 0;

	}
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AMRNB
	else if(strcmp(argv[2], "AMR-NB") == 0)
	{
		audio_codec_para.codec = AUD_FORMAT_AMRNB;
		audio_codec_para.format = AUD_FORMAT_S16_LE;
		audio_codec_para.format_size = 2;
	}
#endif
	else
	{
		print_msg_queue("format parameter(%s) error.\n", argv[2]);
		return -1;
	}

	audio_codec_para.sec = simple_strtoul(argv[3], NULL, 0);
	audio_codec_para.rate = simple_strtoul(argv[4], NULL, 0);
	audio_codec_para.bitrate = audio_codec_para.rate;
	audio_codec_para.filename = argv[5];

	if (pdPASS != xTaskCreate(vAudioCodec_cmd_Task, "audio codec test", STACK_SIZE_8K *4, (void*) &audio_codec_para, 2, NULL))
	{
		print_msg_queue("Could not create task of audio codec\n");
	}
	return 0;
}

int cmd_audio_flyplay(int argc, char* argv[])
{
	print_msg_queue("fly play\n");

	return 0;
}




void vAudioTwoway_cmd_Task(void* pvParameters)
{
	audio_cmd_codec_st *para = (audio_cmd_codec_st *)pvParameters;
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
		print_msg_queue("Failed to allocate memory. --- %d\n", para->size);
		goto out;
	}
	print_msg_queue("Buffer address:0x%08x\n", buf);
	print_msg_queue("Buffer size:0x%08x\n", para->size);

	para->type = AUD_CAP_STREAM;
	retval = audio_open(&audio_stream_cap, para->type);
	if(retval < 0)
	{
		print_msg_queue("Failed to open device for capture.\n");
		goto err_cap;
	}
	print_msg_queue("audio open of capture is OK.\n");

	para->type = AUD_PLY_STREAM;
	retval = audio_open(&audio_stream_ply, para->type);
	if(retval < 0)
	{
		print_msg_queue("Failed to open device for play.\n");
		goto err_ply;
	}
	print_msg_queue("audio open of play is OK.\n");

	retval = audio_params_alloca(&params);
	if(retval < 0)
	{
		print_msg_queue("Failed to allocate memory.\n");
		goto err_ply;
	}
	print_msg_queue("Allocate parameters of capture OK\n");

	audio_params_set_rate(audio_stream_cap, params, para->rate);
	audio_params_set_format(audio_stream_cap, params, para->format);
	retval = audio_set_params(audio_stream_cap, params);
	if(retval < 0)
	{
		print_msg_queue("Failed to set parameters of capture .\n");
		goto err_ply;
	}
	print_msg_queue("Set parameters of capture OK\n");

	audio_params_set_rate(audio_stream_ply, params, para->rate);
	audio_params_set_format(audio_stream_ply, params, para->format);
	retval = audio_set_params(audio_stream_ply, params);
	if(retval < 0)
	{
		print_msg_queue("Failed to set parameters of play .\n");
		goto err_ply;
	}
	print_msg_queue("Set parameters of play OK\n");

	if (para->codec){
		buf_dst = (uint8_t *)pvPortMalloc(para->size, GFP_KERNEL, MODULE_CLI);
		if(buf_dst == NULL)
		{
			print_msg_queue("Failed to allocate memory. --- %d\n", para->size);
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

	print_msg_queue("para->size: %d, size %d\n", para->size ,size);

	if (para->filename != NULL)
		bWriteFile = true;

	if (bWriteFile){
		fr = f_open(&fdst, para->filename, FA_CREATE_ALWAYS | FA_WRITE);
		if (fr)
		{
			print_msg_queue("Open file of Record is fail\n");
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
				print_msg_queue("overrun\n");
				audio_resume(audio_stream_cap);
				continue;
			}
			else
			{
				print_msg_queue("Error capture status\n");
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
							print_msg_queue("Write data is fail\n");
							goto err_run;
						}
					}

					memset(pbuf,0,size);
					iaacdecode =  snx_aac_decode (aac_dec,buf_dst,pbuf,buf_dst_size,&size);
					if (AUD_CODEC_SUCCESS == iaacdecode && size>0){
						//print_msg_queue("aac codec done\n");
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
							print_msg_queue("Write data is fail\n");
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
							print_msg_queue("Write data is fail\n");
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
							print_msg_queue("Write data is fail\n");
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
							print_msg_queue("Write data is fail\n");
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
				print_msg_queue("overrun\n");
				audio_resume(audio_stream_ply);
				continue;
			}
			else
			{
				print_msg_queue("Error play status\n");
				goto err_run;
			}
		}
			pbuf = pbuf + size;
	}

	print_msg_queue("End of  twoway\n");

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

	print_msg_queue("close codec done\n");

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
	audio_params_free(params);
	if (buf)
		vPortFree(buf);
	if (para->codec)
		vPortFree(buf_dst);
	print_msg_queue("exit\n");

	vTaskDelete(NULL);
}

static void twoway_usage() {   
	print_msg_queue(" Usage: twoway [AUDIO_TYPE] [SEC] [SAMPLE_RATE] [BITRATE] [FILENAME-optional]\n");
	print_msg_queue(" AUDIO_TYPE: \n");
	print_msg_queue("      ");
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_ALAW
	print_msg_queue("A-LAW, ");
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_MULAW
	print_msg_queue("MU-LAW, ");
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_G726
	print_msg_queue("G726, ");
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AUD32
	print_msg_queue("AUD32, ");
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AAC
	print_msg_queue("AAC, ");
#endif
	print_msg_queue("S16_LE, U16_LE\n");
	print_msg_queue(" SEC: how many seconds\n");
	print_msg_queue(" SAMPLE_RATE: sample rate\n");
	print_msg_queue(" BITRATE: Bitrate\n");
	print_msg_queue(" FILENAME: Filename to save\n");
	print_msg_queue(" Example: \n");
	print_msg_queue("      twoway A-LAW 10 48000 48000 test.pcm \n");
} 


int cmd_audio_twoway(int argc, char* argv[])
{
	uint32_t ubuf_size = 32768;
	uint32_t ubuf_threshold = 8192;
	int32_t  iis_block = 0;
	audio_cmd_codec_st *audio_codec_para_cmd;

	if(argc != 5 && argc != 6)
	{
		print_msg_queue("parameter number error -- %d\n", argc);
		twoway_usage();
		return pdFAIL;                                                      
	}

	audio_codec_para_cmd = (struct audio_cmd_codec_st *)pvPortMalloc(sizeof(struct audio_cmd_codec_st), GFP_KERNEL, MODULE_CLI);
	if (audio_codec_para_cmd == NULL) {
		print_msg_queue("Failed to allocate memory. --- %d\n");
		return pdFAIL;
	}

	memset(audio_codec_para_cmd,'0',sizeof(struct audio_cmd_codec_st));

#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_ALAW
	if(strcmp(argv[1], "A-LAW") == 0)
	{
		audio_codec_para_cmd->codec = AUD_FORMAT_A_LAW;
		audio_codec_para_cmd->format = AUD_FORMAT_S16_LE;
		audio_codec_para_cmd->format_size = 2;
	}
	else 
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_MULAW
	if(strcmp(argv[1], "MU-LAW") == 0)
	{
		audio_codec_para_cmd->codec = AUD_FORMAT_MU_LAW;
		audio_codec_para_cmd->format = AUD_FORMAT_S16_LE;
		audio_codec_para_cmd->format_size = 2;
	}
	else 
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_G726
	if(strcmp(argv[1], "G726") == 0)
	{
		audio_codec_para_cmd->codec = AUD_FORMAT_G726;
		audio_codec_para_cmd->format = AUD_FORMAT_S16_LE;
		audio_codec_para_cmd->format_size = 2;
	}
	else 
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AUD32
	if(strcmp(argv[1], "AUD32") == 0)
	{
		audio_codec_para_cmd->codec = AUD_FORMAT_AUD32;
		audio_codec_para_cmd->format = AUD_FORMAT_S16_LE;
		audio_codec_para_cmd->format_size = 2;
	}
	else 
#endif
#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AAC
	if(strcmp(argv[1], "AAC") == 0)
	{
		audio_codec_para_cmd->codec = AUD_FORMAT_AAC;
		audio_codec_para_cmd->format = AUD_FORMAT_S16_LE;
		audio_codec_para_cmd->format_size = 2;
	}
	else 
#endif
	if(strcmp(argv[1], "S16_LE") == 0)
	{
		audio_codec_para_cmd->codec = NULL;
		audio_codec_para_cmd->format = AUD_FORMAT_S16_LE;
		audio_codec_para_cmd->format_size = 2;
	}
	else if(strcmp(argv[1], "U16_LE") == 0)
	{
		audio_codec_para_cmd->codec = NULL;
		audio_codec_para_cmd->format = AUD_FORMAT_U16_LE;
		audio_codec_para_cmd->format_size = 2;
	}
	else
	{
		print_msg_queue("format parameter(%s) error.\n", argv[2]);
		return -1;
	}

	audio_codec_para_cmd->sec = simple_strtoul(argv[2], NULL, 0);
	audio_codec_para_cmd->rate = simple_strtoul(argv[3], NULL, 0);
	audio_codec_para_cmd->buf_size = ubuf_size;
	audio_codec_para_cmd->buf_threshold = ubuf_threshold;
	audio_codec_para_cmd->is_block = iis_block;
	audio_codec_para_cmd->bitrate = simple_strtoul(argv[4], NULL, 0);
	if (argc >=5)
		audio_codec_para_cmd->filename = argv[5];
	audio_codec_para_cmd->size = audio_codec_para_cmd->sec * audio_codec_para_cmd->rate * audio_codec_para_cmd->format_size;


	if (pdPASS != xTaskCreate(vAudioTwoway_cmd_Task, "audio two way test", STACK_SIZE_8K *4, (void*) audio_codec_para_cmd, 2, NULL))
	{
		print_msg_queue("Could not create task of audio two way\n");
	}



	return 0;
}
