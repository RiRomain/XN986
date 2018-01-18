/**
 *
 * SONiX SDK Example Code
 * Category: Video Encode
 * File: snx_m2m_dynamic_fps_bps.c
 * Usage: 
 *	 1. The result video file would be saved in /tmp directory
 *	 2. Execute snx_m2m_one_stream (/usr/bin/snx_m2m_dynamic_fps_bps)
 *       3. The video encoding would start
 *       4. After the frame_num is reached, the encoding would stop
 *       5. Check the file recorded in the SD card by using VLC
 *	 6. Modify bit rate and frame rate from console ex:INPUT >>>f:15 b:1024000
 * NOTE:
 *       Recording all streams to SD card would cause the target framerate can
 *       not be reached because of the bandwidth leakage of SD card.
 */
#include "snx_video_codec.h"

/*-----------------------------------------------------------------------------
 * Example Code Configuration
 *----------------------------------------------------------------------------*/

/*----------------- Stream 1 M2M stream --------------------------------------*/
#define FORMAT			V4L2_PIX_FMT_H264	// V4L2_PIX_FMT_H264 or V4L2_PIX_FMT_MJPEG or V4L2_PIX_FMT_SNX420
											// Video Encode Format

#define WIDTH			640			// Video Encoded Frame Width
#define HEIGHT			480			// Video Encoded Frame Height
#define FRAME_RATE		30					// Video Encoded Frame Rate
#define IMG_DEV_NAME		"/dev/video0"		// ISP Device Node
#define CAP_DEV_NAME		"/dev/video1"		// Video Codec Device Node for M2M
#define FRAME_NUM		900			// Video Encoded Frame Number
#define SCALE			1			// 1: 1, 2: 1/2, 4: 1/4 scaling down
#define FILENAME		"/tmp/video1.h264"	// Video Encoded File Path

#define BIT_RATE		256000			// Bitrate control for H264 ONLY


/*-----------------------------------------------------------------------------
 * GLOBAL Variables
 *----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
 * Example Code Flow
 *----------------------------------------------------------------------------*/




int main(int argc, char **argv)
{
	int ret = 0;
	stream_conf_t *stream1 = NULL;
	stream1 = malloc(sizeof(stream_conf_t));
	memset(stream1, 0, sizeof(stream_conf_t));
	struct snx_m2m *m2m = &stream1->m2m;
	
	/*--------------------------------------------------------
		stream config setup
	---------------------------------------------------------*/
	m2m->isp_fps = 30;
	m2m->m2m_buffers = 2;
	m2m->ds_font_num = DS_BUFFER_SIZE;
	m2m->scale = SCALE;
	m2m->codec_fps = FRAME_RATE;
	m2m->width = WIDTH;
	m2m->height = HEIGHT;
	m2m->gop = m2m->isp_fps;
	m2m->codec_fmt = FORMAT;
	m2m->out_mem = V4L2_MEMORY_USERPTR;
	m2m->m2m = 1;											/* M2M stream */
	m2m->isp_mem = V4L2_MEMORY_MMAP;
	m2m->isp_fmt = V4L2_PIX_FMT_SNX420;
	m2m->bit_rate = BIT_RATE;
	
	strcpy(m2m->isp_dev,IMG_DEV_NAME);
	strcpy(m2m->codec_dev,CAP_DEV_NAME);
	
	if(strlen(FILENAME))
		stream1->fd = open(FILENAME, O_RDWR | O_NONBLOCK | O_CREAT);
	stream1->frame_num = FRAME_NUM;

	/*--------------------------------------------------------
		Start 
	---------------------------------------------------------*/
	ret = pthread_create(&stream1->stream_thread, NULL, (void *)snx_m2m_cap_rc_flow, stream1);
	if(ret != 0) {
		perror_exit(1, "exit thread creation failed");   
	}
	pthread_detach(stream1->stream_thread);

	while (1) {
		sleep(1);
		snx_dynamic_modify(stream1);
		if(stream1->frame_num == 0)
			break;
	}
	
	
	/*--------------------------------------------------------
		Record End 
	---------------------------------------------------------*/
	close(stream1->fd);
	/* Free the stream configs */
	free(stream1);
	
    return ret;
}
