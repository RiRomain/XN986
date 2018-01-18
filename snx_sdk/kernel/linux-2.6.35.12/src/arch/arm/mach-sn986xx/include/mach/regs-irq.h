/*
 * Create at 2011/05/06 by yanjie_yang
 */

#ifndef __MACH_REGS_IRQ_H
#define __MACH_REGS_IRQ_H

#include <mach/platform.h>
#include <mach/irqs.h>

/* Interrupt controller registers offset */
#define IRQ_SRC_OFFSET			0x00	/* IRQ Source Register */
#define IRQ_MASK_OFFSET			0x04	/* IRQ Enable Register */
#define IRQ_CLR_OFFSET			0x08	/* IRQ Interrupt Clear Register */
#define IRQ_TRG_MODE_OFFSET		0x0c	/* IRQ Trig Mode Register */
#define IRQ_TRG_LEVEL_OFFSET		0x10	/* IRQ Trig Level Register */
#define IRQ_FLAG_OFFSET			0x14	/* IRQ Status Register */

#define FIQ_SRC_OFFSET			0x20	/* FIQ Source Register */
#define FIQ_MASK_OFFSET			0x24	/* FIQ Enable Register */
#define FIQ_CLR_OFFSET			0x28	/* FIQ Interrupt Clear Register */
#define FIQ_TRG_MODE_OFFSET		0x2c	/* FIQ Trig Mode Register */
#define FIQ_TRG_LEVEL_OFFSET		0x30	/* FIQ Trig Level Register */
#define FIQ_FLAG_OFFSET			0x34	/* FIQ Status Register */

/*
 * SONiX interrupt controller physical addresses
 */
#define SNX_IRQ_SRC			(SNX_INTC_BASE + IRQ_SRC_OFFSET)
#define SNX_IRQ_MASK			(SNX_INTC_BASE + IRQ_MASK_OFFSET)
#define SNX_IRQ_CLR			(SNX_INTC_BASE + IRQ_CLR_OFFSET)
#define SNX_IRQ_TRG_MODE		(SNX_INTC_BASE + IRQ_TRG_MODE_OFFSET)
#define SNX_IRQ_TRG_LEVEL		(SNX_INTC_BASE + IRQ_TRG_LEVEL_OFFSET)
#define SNX_IRQ_FLAG			(SNX_INTC_BASE + IRQ_FLAG_OFFSET)

#define SNX_FIQ_SRC			(SNX_INTC_BASE + FIQ_SRC_OFFSET)
#define SNX_FIQ_MASK			(SNX_INTC_BASE + FIQ_MASK_OFFSET)
#define SNX_FIQ_CLR			(SNX_INTC_BASE + FIQ_CLR_OFFSET)
#define SNX_FIQ_TRG_MODE		(SNX_INTC_BASE + FIQ_TRG_MODE_OFFSET)
#define SNX_FIQ_TRG_LEVEL		(SNX_INTC_BASE + FIQ_TRG_LEVEL_OFFSET)
#define SNX_FIQ_FLAG			(SNX_INTC_BASE + FIQ_FLAG_OFFSET)

/*
 * SONiX interrupt controller virtual addresses
 */
#define IRQ_SRC			IO_ADDRESS(SNX_IRQ_SRC)
#define IRQ_MASK		IO_ADDRESS(SNX_IRQ_MASK)
#define IRQ_CLR			IO_ADDRESS(SNX_IRQ_CLR)
#define IRQ_TRG_MODE		IO_ADDRESS(SNX_IRQ_TRG_MODE)
#define IRQ_TRG_LEVEL		IO_ADDRESS(SNX_IRQ_TRG_LEVEL)
#define IRQ_FLAG		IO_ADDRESS(SNX_IRQ_FLAG)

#define FIQ_SRC			IO_ADDRESS(SNX_FIQ_SRC)
#define FIQ_MASK		IO_ADDRESS(SNX_FIQ_MASK)
#define FIQ_CLR			IO_ADDRESS(SNX_FIQ_CLR)
#define FIQ_TRG_MODE		IO_ADDRESS(SNX_FIQ_TRG_MODE)
#define FIQ_TRG_LEVEL		IO_ADDRESS(SNX_FIQ_TRG_LEVEL)
#define FIQ_FLAG		IO_ADDRESS(SNX_FIQ_FLAG)

/*
 * IRQ bits
 */
#define IRQ_GPIO		(0x1<<INT_GPIO)
#define IRQ_I2C1		(0x1<<INT_I2C1)
#define	IRQ_I2C2		(0x1<<INT_I2C2)
#define IRQ_RTC_ALM 		(0x1<<INT_RTC_ALM)
#define IRQ_RTC_WK 		(0x1<<INT_RTC_WK)
#define IRQ_TIMER		(0x1<<INT_TIMER)
#define IRQ_WTD 		(0x1<<INT_WTD)
#define IRQ_SSP 		(0x1<<INT_SSP0)
#define IRQ_UART1_RS		(0x1<<INT_UART1_RS)
#define IRQ_UART1_FIR		(0x1<<INT_UART1_FIR)
#define IRQ_UART2_RS		(0x1<<INT_UART2_RS)
#define IRQ_UART2_FIR		(0x1<<INT_UART2_FIR)
#define IRQ_AHBC		(0x1<<INT_AHBC)
#define IRQ_APBC		(0x1<<INT_APBC)
#define IRQ_DMAC		(0x1<<INT_DMAC)
#define IRQ_MS1  		(0x1<<INT_MS1)
#define IRQ_H264		(0x1<<INT_H264)
#define IRQ_MAC 		(0x1<<INT_MAC)
#define IRQ_AUDIO_PLY		(0x1<<INT_AUDIO_PLY)
#define IRQ_AUDIO_REC		(0x1<<INT_AUDIO_REC)
#define IRQ_SDIO		(0x1<<INT_SDIO)
#define IRQ_SEN_VS		(0x1<<INT_SEN_VS)
#define IRQ_SEN_HW		(0x1<<INT_SEN_HW)
#define IRQ_MS2  		(0x1<<INT_MS2)
#define IRQ_AES  		(0x1<<INT_AES)
#define IRQ_MS2CD		(0x1<<INT_MS2CD)
#define IRQ_JPEG		(0x1<<INT_JPEG)
#define IRQ_EHCI		(0x1<<INT_EHCI)
#define IRQ_BB			(0x1<<INT_BB)
#define IRQ_BB_ERR		(0x1<<INT_BB_ERR)

/* 
 *  FIQ bits
 */
#define FIQ_LCD			(0x1<<INT_LCD)


#define SNX_IRQ_TRIGGER_MODE		(IRQ_WTD | IRQ_RTC_WK | IRQ_RTC_ALM | IRQ_MS2CD)
#define SNX_IRQ_TRIGGER_LEVEL		0x0
#define SNX_FIQ_TRIGGER_MODE		0x0
#define SNX_FIQ_TRIGGER_LEVEL		0x0

#endif
