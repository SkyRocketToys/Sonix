PWD			:= $(shell pwd)
MAKE			?= make
INSTALL_DIR		?= $(PWD)/../rootfs
LIB_INSTALL_DIR		?= $(PWD)/../target

LIB_SRC_DIRS		:= $(PWD)/zbar-0.10-lib
export MAKE

ARCH			?= arm
TARGET			?= $(ARCH)-linux
CROSS_COMPILE		?= $(TARGET)-
CC			= $(CROSS_COMPILE)gcc
AR			= $(CROSS_COMPILE)ar

CFLAGS			= $(PRJ_CFLAGS)
CFLAGS			+= -I$(PWD)
CFLAGS			+= -I$(KERNEL_INC_DIR)
CFLAGS			+= -I$(DRIVER_INC_DIR)

.PHONY: all clean distclean install
.PHONY: library library_clean library_distclean
all: library

$(LIB_SRC_DIRS)/Makefile: $(LIB_SRC_DIRS)/Makefile.in
	cd $(LIB_SRC_DIRS) && ./snx_configure.sh

library: $(LIB_SRC_DIRS)/Makefile $(PWD)/version.o 
	@ for dir in $(LIB_SRC_DIRS); do cd $$dir; $(MAKE) -C $$dir INSTALL_DIR=$(abspath $(LIB_INSTALL_DIR)) || exit 1; done
	@ for dir in $(LIB_SRC_DIRS); do $(MAKE) -C $$dir install prefix=$(abspath $(LIB_INSTALL_DIR)) || exit 1; done
	$(AR) -x ../target/lib/libzbar.a
	$(AR) -r ./libzbar.a *.o

$(PWD)/version.o:$(PWD)/version.c
	$(CC) -c -I$(SDK_CONF_DIR) $^ -o $@

clean: library_clean
	@echo $@

library_clean:
	@ for dir in $(LIB_SRC_DIRS); do \
	if [ -e $$dir/Makefile ]; then \
	$(MAKE) -C $$dir clean || exit 1;\
	fi;\
	done

distclean: library_distclean

library_distclean:
	@echo $@
	@ for dir in $(LIB_SRC_DIRS); do \
	if [ -e $$dir/Makefile ]; then \
	$(MAKE) -C $$dir distclean || exit 1;\
	fi;\
	done

install: 
	@ for dir in $(LIB_SRC_DIRS); do \
	if [ ! -e $$dir/Makefile ]; then \
	cd $$dir;sh snx_configure.sh ; $(MAKE) -C $$dir INSTALL_DIR=$(abspath $(LIB_INSTALL_DIR))||exit 1;\
	fi;\
	$(MAKE) -C $$dir install prefix=$(abspath $(LIB_INSTALL_DIR)) || exit 1;\
	done
	cp -avf $(PWD)/libzbar.a $(PWD)/../target/lib/
