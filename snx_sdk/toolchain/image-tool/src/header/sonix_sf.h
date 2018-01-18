#ifndef __SONIX_SERIAL_FLASH_H__
#define __SONIX_SERIAL_FLASH_H__

#if 0
#define SERIAL_HW_OFFSET     0x000000
#define SERIAL_UBOOT_OFFSET  0x001000
#define	SERIAL_RESCUE_OFFSET 0x500000
#define SERIAL_KERNEL_OFFSET 0x060000
#define SERIAL_ROOTFS_OFFSET 0x300000

#define SERIAL_HW_MAXLEN     0x001000
#define SERIAL_UBOOT_MAXLEN  0x400000
#define SERIAL_RESCUE_MAXLEN 0x004000
#define SERIAL_KERNEL_MAXLEN 0x2A0000
#define SERIAL_ROOTFS_MAXLEN 0xA00000
#define SERIAL_TOTAL_MAXLEN  0x600000
#else
#include "flash_layout.h"
#endif

struct sf_instruction
{
	u32 WREN;
	u32 WRDI;
	u32 RDID;
	u32 RDSR;
	u32 WRSR;
	u32 READ;
	u32 PP;
	u32 SE;
	u32 BE;
	u32 CE;
	u32 DP;
	u32 RES;

	u32 AAI;
	u32 AAW;
	u32 BP;
	u32 EWSR;

	u32 TBP;
	u32 MF;
};

struct serial_flash {
	char *name;
	
	const u32 id0;
	const u32 id1;
	const u32 id2;
	
	u32 page_size;    /* unit: bytes, set to 0x0 if not support page program */
	u32 sector_size;  /* unit: bytes, set to 0x0 if not support sector erase */
	u32 block_size;   /* unit: bytes, set to 0x0 if not support block erase  */
	u32 chip_size;    /* unit: bytes */
	u32 max_hz;
	
	struct sf_instruction *sf_inst;

	int (*readID)(struct sf_instruction *sf_inst, u32 id[]);
	int (*sector_erase)(struct sf_instruction *sf_inst, u32 offset);
	int (*block_erase)(struct sf_instruction *sf_inst, u32 offset);
	int (*cpu_write)(struct sf_instruction *sf_inst, u32 offset, 
		uchar *addr, u32 len);
	int (*dma_write)(struct sf_instruction *sf_inst, u32 offset, 
		uchar *addr, u32 len);
	int (*mdma_write)(struct sf_instruction *sf_inst, u32 offset, 
		uchar *addr, u32 len, u32 mblks);
	////// add on 2013-8-30
	int (*clearSR)(struct sf_instruction *sf_inst);
	int (*ProtectSR)(struct sf_instruction *sf_inst);
	////// end
};
#endif
