MAKE            ?= make
PWD             := $(shell pwd)
ARCH		?= arm
TARGET		?= $(ARCH)-linux
CROSS_COMPILE	?= $(TARGET)-
INSTALL_DIR	?= $(PWD)/rootfs
BIN_INSTALL_DIR	:= $(INSTALL_DIR)/usr

SRC_DIR		?=
SRC_DIR		+= $(PWD)/src/video
SRC_DIR		+= $(PWD)/src/audio
SRC_DIR		+= $(PWD)/src/crypto
SRC_DIR		+= $(PWD)/src/spi

SRC_DIR		+= $(PWD)/src/pwm
SRC_DIR		+= $(PWD)/src/gpio
SRC_DIR		+= $(PWD)/src/watchdog
SRC_DIR		+= $(PWD)/src/video_output
SRC_DIR		+= $(PWD)/src/i2c
SRC_DIR		+= $(PWD)/src/timer
SRC_DIR		+= $(PWD)/src/rtc
SRC_DIR		+= $(PWD)/src/isp_ctl
SRC_DIR		+= $(PWD)/src/ir_ctl
SRC_DIR		+= $(PWD)/src/uart_ctl
SRC_DIR     += $(PWD)/src/ipc_func/audio_tone_detect
SRC_DIR     += $(PWD)/src/ipc_func/qr-scan
SRC_DIR     += $(PWD)/src/ipc_func/record
SRC_DIR     += $(PWD)/src/ipc_func/rtsp_server
SRC_DIR     += $(PWD)/src/ipc_func/snapshot
SRC_DIR     += $(PWD)/src/ipc_func/webserver

.PHONY: all clean distclean install

all:
	for i in $(SRC_DIR);	\
	do						\
		$(MAKE) -C $$i $@||exit 1;	\
	done

clean:
	for i in $(SRC_DIR);	\
	do						\
		$(MAKE) -C $$i $@||exit 1;	\
	done

install:
	for i in $(SRC_DIR);	\
	do						\
		$(MAKE) -C $$i $@ INSTALL_DIR=$(abspath $(BIN_INSTALL_DIR)) || exit 1; \
	done

