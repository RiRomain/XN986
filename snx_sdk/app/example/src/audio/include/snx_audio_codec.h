/**
 *
 * SONiX SDK Example Code
 * Category: audio codec
 * File: snx_audio_codec.h
 * NOTE:
 *       
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>        
#include <pthread.h>
#include <getopt.h>             /* getopt_long() */
#include <fcntl.h>              /* low-level i/o */
#include <malloc.h>
#include <alsa/asoundlib.h>  
#include "snx_common.h"

#define SNX_AUDIO_CAP_TIME			10					//Capture time (second)

typedef struct snx_audio_stream_conf_s {
	char dev_name[20];
	snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
	snd_pcm_format_t  format;
	snd_pcm_uframes_t frame_number;
	int open_mode;
	unsigned int channel;
    unsigned int sample_rate;
	int buffer_size;
	char *buffer;
	FILE *fd;
	pthread_t stream_thread;
} snx_audio_stream_conf_t;

int audio_cap_format_check (snx_audio_stream_conf_t *stream);

int audio_pb_format_check( snx_audio_stream_conf_t *stream);

void snx_audio_cap_flow (void *arg);

void snx_audio_pb_flow (void *arg);


