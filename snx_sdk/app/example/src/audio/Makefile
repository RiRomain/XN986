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
CATEGORY	:= audio

KERNEL_DIR	?= ../../../../kernel/linux-2.6.35.12
MIDDLEWARE_INS_DIR ?= ../../../../middleware/_install
AUTOCONF_DIR	?= ../../../../buildscript/include

INC     	= -I $(KERNEL_DIR)/src/include
INC			+= -I$(MIDDLEWARE_INS_DIR)/include
INC			+= -I$(MIDDLEWARE_INS_DIR)/include/opus
INC     	+= -I$(MIDDLEWARE_INS_DIR)
INC			+= -I $(PWD)/include
CFLAGS      := -Wall -g -O2 $(EXTRA_CFLAGS) $(INC)
LDFLAGS     := $(EXTRA_LDFLAGS) -Wl,--as-needed
LDFLAGS 	+= -pthread -lm

SNX_LIB = $(MIDDLEWARE_INS_DIR)/lib/libasound.so $(MIDDLEWARE_INS_DIR)/lib/libsnx_audio.so
SNX_AUDIO32_LIB = $(MIDDLEWARE_INS_DIR)/lib/libsnx_audio_aud32.so
SNX_OPUS_LIB = $(MIDDLEWARE_INS_DIR)/lib/libopus.so
SNX_AUDIO_CODEC = snx_audio_codec.o

targets = snx_audio_info snx_audio_record snx_audio_playback snx_audio_record_2stream 
targets += snx_audio_vol_ctl snx_audio_sigma_vol_ctrl snx_audio32 snx_opus_encode snx_opus_decode
targets += snx_aac_encode snx_aac_decode

.PHONY : clean distclean all
%.o : %.c
	$(CC) $(CFLAGS) -c $<

all: $(targets)

$(SNX_AUDIO_CODEC): ac/snx_audio_codec.c
	$(CC) $(CFLAGS) -c -o $@ $<

snx_audio_info: snx_audio_info.o $(SNX_LIB)
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

snx_audio_record: snx_audio_record.o $(SNX_LIB) $(SNX_AUDIO_CODEC)
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

snx_audio_playback: snx_audio_playback.o $(SNX_LIB) $(SNX_AUDIO_CODEC)
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

snx_audio_record_2stream: snx_audio_record_2stream.o $(SNX_LIB) $(SNX_AUDIO_CODEC)
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

snx_audio_vol_ctl: snx_audio_vol_ctl.o $(SNX_LIB) $(SNX_AUDIO_CODEC)
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)
	
snx_audio_sigma_vol_ctrl: snx_audio_sigma_vol_ctrl.o $(SNX_LIB) $(SNX_AUDIO_CODEC)
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

snx_audio32: snx_audio32.o $(SNX_LIB) $(SNX_AUDIO32_LIB)
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

snx_opus_encode: opus_encode.o $(SNX_LIB) $(SNX_OPUS_LIB)
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

snx_opus_decode: opus_decode.o $(SNX_LIB) $(SNX_OPUS_LIB)
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

snx_aac_encode: snx_aac_encode.o
	$(CC) $(CFLAGS) -o $@ $^ -L$(MIDDLEWARE_INS_DIR)/lib/ -lasound -lfdk-aac

snx_aac_decode: snx_aac_decode.o
	$(CC) $(CFLAGS) -o $@ $^ -L$(MIDDLEWARE_INS_DIR)/lib/ -lasound -lfdk-aac

.PHONY: install
install:
	@ if [ ! -d $(BIN_INS_DIR)/$(CATEGORY) ]; \
	then \
	install -d $(BIN_INS_DIR)/$(CATEGORY); \
	fi
	install -c $(targets) $(BIN_INS_DIR)/$(CATEGORY)
	install -c Nokia.raw $(BIN_INS_DIR)/$(CATEGORY)
	install -c play.sh $(BIN_INS_DIR)/$(CATEGORY)
	install -c record.sh $(BIN_INS_DIR)/$(CATEGORY)

clean:
	rm -f *.o *.yuv *.bak *.a *.out *.so $(targets)

distclean : clean
	rm -f $(targets)
