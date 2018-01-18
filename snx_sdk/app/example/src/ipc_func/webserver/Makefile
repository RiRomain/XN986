NAME = snx_webserver

PWD     	:= $(shell pwd)
TARGET      ?= arm-linux
CROSS_COMPILE   ?= $(TARGET)-
CPP     	:= $(CROSS_COMPILE)g++
CC      	:= $(CROSS_COMPILE)gcc
AR      	:= $(CROSS_COMPILE)ar
RANLIB      := $(CROSS_COMPILE)ranlib
STRIP 		:= $(CROSS_COMPILE)strip
INSTALL 	?= install


INSTALL_DIR	?= ../../../rootfs/usr
WWW_DIR		= $(INSTALL_DIR)/../root/etc_default/www
CONF_DIR	= $(INSTALL_DIR)/../root/etc_default/mongoose
BIN_INS_DIR	:= $(INSTALL_DIR)/bin/example
LIB_INS_DIR	:= $(INSTALL_DIR)/lib/example

CATEGORY	:= ipc_func/web_server

KERNEL_DIR	?= ../../../../../kernel/linux-2.6.35.12
MIDDLEWARE_INS_DIR ?= ../../../../../middleware/_install
AUTOCONF_DIR	?= ../../../../../buildscript/include

INC     	= -I$(KERNEL_DIR)/src/include
INC     	+= -I$(MIDDLEWARE_INS_DIR)
INC			+= -I$(MIDDLEWARE_INS_DIR)/include
INC 		+= -I$(PWD)/libmongoose

## toolchain
EXTRA_LDFLAGS	= -lssl -lcrypto
EXTRA_CFLAGS	+= -I$(AUTOCONF_DIR)

CFLAGS      := -Wall  -g -O2 $(EXTRA_CFLAGS) $(INC) 
CFLAGS		+= -DNS_ENABLE_SSL
#CFLAGS		+= -DDEBUG
CFLAGS      += -DNS_DISABLE_SOCKETPAIR

LDFLAGS     := -Wl,--as-needed -L$(MIDDLEWARE_INS_DIR)/lib $(EXTRA_LDFLAGS) 


LIBMONGOOSE	= libmongoose/mongoose.o
SRC_OBJ	= src/web_server.o 

FILES_DIR	= $(PWD)/files

#targets =
targets = snx_webserver


.PHONY : clean distclean all

%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

all: $(targets)

snx_webserver:  $(SRC_OBJ) $(LIBMONGOOSE) 
	$(CC) -o $@ $^  $(LDFLAGS)
	$(STRIP) $@


.PHONY: install
install:
	@ if [ ! -d $(BIN_INS_DIR)/$(CATEGORY) ]; \
	then \
	$(INSTALL) -d $(BIN_INS_DIR)/$(CATEGORY); \
	fi
	$(INSTALL) -c $(NAME) $(BIN_INS_DIR)/$(CATEGORY)
	$(INSTALL) -d $(WWW_DIR)
	$(INSTALL) -d $(CONF_DIR)
	cp -f $(FILES_DIR)/mongoose.conf $(CONF_DIR)
	cp -f $(FILES_DIR)/certs/cert.pem $(CONF_DIR)
	cp -rf $(FILES_DIR)/www/* $(WWW_DIR)

clean:
	rm -f *.o *.yuv *.bak *.a *.out *.so $(SRC_OBJ) $(LIBMONGOOSE) $(targets)

distclean: clean
	rm -f $(targets)
