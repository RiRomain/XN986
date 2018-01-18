#
# Make Related Variables 
#
MAKE            ?= make
PWD             := $(shell pwd)
SRC_DIR		?= $(PWD)/src
PERL		:= perl
PHYSF	:= phy_sf.pl

#
# Images 
#
UBOOT_FILE	:= $(DIST_DIR)/u-boot.bin 
KERNEL_FILE	:= $(DIST_DIR)/uImage
ROOTFS_R_FILE	:= $(DIST_DIR)/rootfs.cramfs
ROOTFS_RW_FILE	:= $(DIST_DIR)/rootfs.jffs2
ULOGO_FILE	:= $(DIST_DIR)/u-logo.bmp
ENGINE_FILE	:= $(DIST_DIR)/engine.bin
DONE_UBOOT	:= $(DIST_DIR)/UBOOT.bin
DONE_KERNEL	:= $(DIST_DIR)/KERNEL.bin
DONE_ROOTFS_R	:= $(DIST_DIR)/ROOTFS-R.bin
DONE_NVRAM	:= $(DIST_DIR)/nvram.bin
PHY_FLASH_LAYOUT := $(HW_DIR)/flash-layout/serial_flashlayout.conf

#
#	include
#
# Read in config
-include $(AUTOCONF_DIR)/config/snx_sdk.conf
FLASH           ?= $(if $(CONFIG_SYSTEM_NAND_FLASH_SUPPORT),nand,$(if $(CONFIG_SYSTEM_SERIAL_FLASH_SUPPORT),sf,nand))
MACHINE         ?= $(CONFIG_SYSTEM_PLATFORM)

VER_HW ?= $(if $(CONFIG_SYSTEM_PREVIOUS_PLATFORM),ver)
ifeq ($(VER_HW),ver)
	VER_WHCH = $(CONFIG_SYSTEM_PREVIOUS_VERSION)
else
	VER_WHCH = none
endif

.PHONY: phy_sf
phy_sf:
	$(PERL) $(PHYSF) \
	outir=$(DIST_DIR) \
	src_dir=$(SRC_DIR) \
	phy_flashlayout=$(PHY_FLASH_LAYOUT) \
	sdk_version=$(sdk_version)

PHONY += clean
clean:
	$(Q)$(MAKE) -C $(SRC_DIR)/ clean
