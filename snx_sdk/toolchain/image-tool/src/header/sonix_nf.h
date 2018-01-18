#ifndef __SONIX_NAND_FLASH_H__
#define __SONIX_NAND_FLASH_H__

#include "common.h"

int ms_nand_flash_init(void);
int ms_nand_flash_erase(u32 offset, u32 len);
int ms_nand_flash_write(u32 offset, uchar *buf, u32 len);
#endif
