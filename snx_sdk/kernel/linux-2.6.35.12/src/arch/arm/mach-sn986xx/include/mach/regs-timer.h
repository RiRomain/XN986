/*
 * Create at 2011/05/06 by yanjie_yang
 */

#ifndef __MACH_REGS_TIMER_H
#define __MACH_REGS_TIMER_H

#include <mach/platform.h>

/*
 * SONiX timer controller virtual addresses
 */
#define TIMER1_COUNT		IO_ADDRESS(SNX_TIMER1_COUNT)
#define TIMER1_LOAD		IO_ADDRESS(SNX_TIMER1_LOAD)
#define TIMER1_MATCH1		IO_ADDRESS(SNX_TIMER1_MATCH1)
#define TIMER1_MATCH2		IO_ADDRESS(SNX_TIMER1_MATCH2)

#define TIMER2_COUNT		IO_ADDRESS(SNX_TIMER2_COUNT)
#define TIMER2_LOAD		IO_ADDRESS(SNX_TIMER2_LOAD)
#define TIMER2_MATCH1		IO_ADDRESS(SNX_TIMER2_MATCH1)
#define TIMER2_MATCH2		IO_ADDRESS(SNX_TIMER2_MATCH2)

#define TIMER3_COUNT		IO_ADDRESS(SNX_TIMER3_COUNT)
#define TIMER3_LOAD		IO_ADDRESS(SNX_TIMER3_LOAD)
#define TIMER3_MATCH1		IO_ADDRESS(SNX_TIMER3_MATCH1)
#define TIMER3_MATCH2		IO_ADDRESS(SNX_TIMER3_MATCH2)

#define TIMER_CTRL		IO_ADDRESS(SNX_TIMER_CTRL)
#define TIMER_INTRFLAG		IO_ADDRESS(SNX_TIMER_INTRFLAG)
#define TIMER_INTRMASK		IO_ADDRESS(SNX_TIMER_INTRMASK)

/*
 * SONiX timer controller physical addresses
 */
#define SNX_TIMER1_COUNT		(SNX_TIMER_BASE + TIMER1_COUNT_OFFSET)
#define SNX_TIMER1_LOAD			(SNX_TIMER_BASE + TIMER1_LOAD_OFFSET)
#define SNX_TIMER1_MATCH1		(SNX_TIMER_BASE + TIMER1_MATCH1_OFFSET)
#define SNX_TIMER1_MATCH2		(SNX_TIMER_BASE + TIMER1_MATCH2_OFFSET)

#define SNX_TIMER2_COUNT		(SNX_TIMER_BASE + TIMER2_COUNT_OFFSET)
#define SNX_TIMER2_LOAD			(SNX_TIMER_BASE + TIMER2_LOAD_OFFSET)
#define SNX_TIMER2_MATCH1		(SNX_TIMER_BASE + TIMER2_MATCH1_OFFSET)
#define SNX_TIMER2_MATCH2		(SNX_TIMER_BASE + TIMER2_MATCH2_OFFSET)

#define SNX_TIMER3_COUNT		(SNX_TIMER_BASE + TIMER3_COUNT_OFFSET)
#define SNX_TIMER3_LOAD			(SNX_TIMER_BASE + TIMER3_LOAD_OFFSET)
#define SNX_TIMER3_MATCH1		(SNX_TIMER_BASE + TIMER3_MATCH1_OFFSET)
#define SNX_TIMER3_MATCH2		(SNX_TIMER_BASE + TIMER3_MATCH2_OFFSET)

#define SNX_TIMER_CTRL			(SNX_TIMER_BASE + TIMER_CTRL_OFFSET)
#define SNX_TIMER_INTRFLAG		(SNX_TIMER_BASE + TIMER_INTRFLAG_OFFSET)
#define SNX_TIMER_INTRMASK		(SNX_TIMER_BASE + TIMER_INTRMASK_OFFSET)

/* Timer controller registers offset */
#define TIMER1_COUNT_OFFSET		0x00	/* Timer1 Counter */
#define TIMER1_LOAD_OFFSET		0x04	/* Timer1 Auto Reload Value */
#define TIMER1_MATCH1_OFFSET		0x08	/* Timer1 Match Value1 */
#define TIMER1_MATCH2_OFFSET		0x0c	/* Timer1 Match Value2  */

