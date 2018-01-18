#include <stdio.h>
#include <stdlib.h>
#include <zbar.h>
#include <sys/stat.h>
#include <signal.h> 
#include <string.h>
#include <getopt.h>   
#include <linux/videodev2.h>
#include "snx_vc_lib.h"
#include "snx_rc_lib.h"

extern int sd_record_en;

#define WIFI_TEMP_FILE "/tmp/wifi_temp"



static int gQuit_scan_flag = 1;
static int gRec_success_flag = 0;

/* signal handler to disable qr scan process */
static void sig_handler(void)
{
	fprintf(stderr, "Receive signal to terminate \n");
	gQuit_scan_flag = 0;
	gRec_success_flag = 0;
	system("echo 0 > /proc/isp/ae/offset");
}

#define IMG_DEV_NAME	"/dev/video0"
#define IMG1_DEV_NAME	"/dev/video3"
#define CAP_DEV_NAME	"/dev/video1"
#define CAP1_DEV_NAME	"/dev/video2"

#define BEEP_PCM                "/etc/notify/reset.pcm"


static const char short_options[] = "hW:H:a:r:bp:";

static const struct option long_options[] = {
    {"help", no_argument, NULL, 'h'},
    {"width", required_argument, NULL, 'W'},
    {"height", required_argument, NULL, 'H'},
    {"ae_offset", required_argument, NULL, 'a'},
    {"record", required_argument, NULL, 'r'},
    {"beep", no_argument, NULL, 'b'},
    {"path", required_argument, NULL, 'p'},
    {0, 0, 0, 0}
};


static void usage(FILE * fp, int argc, char ** argv) {   
    fprintf(fp, "Usage: %s [options]/n\n"   
        "Options:\n"
        "-h Print this message\n"
        "-W | --width        Frame Width (default=640)\n"
        "-H | --height       Frame Height (default=480)\n"
        "-a | --ae_offset    AE offset adjust 1=Enable, 0=Disable(default=1)\n"
        "-r | --record       record YUV for debug 1=Enable, 0=Disable(default=0)\n"
        "-b | --beep         beep when scan success default /etc/notify/reset.pcm\n"
        "-p | --beep         beep pcm path\n"
        "\n"
        "", argv[0]);   
}

void snx_play_audio(char *src_audio)
{
	char cmd[64] = {0};
	system ("/bin/gpio_ms1 -n 7 -m 1 -v 1");
	usleep(35000); // dealy 35ms
	//system("/usr/bin/pcm_play  /etc/notify/xxx.pcm");
	//sprintf(cmd, "%s %s",PCM_PLAY, src_audio);
	//system(cmd);
	pcm_play(src_audio);
	system ("/bin/gpio_ms1 -n 7 -m 1 -v 0");
}


