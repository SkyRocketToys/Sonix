#include <stdlib.h>
#include "cmd_debug.h"
#include <generated/snx_sdk_conf.h>
#include <trcUser.h>
#include <i2c/i2c.h>
#include <sensor/sensor.h>
#include <libmid_audio/audio.h>
#include <audio/audio_dri.h>
#include <audio/audio_error.h>
#include <task.h>
#include <libmid_fatfs/ff.h>
#include <string.h>
#include <nonstdlib.h>
#include <sys_clock.h>
#ifdef CONFIG_MIDDLEWARE_TONE_DETECTION
#include <libmid_td/td.h>
#endif
#include <libmid_fatfs/ff.h>
#include <libmid_cyassl/cyassl/ctaocrypt/aes.h>

#define INLENGTH        256
#define DetectTrigLen	0

struct memory_test_para
{
	uint32_t count;
	uint32_t value;
	uint8_t is_msg;
};
struct memory_test_para mem_para;

void vmemorytesttask(void *pvParameters)
{
	struct memory_test_para *para = (struct memory_test_para *)pvParameters;
	uint32_t memory_test_times = 1;
	uint32_t count = 0;
	uint32_t size[3] = {1, 2, 4};
	uint32_t word1 = 0;
	uint16_t hword1 = 0;
	uint8_t byte1 = 0;
	uint32_t word2 = 0;
	uint16_t hword2 = 0;
	uint8_t byte2 = 0;
	uint8_t bytetmp = 0;
	uint16_t hwordtmp = 0;
	uint16_t wordtmp = 0;
	uint32_t *pword = NULL;
	uint16_t *phword = NULL;
	uint8_t *pbyte= NULL;
	uint32_t i, j = 0;

	count = para->count;
	count = count * 1024; // count K Bytes
	do {
		count =para->count;
		count = count * 1024; // count K Bytes
		if (para->is_msg)
			if (!(memory_test_times % 100))
				print_msg("The times of memory test is %d\n",memory_test_times);

		if (size[j] == 1)
		{
			count = count * 4;
			pbyte = (uint8_t *)pvPortMalloc(sizeof(uint8_t)*count, GFP_KERNEL, MODULE_CLI);
			if (pbyte == NULL)
			{
				print_msg("allocate memory(byte) is fail\n");
				goto done;
			}
			byte1 = (uint8_t)para->value;
		}
		else if (size[j] == 2)
		{
			count = count * 2;
			phword = (uint16_t *)pvPortMalloc(sizeof(uint16_t)*count, GFP_KERNEL, MODULE_CLI);
			if (phword == NULL)
			{
				print_msg("allocate memory(half word) is fail\n");
				goto done;
			}
			hword1 = (uint16_t)para->value;
		}
		else if (size[j] == 4)
		{
			pword = (uint32_t *)pvPortMalloc(sizeof(uint32_t) * count, GFP_KERNEL, MODULE_CLI);
			if (pword == NULL)
			{
				print_msg("allocate memory(word) is fail\n");
				goto done;
			}
			word1 = (uint32_t)para->value;
		}

		// write data
		for (i = 0; i < count; i++)
		{
			if (size[j] == 1)
			{
				*(pbyte + i) = byte1;
			}
			else if (size[j] == 2)
			{
				*(phword + i) = hword1;
			}
			else if (size[j] == 4)
			{
				*(pword + i) = word1;
			}
		}

		// read data and compare
		for (i = 0; i < count; i++)
		{
			if (size[j] == 1)
			{
				byte2 = *(pbyte + i);
				bytetmp = byte1 - byte2;
				if (bytetmp != 0x00)
				{
					print_msg("The %dth data compared(byte) is fail\n",i);
					print_msg("data1 is %d\n",byte1);
					print_msg("data2 is %d\n",byte2);
					break;
				}
			}
			else if (size[j] == 2)
			{
				hword2 = *(phword + i);
				hwordtmp = hword1 - hword2;
				if (hwordtmp != 0x0000)
				{
					print_msg("The %dth data compared(half word) is fail\n",i);
					print_msg("data1 is %d\n",hword1);
					print_msg("data2 is %d\n",hword2);
					break;
				}
			}
			else if (size[j] == 4)
			{
				word2 = *(pword + i);
				wordtmp = word1 - word2;
				if (wordtmp != 0x00000000)
				{
					print_msg("The %dth data compared(word) is fail\n",i);
					print_msg("data1 is %d\n",word1);
					print_msg("data2 is %d\n",word2);
					break;
				}
			}
		}

		// free memory
		if (size[j] == 1)
			vPortFree(pbyte);
		else if (size[j] == 2)
			vPortFree(phword);
		else if (size[j] == 4)
			vPortFree(pword);
		j++;
		if (j >= 3)
			j = 0;
		memory_test_times ++;
	}while(1);
	print_msg("The task of memory test is closed\n");
done:
	memory_test_times ++;
	//vTaskDelete(NULL);
}

