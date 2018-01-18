/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/blkdev.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/dma-mapping.h>
#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
#include <linux/mmc/sd.h>
#include <linux/mmc/mmc.h>
#include <linux/clk.h>
#include <linux/irq.h>

#include <asm/dma.h>
#include <asm/dma-mapping.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <mach/regs-ms.h>
#include <mach/ms.h>
#include "mmc.h" 
#include "snx_sd.h"


//#define MMC_DEBUG
 
#ifdef MMC_DEBUG
#define DBG(x...)       printk(KERN_INFO x)
#else
#define DBG(x...)       do { } while (0)
#endif

//#define DEBUG_ME
#ifdef DEBUG_ME
#define DBG_ME(x...) 	printk(KERN_INFO x)
#else
#define DBG_ME(x...)   do { } while (0)
#endif

#define DRIVER_NAME "snx_sd" 
#define PFX DRIVER_NAME ": "

#define SD_ENABLE	1
#define SD_DISABLE 	0
#define SD_CTL_MODE_SPI	0x1
#define SD_CTL_MODE_SD	0x2
#define SD_READ_MODE    0x1
#define SD_WRITE_MODE   0x0
/*SD BUS WIDTH */
#define SD_1BIT         0x0
#define SD_4BIT         0x2

//for MS2
#define SD_CD_PIN	1	//card_detect: MS2 MS_IO[1]
#define SD_WP_PIN	2	//write_protect: MS2 MS_IO[2]

#define MSRDY_CNT	0xffffffff

static void __iomem *base = NULL;	//MS base virtual addr

int cmd2_flag=0;
int card_stat; //0:exit, 1: notexit
static struct mmc_host 	*mmc = NULL;

#define RESSIZE(ressource) (((ressource)->end - (ressource)->start)+1)

static irqreturn_t snx_sd_irq(int irq, void *dev_id)
{
	unsigned long iflags;
	struct snx_sd_host *host = (struct snx_sd_host *)dev_id;
	
	DBG_ME("snx_sd_irq:	<--------\n");

	/* Check for things not supposed to happen */
	if(!host){ 
		DBG_ME("snx_sd_irq:	host == NULL\n"); //wuyang
		return IRQ_HANDLED;
	}
	
	spin_lock_irqsave( &host->complete_lock, iflags);
	if(ms_check_msrdy_flag(base)){
		host->mrq->cmd->error = MMC_ERR_NONE;
		ms_clear_msrdy_flag(base, 0x1); 
		if(ms_check_mserr_flag(base)) {
			printk(KERN_ERR"snx_sd_irq: host->mrq->data->error = MMC_ERR_FAILED 1\n");
			host->mrq->data->error = MMC_ERR_FAILED;
			ms_clear_mserr_flag(base, 0x1);
		}
		else {
			host->mrq->data->error = MMC_ERR_NONE;
			goto transfer_closed;
		}
	}
	else{
		host->mrq->cmd->error = MMC_ERR_FAILED;
		printk(KERN_ERR"snx_sd_irq: host->mrq->cmd->error = MMC_ERR_FAILED\n");
		if(ms_check_mserr_flag(base)) {
			printk(KERN_ERR"snx_sd_irq: host->mrq->data->error = MMC_ERR_FAILED 2\n");
			host->mrq->data->error = MMC_ERR_FAILED;
			ms_clear_mserr_flag(base, 0x1);
		}
		else {
                     host->mrq->data->error = MMC_ERR_NONE;
		}
	}	
	spin_unlock_irqrestore( &host->complete_lock, iflags);
	return IRQ_HANDLED;

transfer_closed:
	spin_unlock_irqrestore( &host->complete_lock, iflags);
	complete(&host->complete_dma); 
	return IRQ_HANDLED;
}

/* ISR for the CardDetect Pin */
/*
static irqreturn_t snx_sd_irq_cd(int irq, void *dev_id)
{
	struct snx_sd_host *host = (struct snx_sd_host *)dev_id;
	unsigned int data;

	//MS2
	data = *((volatile unsigned *)(MS_MS_IO_I)); 
	data = (data & (0x1<<SD_CD_PIN))>>SD_CD_PIN;
	
	DBG_ME("snx_sd_irq_cd: ard_stat=%d data=%d\n", card_stat, data);

	if(data != card_stat){
		cmd2_flag = 0;
		card_stat = data;
		mmc_detect_change(host->mmc, SNXSDI_CDLATENCY);
	}
	
	DBG_ME("snx_sd_irq_cd: card_stat=%d data=%d\n", card_stat, data);
	
	ms_clear_cd_intr_flag(0x1); //modified by gtm
	
	return IRQ_HANDLED;
}
*/
static int snx_sd_check_msrdy(void __iomem *base, int cnt)
{
	int ret= 0;
//	DBG(PFX "snx_sd_check_msrdy:	-------->\n");
        while (cnt){
        	cnt--;
	        if(ms_check_msrdy(base)){
	        	ret = 0;
		        break; //ready
		}
	        if((ms_check_msrdy(base)==0) && (cnt==0)){
		        ret = 1; //false
		        printk(KERN_ERR PFX "snx_sd_check_msrdy: ERROR MSRDY not ready\n"); 
		}
	}
	return ret;
}


static int snx_sd_mdma_write(u32 dma_size, dma_addr_t addr_dma, int dma_bnum, u32 timecnt, struct completion *complete_dma) 
{		
	dma_size = dma_size - 1;
	dma_bnum = dma_bnum - 1;
       DBG_ME("snx_sd_mdma_write: dma_size=%x, addr_dma=%x, dma_bnum=%d, timecnt=%x\n", 
	   	dma_size, addr_dma, dma_bnum, timecnt);

	//set MDMA of Mass_storage 
	ms_set_dmasize(base, dma_size);
	ms_set_timecount(base, timecnt);
	ms_set_dmaaddr(base, addr_dma); 
	ms_msmdma_en_switch(base, SD_ENABLE);
	ms_dmablock(base, dma_bnum);
	ms_msdma_rw_switch(base, SD_WRITE_MODE);
	ms_msdma_en_switch(base, SD_ENABLE);

	wait_for_completion(complete_dma);
	DBG_ME("snx_sd_mdma_write: [DAT] DMA complete.\n");
	
	if(snx_sd_check_msrdy(base, MSRDY_CNT)){
		printk(KERN_ERR PFX "snx_sd_mdma_write: ERROR set mdma reg -> msrdy error\n");
		goto msmdma_write_error;
	}
	
	if(ms_check_msmdma_ok(base)){
	        if(ms_check_msmdma_timeout(base)){
	 	       printk(KERN_ERR PFX "snx_sd_mdma_write: ERROR M DMA not complete with Time out flag and  mdma out flag not valid\n"); 
	 	       goto msmdma_write_error;
	        }
	        else{
                       DBG_ME(PFX "snx_sd_mdma_write: OK M DMA complete and no Time out flag\n");	
               }
	}
	else{
		if(ms_check_msmdma_timeout(base)){
	        	printk(KERN_ERR PFX "snx_sd_mdma_write: ERROR M DMA not complete with Time out flag!!!\n");
                }
                else{
			printk(KERN_ERR PFX "snx_sd_mdma_write: ERROR M DMA not complete!!!\n");
		}
		goto msmdma_write_error;
	}
	
	ms_msdma_en_switch(base, SD_DISABLE);
	ms_msmdma_en_switch(base, SD_DISABLE);
	//if data write to sd card
	/*
	ms_set_spibusy_tri(SD_ENABLE);
	if(snx_sd_check_msrdy(MSRDY_CNT)){
	        printk(KERN_ERR PFX "snx_sd_mdma_write: ERROR spibusytri -> msrdy error\n");
	        return 1;
	}
	*/
        if(ms_read_sudmablock(base) == (dma_bnum+1)){
		DBG_ME("snx_sd_mdma_write: OK read_sudmablock=(dma_bnum+1)\n");
	        return 0;    
	}
	else{
		printk(KERN_ERR PFX "snx_sd_mdma_write: ERROR Success Block Num Error\n");
	        return 1;
	}
	
msmdma_write_error:
        ms_msdma_en_switch(base, SD_DISABLE); 
        ms_msmdma_en_switch(base, SD_DISABLE);
	return 1;
}

