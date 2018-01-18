#include "sn986_play.h"
#include "snx_lib.h"
#if 1
#include "data_buf.h"
av_buffer *video_pre_buffer = NULL;
av_buffer *audio_pre_buffer = NULL;
#endif

const char H264marker[] = {0,0,0,1};
unsigned char skip_frame[36] = {
       0x00, 0x00, 0x00, 0x01, 0x01, 0x9A,
        0x00, 0x00, 0xAF, 0x00, 0x00, 0x03,
        0x00, 0x00, 0x03, 0x00, 0x00, 0x03,
        0x00, 0x00, 0x03, 0x00, 0x00, 0x03,
        0x00, 0x00, 0x03, 0x00, 0x00, 0x03,
        0x00, 0x00, 0x03, 0x00, 0x01, 0x03
};

#if VIDEO_PRE_BUFFER

void * snx98_avplay_play(void *arg)
{
	struct snx_avplay_info *avinfo = (struct snx_avplay_info *)arg;
	int ret;
	int keyFrame = 0;
	int frame_count=0;
	int play_interval_time = 0;
	
	unsigned long long pre_time=0, cur_time=0;
	int video_duration = 0;
	int audio_duration = 0;
	int arrival_time = 0;
	int video_fps_interval;
	int audio_interval;
	int video_start_flag = 0;
	int audio_first_flag = 1;


	video_fps_interval = (1000000 / avinfo->fps) ;
	audio_interval = (100000); //use 100ms as default
	arrival_time = video_fps_interval;

	play_interval_time = 3000;

	while(1) {
		gettimeofday(&avinfo->timestamp,NULL);
		printf("[%d.%d] eof: %d,  %d\n",avinfo->timestamp.tv_sec, avinfo->timestamp.tv_usec/1000, avinfo->read_eof, avinfo->actual_frames);
		
		{
			if(pre_time == 0) {
				pre_time = avinfo->timestamp.tv_sec * 1000000 + avinfo->timestamp.tv_usec;
			}
			else { //calculate how much frames we lost during SD encode to reach FPS
				cur_time = avinfo->timestamp.tv_sec * 1000000 + avinfo->timestamp.tv_usec;
				arrival_time = cur_time - pre_time;

				video_duration += arrival_time;
				
				if ((video_duration >= video_fps_interval) && (frame_count < avinfo->actual_frames)) {
					char *tmp_data;
					int tmp_size = 0;
					
					read_buffer_data(video_pre_buffer, &tmp_data,&tmp_size);

					if (tmp_size) {

						avinfo->rp_interval = arrival_time;
						if (video_duration > video_fps_interval)
							video_duration -= video_fps_interval;

						printf("[%d.%d] video dispatching :%d bytes (%d)\n",avinfo->timestamp.tv_sec, avinfo->timestamp.tv_usec/1000, tmp_size, frame_count);

#if 0
						if ( (tmp_size >= sizeof(H264marker)) && (memcmp(tmp_data,H264marker,sizeof(H264marker)) == 0) ){
							if((tmp_data[sizeof(H264marker)]&0x1f) == 7) {
								keyFrame = 1;
							} else {
								keyFrame = 0;
							}
						}
#endif
						if (avinfo->video_cb)
							avinfo->video_cb(&(avinfo->timestamp), tmp_data, tmp_size, keyFrame);

						video_start_flag = 1;
						frame_count++;

					}

					// wait to the last frame done.
					if ((avinfo->read_eof) &&(frame_count >= avinfo->actual_frames)) {

							avinfo->read_eof = 0;
							fprintf(stderr,"Play to the file end (%d /%d)\n",frame_count, avinfo->actual_frames);
							if (avinfo->repeat) {
								//fprintf(stderr,"file finished, back to the head (%d/%d)\n",frame_count, avinfo->frames_num);
								frame_count = 0;
							} else {
								printf("Play End\n");
								snx986_avplay_stop(avinfo);
							}
					}
				}

				audio_duration += arrival_time;
				
				if ((audio_duration >= audio_interval) || (audio_first_flag)) {
					char *tmp_data;
					int tmp_size = 0;
					
					
					if(video_start_flag) { //wait for video start first
						read_buffer_data(audio_pre_buffer, &tmp_data,&tmp_size);
						if (tmp_size) {

							if(audio_duration > audio_interval)
								audio_duration -= audio_interval;

							audio_interval = (((1000000/avinfo->audio_pb->sample_rate) * tmp_size) * 8) / 10; //calculate audio time interval * 0.7 ratio
			
							//printf("[%d.%d] audio dispatching :%d bytes\n",avinfo->timestamp.tv_sec, avinfo->timestamp.tv_usec/1000, tmp_size);
							if(avinfo->audio_cb)
								avinfo->audio_cb(&(avinfo->timestamp), tmp_data, tmp_size, NULL);

							audio_first_flag = 0;
						}
					}
				}

				pre_time = cur_time;
			}
		}
		
		usleep(play_interval_time);

		if(avinfo->started == 0) {
			break;
		}

	} // while(1)

	ret = 0;
finally:
	//destory_playing(avinfo->play_source);
	pthread_exit((void *)ret);
}

