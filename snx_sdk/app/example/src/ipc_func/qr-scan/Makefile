PWD     	:= $(shell pwd)
TARGET		?= arm-linux
CROSS_COMPILE   ?= $(TARGET)-
CPP     	:= $(CROSS_COMPILE)g++
CC      	:= $(CROSS_COMPILE)gcc
AR      	:= $(CROSS_COMPILE)ar
RANLIB		:= $(CROSS_COMPILE)ranlib

MIDDLEWARE_INS_DIR ?= ../../../../../middleware/_install
AUTOCONF_DIR	?= ../../../../../buildscript/include
EXTRA_CFLAGS    += -I$(AUTOCONF_DIR)

CFLAGS		= -O2  -Werror $(EXTRA_CFLAGS)

CFLAGS 		+= -I$(MIDDLEWARE_INS_DIR)/include
CFLAGS		+= -I$(MIDDLEWARE_INS_DIR)/include/snx_vc
CFLAGS		+= -I$(MIDDLEWARE_INS_DIR)/include/snx_rc

LDFLAGS		+= -L$(MIDDLEWARE_INS_DIR)/lib
LDFLAGS		+= -lasound -lzbar -lsnx_vc -lsnx_rc -lgpio -lsnx_isp 

BIN_DIR		= $(INSTALL_DIR)/bin/example/ipc_func/qr_scan
SNX_EZ_DIR	= $(INSTALL_DIR)/root/etc_default/$(CONF_DIR)
PCM_DIR		= $(INSTALL_DIR)/../root/etc_default/notify/
PCM_FILE 	= $(PWD)/notify/*
DEF_DIR		= files


INSTALL		= install
PCM_PLAY_OBJ    = pcm_play.o
SNX_SCAN_OBJS	= snx_scan.o
targets = snx_scan

all: $(targets)

snx_scan:  $(SNX_SCAN_OBJS) $(PCM_PLAY_OBJ)
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS)  $<

clean:
	rm -f *.o $(targets)

install:
	$(INSTALL) -d $(BIN_DIR)
	$(INSTALL) snx_scan $(BIN_DIR)
	$(INSTALL) -d $(PCM_DIR)
	cp $(PCM_FILE) $(PCM_DIR)


