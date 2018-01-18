/**
 *
 * SONiX SDK Example Code
 * Category: Video Encode Rate Contorl tool
 * File: snx_rc_ctl.c
 * Usage: 
 *	 1. Make sure at least a stream has been run (./snx_rc_ctl)
 *       2. Input the correct dev name of the stream (refer to the usage )
 *       3. Input the rest of arguments you want to setup (refer to the usage)
 *	 4. Example: ./snx_rc_ctl 
 * NOTE:
 *       Recording all streams to SD card would cause the target framerate can
 *       not be reached because of the bandwidth leakage of SD card.
 */
#include <stdint.h>
#include "snx_video_codec.h"
#include <sys/msg.h>

/*-----------------------------------------------------------------------------
 * Example Code Configuration
 *----------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------
 * GLOBAL Variables
 *----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
 * Example Code Flow
 *----------------------------------------------------------------------------*/


/*
	The entrance of this file.
	One M2M streams would be created.
	The configuration can set on the setting definitions above. 
	After the stream conf are done, the thread of the stream would be created.
	The bitstreams of the stream will be record on the SD card.
*/

static const char short_options[] = "hd:m:v:x:y:t:a:f:w:2:u:r:sb:i:c:o:e:j:k:g:l:n:p:q:z:1:";

static const struct option long_options[] = {
    {"help", no_argument, NULL, 'h'},
    {"device", required_argument, NULL, 'd'},
    {"mdrc", required_argument, NULL, 'm'},
    {"md_recover", required_argument, NULL, 'v'},
    {"md_m", required_argument, NULL, 'x'},
    {"md_n", required_argument, NULL, 'y'},
    {"md_th", required_argument, NULL, 't'},
    {"md_add_br", required_argument, NULL, 'a'},
    {"md_max_fps", required_argument, NULL, 'f'},
    {"md_isp_nr", required_argument, NULL, 'w'},
    {"md_2dnr", required_argument, NULL, '2'},
    {"rc_up", required_argument, NULL, 'u'},
    {"rc_rate", required_argument, NULL, 'r'},
    {"show", no_argument, NULL, 's'},
    {"bps", required_argument, NULL, 'b'},
    {"isp_fps", required_argument, NULL, 'i'},
    {"codec_fps", required_argument, NULL, 'c'},

    {"iframe_upbound", required_argument, NULL, 'o'},
    {"md_cnt_en", required_argument, NULL, 'e'},
    {"md_cnt_th", required_argument, NULL, 'j'},
    {"md_cnt_sum_th", required_argument, NULL, 'p'},
    {"md_cnt_bps", required_argument, NULL, 'k'},
    {"md_cnt_bps2", required_argument, NULL, 'n'},
    {"md_cnt_gop_multiple", required_argument, NULL, 'g'},
    {"md_cnt_count", required_argument, NULL, 'l'},
    {"md_cnt_lowbound", required_argument, NULL, 'z'},
    {"md_cnt_qp", required_argument, NULL, 'q'},
    {"md_cnt_absy", required_argument, NULL, '1'},


    {0, 0, 0, 0}
};


static void usage(FILE * fp, int argc, char ** argv) {   
    fprintf(fp, "Usage: %s [options]/n\n"   
        "Options:\n"
        "-h Print this message\n"
        "-d | --device	     [1_xs] select on /proc/codec/* \n"
        "		       [1] means /dev/video1\n"
        "		       [2] means /dev/video2\n"
        "		       [x] can have two options\n"
        "		           [h] for H264\n"
        "		           [j] for MJPEG\n"
	"			   [s] scaling stream\n"
        "-m | --mdrc	     motion detection rate contorl enable (default 1)\n"
        "-e | --rc_ext       Get all config value\n"
        "-v | --md_recover   finish MD, wating frame counter (default 25)\n"
        "-x | --md_m	     corner width MD block  ( < 1/2*(16), default 3 )\n"
        "-y | --md_n	     corner height MD block ( < 1/2*(12), default 3 )\n"
        "-t | --md_th	     corner trigger counter ( <= (md_m * md_n), default 1 )\n"
        "-a | --md_add_br    MD enable, add bitrate ( Kbps, default 0 )\n"
        "-f | --md_max_fps   MD enable, output frame rate ( < sensor fps, default 8 )\n"
        "-w | --md_isp_nr    finish MD, wating blured frame counter (default 23)\n"
        "-2 | --md_2dnr	     MD enable, blured level ( 0 ~ 4, default 1 )\n"
        "-u | --rc_up	     max bit rate == bit_rate * (1 + (rc_up /32)) (default 6 )\n"
        "-r | --rc_rate	     bit_rate_exceed = bitrate_exceed *(1 - (1/ rc_rate)) (default 50)\n"
        "-b | --bps	     bps\n"
        "-i | --isp_fps	     isp fps\n"
        "-c | --codec_fps    codec fps\n"
        "-o | --iframe_upbound    \n"
        "-e | --md_cnt_en     \n"
        "-j | --md_cnt_th     \n"
        "-p | --md_cnt_sum_th     \n"
        "-k | --md_cnt_bps    \n"
        "-n | --md_cnt_bps2    \n"
        "-g | --md_cnt_gop_multiple     \n"
        "-l | --md_cnt_count     \n"
        "-z | --md_cnt_lowbound     \n"
        "-q | --md_cnt_qp     \n"
        "-1 | --md_cnt_absy     \n"


	"", argv[0]);   
}


