#
# Make Related Variables 
#
MAKE            ?= make
PWD             := $(shell pwd)
SRC_DIR		?= $(PWD)/src
PERL		:= perl
SYSPARSE	:= sysparser.pl

#
# Images 
#
UBOOT_FILE	:= $(DIST_DIR)/u-boot.bin 
KERNEL_FILE	:= $(DIST_DIR)/uImage
RESCUE_FILE	:= $(DIST_DIR)/rescue_uImage
ROOTFS_R_FILE	:= $(DIST_DIR)/rootfs.cramfs
ROOTFS_RW_FILE	:= $(DIST_DIR)/rootfs.jffs2
ULOGO_FILE	:= $(DIST_DIR)/u-logo.bmp
ENGINE_FILE	:= $(DIST_DIR)/engine.bin
DONE_UBOOT	:= $(DIST_DIR)/UBOOT.bin
DONE_KERNEL	:= $(DIST_DIR)/KERNEL.bin
DONE_ROOTFS_R	:= $(DIST_DIR)/ROOTFS-R.bin
DONE_NVRAM	:= $(DIST_DIR)/nvram.bin
DONE_RESCUE	:= $(DIST_DIR)/RESCUE.bin
SYSCONFIG_FILE	:= $(SCRIPTS_DIR)/systemConfig.xls
AUTOCONF_DIR	?= ../../buildscript/include
KCONFIG_CONFIG	:= .config

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

SONIX_660 ?= $(if $(CONFIG_SYSTEM_PLATFORM_SN98660),yes,$(if $(CONFIG_SYSTEM_PLATFORM_ST58660FPGA),yes,no))
ifeq ($(SONIX_660), yes)
	SONIX_DDR_PROJECT = $(CONFIG_MAIN_SETTING)_$(CONFIG_MEM_SIZE_STRING)
else
	SONIX_DDR_PROJECT = $(CONFIG_DDR_PROJECT)
endif

.PHONY: bootimage kernelimage rootfsimage rootfs_rwimage firmware engine
bootimage: $(DIST_DIR)/u-logo.bmp
	$(PERL) $(SYSPARSE) \
	dotconfig=$(KCONFIG_CONFIG) \
	image=uboot \
	platform=$(MACHINE) \
	hw_version=$(VER_WHCH) \
	flash_type=$(FLASH) \
	uboot=$(UBOOT_FILE) \
	kernel=$(KERNEL_FILE) \
	rootfs_r=$(ROOTFS_R_FILE) \
	rootfs_rw=$(ROOTFS_RW_FILE) \
	u_logo=$(ULOGO_FILE) \
	engine=$(ENGINE_FILE) \
	ddr_freq=$(CONFIG_DDR_FREQ) \
	ddr_project=$(SONIX_DDR_PROJECT) \
	ddr0_ddr_str=$(CONFIG_FIRST_DDR_STRING)	\
	ddr1_ddr_str=$(CONFIG_SECOND_DDR_STRING)	\
	ddr2_ddr_str=$(CONFIG_THIRD_DDR_STRING)	\
	st58660_type=$(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) \
	sn98660_type=$(CONFIG_SYSTEM_PLATFORM_SN98660)	\
	sdk_version="$(sdk_version)" \
	isp_path=$(srctree)/scripts/configs/ISP/ \
	config_path=$(HW_DIR)/ \
	done_uboot_file=$(DONE_UBOOT)	\
	done_kernel_file=$(DONE_KERNEL)	\
	done_rootfsr_file=$(DONE_ROOTFS_R)	\
	done_nvram_file=$(DONE_NVRAM)\
	
#u-logo.bmp should be modified when imgtool is ready
$(DIST_DIR)/u-logo.bmp:
	$(Q)mkdir -p $(DIST_DIR)
	cp $(SCRIPTS_DIR)/u-logo.bmp $@

rescueimage:
	$(PERL) $(SYSPARSE) \
	dotconfig=$(KCONFIG_CONFIG) \
	image=rescue \
	platform=$(MACHINE) \
	hw_version=$(VER_WHCH) \
	flash_type=$(FLASH) \
	uboot=$(UBOOT_FILE) \
	rescue=$(RESCUE_FILE) \
	rootfs_r=$(ROOTFS_R_FILE) \
	rootfs_rw=$(ROOTFS_RW_FILE) \
	u_logo=$(ULOGO_FILE) \
	engine=$(ENGINE_FILE) \
	ddr_freq=$(CONFIG_DDR_FREQ) \
	ddr_project=$(SONIX_DDR_PROJECT) \
	sdk_version="$(sdk_version)" \
	isp_path=$(srctree)/scripts/configs/ISP/ \
	config_path=$(HW_DIR)/ \
	done_uboot_file=$(DONE_UBOOT)	\
	done_rescue_file=$(DONE_RESCUE)	\
	done_rootfsr_file=$(DONE_ROOTFS_R)	\
	done_nvram_file=$(DONE_NVRAM)\

