/*
 * Handle motion alarm recording and schedule recording
 *
*/
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/vfs.h>
#include <unistd.h>
#include "util.h"
#include "sn98600_record.h"
#include "sn98600_record_audio.h"
#include "sn98600_ctrl.h"
#include "snx_rc_lib.h"
#include "snx_lib.h"

#include "save_media.h"
#include "data_buf.h"
#include "avhandler.h"

save_media_source *sd_record = NULL;
av_buffer *video_pre_buffer = NULL;
av_buffer *audio_pre_buffer = NULL;

static sonix_audio *audio_record = NULL;


#define SD_INSERT		1
#define SD_OUT			0

#define RECORD_TRIG			(1<<0)
#define ALARM_TRIG			(1<<1)

int sd_alarm_record_en = 0;  	// 0 disable  1 enable
int sd_record_en = 0;			// 0 disable  1 enable   // the switch to start save data to SD card.
int sd_record_start = 0;
#if TIMELAPSE_SUPPORT
int timelapse = 0;		
#endif
int md_record_status = 0;  		// 0 stop  1 start

static int trigger_status	= 0;
static int isRecordUsed = 0;
static int record_status = 0;		// 0:off; 1:on
static int sd_insert_init_done = 0;
static int reserved_record_size_mb	= 0;
static int reserved_alarm_size_mb = 0;

static int clos_trig = 0;
static int md_clos_trig = 0;

/* record accmulation count */
static int accmulation_fp = 0;
static int md_accmulation_fp = 0;

static pthread_t record_id;
static int sd_has_inserted = 0;

extern pthread_mutex_t audio_sync_lock; 	//haoweilo
extern pthread_mutex_t md_audio_sync_lock; 	//haoweilo

extern int total_accmulation_frame;			//Get Accmulation frames from avhandler.cpp

#define SD_RECORD_DBG

#ifdef SD_RECORD_DBG
static int dbg_print_cnt = 0;
#endif

struct SD_FS
{
	int total_size;
	int total_free_size;

	int total_record_size;
	int total_alarm_size;

	int total_free_record_size;
	int total_free_alarm_size;
};

static int start_audio_record(void);
static int stop_audio_record(void);


/* check SD insert or not */
int check_sd_workable(void)
{
  int res_1 = -1;
  
	int remove = 0;
  snx_get_file_value(SD_REMOVAL_INFO, &remove, 10);
  system ("find /tmp -name \'mmcblk*\' > /tmp/what.mount.log");
  res_1 = system ("grep -c \"mmcblk\" /tmp/what.mount.log > /tmp/num.mmc.mt.log");
		
	if((file_exist("/tmp/mmc.all.log") && remove == 0) && (res_1 == 0)){
		return 1;
	}else{
		printf("## sd remove info : %d and sd_has_inserted %d : %d \n", remove,sd_has_inserted,res_1);
		
		sd_has_inserted = 0;
		return 0;
	}

}


/* request the specified partition size */
int request_partition_size(char* specified_path)
{
	unsigned long long size = 0;
	int size_mb = 0;
	struct statfs buf;
	int remove = 0;
	
	snx_get_file_value(SD_REMOVAL_INFO, &remove, 10);
	if (remove == 1){
		fprintf(stderr, "sd removed,cannot request_partition_size\n");
		return -1;
	}
		
	if (statfs(specified_path, &buf) < 0) {
		fprintf(stderr, "check %s storage failed!! \n", specified_path);
		return -1;
	}

	size = (unsigned long long)buf.f_bsize * buf.f_blocks;

	//fprintf(stderr, "size = %lld \n", size>>20);

	size_mb = (int) (size>>20);
	return size_mb;
}

/* request the specified partition free size */
int request_partition_free_space(char* specified_path)
{
	unsigned long long size = 0;
	int size_mb = 0;
	struct statfs buf;
	int remove = 0;
	
	snx_get_file_value(SD_REMOVAL_INFO, &remove, 10);
	if (remove == 1){
		fprintf(stderr, "sd removed,cannot request_partition_free_space\n");
		return -1;
	}
	
	if (statfs(specified_path, &buf) < 0) {
		fprintf(stderr, "check %s storage failed!! \n", specified_path);
		return -1;
	}

	size = (unsigned long long)buf.f_bsize * buf.f_bfree;
	
	//fprintf(stderr, "size = %lld \n", size>>20);

	size_mb = (int) (size>>20);
	return size_mb;

}

