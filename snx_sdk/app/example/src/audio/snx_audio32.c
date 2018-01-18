#include <stdio.h>
#include <stdint.h>
#include <string.h>        
#include <pthread.h>
#include <getopt.h>             /* getopt_long() */
#include <fcntl.h>              /* low-level i/o */
#include <malloc.h>
#include <math.h>
#include "snx_audio_codec/snx_audio_aud32.h"

#define FRAME_SIZE	320 // samples, 16-bits

/* 
	Audio32 supports,
	SNX_AUD32_FMT8_8KBPS
	SNX_AUD32_FMT8_16KBPS
	SNX_AUD32_FMT16_8K_8KBPS
	SNX_AUD32_FMT16_16K_16KBPS
	SNX_AUD32_FMT16_16K_24KBPS
	SNX_AUD32_FMT16_16K_32KBPS
	SNX_AUD32_FMT16_32K_32KBPS
	SNX_AUD32_FMT16_32K_64KBPS
*/ 

#define BIT_RATE	SNX_AUD32_FMT16_16K_16KBPS 

char audio_src[128];
char audio_dest[128];
float decode_gain = 1.0f;

static const char short_options[] = "hm:i:o:s:b:a:g:";

static const struct option long_options[] = {
    {"help", no_argument, NULL, 'h'},
	{"mode", required_argument, NULL, 'm'},
	{"input", required_argument, NULL, 'i'},
	{"output", required_argument, NULL, 'o'},
	{"samplerate", required_argument, NULL, 's'},
	{"bitrate", required_argument, NULL, 'b'},
	{"bit", required_argument, NULL, 'a'},
	{"decodegain", required_argument, NULL, 'g'},
    {0, 0, 0, 0}
};

static void usage(FILE * fp, int argc, char ** argv) {   
    fprintf(fp, "Usage: %s [options]/n\n"   
		"Options:\n"
		"-h                 Print this message\n"
        "-m | --mode		0: record, 1: playback (default: 0)\n"
        "-i | --input		filename \n"
        "-o | --output		filename \n"
        "-s | --samplerate	8 / 16 / 32(khz, default is 32K_32Kbps) \n"
        "-b | --bitrate 	8 / 16 / 24 / 32 / 64 (kbps, default is 32K_32Kbps) \n"
        "-a | --bit 		8 / 16 (bit, default is 16) \n"
        "-g | --decodegain 	decode gain (default is 1) \n"
        "Audio32 format List\n"
        "SNX_AUD32_FMT8_8KBPS\n"
		"SNX_AUD32_FMT8_16KBPS\n"
		"SNX_AUD32_FMT16_8K_8KBPS\n"
		"SNX_AUD32_FMT16_16K_16KBPS\n"
		"SNX_AUD32_FMT16_16K_24KBPS\n"
		"SNX_AUD32_FMT16_16K_32KBPS\n"
		"SNX_AUD32_FMT16_32K_32KBPS\n"
		"SNX_AUD32_FMT16_32K_64KBPS\n"
        "Ex:   %s -m 0 -i /tmp/test.pcm -o /tmp/test.en\n"
		"\n"

		"", argv[0],argv[0]);   
}

int encode (enum SNX_AUD32_FORMAT format)
{
	struct SNX_AUD32_CONTEXT *aud32_enc;
	FILE *fp_in, *fp_out;
	int size, r_size, out_size, f_cnt = 0;
	short in_word[FRAME_SIZE], out_word[FRAME_SIZE];
	
	fp_in = fopen (audio_src, "rb");
	fp_out = fopen (audio_dest, "wb");
	if (!fp_in || !fp_out) {
		printf ("open file fail.\n");
		return -1;
	}

	aud32_enc = snx_aud32_open (SNX_AUD32_TYPE_ENCODER, format);
	if (!aud32_enc) {
		printf ("snx_aud32_open fail.\n");
		return -1;
	}

	printf ("\nencoder start.\n");
	size = fread ((int8_t *)in_word, 1, aud32_enc->pcm_bytes_per_frame,
									fp_in);
	
	while (size == aud32_enc->pcm_bytes_per_frame) {
		f_cnt++;
		out_size = aud32_enc->aud32_bytes_per_frame;
		
		r_size = snx_aud32_encode (aud32_enc,
				(int8_t *)in_word, (int8_t *)out_word,
				size, &out_size);
		if (r_size != size)
			printf ("size not match, r_size = %d, size = %d",
								r_size, size);

		fwrite (out_word, 1, out_size, fp_out);
		
		size = fread ((int8_t *)in_word, 1,
					aud32_enc->pcm_bytes_per_frame, fp_in);
	}
	fclose (fp_in);
	fclose (fp_out);
	snx_aud32_close (aud32_enc);
	
	printf ("\nencoder done.\n");
	return 0;
}

