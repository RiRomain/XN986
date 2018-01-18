/*
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <mach/regs-gpio.h>
#include <mach/regs-ms.h>

#if 0
#define MS_DEBUG
#endif

#ifdef CONFIG_MS_DEBUG
//#ifdef MS_DEBUG
#define DBG(x...)       printk(KERN_INFO x)
#else
#define DBG(x...)       do { } while (0)
#endif

/*CTL APIs : 0x00*/
void ms_set_msmode(void __iomem *base, unsigned int mode)
{
	/* 
	00: GPIO mode
	01: SD card SPI mode
	10: SD card SD mode
	Others : Reserve
	*/

	unsigned int data;
        DBG(PFX "ms_set_msmode:	---->\n");
	data = *((volatile unsigned *)(base + MS_CTL_OFFSET));
	data &= ~(MS_CTL_MODE); 
	*((volatile unsigned *)(base + MS_CTL_OFFSET)) = (data|mode);
	DBG(PFX "ms_set_msmode:	write to base + MS_CTL_OFFSET = %x\n", (data|mode));
	
}
EXPORT_SYMBOL_GPL(ms_set_msmode);

void ms_msreg_rw_switch(void __iomem *base, unsigned int value)
{
        /*[value] 1: read mode , 0: write mode*/ 
        unsigned int data;
        DBG(PFX "ms_msreg_rw_switch: ---->\n");
        data = *((volatile unsigned *)(base + MS_CTL_OFFSET));
        data &= ~(MS_CTL_REGRW);
        data |= (value<<3);
        *((volatile unsigned *)(base + MS_CTL_OFFSET)) = data;
        DBG(PFX "ms_msreg_rw_switch: write to base + MS_CTL_OFFSET = %x\n", data);
}
EXPORT_SYMBOL_GPL(ms_msreg_rw_switch);

void ms_msdma_en_switch(void __iomem *base, unsigned int isEnable)
{
        /*[isEnable] 1: enable DMA , 0: disable DMA*/
	unsigned int data;
        DBG(PFX "ms_msdma_en_switch: ---->\n");
	data = *((volatile unsigned *)(base + MS_CTL_OFFSET));
	data &= ~(MS_CTL_DMAEN);
	data |= (isEnable <<4);
	*((volatile unsigned *)(base + MS_CTL_OFFSET)) = data;	
	DBG(PFX "ms_msdma_en_switch: write to base + MS_CTL_OFFSET = %x\n", data);
}
EXPORT_SYMBOL_GPL(ms_msdma_en_switch);

void ms_msdma_rw_switch(void __iomem *base, unsigned int value)
{
        /*[value] 1: read mode , 0: write mode*/
	unsigned int data;
	DBG(PFX "ms_msdma_rw_switch: ---->\n");
	data = *((volatile unsigned *)(base + MS_CTL_OFFSET));
	data &= ~(MS_CTL_DMARW);
	data |= (value <<5);
	*((volatile unsigned *)(base + MS_CTL_OFFSET)) = data;	
	DBG(PFX "ms_msdma_rw_switch: write to base + MS_CTL_OFFSET = %x\n", data);
}
EXPORT_SYMBOL_GPL(ms_msdma_rw_switch);

void ms_extra_en_switch(void __iomem *base, unsigned int isEnable)
{
        /*[isEnable] 1: enable Extra ECC DATA enable , 0: disable*/
	unsigned int data;
	DBG(PFX "ms_extra_en_switch: ---->\n");
	data = *((volatile unsigned *)(base + MS_CTL_OFFSET));
	data &= ~(MS_CTL_EXTRAEN);
	data |= (isEnable <<6);
	*((volatile unsigned *)(base + MS_CTL_OFFSET)) = data;	
	DBG(PFX "ms_extra_en_switch: write to base + MS_CTL_OFFSET = %x\n", data);
}
EXPORT_SYMBOL_GPL(ms_extra_en_switch);

