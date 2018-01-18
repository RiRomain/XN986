
#ifndef __MS_NADD_FLASH_H__ 
#define __MS_NADD_FLASH_H__ 

#include "common.h"

#define BIT0  (1<<0)
#define BIT1  (1<<1)
#define BIT2  (1<<2)
#define BIT3  (1<<3)
#define BIT4  (1<<4)
#define BIT5  (1<<5)
#define BIT6  (1<<6)
#define BIT7  (1<<7)
#define BIT8  (1<<8)
#define BIT9  (1<<9)
#define BIT10 (1<<10)
#define BIT11 (1<<11)
#define BIT12 (1<<12)
#define BIT13 (1<<13)
#define BIT14 (1<<14)
#define BIT15 (1<<15)
#define BIT16 (1<<16)
#define BIT17 (1<<17)
#define BIT18 (1<<18)
#define BIT19 (1<<19)
#define BIT20 (1<<20)
#define BIT21 (1<<21)
#define BIT22 (1<<22)
#define BIT23 (1<<23)
#define BIT24 (1<<24)
#define BIT25 (1<<25)
#define BIT26 (1<<26)
#define BIT27 (1<<27)
#define BIT28 (1<<28)
#define BIT29 (1<<29)
#define BIT30 (1<<30)
#define BIT31 (1<<31)

//=========================================================================
// Address offset 0x00  (MS1 CTL)
//=========================================================================
#define MS1_CTL_MODE_BIT			0  
#define MS1_CTL_REGRW_BIT			3  
#define MS1_CTL_DMAEN_BIT			4  
#define MS1_CTL_DMARW_BIT			5  
#define MS1_CTL_EXTRAECC_BIT		6  
#define MS1_CTL_ECCEN_BIT			7  
#define MS1_CTL_RDY_BIT				8  
#define MS1_CTL_CMD_NIBBLE_TRG_BIT	11  
#define MS1_CTL_NF_MP_BIT			14  
#define MS1_CTL_RDWIDTH_BIT			18  
#define MS1_CTL_WRWIDTH_BIT			20  
#define MS1_CTL_NF_INFOWR_BIT		23  

#define MS1_CTL_MODE_MASK			(BIT0 | BIT1 | BIT2)  
#define MS1_CTL_REGRW_MASK			(BIT3)  
#define MS1_CTL_DMAEN_MASK			(BIT4)  
#define MS1_CTL_DMARW_MASK			(BIT5)  
#define MS1_CTL_EXTRAECC_MASK		(BIT6)  
#define MS1_CTL_ECCEN_MASK			(BIT7)  
#define MS1_CTL_RDY_MASK			(BIT8)  
#define MS1_CTL_CMD_NIBBLE_TRG_MASK	(BIT11)  
#define MS1_CTL_NF_MP_MASK			(BIT14)  
#define MS1_CTL_RDWIDTH_MASK		(BIT18 | BIT19)  
#define MS1_CTL_WRWIDTH_MASK		(BIT20 | BIT21)  
#define MS1_CTL_NF_INFOWR_MASK		(BIT23)  

//=========================================================================
// Address offset 0x04  (MS1 DMA SIZE)
//=========================================================================
#define MS1_DMA_SIZE_BIT		0  
#define MS1_DMA_SIZE_MASK		(BIT0  | BIT1 | BIT2 | BIT3 | BIT4 | \
								 BIT5  | BIT6 | BIT7 | BIT8 | BIT9 | \
						 		 BIT10 | BIT11)

//=========================================================================
// Address offset 0x08
//=========================================================================
#define NAND_GPIO_O_BIT			0  
#define NAND_GPIO_OE_BIT		10  
#define NAND_GPIO_I_BIT			20  

#define NAND_GPIO_O_MASK		(BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | \
								 BIT5 | BIT6 | BIT7 | BIT8 | BIT9) 	
								 
#define NAND_GPIO_OE_MASK		(BIT10 | BIT11 | BIT12 | BIT13 | BIT14 | \
								 BIT15 | BIT16 | BIT17 | BIT18 | BIT19)
								 
#define NAND_GPIO_I_MASK		(BIT20 | BIT21 | BIT22 | BIT23 | BIT24 | \
								 BIT25 | BIT26 | BIT27 | BIT28 | BIT29)
 
//=========================================================================
// Address offset 0x0c
//=========================================================================
#define NAND_CLR_AV_EMPTY_BIT			0
#define NAND_CLR_INFO_OVF_PART1_BIT		1

#define NAND_CLR_AV_EMPTY_MASK			(BIT0)
#define NAND_CLR_INFO_OVF_PART1_MASK	(BIT1  | BIT2  | BIT3  | BIT4  | BIT5  | \
									     BIT6  | BIT7  | BIT8  | BIT9  | BIT10 | \
									     BIT11 | BIT12 | BIT13 | BIT14 | BIT15 | \
									     BIT16 | BIT17 | BIT18 | BIT19 | BIT20 | \
									     BIT21 | BIT22 | BIT23 | BIT24 | BIT25 | \
									     BIT26 | BIT27 | BIT28 | BIT29 | BIT30 | \
									     BIT31)  

//=========================================================================
// Address offset 0x10
//=========================================================================
#define NAND_CLR_INFO_OVF_PART2_BIT		0
#define NAND_CLR_DATA_OVF_PART1_BIT		10
							
#define NAND_CLR_INFO_OVF_PART2_MASK	(BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | \
										 BIT6 | BIT7 | BIT8 | BIT9) 
										 
#define NAND_CLR_DATA_OVF_PART1_MASK	(BIT10 | BIT11 | BIT12 | BIT13 | BIT14 | \
										 BIT15 | BIT16 | BIT17 | BIT18 | BIT19 | \
										 BIT20 | BIT21 | BIT22 | BIT23 | BIT24 | \
										 BIT25 | BIT26 | BIT27 | BIT28 | BIT29 | \
										 BIT30 | BIT31) 

//=========================================================================
// Address offset 0x14
//=========================================================================
#define NAND_CLR_DATA_OVF_PART2_BIT		0
#define NAND_CLR_CRC_ERROR_BIT			19
						
#define NAND_CLR_DATA_OVF_PART2_MASK	(BIT0  | BIT1  | BIT2  | BIT3  | BIT4  | BIT5  | \
										 BIT6  | BIT7  | BIT8  | BIT9  | BIT10 | BIT11 | \
										 BIT12 | BIT13 | BIT14 | BIT15 | BIT16 | BIT17 | \
										 BIT18)
										 
#define NAND_CLR_CRC_ERROR_MASK			(BIT19)
						
//=========================================================================
// Address offset 0x18
//=========================================================================
#define NAND_AV_EMPTY_BIT			0
#define NAND_INFO_OVF_PART1_BIT		1
						
#define NAND_AV_EMPTY_MASK			(BIT0)

#define NAND_INFO_OVF_PART1_MASK	(BIT1  | BIT2  | BIT3  | BIT4  | BIT5  | BIT6  | \
									 BIT7  | BIT8  | BIT9  | BIT10 | BIT11 | BIT12 | \
									 BIT13 | BIT14 | BIT15 | BIT16 | BIT17 | BIT18 | \
									 BIT19 | BIT20 | BIT21 | BIT22 | BIT23 | BIT24 | \
									 BIT25 | BIT26 | BIT27 | BIT28 | BIT29 | BIT30 | \
									 BIT31)  

//=========================================================================
// Address offset 0x1c
//=========================================================================
#define NAND_INFO_OVF_PART2_BIT		0
#define NAND_DATA_OVF_PART1_BIT		10
							
#define NAND_INFO_OVF_PART2_MASK    (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | \
									 BIT7 | BIT8 | BIT9)
									 
