//=======================================================================
// 	Copyright Sonix Technology Corp 2007.  All rights reserved.
//-----------------------------------------------------------------------
// File Name : nand_reg.h
// Function  : Control bit and mask 
// Program   : davian_kuo
// Date      : 2012/05/28
// Version   : 0.0
// History
//   0.0 : 2012/05/28 : davian_kuo : IP verify on ST58600
//=======================================================================
#ifndef __NAND_REG_H__
#define __NAND_REG_H__

#define BASE_MS1					0

#define BIT0			0x01
#define BIT1			0x02
#define BIT2			0x04
#define BIT3			0x08
#define BIT4			0x10
#define BIT5			0x20
#define BIT6			0x40
#define BIT7			0x80
#define BIT8			0x0100
#define BIT9			0x0200
#define BIT10			0x0400
#define BIT11			0x0800
#define BIT12			0x1000
#define BIT13			0x2000
#define BIT14			0x4000
#define BIT15			0x8000
#define BIT16			0x010000
#define BIT17			0x020000
#define BIT18			0x040000
#define BIT19			0x080000
#define BIT20			0x100000
#define BIT21			0x200000
#define BIT22			0x400000
#define BIT23			0x800000
#define BIT24			0x01000000
#define BIT25			0x02000000
#define BIT26			0x04000000
#define BIT27			0x08000000
#define BIT28			0x10000000
#define BIT29			0x20000000
#define BIT30			0x40000000
#define BIT31			0x80000000
//=========================================================================
// Address offset 0x00  (MS1 CTL)
//=========================================================================
#define MS1_CTL_MODE_BIT			0  
#define MS1_CTL_REGRW_BIT			3  
#define MS1_CTL_DMAEN_BIT			4  
#define MS1_CTL_DMARW_BIT			5  
#define MS1_CTL_EXTRAECC_BIT			6  
#define MS1_CTL_ECCEN_BIT			7  
#define MS1_CTL_RDY_BIT				8  
#define MS1_CTL_CMD_NIBBLE_TRG_BIT		11  
#define MS1_CTL_NF_MP_BIT			14  
#define MS1_CTL_RDWIDTH_BIT			18  
#define MS1_CTL_WRWIDTH_BIT			20  
#define MS1_CTL_NF_INFOWR_BIT			23  
#define MS1_CTL_NF_SPEED_BIT				24

#define MS1_CTL_MODE_MASK			(BIT0 | BIT1 | BIT2)  
#define MS1_CTL_REGRW_MASK			(BIT3)  
#define MS1_CTL_DMAEN_MASK			(BIT4)  
#define MS1_CTL_DMARW_MASK			(BIT5)  
#define MS1_CTL_EXTRAECC_MASK			(BIT6)  
#define MS1_CTL_ECCEN_MASK			(BIT7)  
#define MS1_CTL_RDY_MASK			(BIT8)  
#define MS1_CTL_CMD_NIBBLE_TRG_MASK		(BIT11)  
#define MS1_CTL_NF_MP_MASK			(BIT14)  
#define MS1_CTL_RDWIDTH_MASK			(BIT18 | BIT19)  
#define MS1_CTL_WRWIDTH_MASK			(BIT20 | BIT21)  
#define MS1_CTL_NF_INFOWR_MASK			(BIT23)  
#define MS1_CTL_NF_SPEED_MASK			(BIT24 | BIT25| BIT26 | BIT27|BIT28|BIT29|BIT30|BIT31)

//=========================================================================
// Address offset 0x04  (MS1 DMA SIZE)
//=========================================================================
#define MS1_DMA_SIZE_BIT			0  

#define MS1_DMA_SIZE_MASK			(BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7 | BIT8 | BIT9 | \
						 BIT10 | BIT11)

						
//=========================================================================
// Address offset 0x30  (MS1 GPIO I)
//=========================================================================
#define MS1_GPIO_I_MASK				(BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7 | BIT8 | BIT9 | \
						 BIT10 | BIT11 | BIT12 | BIT13 | BIT14)

//=========================================================================
// Address offset 0x34  (MS1 GPIO O)
//=========================================================================
#define NAND_CMD_IO_CE_BIT			14


