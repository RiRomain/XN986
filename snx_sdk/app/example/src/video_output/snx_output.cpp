
#include "tv.h"
#include "output.h"
#include "capture.h"
CVideoOutput::CVideoOutput(int ch, int x, int y, int w, int h, int sw, int sh, enum v4l2_memory iotype, int nbuffers):
							_ch(ch),
							_x(x),
							_y(y),
							_w(w),
							_h(h),
							_sw(sw),
							_sh(sh),
							_memtype(iotype),
							_nbuffers(nbuffers)
{
}

CVideoOutput::~CVideoOutput()
{
}

int CVideoOutput::Open()
{
	int i, ret;
	for(i = 0; i < 255; i++){
    /* Open /dev/video2 and check struct v4l2_capability member capabilities 
      support V4L2_CAP_VIDEO_OUTPUT &  V4L2_CAP_STREAMING and 
      struct v4l2_capability member driver if match ¡§vo¡¨ */
		ret = detect_video_device(V4L2_CAP_VIDEO_OUTPUT, "vo", i);
		if(!ret)
			break;
	}
	if(i == 255){
		goto err;
	}
  printf ("vo i=%x\n",i);
	sprintf(_name, "/dev/video%d", i);
  /* open /dev/video2 & type O_RDWR | O_NONBLOCK */
	_fd =  open(_name, O_RDWR | O_NONBLOCK, 0);	 
	if (_fd == -1){
		goto err;
	}
	/* do Video Output device Initilize */		
	if(DeviceInitilize() <0)
		goto err;

	return 0;
err:
	return -1;
}

int CVideoOutput::Close(void)
{
  /* do Video Output device clean up */
	DeviceCleanup();
	close(_fd);
	return 0;
}