#endif

void * snx98_avplay_read(void *arg)
{
	struct snx_avplay_info *avinfo = (struct snx_avplay_info *)arg;
	int ret;
	int keyFrame = 0;
	int main_size = 0;
	int frame_count=0;
	int play_interval_time = 0;
	
	unsigned long long pre_time=0, cur_time=0;
	int video_duration = 0;
	int audio_duration = 0;
	int arrival_time = 0;
	int video_fps_interval;
	int audio_interval;
	int video_first_flag = 1;
	int audio_first_flag = 1;
	void *data = NULL;
	int size = 0;

	// Calculate Video/audio time interval
#if VIDEO_PRE_BUFFER
	avinfo->actual_frames = frame_count;
	video_fps_interval = (1000000 / avinfo->fps) ;
	play_interval_time = 5000;
#endif
	
	audio_interval = (100000); //use 100ms as default


	printf("AVplay thread %dx%d (%d / %d)\n", avinfo->width, avinfo->height, play_interval_time, video_fps_interval);
	
	while(1) {

		gettimeofday(&avinfo->timestamp,NULL);

		if(pre_time == 0) {
			pre_time = avinfo->timestamp.tv_sec * 1000000 + avinfo->timestamp.tv_usec;
			continue;
		}
		else { //calculate how much frames we lost during SD encode to reach FPS
			cur_time = avinfo->timestamp.tv_sec * 1000000 + avinfo->timestamp.tv_usec;
			arrival_time = cur_time - pre_time;

#if VIDEO_PRE_BUFFER
			video_duration += arrival_time;

			if (video_duration >= avinfo->rp_interval) {   


			} else {
				data = NULL;
				size = 0;
				pre_time = cur_time;
				usleep(play_interval_time);
				continue;
			}
#endif
		}

			ret = read_playing_data(avinfo->play_source, &data, &size);
			printf("[%d.%d] type:%d, size: %d\n",avinfo->timestamp.tv_sec, avinfo->timestamp.tv_usec/1000, ret, size);


		if(ret == TYPE_NONE){
			//fprintf(stderr,"ret is %d\n",ret);
			//break;
		}
		else if(ret == TYPE_VIDEO){
#if DEBUG_AV
			fwrite(data,size,1,avinfo->videofile);
#endif


#if VIDEO_PRE_BUFFER

			//printf("Video %d\n", size);
			if ((size >sizeof(skip_frame))&& memcmp(data,skip_frame,sizeof(skip_frame)) != 0) {
				write_buffer_data(video_pre_buffer,(char *)data, size );
				avinfo->actual_frames++;
				if (video_duration > avinfo->rp_interval)
					video_duration -= avinfo->rp_interval;
				
			} else {
				avinfo->skipframe_count++;
				reset_read_playing_data(data);
				pre_time = cur_time;
				continue;
				//printf("skipframe_count : %d\n", avinfo->skipframe_count);
			}
#else
			//haoweilo
			if ((size >sizeof(skip_frame))&& memcmp(data,skip_frame,sizeof(skip_frame)) != 0){

				memcpy(avinfo->ctx->m2m->dec_out_buffers[avinfo->ctx->m2m->dec_out_index].start, data, size);
				if (avinfo->video_cb) {
					avinfo->video_cb(&(avinfo->timestamp), avinfo->ctx->m2m->dec_out_buffers[avinfo->ctx->m2m->dec_out_index].start, size, keyFrame);
				}
			}
			frame_count++;
			//haowei
#endif

		}else if(ret == TYPE_AUDIO){
#if DEBUG_AV
			fwrite(data,size,1,avinfo->audiofile);
#endif

			//printf("Audio %d\n", size);
			audio_interval = (((1000000/avinfo->audio_pb->sample_rate) * size) * 7) / 10; //calculate audio time interval * 0.7 ratio
			write_buffer_data(audio_pre_buffer,(char *)data, size );
#if VIDEO_PRE_BUFFER
			// When getting Audio Data store immediately then back to fetch video right away.
			reset_read_playing_data(data);
			pre_time = cur_time;
			continue;
#endif
		}
		reset_read_playing_data(data);

		{

#if VIDEO_PRE_BUFFER

#else
				audio_duration += arrival_time;
				
				if ((audio_duration >= audio_interval) || (audio_first_flag)) {
					char *tmp_data;
					int tmp_size = 0;
					
					if(audio_duration > audio_interval)
						audio_duration -= audio_interval;
					
					read_buffer_data(audio_pre_buffer, &tmp_data,&tmp_size);

					if (tmp_size) {
						
						//printf("[%d.%d] audio dispatching :%d bytes\n",avinfo->timestamp.tv_sec, avinfo->timestamp.tv_usec/1000, tmp_size);
						if(avinfo->audio_cb)
							avinfo->audio_cb(&(avinfo->timestamp), tmp_data, tmp_size, NULL);

						audio_first_flag = 0;
					}
				}
#endif
				pre_time = cur_time;

		}

#if VIDEO_PRE_BUFFER
		if (ret == TYPE_NONE) {

			avinfo->read_eof = 1;

			if (avinfo->repeat) {
				fprintf(stderr,"file finished, back to the head (%d/%d)\n",avinfo->actual_frames, avinfo->frames_num);
				set_playing_time(avinfo->play_source,0);
			} else {
				printf("File End\n");
				break;
			}
		}
#else
		if ((ret == TYPE_NONE) && (frame_count >= avinfo->frames_num)) {
			if (avinfo->repeat) {
				fprintf(stderr,"file finished, back to the head (%d)\n", avinfo->frames_num);
				set_playing_time(avinfo->play_source,0);
			} else {
				//printf("File End\n");
				snx986_avplay_stop(avinfo);
			}
		}
#endif
		if(avinfo->started == 0) {
			break;
		}



	} // while(1)

	ret = 0;
finally:
	//destory_playing(avinfo->play_source);
#if DEBUG_AV

	fclose(avinfo->videofile);
	fclose(avinfo->audiofile);
#endif
	pthread_exit((void *)ret);
	
}


