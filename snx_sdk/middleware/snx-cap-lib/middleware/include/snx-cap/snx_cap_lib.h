#ifndef __SNX_CAP_LIB_H__
#define __SNX_CAP_LIB_H__

#ifdef __cplusplus
extern "C"{
#endif




#define	SNX_ERR_NMEM		0xa0000001
#define	SNX_ERR_MFUC		0xa0000002
#define	SNX_ERR_NEXIST		0xa0000003
#define	SNX_ERR_NRES		0xa0000004
#define	SNX_ERR_NFPS		0xa0000005
#define	SNX_ERR_NGOP		0xa0000006
#define	SNX_ERR_NCODEC	0xa0000007
#define	SNX_ERR_NBIT		0xa0000008
#define	SNX_ERR_NQP		0xa0000009
#define	SNX_ERR_NPATH		0xa000000a
#define	SNX_ERR_TLONG		0xa000000b
#define	SNX_ERR_NDISK		0xa000000c
#define	SNX_ERR_NPERM		0xa000000d
#define	SNX_ERR_MTIME		0xa000000e
#define	SNX_ERR_NBUSY		0xa000000f
#define	SNX_ERR_NATTR		0xa0000010
#define	SNX_ERR_NOWNER	0xa0000011
#define	SNX_ERR_NOPEN		0xa0000012


#define	SNX_STREAM_ATTR_VIDEO_STATUS	0x00000001
#define	SNX_STREAM_ATTR_VIDEO_CODEC		0x00000002
#define	SNX_STREAM_ATTR_VIDEO_RES		0x00000004
#define	SNX_STREAM_ATTR_VIDEO_FPS		0x00000008
#define	SNX_STREAM_ATTR_VIDEO_BIT_MODE	0x00000010
#define	SNX_STREAM_ATTR_VIDEO_GOP		0x00000020
#define	SNX_STREAM_ATTR_VIDEO_BIT		0x00000040
#define	SNX_STREAM_ATTR_VIDEO_QP			0x00000080
#define	SNX_STREAM_ATTR_VIDEO			0x000000ff
#define	SNX_STREAM_ATTR_AUDIO_STATUS	0x00000100
#define	SNX_STREAM_ATTR_AUDIO_CODEC		0x00000200
#define	SNX_STREAM_ATTR_AUDIO_BIT		0x00000400
#define	SNX_STREAM_ATTR_AUDIO_SPR		0x00000800
#define	SNX_STREAM_ATTR_AUDIO			0x00000f00

#define SNX_VIDEO_STREAM_NUM			3
#define SNX_AUDIO_STREAM_NUM			3
#define SNX_STREAM_NUM				(SNX_VIDEO_STREAM_NUM+SNX_AUDIO_STREAM_NUM)
#define SNX_STREAM_ID_LEN				64


typedef enum{
	SNX_STATUS_OFF,
	SNX_STATUS_ON
}snx_status_e;


typedef enum{
	SNX_MEDIA_AUDIO,
	SNX_MEDIA_VIDEO	
}snx_media_type_e;

typedef enum{
	SNX_CODEC_MJPEG,
	SNX_CODEC_H264,
	SNX_CODEC_ALAW,
	SNX_CODEC_MULAW,
	SNX_CODEC_G726,
	SNX_CODEC_AUD32
}snx_codec_e;


typedef enum{
	SNX_CBR,
	SNX_VBR
}snx_bit_mode_e;


typedef struct snx_audio
{
	snx_status_e status;
	snx_codec_e codec;		
	int bitrate;
	int samplerate;		
} snx_audio_t;

typedef int ( * snx_attr_callback_t ) (char *stream_id, int attr,      						/*stream id, attrbute */
						char *val);                			/* value */




typedef struct snx_caplib_data
{
	char *data;
	size_t size;
}snx_caplib_data_t;


typedef struct snx_res
{	
	unsigned int width;
	unsigned int height;
}snx_res_t;
/*
typedef struct snx_isp_attr
{
	char node[16];
	snx_res_t res;
	unsigned int fps;
} snx_isp_attr_t;
*/
typedef struct snx_video
{
	snx_status_e status;
	snx_codec_e codec;		
	snx_res_t res;
	unsigned int fps;
	snx_bit_mode_e bit_mode;
	int gop;	
	int bitrate;
	int qp;
} snx_video_t;



typedef struct snx_attr_value
{
	char stream_id[SNX_STREAM_ID_LEN];
	snx_media_type_e type;
	union{
		snx_video_t video;
		snx_audio_t audio;		
	} attr;
	void *priv;
	int stop_flag;
} snx_attr_value_t;


typedef struct snx_snapshot
{
	int reinit_snap;
	char snap_conf[32];
	snx_res_t snap_res;
	void *priv;
} snx_snapshot_t;


int snx_add_admin_attr_changed_process ();
int snx_del_admin_attr_changed_process();
int snx_add_attr_changed_callback(const char *stream_id, int attr, snx_attr_callback_t pf_callback);         
int snx_del_attr_changed_callback(const char *stream_id, int attr);                                                         
int snx_get_stream_attr(int attr , snx_attr_value_t *val);
int snx_set_stream_attr(int attr , snx_attr_value_t *val);
int snx_get_osd_ctl_intf (char *info);
int snx_get_video_status (snx_codec_e codec, snx_res_t *res);
void snx_set_h264_iframe(snx_attr_value_t *attr);
int snx_init_snapshot (char *filename);
int snx_do_snapshot (char *path, char *suffix, char **filename);
int snx_deinit_snapshot ();
int snx_caplib_init(snx_attr_value_t *attr);
int snx_caplib_destroy (snx_attr_value_t *attr);
int snx_caplib_read_data(const snx_attr_value_t *attr, snx_caplib_data_t **outbuf);
int snx_caplib_reset_data(const snx_attr_value_t *attr , snx_caplib_data_t *freebuf);



#ifdef __cplusplus
}
#endif

#endif //__SNX_CAP_LIB_H__
