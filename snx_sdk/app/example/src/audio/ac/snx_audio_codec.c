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

int audio_cap_format_check( snx_audio_stream_conf_t *stream)
{
	if (stream == NULL) return -1;
	
	switch (stream->format) {
	
		case SND_PCM_FORMAT_S8:
		case SND_PCM_FORMAT_U8:
			stream->frame_number = 40;
			strcpy(stream->dev_name, "hw:0,0");
			break;
		case SND_PCM_FORMAT_S16:
		case SND_PCM_FORMAT_U16:
			stream->frame_number = 80;
			if (stream->sample_rate == 8000) {
				strcpy(stream->dev_name, "snx_audio_pcm_ex");
			} else if (stream->sample_rate == 16000) {
				strcpy(stream->dev_name, "snx_audio_pcm_16k_ex");
			} else if (stream->sample_rate == 32000) {
				strcpy(stream->dev_name, "snx_audio_pcm_32k_ex");
			} else if (stream->sample_rate == 48000) {
				strcpy(stream->dev_name, "snx_audio_pcm_48k_ex");
			} else {
				printf ("Wrong sample_rate: %d\n",stream->sample_rate);
				return -1;
			}
			break;
		case SND_PCM_FORMAT_A_LAW:
			stream->frame_number = 80;
			strcpy(stream->dev_name, "snx_audio_alaw_ex");
			break;
		case SND_PCM_FORMAT_MU_LAW:
			stream->frame_number = 80;
			strcpy(stream->dev_name, "snx_audio_mulaw_ex");
			break;
		case SND_PCM_FORMAT_G726_16:
			stream->frame_number = 80;
			strcpy(stream->dev_name, "snx_audio_g726_ex");
			break;
		case SND_PCM_FORMAT_G726_24:
			stream->frame_number = 80;
			strcpy(stream->dev_name, "snx_audio_g726_ex");
			break;
		case SND_PCM_FORMAT_G726_32:
			stream->frame_number = 80;
			strcpy(stream->dev_name, "snx_audio_g726_ex");
			break;
		case SND_PCM_FORMAT_G726_40:
			stream->frame_number = 160;
			strcpy(stream->dev_name, "snx_audio_g726_ex");
			break;
		case SND_PCM_FORMAT_G722:
			stream->frame_number = 80;
			strcpy(stream->dev_name, "snx_audio_g722_ex");
			break;
		default:
			printf("[SNX-AUDIO]WRONG AUDIO FORMAT\n");
			return -1;
	}
	return 0;
}

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
			stream->frame_number = 160;
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
			stream->frame_number = 160;
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

void snx_audio_cap_flow (void *arg)
{
	int rc;
	int size;
    int dir = 0;
	int _format_bits;
	snx_audio_stream_conf_t *stream = (snx_audio_stream_conf_t *)arg;
	snd_pcm_t *handle = stream->handle;
    snd_pcm_hw_params_t *params = stream->params;
	FILE *fd = stream->fd;
	char *buffer = stream->buffer;
	snd_pcm_uframes_t cached_frames = 0;
	snd_pcm_uframes_t frames;
	int ptr = 0;
	
	rc = snd_pcm_open(&handle, stream->dev_name, SND_PCM_STREAM_CAPTURE, stream->open_mode);
	if (rc < 0) {  
		fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(rc));  
        exit(1);  
    }  

    /* Allocate a hardware parameters object. */  
    snd_pcm_hw_params_alloca(&params);  
  
    /* Fill it in with default values. */  
    snd_pcm_hw_params_any(handle, params);  
  
    /* Set the desired hardware parameters. */  
  
    /* Interleaved mode */  
    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED );
	
	
	_format_bits = snd_pcm_format_width(stream->format);
    snd_pcm_hw_params_set_format(handle, params, stream->format);  
  
    /* one channel (mono) */  
    snd_pcm_hw_params_set_channels(handle, params, stream->channel);  
  
    /* 8KHz sampling rate */  
    snd_pcm_hw_params_set_rate(handle, params, stream->sample_rate, dir);  
  
    snd_pcm_hw_params_set_period_size(handle,params, stream->frame_number, dir);  
  
	printf("[SNX-AUDIO] frame number : %d, format_bits: %d\n", stream->frame_number, _format_bits); 
	
    /* Write the parameters to the driver */  
    rc = snd_pcm_hw_params(handle, params);  
    if (rc < 0) {  
        fprintf(stderr,  
            "unable to set hw parameters: %s\n",  
            snd_strerror(rc));
        exit(1);
    }
	
	/* Use a buffer large enough to hold 2 period */  
    size = ((stream->frame_number * stream->channel * _format_bits) >> 3) << 1; 
    buffer = (char *) malloc(size);
  
    /* We want to loop for 5 seconds */  
	frames = SNX_AUDIO_CAP_TIME * stream->sample_rate; //frame numbers
	printf("[SNX-AUDIO] %d second(s) : %d frames\n", SNX_AUDIO_CAP_TIME, frames); 
	
    while (frames) {
		unsigned int cached_frames_size;
		//int ptr = 0;
		
        rc = snd_pcm_readi(handle, buffer+ptr, ((stream->frame_number << 1) - cached_frames) ); //get 2 period
		
		if (rc < 0) {
			if (rc == -EPIPE) {  
				/* EPIPE means overrun */  
				//fprintf(stderr, "overrun occurred\n");  
				snd_pcm_prepare(handle);
			} else if ( rc == -EAGAIN ) {
				snd_pcm_wait(handle, 1000);
			}
			//fprintf(stderr,  "error from read: %s\n", snd_strerror(rc));
			continue;
        }
		
		ptr += (rc * stream->channel * _format_bits >> 3);
		cached_frames = cached_frames + rc;
		
		if ( cached_frames == (stream->frame_number << 1) ) {
		
			if (frames >= cached_frames ) {
				cached_frames_size = cached_frames * stream->channel * _format_bits >> 3;
				frames = frames - cached_frames;
			} else {
				cached_frames_size = frames * stream->channel * _format_bits >> 3;
				frames = 0;
			}
			
			//printf(" rc = %d, cached_frame : %d\n", rc, cached_frames_size);
			fwrite(buffer,  cached_frames_size, 1, fd);
			cached_frames = 0;
			ptr = 0;
		}
    }

    snd_pcm_drain(handle);  
    snd_pcm_close(handle);  
    free(buffer);  
}


void snx_audio_pb_flow (void *arg)
{
	int rc;
	int size;
    int dir = 0;
	int _format_bits;
	snx_audio_stream_conf_t *stream = (snx_audio_stream_conf_t *)arg;
	snd_pcm_t *handle = stream->handle;
    snd_pcm_hw_params_t *params = stream->params;
	FILE *fd = stream->fd;
	char *buffer = stream->buffer;
	
	rc = snd_pcm_open(&handle, stream->dev_name, SND_PCM_STREAM_PLAYBACK, stream->open_mode);
	if (rc < 0) {  
		fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(rc));  
        exit(1);  
    }  

    /* Allocate a hardware parameters object. */  
    snd_pcm_hw_params_alloca(&params);  
  
    /* Fill it in with default values. */  
    snd_pcm_hw_params_any(handle, params);  
  
    /* Set the desired hardware parameters. */  
  
    /* Interleaved mode */  
    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED );
	
	
	_format_bits = snd_pcm_format_width(stream->format);
    snd_pcm_hw_params_set_format(handle, params, stream->format);  
  
    /* one channel (mono) */  
    snd_pcm_hw_params_set_channels(handle, params, stream->channel);  
  
    /* 8KHz sampling rate */  
    snd_pcm_hw_params_set_rate(handle, params, stream->sample_rate, dir);  
  
    snd_pcm_hw_params_set_period_size(handle,params, stream->frame_number, dir);  
  
	printf("[SNX-AUDIO] frame number : %d, format_bits: %d\n", stream->frame_number, _format_bits); 
	
    /* Write the parameters to the driver */  
    rc = snd_pcm_hw_params(handle, params);  
    if (rc < 0) {  
        fprintf(stderr,  
            "unable to set hw parameters: %s\n",  
            snd_strerror(rc));
        exit(1);
    }
  
    /* Use a buffer large enough to hold 2 period */  
    size = ((stream->frame_number * stream->channel * _format_bits) >> 3) << 1; 
    buffer = (char *) malloc(size);

    while (!feof(fd)) {
		unsigned int read_frame_size;
		unsigned int read_frame, cached_frames = 0;
		unsigned int cached_frame_size=0;
		int ptr = 0;
		
		read_frame_size = fread(buffer , 1, size, fd);
		read_frame = read_frame_size * 8 / _format_bits;
		
		printf(" read_frame = %d, read_frame_size : %d\n", read_frame, read_frame_size);
		while ( read_frame ) {
			rc = snd_pcm_writei(handle, buffer+ptr, (read_frame - cached_frames));
			
			if (rc == -EPIPE) {  
				/* EPIPE means overrun */  
				fprintf(stderr, "overrun occurred\n");  
				snd_pcm_prepare(handle);
				continue;
			} else if (rc < 0) {  
				fprintf(stderr,  "error from read: %s\n", snd_strerror(rc));
				continue;
			}

			ptr += (rc * stream->channel * _format_bits >> 3);
			cached_frames = cached_frames + rc;
			
			if (read_frame >= rc ) {
				cached_frame_size = cached_frames * stream->channel * _format_bits >> 3;
				read_frame = read_frame - rc;
			} 
			
			printf(" rc = %d, cached_frame : %d\n", rc, cached_frame_size);
			
		}
    }
	
    snd_pcm_drain(handle);  
    snd_pcm_close(handle);  
    free(buffer);  

}
