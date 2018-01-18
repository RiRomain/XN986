/*
 * mtdram - a test mtd device
 * Author: Alexander Larsson <alex@cendio.se>
 *
 * Copyright (c) 1999 Alexander Larsson <alex@cendio.se>
 * Copyright (c) 2005 Joern Engel <joern@wh.fh-wedel.de>
 *
 * This code is GPL
 *
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/ioport.h>
#include <linux/vmalloc.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/mtd/compatmac.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/mtdram.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/physmap.h>

#include <asm/io.h>

#include "snx_sd.h"

//static unsigned long total_size = 1;

//#define MTDRAM_TOTAL_SIZE (total_size * 1024)
//#define MTDRAM_ERASE_SIZE (erase_size * 1024)

#define MTD_WRITE_SIZE 512
#define MTD_ERASE_SIZE 512

//#ifdef MODULE
//module_param(total_size, ulong, 0);
//MODULE_PARM_DESC(total_size, "Total device size in KiB");
//module_param(erase_size, ulong, 0);
//MODULE_PARM_DESC(erase_size, "Device erase block size in KiB");
//#endif

// We could store these in the mtd structure, but we only support 1 device..
static struct mtd_info *mtd_info;

struct mtd_priv {
	
	struct mtd_info 	*mtd;
	struct mtd_partition 	*parts;
	int 			nr_parts;

	struct resource 	*res;
	void __iomem 		*base;
	int 			irq;
	uint64_t		size;
};

static int sd2nand_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	instr->state = MTD_ERASE_DONE;
	mtd_erase_callback(instr);
	return 0;
}

#if 0
static int sd2nand_point(struct mtd_info *mtd, loff_t from, size_t len,
		size_t *retlen, void **virt, resource_size_t *phys)
{
	if (from + len > mtd->size)
		return -EINVAL;

	/* can we return a physical address with this driver? */
	if (phys)
		return -EINVAL;

	*virt = mtd->priv + from;
	*retlen = len;
	return 0;
}

static void sd2nand_unpoint(struct mtd_info *mtd, loff_t from, size_t len)
{
}
#endif

static int sd2nand_read(struct mtd_info *mtd, loff_t from, size_t len,
		size_t *retlen, u_char *buf)
{
	if (from + len > mtd->size)
		return -EINVAL;

//	printk("sd2nand_read: buf = 0x%x from = %d len = %d\n", buf, from, len);
	*retlen = snx_sd_data_read(buf, from >> 9, len);
	return 0;
}

static int sd2nand_write(struct mtd_info *mtd, loff_t to, size_t len,
		size_t *retlen, const u_char *buf)
{
	if (to + len > mtd->size)
		return -EINVAL;

//	printk("sd2nand_write: buf = 0x%x to = %d len = %d\n", buf, to, len);
	*retlen = snx_sd_data_write(to >> 9, buf, len);

	return 0;
}

static int sd2nand_block_isbad(struct mtd_info *mtd, loff_t offs)
{
	return 0;
}


int sd2nand_init_device(struct mtd_info *mtd, uint64_t size, char *name, struct platform_device *pdev)
{

	struct mtd_priv *priv = mtd->priv;
#ifdef CONFIG_MTD_PARTITIONS
	struct physmap_flash_data *pdata = pdev->dev.platform_data;
	struct mtd_partition *parts;
	int nr_parts;
	static const char *part_probes[] = { "cmdlinepart", NULL };
#endif

	/* Setup the MTD structure */
	mtd->name = name;
	mtd->type = MTD_NANDFLASH;
	mtd->flags = MTD_WRITEABLE | MTD_NO_ERASE;
	mtd->size = size;
	mtd->writesize = MTD_WRITE_SIZE;
	mtd->erasesize = MTD_ERASE_SIZE;

	mtd->owner = THIS_MODULE;
	mtd->erase = sd2nand_erase;
//	mtd->point = sd2nand_point;
//	mtd->unpoint = sd2nand_unpoint;
	mtd->read = sd2nand_read;
	mtd->write = sd2nand_write;
	mtd->block_isbad = sd2nand_block_isbad;

	mtd->dev.parent = &pdev->dev;

#ifdef CONFIG_MTD_PARTITIONS
	nr_parts = parse_mtd_partitions(mtd, part_probes, &parts, 0);
	if (nr_parts > 0) {
		priv->parts 	= parts;
		priv->nr_parts 	= nr_parts;
	}
	else if (pdata->parts)
	{
		priv->parts 	= pdata->parts;
		priv->nr_parts 	= pdata->nr_parts;
	}
	else
	{
		printk(KERN_ERR "Nandflash Create Partition Table failure!");
		return -EINVAL;
	}
	add_mtd_partitions(mtd, priv->parts, priv->nr_parts);
#else
	if (add_mtd_device(mtd)) {
		return -EIO;
	}
#endif

	return 0;
}

