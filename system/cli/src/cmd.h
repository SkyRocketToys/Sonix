#ifndef _CMD_H_
#define _CMD_H_

#ifndef AUTOCONF_INCLUDED
#include <generated/snx_sdk_conf.h>
#endif

#ifdef CONFIG_CLI_CMD_STATUS
#include <cmd_status.h>
#endif

#ifdef CONFIG_CLI_CMD_UART
#include <cmd_uart.h>
#endif

#ifdef CONFIG_CLI_CMD_DEBUG
#include <cmd_debug.h>
#endif

#ifdef CONFIG_CLI_CMD_VIDEO
#include <cmd_video.h>
#endif

#ifdef CONFIG_CLI_CMD_ISP
#include <cmd_isp.h>
#endif

#ifdef CONFIG_CLI_CMD_QRSCAN
#include <cmd_qrscan.h>
#endif

#ifdef CONFIG_CLI_CMD_SD
#include <cmd_sd.h>
#endif

#ifdef CONFIG_CLI_CMD_FS
#include <cmd_fs.h>
#endif

#ifdef CONFIG_CLI_CMD_NET
#include <cmd_net.h>
#endif

#ifdef CONFIG_CLI_CMD_VERIFY
#include <cmd_verify.h>
#endif

#ifdef CONFIG_CLI_CMD_SYSTEM
#include <cmd_system.h>
#endif

#ifdef CONFIG_CLI_CMD_NVRAM
#include <cmd_nvram.h>
#endif

#ifdef CONFIG_CLI_CMD_GPIO
#include <cmd_gpio.h>
#endif

#ifdef CONFIG_CLI_CMD_WIFI
#include <cmd_wifi.h>
#endif

#ifdef CONFIG_CLI_CMD_I2C
#include <cmd_i2c.h>
#endif

#ifdef CONFIG_CLI_CMD_AUDIO
#include <cmd_audio.h>
#endif

#ifdef CONFIG_CLI_CMD_PWM
#include <cmd_pwm.h>
#endif

#ifdef CONFIG_CLI_CMD_USBD
#include <cmd_usbd.h>
#endif

#ifdef CONFIG_CLI_CMD_USBH
#include <cmd_usbh.h>
#endif

#ifdef CONFIG_CLI_CMD_RTSP
#include <cmd_rtsp.h>
#endif

#ifdef CONFIG_CLI_CMD_DEMUX_MP4
#include <cmd_demux_mp4.h>
#endif

#ifdef CONFIG_CLI_CMD_RECORD_MP4
#include <cmd_record_mp4.h>
#endif
#ifdef CONFIG_CLI_CMD_DASHCAM
#include <cmd_dashcam.h>
#endif


#ifdef CONFIG_CLI_CMD_MSC
#include <cmd_msc.h>
#endif

#ifdef CONFIG_CLI_CMD_TONE
#include <cmd_tone.h>
#endif

#endif

