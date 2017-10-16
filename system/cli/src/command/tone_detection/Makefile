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
CFLAGS          += -I$(MIDDLEWARE_INC_DIR) -I$(MIDDLEWARE_LIBC_INC_DIR)  -I$(MIDDLEWARE_INC_DIR)/libmid_audio -I$(MIDDLEWARE_INC_DIR)/libmid_td
CFLAGS          += -I$(SDK_CONF_DIR)


OBJ = cmd_tone.o

OBJ_CMD = $(addprefix $(OBJ_DIR)/, $(OBJ))

all:$(OBJ)
	$(LD) -r $(OBJ) -o $(OBJ_DIR)/cmd_tone.o


$(OBJ):
	$(CC) $(CFLAGS) -c $*.c


clean:
	rm -f *.o
#all:
#	@for f in $(CMDS) ; do $(CC) -c $(CFLAGS) $$f.c -o $(OBJ_DIR)/$$f.o; done

