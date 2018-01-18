/**
 *
 * SONiX SDK Example Code
 * Category: SPI
 * File: snx_spi_gpio.c
 * Usage: 
 *		 1. Make sure your platform has attached spi device, "spidev"
 *		 2. Execute "snx_spi_gpio -h" for more information
 * NOTE:
 *		 This example is used to control sonix spi in gpio mode.
 *		 Please make sure the device attached is gpio mode. (device.c)
 */

#include "snx_spi.h"

/*-----------------------------------------------------------------------------
 * Example Code Configuration
 *----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
 * Example Code Flow
 *----------------------------------------------------------------------------*/

static const char short_options[] = "hd:p:t:v:";

static const struct option long_options[] = {
    {"help", no_argument, NULL, 'h'},
	{"device", required_argument, NULL, 'd'},
    {"pin", required_argument, NULL, 'p'},
	{"type", required_argument, NULL, 't'},
	{"value", required_argument, NULL, 'v'},
	
    {0, 0, 0, 0}
};

static void usage(FILE * fp, int argc, char ** argv) {   
    fprintf(fp, "Usage: %s [options]/n\n"   
		"Options:\n"
		"-h                 Print this message\n"
        "-d | --device      /dev/spidev0.0, /dev/spidev1.0 \n"
		"-p | --pin         clk, cs, tx, rx\n"
		"-t | --type        in, out (default: in)\n"
		"-v | --value       0 or 1 (default: 0)\n"

		"", argv[0]);   
}


int main(int argc, char *argv[])   
{
	char *pin_name = NULL;
	char *dev_name = NULL;
	char *type = NULL;
	int mode = 0;
	unsigned char data=0;
	int fd;

	/*--------------------------------------------------------
		Option Value
	---------------------------------------------------------*/
	for (;;)
	{   
		int index;   
		int c;   
		c = getopt_long(argc, argv, short_options, long_options, &index);   

		if (-1 == c)
			break;

		switch (c) {   
			case 0: /* getopt_long() flag */   
				break;
			case 'h':   
				usage(stdout, argc, argv);   
				exit(EXIT_SUCCESS);
			case 'd':
				if (strlen(optarg) > 0) {
					dev_name = malloc(strlen(optarg));
					strcpy(dev_name, optarg);
				}
			case 'p':
				if (strlen(optarg) > 0) {
					pin_name = malloc(strlen(optarg));
					strcpy(pin_name, optarg);
				}
				break;
			case 't':
				if (strlen(optarg) > 0) {
					type = malloc(strlen(optarg));
					strcpy(type, optarg);
				}
				break;
			case 'v':
				switch(*optarg) {
					case '0':
						data = 0;
						break;
					case '1':
						data = 1;
						break;
					default:
						usage(stderr, argc, argv);
						goto EXIT;
				}
				break;
			default:
				usage(stderr, argc, argv);
				goto EXIT;
		}
	}
	
	/*--------------------------------------------------------
		Config setup
	---------------------------------------------------------*/

	if (dev_name) {
		struct spi_ioc_transfer xfer;
		unsigned char buf[2];
		unsigned char spi_mode;
		unsigned char bits;
		unsigned int speed;
		int ret = 0;
		
		fd = open(dev_name, O_RDWR);
		if (fd < 0) {
			printf("[SNX-SPI] can't open device\n");
			goto EXIT;
		}

		ret = ioctl (fd, SPI_IOC_RD_MODE, &spi_mode );
		ret += ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
		ret += ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
		if (ret == -1) {
			printf("[SNX-SPI] Get info error\n");
			goto EXIT;
		}

		if (bits != SPI_GPIO_BPW) {
			bits = SPI_GPIO_BPW;
			ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
			/* Set SPI to GPIO mode */
		}

		printf("[SNX-SPI] Current Information \n MODE: %s (%d), bits per word: %d, max speed %d Hz\n", 
			(bits!=SPI_GPIO_BPW)?"SPI":"GPIO", spi_mode, bits, speed);

		memset(&xfer , 0 , sizeof(xfer) );
		memset(buf , 0 , sizeof(buf) );
	
		if (pin_name) {
			if ( strcmp(pin_name, "clk") == 0)
			{
				buf[0] = SPI_GPIO_CLK_PIN;
			}
			else if ( strcmp(pin_name, "cs") == 0)
			{
				buf[0] = SPI_GPIO_FS_PIN;
			}
			else if ( strcmp(pin_name, "tx") == 0)
			{
				buf[0] = SPI_GPIO_TX_PIN;
			}
			else if ( strcmp(pin_name, "rx") == 0)
			{
				buf[0] = SPI_GPIO_RX_PIN;
			} else {
				printf("[SNX-SPI] WRONG pin number\n");
				goto EXIT;
			}
		}

		if (type) {
			if (strcmp(type, "in") == 0)
				mode = SSP_GPIO_INPUT;
			else if (strcmp(type, "out") == 0)
				mode = SSP_GPIO_OUTPUT;
			else {
					printf("[SNX-SPI] WRONG type\n");
					usage(stderr, argc, argv);
					goto EXIT;
				}
		}
		
		
		xfer.len = 2;
		
		if (mode == SSP_GPIO_INPUT) {
			xfer.rx_buf = (unsigned long) buf ;
		} else if (mode == SSP_GPIO_OUTPUT) {
			buf[1] = data;
			xfer.tx_buf = (unsigned long) buf ;
		}
		
		ioctl ( fd , SPI_IOC_MESSAGE(1), &xfer );

		
		printf("[SNX-SPI] Dev: %s, PIN: %s, GPIO mode: %d, Value: %d\n", dev_name, pin_name, mode, buf[1]);
	}
	else {
		usage(stderr, argc, argv);
		goto EXIT;
	}
	
		
EXIT:
	if(dev_name) {
		free(dev_name);
	}
	if(pin_name) {
		free(pin_name);
	}
	if(type) {
		free(type);
	}
	if(fd) {
		close(fd);
	}

    return 0;
}