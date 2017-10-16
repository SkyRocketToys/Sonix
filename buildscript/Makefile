################################################################
#
# SONIX SDK Makefile 
#
################################################################

# Do not:
# o  print "Entering directory ...";
MAKEFLAGS += --no-print-directory
SHELL=/bin/bash

# Avoid funny character set dependencies
unexport LC_ALL
LC_COLLATE=C
LC_NUMERIC=C
export LC_COLLATE LC_NUMERIC


# To put more focus on warnings, be less verbose as default
# Use 'make V=1' to see the full commands

ifeq ("$(origin V)", "command line")
  KBUILD_VERBOSE = $(V)
endif
ifndef KBUILD_VERBOSE
  KBUILD_VERBOSE = 0
endif

ifeq ("$(origin F)", "command line")
  ENV_FILE = $(F)
endif
ifndef ENV_FILE
  ENV_FILE := 
endif

# That's our default target when none is given on the command line
PHONY := all
all:

# Cancel implicit rules on top Makefile
$(PWD)/Makefile Makefile: ;


srctree		:= $(PWD)
export srctree


KCONFIG_CONFIG	:= .config

ifneq ($(KCONFIG_CONFIG),$(wildcard $(KCONFIG_CONFIG)))
skip-makefile := y
endif


# SHELL used by kbuild
CONFIG_SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
	  else if [ -x /bin/bash ]; then echo /bin/bash; \
	  else echo sh; fi ; fi)

HOSTCC       = gcc
HOSTCXX      = g++
HOSTCFLAGS   = -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer
HOSTCXXFLAGS = -O2


# If KBUILD_VERBOSE equals 0 then the above command will be hidden.
# If KBUILD_VERBOSE equals 1 then the above command is displayed.

ifeq ($(KBUILD_VERBOSE),1)
  quiet =
  Q =
else
  quiet=quiet_
  Q = @
endif

# If the user is running make -s (silent mode), suppress echoing of
# commands

ifneq ($(findstring s,$(MAKEFLAGS)),)
  quiet=silent_
endif

export quiet Q KBUILD_VERBOSE


# We need some generic definitions (do not try to remake the file).
$(srctree)/scripts/Kbuild.include: ;
include $(srctree)/scripts/Kbuild.include

SCRIPTS_DIR		:= $(srctree)/scripts
SDK_CONF_DIR		:= $(srctree)/include

export CONFIG_SHELL HOSTCC HOSTCFLAGS

# Make variables (CC, etc...)
MAKE			:= make
ARCH			?= arm
TARGET			?= $(ARCH)-linux
CROSS_COMPILE		?= $(TARGET)-
OBJCOPY			:= $(CROSS_COMPILE)objcopy
OBJDUMP 		= $(CROSS_COMPILE)objdump
#CODESIZE RBK
CODESIZE		:= $(CROSS_COMPILE)size

BUILD_PWD		?= $(abspath $(PWD))
PRJ_DIR			?= $(abspath $(PWD)/../)

PRJ_OBJ_DIR             := $(PRJ_DIR)/obj/normal
PRJ_OBJ_DIR_RESCUE             := $(PRJ_DIR)/obj/rescue
#PRJ_IMAGE_DIR           := $(PRJ_DIR)/image
#PRJ_IMAGE_DIS           := $(PRJ_IMAGE_DIR)/image.dis

PRJ_INC_DIR             := $(PRJ_DIR)/include/normal
PRJ_LIB_DIR             := $(PRJ_DIR)/lib/normal

KERNEL_INC_DIR          := $(PRJ_INC_DIR)/kernel
KERNEL_LIB_DIR          := $(PRJ_LIB_DIR)/kernel
DRIVER_INC_DIR          := $(PRJ_INC_DIR)/driver
DRIVER_LIB_DIR          := $(PRJ_LIB_DIR)/driver
MIDDLEWARE_INC_DIR      := $(PRJ_INC_DIR)/middleware
MIDDLEWARE_LIBC_INC_DIR := $(PRJ_INC_DIR)/middleware/libmid_nonstdc
MIDDLEWARE_LIB_DIR      := $(PRJ_LIB_DIR)/middleware
NETWORK_INC_DIR         := $(PRJ_INC_DIR)/network
NETWORK_LIB_DIR         := $(PRJ_LIB_DIR)/network
SYSTEM_INC_DIR          := $(PRJ_INC_DIR)/system
SYSTEM_LIB_DIR          := $(PRJ_LIB_DIR)/system


