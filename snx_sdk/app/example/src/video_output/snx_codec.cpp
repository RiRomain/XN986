
#include "tv.h"
#include "codec.h"


CVideoCodec::CVideoCodec(void)
{
	_output = new CInterface(*this, "output");
	_capture = new CInterface(*this, "capture");
}

CVideoCodec::~CVideoCodec(void)
{
	delete _output;
	delete _capture;
}


int CVideoCodec::Open(void)
{
	int i, ret, err, tmp;
	struct v4l2_capability cap;

	for(i = 0; i < 255; i++){
		ret = detect_video_device(V4L2_CAP_VIDEO_CAPTURE, "vc", i);
		if(!ret)
			break;
	}
	if(i == 255){
		goto err;
	}            
  printf ("vc i=%x\n",i);
	sprintf(_name, "/dev/video%d", i);

	_fd =  open(_name, O_RDWR | O_NONBLOCK, 0);	 
	if (_fd == -1){
		goto err;
	}

	if (xioctl(_fd, VIDIOC_QUERYCAP, &cap) == -1) {	 
		fprintf(stderr, "Codec VIDIOC_QUERYCAP error\n");   
		goto err;
	}	

	if (!(cap.capabilities & (V4L2_CAP_VIDEO_CAPTURE|V4L2_CAP_VIDEO_OUTPUT))) {   
		fprintf(stderr, "%s is no video codec device\n", _name);	
		goto err;
	}	

	if (!(cap.capabilities & V4L2_CAP_STREAMING)) {   
		fprintf(stderr, "%s does not support streaming i/o\n", _name);	
		goto err;
	}

	//set block mode
	tmp = fcntl(_fd, F_GETFL, 0);
	tmp &= ~O_NONBLOCK;
	err = fcntl(_fd, F_SETFL, tmp);
	if(err < 0)
		goto err;

	return 0;	
err:
	return -1;
}

int CVideoCodec::Close(void)
{
	close(_fd);
	return 0;
}

CVideoCodec::CInterface::CInterface(CVideoCodec &codec, const char *name):_codec(codec)
{
	strcpy(_name, name);
}

CVideoCodec::CInterface::~CInterface(void)
{
}

int CVideoCodec::CInterface::RequestBuffers(void)
{
	memset(&_req, 0x0, sizeof(_req)); 
	_req.type = _buftype;	
	_req.memory = _memtype;
	_req.count = _nbuffers;
	if (xioctl(_fd, VIDIOC_REQBUFS, &_req) == -1) {
		fprintf(stderr, "%s does not support memory mapping\n", _name);	 
		return -1;	
	}

	if (_req.count < 2) {   
		fprintf(stderr, "Insufficient buffer memory on %s\n", _name);   
		return -1; 
	}
	_nbuffers = _req.count;
	_frames = (snx_frame_ctx_t)calloc(_nbuffers, sizeof(*_frames));	 
	if (!_frames) {   
		fprintf(stderr, "Out of memory\n");   
		return -1;
	}
	memset(_frames, 0x0, sizeof(*_frames)*_nbuffers);

	return 0;
}

int CVideoCodec::CInterface::MmapBuffers(void)
{
	int i;

	for (i = 0; i < _nbuffers; i++) {
		struct v4l2_buffer b;
		memset(&b, 0, sizeof(b));
		b.type = _buftype;
		b.memory = _memtype;
		b.index = i;
		if (xioctl(_fd, VIDIOC_QUERYBUF, &b) == -1){
			fprintf(stderr, "%s query buffer error\n", _name);
			goto err;
		}
		_frames[i].index = b.index;
		if(_memtype == V4L2_MEMORY_MMAP){
			_frames[i].length = b.length;
			_frames[i].userptr = mmap(NULL, b.length, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, b.m.offset);	 
			if (_frames[i].userptr == MAP_FAILED){
				goto err;
			}
		}
	}
	return 0;
err:
	if(i != 0){
		for (i = _nbuffers - 1; i >= 0; i++) {
			if(_frames[i].userptr)
				munmap(_frames[i].userptr, _frames[i].length);
		}
		fprintf(stderr, "%s mem map error\n", _name);
	}
	free(_frames);
	_frames = NULL;	
	return -1;
}

int CVideoCodec::CInterface::QueueBuffers(void)
{
	int i;
	struct v4l2_buffer b;

	if(_memtype== V4L2_MEMORY_MMAP){
		for (i = 0; i < _nbuffers; ++i) {	
			memset(&b, 0, sizeof(b));   
			b.type = _buftype;   
			b.memory = _memtype; 
			b.index = i;
			if (xioctl(_fd, VIDIOC_QBUF, &b) == -1){  
				return -1;
			}
		}
	}
	return 0;
}

