/**
 *
 * SONiX SDK Example Code
 * Category: Audio Codec
 * File: snx_audio_playback.c
 * Usage: 
 *		 1. Make sure the nokia.raw is in the same path with snx_audio_playback
 *		 2. Execute snx_audio_playback (/usr/bin/example/snx_audio_playback)
 *       3. The audio data (SNX_AUDIO_PLAYBACK_FILENAME)
 *  		would be loaded and decoded.
 *       4. The playback would be stop when all data have been decoded.
 * NOTE:
 */
#include "snx_audio_codec.h"

/*-----------------------------------------------------------------------------
 * Example Code Configuration
 *----------------------------------------------------------------------------*/

#define SNX_AUDIO_PLAYBACK_FORMAT		"s16"

										/*  support the following encode format
										SND_PCM_FORMAT_S8, SND_PCM_FORMAT_U8
										SND_PCM_FORMAT_S16, SND_PCM_FORMAT_U16
										
										modify alsa conf for more format support
										SND_PCM_FORMAT_A_LAW, SND_PCM_FORMAT_MU_LAW
										SND_PCM_FORMAT_G726_16, SND_PCM_FORMAT_G726_24
										SND_PCM_FORMAT_G726_32, SND_PCM_FORMAT_G726_40
										SND_PCM_FORMAT_G722
										*/
#define SNX_AUDIO_PLAYBACK_FILENAME		"g726.raw"

/*-----------------------------------------------------------------------------
 * Example Code Flow
 *----------------------------------------------------------------------------*/

/* 
  This example reads from the default PCM device 
  and writes to standard output for 5 seconds of data. 
 */  

static const char short_options[] = "hd:f:s:";

static const struct option long_options[] = {
    {"help", no_argument, NULL, 'h'},
	{"device", required_argument, NULL, 'd'},
	{"file", required_argument, NULL, 'f'},
	{"samplerate", required_argument, NULL, 's'},
	
    {0, 0, 0, 0}
};

static void usage(FILE * fp, int argc, char ** argv) {   
    fprintf(fp, "Usage: %s [options]/n\n"   
		"Options:\n"
		"-h                 Print this message\n"
        "-d | --device		alaw, mulaw, g726, s8, s16, u8, u16, g722\n"
        "-f | --file		filename \n"
        "-s | --samplerate	8 / 16 /32 / 48 (kHz, 16K/32K/48K are used for PCM. default is 8k.) \n"
        "Ex:   %s -d alaw -p test.raw\n"
		"\n"

		"", argv[0],argv[0]);   
}
  
int main(int argc, void* argv[])
{    
    int ret = 0;
    int samplerate = 8;
	snx_audio_stream_conf_t *pb = NULL;
	pb = malloc(sizeof(snx_audio_stream_conf_t));

	char device_name[128];
	char audio_src[128];

	memset(device_name, 0x0, 128);
	memset(audio_src, 0x0, 128);

	strcpy(device_name, SNX_AUDIO_PLAYBACK_FORMAT);
	strcpy(audio_src, SNX_AUDIO_PLAYBACK_FILENAME);

	/*--------------------------------------------------------
		Option Value
	---------------------------------------------------------*/
	for (;;)
	{   
		int index;   
		int c;   
		c = getopt_long(argc, argv, short_options, long_options, &index);   

		if (-1 == c)
			break;

		switch (c) {   
			case 0: /* getopt_long() flag */   
				break;
			case 'h':   
				usage(stdout, argc, argv);   
				exit(EXIT_SUCCESS);   
			case 'd':
				sscanf(optarg, "%s", &device_name);
				break;
			case 'f':
				sscanf(optarg, "%s", &audio_src);
				break;
			case 's':
				sscanf(optarg, "%d", &samplerate);
				break;
			default:
				usage(stderr, argc, argv);
				exit(EXIT_FAILURE);  
		}
	}

	if (!strcmp(device_name, "s8"))
		pb->format = SND_PCM_FORMAT_S8;
	else if (!strcmp(device_name, "s16"))
		pb->format = SND_PCM_FORMAT_S16;
	else if (!strcmp(device_name, "u8"))
		pb->format = SND_PCM_FORMAT_U8;
	else if (!strcmp(device_name, "u16"))
		pb->format = SND_PCM_FORMAT_U16;
	else if (!strcmp(device_name, "alaw"))
		pb->format = SND_PCM_FORMAT_A_LAW;
	else if (!strcmp(device_name, "mulaw"))
		pb->format = SND_PCM_FORMAT_MU_LAW;
	else if (!strcmp(device_name, "g726"))
		pb->format = SND_PCM_FORMAT_G726_16;
	else if (!strcmp(device_name, "g722"))
		pb->format = SND_PCM_FORMAT_G722;
	else {

		printf("[SNX-AUDIO] Wrong format\n");
		goto FREE_PB;
	}

	if ((samplerate != 8) && (samplerate != 16) && (samplerate != 32) && (samplerate != 48)) {
		printf("Wrong samplerate : %d\n", samplerate);
		return 0;
	}

	if ( ((samplerate == 16) || (samplerate == 32) || (samplerate == 48) ) 
		&& (pb->format != SND_PCM_FORMAT_S16) && (pb->format != SND_PCM_FORMAT_U16))
	{
		printf("ONLY PCM S16/U16 support 16Khz samplerate\n");
		return 0;
	}
	samplerate = samplerate * 1000;

	printf("[SNX-AUDIO] format: %s, file: %s\n", device_name, audio_src);

	//pb->format = SNX_AUDIO_PLAYBACK_FORMAT;

	pb->open_mode = 0;
	pb->channel = 1;
	pb->sample_rate = samplerate;

	ret = audio_pb_format_check(pb);
	if (ret < 0) {
		printf("[SNX-AUDIO] audio format check error\n");
		goto FREE_PB;
	}
	
	//pb->fd = fopen(SNX_AUDIO_PLAYBACK_FILENAME,"rb");
	pb->fd = fopen(audio_src,"rb");
	if (pb->fd <=0 ) {
		printf("[SNX-AUDIO] playback file open failed\n");
		return 0;
	}

	ret = pthread_create(&pb->stream_thread, NULL, (void *)snx_audio_pb_flow, pb);
	if(ret != 0) {
		perror_exit(1, "exit thread creation failed");   
	}
	
	pthread_join(pb->stream_thread,NULL);

	fclose(pb->fd);
FREE_PB:
	free(pb);
  
    return 0;  
}
