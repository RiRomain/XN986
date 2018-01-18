#ifndef _SN98600_CTRL_H_
#define _SN98600_CTRL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>



#define MOTION_THREHOLD		50

#define NONE_POST_JPG		0
#define REGISTER_POST_JPG	(1 << 1)
#define MOTION_POST_JPG		(1 << 2)

#define MODE_VIDEO_HD       0
#define MODE_VIDEO_VGA      1
#define MODE_VIDEO_PHOTO    2


int SetMotionAttr(
		 int index,
		 int enable,
		 int x,
		 int y,
		 int w,
		 int h,
		 int threshold);

int SetAlarm(int onoff);
int SetAlarmRecord(int onoff);

void set_Terminate_sig(void);
int get_Terminate_sig(void);

int init_motion();

int start_motion_detect_loop(void);
void destory_motion_detect_loop(void);
void set_sync_time(void);
int get_sync_time(void);


void set_alarm_trig(int val);
int get_alarm_trig(void);

#ifdef __cplusplus
}
#endif

#endif


