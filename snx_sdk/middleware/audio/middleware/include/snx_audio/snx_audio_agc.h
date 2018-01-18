#ifndef SNX_AUDIO_AGC_H
#define SNX_AUDIO_AGC_H

#ifdef __cplusplus
extern "C" {
#endif

int SNX_AGC_Process(short *inputbuf);
void SNX_AGC_Process_init(int AGC_gain_max, int AGC_gain_min, int AGC_gain_default, int AGC_Dynamic_gain_max, int AGC_Dynamic_gain_min, int AGC_Dynamic_Target_Level_Low, int AGC_Dynamic_Target_Level_High, int AGC_Dynamic_updateSpeed, int AGC_bufsize, int sample_rate, int peakamp_thres, int peakcnt_thres, int response_up_step, int response_down_step);
float Cal_dB(int sample_rate, short* input, int samples);

#ifdef __cplusplus
}
#endif

#endif