#define NAND_DATA_OVF_PART1_MASK	(BIT10 | BIT11 | BIT12 | BIT13 | BIT14 | BIT15 | \
									 BIT16 | BIT17 | BIT18 | BIT19 | BIT20 | BIT21 | \
									 BIT22 | BIT23 | BIT24 | BIT25 | BIT26 | BIT27 | \
									 BIT28 | BIT29 | BIT30 | BIT31) 
							
//=========================================================================
// Address offset 0x20
//=========================================================================
#define NAND_DATA_OVF_PART2_BIT		0
#define NAND_CRC_ERROR_BIT			19
						
						
#define NAND_DATA_OVF_PART2_MASK	(BIT0  | BIT1  | BIT2  | BIT3  | BIT4  | BIT5  | \
									 BIT6  | BIT7  | BIT8  | BIT9  | BIT10 | BIT11 | \
									 BIT12 | BIT13 | BIT14 | BIT15 | BIT16 | BIT17 | \
									 BIT18) 
									 
#define NAND_CRC_ERROR_MASK			(BIT19)
						
//=========================================================================
// Address offset 0x24
//=========================================================================
#define NAND_TX_TRG_BIT			0
#define NAND_TX_TRG_OK_BIT      1
						
#define NAND_TX_TRG_MASK		(BIT0)  
#define NAND_TX_TRG_OK_MASK		(BIT1)

//=========================================================================
// Address offset 0x30  (MS1 GPIO I)
//=========================================================================
#define MS1_GPIO_I_MASK		(BIT0  | BIT1  | BIT2  | BIT3  | BIT4  | BIT5  | \
							 BIT6  | BIT7  | BIT8  | BIT9  | BIT10 | BIT11 | \
							 BIT12 | BIT13 | BIT14)

//=========================================================================
// Address offset 0x34  (MS1 GPIO O)
//=========================================================================
#define NAND_CMD_IO_CE_BIT		14


#define NAND_CMD_IO_CE_MASK		(BIT14)

#define MS1_GPIO_O_MASK			(BIT0  | BIT1  | BIT2  | BIT3  | BIT4  | BIT5  | \
								 BIT6  | BIT7  | BIT8  | BIT9  | BIT10 | BIT11 | \
								 BIT12 | BIT13 | BIT14)
//=========================================================================
// Address offset 0x38  (MS1 GPIO OE)
//=========================================================================
#define MS1_GPIO_OE_MASK		(BIT0  | BIT1  | BIT2  | BIT3  | BIT4  | BIT5  | \
								 BIT6  | BIT7  | BIT8  | BIT9  | BIT10 | BIT11 | \
								 BIT12 | BIT13 | BIT14)

//=========================================================================
// Address offset 0x40
//=========================================================================
#define NAND_AV_FORMAT_BIT		0 
#define NAND_AV_FORMAT_MASK		(BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5)  
						
//=========================================================================
// Address offset 0x44  (NAND CMD)
//=========================================================================
#define NAND_ODD_CMD0_BIT			0
#define NAND_ODD_CMD1_BIT			2
#define NAND_EVEV_CMD0_BIT			4
#define NAND_EVEN_CMD1_BIT			6
#define NAND_CMD_NIBBLE_BIT			8
#define NAND_CMD_ADR_CYC_BIT		11
#define NAND_CMD_RD_CMD_CNT_BIT		13
#define NAND_CMD_PG_SIZE_BIT		14
#define NAND_CMD_BLK_SIZE_BIT		16
#define NAND_CMD_LBA_INC_BIT		18
#define NAND_READ_STATUS_CMD_BIT	24

