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
#include <pthread.h>
#include <getopt.h>
#include <errno.h>
#include <fcntl.h>
#include <assert.h>
#include <math.h>
#include <termios.h>

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

#define USE_FRAME_STEP		0
#define DISABLE_EVENT_LOOP	USE_FRAME_STEP
#define _DEBUG_TIME		0
#define _DEBUG_FLOW		0
#define MAX_BUFFER_COUNT	3
#define USE_TERMIOS		0

#define DEC_DEV_NAME		"/dev/video16"

/*******************************************************************************
 * Structure & enumerate declaration
 ******************************************************************************/

enum {
	MODE_PRINT_SCREEN,
	MODE_DUMP_FILE,
	MODE_LCD,
};

typedef struct snx_context {
	int abort;
	int step;
	int output_mode;
	FILE *dst_fp;
	struct snx_m2m *m2m;
	pthread_t process_tid;
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
		    "-c, --codec\t\tFile input format h264, jpeg. (default=h264)\n"
		    "-s, --scale\t\tScaling down ratio 1,2,4, for 1/1, 1/2, 1/4. (default=1)\n"
		    "-o, --output filename\tOutput file name. (default=test.yuv)\n"
		    "-B, --buffer\t\tV4L2 buffer. (default=2)\n"
		    "", argv[0]);
}

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
		fprintf(stderr, "Not implement output mode: %d\n", ctx->output_mode);
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
	long acc_tm = 0;

	// frame processing
	for (;;) {
		struct v4l2_buffer buf = {0};
#if _DEBUG_TIME
		struct timeval s_tv, e_tv, res_tv;
		long tm = 0;
#endif

		if (ctx->abort)
			break;

#if _DEBUG_TIME
		gettimeofday(&s_tv, NULL);
#endif
		ret = snx_bs_reader_read_frame(m2m,
			NULL, 0,
			m2m->dec_out_buffers[m2m->dec_out_index].start,
			m2m->dec_out_buffers[m2m->dec_out_index].length);
		if (ret == -1) { // EOF
			printf("end of file\n");
			ctx->abort = 1;
			continue;
		}
		m2m->dec_out_bytesused = ret;

#if _DEBUG_TIME
		gettimeofday(&e_tv, NULL);
		timersub(&e_tv, &s_tv, &res_tv);
		tm = ((res_tv.tv_sec*1000)+(res_tv.tv_usec/1000));
		printf("frame%d size: %d takes %ld ms\n", frame_count+1,
				m2m->dec_out_bytesused, tm);
		acc_tm += tm;
#endif
#if _DEBUG_FLOW
		hexdump(m2m->dec_out_buffers[m2m->dec_out_index].start, 16);
#endif

		ret = snx_dec_read(m2m);
		if (ret < 0) {
			perror("decode failed");
		}

		frame_count++;

		// output to file, or print on screen
		if (m2m->dec_cap_bytesused != 0)
			frame_output(ctx);

		ret = snx_dec_reset(m2m);
		if (ret < 0) {
			perror("reset failed");
		}

		// step
		if (ctx->step) {
			fd_set read_fds;
			int r;

#if USE_FRAME_STEP
			printf("frame%d press 'space' for next frame, "
					"or 'q' to quit\n", frame_count);
#endif

			FD_ZERO(&read_fds);
			FD_SET(STDIN_FILENO, &read_fds);
step_retry:
			r = select(STDIN_FILENO+1, &read_fds, NULL, NULL, 0);
			if (r < 0) {
				perror("stdin step\n");
				exit(EXIT_FAILURE);
			}

			//r = fgetc(stdin);
			r = getchar();
			switch(r) {
				case ' ':
					break;
				case 'q':
					ctx->abort = 1;
					break;
				default:
					goto step_retry;
			}
		}
	}
#if _DEBUG_TIME
	printf("statistic parser takes %0.2lf ms in average\n", (double)acc_tm/frame_count);
#endif
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

int main(int argc, char **argv)
{
	snx_context *ctx = NULL;
	struct snx_m2m *m2m = NULL;

	int w = 1280, h = 720, codec = 0; // 0:h264, 1:mjpeg
	int scale_ratio = 1, buf_count = 2;
	char src_file[PATH_MAX] = {0};
	char dst_file[PATH_MAX] = "test.yuv";
	int ret = -1;

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
			default:
				usage(stderr, argc, argv);
				exit(EXIT_FAILURE);
		}
	}
	strncpy(src_file, argv[optind], strlen(argv[optind]));

	start_sitio();

	// snx_context allocate and initialize
	ctx = (snx_context*) calloc(1, sizeof(snx_context));
	if (!ctx) {
		printf("create snx_context failed\n");
		exit(EXIT_FAILURE);
	}
	ctx->abort = 0;
	ctx->step = USE_FRAME_STEP;
	ctx->output_mode = MODE_DUMP_FILE; //MODE_PRINT_SCREEN; //MODE_DUMP_FILE; //MODE_LCD;

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
	ctx->m2m = m2m;
	printf("Setting w:%d h:%d codec:%d scale:%d buf:%d\n",
			m2m->width, m2m->height, codec, m2m->scale, m2m->m2m_buffers);

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

	// initial modules
	ret = snx_bs_reader_init(m2m, src_file);
	if (ret) {
		perror("input streaming init failed");
		goto rdr_init_failed;
	}

	m2m->dec_fd = snx_open_device(m2m->dec_dev);
	ret = snx_dec_init(m2m);
	if (ret) {
		printf("decoder init failed\n");
		goto init_failed;
	}

	ret = snx_dec_start(m2m);
	if (ret) {
		printf("decoder start failed\n");
		goto start_failed;
	}
#if 0
	if (ctx->output_mode == MODE_LCD) {
		ret = dsp_init(ctx);
		if (ret) {
			printf("display init failed\n");
			goto init_failed;
		}

		ret = dsp_start(ctx);
		if (ret) {
			printf("display start failed\n");
			goto init_failed;
		}
	}
#endif
	pthread_create(&ctx->process_tid, NULL, mjpeg_process_thread, (void*)ctx);

#if DISABLE_EVENT_LOOP
#else
	event_loop(ctx);
#endif

	pthread_join(ctx->process_tid, NULL);

start_failed:
	snx_dec_stop(m2m);
init_failed:
	snx_dec_uninit(m2m);
rdr_init_failed:
	snx_bs_reader_uninit(m2m);
failed:
	if (ctx->m2m) free(ctx->m2m);
	if (ctx->dst_fp) fclose(ctx->dst_fp);
	if (ctx) free(ctx);

	end_sitio();

	return ret;
}
