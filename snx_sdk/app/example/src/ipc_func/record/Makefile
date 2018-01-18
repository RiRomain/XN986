NAME = snx_record



PWD		:= $(shell pwd)
LIB_DIR = $(PWD)/lib
TARGET      	?= arm-linux
CROSS_COMPILE   ?= $(TARGET)-

## toolchain
EXTRA_CPPFLAGS	=
EXTRA_LDFLAGS	= 
EXTRA_LDFLAGS	+= -Wl,-Bstatic -Wl,-Bdynamic

CC = $(CROSS_COMPILE)g++
AR = $(CROSS_COMPILE)ar
STRIP = $(CROSS_COMPILE)strip

INSTALL = install

## build flags
KERNEL_DIR	?= ../../../../../kernel/linux-2.6.35.12
MIDDLEWARE_INS_DIR ?= ../../../../../middleware/_install
AUTOCONF_DIR	?= ../../../../../buildscript/include


CPPFLAGS = -fvisibility=hidden -ffunction-sections -fdata-sections -fPIC -Wall $(EXTRA_CPPFLAGS)
LDFLAGS = -Wl,--no-export-dynamic -Wl,--gc-sections -Wl,--print-gc-sections $(EXTRA_LDFLAGS)

CPPFLAGS	+= -g -O2 
CPPFLAGS	+= -I$(MIDDLEWARE_INS_DIR)/include
CPPFLAGS	+= -I$(MIDDLEWARE_INS_DIR)/include/snx_vc
CPPFLAGS	+= -I$(MIDDLEWARE_INS_DIR)/include/snx_rc
CPPFLAGS	+= -I$(MIDDLEWARE_INS_DIR)/include/snx_isp
CPPFLAGS	+= -I$(MIDDLEWARE_INS_DIR)/include/snx_record
#CPPFLAGS    += -I$(PWD)/include/libavformat -DHAVE_AV_CONFIG_H -I.
#CPPFLAGS 	+= -I$(PWD)/include/libavutil -DHAVE_AV_CONFIG_H -I.

CPPFLAGS	+= -I$(AUTOCONF_DIR)
CPPFLAGS	+= -I$(AUTOCONF_DIR)/generated
CPPFLAGS	+= -DUSE_SD_RECORD=1
CPPFLAGS	+= -DAUDIO_DUMMY_FRAME_INSERT=1

CPPFLAGS	+= -DTIMELAPSE_SUPPORT=1
#CPPFLAGS    += -DEXTERN_MD_FLAG=1

LDFLAGS		+= -lm -lpthread -ldl
LDFLAGS		+= -L$(MIDDLEWARE_INS_DIR)/lib -L$(MIDDLEWARE_INS_DIR)/../../rootfs/lib/
LDFLAGS		+= -lsnx_vc -lsnx_rc -lsnx_isp 
LDFLAGS		+= -lasound -lsnx_audio
LDFLAGS		+= -lsnx_record
LDFLAGS		+= -lavformat  -lavcodec -lavutil 
LDFLAGS		+= -lxmllib
LDFLAGS		+= -lz

	
SRCS = \
	data_buf.cpp \
	sn98600_record.cpp \
	sn98600_record_audio.cpp \
	sn98600_ctrl.cpp \
	avhandler.cpp \
	main.cpp

AV_SRCS = \
	sn98600_v4l2.cpp

UTIL_SRCS = \
	util.cpp 

	
#SRCS += $(AUDIO_SRC)

AV_OBJS = $(patsubst %.cpp,%.o,$(AV_SRCS))
AV_DEPS = $(patsubst %.cpp,%.d,$(AV_SRCS))
LIB_AV = libav.a

UTIL_OBJS = $(patsubst %.cpp,%.o,$(UTIL_SRCS))
UTIL_DEPS = $(patsubst %.cpp,%.d,$(UTIL_SRCS))
LIB_UTIL = libutil.a

OBJS = $(patsubst %.cpp,%.o,$(SRCS))
DEPS = $(patsubst %.cpp,%.d,$(SRCS))

BININSTALLDIR = $(INSTALL_DIR)/bin/example/ipc_func/record
LIBINSTALLDIR = $(INSTALL_DIR)/lib
SNIP39_DIR    = $(INSTALL_DIR)/../root/etc_default/EXAMPLE
NOTIFY_DIR    = $(PWD)/../../rootfs/root/etc_default/notify

.PHONY: clean all install uninstall
## targets

-include $(DEPS) $(AV_DEPS)

all: $(OBJS) $(LIB_AV) $(LIB_UTIL)
	$(CC) -o $(NAME) $^ -rdynamic $(LDFLAGS)
	$(STRIP) $(NAME)
%.o: %.cpp
	$(CC) $(CPPFLAGS) -c -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"

$(LIB_AV): $(AV_OBJS)
	$(AR) rcs $@ $^

$(LIB_UTIL): $(UTIL_OBJS)
	$(AR) rcs $@ $^

clean:
	rm -f $(OBJS) $(DEPS) $(ARTIFACTS)
	rm -f $(AV_OBJS) $(AV_DEPS) $(LIB_AV)
	rm -f $(UTIL_OBJS) $(UTIL_DEPS) $(LIB_UTIL)
	rm -rf $(NAME)


install: 
	$(INSTALL) -d $(BININSTALLDIR)
	$(INSTALL) $(NAME) $(BININSTALLDIR)
#	$(INSTALL) $(LIB_DIR)/*.so $(MIDDLEWARE_INS_DIR)/lib
	$(INSTALL) -d $(SNIP39_DIR)
	$(INSTALL) files/records.xml $(SNIP39_DIR)
	$(INSTALL) files/snapshot_qp $(SNIP39_DIR)
	$(INSTALL) files/md_threshold $(SNIP39_DIR)
	$(INSTALL) files/md_alarm_onoff $(SNIP39_DIR)
	$(INSTALL) files/md_alarm_record_onoff $(SNIP39_DIR)


