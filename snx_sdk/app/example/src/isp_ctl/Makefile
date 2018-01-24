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
CATEGORY	:= isp_ctl

KERNEL_DIR	?= ../../../../kernel/linux-2.6.35.12
MIDDLEWARE_INS_DIR ?= ../../../../middleware/_install
AUTOCONF_DIR	?= ../../../../buildscript/include

EXTRA_LDFLAGS	=
EXTRA_CFLAGS	+= -I$(AUTOCONF_DIR)

INC    = -I $(KERNEL_DIR)/src/include
INC		+= -I $(MIDDLEWARE_INS_DIR)/include
INC		+= -I $(PWD)/include
INC		+= -I HALLO

LIB		 = -L $(MIDDLEWARE_INS_DIR)/lib
LIB   += -L ../../../../middleware/video/middleware/include/
CFLAGS      := -Wall -g -O2 $(EXTRA_CFLAGS) $(INC) $(LIB)
LDFLAGS	:= -lsnx_isp -lsnx_vc -lpthread
LDFLAGS     += $(EXTRA_LDFLAGS) -Wl,--as-needed


targets = snx_isp_ctl snx_isp_osd_unicode snx_isp_osd_asccii snx_isp_cap snx_isp_ctrls snx_isp_osd snx_isp_md snx_isp_pm

.PHONY : clean distclean all
%.o : %.c
	$(CC) $(CFLAGS) -c $<

all: $(targets)


snx_isp_ctl: snx_isp_ctl.o
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

snx_isp_osd_unicode: snx_isp_osd_unicode.o
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

snx_isp_osd_asccii: snx_isp_osd_asccii.o
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

snx_isp_cap:snx_isp_cap.o
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

snx_isp_ctrls:snx_isp_ctrls.o
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

snx_isp_osd:snx_isp_osd.o
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

snx_isp_md:snx_isp_md.o
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

snx_isp_pm:snx_isp_pm.o
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
