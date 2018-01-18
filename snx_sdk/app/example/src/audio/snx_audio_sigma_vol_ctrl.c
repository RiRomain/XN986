#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>
#include <assert.h>
#include <getopt.h> 
#include <alsa/asoundlib.h>
#include <alsa/control.h>


#define error printf

int snx_audio_crtl_get_items(int card_num, int *items, const char *ctrl_name)
{
	char card[64];
	snd_ctl_elem_info_t *info;
	snd_ctl_elem_id_t *id;
	snd_ctl_elem_value_t *control;
	snd_ctl_elem_type_t type;
	snd_ctl_t *handle = NULL;
	int err = 0;

	sprintf(card, "hw:%i", card_num);
	if((err = snd_ctl_open(&handle, card, 0)) < 0)
	{
		error("Control %s open error: %s\n", card, snd_strerror(err));
		goto ctrl_get_items_exit1;
	}

	snd_ctl_elem_id_alloca(&id);
	snd_ctl_elem_value_alloca(&control);
	snd_ctl_elem_info_alloca(&info);

	snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_MIXER);
	snd_ctl_elem_id_set_name(id, ctrl_name);
	snd_ctl_elem_info_set_id(info, id);
	if((err = snd_ctl_elem_info(handle, info)) < 0)
	{
		error("Cannot find the given element '%s'\n", ctrl_name);
		goto ctrl_get_items_exit2;
	}

	snd_ctl_elem_info_get_id(info, id);
	type = snd_ctl_elem_info_get_type(info);
	switch (type)
	{
		case SND_CTL_ELEM_TYPE_INTEGER:
			*items = snd_ctl_elem_info_get_max(info) - snd_ctl_elem_info_get_min(info) + 1;
			break;

		case SND_CTL_ELEM_TYPE_ENUMERATED:
			*items = snd_ctl_elem_info_get_items(info);
			break;

		default:
			error("the type of '%s' element is error.\n", ctrl_name);
			err = -1;
			goto ctrl_get_items_exit2;
	}

ctrl_get_items_exit2:
//	free(info);
//	free(id);
//	free(control);
	snd_ctl_close(handle);
ctrl_get_items_exit1:
	return err;
}

int snx_audio_ctrl_get(int card_num, int *ctrl_val, const char *ctrl_name)
{
	char card[64];
	snd_ctl_elem_info_t *info;
	snd_ctl_elem_id_t *id;
	snd_ctl_elem_value_t *control;
	snd_ctl_elem_type_t type;
	snd_ctl_t *handle = NULL;
	int err = 0;

	sprintf(card, "hw:%i", card_num);
	if((err = snd_ctl_open(&handle, card, 0)) < 0)
	{
		error("Control %s open error: %s\n", card, snd_strerror(err));
		goto ctrl_get_exit1;
	}

	snd_ctl_elem_info_alloca(&info);
	snd_ctl_elem_id_alloca(&id);
	snd_ctl_elem_value_alloca(&control);

	snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_MIXER);
	snd_ctl_elem_id_set_name(id, ctrl_name);
	snd_ctl_elem_info_set_id(info, id);
	if((err = snd_ctl_elem_info(handle, info)) < 0)
	{
		error("Cannot find the given element '%s'\n", ctrl_name);
		goto ctrl_get_exit2;
	}
	snd_ctl_elem_info_get_id(info, id);
	snd_ctl_elem_value_set_id(control, id);
	if((err = snd_ctl_elem_read(handle, control)) < 0)
	{
		error("'%s' element read error: %s\n", ctrl_name, snd_strerror(err));
		goto ctrl_get_exit2;
	}

	type = snd_ctl_elem_info_get_type(info);
	switch (type)
	{
		case SND_CTL_ELEM_TYPE_INTEGER:
			*ctrl_val = snd_ctl_elem_value_get_integer(control, 0);
			break;

		case SND_CTL_ELEM_TYPE_ENUMERATED:
			*ctrl_val = snd_ctl_elem_value_get_enumerated(control, 0);
			break;

		default:
			error("the type of '%s' element is error.\n", ctrl_name);
			err = -1;
			goto ctrl_get_exit2;
	}


ctrl_get_exit2:
//	free(info);
//	free(id);
//	free(control);
	snd_ctl_close(handle);
ctrl_get_exit1:
	return err;
}

