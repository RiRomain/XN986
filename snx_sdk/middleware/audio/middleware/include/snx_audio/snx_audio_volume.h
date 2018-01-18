#ifndef SNX_AUDIO_VOLUME_H
#define SNX_AUDIO_VOLUME_H

#define SNX_AUDIO_MUTE 		1
#define SNX_AUDIO_UNMUTE	0

#ifdef __cplusplus
extern "C" {
#endif

int snx_audio_mic_vol_get_items(int card_num, int *items);
int snx_audio_mic_vol_set(int card_num, int vol);
int snx_audio_mic_vol_get(int card_num, int *vol);
int snx_audio_mic_vol_set_mute(int card_num, int mute);
int snx_audio_mic_vol_get_mute(int card_num, int *mute);

int snx_audio_spk_vol_get_items(int card_num, int *items);
int snx_audio_spk_vol_set(int card_num, int vol);
int snx_audio_spk_vol_get(int card_num, int *vol);
int snx_audio_spk_vol_set_mute(int card_num, int mute);
int snx_audio_spk_vol_get_mute(int card_num, int *mute);

int snx_audio_r2r_vol_set(int card_num, int vol);
int snx_audio_r2r_vol_get(int card_num, int *vol);

#ifdef __cplusplus
}
#endif

#endif