void ms_ecc_en_switch(void __iomem *base, unsigned int isEnable)
{
        /*[isEnable] 1: enable ECC or CRC, 0: disable*/
        unsigned int data;
        DBG(PFX "ms_ecc_en_switch: ---->\n");
	data = *((volatile unsigned *)(base + MS_CTL_OFFSET));
	data &= ~(MS_CTL_ECCEN);
	data |= (isEnable <<7);
	*((volatile unsigned *)(base + MS_CTL_OFFSET)) = data;	
	DBG(PFX "ms_ecc_en_switch: write to base + MS_CTL_OFFSET = %x\n", data);
}
EXPORT_SYMBOL_GPL(ms_ecc_en_switch);

int ms_check_msrdy(void __iomem *base)
{
        /*return 1: ready*/
	int data;
	//DBG(PFX "ms_check_msrdy:	---->\n");
	data = *((volatile unsigned *)(base + MS_CTL_OFFSET));
	data &= MS_CTL_MSRDY;
	data = (data>> 8);
	//DBG(PFX "ms_check_msrdy:	return value = %x\n", data);
	return data;
}
EXPORT_SYMBOL_GPL(ms_check_msrdy);

void ms_set_spibusy_tri(void __iomem *base, unsigned int isEnable)
{
        /*[isEnable] 1:trigger*/
	unsigned int data;
	DBG(PFX "ms_set_spibusy_tri:	---->\n");
	data = *((volatile unsigned *)(base + MS_CTL_OFFSET));
	data &= ~(MS_CTL_SPIBUSYTRI);
	data |= (isEnable <<12);	
	*((volatile unsigned *)(base + MS_CTL_OFFSET)) = data;	
	DBG(PFX "ms_set_spibusy_tri:	write to base + MS_CTL_OFFSET = %x\n", data);	
}
EXPORT_SYMBOL_GPL(ms_set_spibusy_tri);

void ms_set_spicmd_tri(void __iomem *base, unsigned int isEnable)
{
        /*[isEnable] 1:trigger*/
	unsigned int data;
	DBG(PFX "ms_set_spicmd_tri:        ---->\n");
	data = *((volatile unsigned *)(base + MS_CTL_OFFSET));
	data &= ~(MS_CTL_SPICMDTRI);
	data |= (isEnable <<13);
	*((volatile unsigned *)(base + MS_CTL_OFFSET)) = data;	
	DBG(PFX "ms_set_spicmd_tri:	write to base + MS_CTL_OFFSET = %x\n", data);
}
EXPORT_SYMBOL_GPL(ms_set_spicmd_tri);

void ms_read_data_cmd(void __iomem *base, unsigned int isEnable)
{
        /*[isEnable] 1: sd command is a read data command*/
        unsigned int data;
        DBG(PFX "ms_read_data_cmd:	---->\n");
        data = *((volatile unsigned *)(base + MS_CTL_OFFSET));
        data &= ~(MS_CTL_RAEDDATACMD);
        data |= (isEnable <<22);
        //data = 0xfff001e5;
        *((volatile unsigned *)(base + MS_CTL_OFFSET)) = data; 
        DBG(PFX "ms_read_data_cmd:        write to base + MS_CTL_OFFSET = %x\n", data);
}
EXPORT_SYMBOL_GPL(ms_read_data_cmd);
                                                
void ms_set_msspeed(void __iomem *base, unsigned int msspeed)
{
	unsigned int data;
	DBG(PFX "ms_set_msspeed:      ---->\n");
	data = *((volatile unsigned *)(base + MS_CTL_OFFSET));
	data &= ~(MS_CTL_MSSPEED);
	data |= (msspeed <<24);	
	*((volatile unsigned *)(base + MS_CTL_OFFSET)) = data;	
        DBG(PFX "ms_set_msspeed:	write to base + MS_CTL_OFFSET = %x msspeed = %x\n", data, msspeed);
}
EXPORT_SYMBOL_GPL(ms_set_msspeed);

