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

#include <libmid_rtsp_server/rtsp_server.h>

/*******************************************test********************************************************/
#include <FreeRTOS.h>
#include <task.h>
#include <string.h>
#include "lwip/sockets.h"
/*#include "lwip/err.h"
#include "lwip/netif.h"
#include "lwipApps.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/dhcp.h"
#include "lwip/ip_addr.h"
#include "lwip/netdb.h"*/
//#include <cyassl/ssl.h>

/* Global */

#define BACKLOG 5     // how many pending connections queue will hold

#define BUF_SIZE 200
#define MYPORT 554
#define TCP_TEST_TASK_PRO 55
#define PREVIEW_AUDIO_TEST 0

static xSemaphoreHandle tcp_mutex[BACKLOG];
static char message[1404]="WayneTsai TCP Test\n";
static int test_fd[BACKLOG];			
void tcp_send_test_task1(int *fd){
	int ret;
	int send_fd = *fd;
	while(1){
		xSemaphoreTake(tcp_mutex[0], portMAX_DELAY);
		ret = send(send_fd , message , sizeof(message), 0);
             xSemaphoreGive(tcp_mutex[0]);
		if(ret  < 0)
		{
			print_msg("Send failed: %d\n",ret );
		}else if(ret != sizeof(message)){
			print_msg("Send not all : %d-%d\n",ret ,sizeof(message));
		}
		
		xSemaphoreTake(tcp_mutex[0], portMAX_DELAY);
		ret = send(send_fd , message , sizeof(message), 0);
             xSemaphoreGive(tcp_mutex[0]);
		if(ret  < 0)
		{
			print_msg("Send failed: %d\n",ret );
		}else if(ret != sizeof(message)){
			print_msg("Send not all : %d-%d\n",ret ,sizeof(message));
		}

		xSemaphoreTake(tcp_mutex[0], portMAX_DELAY);
		ret = send(send_fd , message , sizeof(message), 0);
             xSemaphoreGive(tcp_mutex[0]);
		if(ret  < 0)
		{
			print_msg("Send failed: %d\n",ret );
		}else if(ret != sizeof(message)){
			print_msg("Send not all : %d-%d\n",ret ,sizeof(message));
		}

		xSemaphoreTake(tcp_mutex[0], portMAX_DELAY);
		ret = send(send_fd , message , sizeof(message), 0);
             xSemaphoreGive(tcp_mutex[0]);
		if(ret  < 0)
		{
			print_msg("Send failed: %d\n",ret );
		}else if(ret != sizeof(message)){
			print_msg("Send not all : %d-%d\n",ret ,sizeof(message));
		}

		xSemaphoreTake(tcp_mutex[0], portMAX_DELAY);
		ret = send(send_fd , message , sizeof(message), 0);
             xSemaphoreGive(tcp_mutex[0]);
		if(ret  < 0)
		{
			print_msg("Send failed: %d\n",ret );
		}else if(ret != sizeof(message)){
			print_msg("Send not all : %d-%d\n",ret ,sizeof(message));
		}

		xSemaphoreTake(tcp_mutex[0], portMAX_DELAY);
		ret = send(send_fd , message , sizeof(message), 0);
             xSemaphoreGive(tcp_mutex[0]);
		if(ret  < 0)
		{
			print_msg("Send failed: %d\n",ret );
		}else if(ret != sizeof(message)){
			print_msg("Send not all : %d-%d\n",ret ,sizeof(message));
		}

		xSemaphoreTake(tcp_mutex[0], portMAX_DELAY);
		ret = send(send_fd , message , sizeof(message), 0);
             xSemaphoreGive(tcp_mutex[0]);
		if(ret  < 0)
		{
			print_msg("Send failed: %d\n",ret );
		}else if(ret != sizeof(message)){
			print_msg("Send not all : %d-%d\n",ret ,sizeof(message));
		}

		xSemaphoreTake(tcp_mutex[0], portMAX_DELAY);
		ret = send(send_fd , message , sizeof(message), 0);
             xSemaphoreGive(tcp_mutex[0]);
		if(ret  < 0)
		{
			print_msg("Send failed: %d\n",ret );
		}else if(ret != sizeof(message)){
			print_msg("Send not all : %d-%d\n",ret ,sizeof(message));
		}

		xSemaphoreTake(tcp_mutex[0], portMAX_DELAY);
		ret = send(send_fd , message , 728, 0);
             xSemaphoreGive(tcp_mutex[0]);
		if(ret  < 0)
		{
			print_msg("Send failed: %d\n",ret );
		}else if(ret != 728){
			print_msg("Send not all : %d-%d\n",ret ,sizeof(message));
		}

		vTaskDelay(67 / portTICK_RATE_MS);

		
	}
}

