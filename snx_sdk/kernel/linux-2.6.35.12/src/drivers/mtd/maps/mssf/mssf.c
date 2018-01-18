/*
 * MTD serial flash driver for MX25L6405D flash chips
 *
 *  Author:     Nora Chang
 *  Created:    DEC. 25, 2008
 *  Copyright:  (C) 2008 SONIX Technology, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>
#include <linux/dma-mapping.h>
#include <linux/mtd/physmap.h>
#include <linux/platform_device.h>
#include <linux/semaphore.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/delay.h>

#include <asm/mach/flash.h>

#include "ms.h"
#include "mssf.h"

#include "generated/snx_sdk_conf.h"

#include <linux/miscdevice.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <asm/uaccess.h>


#ifdef CONFIG_MTD_PARTITIONS
static const char *part_probes[] = { /* "RedBoot", */ "cmdlinepart", NULL };
#endif

//MXIC
#define FLASH_PAGESIZE		256

/* Status Register bits. */
#define SR_WIP			1         /* Write in progress */
#define SR_WEL			2         /* Write enable latch */
#define SR_BP0			4         /* Block protect 0 */
#define SR_BP1			8         /* Block protect 1 */
#define SR_BP2			0x10      /* Block protect 2 */
#define SR_SRWD			0x80      /* SR write protect */

/* Define max times to check status register before we give up. */
#define	MAX_READY_WAIT_COUNT	100000 //100000
 #define	MAX_MDMA_WAIT_COUNT		100000 //100000
//#define	MAX_READY_WAIT_COUNT	0x10000000 //100000
//#define	MAX_MDMA_WAIT_COUNT		0x10000000 //100000

#ifdef CONFIG_MTD_PARTITIONS
#define	mtd_has_partitions()	(1)
#else
#define	mtd_has_partitions()	(0)
#endif


void snx_check_match_id (int id_type , int i);


struct mssf {
	struct semaphore	lock;
	struct mtd_info		mtd;
	struct sf_instruction	sf_inst;
	unsigned		partitioned;
	u8			command[4];
	struct map_info         map;
#ifdef CONFIG_MTD_PARTITIONS
	struct mtd_partition	*parts;
#endif
};

//init value    To do if add flash id table
#define EN25P64_8M	0
struct sf_id id_en_arry[] = {
			{0xef,0x30,0x17},
			};

#define MX25L12835E	0   //ID {0xc2,0x20,0x19}
#define MX25L12835F	1   //ID {0xc2,0x20,0x18}			
#define MX25L6406E	2   //ID {0xc2,0x20,0x17}
struct sf_id id_mixc_arry[] = {
			{0xc2,0x20,0x19}, /* MX25L12835E */
			{0xc2,0x20,0x18}, /* MX25L12835F */
			{0xc2,0x20,0x17}, /* MX258006E */
			};

#define GD25Q128C	0   //ID {0xc8,0x40,0x18}
#define GD25Q16B	1   //ID {0xc8,0x40,0x15}
struct sf_id id_gd_arry[] = {
			{0xc8,0x40,0x18}, /* GD25Q128C */
//			{0xc8,0x40,0x15}, /* GD25Q16B  - 2M */
			};

#define S25FL127S	0   //ID {0x01,0x20,0x18}
struct sf_id id_spanson_arry[] = {
			{0x01,0x20,0x18}, /* S25FL127S */
			};
#define PMC_64K  0 //Pm25LV512
struct sf_id id_pmc_arry[] = {{0x9d, 0x7b, 0x7f}}; //pmc

struct sf_id id_sst_arry[] = {{1,0,0}, {1,0,0}, {1,0,0}}; //pp aai aaw
#define W25Q128FVEF	0	//ID {0xef,0x40,0x18}
struct sf_id id_winbond_arry[] = {{0xef, 0x40, 0x18}};	//W25Q128FVEF

#define WB25Q256FV	0   //ID {0xc2,0x20,0x19}
struct sf_id id_wb_arry[] = {
			{0xef,0x40,0x19}, /* 256FV*/
			};

//#define DEBUG_ME
#ifdef DEBUG_ME
#define DBG_ME(x...) 	printk(KERN_INFO x)
#else
#define DBG_ME(x...)   do { } while (0)
#endif

//alek modify for support protect use
struct mssf *sf_flash;
#define SUPPORT_SF_PROTECT	0


/****************************************************************************/
struct flash_info {
	char		*name;
	u8		id;
	u16		jedec_id;
	unsigned	block_size;
	unsigned	sector_size;
	unsigned	n_blocks;
};

// default serial flash info
static struct flash_info __devinitdata snx_sf_data [] = {
	/* REVISIT: fill in JEDEC ids, for parts that have them */
	{ "serial flash", 0xff, 0xffff, 0x8000, 0x1000, 32},
	//MX25L6405D sector_size=4k, block_size=64k
};


#if 1
#define SFGPIO_GPIO_WRITE 	1
#define SFGPIO_GPIO_READ 	0

static int snx_sfgpio_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
static int snx_sfgpio_open(struct inode *inode, struct file *file);
static int snx_sfgpio_release(struct inode *inode, struct file *file);

static struct file_operations snx_sfgpio_fops=
{
  	.owner		= THIS_MODULE,
  	.ioctl		= snx_sfgpio_ioctl,
  	.open		= snx_sfgpio_open,
  	.release	= snx_sfgpio_release,
};

static struct miscdevice snx_sfgpio_miscdev=
{
	.minor          = MISC_DYNAMIC_MINOR,
	.name           = "sfgpio",
	.fops           = &snx_sfgpio_fops,
};

struct gpio_pin_info {
    unsigned int  pinumber;//pin number
    unsigned int  mode;	//0:input 1:output
    unsigned int  value;//0:low 1:high
};


static int read_sr(struct mssf *flash);
static int write_status_reg(struct mssf *flash, unsigned int status);
static int setqebit (struct mssf *flash,int en);
static void sfQPIDisable(void);
static void sfQPIEnable(void);


//#define CONFIG_FASRDTRD_AVAILABLE 1 	//MX25L12845E
//#define CONFIG_4DTRD_AVAILABLE 1 		//MX25L12845E
//#define CONFIG_QPI_AVAILABLE 1 		//MX25L12835F


static int snx_sfgpio_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
//	int ret = 0;
	struct gpio_pin_info info;
	unsigned int rdata;

	if (copy_from_user (&info, (void __user *)arg, sizeof(info)))
			return -EFAULT;

	switch(cmd){
		case SFGPIO_GPIO_WRITE:
			mssf_ms_io_wt (info.pinumber ,info.mode ,info.value);
			break;
		case SFGPIO_GPIO_READ:
			rdata = mssf_ms_io_rd (info.pinumber);
			info.value = rdata&0x01;
			info.mode = (rdata>>1);

			if (copy_to_user ((void __user *)arg, &info, sizeof(info)))
			return -EFAULT;

			break;
		 default:
			return -ENOIOCTLCMD;
	}
	return 0;
}

/** static int snx_sfgpio_open(struct inode *inode, struct file *filp)
 *  brief Open device
 */
static int snx_sfgpio_open(struct inode *inode, struct file *file)
{
	//need not switch pin mux	
	return 0;
}

/** static int snx_sfgpio_release(struct inode *inode, struct file *filp)
 *  brief Close device.
 */
static int snx_sfgpio_release(struct inode *inode, struct file *file)
{
	//mssf_ms_io_release();
	return 0;
}

#endif

static inline struct mssf *mtd_to_mssf(struct mtd_info *mtd)
{
	return container_of(mtd, struct mssf, mtd);
}

static int wait_till_msrdy_ready(void)
{
	unsigned int count;
	DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);

	for (count = 0; count < MAX_READY_WAIT_COUNT; count++) {
		if(mssf_check_msrdy())
			return 0;
	}

	DBG_ME ("%s:%d: return value = %x\n",__func__, __LINE__, 1);
        return 1;
}

static int wait_till_msdma_ok(void)
{
	unsigned int count;
	DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);

	for (count = 0; count < MAX_MDMA_WAIT_COUNT; count++) {
		if(mssf_check_msmdma_ok())
			return 0;
	}

	DBG_ME ("%s:%d: return value = %x\n",__func__, __LINE__, 1);
        return 1;
}



/*
ST58660FPGA ADD
-mdma
    -MX25L12835F : QPI
    -MX25L12845E : DT
*/

