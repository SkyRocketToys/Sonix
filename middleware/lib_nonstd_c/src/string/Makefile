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

CFLAGS		= -nostdinc $(PRJ_CFLAGS) -std=c99
CFLAGS		+= -I$(PWD)
CFLAGS		+= -I$(KERNEL_INC_DIR)
CFLAGS		+= -I$(DRIVER_INC_DIR)
CFLAGS		+= -I../include

OBJS_FILES = $(STRING_OBJ_FILES)
OBJS = $(addprefix $(OBJ_DIR)/, $(OBJS_FILES))

.PHONY:all clean distclean install    

#all: $(OBJS_FILE)
all: $(OBJS)

$(OBJ_DIR)/memcmp.o:$(PWD)/memcmp.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/memcpy.o:$(PWD)/memcpy.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/memset.o:$(PWD)/memset.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/strcmp.o:$(PWD)/strcmp.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/strcpy.o:$(PWD)/strcpy.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/strlen.o:$(PWD)/strlen.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/strncmp.o:$(PWD)/strncmp.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/strncpy.o:$(PWD)/strncpy.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/strchr.o:$(PWD)/strchr.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/strchrnul.o:$(PWD)/strchrnul.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/strnlen.o:$(PWD)/strnlen.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/memchr.o:$(PWD)/memchr.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/stpncpy.o:$(PWD)/stpncpy.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/strdup.o:$(PWD)/strdup.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/strcasecmp.o:$(PWD)/strcasecmp.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/strtok.o:$(PWD)/strtok.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/strstr.o:$(PWD)/strstr.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/strcat.o:$(PWD)/strcat.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/index.o:$(PWD)/index.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/strcspn.o:$(PWD)/strcspn.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/memmove.o:$(PWD)/memmove.c
	$(CC) -c $(CFLAGS) $< -o $@
#$(OBJS):
#	$(CC) $(CFLAGS) -c $*.c -o $@
clean:
	rm -rf $(OBJS)

