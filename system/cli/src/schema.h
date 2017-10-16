#ifndef _SCHEMA_H_
#define _SCHEMA_H_

#include <generated/snx_sdk_conf.h>
#include "rtoscli.h"

struct cmd_table *curr_lv;
struct cmd_table cmd_main_tbl[];

#ifdef CONFIG_CLI_CMD_VIDEO
struct cmd_table cmd_video_tbl[];
#endif

#ifdef CONFIG_CLI_CMD_ISP
struct cmd_table cmd_isp_tbl[];
#endif

#ifdef CONFIG_CLI_CMD_NVRAM
struct cmd_table cmd_nvram_tbl[];
#endif

#ifdef CONFIG_CLI_CMD_QRSCAN
struct cmd_table cmd_qr_scan_tbl[];
#endif

#ifdef CONFIG_CLI_CMD_FS
struct cmd_table cmd_fs_tbl[];
#endif

#ifdef CONFIG_CLI_CMD_NET
struct cmd_table cmd_net_tbl[];
#endif

#ifdef CONFIG_CLI_CMD_UART
struct cmd_table cmd_uart_tbl[];
#endif

#ifdef CONFIG_CLI_CMD_DASHCAM
struct cmd_table cmd_dashcam_tbl[];
#endif

#ifdef CONFIG_CLI_CMD_VERIFY
struct cmd_table cmd_verify_tbl[];
struct cmd_table cmd_verify_net_wifi_tbl[];
struct cmd_table cmd_verify_net_mac_tbl[];
struct cmd_table cmd_verify_cyassl_tbl[];
#ifdef CONFIG_CLI_CMD_VERIFY_SD
struct cmd_table cmd_verify_sd_tbl[];
#endif
#ifdef CONFIG_CLI_CMD_VERIFY_FS
struct cmd_table cmd_verify_fs_tbl[];
#endif
struct cmd_table cmd_verify_net_lwip_tbl[];
#ifdef CONFIG_CLI_CMD_VERIFY_SF
struct cmd_table cmd_verify_sf_tbl[];
#endif
#ifdef CONFIG_CLI_CMD_VERIFY_I2C
struct cmd_table cmd_verify_i2c_tbl[];
#endif
struct cmd_table cmd_verify_video_tbl[];
#ifdef CONFIG_CLI_CMD_VERIFY_AUDIO
struct cmd_table cmd_verify_audio_tbl[];
struct cmd_table cmd_verify_agc_tbl[];
#endif
struct cmd_table cmd_verify_net_tbl[];
struct cmd_table cmd_verify_usb_tbl[];
struct cmd_table cmd_verify_usb_host_tbl[];
struct cmd_table cmd_verify_usb_dev_tbl[];
struct cmd_table cmd_verify_usb_host_iad_tbl[];

#ifdef CONFIG_CLI_CMD_VERIFY_EASYSETUP
struct cmd_table cmd_verify_easysetup_tbl[];
#endif
#endif

#ifdef CONFIG_CLI_CMD_SYSTEM
struct cmd_table cmd_system_tbl[];
#endif

#ifdef CONFIG_CLI_CMD_RTSP
struct cmd_table cmd_rtsp_tbl[];
#endif

#ifdef CONFIG_CLI_CMD_DEMUX_MP4
struct cmd_table cmd_demux_mp4_tbl[];
#endif

#ifdef CONFIG_CLI_CMD_RECORD_MP4
struct cmd_table cmd_record_mp4_tbl[];
#endif

#ifdef CONFIG_CLI_CMD_GPIO
struct cmd_table cmd_gpio_tbl[];
#endif
#ifdef CONFIG_CLI_CMD_I2C
struct cmd_table cmd_i2c_tbl[];
#endif
#ifdef CONFIG_CLI_CMD_PWM
struct cmd_table cmd_pwm_tbl[];
#endif

#ifdef CONFIG_CLI_CMD_USBD
struct cmd_table cmd_usbd_tbl[];
#endif

#ifdef CONFIG_CLI_CMD_USBH
struct cmd_table cmd_usbh_tbl[];
#endif

#ifdef CONFIG_CLI_CMD_MSC
struct cmd_table cmd_msc_tbl[];
#endif

