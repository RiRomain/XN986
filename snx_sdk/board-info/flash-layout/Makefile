#
# Make Related Variables 
#
PERL		:= perl
PARTITIONPARSE	:= parse_partition.pl
AUTOCONF_DIR	?= ../../buildscript/include

#
#	include
#
# Read in config
-include $(AUTOCONF_DIR)/config/snx_sdk.conf
FLASH           ?= $(if $(CONFIG_SYSTEM_NAND_FLASH_SUPPORT),nand,$(if $(CONFIG_SYSTEM_SERIAL_FLASH_SUPPORT),sf,nand))
MACHINE         ?= $(CONFIG_SYSTEM_PLATFORM)

ifeq ($(CONFIG_NVRAM_PART), y)
NVRAM_USER_SIZE = $(CONFIG_NVRAM_PART_SIZE)
ifeq ($(CONFIG_NVRAM_FACTORY_SIZE), y)
NVRAM_FACTORY_SIZE = $(CONFIG_NVRAM_PART_SIZE)
else
NVRAM_FACTORY_SIZE = 0
endif
else
NVRAM_USER_SIZE = 0
NVRAM_FACTORY_SIZE = 0
endif

ifeq ($(CONFIG_JFFS2_PART), y)
JFFS2_PART_SIZE = $(CONFIG_JFFS2_PART_SIZE)
JFFS2_PART_MOUNT = $(CONFIG_JFFS2_PART_MOUNT)
else
JFFS2_PART_SIZE = 0
JFFS2_PART_MOUNT = "null"
endif

ifeq ($(CONFIG_RESCUE_SYSTEM_PART), y)
RESCUE_PART_SIZE = $(CONFIG_RESCUE_PART_SIZE)
else
RESCUE_PART_SIZE = 0
endif

ifeq ($(CONFIG_CUSTOM1_PARTITION), y)
CUSTOM1_PART_SIZE = $(CONFIG_CUSTOM1_PART_SIZE)
CUSTOM1_IMAGE_NAME = $(CONFIG_CUSTOM1_IMAGE)
ifeq ($(CONFIG_CUSTOM1_MOUNT), y)
CUSTOM1_PART_MOUNT = $(CONFIG_CUSTOM1_PART_MOUNT)
CUSTOM1_MOUNT_TYPE ?= $(if $(CONFIG_CUSTOM1_MOUNT_EXT2),ext2,$(if $(CONFIG_CUSTOM1_MOUNT_JFFS2),jffs2,ext2))
else
CUSTOM1_PART_MOUNT = "null"
CUSTOM1_MOUNT_TYPE = "null"
endif
else
CUSTOM1_PART_SIZE = 0
CUSTOM1_IMAGE_NAME = "null"
CUSTOM1_PART_MOUNT = "null"
CUSTOM1_MOUNT_TYPE = "null"
endif

ifeq ($(CONFIG_CUSTOM2_PARTITION), y)
CUSTOM2_PART_SIZE = $(CONFIG_CUSTOM2_PART_SIZE)
CUSTOM2_IMAGE_NAME = $(CONFIG_CUSTOM2_IMAGE)
ifeq ($(CONFIG_CUSTOM2_MOUNT), y)
CUSTOM2_PART_MOUNT = $(CONFIG_CUSTOM2_PART_MOUNT)
CUSTOM2_MOUNT_TYPE ?= $(if $(CONFIG_CUSTOM2_MOUNT_EXT2),ext2,$(if $(CONFIG_CUSTOM2_MOUNT_JFFS2),jffs2,ext2))
else
CUSTOM2_PART_MOUNT = "null"
CUSTOM2_MOUNT_TYPE = "null"
endif
else
CUSTOM2_PART_SIZE = 0
CUSTOM2_IMAGE_NAME = "null"
CUSTOM2_PART_MOUNT = "null"
CUSTOM2_MOUNT_TYPE = "null"
endif

LOGO_PART_SIZE = 0

FLASH_SIZE ?= $(if $(CONFIG_SYSTEM_SERIAL_FLASH_16M),16,$(if $(CONFIG_SYSTEM_SERIAL_FLASH_32M),32,16))

ifeq ($(CONFIG_SYSTEM_PLATFORM_ST58660FPGA), y)
PLATFORM_NAME="sn9866x"
else
ifeq ($(CONFIG_SYSTEM_PLATFORM_SN98660), y)
PLATFORM_NAME="sn9866x"
else
PLATFORM_NAME="sn986xx"
endif
endif

#
# target
#
.PHONY: serial_flashlayout.conf
serial_flashlayout.conf:
	$(PERL) $(PARTITIONPARSE) \
	nvram_user_size=$(NVRAM_USER_SIZE) \
	nvram_factory_size=$(NVRAM_FACTORY_SIZE)	\
	etc_part_size=$(CONFIG_ETC_PART_SIZE)	\
	kernel_part_size=$(CONFIG_KERNEL_PART_SIZE)	\
	rootfsr_part_size=$(CONFIG_ROOTFS_PART_SIZE)	\
	jffs2_part_size=$(JFFS2_PART_SIZE)	\
	jffs2_part_mount=$(JFFS2_PART_MOUNT)	\
	rescue_part_size=$(RESCUE_PART_SIZE)	\
	custom1_part_size=$(CUSTOM1_PART_SIZE)	\
	custom1_image_name=$(CUSTOM1_IMAGE_NAME)	\
	custom1_part_mount=$(CUSTOM1_PART_MOUNT)	\
	custom1_mount_type=$(CUSTOM1_MOUNT_TYPE)	\
	custom2_part_size=$(CUSTOM2_PART_SIZE)	\
	custom2_image_name=$(CUSTOM2_IMAGE_NAME)	\
	custom2_part_mount=$(CUSTOM2_PART_MOUNT)	\
	custom2_mount_type=$(CUSTOM2_MOUNT_TYPE)	\
	addto_mount_partition=$(ADDTO_MOUNT_PARTITION)	\
	flash_total_size=$(FLASH_SIZE)	\
	logo_part_size=$(LOGO_PART_SIZE)	\
	image_dir=$(DIST_DIR)	\
	rootfs_dir=$(FS_DIR)	\
	rescuefs_dir=$(RESCUE_FS_DIR)	\
	platform_name=$(PLATFORM_NAME)	\
	
PHONY += clean
clean:
	rm -rf ./serial_flashlayout.conf
