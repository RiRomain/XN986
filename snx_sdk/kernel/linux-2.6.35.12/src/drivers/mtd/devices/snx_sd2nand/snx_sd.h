/*
 *  linux/drivers/mmc/s3c2410mci.h - Samsung S3C2410 SDI Interface driver
 *
 *  Copyright (C) 2004 Thomas Kleffel, All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/mmc/card.h>
#include "../../linux-2.6.35.12/drivers/mmc/core/sd_ops.h"
#include "../../linux-2.6.35.12/drivers/mmc/core/mmc_ops.h"

 //timing_gong add, copy from kernel 2.6.18
#define MMC_ERR_NONE		0
#define MMC_ERR_TIMEOUT		1
//#define MMC_ERR_BADCRC	2
//#define MMC_ERR_FIFO		3
#define MMC_ERR_FAILED		4
//#define MMC_ERR_INVALID	5
//#define MMC_DATA_WRITE	(1 << 8)
//#define MMC_DATA_READ		(1 << 9)
//#define MMC_DATA_STREAM	(1 << 10)
#define MMC_DATA_MULTI		(1 << 11)

struct clk;

//#define SNXSDI_DMA 		0	//Nora: S3C2410SDI_DMA-->SNXSDI_DMA
#define SNX_SD_DMA              0

//#define SNXSDI_CDLATENCY	50	//Nora: S3C2410SDI_CDLATENCY-->SNXSDI_CDLATENCY
					//(delay time of mmc_detect_change)
#define SNX_SD_CDLATENCY        50

//Nora:	s3c2410sdi_waitfor-->sn926sdi_waitfor
//enum sn926sdi_waitfor {
enum snx_sd_waitfor {
	COMPLETION_NONE,
	COMPLETION_CMDSENT,
	COMPLETION_RSPFIN,
	COMPLETION_XFERFINISH,
	COMPLETION_XFERFINISH_RSPFIN,
};

typedef struct snx_mmc_platdata sn926_mmc_pdata_t;	//Nora: s3c24xx_mmc_platdata s3c24xx_mmc_pdata_t
							//-->sn926_mmc_platdata sn926_mmc_pdata_t

//Nora: s3c2410sdi_host-->sn926sdi_host
//struct sn926sdi_host {
struct snx_sd_host {
	struct mmc_host		*mmc;
	snx_mmc_pdata_t		*pdata;		//Nora: s3c24xx_mmc_pdata_t-->sn926_mmc_pdata_t

	struct resource		*mem;
	struct clk		*clk;
	void __iomem		*base;
	int			irq;
	int			irq_cd;
	int			dma;

	struct scatterlist*	cur_sg;		/* Current SG entry */
	unsigned int		num_sg;		/* Number of entries left */
	void*			mapped_sg;	/* vaddr of mapped sg */

	unsigned int		offset;		/* Offset into current entry */
	unsigned int		remain;		/* Data left in curren entry */

	int			size;		/* Total size of transfer */

	struct mmc_request	*mrq;

	unsigned char		bus_width;	/* Current bus width */

	spinlock_t		complete_lock;
	struct completion	complete_request;
	struct completion	complete_dma;
	enum snx_sd_waitfor	complete_what;	//Nora: s3c2410sdi_waitfor-->sn926sdi_waitfor

	long *pio_ptr;
	u32 pio_words;

//Nora: add (ref at91) --
	struct mmc_command *cmd;
        //struct mmc_request *request; //s3c2440:mrq, at91:request

	/*
        * Flag indicating when the command has been sent. This is used to
        * work out whether or not to send the stop
	*/
	unsigned int flags;
//--Nora: add (ref at91)

};

extern int snx_sd_init (struct device *dev,void __iomem *addr, int irq, uint64_t *size);
//extern struct mmc_host* snx_sd_alloc_host (struct platform_device *pdev);
extern size_t snx_sd_data_read (u_char *buffer, unsigned long dev_addr, size_t len);
extern size_t snx_sd_data_write (unsigned long dev_addr, const u_char *buffer, size_t len);
extern void snx_sd_interrupt_handler (struct mmc_host *mmc);
extern int snx_sd_exit (void);