static void sfQPIEnable(void)
{
	unsigned int val;


#if 1
		//chip enable
	mssf_chip_enable_switch(MSSF_CHIP_ENABLE);
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data(0x35);

		if(wait_till_msrdy_ready())
		goto fail_msrdy_error;

		//chip disable
	mssf_chip_enable_switch(MSSF_CHIP_DISABLE);

	return;

#endif

#if 0
//////////////////////////////////////////////////////
	sfSetQPIMode(1);

	sf4BitsSwitch(1);


	//chip enable
	mssf_chip_enable_switch(MSSF_CHIP_ENABLE);

	//write command
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data(0x05);
	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;

	//read data
	mssf_msreg_rw_switch(MSSF_READ_MODE);
	mssf_write_data(0x0);
	if(wait_till_msrdy_ready())
 		goto fail_msrdy_error;
	val = mssf_read_data();

	//chip disable
	mssf_chip_enable_switch(MSSF_CHIP_DISABLE);
	printk("=========================%s:%d: return value = %x\n",__func__, __LINE__, val);


		//chip enable
	mssf_chip_enable_switch(MSSF_CHIP_ENABLE);

	//write command
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
//	mssf_write_data(0xAF);
	mssf_write_data(0x15);
	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;


		//read data
	mssf_msreg_rw_switch(MSSF_READ_MODE);

	mssf_write_data(0x0);
	if(wait_till_msrdy_ready())
 		goto fail_msrdy_error;
	val = mssf_read_data();

	printk("#%x\n",val);

	// 	mssf_write_data(0x0);
	// if(wait_till_msrdy_ready())
 // 		goto fail_msrdy_error;
	// val = mssf_read_data();

	// printk("#%x\n",val);

	// 	mssf_write_data(0x0);
	// if(wait_till_msrdy_ready())
 // 		goto fail_msrdy_error;
	// val = mssf_read_data();

	// printk("#%x\n",val);


		mssf_chip_enable_switch(MSSF_CHIP_DISABLE);

/////////////////////////////////////////////////////////
#endif

fail_msrdy_error :
	printk(" snx_sf_dma_readdata : fail_msrdy_error\n");
	mssf_chip_enable_switch(MSSF_CHIP_DISABLE);
	DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 1);
}

static void sfQPIDisable(void)
{
		//chip enable
	mssf_chip_enable_switch(MSSF_CHIP_ENABLE);
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data(0xF5);

		if(wait_till_msrdy_ready())
		goto fail_msrdy_error;

		//chip disable
	mssf_chip_enable_switch(MSSF_CHIP_DISABLE);

	return;

fail_msrdy_error :
	printk(" snx_sf_dma_readdata : fail_msrdy_error\n");
	mssf_chip_enable_switch(MSSF_CHIP_DISABLE);
	DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 1);
}


static int setqebit (struct mssf *flash,int en)
{
	int count;
	int sr;
	DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);

	for (count = 0; count < MAX_READY_WAIT_COUNT; count++) {
		if((sr = read_sr(flash)) < 0){
			break;
		}
    	else if(sr & SR_WEL){
     		return 0;
		}
	}

	if (en) {
		sr = sr | (1<<6);
	}else{
		sr = sr &(0xbf);
	}
	
	write_status_reg(flash, sr);

	DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 1);       
	return 1;
}

// MX25L12845E
// FASRDTRD (0D) - fast DT read (6 C)
// 4DTRD (ED) - Quad IO DT Read : sf_DT_4bit_mdma_read (8 C)

// MX25L12835F - QPI
// 4READ (EB) - 4 IO read - QPI mode (8/6 C)

void new_feature_support(struct mssf *flash)
{
	//MXIC : noramal, SST: AAI or AAW && need to set WiteCmd and TimeCount
	//mssf_set_wmode(0x0);//0x0: normal mode
	if(flash->sf_inst.typenum == SF_SST_AAI) //sf_inst->typenum
	{
		mssf_set_wmode (SF_AAI);
		mssf_set_wcmd (flash->sf_inst.AAI);
		mssf_set_timecount (flash->sf_inst.TBP);
	}
	else if(flash->sf_inst.typenum == SF_SST_AAW)
	{
		mssf_set_wmode (SF_AAW);
		mssf_set_wcmd (flash->sf_inst.AAW);
		mssf_set_timecount (flash->sf_inst.TBP);
	}
	else
		mssf_set_wmode (SF_NORMAL);

	sfSetAddrCyc(0);

	sfSetRDDummyByte(0);
	sfSetWRDummyByte(0);
	sfSetDummyCyc(0);
	sfSetDummyEN(0);
	sfSetCacheWcmd(flash->sf_inst.PP);
	sfSetCacheRcmd(flash->sf_inst.READSF);

	sfSetWEnCmd(flash->sf_inst.WREN);
	sfSetStatusCmd(flash->sf_inst.RDSR);

#ifdef CONFIG_FASRDTRD_AVAILABLE
	sfSetCacheRcmd(flash->sf_inst.FASTDTRD);

	sfSetDummyCyc(5);
	sfSetRDDummyByte(1);
//	sfSetDummyEN(1);
//	sfDoubleRateSwitch(1);
#endif

#ifdef CONFIG_4DTRD_AVAILABLE
//	setqebit (flash, 0);
//	setqebit (flash, 1);

	sfSetCacheRcmd(flash->sf_inst.DTRD4);
	sfSetDummyCyc(7);
	sfSetRDDummyByte(1);
//	sfSetDummyEN(1);

//	sfDoubleRateSwitch(1);
//	sf4BitsSwitch(1);
#endif

#ifdef CONFIG_QPI_AVAILABLE
	// sfSetCacheRcmd(flash->sf_inst.QPIREAD); //Quad IO Fast Read cyc=6
	// sfSetDummyCyc(5); 		
	// sfSetRDDummyByte(1);

	//FAST READ
	sfSetCacheRcmd(0x0B);	//Fast Read cyc=8
	sfSetDummyCyc(7); 		
	sfSetRDDummyByte(1);

//	sfSetDummyEN(1);
#endif
}




/*
	ST58660FPGA END
*/
/*
 * Read the status register, Return the status register value.
 * Returns negative if error occurred.
 */
static int read_sr(struct mssf *flash)
{
	u8 val;
	DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);

	//chip enable
	mssf_chip_enable_switch(MSSF_CHIP_ENABLE);

	//write command
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data(flash->sf_inst.RDSR);
	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;

	//read data
	mssf_msreg_rw_switch(MSSF_READ_MODE);
	mssf_write_data(0x0);
	if(wait_till_msrdy_ready())
 		goto fail_msrdy_error;
	val = mssf_read_data();

	//chip disable
	mssf_chip_enable_switch(MSSF_CHIP_DISABLE);
	DBG_ME ("%s:%d: return value = %x\n",__func__, __LINE__, val);

	return val;

fail_msrdy_error :
	mssf_chip_enable_switch(MSSF_CHIP_DISABLE);
	DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, -1);
	return -1;
}

static int wait_till_write_enable_ready(struct mssf *flash)
{
	int count;
	int sr;
	DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);

	for (count = 0; count < MAX_READY_WAIT_COUNT; count++) {
		if((sr = read_sr(flash)) < 0){
			break;
		}
    		else if(sr & SR_WEL){
     			return 0;
		}
	}
	DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 1);       
	return 1;
}

/*
 * Set write enable 
 * Returns negative if error occurred.
 */
static inline int write_enable(struct mssf *flash)
{
	//chip enable 
	DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	mssf_chip_enable_switch(MSSF_CHIP_ENABLE);

	//write command
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data(flash->sf_inst.WREN);
	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;

	//chip disable
	mssf_chip_enable_switch(MSSF_CHIP_DISABLE);
	DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 0); 

	return 0;

fail_msrdy_error :
        mssf_chip_enable_switch(MSSF_CHIP_DISABLE);
        DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 1); 
        return 1;
}

/*
 * Service routine to read status register until ready, or timeout occurs.
 * Returns non-zero if error.
 */
static int wait_till_ready(struct mssf *flash)
{
	int count;
	int sr;
	DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);

	for (count = 0; count < MAX_READY_WAIT_COUNT; count++) {
		if((sr = read_sr(flash)) < 0) {
			break;
		}
		else if(!(sr & SR_WIP))
			return 0;
	}

	DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 1);
	return 1;
}

static inline int write_extend_addr_register(struct mssf *flash, int value)
{
	if (write_enable(flash))
		goto fail_msrdy_error;

	mssf_chip_enable_switch(MSSF_CHIP_ENABLE);
	//write command
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data(0xc5);
	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;
	
	mssf_write_data(value);
	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;

	mssf_chip_enable_switch(MSSF_CHIP_DISABLE);
	return 0;

fail_msrdy_error:
	mssf_chip_enable_switch(MSSF_CHIP_DISABLE);
	return 1;
}

/*
 * Erase one sector or one block of flash memory at offset 
 * "offset" is any address within the sector or block which should be erased.
 * Returns 0 if successful, non-zero otherwise.
 */
static int erase_sector(struct mssf *flash, unsigned int offset)
{
	// Wait until finished previous write command.
	DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	if(wait_till_ready(flash))
		return 1;

	if (offset >= 0x1000000)
		write_extend_addr_register(flash, 1);	
	if(wait_till_ready(flash))
		return 1;

	// Send write enable, then erase commands.
	 if(write_enable(flash))
	        return 1;
	// printk("sector write enable ok\n");
	if(wait_till_write_enable_ready(flash))
		return 1;

	// Set up erase command.
	flash->command[0] = flash->sf_inst.SE;
	flash->command[1] = offset >> 16;
	flash->command[2] = offset >> 8;
	flash->command[3] = offset;

	//chip enable
	mssf_chip_enable_switch(MSSF_CHIP_ENABLE);

	//write instruction
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data(flash->command[0]);
	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;
	//printk("sector write cmd\n");

	//write addr_1
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data(flash->command[1]);
	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;

	//write addr_2
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data(flash->command[2]);
	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;

	//write addr_3
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data(flash->command[3]);
	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;
	//printk("addr write end\n");

	//chip disable
	mssf_chip_enable_switch(MSSF_CHIP_DISABLE);
	if (offset >= 0x1000000) {
		if(wait_till_ready(flash))
			return 1;
		write_extend_addr_register(flash, 0);	
	}

	DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 0);

	return 0;