void tcp_send_test_task2(int *fd){
	int ret;
	int send_fd = *fd;
	while(1){
		xSemaphoreTake(tcp_mutex[1], portMAX_DELAY);
		ret = send(send_fd , message , sizeof(message), 0);
		xSemaphoreGive(tcp_mutex[1]);
		if(ret  < 0)
		{
			print_msg("Send failed: %d\n",ret );
		}else if(ret != sizeof(message)){
			print_msg("Send not all : %d-%d\n",ret ,sizeof(message));
		}
		vTaskDelay(67 / portTICK_RATE_MS);
	}
}

void tcp_send_test_task3(int *fd){
	int ret;
	int send_fd = *fd;
	while(1){
		xSemaphoreTake(tcp_mutex[2], portMAX_DELAY);
		ret = send(send_fd , message , sizeof(message), 0);
		xSemaphoreGive(tcp_mutex[2]);
		if(ret  < 0)
		{
			print_msg("Send failed: %d\n",ret );
		}else if(ret != sizeof(message)){
			print_msg("Send not all : %d-%d\n",ret ,sizeof(message));
		}
		vTaskDelay(67 / portTICK_RATE_MS);
	}
}

void tcp_selected_test_task() {

      int sock_fd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct sockaddr_in server_addr;    // server address information
    struct sockaddr_in client_addr; // connector's address information
    socklen_t sin_size;
//    int yes = 1;
    char buf[BUF_SIZE];
	char *ptr;
    int ret;
    int i;
    fd_set fdsr;
    int maxsock;
    struct timeval tv;

    int fd_A[BACKLOG];    // accepted connection fd
    int conn_amount;    // current connection amount
	
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        print_msg("socket error\n");
        return;
    }

    
    server_addr.sin_family = AF_INET;         // host byte order
    server_addr.sin_port = htons(MYPORT);     // short, network byte order
    server_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
    memset(server_addr.sin_zero, '\0', sizeof(server_addr.sin_zero));

    if (bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        print_msg("bind error\n");
        return;
    }

    if (listen(sock_fd, BACKLOG) == -1) {
        print_msg("listen error\n");
        return;
    }

    print_msg("listen port %d\n", MYPORT);

    for(i = 0; i < BACKLOG; i++)
		tcp_mutex[i] = xSemaphoreCreateMutex();
print_msg("----------%d\n", __LINE__);
    conn_amount = 0;
    sin_size = sizeof(client_addr);
    maxsock = sock_fd;
    ptr = buf;
    while (1) {
        // initialize file descriptor set
        FD_ZERO(&fdsr);
        FD_SET(sock_fd, &fdsr);
        // timeout setting
        tv.tv_sec = 2;
        tv.tv_usec = 0;

        // add active connection to fd set
        for (i = 0; i < BACKLOG; i++) {
            if (fd_A[i] != 0) {
                FD_SET(fd_A[i], &fdsr);
            }
        }
print_msg("----------%d\n", __LINE__);
        ret = select(maxsock + 1, &fdsr, NULL, NULL, &tv);
        if (ret < 0) {
            print_msg("select error");
            break;
        } else if (ret == 0) {
            print_msg("timeout\n");
            continue;
        }
print_msg("----------%d\n", __LINE__);
        // check every fd in the set
        for (i = 0; i < conn_amount; i++) {
            if (FD_ISSET(fd_A[i], &fdsr)) {               
redo:	
                xSemaphoreTake(tcp_mutex[i], portMAX_DELAY);
                ret = recv(fd_A[i], ptr, 1, 0);//print_msg("receive ret is %d                ",ret);
                xSemaphoreGive(tcp_mutex[i]);
                if (ret < 0) {        // client close
                    print_msg("*******client[%d] receive -1:no data\n", i);
                    //close(fd_A[i]);
                    //FD_CLR(fd_A[i], &fdsr);
                    //fd_A[i] = 0;
                }else if(ret > 0){
			print_msg("%d  ",buf[ptr -buf]);
			ptr ++;
			if(ptr -buf >= BUF_SIZE){
				print_msg("----------over  ");
		             ptr = buf;
		      } 
			goto redo;
                }else if(ret == 0){
                	print_msg("***client[%d] receive 0\n", i);
                }
			
            }
        }
        // check whether a new connection comes
        if (FD_ISSET(sock_fd, &fdsr)) { print_msg("fd_isset OK\n");
            new_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &sin_size);
            if (new_fd <= 0) {
                print_msg("accept error\n");
                continue;
            }
		fcntl(new_fd, F_SETFL, fcntl(new_fd, F_GETFL, 0) | O_NONBLOCK);
		print_msg("accept OK\n");
            // add to fd queue
            if (conn_amount < BACKLOG) {print_msg("create new client service\n");
                fd_A[conn_amount++] = new_fd;
             /*   if(conn_amount == 1){
                    if (pdPASS != xTaskCreate(tcp_send_test_task1, "rtsp_sending1", 4096, &new_fd,
					TCP_TEST_TASK_PRO, NULL))
				print_msg("<<test>><%s><%d> Could not create task\n", __func__, __LINE__);
                }else if(conn_amount == 2){
                    if (pdPASS != xTaskCreate(tcp_send_test_task2, "rtsp_sending2", 4096, &new_fd,
					TCP_TEST_TASK_PRO, NULL))
				print_msg("<<test>><%s><%d> Could not create task\n", __func__, __LINE__);
                }else if(conn_amount == 3){
                    if (pdPASS != xTaskCreate(tcp_send_test_task3, "rtsp_sending3", 4096, &new_fd,
					TCP_TEST_TASK_PRO, NULL))
				print_msg("<<test>><%s><%d> Could not create task\n", __func__, __LINE__);
                }*/
                test_fd[conn_amount - 1] = new_fd;
                print_msg("new connection client[%d] %s:%d\n", conn_amount,
                        inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                if (new_fd > maxsock)
                    maxsock = new_fd;
            }else {
                print_msg("max connections arrive, exit\n");
                send(new_fd, "bye", 4, 0);
                close(new_fd);
                break;
            }
        }
    }

}

