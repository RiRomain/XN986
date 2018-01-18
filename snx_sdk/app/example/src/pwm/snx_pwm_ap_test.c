#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <getopt.h>
#include "snx_pwm.h"
#define ONE_SEC   1000000000  

struct option long_options[] = {
	{ "pwmID", 1,   NULL,    'p'     },  
	{ "type" , 1,   NULL,    't'     },  
	{ "level", 1,   NULL,    'l'     },  
	{ NULL   , 0,   NULL,     0      },  
};

static void usage(char *name)
{
	printf("\tUsage:\n\n");
	printf("\t\t %s -p 0 -t [0|1|2|3] -l 5\n\n", name);
	printf("\tOptions\n");
	printf("\t-pwmID  (-p: 0 | 1),  0 means select pwm1,1 means select pwm2\n");
	printf("\t-type (-t : 0|1|2|3|4), 0 means change duty ,\n");
  printf("\t                        1 means duty = 1/2 period,\n");
  printf("\t                        2 means read gpio,\n");
  printf("\t                        3 means inverse,\n");
  printf("\t                        4 means disable pwm\n");
	printf("\t-level  (-l): when test_type is change duty,the value should be 1-10,\n");
  printf("\t              value means duty = level / 10 * period \n");
	printf("\n\n");
	exit(-1);
}

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
	while(isxdigit(*cp) && (value = isdigit(*cp) ? *cp - '0' : 
		(islower(*cp) ? toupper(*cp) : *cp) - 'A' + 10 ) < base) {
		retval = retval * base + value;
		cp++;
	}

	return retval;
}



int main(int argc, char *argv[])
{
	int fd = -1;
	int retval = 0, value=2;
	int id,level,type;
	struct pwm_config_param pwm_param;
	
	while((retval = getopt_long (argc, argv, "p:t:l:", long_options, NULL)) != -1) {
		switch(retval) {
		case 'p':
			id = simple_strtoi(optarg, 0);
			if(id != SNX_PWM_1 && id != SNX_PWM_2)
				usage(argv[0]);
			break;
			
		case 't':
			type = simple_strtoi(optarg, 0);
			break;
		
		case 'l':
			level = simple_strtoi(optarg, 0);
			break;

		default:
			usage(argv[0]);
		}
	}


	if(type == 0){
		if(level < 1 || level > 10)
			usage(argv[0]);
	} else if (type == 1){
		if(level < 1 || level > 5)
			usage(argv[0]);
	} else if(type > 5 || type < 0)
		usage(argv[0]);
	
	fd = open(PWM_DEVICE, O_RDWR, 0);
	if(fd < 0)
	{
		printf("open the file %s failed\n",PWM_DEVICE);
		return PWM_FAIL;
	}
		
	if(ioctl(fd, SONIX_PWM_REQUEST, &id))
	{
		printf("request failed\n");
		ioctl(fd, SONIX_PWM_FREE, &id);
		close(fd);
		return PWM_FAIL;
	}
 
  
	if(type == 0){
		pwm_param.period_ns = ONE_SEC;// 1s
		pwm_param.duty_ns = level * ONE_SEC/10;// (level)*100ms
	}else if(type == 1){
		printf("the level value is %d\n",level);
		pwm_param.period_ns = ONE_SEC ; // 1s/ level
		pwm_param.duty_ns = pwm_param.period_ns / 2; // 50%
	}else if(type == 2){
		if(ioctl(fd, SONIX_PWM_READ, &value))
		{
			printf("read gpio mode fail\n");
		}
		printf("please check pwm gpio!  the read value is %d\n",value);
	} else if(type == 3){
		if(ioctl(fd, SONIX_PWM_INVERSE, &value))
		{
			printf("inverse fail\n");
		}
		printf("please check oscillograph\n");
	} else if(type == 4){
		if(ioctl(fd, SONIX_PWM_DISABLE, &id))
			printf("disable failed\n");
	}

	if(type == 1 || type == 0){
		if(ioctl(fd, SONIX_PWM_DISABLE, &id))
			printf("disable failed\n");
		
		if(ioctl(fd, SONIX_PWM_CONFIG, &pwm_param))
			printf("config failed\n");

		if(ioctl(fd, SONIX_PWM_ENABLE, &id))
			printf("enable failed\n");
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

