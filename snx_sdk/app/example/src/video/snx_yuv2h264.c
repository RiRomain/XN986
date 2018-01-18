/**
 * Sonix decoder driver verification example code - normal playback
 *
 * @author Evan Chang
 * @date 04/10/15
 * @version 1.0.0
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <getopt.h>
#include <errno.h>
#include <fcntl.h>
#include <assert.h>
#include <math.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include <linux/limits.h>
#include <linux/videodev2.h>

#include "snx_common.h"
#include "snx_vc_lib.h"
#include "snx_rc_lib.h"
#include "save_media.h"

/*******************************************************************************
 * Defines
 ******************************************************************************/

#define _DEBUG_TIME				0
#define _DEBUG_FLOW				0
#define MAX_BUFFER_COUNT		3

#define ENC_DEV_NAME			"/dev/video1"
#define DEC_DEV_NAME			"/dev/video16"

#define Y4M_MAGIC				"YUV4MPEG2"
#define MAX_YUV4_HEADER			80
#define Y4M_FRAME_MAGIC			"FRAME"
#define MAX_YUV4_FRAME_HEADER	80


#define RECORD_CONFIG       	"/etc/EXAMPLE/yuv2h264.xml"


/*******************************************************************************
 * Structure & enumerate declaration
 ******************************************************************************/

// Used in debug decoder output
enum {
	NONE,
	PROGRESSIVE,
	TOP_FIELD_FIRST,
	BOT_FIELD_FIRST,
	MIXED
};

enum {
	UNKNOWN,
	SQUARE,
	SVCD,
	DVD_NARROW,
	DVD_WIDE,
	DV_PAL
};

enum {
	YUV420JPEG, // 4:2:0 with biaxially-displaced chroma planes
	YUV420PALDV, // 4:2:0 with vertically-displaced chroma planes
	YUV420,
	YUV422,
	YUV444,
};

typedef struct {
	char *frame_data;
	char *y_plane;
	char *u_plane;
	char *v_plane;
	size_t frame_sz, y_sz, uv_sz;
} yuv_plane;

typedef struct {
	unsigned width;
	unsigned height;
	float framerate;
	unsigned interlace;
	unsigned aspect_ratio;
	unsigned colour_space;
	yuv_plane plane;
	int header_found;
} y4m_header_param;


enum {
	MODE_PRINT_SCREEN,
	MODE_DUMP_FILE,
	MODE_PACK_FILE,
	MODE_LCD,
};

