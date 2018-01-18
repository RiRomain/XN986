#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
//alek add
#include <sys/mman.h>
#include <semaphore.h>
#define ALL_AT_ONCE   1      //now all at onec

//add by xyd
#define __LIBNVRAM__
#include "nvram.h"

#define BUFF_LINE 		256
#define ALL_FILE 		0x1
#define NV_COMMIT		0x2

#define POINT_MARK		"__POINT__"
#define POINT			'.'
#define SKEWLINE_MARK	"__SKEWLINE__"
#define SKEWLINE		'/'
#define LINE_MARK		"__SHORTLINE__"
#define LINE_CHAR		'-'
//xyd



nvram_info_t    *nv_info;
int             nv_info_id;
sem_t           *mutex;
unsigned long nvr_va;


/* debug */
#define LIBNVRAM_DEBUG  0
#define LIBNV_PRINT(x, ...) do { if (LIBNVRAM_DEBUG) printf("%s %d: " x, __FILE__, __LINE__, ## __VA_ARGS__); } while(0)
#define LIBNV_ERROR(x, ...) do { fprintf(stderr,"%s %d: ERROR! " x, __FILE__, __LINE__, ## __VA_ARGS__); } while(0)

#if 0
void update_nv_info(INFO info)
{

#ifndef ALL_AT_ONCE 
  // should be run in mutex
  config_info_t   *config_info_p;
  config_info_p = info->state;                    //point to the config_info
  config_info_p->offset = info->offset;
  config_info_p->size = GET_INFO_SIZE(info->misc);

#endif
}
#endif

/* 
 0. allocate nvram table for the first time use
 1. init nvram table
 2. get the fd of the nvram cache
*/
int nvram_init(int nvram_id)
{
  int             fd_nv_dev;
  key_t           nv_key;
  nvram_ioctl_t   nvr;

	LIBNV_PRINT("--> nvram_init\n");
    
  if (nvram_id != NVRAM_ID) {
    LIBNV_ERROR("nvram ID dis-match\n");
    return -1;
  }
    
  fd_nv_dev = open(NV_DEV,  O_RDWR );
  if (fd_nv_dev < 0) {
    perror(NV_DEV);
    return -1;
  }
    memset(&nvr,0,sizeof(nvram_ioctl_t));
  nvr.param1 = NVRAM_ID;
    
  // Check Lib and Flash NVRAM ID
  
  if (ioctl(fd_nv_dev, SONIX_NVRAM_IOCTL_INIT, &nvr) < 0) {
    perror("ID check failed");
    close(fd_nv_dev);
    return -1;
  } 
  // CRC check error 
  if (nvr.param2 < 0) {
    LIBNV_ERROR("CRC check error in NVRAM\n");
    close(fd_nv_dev);
    return -1;
  }

  sem_unlink("nvram_info");
  //avoid rentrant problem
  mutex = sem_open("nvram_info", O_CREAT, 0644, 1);
  if(mutex == SEM_FAILED) {
    LIBNV_ERROR("MUTEX OPEN Failed\n");
    close(fd_nv_dev);
    return -1;
  }
    
  nv_key  = ftok(".", NVRAM_ID);
    
  nv_info_id = shmget(nv_key, sizeof(nvram_info_t),0644|IPC_CREAT);
    
  if (nv_info_id < 0) {
    LIBNV_ERROR("SHM OPEN Failed\n");
    close(fd_nv_dev);
    return -1;
  }
    
  sem_wait(mutex);
    
  nv_info = (nvram_info_t *) shmat(nv_info_id, NULL, 0);
 // memset(nv_info,0,sizeof(nvram_info_t));
//  printf("%d %x\n",nv_info->nvr_id,nv_info->nvr_va);  
  if ((nv_info->nvr_id == 0)  || (nvr_va == 0)  || (nvr_va == 0xffffffff)) {             //First init
    nv_info->nvr_id = NVRAM_ID;
    //LIBNV_PRINT ("sizeof(nvram_t) = %d\n",sizeof(nvram_t));
    
    nvr_va =(unsigned long) mmap(NULL, sizeof(nvram_t), PROT_READ | PROT_WRITE , MAP_SHARED, fd_nv_dev, 0);
    
  }
    
  sem_post(mutex);
	close(fd_nv_dev);
  return nv_info->nvr_id;
}

