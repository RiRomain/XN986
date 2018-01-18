#ifndef PLAY_H_
#define PLAY_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <stdlib.h>
#include "snx_vc_lib.h"
#include "play_media.h"
#include "snx_audio_codec.h"
	
enum {
	MODE_PRINT_SCREEN,
	MODE_DUMP_FILE,
	MODE_LCD,
};

typedef struct snx_context {
	int abort;
	int step;
	int output_mode;
	FILE *dst_fp;
	struct snx_m2m *m2m;
	pthread_t process_tid;
} snx_context;


typedef void (*videocallback)(
         const struct timeval *tstamp, void *data, size_t len,
         int keyframe);

typedef void (*audiocallback)(
         const struct timeval *tstamp, void *data, size_t len,
         void *cbarg);

struct snx_avplay_info { 
	char filename[128];
#if DEBUG
	FILE *audiofile = NULL;
	FILE *videofile = NULL;
#endif
	p_play_source play_source;

	struct timeval timestamp;
	int width;
	int height;
	int frames_num;
#if VIDEO_PRE_BUFFER
	int skipframe_count;
	int actual_frames;
	int read_eof;
	int rp_interval;
#endif
	int fps;
	int started;
	int repeat;
	enum AV_CODEC video_type;
	enum AV_CODEC audio_type;
	pthread_t thread_id;
#if VIDEO_PRE_BUFFER
	pthread_t play_thread_id;
#endif

	void *cbarg;

	snx_context *ctx;
	snx_audio_stream_conf_t *audio_pb;

	videocallback video_cb;
	audiocallback audio_cb;
};

void * snx98_avplay_read(void *arg);
int snx986_avplay_start(struct snx_avplay_info *avinfo);
int snx986_avplay_stop(struct snx_avplay_info *avinfo);
int snx986_avplay_open(struct snx_avplay_info *avinfo);
struct snx_avplay_info * snx986_avplay_new(void);
int snx986_avplay_free(struct snx_avplay_info *avinfo);



#ifdef __cplusplus
 }

#endif

#endif /* PLAY_H_ */
