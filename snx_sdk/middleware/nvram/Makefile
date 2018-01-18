PWD			:= $(shell pwd)
MAKE			?= make
SRC_DIR			:= $(PWD)/src
INSTALL_DIR		?= $(PWD)/rootfs
LIB_INS_DIR		?= $(PWD)/middleware

.PHONY: all clean distclean install

all:
	$(MAKE) -C $(SRC_DIR) $@ LIB_INSTALL_DIR=$(abspath $(LIB_INS_DIR))


clean:
	$(MAKE) -C $(SRC_DIR) $@


distclean:
	$(MAKE) -C $(SRC_DIR) $@


install:
	$(MAKE) -C $(SRC_DIR) $@ INSTALL_DIR=$(abspath $(INSTALL_DIR))


