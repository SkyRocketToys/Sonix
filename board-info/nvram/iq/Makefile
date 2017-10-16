PWD		:= $(shell pwd)
INSTALL_DIR	?= $(PWD)/target
INSTALL_INC_DIR	:= $(INSTALL_DIR)/include/iq
INSTALL_LIB_DIR := $(INSTALL_DIR)/lib
OBJ_DIR		?=$(PWD)/obj

ARCH            ?= arm
TARGET          ?= $(ARCH)-linux
CROSS_COMPILE   ?= $(TARGET)-
CC		= $(CROSS_COMPILE)gcc
AR		= $(CROSS_COMPILE)ar

CFLAGS		= $(PRJ_CFLAGS)
CFLAGS		+= -I$(PWD)
CFLAGS		+= -I$(SDK_CONF_DIR)


-include $(SDK_CONF_DIR)/config/snx_sdk.conf

INS_SENSOR		:=

ifdef CONFIG_MODULE_SENSOR_OV9715
INS_SENSOR		+= ov9715
endif

ifdef CONFIG_MODULE_SENSOR_OV2715
INS_SENSOR		+= ov2715
endif

ifdef CONFIG_MODULE_SENSOR_OV9750
INS_SENSOR		+= ov9750
endif

ifdef CONFIG_MODULE_SENSOR_IMX238
INS_SENSOR		+= imx238
endif

ifdef CONFIG_MODULE_SENSOR_AR0330
INS_SENSOR		+= ar0330
endif

ifdef CONFIG_MODULE_SENSOR_TW9912
INS_SENSOR		+= tw9912
endif

ifdef CONFIG_MODULE_SENSOR_IMX225
INS_SENSOR		+= imx225
endif

ifdef CONFIG_MODULE_SENSOR_IMX322
INS_SENSOR      += imx322
endif

ifdef CONFIG_MODULE_SENSOR_CS1211K
INS_SENSOR		+= cs1211k
endif

ifdef CONFIG_MODULE_SENSOR_H42
INS_SENSOR              += h42
endif

ifdef CONFIG_MODULE_SENSOR_H62
INS_SENSOR              += h62
endif

ifdef CONFIG_MODULE_SENSOR_GC1024
INS_SENSOR              += gc1024
endif

ifdef CONFIG_MODULE_SENSOR_OV9732
INS_SENSOR              += ov9732
endif

ifdef CONFIG_MODULE_SENSOR_SC2035
INS_SENSOR              += sc2035
endif

ifdef CONFIG_MODULE_SENSOR_AR0237
INS_SENSOR		+= ar0237
endif

ifdef CONFIG_MODULE_SENSOR_HM2130
INS_SENSOR              += hm2130
endif

ifdef CONFIG_MODULE_SENSOR_OV2740
INS_SENSOR              += ov2740
endif

ifdef CONFIG_MODULE_SENSOR_OV7725
INS_SENSOR              += ov7725
endif

ifdef CONFIG_MODULE_SENSOR_GC0308
INS_SENSOR              += gc0308
endif

.PHONY:all clean distclean install
all:
	echo '#define RES_IQ_SIZE CONFIG_MAX_IQ_SIZE*1024' > sensor.h
	for ss in $(INS_SENSOR); do $(MAKE) -C $$ss; wc -c $$ss/IQ.bin | awk '{printf("#define '$$ss'_SIZE %d\n", $$1)}' >> sensor.h; done	
	$(CC) -c $(CFLAGS) iq.S -o iq.o
	arm-linux-objcopy --gap-fill=0xff -O binary iq.o IQ.bin

clean:
	for ss in $(INS_SENSOR); do $(MAKE) -C $$ss clean; done
	rm -rf sensor.h iq.o IQ.bin


distclean:


