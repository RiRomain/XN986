PWD		:= $(shell pwd)
TARGET		?= arm-linux
CROSS_COMPILE	?= $(TARGET)-
CPP		:= $(CROSS_COMPILE)g++
CC		:= $(CROSS_COMPILE)gcc
CFLAGS 	:= -c -O2 -Wall
INSTALL_DIR	?= ../rootfs/usr
BIN_INS_DIR	:= $(INSTALL_DIR)/bin/example
LIB_INS_DIR	:= $(INSTALL_DIR)/lib/example
VIDEO_OUTPUT	:= video_output
MIDDLEWARE_INS_DIR := ../../../../middleware/_install/
#INC 	= -I ./ -L $(MIDDLEWARE_INS_DIR)/lib -I $(MIDDLEWARE_INS_DIR)/include -lmtable -DUSE_M_TABLE -lsnxconf
INC 	= -I ./ -L $(MIDDLEWARE_INS_DIR)/lib -I $(MIDDLEWARE_INS_DIR)/include -lmtable -lsnxconf
JTHREAD_DIR	:= ./jthread/pthread/

export CROSS_COMPILE

APP_OBJS	= snx_capture.o snx_output.o snx_codec.o
targets = jthread_lib $(APP_OBJS) snx_videoout_test snx_fb_test

.PHONY: all
all: $(targets)

snx_videoout_test: snx_tv.cpp
	$(CPP) $(INC) -O2 -Wall -lpthread $< $(APP_OBJS) libjthread.a -o $@
	
jthread_lib:
	make -C $(JTHREAD_DIR)
	cp $(JTHREAD_DIR)/lib/libjthread.a ./

$(APP_OBJS): %.o: %.cpp
	$(CPP) $(INC)  $(CFLAGS) $< -o $@	

snx_fb_test: snx_fb_test.o
	$(CC) $^ -o $@

.PHONY: clean
clean:
	-rm -rf $(targets) *.o *.a
	make -C $(JTHREAD_DIR) clean

.PHONY: install
install:
	@ if [ ! -d $(BIN_INS_DIR)/$(VIDEO_OUTPUT) ]; \
	then \
	install -d $(BIN_INS_DIR)/$(VIDEO_OUTPUT); \
	fi
	install -c snx_videoout_test snx_fb_test $(BIN_INS_DIR)/$(VIDEO_OUTPUT)
	

