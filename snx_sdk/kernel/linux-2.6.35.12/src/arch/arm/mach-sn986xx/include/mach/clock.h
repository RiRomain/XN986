/*
 * Create at 2011/05/06 by yanjie_yang
 */

#ifndef __MACH_CLOCK_H
#define __MACH_CLOCK_H

#include <linux/module.h>

struct clk 
{
	struct list_head	node;
	struct module		*owner;
	struct clk		*parent;
	const char		*name;
	unsigned long		rate;
	unsigned int		rate_raise;
	unsigned int		ratio;
	unsigned int		ratio_addr;
	unsigned int		ratio_start_bit;
	unsigned int		ratio_mask;
	unsigned int		gate;
	unsigned int		gate_addr;
	unsigned int		enabled;
	void			(*enable)(struct clk *clk);
	void			(*disable)(struct clk *clk);
	unsigned int		(*get_ratio)(struct clk *clk);
};

int clk_register(struct clk *clk);
void clk_unregister(struct clk *clk);

#endif
