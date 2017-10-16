-include $(AUTOCONF_DIR)/config/snx_sdk.conf

PWD		:= $(shell pwd)
INSTALL_DIR	?= $(PWD)/target
INSTALL_INC_DIR	:= $(INSTALL_DIR)/include/libmid_json
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
CFLAGS		+= -I$(MIDDLEWARE_INC_DIR)
CFLAGS		+= -I$(MIDDLEWARE_LIBC_INC_DIR)

STATIC_LIB = $(OBJ_DIR)/libmid_json.a

OBJS_FILES = snx_json.o arraylist.o json_object.o json_tokener.o linkhash.o printbuf.o random_seed.o config.o version.o

OBJS = $(addprefix $(OBJ_DIR)/, $(OBJS_FILES))

.PHONY:all clean distclean install    
all:$(OBJ_DIR) $(STATIC_LIB)
$(OBJ_DIR):
	[ -d "$@" ] || mkdir -p $@

$(STATIC_LIB):$(OBJS)
	$(AR) -r $@ $(OBJS)

$(OBJ_DIR)/snx_json.o:$(PWD)/snx_json.c
	$(CC) -c $(CFLAGS) $^ -o $@	
$(OBJ_DIR)/arraylist.o:$(PWD)/arraylist.c
	$(CC) -c $(CFLAGS) $^ -o $@
$(OBJ_DIR)/json_object.o:$(PWD)/json_object.c
	$(CC) -c $(CFLAGS) $^ -o $@
$(OBJ_DIR)/json_tokener.o:$(PWD)/json_tokener.c
	$(CC) -c $(CFLAGS) $^ -o $@
$(OBJ_DIR)/linkhash.o:$(PWD)/linkhash.c
	$(CC) -c $(CFLAGS) $^ -o $@
$(OBJ_DIR)/printbuf.o:$(PWD)/printbuf.c
	$(CC) -c $(CFLAGS) $^ -o $@
$(OBJ_DIR)/random_seed.o:$(PWD)/random_seed.c
	$(CC) -c $(CFLAGS) $^ -o $@
$(OBJ_DIR)/config.o:$(PWD)/private/config.c
	$(CC) -c $(CFLAGS) $^ -o $@
$(OBJ_DIR)/version.o:$(PWD)/version.c
	$(CC) -c -I$(SDK_CONF_DIR) $^ -o $@

clean:
	rm -rf $(OBJ_DIR) $(OBJS) $(STATIC_LIB)

distclean:

install:
	[ -d "$(INSTALL_INC_DIR)" ] || mkdir -p $(INSTALL_INC_DIR)
	cp snx_json.h $(INSTALL_INC_DIR)
	[ -d "$(INSTALL_LIB_DIR)" ] || mkdir -p $(INSTALL_LIB_DIR)
	cp $(STATIC_LIB) $(INSTALL_LIB_DIR)
