#ifndef __SNX_VC_LIB_H__
#define __SNX_VC_LIB_H__

#ifdef __cplusplus
extern "C"{
#endif

/** \file snx_vc_lib.h
 * Functions in this file as below :
 * \n  SONiX Video middleware header file, which include ..., ... functions.
 * \n  
 * \n 
 * \author Raymond Chu
 * \date   2015-05-05 update
 */

#include "generated/snx_sdk_conf.h"

#define	SNX_STREAM_STOP		0
#define	SNX_STREAM_START	1
#define	SNX_STREAM_PAUSE	2
typedef enum  {
	JPEG_CHROMA_SAMPLE_420 = 0,
	JPEG_CHROMA_SAMPLE_422,
} snx_mjpeg_sample_format;

// Unit: 16 pixels.
typedef	struct snx_cds_position
{
	unsigned int	start_x;
	unsigned int	start_y;
}POSITION;

// 
// Font	dimension.
// For picture:	
// dim_x = pic_width /16;
// dim_y = pic_height /	16;

typedef	struct snx_cds_dimension
{
	unsigned int	dim_x;
	unsigned int	dim_y;
}DIMENSION;

//
// [txty   txtcb   txtcr   bgy	 bgcb	bgcr]	
// Foreground Y, Cb, Cr. Background Y, Cb, Cr 
//
typedef	struct snx_cds_color
{
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
	unsigned int	color_R;
	unsigned int	color_G;
	unsigned int	color_B;
#endif
	unsigned int	color_Y;
	unsigned int	color_Cb;
	unsigned int	color_Cr;	
}COLOR;

//
// Color attr: [weighting mode]	 
// Weighting between background	and foreground.	
// Mode	is used	to select foreground or	background color.
//
// (1) if mode is 0 or 1, weight will be used,	
// and 0 stands	for background is transparent,	
// and 1 stands	for background is not transparent.
// (2) if mode is 2 or 3, weight will not be used,
// and 2 stands	for background is transparent,
// and 3 stands	for backgound is not transparent.
//

typedef	struct snx_cds_color_attr
{
	int	weight;			    // range: 0~7 //
	int	mode;			   // range: 0~3 //
}COLOR_ATTR;

/// 
// scaling_up_ratio  0:	1x1;   1: 2x2;	 2: 4x4	
///
typedef	enum snx_cds_scaling_ratio
{
	SCALING_1X1 = 0,
	SCALING_2X2,
	SCALING_4X4
}SCALING_UP_RATIO;

//#ifdef CONFIG_SYSTEM_PLATFORM_ST58660FPGA
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
#define	BI_RGB			0
#define	BI_RLE8			1
#define	BI_RLE4			2
#define	BI_BITFIELDS		3
#define	COLORMAP_OFFSET	0x36


typedef	struct snx_cds_rgb_quad
{
	unsigned char blue;
	unsigned char green;
	unsigned char red;
	unsigned char reserved;
}RGB_QUAD;
#endif	// CONFIG_SYSTEM_PLATFORM_ST58660FPGA

struct snx_cds
{
	char dev_name[64];
	char bmp_file[64];
	char *string;
	struct snx_cds_position pos;
	struct snx_cds_dimension dim;
	struct snx_cds_color t_color;
	struct snx_cds_color b_color;
	struct snx_cds_color bmp_threshold;

	struct snx_cds_color_attr attr;
	unsigned int enable;
	unsigned int scale;

	size_t width;
	size_t height;
	// bmp used
	unsigned int *bmp_src;
	unsigned int *bmp_dst;
	int bmp_size;
	int show_font;
//#ifdef CONFIG_SYSTEM_PLATFORM_ST58660FPGA
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
	unsigned short planes;		// number of color planes
	unsigned short bit_count;	// number of bits per pixel
	unsigned int compress_type;
	unsigned int image_size;	// size of image data
	unsigned int hw_size;		// size of image data
	unsigned int transparent_mode;
	size_t font_width;
	size_t font_height;
	struct snx_cds_rgb_quad bmp_rgb[256];
	unsigned char font_table_path[128];
	unsigned char bmp_rounding;
	unsigned char bytes_per_line[64];
	unsigned char pixels_per_line[64];
	unsigned int str_unicode[128];
	unsigned char str_len;
#endif	// CONFIG_SYSTEM_PLATFORM_ST58660FPGA	
};

struct buffer
{
    void *start;
    size_t length;
};


/*!
	\struct snx_m2m snx_vc_lib.h "include\snx_vc\snx_vc_lib.h"
	\brief struct snx_m2m
*/
struct snx_m2m
{
	char isp_dev[12];			//!< isp device name
	char codec_dev[12];			//!< encoder device name
	char dec_dev[13];			//!< 
	char ds_dev_name[64];		//!< Codec data stamp proc path name

	int isp_fd;					//!< isp file descriptor
	int codec_fd;				//!< encoder file descriptor
	int dec_fd;					//!< decoder file descriptor
	unsigned int last;			//!<
	// Sonix Set
	unsigned int m2m;			//!< V4L2 memory to memory mode

//	int socket_fd;				//!<
//	int thread_num;				//!<
	unsigned int scale;			//!< scale down set
	unsigned int ds_font_num;	//!< Codec data stamp alloction font size number

	int dyn_fps_en;
	struct timeval timestamp;

	// Basic Set
	size_t width;
	size_t height;
	int isp_fps;
	int codec_fps;
    
	int bit_rate;
	int qp;
	int gop;
	int m2m_buffers;
	unsigned int flags;
	int force_i_frame;

	// V4L2 Set
	struct buffer *isp_buffers;
	struct buffer *cap_buffers;
	struct buffer *out_buffers;

	struct buffer *dec_out_buffers;
	struct buffer *dec_cap_buffers;

	int isp_index;
	int cap_index;
	int cap_bytesused;

	int dec_cap_index;
	int dec_cap_bytesused;
	int dec_out_index;
	int dec_out_bytesused;

	unsigned int isp_fmt;
	unsigned int out_fmt;
	unsigned int codec_fmt;
	unsigned int dec_fmt;

	unsigned int cap_mem;
	unsigned int out_mem;  
	unsigned int isp_mem;

	unsigned int dec_out_mem;  
	unsigned int dec_cap_mem;

//#ifdef CONFIG_SYSTEM_PLATFORM_ST58660FPGA
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
	//MBRC
	unsigned int mbrc_qp_part;
	unsigned int mbrc_mad_sum;
	unsigned int mbrc_bs_actual0;
	unsigned int mbrc_bs_actual1;
	unsigned int mbrc_bs_actual2;
	unsigned int mbrc_bs_actual3;
#endif	// CONFIG_SYSTEM_PLATFORM_ST58660FPGA

	void *p_rdr;
	int codec_streamon;
};


/*
// add video output
int snx_vo_init(struct snx_m2m *m2m);
int snx_vo_start(struct snx_m2m *m2m);
int snx_vo_read(struct snx_m2m *m2m);
int snx_vo_reset(struct snx_m2m *m2m);
int snx_vo_stop(struct snx_m2m *m2m);
int snx_vo_uninit(struct snx_m2m *m2m);
*/

/*!
	\defgroup Video middleware video group function
	\details code flow example (Detailed example on /app/example/src/video/...)
\verbatim
	m2m->codec_fd = snx_open_device(m2m->codec_dev);	
	ret = snx_codec_init(m2m);	
	ret = snx_codec_start(m2m);
		snx_codec_set_gop(m2m);
	while(1) {
		ret = snx_codec_read(m2m);
		.... (Get Frame from pointer m2m->cap_buffers[m2m->cap_index].start)
		ret = snx_codec_reset(m2m);
	}
\endverbatim	

	\details m2m code flow example (Detailed example on /app/example/src/video/...)

	@{

	\fn int snx_open_device(char *dev_name)
	\description Open device name
	\n
	\param snx_m2m : video middleware struct
	\n
	\return file descriptor

	\fn int snx_isp_init(struct snx_m2m *m2m)
	\description  Integrate V4L2 I/O control: 
	\n VIDIOC_QUERYCAP, VIDIOC_CROPCAP,
	\n VIDIOC_S_CROP, VIDIOC_S_PARM, 
	\n VIDIOC_G_PARM, VIDIOC_S_FMT
	\n VIDIOC_REQBUFS, VIDIOC_QUERBUF
	\n
	\param snx_m2m : video middleware struct
	\n
	\return ret

	\fn int snx_isp_start(struct snx_m2m *m2m)
	\description Integrate V4L2 I/O control: VIDIOC_QBUF, VIDIOC_STREAMON
	\n
	\param snx_m2m : video middleware struct
	\n
	\return ret

	\fn int snx_isp_read(struct snx_m2m *m2m)
	\description Integrate V4L2 I/O control: VIDIOC_DQBUF (reserved)
	\n
	\param snx_m2m : video middleware struct
	\n
	\return ret

	\fn int snx_isp_reset(struct snx_m2m *m2m)
	\description Integrate V4L2 I/O control: VIDIOC_QBUF (reserved)
	\n
	\param snx_m2m : video middleware struct
	\n
	\return ret

	\fn int snx_isp_stop(struct snx_m2m *m2m)
	\description Integrate V4L2 I/O control: VIDIOC_STREAMOFF
	\n
	\param snx_m2m : video middleware struct
	\n
	\return ret
  
	\fn int snx_isp_uninit(struct snx_m2m *m2m)
	\description release memory
	\n
	\param snx_m2m : video middleware struct
	\n
	\return ret

	\fn int snx_codec_init(struct snx_m2m *m2m)
	\description  Integrate V4L2 I/O control: VIDIOC_QUERYCAP, VIDIOC_S_CTRL and VIDIOC_S_FMT
	\n
	\param snx_m2m : video middleware struct
	\n
	\return ret
 
	\fn int snx_codec_start(struct snx_m2m *m2m)
	\description Integrate V4L2 I/O control: 
	\n VIDIOC_REQBUFS, VIDIOC_QUERBUF
	\n VIDIOC_QBUF, VIDIOC_STREAMON
	\n
	\param snx_m2m : video middleware struct
	\n
	\return ret
	\n

	\fn int snx_codec_read(struct snx_m2m *m2m)
	\description Integrate V4L2 I/O control: VIDIOC_DQBUF, VIDIOC_QBUF
	\n
	\param snx_m2m : video middleware struct
	\n
	\return ret

	\fn int snx_codec_reset(struct snx_m2m *m2m)
	\description Integrate V4L2 I/O control: VIDIOC_DQBUF, include snx_isp_read() API
	\n
	\param snx_m2m : video middleware struct
	\n
	\return ret

	\fn int snx_codec_stop(struct snx_m2m *m2m)
	\description Integrate V4L2 I/O control: VIDIOC_STREAMOFF
	\n
	\param snx_m2m : video middleware struct
	\n
	\return ret
 
 	\fn snx_codec_uninit(struct snx_m2m *m2m)
	\description release memory 
	\n
	\param snx_m2m : video middleware struct
	\n
	\return ret

	\fn int snx_codec_set_qp(struct snx_m2m *m2m, unsigned int type)
	\description Set H264/JPEG encoder QP value
	\n
	\param snx_m2m : video middleware struct
	\n
	\return ret

	\fn int snx_codec_set_gop(struct snx_m2m *m2m)
	\description  Set H264 encoder GOP value
	\n
	\param snx_m2m : video middleware struct
	\n
	\return ret

 @} 
*/
int snx_open_device(char *dev_name);
int snx_isp_init(struct snx_m2m *m2m);
int snx_isp_start(struct snx_m2m *m2m);
int snx_isp_read(struct snx_m2m *m2m);
int snx_isp_reset(struct snx_m2m *m2m);
int snx_isp_stop(struct snx_m2m *m2m);
int snx_isp_uninit(struct snx_m2m *m2m);

int snx_codec_init(struct snx_m2m *m2m);
int snx_codec_start(struct snx_m2m *m2m);
int snx_codec_read(struct snx_m2m *m2m);
int snx_codec_reset(struct snx_m2m *m2m);
int snx_codec_stop(struct snx_m2m *m2m);
int snx_codec_uninit(struct snx_m2m *m2m);

int snx_codec_set_qp(struct snx_m2m *m2m, unsigned int type);
int snx_codec_set_gop(struct snx_m2m *m2m);

int snx_bs_reader_init(struct snx_m2m *m2m, const char *filename);
int snx_bs_reader_uninit(struct snx_m2m *m2m);
/**
 * before calling this helper function, application should guarantee there is a 
 * complete frame in bs_buf if input filename is /dev/mem
 */
int snx_bs_reader_read_frame(struct snx_m2m *m2m,
		const char *bs_buf, size_t bs_buf_sz,
		const char *dst_buf, size_t dst_buf_sz);

int snx_dec_init(struct snx_m2m *m2m);
int snx_dec_start(struct snx_m2m *m2m);
int snx_dec_read(struct snx_m2m *m2m);
int snx_dec_reset(struct snx_m2m *m2m);
int snx_dec_stop(struct snx_m2m *m2m);
int snx_dec_uninit(struct snx_m2m *m2m);
int snx_dec_set_mjpeg_sample_format(struct snx_m2m *m2m, snx_mjpeg_sample_format format);

int snx_dec2enc(struct snx_m2m *m2m);
int snx_dec2enc_reset(struct snx_m2m *m2m);

//int snx_cds_set_bmp(char *dev_name, char *bmp_file, SCALING_UP_RATIO scaling_up_ratio);
int snx_cds_set_bmp(struct snx_cds *cds);
int snx_cds_get_datastamp(char *dev_name, char *data, int len);
int snx_cds_set_datastamp(char *dev_name, char *data, int len);
int snx_cds_get_position(char *dev_name, POSITION *position, DIMENSION *dimension);
int snx_cds_set_position(char *dev_name, POSITION *position, DIMENSION *dimension);
int snx_cds_get_color(char *dev_name, COLOR *forg_color, COLOR *bkg_color);
int snx_cds_set_color(char *dev_name, COLOR *forg_color, COLOR *bkg_color);
int snx_cds_get_color_attr(char *dev_name, COLOR_ATTR *color_attr);
int snx_cds_set_color_attr(char *dev_name, COLOR_ATTR *color_attr);
int snx_cds_get_scale(char *dev_name, SCALING_UP_RATIO *scaling_up_ratio);
int snx_cds_set_scale(char *dev_name, SCALING_UP_RATIO scaling_up_ratio);
int snx_cds_get_enable(char *dev_name, int *enable);
int snx_cds_set_enable(char *dev_name, int enable);

int snx_cds_set_all(char *dev_name, struct snx_cds *cds);
int snx_scale_down_bmppicture (char *in_bmp_file, char* out_bmp_file);
int snx_scale_up_bmppicture (char *in_bmp_file,char *out_bmp_file);

//#ifdef CONFIG_SYSTEM_PLATFORM_ST58660FPGA
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
int snx_cds_set_transparent_mode(char *dev_name, unsigned int mode);
//int snx_cds_set_font_table(struct snx_cds *cds, char *outside_ascii);
unsigned int* snx_cds_get_font_table(struct snx_cds *cds);
int snx_cds_set_string(struct snx_cds *cds, char *outside_ascii, unsigned int *outside_font, int font_size);
#else
int snx_cds_set_string(char *dev_name, char *data, SCALING_UP_RATIO scaling_up_ratio);
#endif	// CONFIG_SYSTEM_PLATFORM_ST58660FPGA

#ifdef __cplusplus
}
#endif

#endif //__SNX_VC_LIB_H__
