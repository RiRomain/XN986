#include <stdio.h>    // for printf()  
#include <stdlib.h>

#include <unistd.h>   // for pause()  
#include <signal.h>   // for signal()  
#include <string.h>   // for memset()  
#include <sys/time.h> // struct itimeral. setitimer()   
#include <sys/stat.h>
#include <ctype.h>
#include <sys/mman.h>
#include <fcntl.h>

/*
1.	Preview Front-Cam
2.	Preview Rear-Cam
3.	Preview PiP
4.	Playback Front-Cam
5.	Playback Rear-Cam
*/
#define SONIX_EDR_STATUS_NUM	5
#define SONIX_EDR_STATUS_PREVIEW_FRONT_CAM	1
#define SONIX_EDR_STATUS_PREVIEW_REAR_CAM	2
#define SONIX_EDR_STATUS_PREVIEW_PIP		3
#define SONIX_EDR_STATUS_PLAYBACK_FRONT_CAM	4
#define SONIX_EDR_STATUS_PLAYBACK_REAR_CAM	5


#define GPIO_SUCCESS 0
#define GPIO_FAIL    1
#define BUFFER_SIZE		63

int edr_mode = 1;

int snx_gpio_read(int pin_number, int *value)
{
	char buf[BUFFER_SIZE];
	int fd, len;

	memset (buf,0,BUFFER_SIZE);
	sprintf(buf, "echo in > /sys/class/gpio/gpio%d/direction", pin_number);
	system (buf);

	sprintf(buf, "/sys/class/gpio/gpio%d/value", pin_number);
	fd = open(buf, O_RDWR);
	if(fd < 0)
		return GPIO_FAIL;

	read(fd, buf, BUFFER_SIZE);
	sscanf(buf, "%d", value);

	close(fd);

	return GPIO_SUCCESS;
}


void sigalrm_fn(int sig)
{


	  
	alarm(1);
	
	    
}

int check_if_gpio2_is_pressed(void)
{
	int gpio2_value = 0;
	int gpio2_is_pressed = 0;


//	fprintf(stdout,"%s:%d:\n",__FILE__,__LINE__);fflush(stdout);

	// GPIO reset to default
	snx_gpio_read (2, &gpio2_value);

	if(gpio2_value==1){
		gpio2_is_pressed=0;
	}
	else
	if(gpio2_value==0){
		gpio2_is_pressed=1;
	}	

	return gpio2_is_pressed;
}

int snx_edr_status_change(int *status)
{
	(*status)++;
	if((*status)>SONIX_EDR_STATUS_NUM){
		(*status)=1;
	}

	return 0;
}

int snx_edr_process(int status)
{
	system("sync");
	system("sync");
	system("sync");
  edr_mode = 1;

	system("killall -15 snx_edr");
	system("killall -9 snx_edr");
	system("killall -9 tv");
  system("echo 1 > /proc/sys/vm/drop_caches");
  if (edr_mode == 0)
  {
   system("killall -9 snx_m2m_one_stream_with_rc");
	 system("killall -15 SONiX_UVC_TestAP");
	 system("killall -9 SONiX_UVC_TestAP");
	}
  
  if(status==SONIX_EDR_STATUS_PREVIEW_FRONT_CAM){
		fprintf(stdout,"\nPreview Front-Cam\n");fflush(stdout);
		system("snx_m2m_one_stream_with_rc /media/mmcblk0p1/video1.h264&");
    system("SONiX_UVC_TestAP /dev/video5 -c 30 -f H264 -r -s 1280x720 --fr 30 -n 4 --xuset-br 4000000 &");
    //system("snx_uvc_record -W 1280 -H 720 -c h264 -s 1 -o /media/mmcblk0p1/uvc.h264 -B 2 uvcvideo &");
		system("tv -W 320 -H 480 -x 0 -y 0 -n 320 -v 480 -R 30 &");
	}

	else if(status==SONIX_EDR_STATUS_PREVIEW_REAR_CAM){
		fprintf(stdout,"\nPreview Rear-Cam\n");fflush(stdout);
    
    system("snx_edr -W 320 -H 240 -c jpeg -s 1 -x 0 -y 0 -n 320 -v 480 -o lcd -B 2 uvcvideo &");
    
				
	}
	else if(status==SONIX_EDR_STATUS_PREVIEW_PIP){
		fprintf(stdout,"\nPreview PiP\n");fflush(stdout);
    system("snx_edr -W 320 -H 240 -c jpeg -s 2 -x 160 -y 0 -n 160 -v 120 -o lcd -B 2 uvcvideo &");
    usleep(500000);
		system("tv -W 320 -H 480  -x 0 -y 0 -n 320 -v 480 -R 30 &");
		
	}
	
	else if(status==SONIX_EDR_STATUS_PLAYBACK_FRONT_CAM){
    if (edr_mode != 0)
    {
      system("killall -9 snx_m2m_one_stream_with_rc");
	    system("killall -15 SONiX_UVC_TestAP"); 
      system ("sync"); 
		}
    fprintf(stdout,"\nPlayback Front-Cam\n");fflush(stdout);
		system("snx_edr -W 1280 -H 720 -c h264 -s 4 -x 0 -y 0 -n 320 -v 480 -o lcd -B 2 /media/mmcblk0p1/video1.h264 &");		
	}

	else if(status==SONIX_EDR_STATUS_PLAYBACK_REAR_CAM){
		fprintf(stdout,"\nPlayback Rear-Cam\n");fflush(stdout);
    if (edr_mode != 0)
    {
      system("killall -9 snx_m2m_one_stream_with_rc");
	    system("killall -15 SONiX_UVC_TestAP"); 
      system ("sync"); 
		}
		system("snx_edr -W 1280 -H 720 -c h264 -s 4 -x 0 -y 0 -n 320 -v 480 -o lcd -B 2 /media/mmcblk0p1/uvc.h264 &");
	}
	
	
	return 0;
}

int main(int argc, char **argv)  
{
	int gpio2_pressed=0;
	int old_gpio2_pressed;
	int snx_edr_status=1;
  if (argc == 2)
  {
    edr_mode = atoi (argv[1]);
  }
	system ("echo 2 > /sys/class/gpio/export");
	//Initial GPIO2 PIN
//	system("echo 2 > /sys/class/gpio/export 1>/dev/null 2>/dev/null");
	
	
  	// set signal handlers 
	signal(SIGALRM, sigalrm_fn);
	alarm(1);
	
	snx_edr_process(snx_edr_status);	
	
	while(1){
//		fprintf(stdout,"%s:%d:\n",__FILE__,__LINE__);fflush(stdout);
		gpio2_pressed=check_if_gpio2_is_pressed();
		if(gpio2_pressed==1){
//			printf("\n");
//			printf("GPIO2 button is Pressing!!\n");
			old_gpio2_pressed=gpio2_pressed;
		}
		else
		if(gpio2_pressed==0){
			if(old_gpio2_pressed==1){
				printf("\n");
				printf("GPIO2 button is Pressed!!\n");
				snx_edr_status_change(&snx_edr_status);
				snx_edr_process(snx_edr_status);
			}
			old_gpio2_pressed=gpio2_pressed;
		}

		usleep(200000);
	}

	return 0;
}

