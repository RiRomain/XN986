PWD			:= $(shell pwd)
MAKE			?= make
INSTALL_DIR		?= $(PWD)/../rootfs
LIB_INSTALL_DIR		?= $(INSTALL_DIR)

#SDL_LIB_SRC_DIR		:= $(PWD)/sdl-lib

LIB_SRC_DIRS		:= $(PWD)/nvram-lib
LIB_SRC_DIRS		+= $(PWD)/nvram-utility
#LIB_SRC_DIRS		+= $(PWD)/nvram-init
LIBNVRAM_DIR		?= $(PWD)/../../../buildscript/include/libnvram
export LIBNVRAM_DIR
export MAKE

.PHONY: all clean distclean install
.PHONY: library library_clean library_distclean
all: library

library:
	@ for dir in $(LIB_SRC_DIRS); do $(MAKE) -C $$dir INSTALL_DIR=$(abspath $(LIB_INSTALL_DIR)) || exit 1; done
	@ for dir in $(LIB_SRC_DIRS); do $(MAKE) -C $$dir install INSTALL_DIR=$(abspath $(LIB_INSTALL_DIR)) || exit 1; done


clean: library_clean
	@echo $@

library_clean:
	@ for dir in $(LIB_SRC_DIRS); do $(MAKE) -C $$dir clean || exit 1; done

distclean: library_distclean

library_distclean:
	@echo $@
	@ for dir in $(LIB_SRC_DIRS); do $(MAKE) -C $$dir distclean || exit 1; done

install:
	@ for dir in $(LIB_SRC_DIRS); do $(MAKE) -C $$dir install INSTALL_DIR=$(abspath $(INSTALL_DIR)) || exit 1; done