int get_first_available_partition(int *block_id, int *partition_id, struct SD_FS *sd_fs)
{
	FILE *fp = NULL;
	char getvalue[128];
	char target[128];
	char mount_type[16];
	char *name = NULL;
	int check = 0;
	int ref_idx = 0;
	int blk_idx = 0;
	int ref_size_mb = 0;
	int ref_free_size_mb = 0;
	int wait_check = 0;
	
	fp = fopen("/tmp/mmc.all.log", "r");
	if(!fp) {
		fprintf(stderr, "open %s failed!! \n", "/tmp/mmc.all.log");
		return -1;
	}
	// set beginning
	fseek(fp, 0, SEEK_SET);
	
	memset(getvalue, 0x00, sizeof(getvalue));
	fgets(getvalue, sizeof(getvalue), fp);	  // Read next record

	name = strstr(getvalue, "mmcblk");
	if(name) {
		fprintf(stderr, "name = %s \n", name);
		name += 6;
		blk_idx = atoi(name);
		fprintf(stderr, "blk_idx = %d \n", blk_idx);

		/* Add condition verify for mmclbk */
		name += 2;
		if(name) {
			if((strncasecmp(name, "vfat", 4) == 0) /*|| (strncasecmp(name, "ntfs", 4) == 0) || (strncasecmp(name, "exfat", 5) == 0)*/){
				ref_idx = -1;
				check = 1;
				memset(mount_type, 0x00, sizeof(mount_type));
				memcpy(mount_type, name, strlen(name));
				fprintf(stderr, "mount_type = %s \n", mount_type);
			}
		}

	}
	else{
		fclose(fp);
		return -1;
	}

	while(!check) {
		memset(getvalue, 0x00, sizeof(getvalue));
		fgets(getvalue, sizeof(getvalue), fp);	  // Read next record
		fprintf(stderr, "getvalue = %s \n", getvalue);
		
		memset(target, 0x00, sizeof(target));
		sprintf(target, "mmcblk%dp", blk_idx);
		
		name = strstr(getvalue, target);
		if(name) {
			fprintf(stderr, "%s \n", name);
			name += 8;
			ref_idx = atoi(name);
			fprintf(stderr, "partition id = %d \n", ref_idx);

			name += 2;
			fprintf(stderr, "%s \n", name);
			if((strncasecmp(name, "vfat", 4) == 0) /*|| (strncasecmp(name, "ntfs", 4) == 0) || (strncasecmp(name, "exfat", 5) == 0)*/) {
				check = 1;
				memset(mount_type, 0x00, sizeof(mount_type));
				memcpy(mount_type, "vfat", strlen("vfat")); //alek
				fprintf(stderr, "mount_type = %s \n", mount_type);
				break;
			}
		}
		
		if(feof(fp)) {
			break;
		}		
	}
	fclose(fp);
	
	if(check == 0) {
		fprintf(stderr, "Cannot find out any available partition!!\n");
		return -1;
	}

	/* check mount ready */
	memset(target, 0x00, sizeof(target));
	if(ref_idx == -1) {
		sprintf(target, "/tmp/mmcblk%d", blk_idx);
	}
	else {
		sprintf(target, "/tmp/mmcblk%dp%d", blk_idx, ref_idx);
	}
	
	while(file_exist(target) == 0) {
		if(wait_check > 10)
			return -1;
		fprintf(stderr, "mount partition not ready!!\n");
		wait_check++;
		sleep(1);
	}
	
	wait_check = 0;
	
	/*use ls to check filesystem in the sd card*/
	memset(target, 0x00, sizeof(target));
	if(ref_idx == -1) {
		sprintf(target, "ls /media/mmcblk%d/%s/%s -al > /dev/null", blk_idx, SD_EXAMPLE,SD_RECORD_PATH);
	}
	else {
		sprintf(target, "ls /media/mmcblk%dp%d/%s/%s -al > /dev/null", blk_idx, ref_idx, SD_EXAMPLE,SD_RECORD_PATH);
	}
	system(target);

	if (sd_alarm_record_en) {		//if Motion detection record enable

		memset(target, 0x00, sizeof(target));
		if(ref_idx == -1) {
			sprintf(target, "ls /media/mmcblk%d/%s/%s -al > /dev/null", blk_idx, SD_EXAMPLE,SD_ALARM_PATH);
		}
		else {
			sprintf(target, "ls /media/mmcblk%dp%d/%s/%s -al > /dev/null", blk_idx, ref_idx, SD_EXAMPLE,SD_ALARM_PATH);
		}
		system(target);

	}
	while(wait_check < 2){
		
		/* check mount device is RW or not */
		system("mount > /tmp/m_info");
	
		fp = fopen("/tmp/m_info", "r");
		if(!fp) {
			fprintf(stderr, "open %s failed!! \n", "/tmp/m_info");
			/*umount /media  ??*/
			memset(target, 0x00, sizeof(target));
			if(ref_idx == -1) {
				sprintf(target, "umount /media/mmcblk%d", blk_idx);
			}
			else {
				sprintf(target, "umount /media/mmcblk%dp%d", blk_idx, ref_idx);
			}
			system(target);
			return -1;
		}
	
		memset(target, 0x00, sizeof(target));
		if(ref_idx == -1)
			sprintf(target, "mmcblk%d", blk_idx);
		else
			sprintf(target, "mmcblk%dp%d", blk_idx,ref_idx);
	
		while(1) {
			memset(getvalue, 0x00, sizeof(getvalue));
			fgets(getvalue, sizeof(getvalue), fp);	  // Read next record
			//fprintf(stderr, "getvalue = %s \n", getvalue);
		
			name = strstr(getvalue, target);
			if(name) {
				fprintf(stderr, "device mount : %s \n", name);
			
				name = strstr(getvalue, "rw");
				if(!name){
					fprintf(stderr, "the device is NOT RW !!\n ");
         // printf ("wait_check = %d\n",wait_check);
					if(wait_check<1){
						/* Add Leo recommand to do remote again ! */
						memset(target, 0x00, sizeof(target));
						if(ref_idx == -1) {
							sprintf(target, "mount -o rw,remount -t %s /dev/mmcblk%d /media/mmcblk%d",
							mount_type, blk_idx,  blk_idx);
						}
						else {
							sprintf(target, "mount -o rw,remount -t %s /dev/mmcblk%dp%d /media/mmcblk%dp%d",
							mount_type, blk_idx, ref_idx,  blk_idx, ref_idx);
						}
						fprintf(stderr, "target = %s ..\n", target);
						system(target);
					
						/*use ls to check filesystem in the sd card*/
						memset(target, 0x00, sizeof(target));
						if(ref_idx == -1) {
							sprintf(target, "ls /media/mmcblk%d/%s/%s -al > /dev/null", blk_idx, SD_EXAMPLE,SD_RECORD_PATH);
						}
						else {
							sprintf(target, "ls /media/mmcblk%dp%d/%s/%s -al > /dev/null", blk_idx, ref_idx, SD_EXAMPLE,SD_RECORD_PATH);
						}
						system(target);

						if (sd_alarm_record_en) {		//if Motion detection record enable
							memset(target, 0x00, sizeof(target));
							if(ref_idx == -1) {
								sprintf(target, "ls /media/mmcblk%d/%s/%s -al > /dev/null", blk_idx, SD_EXAMPLE,SD_ALARM_PATH);
							}
							else {
								sprintf(target, "ls /media/mmcblk%dp%d/%s/%s -al > /dev/null", blk_idx, ref_idx, SD_EXAMPLE,SD_ALARM_PATH);
							}
							system(target);
						}

					}
					
					fclose(fp);
					system("rm -rf /tmp/m_info");
					check = 0;
					break;
				}else{
					fprintf(stderr, "mount device is RW \n");
					fclose(fp);
					system("rm -rf /tmp/m_info");
					check = 1;
					break;
				}
				
			}
		
			if(feof(fp)) {
				fprintf(stderr, "can not find RW device\n");
				/*umount /media  ??*/
				memset(target, 0x00, sizeof(target));
				if(ref_idx == -1) {
					sprintf(target, "umount /media/mmcblk%d", blk_idx);
				}
				else {
					sprintf(target, "umount /media/mmcblk%dp%d", blk_idx, ref_idx);
				}	
				system(target);
				fclose(fp);
				system("rm -rf /tmp/m_info");
				check = 0;
				wait_check = 3; // exist check loop
				break;
			}		
		}
		printf (".......\n");
		if(check == 0)
			wait_check++;
		else
			break;
	}
	
	if(check == 0) {
		fprintf(stderr, "the device is NOT RW after trying twice\n");
		return -1;
	}
	
	memset(target, 0x00, sizeof(target));
	if(ref_idx == -1) {
		sprintf(target, "/media/mmcblk%d", blk_idx);
	}
	else {
		sprintf(target, "/media/mmcblk%dp%d", blk_idx, ref_idx);
	}

	ref_size_mb = request_partition_size(target);
	if (ref_size_mb == -1){
		return -1;
	}
	
	ref_free_size_mb = request_partition_free_space(target);
	if (ref_free_size_mb == -1){
		return -1;
	}

	sd_has_inserted = 1;

	*partition_id = ref_idx;
	*block_id = blk_idx;
	
	memset(sd_fs, 0x00, sizeof(struct SD_FS));
	sd_fs->total_size = ref_size_mb;
	sd_fs->total_free_size = ref_free_size_mb;

	return 0;

}

