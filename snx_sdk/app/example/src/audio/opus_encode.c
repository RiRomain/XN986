/*
* usage: 
* 	./opus_encode -f sample_rate bit_rate infile outfile
* 	./opus_encode -d sample_rate bit_rate capture_time(in second) outfile
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
#define APPLICATION OPUS_APPLICATION_VOIP
#define BANDWIDTH OPUS_BANDWIDTH_NARROWBAND
#define CVBR	1

void usage(void)
{
	fprintf(stderr, "args error\n");
	fprintf(stderr, "./opus_encode -f sample_rate bit_rate frame_size complexity(0 ~ 10) VBR  infile outfile\n");
	fprintf(stderr, "./opus_encode -d sample_rate bit_rate frame_size complexity(0 ~ 10) VBR capture_time(in second) outfile\n");
}

int main(int argc, char *argv[])
{
	int index, size;
	snd_pcm_t *pcm = NULL;
	snd_pcm_hw_params_t *params;
	int frames;
	char *pcm_buf, *opus_buf, *buf;
	int retval = 0;
	int res;
	OpusEncoder *encoder=NULL;
	int application = APPLICATION;

	int args = 1;
	int is_file_mode = 0;
	unsigned int sample_rate, bitrate, frame_size, complexity, vbr, cap_time;
	char *infile_name, *outfile_name;
	FILE *out_fp = NULL, *in_fp = NULL;
	unsigned int encode_frame = 0, remain_frame = 0;

	if(argc != 9)
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

	bitrate = atol(argv[args]);
	args++;
	
	frame_size = atol(argv[args]);
	args++;

	complexity = atol(argv[args]);
	args++;

	vbr = atol(argv[args]);
	args++;

	if(!is_file_mode)
	{
		cap_time = atol(argv[args]);
		args++;
	}

	outfile_name = argv[argc - 1];
	if(is_file_mode)
		infile_name = argv[argc - 2];

	if(is_file_mode)
		printf("File mode:\n");
	else
	{
		printf("Device mode:\n");
		printf("Capture time: %d second\n", cap_time);
	}
	printf("sample rate:%d\n", sample_rate);
	printf("bit rate:%d\n", bitrate);
	printf("frame size:%d\n", frame_size);
	printf("complexity:%d\n", complexity);
	printf("VBR:%d\n", vbr);
	printf("\n\n");
	
  
	out_fp = fopen(outfile_name, "wb+");
	if(out_fp == NULL)
	{
		fprintf(stderr, "open output file %s failed\n", outfile_name);
		retval = -1;
		goto end;
	}

	if(is_file_mode)
	{
		in_fp = fopen(infile_name, "rb");
		if(in_fp == NULL)
		{
			fprintf(stderr, "open input file %s failed\n", infile_name);
			retval = -1;
			goto end;
		}
	}
	else
	{
		/* Open PCM device for playback. */
		res = snd_pcm_open(&pcm, DEVICE, SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK);
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
	
	/* Init opus encoder */
	encoder = opus_encoder_create(sample_rate, CHANNEL, application, &res);
	if (res != OPUS_OK)
	{
		fprintf(stderr, "Cannot create encoder: %s\n", opus_strerror(res));
		retval = -2;
		goto end;
	}

	opus_encoder_ctl(encoder, OPUS_SET_BITRATE(bitrate));
	opus_encoder_ctl(encoder, OPUS_SET_BANDWIDTH(BANDWIDTH));
	opus_encoder_ctl(encoder, OPUS_SET_VBR(vbr));
//	opus_encoder_ctl(encoder, OPUS_SET_VBR_CONSTRAINT(CVBR));
	opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(complexity));
	
	size = frame_size * PER_SAMPLE_BYTE;
	pcm_buf = (char *) malloc(size);
	if(pcm_buf == NULL)
	{
		fprintf(stderr, "malloc pcm buffer failed\n");
		retval = -1;
		goto end;
	}

	opus_buf = (char *) malloc(size);
	if(opus_buf == NULL)
	{
		fprintf(stderr, "malloc opus buffer failed\n");
		retval = -1;
		goto end;
	}

	printf("start capture\n");
	index = 0;
	while(1)
	{
		if(!is_file_mode)
		{
			if(index >= (sample_rate / frame_size * cap_time))
			{
				printf("end capture\n");
				goto end;
			}
		}
		frames = frame_size - remain_frame;
		buf = pcm_buf + remain_frame * PER_SAMPLE_BYTE;
		while(frames > 0)
		{
			if(is_file_mode)
			{
				res = fread(buf, 1, frames * PER_SAMPLE_BYTE, in_fp);
				if(res <= 0)
				{
					printf("end capture\n");
					goto end;
				}
				frames -= res / PER_SAMPLE_BYTE;
				buf += res;
			}
			else
			{
				res = snd_pcm_readi(pcm, buf, frames);
				if (res > 0)
				{
					buf += res * PER_SAMPLE_BYTE;
					frames -= res;
				}
				else
				{
					if (res == -EPIPE)
					{
						fprintf(stderr, "overrun occurred\n");
						snd_pcm_prepare(pcm);
					}
					else if (res == -EAGAIN)
					{
						snd_pcm_wait(pcm, 1000);
					}
					else
					{
						fprintf(stderr,"error from readi: %s\n",snd_strerror(res)); 
						retval = -1;
						goto end;
					}
				}
			}
		}

		/* opus encode */
		res = opus_encode(encoder, (const opus_int16 *)pcm_buf, frame_size, opus_buf, size);	
		if(res < 0)
		{
			fprintf(stderr, "error opus encode:%d\n", res);
			retval = -3;
			goto end;
		}
		encode_frame = opus_packet_get_samples_per_frame(opus_buf, sample_rate) * \
			opus_packet_get_nb_frames(opus_buf, res);
		remain_frame = frame_size - encode_frame;
		if(remain_frame)
			memcpy(pcm_buf, pcm_buf + encode_frame * PER_SAMPLE_BYTE, remain_frame * PER_SAMPLE_BYTE);

		printf("%d:encode data size %d\n", index, res);
		printf("%d:encode frame %d\n", index, encode_frame);
		printf("%d:remain frame %d\n", index, remain_frame);

		/* write file */
		fwrite(&res, sizeof(res), 1, out_fp);
		fwrite(opus_buf, 1, res, out_fp);
		
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
	if(encoder != NULL)
		opus_encoder_destroy(encoder);

	return retval;
}
