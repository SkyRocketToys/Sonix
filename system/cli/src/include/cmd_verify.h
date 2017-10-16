#ifndef _CMD_VERIFY_H_
#define _CMD_VERIFY_H_

int cmd_verify_memtest(int argc, char* argv[]);
#ifdef CONFIG_MIDDLEWARE_TONE_DETECTION
int cmd_verify_tone_detection_test(int argc, char* argv[]);
int cmd_verify_tone_detection_loopback(int argc, char* argv[]);
#endif
int cmd_verify_audio_codec(int argc, char* argv[]);
int cmd_verify_audio_play(int argc, char* argv[]);
int cmd_verify_audio_record(int argc, char* argv[]);
int cmd_verify_audio_record_agc(int argc, char* argv[]);
int cmd_verify_audio_aec_start(int argc, char* argv[]);
int cmd_verify_audio_aec_stop(int argc, char* argv[]);
int cmd_verify_audio_set_agc(int argc, char* argv[]);
int cmd_verify_audio_agc_info(int argc, char* argv[]);
int cmd_verify_audio_flyplay(int argc, char* argv[]);
int cmd_verify_audio_twoway(int argc, char* argv[]);
int cmd_verify_audio_multi(int argc, char* argv[]);
int cmd_verify_net_mac_loopback(int argc, char* argv[]);
int cmd_verify_net_iperf(int argc, char* argv[]);
int cmd_verify_net_throughput(int argc, char* argv[]);
int cmd_verify_net_ping(int argc, char* argv[]);
int cmd_verify_i2c_rdid(int argc, char* argv[]);
int cmd_verify_fs_rwtest(int argc, char* argv[]);
int cmd_verify_sd_rwtest(int argc, char* argv[]);
int cmd_verify_sf_rwtest(int argc, char* argv[]);
int cmd_verify_video_rec_start(int argc, char* argv[]);
int cmd_verify_video_rec_stop(int argc, char* argv[]);
int cmd_verify_usbh_msc(int argc, char* argv[]);
int cmd_verify_usbd_msc(int argc, char* argv[]);
int cmd_verify_usbd_uvc(int argc, char* argv[]);
int cmd_verify_usbh_iad(int argc, char* argv[]);
int cmd_verify_easysetup_start(int argc, char* argv[]);

#define CMD_TBL_VERIFY    CMD_TBL_ENTRY(          \
	"verify",		6,	NULL,       \
	"+verify		- Main the verify",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_verify_tbl,		cmd_main_tbl			\
),

#define CMD_TBL_VERIFY_TDTEST    CMD_TBL_ENTRY(          \
	"tdtest",		6,	cmd_verify_tone_detection_test,       \
	"tdtest		- Tone detection test",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VERIFY_MEMTEST    CMD_TBL_ENTRY(          \
	"memtest",		7,		cmd_verify_memtest,       \
	"memtest		- Memory test", CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL                    \
),

#define CMD_TBL_VERIFY_TDLOOPBACK    CMD_TBL_ENTRY(          \
	"tdlo",		4,	cmd_verify_tone_detection_loopback,       \
	"tdlo		- Tone detection loopback test",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VERIFY_VIDEO    CMD_TBL_ENTRY(          \
	"video",		5,	NULL,       \
	"+video		- Video test",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_verify_video_tbl,		cmd_verify_tbl			\
),