/*
    Free the nvram table when init_cout = 0
    Commit all modification.
*/
int nvram_close(void)
{
  int             count;
  struct shmid_ds ds_buf;
    
	LIBNV_PRINT("--> nvram_close\n");
    
	
  if ((nv_info == NULL) || (mutex == NULL)) {
    LIBNV_ERROR("Please call init first\n");
    return -1;
  }
    
  //Check NVRAM version between Libs and cache
  if ((nv_info->nvr_id == 0) || (nv_info->nvr_id != NVRAM_ID)) {
    LIBNV_ERROR("NVRAM ID does not match!\n");
    return -1;
  }
    
    
  sem_wait(mutex);
  if(shmctl(nv_info_id, IPC_STAT, &ds_buf) < 0) {
    LIBNV_ERROR("share memory ctl failed!\n");
    return -1;
  }
  sem_post(mutex);
    
    
  if(ds_buf.shm_nattch == 1) {
        
    sem_wait(mutex);
    munmap((void *)(nvr_va),sizeof(nvram_t));
    nvr_va=0;
    if(shmdt(nv_info) < 0) {
    LIBNV_ERROR("share memory deattach failed!\n");
    return -1;
    }
    shmctl(nv_info_id, IPC_RMID,NULL);
    sem_post(mutex);
    sem_close(mutex);
  } else {
    LIBNV_PRINT("number of attached share memory: %d\n", ds_buf.shm_nattch);
  }
    
    
  return 0;
}

/*
    nvram_get
    Check the state of config to decide if do CRC CHECK or not.
    Config state update.
*/


void nvram_get(INFO* info,void* data)
{
  char            state;
  char            type;
  unsigned int    size;

  if(!(info->offset==0&&GET_INFO_SIZE(info->misc)==sizeof(nvram_t))
  	&&(info->offset < sizeof(unsigned int) || 
    GET_INFO_SIZE(info->misc) > sizeof(nvram_t))){
    LIBNV_ERROR("INFO value out of size error\n");
   // return -1;
   //modify by xyd
   return;
  }
  if ((nv_info == NULL) || (mutex == NULL)) {
    LIBNV_ERROR("Please call init first\n");
    return ;
  }
    
  //Check NVRAM version between Libs and cache
  if ((nv_info->nvr_id == 0) ||
    (nv_info->nvr_id != NVRAM_ID)) {
    LIBNV_ERROR("NVRAM ID does not match!\n");
    return ;
  }
    
  type = GET_INFO_TYPE(info->misc);
  size = GET_INFO_SIZE(info->misc);
//  printf("size = %x\n",size);
  sem_wait(mutex);
  if(info->offset==0&&GET_INFO_SIZE(info->misc)==sizeof(nvram_t))
  	state=CONF_STATE_VALID;
  else
  state = *(info->state);
  
  sem_post(mutex);
    
  if(type == CONFIG_TYPE) {
    LIBNV_PRINT("NVRAM config get ...\n");
  } else {
    LIBNV_PRINT("NVRAM Element get ...\n");
  }
  
	//check the state of the config
  switch (state)
  {
    case CONF_STATE_VALID:
    case CONF_STATE_MODIFIED:
		
      memcpy(data, (void* )(nvr_va + info->offset), size);
      		
      break;
    case CONF_STATE_FAILED:
      LIBNV_ERROR("BAD CRC\n");
      return ;
    default:
      LIBNV_ERROR("Wrong config STATE\n");
      return ;
  }
	return ;
}

