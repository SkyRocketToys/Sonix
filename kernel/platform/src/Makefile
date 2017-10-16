PWD		:= $(shell pwd)
INSTALL_DIR	?= $(PWD)/target
INSTALL_INC_DIR	:= $(INSTALL_DIR)/include
INSTALL_LIB_DIR := $(INSTALL_DIR)/lib
OBJ_DIR		?= $(PWD)/obj

-include $(SDK_CONF_DIR)/config/snx_sdk.conf
ARCH            ?= arm
TARGET          ?= $(ARCH)-linux
CROSS_COMPILE   ?= $(TARGET)-
CC		= $(CROSS_COMPILE)gcc
AR		= $(CROSS_COMPILE)ar

CFLAGS		= $(PRJ_CFLAGS) -nostdinc
CFLAGS		+= -I$(PWD)
CFLAGS		+= -I$(KERNEL_INC_DIR))

#Default
PLATFORM=sn9866x

ifeq ($(CONFIG_SYSTEM_PLATFORM_SN98660), y)
PLATFORM=sn9866x
endif

ifeq ($(CONFIG_SYSTEM_PLATFORM_SN98661), y)
PLATFORM=sn9866x
endif

ifeq ($(CONFIG_SYSTEM_PLATFORM_SN98670), y)
PLATFORM=sn9866x
endif

ifeq ($(CONFIG_SYSTEM_PLATFORM_SN98671), y)
PLATFORM=sn9866x
endif

ifeq ($(CONFIG_SYSTEM_PLATFORM_SN98672), y)
PLATFORM=sn9866x
endif

ifeq ($(CONFIG_SYSTEM_PLATFORM_SN98293), y)
PLATFORM=sn9866x
endif

.PHONY:all clean distclean install    
all:
	#ls -l
	#echo $(CONFIG_SYSTEM_PLATFORM)
clean:
	rm -rf $(OBJ_DIR)

distclean:

install:
	[ -d "$(INSTALL_INC_DIR)" ] || mkdir -p $(INSTALL_INC_DIR)
	cp -f $(PLATFORM)/register.h $(INSTALL_INC_DIR)
