
#include <sys/stat.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	struct stat tst;
	int iFlag = 0;
	/*
	ifconfig eth0 down
	hostapd -B /etc/hostapd_wpa2.conf
	ifconfig wlan0 10.42.0.1 netmask 255.255.255.0
	udhcpd /etc/udhcpd_wpa2.conf

	echo "Start sonix-proj ..."
	/bin/sonix-proj &
	echo

	card-detect &
	*/
	//system("record&");
	int iTarget = -100;
	char chTarget[10];
	
	///itoa(iTarget, chTarget, 10);
	
	//printf("chTarget %s\n", chTarget);
		
	int iupdown = 1;
	while(1)
	{
		if (iTarget == -100)
			iupdown = 1;
		else if (iTarget == 50)
			iupdown = 0;

		if (iupdown)
			iTarget += 25;
		else
			iTarget -= 25;
		
		
			
		
		FILE *fp = fopen("/proc/isp/ae/offset","wb");	
		//printf("chTarget %s\n", chTarget);
		sprintf(chTarget, "%d", iTarget);
		fwrite(chTarget, 1, 10, fp);
		fclose(fp);
		/*if ((stat("/dev/mmcblk0", &tst) != -1) && (stat("/media/mmcblk0", &tst) != -1))
		{
			if(!iFlag)
			{
				printf("card detect\n");
				system("touch /etc/record_flag");
				iFlag = 1;
			}	
		}
		else
		{
				iFlag = 0;
		}*/
			 	
			 	
		usleep(1000*300);
	}
	
	return 0;	
}