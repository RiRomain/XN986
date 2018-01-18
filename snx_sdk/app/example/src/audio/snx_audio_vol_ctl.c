/**
 *
 * SONiX SDK Example Code
 * Category: Audio Codec
 * File: snx_audio_record.c
 * Usage: 
 *		 1. Make sure a SD card has been inserted and mounted (/media/sda)
 *		 2. Execute snx_audio_record (/usr/bin/example/snx_audio_record)
 *       3. The encoding would start and write to the sd card.
 *       4. It would stop after 10 seconds encoding.
 * NOTE:
 */

#include "snx_audio_codec.h"
#include <snx_audio/snx_audio_volume.h>

/*-----------------------------------------------------------------------------
 * Example Code Configuration
 *----------------------------------------------------------------------------*/

#define CARDNUM	0
#define PERCENTAGE 100

/*-----------------------------------------------------------------------------
 * Example Code Flow
 *----------------------------------------------------------------------------*/

/* 
  This example reads from the specifiend PCM device 
  and writes to SD card for 10 seconds of data. 
 */  

static const char short_options[] = "hd:v:mi";

static const struct option long_options[] = {
    {"help", no_argument, NULL, 'h'},
    {"device", required_argument, NULL, 'd'},
    {"vol", required_argument, NULL, 'v'},
    {"mute", no_argument, NULL, 'm'},
	{"info", no_argument, NULL, 'i'},
	
    {0, 0, 0, 0}
};

static void usage(FILE * fp, int argc, char ** argv) {   
    fprintf(fp, "Usage: %s [options]/n\n"   
		"Options:\n"
		"-h Print this message\n"
        "-d | --device    mic or spk \n"
		"-m | --mute      mute this device\n"
		"-v | --vol       Set vol value (0 ~ 19)\n"
		"-i | --info      get the vol info\n"

		"", argv[0]);   
}

int main(int argc, char *argv[])   
{

	char *dev;
	int mute_flag = 0;
	int vol_flag = 0;
	int info_flag = 0;
	int vol_tune;
	int cur_vol;
	int info_mute;
	int info_vol_cur;
	int info_vol_range;
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
				if (strlen(optarg) > 0) {
					dev = malloc(strlen(optarg));
					if((!strcasecmp(optarg, "mic")) || (!strcasecmp(optarg, "spk"))) {
						strcpy(dev, optarg);
						//printf("[SNX-AUDIO] Device name: %s\n", dev);
					} else {
						printf("[SNX-AUDIO] WORONG device name\n");
						exit(EXIT_FAILURE);
					}
				}
				break;
			case 'v':
				vol_tune = atoi(optarg);
				if (vol_tune <= 0)
					vol_tune = 0;
				vol_flag = 1;
				break;
			case 'm':
				mute_flag = 1;
				break;
			case 'i':
				info_flag = 1;
				break;
			default:
				usage(stderr, argc, argv);
				if(dev) {
					free(dev);
				}
				exit(EXIT_FAILURE);
		}
	}

	if (dev) {
	
		if(!strcasecmp(dev, "mic"))
		{	
			snx_audio_mic_vol_get_mute(CARDNUM, &info_mute);
			snx_audio_mic_vol_get_items(CARDNUM, &info_vol_range);
			snx_audio_mic_vol_get(CARDNUM, &info_vol_cur);
			
			if (info_flag == 1) {
				if (info_mute) {
					printf("[SNX-AUDIO] MIC is on mute state\n");
				}
				printf("[SNX-AUDIO] MIC Vol range: 0 ~ %d; Current Vol : %d\n", (info_vol_range-1), info_vol_cur);
			}
				
			if(mute_flag == 1) {
				snx_audio_mic_vol_set_mute(CARDNUM, SNX_AUDIO_MUTE);
				printf("[SNX-AUDIO] Mute MIC\n");
			}
			
			if ( vol_flag == 1) {
				snx_audio_mic_vol_set_mute(CARDNUM, SNX_AUDIO_UNMUTE);
				printf("[SNX-AUDIO] Un-mute MIC\n");
				
				if ( vol_tune > (info_vol_range - 1))
					vol_tune = (info_vol_range - 1);
				snx_audio_mic_vol_set(CARDNUM,vol_tune);
				
				snx_audio_mic_vol_get(CARDNUM, &cur_vol);
				printf("[SNX-AUDIO] New MIC Vol : %d\n", cur_vol);
			}
			
			
		}
	
		if(!strcasecmp(dev, "spk"))
		{
			snx_audio_spk_vol_get_mute(CARDNUM, &info_mute);
			snx_audio_spk_vol_get_items(CARDNUM, &info_vol_range);
			snx_audio_spk_vol_get(CARDNUM, &info_vol_cur);
			
			if (info_flag == 1) {
				if (info_mute) {
					printf("[SNX-AUDIO] SPK is on mute state\n");
				}
				printf("[SNX-AUDIO] SPK Vol range: 0 ~ %d; Current Vol : %d\n", (info_vol_range - 1), info_vol_cur);
			}
				
			if(mute_flag == 1) {
				snx_audio_spk_vol_set_mute(CARDNUM, SNX_AUDIO_MUTE);
				printf("[SNX-AUDIO] Mute SPK\n");
			}
			
			if ( vol_flag == 1) {
				snx_audio_spk_vol_set_mute(CARDNUM, SNX_AUDIO_UNMUTE);
				printf("[SNX-AUDIO] Un-mute SPK\n");
				
				if ( vol_tune > (info_vol_range - 1))
					vol_tune = (info_vol_range - 1);
				snx_audio_spk_vol_set(CARDNUM,vol_tune);
				
				snx_audio_spk_vol_get(CARDNUM, &cur_vol);
				printf("[SNX-AUDIO] New SPK Vol : %d\n", cur_vol);
			}
		}
		
		free(dev);
	}

    return 0;
}