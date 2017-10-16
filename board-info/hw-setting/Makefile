# Read in config

SDK_CONF_DIR	?= $(srctree)/include
-include $(SDK_CONF_DIR)/config/snx_sdk.conf

VER_HW ?= $(if $(CONFIG_SYSTEM_PREVIOUS_PLATFORM),ver)
ifeq ($(VER_HW),ver)
	VER_WHCH = $(CONFIG_SYSTEM_PREVIOUS_VERSION)
else
	VER_WHCH = none
endif

WHICH_VALUE =
PLATFORM =

ifeq ($(CONFIG_SYSTEM_PLATFORM_ST58660FPGA), y)
	MUTI_HWSETTING=yes
	HW_PATH="./sn9866x"
else ifeq ($(CONFIG_SYSTEM_PLATFORM_SN98660), y)
	MUTI_HWSETTING=yes
	HW_PATH="./sn9866x"
else ifeq ($(CONFIG_SYSTEM_PLATFORM_SN98670), y)
	MUTI_HWSETTING=yes
	HW_PATH="./sn9866x"
else ifeq ($(CONFIG_SYSTEM_PLATFORM_SN98671), y)
	MUTI_HWSETTING=yes
	HW_PATH="./sn9866x"
else ifeq ($(CONFIG_SYSTEM_PLATFORM_SN98672), y)
	MUTI_HWSETTING=yes
	HW_PATH="./sn9866x"
else ifeq ($(CONFIG_SYSTEM_PLATFORM_SN98661), y)
	MUTI_HWSETTING=yes
	HW_PATH="./sn9866x"
else ifeq ($(CONFIG_SYSTEM_PLATFORM_SN98293), y)
	MUTI_HWSETTING=yes
	HW_PATH="./sn9866x"
endif
# endif
# endif
# endif

.PHONY: sdk keil jlink sdk-f quake-h
sdk:
	perl hw_setting_convert.pl $(MUTI_HWSETTING) $(VER_WHCH) $(HW_PATH) sdk

keil:
	perl hw_setting_convert.pl $(MUTI_HWSETTING) $(VER_WHCH) $(HW_PATH) keil
	
jlink:
	perl hw_setting_convert.pl $(MUTI_HWSETTING) $(VER_WHCH) $(HW_PATH) jlink
	
sdk-f:
	perl hw_setting_convert.pl $(MUTI_HWSETTING) $(VER_WHCH) $(HW_PATH) sdk -f $(WHICH_VALUE) $(PLATFORM) $(END_FILE)

quake-h:
	perl hw_setting_convert.pl $(MUTI_HWSETTING) $(VER_WHCH) $(HW_PATH) quake -h $(WHICH_VALUE)
	
PHONY += hwsetting-clean
hwsetting-clean:
	rm -rf ./sdk ./quake ./keil ./jlink ./sdk_$(VER_WHCH) ./quake_$(VER_WHCH) ./keil_$(VER_WHCH) ./jlink_$(VER_WHCH)
