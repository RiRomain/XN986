/* linux/drivers/mtd/nand/snx_nand.c
 *
 *  Overview:
 *   priv is the MTD driver for SONiX NAND flash devices. It should be
 *   capable of working with almost all NAND chips currently available.
 *   Basic support for Samsung chips is provided.
 *
 * Copyright (c) 2009 SONiX
 *	http://www.sonix.com.tw
 *	Saxen Ko <saxenko@sonix.com.tw>
 *
 * SONiX NAND flash driver
 *
 * Changelog:
 *	23-Feb-2009  BJD  Initial version
 *
 * $Id: snx_nand.c,v 1.00 2009/02/23 16:06:49 bjd Exp $
 *
 * priv program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
      
#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/physmap.h>

#include <asm/sizes.h>
#include <asm/io.h>

#include <mach/regs-ms.h>
#include <mach/ms.h>
//#include "nand_reg.h"
#include <linux/mtd/nand_reg.h>

#if 0
#define SONIX_NAND_MAGIC	'n'  /*!< Magic number of the nand driver */
#define SONIX_NAND_GET		_IOR(SONIX_NAND_MAGIC, 1, unsigned int)
#define SONIX_NAND_SET		_IOW(SONIX_NAND_MAGIC, 2, unsigned int)
#define SONIX_NAND_DIR		_IOWR(SONIX_NAND_MAGIC, 3, unsigned int)
#endif

#define PAGE_GPIO		0
#define NAND_IRQ_NAM			"snx_nand"
void __iomem	*nand_base_addr;

#define snx_nand_readl(host, port)              __raw_readl((host->base + port))
#define snx_nand_writel(host, port, v)          __raw_writel(v, (host->base + port))


#ifdef MTD_DEBUG
#define DBG(x...) printk(KERN_INFO x)
#else
#define DBG(x) do { } while (0)
//#define DBG(fmt,args...) printk("[snxnand]:" fmt, ##args);
#endif

//#define DEBUG_ME
#ifdef DEBUG_ME
#define DBG_ME(x...) 	printk(KERN_INFO x)
#else
#define DBG_ME(x...)   do { } while (0)
#endif

#define ECC_ERR_INTER			1
#define ECC_ERR_NONE			0

#define OOB_FREE_POS_4K		104
#define OOB_FREE_POS_2K		2
#define OOB_FREE_POS_512	4	//3

unsigned int OOB_FREE_POS = OOB_FREE_POS_2K;
unsigned int NF_PageSize = 1;
unsigned int NF_BlockSize = 3;

#define STATUS_XFER_NONE 0
#define STATUS_XFER_DONE 1
static struct mtd_info *snx_mtd = NULL;

struct snx_nand_priv {
	struct nand_chip		chip;
	struct platform_device	*pdev;
	struct mtd_partition	*parts;
	struct resource			*area;
	void __iomem			*regs;
	dma_addr_t				dmaaddr;
	unsigned char			*dmabuf;
	unsigned int        base;
	int						use_dma;
	int						irq;
	wait_queue_head_t		nand_wq;
	int						xfer_statue;
	int				ecc_intererr;
};

#define ADD_UDELAY_1  1

//=========================================================================
// Set MS1 operation mode. 
// [0]:GPIO; [1]:NF; [2]:SD; [3]:SF 
//=========================================================================
static void snx_nand_setmsmode (struct snx_nand_priv* priv, enum ms1_mode mode)
{
	u32 data = 0;
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	//data = inw ((u32 *)MS1_CTL);
	data = snx_nand_readl (priv, MS1_CTL);
	data &= ~MS1_CTL_MODE_MASK;
	data |= (mode << MS1_CTL_MODE_BIT);
	snx_nand_writel (priv, MS1_CTL, data);
  DBG_ME ("%s:%d: write to MS1_CTL = %x\n",__func__, __LINE__, data);
}

//=========================================================================
// Set MS1 register r/w mode. 
// [0]:write; [1]:read 
//=========================================================================
static void snx_nand_regrw (struct snx_nand_priv* priv, enum ms1_reg_rw_mode mode)
{
	u32 data;
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	data = snx_nand_readl (priv, MS1_CTL);
	data &= ~MS1_CTL_REGRW_MASK;
	data |= (mode << MS1_CTL_REGRW_BIT);
	snx_nand_writel (priv, MS1_CTL, data);
  DBG_ME ("%s:%d: write to MS1_CTL = %x\n",__func__, __LINE__, data);
}

//=========================================================================
// Enable extra ECC for MS1.  
//=========================================================================
static void snx_nand_extraswitch (struct snx_nand_priv* priv, u32 isEnable)
{
	u32 data;
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	data = snx_nand_readl (priv, MS1_CTL);
	data &= ~MS1_CTL_EXTRAECC_MASK;
	data |= (isEnable << MS1_CTL_EXTRAECC_BIT);
	snx_nand_writel (priv, MS1_CTL, data);
  DBG_ME ("%s:%d: write to MS1_CTL = %x\n",__func__, __LINE__, data);
}

//=========================================================================
// Check MDMA finisih timeout for MS1.  0x50 bit2 
//=========================================================================
u32 snx_nand_clearmdmatimeout (struct snx_nand_priv* priv)
{
	u32 data;
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	data = snx_nand_readl (priv, MS1_MDMAECC);
	data &= MS1_MDMATIMEOUT_MASK;
	data = data >> MS1_MDMATIMEOUT_BIT;
  DBG_ME ("%s:%d: return value = %x\n",__func__, __LINE__, data);
	return data;
}
//=========================================================================
// Enable ECC or CRC for MS1.  
//=========================================================================
static void snx_nand_eccswitch (struct snx_nand_priv* priv, u32 isEnable)
{
	u32 data;
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	data = snx_nand_readl (priv, MS1_CTL);
	data &= ~MS1_CTL_ECCEN_MASK;
	data |= (isEnable << MS1_CTL_ECCEN_BIT);
	snx_nand_writel (priv, MS1_CTL, data);
  DBG_ME ("%s:%d: write to MS1_CTL = %x\n",__func__, __LINE__, data);
}

//=========================================================================
// Enable DMA for MS1.  
//=========================================================================
static void snx_nand_dmaswitch (struct snx_nand_priv* priv, u32 isEnable)
{
	u32 data;
	DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	data = snx_nand_readl (priv, MS1_CTL);
	data &= ~MS1_CTL_DMAEN_MASK;
	data |= (isEnable << MS1_CTL_DMAEN_BIT);
	snx_nand_writel (priv, MS1_CTL, data);
  DBG_ME ("%s:%d: write to MS1_CTL = %x\n",__func__, __LINE__, data);
}

//=========================================================================
// Set MS1 DMA r/w mode. 
// [0]:write; [1]:read 
//=========================================================================
static void snx_nand_dmarw (struct snx_nand_priv* priv, u32 value)
{
	u32 data;
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	data = snx_nand_readl (priv, MS1_CTL);
	data &= ~MS1_CTL_DMARW_MASK;
	data |= (value << MS1_CTL_DMARW_BIT);
	snx_nand_writel (priv, MS1_CTL, data);
  DBG_ME ("%s:%d: write to MS1_CTL = %x\n",__func__, __LINE__, data);
}

