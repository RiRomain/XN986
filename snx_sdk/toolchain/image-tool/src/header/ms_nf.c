#include "common.h"

#include "ms_nf.h"

//=========================================================================
// Set MS1 operation mode. 
// [0]:GPIO; [1]:NF; [2]:SD; [3]:SF 
//=========================================================================
void ms1_set_mode(enum ms1_mode mode)
{
	u32 data = 0;
	
	data  = readl(MS1_CTL);
	data &= ~MS1_CTL_MODE_MASK;
	data |= (mode << MS1_CTL_MODE_BIT);
	
	writel(data, MS1_CTL);
}

void ms1_info_disable(void)
{
	u32 data = 0;
	
	data  = readl(MS1_CTL);
	data |= BIT23;
	writel(data, MS1_CTL);
}


//=========================================================================
// Set MS1 register r/w mode. 
// [0]:write; [1]:read 
//=========================================================================
void ms1_set_RegRW(enum ms1_reg_rw_mode mode)
{
	u32 data;

	data  = readl(MS1_CTL);
	data &= ~MS1_CTL_REGRW_MASK;
	data |= (mode << MS1_CTL_REGRW_BIT);
	
	writel(data, MS1_CTL);
}

//=========================================================================
// Enable extra ECC for MS1.  
//=========================================================================
void ms1_en_ExtraEcc(u32 isEnable)
{
	u32 data;

	data  = readl(MS1_CTL);
	data &= ~MS1_CTL_EXTRAECC_MASK;
	data |= (isEnable << MS1_CTL_EXTRAECC_BIT);
	
	writel(data, MS1_CTL);
}

//=========================================================================
// Enable ECC or CRC for MS1.  
//=========================================================================
void ms1_en_ecc(u32 isEnable)
{
	u32 data;

	data  = readl(MS1_CTL);
	data &= ~MS1_CTL_ECCEN_MASK;
	data |= (isEnable << MS1_CTL_ECCEN_BIT);
	
	writel(data, MS1_CTL);
}

//=========================================================================
// Enable DMA for MS1.  
//=========================================================================
void ms1_en_dma(u32 isEnable)
{
	u32 data;
	
	data  = readl(MS1_CTL);
	data &= ~MS1_CTL_DMAEN_MASK;
	data |= (isEnable << MS1_CTL_DMAEN_BIT);
	
	writel(data, MS1_CTL);
}

//=========================================================================
// Set MS1 DMA r/w mode. 
// [0]:write; [1]:read 
//=========================================================================
void ms1_set_DmaRW(u32 value)
{
	u32 data;
	
	data  = readl(MS1_CTL);
	data &= ~MS1_CTL_DMARW_MASK;
	data |= (value << MS1_CTL_DMARW_BIT);
	
	writel(data, MS1_CTL);
}

//=========================================================================
// Check r/w ready for MS1.  
//=========================================================================
u32 check_ms1_rdy (void)
{
	u32 data;

	data   = readl(MS1_CTL);
	data  &= MS1_CTL_RDY_MASK;
	data >>= MS1_CTL_RDY_BIT;
	
	return data;
}

//=========================================================================
// MS1 set SPI command.
// Write: command[31:0] for SPI/SD command token
// Read: response 1~4 for SPI/SD command token 
//=========================================================================
void ms1_set_SpiCmd (u32 cmd)		//SPI_CMD0, SPI_CMD1, SPI_CMD2, SPI_CMD3, (SPI_CMD4 seems useless)
{
	writel(cmd, MS1_SPI_CMD);
}

//=========================================================================
// MS1 set SPI index.
// Write: index[7:0] for SPI/SD command token
// Read: response 0 for SPI/SD command token 
//=========================================================================
void ms1_set_SpiIdx (u32 idx)		//SPI_CMD0, SPI_CMD1, SPI_CMD2, SPI_CMD3, (SPI_CMD4 seems useless)
{
	writel(idx, MS1_SPI_IDX);
}

//=========================================================================
// MS1 set DMA size.
// transfer length: (DMA_SIZE + 1)byte
//=========================================================================
void ms1_set_DmaSize (u32 size)
{
	u32 data;

	data  = readl(MS1_DMA_SIZE);
	data &= ~MS1_DMA_SIZE_MASK;
	data |= (size << MS1_DMA_SIZE_BIT);
	
	writel(data, MS1_DMA_SIZE);
}

