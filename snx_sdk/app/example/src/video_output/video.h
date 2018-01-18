
#ifndef __VIDEO_H__
#define __VIDEO_H__


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
#include <asm/types.h>			/* for videodev2.h */	
#include <linux/videodev2.h> 

#include "api/snxtype.h"


enum{
	RAW_DATA = 0x0,
	H264_DATA,
	MJPEG_DATA,
};

static inline void errno_exit(const char * s) {   
	fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));   
	exit(EXIT_FAILURE);
}	

static inline int xioctl(int fd, int request, void * arg) {   
	int r;	 
	do{
		r = ioctl(fd, request, arg);   
	}while (-1 == r && EINTR == errno);   

	return r;
}

static inline int detect_video_device(int type, const char *name, int index)
{
	int ret, fd;
	char buf[16];
	struct v4l2_capability cap;

	sprintf(buf, "/dev/video%d", index);
	fd =  open(buf, O_RDWR | O_NONBLOCK, 0);	
	if(fd<0)
		return fd;

	if (xioctl(fd, VIDIOC_QUERYCAP, &cap) == -1) {
		ret = -1;
		goto fail;	
	}	
	
	if (!(cap.capabilities & type)) {
		ret = -1;
		goto fail;
	}	
	if(!strstr((const char *)cap.driver, name)){
		ret = -1;
		goto fail;
	}
	ret = 0;

fail:	
	close(fd);
	return ret;
}


#endif /*__VIDEO_H__*/
