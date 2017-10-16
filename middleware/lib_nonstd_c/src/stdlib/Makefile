include ../Makefile.in

PWD		:= $(shell pwd)
INSTALL_DIR	?= $(PWD)/target
INSTALL_INC_DIR	:= $(INSTALL_DIR)/include/libmid_nonstdc
INSTALL_LIB_DIR := $(INSTALL_DIR)/lib
OBJ_DIR		?= $(PWD)/obj

ARCH            ?= arm
TARGET          ?= $(ARCH)-linux
CROSS_COMPILE   ?= $(TARGET)-
CC		= $(CROSS_COMPILE)gcc
AR		= $(CROSS_COMPILE)ar

CFLAGS		= $(PRJ_CFLAGS) -std=c99
CFLAGS		+= -I$(PWD)
CFLAGS		+= -I$(KERNEL_INC_DIR)
CFLAGS		+= -I$(DRIVER_INC_DIR)
CFLAGS		+= -I../include -nostdinc

OBJS_FILES = $(STDLIB_OBJ_FILES)
OBJS = $(addprefix $(OBJ_DIR)/, $(OBJS_FILES))

.PHONY:all clean distclean install    

#all: $(OBJS_FILE)
all: $(OBJS)

$(OBJ_DIR)/strtol.o:$(PWD)/strtol.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/atoi.o:$(PWD)/atoi.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/atol.o:$(PWD)/atol.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/atof.o:$(PWD)/atof.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/malloc.o:$(PWD)/malloc.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/abs.o:$(PWD)/abs.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/rand.o:$(PWD)/rand.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/strtod.o:$(PWD)/strtod.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/qsort.o:$(PWD)/qsort.c
	$(CC) -c $(CFLAGS) $< -o $@
#$(OBJS):
#	$(CC) $(CFLAGS) -c $*.c -o $@
clean:
	rm -rf $(OBJS)

