/**
 * Sonix decoder driver verification example code - normal playback
 *
 * @author Evan Chang
 * @date 01/01/15
 * @version 1.0.0
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <getopt.h>
#include <errno.h>
#include <fcntl.h>
#include <assert.h>
#include <math.h>
//#include <termios.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include <linux/limits.h>
#include <linux/videodev2.h>

#include "snx_common.h"
#include "snx_vc_lib.h"


/*******************************************************************************
 * Defines
 ******************************************************************************/

#define LCD_VERIFY		0
#define _DEBUG_TIME   0
#define USE_FRAME_STEP		0
#define DISABLE_EVENT_LOOP	1//USE_FRAME_STEP
#define MAX_BUFFER_COUNT	3
#define USE_TERMIOS		0
#define UVC_CAM_291B		1

#define DSP_DEV_NAME		"/dev/video3"
#define DEC_DEV_NAME		"/dev/video16"

//initial output resolution
int _x = 0,_y = 0,_sw = 320 ,_sh = 480;
/*******************************************************************************
 * Structure & enumerate declaration
 ******************************************************************************/


enum {
	MODE_PRINT_SCREEN,
	MODE_DUMP_FILE,
	MODE_LCD,
  MODE_DUMP_H264FILE,
};

typedef struct snx_context {
	int abort;
	int step;
	int output_mode;
	FILE *dst_fp;
	struct snx_m2m *m2m;
	pthread_t process_tid;

	int dsp_fd;

	int uvc_fd;
	int uvc_index;
	struct buffer *uvc_cap_buffers;
} snx_context;

/*******************************************************************************
 * Function declaration
 ******************************************************************************/

static int frame_output(snx_context *ctx);
void *mjpeg_process_thread(void *arg);
void event_loop(snx_context *ctx);

/*******************************************************************************
 * Global variables
 ******************************************************************************/
snx_context *g_ctx = NULL;
static struct termios old_term;

static const char short_options[] = "hW:H:c:s:o:B:";

static const struct option long_options[] = {
	{"help", no_argument, NULL, 'h'},
	{"width", required_argument, NULL, 'W'},
	{"height", required_argument, NULL, 'H'},
	{"codec", required_argument, NULL, 'c'},
	{"scale", required_argument, NULL, 's'},
	{"output", required_argument, NULL, 'o'},
	{"buffer", required_argument, NULL, 'B'},

	{NULL, 0, NULL, 0}
};

/*******************************************************************************
 * Function definition
 ******************************************************************************/

static inline int xioctl(int fd, int request, void *arg)
{
	int r;

	do {
		r = ioctl(fd, request, arg);
	} while (r == -1 && EAGAIN == errno);

	return r;
}

void hexdump(char *buf, size_t len)
{
	char *ptr = (char*) buf;
	int i;

	for (i = 0; i < len; ++i) {
		// 16 bytes align
		if (!(i & 0x0F)) printf("\n");

		printf("%02X ", ptr[i]);
	}
	printf("\n\n");
}

static void usage(FILE *fp, int argc, char **argv)
{
	fprintf(fp, "Usage: %s [options] input\n"
		    "Options:\n"
		    "-h Print this message\n"
		    "-W, --width\t\tFrame Width. (default=1280)\n"
		    "-H, --height\t\tFrame Height. (default=720)\n"
		    "-c, --codec\t\tFile input format (default=h264)\n"
		    "-s, --scale\t\tScaling down ratio 1,2,4, for 1/1, 1/2, 1/4. (default=1)\n"
		    "-o, --output filename\n\tOutput file name. (default=uvc.h264)\n\tlcd\n"
		    "-B, --buffer\t\tV4L2 buffer. (default=2)\n\n"

		    "input:\n\tfile path.\n\tuvcvideo\n\n"
		    "", argv[0]);
}
#if USE_TERMIOS
void start_sitio(void)
{
	struct termios tios;

	// get current terminal settings
	tcgetattr(0, &old_term);

	// disable buffered
	setvbuf(stdout, NULL, _IONBF, 0);

	tios = old_term;
	tios.c_lflag &= ~ICANON;
	tios.c_lflag &= ~ECHO;
	tios.c_cc[VMIN] = 1;
	tios.c_cc[VTIME] = 0;
	tcsetattr(0, TCSANOW, &tios);
}