static int snx_sd_mdma_read(u32 dma_size, dma_addr_t addr_dma, int dma_bnum, u32 timecnt, struct completion *complete_dma) 
{
	dma_size = dma_size - 1;
	dma_bnum = dma_bnum - 1;
       DBG_ME("snx_sd_mdma_read: dma_size=%x, addr_dma=%x, dma_bnum=%d, timecnt=%x\n", 
			dma_size, addr_dma, dma_bnum, timecnt);

	//set MDMA of Mass_storage 
	ms_set_dmasize(base, dma_size);
	ms_set_timecount(base, timecnt);
	ms_set_dmaaddr(base, addr_dma); 
	ms_msmdma_en_switch(base, SD_ENABLE);
	ms_dmablock(base, dma_bnum);
	ms_msdma_rw_switch(base, SD_READ_MODE);
	ms_msdma_en_switch(base, SD_ENABLE);
	
	wait_for_completion(complete_dma);
	DBG_ME("snx_sd_mdma_read: [DAT] DMA complete.\n");
	                        
	if(snx_sd_check_msrdy(base, MSRDY_CNT)){
		printk(KERN_ERR PFX "snx_sd_mdma_read: ERROR set mdma reg -> msrdy error\n");
		goto msmdma_read_error;
	}
	if(ms_check_msmdma_ok(base)){
	        if(ms_check_msmdma_timeout(base)){
	 	       printk(KERN_ERR PFX "snx_sd_mdma_read: ERROR M DMA not complete with Time out flag and mdma out flag not valid\n");
	 	       goto msmdma_read_error;
	        }
	        else{
                       DBG_ME("snx_sd_mdma_read: OK M DMA complete and no Time out flag\n");	
               }
	}
	else{
		if(ms_check_msmdma_timeout(base)){
			printk(KERN_ERR PFX "snx_sd_mdma_raed: ERROR M DMA not complete with Time out flag!!!\n");
                }
                else{
			printk(KERN_ERR PFX "snx_sd_mdma_read: ERROR M DMA not complete!!!\n");
		}
		goto msmdma_read_error;
	}
	ms_msdma_en_switch(base, SD_DISABLE);
	ms_msmdma_en_switch(base, SD_DISABLE);
	
        if(ms_read_sudmablock(base) == (dma_bnum+1)){
		DBG_ME("snx_sd_mdma_read:	OK read_sudmablock=(dma_bnum+1)\n");
	        return 0;    
	}
	else{
		printk(KERN_ERR PFX "snx_sd_mdma_read: ERROR Success Block Num Error\n");
	        return 1;
	}
	
msmdma_read_error:
        ms_msdma_en_switch(base, SD_DISABLE); 
        ms_msmdma_en_switch(base, SD_DISABLE);
	return 1;
}

static int snx_sd_dma_write(u32 dma_size, dma_addr_t addr_dma, unsigned char bus_width, struct completion *complete_dma) 
{
	u32 resp, resp1, resp2;
	
	dma_size = dma_size - 1;
       DBG_ME("snx_sd_dma_write: dma_size=%x, addr_dma=%x, bus_width=%x\n", dma_size, addr_dma, bus_width);

        //set DMA of Mass_storage 
	ms_msdma_rw_switch(base, SD_WRITE_MODE);
	ms_set_dmasize(base, dma_size);
	ms_set_dmaaddr(base, addr_dma);
	//send 8 dummy clock
	ms_msreg_rw_switch(base, SD_WRITE_MODE);
	ms_write_dummy(base, 0x1);
	if(snx_sd_check_msrdy(base, MSRDY_CNT)){
		printk(KERN_ERR PFX "snx_sd_dma_write:  ERROR   send 8 dummy clock -> msrdy error\n");
		goto dma_write_error;
	}
	// start DMA write
	if(bus_width == MMC_BUS_WIDTH_4)
		ms_write_data(base, 0xf0);
	else
		ms_write_data(base, 0xfe);
	if(snx_sd_check_msrdy(base, MSRDY_CNT)){
		printk(KERN_ERR PFX "snx_sd_dma_write:  ERROR   dma write start -> msrdy error\n");
		goto dma_write_error;			
	}	
	//start DMA transfer		
	ms_msdma_en_switch(base, SD_ENABLE);
	
	wait_for_completion(complete_dma);
	DBG(PFX "snx_sd_dma_write:    [DAT] DMA complete.\n");
	
	if(snx_sd_check_msrdy(base, MSRDY_CNT)){
		printk(KERN_ERR PFX "snx_sd_dma_write:  ERROR   start DMA transfer -> msrdy error\n");
		//disable DMA
		ms_msdma_en_switch(base, SD_DISABLE);	
		goto dma_write_error;
	}
	//disable DMA
	ms_msdma_en_switch(base, SD_DISABLE);

	//read response?
	resp1 = ms_read_spi_index(base);
	resp2 = ms_read_spi_cmd(base);
	resp = ((resp1&0xff)<<24) | ((resp2&0xffffff)>>8);
	//if(resp!=0x05000009)
	//	DBG(PFX "Error SD mode dma write CRC error??\n");
		
	resp = (resp&0xe000000)>>25;
	if(resp == 0x5){
		printk(KERN_ERR PFX "snx_sd_dma_write: ERROR Write CRC Error at DMA WRITE MODE\n");
	}
	//if data write to sd card
	ms_set_spibusy_tri(base, SD_ENABLE);
	if(snx_sd_check_msrdy(base, MSRDY_CNT)){
	        printk(KERN_ERR PFX "snx_sd_dma_write: ERROR spibusytri -> msrdy error\n");
	        goto dma_write_error;
	}
	//send 8 dummy clock
	ms_msreg_rw_switch(base, SD_WRITE_MODE);
	ms_write_dummy(base, 0x1);
	if(snx_sd_check_msrdy(base, MSRDY_CNT)){
		printk(KERN_ERR PFX "snx_sd_dma_write: ERROR send 8 dummy clock msrdy error\n");
		goto dma_write_error;
	}

	return 0;

dma_write_error:	
	return 1;
}

