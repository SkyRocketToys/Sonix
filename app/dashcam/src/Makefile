PWD		:= $(shell pwd)
INSTALL_DIR	?= $(PWD)/../image
OBJ_DIR		?= $(PWD)/../obj

ARCH            ?= arm
TARGET          ?= $(ARCH)-linux
CROSS_COMPILE   ?= $(TARGET)-
CC              = $(CROSS_COMPILE)gcc
AR              = $(CROSS_COMPILE)ar
LD		= $(CROSS_COMPILE)ld
OBJCOPY		= $(CROSS_COMPILE)objcopy

HAS_MCU_V2 = y

-include $(SDK_CONF_DIR)/config/snx_sdk.conf
CFLAGS		= $(PRJ_CFLAGS) 
CFLAGS		+= -I$(KERNEL_INC_DIR)
CFLAGS		+= -I$(DRIVER_INC_DIR)
CFLAGS		+= -I$(MIDDLEWARE_INC_DIR)
CFLAGS		+= -I$(MIDDLEWARE_LIBC_INC_DIR)
CFLAGS		+= -I$(PWD)/main_flow/
CFLAGS		+= -I$(PWD)/main_flow/record
CFLAGS		+= -I$(PWD)/main_flow/video
CFLAGS		+= -I$(PWD)/main_flow/audio
CFLAGS		+= -I$(PWD)/main_flow/upgrade
CFLAGS		+= -I$(PWD)/main_flow/daemon
CFLAGS		+= -I$(PWD)/main_flow/watch_task
CFLAGS		+= -I$(PWD)/main_flow/utility
CFLAGS		+= -I$(PWD)/main_flow/sensor_cap
CFLAGS		+= -I$(PWD)/main_flow/playback
CFLAGS		+= -I$(PWD)/main_flow/timestamp_osd
CFLAGS		+= -I$(PWD)/usbd_ext/
CFLAGS		+= -I$(SDK_CONF_DIR)

ifeq ($(CONFIG_MIDDLEWARE_MCU_CTRL), y)
ifeq ($(HAS_MCU_V2), y)
CFLAGS		+= -I$(PWD)/mcu_v2/
else
CFLAGS		+= -I$(PWD)/mcu/
endif
endif
CFLAGS		+= -I$(PWD)/main_flow/timestamp_osd/
CFLAGS		+= -I$(PWD)/main_flow/streammgr/
CFLAGS		+= -I$(DRIVER_INC_DIR)/wifi
CFLAGS		+= -I$(MIDDLEWARE_INC_DIR)/libmid_lwip/include -I$(MIDDLEWARE_INC_DIR)/libmid_lwip/include/ipv4
CFLAGS		+= -I$(MIDDLEWARE_INC_DIR)/libmid_mcu
CFLAGS		+= -I$(MIDDLEWARE_INC_DIR)/libmid_cyassl
CFLAGS		+= -I$(MIDDLEWARE_INC_DIR)/libmid_automount
CFLAGS		+= -I$(MIDDLEWARE_INC_DIR)/libmid_rtsp_server
CFLAGS		+= -I$(MIDDLEWARE_INC_DIR)/libmid_td
CFLAGS		+= -I$(MIDDLEWARE_INC_DIR)/libmid_filemanager
CFLAGS		+= -I$(SDK_CONF_DIR)

LDFLAGS		= $(PRJ_LDFLAGS)
LDFLAGS		+= -L $(DRIVER_LIB_DIR)
LDFLAGS		+= -L $(KERNEL_LIB_DIR)
LDFLAGS		+= -L $(MIDDLEWARE_LIB_DIR)
LDFLAGS		+= -L $(PWD)/usbd_ext
LDFLAGS		+= -L $(PWD)/main_flow/timestamp_osd
LDFLAGS		+= -L $(PWD)/main_flow/sensor_cap
LDFLAGS		+= -L $(PWD)/main_flow/playback
LDFLAGS		+= -L $(PWD)/main_flow/streammgr
LDFLAGS		+= -L $(PWD)/main_flow/video
LDFLAGS		+= -L $(PWD)/main_flow/audio
LDFLAGS		+= -L $(PWD)/main_flow/record

