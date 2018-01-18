MAKE            ?= make
PWD             := $(shell pwd)
SRC_DIR		?= $(PWD)/src
OBJ_DIR		?= $(PWD)/obj
ARCH		?= arm
CROSS_COMPILE	?= $(ARCH)-linux-
INSTALL_DIR	?= $(PWD)/image
PATCH_DIR	?= $(PWD)/patch
AUTOCONF_DIR	?= ../../buildscript/include

include $(AUTOCONF_DIR)/config/snx_sdk.conf
FLASH           ?= $(if $(CONFIG_SYSTEM_NAND_FLASH_SUPPORT),nand,$(if $(CONFIG_SYSTEM_SERIAL_FLASH_SUPPORT),sf,nand))
MACHINE         ?= $(if $(CONFIG_SYSTEM_PLATFORM_ST58600FPGA),sn986xx_fpga,$(CONFIG_SYSTEM_PLATFORM))
RESCUE_FS_STR	?= "$(subst /,\/,$(RESCUE_FS_DIR))"

export ARCH CROSS_COMPILE

.PHONY: all clean distclean install menuconfig 

all: $(OBJ_DIR)/.config
	$(Q)$(MAKE) -C $(OBJ_DIR) uImage

menuconfig: $(OBJ_DIR)/.config
	$(Q)$(MAKE) -C $(OBJ_DIR) $@

$(OBJ_DIR)/.config:$(AUTOCONF_DIR)/config/snx_sdk.conf
	$(MAKE) -C $(PATCH_DIR) .patched
	@ if ! [ -d $(OBJ_DIR) ]; then \
		mkdir -p $(OBJ_DIR); \
	fi
ifeq ($(COMPILE_RESCUE), yes)
	$(MAKE) -C $(SRC_DIR) O=$(OBJ_DIR) $(CONFIG_KERNEL_RESCUE_DEF_CONFIG)
else	
	$(MAKE) -C $(SRC_DIR) O=$(OBJ_DIR) $(CONFIG_KERNEL_NORMAL_DEF_CONFIG)
endif
	$(Q)sed -i -e 's/CONFIG_PLL_CLK=.*$$/CONFIG_PLL_CLK=$(CONFIG_SYSTEM_PERFORMANCE_CLK)/g' $(OBJ_DIR)/.config;
	$(Q)sed -i -e 's/CONFIG_TICK_CLOCK_RATIO=.*$$/CONFIG_TICK_CLOCK_RATIO=$(CONFIG_SYSTEM_APB_CLOCK_RATIO)/g' $(OBJ_DIR)/.config;

ifeq ($(CONFIG_SYSTEM_PLATFORM_ST58660FPGA), y)
	$(Q)sed -i -e 's/CONFIG_TICK_CLOCK_RATIO=.*$$/CONFIG_TICK_CLOCK_RATIO=8/g' $(OBJ_DIR)/.config;
endif

clean:
	$(Q) if [ -d $(OBJ_DIR) ]; then \
		$(MAKE) -C $(OBJ_DIR) $@; \
	fi
	$(MAKE) -C $(PATCH_DIR) $@


distclean:
	$(Q) if [ -d $(OBJ_DIR) ]; then \
		$(MAKE) -C $(OBJ_DIR) mrproper; \
		rm -rf $(OBJ_DIR); \
	fi


install:
	@ if ! [ -d $(INSTALL_DIR) ]; then \
		install -d $(INSTALL_DIR); \
	fi
ifeq ($(COMPILE_RESCUE), yes)
	install -c $(OBJ_DIR)/arch/$(ARCH)/boot/uImage $(INSTALL_DIR)/rescue_uImage
else
	install -c $(OBJ_DIR)/arch/$(ARCH)/boot/uImage $(INSTALL_DIR)
endif