/*DMA_SIZE APIs : 0x04*/
void ms_set_dmasize(void __iomem *base, unsigned int size)
{
	unsigned int data;
	DBG(PFX "ms_set_dmasize:      ---->\n");
	data = *((volatile unsigned *)(base + MS_DMA_SIZE_OFFSET));
	data &= ~(MS_DMA_SIZE_MSDMASIZE);	
	data |= (size <<0);
	*((volatile unsigned *)(base + MS_DMA_SIZE_OFFSET)) = data;	
	DBG(PFX "ms_set_dmasize:	write to MS_DMA_SIZE = %x\n", data);
}
EXPORT_SYMBOL_GPL(ms_set_dmasize);

/*CRC1 APIs : 0x08*/
unsigned int ms_read_crc16_0(void __iomem *base)
{
        unsigned int data;
        DBG(PFX "ms_read_crc16_0:      ---->\n");
        data = *((volatile unsigned *)(base + MS_CRC1_OFFSET));
        data &= (MS_CRC1_CRC16_0_L | MS_CRC1_CRC16_0_H);
        data = (data>>0);
        DBG(PFX "ms_read_crc16_0:      return value = %x\n", data);
        return data;
}
EXPORT_SYMBOL_GPL(ms_read_crc16_0);

unsigned int ms_read_crc7(void __iomem *base)
{
        unsigned int data;
        DBG(PFX "ms_read_crc7:      ---->\n");
        data = *((volatile unsigned *)(base + MS_CRC1_OFFSET));
        data &= MS_CRC1_CRC7;
        data = (data>> 16);  
        DBG(PFX "ms_read_crc7:      return value = %x\n", data);
        return data;
}
EXPORT_SYMBOL_GPL(ms_read_crc7);

/*CRC2 APIs : 0x0c*/
unsigned int ms_read_crc16_1(void __iomem *base)
{
        unsigned int data;
        DBG(PFX "ms_read_crc16_1:      ---->\n");
        data = *((volatile unsigned *)(base + MS_CRC2_OFFSET));
        data &= MS_CRC2_CRC16_1;
        data = (data>>0);
        DBG(PFX "ms_read_crc16_1:      return value = %x\n", data);
        return data;
}
EXPORT_SYMBOL_GPL(ms_read_crc16_1);
                                         
/*CRC3 APIs : 0x10*/
unsigned int ms_read_crc16_2and16_3(void __iomem *base)
{
        unsigned int data;
        DBG(PFX "ms_read_crc16_2and16_3:      ---->\n");
        data = *((volatile unsigned *)(base + MS_CRC3_OFFSET));
        data &= (MS_CRC2_CRC16_3|MS_CRC2_CRC16_2);
        data = (data>> 0);
        DBG(PFX "ms_read_crc16_2and16_3:      return value = %x\n", data);
        return data;
}
EXPORT_SYMBOL_GPL(ms_read_crc16_2and16_3);
                                        
/*MS_IO_I APIs : 0x30*/
unsigned int ms_ms_io_i(void __iomem *base)
{
	unsigned int data;
	DBG(PFX "ms_ms_io_i:      ---->\n");
	data = *((volatile unsigned *)(base + MS_MS_IO_I_OFFSET));
        data &= (MS_MS_IO_I_I);
        data = (data >> 0);
        DBG(PFX "ms_ms_io_i:      return value = %x\n", data);
        return data;
}
EXPORT_SYMBOL_GPL(ms_ms_io_i);

/*MS_IO_O APIs : 0x34*/
//set MS_IO8
void ms_ms_io_o_8(void __iomem *base, unsigned int value)
{
        unsigned int data;
        DBG(PFX "ms_ms_io_o_8:      ---->\n");
        data = *((volatile unsigned *)(base + MS_MS_IO_O_OFFSET));
        data &= ~(MS_MS_IO_O_O);
        //data |= (value << 8);
        data |= (value << 0);
        *((volatile unsigned *)(base + MS_MS_IO_O_OFFSET)) = data;
        DBG(PFX "ms_ms_io_o_8:      write to MS_MS_IO_O = %x\n", data);
}
EXPORT_SYMBOL_GPL(ms_ms_io_o_8);

