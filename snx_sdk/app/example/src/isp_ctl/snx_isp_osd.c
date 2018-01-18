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

#define ENABLE 0x1
#define DISABLE 0x0
#define TRANSPARENCY 0x00
#define OPACITY 0xff

#define	TEMPLATE_LENGHT	32 
#define TIMESTAMP_TEMP "0123456789:/.-"
#define SPACE_TEMP " "


static char ascii32x16[]=
{
	#include "ascii32x16.c"

};

static int SetOSDEnable(int enable)
{
	snx_isp_osd_enable_set(0, enable);
	snx_isp_osd_enable_set(1, enable);
	return 0;
}

static int SetOSDData(char *str, int timestamp)
{
	snx_isp_osd_data_str_set(0, str);
	snx_isp_osd_data_str_set(1, str);
	snx_isp_osd_timestamp_set(0, timestamp);
	snx_isp_osd_timestamp_set(1, timestamp);
	return 0;
}

static  int SetOSDTemplate(char *str)
{
	snx_isp_osd_template_set(0, str);
	snx_isp_osd_template_set(1, str);
	return 0;
}

static int SetOSDFont(char *font)
{
	snx_isp_osd_font_set(0, font);
	snx_isp_osd_font_set(1, font);
	return 0;
}

static int SetOSDGain(int gain)
{
	snx_isp_osd_gain_set(0, gain);
	snx_isp_osd_gain_set(1, gain);
	return 0;
}

static int SetOSDPosition(int x, int y)
{
	snx_isp_osd_position_set(0, x, y);
	snx_isp_osd_position_set(1, x, y);
	return 0;
}

static int SetOSDTxtColor(int color)
{
	snx_isp_osd_txt_color_set(0, color);
	snx_isp_osd_txt_color_set(1, color);
	return 0;
}

static int SetOSDBgColor(int color)
{
	snx_isp_osd_bg_color_set(0, color);
	snx_isp_osd_bg_color_set(1, color);
	return 0;
}

static int SetOSDTxtTransp(int transp)
{
	snx_isp_osd_txt_transp_set(0, transp);
	snx_isp_osd_txt_transp_set(1, transp);
	return 0;
}

static int SetOSDBgTransp(int transp)
{
	snx_isp_osd_bg_transp_set(0, transp);
	snx_isp_osd_bg_transp_set(1, transp);
	return 0;
}

static int SetOSD(char *text, int timestamp, int enable)
{
	char templet[TEMPLATE_LENGHT + 1];
	char font[2048];                       //TEMPLATE_LENGHT*64=2048
	int	ts_flag=0;
	int	txt_flag=0;
	int	m_txt_flag=0;
	int i=0,j=0;
	FILE *fp;

	memset(templet, 0, TEMPLATE_LENGHT+1);
	memset(font, 0, 2048);
	strcat(templet,SPACE_TEMP); //generate template
	if(timestamp==1)
	{
		strcat(templet,TIMESTAMP_TEMP);
		ts_flag=1;
	}
	if(strlen(text)!=0)
	{
		for(i=0,j=strlen(templet); i<strlen(text); i++)
		{
			if(index(templet,text[i]) == NULL)
			{
				if(j<TEMPLATE_LENGHT)
					templet[j++]=text[i];
				else
				{
					fprintf(stderr, "osd text too long to store.");
					break;
				}
			}
		}
		txt_flag=1;
	}
	//generate font
	{
		int i=0,index=0;
		for(i =0; i < strlen(templet); i++)
		{
			index = templet[i];
			index *= 64;
			memcpy(font +i*64, ascii32x16+index, 64);
		}
	}

	SetOSDTemplate(templet);
	SetOSDFont(font);
	SetOSDData(text, timestamp);
	SetOSDEnable(enable);

	return 0;
}

int setOSDAll()
{
#define	KEY_LENGTH		64 	
	char  *enable_g = "on";
	char  *text_g = "sonix";
	char  *timestamp_g = "on";
	char  *txt_color_g = "0xFFFFFF"; 
	char  *txt_transp_g = "off";
	char  *bg_color_g = "0x000000";
	char  *bg_transp_g = "on";
	char  *gain_g = "1"; 
	char  *startX_g = "0";
	char  *startY_g = "0";
	int i_enable = 0, i_timestamp = 0, i_txt_transp = 0, i_bg_transp = 0;

	if(!strcasecmp(enable_g, "on"))
		i_enable=1;
	else
		i_enable=0;	
	if(!strcasecmp(timestamp_g, "on"))
		i_timestamp=1;
	else
		i_timestamp=0;
	if(!strcasecmp(txt_transp_g, "on"))
		i_txt_transp=TRANSPARENCY;
	else
		i_txt_transp=OPACITY;
	if(!strcasecmp(bg_transp_g, "on"))
		i_bg_transp=TRANSPARENCY;
	else
		i_bg_transp=OPACITY;	
	
	SetOSDTxtColor(strtol(txt_color_g, NULL, 16));
	SetOSDBgColor(strtol(bg_color_g, NULL, 16));
	SetOSDTxtTransp(i_txt_transp);
	SetOSDBgTransp(i_bg_transp);
	SetOSDGain(atoi(gain_g));
	SetOSDPosition(atoi(startX_g), atoi(startY_g));
	SetOSD(text_g, i_timestamp, i_enable);		

	return 0;
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

		if((ret = snx_isp_init(m2m) != 0))
			goto err_init;
		snx_isp_start(m2m);
	}

	setOSDAll();

	while(1) {
		snx_isp_read (m2m);

		count_num ++;

		fwrite(m2m->isp_buffers[m2m->isp_index].start,  ((m2m->width*m2m->height*3)/2), 1, fd);
		snx_isp_reset (m2m);

    		frame_num--;
		if(frame_num == 0)
			break;
	}

	SetOSDEnable(0x0);

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

void sigterm_handler(int sig)
{
	SetOSDEnable(0x0);
	exit(0);
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

	signal(SIGINT, sigterm_handler); /* Interrupt (ANSI). */
	signal(SIGQUIT, sigterm_handler); /* Quit (POSIX). */
	signal(SIGTERM, sigterm_handler); /* Termination (ANSI). */

	ret = pthread_create(&m2m_thread, NULL, (void *)snx_m2m_thread, m2m);
	if(ret != 0) {
		fprintf(stderr, "exit thread creation failed");   
	}
	pthread_join(m2m_thread,NULL);

	return 0;
}

