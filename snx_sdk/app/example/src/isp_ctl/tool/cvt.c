#include <stdio.h>	 
#include <stdlib.h>   
#include <string.h>   
#include <assert.h>   
#include <getopt.h> 			/* getopt_long() */   
#include <fcntl.h>				/* low-level i/o */   
#include <unistd.h>   
#include <errno.h>	 
#include <malloc.h>  
#include <signal.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>	
#include <sys/types.h>	 
#include <sys/time.h>	
#include <sys/mman.h>	
#include <sys/ioctl.h>	 



int main(){
	int i, fd,_fd,len, sz;
	unsigned char c[64], buf[1024];
	fd = open("dat.bin", O_RDONLY, 0660);
	_fd = open("ascii32x16.dat", O_WRONLY | O_CREAT | O_TRUNC, 0660);
	while(read(fd, c, 64) > 0){
		for(i=0; i < 64;i ++){
			unsigned char __x = c[i];

			c[i] = (((__x&0x01)>>0)<<7|
					((__x&0x02)>>1)<<6|
					((__x&0x04)>>2)<<5|
					((__x&0x08)>>3)<<4|
					((__x&0x10)>>4)<<3|
					((__x&0x20)>>5)<<2|
					((__x&0x40)>>6)<<1|
					((__x&0x80)>>7)<<0);
			//sz = sprintf(buf, "0x%02x", c[i]);
			//sz = sprintf(buf, "0x%02x", __x);
			//buf[sz++] = ',';
			//write(_fd, buf, sz);
		}
		write(_fd, c, 64);
		//write(_fd, buf, sz);
	}
	close(_fd);
	close(fd);
	return 0;
}