/*MS_IO_OE APIs : 0x38*/
//set MS_IO8
void ms_ms_io_oe_8(void __iomem *base, unsigned int value)
{
        unsigned int data;
        DBG(PFX "ms_ms_io_oe_8:      ---->\n");
        data = *((volatile unsigned *)(base + MS_MS_IO_OE_OFFSET));
        data &= ~(MS_MS_IO_OE_OE);
        //data |= (value << 8);
        data |= (value <<0);
        *((volatile unsigned *)(base + MS_MS_IO_OE_OFFSET)) = data; 
        DBG(PFX "ms_ms_io_oe_8:      write to MS_MS_IO_OE = %x\n", data);
}
EXPORT_SYMBOL_GPL(ms_ms_io_oe_8);
        

/*SPI_CMD APIs : 0x3c*/
void ms_write_spi_cmd(void __iomem *base, unsigned int cmd)
{
        DBG(PFX "ms_write_spi_cmd:      ---->\n");
	*((volatile unsigned *)(base + MS_SPI_CMD_OFFSET)) = cmd;
	DBG(PFX "ms_write_spi_cmd:      write to MS_SPI_CMD = %x\n", cmd);
}
EXPORT_SYMBOL_GPL(ms_write_spi_cmd);

unsigned int ms_read_spi_cmd(void __iomem *base)
{
        unsigned int data;
        DBG(PFX "ms_read_spi_cmd:      ---->\n");
        data = *((volatile unsigned *)(base + MS_SPI_CMD_OFFSET));
        DBG(PFX "ms_read_spi_cmd:      return value = %x\n", data);
        return data;
}                 
EXPORT_SYMBOL_GPL(ms_read_spi_cmd);

/*SD SPI_INDEX APIs ; 0x40*/
void ms_write_spi_index(void __iomem *base, unsigned int index)
{
 	unsigned int data;
 	DBG(PFX "ms_write_spi_index:      ---->\n");
 	data = *((volatile unsigned *)(base + MS_SPI_INDEX_OFFSET));
        data &= ~(MS_SPI_INDEX_INDEX);
        data |= (index << 0);
        *((volatile unsigned *)(base + MS_SPI_INDEX_OFFSET)) = data;
        DBG(PFX "ms_write_spi_index:      write to MS_SPI_INDEX = %x\n", data);
}
EXPORT_SYMBOL_GPL(ms_write_spi_index);
                                          
unsigned int ms_read_spi_index(void __iomem *base)
{
        unsigned int data;
        DBG(PFX "ms_read_spi_index:      ---->\n");
        data = *((volatile unsigned *)(base + MS_SPI_INDEX_OFFSET));
        data &= (MS_SPI_INDEX_INDEX);
        data = (data>>0);
        DBG(PFX "ms_read_spi_index:      return value=%x\n", data);
        return data;
}
EXPORT_SYMBOL_GPL(ms_read_spi_index);

/*DMA_BLKSU APIs : 0x48*/
void ms_dmablock(void __iomem *base, unsigned int dmaBlockNum)
{
	unsigned int data;
	DBG(PFX "ms_dmablock:	---->\n");
	data = *((volatile unsigned *)(base + MS_DMA_BLKSU_OFFSET));
	data &= ~(MS_DMA_BLKSU_BLK);
	data |= (dmaBlockNum <<0);
	*((volatile unsigned *)(base + MS_DMA_BLKSU_OFFSET)) = data;	
	DBG(PFX "ms_dmablock:      write to MS_DMA_BLKSU = %x\n", data);
}
EXPORT_SYMBOL_GPL(ms_dmablock);

unsigned int ms_read_sudmablock(void __iomem *base)
{
	unsigned int data;
	DBG(PFX "ms_read_sudmablock:   ---->\n");
	data = *((volatile unsigned *)(base + MS_DMA_BLKSU_OFFSET));
	data &= MS_DMA_BLKSU_SUBLK;
	data = (data>> 16);
	DBG(PFX "ms_read_sudmablock:   return data = %x\n", data);
	return data;
}
EXPORT_SYMBOL_GPL(ms_read_sudmablock);