typedef struct snx_context {
	int abort;
	int step;
	int output_mode;
	int frames;
	FILE *src_fp, *dst_fp;
	struct snx_m2m *m2m;
	pthread_t process_tid;
	y4m_header_param y4m;
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


static const char short_options[] = "hf:o:W:H:r:s:b:p";

static const struct option long_options[] = {
	{"help", no_argument, NULL, 'h'},
	{"frames", required_argument, NULL, 'f'},
	{"width", required_argument, NULL, 'W'},
	{"height", required_argument, NULL, 'H'},
	{"framerate", required_argument, NULL, 'r'},
	{"scale", required_argument, NULL, 's'},
	{"bitrate", required_argument, NULL, 'b'},
	{"output", required_argument, NULL, 'o'},
	{"package", no_argument, NULL, 'p'},
	{NULL, 0, NULL, 0}
};

save_media_source *sd_record = NULL;

/*******************************************************************************
 * Y4M Function definition
 ******************************************************************************/

static int parse_colour_space(char *buf, int *depth)
{
	int colour_space;

	if (depth) *depth = 8;

	if (!strncmp("420", buf, 3)) {
		char *ext = buf + 3;
		if (!strncmp("jpeg", ext, 4))
			colour_space = YUV420JPEG;
		else if (!strncmp("paldv", ext, 5))
			colour_space = YUV420PALDV;
		else
			colour_space = YUV420;
	}
	else if (!strncmp("422", buf, 3))
		colour_space = YUV422;
	else if (!strncmp("444", buf, 3))
		colour_space = YUV444;

	return colour_space;
}

/**
 * y4m module helper functions
 *
 * int open_file(snx_context *, const char *);
 * int read_frame(snx_context *, yuv_plane *);
 * int close_file(snx_context *);
 */
static int open_file(snx_context *ctx, const char *filename)
{
	y4m_header_param *y4m = &ctx->y4m;
	char header[MAX_YUV4_HEADER];
	char *token_start, *token_end, *header_end;
	int header_length=0, frame_length=0, plane_length=0, num_frames=0;
	uint32_t numerator, denominator;
	uint64_t init_pos, file_length;
	int i;

	ctx->src_fp = fopen(filename, "rb");
	if (!ctx->src_fp) {
		printf("cannot open file: %s\n", filename);
		return -1;
	}

	// read header
	memset(header, 0, sizeof(header));
	for (i = 0; i < MAX_YUV4_HEADER; ++i) {
		header[i] = fgetc(ctx->src_fp);
		if (header[i] == 0x0a)
			break;
	}
	if (i == MAX_YUV4_HEADER || strncmp(header, Y4M_MAGIC, strlen(Y4M_MAGIC))) {
		printf("NO Y4M header found, use parameter setting.\n");
		goto PARAM_SETTING;
	}

	y4m->header_found = 1;

	// parsing header
	header_end = &header[i+1];
	header_length = i+1;
	token_start = &header[strlen(Y4M_MAGIC)+1];

	while(token_start < header_end) {

		if (*token_start == 0x20)
			continue;

		switch (*token_start++) {
			case 'W': // Width
				y4m->width = strtol(token_start, &token_end, 10);
				token_start = token_end;
				break;
			case 'H': // Height
				y4m->height = strtol(token_start, &token_end, 10);
				token_start = token_end;
				break;
			case 'F': // Frame per second
				if (sscanf(token_start, "%u:%u", &numerator, &denominator) == 2)
					y4m->framerate = (float)numerator/denominator;
				token_start = strchr(token_start, 0x20);
				break;
			case 'I': // Interlacing
				switch(*token_start++) {
					case 'p':
						y4m->interlace = PROGRESSIVE;
						break;
					case 't':
						y4m->interlace = TOP_FIELD_FIRST;
						break;
					case 'b':
						y4m->interlace = BOT_FIELD_FIRST;
						break;
					case 'm':
						y4m->interlace = MIXED;
						break;
				}
				break;
			case 'A': // Aspect ratio
				if (sscanf(token_start, "%u:%u", &numerator, &denominator) == 2) {
					if (numerator == 0)
						y4m->aspect_ratio = UNKNOWN;
					else if (numerator == 1)
						y4m->aspect_ratio = SQUARE;
					else if (numerator == 4) {
						if (denominator == 3)
							y4m->aspect_ratio = SVCD;
						else if (denominator == 5)
							y4m->aspect_ratio = DVD_NARROW;
					}
					else if (numerator == 32)
						y4m->aspect_ratio = DVD_WIDE;
					else if (numerator == 128)
						y4m->aspect_ratio = DV_PAL;
				}
				token_start = strchr(token_start, 0x20);
				break;
			case 'C': // Colour space
				y4m->colour_space = parse_colour_space(token_start, NULL);
				token_start = strchr(token_start, 0x20);
				break;
		}

		if (!token_start)
			break;
		token_start++;
	}

PARAM_SETTING:

	init_pos = ftell(ctx->src_fp);
	fseek(ctx->src_fp, 0, SEEK_END);
	file_length = ftell(ctx->src_fp);

	if(y4m->header_found)
		fseek(ctx->src_fp, init_pos, SEEK_SET);
	else
		fseek(ctx->src_fp, 0, SEEK_SET);


	plane_length = (y4m->width * y4m->height * 3) >> 1;

	if(y4m->header_found)
		frame_length = strlen(Y4M_FRAME_MAGIC) + 1 + plane_length;
	else
		frame_length = plane_length;

	num_frames = (file_length - header_length) / frame_length;
	if ((ctx->frames == 0) || (ctx->frames > num_frames)) {
		ctx->frames = num_frames;
	}

	// malloc yuv plane buffer
	y4m->plane.frame_sz = plane_length;
	y4m->plane.y_sz = y4m->width * y4m->height;
	y4m->plane.uv_sz= y4m->plane.y_sz >> 2;
	y4m->plane.frame_data = malloc(sizeof(char) * plane_length);
	y4m->plane.y_plane = y4m->plane.frame_data;
	y4m->plane.u_plane = y4m->plane.y_plane + y4m->plane.y_sz;
	y4m->plane.v_plane = y4m->plane.u_plane + y4m->plane.uv_sz;

	return 0;
}

static int read_frame(snx_context *ctx)
{
	y4m_header_param *y4m = &ctx->y4m;
	yuv_plane *plane = &y4m->plane;
	char header[MAX_YUV4_FRAME_HEADER];
	int i;

	if(y4m->header_found) {
	// read frame header
		for (i = 0; i < MAX_YUV4_FRAME_HEADER; ++i) {
			header[i] = fgetc(ctx->src_fp);
			if (header[i] == 0x0a)
				break;
		}
		if (i == MAX_YUV4_FRAME_HEADER ||
				strncmp(header, Y4M_FRAME_MAGIC, strlen(Y4M_FRAME_MAGIC))) {
			printf("invalid frame header\n");
			return -1;
		}
	}

	// FIXME: adjust reading size if needed.
	i = fread(plane->frame_data, sizeof(char), plane->frame_sz, ctx->src_fp);
	if (i == 0) {
		return EOF;
	} else if (i != plane->frame_sz) {
		printf("fatal: read frame failed\n");
		return -2;
	}

	return 0;
}

static int close_file(snx_context *ctx)
{
	y4m_header_param *y4m = &ctx->y4m;
	if (y4m->plane.frame_data) {
		free(y4m->plane.frame_data);
		memset(&y4m->plane, 0, sizeof(yuv_plane));
	}

	if (ctx->src_fp) {
		fclose(ctx->src_fp);
		ctx->src_fp = NULL;
	}

	return 0;
}

/**
 * snx_420p_to_420line
 *
 * @descript:
 *   Providing helper function to convert snx420 to 420p YUV colour space.
 *
 * @param:
 *   plane - yuv plane, refer to struct yuv_plane for more details.
 *   p_out - output buffer pointer, construct and destruct outside this function.
 *   width, height - frame width and height.
 */
#define MB_SIZE	4
static int snx_420p_to_420line(yuv_plane *plane, char *p_out,
		size_t width, size_t height)
{
	size_t y_pitch = width;
	size_t uv_pitch = y_pitch >> 1;
	int num_mb = 0, num_lines = 0;
	char *p = p_out, *py, *pu, *pv;

	if (width & 0x3) {
		printf("unalignment pitch\n");
		return -1;
	}

	for (;num_lines < height; ++num_lines) {

		// adjust plane address in pointers
		py = plane->y_plane + (y_pitch * num_lines);
		pu = plane->u_plane + (uv_pitch * (num_lines >> 1));
		pv = plane->v_plane + (uv_pitch * (num_lines >> 1));

		for (num_mb = 0; num_mb < (y_pitch >> 2); ++num_mb) {
			memcpy(p, py, MB_SIZE);
			p += MB_SIZE; py += MB_SIZE;
			if (!(num_lines & 0x01L)) {
				// process UV-plane only odd line
				*p = *pu++; p++;
				*p = *pv++; p++;
				*p = *pu++; p++;
				*p = *pv++; p++;
			}
		}
	}

	return 0;
}

/*******************************************************************************
 * Application Function definition
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
		    "-f, --frames\t\tFrame Number.\n"
		    "-r, --framerate\t\tFrame rate.\n"
		    "-W, --width\t\tFrame width.\n"
		    "-H, --height\t\tFrame height.\n"
		    "-s, --scale\t\tFrame scale. (1,2,4)\n"
		    "-b, --bitrate\t\tFrame bitrate (Kbps) (default=1024).\n"
		    "-o, --output filename\tOutput file name. (default=inputname.h264)\n"
		    "-p, --package output AVI/MP4 packed file\n"
		    "", argv[0]);
}

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
		printf("capture buffer, bytes: %d\n", m2m->cap_bytesused);
#endif
		hexdump(m2m->cap_buffers[m2m->cap_index].start,
				16/*m2m->cap_bytesused*/);

	} else if (ctx->output_mode == MODE_DUMP_FILE) {

		// write data from encoder output
		ret = fwrite(m2m->cap_buffers[m2m->cap_index].start,
				1, m2m->cap_bytesused, ctx->dst_fp);
		if(m2m->cap_bytesused % 16 !=0)	{			
			int i;
			for(i = 0;i < 16-(m2m->cap_bytesused % 16);i++)
				putc(0,ctx->dst_fp);
		}  //16 byte alignment, match with c-model
	} else if (ctx->output_mode == MODE_PACK_FILE) {

		// write data from encoder output
		save_data(sd_record, 1, m2m->cap_buffers[m2m->cap_index].start, m2m->cap_bytesused, "video");

	} else if (ctx->output_mode == MODE_LCD) {
		fprintf(stderr, "Not implement output mode: %d\n", ctx->output_mode);
	} else {
		fprintf(stderr, "Unknown output mode: %d\n", ctx->output_mode);
	}

