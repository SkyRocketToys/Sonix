#include "FreeRTOS.h"
#include <bsp.h>
#include <stdio.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include <vc/snx_vc.h>
#include <isp/isp.h>
#include "printlog.h"
#include <libmid_vc/snx_mid_vc.h>
#include <nonstdlib.h>
#include <string.h>
#include <libmid_fatfs/ff.h>
#include <sys_clock.h>
#include <cmd_video.h>

#define DEBUG 1
//#define printf(fmt, args...) if(DEBUG) print_msg((fmt), ##args)
#define printf(fmt, args...) if(DEBUG) print_msg_queue((fmt), ##args)

static uint32_t motion_person_bmp[] = 
{
0X0C364D42, 0X00000000, 0X00360000, 0X00280000,
0X00200000, 0X00200000, 0X00010000, 0X00000018,
0X0C000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00FF8000, 0X8000FF80,
0XFF8000FF, 0X00FF8000, 0X0000FF80, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00FF8000, 0X8000FF80,
0XFF8000FF, 0X00FF8000, 0X0000FF80, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X80000000, 0XFF8000FF, 0X00FF8000, 0X8000FF80,
0XFF8000FF, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X80000000, 0XFF8000FF, 0X00FF8000, 0X8000FF80,
0XFF8000FF, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X80000000, 0XFF8000FF, 0X00FF8000, 0X8000FF80,
0XFF8000FF, 0X00FF8000, 0X8000FF80, 0XFF8000FF,
0X00FF8000, 0X0000FF80, 0X00000000, 0X00FF8000,
0X8000FF80, 0XFF8000FF, 0X00FF8000, 0X8000FF80,
0XFF8000FF, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X80000000, 0XFF8000FF, 0X00FF8000, 0X8000FF80,
0XFF8000FF, 0X00FF8000, 0X8000FF80, 0XFF8000FF,
0X00FF8000, 0X0000FF80, 0X00000000, 0X00FF8000,
0X8000FF80, 0XFF8000FF, 0X00FF8000, 0X8000FF80,
0XFF8000FF, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X80000000, 0XFF8000FF, 0X00FF8000, 0X8000FF80,
0XFF8000FF, 0X00FF8000, 0X8000FF80, 0XFF8000FF,
0X00FF8000, 0X0000FF80, 0X00000000, 0X00FF8000,
0X8000FF80, 0XFF8000FF, 0X00FF8000, 0X0000FF80,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X80000000, 0XFF8000FF, 0X00FF8000, 0X8000FF80,
0XFF8000FF, 0X00FF8000, 0X8000FF80, 0XFF8000FF,
0X00FF8000, 0X0000FF80, 0X00000000, 0X00FF8000,
0X8000FF80, 0XFF8000FF, 0X00FF8000, 0X0000FF80,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X80000000, 0XFF8000FF, 0X00FF8000, 0X8000FF80,
0XFF8000FF, 0X00FF8000, 0X8000FF80, 0XFF8000FF,
0X00FF8000, 0X0000FF80, 0X00000000, 0X00FF8000,
0X8000FF80, 0XFF8000FF, 0X00FF8000, 0X0000FF80,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X80000000, 0XFF8000FF, 0X00FF8000, 0X8000FF80,
0XFF8000FF, 0X00FF8000, 0X8000FF80, 0XFF8000FF,
0X00FF8000, 0X0000FF80, 0X00000000, 0X00FF8000,
0X8000FF80, 0XFF8000FF, 0X00FF8000, 0X0000FF80,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X80000000, 0XFF8000FF,
0X00FF8000, 0X8000FF80, 0XFF8000FF, 0X00FF8000,
0X8000FF80, 0XFF8000FF, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X80000000, 0XFF8000FF,
0X00FF8000, 0X8000FF80, 0XFF8000FF, 0X00FF8000,
0X8000FF80, 0XFF8000FF, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X80000000, 0XFF8000FF,
0X00FF8000, 0X8000FF80, 0XFF8000FF, 0X00FF8000,
0X8000FF80, 0XFF8000FF, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X80000000, 0XFF8000FF,
0X00FF8000, 0X8000FF80, 0XFF8000FF, 0X00FF8000,
0X8000FF80, 0XFF8000FF, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X80000000, 0XFF8000FF,
0X00FF8000, 0X8000FF80, 0XFF8000FF, 0X00FF8000,
0X8000FF80, 0XFF8000FF, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X80000000, 0XFF8000FF,
0X00FF8000, 0X8000FF80, 0XFF8000FF, 0X00FF8000,
0X8000FF80, 0XFF8000FF, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00FF8000, 0X8000FF80, 0XFF8000FF, 0X00FF8000,
0X8000FF80, 0XFF8000FF, 0X00000000, 0X80000000,
0XFF8000FF, 0X00FF8000, 0X8000FF80, 0XFF8000FF,
0X00FF8000, 0X0000FF80, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00FF8000, 0X8000FF80, 0XFF8000FF, 0X00FF8000,
0X8000FF80, 0XFF8000FF, 0X00000000, 0X80000000,
0XFF8000FF, 0X00FF8000, 0X8000FF80, 0XFF8000FF,
0X00FF8000, 0X0000FF80, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X80000000,
0XFF8000FF, 0X00FF8000, 0X0000FF80, 0X00000000,
0X00FF8000, 0X8000FF80, 0XFF8000FF, 0X00FF8000,
0X8000FF80, 0XFF8000FF, 0X00FF8000, 0X8000FF80,
0XFF8000FF, 0X00FF8000, 0X8000FF80, 0XFF8000FF,
0X00FF8000, 0X0000FF80, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X80000000,
0XFF8000FF, 0X00FF8000, 0X0000FF80, 0X00000000,
0X00FF8000, 0X8000FF80, 0XFF8000FF, 0X00FF8000,
0X8000FF80, 0XFF8000FF, 0X00FF8000, 0X8000FF80,
0XFF8000FF, 0X00FF8000, 0X8000FF80, 0XFF8000FF,
0X00FF8000, 0X0000FF80, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X80000000,
0XFF8000FF, 0X00FF8000, 0X0000FF80, 0X00000000,
0X00000000, 0X80000000, 0XFF8000FF, 0X00FF8000,
0X8000FF80, 0XFF8000FF, 0X00FF8000, 0X8000FF80,
0XFF8000FF, 0X00FF8000, 0X8000FF80, 0XFF8000FF,
0X00FF8000, 0X0000FF80, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X80000000,
0XFF8000FF, 0X00FF8000, 0X0000FF80, 0X00000000,
0X00000000, 0X80000000, 0XFF8000FF, 0X00FF8000,
0X8000FF80, 0XFF8000FF, 0X00FF8000, 0X8000FF80,
0XFF8000FF, 0X00FF8000, 0X8000FF80, 0XFF8000FF,
0X00FF8000, 0X0000FF80, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X80000000,
0XFF8000FF, 0X00FF8000, 0X8000FF80, 0XFF8000FF,
0X00FF8000, 0X8000FF80, 0XFF8000FF, 0X00FF8000,
0X8000FF80, 0XFF8000FF, 0X00FF8000, 0X8000FF80,
0XFF8000FF, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X80000000,
0XFF8000FF, 0X00FF8000, 0X8000FF80, 0XFF8000FF,
0X00FF8000, 0X8000FF80, 0XFF8000FF, 0X00FF8000,
0X8000FF80, 0XFF8000FF, 0X00FF8000, 0X8000FF80,
0XFF8000FF, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00FF8000, 0X8000FF80, 0XFF8000FF,
0X00FF8000, 0X8000FF80, 0XFF8000FF, 0X00FF8000,
0X8000FF80, 0XFF8000FF, 0X00FF8000, 0X0000FF80,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00FF8000, 0X8000FF80, 0XFF8000FF,
0X00FF8000, 0X8000FF80, 0XFF8000FF, 0X00FF8000,
0X8000FF80, 0XFF8000FF, 0X00FF8000, 0X0000FF80,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X80000000, 0XFF8000FF,
0X00FF8000, 0X8000FF80, 0XFF8000FF, 0X00FF8000,
0X8000FF80, 0XFF8000FF, 0X00FF8000, 0X8000FF80,
0XFF8000FF, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X80000000, 0XFF8000FF,
0X00FF8000, 0X8000FF80, 0XFF8000FF, 0X00FF8000,
0X8000FF80, 0XFF8000FF, 0X00FF8000, 0X8000FF80,
0XFF8000FF, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00FF8000,
0X8000FF80, 0XFF8000FF, 0X00FF8000, 0X8000FF80,
0XFF8000FF, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00FF8000,
0X8000FF80, 0XFF8000FF, 0X00FF8000, 0X8000FF80,
0XFF8000FF, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00FF8000,
0X8000FF80, 0XFF8000FF, 0X00FF8000, 0X8000FF80,
0XFF8000FF, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000, 0X00000000, 0X00FF8000,
0X8000FF80, 0XFF8000FF, 0X00FF8000, 0X8000FF80,
0XFF8000FF, 0X00000000, 0X00000000, 0X00000000,
0X00000000, 0X00000000,
};


