/**
 *
 * SONiX SDK Example Code
 * Category: Video Encode Data Stamp
 * File: snx_vc_ds.c
 * Usage: 
 *		1. Make sure at least a stream has been run (./snx_vc_ds)
 *		2. Input the correct dev name of the stream (refer to the usage )
 *		3. Input the rest of arguments you want to setup (refer to the usage)
 *		4. Example: ./snx_vc_ds -d 1_h -x 68 -y 40 -l 10 -v 4 -s 0 -e 1 -t R -b W -w 4 -m 0 -f /media/mmcblk0/logo160x64.bmp 
 *		5. [ST58660]
			./snx_vc_ds -d 1_h -x 68 -y 40 -s 0 -e 1 -w 4 -m 0 -T 0 -f /media/mmcblk0/logo160x64.bmp		(datastamp from bmp file)
 *			./snx_vc_ds -i SONIX -d 1_h -e 1 -m 1 -w 4 -s 0 -x 0 -y 0 -t R -b W -T 0 -p xxx.font	(using outside font table)
 * NOTE:
 *       Recording all streams to SD card would cause the target framerate can
 *       not be reached because of the bandwidth leakage of SD card.
 */
#include <stdint.h>
#include "snx_video_codec.h"
#include "generated/snx_sdk_conf.h"

/*-----------------------------------------------------------------------------
 * Example Code Configuration
 *----------------------------------------------------------------------------*/

/*----------------- Data Stamp config   --------------------------------------*/
#define DS_ENABLE		1					// Data Stamp Enable
#define DS_TEXT_COLOR	DS_BLUE				// Data Stamp Text Color
#define DS_BG_COLOR		DS_YELLOW			// Data Stamp Background Color
#define DS_MODE			2					// Data stamp mode [0~3]
											// 0: text(wighting) + background(transparent)
											// 1: text(wighting) + background
											// 2: text           + background(transparent)
											// 3: text           + background
											
#define DS_WEIGHT		0					// TEXT transparent weight [1~7] Transparent ~ Solid
#define DS_SCALE		0					// [0~2] 0: 1x1 1: 2x2 2: 4x4 Data Stamp Scale 
#define DS_STARTX		0					// Start posision of Data stamp
#define DS_STARTY		0					// Start posision of Data stamp
#define DS_STRING		"Hello World"		// Data Stamp String Data


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
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
static const char short_options[] = "hd:t:b:m:w:i:s:x:y:l:v:e:f:S:r:o:T:p:H:R:";
#else
static const char short_options[] = "hd:t:b:m:w:i:s:x:y:l:v:e:f:S:r:o:";
#endif

static const struct option long_options[] = {
    {"help", no_argument, NULL, 'h'},
    {"device", required_argument, NULL, 'd'},
    {"text", required_argument, NULL, 't'},
    {"background", required_argument, NULL, 'b'},
    {"mode", required_argument, NULL, 'm'},
    {"weighting", required_argument, NULL, 'w'},
    {"string", required_argument, NULL, 'i'},
    {"scale", required_argument, NULL, 's'},
    {"start_x", required_argument, NULL, 'x'},
    {"start_y", required_argument, NULL, 'y'},
    {"dim_h", required_argument, NULL, 'l'},
    {"dim_v", required_argument, NULL, 'v'},
    {"enable", required_argument, NULL, 'e'},
    {"bmp_file", required_argument, NULL, 'f'},
    {"bmp_scale", required_argument, NULL, 'S'},
    {"show_font", required_argument, NULL, 'r'},
    {"font_table", required_argument, NULL, 'o'},
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
    {"transparent_mode", required_argument, NULL, 'T'},
	{"font_table path", required_argument, NULL, 'p'},
	{"bmp background threshold", required_argument, NULL, 'H'},
	{"bmp_rounding", required_argument, NULL, 'R'},
#endif
    {0, 0, 0, 0}
};


static void usage(FILE * fp, int argc, char ** argv) {   
    fprintf(fp, "Usage: %s [options]/n\n"   
        "Options:\n"
        "-h Print this message\n"
        "-d | --device	[1_hs] select on /proc/codec/* \n"
        "		       	[1] means /dev/video1\n"
        "				[2] means /dev/video2\n"
        "				  [h] means H.264\n"
		"				   [s] scaling stream\n"
        "-t | --text	     text color,       R:Red    G:Green   B:Blue\n"
	"-b | --background   background color, Y:Yellow M:Magenta C:Cyan\n"
	"				       W:White  K:Black\n"
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
	"-m | --mode         0: Weighting mode for text color"
	"					 1: Use direct text color mode"
#else
	"-m | --mode         0: text(wighting) + background(transparent)\n"
	"                    1: text(wighting) + background\n"
	"                    2: text           + background(transparent)\n"
	"                    3: text           + background\n"
#endif
	"-w | --weighting    OUT=(DS_WT*TEXT+(8-DS_WT)*IMG)/8 \n"
	"-i | --string       input string\n"
	"-f | --bmp_file     input bitmap file\n"
	"-s | --scale        scale 0: 1x1 1: 2x2 2: 4x4\n"
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
	"-x | --start_x      postiton unit is 1 pixel\n"
	"-y | --start_y      postiton unit is 1 pixel\n"
#else
	"-S | --bmp_scale	 scale down/up bmp picture 0: 1x1 1; (1/2)x(1/2) 2: 2x2"
	"-x | --start_x      postiton unit is 16 pixel\n"
	"-y | --start_y      postiton unit is 16 pixel\n"
#endif
	"-l | --dim_h        dimension horizontal\n"
	"-v | --dim_v	     dimension vertical\n"
	"-o | --font_table   Used font table\n"
	"-e | --enable	     Codec Data Stamp enable\n"
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)	
	"-T | --transparent_mode	transparent mode\n"
	"-H | --bmp_threshold		bmp background threshold\n"
	"-R | --bmp_rounding		bmp 888 to 332 rounding\n"
#endif
	"", argv[0]);   
}

