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
CATEGORY	:= crypto

INC     	= -I $(KERNEL_DIR)/src/include
INC			+= -I $(PWD)/include
CFLAGS      := -Wall -g -O2 $(EXTRA_CFLAGS) $(INC)
LDFLAGS     := $(EXTRA_LDFLAGS) -Wl,--as-needed


SNX_CRYPTO = snx_crypto_flow.o

targets = snx_crypto_example

.PHONY : clean distclean all
%.o : %.c
	$(CC) $(CFLAGS) -c $<

all: $(targets)

$(SNX_CRYPTO): crypto/snx_crypto_flow.c
	$(CC) $(CFLAGS) -c -o $@ $<

snx_crypto_example: snx_crypto_example.o $(SNX_CRYPTO)
	$(CC) $(CFLAGS) -o $@ $+ $(LDFLAGS)

	
.PHONY: install
install:
	@ if [ ! -d $(BIN_INS_DIR)/$(CATEGORY) ]; \
	then \
	install -d $(BIN_INS_DIR)/$(CATEGORY); \
	fi
	install -c $(targets) $(BIN_INS_DIR)/$(CATEGORY)

clean:
	rm -f *.o *.yuv *.bak *.a *.out *.so $(targets)

distclean : clean
	rm -f $(targets)
