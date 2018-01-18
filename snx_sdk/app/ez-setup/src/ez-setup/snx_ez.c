#include <stdio.h>    // for printf()  
#include <stdlib.h>

#include <unistd.h>   // for pause()  
#include <signal.h>   // for signal()  
#include <string.h>   // for memset()  
#include <sys/time.h> // struct itimeral. setitimer()   
#include <sys/stat.h>
#include <ctype.h>

//#include "des.h"
#include "snx_ez_lib.h"
#include "snx_gpio.h"

#define RESET_DEVICE			"/tmp/reset_device"


static volatile int proxy = 8;
static volatile int debug = 0;
static volatile int restart = 0;
static volatile int ez_stat = 0;	// 0: wating start, 1: ready -1: need restart

static volatile sig_atomic_t wifi_status_ok = 1; 

static int wifi_retry_sec = 10;

static char dev[16] = WIFI_DEV;

#define RALINK_CONF	"/etc/Wireless/RT2870AP/RT2870AP.dat"	


void snx_ralink_conf(char *uid, char *key)
{
	char sys_cmd[128];
	
	// Remove ssid line
	// sed -e '/^SSID=/d' /etc/Wireless/RT2870AP/RT2870AP.dat
	memset(sys_cmd, 0x0, 128);
	sprintf(sys_cmd,"sed -i '/^SSID=/d' %s", RALINK_CONF);
	system(sys_cmd);

	// echo "SSID=new_ssid" >> /etc/Wireless/RT2870AP/RT2870AP.dat
	memset(sys_cmd, 0x0, 128);
	sprintf(sys_cmd,"echo -e \"\\n\"SSID=%s >> %s", uid, RALINK_CONF);
	system(sys_cmd);

	// Remove ssid line
	// sed -e '/^WPAPSK=/d' /etc/Wireless/RT2870AP/RT2870AP.dat
	memset(sys_cmd, 0x0, 128);
	sprintf(sys_cmd,"sed -i '/^WPAPSK=/d' %s", RALINK_CONF);
	system(sys_cmd);

	// echo "WPAPSK=key" >> /etc/Wireless/RT2870AP/RT2870AP.dat
	memset(sys_cmd, 0x0, 128);
	sprintf(sys_cmd,"echo -e \"\\n\"WPAPSK=%s >> %s", key, RALINK_CONF);
	system(sys_cmd);
	
	memset(sys_cmd, 0x0, 128);
	sprintf(sys_cmd,"sed -i '/^AuthMode=/d' %s", RALINK_CONF);
	system(sys_cmd);

	memset(sys_cmd, 0x0, 128);
	sprintf(sys_cmd,"echo -e \"\\n\"AuthMode=%s >> %s", "WPA2PSK", RALINK_CONF);
	system(sys_cmd);

	memset(sys_cmd, 0x0, 128);
	sprintf(sys_cmd,"sed -i '/^EncrypType=/d' %s", RALINK_CONF);
	system(sys_cmd);

	memset(sys_cmd, 0x0, 128);
	sprintf(sys_cmd,"echo -e \"\\n\"EncrypType=%s >> %s", "AES", RALINK_CONF);
	system(sys_cmd);
	
	
}
void snx_start_scan(int mode)
{
	char sys_cmd[128];
	int ret=0;

	if(mode & EZ_METHOD_WIFI) {		
#if BCM==1
		char ssid[1024];
		char passwd[1024];

		snx_read_uid((char **)&ssid, sizeof(ssid));
		snx_set_ssid(ssid, (char **)&passwd, NONWRITE);

		memset(sys_cmd, 0x0, 128);
		//dhd_helper iface wlan0 ssid test123 bgnmode bgn chan 3 amode wpawpa2psk emode tkipaes key sonix123
		sprintf(sys_cmd,"dhd_helper iface wlan0 ssid %s bgnmode bgn chan 3 amode wpa2psk emode tkipaes key %s"
				, ssid, passwd);
		ret = system(sys_cmd);

#elif RTL8188==1
		memset(sys_cmd, 0x0, 128);
		sprintf(sys_cmd,"killall hostapd");
		ret = system(sys_cmd);

		memset(sys_cmd, 0x0, 128);
		sprintf(sys_cmd,"killall udhcpd");
		ret = system(sys_cmd);

		memset(sys_cmd, 0x0, 128);
		sprintf(sys_cmd,"hostapd -B %s%s", EZ_DIR, "hostapd_wpa2.conf");
		ret = system(sys_cmd);

#endif
		memset(sys_cmd, 0x0, 128);
		sprintf(sys_cmd,"ifconfig wlan0 up");
		ret = system(sys_cmd);	

		memset(sys_cmd, 0x0, 128);
//		sprintf(sys_cmd,"ifconfig wlan0 5.5.1.253");
		sprintf(sys_cmd,"ifconfig wlan0 10.42.0.1");
		ret = system(sys_cmd);

		memset(sys_cmd, 0x0, 128);
		sprintf(sys_cmd,"udhcpd %sudhcpd.conf", EZ_DIR);
		ret = system(sys_cmd);	

	} //if(mode & EZ_METHOD_WIFI)

}

