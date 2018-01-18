/*
 * SONIX EHCI HCD (Host Controller Driver) for USB.
 *
 * Modified for SONIX EHC
 *  by Jingdar Du <jingdar_du@sonix.com.tw>
 *
 * This file is licenced under the GPL.
 */

#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/ioctl.h>
#include <asm/uaccess.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/irq.h>


#define USB_HOST_CONFIG   (USB_MSR_BASE + USB_MSR_MCFG)
#define USB_MCFG_PFEN     (1<<31)
#define USB_MCFG_RDCOMB   (1<<30)
#define USB_MCFG_SSDEN    (1<<23)
#define USB_MCFG_PHYPLLEN (1<<19)
#define USB_MCFG_EHCCLKEN (1<<17)
#define USB_MCFG_UCAM     (1<<7)
#define USB_MCFG_EBMEN    (1<<3)
#define USB_MCFG_EMEMEN   (1<<2)

#define USBH_ENABLE_CE    (USB_MCFG_PHYPLLEN | USB_MCFG_EHCCLKEN)

#ifdef CONFIG_DMA_COHERENT
#define USBH_ENABLE_INIT  (USBH_ENABLE_CE \
                         | USB_MCFG_PFEN | USB_MCFG_RDCOMB \
                         | USB_MCFG_SSDEN | USB_MCFG_UCAM \
                         | USB_MCFG_EBMEN | USB_MCFG_EMEMEN)
#else
#define USBH_ENABLE_INIT  (USBH_ENABLE_CE \
                         | USB_MCFG_PFEN | USB_MCFG_RDCOMB \
                         | USB_MCFG_SSDEN \
                         | USB_MCFG_EBMEN | USB_MCFG_EMEMEN)
#endif
#define USBH_DISABLE      (USB_MCFG_EBMEN | USB_MCFG_EMEMEN)

extern int usb_disabled(void);

/*-------------------------------------------------------------------------*/

static void sonix_start_ehc(struct platform_device *dev)
{
	pr_debug(__FILE__ ": starting Au1xxx EHCI USB Controller\n");
#if 0
	/* write HW defaults again in case Yamon cleared them */
	if (au_readl(USB_HOST_CONFIG) == 0) {
		au_writel(0x00d02000, USB_HOST_CONFIG);
		au_readl(USB_HOST_CONFIG);
		udelay(1000);
	}
	/* enable host controller */
	au_writel(USBH_ENABLE_CE | au_readl(USB_HOST_CONFIG), USB_HOST_CONFIG);
	au_readl(USB_HOST_CONFIG);
	udelay(1000);
	au_writel(USBH_ENABLE_INIT | au_readl(USB_HOST_CONFIG),
		  USB_HOST_CONFIG);
	au_readl(USB_HOST_CONFIG);
	udelay(1000);
#endif


	pr_debug(__FILE__ ": Clock to USB host has been enabled\n");
}

static void sonix_stop_ehc(struct platform_device *dev)
{
	pr_debug(__FILE__ ": stopping Au1xxx EHCI USB Controller\n");

#if 0
	/* Disable mem */
	au_writel(~USBH_DISABLE & au_readl(USB_HOST_CONFIG), USB_HOST_CONFIG);
	udelay(1000);
	/* Disable clock */
	au_writel(~USB_MCFG_EHCCLKEN & au_readl(USB_HOST_CONFIG),
		  USB_HOST_CONFIG);
	au_readl(USB_HOST_CONFIG);
#endif
}

/*-------------------------------------------------------------------------*/

/* configure so an HC device and id are always provided */
/* always called with process context; sleeping is OK */

/**
 * usb_ehci_sonix_probe - initialize Au1xxx-based HCDs
 * Context: !in_interrupt()
 *
 * Allocates basic resources for this USB host controller, and
 * then invokes the start() method for the HCD associated with it
 * through the hotplug entry's driver_data.
 *
 */

int usb_ehci_sonix_probe(const struct hc_driver *driver,
			  struct usb_hcd **hcd_out, struct platform_device *dev)
{
	int retval = 0;
	struct usb_hcd *hcd;
	struct ehci_hcd *ehci;

	sonix_start_ehc(dev);
	if (dev->resource[1].flags != IORESOURCE_IRQ) {
		pr_debug("resource[1] is not IORESOURCE_IRQ");
		retval = -ENOMEM;
	}
	hcd = usb_create_hcd(driver, &dev->dev, "sonix-ehci");
	if (!hcd)
		return -ENOMEM;
	hcd->rsrc_start = dev->resource[0].start;
	hcd->rsrc_len = dev->resource[0].end - dev->resource[0].start + 1;

	if (!request_mem_region(hcd->rsrc_start, hcd->rsrc_len, hcd_name)) {
		pr_debug("request_mem_region failed");
		retval = -EBUSY;
		goto err1;
	}

	hcd->regs = ioremap(hcd->rsrc_start, hcd->rsrc_len);
	if (!hcd->regs) {
		pr_debug("ioremap failed");
		retval = -ENOMEM;
		goto err2;
	}

