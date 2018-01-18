/* 
  A Minimal Capture Program

  This program opens an audio interface for capture, configures it for
  stereo, 16 bit, 44.1kHz, interleaved conventional read/write
  access. Then its reads a chunk of random data from it, and exits. It
  isn't meant to be a real program.

  From on Paul David's tutorial : http://equalarea.com/paul/alsa-audio.html

  Fixes rate and buffer problems

  sudo apt-get install libasound2-dev
  gcc -o alsa-record-example -lasound alsa-record-example.c && ./alsa-record-example hw:0
*/
 
#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <math.h>
#include <netinet/in.h>
#include <openssl/evp.h>
#include <getopt.h>
#include "HF_ToneDetect_LIB.h"

#define INLENGTH	256
#define DetectTrigLen 0//15
#define	OVERLAP		1

struct command_format {
	unsigned char seq:7;
	unsigned char endflag:1;
	unsigned char len:5;
	unsigned char set:3;
	unsigned char data[26];
	unsigned short checksum;
};
unsigned char rec_flag[40];
unsigned char rec_len[40];
int curr_set = 0;

int aes_cbc_decrypt(unsigned char* in, int inl, unsigned char *out, unsigned char *key, unsigned char *iv)
{
        int len = 0;
        int outl = 0;

	EVP_CIPHER_CTX ctx;

	EVP_CIPHER_CTX_init(&ctx);

	EVP_DecryptInit_ex(&ctx, EVP_aes_128_cbc(), NULL, (unsigned char *)key, iv);

	EVP_DecryptUpdate(&ctx, out+len, &outl, in+len, inl);
	len += outl;

	EVP_DecryptFinal_ex(&ctx, out+len, &outl);
	len+=outl;

	if (len % 16 == 0) {
		len = len - out[inl - 1];
	}
	out[len]=0;

	EVP_CIPHER_CTX_cleanup(&ctx);

	return len;	//return len
}

/*
 * Return 0 is correct, 1 is checksum error
 */
int checksum(unsigned short *addr, int count, unsigned short cksum)
{
	unsigned long sum = 0;
	unsigned int overflow = 0;
	int ret = 0;
	int k = 0;

	while (count > 1) {
		sum += ntohs(*addr);
		count -= 2;
		addr++;
	}

	if (count) {
		sum += ntohs(*(unsigned char *)addr);
	}

	//while (sum >> 16)
	overflow = (sum >> 16) & 0xffff;
	sum = (sum & 0xffff) + overflow;

	sum = (~sum & 0xffff);

	if (sum == cksum) {
		ret = 0;
	} else {
		ret = 1;
		printf("Checksum error!!!!!\n");
	}
	return ret;
}

int compose_data(unsigned char *block)
{
	struct command_format *cmd;
	int ret = 0;

	cmd = (struct command_format *)block;

#ifdef TD_DEBUG
	printf("Debug compose data\n");
	printf("flag = 0x%x\n", cmd->endflag);
	printf("seq = 0x%x\n", cmd->seq);
	printf("set = 0x%x\n", cmd->set);
	printf("len = %d\n", cmd->len);
	printf("checksum = 0x%x\n", ntohs(cmd->checksum));
#endif
	ret = checksum((unsigned short *)cmd->data, (int)cmd->len, ntohs(cmd->checksum));

	return ret;
}

void show_help(void)
{
	printf("Help information\n");

	return;
}

void show_version(void)
{
	// Show version
	printf("Version 1.16 draft test tool\n");

	return;
}

