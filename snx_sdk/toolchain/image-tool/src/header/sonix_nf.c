#include "common.h"
#include "ms_nf.h"
#include "sonix_nf.h"

#define COL_ADDR_1(addr)  (addr & 0xFF)          /* 0  ~ 7,  8bits, A0  ~ A7  */
#define COL_ADDR_2(addr)  ((addr >> 8)  & 0x7)   /* 8  ~ 10, 3bits, A8  ~ A10 */
#define ROW_ADDR_1(addr)  ((addr >> 11) & 0xFF)  /* 11 ~ 18, 8bits, A12 ~ A19 */
#define ROW_ADDR_2(addr)  ((addr >> 19) & 0xFF)  /* 19 ~ 26, 8bits, A20 ~ A27 */

#define OP_COMPLETE  (0x1 << 6)  				 /* I/O[6] == 1 */
#define OP_SUCCESS   (0x1 << 0)  				 /* I/O[0] == 0 */

struct ms_nand_flash {
	u32 page_size;
	u32 block_size;
	u32 block_total;

	uchar id1th;
	uchar id2th;
	uchar id3th;
	uchar id4th;
	uchar id5th;
};

static struct ms_nand_flash flash = {2 * 1024, 64, 1024};

static void udelay(volatile u32 cnt) 
{
	while(cnt--);
}
							
static void nand_flash_init(struct ms_nand_flash *msnf)
{	
	ms1_set_mode(NF_MODE);	
	nf_set_AddrCyc(1);
	ms1_en_ExtraEcc(DISABLE);
	ms1_en_ecc(DISABLE);
	ms1_en_EccErrIntr(DISABLE);
	nf_set_rd_width(0x0);
	nf_set_wr_width(0x0);
	nf_set_LBAIncMode(0x0);

	/**
	 * write page size (K), 
	 * 1 page = 2K bytes
	 */
	nf_set_PageSize(0x1);
	
	/**
	 * write count of peges per block, 
	 * 1 block =  64 pages
	 */
	nf_set_BlkSize(0x2);
	
	nf_set_ReadCmdCnt(0);
	ms1_en_dma(DISABLE);			
	ms1_en_Mdma(DISABLE);

	clear_nf_PrgErr();
	clear_nf_InfoCRCErr();
	clear_nf_EraseErr();
	clear_ms1_CRCWErr();
	clear_ms1_EccErr();		
	clear_ms1_RdyIntr();
	clear_ms1_AhbErrIntr();
	clear_ms1_EccErrIntr();
}

static int nand_flash_check(struct ms_nand_flash *msnf)
{	
	u32 status;
	
	/**
	 * Read nand flash ID
	 * 1. Write command and address to NAND
	 */
	ms1_set_RegRW(WRITE_MODE);				
	ms1_write_cmd(0x90);					
	ms1_write_addr(0x00);

	/**
	 * 2. Read back data, include Maker Code,
	 *    Device Code, reserved Code, multiPlane Code
	 */
	ms1_set_RegRW(READ_MODE);
	msnf->id1th = ms1_read_data(0x00);
	msnf->id2th = ms1_read_data(0x00);
	msnf->id3th = ms1_read_data(0x00);
	msnf->id4th = ms1_read_data(0x00);
	msnf->id5th = ms1_read_data(0x00);

	/**
	 * 3. check read ID success
	 */
	ms1_set_RegRW(WRITE_MODE);
	ms1_write_cmd(0x70);
	ms1_set_RegRW(READ_MODE);
	status = ms1_read_data(0x00);

	if((msnf->id1th == 0xec) && (msnf->id2th == 0xf1))  
		return 0;

	serial_printf("nand_flash initial failed\n");
	return -1;
}

static int nand_flash_erase(u32 offset)
{								
	int status = 0;

	/**
	 * erase block
	 * 1. Write command and address to NAND
	 */
	ms1_set_RegRW(WRITE_MODE);
	ms1_write_cmd(0x60);
	ms1_write_addr(ROW_ADDR_1(offset));
	ms1_write_addr(ROW_ADDR_2(offset));;
	ms1_write_cmd (0xD0);		
	udelay(10000);
		
	while (!check_ms1_rdy());
		
	/**
	 * 2. Read Status, check complete
	 */
	do {						
		ms1_set_RegRW(WRITE_MODE);
		ms1_write_cmd(0x70);
		ms1_set_RegRW (READ_MODE);
		status = ms1_read_data(0x00);
			
	} while ((status & OP_COMPLETE) != OP_COMPLETE);

	/**
	 * 3. check erase success
	 */
	if(status & OP_SUCCESS)
		return -1;

	return 0;
}