void end_sitio(void)
{
	tcsetattr(0, TCSANOW, &old_term);
}
#endif
static int frame_output(snx_context *ctx)
{
	struct snx_m2m *m2m = NULL;
	int ret;

	if (!ctx || !ctx->m2m) {
		ret = -1;
		goto failed;
	}
	m2m = ctx->m2m;

	if (ctx->output_mode == MODE_PRINT_SCREEN) {
#if _DEBUG_FLOW
		printf("capture buffer, bytes: %d\n", m2m->dec_cap_bytesused);
#endif
		hexdump(m2m->dec_cap_buffers[m2m->dec_cap_index].start, 16/*m2m->dec_cap_bytesused*/);

	} else if (ctx->output_mode == MODE_DUMP_FILE) {

		ret = fwrite(m2m->dec_cap_buffers[m2m->dec_cap_index].start,
				1, m2m->dec_cap_bytesused, ctx->dst_fp);
		if (ret != m2m->dec_cap_bytesused) {
			int sz_not_writen = m2m->dec_cap_bytesused - ret;
			printf("error write buffer ret:%d\n", ret);
			ret = fwrite(m2m->dec_cap_buffers[m2m->dec_cap_index].start + ret,
					1, sz_not_writen, ctx->dst_fp);
			printf("write file final ret:%d\n", ret);
		}

		//ret = buf.bytesused & 0x0F;
		//while (16 - ret) {
		//	putc(0, ctx->dst_fp);
		//	ret++;
		//} // 16 bytes alignment

	} else if (ctx->output_mode == MODE_LCD) {
#if LCD_VERIFY
#else
		struct v4l2_buffer dsp_buf;
#if _DEBUG_TIME
		struct timeval s_tv, e_tv, res_tv;
#endif
#if 0 // TODO: display select is not working, try to add this function later.
		fd_set fds;

		while(1) {
			FD_ZERO(&fds);
			FD_SET(ctx->dsp_fd, &fds);

			printf("before dsp select\n");
			ret = select(ctx->dsp_fd +1, &fds, &fds, NULL, 0);
			if (ret < 0) {
				perror("dsp select");
				ret = -EBUSY;
			} else if (ret == 0) {
				// timeout
				printf("lcd timeout\n");
			}

			if (FD_ISSET(ctx->dsp_fd, &fds))
				break;
		}
		printf("after dsp select\n");
#endif
#if _DEBUG_TIME
		gettimeofday(&s_tv, NULL);
#endif
		CLEAR(dsp_buf);
		dsp_buf.index = m2m->dec_cap_index;
		dsp_buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
		dsp_buf.memory = V4L2_MEMORY_USERPTR;
		dsp_buf.m.userptr = m2m->dec_cap_buffers[dsp_buf.index].start;
		dsp_buf.length = m2m->dec_cap_bytesused;

		ret = ioctl(ctx->dsp_fd, VIDIOC_QBUF, &dsp_buf);
		if (ret != 0)
			perror("dsp qbuf failed");

#if 0
		while(1) {
			FD_ZERO(&fds);
			FD_SET(ctx->dsp_fd, &fds);

			printf("before dsp select\n");
			ret = select(ctx->dsp_fd +1, &fds, &fds, NULL, 0);
			if (ret < 0) {
				perror("dsp select");
				ret = -EBUSY;
			} else if (ret == 0) {
				// timeout
				printf("lcd timeout\n");
			}

			if (FD_ISSET(ctx->dsp_fd, &fds))
				break;
		}
#endif

		CLEAR(dsp_buf);
		dsp_buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
		dsp_buf.memory = V4L2_MEMORY_USERPTR;
		ret = ioctl(ctx->dsp_fd, VIDIOC_DQBUF, &dsp_buf);
		if (ret != 0) {
			perror("dsp dqbuf");
		}
#if _DEBUG_TIME		
		gettimeofday(&e_tv, NULL);
		timersub(&e_tv, &s_tv, &res_tv);
		printf("display- takes %ld ms\n",
				((res_tv.tv_sec*1000)+(res_tv.tv_usec/1000)));
#endif
#endif // LCD_VERIFY
	} else {
		fprintf(stderr, "Unkown output mode: %d\n", ctx->output_mode);
	}

failed:
	return ret;
}

