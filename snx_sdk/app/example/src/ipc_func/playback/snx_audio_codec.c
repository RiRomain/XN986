/**
 *
 * SONiX SDK Example Code
 * Category: Audio capture and playback flow
 * File: snx_audio_codec.c
 *
 * NOTE:
 *       
 */
 
#include "snx_audio_codec.h"

int audio_pb_format_check( snx_audio_stream_conf_t *stream)
{
	if (stream == NULL) return -1;
	
	switch (stream->format) {
	
		case SND_PCM_FORMAT_S8:
		case SND_PCM_FORMAT_U8:
			stream->frame_number = 160;
			strcpy(stream->dev_name, "hw:0");
			break;
		case SND_PCM_FORMAT_S16:
		case SND_PCM_FORMAT_U16:
			stream->frame_number = 800;
			if (stream->sample_rate == 8000) {
				strcpy(stream->dev_name, "snx_audio_pcm_pb_ex");
			} else if (stream->sample_rate == 16000) {
				strcpy(stream->dev_name, "snx_audio_pcm_pb_16k_ex");
			} else if (stream->sample_rate == 32000) {
				strcpy(stream->dev_name, "snx_audio_pcm_pb_32k_ex");
			} else if (stream->sample_rate == 48000) {
				strcpy(stream->dev_name, "snx_audio_pcm_pb_48k_ex");
			} else {
				printf ("Wrong sample_rate: %d\n",stream->sample_rate);
				return -1;
			}
			break;
/*  Alsa conf modification is needed */
		case SND_PCM_FORMAT_A_LAW:
			stream->frame_number = 800;
			strcpy(stream->dev_name, "snx_audio_alaw_pb_ex");
			break;
		case SND_PCM_FORMAT_MU_LAW:
			stream->frame_number = 160;
			strcpy(stream->dev_name, "snx_audio_mulaw_pb_ex");
			break;
		case SND_PCM_FORMAT_G726_16:
			stream->frame_number = 160;
			strcpy(stream->dev_name, "snx_audio_g726_pb_ex");
			break;
		case SND_PCM_FORMAT_G726_24:
			stream->frame_number = 160;
			strcpy(stream->dev_name, "snx_audio_g726_pb_ex");
			break;
		case SND_PCM_FORMAT_G726_32:
			stream->frame_number = 160;
			strcpy(stream->dev_name, "snx_audio_g726_pb_ex");
			break;
		case SND_PCM_FORMAT_G726_40:
			stream->frame_number = 160;
			strcpy(stream->dev_name, "snx_audio_g726_pb_ex");
			break;
		case SND_PCM_FORMAT_G722:
			stream->frame_number = 160;
			strcpy(stream->dev_name, "snx_audio_g722_pb_ex");
			break;

		default:
			printf("[SNX-AUDIO]WRONG AUDIO FORMAT\n");
			return -1;
	}
	return 0;
}

void snx_audio_pb_init(void *arg){
	int rc;
	int size;
    int dir = 0;
	snx_audio_stream_conf_t *stream = (snx_audio_stream_conf_t *)arg;
    snd_pcm_hw_params_t *params = stream->params;
	
	rc = snd_pcm_open(&(stream->handle), stream->dev_name, SND_PCM_STREAM_PLAYBACK, stream->open_mode);
	if (rc < 0) {  
		fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(rc));  
        exit(1);  
    }  

    /* Allocate a hardware parameters object. */  
    snd_pcm_hw_params_alloca(&params);  
  
    /* Fill it in with default values. */  
    snd_pcm_hw_params_any(stream->handle, params);  
  
    /* Set the desired hardware parameters. */  
  
    /* Interleaved mode */  
    snd_pcm_hw_params_set_access(stream->handle, params, SND_PCM_ACCESS_RW_INTERLEAVED );
	
	
	stream->_format_bits = snd_pcm_format_width(stream->format);
    snd_pcm_hw_params_set_format(stream->handle, params, stream->format);  
  
    /* one channel (mono) */  
    snd_pcm_hw_params_set_channels(stream->handle, params, stream->channel);  
  
    /* 8KHz sampling rate */  
    snd_pcm_hw_params_set_rate(stream->handle, params, stream->sample_rate, dir);  
  
    snd_pcm_hw_params_set_period_size(stream->handle,params, stream->frame_number, dir);  
  
	printf("[SNX-AUDIO] frame number : %d, format_bits: %d\n", stream->frame_number, stream->_format_bits); 
	
    /* Write the parameters to the driver */  
    rc = snd_pcm_hw_params(stream->handle, params);  
    if (rc < 0) {  
        fprintf(stderr,  
            "unable to set hw parameters: %s\n",  
            snd_strerror(rc));
        exit(1);
    }

#if 1   //haowei
     {
         snd_pcm_uframes_t frames_info;
         printf("\n----------- ALSA HW -----------\n");
         snd_pcm_hw_params_get_period_size(params, &frames_info, 0 );
         printf("period size: %d\n", (int)frames_info);
         snd_pcm_hw_params_get_buffer_size(params, &frames_info );
         printf("buffer size: %d\n", (int)frames_info);
         printf("device Name: %s\n", stream->dev_name);
         printf("\n-----------------------------\n");
     }

 #endif

}

void snx_audio_pb_uninit(void *arg){
	snx_audio_stream_conf_t *stream = (snx_audio_stream_conf_t *)arg;
    if (stream->handle) {
	snd_pcm_drain(stream->handle);  
    snd_pcm_close(stream->handle);  
	}
}