#if 0
int get_biggest_partition(int *partition_id, struct SD_FS *sd_fs)
{
	FILE *fp = NULL;
	int ref_idx = 0;
	int ref_size_mb = 0;
	int ref_free_size_mb = 0;
	char target[128];
	char syscmd[128];
	char getvalue[64];
	char *name = NULL;
	int check = 0;

	memset(syscmd, 0x00, sizeof(syscmd));

	sprintf(syscmd, "ls /media/ | grep  \"%s\" > /tmp/sd_result", "mmcblk0p");
	system(syscmd);

	fp = fopen("/tmp/sd_result", "r");
	if(!fp) {
		fprintf(stderr, "open %s failed!! \n", "/tmp/sd_result");
		return -1;
	}
	
	// set beginning
	fseek(fp, 0, SEEK_SET);
	
	while(1) {
		memset(getvalue, 0x00, sizeof(getvalue));
		fgets(getvalue, sizeof(getvalue), fp);	  // Read next record
		//fprintf(stderr, "%s \n", getvalue);

		name = strstr(getvalue, "mmcblk0p");
		if(name) {
			name+=8;
			ref_idx = atoi(name);
			//fprintf(stderr, "First available partition id = %d \n", ref_idx);
			check = 1;
			break;
		}
		
		if(feof(fp)) {
			break;
		}		
	}
	fclose(fp);
	system("rm /tmp/sd_result");
	if(check == 0) {
		fprintf(stderr, "Cannot find out any available partition!!\n");
		return -1;
	}
	//ref_idx = id;
	memset(target, 0x00, sizeof(target));
	sprintf(target, "/media/mmcblk0p%d", ref_idx);

	ref_size_mb = request_partition_size(target);
	ref_free_size_mb = request_partition_free_space(target);

	*partition_id = ref_idx;
	
	memset(sd_fs, 0x00, sizeof(struct SD_FS));
	sd_fs->total_size = ref_size_mb;
	sd_fs->total_free_size = ref_free_size_mb;

	//fprintf(stderr, "ref_idx = %d, ref_size_mb = %d ref_free_size_mb = %d \n", 
	//	ref_idx, ref_size_mb, ref_free_size_mb);

	
	return 0;
}

#endif
int get_current_used_space(int block, int partition, int record_type)
{
	FILE *fp = NULL;
	int used_size_mb = 0;
	char target[128];
	char getvalue[128];
	int ret = 0;

	
	memset(target, 0x00, sizeof(target));

	if(partition == -1) {
		sprintf(target, "du /media/mmcblk%d/%s/%s > /tmp/usage &", block, SD_EXAMPLE,
			(record_type == SD_REC_SCHED)?(SD_RECORD_PATH):(SD_ALARM_PATH));
	}
	else {
		sprintf(target, "du /media/mmcblk%dp%d/%s/%s > /tmp/usage &", block, partition, SD_EXAMPLE,
			(record_type == SD_REC_SCHED)?(SD_RECORD_PATH):(SD_ALARM_PATH));

	}

	ret = card_async_operation("du",target,SD_EXAMPLE);
	if (ret == -1)
		return -1;
	
	fp = fopen("/tmp/usage", "rb");
	if(!fp) {
		fprintf(stderr, "Cannot open /tmp/usage for record_type %d \n", record_type);
		return -1;
	}
	
	memset(getvalue, 0x00, sizeof(getvalue));
	fgets(getvalue, sizeof(getvalue), fp);	  // Read next record
	fclose(fp);

	//fprintf(stderr, "%s-[%d] with %d KB \n", __FUNCTION__, record_type, atoi(getvalue));
	
	used_size_mb = atoi(getvalue)>>10;

	//fprintf(stderr, "used_size_mb = %d \n", used_size_mb);

	system("rm /tmp/usage");
	return used_size_mb;
	
}

int check_SD_EXAMPLE_folder(int block, int partition)
{
	char target[128];
	char syscmd[128];
	
	memset(target, 0x00, sizeof(target));
	if(partition == -1)
		sprintf(target, "/media/mmcblk%d/%s", block, SD_EXAMPLE);
	else
		sprintf(target, "/media/mmcblk%dp%d/%s", block, partition, SD_EXAMPLE);

	if(file_exist(target) == 0) {
		memset(syscmd, 0x00, sizeof(syscmd));
		sprintf(syscmd, "mkdir %s", target);
		fprintf(stderr, "%s \n", syscmd);
		system(syscmd);
		return 0;
	}
	else
		return 1;

}

int get_folder_info(int block, int partition, int record_type, struct SD_FS *sd_fs)
{
	int used_size = 0;
	char target[128];
	memset(target, 0x00, sizeof(target));

	if(partition == -1) {
		sprintf(target, "/media/mmcblk%d/%s/%s", block, SD_EXAMPLE, 
			(record_type == SD_REC_SCHED)?(SD_RECORD_PATH):(SD_ALARM_PATH));
	}
	else {
		sprintf(target, "/media/mmcblk%dp%d/%s/%s", block, partition, SD_EXAMPLE, 
			(record_type == SD_REC_SCHED)?(SD_RECORD_PATH):(SD_ALARM_PATH));
	}
	
	if(file_exist(target) == 0) {
		char syscmd[128];
		/* empty folder */
		if(record_type == SD_REC_SCHED) {
			sd_fs->total_record_size = 0;
			sd_fs->total_free_record_size = 0;
		}
		else {
			sd_fs->total_alarm_size = 0;
			sd_fs->total_free_alarm_size = 0;
		}

		memset(syscmd, 0x00, sizeof(syscmd));
		sprintf(syscmd, "mkdir %s", target);
		system(syscmd);

		return 0;
	}
	else {
		used_size = get_current_used_space(block, partition, record_type);
		if(used_size == -1)
			return -1;
		
		if(record_type == SD_REC_SCHED)
			sd_fs->total_record_size = used_size;
		else
			sd_fs->total_alarm_size = used_size;

	}

	return 1;
}

/* create Schedule or MD alarm recording file name */
void create_storage_filename(char* name_string, int block, int partition, int record_type)
{
    time_t timep;
    struct tm *tp;
	char MM[2], DD[2], hh[2], mm[2], ss[2];

    time(&timep);
    tp = localtime(&timep);

	if ((1 + tp->tm_mon) < 10)
		sprintf(MM, "0%d", (1 + tp->tm_mon));
	else
		sprintf(MM, "%d", (1 + tp->tm_mon));

	if (tp->tm_mday < 10)
		sprintf(DD, "0%d", tp->tm_mday);
	else
		sprintf(DD, "%d", tp->tm_mday);

	if (tp->tm_hour < 10)
		sprintf(hh, "0%d", tp->tm_hour);
	else
		sprintf(hh, "%d", tp->tm_hour);

	if (tp->tm_min < 10)
		sprintf(mm, "0%d", tp->tm_min);
	else
		sprintf(mm, "%d", tp->tm_min);

	if (tp->tm_sec < 10)
		sprintf(ss, "0%d", tp->tm_sec);
	else
		sprintf(ss, "%d", tp->tm_sec);

	sprintf(name_string, "/media/mmcblk%dp%d/%s/%s/%d%s%s-%s%s%s.mp4", block, partition, SD_EXAMPLE,
		(record_type == SD_REC_SCHED)?(SD_RECORD_PATH):(SD_ALARM_PATH), 
		(1900 + tp->tm_year), MM, DD, hh, mm, ss);

	fprintf(stderr, "%s with %s \n", __FUNCTION__, name_string);

}