/*******************************************test********************************************************/

#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AAC
/***************************************************aac*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <audio/audio_dri.h>
#include <audio/audio_error.h>
#include <libmid_audio/audio.h>
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

struct aac_info_st
{
	unsigned int aot;
	unsigned int eld_sbr;
	unsigned int sample_rate;
	unsigned int vbr;
	unsigned int bitrate;
	unsigned int high_quality;
	AACENC_InfoStruct enc_info;
};

void usage(void)
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

static const char *aac_get_error(AACENC_ERROR err)
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


int aac_encoder_init_1(HANDLE_AACENCODER *phandle, struct aac_info_st *paac_info)
{
	HANDLE_AACENCODER handle;
	AACENC_ERROR err;
	int retval = 0;

	err = aacEncOpen(&handle, 0x01|0x02|0x10, CHANNEL);
	if(err != AACENC_OK)
	{
		print_msg( "Unable to open the encoder: %s\n", aac_get_error(err));
		retval = -11;
		goto out;
	}

	err = aacEncoder_SetParam(handle, AACENC_AOT, paac_info->aot);
	if(err != AACENC_OK)
	{
		print_msg( "Unable to set the AOT %d: %s\n", paac_info->aot, aac_get_error(err));
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
			print_msg( "Unable to enable SBR for ELD: %s\n", aac_get_error(err));
			retval = -13;
			goto err;
		}
	}

	err = aacEncoder_SetParam(handle, AACENC_SAMPLERATE, paac_info->sample_rate);
	if(err != AACENC_OK)
	{
		print_msg( "Unable to set the sample rate %d: %s\n", paac_info->sample_rate, aac_get_error(err));
		retval = -14;
		goto err;
	}

	err = aacEncoder_SetParam(handle, AACENC_CHANNELMODE, CHANNEL_MODE);
	if(err != AACENC_OK)
	{
		print_msg( "Unable to set channel mode %d: %s\n", CHANNEL_MODE, aac_get_error(err));
		retval = -15;
		goto err;
	}

	err = aacEncoder_SetParam(handle, AACENC_CHANNELORDER, CHANNEL_ORDER);
	if(err != AACENC_OK)
	{
		print_msg( "Unable to set wav channel order %d: %s\n", CHANNEL_ORDER, aac_get_error(err));
		retval = -16;
		goto err;
	}

	if(paac_info->vbr)
	{
		err = aacEncoder_SetParam(handle, AACENC_BITRATEMODE, paac_info->vbr);
		if(err != AACENC_OK)
		{
			print_msg( "Unable to set the VBR bitrate mode %d: %s\n", paac_info->vbr, aac_get_error(err));
			retval = -17;
			goto err;
		}
	}
	else
	{
		err = aacEncoder_SetParam(handle, AACENC_BITRATE, paac_info->bitrate);
		if(err != AACENC_OK)
		{
			print_msg( "Unable to set the bitrate %d: %s\n", paac_info->bitrate, aac_get_error(err));
			retval = -18;
			goto err;
		}
	}

	err = aacEncoder_SetParam(handle, AACENC_TRANSMUX, TRANSPORT_TYPE);
	if(err != AACENC_OK)
	{
		print_msg( "Unable to set the transmux format: %s\n", aac_get_error(err));
		retval = -19;
		goto err;
	}

	err = aacEncoder_SetParam(handle, AACENC_AFTERBURNER, paac_info->high_quality);
	if(err != AACENC_OK)
	{
		print_msg( "Unable to set afterburner to %d: %s\n", paac_info->high_quality, aac_get_error(err));
		retval = -21;
		goto err;
	}

	err = aacEncEncode(handle, NULL, NULL, NULL, NULL);
	if(err != AACENC_OK)
	{
		print_msg( "Unable to initialize the encoder: %s\n", aac_get_error(err));
		retval = -23;
		goto err;
	}

	err = aacEncInfo(handle, &(paac_info->enc_info));
	if(err != AACENC_OK)
	{
		print_msg( "Unable to get encoder info: %s\n", aac_get_error(err));
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

struct audio_stream_st *audio_device_init( unsigned int sample_rate, int cap_format)
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

//out:
//	return NULL;

cmd_err:
	print_msg("init audio device error.\n");
	return NULL;
}

int run_aac()
{
	struct audio_stream_st *audio_stream;
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
	struct aac_info_st aac_info;

//	int args = 1;
	int is_file_mode = 0;
	unsigned int cap_time, cap_frames;
//	char *infile_name, *outfile_name;
	FIL out_fp;

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
		retval = audio_device_init(aac_info.sample_rate, &pcm);
		if(retval < 0)
			goto end;
	}*/

	retval = aac_encoder_init_1(&handle, &aac_info);
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
		audio_stream = audio_device_init(aac_info.sample_rate, AUD_FORMAT_S16_LE);
		if(audio_stream == NULL)
			goto end;
	}


	print_msg("start capture\n");
	cap_frames = 0;
	remain_frame = 0;
	while(1)
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
			print_msg( "Unable to encode frame: %s\n", aac_get_error(err));
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
//			int write_size;
			print_msg("Encoder over. aac size is %d\n",out_args.numOutBytes);
			send_rtp_data(3, aac_buf, out_args.numOutBytes, NULL, PREVIEW_STREAM);
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

	return retval;

