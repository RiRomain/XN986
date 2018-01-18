#ifndef _AV_HANDLER_H_
#define _AV_HANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <pthread.h>


typedef struct snx_av_conf_s {

    int m2m_en;
    int bitrate;
    int sd_alarm_record_en;
    int videofps;
    int ispfps;
    int videores;
    int codec_dev;
    int gop;

} snx_av_conf_t;

int open_avhandler(snx_av_conf_t *psnx_av_conf);
int start_avhandler();
int stop_avhandler();
int close_avhandler();
int avhandler_get_codecfps();

#ifdef __cplusplus
}
#endif

#endif