void snx_stop_scan(int mode)
{
	char sys_cmd[128];
	int ret=0;
	if(mode & EZ_METHOD_WIFI) {

#if RTL8188==1
		//Disable ap ez_mode
		memset(sys_cmd, 0x0, 128);
		sprintf(sys_cmd,"killall hostapd");
		ret = system(sys_cmd);
#endif
		memset(sys_cmd, 0x0, 128);
		sprintf(sys_cmd,"killall udhcpd");
		ret = system(sys_cmd);	

	}
}

static inline int  snx_start_stream()
{
	char sys_cmd[128];
	int ret=0;

	system("/bin/gpio_led -n 1 -m 1 -v 1");// blue led (pwm2) on
	system("/bin/gpio_led -n 0 -m 1 -v 0");// green led (pwm2) off

/*
	// CMD http-tunneling-serv
	memset(sys_cmd,	0x0, 128);
	sprintf(sys_cmd,"%s&", CMD_TUNNELING);
	ret = system(sys_cmd);
	// CMD sonix-proj
	memset(sys_cmd,	0x0, 128);
	sprintf(sys_cmd,"%s&", CMD_PROJ);
	ret = system(sys_cmd);
	// CMD twowayaudio
	memset(sys_cmd,	0x0, 128);
	sprintf(sys_cmd,"%s&", CMD_TWOWAY);
	ret = system(sys_cmd);
*/
	return ret;
}

void snx_stop_stream(void)
{
/*
	system ("killall cstreamer");
	system ("killall minissdpd");
	system ("sleep 1");
*/	
//	system ("/etc/SNIP39/snx_stop.sh");


	system ("killall sonix-proj");
	system ("killall twowayaudio");
	system ("killall http-tunneling-serv");

	
}

void snx_restart_stream(void)
{
	struct stat tst;
	char sys_cmd[128];
	int devnum;

	printf("---------------------------------\n");
	printf(" %s\n",__func__);
	printf("---------------------------------\n");

	snx_stop_stream();

	snx_get_file_value("/sys/bus/usb/devices/1-1/devnum", &devnum, 10);
	
	// detect USB WIFI modules
	// stat("/proc/bus/usb/001/002", &tst)
	memset(sys_cmd, 0x0, 128);
	sprintf(sys_cmd,"%s%.3d", "/proc/bus/usb/001/", devnum);

	if (stat(sys_cmd, &tst) != -1) {
		memset(dev, 0x0, 16);
		sprintf(dev, WIFI_DEV);
		snx_start_stream();
	}
	else {
		printf("NON USB wifi\n");
		memset(dev, 0x0, 16);
		sprintf(dev, ETH_DEV);
		snx_start_stream();
	}
}