//cmd_err:
//	print_msg( "args error\n");
//	usage();
//	return retval;

}
/***************************************************aac*********************************************************************/
#endif //if AAC defined




/*******************************audio*************************************/
#include <unistd.h>
#include <time.h>
#include "libmid_audio/audio.h"

int run_audio()
{
	int32_t retval = 0;
	int32_t type, format, format_size, status, is_block;
	uint32_t rate, buf_size, buf_threshold;
	struct audio_stream_st *audio_stream;
	struct params_st *params;
	uint8_t *buf, *pbuf;
	uint32_t size,size2;
//	int32_t i;

	print_msg("\naudio test task run.\n");

	
	type = AUD_CAP_STREAM;
	print_msg("Capture\n");
	

	format = AUD_FORMAT_A_LAW;
	format_size = 2;
	
	rate = 8000;
	format_size = 1;
	buf_size = 8192;
	buf_threshold = 256;	
	is_block = 1;

	//size = 5 * rate * format_size;
	size2 = size = 800;
	buf = (uint8_t *)pvPortMalloc(size, GFP_KERNEL, MODULE_CLI);
	if(buf == NULL)
	{
		print_msg("Failed to allocate memory. --- %d\n", size);
		retval = -2;
		goto out;
	}
	print_msg("Buffer address:0x%08x\n", buf);
	print_msg("Buffer size:0x%08x\n", size);
	

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
	while(1){
		pbuf = buf;
		size = 800;
		while(size > 0)
		{
			if(type == AUD_CAP_STREAM)
				retval = audio_read(audio_stream, pbuf, size, is_block);
			
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
					print_msg("-----------------------retval:%d Bytes\n", retval);
					vTaskDelay(50 / portTICK_RATE_MS);
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
		send_rtp_data(3, buf, size2, NULL, PREVIEW_STREAM);
/*		xSemaphoreTake(tcp_mutex[0], portMAX_DELAY);
		ret = send(test_fd[0] , audio->audio_buf, audio->audio_buf_len, 0);
	      xSemaphoreGive(tcp_mutex[0]);
		if(ret  < 0)
		{
			print_msg("Send failed: %d\n",ret );
		}else if(ret != audio->audio_buf_len){
			print_msg("Send not all : %d-%d\n",ret ,audio->audio_buf_len);
		}*/
}
err3:
	audio_drain(audio_stream);
err2:
	audio_close(audio_stream);
err1:
	vPortFree(buf);
out:
	return retval;

//cmd_err:
//	print_msg("Command error.\n");
//	return -1;
}


/*******************************audio**************************************************************************************************/


#define APP_VC_TASK 0

#define DS5_DEBUG 1
#define DEBUG 1
//#define print_msg(fmt, args...) if(DEBUG) print_msg((fmt), ##args)
#define print_msg(fmt, args...) if(DEBUG) print_msg_queue((fmt), ##args)

FIL MyFile;


struct snx_temp1 {
	int total_size;
	unsigned char *ptr;
	unsigned char *buf;	
};

struct snx_app1 {
	char name[16];
	int dup;
	int mode;
	struct snx_m2m *m2m;
};

static struct snx_app1 app[4]= { {"h264" , 0, FMT_H264, NULL ,},
								{"jpeg" , 0, FMT_MJPEG, NULL ,},
								{"djpeg", 1, FMT_H264, NULL ,},
								{"dh264", 1, FMT_MJPEG, NULL ,},};

static struct snx_m2m m2m_task0={-1};
static struct snx_m2m m2m_task1={-1};
static struct snx_m2m m2m_task2={-1};

unsigned char *snx_temp_alloc1(int temp_buf_size)
{
	unsigned char *buf = 0;
	buf = (unsigned char*)pvPortMalloc(temp_buf_size, GFP_KERNEL, MODULE_CLI);
	if ( 0 == buf ) {
		print_msg("<%s>pvPortMallo fail 0x%x\n",__func__, buf);
		return pdFAIL;
	}
//	memset((unsigned *)buf, 0x0, temp_buf_size);
	return buf;	
}


int snx_temp_write1(int mode
							, struct snx_temp1 *snx_temp1
							, struct snx_m2m *m2m
							, int temp_buf_size
							, int dup)
{
	int bs_size = 0;
	unsigned char *ptr = NULL;
//	int wbytes = 0;
//	int len, ret, tmp1, tmp2;
//	unsigned char *tmp_ptr = NULL;
	if(mode & FMT_H264) {
		bs_size = snx_video_h264_stream(m2m, dup, (unsigned int *)&ptr);
	}
	else if	(mode & FMT_MJPEG) {
		bs_size = snx_video_jpeg_stream(m2m, dup, (unsigned int *)&ptr);
	}

	//print_msg("bs_size = %d\n", bs_size);

	if((snx_temp1->total_size + bs_size) < temp_buf_size) {
	//	if (f_write(&MyFile, (unsigned char*)(ptr), bs_size, (void *)&wbytes) != FR_OK) {
			//print_msg("video save to sd error!!!, cnt = %d\n", wbytes);
	//	}
		send_rtp_data(1, ptr, bs_size, NULL, PREVIEW_STREAM);
/*		len = bs_size;
		tmp_ptr = ptr;
		tmp1 = 1404;
		while(len > 0){
			tmp2 = (tmp1 < len)?tmp1:len;
			xSemaphoreTake(tcp_mutex[0], portMAX_DELAY);
			ret = send(test_fd[0] , tmp_ptr  , tmp2, 0);
	             xSemaphoreGive(tcp_mutex[0]);
			if(ret  < 0)
			{
				print_msg("Send failed: %d\n",ret );
			}else if(ret != tmp2){
				print_msg("Send not all : %d-%d\n",ret ,tmp2);
			}
			len = len - tmp2;
			tmp_ptr = ptr + tmp2;
		}*/
#if 0
		memcpy((unsigned char *)snx_temp1->ptr
			, (unsigned char*)(ptr)
			, bs_size);
		snx_temp1->ptr += bs_size;
		snx_temp1->total_size += bs_size;
#endif
	}
	else {
		print_msg("memory full\n");
	}
	return bs_size;
}

void vTaskRecord1( void *pvParameters )
{
	int temp_buf_size = 3 * 1024 * 1024;
	struct snx_app1 *app =  (struct snx_app1*)pvParameters;
	struct snx_m2m *m2m =  app->m2m;

	TickType_t tick_new = 0, tick = 0;
	int timer=0;
	int time_debug = 1;

	int bps=0, fps=0;
	int width=0, height=0, scale=0;
	int size;
//	int test=0;
	QueueHandle_t *recv;	
	int ready;
	struct snx_temp1 snx_temp1;
	char filename[40];
	system_date_t t;

	memset(filename, 0, 40);

	get_date(&t);
	sprintf(filename, "video_%d%d%d_%d%d%d.h264", t.year, t.month, t.day, t.hour, t.minute, t.second);

	// Open file on SD
	if(f_open(&MyFile, filename, FA_CREATE_ALWAYS | FA_WRITE) == FR_OK) {
		print_msg_queue("Video file open ok\n");
	} else {
		print_msg_queue("Video file open fail\n");
	}


	memset(&snx_temp1, 0x0, sizeof(struct snx_temp1));

	recv = snx_video_task_recv(m2m, app->mode, app->dup);

	snx_temp1.total_size = 0;
	snx_temp1.buf = snx_temp_alloc1(temp_buf_size);
	snx_temp1.ptr = (unsigned char *)snx_temp1.buf;


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
					print_msg("<<test>><%s><%d> vc2file timeout\n", __func__, __LINE__);
				}
			}
			
			if(m2m->start == 0)
				break;

			if(ready == 0)
				break;

			size = snx_temp_write1(app->mode, &snx_temp1, m2m, temp_buf_size, app->dup);
