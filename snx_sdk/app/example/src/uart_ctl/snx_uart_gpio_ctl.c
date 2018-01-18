#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h> 
#include <sys/ioctl.h>
#include "snx_gpio.h"

#define UART_CMD_DIR		0x10
#define UART_CMD_READ		0x11
#define UART_CMD_WRITE		0x12

#define UART_DIR_IN			0x0
#define UART_DIR_OUT		0x1

#define UART_PIN_TX			0x0
#define UART_PIN_RX			0x1

#define UART1_DEVICE	"/dev/ttyS0"
#define UART2_DEVICE	"/dev/ttyS1"

static void usage(FILE * fp, int argc, char ** argv) {   
    fprintf(fp, "Usage: %s [options]/n\n"   
        "Options:\n"
        "-h Print this message\n"
		"-n GPIO number 0:UART1[0] 1:UART1[1] 2:UART2[0] 3:UART2[1]\n"
        "-m mode 1:output 0:input\n"
        "-v if output, 1:high 0:low\n"
        "", argv[0]);   
}
static const char short_options[] = "hn:m:v:";  
static const struct option long_options[] =
{
	{ "help", no_argument, NULL, 'h' },
	{ "Num", required_argument, NULL, 'n' },
	{ "Mode", required_argument, NULL, 'm' },
	{ "Value", required_argument, NULL, 'v' },
	{ 0, 0, 0, 0 }
}; 
int main (int argc, char **argv)  
{ 
	int num = -1,mode = 0, val = 1;
	gpio_pin_info info;
	int fd, ret;

	for (;;)
	{   
		int index;   
		int c;   
		c = getopt_long(argc, argv, short_options, long_options, &index);   
		if (-1 == c)   
		{
    	break;
    }      
		switch (c)
		{   //n:i:d:av:t:
			case 0: /* getopt_long() flag */   
				break;
			case 'h':   
				usage(stdout, argc, argv);   
				exit(0);   
			case 'n':   
				sscanf(optarg, "%d", &num); 
				break;
			case 'm':   
				sscanf(optarg, "%d", &mode); 
				break;   
			case 'v':
				sscanf(optarg, "%d", &val);
				break;
			default:   
				usage(stderr, argc, argv);   
				exit(1);   
		}   
	}
  //printf ("%d %d %d\n",num,mode,val);

	if (num < 0 || num >3 || mode <0 || mode >1 )
	{
		usage(stderr, argc, argv);   
		exit(1);  
	}     
  
  	if (num == 0 || num == 1) {
  		if((fd = open (UART1_DEVICE, O_RDWR)) < 0){
			printf("Open Uart UART1 Device Failed.\n");
			exit(1);
		}
		
		info.pinumber = num;
  	} else {
  		if((fd = open (UART2_DEVICE, O_RDWR)) < 0){
			printf("Open Uart UART1 Device Failed.\n");
			exit(1);
		}
		info.pinumber = num - 2;
  	}

	info.mode = mode;

	if((ret = ioctl (fd, UART_CMD_DIR, &info)) < 0){
			fprintf(stderr, "setup pinumber:%d (%d) direction mode error\n", info.pinumber, num);
			return ret;
	}

	if(mode == 1){
		info.value = !!val;
		if((ret = ioctl (fd, UART_CMD_WRITE, &info)) < 0){
			fprintf(stderr, "write bit error\n");
			return ret;
		}
	}else
	{
		if((ret = ioctl (fd, UART_CMD_READ, &info)) < 0){
			fprintf(stderr, "read bit error\n");
			return ret;
		}

		printf ("%d\n",info.value);
		return !!info.value;
	}
	
	close(fd);

	return 2;   
         
}