static int snx_sd_dma_read(u32 dma_size, dma_addr_t addr_dma, struct completion *complete_dma)
{
	dma_size = dma_size - 1;
	DBG_ME("snx_sd_dma_read: dma_size=%x, addr_dma=%x\n", dma_size, addr_dma);
	
	ms_ecc_en_switch(base, SD_DISABLE);
	ms_ecc_en_switch(base, SD_ENABLE);
	//set DMA of Mass_storage 
	ms_msdma_rw_switch(base, SD_READ_MODE);
	ms_set_dmasize(base, dma_size);
	ms_set_dmaaddr(base, addr_dma);
	//start DMA transfer		
	ms_msdma_en_switch(base, SD_ENABLE);
		
	wait_for_completion(complete_dma);
	DBG_ME("snx_sd_dma_read: [DAT] DMA complete.\n");
	
	if(snx_sd_check_msrdy(base, MSRDY_CNT)){
		printk(KERN_ERR PFX "snx_sd_dma_read: ERROR start DMA transfer -> msrdy error\n");
		//disable DMA
		ms_msdma_en_switch(base, SD_DISABLE);
		goto dma_read_error;
	}
	//disable DMA
	ms_msdma_en_switch(base, SD_DISABLE);
	//send 8 dummy clock
	ms_write_dummy(base, 0x1);
	if(snx_sd_check_msrdy(base, MSRDY_CNT)){
		printk(KERN_ERR PFX "snx_sd_dma_read: ERROR send 8 dummy clock -> msrdy error\n");
		goto dma_read_error;
	}
	
	//check crc
	if(ms_check_crc_err(base)) 
		goto crc_error;	
	
	return 0;
	
dma_read_error:
	printk(KERN_ERR PFX "snx_sd_dma_read: ERROR dma read error\n");
	return 1;
crc_error:
	//printk(KERN_ERR PFX "snx_sd_dma_read: ERROR crc error\n");
	return 0;
}

