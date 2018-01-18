/**
 *
 * SONiX SDK Example Code
 * Category: audio codec
 * File: snx_audio_codec.h
 * NOTE:
 *       
 */
#ifndef AUDIO_H_
#define AUDIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>        
#include <malloc.h>
#include <alsa/asoundlib.h>  

typedef struct snx_audio_stream_conf_s {
	char dev_name[30];
	snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
	snd_pcm_format_t  format;
	snd_pcm_uframes_t frame_number;
	int open_mode;
	unsigned int channel;
    unsigned int sample_rate;
    int _format_bits;
    FILE *dst_fp;
} snx_audio_stream_conf_t;


int audio_pb_format_check( snx_audio_stream_conf_t *stream);
void snx_audio_pb_init (void *arg);
void snx_audio_pb_uninit(void *arg);

#ifdef __cplusplus
 }

#endif
#endif /* PLAY_H_ */
