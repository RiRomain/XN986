/**
 *
 * SONiX SDK Example Code
 * Category: SPI
 * File: snx_spi_gpio.c
 * Usage: 
 *		 1. Make sure your platform has attached spi device, "spidev"
 *		 2. Execute "snx_spi_ctl" 
 * NOTE:
 *		 This example is used to control sonix spi in gpio mode.
 *		 Please make sure the device attached is gpio mode. (device.c)
 */

#include "snx_spi.h"

/*-----------------------------------------------------------------------------
 * Example Code Configuration
 *----------------------------------------------------------------------------*/
#define SPI_DEV_NAME 		"/dev/spidev0.0"		/* /dev/spidev1.0" */
#define SPI_MODE 			"SPI"					/* SPI */
#define SPI_OP_MODE			"WRRD"					/* RD, WR, WRRD */ 
 			
#define SPI_BITS_PER_WORD   8						/* 8 , 16, 32 , 64 */ 
													/*how many bits would be xfer in one chip select period*/

#define SPI_MAX_SPEED    	1000000 				/* 4MHz (MAX: 6MHz) */
#define SPI_DATA_NUM		8						/* How many data would be xfer */
#define SPI_DATA 			"0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08"				
							/* one byte by one byte, splitted by ",*/


static int simple_strtoi(const char *cp, int base)
{
	int retval = 0;
	int value;

	if(*cp == '0') {
		cp++;
		if((*cp == 'x') && isxdigit(cp[1])) {
			base = 16;
			cp++;
		}
		if(!base) {
			base = 8;
		}
	}
	if(!base) {
		base = 10;
	}
	while(isxdigit(*cp) && (value = isdigit(*cp) ? (*cp - '0') : 
		(islower(*cp) ? toupper(*cp) : *cp) - 'A' + 10 ) < base) {
		retval = retval * base + value;
		cp++;
	}

	return retval;
}

static int data_split(const char * origin, unsigned char *data)
{
	int cnt = 0;
	char *p;
	unsigned char *p_data = data;
	const char *split = ",";
	
	p = strtok(origin, split);
	while (p != NULL) {
		printf("got data: %s\n", p);
		*p_data = (unsigned char) simple_strtoi(p, 16);
		cnt++;
		p_data ++;
		p = strtok(NULL, split);
	}
	return cnt;
}

/*-----------------------------------------------------------------------------
 * Example Code Flow
 *----------------------------------------------------------------------------*/

int main(int argc, char *argv[])   
{
	char *data_str;
	int ret = 0;
	int fd;
	int buf_size;
	int xfer_size;
	struct spi_ioc_transfer *xfer;
	unsigned char *buf;
	unsigned char *rx_buf;
	unsigned char mode;
	unsigned char bits;
	unsigned int speed;
	int op_mode;
	
	/*--------------------------------------------------------
		Config setup
	---------------------------------------------------------*/

	fd = open(SPI_DEV_NAME, O_RDWR);
	if (fd < 0) {
		printf("[SNX-SPI] can't open device\n");
		goto EXIT;
	}

	ret = ioctl (fd, SPI_IOC_RD_MODE, &mode );
	ret += ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	ret += ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1) {
		printf("[SNX-SPI] Get info error\n");
		goto EXIT;
	}


	if (strcmp(SPI_MODE, "SPI") == 0) {
		bits = SPI_BITS_PER_WORD;
		buf_size = SPI_DATA_NUM;
	} else {
		printf("[SNX-SPI] Wrong SPI OP MODE\n");
		goto EXIT;
	}


	if (strcmp(SPI_OP_MODE, "RD") == 0) {
		op_mode = SPI_RD_OP_MODE;
		xfer_size = (sizeof(struct spi_ioc_transfer) * 1);
		speed = (SPI_MAX_SPEED);
		
	} else if (strcmp(SPI_OP_MODE, "WR") == 0) {
		op_mode = SPI_WR_OP_MODE;
		xfer_size = sizeof(struct spi_ioc_transfer) * 1;
		speed = SPI_MAX_SPEED;

	} else if (strcmp(SPI_OP_MODE, "WRRD") == 0) {
		op_mode = SPI_WRRD_OP_MODE;
		xfer_size = sizeof(struct spi_ioc_transfer) * 1;
		speed = SPI_MAX_SPEED;

	} else {
		printf("[SNX-SPI] Wrong SPI OP MODE\n");
		goto EXIT;
	}

	ret = ioctl (fd, SPI_IOC_WR_MODE, &mode );
	ret += ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	ret += ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1) {
		printf("[SNX-SPI] Write info error\n");
		goto EXIT;
	}


	buf = malloc(buf_size);
	xfer = malloc(xfer_size);

	memset(xfer , 0 , xfer_size);
	memset(buf , 0 , buf_size);

	data_str = malloc(strlen(SPI_DATA));
	strcpy(data_str, SPI_DATA);

	ret = data_split(data_str, buf);
	if ((ret != buf_size) && (op_mode != SPI_RD_OP_MODE)) {
		printf("[SNX-SPI] Wrong number of data(%d, %d)\n", ret, op_mode);
		goto EXIT;
	}

	switch (op_mode) {
		case SPI_RD_OP_MODE:
			xfer->len = SPI_DATA_NUM;
			xfer->rx_buf = (unsigned long) buf ;
			ioctl ( fd , SPI_IOC_MESSAGE(1), xfer );
			break;
		case SPI_WR_OP_MODE:
			xfer->len = SPI_DATA_NUM;
			xfer->tx_buf = (unsigned long) buf ;
			ioctl ( fd , SPI_IOC_MESSAGE(1), xfer );
			break;
		case SPI_WRRD_OP_MODE:
				rx_buf = malloc(buf_size);
				memset(rx_buf, 0, buf_size);

				xfer->len = SPI_DATA_NUM;
				xfer->tx_buf = (unsigned long) buf;
				xfer->rx_buf = (unsigned long) rx_buf;
				ioctl ( fd , SPI_IOC_MESSAGE(1), xfer );
				
				{
					unsigned char *p_buf = rx_buf;
					int i;
					printf("[SNX-SPI] RX Data:\n");
					for (i = 0; i < buf_size; i++) {
						printf("Data[%d]: 0x%02x\n", i, *p_buf);
						p_buf++;
					}
				}
			break;
		default:
			printf("[SNX-SPI] Wrong SPI OP MODE\n");
			break;
	}

	{
		unsigned char *p_buf = buf;
		int i;
		printf("[SNX-SPI] Rsult:\n");
		for (i = 0; i < buf_size; i++) {
			printf("Data[%d]: 0x%02x\n", i, *p_buf);
			p_buf++;
		}
	}
	
		
EXIT:
	if(data_str)
		free(data_str);
	if(xfer) {
		free(xfer);
	}
	if(buf) {
		free(buf);
	}
	if(rx_buf) {
		free(rx_buf);
	}
	if(fd) {
		close(fd);
	}

    return 0;
}
