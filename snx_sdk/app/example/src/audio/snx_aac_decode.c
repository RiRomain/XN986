#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <fdk-aac/aacdecoder_lib.h>

#define DEVICE				"hw:0,0"
#define CHANNEL				1
#define FORMAT				SND_PCM_FORMAT_S16_LE
#define PER_SAMPLE_BYTE			2

/*
ERR_CONCEAL: Error concealment method
0: Spectral muting.
1: Noise substitution (see CONCEAL_NOISE).
2: Energy interpolation (adds additional signal delay of one frame, see CONCEAL_INTER).
*/
#define ERR_CONCEAL			2

/*
DRC_BOOST: Dynamic Range Control: boost, where [0] is none and [127] is max boost
*/
#define DRC_BOOST			0

/*
DRC_CUT: Dynamic Range Control: attenuation factor, where [0] is none and [127] is max compression
*/
#define DRC_CUT				0

/*
DRC_LEVEL: Dynamic Range Control: reference level, quantized to 0.25dB steps where [0] is 0dB and [127] is -31.75dB
*/
#define DRC_LEVEL			0

/*
DRC_HEAVY: Dynamic Range Control: heavy compression, where [1] is on (RF mode) and [0] is off
*/
#define DRC_HEAVY			0

/*
LIMIT_ENABLE: Signal level limiting enable
-1: Auto-config. Enable limiter for all non-lowdelay configurations by default.
0: Disable limiter in general.
1: Enable limiter always. It is recommended to call the decoder with a AACDEC_CLRHIST flag to reset all states when the limiter switch is changed explicitly.
*/
#define LIMIT_ENABLE			-1

/*
1024 or 960 for AAC-LC
2048 or 1920 for HE-AAC (v2)
512 or 480 for AAC-LD and AAC-ELD
*/
#define PCM_BUF_SIZE			(2048 * CHANNEL * PER_SAMPLE_BYTE)

#define AAC_BUF_SIZE			768

void usage(void)
{
	fprintf(stderr, "usage:\n");
	fprintf(stderr, "./aac_decode -f infile outfile\n");
	fprintf(stderr, "./aac_decode -d infile\n");
}

int aac_decoder_init(HANDLE_AACDECODER *phandle)
{
	HANDLE_AACDECODER handle;
	int retval = 0;

	handle = aacDecoder_Open(TT_MP4_ADTS, 1);
	if(handle == NULL)
	{
		fprintf(stderr, "Error opening decoder\n");
		retval = -11;
		goto out;
	}

	if(aacDecoder_SetParam(handle, AAC_CONCEAL_METHOD, ERR_CONCEAL) != AAC_DEC_OK)
	{
		fprintf(stderr, "Unable to set error concealment method\n");
		retval = -12;
		goto err;
	}

	if(aacDecoder_SetParam(handle, AAC_PCM_MAX_OUTPUT_CHANNELS, CHANNEL) != AAC_DEC_OK) 
	{ 
		fprintf(stderr, "Unable to set output channels in the decoder\n"); 
		retval = -13; 
		goto err; 
	}


	if(aacDecoder_SetParam(handle, AAC_DRC_BOOST_FACTOR, DRC_BOOST) != AAC_DEC_OK)
	{
		fprintf(stderr, "Unable to set DRC boost factor in the decoder\n");
		retval = -14;
		goto err;
	}

	if(aacDecoder_SetParam(handle, AAC_DRC_ATTENUATION_FACTOR, DRC_CUT) != AAC_DEC_OK)
	{
		fprintf(stderr, "Unable to set DRC attenuation factor in the decoder\n");
		retval = -15;
		goto err;
	}

	if(aacDecoder_SetParam(handle, AAC_DRC_REFERENCE_LEVEL, DRC_LEVEL) != AAC_DEC_OK)
	{
		fprintf(stderr, "Unable to set DRC reference level in the decoder\n");
		retval = -16;
		goto err;
	}

	if(aacDecoder_SetParam(handle, AAC_DRC_HEAVY_COMPRESSION, DRC_HEAVY) != AAC_DEC_OK)
	{
		fprintf(stderr, "Unable to set DRC heavy compression in the decoder\n");
		retval = -17;
		goto err;
	}

/*
	if(aacDecoder_SetParam(handle, AAC_TPDEC_CLEAR_BUFFER, 1) != AAC_DEC_OK)
	{
		fprintf(stderr, "failed to clear buffer when flushing\n");
		retval = -19;
		goto err;
	}
*/


	*phandle = handle;

out:
	return retval;

err:
	aacDecoder_Close(handle);
	return retval;
}

