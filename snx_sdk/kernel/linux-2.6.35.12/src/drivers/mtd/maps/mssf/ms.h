#ifndef SNX_MS_H
#define SNX_MS_H

//serial flash wtite mode
#define SF_NORMAL			0x0
#define SF_AAI				0x1
#define SF_AAW				0x2

//sf_type
#define SF_SST_AAI			0
#define SF_SST_AAW			1
#define SF_SST_PP			2
#define SF_MXIC_AMIC			3
#define SF_PMC				4
#define SF_EN				5
#define SF_GD				6
#define SF_SPANSION			7
#define SF_WINBOND			8

struct sf_instruction
{
	int typenum;
	
	unsigned int WREN;
	unsigned int WRDI;
	unsigned int RDID;
	unsigned int RDSR;
	unsigned int WRSR;
	unsigned int READSF;
	unsigned int PP;
	unsigned int SE;
	unsigned int BE;
	unsigned int DP;
	unsigned int RES;

	unsigned int AAI;	//SST
	unsigned int AAW;	//SST
	unsigned int BP;	//SST
	unsigned int EWSR;	//SST

	unsigned int TBP;	//For spi mode sf dma write : Time for byte program

	unsigned int RDQPIID;
	unsigned int QPIREAD;
	unsigned int QPIRD_DUMMY;
	unsigned int QPIWR_DUMMY;
	unsigned int FASTRD;
	unsigned int FASTRD_DUMMY;
	unsigned int DREAD;
	unsigned int DREAD_DUMMY;

	unsigned int QPP;

	unsigned int FASTDTRD;
	unsigned int DTRD4;
};

#define SF_NULL	0

struct sf_id
{
	unsigned int id1;
	unsigned int id2;
	unsigned int id3;
};

void sf_SetInstruction (int sf_type, struct sf_instruction *sf_inst);
/*0x00*/
void mssf_set_msmode(unsigned mode);
void mssf_msreg_rw_switch(unsigned int value);
void mssf_msdma_en_switch(unsigned int value);
void mssf_msdma_rw_switch(unsigned int value);
void mssf_extra_en_switch(unsigned int value);
void mssf_ecc_en_switch(unsigned int value);
int mssf_check_msrdy(void);
void mssf_crc_cal_switch(unsigned int value);
void mssf_set_wmode(unsigned int sfwmode);
void mssf_set_msspeed(unsigned int msspeed);
/*0x04*/
void mssf_set_dmasize(unsigned int size);
/*0x08*/
unsigned int mssf_read_crc16(void);
//0x34, 0x38
void mssf_ms_io_oe_switch(unsigned int value);
void mssf_chip_enable_switch(unsigned int value);
//0x40
void mssf_set_wcmd(unsigned int sfCmd);
//0x48
void mssf_dmablock(unsigned int dmaBlockNum);
unsigned int mssf_read_sudmablock(void);
//0x4c
void mssf_set_timecount(unsigned int timeCnt);
 //0x50
void mssf_msmdma_en_switch(unsigned int value);
int mssf_check_msmdma_ok(void);
int mssf_check_msmdma_timeout(void);
void mssf_msrdy_Intr_en_switch(unsigned int value);
int mssf_check_msrdy_flag(void);
void mssf_clear_msrdy_flag(unsigned int value);
void mssf_mserr_Intr_en_switch(unsigned int value);
int mssf_check_mserr_flag(void);
void mssf_clear_mserr_flag(unsigned int value);
//0x54
void mssf_set_lba(unsigned int value);
//0x5c
void mssf_set_dmaaddr(unsigned int addr);
//0x74
void mssf_write_data(unsigned int value);
unsigned int mssf_read_data(void);

//sfgpio
void mssf_ms_io_wt (unsigned int num,unsigned int mode ,unsigned int value);
unsigned int mssf_ms_io_rd (unsigned int num);
void mssf_ms_io_release(void);


/*
ST58660FPGA ADD
-mdma
    -MX25L12835F : QPI
    -MX25L12845E : DT
*/
void sfSetCacheWcmd(unsigned int cmd);
void sfSetRDDummyByte(unsigned int n);
void sfSetWRDummyByte(unsigned int n);
void sfSetAddrCyc(unsigned int n);
void sfSetCmdNibble(unsigned int n);
void sfSetCacheRcmd(unsigned int cmd);
void sfSetErasecmd(unsigned int cmd);
void sfSetWEnCmd(unsigned int cmd);
void sfSetStatusCmd(unsigned int cmd);
void sfSetQPIMode(unsigned int en);
void sfSetDummyEN(unsigned int en);
void sfSetDummyCyc(unsigned int cyc);
void sfSetPhsDelay(unsigned int d);
void sf4BitsSwitch(unsigned int isEnable);
void sfDoubleRateSwitch(unsigned int isEnable);
unsigned int sfis4BitMode(void);
void sfSetMdmaStartAddr(unsigned int addr);

#endif
