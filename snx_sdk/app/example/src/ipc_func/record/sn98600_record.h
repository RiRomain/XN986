#ifndef _SN98600_RECORD_H_
#define _SN98600_RECORD_H_

#ifdef __cplusplus
extern "C" {
#endif

/* SD record enable */
#define SD_INSERT_RECORD_EN	1

/* SD record type */
#define SD_REC_SCHED		0
#define SD_REC_ALARM		1



#define DEFAULT_OTHER_USED_LOW_LIMIT_VALUE 200

int get_sd_available(void);
int start_record_loop(void);
void destory_record_loop(void);
void stop_all_record_action(void);


void up_record_count(void);

int get_record_count(void);
void write_done_file(void);
void set_close_trig(int val);

int get_mdrecord_count(void);
void write_done_md_file(void);
void set_md_close_trig(int val);

extern struct save_media_source *sd_record;

#ifdef __cplusplus
}
#endif

#endif