/*TMCNT APIs : 0x4c*/
void ms_set_timecount(void __iomem *base, unsigned int timeCnt)
{
	unsigned int data;
	DBG(PFX "ms_set_timecount:   ---->\n");
	data = *((volatile unsigned *)(base + MS_TMCNT_OFFSET));		
	data &= ~(MS_TMCNT_TMCNT);								 
	data |= (timeCnt <<0); 
	*((volatile unsigned *)(base + MS_TMCNT_OFFSET)) = data;	 
	DBG(PFX "ms_set_timecount:      write to MS_TMCNT = %x\n", data);
	
}
EXPORT_SYMBOL_GPL(ms_set_timecount);

/*MDMAECC APIs : 0x50*/
void ms_msmdma_en_switch(void __iomem *base, unsigned int isEnable)
{
	unsigned int data;
	DBG(PFX "ms_msmdma_en_switch:   ---->\n");
	data = *((volatile unsigned *)(base + MS_MDMAECC_OFFSET));
	data &= ~(MS_MDMAECC_MS_M_DMA_EN);
	data |= (isEnable >>0);
	*((volatile unsigned *)(base + MS_MDMAECC_OFFSET)) = data;	
	DBG(PFX "ms_msmdma_en_switch:	write to base + MS_MDMAECC_OFFSET = %x\n", data);
}
EXPORT_SYMBOL_GPL(ms_msmdma_en_switch);

int ms_check_msmdma_ok(void __iomem *base)
{
	int data;
	DBG(PFX "ms_check_msmdma_ok:   ---->\n");
	data = *((volatile unsigned *)(base + MS_MDMAECC_OFFSET));
	data &=MS_MDMAECC_MS_M_DMA_OK;
	data =(data>>1);
	DBG(PFX "ms_check_msmdma_ok:   return data=%x\n", data);
	return data;	
}
EXPORT_SYMBOL_GPL(ms_check_msmdma_ok);

int ms_check_msmdma_timeout(void __iomem *base)
{
	int data;
	DBG(PFX "ms_check_msmdma_timeout:   ---->\n");
	data = *((volatile unsigned *)(base + MS_MDMAECC_OFFSET));
	data &=MS_MDMAECC_MS_M_DMA_TIME_OUT;
	data =(data>>2);
	DBG(PFX "ms_check_msmdma_timeout:   return data=%x\n", data);
	return data;	
}
EXPORT_SYMBOL_GPL(ms_check_msmdma_timeout);

int ms_check_crc_err(void __iomem *base)
{
        int data;
        DBG(PFX "ms_check_crc_err:   ---->\n");
        data = *((volatile unsigned *)(base + MS_MDMAECC_OFFSET));
        data &=MS_MDMAECC_CRCERR;
//	 data =(data>>8);   
        data =(data>>7);                         //modified by zbl;
        DBG(PFX "ms_check_crc_err:   return data=%x\n", data);
        return data;    
}
EXPORT_SYMBOL_GPL(ms_check_crc_err);

int ms_check_crc_fail(void __iomem *base)
{
        int data;
        DBG(PFX "ms_check_crc_fail:   ---->\n");
        data = *((volatile unsigned *)(base + MS_MDMAECC_OFFSET));
        data &=MS_MDMAECC_CRCFAIL;
        data =(data>>16);
        DBG(PFX "ms_check_crc_fail:   return data=%x\n", data);
        return data;     
}
EXPORT_SYMBOL_GPL(ms_check_crc_fail);
                                        
void ms_msrdy_Intr_en_switch(void __iomem *base, unsigned int isEnable)
{
	unsigned int data;
	DBG(PFX "ms_msrdy_Intr_en_switch:   ---->\n");
	data = *((volatile unsigned *)(base + MS_MDMAECC_OFFSET));
	data &= ~(MS_MDMAECC_MS_RDY_INTR_EN);
//	data |= (isEnable <<24);
       data |= (isEnable <<8);    //modified by zbl
	*((volatile unsigned *)(base + MS_MDMAECC_OFFSET)) = data;	
	DBG(PFX "ms_msrdy_Intr_en_switch:	write to base + MS_MDMAECC_OFFSET = %x\n", data);
}
EXPORT_SYMBOL_GPL(ms_msrdy_Intr_en_switch);

int ms_check_msrdy_flag(void __iomem *base)
{
	int data;
	DBG(PFX "ms_check_msrdy_flag:   ---->\n");
	data = *((volatile unsigned *)(base + MS_MDMAECC_OFFSET));
	data &=MS_MDMAECC_MS_RDY_FLAG;
//	data =(data>>25);
	data =(data>>16);       //modified by zbl
	DBG(PFX "ms_check_msrdy_flag:   return data = %x\n", data);
	return data;	
}
EXPORT_SYMBOL_GPL(ms_check_msrdy_flag);

/*void ms_clear_msrdy_flag(unsigned int value)
{
        //write 0 to clear this flag
	unsigned int data;
	DBG(PFX "ms_clear_msrdy_flag:   ---->\n");
	data = *((volatile unsigned *)(base + MS_MDMAECC_OFFSET));
	data &= ~(base + MS_MDMAECC_OFFSET_MS_RDY_FLAG);
	data |= (value>>25);
	*((volatile unsigned *)(base + MS_MDMAECC_OFFSET)) = data;	
	DBG(PFX "ms_clear_msrdy_flag:	write to base + MS_MDMAECC_OFFSET = %x\n", data);
} */
	
void ms_clear_msrdy_flag(void __iomem *base, unsigned int value)       //modified by zbl
{
        //write 1 to clear this flag
	unsigned int data;
	DBG(PFX "ms_clear_msrdy_flag:   ---->\n");
	data = *((volatile unsigned *)(base + MS_MDMAECC_OFFSET));
	data &= ~(MS_MDMAECC_CLR_MS_RDY_FLAG);
	data |= (value<<24);
	*((volatile unsigned *)(base + MS_MDMAECC_OFFSET)) = data;	
	DBG(PFX "ms_clear_msrdy_flag:	write to base + MS_MDMAECC_OFFSET = %x\n", data);
}
EXPORT_SYMBOL_GPL(ms_clear_msrdy_flag);

void ms_mserr_Intr_en_switch(void __iomem *base, unsigned int isEnable)
{
	unsigned int data;
	DBG(PFX "ms_mserr_Intr_en_switch:   ---->\n");
	data = *((volatile unsigned *)(base + MS_MDMAECC_OFFSET));
	data &= ~(MS_MDMAECC_MS_ERR_INTR_EN);
	//data |= (isEnable <<26);
	data |= (isEnable <<9);               //modified by zbl
	*((volatile unsigned *)(base + MS_MDMAECC_OFFSET)) = data;	
	DBG(PFX "ms_mserr_Intr_en_switch:   write to base + MS_MDMAECC_OFFSET = %x\n", data);
}
EXPORT_SYMBOL_GPL(ms_mserr_Intr_en_switch);

int ms_check_mserr_flag(void __iomem *base)
{
	int data;
	DBG(PFX "ms_check_mserr_flag:   ---->\n");
	data = *((volatile unsigned *)(base + MS_MDMAECC_OFFSET));
	data &=MS_MDMAECC_MS_ERR_FLAG;
	//data =(data>>27);
	data =(data>>17);       //modified by zbl
	DBG(PFX "ms_check_mserr_flag:   return data = %x\n", data);
	return data;	
}
EXPORT_SYMBOL_GPL(ms_check_mserr_flag);

void ms_clear_mserr_flag(void __iomem *base, unsigned int value)   //modified by zbl
{
        //write 1 to clear this flag
	unsigned int data;
	DBG(PFX "ms_clear_mserr_flag:   ---->\n");
	data = *((volatile unsigned *)(base + MS_MDMAECC_OFFSET));
	data &= ~(MS_MDMAECC_CLR_MS_ERR_FLAG);
	data |= (value<<25);
	*((volatile unsigned *)(base + MS_MDMAECC_OFFSET)) = data;	
	DBG(PFX "ms_clear_mserr_flag:	write to base + MS_MDMAECC_OFFSET = %x\n", data);
} 
EXPORT_SYMBOL_GPL(ms_clear_mserr_flag);

