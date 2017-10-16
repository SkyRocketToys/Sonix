PWD			:= $(shell pwd)
MAKE			?= make
SRC_DIR			:= $(PWD)/src
INSTALL_DIR		?= $(PWD)/target
OBJ_DIR			?= $(PWD)/obj

.PHONY: all clean distclean install

all:
	$(MAKE) -C $(SRC_DIR) $@ OBJ_DIR=$(abspath $(OBJ_DIR))

clean:
	$(MAKE) -C $(SRC_DIR) $@ OBJ_DIR=$(abspath $(OBJ_DIR))


distclean:
	$(MAKE) -C $(SRC_DIR) $@


install:
	$(MAKE) -C $(SRC_DIR) $@ INSTALL_DIR=$(abspath $(INSTALL_DIR)) OBJ_DIR=$(abspath $(OBJ_DIR))
	rm -rf $(PWD)/target/lib/libzbar.la
	rm -rf $(PWD)/target/lib/pkgconfig