static inline void snx_monitor(void)
{
	struct stat tst;
	char sys_cmd[128];
	FILE *pFile;
	int pid=0, len=0;

/*
//	if(snx_get_file_value(GALAXY_PID_FILE, &pid, 10) == -1)
	if(snx_get_file_value(CSTREAM_PID_FILE, &pid, 10) == -1)
		return;
	sprintf(sys_cmd,"/proc/%d", pid);

	if (stat(sys_cmd, &tst) == -1) {
		printf(" restart !!!!!!\n");
//		printf("Can't find %s \n", GALAXY_PID_FILE);
		printf("Can't find %s \n", CSTREAM_PID_FILE);
		snx_restart_stream();
	}
*/

}


//static inline void snx_set_sta(void)
static int snx_set_sta(void)
{
	char sys_cmd[128];
	struct stat tst;
	int ret;

  	wifi_status_ok = 0;
	// CMD "wpa_supplicant -B -Dwext -c /etc/EZ_DIR/wpa_supplicant.conf -i wlan0 "
	memset(sys_cmd, 0x0, 128);
	sprintf(sys_cmd,"%s -c %s%s -i %s"
		, CMD_STA
		, EZ_DIR, CONF_STA
		, WIFI_DEV);
	ret = system(sys_cmd);

	// CMD udhcpc -i wlan0
	memset(sys_cmd, 0x0, 128);
	sprintf(sys_cmd,"%s%s", CMD_DHCPC, WIFI_DEV);
	ret = system(sys_cmd);
  	wifi_status_ok = 1;
	// Check for DHCP dispatch ready
	ret = snx_net_rdy();
	return ret;
	
}

static inline void snx_gpio_detect(void)
{
	int reset = 0;
	int count = 0;
	char cmd[64] = {0};
	char sys_cmd[128];

	gpio_pin_info info;

	// GPIO reset to default
	info.pinumber = GPIO_PIN_2;
	info.mode = 0;
	snx_gpio_read (&info);

	reset = file_exist(RESET_DEVICE);
	if ((info.value == 0) || (reset == 1)) {
		printf ("reset bottom \n");

		// In EZ-SETUP mode
		snx_set_ez(EZ_SETUP_GPIO_START);
		if(reset == 1) {
			system("rm /tmp/reset_device");
		}

		printf ("reset to default setting\n");
		ez_stat = -1;

		system("/bin/gpio_led -n 0 -m 1 -v 0");// green led (pwm2) off
		system("/bin/gpio_led -n 1 -m 1 -v 0");// blue led (pwm2) off
		system("/bin/gpio_led -n 0 -m 1 -v 1");// green led (pwm2) on
		system("/bin/snx_pwm_period 0 500 1000"); // (Green led)pwm2 start blink				

		snx_kill_ez();
		snx_stop_stream();
		
		snx_rst_def();
		
		// play audio file for reset
		if(reset != 1) {
			snx_play_audio(RESET_PCM);
			// CP /root/etc_default/ --> /etc/
			// Read NVRAM UID to /etc/SNIP39/SNIP39_UID.conf
			// Set /etc/SNIP39/SNIP39_UID.conf to /etc/SNIP39/hostap_XXX.conf
		}
		printf ("reset\n");

		snx_set_ez_mode(EZ_MODE_SCAN);
		snx_set_ez(EZ_SETUP_GPIO_END);

       	}	// 	if (info.value == 0)
}

void sigalrm_fn(int sig)
{
  	static int count = 0;
	char sys_cmd[128];
	int ret;
	static int wifi_count = 0;
	
	// gpio detect reset to default
	snx_gpio_detect();
	if(ez_stat == 1) {
		wifi_count++;
		if(wifi_count >= wifi_retry_sec) {
			wifi_count = 0;
			// CMD 
			memset(sys_cmd, 0x0, 128);
			sprintf(sys_cmd,"%s%s %s %s"
				, EZ_DIR
				, CMD_RENEW
				, RENEW_PING
				, RENEW_PID_FILE);
//			ret = system(sys_cmd);

		}
		// monitor
		snx_monitor();
	}

	if(wifi_status_ok == 0){
		count++;
//		if(count == 60)
//			snx_play_audio(ROUTER_FAIL_PCM, DEFAULT_VOL);
	}else
		count = 0;

	alarm(2);
}


void sigterm_fn(int sig)
{
	char sys_cmd[128];

	// Prevent to be locked in udhcpc loop
	snx_set_ez(EZ_SETUP_GPIO_START);

	snx_kill_ez();
	snx_stop_stream();

	
	ez_stat = -1;
	snx_set_ez_mode(EZ_MODE_QUIT);

	// touch /var/run/quit_temp

	// remove snx_ez pid
	memset(sys_cmd, 0x0, 128);
	sprintf(sys_cmd,"rm %s", EZ_PID_FILE);
	system(sys_cmd);
//	exit(0);
}

static void usage(FILE * fp, int argc, char ** argv) {   
    fprintf(fp, "Usage: %s [options]/n\n"   
        "Options:\n"
	"-h Print this message\n"
	"-d show debug message\n"
	"-r restart \n"
	"-m scan mode wifi=1, qr_scan=2, audio_tone=4 \n"
	"-u udhcpc scan second \n"
	"-w wifi or non wifi default (wifi=1) \n"
	"\n"
        "", argv[0]);   
}


//static char cb_string[512];

//int snx_cmd_str_cb(char *recv, char **resp)
void snx_cmd_str_cb(const char *recv)
{
	// Get string
	if(recv != NULL)
		printf("Get recv string===%s===\n", recv);

	// Set string
//	sprintf(cb_string,"example string");
//	*resp = cb_string;
}



