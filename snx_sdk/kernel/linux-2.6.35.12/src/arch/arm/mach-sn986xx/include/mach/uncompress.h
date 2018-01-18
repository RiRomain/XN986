/*
 * Create at 2011/05/06 by yanjie_yang
 */
	
#ifndef __MACH_UNCOMPRESS_H
#define __MACH_UNCOMPRESS_H

#include <asm/io.h>

#include <mach/regs-serial.h>

/*
 * This does not append a newline
 */
static inline void putc(int c)
{
	while (!(readl(SNX_UART_LSR) & TX_RDY_BIT))		/* physical address */
		barrier();
	writel(c, SNX_UART_THR);
}

static inline void flush(void)
{
	/* wait for transmission to complete */
//	while (!(readl(SNX_UART_LSR) & LSR_TE))		/* physical address */
	while (!(readl(SNX_UART_LSR) & TX_RDY_BIT))		/* physical address */
		barrier();
}

/*
 * nothing to do
 */
#define arch_decomp_setup()
#define arch_decomp_wdog()

#endif
