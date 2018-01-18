/*
 * Create at 2011/05/06 by yanjie_yang
 */

#ifndef __MACH_IRQS_H
#define __MACH_IRQS_H

/* 
 *  IRQ numbers for interrupt handler 
 */
#define IRQ_NR_BASE		0
#define INT_GPIO			(IRQ_NR_BASE + 0)
#define INT_I2C0			(IRQ_NR_BASE + 1)
#define INT_I2C1			(IRQ_NR_BASE + 2)
#define INT_RTC_ALM 		(IRQ_NR_BASE + 3)
#define INT_RTC_WK 		(IRQ_NR_BASE + 4)
#define INT_TIMER			(IRQ_NR_BASE + 5)
#define INT_WTD 			(IRQ_NR_BASE + 6)
#define INT_SSP0 			(IRQ_NR_BASE + 7)
#define INT_UART0_RS		(IRQ_NR_BASE + 8)
#define INT_UART0_FIR		(IRQ_NR_BASE + 9)
#define INT_UDC              (IRQ_NR_BASE + 9)
#define INT_UART1_RS		(IRQ_NR_BASE + 10)
#define INT_UART1_FIR		(IRQ_NR_BASE + 11)
#define INT_AHBC			(IRQ_NR_BASE + 12)
#define INT_APBC			(IRQ_NR_BASE + 13)
#define INT_DMAC			(IRQ_NR_BASE + 14)
#define INT_MS1  			(IRQ_NR_BASE + 15)
#define INT_H264			(IRQ_NR_BASE + 16)
#define INT_MAC 			(IRQ_NR_BASE + 17)
#define INT_AUDIO_PLY	(IRQ_NR_BASE + 18)
#define INT_AUDIO_REC	(IRQ_NR_BASE + 19)
#define INT_SDIO			(IRQ_NR_BASE + 20)
#define INT_ISP_VS		(IRQ_NR_BASE + 21)
#define INT_ISP_HW		(IRQ_NR_BASE + 22)
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
#define INT_ISP_HS			(IRQ_NR_BASE + 23)
#else
#define INT_VO			(IRQ_NR_BASE + 23)
#endif
#define INT_EHCI			(IRQ_NR_BASE + 24)
#define INT_AES  			(IRQ_NR_BASE + 25)
#define INT_MS2			(IRQ_NR_BASE + 26)
#define INT_MS2CD		(IRQ_NR_BASE + 27)
#define INT_JPEGD		(IRQ_NR_BASE + 28)
#define INT_MS3			(IRQ_NR_BASE + 29)
#define INT_JPEGE			(IRQ_NR_BASE + 30)
#define INT_SSP1		(IRQ_NR_BASE + 31)


/* 
 *  FIQ interrupts definitions are the same the INT definitions.
 */
#define FIQ_NR_BASE		32
//#define INT_LCD			(FIQ_NR_BASE + 2)

#define INT_EXT_BASE		64 

#define NR_IRQS				96
#define SNX_IRQ_NR			32
#define SNX_FIQ_NR			3
#define SNX_EXT_INT_NR		32 /*32 GPIO interrupts*/

#endif
