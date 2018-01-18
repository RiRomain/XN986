/*
 * Create at 2011/05/06 by yanjie_yang
 */

#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/sysdev.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/physmap.h>
#include <linux/mtd/partitions.h>
#include <linux/i2c.h>
#include <media/soc_camera.h>
#include <linux/spi/spi.h>

#include <mach/platform.h>
#include <mach/irqs.h>
#include <mach/clock.h>
#include <mach/snx_media.h>

u64 snx_device_dmamask = DMA_BIT_MASK(32);

#ifdef CONFIG_SERIAL_SNX
static struct resource snx_uart0_resources[] = {
	[0] = {
		.start	= SNX_UART0_BASE,
		.end	= SNX_UART0_BASE + SZ_4K - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= INT_UART0_RS,
		.end	= INT_UART0_RS,
		.flags	= IORESOURCE_IRQ,
	},
};


static struct resource snx_uart1_resources[] = {
	[0] = {
		.start	= SNX_UART1_BASE,
		.end	= SNX_UART1_BASE + SZ_4K - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= INT_UART1_RS,
		.end	= INT_UART1_RS,
		.flags	= IORESOURCE_IRQ,
	},
};


static struct platform_device snx_uart0_device = {
	.name		= "snx_uart",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(snx_uart0_resources),
	.resource	= snx_uart0_resources,
};

static struct platform_device snx_uart1_device = {
	.name		= "snx_uart",
	.id		= 1,
	.num_resources	= ARRAY_SIZE(snx_uart1_resources),
	.resource	= snx_uart1_resources,
};

#endif