kernelimage:
	$(PERL) $(SYSPARSE) \
	dotconfig=$(KCONFIG_CONFIG) \
	image=kernel \
	platform=$(MACHINE) \
	hw_version=$(VER_WHCH) \
	flash_type=$(FLASH) \
	uboot=$(UBOOT_FILE) \
	kernel=$(KERNEL_FILE) \
	rootfs_r=$(ROOTFS_R_FILE) \
	rootfs_rw=$(ROOTFS_RW_FILE) \
	u_logo=$(ULOGO_FILE) \
	engine=$(ENGINE_FILE) \
	ddr_freq=$(CONFIG_DDR_FREQ) \
	ddr_project=$(SONIX_DDR_PROJECT) \
	sdk_version="$(sdk_version)" \
	isp_path=$(srctree)/scripts/configs/ISP/ \
	config_path=$(HW_DIR)/ \
	done_uboot_file=$(DONE_UBOOT)	\
	done_kernel_file=$(DONE_KERNEL)	\
	done_rootfsr_file=$(DONE_ROOTFS_R)	\
	done_nvram_file=$(DONE_NVRAM)

rootfsimage:
	$(PERL) $(SYSPARSE) \
	dotconfig=$(KCONFIG_CONFIG) \
	image=rootfs-r \
	platform=$(MACHINE) \
	hw_version=$(VER_WHCH) \
	flash_type=$(FLASH) \
	uboot=$(UBOOT_FILE) \
	kernel=$(KERNEL_FILE) \
	rootfs_r=$(ROOTFS_R_FILE) \
	rootfs_rw=$(ROOTFS_RW_FILE) \
	u_logo=$(ULOGO_FILE) \
	engine=$(ENGINE_FILE) \
	ddr_freq=$(CONFIG_DDR_FREQ) \
	ddr_project=$(SONIX_DDR_PROJECT) \
	sdk_version="$(sdk_version)" \
	isp_path=$(srctree)/scripts/configs/ISP/ \
	config_path=$(HW_DIR)/ \
	done_uboot_file=$(DONE_UBOOT)	\
	done_kernel_file=$(DONE_KERNEL)	\
	done_rootfsr_file=$(DONE_ROOTFS_R)	\
	done_nvram_file=$(DONE_NVRAM)

rootfs_rwimage:
	$(PERL) $(SYSPARSE) \
	dotconfig=$(KCONFIG_CONFIG) \
	image=rootfs-rw \
	platform=$(MACHINE) \
	hw_version=$(VER_WHCH) \
	flash_type=$(FLASH) \
	uboot=$(UBOOT_FILE) \
	kernel=$(KERNEL_FILE) \
	rootfs_r=$(ROOTFS_R_FILE) \
	rootfs_rw=$(ROOTFS_RW_FILE) \
	u_logo=$(ULOGO_FILE) \
	engine=$(ENGINE_FILE) \
	ddr_freq=$(CONFIG_DDR_FREQ) \
	ddr_project=$(SONIX_DDR_PROJECT) \
	sdk_version="$(sdk_version)" \
	isp_path=$(srctree)/scripts/configs/ISP/ \
	config_path=$(HW_DIR)/ \
	done_uboot_file=$(DONE_UBOOT)	\
	done_kernel_file=$(DONE_KERNEL)	\
	done_rootfsr_file=$(DONE_ROOTFS_R)	\
	done_nvram_file=$(DONE_NVRAM)
	
firmware_factory:
	$(PERL) $(SYSPARSE) \
	dotconfig=$(KCONFIG_CONFIG) \
	image=firmware_factory \
	platform=$(MACHINE) \
	hw_version=$(VER_WHCH) \
	flash_type=$(FLASH) \
	uboot=$(UBOOT_FILE) \
	kernel=$(KERNEL_FILE) \
	rootfs_r=$(ROOTFS_R_FILE) \
	rootfs_rw=$(ROOTFS_RW_FILE) \
	u_logo=$(ULOGO_FILE) \
	engine=$(ENGINE_FILE) \
	ddr_freq=$(CONFIG_DDR_FREQ) \
	ddr_project=$(SONIX_DDR_PROJECT) \
	ddr0_ddr_str=$(CONFIG_FIRST_DDR_STRING)	\
	ddr1_ddr_str=$(CONFIG_SECOND_DDR_STRING)	\
	ddr2_ddr_str=$(CONFIG_THIRD_DDR_STRING)	\
	sdk_version="$(sdk_version)" \
	isp_path=$(srctree)/scripts/configs/ISP/ \
	config_path=$(HW_DIR)/ \
	done_uboot_file=$(DONE_UBOOT) \
	done_kernel_file=$(DONE_KERNEL) \
	done_rootfsr_file=$(DONE_ROOTFS_R) \
	done_nvram_file=$(DONE_NVRAM) \
	done_rescue_file=$(DONE_RESCUE) \
	rescue_system=$(CONFIG_RESCUE_SYSTEM)	\
	st58660_type=$(CONFIG_SYSTEM_PLATFORM_ST58660FPGA)	\
	sn98660_type=$(CONFIG_SYSTEM_PLATFORM_SN98660)	\
	uboot_update=$(CONFIG_UBOOT_UPDATE)	\
	kernel_update=$(CONFIG_KERNEL_UPDATE)	\
	rootfs_update=$(CONFIG_ROOTFS_UPDATE)	\
	jffs2_update=$(CONFIG_JFFS2_UPDATE)	\
	rescue_update=$(CONFIG_RESCUE_UPDATE)	\
	etc_update=$(CONFIG_ETC_UPDATE)	\
	user_update=$(CONFIG_USER_UPDATE)	\
	factory_update=$(CONFIG_FACTORY_UPDATE)	\
	custom1_update=$(CONFIG_CUSTOM1_UPDATE)	\
	custom2_update=$(CONFIG_CUSTOM2_UPDATE)	\
	