/*
    nvram_set
    Check the state of config to decide if do CRC check or not.
    Config state update.
*/
int nvram_set(INFO* info, void *data)
{
  char            state;
  unsigned int    size;
  char            type;

  if(info->offset < sizeof(unsigned int) || 
    GET_INFO_SIZE(info->misc) > sizeof(nvram_t)-sizeof(unsigned int)){
   // printf("0x%x 0x%x\n",info->offset,GET_INFO_SIZE(info->misc));
    LIBNV_ERROR("INFO value out of size error\n");
    return -1;
  }  
  if ((nv_info == NULL) || (mutex == NULL)) {
    LIBNV_ERROR("Please call init first\n");
    return -1;
  }
    
  //Check NVRAM version between APPs and Libs
  if ((nv_info->nvr_id == 0) || (nv_info->nvr_id != NVRAM_ID)) {
    LIBNV_ERROR("NVRAM ID does not match!\n");
    return -1;
  }
   
    
  type = GET_INFO_TYPE(info->misc);
  size = GET_INFO_SIZE(info->misc);
  
    
  sem_wait(mutex);
   
  state = *(info->state);

  sem_post(mutex);
   
  if(type == CONFIG_TYPE) {
    LIBNV_PRINT("NVRAM config set ...\n");
  } else {
    LIBNV_PRINT("NVRAM Element set ...\n");
  }
            
	//check the state of the config
  switch (state)
  {
    case CONF_STATE_VALID:
      sem_wait(mutex);
      *(info->state) = CONF_STATE_MODIFIED;
      sem_post(mutex);
            
    case CONF_STATE_MODIFIED:
        memcpy((void* )(nvr_va + info->offset), data, size);         
        break;
    case CONF_STATE_FAILED:
        LIBNV_ERROR("BAD CRC\n");
        break;
    default:
        LIBNV_ERROR("Wrong config STATE\n");
        return -1;
  }
	return 0;
}


/*
 * write flash from cache
 * Check the state of all configs.
 * Commit if it is modified, otherwise we do nothing.
 */
int nvram_commit(INFO* info)
{
  int             fd_nv_dev;
	config_info_t   *tmp_nv_p;
  nvram_ioctl_t   nvr;
  char            type;
  unsigned int    size;
  int             cnt;
//  if (info == NULL) {
//    LIBNV_ERROR("NULL info\n");
//    return -1;
//  }
  if((info->offset != 0) || (info->misc < (sizeof(nvram_t)))){
    LIBNV_ERROR("commit only support commit all\n");
    info->offset = 0;
    info->misc = sizeof(nvram_t);
  } 
    
  if ((nv_info == NULL) || (mutex == NULL)) {
    LIBNV_ERROR("Please call init first\n");
    return -1;
  }
    
  if (info->misc == 0) {
    LIBNV_ERROR("WRONG info\n");
    return -1;
  }
    
  //Check NVRAM version between APPs and Libs
  if ((nv_info->nvr_id == 0) || (nv_info->nvr_id != NVRAM_ID)) {
    LIBNV_ERROR("NVRAM ID does not match!\n");
    return -1;
  }
  //printf("nvram_commit 0\n");  
  fd_nv_dev = open(NV_DEV, O_RDWR);
  if (fd_nv_dev < 0) {
    perror(NV_DEV);
    return -1;
  }
    
  type = GET_INFO_TYPE(info->misc);
  size = GET_INFO_SIZE(info->misc);
  //printf("nvram_commit 1\n");   
  //Commit all NVRAM cache
  if(info->offset == 0 && size == sizeof(nvram_t)) {

#ifdef ALL_AT_ONCE
        //COMMIT all config at once
    nvr.param1 = 0;         //offset
    nvr.param2 = size;      //size
    //printf("nvram_commit 2\n");             
    if (ioctl(fd_nv_dev, SONIX_NVRAM_IOCTL_COMMIT, &nvr) < 0) {
        
      sem_post(mutex);
     // perror("ioctl");
      close(fd_nv_dev);
      return -1;
    }

#endif
    tmp_nv_p = (config_info_t*) nv_info;
    //check every config state
    for(cnt=0;cnt < IPCAM_CON_NUM;cnt++, tmp_nv_p++) {
            
    //Only commit the modified config
      sem_wait(mutex);
      if(tmp_nv_p->state == CONF_STATE_MODIFIED) {
#ifndef ALL_AT_ONCE
      //COMMIT all by parsing each config state.
        nvr.param1 = tmp_nv_p->offset;      //offset
        nvr.param2 = tmp_nv_p->size;        //size
                
        if (ioctl(fd_nv_dev, SONIX_NVRAM_IOCTL_COMMIT, &nvr) < 0) {
          sem_post(mutex);
          perror("ioctl");
          close(fd_nv_dev);
          return -1;
        }
#endif
        tmp_nv_p->state = CONF_STATE_VALID;
        sem_post(mutex);
      } else {
        sem_post(mutex);
        continue;
      }
    }
    
  } else {
    // commit the specified config
    if (type != CONFIG_TYPE) {
      LIBNV_ERROR("Allow CONFIG commit only\n");
      return -1;
    }
        
    nvr.param1 = info->offset;      //offset
    nvr.param2 = size;              //size
        
    sem_wait(mutex);
    if(*info->state == CONF_STATE_MODIFIED) {

      if (ioctl(fd_nv_dev, SONIX_NVRAM_IOCTL_COMMIT, &nvr) < 0) {
        sem_post(mutex);
        perror("ioctl");
        close(fd_nv_dev);
        return -1;
      }
            
      *info->state = CONF_STATE_VALID;
    }
    sem_post(mutex);

  }
  close(fd_nv_dev);

	return 0;
}

