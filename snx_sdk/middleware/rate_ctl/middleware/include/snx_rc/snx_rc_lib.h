#ifndef __SNX_RC_LIB_H__
#define __SNX_RC_LIB_H__

#ifdef __cplusplus
extern "C"{
#endif

#define SNX_RC_INIT	0
#define SNX_RC_REINIT	1

#include "generated/snx_sdk_conf.h"
#include <stdio.h>


#define SNX_RC_MSG_KEY 98624


#if MD_DEBUG==1
// MD input
#define RC_CONFIG			"rc.config"		// md_recover = 25;


#define MD_RECOVER			"md_recover"		// md_recover = 25;
#define MD_M				"md_m"			// md_m = 3;
#define MD_N				"md_n"			// md_n = 3;
#define MD_TH				"md_th"			// md_th = 1;
#define MD_ADD_BITRATE		"md_can_add_bitrate"	// md_can_add_bitrate = 0;
#define MAX_FPS				"max_fps"		// fps = 30;
#define MD_MAX_FPS			"md_max_fps"		// fps = 8;
#define MD_ISP_NR			"md_2dnr_cnt"		// md_isp_nr = 23;
#define MDRC_EN				"md_ratectl_en"		// enable md rate control
#define MD_2DNR				"md_2dnr"		// 2DNR

#define RC_UP				"rc_up"			// rc_up = 6;
#define RC_RATE				"rc_rate"		// rc_rate = 50;
#define RC_MAX_QP			"rc_max_qp"		// rc_max_qp = 40
#define RC_MIN_QP			"rc_min_qp"		// rc_max_qp = 0


#define MBRC_EN				"mbrc_en"		// mbrc enable = 1;
#define SKIP_RC_CYCLE		"skip_rc_cycle"		// skip rc cycle = 0 ~ N;

#define IFRAME_UPBOUND		"iframe_upbound"		//

#define MD_CNT_TH			"md_cnt_th"		//
#define MD_CNT_EN			"md_cnt_en"		//
#define MD_CNT_SUM_TH		"md_cnt_sum_th"		//
#define MD_CNT_BPS			"md_cnt_bps"		//
#define MD_CNT_BPS2			"md_cnt_bps2"		//
#define MD_CNT_GOP_MULTI	"md_cnt_gop_multiple"		//
#define MD_CNT_COUNT		"md_cnt_count"		//
#define MD_CNT_LOWBOUND		"md_cnt_lowbound"		//
#define MD_CNT_QP			"md_cnt_qp"		//
#define MD_CNT_ABSY			"md_cnt_absy"		//

#endif // MD_DEBUG

struct snx_rc_ext
{
	int mdrc_en;		//motion detection rate contorl enable (default 1)
	int md_recover;		//finish MD, wating frame counter (default 25)
	int md_m;		//corner width MD block  ( < 1/2*(16), default 3 )
	int md_n;		// corner height MD block ( < 1/2*(12), default 3 )
	int md_th;		// corner trigger counter ( <= (md_m * md_n), default 1 )
	int md_can_add_bitrate;	// MD enable, add bitrate ( Kbps, default 0 )

	int md_max_fps;		// MD enable, output frame rate ( < sensor fps, default 8 )
	int md_isp_nr;		// finish MD, wating blured frame counter (default 23)
	int md_2dnr;		// MD enable, blured level ( 0 ~ 4, default 1 )
	int rc_up;		// max bit rate == bit_rate * (1 + (rc_up /32)) (default 6 )
	int rc_rate;		// bit_rate_exceed = bitrate_exceed *(1 - (1/ rc_rate)) (default 50)
	int mbrc_en;
	int skip_rc_cycle;
	// rc 
	int rc_update;
	int bps;
	int isp_fps;
	float codec_fps;

	int Iframe_UpBound ;	//

	int md_cnt_en;
	int md_cnt_th;
	int md_cnt_sum_th;
	int md_cnt_bps;
	int md_cnt_bps2;
	int md_cnt_gop_multiple;
	int md_cnt_count;
	int md_cnt_lowbound;
	int md_cnt_qp;
	int md_cnt_absy;
	
	int snx_msg_id;

	int rc_max_qp;
	int rc_min_qp;
	
	char rc_folder[64];
		
};

typedef struct snx_rc_msg
{	
	long msg_type;	     //pipe num
//	unsigned int cmd;
	struct snx_rc_ext snx_rc_ext;

}snx_rc_msg_t;

// snx rate control message command type
// application --> library
#define	SNX_SET_BPS			4
#define	SNX_SET_ISP_FPS		5
#define	SNX_SET_CODEC_FPS	6

#define	SNX_RC_SET_ISP		8
#define	SNX_RC_GET_EXT		9
#define	SNX_RC_SET_EXT		10
#define	SNX_RC_MSG_TYPE		-15

// library --> application
#define	SNX_RC_RESP_EXT		16



#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
#define V4L2_CID_MBRC_ENABLE			(V4L2_CID_USER_BASE + 0x1005)
#define V4L2_CID_MBRC_BS_TARGET0		(V4L2_CID_USER_BASE + 0x1006)
#define V4L2_CID_MBRC_BS_TARGET1		(V4L2_CID_USER_BASE + 0x1007)
#define V4L2_CID_MBRC_BS_TARGET2		(V4L2_CID_USER_BASE + 0x1008)
#define V4L2_CID_MBRC_BS_TARGET3		(V4L2_CID_USER_BASE + 0x1009)
#define V4L2_CID_MBRC_MAD_AVE			(V4L2_CID_USER_BASE + 0x100A)

#define EXT_SIZE_LARGE	2
#define EXT_SIZE_MEDIUM	1
#define EXT_SIZE_NONE	0

struct snx_mroi
{
	unsigned int region_num;
	unsigned char weight; 
	int qp;
	unsigned int pos_x;
	unsigned int dim_x;
	unsigned int pos_y;
	unsigned int dim_y;
	unsigned char ext_size;	
};
#endif	// CONFIG_SYSTEM_PLATFORM_ST58660FPGA

struct snx_rc
{
	int codec_fd;
	size_t width;
	size_t height;
	unsigned int gop;
	int frames_gop;
	int Targetbitrate;	
	int rtn_quant;
	int Previous_frame_size;
	int BitRemain;
	int IFrameAve;
	int IntraPreDeviation;
	int FirstPQP;
	int IntraQP;
	int FrameCntRemain;
	int framerate;
	int total_quant_gop;
	int QpAdjEnable;
	int QpAdjStopCnt;
	int QpDifSum;
	int NFirstGOP;
	int IFrameCnt;
	int CurSecHaveIFrame;
	int TotalBitPerSec;
	int frames;
	int current_frame_size;

//initial value
	int UpBoundUp;
	int UpBoundDw;
	int ConvergenceRate;
	int QPStopCnt;
	int SumQPBound;
	int UpBoundBytesPerSec;
	int BitExceed;
	int RealBytesPerSec;
	int SumQPCnt;
	int I_frame_size[4];
	int bufsize;

	// Ryan add 20150313, For skip rate control
	int AveTarFrameSize; 
	int RC_Calculate_Cycle;
	int RC_Skip_Enable;
	
	// Ryan add 2014/04/07
	int Pre_Framerate;
	int md_flag;	// re-init rate contorl function
	
	// Ryan add 2015/04/27
	int Iframe_Control ;
	int AVE_quant ;
	int Iframe_UpBound ;
	int total_quant_per_second;
	int AVE_quant_per_second ;
	int AVE_quant_Cnt;

	int stable_count;
	int in_md;
	int can_add_bitrate;
	int Tempbitrate;
	

	// 2015 0502
	int md_cnt_count;	
	int md_cnt_down;
	int md_cnt_bps;
	int md_cnt_lowlux;
	
	int md_cnt;
	
	int iframe_large;

#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
//MBRC
	unsigned int predbit;
	unsigned int mb_num;
	unsigned char ratio[4];
	unsigned int bs_target_sz[4];
	int mad_sum;
	int mad_ave;
	unsigned int qp_part;
	unsigned int bs_actual0;
	unsigned int bs_actual1;
	unsigned int bs_actual2;
	unsigned int bs_actual3;
	unsigned int current_slice_size[4];
	unsigned int previous_slice_size[4];
	unsigned int current_slice_qp[4];
	unsigned int previous_slice_qp[4];
	unsigned int current_par_slice_size[4];
	unsigned int previous_par_slice_size[4];
	unsigned char par_qp_adj[7];
	unsigned char par_qp_num[8];
	unsigned char mb_qp_adj[6];
	unsigned char mb_qp_num[7];
	unsigned char mb_qp_max;
	unsigned char mb_qp_min;
	unsigned int mb_mode;
	unsigned int mb_thd;
	unsigned char skin_det_en;
	unsigned char skin_qp_sub1;
	unsigned char skin_qp_sub2;
	unsigned char scene_chd_det_en;
	unsigned char scene_chd_det_adj;
	unsigned char scene_chd_det_num;
	
//MROI
	unsigned int sum_area;
	struct snx_mroi mroi_region[8];


#endif	// CONFIG_SYSTEM_PLATFORM_ST58660FPGA
	struct snx_rc_ext snx_rc_ext;
	
	FILE *md_count_fp;
	FILE *ae_fps_fp;
	FILE *target_fps_fp;
	FILE *isp_fps_fp;
	
	char target_fps_proc[64];
	char isp_fps_proc[64];
	char rc_folder[64];

};


//extern int snx_codec_rc_init(struct snx_rc *rc);
extern int snx_codec_rc_init(struct snx_rc *rc, int reinit);

/*
#ifdef CONFIG_SYSTEM_PLATFORM_ST58660FPGA
#ifndef __SNX_VC_LIB_H__
#include "snx_vc_lib.h"
#endif	// struct snx_m2m
#endif	// CONFIG_SYSTEM_PLATFORM_ST58660FPGA
*/

extern int snx_codec_rc_update(struct snx_m2m *m2m, struct snx_rc *rc);
extern int snx_codec_rc_reinit(struct snx_m2m *m2m, struct snx_rc *rc);
extern int snx_420line_to_420(char *p_in, char *p_out, unsigned int width, unsigned int height);
extern int snx_420_to_420line(char *p_in, char *p_out, unsigned int width, unsigned int height);
//extern int snx_md_corner (int idx, unsigned int m, unsigned int n, int count);
extern int snx_md_corner (int idx, struct snx_rc *rc);
extern int snx_isp_denoise(int level);
extern int snx_md_drop_fps (struct snx_rc *rate_ctl, int *force_i_frame);
extern int snx_get_file_value(char *path, int *value, int base);
extern int snx_get_file_string(char *path, char *str);
extern int snx_set_file_value(char *path, int *value, int base);
extern int snx_get_file_string(char *path, char *str);
extern int snx_set_file_string(char *path, char *str);


#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
int snx_codec_mbrc_enable(int codec_fd,unsigned char enable);
int snx_codec_mbrc_bs_target(int codec_fd,unsigned int target, unsigned char index);
int snx_codec_mbrc_mad_ave(int codec_fd, unsigned int mad_ave);
void snx_codec_mbrc_set_par_qp_adj(struct snx_m2m *m2m, struct snx_rc *rc);
void snx_codec_mbrc_set_par_qp_num(struct snx_m2m *m2m, struct snx_rc *rc);
void snx_codec_mbrc_set_mb_qp_adj(struct snx_m2m *m2m, struct snx_rc *rc);
void snx_codec_mbrc_set_mb_qp_num(struct snx_m2m *m2m, struct snx_rc *rc);
void snx_codec_mbrc_set_qp_max_min(struct snx_m2m *m2m, struct snx_rc *rc);
void snx_codec_mbrc_set_mode_thd(struct snx_m2m *m2m, struct snx_rc *rc);

extern void snx_codec_mroi_enable(struct snx_m2m *m2m, unsigned char enable);
extern void snx_codec_mroi_set_region_attr(struct snx_m2m *m2m, struct snx_rc *rc, int num);
extern void snx_codec_mroi_set_hightbitrate(struct snx_m2m *m2m, struct snx_rc *rc);

#endif	// CONFIG_SYSTEM_PLATFORM_ST58660FPGA
int snx_rc_msg_create (char *device);
//int snx_rc_msg_recv (int snx_msg_id, snx_rc_msg_t *snx_rc_recv, int type);
int snx_rc_msg_recv (int snx_msg_id, snx_rc_msg_t *snx_rc_recv, struct snx_rc_ext *rc_ext, int type);
int snx_rc_msg_send (int snx_msg_id, snx_rc_msg_t snx_rc_send);

int snx_rc_ext_set(struct snx_rc_ext *rc_ext);

int snx_rc_ext_get(struct snx_rc_ext *rc_ext);
//int snx_rc_ext_get(struct snx_rc *rc);


#ifdef __cplusplus
}
#endif
#endif //__SNX_RC_LIB_H__
