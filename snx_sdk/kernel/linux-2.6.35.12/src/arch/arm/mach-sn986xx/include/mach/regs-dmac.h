
 /** \file linux/include/asm-arm/arch-sn926/regs-dmac.h 
   * \n Functions in this file are show : 
   * \n 1.ST58200 Uart Control registers offset address 
   * \n 2.Control registers' bit operation 
   * \n Copyright (c) 2008 SONIX Limited.  All rights reserved.
   * \n 
   * \author yanjie_yang, Qingbin Li 
   * \date   2010-10-26 */


#ifndef __ASM_ARCH_REGS_DMAC_H
#define __ASM_ARCH_REGS_DMAC_H

#include <mach/platform.h>

/** * \defgroup REGS_DMAC_H_G1 ST58200 DMAC Registers address offset 
  * Defined helper macro of the dma global register's offset address  
  * \n and register's bits operations 
  * @{ */
#define DMA_INT						0x0 /*!< offset address of Interrupt status register */
#define DMA_INT_TC					0x4 /*!< offset address of Interrupt for terminal count status register */
#define DMA_INT_TC_CLR				0x8 /*!< offset address of Interrupt for terminal count clear register */
#define DMA_INT_ERRABT				0xC /*!< offset address of Interrupt for Error/Abort status register */
#define DMA_INT_ERRABT_CLR			0x10 /*!< offset address of Interrupt for Error/Abort clear register */
#define DMA_TC						0x14 /*!< offset address of Terminal count status register */
#define DMA_ERRABT					0x18 /*!< offset address of Error/Abort status register */
#define DMA_CH_EN					0x1C /*!< offset address of Channel enable status register */
#define DMA_CH_BUSY				0x20 /*!< offset address of Channel busy register status register */
#define DMA_CSR						0x24 /*!< offset address of Main configuration status register */
#define DMA_SYNC					0x28 /*!< offset address of Sync register */
#define DMAC_REVISION				0x30 /*!< offset address of DMAC revision register*/
#define DMAC_FEATURE				0x34 /*!< offset address of DMAC feature register */
/** @} */


/** * \defgroup REGS_DMAC_H_G2 ST58200 DMAC Channel base address offset 
  * Defined helper macro of the dma channel base address offset  
  * \n and register's bits operations 
  * @{ */
#define DMA_CHANNEL_OFFSET		0x20   /*!<  channel address offset size*/
#define DMA_CHANNEL0_BASE			0x100 /*!<  channel 0 address offset*/
#define DMA_CHANNEL1_BASE			0x120 /*!<  channel 1 address offset*/
#define DMA_CHANNEL2_BASE			0x140 /*!<  channel 2 address offset*/
#define DMA_CHANNEL3_BASE			0x160 /*!<  channel 3 address offset*/
#define DMA_CHANNEL4_BASE			0x180 /*!<  channel 4 address offset*/
#define DMA_CHANNEL5_BASE			0x1a0 /*!<  channel 5 address offset*/
#define DMA_CHANNEL6_BASE			0x1c0 /*!<  channel 6 address offset*/
#define DMA_CHANNEL7_BASE			0x1e0 /*!<  channel 7 address offset*/
/** @} */


/** * \defgroup REGS_DMAC_H_G3 ST58200 DMAC Channel Registers address offset 
  * Defined helper macro of the dma channel register's address offset  
  * \n and register's bits operations 
  * @{ */
#define DMA_CHANNEL_CSR_OFFSET		0x0 /*!<  channel Channel Control Register*/
#define DMA_CHANNEL_CFG_OFFSET		0x4 /*!<  channel Channel Configuration Register*/
#define DMA_CHANNEL_SRCADDR_OFFSET	0x8 /*!<  channel source address register offset*/
#define DMA_CHANNEL_DSTADDR_OFFSET	0xc /*!<  channel destination address register offset*/
#define DMA_CHANNEL_LLP_OFFSET		0x10 /*!<  channel lld address register offset*/
#define DMA_CHANNEL_SIZE_OFFSET		0x14 /*!<  channel transfer size register address offset*/
/** @} */

/** * \defgroup REGS_DMAC_H_G4 ST58200 DMAC CSR
  * Defined helper macro of bit mapping of main configuration status register(CSR)  
  * \n and bit mapping of channel control register (Cn_CSR)
  * @{ */
#define DMA_CSR_M1ENDIAN				0x00000004 /*!<  master 1 endian*/
#define DMA_CSR_M0ENDIAN				0x00000002 /*!<  master 0 endian*/
#define DMA_CSR_DMACEN				0x00000001 /*!<  enable bit*/

#define DMA_CSR_TC_MSK					0x80000000 /*!<  Terminal count status mask for current transaction*/
#define DMA_CSR_CHPRJ_HIGHEST			0x00C00000 /*!<  Channel priority level*/
#define DMA_CSR_CHPRJ_2ND				0x00800000 /*!<  Channel priority level*/
#define DMA_CSR_CHPRJ_3RD				0x00400000 /*!<  Channel priority level*/
#define DMA_CSR_PRTO3					0x00200000 /*!<  Protection information for cacheability*/
#define DMA_CSR_PRTO2					0x00100000 /*!<  Protection information for cacheability*/
#define DMA_CSR_PRTO1					0x00080000 /*!<  Protection information for cacheability*/
#define DMA_CSR_SRC_BURST_SIZE_1		0x00000000 /*!<  burst size selection 1*/
#define DMA_CSR_SRC_BURST_SIZE_4		0x00010000 /*!<  burst size selection 4*/
#define DMA_CSR_SRC_BURST_SIZE_8		0x00020000 /*!<  burst size selection 8*/
#define DMA_CSR_SRC_BURST_SIZE_16		0x00030000 /*!<  burst size selection 16*/
#define DMA_CSR_SRC_BURST_SIZE_32		0x00040000 /*!<  burst size selection 32*/
#define DMA_CSR_SRC_BURST_SIZE_64		0x00050000 /*!<  burst size selection 64*/
#define DMA_CSR_SRC_BURST_SIZE_128	0x00060000 /*!<  burst size selection 128*/
#define DMA_CSR_SRC_BURST_SIZE_256	0x00070000 /*!<  burst size selection 256*/