//=========================================================================
// Check r/w ready for MS1.  
//=========================================================================
static u32 snx_nand_checkmsready (struct snx_nand_priv* priv)
{
	u32 data;
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	data = snx_nand_readl (priv, MS1_CTL);
	data &= MS1_CTL_RDY_MASK;
	data >>= MS1_CTL_RDY_BIT;
  DBG_ME ("%s:%d: return value = %x\n",__func__, __LINE__, data);
	return data;
}

//=========================================================================
// MS1 set DMA size.
// transfer length: (DMA_SIZE + 1)byte
//=========================================================================
static void snx_nand_dmasize (struct snx_nand_priv* priv, u32 size)
{
	u32 data;
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	data = snx_nand_readl (priv, MS1_DMA_SIZE);
	data &= ~MS1_DMA_SIZE_MASK;
	data |= (size << MS1_DMA_SIZE_BIT);
	snx_nand_writel (priv, MS1_DMA_SIZE, data);
  DBG_ME ("%s:%d: write to MS1_DMA_SIZE = %x\n",__func__, __LINE__, data);
}

//=========================================================================
// MS1 set DMA block number.
// DMA block numbers = DMA_BLOCk + 1
//=========================================================================
static void snx_nand_dmablock (struct snx_nand_priv* priv, u32 dmaBlockNum)
{
	u32 data;
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	data = snx_nand_readl (priv, MS1_DMA_BLKSU);
	data &= ~MS1_DMA_BLOCK_MASK;
	data |= (dmaBlockNum << MS1_DMA_BLOCK_BIT);
	snx_nand_writel (priv, MS1_DMA_BLKSU, data);
  DBG_ME ("%s:%d: write to MS1_DMA_BLKSU = %x\n",__func__, __LINE__, data);
}

//=========================================================================
// MS1 set time count.
// Time is = clk_cycle * (TIME_CNT * 4)  
//=========================================================================
static void snx_nand_settimecount (struct snx_nand_priv* priv, u32 timeCnt)
{
	u32 data;
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	data = snx_nand_readl (priv, MS1_TIME_CNT);
	data &= ~MS1_TIME_CNT_MASK;
	data |= (timeCnt << MS1_TIME_CNT_BIT);
	snx_nand_writel (priv, MS1_TIME_CNT, data);
  DBG_ME ("%s:%d: write to MS1_TIME_CNT = %x\n",__func__, __LINE__, data);
}

//=========================================================================
// Enable MDMA for MS1.  
//=========================================================================
#if 0
static void snx_nand_mdmaswitch (struct snx_nand_priv* priv, u32 isEnable)
{
	u32 data;
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	data = snx_nand_readl (priv, MS1_MDMAECC);
	data &= ~MS1_MDMAEN_MASK;
	data |= (isEnable << MS1_MDMAEN_BIT);
	snx_nand_writel (priv, MS1_MDMAECC, data);
  DBG_ME ("%s:%d: write to MS1_MDMAECC = %x\n",__func__, __LINE__, data);
}
#endif
//=========================================================================
// Check CRC_W err for MS1. 
// SD: Write Data CRC Check Error
// NF: 2 bit upon, ECC err correct is faild 
//=========================================================================
static u32 snx_nand_checkeccerrcnt (struct snx_nand_priv* priv)
{
	u32 data,ret;
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	ret = snx_nand_readl (priv, MS1_MDMAECC);
	data = ret;
	ret &= MS1_CRCERR_MASK;
	ret >>= MS1_CRCERR_BIT;
	if(ret){
		data = data | (1 << MS1_CRCERR_BIT);
		snx_nand_writel (priv, MS1_MDMAECC, data);
		printk ("%s:%d ms check ecc err  ret\n",__func__, __LINE__);
	}
  DBG_ME ("%s:%d:  return value = %x\n",__func__, __LINE__, ret);
	return ret;
}

//=========================================================================
// Check ECC err for MS1.  
// SD: Read Data CRC Check Error
// NF: 1 bit, ECC err can correct 
//=========================================================================
static u32 snx_nand_checkeccerr (struct snx_nand_priv* priv)
{
	u32 data,ret;
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	ret = snx_nand_readl (priv, MS1_MDMAECC);
	data = ret;
	ret &= MS1_ECCERR_MASK;
	ret >>= MS1_ECCERR_BIT;
	if(ret){
		data = data | (1 << MS1_ECCERR_BIT);
		snx_nand_writel (priv, MS1_MDMAECC, data);
    DBG_ME ("%s:%d ms check ecc err  ret\n",__func__, __LINE__);
	}
	DBG_ME ("%s:%d:  return value = %x\n",__func__, __LINE__, ret);
	return ret;
}
static void snx_nand_setspeed(struct snx_nand_priv* priv, u32 speed)
{
	u32 data;
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	data = snx_nand_readl (priv, MS1_CTL);
	data &= ~MS1_CTL_NF_SPEED_MASK;
	data |= (speed << MS1_CTL_NF_SPEED_BIT);
	snx_nand_writel (priv, MS1_CTL, data);
  DBG_ME ("%s:%d: write to MS1_CTL = %x\n",__func__, __LINE__, data);
}

//=========================================================================
// MS1 set DMA addr.
// Please set DMA_ADDR[1:0] = 0 to meet 4 byte alignment.
//=========================================================================
static void snx_nand_dmaaddr (struct snx_nand_priv* priv, u32 addr)
{
	//addr &= 0xfffffffc;
	snx_nand_writel (priv, MS1_DMAADDR, addr);
  DBG_ME ("%s:%d: write to MS1_DMAADDR = %x\n",__func__, __LINE__, addr);
}

//=========================================================================
// MS1 write command.
//=========================================================================
static void snx_nand_writecommand (struct snx_nand_priv* priv, u32 cmd)
{
	cmd &= 0xff; 
	snx_nand_writel (priv, MS1_REG_CMD, cmd);
  DBG_ME ("%s:%d: write to MS1_REG_CMD = %x\n",__func__, __LINE__, cmd);
}

//=========================================================================
// MS1 write addr.  
//=========================================================================
static void snx_nand_writeaddr (struct snx_nand_priv* priv, u32 addr)
{
	addr &= 0xff;
	snx_nand_writel (priv, MS1_REG_ADDR, addr);
  DBG_ME ("%s:%d: write to MS1_REG_ADDR = %x\n",__func__, __LINE__, addr);
}

//=========================================================================
// MS1 write data.  
//=========================================================================
static void snx_nand_writedata (struct snx_nand_priv* priv, u32 data)
{
	data &= 0xff;
	snx_nand_writel (priv, MS1_REG_DATA, data);
  DBG_ME ("%s:%d: write to MS1_REG_DATA = %x\n",__func__, __LINE__, data);
}

//=========================================================================
// MS1 read data.  
//=========================================================================
static u32 snx_nand_readdata (struct snx_nand_priv* priv, u32 value)
{
	snx_nand_writel (priv, MS1_REG_DATA, value);
  
	return snx_nand_readl (priv, MS1_REG_DATA);
}

//=========================================================================
// Set nibble command for NF.
// [0]:reset NF; [1]:erase NF in multi-plane mode;
// [2]:erase NF; [3]:read INFO data from NF
// others: reserved
//=========================================================================
static void snx_nand_setnibblecmd (struct snx_nand_priv* priv, enum nibble_cmd mode)
{
	u32 data;
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	data = snx_nand_readl (priv, MS1_NF_CMD);	
	data &= ~NAND_CMD_NIBBLE_MASK;
	data |= (mode << NAND_CMD_NIBBLE_BIT);
	snx_nand_writel (priv, MS1_NF_CMD, data);
  DBG_ME ("%s:%d: write to MS1_NF_CMD = %x\n",__func__, __LINE__, data);
}

