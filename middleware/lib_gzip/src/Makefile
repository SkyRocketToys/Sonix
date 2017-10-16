PWD		:= $(shell pwd)
INSTALL_DIR	?= $(PWD)/../target
INSTALL_INC_DIR	:= $(INSTALL_DIR)/include
OBJ_DIR		?= $(PWD)/obj

GZIP_VER	?= 1.2.4
GZIP_SRC_DIR	:= gzip-$(GZIP_VER)

.PHONY: all clean distclean install
all:
	$(MAKE) -C $(abspath $(GZIP_SRC_DIR)) $@ OBJ_DIR=$(abspath $(OBJ_DIR))

clean:
	$(MAKE) -C $(GZIP_SRC_DIR) $@ OBJ_DIR=$(abspath $(OBJ_DIR))

distclean:
	$(MAKE) -C $(GZIP_SRC_DIR) $@
	
install: 
	$(MAKE) -C $(GZIP_SRC_DIR) $@ \
		INSTALL_DIR=$(abspath $(INSTALL_DIR)) \
		OBJ_DIR=$(abspath $(OBJ_DIR))

