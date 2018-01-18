

CPP 	:= $(CROSS_COMPILE)g++
CC	:= $(CROSS_COMPILE)gcc
LD	:= $(CROSS_COMPILE)ld
AR	:= $(CROSS_COMPILE)ar
SIZE	:= $(CROSS_COMPILE)size
OBJDUMP := $(CROSS_COMPILE)objdump
STRIP	:= $(CROSS_COMPILE)strip

MAKE	:= make
CD	:= cd
CP	:= cp

TARGETS := ./
HEADERS := ../

DST_PATH := ./
LIB_PATH := ./lib


#CFLAGS 	:= -c -g3 -O2 -Wall

CFLAGS 	:= -c -O2 -Wall
ARFLAGS := -ruv
OBJDUMPFLAGS := -d

LDFLAGS := -Map map.txt -N -g



OBJS 	=  jmutex.o jthread.o


INC 	= -I $(TARGETS) -I $(HEADERS)

all:	rm_lib_dir mk_lib_dir $(OBJS)
	$(AR) -r $(LIB_PATH)/libjthread.a $(OBJS)


$(OBJS): %.o: %.cpp
	$(CPP) $(INC)  $(CFLAGS) $< -o $@



mk_lib_dir:
	mkdir $(LIB_PATH)

rm_lib_dir:
	rm -rf $(LIB_PATH)

.PHONY:clean
clean:
	rm -rf ./*.o $(LIB_PATH)