PRJ_INC_DIR_RESCUE             := $(PRJ_DIR)/include/rescue
PRJ_LIB_DIR_RESCUE             := $(PRJ_DIR)/lib/rescue

KERNEL_INC_DIR_RESCUE          := $(PRJ_INC_DIR_RESCUE)/kernel
KERNEL_LIB_DIR_RESCUE          := $(PRJ_LIB_DIR_RESCUE)/kernel
DRIVER_INC_DIR_RESCUE          := $(PRJ_INC_DIR_RESCUE)/driver
DRIVER_LIB_DIR_RESCUE          := $(PRJ_LIB_DIR_RESCUE)/driver
MIDDLEWARE_INC_DIR_RESCUE      := $(PRJ_INC_DIR_RESCUE)/middleware
MIDDLEWARE_LIBC_INC_DIR_RESCUE := $(PRJ_INC_DIR_RESCUE)/middleware/libmid_nonstdc
MIDDLEWARE_LIB_DIR_RESCUE      := $(PRJ_LIB_DIR_RESCUE)/middleware
NETWORK_INC_DIR_RESCUE         := $(PRJ_INC_DIR_RESCUE)/network
NETWORK_LIB_DIR_RESCUE         := $(PRJ_LIB_DIR_RESCUE)/network
SYSTEM_INC_DIR_RESCUE          := $(PRJ_INC_DIR_RESCUE)/system
SYSTEM_LIB_DIR_RESCUE          := $(PRJ_LIB_DIR_RESCUE)/system




SYSTEM_TRACE_INC_DIR    := $(PRJ_DIR)/system/trace/src/include

CROSSTOOL_DIR		:= $(PRJ_DIR)/tool/crosstool-4.5.2
PATH			:= $(HOME)/bin:$(CROSSTOOL_DIR)/bin:$(PATH)
IMGTOOL_DIR		:= $(PRJ_DIR)/tool/image-tool

DEBUG_FLAG		= -g
PRJ_CFLAGS		:= -mcpu=arm926ej-s -Wall -Wno-parentheses -nostdinc -O2 -fomit-frame-pointer -lgcc $(DEBUG_FLAG) -I$(MIDDLEWARE_LIBC_INC_DIR)
PRJ_LDFLAGS		:= -nostdlib -nodefaultlibs -nostartfiles -L$(CROSSTOOL_DIR)/lib/gcc/arm-unknown-linux-uclibcgnueabi/4.5.2 -lgcc
PRJ_LDFLAGS		+= -T $(PRJ_DIR)/kernel/freertos-8.2.0/src/portable/gcc/sn986xx/sn986xx.ld
PRJ_ASFLAGS		:= -mcpu=arm926ej-s $(DEBUG_FLAG)

-include $(SDK_CONF_DIR)/config/snx_sdk.conf

HW_DIR		?= $(PRJ_DIR)/board-info
GPIO_CONF=$(SDK_CONF_DIR)/generated/snx_gpio_conf.h
#FLASHLAYOUT_CONF=$(SDK_CONF_DIR)/generated/snx_flashlayout_conf.h
#NVRAM_CONF=$(SDK_CONF_DIR)/generated/nvram.h
#DONE_NVRAM	:= $(PRJ_IMAGE_DIR)/nvram.bin
#DONE_FACTORY	:= $(PRJ_IMAGE_DIR)/factory.bin
#DONE_DATA	:= $(PRJ_IMAGE_DIR)/data.bin

export GPIO_CONF

