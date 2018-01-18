/*
 * Create at 2011/05/06 by yanjie_yang
 */

#ifndef __MACH_MEMORY_H
#define __MACH_MEMORY_H

/*
 * Physical DRAM offset.
 */
#define PHYS_OFFSET	UL(0x00000000)

/*
 * TCM memory whereabouts
 */
#define ITCM_OFFSET	0xffff4000
#define ITCM_END	0xffff7fff
#define DTCM_OFFSET	0xffff8000
#define DTCM_END	0xffff8000

#endif
