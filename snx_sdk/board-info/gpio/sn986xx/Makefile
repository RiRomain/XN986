#
# Make Related Variables 
#
PERL		:= perl
GPIOPARSE	:= parse_gpio.pl
AUTOCONF_DIR	?= ../../buildscript/include

#
#	include
#
# Read in config
-include $(AUTOCONF_DIR)/config/snx_sdk.conf
FLASH           ?= $(if $(CONFIG_SYSTEM_NAND_FLASH_SUPPORT),nand,$(if $(CONFIG_SYSTEM_SERIAL_FLASH_SUPPORT),sf,nand))
MACHINE         ?= $(CONFIG_SYSTEM_PLATFORM)

#
# target
#
.PHONY: snx_gpio_conf.h
snx_gpio_conf.h:
	$(PERL) $(GPIOPARSE) \
	platform=$(MACHINE) \
	flash_type=$(FLASH)
	
PHONY += clean
clean:
	rm -rf ./snx_gpio_conf.h
