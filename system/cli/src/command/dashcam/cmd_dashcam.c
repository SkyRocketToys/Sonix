#include <FreeRTOS.h>
#include <bsp.h>
#include <stdio.h>
#include <stdlib.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include <isp/isp.h>
#include "printlog.h"
#include <nonstdlib.h>
#include <string.h>
#include <libmid_fatfs/ff.h>
#include <sys_clock.h>
#include <cmd_dashcam.h>
//#include <libmid_nvram/snx_mid_nvram.h>


#include <video/video_main.h>
#include <video/snapshot.h>
#include <audio/audio_main.h>
#include <main_flow.h>
#include <../main_flow/errno.h>
#include <../usbd_ext/snx_json_usbd.h>
#include <utility.h>

#define DEBUG 1
//#define printf(fmt, args...) if(DEBUG) print_msg((fmt), ##args)
#define printf(fmt, args...) if(DEBUG) print_msg_queue((fmt), ##args)


extern video_info_t ispch0_info, ispch1_info;
extern audio_info_t audio_info;


static void cmd_auto_test(void *pvParameters)
{
	int resolution=0;

	while(1) {
		if(resolution==0){
			snx_usbd_create_json_set_video_parameters_cmd((512*1024), resolution, 15);

			resolution=1;
		}
		else {
			snx_usbd_create_json_set_video_parameters_cmd((1*1024*1024), 1, 30);
			resolution=0;
		}

		vTaskDelay( 10000 / portTICK_RATE_MS);
	}
}

int cmd_dashcam_resolution_set(int argc, char* argv[])
{
	if (argc == 2) {

		if (atoi(argv[1]) == 0)
			snx_usbd_create_json_set_video_parameters_cmd((512*1024), 0, 15);
		// VGA:0 HD:1 FHD:2
		else if (atoi(argv[1]) == 1)
			snx_usbd_create_json_set_video_parameters_cmd((1*1024*1024), 1, 30);
		else if (atoi(argv[1]) == 2)
			if (pdPASS != xTaskCreate(cmd_auto_test, "cmd_auto_test", STACK_SIZE_8K, NULL, 40, NULL))
			{
				print_msg_queue("Fail test\n");
			}

	}
	else {
		print_msg_queue("\n");
		print_msg_queue("Usage: res [Option]\n");
		print_msg_queue("Available options are\n");
		print_msg_queue("0:  Set VGA resolution.\n");
		print_msg_queue("1:  Set HD resolution.\n");
		print_msg_queue("2:  Switch Test Mode.\n");
	}
	
	return pdPASS;
}


static char mode_info[4][12]= {	"NULL",
								"H264",
								"MJPEG",
								"H264/MJPEG"};


int cmd_dashcam_info_get(int argc, char* argv[])
{
	int sec_interval = 0;
	
	print_msg_queue("Channel0(%dX%d)\t %dfps %dKbps GOP:%d mode:dup_mode=%s %s\n"
						, ispch0_info.width
						, ispch0_info.height
						, ispch0_info.ucFps
						, ispch0_info.uiBps>>10
						, ispch0_info.ucGop
						, mode_info[ispch0_info.ucStreamMode>>1]
						, mode_info[ispch0_info.ucStreamModeusedup1>>1]
						,chk_preview_use_isp0dup()==1?"preivew":""
						);

	print_msg_queue("Channel1(%dX%d)\t %dfps %dKbps GOP:%d mode:dup_mode=%s %s\n"
						, ispch1_info.width
						, ispch1_info.height
						, ispch1_info.ucFps
						, ispch1_info.uiBps>>10
						, ispch1_info.ucGop
						, mode_info[ispch1_info.ucStreamMode>>1]
						, mode_info[ispch1_info.ucStreamModeusedup1>>1]
						,chk_preview_use_isp0dup()==0?"preivew":""
						);
	if (argc == 2) {
		sec_interval = atoi(argv[1]) * 1000;
		ispch0_info.time_interval = sec_interval;
		ispch1_info.time_interval = sec_interval;
		audio_info.time_interval = sec_interval;
	}
/*	
	if (argc == 2) {
		if (atoi(argv[1]) == 0)
			dashcam_preview_info();
		else if (atoi(argv[1]) == 1)
			dashcam_record_info();
	}
	else {
		print_msg_queue("\n");
		print_msg_queue("Usage: info [time_interval]\n");
		print_msg_queue("Available options are\n");
		print_msg_queue("0:  Show preveiw info.\n");
		print_msg_queue("1:  Show record info.\n");
		print_msg_queue("2:  Show ----- info.\n");
	}
*/
	return pdPASS;
}