#define NAND_CMD_IO_CE_MASK			(BIT14)
#define MS1_GPIO_O_MASK				(BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7 | BIT8 | BIT9 | \
						 BIT10 | BIT11 | BIT12 | BIT13 | BIT14 )
//=========================================================================
// Address offset 0x38  (MS1 GPIO OE)
//=========================================================================
#define MS1_GPIO_OE_MASK			(BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7 | BIT8 | BIT9 | \
						 BIT10 | BIT11 | BIT12 | BIT13 | BIT14 )

						
//=========================================================================
// Address offset 0x44  (NAND CMD)
//=========================================================================
#define NAND_ODD_CMD0_BIT			0
#define NAND_ODD_CMD1_BIT			2
#define NAND_EVEV_CMD0_BIT			4
#define NAND_EVEN_CMD1_BIT			6
#define NAND_CMD_NIBBLE_BIT			8
#define NAND_CMD_ADR_CYC_BIT			11
#define NAND_CMD_RD_CMD_CNT_BIT			13
#define NAND_CMD_PG_SIZE_BIT			14
#define NAND_CMD_BLK_SIZE_BIT			16
#define NAND_CMD_LBA_INC_BIT			18
#define NAND_READ_STATUS_CMD_BIT		24

#define NAND_ODD_CMD0_MASK			(BIT0)
#define NAND_ODD_CMD1_MASK			(BIT2)
#define NAND_EVEV_CMD0_MASK			(BIT4)
#define NAND_EVEN_CMD1_MASK			(BIT6)
#define NAND_CMD_NIBBLE_MASK			(BIT8 | BIT9 | BIT10)	
#define NAND_CMD_ADR_CYC_MASK			(BIT11 | BIT12)
#define NAND_CMD_RD_CMD_CNT_MASK		(BIT13)
#define NAND_CMD_PG_SIZE_MASK			(BIT14 |BIT15)
#define NAND_CMD_BLK_SIZE_MASK			(BIT16 | BIT17)
#define NAND_CMD_LBA_INC_MASK			(BIT18 | BIT19 | BIT20 | BIT21)		
#define NAND_READ_STATUS_CMD_MASK		(BIT24 | BIT25 | BIT26 | BIT27 | BIT28 | BIT29 | BIT30 | BIT31) 
						
//=========================================================================
// Address offset 0x48  (MS1 DMA BLOCK)
//=========================================================================
#define MS1_DMA_BLOCK_BIT			0 

#define MS1_DMA_BLOCK_MASK			(BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7 | BIT8 | BIT9 | \
						 BIT10 | BIT11 | BIT12 | BIT13 | BIT14 | BIT15)

//=========================================================================
// Address offset 0x4c  (MS1 TIME COUNT)
//=========================================================================
#define MS1_TIME_CNT_BIT			0 

#define MS1_TIME_CNT_MASK			(BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7 | BIT8 | BIT9 | \
						 BIT10 | BIT11 | BIT12 | BIT13 | BIT14 | BIT15 | BIT16 | BIT17 | BIT18 | \
						 BIT19 | BIT20 | BIT21 | BIT22 | BIT23 | BIT24 | BIT25 | BIT26 | BIT27 | \
						 BIT28 | BIT29) 

//=========================================================================
// Address offset 0x50  (MS1 MDMA ECC)
//=========================================================================
#define MS1_MDMAEN_BIT				0
#define MS1_MDMAOK_BIT				1
#define MS1_MDMATIMEOUT_BIT			2
#define MS1_NF_PRG_ERR_BIT			3
#define MS1_NF_INFO_ERR_BIT			4
#define MS1_NF_ERASE_ERR_BIT			5
#define MS1_CRCERR_BIT				6
#define MS1_ECCERR_BIT				7
#define MS1_RDYEN_BIT				8
#define MS1_ERREN_BIT				9
#define MS1_ECCEN_BIT				10
#define MS1_RDYFLAG_BIT				16
#define MS1_ERRFLAG_BIT				17
#define MS1_ECCFLAG_BIT				18
#define MS1_CLR_RDY_BIT				24
#define MS1_CLR_ERR_BIT				25
#define MS1_CLR_ECC_BIT				26