/*
 * Reset data of NVRAM cache from flash.
 * Unit: config.
 */

int nvram_reset(INFO* info)
{
	int             fd_nv_dev;
  void *          config_tmp;
  nvram_ioctl_t   nvr;
  char            type;
  unsigned int    size;
  config_info_t*  tmp_nv_p;
  int             cnt;

  if((info->offset != 0) || (info->misc < (sizeof(nvram_t))))
  {
    LIBNV_ERROR("reset only support reset all\n");
    info->offset = 0;
    info->misc = sizeof(nvram_t);
  }  
  if ((nv_info == NULL) || (mutex == NULL)) {
    LIBNV_ERROR("Please call init first\n");
    return -1;
  }
    
  //Check NVRAM version between APPs and Libs
  if ((nv_info->nvr_id == 0) || (nv_info->nvr_id != NVRAM_ID)) {
    LIBNV_ERROR("NVRAM ID does not match!\n");
    return -1;
  }

  if (info->misc == 0) {
    LIBNV_ERROR("WRONG info\n");
    return -1;
  }
    
  fd_nv_dev = open(NV_DEV, O_RDONLY );
  if (fd_nv_dev < 0) {
    perror(NV_DEV);
    return -1;
  }
    
  type = GET_INFO_TYPE(info->misc);
  size = GET_INFO_SIZE(info->misc);
    
  //Reset all NVRAM cache
  if(info->offset == 0 && size == sizeof(nvram_t)) {

#ifdef ALL_AT_ONCE
        //RESET all config at once
    nvr.param1 = 0;             //offset
    nvr.param2 = size;          //size
                
    if (ioctl(fd_nv_dev, SONIX_NVRAM_IOCTL_RESET, &nvr) < 0) {
      sem_post(mutex);
      perror("ioctl");
      close(fd_nv_dev);
      return -1;
    }
     
#endif
    tmp_nv_p = (config_info_t*) nv_info;
        
    //check every config state
    for(cnt=0;cnt < IPCAM_CON_NUM;cnt++, tmp_nv_p++) {
            
    //Only commit the modified config
      sem_wait(mutex);
      if((tmp_nv_p->state == CONF_STATE_MODIFIED) || (tmp_nv_p->state == CONF_STATE_FAILED)) {

#ifndef ALL_AT_ONCE
    //RESET all by parsing each config state.
        nvr.param1 = tmp_nv_p->offset;      //offset
        nvr.param2 = tmp_nv_p->size;        //size
               
        if (ioctl(fd_nv_dev, SONIX_NVRAM_IOCTL_RESET, &nvr) < 0) {
          sem_post(mutex);
          perror("ioctl");
          close(fd_nv_dev);
          return -1;
        }
#endif
        tmp_nv_p->state = CONF_STATE_VALID;
        sem_post(mutex);
      } else {
        sem_post(mutex);
        continue;
      }
    } 

  } else {
  // reset the specified config
        
    if (type != CONFIG_TYPE) {
      LIBNV_ERROR("Allow CONFIG reset only\n");
      return -1;
    }
        
    nvr.param1 = info->offset;              //offset
    nvr.param2 = size;                      //size
        
    sem_wait(mutex);
    if((*info->state == CONF_STATE_MODIFIED) || (*info->state == CONF_STATE_FAILED)) {

      if (ioctl(fd_nv_dev, SONIX_NVRAM_IOCTL_RESET, &nvr) < 0) {
        sem_post(mutex);
        perror("ioctl");
        close(fd_nv_dev);
        return -1;
      }
            
      *info->state = CONF_STATE_VALID;
    }
    sem_post(mutex);

  }
    
  close(fd_nv_dev);
	return 0;
}

