/*
 * Create at 2011/05/06 by yanjie_yang
 */

#ifndef __MACH_IO_H
#define __MACH_IO_H

#define IO_SPACE_LIMIT 0xffffffff

/*
 * We don't actually have real ISA nor PCI buses, but there is so many
 * drivers out there that might just work if we fake them...
 */
#define __io(a)			((void __iomem *)(a))
#define __mem_pci(a)		(a)

#endif
