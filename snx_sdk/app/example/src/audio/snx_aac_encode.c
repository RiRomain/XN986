/*
* usage: 
* 	./opus_encode -f sample_rate bit_rate infile outfile
* 	./opus_encode -d sample_rate bit_rate capture_time(in second) outfile
*/

#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <fdk-aac/aacenc_lib.h>

#define DEVICE			"hw:0,0"
#define CHANNEL			1
#define FORMAT			SND_PCM_FORMAT_S16_LE
#define PER_SAMPLE_BYTE		2


#define CHANNEL_MODE		MODE_1

/*
CHANNEL_ORDER:
0: MPEG channel ordering
1: WAVE file format channel ordering
*/
#define CHANNEL_ORDER		1

/*
TRANSPORT_TYPE:
TT_MP4_RAW: raw access units
TT_MP4_ADIF: ADIF bitstream format
TT_MP4_ADTS: ADTS bitstream format
TT_MP4_LATM_MCP1: Audio Mux Elements (LATM) with muxConfigPresent = 1
TT_MP4_LATM_MCP0: Audio Mux Elements (LATM) with muxConfigPresent = 0, out of band StreamMuxConfig
TT_MP4_LOAS: Audio Sync Stream (LOAS)
*/
#define TRANSPORT_TYPE		TT_MP4_ADTS

/*
SIGNALING_MODE:
SIG_IMPLICIT: Implicit backward compatible signaling (default for non-MPEG-4 based AOT’s and for the transport formats ADIF and ADTS)
SIG_EXPLICIT_BW_COMPATIBLE: Explicit backward compatible signaling
SIG_EXPLICIT_HIERARCHICAL: Explicit hierarchical signaling (default for MPEG-4 based AOT’s and for all transport formats excluding ADIF and ADTS)
*/
//#define SIGNALING_MODE		SIG_IMPLICIT

struct aac_info_st
{
	unsigned int aot;
	unsigned int eld_sbr;
	unsigned int sample_rate;
	unsigned int vbr;
	unsigned int bitrate;
	unsigned int high_quality;
	AACENC_InfoStruct enc_info;
};

void usage(void)
{
	fprintf(stderr, "usage:\n");
	fprintf(stderr, "./aac_encode -f aot eld_sbr sample_rate vbr bitrate high_quality infile outfile\n");
	fprintf(stderr, "./aac_encode -d aot eld_sbr sample_rate vbr bitrate high_quality capture_time(in second) outfile\n");
	fprintf(stderr, "\naot: audio object type\n");
	fprintf(stderr, "2: MPEG-4 AAC Low Complexity.\n");
	fprintf(stderr, "5: MPEG-4 AAC Low Complexity with Spectral Band Replication (HE-AAC).\n");
	fprintf(stderr, "23: MPEG-4 AAC Low-Delay.\n");
	fprintf(stderr, "39: MPEG-4 AAC Enhanced Low-Delay.\n");
	fprintf(stderr, "\neld_sbr: MPEG-4 AAC Enhanced Low-Delay with Spectral Band Replication.\n");
	fprintf(stderr, "1: enable SBR, 0: disable SBR\n");
	fprintf(stderr, "\nvbr: variable bitrate\n");
	fprintf(stderr, "0:cbr, 1~5:vbr\n");
	fprintf(stderr, "\nbitrate: bitrate in cbr mode, this parameter is useless in vbr mode.\n");
	fprintf(stderr, "\nhigh_quality: 1: high quality, 0:normal quality\n");
	fprintf(stderr, "\n\n");
}

static const char *aac_get_error(AACENC_ERROR err)
{
	switch (err)
	{
		case AACENC_OK:
			return "No error";
		case AACENC_INVALID_HANDLE:
			return "Invalid handle";
		case AACENC_MEMORY_ERROR:
			return "Memory allocation error";
		case AACENC_UNSUPPORTED_PARAMETER:
			return "Unsupported parameter";
		case AACENC_INVALID_CONFIG:
			return "Invalid config";
		case AACENC_INIT_ERROR:
			return "Initialization error";
		case AACENC_INIT_AAC_ERROR:
			return "AAC library initialization error";
		case AACENC_INIT_SBR_ERROR:
			return "SBR library initialization error";
		case AACENC_INIT_TP_ERROR:
			return "Transport library initialization error";
		case AACENC_INIT_META_ERROR:
			return "Metadata library initialization error";
		case AACENC_ENCODE_ERROR:
			return "Encoding error";
		case AACENC_ENCODE_EOF:
			return "End of file";
		default:
			return "Unknown error";
	}
}


