#ifndef SNX_AEC_H
#define SNX_AEC_H


#ifdef __cplusplus 
extern "C" {
#endif

void SNX_AEC_Process(short *indata, short *refdata, short *outdata);
void SNX_DelayTracking(short *delayFrame);
int SNX_AEC_Start (char* buf, int size);
void SNX_AEC_Init(short indatasize, short refdatasize, short outdatasize);
void SNX_AEC_Close(void);
unsigned int SNX_AEC_set_gain_min(unsigned int gain);

void mic_tx_ctl_init(int rate);
void mic_tx_ctl(int num_frames);
void pb_rx_ctl_init(int rate);
void pb_rx_ctl(int num_frames);

#ifdef __cplusplus 
}
#endif

#endif //SNX_AEC_H