#define DMA_CSR_ABT					0x00008000 /*!<  Transaction abort*/
#define DMA_CSR_SRC_WIDTH_8			0x00000000 /*!<  transfer width 8 bits*/
#define DMA_CSR_SRC_WIDTH_16			0x00000800 /*!<  transfer width 16  bits*/
#define DMA_CSR_SRC_WIDTH_32			0x00001000 /*!<  transfer width 32  bits*/

#define DMA_CSR_DST_WIDTH_8			0x00000000 /*!<  transfer width 8 bits*/
#define DMA_CSR_DST_WIDTH_16			0x00000100 /*!<  transfer width 16 bits*/
#define DMA_CSR_DST_WIDTH_32			0x00000200 /*!<  transfer width 32 bits*/

#define DMA_CSR_MODE_NORMAL			0x00000000 /*!<  normal operate mode*/
#define DMA_CSR_MODE_HANDSHAKE		0x00000080 /*!<  handshake operate mode*/

#define DMA_CSR_SRC_INCREMENT			0x00000000 /*!<  Source address control increment*/
#define DMA_CSR_SRC_DECREMENT		0x00000020 /*!<  Source address control decrement*/
#define DMA_CSR_SRC_FIX				0x00000040 /*!<  Source address control fix*/

#define DMA_CSR_DST_INCREMENT			0x00000000 /*!<  Destination Address Control increment*/
#define DMA_CSR_DST_DECREMENT		0x00000008 /*!<  Destination Address Control decrement*/
#define DMA_CSR_DST_FIX				0x00000010 /*!<  Destination Address Control fix*/

#define DMA_CSR_SRC_SEL				0x00000004 /*!<  source master selection*/
#define DMA_CSR_DST_SEL				0x00000002 /*!<  destination master selection*/
#define DMA_CSR_CH_ENABLE				0x00000001 /*!<  Channel Enable*/	 

#define DMA_CSR_CHPR1					0x00C00000 /*!<  Channel priority level mask*/
#define DMA_CSR_SRC_SIZE				0x00070000 /*!<  Channel source size mask*/
#define DMA_CSR_SRC_WIDTH				0x00003800 /*!<  Channel source width size mask*/
#define DMA_CSR_DST_WIDTH				0x00000700 /*!<  Channel destination width size mask*/	 
#define DMA_CSR_SRCAD_CTL				0x00000060 /*!<  Channel souce address control mask*/
#define DMA_CSR_DSTAD_CTL				0x00000018 /*!<  Channel destination address control mask*/
/** @} */


/* bit mapping of channel configuration register */
#define DMA_CFG_INT_ERR_MSK_Disable	0x00000000 /*!<  interrupt error mask*/
#define DMA_CFG_INT_TC_MSK_Disable	0x00000000 /*!<  interrupt  terminal count  mask*/

/** * \defgroup REGS_DMAC_H_G5 ST58200 DMAC Linked List Control Descriptor 
  * Defined helper macro of bit mapping of Linked List Control Descriptor  
  * \n and register's bits operations 
  * @{ */

#define DMA_LLP_TC_MSK					0x10000000 /*!<  Terminal count status mask for current transaction*/

#define DMA_LLP_SRC_WIDTH_8			0x00000000 /*!<  llp source width 8 bits*/
#define DMA_LLP_SRC_WIDTH_16			0x02000000 /*!<  llp source width 16 bits*/
#define DMA_LLP_SRC_WIDTH_32			0x04000000 /*!<  llp source width 32 bits*/

#define DMA_LLP_DST_WIDTH_8			0x00000000 /*!<  llp destination width 8 bits*/
#define DMA_LLP_DST_WIDTH_16			0x00400000 /*!<  llp destination width 16 bits*/
#define DMA_LLP_DST_WIDTH_32			0x00800000 /*!<  llp destination width 32 bits*/

#define DMA_LLP_SRC_INCREMENT			0x00000000 /*!<  llp souce address increment*/
#define DMA_LLP_SRC_DECREMENT			0x00100000 /*!<  llp souce address decrement*/
#define DMA_LLP_SRC_FIX				0x00200000 /*!<  llp souce address fix*/

#define DMA_LLP_DST_INCREMENT			0x00000000 /*!<  llp destination address increment*/
#define DMA_LLP_DST_DECREMENT			0x00040000 /*!<  llp destination address decrement*/
#define DMA_LLP_DST_FIX				0x00080000 /*!<  llp destination address fix*/

#define DMA_LLP_SRC_SEL				0x00020000 /*!<  LLP source master selection*/
#define DMA_LLP_DST_SEL				0x00010000 /*!<  LLP destination master selection*/

/** @} */

#endif