#ifdef CONFIG_CLI_CMD_AUDIO
struct cmd_table cmd_audio_tbl[];
#endif

#ifdef CONFIG_CLI_CMD_WIFI
struct cmd_table cmd_wifi_tbl[];
#endif

#ifdef CONFIG_CLI_CMD_TONE
struct cmd_table cmd_tone_tbl[];
#endif

// =======================================================

#ifdef CONFIG_CLI_CMD_VERIFY
struct cmd_table cmd_verify_tbl[17] = {
	CMD_TBL_VERIFY
#ifdef CONFIG_CLI_CMD_VERIFY_MEMTEST
	CMD_TBL_VERIFY_MEMTEST
#endif
#ifdef CONFIG_MIDDLEWARE_TONE_DETECTION
	CMD_TBL_VERIFY_TDTEST
	CMD_TBL_VERIFY_TDLOOPBACK
#endif
	CMD_TBL_VERIFY_VIDEO
#ifdef CONFIG_CLI_CMD_VERIFY_AUDIO
	CMD_TBL_VERIFY_AUDIO
#endif
	CMD_TBL_VERIFY_NET
	CMD_TBL_VERIFY_CYASSL

#ifdef CONFIG_CLI_CMD_VERIFY_SD
	CMD_TBL_VERIFY_SD
#endif
#ifdef CONFIG_CLI_CMD_VERIFY_FS
	CMD_TBL_VERIFY_FS
#endif
#ifdef CONFIG_CLI_CMD_VERIFY_SF
	CMD_TBL_VERIFY_SF
#endif
#ifdef CONFIG_CLI_CMD_VERIFY_I2C
	CMD_TBL_VERIFY_I2C
#endif
	CMD_TBL_VERIFY_USB
#ifdef CONFIG_CLI_CMD_QRSCAN
	CMD_TBL_QR_SCAN	
#endif
#ifdef CONFIG_CLI_CMD_VERIFY_EASYSETUP
	CMD_TBL_VERIFY_EASYSETUP
#endif	
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};

struct cmd_table cmd_verify_net_wifi_tbl[4] = {
	CMD_TBL_VERIFY_NET_WIFI
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};

struct cmd_table cmd_verify_net_mac_tbl[5] = {
	CMD_TBL_VERIFY_NET_MAC
	CMD_TBL_VERIFY_NET_MAC_LOOPBACK
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};

struct cmd_table cmd_verify_cyassl_tbl[4] = {
		CMD_TBL_VERIFY_CYASSL
		CMD_TBL_HELP
		CMD_TBL_BACK
		CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};

