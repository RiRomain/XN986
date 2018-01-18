NAME = snx_rtsp_server

PWD     	:= $(shell pwd)
TARGET      ?= arm-linux
CROSS_COMPILE   ?= $(TARGET)-
CPP     	:= $(CROSS_COMPILE)g++
CC      	:= $(CROSS_COMPILE)g++
AR      	:= $(CROSS_COMPILE)ar
RANLIB      := $(CROSS_COMPILE)ranlib
STRIP 		:= $(CROSS_COMPILE)strip
INSTALL_DIR	?= ../rootfs/usr
BIN_INS_DIR	:= $(INSTALL_DIR)/bin/example
LIB_INS_DIR	:= $(INSTALL_DIR)/lib/example
CATEGORY	:= ipc_func/playback

KERNEL_DIR	?= ../../../../../kernel/linux-2.6.35.12
MIDDLEWARE_INS_DIR ?= ../../../../../middleware/_install
AUTOCONF_DIR	?= ../../../../../buildscript/include

INC     	= -I$(KERNEL_DIR)/src/include
INC     	+= -I$(MIDDLEWARE_INS_DIR)
INC     	+= -I$(MIDDLEWARE_INS_DIR)/include/snx_vc
INC			+= -I$(MIDDLEWARE_INS_DIR)/include/snx_rc
INC			+= -I$(MIDDLEWARE_INS_DIR)/include/snx_isp
INC			+= -I$(MIDDLEWARE_INS_DIR)/include/snx_record
INC			+= -I$(MIDDLEWARE_INS_DIR)/include
INC 		+= -I$(PWD)/include

## toolchain
EXTRA_LDFLAGS	= 
EXTRA_CFLAGS	+= -I$(AUTOCONF_DIR)
EXTRA_LDFLAGS   += -Wl,-Bstatic -Wl,-Bdynamic

CFLAGS      := -Wall -O2 $(EXTRA_CFLAGS) $(INC) -DSOCKLEN_T=socklen_t -DNO_SSTREAM=1 -D_LARGEFILE_SOURCE=1  -D_FILE_OFFSET_BITS=64 -static
#CFLAGS		+= -DDEBUG=1
CFLAGS 		+= -DVIDEO_PRE_BUFFER=1

LDFLAGS     := $(EXTRA_LDFLAGS) -Wl,--as-needed -lpthread
LDFLAGS 	+= -Wl,--no-export-dynamic -Wl,--gc-sections -Wl,--print-gc-sections $(EXTRA_LDFLAGS)



LDFLAGS		+= -lm -ldl
LDFLAGS		+= -L$(MIDDLEWARE_INS_DIR)/lib
LDFLAGS		+= -lsnx_record
LDFLAGS		+= -lavformat  -lavcodec -lavutil 
LDFLAGS		+= -lxml
LDFLAGS		+= -lz

SNX_VC_LIB = $(MIDDLEWARE_INS_DIR)/lib/libsnx_vc.so
#SNX_RC_LIB = $(MIDDLEWARE_INS_DIR)/lib/libsnx_rc.so
#SNX_RC_LIB += $(MIDDLEWARE_INS_DIR)/lib/libsnx_isp.so 
SNX_AUDIO_LIB = $(MIDDLEWARE_INS_DIR)/lib/libasound.so $(MIDDLEWARE_INS_DIR)/lib/libsnx_audio.so


SRC_OBJ	= snx_playback.o
SRC_OBJ	+= sn986_play.o
SRC_OBJ	+= data_buf.o
SRC_OBJ	+= snx_audio_codec.o

#targets =
targets = snx_avplay


.PHONY : clean distclean all
%.o : %.cpp
	$(CPP) -c $(CFLAGS) $<

%.o : %.c
	$(CC) -c $(CFLAGS) $<

all: $(targets)

snx_avplay:  $(SRC_OBJ) $(SNX_VC_LIB) $(SNX_RC_LIB) $(SNX_AUDIO_LIB)
	$(CPP) -o $@ $^ -rdynamic $(LDFLAGS)
	$(STRIP) $@


.PHONY: install
install:
	@ if [ ! -d $(BIN_INS_DIR)/$(CATEGORY) ]; \
	then \
	install -d $(BIN_INS_DIR)/$(CATEGORY); \
	fi
	install -c $(NAME) $(BIN_INS_DIR)/$(CATEGORY)

clean:
	rm -f *.o *.yuv *.bak *.a *.out *.so *.d $(targets)

distclean : clean
	rm -f $(targets)
