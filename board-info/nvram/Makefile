#
# Make Related Variables 
#
MAKE            ?= make
PWD             := $(shell pwd)
PERL		:= perl
NVRAMPARSE	:= parse_nvram.pl
SDK_CONF_DIR	?= $(srctree)/include
ORIGINAL_DASHCAM_FILE	:= rtos_dashcam_nvram.txt
ORIGINAL_DRONE_FILE	:= rtos_drone_nvram.txt
ORIGINAL_STREAMING_FILE	:= rtos_streaming_nvram.txt
ORIGINAL_IPC_FILE := rtos_ipc_nvram.txt
ORIGINALFILE	:= rtos_nvram.txt

#
#	include
#
# Read in config
-include $(SDK_CONF_DIR)/config/snx_sdk.conf
FLASH           ?= $(if $(CONFIG_SYSTEM_NAND_FLASH_SUPPORT),nand,$(if $(CONFIG_SYSTEM_SERIAL_FLASH_SUPPORT),sf,nand))
MACHINE         ?= $(CONFIG_SYSTEM_PLATFORM)
ifeq ($(CONFIG_MIDDLEWARE_GZIP), y)
DEFLATE_LV	:= $(CONFIG_MIDDLEWARE_GZIP_LEVEL)
else
DEFLATE_LV	:= 6
endif

include $(srctree)/Makefile.version

#
# target
#
.PHONY: nvram
nvram:
#ifeq ($(CONFIG_APP_DRONE), y)
#	cp $(ORIGINAL_DRONE_FILE) $(ORIGINALFILE)
#else
#	cp $(ORIGINAL_DASHCAM_FILE) $(ORIGINALFILE)
#endif
ifeq ($(CONFIG_APP_DASHCAM), y)
ifeq ($(CONFIG_APP_DRONE), y)
ifeq ($(CONFIG_APP_STREAMING), y)
	cp $(ORIGINAL_STREAMING_FILE) $(ORIGINALFILE)
else
	cp $(ORIGINAL_DRONE_FILE) $(ORIGINALFILE)
endif
else
	cp $(ORIGINAL_DASHCAM_FILE) $(ORIGINALFILE)
endif
else
ifeq ($(CONFIG_APP_IPCAM), y)
	cp $(ORIGINAL_IPC_FILE) $(ORIGINALFILE)
endif
endif

	$(MAKE) -C iq
	if [ -e iq/IQ.bin ]; then \
		cp iq/IQ.bin .; \
	else \
		touch IQ.bin; \
	fi
	./update_nvram.sh rtos_nvram.txt $(CONFIG_VALUE_USB_DEVICE_MODE) $(CONFIG_VALUE_USB_DEVICE_HPD_SRC) $(CONFIG_MODULE_USB_DEVICE_ASIC_IO_NUM) $(CONFIG_MODULE_USB_DEVICE_PLUGIN_TRIG_LEV)
	$(PERL) $(NVRAMPARSE)	\
		orifile=$(ORIGINALFILE) \
		fileziped=$(CONFIG_NVRAM_FILES_ZIP) \
		fileziped_evl=$(DEFLATE_LV) \
		sdk_version="$(sdk_version)" \
		nvram_bin=$(DONE_NVRAM) \
		factory_bin=$(DONE_FACTORY) \
		data_bin=$(DONE_DATA)

PHONY += clean
clean:
	$(MAKE) -C iq clean
	rm -rf DONE_NVRAM DONE_FACTORY DONE_DATA
	rm -f *.d IQ.bin
