PWD			:= $(shell pwd)
ARCH			?= arm
CROSS_COMPILE		?= $(ARCH)-linux-
vKERNEL			?= 2.6.35.12
INSTALL_DIR		?= ../../../lib/modules/$(vKERNEL)/kernel/drivers
KO_INSTALL_DIR	?= ../../../ko/vc2
EXTRA_CFLAGS		+= -O2 -DUSE_DEF_FONT=0 -I$(AUTOCONF_DIR) -Wall


-include $(AUTOCONF_DIR)/config/snx_sdk.conf

export ARCH CROSS_COMPILE

OBJS_ISP 		:= dummy.o
OBJS_ISP2 		:= dummy2.o

OBJS_CODEC 		:= snx_vid_intr.o
OBJS_CODEC 		+= snx_ic_datastamp.o
OBJS_CODEC 		+= snx_ic.o snx_h264.o
OBJS_CODEC 		+= snx_mjpeg.o
OBJS_CODEC 		+= snx_vid_codec.o
OBJS_CODEC 		+= snx_ctx_ctl.o

ifeq ($(CONFIG_SYSTEM_SERIES),"sn9866x")
OBJS_CODEC 		+= snx_ic_mroi.o
OBJS_CODEC 		+= snx_ic_mbrc.o
endif

OBJS_DEC		:= snx_vid_dec.o
OBJS_DEC		+= snx_h264.o snx_ic.o
OBJS_DEC		+= snx_mjpeg.o

OBJS_SNX_VB2		:= snx_videobuf2.o

MODULE_ISP		= snx_dummy
MODULE_ISP2		= snx_dummy2
MODULE_CODEC		= snx_vc
MODULE_SNX_VB2		= snx_vb2
MODULE_DEC		= snx_vc_dec

#MODULES			= $(MODULE_CODEC).ko $(MODULE_SNX_VB2).ko $(MODULE_DEC).ko
MODULES			= $(MODULE_CODEC).ko $(MODULE_SNX_VB2).ko


$(MODULE_ISP)-objs	:= $(OBJS_ISP)
$(MODULE_ISP2)-objs	:= $(OBJS_ISP2)
$(MODULE_CODEC)-objs	:= $(OBJS_CODEC)
$(MODULE_SNX_VB2)-objs	:= $(OBJS_SNX_VB2)
$(MODULE_DEC)-objs	:= $(OBJS_DEC)

#obj-m			:= $(MODULE_CODEC).o $(MODULE_SNX_VB2).o $(MODULE_DEC).o
obj-m			:= $(MODULE_CODEC).o $(MODULE_SNX_VB2).o

ifneq ($(CONFIG_SYSTEM_SERIES),"sn9866x")
obj-m			+= $(MODULE_DEC).o
MODULES			+= $(MODULE_DEC).ko
endif

ifeq ($(DUMMY_ISP),1)
obj-m			+= $(MODULE_ISP).o $(MODULE_ISP2).o
MODULES			+= $(MODULE_ISP).ko $(MODULE_ISP2).ko
endif

modules:	
	@ if [ -z "$(KERNEL_DIR)" ]; then \
		echo "Error! Please set kernel directory by 'KERNEL_DIR' variable"; \
		exit 1; \
	fi;
	@ if [ ! -d $(KO_INSTALL_DIR) ]; \
	then \
		install -d $(KO_INSTALL_DIR); \
	fi	
	$(MAKE) -C $(KERNEL_DIR)/src M=$(PWD) modules
	install -c $(MODULE_CODEC).ko $(KO_INSTALL_DIR)
	install -c $(MODULE_SNX_VB2).ko $(KO_INSTALL_DIR)
ifneq ($(CONFIG_SYSTEM_SERIES),"sn9866x")
	install -c $(MODULE_DEC).ko $(KO_INSTALL_DIR)
endif
	install -c Makefile $(KO_INSTALL_DIR)

.PHONY: clean
clean:
	-rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions Module.symvers modules.order

install:
	@ if [ ! -d $(INSTALL_DIR) ]; \
	then \
		install -d $(INSTALL_DIR); \
	fi
	install -c $(abspath $(KO_INSTALL_DIR))/$(MODULE_CODEC).ko $(INSTALL_DIR)
	install -c $(abspath $(KO_INSTALL_DIR))/$(MODULE_SNX_VB2).ko $(INSTALL_DIR)
ifneq ($(CONFIG_SYSTEM_SERIES),"sn9866x")
	install -c $(abspath $(KO_INSTALL_DIR))/$(MODULE_DEC).ko $(INSTALL_DIR)
endif

uninstall:
	@ if [ ! -d $(INSTALL_DIR) ]; \
	then \
		install -d $(INSTALL_DIR); \
	fi
	rm -rf  $(INSTALL_DIR)/$(MODULE_CODEC).ko
	rm -rf  $(INSTALL_DIR)/$(MODULE_SNX_VB2).ko
	rm -rf  $(INSTALL_DIR)/$(MODULE_DEC).ko
