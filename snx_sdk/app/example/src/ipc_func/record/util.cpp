#include "util.h"
#include <time.h>
#include "sn98600_ctrl.h"
#include "snx_rc_lib.h"


int file_exist(const char *file_name)
{
	if(access(file_name, F_OK) == 0)
		return 1;
	else 
		return 0;
}


int snx_get_file_string(char *path, char *str)
{
	FILE *fp = NULL;
	char *buf = NULL;
	int size;

	/* Open file for both reading and writing */
	fp = fopen(path, "r");
	if (fp == NULL) {
		printf("open %s failed\n", path);
		return -1;
	}

	/* Seek to the beginning of the file */
	fseek(fp, SEEK_SET, 0);
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);

	buf = (char *)malloc(size);
	if (!buf) {
		printf("Cannot allocate buf %d bytes\n", size);
		return -1;
	}

	/* Read and display data */
	fseek(fp, SEEK_SET, 0);
	fread(buf, 1, size, fp);

	memcpy(str, buf, size - 1);	// except '\0'

	free(buf);
	fclose(fp);

	return 0;
}

int snx_file_inc_str(char *file, const char *str)
{
     int count = 0;
     char line[80];
     FILE *fp = fopen (file, "r");
     if (fp == NULL) {
         printf ("open %s failed\n", file);
         return 0;
     }

     while (1) {
         fgets(line, 80, fp);
         if (feof(fp))
             break;
 //      printf ("%s", line);
         if (!strncmp(line, str, strlen(str)))
             count++;
     }

     fclose(fp);
     return count;
}


int card_async_operation(char* cmd, char* target,  char* search)
{
	int remove = 0,count = 0;
	int finish = 0;
	int ret = -1;
	char syscmd[128];
	FILE *fp = NULL;
	char *name = NULL;

	snx_get_file_value(SD_REMOVAL_INFO, &remove, 10);
	if (remove == 0) {
		//printf("will run:%s\n",target);
		system(target);
	}
	else {
		printf("sd doesnot exist so cannot du anything !!\n");
		return -1;
	}
	
	while (count++ < 500){
		memset(syscmd, 0, sizeof(syscmd));
		sprintf(syscmd,"ps | grep %s > /tmp/cmdinfo",cmd);		
		system(syscmd);
		//printf("11#########\n");
		//system("cat /tmp/cmdinfo");
		//printf("22#########\n");
		fp = fopen("/tmp/cmdinfo", "r");
		if(!fp) {
			fprintf(stderr, "open %s failed!! \n", "/tmp/cmdinfo");
			return -1;
		}
		// set beginning
		fseek(fp, 0, SEEK_SET);
		while(1){
			memset(syscmd, 0x00, sizeof(syscmd));
			fgets(syscmd, sizeof(syscmd), fp);	  // Read next record
			//fprintf(stderr, "getvalue = %s \n", syscmd);
			
			name = strstr(syscmd, search);
			if(name) {
				break;
			}
			if(feof(fp)) {
				finish = 1;
				break;
			}
		}
		fclose(fp);
		system("rm /tmp/cmdinfo -rf");

		if (finish == 1){
			//printf("%s result /tmp/usage exists\n",cmd);
			ret = 0;
			break;
		}else{
			snx_get_file_value(SD_REMOVAL_INFO, &remove, 10);
			if (remove ==1){
				printf("sd doesnot exist when %s !!\n",cmd);
				memset(syscmd, 0, sizeof(syscmd));
				sprintf(syscmd,"killall -9 %s",cmd);
				system(syscmd);
				return -1;
			}
		}
		//printf("wait for  %d !!\n",count);	
		usleep(50000);
		
	}
	
	if (ret == -1)
		printf("Error: wait for %s result BUT exceed %d, cmd num is %d !!\n",cmd,count,finish);	
		
	return ret;
}
