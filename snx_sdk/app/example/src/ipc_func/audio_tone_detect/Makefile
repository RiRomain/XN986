PWD     	:= $(shell pwd)
TARGET		?= arm-linux
CROSS_COMPILE   ?= $(TARGET)-
CC      	:= $(CROSS_COMPILE)g++
STRIP       := $(CROSS_COMPILE)strip
BIN_DIR		= $(INSTALL_DIR)/bin
INSTALL_DIR ?= ../rootfs/usr
INSTALL		?= install
CATEGORY    := ipc_func/rtsp_server

MIDDLEWARE_INS_DIR ?= ../../../../../middleware/_install
AUTOCONF_DIR	?= ../../../../../buildscript/include

EXTRA_CFLAGS    += -I$(AUTOCONF_DIR)

PCM_DIR		= $(INSTALL_DIR)/../root/etc_default/notify/
PCM_FILE 	= $(PWD)/notify/*

CFLAGS		= -fvisibility=hidden 
CFLAGS		+= -I$(PWD) -I$(MIDDLEWARE_INS_DIR)/include
CFLAGS		+= $(APP_FLAGS)


LDFLAGS		= -Wl,--no-export-dynamic -L$(MIDDLEWARE_INS_DIR)/lib
LDFLAGS		+= -ltone -lasound -lssl -lcrypto

######################################################

PCM_PLAY_OBJ    = pcm_play.o
TARGET := TDMain

all: $(TARGET)

TDMain:	 $(PCM_PLAY_OBJ) arec.o
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

TDMain_test: arec_test.o $(PCM_PLAY_OBJ)
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS)  $<

clean:
	rm -f *.o $(TARGET)
	#rm -f TDMain_test
install:
	$(INSTALL) -d $(BIN_DIR)
	$(INSTALL) $(TARGET) $(BIN_DIR)
	$(INSTALL) -d $(PCM_DIR)
	cp $(PCM_FILE) $(PCM_DIR)