#define TIMER2_COUNT_OFFSET		0x10	/* Timer2 Counter */
#define TIMER2_LOAD_OFFSET		0x14	/* Timer2 Auto Reload Value */
#define TIMER2_MATCH1_OFFSET		0x18	/* Timer2 Match Value1 */
#define TIMER2_MATCH2_OFFSET		0x1c	/* Timer2 Match Value2 */

#define TIMER3_COUNT_OFFSET		0x20	/* Timer3 Counter */
#define TIMER3_LOAD_OFFSET		0x24	/* Timer3 Auto Reload Value */
#define TIMER3_MATCH1_OFFSET		0x28	/* Timer3 Match Value1 */
#define TIMER3_MATCH2_OFFSET		0x2c	/* Timer3 Match Value2 */

#define TIMER_CTRL_OFFSET		0x30	/* Timer1/2/3 Control Register */
#define TIMER_INTRFLAG_OFFSET		0x34	/* Timer Interrupt State */
#define TIMER_INTRMASK_OFFSET		0x38	/* Timer Interrupt Mask */

/*
 * TIMER_CTRL register bits
 */
#define TM3UPDOWN		(0x1<<11)	/* Timer3 Up/Down Count */
#define TM2UPDOWN		(0x1<<10)	/* Timer2 Up/Down Count */
#define TM1UPDOWN		(0x1<<9)	/* Timer1 Up/Down Count */

#define TM3OFENABLE		(0x1<<8)	/* Timer3 Overflow Interrupt Enable */
#define TM3CLOCK		(0x1<<7)	/* Timer3 Clock Source */
#define TM3ENABLE		(0x1<<6)	/* Timer3 Enable */

#define TM2OFENABLE		(0x1<<5)	/* Timer2 Overflow Interrupt Enable */
#define TM2CLOCK		(0x1<<4)	/* Timer2 Clock Source */
#define TM2ENABLE		(0x1<<3)	/* Timer2 Enable */

#define TM1OFENABLE		(0x1<<2)	/* Timer1 Overflow Interrupt Enable */
#define TM1CLOCK		(0x1<<1)	/* Timer1 Clock Source */
#define TM1ENABLE		(0x1)		/* Timer1 Enable */

/*
 * Timer interrupt bits
 */
#define CLR_TM3OF		(0x1<<17)	/* Clear Timer3 Overflow */
#define CLR_TM3MATCH2		(0x1<<16)	/* Clear Timer3 Match Value2 */
#define CLR_TM3MATCH1		(0x1<<15)	/* Clear Timer3 Match Value1 */

#define CLR_TM2OF		(0x1<<14)	/* Clear Timer2 Overflow */
#define CLR_TM2MATCH2		(0x1<<13)	/* Clear Timer2 Match Value2 */
#define CLR_TM2MATCH1		(0x1<<12)	/* Clear Timer2 Match Value1 */

#define CLR_TM1OF		(0x1<<11)	/* Clear Timer1 Overflow */
#define CLR_TM1MATCH2		(0x1<<10)	/* Clear Timer1 Match Value2 */
#define CLR_TM1MATCH1		(0x1<<9)		/* Clear Timer1 Match Value1 */

#define CLR_TM_INT_STS		(CLR_TM1MATCH1 | CLR_TM1MATCH2 | CLR_TM1OF \
				 | CLR_TM2MATCH1 | CLR_TM2MATCH2 | CLR_TM2OF \
				 | CLR_TM3MATCH1 | CLR_TM3MATCH2 | CLR_TM3OF)

#define TM3OF			(0x1<<8)	/* Timer3 Overflow */
#define TM3MATCH2		(0x1<<7)	/* Timer3 Match Value2 */
#define TM3MATCH1		(0x1<<6)	/* Timer3 Match Value1 */

#define TM2OF			(0x1<<5)	/* Timer2 Overflow */
#define TM2MATCH2		(0x1<<4)	/* Timer2 Match Value2 */
#define TM2MATCH1		(0x1<<3)	/* Timer2 Match Value1 */

#define TM1OF			(0x1<<2)	/* Timer1 Overflow */
#define TM1MATCH2		(0x1<<1)	/* Timer1 Match Value2 */
#define TM1MATCH1		(0x1)		/* Timer1 Match Value1 */

#endif
