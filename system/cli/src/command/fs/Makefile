PWD             := $(shell pwd)
INSTALL_DIR     ?= $(PWD)/target
INSTALL_INC_DIR := $(INSTALL_DIR)/include/cli
INSTALL_LIB_DIR := $(INSTALL_DIR)/lib
OBJ_DIR         ?=$(PWD)/obj

ARCH            ?= arm
TARGET          ?= $(ARCH)-linux
CROSS_COMPILE   ?= $(TARGET)-
CC              = $(CROSS_COMPILE)gcc
AR              = $(CROSS_COMPILE)ar

CFLAGS          = $(PRJ_CFLAGS) -nostdinc
CFLAGS          += -I$(PWD)
CFLAGS          += -I$(PWD)/../../
CFLAGS          += -I$(PWD)/../../include
CFLAGS          += -I$(KERNEL_INC_DIR)
CFLAGS          += -I$(DRIVER_INC_DIR)
CFLAGS          += -I$(MIDDLEWARE_INC_DIR) -I$(MIDDLEWARE_LIBC_INC_DIR) -I$(MIDDLEWARE_INC_DIR)/libmid_cyassl -I$(MIDDLEWARE_INC_DIR)/libmid_lwip/include -I$(MIDDLEWARE_INC_DIR)/libmid_lwip/include/ipv4
CFLAGS          += -I$(SDK_CONF_DIR)

OBJ = cmd_fs.o

OBJ_CMD= $(addprefix $(OBJ_DIR)/, $(OBJ))


#$(OBJ_DIR)/cmd_debug.o:$(PWD)/cmd_debug.c
all:$(OBJ_CMD)


$(OBJ_DIR)/cmd_fs.o:cmd_fs.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
#$(OBJ):
#	$(CC) $(CFLAGS) -c $*.c

