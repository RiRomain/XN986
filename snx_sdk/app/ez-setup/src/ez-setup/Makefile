######################################################
PWD     		:= $(shell pwd)
TARGET			?= arm-linux
CROSS_COMPILE   	?= $(TARGET)-
CPP     		:= $(CROSS_COMPILE)g++
CC      		:= $(CROSS_COMPILE)gcc
AR      		:= $(CROSS_COMPILE)ar
RANLIB			:= $(CROSS_COMPILE)ranlib

######################################################
-include $(AUTOCONF_DIR)/config/snx_sdk.conf

BIN_DIR			= $(INSTALL_DIR)/bin
CONFIG_APP_EZ_DIR	?= SNIP39
SNX_EZ_DIR		= $(INSTALL_DIR)/root/etc_default/$(CONFIG_APP_EZ_DIR)
DEF_DIR			= files

INSTALL			= install

### WEP=0 / WPA=1 / WPA2=2
WIFI_ENCRYPT	= 2
######################################################
SSID_NEW	= 36099990220
SSID_KEY_NEW	= zzzz8998
# 10.42.0.100 ~ 10.42.0.251
IP_RANGE_NEW	= 10.42.0.

SSID_OLD	= SNIP39_AAINAMMK
SSID_KEY_OLD	= 2mpAzUUmsU77q
# 5.5.1.100 ~ 5.5.1.251
IP_RANGE_OLD	= 5.5.1.
######################################################

DEF_CONF	= default.conf
UID_CONF	= SNIP39_UID.conf

DHCPD 		= udhcpd.conf
WPA_SUPPLICANT	= wpa_supplicant.conf
UDHCPC_RENEW	= udhcpc_renew.sh




ifeq ($(WIFI_ENCRYPT), 0)
HOSTAP		= hostapd_wep.conf
endif
ifeq ($(WIFI_ENCRYPT), 1)
HOSTAP		= hostapd_wpa.conf
endif
ifeq ($(WIFI_ENCRYPT), 2)
HOSTAP		= hostapd_wpa2.conf
endif

SNX_EZ_LIB	= libsnx_ez.so
EZ_OBJS		= snx_ez.o
SNX_HTTP_OBJS	= snx_http.o
SNX_RST_OBJS	= snx_rst.o
SNX_SCAN_OBJS	= snx_scan.o

CFLAGS		= -O2 $(APP_FLAGS) -Werror

ifeq ($(CONFIG_MODULE_USB_WIFI_BCM43143B0), y)
CFLAGS 		+= -DBCM=1
endif

ifeq ($(CONFIG_MODULE_USB_WIFI_RT3070_AP), y)
CFLAGS 		+= -DRT3070=1
RALINK_CONF	= RT2870AP
endif

ifeq ($(CONFIG_MODULE_USB_WIFI_MT7601U_AP), y)
CFLAGS 		+= -DMT7601=1
RALINK_CONF	= RT2870AP
endif

ifeq ($(CONFIG_MODULE_USB_WIFI_RTL8188EUS_4_3_0_3_10997), y)
CFLAGS 		+= -DRTL8188=1
endif

CFLAGS 		+= -DEZ_DIR=\"/etc/$(CONFIG_APP_EZ_DIR)/\"

CFLAGS 		+= -I$(MIDDLEWARE_INS_DIR)/include
CFLAGS		+= -I$(MIDDLEWARE_INS_DIR)/include/snx_vc
CFLAGS		+= -I$(MIDDLEWARE_INS_DIR)/include/snx_rc
CFLAGS		+= -I$(AUTOCONF_DIR)
CFLAGS		+= -I$(AUTOCONF_DIR)/generated

CFLAGS 		+= -Iinclude -lcrypto -lssl -lzbar -lsnx_vc -lsnx_rc -lnvram -lgpio -lsnx_isp 

LDFLAGS		+= -L$(MIDDLEWARE_INS_DIR)/lib/

CFLAGS		+= -fvisibility=hidden
LDFLAGS		+= -Wl,--no-export-dynamic


EZ_LDFLAGS	= -lsnx_ez

targets = snx_ez snx_rst
targets +=  snx_scan
targets +=  snx_ae_ctl

all: $(targets)

snx_ae_ctl: snx_ae_ctl.o 
	$(CC) $(CFLAGS) $? $(LDFLAGS) -o $@

snx_scan:  $(SNX_SCAN_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SNX_SCAN_OBJS) -o $@ 


snx_ez: $(EZ_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(EZ_OBJS) $(EZ_LDFLAGS) -o $@ 

snx_rst: $(SNX_RST_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SNX_RST_OBJS) $(EZ_LDFLAGS) -o $@ 

	
clean:
	rm -f *.o $(targets) $(SNX_EZ_LIB)

install:
	$(INSTALL) -d $(BIN_DIR)
	$(INSTALL) snx_ez $(BIN_DIR)
	$(INSTALL) snx_rst $(BIN_DIR)
	$(INSTALL) snx_scan $(BIN_DIR)
	$(INSTALL) snx_ae_ctl $(BIN_DIR)

	$(INSTALL) -d $(SNX_EZ_DIR)
	$(INSTALL) $(DEF_DIR)/$(HOSTAP) $(SNX_EZ_DIR)/$(HOSTAP)
	$(INSTALL) $(DEF_DIR)/$(DHCPD) $(SNX_EZ_DIR)/$(DHCPD)
	$(INSTALL) $(DEF_DIR)/$(WPA_SUPPLICANT) $(SNX_EZ_DIR)/$(WPA_SUPPLICANT)
	$(INSTALL) $(DEF_DIR)/$(DEF_CONF) $(SNX_EZ_DIR)/$(DEF_CONF)
	$(INSTALL) $(DEF_DIR)/$(UID_CONF) $(SNX_EZ_DIR)/$(UID_CONF)
	$(INSTALL) $(DEF_DIR)/$(UDHCPC_RENEW) $(SNX_EZ_DIR)/$(UDHCPC_RENEW)

ifeq ($(CONFIG_MODULE_USB_WIFI_RT3070_AP), y)
	$(INSTALL) -d $(INSTALL_DIR)/root/etc_default/Wireless/$(RALINK_CONF)
	$(INSTALL) $(DEF_DIR)/$(RALINK_CONF).dat $(INSTALL_DIR)/root/etc_default/Wireless/$(RALINK_CONF)/$(RALINK_CONF).dat
endif

	sed -i 's/$(SSID_OLD)/$(SSID_NEW)/g' $(SNX_EZ_DIR)/$(HOSTAP)
	sed -i 's/$(SSID_KEY_OLD)/$(SSID_KEY_NEW)/g' $(SNX_EZ_DIR)/$(HOSTAP)
	sed -i 's/$(SSID_KEY_OLD)/$(SSID_KEY_NEW)/g' $(SNX_EZ_DIR)/$(HOSTAP)
	sed -i 's/$(IP_RANGE_OLD)/$(IP_RANGE_NEW)/g' $(SNX_EZ_DIR)/$(DHCPD)