int ms_check_eccerr_flag(void __iomem *base)
{
	int data;
	DBG(PFX "ms_check_eccerr_flag:   ---->\n");
	data = *((volatile unsigned *)(base + MS_MDMAECC_OFFSET));
	data &=MS_MDMAECC_ECC_ERR_FLAG;
	//data =(data>>27);
	data =(data>>18);       //modified by zbl
	DBG(PFX "ms_check_eccerr_flag:   return data = %x\n", data);
	return data;	
}
EXPORT_SYMBOL_GPL(ms_check_eccerr_flag);

void ms_clear_eccerr_flag(void __iomem *base, unsigned int value)   //modified by zbl
{
        //write 1 to clear this flag
	unsigned int data;
	DBG(PFX "ms_clear_eccerr_flag:   ---->\n");
	data = *((volatile unsigned *)(base + MS_MDMAECC_OFFSET));
	data &= ~(MS_MDMAECC_CLR_ECC_ERR_FLAG);
	data |= (value<<26);
	*((volatile unsigned *)(base + MS_MDMAECC_OFFSET)) = data;	
	DBG(PFX "ms_clear_eccerr_flag:	write to base + MS_MDMAECC_OFFSET = %x\n", data);
} 
EXPORT_SYMBOL_GPL(ms_clear_eccerr_flag);

void ms_cd_intr_en_switch(void __iomem *base, unsigned int isEnable)
{
      unsigned int data;
      DBG(PFX "ms_cd_intr_en_switch:   ---->\n");
      data = *((volatile unsigned *)(base + MS_MDMAECC_OFFSET));
      data &= ~(MS_MDMAECC_DETECT_INTR_EN);
//      data |= (isEnable <<30);
      data |= (isEnable <<11);        //modified by zbl
      *((volatile unsigned *)(base + MS_MDMAECC_OFFSET)) = data;
      DBG(PFX "ms_cd_intr_en_switch:   write to base + MS_MDMAECC_OFFSET = %x\n", data);
}
EXPORT_SYMBOL_GPL(ms_cd_intr_en_switch);

int ms_cd_intr_flag(void __iomem *base)
{
      int data;
      DBG(PFX "ms_cd_intr_flag:   ---->\n");
      data = *((volatile unsigned *)(base + MS_MDMAECC_OFFSET));
      data &= MS_MDMAECC_DETECT_FLAG;
      //data = (data>>31);
      data = (data <<19);  // modified by gtm
      DBG(PFX "ms_cd_intr_flag:   data = %x\n", data);
      return data;    
       
}
EXPORT_SYMBOL_GPL(ms_cd_intr_flag);
 
/*
//write 0 to clear
void ms_clear_cd_intr_flag(unsigned int value)
{
      unsigned int data;
      DBG(PFX "ms_clear_cd_intr_flag:   ---->\n");
      data = *((volatile unsigned *)(base + MS_MDMAECC_OFFSET));
      data &= ~(MS_MDMAECC_DETECT_FLAG);
      data |= (value>>31);
      *((volatile unsigned *)(base + MS_MDMAECC_OFFSET)) = data;
      DBG(PFX "ms_clear_cd_intr_flag:   write to base + MS_MDMAECC_OFFSET = %x\n", data);
} 
*/

//write 1 to clear
void ms_clear_cd_intr_flag(void __iomem *base, unsigned int value) //modified by gtm
{
      unsigned int data;
      DBG(PFX "ms_clear_cd_intr_flag:   ---->\n");
      data = *((volatile unsigned *)(base + MS_MDMAECC_OFFSET));
      data &= ~(MS_MDMAECC_CLR_DETECT_FLAG);
      data |= (value<<27);
      *((volatile unsigned *)(base + MS_MDMAECC_OFFSET)) = data;
      DBG(PFX "ms_clear_cd_intr_flag:   write to base + MS_MDMAECC_OFFSET = %x\n", data);
} 
EXPORT_SYMBOL_GPL(ms_clear_cd_intr_flag);
 