failed:
	return ret;
}

/**
 * transcode example
 *
 * @descript:
 *
 *   Helper function snx_dec2enc provides memory-to-memory service allows
 * transcoding to another format bit-stream.
 *
 *               -----------   -----------
 *   H.264/JPG-->| Decoder |-->| Encoder |-->JPG/H.264
 *               -----------   -----------
 */
void * dec2enc(void *arg)
{
	struct v4l2_control ctrl;
	snx_context *ctx = (snx_context*) arg;
	struct snx_m2m *m2m = ctx->m2m;
	int ret, frame_count = 0;
	struct snx_rc *rc = NULL;			//rate control use

#if V4L2_USERPTR
	m2m->dec_fd = snx_open_device(m2m->dec_dev);
	ret = snx_dec_init(m2m);
	if (ret) {
		printf("decoder init failed\n");
		goto failed;
	}

	ret = snx_dec_start(m2m);
	if (ret) {
		printf("decoder start failed\n");
		goto failed;
	}
#endif

	// Scaling down if needed
	if (m2m->scale > 1) {
		m2m->width = m2m->width / m2m->scale;
		m2m->height = m2m->height / m2m->scale;
		m2m->scale = 1;
	}

	m2m->codec_fd = snx_open_device(m2m->codec_dev);
	ret = snx_codec_init(m2m);
	if (ret) {
		printf("encoder init failed\n");
		goto failed;
	}

#if V4L2_USERPTR
	// special setting entropy mode
	CLEAR(ctrl);
	ctrl.id = V4L2_CID_MPEG_VIDEO_H264_ENTROPY_MODE;
	ctrl.value = V4L2_MPEG_VIDEO_H264_ENTROPY_MODE_CAVLC;
	ret = ioctl(m2m->codec_fd, VIDIOC_S_CTRL, &ctrl);
	if (ret)
		printf("encoder set entropy mode failed\n");
#endif
	/* Set Codec GOP */
	snx_codec_set_gop(m2m);

	/* Bitrate Rate Control is only support for H264 */
	if(m2m->codec_fmt == V4L2_PIX_FMT_H264) {
		
		if (m2m->bit_rate) {
			rc = malloc(sizeof(struct snx_rc));
			
			/* Initialize rate control arguments */
			rc->width = m2m->width/m2m->scale;	//Bit-rate control width
			rc->height = m2m->height/m2m->scale;	//Bit rate control height
			rc->codec_fd = m2m->codec_fd;		//point to the codec fd
			rc->Targetbitrate = m2m->bit_rate;	//rate control target bitrate
			rc->framerate = m2m->codec_fps;		//point to the codec frame rate
			rc->gop = m2m->gop;					//codec gop
			//rc->reinit = 1;
			/*Initialize rate control */
			m2m->qp = snx_codec_rc_init(rc, SNX_RC_INIT);
		} else {
			//H264 QP
			snx_codec_set_qp(m2m,V4L2_CID_MPEG_VIDEO_H264_I_FRAME_QP); 
  			snx_codec_set_qp(m2m,V4L2_CID_MPEG_VIDEO_H264_P_FRAME_QP);
		}
	} else {
		// JPEG 
		snx_codec_set_qp(m2m , 0);
	}

	ret = snx_codec_start(m2m);
	if (ret) {
		printf("encoder start failed\n");
		goto failed;
	}

	printf("Transcoding y4m to h.264");
	fflush(stdout);

	// frame processing
	for (; frame_count < ctx->frames; ++frame_count) {
		struct v4l2_buffer buf;
		y4m_header_param *y4m = &ctx->y4m;
		yuv_plane *plane = &y4m->plane;
		fd_set read_fds;

		if (ctx->abort)
			break;

		ret = read_frame(ctx);
		if (ret == EOF) { // EOF
			printf("end of file\n");
			ctx->abort = 1;
			continue;
		} else if (ret == -2) {
			printf("read frame failed\n");
			break;
		}

#if V4L2_USERPTR
		ret = snx_420p_to_420line(plane, m2m->dec_cap_buffers[0].start,
				y4m->width, y4m->height);
		if (ret < 0) {
			break;
		}
#else
		if (m2m->out_mem == V4L2_MEMORY_MMAP) {
			ret = snx_420p_to_420line(plane, m2m->out_buffers[m2m->cap_index].start,
					y4m->width, y4m->height);
			if (ret < 0) {
				break;
			}
		} else {
			printf("Wrong out_mem type %d\n", m2m->out_mem);
			break;
		}
#endif

#if _DEBUG_FLOW
		printf("input buffer data:");
		hexdump(m2m->dec_cap_buffers[0].start, 16);
#endif

#if V4L2_USERPTR
		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
		buf.memory = m2m->out_mem;
		buf.index = m2m->dec_cap_index;
		if (buf.memory == V4L2_MEMORY_USERPTR) {
			buf.m.userptr = (unsigned long) m2m->dec_cap_buffers[buf.index].start;
			buf.length = m2m->dec_cap_buffers[buf.index].length;
		}

		ret = ioctl(m2m->codec_fd, VIDIOC_QBUF, &buf);
		if (ret < 0) {
			perror("capture queue buffer");
			goto failed;
		}
	
		FD_ZERO(&read_fds);
		FD_SET(m2m->codec_fd, &read_fds);
	
		ret = select(m2m->codec_fd + 1, &read_fds, NULL, NULL, 0);
		if (ret < 0) {
			perror("select");
			goto failed;
		}
	
		// output dequeue
		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
		buf.memory = m2m->out_mem;
		ret = ioctl(m2m->codec_fd, VIDIOC_DQBUF, &buf);
		if (ret < 0) {
			perror("output dequeue buffer");
			goto failed;
		}

		// capture dequeue
		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		ret = ioctl(m2m->codec_fd, VIDIOC_DQBUF, &buf);
		if (ret < 0) {
			perror("capture dequeue buffer");
			goto failed;
		}
		m2m->flags = buf.flags;
		m2m->cap_index = buf.index;
		m2m->cap_bytesused = buf.bytesused;
#else
		//V4L2_MEMORY_MMAP
		/* Read from Video Codec */
		ret = snx_codec_read(m2m);
#endif

#if _DEBUG_FLOW
		printf("output buffer data:");
		hexdump(m2m->cap_buffers[m2m->cap_index].start, 16);
#endif

		// output to file
		if (m2m->cap_bytesused != 0) {
			frame_output(ctx);
			/* 
				Bit Rate Control Flow 
				Update the QP of the next frame to keep the bitrate. (CBR).
			*/
			if((m2m->bit_rate) && (m2m->codec_fmt == V4L2_PIX_FMT_H264)) {
				m2m->qp = snx_codec_rc_update(m2m, rc);
			        snx_md_drop_fps(rc, &m2m->force_i_frame);
			}
		}

#if V4L2_USERPTR
		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = m2m->cap_index;
		ret = ioctl(m2m->codec_fd, VIDIOC_QBUF, &buf);
		if (ret < 0) {
			perror("reset failed");
			goto failed;
		}
#else
		//V4L2_MEMORY_MMAP
		/* Reset Codec for the next frame */
		ret = snx_codec_reset(m2m);
#endif
		printf(".");
		fflush(stdout);

	}

	printf("done!\n");
	printf("total %d frame\n", frame_count);
	ctx->abort = 1;

failed:
	if(rc)
		free(rc);
	pthread_exit(NULL);
}