int aac_encoder_init(HANDLE_AACENCODER *phandle, struct aac_info_st *paac_info)
{
	HANDLE_AACENCODER handle;
	AACENC_ERROR err;
	int retval = 0;

	err = aacEncOpen(&handle, 0x01|0x02|0x10, CHANNEL);
	if(err != AACENC_OK)
	{
		fprintf(stderr, "Unable to open the encoder: %s\n", aac_get_error(err));
		retval = -11;
		goto out;
	}

	err = aacEncoder_SetParam(handle, AACENC_AOT, paac_info->aot);
	if(err != AACENC_OK)
	{
		fprintf(stderr, "Unable to set the AOT %d: %s\n", paac_info->aot, aac_get_error(err));
		retval = -12;
		goto err;
	}

	if(
		(paac_info->aot == 39)
		&&
		(paac_info->eld_sbr == 1)
	  )
	{
		err = aacEncoder_SetParam(handle, AACENC_SBR_MODE, 1);
		if(err != AACENC_OK)
		{
			fprintf(stderr, "Unable to enable SBR for ELD: %s\n", aac_get_error(err));
			retval = -13;
			goto err;
		}
	}

	err = aacEncoder_SetParam(handle, AACENC_SAMPLERATE, paac_info->sample_rate);
	if(err != AACENC_OK)
	{
		fprintf(stderr, "Unable to set the sample rate %d: %s\n", paac_info->sample_rate, aac_get_error(err));
		retval = -14;
		goto err;
	}

	err = aacEncoder_SetParam(handle, AACENC_CHANNELMODE, CHANNEL_MODE);
	if(err != AACENC_OK)
	{
		fprintf(stderr, "Unable to set channel mode %d: %s\n", CHANNEL_MODE, aac_get_error(err));
		retval = -15;
		goto err;
	}

	err = aacEncoder_SetParam(handle, AACENC_CHANNELORDER, CHANNEL_ORDER);
	if(err != AACENC_OK)
	{
		fprintf(stderr, "Unable to set wav channel order %d: %s\n", CHANNEL_ORDER, aac_get_error(err));
		retval = -16;
		goto err;
	}

	if(paac_info->vbr)
	{
		err = aacEncoder_SetParam(handle, AACENC_BITRATEMODE, paac_info->vbr);
		if(err != AACENC_OK)
		{
			fprintf(stderr, "Unable to set the VBR bitrate mode %d: %s\n", paac_info->vbr, aac_get_error(err));
			retval = -17;
			goto err;
		}
	}
	else
	{
		err = aacEncoder_SetParam(handle, AACENC_BITRATE, paac_info->bitrate);
		if(err != AACENC_OK)
		{
			fprintf(stderr, "Unable to set the bitrate %d: %s\n", paac_info->bitrate, aac_get_error(err));
			retval = -18;
			goto err;
		}
	}

	err = aacEncoder_SetParam(handle, AACENC_TRANSMUX, TRANSPORT_TYPE);
	if(err != AACENC_OK)
	{
		fprintf(stderr, "Unable to set the transmux format: %s\n", aac_get_error(err));
		retval = -19;
		goto err;
	}

	err = aacEncoder_SetParam(handle, AACENC_AFTERBURNER, paac_info->high_quality);
	if(err != AACENC_OK)
	{
		fprintf(stderr, "Unable to set afterburner to %d: %s\n", paac_info->high_quality, aac_get_error(err));
		retval = -21;
		goto err;
	}

	err = aacEncEncode(handle, NULL, NULL, NULL, NULL);
	if(err != AACENC_OK)
	{
		fprintf(stderr, "Unable to initialize the encoder: %s\n", aac_get_error(err));
		retval = -23;
		goto err;
	}

	err = aacEncInfo(handle, &(paac_info->enc_info));
	if(err != AACENC_OK)
	{
		fprintf(stderr, "Unable to get encoder info: %s\n", aac_get_error(err));
		retval = -24;
		goto err;
	}



	*phandle = handle;

out:
	return retval;

err:
	aacEncClose(&handle);
	return retval;
}