#ifeq ($(CONFIG_SYSTEM_TRACE_SELECT), y)
#LDFLAGS		+= --start-group -lkernel -ltimer -lcli -lbootinfo -ltrace -lmac -luart -lmid_nonstdc -lvc -lmid_vc -lmid_isp -lrtsp_server -li2c -lsensor -lisp -laudio_dri -lmid_audio -lmid_aec -lsf -lsd -lmid_sd -lmid_sf -lmid_fatfs -lusb_host -lmid_usbd -lusb_device -lmid_nvram -lmid_json -lmid_rec -lmid_lwip -lwifi -lmid_td -lpwm -lgpio -lmid_cyassl -lfdk-aac -lmid_fwupgrade -lwdt -lmid_automount -lmid_aac --end-group
#else
LDFLAGS		+= --start-group -lkernel -ltimer -lmid_td -lcli -lbootinfo -luart -lmid_nonstdc -lvc -lmid_vc -lmid_isp \
		-lmid_rtsp_server -li2c -lsensor -lisp -laudio_dri -lmid_audio -lsf -lsd -lmid_sd -lmid_sf \
		-lmid_fatfs -lusb_host -lmid_usbd -lusb_device -lmid_nvram -lmid_json -lmid_rec -lmid_lwip -lwifi\
		-lpwm -lgpio -lmid_cyassl -lmid_fwupgrade -lwdt -lmid_automount -lmid_filemanager
#endif


INTERNAL_FLAGS  = -lapp_osd -lapp_playback -lapp_sencap -lapp_usbdext -lapp_streammgr -lapp_video -lapp_audio -lapp_record

LDFLAGS		+= $(INTERNAL_FLAGS)


ifeq ($(CONFIG_MIDDLEWARE_AUDIO_PROCESS_AEC), y)
LDFLAGS		+= -lmid_aec
endif

ifeq ($(CONFIG_MIDDLEWARE_AUDIO_CODEC_AAC), y)
LDFLAGS		+= -lfdk-aac -lmid_aac
endif

ifeq ($(CONFIG_MIDDLEWARE_AUDIO_CODEC_AUD32), y)
LDFLAGS		+= -lmid_aud32
endif

ifeq ($(CONFIG_MIDDLEWARE_AUDIO_CODEC_ALAW), y)
LDFLAGS		+= -lmid_alaw
endif

ifeq ($(CONFIG_MIDDLEWARE_AUDIO_CODEC_MULAW), y)
LDFLAGS		+= -lmid_mulaw
endif

ifeq ($(CONFIG_MIDDLEWARE_AUDIO_CODEC_G726), y)
LDFLAGS		+= -lmid_g726
endif

ifeq ($(CONFIG_MIDDLEWARE_AUDIO_CODEC_AMRNB), y)
LDFLAGS		+= -lmid_amr-nb
endif

ifeq ($(CONFIG_MIDDLEWARE_AUDIO_CODEC_AMRWB), y)
LDFLAGS		+= -lmid_amrwb
endif

ifeq ($(CONFIG_MODULE_MAC_SUPPORT), y)
LDFLAGS += -lmac
endif

ifeq ($(CONFIG_MIDDLEWARE_ZBAR), y)
LDFLAGS += -lzbar
endif

ifeq ($(CONFIG_MIDDLEWARE_TONE_DETECTION), y)
LDFLAGS += -ltd -ltd_decode
endif

ifeq ($(CONFIG_MIDDLEWARE_MCU_CTRL), y)
LDFLAGS += -lmid_mcu
endif

ifeq ($(CONFIG_SYSTEM_TRACE_SELECT), y)
LDFLAGS += -ltrace
endif

LDFLAGS		+= --end-group
LDFLAGS         += -lgcc