int snx986_avplay_start(struct snx_avplay_info *avinfo)
{
	int ret =0;
#if VIDEO_PRE_BUFFER
	if (video_pre_buffer)
		read_write_pos_sync(video_pre_buffer);
	else {
		ret = -1;
		printf("video pre buffer write pos sync failed\n");
		goto finally;
	}
#endif

	if (audio_pre_buffer)
		read_write_pos_sync(audio_pre_buffer);
	else {
		ret = -1;
		printf("audio pre buffer write pos sync failed\n");
		goto finally;
	}

	avinfo->started = 1;
	
	ret = pthread_create(&(avinfo->thread_id), NULL, snx98_avplay_read, (void *)avinfo);
	if(ret != 0) {
		printf("Can't create thread: %s\n", strerror(ret));
		goto finally;
	}

#if VIDEO_PRE_BUFFER
	ret = pthread_create(&(avinfo->play_thread_id), NULL, snx98_avplay_play, (void *)avinfo);
	if(ret != 0) {
		printf("Can't create thread: %s\n", strerror(ret));
		goto finally;
	}
#endif

finally:
	return ret;
}

int snx986_avplay_stop(struct snx_avplay_info *avinfo)
{
	int ret=0;
	void *thread_result;


	if (!avinfo) {
		ret = EINVAL;
		printf("null argument");
		goto finally;
	}


	if (!avinfo->started) {
		ret = -1;
		printf("not started");
		goto finally;
	}

	printf("stopping: %s \n", avinfo->filename);
	avinfo->started = 0;

#if VIDEO_PRE_BUFFER
	ret = pthread_join(avinfo->play_thread_id, &thread_result);
#endif
	ret += pthread_join(avinfo->thread_id, &thread_result);
	if(ret != 0)
		printf("thread join failed ret==%d\n", ret);

	ret = (int)thread_result;

	ret = 0;

#if VIDEO_PRE_BUFFER
	if (video_pre_buffer)
		read_write_pos_sync(video_pre_buffer);
	else {
		ret = -1;
		goto finally;
	}
#endif
	if (audio_pre_buffer)
		read_write_pos_sync(audio_pre_buffer);
	else {
		ret = -1;
		goto finally;
	}

finally:
	return ret;

}