int audio_device_init(unsigned int sample_rate, snd_pcm_t **ppcm)
{
	snd_pcm_t *pcm = NULL;
	snd_pcm_hw_params_t *params;
	int retval = 0;

	/* Open PCM device for playback. */
	retval = snd_pcm_open(&pcm, DEVICE, SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK);
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
	snd_pcm_t *pcm = NULL;
	unsigned int frames, remain_frame, frame_size;
	char *pcm_buf, *aac_buf, *buf;
	int pcm_buf_size, aac_buf_size, size;
	int retval = 0;
	int res;
	HANDLE_AACENCODER handle;
	AACENC_ERROR err;
	AACENC_BufDesc in_buf   = { 0 }, out_buf = { 0 };
	AACENC_InArgs  in_args  = { 0 };
	AACENC_OutArgs out_args = { 0 };
	int pcm_buf_identifier = IN_AUDIO_DATA, aac_buf_identifier = OUT_BITSTREAM_DATA;
	int pcm_element_size = 2, aac_element_size = 1;
	struct aac_info_st aac_info;

	int args = 1;
	int is_file_mode = 0;
	unsigned int cap_time, cap_frames;
	char *infile_name, *outfile_name;
	FILE *out_fp = NULL, *in_fp = NULL;

	if(argc != 10)
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

	aac_info.aot = atoi(argv[args]);
	args++;

	aac_info.eld_sbr = atoi(argv[args]);
	args++;

	aac_info.sample_rate = atol(argv[args]);
	args++;

	aac_info.vbr = atoi(argv[args]);
	args++;

	aac_info.bitrate = atol(argv[args]);
	args++;
	
	aac_info.high_quality = atoi(argv[args]);
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
	printf("audio object type:%d\n", aac_info.aot);
	printf("sbr on eld:%d\n", aac_info.eld_sbr);
	printf("sample rate:%d\n", aac_info.sample_rate);
	printf("VBR:%d\n", aac_info.vbr);
	printf("bit rate:%d\n", aac_info.bitrate);
	printf("high quality:%d\n", aac_info.high_quality);
	printf("\n\n");
	
  
	out_fp = fopen(outfile_name, "wb+");
	if(out_fp == NULL)
	{
		fprintf(stderr, "open output file %s failed\n", outfile_name);
		retval = -3;
		goto end;
	}

	if(is_file_mode)
	{
		in_fp = fopen(infile_name, "rb");
		if(in_fp == NULL)
		{
			fprintf(stderr, "open input file %s failed\n", infile_name);
			retval = -4;
			goto end;
		}
	}
	else
	{
		retval = audio_device_init(aac_info.sample_rate, &pcm);
		if(retval < 0)
			goto end;
	}

	retval = aac_encoder_init(&handle, &aac_info);
	if(retval < 0)
		goto end;
	
	

	printf("frame size:%d\n", aac_info.enc_info.frameLength);	
	printf("max output bytes:%d\n\n", aac_info.enc_info.maxOutBufBytes);
	frame_size = aac_info.enc_info.frameLength;
	pcm_buf_size = frame_size * PER_SAMPLE_BYTE;
	pcm_buf = (char *) malloc(pcm_buf_size);
	if(pcm_buf == NULL)
	{
		fprintf(stderr, "malloc pcm buffer failed\n");
		retval = -5;
		goto end;
	}

	aac_buf_size = aac_info.enc_info.maxOutBufBytes;
	aac_buf = (char *) malloc(aac_buf_size);
	if(aac_buf == NULL)
	{
		fprintf(stderr, "malloc opus buffer failed\n");
		retval = -6;
		goto end;
	}

	printf("start capture\n");
	cap_frames = 0;
	remain_frame = 0;
	while(1)
	{
		if(!is_file_mode)
		{
			if(cap_frames >= (aac_info.sample_rate * cap_time))
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
				res = fread(buf, PER_SAMPLE_BYTE, frames, in_fp);
				if(res <= 0)
				{
					if(feof(in_fp))
						break;

					fprintf(stderr, "read file error---%d\n", res);
					retval = -51;
					goto end;
				}
			}
			else
			{
				res = snd_pcm_readi(pcm, buf, frames);
				if(res < 0)
				{
					if (res == -EPIPE)
					{
						fprintf(stderr, "overrun occurred\n");
						snd_pcm_prepare(pcm);
						continue;
					}
					else if (res == -EAGAIN)
					{
						snd_pcm_wait(pcm, 1000);
						continue;
					}
					else
					{
						fprintf(stderr,"error from readi: %s\n",snd_strerror(res)); 
						retval = -52;
						goto end;
					}
				}
			}

			buf += res * PER_SAMPLE_BYTE;
			frames -= res;
			cap_frames += res;
		}

		frames = frame_size - frames;
		size = frames * PER_SAMPLE_BYTE;

		if(frames < frame_size)
			in_args.numInSamples = -1;
		else
		{
			in_args.numInSamples = frames;
			in_buf.numBufs = 1;
			in_buf.bufs = (void**)&pcm_buf;
			in_buf.bufferIdentifiers = &pcm_buf_identifier;
			in_buf.bufSizes = &size;
			in_buf.bufElSizes = &pcm_element_size;
		}

		out_buf.numBufs = 1;
		out_buf.bufs = (void**)&aac_buf;
		out_buf.bufferIdentifiers = &aac_buf_identifier;
		out_buf.bufSizes = &aac_buf_size;
		out_buf.bufElSizes = &aac_element_size;

		err = aacEncEncode(handle, &in_buf, &out_buf, &in_args, &out_args);
		if(err != AACENC_OK)
		{
			if((frames < frame_size) && (err == AACENC_ENCODE_EOF))
			{
				printf("end capture\n");
				goto end;
			}
			fprintf(stderr, "Unable to encode frame: %s\n", aac_get_error(err));
			retval = -53;
			goto end;
		}

		remain_frame = frames - out_args.numInSamples;
		if(remain_frame)
			memcpy(pcm_buf, pcm_buf + out_args.numInSamples * PER_SAMPLE_BYTE, remain_frame * PER_SAMPLE_BYTE);

		/* write file */
		if(out_args.numOutBytes)
			fwrite(aac_buf, 1, out_args.numOutBytes, out_fp);
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
		aacEncClose(&handle);

	return retval;

cmd_err:
	fprintf(stderr, "args error\n");
	usage();
	return retval;

}