/*
static void snx_sd_set_card_detect(unsigned int pin)
{
	unsigned int data;
	
	data = *((volatile unsigned *)(MS_MS_IO_OE));
	data &= ~(0x1<<pin);
	*((volatile unsigned *)(MS_MS_IO_OE)) = data;
	//detect card_stat
	data = *((volatile unsigned *)(MS_MS_IO_I)); 
	card_stat = (data & (0x1<<pin))>>pin; 
	DBG_ME("snx_sd_set_card_detect: card_stat=%d\n", card_stat);
 
}
static void snx_sd_set_write_protect(unsigned int pin)
{
	unsigned int data;
	DBG(PFX "snx_sd_set_write_protect:      -------->\n");
	
	data = *((volatile unsigned *)(MS_MS_IO_OE));
	data &= ~(0x1<<pin);
	*((volatile unsigned *)(MS_MS_IO_OE)) = data;
	//dbg : write protect stat
        data = *((volatile unsigned *)(MS_MS_IO_I)); 
        DBG(PFX "snx_sd_set_write_protect: MS_MS_IO_I=%x\n", data);
}
*/
static int snx_sd_scmd_rresp(struct mmc_request *mrq) {
	
	u32 resp1, resp2;

	ms_msreg_rw_switch(base, SD_WRITE_MODE);
	ms_write_spi_index(base, mrq->cmd->opcode);
	ms_write_spi_cmd(base, mrq->cmd->arg);
	
	if((mrq->cmd->opcode==MMC_READ_MULTIPLE_BLOCK)
		|| (mrq->cmd->opcode==MMC_WRITE_MULTIPLE_BLOCK)) {
//		|| (mrq->cmd->opcode==MMC_WRITE_BLOCK)) {
		DBG_ME("snx_sd_scmd_rresp: opcode is MMC_READ_MULTIPLE_BLOCK"
			"OR MMC_WRITE_MULTIPLE_BLOCK\n");
	}
	else {	
		ms_set_spicmd_tri(base, SD_ENABLE);
		if(snx_sd_check_msrdy(base, MSRDY_CNT)){
			printk(KERN_ERR PFX "snx_sd_scmd_rresp: ERROR   SD send cmd fail.\n");
			goto scmd_rresp_error;
		}
		
		if(mrq->cmd->opcode==MMC_SELECT_CARD){
			ms_set_spibusy_tri(base, SD_ENABLE);
			if(snx_sd_check_msrdy(base, MSRDY_CNT)){
			        printk(KERN_ERR PFX "snx_sd_scmd_rresp:   ERROR spibusytri -> msrdy error\n");
			        goto scmd_rresp_error;
			}
		}

		/*  Read  */
		if (mmc_resp_type(mrq->cmd) == MMC_RSP_NONE){
			DBG_ME("snx_sd_scmd_rresp: Resp type is MMC_RSP_NONE\n");
		}
		else if (mmc_resp_type(mrq->cmd) == MMC_RSP_R2)
		{
			DBG_ME("snx_sd_scmd_rresp: Resp type is MMC_RSP_R2\n");
			
			//Get response 2-5 byte
			resp1 = ms_read_spi_index(base);
			resp2 = ms_read_spi_cmd(base);
			mrq->cmd->resp[0] = resp2;
			
			//Get response 6-10 byte
			ms_msreg_rw_switch(base, SD_READ_MODE);
			ms_write_autoresponse(base, 0x1);//write value can be any value
			if(snx_sd_check_msrdy(base, MSRDY_CNT)){
				printk(KERN_ERR PFX "snx_sd_scmd_rresp: ERROR   read resp2 6-10 byte msrdy error\n");
				goto scmd_rresp_error;
			}
			resp1 = ms_read_spi_index(base);
			resp2 = ms_read_spi_cmd(base);
			mrq->cmd->resp[1] = (resp1&0x000000ff)<<24;
			mrq->cmd->resp[1] |= (resp2&0xffffff00)>>8;
			mrq->cmd->resp[2] = (resp2&0x000000ff)<<24;
			
		        //Get Response 11-15
	        	ms_msreg_rw_switch(base, SD_READ_MODE);
		        ms_write_autoresponse(base, 0x1);//write value can be any value
		        if(snx_sd_check_msrdy(base, MSRDY_CNT)){
	        		printk(KERN_ERR PFX "snx_sd_scmd_rresp: ERROR   read resp2 11-15 byte msrdy error\n");
		             	goto scmd_rresp_error;
	             	}
			resp1 = ms_read_spi_index(base);
			resp2 = ms_read_spi_cmd(base);
			mrq->cmd->resp[2] |= (resp1&0x000000ff)<<16;
			mrq->cmd->resp[2] |= (resp2&0xffff0000)>>16;
			mrq->cmd->resp[3] = (resp2&0x0000ffff)<<16;
			
	        	//Get respose 16
		        ms_write_cmd(base, 0x1);
		        if(snx_sd_check_msrdy(base, MSRDY_CNT)){
	        		printk(KERN_ERR PFX "snx_sd_scmd_rresp: ERROR   read resp2 16 byte msrdy error\n");
		        	goto scmd_rresp_error;
			}
	        	resp1 = ms_read_cmd(base);
		        mrq->cmd->resp[3] |= (resp1&0x000000ff)<<8;
				
	        	//Get respose 17
		        ms_write_cmd(base, 0x1);
		        if(snx_sd_check_msrdy(base, MSRDY_CNT)){
	        		printk(KERN_ERR PFX "snx_sd_scmd_rresp: ERROR   read resp2 17 byte msrdy error\n");
		               	goto scmd_rresp_error;
			}
	        	resp1 = ms_read_cmd(base);
		        mrq->cmd->resp[3] |= (resp1&0x000000ff)>>0;
	        	
		        if ((mrq->cmd->resp[0]==0xffffffff)&&(mrq->cmd->resp[1]==0xffffffff)
		        	&&(mrq->cmd->resp[2]==0xffffffff)&&(mrq->cmd->resp[3]==0xffffffff)
		        	&&(cmd2_flag==1)) {
	        		//printk(KERN_ERR PFX "snx_sd_scmd_rresp: ERROR MMC_ERR_TIMEOUT 1\n");
	        		mrq->cmd->error = MMC_ERR_TIMEOUT;
			}
			else if ((mrq->cmd->resp[0]==0x0)&&(mrq->cmd->resp[1]==0x0)
				&&(mrq->cmd->resp[2]==0x0)&&(mrq->cmd->resp[3]==0x0)
				&&(cmd2_flag==1)) {
	        		//printk(KERN_ERR PFX "snx_sd_scmd_rresp:ERROR MMC_ERR_TIMEOUT 2\n");
	        		mrq->cmd->error = MMC_ERR_TIMEOUT;
			}
			else if ((mrq->cmd->resp[0]==0xffffffff)&&(mrq->cmd->resp[1]==0xffffffff)
		        	&&(mrq->cmd->resp[2]==0xffffffff)&&(mrq->cmd->resp[3]==0xffffffff)
		        	&&(cmd2_flag!=1)) {
	        		//printk(KERN_ERR PFX "snx_sd_scmd_rresp: ERROR MMC_ERR_FAILED 1\n");
	        		mrq->cmd->error = MMC_ERR_FAILED;
			}
			else
				cmd2_flag=1;
		}
		else
		{
			DBG_ME("snx_sd_scmd_rresp: Resp type is other\n");
			resp1 = ms_read_spi_index(base);
			resp2 = ms_read_spi_cmd(base);
			mrq->cmd->resp[0] = resp2;
			mrq->cmd->resp[1] = resp1;
		        
		        if ((mrq->cmd->resp[0]==0xffffffff)&&(mrq->cmd->resp[1]==0xff)) {
				//printk(KERN_ERR PFX "snx_sd_scmd_rresp: ERROR MMC_ERR_FAILED 2\n");
	        		mrq->cmd->error = MMC_ERR_FAILED;
			}
	        }
	        if(mrq->cmd->opcode!=SD_APP_SEND_SCR){
	        	if((mrq->cmd->opcode==SD_SWITCH) && (mrq->cmd->arg!=SD_BUS_WIDTH_1)
					&&(mrq->cmd->arg!=SD_BUS_WIDTH_4)){
	        		;
			}
			else if((mrq->cmd->opcode==MMC_READ_SINGLE_BLOCK) || 
				(mrq->cmd->opcode==MMC_WRITE_BLOCK)) {
				;
			}
			else{
				//send 8 dummy clock
				ms_msreg_rw_switch(base, SD_WRITE_MODE);
				ms_write_dummy(base, 0x1);
				if(snx_sd_check_msrdy(base, MSRDY_CNT)){
					printk(KERN_ERR PFX "snx_sd_scmd_rresp: ERROR   send 8 dummy clock msrdy error\n");
					goto scmd_rresp_error;
				}
			}
		}
		
	        
	}
	DBG_ME("snx_sd_scmd_rresp: mrq->cmd->opcode=%d\n", mrq->cmd->opcode);
	DBG_ME("snx_sd_scmd_rresp: mrq->cmd->arg=0x%x\n", mrq->cmd->arg);
	DBG_ME("snx_sd_scmd_rresp: mrq->cmd->resp[0]=0x%x\n", mrq->cmd->resp[0]);
	DBG_ME("snx_sd_scmd_rresp: mrq->cmd->resp[1]=0x%x\n", mrq->cmd->resp[1]);
	DBG_ME("snx_sd_scmd_rresp: mrq->cmd->resp[2]=0x%x\n", mrq->cmd->resp[2]);
	DBG_ME("snx_sd_scmd_rresp: mrq->cmd->resp[3]=0x%x\n", mrq->cmd->resp[3]);

	return 0;

scmd_rresp_error:
	mrq->cmd->error = MMC_ERR_FAILED;
	return 1;
}

static int snx_sd_read_data(struct mmc_request *mrq, int dma_len, u32 dma_size, struct completion *complete_dma){
	int i, j, res, sdi_bnum, sdi_bnum_su;	
	dma_addr_t addr_dma;
	char *addr = NULL;
	char *sg_addr = NULL;
	u32 mdma_timecnt;
	
	for(i=0; i<dma_len; i++) { 
		if(sg_dma_len(&mrq->data->sg[i]) == 512) {
			res = snx_sd_dma_read(sg_dma_len(&mrq->data->sg[i]), sg_dma_address(&mrq->data->sg[i]),
				complete_dma);
			
			DBG_ME("snx_sd_read_data: sg_dma_len(&mrq->data->sg[i])=%u, sg_dma_address(&mrq->data->sg[i])=%x,"
				" dma_size=%u\n", sg_dma_len(&mrq->data->sg[i]),sg_dma_address(&mrq->data->sg[i]), dma_size);

			if(mrq->data->error == MMC_ERR_NONE) {		
				mrq->data->bytes_xfered += sg_dma_len(&mrq->data->sg[i]);
			} else {
				printk(KERN_ERR PFX "snx_sd_read_data: ERROR mrq->data->error != MMC_ERR_NONE 1\n");
				mrq->data->bytes_xfered += 0;
			}
		}
		else if (sg_dma_len(&mrq->data->sg[i]) > 512) {
			addr_dma = sg_dma_address(&mrq->data->sg[i]);
			sdi_bnum = sg_dma_len(&mrq->data->sg[i]) / dma_size;	
			
			DBG_ME("snx_sd_read_data: sg_dma_len(&mrq->data->sg[i])=%u, sg_dma_address(&mrq->data->sg[i])=%x,"
				"sdi_bnum = %u \n", sg_dma_len(&mrq->data->sg[i]),sg_dma_address(&mrq->data->sg[i]), sdi_bnum);
			
			//use mdma read
			mdma_timecnt = 0x3fffffff;
			res = snx_sd_mdma_read(dma_size,addr_dma,sdi_bnum, mdma_timecnt, complete_dma);	
			if(mrq->data->error == MMC_ERR_NONE) {	
				sdi_bnum_su = ms_read_sudmablock(base);
				mrq->data->bytes_xfered += dma_size*sdi_bnum_su;
			} else {
				printk(KERN_ERR PFX "snx_sd_read_data: ERROR mrq->data->error != MMC_ERR_NONE 2\n");
				mrq->data->bytes_xfered += 0;
			}
		}
		else {
			if(mrq->cmd->opcode==SD_APP_SEND_SCR)
			      	dma_size = 16;
			      	//dma_size = 8;
			else if(mrq->cmd->opcode==SD_SWITCH)
				dma_size = 64;
			else
				dma_size = sg_dma_len(&mrq->data->sg[i]);
			
			DBG_ME("snx_sd_read_data: <512 sg_dma_len(&mrq->data->sg[i])=%u, dma_size=%u\n", 
				sg_dma_len(&mrq->data->sg[i]), dma_size);
		                        
		        addr = dma_alloc_coherent(NULL, dma_size, &addr_dma, GFP_KERNEL | GFP_DMA);
		        if (addr == NULL) {
                        	res = -ENOMEM;
				goto read_data_error;
			}
			memset(addr, 0, dma_size);

			res = snx_sd_dma_read(dma_size, addr_dma, complete_dma);
						
                     //sg_addr = page_address(mrq->data->sg[i].page)+mrq->data->sg[i].offset;
			sg_addr = sg_virt(&mrq->data->sg[i]);                   
			for(j=0; j<sg_dma_len(&mrq->data->sg[i]); j++){
				*(sg_addr+j) = *(addr+j);
				DBG_ME(PFX "snx_sd_read_data:      *(sg_addr+%d)=%x\n", j, *(sg_addr+j));
			}
			if(mrq->data->error == MMC_ERR_NONE) {	
				mrq->data->bytes_xfered += sg_dma_len(&mrq->data->sg[i]); 
			} else {
				printk(KERN_ERR PFX "snx_sd_read_data: ERROR mrq->data->error != MMC_ERR_NONE 3\n");
				mrq->data->bytes_xfered = 0;
			}
			dma_free_coherent(NULL, dma_size,  addr, addr_dma);
		}
		DBG_ME("snx_sd_read_data: mrq->data->bytes_xfered=%x\n", mrq->data->bytes_xfered); 
	}
	return 0;

read_data_error:
	dma_free_coherent(NULL, dma_size,  addr, addr_dma);
	return 1;
}

static int snx_sd_write_data(struct mmc_request *mrq, int dma_len, u32 dma_size, unsigned char bus_width, struct completion *complete_dma){
	int i, res, sdi_bnum, sdi_bnum_su;	
	dma_addr_t addr_dma;
	u32 mdma_timecnt;

	for(i=0; i<dma_len; i++) {
		if(sg_dma_len(&mrq->data->sg[i]) == 512) { 
			res = snx_sd_dma_write(sg_dma_len(&mrq->data->sg[i]), sg_dma_address(&mrq->data->sg[i]), 
				bus_width, complete_dma);
			DBG_ME("snx_sd_write_data: sg_dma_len(&mrq->data->sg[i]=%u, dma_size=%u\n", 
				sg_dma_len(&mrq->data->sg[i]), dma_size);
			if(mrq->data->error == MMC_ERR_NONE) {
				mrq->data->bytes_xfered += sg_dma_len(&mrq->data->sg[i]);
			} else {
				printk(KERN_ERR PFX "snx_sd_write_data: ERROR	mrq->data->error != MMC_ERR_NONE 1\n");
				mrq->data->bytes_xfered += 0;
			}
		}
		else if (sg_dma_len(&mrq->data->sg[i]) > 512) {
			addr_dma = sg_dma_address(&mrq->data->sg[i]);
			sdi_bnum = sg_dma_len(&mrq->data->sg[i])/dma_size;	
			DBG_ME("snx_sd_write_data: sg_dma_len(&mrq->data->sg[i])=%u, dma_size=%u, sdi_bnum=%u\n", 
				sg_dma_len(&mrq->data->sg[i]), dma_size,sdi_bnum);

			//use mdma write
			mdma_timecnt = 0x3fffffff;
			res = snx_sd_mdma_write(dma_size,addr_dma,sdi_bnum, mdma_timecnt, complete_dma);									
			if(mrq->data->error == MMC_ERR_NONE) {
				sdi_bnum_su = ms_read_sudmablock(base);
				mrq->data->bytes_xfered += dma_size*sdi_bnum_su;
			} else {
				printk(KERN_ERR PFX "snx_sd_write_data: ERROR mrq->data->error != MMC_ERR_NONE 2\n");
				mrq->data->bytes_xfered += 0;
			}
		}
		else {
			printk(KERN_ERR PFX "snx_sd_write_data: error situation: dma_size can't < 512\n");
		}
		DBG_ME("snx_sd_read_data: mrq->data->bytes_xfered=0x%x\n", mrq->data->bytes_xfered);
	}
	if(mrq->data->error != MMC_ERR_NONE)
		return 1;
	else
		return 0;
}
static void snx_sd_request(struct mmc_host *mmc, struct mmc_request *mrq) {
 	
 	struct snx_sd_host *host = mmc_priv(mmc);
	struct device *dev = mmc_dev(host->mmc);
	struct platform_device *pdev = to_platform_device(dev);
	
	u32 sdi_bsize, dma_size;
	int dma_len,msrdy_fail_flag, scr_dma_err_flag;
#ifdef DEBUG_ME
	int i, j = 0;	//wuyang
#endif
	dma_len = 0;
	msrdy_fail_flag=0;
	scr_dma_err_flag = 0;	
	sdi_bsize= 0;
	dma_size = 512;

	
	host->mrq = mrq;
	
	//for scr read
	if(mrq->cmd->opcode==SD_APP_SEND_SCR){
		ms_msreg_rw_switch(base, SD_WRITE_MODE);
		ms_read_data_cmd(base, SD_ENABLE);
	}
	
	//send command
	if(snx_sd_scmd_rresp(mrq)){
		goto request_done;
	}

	//data r/w	
	if ((mrq->data)&&(mrq->cmd->error != MMC_ERR_FAILED)) {
		init_completion(&host->complete_dma);
		
		mrq->data->bytes_xfered = 0;
		sdi_bsize = mrq->data->blksz;
		host->size = mrq->data->blksz;
		
		DBG_ME("request: [DAT] bsize:%u blocks:%u bytes:%u\n",
			sdi_bsize, mrq->data->blocks, mrq->data->blocks * sdi_bsize);

		dma_len = dma_map_sg(&pdev->dev, mrq->data->sg, mrq->data->sg_len,
			mrq->data->flags & MMC_DATA_READ ? DMA_FROM_DEVICE : DMA_TO_DEVICE);

		if(!(mrq->data->flags & MMC_DATA_WRITE)) {
			DBG_ME("snx_sd_request:  Read data\n");
			if(snx_sd_read_data(mrq, dma_len, dma_size, &host->complete_dma)){
				printk(KERN_ERR PFX "snx_sd_request:   ERROR read data error\n");
			}	
		}
		
		if(mrq->data->flags & MMC_DATA_WRITE) {
			struct scatterlist *sg;
			sg=&mrq->data->sg[0];
			host->pio_words=sdi_bsize>>2;
			
			//host->pio_ptr=page_address(sg->page)+sg->offset;
			host->pio_ptr=sg_virt(sg);

#ifdef DEBUG_ME		//wuyang
			printk("\n**************************************\n");
			while(j < mrq->data->blocks){
				printk("\n block%x: ",j);

				i = 0;
				while(i<16)
					printk("%x ",*((char *)sg_virt(sg)+(j*sdi_bsize)+(i++)));
			j++;
			}
			printk("\n**************************************\n");
#endif
			DBG_ME("snx_sd_request:  Write  data\n");

			if(snx_sd_write_data(mrq, dma_len, dma_size, host->bus_width, &host->complete_dma)){
				printk(KERN_ERR PFX "snx_sd_request:   ERROR write data error\n");
			}
		}
		host->mrq = mrq;
	}	

	host->mrq = NULL;

	/* If we have no data transfer we are finished here */
	if (!mrq->data){
		goto request_done;
	}

	// FIXME no dma for write operation
	if (mrq->data) {
	        dma_unmap_sg(&pdev->dev, mrq->data->sg, dma_len, mrq->data->flags 
				& MMC_DATA_READ ? DMA_FROM_DEVICE : DMA_TO_DEVICE);
	}
	// wuyang
/*	if((mrq->data->stop)&&(!(mrq->data->flags&MMC_DATA_MULTI))) {
		mmc_wait_for_cmd(mmc, mrq->data->stop, 3);
	}
*/

request_done:
	DBG(PFX "snx_sd_request: 	SD reauest done.\n");

	if(mrq->cmd->opcode==SD_APP_SEND_SCR){
	        ms_msreg_rw_switch(base, SD_WRITE_MODE);
	        ms_read_data_cmd(base, SD_DISABLE);  
	        //send 8 dummy clock
		ms_msreg_rw_switch(base, SD_WRITE_MODE);
		ms_write_dummy(base, 0x1);
		if(snx_sd_check_msrdy(base, MSRDY_CNT)){
			printk(KERN_ERR PFX "snx_sd_request:	ERROR	send 8 dummy clock msrdy error\n");
		}
	}

        mrq->done(mrq);
        
}

static int snx_sd_enable(void)
{
	int ret = -1;
	
	ms_set_msmode(base, 0); //must not be SD mode before initialize,any value will work

	ms_set_msmode(base, SD_CTL_MODE_SD); //set to be sd mode
//	*((volatile unsigned *)(MS_MDMAECC)) = 0xff << 24;
	ms_cd_intr_en_switch(base, SD_ENABLE); 
	ms_extra_en_switch(base, SD_ENABLE);//set related register
       	ms_ecc_en_switch(base, SD_ENABLE);  
	ms_set_msspeed(base, 0x82); 
	ms_set_lba(base, 0x3ff);
	
	ms_msreg_rw_switch(base, SD_READ_MODE); //let sd card ready to accept cmd
	ms_write_dummy(base, 0x1);//write value can be any value

	ret = snx_sd_check_msrdy(base, MSRDY_CNT);
	if(ret)
		printk(KERN_ERR"SD initialisation failed.\n");
	else
		printk(KERN_INFO "SD initialisation done.\n");
		
	return ret;
}
static void snx_sd_disable(void)
{
	ms_set_msmode(base, 0); //must not be SD mode before initialize,any value will work
	ms_msrdy_Intr_en_switch(base, SD_DISABLE);
	ms_mserr_Intr_en_switch(base, SD_DISABLE);
}


static void snx_sd_set_ios(struct mmc_host *mmc, struct mmc_ios *ios) {
	struct snx_sd_host *host = mmc_priv(mmc);
	/* Set power */
	switch(ios->power_mode) {
		case MMC_POWER_ON:
		case MMC_POWER_UP:
			//sd enable interrupt
			ms_msrdy_Intr_en_switch(base, SD_ENABLE);
			ms_mserr_Intr_en_switch(base, SD_ENABLE);
			break;
			
		case MMC_POWER_OFF:
			//sd initial
			if(snx_sd_enable()){
				printk(KERN_ERR PFX "initialisation SD fail.\n");
			}
			ms_msrdy_Intr_en_switch(base, SD_DISABLE);
			ms_mserr_Intr_en_switch(base, SD_DISABLE);
			break;
		default:
			printk(KERN_ERR PFX "snx_sd_set_ios: Power Mode Unknow\n");
			/*	
			if (host->pdata->set_power)
				(host->pdata->set_power)(0);
			*/
	}

	host->bus_width = ios->bus_width;
	//ios->clock = clk_get_rate(host->clk) / 2;

	if(host->bus_width==2){
		ms_set_msspeed(base, 0x0);	
	}
	DBG_ME("snx_sd_set_ios: ios->clock=0x%x, bus_width=0x%x\n", ios->clock,host->bus_width);
}
/*
static int snx_sd_get_ro(struct mmc_host *mmc)
{
    struct snx_sd_host *host = mmc_priv(mmc);
    unsigned long flags;
    int present;
            
    DBG(PFX "snx_sd_get_ro:	---->\n");
            
    spin_lock_irqsave(&host->complete_lock, flags);
    //MS2
    present = *((volatile unsigned *)(MS_MS_IO_I));
    DBG(PFX "snx_sd_get_ro:   MS_MS_IO_I=%x\n", present);
    present = present&(0x1<<SD_WP_PIN);
    present = present>>SD_WP_PIN;
    present &= 0x01;
    DBG(PFX "snx_sd_get_ro:	present=%x\n", present);    

    spin_unlock_irqrestore(&host->complete_lock, flags);
                               
    return present;
}
*/


