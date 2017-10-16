/** 
* @file 
* this is usb device driver with middleware
* snx_xu_mpt.c 
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stddef.h>
#include <FreeRTOS.h>
#include <task.h>
#include <bsp.h>

#include <libmid_isp/snx_mid_isp.h>
#include <libmid_nvram/snx_mid_nvram.h>

#include <libmid_audio/audio.h>
#include <audio/audio_error.h>
#include <libmid_usbd/mid_usbd.h>
#include <usb_device/usbd_uvc.h>
#include "usbd_uvc_vc.h"
#include "snx_xu_mpt.h"
#include <libmid_fatfs/ff.h>
#include <gpio/gpio.h>
#include <wifi/wifi_api.h>
#include <generated/snx_sdk_conf.h>
#include "lwip/netif.h"
#include "lwip/dhcp.h"


/** \defgroup func_usbd USB device function
 *  \ingroup mid_usbd
 * @{
 */

//#define DEBUG_PRINT
#ifdef  DEBUG_PRINT
#define debug_print(format, arg...) print_msg("[xu mpt]"format, ##arg)
#else 
#define debug_print(format, ...)
#endif

#define INET6_ADDRSTRLEN 46

#define NET_IP4_MODE		0
#define NET_IP6_MODE		1
#define NET_DHCP_MODE 		0
#define NET_STATIC_MODE 	1

#define AUDIO_REC_COMPLETED			0x00
#define AUDIO_REC_FAILED			0x01
#define AUDIO_REC_NOT_YET_COMPLETE	0xFF
#define AUDIO_PB_NO_ERR			0x00
#define AUDIO_PB_OPEN_AUDIO_DEVICE_FAIL	0x01
#define AUDIO_PB_AUDIO_FILE_NOT_FOUND	0x02
#define AUDIO_PB_ERROR_UNKNOWN		0xFF

#define AUDIO_TEST_FILE				"audio.pcm"
#define AUDIO_BUFF_SIZE				160

#define SD_TEST_COMPLETED			0x00
#define SD_TEST_NOT_YET_COMPLETE	0x01
#define SD_TEST_FAILED				0x02
#define SD_NO_DETECT_SD_CARD		0xFF
#define SD_TEST_FILE				"sdTest.txt"

#define GPIO_FUNC_GPIO	0
#define GPIO_FUNC_PWM	1
#define GPIO_FUNC_MS1	2
#define GPIO_FUNC_AUDIO	3
#define GPIO_FUNC_I2C	4
#define GPIO_FUNC_SPI	5
#define GPIO_FUNC_ERR	(-1)

gpio_pin_info ircut_p;
#define IR_DAY_MODE			1
#define IR_NIGHT_MODE		0

static struct mpt_status_t mpt_status = {0};
static uint8_t mpt_af_report_osd_position = 0;


static int mpt_message_send(mpt_module_manager * module, int cmd, char* data, int len);

/**
* @brief interface function - MP tool function : get version
* @param data the point of buffer 
* @param len the length of data 
* @return value of error code.
*/
static int mpt_get_version (char* data, int len)
{
	data[1] = 1;
	data[2] = 0;
	debug_print ("%s:%d \n", __FUNCTION__, __LINE__);
	return 0;
}

static void set_osd_enable(int enable);
void vTaskUsbdUvcXuMptOSD(void);

static int mpt_osd_af_report (char* data, int len)
{
	int isp_osd_enable;
	
//	debug_print ("%s:%d \n", __FUNCTION__, __LINE__);
	isp_osd_enable=data[1];
	mpt_af_report_osd_position = data[2];

	mpt_osd_af_report_set_position();

	if(isp_osd_enable){
		if (mpt_status.af2osd_task == NULL) {
			if (pdPASS != xTaskCreate(vTaskUsbdUvcXuMptOSD, "mpt_af2osd", STACK_SIZE_2K, 0, PRIORITY_TASK_DRV_USBD_PROCESS, &mpt_status.af2osd_task)) {
				print_msg("Create AF report to OSD task failed\n");
				return -1;
			}
		}
	}
	else{
		set_osd_enable(ISP_DISABLE);
		if(mpt_status.af2osd_task){
			vTaskDelete(mpt_status.af2osd_task);
			mpt_status.af2osd_task = NULL;
		}
	}
	
	return 0;
}

/**
* @brief interface function - MP tool function : start record
* @param rec_time the time of recode 
* @return value of error code.
*/
static int mpt_record_start (unsigned char rec_time)
{
	int ret;

	FIL file;
	int32_t retval = 0;
	int32_t type, format, format_size, status, is_block;
	uint32_t rate, buf_size, buf_threshold, rw_size, rec_size, read_size;
	struct audio_stream_st *audio_stream;
	struct params_st *params;
	uint8_t *pbuf;
	uint32_t block_count;
	uint32_t cur_blk = 0;
	
	type = AUD_CAP_STREAM;
	format = AUD_FORMAT_S16_LE;
	format_size = 2;
	rate = 16000;
	buf_size = 32768;
	buf_threshold = 8192;
	is_block = 0;
	rec_size = rec_time * rate * format_size;
	
	block_count = rec_size / AUDIO_BUFF_SIZE;
	debug_print ("%s:%d time %ds, size %d\n", __FUNCTION__, __LINE__, rec_time, rec_size);

	mpt_status.audio_rec_status = AUDIO_REC_NOT_YET_COMPLETE;
	pbuf = (uint8_t *)pvPortMalloc(AUDIO_BUFF_SIZE, GFP_KERNEL, MODULE_APP);
	if(pbuf == NULL)
	{
		print_msg("[mpt] Failed to allocate memory. %d\n");
		retval = -2;
		goto out;
	}

	if((ret=f_open(&file, AUDIO_TEST_FILE,  FA_WRITE|FA_CREATE_ALWAYS)) != FR_OK)
	{
		print_msg("[mpt] file creat fail\n");
		goto err1;
	}
	
	retval = audio_open(&audio_stream, type);
	if(retval < 0)
	{
		print_msg("[mpt] Failed to open device.\n");
		retval = -3;
		goto err2;
	}
	print_msg("[mpt] audio open ok.\n");

	retval = audio_params_alloca(&params);
	if(retval < 0)
	{
		print_msg("[mpt] Failed to allocate memory.\n");
		retval = -4;
		goto err2;
	}	

	print_msg("[mpt] Allcate paramters ok\n");
	audio_params_set_rate(audio_stream, params, rate);
	audio_params_set_format(audio_stream, params, format);
	retval = audio_set_params(audio_stream, params);
	if(retval < 0)
	{
		print_msg("[mpt] Failed to set paramters.\n");
		retval = -5;
		goto err3;
	}	
	print_msg("[mpt] Set paramters ok\n");

	read_size = AUDIO_BUFF_SIZE;
	for (cur_blk = 0; cur_blk < block_count; cur_blk ++)
	{
	reread:
		retval = audio_read(audio_stream, pbuf, read_size, is_block);

		if(retval > 0)
		{
			if((ret = f_write(&file, pbuf, retval,&rw_size))!=FR_OK || (retval!=rw_size))
			{
				print_msg("[mpt] write data fail\n");
				goto err4;		
			}
		}
		else
		{
			if(retval == -EAGAIN)
			{
				vTaskDelay(10 / portTICK_RATE_MS);
				goto reread;
			}

			audio_status(audio_stream, &status);
			if(status == AUD_STATE_XRUN)
			{
				print_msg("[mpt] overrun\n");
				audio_resume(audio_stream);
			}
			else
			{
				print_msg("[mpt] Error capture status\n");
				retval = -6;
				goto err4;
			}
		}
	}

err4:
	audio_drain(audio_stream);
err3:
	audio_close(audio_stream);
err2:
	f_close(&file);
err1:
	vPortFree(pbuf);
out:	
	audio_params_free(params);
	if( retval < 0)
		mpt_status.audio_rec_status = AUDIO_REC_FAILED;
	else
		mpt_status.audio_rec_status = AUDIO_REC_COMPLETED;
	print_msg("[mpt] Audio record completed (ret %d).\n", retval);
	
	return retval;
}
/**
* @brief interface function - MP tool function : record result
* @param data the point of buffer 
* @param len the length of data 
* @return value of error code.
*/
static int mpt_record_result (char* data, int len)
{
	debug_print ("%s:%d \n", __FUNCTION__, __LINE__);

	data[1] = mpt_status.audio_rec_status;
	
	return 0;
}
/**
* @brief interface function - MP tool function : start playback
* @return value of error code.
*/
static int mpt_playback_start (char *data)
{
	int ret;

	FIL file;
	int32_t retval = 0;
	int32_t type, format, format_size, status, is_block;
	uint32_t rate, buf_size, buf_threshold, rw_size, pb_size, read_size;
	struct audio_stream_st *audio_stream;
	struct params_st *params = NULL;
	uint32_t block_count;
	uint32_t cur_blk = 0;
	uint8_t *pbuf;
	char audio_file[64] = {0};
	read_size = AUDIO_BUFF_SIZE;
	
	type = AUD_PLY_STREAM;
	format = AUD_FORMAT_S16_LE;
	format_size = 2;
	rate = 16000;
	buf_size = 32768;
	buf_threshold = 8192;
	is_block = 0;
	
	debug_print ("%s:%d[]\n", __FUNCTION__, __LINE__);
	
	mpt_status.audio_pb_status = AUDIO_PB_NO_ERR;
	if(data[2] == 0){
		memcpy(audio_file, AUDIO_TEST_FILE, sizeof(AUDIO_TEST_FILE));
	}
	else if(data[2] == 1){
		memcpy(audio_file, &data[4], data[3]);
	}
	else{
		retval = -1;
		goto out;
	}
	
	pbuf = (uint8_t *)pvPortMalloc(AUDIO_BUFF_SIZE, GFP_KERNEL, MODULE_APP);
	if(pbuf == NULL)
	{
		print_msg("[mpt] Failed to allocate memory.\n");
		retval = -2;
		goto out;
	}
	print_msg("[mpt] Buffer address:0x%08x\n", pbuf);

	if((ret=f_open(&file, audio_file, FA_OPEN_EXISTING | FA_READ)) != FR_OK)
	{
		print_msg("[mpt] %s file open failed\n", audio_file);		
		mpt_status.audio_pb_status = AUDIO_PB_AUDIO_FILE_NOT_FOUND;
		goto err1;
	}
	pb_size = f_size(&file);
	vTaskDelay(1000 / portTICK_RATE_MS);
	
	retval = audio_open(&audio_stream, type);
	if(retval < 0)
	{
		print_msg("[mpt] Failed to open device.\n");
		retval = -3;
		mpt_status.audio_pb_status = AUDIO_PB_OPEN_AUDIO_DEVICE_FAIL;
		goto err2;
	}
	print_msg("[mpt] audio open ok. file size %d\n", pb_size);
	block_count = pb_size / AUDIO_BUFF_SIZE;

	retval = audio_params_alloca(&params);
	if(retval < 0)
	{
		print_msg("[mpt] Failed to allocate memory.\n");
		retval = -4;
		goto err2;
	}	

	print_msg("[mpt] Allcate paramters ok\n");
	audio_params_set_rate(audio_stream, params, rate);
	audio_params_set_format(audio_stream, params, format);
	retval = audio_set_params(audio_stream, params);
	if(retval < 0)
	{
		print_msg("[mpt] Failed to set paramters.\n");
		retval = -5;
		goto err3;
	}	
	print_msg("[mpt] Set paramters ok\n");

	for (cur_blk = 0; cur_blk < block_count; cur_blk ++)
	{
		if((ret = f_read(&file, pbuf, read_size,&rw_size))!=FR_OK)
		{
			print_msg("[mpt] read data fail, size %d, read %d\n", read_size, rw_size);
			goto err4;		
		}
		
	rewrite:
		retval = audio_write(audio_stream, pbuf, read_size, is_block);
		if(retval <= 0)
		{
			if(retval == -EAGAIN)
			{
				vTaskDelay(50 / portTICK_RATE_MS);
				goto rewrite;
			}

			audio_status(audio_stream, &status);
			if(status == AUD_STATE_XRUN)
			{
				print_msg("[mpt] overrun\n");
				audio_resume(audio_stream);
			}
			else
			{
				print_msg("[mpt] Error capture status\n");
				retval = -6;
				goto err4;
			}
		}
	}

err4:
	audio_drain(audio_stream);
err3:
	audio_close(audio_stream);
err2:
	f_close(&file);
err1:
	vPortFree(pbuf);
out:
	if(params)	audio_params_free(params);
	if( retval < 0)
		if(mpt_status.audio_pb_status == AUDIO_PB_NO_ERR)
			mpt_status.audio_pb_status = AUDIO_PB_ERROR_UNKNOWN;
	print_msg("[mpt] Audio playback completed (ret %d).\n", retval);
	
	return retval;
}

