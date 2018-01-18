//=======================================================================
// 	Copyright Sonix Technology Corp 2011.  All rights reserved.
//-----------------------------------------------------------------------
// File Name : spi_reg.h
// Function  : 
// Program   : yanjie_yang
// Date      : 2011/10/13
// Version   : 0.0
// History
//=======================================================================
#ifndef __SPI_REG_H__
#define __SPI_REG_H__

#include "bits.h"

//=========================================================================
// Address offset 0x00
//=========================================================================
#define SSP_SCLKPH_BIT				0
#define SSP_SCLKPO_BIT				1
#define SSP_OPM_BIT					2
#define SSP_FSJSTFY_BIT				4
#define SSP_FSPO_BIT				5
#define SSP_LSB_BIT					6
#define SSP_LBM_BIT					7
#define SSP_FSDIST_BIT				8
#define SSP_FFMT_BIT				12
#define SSP_GPIO_BIT				15
#define SSP_CLK_GPIO_OE_BIT			16
#define SSP_FS_GPIO_OE_BIT			17
#define SSP_TX_GPIO_OE_BIT			18
#define SSP_RX_GPIO_OE_BIT			19
#define SSP_CLK_GPIO_O_BIT 			20
#define SSP_FS_GPIO_O_BIT			21
#define SSP_TX_GPIO_O_BIT			22
#define SSP_RX_GPIO_O_BIT			23
#define SSP_CLK_GPIO_I_BIT 			24
#define SSP_FS_GPIO_I_BIT			25
#define SSP_TX_GPIO_I_BIT			26
#define SSP_RX_GPIO_I_BIT			27

#define SSP_SCLKPH_MASK				BIT0
#define SSP_SCLKPO_MASK				BIT1
#define SSP_OPM_MASK				(BIT2 | BIT3)
#define SSP_FSJSTFY_MASK			BIT4
#define SSP_FSPO_MASK				BIT5
#define SSP_LSB_MASK				BIT6
#define SSP_LBM_MASK				BIT7
#define SSP_FSDIST_MASK				(BIT8 | BIT9)
#define SSP_FFMT_MASK				(BIT12 | BIT13 | BIT14)
#define SSP_GPIO_MASK				(BIT15)
#define SSP_CLK_GPIO_OE_MASK 		(BIT16)
#define SSP_FS_GPIO_OE_MASK			(BIT17)
#define SSP_TX_GPIO_OE_MASK			(BIT18)
#define SSP_RX_GPIO_OE_MASK			(BIT19)
#define SSP_CLK_GPIO_O_MASK			(BIT20)
#define SSP_FS_GPIO_O_MASK			(BIT21)
#define SSP_TX_GPIO_O_MASK			(BIT22)
#define SSP_RX_GPIO_O_MASK			(BIT23)
#define SSP_CLK_GPIO_I_MASK			(BIT24)
#define SSP_FS_GPIO_I_MASK			(BIT25)
#define SSP_TX_GPIO_I_MASK			(BIT26)
#define SSP_RX_GPIO_I_MASK			(BIT27)

#define SSP_SPI_MODE					0x0
#define SSP_GPIO_MODE				0x1


//=========================================================================
// Address offset 0x04
//=========================================================================
#define SSP_SCLKDIV_BIT				0
#define SSP_SDL_BIT					16
#define SSP_PDL_BIT					24

#define SSP_SCLKDIV_MASK			(BIT0 | BIT1 | BIT2 | BIT3 \
		| BIT4 | BIT5 | BIT6 | BIT7 | BIT8 | BIT9 | BIT10 | BIT11 \
		| BIT12 | BIT13 | BIT14 | BIT15)
#define SSP_SDL_MASK				(BIT16 | BIT17 | BIT18 \
		| BIT19 | BIT20)
#define SSP_PDL_MASK				(BIT24 | BIT25 | BIT26 \
		| BIT27 | BIT28 | BIT29 | BIT30 | BIT31)

//=========================================================================
// Address offset 0x08
//=========================================================================
#define SSP_EN_BIT					0
#define SSP_TXDOE_BIT				1
#define SSP_RXF_CLR_BIT				2
#define SSP_TXF_CLR_BIT				3
#define SSP_ACWRST_BIT				4
#define SSP_ACCRST_BIT				5
#define SSP_RST_BIT					6

#define SSP_EN_MASK					BIT0
#define SSP_TXDOE_MASK				BIT1
#define SSP_RXF_CLR_MASK			BIT2
#define SSP_TXF_CLR_MASK			BIT3
#define SSP_ACWRST_MASK				BIT4
#define SSP_ACCRST_MASK				BIT5
#define SSP_RST_MASK				BIT6

