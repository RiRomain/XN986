/*---------------------- created by Perl tool -------------------------------*/
/*
 * Perl tool version: XXXXX
 * Auther: XXXXXX
 */

 
/*  pre-define */
#define NVRAM_ID    0x4dae7f6c
#define uint8_t unsigned char
#define uint16_t unsigned short
#define uint32_t unsigned long
#define int32_t signed long
#pragma pack (1)



#define IPCAM_CON_NUM    2


/*  Config structure. */
typedef struct UserRelated_s {
		char UID[280];
		char SSID[300];
		char Password[300];
} UserRelated_t;



/* Configuration Structures useful for managing node. */
typedef struct nvram_s	{
		//save the NVRAM_ID at the first 4 bytes.
		unsigned int	nvr_id;
		//version
		char nvram_version[64];
		//body
		UserRelated_t		UserRelated_node;

    char inter_ff[12];
    
    char skip_zero[14];
    //crc
		unsigned short int nvram_crc;

} nvram_t;


/* The IOCTL commands for nvram lib */
#define SONIX_NVRAM_IOCTL_INIT          (0x00)
#define SONIX_NVRAM_IOCTL_COMMIT        (0x01)
#define SONIX_NVRAM_IOCTL_RESET         (0x03)



/* IOCTL parameter (nvram lib)*/
typedef struct nvram_ioctl_s {
        int         param1;
        int         param2;
} nvram_ioctl_t;


//NVRAM_CONFIG_STATE for nvram lib
#define CONF_STATE_VALID      (0)
#define CONF_STATE_MODIFIED   (1)
#define CONF_STATE_FAILED     (2)


typedef struct config_info_s {
        char            state;               //the status of sub_struct
        unsigned long   offset;
        unsigned int    size;
} config_info_t;


/* used in nvram library */
typedef struct nvram_info_s {
		config_info_t		UserRelated_info;
		
        unsigned long           nvr_va;
        int                     nvr_id;
} nvram_info_t;


/* -----------------------	UserRelated Definition	-----------------------------*/
#define USERRELATED { \
                            info.state = &(nv_info->UserRelated_info.state); \
                            info.offset = (unsigned long)&(nvram_tmp.UserRelated_node) - (unsigned long)&(nvram_tmp); \
                            info.misc   =  sizeof(nvram_tmp.UserRelated_node); \
                            return &info; \
                            }
#define USERRELATED_UID { \
                            info.state = &(nv_info->UserRelated_info.state); \
                            info.offset = (unsigned long)&(nvram_tmp.UserRelated_node.UID) - (unsigned long)&(nvram_tmp); \
                            info.misc   = sizeof(nvram_tmp.UserRelated_node.UID); \
                            return &info; \
                            }
#define USERRELATED_SSID { \
                            info.state = &(nv_info->UserRelated_info.state); \
                            info.offset = (unsigned long)&(nvram_tmp.UserRelated_node.SSID) - (unsigned long)&(nvram_tmp); \
                            info.misc   = sizeof(nvram_tmp.UserRelated_node.SSID); \
                            return &info; \
                            }
#define USERRELATED_PASSWORD { \
                            info.state = &(nv_info->UserRelated_info.state); \
                            info.offset = (unsigned long)&(nvram_tmp.UserRelated_node.Password) - (unsigned long)&(nvram_tmp); \
                            info.misc   = sizeof(nvram_tmp.UserRelated_node.Password); \
                            return &info; \
                            }
                           
/* ----------------------- NVRAM definition ----------------------------------*/

#define NVRAM       {\
                            info.state = NULL; \
                            info.offset = 0; \
                            info.misc   = sizeof(nvram_t); \
                            return &info; \
                            }
					
							
typedef struct info_s {
        char            *state;
        unsigned long   offset;
#define CONFIG_TYPE         (1)
#define ELEMENT_TYPE        (0)
        unsigned int    misc;           //[31]Type, [30:0] Size
} info_t; 

//#ifndef LIB_NVRAM
extern nvram_info_t *nv_info;

/*  ------------------------------------------------------------------------ */
nvram_t             nvram_tmp;
info_t              info;

#define INFO_CONFIG_TYPE        (CONFIG_TYPE << 31)                 //Put type into the MSB
#define GET_INFO_SIZE(x)        (x & (~(0x1<<31)))                  //Get info size
#define GET_INFO_TYPE(x)        (x >> 31)                           //Get info type



typedef  info_t  INFO;

#ifdef __KERNEL__

#define SONIX_NVRAM_DEVNAME "nvram"
#define SONIX_NVRAM_MTDNAME  "userconfig"
#define SONIX_NVRAM_OFFSET   0x4

#else
#define NV_DEV      "/dev/nvram"            //NVRAM DEV

/* NVRAM_LIB APIs in user space */

int     nvram_init (int nvram_id);
void    nvram_get(INFO* info,void* data);
int     nvram_set(INFO* info, void* data);
int     nvram_commit(INFO* info);
int     nvram_reset(INFO* info);
int     nvram_close(void);
int     nvram_set_str(INFO *info_e,const char *data);
char    *nvram_get_str(INFO *info_e);
int     nvram_set_str_cache(INFO *info_e,const char *data);
int     nvram_from_cfgfile(INFO *info_e,const char *filename);
int     nvram_from_cfgfile_all(INFO *info_e,const char *filename);
int     nvram_from_cfgfile_simple(INFO *info_e,const char *filename);
int     nvram_from_cfgfile_cache(INFO *info_e,const char *filename);
int 		nvram_from_cfgfile_all_cache(INFO *info_e,const char *filename);
int 		nvram_from_cfgfile_simple_cache(INFO *info_e,const char *filename);
int 		nvram_to_cfgfile(INFO *info_e,const char *filename);
int 		nvram_reset_all();
int 		nvram_commit_all();
int 		fname_convert_infoname(const char *fname,char *info_name);
INFO *get_info(const char * info_name);

#ifndef NULL
#define NULL		(void*) 0
#endif
#ifndef __LIBNVRAM__
extern char *all_config;
#else

inline INFO* USERRELATED_FUN()
USERRELATED

inline INFO* USERRELATED_UID_FUN()
USERRELATED_UID

inline INFO* USERRELATED_SSID_FUN()
USERRELATED_SSID

inline INFO* USERRELATED_PASSWORD_FUN()
USERRELATED_PASSWORD

/*======================================*/

inline INFO* NVRAM_FUN()
NVRAM

/*======================================*/
INFO *get_info(const char *infoname)
{
char info_name[256];
fname_convert_infoname(infoname,info_name);
	if(!strcasecmp(info_name,"USERRELATED"))
		return USERRELATED_FUN();
	if(!strcasecmp(info_name,"USERRELATED_UID"))
		return USERRELATED_UID_FUN();
	if(!strcasecmp(info_name,"USERRELATED_SSID"))
		return USERRELATED_SSID_FUN();
	if(!strcasecmp(info_name,"USERRELATED_PASSWORD"))
		return USERRELATED_PASSWORD_FUN();
  if(!strcasecmp(info_name,"NVRAM"))
		return  NVRAM_FUN();

  return NULL;
}
char *all_config=
		"UserRelated\n"
		"UserRelated_UID\n"
		"UserRelated_SSID\n"
		"UserRelated_Password\n";

#endif
#endif