#define CMD_TBL_VERIFY_VIDER_REC_START    CMD_TBL_ENTRY(          \
	"recstart",		8,	cmd_verify_video_rec_start,       \
	"recstart	- Video rec start",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VERIFY_VIDER_REC_STOP    CMD_TBL_ENTRY(          \
	"recstop",		7,	cmd_verify_video_rec_stop,       \
	"recstop		- Video rec stop",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VERIFY_AUDIO_PLAY    CMD_TBL_ENTRY(          \
	"aplay",		5,	cmd_verify_audio_play,       \
	"aplay		- Audio play",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VERIFY_AUDIO_RECORD    CMD_TBL_ENTRY(          \
	"arec",		4,	cmd_verify_audio_record,       \
	"arec		- Audio record(A-Law)",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VERIFY_AUDIO_AGC    CMD_TBL_ENTRY(          \
	"agc",		3,	NULL,       \
	"+agc		- Audio record with Auto Gain Control",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_verify_agc_tbl,		cmd_verify_audio_tbl			\
),

#define CMD_TBL_VERIFY_AUDIO_AEC_START    CMD_TBL_ENTRY(          \
	"aecstart",		8,	cmd_verify_audio_aec_start,       \
	"aecstart	- Acoustic Echo Cancellation start ",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VERIFY_AUDIO_AEC_STOP    CMD_TBL_ENTRY(          \
	"aecstop",		7,	cmd_verify_audio_aec_stop,       \
	"aecstop		- Acoustic Echo Cancellation stop ",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VERIFY_AUDIO_CODEC    CMD_TBL_ENTRY(          \
	"acodec",		6,	cmd_verify_audio_codec,       \
	"acodec		- Audio codec enc/dec test",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VERIFY_AUDIO_FLYPLAY    CMD_TBL_ENTRY(          \
	"flyplay",		7,	cmd_verify_audio_flyplay,       \
	"flyplay		- Play audio from record directly",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VERIFY_AUDIO_TWOWAY    CMD_TBL_ENTRY(          \
	"twoway",		6,	cmd_verify_audio_twoway,       \
	"twoway		- capture and play at the same time",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VERIFY_AUDIO_MULTI    CMD_TBL_ENTRY(          \
	"multi",		5,	cmd_verify_audio_multi,       \
	"multi		- multi process capture and play at the same time",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VERIFY_AUDIO    CMD_TBL_ENTRY(          \
	"audio",		5,	NULL,       \
	"+audio		- Audio test",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_verify_audio_tbl,		cmd_verify_tbl			\
),

#define CMD_TBL_VERIFY_AGC_REC    CMD_TBL_ENTRY(          \
	"agcrec",		6,	cmd_verify_audio_record_agc,       \
	"agcrec		- Audio record with AGC(PCM S16 LE)",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VERIFY_AGC_GET_INFO    CMD_TBL_ENTRY(          \
	"agcinfo",		7,	cmd_verify_audio_agc_info,       \
	"agcinfo		- Show AGC current parameters",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VERIFY_AGC_SET_SEC    CMD_TBL_ENTRY(          \
	"agcrsec",		7,	cmd_verify_audio_set_agc,       \
	"agcrsec		- Set audio record length(sec)",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VERIFY_AGC_SET_GAIN_MAX    CMD_TBL_ENTRY(          \
	"agcgmax",		7,	cmd_verify_audio_set_agc,       \
	"agcgmax		- Set AGC maximum Gain value",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VERIFY_AGC_SET_GAIN_MIN    CMD_TBL_ENTRY(          \
	"agcgmin",		7,	cmd_verify_audio_set_agc,       \
	"agcgmin		- Set AGC minimum Gain value",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VERIFY_AGC_SET_GAIN_DEFAULT    CMD_TBL_ENTRY(          \
	"agcgdef",		7,	cmd_verify_audio_set_agc,       \
	"agcgdef		- Set AGC default Gain value",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VERIFY_AGC_SET_DYN_GAIN_MAX    CMD_TBL_ENTRY(          \
	"agcdgmax",		8,	cmd_verify_audio_set_agc,       \
	"agcdgmax	- Set AGC dynamic maximum Gain value",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VERIFY_AGC_SET_DYN_GAIN_MIN    CMD_TBL_ENTRY(          \
	"agcdgmin",		8,	cmd_verify_audio_set_agc,       \
	"agcdgmin	- Set AGC dynamic minimum Gain value",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VERIFY_AGC_SET_DYN_TARGET_LEVEL_HIGH    CMD_TBL_ENTRY(          \
	"agcdthigh",		9,	cmd_verify_audio_set_agc,       \
	"agcdthigh	- Set AGC dynamic target level high bound",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VERIFY_AGC_SET_DYN_TARGET_LEVEL_LOW    CMD_TBL_ENTRY(          \
	"agcdtlow",		8,	cmd_verify_audio_set_agc,       \
	"agcdtlow	- Set AGC dynamic target level low bound",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VERIFY_AGC_SET_DYN_UPDATE_SPEED    CMD_TBL_ENTRY(          \
	"agcduspeed",		10,	cmd_verify_audio_set_agc,       \
	"agcduspeed	- Set AGC dynamic gain level update speed",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VERIFY_AGC_SET_BUFSIZE    CMD_TBL_ENTRY(          \
	"agcbsize",		8,	cmd_verify_audio_set_agc,       \
	"agcbsize	- Set AGC buffer size(samples)",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VERIFY_AGC_SET_SAMPLE_RATE    CMD_TBL_ENTRY(          \
	"agcrate",		7,	cmd_verify_audio_set_agc,       \
	"agcrate		- Set AGC sample rate",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VERIFY_AGC_SET_PEAKAMP_THD    CMD_TBL_ENTRY(          \
	"agcpth",		6,	cmd_verify_audio_set_agc,       \
	"agcpth		- Set AGC peak energy threshold",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VERIFY_AGC_SET_PEAKCNT_THD    CMD_TBL_ENTRY(          \
	"agcpcnt",		7,	cmd_verify_audio_set_agc,       \
	"agcpcnt		- Set AGC peak counts threshold",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VERIFY_AGC_SET_UPSTEP    CMD_TBL_ENTRY(          \
	"agcupstep",		9,	cmd_verify_audio_set_agc,       \
	"agcupstep	- Set AGC Gain increase speed(in frames)",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VERIFY_AGC_SET_DOWNSTEP    CMD_TBL_ENTRY(          \
	"agcdnstep",		9,	cmd_verify_audio_set_agc,       \
	"agcdnstep	- Set AGC Gain decrease speed(in frames)",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VERIFY_NET    CMD_TBL_ENTRY(          \
	"net",		3,	NULL,       \
	"+net		- Net test",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_verify_net_tbl,		cmd_verify_tbl			\
),

#define CMD_TBL_VERIFY_NET_WIFI    CMD_TBL_ENTRY(          \
	"wifi",		4,	NULL,       \
	"+wifi		- Wifi test",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_verify_net_wifi_tbl,		cmd_verify_tbl			\
),

#define CMD_TBL_VERIFY_NET_MAC    CMD_TBL_ENTRY(          \
	"mac",		3,	NULL,       \
	"+mac		- Mac test",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_verify_net_mac_tbl,		cmd_verify_tbl			\
),

#define CMD_TBL_VERIFY_NET_IPERF    CMD_TBL_ENTRY(          \
	"iperf",		5,	cmd_verify_net_iperf,       \
	"iperf		- Iperf test",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VERIFY_NET_THROUGHPUT    CMD_TBL_ENTRY(          \
	"tp",		2,	cmd_verify_net_throughput,       \
	"tp		- Networking throughput test",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VERIFY_NET_PING    CMD_TBL_ENTRY(          \
	"ping",		4,	cmd_verify_net_ping,       \
	"ping		- Ping test",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),


#define CMD_TBL_VERIFY_NET_MAC_LOOPBACK    CMD_TBL_ENTRY(          \
	"loopback",		8,	cmd_verify_net_mac_loopback,       \
	"loopback		- Mac loopback test",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VERIFY_NET_LWIP    CMD_TBL_ENTRY(          \
	"lwip",		4,	NULL,       \
	"+lwip		- lwip test",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_verify_net_lwip_tbl,		cmd_verify_tbl			\
),

#define CMD_TBL_VERIFY_CYASSL    CMD_TBL_ENTRY(          \
	"cyassl",		6,	NULL,       \
	"+cyassl		- CyaSSL test",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_verify_cyassl_tbl,		cmd_verify_tbl			\
),

#define CMD_TBL_VERIFY_SD    CMD_TBL_ENTRY(          \
	"sd",		2,	NULL,       \
	"+sd		- SD card test",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_verify_sd_tbl,		cmd_verify_tbl			\
),

#define CMD_TBL_VERIFY_SD_RW    CMD_TBL_ENTRY(          \
	"rwtest",		6,	cmd_verify_sd_rwtest,       \
	"rwtest		- SD card read/write test",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VERIFY_FS    CMD_TBL_ENTRY(          \
	"fs",		2,	NULL,       \
	"+fs		- FS card test",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_verify_fs_tbl,		cmd_verify_tbl			\
),

#define CMD_TBL_VERIFY_FS_RW    CMD_TBL_ENTRY(          \
	"rwtest",		6,	cmd_verify_fs_rwtest,       \
	"rwtest		- File system read/write test",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VERIFY_SF    CMD_TBL_ENTRY(          \
	"sf",		2,	NULL,       \
	"+sf		- Serial Flash test",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_verify_sf_tbl,		cmd_verify_tbl			\
),


#define CMD_TBL_VERIFY_SF_RW    CMD_TBL_ENTRY(          \
	"rwtest",		6,	cmd_verify_sf_rwtest,       \
	"rwtest		- Serial flash read/write test",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),


#define CMD_TBL_VERIFY_I2C    CMD_TBL_ENTRY(          \
	"i2c",		3,	NULL,       \
	"+i2c		- i2c test",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_verify_i2c_tbl,		cmd_verify_tbl			\
),

#define CMD_TBL_VERIFY_I2C_RDID    CMD_TBL_ENTRY(          \
	"rdid",		4,	cmd_verify_i2c_rdid,       \
	"rdid		- Read sensor id test",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VERIFY_EASYSETUP    CMD_TBL_ENTRY(          \
	"es",		2,	NULL,       \
	"+es		- Easy setup test.",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_verify_easysetup_tbl,		cmd_verify_tbl			\
),

#define CMD_TBL_VERIFY_EASYSETUP_START    CMD_TBL_ENTRY(          \
	"es_start",	8,	cmd_verify_easysetup_start,       \
	"es_start	- Start Easy Setup.",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		cmd_verify_easysetup_tbl			\
),

#define CMD_TBL_VERIFY_USB    CMD_TBL_ENTRY(          \
	"usb",		3,	NULL,       \
	"+usb		- usb test",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_verify_usb_tbl,		cmd_verify_tbl			\
),

#define CMD_TBL_VERIFY_USB_HOST    CMD_TBL_ENTRY(          \
	"host",		4,	NULL,       \
	"+host		- Host test",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_verify_usb_host_tbl,		cmd_verify_usb_tbl			\
),

#define CMD_TBL_VERIFY_USB_HOST_MSC    CMD_TBL_ENTRY(          \
	"msc",		3,	cmd_verify_usbh_msc,       \
	"msc		- MSC test",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		cmd_verify_usb_host_tbl			\
),

#define CMD_TBL_VERIFY_USB_DEV    CMD_TBL_ENTRY(          \
	"device",		6,	NULL,       \
	"+device		- Device test",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_verify_usb_dev_tbl,		cmd_verify_usb_tbl			\
),

#define CMD_TBL_VERIFY_USB_DEV_MSC    CMD_TBL_ENTRY(          \
	"msc",		3,	cmd_verify_usbd_msc,       \
	"msc		- MSC test",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		cmd_verify_usb_dev_tbl			\
),

#define CMD_TBL_VERIFY_USB_DEV_UVC    CMD_TBL_ENTRY(          \
	"uvc",		3,	cmd_verify_usbd_uvc,       \
	"uvc		- UVC test",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		cmd_verify_usb_dev_tbl			\
),

#define CMD_TBL_VERIFY_USB_HOST_IAD    CMD_TBL_ENTRY(          \
	"hostiad",	7,	cmd_verify_usbh_iad,       \
	"hostiad		- USB host iad test",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		cmd_verify_usb_host_iad_tbl			\
),

#endif
