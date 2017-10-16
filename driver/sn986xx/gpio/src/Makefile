PWD		:= $(shell pwd)
INSTALL_DIR	?= $(PWD)/target
INSTALL_INC_DIR	:= $(INSTALL_DIR)/include/gpio
INSTALL_LIB_DIR := $(INSTALL_DIR)/lib
OBJ_DIR		?=$(PWD)/obj

ARCH            ?= arm
TARGET          ?= $(ARCH)-linux
CROSS_COMPILE   ?= $(TARGET)-
CC		= $(CROSS_COMPILE)gcc
AR		= $(CROSS_COMPILE)ar

CFLAGS		= $(PRJ_CFLAGS)
CFLAGS		+= -I$(PWD)
CFLAGS		+= -I$(KERNEL_INC_DIR)
CFLAGS		+= -I$(DRIVER_INC_DIR)

STATIC_LIB = $(OBJ_DIR)/libgpio.a

OBJS_FILES = gpio.o version.o
OBJS = $(addprefix $(OBJ_DIR)/, $(OBJS_FILES))

.PHONY:all clean distclean install    
all:$(OBJ_DIR) $(STATIC_LIB)
$(OBJ_DIR):
	[ -d "$@" ] || mkdir -p $@
$(STATIC_LIB):$(OBJS)
	$(AR) -r $@ $^

$(OBJ_DIR)/gpio.o:$(PWD)/gpio.c
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJ_DIR)/version.o:$(PWD)/version.c
	$(CC) -c -I$(SDK_CONF_DIR) $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(OBJS) $(STATIC_LIB)

distclean:

install:
	[ -d "$(INSTALL_INC_DIR)" ] || mkdir -p $(INSTALL_INC_DIR)
	cp gpio.h $(INSTALL_INC_DIR)
	[ -d "$(INSTALL_LIB_DIR)" ] || mkdir -p $(INSTALL_LIB_DIR)
	cp $(STATIC_LIB) $(INSTALL_LIB_DIR)