int CVideoOutput::DeviceInitilize(void)
{
	int i, err, tmp;

	/* set block mode */
	tmp = fcntl(_fd, F_GETFL, 0);
	tmp &= ~O_NONBLOCK;
	err = fcntl(_fd, F_SETFL, tmp);
	if(err < 0)
		return -1;

	i = 0;
  /* set ioctl VIDIOC_S_OUTPUT  */
	ioctl (_fd, VIDIOC_S_OUTPUT, &i);
  /* get  v4l2_capability _cap */
	if (xioctl(_fd, VIDIOC_QUERYCAP, &_cap) == -1) {
		fprintf(stderr, "%s is no V4L2 device\n", _name);	
		return -1;
	}	
  /* check support V4L2_CAP_VIDEO_OUTPUT */ 
	if (!(_cap.capabilities & V4L2_CAP_VIDEO_OUTPUT)) {   
		fprintf(stderr, "%s is no video capture device\n", _name);	
		return -1;   
	}	
  /* check support V4L2_CAP_STREAMING */ 
	if (!(_cap.capabilities & V4L2_CAP_STREAMING)) {   
		fprintf(stderr, "%s does not support streaming i/o\n", _name);	
		return -1;
	}

	memset(&_cropcap, 0, sizeof(_cropcap));	
	_cropcap.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY;   
	if (xioctl(_fd, VIDIOC_CROPCAP, &_cropcap) == 0) {
     
		_crop.c = _cropcap.defrect;
		_crop.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY;   
		_crop.c.left = _x;   
		_crop.c.top = _y;
		_crop.c.width = _sw;
		_crop.c.height = _sh;
    /* set ioctl VIDIOC_S_CROP, parameter v4l2_crop crop */
		if (-1 == xioctl(_fd, VIDIOC_S_CROP, &_crop)) {
			fprintf(stderr, "set crop error\n");
			return -1;
		}
	} else {
		fprintf(stderr, "cropcap error\n");
		return -1;
	}

	memset(&_fmt, 0, sizeof(_fmt));
  
	_fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	_fmt.fmt.pix.width = _w;
	_fmt.fmt.pix.height = _h;
	_fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_SNX420;
	_fmt.fmt.pix.field = (enum v4l2_field)_ch;
  /* set ioctl VIDIOC_S_FMT  parameter v4l2_format _fmt  */
	if (xioctl(_fd, VIDIOC_S_FMT, &_fmt) == -1){
		fprintf(stderr, "set format error\n");
		return -1;
	}

	_frame_sz = _fmt.fmt.pix.height*_fmt.fmt.pix.bytesperline;

	memset(&_req, 0x0, sizeof(_req));
  /* set v4l2_requestbuffers parameter */
	_req.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	_req.count = _nbuffers;	
	_req.memory = _memtype;
	if (xioctl(_fd, VIDIOC_REQBUFS, &_req) == -1){
		fprintf(stderr, "request buffer error\n");
		return -1;
	}

	if (_req.count < 2){
		fprintf(stderr, "Insufficient buffer memory on %s\n", _name);   
		return -1;
	}
	_nbuffers = _req.count;
  /* alloc memory */
	_frames = (snx_frame_ctx_t)calloc(_nbuffers, sizeof(*_frames));	 
	if (!_frames) {   
		fprintf(stderr, "Out of memory\n");   
		return -1; 
	}

	for (i = 0; i < _nbuffers; i++) {
		struct v4l2_buffer b;
		memset(&b, 0, sizeof(b)); 
    /* set v4l2_buffer parameter */
		b.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
		b.memory = _memtype;	 
		b.index = i;
		b.field = (enum v4l2_field)_ch;
		if (xioctl(_fd, VIDIOC_QUERYBUF, &b) == -1){
			fprintf(stderr, "query buffer error\n");
			goto err;
		}
		_frames[i].index = b.index;
		if(_memtype == V4L2_MEMORY_MMAP){
			_frames[i].length = b.length;
      /* do mmap */
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

int CVideoOutput::CaptureStart(void)
{
	int i;   
	enum v4l2_buf_type type;   
	if(_memtype == V4L2_MEMORY_MMAP){
		for (i = 0; i < _nbuffers; ++i) {
			struct v4l2_buffer b;   
			memset(&b, 0, sizeof(b)); 
      /* set v4l2_buffer parameter */  
			b.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;   
			b.memory = _memtype;
			b.index = i;
			b.field = (enum v4l2_field)_ch;
			if (xioctl(_fd, VIDIOC_QBUF, &b) == -1){
				fprintf(stderr, "queue buffer error\n");
				return -1;
			}
		}
	}	
	type = V4L2_BUF_TYPE_VIDEO_OUTPUT;  
  /* set ioctl VIDIOC_STREAMON type V4L2_BUF_TYPE_VIDEO_OUTPUT */  
	if (xioctl(_fd, VIDIOC_STREAMON, &type) == -1){	
		fprintf(stderr, "stream on error\n");
		return -1;
	}
	return 0;
}

int CVideoOutput::CaptureStop(void)
{
	enum v4l2_buf_type type;   
	
	type = V4L2_BUF_TYPE_VIDEO_OUTPUT;  
  /* set ioctl VIDIOC_STREAMOFF */ 
	if (-1 == xioctl(_fd, VIDIOC_STREAMOFF, &type)){ 
		fprintf(stderr, "stream off error\n");
		return -1;
	}

	return 0;
}

int CVideoOutput::DeviceCleanup(void)
{
	int i;
	if(_memtype == V4L2_MEMORY_MMAP){
    /* if use V4L2_MEMORY_MMAP,must munmap */ 
		for (i = 0; _frames && i < _nbuffers; ++i){
			munmap(_frames[i].userptr, _frames[i].length);
		}
	}                                                  
	free(_frames);
	return 0;
}

int CVideoOutput::Run()
{
	CaptureStart();
	return 0;
}

int CVideoOutput::Stop(void)
{
	CaptureStop();
	return 0;
}

snx_frame_ctx_t CVideoOutput::DeFrame()
{
	struct v4l2_buffer buf;
	snx_frame_ctx_t sb;

	memset(&buf, 0, sizeof(buf));   
	buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	buf.memory = _memtype;
	buf.field = (enum v4l2_field)_ch;
  /* set ioctl  VIDIOC_DQBUF, parameter v4l2_buffer buf */
	if(xioctl(_fd, VIDIOC_DQBUF, &buf) == -1) {
		fprintf(stderr, "dequeue buffer error\n");	
		return NULL;
	}
	sb = &_frames[buf.index];
	sb->size = buf.bytesused;
	return sb;
}

int CVideoOutput::EnFrame(snx_frame_ctx_t buffer)
{
	struct v4l2_buffer buf;

	memset(&buf, 0, sizeof(buf));   
	buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	buf.memory = _memtype;
	buf.index = buffer->index;
	buf.field = (enum v4l2_field)_ch;
	if(_memtype == V4L2_MEMORY_USERPTR){
		buf.m.userptr = (unsigned long)buffer->userptr;
		buf.length = buffer->length;
	}
  /* set ioctl  VIDIOC_QBUF, parameter v4l2_buffer buf */
	if (xioctl(_fd, VIDIOC_QBUF, &buf) == -1){
		fprintf(stderr, "dequeue buffer error\n");
		return -1;
	}

	return 0;
}