#ifdef CONFIG_I2C_SNX
static struct resource i2c0_resources[] = {
	{
		.start	= SNX_I2C0_BASE,
		.end	= SNX_I2C0_BASE + SZ_4K - 1,
		.flags	= IORESOURCE_MEM,
	}, {
		.start	= INT_I2C0,
		.end	= INT_I2C0,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct resource i2c1_resources[] = {
	{
		.start	= SNX_I2C1_BASE,
		.end	= SNX_I2C1_BASE + SZ_4K - 1,
		.flags	= IORESOURCE_MEM,
	}, {
		.start	= INT_I2C1,
		.end	= INT_I2C1,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device snx_i2c0_device = {
	.name		= "snx_i2c",
	.id		= 0,
	.resource	= i2c0_resources,
	.num_resources	= ARRAY_SIZE(i2c0_resources),
};

static struct platform_device snx_i2c1_device = {
	.name		= "snx_i2c",
	.id		= 1,
	.resource	= i2c1_resources,
	.num_resources	= ARRAY_SIZE(i2c1_resources),
};
#endif

#ifdef CONFIG_MTD_SNX
#if defined(CONFIG_MTD_SNX_NOR)
static struct mtd_partition snx_mtd_partitions[] = {
	{
	      .name		= "boot",
	      .offset		= 0,
	      .size		= SZ_128K,
	      .mask_flags	= MTD_WRITEABLE, /* force read-only */
	}, {
	      .name		= "params",
	      .offset		= MTDPART_OFS_APPEND,
	      .size		= SZ_128K,
	      .mask_flags	= MTD_WRITEABLE, /* force read-only */
	}, {
	      .name		= "kernel",
	      .offset		= MTDPART_OFS_APPEND,
	      .size		= SZ_1M + SZ_512K,
	      .mask_flags	= 0
	}, {
#if 0
	      .name		= "ramdisk",
	      .offset		= MTDPART_OFS_APPEND,
	      .size		= SZ_4M,
	      .mask_flags	= 0
	}, {
#endif
	      .name		= "jffs2",
	      .offset		= MTDPART_OFS_APPEND,
	      .size		= SZ_512K,
	      .mask_flags	= 0
	}, {
	      .name		= "cramfs",
	      .offset		= MTDPART_OFS_APPEND,
	      .size		= MTDPART_SIZ_FULL, /* all other free space*/
	      .mask_flags	= 0
	}
};
#elif defined(CONFIG_MTD_SNX_SF)

/* for 16 M sf .if run uboot->kernel->rootfs on serial flash,
no use, will partition in uboot*/
static struct mtd_partition snx_mtd_partitions[] = {
	{
	      .name		= "boot",
	      .offset		= 0,
	      .size		= SZ_512K + SZ_256K,
	      .mask_flags	= MTD_WRITEABLE, /* force read-only */
	}, {
	      .name		= "kernel",
	      .offset		= MTDPART_OFS_APPEND,
	      .size		= SZ_1M + SZ_2M,
	      .mask_flags	= MTD_WRITEABLE, /* force read-only */
	}, {
	      .name		= "rootfs",
	      .offset		= MTDPART_OFS_APPEND,
	      .size		= SZ_8M + SZ_2M,
	      .mask_flags	= 0
	}, {
	      .name		= "jffs2-rw",
	      .offset		= MTDPART_OFS_APPEND,
	      .size		= SZ_1M,
	      .mask_flags	= 0
	}, {
	      .name             = "userconfig", // for nvram
	      .offset           = MTDPART_OFS_APPEND,
	      .size             = SZ_256K,
	      .mask_flags       = 0,
	}, {
	      .name		= "logo",
	      .offset		= MTDPART_OFS_APPEND,
	      .size		= MTDPART_SIZ_FULL, /* all other free space*/
	      .mask_flags	= 0,
	}
};

#else /* SONFIG_MTD_NAND_SNX */
/*if run uboot->kernel->rootfs on nand flash,
no use, will partition in uboot*/
static struct mtd_partition snx_mtd_partitions[] = {
	{
	      .name       = "uboot",
	      .offset     = 0,
	      .size       = SZ_2M,
	      .mask_flags = MTD_WRITEABLE, 		/* force read-only */
	}, {
	      .name       = "kernel",
	      .offset     = MTDPART_OFS_APPEND,
	      .size       = SZ_1M + SZ_2M,
	      .mask_flags = MTD_WRITEABLE, 		/* force read-only */
	}, {
	      .name       = "cramfs",
	      .offset     = MTDPART_OFS_APPEND,
	      .size       = SZ_16M,
	      .mask_flags = MTD_WRITEABLE, 		/* force read-only */
	}, {
	      .name       = "jffs2",
	      .offset     = MTDPART_OFS_APPEND,
	      .size       = SZ_32M,
	      .mask_flags = 0,
	}, {
	      .name       = "userconfig",               //for nvram
	      .offset     = MTDPART_OFS_APPEND,
	      .size       = SZ_256K,
	      .mask_flags = 0,
	}, {
	      .name       = "user",
	      .offset     = MTDPART_OFS_APPEND,
	      .size       = 0x07C9ffff - 0x03540000 + 1, //7.375M
	      .mask_flags = MTD_WRITEABLE, 		/* force read-only */
	}/*, {//alek add test NVRAM
	      .name       = "Config",
	      .offset     = MTDPART_OFS_APPEND,
	      .size       = SZ_256K,
	      .mask_flags = 0,
	}*/, {
	      .name       = "other",
	      .offset     = MTDPART_OFS_APPEND,
	      .size       = MTDPART_SIZ_FULL,
	      .mask_flags = MTD_WRITEABLE, 		/* force read-only */
	},
};
#endif

static struct physmap_flash_data snx_flash_data = {
	.width		= 2,
	.parts		= snx_mtd_partitions,
	.nr_parts	= ARRAY_SIZE(snx_mtd_partitions),
};

#endif

#ifdef CONFIG_MTD_SNX_NOR
static struct resource snx_nor_resources[] = {
	[0] = {
		.start	= SNX_ROM_BASE,
		.end	= SNX_ROM_BASE + SZ_8M - 1,
		.flags	= IORESOURCE_MEM,
	},
};
static struct platform_device snx_nor_device = {
	.name		= "snx_nor",
	.id		= -1,
	.dev		= {
		.platform_data	= &snx_flash_data,
	},
	.num_resources	= ARRAY_SIZE(snx_nor_resources),
	.resource	= snx_nor_resources,
};
#endif

#ifdef CONFIG_MTD_SNX_SF
static struct resource snx_sf_resources[] = {
	[0] = {
		.start	= SNX_MS1_BASE,
		.end	= SNX_MS1_BASE + SZ_4K - 1,
		.flags	= IORESOURCE_MEM,
	},
};
static struct platform_device snx_sf_device = {
	.name		= "snx_sf",
	.id		= -1,
	.dev		= {
		.platform_data	= &snx_flash_data,
	},
	.num_resources	= ARRAY_SIZE(snx_sf_resources),
	.resource	= snx_sf_resources,
};
#endif

#ifdef CONFIG_MTD_NAND_SNX
static struct resource snx_nand_resources[] = {
	[0] = {
		.start	= SNX_MS1_BASE,
		.end	= SNX_MS1_BASE + SZ_4K - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= INT_MS1,
		.end	= 	INT_MS1,
		.flags	= IORESOURCE_IRQ,
	}
};
static struct platform_device snx_nand_device = {
	.name		= "snx_nand",
	.id		= -1,
	.dev		= {
		.platform_data	= &snx_flash_data,
	},
	.num_resources	= ARRAY_SIZE(snx_nand_resources),
	.resource	= snx_nand_resources,
};
#endif

#ifdef CONFIG_SNX_MAC
static struct snx_eth_data snx_mac_data = {
	.dev_addr	= "\x0\x84\x14\x4e\x49\x0",	/* Ethernet MAC address */
	.phy_id		= 1,
};

static struct resource snx_mac_resources[] = {
	[0] = {
		.start	= SNX_MAC_BASE,
		.end	= SNX_MAC_BASE + SZ_4K - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= INT_MAC,
		.end	= INT_MAC,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device snx_mac_device = {
	.name		= "snx_mac",
	.id		= -1,
	.dev		= {
		.platform_data	= &snx_mac_data,
	},
	.num_resources	= ARRAY_SIZE(snx_mac_resources),
	.resource	= snx_mac_resources,
};
#endif

//#ifdef CONFIG_SOUND_SNX
static struct resource snx_audio_resources[] = {
	[0] = {
		.start	= SNX_AUDIO_BASE,
		.end	= SNX_AUDIO_BASE + SZ_4K - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= INT_AUDIO_PLY,
		.end	= INT_AUDIO_PLY,
		.flags	= IORESOURCE_IRQ,
	},
	[2] = {
		.start	= INT_AUDIO_REC,
		.end	= INT_AUDIO_REC,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device snx_audio_device = {
	.name		= "snx_audio",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(snx_audio_resources),
	.resource	= snx_audio_resources,
};
//#endif

#ifdef CONFIG_MMC

static struct resource snx_mmcsf_resources[] = {
		[0] = {
				.start	= SNX_MS1_BASE,
				.end	= SNX_MS1_BASE + SZ_4K - 1,
				.flags	= IORESOURCE_MEM,
		},
		[1] = {
				.start	= INT_MS1,
				.end	= 	INT_MS1,
				.flags	= IORESOURCE_IRQ,
		}
};

static struct platform_device snx_mmcsf_device = {
	  .name 		  = "snx_mmcsf",
	  .id			  = -1,
	  .num_resources  = ARRAY_SIZE(snx_mmcsf_resources),
	  .resource 	  = snx_mmcsf_resources,
};


static struct resource snx_sd_resources[] = {
        [0] = {
                .start  = SNX_MS2_BASE,
                .end    = SNX_MS2_BASE + SZ_4K - 1,
                .flags  = IORESOURCE_MEM,
        },
        [1] = {
                .start  = INT_MS2,
                .end    = INT_MS2,
                .flags  = IORESOURCE_IRQ,
        },
        [2] = {
                .start  = INT_MS2CD,
                .end    = INT_MS2CD,
                .flags  = IORESOURCE_IRQ,
        },
};

static struct platform_device snx_sd_device = {
      .name           = "snx_sd",
      .id             = -1,
      .num_resources  = ARRAY_SIZE(snx_sd_resources),
      .resource       = snx_sd_resources,
};
#endif

static struct resource snx_hdma_resources[] = {
	[0] = {
			.start  = SNX_DMAC_BASE,
			.end    = SNX_DMAC_BASE + SZ_512 - 1,
			.flags  = IORESOURCE_MEM,
	},
	[1] = {
			.start  = INT_DMAC,
			.end    = INT_DMAC,
			.flags  = IORESOURCE_IRQ,
	},
};

static struct platform_device snx_hdma_device = {
		.name = "snx_hdma",
		.id       = -1,
		.dev  = {
				.dma_mask               = &snx_device_dmamask,
				.coherent_dma_mask      = DMA_BIT_MASK(32),
		},
		.resource       = snx_hdma_resources,
		.num_resources  = ARRAY_SIZE(snx_hdma_resources),
};

#ifdef CONFIG_USB

static struct resource snx_ehci_resources[] = {
        [0] = {
                .start  = SNX_EHCI_BASE,		//(0x90800000),
                .end    = SNX_EHCI_BASE + SZ_4K - 1,	//(0x90800000+0x100),
                .flags  = IORESOURCE_MEM,
         },
        [1] = {
                .start  = (INT_EHCI),
                .end    = (INT_EHCI),
                .flags  = IORESOURCE_IRQ,
         },

};

static struct platform_device snx_ehci_device = {
             .name	= "snx_ehci",
             .id		= 0,
             .dev		= {
			.coherent_dma_mask	= DMA_BIT_MASK(32),
			.dma_mask			= &snx_device_dmamask,
             },
             .num_resources	= ARRAY_SIZE(snx_ehci_resources),
             .resource			= snx_ehci_resources,
};
#endif

static struct spi_board_info snx_spi_info[] = {
	{
		.modalias		= "spidev",
		.bus_num	= 0,
		.max_speed_hz	= 6000000,
		.chip_select	= 0,
	}
#ifdef CONFIG_MACH_SN98610
	,{
		.modalias		= "spidev",
		.bus_num	= 1,
		.max_speed_hz	= 6000000,
		.chip_select	= 0,
	}
#endif
/*	,{
		.modalias		= "spidev",
		.bus_num	= 0,
		.max_speed_hz	= 8000000,
		.chip_select	= 0,
	},{
		.modalias		= "spidev",
		.bus_num	= 1,
		.max_speed_hz	= 8000000,
		.chip_select	= 0,
	}*/
};

static struct resource spi0_resources[] = {
	{
		.start	= SNX_SSP0_BASE,
		.end	= SNX_SSP0_BASE + SZ_4K - 1,
		.flags	= IORESOURCE_MEM,
	}, {
		.start	= INT_SSP0,
		.end	= INT_SSP0,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device snx_spi0_device = {
	.name		= "snx-spi",
	.id		= 0,
	.resource	= spi0_resources,
	.num_resources	= ARRAY_SIZE(spi0_resources),
};

#ifdef CONFIG_MACH_SN98610
static struct resource spi1_resources[] = {
	{
		.start	= SNX_SSP1_BASE,
		.end	= SNX_SSP1_BASE + SZ_4K - 1,
		.flags	= IORESOURCE_MEM,
	}, {
		.start	= INT_SSP1,
		.end	= INT_SSP1,
		.flags	= IORESOURCE_IRQ,
	},
};
static struct platform_device snx_spi1_device = {
	.name		= "snx-spi",
	.id		= 1,
	.resource	= spi1_resources,
	.num_resources	= ARRAY_SIZE(spi1_resources),
};
#endif

static struct i2c_board_info snx_i2c0_devs[] = {
	{
		I2C_BOARD_INFO("tidrv201", 0x1c),
	},
};

static struct i2c_board_info snx_i2c1_devs[] = {
	{
		I2C_BOARD_INFO("wm8960", 0x34),
	},
	/*
	{
		I2C_BOARD_INFO("tw2866", 0x50),
	},
	{
		I2C_BOARD_INFO("aic23", 0x34),
	},
	*/
	{
		I2C_BOARD_INFO("fm36", 0x60),
	}
};
#if !defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) && !defined(CONFIG_SYSTEM_PLATFORM_SN98660)
static struct resource snx_vo_resources[] = {
	[0] = {
		.start	= SNX_VO_BASE,		//(0x90700000),
		.end		= SNX_VO_BASE + SZ_4K - 1,	//(0x90700000+0x100),
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= (INT_VO),
		.end	= (INT_VO),
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device snx_vo_device = {
	.name = "snx_vo",
	.id = -1,
       .dev	= {
		.coherent_dma_mask      = DMA_BIT_MASK(32),
              .dma_mask               = &snx_device_dmamask,
	},
	.num_resources = ARRAY_SIZE(snx_vo_resources),
	.resource = snx_vo_resources,
};
#endif
static struct resource snx_vc_resources[] = {
	[0] = {
		.start	= SNX_H264_BASE,		//(0x90B00000),
		.end		= SNX_H264_BASE + SZ_4K - 1,	//(0x90B00000+0x100),
		.flags	= IORESOURCE_MEM,
		.name	= "h264 codec reg",
	},
	[1] = {
		.start	= SNX_IMGCTL_BASE,
		.end		= SNX_IMGCTL_BASE + SZ_4K - 1,
		.flags	= IORESOURCE_MEM,
		.name	= "image control reg"
	 },
	[2] = {
		.start	= SNX_JPEGD_BASE,
		.end		= SNX_JPEGD_BASE + SZ_4K - 1,
		.flags	= IORESOURCE_MEM,
		.name 	= "jpeg decode reg"
	},
	[3] = {
		.start	= INT_H264,
		.end		= INT_H264,
		.flags	= IORESOURCE_IRQ,
		.name	= "h264 codec irq",
	},
	[4] = {
		.start	= INT_JPEGD,
		.end		= INT_JPEGD,
		.flags	= IORESOURCE_IRQ,
		.name	= "jpeg decode irq"
	},

	[5] = {
		.start	= INT_H264,
		.end		= INT_H264,
		.flags	= IORESOURCE_IRQ,
		.name	= "jpeg encode irq"
	},
};

static struct platform_device snx_vc_device = {
	.name = "snx_vc",
	.id = -1,
	.num_resources = ARRAY_SIZE(snx_vc_resources),
	.resource = snx_vc_resources,
	.dev = {
		.dma_mask = &snx_device_dmamask,
		.coherent_dma_mask = DMA_BIT_MASK(32),
       },
};

static struct resource snx_rtc_resources[] = {
        [0] = {
                .start  = (INT_RTC_ALM),
                .end    = (INT_RTC_ALM),
                .flags  = IORESOURCE_IRQ,
        },
        [1] = {
                .start  = (INT_RTC_WK),
                .end    = (INT_RTC_WK),
                .flags  = IORESOURCE_IRQ,
        },
	[3] = {
		.start	= SNX_RTC_BASE,
		.end	= SNX_RTC_BASE + SZ_4K - 1,
		.flags	= IORESOURCE_MEM,
	}
};

static struct resource snx_wdt_resources[] = {
	[0] = {
		.start  = (INT_WTD),
		.end    = (INT_WTD),
		.flags  = IORESOURCE_IRQ,
	},
	[1] = {
		.start  = SNX_WTD_BASE,
		.end    = SNX_WTD_BASE + SZ_512 - 1,
		.flags  = IORESOURCE_MEM,
	}
};

static struct platform_device snx_wdt_device = {
	.name	= "snx_wdt",
	.id     = -1,
	.num_resources      = ARRAY_SIZE(snx_wdt_resources),
        .resource           = snx_wdt_resources,
};

struct platform_device snx_rtc_device = {
	.name             = "snx_rtc",
        .id               = -1,
        .num_resources    = ARRAY_SIZE(snx_rtc_resources),
        .resource         = snx_rtc_resources,
};

static struct resource snx_pwm_resources[] = {
	[0] = {
		.start  = SNX_PWM1_BASE,
		.end    = SNX_PWM1_BASE + SZ_512 - 1,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.start  = SNX_PWM2_BASE,
		.end    = SNX_PWM2_BASE + SZ_512 - 1,
		.flags  = IORESOURCE_MEM,
	},
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
	[2] = {
		.start  = SNX_PWM3_BASE,
		.end    = SNX_PWM3_BASE + SZ_512 - 1,
		.flags  = IORESOURCE_MEM,
	},
#endif
};

static struct platform_device snx_pwm_device = {
    .name                   = "snx_pwm",
    .id                         = -1,
    .num_resources      = ARRAY_SIZE(snx_pwm_resources),
    .resource               = snx_pwm_resources,
};

static struct resource snx_crypto_resources[] = {
	[0] = {
		.start  = (INT_AES),
		.end    = (INT_AES),
		.flags  = IORESOURCE_IRQ,
	},
	[1] = {
		.start  = SNX_CIPHER_BASE,
		.end    = SNX_CIPHER_BASE + SZ_512 - 1,
		.flags  = IORESOURCE_MEM,
	}
};

static struct platform_device snx_crypto_device = {
	.name	= "snx_crypto",
	.id     = -1,
	.num_resources      = ARRAY_SIZE(snx_crypto_resources),
	.resource           = snx_crypto_resources,
};


static struct resource snx_gpio_resources[] = {
	[0] = {
		.start	= SNX_GPIO_BASE,
		.end		= SNX_GPIO_BASE + SZ_4K - 1,
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device snx_gpio_device = {
	.name				= "snx_gpio",
	.id 					= -1,
	.num_resources		= ARRAY_SIZE(snx_gpio_resources),
	.resource				= snx_gpio_resources,
};

static struct resource snx_sys_resources[] = {
	[0] = {
		.start	= SNX_SYS_BASE,
		.end	= SNX_SYS_BASE + SZ_4K - 1,
		.flags	= IORESOURCE_MEM,
	},
};

static struct platform_device snx_sys_device = {
	.name		= "snx_sys",
	.id 		= -1,
	.num_resources	= ARRAY_SIZE(snx_sys_resources),
	.resource	= snx_sys_resources,
};

static struct resource snx_isp_resources[] = {
	[0] = {
		.start	= SNX_ISP_BASE,
		.end	= SNX_ISP_BASE + SZ_4K + SZ_2K - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= SNX_MIPI_BASE,
		.end	= SNX_MIPI_BASE + SZ_2K - 1,
		.flags	= IORESOURCE_MEM,
	},
	[2] = {
		.start	= INT_ISP_VS,
		.end	= INT_ISP_VS,
		.flags	= IORESOURCE_IRQ,
	},
	[3] = {
		.start	= INT_ISP_HW,
		.end		= INT_ISP_HW,
		.flags	= IORESOURCE_IRQ,
	},
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
	[4] = {
		.start	= INT_ISP_HS,
		.end		= INT_ISP_HS,
		.flags	= IORESOURCE_IRQ,
	},
#endif
};

static struct snx_camera_pdata camera_pdata={
	.mclk_hz = 24000000, /*24MHz*/
};

static struct platform_device snx_isp_device = {
	.name		= "snx_isp",
	.id		= 0,
	.dev            = {
		.coherent_dma_mask      = DMA_BIT_MASK(32),
		.dma_mask               = &snx_device_dmamask,
		.platform_data 		= &camera_pdata,
         },
	.num_resources  = ARRAY_SIZE(snx_isp_resources),
	.resource	= snx_isp_resources,
};

static struct resource snx_udc_resources[] = {
	[0] = {
		.start  = (INT_UDC),
		.end    = (INT_UDC),
		.flags  = IORESOURCE_IRQ,
	},
	[1] = {
		.start  = SNX_UDC_BASE,
		.end    = SNX_UDC_BASE + SZ_4K - 1,
		.flags  = IORESOURCE_MEM,
	}
};

static struct platform_device snx_udc_device = {
	.name	= "snx_udc",
	.id     = -1,
	.num_resources      = ARRAY_SIZE(snx_udc_resources),
    .resource           = snx_udc_resources,
};

static struct platform_device *sn98600_devices[] __initdata = {
#ifdef CONFIG_SERIAL_SNX
	&snx_uart0_device,

	&snx_uart1_device,

#endif
#ifdef CONFIG_I2C_SNX
	&snx_i2c0_device,
	&snx_i2c1_device,
#endif

	&snx_rtc_device,
	&snx_wdt_device,
	&snx_pwm_device,
	&snx_sys_device,
	&snx_gpio_device,
	&snx_crypto_device,
	&snx_spi0_device,
#ifdef CONFIG_MACH_SN98610
	&snx_spi1_device,
#endif
/*
#ifdef CONFIG_MTD_SNX
	&snx_flash_device,
#endif
*/
#ifdef CONFIG_MTD_SNX_NOR
	&snx_nor_device,
#endif
#ifdef CONFIG_MTD_SNX_SF
	&snx_sf_device,
#endif

#ifdef CONFIG_MTD_NAND_SNX
	&snx_nand_device,
#endif

#ifdef CONFIG_MTD_SNX_SD2NAND
	&snx_sd2nand_device,
#endif

#ifdef CONFIG_SNX_MAC
	&snx_mac_device,
#endif
//#ifdef CONFIG_SOUND_SNX
	&snx_audio_device,
//#endif

#ifdef CONFIG_MMC
        //&snx_mmcsf_device,
        &snx_sd_device,
#endif
#ifdef CONFIG_USB
	&snx_ehci_device,
#endif
	&snx_udc_device,
	&snx_hdma_device,
#if !defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) && !defined(CONFIG_SYSTEM_PLATFORM_SN98660)
	&snx_vo_device,
#endif
	&snx_vc_device,
	&snx_isp_device,
};

void __init sn98600_init(void)
{
//	snx_clock_init();

	if(resource_size(&reserved_video_mem[0]))
		snx_declare_coherent_mem(&snx_isp_device.dev, &reserved_video_mem[0]);
	if(resource_size(&reserved_video_mem[1]))
		snx_declare_coherent_mem(&snx_vc_device.dev, &reserved_video_mem[1]);
#if !defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) && !defined(CONFIG_SYSTEM_PLATFORM_SN98660)
	if(resource_size(&reserved_video_mem[2]))
		snx_declare_coherent_mem(&snx_vo_device.dev, &reserved_video_mem[2]);
#endif
	i2c_register_board_info(0, snx_i2c0_devs, ARRAY_SIZE(snx_i2c0_devs));
	i2c_register_board_info(1, snx_i2c1_devs, ARRAY_SIZE(snx_i2c1_devs));
	spi_register_board_info (snx_spi_info, ARRAY_SIZE(snx_spi_info));
	platform_add_devices(sn98600_devices, ARRAY_SIZE(sn98600_devices));
}
