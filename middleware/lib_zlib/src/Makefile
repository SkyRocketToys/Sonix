PWD		:= $(shell pwd)
INSTALL_DIR	?= $(PWD)/../target
INSTALL_INC_DIR	:= $(INSTALL_DIR)/include/libmid_zlib
OBJ_DIR		?=$(PWD)/obj

ZLIB_VER		?= 1.2.8
ZLIB_SRC_DIR	:= zlib-$(ZLIB_VER)
TARGET			?= arm-linux
TARGET_DIR		= $(INSTALL_DIR)
CFLAGS			= $(PRJ_CFLAGS)


$(ZLIB_SRC_DIR)/Makefile:
	cd $(ZLIB_SRC_DIR) && ./configure

.PHONY: all 
all:$(ZLIB_SRC_DIR)/Makefile
	cd $(ZLIB_SRC_DIR); \
	CHOST=$(TARGET) CFLAGS="$(CFLAGS)" ./configure --static --prefix=$(TARGET_DIR) \
	--includedir=$(INSTALL_INC_DIR);
	$(MAKE) -C $(ZLIB_SRC_DIR)

.PHONY: clean
clean:
	@ if [ -f $(ZLIB_SRC_DIR)/Makefile ]; \
	then \
		$(MAKE) -C $(ZLIB_SRC_DIR) clean; \
		$(MAKE) -C $(ZLIB_SRC_DIR) distclean; \
	fi
.PHONY: distclean
distclean:
	rm -rf $(OBJ_DIR) $(OBJS) $(STATIC_LIB)

.PHONY: install
install: 
	$(MAKE) -C $(ZLIB_SRC_DIR) install

