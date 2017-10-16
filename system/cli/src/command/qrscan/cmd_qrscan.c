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
#include "cmd_qrscan.h"
#include <nonstdlib.h>
#include <string.h>
#include <bsp.h>
#include <queue.h>
#include <semphr.h>
#include "cmd_video.h"
#include <vc/snx_vc.h>
#include <libmid_vc/snx_mid_vc.h>
#include <nonstdlib.h>
#include <string.h>
#include <libmid_fatfs/ff.h>
#include <sys_clock.h>
#include <zbar.h>

/** \defgroup cmd_qrscan QR code scan commands
 *  \ingroup system_cli
 * @{
 */

//#define DEBUG_WRITE_YUV_FILE 1

//#define DEBUG_READ_STD_YUV_FILE 1
#ifdef DEBUG_READ_STD_YUV_FILE
/*	
	STANDARD QR CODE PATTERN YUV420 FILE 
	The content is "sonix2015"
*/
#define STD_YUV_FILE "./std.yuv"
#endif

int cmd_qr_scan_decode(int argc, char* argv[])
{
	int ret;
	int i, width, height, rate;
	static int counts;
	static int isp_buffer_num;
	struct snx_frame_ctx ctx[2];
	int   yuv_out_size;
	unsigned char *yuv_out;

#ifdef DEBUG_WRITE_YUV_FILE
	FIL MyWFile;
	int wbytes = 0;
	system_date_t t;
	char filename[512];
#endif

#ifdef DEBUG_READ_STD_YUV_FILE
	FIL MyRFile;
	unsigned int rbytes = 0;	
#endif

	print_msg("\n\n");

	zbar_image_scanner_t *scanner = NULL;
	/* create a reader */
	scanner = zbar_image_scanner_create();
	/* configure the reader */
	zbar_image_scanner_set_config(scanner, 0, ZBAR_CFG_ENABLE, 1);

#ifdef DEBUG_WRITE_YUV_FILE
	get_date(&t);
	sprintf(filename, "video_%04d%02d%02d_%02d%02d%02d.yuv", t.year, t.month, t.day, t.hour, t.minute, t.second);

	 //Open file on SD card.
	 if(f_open(&MyWFile, filename, FA_CREATE_ALWAYS | FA_WRITE) == FR_OK) {
		print_msg("Video file open ok\n");
	} else {
		print_msg("Video file open fail\n");
		return pdFAIL;
	}
#endif

	width = 640;//simple_strtoul(argv[1], NULL, 0);;
	height = 480;//simple_strtoul(argv[2], NULL, 0);
	rate = 30;//simple_strtoul(argv[3], NULL, 0);

	print_msg("Resolution : %d x %d \n",width,height);
	print_msg("Frame Rate : %d fps \n",rate);


	yuv_out_size=((width)*(height)*3)/2;
	print_msg("yuv_out_size=%d=0x%x\n",yuv_out_size,yuv_out_size);
	yuv_out 			= (unsigned char *)pvPortMalloc(yuv_out_size, GFP_KERNEL, MODULE_CLI);
	print_msg("yuv_out=0x%08x\n",yuv_out);

#ifdef DEBUG_READ_STD_YUV_FILE
	/*  This "./std.yuv" file is on SD card root directory.  */
	if (f_open(&MyRFile, STD_YUV_FILE, FA_READ) == FR_OK) {
		print_msg("%s open ok\n",STD_YUV_FILE);
	} else {
		print_msg("%s open fail\n",STD_YUV_FILE);
		return pdFAIL;		
	}

	f_read(&MyRFile, yuv_out, yuv_out_size, &rbytes);

	f_close(&MyRFile);
#endif

	if(width == 0 || height == 0 || rate == 0){
		print_msg("Video args error!!\n");
		print_msg("width=%d\n",width);
		print_msg("height=%d\n",height);
		print_msg("rate=%d\n",rate);
		return 0;
	}

	if((ret = snx_isp_open(0, width, height, rate, VIDEO_PIX_FMT_SNX420)) == pdFAIL){
		print_msg("%s:%d:open video device error!\n",__func__,__LINE__);
		return pdFAIL;
	}

	isp_buffer_num = 2;
	if((ret = snx_isp_reqbufs(0, &isp_buffer_num)) == pdFAIL){
		print_msg("request buffers error!\n");
		goto __close_video_device;
	}

	for(i = 0; i < isp_buffer_num; i++){
		ctx[i].index = i;
		if((ret = snx_isp_querybuf(0, &ctx[i])) == pdFAIL){
			print_msg("query buffers error!\n");
			goto __close_video_device;
		}
	}
	
	for(i = 0; i < isp_buffer_num; i++){
		ctx[i].index = i;
		if((ret =snx_isp_qbuf(0, &ctx[i])) == pdFAIL){
			print_msg("enqueue buffer error!\n");
			goto __close_video_device;
		}
	}

	if((ret = snx_isp_streamon(0)) == pdFAIL){
		print_msg("video stream on error!\n");
		goto __close_video_device;
	}

	snx_isp_print_drop_frame(0);

	/* wrap image data */
	zbar_image_t *image = zbar_image_create();
	zbar_image_set_format(image, *(int*)"Y800");
	zbar_image_set_size(image, width, height);
	/*
		Be careful that zbar_image_set_data() & zbar_image_destroy() 
		will call zbar_image_free_data() to free img->data(==yuv_out).
		So Don't use zbar_image_set_data() & zbar_image_destroy() inside the while(1) loop.
		Or on FreeRTOS,this will result in a data abort system error.
	*/
	zbar_image_set_data(image,yuv_out,yuv_out_size, zbar_image_free_data);

	print_msg("QR Code Scan start decoding!\n");

	counts = 0;
	while(1){
#ifdef DEBUG_READ_STD_YUV_FILE		
		if(counts>=5){
			break;
		}
#endif		

#ifdef DEBUG_WRITE_YUV_FILE		
		if(counts>=100){
			break;
		}
#endif	

		struct snx_frame_ctx vb;
		if((ret = snx_isp_dqbuf(0, &vb)) == pdFAIL)
			goto __close_video_stream;

		print_msg("frame:%d %d %d\n", counts++, vb.index, vb.size);

#ifdef DEBUG_READ_STD_YUV_FILE
		/* Just use the same STD_YUV_FILE to test zbar functions correctness. */
#else
		snx_420line_to_420((char *)vb.userptr, (char *)yuv_out, 640, 480);
#endif
		/* scan the image for barcodes */
		int n = zbar_scan_image(scanner, image);//cost much time if the picture's size is very big
		//print_msg("scanner->syms->nsyms=%d\n",n);/* number of filtered symbols */

		/* extract results */
		const zbar_symbol_t *symbol = zbar_image_first_symbol(image);


#ifdef DEBUG_WRITE_YUV_FILE
		if(symbol==NULL){
			print_msg("symbol==NULL\n");
		}
		else{
			print_msg("symbol has SomeThing!!!!\n");
			if (f_write(&MyWFile, (unsigned char*)(yuv_out), vb.size, (void *)&wbytes) != FR_OK) {
				print_msg("video save to sd error!!!, wbytes = %d\n", wbytes);
			}
		}
#endif

		for(; symbol; symbol = zbar_symbol_next(symbol)) {
			/* do something useful with results */
			zbar_symbol_type_t typ = zbar_symbol_get_type(symbol);
			const char *data = zbar_symbol_get_data(symbol);
			print_msg("decoded %s symbol \"%s\"\n",
			zbar_get_symbol_name(typ), data);
		}


		if((ret =snx_isp_qbuf(0, &vb)) == pdFAIL)
			goto __close_video_device;		
		
	}


#ifdef DEBUG_WRITE_YUV_FILE
	f_close(&MyWFile);
#endif

    /* clean up */
    zbar_image_destroy(image);
	
    zbar_image_scanner_destroy(scanner);

__close_video_stream:
	snx_isp_streamoff(0);
__close_video_device:
	snx_isp_close(0);
	snx_isp_print_drop_frame(1);

	return pdPASS;
}