#ifdef CONFIG_CLI_CMD_VERIFY_SD
struct cmd_table cmd_verify_sd_tbl[5] = {
		CMD_TBL_VERIFY_SD
		CMD_TBL_VERIFY_SD_RW	
		CMD_TBL_HELP
		CMD_TBL_BACK
		CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif
#ifdef CONFIG_CLI_CMD_VERIFY_FS
struct cmd_table cmd_verify_fs_tbl[5] = {
		CMD_TBL_VERIFY_FS
		CMD_TBL_VERIFY_FS_RW
		CMD_TBL_HELP
		CMD_TBL_BACK
		CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif

#ifdef CONFIG_CLI_CMD_VERIFY_SF
struct cmd_table cmd_verify_sf_tbl[5] = {
		CMD_TBL_VERIFY_SF
		CMD_TBL_VERIFY_SF_RW
		CMD_TBL_HELP
		CMD_TBL_BACK
		CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif
#ifdef CONFIG_CLI_CMD_VERIFY_I2C
struct cmd_table cmd_verify_i2c_tbl[5] = {
		CMD_TBL_VERIFY_I2C
		CMD_TBL_VERIFY_I2C_RDID
		CMD_TBL_HELP
		CMD_TBL_BACK
		CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif
#ifdef CONFIG_CLI_CMD_VERIFY_AUDIO
struct cmd_table cmd_verify_audio_tbl[11] = {
	CMD_TBL_VERIFY_AUDIO
	CMD_TBL_VERIFY_AUDIO_CODEC
	CMD_TBL_VERIFY_AUDIO_PLAY
	CMD_TBL_VERIFY_AUDIO_RECORD
	CMD_TBL_VERIFY_AUDIO_AGC
	CMD_TBL_VERIFY_AUDIO_AEC_START
	CMD_TBL_VERIFY_AUDIO_AEC_STOP
	CMD_TBL_VERIFY_AUDIO_TWOWAY
	CMD_TBL_VERIFY_AUDIO_MULTI
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};

struct cmd_table cmd_verify_agc_tbl[21] = {
	CMD_TBL_VERIFY_AUDIO_AGC
	CMD_TBL_VERIFY_AGC_REC
	CMD_TBL_VERIFY_AGC_GET_INFO
	CMD_TBL_VERIFY_AGC_SET_SEC
	CMD_TBL_VERIFY_AGC_SET_GAIN_MAX
	CMD_TBL_VERIFY_AGC_SET_GAIN_MIN
	CMD_TBL_VERIFY_AGC_SET_GAIN_DEFAULT
	CMD_TBL_VERIFY_AGC_SET_DYN_GAIN_MAX
	CMD_TBL_VERIFY_AGC_SET_DYN_GAIN_MIN
	CMD_TBL_VERIFY_AGC_SET_DYN_TARGET_LEVEL_HIGH
	CMD_TBL_VERIFY_AGC_SET_DYN_TARGET_LEVEL_LOW
	CMD_TBL_VERIFY_AGC_SET_DYN_UPDATE_SPEED
	CMD_TBL_VERIFY_AGC_SET_BUFSIZE
	CMD_TBL_VERIFY_AGC_SET_SAMPLE_RATE
	CMD_TBL_VERIFY_AGC_SET_PEAKAMP_THD
	CMD_TBL_VERIFY_AGC_SET_PEAKCNT_THD
	CMD_TBL_VERIFY_AGC_SET_UPSTEP
	CMD_TBL_VERIFY_AGC_SET_DOWNSTEP
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif

struct cmd_table cmd_verify_net_lwip_tbl[4] = {
		CMD_TBL_VERIFY_NET_LWIP
		CMD_TBL_HELP