static struct mmc_host_ops snx_sd_ops = {
	.request = snx_sd_request, 
	.set_ios  = snx_sd_set_ios, 
//	.get_ro   = snx_sd_get_ro,	//sd card ReadOnly Flag.
};

/*
static struct sn926_mmc_platdata sn926_mmc_defplat = {
	.detect	= SD_CD_PIN,
	.wprotect = SD_WP_PIN,
	//.set_power	= 
	.set_power	= NULL,
	//.f_max		= 3000000,//?
	.ocr_avail	= MMC_VDD_32_33,
};
*/


static struct mmc_host* snx_sd_alloc_host(struct device *dev, int irq)
{
//	sn926_mmc_pdata_t	*pdata;
	struct mmc_host 	*mmc;
	struct snx_sd_host 	*host;
	int ret;
	                        
	mmc = mmc_alloc_host(sizeof(struct snx_sd_host), dev); 
	if (!mmc) {
		ret = -ENOMEM;
		goto probe_out;
	}
	DBG_ME("SD to Nand host %s allocated!\n", mmc_hostname(mmc));

	host = mmc_priv(mmc);
	host->irq = irq;
	
	spin_lock_init( &host->complete_lock );
	host->mmc 		= mmc;
/*	
	pdata = pdev->dev.platform_data;
	if (!pdata) {
		pdev->dev.platform_data = &sn926_mmc_defplat;
		pdata = &sn926_mmc_defplat;
	}
	host->pdata = pdata;
	
	//set detect intr enable
	ms_cd_intr_en_switch(SD_ENABLE); 
	snx_sd_set_card_detect(pdata->detect);	
	snx_sd_set_write_protect(pdata->wprotect);
	
	}
*/

	if(request_irq(host->irq, snx_sd_irq, 0, DRIVER_NAME, host)) {
		printk(KERN_ERR PFX "failed to request sdi interrupt.\n");
		ret = -ENOENT;
		goto probe_free_host;
	}

/*	host->irq_cd = platform_get_irq(pdev, 1);

	if(request_irq(host->irq_cd, snx_sd_irq_cd, IRQF_TRIGGER_RISING |
					IRQF_TRIGGER_FALLING, DRIVER_NAME, host)) {
		printk(KERN_ERR PFX "failed to request card detect interrupt.\n" );
		ret = -ENOENT;
		goto probe_free_irq;
	}
*/
	host->clk = clk_get(dev, "ms_clk");
	if (IS_ERR(host->clk)) {
		printk(KERN_ERR PFX "failed to find clock source.\n");
		ret = PTR_ERR(host->clk);
		host->clk = NULL;
		goto probe_free_host;
	}

	if((ret = clk_enable(host->clk))) {
		printk(KERN_ERR PFX "failed to enable clock source.\n");
		goto clk_unuse;
	}

	mmc->ops	= &snx_sd_ops; 
	mmc->ocr_avail  = MMC_VDD_32_33;
	mmc->f_min      = clk_get_rate(host->clk) / 512;
	mmc->f_max      = clk_get_rate(host->clk) / 2;
	mmc->caps       = MMC_CAP_4_BIT_DATA | MMC_CAP_SD_HIGHSPEED; // 4-bit bus width and highspeed
	
//	if(pdata->f_max && (mmc->f_max>pdata->f_max))
//		mmc->f_max = pdata->f_max;
	
	DBG(PFX "snx_sd_probe:	mmc->f_min=%x\n", mmc->f_min);
	DBG(PFX "snx_sd_probe:	mmc->f_max=%x\n", mmc->f_max);	
	/*
	 * Set the maximum segment size.  Since we aren't doing DMA
	 * (yet) we are only limited by the data length register.
	 */
#if 0
	//use default
	mmc->max_blk_count = 4096;
	mmc->max_blk_size   = 512;
	mmc->max_req_size = 4096 * 512; /* 512B per sector */
	mmc->max_seg_size = mmc->max_req_size;
#endif
	
	if((ret = mmc_add_host(mmc))) {
		printk(KERN_ERR PFX "failed to add mmc host.\n");
		goto clk_disable;
	}
/*
	mmc_detect_change(mmc, 0);

	platform_set_drvdata(pdev, mmc);

	ms_msrdy_Intr_en_switch(SD_ENABLE);
	ms_mserr_Intr_en_switch(SD_ENABLE);
*/	
	
	return mmc;
	
clk_disable:
	clk_disable(host->clk);

 clk_unuse:
	clk_put(host->clk);
/*
 probe_free_irq_cd:
 	free_irq(host->irq_cd, host);

 probe_free_irq:
  	free_irq(host->irq, host);

 probe_iounmap:
	iounmap(host->base);

 probe_free_mem_region:
	release_mem_region(host->mem->start, RESSIZE(host->mem));
*/
 probe_free_host:
	mmc_free_host(mmc);
 probe_out:
	return NULL;
}

static int snx_sd_free_host(void)
{
	struct snx_sd_host 	*host = mmc_priv(mmc);

//	mmc_remove_host(mmc);
	clk_disable(host->clk);
	clk_put(host->clk);
// 	free_irq(host->irq_cd, host);
 	free_irq(host->irq, host);
//	iounmap(host->base);
//	platform_set_drvdata(pdev, NULL);
//	release_mem_region(host->mem->start, RESSIZE(host->mem));
	mmc_free_host(mmc);
//	ms_cd_intr_en_switch(SD_DISABLE); 
	return 0;
}

extern void mmc_set_chip_select(struct mmc_host *host, int mode);

extern int mmc_go_idle(struct mmc_host *host);