//alek add check sd block is rw or ro
#define CHECK_SD_RW_RO 1
#define SD_MODE_TXT "/tmp/sd_mode"
#define FILE_OPEN_ERROR 1
#define MODE_RW 0
#define MODE_RO -1
#if CHECK_SD_RW_RO
/*check sd mode*/
int snx_check_SD_mode (int block, int partition)
{
  FILE *fp = NULL;
	char sd_info[128];
  char syscmd[128];
  char mount_type[16];
  int ret = 0;
  memset(syscmd, 0x00, sizeof(syscmd));
	if(partition == -1) {
		sprintf(syscmd, "mount | grep /media/mmcblk%d > %s ", block,SD_MODE_TXT);
	}
	else {
		sprintf(syscmd, "mount | grep /media/mmcblk%dp%d > %s ", block, partition,SD_MODE_TXT);
	}
  system(syscmd);
  
  fp = fopen(SD_MODE_TXT, "r");
  if(fp==NULL) 
  {
    fprintf(stderr, "open %s failed!! \n", SD_MODE_TXT);
    return FILE_OPEN_ERROR;
  }    
  memset(sd_info, 0x00, sizeof(sd_info));

	fgets(sd_info, sizeof(sd_info), fp);	  
	fprintf(stderr, "sd_info = %s \n", sd_info);
		
	if (strstr(sd_info, "rw") == NULL)
  {
    printf ("now SD is not RW mode \n");
    ret = MODE_RO; 
    printf ("must do sd remount \n");
    if(strstr(sd_info,"vfat") != NULL)
    {
      memset(mount_type, 0x00, sizeof(mount_type));
			memcpy(mount_type, "vfat", strlen("vfat"));
    }  
    memset(syscmd, 0x00, sizeof(syscmd));
    if(partition == -1) {
		  sprintf(syscmd, "mount -o rw,remount -t %s /dev/mmcblk%d /media/mmcblk%d",
			mount_type, block,  block);
		}
		else {
		  sprintf(syscmd, "mount -o rw,remount -t %s /dev/mmcblk%dp%d /media/mmcblk%dp%d",
			mount_type, block, partition,  block, partition);
		}
		fprintf(stderr, "syscmd = %s \n", syscmd);
		system(syscmd);
  }
  else
  {
    printf ("now SD is RW\n");
    ret = MODE_RW;
  }  
  fclose (fp);
  memset(syscmd, 0x00, sizeof(syscmd));
  sprintf (syscmd,"rm %s",SD_MODE_TXT);
  system(syscmd);
  return ret;
}
int snx_remove_recordfile (char* removefilepath)
{
  int ret;
	char syscmd[128];
	char filename[128];
	char target[128]; 
  char *name = NULL;
	int check = 0;
	int partition_idx = -1;
	int block_idx = 0; 
  name = strstr(removefilepath,"mmcblk");
  if (name != NULL)
  {
    name = name + 6;
    block_idx = atoi (name);
    printf ("block_idx = %d\n",block_idx);
    memset (target,0,sizeof(target));
    sprintf(target, "mmcblk%dp", block_idx);
    name = strstr (removefilepath,target) ;
    if (name != NULL)
    {
      name += 8;
			partition_idx = atoi(name); 
      printf ("partition_idx %d\n",partition_idx);     
    }    
  }
  else
  {
    printf ("file path error\n");
    return FILE_OPEN_ERROR;
  }
  memset(syscmd,0,sizeof(syscmd));
  sprintf (syscmd,"rm -f %s",removefilepath);
  system (syscmd); 
  //check sd block mode
  while(snx_check_SD_mode(block_idx,partition_idx) == MODE_RO) 
  {
    printf ("now SD is RO mode, will remount SD block\n");
  } 
  printf ("now SD is RW mode\n");
  // check file exist
  if(access(removefilepath, F_OK) == 0)
  {
    // remove again
    printf ("file remove again\n");
    memset(syscmd,0,sizeof(syscmd));
    sprintf (syscmd,"rm -f %s",removefilepath);
    system (syscmd); 
    
    
  }
  else
  {
    printf ("file remove ok\n");   
  }
  return 0;
  
}
#endif

int rm_oldest_file(int block, int partition, int record_type)
{
	FILE *fp = NULL;
	char syscmd[128];
	char filename[128];
	char target[128];
	int remove = 0;
	int ret = 0;

	memset(syscmd, 0x00, sizeof(syscmd));
	if(partition == -1) {
		sprintf(syscmd, "ls -tr /media/mmcblk%d/%s/%s > /tmp/file_sort_table &", 
			block, SD_EXAMPLE, (record_type == SD_REC_SCHED)?(SD_RECORD_PATH):(SD_ALARM_PATH));
	}
	else {
		sprintf(syscmd, "ls -tr /media/mmcblk%dp%d/%s/%s > /tmp/file_sort_table &", 
			block, partition, SD_EXAMPLE, (record_type == SD_REC_SCHED)?(SD_RECORD_PATH):(SD_ALARM_PATH));
	}

	ret = card_async_operation("ls",syscmd,SD_EXAMPLE);
	if (ret == -1)
		return -1;
	//system(syscmd);

	fp = fopen("/tmp/file_sort_table", "rb");
	if(!fp) {
		fprintf(stderr, "open %s failed!! \n", "/tmp/file_sort_table");
		return -1;
	}

	fgets(filename, sizeof(filename), fp);

	fclose(fp);

	system("rm /tmp/file_sort_table");
	
	if(strlen(filename) == 0) {
		fprintf(stderr, "Nothing to be removed!! \n");
		return 0;
	}

	memset(syscmd, 0x00, sizeof(syscmd));
	memset(target, 0x00, sizeof(target));
	if(partition == -1) {
		sprintf(syscmd, "rm -f /media/mmcblk%d/%s/%s/%s", 
			block, SD_EXAMPLE, (record_type == SD_REC_SCHED)?(SD_RECORD_PATH):(SD_ALARM_PATH), filename);
				
		sprintf(target, "/media/mmcblk%d/%s/%s/%s", 
			block, SD_EXAMPLE, (record_type == SD_REC_SCHED)?(SD_RECORD_PATH):(SD_ALARM_PATH), filename);
				
		
	}
	else {
		sprintf(syscmd, "rm -f /media/mmcblk%dp%d/%s/%s/%s", 
			block, partition, SD_EXAMPLE, (record_type == SD_REC_SCHED)?(SD_RECORD_PATH):(SD_ALARM_PATH), filename);
				
		sprintf(target, "/media/mmcblk%dp%d/%s/%s/%s", 
			block, partition, SD_EXAMPLE, (record_type == SD_REC_SCHED)?(SD_RECORD_PATH):(SD_ALARM_PATH), filename);
				
		
	}
	
	snx_get_file_value(SD_REMOVAL_INFO, &remove, 10);
	if (remove == 0) { 
//alek add   
#if CHECK_SD_RW_RO
    ret = snx_remove_recordfile (target);
    if (ret == -1)
    {
      printf("rm files fail \n");
      return -1;
    }  
#else  
		int ret = 0;
		fprintf(stderr, "WILL rm oldest file %s \n", target);
		system(syscmd);
		ret = file_exist(target);
		fprintf(stderr, "file_exist %d \n",ret);
		if (ret){
			printf("rm files fail \n");
			return -1;
			/*memset(target, 0x00, sizeof(target));
			if (partition == -1) {
				sprintf(target, "mount -o rw,remount -t %s /dev/mmcblk%d /media/mmcblk%d",
							mount_type, block,  block);
			} else {
				sprintf(target, "mount -o rw,remount -t %s /dev/mmcblk%dp%d /media/mmcblk%dp%d",
							mount_type, block, partition,  block, partition);
			}
			system(target);
			if (file_exist(target)){
				printf("2nd unlink fail return %d and errno is %d (%s)\n",ret,errno,strerror(errno));
				return -1;
			}*/
		}
#endif //CHECK_SD_RW_RO    
	}
	else 
		return -1;

	fprintf(stderr, "rm oldest file %s \n", syscmd);
	
	return 0;
	
}