#define NAND_ODD_CMD0_MASK			(BIT0)
#define NAND_ODD_CMD1_MASK			(BIT2)
#define NAND_EVEV_CMD0_MASK			(BIT4)
#define NAND_EVEN_CMD1_MASK			(BIT6)
#define NAND_CMD_NIBBLE_MASK		(BIT8 | BIT9 | BIT10)	
#define NAND_CMD_ADR_CYC_MASK		(BIT11 | BIT12)
#define NAND_CMD_RD_CMD_CNT_MASK	(BIT13)
#define NAND_CMD_PG_SIZE_MASK		(BIT14 |BIT15)
#define NAND_CMD_BLK_SIZE_MASK		(BIT16 | BIT17)
#define NAND_CMD_LBA_INC_MASK		(BIT18 | BIT19 | BIT20 | BIT21)

#define NAND_READ_STATUS_CMD_MASK	(BIT24 | BIT25 | BIT26 | BIT27 | BIT28 | \
									 BIT29 | BIT30 | BIT31) 
						
//=========================================================================
// Address offset 0x48  (MS1 DMA BLOCK)
//=========================================================================
#define MS1_DMA_BLOCK_BIT		0 

#define MS1_DMA_BLOCK_MASK		(BIT0  | BIT1  | BIT2  | BIT3  | BIT4  | BIT5  | \
								 BIT6  | BIT7  | BIT8  | BIT9  | BIT10 | BIT11 | \
								 BIT12 | BIT13 | BIT14 | BIT15)

//=========================================================================
// Address offset 0x4c  (MS1 TIME COUNT)
//=========================================================================
#define MS1_TIME_CNT_BIT		0 

#define MS1_TIME_CNT_MASK		(BIT0  | BIT1  | BIT2  | BIT3  | BIT4  | BIT5  | \
								 BIT6  | BIT7  | BIT8  | BIT9  | BIT10 | BIT11 | \
								 BIT12 | BIT13 | BIT14 | BIT15 | BIT16 | BIT17 | \
								 BIT18 | BIT19 | BIT20 | BIT21 | BIT22 | BIT23 | \
								 BIT24 | BIT25 | BIT26 | BIT27 | BIT28 | BIT29) 

//=========================================================================
// Address offset 0x50  (MS1 MDMA ECC)
//=========================================================================
#define MS1_MDMAEN_BIT				0
#define MS1_MDMAOK_BIT				1
#define MS1_MDMATIMEOUT_BIT			2
#define MS1_NF_PRG_ERR_BIT			3
#define MS1_NF_INFO_ERR_BIT			4
#define MS1_NF_ERASE_ERR_BIT		5
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
#define MS1_MDMATIMEOUT_MASK		(BIT2)
#define MS1_NF_PRG_ERR_MASK			(BIT3)
#define MS1_NF_INFO_ERR_MASK		(BIT4)
#define MS1_NF_ERASE_ERR_MASK		(BIT5)
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
#define NAND_SIZELBA_LBA_BIT	0		

#define NAND_SIZELBA_LBA_MASK	(BIT0  | BIT1  | BIT2  | BIT3  | BIT4  | BIT5  | \
								 BIT6  | BIT7  | BIT8  | BIT9  | BIT10 | BIT11 | \
								 BIT12 | BIT13 | BIT14 | BIT15 | BIT16 | BIT17 | \
								 BIT18 | BIT19 | BIT20 | BIT21 | BIT22) 

//=========================================================================
// Register address offset.
//=========================================================================
#define BASE_MS1            (0x90900000)
#define MS1_CTL				(BASE_MS1)
#define MS1_DMA_SIZE		(BASE_MS1 + 0x04)
#define MS1_GPIO_I			(BASE_MS1 + 0x30)
#define MS1_GPIO_O			(BASE_MS1 + 0x34)
#define MS1_GPIO_OE			(BASE_MS1 + 0x38)
#define MS1_SPI_CMD			(BASE_MS1 + 0x3c)
#define MS1_SPI_IDX			(BASE_MS1 + 0x40)
#define MS1_NF_CMD			(BASE_MS1 + 0x44)
#define MS1_DMA_BLKSU		(BASE_MS1 + 0x48)
#define MS1_TIME_CNT		(BASE_MS1 + 0x4c)
#define MS1_MDMAECC			(BASE_MS1 + 0x50)
#define MS1_LBA_R			(BASE_MS1 + 0x58)
#define MS1_DMAADDR			(BASE_MS1 + 0x5c)
#define MS1_REG_CMD			(BASE_MS1 + 0x70)
#define MS1_REG_ADDR		(BASE_MS1 + 0x74)
#define MS1_REG_DATA		(BASE_MS1 + 0x78)


