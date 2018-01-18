/*
 * Create at 2011/05/06 by yanjie_yang
 */

#ifndef __MACH_REGS_SYS_H
#define __MACH_REGS_SYS_H
#include <mach/platform.h>

/*
 * SONiX clock & power management unit controller virtual addresses
 */
#define SYS_MISC			IO_ADDRESS(SNX_SYS_MISC)
#define SYS_CLKSRC			IO_ADDRESS(SNX_SYS_CLKSRC)
#define SYS_CLKGATE			IO_ADDRESS(SNX_SYS_CLKGATE)
#define SYS_ASIC_ID			IO_ADDRESS(SNX_SYS_ASIC_ID)
#define SYS_CTRL2			IO_ADDRESS(SNX_SYS_CTRL2)
#define SYS_CLKGATE1			IO_ADDRESS(SNX_SYS_CLKGATE1)
#define SYS_CLKSRC1			IO_ADDRESS(SNX_SYS_CLKSRC1)

/*
 * SONiX clock & power management unit controller physical addresses
 */
#define SNX_SYS_MISC			(SNX_SYS_BASE + SYS_MISC_OFFSET)
#define SNX_SYS_CLKSRC			(SNX_SYS_BASE + SYS_CLKSRC_OFFSET)
#define SNX_SYS_CLKGATE			(SNX_SYS_BASE + SYS_CLKGATE_OFFSET)
#define SNX_SYS_ASIC_ID			(SNX_SYS_BASE + SYS_ASIC_ID_OFFSET)
#define SNX_SYS_CTRL2			(SNX_SYS_BASE + SYS_CTRL2_OFFSET)
#define SNX_SYS_CLKGATE1		(SNX_SYS_BASE + SYS_CLKGATE1_OFFSET)
#define SNX_SYS_CLKSRC1			(SNX_SYS_BASE + SYS_CLKSRC1_OFFSET)

/* Clock & power management unit controller registers offset */
#define SYS_MISC_OFFSET			0x00	/* Misc power management setting */ 
#define SYS_CLKSRC_OFFSET		0x04	/* Clock source setting */
#define SYS_CLKGATE_OFFSET		0x08	/* Clock gate setting */
#define SYS_ASIC_ID_OFFSET		0x10
#define SYS_CTRL2_OFFSET		0x14	
#define SYS_CLKGATE1_OFFSET		0x18	/* Clock gate1 setting */
#define SYS_CLKSRC1_OFFSET		0x1C	/* Clock source1 setting */

/*
 * CLKSRC register bits
 */
#define MISC_PWR_DOWN_BIT		0
#define MISC_PWN_DOWN_MODE_BIT		1
#define MISC_SEN_PULL_DOWN_BIT		2
#define MISC_REG_CK_MODE_BIT		3
#define MISC_TESTER_BIT			4
#define MISC_RMII_MODE_BIT		5
#define MISC_MAC_PHY_CLK_OE_BIT		6
#define MISC_PLL800_ICP_BIT		7
#define MISC_PLL800_SW_BIT		11
#define MISC_PLL800_DIV_BIT		15
#define MISC_PLL300_EN_BIT		22
#define MISC_PLL300_DIV_BIT		23
#define MISC_PLL100_EN_BIT		29
#define MISC_PLL480_EN_BIT		30
#define MISC_DDR_VREF_EN_BIT		31

#define MISC_PLL800_ICP_MASK		(BIT(7) | BIT(8) | BIT(9) | BIT(10))
#define MISC_PLL800_SW_MASK		(BIT(11) | BIT(12) | BIT(13) | BIT(14))
#define MISC_PLL800_DIV_MASK		(BIT(15) | BIT(16) | BIT(17) | BIT(18) | BIT(19) | BIT(20) | BIT(21))
#define MISC_PLL300_DIV_MASK		(BIT(23) | BIT(24) | BIT(25) | BIT(26) | BIT(27) | BIT(28))


/*
 * CLKSRC register bits
 */
#define CLKSRC_DDR_BIT			0
#define CLKSRC_CPU_BIT			4
#define CLKSRC_AHB_BIT			8
#define CLKSRC_APB_BIT			12
#define CLKSRC_AHB2_BIT			15
#define CLKSRC_ISP_BIT			20
#define CLKSRC_LCD_BIT			24
#define CLKSRC_JPGDEC_BIT		28