//add by xyd
static char *read_file(const char *fname,unsigned int mode,size_t *filesize);
static int nv_write_cfgfile_to_nvram(INFO* info_e,const char *fname,unsigned int mode);

int nvram_commit_all()
{
	INFO info_commit; 
	int result;//only support commit all for spec
 	info_commit.offset = 0; 
 	info_commit.misc = sizeof(nvram_t); 
  	result = nvram_commit (&info_commit);   // if return -1 no commit
  	return result;
}
int nvram_reset_all()
{
	INFO info_reset;                //only support reset all for spec
  	info_reset.offset = 0; 
 	info_reset.misc = sizeof(nvram_t); 
 	nvram_reset (&info_reset);    // if return -1 no reset 
}
int nvram_set_str(INFO *info_e, const char *data)
{
	int ret =0;
	ret = nvram_set_str_cache(info_e, data);
	if(ret>=0)
		nvram_commit_all();
	return ret;
}
int nvram_set_str_cache(INFO *info_e,const char *data)
{
	char *buf=NULL;
	size_t size=0;
	size_t len=0;
	int ret = 0;

	len  = strlen(data);
	size = GET_INFO_SIZE(info_e->misc);

	if(len>size)
	{
		fprintf(stderr,"This info have no enough size to store the string.\n");
		return -1;
	}

	if((buf=malloc(size +1))==NULL)
		return -1;
	memset(buf,0,size+1);
	memcpy(buf,data,size < len ? size : len);
	
 	ret = nvram_set(info_e,buf);
	free(buf);
	return ret;
}

char *nvram_get_str(INFO *info_e)
{
	char *ret=NULL;
	char *data =NULL;
	size_t size=0;
	size_t len=0;

	size = GET_INFO_SIZE(info_e->misc);
	if((data=malloc(size+1))==NULL)
		return NULL;
	memset(data,0,size+1);
	nvram_get(info_e,data);
	ret = strdup(data);
	free(data);
	return ret;
}

int nvram_from_cfgfile_cache(INFO *info_e,const char *fname)
{
#ifdef NVRAM_SIMPLE_FILE
		return nv_write_cfgfile_to_nvram(info_e, fname, 0);
#else
		return nv_write_cfgfile_to_nvram(info_e, fname, ALL_FILE);
#endif
}

int nvram_from_cfgfile_all_cache(INFO *info_e,const char *fname)
{
	return nv_write_cfgfile_to_nvram(info_e, fname, ALL_FILE);
}

int nvram_from_cfgfile_simple_cache(INFO *info_e,const char *fname)
{
	return nv_write_cfgfile_to_nvram(info_e, fname, 0);
}

int nvram_from_cfgfile(INFO *info_e,const char *fname)
{
#ifdef NVRAM_SIMPLE_FILE
	return nv_write_cfgfile_to_nvram(info_e, fname, NV_COMMIT);
#else
	return nv_write_cfgfile_to_nvram(info_e, fname, ALL_FILE|NV_COMMIT);
#endif
}

int nvram_from_cfgfile_all(INFO *info_e,const char *fname)
{
	return nv_write_cfgfile_to_nvram(info_e, fname, ALL_FILE|NV_COMMIT);
}

int nvram_from_cfgfile_simple(INFO *info_e,const char *fname)
{
	return nv_write_cfgfile_to_nvram(info_e, fname, NV_COMMIT);
}


int nvram_to_cfgfile(INFO *info_e,const char *fname)
{
	FILE *fp=NULL;
	char *file_data=NULL;
	size_t info_size =0;

	info_size = GET_INFO_SIZE(info_e->misc);
	if((file_data=malloc(info_size+1))==NULL)
	{
		return -1;
	}
	memset(file_data,0,info_size+1);
	nvram_get(info_e,file_data);
	if((fp=fopen(fname,"w"))==NULL)
		return -1;
	fprintf(fp,"%s",file_data);
	fclose(fp);
	free(file_data);
	return 0;
	
}