// check JFIF in detail
void *mjpeg_process_thread(void *arg)
{
	snx_context *ctx = (snx_context*) arg;
	struct snx_m2m *m2m = ctx->m2m;
	int frame_sz;
	int ret, frame_count = 0;
	int fileid = 0;
  struct timeval s1_tv, e1_tv, res1_tv;
	// frame processing
	for (;;) {
		struct v4l2_buffer buf = {0};
		char *raw_buf = NULL;
		size_t file_sz = 0;

		if (ctx->abort)
			break;



		// read frame
			fd_set read_fds;
			FD_ZERO(&read_fds);
			FD_SET(ctx->uvc_fd, &read_fds);

			ret = select(ctx->uvc_fd +1, &read_fds, NULL, NULL, 0);
			if (ret < 0) {
				perror("uvc select");
				return -1;
			}

			CLEAR(buf);
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_MMAP;
			ret = ioctl(ctx->uvc_fd, VIDIOC_DQBUF, &buf);
			if (ret < 0) {
				perror("uvc de-queue buffer failed");
				return -1;
			}
			ctx->uvc_index = buf.index;
#if _DEBUG_TIME       
			printf("frame %d, size %u\n", frame_count, buf.bytesused);
#endif      
//			hexdump(ctx->uvc_cap_buffers[ctx->uvc_index].start, 16);
			if (buf.bytesused > 16) {
//				hexdump(ctx->uvc_cap_buffers[ctx->uvc_index].start
//					+ buf.bytesused - 16, 16);
			} else {
				// special case, remove me.
				printf("### drop frame#%d ###\n", frame_count);
				frame_count++;

				CLEAR(buf);
				buf.index = ctx->uvc_index;
				buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				buf.memory = V4L2_MEMORY_MMAP;
				ret = ioctl(ctx->uvc_fd, VIDIOC_QBUF, &buf);
				if (ret < 0) {
					perror("uvc queue buffer failed");
					return -1;
				}
				continue;//exit(EXIT_FAILURE);
			}
      
      // save file
      if (buf.bytesused & 0x0000000FL) {
			size_t concat_bytes = 0x10L - (buf.bytesused & 0x0000000FL);
			memset(ctx->uvc_cap_buffers[ctx->uvc_index].start+buf.bytesused, 0, concat_bytes);
			buf.bytesused += concat_bytes;
		}
      fwrite(ctx->uvc_cap_buffers[ctx->uvc_index].start,
				1, buf.bytesused, ctx->dst_fp);
			CLEAR(buf);
			buf.index = ctx->uvc_index;
			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_MMAP;
			ret = ioctl(ctx->uvc_fd, VIDIOC_QBUF, &buf);
			if (ret < 0) {
				perror("uvc queue buffer failed");
				return -1;
			}

	}


failed:
	pthread_exit(NULL);
}



