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
#include "HF_ToneDetect_LIB.h"
#include "pcm_play.h"

#define INLENGTH	256
#define DetectTrigLen 0//15
#define	OVERLAP		1
#define AUDIO_FRAMES	256

#define BEEP_PCM                "/etc/notify/beep.pcm"
#define TONE_DEVICE				"snx_audio_pcm_48k_ex"

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

int compose_data(unsigned char *aes_enc, unsigned char *block)
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

	if (!ret) { /* checksum is correct */
		if (cmd->set != curr_set) {
			memset(rec_flag, 0, sizeof(rec_flag));
			memset(rec_len, 0, sizeof(rec_len));

			curr_set = cmd->set;
		}
		
		memcpy(aes_enc + (cmd->seq * 26), cmd->data, cmd->len);
		rec_flag[cmd->seq] = 1;
		rec_len[cmd->seq] = cmd->len & 0x1f;

		if (cmd->endflag != 0)
			rec_flag[cmd->seq] |= (0x1 << 7);
	}

	return ret;
}

void snx_play_audio(char *src_audio)
{
	char cmd[64] = {0};
	system ("/bin/gpio_ms1 -n 7 -m 1 -v 1");
	usleep(35000); // dealy 35ms
	pcm_play(src_audio);
	system ("/bin/gpio_ms1 -n 7 -m 1 -v 0");
}