int CVideoCodec::CInterface::UnmapBuffers(void)
{
	int i;
	if(_memtype == V4L2_MEMORY_MMAP){
		for (i = 0; _frames && i < _nbuffers; ++i){
			munmap(_frames[i].userptr, _frames[i].length);
		}
	}
	free(_frames);
	return 0;
}


int CVideoCodec::CInterface::Initialize(int *w, int *h, int rate,  __u32 pix, enum v4l2_buf_type buftype, enum v4l2_memory memtype, int nbuffers)
{
	_rate = rate;
	_pix = pix;
	_buftype =buftype;
	_memtype = memtype;
	_nbuffers = nbuffers;

	_fd = _codec.GetFd();

	//set output data fmt
	memset(&_fmt, 0, sizeof(_fmt));

	if(*w == 0 || *h == 0){
		//get major channel data format
		if( !(pix == V4L2_PIX_FMT_SNX420 && buftype == V4L2_BUF_TYPE_VIDEO_OUTPUT))
			return -1;

		_fmt.type = _buftype;   
		if (xioctl(_fd, VIDIOC_G_FMT, &_fmt) == -1)
			return -1;

		*w = _w = _fmt.fmt.pix.width;
		*h = _h = _fmt.fmt.pix.height;
		return 0;
	}

	_fmt.type = _buftype;   
	_fmt.fmt.pix.width = *w;   
	_fmt.fmt.pix.height = *h;
	_fmt.fmt.pix.pixelformat = _pix;
	if (xioctl(_fd, VIDIOC_S_FMT, &_fmt) == -1){
		fprintf(stderr, "%s set format error\n", _name);
		return -1;
	}
	_frame_size = _fmt.fmt.pix.sizeimage;
	*w = _w = _fmt.fmt.pix.width;
	*h = _h = _fmt.fmt.pix.height;

	memset(&_parm, 0, sizeof(_parm));
	_parm.type = buftype;
	_parm.parm.capture.timeperframe.numerator = 1;
	_parm.parm.capture.timeperframe.denominator = _rate;
	if (xioctl(_fd, VIDIOC_S_PARM, &_parm) == -1)	{
		fprintf(stderr, "set paramter error\n");
		return -1;
	}

	if(RequestBuffers() < 0){
		fprintf(stderr, "%s request buffer error\n", _name);
		return -1;
	}

	if(MmapBuffers() < 0){
		fprintf(stderr, "%s request buffer error\n", _name);
		return -1;
	}

	if(QueueBuffers() < 0){
		fprintf(stderr, "%s queue buffer error\n", _name);
		return -1;
	}

	return 0;
}



int CVideoCodec::CInterface::Run(void)
{
	if (-1 == xioctl(_fd, VIDIOC_STREAMON, &_memtype)){	 
		fprintf(stderr, "%s stream on error\n", _name);
		return -1;
	}
	return 0;
}

int CVideoCodec::CInterface::Stop(void)
{
	if (-1 == xioctl(_fd, VIDIOC_STREAMOFF, &_memtype)){	 
		fprintf(stderr, "%s stream off error\n", _name);
		return -1;
	}
	return 0;
}

snx_frame_ctx_t CVideoCodec::CInterface::DeFrame(void)
{
	struct v4l2_buffer buf;
	snx_frame_ctx_t sb;

	memset(&buf, 0, sizeof(buf));   
	buf.memory = _memtype;
	buf.type = _buftype;
	if(xioctl(_fd, VIDIOC_DQBUF, &buf) == -1) {	
		switch (errno) {   
			fprintf(stderr, "%s dequeue buffer error\n", _name);
			return NULL;
		}
	}
	sb = &_frames[buf.index];
	sb->size = buf.bytesused;
	return sb;
}

int CVideoCodec::CInterface::EnFrame(snx_frame_ctx_t buffer)
{
	struct v4l2_buffer buf;

	memset(&buf, 0, sizeof(buf));   
	buf.memory = _memtype;
	buf.index = buffer->index;
	buf.type = _buftype;
	if(buf.memory == V4L2_MEMORY_USERPTR){
		buf.m.userptr = (unsigned long)buffer->userptr;
		buf.length = buffer->length;
	}	
	if(xioctl(_fd, VIDIOC_QBUF, &buf) == -1) {	
		switch (errno) {
			fprintf(stderr, "%s enqueue buffer error\n", _name);
			return -1;
		}
	}

	return 0;
}


int CVideoCodec::CInterface::Cleanup(void)
{
	UnmapBuffers();
	return 0;
}