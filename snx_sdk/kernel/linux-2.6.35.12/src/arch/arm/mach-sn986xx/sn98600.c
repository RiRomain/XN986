/*
 * Create at 2011/05/06 by yanjie_yang
 */
#include <linux/init.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>

#include <mach/platform.h>
#include <mach/sn98600.h>

MACHINE_START(SN986XX, "SONiX SN98600 Development Platform")
	.phys_io	= SNX_UART0_BASE,
	.io_pg_offst	= ((IO_ADDRESS(SNX_UART0_BASE)) >> 18) & 0xfffc,
	.boot_params	= 0x00000100,
	.map_io		= sn98600_map_io,
	.init_irq	= sn98600_init_irq,
	.timer		= &snx_timer,
	.init_machine	= sn98600_init,
MACHINE_END
