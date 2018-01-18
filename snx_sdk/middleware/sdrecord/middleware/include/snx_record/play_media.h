#ifndef  __PLAY_MEDIA_H__
#define __PLAY_MEDIA_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libavutil/parseutils.h>
#include <libavutil/opt.h>

#undef  malloc
#undef  free
#undef  realloc
#undef  time
#undef  sprintf
#undef  strcat
#undef  strncpy
#undef  exit
#undef  printf
#undef  fprintf


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/vfs.h>


#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif



enum AV_CODEC {
	NONE_FORMAT,
	ALAW_FORMAT,
	MULAW_FORMAT,
	G726_FORMAT,
	MJPEG_FORMAT,
	H264_FORMAT,
};



enum AV_TYPE{
    TYPE_NONE,
    TYPE_VIDEO,
    TYPE_AUDIO,
};

typedef struct play_media_source{
	AVInputFormat *file_fmt;
	AVFormatContext *_fmt_play;
	int audio_index;
	int video_index;
	char filename[128];
	int set_time_flag;
}play_media_source, *p_play_source;



p_play_source create_playing(const char *filename);
void destory_playing(p_play_source play);
int read_playing_video_resolution(p_play_source play,int *width, int *height);
int read_playing_video_fps(p_play_source play,int *fps);
int read_playing_video_frames_num(p_play_source play,int *frames_num);
enum AV_CODEC read_playing_av_type(p_play_source play, enum AV_TYPE type);
enum AV_TYPE read_playing_data(p_play_source play,void **data,int *size);
int reset_read_playing_data(void *data);
void set_playing_time(p_play_source play,int64_t timestamp);

#ifdef __cplusplus
}
#endif

#endif



