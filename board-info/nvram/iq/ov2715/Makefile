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


IQS	:= FHD30 HD60 


.PHONY:all clean distclean install
all:
	truncate -s 0 size.h;
	for ss in $(IQS); do wc -c $$ss.bin | awk '{printf("#define '$$ss'_SIZE %d\n", $$1)}' >> size.h; done
	$(CC) -c $(CFLAGS) iq.S -o iq.o
	arm-linux-objcopy --gap-fill=0xff -O binary iq.o IQ.bin

clean:
	rm -rf size.h iq.o IQ.bin


distclean:


