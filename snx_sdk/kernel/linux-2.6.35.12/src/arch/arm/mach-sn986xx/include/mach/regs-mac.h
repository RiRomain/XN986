/*
 * Create at 2011/05/06 by yanjie_yang
 */

#ifndef __MACH_REGS_MAC_H
#define __MACH_REGS_MAC_H

#include <mach/platform.h>

#define MAC_ISR			(MAC_ISR_OFFSET)
#define MAC_IER			(MAC_IER_OFFSET)
#define MAC_MADR		(MAC_MADR_OFFSET)
#define MAC_LADR		(MAC_LADR_OFFSET)
#define MAC_MAHT0		(MAC_MAHT0_OFFSET)
#define MAC_MAHT1		(MAC_MAHT1_OFFSET)
#define MAC_TXPD		(MAC_TXPD_OFFSET)
#define MAC_RXPD		(MAC_RXPD_OFFSET)
#define MAC_TXRBADR		(MAC_TXRBADR_OFFSET)
#define MAC_RXRBADR		(MAC_RXRBADR_OFFSET)
#define MAC_ITC			(MAC_ITC_OFFSET)
#define MAC_APTC		(MAC_APTC_OFFSET)
#define MAC_DBLAC		(MAC_DBLAC_OFFSET)
#define MAC_MACCR		(MAC_MACCR_OFFSET)
#define MAC_MACSR		(MAC_MACSR_OFFSET)
#define MAC_PHYCR		(MAC_PHYCR_OFFSET)
#define MAC_PHYWDATA		(MAC_PHYWDATA_OFFSET)
#define MAC_FCR			(MAC_FCR_OFFSET)
#define MAC_BPR			(MAC_BPR_OFFSET)

/*
 * SONiX MAC Controller virtual addresses
 */
#if 0
#define MAC_ISR			IO_ADDRESS(SNX_MAC_ISR)
#define MAC_IER			IO_ADDRESS(SNX_MAC_IER)
#define MAC_MADR		IO_ADDRESS(SNX_MAC_MADR)
#define MAC_LADR		IO_ADDRESS(SNX_MAC_LADR)
#define MAC_MAHT0		IO_ADDRESS(SNX_MAC_MAHT0)
#define MAC_MAHT1		IO_ADDRESS(SNX_MAC_MAHT1)
#define MAC_TXPD		IO_ADDRESS(SNX_MAC_TXPD)
#define MAC_RXPD		IO_ADDRESS(SNX_MAC_RXPD)
#define MAC_TXRBADR		IO_ADDRESS(SNX_MAC_TXRBADR)
#define MAC_RXRBADR		IO_ADDRESS(SNX_MAC_RXRBADR)
#define MAC_ITC			IO_ADDRESS(SNX_MAC_ITC)
#define MAC_APTC		IO_ADDRESS(SNX_MAC_APTC)
#define MAC_DBLAC		IO_ADDRESS(SNX_MAC_DBLAC)
#define MAC_MACCR		IO_ADDRESS(SNX_MAC_MACCR)
#define MAC_MACSR		IO_ADDRESS(SNX_MAC_MACSR)
#define MAC_PHYCR		IO_ADDRESS(SNX_MAC_PHYCR)
#define MAC_PHYWDATA		IO_ADDRESS(SNX_MAC_PHYWDATA)
#define MAC_FCR			IO_ADDRESS(SNX_MAC_FCR)
#define MAC_BPR			IO_ADDRESS(SNX_MAC_BPR)
#endif

/*
 * SONiX MAC Controller physical addresses
 */
