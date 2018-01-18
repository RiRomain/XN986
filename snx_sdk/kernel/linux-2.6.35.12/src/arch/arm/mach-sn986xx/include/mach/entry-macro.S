/*
 * Create at 2011/05/06 by yanjie_yang
 */

/*
 * Low-level IRQ helper macros
 */

#include <mach/regs-irq.h>

		/* currently don't need an disable_fiq macro */
		.macro	disable_fiq
		.endm

		.macro  get_irqnr_preamble, base, tmp
		.endm

		.macro  arch_ret_to_user, tmp1, tmp2
		.endm

		.macro	get_irqnr_and_base, irqnr, irqstat, base, tmp
		ldr	\base, =IO_ADDRESS(SNX_INTC_BASE)
		ldr	\irqstat, [\base, #IRQ_FLAG_OFFSET]	@ get masked status
		mov	\irqnr, #0
		teq	\irqstat, #0
		beq	1003f

1001:		tst	\irqstat, #0xf
		bne	1002f
		add	\irqnr, \irqnr, #4
		movs	\irqstat, \irqstat, lsr #4
		bne	1001b
1002:		tst	\irqstat, #1
		bne	1003f
		add	\irqnr, \irqnr, #1
		movs	\irqstat, \irqstat, lsr #1
		bne	1002b
1003:		/* EQ will be set if no irqs pending */

@		clz	\irqnr, \irqstat
@1003:		/* EQ will be set if we reach MAXIRQNUM */
		.endm

		.macro	get_fiqnr_and_base, fiqnr, fiqstat, base, tmp
		ldr	\base, =IO_ADDRESS(SNX_INTC_BASE)
		ldr	\fiqstat, [\base, #FIQ_FLAG_OFFSET]	@ get masked status
		mov	\fiqnr, #32
		teq	\fiqstat, #0
		beq	1003f

1001:		tst	\fiqstat, #0xf
		bne	1002f
		add	\fiqnr, \fiqnr, #4
		movs	\fiqstat, \fiqstat, lsr #4
		bne	1001b
1002:		tst	\fiqstat, #1
		bne	1003f
		add	\fiqnr, \fiqnr, #1
		movs	\fiqstat, \fiqstat, lsr #1
		bne	1002b
1003:		/* EQ will be set if no irqs pending */

@		clz	\fiqnr, \fiqstat
@1003:		/* EQ will be set if we reach MAXIRQNUM */
		.endm