fail_msrdy_error :
        mssf_chip_enable_switch(MSSF_CHIP_DISABLE);
        DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 1);
        return 1;
}

static int erase_block(struct mssf *flash, unsigned int offset)
{
	// Wait until finished previous write command.
	DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	if(wait_till_ready(flash))
		return 1;

	if (offset >= 0x1000000)
		write_extend_addr_register(flash, 1);		
	if(wait_till_ready(flash))
		return 1;
	// Send write enable, then erase commands.
	if(write_enable(flash))
		return 1;
	if(wait_till_write_enable_ready(flash))
		return 1;

	// Set up command buffer.
	flash->command[0] = flash->sf_inst.BE;
	flash->command[1] = offset >> 16;
	flash->command[2] = offset >> 8;
	flash->command[3] = offset;

	//chip enable
	mssf_chip_enable_switch(MSSF_CHIP_ENABLE);

	//write instruction
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data(flash->command[0]);
	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;

	//write addr_1
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data(flash->command[1]);
	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;

	//write addr_2
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data(flash->command[2]);
	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;

	//write addr_3
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data(flash->command[3]);
	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;

	//chip disable
	mssf_chip_enable_switch(MSSF_CHIP_DISABLE);
	if (offset >= 0x1000000) {
		if(wait_till_ready(flash))
			return 1;
		write_extend_addr_register(flash, 0);		
	}
	DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 0);

	return 0;

fail_msrdy_error :
        mssf_chip_enable_switch(MSSF_CHIP_DISABLE);
        DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 1);
        return 1;
}

static int write_status_reg(struct mssf *flash, unsigned int status)
{	
	//chip enable 
	DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	mssf_chip_enable_switch(MSSF_CHIP_ENABLE);

	//write command
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data(flash->sf_inst.WRSR);
	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;

	//write data
	mssf_write_data(status);
	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;

	//chip disable
	mssf_chip_enable_switch(MSSF_CHIP_DISABLE);

  	DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 0);
	return 0;

fail_msrdy_error :
        mssf_chip_enable_switch(MSSF_CHIP_DISABLE);
        DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 1);
        return 1;
}

int sf_wrsr_sst (struct mssf *flash)
{
	//change BP0 and BP1
	//EWSR
	//Set serial flash CE = 0
	DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	mssf_chip_enable_switch(MSSF_CHIP_ENABLE);

	//write instruction
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data (flash->sf_inst.EWSR);

	if(wait_till_msrdy_ready())
	        goto fail_msrdy_error;

	//Set serial flash CE = 1
	mssf_chip_enable_switch(MSSF_CHIP_DISABLE);


	//WRSR
	//Set serial flash CE = 0
	mssf_chip_enable_switch(MSSF_CHIP_ENABLE);

	//write instruction
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data (flash->sf_inst.WRSR);

	if(wait_till_msrdy_ready())
	        goto fail_msrdy_error;

	//write addr
	mssf_write_data (0x0);

	if(wait_till_msrdy_ready())
	        goto fail_msrdy_error;

	mssf_chip_enable_switch(MSSF_CHIP_DISABLE);

	DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 0);
	return 0;

fail_msrdy_error :
	mssf_chip_enable_switch(MSSF_CHIP_DISABLE);
	DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 1);
	return 1;
}

/////////////add cleansw protect fun/////////////////
void sf_modifysw_to_clean (struct mssf *sf_flash)
{
	write_status_reg(sf_flash, 0x0);
}

///////////add modify cleansw protectfun/////////////
void sf_modifysw_to_protect (struct mssf *sf_flash)
{
	write_status_reg(sf_flash, 0x3c);
}

static int sf_init(struct mssf *flash)
{
	int status;
	int count = 0;
	DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);

	//set to be serial flash mode
	mssf_set_msmode(MSSF_SF_MODE);

	//chip disable
	mssf_chip_enable_switch(MSSF_CHIP_DISABLE);

	//set related reg
	mssf_extra_en_switch(MSSF_DISABLE);
	mssf_ecc_en_switch(MSSF_DISABLE);
	mssf_set_msspeed(0x1);

	status = read_sr(flash);
	if(status!=0x0) {
		do {
			if((flash->sf_inst.typenum == SF_SST_AAI) ||
				(flash->sf_inst.typenum == SF_SST_AAW) ||
				(flash->sf_inst.typenum == SF_SST_PP))
			{
				//WRSR
				if(sf_wrsr_sst (flash))
					return 1;
			}
			else
			{
				if(write_enable(flash))
					return 1;
				if(wait_till_write_enable_ready(flash))
					return 1;
				//WRSR
				if(write_status_reg(flash, 0x0))
					return 1;
			}
			count++;

			if (count > 10000)
			{
				printk("can't not write sf register,may be no insert,so sf driver load fail\n");
				return 1;
			}

			//RDSR
			status = read_sr(flash);
		} while(status != 0x0);
	}
	DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 0);
	return 0;
}

/****************************************************************************/
/*
 * MTD implementation
 */
 
/*
 * Erase an address range on the flash chip.  The address range may extend
 * one or more erase sectors.  Return an error is there is a problem erasing.
 */
//using block erase 
static int snx_sf_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	struct mssf *flash = mtd_to_mssf(mtd);
	unsigned int addr, len;
	unsigned int ret = 0;

	struct flash_info *flash_info = NULL;

	flash_info = &snx_sf_data[0];

#if 1
	DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	// sanity checks 
	if(instr->addr + instr->len > (uint32_t)(flash->mtd.size)){
		printk("erase: sanity check error\n");
		return -EINVAL;
	}
	if(((uint32_t)(instr->addr) %  mtd->erasesize) != 0
			|| ((uint32_t)(instr->len) %  mtd->erasesize) != 0) {
		printk("erase: sanity check error\n");
		return -EINVAL;
	}
#if SUPPORT_SF_PROTECT
	sf_modifysw_to_clean (flash);
#endif
	addr = instr->addr;
	len = instr->len;

	down(&flash->lock);

 // 	printk("sc=%x,bk=%x,addr=%x,len=%x\n",flash_info->sector_size,flash_info->block_size,addr,len);

#if 0
	while (len) {
		// fail in jffs2 4k erase
		// if(flash->sf_inst.typenum == SF_EN)
		// 	ret = erase_sector(flash, addr);
		// else
		// 	ret = erase_block(flash, addr); <<- always do  block
		
		ret = erase_sector(flash, addr);	

		if(ret) {
			instr->state = MTD_ERASE_FAILED;
			up(&flash->lock);
#if SUPPORT_SF_PROTECT
  sf_modifysw_to_protect (flash);
#endif      
			return -EIO;
		}

		addr += mtd->erasesize;
		len -= mtd->erasesize;
	}
#else
	while (len) {
		if(flash->sf_inst.typenum == SF_SPANSION) {
			if (addr < 0x10000) { //4K
				ret = erase_sector(flash, addr);
				addr += flash_info->sector_size;
				len -= flash_info->sector_size;
			} else if ((len >= flash_info->block_size) && ((addr % flash_info->block_size) == 0)) { //64K
				ret = erase_block(flash, addr);
				addr += flash_info->block_size;
				len -= flash_info->block_size;
			} else
				ret = -1;

			if(ret) {
				instr->state = MTD_ERASE_FAILED;
				up(&flash->lock);
				#if SUPPORT_SF_PROTECT
  					sf_modifysw_to_protect (flash);
				#endif
				return -EIO;
			}
		} else {
			if ((len >= flash_info->block_size) && ((addr % flash_info->block_size) == 0)) {
				ret = erase_block(flash, addr);
				addr += flash_info->block_size;
				len -= flash_info->block_size;
			} else {
				ret = erase_sector(flash, addr);
				addr += flash_info->sector_size;
				len -= flash_info->sector_size;
			}

			if(ret) {
				instr->state = MTD_ERASE_FAILED;
				up(&flash->lock);
				#if SUPPORT_SF_PROTECT
  					sf_modifysw_to_protect (flash);
				#endif
				return -EIO;
			}
		}
	}
#endif

  	up(&flash->lock);

#if SUPPORT_SF_PROTECT
	sf_modifysw_to_protect (flash);
#endif
	instr->state = MTD_ERASE_DONE;
	mtd_erase_callback(instr);
	DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 0);
#endif
	return 0;
}

/*
 * Read an address range from the flash chip.  The address range
 * may be any size provided it is within the physical boundaries.
 */
static int snx_sf_cpu_readdata(struct mtd_info *mtd, loff_t src, size_t page_size, u_char *buf)
{
	struct mssf *flash = mtd_to_mssf(mtd);
	unsigned int i;
	DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	if (src >= 0x1000000)
		write_extend_addr_register(flash, 1);		
	// Wait until finished previous write command.
	if(wait_till_ready(flash)) {
		return 1;
	}

	// Set up the opcode in the write buffer. 
	flash->command[0] = flash->sf_inst.READSF;
	flash->command[1] =  ((unsigned int)src & 0xff0000) >> 16;
	flash->command[2] =  ((unsigned int)src & 0xff00) >> 8;
	flash->command[3] =  ((unsigned int)src & 0xff);

	//chip enable
	mssf_chip_enable_switch(MSSF_CHIP_ENABLE);

	//write instruction
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data(flash->command[0]);
	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;

	//write addr
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data(flash->command[1]);
	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;
        	
	//write addr
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data(flash->command[2]);
	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;

	//write addr
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data(flash->command[3]);
	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;

	//read data__cpu
	for(i = 0; i < page_size; i++)
	{
		mssf_msreg_rw_switch(MSSF_READ_MODE);
		mssf_write_data(0x0);
		if(wait_till_msrdy_ready())
			goto fail_msrdy_error;
		*(buf+i) = mssf_read_data();
	}

	//chip disable
	mssf_chip_enable_switch(MSSF_CHIP_DISABLE);
	if (src >= 0x1000000)
		write_extend_addr_register(flash, 0);		

	DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 0);
	return 0;

fail_msrdy_error :
	printk(" snx_sf_cpu_readdata : fail_msrdy_error\n");
        mssf_chip_enable_switch(MSSF_CHIP_DISABLE);
        DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 1);
        return 1;
}


//#ifdef CONFIG_SYSTEM_PLATFORM_ST58660FPGA
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)

static int snx_sf_dma_readdata(struct mtd_info *mtd, loff_t src, size_t page_size, u_char *buf, dma_addr_t addr_dma, char *addr)
{
	struct mssf *flash = mtd_to_mssf(mtd);
	unsigned int dma_size;

	DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	if(page_size % FLASH_PAGESIZE != 0) {
		printk("serial flash dma read dma size error\n");
		return 1;
	}

	// Wait until finished previous write command.
	if(wait_till_ready(flash)) {
		return 1;
	}

	mssf_msdma_rw_switch(MSSF_READ_MODE);
	mssf_set_dmaaddr((unsigned int)(addr_dma)); 	//ddr address

	dma_size = page_size - 1;
	mssf_set_dmasize(dma_size);

	mssf_dmablock(0);
	sfSetMdmaStartAddr(src);

#ifdef CONFIG_FASRDTRD_AVAILABLE
	sfSetDummyEN(1);
	sfDoubleRateSwitch(1);
#endif

#ifdef CONFIG_4DTRD_AVAILABLE
	setqebit (flash, 0);
	setqebit (flash, 1);

	sfSetDummyEN(1);

	sfDoubleRateSwitch(1);
	sf4BitsSwitch(1);
#endif

#ifdef CONFIG_QPI_AVAILABLE
	// sf4BitsSwitch(0);
	// sfQPIEnable();

	// sfSetQPIMode(1);
	// sf4BitsSwitch(1);

	sfSetDummyEN(1);	//FAST READ
#endif

	//write data
	//MS_DMA_EN = 1 dma enable
	mssf_msmdma_en_switch(MSSF_ENABLE);
	mssf_msdma_en_switch(MSSF_ENABLE);


	if(wait_till_msrdy_ready())
		goto fail_msdma_error;

	if(wait_till_msdma_ok())
		goto fail_msdma_error;

	//MS_DMA_EN = 0 dma disable
	mssf_msdma_en_switch(MSSF_DISABLE);
	mssf_msmdma_en_switch(MSSF_DISABLE);

#ifdef CONFIG_FASRDTRD_AVAILABLE
	sfSetDummyEN(0);
	sfDoubleRateSwitch(0);
#endif

#ifdef CONFIG_4DTRD_AVAILABLE
	sfSetDummyEN(0);

	sfDoubleRateSwitch(0);
	sf4BitsSwitch(0);
	setqebit (flash, 0);
#endif

#ifdef CONFIG_QPI_AVAILABLE
	// sfQPIDisable();

	// sfSetQPIMode(0);
	// sf4BitsSwitch(0);

	sfSetDummyEN(0);	//FAST READ
#endif

	memcpy(buf, addr, page_size);

	DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 0);
	return 0;

fail_msdma_error:
	printk(" snx_sf_dma_readdata : fail_msdma_error\n");
	mssf_msdma_en_switch(MSSF_DISABLE);
	mssf_msmdma_en_switch(MSSF_DISABLE);
	DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 1);
	return 1;
}

#else

static int snx_sf_dma_readdata(struct mtd_info *mtd, loff_t src, size_t page_size, u_char *buf, dma_addr_t addr_dma, char *addr)
{
	struct mssf *flash = mtd_to_mssf(mtd);
	unsigned int dma_size;

	DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	if(page_size % FLASH_PAGESIZE != 0) {
		printk("serial flash dma read dma size error\n");
		return 1;
	}
	
	if (src >= 0x1000000)
		write_extend_addr_register(flash, 1);		

	dma_size = page_size - 1;
	mssf_set_dmasize(dma_size);
	mssf_msdma_rw_switch(MSSF_READ_MODE);
	mssf_set_dmaaddr((unsigned int)(addr_dma));

	// Wait until finished previous write command.
	if(wait_till_ready(flash)) {
		return 1;
	}

	// Set up the opcode in the write buffer.
	flash->command[0] = flash->sf_inst.READSF;
	flash->command[1] =  ((unsigned int)src & 0xff0000) >> 16;
	flash->command[2] =  ((unsigned int)src & 0xff00) >> 8;
	flash->command[3] = ((unsigned int)src & 0xff);

	//chip enable
	mssf_chip_enable_switch(MSSF_CHIP_ENABLE);

	//write instruction
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data(flash->command[0]);
	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;

	//write addr
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data(flash->command[1]);
	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;

	//write addr
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data(flash->command[2]);
	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;

	//write addr
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data(flash->command[3]);
	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;

	//Read data
	//MS_DMA_EN = 1 dma enable      
	mssf_msdma_en_switch(MSSF_ENABLE);

	if(wait_till_msrdy_ready()) 
		goto fail_msdma_error; 

	//MS_DMA_EN = 0 dma disable
	mssf_msdma_en_switch(MSSF_DISABLE);

	//chip disable
	mssf_chip_enable_switch(MSSF_CHIP_DISABLE);
	if (src >= 0x1000000)
		write_extend_addr_register(flash, 0);		

	memcpy(buf, addr, page_size);

	DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 0);
	return 0;

fail_msdma_error:
	printk(" snx_sf_dma_readdata : fail_msdma_error\n");
	mssf_msdma_en_switch(MSSF_DISABLE);
fail_msrdy_error :
	printk(" snx_sf_dma_readdata : fail_msrdy_error\n");
	mssf_chip_enable_switch(MSSF_CHIP_DISABLE);
	DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 1);
        return 1;
}
#endif


/*  alek modify */
dma_addr_t addr_dma_sf;
char *_buf_addr = NULL;
size_t dma_alloc_size = FLASH_PAGESIZE;
int dma_flag = 0;

static int snx_sf_read_dma(struct mtd_info *mtd, loff_t from, size_t len,
	size_t *retlen, u_char *buf)
{

	struct mssf *flash = mtd_to_mssf(mtd);
//	unsigned int ret=0;
	unsigned int page_size;
//	dma_addr_t addr_dma;
//	char *addr = NULL;
//	size_t dma_size = FLASH_PAGESIZE;
//	int dma_flag = 0;
	DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);

	// sanity checks
	if(!len) {
		printk("dma read: len=0\n");
		return(0);
	}

	if(from + len > (uint32_t)(flash->mtd.size)) {
		printk("dma read: sanity check error\n");
		return -EINVAL;
	}

	if(retlen) {
		*retlen = 0;
	}

	down(&flash->lock);  

#if 0
	if(len>FLASH_PAGESIZE)
	{
		dma_flag = 1;
		addr = dma_alloc_coherent(NULL, dma_size, &addr_dma_sf, GFP_KERNEL | GFP_DMA);
		if(addr == NULL) {
			ret = -ENOMEM;
			goto err;
		}
		memset(addr, 0, dma_size);	
	}
#endif  

	while(len)
	{
		if(len<FLASH_PAGESIZE) {
			page_size = len;
			//printk("cpu_read ======> from = 0x%x, buf=0x%x, page_size=%x\n", (unsigned int)from, (unsigned int)buf, (unsigned int)page_size);
			if(snx_sf_cpu_readdata(mtd, from, page_size, buf))
				goto readerr;
		}else {
			page_size = FLASH_PAGESIZE;
			//printk("dma_read ======> from = 0x%x, buf=0x%x, page_size=%x\n", (unsigned int)from, (unsigned int)buf, (unsigned int)page_size);
			
			if(snx_sf_dma_readdata(mtd, from, page_size, buf, addr_dma_sf, _buf_addr))	
				goto readerr;
		}
		from += page_size;
		buf += page_size;
		len -=page_size;
		*retlen += page_size;
	}

  //start_measure_time ();
#if 0  
	if(dma_flag == 1) {
		dma_free_coherent(NULL, dma_size,  addr, addr_dma);
		dma_flag=0;
	}
#endif

	up(&flash->lock);
	DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 0);

	return 0;
//err:
	printk(" snx_sf_read_dma : dma_alloc_coherent err\n");
//	dma_free_coherent(NULL, dma_size,  addr, addr_dma);
readerr:
	printk(" snx_sf_read_dma : readerr\n");

	up(&flash->lock);
	DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 1);
	return 1;
}

static int snx_sf_cpu_AAIW_program(struct mtd_info *mtd, loff_t src, size_t page_size, const u_char *buf)
{
	struct mssf *flash = mtd_to_mssf(mtd);
	unsigned int i = 0, status;

  	DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);

	//write enable
	if(wait_till_ready(flash)) {
		return 1;
	}
	if (src >= 0x1000000)
		write_extend_addr_register(flash, 1);		

	if(wait_till_ready(flash)) {
		return 1;
	}
	if(write_enable(flash))
		return 1;
	if(wait_till_write_enable_ready(flash))
		return 1;

	// Set up the opcode in the write buffer.
	if(flash->sf_inst.typenum == SF_SST_AAI)
		flash->command[0] = flash->sf_inst.AAI;	//SST_AAI
	else if(flash->sf_inst.typenum == SF_SST_AAW)
		flash->command[0] = flash->sf_inst.AAW;	//SST_AAW
	//flash->command[0] = flash->sf_inst.PP;
	flash->command[1] =  ((unsigned int)src & 0xff0000) >> 16;
	flash->command[2] =  ((unsigned int)src & 0xff00) >> 8;
	flash->command[3] =  ((unsigned int)src & 0xff);

	//chip enable
	mssf_chip_enable_switch(MSSF_CHIP_ENABLE);

	//write instruction
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data(flash->command[0]);
	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;

	//write addr_1
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data(flash->command[1]);
	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;
		
	//write addr_2
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data(flash->command[2]);
	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;

	//write addr_3
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data(flash->command[3]);
	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;

	mssf_write_data(*(buf+i));
	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;

	mssf_chip_enable_switch(MSSF_CHIP_DISABLE);
	//RDSR__check WIP pin
	do{
		status = read_sr(flash);
		//printf("status = 0x%x\n", status);
	}while ((status & 0x1) == 0x1);

	//write data
	for(i = i + 1; i < page_size; i++)
	{
		mssf_chip_enable_switch(MSSF_CHIP_ENABLE);

		mssf_msreg_rw_switch(MSSF_WRITE_MODE);
		mssf_write_data(flash->command[0]);
		if(wait_till_msrdy_ready())
			goto fail_msrdy_error;

		mssf_write_data(*(buf+i));

		if(wait_till_msrdy_ready())
			goto fail_msrdy_error;

		mssf_chip_enable_switch(MSSF_CHIP_DISABLE);
		//RDSR__check WIP pin
		do{
			status = read_sr(flash);
			//printf("status = 0x%x\n", status);
		}while ((status & 0x1) == 0x1);
        }

	mssf_chip_enable_switch(MSSF_CHIP_ENABLE);
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data (flash->sf_inst.WRDI);

	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;

	//chip disable
	mssf_chip_enable_switch(MSSF_CHIP_DISABLE);
	if (src >= 0x1000000) {
		if(wait_till_ready(flash)) {
			return 1;
		}
		write_extend_addr_register(flash, 0);		
	}

	DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 0);
	return 0;

fail_msrdy_error :
	printk(" snx_sf_cpu_program : fail_msrdy_error\n");
        mssf_chip_enable_switch(MSSF_CHIP_DISABLE);
        DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 1);
        return 1;
}

/*
 * Write an address range to the flash chip.  Data must be written in
 * FLASH_PAGESIZE chunks.  The address range may be any size provided
 * it is within the physical boundaries.
 */
static int snx_sf_cpu_program(struct mtd_info *mtd, loff_t src, size_t page_size, const u_char *buf)
{
	struct mssf *flash = mtd_to_mssf(mtd);
	unsigned int i;

	DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	if(wait_till_ready(flash)) {
		return 1;
	}

	if (src >= 0x1000000)
		write_extend_addr_register(flash, 1);		

	if(wait_till_ready(flash)) {
		return 1;
	}
	//write enable
	if(write_enable(flash))
		return 1;
	if(wait_till_write_enable_ready(flash))
		return 1;

	// Set up the opcode in the write buffer.
	flash->command[0] = flash->sf_inst.PP;
	flash->command[1] =  ((unsigned int)src & 0xff0000) >> 16;
	flash->command[2] =  ((unsigned int)src & 0xff00) >> 8;
	flash->command[3] =  ((unsigned int)src & 0xff);

	//chip enable
	mssf_chip_enable_switch(MSSF_CHIP_ENABLE);

	//write instruction
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data(flash->command[0]);
	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;

	//write addr_1
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data(flash->command[1]);
	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;
		
	//write addr_2
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data(flash->command[2]);
	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;

	//write addr_3
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data(flash->command[3]);
	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;

	//write data
	for(i=0; i<page_size; i++)
	{
		mssf_write_data(*(buf+i));
		if(wait_till_msrdy_ready())
			goto fail_msrdy_error;
	}

	//chip disable
	mssf_chip_enable_switch(MSSF_CHIP_DISABLE);
	if (src >= 0x1000000) {
		if(wait_till_ready(flash)) {
			return 1;
		}
		write_extend_addr_register(flash, 0);		
	}

	DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 0);
	return 0;

fail_msrdy_error :
	printk(" snx_sf_cpu_program : fail_msrdy_error\n");
        mssf_chip_enable_switch(MSSF_CHIP_DISABLE);
        DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 1);
        return 1;
}

//#ifdef CONFIG_SYSTEM_PLATFORM_ST58660FPGA
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)

static int snx_sf_dma_program(struct mtd_info *mtd, loff_t src, size_t page_size, const u_char *buf, dma_addr_t addr_dma, char *addr)
{
	struct mssf *flash = mtd_to_mssf(mtd);
	unsigned int dma_size;
	unsigned int sudma;

	DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);

	if(page_size % FLASH_PAGESIZE != 0) {
		printk("serial flash dma program dma size error\n");
		return 1;
	}

	memcpy(addr, buf, page_size);

	// Wait until finished previous write command.
	if(wait_till_ready(flash)) {
		return 1;
	}

	mssf_msdma_rw_switch(MSSF_WRITE_MODE);
	mssf_set_dmaaddr((unsigned int)(addr_dma));		//ddr address

	dma_size = page_size - 1;
	mssf_set_dmasize(dma_size);  
	mssf_dmablock(0);

	sfSetMdmaStartAddr(src);


	//write data
	//MS_DMA_EN = 1 dma enable
	mssf_msmdma_en_switch(MSSF_ENABLE);
	mssf_msdma_en_switch(MSSF_ENABLE);


	if(wait_till_msrdy_ready())
		goto fail_msdma_error;


	if(wait_till_msdma_ok())
		goto fail_msdma_error;


//	sudma = mssf_read_sudmablock();
//	printk("==>%x,%x,%x <<== %x,%x,%x,%x\n",dma_size,src,sudma,flash->sf_inst.PP,flash->sf_inst.READSF,flash->sf_inst.WREN,flash->sf_inst.RDSR);


	//MS_DMA_EN = 0 dma disable
	mssf_msdma_en_switch(MSSF_DISABLE);
	mssf_msmdma_en_switch(MSSF_DISABLE);



		//chip disable
//	mssf_chip_enable_switch(MSSF_CHIP_DISABLE);

	DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 0);
	return 0;