int main(int argc, char **argv)
{
	int ret = 0;
	struct snx_cds *cds = NULL;
	cds = malloc(sizeof(struct snx_cds));
	struct snx_cds_color *cds_color;
	int bitmap_flag = 0, bitmap_scale = 0, font_table=0;
	
	/*--------------------------------------------------------
		Data Stamp Setup (Default)
	---------------------------------------------------------*/
	memset(cds ,0x0, sizeof(struct snx_cds));

	/*--------------------------------------------------------
		Option Value
	---------------------------------------------------------*/

	cds_color = &cds->bmp_threshold;
	cds_color->color_Y = 190;
	cds_color->color_Cb = 190;
	cds_color->color_Cr = 190;
	
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
			case 'f':
				sprintf(cds->bmp_file, "%s", optarg);
				bitmap_flag = 1;
				printf("[CDS] get bmp file: %s\n", cds->bmp_file);
				break;
			case 'd':
				if (strlen(optarg) != 0) {
					sprintf(cds->dev_name, "/proc/codec/%s", optarg);
					if(cds->dev_name[14] != 'h'){
						printf("please specify %c_h, like ./snx_xxx -d %c_h ....\n", cds->dev_name[12], cds->dev_name[12]);
						exit(EXIT_FAILURE);
					}
					snx_vc_data_stamp(DS_GET_ALL, cds);
				}
				break;
			case 'e':
				sscanf(optarg, "%d", &cds->enable);				
				break;
			case 't':
			case 'b':
				if( c == 't')
					cds_color = &cds->t_color;
				else
					cds_color = &cds->b_color;
				
				switch(*optarg) {
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
					case 'R': 
						sscanf(DS_RED, "%u %u %u", &cds_color->color_R, &cds_color->color_G, &cds_color->color_B);
						break;
					case 'G': 
						sscanf(DS_GREEN, "%u %u %u", &cds_color->color_R, &cds_color->color_G, &cds_color->color_B);
						break;
					case 'B': 
						sscanf(DS_BLUE, "%u %u %u", &cds_color->color_R, &cds_color->color_G, &cds_color->color_B);
						break;
					case 'Y': 
						sscanf(DS_YELLOW, "%u %u %u", &cds_color->color_R, &cds_color->color_G, &cds_color->color_B);
						break;
					case 'M': 
						sscanf(DS_MAGENTA, "%u %u %u", &cds_color->color_R, &cds_color->color_G, &cds_color->color_B);
						break;
					case 'C': 
						sscanf(DS_CYAN, "%u %u %u", &cds_color->color_R, &cds_color->color_G, &cds_color->color_B);
						break;
					case 'K': 
						sscanf(DS_BLACK, "%u %u %u", &cds_color->color_R, &cds_color->color_G, &cds_color->color_B);
						break;
					case 'W': 
						sscanf(DS_WHITE, "%u %u %u", &cds_color->color_R, &cds_color->color_G, &cds_color->color_B);
						break;
#else
					case 'R': 
						sscanf(DS_RED, "%u %u %u", &cds_color->color_Y, &cds_color->color_Cb, &cds_color->color_Cr);
						break;
					case 'G': 
						sscanf(DS_GREEN, "%u %u %u", &cds_color->color_Y, &cds_color->color_Cb, &cds_color->color_Cr);
						break;
					case 'B': 
						sscanf(DS_BLUE, "%u %u %u", &cds_color->color_Y, &cds_color->color_Cb, &cds_color->color_Cr);
						break;
					case 'Y': 
						sscanf(DS_YELLOW, "%u %u %u", &cds_color->color_Y, &cds_color->color_Cb, &cds_color->color_Cr);
						break;
					case 'M': 
						sscanf(DS_MAGENTA, "%u %u %u", &cds_color->color_Y, &cds_color->color_Cb, &cds_color->color_Cr);
						break;
					case 'C': 
						sscanf(DS_CYAN, "%u %u %u", &cds_color->color_Y, &cds_color->color_Cb, &cds_color->color_Cr);
						break;
					case 'K': 
						sscanf(DS_BLACK, "%u %u %u", &cds_color->color_Y, &cds_color->color_Cb, &cds_color->color_Cr);
						break;
					case 'W': 
						sscanf(DS_WHITE, "%u %u %u", &cds_color->color_Y, &cds_color->color_Cb, &cds_color->color_Cr);
						break;
#endif
					default: 
						usage(stderr, argc, argv);   
						exit(EXIT_FAILURE);   
				}
				break;
			case 'm':
				sscanf(optarg, "%d", &cds->attr.mode);
				break;
			case 'w':
				sscanf(optarg, "%d", &cds->attr.weight);
				break;
			case 's':
				sscanf(optarg, "%d", &cds->scale);
//				snx_vc_data_stamp(DS_SET_SCALE, cds);
				break;
			case 'i':
				cds->string = malloc(strlen(optarg)+1);
				strcpy(cds->string, optarg);
                printf("[CDS] get string: %s\n", cds->string);
				break;
			case 'x':
				sscanf(optarg, "%d", &cds->pos.start_x);
				break;
			case 'y':
				sscanf(optarg, "%d", &cds->pos.start_y);
				break;
			case 'l':
				sscanf(optarg, "%d", &cds->dim.dim_x);
				break;
			case 'v':
				sscanf(optarg, "%d", &cds->dim.dim_y);
				break;
			case 'r':
				sscanf(optarg, "%d", &cds->show_font);
				break;			
			case 'S':
				bitmap_scale = atoi(optarg);
				break;
			case 'o':
				font_table = atoi(optarg);
				break;
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
			case 'T':
				cds->transparent_mode = atoi(optarg);
				break;
			case 'p':
				sscanf(optarg, "%s", cds->font_table_path);
				break;
			case 'H':
				cds->bmp_threshold.color_R = atoi(optarg);
				cds->bmp_threshold.color_G = atoi(optarg);
				cds->bmp_threshold.color_B = atoi(optarg);
				break;
			case 'R':
				cds->bmp_rounding = atoi(optarg);
				break;
#endif

			default:
				usage(stderr, argc, argv);
				exit(EXIT_FAILURE);
		}
	}

	if(cds->enable == 0){
		snx_vc_data_stamp(DS_SET_EN, cds);
		return 0;
	}

	if(cds->string || bitmap_flag == 1) {
		if(bitmap_flag && (cds->string == NULL)) {
			char tmp[64];
			char *pch;
			char delim[32] = "/proc/codec/";
			pch = strtok(cds->dev_name, delim);

			if(bitmap_scale == 1)
			{
				sprintf(tmp, "/tmp/%s_1.tmp", pch);
				snx_scale_down_bmppicture (cds->bmp_file, tmp);
				sprintf(cds->bmp_file, tmp);
			}
			else if(bitmap_scale == 2)
			{
				sprintf(tmp, "/tmp/%s_2.tmp", pch);
				snx_scale_up_bmppicture(cds->bmp_file, tmp);
				sprintf(cds->bmp_file, tmp);
			}
			else if(bitmap_scale == 3)
			{
				char tmp2[64];
				sprintf(tmp, "/tmp/%s_3.tmp", pch);
				sprintf(tmp2, "/tmp/%s_4.tmp", pch);
				snx_scale_down_bmppicture (cds->bmp_file, tmp);
				snx_scale_up_bmppicture(tmp, tmp2);
				sprintf(cds->bmp_file, tmp2);
			}
			
			//printf("[DBG] bmp data stamp\r\n");
			snx_vc_data_stamp(DS_SET_BMP, cds);
		} else if (!bitmap_flag){
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
			snx_vc_data_stamp(DS_SET_FONT_STRING, cds);
#else
			//printf("[DBG] string data stamp\r\n");
			if(font_table == 0)
				snx_vc_data_stamp(DS_SET_STRING, cds);
			else //if want use font table
				snx_vc_data_stamp(DS_SET_FONT_STRING, cds);
#endif
		}
	}
	/* By default, we use the length * 1 */
	if ((cds->string) && ((cds->dim.dim_x == 0) || (cds->dim.dim_y == 0))) {
		cds->dim.dim_x = strlen(cds->string);
		cds->dim.dim_y = 1;
	}
	
    if (cds->attr.weight == 0 )
        cds->attr.weight = 1; //default weight value

	snx_vc_data_stamp(DS_SET_SCALE, cds);
	snx_vc_data_stamp(DS_SET_POS, cds);
	snx_vc_data_stamp(DS_SET_COLOR_ATTR, cds);
	snx_vc_data_stamp(DS_SET_COLOR, cds);
	snx_vc_data_stamp(DS_SET_EN, cds);

	/*--------------------------------------------------------
		Record End 
	---------------------------------------------------------*/
	/* Free the stream configs */
	if(cds->string)
		free(cds->string);
	free(cds);
	
    return ret;
}