static int mpt_playback_result(char* data, int len)
{
	debug_print ("%s:%d \n", __FUNCTION__, __LINE__);

	data[1] = mpt_status.audio_pb_status;
	
	return 0;
}

/**
* @brief interface function - MP tool function : detect SD card Insert state
* @param data the point of buffer 
* @param len the length of data 
* @return value of error code.
*/
static int mpt_sd_insert_state (char* data, int len)
{
	int ret;
	
	debug_print ("%s:%d \n", __FUNCTION__, __LINE__);
	
	ret = fs_cmd_mount(0);/*mount sd card*/
	if(ret != FR_OK){
		print_msg("!!!!!!!!sd mount fail");
		data[1] = 0;
	}
	else{
		data[1] = 1;
	}
	
	return ret;
}
/**
* @brief interface function - MP tool function : generate random number 
* @param output output buffer
* @param sz the size of data 
* @return value of error code.
*/
int mpt_rand_generate(unsigned char * output, unsigned int sz)
{
	unsigned int i;
	
	srand(xTaskGetTickCount());
	for (i=0 ; i< sz ; i++)
	{
		output[i] = rand()%256;
	} 

	return 0;
}
/**
* @brief interface function - MP tool function : SD crad read/write test 
* @param data output buffer
* @param len the length of data 
* @return value of error code.
*/
static int mpt_sd_rw_test (char* data, int len)
{
	FIL file;
	int ret, loop_time, i;
	unsigned int file_size,rw_size;
	unsigned char * rand_buf;
	unsigned char * compare_buf;
	unsigned int rand_size = 1 << 10 << 10;/*1M*/
	unsigned int compare_buf_size = 512 << 10;/*512k*/
	struct timeval 	tv_start,tv_end;
	unsigned int r_speed = 0, w_speed = 0;
	unsigned int r_time=0,w_time=0;/**/
	unsigned int write_size, read_size, remainder_size, remainder_rand_size;
	
	file_size = data[2] | data[3] << 8 | data[4] << 16 | data[5] << 24;
	loop_time = data[6];
	debug_print ("%s:%d, size %d, loop_time %d\n", __FUNCTION__, __LINE__, file_size, loop_time);

	mpt_status.sd_rw_status = SD_TEST_NOT_YET_COMPLETE;
	rand_buf = (unsigned char *)pvPortMalloc(rand_size, GFP_KERNEL, MODULE_APP);
	if(rand_buf == NULL){
		print_msg("%s:%d malloc buffer failed!\r\n", __FUNCTION__, __LINE__);
		goto fail_exit;
	}
	compare_buf = (unsigned char *)pvPortMalloc(compare_buf_size, GFP_KERNEL, MODULE_APP);
	if(compare_buf == NULL){
		print_msg("%s:%d malloc buffer failed!\r\n", __FUNCTION__, __LINE__);
		goto fail_exit2;
	}

	for(i = 0; i < loop_time; i++){	
		mpt_rand_generate(rand_buf, rand_size);
		/*****        write           *****/
		if((ret=f_open(&file, SD_TEST_FILE,  FA_WRITE|FA_CREATE_ALWAYS)) != FR_OK)
		{
			print_msg("%s:%d file sdTest.txt creat fail(ret = %d, loop time = %d)\n",
				__FUNCTION__, __LINE__, ret, i+1);
			goto fail_exit3;
		}
		
		remainder_size = file_size;
		w_time = 0;
		while(remainder_size>0){
			write_size = (remainder_size > rand_size) ? rand_size : remainder_size;
			remainder_size = remainder_size - write_size;
			gettimeofday(&tv_start, NULL);			
			if((ret = f_write(&file, rand_buf, write_size,&rw_size))!=FR_OK || (write_size!=rw_size))
			{
				print_msg("%s:%d write data fail(ret = %d, loop time = %d), %x, %x\n",
					__FUNCTION__, __LINE__, ret, i+1, write_size,rw_size);
				f_close(&file);
				goto fail_exit3;		
			}
			gettimeofday(&tv_end, NULL);
			w_time += (tv_end.tv_sec - tv_start.tv_sec) * 1000000 + tv_end.tv_usec - tv_start.tv_usec;
		}
		
		w_speed += (file_size >> 10) * 1000 / (w_time / 1000);
		f_close(&file);

		FILINFO fno;
		if(f_stat(SD_TEST_FILE,&fno) != FR_OK)
			goto fail_exit3;
		if(fno.fsize != file_size){
			print_msg("%s:%d write size error(ret = %d, loop time = %d)\n",
				__FUNCTION__, __LINE__, ret, i+1);
			goto fail_exit3;
		}			

		/*****       read           *****/
		if((ret=f_open(&file, SD_TEST_FILE ,  FA_READ))!=FR_OK)
		{
			print_msg("%s:%d file sdTest.txt open fail(ret = %d, loop time = %d)\n",
				__FUNCTION__, __LINE__, ret, i+1);
			goto fail_exit3;
		}

		unsigned int rand_offset = 0, 
		remainder_size = file_size;
		r_time = 0;
		while(remainder_size > 0){
			read_size = (remainder_size > compare_buf_size) ? compare_buf_size : remainder_size;
			remainder_rand_size = rand_size - rand_offset;
			if(read_size >= remainder_rand_size)
				read_size = remainder_rand_size;
			remainder_size = remainder_size - read_size;
			
			gettimeofday(&tv_start, NULL);			
			if((ret = f_read(&file, compare_buf, read_size,&rw_size))!=FR_OK || (read_size!=rw_size))
			{
				print_msg("%s:%d read data fail(ret = %d, loop time = %d), %x, %x\n",
					__FUNCTION__, __LINE__, ret, i+1, read_size,rw_size);
				f_close(&file);
				goto fail_exit3;		
			}
			gettimeofday(&tv_end, NULL);			
			r_time += (tv_end.tv_sec - tv_start.tv_sec) * 1000000 + tv_end.tv_usec - tv_start.tv_usec;
			
			/*****		 compare		*****/
			ret = memcmp(rand_buf + rand_offset, compare_buf, read_size);
			if(ret != 0){
				print_msg("%s:%d compare failed(ret = %d, loop time = %d)\n",
					__FUNCTION__, __LINE__, ret, i+1);
				goto fail_exit3;		
			
			}
			rand_offset += read_size;
			if(read_size == remainder_rand_size)
				rand_offset = 0;
		}
		r_speed += (file_size >> 10) * 1000 / (r_time / 1000);
		f_close(&file);

		print_msg("The %d time test success.\n", i+1);
	}
	mpt_status.sd_r_speed = r_speed / i;/*KBytes*/
	mpt_status.sd_w_speed = w_speed / i;/*KBytes*/
	print_msg("speed read %dkBps, w %dkBps\n", mpt_status.sd_r_speed, mpt_status.sd_w_speed);

	mpt_status.sd_rw_status = SD_TEST_COMPLETED;
	vPortFree(rand_buf);
	vPortFree(compare_buf);
	return 0;
	
fail_exit3:
	vPortFree(compare_buf);
fail_exit2:
	vPortFree(rand_buf);
fail_exit:
	mpt_status.sd_rw_status = SD_TEST_FAILED;
	return -1;
}
/**
* @brief interface function - MP tool function : SD crad read/write test result
* @param data output buffer
* @param len the length of data 
* @return value of error code.
*/
static int mpt_sd_rw_result (char* data, int len)
{
	int ret = 0;
	debug_print ("%s:%d \n", __FUNCTION__, __LINE__);

	/*no sd*/
	if (0) {
		data[1] = 0xff;
		return ret;
	}
	
	data[1] = mpt_status.sd_rw_status;
	data[2] = mpt_status.sd_r_speed & 0xff;
	data[3] = (mpt_status.sd_r_speed >> 8) & 0xff;
	data[4] = (mpt_status.sd_r_speed >> 16) & 0xff;
	data[5] = (mpt_status.sd_r_speed >> 24) & 0xff;
	data[6] = mpt_status.sd_w_speed & 0xff;
	data[7] = (mpt_status.sd_w_speed >> 8) & 0xff;
	data[8] = (mpt_status.sd_w_speed >> 16) & 0xff;
	data[9] = (mpt_status.sd_w_speed >> 24) & 0xff;

	return ret;
}
/**
* @brief interface function - MP tool function : Get GPIO information
* @param number the number of GPIO
* @param pinumber the number of GPIO pin 
* @return value of error code.
*/
static int mpt_gpio_get_info(unsigned int number, unsigned int* pinumber)
{
	int func = GPIO_FUNC_ERR;
	
	switch(number){
		/*       gpio         */
		case MPT_GPIO_GPIO0:
			*pinumber = GPIO_PIN_0;
			func = GPIO_FUNC_GPIO;
			break;
		case MPT_GPIO_GPIO1:
			*pinumber = GPIO_PIN_1;
			func = GPIO_FUNC_GPIO;
			break;
		case MPT_GPIO_GPIO2:
			*pinumber = GPIO_PIN_2;
			func = GPIO_FUNC_GPIO;
			break;
		case MPT_GPIO_GPIO3:
			*pinumber = GPIO_PIN_3;
			func = GPIO_FUNC_GPIO;
			break;
		case MPT_GPIO_GPIO4:
			*pinumber = GPIO_PIN_4;
			func = GPIO_FUNC_GPIO;
			break;
		case MPT_GPIO_GPIO6:
			*pinumber = GPIO_PIN_6;
			func = GPIO_FUNC_GPIO;
			break;
		/*       pwm        */
		case MPT_GPIO_PWM1:
			*pinumber = PWM_GPIO_PIN0;
			func = GPIO_FUNC_PWM;
			break;
		case MPT_GPIO_PWM2:
			*pinumber = PWM_GPIO_PIN1;
			func = GPIO_FUNC_PWM;
			break;
		/*       ms1        */
		case MPT_GPIO_MS1_IO0:
			*pinumber = 0;/*have no define MS1_GPIO_PIN0*/
			func = GPIO_FUNC_MS1;
			break;
		case MPT_GPIO_MS1_IO1:
			*pinumber = MS1_GPIO_PIN1;
			func = GPIO_FUNC_MS1;
			break;
		case MPT_GPIO_MS1_IO2:
			*pinumber = MS1_GPIO_PIN2;
			func = GPIO_FUNC_MS1;
			break;			
		case MPT_GPIO_MS1_IO3:
			*pinumber = MS1_GPIO_PIN3;
			func = GPIO_FUNC_MS1;
			break;
		case MPT_GPIO_MS1_IO4:
			*pinumber = MS1_GPIO_PIN4;
			func = GPIO_FUNC_MS1;
			break;
		case MPT_GPIO_MS1_IO5:
			*pinumber = MS1_GPIO_PIN5;
			func = GPIO_FUNC_MS1;
			break;
		/*		 audio		*/
		case MPT_GPIO_AUD_IO0:
			*pinumber = AUD_GPIO_PIN0;
			func = GPIO_FUNC_AUDIO;
			break;
		case MPT_GPIO_AUD_IO1:
			*pinumber = AUD_GPIO_PIN1;
			func = GPIO_FUNC_AUDIO;
			break;
		case MPT_GPIO_AUD_IO2:
			*pinumber = AUD_GPIO_PIN2;
			func = GPIO_FUNC_AUDIO;
			break;
		case MPT_GPIO_AUD_IO3:
			*pinumber = AUD_GPIO_PIN3;
			func = GPIO_FUNC_AUDIO;
			break;
		case MPT_GPIO_AUD_IO4:			
			*pinumber = AUD_GPIO_PIN4;
			func = GPIO_FUNC_AUDIO;
			break;
		/*       i2c       */
		case MPT_GPIO_SCL2:
		case MPT_GPIO_SDA2:
			func = GPIO_FUNC_I2C;
			break;
		/*       spi       */
		case MPT_GPIO_SSP1_CLK:
			*pinumber = SPI_GPIO_CLK_PIN;
			func = GPIO_FUNC_SPI;
			break;
		case MPT_GPIO_SSP1_TX:
			*pinumber = SPI_GPIO_TX_PIN;
			func = GPIO_FUNC_SPI;
			break;
		case MPT_GPIO_SSP1_FS:
			*pinumber = SPI_GPIO_FS_PIN;
			func = GPIO_FUNC_SPI;
			break;
		case MPT_GPIO_SSP1_RX:
			*pinumber = SPI_GPIO_RX_PIN;
			func = GPIO_FUNC_SPI;
			break;
		default:
			func = GPIO_FUNC_ERR;
			break;							
	}
	debug_print ("%s:%d num %d, func %d\n", __FUNCTION__, __LINE__, *pinumber, func);	
	return func;
}
/**
* @brief interface function - MP tool function : set GPIO out
* @param data the buffer of data
* @param len data length 
* @return value of error code.
*/
static int mpt_gpio_out (char* data, int len)
{
	int ret = 0;
	gpio_pin_info info;
	unsigned int number;
	int func;
	debug_print ("%s:%d num %d, %d\n", __FUNCTION__, __LINE__, data[1], data[2]);
	
	number = data[1];
	info.mode = 1;
	info.value = data[2];

	func = mpt_gpio_get_info( number, &info.pinumber );
	switch(func){
		case GPIO_FUNC_GPIO:
			snx_gpio_open();
			snx_gpio_write(info);
			snx_gpio_close();
			break;
		case GPIO_FUNC_PWM:
			snx_pwm_gpio_open();
			snx_pwm_gpio_write(info);
			snx_pwm_gpio_close();
			break;
		case GPIO_FUNC_MS1:
			snx_ms1_gpio_open();
			snx_ms1_gpio_write(info);
			snx_ms1_gpio_close();
			break;
		case GPIO_FUNC_AUDIO:
			snx_audio_gpio_open();
			snx_audio_gpio_write(info);
			snx_audio_gpio_close();
			break;
		case GPIO_FUNC_I2C:
			snx_i2c_gpio_open();
			snx_i2c_gpio_write(info);
			snx_i2c_gpio_close();
			break;
		case GPIO_FUNC_SPI:
			snx_spi_gpio_open();
			snx_spi_gpio_write(info);
			snx_spi_gpio_close();
			break;
		default:
			ret = MPT_XU_CMD_ERR;
			print_msg("not support the gpio %d\n", info.pinumber);
			break;
	}
	
	return ret;
}
/**
* @brief interface function - MP tool function : set GPIO in
* @param data the buffer of data
* @param len data length 
* @return value of error code.
*/
static int mpt_gpio_in (char* data, int len)
{
	int ret = 0;
	gpio_pin_info info;
	unsigned int number;
	int func;
	debug_print ("%s:%d num %d\n", __FUNCTION__, __LINE__, mpt_status.gpio_num);	
	
	number = mpt_status.gpio_num;	
	info.mode = 0;
	func = mpt_gpio_get_info( number, &info.pinumber );
	switch( func ){
		case GPIO_FUNC_GPIO:
			snx_gpio_open();
			snx_gpio_read(&info);
			snx_gpio_close();
			break;
		case GPIO_FUNC_PWM:
			snx_pwm_gpio_open();
			snx_pwm_gpio_read(&info);
			snx_pwm_gpio_close();
			break;
		case GPIO_FUNC_MS1:
			snx_ms1_gpio_open();
			snx_ms1_gpio_read(&info);
			snx_ms1_gpio_close();
			break;
		case GPIO_FUNC_AUDIO:
			snx_audio_gpio_open();
			snx_audio_gpio_read(&info);
			snx_audio_gpio_close();
			break;
		case GPIO_FUNC_I2C:
			snx_i2c_gpio_open();
			snx_i2c_gpio_read(&info);
			snx_i2c_gpio_close();
			break;
		case GPIO_FUNC_SPI:
			snx_spi_gpio_open();
			snx_spi_gpio_read(&info);
			snx_spi_gpio_close();
			break;
		default:
			ret = MPT_XU_CMD_ERR;
			print_msg("not support the gpio %d\n", info.pinumber);
			break;
	}
	
	print_msg("the gpio %d %d\n", info.pinumber, info.value);
	data[1] = mpt_status.gpio_num;
	data[2] = info.value;
	
	return ret;
}

