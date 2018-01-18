#ifndef SNX_SPI_H
#define SNX_SPI_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>        
#include <getopt.h>             /* getopt_long() */
#include <fcntl.h>              /* low-level i/o */
#include <ctype.h>
#include <malloc.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#define SSP_GPIO_INPUT			0
#define SSP_GPIO_OUTPUT			1

#define SPI_RD_OP_MODE			0
#define SPI_WR_OP_MODE			1
#define SPI_WRRD_OP_MODE		2

#endif
