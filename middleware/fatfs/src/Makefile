PWD		:= $(shell pwd)
INSTALL_DIR	?= $(PWD)/target
INSTALL_INC_DIR	:= $(INSTALL_DIR)/include/libmid_fatfs
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
CFLAGS		+= -I$(SDK_CONF_DIR)

STATIC_LIB = $(OBJ_DIR)/libmid_fatfs.a

OBJS_FILES = ff.o diskio.o unicode.o ffcli.o version.o
OBJS = $(addprefix $(OBJ_DIR)/, $(OBJS_FILES))

INC_FILES = ff.h ffconf.h integer.h

.PHONY:all clean distclean install    
all: $(OBJ_DIR) $(STATIC_LIB)
$(OBJ_DIR):
	[ -d "$@" ] || mkdir -p $@
$(STATIC_LIB):$(OBJS)
	$(AR) -r $@ $(OBJS)

$(OBJ_DIR)/%.o:$(PWD)/%.c
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJ_DIR)/unicode.o:$(PWD)/option/unicode.c
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJ_DIR)/version.o:$(PWD)/version.c
	$(CC) -c -I$(SDK_CONF_DIR) $^ -o $@
	
clean:
	rm -rf $(OBJ_DIR) $(OBJS) $(STATIC_LIB)

distclean:

install:
	[ -d "$(INSTALL_INC_DIR)" ] || mkdir -p $(INSTALL_INC_DIR)
	[ -d "$(INSTALL_LIB_DIR)" ] || mkdir -p $(INSTALL_LIB_DIR)
	cp $(STATIC_LIB) $(INSTALL_LIB_DIR)
	cp -af $(INC_FILES) $(INSTALL_INC_DIR)
