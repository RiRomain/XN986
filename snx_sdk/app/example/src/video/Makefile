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
SNIP39_DIR  := $(INSTALL_DIR)/../root/etc_default/EXAMPLE
CATEGORY	:= video
MIDDLEWARE_INS_DIR ?= ../../../../middleware/_install
KERNEL_DIR	?= ../../../../kernel/linux-2.6.35.12
AUTOCONF_DIR    ?= ../../../../buildscript/include

-include $(AUTOCONF_DIR)/config/snx_sdk.conf

INC     	= -I $(KERNEL_DIR)/src/include
INC     	+= -I$(MIDDLEWARE_INS_DIR)
INC     	+= -I$(MIDDLEWARE_INS_DIR)/include/snx_vc
INC			+= -I$(MIDDLEWARE_INS_DIR)/include/snx_rc
INC			+= -I$(MIDDLEWARE_INS_DIR)/include/snx_record	
INC			+= -I $(PWD)/include
EXTRA_CFLAGS	+= -I$(AUTOCONF_DIR)
CFLAGS		:= -Wall -g -O2 $(EXTRA_CFLAGS) $(INC)
#CFLAGS		+= -Werror	
LDFLAGS     := $(EXTRA_LDFLAGS) -Wl,--as-needed
LDFLAGS		+= -L$(MIDDLEWARE_INS_DIR)/lib		
LDFLAGS 	+= -pthread -ldl
YUV2H264_LDFLAGS		= -lsnx_record 
YUV2H264_LDFLAGS		+= -lavformat  -lavcodec -lavutil -lxml -lz

SNX_VC_LIB = $(MIDDLEWARE_INS_DIR)/lib/libsnx_vc.so
SNX_RC_LIB = $(MIDDLEWARE_INS_DIR)/lib/libsnx_rc.so
SNX_RC_LIB += $(MIDDLEWARE_INS_DIR)/lib/libsnx_isp.so 

SNX_VIDEO_CODEC = snx_video_codec.o

targets = snx_m2m_one_stream snx_m2m_two_stream snx_m2m_capture_2stream snx_m2m_capture_4stream
targets += snx_m2m_one_stream_with_rc snx_m2m_capture_2stream_with_rc
targets += snx_m2m_infile_stream_with_rc
targets += snx_vc_ds
targets += snx_m2m_one_stream_yuv
targets += snx_m2m_dynamic_fps_bps
targets += snx_rc_ctl
targets += snx_yuv2h264
targets += snx_uvc_record 

ifeq ($(CONFIG_SYSTEM_SERIES),"sn986xx")
targets += snx_dec2yuv420line
targets += snx_dec2videoout
targets += sonix_edr_demo 
targets += snx_edr
endif

ifeq ($(CONFIG_SYSTEM_SERIES),"sn9866x")
targets += snx_rc_mroi
endif


.PHONY : clean distclean all
%.o : %.c
	$(CC) $(CFLAGS) -c $<

all: $(targets)

$(SNX_VIDEO_CODEC): vc/snx_video_codec.c
	$(CC) $(CFLAGS) -c -o $@ $<

snx_m2m_one_stream: snx_m2m_one_stream.o $(SNX_VIDEO_CODEC) $(SNX_VC_LIB) $(SNX_RC_LIB)
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

snx_m2m_infile_stream_with_rc: snx_m2m_infile_stream_with_rc.o $(SNX_VIDEO_CODEC) $(SNX_VC_LIB) $(SNX_RC_LIB)
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)


snx_m2m_two_stream: snx_m2m_two_stream.o $(SNX_VIDEO_CODEC) $(SNX_VC_LIB) $(SNX_RC_LIB)
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

snx_m2m_capture_2stream: snx_m2m_capture_2stream.o $(SNX_VIDEO_CODEC) $(SNX_VC_LIB) $(SNX_RC_LIB)
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

snx_m2m_capture_4stream: snx_m2m_capture_4stream.o $(SNX_VIDEO_CODEC) $(SNX_VC_LIB) $(SNX_RC_LIB)
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

snx_m2m_one_stream_with_rc: snx_m2m_one_stream_with_rc.o $(SNX_VIDEO_CODEC) $(SNX_VC_LIB) $(SNX_RC_LIB)
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

snx_m2m_dynamic_fps_bps: snx_m2m_dynamic_fps_bps.o $(SNX_VIDEO_CODEC) $(SNX_VC_LIB) $(SNX_RC_LIB)
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

snx_m2m_capture_2stream_with_rc: snx_m2m_capture_2stream_with_rc.o $(SNX_VIDEO_CODEC) $(SNX_VC_LIB) $(SNX_RC_LIB)
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

snx_vc_ds: snx_vc_ds.o $(SNX_VIDEO_CODEC) $(SNX_VC_LIB) $(SNX_RC_LIB)
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

snx_m2m_one_stream_yuv: snx_m2m_one_stream_yuv.o $(SNX_VIDEO_CODEC) $(SNX_VC_LIB) $(SNX_RC_LIB)
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

snx_rc_ctl: snx_rc_ctl.o $(SNX_VIDEO_CODEC) $(SNX_VC_LIB) $(SNX_RC_LIB)
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS) -Werror
	
snx_rc_mroi: snx_rc_mroi.o $(SNX_VIDEO_CODEC) $(SNX_VC_LIB) $(SNX_RC_LIB)
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

snx_dec2yuv420line: snx_dec2yuv420line.o  $(SNX_VC_LIB) $(SNX_RC_LIB)
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS) -lm 
  
snx_yuv2h264: snx_yuv2h264.o  $(SNX_VC_LIB) $(SNX_RC_LIB)
	$(CPP) -o $@ $^ -rdynamic $(LDFLAGS) $(YUV2H264_LDFLAGS) -lm 
	#$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS) $(YUV2H264_LDFLAGS) -lm 

snx_dec2videoout: snx_dec2videoout.o  $(SNX_VC_LIB) $(SNX_RC_LIB)
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS) -lm       

sonix_edr_demo: sonix_edr_demo.o  $(SNX_VC_LIB) $(SNX_RC_LIB)
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS) -lm       

snx_edr: snx_edr.o  $(SNX_VC_LIB) $(SNX_RC_LIB)
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS) -lm  

snx_uvc_record: snx_uvc_record.o  $(SNX_VC_LIB) $(SNX_RC_LIB)
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS) -lm  	
	
.PHONY: install
install:
	@ if [ ! -d $(BIN_INS_DIR)/$(CATEGORY) ]; \
	then \
	install -d $(BIN_INS_DIR)/$(CATEGORY); \
	fi
	@ if [ ! -d $(SNIP39_DIR) ]; \
	then \
	install -d $(SNIP39_DIR); \
	fi
	install -c $(targets) $(BIN_INS_DIR)/$(CATEGORY)
	install files/yuv2h264.xml $(SNIP39_DIR)
clean:
	rm -f *.o  *.bak *.a *.out *.so $(targets)

distclean : clean
	rm -f $(targets)
