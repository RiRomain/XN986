/*
 * Copyright (c) 2009 SONIX Limited.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * Saxen 03/23/2009 created
 * Davian 04/21/2013 modified
 */
#ifndef __MACH_MS_H
#define __MACH_MS_H

/*CTL APIs : 0x00*/
extern void ms_set_mode (unsigned mode);
extern void ms_set_cpu_rw (unsigned int value);
extern void ms_dma_en (unsigned int isEnable);
extern void ms_set_dma_rw (unsigned int value);
extern void ms_extra_en (unsigned int isEnable);
extern void ms_ecc_en (unsigned int isEnable);
extern unsigned int ms_check_msrdy (void);
extern void ms_set_spibusy_tri (unsigned int isEnable);
extern void ms_set_spicmd_tri (unsigned int isEnable);
extern void ms_read_data_cmd (unsigned int isEnable);
extern void ms_set_speed (unsigned int msspeed);

/*DMA_SIZE APIs : 0x04*/
extern void ms_set_dmasize (unsigned int size);

/*CRC1 APIs : 0x08*/
extern unsigned int ms_read_crc16_0 (void);
extern unsigned int ms_read_crc7 (void);

/*CRC2 APIs : 0x0c*/
extern unsigned int ms_read_crc16_1 (void);

/*CRC3 APIs : 0x10*/
extern unsigned int ms_read_crc16_2and16_3 (void);

/*MS_IO_I APIs : 0x30*/
extern unsigned int ms_io_i (void);

/*MS_IO_O APIs : 0x34*/
extern void ms_io_o_8 (unsigned int value); //set MS_IO8

/*MS_IO_OE APIs : 0x38*/
extern void ms_io_oe_8 (unsigned int value); //set MS_IO8

/*SPI_CMD APIs : 0x3c*/
extern void ms_write_spi_cmd (unsigned int cmd);
extern unsigned int ms_read_spi_cmd (void);

/*SD SPI_INDEX APIs ; 0x40*/
extern void ms_write_spi_index (unsigned int index);
extern unsigned int ms_read_spi_index (void);

/*DMA_BLKSU APIs : 0x48*/
extern void ms_dmablock (unsigned int dmaBlockNum);
extern unsigned int ms_read_sudmablock (void);

/*TMCNT APIs : 0x4c*/
extern void ms_set_timecount (unsigned int timeCnt);

/*MDMAECC APIs : 0x50*/
extern void ms_set_multi_block_xfer (unsigned int isEnable);
extern unsigned int ms_check_mdma_ok (void);
extern unsigned int ms_check_mdma_timeout (void);
extern unsigned int ms_check_rd_crc_err (void);
extern unsigned int ms_check_wt_crc_err (void);
extern void ms_rdy_intr_en (unsigned int isEnable);
extern unsigned int ms_check_rdy_flag (void);
extern void ms_clear_rdy_flag (unsigned int value);
extern void ms_err_intr_en (unsigned int isEnable);
extern unsigned int ms_check_err_flag (void);
extern void ms_clear_err_flag (unsigned int value);
extern void ms_cd_intr_en (unsigned int isEnable);
extern unsigned int ms_check_cd_intr_flag (void);
extern void ms_clear_cd_intr_flag (unsigned int value);
/*LBA APIs : 0x54*/
extern void ms_set_lba (unsigned int value);

/*DMA_ADDR APIs : 0x5c*/
extern void ms_set_dmaaddr (unsigned int addr);

/*REG_CMD APIs : 0x70*/
extern void ms_write_cmd( unsigned int cmd);
extern unsigned int ms_read_cmd (void);

/*REG_DATA APIs : 0x74*/
extern void ms_write_data (unsigned int value);
extern unsigned int ms_read_data (void);

/*REG_DUMMYCLOCK APIs : 0x78*/
extern void ms_write_dummy (unsigned int value);

/*AUTO_RESPONSE APIs : 0x7c*/
extern void ms_write_autoresponse (unsigned int value);

#endif

