
#ifndef __SNX_VIDEO_TYPE_H__
#define __SNX_VIDEO_TYPE_H__

#ifdef __cplusplus
extern "C"{
#endif

#define V4L2_PIX_FMT_SNX420   v4l2_fourcc('S', '4', '2', '0')

#define V4L2_PIX_FMT_H264	  		v4l2_fourcc('H', '2', '6', '4') /* H.264 Annex-B NAL Units */
#define V4L2_PIX_FMT_H264_MJPEG	  	v4l2_fourcc('H', 'M', '0', '0') /* H.264 & MJPEG */


typedef struct snx_frame_ctx{
	int index;
	int iotype;
	void *userptr;
	int length;
	int size;
	int reserved;
}*snx_frame_ctx_t;

#ifdef __cplusplus
}
#endif


#endif /*__SNX_VIDEO_TYPE_H__*/