//=========================================================================
// Trigger to start nibble command for NF.
//=========================================================================
static void snx_nand_trgNibblecmd (struct snx_nand_priv* priv)
{
	u32 data;
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	data = snx_nand_readl (priv, MS1_CTL);
	data &= ~MS1_CTL_CMD_NIBBLE_TRG_MASK;
	data |= (1 << MS1_CTL_CMD_NIBBLE_TRG_BIT);
	snx_nand_writel (priv, MS1_CTL, data);
  DBG_ME ("%s:%d: write to MS1_CTL = %x\n",__func__, __LINE__, data);
}

//=========================================================================
// Read low pules width for NF.
// Range: 1~3
//=========================================================================
static void snx_nand_setrdwidth (struct snx_nand_priv* priv, u32 val)
{
	u32 data;
	DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);		
	data = snx_nand_readl (priv, MS1_CTL);
	data &= ~MS1_CTL_RDWIDTH_MASK;
	data |= (val << MS1_CTL_RDWIDTH_BIT);
	snx_nand_writel (priv, MS1_CTL, data);
  DBG_ME ("%s:%d: write to MS1_CTL = %x\n",__func__, __LINE__, data);
}

//=========================================================================
// Write low pules width for NF.
// Range: 1~3
//=========================================================================
static void snx_nand_setwrwidth (struct snx_nand_priv* priv, u32 val)
{
	u32 data;
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);		
	data = snx_nand_readl (priv, MS1_CTL);
	data &= ~MS1_CTL_WRWIDTH_MASK;
	data |= (val << MS1_CTL_WRWIDTH_BIT);
	snx_nand_writel (priv, MS1_CTL, data);
  DBG_ME ("%s:%d: write to MS1_CTL = %x\n",__func__, __LINE__, data);
}

//=========================================================================
// Set info data r/w mode for NF.
// DMA write:
// [0]:info data won't be written to NF
// [1]:info data will be written to NF
// DMA read:
// [0]:info data won't read into register
// [1]:info data will be read into register
//=========================================================================
static void snx_nand_setnfinfomode (struct snx_nand_priv* priv, u32 cmd)
{
	u32 data;
	DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	data = snx_nand_readl (priv, MS1_CTL);
	data &= ~MS1_CTL_NF_INFOWR_MASK;
	data |= (cmd << MS1_CTL_NF_INFOWR_BIT);
	snx_nand_writel (priv, MS1_CTL, data);
  DBG_ME ("%s:%d: write to MS1_CTL = %x\n",__func__, __LINE__, data);
}

//=========================================================================
// Set addr cycle for NF.
// [0]:addr cycle = 3; [1]:addr cycle = 4;
// [2]:addr cycle = 5; others:reserved
//=========================================================================
static void snx_nand_setaddrcyc (struct snx_nand_priv* priv, u32 val)
{
	u32 data;
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	data = snx_nand_readl (priv, MS1_NF_CMD);
	data &= ~NAND_CMD_ADR_CYC_MASK;
	data |= (val << NAND_CMD_ADR_CYC_BIT);
	snx_nand_writel (priv, MS1_NF_CMD, data);	
  DBG_ME ("%s:%d: write to MS1_NF_CMD = %x\n",__func__, __LINE__, data);
}

//=========================================================================
// Set reading command count for NF.
// [0]: one command when read.
// [1]: two commands when read. (MLC)
//=========================================================================
static void snx_nand_setreadcmdcnt (struct snx_nand_priv* priv, u32 readCmdCnt)
{
	u32 data;
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	data = snx_nand_readl (priv, MS1_NF_CMD);
	data &= ~NAND_CMD_RD_CMD_CNT_MASK;
	data |= (readCmdCnt << NAND_CMD_RD_CMD_CNT_BIT);
	snx_nand_writel (priv, MS1_NF_CMD, data);
  DBG_ME ("%s:%d: write to MS1_NF_CMD = %x\n",__func__, __LINE__, data);
}

//=========================================================================
// Set page size for NF.
// [0]:512 B/page; [1]:2k B/page;
// [2]:4k B/page; others:reserved
//=========================================================================
static void snx_nand_setpagesize (struct snx_nand_priv* priv, u32 value)
{
	u32 data;
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	data = snx_nand_readl (priv, MS1_NF_CMD);
	data &= ~NAND_CMD_PG_SIZE_MASK;
	data |= (value << NAND_CMD_PG_SIZE_BIT);
	snx_nand_writel (priv, MS1_NF_CMD, data);
  DBG_ME ("%s:%d: write to MS1_NF_CMD = %x\n",__func__, __LINE__, data);
}

//=========================================================================
// Set block size for NF.
// [0]:16 page; [1]:32 page;
// [2]:64 page; [3]:128 page
//=========================================================================
static void snx_nand_setblocksize (struct snx_nand_priv* priv, u32 value)
{	
	u32 data;
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	data = snx_nand_readl (priv, MS1_NF_CMD);
	data &= ~NAND_CMD_BLK_SIZE_MASK;
	data |= (value << NAND_CMD_BLK_SIZE_BIT);
	snx_nand_writel (priv, MS1_NF_CMD, data);
  DBG_ME ("%s:%d: write to MS1_NF_CMD = %x\n",__func__, __LINE__, data);
}

//=========================================================================
// Set LBA increment for NF.
// [0]:No inc; [1]:inc 1 for every DMA block;
// [2]:inc 1 for every 2 DMA block; [3]:inc 1 for every 4 DMA block;
// [4]:inc 1 for every 8 DMA block; [5]:inc 1 for every 16 DMA block;
// [6]:inc 1 for every 32 DMA block; [7]:inc 1 for every 64 DMA block;
// [8]:inc 1 for every 128 DMA block; others: reserved
//=========================================================================
static void snx_nand_setlbaincmode (struct snx_nand_priv* priv, u32 cmd)
{
	u32 data;
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	data = snx_nand_readl (priv, MS1_NF_CMD);
	data &= ~NAND_CMD_LBA_INC_MASK;
	data |= (cmd << NAND_CMD_LBA_INC_BIT);
	snx_nand_writel (priv, MS1_NF_CMD, data);
  DBG_ME ("%s:%d: write to MS1_NF_CMD = %x\n",__func__, __LINE__, data);
}

static void snx_nand_clear_intr_flags(struct snx_nand_priv* priv)
{
	int data;
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	data = snx_nand_readl (priv, MS1_MDMAECC);
	
	if(data & MS1_RDYFLAG_MASK){
		data |= 1 << MS1_CLR_RDY_BIT;
		data &= ~0xc0;// don't clear faile and ecc flag
	}
	if(data & MS1_ECCFLAG_MASK){
		priv->ecc_intererr = ECC_ERR_INTER;
		data |= 1 << MS1_CLR_ECC_BIT;
		data &= ~0xc0;// don't clear faile and ecc flag
	}
	snx_nand_writel (priv, MS1_MDMAECC, data);
  DBG_ME ("%s:%d: write to MS1_MDMAECC = %x\n",__func__, __LINE__, data);
}

