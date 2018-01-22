



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
CATEGORY	:= ipc_func/snapshot

KERNEL_DIR	?= ../../../../../kernel/linux-2.6.35.12
MIDDLEWARE_INS_DIR ?= ../../../../../middleware/_install
AUTOCONF_DIR	?= ../../../../../buildscript/include

INC     	= -I $(KERNEL_DIR)/src/include
INC     	+= -I$(MIDDLEWARE_INS_DIR)
INC			+= -I $(PWD)/include
INC			+= -I$(MIDDLEWARE_INS_DIR)/include/snx_rc

EXTRA_CFLAGS	+= -I$(AUTOCONF_DIR)
CFLAGS      := -Wall -g -O2 $(EXTRA_CFLAGS) $(INC)
LDFLAGS     := $(EXTRA_LDFLAGS) -Wl,--as-needed
LDFLAGS 	+= -pthread

SNX_VC_LIB = $(MIDDLEWARE_INS_DIR)/lib/libsnx_vc.so
SNX_RC_LIB = $(MIDDLEWARE_INS_DIR)/lib/libsnx_rc.so
SNX_RC_LIB += $(MIDDLEWARE_INS_DIR)/lib/libsnx_isp.so 
#SNX_RC_LIB += $(MIDDLEWARE_INS_DIR)/lib/libsnx_common.so

SNX_VIDEO_CODEC = snx_video_codec.o

targets = snx_snapshot 

.PHONY : clean distclean all
%.o : %.c
	$(CC) $(CFLAGS) -c $<

all: $(targets)


#$(SNX_VIDEO_CODEC): snx_video_codec.c
#	$(CC) $(CFLAGS) -c -o $@ $<

snx_snapshot: snx_m2m_one_stream.o $(SNX_VIDEO_CODEC) $(SNX_VC_LIB) $(SNX_RC_LIB)
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
