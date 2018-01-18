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
 */
#include "snx_audio_codec.h"

/*-----------------------------------------------------------------------------
 * Example Code Configuration
 *----------------------------------------------------------------------------*/

#define SNX_AUDIO_CAP_FORMAT		"s16"
									/*  support the following encode format
										SND_PCM_FORMAT_S8, SND_PCM_FORMAT_U8
										SND_PCM_FORMAT_S16, SND_PCM_FORMAT_U18
										SND_PCM_FORMAT_A_LAW, SND_PCM_FORMAT_MU_LAW
										SND_PCM_FORMAT_G726_16, SND_PCM_FORMAT_G726_24
										SND_PCM_FORMAT_G726_32, SND_PCM_FORMAT_G726_40
										SND_PCM_FORMAT_G722
									*/
#define SNX_AUDIO_CAP_FILENAME		"/tmp/audio1"

/*-----------------------------------------------------------------------------
 * Example Code Flow
 *----------------------------------------------------------------------------*/

/* 
  This example reads from the specifiend PCM device 
  and writes to SD card for 10 seconds of data. 
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
        "-s | --samplerate	8 / 16 / 32 / 48 (kHz, 16K/32K/48K are used for PCM. default is 8k.) \n"
        "Ex:   %s -d alaw -p test.raw\n"
		"\n"

		"", argv[0],argv[0]);   
}
  
int main(int argc, void* argv[]) 
{

	int ret = 0;
	int samplerate = 8;
	snx_audio_stream_conf_t *cap = NULL;
	cap = malloc(sizeof(snx_audio_stream_conf_t));

	char device_name[128];
	char audio_src[128];

	memset(device_name, 0x0, 128);
	memset(audio_src, 0x0, 128);

	strcpy(device_name, SNX_AUDIO_CAP_FORMAT);
	strcpy(audio_src, SNX_AUDIO_CAP_FILENAME);

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
		cap->format = SND_PCM_FORMAT_S8;
	else if (!strcmp(device_name, "s16"))
		cap->format = SND_PCM_FORMAT_S16;
	else if (!strcmp(device_name, "u8"))
		cap->format = SND_PCM_FORMAT_U8;
	else if (!strcmp(device_name, "u16"))
		cap->format = SND_PCM_FORMAT_U16;
	else if (!strcmp(device_name, "alaw"))
		cap->format = SND_PCM_FORMAT_A_LAW;
	else if (!strcmp(device_name, "mulaw"))
		cap->format = SND_PCM_FORMAT_MU_LAW;
	else if (!strcmp(device_name, "g726"))
		cap->format = SND_PCM_FORMAT_G726_16;
	else if (!strcmp(device_name, "g722"))
		cap->format = SND_PCM_FORMAT_G722;
	else {

		printf("[SNX-AUDIO] Wrong format\n");
		goto FREE_CAP;
	}

	if ((samplerate != 8) && (samplerate != 16) && (samplerate != 32) && (samplerate != 48)) {
		printf("Wrong samplerate : %d\n", samplerate);
		return 0;
	}

	if ( ((samplerate == 16) || (samplerate == 32) || (samplerate == 48) ) 
		&& (cap->format != SND_PCM_FORMAT_S16) && (cap->format != SND_PCM_FORMAT_U16))
	{
		printf("ONLY PCM S16/U16 support 16Khz / 32Khz / 48Khz samplerate\n");
		return 0;
	}

	samplerate = samplerate * 1000;
		

	printf("[SNX-AUDIO] format: %s, file: %s\n", device_name, audio_src);

	//cap->format = SNX_AUDIO_CAP_FORMAT;
	
	cap->open_mode = SND_PCM_NONBLOCK;
	cap->channel = 1;
	
	cap->sample_rate = samplerate;

	ret = audio_cap_format_check(cap);
	if (ret < 0) {
		printf("[SNX-AUDIO] audio format check error\n");
		goto FREE_CAP;
	}
	
	//cap->fd = fopen(SNX_AUDIO_CAP_FILENAME,"wb");
	cap->fd = fopen(audio_src,"wb");
	if (cap->fd <=0 ) {
		printf("[SNX-AUDIO] capture file open failed\n");
		return 0;
	}

	ret = pthread_create(&cap->stream_thread, NULL, (void *)snx_audio_cap_flow, cap);
	if(ret != 0) {
		perror_exit(1, "exit thread creation failed");   
	}
	
	pthread_join(cap->stream_thread,NULL);

	fclose(cap->fd);
	
FREE_CAP:
	free(cap);
  
    return 0;
}
