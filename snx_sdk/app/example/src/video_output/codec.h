
#ifndef __CODEC_H__
#define __CODEC_H__


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

#include <queue>

#include "video.h"


class CVideoCodec
{
public:
	CVideoCodec(void);
	virtual ~CVideoCodec(void);

	int Open(void);
	int Close(void);

	inline int GetFd(void){return _fd;}

public:
	
	class CInterface
	{
	public:
		CInterface(CVideoCodec &codec, const char *name);
		virtual ~CInterface(void);

		int Initialize(int *w, int *h, int rate, __u32 pix, enum v4l2_buf_type buftype, enum v4l2_memory memtype, int nbuffers);

		inline enum v4l2_buf_type GetBufType(void){return _buftype;}
		inline enum v4l2_memory GetMemType(void){return _memtype;}
		inline int GetBufferCount(void){return _nbuffers;}
		inline int GetWidth(void){return _w;}
		inline int GetHeight(void){return _h;}
		snx_frame_ctx_t DeFrame(void);
		int EnFrame(snx_frame_ctx_t frame);
		int Run(void);
		int Stop(void);
		int Cleanup(void);

	private:

		int RequestBuffers(void);
		int MmapBuffers(void);
		int QueueBuffers(void);
		int UnmapBuffers(void);

		int _fd;

		int _w, _h, _rate;
		__u32 _pix;
		enum v4l2_buf_type _buftype;
		enum v4l2_memory _memtype;
		int _frame_size;
		int _nbuffers;
		char _name[32];

		struct v4l2_format _fmt;
		struct v4l2_streamparm _parm;

		struct v4l2_requestbuffers _req;
		snx_frame_ctx_t _frames;

		CVideoCodec &_codec;
	};

	CInterface *_output;
	CInterface *_capture;

private:


	char _name[32];
	int _fd;

	//struct timeval _stv, _etv;
	//struct timezone _stz, _etz;
};

#endif /*__CODEC_H__*/
