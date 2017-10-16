#
# Make Related Variables 
#
PERL		:= perl
PARTITIONPARSE	:= parse_partition.pl
SDK_CONF_DIR	?= $(srctree)/include
#
#	include
#
# Read in config
-include $(SDK_CONF_DIR)/config/snx_sdk.conf
FLASH           ?= $(if $(CONFIG_SYSTEM_NAND_FLASH_SUPPORT),nand,$(if $(CONFIG_SYSTEM_SERIAL_FLASH_SUPPORT),sf,nand))
MACHINE         ?= $(CONFIG_SYSTEM_PLATFORM)

SERIAL_FLASH_LAYOUT_CONF	?= serial_flashlayout.conf

ifeq ($(CONFIG_NVRAM_PART), y)
NVRAM_USER_SIZE = $(CONFIG_NVRAM_PART_SIZE)
else
NVRAM_USER_SIZE = 0
endif

ifeq ($(CONFIG_JFFS2_PART), y)
JFFS2_PART_SIZE = $(CONFIG_JFFS2_PART_SIZE)
JFFS2_PART_MOUNT = $(CONFIG_JFFS2_PART_MOUNT)
else
JFFS2_PART_SIZE = 0
JFFS2_PART_MOUNT = "null"
endif

LOGO_PART_SIZE = 0

ifeq ($(CONFIG_SYSTEM_SERIAL_FLASH_16M),y)
FLASH_SIZE=16
endif
ifeq ($(CONFIG_SYSTEM_SERIAL_FLASH_8M),y)
FLASH_SIZE=8
endif
ifeq ($(CONFIG_SYSTEM_SERIAL_FLASH_4M),y)
FLASH_SIZE=4
endif
ifeq ($(CONFIG_SYSTEM_SERIAL_FLASH_2M),y)
FLASH_SIZE=2
endif
ifeq ($(CONFIG_SYSTEM_SERIAL_FLASH_1M),y)
FLASH_SIZE=1
endif


ifeq ($(CONFIG_SYSTEM_PLATFORM_ST58660FPGA), y)
PLATFORM_NAME="sn9866x"
else ifeq ($(CONFIG_SYSTEM_PLATFORM_SN98660), y)
PLATFORM_NAME="sn9866x"
else ifeq ($(CONFIG_SYSTEM_PLATFORM_SN98670), y)
PLATFORM_NAME="sn9866x"
else ifeq ($(CONFIG_SYSTEM_PLATFORM_SN98671), y)
PLATFORM_NAME="sn9866x"
else ifeq ($(CONFIG_SYSTEM_PLATFORM_SN98672), y)
PLATFORM_NAME="sn9866x"
else ifeq ($(CONFIG_SYSTEM_PLATFORM_SN98661), y)
PLATFORM_NAME="sn9866x"
else ifeq ($(CONFIG_SYSTEM_PLATFORM_SN98293), y)
PLATFORM_NAME="sn9866x"
else
PLATFORM_NAME="sn986xx"
endif
# endif
# endif
# endif

PARAMS		:= rescue_system=$(CONFIG_RESCUE_SYSTEM)
PARAMS		+= platform_name=$(PLATFORM_NAME)
PARAMS		+= bootsel_size=$(CONFIG_BOOTSEL_SIZE)
PARAMS		+= nvram_size=$(NVRAM_USER_SIZE)
PARAMS		+= factory_size=$(CONFIG_FACTORY_SIZE)
PARAMS		+= data_section_size=$(CONFIG_DATA_SECTION_SIZE)
ifneq ($(CONFIG_STORAGE_SECTION_SIZE), 0)
PARAMS		+= storage_section_size=$(CONFIG_STORAGE_SECTION_SIZE)
endif
PARAMS		+= rtos_size=$(CONFIG_RTOS_SIZE)
PARAMS		+= rescue_size=$(CONFIG_RESCUE_PART_SIZE)
PARAMS		+= flash_total_size=$(FLASH_SIZE)
PARAMS		+= image_dir=$(PRJ_IMAGE_DIR)
PARAMS		+= imagetool_dir=$(IMGTOOL_DIR)
PARAMS		+= config_file=$(SERIAL_FLASH_LAYOUT_CONF)
PARAMS		+= flashloyout_conf=$(FLASHLAYOUT_CONF)

#$(warning ### PARAMS = "$(PARAMS)")

#
# target
#
.PHONY: $(SERIAL_FLASH_LAYOUT_CONF)
#$(SERIAL_FLASH_LAYOUT_CONF):
#	$(PERL) $(PARTITIONPARSE) \
#	rescue_system=$(CONFIG_RESCUE_SYSTEM)	\
#	platform_name=$(PLATFORM_NAME)	\
#	bootsel_size=$(CONFIG_BOOTSEL_SIZE)	\
#	nvram_size=$(NVRAM_USER_SIZE) \
#	factory_size=$(CONFIG_FACTORY_SIZE) \
#	data_section_size=$(CONFIG_DATA_SECTION_SIZE) \
#	storage_section_size=$(CONFIG_STORAGE_SECTION_SIZE) \
#	rtos_size=$(CONFIG_RTOS_SIZE)	\
#	rescue_size=$(CONFIG_RESCUE_PART_SIZE)	\
#	flash_total_size=$(FLASH_SIZE)	\
#	image_dir=$(PRJ_IMAGE_DIR)	\
#	imagetool_dir=$(IMGTOOL_DIR)	\
#	config_file=$(SERIAL_FLASH_LAYOUT_CONF)	\
#	flashloyout_conf=$(FLASHLAYOUT_CONF)
$(SERIAL_FLASH_LAYOUT_CONF):
	$(PERL) $(PARTITIONPARSE) $(PARAMS)
	
	
PHONY += clean
clean:
	rm -rf $(SERIAL_FLASH_LAYOUT_CONF)
	rm -rf $(FLASHLAYOUT_CONF)
