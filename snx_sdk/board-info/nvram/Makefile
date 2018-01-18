#
# Make Related Variables 
#
MAKE            ?= make
PWD             := $(shell pwd)
PERL		:= perl
NVRAMPARSE	:= parse_nvram.pl
AUTOCONF_DIR	?= $(srctree)/include
ORIGINALFILE	:= ori_file.txt

#
#	include
#
# Read in config
-include $(AUTOCONF_DIR)/config/snx_sdk.conf
FLASH           ?= $(if $(CONFIG_SYSTEM_NAND_FLASH_SUPPORT),nand,$(if $(CONFIG_SYSTEM_SERIAL_FLASH_SUPPORT),sf,nand))
MACHINE         ?= $(CONFIG_SYSTEM_PLATFORM)

include $(srctree)/Makefile.version

#
# target
#
.PHONY: nvram
nvram:
	$(PERL) $(NVRAMPARSE)	\
		orifile=$(ORIGINALFILE) \
		sdk_version="$(sdk_version)" \

PHONY += clean
clean:
	rm -rf ./nvram.h ./nvram.bin
