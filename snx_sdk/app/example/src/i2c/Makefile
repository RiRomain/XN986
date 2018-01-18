PWD		:= $(shell pwd)
ARCH		?= arm
CROSS_COMPILE	?= $(ARCH)-linux-
vKERNEL		?= 2.6.35.12
CPP     	:= $(CROSS_COMPILE)g++
CC      	:= $(CROSS_COMPILE)gcc
AR      	:= $(CROSS_COMPILE)ar
RANLIB      := $(CROSS_COMPILE)ranlib

MIDDLEWARE_INS_DIR ?= ../../../../middleware/_install
KERNEL_DIR  ?= ../../../../kernel/linux-2.6.35.12
AUTOCONF_DIR    ?= ../../../../buildscript/include


INSTALL_DIR	?= ../rootfs/usr
BIN_INS_DIR	:= $(INSTALL_DIR)/bin/example
LIB_INS_DIR	:= $(INSTALL_DIR)/lib/example

I2C	:= i2c
export ARCH CROSS_COMPILE

MODULE = snx_i2c_api_module
targets = snx_i2c_test

obj-m	:= $(MODULE).o


all:
#	@ if [ -z "$(KERNEL_DIR)" ]; then \
#		echo "Error! Please set kernel directory by 'KERNEL_DIR' variable"; \
#		exit 1; \
#	fi;
#	$(MAKE) -C $(KERNEL_DIR)/src M=$(PWD) modules
#	arm-linux-gcc -o $(targets) $(targets).c
	arm-linux-gcc -o snx_i2c_ctl snx_i2c_ctl.c	

.PHONY: clean
clean:
	-rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions Module.symvers $(targets) snx_i2c_ctl modules.order

.PHONY: install
install:
	@ if [ ! -d $(BIN_INS_DIR)/$(I2C) ]; \
	then \
		install -d $(BIN_INS_DIR)/$(I2C); \
	fi
#	@ if [ ! -d $(LIB_INS_DIR)/$(I2C) ]; \
#	then \
#		install -d $(LIB_INS_DIR)/$(I2C); \
#	fi
#	install -c $(MODULE).ko $(LIB_INS_DIR)/$(I2C)
#	install -c $(targets) $(BIN_INS_DIR)/$(I2C)
	install -c snx_i2c_ctl $(BIN_INS_DIR)/$(I2C)

