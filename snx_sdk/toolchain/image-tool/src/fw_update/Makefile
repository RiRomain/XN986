#CC = arm-linux-gcc
CROSS_COMPILE ?= arm-linux-
CC = $(CROSS_COMPILE)gcc

all:
	$(CC) -o fw_burn.o -c fw_burn.c -fvisibility=hidden -Wl,--no-export-dynamic -I$(AUTOCONF_DIR)
	$(CC) -o image.o -c image.S -fvisibility=hidden -Wl,--no-export-dynamic
	$(CC) -o firmware_update fw_burn.o image.o -fvisibility=hidden -Wl,--no-export-dynamic

clean:
	rm -rf *.o FIRMWARE.bin