static int __devinit snx_sd2nand_probe(struct platform_device *pdev)
{
	int err;
	struct mtd_priv *priv;
	struct resource *res;
	int size;

//	printk("sd2nand_probe --->\n");

//	if (!total_size)
//		return -EINVAL;

	/* Allocate some memory */
	mtd_info = kmalloc(sizeof(struct mtd_info), GFP_KERNEL);
	if (!mtd_info)
		return -ENOMEM;

	memset(mtd_info, 0, sizeof(*mtd_info));

	priv = kmalloc(sizeof(struct mtd_priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	mtd_info->priv = priv;

	res  = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	size = res->end - res->start + 1;

	priv->res = request_mem_region(res->start, size, pdev->name);

	if (priv->res == NULL) {
		printk("request_mem_region: cannot reserve register region\n");
		err = -ENOENT;
		return err;
	}

	priv->base = ioremap(res->start, size);
	if (priv->base == NULL) {
		printk("ioremap: cannot reserve register region\n");
		err = -EIO;
		goto release_mem_region;
	}

	priv->irq = platform_get_irq(pdev, 0);


	if(snxsd_init(&pdev->dev, priv->base, priv->irq, &priv->size)){
		printk("snxsd_init failed!\n");
		err = -EIO;
		goto iounmap;
	}

	err = sd2nand_init_device(mtd_info, priv->size, "SNX SD to Nand Flash", pdev);
	if (err) {
		kfree(mtd_info);
		mtd_info = NULL;
		return err;
	}
	platform_set_drvdata(pdev, mtd_info);

	return 0;
iounmap:
	iounmap(priv->base);
release_mem_region:
	release_mem_region(res->start, size);

	return err;
}

static int __devexit snx_sd2nand_remove(struct platform_device *pdev)
{
	struct mtd_priv *priv = mtd_info->priv;
	int size;

	if (priv->base != NULL) {
		iounmap(priv->base);
		priv->base = NULL;
	}

	size = priv->res->end - priv->res->start + 1;
	release_mem_region(priv->res->start, size);

	if (priv->res != NULL) {
		release_resource(priv->res);
		kfree(priv->res);
		priv->res = NULL;
	}

	if (mtd_info) {
		del_mtd_device(mtd_info);
		vfree(mtd_info->priv);
		kfree(mtd_info);
	}
	
	snxsd_exit();

	platform_set_drvdata(pdev, NULL);

	return 0;
}
static struct platform_driver snx_sd2nand_driver = {
	.probe	= snx_sd2nand_probe,
	.remove = snx_sd2nand_remove,
	.driver = {
		.name	= "snx_sd2nand",
		.owner	= THIS_MODULE,
	},
};

static int __init snx_sd2nand_init(void)
{
#if 0
	int ret = 0;
	printk(KERN_INFO "SONIX SD2NAND Init!\n");
	ret = platform_driver_register(&snx_sd2nand_driver);
	if (ret) {
		printk("platform_driver_register failed!\n");
	}
	return ret;
#endif
}


static void __exit snx_sd2nand_exit(void)
{
#if 0
	platform_driver_unregister(&snx_sd2nand_driver);
#endif
}

module_init (snx_sd2nand_init);
module_exit (snx_sd2nand_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Yang Wu");
MODULE_DESCRIPTION ("SONIX SD to NAND flash MTD driver");