int main (int argc, char **argv)
{
	struct snx_m2m *m2m = NULL;
	unsigned char * pYUV420 = NULL;
	FILE * fd_yuv ;
	FILE *fp = NULL, *fp_ae = NULL;
	
	int beep_flag = 0;
	int n;
	int size;
	const zbar_symbol_t *symbol;
	zbar_image_t *image;
	zbar_image_scanner_t *scanner = NULL;
	const char *data = NULL;
//	int count = 30;

	int ae_offset = 1;
	int iTarget = -100;
	char chTarget[10];
	int iupdown = 1;
	char *buf;
	int yuv_record = 0;
	
	char audio_src[128];

	/* add signal handler */
	signal(SIGINT, (__sighandler_t)sig_handler); 
	signal(SIGQUIT, (__sighandler_t)sig_handler);
	signal(SIGTERM, (__sighandler_t)sig_handler);
	
	m2m = malloc(sizeof(struct snx_m2m));
	memset(m2m, 0x0, sizeof(struct snx_m2m));

	m2m->isp_fps = 30;
	m2m->m2m_buffers = 2;
	m2m->scale = 1;
	m2m->codec_fps = 30;
	m2m->width = 640;
	m2m->height = 480;
	m2m->gop = m2m->isp_fps;
	m2m->out_mem = V4L2_MEMORY_USERPTR;
	m2m->m2m = 1;			/* M2M stream */
	m2m->isp_mem = V4L2_MEMORY_MMAP;
	m2m->isp_fmt = V4L2_PIX_FMT_SNX420;
	
	strcpy(m2m->isp_dev,IMG_DEV_NAME);
	strcpy(m2m->codec_dev,CAP_DEV_NAME);

	strcpy(audio_src, BEEP_PCM);
	
	for (;;)
	{   
		int index;   
		int c;   
		c = getopt_long(argc, argv, short_options, long_options, &index);   
		if (-1 == c)   
			break;   
		switch (c)
		{   
			case 0: // getopt_long() flag 
				break;
			case 'h':   
				usage(stdout, argc, argv);   
				exit(EXIT_SUCCESS);   
			case 'W':
				sscanf(optarg, "%d", &m2m->width);
				break;
			case 'H':
				sscanf(optarg, "%d", &m2m->height);
				break;		
			case 'a':
				sscanf(optarg, "%d", &ae_offset);
				break;	
			case 'r':
				sscanf(optarg, "%d", &yuv_record);
				break;
			case 'b':
				beep_flag = 1;
				break;
			case 'p':
				sscanf(optarg, "%s", &audio_src);
				break;
			default:   
				usage(stderr, argc, argv);   
				exit(EXIT_FAILURE);   
		}   
	}

	size =  ((m2m->width/m2m->scale) * (m2m->height/m2m->scale)*3 )>>1;

	pYUV420 = (unsigned char*)malloc(size);
	buf = (unsigned char*)malloc(m2m->width * m2m->height);
	memset(buf, 0x00, m2m->width * m2m->height);
	
	if(yuv_record==1) {
		fd_yuv = fopen("/mnt/test/test.yuv","wb");
	}
	// Open ISP device 
	m2m->isp_fd = snx_open_device(m2m->isp_dev);
	snx_isp_init(m2m);
	snx_isp_start(m2m);

	/* create a reader */
	scanner = zbar_image_scanner_create();
	/* configure the reader */
	zbar_image_scanner_set_config(scanner, 0, ZBAR_CFG_ENABLE, 1);
	image = zbar_image_create();
	zbar_image_set_format(image, *(int*)"Y800");
	zbar_image_set_size(image, m2m->width, m2m->height);

	zbar_image_set_data(image, pYUV420,size, zbar_image_free_data);

	while(gQuit_scan_flag){
		snx_isp_read(m2m);
		snx_420line_to_420(m2m->isp_buffers[m2m->cap_index].start, pYUV420
				, (m2m->width/m2m->scale)
				, (m2m->height/m2m->scale));

		if(yuv_record==1) {
			fwrite(pYUV420,  (m2m->width * m2m->height), 1, fd_yuv);
			fwrite(buf,  (m2m->height * m2m->height)>>1, 1, fd_yuv);
		}
		snx_isp_reset(m2m);


		/* scan the image for barcodes */
		n = zbar_scan_image(scanner, image);//cost mush time if the picture's size is very big
		/* extract results */
		symbol = zbar_image_first_symbol(image);

		for (; symbol; symbol = zbar_symbol_next(symbol)) {
			/* do something useful with results */
			zbar_symbol_type_t type = zbar_symbol_get_type(symbol);
			data = zbar_symbol_get_data(symbol);
			printf("decoded %s symbol \"%s\"\n", zbar_get_symbol_name(type), data);
//			count --;
//			if(count == 0) {
			gQuit_scan_flag = 0;
			gRec_success_flag = 1;

			fp_ae = fopen("/proc/isp/ae/offset","wb");	
			sprintf(chTarget, "%d", 0);
			fwrite(chTarget, 1, 10, fp_ae);
			fclose(fp_ae);

//			}
			break;
		}

		if(ae_offset==1) {
			if (iTarget == -75)
				iupdown = 1;
			else if (iTarget == 50)
				iupdown = 0;
			if (iupdown)
				iTarget += 25;
			else
				iTarget -= 25;
		
			fp_ae = fopen("/proc/isp/ae/offset","wb");	
			sprintf(chTarget, "%d", iTarget);

//			printf("<<test>><%s><%d>chTarget ==%s\n",__func__, __LINE__,chTarget);

			fwrite(chTarget, 1, 10, fp_ae);
			fclose(fp_ae);

		}

	}

	system("echo 0 > /proc/isp/ae/offset");

	if (gRec_success_flag == 1) {
		fprintf(stderr, "extract code done!! \n");
		fp = fopen(WIFI_TEMP_FILE, "wb");
		if (fp == NULL) {
			fprintf(stderr, "[%s] open %s failed.\n", __FUNCTION__, WIFI_TEMP_FILE);
		} else {
			fwrite(data, strlen(data), 1, fp);
			fclose(fp);
			if(beep_flag)
				snx_play_audio(audio_src);
		}
	}

	/* clean up */
	zbar_image_destroy(image); // destory image ==	free(pYUV420);
	zbar_image_scanner_destroy(scanner);

	snx_isp_stop(m2m);
	snx_isp_uninit(m2m);
	free(m2m);
	if (buf)
		free(buf);

	if(yuv_record==1) {
		close(fd_yuv);
	}

/////////////////////////////////
	
}
