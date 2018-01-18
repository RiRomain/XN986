#ifndef  __SAVE_MEDIA_H__
#define __SAVE_MEDIA_H__

#ifdef __cplusplus
extern "C" {
#endif


#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
//#include <libavdevice/avdevice.h>
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
#include "xmllib.h"
#include "parse_config.h"
#include "linklist.h"

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#define OPEN_ERROR_FILE "/tmp/open_file_error"

enum AV_FORMAT {
	NONE_FORMAT,
	ALAW_FORMAT,
	MULAW_FORMAT,
	G726_FORMAT,
	MJPEG_FORMAT,
	H264_FORMAT,
};



typedef struct save_video_src
{
	 int codec;
	 char size[32];
	 int fps;
}save_video_src;

typedef struct save_audio_src
{
	 int codec;
	 int samplerate;
	 int bitrate;
	 int frame_size;
}save_audio_src;

typedef struct save_param
{
	 save_audio_src audio_src;
	 save_video_src video_src;
	 int prerecord ;
	 int enable_audio;
	 int segment_time;
	 int ddr_buf_size;
	 int slice_size;
	 char file_path[128];
	 char file_header[32];
	 char header_fmt[32];
	 char thread_name[32];
	 char file_type[32];
}save_param;




typedef struct save_media_source{
	pthread_t _t;
	volatile int _thread_exit;
	volatile int _running;
	volatile int _recording_state;
	char _source_name[32];
	char _video_device[32];
	char _audio_device[32];
	char _size[32];
	int _video_format;
	int _audio_format;
	int _fps;
	int _samplerate;
	int _audio_bitrate;
	int _audio_format_bits;
	int _audio_frame_size;
	int _copy_extradata;
	int64_t _video_dts_record;
	int64_t _audio_dts_record;
	LNode save_video_pkt_list;
	LNode save_audio_pkt_list;
	pthread_mutex_t save_video_list_lock;
	pthread_mutex_t save_audio_list_lock;
	char _save_filename[128] ;
	char _save_path[128] ;
	char _file_type[32];
	//pthread_mutex _pic_state_lock;

	char record_file_name[50][128];
	//static int record_file_num;
	char _pic_header[32];
	char _file_header[32];
	char _header_fmt[32];
	int seg_time;
	int ddr_buf_size;
	int slice_size;
	struct timeval tv_start;
	struct timeval tv_end;
	AVFormatContext *_cap_ctx;
	AVFormatContext *_fmt_save;

	AVCodecContext _dummy_codec;
	AVRational _format_time_base;
	int _no_audio;
	volatile int _prerecord;
	//volatile int _tmp_prerecord;
	int recording_id;
	xml_doc record_config;
	struct save_media_source *next;
}save_media_source, *p_save_source;


/*
int get_video_format()
{
	return _video_format;
}
const char *name(){
	return _source_name.c_str();
}*/
/******recording state 1:start 0:stop********/
void set_record_start(p_save_source save,int index);
void set_record_stop(p_save_source save,int index);
int get_record_state(p_save_source save,int index);
int get_prerecord_time(p_save_source save,int index);
void set_prerecord_time(p_save_source save,int index,int time);
const char *get_record_file_name(p_save_source save,int index);
void set_record_file_header(p_save_source save,int index,char *name);
void set_record_path(p_save_source save,int index,char *name);

/*
void set_file_name_header(char *file_head){
	_file_header = file_head;
}
void set_snapshoot_state(bool state){
	lock_guard<pthread_mutex> guard(_pic_state_lock);
	_is_snapshot = state;
}
bool get_snapshot_state(){
	lock_guard<pthread_mutex> guard(_pic_state_lock);
	return _is_snapshot;
}
*/
p_save_source create_recording(const char *filename);
void destory_recording(p_save_source save);

void save_data(p_save_source save,int index,void *data,int size,char *type);

#ifdef __cplusplus
}
#endif

#endif