//=========================================================================
// MS1 set DMA block number.
// DMA block numbers = DMA_BLOCk + 1
//=========================================================================
void ms1_set_DmaBlk (u32 dmaBlockNum)
{
	u32 data;

	data  = readl(MS1_DMA_BLKSU);
	data &= ~MS1_DMA_BLOCK_MASK;
	data |= (dmaBlockNum << MS1_DMA_BLOCK_BIT);
	
	writel(data, MS1_DMA_BLKSU);
}

//=========================================================================
// MS1 set time count.
// Time is = clk_cycle * (TIME_CNT * 4)  
//=========================================================================
void ms1_set_TimeCount (u32 timeCnt)
{
	u32 data;

	data  = readl(MS1_TIME_CNT);
	data &= ~MS1_TIME_CNT_MASK;
	data |= (timeCnt << MS1_TIME_CNT_BIT);
	
	writel(data, MS1_TIME_CNT);
}

//=========================================================================
// Enable MDMA for MS1.  
//=========================================================================
void ms1_en_Mdma(u32 isEnable)
{
	u32 data;

	data  = readl(MS1_MDMAECC);
	data &= ~MS1_MDMAEN_MASK;
	data |= (isEnable << MS1_MDMAEN_BIT);
	
	writel(data, MS1_MDMAECC);
}

//=========================================================================
// Check MDMA finish for MS1.  
//=========================================================================
u32 check_ms1_MdmaOk (void)
{
	u32 data;

	data  = readl(MS1_MDMAECC);
	data &= MS1_MDMAOK_MASK;
	data  = data >> MS1_MDMAOK_BIT;
    
	return data;
}

//=========================================================================
// Check MDMA finisih timeout for MS1.  
//=========================================================================
u32 check_ms1_MdmaTimeOut (void)
{
	u32 data;

	data  = readl(MS1_MDMAECC);
	data &= MS1_MDMATIMEOUT_MASK;
	data  = data >> MS1_MDMATIMEOUT_BIT;

	return data;
}

//=========================================================================
// Check CRC_W err for MS1. 
// SD: Write Data CRC Check Error
// NF: 2 bit upon, ECC err correct is faild 
//=========================================================================
u32 check_ms1_CRCWErr (void)
{
	u32 data;

	data   = readl(MS1_MDMAECC);
	data  &= MS1_CRCERR_MASK;
	data >>= MS1_CRCERR_BIT;
	
	return data;
}

//=========================================================================
// Clear CRC_W err flag for MS1.  
//=========================================================================
void clear_ms1_CRCWErr (void)
{
	u32 data;

	data  = readl(MS1_MDMAECC);
	data |= 1 << MS1_CRCERR_BIT;
	
	writel(data, MS1_MDMAECC);
}

//=========================================================================
// Check ECC err for MS1.  
// SD: Read Data CRC Check Error
// NF: 1 bit, ECC err can correct 
//=========================================================================
u32 check_ms1_EccErr (void)
{
	u32 data;

	data   = readl(MS1_MDMAECC);
	data  &= MS1_ECCERR_MASK;
	data >>= MS1_ECCERR_BIT;
	
	return data;
}

//=========================================================================
// Clear ECC err flag for MS1.  
//=========================================================================
void clear_ms1_EccErr (void)
{
	u32 data;

	data  = readl(MS1_MDMAECC);
	data |= 1 << MS1_ECCERR_BIT;
	
	writel(data, MS1_MDMAECC);
}

//=========================================================================
// Enable intr for MS_RDY 0 -> 1   
//=========================================================================
void ms1_en_RdyIntr (u32 isEnable)
{
	u32 data;

	data  = readl(MS1_MDMAECC);
	data &= ~MS1_RDYEN_MASK;
	data |= (isEnable << MS1_RDYEN_BIT);
	
	writel(data, MS1_MDMAECC);
}

//=========================================================================
// Enable intr for ABH bus err  
//=========================================================================
void ms1_en_AhbErrIntr (u32 isEnable)
{
	u32 data;

	data  = readl(MS1_MDMAECC);
	data &= ~MS1_ERREN_MASK;
	data |= (isEnable << MS1_ERREN_BIT);
	
	writel(data, MS1_MDMAECC);
}

//=========================================================================
// Enable intr for CRC_W_ERR == 1 or ECC_ERR == 1  
//=========================================================================
void ms1_en_EccErrIntr (u32 isEnable)
{
	u32 data;

	data  = readl(MS1_MDMAECC);
	data &= ~MS1_ECCEN_MASK;
	data |= (isEnable << MS1_ECCEN_BIT);
	
	writel(data, MS1_MDMAECC);
}