int audio_device_init(unsigned int sample_rate, snd_pcm_t **ppcm)
{
	snd_pcm_t *pcm = NULL;
	snd_pcm_hw_params_t *params;
	int retval = 0;

	/* Open PCM device for playback. */
	retval = snd_pcm_open(&pcm, DEVICE, SND_PCM_STREAM_PLAYBACK, 0);
	if (retval < 0) 
	{
		fprintf(stderr,"unable to open pcm device: %s\n",snd_strerror(retval));
		retval = -21;
		goto out;
 	}

	/* Allocate a hardware parameters object. */
	snd_pcm_hw_params_alloca(&params);

	/* Fill it in with default values. */
	retval = snd_pcm_hw_params_any(pcm, params);
	if (retval < 0) 
	{
		fprintf(stderr, "snd_pcm_hw_params_any");
		retval = -22;
		goto err;
	}

	/* Interleaved mode */
	retval = snd_pcm_hw_params_set_access(pcm, params,SND_PCM_ACCESS_RW_INTERLEAVED);
	if (retval < 0) 
	{
		fprintf(stderr, "snd_pcm_hw_params_set_access");
		retval = -23;
		goto err;
	}

	/* sample format */
	retval = snd_pcm_hw_params_set_format(pcm, params, FORMAT);
	if (retval < 0)
	{
		fprintf(stderr, "snd_pcm_hw_params_set_format");
		retval = -24;
		goto err;
	}

	/* channels */
	retval = snd_pcm_hw_params_set_channels(pcm, params, CHANNEL);
	if (retval < 0)
	{
		fprintf(stderr, "snd_pcm_hw_params_set_channels");
		retval = -25;
		goto err;
	}

	/* sampling rate  */
	retval = snd_pcm_hw_params_set_rate(pcm, params, sample_rate, 0);
	if (retval < 0)
	{
		fprintf(stderr, "snd_pcm_hw_params_set_rate_near");
		retval = -26;
		goto err;
	}

	/* Write the parameters to the driver */
	retval = snd_pcm_hw_params(pcm, params);
	if (retval < 0)
	{
  		fprintf(stderr,"unable to set hw parameters: %s\n",snd_strerror(retval));
  		retval = -27;
  		goto err;
	}

	*ppcm = pcm;

out:
	return retval;

err:
	snd_pcm_close(pcm);
	return retval;
}

