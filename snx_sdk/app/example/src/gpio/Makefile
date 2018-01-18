PWD     	:= $(shell pwd)
TARGET      ?= arm-linux
CROSS_COMPILE   ?= $(TARGET)-
CPP     	:= $(CROSS_COMPILE)g++
CC      	:= $(CROSS_COMPILE)gcc
AR      	:= $(CROSS_COMPILE)ar
RANLIB      := $(CROSS_COMPILE)ranlib

INSTALL_DIR	?= ../rootfs/usr
BIN_INS_DIR	:= $(INSTALL_DIR)/bin/example
LIB_INS_DIR	:= $(INSTALL_DIR)/lib/example
CATEGORY	:= gpio

MIDDLEWARE_INS_DIR ?= ../../../../middleware/_install
KERNEL_DIR  ?= ../../../../kernel/linux-2.6.35.12
AUTOCONF_DIR    ?= ../../../../buildscript/include
LIBGPIO_DIR             := $(AUTOCONF_DIR)/generated

INC     	= -I $(KERNEL_DIR)/src/include
INC		+= -I $(MIDDLEWARE_INS_DIR)/include
INC			+= -I $(PWD)/include
INC 		+= -I$(LIBGPIO_DIR)
LIB		= -L $(MIDDLEWARE_INS_DIR)/lib
CFLAGS      := -Wall -g -O2 $(EXTRA_CFLAGS) $(INC) $(LIB)
LDFLAGS     := $(EXTRA_LDFLAGS) -Wl,--as-needed
LDFLAGS	+= -lgpio


targets = snx_gpio_ctl

targets += snx_gpio_test

.PHONY : clean distclean all
%.o : %.c
	$(CC) $(CFLAGS) -c $<

all: $(targets)


snx_gpio_ctl: snx_gpio_ctl.o 
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

snx_gpio_test: snx_gpio_test.o 
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

	
.PHONY: install
install:
	@ if [ ! -d $(BIN_INS_DIR)/$(CATEGORY) ]; \
	then \
	install -d $(BIN_INS_DIR)/$(CATEGORY); \
	fi
	install -c $(targets) $(BIN_INS_DIR)/$(CATEGORY)

clean:
	rm -f *.o *.yuv *.bak *.a *.out *.so $(targets)

distclean : clean
	rm -f $(targets)


#INSTALL_DIR	?= ../rootfs/usr
#BIN_INS_DIR	:= $(INSTALL_DIR)/bin/example
#LIB_INS_DIR	:= $(INSTALL_DIR)/lib/example
#
#targets = snx_gpio_test
#GPIO	:= gpio
#all:
#	arm-linux-gcc -o snx_gpio_test snx_gpio_test.c
#
#.PHONY: clean
#clean:
#	-rm -rf *.o  snx_gpio_test
#
#.PHONY: install
#install:
#	@ if [ ! -d $(BIN_INS_DIR)/$(GPIO) ]; \
#	then \
#		install -d $(BIN_INS_DIR)/$(GPIO); \
#	fi
#	install -c $(targets) $(BIN_INS_DIR)/$(GPIO)