fail_msdma_error:
	printk("snx_sf_dma_program : fail_msdma_error\n");
	mssf_msdma_en_switch(MSSF_DISABLE);
	mssf_msmdma_en_switch(MSSF_DISABLE);
	DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 1);
	return 1;
}
#else
static int snx_sf_dma_program(struct mtd_info *mtd, loff_t src, size_t page_size, const u_char *buf, dma_addr_t addr_dma, char *addr)
{
	struct mssf *flash = mtd_to_mssf(mtd);
	unsigned int dma_size;

	DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);

	if(page_size % FLASH_PAGESIZE != 0) {
		printk("serial flash dma program dma size error\n");
		return 1;
	}
	
	memcpy(addr, buf, page_size);

	dma_size = page_size - 1;
	mssf_set_dmasize(dma_size);
        mssf_msdma_rw_switch(MSSF_WRITE_MODE);
	mssf_set_dmaaddr((unsigned int)(addr_dma));

	// Wait until finished previous write command.
	if(wait_till_ready(flash)) {
		return 1;
	}

	if (src >= 0x1000000)
		write_extend_addr_register(flash, 1);		

	if(wait_till_ready(flash)) {
		return 1;
	}
	//write enable
	if(write_enable(flash))
		return 1;

	if(wait_till_write_enable_ready(flash))
		return 1;

	// Set up the opcode in the write buffer.
	//flash->command[0] = OPCODE_PP; //MXIC:PP, SST:AAI or AAW
	if(flash->sf_inst.typenum == SF_SST_AAI)
		flash->command[0] = flash->sf_inst.AAI;	//SST_AAI
	else if(flash->sf_inst.typenum == SF_SST_AAW)
		flash->command[0] = flash->sf_inst.AAW;	//SST_AAW
	else
		flash->command[0] = flash->sf_inst.PP;
	flash->command[1] =  ((unsigned int)src & 0xff0000) >> 16;
	flash->command[2] =  ((unsigned int)src & 0xff00) >> 8;
	flash->command[3] =  ((unsigned int)src & 0xff);

	//chip enable
	mssf_chip_enable_switch(MSSF_CHIP_ENABLE);

	//write instruction
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data(flash->command[0]);
	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;

	//write addr_1
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data(flash->command[1]);
	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;
		
	//write addr_2
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data(flash->command[2]);
	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;

	//write addr_3
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data(flash->command[3]);
	if(wait_till_msrdy_ready())
		goto fail_msrdy_error;

	//MXIC : noramal, SST: AAI or AAW && need to set WiteCmd and TimeCount
	//mssf_set_wmode(0x0);//0x0: normal mode
	if(flash->sf_inst.typenum == SF_SST_AAI)
	{
		mssf_set_wmode (SF_AAI);
		mssf_set_wcmd (flash->sf_inst.AAI);
		mssf_set_timecount (flash->sf_inst.TBP);
	}
	else if(flash->sf_inst.typenum == SF_SST_AAW)
	{
		mssf_set_wmode (SF_AAW);
		mssf_set_wcmd (flash->sf_inst.AAW);
		mssf_set_timecount (flash->sf_inst.TBP);
	}
	else
		mssf_set_wmode (SF_NORMAL);

	mssf_clear_mserr_flag(1);
	mssf_clear_msrdy_flag(1);
	//mssf_set_timecount(0xfffffff);

	//write data
	//MS_DMA_EN = 1 dma enable
	mssf_msdma_en_switch(MSSF_ENABLE);

	if(wait_till_msrdy_ready())
		goto fail_msdma_error;

	//MS_DMA_EN = 0 dma disable
	mssf_msdma_en_switch(MSSF_DISABLE);

	if((flash->sf_inst.typenum == SF_SST_AAI) || (flash->sf_inst.typenum == SF_SST_AAW))
	{
		//WRDI
		//Set serial flash CE = 0
		//sfChipEnable(sfMode);
		//write instruction
		mssf_msreg_rw_switch(MSSF_WRITE_MODE);
		mssf_write_data (flash->sf_inst.WRDI);
		if(wait_till_msrdy_ready())
			goto fail_msdma_error;
	}

	//chip disable
	mssf_chip_enable_switch(MSSF_CHIP_DISABLE);

	if (src >= 0x1000000) {
		if(wait_till_ready(flash)) {
			return 1;
		}
		write_extend_addr_register(flash, 0);		
	}
	DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 0);
	return 0;

fail_msdma_error:
        printk("snx_sf_dma_program : fail_msdma_error\n");
        mssf_msdma_en_switch(MSSF_DISABLE);
fail_msrdy_error:
	printk("snx_sf_dma_program : fail_msrdy_error\n");
        mssf_chip_enable_switch(MSSF_CHIP_DISABLE);
        DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 1);
        return 1;
}
#endif

#if 0

static int snx_sf_write_dma(struct mtd_info *mtd, loff_t to, size_t len,
	size_t *retlen, const u_char *buf)
{
	struct mssf *flash = mtd_to_mssf(mtd);
	unsigned int i;//ret=0;
	unsigned int page_offset, page_size;

	unsigned int trsblksize;
	unsigned int lastsize;
	unsigned int trstimes;


#if SUPPORT_SF_PROTECT
	sf_modifysw_to_clean (flash);
#endif
	int (*cpu_program)(struct mtd_info *mtd, loff_t src, size_t page_size, const u_char *buf);

	DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	if((flash->sf_inst.typenum == SF_SST_AAI) || (flash->sf_inst.typenum == SF_SST_AAW))
		cpu_program = snx_sf_cpu_AAIW_program;
	else
		cpu_program = snx_sf_cpu_program;

	if(retlen) {
#if SUPPORT_SF_PROTECT
		sf_modifysw_to_protect (flash);
#endif  
		*retlen = 0;
	}

	// sanity checks 
	if(!len) {
#if SUPPORT_SF_PROTECT
		sf_modifysw_to_protect (flash);
#endif  
		printk("dma write: len=0\n");
		return(0);
	}

	if(to + len > (uint32_t)(flash->mtd.size)) {
		printk("dma write: sanity check error\n");
		#if SUPPORT_SF_PROTECT
			sf_modifysw_to_protect (flash);
		#endif
		return -EINVAL;
	}

	down(&flash->lock);

////////////////////////////////MDMA/////////////////////////////////////////////////////
	for (i=0; i<25; i++) {
		trsblksize = 1<<i;
		//max 16mb : 0b1,0000,0000,0000,0000,0000,0000	
		if (trsblksize > len) {
			trsblksize = (1<<(i-1));
			break;
		}
	}

	trstimes = len / trsblksize;
	lastsize = len % trsblksize;


	// Wait until finished previous write command.
	if(wait_till_ready(flash)) {
		return 1;
	}

	printk("buf=%x,trsblksize=%x,trstimes=%x,to=%x,len=%x\n",buf,trsblksize,trstimes,to,len);


	mssf_msdma_rw_switch(MSSF_WRITE_MODE);
	mssf_set_dmaaddr((unsigned int)(buf));		//ddr address

	mssf_set_dmasize(trsblksize-1);  
	mssf_dmablock(trstimes);

	sfSetMdmaStartAddr(to);


	//write data
	//MS_DMA_EN = 1 dma enable
	mssf_msmdma_en_switch(MSSF_ENABLE);
	mssf_msdma_en_switch(MSSF_ENABLE);


	if(wait_till_msrdy_ready())
		goto writeerr;


	if(wait_till_msdma_ok())
		goto writeerr;

	//MS_DMA_EN = 0 dma disable
	mssf_msdma_en_switch(MSSF_DISABLE);
	mssf_msmdma_en_switch(MSSF_DISABLE);

	DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 0);

////////////////////////////////CPU////////////////////////////////////////
	if((*cpu_program)(mtd, (to + trstimes*trsblksize), lastsize, (buf + trstimes*trsblksize)))
		goto writeerr;

/////////////////////////////////END/////////////////////////////////////////

	up(&flash->lock);

#if SUPPORT_SF_PROTECT
	sf_modifysw_to_protect (flash);
#endif
	DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 0);
	return 0;

//err:
	printk("snx_sf_write_dma : dma_alloc_coherent err\n");
//	dma_free_coherent(NULL, dma_size,  addr, addr_dma);
writeerr:
	printk("snx_sf_dma_program : fail_msdma_error\n");
	mssf_msdma_en_switch(MSSF_DISABLE);
	mssf_msmdma_en_switch(MSSF_DISABLE);
	DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 1);

	printk("------------>snx_sf_write_dma------------>writeerr\n");
	up(&flash->lock);
#if SUPPORT_SF_PROTECT
	sf_modifysw_to_protect (flash);
#endif
	DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 1);
	return 1;
}


#else

static int snx_sf_write_dma(struct mtd_info *mtd, loff_t to, size_t len,
	size_t *retlen, const u_char *buf)
{
	struct mssf *flash = mtd_to_mssf(mtd);
	unsigned int i;//ret=0;
	unsigned int page_offset, page_size;
//	dma_addr_t addr_dma;
//	char *addr = NULL;
//	size_t dma_size= FLASH_PAGESIZE;
//	int dma_flag = 0;
#if SUPPORT_SF_PROTECT
	sf_modifysw_to_clean (flash);
#endif
	int (*cpu_program)(struct mtd_info *mtd, loff_t src, size_t page_size, const u_char *buf);

	DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	if((flash->sf_inst.typenum == SF_SST_AAI) || (flash->sf_inst.typenum == SF_SST_AAW))
		cpu_program = snx_sf_cpu_AAIW_program;
	else
		cpu_program = snx_sf_cpu_program;

	if(retlen) {
#if SUPPORT_SF_PROTECT
		sf_modifysw_to_protect (flash);
#endif  
		*retlen = 0;
	}

	// sanity checks 
	if(!len) {
#if SUPPORT_SF_PROTECT
		sf_modifysw_to_protect (flash);
#endif  
		printk("dma write: len=0\n");
		return(0);
	}

	if(to + len > (uint32_t)(flash->mtd.size)) {
		printk("dma write: sanity check error\n");
#if SUPPORT_SF_PROTECT
		sf_modifysw_to_protect (flash);
#endif
		return -EINVAL;
	}

	down(&flash->lock);
#if 0
	if(len > FLASH_PAGESIZE)
	{
		dma_flag=1;
		addr = dma_alloc_coherent(NULL, dma_size, &addr_dma, GFP_KERNEL | GFP_DMA);
		if(addr == NULL) {
			ret = -ENOMEM;
			goto err;
		}
		memset(addr, 0, dma_size);
	}
#endif

	// what page do we start with?
	page_offset = to % FLASH_PAGESIZE;
	//if(page_offset + len <= FLASH_PAGESIZE) {
	if(page_offset + len < FLASH_PAGESIZE)
	{
		//<= one page size
		//printk("cpu__one_page ======> to = 0x%x, buf=0x%x, len=%x\n", (unsigned int)to, (unsigned int)buf, (unsigned int)len);
		if((*cpu_program)(mtd, to, len, buf))
			goto writeerr;
		*retlen = len;
	}
	else 
	{
		//> one page size
		// the size of data remaining on the first page 
		page_size = FLASH_PAGESIZE - page_offset;
		
		//printk("%x,%x\n",FLASH_PAGESIZE,page_offset);
		//printk("cpu__remin first page ======> to = 0x%x, buf=0x%x, len=%x\n", (unsigned int)to, (unsigned int)buf, (unsigned int)page_size);
		

		if((*cpu_program)(mtd, to, page_size, buf))
			goto writeerr;
		*retlen = page_size;
		
		// write everything in PAGESIZE chunks 
		for (i = page_size; i < len; i += page_size) 
		{
			page_size = len - i;
			if(page_size > FLASH_PAGESIZE) {
				//using dma
				page_size = FLASH_PAGESIZE;
			//	printk("dma write======> to+i = 0x%x, buf+i=0x%x, len=%x\n", ((unsigned int)to+i), ((unsigned int)buf+i), (unsigned int)page_size);
				if(snx_sf_dma_program(mtd, (to+i), page_size, (buf+i), addr_dma_sf, _buf_addr))
					goto writeerr;
			}
			else {
				//using cpu		
			//	printk("cpu write======> to+i = 0x%x, buf+i=0x%x, len=%x\n", ((unsigned int)to+i), ((unsigned int)buf+i), (unsigned int)page_size);
				if((*cpu_program)(mtd, (to+i), page_size, (buf+i)))
					goto writeerr;
			}
			if(retlen)
				*retlen += page_size;
		}
	}

#if 0
	if(dma_flag == 1)
	{
		dma_free_coherent(NULL, dma_size,  addr, addr_dma);
		dma_flag=0;
	}
#endif

	up(&flash->lock);

#if SUPPORT_SF_PROTECT
	sf_modifysw_to_protect (flash);
#endif
	DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 0);
	return 0;

//err:
	printk("snx_sf_write_dma : dma_alloc_coherent err\n");
//	dma_free_coherent(NULL, dma_size,  addr, addr_dma);
writeerr:
	printk("------------>snx_sf_write_dma------------>writeerr\n");
	up(&flash->lock);
#if SUPPORT_SF_PROTECT
	sf_modifysw_to_protect (flash);
#endif
	DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 1);
	return 1;
}

#endif

int get_id_index (struct sf_id *id_arry, int arry_count, struct sf_id check_id)
{
	int index=0;
	
	for (index = 0; index < arry_count; index++) {
		if(id_arry[index].id1 != check_id.id1)
			continue;
		if(id_arry[index].id2 != check_id.id2)
			continue;
		if(id_arry[index].id3 != check_id.id3)
			continue;
		break;
	}
	
	return index;
}

int sf_checkid (struct sf_id id, int typenum)
{
	int cnt,index;
	DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);

	switch (typenum) {
		case SF_MXIC_AMIC:
			cnt = sizeof(id_mixc_arry) / sizeof(struct sf_id);
			
			index = get_id_index (id_mixc_arry, cnt, id);
			
    		if ((index == MX25L12835E)||(index == MX25L12835F)) {
    			snx_sf_data->block_size = 32 * 1024; //32k
				snx_sf_data->sector_size = 4 * 1024; //4k
				snx_sf_data->n_blocks = 512;//16M = 256*64k
				return 1;
    		}
			else if(index == MX25L6406E) {
				snx_sf_data->block_size = 32 * 1024; //32k
				snx_sf_data->sector_size = 4 * 1024; //4k
				snx_sf_data->n_blocks = 256;//8M = 128*64k
				return 1;
			}
    // 		else if (index == MX25L1605D) {
    // 			snx_sf_data->block_size = 256 * 16 * 16; //64k
				// snx_sf_data->sector_size = 256 * 16; //4k
				// snx_sf_data->n_blocks = 32; //2M = 32 * 64k
				// return 1;
    // 		}

			break;
		case SF_SPANSION:
			cnt = sizeof(id_spanson_arry) / sizeof(struct sf_id);
			
			index = get_id_index (id_spanson_arry, cnt, id);
			
			if (index == S25FL127S) {
				snx_sf_data->block_size = 64 * 1024; //64k
				snx_sf_data->sector_size = 4 * 1024; //4k
				snx_sf_data->n_blocks = 256; //16M = 256 * 64k
				return 1;
    			}
			break;
		case SF_GD:
			cnt = sizeof(id_gd_arry) / sizeof(struct sf_id);
			
			index = get_id_index (id_gd_arry, cnt, id);
			
			if (index == GD25Q128C) {
				snx_sf_data->block_size = 32 * 1024; //32k
				snx_sf_data->sector_size = 4 * 1024; //4k
				snx_sf_data->n_blocks = 512; //16M = 512 * 32k
				return 1;
    			}
    // 			else if (index == GD25Q16B) {
    // 			snx_sf_data->block_size = 256 * 16 * 16; //64k
				// snx_sf_data->sector_size = 256 * 16; //4k
				// snx_sf_data->n_blocks = 32; //2M = 32 * 64k
				// return 1;	
    // 			}
			break;
		case SF_EN:
			cnt = sizeof(id_en_arry) / sizeof(struct sf_id);
			
			index = get_id_index (id_en_arry, cnt, id);
			
			if (index == EN25P64_8M) {
				snx_sf_data->block_size = 256 * 256;
				snx_sf_data->sector_size = 256 * 256; //64k
				snx_sf_data->n_blocks = 128;//8M = 128*64k
				return 1;
    			}
			break;
		case SF_PMC:
			cnt = sizeof(id_pmc_arry) / sizeof(struct sf_id);
			
			index = get_id_index (id_pmc_arry, cnt, id);
			
			if (index == PMC_64K) {
				snx_sf_data->block_size = 256 * 16; //4k
				snx_sf_data->sector_size = 256 * 16 * 8; //32k
				snx_sf_data->n_blocks = 256 * 16 * 8 * 2;//64k
				return 1;
    			}
			break;
		case SF_WINBOND:
			cnt = sizeof(id_winbond_arry) / sizeof(struct sf_id);
		
			index =  get_id_index  (id_winbond_arry, cnt, id);

			if (index == W25Q128FVEF)  {
				snx_sf_data->block_size = 32 * 1024; //32k
				snx_sf_data->sector_size = 4 * 1024; //4k
				snx_sf_data->n_blocks = 1024; //32MB = 512 * 32k
				return 1;
			}
			break;
		case SF_SST_AAI:
		case SF_SST_AAW:
		case SF_SST_PP:
		default:
			break;
	}
	
	return 0;
}

//=======================================================================
//MXIC, GD , S25FL127S
static int sf_readidtest_type1 (struct mssf *flash)
{
	struct sf_id id = {0,0,0};
	DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);

	//serial flash initial
	if(sf_init(flash))
		return 0;

	//Set serial flash CE = 0
	mssf_chip_enable_switch(MSSF_CHIP_ENABLE);//MSSF_WRITE_MODE

	//write command to read sf reg status
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);

	//printk("flash->sf_inst.RDID %x\n",flash->sf_inst.RDID);
	mssf_write_data (flash->sf_inst.RDID);
	if(wait_till_msrdy_ready()) {
		DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 0);
		return 0;
	}

	//read Manufacturer ID
	mssf_msreg_rw_switch(MSSF_READ_MODE);
	mssf_write_data (0x0); 
	if(wait_till_msrdy_ready()) {
		DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 0);
		return 0;
	}
	id.id1 = mssf_read_data ();
	//printk ("ID1 = %x\n",id.id1);

	//read Memory Type ID
	mssf_msreg_rw_switch(MSSF_READ_MODE);
	mssf_write_data (0x0);
	if(wait_till_msrdy_ready()) {
		DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 0);
		return 0;
	}
	id.id2 = mssf_read_data ();
   	//printk ("ID2 = 0x%x\n", id.id2);

	//read device ID
	mssf_msreg_rw_switch(MSSF_READ_MODE);
	mssf_write_data (0x0);
	if(wait_till_msrdy_ready()) {
		DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 0);
		return 0;
	}
	id.id3 = mssf_read_data ();
	//printk("ID3 = 0x%x\n", id.id3);

	//Set serial flash CE = 1
	mssf_chip_enable_switch(MSSF_CHIP_DISABLE);

	return sf_checkid(id, flash->sf_inst.typenum);
}

