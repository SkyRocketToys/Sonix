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
CFLAGS		+= -I$(PWD)/main_flow/audio
CFLAGS		+= -I$(PWD)/main_flow/upgrade
CFLAGS		+= -I$(PWD)/usbd_ext/

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
CFLAGS		+= -I$(MIDDLEWARE_INC_DIR)/libmid_cyassl
CFLAGS		+= -I$(MIDDLEWARE_INC_DIR)/libmid_automount
CFLAGS		+= -I$(MIDDLEWARE_INC_DIR)/libmid_rtsp_server
CFLAGS		+= -I$(SDK_CONF_DIR)

CFLAGS		+= -Werror

LDFLAGS		= $(PRJ_LDFLAGS)
LDFLAGS		+= -L $(DRIVER_LIB_DIR)
LDFLAGS		+= -L $(KERNEL_LIB_DIR)
LDFLAGS		+= -L $(MIDDLEWARE_LIB_DIR)
LDFLAGS		+= --start-group -lkernel -ltimer -lcli -lbootinfo -luart -lmid_nonstdc -lvc -lmid_vc -lmid_isp \
		-lmid_rtsp_server -li2c -lsensor -lisp -laudio_dri -lmid_audio -lsf -lsd -lmid_sd -lmid_sf \
		-lmid_fatfs -lusb_host -lmid_usbd -lusb_device -lmid_nvram -lmid_json -lmid_rec -lmid_lwip -lwifi \
		-lpwm -lgpio -lmid_cyassl -lmid_fwupgrade -lwdt -lmid_automount -lrtc
#endif

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

LDFLAGS_RESCUE		= $(PRJ_LDFLAGS)
LDFLAGS_RESCUE		+= -L $(DRIVER_LIB_DIR_RESCUE)
LDFLAGS_RESCUE		+= -L $(KERNEL_LIB_DIR_RESCUE)
LDFLAGS_RESCUE		+= -L $(MIDDLEWARE_LIB_DIR_RESCUE)
ifeq ($(CONFIG_SYSTEM_TRACE_SELECT), y)
LDFLAGS_RESCUE 		+= --start-group -lkernel -ltimer -lcli -lbootinfo -ltrace -lmac -luart -lmid_nonstdc -lvc -lmid_vc -lmid_isp -lmid_rtsp_server -li2c -lsensor -lisp -laudio_dri -lmid_audio -lsf -lsd -lmid_sd -lmid_sf -lmid_fatfs -lusb_host -lmid_usbd -lusb_device -lmid_nvram  -lmid_json -lmid_rec -lmid_lwip -lwifi -lpwm -lgpio -ltd -lmid_cyassl -ltd_decode -lfdk-aac -lautomount -lmid_aac --end-group
else
LDFLAGS_RESCUE 		+= --start-group -lkernel -ltimer -lcli -lbootinfo -lmac -luart -lmid_nonstdc -lvc -lmid_vc -lmid_isp -lmid_rtsp_server -li2c -lsensor -lisp -laudio_dri -lmid_audio -lsf -lsd -lmid_sd -lmid_sf -lmid_fatfs -lusb_host -lmid_usbd -lusb_device -lmid_nvram -lmid_json -lmid_rec -lmid_lwip -lwifi -lpwm -lgpio -ltd -lmid_cyassl -ltd_decode -lfdk-aac -lautomount -lmid_aac --end-group
endif
ifeq ($(CONFIG_MIDDLEWARE_ZLIB), y)
LDFLAGS_RESCUE		+= -lz
else
LDFLAGS_RESCUE		+= -lgzip
endif
LDFLAGS_RESCUE		+= -lgcc


IMAGE = $(OBJ_DIR)/image.elf
IMAGE_BIN = $(OBJ_DIR)/image.bin
IMAGE_RESCUE = $(OBJ_DIR)/image_rescue.elf
IMAGE_BIN_RESCUE = $(OBJ_DIR)/image_rescue.bin


OBJ_FILES = main.o init.o main_flow.o usbd_ext.o
OBJ_FILES += sd_fw_upgrade.o version.o

ifeq ($(CONFIG_MIDDLEWARE_MCU_CTRL), y)
endif

ifdef CONFIG_UDP_FLOW

endif

OBJS = $(addprefix $(OBJ_DIR)/, $(OBJ_FILES))

OBJ_FILES_RESCUE = main.o init.o main_flow.o usbd_ext.o 
OBJS_RESCUE = $(addprefix $(OBJ_DIR)/, $(OBJ_FILES))


.PHONY: all clean distclean install

ifeq ($(CONFIG_RESCUE_SYSTEM), y)
all:$(OBJ_DIR) $(IMAGE) $(IMAGE_RESCUE)
else
all:$(OBJ_DIR) $(IMAGE)
endif



$(OBJ_DIR):
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

$(OBJ_DIR)/sd_fw_upgrade.o:$(PWD)/main_flow/upgrade/sd_fw_upgrade.c
	$(CC) -c $(CFLAGS) $< -o $@	

$(OBJ_DIR)/usbd_ext.o:$(PWD)/usbd_ext/usbd_ext.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/version.o:$(PWD)/version.c
	$(CC) -c -I$(SDK_CONF_DIR) $< -o $@
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
