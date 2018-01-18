#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include "snx_rc_lib.h"

#include "isp_lib_api.h"
#include "sn98600_ctrl.h"
#include "avhandler.h"
#include "util.h"


static pthread_t monitor_id;

static int flag_sync_net_time 	= 0;
static int kill_sig = 0;
static int alarm_trig = 0;


void set_alarm_trig(int val)
{
	alarm_trig = val;
}

int get_alarm_trig(void)
{
	return alarm_trig;
}

void set_Terminate_sig(void)
{
	kill_sig = 1;
}

int get_Terminate_sig(void)
{
	return kill_sig;
}


void set_sync_time(void)
{
	flag_sync_net_time = 1;
}

int get_sync_time(void)
{
	return flag_sync_net_time;
}

int SetMotionAttr(
		 int index,
		 int enable,
		 int x,
		 int y,
		 int w,
		 int h,
		 int threshold
		)
{
	unsigned int mask[6] = {0};
	int md_threshold = 1000;

	snx_get_file_value(MD_THRESHOLD, &md_threshold, 10);
	printf("set md threshold %d \n",md_threshold);
	snx_isp_md_threshold_set(md_threshold);//0x0~0xffff
	snx_isp_md_int_threshold_set(1);
#if 0	
	printf("mask(%d): 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", \
		     enable, mask[0], mask[1], mask[2], mask[3], mask[4], mask[5] );
#endif	
	snx_isp_md_block_mask_set(mask);
	
	snx_isp_md_int_timeout_set(1000);
	snx_isp_md_enable_set(enable);

	return 0;
}



int init_motion()
{
	
	return 0;
}


static int monitor_flag = 0;

void *monitor_loop(void *param)
{
	static time_t last_motion_time = 0;
//	static time_t last_audio_time = 0;
	int status=0;
	int md_alarm_onoff = 0;
	int md_alarm_record_onoff = 0;

#if EXTERN_MD_FLAG
	char syscmd[128];

	memset(syscmd, 0x00, sizeof(syscmd));
	sprintf(syscmd, "echo 0 > %s", MD_ALARM_STATUS);
#endif

	fprintf(stderr, "[monitor_loop] wait sync time\n");
#if 0	
	while(flag_sync_net_time==0) {
		// waitting for time sync ready
		sleep(1);
		 if (!monitor_flag)
			return NULL;
	}
#endif
	last_motion_time = time(NULL);

	fprintf(stderr, "[monitor_loop] start\n");
	while(monitor_flag)
	{

		snx_get_file_value(MD_ALARM_ONOFF, &md_alarm_onoff, 10);
		snx_get_file_value(MD_ALARM_RECORD_ONOFF, &md_alarm_record_onoff, 10);
#if EXTERN_MD_FLAG
		snx_get_file_value(MD_ALARM_STATUS, &status, 10);
#else
		snx_isp_md_int_get(&status); /* interrupt. if 0 timeout, else have motion */
#endif
		//fprintf(stderr, "md status = %d, alarm_onoff = %d, record_onoff = %d \n", status, md_alarm_onoff,  md_alarm_record_onoff);
		if(0 == status)
		{
			// no motion report
		}
#if EXTERN_MD_FLAG
		else if(2 == status)
		{
			//re-sync time when tstreamer starts to run
			fprintf(stderr, "[monitor_loop] re-sync time %d\n", status);
			last_motion_time = time(NULL);
			system(syscmd);
		}
#endif
		else
		{
			if(md_alarm_onoff)
			{
					time_t cur_time = time(NULL);
					if((cur_time - last_motion_time) >= MOTION_TIME_INTERVAL)
					{
						/* alarm record */
						if(md_alarm_record_onoff)
							set_alarm_trig(1);

						last_motion_time = cur_time;
					}
			}
#if EXTERN_MD_FLAG
			system(syscmd);
#endif
		}
		sleep(1);
	}
	return NULL;

}

int start_motion_detect_loop(void)
{
#if EXTERN_MD_FLAG
	char syscmd[128];
	memset(syscmd, 0x00, sizeof(syscmd));
	sprintf(syscmd, "echo 0 > %s", MD_ALARM_STATUS);
	system(syscmd);
#else
	// set default middleware motion setting
	SetMotionAttr(1,1,0,0,12,8,20);
#endif
	monitor_flag = 1;
	if(pthread_create(&monitor_id, NULL, monitor_loop, NULL) != 0) {
		fprintf(stderr,"[monitor]pthread_create failed\n");
		return -1;
	}
	
	fprintf(stderr, "start_motion_detect_loop \n");
	return 0;
}

void destory_motion_detect_loop(void)
{
	fprintf(stderr, "destory_motion_detect_loop start \n");
	monitor_flag = 0;
	pthread_join(monitor_id, NULL);
	fprintf(stderr, "destory_motion_detect_loop done \n");
}