extern struct snx_m2m *snx_m2m_channel0;
extern struct snx_m2m *snx_m2m_channel1;

//struct snx_m2m *snx_m2m_channel0;
//struct snx_m2m *snx_m2m_channel1;

//struct snx_m2m m2m_task2={-1};


unsigned char *snx_temp_alloc(int temp_buf_size)
{
	unsigned char *buf = 0;
	buf = (unsigned char*)pvPortMalloc(temp_buf_size, GFP_KERNEL, MODULE_CLI);
	if ( 0 == buf ) {
		printf("<%s>pvPortMallo fail 0x%x\n",__func__, buf);
		return pdFAIL;
	}
//	memset((unsigned *)buf, 0x0, temp_buf_size);
	return buf;	
}

int snx_temp_write(int mode
							, struct snx_temp *snx_temp
							, struct snx_m2m *m2m
							, int temp_buf_size
							, int dup)
{
	int bs_size = 0;
	unsigned char *ptr = NULL;
	int wbytes = 0;

	if(mode & FMT_H264) {
		bs_size = snx_video_h264_stream(m2m, dup, (unsigned int *)&ptr);
	}
	else if	(mode & FMT_MJPEG) {
		bs_size = snx_video_jpeg_stream(m2m, dup, (unsigned int *)&ptr);
	}

	//print_msg("bs_size = %d\n", bs_size);

	if((snx_temp->total_size + bs_size) < temp_buf_size) {
		if (f_write(&(snx_temp->video_file), (unsigned char*)(ptr), bs_size, (void *)&wbytes) != FR_OK) {
			print_msg("video save to sd error!!!, cnt = %d\n", wbytes);
		}
#if 0
		memcpy((unsigned char *)snx_temp->ptr
			, (unsigned char*)(ptr)
			, bs_size);
		snx_temp->ptr += bs_size;
		snx_temp->total_size += bs_size;
#endif
	}
	else {
		print_msg("memory full\n");
	}
	return bs_size;
}




int snx_channel_set_vc(struct snx_m2m *m2m, int channel)
{
	int mode = 0;
	m2m->channel = channel;
	// setting dep. isp	
	if(channel == 0) { // FOR ISP 
		snx_video_set_resolution(m2m, 1280, 720);
	}
	else if(channel == 1) {
		snx_video_set_resolution(m2m, 320, 240);
	}
	else{ // FOR snx_dummy
		snx_video_set_resolution(m2m, 640, 480);
	}
	snx_isp_set_fps(m2m, 30);
	snx_video_set_scale(m2m, 1);

	mode |= FMT_H264; // H264
//	mode |= FMT_MJPEG; // JPEG

	// setting vc path 0
	snx_video_set_mode(m2m, 0, mode );
	snx_video_set_fps(m2m, 0, 30, FMT_H264);
	snx_video_set_fps(m2m, 0, 30, FMT_MJPEG);
	snx_video_set_bps(m2m, 0, 3*1024*1024);
	snx_video_set_gop(m2m, 0, 0); // fps == gop
	snx_video_set_qp(m2m, 0, 23, FMT_H264); // set h264 qp value
	snx_video_set_qp(m2m, 0, 64, FMT_MJPEG); // set jpeg qp value

	snx_video_set_bufs(m2m, 0, 1); // 
	snx_video_set_percent(m2m, 0, FMT_H264, 100); // 
	snx_video_set_percent(m2m, 0, FMT_MJPEG, 100); // 

	snx_set_mbrc_en(m2m, 0, 0);				// disable macro block based rate control
	snx_set_qp_boundary(m2m, 0, 35, 20);	// set h264 QP range between 20~35
	snx_set_delta_qp(m2m, 0, 5, 5);			// delta QP up bound (default 8, dashcam suggest set 5)
	snx_set_mdrc_en(m2m, 0, 0); 			// disable motion detection rate control
	snx_set_mdcnt_en(m2m, 0, 0);			// disable auto low bitrate control    
//	snx_video_set_refp(m2m, 0, 2);

	// setting vc path 1
	mode = 0;
	snx_video_set_mode(m2m, 1, mode );
	snx_video_set_fps(m2m, 1, 15, FMT_H264);
	snx_video_set_fps(m2m, 1, 15, FMT_MJPEG);
	snx_video_set_bps(m2m, 1, 128*1024);
	snx_video_set_gop(m2m, 1, 0); // fps == gop
	snx_video_set_qp(m2m, 1, 23, FMT_H264); // set h264 qp value
	snx_video_set_qp(m2m, 1, 64, FMT_MJPEG); // set jpeg qp value

	snx_video_set_bufs(m2m, 1, 1); // 
	snx_video_set_percent(m2m, 1, FMT_H264, 100); // 
	snx_video_set_percent(m2m, 1, FMT_MJPEG, 100); // 

	return pdPASS;
}

