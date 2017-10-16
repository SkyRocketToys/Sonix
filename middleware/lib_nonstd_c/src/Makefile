include Makefile.in

PWD             := $(shell pwd)
INSTALL_DIR     ?= $(PWD)/target
INSTALL_INC_DIR := $(INSTALL_DIR)/include/libmid_nonstdc
INSTALL_LIB_DIR := $(INSTALL_DIR)/lib
OBJ_DIR         ?=$(PWD)/obj

ARCH            ?= arm
TARGET          ?= $(ARCH)-linux
CROSS_COMPILE   ?= $(TARGET)-
CC              = $(CROSS_COMPILE)gcc
AR              = $(CROSS_COMPILE)ar

STATIC_LIB = $(OBJ_DIR)/libmid_nonstdc.a
OBJS_FILES = $(STDIO_OBJ_FILES) $(STDLIB_OBJ_FILES) $(MATH_OBJ_FILES) $(STRING_OBJ_FILES) $(TIME_OBJ_FILES) $(NETWORK_OBJ_FILES) $(CTYPE_OBJ_FILES) $(UNISTD_OBJ_FILES)
OBJS_FILES += version.o
OBJS = $(addprefix $(OBJ_DIR)/, $(OBJS_FILES))
SUBDIRS = stdio string math time stdlib network ctype unistd

all: $(OBJ_DIR) $(STATIC_LIB)

$(OBJ_DIR):
	[ -d "$@" ] || mkdir -p $@

$(STATIC_LIB): subdirs $(OBJ_DIR)/version.o
	$(AR) -r $@ $(OBJS)

subdirs:
	@for dir in $(SUBDIRS) ; do $(MAKE) -C $$dir || exit 1 ; done

$(OBJ_DIR)/version.o:$(PWD)/version.c
	$(CC) -c -I$(SDK_CONF_DIR) $^ -o $@

clean:
	@for dir in $(SUBDIRS) ; do $(MAKE) -C $$dir clean || exit 1 ; done
	rm -rf $(OBJ_DIR) $(STATIC_LIB)

distclean:

install:
	[ -d "$(INSTALL_INC_DIR)" ] || mkdir -p $(INSTALL_INC_DIR)
	[ -d "$(INSTALL_LIB_DIR)" ] || mkdir -p $(INSTALL_LIB_DIR)
	cp $(STATIC_LIB) $(INSTALL_LIB_DIR)
	cp -af include/* $(INSTALL_INC_DIR)