int main(int argc, char **argv)
{
	snx_context *ctx = NULL;
	struct snx_m2m *m2m = NULL;

	int frames = 0, buf_count = 1;
	char src_file[PATH_MAX] = {0};
	char dst_file[PATH_MAX] = {0};
	int ret = -1;
	int width = 1280, height = 720, framerate =30 , scale = 1, bitrate = 1024;
	int pack = 0;

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
			case 'f':
				sscanf(optarg, "%d", &frames);
				break;
			case 'W':
				sscanf(optarg, "%d", &width);
				if((width <= 0 ) || (width > 1920)) {
					printf("Wrong width param (%d)\n", width);
					exit(EXIT_SUCCESS);
				}
				break;
			case 'H':
				sscanf(optarg, "%d", &height);
				if((height <= 0 ) || (height > 1200)) {
					printf("Wrong height param (%d)\n", height);
					exit(EXIT_SUCCESS);
				}
				break;
			case 'r':
				sscanf(optarg, "%d", &framerate);
				if(framerate <= 0 ) {
					printf("Wrong framerate param (%d)\n", framerate);
					exit(EXIT_SUCCESS);
				}
				break;
			case 's':
				sscanf(optarg, "%d", &scale);
				if(((scale < 1) || (scale > 4)) || (scale == 3)) {
					printf("wrong scale param(%d)\n", scale);
					scale = 1;
				}
				break;
			case 'b':
				sscanf(optarg, "%d", &bitrate);
				if(bitrate <= 0 ) {
					printf("Wrong bitrate param (%d)\n", bitrate);
					exit(EXIT_SUCCESS);
				}
				break;

			case 'o':
				memset(dst_file, 0, sizeof(dst_file));
				strcpy(dst_file, optarg);
				break;

			case 'p':
				pack = 1;
				break;
			default:
				usage(stderr, argc, argv);
				exit(EXIT_FAILURE);
		}
	}
	strncpy(src_file, argv[optind], strlen(argv[optind]));

	if(strlen(dst_file) == 0)
		sprintf(dst_file, "%s.h264",src_file);

	// snx_context allocate and initialize
	ctx = (snx_context*) calloc(1, sizeof(snx_context));
	if (!ctx) {
		printf("create snx_context failed\n");
		exit(EXIT_FAILURE);
	}
	ctx->abort = 0;
	//ctx->step = USE_FRAME_STEP;
	ctx->frames = frames;

	if (pack)
		ctx->output_mode = MODE_PACK_FILE; 
	else
		ctx->output_mode = MODE_DUMP_FILE;

	ctx->y4m.width = width;
	ctx->y4m.height = height;
	ctx->y4m.framerate = framerate;
	ctx->y4m.interlace = PROGRESSIVE;
	ctx->y4m.aspect_ratio = UNKNOWN;
	ctx->y4m.colour_space = YUV420;
	// Open y4m file, and also parse header
	ret = open_file(ctx, src_file);
	if (ctx->y4m.colour_space > YUV420) {
		printf("invalid colour space, YUV 420p only\n");
		goto failed;
	}

	printf("\n----- YUV INFO -----\n");
	printf("Width:\t%d\n", ctx->y4m.width);
	printf("Height:\t%d\n", ctx->y4m.height);
	printf("framerate:\t%d\n", (int)ctx->y4m.framerate);
	printf("interlace:\t%d\n", ctx->y4m.interlace);
	printf("colour_space:\t%d\n", ctx->y4m.colour_space);
	printf("frames:\t%d\n", ctx->frames);
	printf("\n--------------------\n");

	// snx_m2m allocate and initialize
	m2m = (struct snx_m2m*) calloc(1, sizeof(struct snx_m2m));
	if (!m2m) {
		printf("create snx_m2m failed\n");
		goto failed;
	}
	m2m->m2m = 1;
	m2m->width = ctx->y4m.width;
	m2m->height = ctx->y4m.height;
	m2m->scale = scale;
	m2m->m2m_buffers = 1;
	// encoder params
	m2m->isp_fps = floor(ctx->y4m.framerate);
	m2m->isp_fmt = V4L2_PIX_FMT_SNX420;
	m2m->codec_fps = m2m->isp_fps;
	m2m->codec_fmt = V4L2_PIX_FMT_H264;
	