static int nand_flash_write(u32 offset, uchar *buf, u32 len)
{
	int i;
	u32 status = 0;
	
	/**
	 * Page Program
	 * 1. Write command and address to NAND
	 */
	ms1_set_RegRW(WRITE_MODE);
	ms1_write_cmd(0x80);
	ms1_write_addr(COL_ADDR_1(offset));
	ms1_write_addr(COL_ADDR_2(offset));
	ms1_write_addr(ROW_ADDR_1(offset));
	ms1_write_addr(ROW_ADDR_2(offset));
	udelay (1000);

	/**
	 * 2. write data
	 */
	for (i = 0; i < len; i++)
		ms1_write_data(buf[i]);

	/**
	 * 3. write command2 for page program
	 */
	ms1_write_cmd(0x10);
	udelay(1000);
	
	while (!check_ms1_rdy());
	udelay(100000);

	/**
	 * 4. check status, wait complete
	 */
	do {
		ms1_set_RegRW(WRITE_MODE);
		ms1_write_cmd(0x70);
		ms1_set_RegRW(READ_MODE);
		status = ms1_read_data(0x00);
	} while ((status & OP_COMPLETE) != OP_COMPLETE);

	/**
	 * 5. check write success
	 */
	 if(status & OP_SUCCESS)
	 	return -1;
	
	return 0;
}

#if 0  // Hardware DMA BUG
static int nand_flash_dma_write(u32 offset, uchar *buf, u32 len)
{
	u32 status = 0;

	ms1_en_ExtraEcc(ENABLE);
	ms1_en_ecc(ENABLE);
	
	/**
	 * Page Program
	 * 1. Write command and address to NAND
	 */
	ms1_set_RegRW(WRITE_MODE);
	ms1_write_cmd(0x80);
	ms1_write_addr(COL_ADDR_1(offset));
	ms1_write_addr(COL_ADDR_2(offset));
	ms1_write_addr(ROW_ADDR_1(offset));
	ms1_write_addr(ROW_ADDR_2(offset));

	udelay (1000);
	while (!check_ms1_rdy());

	ms1_set_DmaRW(WRITE_MODE);
	ms1_set_DmaSize(len -1);
	ms1_set_DmaAddr((u32)buf);
	ms1_set_DmaBlk(0);

	// Enable DMA
	ms1_en_dma (ENABLE);
	udelay (1000);
	while (!check_ms1_rdy());
		
	// Disable DMA
	ms1_en_dma (DISABLE);

	/**
	 * 3. write command2 for page program
	 */			
	ms1_write_cmd(0x10);	
	udelay (1000);
	while (!check_ms1_rdy());
	udelay (10000);

	/**
	 * 4. check status, wait complete
	 */
	do {
		ms1_set_RegRW(WRITE_MODE);
		ms1_write_cmd(0x70);
		ms1_set_RegRW(READ_MODE);
		status = ms1_read_data(0x00);
	} while ((status & OP_COMPLETE) != OP_COMPLETE);

	ms1_en_ExtraEcc(DISABLE);
	ms1_en_ecc(DISABLE);
	
	/**
	 * 5. check write success
	 */
	 if(status & OP_SUCCESS)
	 	return -1;
	
	return 0;
}
#endif 

/**************************************************
 ** API
 **************************************************/
int ms_nand_flash_init(void)
{
	 nand_flash_init(&flash);
	 return nand_flash_check(&flash);
}

int ms_nand_flash_erase(u32 offset, size_t len)
{
	u32 top_addr;
	u32 block_size;
	
	block_size = flash.page_size * flash.block_size;

	serial_printf("Erase: offset 0x%08x, len 0x%08x\n", offset, len); 

	if((offset + len) > (flash.block_total * block_size)) {
		serial_printf("address out of flash size\n");
		return -EINVAL;
	}
		
	if((offset % block_size) || (len % block_size)) {
		serial_printf("offset/len not multiple of block size\n");
		return -EINVAL;
	}

	top_addr =  offset + len;

	serial_printf("Erase:");
	for(; offset < top_addr; offset += block_size) {
		if(nand_flash_erase(offset)) {
			//return -EFAULT;
			serial_printf("Erase Failed: offset = 0x%08x len = 0x%08x\n",
				offset, len);
		}
		serial_printf(".");
	}
	
	serial_printf("\n");
	
	return 0;
}


