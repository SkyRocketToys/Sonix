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
CFLAGS		+= -I$(PWD)/usbd_ext/
CFLAGS		+= -I$(MIDDLEWARE_INC_DIR)/libmid_lwip/include -I$(MIDDLEWARE_INC_DIR)/libmid_lwip/include/ipv4
CFLAGS		+= -I$(MIDDLEWARE_INC_DIR)/libmid_cyassl
CFLAGS		+= -I$(MIDDLEWARE_INC_DIR)/libmid_websockets
CFLAGS		+= -I$(MIDDLEWARE_INC_DIR)/libmid_rtsp_server
CFLAGS		+= -I$(MIDDLEWARE_INC_DIR)/libmid_automount
CFLAGS		+= -I$(SDK_CONF_DIR)

-include $(SDK_CONF_DIR)/config/snx_sdk.conf
LDFLAGS		= $(PRJ_LDFLAGS)
LDFLAGS		+= -L $(DRIVER_LIB_DIR)
LDFLAGS		+= -L $(KERNEL_LIB_DIR)
LDFLAGS		+= -L $(MIDDLEWARE_LIB_DIR)

LDFLAGS		+= --start-group -lkernel -ltimer -lcli -lbootinfo \
		-lmac -luart -lmid_nonstdc -lvc -lmid_vc -lmid_isp -lmid_rtsp_server \
		-li2c -lsensor -lisp -laudio_dri -lmid_audio -lmid_aec -lsf -lsd \
		-lmid_sd -lmid_sf -lmid_fatfs -lusb_host -lmid_usbd -lusb_device \
		-lmid_nvram -lmid_json -lmid_rec -lmid_lwip -lwifi -lpwm -lgpio \
		-lmid_cyassl -lfdk-aac -lmid_fwupgrade -lwdt -lz -lmid_aac -lmid_aud32 \
		-lmid_alaw -lmid_mulaw -lmid_g726 -lmid_amr-nb -lmid_amrwb \
		-lmid_automount
		
ifeq ($(CONFIG_SYSTEM_TRACE_SELECT), y)
LDFLAGS += -ltrace
endif

ifeq ($(CONFIG_MIDDLEWARE_MCU_CTRL), y)
LDFLAGS += -lmid_mcu
endif

LDFLAGS         += --end-group
LDFLAGS         += -lgcc

LDFLAGS_RESCUE		= $(PRJ_LDFLAGS)
LDFLAGS_RESCUE		+= -L $(DRIVER_LIB_DIR_RESCUE)
LDFLAGS_RESCUE		+= -L $(KERNEL_LIB_DIR_RESCUE)
LDFLAGS_RESCUE		+= -L $(MIDDLEWARE_LIB_DIR_RESCUE)


LDFLAGS_RESCUE		+= --start-group -lkernel -ltimer -lcli -lbootinfo -lmac \
			-luart -lmid_nonstdc -lvc -lmid_vc -lmid_isp -lmid_rtsp_server -li2c \
			-lsensor -lisp -laudio_dri -lmid_audio -lsf -lsd -lmid_sd -lmid_sf \
			-lmid_fatfs -lusb_host -lmid_usbd -lusb_device -lmid_nvram  \
			-lmid_json -lmid_rec -lmid_lwip -lwifi -lpwm -lgpio -lmid_td \
			-lmid_cyassl -lfdk-aac -lz -lmid_aac


ifeq ($(CONFIG_SYSTEM_TRACE_SELECT), y)
LDFLAGS_RESCUE += -ltrace
endif

ifeq ($(CONFIG_MIDDLEWARE_MCU_CTRL), y)
LDFLAGS_RESCUE += -lmcu
endif

LDFLAGS_RESCUE		+= --end-group
LDFLAGS_RESCUE		+= -lgcc


IMAGE = $(OBJ_DIR)/image.elf
IMAGE_BIN = $(OBJ_DIR)/image.bin
IMAGE_RESCUE = $(OBJ_DIR)/image_rescue.elf
IMAGE_BIN_RESCUE = $(OBJ_DIR)/image_rescue.bin


OBJ_FILES = main.o init.o usbd_ext.o snx_xu_mpt.o usbd_uvc_vc.o
ifeq ($(CONFIG_MIDDLEWARE_MCU_CTRL), y)
OBJ_FILES += mcu.o
endif

OBJ_FILES += version.o

OBJS = $(addprefix $(OBJ_DIR)/, $(OBJ_FILES))

OBJ_FILES_RESCUE = main.o init.o usbd_ext.o snx_xu_mpt.o usbd_uvc_vc.o
OBJ_FILES_RESCUE += version.o
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
$(OBJ_DIR)/usbd_ext.o:$(PWD)/usbd_ext/usbd_ext.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/snx_xu_mpt.o:$(PWD)/usbd_ext/snx_xu_mpt.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/usbd_uvc_vc.o:$(PWD)/usbd_ext/usbd_uvc_vc.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/mcu.o:$(PWD)/mcu.c
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJ_DIR)/version.o:$(PWD)/version.c
	$(CC) -c -I$(SDK_CONF_DIR) $^ -o $@

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
