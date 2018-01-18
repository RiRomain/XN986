
#ifndef __CAPTURE_H__
#define __CAPTURE_H__


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

#include "api/sn926_isp_interface.h"

class CVideoCapture
{
public:
	CVideoCapture(int w, int h, int rate, enum v4l2_memory iotype, int nbuffers);
	virtual ~CVideoCapture();

	int Open();
	int Run();
	int Stop(void);
	int Close(void);

	inline int GetFd(void){return _fd;}
	inline int GetCH(void){return _ch;}	
	inline int GetWidth(void){return _w;}
	inline int GetHeight(void){return _h;}
	inline int GetFmt(void){return V4L2_PIX_FMT_YUV420;}
	inline int GetRate(void){return _rate;}
	inline int GetBufferCount(void){return _nbuffers;}
	inline enum v4l2_memory GetIOtype(void){return _memtype;}

	snx_frame_ctx_t DeFrame();
	int EnFrame(snx_frame_ctx_t frame);

private:
	int DeviceInitilize(void);	
	int CaptureStart(void);
	int CaptureStop(void);
	int DeviceCleanup(void);

	char _name[32];
	int _fd;
	int _ch, _w, _h, _sw, _sh, _rate;
	enum v4l2_memory _memtype;
	int _frame_sz;

	struct v4l2_capability _cap;
	struct v4l2_cropcap _cropcap;
	struct v4l2_crop _crop;
	struct v4l2_format _fmt;
	struct v4l2_streamparm _parm;
	struct v4l2_requestbuffers _req;

	snx_frame_ctx_t _frames;
	int _nbuffers;
};

#endif /*__CAPTURE_H__*/
