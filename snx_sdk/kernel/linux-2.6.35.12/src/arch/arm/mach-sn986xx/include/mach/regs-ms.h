/*
 * linux/include/asm-arm/arch-sn926/regs-ms.h
 *
 * Copyright (c) 2008 SONIX Limited.  All rights reserved.
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
 * Nora 04/21/2009 created
*/

#ifndef __ASM_ARCH_REGS_MS_H
#define __ASM_ARCH_REGS_MS_H

#include <mach/platform.h>

/*
 * SONiX Mass Storage Controller virtual addresses
*/
#define		MS_CTL				IO_ADDRESS(SNX_MS_CTL)
#define 	MS_CTL_MODE			0x00000007
#define 	MS_CTL_REGRW			0x00000008
#define 	MS_CTL_DMAEN          		0x00000010
#define 	MS_CTL_DMARW          		0x00000020
#define 	MS_CTL_EXTRAEN        		0x00000040
#define 	MS_CTL_ECCEN          		0x00000080
#define 	MS_CTL_MSRDY          		0x00000100
#define 	MS_CTL_SPIBUSYTRI		0x00001000
#define 	MS_CTL_SPICMDTRI 		0x00002000
#define 	MS_CTL_RAEDDATACMD      	0x00400000
#define 	MS_CTL_MSSPEED        		0xff000000

#define  	MS_DMA_SIZE			IO_ADDRESS(SNX_MS_DMA_SIZE)
#define 	MS_DMA_SIZE_MSDMASIZE		0x00000fff

#define		MS_CRC1				IO_ADDRESS(SNX_MS_CRC1)			
#define 	MS_CRC1_CRC16_0_L             	0x000000ff
#define 	MS_CRC1_CRC16_0_H             	0x0000ff00
#define 	MS_CRC1_CRC7      	       	0x007f0000

#define		MS_CRC2				IO_ADDRESS(SNX_MS_CRC2)			
#define 	MS_CRC2_CRC16_1         	0x0000ffff

#define		MS_CRC3				IO_ADDRESS(SNX_MS_CRC3)			
#define 	MS_CRC2_CRC16_2		        0x0000ffff
#define 	MS_CRC2_CRC16_3         	0xffff0000

#define		MS_MS_IO_I			IO_ADDRESS(SNX_MS_MS_IO_I)		
#define 	MS_MS_IO_I_I			0x000000ff

#define		MS_MS_IO_O			IO_ADDRESS(SNX_MS_MS_IO_O)		
#define 	MS_MS_IO_O_O                  	0x000000ff

#define		MS_MS_IO_OE			IO_ADDRESS(SNX_MS_MS_IO_OE)		
#define 	MS_MS_IO_OE_OE                	0x000000ff

#define   	MS_SPI_CMD			IO_ADDRESS(SNX_MS_SPI_CMD)		
#define 	MS_SPI_CMD_CMD                	0xffffffff

#define   	MS_SPI_INDEX			IO_ADDRESS(SNX_MS_SPI_INDEX)		
#define 	MS_SPI_INDEX_INDEX 		0x000000ff

#define  	MS_DMA_BLKSU			IO_ADDRESS(SNX_MS_DMA_BLKSU)		
#define 	MS_DMA_BLKSU_BLK        	0x0000ffff
#define 	MS_DMA_BLKSU_SUBLK            	0xffff0000

#define  	MS_TMCNT			IO_ADDRESS(SNX_MS_TMCNT)		
#define 	MS_TMCNT_TMCNT               	0x3fffffff

#define		MS_MDMAECC			IO_ADDRESS(SNX_MS_MDMAECC)		
#define 	MS_MDMAECC_MS_M_DMA_EN    	0x00000001
#define 	MS_MDMAECC_MS_M_DMA_OK    	0x00000002
#define 	MS_MDMAECC_MS_M_DMA_TIME_OUT 	0x00000004
#define 	MS_MDMAECC_CRCERR             	0x00000080
#define		MS_MDMAECC_MS_RDY_INTR_EN 	0x00000100
#define 	MS_MDMAECC_MS_ERR_INTR_EN     	0x00000200
#define 	MS_MDMAECC_DETECT_INTR_EN       0x00000800

#define 	MS_MDMAECC_CRCFAIL            	0x00010000

#define 	MS_MDMAECC_MS_RDY_FLAG		0x00010000
#define 	MS_MDMAECC_MS_ERR_FLAG		0x00020000
#define		MS_MDMAECC_ECC_ERR_FLAG		0x00040000
#define 	MS_MDMAECC_DETECT_FLAG		0x00080000
#define 	MS_MDMAECC_CLR_MS_RDY_FLAG	0x01000000
#define 	MS_MDMAECC_CLR_MS_ERR_FLAG	0x02000000
#define 	MS_MDMAECC_CLR_ECC_ERR_FLAG	0x04000000
#define 	MS_MDMAECC_CLR_DETECT_FLAG	0x08000000

#define  	MS_LBA				IO_ADDRESS(SNX_MS_LBA)			
#define 	MS_LBA_LBAW                   	0x00003e00

#define  	MS_DMA_ADDR	 		IO_ADDRESS(SNX_MS_DMA_ADDR)
#define   	MS_REG_CMD			IO_ADDRESS(SNX_MS_REG_CMD)		
#define   	MS_REG_DATA			IO_ADDRESS(SNX_MS_REG_DATA)		
#define   	MS_REG_DUMMYCLOCK		IO_ADDRESS(SNX_MS_REG_DUMMYCLOCK)
#define   	MS_AUTO_RESPONSE		IO_ADDRESS(SNX_MS_AUTO_RESPONSE)	


/* Mass Storage controller registers offset */
#define		MS_CTL_OFFSET			0x00  
#define		MS_DMA_SIZE_OFFSET		0x04            
#define		MS_CRC1_OFFSET			0x08
#define		MS_CRC2_OFFSET			0x0c
#define		MS_CRC3_OFFSET			0x10

#define		MS_MS_IO_I_OFFSET		0x30
#define		MS_MS_IO_O_OFFSET		0x34
#define		MS_MS_IO_OE_OFFSET		0x38  
#define		MS_SPI_CMD_OFFSET		0x3c  
#define		MS_SPI_INDEX_OFFSET		0x40

#define		MS_DMA_BLKSU_OFFSET		0x48  
#define		MS_TMCNT_OFFSET			0x4c  
#define		MS_MDMAECC_OFFSET		0x50
#define		MS_LBA_OFFSET			0x54  

#define		MS_DMA_ADDR_OFFSET		0x5c

#define		MS_REG_CMD_OFFSET		0x70
#define		MS_REG_DATA_OFFSET		0x74
#define		MS_REG_DUMMYCLOCK_OFFSET	0x78
#define		MS_AUTO_RESPONSE_OFFSET		0x7c

/*
 * SONiX Mass Storage Controller (serial flash) physical addresses
 */
#define SNX_MSREG(x) (SNX_MS1_BASE + (x))

#define		SNX_MS_CTL			SNX_MSREG(MS_CTL_OFFSET)
#define		SNX_MS_DMA_SIZE			SNX_MSREG(MS_DMA_SIZE_OFFSET)
#define		SNX_MS_CRC1			SNX_MSREG(MS_CRC1_OFFSET)
#define		SNX_MS_CRC2			SNX_MSREG(MS_CRC2_OFFSET)
#define		SNX_MS_CRC3			SNX_MSREG(MS_CRC3_OFFSET)
#define		SNX_MS_MS_IO_I			SNX_MSREG(MS_MS_IO_I_OFFSET)
#define		SNX_MS_MS_IO_O			SNX_MSREG(MS_MS_IO_O_OFFSET)
#define		SNX_MS_MS_IO_OE			SNX_MSREG(MS_MS_IO_OE_OFFSET)
#define		SNX_MS_SPI_CMD			SNX_MSREG(MS_SPI_CMD_OFFSET)
#define		SNX_MS_SPI_INDEX		SNX_MSREG(MS_SPI_INDEX_OFFSET)
#define		SNX_MS_DMA_BLKSU		SNX_MSREG(MS_DMA_BLKSU_OFFSET)
#define		SNX_MS_TMCNT			SNX_MSREG(MS_TMCNT_OFFSET)
#define		SNX_MS_MDMAECC			SNX_MSREG(MS_MDMAECC_OFFSET)
#define		SNX_MS_LBA			SNX_MSREG(MS_LBA_OFFSET)
#define		SNX_MS_DMA_ADDR			SNX_MSREG(MS_DMA_ADDR_OFFSET)
#define		SNX_MS_REG_CMD			SNX_MSREG(MS_REG_CMD_OFFSET)
#define		SNX_MS_REG_DATA			SNX_MSREG(MS_REG_DATA_OFFSET)
#define		SNX_MS_REG_DUMMYCLOCK		SNX_MSREG(MS_REG_DUMMYCLOCK_OFFSET)
#define		SNX_MS_AUTO_RESPONSE		SNX_MSREG(MS_AUTO_RESPONSE_OFFSET)

#endif /* __ASM_ARCH_REGS_MS_H */