int get_sd_available(void)
{
	return sd_insert_init_done;
}

void stop_all_record_action(void)
{
	if(record_status) {
		if(record_status & RECORD_TRIG)
			set_record_stop(sd_record, RECORD_CHANNEL_SCHED);
		if (sd_alarm_record_en) {		//if Motion detection record enable
			if(record_status & ALARM_TRIG){
				//set_record_stop(sd_record, RECORD_CHANNEL_MD);
				md_record_status = 0;
				set_record_stop(sd_record, RECORD_CHANNEL_MD);
				while(get_record_state(sd_record, RECORD_CHANNEL_MD));
				read_write_pos_sync(video_pre_buffer);
				read_write_pos_sync(audio_pre_buffer);
			}
		}
	}

}

int get_close_trig(void)
{
	return clos_trig;
}

void set_close_trig(int val)
{
	clos_trig = val;
}

int get_md_close_trig(void)
{
	return md_clos_trig;
}

void set_md_close_trig(int val)
{
	md_clos_trig = val;
}


void up_record_count(void)
{
	if(record_status & RECORD_TRIG) {
		accmulation_fp++;		
	}
	//fprintf(stderr, "accmulation_fp = %d \n", accmulation_fp);

	if(record_status & ALARM_TRIG) {
		md_accmulation_fp++;		
	}
	//fprintf(stderr, "md_accmulation_fp = %d \n", md_accmulation_fp);
}

int get_record_count(void)
{
	if(record_status & RECORD_TRIG) {
		return accmulation_fp;
	}
	else
		return 0;
}

int get_mdrecord_count(void)
{
	if(record_status & ALARM_TRIG) {
		return md_accmulation_fp;
	}
	else
		return 0;
}


unsigned long long	audio_accumlation_duration = 0; 
unsigned long long	pre_audio_reach_time =  0; 

unsigned long long	md_audio_accumlation_duration = 0; 
unsigned long long	md_pre_audio_reach_time =  0; 

void write_done_file(void)
{

	pthread_mutex_lock(&audio_sync_lock); 
	accmulation_fp = 0;
	audio_accumlation_duration = 0;
	pre_audio_reach_time = 0;
	pthread_mutex_unlock(&audio_sync_lock);	

	fprintf(stderr, "[%s]accmulation_fp = %d audio_accumlation_duration = %d\n", __FUNCTION__, accmulation_fp, audio_accumlation_duration);
}

void write_done_md_file(void)
{
	
	pthread_mutex_lock(&md_audio_sync_lock); 
	md_accmulation_fp = 0;
	md_audio_accumlation_duration = 0;
	md_pre_audio_reach_time = 0;
	pthread_mutex_unlock(&md_audio_sync_lock);	

	fprintf(stderr, "[%s]md_accmulation_fp = %d md_audio_accumlation_duration = %d\n", __FUNCTION__, md_accmulation_fp, md_audio_accumlation_duration);
}