#if 1
		tick_new = xTaskGetTickCount();
		timer = (tick_new - (tick))/configTICK_RATE_HZ;
		if(timer >= time_debug) { // update tick;
			tick = tick_new;
			if(bps != 0)
				print_msg(" (%dX%d) %s %d Kbps fps=%d qp=%d\n"
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
				print_msg(" (%dX%d) %s %d Kbps fps=%d qp=%d\n"
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
/*
		if(app->mode==FMT_H264) {
			test += 20;
			if(test < 500) {
				vTaskDelay( test / portTICK_RATE_MS );
			}
			else if(test == 500){
				print_msg("static\n");
			}
		}
*/
		
		}
	}
/*	
	print_msg("dump binary memory Z:\\src\\freertos_prj\\test%d%d%d.%s 0x%x 0x%x\n"
			, m2m->channel
			, app->mode
			, app->dup
			, (app->mode==FMT_H264)?"h264":"jpg"
			, snx_temp1.buf, snx_temp1.ptr);
*/
//		print_msg("dump binary memory Z:\\src\\freertos_prj\\test.yuv 0x%x +115200\n"
//						, m2m->vc.mid);

	f_close(&MyFile);

	vPortFree( (void *)snx_temp1.buf );
	vQueueDelete(*recv);
	vTaskDelete(NULL);

}