void event_loop(snx_context *ctx)
{
	fd_set read_fds;
	int ret;

	while(1) {
		struct timeval time_out;
		int cmd;

		if (ctx->abort) {
			printf("exit now\n");
			break;
		}

		FD_ZERO(&read_fds);
		FD_SET(STDIN_FILENO, &read_fds);

		time_out.tv_sec = 1;
		time_out.tv_usec = 0;

		ret = select(STDIN_FILENO + 1, &read_fds, NULL, NULL, &time_out);
		if (ret == 0) { // timeout
			continue;
		} else if (ret < 0) {
			perror("stdin select\n");
			ctx->abort = 1;
			break;
		}

		//cmd = fgetc(stdin);
		cmd = getchar();
		switch(cmd) {
			case 'q':
				ctx->abort = 1;
				printf("snx_dec_pb quit\n");
				return;
			case ' ':
				// handled step by step
				break;
			default:
				printf("Unknown command %c\n", cmd);
				break;
		}
	}
}

static int uvc_open(struct snx_context *ctx, const char *drvname)
{
#if 0
	struct v4l2_capability cap;
	int fd, ret;

	fd = open(devname, O_RDWR);
	if (fd < 0) {
		perror("uvc open failed");
		return -1;
	}

	CLEAR(cap);
	ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);
	if (ret < 0) {
		perror("uvc querycap failed");
		return ret;
	}

	ctx->uvc_fd = fd;

	printf("Device %s opened: %s.\n", devname, cap.card);
	return ret;
#else
	struct snx_m2m *m2m = ctx->m2m;
	struct v4l2_capability cap;
	char devname[16];
	int fd, i;
	int skip4snx291b = 0;

#if UVC_CAM_291B
	if (m2m->dec_fmt == V4L2_PIX_FMT_H264) {
		skip4snx291b = 1;
	}
#endif

	for (i = 0; i < 255; ++i) {
		memset(devname, 0, 16);
		sprintf(devname, "/dev/video%d", i);
		printf("open device %s\n", devname);
		fd = open(devname, O_RDWR, 0);
		if (fd < 0) {
			continue;
		}

		if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
			fclose(fd);
			continue;
		}
		printf("driver:\t%s\n", cap.driver);
		printf("card:\t%s\n", cap.card);
		printf("bus info:\t%s\n", cap.bus_info);

		if (strstr((const char*) cap.driver, drvname)) {
			if (skip4snx291b) {
				skip4snx291b--;
				goto close_291B_input0;
			}
			break;
		}
close_291B_input0:
		close(fd);
	}

	if (i == 255) {
		printf("uvc open failed: no %s device\n", drvname);
		return -1;
	}

	// blocking (in uvc don't use non-blocking)
	i = fcntl(fd, F_GETFL, 0);
	i &= ~O_NONBLOCK;
	if (fcntl(fd, F_SETFL, i) < 0) {
		close(fd);
		return -1;
	}

	ctx->uvc_fd = fd;
	printf("Device %s opened: %s.\n", devname, cap.card);
	return 0;
#endif
}

