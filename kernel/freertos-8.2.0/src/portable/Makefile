PWD             := $(shell pwd)
INSTALL_DIR     ?= $(PWD)/../../target
OBJ_DIR         ?= $(PWD)/../../obj
GCC_CHIP_DIR    := $(PWD)/gcc/sn986xx
MEM_DIR         := $(PWD)/mem
INTC            := $(GCC_CHIP_DIR)/intc
TIMER           := $(GCC_CHIP_DIR)/timer
UART            := $(GCC_CHIP_DIR)/uart

ARCH            ?= arm
TARGET          ?= $(ARCH)-linux
CROSS_COMPILE   ?= $(TARGET)-
CC              = $(CROSS_COMPILE)gcc
AS              = $(CROSS_COMPILE)as
LD              = $(CROSS_COMPILE)ld

CFLAGS          = $(PRJ_CFLAGS) -I$(KERNEL_INC_DIR)
CFLAGS          += -I$(SYSTEM_TRACE_INC_DIR) -I$(PWD)/../../../../middleware/lib_nonstd_c/src/include
CFLAGS          += -I$(PWD)/../include
CFLAGS          += -I$(GCC_CHIP_DIR)
CFLAGS          += -I$(INTC)
CFLAGS          += -I$(TIMER)
CFLAGS          += -I$(UART)
CFLAGS		+= -I$(SDK_CONF_DIR)

ASFLAGS         = $(PRJ_ASFLAGS)

OBJ_FILES = startup.o port.o portISR.o mmu.o heap_4_cb.o heap_4_ncnb.o mem.o interrupt.o timer.o clock.o

OBJ_FILES += heap_4_pre_ncnb.o heap_4_pre_cb.o

OBJS = $(addprefix $(OBJ_DIR)/, $(OBJ_FILES))

PORTABLS_OBJ_FILE = portable.o
PORTABLE_OBJ = $(addprefix $(OBJ_DIR)/, $(PORTABLS_OBJ_FILE))

$(PORTABLE_OBJ):$(OBJS)
	$(LD) -r $^ -o $@
clean:
	rm -rf $(OBJS) $(PORTABLE_OBJ)

$(OBJ_DIR)/startup.o : $(GCC_CHIP_DIR)/startup.S
	$(AS) -c $(ASFLAGS) $< -o $@
$(OBJ_DIR)/port.o : $(GCC_CHIP_DIR)/port.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/portISR.o : $(GCC_CHIP_DIR)/portISR.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/mmu.o : $(GCC_CHIP_DIR)/mmu.S
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/clock.o : $(GCC_CHIP_DIR)/clock.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/heap_4_cb.o : $(MEM_DIR)/heap_4_cb.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/heap_4_ncnb.o : $(MEM_DIR)/heap_4_ncnb.c
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJ_DIR)/heap_4_pre_cb.o : $(MEM_DIR)/heap_4_pre_cb.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/heap_4_pre_ncnb.o : $(MEM_DIR)/heap_4_pre_ncnb.c
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJ_DIR)/mem.o : $(MEM_DIR)/mem.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/interrupt.o : $(INTC)/interrupt.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/timer.o : $(TIMER)/timer.c
	$(CC) -c $(CFLAGS) $< -o $@