int snx_vc_get(struct snx_m2m *m2m, int channel)
{
	int width, height, scale;
	
	if(m2m->channel == -1) {
		snx_video_init(m2m);
		snx_channel_set_vc(m2m, channel);		
	}
	
	snx_video_get_resolution(m2m, &width, &height);
	scale = snx_video_get_scale(m2m);
	
	printf(" mode           = %s %s\n"
			, ((snx_video_get_mode(m2m, 0)&FMT_H264)>0)?"H264":""
			, ((snx_video_get_mode(m2m, 0)&FMT_MJPEG)>0)?"JPEG":"");
	printf(" resolution     = %d X %d\n", width, height);
	printf(" isp_fps        = %d fps\n", snx_isp_get_fps(m2m));
	printf(" fps            = H264 %d fps JPEG %d fps  GOP = %d\n"
			, snx_video_get_fps(m2m, 0, FMT_H264)
			, snx_video_get_fps(m2m, 0, FMT_MJPEG)
			, snx_video_get_gop(m2m, 0));
	printf(" bps            = %d Kbps    qp = %d\n"
			, snx_video_get_bps(m2m, 0)>>10
			, snx_video_get_qp(m2m, 0, FMT_MJPEG));			
	printf(" scale          = X%d\n", (1<<scale));
	printf(" bufs           = %d    h264_percent = %d%% jpeg_percent = %d%%\n"
			, snx_video_get_bufs(m2m, 0)
			, snx_video_get_percent(m2m, 0, FMT_H264)
			, snx_video_get_percent(m2m, 0, FMT_MJPEG));

	printf(" dup mode       = %s %s\n"
			, ((snx_video_get_mode(m2m, 1)&FMT_H264)>0)?"H264":""
			, ((snx_video_get_mode(m2m, 1)&FMT_MJPEG)>0)?"JPEG":"");
	printf(" dup resolution = %d X %d\n", width>>scale, height>>scale);
	printf(" dup fps        = H264 %d fps JPEG %d fps  GOP = %d\n"
			, snx_video_get_fps(m2m, 1, FMT_H264)
			, snx_video_get_fps(m2m, 1, FMT_MJPEG)
			, snx_video_get_gop(m2m, 1));
	printf(" dup bps        = %d Kbps    qp = %d\n"
			, snx_video_get_bps(m2m, 1)>>10
			, snx_video_get_qp(m2m, 1, FMT_MJPEG));
	printf(" bufs           = %d    h264_percent = %d%% jpeg_percent = %d%%\n"
			, snx_video_get_bufs(m2m, 1)
			, snx_video_get_percent(m2m, 1, FMT_H264)
			, snx_video_get_percent(m2m, 1, FMT_MJPEG));
	printf(" resume_time    = %d    suspend_time = %d\n"
			, snx_preview_get_resume(m2m)
			, snx_preview_get_suspend(m2m));
///////////////

	return pdPASS;
}

int snx_rc_get(struct snx_m2m *m2m, int channel, int dup)
{
	int value, value1, value2, value3;
	if(m2m->channel == -1) {
		snx_video_init(m2m);
		snx_channel_set_vc(m2m, channel);		
	}

	snx_get_mdrc_en(m2m, dup, &value);
	printf(" mdrc_en(md)               = %d\n", value);
	snx_get_md_mn(m2m, dup, &value, &value1);
	printf(" md_pixel (md_pixel)       = %d X %d\n", value, value1);

	snx_get_md_th(m2m, dup, &value, &value1);
	printf(" md_th(md_th)              = %d\n", value);
	printf(" md_recover(md_recover)    = %d\n", value1);

	snx_get_md_2dnr(m2m, dup, &value, &value1);
	printf(" md_2dnr(md_2dnr)          = %d\n", value);
	printf(" md_isp_nr(md_isp_nr)      = %d\n", value1);

	snx_get_md_fpsbps(m2m, dup, &value, &value1);
	printf(" md_max_fps(md_fps)        = %d\n", value);
	printf(" md_can_add_bitrate(md_bps)= %d\n", value1);
	
	snx_get_mdcnt_en(m2m, dup, &value);
	printf(" mdcnt_en(mdcnt)           = %d\n", value);
	snx_get_mdcnt_th(m2m, dup, &value, &value1);
	printf(" md_cnt_sum_th(sum_th)     = %d\n", value);
	printf(" md_cnt_th(th)             = %d\n", value1);
	snx_get_mdcnt_bps(m2m, dup, &value, &value1);
	printf(" md_cnt_bps(bps) = %d (bps2) = %d \n", value, value1);

	snx_get_mdcnt_lowbound(m2m, dup, &value, &value1);
	printf(" md_cnt_lowbound(low)      = %d\n", value);
	printf(" md_cnt_qp(qp)             = %d\n", value1);
	snx_get_mdcnt_absy(m2m, dup, &value);
	printf(" md_cnt_absy(absy)         = %d\n", value);
	snx_get_mdcnt_gop(m2m, dup, &value);
	printf(" snx_set_mdcnt_gop(gop_mul)= %d\n", value);
	snx_get_mdcnt_count(m2m, dup, &value);
	printf(" md_cnt_count(count)       = %d\n", value);

	snx_get_qp_boundary(m2m, dup, &value, &value1);
	printf(" max qp(bound_qp)          = %d\n", value);
	printf(" min qp(bound_qp)          = %d\n", value1);
	
	snx_get_ext_qp(m2m, dup, &value, &value1, &value2, &value3);
	printf(" pframe_sum_num(ext_qp)    = %d\n", value);
	printf(" maxqp_range(ext_qp)       = %d\n", value1);
	printf(" extend_max_qp(ext_qp)     = %d\n", value2);
	printf(" upperpframesize(ext_qp)   = %d\n", value3);

	return pdPASS;
}

