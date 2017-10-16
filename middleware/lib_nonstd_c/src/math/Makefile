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

CFLAGS		= $(PRJ_CFLAGS) -nostdinc -std=c99
CFLAGS		+= -I$(PWD)
CFLAGS		+= -I$(KERNEL_INC_DIR)
CFLAGS		+= -I$(DRIVER_INC_DIR)
CFLAGS		+= -I../include


OBJS_FILES = $(MATH_OBJ_FILES)
OBJS = $(addprefix $(OBJ_DIR)/, $(OBJS_FILES))

.PHONY:all clean distclean install    
all: $(OBJS)


$(OBJ_DIR)/acos.o:$(PWD)/acos.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/atan.o:$(PWD)/atan.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/atan2.o:$(PWD)/atan2.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/ceil.o:$(PWD)/ceil.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/cos.o:$(PWD)/cos.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/exp.o:$(PWD)/exp.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/floor.o:$(PWD)/floor.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/log.o:$(PWD)/log.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/sin.o:$(PWD)/sin.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/tan.o:$(PWD)/tan.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/sqrt.o:$(PWD)/sqrt.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/__sin.o:$(PWD)/__sin.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/__tan.o:$(PWD)/__tan.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/__cos.o:$(PWD)/__cos.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/__rem_pio2.o:$(PWD)/__rem_pio2.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/__rem_pio2_large.o:$(PWD)/__rem_pio2_large.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/scalbn.o:$(PWD)/scalbn.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/log2.o:$(PWD)/log2.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/pow.o:$(PWD)/pow.c
	$(CC) -c $(CFLAGS) $< -o $@
clean:
	rm -rf $(OBJ_DIR) $(OBJS)