int cmd_dashcam_start(int argc, char* argv[])
{
	main_flow_init();
	return pdPASS;
}

int cmd_dashcam_stop(int argc, char* argv[])
{
	all_task_uinit(TASK_KEEP_USBD);
	return pdPASS;
}

int cmd_dashcam_snapshot(int argc, char* argv[])
{
	int ret = 0;
	int pic_num = 0;
	int queue_num = 0;
	int sdstate = 0;

	if (argc == 2) {
		pic_num = atoi(argv[1]) ;
		if(pic_num > 5){
			print_msg_queue("ERROR: wrong number\n");
			goto fail;
		}
			
		sdstate = app_uti_chk_sd_state();
		if(sdstate != SD_OK) {
			print_msg_queue("ERROR: sd is not ready\n");
			return 0;
		}

		if(mf_snapshot_get_isfull() == 1) {
			print_msg_queue("ERROR: photo folder is full.\n");
			return 0;
		}

		if (check_takepic_task() != 0) {
			print_msg_queue("ERROR: Continuous shooting is already running.\n");
			return 0;
		}

		if ((chk_record_is_running() == 0)
#ifndef CONFIG_APP_DRONE
	        || (chk_protect_is_running() == 0)
	        || (chk_lapse_is_running() == 0)
#endif
	        || (chk_isp0_is_running() == 0)
	        || (chk_isp1_is_running() == 0) ) {
			print_msg_queue("ERROR: ALL Task have not init success\n");
			return 0;
		}

		queue_num = mf_snapshot_get_queue_num();
		if ( (queue_num + pic_num) > SHAPSHOT_MAX_QUEUE_ITEM) {
			print_msg_queue("ERROR: Queue number is full.\n");
			
		}
		ret =  app_uti_take_picture(pic_num);
		if(ret == OK){
			print_msg_queue(" SNAPSHOT SUCCESS!! \n");
			return 0;
		}

	}

fail:
		print_msg_queue("\n");
		print_msg_queue("Usage: snapshot [num]\n");
		print_msg_queue("num can be from 1 to 5\n");
		return 0;
}

int cmd_dashcam_preview_audio(int argc, char* argv[])
{
	int status = 0; 
	if (argc == 2) {
		status = atoi(argv[1]) ;
		if( status != PREVIEW_AUDIO_ON && status != PREVIEW_AUDIO_OFF ){
			print_msg_queue("ERROR: wrong setting\n");
			goto fail;
		}

		if(status == app_uti_get_preview_audio_mode()){
			print_msg_queue("Setting is the same\n");
			return 0;
		}
			
		if((chk_record_is_running() == 0)
#ifndef CONFIG_APP_DRONE
	        || (chk_protect_is_running() == 0)
	        || (chk_lapse_is_running() == 0)
#endif
	        || (chk_isp0_is_running() == 0)
	        || (chk_isp1_is_running() == 0)) {
			if (chk_record_is_running() == 0)
				print_msg_queue("Record Task have not init success\n");
			if (chk_isp0_is_running() == 0)
				print_msg_queue("ISP0 Task have not init success\n");
			if (chk_isp1_is_running() == 0)
				print_msg_queue("ISP1 Task have not init success\n");
#ifndef CONFIG_APP_DRONE
			if (chk_protect_is_running() == 0)
				print_msg_queue("Protect Task have not init success\n");
			if (chk_lapse_is_running() == 0)
				print_msg_queue("Timelapse Task have not init success\n");
#endif
			return -1;
		}

		destroy_rtsp_server();

		if ((chk_preview_use_isp0dup() == 1) ) {	//resolution change
			set_isp0dup_closeflag();
			set_isp1_closeflag();
		} else if((chk_preview_use_isp0dup() == 0)) {	//resolution change
			set_isp1_closeflag();
		}else{
			print_msg_queue("ERROR: something wrong\n");
			return -1;
		}
			
		app_uti_set_preview_audio_mode(status);
		init_rtsp_server();
		return 0;
	}
fail:
		print_msg_queue("\n");
		print_msg_queue("Usage: prevaudio [flag]\n");
		print_msg_queue("flag =1 means turn on preview audio\n");
		print_msg_queue("flag =0 means turn off preview audio\n");
		return 0;
	
}