int snx986_avplay_open (struct snx_avplay_info *avinfo)
{
	char filename[128];
	int ret;
	if(avinfo == NULL)
		return -1;
	sprintf(filename,"%s",avinfo->filename);

	avinfo->play_source = create_playing(filename);
	if (avinfo->play_source == NULL)
		return -1;

#if DEBUG_AV
	avinfo->videofile = fopen("/tmp/video.h264", "wb");
	avinfo->audiofile = fopen("/tmp/audio.alaw", "wb");
#endif

	avinfo->video_type = read_playing_av_type(avinfo->play_source, TYPE_VIDEO);
	avinfo->audio_type = read_playing_av_type(avinfo->play_source, TYPE_AUDIO);
	
	fprintf(stderr,"video type is %d, audio type is %d\n",avinfo->video_type,avinfo->audio_type);

	ret = read_playing_video_resolution(avinfo->play_source, &avinfo->width, &avinfo->height);
	if(ret < 0){
		fprintf(stderr,"Read video resolution error\n");
	}
	else
		fprintf(stderr,"video res is %dx%d\n",avinfo->width,avinfo->height);

	ret = read_playing_video_frames_num(avinfo->play_source, &avinfo->frames_num);
	if(ret < 0){
		fprintf(stderr,"Read video frames number error\n");
	}
	else
		fprintf(stderr,"video frames is %d\n",avinfo->frames_num);
	
	ret = read_playing_video_fps(avinfo->play_source, &avinfo->fps);
	if(ret < 0){
		fprintf(stderr,"Read video frames fps error\n");
	}
	else
		fprintf(stderr,"video fps is %d\n",avinfo->fps);

#if VIDEO_PRE_BUFFER
	video_pre_buffer = init_av_buffer(VIDEO_PRE_BUFFER_SIZE, USED_VIDEO_PRE_BUF_NUM, MAX_VIDEO_PRE_BUF_NUM);
	if(video_pre_buffer == NULL){
		fprintf(stderr, "Video pre buffer init error\n");
		return NULL;
	}
#endif
	audio_pre_buffer = init_av_buffer(AUDIO_PRE_BUFFER_SIZE, USED_AUDIO_PRE_BUF_NUM, MAX_AUDIO_PRE_BUF_NUM);
	if(audio_pre_buffer == NULL){
		fprintf(stderr," Audio pre buffer init error\n");
		return NULL;
	}

	return 0;
}


static int snx986_avplay_close(struct snx_avplay_info *avinfo)
{
	int ret = 0;

	if (avinfo->started) {
		if ((ret = snx986_avplay_stop(avinfo))) {
			printf("failed to stop '%s': %s", avinfo->filename, strerror(ret));
			goto finally;
		}
	}

	if (avinfo->play_source)
		destory_playing(avinfo->play_source);
	
	//printf("closing");

	if (strlen(avinfo->filename)) {
		strcpy(avinfo->filename, "\0");
	}


	if(audio_pre_buffer)
		deinit_av_buffer(audio_pre_buffer);
#if VIDEO_PRE_BUFFER
	if(video_pre_buffer)
		deinit_av_buffer(video_pre_buffer);
	
#endif
finally:
	return ret;
}

struct snx_avplay_info * snx986_avplay_new(void)
{
	int ret = 0;
	struct snx_avplay_info *avinfo = NULL;

