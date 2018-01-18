PWD     	:= $(shell pwd)
TARGET      ?= arm-linux
CROSS_COMPILE   ?= $(TARGET)-
CPP     	:= $(CROSS_COMPILE)g++
CC      	:= $(CROSS_COMPILE)gcc
AR      	:= $(CROSS_COMPILE)ar
RANLIB      := $(CROSS_COMPILE)ranlib
INSTALL_DIR	?= ../rootfs/usr/bin

INC     	= -I $(KERNEL_DIR)/src/include
INC			+= -I $(MIDDLEWARE_INS_DIR)/include
INC			+= -I $(AUTOCONF_DIR)

LIB			= -L $(MIDDLEWARE_INS_DIR)/lib

CFLAGS      := -Wall -g -O2 $(EXTRA_CFLAGS) $(INC) $(LIB)

LDFLAGS		:= -lsnx_isp
LDFLAGS		+= $(EXTRA_LDFLAGS) -Wl,--as-needed

targets = snx_mcu2511_ctrl
targets += snx_mcu2711_wdt_ctrl
targets += snx_mcu2711_ir_ctrl

.PHONY : clean distclean all
%.o : %.c
	$(CC) $(CFLAGS) -c $<

all: $(targets)

snx_mcu2511_ctrl: snx_mcu2511_ctrl.o 
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

snx_mcu2711_wdt_ctrl: snx_mcu2711_wdt_ctrl.o 
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)	

snx_mcu2711_ir_ctrl: snx_mcu2711_ir_ctrl.o 
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)	
	
.PHONY: install
install:
	@ if [ ! -d $(INSTALL_DIR) ]; \
	then \
	install -d $(INSTALL_DIR); \
	fi
	install -c $(targets) $(INSTALL_DIR)

clean:
	rm -f *.o *.yuv *.bak *.a *.out *.so $(targets)

distclean : clean
	rm -f $(targets)