//=========================================================================
// Check intr flag for MS_RDY 0 -> 1  
//=========================================================================
u32 check_ms1_RdyIntr (void)
{
	u32 data;

	data   = readl(MS1_MDMAECC);
	data  &= MS1_RDYFLAG_MASK;
	data >>= MS1_RDYFLAG_BIT;
	
	return data;
}

//=========================================================================
// Check intr for AHB bus err.  
//=========================================================================
u32 check_ms1_AhbErrIntr (void)
{
	u32 data;

	data   = readl(MS1_MDMAECC);
	data  &= MS1_ERRFLAG_MASK;
	data >>= MS1_ERRFLAG_BIT;
	
	return data;
}

//=========================================================================
// Check Intr for CRC_W_ERR == 1 or ECC_ERR == 1  
//=========================================================================
u32 check_ms1_EccErrIntr (void)
{
	u32 data;

	data   = readl(MS1_MDMAECC);
	data  &= MS1_ECCFLAG_MASK;
	data >>= MS1_ECCFLAG_BIT;
	
	return data;
}

//=========================================================================
// Clear intr flag for MS_RDY.  
//=========================================================================
void clear_ms1_RdyIntr (void)
{
	u32 data;

	data  = readl(MS1_MDMAECC);
	data |= 0x1 << MS1_CLR_RDY_BIT;
	
	writel(data, MS1_MDMAECC);
}

//=========================================================================
// Clear intr flag for AHB bus err.  
//=========================================================================
void clear_ms1_AhbErrIntr (void)
{
	u32 data;

	data  = readl(MS1_MDMAECC);
	data |= 1 << MS1_CLR_ERR_BIT;
	
	writel(data, MS1_MDMAECC);
}

//=========================================================================
// Clear intr ECCERR for CRC_W_ERR == 1 or ECC_ERR == 1
//=========================================================================
void clear_ms1_EccErrIntr (void)
{
	u32 data;

	data  = readl(MS1_MDMAECC);
	data |= 1 << MS1_CLR_ECC_BIT;
	
	writel(data, MS1_MDMAECC);
}

//=========================================================================
// MS1 set DMA addr.
// Please set DMA_ADDR[1:0] = 0 to meet 4 byte alignment.
//=========================================================================
void ms1_set_DmaAddr (u32 addr)
{
	writel(addr, MS1_DMAADDR);
}

//=========================================================================
// MS1 write command.
//=========================================================================
void ms1_write_cmd (u32 cmd)
{
	cmd &= 0xff;
	writel(cmd, MS1_REG_CMD);
}

//=========================================================================
// MS1 write addr.  
//=========================================================================
void ms1_write_addr (u32 addr)
{
	addr &= 0xff;
	writel(addr, MS1_REG_ADDR);
}

//=========================================================================
// MS1 write data.  
//=========================================================================
void ms1_write_data (u32 data)
{
	data &= 0xff;
	writel(data, MS1_REG_DATA);
}

//=========================================================================
// MS1 read data.  
//=========================================================================
u32 ms1_read_data (u32 value)
{
	writel(value, MS1_REG_DATA);

	return readl(MS1_REG_DATA);
}

//=========================================================================
// Read GPIO data in.
//=========================================================================
u32 ms1_gpio_read(u32 gpio_num)
{
	u32 data = 0;

	data = readl(MS1_GPIO_I);
	data = (data >> gpio_num) & 0x1;

	return data;
}

//=========================================================================
// Write GPIO data out.
//=========================================================================
void ms1_gpio_write(u32 val, u32 gpio_num)
{
	u32 data = 0;	

	data  = readl(MS1_GPIO_O);
	data &= (~(0x1 << gpio_num));
	data |= ((val & 0x1) << gpio_num);
	
	writel(data, MS1_GPIO_O);	
}

//=========================================================================
// MS1 GPIO CE mode
// 0: input mode, default
// 1: output mode
//=========================================================================
void ms1_gpio_mode(u32 val, u32 gpio_num)
{
	u32 data;
	
	data  = readl(MS1_GPIO_OE);
	data &= (~(0x1 << gpio_num));
	data |= ((val & 0x1) << gpio_num);
	
	writel(data, MS1_GPIO_OE);
}

//=========================================================================
// Set nibble command for NF.
// [0]:reset NF; [1]:erase NF in multi-plane mode;
// [2]:erase NF; [3]:read INFO data from NF
// others: reserved
//=========================================================================
void nf_set_NibbleCmd (enum nibble_cmd mode)
{
	u32 data;

	data  = readl(MS1_NF_CMD);	
	data &= ~NAND_CMD_NIBBLE_MASK;
	data |= (mode << NAND_CMD_NIBBLE_BIT);
	
	writel(data, MS1_NF_CMD);
}

