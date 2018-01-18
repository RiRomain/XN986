PWD		:= $(shell pwd)
ARCH		?= arm
CROSS_COMPILE	?= $(ARCH)-linux-
vKERNEL		?= 2.6.35.12
CPP     	:= $(CROSS_COMPILE)g++
CC      	:= $(CROSS_COMPILE)gcc
AR      	:= $(CROSS_COMPILE)ar
RANLIB      := $(CROSS_COMPILE)ranlib

MIDDLEWARE_INS_DIR ?= ../../../../middleware/_install
KERNEL_DIR  ?= ../../../../kernel/linux-2.6.35.12
AUTOCONF_DIR    ?= ../../../../buildscript/include
LIBGPIO_DIR             := $(AUTOCONF_DIR)/generated

INC     	= -I $(KERNEL_DIR)/src/include 
INC     	+= -I$(MIDDLEWARE_INS_DIR)/include
INC 		+= -I$(LIBGPIO_DIR)

CFLAGS      := -Wall -g -O2 $(INC)
LDFLAGS     := $(EXTRA_LDFLAGS) -Wl,--as-needed

INSTALL_DIR	?= ../rootfs/usr/bin
BIN_INS_DIR	:= $(INSTALL_DIR)/bin/example
LIB_INS_DIR	:= $(INSTALL_DIR)/lib/example
CATEGORY    := uart_ctl

SNX_GPIO_LIB = $(MIDDLEWARE_INS_DIR)/lib/libgpio.so

targets = snx_uart_ctl
targets += snx_uart_gpio_ctl

.PHONY : clean distclean all
%.o : %.c
	$(CC) $(CFLAGS) -c $<

all: $(targets)

snx_uart_ctl: uart_ctl.o $(SNX_GPIO_LIB)
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS) 

snx_uart_gpio_ctl: snx_uart_gpio_ctl.o 
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

