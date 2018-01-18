#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h> 	
#include "snx_gpio.h"
static void usage(FILE * fp, int argc, char ** argv) {   
    fprintf(fp, "Usage: %s [options]/n\n"   
        "Options:\n"
        "-h Print this message\n"
#if (CONFIG_GPIO_04_ENABLE && CONFIG_GPIO_06_ENABLE)
        "-n GPIO number 0:GPIO[0] 1:GPIO[1] 2:GPIO[2] 3:GPIO[3] 4:GPIO[4] 6:GPIO[6]\n"
#else
		"-n GPIO number 0:GPIO[0] 1:GPIO[1] 2:GPIO[2] 3:GPIO[3]\n"
#endif
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
  int index, num = -1,mode = 0, val = 1;


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
  #if (CONFIG_GPIO_04_ENABLE && CONFIG_GPIO_06_ENABLE)
  if (num < 0 || num >6 || mode <0 || mode >1 || num == 5)
  #else
  if (num < 0 || num >3 || mode <0 || mode >1 )
  #endif
  {
    usage(stderr, argc, argv);   
		exit(1);  
  }     
  
  gpio_pin_info info;

	snx_gpio_open();

	info.pinumber = num;
	info.mode = mode;
	info.value = val;
	if(snx_gpio_write(info) == GPIO_FAIL)
	  printf ("write gpio%d error\n",num);
	if(mode == 0)
	{
	  snx_gpio_read(&info);
	 printf ("%d\n",info.value);
	  return info.value;
	}
	
	snx_gpio_close();

   return 2;   
         
}
