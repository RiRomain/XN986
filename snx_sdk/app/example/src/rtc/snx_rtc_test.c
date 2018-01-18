#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include <linux/rtc.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define RTC_DEVICE    "/dev/rtc0"
#define RTC_SUCCESS   0
#define RTC_FAIL      1

void usage (void)
{
	printf ("RTC Test Application.\n");
	printf ("0) Exit\n");
	printf ("1) snx_rtc_timer_test\n");
	printf ("2) snx_rtc_alarm_interrupt_test\n");
	printf ("3) snx_rtc_wakeup_interrupt_test\n");
	printf ("> ");
}

int snx_rtc_timer_test ()
{
  int ret, i;
	int fd;
  struct rtc_time tm;
  memset (&tm, 0, sizeof (tm));
	tm.tm_year = (2013 - 1900); // set year = tm_year + 1900
	tm.tm_mon = (12 - 1);      // set mon = tm_mon + 1
	tm.tm_mday = 10;
	tm.tm_hour = 10;
	tm.tm_min = 10;
	tm.tm_sec = 50;
	fd = open (RTC_DEVICE, O_RDONLY);
	if (!fd) {
		printf ("Open Failed!\n");
		return RTC_FAIL;
	}
  if (ioctl (fd, RTC_SET_TIME, &tm))
  {
    printf ("RTC_SET_TIME fail\n");
    return RTC_FAIL;
  }
  // sleep 10s  so output = 2013-12-10 10:11:00
  if (ioctl (fd, RTC_RD_TIME, &tm))
  {
		return RTC_FAIL;
  }
  printf ("now set time => (%04d-%02d-%02d %02d:%02d:%02d)\n",
			tm.tm_year + 1900, tm.tm_mon + 1 , tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec);
  for (i = 0; i < 10; i++) {
		sleep (1);
		printf (".");
		fflush (stdout);
	}
  // sleep 10s  so output = 2013-12-10 10:11:00
  if (ioctl (fd, RTC_RD_TIME, &tm))
  {
		return RTC_FAIL;
  }
  printf ("\nafter 10s, now get time => (%04d-%02d-%02d %02d:%02d:%02d)\n",
			tm.tm_year + 1900, tm.tm_mon + 1 , tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec);  
  close (fd);
  return RTC_SUCCESS;  
  
} 

int snx_rtc_alarm_test ()
{
  int ret, i;
	int fd;
  struct rtc_time tm;
  memset (&tm, 0, sizeof (tm));
	tm.tm_year = (2013 - 1900); // set year = tm_year + 1900
	tm.tm_mon = (12 - 1);      // set mon = tm_mon + 1
	tm.tm_mday = 10;
	tm.tm_hour = 10;
	tm.tm_min = 10;
	tm.tm_sec = 50;
	fd = open (RTC_DEVICE, O_RDONLY);
	if (!fd) {
		printf ("Open Failed!\n");
		return RTC_FAIL;
	}
  if (ioctl (fd, RTC_SET_TIME, &tm))
  {
    printf ("RTC_SET_TIME fail\n");
    return RTC_FAIL;
  }
  printf ("now time => (%04d-%02d-%02d %02d:%02d:%02d)\n",
		tm.tm_year + 1900, tm.tm_mon + 1 , tm.tm_mday,
		tm.tm_hour, tm.tm_min, tm.tm_sec);
    
  tm.tm_year = (2013 - 1900); // set year = tm_year + 1900
	tm.tm_mon = (12 - 1);      // set mon = tm_mon + 1
	tm.tm_mday = 10;
	tm.tm_hour = 10;
	tm.tm_min = 10;
	tm.tm_sec = 55;
  if (ioctl (fd, RTC_ALM_SET, &tm))
  {
    printf ("RTC_SET_TIME fail\n");
    return RTC_FAIL;
  }
  if (ioctl (fd, RTC_ALM_READ, &tm))
  {
    printf ("RTC_ALM_READ fail\n");
		return RTC_FAIL;
  }
  printf ("set alarm time is => (%04d-%02d-%02d %02d:%02d:%02d)\n",
			tm.tm_year + 1900, tm.tm_mon + 1 , tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec);
  //enable alarm    
  if (ioctl (fd, RTC_AIE_ON, NULL))
  {
    printf ("RTC_AIE_ON fail\n");
		return RTC_FAIL;
  }
  if (read (fd, &ret, sizeof (unsigned long)) == -1)
		perror ("Read RTC alarm interrupt failed");
	else 
		fprintf (stderr,"done(%d)\n", ret);  
  if (ioctl (fd, RTC_AIE_OFF, NULL))
  {
    printf ("RTC_AIE_OFF fail\n");
		return RTC_FAIL;
  }
  
  if (ioctl (fd, RTC_RD_TIME, &tm))
  {
    printf ("RTC_RD_TIME fail\n");
		return RTC_FAIL;
  }
  // must = 2013-12-10 10:10:55
  printf ("get alarm interrput time == set alarm time  => (%04d-%02d-%02d %02d:%02d:%02d)\n",
			tm.tm_year + 1900, tm.tm_mon + 1 , tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec);  
  close (fd);
  return RTC_SUCCESS;    
  
}