static void snx_nand_enable_interrupts(struct snx_nand_priv* priv, u32 isEnable)
{
	int data;
	DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	data = snx_nand_readl (priv, MS1_MDMAECC);
	data &= ~MS1_RDYEN_MASK;
	//data &= ~MS1_ERREN_MASK;
	data &= ~MS1_ECCEN_MASK;
	data |= isEnable << MS1_RDYEN_BIT;
	//data |= 1 << MS1_ERREN_BIT;
	data |= isEnable << MS1_ECCEN_BIT;
	snx_nand_writel (priv, MS1_MDMAECC, data);
  DBG_ME ("%s:%d: write to MS1_MDMAECC = %x\n",__func__, __LINE__, data);
}

static int snx_nand_open(struct inode *inode, struct file *file)
{
        file->private_data = NULL;
        return 0;
}

static int snx_nand_release(struct inode *inode, struct file *file)
{
        return 0;
}

struct gpio_pin_info{
	unsigned int pinumber;	//pin number
	unsigned int mode;	//0:input 1:output
	unsigned int value;	//0:low 1:high
};

#define NANDGPIO_GPIO_WRITE	1
#define NANDGPIO_GPIO_READ	0

static int ms_nand_io(unsigned int num, unsigned int mode, unsigned int value)
{
	unsigned int data = 0;

	if (mode) {
		/* GPIO output mode, mode = 1 */
		data = ioread32(nand_base_addr + MS1_GPIO_OE);
		data |= 0x1 << num;
		iowrite32 (data, nand_base_addr + MS1_GPIO_OE);

		data = ioread32(nand_base_addr + MS1_GPIO_O);
		if (value)
			data |= 0x1 << num;	/* pull high */
		else
			data &= ~(0x1 << num);	/* pull low */

		iowrite32 (data, nand_base_addr + MS1_GPIO_O);
	} else {
		/* GPIO input mode, mode = 0 */
		data = ioread32(nand_base_addr + MS1_GPIO_OE);
		data &= ~(0x1 << num);
		iowrite32 (data, nand_base_addr + MS1_GPIO_OE);

		/* Get data from MS_IO_I*/
		data = ioread32(nand_base_addr + MS1_GPIO_I);

		return (data >> num) & 0x1;	/* return value is 0 or 1 */
	}

	return -1;
}

static int snx_nand_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	struct gpio_pin_info info;
	int val = 0;

	if (copy_from_user(&info, (void __user *)arg, sizeof(info)))
		return -EFAULT;

	switch (cmd) {
	case NANDGPIO_GPIO_WRITE:
		ms_nand_io(info.pinumber, info.mode, info.value);

		break;
	case NANDGPIO_GPIO_READ:
		val = ms_nand_io(info.pinumber, info.mode, info.value);
		info.value = val;

		if (copy_to_user((void __user *)arg, &info, sizeof(info)))
			return -EFAULT;

		break;
	default:
		return -ENOIOCTLCMD;
	}

	return 0;
}

static uint8_t bbt_pattern[] = {'B', 'b', 't', '0' };
static uint8_t mirror_pattern[] = {'1', 't', 'b', 'B' };

//
// controller and mtd information
//
static struct nand_ecclayout nand_hw_eccoob_512 = {
	.eccbytes = 3,
	.eccpos = { 0, 1, 2 },
	.oobfree = {
		{.offset = 4,	// reserve 1 byte
		 .length = 12}}	// reserve 1 byte, left 12 bytes
};

static struct nand_ecclayout nand_hw_eccoob_2048 = {
	.eccbytes = 24,
	.eccpos = {
		2,  3,  4,  5,  6,  7,  8,  9,
		10, 11, 12 ,13,14,15,16,17,18,19,20,21,22,23,24,25},
	.oobfree = {
		{.offset = 52,
		 .length = 12}}
};

static struct nand_ecclayout nand_hw_eccoob_4096 = {
	.eccbytes = 13,
	.eccpos = {
		0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
		10, 11, 12 },
	.oobfree = {
		{.offset = 16,	// FIXME: can't indicate over 64
		 .length = 12}}
};

//static uint8_t jffs2_pattern[] = {0x85, 0x19, 0x03, 0x20, 0x08, 0x00, 0x00, 0x00};



static struct nand_bbt_descr snx_bbt_main_descr = {
	.options = NAND_BBT_VERSION | NAND_BBT_LASTBLOCK | NAND_BBT_WRITE |
			NAND_BBT_2BIT | NAND_BBT_PERCHIP | NAND_BBT_CREATE,
	.offs = 8,
	.len = 4,
	.veroffs = 12,
	.maxblocks = NAND_BBT_SCAN_MAXBLOCKS,
	.pattern = bbt_pattern,
};

static struct nand_bbt_descr snx_bbt_mirror_descr = {
	.options = NAND_BBT_VERSION | NAND_BBT_LASTBLOCK | NAND_BBT_WRITE |
			NAND_BBT_2BIT | NAND_BBT_PERCHIP | NAND_BBT_CREATE,
	.offs = 8,
	.len = 4,
	.veroffs = 12,
	.maxblocks = 4,
	.pattern = mirror_pattern,
};

static struct nand_bbt_descr snx_bbt_main_descr_2K = {
	.options = NAND_BBT_VERSION | NAND_BBT_LASTBLOCK | NAND_BBT_WRITE |
			NAND_BBT_2BIT | NAND_BBT_PERCHIP | NAND_BBT_CREATE,
	.offs = 56,
	.len = 4,
	.veroffs = 60,
	.maxblocks = NAND_BBT_SCAN_MAXBLOCKS,
	.pattern = bbt_pattern,
};

static struct nand_bbt_descr snx_bbt_mirror_descr_2K = {
	.options = NAND_BBT_VERSION | NAND_BBT_LASTBLOCK | NAND_BBT_WRITE |
			NAND_BBT_2BIT | NAND_BBT_PERCHIP | NAND_BBT_CREATE,
	.offs = 56,
	.len = 4,
	.veroffs = 60,
	.maxblocks = NAND_BBT_SCAN_MAXBLOCKS,
	.pattern = mirror_pattern,
};

static struct nand_bbt_descr snx_bbt_main_descr_4K = {
	.options = NAND_BBT_VERSION | NAND_BBT_LASTBLOCK | NAND_BBT_WRITE |
			NAND_BBT_2BIT | NAND_BBT_PERCHIP | NAND_BBT_CREATE,
	.offs = 108,
	.len = 4,
	.veroffs = 112,
	.maxblocks = NAND_BBT_SCAN_MAXBLOCKS,
	.pattern = bbt_pattern,
};

static struct nand_bbt_descr snx_bbt_mirror_descr_4K = {
	.options = NAND_BBT_VERSION | NAND_BBT_LASTBLOCK | NAND_BBT_WRITE |
			NAND_BBT_2BIT | NAND_BBT_PERCHIP | NAND_BBT_CREATE,
	.offs = 108,
	.len = 4,
	.veroffs = 112,
	.maxblocks = NAND_BBT_SCAN_MAXBLOCKS,
	.pattern = mirror_pattern,
};

#ifdef CONFIG_MTD_PARTITIONS
static const char *part_probes[] = { "cmdlinepart", NULL };
#endif

static irqreturn_t nandflash_interrupt_handler(int irq, void *data)
{ 
	struct snx_nand_priv *priv = data;
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	//DBG("Enter nandflash_interrupt_handler..\n");
	snx_nand_clear_intr_flags(priv);
	priv->xfer_statue |= STATUS_XFER_DONE;
	wake_up_interruptible(&priv->nand_wq);
	
	return IRQ_HANDLED;
}