/**
* @brief interface function - MP tool function :   set SSID of network
* @param data the buffer of data
* @param len data length 
* @return value of error code.
*/
static int mpt_net_sta_ssid (char* data, int len)
{
	debug_print ("%s:%d \n", __FUNCTION__, __LINE__);
	
	memcpy(mpt_status.sWiFiSSID, &data[2], data[1]);
	mpt_status.sWiFiSSID[data[1]] = 0;
	print_msg("set ssid %s\n", mpt_status.sWiFiSSID);
	
	return 0;
}
/**
* @brief interface function - MP tool function :   set password of network
* @param data the buffer of data
* @param len data length 
* @return value of error code.
*/
static int mpt_net_sta_passwd (char* data, int len)
{
	debug_print ("%s:%d \n", __FUNCTION__, __LINE__);
	
	memcpy(mpt_status.sWiFiPWD, &data[2], data[1]);
	mpt_status.sWiFiPWD[data[1]] = 0;
	print_msg("set PWD %s\n", mpt_status.sWiFiPWD);

	return 0;
}
/**
* @brief interface function - MP tool function :   parse addr of network
* @param data the buffer of data
* @param len data length 
* @return value of error code.
*/
static int mpt_net_ip4_addr (ip_addr_t *addr, char* data, int len)
{
	debug_print ("%s:%d \n", __FUNCTION__, __LINE__);
	int i = 0, j = 0, k = 0;
	unsigned char d[4];

	for(i = 0; i <= len; i++){
		if((*(data+i) == '.') || (*(data+i) == 0)){
			*(data+i) = 0;
			d[k++] = atoi(data+j);
			j = i + 1;
		}
	}
	IP4_ADDR(addr, d[0], d[1], d[2], d[3]);

	return 0;
}
/**
* @brief interface function - MP tool function :   show IP address 
* @param data none
* @param len none
* @return value of error code.
*/
extern struct netif EMAC_if;
static int mpt_net_sta_ipaddr (char* data, int len)
{
	debug_print ("%s:%d \n", __FUNCTION__, __LINE__);
	
	unsigned char net_mode, ip_mode, ipaddr_len, mask_len, gw_len;
	ip_addr_t ip_addr, sn_mask, gw_addr;
	char *p = data + 1;
	struct netif E_if;
	struct netif * netif = &EMAC_if;

	ip_mode = ((*p)&0x01) ? NET_IP6_MODE : NET_IP4_MODE;
	net_mode = ((*p++)&0x02) ? NET_STATIC_MODE : NET_DHCP_MODE;

	if (ip_mode == NET_IP6_MODE){
		print_msg("[warning] not support IPv6 mode\n");
		return MPT_XU_CMD_ERR;
	}
	if (net_mode == NET_DHCP_MODE){
		dhcp_start(netif);
	}
	else {
		ipaddr_len = *p++;
		mask_len = *(p+ipaddr_len);
		*(p+ipaddr_len) = 0;
		mpt_net_ip4_addr(&ip_addr, p, ipaddr_len);

		p += ipaddr_len + 1;
		gw_len = *(p+mask_len);
		*(p+mask_len) = 0;
		mpt_net_ip4_addr(&sn_mask, p, mask_len);

		p += mask_len + 1;		
		*(p+gw_len) = 0;
		mpt_net_ip4_addr(&gw_addr, p, gw_len);
		
		print_msg("Net static mode. Set ip 0x%x, mask 0x%x, gw 0x%x\n", ip_addr, sn_mask, gw_addr);
		dhcp_stop(netif);
		netif_set_ipaddr(netif, &ip_addr);
		netif_set_netmask(netif, &sn_mask);
		netif_set_gw(netif, &gw_addr);	
	}
	return 0;
}
/**
* @brief interface function - MP tool function :   set SSID
* @param data the buffer of data
* @param len the length of data
* @return value of error code.
*/
static int mpt_net_ap_ssid (char* data, int len)
{
	debug_print ("%s:%d \n", __FUNCTION__, __LINE__);
	
	memcpy(mpt_status.sWiFiSSID, &data[2], data[1]);
	mpt_status.sWiFiSSID[data[1]] = 0;
	print_msg("set ssid %s\n", mpt_status.sWiFiSSID);
	
	return 0;
}
/**
* @brief interface function - MP tool function :   set AP Password
* @param data the buffer of data
* @param len the length of data
* @return value of error code.
*/
static int mpt_net_ap_passwd (char* data, int len)
{
	debug_print ("%s:%d \n", __FUNCTION__, __LINE__);
	debug_print ("wifi pwd mode: %d \n",data[1]);
	if(data[1] <= 0x01){
		memset(mpt_status.sWiFiPWD, 32, 0);
		memcpy(mpt_status.sWiFiPWD, &data[3], data[2]);
		//mpt_status.sWiFiPWD[5] = 0;
		print_msg("set PWD %s\n", mpt_status.sWiFiPWD);
	}
	else if(data[1] == 0xff){
		mpt_status.sWiFiPWD[0] = 0;
		print_msg("set no encrypt\n");
	}
	else{
		print_msg("not support encrypt method\n");
		return MPT_XU_CMD_ERR;
	}
	
	return 0;
}
/**
* @brief interface function - MP tool function :   show  AP IP ADDRESS
* @param data none
* @param len none
* @return value of error code.
*/
static int mpt_net_ap_ipaddr (char* data, int len)
{
	debug_print ("%s:%d \n", __FUNCTION__, __LINE__);
	
	return 0;
}
/**
* @brief interface function - MP tool function :    get SSID of network
* @param data the buffer of data
* @param len the length of data
* @return value of error code.
*/
static int mpt_get_net_sta_ssid (char* data, int len)
{
	debug_print ("%s:%d \n", __FUNCTION__, __LINE__);

	data[1] = sizeof(mpt_status.sWiFiSSID);
	memcpy(&data[2], mpt_status.sWiFiSSID, data[1]);
	print_msg("[xu mpt]get ssid %s\n", &data[2]);
	
	return 0;
}
/**
* @brief interface function - MP tool function :    get password of network
* @param data the buffer of data
* @param len the length of data
* @return value of error code.
*/
static int mpt_get_net_sta_passwd (char* data, int len)
{
	debug_print ("%s:%d \n", __FUNCTION__, __LINE__);

	data[1] = sizeof(mpt_status.sWiFiPWD);
	memcpy(&data[2], mpt_status.sWiFiPWD, data[1]);
	print_msg("[xu mpt]get PWD %s\n", &data[2]);

	return 0;
}
/**
* @brief interface function - MP tool function :    get AP SSID of network
* @param data the buffer of data
* @param len the length of data
* @return value of error code.
*/
static int mpt_get_net_ap_ssid (char* data, int len)
{
	debug_print ("%s:%d \n", __FUNCTION__, __LINE__);
	USHORT ssid_len;
	
	WiFi_QueryAndSet(QID_BEACON_SSID, &data[2], &ssid_len);
	data[1] = ssid_len;
	print_msg("[xu mpt]get ssid %s\n", &data[2]);
	
	return 0;
}
/**
* @brief interface function - MP tool function :    get AP password of network
* @param data the buffer of data
* @param len the length of data
* @return value of error code.
*/
static int mpt_get_net_ap_passwd (char* data, int len)
{
	debug_print ("%s:%d \n", __FUNCTION__, __LINE__);
	USHORT pwd_len;

	WiFi_QueryAndSet(QID_SECURITY_WEP, &data[3], &pwd_len);		
	if(pwd_len == 5){
		data[1] = 0x00;/*WEP64*/
	}
	else if(pwd_len == 0){
		data[1] = 0xff; /*no encrypt*/
	}
	else{
		print_msg("[warning]xu mpt encrypt method wrong\n");
	}
	data[2] = pwd_len;
	print_msg("[xu mpt]get PWD %s\n", &data[3]);
	
	return 0;
}
/**
* @brief interface function - MP tool function :    get IP address of network
* @param data the buffer of data
* @param len the length of data
* @return value of error code.
*/
static int mpt_get_net_ipaddr (char* data, int len)
{
	debug_print ("%s:%d \n", __FUNCTION__, __LINE__);

	data[1] = mpt_status.ip_mode;
	data[2] = 15;
	get_local_ip(&data[3], data[2]);

	print_msg("get net ip addr %s\n", &data[3]);

	return 0;
}
/**
* @brief interface function - MP tool function :    get MAC of network
* @param data the buffer of data
* @param len the length of data
* @return value of error code.
*/
static int mpt_get_net_mac (char* data, int len)
{
	unsigned char * macAddr = wlan_get_get_mac_addr();

	debug_print ("%s:%d \n", __FUNCTION__, __LINE__);

	sprintf (data + 1, "%02x:%02x:%02x:%02x:%02x:%02x", macAddr[0],
		macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
	
	print_msg("get str: %s \n", data+1);
		
	return 0;
}
/**
* @brief interface function - MP tool function :    show MAC of network
* @param data none
* @param len none
* @return value of error code.
*/
static int mpt_net_mac (char* data, int len)
{
	debug_print ("%s:%d \n", __FUNCTION__, __LINE__);
		
	return 0;
}
/**
* @brief interface function - MP tool function :   active network
* @param data the buffer of data
* @param len the length of data
* @return value of error code.
*/
static int mpt_net_active (char* data, int len)
{
	debug_print ("%s:%d \n", __FUNCTION__, __LINE__);
	int ret = 0;

#if CONFIG_MODULE_WIFI_SUPPORT
#if CONFIG_WIFI_MODE_AP
	if(data[1] & 0x01){
		print_msg("[warning]now is AP mode\n");
		return MPT_XU_CMD_ERR;
	}
#else
	if((data[1] & 0x01) == 0){
		print_msg("[warning]now is STA mode\n");
		return MPT_XU_CMD_ERR;
	}
#endif
	ret = mpt_message_send(&mpt_status.common, mpt_status.mpt_cur_cmd, data, len);
#endif
	
	return ret;
}
/**
* @brief interface function - MP tool function :   active network
* @param module the buffer of data
* @return value of error code.
*/
static int mpt_net_sta_active(mpt_module_manager* module)
{
	WiFi_Task_Init(NULL, WIFI_RUN_MODE_DEV);

	return 0;
}
/**
* @brief interface function - MP tool function :   active AP 
* @param module the buffer of data
* @return value of error code.
*/
static int mpt_net_ap_active(mpt_module_manager* module)
{
	debug_print ("%s:%d  %s %s\n", __FUNCTION__, __LINE__, mpt_status.sWiFiSSID, mpt_status.sWiFiPWD);
	int len;
	int enc = 0;

	WiFi_QueryAndSet(SET_BEACON_OFF, NULL, NULL);
	
	len = strlen(mpt_status.sWiFiSSID);
	WiFi_QueryAndSet(SET_BEACON_SSID, mpt_status.sWiFiSSID, &len);
	
	if( snx_nvram_integer_get("WIFI_DEV", "AP_AUTH_MODE", &enc) != NVRAM_SUCCESS) {
		print_msg("get nvram failed.");
		return -1;
	}

	len = strlen(mpt_status.sWiFiPWD);

	if (enc == WEP_MODE) {
		if(len == 5){
			WiFi_QueryAndSet(SET_SECURITY_WEP,  mpt_status.sWiFiPWD, &len);
		}
		else if(len == 0){
			WiFi_QueryAndSet(SET_SECURITY_WEP_DISABLED, NULL, NULL);
		}
		else{
			return -1;
		}
	} else if (enc == WPA_MODE || enc == WPA2_MODE) {
		WiFi_QueryAndSet((enc == WPA_MODE)?(SET_SECURITY_WPA):(SET_SECURITY_WPA2), (unsigned char *)mpt_status.sWiFiPWD, &len);
	} else {
		print_msg("Set WiFi PWD ERROR\n");
	}
	WiFi_QueryAndSet(SET_BEACON_ON, NULL, NULL);
	print_msg("Set AP active, ssid: %s, password: %s\n", mpt_status.sWiFiSSID, mpt_status.sWiFiPWD);

	return 0;
}

static int mpt_get_net_mode (char* data, int len)
{
#if CONFIG_MODULE_WIFI_SUPPORT
#if CONFIG_WIFI_MODE_AP
	data[1] = 1 << 0;
#else
	data[1] = 1 << 1;
#endif
#endif
	
	return 0;
}

static int mpt_video_fmt (char* data, int len)
{
	debug_print ("%s:%d len=0x%x, mpt_cur_cmd=0x%x\n", __FUNCTION__, __LINE__, len, data[1]);
	
	return 0;
}

static int mpt_video_fps (char* data, int len)
{
	debug_print ("%s:%d len=0x%x, mpt_cur_cmd=0x%x\n", __FUNCTION__, __LINE__, len, data[1]);
	
	return 0;
}

static int mpt_video_bitrate (char* data, int len)
{
	unsigned short bitrate;

	bitrate = data[1] | data[2] << 8;

	debug_print ("%s:%d len=0x%x, value=0x%x\n", __FUNCTION__, __LINE__, len, bitrate);
	
	return 0;
}

static int mpt_video_resolution (char* data, int len)
{
	int ret;
	unsigned short width,height;

	width = data[1] | data[2] << 8;
	height = data[3] | data[4] << 8;

	debug_print ("%s:%d len=0x%x, width=0x%x,height=0x%x\n", __FUNCTION__, __LINE__, len, width, height);
	
	return 0;
}

static int mpt_get_all_resolution (char* data, int len)
{	
	debug_print ("%s:%d \n", __FUNCTION__, __LINE__);
	
	return 0;
}

static int mpt_get_video_resolution (char* data, int len)
{
	debug_print ("%s:%d \n", __FUNCTION__, __LINE__);	

	return 0;
}

static int mpt_get_video_fps (char* data, int len)
{
	debug_print ("%s:%d \n", __FUNCTION__, __LINE__);

	return 0;
}

static int mpt_get_video_bitrate (char* data, int len)
{
	debug_print ("%s:%d \n", __FUNCTION__, __LINE__);

	return 0;
}

static int mpt_get_video_format (char* data, int len)
{	
	debug_print ("%s:%d \n", __FUNCTION__, __LINE__);

	return 0;
}

static int mpt_ir_cut_set_mode (int op)
{
	int ret = 0;

	debug_print ("%s:%d \n", __FUNCTION__, __LINE__);
	if (op == IR_DAY_MODE) {
		ircut_p.value	 = 0;
		snx_isp_filter_saturation_set(0x40);
		print_msg("[IR-CTL] Day mode\n");
	} else if ( op == IR_NIGHT_MODE ) {
		ircut_p.value	 = 1;
		snx_isp_filter_saturation_set( 0x0 );
		print_msg("[IR-CTL] Night mode\n");
	} else {
		print_msg("[IR-Cut] Wrong operation!\n");
		ret = MPT_XU_CMD_ERR;
	}

	if (snx_ms1_gpio_write(ircut_p) == GPIO_FAIL){
		print_msg ("msi%d gpio fail\n",ircut_p.pinumber);
		ret = MPT_XU_CMD_ERR;		
	}

	vTaskDelay(120 / portTICK_RATE_MS);

	if (op == IR_DAY_MODE) {
		ircut_p.value	 = 0;
	} else if ( op == IR_NIGHT_MODE ) {
		ircut_p.value	 = 0;
	} else {
		print_msg("[IR-Cut] Wrong operation!\n");
		ret = MPT_XU_CMD_ERR;		
	}

	if (snx_ms1_gpio_write(ircut_p) == GPIO_FAIL){
		print_msg ("msi%d gpio fail\n",ircut_p.pinumber);
		ret = MPT_XU_CMD_ERR;		
	}
	
	return ret;
}

static int mpt_vdo_mirror_set(char* data, int len)
{
	int en, sz;

	en = (int)data[1];
	debug_print("%s set mirror %d\n", __func__, en);
	sz = snx_isp_sensor_mirror_set(en);
	if(sz < 0)
		return MPT_XU_CMD_ERR;
	
	return 0;
}

static int mpt_vdo_mirror_get(char* data, int len)
{
	int en, sz;
	
	sz = snx_isp_sensor_mirror_get(&en);
	if(sz < 0)
		return MPT_XU_CMD_ERR;	
	data[1] = (unsigned char)en;
	debug_print("%s en %d\n", __func__, en);
	
	return 0;
}

static int mpt_vdo_flip_set(char* data, int len)
{
	int en, sz;

	en = (int)data[1];
	debug_print("%s set flip %d\n", __func__, en);
	sz = snx_isp_sensor_flip_set(en);
	if(sz < 0)
		return MPT_XU_CMD_ERR;
	
	return 0;}

static int mpt_vdo_flip_get(char* data, int len)
{
	int en, sz;
	
	sz = snx_isp_sensor_flip_get(&en);
	if(sz < 0)
		return MPT_XU_CMD_ERR;
	data[1] = (unsigned char)en;
	debug_print("%s en %d\n", __func__, en);
	
	return 0;
}

static int mpt_vdo_br_set(char* data, int len)
{
	int bps;
	
	bps = (data[1] | (data[2] << 8)) << 10;
	snx_usbd_uvc_set_bitrate(bps);

	return 0;
}

static int mpt_vdo_br_get(char* data, int len)
{
	int bps;

	bps = snx_usbd_uvc_get_bitrate();
	debug_print("%s bps %d\n", __func__, bps);
	bps = bps >> 10;
	data[1] = bps & 0xff;
	data[2] = (bps >> 8) & 0xff;

	return 0;
}

static int 
mpt_message_send(
	mpt_module_manager * module, 
	int cmd, 
	char* data, 
	int len
)
{
	int ret;

	ret = xSemaphoreTake(module->sem, 0);
	if(ret == pdPASS){
		memcpy(module->data, data, len);
		xSemaphoreGive(module->sem);
		ret = xQueueSendToBack(module->queue, &cmd, 0);
	}	

	if(ret != pdPASS)
		ret = NOT_READY_ERR;
	else
		ret = NO_ERROR_ERR;
	
	return ret;
}

static int snx_sys_xu_mpt_set (char* data, int len)
{
	debug_print ("%s:%d len=0x%x, mpt_cur_cmd=0x%x\n", __FUNCTION__, __LINE__, len, data[0]);
	int ret;

	switch (data[0] & 0x7f) {
	case MPT_GET_VERSION:
		mpt_status.mpt_cur_cmd = MPT_GET_VERSION;
		break;
	case MPT_OSD_AF_REPORT:
		mpt_status.mpt_cur_cmd = MPT_OSD_AF_REPORT;
		return mpt_osd_af_report(data, len);
	case MPT_ADO_RECORD:
		mpt_status.mpt_cur_cmd = MPT_ADO_RECORD;
		return mpt_message_send(&mpt_status.audio, mpt_status.mpt_cur_cmd, data, len);
	case MPT_ADO_RECORD_GET_RESULT:
		mpt_status.mpt_cur_cmd = MPT_ADO_RECORD_GET_RESULT;
		break;
	case MPT_ADO_PLAYBACK:
		mpt_status.mpt_cur_cmd = MPT_ADO_PLAYBACK;
		return mpt_message_send(&mpt_status.audio, mpt_status.mpt_cur_cmd, data, len);
	case MPT_ADO_PLAYBACK_GET_RESULT:
		mpt_status.mpt_cur_cmd = MPT_ADO_PLAYBACK_GET_RESULT;
		break;
	case MPT_SD_INSERT_STATE:
		mpt_status.mpt_cur_cmd = MPT_SD_INSERT_STATE;
		break;
	case MPT_SD_RW_TEST:
		mpt_status.mpt_cur_cmd = MPT_SD_RW_TEST;
		return mpt_message_send(&mpt_status.sd, mpt_status.mpt_cur_cmd, data, len);
	case MPT_SD_RW_GET_RESULT:
		mpt_status.mpt_cur_cmd = MPT_SD_RW_GET_RESULT;
		break;
	case MPT_GPIO_OUT:
		mpt_status.mpt_cur_cmd = MPT_GPIO_OUT;
		return mpt_gpio_out (data, len);
	case MPT_GPIO_IN:
		mpt_status.mpt_cur_cmd = MPT_GPIO_IN;
		mpt_status.gpio_num = data[1];
		break;
	case MPT_NET_STA_SSID:
		mpt_status.mpt_cur_cmd = MPT_NET_STA_SSID;
		if (data[0] & 0x80)
			return 0;
		return mpt_net_sta_ssid (data, len);
	case MPT_NET_STA_PASSWD:
		mpt_status.mpt_cur_cmd = MPT_NET_STA_PASSWD;
		if (data[0] & 0x80)
			return 0;
		return mpt_net_sta_passwd (data, len);
	case MPT_NET_STA_IPADDR:
		mpt_status.mpt_cur_cmd = MPT_NET_STA_IPADDR;
		if (data[0] & 0x80) {
			mpt_status.ip_mode = data[1];
			return 0;
		}
		return mpt_net_sta_ipaddr (data, len);
	case MPT_NET_AP_SSID:
		mpt_status.mpt_cur_cmd = MPT_NET_AP_SSID;
		if (data[0] & 0x80)
			return 0;
		return mpt_net_ap_ssid (data, len);
	case MPT_NET_AP_PASSWD:
		mpt_status.mpt_cur_cmd = MPT_NET_AP_PASSWD;
		if (data[0] & 0x80)
			return 0;
		return mpt_net_ap_passwd (data, len);
	case MPT_NET_AP_IPADDR:
		mpt_status.mpt_cur_cmd = MPT_NET_AP_IPADDR;
		if (data[0] & 0x80) {
			mpt_status.ip_mode = data[1];
			return 0;
		}
		return mpt_net_ap_ipaddr (data, len);
	case MPT_NET_MACADDR:
		mpt_status.mpt_cur_cmd = MPT_NET_MACADDR;
		if (data[0] & 0x80)
			return 0;
		return mpt_net_mac (data, len);
	case MPT_NET_ACTIVE:
		mpt_status.mpt_cur_cmd = MPT_NET_ACTIVE;
		return mpt_net_active (data, len);
	case MPT_NET_VDO_GET_ALL_RESOLUTION:
		mpt_status.mpt_cur_cmd = MPT_NET_VDO_GET_ALL_RESOLUTION;
		return 0;
	case MPT_NET_VDO_FMT:
		mpt_status.mpt_cur_cmd = MPT_NET_VDO_FMT;
		if (data[0] & 0x80)
			return 0;
		return mpt_video_fmt (data, len);
	case MPT_NET_VDO_RESOLUTION:
		mpt_status.mpt_cur_cmd = MPT_NET_VDO_RESOLUTION;
		if (data[0] & 0x80)
			return 0;
		return mpt_video_resolution (data, len);
	case MPT_NET_VDO_FR:
		mpt_status.mpt_cur_cmd = MPT_NET_VDO_FR;
		if (data[0] & 0x80)
			return 0;
		return mpt_video_fps (data, len);
	case MPT_NET_VDO_BR:
		mpt_status.mpt_cur_cmd = MPT_NET_VDO_BR;
		if (data[0] & 0x80)
			return 0;
		return mpt_video_bitrate (data, len);
	case MPT_PROG_PROCESS:
		mpt_status.mpt_cur_cmd = MPT_IVALID_MODE;
		return MPT_XU_CMD_ERR;/*not support in rtos*/
	case MPT_NET_SUPPORT_MODE:
		mpt_status.mpt_cur_cmd = MPT_NET_SUPPORT_MODE;
		break;
	case MPT_IR_CUT_SET_MODE:
		mpt_status.mpt_cur_cmd = MPT_IR_CUT_SET_MODE;
		return mpt_message_send(&mpt_status.common, mpt_status.mpt_cur_cmd, data, len);
	case MPT_VDO_MIRROR:
		mpt_status.mpt_cur_cmd = MPT_VDO_MIRROR;
		if (data[0] & 0x80)
			return 0;
		mpt_vdo_mirror_set(data, len);
		break;
	case MPT_VDO_FLIP:
		mpt_status.mpt_cur_cmd = MPT_VDO_FLIP;
		if (data[0] & 0x80)
			return 0;
		mpt_vdo_flip_set(data, len);
		break;
	case MPT_VDO_BR:
		mpt_status.mpt_cur_cmd = MPT_VDO_BR;		
		if (data[0] & 0x80)
			return 0;
		mpt_vdo_br_set(data, len);
		break;
	default:
		mpt_status.mpt_cur_cmd = MPT_IVALID_MODE;
		return UNKNOWN_ERR;
	}
	return 0;
}

static int snx_sys_xu_mpt_get (char* data, int len)
{
	debug_print ("%s:%d len=0x%x, mpt_cur_cmd=0x%x\n", __FUNCTION__, __LINE__, len, mpt_status.mpt_cur_cmd);

	switch (mpt_status.mpt_cur_cmd) {
	case MPT_GET_VERSION:
		data[0] = MPT_GET_VERSION | 0x80;
		return mpt_get_version (data, len);
		break;
	case MPT_ADO_RECORD_GET_RESULT:
		data[0] = MPT_ADO_RECORD_GET_RESULT | 0x80;
		return mpt_record_result (data, len);
	case MPT_ADO_PLAYBACK_GET_RESULT:
		data[0] = MPT_ADO_PLAYBACK_GET_RESULT | 0x80;
		return mpt_playback_result(data, len);
	case MPT_SD_INSERT_STATE:
		data[0] = MPT_SD_INSERT_STATE | 0x80;
		return mpt_sd_insert_state (data, len);
	case MPT_SD_RW_GET_RESULT:
		data[0] = MPT_SD_RW_GET_RESULT | 0x80;
		return mpt_sd_rw_result (data, len);
	case MPT_GPIO_IN:
		data[0] = MPT_GPIO_IN | 0x80;
		return mpt_gpio_in (data, len);
	case MPT_NET_STA_SSID:
		data[0] = MPT_NET_STA_SSID | 0x80;
		return mpt_get_net_sta_ssid (data, len);
	case MPT_NET_STA_PASSWD:
		data[0] = MPT_NET_STA_PASSWD | 0x80;
		return mpt_get_net_sta_passwd (data, len);
	case MPT_NET_STA_IPADDR:
		data[0] = MPT_NET_STA_IPADDR | 0x80;
		return mpt_get_net_ipaddr (data, len);
	case MPT_NET_AP_SSID:
		data[0] = MPT_NET_AP_SSID | 0x80;
		return mpt_get_net_ap_ssid (data, len);
	case MPT_NET_AP_PASSWD:
		data[0] = MPT_NET_AP_PASSWD | 0x80;
		return mpt_get_net_ap_passwd (data, len);
	case MPT_NET_AP_IPADDR:
		data[0] = MPT_NET_AP_IPADDR | 0x80;
		return mpt_get_net_ipaddr (data, len);
	case MPT_NET_MACADDR:
		data[0] = MPT_NET_MACADDR | 0x80;
		return mpt_get_net_mac (data, len);
	case MPT_NET_VDO_GET_ALL_RESOLUTION:
		data[0] = MPT_NET_VDO_GET_ALL_RESOLUTION | 0x80;
		return mpt_get_all_resolution (data, len);
	case MPT_NET_VDO_FMT:
		data[0] = MPT_NET_VDO_FMT | 0x80;
		return mpt_get_video_format (data, len);
	case MPT_NET_VDO_RESOLUTION:
		data[0] = MPT_NET_VDO_RESOLUTION | 0x80;
		return mpt_get_video_resolution (data, len);
	case MPT_NET_VDO_FR:
		data[0] = MPT_NET_VDO_FR | 0x80;
		return mpt_get_video_fps (data, len);
	case MPT_NET_VDO_BR:
		data[0] = MPT_NET_VDO_BR | 0x80;
		return mpt_get_video_bitrate (data, len);
	case MPT_NET_SUPPORT_MODE:
		data[0] = MPT_NET_SUPPORT_MODE | 0x80;
		return mpt_get_net_mode (data, len);		
	case MPT_VDO_MIRROR:
		data[0] = MPT_VDO_MIRROR | 0x80;
		mpt_vdo_mirror_get(data, len);
		break;
	case MPT_VDO_FLIP:
		data[0] = MPT_VDO_FLIP | 0x80;
		mpt_vdo_flip_get(data, len);
		break;
	case MPT_VDO_BR:
		data[0] = MPT_VDO_BR | 0x80;
		mpt_vdo_br_get(data, len);
		break;
	default:
		return UNKNOWN_ERR;
	}
	return 0;
}

int snx_sys_xu_mpt (char* data, unsigned char rw_mode)
{
	debug_print ("%s:%d mode=0x%x\n", __FUNCTION__, __LINE__, rw_mode);
	
	if (rw_mode)
		return snx_sys_xu_mpt_get (data, XU_SYS_MPT_LEN);
	else
		return snx_sys_xu_mpt_set (data, XU_SYS_MPT_LEN);
}
/**
* @brief interface function - MP tool function :  XU process task of MP tool  
*/
void vTaskUsbdUvcXuMptCommon(void)
{
	print_msg("===== usbd uvc xu mpt common task init =====\n");
	int cmd;
	unsigned int len = mpt_status.data_length;
	mpt_module_manager* module = &mpt_status.common;
	char* addr[32];
		
	while(1){
		xQueueReceive(module->queue, &cmd, portMAX_DELAY);
		
		xSemaphoreTake(module->sem, portMAX_DELAY);
		
		debug_print("%s %d, receive cmd 0x%x\n", __func__, __LINE__, cmd);
		if(cmd == MPT_IR_CUT_SET_MODE){
			int op = module->data[1];
			xSemaphoreGive(module->sem);
			mpt_ir_cut_set_mode(op);
		}
		else if (cmd == MPT_NET_ACTIVE){
			unsigned char mode = module->data[1];
			xSemaphoreGive(module->sem);
			if(mode & 0x01){
				mpt_net_sta_active(module);
			}
			else{
				mpt_net_ap_active(module);
			}
		}
		else{
			xSemaphoreGive(module->sem);
		}
	}
}

void vTaskUsbdUvcXuMptSd(void)
{
	print_msg("===== usbd uvc xu mpt sd task init =====\n");
	int cmd;
	unsigned int len = mpt_status.data_length;
	mpt_module_manager* module = &mpt_status.sd;
	
	while(1){
		xQueueReceive(module->queue, &cmd, portMAX_DELAY);
		
		xSemaphoreTake(module->sem, portMAX_DELAY);
		
		debug_print("%s %d, receive cmd 0x%x\n", __func__, __LINE__, cmd);
		if(cmd == MPT_SD_RW_TEST){
			mpt_sd_rw_test (module->data, len);
		}
		
		xSemaphoreGive(module->sem);
	}

}

void vTaskUsbdUvcXuMptAudio(void)
{
	print_msg("===== usbd uvc xu mpt audio task init =====\n");
	int cmd;
	unsigned int len = mpt_status.data_length;
	mpt_module_manager* module = &mpt_status.audio;
	
	while(1){
		xQueueReceive(module->queue, &cmd, portMAX_DELAY);	
		
		xSemaphoreTake(module->sem, portMAX_DELAY);
		
		debug_print("%s %d, receive cmd 0x%x\n", __func__, __LINE__, cmd);
		
		if(cmd == MPT_ADO_RECORD){
			mpt_record_start(module->data[1]);
		}
		else if(cmd == MPT_ADO_PLAYBACK){
			mpt_playback_start(module->data);
		}
		
		xSemaphoreGive(module->sem);
	}
}

#define	TS_FONT_IN_NVRAM			1

#if TS_FONT_IN_NVRAM
static char *asccii = " 0123456789:/-_.Sonix";
static char *nvram_pack = "app_osd_ctrl_font";
static char *nvram_config = "12x16_Arial_SN98660_v2.dat";

static char *font_config_16 = "FontFile_16.bin";
static char *font_config_48 = "FontFile_48.bin";
#endif

/**
* @brief set osd enable to channel 0 and 1.
* @param enable set 1 if enable.
*/
static void set_osd_enable(int enable)
{
	snx_isp_osd_enable_set(ISP_CH_0, enable);
	snx_isp_osd_enable_set(ISP_CH_1, enable);
}

static void set_osd_data(char *str, int timestamp)
{
	snx_isp_osd_data_str_set(ISP_CH_0, str);
	snx_isp_osd_data_str_set(ISP_CH_1, str);
	snx_isp_osd_timestamp_set(ISP_CH_0, timestamp);
	snx_isp_osd_timestamp_set(ISP_CH_1, timestamp);
}

static void set_osd_template(char *str)
{
	snx_isp_osd_template_set(ISP_CH_0, str);
	snx_isp_osd_template_set(ISP_CH_1, str);
}

static void set_osd_font(char *font)
{
	snx_isp_osd_font_set(ISP_CH_0, font);
	snx_isp_osd_font_set(ISP_CH_1, font);
}

static void set_osd_gain(int ch, int gain)
{
	snx_isp_osd_gain_set(ch, gain);
}

static void set_osd_line1_position(int x, int y)
{
	snx_isp_osd_line_x_set(ISP_CH_0, ISP_OSD_LINE_2, x);
	snx_isp_osd_line_x_set(ISP_CH_1, ISP_OSD_LINE_2, x);
	snx_isp_osd_line_y_set(ISP_CH_0, ISP_OSD_LINE_2, y);
	snx_isp_osd_line_y_set(ISP_CH_1, ISP_OSD_LINE_2, y);
}

static void set_osd_line2_position(int x, int y)
{
	snx_isp_osd_line_x_set(ISP_CH_0, ISP_OSD_LINE_1, x);
	snx_isp_osd_line_x_set(ISP_CH_1, ISP_OSD_LINE_1, x);
	snx_isp_osd_line_y_set(ISP_CH_0, ISP_OSD_LINE_1, y);
	snx_isp_osd_line_y_set(ISP_CH_1, ISP_OSD_LINE_1, y);
}

static void set_osd_txt_color(int color)
{
	snx_isp_osd_txt_color_set(ISP_CH_0, color);
	snx_isp_osd_txt_color_set(ISP_CH_1, color);
}

static void set_osd_bg_color(int color)
{
	//snx_isp_osd_bg_color_set(ISP_CH_0, color);
	//snx_isp_osd_bg_color_set(ISP_CH_1, color);
	
	snx_isp_osd_bgd_color_set(ISP_CH_0, color);
	snx_isp_osd_bgd_color_set(ISP_CH_1, color);
}

static void set_osd_txt_transp(int transp)
{
	snx_isp_osd_txt_transp_set(ISP_CH_0, transp);
	snx_isp_osd_txt_transp_set(ISP_CH_1, transp);
}

static void set_osd_bg_transp(int transp)
{
	snx_isp_osd_bg_transp_set(ISP_CH_0, transp);
	snx_isp_osd_bg_transp_set(ISP_CH_1, transp);
}

static void set_osd_font_size(int size_index)
{
	snx_isp_osd_size_set(ISP_CH_0, size_index);
	snx_isp_osd_size_set(ISP_CH_1, size_index);
}


static void set_osd_line1_txt(const char *txt)
{
	snx_isp_osd_line_txt_set(ISP_CH_0, ISP_OSD_LINE_2, txt);
	snx_isp_osd_line_txt_set(ISP_CH_1, ISP_OSD_LINE_2, txt);
}

static void set_osd_line2_txt(const char *txt)
{
	snx_isp_osd_line_txt_set(ISP_CH_0, ISP_OSD_LINE_1, txt);
	snx_isp_osd_line_txt_set(ISP_CH_1, ISP_OSD_LINE_1, txt);
}

void mpt_osd_af_report_set_position(void)
{
	int img_width = 0, img_height = 0;
	int x = 0, y = 0;

	snx_usbd_uvc_get_cur_img_info(&img_width, &img_height);

	if ((img_width == 0) || (img_height == 0)) {
		print_msg("%s, get preview width/height information failed\n", __FUNCTION__);
		return;
	}

	/* font size 12x24, AF report length is 8 , gain is 3 */
	switch(mpt_af_report_osd_position) {
	case 2:
		x = img_width-(12*8*3);
		y = 0;
		break;

	case 3:
		x = 0;
		y = img_height-(24*3);
		break;

	case 4:
		x = img_width-(12*8*3);
		y = img_height-(24*3);
		break;

	case 0:
	case 1:
	default:
		x = 0;
		y = 0;
		break;
	}
	//debug_print("width=%d, height=%d, x=%d, y%d\n", img_width, img_height, x, y);
	set_osd_line1_position(x, y);
}

void vTaskUsbdUvcXuMptOSD(void)
{
	unsigned int len = 0;
	char str[32];
	int ret = 0;
	
#if TS_FONT_IN_NVRAM
	unsigned char *font_buf;
	int w0_sum;

	ret = snx_nvram_get_data_len(nvram_pack, nvram_config, &len);
	if((ret != NVRAM_SUCCESS) || (len == 0)){
		print_msg("NVRAM get font size error. ret=%d, size=%d !!!!!!\n", ret, len);
		goto fail1;
	}
	
	font_buf = (unsigned char *)pvPortMalloc(len, GFP_KERNEL, MODULE_APP);
	if(font_buf == NULL){
		print_msg("Could not allocation osd font memory space!!!!!!\n");
		goto fail1;
	}
	
	ret = snx_nvram_binary_get(nvram_pack, nvram_config, font_buf);
	if(ret != NVRAM_SUCCESS){
		print_msg("NVRAM get font data error . ret=%d!!!!!!\n", ret);
		vPortFree(font_buf);
		goto fail1;
	}
#endif

	set_osd_enable(ISP_DISABLE);
	set_osd_txt_color(0xFFFFFF);		//white
	set_osd_bg_color(0x000000);			//black
	set_osd_txt_transp(0xFF);
	set_osd_bg_transp(0xFF);
	set_osd_gain(ISP_CH_0, 0x03);
	set_osd_gain(ISP_CH_1, 0x01);
	set_osd_line1_position(0, 0);
	//set_osd_line2_position(24, 0);
		
	set_osd_template(asccii);
#if TS_FONT_IN_NVRAM
	set_osd_font_size(1);				//font size 12*16
	set_osd_font((char *)font_buf);
	vPortFree(font_buf);
#else
	set_osd_font_size(2);				//font size 16*16
	set_osd_font(ascii16x16);
#endif
	set_osd_data("", ISP_DISABLE);
	set_osd_enable(ISP_ENABLE);
	
	while(1){
		snx_isp_af_w0_sum_get(&w0_sum);
		sprintf(str, "%08d", w0_sum);
		//print_msg("w0_sum=%ld\n",w0_sum);
		set_osd_line1_txt(str);
		vTaskDelay(300/portTICK_RATE_MS);
	}

fail1:
	vTaskDelete(NULL);
	mpt_status.af2osd_task = NULL;
}

static int mpt_module_create(
	mpt_module_manager * module,
	void (*task)(void),
	char * name, 
	size_t stack, 
	int priority
)
{
	int ret = 0;
	if (pdPASS != xTaskCreate(task, name, stack, 0,priority, module->task)){
		return -1;
	}
	module->queue = xQueueCreate(1, sizeof(int));	
	if(module->queue == NULL){
		return -2;
	}
	module->data = (char*)pvPortMalloc(XU_SYS_MPT_LEN, GFP_KERNEL, MODULE_APP);
	if(module->data == NULL){
		return -3;
	}
	module->sem = xSemaphoreCreateMutex();	
	if(module->sem == NULL){
		return -4;
	}

	debug_print("%s %d (ret=%d)\n", __func__, __LINE__, ret);
	return ret;
}

static int mpt_module_delete(
	mpt_module_manager * module
)
{
	if(module->task){
		vTaskDelete(module->task);
		module->task = NULL;
	}
	if(module->data){
		vPortFree(module->data);
		module->data = NULL;
	}
	if(module->queue){
		vQueueDelete(module->queue);
		module->queue = NULL;
	}
	if(module->sem){
		vSemaphoreDelete(module->sem);
		module->sem = NULL;
	}
	
	return 0;
}

int initUsbdUvcXuMpt(void)
{
	int ret;
	struct mpt_status_t* pstatus = &mpt_status;
	
	/*common*/
	ret = mpt_module_create(&pstatus->common, vTaskUsbdUvcXuMptCommon, "mpt common", 
		STACK_SIZE_2K,PRIORITY_TASK_DRV_USBD_PROCESS);
	if(ret != 0){
		ret = -1;
		goto fail_exit1;
	}

	/*SD*/
	ret = mpt_module_create(&pstatus->sd, vTaskUsbdUvcXuMptSd, "mpt sd", 
		STACK_SIZE_2K,PRIORITY_TASK_DRV_USBD_PROCESS);
	if(ret != 0){
		ret = -2;
		goto fail_exit2;
	}

	/*audio*/
	ret = mpt_module_create(&pstatus->audio, vTaskUsbdUvcXuMptAudio, "mpt audio", 
		STACK_SIZE_2K,PRIORITY_TASK_DRV_USBD_PROCESS);
	if(ret != 0){
		ret = -3;
		goto fail_exit3;
	}


	pstatus->data_length = XU_SYS_MPT_LEN;

	return 0;

fail_exit3:
	mpt_module_delete(&pstatus->audio);
fail_exit2:
	mpt_module_delete(&pstatus->sd);
fail_exit1:
	mpt_module_delete(&pstatus->common);
	return ret;
}

int uninitUsbdUvcXuMpt(void)
{
	struct mpt_status_t* pstatus = &mpt_status;
	
	mpt_module_delete(&pstatus->common);
	mpt_module_delete(&pstatus->sd);
	mpt_module_delete(&pstatus->audio);

	return 0;
}

uint8_t usbd_uvc_events_sys_xu_mpt(uint8_t uvc_req)
{
	uint8_t err_state = NO_ERROR_ERR;
	uint32_t len = XU_SYS_MPT_LEN;
	uint8_t data[XU_SYS_MPT_LEN] = {0};

	USBD_MID_PRINT("SONiX XU_SYS_MPT_CTRL request (uvc_req %02x)\n", uvc_req);

	switch (uvc_req)
	{
	case UVC_SET_CUR:
		usbd_mid_uvc_read_data(data, &len);
		if(len != XU_SYS_MPT_LEN)
			print_msg("mpt set_cur length err!!!!!\n");
		err_state = snx_sys_xu_mpt(data, 0);
		break;

	case UVC_GET_CUR:
		err_state = snx_sys_xu_mpt(data, 1);
		break;

	case UVC_GET_DEF:
	case UVC_GET_MIN:
		memset(data, 0x00, XU_SYS_MPT_LEN);
		len = XU_SYS_MPT_LEN;
		break;

	case UVC_GET_MAX:
		memset(data, 0xFF, XU_SYS_MPT_LEN);
		len = XU_SYS_MPT_LEN;
		break;

	case UVC_GET_RES:
		memset(data, 0x00, XU_SYS_MPT_LEN);
		data[0] = 1;
		len = XU_SYS_MPT_LEN;
		break;

	case UVC_GET_LEN:
		data[0] = XU_SYS_MPT_LEN;
		data[1] = 0;
		len = 2;
		break;

	case UVC_GET_INFO:
		data[0] = 0x03;
		len = 1;
		break;

	default:
		err_state = INVALID_REQUEST_ERR;
		break;
	}

	if(uvc_req & 0x80)
		usbd_mid_uvc_write_data(data,len);

	return err_state;
}
/** @} */