#if APP_VC_TASK

void vTaskVC1( void *pvParameters )
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


int snx_task_set_vc1(struct snx_m2m *m2m, int task)
{
//	struct snx_vc *vc =  &m2m->vc;
	int mode = 0;
	m2m->channel = task;
	// setting dep. isp	
	if(task == 0) { // FOR ISP 
		snx_video_set_resolution(m2m, 1280, 720);
	}
	else if(task == 1) {
		snx_video_set_resolution(m2m, 640, 480);
	}
	else{ // FOR snx_dummy
		snx_video_set_resolution(m2m, 640, 480);
	}
	snx_isp_set_fps(m2m, 15);
	snx_video_set_scale(m2m, 1);

	mode |= FMT_H264; // H264
//	mode |= FMT_MJPEG; // JPEG

	// setting vc path 0
	snx_video_set_mode(m2m, 0, mode );
	snx_video_set_fps(m2m, 0, 15, FMT_H264);
	snx_video_set_fps(m2m, 0, 15, FMT_MJPEG);
	snx_video_set_bps(m2m, 0, 1024*1024);
	snx_video_set_gop(m2m, 0, 0); // fps == gop
	snx_video_set_qp(m2m, 0, 23, FMT_H264); // set h264 qp value
	snx_video_set_qp(m2m, 0, 64, FMT_MJPEG); // set jpeg qp value

	// setting vc path 1
	mode = 0;
	snx_video_set_mode(m2m, 1, mode );
	snx_video_set_fps(m2m, 1, 15, FMT_H264);
	snx_video_set_fps(m2m, 1, 15, FMT_MJPEG);
	snx_video_set_bps(m2m, 1, 128*1024);
	snx_video_set_gop(m2m, 1, 0); // fps == gop
	snx_video_set_qp(m2m, 1, 23, FMT_H264); // set h264 qp value
	snx_video_set_qp(m2m, 1, 64, FMT_MJPEG); // set jpeg qp value

	return pdPASS;
}

