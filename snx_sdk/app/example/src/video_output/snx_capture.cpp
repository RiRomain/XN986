
#include "tv.h"
#include "capture.h"


CVideoCapture::CVideoCapture(int w, int h, int rate, enum v4l2_memory iotype, int nbuffers):
				_w(w),
				_h(h),
				_rate(rate),
				_memtype(iotype),
				_nbuffers(nbuffers)
{
	_sw = _w;
	_sh = _h;
}


CVideoCapture::~CVideoCapture()
{
}



int CVideoCapture::Open(void)
{
	int i, ret, err, tmp;
	for(i = 0; i < 255; i++){
		ret = detect_video_device(V4L2_CAP_VIDEO_CAPTURE, "isp", i);
		if(!ret)
			break;
	}
	if(i == 255){
    
		goto err;
	}
  printf ("isp i=%x\n",i);
	sprintf(_name, "/dev/video%d", i);

	_fd =  open(_name, O_RDWR, 0);	 
	if (_fd == -1){
    
		goto err;
	}

	//set block mode
	tmp = fcntl(_fd, F_GETFL, 0);
	tmp &= ~O_NONBLOCK;
	err = fcntl(_fd, F_SETFL, tmp);
	if(err < 0)
		goto err;
  
	if(DeviceInitilize() <0)
		goto err ;
    
	return 0;	
err:
	return -1;
}

int CVideoCapture::Close(void)
{
	DeviceCleanup();
	close(_fd);
	return 0;
}

int CVideoCapture::DeviceInitilize(void)
{
	int i;

	if (xioctl(_fd, VIDIOC_QUERYCAP, &_cap) == -1) {	 
		return -1;
	}	

	if (!(_cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {   
		fprintf(stderr, "%s is no video capture device\n", _name);	
		return -1;
	}

	if (!(_cap.capabilities & V4L2_CAP_STREAMING)) {   
		fprintf(stderr, "%s does not support streaming i/o\n", _name);	
		return -1;
	}  

	memset(&_cropcap, 0, sizeof(_cropcap));   
	_cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;   
	if (xioctl(_fd, VIDIOC_CROPCAP, &_cropcap) == 0) {
		_crop.c = _cropcap.defrect;
		_crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;   
		_crop.c.left = 0;   
		_crop.c.top = 0;	
		_crop.c.width = _w;
		_crop.c.height = _h;

		if (-1 == xioctl(_fd, VIDIOC_S_CROP, &_crop)) {
			fprintf(stderr, "set crop error\n");
			return -1;
		}
	} else {   
		/* Errors ignored. */	
		fprintf(stderr, "!! has no ability to crop!!\n");  
		return -1;
	}	

	memset(&_parm, 0, sizeof(_parm));
	_parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	_parm.parm.capture.timeperframe.numerator = 1;
	_parm.parm.capture.timeperframe.denominator = _rate;
	if (xioctl(_fd, VIDIOC_S_PARM, &_parm) == -1)	{
		fprintf(stderr, "set paramter error\n");
		return -1;
	}

	memset(&_parm, 0, sizeof(_parm));
	_parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (xioctl(_fd, VIDIOC_G_PARM, &_parm) == -1){
		fprintf(stderr, "get paramter error\n");
		return -1;
	}
	_rate = _parm.parm.capture.timeperframe.denominator;

	memset(&_fmt, 0, sizeof(_fmt));   
	_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;   
	_fmt.fmt.pix.width = _w;
	_fmt.fmt.pix.height = _h;
	_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_SNX420;
	if (xioctl(_fd, VIDIOC_S_FMT, &_fmt) == -1){
		fprintf(stderr, "set format error\n");
		return -1;
	}
	_ch = (int)_fmt.fmt.pix.priv;
	_frame_sz = _fmt.fmt.pix.height*_fmt.fmt.pix.bytesperline;

	memset(&_req, 0x0, sizeof(_req));
	_req.count = _nbuffers;
	_req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	_req.memory = (enum v4l2_memory)_memtype;
	if (xioctl(_fd, VIDIOC_REQBUFS, &_req) == -1) {	
		fprintf(stderr, "%s request buffer error\n", _name);	 
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

	for (i = 0; i < _nbuffers; i++) {
		struct v4l2_buffer b;
		memset(&b, 0, sizeof(b));
		b.index = i;		
		b.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (xioctl(_fd, VIDIOC_QUERYBUF, &b) == -1){
			fprintf(stderr, "query buffer error\n");
			goto err;
		}
		_frames[i].index = b.index;
		if(_memtype == V4L2_MEMORY_MMAP){
			_frames[i].length = b.length;
			_frames[i].userptr = mmap(NULL, b.length, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, b.m.offset);	 
			if (_frames[i].userptr == MAP_FAILED)   
				goto err;
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


int CVideoCapture::CaptureStart(void)
{
	int i;   
	enum v4l2_buf_type type;   

	if(_memtype == V4L2_MEMORY_MMAP){
		for (i = 0; i < _nbuffers; ++i) {	
			struct v4l2_buffer b;   
			memset(&b, 0, sizeof(b));
			b.index = i;			
			b.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;   
			b.memory = _memtype;
			if (xioctl(_fd, VIDIOC_QBUF, &b) == -1) { 
				fprintf(stderr, "queue buffer error\n");
				return -1;
			}
		}
	}
	
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;   
	if (xioctl(_fd, VIDIOC_STREAMON, &type) == -1){
		fprintf(stderr, "stream on error\n");
		return -1;
	}

	return 0;
}


int CVideoCapture::CaptureStop(void)
{
	enum v4l2_buf_type type;   
	
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;   
	if (-1 == xioctl(_fd, VIDIOC_STREAMOFF, &type)){
		fprintf(stderr, "stream off error\n");
		return -1;
	}

	return 0;
}

int CVideoCapture::DeviceCleanup(void)
{
	int i;
	if(_memtype == V4L2_MEMORY_MMAP){
		for (i = 0; _frames && i < _nbuffers; ++i){
			munmap(_frames[i].userptr, _frames[i].length);
		}
	}
	free((void *)_frames);
	return 0;
}

int CVideoCapture::Run()
{
	CaptureStart();
	return 0;
}

int CVideoCapture::Stop(void)
{
	CaptureStop();
	return 0;
}

snx_frame_ctx_t CVideoCapture::DeFrame()
{
	struct v4l2_buffer buf;
	snx_frame_ctx_t sb;

	memset(&buf, 0, sizeof(buf));   
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = _memtype;
	if(xioctl(_fd, VIDIOC_DQBUF, &buf) == -1) {	
		fprintf(stderr, "dequeue buffer error\n");
		return NULL;
	}
	sb = &_frames[buf.index];
	sb->size = buf.bytesused;
	return sb;
}

int CVideoCapture::EnFrame(snx_frame_ctx_t buffer)
{
	struct v4l2_buffer buf;

	memset(&buf, 0, sizeof(buf));   
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = _memtype;
	buf.index = buffer->index;
	if(_memtype == V4L2_MEMORY_USERPTR){
		buf.m.userptr = (unsigned long)buffer->userptr;
		buf.length = buffer->length;
	}
	if (xioctl(_fd, VIDIOC_QBUF, &buf) == -1){
		fprintf(stderr, "enqueue buffer error\n");
		return -1;
	}	

	return 0;
}