int sn926sd_init(struct device *dev, void __iomem *addr, int irq,uint64_t *size)
{
	int ret = 0;
	DBG(PFX "sn926sd_init ---->\n");

	base = addr;

	mmc = snx_sd_alloc_host(dev, irq);
	if(!mmc){
		printk("uninitialized mmc_host!\n");
		return -1;
	}
	
	msleep(85);	// wait for card to finish initialization
	if(!mmc->card){
		printk("host->card uninitialized!\n");
		return -1;
	}
	
	DBG("card->cid.manfid = %d\n",mmc->card->cid.manfid);
	DBG("card->cid.oemid = 0x%x\n",mmc->card->cid.oemid);
	DBG("card->cid.prod_name = %s\n",mmc->card->cid.prod_name);
	DBG("card->cid.hwrev = %d\n",mmc->card->cid.hwrev);
	DBG("card->cid.fwrev = %d\n",mmc->card->cid.fwrev);
	DBG("card->cid.serial = 0x%x\n",mmc->card->cid.serial);
	DBG("card->cid.year = %d\n",mmc->card->cid.year);
	DBG("card->cid.month = %d\n",mmc->card->cid.month);
	
	DBG("card->csd.capacity = %d\n",mmc->card->csd.capacity);
	DBG("size = %lld\n",(uint64_t)(mmc->card->csd.capacity << (mmc->card->csd.read_blkbits - 9))<<9);

	*size = (uint64_t)(mmc->card->csd.capacity << (mmc->card->csd.read_blkbits - 9)) << 9;

	return ret;
}
int sn926sd_exit(void)
{
	DBG_ME("sn926sd_exit --->\n");
	snx_sd_disable();
	snx_sd_free_host();
	
	return 0;
}

/*
 * Wait for the card to finish the busy state
 */
/*
static int mmc_wait_busy(struct mmc_card *card)
{
	int ret, busy;
	struct mmc_command cmd;

	busy = 0;
	do {
		memset(&cmd, 0, sizeof(struct mmc_command));

		cmd.opcode = MMC_SEND_STATUS;
		cmd.arg = card->rca << 16;
		cmd.flags = MMC_RSP_R1 | MMC_CMD_AC;

		ret = mmc_wait_for_cmd(card->host, &cmd, 0);
		if (ret)
			break;

		if (!busy && !(cmd.resp[0] & R1_READY_FOR_DATA)) {
			busy = 1;
			printk(KERN_INFO "%s: Warning: Host did not "
				"wait for busy state to end.\n",
				mmc_hostname(card->host));
		}
	} while (!(cmd.resp[0] & R1_READY_FOR_DATA));

	return ret;
}
*/

static unsigned int snx_sd_block_rw(struct scatterlist *sg, unsigned int sg_len, unsigned long dev_addr, unsigned int blocks, int blksz, int write)
{

//	struct scatterlist sg;
//	unsigned int sg_len = 1;

//	int ret;

	struct mmc_request mrq;
	struct mmc_command cmd;
	struct mmc_command stop;
	struct mmc_data data;

//	int blocks = size / blksz;

//	blocks = (size > blocks*blksz) ? (blocks+1):blocks;

//	size = blocks * blksz;
	
//	sg_init_one(&sg, buf, size);

//	if (blocks > mmc->max_blk_count)
//		blocks = mmc->max_blk_count;
	
	memset(&mrq, 0, sizeof(struct mmc_request));
	memset(&cmd, 0, sizeof(struct mmc_command));
	memset(&data, 0, sizeof(struct mmc_data));
	memset(&stop, 0, sizeof(struct mmc_command));

	mrq.cmd = &cmd;
	mrq.data = &data;
	mrq.stop = &stop;

	if (blocks > 1) {
		cmd.opcode = write ?
			MMC_WRITE_MULTIPLE_BLOCK : MMC_READ_MULTIPLE_BLOCK;
	} else {
		cmd.opcode = write ?
			MMC_WRITE_BLOCK : MMC_READ_SINGLE_BLOCK;
	}

	cmd.arg = dev_addr;
	if (!mmc_card_blockaddr(mmc->card))
		cmd.arg <<= 9;

	cmd.flags = MMC_RSP_R1 | MMC_CMD_ADTC;

	if (blocks == 1)
		mrq.stop = NULL;
	else {
		stop.opcode = MMC_STOP_TRANSMISSION;
		stop.arg = 0;
		stop.flags = MMC_RSP_R1B | MMC_CMD_AC;
	}

	data.blksz = blksz;
	data.blocks = blocks;
	data.flags = write ? MMC_DATA_WRITE : MMC_DATA_READ;
	data.sg = sg;
	data.sg_len = sg_len;
	
	mmc_set_data_timeout(mrq.data, mmc->card);
	
	DBG_ME("blksz: %d blocks: %d sg->length: %d\n",data.blksz, data.blocks, data.sg->length); 
	mmc_wait_for_req(mmc, &mrq);
/*	
	ret = mmc_wait_busy(mmc->card);
	if (ret)
		return ret;
*/
	if (cmd.error)
		return cmd.error;
	if (data.error)
		return data.error;
	
	if(data.bytes_xfered != blocks * blksz)
	       return 1;
	
	return 0;
}

static unsigned int snx_sd_data_rw(u_char *buffer, unsigned long dev_addr, unsigned long len, int write)
{
        int ret;
	unsigned int size;
	unsigned int blocks;
	unsigned int blksz = 512;
	struct scatterlist sg;
	unsigned int sg_len = 1;
	unsigned long addr = dev_addr;
	u_char *buf = buffer;
//	int disable_multi = 0;

	unsigned int total_blk_count;
	unsigned int max_blk_count;
	
//	DBG_ME("snx_sd_data_rw: buff addr: 0x%x dev addr: %d len : %d\n",buf, addr, len);
	
	size = PAGE_SIZE * 2; 
	size = min(size, mmc->max_req_size);
	size = min(size, mmc->max_seg_size);
	size = min(size, mmc->max_blk_count * blksz);
	

	total_blk_count = (len + blksz -1) / blksz;
	max_blk_count = size / blksz;

	while (total_blk_count > 0) {
	
		blocks = min(total_blk_count, max_blk_count);
		size = blocks * blksz;
		
		DBG_ME("buff addr: 0x%x	dev addr: %x\n",buf, addr);
		sg_init_one(&sg, buf, size);

		do{
			ret = snx_sd_block_rw(&sg, sg_len, addr, blocks, blksz, write);
			if(ret)
				printk("block rw failed, retry...\n");
		}while(ret);

		buf += size;
		addr += blocks;
		total_blk_count -= blocks;
	}
	return 0;

}
size_t snx_sd_data_read(u_char *buffer, unsigned long dev_addr, size_t len)
{
	mmc_claim_host(mmc);

	snx_sd_data_rw(buffer, dev_addr, len, 0);
	
	mmc_release_host(mmc);
	
	return len;
}
size_t snx_sd_data_write(unsigned long dev_addr, const u_char *buffer, size_t len)
{
	mmc_claim_host(mmc);
	
	DBG_ME("snx_sd_data_write -> dev_addr: %x buffer: %x len: %d\n", dev_addr, buffer, len);
	snx_sd_data_rw ((u_char *)buffer, dev_addr, len, 1);
	
	mmc_release_host(mmc);

	return len;
}