static int uvc_init(snx_context *ctx)
{
	struct snx_m2m *m2m = ctx->m2m;
	struct v4l2_input input;
	struct v4l2_format fmt;
	struct v4l2_streamparm parm;
	struct v4l2_requestbuffers req;
	struct v4l2_buffer buf;
	int fd = ctx->uvc_fd, type;
	int ret, i;
	__u32 _input = 0;

	// enum inputs
	for (i = 0; ; ++i) {
		CLEAR(input);
		input.index = i;
		ret = ioctl(fd, VIDIOC_ENUMINPUT, &input);
		if (ret < 0) {
			printf("uvc enuminput %u break.\n", input.index);
			break;
		}

		if (i != input.index)
			printf("Warning: driver returned wrong input index %u.\n", input.index);
		printf("input %u: %s.\n", i, input.name);
	}

	// set input
	ret = ioctl(fd, VIDIOC_S_INPUT, &_input);
	if (ret < 0) {
		perror("uvc s_input failed");
		return ret;
	}

	CLEAR(fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = m2m->width;
	fmt.fmt.pix.height = m2m->height;
	fmt.fmt.pix.pixelformat = m2m->dec_fmt;
	fmt.fmt.pix.field = V4L2_FIELD_ANY;

	ret = ioctl(fd, VIDIOC_S_FMT, &fmt);
	if (ret < 0) {
		perror("uvc s_fmt failed");
		return ret;
	}

	CLEAR(parm);
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ioctl(fd, VIDIOC_G_PARM, &parm);
	if (ret < 0) {
		perror("uvc g_parm failed");
		return ret;
	}

	parm.parm.capture.timeperframe.numerator = 1;
	parm.parm.capture.timeperframe.denominator = 30;
	ret = ioctl(fd, VIDIOC_S_PARM, &parm);
	if (ret < 0) {
		perror("uve s_parm failed");
		return ret;
	}

	CLEAR(req);
	req.count = 2;
  
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	ret = ioctl(fd, VIDIOC_REQBUFS, &req);
	if (ret < 0) {
		perror("uvc reqbufs failed");
		return ret;
	}
	printf("uvc: %u buffers allocated.\n", req.count);

	ctx->uvc_cap_buffers = calloc(req.count, sizeof(struct buffer));
	for (i = 0; i < req.count; ++i) {
		CLEAR(buf);
		buf.index = i;
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		ret = ioctl(fd, VIDIOC_QUERYBUF, &buf);
		if (ret < 0) {
			perror("uvc querybuf failed");
			return ret;
		}
		printf("length %u offset: %u\n", buf.length, buf.m.offset);

		// mmap for PROT_READ|WRITE
		ctx->uvc_cap_buffers[i].length = buf.length;
		ctx->uvc_cap_buffers[i].start = mmap(NULL, buf.length,
				PROT_READ|PROT_WRITE, MAP_SHARED,
				fd, buf.m.offset);
		if (ctx->uvc_cap_buffers[i].start == MAP_FAILED) {
			perror("uvc mmap failed");
			return -1;
		}
	}

	for (i = 0; i < req.count; ++i) {
		CLEAR(buf);
		buf.index = i;
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		ret = ioctl(fd, VIDIOC_QBUF, &buf);
		if (ret < 0) {
			perror("uvc qbuf failed");
			return ret;
		}
	}

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ioctl(fd, VIDIOC_STREAMON, &type);
	if (ret < 0) {
		perror("uvc streamon failed");
		return ret;
	}

	ctx->uvc_fd = fd;
	ctx->uvc_index = 0;
	return ret;
}

static int dsp_init(snx_context *ctx)
{
	struct v4l2_capability cap;
	struct v4l2_format fmt;
	struct v4l2_cropcap cropcap;
	struct snx_m2m *m2m = ctx->m2m;
  struct v4l2_crop _crop;  
	int ret;

	ctx->dsp_fd = snx_open_device(DSP_DEV_NAME);

	CLEAR(cap);
	ret = ioctl(ctx->dsp_fd, VIDIOC_QUERYCAP, &cap);
	if (ret != 0) {
		perror("ioctl querycap");
		goto failed;
	}

	CLEAR(cropcap);
	ret = ioctl(ctx->dsp_fd, VIDIOC_CROPCAP, &cropcap);
	if (ret != 0) {
		perror("ioctl cropcap");
		goto failed;
	}
  	_crop.c = cropcap.defrect;
		_crop.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY;   
		_crop.c.left = _x;   
		_crop.c.top = _y;
		_crop.c.width = _sw;
		_crop.c.height = _sh;

		if (-1 == xioctl(ctx->dsp_fd, VIDIOC_S_CROP, &_crop)) {
			fprintf(stderr, "set crop error\n");
			return -1;
		}
  

	CLEAR(fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	fmt.fmt.pix.width = m2m->width / m2m->scale;
	fmt.fmt.pix.height = m2m->height / m2m->scale;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_SNX420;
	fmt.fmt.pix.field = V4L2_FIELD_ANY;
	ret = ioctl(ctx->dsp_fd, VIDIOC_S_FMT, &fmt);
	if (ret != 0) {
		perror("ioctl output set format");
		goto failed;
	}

	printf ("w: %d, h: %d, scale: 1/%d\n", m2m->width, m2m->height, m2m->scale);
#if 0
	dev_num = atoi(strtok(m2m->codec_dev,"/dev/video"));
	if(m2m->codec_fmt == V4L2_PIX_FMT_H264)
		ds_fmt='h';	
	else if (m2m->codec_fmt == V4L2_PIX_FMT_MJPEG)
		ds_fmt='j';
	if(m2m->scale == 1)
		ds_scale=0x0;
	else
		ds_scale='s';
	sprintf(m2m->ds_dev_name, "%d_%c%c",dev_num,ds_fmt,ds_scale);
#endif
failed:
	return ret;
}

static int dsp_start(snx_context *ctx)
{
	struct v4l2_buffer buf;
	struct v4l2_requestbuffers req;
	enum v4l2_buf_type type;
	int ret, i;
	struct snx_m2m *m2m = ctx->m2m;

	CLEAR(req);
	// reqbuf
	req.count = m2m->m2m_buffers;
	req.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
#if LCD_VERIFY
	req.memory = V4L2_MEMORY_MMAP;
#else
	req.memory = V4L2_MEMORY_USERPTR;
#endif
	ret = ioctl(ctx->dsp_fd, VIDIOC_REQBUFS, &req);
	if (ret != 0) {
		perror("ioctl output VIDIOC_REQBUFS");
		goto failed;
	}

	// NOTICE: mapping request buffer result from driver.
	if (req.count != m2m->m2m_buffers)
		m2m->m2m_buffers = req.count;

	CLEAR(buf);
	// out querybuf
	buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
#if LCD_VERIFY
	req.memory = V4L2_MEMORY_MMAP;
	ctx->out_buffers = calloc(ctx->buf_count, sizeof(*ctx->out_buffers));
#else
	req.memory = V4L2_MEMORY_USERPTR;
#endif
	for (i = 0; i < m2m->m2m_buffers; ++i) {
		buf.index = i;
		ret = ioctl(ctx->dsp_fd, VIDIOC_QUERYBUF, &buf);
		if (ret != 0) {
			perror("ioctl output querybuf");
			goto failed;
		}
#if LCD_VERIFY
		ctx->out_buffers[i].length = buf.length;
		ctx->out_buffers[i].start = mmap(NULL, buf.length,
				PROT_READ | PROT_WRITE, MAP_SHARED,
				ctx->dsp_fd, buf.m.offset);
		if (ctx->out_buffers[i].start == MAP_FAILED) {
			perror("mmap output querybuf");
			goto failed;
		}
#endif
	}

	// streamon
	type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	ret = ioctl(ctx->dsp_fd, VIDIOC_STREAMON, &type);
	if (ret != 0) {
		perror("ioctl");
		goto failed;
	}

failed:
	return ret;
}

#if LCD_VERIFY
static void pattern_gen(SnxContext *ctx,
		uint8_t red, uint8_t green, uint8_t blue, const char *dst)
{
	uint32_t w = ctx->width, h = ctx->height;
	uint32_t buf_sz = w * h * 3 / 2;
	char *p = dst;
	int pix, line, pitch = 2 * w;
	uint8_t y, u, v, r, g, b;
	uint32_t y_sz = w * h, uv_sz = uv_sz / 4;
	r = red;
	g = green;
	b = blue;
	//y = 0.299 * r + 0.587 * g + 0.114 * b;
	//u = -0.147 * r - 0.289 * g + 0.436 * b;
	//v = 0.615 * r - 0.515 * g - 0.100 * b;
	y = 0.299 * r + 0.587 * g + 0.114 * b;
	u = -0.169 * r - 0.331 * g + 0.499 * b + 128;
	v = 0.499 * r - 0.418 * g - 0.0813 * b + 128;

	for (line = 0; line < h; ++line) {
		if (line & 0x1) {
			for ( pix = 0; pix < (w/4); ++pix) {
				*p = y; p++;
				*p = y; p++;
				*p = y; p++;
				*p = y; p++;
			}
		} else {
			for ( pix = 0; pix < (pitch/8); ++pix) {
				*p = y; p++;
				*p = y; p++;
				*p = y; p++;
				*p = y; p++;
				*p = u; p++;
				*p = v; p++;
				*p = u; p++;
				*p = v; p++;
			}
		}
	}
}

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} snx_colour;

static int lcd_verify(void)
{
	SnxContext *ctx;
	struct v4l2_buffer buf;
	snx_colour colour[3] = {0};
	int ret, i = 0;

	ctx = calloc(1, sizeof(*ctx));
	if (!ctx) {
		perror("alloc ctx");
		return -1;
	}
	ctx->width = 480;
	ctx->height = 320;
	ctx->codec = 1; // mjpeg
	ctx->scale_ratio = 1;
	ctx->buf_count = 2;

	// set pattern
	colour[0].r = 255;
	colour[1].g = 255;
	colour[2].b = 255;

	ret = dsp_init(ctx);
	if (ret < 0) {
		printf("init vo failed\n");
		return -1;
	}

	ret = dsp_start(ctx);
	if (ret < 0) {
		printf("start vo failed\n");
		return -1;
	}

	while (1) {
		pattern_gen(ctx, colour[i].r, colour[i].g, colour[i].b,
				ctx->out_buffers[0].start);
		buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = 0;
		buf.bytesused = ctx->width * ctx->height * 3 / 2;

		ret = ioctl(ctx->dsp_fd, VIDIOC_QBUF, &buf);
		if (ret < 0) {
			perror("vo QBUF");
		}

		ret = ioctl(ctx->dsp_fd, VIDIOC_DQBUF, &buf);
		if (ret < 0) {
			perror("vo DQBUF");
		}

		i++;
		if (i == 3) i = 0;
		sleep(1);
	}

	close(ctx->dsp_fd);
	return 0;
}
#endif

void sigterm_fn(int sig)
{
  struct snx_context *ctx = g_ctx;
  if (!ctx) {
    printf("no global context exsit\n");
    return -1;
  }
  printf("###### exiting ######\n");
  ctx->abort = 1;
  return;
}

int main(int argc, char **argv)
{
	snx_context *ctx = NULL;
	struct snx_m2m *m2m = NULL;

	int w = 1280, h = 720, codec = 0; // 0:h264, 1:mjpeg
	int scale_ratio = 1, buf_count = 2;
	char src_file[PATH_MAX] = {0};
	char dst_file[PATH_MAX] = "test.yuv";
	int ret = -1;
#if 1
  struct sigaction sa;
  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_handler = sigterm_fn;
  if(sigaction(SIGTERM, &sa, NULL)) {
    printf("Failed to set SIGTERM handler. EXITING");
    return 0;
  }

#endif  
#if LCD_VERIFY
	lcd_verify();
#endif

	if (argc < 2) {
		usage(stderr, argc, argv);
		exit(EXIT_FAILURE);
	}

	while(1) {
		int c, index;
		c = getopt_long(argc, argv, short_options, long_options, &index);
		if (-1 == c) break;

		switch(c)
		{
			case 'h':
				usage(stdout, argc, argv);
				exit(EXIT_SUCCESS);
			case 'W':
				sscanf(optarg, "%d", &w);
				break;
			case 'H':
				sscanf(optarg, "%d", &h);
				break;
			case 'c':
				codec = (!strncmp(optarg, "jpeg", 4));
				break;
			case 's':
				sscanf(optarg, "%d", &scale_ratio);
				break;
			case 'o':
				memset(dst_file, 0, sizeof(dst_file));
				strcpy(dst_file, optarg);
				break;
			case 'B':
				sscanf(optarg, "%d", &buf_count);
				break;

				break;           
			default:
				usage(stderr, argc, argv);
				exit(EXIT_FAILURE);
		}
	}
	strncpy(src_file, argv[optind], strlen(argv[optind]));

#if USE_TERMIOS
	start_sitio();
#endif

	// snx_context allocate and initialize
	ctx = (snx_context*) calloc(1, sizeof(snx_context));
	if (!ctx) {
		printf("create snx_context failed\n");
		exit(EXIT_FAILURE);
	}
	ctx->abort = 0;
	ctx->step = USE_FRAME_STEP;
	if (strncmp(dst_file, "lcd", 3) == 0) {
		ctx->output_mode = MODE_LCD;
	} else {
		ctx->output_mode = MODE_DUMP_H264FILE;
	}
	//ctx->output_mode = MODE_DUMP_FILE; //MODE_PRINT_SCREEN; //MODE_DUMP_FILE; //MODE_LCD;
  g_ctx = ctx;
	// snx_m2m allocate and initialize
	m2m = (struct snx_m2m*) calloc(1, sizeof(struct snx_m2m));
	if (!m2m) {
		printf("create snx_m2m failed\n");
		goto failed;
	}
	m2m->m2m = 1;
	m2m->width = w;
	m2m->height = h;
	m2m->scale = scale_ratio;
	m2m->m2m_buffers = fmin(MAX_BUFFER_COUNT, buf_count);
	m2m->dec_fmt = codec? V4L2_PIX_FMT_MJPEG: V4L2_PIX_FMT_H264;
	//m2m->dec_out_mem = V4L2_MEMORY_MMAP;
	//m2m->dec_cap_mem = V4L2_MEMORY_MMAP;
	strcpy(m2m->dec_dev, DEC_DEV_NAME);

	// for overlay-mixer
	m2m->isp_fps = 30;
	m2m->isp_mem = V4L2_MEMORY_MMAP;
	m2m->isp_fmt = V4L2_PIX_FMT_SNX420;
	m2m->codec_fps = 30;
	m2m->codec_fmt = V4L2_PIX_FMT_H264;
	m2m->out_mem = V4L2_MEMORY_USERPTR;
	m2m->ds_font_num = 128;
	m2m->bit_rate = 1048576;
	strcpy(m2m->isp_dev, "/dev/video0");
	strcpy(m2m->codec_dev, "/dev/video1");
	ctx->m2m = m2m;
	printf("Setting w:%d h:%d codec:%d scale:%d buf:%d\n",
			m2m->width, m2m->height, codec, m2m->scale, m2m->m2m_buffers);

#if 1
	// output file open
	if ((ctx->output_mode == MODE_DUMP_FILE || ctx->output_mode == MODE_DUMP_H264FILE)  && strlen(dst_file) != 0) {
		ctx->dst_fp = fopen(dst_file, "w+b");
		if (!ctx->dst_fp) {
			ret = -1;
			perror("open output file failed");
			//printf("open output file '%s' failed\n", dst_file);
			goto failed;
		}
	}

	// initial modules
	if (strncmp(src_file, "uvcvideo", 8) == 0) {
		ret = uvc_open(ctx, src_file);
		if (ret < 0) {
			printf("uvc open failed\n");
			exit(EXIT_FAILURE);
		}

		ret = uvc_init(ctx);
		if (ret < 0) {
			printf("uvc init failed\n");
			exit(EXIT_FAILURE);
		}
   }
#endif


#if 1
	pthread_create(&ctx->process_tid, NULL, mjpeg_process_thread, (void*)ctx);
#else
	pthread_create(&ctx->process_tid, NULL, overlay_thread, (void*)ctx);
#endif

#if DISABLE_EVENT_LOOP
#else
	event_loop(ctx);
#endif

	pthread_join(ctx->process_tid, NULL);


failed:
	if (ctx->m2m) free(ctx->m2m);
	if (ctx->dst_fp) fclose(ctx->dst_fp);
	if (ctx) free(ctx);

#if USE_TERMIOS
	end_sitio();
#endif

	return ret;
}
