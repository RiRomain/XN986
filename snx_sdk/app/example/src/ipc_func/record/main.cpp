#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <signal.h>
#include <getopt.h>             /* getopt_long() */
#include "avhandler.h"
#include "util.h"
#include "sn98600_ctrl.h"
#include "sn98600_v4l2.h"
#include "save_media.h"

extern int sd_record_en;

#if TIMELAPSE_SUPPORT
extern int timelapse;
#endif

#if TIMELAPSE_SUPPORT
static const char short_options[] = "hmcb:i:s:ad:g:t:";
#else
static const char short_options[] = "hmcb:i:s:ad:g:";
#endif

static const struct option long_options[] = {
    {"help", no_argument, NULL, 'h'},
    {"m2m", no_argument, NULL, 'm'},
    {"capture", no_argument, NULL, 'c'},
	{"bitrate", required_argument, NULL, 'b'},
	{"ispfps", required_argument, NULL, 'i'},
	{"schedule", required_argument, NULL, 's'},
	{"alarm", no_argument, NULL, 'a'},
	{"device", required_argument, NULL, 'd'},
	{"gop", required_argument, NULL, 'g'},
#if TIMELAPSE_SUPPORT
	{"timelapse", required_argument, NULL, 't'},
#endif
    {0, 0, 0, 0}
};

static void usage(FILE * fp, int argc, char ** argv) {   
    fprintf(fp, "Usage: %s [options]/n\n"   
		"Options:\n"
		"-h                 Print this message\n"
        "-m | --m2m         m2m path enable (default)\n"
        "-c | --capture		capture path enable (exclusive with m2m)\n"
        "-b | --bitrate		Video recording bitrate (Kbps) (Default 1024)\n"
        "-s | --schedule	schedule recording enable (Dis: 0/ En: 1 Default is 1)\n"
        "-a | --alarm		alarm recording enable \n"
        "-d | --device		Video recording device (1 / 2, Default 1)\n"
        "-g | --gop			gop (Default = videofps)\n"
#if TIMELAPSE_SUPPORT
        "-t | --timelapse	enable timelapse and set intervals (Default is 0)\n"
#endif
        "Ex:   %s -m -b 768 \n"
		"\n"

		"", argv[0],argv[0]);   
}

static void sigstop(int arg)
{
	set_Terminate_sig();
	close_avhandler();
	exit(0);
}



int main(int argc,char *argv[])
{
	int rs = -1;
	int bit_rate, ispfps, device_num, schedule_opt, timelapse_opt, gop;
	snx_av_conf_t snx_av_conf;
	memset(&snx_av_conf, 0x0, sizeof(snx_av_conf_t));
	
	snx_av_conf.m2m_en = 1;
	snx_av_conf.bitrate = 1024;
	snx_av_conf.videofps = 15;
	snx_av_conf.ispfps = 30;
	snx_av_conf.gop = 0;
	snx_av_conf.videores = RESOLUTION_HD;

	snx_av_conf.codec_dev = 1;  // Default is /dev/video1 

#if TIMELAPSE_SUPPORT
	timelapse = 0;  			// Default timelapse is disable  
#endif

	sd_record_en = 1; 			// Default Schedule recording is enabled
	/*--------------------------------------------------------
		Option Value
	---------------------------------------------------------*/
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
			case 'm':
				snx_av_conf.m2m_en = 1; //default value
				break;
			case 'c':
				snx_av_conf.m2m_en = 0;
				break;
			case 'a':
				snx_av_conf.sd_alarm_record_en = 1;
				break;
			case 'b':
				bit_rate = atoi(optarg);

				fprintf(stderr, "bitrate = %d \n", bit_rate);

				snx_av_conf.bitrate = bit_rate;
             	
				if(snx_av_conf.bitrate < 1 || snx_av_conf.bitrate > 10240)
					snx_av_conf.bitrate = 1024; //default 1Mbps

				
				break;
			case 's':
				schedule_opt = atoi(optarg);

				fprintf(stderr, "schedule recording = %d \n", schedule_opt);

				sd_record_en = schedule_opt;
             	
				if(sd_record_en < 0 || sd_record_en > 1)
					sd_record_en = 1; //default 1

				break;
			case 'i':
				ispfps = atoi(optarg);

				fprintf(stderr, "ispfps = %d \n", ispfps);

				snx_av_conf.ispfps = ispfps;
             	
				if(snx_av_conf.ispfps < 1 || snx_av_conf.ispfps > 30)
					snx_av_conf.ispfps = 30; //default 30

				break;
			case 'd':
				device_num = atoi(optarg);

				fprintf(stderr, "device_num = %d \n", device_num);

				snx_av_conf.codec_dev = device_num;
             	
				if(snx_av_conf.codec_dev < 1 || snx_av_conf.codec_dev > 2)
					snx_av_conf.codec_dev = 1; //default 1

				break;
			case 'g':
				gop = atoi(optarg);

				fprintf(stderr, "gop = %d \n", gop);

				snx_av_conf.gop = gop;
             	
				if(snx_av_conf.gop < 1 )
					snx_av_conf.gop = 1; //default 1

				break;
#if TIMELAPSE_SUPPORT
			case 't':
				timelapse_opt = atoi(optarg);

				fprintf(stderr, "timelapse = %d \n", timelapse_opt);

				timelapse = timelapse_opt;
             	
				if(timelapse < 0)
					timelapse = 0; //default 0

				break;
#endif
			default:
				usage(stderr, argc, argv);
				exit(EXIT_SUCCESS);  
		}
	}
#if 1
	{

	char c_videofps[128];
	char c_videores[128];
	xml_doc video_config;

	memset(c_videofps, 0x00, sizeof(c_videofps));
	memset(c_videores, 0x00, sizeof(c_videores));

	struct stat tst;

	if (stat(RECORD_CONFIG, &tst) == -1) 
	{

		printf("[SDRECORD] NO records.xml is found\n");
		return 0;
	}
	/* ONLY CHECK ID1 config for video setup */
	read_config(RECORD_CONFIG,video_config, 1, "videofps", c_videofps);
	read_config(RECORD_CONFIG,video_config, 1, "videores", c_videores);

	if (strlen(c_videofps) != 0)
		snx_av_conf.videofps = atoi(c_videofps);
	if ((snx_av_conf.videofps < 1) || (snx_av_conf.videofps > 30) )
		snx_av_conf.videofps = 15;

	if (!strcmp(c_videores, "1280x720"))
		snx_av_conf.videores = RESOLUTION_HD;
	else if ((!strcmp(c_videores, "640x480")) || (!strcmp(c_videores, "640x360")))
		snx_av_conf.videores = RESOLUTION_VGA_264;
	else
		snx_av_conf.videores = RESOLUTION_HD;

	}

	if (snx_av_conf.gop == 0)
		snx_av_conf.gop = snx_av_conf.videofps;

#endif

	/* Register sig function */
	signal(SIGINT,	sigstop);
	signal(SIGTERM, sigstop);

	rs = open_avhandler(&snx_av_conf);
	if(rs != 0)
		goto finally;

	while(1)
	{

/*! To do : this part will be reprogramming for much more flexible */
		sleep(2);		
	}

finally:
	sleep(1);
	close_avhandler();

/*! To do end */
	return 0;
	
}
