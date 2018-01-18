MAKE            ?= make
PWD             := $(shell pwd)
SRC_DIR		?= $(PWD)/src
ARCH		?= arm
CROSS_COMPILE	?= $(ARCH)-linux-
INSTALL_DIR	?= $(PWD)/rootfs
OBJ_DIR     ?= $(PWD)/obj/normal
AUTOCONF_DIR    ?= ../../buildscript/include

-include $(AUTOCONF_DIR)/config/snx_sdk.conf

.PHONY: all menuconfig clean distclean install

all: $(OBJ_DIR)/.config
	$(Q)$(MAKE) CROSS_COMPILE=$(CROSS_COMPILE) CONFIG_PREFIX="$(INSTALL_DIR)" -C $(OBJ_DIR)

$(OBJ_DIR)/.config:
	$(Q)if [ -z "$(AUTOCONF_DIR)" ]; then \
		echo "Error! Please set 'AUTOCONF' directory by 'AUTOCONF_DIR' variable"; \
		exit 1; \
	else \
		if [ ! -e "$(AUTOCONF_DIR)/config/snx_sdk.conf" ]; then \
			echo "Error! include/config/snx_sdk.conf is missing."; \
			exit 2; \
		fi; \
	fi;
	@ if ! [ -d $(OBJ_DIR) ]; then \
		mkdir -p $(OBJ_DIR);\
	fi 
ifeq ($(COMPILE_RESCUE), yes)
		$(Q)$(MAKE) CROSS_COMPILE=$(CROSS_COMPILE) -C $(SRC_DIR) O=$(OBJ_DIR) sn986xx_rescue_defconfig
else
		$(Q)$(MAKE) CROSS_COMPILE=$(CROSS_COMPILE) -C $(SRC_DIR) O=$(OBJ_DIR) $(CONFIG_SYSTEM_SERIES)_defconfig
endif

menuconfig: $(OBJ_DIR)/.config
	$(Q)$(MAKE) -C $(OBJ_DIR) $@

clean:
	@ if [ -d $(OBJ_DIR) ]; then \
		$(MAKE) -C $(OBJ_DIR) $@; \
		$(MAKE) -C $(OBJ_DIR) $@; \
	fi 



distclean:
	@ if [ -d $(OBJ_DIR) ]; then \
		$(MAKE) -C $(OBJ_DIR) $@; \
		rm -rf $(OBJ_DIR); \
	fi

install:
	@ if ! [ -d $(INSTALL_DIR) ]; then \
		install -d $(INSTALL_DIR); \
	fi
	$(Q)$(MAKE) CONFIG_PREFIX="$(INSTALL_DIR)" -C $(OBJ_DIR) $@