//=========================================================================
// Address offset 0x0c
//=========================================================================
#define SSP_RXF_FULL_BIT			0
#define SSP_TXF_NFULL_BIT			1
#define SSP_BUSY_BIT				2
#define SSP_RXF_VE_BIT				4
#define SSP_TXF_VE_BIT				12

#define SSP_RXF_FULL_MASK			BIT0
#define SSP_TXF_NFULL_MASK			BIT1
#define SSP_BUSY_MASK				BIT2
#define SSP_RXF_VE_MASK				(BIT4 | BIT5 | BIT6 \
		| BIT7 | BIT8)
#define SSP_TXF_VE_MASK				(BIT12 | BIT13 | BIT14 \
		| BIT15 | BIT16)

//=========================================================================
// Address offset 0x10
//=========================================================================
#define SSP_RXF_OR_INTR_EN_BIT			0
#define SSP_TXF_UR_INTR_EN_BIT			1
#define SSP_RXF_TH_INTR_EN_BIT			2
#define SSP_TXF_TH_INTR_EN_BIT			3
#define SSP_RXF_DMA_EN_BIT				4
#define SSP_TXF_DMA_EN_BIT				5
#define AC97_FC_INTR_EN_BIT				6
#define SSP_RXF_TH_BIT					8
#define SSP_TXF_TH_BIT					12

#define SSP_RXF_OR_INTR_EN_MASK			BIT0
#define SSP_TXF_UR_INTR_EN_MASK			BIT1
#define SSP_RXF_TH_INTR_EN_MASK			BIT2
#define SSP_TXF_TH_INTR_EN_MASK			BIT3
#define SSP_RXF_DMA_EN_MASK				BIT4
#define SSP_TXF_DMA_EN_MASK				BIT5
#define AC97_FC_INTR_EN_MASK			BIT6
#define SSP_RXF_TH_MASK					(BIT8 | BIT9 | BIT10 | BIT11)
#define SSP_TXF_TH_MASK					(BIT12 | BIT13 | BIT14 | BIT15)

//=========================================================================
// Address offset 0x14
//=========================================================================
#define SSP_RXF_OR_FLAG_BIT				0
#define SSP_TXF_UR_FLAG_BIT				1
#define SSP_RXF_TH_FLAG_BIT				2
#define SSP_TXF_TH_FLAG_BIT				3
#define AC97_FC_FLAG_BIT				4

#define SSP_RXF_OR_FLAG_MASK			BIT0
#define SSP_TXF_UR_FLAG_MASK			BIT1
#define SSP_RXF_TH_FLAG_MASK			BIT2
#define SSP_TXF_TH_FLAG_MASK			BIT3
#define AC97_FC_FLAG_MASK				BIT4

//=========================================================================
// Address offset 0x20
//=========================================================================
#define CODECID_BIT				0
#define SLOT12V_BIT				3
#define SLOT11V_BIT				4
#define SLOT10V_BIT				5
#define SLOT9V_BIT				6
#define SLOT8V_BIT				7
#define SLOT7V_BIT				8
#define SLOT6V_BIT				9
#define SLOT5V_BIT				10
#define SLOT4V_BIT				11
#define SLOT3V_BIT				12
#define SLOT2V_BIT				13
#define SLOT1V_BIT				14

#define CODECID_MASK			(BIT0 | BIT1)
#define SLOT12V_MASK			BIT3
#define SLOT11V_MASK			BIT4
#define SLOT10V_MASK			BIT5
#define SLOT9V_MASK				BIT6
#define SLOT8V_MASK				BIT7
#define SLOT7V_MASK				BIT8
#define SLOT6V_MASK				BIT9
#define SLOT5V_MASK				BIT10
#define SLOT4V_MASK				BIT11
#define SLOT3V_MASK				BIT12
#define SLOT2V_MASK				BIT13
#define SLOT1V_MASK				BIT14




//=========================================================================
// Register address offset
//=========================================================================

#define SSP_CTRL0_OFFSET				(0)
#define SSP_CTRL1_OFFSET				(0x04)
#define SSP_CTRL2_OFFSET				(0x08)
#define SSP_STATUS_OFFSET				(0x0c)
#define SSP_INTR_EN_OFFSET				(0x10)
#define SSP_INTR_STATUS_OFFSET			(0x14)
#define SSP_DATA_OFFSET				(0x18)
#define SSP_SLOT_OFFSET				(0x20)



#endif	/* __SPI_REG_H__  */
