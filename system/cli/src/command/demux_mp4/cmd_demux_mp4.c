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
#include "mp4.h"

char filename[64];

static void demux_mp4(void *pvParameters){
	//char *filename = (char *)pvParameters;

	MP4_read_Info_t pMp4ReadInfo = {0};
	int ret;
	void *data;
	uint32_t size = 1;
	uint32_t write_size;
	uint8_t frm_type;
	FIL videofile;
	FIL audiofile;
	char codec[32];
	int width; int height;int simplerate;
	int num = 0;
	sprintf(pMp4ReadInfo.Filename, "%s", filename);
	print_msg("File %s will be demuxed\n",filename);					
	if(f_open(&videofile, "xxx.h264", FA_WRITE|FA_CREATE_ALWAYS) != FR_OK)
	{
		print_msg("open video file fail\n");
		goto fail;
	}
	if(f_open(&audiofile, "xxx.aac", FA_WRITE|FA_CREATE_ALWAYS) != FR_OK)
	{
		print_msg("open audio file fail\n");
		goto fail;
	}
	ret = mp4_read_init(&pMp4ReadInfo);
	if(ret == pdFAIL){
		print_msg("Init file  %s fail\n", filename);
		goto fail;
	}
	mp4_seek_frame(&pMp4ReadInfo, 50);
	print_msg( "Init read mp4 file %d\n",ret);
	while(size != 0){
		mp4_read_frame(&pMp4ReadInfo, &data, &size, &frm_type);
		if(frm_type == RETURN_MP4_NONE){
			break;
		}else if(frm_type == RETURN_MP4_VIDEO){
			f_write(&videofile, data, size, &write_size);
			if(size != write_size){
				print_msg("write bin to sd fail........... size=%d  %d.\n", size, write_size);
			}
		}else if(frm_type == RETURN_MP4_AUDIO){
			f_write(&audiofile, data, size, &write_size);
			if(size != write_size){
				print_msg("write bin to sd fail........... size=%d  %d.\n", size, write_size);
			}
		}
	}

	mp4_seek_frame(&pMp4ReadInfo, 40);
	print_msg( "Init read mp4 file %d\n",ret);
	size = 1;
	while(size != 0){
		mp4_read_frame(&pMp4ReadInfo, &data, &size, &frm_type);
		if(frm_type == RETURN_MP4_NONE){
			break;
		}else if(frm_type == RETURN_MP4_VIDEO){
			f_write(&videofile, data, size, &write_size);
			if(size != write_size){
				print_msg("write bin to sd fail........... size=%d  %d.\n", size, write_size);
			}
			num ++;
			if(num == 300)
				break;
		}else if(frm_type == RETURN_MP4_AUDIO){
			f_write(&audiofile, data, size, &write_size);
			if(size != write_size){
				print_msg("write bin to sd fail........... size=%d  %d.\n", size, write_size);
			}
		}
	}
	f_close(&videofile);
	f_close(&audiofile);

	ret = mp4_read_video_codec(&pMp4ReadInfo, codec);
	if(ret == pdFAIL){
		print_msg("Read video codec fail\n");
		goto fail;
	}
	print_msg("  codec %s %d\n",codec,ret);
	mp4_read_video_resolution(&pMp4ReadInfo,&width,&height);
	if(ret == pdFAIL){
		print_msg("Read video resolution fail\n");
		goto fail;
	}
	print_msg("  width %d height %d\n",width, height);
	ret = mp4_read_audio_codec(&pMp4ReadInfo, codec);
	if(ret == pdFAIL){
		print_msg("Read audio codec fail\n");
		goto fail;
	}
	print_msg("  codec %s %d\n",codec,ret);
	mp4_read_audio_simplerate(&pMp4ReadInfo, &simplerate);
	if(ret == pdFAIL){
		print_msg("Read audio simplerate fail\n");
		goto fail;
	}
	print_msg("  simplerate %d \n",simplerate);
	ret = mp4_read_uninit(&pMp4ReadInfo);
	if(ret == pdFAIL){
		print_msg("Uninit file  %s fail\n", filename);
		goto fail;
	}
	print_msg("demux over 1\n");

fail:
	vTaskDelete(NULL);;

}

int cmd_demux_mp4_start(int argc, char* argv[])
{ 

	if(argc != 2){
		print_msg(" demux filename ( task demux start)\n");
		return pdFAIL;
	}
 	
	
 	sprintf(filename, "%s", argv[1]);
	//print_msg("filename is %s,%d\n",filename,strlen(filename));
	if(pdPASS != xTaskCreate(demux_mp4, "demux_mp4", 2048 * 4, NULL,
						PRIORITY_TASK_APP_TEST01, NULL))
		print_msg("<<test>><%s><%d> Could not create task\n", __func__, __LINE__);
 


	return pdPASS;
}


