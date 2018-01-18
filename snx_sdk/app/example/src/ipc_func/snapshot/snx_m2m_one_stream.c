/**
 *
 * SONiX SDK Example Code
 * Category: Video Encode
 * File: snx_m2m_one_stream.c
 * Usage: 
 *		 1. The result video file would be saved in /tmp directory
 *		 2. Execute snx_m2m_one_stream (/usr/bin/snx_m2m_one_stream)
 *       3. The video encoding would start
 *       4. After the frame_num is reached, the encoding would stop
 *       5. Check the file recorded in the SD card by using VLC
 * NOTE:
 *       Recording all streams to SD card would cause the target framerate can
 *       not be reached because of the bandwidth leakage of SD card.
 */
#include "snx_video_codec.h"

/*-----------------------------------------------------------------------------
 * Example Code Configuration
 *----------------------------------------------------------------------------*/

#define SNX_SNAPSHOT_VER	"V0.1.1"

/*----------------- Stream 1 M2M stream --------------------------------------*/
#define FORMAT			V4L2_PIX_FMT_MJPEG	// V4L2_PIX_FMT_H264 or V4L2_PIX_FMT_MJPEG or V4L2_PIX_FMT_SNX420
											// Video Encode Format

#define WIDTH			1280				// Video Encoded Frame Width
#define HEIGHT			720					// Video Encoded Frame Height
#define FRAME_RATE		30					// Video Encoded Frame Rate
#define IMG_DEV_NAME	"/dev/video0"		// ISP Device Node
#define CAP_DEV_NAME	"/dev/video1"		// Video Codec Device Node for M2M
#define FRAME_NUM		3  					// Video Encoded Frame Number
#define SCALE			1					// 1: 1, 2: 1/2, 4: 1/4 scaling down
#define FILENAME		"/tmp"				// Video Encoded File Path
/*#define SNAPSHOT_TRIGGER "/tmp/snapshot_en"*/
#define YUV_DIV_RATE	5					// Default YUV Rate divider

/*-----------------------------------------------------------------------------
 * GLOBAL Variables
 *----------------------------------------------------------------------------*/

char quit = 0;
/*-----------------------------------------------------------------------------
 * Example Code Flow
 *----------------------------------------------------------------------------*/
void sighandler(int n)
{ 
	printf("Signal received (%d)\n", n);
	quit = 1;
}

/*
	The entrance of this file.
	One M2M streams would be created.
	The configuration can set on the setting definitions above. 
	After the stream conf are done, the thread of the stream would be created.
	The bitstreams of the stream will be record on the SD card.
*/