/*
"SONIX (main)> example> qrscan> decode" usage example:
>>
>>
>>
>>snx i2c:0 controller at 0x98300000 (irq = 1)
snx i2c:1 controller at 0x98400000 (irq = 2)
isp camera driver loaded
Audio Driver: audio driver init ok.
Audio Middleware: register a-law sucess.
Audio Middleware: register mu-law sucess.
Audio Middleware: register g726 sucess.
Audio Middleware: register aud32 sucess.
Audio Middleware: Audio codec initialize is done.
Audio Middleware: audio middleware init ok.
serial_flash(MXIC) , size = 16MB
NVRAM Middleware: init ok.
====================================
=========  FreeRTOS v8.2.0  ========
====================================
-----------------------
        IC+ IP101G
        PHY Addr : 0x1
-----------------------
phytype : 0x2430c54 phyadd : 0x1
100 Mbps FullDuplex (Auto Negotiation)

Wait SEM_WAKEUP_WIFI...scan:ov9715
Device MAC: AA  BB  CC  DD  EE  FF
SONIX (main)> NVRAM process task started.

SONIX (main)> ov9715 Product ID 97:11 Manufacturer ID 7f:a2
IQ.bin OK!

SONIX (main)> example
isp             - isp command table
qrscan          - qrscan command table
help            - Show usage message
back            - Back to prev level

SONIX (example)> qrscan
decode          - QR SCAN command decode
                        example: decode
help            - Show usage message
back            - Back to prev level

SONIX (qrscan)> decode

yuv_out_size=460800=0x70800
yuv_out=0x003438a0
QR Code Scan start decoding!

frame:0 0 460800
frame:1 1 460800
frame:2 0 460800
frame:3 1 460800
frame:4 0 460800
frame:5 1 460800
frame:6 0 460800
frame:7 1 460800
frame:8 0 460800
i2c:0 timeout
frame:9 1 460800
frame:10 0 460800
frame:11 1 460800
frame:12 0 460800
decoded QR-Code symbol "FreeRTOS zbar-1.0"
frame:13 1 460800
frame:14 0 460800
frame:15 1 460800
frame:16 0 460800
decoded QR-Code symbol "FreeRTOS zbar-1.0"
frame:17 1 460800
decoded QR-Code symbol "FreeRTOS zbar-1.0"
frame:18 0 460800
frame:19 1 460800
frame:20 0 460800
frame:21 1 460800
frame:22 0 460800
frame:23 1 460800
frame:24 0 460800
frame:25 1 460800
frame:26 0 460800
frame:27 1 460800
frame:28 0 460800
frame:29 1 460800
frame:30 0 460800
frame:31 1 460800
frame:32 0 460800
frame:33 1 460800
frame:34 0 460800
frame:35 1 460800
frame:36 0 460800
frame:37 1 460800
frame:38 0 460800
frame:39 1 460800
frame:40 0 460800
frame:41 1 460800
frame:42 0 460800
frame:43 1 460800
frame:44 0 460800
frame:45 1 460800
frame:46 0 460800
decoded QR-Code symbol "http://jia.360.cn/"
frame:47 1 460800
decoded QR-Code symbol "http://jia.360.cn/"
frame:48 0 460800
decoded QR-Code symbol "http://jia.360.cn/"
frame:49 1 460800
frame:50 0 460800
frame:51 1 460800
frame:52 0 460800
frame:53 1 460800
frame:54 0 460800
frame:55 1 460800
frame:56 0 460800
frame:57 1 460800
frame:58 0 460800
frame:59 1 460800
frame:60 0 460800
frame:61 1 460800
frame:62 0 460800
*/