int main(int argc, char **argv)
{
	struct stat tst;
	int ret;
	char sys_cmd[128];
	char debug_cmd[32];
	int pid;
	char uid[1024];
	char key[1024];
	int mode=7;
	int usb_wifi=1;
	struct sigaction sa;

	gpio_pin_info info;

	snx_set_ez_mode(EZ_MODE_NONE);

	memset(sys_cmd, 0x0, 128);
	pid = getpid();
	sprintf(sys_cmd,"echo %d > %s", pid, EZ_PID_FILE);
	system(sys_cmd);

	for (;;) {
		int index;
		int c;
		c = getopt(argc, argv, "p:d:r:u:m:w:");
		if (-1 == c) 
			break;
		switch (c) {
			case 'h':   
				usage(stdout, argc, argv);   
				exit(EXIT_SUCCESS);   
			case 'p':
				sscanf(optarg, "%d", &proxy);
				break;
			case 'd':
				sscanf(optarg, "%d", &debug);
				break;
			case 'r':
				sscanf(optarg, "%d", &restart);
				break;
			case 'u':
				sscanf(optarg, "%d", &wifi_retry_sec);
				break;
			case 'm':
				sscanf(optarg, "%d", &mode);
				break;
			case 'w':
				sscanf(optarg, "%d", &usb_wifi);
				break;
			default:   
				usage(stderr, argc, argv);
				exit(EXIT_FAILURE);
		}
	}
	// Bootup init UID from NVRAM
	// Read NVRAM UID to /etc/SNIP39/SNIP39_UID.conf
	snx_read_uid((char **)&uid, sizeof(uid));

	// IF argument 1 is NULL, read UID from /etc/SNIP39/SNIP39_UID.conf
	// IF argument 2 is NULL, key can't call back
	// IF argument 3 is WRITE/NONWRITE, WRITE is ssid & key to /etc/SNIP39/hostap_XXX.conf
	snx_set_ssid(uid, (char **)&key, WRITE);

	// GPIO init
	snx_gpio_open ();
	info.pinumber = GPIO_PIN_2;
	info.mode = 1;
	info.value = 1;
	if(snx_gpio_write(info) == GPIO_FAIL)
		printf ("write gpio%d error\n",GPIO_PIN_2);

	while(snx_get_ez_mode() != EZ_MODE_QUIT) {
		printf("start sonix ez monitor\n");

		ez_stat = 0;

		// set signal handlers 

		signal(SIGALRM, sigalrm_fn);
		alarm(1);
		memset(&sa, 0, sizeof(struct sigaction));
		sa.sa_handler = sigterm_fn;
		if(sigaction(SIGTERM, &sa, NULL)) {
			printf("Failed to set SIGTERM handler. EXITING");

			snx_set_ez_mode(EZ_MODE_QUIT);
			goto finally;
		}
		if(sigaction(SIGINT, &sa, NULL)) {
			printf("Failed to set SIGINT handler. EXITING");

			snx_set_ez_mode(EZ_MODE_QUIT);
			goto finally;
		}

		// detect USB WIFI modules
		if(usb_wifi) {// WIFI mode
			// CMD ifconfig eth0 down
			memset(dev, 0x0, 16);
			sprintf(dev, WIFI_DEV);

			memset(sys_cmd, 0x0, 128);
			sprintf(sys_cmd,"%s", "ifconfig eth0 down");
			ret = system(sys_cmd);

			// detect Scan / STA mode from /etc/SNIP39/default.conf
			if(snx_get_mode() == 1) { // STA mode
				printf("System on STA Mode\n");
				snx_set_ez_mode(EZ_MODE_STA);

				// Check wifi AP/STA mode driver
				snx_wifi_driver_check();

				// Check wifi device ready
	      			snx_wifi_dev_rdy();
				snx_set_sta();
			}
			else {	// Scan mode
				// Parser information from QR information 
				printf("System on Scan Mode\n");
				snx_set_ez_mode(EZ_MODE_SCAN);

				if(mode & EZ_METHOD_WIFI){
#if RT3070==1 || MT7601==1
					snx_ralink_conf(uid, key);
#endif
					// Check wifi AP/STA mode driver
					snx_wifi_driver_check();

					// Check wifi device ready
		      			snx_wifi_dev_rdy();
				}

				while (snx_get_ez() != EZ_SETUP_GPIO_START) {

					snx_start_scan(mode);
					
					if(snx_scan_check(dev
//							, EZ_METHOD_WIFI | EZ_METHOD_QR | EZ_METHOD_TONE
							, mode
							, snx_cmd_str_cb) == -1)
						goto finally;

					// Check wifi AP/STA mode driver
					snx_wifi_driver_check();

					snx_stop_scan(mode);

					if (snx_set_sta()) {		// Network is ready

						snx_set_ez(EZ_SETUP_NONE);
						snx_set_mode(1);
						break;
					}
				}

				system("killall	gpio3_blink");
				
				fprintf(stderr,	"end to	scan!! \n");
			}
			if(ez_stat	== -1) {
				fprintf(stderr,	"xxxxxxxx ez_stat = -1 xxxxxxxx\n");
				goto finally;
			}
			
			snx_start_stream();

		} //if(usb_wifi) NON USB WIFI
		else {
			printf("NON USB	wifi\n");
			memset(dev, 0x0, 16);
			sprintf(dev, ETH_DEV);

			if(snx_get_mode() == 1)	{ // STA mode
			}
			else {	// Scan mode
				if(snx_scan_check(WIFI_DEV
//						, EZ_METHOD_WIFI | EZ_METHOD_QR | EZ_METHOD_TONE
						, mode						
						, snx_cmd_str_cb) == -1)
					goto finally;
				snx_set_mode(1); // Set STA mode
				system("killall	gpio3_blink");
				fprintf(stderr,	"end to	scan!! \n");
			}
			if(ez_stat	== -1) {
				fprintf(stderr,	"xxxxxxxx ez_stat = -1 xxxxxxxx\n");
				goto finally;
			}
			snx_start_stream();

		} // else // if (stat(sys_cmd, &tst) != -1)
		ez_stat = 1;

finally:
		while(snx_get_ez_mode() != EZ_MODE_QUIT) {
			pause();
			if(snx_get_ez_def()) {
				snx_set_ez_def(0);
				break;
			}
		} // while(1)
	}//	while(1) 


}