int main (int argc, char *argv[])
{
	int i = 0, j = 0, k = 0;
	int err;
	char *buffer;
	int buffer_frames = AUDIO_FRAMES;
	unsigned int rate = 48000;
	snd_pcm_t *capture_handle;
	snd_pcm_hw_params_t *hw_params;
	snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
	FILE *faudio;
	FILE *fwifi;
	char head[44];	// Porting start tag
	char SN[50] = "1111111111111111";
	short data[INLENGTH];
	long CommandVal;
	short CmdFrmCnt;
	int cnt0 = 0;
	unsigned char decode_data[30];
	unsigned char *aes_encrypt_data;
	unsigned char *aes_decrypt_data;
	int enc_len = 0, dec_len = 0;
	int ret = 0;
	int recv_cnt = 0, recv_len = 0;

	char audio_src[128];

	unsigned char aes_key[16] = {0x35, 0x67, 0xaf, 0xcd, 0x25, 0xaf, 0xce, 0x12,
				 0xad, 0x63, 0xd7, 0x2b, 0x82, 0xf9, 0x05, 0x4a};
	unsigned char aes_iv[16] = {0xb3, 0xfd, 0x42, 0x2a, 0x85, 0x63, 0x78, 0x23,
					 0xaf, 0xfb, 0x8c, 0x24, 0x58, 0x61, 0x45, 0xda};
#if 0
	unsigned char test_pattern[64] = {0x08, 0xDF, 0x7F, 0x08, 0xEA, 0x80, 0x24, 0xDE,
			0x39, 0x23, 0x66, 0x84, 0x06, 0x44, 0xAC, 0x6F,
			0xF5, 0xA8, 0x96, 0x34, 0x97, 0x81, 0x09, 0x86,
			0x9D, 0xD7, 0xF2, 0x9A, 0x3A, 0x54, 0xCB, 0x90,
			0x6B, 0xE4, 0x48, 0x71, 0x6F, 0x73, 0x9E, 0x39,
			0x04, 0xC1, 0x21, 0x69, 0xE3, 0x29, 0xC4, 0x29,
			0x3B, 0x54, 0xFD, 0x8F, 0x31, 0x50, 0x61, 0x80,
			0x75, 0xC0, 0x29, 0x6F, 0x48, 0xF5, 0x5C, 0xBF};
#endif

	// Show version
	printf("Version 1.17 v3 backport\n");

	aes_encrypt_data = (unsigned char *)malloc(1024);
	memset(aes_encrypt_data, 0, 1024);
	memset(rec_flag, 0, 40);
	memset(rec_len, 0, 40);

	strcpy(audio_src, BEEP_PCM);

	curr_set = 0;

#if 0
	aes_decrypt_data = (unsigned char *)malloc(64);
	dec_len = aes_cbc_decrypt(test_pattern, 64,
			aes_decrypt_data, aes_key, aes_iv);

        printf("\nDecode Data Checking(Hex) :\n");
        for (j = 0 ; j < 64 ; j++) {
                if (j > 7 && j % 8 ==0)
                        printf("\n");
                printf("0x%02x ", aes_decrypt_data[j]);
        }


	free(aes_decrypt_data);

	return 0;
#endif

#if 0
	if (argc != 2) {

		printf("Usage: %s DeviceName\n", argv[0]);
		printf("\t example: %s snx_audio_pcm_48k_ex\n", argv[0]);
		exit (1);
	}
#endif 

	//faudio = fopen("myaudio.raw", "wb");

	if ((err = snd_pcm_open (&capture_handle, TONE_DEVICE, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
		fprintf (stderr, "cannot open audio device %s (%s)\n",
				TONE_DEVICE, snd_strerror (err));
		exit (1);
	}
 
	fprintf(stdout, "audio interface opened\n");
		   
	if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
		fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
				snd_strerror (err));
		exit (1);
	}

	fprintf(stdout, "hw_params allocated\n");
				 
	if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) {
		fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
				snd_strerror (err));
		exit (1);
	}
 
	fprintf(stdout, "hw_params initialized\n");
	
	if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		fprintf (stderr, "cannot set access type (%s)\n",
				snd_strerror (err));
		exit (1);
	}
 
	fprintf(stdout, "hw_params access setted\n");
	
	if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, format)) < 0) {
		fprintf (stderr, "cannot set sample format (%s)\n",
				snd_strerror (err));
		exit (1);
	}
 
	fprintf(stdout, "hw_params format setted\n");
	
	if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, &rate, 0)) < 0) {
		fprintf (stderr, "cannot set sample rate (%s)\n",
				snd_strerror (err));
		exit (1);
	}
	
	fprintf(stdout, "hw_params rate setted\n");
 
	if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, 1)) < 0) {
		fprintf (stderr, "cannot set channel count (%s)\n",
				snd_strerror (err));
		exit (1);
	}
 
	fprintf(stdout, "hw_params channels setted\n");
	
	if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) {
		fprintf (stderr, "cannot set parameters (%s)\n",
				snd_strerror (err));
		exit (1);
	}
 
	fprintf(stdout, "hw_params setted\n");
	
	snd_pcm_hw_params_free (hw_params);
 
	fprintf(stdout, "hw_params freed\n");
	
	if ((err = snd_pcm_prepare (capture_handle)) < 0) {
		fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
				snd_strerror (err));
		exit (1);
	}
 
	fprintf(stdout, "audio interface prepared\n");
 
	//printf("alloc size = %d\n", 128 * snd_pcm_format_width(format) / 8 * 1);	/* alloc 256 bytes */
	buffer = (char *)malloc(buffer_frames * snd_pcm_format_width(format) / 8 * 1);
 
	fprintf(stdout, "buffer allocated\n");
 
	// Initialization of Detecting Tone Signal
	if (ToneInit(SN) < 0)
		goto out;

	CmdFrmCnt = DetectTrigLen;
	CommandVal = -1;

	printf("==== Starting Recording ====\n");

	while (1) {
	//for (i = 0 ; i < 3750 * 2 ; ++i) {	/* Record 10 seconds */
start:

		while(buffer_frames > 0) {

		err = snd_pcm_readi (capture_handle, buffer, buffer_frames);
			if(err <= 0) {
				if(err == -EPIPE) {
					printf("@@@@@@ overrun!\n");
					snd_pcm_prepare(capture_handle);
				} else if(err == -EAGAIN) {
					usleep(100);
					printf("@@@@@ usleep 100....\n");
					//	snd_pcm_wait(audio->pcm, 1000);
				}else {
					fprintf (stderr, "@@@@@ read from audio interface failed (%d) (%s)\n",
					err, snd_strerror (err));

			exit(1);
				}

			}else {
			buffer += err * sizeof(short);
			buffer_frames -= err;
			}

		}

		if (buffer_frames == 0) {
			buffer_frames = AUDIO_FRAMES;
			buffer -= AUDIO_FRAMES * sizeof(short);
		}

#if 0		
		if ((err = snd_pcm_readi (capture_handle, buffer, buffer_frames)) != buffer_frames) {
			fprintf (stderr, "read from audio interface failed (%s)\n",
					err, snd_strerror (err));
			exit (1);
		}
#endif

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
                        ret = compose_data(aes_encrypt_data, decode_data);

			//CommandVal = -1;
			//CmdFrmCnt = 0; // Reset Frame Count

			ToneInit(SN);
			CmdFrmCnt = DetectTrigLen;
			CommandVal = -1;

		}
		//fprintf(stdout, "read %d done\n", i);
		//fwrite(buffer, 1, 256, faudio);

		/* check rec_flag array */
		for (j = 0 ; j < sizeof(rec_flag) ; j++) {
			recv_cnt += rec_flag[j] & 0x1;
			recv_len += rec_len[j] & 0xff;

			if ((rec_flag[j] == 0x81) && (recv_cnt == j + 1)) {
				/* Call AES decryption */
				//printf("Recv len = %d\n", recv_len);

				if (recv_len % 16 != 0) {
					enc_len = ((recv_len / 16) + 1) * 16;
				} else {
					enc_len = recv_len;
				}
				aes_decrypt_data = (unsigned char *)malloc(enc_len);
				dec_len = aes_cbc_decrypt(aes_encrypt_data, enc_len,
						aes_decrypt_data, aes_key, aes_iv);

#ifdef TD_DEBUG
	                        printf("\nAES decryption Data Checking(Hex) :\n");
	                        for (k = 0 ; k < enc_len ; k++) {
	                                if (k > 7 && k % 8 == 0)
	                                        printf("\n");
	                                printf("0x%02x ", aes_decrypt_data[k]);
	                        }
	                        printf("\n");
#endif

				fwifi = fopen("/tmp/ssid.txt", "wb");
				if (fwifi) {
					printf("dec_len = %d\n", dec_len);
					fwrite(aes_decrypt_data, 1, dec_len, fwifi);
					printf("Detected!!!!!!!\n");
					printf("\t%s\n", aes_decrypt_data);
					fclose(fwifi);

					snx_play_audio(audio_src);

					free(aes_decrypt_data);
				}
				goto out;
			}

		}
		recv_cnt = 0;
		recv_len = 0;
	}
 
out:
	free(buffer);
 
//	fprintf(stdout, "buffer freed\n");
	
	snd_pcm_close (capture_handle);
//	fprintf(stdout, "audio interface closed\n");
//	fclose(faudio);

	free(aes_encrypt_data);

	exit (0);
}