/* record handle thread */
void *hanlde_record_thr(void *param)
{
	int sd_current_status = 0;		// sd current status
	int is_first_insert = 0;		// check sd is frist insert or not
	int ret = 0;
	int blk_id = 0;
	int partition_id = 0;
	int alarm_free_size = 0;
	int record_free_size = 0;
	int other_used = 0;
	char path[128];
	struct SD_FS sd_fs;
	static time_t last_md_time = 0;
	static time_t cur_md_time = 0;
	static time_t last_shed_time = 0;
	static time_t cur_shed_time = 0;
	int other_used_low_limit;
	int size = 0;

	reserved_alarm_size_mb = 1;
	reserved_record_size_mb = 5;
	record_status = 0;
	
//	while(get_sync_time() == 0) {
//		sleep(1);
		 if (!isRecordUsed)
			return NULL;
//	}

	sd_record = create_recording(RECORD_CONFIG);
	if(sd_record == NULL) {
		fprintf(stderr, "sd record init failed!! \n");
		return NULL;
	}
	if (sd_alarm_record_en) {		//if Motion detection record enable
		video_pre_buffer = init_av_buffer(VIDEO_PRE_BUFFER_SIZE, USED_VIDEO_PRE_BUF_NUM, MAX_VIDEO_PRE_BUF_NUM);
		if(video_pre_buffer == NULL){
			fprintf(stderr, "Video pre buffer init error\n");
			return NULL;
		}
		audio_pre_buffer = init_av_buffer(AUDIO_PRE_BUFFER_SIZE, USED_AUDIO_PRE_BUF_NUM, MAX_AUDIO_PRE_BUF_NUM);
		if(audio_pre_buffer == NULL){
			fprintf(stderr," Audio pre buffer init error\n");
			return NULL;
		}
	}
	while(isRecordUsed) {
		if(check_sd_workable() == 1) {
			if(sd_current_status == 0) {
				sd_current_status = 1;
				is_first_insert = 1;
			}
			
			/* first time insert */
			if(is_first_insert) {
				ret = get_first_available_partition(&blk_id, &partition_id, &sd_fs);
				if(ret == -1) {
					is_first_insert = 1;
					goto WAIT_STAGE;
				}
				fprintf(stderr, "SD insert!! \n");

				is_first_insert = 0;
				fprintf(stderr, "partition id = %d with total size = %d (MB) and free size = %d (MB)\n", 
					partition_id, sd_fs.total_size, sd_fs.total_free_size);
				/* check record and alarm folder size */

				check_SD_EXAMPLE_folder(blk_id, partition_id);

				ret = get_folder_info(blk_id, partition_id, SD_REC_SCHED, &sd_fs);
				if(ret == -1) {
					is_first_insert = 1;
					goto WAIT_STAGE;
				}
				
				if (sd_alarm_record_en) {		//if Motion detection record enable
					ret = get_folder_info(blk_id, partition_id, SD_REC_ALARM, &sd_fs);
					if(ret == -1) {
						is_first_insert = 1;
						goto WAIT_STAGE;
					}
				}

				fprintf(stderr, "record used size = %d, md used size = %d \n",
					sd_fs.total_record_size, sd_fs.total_alarm_size);

				other_used = sd_fs.total_record_size + sd_fs.total_alarm_size + sd_fs.total_free_size;

				fprintf(stderr, "available SD space = %d \n", other_used);

#if 0				
				if(snx_get_file_value(ETC_OTHER_USED_LOW_LIMIT, &other_used_low_limit,10) == -1){
					other_used_low_limit=DEFAULT_OTHER_USED_LOW_LIMIT_VALUE;
				}
				
				if ( other_used < other_used_low_limit ) {
					fprintf(stderr, "\n\n");
					fprintf(stderr, "SD card available space is smaller than %ldMBytes!!\n",other_used_low_limit );
					fprintf(stderr, "Please clear some space or change another enough space SD card!!\n" );
					fprintf(stderr, "\n\n");
					is_first_insert = 1;
					goto WAIT_STAGE;
				}
#endif

				if(sd_fs.total_record_size == 0 && sd_fs.total_alarm_size == 0) {
					sd_fs.total_record_size = sd_fs.total_free_size * SD_RECORD_UPBD / 100;
					sd_fs.total_alarm_size = sd_fs.total_free_size * SD_ALARM_UPBD / 100;
				}
				else {
					sd_fs.total_record_size = 
						other_used * SD_RECORD_UPBD / 100;

					sd_fs.total_alarm_size = 
						other_used * SD_ALARM_UPBD / 100;
				}

				sd_fs.total_free_record_size = sd_fs.total_record_size;
				size = get_current_used_space(blk_id, partition_id, SD_REC_SCHED);
				if (size == -1) {
					is_first_insert = 1;
					goto WAIT_STAGE;
				}
				sd_fs.total_free_record_size = sd_fs.total_free_record_size - size;

				if (sd_alarm_record_en) {		//if Motion detection record enable
					sd_fs.total_free_alarm_size = sd_fs.total_alarm_size;
					size = get_current_used_space(blk_id, partition_id, SD_REC_ALARM);
					if (size == -1) {
						is_first_insert = 1;
						goto WAIT_STAGE;
					}
					sd_fs.total_free_alarm_size = sd_fs.total_free_alarm_size -size ;
				}
				
				fprintf(stderr, "[REARR]record total free size = %d, md total free size = %d \n",
					sd_fs.total_free_record_size, sd_fs.total_free_alarm_size);

				
				/* set path parameter */
				memset(path, 0x00, sizeof(path));
				if(partition_id == -1)
					sprintf(path, "/media/mmcblk%d/%s/record/", blk_id, SD_EXAMPLE);
				else
					sprintf(path, "/media/mmcblk%dp%d/%s/record/", blk_id, partition_id, SD_EXAMPLE);
				
				write_config(RECORD_CONFIG, sd_record->record_config, RECORD_CHANNEL_SCHED, "path", path);
				set_record_path(sd_record, RECORD_CHANNEL_SCHED, path);

				memset(path, 0x00, sizeof(path));
				if(partition_id == -1)
					sprintf(path, "/media/mmcblk%d/%s/md/", blk_id, SD_EXAMPLE);
				else
					sprintf(path, "/media/mmcblk%dp%d/%s/md/", blk_id, partition_id, SD_EXAMPLE);
				
				write_config(RECORD_CONFIG, sd_record->record_config, RECORD_CHANNEL_MD, "path", path);
				set_record_path(sd_record, RECORD_CHANNEL_MD, path);
				
				sd_insert_init_done = 1;
				
#if SD_INSERT_RECORD_EN

//Legacy Scheduling record control
#if 0 
				/* startup record */
				set_record_start(sd_record, RECORD_CHANNEL_SCHED);
				// reset accmulation_fp
				//accmulation_fp = 0;
				
				pthread_mutex_lock(&audio_sync_lock); 		//haoweilo
				accmulation_fp = 0;
				audio_accumlation_duration = 0;
				pre_audio_reach_time = 0;
				pthread_mutex_unlock(&audio_sync_lock);		//haoweilo

				record_status |= RECORD_TRIG;
				last_shed_time = time(NULL);
				fprintf(stderr, "record_status = %d \n", record_status);
#endif
#endif
			}
			else {
				
				if (sd_alarm_record_en) {		//if Motion detection record enable
					if(get_alarm_trig()) {
						if(record_status & ALARM_TRIG) {
							fprintf(stderr, "Alarm continue!\n");
						}
						else {
							fprintf(stderr, "alarm record start \n");
							
							

							//pthread_mutex_lock(&md_audio_sync_lock); 		//haoweilo
							md_accmulation_fp = 0;
							md_audio_accumlation_duration = 0;
							md_pre_audio_reach_time = 0;
							//pthread_mutex_unlock(&md_audio_sync_lock);		//haoweilo

							record_status |= ALARM_TRIG;
							//last_md_time = time(NULL);
							set_alarm_trig(0);
							set_record_start(sd_record, RECORD_CHANNEL_MD);
							md_record_status = 1;
						}
					}
					
					if(record_status & ALARM_TRIG) {

						if(get_md_close_trig()) {
						//cur_md_time = time(NULL);
						//if(cur_md_time - last_md_time >= MOTION_TIME_INTERVAL) {
							md_record_status = 0;
							//haowei comment
							//set_record_stop(sd_record, RECORD_CHANNEL_MD);
							while(get_record_state(sd_record, RECORD_CHANNEL_MD));
							read_write_pos_sync(video_pre_buffer);
							read_write_pos_sync(audio_pre_buffer);
							fprintf(stderr, "alarm record end \n");
							record_status &= ~ALARM_TRIG;

							set_md_close_trig(0);
						//	last_md_time = cur_md_time;
						}
					}
				}

				if(record_status & RECORD_TRIG) {
					if(get_close_trig()) {
						while(get_record_state(sd_record, RECORD_CHANNEL_SCHED));
						fprintf(stderr, "record end \n");
						record_status &= ~RECORD_TRIG;
						set_close_trig(0);

						/* start another avi */
						set_record_start(sd_record, RECORD_CHANNEL_SCHED);
						
						//accmulation_fp = 0;						//haoweilo

						pthread_mutex_lock(&audio_sync_lock); 		//haoweilo
						accmulation_fp = 0;
						audio_accumlation_duration = 0;
						pre_audio_reach_time = 0;
						pthread_mutex_unlock(&audio_sync_lock);		//haoweilo
						
						record_status |= RECORD_TRIG;
					}
#if 0
					cur_shed_time =  time(NULL);
					if(cur_shed_time - last_shed_time >= 600) { // cut the 10 min segment
						set_record_stop(sd_record, RECORD_CHANNEL_SCHED);
						while(get_record_state(sd_record, RECORD_CHANNEL_SCHED));
						fprintf(stderr, "record end \n");
						record_status &= ~RECORD_TRIG;
						last_shed_time = cur_shed_time;

						/* start another avi */
						set_record_start(sd_record, RECORD_CHANNEL_SCHED);
						record_status |= RECORD_TRIG;

					}
#endif
				}

				//fprintf(stderr, "record_status = %d \n", record_status);
				/* check alarm and record space */

				if (sd_alarm_record_en) {		//if Motion detection record enable
					size = get_current_used_space(blk_id, partition_id, SD_REC_ALARM);
					if (size == -1) {
						is_first_insert = 1;
						goto WAIT_STAGE;
					}
					alarm_free_size = sd_fs.total_alarm_size - size;
					if(alarm_free_size < reserved_alarm_size_mb) {
						/* remove the oldest file */
						fprintf(stderr, "Warning: To remove the oldest file from alarm \n");
						ret = rm_oldest_file(blk_id, partition_id, SD_REC_ALARM);
						if (ret == -1) {
							is_first_insert = 1;
							goto WAIT_STAGE;
						}
					}
				}

				size = get_current_used_space(blk_id, partition_id, SD_REC_SCHED);
				if (size == -1) {
					is_first_insert = 1;
					goto WAIT_STAGE;
				}
				record_free_size = sd_fs.total_record_size - size;
				if(record_free_size < reserved_record_size_mb) {
					/* remove the oldest file */ 
					fprintf(stderr, "Warning: To remove the oldest file from record \n");
					ret = rm_oldest_file(blk_id, partition_id, SD_REC_SCHED);
					if (ret == -1) {
						is_first_insert = 1;
						goto WAIT_STAGE;
					}

				}
				
#ifdef SD_RECORD_DBG
				if(dbg_print_cnt == 15) {
					fprintf(stderr, "\n [record_status: %d] alarm_free_size = %d, record_free_size = %d \n", record_status, alarm_free_size, record_free_size);
					dbg_print_cnt = 0;

				}
				dbg_print_cnt++;
#endif
			}


/* Start to Scheduling Recording */
			if (sd_record_en) {			//haoweilo
#if SD_INSERT_RECORD_EN
				if (sd_record_start == 0) {
					/* startup record */
					set_record_start(sd_record, RECORD_CHANNEL_SCHED);
					// reset accmulation_fp
					pthread_mutex_lock(&audio_sync_lock); 		//haoweilo
					accmulation_fp = 0;
					audio_accumlation_duration = 0;
					pre_audio_reach_time = 0;
					pthread_mutex_unlock(&audio_sync_lock); 		//haoweilo
					record_status |= RECORD_TRIG;
					last_shed_time = time(NULL);
					sd_record_start = 1;
					fprintf(stderr, "record_status = %d \n", record_status);
				}
		
			} else {
				if (sd_record_start) {
					set_record_stop(sd_record, RECORD_CHANNEL_SCHED);
					sd_record_start = 0;
				}
#endif
			}

		}	// SD workable 
		else {
			sd_current_status = 0;
			if(sd_insert_init_done) 
				sd_insert_init_done = 0;

			if(record_status) {
				fprintf(stderr, "SD isn't workable!! \n");
			
				if(record_status & RECORD_TRIG) {
					set_record_stop(sd_record, RECORD_CHANNEL_SCHED);
					sd_record_start = 0;
				}
				
				if (sd_alarm_record_en) {		//if Motion detection record enable		
					if(record_status & ALARM_TRIG){
						//set_record_stop(sd_record, RECORD_CHANNEL_MD);
						md_record_status = 0;
						set_record_stop(sd_record, RECORD_CHANNEL_MD);
						while(get_record_state(sd_record, RECORD_CHANNEL_MD));

						read_write_pos_sync(video_pre_buffer);
						read_write_pos_sync(audio_pre_buffer);
					}
				}
				record_status = 0;

				//system("sync");
				sleep(1);
				memset(path, 0x00, sizeof(path));
				if(partition_id == -1)
					sprintf(path, "umount /media/mmcblk%d", blk_id);
				else
					sprintf(path, "umount /media/mmcblk%dp%d", blk_id, partition_id);
				system(path);
				memset(path, 0x00, sizeof(path));
				if(partition_id == -1)
					sprintf(path, "rm -rf /media/mmcblk%d", blk_id);
				else
					sprintf(path, "rm -rf /media/mmcblk%dp%d", blk_id, partition_id);
				system(path);
				fprintf(stderr, "record_status = %d \n", record_status);
			}
		}
WAIT_STAGE:
		sleep(1);
	} //while
	if(sd_insert_init_done)
		sd_insert_init_done = 0;
	
	/* if receive kill signal */
	if(record_status) {
		
		if(record_status & RECORD_TRIG)
			set_record_stop(sd_record, 1);
		
		if (sd_alarm_record_en) {		//if Motion detection record enable
			if(record_status & ALARM_TRIG){
				//set_record_stop(sd_record, 2);
				md_record_status = 0;
				set_record_stop(sd_record, RECORD_CHANNEL_MD);
				while(get_record_state(sd_record, RECORD_CHANNEL_MD));
				read_write_pos_sync(video_pre_buffer);
				read_write_pos_sync(audio_pre_buffer);
			}
		}
		
		record_status = 0;
		
		//system("sync");
		/*
		memset(path, 0x00, sizeof(path));
		sprintf(path, "umount /media/mmcblk%dp%d", blk_id, partition_id);
		system(path);
		memset(path, 0x00, sizeof(path));
		sprintf(path, "rm -rf /media/mmcblk%dp%d", blk_id, partition_id);
		system(path);
		*/
	}

	return NULL;
}


int start_record_loop(void)
{
	int ret = 0;
	isRecordUsed = 1;

	start_audio_record();

	if(pthread_create(&record_id, NULL, hanlde_record_thr, NULL) != 0) {
		fprintf(stderr,"[record]pthread_create failed\n");
		return -1;
	}
	
	fprintf(stderr, "start_record_loop \n");
	return 0;
}

void destory_record_loop(void)
{
	isRecordUsed = 0;
	pthread_join(record_id, NULL);
	stop_audio_record();
	destory_recording(sd_record);
	sd_record = NULL;
	if(audio_pre_buffer)
		deinit_av_buffer(audio_pre_buffer);

	if(video_pre_buffer)
		deinit_av_buffer(video_pre_buffer);
	
	fprintf(stderr, "destory_record_loop done\n");
}


static void audio_record__data_cb(struct sonix_audio *audio, 
			const struct timeval *tv, 
			void *data, 
			size_t len, 
			void *cbarg)
{
	int dummy_flag = 1;
#if AUDIO_DUMMY_FRAME_INSERT 
	unsigned long long audio_cur_time = 0;
	unsigned long long av_interval_time = 0;
	unsigned long long  video_time;
	int arrival_time = 0;
	unsigned long long tmp_accu_duration = 0;
#endif

#if USE_SD_RECORD

	if(sd_record != NULL) {
		
		
#if AUDIO_DUMMY_FRAME_INSERT

		if(record_status & RECORD_TRIG) {
				if(pre_audio_reach_time == 0) {
					pre_audio_reach_time = tv->tv_sec * 1000 + (tv->tv_usec/1000);
				}
				else { //calculate how much frames we lost during SD encode to reach FPS
					audio_cur_time = tv->tv_sec * 1000 + (tv->tv_usec/1000);
					if (audio_cur_time > pre_audio_reach_time)
						arrival_time = audio_cur_time - pre_audio_reach_time;
					else
						arrival_time = AUDIO_COUNT_TIME_INTERVAL;
					
					pthread_mutex_lock(&audio_sync_lock); 			//haoweilo
					audio_accumlation_duration += arrival_time;
					pre_audio_reach_time = audio_cur_time;
					pthread_mutex_unlock(&audio_sync_lock); 		//haoweilo
				}

				
				video_time = (get_record_count() * 1000) / avhandler_get_codecfps() ;

				//printf("arrival : %d\n", arrival_time);
				//printf("video_time : %d, audio_accumlation_duration: %d\n", video_time, audio_accumlation_duration);
				pthread_mutex_lock(&audio_sync_lock); 			//haoweilo
				tmp_accu_duration = audio_accumlation_duration;
				pthread_mutex_unlock(&audio_sync_lock); 		//haoweilo

				if (video_time >= tmp_accu_duration) {

					save_data(sd_record, 1, data, len, "audio");
					
					av_interval_time = video_time - tmp_accu_duration;
					
					if (av_interval_time > AUDIO_INSERT_THRESHOLD) {

						tmp_accu_duration += AUDIO_COUNT_TIME_INTERVAL;
						
						save_data(sd_record, 1, data, len, "audio");
					}

				} else {
					av_interval_time = tmp_accu_duration - video_time;
					if (av_interval_time > AUDIO_SKIP_THRESHOLD ) {

						tmp_accu_duration -= AUDIO_COUNT_TIME_INTERVAL;
					} else {

						save_data(sd_record, 1, data, len, "audio"); 
					}
				}

				pthread_mutex_lock(&audio_sync_lock); 			//haoweilo
				audio_accumlation_duration = tmp_accu_duration;
				pthread_mutex_unlock(&audio_sync_lock); 		//haoweilo
				
		} else {
			pthread_mutex_lock(&audio_sync_lock); 			//haoweilo
			audio_accumlation_duration = 0;
			pre_audio_reach_time = 0;
			pthread_mutex_unlock(&audio_sync_lock); 		//haoweilo
		}
			//printf("video fpscount: %d, audio count: %d\n", get_record_count(),audio_local_fp);

#else	// !AUDIO_DUMMY_FRAME_INSERT
		save_data(sd_record, 1, data, len, "audio");
#endif

		if (sd_alarm_record_en) {		//if Motion detection record enable

#if 0			
#if AUDIO_DUMMY_FRAME_INSERT

			if(record_status & ALARM_TRIG) {
					if(md_pre_audio_reach_time == 0) {
						md_pre_audio_reach_time = tv->tv_sec * 1000 + (tv->tv_usec/1000);
					}
					else { //calculate how much frames we lost during SD encode to reach FPS
						audio_cur_time = tv->tv_sec * 1000 + (tv->tv_usec/1000);
						if (audio_cur_time > md_pre_audio_reach_time)
							arrival_time = audio_cur_time - md_pre_audio_reach_time;
						else
							arrival_time = AUDIO_COUNT_TIME_INTERVAL;
						
						pthread_mutex_lock(&md_audio_sync_lock); 			//haoweilo
						md_audio_accumlation_duration += arrival_time;
						md_pre_audio_reach_time = audio_cur_time;
						pthread_mutex_unlock(&md_audio_sync_lock); 		//haoweilo
					}
					
					video_time = (get_mdrecord_count() * 1000) / avhandler_get_codecfps() ;

					//printf("arrival : %d\n", arrival_time);
					//printf("video_time : %d, md_audio_accumlation_duration: %d\n", video_time, md_audio_accumlation_duration);
					pthread_mutex_lock(&md_audio_sync_lock); 			//haoweilo
					tmp_accu_duration = md_audio_accumlation_duration;
					pthread_mutex_unlock(&md_audio_sync_lock); 		//haoweilo

					if (video_time >= tmp_accu_duration) {
						
						av_interval_time = video_time - tmp_accu_duration;
						
						if (av_interval_time > AUDIO_INSERT_THRESHOLD) {

							tmp_accu_duration += AUDIO_COUNT_TIME_INTERVAL;

							dummy_flag = 2;
							//printf("Insert audio frame\n");
						} else {
							dummy_flag = 1;
						}

					} else {
						av_interval_time = tmp_accu_duration - video_time;
						if (av_interval_time > AUDIO_SKIP_THRESHOLD ) {

							tmp_accu_duration -= AUDIO_COUNT_TIME_INTERVAL;
							dummy_flag = 0;
							//printf("skip audio frame\n");
						} else {

							dummy_flag = 1;
						}
					}

					pthread_mutex_lock(&md_audio_sync_lock); 			//haoweilo
					md_audio_accumlation_duration = tmp_accu_duration;
					pthread_mutex_unlock(&md_audio_sync_lock); 		//haoweilo
					
			} else {
				pthread_mutex_lock(&md_audio_sync_lock); 			//haoweilo
				md_audio_accumlation_duration = 0;
				md_pre_audio_reach_time = 0;
				pthread_mutex_unlock(&md_audio_sync_lock); 		//haoweilo
			}

#endif
#endif
			while (dummy_flag > 0) {

				write_buffer_data(audio_pre_buffer, (char *)data, len);
				dummy_flag--;
			}

			if(md_record_status == 1){
				char *tmp_data;
				int tmp_size;
				int i;
				for(i = 0; i < 4; i++){
					read_buffer_data(audio_pre_buffer, &tmp_data, &tmp_size);
					if(tmp_size > 0)
						save_data(sd_record, 2,(void *) tmp_data, tmp_size, "audio");
				}
			}
		}
	}
#endif
	return;
}

