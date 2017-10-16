PWD             := $(shell pwd)
INSTALL_DIR     ?= $(PWD)/../target
OBJ_DIR         ?= $(PWD)/../obj
PORTABLE_DIR    := $(PWD)/portable
PORTABLE_HEAD_FILE_DIR  := $(PWD)/portable/gcc/sn986xx

MAKE            ?= make
ARCH            ?= arm
TARGET          ?= $(ARCH)-linux
CROSS_COMPILE   ?= $(TARGET)-
CC              = $(CROSS_COMPILE)gcc
AR              = $(CROSS_COMPILE)ar
LD 				= $(CROSS_COMPILE)ld
OBJCOPY 		= $(CROSS_COMPILE)objcopy
OBJDUMP 		= $(CROSS_COMPILE)objdump
STRIP			= $(CROSS_COMPILE)strip

-include $(SDK_CONF_DIR)/config/snx_sdk.conf
CFLAGS          = $(PRJ_CFLAGS)
CFLAGS          += -I$(PWD)/include -I$(PWD)/../../../middleware/lib_nonstd_c/src/include
ifeq ($(CONFIG_SYSTEM_TRACE_SELECT), y)
CFLAGS          += -I$(SYSTEM_TRACE_INC_DIR)
CFLAGS          += -I$(PWD)/portable/gcc/sn986xx/intc
endif
CFLAGS          += -I$(PORTABLE_HEAD_FILE_DIR)
CFLAGS          += -I$(PWD)/bootsel/

ASFLAGS         = $(PRJ_ASFLAGS)
DEFS =  -DSTDC_HEADERS=1 -DHAVE_UNISTD_H=1 -DDIRENT=1 -DNOMEMCPY=1 -DNO_DIR=1 -DNO_UTIME=1

STATIC_LIB = $(OBJ_DIR)/libkernel.a

OBJ_FILES = queue.o list.o tasks.o timers.o event_groups.o
OBJ_FILES += version.o
OBJS = $(addprefix $(OBJ_DIR)/, $(OBJ_FILES))


BTSEL_LDFLAGS		:= -nostdlib -nodefaultlibs -nostartfiles -L$(PWD)/../../../tool/crosstool-4.5.2/lib/gcc/arm-unknown-linux-uclibcgnueabi/4.5.2 -lgcc
BTSEL_LDFLAGS		+= -T $(PWD)/bootsel/boot.ld
BTSEL_LDFLAGS		+= -lgcc


BTSEL_OBJS = $(PWD)/bootsel/boot.o
INFLATE_OBJS = $(PWD)/bootsel/inflate.o
UTILE_OBJS = $(PWD)/bootsel/util.o
BTSEL_IMAGE = $(PWD)/bootsel/btsel_image.elf
BTSEL_IMAGE_BIN = $(PWD)/bootsel/bootsel.bin
BTSEL_DIS = $(PWD)/bootsel/btsel_image.dis

PORTABLS_OBJ_FILE = portable.o
PORTABLE_OBJ = $(addprefix $(OBJ_DIR)/, $(PORTABLS_OBJ_FILE))

.PHONY: all clean distclean install bootselimg
all:$(OBJ_DIR) FreeRTOSConfig priority $(STATIC_LIB) bootselimg
$(OBJ_DIR):
	[ -d "$@" ] || mkdir -p $@
FreeRTOSConfig:
ifeq ($(CONFIG_SYSTEM_TRACE_SELECT), y)
	$(Q)sed -i -e 's/#define configUSE_TRACE_FACILITY.*$$/#define configUSE_TRACE_FACILITY          1/g' $(PWD)/include/FreeRTOSConfig.h;
	$(Q)sed -i -e 's/\/\/#include \"trcKernelPort.h\".*$$/#include \"trcKernelPort.h\"/g' $(PWD)/include/FreeRTOSConfig.h;
else
	$(Q)sed -i -e 's/#define configUSE_TRACE_FACILITY.*$$/#define configUSE_TRACE_FACILITY          0/g' $(PWD)/include/FreeRTOSConfig.h;
	$(Q)sed -i -e 's/^#include \"trcKernelPort.h\".*$$/\/\/#include \"trcKernelPort.h\"/g' $(PWD)/include/FreeRTOSConfig.h;
endif
	##### tick rate hz #######
	$(Q)sed -i -e 's/#define configTICK_RATE_HZ.*$$/#define configTICK_RATE_HZ                ( ( TickType_t ) $(CONFIG_FREERTOS_TICK_HZ) )/g' $(PWD)/include/FreeRTOSConfig.h;

