PWD		:= $(shell pwd)
INSTALL_DIR	?= $(PWD)/target
INSTALL_INC_DIR	:= $(INSTALL_DIR)/include/libmid_websockets
INSTALL_LIB_DIR := $(INSTALL_DIR)/lib
OBJ_DIR		?=$(PWD)/obj

ARCH            ?= arm
TARGET          ?= $(ARCH)-linux
CROSS_COMPILE   ?= $(TARGET)-
CC		= $(CROSS_COMPILE)gcc
AR		= $(CROSS_COMPILE)ar

CFLAGS		= $(PRJ_CFLAGS)
CFLAGS		+= -I$(KERNEL_INC_DIR)
CFLAGS		+= -I$(DRIVER_INC_DIR)
CFLAGS		+= -I$(MIDDLEWARE_INC_DIR) -I$(MIDDLEWARE_INC_DIR)/libmid_lwip/include -I$(MIDDLEWARE_INC_DIR)/libmid_lwip/include/ipv4
CFLAGS		+= -I$(MIDDLEWARE_LIBC_INC_DIR)

LDFLAGS		= $(PRJ_LDFLAGS)
LDFLAGS		+= -L $(DRIVER_LIB_DIR)
LDFLAGS		+= -L $(KERNEL_LIB_DIR)
LDFLAGS		+= -L $(MIDDLEWARE_LIB_DIR)
LDFLAGS		+= -lmid_lwip

STATIC_LIB = $(OBJ_DIR)/libmid_websockets.a

OBJS_FILES = client.o common.o utf8.o version.o

OBJS = $(addprefix $(OBJ_DIR)/, $(OBJS_FILES))



.PHONY:all clean distclean install    
all:$(OBJ_DIR) $(STATIC_LIB)
$(OBJ_DIR):
	[ -d "$@" ] || mkdir -p $@

$(STATIC_LIB):$(OBJS)
	$(AR) -r $@ $(OBJS)


$(OBJ_DIR)/common.o:$(PWD)/common.c
	$(CC) -c $(CFLAGS) $^ -o $@
$(OBJ_DIR)/utf8.o:$(PWD)/utf8.c
	$(CC) -c $(CFLAGS) $^ -o $@

$(OBJ_DIR)/client.o:$(PWD)/client.c
	$(CC) -c $(CFLAGS) $^ -o $@	

$(OBJ_DIR)/version.o:$(PWD)/version.c
	$(CC) -c -I$(SDK_CONF_DIR) $^ -o $@


clean:
	rm -rf $(OBJ_DIR) $(OBJS) $(STATIC_LIB)

distclean:

install:
	[ -d "$(INSTALL_INC_DIR)" ] || mkdir -p $(INSTALL_INC_DIR)
	cp common.h utf8.h client.h $(INSTALL_INC_DIR)
	[ -d "$(INSTALL_LIB_DIR)" ] || mkdir -p $(INSTALL_LIB_DIR)
	cp $(STATIC_LIB) $(INSTALL_LIB_DIR)