int ms_nand_flash_write(u32 offset, uchar *buf, size_t len)
{
	int retval;
	u32 bytes;
	u32 top_addr;
	u32 actual_len;

	serial_printf("Write: offset 0x%08x, buf = 0x%08x , len 0x%08x\n", 
		offset, buf, len);
	
	if((offset + len) > (flash.block_total * flash.block_size 
		* flash.page_size)) {
		serial_printf("address out of flash size\n");
		return -EINVAL;
	}

	bytes = offset % flash.page_size;
	top_addr = offset + len;

	serial_printf("Write:");
	
	for(; offset < top_addr; offset += actual_len) {
		actual_len = min(top_addr - offset, 
			flash.page_size - bytes);

		retval = nand_flash_write(offset, buf, actual_len);
		if(retval)
			return -EFAULT;

		serial_printf(".");
		
		bytes =0;
		buf += actual_len;
	}

	serial_printf("\n");

	return 0;
}

#if 0 // Hardware DMA BUG
int ms_nand_flash_erase_write_skip_bad(u32 offset, uchar *buf, size_t len)
{
	int retval;
	u32 bytes;
	u32 actual_len;
	u32 erase_block;
	u32 block_size;

	serial_printf("Write: offset 0x%08x, buf = 0x%08x , len 0x%08x", 
		offset, buf, len);
	
	if((offset + len) > (flash.block_total * flash.block_size 
		* flash.page_size)) {
		serial_printf("address out of flash size\n");
		return -EINVAL;
	}

	block_size = flash.block_size * flash.page_size;
    erase_block = offset - offset % block_size;
	
	bytes = offset % flash.page_size;
	
	for(; len > 0; len -= actual_len) {
		/**
		 * erase block
		 */
		if(offset >= erase_block) {
			serial_printf("\nErase and write block 0x%08x: ", erase_block);
			while(nand_flash_erase(erase_block)) {
				/**
				 * if erase failed, this block is bad and skiping 
				 */
				serial_printf("Bad block: 0x%08x, skip\n",erase_block);
				offset += block_size;
				erase_block += block_size;
			}
#if 0
			serial_printf("Write: offset 0x%08x, buf = 0x%08x , len 0x%08x 0x%08x\n",  
			 	offset, buf, actual_len, len);
#endif 
			erase_block += block_size;
		}

		/**
		 *  write data
		 */
		 
		actual_len = min(len, flash.page_size - bytes);
		retval = nand_flash_dma_write(offset, buf, actual_len);
		if(retval)
			return -EFAULT;

		serial_printf(".");
		
		bytes =0;
		buf += actual_len;
		offset += actual_len;
	}

	serial_printf("\n");

	return 0;
}
#endif



