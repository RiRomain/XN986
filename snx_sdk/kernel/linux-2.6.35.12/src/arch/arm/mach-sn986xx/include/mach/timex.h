/*
 * Create at 2011/05/06 by yanjie_yang
 */

#ifndef __MACH_TIMEX_H
#define __MACH_TIMEX_H

#include <mach/platform.h>
#include <linux/seq_file.h>

#include "generated/snx_sdk_conf.h"

#ifndef CLOCK_TICK_RATE
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
#define CLOCK_TICK_RATE			(((CONFIG_PLL_CLK / CONFIG_TICK_CLOCK_RATIO)<20000000)?(CONFIG_PLL_CLK / CONFIG_TICK_CLOCK_RATIO):10000000)
#else
#define CLOCK_TICK_RATE			(CONFIG_PLL_CLK / CONFIG_TICK_CLOCK_RATIO)
#endif
#endif

#ifdef CONFIG_SPECIAL_HW_TIMER
typedef void (*timer_handler_t) (unsigned long arg);

int request_hw_timer(void);
int free_hw_timer(int timer_id);
int set_hw_timer_alarm(int timer_id, unsigned int ms, timer_handler_t handler, unsigned long arg);
int get_hw_timer_time(int timer_id, struct timeval* tv);
int enable_hw_timer_measure_mode(int timer_id);
int disable_hw_timer_measure_mode(int timer_id);
int enable_hw_timer(int timer_id);
int disable_hw_timer(int timer_id);
#endif


#ifdef CONFIG_SNX_LOG_TIMESTAMP
void _snx_log_timestamp(int tag);
void _snx_show_timestamp(struct seq_file *f);
int init_timestamp_buf(void);

#define snx_log_timestamp(x)	_snx_log_timestamp(x)
#define snx_show_timestamp(x)	_snx_show_timestamp(x)
#else
#define snx_log_timestamp(x)	do { } while (0)
#define snx_show_timestamp(x)	do { } while (0)

#endif

#endif