#
# Images 
#
export PRJ_CFLAGS PRJ_LDFLAGS PRJ_ASFLAGS 
export PRJ_DIR KERNEL_INC_DIR DRIVER_INC_DIR MIDDLEWARE_INC_DIR NETWORK_INC_DIR SYSTEM_INC_DIR MIDDLEWARE_LIBC_INC_DIR
export KERNEL_LIB_DIR DRIVER_LIB_DIR MIDDLEWARE_LIB_DIR NETWORK_LIB_DIR SYSTEM_LIB_DIR

export KERNEL_INC_DIR_RESCUE DRIVER_INC_DIR_RESCUE MIDDLEWARE_INC_DIR_RESCUE NETWORK_INC_DIR_RESCUE SYSTEM_INC_DIR_RESCUE MIDDLEWARE_LIBC_INC_DIR_RESCUE
export KERNEL_LIB_DIR_RESCUE DRIVER_LIB_DIR_RESCUE MIDDLEWARE_LIB_DIR_RESCUE NETWORK_LIB_DIR_RESCUE SYSTEM_LIB_DIR_RESCUE


export ARCH CROSS_COMPILE
export CROSSTOOL_DIR
export SCRIPTS_DIR SDK_CONF_DIR
export HW_DIR SYSTEM_TRACE_INC_DIR IMGTOOL_DIR


export PRJ_OBJ_DIR_RESCUE PRJ_OBJ_DIR PRJ_OBJ_DIR_RESCUE 

export sdk_version


# ===========================================================================
# Rules shared between *config targets and build targets

# Basic helpers built in scripts/
PHONY += scripts_basic
scripts_basic:
	$(Q)$(MAKE) $(build)=scripts/basic
	$(Q)rm -f .tmp_quiet_recordmcount

# To avoid any implicit rule to kick in, define an empty command.
scripts/basic/%: scripts_basic ;


# To make sure we do not include .config for any of the *config targets
# catch them early, and hand them over to scripts/kconfig/Makefile
# It is allowed to specify more targets when calling make, including
# mixing *config targets and build targets.
# For example 'make oldconfig all'.
# Detect when mixed targets is specified, and make a second invocation
# of make so .config is not included in this case either (for *config).

no-dot-config-targets := help

#The default target
config-targets := 0
#mixed-targets  := 0
dot-config     := 1
code-size      := 0

ifneq ($(filter $(no-dot-config-targets), $(MAKECMDGOALS)),)
	ifeq ($(filter-out $(no-dot-config-targets), $(MAKECMDGOALS)),)
		dot-config := 0
	endif
endif

ifneq ($(filter config %config,$(MAKECMDGOALS)),)
	config-targets := 1
endif

ifeq ($(config-targets),1)
# ===========================================================================
# *config targets only - make sure prerequisites are updated, and descend
# in scripts/kconfig to make the *config target

export KBUILD_DEFCONFIG KBUILD_KCONFIG

#SETUP_ENV := $(shell ./setup_env.sh $(ENV_FILE))
Makefile.mod: $(ENV_FILE)
	./setup_env.sh $(ENV_FILE)

config: Makefile.mod scripts_basic FORCE
	$(Q)mkdir -p include/config
	$(Q)$(MAKE) $(build)=scripts/kconfig $@
	$(Q)if [ .config -nt include/config/snx_sdk.conf ]; then \
		$(MAKE) $(build)=scripts/kconfig silentoldconfig; \
	fi 

	$(Q)if ! [ -d include/generated ]; then \
		mkdir -p include/generated;	\
	fi;
	$(Q)$(MAKE) -C $(HW_DIR)/gpio/sn986xx/ > /dev/null

	$(Q)cp .mod_version include/generated/snx_mod_version.h
#	if ! [ -d $(PRJ_IMAGE_DIR) ]; then \
		install -d $(PRJ_IMAGE_DIR); \
	fi
#	$(Q)$(MAKE) -C $(HW_DIR)/nvram/ > /dev/null
#	$(Q)$(MAKE) -C $(HW_DIR)/flash-layout serial_flashlayout.conf ADDTO_MOUNT_PARTITION=no && cd $(PRJ_DIR) || exit 1