//=======================================================================
//PMC
static int sf_readidtest_type2(struct mssf *flash)
{
	struct sf_id id = {1,1,1};
	DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);

	//serial flash initial
	if(sf_init(flash))
		return 0;

	//Set serial flash CE = 0
	mssf_chip_enable_switch(MSSF_CHIP_ENABLE);

	//write command to read sf reg status 
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data (flash->sf_inst.RDID); 
	//printk("the id cmd2 is 0x%x\n",flash->sf_inst.RDID);
	if(wait_till_msrdy_ready()) {
		return 0;
	}

	//dummy bytes__3 times
	mssf_write_data (0x0);
	if(wait_till_msrdy_ready()) {
		return 0;
	}
	mssf_write_data (0x0); 
	if(wait_till_msrdy_ready()) {
		return 0;
	}
	mssf_write_data (0x0); 
	if(wait_till_msrdy_ready()) {
		return 0;
	}

	//read ID1
	mssf_msreg_rw_switch(MSSF_READ_MODE);
	mssf_write_data (0x0); 
	if(wait_till_msrdy_ready()) {
		return 0;
	}
	id.id1= mssf_read_data ();
	//printk("ID1 = 0x%x\n", id.id1);

	//read ID2
	mssf_msreg_rw_switch(MSSF_READ_MODE);
	mssf_write_data (0x0);
	if(wait_till_msrdy_ready()) {
		return 0;
	}
	id.id2 = mssf_read_data ();
	//printk("ID2 = 0x%x\n", id.id2);

	//read ID3
	mssf_msreg_rw_switch(MSSF_READ_MODE);
	mssf_write_data (0x0);
	if(wait_till_msrdy_ready()) {
		return 0;
	}
	id.id3 = mssf_read_data ();
	//printk("ID3 = 0x%x\n", id.id3);

	//Set serial flash CE = 1
	mssf_chip_enable_switch(MSSF_CHIP_DISABLE);

	return sf_checkid(id, flash->sf_inst.typenum);
}

//=======================================================================
//SST
static int sf_readidtest_type3(struct mssf *flash)
{
	struct sf_id id = {1,1,1};
	DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);

	//serial flash initial
	if(sf_init(flash))
		return 0;

	//Set serial flash CE = 0
	 mssf_chip_enable_switch(MSSF_CHIP_ENABLE);

	//write command to read sf reg status 
	mssf_msreg_rw_switch(MSSF_WRITE_MODE);
	mssf_write_data(flash->sf_inst.RDID);
	//printk("the id cmd3 is 0x%x\n",flash->sf_inst.RDID);
	if(wait_till_msrdy_ready()) {
		return 0;
	}

	//write addr
	mssf_write_data(0x0);
	if(wait_till_msrdy_ready()) {
		return 0;
	}
	mssf_write_data(0x0);
	if(wait_till_msrdy_ready()) {
		return 0;
	}
	mssf_write_data(0x0);
	if(wait_till_msrdy_ready()) {
		return 0;
	}

	//read Manufacture ID
	mssf_msreg_rw_switch(MSSF_READ_MODE);
	mssf_write_data(0x0);
	if(wait_till_msrdy_ready()) {
		return 0;
	}
	id.id1= mssf_read_data();
	//printk("Manufacture ID = 0x%x\n", id.id1);

	//read Device ID
	mssf_msreg_rw_switch(MSSF_READ_MODE);
	mssf_write_data(0x0); 
	if(wait_till_msrdy_ready()) {
		return 0;
	}
	id.id2 = mssf_read_data();
	//printk("Device ID = 0x%x\n", id.id2);

	//Set serial flash CE = 1
	mssf_chip_enable_switch(MSSF_CHIP_DISABLE);
 
	return sf_checkid(id, flash->sf_inst.typenum);
}


static int snx_init_sf_type(struct mssf *flash)	 
{
	DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);

	sf_SetInstruction(SF_MXIC_AMIC, &flash->sf_inst);
	if(sf_readidtest_type1(flash))
		return 0;

	sf_SetInstruction(SF_EN, &flash->sf_inst);
	if(sf_readidtest_type1(flash))
		return 0;

	sf_SetInstruction(SF_GD, &flash->sf_inst);
	if(sf_readidtest_type1(flash))
		return 0;

	sf_SetInstruction(SF_PMC, &flash->sf_inst);
	if(sf_readidtest_type2(flash))
		return 0;

	sf_SetInstruction(SF_SST_PP, &flash->sf_inst);
	if(sf_readidtest_type3(flash))
		return 0;

	sf_SetInstruction(SF_SST_AAW, &flash->sf_inst);
	if(sf_readidtest_type3(flash))
		return 0;

	sf_SetInstruction(SF_SST_AAI, &flash->sf_inst);
	if(sf_readidtest_type3(flash))
		return 0;

	sf_SetInstruction(SF_SPANSION, &flash->sf_inst);
	if(sf_readidtest_type1(flash))
		return 0;
	sf_SetInstruction(SF_WINBOND, &flash->sf_inst);
	if(sf_readidtest_type1(flash))
		return 0;

	
	DBG_ME ("%s:%d: return value = %d\n",__func__, __LINE__, 1);  

	return 1;
}

static int __devinit snx_sf_probe(struct platform_device *pdev)
{
	int ret = -ENOENT;

	int err = 0;
#ifdef CONFIG_MTD_PARTITIONS
	struct physmap_flash_data *pdata = pdev->dev.platform_data;
#endif

 	DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);

	sf_flash = kzalloc(sizeof *sf_flash, GFP_KERNEL);

	if(!sf_flash) {
		return -ENOMEM;
	}

	//modify test
	if(snx_init_sf_type(sf_flash))
		goto out_free_info;

	//delezue add : fpga58660 feature
//#ifdef CONFIG_SYSTEM_PLATFORM_ST58660FPGA
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)

	new_feature_support(sf_flash);
#endif

	init_MUTEX(&sf_flash->lock);
	platform_set_drvdata(pdev, sf_flash);
#if 1
	dma_flag = 1;
	_buf_addr = dma_alloc_coherent(NULL, dma_alloc_size, &addr_dma_sf, GFP_KERNEL | GFP_DMA);
	if(_buf_addr == NULL) {
		return -ENOMEM;
		//ret = -ENOMEM;
		//goto err;
	}
	memset(_buf_addr, 0, dma_alloc_size);
#endif
	sf_flash->mtd.name = "serial flash";
	sf_flash->mtd.type = MTD_NORFLASH;
	sf_flash->mtd.writesize = 1;
	sf_flash->mtd.flags = MTD_CAP_NORFLASH;
	sf_flash->mtd.size = snx_sf_data->block_size * snx_sf_data->n_blocks;

//	if(sf_flash->sf_inst.typenum == SF_EN)
		sf_flash->mtd.erasesize = snx_sf_data->sector_size;
//	else
//		sf_flash->mtd.erasesize = snx_sf_data->block_size;

	sf_flash->mtd.erase = snx_sf_erase;
	sf_flash->mtd.read = snx_sf_read_dma;
	sf_flash->mtd.write = snx_sf_write_dma;

#ifdef CONFIG_MTD_PARTITIONS
	sf_flash->mtd.name = "snx-spi";
	err = parse_mtd_partitions(&sf_flash->mtd, part_probes, &sf_flash->parts, 0);
	if(err > 0) {
		add_mtd_partitions(&sf_flash->mtd, sf_flash->parts, err);
	}
	else if(pdata->parts) {
		add_mtd_partitions(&sf_flash->mtd, pdata->parts, pdata->nr_parts);
	}
	else {
		printk(KERN_ERR "SPI flash Create Partition Table failuer!");
		goto out_free_info;
	}
#else
	if(add_mtd_device(&sf_flash->mtd)) {
		return -EIO;
	}
#endif

/*add : gpio iotcl*/
#if 1
	ret = misc_register(&snx_sfgpio_miscdev);
        if (ret) 
	        goto out_free_info;
#endif

	return 0;

out_free_info:
	kfree(sf_flash);
	return err;
}

static int __devexit snx_sf_remove(struct platform_device *pdev)
{
	struct mssf *flash = platform_get_drvdata(pdev);
	platform_set_drvdata(pdev, NULL);
	DBG_ME ("%s:%d: ---->\n", __func__, __LINE__);
	if(flash) {
#ifdef CONFIG_MTD_PARTITIONS
		if(flash->parts) {
			del_mtd_partitions(&flash->mtd);
			kfree(flash->parts);
		} else {
			del_mtd_device(&flash->mtd);
		}
#else
		del_mtd_device(&flash->mtd);
#endif
#if 1
		if(dma_flag == 1) {
			dma_free_coherent(NULL, dma_alloc_size, _buf_addr, addr_dma_sf);
			dma_flag=0;
		}
#endif
		map_destroy(&flash->mtd);
		release_mem_region(flash->map.phys, (uint32_t)flash->map.size);
		iounmap(flash->map.virt);
		kfree(flash);
	}
	return 0;
}

static struct platform_driver snx_sf_driver = {
	.probe  = snx_sf_probe,
	.remove = __devexit_p(snx_sf_remove),
	.driver = {
		.name   = "snx_sf",
		.owner  = THIS_MODULE,
        },
};

static int snx_sf_init(void)
{
//	printk("SNX_SF_INIT..\n");
	return platform_driver_register(&snx_sf_driver);
}

static void snx_sf_exit(void)
{
	platform_driver_unregister(&snx_sf_driver);
}

module_init(snx_sf_init);
module_exit(snx_sf_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nora Chang");
MODULE_DESCRIPTION("MTD Mass Storage Serial Flash driver for MX25L6405D flash chips");