	if (!(avinfo = (struct snx_avplay_info *)calloc(1, sizeof(struct snx_avplay_info)))) {
		ret = errno ? errno : -1;
		printf("calloc: %s", strerror(ret));
		goto finally;
	}

finally:
	
	return avinfo;
}

int snx986_avplay_free(struct snx_avplay_info *avinfo)
{
	int ret=0;


	if(!avinfo) {
		ret = EINVAL;
		goto finally;
	}
	
	ret = snx986_avplay_close(avinfo);
	if(ret != 0) {
		goto finally;
	}


	if(avinfo) {
		free(avinfo);
		avinfo = NULL;
	}
		
	ret = 0;
finally:
	return ret;
}

#if 0
int main(int   argc,   char*   argv[])
{
	FILE *file1 = NULL;
	FILE *file2 = NULL;
	int main_size = 0;
	int ret;
	int i = 0,j = 0,k = 0;
	int width,height;
	int frames_num;
	char filename[128];
	sprintf(filename,"%s",argv[1]);
	file1 = fopen("/mnt/bbb.h264", "wb");
	file2 = fopen("/mnt/ccc.alaw", "wb");
	//p_play_source play = create_playing("/mnt/2015-02-26-15-42-37.avi");

	p_play_source play = create_playing(filename);

	enum AV_CODEC video_type = read_playing_av_type(play, TYPE_VIDEO);
	enum AV_CODEC audio_type = read_playing_av_type(play, TYPE_AUDIO);
	fprintf(stderr,"video type is %d, audio type is %d\n",video_type,audio_type);
	ret = read_playing_video_resolution(play, &width, &height);
	if(ret < 0){
		fprintf(stderr,"Read video resolution error\n");
	}
	else
		fprintf(stderr,"video res is %dx%d\n",width,height);
	ret = read_playing_video_frames_num(play,&frames_num);
	if(ret < 0){
		fprintf(stderr,"Read video frames number error\n");
	}
	else
		fprintf(stderr,"video frames is %d\n",frames_num);
	//set_playing_time(play,5);
	while(1){
		void *data = NULL;
		int size;
		int ret = read_playing_data(play, &data, &size);
		if(ret == TYPE_NONE){
			fprintf(stderr,"ret is %d\n",ret);
			break;
		}
		else if(ret == TYPE_VIDEO){
			fwrite(data,size,1,file1);
			main_size += size;
			i++;
			//fprintf(stderr,"write %d,size %d\n",i,main_size);
			//fprintf(stderr,"write size is %d\n",main_size);
		}else if(ret == TYPE_AUDIO){
			fwrite(data,size,1,file2);
		}
		reset_read_playing_data(data);
		
		if(i >= 45){
			fprintf(stderr,"write over 45\n");
			break;
		}
	}
	set_playing_time(play,10);

	while(1){
		void *data = NULL;
		int size;
		int ret = read_playing_data(play, &data, &size);
		if(ret == TYPE_NONE){
			fprintf(stderr,"ret is %d\n",ret);
			break;
		}
		else if(ret == TYPE_VIDEO){
			fwrite(data,size,1,file1);
			main_size += size;
			j++;
			//fprintf(stderr,"write %d,size %d\n",j,main_size);
			//fprintf(stderr,"write size is %d\n",main_size);
		}else if(ret == TYPE_AUDIO){
			fwrite(data,size,1,file2);
		}
		reset_read_playing_data(data);
		if(j >= 45){
			fprintf(stderr,"write over 45\n");
			break;
		}
	}

	set_playing_time(play,20);

	while(1){
		void *data = NULL;
		int size;
		int ret = read_playing_data(play, &data, &size);
		if(ret == TYPE_NONE){
			fprintf(stderr,"ret is %d\n",ret);
			break;
		}
		else if(ret == TYPE_VIDEO){
			fwrite(data,size,1,file1);
			main_size += size;
			k++;
			//fprintf(stderr,"write %d,size %d\n",j,main_size);
			//fprintf(stderr,"write size is %d\n",main_size);
		}else if(ret == TYPE_AUDIO){
			fwrite(data,size,1,file2);
		}
		reset_read_playing_data(data);
		if(k >= 45){
			fprintf(stderr,"write over 45\n");
			break;
		}
	}

	destory_playing(play);
	fclose(file1);
	fclose(file2);
	return 1;
}

#endif

