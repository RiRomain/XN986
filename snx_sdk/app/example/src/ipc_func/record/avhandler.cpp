#include "avhandler.h"
#include <sys/time.h>
#include <pthread.h>

#include "sn98600_v4l2.h"
#include "sn98600_ctrl.h"
#include "snx_vc_lib.h"
#include "snx_rc_lib.h"
#include "sn98600_record.h"
#include "save_media.h"
#include "data_buf.h"


extern save_media_source *sd_record;
extern av_buffer *video_pre_buffer;

extern int sd_alarm_record_en;
extern int md_record_status;

#if TIMELAPSE_SUPPORT
extern int timelapse;
#endif

pthread_mutex_t audio_sync_lock;	//haoweilo
pthread_mutex_t md_audio_sync_lock;	//haoweilo

static struct snx_v4l2_video *hd_video = NULL;
int cur_ubAVI_H264_PIC = 0;
int cur_ubAVI_H264_POC = 0;

unsigned long long pre_reach_time =  0;

int fps_time_interval = H264_FPS_TIME_INTERVAL;
int total_accmulation_frame = ACCMULATION_FRAME;
int total_md_accmulation_frame = MD_ACCMULATION_FRAME;

unsigned char skip_frame[36] = {
	0x00, 0x00, 0x00, 0x01, 0x01, 0x9A,
	0x00, 0x00, 0xAF, 0x00, 0x00, 0x03,
	0x00, 0x00, 0x03, 0x00, 0x00, 0x03,
	0x00, 0x00, 0x03, 0x00, 0x00, 0x03,
	0x00, 0x00, 0x03, 0x00, 0x00, 0x03,
	0x00, 0x00, 0x03, 0x00, 0x01, 0x03
};

static int accumlation_duration = 0;
static int local_fp = 0;

