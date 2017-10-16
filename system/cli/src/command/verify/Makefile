-include $(SDK_CONF_DIR)/config/snx_sdk.conf

PWD             := $(shell pwd)
INSTALL_DIR     ?= $(PWD)/target
INSTALL_INC_DIR := $(INSTALL_DIR)/include/cli
INSTALL_LIB_DIR := $(INSTALL_DIR)/lib
OBJ_DIR         ?=$(PWD)/obj

ARCH            ?= arm
TARGET          ?= $(ARCH)-linux
CROSS_COMPILE   ?= $(TARGET)-
CC              = $(CROSS_COMPILE)gcc
LD              = $(CROSS_COMPILE)ld
AR              = $(CROSS_COMPILE)ar

CFLAGS          = $(PRJ_CFLAGS) -nostdinc
CFLAGS          += -I$(PWD)
CFLAGS          += -I$(PWD)/../../
CFLAGS          += -I$(PWD)/../../include
CFLAGS          += -I$(KERNEL_INC_DIR)
CFLAGS          += -I$(DRIVER_INC_DIR)
CFLAGS          += -I$(MIDDLEWARE_INC_DIR) -I$(MIDDLEWARE_LIBC_INC_DIR) -I$(MIDDLEWARE_INC_DIR)/libmid_cyassl -I$(MIDDLEWARE_INC_DIR)/libmid_lwip/include -I$(MIDDLEWARE_INC_DIR)/libmid_lwip/include/ipv4 -I$(MIDDLEWARE_INC_DIR)/fdk-aac -I$(MIDDLEWARE_INC_DIR)/libmid_audio -I$(DRIVER_INC_DIR)/wifi -I$(MIDDLEWARE_INC_DIR)/libmid_snx-ez/include
CFLAGS          += -I$(SDK_CONF_DIR)

#OBJ = cmd_verify.o snx_audio_aac.o

OBJ = cmd_verify.o

ifeq ($(CONFIG_CLI_CMD_VERIFY_MEMTEST),y)
OBJ += cmd_verify_memtest.o
endif

ifeq ($(CONFIG_CLI_CMD_VERIFY_TDTEST),y)
OBJ += cmd_verify_td_test.o
endif

ifeq ($(CONFIG_CLI_CMD_VERIFY_AUDIO),y)
OBJ += cmd_verify_audio.o
endif

ifeq ($(CONFIG_CLI_CMD_VERIFY_FS),y)
OBJ += cmd_verify_fs.o
endif

ifeq ($(CONFIG_CLI_CMD_VERIFY_I2C),y)
OBJ += cmd_verify_i2c.o
endif

ifeq ($(CONFIG_CLI_CMD_VERIFY_SF),y)
OBJ += cmd_verify_sf.o
endif

ifeq ($(CONFIG_CLI_CMD_VERIFY_SD),y)
OBJ += cmd_verify_sd.o
endif

ifeq ($(CONFIG_CLI_CMD_VERIFY_VIDEO),y)
OBJ += cmd_verify_video.o
endif

ifeq ($(CONFIG_CLI_CMD_VERIFY_EASYSETUP),y)
OBJ += cmd_verify_easysetup.o
endif
OBJ_CMD = $(addprefix $(OBJ_DIR)/, $(OBJ))

all:$(OBJ)
	$(LD) -r $(OBJ) -o $(OBJ_DIR)/cmd_verify.o


$(OBJ):
	$(CC) $(CFLAGS) -c $*.c


clean:
	rm -f *.o
#all:
#	@for f in $(CMDS) ; do $(CC) -c $(CFLAGS) $$f.c -o $(OBJ_DIR)/$$f.o; done