int main (int argc, char *argv[])
{
	int i = 0, j = 0, k = 0;
	int err;
	char *buffer;
	int buffer_frames = 128;
	unsigned int rate = 48000;
	snd_pcm_t *capture_handle;
	snd_pcm_hw_params_t *hw_params;
	snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
	FILE *faudio;
	FILE *flog;
	FILE *fwifi;
	char head[44];	// Porting start tag
	char SN[50] = "1111111111111111";
	short data[INLENGTH];
	long CommandVal;
	short CmdFrmCnt;
	int cnt0 = 0;
	unsigned char decode_data[30];

	int enc_len = 0, dec_len = 0;
	int ret = 0;
	int recv_cnt = 1;
	char *adev = "hw:0";
	int option = 0;
	char rec_name[32];
	char log_name[32];
	const char *short_option = "hvr:l:";
	const struct option long_options[] = {
			{"help", no_argument, NULL, 'h'},
			{"version", no_argument, NULL, 'v'},
			{"record", required_argument, NULL, 'r'},
			{"log", required_argument, NULL, 'l'},
			{NULL, 0, NULL, 0}
	};

	while (1) {
		option = getopt_long(argc, argv, short_option, long_options, 0);

		if (option == -1)
			break;

		//printf("optine is %d\n", option);
		switch (option) {
		case 'h':
			show_help();
			break;
		case 'v':
			show_version();
			break;
		case 'r':
			printf("record file = %s len = %d\n", optarg, strlen(optarg));
			if (strlen(optarg) < sizeof(rec_name) && strlen(optarg) > 0) {
				memset(rec_name, 0, sizeof(rec_name));
				strncpy(rec_name, optarg, strlen(optarg));
			} else {
				printf("Error!!! file name too long or zero....\n");
				goto exit;
			}
			break;
		case 'l':
			printf("log file = %s len = %d\n", optarg, strlen(optarg));
			memset(log_name, 0, sizeof(log_name));
			strncpy(log_name, optarg, strlen(optarg));
			break;

			break;
		default:
			show_help();
			break;
		}
	}

	faudio = fopen(rec_name, "wb");
	flog = fopen(log_name, "w");
	fprintf(flog, "==== Audio Tone Detection Report ====\n");

	if ((err = snd_pcm_open (&capture_handle, adev, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
		printf ("cannot open audio device %s (%s)\n",
				argv[1], snd_strerror (err));
		exit (1);
	}
 
	printf("audio interface opened\n");
		   
	if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
		printf ("cannot allocate hardware parameter structure (%s)\n",
				snd_strerror (err));
		exit (1);
	}

	printf("hw_params allocated\n");
				 
	if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) {
		printf ("cannot initialize hardware parameter structure (%s)\n",
				snd_strerror (err));
		exit (1);
	}
 
	printf("hw_params initialized\n");
	
	if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		printf ("cannot set access type (%s)\n",
				snd_strerror (err));
		exit (1);
	}
 
	printf("hw_params access setted\n");
	
	if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, format)) < 0) {
		printf ("cannot set sample format (%s)\n",
				snd_strerror (err));
		exit (1);
	}
 
	printf("hw_params format setted\n");
	
	if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &rate, 0)) < 0) {
		printf ("cannot set sample rate (%s)\n",
				snd_strerror (err));
		exit (1);
	}
	
	printf("hw_params rate setted\n");
 
	if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, 1)) < 0) {
		printf ("cannot set channel count (%s)\n",
				snd_strerror (err));
		exit (1);
	}
 
	fprintf(stdout, "hw_params channels setted\n");
	
	if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) {
		printf ("cannot set parameters (%s)\n",
				snd_strerror (err));
		exit (1);
	}
 
	printf("hw_params setted\n");
	
	snd_pcm_hw_params_free (hw_params);
 
	printf("hw_params freed\n");
	
	if ((err = snd_pcm_prepare (capture_handle)) < 0) {
		printf ("cannot prepare audio interface for use (%s)\n",
				snd_strerror (err));
		exit (1);
	}
 
	printf("audio interface prepared\n");
 
	//printf("alloc size = %d\n", 128 * snd_pcm_format_width(format) / 8 * 1);	/* alloc 256 bytes */
	buffer = (char *)malloc(buffer_frames * snd_pcm_format_width(format) / 8 * 1);
 
	printf("buffer allocated\n");
 
	// Initialization of Detecting Tone Signal
	ToneInit(SN);
	CmdFrmCnt = DetectTrigLen;
	CommandVal = -1;

	printf("==== Starting Recording ====\n");

	//while (1) {
	for (i = 0 ; i < 3750 * 20; ++i) {	/* Record 200 seconds */
		if ((err = snd_pcm_readi (capture_handle, buffer, buffer_frames)) != buffer_frames) {
			fprintf (stderr, "read from audio interface failed (%s)\n",
					err, snd_strerror (err));
			exit (1);
		}

		fwrite(buffer, 1, (INLENGTH), faudio);

		/* Porting tag */
		if (CommandVal == -1 && CmdFrmCnt >= DetectTrigLen)
			CommandVal = ToneDetect((short *)buffer, INLENGTH, decode_data);
			//TDTrigFlag = ToneDetectCore(data,INLENGTH,CommandVal);

		if (CommandVal != -1) {

			// Display Command of Tone Signal
			//printf("(Command Value	%03d)\n",CommandVal);

#ifdef TD_DEBUG
			printf("\nDecode Data Checking(Hex) :\n");
                        for (j = 0 ; j < 30 ; j++) {
                                if (j > 7 && j % 8 ==0)
                                        printf("\n");
                                printf("0x%02x ", decode_data[j]);
                        }
                        printf("\n");
#endif
                        /* Combination Data */
                        ret = compose_data(decode_data);

                	if (ret == 0) { /* checksum is correct */
                		printf("PASS\n");
                		fprintf(flog, "Counter %d : Pass\n", recv_cnt);
                	} else {
                		printf("No PASS\n");
                		fprintf(flog, "Counter %d : Fail!!!\n", recv_cnt);
                	}

			ToneInit(SN);
			CmdFrmCnt = DetectTrigLen;
			CommandVal = -1;

			recv_cnt++;
		}

		/* check rec_flag array */
		if (decode_data[0] & 0x80) {
			printf("The last packet\n");
			break;
		}
	}
 
out:
	free(buffer);
 
//	fprintf(stdout, "buffer freed\n");
	
	snd_pcm_close (capture_handle);
//	fprintf(stdout, "audio interface closed\n");

	if (faudio)
		fclose(faudio);

	if (flog)
		fclose(flog);


exit:
	return 0;
}
