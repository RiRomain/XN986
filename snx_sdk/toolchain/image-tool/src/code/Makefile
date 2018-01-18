#

CC := gcc
CFLAG += -Wall -Os -I$(AUTOCONF_DIR)

TARGET := image_tool

OBJ := crc.o
OBJ += flash.o
OBJ += uboot.o
OBJ += hw_setting.o
OBJ += main.o

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAG) -o $@ $(OBJ)

%.o: %.c
	$(CC) -c $(CFLAG)  $<

clean:
	$(RM) $(OBJ) ./image_tool