int cmd_dbg_memory(int argc, char* argv[])
{
	if(argc != 4)
	{
		print_msg("parameter number error -- %d\n", argc);
		return -1;
	}

	mem_para.count = simple_strtoul(argv[1], NULL, 0);
	mem_para.value = simple_strtoul(argv[2], NULL, 0);
	mem_para.is_msg = simple_strtoul(argv[3], NULL, 0);
#if 0
	if (pdPASS != xTaskCreate(vmemorytesttask, "memory test task", STACK_SIZE_512, (void*) &mem_para, 2, NULL))
	{
		print_msg("Could not create task of memory test\n");
	}
#else
	vmemorytesttask(&mem_para);
#endif
	return 0;
}

int cmd_dbg_cmd01(int argc, char* argv[])
{
	return 0;
}

int cmd_dbg_cmd02(int argc, char* argv[])
{
	return 0;
}

#ifdef CONFIG_SYSTEM_TRACE_SELECT
int cmd_dbg_trace(int argc, char* argv[])
{
	FIL MyFile;
	int wbytes = 0;

	int trace_start = 1;
	if(argc > 1){
		trace_start = simple_strtoul(argv[1], NULL, 10);
	}

	print_msg_queue ("trace argv[1] = %d, %s\n", trace_start, argv[1]);
	if (trace_start) {
		vTraceClear ();
		if (! uiTraceStart()) {
			print_msg_queue("Error: trace start failed!!! \n");
		} else {
			print_msg_queue("Trace restart! \n");
			print_msg_queue("Trace recorder addr = 0x%x, recorder buf size = 0x%x\n", (unsigned int)vTraceGetTraceBuffer(), (unsigned int)uiTraceGetTraceBufferSize());
		}
	} else {
		vTraceStop();

		if (f_open(&MyFile, "trace.log", FA_CREATE_ALWAYS | FA_WRITE) == FR_OK) {
			print_msg_queue("trace.log open ok\n");
		} else {
			print_msg_queue("trace.log open fail\n");
			goto out;
		}

		if (f_write(&MyFile, (unsigned int)vTraceGetTraceBuffer(), (unsigned int)uiTraceGetTraceBufferSize(), (void *)&wbytes) != FR_OK) {
			print_msg_queue("trace.log to sd error!!!, cnt = %d\n", wbytes);
		}

		f_close(&MyFile);
		print_msg_queue("Trace stop! \n");
	}


out:
	return 0;
}
#else
int cmd_dbg_trace(int argc, char* argv[])
{
	
	print_msg_queue("Not support Trace !!! \n");

	return 0;
}
#endif
#if 0
static struct i2c_board_info snx_i2c0_devs[] = {
	{
		I2C_BOARD_INFO("ov9715", 0x60),
		.__init = ov9715_module_init,
		.__cleanup = ov9715_module_exit,
	},
};
#endif
int cmd_dbg_i2c(int argc, char* argv[])
{
#if 0
	int ret;
	struct i2c_client *client;
	struct i2c_adapter *adap;
	struct i2c_board_info *bdi = &snx_i2c0_devs[0];

	{//setup clock divisor & enable mclk
		unsigned int v, mclk, div, base;

		mclk = 24000000;
		base = 0x90600000;
	
		v = inl(base);

		if (v & (0x1<<31))
			div = 120000000 / mclk;
		else
			div = 96000000 / mclk;

		v &= ~(0xff<<8|0x1);
		v |= (div<<8|0x1);
		outl(base, v);
	}

	vTaskDelay( 50/portTICK_PERIOD_MS ); //50ms

	if(!(adap = i2c_get_adapter(0))){
		print_msg_queue("Cannot get I2C adapter #%d. No driver?\n", 0);
		return pdFAIL;
	}

	if(!(client = i2c_new_device(adap, bdi))){
		return pdFAIL;
	}

	if((ret = bdi->__init(client, 24000000)) == pdFAIL)
		goto out;	


	bdi->__cleanup(client);

out:
	i2c_release_device(client);
#endif

	return pdPASS;
}