firmware:
	$(PERL) $(SYSPARSE) \
	dotconfig=$(KCONFIG_CONFIG) \
	image=firmware \
	platform=$(MACHINE) \
	hw_version=$(VER_WHCH) \
	flash_type=$(FLASH) \
	uboot=$(UBOOT_FILE) \
	kernel=$(KERNEL_FILE) \
	rootfs_r=$(ROOTFS_R_FILE) \
	rootfs_rw=$(ROOTFS_RW_FILE) \
	u_logo=$(ULOGO_FILE) \
	engine=$(ENGINE_FILE) \
	ddr_freq=$(CONFIG_DDR_FREQ) \
	ddr_project=$(SONIX_DDR_PROJECT) \
	ddr0_ddr_str=$(CONFIG_FIRST_DDR_STRING)	\
	ddr1_ddr_str=$(CONFIG_SECOND_DDR_STRING)	\
	ddr2_ddr_str=$(CONFIG_THIRD_DDR_STRING)	\
	sdk_version="$(sdk_version)" \
	isp_path=$(srctree)/scripts/configs/ISP/ \
	config_path=$(HW_DIR)/ \
	done_uboot_file=$(DONE_UBOOT) \
	done_kernel_file=$(DONE_KERNEL) \
	done_rootfsr_file=$(DONE_ROOTFS_R) \
	done_nvram_file=$(DONE_NVRAM) \
	done_rescue_file=$(DONE_RESCUE) \
	rescue_system=$(CONFIG_RESCUE_SYSTEM)	\
	st58660_type=$(CONFIG_SYSTEM_PLATFORM_ST58660FPGA)	\
	sn98660_type=$(CONFIG_SYSTEM_PLATFORM_SN98660)	\
	uboot_update=$(CONFIG_UBOOT_UPDATE)	\
	kernel_update=$(CONFIG_KERNEL_UPDATE)	\
	rootfs_update=$(CONFIG_ROOTFS_UPDATE)	\
	jffs2_update=$(CONFIG_JFFS2_UPDATE)	\
	rescue_update=$(CONFIG_RESCUE_UPDATE)	\
	etc_update=$(CONFIG_ETC_UPDATE)	\
	user_update=$(CONFIG_USER_UPDATE)	\
	factory_update=$(CONFIG_FACTORY_UPDATE)	\
	custom1_update=$(CONFIG_CUSTOM1_UPDATE)	\
	custom2_update=$(CONFIG_CUSTOM2_UPDATE)	

engine:
	$(PERL) $(SYSPARSE) \
	dotconfig=$(KCONFIG_CONFIG) \
	image=engine \
	platform=$(MACHINE) \
	hw_version=$(VER_WHCH) \
	flash_type=$(FLASH) \
	uboot=$(UBOOT_FILE) \
	kernel=$(KERNEL_FILE) \
	rootfs_r=$(ROOTFS_R_FILE) \
	rootfs_rw=$(ROOTFS_RW_FILE) \
	u_logo=$(ULOGO_FILE) \
	engine=$(ENGINE_FILE) \
	ddr_freq=$(CONFIG_DDR_FREQ) \
	ddr_project=$(SONIX_DDR_PROJECT) \
	sdk_version="$(sdk_version)" \
	isp_path=$(srctree)/scripts/configs/ISP/ \
	config_path=$(HW_DIR)/ \
	done_uboot_file=$(DONE_UBOOT)	\
	done_kernel_file=$(DONE_KERNEL)	\
	done_rootfsr_file=$(DONE_ROOTFS_R)	\
	done_nvram_file=$(DONE_NVRAM)

PHONY += clean
clean:
	rm -f ./*.d ./isp.txt ./u-logo.bmp ./flash_layout_firmware.bin ./src/header/*.o ./src/code/*.o ./src/header/flash_layout.h
	$(Q)$(MAKE) -C ./src/code clean && $(Q)$(MAKE) -C ./src/header clean && $(Q)$(MAKE) -C ./phy_sf clean && $(Q)$(MAKE) -C ./src/fw_update clean && $(Q)$(MAKE) -C ./src/md5_encrypt clean