//=========================================================================
// MS1 mode list.
//=========================================================================
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

//=========================================================================
// NF nibble command mode list.
//=========================================================================
enum nibble_cmd
{
	NAND_NIBBLE_RESET = 0,
	NAND_NIBBLE_MERASE,
	NAND_NIBBLE_ERASE,
	NAND_NIBBLE_READINFO
};

#ifndef ENABLE
#define ENABLE	1
#endif

#ifndef DISABLE
#define	DISABLE	0
#endif

//=========================================================================
// Function API
//=========================================================================
extern void ms1_set_mode (enum ms1_mode mode);
extern void ms1_info_disable(void);
extern void ms1_set_RegRW (enum ms1_reg_rw_mode mode); 
extern void ms1_set_DmaRW (u32 value);
extern void ms1_en_ExtraEcc (u32 isEnable);
extern void ms1_en_ecc (u32 isEnable);
extern void ms1_en_dma (u32 isEnable);
extern void ms1_en_Mdma (u32 isEnable);
extern void clear_ms1_CRCWErr (void);
extern void clear_ms1_EccErr (void);
extern void clear_ms1_RdyIntr (void);
extern void clear_ms1_AhbErrIntr (void);
extern void clear_ms1_EccErrIntr (void);
extern void ms1_set_DmaSize (u32 size);
extern void ms1_set_DmaBlk (u32 dmaBlockNum);
extern void ms1_set_DmaAddr (u32 addr);
extern u32 check_ms1_rdy (void);
extern u32 check_ms1_MdmaOk (void);
extern u32 check_ms1_MdmaTimeOut (void);
extern u32 check_ms1_CRCWErr (void);
extern u32 check_ms1_CRCWErr (void);
extern u32 check_ms1_EccErr (void);
extern void ms1_set_SpiCmd (u32 cmd);
extern void ms1_set_SpiIdx (u32 cmd);
extern void ms1_set_TimeCount (u32 timeCnt);
extern u32 check_ms1_EccErrIntr (void);
extern void ms1_en_EccErrIntr (u32 isEnable);
extern void ms1_write_cmd (u32 val);
extern void ms1_write_addr (u32 val);
extern void ms1_write_data (u32 val);
extern u32  ms1_read_data (u32 val);
extern u32  ms1_gpio_read(u32 gpio_num);
extern void ms1_gpio_write(u32 val, u32 gpio_num);
extern void ms1_gpio_mode(u32 val, u32 gpio_num);
void nf_dma_rw_test (void);
void nf_set_NibbleCmd (enum nibble_cmd mode);
void nf_Trg_NibbleCmd (void);
void nf_en_MPdma (u32 isEnable);
void nf_set_rd_width (u32 val);
void nf_set_wr_width (u32 val);
void nf_set_ReadStatus (u32 cmd);
void nf_set_AddrCyc (u32 val);
void nf_set_LBAIncMode (u32 cmd);
u32 check_nf_PrgErr (void);

void clear_nf_PrgErr (void);
void clear_nf_InfoCRCErr (void);
void clear_nf_EraseErr (void);
void nf_set_InfoMode (u32 cmd);
void nf_set_ReadCmdCnt (u32 readCmdCnt);
void nf_set_PageSize (u32 value);
void nf_set_BlkSize (u32 value);
u32 check_nf_InfoCrcErr (void);
int nf_read_NandLBA_R (void);

#endif
