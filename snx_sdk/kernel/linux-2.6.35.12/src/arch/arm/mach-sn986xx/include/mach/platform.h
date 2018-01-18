/*
 * Create at 2011/05/06 by yanjie_yang
 */

#ifndef __MACH_PLATFORM_H
#define __MACH_PLATFORM_H

/*
 * SN98600 register mapping:
 *
 * 0x90000000 - 0x90cfffff <--> 0xf0000000 - 0xf0cfffff		: AHB Device
 * 0x90d00000 - 0x97ffffff <--> 0xf0d00000 - 0xf7ffffff		: Reserved
 * 0x98000000 - 0x98cfffff <--> 0xf8000000 - 0xf8cfffff		: APB Device
 */
#define AHB_PHYS_BASE		0x90000000
#define AHB_VIRT_BASE		0xf0000000
//#define APB_PHYS_BASE		0X98000000
//#define APB_VIRT_BASE		0Xf8000000

#define PIO_BASE		AHB_PHYS_BASE
#define VIO_BASE		AHB_VIRT_BASE

/* macro to get at IO space when running virtually */
#define IO_ADDRESS(x)		(((x) & 0x0fffffff) + 0xf0000000)

#define io_p2v(x)		( (x) - PIO_BASE + VIO_BASE )
#define io_v2p(x)		( (x) - VIO_BASE + PIO_BASE )

/*
 * SN98600 device physical base addresses
 */
/* AHB devices */
#define SNX_AHBC_BASE			0x90000000
#define SNX_APBC_BASE			0x90100000
#define SNX_SMC_BASE			0x90200000
#define SNX_DDRC_BASE			0x90300000
#define SNX_DMAC_BASE			0x90400000
#define SNX_MAC_BASE			0x90500000
#define SNX_ISP_BASE			0x90600000
#define SNX_VO_BASE			0x90700000
#define SNX_EHCI_BASE			0x90800000
#define SNX_MS1_BASE			0x90900000
#define SNX_AUDIO_BASE			0x90a00000
#define SNX_H264_BASE			0x90b00000
#define SNX_IMGCTL_BASE			0x90c00000
#define SNX_JPEGD_BASE			0x90d00000
#define SNX_CIPHER_BASE			0x90e00000
#define SNX_BB_BASE			0x90f00000
#define SNX_MS2_BASE			0x91000000
#define SNX_MS3_BASE			0x91100000
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_ST58660)
#define SNX_MIPI_BASE                   0x90601800
#else
#define SNX_MIPI_BASE			0x91400000
#endif

/* APB device */
#define SNX_SYS_BASE			0x98000000
#define SNX_GPIO_BASE			0x98100000
#define SNX_INTC_BASE			0x98200000
#define SNX_I2C0_BASE			0x98300000
#define SNX_I2C1_BASE			0x98400000
#define SNX_RTC_BASE			0x98500000
#define SNX_TIMER_BASE			0x98600000
#define SNX_WTD_BASE			0x98700000
#define SNX_MOTOR_BASE			0x98800000
#define SNX_SSP0_BASE			0x98900000
#define SNX_UART0_BASE			0x98a00000
#define SNX_UART1_BASE			0x98b00000
#define SNX_SDIO_BASE			0x98c00000
#define SNX_PWM1_BASE			0x99000000
#define SNX_PWM2_BASE			0x99100000
#define SNX_PWM3_BASE			0x99300000
#define SNX_SSP1_BASE			0x99200000
#define SNX_UDC_BASE            0x91600000


/*
 * Flash Setting
 */
#define SNX_ROM_BASE			0x80000000
#define SNX_ROM_SIZE			0x00800000

#ifndef __ASSEMBLY__
/*
 * Clock Settings
 */
extern int snx_clock_init(void);
extern unsigned long snx_apb_clk_rate;
#define APB_CLK				snx_apb_clk_rate

struct snx_eth_data
{
	unsigned char	dev_addr[6];
	unsigned char	phy_id;
};
#endif

#endif
