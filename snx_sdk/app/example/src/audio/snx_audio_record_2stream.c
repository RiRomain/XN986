/**
 *
 * SONiX SDK Example Code
 * Category: Audio Codec
 * File: snx_audio_record.c
 * Usage: 
 *		 1. The record result would be saved in /tmp folder.
 *		 2. Execute snx_audio_record (/usr/bin/example/snx_audio_record)
 *       3. The encoding would start and write to the sd card.
 *       4. It would stop after 10 seconds encoding.
 *		 5. Copy the audio data from /tmp folder to the sdcard 
 * NOTE:
 * 		 Due to the hw limitation, we do NOT Support 8bit capture and 16bit capture
 *		 at the same time.
 */
#include "snx_audio_codec.h"

/*-----------------------------------------------------------------------------
 * Example Code Configuration
 *----------------------------------------------------------------------------*/

/*----------------- Stream 1 record stream -----------------------------------*/
#define STREAM1_CAP_FORMAT			SND_PCM_FORMAT_S16
									/*  support the following encode format
										SND_PCM_FORMAT_S8, SND_PCM_FORMAT_U8
										SND_PCM_FORMAT_S16, SND_PCM_FORMAT_U16
										SND_PCM_FORMAT_A_LAW, SND_PCM_FORMAT_MU_LAW
										SND_PCM_FORMAT_G726_16, SND_PCM_FORMAT_G726_24
										SND_PCM_FORMAT_G726_32, SND_PCM_FORMAT_G726_40
										SND_PCM_FORMAT_G722
									*/
#define STREAM1_CAP_FILENAME		"/tmp/audio1.raw"

/*----------------- Stream 2 record stream -----------------------------------*/
#define STREAM2_CAP_FORMAT			SND_PCM_FORMAT_S16
									/*  support the following encode format
										SND_PCM_FORMAT_S8, SND_PCM_FORMAT_U8
										SND_PCM_FORMAT_S16, SND_PCM_FORMAT_U16
										SND_PCM_FORMAT_A_LAW, SND_PCM_FORMAT_MU_LAW
										SND_PCM_FORMAT_G726_16, SND_PCM_FORMAT_G726_24
										SND_PCM_FORMAT_G726_32, SND_PCM_FORMAT_G726_40
										SND_PCM_FORMAT_G722
									*/
#define STREAM2_CAP_FILENAME		"/tmp/audio2.raw"

/*-----------------------------------------------------------------------------
 * Example Code Flow
 *----------------------------------------------------------------------------*/

/* 
  This example reads from the specifiend PCM device 
  and writes to SD card for 10 seconds of data. 
 */  

int main()   
{

	int ret = 0;
	snx_audio_stream_conf_t *cap1 = NULL;
	snx_audio_stream_conf_t *cap2 = NULL;
	
	/*--------------------------------------------------------
		stream 1 setup
	---------------------------------------------------------*/
	cap1 = malloc(sizeof(snx_audio_stream_conf_t));

	cap1->format = STREAM1_CAP_FORMAT;
	
	ret = audio_cap_format_check(cap1);
	if (ret < 0) {
		printf("[SNX-AUDIO] audio format check error\n");
		goto FREE_CAP;
	}
	
	cap1->open_mode = SND_PCM_NONBLOCK;
	cap1->channel = 1;
	cap1->sample_rate = 8000;
	
	cap1->fd = fopen(STREAM1_CAP_FILENAME,"wb");
	if (cap1->fd <=0 ) {
		printf("[SNX-AUDIO] capture file open failed\n");
		ret = -1;
		goto FREE_CAP;
	}
	
	/*--------------------------------------------------------
		stream 2 setup
	---------------------------------------------------------*/
	cap2 = malloc(sizeof(snx_audio_stream_conf_t));

	cap2->format = STREAM2_CAP_FORMAT;
	
	ret = audio_cap_format_check(cap2);
	if (ret < 0) {
		printf("[SNX-AUDIO] audio format check error\n");
		fclose(cap1->fd);
		goto FREE_CAP;
	}
	
	cap2->open_mode = SND_PCM_NONBLOCK;
	cap2->channel = 1;
	cap2->sample_rate = 8000;
	
	cap2->fd = fopen(STREAM2_CAP_FILENAME,"wb");
	if (cap2->fd <=0 ) {
		printf("[SNX-AUDIO] capture file open failed\n");
		ret = -1;
		fclose(cap1->fd);
		goto FREE_CAP;
	}

	/*--------------------------------------------------------
		Start 
	---------------------------------------------------------*/
	
	ret = pthread_create(&cap1->stream_thread, NULL, (void *)snx_audio_cap_flow, cap1);
	ret += pthread_create(&cap2->stream_thread, NULL, (void *)snx_audio_cap_flow, cap2);
	if(ret != 0) {
		perror_exit(1, "exit thread creation failed");
		goto CLOSE_FD;
	}
	
	pthread_join(cap2->stream_thread,NULL);
	pthread_join(cap1->stream_thread,NULL);

CLOSE_FD:
	fclose(cap2->fd);
	fclose(cap1->fd);

FREE_CAP:
	free(cap2);
	free(cap1);
	
    return ret;
}