/*
 * Debugging macro include header
 *
 *  Copyright (C) 1994-1999 Russell King
 *  Moved from linux/arch/arm/kernel/debug.S by Ben Dooks
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

/*
 * Saxen Ko 09/13/2011 created
 */
	
#include <mach/regs-serial.h>

		.macro	addruart,rx,tmp
		mrc	p15, 0, \rx, c1, c0
		tst	\rx, #1				@ MMU enabled ?
		ldreq	\rx, =SNX_UART0_BASE		@ physical base address of UART1
		ldrne	\rx, =IO_ADDRESS(SNX_UART0_BASE)	@ virtual base address of UART1
		.endm

		.macro	senduart,rd,rx
		strb	\rd, [\rx, #RS_DATA]
		.endm

		.macro	waituart,rd,rx
1001:		ldrb	\rd, [\rx, #UART_CONFIG]	@ LSR
		tst	\rd, #TX_RDY_BIT		@ test TX ready
		beq	1001b
		.endm

		.macro	busyuart,rd,rx
		mov	\rd, #0x100
1010:		subs	\rd, \rd, #1
		bne	1010b
		.endm
		