static void video_h264hd_cb(struct snx_v4l2_video *video,
			const struct timeval *tv, 
			void *data, 
			size_t len,
			int keyFrame )
{
	int ret = 0;
	unsigned char *Skiptr = NULL;
	unsigned char symbol_pic = 0, symbol_poc = 0;
	static unsigned long serialNum = 0;
	unsigned long long cur_time = 0;
	int process_count = 0;
	int arrival_time = 0;
	int i = 0;
	int record_gop = video->rate_ctl->gop;

	
	if(data == NULL || len == 0) {
		fprintf(stderr, "data is null or len is zero\n");
		return;
	}
	
#if USE_SD_RECORD
	if(sd_record != NULL) {
		if(pre_reach_time == 0) {
			pre_reach_time = tv->tv_sec * 1000000 + tv->tv_usec;
			process_count = 0;
		}
		else { //calculate how much frames we lost during SD encode to reach FPS
			cur_time = tv->tv_sec * 1000000 + tv->tv_usec;
			arrival_time = cur_time - pre_reach_time;
			if(arrival_time > fps_time_interval)
				arrival_time -= fps_time_interval;
			else
				arrival_time = 0;
			
			accumlation_duration += arrival_time;
			if(accumlation_duration > fps_time_interval) {
				process_count = accumlation_duration/fps_time_interval;
				accumlation_duration -= process_count*fps_time_interval;
			}
			else
				process_count = 0;
			
			pre_reach_time = cur_time;
		}

		/* Add skip frame */
		if(process_count > 0) {
			for(i=0 ; i<process_count; i++) {

#if TIMELAPSE_SUPPORT
				if(timelapse)
					local_fp++;
				else {
#endif
					up_record_count();
					local_fp++;
					Skiptr = &skip_frame[0];
					Skiptr += 6;

					symbol_pic = *Skiptr;
					symbol_poc = *(Skiptr+1);
					
					*Skiptr = cur_ubAVI_H264_PIC;
					*(Skiptr+1) = cur_ubAVI_H264_POC;
					
					cur_ubAVI_H264_POC += 2;
					save_data(sd_record, RECORD_CHANNEL_SCHED, (void *)&skip_frame[0], 36, "video");

					if (sd_alarm_record_en) {		//if Motion detection record enable
						write_buffer_data(video_pre_buffer, (char *)&skip_frame[0], 36);
						if(md_record_status == 1){
							char *tmp_data;
							int tmp_size;
							int j;
							for(j = 0; j < 3; j++){
								read_buffer_data(video_pre_buffer, &tmp_data, &tmp_size);
								if(tmp_size > 0)
									save_data(sd_record, RECORD_CHANNEL_MD, (void *)tmp_data, tmp_size, "video");
							}
						}
					}	
					
					if(get_record_count() >= total_accmulation_frame ) {
						write_done_file();
						if(get_sd_available()) {
							set_record_stop(sd_record, RECORD_CHANNEL_SCHED);
							set_close_trig(1);
						}
					}

					if(get_mdrecord_count() >= total_md_accmulation_frame ) {
						write_done_md_file();
						if(get_sd_available()) {
							set_record_stop(sd_record, RECORD_CHANNEL_MD);
							set_md_close_trig(1);
						}
					}
#if TIMELAPSE_SUPPORT
				}
#endif
			} //for loop
		} //if(process_count > 0) {

		if(keyFrame) {
			up_record_count();
			if(local_fp < (record_gop - 1))      //missing some frames in the middle of gop
				fprintf(stderr, "local_fp = %d \n" ,local_fp);

			// reset PIC & POC
			cur_ubAVI_H264_PIC = 2;
			cur_ubAVI_H264_POC = 3;
			//add for AV sync
			if((accumlation_duration)<= (fps_time_interval*2/3))
				accumlation_duration = 0;
			local_fp = 0;
		}
		else {

#if TIMELAPSE_SUPPORT
			if (timelapse)
				local_fp++;
			else {
#endif
				up_record_count();
				local_fp++;
				Skiptr = (unsigned char *)data;
				Skiptr += 6;
				*Skiptr = cur_ubAVI_H264_PIC;
				*(Skiptr+1) = cur_ubAVI_H264_POC;
				
				cur_ubAVI_H264_PIC += 2;
				cur_ubAVI_H264_POC += 2;
#if TIMELAPSE_SUPPORT
			}
#endif

		}

#if TIMELAPSE_SUPPORT
		//TIMELAPSE
		if (timelapse) {
			if (keyFrame)
				save_data(sd_record, RECORD_CHANNEL_SCHED, data, len, "video");
		} else
#endif
			save_data(sd_record, RECORD_CHANNEL_SCHED, data, len, "video");

		if (sd_alarm_record_en) {		//if Motion detection record enable
			write_buffer_data(video_pre_buffer, (char *)data, len);
			if(md_record_status == 1){
				char *tmp_data;
				int tmp_size;
				int i;
				for(i = 0; i < 3; i++){
					read_buffer_data(video_pre_buffer, &tmp_data, &tmp_size);
					if(tmp_size > 0)
						save_data(sd_record, RECORD_CHANNEL_MD, (void *)tmp_data, tmp_size, "video");
				}
			}
		}

		if(get_record_count() >= total_accmulation_frame ) {
			write_done_file();
			if(get_sd_available()) {
				set_record_stop(sd_record, RECORD_CHANNEL_SCHED);
				set_close_trig(1);
			}
		}

		if(get_mdrecord_count() >= total_md_accmulation_frame ) {
			write_done_md_file();
			if(get_sd_available()) {
				set_record_stop(sd_record, RECORD_CHANNEL_MD);
				set_md_close_trig(1);
			}
		}
	}

#endif

	serialNum++;
streaming:

	if(ret!=0){
		fprintf(stderr, "[%d]video_h264hd_cb serN:%lu, len = %d \n", ret, serialNum, len);
	}
	
	return;
}

int avhandler_get_codecfps()
{
	if(!hd_video)
		return 0;
	if(!hd_video->m2m)
		return 0;
	return hd_video->m2m->codec_fps;
}

