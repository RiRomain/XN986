#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#define DEVICE_NAME			"/dev/snx_timer_test"

static void usage(char *name)
{
	printf("\tUsage:\n\n");
	printf("\t%s [1|2|3] \n\n", name);
	printf("\tOptions\n");
  printf("\t1 timer_measure_test  \n");
  printf("\t2 timer_alarm_test \n");
  printf("\t3 timer_measure_and_alarm_test \n");
  printf("\t4 timer_cpu_clock_test \n");
  
}

int main(int argc, char *argv[])
{
	int fd = -1;
	int retval = 0;
	int cmd;

	if(argc != 2 || (atoi(argv[1]) < 1) || (atoi(argv[1]) > 4))
	{
    usage (argv[0]);
  	return -1;
  }  

	fd = open(DEVICE_NAME, O_RDWR, 0);
	if(fd < 0)
	{
		printf("open the file /dev/snx_timer_test failed\n");
		return -1;
	}

	cmd = atoi(argv[1]);
	printf("command = %d\n", cmd);


	write(fd, &cmd, sizeof(cmd));
	
	if(close(fd))
	{
		printf("close the file /dev/snx_timer_test failed.\n");
		return -1;
	}

	return 0;
}
