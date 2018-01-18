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

LIBGPIO_DIR		:= $(AUTOCONF_DIR)/generated

INC     	= -I $(KERNEL_DIR)/src/include 
INC     	+= -I$(MIDDLEWARE_INS_DIR)/include
INC 		+= -I$(LIBGPIO_DIR)
CFLAGS      := -Wall -g -O2 $(INC)
LDFLAGS     := $(EXTRA_LDFLAGS) -Wl,--as-needed

INSTALL_DIR	?= ../rootfs/usr
BIN_INS_DIR	:= $(INSTALL_DIR)/bin/example
LIB_INS_DIR	:= $(INSTALL_DIR)/lib/example


LDFLAGS		:= -L$(MIDDLEWARE_INS_DIR)/lib
LDFLAGS 		+= -lgpio


targets = snx_pwm_period_ctl
#targets += snx_pwm_ap_test
pwm_gpio = snx_pwm_gpio_ctl
PWM	:= pwm

all: $(pwm_gpio)
	arm-linux-gcc -o snx_pwm_period_ctl snx_pwm_period_ctl.c
#	arm-linux-gcc -o snx_pwm_ap_test snx_pwm_ap_test.c	

$(pwm_gpio): $(LIBGPIO_DIR)/snx_gpio_conf.h
	$(CC)  $(pwm_gpio).c -o $(pwm_gpio) $(CFLAGS) $(LDFLAGS)

.PHONY: clean
clean:
	-rm -rf *.o  snx_pwm_period_ctl snx_pwm_ap_test $(pwm_gpio)

.PHONY: install
install:
	@ if [ ! -d $(BIN_INS_DIR)/$(PWM) ]; \
	then \
		install -d $(BIN_INS_DIR)/$(PWM); \
	fi
	install -c $(targets) $(BIN_INS_DIR)/$(PWM)