#if V4L2_USERPTR
	m2m->out_mem = V4L2_MEMORY_USERPTR;
#else
	m2m->out_mem = V4L2_MEMORY_MMAP;
	m2m->out_fmt = V4L2_PIX_FMT_SNX420;
#endif
	

	m2m->bit_rate = bitrate << 10;
	strcpy(m2m->codec_dev, ENC_DEV_NAME);

#if V4L2_USERPTR
	// decoder params
	m2m->dec_fmt = V4L2_PIX_FMT_H264;
	m2m->dec_out_mem = V4L2_MEMORY_MMAP;
	m2m->dec_cap_mem = V4L2_MEMORY_MMAP;
	strcpy(m2m->dec_dev, DEC_DEV_NAME);
#endif

	ctx->m2m = m2m;
	printf("Setting w:%d h:%d fr:%0.2f num_frames %d\n", m2m->width, m2m->height,
			ctx->y4m.framerate, ctx->frames);

	printf("\n----- M2M INFO -----\n");
	printf("m2m->m2m:\t%d\n", m2m->m2m);
	printf("m2m->width:\t%d\n", m2m->width);
	printf("m2m->height:\t%d\n", m2m->height);
	printf("m2m->scale:\t%d\n", m2m->scale);
	printf("m2m->codec_fps:\t%d\n", m2m->codec_fps);
	printf("m2m->bit_rate:\t%d\n", m2m->bit_rate);
	printf("m2m->codec_dev:\t%s\n", m2m->codec_dev);
	printf("m2m->dec_dev:\t%s\n", m2m->dec_dev);
	printf("\n--------------------\n");

	// output file open
	if (ctx->output_mode == MODE_DUMP_FILE && strlen(dst_file) != 0) {
		ctx->dst_fp = fopen(dst_file, "w+b");
		if (!ctx->dst_fp) {
			ret = -1;
			perror("open output file failed");
			//printf("open output file '%s' failed\n", dst_file);
			goto failed;
		}
	}

	if (ctx->output_mode == MODE_PACK_FILE) {
		// Initialize to pack AVI/MP4 
		char tmp_str[128];
		struct stat tst;
		xml_doc record_config;

		if (stat(RECORD_CONFIG, &tst) == -1) 
		{

			printf("[SNX_YUV2H264] NO yuv2h264.xml is found in %s\n", RECORD_CONFIG);
			goto failed;
		}

		memset(tmp_str,0x0, 128);
		sprintf(tmp_str, "%dx%d", m2m->width, m2m->height);
		write_config(RECORD_CONFIG, record_config, 1, "fileheader", dst_file);
		write_config(RECORD_CONFIG, record_config, 1, "videores", tmp_str);
		sprintf(tmp_str, "%d", m2m->codec_fps);
		write_config(RECORD_CONFIG, record_config, 1, "videofps", tmp_str);

		sd_record = create_recording(RECORD_CONFIG);
		if (sd_record == NULL) {
			fprintf(stderr, "avi file init failed!! \n");
				//printf("open output file '%s' failed\n", dst_file);
			goto failed;
		} 
		set_record_start(sd_record, 1);
	}

	pthread_create(&ctx->process_tid, NULL, dec2enc, (void*)ctx);

	while(!ctx->abort){
		sleep(1);
	}

	pthread_join(ctx->process_tid, NULL);

	if (ctx->output_mode == MODE_PACK_FILE) {
		// END of Packing
		set_record_stop(sd_record, 1);
	}

failed:
	close_file(ctx);
	if (ctx->m2m) free(ctx->m2m);
	if (ctx->dst_fp) fclose(ctx->dst_fp);
	if (ctx) free(ctx);
	if (sd_record) destory_recording(sd_record);

	return ret;
}