int cmd_video_vc_set(int argc, char* argv[])
{
	struct snx_m2m *m2m=0;
	int channel=0;
	int dup = 0;
	int i;

	if (argc < 2) {
		printf(" Usage: vcset [channel] [[item] [value]] ..\n");
		printf(" channel: 0, 1 (default 0) \n");
		printf(" item:\n");
		printf("      dup: dup path (default 0) \n");
		printf("      mode: H264=2, JPEG=4, H264+JPEG=6\n");
		printf("      res: width height\n");
		printf("      fps: type frame_rate (type: isp=0, h264= 1, jpeg=2\n");
		printf("      bps: h264 bit rate\n");
		printf("      gop: H264 gop value\n");
		printf("      qp:  motion jpeg qp value\n");
		printf("      bufs:  buffer number\n");
		printf("      h264_percent: h264 buffer percent\n");
		printf("      jpeg_percent: jpeg buffer percent\n");
		printf("      preview_time: resume/suspend time\n");

		return pdFAIL;
	}

	channel = simple_strtoul(argv[1], NULL, 10);

	if (channel == 0) {
		m2m = snx_m2m_channel0;
	} else if (channel == 1) {
		m2m = snx_m2m_channel1;
//	} else {
//		m2m = &m2m_task2;
	}

	for (i=2 ; i<argc ;i=i+2) {
		if(strcmp(argv[i], "dup") == 0)
			dup = simple_strtoul(argv[i+1], NULL, 10);
		if(strcmp(argv[i], "mode") == 0)
			snx_video_set_mode(m2m, dup, simple_strtoul(argv[i+1], NULL, 10) );
		if(strcmp(argv[i], "res") == 0) {
			snx_video_set_resolution(m2m
						, simple_strtoul(argv[i+1], NULL, 10)
						, simple_strtoul(argv[i+2], NULL, 10));
			i++;
		}
		if(strcmp(argv[i], "fps") == 0) {
			int type=0;
			type = simple_strtoul(argv[i+1], NULL, 10);
			if(type == 0)
				snx_isp_set_fps(m2m, simple_strtoul(argv[i+2], NULL, 10));
			else
				snx_video_set_fps(m2m, dup, simple_strtoul(argv[i+2], NULL, 10), (type<<1));
			i++;

		}
		if(strcmp(argv[i], "bps") == 0)
			snx_video_set_bps(m2m, dup, simple_strtoul(argv[i+1], NULL, 10)); // set h264 bps
		if(strcmp(argv[i], "qp") == 0)
			snx_video_set_qp(m2m, dup, simple_strtoul(argv[i+1], NULL, 10), FMT_MJPEG); // set jpeg qp value

		if(strcmp(argv[i], "gop") == 0)
			snx_video_set_gop(m2m, dup, simple_strtoul(argv[i+1], NULL, 10)); // set h264 gop value

		if(strcmp(argv[i], "bufs") == 0)
			snx_video_set_bufs(m2m, dup, simple_strtoul(argv[i+1], NULL, 10)); // 
		
		if(strcmp(argv[i], "h264_percent") == 0)
			snx_video_set_percent(m2m, dup, FMT_H264, simple_strtoul(argv[i+1], NULL, 10)); // 
		
		if(strcmp(argv[i], "jpeg_percent") == 0)
			snx_video_set_percent(m2m, dup, FMT_MJPEG, simple_strtoul(argv[i+1], NULL, 10)); // 
		if(strcmp(argv[i], "preview_time") == 0)
			snx_preview_ctrl_time(m2m, dup
					, simple_strtoul(argv[i+1], NULL, 10)
					, simple_strtoul(argv[i+2], NULL, 10));
	}
	printf(" =========channel %d info========\n", channel);
	snx_vc_get(m2m, channel);

	return 0;
}
/*

struct snx_info {
	char name[16];
	// call back function
	int parameter;
	int (*set)(struct snx_m2m *m2m, int dup, int value);
	int (*get)(struct snx_m2m *m2m, int dup, int *value);
	
};

static struct snx_info info[] ={
			{"md", 1, snx_set_mdrc_en, snx_get_mdrc_en},
			{"md_m", 1, NULL, NULL},
			{"md_n", 1, NULL, NULL},
			{"md_th", 1, NULL, NULL},
			{"md_recover", 1, NULL, NULL},
			{"md_2dnr", 1, NULL, NULL},
			{"md_isp_nr", 1, NULL, NULL},
			{"md_fps", 1, NULL, NULL},
			{"md_bps", 1, NULL, NULL},
			{"bb", 1, NULL, NULL},		
			};
		if(strcmp(argv[i], "md_fps") == 0)
		if(strcmp(argv[i], "md_bps") == 0)
		if(strcmp(argv[i], "mdcnt") == 0)
			snx_set_mdcnt_en(m2m, 0, simple_strtoul(argv[i+1], NULL, 10));
		if(strcmp(argv[i], "sum_th") == 0)
			snx_set_mdcnt_th(m2m, 0, simple_strtoul(argv[i+1], NULL, 10), 0);
		if(strcmp(argv[i], "th") == 0)
			snx_set_mdcnt_th(m2m, 0, 0, simple_strtoul(argv[i+1], NULL, 10));
		if(strcmp(argv[i], "bps") == 0)
			snx_set_mdcnt_bps(m2m, 0, simple_strtoul(argv[i+1], NULL, 10), 0);
		if(strcmp(argv[i], "bps2") == 0)
			snx_set_mdcnt_bps(m2m, 0, 0, simple_strtoul(argv[i+1], NULL, 10));
		if(strcmp(argv[i], "low") == 0)
			snx_set_mdcnt_lowbound(m2m, 0, simple_strtoul(argv[i+1], NULL, 10), 0);
		if(strcmp(argv[i], "qp") == 0)
			snx_set_mdcnt_lowbound(m2m, 0, 0, simple_strtoul(argv[i+1], NULL, 10));
		if(strcmp(argv[i], "absy") == 0)
			snx_set_mdcnt_absy(m2m, 0, simple_strtoul(argv[i+1], NULL, 10));
		if(strcmp(argv[i], "gop_mul") == 0)
			snx_set_mdcnt_gop(m2m, 0, simple_strtoul(argv[i+1], NULL, 10));
		if(strcmp(argv[i], "count") == 0)
			snx_set_mdcnt_count(m2m, 0, simple_strtoul(argv[i+1], NULL, 10));
*/

