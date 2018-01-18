#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<stdio.h>
#include<unistd.h>
#include<linux/watchdog.h>
#include<time.h>

#define WATCHDOG_DEVICE "/dev/watchdog"
#define WATCHDOG_SUCCESS    0
#define WATCHDOG_FAIL       1
static void usage(char *name)
{
	printf("\tUsage:\n\n");
	printf("\t%s [1|2|3] \n\n", name);
	printf("\tOptions\n");
  	printf("\t1 keepalive test \n");
  	printf("\t2 reset test \n");
  	printf("\t3 get info test \n");
  
}  
int snx_watchdog_reset_test ()
{
    int ret;
    int fd;
    int i;
    unsigned long v, timeout;
    fd = open(WATCHDOG_DEVICE, O_RDONLY);
    if(!fd) {
        printf("Open Failed!\n");
        return WATCHDOG_FAIL;
    }

	/* Set Timeout */
    timeout = 5; // in watchdog driver default is 5s
    ret = ioctl(fd, WDIOC_SETTIMEOUT,  &timeout);
	
	/* Enable WDT */
    v = WDIOS_ENABLECARD; // enable watchdog 
    ret = ioctl(fd, WDIOC_SETOPTIONS,  &v);

	/* Start Test */ 
    for(i =0; i < 20; i++)
    {
      sleep(1);
	  /* NO keep alive */
      printf("time %d\n", i);
    }


    close (fd);
	/* Failed if the system did NOT reboot */
    return WATCHDOG_FAIL; 
}
int snx_watchdog_keepalive_test ()
{
    int ret;
    int fd;
    int i;
    unsigned long v,timeout;
    fd = open(WATCHDOG_DEVICE, O_RDONLY);
    if(!fd) {
        printf("Open Failed!\n");
        return WATCHDOG_FAIL;
    }
	/* Set Timeout */
    timeout = 5; // in watchdog driver default is 5s
    ret = ioctl(fd, WDIOC_SETTIMEOUT,  &timeout);
    
	/* Enable WDT */
	v = WDIOS_ENABLECARD; // enable watchdog 
    ret = ioctl(fd, WDIOC_SETOPTIONS,  &v);

	/* Start Test */ 
    for(i =0; i < 10; i++)
    {
      sleep(1);

	  /* keep feeding the dog */
      ret = ioctl(fd, WDIOC_KEEPALIVE,  NULL);

      printf("time %d\n", i);
    }

	/* Disable WDT */
    v = WDIOS_DISABLECARD;
    ret = ioctl(fd, WDIOC_SETOPTIONS,  &v);

    close (fd);
    // to check watchdog disable ok, no reset 
    
	/* sleep over timeout to check if the WDT has been disabled */
	sleep (timeout);
    printf ("check watchdog disable ok\n");
    return WATCHDOG_SUCCESS;
}
int snx_watchdog_getinfo_test ()
{
    int ret;
    int fd;
    int value;
    struct watchdog_info wdt_info;
    fd = open(WATCHDOG_DEVICE, O_RDONLY);
    if(!fd) {
        printf("Open Failed!\n");
        return WATCHDOG_FAIL;
    }
    
	ioctl(fd, WDIOC_GETSUPPORT,  &wdt_info);
    /*Record if the watchdog timer reaches 0 or not.
      0: Does not reach 0
      1: Reaches 0  */
    ioctl(fd, WDIOC_GETSTATUS,  &value); 
    close(fd);
    
	printf ("firmware_version : %d\n",wdt_info.firmware_version);
    printf ("identity : %s\n",wdt_info.identity);
    printf ("value = %d\n",value);
    return WATCHDOG_SUCCESS;
}    
    

int main(int argc, char *argv[])
{
  if (argc == 2)
  {
	/* 
	 * 1 : Keep alive programing test 
	 * 2 : timeout reset test
	 * 3 : get wdt info
	 */
    if (atoi(argv[1]) == 1)
      snx_watchdog_keepalive_test ();
    else if(atoi(argv[1]) == 2) 
     snx_watchdog_reset_test();
    else if(atoi(argv[1]) == 3) 
     snx_watchdog_getinfo_test ();
    else
     usage(argv[0]);             
  }
  else
    usage(argv[0]);
}