int main(int argc, char **argv)
{
	int ret = 0;
	char outputpath[120];
	char syscmd[120];
	stream_conf_t *stream1 = NULL;
	stream1 = malloc(sizeof(stream_conf_t));
	memset(stream1, 0, sizeof(stream_conf_t));
	struct snx_m2m *m2m = &stream1->m2m;
	// decode parameters
	int c = 0;
	int yuv_output = 0;

	struct stat tst;

	/*--------------------------------------------------------
		stream config setup
	---------------------------------------------------------*/
	memset(syscmd, 0x00, sizeof(outputpath));
	

	m2m->isp_fps = FRAME_RATE;
	m2m->m2m_buffers = 2;
	m2m->ds_font_num = DS_BUFFER_SIZE;
	m2m->scale = SCALE;
	m2m->codec_fps = FRAME_RATE;
	m2m->width = WIDTH;
	m2m->height = HEIGHT;
	m2m->gop = m2m->isp_fps;
	m2m->codec_fmt = FORMAT;
	m2m->out_mem = V4L2_MEMORY_USERPTR;
	m2m->m2m = 0;								//Default its a Capture Path
	m2m->isp_mem = V4L2_MEMORY_MMAP;
    m2m->isp_fmt = V4L2_PIX_FMT_SNX420;
    m2m->qp = 60;
    m2m->dyn_fps_en = 0;

	strcpy(m2m->isp_dev,IMG_DEV_NAME);
	strcpy(m2m->codec_dev,CAP_DEV_NAME);
	/*strcpy(outputpath,"/tmp");*/
	strcpy(outputpath,"/tmp/www");

	stream1->outputpath = outputpath;
	stream1->frame_num = FRAME_NUM;
	stream1->debug = 0;
	stream1->live = 1;
	stream1->yuv_frame = YUV_DIV_RATE;
	stream1->y_only = 0;

	while ((c = getopt (argc, argv, "hmo:i:f:W:H:q:n:s:drv:y")) != -1)
	{
		switch (c)
		{
			case 'o':	strcpy(outputpath, optarg); break;
			case 'm':	m2m->m2m = 1; break;
			case 'i':	m2m->isp_fps = atoi(optarg); break; 
			case 'f':	m2m->codec_fps = atoi(optarg); break; 
			case 'W':	m2m->width = atoi(optarg); break;
			case 'H':	m2m->height = atoi(optarg); break;
			case 'q':	m2m->qp = atoi(optarg); break;
			case 'n':	stream1->frame_num = atoi(optarg); break;
			case 's':	m2m->scale = atoi(optarg); break;
			case 'd':	stream1->debug = 1; break;
			case 'r':	yuv_output = 1; break;
			case 'v':	stream1->yuv_frame = atoi(optarg); break;
			case 'y':	stream1->y_only = 1; break;
			case 'h':
			default:
			{
				printf("Usage: %s [options]/n\n"   
				"Version: %s\n"
				"Options:\n"
				"\t-h			Print this message\n"
		        "\t-m			m2m path enable (default is Capture Path)\n"
		        "\t-o			outputPath (default is /tmp)\n"
		        "\t-i			isp fps (Only in M2M path, default is 30)\n"
		        "\t-f			codec fps (default is 30 fps, NOT more than M2M path)\n"
		        "\t-W			Capture Width (Default is 1280, depends on M2M path)\n"
		        "\t-H			Capture Height (Default is 720, depends on M2M path)\n"
		        "\t-q			JPEG QP (Default is 60)\n"
		        "\t-n			Num of Frames to capture (Default is 3)\n"
		        "\t-s			scaling mode (default is 1,  1: 1, 2: 1/2, 4: 1/4 )\n"
		        "\t-r			YUV data output enable\n"
		        "\t-v			YUV capture rate divider (default is 5)\n"
		        "\tM2M Example:   %s -m -i 30 -f 30 -q 120 /dev/video1\n"
		        "\tcapture Example:   %s -n 1 -q 120 /dev/video1\n"
				"\n"
				"", argv[0],SNX_SNAPSHOT_VER, argv[0],argv[0]);   
				exit(0);
			}
		}
	}

	if (optind<argc)
	{
		strcpy(m2m->codec_dev,argv[optind]);
	}

	if (yuv_output) {
		if (stream1->yuv_frame <= 0) {
			printf("Wrong YUV Rate divider parameter (%d)\n", stream1->yuv_frame);
			exit(0);
		}
		if (stream1->yuv_frame >  m2m->isp_fps) {
			printf("YUV rate divider should <= isp fps (%d)\n", stream1->yuv_frame);
			exit(0);
		}

		m2m->codec_fmt = V4L2_PIX_FMT_SNX420;
	}
	
			printf("\n\n------- V4L2 Infomation -------- \n");
			printf("Device Name: %s\n", m2m->codec_dev);
			printf("m2m_en: %d\n", m2m->m2m);
			printf("codec_dev: %s\n", m2m->codec_dev);
			printf("codec_fps: %d\n", m2m->codec_fps);
			if(m2m->m2m)
				printf("isp_fps: %d\n", m2m->isp_fps);
			printf("width: %d\n", m2m->width);
			printf("height: %d\n", m2m->height);
			printf("scale: %d\n", m2m->scale);
			printf("qp: %d\n", m2m->qp);
			printf("outputpath: %s\n", outputpath);
			if(yuv_output) {
				printf("YUV Output enable: %d\n", yuv_output);
				printf("YUV Rate divider: %d\n", stream1->yuv_frame);
				printf("Y only output: %d\n", stream1->y_only);
			}
			printf("\n----------------------------- \n\n");

	
	signal(SIGINT,sighandler);
	/*--------------------------------------------------------
		Start 
	---------------------------------------------------------*/
	ret = pthread_create(&stream1->stream_thread, NULL, (void *)snx_m2m_cap_rc_flow, stream1);
	if(ret != 0) {
		perror_exit(1, "exit thread creation failed");   
	}
	pthread_detach(stream1->stream_thread);

	while (1) {

		if (quit) {
			stream1->live = 0;
			break;
		}

		if ((stream1->state == 0)) {
			stream1->state = 1;
			printf("Start to snapshot\n");
		/*	memset(syscmd, 0x00, sizeof(syscmd)); */
			/*sprintf(syscmd, "rm -f %s", SNAPSHOT_TRIGGER); */
		/*	system(syscmd);      */
		}

		if ((stream1->state == 1)) {
			printf("Snapshot is under working\n");
			/*memset(syscmd, 0x00, sizeof(syscmd)); */
			/*sprintf(syscmd, "rm -f %s", SNAPSHOT_TRIGGER);*/
			/*system(syscmd);  */
		}

		sleep(1);
	}
	
	/*--------------------------------------------------------
		Record End 
	---------------------------------------------------------*/
	/* Free the stream configs */
	free(stream1);
	
    return ret;
}