int cmd_video_rc_set(int argc, char* argv[])
{
	struct snx_m2m *m2m=0;
	int channel=0, dup=0;
	int i;
	if(argc < 3){
		printf(" rcset [channel] [dup] [[item] [value] ..] ... (channel= 0 rc info)\n");
		printf(" channel: 0, 1 (default 0)\n");
		printf(" dup: dup path 0, 1 (default 0) \n");
		printf(" item: \n");
		printf("      md: motion detection rate contorl enable \n");
		printf("      md_pixel: [2 value] corner width, height MD block\n");
		printf("      md_th: corner trigger counte  \n");
		printf("      md_recover: md_recover   \n");
		printf("      md_2dnr: md_2dnr         \n");
		printf("      md_isp_nr: md_isp_nr     \n");
		printf("      md_fps: md_max_fps        \n");
		printf("      md_bps: md_can_add_bitrate\n");
		printf("      mdcnt: mdcnt_en           \n");
		printf("      sum_th: md_cnt_sum_th     \n");
		printf("      th:md_cnt_th \n");
		printf("      bps: [2 value] md_cnt_bps md_cnt_bps2\n");
		printf("      low: md_cnt_lowbound     \n");
		printf("      qp: md_cnt_qp            \n");
		printf("      absy: md_cnt_absy        \n");
		printf("      gop_mul: snx_set_mdcnt_gop\n");
		printf("      count: md_cnt_count      \n");
		printf("      bound_qp: [2 value] max qp, min qp \n");
		printf("      ext_qp: [4 value] pframe_sum_num, maxqp_range, extend_max_qp upperpframesize  \n");
		return pdFAIL;
	}

	channel =simple_strtoul(argv[1], NULL, 10);
	dup =simple_strtoul(argv[2], NULL, 10);

	if(channel == 0) {
		m2m = snx_m2m_channel0;
	}
	else if (channel == 1){
		m2m = snx_m2m_channel1;
	}
//	else
//		m2m= &m2m_task2;

		
	for (i=3 ; i<argc ;i=i+2) {
		if(strcmp(argv[i], "md") == 0)
			snx_set_mdrc_en(m2m, dup, simple_strtoul(argv[i+1], NULL, 10));
		if(strcmp(argv[i], "md_pixel") == 0) {
			snx_set_md_mn(m2m, dup, simple_strtoul(argv[i+1], NULL, 10), 0);
			snx_set_md_mn(m2m, dup, simple_strtoul(argv[i+2], NULL, 10), 0);
		}
		if(strcmp(argv[i], "md_th") == 0)
			snx_set_md_th(m2m, dup, simple_strtoul(argv[i+1], NULL, 10), 0);
		if(strcmp(argv[i], "md_recover") == 0)
			snx_set_md_th(m2m, dup, 0, simple_strtoul(argv[i+1], NULL, 10));
		if(strcmp(argv[i], "md_2dnr") == 0)
			snx_set_md_2dnr(m2m, dup, simple_strtoul(argv[i+1], NULL, 10), 0);
		if(strcmp(argv[i], "md_isp_nr") == 0)
			snx_set_md_2dnr(m2m, dup, 0, simple_strtoul(argv[i+1], NULL, 10));
		if(strcmp(argv[i], "md_fps") == 0)
			snx_set_md_fpsbps(m2m, dup, simple_strtoul(argv[i+1], NULL, 10), 0);
		if(strcmp(argv[i], "md_bps") == 0) {
			snx_set_md_fpsbps(m2m, dup, 0, simple_strtoul(argv[i+1], NULL, 10));
		}
		if(strcmp(argv[i], "mdcnt") == 0)
			snx_set_mdcnt_en(m2m, dup, simple_strtoul(argv[i+1], NULL, 10));
		if(strcmp(argv[i], "sum_th") == 0)
			snx_set_mdcnt_th(m2m, dup, simple_strtoul(argv[i+1], NULL, 10), 0);
		if(strcmp(argv[i], "th") == 0)
			snx_set_mdcnt_th(m2m, dup, 0, simple_strtoul(argv[i+1], NULL, 10));
		if(strcmp(argv[i], "bps") == 0) {
			snx_set_mdcnt_bps(m2m, dup
							, simple_strtoul(argv[i+1], NULL, 10)
							, simple_strtoul(argv[i+2], NULL, 10));
		}
		if(strcmp(argv[i], "low") == 0)
			snx_set_mdcnt_lowbound(m2m, dup, simple_strtoul(argv[i+1], NULL, 10), 0);
		if(strcmp(argv[i], "qp") == 0)
			snx_set_mdcnt_lowbound(m2m, dup, 0, simple_strtoul(argv[i+1], NULL, 10));
		if(strcmp(argv[i], "absy") == 0)
			snx_set_mdcnt_absy(m2m, dup, simple_strtoul(argv[i+1], NULL, 10));
		if(strcmp(argv[i], "gop_mul") == 0)
			snx_set_mdcnt_gop(m2m, dup, simple_strtoul(argv[i+1], NULL, 10));
		if(strcmp(argv[i], "count") == 0)
			snx_set_mdcnt_count(m2m, dup, simple_strtoul(argv[i+1], NULL, 10));

		if(strcmp(argv[i], "ext_qp") == 0){
			snx_set_ext_qp(m2m, dup
						, simple_strtoul(argv[i+1], NULL, 10)
						, simple_strtoul(argv[i+2], NULL, 10)
						, simple_strtoul(argv[i+3], NULL, 10)
						, simple_strtoul(argv[i+4], NULL, 10));
		}
		if(strcmp(argv[i], "bound_qp") == 0)
			snx_set_qp_boundary(m2m, dup
						, simple_strtoul(argv[i+1], NULL, 10)
						, simple_strtoul(argv[i+2], NULL, 10));

	}

	printf(" =========channel %d info========\n", channel);
	snx_rc_get(m2m, channel, dup);
	return pdPASS;	
}