int decode (enum SNX_AUD32_FORMAT format)
{
	struct SNX_AUD32_CONTEXT *aud32_dec;
	FILE *fp_in, *fp_out;
	int size, r_size, out_size, f_cnt = 0;
	short in[FRAME_SIZE+1], out[FRAME_SIZE+1];
	char *in_word = (char *)in, *out_word = (char *)out;

	// decode suport un-alignment buffer ptr
	in_word = ((int)in_word & 1) ? (in_word + 1) : in_word;
	out_word = ((int)out_word & 1) ? (out_word + 1) : out_word;
	
	fp_in = fopen (audio_src, "rb");
	fp_out = fopen (audio_dest, "wb");
	if (!fp_in || !fp_out) {
		printf ("open file fail.\n");
		return -1;
	}

	aud32_dec = snx_aud32_open (SNX_AUD32_TYPE_DECODER, format);
	if (!aud32_dec) {
		printf ("snx_aud32_open fail.\n");
		return -1;
	}

	if (decode_gain > 0)
		snx_aud32_decode_gain(decode_gain);


	printf ("\ndecoder start.\n");
	size = fread ((int8_t *)in_word, 1, aud32_dec->aud32_bytes_per_frame,
									fp_in);
	
	while (size == aud32_dec->aud32_bytes_per_frame) {
		f_cnt++;
		out_size = aud32_dec->pcm_bytes_per_frame;
		
		r_size = snx_aud32_decode (aud32_dec,
				(int8_t *)in_word, (int8_t *)out_word,
				size, &out_size);
		if (r_size != size)
			printf ("size not match, r_size = %d, size = %d",
								r_size, size);

		fwrite (out_word, 1, out_size, fp_out);
		
		size = fread ((int8_t *)in_word, 1,
				aud32_dec->aud32_bytes_per_frame, fp_in);
	}
	fclose (fp_in);
	fclose (fp_out);
	snx_aud32_close (aud32_dec);
	
	printf ("\ndecoder done.\n");
	return 0;
}

int main (int argc, void* argv[])
{
	int mode = 0; //default record;
	int samplerate = 32;
	int bit = 16;
	int bitrate = 32;
	enum SNX_AUD32_FORMAT audio32_format = BIT_RATE;

	memset(audio_dest, 0x0, 128);
	memset(audio_src, 0x0, 128);

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
				return -1;
			case 'm':
				sscanf(optarg, "%d", &mode); 
				break;
			case 'i':
				sscanf(optarg, "%s", &audio_src);
				break;
			case 'o':
				sscanf(optarg, "%s", &audio_dest);
				break;
			case 's':
				sscanf(optarg, "%d", &samplerate); 
				break;
			case 'b':
				sscanf(optarg, "%d", &bitrate); 
				break;
			case 'a':
				sscanf(optarg, "%d", &bit);
				break;
			case 'g':
				sscanf(optarg, "%f", &decode_gain);
				break;
			default:
				usage(stderr, argc, argv);
				return -1;
		}
	}

	switch(bitrate)
	{
		case 8:

			if ((samplerate == 8) && (bit == 8))
				audio32_format = SNX_AUD32_FMT8_8KBPS;
			else if ((samplerate == 8) && (bit == 16))
				audio32_format = SNX_AUD32_FMT16_8K_8KBPS;
			else {
				printf ("unkown audio32 format s%d_%dkhz_%dkbps\n", bit, samplerate, bitrate);
				return 0;
			}
			break;
		case 16:
			if ((samplerate == 8) && (bit == 8))
				audio32_format = SNX_AUD32_FMT8_16KBPS;
			else if ((samplerate == 16) && (bit == 16))
				audio32_format = SNX_AUD32_FMT16_16K_16KBPS;
			else {
				printf ("unkown audio32 format s%d_%dkhz_%dkbps\n", bit, samplerate, bitrate);
				return 0;
			}
			break;
		case 24:
			if ((samplerate == 16) && (bit == 16))
				audio32_format = SNX_AUD32_FMT16_16K_24KBPS;
			else {
				printf ("unkown audio32 format s%d_%dkhz_%dkbps\n", bit, samplerate, bitrate);
				return 0;
			}
			break;
		case 32:
			if ((samplerate == 16) && (bit == 16))
				audio32_format = SNX_AUD32_FMT16_16K_32KBPS;
			else if ((samplerate == 32) && (bit == 16))
				audio32_format = SNX_AUD32_FMT16_32K_32KBPS;
			else {
				printf ("unkown audio32 format s%d_%dkhz_%dkbps\n", bit, samplerate, bitrate);
				return 0;
			}
			break;
		case 48:
			if ((samplerate == 32) && (bit == 16))
				audio32_format = SNX_AUD32_FMT16_32K_48KBPS;
			else {
				printf ("unkown audio32 format s%d_%dkhz_%dkbps\n", bit, samplerate, bitrate);
				return 0;
			}
			break;
		default:
			printf ("unkown bitrate %d\n", bitrate);
			return 0;
	}

	if (decode_gain <= 0.0f) {
		printf ("wrong decode gain %d\n", decode_gain);
		return 0;
	}

	if (mode == 0)
		encode (audio32_format);
	else if (mode == 1)
		decode (audio32_format);
	else
		printf ("unkown mode %d\n", mode);
	return 0;
}