ifeq ($(CONFIG_MIDDLEWARE_ZLIB), y)
LDFLAGS		+= -lz
else
LDFLAGS		+= -lgzip
endif

ifeq ($(CONFIG_MODULE_RTC_SUPPORT), y)
LDFLAGS		+= -lrtc
endif


### RESCUE LIB : SD CARD
LDFLAGS_SD_RESCUE		= $(PRJ_LDFLAGS)
LDFLAGS_SD_RESCUE		+= -L $(DRIVER_LIB_DIR_RESCUE)
LDFLAGS_SD_RESCUE		+= -L $(KERNEL_LIB_DIR_RESCUE)
LDFLAGS_SD_RESCUE		+= -L $(MIDDLEWARE_LIB_DIR_RESCUE)
LDFLAGS_SD_RESCUE		+= --start-group -lkernel -ltimer -lcli_rs -lbootinfo -luart -lmid_nonstdc \
							-lsf -lsd -lmid_sd -lmid_sf \
							-lmid_fatfs -lusb_host \
							-lmid_fwupgrade_rs -lwdt -lmid_automount 
LDFLAGS_SD_RESCUE		+= --end-group
LDFLAGS_SD_RESCUE		+= -lgcc


ifeq ($(CONFIG_MIDDLEWARE_ZLIB), y)
LDFLAGS_SD_RESCUE		+= -lz
else
LDFLAGS_SD_RESCUE		+= -lgzip
endif

### RESCUE LIB : USB DEV
LDFLAGS_UD_RESCUE		= $(PRJ_LDFLAGS)
LDFLAGS_UD_RESCUE		+= -L $(DRIVER_LIB_DIR_RESCUE)
LDFLAGS_UD_RESCUE		+= -L $(KERNEL_LIB_DIR_RESCUE)
LDFLAGS_UD_RESCUE		+= -L $(MIDDLEWARE_LIB_DIR_RESCUE)
LDFLAGS_UD_RESCUE		+= --start-group -lkernel -ltimer -lcli_rs -lbootinfo -luart -lmid_nonstdc \
							-lsf -lmid_sf \
							-lmid_usbd_rs -lusb_device_rs \
							-lwdt -lpwm -lgpio
LDFLAGS_UD_RESCUE		+= --end-group
LDFLAGS_UD_RESCUE		+= -lgcc


ifeq ($(CONFIG_MIDDLEWARE_ZLIB), y)
LDFLAGS_UD_RESCUE		+= -lz
else
LDFLAGS_UD_RESCUE		+= -lgzip
endif


OBJ_DIRS = $(OBJ_DIR)

IMAGE = $(OBJ_DIR)/image.elf
IMAGE_BIN = $(OBJ_DIR)/image.bin
IMAGE_RESCUE = $(OBJ_DIR)/image_rescue.elf
IMAGE_BIN_RESCUE = $(OBJ_DIR)/image_rescue.bin


OBJ_FILES = main.o init.o main_flow.o video_main.o snapshot.o rec_common.o rec_fatfs.o rec_cardsize.o rec_schedule.o sd_fw_upgrade.o tunninguse.o socket_ctrl.o json_cmd.o online_fw_upgrade.o watch_task.o user_param.o version.o utility.o
ifeq ($(CONFIG_MIDDLEWARE_MCU_CTRL), y)
OBJ_FILES += mcu.o
endif

ifdef CONFIG_UDP_FLOW
OBJ_FILES += udp_cmd.o
endif

ARDUPILOT_OBJ_DIR=$(OBJ_DIR)/ArduPilot
OBJ_DIRS += $(ARDUPILOT_OBJ_DIR) $(ARDUPILOT_OBJ_DIR)/web_server $(ARDUPILOT_OBJ_DIR)/web_server/files $(ARDUPILOT_OBJ_DIR)/util

ifdef CONFIG_DEV_CONSOLE
ARDUPILOT_FILES += dev_console.o
endif

