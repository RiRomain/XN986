#ifndef _SN98600_RECORD_AUDIO_H_
#define _SN98600_RECORD_AUDIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sn98600_audio_ctrl.h"

int snx98600_record_audio_start(struct sonix_audio *audio);
int snx98600_record_audio_stop(struct sonix_audio *audio);
void snx98600_record_audio_free(struct sonix_audio * audio);
struct sonix_audio *snx98600_record_audio_new(char *filename, sonix_audio_cb cb, void *cbarg);

#ifdef __cplusplus
}
#endif


#endif


