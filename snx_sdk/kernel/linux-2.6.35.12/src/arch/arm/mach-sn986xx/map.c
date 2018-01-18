/*
 * Create at 2011/05/06 by yanjie_yang
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/platform_device.h>

#include <asm/mach/map.h>
#include <asm/memory.h>
#include <asm/sizes.h>
#include <asm/io.h>
#include <asm/page.h>

#include <mach/platform.h>
#include <mach/snx_media.h>

static struct map_desc sn98600_io_desc[] __initdata = 
{
	{
		.virtual	=  IO_ADDRESS(SNX_AHBC_BASE),
		.pfn		= __phys_to_pfn(SNX_AHBC_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	=  IO_ADDRESS(SNX_APBC_BASE),
		.pfn		= __phys_to_pfn(SNX_APBC_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	=  IO_ADDRESS(SNX_SMC_BASE),
		.pfn		= __phys_to_pfn(SNX_SMC_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	=  IO_ADDRESS(SNX_DDRC_BASE),
		.pfn		= __phys_to_pfn(SNX_DDRC_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	=  IO_ADDRESS(SNX_DMAC_BASE),
		.pfn		= __phys_to_pfn(SNX_DMAC_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	=  IO_ADDRESS(SNX_MAC_BASE),
		.pfn		= __phys_to_pfn(SNX_MAC_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	=  IO_ADDRESS(SNX_ISP_BASE),
		.pfn		= __phys_to_pfn(SNX_ISP_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	=  IO_ADDRESS(SNX_VO_BASE),
		.pfn		= __phys_to_pfn(SNX_VO_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	=  IO_ADDRESS(SNX_MS1_BASE),
		.pfn		= __phys_to_pfn(SNX_MS1_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	=  IO_ADDRESS(SNX_AUDIO_BASE),
		.pfn		= __phys_to_pfn(SNX_AUDIO_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	=  IO_ADDRESS(SNX_H264_BASE),
		.pfn		= __phys_to_pfn(SNX_H264_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	=  IO_ADDRESS(SNX_IMGCTL_BASE),
		.pfn		= __phys_to_pfn(SNX_IMGCTL_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	=  IO_ADDRESS(SNX_JPEGD_BASE),
		.pfn		= __phys_to_pfn(SNX_JPEGD_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	=  IO_ADDRESS(SNX_CIPHER_BASE),
		.pfn		= __phys_to_pfn(SNX_CIPHER_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	=  IO_ADDRESS(SNX_BB_BASE),
		.pfn		= __phys_to_pfn(SNX_BB_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE	
	}, {
		.virtual	=  IO_ADDRESS(SNX_MS2_BASE),
		.pfn		= __phys_to_pfn(SNX_MS2_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	=  IO_ADDRESS(SNX_SYS_BASE),
		.pfn		= __phys_to_pfn(SNX_SYS_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	=  IO_ADDRESS(SNX_GPIO_BASE),
		.pfn		= __phys_to_pfn(SNX_GPIO_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	=  IO_ADDRESS(SNX_INTC_BASE),
		.pfn		= __phys_to_pfn(SNX_INTC_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	=  IO_ADDRESS(SNX_I2C0_BASE),
		.pfn		= __phys_to_pfn(SNX_I2C0_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	=  IO_ADDRESS(SNX_I2C1_BASE),
		.pfn		= __phys_to_pfn(SNX_I2C1_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	=  IO_ADDRESS(SNX_RTC_BASE),
		.pfn		= __phys_to_pfn(SNX_RTC_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	=  IO_ADDRESS(SNX_TIMER_BASE),
		.pfn		= __phys_to_pfn(SNX_TIMER_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	=  IO_ADDRESS(SNX_WTD_BASE),
		.pfn		= __phys_to_pfn(SNX_WTD_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	=  IO_ADDRESS(SNX_MOTOR_BASE),
		.pfn		= __phys_to_pfn(SNX_MOTOR_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	=  IO_ADDRESS(SNX_SSP0_BASE),
		.pfn		= __phys_to_pfn(SNX_SSP0_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	=  IO_ADDRESS(SNX_UART0_BASE),
		.pfn		= __phys_to_pfn(SNX_UART0_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	=  IO_ADDRESS(SNX_UART1_BASE),
		.pfn		= __phys_to_pfn(SNX_UART1_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
		.virtual	=  IO_ADDRESS(SNX_SDIO_BASE),
		.pfn		= __phys_to_pfn(SNX_SDIO_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, {
	                .virtual        =  IO_ADDRESS(SNX_EHCI_BASE),
	                .pfn            = __phys_to_pfn(SNX_EHCI_BASE),
	                .length         = SZ_4K,
	                .type           = MT_DEVICE
        	}, {
		.virtual	=  IO_ADDRESS(SNX_SSP1_BASE),
		.pfn		= __phys_to_pfn(SNX_SSP1_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE
	}, { 
		.virtual        =  IO_ADDRESS(SNX_UDC_BASE), 
		.pfn            = __phys_to_pfn(SNX_UDC_BASE), 
		.length         = SZ_4K, 
		.type           = MT_DEVICE 
	}
};

static unsigned long isp_mem_size = (0 * SZ_1M);
static int __init isp_mem_size_setup(char *str)
{
	char *endp;
	isp_mem_size  = memparse(str, &endp);
	return 0;
}
early_param("isp", isp_mem_size_setup);

static unsigned long vc_mem_size = (0 * SZ_1M);
static int __init vc_mem_size_setup(char *str)
{
	char *endp;
	vc_mem_size  = memparse(str, &endp);
	return 0;
}
early_param("vc", vc_mem_size_setup);

static unsigned long vo_mem_size = (0 * SZ_1M);
static int __init vo_mem_size_setup(char *str)
{
	char *endp;
	vo_mem_size  = memparse(str, &endp);
	return 0;
}
early_param("vo", vo_mem_size_setup);


struct resource reserved_video_mem[] = {
	[0] = {
		.name	= "isp",
		.start	= 0,
		.end	= -1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.name	= "vc",
		.start	= 0,
		.end	= -1,
		.flags	= IORESOURCE_MEM,
	},
	[2] = {
		.name	= "vo",
		.start	= 0,
		.end	= -1,
		.flags	= IORESOURCE_MEM,
	},
};
EXPORT_SYMBOL(reserved_video_mem);

void __init sn98600_map_io(void)
{
	if (isp_mem_size)	
		snx_bootmem_reserve(&reserved_video_mem[0], isp_mem_size);
	if (vc_mem_size)
		snx_bootmem_reserve(&reserved_video_mem[1], vc_mem_size);
	if (vo_mem_size)
		snx_bootmem_reserve(&reserved_video_mem[2], vo_mem_size);

	iotable_init(sn98600_io_desc, ARRAY_SIZE(sn98600_io_desc));
}