#define MS1_MDMAEN_MASK				(BIT0)
#define MS1_MDMAOK_MASK				(BIT1)
#define MS1_MDMATIMEOUT_MASK			(BIT2)
#define MS1_NF_PRG_ERR_MASK			(BIT3)
#define MS1_NF_INFO_ERR_MASK			(BIT4)
#define MS1_NF_ERASE_ERR_MASK			(BIT5)
#define MS1_CRCERR_MASK				(BIT6)
#define MS1_ECCERR_MASK				(BIT7)
#define MS1_RDYEN_MASK				(BIT8)
#define MS1_ERREN_MASK				(BIT9)
#define MS1_ECCEN_MASK				(BIT10)
#define MS1_RDYFLAG_MASK			(BIT16)
#define MS1_ERRFLAG_MASK			(BIT17)
#define MS1_ECCFLAG_MASK			(BIT18)
#define MS1_CLR_RDY_MASK			(BIT24)
#define MS1_CLR_ERR_MASK			(BIT25)
#define MS1_CLR_ECC_MASK			(BIT26)

//=========================================================================
// Address offset 0x54  (NF LBA_w)
//=========================================================================

//=========================================================================
// Address offset 0x58 (NF LBA_R)
//=========================================================================
#define NAND_SIZELBA_LBA_BIT			0		

#define NAND_SIZELBA_LBA_MASK			(BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7 | BIT8 | BIT9 | \
						 BIT10 | BIT11 | BIT12 | BIT13 | BIT14 | BIT15 | BIT16 | BIT17 | BIT18 | \
						 BIT19 | BIT20 | BIT21 | BIT22) 


//=========================================================================
// Register address offset.
//=========================================================================
#define MS1_CTL				(BASE_MS1)
#define MS1_DMA_SIZE			(BASE_MS1 + 0x04)
#define MS1_GPIO_I			(BASE_MS1 + 0x30)
#define MS1_GPIO_O			(BASE_MS1 + 0x34)
#define MS1_GPIO_OE			(BASE_MS1 + 0x38)
#define MS1_SPI_CMD			(BASE_MS1 + 0x3c)
#define MS1_SPI_IDX			(BASE_MS1 + 0x40)
#define MS1_NF_CMD			(BASE_MS1 + 0x44)
#define MS1_DMA_BLKSU			(BASE_MS1 + 0x48)
#define MS1_TIME_CNT			(BASE_MS1 + 0x4c)
#define MS1_MDMAECC			(BASE_MS1 + 0x50)
#define MS1_LBA_R			(BASE_MS1 + 0x58)
#define MS1_DMAADDR			(BASE_MS1 + 0x5c)
#define MS1_REG_CMD			(BASE_MS1 + 0x70)
#define MS1_REG_ADDR			(BASE_MS1 + 0x74)
#define MS1_REG_DATA			(BASE_MS1 + 0x78)

enum ms1_mode
{
	GPIO_MODE = 0,
 	NF_MODE,
 	SD_MODE,
 	SF_MODE
};

//=========================================================================
// MS1 register r/w mode list.
//=========================================================================
enum ms1_reg_rw_mode
{
	WRITE_MODE = 0,
	READ_MODE
};

enum nibble_cmd
{
	NAND_NIBBLE_RESET = 0,
	NAND_NIBBLE_MERASE,
	NAND_NIBBLE_ERASE,
	NAND_NIBBLE_READINFO
};


#define NAND_WRITE_MODE		0
#define NAND_READ_MODE		1

#define NAND_DMA_READ			1
#define NAND_DMA_WRITE		0

#define NAND_DMA_ENABLE		1
#define NAND_DMA_DISABLE		0

#define NAND_ECC_DISABLE		NAND_DMA_DISABLE
#define NAND_ECC_ENABLE		NAND_DMA_ENABLE	

#define NAND_EXTRA_ENABLE		NAND_DMA_ENABLE
#define NAND_EXTRA_DISABLE	NAND_DMA_DISABLE


#define NAND_MDMA_ENABLE		NAND_DMA_ENABLE
#define NAND_MDMA_DISABLE		NAND_DMA_DISABLE



#endif	/* __NAND_REG_H__  */
