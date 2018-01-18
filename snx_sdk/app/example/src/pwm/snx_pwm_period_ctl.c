#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include "snx_pwm.h"
#define ONE_MSEC   1000000 
static void usage(char *name)
{   
  printf ("please input: pwm_id duty time periodt time\n");
  
  printf ("pwm_id:the value must be 0 or 1, 0 means use pwm2,1 means use pwm1\n");
	printf ("duty :duty keep time(millisecond ) must > 0\n");
	printf ("period :period time (millisecond ) must > 0\n");
}
int main(int argc, char *argv[])
{
	int fd = -1;
	int retval = 0;
	int id;
	struct pwm_config_param pwm_param;

	if(argc != 4){
		usage (argv[0]);
		return PWM_FAIL;
	}

	fd = open(PWM_DEVICE, O_RDWR, 0);
	if(fd < 0)
	{
		printf("open the file %s failed\n",PWM_DEVICE);
		return PWM_FAIL;
	}
  if (atoi(argv[1]) != 0 && atoi(argv[1]) != 1)
  {
    usage (argv[0]);
		return PWM_FAIL;
  }
	id = atoi(argv[1]);
  if (atoi(argv[2]) > 0 && atoi(argv[3]) > 0)
  {
    if (atoi(argv[2]) >= atoi(argv[3]))
    {
      pwm_param.duty_ns= atoi(argv[3]) * ONE_MSEC;
      pwm_param.period_ns= atoi(argv[2]) * ONE_MSEC;
    }
    else
    {
      pwm_param.duty_ns= atoi(argv[2]) * ONE_MSEC;
      pwm_param.period_ns= atoi(argv[3]) * ONE_MSEC;
    }
  }
  else 
  {
    usage (argv[0]);
    return PWM_FAIL;
  }  
	if(id != SNX_PWM_1 && id != SNX_PWM_2){
		printf("id = %d\n", id);
		close(fd);
		return PWM_FAIL;
	}

	if(ioctl(fd, SONIX_PWM_REQUEST, &id))
	{
		printf("request failed\n");
		close(fd);
		return PWM_FAIL;
	}

	if(ioctl(fd, SONIX_PWM_DISABLE, &id))
	{
		printf("disable failed\n");
		close(fd);
		return PWM_FAIL;
	}

	if(ioctl(fd, SONIX_PWM_CONFIG, &pwm_param))
	{
		printf("config failed\n");
		close(fd);
		return PWM_FAIL;
	}

	if(ioctl(fd, SONIX_PWM_ENABLE, &id))
	{
		printf("enable failed\n");
		close(fd);
		return PWM_FAIL;
	}

	if(ioctl(fd, SONIX_PWM_FREE, &id))
	{
		printf("free failed\n");
		close(fd);
		return PWM_FAIL;
	}
	
	if(close(fd))
	{
		printf("close the file /dev/test_timer failed.\n");
		return PWM_FAIL;
	}

	return PWM_SUCCESS;
}