int open_avhandler(snx_av_conf_t *psnx_av_conf)
{
	int rc = 0, isp0_width = 0;

	//! video input data (HD) callback 

	fprintf(stderr, "########## start inital video ##########\n");
	if (!(hd_video = snx98600_video_new())) {
		rc = errno ? errno : -1;
		fprintf(stderr, "failed to create video instance: %s\n", strerror(rc));
		goto finally;
	}

	pthread_mutex_init(&audio_sync_lock, NULL );		//haoweilo
	pthread_mutex_init(&md_audio_sync_lock, NULL );		//haoweilo
	
	//hd_video->resolution_type = RESOLUTION_HD;
	hd_video->resolution_type = (RESOLUTION_TYPE) psnx_av_conf->videores;
	hd_video->cb = video_h264hd_cb;

	hd_video->m2m->m2m = psnx_av_conf->m2m_en;
	hd_video->m2m->codec_fps = psnx_av_conf->videofps;
	hd_video->m2m->isp_fps = psnx_av_conf->ispfps;
	hd_video->m2m->bit_rate = psnx_av_conf->bitrate << 10; //(Kbps)

	hd_video->rate_ctl->gop = psnx_av_conf->gop;

	// setup codec DEV
	if (psnx_av_conf->codec_dev == 1 )
		strcpy(hd_video->m2m->codec_dev, CAP_DEV_NAME);
	else if (psnx_av_conf->codec_dev == 2 )
		strcpy(hd_video->m2m->codec_dev, CAP1_DEV_NAME);
	else
		strcpy(hd_video->m2m->codec_dev, CAP_DEV_NAME);

#if TIMELAPSE_SUPPORT
	if(timelapse)
		hd_video->rate_ctl->gop = hd_video->rate_ctl->gop * timelapse;
#endif

	//Enable SD alarm record
	sd_alarm_record_en = psnx_av_conf->sd_alarm_record_en;

#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
		if (hd_video->m2m->codec_fmt  == V4L2_PIX_FMT_H264) {

			if ( !strcmp(hd_video->m2m->codec_dev, "/dev/video1") )
				strcpy(hd_video->m2m->ds_dev_name, "1_h");
			else if ( !strcmp(hd_video->m2m->codec_dev, "/dev/video2") )
				strcpy(hd_video->m2m->ds_dev_name, "2_h");
			else {
				printf("[MROI Setting] wrong device name\n");
			}
		}
#endif

	snx98600_video_open(hd_video, NULL );
	snx98600_video_start(hd_video);

	fps_time_interval = (1000000 / hd_video->m2m->codec_fps) + 1;    //calclate the interval time between frames.
	
	/* Total Accmulation Setting */
	total_accmulation_frame = RECORDING_TIME * hd_video->m2m->codec_fps;
	total_md_accmulation_frame  = MOTION_TIME_INTERVAL * hd_video->m2m->codec_fps;
	
	printf("\n\n------- V4L2 Infomation -------- \n");
	printf("m2m_en: %d\n", hd_video->m2m->m2m);
	printf("codec_dev: %s\n", hd_video->m2m->codec_dev);
	printf("codec_fps: %d\n", hd_video->m2m->codec_fps);
	if(hd_video->m2m->m2m)
		printf("isp_fps: %d\n", hd_video->m2m->isp_fps);
	printf("width: %d\n", hd_video->m2m->width);
	printf("height: %d\n", hd_video->m2m->height);
	printf("scale: %d\n", hd_video->m2m->scale);
	printf("bit_rate: %d\n", hd_video->m2m->bit_rate);
	printf("dyn_fps_en: %d\n", hd_video->m2m->dyn_fps_en);
	if(hd_video->m2m->dyn_fps_en) {
		printf("framerate: %d\n", hd_video->rate_ctl->framerate);
	}
	printf("GOP: %d\n", hd_video->rate_ctl->gop);
	printf("ds_font_num: %d\n", hd_video->m2m->ds_font_num);
	printf("fps_time_interval: %d\n", fps_time_interval);
	printf("total_accmulation_frame: %d\n", total_accmulation_frame);
	printf("md_accmulation_frame after event: %d\n", total_md_accmulation_frame);
	printf("\n----------------------------- \n\n");

#if USE_SD_RECORD
	start_record_loop();
#endif

//	snx_get_file_value("/proc/isp/osd/0/width", &isp0_width, 16);

	start_motion_detect_loop();
finally:
	return rc;
}

extern struct ConfigParse camera_config;
int start_avhandler()
{
	int rc = 0;
	fprintf(stderr, "%s \n", __FUNCTION__);
	fprintf(stderr, "%s end\n", __FUNCTION__);
	return rc;
}

int stop_avhandler()
{
	int rc = 0;
	fprintf(stderr, "%s \n", __FUNCTION__);
	fprintf(stderr, "%s end\n", __FUNCTION__);
	return rc;
}


int close_avhandler()
{
	int rc = 0;
	
#if USE_SD_RECORD
	destory_record_loop();
#endif
	
//	destory_motion_detect_loop();

	if(!hd_video) {
		fprintf(stderr, "[%s]Invaild pointer video source \n", __FUNCTION__);
		rc = -1;
		goto finally;
	}

	if ((rc = snx98600_video_free(hd_video))) {
		fprintf(stderr, "failed to close video source: %s\n", strerror(rc));
		goto finally;
	}

finally:
	pthread_mutex_destroy(&audio_sync_lock);	//haoweilo
	pthread_mutex_destroy(&md_audio_sync_lock);	//haoweilo
	return rc;
}