#define SNX_MAC_ISR			(SNX_MAC_BASE + MAC_ISR_OFFSET)
#define SNX_MAC_IER			(SNX_MAC_BASE + MAC_IER_OFFSET)
#define SNX_MAC_MADR			(SNX_MAC_BASE + MAC_MADR_OFFSET)
#define SNX_MAC_LADR			(SNX_MAC_BASE + MAC_LADR_OFFSET)
#define SNX_MAC_MAHT0			(SNX_MAC_BASE + MAC_MAHT0_OFFSET)
#define SNX_MAC_MAHT1			(SNX_MAC_BASE + MAC_MAHT1_OFFSET)
#define SNX_MAC_TXPD			(SNX_MAC_BASE + MAC_TXPD_OFFSET)
#define SNX_MAC_RXPD			(SNX_MAC_BASE + MAC_RXPD_OFFSET)
#define SNX_MAC_TXRBADR			(SNX_MAC_BASE + MAC_TXRBADR_OFFSET)
#define SNX_MAC_RXRBADR			(SNX_MAC_BASE + MAC_RXRBADR_OFFSET)
#define SNX_MAC_ITC			(SNX_MAC_BASE + MAC_ITC_OFFSET)
#define SNX_MAC_APTC			(SNX_MAC_BASE + MAC_APTC_OFFSET)
#define SNX_MAC_DBLAC			(SNX_MAC_BASE + MAC_DBLAC_OFFSET)
#define SNX_MAC_MACCR			(SNX_MAC_BASE + MAC_MACCR_OFFSET)
#define SNX_MAC_MACSR			(SNX_MAC_BASE + MAC_MACSR_OFFSET)
#define SNX_MAC_PHYCR			(SNX_MAC_BASE + MAC_PHYCR_OFFSET)
#define SNX_MAC_PHYWDATA		(SNX_MAC_BASE + MAC_PHYWDATA_OFFSET)
#define SNX_MAC_FCR			(SNX_MAC_BASE + MAC_FCR_OFFSET)
#define SNX_MAC_BPR			(SNX_MAC_BASE + MAC_BPR_OFFSET)

/* MAC controller registers offset */
#define MAC_ISR_OFFSET			0x00	/* Interrupt Status Register */
#define MAC_IER_OFFSET			0x04	/* Interrupt Enable Register */
#define MAC_MADR_OFFSET			0x08	/* MAC Most Significant Address Register */
#define MAC_LADR_OFFSET			0x0c	/* MAC Least Significant Address Register */
#define MAC_MAHT0_OFFSET		0x10	/* Multicast Address Hash Table 0 Register */
#define MAC_MAHT1_OFFSET		0x14	/* Multicast Address Hash Table 1 Register */
#define MAC_TXPD_OFFSET			0x18	/* Transmit Poll Demand Register */
#define MAC_RXPD_OFFSET			0x1c	/* Receive Poll Demand Register */
#define MAC_TXRBADR_OFFSET		0x20	/* Transmit Ring Base Address Register */
#define MAC_RXRBADR_OFFSET		0x24	/* Receive Ring Base Address Register */
#define MAC_ITC_OFFSET			0x28	/* Interrupt Timer Control Register */
#define MAC_APTC_OFFSET			0x2c	/* Automatic Polling Timer Control Register */
#define MAC_DBLAC_OFFSET		0x30	/* DMA Burst Length and Arbitration Control Register */
#define MAC_MACCR_OFFSET		0x88	/* MAC Control Register */
#define MAC_MACSR_OFFSET		0x8c	/* MAC Status Register */
#define MAC_PHYCR_OFFSET		0x90	/* PHY Control Register */
#define	MAC_PHYWDATA_OFFSET		0x94	/* PHY Write Data Register */
#define MAC_FCR_OFFSET			0x98	/* Flow Control Register */
#define MAC_BPR_OFFSET			0x9c	/* Back Pressure Register */

/*
 * MAC interrupt bits
 */