static int start_audio_record(void)
{
	int rc = 0;

	//! audio input data callback 
	if (!(audio_record = snx98600_record_audio_new(AUDIO_RECORD_DEV, audio_record__data_cb, NULL))) {
		rc = errno ? errno : -1;
		fprintf(stderr, "failed to create audio source: %s\n", strerror(rc));
		goto finally;
	}
//	SetMicVolume(100);
#if TIMELAPSE_SUPPORT

	if(timelapse) {
		fprintf(stderr, "Timelasp_enable, NO audio start\n");
	} else {

#endif

		if ((rc = snx98600_record_audio_start(audio_record))) {
			fprintf(stderr, "failed to start audio source: %s\n", strerror(rc));
			goto finally;
		}

#if TIMELAPSE_SUPPORT

	}
#endif

finally:
	return rc;

}

static int stop_audio_record(void)
{
	int rc = 0;

	fprintf(stderr, "%s \n", __FUNCTION__);

#if TIMELAPSE_SUPPORT
	if(timelapse) {
		fprintf(stderr, "Timelasp_enable, NO audio start\n");
	} else {
#endif

		if ((rc = snx98600_record_audio_stop(audio_record))) {
			fprintf(stderr, "failed to start audio source: %s\n", strerror(rc));
			goto finally;
		}
		
#if TIMELAPSE_SUPPORT
	}
#endif
	if (audio_record) {
		snx98600_record_audio_free(audio_record);
		audio_record = NULL;
	}
	  
finally:
	return rc;
}



