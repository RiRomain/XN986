TARGET		?= arm-linux
CROSS_COMPILE	?= $(TARGET)-
INSTALL_DIR	?= ../rootfs/usr
BIN_INS_DIR	:= $(INSTALL_DIR)/bin/example
LIB_INS_DIR	:= $(INSTALL_DIR)/lib/example
RTC	:= rtc
CC		:= $(CROSS_COMPILE)gcc

targets = snx_rtc_test

.PHONY: all
all: $(targets)

snx_rtc_test: snx_rtc_test.o
	$(CC) $^ -o $@

.PHONY: clean
clean:
	-rm -rf $(targets) *.o

.PHONY: install
install:
	@ if [ ! -d $(BIN_INS_DIR)/$(RTC) ]; \
	then \
		install -d $(BIN_INS_DIR)/$(RTC); \
	fi
	install -c $(targets) $(BIN_INS_DIR)/$(RTC)