int snx_vc_get_info1(struct snx_m2m *m2m, int task)
{
	int width, height, scale;
	
	if(m2m->channel == -1) {
		snx_video_init(m2m);
		snx_task_set_vc1(m2m, task);		
	}
	
	snx_video_get_resolution(m2m, &width, &height);
	scale = snx_video_get_scale(m2m);
	
	print_msg(" mode           = %s %s\n"
			, ((snx_video_get_mode(m2m, 0)&FMT_H264)>0)?"H264":""
			, ((snx_video_get_mode(m2m, 0)&FMT_MJPEG)>0)?"JPEG":"");
	print_msg(" resolution     = %d X %d\n", width, height);
	print_msg(" isp_fps        = %d fps\n", snx_isp_get_fps(m2m));
	print_msg(" fps            = H264 %d fps JPEG %d fps\n"
			, snx_video_get_fps(m2m, 0, FMT_H264)
			, snx_video_get_fps(m2m, 0, FMT_MJPEG));
	print_msg(" bps            = %d Kbps    qp = %d\n"
			, snx_video_get_bps(m2m, 0)>>10
			, snx_video_get_qp(m2m, 0, FMT_MJPEG));
	print_msg(" scale          = X%d\n", (1<<scale));
	print_msg(" dup mode       = %s %s\n"
			, ((snx_video_get_mode(m2m, 1)&FMT_H264)>0)?"H264":""
			, ((snx_video_get_mode(m2m, 1)&FMT_MJPEG)>0)?"JPEG":"");
	print_msg(" dup resolution = %d X %d\n", width>>scale, height>>scale);
	print_msg(" dup fps        = H264 %d fps JPEG %d fps\n"
			, snx_video_get_fps(m2m, 1, FMT_H264)
			, snx_video_get_fps(m2m, 1, FMT_MJPEG));
	print_msg(" dup bps        = %d Kbps    qp = %d\n"
			, snx_video_get_bps(m2m, 1)>>10
			, snx_video_get_qp(m2m, 1, FMT_MJPEG));

	return pdPASS;
}

int snx_rc_get_info1(struct snx_m2m *m2m, int task)
{
	int value, value1;
	if(m2m->channel == -1) {
		snx_video_init(m2m);
		snx_task_set_vc1(m2m, task);		
	}

	snx_get_mdrc_en(m2m, 0, &value);
	print_msg(" mdrc_en(md)               = %d\n", value);
	snx_get_md_mn(m2m, 0, &value, &value1);
	print_msg(" md_m(md_m)                = %d\n", value);
	print_msg(" md_n(md_n)                = %d\n", value1);


	snx_get_md_th(m2m, 0, &value, &value1);
	print_msg(" md_th(md_th)              = %d\n", value);
	print_msg(" md_recover(md_recover)    = %d\n", value1);

	snx_get_md_2dnr(m2m, 0, &value, &value1);
	print_msg(" md_2dnr(md_2dnr)          = %d\n", value);
	print_msg(" md_isp_nr(md_isp_nr)      = %d\n", value1);

	snx_get_md_fpsbps(m2m, 0, &value, &value1);
	print_msg(" md_max_fps(md_fps)        = %d\n", value);
	print_msg(" md_can_add_bitrate(md_bps)= %d\n", value1);
	
	snx_get_mdcnt_en(m2m, 0, &value);
	print_msg(" mdcnt_en(mdcnt)           = %d\n", value);
	snx_get_mdcnt_th(m2m, 0, &value, &value1);
	print_msg(" md_cnt_sum_th(sum_th)     = %d\n", value);
	print_msg(" md_cnt_th(th)             = %d\n", value1);
	snx_get_mdcnt_bps(m2m, 0, &value, &value1);
	print_msg(" md_cnt_bps(bps)           = %d\n", value);
	print_msg(" md_cnt_bps2(bps2)         = %d\n", value1);

	snx_get_mdcnt_lowbound(m2m, 0, &value, &value1);
	print_msg(" md_cnt_lowbound(low)      = %d\n", value);
	print_msg(" md_cnt_qp(qp)             = %d\n", value1);
	snx_get_mdcnt_absy(m2m, 0, &value);
	print_msg(" md_cnt_absy(absy)         = %d\n", value);
	snx_get_mdcnt_gop(m2m, 0, &value);
	print_msg(" snx_set_mdcnt_gop(gop_mul)= %d\n", value);
	snx_get_mdcnt_count(m2m, 0, &value);
	print_msg(" md_cnt_count(count)       = %d\n", value);

	return pdPASS;
}


