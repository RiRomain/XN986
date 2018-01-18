#ifndef __SNX_LIB_H__
#define __SNX_LIB_H__

#ifdef __cplusplus
extern "C" {
#endif


#define    IMG_DEV_NAME    "/dev/video0"
#define    CAP_DEV_NAME    "/dev/video1"
#define    CAP1_DEV_NAME    "/dev/video2"
    
/* config files setting */

#define CONF_FOLDER				"/etc/EXAMPLE/"

#define MD_ALARM_STATUS          "/tmp/md_status"

#define MD_THRESHOLD            "/etc/EXAMPLE/md_threshold"
#define MD_ALARM_ONOFF          "/etc/EXAMPLE/md_alarm_onoff"
#define MD_ALARM_RECORD_ONOFF	"/etc/EXAMPLE/md_alarm_record_onoff"
#define SNAPSHOT_QP             "/etc/EXAMPLE/snapshot_qp"
#define SD_REMOVAL_INFO			"/proc/mmc/removal"

#define CMD_COPY            	"cp -f"


/* recording setting */
#define AUDIO_RECORD_DEV    	"snx_audio_alaw"
#define SD_EXAMPLE          	"SONIX"
#define SD_ALARM_PATH       	"md"
#define SD_RECORD_PATH      	"record"
#define RECORD_CONFIG       	"/etc/EXAMPLE/records.xml"
#if TIMELAPSE_SUPPORT
#define RECORDING_TIME           300 //default 300 seconds for timelapse
#else
#define RECORDING_TIME           600 //default 600 seconds
#endif

/* "record id" defined in records.xml */
#define RECORD_CHANNEL_SCHED     1
#define RECORD_CHANNEL_MD        2

#define SD_ALARM_UPBD            10
#define SD_RECORD_UPBD           90

/* audio recording setting */
#define    SAMPLE_RATE               8000        // 8K
#define    FORMAT_BIT                8
#define     READ_BYTE                800
#define MAX_FRAME_NUM                8192
#define AUDIO_COUNT_TIME_INTERVAL       ((1000 * READ_BYTE) / SAMPLE_RATE )      //unit: ms
#define AUDIO_ACCMULATION_COUNT         ((RECORDING_TIME * SAMPLE_RATE) / READ_BYTE)
#define AUDIO_SKIP_THRESHOLD         1 * (AUDIO_COUNT_TIME_INTERVAL)
#define AUDIO_INSERT_THRESHOLD       2 * (AUDIO_COUNT_TIME_INTERVAL)

/*  video recording setting */
#define H264_FPS_TIME_INTERVAL    66667               //default 15fps interval
#define ACCMULATION_FRAME        RECORDING_TIME * 15  //default 9000 frames for 15fps

/* MD recording setting */
#define MOTION_TIME_INTERVAL     30
#define MD_ACCMULATION_FRAME     MOTION_TIME_INTERVAL * 15  //default 30 seconds for 15fps after event

/* MD pre-record setting */
#define VIDEO_PRE_BUFFER_SIZE        4500000            //for 1Mbps bitrate 30 second use
#define AUDIO_PRE_BUFFER_SIZE         262080
#define MAX_VIDEO_PRE_BUF_NUM    (15 * MOTION_TIME_INTERVAL)    //15fps x pre-record seconds
#define MAX_AUDIO_PRE_BUF_NUM    ((MOTION_TIME_INTERVAL * SAMPLE_RATE) / READ_BYTE)   
#define USED_VIDEO_PRE_BUF_NUM   (15 * MOTION_TIME_INTERVAL)    //15fps x pre-record seconds
#define USED_AUDIO_PRE_BUF_NUM   ((MOTION_TIME_INTERVAL * SAMPLE_RATE) / READ_BYTE)

#ifdef __cplusplus
}

#endif

#endif //__SNX_LIB_H__
