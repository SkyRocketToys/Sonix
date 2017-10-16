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

CFLAGS		= $(PRJ_CFLAGS) 
CFLAGS		+= -I$(KERNEL_INC_DIR)
CFLAGS		+= -I$(DRIVER_INC_DIR)
CFLAGS		+= -I$(MIDDLEWARE_INC_DIR)
CFLAGS		+= -I$(MIDDLEWARE_LIBC_INC_DIR)
CFLAGS		+= -I$(SDK_CONF_DIR)

-include $(SDK_CONF_DIR)/config/snx_sdk.conf
LDFLAGS		= $(PRJ_LDFLAGS)
LDFLAGS		+= -L $(DRIVER_LIB_DIR)
LDFLAGS		+= -L $(KERNEL_LIB_DIR)
LDFLAGS		+= -L $(MIDDLEWARE_LIB_DIR)

LDFLAGS		+= --start-group -lkernel -ltimer -lcli -luart -lmid_nonstdc -lvc -lmid_vc -lmid_isp \
		-lmid_rtsp_server -li2c -lsensor -lisp -laudio_dri -lmid_audio -lmid_aec -lsf -lsd -lmid_sd -lmid_sf \
		-lmid_fatfs -lusb_host -lmid_usbd -lusb_device -lmid_nvram -lmid_rec -lmid_lwip -lwifi \
		-lpwm -lgpio -lmid_cyassl -lfdk-aac -lmid_fwupgrade -lrtc -lwdt -lmid_automount -lz -lmid_aac  -lmid_aud32 -lmid_alaw -lmid_mulaw -lmid_g726 -lmid_amr-nb -lmid_amrwb

ifeq ($(CONFIG_MODULE_MAC_SUPPORT), y)
LDFLAGS += -lmac
endif

ifeq ($(CONFIG_MIDDLEWARE_ZBAR), y)
LDFLAGS += -lzbar
endif

ifeq ($(CONFIG_MIDDLEWARE_TONE_DETECTION), y)
LDFLAGS += -lmid_td
endif

ifeq ($(CONFIG_MIDDLEWARE_EASYSETUP), y)
LDFLAGS += -lmid_snx-ez
endif

ifeq ($(CONFIG_SYSTEM_TRACE_SELECT), y)
LDFLAGS += -ltrace
endif

ifeq ($(CONFIG_MIDDLEWARE_MCU_CTRL), y)
LDFLAGS += -lmid_mcu
endif

LDFLAGS		+= --end-group
LDFLAGS		+= -lgcc

IMAGE = $(OBJ_DIR)/image.elf
IMAGE_BIN = $(OBJ_DIR)/image.bin

OBJ_FILES = main.o init.o
OBJ_FILES += version.o

ifeq ($(CONFIG_MIDDLEWARE_MCU_CTRL), y)
OBJ_FILES += mcu.o
endif

OBJS = $(addprefix $(OBJ_DIR)/, $(OBJ_FILES))

.PHONY: all clean distclean install    
all:$(OBJ_DIR) $(IMAGE)
$(OBJ_DIR):
	[ -d "$@" ] || mkdir -p $@
$(IMAGE):$(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -o $(IMAGE)
	$(OBJCOPY) -O binary -R .note -R .comment -S $(IMAGE) $(IMAGE_BIN)

$(OBJ_DIR)/main.o:$(PWD)/main.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/init.o:$(PWD)/init.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/mcu.o:$(PWD)/mcu.c
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJ_DIR)/version.o:$(PWD)/version.c
	$(CC) -c -I$(SDK_CONF_DIR) $^ -o $@

#$(OBJ_DIR)/printlog.o:$(PWD)/printlog.c
#	$(CC) -c $(CFLAGS) $< -o $@
clean:
	rm -rf $(OBJ_DIR) $(OBJS) $(IMAGE)

distclean:

install:
	[ -d "$(INSTALL_DIR)" ] || mkdir -p $(INSTALL_DIR) 
	cp $(IMAGE) $(INSTALL_DIR)
	cp -f $(IMAGE_BIN) $(INSTALL_DIR)
	