#define PHYSTS_CHG		(0x1<<9)     	/* PHY link status change */
#define AHB_ERR			(0x1<<8)     	/* AHB error */
#define RPKT_LOST		(0x1<<7)     	/* Received packet lost due to RX FIFO full */
#define RPKT_OK			(0x1<<6)     	/* Packet received into RX FIFO successfully */
#define XPKT_LOST		(0x1<<5)     	/* Packet transmitted to Ethernet lost due to collision */
#define XPKT_OK			(0x1<<4)     	/* Packet transmitted to Ethernet successfully */
#define NOTXBUF			(0x1<<3)     	/* Transmit buffer not available */
#define XPKT_FINISH		(0x1<<2)     	/* TXDMA has moved data into TX FIFO */
#define NORXBUF			(0x1<<1)     	/* Receive buffer not available */
#define RPKT_FINISH		(0x1)     	/* RXDMA has received packets to RX buffer successfully */

#define MAC_ALLINTRS		(0x3ff)     	/* All interrupts of MAC */

/*
 * MAC control register bits
 */
#define MACCR_SPD		(0x1<<18)     	/* Speed mode: 1:100 Mbps; 0:10 Mbps */
#define MACCR_RX_BRO		(0x1<<17)     	/* Receive broadcast packet */
#define MACCR_RX_MUL		(0x1<<16)     	/* Receive all multicast packet */
#define MACCR_FD		(0x1<<15)     	/* Full duplex mode */
#define MACCR_CRC_APD		(0x1<<14)     	/* Append CRC to packet to be transmitted */
#define MACCR_RX_ALL		(0x1<<12)     	/* Not check destination address of incoming packet */
#define MACCR_RX_FTL		(0x1<<11)     	/* Store incoming packet great than 1518 bytes */
#define MACCR_RX_RUNT		(0x1<<10)     	/* Store incoming packet less than 64 bytes */
#define MACCR_MUL_HT		(0x1<<9)     	/* Store incoming multcast packet */
#define MACCR_RX_EN		(0x1<<8)     	/* Enable Receiver */
#define MACCR_ENRX_IN_HALF	(0x1<<6)     	/* Enale reception when transmitting in half duplex mode*/
#define MACCR_TX_EN		(0x1<<5)     	/* Enable Transmitter */
#define MACCR_CRC_DIS		(0x1<<4)     	/* Disable CRC check when receiving packet */
#define MACCR_LOOP_EN		(0x1<<3)     	/* Internal loop back mode */
#define MACCR_RST		(0x1<<2)     	/* Reset Ethernet MAC controller */
#define MACCR_RXDMA_EN		(0x1<<1)     	/* Enable receive DMA channel */
#define MACCR_TXDMA_EN		(0x1)     	/* Enable transmit DMA channel */

/*
 * MAC status register bits
 */
#define MACSR_COL_EXCEED	(0x1<<11)     	/* Collision amount exceed 16  */
#define MACSR_LATE_COL		(0x1<<10)     	/* Detect a late collision */
#define MACSR_XPKT_LOST		(0x1<<9)     	/* Packet transmitted to Ethernet lost due to collision */
#define MACSR_XPKT_OK		(0x1<<8)     	/* Packet transmitted to Ethernet successfully */
#define MACSR_RUNT		(0x1<<7)     	/* Detect a runt packet */
#define MACSR_FTL		(0x1<<6)     	/* Detect a frame that is too long */
#define MACSR_CRC_ERR		(0x1<<5)     	/* CRC error of incoming packet */
#define MACSR_RPKT_LOST		(0x1<<4)     	/* Received packet lost due to RX FIFO full */
#define MACSR_RPKT_OK		(0x1<<3)     	/* Packet received into RX FIFO successfully */
#define MACSR_COL		(0x1<<2)     	/* Drop packet due to collision */
#define MACSR_BRO		(0x1<<1)     	/* Incoming packet for broadcast address */
#define MACSR_MUL		(0x1)     	/* Incoming packet for multicast address */

/*
 * PHY control register bits
 */
#define PHYCR_MIIWR		(0x1<<27)	/* Write PHY operation */
#define PHYCR_MIIRD		(0x1<<26)	/* Read PHY operation */

/*
 * Flow control register bits
 */
#define FC_EN			(0x1)		/* Flow control mode enable */
/*
 * Back pressure register bits
 */
#define BP_EN			(0x1)		/* Back pressure mode enable */

#endif