		CMD_TBL_BACK
		CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};

#ifdef CONFIG_CLI_CMD_VERIFY_EASYSETUP
struct cmd_table cmd_verify_easysetup_tbl[5] = {
	CMD_TBL_VERIFY_EASYSETUP
	CMD_TBL_VERIFY_EASYSETUP_START
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif
struct cmd_table cmd_verify_net_tbl[10] = {
	CMD_TBL_VERIFY_NET
	CMD_TBL_VERIFY_NET_MAC
	CMD_TBL_VERIFY_NET_WIFI
	CMD_TBL_VERIFY_NET_LWIP
	CMD_TBL_VERIFY_NET_IPERF
	CMD_TBL_VERIFY_NET_PING
	CMD_TBL_VERIFY_NET_THROUGHPUT
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};

struct cmd_table cmd_verify_video_tbl[6] = {
	CMD_TBL_VERIFY_VIDEO
	CMD_TBL_VERIFY_VIDER_REC_START
	CMD_TBL_VERIFY_VIDER_REC_STOP
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};

struct cmd_table cmd_verify_usb_tbl[7] = {
	CMD_TBL_VERIFY_USB
	CMD_TBL_VERIFY_USB_HOST
	CMD_TBL_VERIFY_USB_DEV
	CMD_TBL_VERIFY_USB_HOST_IAD
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL, NULL)
};

struct cmd_table cmd_verify_usb_host_tbl[5] = {
	CMD_TBL_VERIFY_USB_HOST
	CMD_TBL_VERIFY_USB_HOST_MSC
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL, NULL)
};

struct cmd_table cmd_verify_usb_dev_tbl[6] = {
	CMD_TBL_VERIFY_USB_DEV
	CMD_TBL_VERIFY_USB_DEV_MSC
	CMD_TBL_VERIFY_USB_DEV_UVC
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL, NULL)
};

struct cmd_table cmd_verify_usb_host_iad_tbl[4] = {
	CMD_TBL_VERIFY_USB_HOST_IAD
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL, NULL)
};


#ifdef CONFIG_CLI_CMD_QRSCAN
struct cmd_table cmd_qr_scan_tbl[5] = {
	CMD_TBL_QR_SCAN
	CMD_TBL_QR_SCAN_DECODE
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif

#endif	// End of CONFIG_CLI_CMD_VERIFY

#ifdef CONFIG_CLI_CMD_RTSP
struct cmd_table cmd_rtsp_tbl[6] = {
	CMD_TBL_RTSP
	CMD_TBL_RTSP_START
	CMD_TBL_RTSP_STOP
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif	// End of CONFIG_CLI_CMD_RTSP

// =====================================================================

#ifdef CONFIG_CLI_CMD_DEMUX_MP4
struct cmd_table cmd_demux_mp4_tbl[5] = {
	CMD_TBL_DEMUX_MP4
	CMD_TBL_DEMUX_MP4_START
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif

#ifdef CONFIG_CLI_CMD_RECORD_MP4
struct cmd_table cmd_record_mp4_tbl[6] = {
	CMD_TBL_RECORD_MP4
	CMD_TBL_RECORD_MP4_START
	CMD_TBL_RECORD_MP4_STOP
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif


#ifdef CONFIG_CLI_CMD_NET
struct cmd_table cmd_net_wifi_tbl[4] = {
	CMD_TBL_NET_WIFI
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};

struct cmd_table cmd_net_mac_tbl[4] = {
	CMD_TBL_NET_MAC
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};

struct cmd_table cmd_net_tbl[8] = {
	CMD_TBL_NET
	CMD_TBL_NET_WIFI
	CMD_TBL_NET_MAC
	CMD_TBL_NET_NETINFO
#if CONFIG_WIFI_MODE_AP
	CMD_TBL_NET_DHCPSINFO
#endif
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif	// End of CONFIG_CLI_CMD_NET

#ifdef CONFIG_CLI_CMD_SYSTEM
struct cmd_table cmd_system_tbl[9] = {
	CMD_TBL_SYSTEM
	CMD_TBL_SYS_DATE
	CMD_TBL_SYS_FREE
	CMD_TBL_SYS_REBOOT
	CMD_TBL_SYS_PHYMEM_RW
	CMD_TBL_SYS_SET_DBG_PRINT
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif

#ifdef CONFIG_CLI_CMD_VIDEO
struct cmd_table cmd_video_tbl[8] = {
	CMD_TBL_VIDEO
	CMD_TBL_VIDEO_RC_SET
	CMD_TBL_VIDEO_VC_SET
	CMD_TBL_VIDEO_DS_SET
	CMD_TBL_VIDEO_MROI_SET
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif	// End of CONFIG_CLI_CMD_VIDEO

#ifdef CONFIG_CLI_CMD_ISP
struct cmd_table cmd_isp_tbl[11] = {
	CMD_TBL_ISP
	CMD_TBL_ISP_CAPTURE
	CMD_TBL_ISP_ECHO
	CMD_TBL_ISP_CAT
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif

#ifdef CONFIG_CLI_CMD_NVRAM
struct cmd_table cmd_nvram_tbl[6] = {
	CMD_TBL_NVRAM
	CMD_TBL_NVRAM_LIST
	CMD_TBL_NVRAM_SET
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif
// End of CONFIG_CLI_CMD_NVRAM


#ifdef CONFIG_CLI_CMD_DEBUG
struct cmd_table cmd_debug_tbl[10] = {
	CMD_TBL_DEBUG
	CMD_TBL_DBG_MEMORY
	CMD_TBL_DBG_CMD01
	CMD_TBL_DBG_CMD02
	CMD_TBL_DBG_TRACE
	CMD_TBL_DBG_AUDIO
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif

#ifdef CONFIG_CLI_CMD_STATUS
struct cmd_table cmd_status_tbl[8] = {
	CMD_TBL_STATUS
	CMD_TBL_STAT_SYSINFO
	CMD_TBL_STAT_CPU
	CMD_TBL_STAT_INTR
	CMD_TBL_STAT_MCU
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif

#ifdef CONFIG_CLI_CMD_FS
struct cmd_table cmd_fs_tbl[15] = {
	CMD_TBL_FS
	CMD_TBL_FS_MOUNT
	CMD_TBL_FS_UMOUNT
	CMD_TBL_FS_LS
	CMD_TBL_FS_PWD
	CMD_TBL_FS_CD
	CMD_TBL_FS_MKDIR
	CMD_TBL_FS_RM
	CMD_TBL_FS_DU
	CMD_TBL_FS_WF
	CMD_TBL_FS_FMT
//	CMD_TBL_FS_SDRD
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif	// End of CONFIG_CLI_CMD_FS

#ifdef CONFIG_CLI_CMD_SD
struct cmd_table cmd_sd_tbl[6] = {
	CMD_TBL_SD
	CMD_TBL_SD_READ
	CMD_TBL_SD_RWTEST
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif	// End of CONFIG_CLI_CMD_SD

#ifdef CONFIG_CLI_CMD_UART
struct cmd_table cmd_uart_tbl[6] = {
	CMD_TBL_UART
	CMD_TBL_UART_STATUS
	CMD_TBL_UART2_VERIFY
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif	// End of CONFIG_CLI_CMD_UART

#ifdef CONFIG_CLI_CMD_GPIO
struct cmd_table cmd_gpio_tbl[16] = {
	CMD_TBL_GPIO
	CMD_TBL_GPIO_CTRL
	CMD_TBL_PWM_GPIO_CTRL
	CMD_TBL_SPI_GPIO_CTRL
	CMD_TBL_MS1_GPIO_CTRL
	CMD_TBL_AUD_GPIO_CTRL
	CMD_TBL_I2C_GPIO_CTRL
	CMD_TBL_GPIO_CTRL_INTERRUPT
	CMD_TBL_GPIO_CTRL_CLICK
	CMD_TBL_UART2_GPIO_CTRL
	CMD_TBL_JTAG_GPIO_CTRL
	CMD_TBL_TOTALDEV_GPIO_CTRL
	CMD_TBL_MS2_GPIO_CTRL
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif	// End of CONFIG_CLI_CMD_GPIO

#ifdef CONFIG_CLI_CMD_I2C
struct cmd_table cmd_i2c_tbl[5] = {
	CMD_TBL_I2C
	CMD_TBL_I2C_CTRL
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif	// End of CONFIG_CLI_CMD_I2C

#ifdef CONFIG_CLI_CMD_AUDIO
struct cmd_table cmd_audio_tbl[8] = {
	CMD_TBL_AUDIO
	CMD_TBL_AUDIO_REC
	CMD_TBL_AUDIO_PLAY
	CMD_TBL_AUDIO_TWOWAY
  CMD_TBL_AUDIO_CODEC     
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif	// End of CONFIG_CLI_CMD_AUDIO


#ifdef CONFIG_CLI_CMD_PWM
struct cmd_table cmd_pwm_tbl[5] = {
	CMD_TBL_PWM
	CMD_TBL_PWM_CTRL
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif	// End of CONFIG_CLI_CMD_I2C

#ifdef CONFIG_CLI_CMD_USBD
struct cmd_table cmd_usbd_tbl[5] = {
	CMD_TBL_USBD
	CMD_TBL_USBD_CLASS_MODE
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif

#ifdef CONFIG_CLI_CMD_USBH
struct cmd_table cmd_usbh_tbl[7] = {
	CMD_TBL_USBH
	CMD_TBL_UVC_GET_INFO
	CMD_TBL_UVC_START	
	CMD_TBL_UVC_STOP	
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif

#ifdef CONFIG_CLI_CMD_MSC
struct cmd_table cmd_msc_tbl[13] = {
	CMD_TBL_MSC
	CMD_TBL_MSC_TEST_TIME
	CMD_TBL_MSC_TEST_FORMAT
	CMD_TBL_MSC_TEST_START_POS
	CMD_TBL_MSC_TEST_END_POS
	CMD_TBL_MSC_TEST_COM_TYPE
	CMD_TBL_MSC_TEST_CLR
	CMD_TBL_MSC_DEV_INFO
	CMD_TBL_MSC_TEST_PARAMS
	CMD_TBL_MSC_TEST_EXE
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif

#ifdef CONFIG_CLI_CMD_WIFI
struct cmd_table cmd_wifi_tbl[8] = {
	CMD_TBL_WIFI
	CMD_TBL_WIFI_CTRL
	CMD_TBL_WIFI_AP
	CMD_TBL_WIFI_STA
	CMD_TBL_WIFI_INFO
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif	// End of CONFIG_CLI_CMD_WIFI

#ifdef CONFIG_CLI_CMD_TONE
struct cmd_table cmd_tone_tbl[6] = {
	CMD_TBL_TONE
	CMD_TBL_TONE_DETECT
	CMD_TBL_TONE_RAW_DATA
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif	// End of CONFIG_CLI_CMD_TONE


#ifdef CONFIG_CLI_CMD_DASHCAM
struct cmd_table cmd_dashcam_tbl[12] = {
	CMD_TBL_DASHCAM
	CMD_TBL_DASHCAM_RES
	CMD_TBL_DASHCAM_INFO
	CMD_TBL_VIDEO_VC_SET
	CMD_TBL_DASHCAM_START 
	CMD_TBL_DASHCAM_STOP
	CMD_TBL_DASHCAM_SNAPSHOT
	CMD_TBL_DASHCAM_PREVIEW_AUDIO
	CMD_TBL_SYS_FREE
	CMD_TBL_HELP
	CMD_TBL_BACK
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif	// End of CONFIG_CLI_CMD_DASHCAM

struct cmd_table cmd_main_tbl[27] = {
	CMD_TBL_MAIN
#ifdef CONFIG_CLI_CMD_DEBUG
	CMD_TBL_DEBUG
#endif
#ifdef CONFIG_CLI_CMD_FS
	CMD_TBL_FS
#endif
#ifdef CONFIG_CLI_CMD_GPIO
	CMD_TBL_GPIO
#endif
#ifdef CONFIG_CLI_CMD_WIFI
	CMD_TBL_WIFI
#endif
#ifdef CONFIG_CLI_CMD_TONE
	CMD_TBL_TONE
#endif 
#ifdef CONFIG_CLI_CMD_I2C
	CMD_TBL_I2C
#endif
#ifdef CONFIG_CLI_CMD_AUDIO
	CMD_TBL_AUDIO
#endif
#ifdef CONFIG_CLI_CMD_ISP
	CMD_TBL_ISP
#endif
#ifdef CONFIG_CLI_CMD_NET
	CMD_TBL_NET
#endif
#ifdef CONFIG_CLI_CMD_NVRAM
	CMD_TBL_NVRAM
#endif
#ifdef CONFIG_CLI_CMD_PWM
	CMD_TBL_PWM
#endif
#ifdef CONFIG_CLI_CMD_RTSP
	CMD_TBL_RTSP
#endif
#ifdef CONFIG_CLI_CMD_DEMUX_MP4
	CMD_TBL_DEMUX_MP4
#endif
#ifdef CONFIG_CLI_CMD_RECORD_MP4
	CMD_TBL_RECORD_MP4
#endif
#ifdef CONFIG_CLI_CMD_SD
	CMD_TBL_SD
#endif
#ifdef CONFIG_CLI_CMD_STATUS
	CMD_TBL_STATUS
#endif
#ifdef CONFIG_CLI_CMD_SYSTEM
	CMD_TBL_SYSTEM
#endif
#ifdef CONFIG_CLI_CMD_UART
	CMD_TBL_UART
#endif
#ifdef CONFIG_CLI_CMD_USBD
	CMD_TBL_USBD
#endif
#ifdef CONFIG_CLI_CMD_USBH
	CMD_TBL_USBH
#endif
#ifdef CONFIG_CLI_CMD_VERIFY
	CMD_TBL_VERIFY
#endif
#ifdef CONFIG_CLI_CMD_VIDEO
	CMD_TBL_VIDEO
#endif
#ifdef CFG_ENABLE_LOGIN
	CMD_TBL_LOGOUT
#endif
#ifdef CONFIG_CLI_CMD_MSC
	CMD_TBL_MSC
#endif
#ifdef CONFIG_CLI_CMD_DASHCAM
	CMD_TBL_DASHCAM
#endif
	CMD_TBL_HELP
	CMD_TBL_ENTRY(NULL, 0, NULL, NULL, 0, NULL ,NULL)
};
#endif


