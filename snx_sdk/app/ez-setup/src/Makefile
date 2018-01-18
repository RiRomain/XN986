
PACKAGE		= ez-setup
PACKAGE		+= pcm_play

MULTI_VIDEO		= 0
SOUND_DEBUG		= 1
WIFI_CHECK		= 0
### WEP=0 / WPA=1 / WPA2=2
WIFI_ENCRYPT		= 2
CMD_ENC			= 1

########################################################################

INSTALL_DIR     ?= ../rootfs

APP_FLAGS	+= -DMULTI_VIDEO=$(MULTI_VIDEO)
APP_FLAGS	+= -DWIFI_ENCRYPT=$(WIFI_ENCRYPT)
APP_FLAGS	+= -DSOUND_FOR_DEBUG=$(SOUND_DEBUG)
APP_FLAGS 	+= -DWIFI_CHECK_DEBUG=$(WIFI_CHECK)
APP_FLAGS	+= -DCMD_ENC=$(CMD_ENC)
#APP_FLAGS 	+= -g

.PHONY:	clean all install

all:
	@ for dir in $(PACKAGE); do $(MAKE) -C  $$dir APP_FLAGS="$(APP_FLAGS)" || exit 1; done


install: 
	@ for dir in $(PACKAGE); \
	do $(MAKE) -C $$dir APP_360="$(APP_360)" WIFI_ENCRYPT="$(WIFI_ENCRYPT)"  install \
	|| exit 1; done

clean: 
	@ for dir in $(PACKAGE); do $(MAKE) -C $$dir clean || exit 1; done
	rm -f ./target/sbin/*
