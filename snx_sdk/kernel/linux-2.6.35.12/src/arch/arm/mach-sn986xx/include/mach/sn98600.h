/*
 * Create at 2011/05/06 by yanjie_yang
 */

#ifndef __MACH_SN98600_H
#define __MACH_SN98600_H

extern void __init sn98600_map_io(void);
extern void __init sn98600_init_irq(void);
extern struct sys_timer snx_timer;
extern void __init sn98600_init(void);

#endif