static int snx_nand_correct_data(struct mtd_info *mtd, u_char *dat,
				     u_char *read_ecc, u_char *calc_ecc);
//
// IO Functions for SONiX plateform
//

void nand_cmd_nibble(int cmd)
{
	long timeout;
	struct snx_nand_priv *priv = snx_mtd->priv;
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	snx_nand_regrw(priv, NAND_WRITE_MODE);
	// Write command/address to Nand
	snx_nand_settimecount(priv, 0x100000);
	snx_nand_setnibblecmd(priv, cmd);
	snx_nand_trgNibblecmd(priv);

	priv->xfer_statue = STATUS_XFER_NONE;
	timeout = wait_event_interruptible_timeout(
		priv->nand_wq,\
		(priv->xfer_statue & STATUS_XFER_DONE),
		1000);
#if ADD_UDELAY_1    
  udelay (1);
#endif  
	while (!snx_nand_checkmsready(priv));
}

/* snx_nand_hwcontrol
 *
 * Issue command and address cycles to the chip
*/
static void snx_nand_hwcontrol(struct mtd_info *mtd, int cmd,
				   unsigned int ctrl)
{
	struct snx_nand_priv *priv = snx_mtd->priv;
	snx_nand_regrw(priv, NAND_WRITE_MODE);	// (ctrl & NAND_CLE)
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	if (cmd == NAND_CMD_NONE)
	{
		snx_nand_setmsmode(priv, NF_MODE);
		snx_nand_setaddrcyc(priv, 1);
		snx_nand_setlbaincmode(priv, 0);
		snx_nand_setspeed(priv, 0x00);
		snx_nand_setrdwidth(priv, 0x00);
		snx_nand_setwrwidth(priv, 0x00);
		snx_nand_setpagesize(priv, 1);
		snx_nand_setblocksize(priv,  2);
		snx_nand_setreadcmdcnt(priv, 0);
		snx_nand_setnfinfomode(priv, 1);
		snx_nand_dmablock(priv, 0);
		snx_nand_eccswitch(priv, NAND_ECC_DISABLE);
		snx_nand_extraswitch(priv, NAND_EXTRA_ENABLE);
		snx_nand_eccswitch(priv, NAND_ECC_ENABLE);
		
		snx_nand_dmaswitch(priv, NAND_DMA_DISABLE);
		snx_nand_dmarw(priv, NAND_DMA_DISABLE);	
		udelay(1);
		return;
	}

	if (ctrl & NAND_ALE) // set addr
		snx_nand_writeaddr(priv, cmd);
	else	// set cmd
		snx_nand_writecommand(priv, cmd);
}

// *********************************************************************
// over-ride the standard functions for SONiX plateform. We can
// use read/write block to move the data buffers to/from the controller
// ---------------------------------------------------------------------

static void snx_nand_read_buf(struct mtd_info *mtd, u_char *buf, int len)
{
	int i;
  
	
	struct snx_nand_priv *priv = snx_mtd->priv;
  
	struct nand_chip *chip = (struct nand_chip *) &(priv->chip);
	DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
		snx_nand_regrw(priv, NAND_READ_MODE);
		udelay(chip->chip_delay);
#if ADD_UDELAY_1    
    udelay (1);
#endif    
		while (!snx_nand_checkmsready(priv))
		udelay(chip->chip_delay>>2);
		for (i = 0; i < len; i++)
			buf[i] = snx_nand_readdata(priv, 0x00);

	
}

static void snx_nand_write_buf(struct mtd_info *mtd, const u_char *buf, int len)
{
	int i;
	
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	snx_nand_regrw(mtd->priv, NAND_WRITE_MODE);
	for (i = 0; i < len; i++)
		snx_nand_writedata(mtd->priv, buf[i]);
}

static uint8_t snx_nand_read_byte(struct mtd_info *mtd)
{
	int ret;
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	snx_nand_regrw(mtd->priv, NAND_READ_MODE);
	ret = snx_nand_readdata(mtd->priv, 0x00);
  DBG_ME ("%s:%d: return value = %d\n", __func__, __LINE__,ret);
	return ret;
}

/**
 * snx_nand_devready - [DEFAULT] device check ready
 * @mtd:	MTD info structure
 *
 * check device is standby for using
 *
 * returns 0 if the nand is busy, 1 if it is ready
 */
static int snx_nand_devready(struct mtd_info *mtd)
{
#if ADD_UDELAY_1    
  udelay (1);
#endif
	return snx_nand_checkmsready(mtd->priv);
}

static int snx_waitfunc(struct mtd_info *mtd, struct nand_chip *this)
{
	int status=0;
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	while ((status&0xc1)!=0xc0)
	{
		snx_nand_regrw(mtd->priv, NAND_WRITE_MODE);
		snx_nand_writecommand(mtd->priv, 0x70);
		snx_nand_regrw(mtd->priv, NAND_READ_MODE);
		status=snx_nand_readdata(mtd->priv, 0x00);
	}
	return NAND_STATUS_READY;
}

static int snx_nand_read_oob(struct mtd_info *mtd, struct nand_chip *chip,
			     int page, int sndcmd)
{
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);

  
	//printk("Enter snx_nand_read_oob at page 0x%x\n", page);
	memset(chip->oob_poi, 0xff, mtd->oobsize);
  
	if (sndcmd)
	{
		snx_nand_regrw(mtd->priv, NAND_WRITE_MODE);
		chip->cmdfunc(mtd, NAND_CMD_READ0, mtd->writesize, page);
		sndcmd = 0;
	}
	chip->read_buf(mtd, chip->oob_poi, mtd->oobsize);
  
  
  
	return 0;
}

static int snx_nand_write_oob(struct mtd_info *mtd, struct nand_chip *chip,
			      int page)
{
//  printk("w,");
  
  
	struct nand_oobfree *free = chip->ecc.layout->oobfree;
  
	int length = free->length;
	int offset = free->offset;
	
	int status = 0;
  
	const uint8_t *buf = chip->oob_poi;
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	//printk("Enter snx_nand_write_oob at page 0x%x, offset=%d\n", page, offset);
	snx_nand_regrw(mtd->priv, NAND_WRITE_MODE);
	chip->cmdfunc(mtd, NAND_CMD_SEQIN, (mtd->writesize + offset), page);
	chip->write_buf(mtd, (buf+offset), length);
	// Send command to program the OOB data
	chip->cmdfunc(mtd, NAND_CMD_PAGEPROG, -1, -1);

	status = chip->waitfunc(mtd, chip);
  
 
  
	return status & NAND_STATUS_FAIL ? -EIO : 0;
}

static void snx_nand_write_page(struct mtd_info *mtd,
					  struct nand_chip *chip, const uint8_t *buf)
{
	int stat;
  
	long timeout;
  
	struct nand_oobfree *free = chip->ecc.layout->oobfree;
  
	int length = free->length;
	int offset = free->offset;
  
	struct snx_nand_priv *priv = mtd->priv;
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	memcpy(priv->dmabuf, buf, mtd->writesize);
	