int ms_nand_flash_update(void) 
{
	int i = 0;
	u32 page_bytes, block_bytes, chip_bytes;
	
	page_bytes  = flash.page_size;
	block_bytes = flash.block_size * page_bytes;
	chip_bytes  = flash.block_total * block_bytes;

	if(FLASH_INFO_LEN > 0 ) {
		if(ms_nand_flash_erase(0x0, block_bytes))
			return -1;


		for(i = 0; i < 10; i++) {
			serial_printf("write flash_info %d\n", i);
			if(ms_nand_flash_write(0x0 + page_bytes * i, 
				(uchar *)FLASH_INFO_START, FLASH_INFO_LEN))
				return -1;
		}
	}
	
	if(HW_SETTING_LEN > 0) {
		#if 0
		serial_printf("write HW_setting 1\n");
		if(ms_nand_flash_write(block_bytes / 2, (uchar *)HW_SETTING_START, 
			HW_SETTING_LEN))
			return -1;
	
		serial_printf("write HW_setting 2\n");
		if(ms_nand_flash_erase(block_bytes * 7, block_bytes))
			return -1;
		if(ms_nand_flash_write(block_bytes * 7, (uchar *)HW_SETTING_START, 
			HW_SETTING_LEN))
			return -1;

		serial_printf("write HW_setting 3\n");
		if(ms_nand_flash_erase(block_bytes * (flash.block_total - 3 - 8), 
			block_bytes))
			return -1;
		if(ms_nand_flash_write(block_bytes * (flash.block_total - 3 - 8), 
			(uchar *)HW_SETTING_START, HW_SETTING_LEN))
			return -1;
		#else
		serial_printf("write HW_setting 1\n");
		if(ms_nand_flash_write(NAND_HW_SETTING_1_STR, (uchar *)HW_SETTING_START, 
			HW_SETTING_LEN))
			return -1;
	
#if 1	
		serial_printf("write HW_setting 2\n");
		if(ms_nand_flash_erase(NAND_HW_SETTING_2_STR, (NAND_U_BOOT_2_STR - NAND_HW_SETTING_2_STR)))
			return -1;
		if(ms_nand_flash_write(NAND_HW_SETTING_2_STR, (uchar *)HW_SETTING_START, 
			HW_SETTING_LEN))
			return -1;

		serial_printf("write HW_setting 3\n");
		if(ms_nand_flash_erase(NAND_HW_SETTING_3_STR, (NAND_U_BOOT_3_STR- NAND_HW_SETTING_3_STR)))
			return -1;
		if(ms_nand_flash_write(NAND_HW_SETTING_3_STR, 
			(uchar *)HW_SETTING_START, HW_SETTING_LEN))
			return -1;	
		#endif	
#endif
	}

	if(U_BOOT_LEN > 0) {
		serial_printf("write u-boot 1\n");
		if ((NAND_RESERVE_1_STR - NAND_U_BOOT_1_STR) != 0) {
		if(ms_nand_flash_erase(NAND_U_BOOT_1_STR, (NAND_RESERVE_1_STR - NAND_U_BOOT_1_STR)))
			return -1;
		if(ms_nand_flash_write(NAND_U_BOOT_1_STR, (uchar *)U_BOOT_START, 
			U_BOOT_LEN))
			return -1;
		}
#if 1
		serial_printf("write u-boot 2\n");
		if ((NAND_RESERVE_2_STR- NAND_U_BOOT_2_STR) != 0){
		if(ms_nand_flash_erase(NAND_U_BOOT_2_STR, (NAND_RESERVE_2_STR- NAND_U_BOOT_2_STR)))
			return -1;
		if(ms_nand_flash_write(NAND_U_BOOT_2_STR,(uchar *) U_BOOT_START, 
			U_BOOT_LEN))
			return -1;
		}

		serial_printf("write u-boot 3\n");
		if ((NAND_RESERVE_3_STR - NAND_U_BOOT_3_STR) != 0) {
		if(ms_nand_flash_erase(NAND_U_BOOT_3_STR, (NAND_RESERVE_3_STR - NAND_U_BOOT_3_STR)))
			return -1;
		if(ms_nand_flash_write(NAND_U_BOOT_3_STR, 
			(uchar *)U_BOOT_START, U_BOOT_LEN))
			return -1;
		}
#endif
	}

	#if 0
	if(U_ENV_LEN > 0) {
		serial_printf("write u-env 1\n");
		if(ms_nand_flash_erase(NAND_U_ENV_1_STR, (NAND_FLASH_LAYOUT_1_STR - NAND_U_ENV_1_STR)))
			return -1;
		if(ms_nand_flash_write(NAND_U_ENV_1_STR, (uchar *)U_ENV_START, 
			U_ENV_LEN))
			return -1;

		serial_printf("write u-env 2\n");
		if(ms_nand_flash_erase(NAND_U_ENV_2_STR, (NAND_FLASH_LAYOUT_2_STR - NAND_U_ENV_2_STR)))
			return -1;
		if(ms_nand_flash_write(NAND_U_ENV_2_STR, (uchar *)U_ENV_START, 
			U_ENV_LEN))
			return -1;

		serial_printf("write u-env 3\n");
		if(ms_nand_flash_erase(NAND_U_ENV_3_STR, (NAND_FLASH_LAYOUT_3_STR - NAND_U_ENV_3_STR)))
			return -1;
		if(ms_nand_flash_write(NAND_U_ENV_3_STR, (uchar *)U_ENV_START, 
			U_ENV_LEN))
			return -1;
	}
	#else
	serial_printf("erase u-env 1\n");
		if(ms_nand_flash_erase(NAND_U_ENV_1_STR, (NAND_FLASH_LAYOUT_1_STR - NAND_U_ENV_1_STR)))
			return -1;

	serial_printf("erase u-env 2\n");
		if(ms_nand_flash_erase(NAND_U_ENV_2_STR, (NAND_FLASH_LAYOUT_2_STR - NAND_U_ENV_2_STR)))
			return -1;	

	serial_printf("erase u-env 3\n");
		if(ms_nand_flash_erase(NAND_U_ENV_3_STR, (NAND_FLASH_LAYOUT_3_STR - NAND_U_ENV_3_STR)))
			return -1;	
	#endif

	if(FLASH_LAYOUT_LEN > 0) {
	
		ms1_info_disable();

		serial_printf("write flash-layout 1\n");
		if ((NAND_HW_SETTING_2_STR - NAND_FLASH_LAYOUT_1_STR) != 0) {
		if(ms_nand_flash_erase(NAND_FLASH_LAYOUT_1_STR, (NAND_HW_SETTING_2_STR - NAND_FLASH_LAYOUT_1_STR)))
			return -1;
		if(ms_nand_flash_write(NAND_FLASH_LAYOUT_1_STR, (uchar *)FLASH_LAYOUT_START, 
			FLASH_LAYOUT_LEN))
			return -1;
		}

		serial_printf("write flash-layout 2\n");
		if ((NAND_FACTORY_1_STR - NAND_FLASH_LAYOUT_2_STR) != 0) {
		if(ms_nand_flash_erase(NAND_FLASH_LAYOUT_2_STR, (NAND_FACTORY_1_STR - NAND_FLASH_LAYOUT_2_STR)))
			return -1;
		if(ms_nand_flash_write(NAND_FLASH_LAYOUT_2_STR, (uchar *)FLASH_LAYOUT_START, 
			FLASH_LAYOUT_LEN))
			return -1;
		}

		serial_printf("write flash-layout 3\n");
		if ((NAND_BBT_STR - NAND_FLASH_LAYOUT_3_STR) != 0) {
		if(ms_nand_flash_erase(NAND_FLASH_LAYOUT_3_STR, (NAND_BBT_STR - NAND_FLASH_LAYOUT_3_STR)))
			return -1;
		if(ms_nand_flash_write(NAND_FLASH_LAYOUT_3_STR, (uchar *)FLASH_LAYOUT_START, 
			FLASH_LAYOUT_LEN))
			return -1;
		}
	}

#if 0
	if(FACTORY_LEN > 0) {
		serial_printf("write factory 1\n");
		if(ms_nand_flash_erase(NAND_FACTORY_1_STR, (NAND_KERNEL_STR - NAND_FACTORY_1_STR)))
			return -1;
		if(ms_nand_flash_write(NAND_FACTORY_1_STR, (uchar *)FACTORY_START, 
			FACTORY_LEN))
			return -1;

		serial_printf("write factory 2\n");
		if(ms_nand_flash_erase(NAND_FACTORY_2_STR, (NAND_USER_3_STR - NAND_FACTORY_2_STR)))
			return -1;
		if(ms_nand_flash_write(NAND_FACTORY_2_STR, (uchar *)FACTORY_START, 
			FACTORY_LEN))
			return -1;

		serial_printf("write factory 3\n");
		if(ms_nand_flash_erase(NAND_FACTORY_3_STR, (NAND_HW_SETTING_3_STR - NAND_FACTORY_3_STR)))
			return -1;
		if(ms_nand_flash_write(NAND_FACTORY_3_STR, (uchar *)FACTORY_START, 
			FACTORY_LEN))
			return -1;
	}


	if(U_LOGO_LEN > 0) {
		serial_printf("write logo\n");
		if(ms_nand_flash_erase(NAND_U_LOGO_STR, (NAND_RESCUE_STR - NAND_U_LOGO_STR)))
			return -1;
		if(ms_nand_flash_write(NAND_U_LOGO_STR, (uchar *)U_LOGO_START, 
			U_LOGO_LEN))
			return -1;
	}
#endif

#if 0
	if(RESCUE_LEN > 0) {
		serial_printf("write rescue \n");
		if(ms_nand_flash_erase(NAND_RESCUE_STR, (NAND_USER_2_STR - NAND_RESCUE_STR)))
			return -1;
		if(ms_nand_flash_write(NAND_RESCUE_STR, (uchar *)RESCUE_START, 
			RESCUE_LEN))
			return -1;
	}
#endif 

	
	return 0;
}