int cmd_video_ds_set(int argc, char* argv[])
{
	struct snx_m2m *m2m=0;
	int channel;
	uint8_t vc_enc_num;
	uint8_t enable;
	uint8_t mode;
	uint8_t weight;
	uint8_t scale;
	uint32_t pos_x;
	uint32_t pos_y;
	uint8_t transparent;
	uint32_t bmp_thres = 0;
	uint8_t bmp_rounding = 0;

	if(argc!=11){
		printf(" usage: ds_set (channel) (vc_enc#) (enable) (mode) (weight) (scale)");
		printf("(pos_x) (pos_y) (transparent) (src)\n");
		printf("src can be bmp or string\n");
		return pdFAIL;
	}
	channel =simple_strtoul(argv[1], NULL, 10);
	if(channel == 0) {
		m2m = snx_m2m_channel0;
	}
	else if (channel == 1){
		m2m = snx_m2m_channel1;
	}
//	else
//		m2m= &m2m_task2;

	struct snx_vc *vc =  &m2m->vc;

	vc_enc_num =simple_strtoul(argv[2], NULL, 10);
	enable = simple_strtoul(argv[3], NULL, 10);
	mode = simple_strtoul(argv[4], NULL, 10);
	weight = simple_strtoul(argv[5], NULL, 10);
	scale = simple_strtoul(argv[6], NULL, 10);
	pos_x = simple_strtoul(argv[7], NULL, 10);
	pos_y = simple_strtoul(argv[8], NULL, 10);
	transparent = simple_strtoul(argv[9], NULL, 10);

//Data Stamp
	snx_set_ds_en(&vc->vc_enc[vc_enc_num], 0);

	if(!enable)
			return pdPASS;

	snx_set_ds_mode(&vc->vc_enc[vc_enc_num].snx_vc_ds, mode);
	snx_set_ds_weight(&vc->vc_enc[vc_enc_num].snx_vc_ds, weight);
	snx_set_ds_scale(&vc->vc_enc[vc_enc_num].snx_vc_ds, scale);
	snx_set_ds_pos(&vc->vc_enc[vc_enc_num].snx_vc_ds, pos_x, pos_y);
	snx_set_ds_transparent(&vc->vc_enc[vc_enc_num].snx_vc_ds, transparent);

	if(!strcmp(argv[10],"string")){	//string

		uint32_t *font_table;

		/*
		sscanf(DS_RED, "%u %u %u", vc->vc_enc[0].snx_vc_ds.t_color.color_R
			, vc->vc_enc[0].snx_vc_ds.t_color.color_G
			, vc->vc_enc[0].snx_vc_ds.t_color.color_B);

		sscanf(DS_YELLOW, "%u %u %u", vc->vc_enc[0].snx_vc_ds.b_color.color_R
			, vc->vc_enc[0].snx_vc_ds.b_color.color_G
			, vc->vc_enc[0].snx_vc_ds.b_color.color_B);
		 */

		snx_set_ds_string_color(&vc->vc_enc[vc_enc_num].snx_vc_ds, 0, 0, 0, 255, 255, 255);
		font_table = snx_get_ds_font_table(&vc->vc_enc[vc_enc_num].snx_vc_ds, NULL, NULL);
		if(font_table)
			snx_set_ds_string(&vc->vc_enc[vc_enc_num].snx_vc_ds, NULL, font_table);

	}else if(!strcmp(argv[10],"bmp")){	//bmp
		snx_set_ds_bmp_threshold(&vc->vc_enc[vc_enc_num].snx_vc_ds, bmp_thres, bmp_thres, bmp_thres);
		snx_set_ds_bmp_rounding(&vc->vc_enc[vc_enc_num].snx_vc_ds, bmp_rounding);
		vc->vc_enc[vc_enc_num].snx_vc_ds.ds_src = (unsigned char*)pvPortMalloc(3128, GFP_KERNEL, MODULE_CLI);
		memcpy(vc->vc_enc[vc_enc_num].snx_vc_ds.ds_src, &motion_person_bmp, 3128);
#if 0
		unsigned char *bmp_scale_buffer;
		bmp_scale_buffer = pvPortMalloc(3128*4, GFP_KERNEL, MODULE_CLI);
		snx_set_ds_bmp_scale_down(vc->vc_enc[vc_enc_num].snx_vc_ds.ds_src, bmp_scale_buffer);
		vPortFree( (void *)vc->vc_enc[vc_enc_num].snx_vc_ds.ds_src );
		vc->vc_enc[vc_enc_num].snx_vc_ds.ds_src = bmp_scale_buffer;
#endif
		snx_set_ds_bmp(&vc->vc_enc[vc_enc_num].snx_vc_ds);
		vPortFree( (void *)vc->vc_enc[vc_enc_num].snx_vc_ds.ds_src );
		vc->vc_enc[0].snx_vc_ds.ds_src = NULL;
	}else
		print_msg("src format is error\n");

	snx_set_ds_en(&vc->vc_enc[vc_enc_num], enable);
	////////////////////////////////////////////////////////////////////////////////
	return pdPASS;
}

