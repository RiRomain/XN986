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

int pcm_play(char *audio_src);