%config: Makefile.mod scripts_basic FORCE
	$(Q)mkdir -p include/config
	$(Q)$(MAKE) $(build)=scripts/kconfig $@
	$(Q)if [ .config -nt include/config/snx_sdk.conf ]; then \
		$(MAKE) $(build)=scripts/kconfig silentoldconfig; \
	fi

	$(Q)if ! [ -d include/generated ]; then \
		mkdir -p include/generated;	\
	fi;
	$(Q)$(MAKE) -C $(HW_DIR)/gpio/sn986xx/ > /dev/null

	$(Q)cp .mod_version include/generated/snx_mod_version.h
#	if ! [ -d $(PRJ_IMAGE_DIR) ]; then \
		install -d $(PRJ_IMAGE_DIR); \
	fi
#	$(Q)$(MAKE) -C $(HW_DIR)/nvram/ > /dev/null
#	$(Q)$(MAKE) -C $(HW_DIR)/flash-layout serial_flashlayout.conf ADDTO_MOUNT_PARTITION=no && cd $(PRJ_DIR) || exit 1


else
# ===========================================================================
# Build targets only - this includes vmlinux, arch specific targets, clean
# targets and others. In general all targets except *config targets.

# Additional helpers built in scripts/
# Carefully list dependencies so we do not try to build scripts twice
# in parallel
PHONY += scripts
scripts: scripts_basic include/config/snx_sdk.conf include/config/snx_sdk_tristate.conf
	$(Q)$(MAKE) $(build)=$(@)


ifeq ($(dot-config),1)
# Read in config
-include include/config/snx_sdk.conf

# Read in dependencies to all Kconfig* files, make sure to run
# oldconfig if changes are detected.
-include include/config/auto.conf.cmd

# To avoid any implicit rule to kick in, define an empty command
#$(KCONFIG_CONFIG) include/config/auto.conf.cmd: ;

# If .config is newer than include/config/snx_sdk.conf, someone tinkered
# with it and forgot to run make oldconfig.
# if auto.conf.cmd is missing then we are probably in a cleaned tree so
# we execute the config step to be sure to catch updated Kconfig files
#include/config/%.conf: $(KCONFIG_CONFIG) include/config/auto.conf.cmd
#	$(Q)$(MAKE) -f $(srctree)/Makefile silentoldconfig
#	$(Q)echo "Update autoconf.h"

else
# Dummy target needed, because used as prerequisite
include/config/snx_sdk.conf: ;
endif # $(dot-config)

PRJ_IMAGE_DIR  := $(PRJ_DIR)/image/$(CONFIG_IMAGE_SUBDIR)
PRJ_IMAGE_DIS           := $(PRJ_IMAGE_DIR)/image.dis
FLASHLAYOUT_CONF=$(SDK_CONF_DIR)/generated/snx_flashlayout_conf.h
NVRAM_CONF=$(SDK_CONF_DIR)/generated/nvram.h

DONE_NVRAM	:= $(PRJ_IMAGE_DIR)/nvram.bin
DONE_FACTORY	:= $(PRJ_IMAGE_DIR)/factory.bin
DONE_DATA	:= $(PRJ_IMAGE_DIR)/data.bin

export PRJ_IMAGE_DIR PRJ_IMAGE_DIS DONE_NVRAM DONE_FACTORY DONE_DATA NVRAM_CONF FLASHLAYOUT_CONF

# build Makefile.version
IGNORE := $(shell test -f Makefile.version || cp Makefile.version.old Makefile.version; ./Makever.sh Makefile.version)

include Makefile.version
include Makefile.menu
# We process the rest of the Makefile if this is the final invocation of make
ifeq ($(skip-makefile),)
include Makefile.mod

mod_time=$(date)_$(time)
all: $(MODULES-y)
$(MODULES-y):distribute-pre base_image
base_image: distribute-pre
#	$(Q)$(MAKE) -C $(HW_DIR)/nvram/ > /dev/null
	$(Q)$(MAKE) -C $(HW_DIR)/flash-layout serial_flashlayout.conf ADDTO_MOUNT_PARTITION=no && cd $(PRJ_DIR) || exit 1