int cmd_video_mroi_set(int argc, char* argv[])
{
	struct snx_m2m *m2m=0;
	int channel;
	uint8_t vc_enc_num;
	uint8_t region_num;
	uint8_t region_en;
	uint8_t weight;
	int8_t qp;
	uint32_t pos_x;
	uint32_t pos_y;
	uint32_t dim_x;
	uint32_t dim_y;
	uint8_t ext_size;

	if(argc != 12){
		printf(" usage: mroi_set (channel) (vc_enc#) (region#) (region_en) (weight) (qp) (pos_x) (pos_y) (dim_x) (dim_y) (ext_size)\n");
		return pdFAIL;
	}
	channel =simple_strtoul(argv[1], NULL, 10);
	if(channel == 0) {
		m2m = snx_m2m_channel0;
	}
	else if (channel == 1){
		m2m = snx_m2m_channel1;
	}
//	else
//		m2m= &m2m_task2;

	struct snx_vc *vc =  &m2m->vc;

	vc_enc_num =simple_strtoul(argv[2], NULL, 10);

	region_num = simple_strtoul(argv[3], NULL, 10);
	region_en = simple_strtoul(argv[4], NULL, 10);
	weight = simple_strtoul(argv[5], NULL, 10);
	qp = simple_strtol(argv[6], NULL, 10);
	pos_x = simple_strtoul(argv[7], NULL, 10);
	pos_y = simple_strtoul(argv[8], NULL, 10);
	dim_x = simple_strtoul(argv[9], NULL, 10);
	dim_y = simple_strtoul(argv[10], NULL, 10);
	ext_size = simple_strtoul(argv[11], NULL, 10);

//MROI
	snx_set_mroi_mode_en(&vc->vc_enc[vc_enc_num].snx_rc, 0);
	snx_set_mroi_region_en(&vc->vc_enc[vc_enc_num].snx_rc, region_num, region_en);
	snx_set_mroi_frame_size(&vc->vc_enc[vc_enc_num].snx_rc, m2m->width, m2m->height);
	snx_set_mroi_weight(&vc->vc_enc[vc_enc_num].snx_rc, weight, region_num);
	snx_set_mroi_qp(&vc->vc_enc[vc_enc_num].snx_rc, qp, region_num);
	snx_set_mroi_pos_dim(&vc->vc_enc[vc_enc_num].snx_rc, pos_x, pos_y, dim_x, dim_y, region_num);
	snx_set_mroi_ext(&vc->vc_enc[vc_enc_num].snx_rc, ext_size, region_num);
	snx_set_mroi_mode_en(&vc->vc_enc[vc_enc_num].snx_rc, 1);

	return pdPASS;
}
