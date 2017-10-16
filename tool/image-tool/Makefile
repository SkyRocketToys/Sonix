#
# Make Related Variables 
#
MAKE            ?= make
PWD             := $(shell pwd)
SRC_DIR		?= $(PWD)/src
PERL		:= perl
SYSPARSE	:= sysparser.pl

#
# Images 
#
SDK_CONF_DIR	?= $(srctree)/include
KCONFIG_CONFIG	:= .config

#
#	include
#
# Read in config
-include $(SDK_CONF_DIR)/config/snx_sdk.conf
MACHINE         ?= $(CONFIG_SYSTEM_PLATFORM)

SONIX_DDR_PROJECT = $(CONFIG_MAIN_SETTING)_$(CONFIG_MEM_SIZE_STRING)

.PHONY: image_tool update_bin md5_encrypt rtos_bin firmware firmware_factory phy_sf rescue_bin bootsel_bin
rtos_bin: image_tool update_bin
	$(PERL) $(SYSPARSE) \
	image_path=$(PRJ_IMAGE_DIR) \
	config_path=$(HW_DIR)/ \
	image=rots_bin \
	rtos_image_zip=$(CONFIG_RTOS_IMAGE_ZIP)

rescue_bin: image_tool update_bin
	$(PERL) $(SYSPARSE) \
	image_path=$(PRJ_IMAGE_DIR) \
	config_path=$(HW_DIR)/ \
	image=rescue_bin \
	rtos_image_zip=$(CONFIG_RTOS_IMAGE_ZIP)

bootsel_bin: image_tool update_bin
	$(PERL) $(SYSPARSE) \
	image_path=$(PRJ_IMAGE_DIR) \
	config_path=$(HW_DIR)/ \
	image=bootsel_bin


image_tool:
	$(MAKE) -C ./src/code/

update_bin:
	$(MAKE) -C ./src/header/ clean CONFIG_START=0x00800000
	$(MAKE) -C ./src/header/ CONFIG_START=0x00800000 FW_F=1
#	$(MAKE) -C ./src/header/ clean CONFIG_START=0x1000000
#	$(MAKE) -C ./src/header/ CONFIG_START=0x1000000 FW_F=1


ifeq ($(CONFIG_DDR_TOTAL_SIZE),64)
firmware_update_bin:
	$(MAKE) -C ./src/header/ clean CONFIG_START=0x3ff0000
	$(MAKE) -C ./src/header/ CONFIG_START=0x3ff0000
else
firmware_update_bin:
	$(MAKE) -C ./src/header/ clean CONFIG_START=0x1ff0000
	$(MAKE) -C ./src/header/ CONFIG_START=0x1ff0000
endif


md5_encrypt:
	$(MAKE) -C ./src/md5_encrypt/

firmware_factory: image_tool update_bin md5_encrypt
	$(PERL) $(SYSPARSE) \
	dotconfig=$(KCONFIG_CONFIG) \
	image=firmware_factory \
	platform=$(MACHINE) \
	ddr_freq=$(CONFIG_DDR_FREQ) \
	ddr_project=$(SONIX_DDR_PROJECT) \
	ddr0_ddr_str=$(CONFIG_FIRST_DDR_STRING)	\
	ddr1_ddr_str=$(CONFIG_SECOND_DDR_STRING)	\
	ddr2_ddr_str=$(CONFIG_THIRD_DDR_STRING)	\
	sdk_version="$(sdk_version)" \
	config_path=$(HW_DIR)/ \
	hw_setting_update=$(CONFIG_HWSETTING_UPDATE)	\
	flashlayout_update=$(CONFIG_FLASH_LAYOUT_UPDATE)	\
	user_update=$(CONFIG_USER_UPDATE)	\
	image_dis=$(PRJ_IMAGE_DIS)	\
	rescue_system=$(CONFIG_RESCUE_SYSTEM)	\
	firmware_f_sp=$(CONFIG_FIRMWARE_F_SP)	\
	bypass_tag=$(CONFIG_BYPASS_TAG)	\
	enable_debug_mode=$(CONFIG_ENABLE_DEBUG_MODE)	\
	rtos_image_zip=$(CONFIG_RTOS_IMAGE_ZIP)	\

firmware: image_tool firmware_update_bin md5_encrypt
	$(PERL) $(SYSPARSE) \
	dotconfig=$(KCONFIG_CONFIG) \
	image=firmware \
	platform=$(MACHINE) \
	ddr_freq=$(CONFIG_DDR_FREQ) \
	ddr_project=$(SONIX_DDR_PROJECT) \
	ddr0_ddr_str=$(CONFIG_FIRST_DDR_STRING)	\
	ddr1_ddr_str=$(CONFIG_SECOND_DDR_STRING)	\
	ddr2_ddr_str=$(CONFIG_THIRD_DDR_STRING)	\
	sdk_version="$(sdk_version)" \
	config_path=$(HW_DIR)/ \
	hw_setting_update=$(CONFIG_HWSETTING_UPDATE)	\
	flashlayout_update=$(CONFIG_FLASH_LAYOUT_UPDATE)	\
	user_update=$(CONFIG_USER_UPDATE)	\
	image_dis=$(PRJ_IMAGE_DIS)	\
	rescue_system=$(CONFIG_RESCUE_SYSTEM)	\
	firmware_f_sp=$(CONFIG_FIRMWARE_F_SP)	\
	bypass_tag=$(CONFIG_BYPASS_TAG)	\
	enable_debug_mode=$(CONFIG_ENABLE_DEBUG_MODE)	\
	rtos_image_zip=$(CONFIG_RTOS_IMAGE_ZIP)	\

phy_sf:
	$(Q)cd ./phy_sf && $(MAKE) clean && $(MAKE) phy_sf && cd $(PRJ_DIR) || exit 1
	$(Q)if ! [ -e $(PRJ_IMAGE_DIR)/PHY-SF.bin ]; then \
		echo "No PHY-SF.bin is generated."; \
		exit 1; \
	fi;
	$(Q)if ! [ -e $(PRJ_IMAGE_DIR)/phy_sf_report.txt ]; then \
		echo "No phy_sf_report.txt is generated."; \
		exit 1; \
	fi;

PHONY += clean
clean:
	rm -f ./*.d ./src/header/*.o ./src/code/*.o ./md5_encrypt.bin
	$(MAKE) -C ./src/code clean && $(MAKE) -C ./src/header clean && $(MAKE) -C ./phy_sf clean && $(MAKE) -C ./src/md5_encrypt clean
