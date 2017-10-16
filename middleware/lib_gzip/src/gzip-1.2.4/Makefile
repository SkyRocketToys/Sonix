PWD		:= $(shell pwd)
INSTALL_DIR	?= $(PWD)/target
INSTALL_INC_DIR	:= $(INSTALL_DIR)/include/
INSTALL_LIB_DIR	:= $(INSTALL_DIR)/lib
OBJ_DIR		?= $(PWD)/obj

-include $(SDK_CONF_DIR)/config/snx_sdk.conf

ARCH		?= arm
TARGET		?= $(ARCH)-linux
CROSS_COMPILE	?= $(TARGET)-
CC		= $(CROSS_COMPILE)gcc
AR		= $(CROSS_COMPILE)ar
CFLAGS		+= -nostdinc
CFLAGS		+= -DFREERTOS
CFLAGS		+= -DNO_STDLIB_H -DNO_STRING_H #-DUNALIGNED_OK
ifeq ($(CONFIG_MIDDLEWARE_GZIP_DYN_ALLOC), y)
CFLAGS		+= -DDYN_ALLOC 
endif
ifeq ($(CONFIG_MIDDLEWARE_GZIP_DEBUG_VERBOSE), y)
CFLAGS		+= -D_DEBUG_VERBOSE=1
CFLAGS		+= $(PRJ_CFLAGS)
endif
CFLAGS          += -I$(KERNEL_INC_DIR) -I$(MIDDLEWARE_LIBC_INC_DIR) -I$(SDK_CONF_DIR)

STATIC_LIB	= $(OBJ_DIR)/libgzip.a
OBJS_FILES	= inflate.o deflate.o util.o trees.o bits.o snx_gzip.o
#OBJS_FILES	+= zip.o unzip.o
OBJS_FILES	+= version.o
OBJS		= $(addprefix $(OBJ_DIR)/, $(OBJS_FILES))

.PHONY: all clean distclean install

all: $(OBJ_DIR) $(STATIC_LIB)

$(OBJ_DIR):
	[ -d "$@" ] || mkdir -p $@

$(STATIC_LIB): $(OBJS)
	$(AR) -r $@ $(OBJS)

$(OBJ_DIR)/zip.o: $(PWD)/zip.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/deflate.o: $(PWD)/deflate.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/trees.o: $(PWD)/trees.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/bits.o: $(PWD)/bits.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/unzip.o: $(PWD)/unzip.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/inflate.o: $(PWD)/inflate.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/util.o: $(PWD)/util.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/snx_gzip.o: $(PWD)/snx_gzip.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/version.o:$(PWD)/version.c
	$(CC) -c -O0 -I$(SDK_CONF_DIR) $< -o $@
	
distclean: clean

clean:
	rm -rf $(OBJ_DIR) $(STATIC_LIB)

install:
	[ -d "$(INSTALL_INC_DIR)" ] || mkdir -p $(INSTALL_INC_DIR)
	[ -d "$(INSTALL_LIB_DIR)" ] || mkdir -p $(INSTALL_LIB_DIR)
	cp $(STATIC_LIB) $(INSTALL_LIB_DIR)
	cp -af gzip.h $(INSTALL_INC_DIR)