#define CLKSRC_DDR_MASK			(BIT(0) | BIT(1) | BIT(2) | BIT(3))
#define CLKSRC_CPU_MASK			(BIT(4) | BIT(5) | BIT(6) | BIT(7))
#define CLKSRC_AHB_MASK			(BIT(8) | BIT(9) | BIT(10) | BIT(11))
#define CLKSRC_APB_MASK			(BIT(12) | BIT(13) | BIT(14))
#define CLKSRC_AHB2_MASK		(BIT(15) | BIT(16) | BIT(17) | BIT(18) | BIT(19))
#define CLKSRC_ISP_MASK			(BIT(20) | BIT(21) | BIT(22) | BIT(23))
#define CLKSRC_LCD_MASK			(BIT(24) | BIT(25) | BIT(26) | BIT(27))
#define CLKSRC_JPGDEC_MASK		(BIT(28) | BIT(29) | BIT(30) | BIT(31))

/*
 * CLKSRC1 register bits
 */
#define CLKSRC1_IMG_BIT			0
#define CLKSRC1_H264_BIT		4

#define CLKSRC1_IMG_MASK		(BIT(0) | BIT(1) | BIT(2) | BIT(3))
#define CLKSRC1_H264_MASK		(BIT(4) | BIT(5) | BIT(6) | BIT(7))

/*
 * CLKGATE register bits
 */
#define CLKGATE_UART1			(0x1<<31)     	/* UART1 controller clock gate */
#define CLKGATE_UART0			(0x1<<30)     	/* UART0 controller clock gate */
#define CLKGATE_SSP2			(0x1<<29)
#define CLKGATE_SSP1			(0x1<<28)     	/* SSP1 controller clock gate */
#define CLKGATE_I2C1			(0x1<<27)     	/* I2C1 controller clock gate */
#define CLKGATE_I2C0			(0x1<<26)     	/* I2C0 controller clock gate */
#define CLKGATE_H264			(0x1<<25)     	/* H264 clock gate */
#define CLKGATE_AUDIO			(0x1<<24)     	/* Audio controller clock gate */
#define CLKGATE_MS2			(0x1<<23)
#define CLKGATE_MS1			(0x1<<22)     	/* Mass storage controller clock gate */
#define CLKGATE_MAC			(0x1<<21)     	/* MAC controller clock gate */
#define CLKGATE_DMAC			(0x1<<20)      	/* DMA controller clock gate */
#define CLKGATE_LCD			(0x1<<19)      	/* LCD controller clock gate */

#define CLKGATE_ISP_OSD			(0x1<<18)
#define CLKGATE_ISP_FIRF1		(0x1<<17)
#define CLKGATE_ISP_FIRF0		(0x1<<16)
#define CLKGATE_ISP_YUVF1		(0x1<<15)
#define CLKGATE_ISP_YUVF0		(0x1<<14)
#define CLKGATE_ISP_SCAL1		(0x1<<13)
#define CLKGATE_ISP_SCAL0		(0x1<<12)
#define CLKGATE_ISP_YUV			(0x1<<11)
#define CLKGATE_ISP_MASK		(0x1<<10)
#define CLKGATE_ISP_MD			(0x1<<9)
#define CLKGATE_ISP_RPT			(0x1<<8)
#define CLKGATE_ISP_Y2R			(0x1<<7)
#define CLKGATE_ISP_RGB			(0x1<<6)
#define CLKGATE_ISP_RAW			(0x1<<5)
#define CLKGATE_ISP_PAT			(0x1<<4)
#define CLKGATE_ISP_POST_CLR		(0x1<<3)
#define CLKGATE_ISP_PRE_CLR		(0x1<<2)
#define CLKGATE_SENSOR			(0x1<<1)      	/* Sensor controller clock gate */
#define CLKGATE_DDRC			(0x1<<0)      	/* DDR controller clock gate */

/*
 * CLKGATE1 register bits
 */
#define CLKGATE1_PWM3			(0x1<<14)
#define CLKGATE1_IMG			(0x1<<9)
#define CLKGATE1_MIPI			(0x1<<8)
#define CLKGATE1_AHB_MON		(0x1<<7)
#define CLKGATE1_PWM2			(0x1<<6)
#define CLKGATE1_PWM1			(0x1<<5)
#define CLKGATE1_AES			(0x1<<4)
#define CLKGATE1_DLC_SPI		(0x1<<3)
#define CLKGATE1_USB			(0x1<<2)
#define CLKGATE1_JPGDEC			(0x1<<1)
#define CLKGATE1_JPGENC			(0x1<<0)

/*
 * CTRL2 register bits
 */
#define CTRL2_H264_CLK_SRC_BIT		15


#endif