	ehci = hcd_to_ehci(hcd);
	ehci->caps = hcd->regs;
	ehci->regs = hcd->regs + HC_LENGTH(readl(&ehci->caps->hc_capbase));

	/* cache this readonly data; minimize chip reads */
	ehci->hcs_params = readl(&ehci->caps->hcs_params);

	/* ehci_hcd_init(hcd_to_ehci(hcd)); */
	retval = usb_add_hcd(hcd, dev->resource[1].start, IRQF_DISABLED | IRQF_SHARED);

	if (retval == 0)
		return retval;
	sonix_stop_ehc(dev);
	iounmap(hcd->regs);
err2:
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
err1:
	usb_put_hcd(hcd);
	return retval;
}

/* may be called without controller electrically present */
/* may be called with controller, bus, and devices active */

/**
 * usb_ehci_hcd_sonix_remove - shutdown processing for Au1xxx-based HCDs
 * @dev: USB Host Controller being removed
 * Context: !in_interrupt()
 *
 * Reverses the effect of usb_ehci_hcd_sonix_probe(), first invoking
 * the HCD's stop() method.  It is always called from a thread
 * context, normally "rmmod", "apmd", or something similar.
 *
 */
void usb_ehci_sonix_remove(struct usb_hcd *hcd, struct platform_device *dev)
{
	usb_remove_hcd(hcd);
	iounmap(hcd->regs);
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
	usb_put_hcd(hcd);
	sonix_stop_ehc(dev);
}

/*-------------------------------------------------------------------------*/

static const struct hc_driver ehci_sonix_hc_driver = {
	.description = hcd_name,
	.product_desc = "snx_ehci",
	.hcd_priv_size = sizeof(struct ehci_hcd),

	/*
	 * generic hardware linkage
	 */
	.irq = ehci_irq,
	.flags = HCD_MEMORY | HCD_USB2,

	/*
	 * basic lifecycle operations
	 */
	.reset = ehci_init,
	.start = ehci_run,
	.stop  = ehci_stop,

	/*
	 * managing i/o requests and associated device resources
	 */
	.urb_enqueue = ehci_urb_enqueue,
	.urb_dequeue = ehci_urb_dequeue,
	.endpoint_disable = ehci_endpoint_disable,

	/*
	 * scheduling support
	 */
	.get_frame_number = ehci_get_frame,

	/*
	 * root hub support
	 */
	.hub_status_data  = ehci_hub_status_data,
	.hub_control      = ehci_hub_control,

#ifdef	CONFIG_PM
	.hub_suspend = ehci_hub_suspend,
	.hub_resume  = ehci_hub_resume,
#endif
//	.clear_tt_buffer_complete = ehci_clear_tt_buffer_complete,

};

/*-------------------------------------------------------------------------*/

static int ehci_hcd_sonix_drv_probe(struct platform_device *pdev)
{
	struct usb_hcd *hcd = NULL;
	int ret;
	pr_debug("In ehci_hcd_sonix_drv_probe\n");

	if (usb_disabled())
	{
		return -ENODEV;
	}

	ret = usb_ehci_sonix_probe(&ehci_sonix_hc_driver, &hcd, pdev);

	return ret;
}

static int ehci_hcd_sonix_drv_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);

	usb_ehci_sonix_remove(hcd, pdev);
	return 0;
}

/*TBD*/
/*static int ehci_hcd_sonix_drv_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct usb_hcd *hcd = dev_get_drvdata(dev);

	return 0;
}
static int ehci_hcd_sonix_drv_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct usb_hcd *hcd = dev_get_drvdata(dev);

	return 0;
}
*/
MODULE_ALIAS("sonix-ehci");

/*
static struct resource snx_ehci_resources[] = {
        [0] = {
                .start  = (SNX_EHCI_BASE),		//(0x90800000),
                .end    = (SNX_EHCI_BASE+0x100),	//(0x90800000+0x100),
                .flags  = IORESOURCE_MEM,
         },
        [1] = {
                .start  = INT_EHCI,			//24,
                .end    = INT_EHCI,			//24,
                .flags  = IORESOURCE_IRQ,
         },

};

static struct sonix_device ehci_hcd_sonix_device = {
             .name           = SNX_DEVICE_NAME_EHCI,
             .id             = -1,
             .dev            = {
			.coherent_dma_mask      = 0xffffffff,
			.dma_mask		= 0xffffffff, //20091009
                        .bus = &sonix_bus_type_ahb,
             },
             .num_resources  = ARRAY_SIZE(snx_ehci_resources),
             .resource       = snx_ehci_resources,
};
*/

static struct platform_driver ehci_hcd_sonix_driver = {
	.probe = ehci_hcd_sonix_drv_probe,
	.remove = ehci_hcd_sonix_drv_remove,
	/*.suspend      = ehci_hcd_sonix_drv_suspend, */
	/*.resume       = ehci_hcd_sonix_drv_resume, */
	.driver = {
		.name = "snx_ehci",	//SNX_DEVICE_NAME_EHCI,
		.owner = THIS_MODULE,
		//.bus = &sonix_bus_type_ahb,
	},
};