/*
DEBUG_READ_STD_YUV_FILE usage example:
>>
>>
>>
>>snx i2c:0 controller at 0x98300000 (irq = 1)
snx i2c:1 controller at 0x98400000 (irq = 2)
isp camera driver loaded
Audio Driver: audio driver init ok.
Audio Middleware: register a-law sucess.
Audio Middleware: register mu-law sucess.
Audio Middleware: register g726 sucess.
Audio Middleware: register aud32 sucess.
Audio Middleware: Audio codec initialize is done.
Audio Middleware: audio middleware init ok.
serial_flash(MXIC) , size = 16MB
NVRAM Middleware: init ok.
====================================
=========  FreeRTOS v8.2.0  ========
====================================
-----------------------
        IC+ IP101G
        PHY Addr : 0x1
-----------------------
phytype : 0x2430c54 phyadd : 0x1
100 Mbps FullDuplex (Auto Negotiation)

Wait SEM_WAKEUP_WIFI...scan:ov9715
Device MAC: AA  BB  CC  DD  EE  FF
SONIX (main)> NVRAM process task started.
ov9715 Product ID 97:11 Manufacturer ID 7f:a2
IQ.bin OK!

SONIX (main)> fs
mount           - mount a filesystem
umount          - unmount file systems
ls              - list directory contents
pwd             - print name of current working directory
cd              - change directory
mkdir           - make directory
rm              - remove file or directory
du              - estimate file space usage
wf              - write memory to file
sdrd            - read sd data
help            - Show usage message
back            - Back to prev level

---->SD 2.0 initial
SD 2.0 -> Capacity = 0x76c000 KBytes


mount 0 success

SONIX (fs)> ls
-    1798136 2015- 9- 2 19:47 FIRMWARE_660R_F.bin
-       4084 2014-11-11 10: 0 snx_u-boot_env.bin
-     460800 2015- 9- 2 13:59 std.yuv

SONIX (fs)> back
+system         - Main the system
+example        - Example command table
+status         - show status
+video          - Video command table
+fs             - file system command table
+sd             - sd card command table
+net            - Main the net
+uart           - uart table
+verify         - Main the verify
+debug          - Main the debug
help            - Show usage message

SONIX (main)> example
isp             - isp command table
qrscan          - qrscan command table
help            - Show usage message
back            - Back to prev level

SONIX (example)> qrscan
decode          - QR SCAN command decode
                        example: decode
help            - Show usage message
back            - Back to prev level

SONIX (qrscan)> decode

yuv_out_size=460800=0x70800
yuv_out=0x00343c40

./std.yuv open ok
QR Code Scan start decoding!
frame:0 0 460800
decoded QR-Code symbol "sonix2015"
frame:1 1 460800
decoded QR-Code symbol "sonix2015"
frame:2 0 460800
decoded QR-Code symbol "sonix2015"
frame:3 1 460800
decoded QR-Code symbol "sonix2015"
frame:4 0 460800
decoded QR-Code symbol "sonix2015"

SONIX (qrscan)>

*/