/*LBA APIs : 0x54*/
void ms_set_lba(void __iomem *base, unsigned int value)
{
	unsigned int data;
	DBG(PFX "ms_set_lba:   ---->\n");
	data = *((volatile unsigned *)(base + MS_LBA_OFFSET));
	data &= ~(MS_LBA_LBAW);
	data |= (value << 9);
	*((volatile unsigned *)(base + MS_LBA_OFFSET)) = data;	
	DBG(PFX "ms_set_lba:	write to MS_LBA = %x\n", data);
}
EXPORT_SYMBOL_GPL(ms_set_lba);

/*DMA_ADDR APIs : 0x5c*/
void ms_set_dmaaddr(void __iomem *base, unsigned int addr)
{
        DBG(PFX "ms_set_dmaaddr:   ---->\n");
	*((volatile unsigned *)(base + MS_DMA_ADDR_OFFSET)) = addr;
	DBG(PFX "ms_set_dmaaddr:	write to base + MS_DMA_ADDR_OFFSET = %x\n", addr);
}
EXPORT_SYMBOL_GPL(ms_set_dmaaddr);

/*REG_CMD APIs : 0x70*/
void ms_write_cmd(void __iomem *base, unsigned int cmd)
{
        DBG(PFX "ms_write_cmd:   ---->\n");
	*((volatile unsigned *)(base + MS_REG_CMD_OFFSET)) = cmd;
	DBG(PFX "ms_write_cmd:	write to base + MS_REG_CMD_OFFSET = %x\n", cmd);
}
EXPORT_SYMBOL_GPL(ms_write_cmd);

unsigned int ms_read_cmd(void __iomem *base)
{
        unsigned int data;
        DBG(PFX "ms_read_cmd:   ---->\n");
        data = *((volatile unsigned *)(base + MS_REG_CMD_OFFSET));
        DBG(PFX "ms_read_cmd:   return data = %x\n", data);
        return data;
        //return *((volatile unsigned *)(base + MS_REG_CMD_OFFSET));
}
EXPORT_SYMBOL_GPL(ms_read_cmd);
                
/*REG_DATA APIs : 0x74*/
void ms_write_data(void __iomem *base, unsigned int value)
{
      DBG(PFX "ms_write_data:   ---->\n");
      *((volatile unsigned *)(base + MS_REG_DATA_OFFSET)) = value;
      DBG(PFX "ms_write_data:	write to base + MS_REG_DATA_OFFSET = %x\n", value);
}
EXPORT_SYMBOL_GPL(ms_write_data);

unsigned int ms_read_data(void __iomem *base)
{
	unsigned int value;
        DBG(PFX "ms_read_data:   ---->\n");
	value = *((volatile unsigned *)(base + MS_REG_DATA_OFFSET));
	DBG(PFX "ms_read_data:   return value = %x\n", value);
	return value;
}	
EXPORT_SYMBOL_GPL(ms_read_data);

/*REG_DUMMYCLOCK APIs : 0x78*/
void ms_write_dummy(void __iomem *base, unsigned int value)
{
        DBG(PFX "ms_write_dummy:   ---->\n");
	*((volatile unsigned *)(base + MS_REG_DUMMYCLOCK_OFFSET)) = value;
	DBG(PFX "ms_write_dummy:	write to MS_REG_DUMMYCLOCK = %x\n", value);
}
EXPORT_SYMBOL_GPL(ms_write_dummy);

/*AUTO_RESPONSE APIs : 0x7c*/
void ms_write_autoresponse(void __iomem *base, unsigned int value)
{
        DBG(PFX "ms_write_autoresponse:   ---->\n");
	*((volatile unsigned *)(base + MS_AUTO_RESPONSE_OFFSET)) = value;
	DBG(PFX "ms_write_autoresponse:   write to MS_AUTO_RESPONSE = %x\n", value);
}
EXPORT_SYMBOL_GPL(ms_write_autoresponse);
