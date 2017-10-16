#include <FreeRTOS.h>
#include <bsp.h>
#include <task.h>
#include <nonstdlib.h>
#include <cmd_verify.h>
#include <cmd_video.h>

#include <isp/isp.h>
#include <vc/snx_vc.h>
#include <libmid_vc/snx_mid_vc.h>

/** \defgroup cmd_verify_video Video test commands
 *  \ingroup cmd_verify
 * @{
 */

#define APP_VC_TASK 1
#define DEBUG 1

//#define printf(fmt, args...) if(DEBUG) print_msg((fmt), ##args)
#define printf(fmt, args...) if(DEBUG) print_msg_queue((fmt), ##args)


#include <libmid_fatfs/ff.h>
//FIL VideoFile;


static struct snx_app app[4]= { {"h264" , 0, FMT_H264, NULL ,},
								{"jpeg" , 0, FMT_MJPEG, NULL ,},
								{"djpeg", 1, FMT_H264, NULL ,},
								{"dh264", 1, FMT_MJPEG, NULL ,},};

extern struct snx_m2m *snx_m2m_channel0;
extern struct snx_m2m *snx_m2m_channel1;

struct snx_m2m m2m_task0={-1};
struct snx_m2m m2m_task1={-1};


#if APP_VC_TASK

void vTaskVC( void *pvParameters )
{
	struct snx_m2m *m2m =  (struct snx_m2m*)pvParameters;

	if(snx_video_get_mode(m2m, 0) && FMT_H264)
		m2m->h264 = xQueueCreate(10, sizeof(unsigned int));
	if(snx_video_get_mode(m2m, 0) && FMT_MJPEG)	
		m2m->jpeg = xQueueCreate(10, sizeof(unsigned int));
	if(snx_video_get_mode(m2m, 1) && FMT_H264)
		m2m->h264_dup = xQueueCreate(10, sizeof(unsigned int));
	if(snx_video_get_mode(m2m, 1) && FMT_MJPEG)
		m2m->jpeg_dup = xQueueCreate(10, sizeof(unsigned int));


	snx_video_start(m2m);
	m2m->start =1;

	while(1) {
		snx_video_read(m2m);
		if(snx_video_get_mode(m2m, 0) && FMT_H264)
			xQueueSendToBackFromISR(m2m->h264, (void*) &m2m->start, 0);
		if(snx_video_get_mode(m2m, 0) && FMT_MJPEG)
			xQueueSendToBackFromISR(m2m->jpeg, (void*) &m2m->start, 0);
		if(snx_video_get_mode(m2m, 1) && FMT_H264)
			xQueueSendToBackFromISR(m2m->h264_dup, (void*) &m2m->start, 0);
		if(snx_video_get_mode(m2m, 1) && FMT_MJPEG)
			xQueueSendToBackFromISR(m2m->jpeg_dup, (void*) &m2m->start, 0);
		if(	m2m->start == 0)
			break;
	}
	snx_video_stop(m2m);
	snx_video_uninit(m2m);
	vTaskDelete(NULL);
}
#endif

