PWD		:= $(shell pwd)
ARCH		?= arm
CROSS_COMPILE	?= $(ARCH)-linux-
vKERNEL		?= 2.6.35.12
INSTALL_DIR	?= ../rootfs/usr
BIN_INS_DIR	:= $(INSTALL_DIR)/bin/example
LIB_INS_DIR	:= $(INSTALL_DIR)/lib/example
KERNEL_DIR ?= ../../../../kernel/linux-2.6.35.12/
TIMER	:= timer
export ARCH CROSS_COMPILE

MODULE = snx_timer_test_module


obj-m	:= $(MODULE).o


all:
	@ if [ -z "$(KERNEL_DIR)" ]; then \
		echo "Error! Please set kernel directory by 'KERNEL_DIR' variable"; \
		exit 1; \
	fi;
	$(MAKE) -C $(KERNEL_DIR)/src M=$(PWD) modules
	arm-linux-gcc -o snx_timer_test snx_timer_test.c

.PHONY: clean
clean:
	-rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions Module.symvers snx_timer_test modules.order

.PHONY: install
install:
	@ if [ ! -d $(BIN_INS_DIR)/$(TIMER) ]; \
	then \
		install -d $(BIN_INS_DIR)/$(TIMER); \
	fi
	@ if [ ! -d $(LIB_INS_DIR)/$(TIMER) ]; \
	then \
		install -d $(LIB_INS_DIR)/$(TIMER); \
	fi
	install -c $(MODULE).ko $(LIB_INS_DIR)/$(TIMER)
	install -c snx_timer_test $(BIN_INS_DIR)/$(TIMER)
	install -c timer_sh.sh $(BIN_INS_DIR)/$(TIMER)