int snx_audio_ctrl_set(int card_num, int ctrl_val, const char *ctrl_name)
{
	char card[64];
	snd_ctl_elem_info_t *info;
	snd_ctl_elem_id_t *id;
	snd_ctl_elem_value_t *control;
	snd_ctl_elem_type_t type;
	snd_ctl_t *handle = NULL;
	int err = 0;

	sprintf(card, "hw:%i", card_num);
	if((err = snd_ctl_open(&handle, card, 0)) < 0)
	{
		error("Control %s open error: %s\n", card, snd_strerror(err));
		goto ctrl_set_exit1;
	}

	snd_ctl_elem_id_alloca(&id);
	snd_ctl_elem_value_alloca(&control);
	snd_ctl_elem_info_alloca(&info);

	snd_ctl_elem_id_set_interface(id, SND_CTL_ELEM_IFACE_MIXER);
	snd_ctl_elem_id_set_name(id, ctrl_name);
	snd_ctl_elem_info_set_id(info, id);
	if((err = snd_ctl_elem_info(handle, info)) < 0)
	{
		error("Cannot find the given element '%s'\n", ctrl_name);
		goto ctrl_set_exit2;
	}

	snd_ctl_elem_info_get_id(info, id);
	snd_ctl_elem_value_set_id(control, id);
	type = snd_ctl_elem_info_get_type(info);
	switch (type)
	{
		case SND_CTL_ELEM_TYPE_INTEGER:
			snd_ctl_elem_value_set_integer(control, 0, ctrl_val);
			break;

		case SND_CTL_ELEM_TYPE_ENUMERATED:
			snd_ctl_elem_value_set_enumerated(control, 0, ctrl_val);
			break;

		default:
			error("the type of '%s' element is error.\n", ctrl_name);
			err = -1;
			goto ctrl_set_exit2;
	}

	if((err = snd_ctl_elem_write(handle, control)) < 0)
	{
		error("'%s' element write error: %s\n", ctrl_name, snd_strerror(err));
		goto ctrl_set_exit2;
	}

ctrl_set_exit2:
//	free(info);
//	free(id);
//	free(control);
	snd_ctl_close(handle);
ctrl_set_exit1:
	return err;
}


#define CARDNUM	0

static const char short_options[] = "hrfv:i";

static const struct option long_options[] = {
    {"help", no_argument, NULL, 'h'},
    {"rough", no_argument, NULL, 'r'},
    {"fine", no_argument, NULL, 'f'},
    {"vol", required_argument, NULL, 'v'},
    {0, 0, 0, 0}
};

static void usage(FILE * fp, int argc, char ** argv) {   
    fprintf(fp, "Usage: %s [options]/n\n"   
		"Options:\n"
		"-h Print this message\n"
		"-r Rough adjusting sound\n"
		"-f Fine adjusting sound\n"
		"-v Set vol value\n"
		"", argv[0]);   
}

int main(int argc, char *argv[])   
{
	int vol_flag = 0;
	int vol_tune;
	int cur_vol;
	int info_vol_cur;
	int info_vol_range;
	char ctrl_name[128]="NULL";
	
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

			case 'r':
				strcpy(ctrl_name, "SIG Capture Boost");
				break;

			case 'f':
				strcpy(ctrl_name, "SIG Capture PGA");
				break;

			case 'v':
				vol_tune = atoi(optarg);
				if (vol_tune <= 0)
					vol_tune = 0;
				vol_flag = 1;
				break;

			default:
				usage(stderr, argc, argv);
				exit(EXIT_FAILURE);
		}
	}

	if(strcmp(ctrl_name, "NULL") != 0)
	{
		snx_audio_crtl_get_items(CARDNUM, &info_vol_range, ctrl_name);
		snx_audio_ctrl_get(CARDNUM, &info_vol_cur, ctrl_name);
		printf("[SNX-AUDIO] Sigma-delta Vol (%s) range: 0 ~ %d; Current Vol : %d\n", ctrl_name, (info_vol_range-1), info_vol_cur);

		if(vol_flag == 1)
		{
			if(vol_tune > (info_vol_range - 1))
				vol_tune = (info_vol_range - 1);
			snx_audio_ctrl_set(CARDNUM, vol_tune, ctrl_name);
			snx_audio_ctrl_get(CARDNUM, &cur_vol, ctrl_name);
			printf("[SNX-AUDIO] New Sigma-delta Vol (%s) : %d\n", ctrl_name, cur_vol);
		}
	}

    return 0;
}