//=========================================================================
// Trigger to start nibble command for NF.
//=========================================================================
void nf_Trg_NibbleCmd (void)
{
	u32 data;

	data = readl(MS1_CTL);
	data &= ~MS1_CTL_CMD_NIBBLE_TRG_MASK;
	data |= (1 << MS1_CTL_CMD_NIBBLE_TRG_BIT);
	
	writel(data, MS1_CTL);
}

//=========================================================================
// Enable multi-plane w/r for NF.
//=========================================================================
void nf_en_MPdma(u32 isEnable)
{
	u32 data;

	data = readl(MS1_CTL);
	data &= ~MS1_CTL_NF_MP_MASK;
	data |= (isEnable << MS1_CTL_NF_MP_BIT);
	
	writel(data, MS1_CTL);
}

//=========================================================================
// Read low pules width for NF.
// Range: 1~3
//=========================================================================
void nf_set_rd_width (u32 val)
{
	u32 data;
			
	data = readl(MS1_CTL);
	data &= ~MS1_CTL_RDWIDTH_MASK;
	data |= (val << MS1_CTL_RDWIDTH_BIT);
	
	writel(data, MS1_CTL);
}

//=========================================================================
// Write low pules width for NF.
// Range: 1~3
//=========================================================================
void nf_set_wr_width (u32 val)
{
	u32 data;

	data  = readl(MS1_CTL);
	data &= ~MS1_CTL_WRWIDTH_MASK;
	data |= (val << MS1_CTL_WRWIDTH_BIT);
	
	writel(data, MS1_CTL);
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
void nf_set_InfoMode (u32 cmd)
{
	u32 data;
	
	data  = readl(MS1_CTL);
	data &= ~MS1_CTL_NF_INFOWR_MASK;
	data |= (cmd << MS1_CTL_NF_INFOWR_BIT);
	
	writel(data, MS1_CTL);
}

//=========================================================================
// Set odd cmd0 for NF.
// [0]:0x80; [1]:0x81
//=========================================================================
void nf_set_OddCmd0 (u32 cmd)
{
	u32 data;

	data  = readl(MS1_NF_CMD);
	data &= ~NAND_ODD_CMD0_MASK;
	data |= cmd << NAND_ODD_CMD0_BIT;
	
	writel(data, MS1_NF_CMD);	
}

//=========================================================================
// Set odd cmd1 for NF.
// [0]:0x10; [1]:0x15; [2]:0x11
//=========================================================================
void nf_set_OddCmd1 (u32 cmd)
{
	u32 data;

	data  = readl(MS1_NF_CMD);
	data &= ~NAND_ODD_CMD1_MASK;
	data |= cmd << NAND_ODD_CMD1_BIT;
	
	writel(data, MS1_NF_CMD);	
}

//=========================================================================
// Set even cmd0 for NF.
// [0]:0x80; [1]:0x81
//=========================================================================
void nf_set_EvenCmd0 (u32 cmd)
{
	u32 data;

	data  = readl(MS1_NF_CMD);
	data &= ~NAND_EVEV_CMD0_MASK;
	data |= cmd << NAND_EVEV_CMD0_BIT;
	
	writel(data, MS1_NF_CMD);	
}

//=========================================================================
// Set even cmd1 for NF.
// [0]:0x10; [1]:0x15; [2]:0x11
//=========================================================================
void nf_set_EvenCmd1 (u32 cmd)
{
	u32 data;

	data  = readl(MS1_NF_CMD);
	data &= ~NAND_EVEN_CMD1_MASK;
	data |= cmd << NAND_EVEN_CMD1_BIT;
	
	writel(data, MS1_NF_CMD);	
}

//=========================================================================
// Read status cmd for NF.
//=========================================================================
void nf_set_ReadStatus (u32 cmd)
{
	u32 data;

	data  = readl(MS1_NF_CMD);
	data &= ~NAND_READ_STATUS_CMD_MASK;
	data |= cmd << NAND_READ_STATUS_CMD_BIT;
	
	writel(data, MS1_NF_CMD);	
}

//=========================================================================
// Set addr cycle for NF.
// [0]:addr cycle = 3; [1]:addr cycle = 4;
// [2]:addr cycle = 5; others:reserved
//=========================================================================
void nf_set_AddrCyc (u32 val)
{
	u32 data;

	data  = readl(MS1_NF_CMD);
	data &= ~NAND_CMD_ADR_CYC_MASK;
	data |= (val << NAND_CMD_ADR_CYC_BIT);
	
	writel(data, MS1_NF_CMD);	
}

//=========================================================================
// Set reading command count for NF.
// [0]: one command when read.
// [1]: two commands when read. (MLC)
//=========================================================================
void nf_set_ReadCmdCnt (u32 readCmdCnt)
{
	u32 data;

	data  = readl(MS1_NF_CMD);
	data &= ~NAND_CMD_RD_CMD_CNT_MASK;
	data |= (readCmdCnt << NAND_CMD_RD_CMD_CNT_BIT);
	
	writel(data, MS1_NF_CMD);
}

//=========================================================================
// Set page size for NF.
// [0]:512 B/page; [1]:2k B/page;
// [2]:4k B/page; others:reserved
//=========================================================================
void nf_set_PageSize (u32 value)
{
	u32 data;

	data  = readl(MS1_NF_CMD);
	data &= ~NAND_CMD_PG_SIZE_MASK;
	data |= (value << NAND_CMD_PG_SIZE_BIT);
	
	writel(data, MS1_NF_CMD);
}

//=========================================================================
// Set block size for NF.
// [0]:16 page; [1]:32 page;
// [2]:64 page; [3]:128 page
//=========================================================================
void nf_set_BlkSize (u32 value)
{	
	u32 data;

	data  = readl(MS1_NF_CMD);
	data &= ~NAND_CMD_BLK_SIZE_MASK;
	data |= (value << NAND_CMD_BLK_SIZE_BIT);
	
	writel(data, MS1_NF_CMD);
}

//=========================================================================
// Set LBA increment for NF.
// [0]:No inc; [1]:inc 1 for every DMA block;
// [2]:inc 1 for every 2 DMA block; [3]:inc 1 for every 4 DMA block;
// [4]:inc 1 for every 8 DMA block; [5]:inc 1 for every 16 DMA block;
// [6]:inc 1 for every 32 DMA block; [7]:inc 1 for every 64 DMA block;
// [8]:inc 1 for every 128 DMA block; others: reserved
//=========================================================================
void nf_set_LBAIncMode (u32 cmd)
{
	u32 data;

	data  = readl(MS1_NF_CMD);
	data &= ~NAND_CMD_LBA_INC_MASK;
	data |= (cmd << NAND_CMD_LBA_INC_BIT);
	
	writel(data, MS1_NF_CMD);
}

//=========================================================================
// Check multi-DMA program err for NF. 
//=========================================================================
u32 check_nf_PrgErr (void)
{
	u32 data;

	data  = readl(MS1_MDMAECC);
	data &= MS1_NF_PRG_ERR_MASK;
	data >>= MS1_NF_PRG_ERR_BIT;
	
	return data;
}

//=========================================================================
// Clear multi-DMA program err flag for NF.  
//=========================================================================
void clear_nf_PrgErr (void)
{
	u32 data;

	data  = readl(MS1_MDMAECC);
	data |= 1 << MS1_NF_PRG_ERR_BIT;
	
	writel(data, MS1_MDMAECC);
}

//=========================================================================
// Check INFO CRC err for NF. 
//=========================================================================
u32 check_nf_InfoCrcErr (void)
{
	u32 data;

	data = readl(MS1_MDMAECC);
	data &= MS1_NF_INFO_ERR_MASK;
	data >>= MS1_NF_INFO_ERR_BIT;
	
	return data;
}

//=========================================================================
// Clear INFO CRC err flag for NF.  
//=========================================================================
void clear_nf_InfoCRCErr (void)
{
	u32 data;

	data = readl(MS1_MDMAECC);
	data |= 1 << MS1_NF_INFO_ERR_BIT;
	
	writel(data, MS1_MDMAECC);
}

//=========================================================================
// Check erase err for NF. 
//=========================================================================
u32 check_nf_EraseErr (void)
{
	u32 data;

	data = readl(MS1_MDMAECC);
	data &= MS1_NF_ERASE_ERR_MASK;
	data >>= MS1_NF_ERASE_ERR_BIT;
	
	return data;
}

//=========================================================================
// Clear erase err flag for NF.  
//=========================================================================
void clear_nf_EraseErr (void)
{
	u32 data;

	data = readl(MS1_MDMAECC);
	data |= 1 << MS1_NF_ERASE_ERR_BIT;
	
	writel(data, MS1_MDMAECC);
}

//=========================================================================
// Read LBA data info for NF.
//=========================================================================
int nf_read_NandLBA_R (void)
{
	int data;
	
	data = readl(MS1_LBA_R);
	data &= NAND_SIZELBA_LBA_BIT;
	
	return data;
}