int cmd_dbg_audio(int argc, char* argv[])
{
	int32_t retval = 0;
	int32_t type, format, format_size, status, is_block;
	uint32_t rate, buf_size, buf_threshold;
	struct audio_stream_st *audio_stream;
	struct params_st *params;
	uint8_t *buf, *pbuf;
	uint32_t size;
//	int32_t i;

	print_msg("\naudio test task run.\n");

	if(argc != 7)
	{
		print_msg("parameter number error -- %d\n", argc);
		goto cmd_err;
	}

	if(strcmp(argv[1], "cap") == 0)
	{
		type = AUD_CAP_STREAM;
		print_msg("Capture\n");
	}
	else if(strcmp(argv[1], "ply") == 0)
	{
		type = AUD_PLY_STREAM;
		print_msg("Playback\n");
	}
	else
	{
		print_msg("type parameter(%s) error.\n", argv[1]);
		goto cmd_err;
	}

	if(strcmp(argv[2], "S8") == 0)
	{
		format = AUD_FORMAT_S8;
		format_size = 1;
	}
	else if(strcmp(argv[2], "U8") == 0)
	{
		format = AUD_FORMAT_U8;
		format_size = 1;
	}
	else if(strcmp(argv[2], "S16_LE") == 0)
	{
		format = AUD_FORMAT_S16_LE;
		format_size = 2;
	}
	else if(strcmp(argv[2], "U16_LE") == 0)
	{
		format = AUD_FORMAT_U16_LE;
		format_size = 2;
	}
	else
	{
		print_msg("format parameter(%s) error.\n", argv[2]);
		goto cmd_err;
	}

	rate = simple_strtoul(argv[3], NULL, 0);
	buf_size = simple_strtoul(argv[4], NULL, 0);
	buf_threshold = simple_strtoul(argv[5], NULL, 0);
	is_block = simple_strtoul(argv[6], NULL, 0);


	size = 20 * rate * format_size;

	buf = (uint8_t *)pvPortMalloc(size, GFP_KERNEL, MODULE_CLI);
	if(buf == NULL)
	{
		print_msg("Failed to allocate memory. --- %d\n", size);
		retval = -2;
		goto out;
	}
	print_msg("Buffer address:0x%08x\n", buf);
	print_msg("Buffer size:0x%08x\n", size);
	if(type == AUD_PLY_STREAM)
		vTaskDelay(1000 / portTICK_RATE_MS);

	retval = audio_open(&audio_stream, type);
	if(retval < 0)
	{
		print_msg("Failed to open device.\n");
		retval = -3;
		goto err1;
	}
	print_msg("audio open ok.\n");

	retval = audio_params_alloca(&params);
	if(retval < 0)
	{
		print_msg("Failed to allocate memory.\n");
		retval = -4;
		goto err2;
	}	

	print_msg("Allcate paramters ok\n");
	audio_params_set_rate(audio_stream, params, rate);
	audio_params_set_format(audio_stream, params, format);
	retval = audio_set_params(audio_stream, params);
	if(retval < 0)
	{
		print_msg("Failed to set paramters.\n");
		retval = -5;
		goto err2;
	}	
	print_msg("Set paramters ok\n");

	pbuf = buf;
	while(size > 0)
	{
		if(type == AUD_CAP_STREAM)
			retval = audio_read(audio_stream, pbuf, size, is_block);
		else
			retval = audio_write(audio_stream, pbuf, size, is_block);

		if(retval > 0)
		{
			print_msg("Process:%d Bytes\n", retval);
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

			audio_status(audio_stream, &status);
			if(status == AUD_STATE_XRUN)
			{
				print_msg("overrun\n");
				audio_resume(audio_stream);
			}
			else
			{
				print_msg("Error capture status\n");
				retval = -6;
				goto err3;
			}
		}
	}

err3:
	audio_drain(audio_stream);
err2:
	audio_close(audio_stream);
err1:
	vPortFree(buf);
out:
	return retval;

cmd_err:
	print_msg("Command error.\n");
	return -1;
}