ifdef CONFIG_MAVLINK_WIFI
ARDUPILOT_FILES += web_server/mavlink_core.o
ARDUPILOT_FILES += mavlink_wifi.o
ARDUPILOT_FILES += mavlink_remote_log.o
ARDUPILOT_FILES += bootloader_client.o
ARDUPILOT_FILES += dev_console_json.o
ARDUPILOT_FILES += tx_upload.o
ARDUPILOT_FILES += talloc.o
ARDUPILOT_FILES += util/print_vprintf.o
ARDUPILOT_FILES += util/ftoa_engine.o
ARDUPILOT_FILES += util/utoa_invert.o
ARDUPILOT_FILES += web_server/web_server.o
ARDUPILOT_FILES += web_server/cgi.o
ARDUPILOT_FILES += web_server/template.o
ARDUPILOT_FILES += web_server/web_files.o
ARDUPILOT_FILES += web_server/functions.o
ARDUPILOT_FILES += web_server/mavlink_json.o
ARDUPILOT_FILES += ublox.o
endif

ARDUPILOT_CFLAGS = -fdiagnostics-show-option -Werror -DSYSTEM_FREERTOS=1

OBJS = $(addprefix $(OBJ_DIR)/, $(OBJ_FILES))
OBJS += $(addprefix $(ARDUPILOT_OBJ_DIR)/, $(ARDUPILOT_FILES))

OBJ_FILES_SD_RESCUE = main_rs.o init_rs.o main_flow_rs.o sd_fw_upgrade_rs.o
OBJS_SD_RESCUE = $(addprefix $(OBJ_DIR)/, $(OBJ_FILES_SD_RESCUE))

OBJ_FILES_UD_RESCUE = main_rs.o init_rs.o main_flow_rs.o usbd_ext_rs.o
OBJS_UD_RESCUE = $(addprefix $(OBJ_DIR)/, $(OBJ_FILES_UD_RESCUE))


ifeq ($(CONFIG_RESCUE_USBDEV_UPGRADE), y)
LDFLAGS_RESCUE = $(LDFLAGS_UD_RESCUE)
OBJ_FILES_RESCUE = $(OBJ_FILES_UD_RESCUE)
endif

ifeq ($(CONFIG_RESCUE_SD_UPGRADE), y)
LDFLAGS_RESCUE = $(LDFLAGS_SD_RESCUE)
OBJ_FILES_RESCUE = $(OBJ_FILES_SD_RESCUE)
endif

### ADD RESCUE LIB & OBJ
#	LDFLAGS_RESCUE	+= -l(lib name)
#	OBJ_FILES_RESCUE	+= (obj.o)		
###

OBJS_RESCUE = $(addprefix $(OBJ_DIR)/, $(OBJ_FILES_RESCUE))

.PHONY: all clean distclean install

ifeq ($(CONFIG_RESCUE_SYSTEM), y)
all:$(OBJ_DIRS) $(IMAGE) $(IMAGE_RESCUE)
else
all:$(OBJ_DIRS) $(IMAGE)
endif

$(OBJ_DIR):
	[ -d "$@" ] || mkdir -p $@

$(ARDUPILOT_OBJ_DIR):
	[ -d "$@" ] || mkdir -p $@

$(ARDUPILOT_OBJ_DIR)/web_server:
	[ -d "$@" ] || mkdir -p $@

$(ARDUPILOT_OBJ_DIR)/web_server/files:
	[ -d "$@" ] || mkdir -p $@

$(ARDUPILOT_OBJ_DIR)/util:
	[ -d "$@" ] || mkdir -p $@

