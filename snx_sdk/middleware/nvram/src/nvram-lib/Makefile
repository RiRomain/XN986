PWD		=$(shell pwd)
TARGET		?= arm-linux
CROSS_COMPILE	?= $(TARGET)-
INSTALL_DIR	?= $(PWD)/_install
LIB_INS_DIR	:= $(INSTALL_DIR)/lib
INC_INS_DIR     := $(INSTALL_DIR)/include
LIBNVRAM_HEADFILE ?=$(LIBNVRAM_DIR)/nvram.h
CC		:= $(CROSS_COMPILE)gcc
CFLAGS = -lpthread -shared
LIBNVRAM = libnvram.so
OBJS =nvram_env.c
.PHONY: all
all: $(LIBNVRAM)
	   
  
$(LIBNVRAM): $(LIBNVRAM_DIR)/nvram.h
	$(MAKE) clean
	$(CC) $(CFLAGS)   -o $(LIBNVRAM)  $(OBJS) -I$(LIBNVRAM_DIR) 	
	if [ ! -d $(LIB_INS_DIR) ]; \
	then \
	install -d $(LIB_INS_DIR); \
	fi
	if [ ! -d $(INC_INS_DIR) ]; \
	then \
	install -d $(INC_INS_DIR); \
	fi
	install $(LIBNVRAM) $(LIB_INS_DIR)
	install $(LIBNVRAM_HEADFILE) $(INC_INS_DIR)


.PHONY: clean
clean:
	-rm -rf  $(EXEC) *.o $(LIBNVRAM)

.PHONY: install
install: 
	if [ ! -d $(LIB_INS_DIR) ]; \
	then \
	install -d $(LIB_INS_DIR); \
	fi
	install $(LIBNVRAM) $(LIB_INS_DIR)
.PHONY: distclean
distclean:


