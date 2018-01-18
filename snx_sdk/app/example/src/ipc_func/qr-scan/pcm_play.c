/*
* usage: pcm_play [file name]
*/

#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

#define SAMPLE_RATE 8000
#define CHANNLE 1
#define FRAMES_SIZE 32
#define FORMAT SND_PCM_FORMAT_S16_LE
#define PER_SAMPLE 2

#if SOUND_FOR_DEBUG
//#define DEVICE	"snx_audio_pcm"
#define DEVICE	"hw:0,0"
#else
#define DEVICE	"snx_audio_pcm_pb_ex"
#endif

int pcm_play(char *audio_src) {
  int rc, size, dir;
  snd_pcm_t *handle;
  snd_pcm_hw_params_t *params;
  snd_pcm_uframes_t frames, ret, read_frames;
  char *buffer;
  char *ptr_buffer;
  int return_value = 0;
  
  FILE *fp = fopen(audio_src, "r");
  if(fp == NULL){
  	fprintf(stderr, "open audio file %s failed\n",audio_src);
  	return_value = -1;
  	goto end;
  }

  /* Open PCM device for playback. */
  rc = snd_pcm_open(&handle, DEVICE, SND_PCM_STREAM_PLAYBACK, 0);
  if (rc < 0) {
  	printf("%s\n",DEVICE);
  	fprintf(stderr,"unable to open pcm device: %s\n",snd_strerror(rc));
  	return_value = -1;
  	goto end;
  }

  /* Allocate a hardware parameters object. */
  snd_pcm_hw_params_alloca(&params);

  /* Fill it in with default values. */
  rc = snd_pcm_hw_params_any(handle, params);
  if (rc < 0) {
	perror("snd_pcm_hw_params_any");
	return_value = -1;
	goto end;
  }

  /* Interleaved mode */
  rc = snd_pcm_hw_params_set_access(handle, params,SND_PCM_ACCESS_RW_INTERLEAVED);
  if (rc < 0) {
	perror("snd_pcm_hw_params_set_access");
	return_value = -1;
	goto end;
  }

  /* Signed 16-bit little-endian format */
  rc = snd_pcm_hw_params_set_format(handle, params, FORMAT);
  if (rc < 0) {
	perror("snd_pcm_hw_params_set_format");
	return_value = -1;
	goto end;
  }

  /* One channels (stereo) */
  rc = snd_pcm_hw_params_set_channels(handle, params, CHANNLE);
  if (rc < 0) {
	perror("snd_pcm_hw_params_set_channels");
	return_value = -1;
	goto end;
  }

  /* 8000 bits/second sampling rate  */
  rc = snd_pcm_hw_params_set_rate(handle, params,SAMPLE_RATE, 0);
  if (rc < 0) {
	perror("snd_pcm_hw_params_set_rate_near");
	return_value = -1;
	goto end;
  }

  /* Set period size to 32 frames. */
  frames = FRAMES_SIZE;

  /* Write the parameters to the driver */
  rc = snd_pcm_hw_params(handle, params);
  if (rc < 0) {
  	fprintf(stderr,"unable to set hw parameters: %s\n",snd_strerror(rc));
  	return_value = -1;
  	goto end;
  }

  /* Use a buffer large enough to hold one period */
  size = frames * PER_SAMPLE *CHANNLE; /* 2 bytes/sample, 1 channels */
  ptr_buffer = buffer = (char *) malloc(size);
  if(buffer == NULL){
  	fprintf(stderr, "malloc failed\n");
  	return_value = -1;
  	goto end;
  }

  while (1) 
  {
		ret = fread(buffer, 1, size, fp);
		if(ret <= 0) 
		{
			fprintf(stderr, "end of file on input\n");
			break;
		}
		//10. write data to device
		read_frames = ret / PER_SAMPLE /CHANNLE;
		ptr_buffer = buffer;
		while(read_frames > 0)
		{
			ret = snd_pcm_writei(handle, ptr_buffer, read_frames);
			if (ret == -EPIPE) {
				//EPIPE means underrun
				fprintf(stderr, "underrun occurred\n");
				//complete setting of hardware£¬make device prepared
				snd_pcm_prepare(handle);
			}
			else if ( ret == -EAGAIN ) {
				usleep(20*1000);
			}
			else if (ret < 0) {
				fprintf(stderr,"error from writei: %s\n",snd_strerror(ret)); 
				return_value = -1;
				goto end;
			}
			else if (ret > 0) {
				ptr_buffer += ret;
				read_frames -= ret;
			}
		}
 }	


end:
  if(buffer != NULL){
	free(buffer);
  }
  if(fp != NULL){
	fclose(fp);
  }
  snd_pcm_drain(handle);
  snd_pcm_close(handle);

  return return_value;
}