void vTaskRecord( void *pvParameters )
{
	int temp_buf_size = 3 * 1024 * 1024;
	struct snx_app *app =  (struct snx_app*)pvParameters;
	struct snx_m2m *m2m =  app->m2m;

	TickType_t tick_new = 0, tick = 0;
	int timer=0;
	int time_debug = 5;

	int bps=0, fps=0;
	int width=0, height=0, scale=0;
	int size;

	QueueHandle_t *recv;
	int ready;
	struct snx_temp snx_temp;
	char filename[40];
	//system_date_t t;

	memset(filename, 0, 40);

	memset(&snx_temp, 0x0, sizeof(struct snx_temp));

	 //Open file on SD
	 if(f_open(&(snx_temp.video_file), "Video.h264", FA_CREATE_ALWAYS | FA_WRITE) == FR_OK) {
		printf("Video file open ok\n");
	} else {
		printf("Video file open fail\n");
	}



	recv = snx_video_task_recv(m2m, app->mode, app->dup);

	snx_temp.total_size = 0;
	snx_temp.buf = snx_temp_alloc(temp_buf_size);
	snx_temp.ptr = (unsigned char *)snx_temp.buf;


	snx_video_get_resolution(m2m, &width, &height);
	scale = snx_video_get_scale(m2m);
	tick = xTaskGetTickCount();

	while(1) {

		if(*recv != NULL) {
	    	for ( ; ; ) {
	    		// The task is blocked until something appears in the queue
				if (xQueueReceive(*recv, (void*) &ready, portMAX_DELAY)) {
	    			break;
				}
				else {
					printf("<<test>><%s><%d> vc2file timeout\n", __func__, __LINE__);
				}
			}

			if(m2m->start == 0)
				break;

			if(ready == 0)
				break;

			size = snx_temp_write(app->mode, &snx_temp, m2m, temp_buf_size, app->dup);
#if 1
		tick_new = xTaskGetTickCount();
		timer = (tick_new - (tick))/configTICK_RATE_HZ;
		if(timer >= time_debug) { // update tick;
			tick = tick_new;
			if(bps != 0)
				printf(" (%dX%d) %s %d Kbps fps=%d qp=%d\n"
						, width>>((app->dup==0)?0:scale)
						, height>>((app->dup==0)?0:scale)
						, (app->mode==FMT_H264)?"H264":"MJPEG"
						, (bps >> 7)/time_debug
						, fps/time_debug
						, snx_video_get_qp(m2m, app->dup, (app->mode==FMT_H264)?FMT_H264:FMT_MJPEG));
				bps = 0;
				fps = 0;
		}

		if(size != 0) {
			fps++;
			bps += size;
		}
#else

		if(snx_video_is_keyframe(m2m, i>>1) == 1) {
				if(bps != 0)
					printf(" (%dX%d) %s %d Kbps fps=%d qp=%d\n"
						, width>>((app->dup==0)?0:scale)
						, height>>((app->dup==0)?0:scale)
						, (app->mode==FMT_H264)?"H264":"MJPEG"
						, (bps >> 7)/time_debug
						, fps/time_debug
						, snx_video_get_qp(m2m, app->dup, (app->mode==FMT_H264)?FMT_H264:FMT_MJPEG));
				bps = 0;
				fps = 0;
		}
		if(size != 0) {
			fps++;
			bps += size;
		}

#endif
		}
	}
	printf("dump to SD Video.h264 \n");
/*
	printf("dump binary memory Z:\\src\\freertos_prj\\test%d%d%d.%s 0x%x 0x%x\n"
			, m2m->channel
			, app->mode
			, app->dup
			, (app->mode==FMT_H264)?"h264":"jpg"
			, snx_temp.buf, snx_temp.ptr);
*/
//		printf("dump binary memory Z:\\src\\freertos_prj\\test.yuv 0x%x +115200\n"
//						, m2m->vc.mid);

//	f_close(&VideoFile);
	f_close(&(snx_temp.video_file));

	vPortFree( (void *)snx_temp.buf );
	vQueueDelete(*recv);
	vTaskDelete(NULL);

}

int cmd_verify_video_rec_start(int argc, char* argv[])
{
	struct snx_m2m *m2m;
	int channel = 0;
	int i = 0;

	fs_cmd_mount(0);

	if(argc != 2){
		printf(" Usage: recstart [channel] \n");
		printf(" channel: 0, 1 \n");

//		printf(" recstart 2 ( channel 2 start)(dummy sensor for test)\n");
		return pdFAIL;
	}

	channel =simple_strtoul(argv[1], NULL, 10);

	if(channel == 0) {
		m2m= &m2m_task0;
		snx_m2m_channel0 = m2m;
	}
	else if (channel == 1){
		m2m= &m2m_task1;
		snx_m2m_channel1 = m2m;
	}
//	else
//		m2m= &m2m_task2;



	if(m2m->channel == -1) {
		snx_video_init(m2m);
		snx_channel_set_vc(m2m, channel);
	}


#if APP_VC_TASK
	if (pdPASS != xTaskCreate(vTaskVC, "vc", 1024, (void*) m2m,
							PRIORITY_TASK_APP_TEST01, NULL))
		printf("<<test>><%s><%d> Could not create task\n", __func__, __LINE__);
#else
	snx_video_task_start(m2m);
#endif

	for(i = 0;i < 4;i++) {
		app[i].m2m = m2m;
		if(app[i].mode & snx_video_get_mode(m2m, app[i].dup)) {
			if (pdPASS != xTaskCreate(vTaskRecord, app[i].name, 2048, (void*) &app[i],
					PRIORITY_TASK_MID_VIDEO, NULL))
				printf("<<test>><%s><%d> Could not create task\n", __func__, __LINE__);
		}
	}

	return pdPASS;
}

int cmd_verify_video_rec_stop(int argc, char* argv[])
{
	struct snx_m2m *m2m;
	int channel;
	if(argc != 2){
		printf(" Usage: recstop [channel] \n");
		printf(" channel: 0, 1 \n");
		return pdFAIL;
	}

	channel =simple_strtoul(argv[1], NULL, 10);
	if(channel == 0) {
		m2m= &m2m_task0;
	}
	else if (channel == 1){
		m2m= &m2m_task1;
	}
//	else
//		m2m= &m2m_task2;

#if APP_VC_TASK
	m2m->start =0;
#else
	snx_video_task_stop(m2m);
#endif
	printf(" (channel %d stop)\n", channel);
	return pdPASS;

}
/** @} */
