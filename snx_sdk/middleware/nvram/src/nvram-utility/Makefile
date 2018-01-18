PWD		=$(shell pwd)
TARGET		?= arm-linux
CROSS_COMPILE	?= $(TARGET)-
INSTALL_DIR	?= $(PWD)/_install
BIN_INS_DIR	:= $(INSTALL_DIR)/bin
CC		:= $(CROSS_COMPILE)gcc
utility = nvram_utility
LN=ln
nvram_get = nvram_get
nvram_set = nvram_set
.PHONY: all
all: $(utility) 
	   
  
$(utility):  $(LIBNVRAM_DIR)/nvram.h
	$(MAKE) clean
	$(CC)  $(utility).c -o $(utility) -I$(LIBNVRAM_DIR) -L../nvram-lib -lnvram
	$(LN) -s $(utility) $(nvram_get)
	$(LN) -s $(utility) $(nvram_set)


.PHONY: clean
clean:
	-rm -rf $(EXEC) *.o $(utility) $(nvram_get) $(nvram_set)

.PHONY: install
install:
	if [ ! -d $(BIN_INS_DIR) ]; \
	then \
	install -d $(BIN_INS_DIR); \
	fi
	cp -rf $(utility) $(nvram_get) $(nvram_set) $(BIN_INS_DIR)
.PHONY: distclean
distclean:


