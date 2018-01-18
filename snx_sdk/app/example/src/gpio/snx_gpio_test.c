#include <stdio.h>	 
#include <stdlib.h>   
#include <string.h>   
#include <assert.h>   
#include <getopt.h> 			/* getopt_long() */   
#include <fcntl.h>				/* low-level i/o */   
#include <unistd.h>   
#include <errno.h>	 
#include <malloc.h>  
#include <signal.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>	
#include <sys/types.h>	 
#include <sys/time.h>	
#include <sys/mman.h>	
#include <sys/ioctl.h>	 
#include <asm/types.h>	
#define GPIO_SUCCESS 0
#define GPIO_FAIL    1

#define BUFFER_SIZE		63
char buf[BUFFER_SIZE];

int snx_gpio_open(int pin_number)
{
	int fd, len;
	memset (buf,0,BUFFER_SIZE);

	fd = open("/sys/class/gpio/export", O_RDWR);
	if(fd < 0)
		return GPIO_FAIL;

	len = sprintf(buf, "%d", pin_number);	
	write(fd, buf, len);
	close(fd);

	return GPIO_SUCCESS;
}

int snx_gpio_read(int pin_number, int *value)
{
	int fd, len;
	memset (buf,0,BUFFER_SIZE);

	sprintf(buf, "/sys/class/gpio/gpio%d/direction", pin_number);
	fd = open(buf, O_RDWR);
	if(fd < 0)
		return GPIO_FAIL;
	write(fd, "in", sizeof("in"));
	close(fd);

	sprintf(buf, "/sys/class/gpio/gpio%d/value", pin_number);
	fd = open(buf, O_RDWR);
	if(fd < 0)
		return GPIO_FAIL;

	read(fd, buf, BUFFER_SIZE);
	sscanf(buf, "%d", value);

	close(fd);

	return GPIO_SUCCESS;
}

int snx_gpio_write(int pin_number, int value)
{
	int fd, len;
	memset (buf,0,BUFFER_SIZE);

	sprintf(buf, "/sys/class/gpio/gpio%d/direction", pin_number);
	fd = open(buf, O_RDWR);
	if(fd < 0)
		return GPIO_FAIL;
	write(fd, "out", sizeof("out"));
	close(fd);

	sprintf(buf, "/sys/class/gpio/gpio%d/value", pin_number);
	fd = open(buf, O_RDWR);
	if(fd < 0)
		return GPIO_FAIL;
	len = sprintf(buf, "%d", value);
	write(fd, buf, len);

	close(fd);

	return GPIO_SUCCESS;
}


int snx_gpio_select(int pin_number, int type, int time)
{
	int ret, fd, len;
  
  sprintf(buf, "/sys/class/gpio/gpio%d/direction", pin_number);
	fd = open(buf, O_RDWR);
	if(fd < 0)
		return GPIO_FAIL;
	write(fd, "in", sizeof("in"));
	close(fd);
	memset (buf,0,BUFFER_SIZE);

	fd_set fds;
	struct timeval timeout;
  /* set interrupt edge mode */
	sprintf(buf, "/sys/class/gpio/gpio%d/edge", pin_number);
	fd = open(buf, O_RDWR);
	if(fd < 0)
		return GPIO_FAIL;
  memset (buf,0,BUFFER_SIZE);
  if (type == 0) 
	len = sprintf(buf, "%s", "none");
  else if (type == 1) 
	 len = sprintf(buf, "%s", "rising"); 
  else if (type == 2) 
	 len = sprintf(buf, "%s", "falling");
  else if (type == 3) 
	 len = sprintf(buf, "%s", "both");  
	write(fd, buf, len);
	close(fd);
  
  /* select interrupt */
	timeout.tv_sec = time/1000;
	timeout.tv_usec = (time%1000)*1000;
  memset (buf,0,BUFFER_SIZE);
	sprintf(buf, "/sys/class/gpio/gpio%d/value", pin_number);
	fd = open(buf, O_RDWR);
  if (fd < 0)
  { 
    printf ("open error");
    return GPIO_FAIL;
  }

	FD_ZERO(&fds); 
	FD_SET(fd,&fds);

	ret = select(fd+1,&fds,NULL,NULL,&timeout); 
	close(fd);

	return GPIO_SUCCESS;
}


int snx_gpio_close(int pin_number)
{
	int fd, len;
	memset (buf,0,BUFFER_SIZE);

	fd = open("/sys/class/gpio/unexport", O_RDWR);
	if(fd < 0)
		return GPIO_FAIL;

	len = sprintf(buf, "%d", pin_number);	
	write(fd, buf, len);
	close(fd);

	return GPIO_SUCCESS;
}





static int snx_gpio_read_tst(int pin_number)
{
	int value;
	struct timeval timeout;

	if (snx_gpio_open(pin_number) != GPIO_SUCCESS)
  {
    printf ("snx_gpio_open fail\n");
    return  GPIO_FAIL;
  }

	fprintf(stderr, "read:\n");
	if (snx_gpio_read(pin_number, &value) != GPIO_SUCCESS)
  {
    printf ("snx_gpio_read fail\n");
    return  GPIO_FAIL; 
  }
	printf ("value = %d\n",value);

	if (snx_gpio_close(pin_number) != GPIO_SUCCESS)
  {
    printf ("snx_gpio_close fail\n");
    return  GPIO_FAIL; 
  }
	return GPIO_SUCCESS;
}

static int snx_gpio_write_tst(int pin_number, int value)
{

	if (snx_gpio_open(pin_number) != GPIO_SUCCESS)
  {
    printf ("snx_gpio_open fail\n");
    return  GPIO_FAIL;
  }
	fprintf(stderr, "write: %d\n",value);
	if (snx_gpio_write(pin_number, value) != GPIO_SUCCESS)
  {
    printf ("snx_gpio_write fail\n");
    return  GPIO_FAIL; 
  }

	if (snx_gpio_close(pin_number) != GPIO_SUCCESS)
  {
    printf ("snx_gpio_close fail\n");
    return  GPIO_FAIL; 
  }
	return GPIO_SUCCESS;
}

static int snx_gpio_interrupt_tst(int pin_number, int c, int type, unsigned long t)
{
	if (snx_gpio_open(pin_number) != GPIO_SUCCESS)
  {
    printf ("snx_gpio_open fail\n");
    return  GPIO_FAIL;
  }

	while(c){
		if (snx_gpio_select(pin_number, type, t) != GPIO_SUCCESS)
    {
      printf ("snx_gpio_select fail\n");
      return  GPIO_FAIL;
    }
		fprintf(stderr, "interrupt occur\n");
		c--;
	}
	if (snx_gpio_close(pin_number) != GPIO_SUCCESS)
  {
    printf ("snx_gpio_close fail\n");
    return  GPIO_FAIL; 
  }
	return GPIO_SUCCESS;
}


static void usage(FILE * fp, int argc, char ** argv) {   
    fprintf(fp, "Usage: %s [options]/n\n"   
        "Options:\n"
        "-h Print this message\n"
        "-n GPIO number\n"
        "-i Interrupt edge set 0: none 1: rasing 2: falling 3: both \n"
        "-d Direction set \"in\" or \"out\" \n"
        "-v Value 1:high 0: low\n"
        "-c Do counts\n"
        "-t Time(ms)\n"
        "", argv[0]);   
}   

static const char short_options[] = "hn:i:d:v:c:t:";   
static const struct option long_options[] =
{
	{ "help", no_argument, NULL, 'h' },
	{ "Num", required_argument, NULL, 'n' },
	{ "Interrupt", required_argument, NULL, 'i' },
	{ "Direction", required_argument, NULL, 'd' },
	{ "Value", required_argument, NULL, 'v' },
	{ "Count", required_argument, NULL, 'c' },
	{ "Time", required_argument, NULL, 't' },
	{ 0, 0, 0, 0 }
}; 


int main(int argc, char **argv)  
{  
	int index, num = 0, val = 1, time = 1,  cout = 1;
	int type = 0;
  char *dir ;  

	for (;;)
	{   
		int index;   
		int c;   
		c = getopt_long(argc, argv, short_options, long_options, &index);   
		if (-1 == c)   
			break;   
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
			case 'i':   
        sscanf(optarg, "%d", &type); 
				break;   
			case 'd':
				dir = optarg;
				break;
			case 'v':
				sscanf(optarg, "%d", &val);
				break;
			case 'c':
				sscanf(optarg, "%d", &cout);
				break;				
			case 't':
				sscanf(optarg, "%d", &time);
				break;
			default:   
				usage(stderr, argc, argv);   
				exit(1);   
		}   
	}   

	if(strcmp("in", dir) == 0)
		snx_gpio_read_tst(num);
	else if(strcmp("out", dir) == 0)
		snx_gpio_write_tst(num, val);
	else
		snx_gpio_interrupt_tst(num, cout, type, time);

	return GPIO_SUCCESS;  

}  