	snx_nand_regrw(priv, NAND_WRITE_MODE);
	snx_nand_setlbaincmode(priv, 0);
	snx_nand_dmasize(priv, mtd->writesize -1);
	snx_nand_dmarw(priv, NAND_DMA_WRITE);
	snx_nand_dmaaddr(priv, priv->dmaaddr);
	snx_nand_dmablock(priv, 0);

	// Enable DMA
	snx_nand_dmaswitch(priv, NAND_DMA_ENABLE);
	priv->xfer_statue = STATUS_XFER_NONE;
	timeout = wait_event_interruptible_timeout(
		priv->nand_wq,\
		(priv->xfer_statue & STATUS_XFER_DONE),
		1000);
	
	if(timeout ==0 || priv->ecc_intererr == ECC_ERR_INTER)
		priv->ecc_intererr = ECC_ERR_NONE;
#if ADD_UDELAY_1    
  udelay (1);
#endif    
	while (!snx_nand_checkmsready(priv));

	snx_nand_dmaswitch(priv, NAND_DMA_DISABLE);

	stat = snx_nand_correct_data(mtd, 0, 0, 0);
	if (stat == -1)
		mtd->ecc_stats.failed++; 
	else
		mtd->ecc_stats.corrected += stat;
	//write oob data
	snx_nand_regrw(mtd->priv, NAND_WRITE_MODE);
	chip->cmdfunc(mtd, NAND_CMD_RNDIN, (mtd->writesize + offset), 0);
	chip->write_buf(mtd, (chip->oob_poi+offset), length);
  

}



static int snx_nand_read_page(struct mtd_info *mtd, struct nand_chip *chip,
			       uint8_t *buf, int page )
{
	int stat;
  
  

  
	long timeout;
  
	struct snx_nand_priv *priv = mtd->priv;
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	chip->cmdfunc(mtd, NAND_CMD_READ0, 0, page);

	snx_nand_regrw(priv, NAND_WRITE_MODE);
	snx_nand_dmasize(priv, mtd->writesize  -1);
	snx_nand_dmarw(priv, NAND_DMA_READ);
	snx_nand_dmaaddr(priv, priv->dmaaddr);
	snx_nand_dmablock(priv, 0);

	
	// Enable DMA
	snx_nand_dmaswitch(priv, NAND_DMA_ENABLE);

	priv->xfer_statue = STATUS_XFER_NONE;
	timeout = wait_event_interruptible_timeout(
		priv->nand_wq,\
		(priv->xfer_statue & STATUS_XFER_DONE),
		1000);
//	printk("\nEnter snx_nand_read_page=0x%x,  timeout=%d\n",page,timeout);
	if(timeout ==0 || priv->ecc_intererr == ECC_ERR_INTER)
		priv->ecc_intererr = ECC_ERR_NONE;
#if ADD_UDELAY_1    
  udelay (1);
#endif	                           
	while (!snx_nand_checkmsready(priv));
	snx_nand_dmaswitch(priv, NAND_DMA_DISABLE);
 
	memcpy(buf, priv->dmabuf, mtd->writesize);
	stat = snx_nand_correct_data(mtd, 0, 0, 0);

	if (stat == -1)
		mtd->ecc_stats.failed++;
	else
		mtd->ecc_stats.corrected += stat;

	snx_nand_regrw(mtd->priv, NAND_WRITE_MODE);
	chip->cmdfunc(mtd, NAND_CMD_READ0, mtd->writesize, 0);	
	chip->read_buf(mtd, chip->oob_poi, mtd->oobsize); // 64


  
	return 0;
}


static void snx_nand_enable_hwecc(struct mtd_info *mtd, int mode)
{
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	snx_nand_eccswitch(mtd->priv, NAND_ECC_DISABLE);
	snx_nand_extraswitch(mtd->priv, NAND_EXTRA_ENABLE);
	snx_nand_eccswitch(mtd->priv, NAND_ECC_ENABLE);
}

static int snx_nand_calculate_ecc(struct mtd_info *mtd, const u_char *dat, u_char *ecc_code)
{
//	DBG("Enter snx_nand_calculate_ecc\n");
	return 0;
}

static int snx_nand_correct_data(struct mtd_info *mtd, u_char *dat,
				     u_char *read_ecc, u_char *calc_ecc)
{
	int failNum, errNum;
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
//	DBG("Enter snx_nand_correct_data\n");

	failNum=snx_nand_checkeccerrcnt(mtd->priv);
	if(failNum){
    printk ("%s:%d Nand ECC Failure: fail number = %x\n",__func__, __LINE__,failNum); 
	}

	errNum=snx_nand_checkeccerr(mtd->priv);
	if(errNum){
    printk ("%s:%d Nand ECC Error: error number = %x\n",__func__, __LINE__,errNum);
    
	}
	if (errNum) // we curently have no method for correcting the error
		return -1;
	else if (failNum) // detect error but fixed
		return failNum;
	else	// success
		return 0;
}

static int snx_nand_verify_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	long timeout;
	struct snx_nand_priv *priv = snx_mtd->priv;
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	// DMA Transfer
	snx_nand_regrw(priv, NAND_WRITE_MODE);
	snx_nand_setlbaincmode(priv, 0);
	snx_nand_dmasize(priv, len);
	snx_nand_dmarw(priv, NAND_DMA_READ);
	snx_nand_dmaaddr(priv, priv->dmaaddr);

	//printk("snx_nand_verify_buf\n");
	udelay(55);
	// Enable DMA
	snx_nand_dmaswitch(priv, NAND_DMA_ENABLE);

//	udelay(50);
	priv->xfer_statue = STATUS_XFER_NONE;
	timeout = wait_event_interruptible_timeout(
		priv->nand_wq,\
		(priv->xfer_statue & STATUS_XFER_DONE),
		1000);
	//printk("snx_nand_verify_buf\n");
#if ADD_UDELAY_1    
  udelay (1);
#endif  
	while (!snx_nand_checkmsready(priv));

	snx_nand_dmaswitch(priv, NAND_DMA_DISABLE);

	if (memcmp(buf, priv->dmabuf, len) != 0)
			return -EFAULT;
	return 0;
}

