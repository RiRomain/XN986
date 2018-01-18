#include <stdio.h>	 
#include <stdlib.h>   
#include <string.h>    
#include <getopt.h> 			/* getopt_long() */   
#include <fcntl.h>				/* low-level i/o */   
#include <unistd.h>   
#include <pthread.h>

//#define DEFAULT_RST_PATH    "/etc/rst_default/rst_default.sh"

#include "snx_ez_lib.h"


static void usage(FILE * fp, int argc, char ** argv) {   
    fprintf(fp, "Usage: %s [options]/n\n"   
        "Options:\n"
        "-h Print this message\n"
        "-f force reset to default (Scan mode)\n"
        "-c Set Scan/STA mode (Scan=0/STA=1) \n"
        "-u Write UID to NVRAM\n"
        "-r system reboot\n"
        "-i system boot init\n"
        "-d close apsta\n"
        "\n\n\n"
        "", argv[0]);   
}



int main(int argc, char **argv)  
{ 
	int value = 0;
	int count = 0;
	int force =0;
	int reboot =0;
//	int del =0;
	int init=0;
	int mode =0;	// AP(0)/STA(1) mode
	int write_uid=0;

	char uid[1024];
	char key[1024];

	for (;;) {   
		int index;   
		int c;   
		c = getopt(argc, argv, "hfridc:u:");
		if (-1 == c)   
			break;   
		switch (c) {   
//			case 0: /* getopt_long() flag */   
//				usage(stderr, argc, argv);   
//				break;
			case 'h':   
				usage(stdout, argc, argv);   
				exit(EXIT_SUCCESS);   
			case 'u':
				memset(uid, 0x0, 1024);
				sprintf(uid,"%s",optarg);
				write_uid = 1;
				break;
			case 'c':
				sscanf(optarg, "%d", &mode);
				break;
			case 'f':
				force =1;
				break;
			case 'r':
				reboot =1;
				break;	
			case 'i':
				init =1;
				break;	
			case 'd':
				snx_kill_ez();
				break;	
			default:
				usage(stderr, argc, argv);
				exit(EXIT_FAILURE);
		}   
	}

	if(init) {	// BOOT init
		// Read NVRAM UID to /etc/SNIP39_UID.conf
		snx_read_uid((char **)&uid, sizeof(uid));
		

		// IF argument 1 is NULL, read UID from /etc/SNIP39/SNIP39_UID.conf
		// IF argument 2 is NULL, key can't call back
		// IF argument 3 is WRITE/NONWRITE, WRITE is ssid & key to /etc/SNIP39/hostap_XXX.conf
		snx_set_ssid(uid, (char **)&key, WRITE);
//		printf("uid == %s key == %s\n", uid, key);
		
	}
	else if(force) {
		if(mode)
			printf ("Force set STA mode\n");
		else
			printf ("Force set Scan mode\n");

		// CP /root/etc_default/ --> /etc/
		// Read NVRAM UID to /etc/SNIP39_UID.conf
		// Set /etc/SNIP39_UID.conf to /etc/hostap/hostap_XXX.conf
		snx_rst_def();
		
		snx_set_mode(mode);
//		system("touch /etc/toapsta");

		if(reboot) {
			printf ("reboot\n");
			system ("reboot");
		}
	}
	else if(write_uid) {

		// Set uid to /etc/SNIP39_UID.conf
		snx_set_uid(uid);

		// Write UID/SSID/KEY to NVRAM
		snx_write_uid(uid);

		// IF argument 1 is NULL, read UID from /etc/SNIP39/SNIP39_UID.conf
		// IF argument 2 is NULL, key can't call back
		// IF argument 3 is WRITE/NONWRITE, WRITE is ssid & key to /etc/SNIP39/hostap_XXX.conf
		snx_set_ssid(uid, (char **)&key, WRITE);
	}
	else{
/*
		printf ("Reset Button\n");
		
		snx_gpio_open (GPIO_PIN_2);
		snx_gpio_write (GPIO_PIN_2,1);
		while (1) {
			snx_gpio_read (GPIO_PIN_2,&value);
			printf ("Button value == %x\n",value);
			sleep (1);
			if (value == 0)
				count++;
			else
				count = 0;
    
			if (count >= 3) {
				printf ("reset to default setting\n");
				snx_rst_def();
//				system (DEFAULT_RST_PATH);
				if(reboot) {
					printf ("reboot\n");
					system ("reboot");
				}
			}
		}
*/
	}

}  