static int nv_write_cfgfile_to_nvram(INFO* info_e,const char *fname,unsigned int mode)
{
	size_t info_size=0;
	size_t file_size=0;
	char *file_data=NULL;
	int ret=0;

	info_size = GET_INFO_SIZE(info_e->misc);
	file_data = read_file(fname,mode,&file_size);
	if(!file_data)
	{
		fprintf(stderr,"Read %s error\n",fname);
		return -1;
	}
	if(info_size<file_size)
	{
		fprintf(stderr,"This info have no enough size to store the file\n");
		free(file_data);
		return -1;
	}
	if(mode&NV_COMMIT)
		ret=nvram_set_str(info_e,file_data);
	else
		ret=nvram_set_str_cache(info_e,file_data);
	free(file_data);
	return ret;
}

static char *read_file(const char *fname,unsigned int mode,size_t *filesize)
{
	FILE *fp=NULL;
	size_t file_size=0;
	char *file_data=NULL;
	char buf[BUFF_LINE];

	const char *xml_suffix=".xml";
	const char *xml_comment_begin_mark="<!--";
	const char *xml_comment_end_mark="-->";
	char *xml_comment_begin=NULL;
	char *xml_comment_end=NULL;

	*filesize=0;
	if((fp=fopen(fname,"r"))==NULL)
		return NULL;
	fseek(fp,0,SEEK_END);
	file_size = ftell(fp);
	fseek(fp,0,SEEK_SET);/*get file size*/
	if((file_data=malloc(file_size+1))==NULL)
	{
		fclose(fp);
		return NULL;
	}
	
	/*xml file*/
	if(!strcasecmp(fname+strlen(fname)-strlen(xml_suffix),xml_suffix))
	{
		file_data[file_size]=0;
		fread(file_data,1,file_size,fp);
		fclose(fp);
		if(!mode&ALL_FILE)/*remove comment*/
		while(xml_comment_begin=strstr(file_data,xml_comment_begin_mark))
		{
			if(!(xml_comment_end=strstr(file_data,xml_comment_end_mark)))
			{
				fprintf(stderr,"%s is not a standerd xml file.\n");
				free(file_data);
				return NULL;
			}
			xml_comment_end+=strlen(xml_comment_end_mark);
			while(*xml_comment_end)
				*xml_comment_begin++=*xml_comment_end++;
			*xml_comment_begin=0;
		}
		file_size=strlen(file_data);
	}
	else/*open source config*/
	{
		file_size=0;
		while(fgets(buf,BUFF_LINE,fp))
		{
			if(mode&ALL_FILE)
				file_size+=sprintf(file_data+file_size,buf);
			else
			{
				if(buf[0]=='#'||buf[0]=='\n')
					continue;
				else
					file_size+=sprintf(file_data+file_size,"%s\n",buf);
			}
		}
	fclose(fp);
	}
	*filesize=file_size;
	return file_data;
}


int fname_convert_infoname(const char *fname,char *info_name)
{
	char *p=NULL;
	char *end=NULL;
	memset(info_name,0,256);
	strcpy(info_name,fname);
	
	while(p=strchr(info_name,POINT))
	{
		end=info_name+strlen(info_name);
		while(p!=end-1)
		{
			end--;
			*(end+strlen(POINT_MARK)-1)=*end;
		}
		memcpy(p,POINT_MARK,strlen(POINT_MARK));
	}
	
	while(p=strchr(info_name,SKEWLINE))
	{
		end=info_name+strlen(info_name);
		while(p!=end-1)
		{
			end--;
			*(end+strlen(SKEWLINE_MARK)-1)=*end;
		}
		memcpy(p,SKEWLINE_MARK,strlen(SKEWLINE_MARK));
	}
	
	while(p=strchr(info_name,LINE_CHAR))
	{
		end=info_name+strlen(info_name);
		while(p!=end-1)
		{
			end--;
			*(end+strlen(LINE_MARK)-1)=*end;
		}
		memcpy(p,LINE_MARK,strlen(LINE_MARK));
	}
	return 0;
}
//xyd

