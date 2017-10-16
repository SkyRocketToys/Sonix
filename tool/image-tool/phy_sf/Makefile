#
# Make Related Variables 
#
MAKE            ?= make
PWD             := $(shell pwd)
SRC_DIR		?= $(PWD)/src
PERL		:= perl
PHYSF	:= phy_sf.pl

#
# Images 
#
RTOS_FILE	:= $(PRJ_IMAGE_DIR)/KERNEL.bin 
DONE_NVRAM	:= $(PRJ_IMAGE_DIR)/nvram.bin
PHY_FLASH_LAYOUT := $(HW_DIR)/flash-layout/serial_flashlayout.conf

#
#	include
#
# Read in config
-include $(SDK_CONF_DIR)/config/snx_sdk.conf
FLASH           ?= $(if $(CONFIG_SYSTEM_NAND_FLASH_SUPPORT),nand,$(if $(CONFIG_SYSTEM_SERIAL_FLASH_SUPPORT),sf,nand))
MACHINE         ?= $(CONFIG_SYSTEM_PLATFORM)

VER_HW ?= $(if $(CONFIG_SYSTEM_PREVIOUS_PLATFORM),ver)
ifeq ($(VER_HW),ver)
	VER_WHCH = $(CONFIG_SYSTEM_PREVIOUS_VERSION)
else
	VER_WHCH = none
endif

.PHONY: phy_sf
phy_sf:
	$(PERL) $(PHYSF) \
	outir=$(PRJ_IMAGE_DIR) \
	src_dir=$(SRC_DIR) \
	phy_flashlayout=$(PHY_FLASH_LAYOUT)

PHONY += clean
clean:
	$(Q)$(MAKE) -C $(SRC_DIR)/ clean
