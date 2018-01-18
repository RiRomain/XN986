/*
* usage: 
* 	./opus_decode -f sample_rate bit_rate infile outfile
* 	./opus_decode -d sample_rate bit_rate infile
*/

#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <opus.h>
#include <opus_types.h>

#define DEVICE	"hw:0,0"
#define CHANNEL 1
#define FORMAT SND_PCM_FORMAT_S16_LE
#define PER_SAMPLE_BYTE 2

void usage(void)
{
	fprintf(stderr, "args error\n");
	fprintf(stderr, "./opus_decode -f sample_rate infile outfile\n");
	fprintf(stderr, "./opus_decode -d sample_rate infile\n");
}

int main(int argc, char *argv[])
{
	snd_pcm_t *pcm = NULL;
	snd_pcm_hw_params_t *params;
	int frames;
	char *opus_buf, *pcm_buf, *buf;
	int opus_buf_size, pcm_buf_size, size;
	OpusDecoder *decoder = NULL;
	int res;
	int retval = 0;

	int args = 1;
	int is_file_mode = 0;
	unsigned int sample_rate, bitrate, frame_size;
	char *infile_name, *outfile_name;
	FILE *out_fp = NULL, *in_fp = NULL;
	int index = 0;

	if(argc < 4)
	{
		usage();
		return -1;
	}

	if(strcmp(argv[args], "-f") == 0)
		is_file_mode = 1;
	else if(strcmp(argv[args], "-d") == 0)	//device mode
		is_file_mode = 0;
	else
	{
		usage();
		return -1;
	}
	args++;

	sample_rate = atol(argv[args]);
	args++;

	frame_size = sample_rate * 3 / 25;

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
	printf("sample rate:%d\n", sample_rate);
	printf("frame size:%d\n", frame_size);
	printf("\n\n");
	
  
	in_fp = fopen(infile_name, "rb");
	if(in_fp == NULL)
	{
		fprintf(stderr, "open input file %s failed\n", infile_name);
		retval = -1;
		goto end;
	}

	if(is_file_mode)
	{
		out_fp = fopen(outfile_name, "wb+");
		if(out_fp == NULL)
		{
			fprintf(stderr, "open output file %s failed\n", outfile_name);
			retval = -1;
			goto end;
		}
	}
	else
	{
		/* Open PCM device for playback. */
		res = snd_pcm_open(&pcm, DEVICE, SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
		if (res < 0) 
		{
			fprintf(stderr,"unable to open pcm device: %s\n",snd_strerror(res));
			retval = -1;
			goto end;
 		}

		/* Allocate a hardware parameters object. */
		snd_pcm_hw_params_alloca(&params);

		/* Fill it in with default values. */
		res = snd_pcm_hw_params_any(pcm, params);
		if (res < 0) 
		{
			perror("snd_pcm_hw_params_any");
			retval = -1;
			goto end;
		}

		/* Interleaved mode */
		res = snd_pcm_hw_params_set_access(pcm, params,SND_PCM_ACCESS_RW_INTERLEAVED);
		if (res < 0) 
		{
			perror("snd_pcm_hw_params_set_access");
			retval = -1;
			goto end;
		}

		/* sample format */
		res = snd_pcm_hw_params_set_format(pcm, params, FORMAT);
		if (res < 0)
		{
			perror("snd_pcm_hw_params_set_format");
			retval = -1;
			goto end;
		}

		/* channels */
		res = snd_pcm_hw_params_set_channels(pcm, params, CHANNEL);
		if (res < 0)
		{
			perror("snd_pcm_hw_params_set_channels");
			retval = -1;
			goto end;
		}

		/* sampling rate  */
		res = snd_pcm_hw_params_set_rate(pcm, params, sample_rate, 0);
		if (res < 0)
		{
			perror("snd_pcm_hw_params_set_rate_near");
			retval = -1;
			goto end;
		}


		/* Write the parameters to the driver */
		res = snd_pcm_hw_params(pcm, params);
		if (res < 0)
		{
  			fprintf(stderr,"unable to set hw parameters: %s\n",snd_strerror(res));
  			retval = -1;
  			goto end;
		}
	}

	/* Init opus decoder */
	decoder = opus_decoder_create(sample_rate, CHANNEL, &res);
	if (res != OPUS_OK)
	{
		fprintf(stderr, "Cannot create decoder: %s\n", opus_strerror(res));
		retval = -2;
		goto end;
	}


	/* Use a buffer large enough to hold one period */
	pcm_buf_size = frame_size * PER_SAMPLE_BYTE;
	pcm_buf = (char *) malloc(pcm_buf_size);
	if(pcm_buf == NULL)
	{
		fprintf(stderr, "malloc pcm buffer failed\n");
		retval = -1;
		goto end;
	}

	opus_buf_size = pcm_buf_size;
	opus_buf = (char *) malloc(opus_buf_size);
	if(opus_buf == NULL)
	{
		fprintf(stderr, "malloc opus buffer failed\n");
		retval = -1;
		goto end;
	}

	while(1)
	{
		int block_size;

		res = fread(&block_size, sizeof(block_size), 1, in_fp);
		if(res < 0)
		{
			fprintf(stderr, "read file error\n");
			goto end;
		}
		printf("%d:decode block size:%d\n", index, block_size);
		
		size = block_size;
		buf = opus_buf;
		while(size > 0)
		{
			res = fread(buf, 1, size, in_fp);
			if(res <= 0)
			{
				fprintf(stderr, "decode end\n");
				goto end;
			}
			size -= res;
			buf += res;
		}

		
		/* opus decode */
		res = opus_decode(decoder, opus_buf, block_size, (opus_int16 *)pcm_buf, frame_size, 0);	
		if(res < 0)
		{
			fprintf(stderr, "error opus decode:%d\n", res);
			retval = -3;
			goto end;
		}

		frames = res;
		buf = pcm_buf;
		while(frames > 0)
		{
			if(is_file_mode)
			{
				res = fwrite(buf, 1, frames * PER_SAMPLE_BYTE, out_fp);
				if(res <= 0)
				{
					fprintf(stderr, "write file error\n");
					goto end;
				}
				frames -= res / PER_SAMPLE_BYTE;
				buf += res;
			}
			else
			{
				res = snd_pcm_writei(pcm, buf, frames);
				if (res <= 0)
				{
					if (res == -EPIPE)
					{
						fprintf(stderr, "underrun occurred\n");
						snd_pcm_prepare(pcm);
					}
					else if (res == -EAGAIN)
					{
						usleep(100);
					}
					else
					{
						fprintf(stderr,"error from writei: %s\n",snd_strerror(res)); 
						retval = -1;
						goto end;
					}
				}
				else
				{
					buf += res * PER_SAMPLE_BYTE;
					frames -= res;
				}
			}
		}

		index++;

	}	


end:
	if(in_fp != NULL)
	{
		fclose(in_fp);
	}

	if(out_fp != NULL)
	{
		fclose(out_fp);
	}

	if(pcm_buf != NULL)
	{
		free(pcm_buf);
	}

	if(opus_buf != NULL)
	{
		free(opus_buf);
	}

	if(pcm != NULL)
	{
		snd_pcm_drain(pcm);
		snd_pcm_close(pcm);
	}
	if(decoder != NULL)
		opus_decoder_destroy(decoder);

	return retval;
}
