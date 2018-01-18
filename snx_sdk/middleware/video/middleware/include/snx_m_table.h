#ifndef __SNX_M_TABLE_H__
#define	__SNX_M_TABLE_H__

#ifdef __cplusplus
extern "C"{
#endif

#define LONG_TXT_LENGTH		32
#define SHORT_TXT_LENGTH		16

typedef struct 
{
	char 	maxResolution[LONG_TXT_LENGTH];
	char		pathNumber[SHORT_TXT_LENGTH];
	char		owner[SHORT_TXT_LENGTH];
	char		ISPChannel[SHORT_TXT_LENGTH];
	char		ISPResolution[LONG_TXT_LENGTH];
	char		captureResolution[LONG_TXT_LENGTH];
	char		captureFormat[LONG_TXT_LENGTH];
	char		exclusive[SHORT_TXT_LENGTH];
} videoformat_t;

#define	FMT_RAW			"V4L2_PIX_FMT_SNX420"
#define	FMT_H264			"V4L2_PIX_FMT_H264"
#define	FMT_MJPEG			"V4L2_PIX_FMT_MJPEG"

int use_video_by_3_paras(char *ispRes, char *cptRes, char *cptfmt);
int use_video_by_2_paras(char *cptRes, char *cptfmt);
int unuse_video_by_3_paras(char *ispRes, char *cptRes, char *cptfmt);
int unuse_video_by_2_paras(char *cptRes, char *cptfmt);
int try_open_maxRes_video_status(const char *type, char * maxRes);
int open_max_res_video(char *type, videoformat_t * entry);
int close_max_res_video(char *type, videoformat_t * entry);
int reset_table();

#ifdef __cplusplus
}
#endif

#endif
