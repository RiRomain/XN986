/**
 *
 * SONiX SDK Example Code
 * Category: Rate Control, Manual Region-of-Interest,
 * File: snx_mroi.c
 * Usage: 
 *	 		1. Make sure at least a stream has been run (./snx_m2m_one_stream)
 *			2. Input the correct dev name of the stream (refer to the usage )
 *			3. Input the rest of arguments you want to setup (refer to the usage)
 *			4. Example: ./snx_mroi -d 1_h -n 3 -w 5 -q 3 -x 0 -y 0 -l 320 -v 80 -e 1
 * NOTE:
 *       
 *       
 */
#include <stdint.h>
#include "snx_video_codec.h"

#define FRAME_WIDTH		1280
#define FRAME_HEIGHT	720

/*
	The entrance of this file.
*/

static const char short_options[] = "hd:n:w:q:w:s:x:y:l:v:e:";

static const struct option long_options[] = {
    {"help", no_argument, NULL, 'h'},
    {"device", required_argument, NULL, 'd'},
    {"region_num", required_argument, NULL, 'n'},
    {"weighting", required_argument, NULL, 'w'},
    {"qp", required_argument, NULL, 'q'},
    {"ext_size", required_argument, NULL, 's'},
    {"start_x", required_argument, NULL, 'x'},
    {"start_y", required_argument, NULL, 'y'},
    {"dim_h", required_argument, NULL, 'l'},
    {"dim_v", required_argument, NULL, 'v'},
    {"enable", required_argument, NULL, 'e'},
    {0, 0, 0, 0}
};


static void usage(FILE * fp, int argc, char ** argv) {   
    fprintf(fp, "Usage: %s [options]/n\n"   
        "Options:\n"
        "-h Print this message\n"
        "-d | --device	[1_xs] select on /proc/codec/* \n"
        "					[1] means /dev/video1\n"
        "					[2] means /dev/video2\n"
        "					[x] can have two options\n"
        "		            	[h] for H264\n"
        "		            	[j] for MJPEG\n"
		"					[s] scaling stream\n"
        "-n | --region_num   \n"
		"-w | --weighting    \n"
		"-q | --qp           \n"
		"-s | --ext_size     \n"
		"-x | --start_x      X-axis start position, unit is 1 pixel\n"
		"-y | --start_y      Y-axis start position, unit is 1 pixel\n"
		"-l | --dim_h        X-axis dimension\n"
		"-v | --dim_v	     Y-axis dimension\n"
		"-e | --enable	     Codec Data Stamp enable\n"

	"", argv[0]);   
}

int main(int argc, char **argv)
{
	int ret = 0;

	struct snx_m2m *m2m = NULL;
	struct snx_rc *rc = NULL;
	int enable=0, qp=0, num=0, weight=0, ext_size=0;
	int pos_x=0, pos_y=0, dim_x=0, dim_y=0;
	int i;

	m2m = malloc(sizeof(struct snx_m2m));
	memset(m2m, 0x0, sizeof(struct snx_m2m));

	rc = malloc(sizeof(struct snx_rc));
	memset(rc, 0x0, sizeof(struct snx_rc));

	
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
			case 'd':
				if (strlen(optarg) != 0) {
					sprintf(m2m->ds_dev_name, "%s", optarg);
				}
				break;
			case 'e':
				sscanf(optarg, "%d", &enable);
				break;
			case 'n':
				sscanf(optarg, "%d", &num);
				break;
			case 'w':
				sscanf(optarg, "%d", &weight);
				break;
			case 'q':
				sscanf(optarg, "%d", &qp);
				break;
			case 's':
				sscanf(optarg, "%d", &ext_size);
				break;
			case 'x':
				sscanf(optarg, "%d", &pos_x);
				break;
			case 'y':
				sscanf(optarg, "%d", &pos_y);
				break;
			case 'l':
				sscanf(optarg, "%d", &dim_x);
				break;
			case 'v':
				sscanf(optarg, "%d", &dim_y);
				break;

			default:
				usage(stderr, argc, argv);
				exit(EXIT_FAILURE);
		}
	}
	
	snx_codec_mroi_enable(m2m, 0);	//need to disable before set region attr.

	rc->width = FRAME_WIDTH;		//need frame width and height to avoid wrong setting of MROI.
	rc->height = FRAME_HEIGHT;
	rc->mroi_region[num].weight = weight;
	rc->mroi_region[num].qp = qp;
	rc->mroi_region[num].pos_x = pos_x;
	rc->mroi_region[num].pos_y = pos_y;
	rc->mroi_region[num].dim_x = dim_x;
	rc->mroi_region[num].dim_y = dim_y;
	rc->mroi_region[num].ext_size = EXT_SIZE_MEDIUM;

	snx_codec_mroi_set_region_attr(m2m, rc, num);

	snx_codec_mroi_enable(m2m, enable);

	/*--------------------------------------------------------
	---------------------------------------------------------*/
	/* Free the stream configs */

	free(rc);
	free(m2m);
	
    return ret;
}
