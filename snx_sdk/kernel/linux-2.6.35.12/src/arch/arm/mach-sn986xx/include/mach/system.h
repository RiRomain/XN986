/*
 *  Copyright (C) 2008 SONIX Limited
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * Kelvin 03/17/2008 created
 */

#ifndef __MACH_SYSTEM_H
#define __MACH_SYSTEM_H

#include <asm/io.h>

#include <mach/regs-wtd.h>

static inline void arch_idle(void)
{
	/*
	 * This should do all the clock switching
	 * and wait for interrupt tricks
	 */
	cpu_do_idle();
}

static inline void arch_reset(char mode, const char *cmd)
{
	if (mode == 's') {
		/* Jump into ROM at address 0 */
		cpu_reset(0);
	} else {
		/* Initialize the watchdog and let it fire */
		writel(0x0, WTD_CTRL);		/* disable watchdog */
		writel(0x1, WTD_LOAD);		/* initialize watchdog load register */
		writel(0x5ab9, WTD_RST);		/* load watchdog count register */
		writel(0x3, WTD_CTRL);		/* enable reset system & start watchdog */
	}
}

#endif