priority:
ifeq ($(CONFIG_APP_IPCAM), y)
	$(Q)cp -a $(PWD)/include/priority/priority_ipcam.h $(PWD)/include/priority.h;
endif
ifeq ($(CONFIG_APP_DASHCAM), y)
	$(Q)cp -a $(PWD)/include/priority/priority_dashcam.h $(PWD)/include/priority.h;
endif
ifeq ($(CONFIG_APP_INTEGRATION), y)
	$(Q)cp -a $(PWD)/include/priority/priority_integration.h $(PWD)/include/priority.h;
endif
ifeq ($(CONFIG_APP_EMPTY), y)
	$(Q)cp -a $(PWD)/include/priority/priority_empty.h $(PWD)/include/priority.h;
endif
ifeq ($(CONFIG_APP_MPTOOL), y)
	$(Q)cp -a $(PWD)/include/priority/priority_mptool.h $(PWD)/include/priority.h;
endif

ifeq ($(CONFIG_APP_QRCODE), y)
	$(Q)cp -a $(PWD)/include/priority/priority_qrcode.h $(PWD)/include/priority.h;
endif


$(STATIC_LIB):$(OBJS) $(PORTABLE_OBJ)
	$(AR) -r $@ $^

CFLAGS += -I$(PWD)/../../platform/src/sn9866x

$(OBJ_DIR)/queue.o:$(PWD)/queue.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/list.o:$(PWD)/list.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/tasks.o:$(PWD)/tasks.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/timers.o:$(PWD)/timers.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/event_groups.o:$(PWD)/event_groups.c
	$(CC) -c $(CFLAGS) $< -o $@
$(OBJ_DIR)/version.o:$(PWD)/version.c
	$(CC) -c -I$(SDK_CONF_DIR) $^ -o $@
$(OBJ_DIR)/status.o:$(PWD)/status.c
	$(CC) -c $(CFLAGS) $< -o $@

bootselimg: bootsel dgzip utile
	$(LD) $(BTSEL_OBJS) $(INFLATE_OBJS) $(UTILE_OBJS) $(BTSEL_LDFLAGS) -o $(BTSEL_IMAGE)
#	$(STRIP) -s $(BTSEL_IMAGE)
	$(OBJCOPY) -O binary $(BTSEL_IMAGE) $(BTSEL_IMAGE_BIN)
#	$(OBJDUMP) -H
#	$(OBJDUMP) -f $(BTSEL_IMAGE) > $(BTSEL_DIS)
	$(OBJDUMP) -DS $(BTSEL_IMAGE) > $(BTSEL_DIS)
	cp -av $(BTSEL_IMAGE_BIN) $(PRJ_IMAGE_DIR)/

bootsel:$(BTSEL_OBJS)
	$(CC) -c $(CFLAGS) $< -o $@

dgzip:$(INFLATE_OBJS)
	$(CC) -c $(DEFS) $(CFLAGS) $< -o $@

utile:$(UTILE_OBJS)
	$(CC) -c $(DEFS) $(CFLAGS) $< -o $@


$(PORTABLE_OBJ)::
	$(MAKE) -C $(PORTABLE_DIR) $@ OBJ_DIR=$(abspath $(OBJ_DIR))

clean:
	$(MAKE) -C $(PORTABLE_DIR) $@ OBJ_DIR=$(abspath $(OBJ_DIR))
	rm -rf $(OBJS) $(STATIC_LIB) $(OBJ_DIR)
	rm -f $(BTSEL_OBJS) $(INFLATE_OBJS) $(UTILE_OBJS)
	rm -f $(BTSEL_IMAGE) $(BTSEL_IMAGE_BIN) $(BTSEL_DIS)

distclean:

install:
	[ -d "$(INSTALL_DIR)/lib" ] || mkdir -p $(INSTALL_DIR)/lib
	[ -d "$(INSTALL_DIR)/include" ] || mkdir -p $(INSTALL_DIR)/include
	cp -rvf $(STATIC_LIB) $(INSTALL_DIR)/lib
	cp -rvf $(PWD)/include/* $(INSTALL_DIR)/include
	cp -rvf $(PORTABLE_HEAD_FILE_DIR)/*.h $(INSTALL_DIR)/include
	cp -rvf $(PORTABLE_HEAD_FILE_DIR)/intc/*.h $(INSTALL_DIR)/include
	cp -rvf $(PORTABLE_HEAD_FILE_DIR)/timer/*.h $(INSTALL_DIR)/include