int main(int argc, char *argv[])
{
	HANDLE_AACDECODER handle = NULL;
	AAC_DECODER_ERROR err;
	snd_pcm_t *pcm = NULL;
	int frames;
	char *aac_buf, *pcm_buf, *buf;
	int aac_buf_size, pcm_buf_size, size;
	int res;
	int retval = 0;

	int args = 1;
	int is_file_mode = 0;
	unsigned int sample_rate, frame_size;
	char *infile_name, *outfile_name;
	FILE *out_fp = NULL, *in_fp = NULL;
	int is_get_aud_info = 0;
	unsigned int valid_bytes;
	int is_file_end = 0;

	if(argc < 3)
	{
		retval = -1;
		goto cmd_err;
	}

	if(strcmp(argv[args], "-f") == 0)
		is_file_mode = 1;
	else if(strcmp(argv[args], "-d") == 0)	//device mode
		is_file_mode = 0;
	else
	{
		retval = -2;
		goto cmd_err;
	}
	args++;

	if(is_file_mode)
	{
		outfile_name = argv[argc - 1];
		infile_name = argv[argc - 2];
	}
	else
		infile_name = argv[argc - 1];

	if(is_file_mode)
		printf("File mode:\n");
	else
		printf("Device mode:\n");

	retval = aac_decoder_init(&handle);
	if(retval < 0)
		goto end;

	in_fp = fopen(infile_name, "rb");
	if(in_fp == NULL)
	{
		fprintf(stderr, "open input file %s failed\n", infile_name);
		retval = -3;
		goto end;
	}


	pcm_buf_size = PCM_BUF_SIZE;
	pcm_buf = (char *)malloc(pcm_buf_size);
	if(pcm_buf == NULL)
	{
		fprintf(stderr, "malloc PCM buffer failed\n");
		retval = 5;
		goto end;
	}

	aac_buf_size = AAC_BUF_SIZE;
	aac_buf = (char *)malloc(aac_buf_size);
	if(aac_buf == NULL)
	{
		fprintf(stderr, "malloc AAC buffer failed\n");
		retval = 6;
		goto end;
	}

	printf("start aac decode\n");
	while(1)
	{
		size = aac_buf_size;
		buf = aac_buf;
		while(size > 0)
		{
			res = fread(buf, 1, size, in_fp);
			if(res <= 0)
			{
				if(feof(in_fp))
				{
					is_file_end = 1;
					break;
				}

				fprintf(stderr, "read file error\n");
				retval = -41;
				goto end;
			}
			size -= res;
			buf += res;
		}
		size = aac_buf_size - size;
		valid_bytes = size;

refill:
		if(aacDecoder_Fill(handle, (UCHAR **)&aac_buf, (const UINT *)&size, &valid_bytes) != AAC_DEC_OK)
		{
			fprintf(stderr, "aacDecoder_Fill failed\n");
			retval = -42;
			goto end;
		}
aac_decode:
		err = aacDecoder_DecodeFrame(handle, (INT_PCM *)pcm_buf, pcm_buf_size, 0);
		if(err == AAC_DEC_NOT_ENOUGH_BITS)
		{
			if(valid_bytes)
				goto refill;

			if(is_file_end)
			{
				printf("end aac decode\n");
				goto end;
			}

			continue;
		}

		if(err != AAC_DEC_OK)
		{
			fprintf(stderr, "aacDecoder_DecodeFrame failed --- 0x%08x\n", err);
			retval = -43;
			goto end;
		}

		if(is_get_aud_info == 0)
		{
			CStreamInfo *info = aacDecoder_GetStreamInfo(handle);
			if(info == NULL)
			{
				fprintf(stderr, "Unable to get stream info\n");
				retval = -44;
				goto end;
			}

			if(info->sampleRate <= 0)
			{
				fprintf(stderr, "Stream info not initialized\n");
				retval = -45;
				goto end;
			}

			sample_rate = info->sampleRate;
			frame_size = info->frameSize;
			printf("sample rate:%d\n", sample_rate);
			printf("frame size:%d\n", frame_size);
			printf("\n\n");
			is_get_aud_info = 1;

			if(is_file_mode)
			{
				out_fp = fopen(outfile_name, "wb+");
				if(out_fp == NULL)
				{
					fprintf(stderr, "open output file %s failed\n", outfile_name);
					retval = -46;
					goto end;
				}
			}
			else
			{
				retval = audio_device_init(sample_rate, &pcm);
				if(retval < 0)
					goto end;
			}
		}
		

		frames = frame_size * CHANNEL;
		buf = pcm_buf;
		while(frames > 0)
		{
			if(is_file_mode)
			{
				res = fwrite(buf,  PER_SAMPLE_BYTE, frames, out_fp);
				if(res <= 0)
				{
					fprintf(stderr, "write file error\n");
					retval = -47;
					goto end;
				}
			}
			else
			{
				res = snd_pcm_writei(pcm, buf, frames);
				if (res < 0)
				{
					if (res == -EPIPE)
					{
						fprintf(stderr, "underrun occurred\n");
						snd_pcm_prepare(pcm);
						continue;
					}
					else if (res == -EAGAIN)
					{
						usleep(100);
						continue;
					}
					else
					{
						fprintf(stderr,"error from writei: %s\n",snd_strerror(res)); 
						retval = -48;
						goto end;
					}
				}
			}
			buf += res * PER_SAMPLE_BYTE;
			frames -= res;
		}

		if(valid_bytes)
			goto refill;
		if(is_file_end)
			goto aac_decode;
	}


end:
	if(in_fp != NULL)
		fclose(in_fp);

	if(out_fp != NULL)
		fclose(out_fp);

	if(pcm_buf != NULL)
		free(pcm_buf);

	if(aac_buf != NULL)
		free(aac_buf);

	if(pcm != NULL)
	{
		snd_pcm_drain(pcm);
		snd_pcm_close(pcm);
	}

	if(handle != NULL)
		aacDecoder_Close(handle);

	return retval;

cmd_err:
	fprintf(stderr, "Command error.\n");
	usage();
	return retval;
}