int cmd_rtsp_start(int argc, char* argv[])
{
	struct snx_m2m *m2m;
	int task = 0;
	int i = 0;
	
#if PREVIEW_AUDIO_TEST
	init_rtsp_server(1);
#else
	init_rtsp_server(0);
#endif
vTaskDelay( 1000/portTICK_PERIOD_MS );print_msg(" delay over\n");
/*******open audio task*********/
/*
if (pdPASS != xTaskCreate(run_audio, "run_audio", 2048, NULL,
							PRIORITY_TASK_APP_TEST01, NULL))
		print_msg("<<test>><%s><%d> Could not create task\n", __func__, __LINE__);*/

#ifdef CONFIG_MIDDLEWARE_AUDIO_CODEC_AAC
if (pdPASS != xTaskCreate(run_aac, "run_aac", 2048 * 4, NULL,
							PRIORITY_TASK_APP_TEST01, NULL))
		print_msg("<<test>><%s><%d> Could not create task\n", __func__, __LINE__);
#endif

/*******open audio task*********/

	if(argc != 2){
		print_msg(" start 0 ( task 0 start)\n");
		print_msg(" start 1 ( task 1 start)\n");
		print_msg(" start 2 ( task 2 start)(dummy sensor for test)\n");
		return pdFAIL;
	}

	task =simple_strtoul(argv[1], NULL, 10);

	if(task == 0) {
		m2m= &m2m_task0;
	}
	else if (task == 1){
		m2m= &m2m_task1;
	}
	else
		m2m= &m2m_task2;
	
	

	if(m2m->channel == -1) {
		snx_video_init(m2m);
		snx_task_set_vc1(m2m, task);
	}
print_msg(" ------%d\n",__LINE__);

#if APP_VC_TASK
	if (pdPASS != xTaskCreate(vTaskVC1, "vc", 1024, (void*) m2m,
							PRIORITY_TASK_APP_TEST01, NULL))
		print_msg("<<test>><%s><%d> Could not create task\n", __func__, __LINE__);
#else
	snx_video_task_start(m2m);
#endif
print_msg(" ------%d\n",__LINE__);
	for(i = 0;i < 4;i++) {
		app[i].m2m = m2m;
		if(app[i].mode & snx_video_get_mode(m2m, app[i].dup)) {
			if (pdPASS != xTaskCreate(vTaskRecord1, app[i].name, 2048, (void*) &app[i],
					17, NULL))
				print_msg("<<test>><%s><%d> Could not create task\n", __func__, __LINE__);
		}
	}
print_msg(" ------%d\n",__LINE__);
	return pdPASS;
}

/*
int cmd_rtsp_start(int argc, char* argv[])
{
	struct snx_m2m *m2m;
	int task = 0;
	int i = 0;

if(argc != 2){
		print_msg(" start 0 ( task 0 start)\n");
		print_msg(" start 1 ( task 1 start)\n");
		print_msg(" start 2 ( task 2 start)(dummy sensor for test)\n");
		return pdFAIL;
	}
task =simple_strtoul(argv[1], NULL, 10);


if (pdPASS != xTaskCreate(tcp_selected_test_task, "rtsp_test", 4096, NULL,
					TCP_TEST_TASK_PRO-38, NULL))
				print_msg("<<test>><%s><%d> Could not create task\n", __func__, __LINE__);

print_msg(" ------%d\n",__LINE__);

task_exit = 0;
task_enable = 1;
falco_audio_new1(0);



vTaskDelay( 3000/portTICK_PERIOD_MS );

	

	if(task == 0) {
		m2m= &m2m_task0;
	}
	else if (task == 1){
		m2m= &m2m_task1;
	}
	else
		m2m= &m2m_task2;
	
	

	if(m2m->channel == -1) {print_msg(" ------%d\n",__LINE__);
		snx_video_init(m2m);print_msg(" ------%d\n",__LINE__);
		snx_task_set_vc1(m2m, task);print_msg(" ------%d\n",__LINE__);
	}


 snx_video_task_start(m2m);

	for(i = 0;i < 4;i++) {
		app[i].m2m = m2m;
		if(app[i].mode & snx_video_get_mode(m2m, app[i].dup)) {
			if (pdPASS != xTaskCreate(vTaskRecord1, app[i].name, 2048, (void*) &app[i],
					PRIORITY_TASK_MID_VIDEO, NULL))
				print_msg("<<test>><%s><%d> Could not create task\n", __func__, __LINE__);
		}
	}
print_msg(" ------%d\n",__LINE__);




	return pdPASS;
}
*/

int cmd_rtsp_stop(int argc, char* argv[])
{
	struct snx_m2m *m2m;
	int task;
/************audio exit***************/

/************audio exit***************/
	if(argc != 2){
		print_msg(" stop 0 ( task 0 stop)\n");
		print_msg(" stop 1 ( task 1 stop)\n");
		print_msg(" stop 2 ( task 2 stop)\n");
		return pdFAIL;
	}

	task =simple_strtoul(argv[1], NULL, 10);
	if(task == 0) {
		m2m= &m2m_task0;
	}
	else if (task == 1){
		m2m= &m2m_task1;
	}
	else
		m2m= &m2m_task2;

#if APP_VC_TASK
	m2m->start =0;
#else
	snx_video_task_stop(m2m);
#endif
destroy_rtsp_server();
	print_msg(" (task %d stop)\n", task);
	return pdPASS;
	
}