static void snx_command(struct mtd_info *mtd, unsigned command, int column, int page_addr)
{
	struct snx_nand_priv *priv = mtd->priv;
	struct nand_chip *chip = (struct nand_chip *) &(priv->chip);

  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
		
	/*
	 * program and erase have their own busy handlers
	 * status and sequential in needs no delay
	 */
	 chip->cmd_ctrl(mtd, command, NAND_CLE);
	
	switch (command) {
	case NAND_CMD_RNDOUT:
		chip->cmd_ctrl(mtd, column, NAND_ALE);
		chip->cmd_ctrl(mtd, column>>8, NAND_ALE);
		chip->cmd_ctrl(mtd, NAND_CMD_RNDOUTSTART, NAND_CLE);
		/*udelay(1000);*/
		return;
	case NAND_CMD_RNDIN:
		chip->cmd_ctrl(mtd, column, NAND_ALE);
		chip->cmd_ctrl(mtd, column>>8, NAND_ALE);
		/*udelay(1000);*/
		return;
	case NAND_CMD_ERASE1:
		chip->cmd_ctrl(mtd, page_addr, NAND_ALE);
		chip->cmd_ctrl(mtd, page_addr>>8, NAND_ALE);
		return;
	case NAND_CMD_PAGEPROG:
		/*udelay(200);*/
		return;
	case NAND_CMD_SEQIN:
		chip->cmd_ctrl(mtd, column, NAND_ALE);
		chip->cmd_ctrl(mtd, column>>8, NAND_ALE);
		chip->cmd_ctrl(mtd, page_addr, NAND_ALE);
		chip->cmd_ctrl(mtd, page_addr>>8, NAND_ALE);
		//printk("the cmd is 0x%x   colume=0x%x,  pageaddr=0x%x\n",command, column,page_addr);
		/*udelay(1000);*/
		return;
	case NAND_CMD_STATUS:
		snx_nand_regrw(priv, NAND_WRITE_MODE);
		chip->cmd_ctrl(mtd, command, NAND_CLE);
		return;
	case NAND_CMD_ERASE2:
		/*for(i=0;i<10;i++){
			udelay(1000);
		}*/
		return;
	case NAND_CMD_READID:
		chip->cmd_ctrl(mtd, 0x00, NAND_ALE);
		return;
	case NAND_CMD_READ0:
		chip->cmd_ctrl(mtd, column, NAND_ALE);
		chip->cmd_ctrl(mtd, column>>8, NAND_ALE);
		chip->cmd_ctrl(mtd, page_addr, NAND_ALE);
		chip->cmd_ctrl(mtd, page_addr>>8, NAND_ALE);
		chip->cmd_ctrl(mtd, 0x30, NAND_CLE);
		//printk("read data column=0x%x   page_addr=0x%x\n",column, page_addr);
		/*udelay(1000);*/
		udelay(50);
		return;
	case NAND_CMD_RESET:
#if 0  
		if (chip->dev_ready)
			break;
#endif      
		udelay(chip->chip_delay);
		chip->cmd_ctrl(mtd, NAND_CMD_STATUS,
			       NAND_CTRL_CLE | NAND_CTRL_CHANGE);
		chip->cmd_ctrl(mtd,
			       NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);
		while (!(chip->read_byte(mtd) & NAND_STATUS_READY)) ;
		//printk("the cmd is reset ok\n");
		return;

		/* This applies to read commands */
	default:
		/*
		 * If we don't have access to the busy pin, we apply the given
		 * command delay
		 */
		if (!chip->dev_ready) {
			udelay(chip->chip_delay);
			return;
		}
	}
	/* Apply this short delay always to ensure that we do wait tWB in
	 * any case on any machine. */

}

/**
 * snx_nand_init_chip - [DEFAULT] device init
 * @mtd:	MTD info structure
 *
 * init a single instance of an chip
 */
static void snx_nand_init_chip(struct nand_chip *chip)
{
	struct snx_nand_priv *priv = snx_mtd->priv;
	//*****************************************
	// Set address of hardware control function
	//-----------------------------------------
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	chip->state = FL_READY;

	chip->dev_ready = snx_nand_devready;

	chip->read_byte = snx_nand_read_byte;
	chip->read_buf	= snx_nand_read_buf;
	chip->write_buf = snx_nand_write_buf;

	chip->cmd_ctrl	= snx_nand_hwcontrol;
	chip->cmdfunc	= snx_command;	// use default func
	chip->waitfunc	= snx_waitfunc;
	chip->verify_buf= snx_nand_verify_buf;

	// 50 us command delay time 
	chip->chip_delay = 50;
	chip->options = NAND_NO_AUTOINCR | NAND_USE_FLASH_BBT;

	// ECC initial
	chip->ecc.mode		= NAND_ECC_HW;
	chip->ecc.steps	= 1;
	chip->ecc.bytes	= 12;
	chip->ecc.total	= 0;
	chip->ecc.size		= 2048;
	chip->ecc.layout	= &nand_hw_eccoob_512;

	chip->ecc.hwctl	= snx_nand_enable_hwecc;
	chip->ecc.calculate = snx_nand_calculate_ecc;
	chip->ecc.correct   = snx_nand_correct_data;

	chip->ecc.write_page = snx_nand_write_page;
	chip->ecc.read_page	= snx_nand_read_page;
	chip->ecc.write_oob = snx_nand_write_oob;
	chip->ecc.read_oob	= snx_nand_read_oob;

	chip->bbt_td		= &snx_bbt_main_descr;
	chip->bbt_md		= &snx_bbt_mirror_descr;

	snx_nand_setmsmode(priv, NF_MODE);
	snx_nand_setaddrcyc(priv, 1);
	snx_nand_eccswitch(priv, 0);
	snx_nand_setlbaincmode(priv, 0);
	snx_nand_setspeed(priv, 0x00);
	snx_nand_setrdwidth(priv, 0x00);
	snx_nand_setwrwidth(priv, 0x00);
	snx_nand_setpagesize(priv, 1);
	snx_nand_setblocksize(priv,  2);
	snx_nand_setreadcmdcnt(priv, 0);
	snx_nand_setnfinfomode(priv, 1);

	
}

static struct file_operations snx_nand_fops=
{
	.owner		= THIS_MODULE,
	.ioctl		= snx_nand_ioctl,
	.open		= snx_nand_open,
	.release	= snx_nand_release,
};

static struct miscdevice snx_nand_miscdev=
{
	.minor		= MISC_DYNAMIC_MINOR,
	.name		= "nandgpio",
	.fops		= &snx_nand_fops,
};

/**
 * snx_nand_remove - [DEFAULT] device remove
 * @pdev:	plateform device structure
 *
 * remove a single instance of an chip
 */
static int __devexit snx_nand_remove(struct platform_device *pdev)
{
	struct snx_nand_priv *priv = snx_mtd->priv;

	misc_deregister(&snx_nand_miscdev);

	if (!snx_mtd)
		return 0;

	snx_nand_enable_interrupts(priv, 0);
	// free nandflash IRQ
	free_irq(priv->irq, priv);

	if (priv->area != NULL) {
		release_resource(priv->area);
		kfree(priv->area);
		priv->area = NULL;
	}

	del_mtd_partitions(snx_mtd);

	del_mtd_device(snx_mtd);
	nand_release(snx_mtd);
	if (priv->dmabuf)
		dma_free_coherent(&pdev->dev, 4096, priv->dmabuf, priv->dmaaddr);

	kfree(snx_mtd);

	return 1;
}

/**
 * snx_nand_probe - [DEFAULT] device probe
 * @pdev:	plateform device structure
 *
 * called by device layer when it finds a device matching
 * one our driver can handled. priv code checks to see if
 * it can allocate all necessary resources then calls the
 * nand layer to look for devices
 */
