#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/sysdev.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/bootmem.h>



void snx_bootmem_reserve(struct resource *res, unsigned long sz)
{
	void *virt_mem;
	dma_addr_t phy_addr;
	
	virt_mem = __alloc_bootmem(sz, PAGE_SIZE, 0x0);

	if (virt_mem != NULL) 
		phy_addr = virt_to_phys(virt_mem);
	else 
		phy_addr = (dma_addr_t)NULL;

	res->start = phy_addr?(phy_addr):(0);
	res->end = phy_addr?(phy_addr + sz - 1):(-1);
	
	return;
}
EXPORT_SYMBOL(snx_bootmem_reserve);

int snx_declare_coherent_mem(struct device *dev, struct resource *res)
{
	int sz, dma;
	dma_addr_t dma_handle;
	
	sz = resource_size(res);
	dma_handle = res->start;
	dma = dma_declare_coherent_memory(dev, dma_handle, dma_handle, sz,
					DMA_MEMORY_MAP | DMA_MEMORY_EXCLUSIVE);
	printk(KERN_INFO "0x%08x bytes system memory reserved "
			"for %s device at 0x%08x\n", sz, res->name, dma_handle);

	return dma & DMA_MEMORY_MAP ? 0 : -ENOMEM;
}
EXPORT_SYMBOL(snx_declare_coherent_mem);


