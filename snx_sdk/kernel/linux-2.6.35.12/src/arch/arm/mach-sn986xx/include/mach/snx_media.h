
#ifndef __MACH_SNX_MEDIA_H__
#define __MACH_SNX_MEDIA_H__

#include <linux/types.h>

/*(1920*1088*12/8)*2bytes=6266880bytes ->align to 2^x ->8M*/
#if 1
#define SNX_ISP_SZ 	resource_size(&reserved_video_mem[0]) 
#define SNX_VC_SZ	resource_size(&reserved_video_mem[1])
#define SNX_VO_SZ	resource_size(&reserved_video_mem[2])
#else
#define SNX_ISP_SZ	(10 * SZ_1M)
#define SNX_VC_SZ	(16 * SZ_1M)
#define SNX_VO_SZ	(8 * SZ_1M)
#endif

struct snx_camera_pdata {
	unsigned long flags;
	unsigned long mclk_hz;
};

//extern u64 snx_device_dmamask;
extern struct resource reserved_video_mem[];

extern void snx_bootmem_reserve(struct resource *res, unsigned long sz);
extern int snx_declare_coherent_mem(struct device *dev, struct resource *res);

#endif /*__MACH_SNX_MEDIA_H__*/
