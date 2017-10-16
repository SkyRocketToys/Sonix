-include $(SDK_CONF_DIR)/config/snx_sdk.conf

PWD		:= $(shell pwd)
INSTALL_DIR	?= $(PWD)/target
INSTALL_INC_DIR	:= $(INSTALL_DIR)/include/cli
INSTALL_LIB_DIR := $(INSTALL_DIR)/lib
OBJ_DIR		?=$(PWD)/obj

ARCH            ?= arm
TARGET          ?= $(ARCH)-linux
CROSS_COMPILE   ?= $(TARGET)-
CC		= $(CROSS_COMPILE)gcc
AR		= $(CROSS_COMPILE)ar

CFLAGS		= $(PRJ_CFLAGS) -nostdinc
CFLAGS		+= -I$(PWD)
CFLAGS		+= -I$(PWD)/include
CFLAGS		+= -I$(KERNEL_INC_DIR)
CFLAGS		+= -I$(DRIVER_INC_DIR)
CFLAGS		+= -I$(MIDDLEWARE_INC_DIR) -I$(MIDDLEWARE_LIBC_INC_DIR) -I$(MIDDLEWARE_INC_DIR)/libmid_cyassl -I$(MIDDLEWARE_INC_DIR)/libmid_lwip/include -I$(MIDDLEWARE_INC_DIR)/libmid_lwip/include/ipv4 -I$(MIDDLEWARE_INC_DIR)/libmid_td
CFLAGS		+= -I$(SDK_CONF_DIR)

STATIC_LIB = $(OBJ_DIR)/libcli.a
STATIC_LIB_RS = $(OBJ_DIR)/libcli_rs.a


OBJS_FILES = printlog.o rtoscli.o receive.o
OBJS_FILES += version.o

OBJS_FILES_RS = printlog.o rtoscli_rs.o receive.o
OBJS_FILES_RS += version.o



ifeq ($(CONFIG_CLI_CMD_UART),y)
SUBDIR += command/uart
endif

ifeq ($(CONFIG_CLI_CMD_VIDEO),y)
SUBDIR += command/video
endif

ifeq ($(CONFIG_CLI_CMD_DEBUG),y)
SUBDIR += command/debug
endif

ifeq ($(CONFIG_CLI_CMD_SYSTEM),y)
SUBDIR += command/system
endif

ifeq ($(CONFIG_CLI_CMD_STATUS),y)
SUBDIR += command/status
endif

ifeq ($(CONFIG_CLI_CMD_NET),y)
SUBDIR += command/net
endif

ifeq ($(CONFIG_CLI_CMD_GPIO),y)
SUBDIR += command/gpio
endif

ifeq ($(CONFIG_CLI_CMD_FS),y)
SUBDIR += command/fs
endif

ifeq ($(CONFIG_CLI_CMD_VERIFY),y)
SUBDIR += command/verify
endif

ifeq ($(CONFIG_CLI_CMD_SD),y)
SUBDIR += command/sd
endif

ifeq ($(CONFIG_CLI_CMD_I2C),y)
SUBDIR += command/i2c
endif

ifeq ($(CONFIG_CLI_CMD_AUDIO),y)
SUBDIR += command/audio
endif

ifeq ($(CONFIG_CLI_CMD_PWM),y)
SUBDIR += command/pwm
endif

ifeq ($(CONFIG_CLI_CMD_QRSCAN),y)
SUBDIR += command/qrscan
endif

ifeq ($(CONFIG_CLI_CMD_USBD),y)
SUBDIR += command/usbd
endif

ifeq ($(CONFIG_CLI_CMD_USBH),y)
SUBDIR += command/usbh
endif

ifeq ($(CONFIG_CLI_CMD_NVRAM),y)
SUBDIR += command/nvram
endif

ifeq ($(CONFIG_CLI_CMD_ISP),y)
SUBDIR += command/isp
endif

ifeq ($(CONFIG_CLI_CMD_SD),y)
SUBDIR += command/sd
endif

ifeq ($(CONFIG_CLI_CMD_RTSP),y)
SUBDIR += command/rtsp
endif

ifeq ($(CONFIG_CLI_CMD_CONFIG),y)
SUBDIR += command/config
endif

ifeq ($(CONFIG_CLI_CMD_DEMUX_MP4),y)
SUBDIR += command/demux_mp4
endif

ifeq ($(CONFIG_CLI_CMD_RECORD_MP4),y)
SUBDIR += command/record_mp4
endif


ifeq ($(CONFIG_CLI_CMD_WIFI),y)
SUBDIR += command/wifi
endif

ifeq ($(CONFIG_CLI_CMD_DASHCAM),y)
SUBDIR += command/dashcam
endif

ifeq ($(CONFIG_CLI_CMD_TONE),y)
SUBDIR += command/tone_detection
endif


OBJS = $(addprefix $(OBJ_DIR)/, $(OBJS_FILES))
OBJS_RS = $(addprefix $(OBJ_DIR)/, $(OBJS_FILES_RS))

.PHONY:all clean distclean install    
all: $(OBJ_DIR) $(STATIC_LIB_RS) $(STATIC_LIB)
$(OBJ_DIR):
	[ -d "$@" ] || mkdir -p $@

$(STATIC_LIB):$(OBJS) subdir
	$(AR) -r $@ $(OBJ_DIR)/*.o

$(STATIC_LIB_RS):$(OBJS_RS)
	$(AR) -r $@ $^


subdir:
	@for dir in $(SUBDIR) ; do $(MAKE) -C $$dir || exit 1 ; done


$(OBJ_DIR)/rtoscli.o:$(PWD)/rtoscli.c
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJ_DIR)/printlog.o:$(PWD)/printlog.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/receive.o:$(PWD)/receive.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/version.o:$(PWD)/version.c
	$(CC) -c -O0 -I$(SDK_CONF_DIR) $^ -o $@

$(OBJ_DIR)/rtoscli_rs.o:$(PWD)/rtoscli_rs.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(OBJS) $(STATIC_LIB) $(STATIC_LIB_RS) $(OBJS_RS)
	@for dir in $(SUBDIR) ; do $(MAKE) -C $$dir clean || exit 1 ; done

distclean:

install:
	[ -d "$(INSTALL_INC_DIR)" ] || mkdir -p $(INSTALL_INC_DIR)
	[ -d "$(INSTALL_LIB_DIR)" ] || mkdir -p $(INSTALL_LIB_DIR)
	cp -af $(STATIC_LIB) $(INSTALL_LIB_DIR)
	cp -af $(STATIC_LIB_RS) $(INSTALL_LIB_DIR)
	cp -af $(PWD)/*.h $(INSTALL_INC_DIR)
