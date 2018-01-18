#ifndef __SNX_LIB_H__
#define __SNX_LIB_H__


#ifdef __cplusplus
extern "C" {
#endif


enum RESOLUTION_TYPE
{
	//RESOLUTION_VGA_MJ
	RESOLUTION_HD_MJ,
	RESOLUTION_VGA_264,
	RESOLUTION_HD,
};

#define    IMG_DEV_NAME    "/dev/video0"
#define    CAP_DEV_NAME    "/dev/video1"
#define    CAP1_DEV_NAME    "/dev/video2"


/* recording setting */
#define AUDIO_RECORD_DEV    	"snx_audio_alaw"


/* audio recording setting */
#define    SAMPLE_RATE               8000        // 8K
#define    FORMAT_BIT                8
#define     READ_BYTE                800
#define MAX_FRAME_NUM                8192
#define AUDIO_COUNT_TIME_INTERVAL       ((1000 * READ_BYTE) / SAMPLE_RATE )      //unit: ms
#define AUDIO_ACCMULATION_COUNT         ((RECORDING_TIME * SAMPLE_RATE) / READ_BYTE)
#define AUDIO_SKIP_THRESHOLD         1 * (AUDIO_COUNT_TIME_INTERVAL)
#define AUDIO_INSERT_THRESHOLD       2 * (AUDIO_COUNT_TIME_INTERVAL)

/*  playback setting */
#define MOTION_TIME_INTERVAL     5

#define VIDEO_PRE_BUFFER_SIZE        1500000            //for 1Mbps bitrate 5 second use
#define AUDIO_PRE_BUFFER_SIZE        131040
#define MAX_VIDEO_PRE_BUF_NUM    (30 * MOTION_TIME_INTERVAL)    //30fps x pre-record seconds
#define MAX_AUDIO_PRE_BUF_NUM    ((MOTION_TIME_INTERVAL * SAMPLE_RATE) / READ_BYTE)
#define USED_VIDEO_PRE_BUF_NUM   (30 * MOTION_TIME_INTERVAL)    //30fps x pre-record seconds
#define USED_AUDIO_PRE_BUF_NUM   ((MOTION_TIME_INTERVAL * SAMPLE_RATE) / READ_BYTE)

#ifdef __cplusplus
}



#endif

#endif //__SNX_LIB_H__
