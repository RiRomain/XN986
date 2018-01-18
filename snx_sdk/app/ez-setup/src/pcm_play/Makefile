PWD     	:= $(shell pwd)
TARGET		?= arm-linux
CROSS_COMPILE   ?= $(TARGET)-
CC      	:= $(CROSS_COMPILE)gcc
BIN_DIR		= $(INSTALL_DIR)/usr/bin
#PCM_DIR		= $(INSTALL_DIR)/usr/share/pcm
PCM_DIR		= $(INSTALL_DIR)/root/etc_default/notify/
INSTALL		= install

CFLAGS		= -I$(MIDDLEWARE_INS_DIR)/include
CFLAGS          += $(APP_FLAGS)
LDFLAGS		= -L$(MIDDLEWARE_INS_DIR)/lib
LDFLAGS		+= -lasound

CFLAGS		+= -fvisibility=hidden
LDFLAGS		+= -Wl,--no-export-dynamic

#PCM_FILE 		= $(PWD)/notify/reset.pcm
PCM_FILE 		= $(PWD)/notify/*
######################################################


TARGET := pcm_play
OBJ += pcm_play.o

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $(OBJ)

%.o: %.c
	$(CC) -c $(CFLAGS)  $<
	
clean:
	rm -f *.o $(TARGET)

install:
	$(INSTALL) -d $(BIN_DIR)
	$(INSTALL) $(TARGET) $(BIN_DIR)

	$(INSTALL) -d $(PCM_DIR)
	cp $(PCM_FILE) $(PCM_DIR)
#	$(INSTALL) $(PCM_FILE) $(PCM_DIR)/$(PCM_FILE)



