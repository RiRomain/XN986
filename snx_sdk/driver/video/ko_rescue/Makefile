PWD		:= $(shell pwd)
MAKE            ?= make
INSTALL_DIR	?= $(PWD)/../../
FIRMWARE_DIR	?= $(PWD)/../../
vKERNEL		?= 2.6.35.12
DRIVER_INS_DIR	:= $(INSTALL_DIR)/lib/modules/$(vKERNEL)/kernel/drivers
FIRMWARE_INS_DIR	:= $(INSTALL_DIR)/root/etc_default/firmware
KO_INSTALL_DIR	:= ../../ko

-include $(AUTOCONF_DIR)/config/snx_sdk.conf

#ISP2	= isp2
VC2	= vc2

ifeq ($(CONFIG_SYSTEM_SERIES),"sn9866x")
ifeq ($(CONFIG_SYSTEM_PLATFORM_ST58660FPGA),y)
	ISP2 = isp_st58660
else
ISP2 = isp3
endif
else
ISP2 = isp2
endif

.PHONY: all clean install
all:
	$(MAKE) -C $(ISP2) KERNEL_DIR=$(abspath $(KERNEL_DIR)) KO_INSTALL_DIR=$(abspath $(KO_INSTALL_DIR))/$(ISP2) || exit 1
	$(MAKE) -C $(VC2) KERNEL_DIR=$(abspath $(KERNEL_DIR)) KO_INSTALL_DIR=$(abspath $(KO_INSTALL_DIR))/$(VC2) || exit 1
	install -c Makefile $(KO_INSTALL_DIR)

clean: 
	$(MAKE) -C $(ISP2) KO_INSTALL_DIR=$(abspath $(KO_INSTALL_DIR))/$(ISP2) $@ || exit 1
	$(MAKE) -C $(VC2) $@ || exit 1

install:
	$(MAKE) -C $(ISP2) $@ INSTALL_DIR=$(abspath $(DRIVER_INS_DIR)) KO_INSTALL_DIR=$(abspath $(KO_INSTALL_DIR))/$(ISP2) FIRMWARE_DIR=$(abspath $(FIRMWARE_INS_DIR)) || exit 1
	$(MAKE) -C $(VC2) $@ INSTALL_DIR=$(abspath $(DRIVER_INS_DIR)) KO_INSTALL_DIR=$(abspath $(KO_INSTALL_DIR))/$(VC2) || exit 1
	
uninstall:
	$(MAKE) -C $(ISP2) $@ INSTALL_DIR=$(abspath $(DRIVER_INS_DIR)) KO_INSTALL_DIR=$(abspath $(KO_INSTALL_DIR))/$(ISP2) FIRMWARE_DIR=$(abspath $(FIRMWARE_INS_DIR)) || exit 1
	$(MAKE) -C $(VC2) $@ INSTALL_DIR=$(abspath $(DRIVER_INS_DIR)) KO_INSTALL_DIR=$(abspath $(KO_INSTALL_DIR))/$(VC2) || exit 1
