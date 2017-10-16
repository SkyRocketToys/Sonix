#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>
#include "cmd_debug.h"
#include "printlog.h"
#include <generated/snx_sdk_conf.h>
#include <trcUser.h>
#include <i2c/i2c.h>
#include <sensor/sensor.h>
#include <isp/isp.h>
#include <libmid_isp/snx_mid_isp.h>

#include <vc/snx_vc.h>
#include <libmid_vc/snx_mid_vc.h>
#include <libmid_fatfs/ff.h>


#include "cmd_isp.h"
#include <nonstdlib.h>
#include <string.h>

static void vIspMotion(void *pvParameters)
{
        int md_enable=1, md_threshold=300, status;
        unsigned int report[6] = {0};
        unsigned int mask[6] = {0};

        snx_isp_md_threshold_set(md_threshold);//0x0~0xffff
        snx_isp_md_int_threshold_set(1);
        snx_isp_md_block_mask_set(mask);
        snx_isp_md_int_timeout_set(1000);
        snx_isp_md_enable_set(md_enable);

        while(1) {
		snx_isp_md_enable_get(&md_enable);
                snx_isp_md_threshold_get(&md_threshold);
                snx_isp_md_int_get(&status); /* interrupt. if 0 timeout, else have motion */
		if(status){
			snx_isp_md_block_report_get(report);
			print_msg_queue("0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n", report[0], report[1], report[2], report[3], report[4], report[5]);
		}
        }

	return;
}

int cmd_isp_capture(int argc, char* argv[])
{
#if 1 // Raymond use middleware interface
	struct snx_m2m m2m_task0={-1};
	FIL VideoFile;

	static int counts;

	struct snx_m2m *m2m;
	int wbytes = 0;
	unsigned char *ptr = NULL;
	int   yuv_out_size;
	unsigned char *yuv_out;


	m2m = &m2m_task0;

	snx_video_set_resolution(m2m, 1280, 720);
	m2m->start = 0;
	snx_isp_set_fps(m2m, 30);

	m2m->isp_mode = VIDEO_PIX_FMT_SNX420;
	m2m->isp_bufs = 2;
	m2m->channel = 1;

	yuv_out_size = ((m2m->width)*(m2m->height)*3)/2;
	print_msg("yuv_out_size=%d=0x%x\n",yuv_out_size,yuv_out_size);
	yuv_out = (unsigned char *)pvPortMalloc(yuv_out_size, GFP_KERNEL, MODULE_CLI);
	print_msg("yuv_out=0x%08x\n",yuv_out);

	 //Open file on SD
	 if(f_open(&VideoFile, "/SoNix/record/Video.yuv", FA_CREATE_ALWAYS | FA_WRITE) == FR_OK) {
		print_msg_queue("Video file open ok\n");
	} else {
		print_msg_queue("Video file open fail\n");
	}

	print_msg_queue("(ch:%d)width:%d height:%d rate:%d\n", m2m->channel, m2m->width, m2m->height, m2m->isp_fps);
	snx_isp_start(m2m);
/*
	if (pdPASS != xTaskCreate(vIspMotion, "motion", 1024, (void*)NULL, 50, NULL)){
		print_msg_queue("could not create isp motion\n");
//		goto __close_video_stream;
	}
*/
	print_msg_queue("start streaming!\n");
	counts = 0;
	while(1){
		snx_isp_read(m2m);
		ptr = m2m->ctx[m2m->channel].userptr;
		print_msg_queue("frame:%d %d 0x%x 0x%x\n", counts
						, m2m->ctx[m2m->channel].size
						, m2m->ctx[m2m->channel].userptr
						, *(ptr));

		snx_420line_to_420((char *)ptr, (char *)yuv_out, m2m->width, m2m->height);

		if (f_write(&VideoFile
					, (unsigned char*)(yuv_out)
					, m2m->ctx[m2m->channel].size
					, (void *)&wbytes) != FR_OK) {
			print_msg("video save to sd error!!!, cnt = %d\n", wbytes);
		}

		if(counts > 10)
			break;
			
		counts++;
	}
	snx_isp_stop(m2m);

	f_close(&VideoFile);

	
#else
	int ret;
	int i, width, height, rate;
	static int counts;

	struct snx_frame_ctx ctx[2];

	width = simple_strtoul(argv[1], NULL, 0);;
	height = simple_strtoul(argv[2], NULL, 0);
	rate = simple_strtoul(argv[3], NULL, 0);

	if(width == 0 || height == 0 || rate == 0){
		print_msg_queue("plesae command like:isp width height rate\n");
		return 0;
	}

	if((ret = snx_isp_open(0, width, height, rate, VIDEO_PIX_FMT_SNX420)) == pdFAIL){
		print_msg_queue("open video device error!\n");
		return pdFAIL;
	}

	counts = 2;
	if((ret = snx_isp_reqbufs(0, &counts)) == pdFAIL){
		print_msg_queue("request buffers error!\n");
		goto __close_video_device;
	}

	for(i = 0; i < counts; i++){
		ctx[i].index = i;
		if((ret = snx_isp_querybuf(0, &ctx[i])) == pdFAIL){
			print_msg_queue("query buffers error!\n");
			goto __close_video_device;
		}
	}
	
	for(i = 0; i < counts; i++){
		ctx[i].index = i;
		if((ret =snx_isp_qbuf(0, &ctx[i])) == pdFAIL){
			print_msg_queue("enqueue buffer error!\n");
			goto __close_video_device;
		}
	}

	if((ret = snx_isp_streamon(0)) == pdFAIL){
		print_msg_queue("video stream on error!\n");
		goto __close_video_device;
	}

	if (pdPASS != xTaskCreate(vIspMotion, "motion", 1024, (void*)NULL, 50, NULL)){
		print_msg_queue("could not create isp motion\n");
		goto __close_video_stream;
	}

	print_msg_queue("start streaming!\n");

	counts = 0;
	while(1){
		struct snx_frame_ctx vb;
		if((ret = snx_isp_dqbuf(0, &vb)) == pdFAIL)
			goto __close_video_stream;

		print_msg_queue("frame:%d %d %d\n", counts++, vb.index, vb.size);

		if((ret =snx_isp_qbuf(0, &vb)) == pdFAIL)
			goto __close_video_device;		
	}

__close_video_stream:
	snx_isp_streamoff(0);
__close_video_device:
	snx_isp_close(0);
#endif
	return pdPASS;
}

int cmd_isp_echo(int argc, char* argv[])
{
	char b[32];
	int s = 32;

	memset(b, 0x0, 32);
	s = sprintf(b, "%s", argv[1]);
	if(snx_isp_ioctl(1, argv[2], b, &s) == pdFAIL){
		return pdFAIL;
	}

	return pdPASS;
}


int cmd_isp_cat(int argc, char* argv[])
{
	char b[32];
	int s = 32;

	memset(b, 0x0, 32);
	if(snx_isp_ioctl(0, argv[1], b, &s) == pdFAIL){
		return pdFAIL;
	}

	print_msg_queue("%s\n", b);

	return pdPASS;
}