int main(int argc, char **argv)
{
	int ret = 0;
	int snx_msg_id = 0;
	snx_rc_msg_t snx_rc_msg;
	char device[16];
	int value =0;
	struct snx_rc_ext snx_rc_ext;
	int show_ext = 0, update_ext = 0;
	int isp_req = 0; // Only one resource 
	int rc_req = 0; // rc
	
	memset(&snx_rc_ext, -1, sizeof(struct snx_rc_ext));
//	snx_rc_ext_get(&snx_rc_msg.snx_rc_ext);
	for (;;)
	{   
		int index;
		int c;
		c = getopt_long(argc, argv, short_options, long_options, &index);
		if (-1 == c)
			break;
		switch (c) {   
			case 0: /* getopt_long() flag */
				break;
			case 'h':   
				usage(stdout, argc, argv);
				exit(EXIT_SUCCESS);
			case 's':
				show_ext =1;
				break;
			case 'd':
				if (strlen(optarg) != 0) {
					printf( "%s\n",optarg);
					sprintf(device, "%s", optarg);
				}
				break;
			case 'm':
				sscanf(optarg, "%d", &value );
				if(value !=0)
					snx_rc_ext.mdrc_en	=1;
				else
					snx_rc_ext.mdrc_en	=0;
				update_ext++;
				isp_req++;
				printf( "SNX_RC_MD_EN = %d\n",snx_rc_ext.mdrc_en);	

				break;
			case 'v':
				sscanf(optarg, "%d", &snx_rc_ext.md_recover );
				update_ext++;
				printf( "SNX_RC_MD_RECOVER = %d\n",snx_rc_ext.md_recover);
				break;
			case 'x':
				sscanf(optarg, "%d", &snx_rc_ext.md_m );
				update_ext++;
				printf( "SNX_RC_MD_M = %d\n",snx_rc_ext.md_m);
				break;
			case 'y':
				sscanf(optarg, "%d", &snx_rc_ext.md_n );
				update_ext++;
				printf( "SNX_RC_MD_N = %d\n",snx_rc_ext.md_n);
				break;
			case 't':
				sscanf(optarg, "%d", &snx_rc_ext.md_th );
				update_ext++;
				printf( "SNX_RC_MD_TH = %d\n",snx_rc_ext.md_th);
				break;
			case 'a':
				sscanf(optarg, "%d", &snx_rc_ext.md_can_add_bitrate );
				update_ext++;
				printf( "SNX_RC_MD_ADD_BR = %d\n",snx_rc_ext.md_can_add_bitrate);
				break;
			case 'f':
				sscanf(optarg, "%d", &snx_rc_ext.md_max_fps );
				update_ext++;
				isp_req++;
				printf( "SNX_RC_MD_MAX_FPS = %d\n",snx_rc_ext.md_max_fps);
				break;
			case 'w':
				sscanf(optarg, "%d", &snx_rc_ext.md_isp_nr );
				update_ext++;
				isp_req++;
				printf( "SNX_RC_MD_ISP_NR = %d\n",snx_rc_ext.md_isp_nr);
				break;
			case '2':
				sscanf(optarg, "%d", &snx_rc_ext.md_2dnr );
				update_ext++;
				isp_req++;
				printf( "SNX_RC_MD_2DNR = %d\n",snx_rc_ext.md_2dnr);
				break;
			case 'u':
				sscanf(optarg, "%d", &snx_rc_ext.rc_up);
				update_ext++;
				printf( "SNX_RC_UP = %d\n",snx_rc_ext.rc_up);
				break;
			case 'r':
				sscanf(optarg, "%d", &snx_rc_ext.rc_rate );
				update_ext++;
				printf( "SNX_RC_RATE = %d\n",snx_rc_ext.rc_rate);
				break;
			case 'b':
				sscanf(optarg, "%d", &snx_rc_ext.bps );
				rc_req++;
				printf( "bps = %d\n",snx_rc_ext.bps);
				break;
			case 'i':
				sscanf(optarg, "%d", &snx_rc_ext.isp_fps );
				rc_req++;
				printf( "isp_fps = %d\n",snx_rc_ext.isp_fps);
				break;
			case 'c':
				sscanf(optarg, "%f", &snx_rc_ext.codec_fps );
				rc_req++;
				printf( "codec_fps = %f\n",snx_rc_ext.codec_fps);
				break;				
			case 'o':
				sscanf(optarg, "%d", &snx_rc_ext.Iframe_UpBound );
				update_ext++;
				printf( "Iframe_UpBound = %d\n",snx_rc_ext.Iframe_UpBound);
				break;
			case 'e':
				sscanf(optarg, "%d", &snx_rc_ext.md_cnt_en );
				update_ext++;
				printf( "md_cnt_en = %d\n",snx_rc_ext.md_cnt_en);
				break;	
			case 'j':
				sscanf(optarg, "%d", &snx_rc_ext.md_cnt_th );
				update_ext++;
				printf( "md_cnt_th = %d\n",snx_rc_ext.md_cnt_th);
				break;	
			case 'p':
				sscanf(optarg, "%d", &snx_rc_ext.md_cnt_sum_th );
				update_ext++;
				printf( "md_cnt_sum_th = %d\n",snx_rc_ext.md_cnt_sum_th);
				break;	
			case 'k':
				sscanf(optarg, "%d", &snx_rc_ext.md_cnt_bps );
				update_ext++;
				printf( "md_cnt_bps = %d\n",snx_rc_ext.md_cnt_bps);
				break;	
			case 'n':
				sscanf(optarg, "%d", &snx_rc_ext.md_cnt_bps2 );
				update_ext++;
				printf( "md_cnt_bps2 = %d\n",snx_rc_ext.md_cnt_bps2);
				break;	

			case 'g':
				sscanf(optarg, "%d", &snx_rc_ext.md_cnt_gop_multiple );
				update_ext++;
				printf( "md_cnt_gop_multiple = %d\n",snx_rc_ext.md_cnt_gop_multiple);
				break;	
			case 'l':
				sscanf(optarg, "%d", &snx_rc_ext.md_cnt_count );
				update_ext++;
				printf( "md_cnt_count = %d\n",snx_rc_ext.md_cnt_count);
				break;					
			case 'z':
				sscanf(optarg, "%d", &snx_rc_ext.md_cnt_lowbound );
				update_ext++;
				printf( "md_cnt_lowbound = %d\n",snx_rc_ext.md_cnt_lowbound);
				break;	
			case 'q':
				sscanf(optarg, "%d", &snx_rc_ext.md_cnt_qp);
				update_ext++;
				printf( "md_cnt_qp = %d\n",snx_rc_ext.md_cnt_qp);
				break;									
			case '1':
				sscanf(optarg, "%d", &snx_rc_ext.md_cnt_absy);
				update_ext++;
				printf( "md_cnt_absy = %d\n",snx_rc_ext.md_cnt_absy);
				break;	
			default:
				usage(stderr, argc, argv);
				exit(EXIT_FAILURE);
		}
	}
//	snx_rc_ext_dump(&snx_rc_ext);

	snx_msg_id =snx_rc_msg_create(device);

	snx_rc_msg.msg_type = SNX_RC_GET_EXT;
	snx_rc_msg_send(snx_msg_id, snx_rc_msg);
	

				
	snx_rc_msg_recv(snx_msg_id, &snx_rc_msg, NULL, SNX_RC_RESP_EXT);



	if(show_ext)
		snx_rc_ext_dump(&snx_rc_msg.snx_rc_ext);
	
	if(rc_req) {
		if(snx_rc_msg.snx_rc_ext.bps != snx_rc_ext.bps && snx_rc_ext.bps != -1) {
			printf( "bps = %d %d\n",snx_rc_msg.snx_rc_ext.bps, snx_rc_ext.bps);
			snx_rc_msg.snx_rc_ext.bps = snx_rc_ext.bps;
			snx_rc_msg.msg_type = SNX_SET_BPS;
			snx_rc_msg_send(snx_msg_id, snx_rc_msg);
		}
		if(snx_rc_msg.snx_rc_ext.isp_fps != snx_rc_ext.isp_fps && snx_rc_ext.isp_fps != -1) {
			printf( "isp fps = %d %d\n",snx_rc_msg.snx_rc_ext.isp_fps, snx_rc_ext.isp_fps);
			snx_rc_msg.snx_rc_ext.isp_fps = snx_rc_ext.isp_fps;
			snx_rc_msg.msg_type = SNX_SET_ISP_FPS;
			snx_rc_msg_send(snx_msg_id, snx_rc_msg);

		}
		if(snx_rc_msg.snx_rc_ext.codec_fps != snx_rc_ext.codec_fps && snx_rc_ext.codec_fps != -1) {
			printf( "codec fps = %.2f %.2f\n",snx_rc_msg.snx_rc_ext.codec_fps, snx_rc_ext.codec_fps);
			snx_rc_msg.snx_rc_ext.codec_fps = snx_rc_ext.codec_fps;
			snx_rc_msg.msg_type = SNX_SET_CODEC_FPS;
			snx_rc_msg_send(snx_msg_id, snx_rc_msg);

		}
	}	
	
	if(update_ext == 0)
		return ret;

	if(snx_rc_ext.mdrc_en != -1)
		snx_rc_msg.snx_rc_ext.mdrc_en = snx_rc_ext.mdrc_en;
	if(snx_rc_ext.md_recover != -1)
		snx_rc_msg.snx_rc_ext.md_recover = snx_rc_ext.md_recover;
	if(snx_rc_ext.md_m != -1)
		snx_rc_msg.snx_rc_ext.md_m = snx_rc_ext.md_m;
	if(snx_rc_ext.md_n != -1)
		snx_rc_msg.snx_rc_ext.md_n = snx_rc_ext.md_n;
	if(snx_rc_ext.md_th != -1)
		snx_rc_msg.snx_rc_ext.md_th= snx_rc_ext.md_th;
	if(snx_rc_ext.md_can_add_bitrate != -1)
		snx_rc_msg.snx_rc_ext.md_can_add_bitrate = snx_rc_ext.md_can_add_bitrate;
	if(snx_rc_ext.md_max_fps != -1)
		snx_rc_msg.snx_rc_ext.md_max_fps = snx_rc_ext.md_max_fps;
	if(snx_rc_ext.md_isp_nr != -1)
		snx_rc_msg.snx_rc_ext.md_isp_nr = snx_rc_ext.md_isp_nr;
	if(snx_rc_ext.md_2dnr != -1)
		snx_rc_msg.snx_rc_ext.md_2dnr = snx_rc_ext.md_2dnr;
	if(snx_rc_ext.rc_up != -1)
		snx_rc_msg.snx_rc_ext.rc_up = snx_rc_ext.rc_up;
	if(snx_rc_ext.rc_rate != -1)
		snx_rc_msg.snx_rc_ext.rc_rate = snx_rc_ext.rc_rate;

	if(snx_rc_ext.Iframe_UpBound != -1)
		snx_rc_msg.snx_rc_ext.Iframe_UpBound = snx_rc_ext.Iframe_UpBound;

	if(snx_rc_ext.md_cnt_en != -1)
		snx_rc_msg.snx_rc_ext.md_cnt_en = snx_rc_ext.md_cnt_en;

	if(snx_rc_ext.md_cnt_th != -1)
		snx_rc_msg.snx_rc_ext.md_cnt_th = snx_rc_ext.md_cnt_th;

	if(snx_rc_ext.md_cnt_sum_th != -1)
		snx_rc_msg.snx_rc_ext.md_cnt_sum_th = snx_rc_ext.md_cnt_sum_th;

	if(snx_rc_ext.md_cnt_bps != -1)
		snx_rc_msg.snx_rc_ext.md_cnt_bps = snx_rc_ext.md_cnt_bps;

	if(snx_rc_ext.md_cnt_bps2 != -1)
		snx_rc_msg.snx_rc_ext.md_cnt_bps2 = snx_rc_ext.md_cnt_bps2;

	if(snx_rc_ext.md_cnt_gop_multiple != -1)
		snx_rc_msg.snx_rc_ext.md_cnt_gop_multiple = snx_rc_ext.md_cnt_gop_multiple;

	if(snx_rc_ext.md_cnt_count!= -1)
		snx_rc_msg.snx_rc_ext.md_cnt_count = snx_rc_ext.md_cnt_count;

	if(snx_rc_ext.md_cnt_lowbound!= -1)
		snx_rc_msg.snx_rc_ext.md_cnt_lowbound = snx_rc_ext.md_cnt_lowbound;
	if(snx_rc_ext.md_cnt_qp!= -1)
		snx_rc_msg.snx_rc_ext.md_cnt_qp = snx_rc_ext.md_cnt_qp;

	if(snx_rc_ext.md_cnt_absy!= -1)
		snx_rc_msg.snx_rc_ext.md_cnt_absy = snx_rc_ext.md_cnt_absy;

//	snx_rc_ext_dump(&snx_rc_msg.snx_rc_ext);

	if(isp_req)
		snx_rc_msg.msg_type = SNX_RC_SET_ISP;
	else
		snx_rc_msg.msg_type = SNX_RC_SET_EXT;

	snx_rc_msg_send(snx_msg_id, snx_rc_msg);

	snx_rc_msg.msg_type = SNX_RC_RESP_EXT;

	snx_rc_msg_recv(snx_msg_id, &snx_rc_msg, NULL, SNX_RC_RESP_EXT);

	snx_rc_ext_dump(&snx_rc_msg.snx_rc_ext);
	
	snx_rc_ext_set(&snx_rc_msg.snx_rc_ext);

	return ret;
}
