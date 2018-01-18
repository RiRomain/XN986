
#ifndef __OUTPUT_H__
#define __OUTPUT_H__


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
#include <sys/stat.h>	
#include <sys/types.h>	 
#include <sys/time.h>	
#include <sys/mman.h>	
#include <sys/ioctl.h>	 
#include <asm/types.h>			/* for videodev2.h */	
#include <linux/videodev2.h> 

#include "video.h"

enum{
	VO_CH_0 = 1,
	VO_CH_1,
	VO_CH_2,
	VO_CH_3,
};

class CVideoOutput
{
public:
	CVideoOutput(int ch, int x, int y, int w, int h, int cw, int sh, enum v4l2_memory iotype, int nbuffers);
	virtual ~CVideoOutput();
	int Open();
	int Run();
	int Stop(void);	
	int Close(void);
	
	inline int GetFd(void){return _fd;}
	inline int GetBufferCount(void){return _nbuffers;}
	inline enum v4l2_memory GetMemType(void){return _memtype;}
	snx_frame_ctx_t DeFrame();
	int EnFrame(snx_frame_ctx_t frame);	
private:
	int DeviceInitilize(void);	
	int CaptureStart(void);
	int CaptureStop(void);
	int DeviceCleanup(void);

	char _name[32];
	int _fd;
	int _ch, _x, _y, _w, _h,  _sw, _sh;
	enum v4l2_memory _memtype;

	struct v4l2_capability _cap;   
	struct v4l2_cropcap _cropcap;	
	struct v4l2_crop _crop;  
	struct v4l2_format _fmt;
	struct v4l2_requestbuffers _req;
	int _frame_sz;
	snx_frame_ctx_t _frames;
	int _nbuffers;

	struct timeval _stv, _etv;
	struct timezone _stz, _etz;
};

#endif /*__OUTPUT_H__*/
