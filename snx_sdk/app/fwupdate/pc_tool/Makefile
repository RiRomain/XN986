##########################################
# Author:       Kelvin Cheung
# Date:         Jan 13, 2009
# Copyright:    (C) SONIX Technology, Inc.
##########################################

ifeq ($(DEBUG),y)
  DEBFLAGS = -O -g # "-O" is needed to expand inlines
else
  DEBFLAGS = -O2
endif

CFLAGS += $(DEBFLAGS)

TARGET		?= arm-linux
CROSS_COMPILE	?= $(TARGET)-

#CC		:= $(CROSS_COMPILE)gcc
CC		:= g++

targets = dt

.PHONY: all
all: $(targets)

dt: dt.o
	$(CC) $^ -o $@

.PHONY: clean
clean:
	-rm -rf $(targets) *.o

.PHONY: install
install: all
	install -c $(targets) $(INSTALLDIR)

.PHONY: uninstall
uninstall:
	-rm -rf $(addprefix $(INSTALLDIR)/,$(targets))