int snx_rtc_wakeup_test ()
{
  int ret, i;
	int fd;
  struct rtc_time tm;
  memset (&tm, 0, sizeof (tm));
	tm.tm_year = (2013 - 1900); // set year = tm_year + 1900
	tm.tm_mon = (12 - 1);      // set mon = tm_mon + 1
	tm.tm_mday = 10;
	tm.tm_hour = 10;
	tm.tm_min = 10;
	tm.tm_sec = 50;
	fd = open (RTC_DEVICE, O_RDONLY);
	if (!fd) {
		printf ("Open Failed!\n");
		return RTC_FAIL;
	}
  if (ioctl (fd, RTC_SET_TIME, &tm))
  {
    printf ("RTC_SET_TIME fail\n");
    return RTC_FAIL;
  }
  if (ioctl (fd, RTC_RD_TIME, &tm))
  {
    printf ("RTC_RD_TIME fail\n");
		return RTC_FAIL;
  }
  printf ("now time => (%04d-%02d-%02d %02d:%02d:%02d)\n",
			tm.tm_year + 1900, tm.tm_mon + 1 , tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec);
      
  tm.tm_year = (2013 - 1900); // set year = tm_year + 1900
	tm.tm_mon = (12 - 1);      // set mon = tm_mon + 1
	tm.tm_mday = 10;
	tm.tm_hour = 10;
	tm.tm_min = 10;
	tm.tm_sec = 55;
  if (ioctl (fd, RTC_WKALM_SET, &tm))
  {
    printf ("RTC_SET_TIME fail\n");
    return RTC_FAIL;
  }
  if (ioctl (fd, RTC_WKALM_RD, &tm))
  {
    printf ("RTC_ALM_READ fail\n");
		return RTC_FAIL;
  }
  printf ("set alarm time is => (%04d-%02d-%02d %02d:%02d:%02d)\n",
			tm.tm_year + 1900, tm.tm_mon + 1 , tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec);
  // enable wakeup    
  if (ioctl (fd, RTC_AIE_ON, NULL))
  {
    printf ("RTC_AIE_ON fail\n");
		return RTC_FAIL;
  }
  if (read (fd, &ret, sizeof (unsigned long)) == -1)
		perror ("Read RTC wakeup interrupt failed");
	else 
		fprintf (stderr,"done(%d)\n", ret);  
  // disable wakeup   
  if (ioctl (fd, RTC_AIE_OFF, NULL))
  {
    printf ("RTC_AIE_OFF fail\n");
		return RTC_FAIL;
  }
  
  if (ioctl (fd, RTC_RD_TIME, &tm))
  {
    printf ("RTC_RD_TIME fail\n");
		return RTC_FAIL;
  }
  // must = 2013-12-10 10:10:55
  printf ("now get wakeup interrupt time == set wakeup time => (%04d-%02d-%02d %02d:%02d:%02d)\n",
			tm.tm_year + 1900, tm.tm_mon + 1 , tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec);  
  close (fd);
  return RTC_SUCCESS;    
  
}


int main (int argc, char *argv[])
{
	int case_id, res;
	unsigned int ret = 0;
	char ch[3];
  usage ();

	memset (ch, 0, sizeof (ch));
	gets (ch);
	case_id = atoi (ch);
    
	if (case_id == 0) {
		printf ("Bye~\n");
		return RTC_FAIL;
	}
  else if (case_id == 1)
    snx_rtc_timer_test ();
  else if (case_id == 2) 
    snx_rtc_alarm_test (); 
  else if (case_id == 3) 
    snx_rtc_wakeup_test (); 
  else
  {
    printf ("cmd error\n");
    return RTC_FAIL;
  }  

		
	return RTC_SUCCESS;
}