/*
DEBUG_WRITE_YUV_FILE usage example:

>>
>>
>>
>>
>>
>>
>>snx i2c:0 controller at 0x98300000 (irq = 1)
snx i2c:1 controller at 0x98400000 (irq = 2)
isp camera driver loaded
Audio Driver: audio driver init ok.
Audio Middleware: register a-law sucess.
Audio Middleware: register mu-law sucess.
Audio Middleware: register g726 sucess.
Audio Middleware: register aud32 sucess.
Audio Middleware: Audio codec initialize is done.
Audio Middleware: audio middleware init ok.
serial_flash(MXIC) , size = 16MB
NVRAM Middleware: init ok.
====================================
=========  FreeRTOS v8.2.0  ========
====================================
-----------------------
        IC+ IP101G
        PHY Addr : 0x1
-----------------------
phytype : 0x2430c54 phyadd : 0x1
100 Mbps FullDuplex (Auto Negotiation)

Wait SEM_WAKEUP_WIFI...scan:ov9715
Device MAC: AA  BB  CC  DD  EE  FF
SONIX (main)> NVRAM process task started.
ov9715 Product ID 97:11 Manufacturer ID 7f:a2
IQ.bin OK!

SONIX (main)> fs
mount           - mount a filesystem
umount          - unmount file systems
ls              - list directory contents
pwd             - print name of current working directory
cd              - change directory
mkdir           - make directory
rm              - remove file or directory
du              - estimate file space usage
wf              - write memory to file
sdrd            - read sd data
help            - Show usage message
back            - Back to prev level

---->SD 2.0 initial
SD 2.0 -> Capacity = 0x76c000 KBytes


mount 0 success

SONIX (fs)> ls
-    1798136 2015- 9- 2 19:47 FIRMWARE_660R_F.bin
-       4084 2014-11-11 10: 0 snx_u-boot_env.bin
-     460800 2015- 9- 2 13:59 std.yuv

SONIX (fs)>
SONIX (fs)> back
+system         - Main the system
+example        - Example command table
+status         - show status
+video          - Video command table
+fs             - file system command table
+sd             - sd card command table
+net            - Main the net
+uart           - uart table
+verify         - Main the verify
+debug          - Main the debug
help            - Show usage message

SONIX (main)> system
date            - Get/Set date
free            - Show memory usage
reboot          - Reboot system
help            - Show usage message
back            - Back to prev level

SONIX (system)> date 2015 9 3 14 01 00

SONIX (system)> date
2015/9/3 - 14:1:1
SONIX (system)>
SONIX (system)> back
+system         - Main the system
+example        - Example command table
+status         - show status
+video          - Video command table
+fs             - file system command table
+sd             - sd card command table
+net            - Main the net
+uart           - uart table
+verify         - Main the verify
+debug          - Main the debug
help            - Show usage message

SONIX (main)> example
isp             - isp command table
qrscan          - qrscan command table
help            - Show usage message
back            - Back to prev level

SONIX (example)> qrscan
decode          - QR SCAN command decode
                        example: decode
help            - Show usage message
back            - Back to prev level

SONIX (qrscan)> decode


Video file open ok
yuv_out_size=460800=0x70800
yuv_out=0x00343da0
QR Code Scan start decoding!
frame:0 0 460800
symbol==NULL
frame:1 1 460800
symbol==NULL
frame:2 0 460800
symbol==NULL
frame:3 1 460800
symbol==NULL
frame:4 0 460800
symbol==NULL
frame:5 1 460800
symbol==NULL
frame:6 0 460800
symbol==NULL
frame:7 1 460800
symbol==NULL
frame:8 0 460800
symbol==NULL
frame:9 1 460800
symbol==NULL
frame:10 0 460800
symbol==NULL
frame:11 1 460800
symbol==NULL
frame:12 0 460800
symbol==NULL
frame:13 1 460800
symbol==NULL
frame:14 0 460800
symbol==NULL
frame:15 1 460800
symbol==NULL
frame:16 0 460800
symbol==NULL
frame:17 1 460800
symbol==NULL
frame:18 0 460800
symbol==NULL
frame:19 1 460800
symbol==NULL
frame:20 0 460800
symbol==NULL
frame:21 1 460800
symbol==NULL
frame:22 0 460800
symbol==NULL
frame:23 1 460800
symbol==NULL
frame:24 0 460800
symbol==NULL
frame:25 1 460800
symbol==NULL
frame:26 0 460800
symbol==NULL
frame:27 1 460800
symbol==NULL
frame:28 0 460800
symbol==NULL
frame:29 1 460800
symbol has SomeThing!!!!
decoded QR-Code symbol "FreeRTOS zbar-1.0"
frame:30 0 460800
symbol has SomeThing!!!!
decoded QR-Code symbol "FreeRTOS zbar-1.0"
frame:31 1 460800
symbol has SomeThing!!!!
decoded QR-Code symbol "FreeRTOS zbar-1.0"
frame:32 0 460800
symbol has SomeThing!!!!
decoded QR-Code symbol "FreeRTOS zbar-1.0"
frame:33 1 460800
symbol has SomeThing!!!!
decoded QR-Code symbol "FreeRTOS zbar-1.0"
frame:34 0 460800
symbol==NULL
frame:35 1 460800
symbol==NULL
frame:36 0 460800
symbol has SomeThing!!!!
decoded QR-Code symbol "FreeRTOS zbar-1.0"
frame:37 1 460800
symbol has SomeThing!!!!
decoded QR-Code symbol "FreeRTOS zbar-1.0"
frame:38 0 460800
symbol has SomeThing!!!!
decoded QR-Code symbol "FreeRTOS zbar-1.0"
frame:39 1 460800
symbol==NULL
frame:40 0 460800
symbol==NULL
frame:41 1 460800
symbol has SomeThing!!!!
decoded QR-Code symbol "FreeRTOS zbar-1.0"
frame:42 0 460800
symbol==NULL
frame:43 1 460800
symbol has SomeThing!!!!
decoded QR-Code symbol "FreeRTOS zbar-1.0"
frame:44 0 460800
symbol has SomeThing!!!!
decoded QR-Code symbol "FreeRTOS zbar-1.0"
frame:45 1 460800
symbol==NULL
frame:46 0 460800
symbol==NULL
frame:47 1 460800
symbol==NULL
frame:48 0 460800
symbol==NULL
frame:49 1 460800
symbol==NULL
frame:50 0 460800
symbol has SomeThing!!!!
decoded QR-Code symbol "FreeRTOS zbar-1.0"
frame:51 1 460800
symbol has SomeThing!!!!
decoded QR-Code symbol "FreeRTOS zbar-1.0"
frame:52 0 460800
symbol has SomeThing!!!!
decoded QR-Code symbol "FreeRTOS zbar-1.0"
frame:53 1 460800
symbol==NULL
frame:54 0 460800
symbol==NULL
frame:55 1 460800
symbol==NULL
frame:56 0 460800
symbol==NULL
frame:57 1 460800
symbol==NULL
frame:58 0 460800
symbol==NULL
frame:59 1 460800
symbol has SomeThing!!!!
decoded QR-Code symbol "FreeRTOS zbar-1.0"
frame:60 0 460800
symbol has SomeThing!!!!
decoded QR-Code symbol "FreeRTOS zbar-1.0"
frame:61 1 460800
symbol has SomeThing!!!!
decoded QR-Code symbol "FreeRTOS zbar-1.0"
frame:62 0 460800
symbol has SomeThing!!!!
decoded QR-Code symbol "FreeRTOS zbar-1.0"
frame:63 1 460800
symbol==NULL
frame:64 0 460800
symbol==NULL
frame:65 1 460800
symbol==NULL
frame:66 0 460800
symbol has SomeThing!!!!
decoded QR-Code symbol "FreeRTOS zbar-1.0"
frame:67 1 460800
symbol has SomeThing!!!!
decoded QR-Code symbol "FreeRTOS zbar-1.0"
frame:68 0 460800
symbol==NULL
frame:69 1 460800
symbol==NULL
frame:70 0 460800
symbol==NULL
frame:71 1 460800
symbol==NULL
frame:72 0 460800
symbol==NULL
frame:73 1 460800
symbol==NULL
frame:74 0 460800
symbol==NULL
frame:75 1 460800
symbol==NULL
frame:76 0 460800
symbol==NULL
frame:77 1 460800
symbol==NULL
frame:78 0 460800
symbol==NULL
frame:79 1 460800
symbol==NULL
frame:80 0 460800
symbol==NULL
frame:81 1 460800
symbol==NULL
frame:82 0 460800
symbol==NULL
frame:83 1 460800
symbol==NULL
frame:84 0 460800
symbol==NULL
frame:85 1 460800
symbol==NULL
frame:86 0 460800
symbol==NULL
frame:87 1 460800
symbol==NULL
frame:88 0 460800
symbol==NULL
frame:89 1 460800
symbol==NULL
frame:90 0 460800
symbol==NULL
frame:91 1 460800
symbol==NULL
frame:92 0 460800
symbol==NULL
frame:93 1 460800
symbol==NULL
frame:94 0 460800
symbol==NULL
frame:95 1 460800
symbol==NULL
frame:96 0 460800
symbol==NULL
frame:97 1 460800
symbol==NULL
frame:98 0 460800
symbol==NULL
frame:99 1 460800
symbol==NULL

SONIX (qrscan)> back
isp             - isp command table
qrscan          - qrscan command table
help            - Show usage message
back            - Back to prev level

SONIX (example)> back
+system         - Main the system
+example        - Example command table
+status         - show status
+video          - Video command table
+fs             - file system command table
+sd             - sd card command table
+net            - Main the net
+uart           - uart table
+verify         - Main the verify
+debug          - Main the debug
help            - Show usage message

SONIX (main)> fs
mount           - mount a filesystem
umount          - unmount file systems
ls              - list directory contents
pwd             - print name of current working directory
cd              - change directory
mkdir           - make directory
rm              - remove file or directory
du              - estimate file space usage
wf              - write memory to file
sdrd            - read sd data
help            - Show usage message
back            - Back to prev level

SONIX (fs)> ls
-    1798136 2015- 9- 2 19:47 FIRMWARE_660R_F.bin
-       4084 2014-11-11 10: 0 snx_u-boot_env.bin
-     460800 2015- 9- 2 13:59 std.yuv
-    9216000 1980- 0- 0 14: 3 video_20150903_140231.yuv

SONIX (fs)> umount 0
unmount 0 success

SONIX (fs)> back
+system         - Main the system
+example        - Example command table
+status         - show status
+video          - Video command table
+fs             - file system command table
+sd             - sd card command table
+net            - Main the net
+uart           - uart table
+verify         - Main the verify
+debug          - Main the debug
help            - Show usage message

SONIX (main)>
*/

/** @} */
