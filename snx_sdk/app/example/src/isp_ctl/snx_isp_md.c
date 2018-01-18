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

int out = 0;

static size_t WIDTH = 1280;
static size_t HEIGHT = 720;

static char *path = "./";
static int frame_num = 0;

static int md_threshold = 0x200; //default
static int md_int_threshold = 1;
static int md_timeout = 1000;  // 1's

void snx_md_thread(void *arg);
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

void snx_md_thread(void *arg)
{
	int status;
	unsigned int mask[6], reports[6];

	fprintf(stderr, "============snx_md_thread============\n");

	/* disable motion detection */
	snx_isp_md_enable_set(0x0);

	/* set threshold parameter to isp driver */
	snx_isp_md_threshold_set(md_threshold);

	/* set int_threshold parameter to isp driver */
	snx_isp_md_int_threshold_set(md_int_threshold);
	
	/* detect all areas */
	memset(mask, 0x0, sizeof(mask));

	/* set mask blosks to isp driver */
	snx_isp_md_block_mask_set(mask);

	/* Interfaces provided by ISP driver:
	*/
	snx_isp_md_int_timeout_set(md_timeout);

	/* enable motion detection */
	snx_isp_md_enable_set(0x1);

	//main loop
	while(1){
		snx_isp_md_int_get(&status); /* interrupt. if 0 timeout, else have motion */

		if(0 == status){
			fprintf(stderr, "motion detection (have no motion status = %d)\n", status);
		}else{
			fprintf(stderr, "Detected Moving!\n");
			snx_isp_md_block_report_get(reports);
			fprintf(stderr, "report:%08x %08x %08x %08x %08x %08x\n", \
				reports[0], reports[1], reports[2], \
					reports[3], reports[4], reports[5]);
		}
		if(out)
			break;
	}

	/* disable motion detection */
	snx_isp_md_enable_set(0x0);

	return ;
}
 
//static int Previous_qp;

void snx_m2m_thread(void *arg)
{
	int count_num = 0;

	int ret;
	struct snx_m2m *m2m = arg;

	fprintf(stderr, "============snx_m2m_thread============\n");

	if(m2m->m2m) {
		m2m->isp_fd = snx_open_device(m2m->isp_dev);	// Open output device

		if((ret = snx_isp_init(m2m) != 0))
			goto err_init;
		snx_isp_start(m2m);
	}

	while(1) {
		snx_isp_read (m2m);

		count_num ++;

		snx_isp_reset (m2m);
		if(out)
			break;
	}


err_init:
	if(m2m->m2m) {
		snx_isp_stop(m2m);
		snx_isp_uninit(m2m);
	}

	if(m2m->m2m)
		close(m2m->isp_fd);

	free(m2m);
}


void sigterm_handler(int sig)
{
	snx_isp_md_enable_set(0x0);
	exit(0);
}

int main(int argc, char **argv)
{
	int ret;
	struct snx_m2m *m2m = NULL;

	pthread_t m2m_thread, md_thread;

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

	signal(SIGINT, sigterm_handler); /* Interrupt (ANSI). */
	signal(SIGQUIT, sigterm_handler); /* Quit (POSIX). */
	signal(SIGTERM, sigterm_handler); /* Termination (ANSI). */

	ret = pthread_create(&m2m_thread, NULL, (void *)snx_m2m_thread, m2m);
	if(ret != 0) {
		fprintf(stderr, "exit thread creation failed");   
	}
	ret = pthread_create(&md_thread, NULL, (void *)snx_md_thread, m2m);
	if(ret != 0) {
		fprintf(stderr, "exit thread creation failed");   
	}

	pthread_join(md_thread,NULL);	
	pthread_join(m2m_thread,NULL);

	return 0;
}