static int snx_nand_probe(struct platform_device *pdev)
{
  
	struct snx_nand_priv *priv;
	struct nand_chip *chip;
	int ret = 0;
  
	int err = 0;
  
#ifdef CONFIG_MTD_PARTITIONS
	struct physmap_flash_data *pdata = pdev->dev.platform_data;
	struct mtd_partition *parts;
	int nr_parts;
#endif
	int size;
  DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	snx_mtd = kzalloc (sizeof(struct mtd_info) + sizeof(struct snx_nand_priv), GFP_KERNEL);
	if (!snx_mtd) {  
    printk ("%s:%d Unable to allocate NAND MTD info structure\n",__func__, __LINE__);
		err = -ENOMEM;
		return err;
	}

	// Get pointer to private data
	priv = (struct snx_nand_priv *)(&snx_mtd[1]);
	chip = (struct nand_chip *) &(priv->chip);

	// Link the private data with the MTD structure
	snx_mtd->priv = priv;
	snx_mtd->owner = THIS_MODULE;

	priv->pdev = (struct platform_device *) pdev;

	priv->area = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(priv->area == NULL){ 
    printk ("%s:%d get io resource memory error\n",__func__, __LINE__);
		err = -ENOENT;
		goto err_free_mtd;
	}
		
	size = priv->area->end - priv->area->start + 1;
	priv->ecc_intererr = ECC_ERR_NONE;

	priv->area = request_mem_region(priv->area->start, size, pdev->name);
	if (priv->area == NULL) {
    printk ("%s:%d cannot reserve register region\n",__func__, __LINE__);  
		err = -ENOENT;
		goto err_free_mtd;
	}

	priv->base = IO_ADDRESS(priv->area->start);

	priv->regs       = ioremap(priv->area->start, size);
	nand_base_addr = (void __iomem *)priv->base;

	if (priv->regs == NULL) { 
    printk ("%s:%d cannot reserve register region\n",__func__, __LINE__);
		err = -EIO;
		goto err_free_resourse;
	}

	platform_set_drvdata(pdev, priv);
	// initial interrupt
	snx_nand_enable_interrupts(priv, 1);
	init_waitqueue_head(&priv->nand_wq);
	priv->irq = platform_get_irq(pdev, 0);
	if(priv->irq == 0){
    printk ("%s:%d failed to get ms nand flash host controller irq\n",__func__, __LINE__);
    goto err_free_resourse;
  }

	err = request_irq(priv->irq, nandflash_interrupt_handler, IRQF_SHARED, NAND_IRQ_NAM, priv);
	if (err < 0)
	{            
		printk(KERN_ERR "snx_nand_probe : register Nandflash interrupt - Fail!\n");
		goto err_free_resourse;
	}

	snx_nand_init_chip(chip);

	chip->IO_ADDR_W = (priv->regs+0x78);
	chip->IO_ADDR_R = chip->IO_ADDR_W;

	if (dma_set_mask (&pdev->dev, DMA_BIT_MASK(32)))
	{
		priv->use_dma = 1;
		pdev->dev.coherent_dma_mask = DMA_BIT_MASK(32);  
		printk("mydev: DMA is supported\n");
	}
	else
	{
		printk("mydev: DMA not supported\n");
		priv->use_dma = 0; /* We'll have to live without DMA */
	}

	// alloc DMA buffer
	priv->dmabuf = dma_alloc_coherent(&pdev->dev, 4096, &priv->dmaaddr, GFP_KERNEL | GFP_DMA);
	if (!priv->dmabuf) {
		printk("dma_alloc_coherent error!\n");
		err = -ENOMEM;
		goto err_free_irq;
	}

		// Check, if we should skip the bad block table scan
	if (chip->options & NAND_SKIP_BBTSCAN)
		return 0;
	if(nand_scan_ident(snx_mtd, 1, NULL)){
	//if (nand_scan(snx_mtd, 1)) {
		printk("nand_scan_ident error!\n");
		err = -ENXIO;
		goto exit_error;
	}

	
	switch (snx_mtd->writesize)
		{
		case 512:
			chip->ecc.size	= 512;
			chip->chip_delay = 15;
			OOB_FREE_POS = OOB_FREE_POS_512;
			chip->ecc.layout	= &nand_hw_eccoob_512;
			NF_PageSize = 0;	// 512 byte
			chip->bbt_td		= &snx_bbt_main_descr;
			chip->bbt_md		= &snx_bbt_mirror_descr;
			break;
		case 2048:
			chip->ecc.size	= 2048;
	//		this->ecc.bytes = 13;
			chip->chip_delay = 25;
			OOB_FREE_POS = OOB_FREE_POS_2K;
			chip->ecc.layout	= &nand_hw_eccoob_2048;
			NF_PageSize = 1;	// 2K
			chip->bbt_td		= &snx_bbt_main_descr_2K;
			chip->bbt_md		= &snx_bbt_mirror_descr_2K;
			break;
		case 4096:
			chip->ecc.size	= 4096;
			chip->chip_delay = 50;
			OOB_FREE_POS = OOB_FREE_POS_4K;
			chip->ecc.layout	= &nand_hw_eccoob_4096;
			NF_PageSize = 2;	// 4K
			chip->bbt_td		= &snx_bbt_main_descr_4K;
			chip->bbt_md		= &snx_bbt_mirror_descr_4K;
			break;
		default:
			// not supported.
			err = -ENXIO;
		}
		snx_mtd->ecclayout = chip->ecc.layout;

		// setup block size
		switch (chip->bbt_erase_shift - chip->page_shift)
		{
		case 7:
			NF_BlockSize = 3;
			break;
		case 6:
			NF_BlockSize = 2;
			break;
		case 5:
			NF_BlockSize = 1;
			break;
		case 4:
			NF_BlockSize = 0;
			break;
		default:
			// not supported.
			err = -ENXIO;
		}
		snx_nand_setpagesize(priv, NF_PageSize);
		snx_nand_setblocksize(priv,  NF_BlockSize);
    
	if(nand_scan_tail(snx_mtd)){
		printk("nand_scan_tail error!\n");
		err = -ENXIO;
		goto exit_error;
	}

#ifdef CONFIG_MTD_PARTITIONS
	snx_mtd->name = "snx-nand";
	nr_parts = parse_mtd_partitions(snx_mtd, part_probes, &parts, 0);
	if (nr_parts > 0) {
		priv->parts = parts;
		add_mtd_partitions(snx_mtd, parts, nr_parts);
	}
	else if (pdata->parts)
	{
		priv->parts = pdata->parts;
		add_mtd_partitions(snx_mtd, pdata->parts, pdata->nr_parts);
	}
	else
	{
		printk(KERN_ERR "Nandflash Create Partition Table failure!");
		goto exit_error;
	}
#else
	if (add_mtd_device(snx_mtd)) {
		return -EIO;
	}
#endif
  
	ret = misc_register(&snx_nand_miscdev);

	return 0;

exit_error:
	dma_free_coherent(&pdev->dev, 4096, priv->dmabuf, priv->dmaaddr);
err_free_irq:
	free_irq(priv->irq, priv);
err_free_resourse:
	release_resource(priv->area);
	kfree(priv->area);
err_free_mtd:
	kfree(snx_mtd);
	
	return err;
}

static int snx_nand_suspend(struct platform_device *dev, pm_message_t pm)
{
	DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	return 0;
}

static int snx_nand_resume(struct platform_device *dev)
{
	DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	return 0;
}

static struct platform_driver snx_nand_driver = {
	.probe		= snx_nand_probe,
	.remove		= __devexit_p(snx_nand_remove),
	.suspend	= snx_nand_suspend,
	.resume		= snx_nand_resume,
	.driver		= {
		.name	= "snx_nand", //"snx_nand",
		.owner	= THIS_MODULE,
	},
};

static int __init snx_nand_init(void)
{
	printk("Load SONiX NAND Flash Driver, (c) 2009 SONiX.\n");
	return platform_driver_register(&snx_nand_driver);
}

static void __exit snx_nand_exit(void)
{
	printk("exit SONiX NAND Flash Driver!\n");
	platform_driver_unregister(&snx_nand_driver);
}

module_init(snx_nand_init);
module_exit(snx_nand_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Saxen Ko <SaxenKo@sonix.com.tw>");
MODULE_DESCRIPTION("SONiX MTD Nand flash driver");