distribute-pre:
	if ! [ -d $(PRJ_IMAGE_DIR) ]; then \
		install -d $(PRJ_IMAGE_DIR); \
	fi
	if ! [ -d $(KERNEL_INC_DIR) ]; then \
		install -d $(KERNEL_INC_DIR); \
	fi
	if ! [ -d $(KERNEL_LIB_DIR) ]; then \
		install -d $(KERNEL_LIB_DIR); \
	fi		
	if ! [ -d $(DRIVER_INC_DIR) ]; then \
		install -d $(DRIVER_INC_DIR); \
	fi
	if ! [ -d $(DRIVER_LIB_DIR) ]; then \
		install -d $(DRIVER_LIB_DIR); \
	fi	
	if ! [ -d $(MIDDLEWARE_INC_DIR) ]; then \
		install -d $(MIDDLEWARE_INC_DIR); \
	fi		
	if ! [ -d $(MIDDLEWARE_LIB_DIR) ]; then \
		install -d $(MIDDLEWARE_LIB_DIR); \
	fi		
	if [ -d $(SYSTEM_TRACE_INC_DIR) ]; then \
		ln -sf $(SYSTEM_TRACE_INC_DIR)/* $(KERNEL_INC_DIR); \
	fi
	if ! [ -d $(KERNEL_INC_DIR_RESCUE) ]; then \
		install -d $(KERNEL_INC_DIR_RESCUE); \
	fi
	if ! [ -d $(KERNEL_LIB_DIR_RESCUE) ]; then \
		install -d $(KERNEL_LIB_DIR_RESCUE); \
	fi		
	if ! [ -d $(DRIVER_INC_DIR_RESCUE) ]; then \
		install -d $(DRIVER_INC_DIR_RESCUE); \
	fi
	if ! [ -d $(DRIVER_LIB_DIR_RESCUE) ]; then \
		install -d $(DRIVER_LIB_DIR_RESCUE); \
	fi	
	if ! [ -d $(MIDDLEWARE_INC_DIR_RESCUE) ]; then \
		install -d $(MIDDLEWARE_INC_DIR_RESCUE); \
	fi		
	if ! [ -d $(MIDDLEWARE_LIB_DIR_RESCUE) ]; then \
		install -d $(MIDDLEWARE_LIB_DIR_RESCUE); \
	fi	

	rm -f codesize.txt; 

MODULES=$(MODULES-y) $(MODULES-n) $(MODULES-) 
showmodules:
	$(Q)count=0
	$(Q)for module in $(MODULES); \
	do \
	num=`echo -n $$module | wc -c`; \
	echo -n "$$module"; \
	if [ "$$num" -lt "8" ]; then \
		echo -n -e "\t\t\t"; \
	elif [ "$$num" -lt "16" ]; then \
		echo -n -e "\t\t"; \
	elif [ "$$num" -lt "24" ]; then \
		echo -n -e "\t"; \
	fi; \
	let count++; \
	if [ $$count -ge 3 ]; then \
		echo; \
		count=0; \
	fi; \
	done
	$(Q)echo

showallmodules:
	$(Q)count=0
	$(Q)for module in $(snx_modules); \
	do \
	num=`echo -n $$module | wc -c`; \
	echo -n "$$module"; \
	if [ "$$num" -lt "8" ]; then \
		echo -n -e "\t\t\t"; \
	elif [ "$$num" -lt "16" ]; then \
		echo -n -e "\t\t"; \
	elif [ "$$num" -lt "24" ]; then \
		echo -n -e "\t"; \
	fi; \
	let count++; \
	if [ $$count -ge 3 ]; then \
		echo; \
		count=0; \
	fi; \
	done
	$(Q)echo

PHONY += rtos_image
rtos_image:
	$(OBJCOPY) -O binary $(PRJ_IMAGE_DIR)/image.elf $(PRJ_IMAGE_DIR)/rtos.bin
	$(Q)cd  $(IMGTOOL_DIR) && $(MAKE) rtos_bin && cd $(PRJ_DIR) || exit 1

PHONY += rescue_image
rescue_image:
	$(OBJCOPY) -O binary $(PRJ_IMAGE_DIR)/image_rescue.elf $(PRJ_IMAGE_DIR)/rescue.bin
	$(Q)cd  $(IMGTOOL_DIR) && $(MAKE) rescue_bin && cd $(PRJ_DIR) || exit 1

PHONY += bootsel_image
bootsel_image:
	$(Q)cd  $(IMGTOOL_DIR) && $(MAKE) bootsel_bin && cd $(PRJ_DIR) || exit 1

PHONY += rtos_dis
rtos_dis: rtos_image
	$(OBJDUMP) -f $(PRJ_IMAGE_DIR)/image.elf > $(PRJ_IMAGE_DIS)


PHONY += flash_layout
flash_layout:
	$(Q)$(MAKE) -C $(HW_DIR)/flash-layout serial_flashlayout.conf ADDTO_MOUNT_PARTITION=no && cd $(PRJ_DIR) || exit 1

PHONY += nvram
nvram:
	$(Q)$(MAKE) -C $(HW_DIR)/nvram/ > /dev/null

PHONY += firmware
ifeq ($(shell echo $(MODULE_ONLY) | tr "[:upper:]" "[:lower:]" ), yes)
firmware: firmware_factory rtos_dis
else
firmware: rtos_image flash_layout firmware_factory rtos_dis
endif
	$(Q)cd  $(IMGTOOL_DIR) && $(MAKE) firmware && cd $(PRJ_DIR) || exit 1
	$(Q)if [ -e $(IMGTOOL_DIR)/FIRMWARE_SB.bin ]; then \
                mv $(IMGTOOL_DIR)/FIRMWARE_SB.bin $(PRJ_IMAGE_DIR)/FIRMWARE_660R.bin; \
        else \
                echo "No firmware_update is generated."; \
		exit 1; \
        fi;

PHONY += firmware_factory
ifeq ($(shell echo $(MODULE_ONLY) | tr "[:upper:]" "[:lower:]" ), yes)
firmware_factory:
else
ifeq ($(CONFIG_RESCUE_SYSTEM), y)
firmware_factory: rtos_image flash_layout rescue_image bootsel_image rtos_dis
else
firmware_factory: rtos_image flash_layout bootsel_image rtos_dis
endif
endif
	$(Q)cd  $(IMGTOOL_DIR) && $(MAKE) firmware_factory && cd $(PRJ_DIR) || exit 1
	$(Q)if [ -e $(IMGTOOL_DIR)/FIRMWARE_SB.bin ]; then \
                mv $(IMGTOOL_DIR)/FIRMWARE_SB.bin $(PRJ_IMAGE_DIR)/FIRMWARE_660R_F.bin; \
        else \
                echo "No FIRMWARE_SB.bin is generated."; \
		exit 1; \
        fi;

PHONY += phy_sf 
ifeq ($(shell echo $(MODULE_ONLY) | tr "[:upper:]" "[:lower:]" ), yes)
phy_sf:
else
phy_sf:rtos_image flash_layout
endif
	$(Q)cd  $(IMGTOOL_DIR) && $(MAKE) phy_sf && cd $(PRJ_DIR) || exit 1
	$(Q)if ! [ -e $(PRJ_IMAGE_DIR)/PHY-SF.bin ]; then \
		echo "No PHY-SF.bin is generated."; \
		exit 1; \
	fi;
	$(Q)if ! [ -e $(PRJ_IMAGE_DIR)/phy_sf_report.txt ]; then \
		echo "No phy_sf_report.txt is generated."; \
		exit 1; \
	fi;

PHONY += install 
install: nvram firmware phy_sf


endif # skip-makefile

# Make clean/distclean
#
MODULES-y-clean=$(patsubst %, %-clean, $(MODULES-y))
MODULES-n-clean=$(patsubst %, %-clean, $(MODULES-n))
MODULES--clean=$(patsubst %, %-clean, $(MODULES-))

distribute-pre-clean:
	rm -rf $(KERNEL_INC_DIR) $(KERNEL_LIB_DIR)
	rm -rf $(MIDDLEWARE_INC_DIR) $(MIDDLEWARE_LIB_DIR)
	rm -rf $(DRIVER_INC_DIR) $(DRIVER_LIB_DIR)
	rm -rf $(PRJ_INC_DIR) $(PRJ_LIB_DIR)
	rm -rf $(PRJ_OBJ_DIR)
	rm -rf $(KERNEL_INC_DIR_RESCUE) $(KERNEL_LIB_DIR_RESCUE)
	rm -rf $(MIDDLEWARE_INC_DIR_RESCUE) $(MIDDLEWARE_LIB_DIR_RESCUE)
	rm -rf $(DRIVER_INC_DIR_RESCUE) $(DRIVER_LIB_DIR_RESCUE)
	rm -rf $(PRJ_INC_DIR_RESCUE) $(PRJ_LIB_DIR_RESCUE)

ifeq ($(skip-makefile),)
CLEAN-depend := $(MODULES-y-clean) 
else
CLEAN-depend = 
endif

PHONY += imagetool-clean
imagetool-clean:
	$(MAKE) clean -C $(IMGTOOL_DIR)
	$(MAKE) clean -C $(HW_DIR)/nvram/

PHONY += clean
clean: $(CLEAN-depend) imagetool-clean distribute-pre-clean Makefile.version-clean
	@echo	'make clean done'

PHONY += distclean
distclean: clean
	$(Q)find scripts \( -name '*.[oas]' -o -name '*.ko' -o -name '.*.cmd' \
		-o -name '.*.d' -o -name '.*.tmp' -o -name '*.mod.c' \) \
		-type f -print | xargs rm -f	
	$(Q)find scripts \( -name 'fixdep' -o -name 'docproc' -o -name 'split-include' \
		-o -name 'autoconf.h' -o -name '.config' -o -name '.config.old' \
		-o -name 'qconf' -o -name 'gconf' -o -name 'kxgettext' \
		-o -name 'mconf' -o -name 'conf' -o -name 'lxdialog' \) \
		-type f -print | xargs rm -f
	$(Q)rm -rf $(SDK_CONF_DIR)
	$(Q)rm -rf $(PRJ_IMAGE_DIR)
	$(Q)rm -f $(KCONFIG_CONFIG)
	$(Q)rm -f ./*.mod
	$(Q)rm -f .config.old
	@echo 'make distclean done'


#
# Make help
#

PHONY += help
help:
	@echo  'Cleaning targets:'
	@echo  '  clean		  - Remove most generated files but keep the config and'
	@echo  '                    enough build support to build external modules'
	@echo  '  distclean	  -'
	@echo  ''
	@echo  'Configuration targets:'
	@$(MAKE) -f $(srctree)/scripts/kconfig/Makefile help
	@echo  ''
	@echo  '  all		  - Build all targets marked with [*]'
	@echo  '  install	  - Build all images'
	@echo  '  image		  - Build all images. same as make install'
	@echo  '  showmodules	  - show all the module name'
	@echo  '  [module name]	  - Build the module'
	@echo  '  [module name]-clean	  - Remove most generated file of the module'

endif #config-targets
#endif # mixed-target

PHONY += FORCE
FORCE: 

# Declare the contents of the .PHONY variable as phony.  We keep that
# information in a variable so we can use it in if_changed and friends.
.PHONY: $(PHONY)

# generate mavlink headers
mavlink:
	mavgen.py --lang C -o include/generated/mavlink --wire-protocol=2.0 ../modules/mavlink/message_definitions/v1.0/ardupilotmega.xml

iwlist:
	sudo iwlist wlan0 scanning|grep -i essid

upload:
	echo "Uploading firmware to 192.168.99.1:2018"
	cat ../image/dashcam/FIRMWARE_660R.bin | nc 192.168.99.1 2018

# generate mavlink headers
mavlink:
	mavgen.py --lang C -o include/generated/mavlink --wire-protocol=2.0 ../modules/mavlink/message_definitions/v1.0/ardupilotmega.xml

Makefile.version-clean:
	rm $(subst -clean,.old,$@)

Makefile.version.old:
	cp $(subst .old,,$@) $@

Makefile.version: Makefile.version.old
	./Makever.sh $@
