/**
 *
 * SONiX SDK Example Code
 * Category: Video Encode
 * File: snx_m2m_one_stream.c
 * Usage: 
 *		 1. Make sure a SD card has been inserted and mounted (/media/sda)
 *		 2. Execute snx_m2m_one_stream (/usr/bin/snx_m2m_one_stream)
 *       3. The video encoding would start
 *       4. After the frame_num is reached, the encoding would stop
 *       5. Check the file recorded in the SD card by using VLC
 * NOTE:
 *       Recording all streams to SD card would cause the target framerate can
 *       not be reached because of the bandwidth leakage of SD card.
 */
#include "snx_audio_codec.h"

/*-----------------------------------------------------------------------------
 * Example Code Configuration
 *----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
 * GLOBAL Variables
 *----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
 * Example Code Flow
 *----------------------------------------------------------------------------*/

int main() {  
    int val;  
  
    printf("ALSA library version: %s\n", SND_LIB_VERSION_STR);  
  
    printf("PCM stream types:\n");  
    for (val = 0; val <= SND_PCM_STREAM_LAST; val++) {  
        printf(" %s\n", snd_pcm_stream_name((snd_pcm_stream_t)val));  
    }  
  
    printf("PCM access types:\n");  
    for (val = 0; val <= SND_PCM_ACCESS_LAST; val++) {  
        printf(" %s\n", snd_pcm_access_name((snd_pcm_access_t)val));  
    }  
  
    printf("PCM formats:\n");  
    for (val = 0; val <= SND_PCM_STREAM_LAST; val++) {  
        if (snd_pcm_format_name((snd_pcm_format_t)val) != NULL) {  
            printf(" %s (%s)\n",   
                  snd_pcm_format_name((snd_pcm_format_t)val),  
                  snd_pcm_format_description((snd_pcm_format_t)val));  
        }  
   }  
   
   printf("\nPCM subformats:\n");  
   for (val = 0; val <= SND_PCM_SUBFORMAT_LAST; val++) {  
       printf(" %s (%s)\n", snd_pcm_subformat_name((snd_pcm_subformat_t)val),  
           snd_pcm_subformat_description((snd_pcm_subformat_t)val));  
   }  
   return 0;  
}