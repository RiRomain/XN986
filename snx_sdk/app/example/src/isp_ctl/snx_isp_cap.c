/**

 */

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include <endian.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <pthread.h>

#include <signal.h>

#include <getopt.h>             /* getopt_long() */
#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>          /* for videodev2.h */

#include <linux/videodev2.h>

////////////////
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
////////////////

#include <snx_isp/isp_lib_api.h>
#include <snx_vc/snx_vc_lib.h>

#define VIDEO_DEV_NAME	"/dev/video0"

static size_t WIDTH = 1280;
static size_t HEIGHT = 720;

static char *path = "./";
static int frame_num = 0;

void snx_m2m_thread(void *arg);

static const char short_options[] = "hW:H:i:n:p:b:B:";

static const struct option long_options[] = {
    {"help", no_argument, NULL, 'h'},
    {"width", required_argument, NULL, 'W'},
    {"height", required_argument, NULL, 'H'},
    {"fps", required_argument, NULL, 'i'},
    {"frames", required_argument, NULL, 'n'},
    {"filename", required_argument, NULL, 'p'},
    {"buffer", required_argument, NULL, 'B'},

    {0, 0, 0, 0}
};


static void usage(FILE * fp, int argc, char ** argv) {   
    fprintf(fp, "Usage: %s [options]/n\n"   
        "Options:\n"
        "-h Print this message\n"
        "-W | --width        Frame Width\n"
        "-H | --height       Frame Height\n"
	"-n | --frames       Number of frames to num [1000]\n"
        "-p | --filename     File save Path\n"
        "-i | --fps      (optional) ISP Frame Rate (default=30) \n"
        "-B | --buffer       (optional) V4L2 buffer (default=4)\n"

        "", argv[0]);   
}
 
//static int Previous_qp;

void snx_m2m_thread(void *arg)
{
	int count_num = 0;

	int ret;
	struct snx_m2m *m2m = arg;
	FILE * fd;

	fprintf(stderr, "============snx_m2m_thread============\n");


	fd = fopen(path,"wb");

	if(m2m->m2m) {
		m2m->isp_fd = snx_open_device(m2m->isp_dev);	// Open output device

		ret = snx_isp_init(m2m);
		if(ret != 0) goto err_init;
		snx_isp_start(m2m);
		fprintf(stderr, "============snx_m2m_thread============  isp\n");
	}


	fprintf(stderr, "============snx_m2m_thread============  codec\n");
	while(1) {
		snx_isp_read (m2m);

		count_num ++;

		fwrite(m2m->isp_buffers[m2m->isp_index].start,  ((m2m->width*m2m->height*3)/2), 1, fd);
		snx_isp_reset (m2m);

    		frame_num--;
		if(frame_num == 0)
			break;
	}//while(1)


err_init:
	if(m2m->m2m) {
		snx_isp_stop(m2m);
		snx_isp_uninit(m2m);
	}

	if(m2m->m2m)
		close(m2m->isp_fd);

        fflush(fd);
        fclose(fd);

 
	fprintf(stderr, "============snx_m2m_thread============End\n");	
	free(m2m);
}

int main(int argc, char **argv)
{
	int ret;
	struct snx_m2m *m2m = NULL;

	pthread_t m2m_thread;

	m2m = malloc(sizeof(struct snx_m2m));
	memset(m2m, 0x0, sizeof(struct snx_m2m));

	m2m->m2m = 1;
	m2m->isp_fps = 30;
	m2m->width = WIDTH;
	m2m->height = HEIGHT;
	m2m->m2m_buffers = 2;
	m2m->isp_mem = V4L2_MEMORY_MMAP;
	m2m->isp_fmt = V4L2_PIX_FMT_SNX420;

	strcpy(m2m->isp_dev,VIDEO_DEV_NAME);

	for (;;)
	{   
		int index;   
		int c;   
		c = getopt_long(argc, argv, short_options, long_options, &index);   
		if (-1 == c)   
			break;   
		switch (c)
		{   
			case 0: /* getopt_long() flag */   
				break;
			case 'h':   
				usage(stdout, argc, argv);   
				exit(EXIT_SUCCESS);
			case 'i':
				sscanf(optarg, "%d", &m2m->isp_fps);
				break;
			case 'W':
				sscanf(optarg, "%d", &m2m->width);
				break;
			case 'H':
				sscanf(optarg, "%d", &m2m->height);
				break;
			case 'n':
				sscanf(optarg, "%d", &frame_num);
				break;
			case 'p':
				path = optarg;
				break;
			case 'B':
				sscanf(optarg, "%d", &m2m->m2m_buffers);
				break;
			default:   
				usage(stderr, argc, argv);   
				exit(EXIT_FAILURE);   
		}   
	}


	ret = pthread_create(&m2m_thread, NULL, (void *)snx_m2m_thread, m2m);
	if(ret != 0) {
		fprintf(stderr, "exit thread creation failed");   
	}
	pthread_join(m2m_thread,NULL);

	return 0;
}

