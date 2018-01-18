#ifndef __CC_H__
#define __CC_H__

#include "./jthread/jmutex.h"
#include "./jthread/jthread.h"
#include "video.h"

class CVideoCapture;
class CVideoCodec;
class CVideoOutput;

class CTvViewer: public JThread{
public:
	CTvViewer(CVideoCapture *capture, CVideoCodec *codec, CVideoOutput *output);	
	virtual ~CTvViewer();
	int Open(void);
	int Run(void);
	int Stop(void);
	int Close(void);
	
private:
	void *Thread();
	char _name[128];

	CVideoCapture *_capture;
	CVideoCodec *_codec;
	CVideoOutput *_output;

	snx_frame_ctx_t b[32];

	struct timeval _stv, _etv;
	struct timezone _stz, _etz;
};

struct TV_OUT{
	class CVideoCapture *capture;
	class CVideoCodec *codec;
	class CVideoOutput *output;
	class CTvViewer *tv;
};


#endif