$(IMAGE):$(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -o $(IMAGE)
	$(OBJCOPY) -O binary -R .note -R .comment -S $(IMAGE) $(IMAGE_BIN)

$(IMAGE_RESCUE):$(OBJS_RESCUE)
	$(LD) $(OBJS_RESCUE) $(LDFLAGS_RESCUE) -o $(IMAGE_RESCUE)
	$(OBJCOPY) -O binary -R .note -R .comment -S $(IMAGE_RESCUE) $(IMAGE_BIN_RESCUE)


$(OBJ_DIR)/main.o:$(PWD)/main.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/init.o:$(PWD)/init.c
	$(CC) -c $(CFLAGS) $< -o $@
#$(OBJ_DIR)/printlog.o:$(PWD)/printlog.c
#	$(CC) -c $(CFLAGS) $< -o $@	
$(OBJ_DIR)/main_flow.o:$(PWD)/main_flow/main_flow.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/video_main.o:$(PWD)/main_flow/video/video_main.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/snapshot.o:$(PWD)/main_flow/video/snapshot.c
	$(CC) -c $(CFLAGS) $< -o $@	
$(OBJ_DIR)/audio_main.o:$(PWD)/main_flow/audio/audio_main.c
	$(CC) -c $(CFLAGS) $< -o $@		
$(OBJ_DIR)/audio_pushtalk.o:$(PWD)/main_flow/audio/audio_pushtalk.c
	$(CC) -c $(CFLAGS) $< -o $@		
$(OBJ_DIR)/rec_fatfs.o:$(PWD)/main_flow/record/rec_fatfs.c
	$(CC) -c $(CFLAGS) $< -o $@	
$(OBJ_DIR)/rec_schedule.o:$(PWD)/main_flow/record/rec_schedule.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/rec_cardsize.o:$(PWD)/main_flow/record/rec_cardsize.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/rec_common.o:$(PWD)/main_flow/record/rec_common.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/sd_fw_upgrade.o:$(PWD)/main_flow/upgrade/sd_fw_upgrade.c
	$(CC) -c $(CFLAGS) $< -o $@	
$(OBJ_DIR)/tunninguse.o:$(PWD)/main_flow/record/tunninguse.c
	$(CC) -c $(CFLAGS) $< -o $@	
$(OBJ_DIR)/socket_ctrl.o:$(PWD)/main_flow/daemon/socket_ctrl.c
	$(CC) -c $(CFLAGS) $< -o $@		
$(OBJ_DIR)/utility.o:$(PWD)/main_flow/utility/utility.c
	$(CC) -c $(CFLAGS) $< -o $@	
ifdef CONFIG_UDP_FLOW
$(OBJ_DIR)/udp_cmd.o:$(PWD)/main_flow/daemon/udp_cmd.c
	$(CC) -c $(CFLAGS) $< -o $@	
endif
ifdef CONFIG_DEV_CONSOLE
$(ARDUPILOT_OBJ_DIR)/dev_console.o:$(PWD)/main_flow/ArduPilot/dev_console.c
	$(CC) -c $(CFLAGS) $(ARDUPILOT_CFLAGS) $< -o $@	
endif	
ifdef CONFIG_MAVLINK_WIFI
$(ARDUPILOT_OBJ_DIR)/%.o:$(PWD)/main_flow/ArduPilot/%.c
	$(CC) -c $(CFLAGS) $(ARDUPILOT_CFLAGS) $< -o $@	
$(PWD)/main_flow/ArduPilot/web_server/files/embedded.c: $(PWD)/main_flow/ArduPilot/web_server/files/*.* $(PWD)/main_flow/ArduPilot/web_server/files/*/*.*
	make -C $(PWD)/main_flow/ArduPilot/web_server/files
$(ARDUPILOT_OBJ_DIR)/web_server/web_files.o:$(PWD)/main_flow/ArduPilot/web_server/files/embedded.c
$(ARDUPILOT_OBJ_DIR)/web_server/%.o:$(PWD)/main_flow/ArduPilot/web_server/%.c
	$(CC) -c $(CFLAGS) $(ARDUPILOT_CFLAGS) $< -o $@	
$(ARDUPILOT_OBJ_DIR)/util/%.o:$(PWD)/main_flow/ArduPilot/util/%.c
	$(CC) -c $(CFLAGS) $(ARDUPILOT_CFLAGS) $< -o $@	
endif	
$(OBJ_DIR)/json_cmd.o:$(PWD)/main_flow/daemon/json_cmd.c
	$(CC) -c $(CFLAGS) $< -o $@	
$(OBJ_DIR)/user_param.o:$(PWD)/main_flow/daemon/user_param.c
	$(CC) -c $(CFLAGS) $< -o $@		
$(OBJ_DIR)/usbd_ext.o:$(PWD)/usbd_ext/usbd_ext.c
	$(CC) -c $(CFLAGS) $< -o $@	
$(OBJ_DIR)/snx_json_usbd.o:$(PWD)/usbd_ext/snx_json_usbd.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/online_fw_upgrade.o:$(PWD)/main_flow/upgrade/online_fw_upgrade.c
	$(CC) -c $(CFLAGS) $< -o $@
ifeq ($(HAS_MCU_V2), y)
$(OBJ_DIR)/mcu.o:$(PWD)/mcu_v2/mcu.c
	$(CC) -c $(CFLAGS) $< -o $@
else
$(OBJ_DIR)/mcu.o:$(PWD)/mcu/mcu.c
	$(CC) -c $(CFLAGS) $< -o $@
endif
$(OBJ_DIR)/watch_task.o:$(PWD)/main_flow/watch_task/watch_task.c
	$(CC) -c $(CFLAGS) $< -o $@
#$(OBJ_DIR)/wifi_sta_mode.o:$(PWD)/main_flow/wifi_sta_mode/wifi_sta_mode.c
#	$(CC) -c $(CFLAGS) $< -o $@

$(OBJ_DIR)/version.o:$(PWD)/version.c
	$(CC) -c -I$(SDK_CONF_DIR) $< -o $@


### RESCUE SRC : SD CARD
# $(OBJ_DIR)/main_sd.o:$(PWD)/main_sd.c
# 	$(CC) -c $(CFLAGS) $< -o $@
# $(OBJ_DIR)/init_sd.o:$(PWD)/init_sd.c
# 	$(CC) -c $(CFLAGS) $< -o $@
# $(OBJ_DIR)/main_flow_sd.o:$(PWD)/main_flow/main_flow_sd.c
# 	$(CC) -c $(CFLAGS) $< -o $@


$(OBJ_DIR)/sd_fw_upgrade_rs.o:$(PWD)/main_flow/upgrade/sd_fw_upgrade_rs.c
	$(CC) -c $(CFLAGS) $< -o $@	

### RESCUE SRC : USB DEV
$(OBJ_DIR)/main_rs.o:$(PWD)/main_rs.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/init_rs.o:$(PWD)/init_rs.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/main_flow_rs.o:$(PWD)/main_flow/main_flow_rs.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/usbd_ext_rs.o:$(PWD)/usbd_ext/usbd_ext_rs.c
	$(CC) -c $(CFLAGS) $< -o $@	


clean:
ifeq ($(CONFIG_RESCUE_SYSTEM), y)
	rm -rf $(OBJ_DIR) $(OBJS) $(IMAGE)
	rm -rf $(OBJS_RESCUE) $(IMAGE_RESCUE)
else
	rm -rf $(OBJ_DIR) $(OBJS) $(IMAGE)	
endif

distclean:

install:
ifeq ($(CONFIG_RESCUE_SYSTEM), y)
	[ -d "$(INSTALL_DIR)" ] || mkdir -p $(INSTALL_DIR) 
	cp $(IMAGE) $(INSTALL_DIR)
	cp -f $(IMAGE_BIN) $(INSTALL_DIR)
	cp $(IMAGE_RESCUE) $(INSTALL_DIR)
	cp -f $(IMAGE_BIN_RESCUE) $(INSTALL_DIR)
else
	[ -d "$(INSTALL_DIR)" ] || mkdir -p $(INSTALL_DIR) 
	cp $(IMAGE) $(INSTALL_DIR)
	cp -f $(IMAGE_BIN) $(INSTALL_DIR)
endif	

