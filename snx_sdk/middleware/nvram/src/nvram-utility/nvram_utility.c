#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "nvram.h"

#define dputs(x) do{if (0) puts(x);}while(0)
#define D(x,...) if(1) printf("%s %d "x,__FUNCTION__,__LINE__,##__VA_ARGS__)
#define putsErr(x,...)	fprintf(stderr,"%s %d "x,__FILE__,__LINE__,##__VA_ARGS__)
#define putsOut(x,...)  fprintf(stdout,"%s %d "x,__FILE__,__LINE__,##__VA_ARGS__)


#define NVRAM_GET 		"nvram_get"
#define NVRAM_SET 		"nvram_set"
#define NVRAM_RESET	"reset"
#define NVRAM_COMMIT	"commit"
#define NVRAM_BACKUP	"backup"
#define NVRAM_RESTORE 	"restore"
#define NVRAM_LIST 		"list"
#define NVRAM_BACKUP_FILE "nvram_backup.bin"

#define NVRAM_ERR 		"error"
#define NVRAM_SUCCESS	"success"

#define OPTION_NUM		2
#define LINE_LENGTH		16
#define LINE_SPACE		8
#define NVRAM_SIZE		256

/*utility_get command entry.*/
int nvram_utility_get(int argc,char **argv);

/*utility_set command entry.*/
int nvram_utility_set(int argc,char **argv);

/*get INFO by conf_name*/
INFO* nvram_get_info(const char *conf_name);

/*get absolute path*/
char *get_absolute_path(const char *path);

/*print data to command line or file as ASCII or binary,according to options 'f' and 'b'*/
int print_data(void *data,size_t data_size,const char *option,const char *file_name);
unsigned long print_line(const void *data,size_t data_size);
int set_data(int argc,char **argv,const char *options);

/*write binary data from file to nvram*/
int nvram_from_binaryfile(INFO *info_e,const char *file_name);
void *get_file_data(const char *file_name,size_t *size);
int usage(const char *cmd);
int get_usage(const char *cmd);
int set_usage(const char *cmd);

int nvram_backup(const char *file_name);
int nvram_restore(const char *file_name);

int main(int argc,char **argv)
{
	char *p=NULL;
	int i=strlen(argv[0])-1;

	for(;i>=0;i--)
	{
		p=argv[0]+i;
		if(*p=='/'&&p++)
			break;
	}
	if(!strcmp(NVRAM_GET,p))
		return nvram_utility_get(argc,argv);
	else if(!strcmp(NVRAM_SET,p))
		return nvram_utility_set(argc,argv);
	
	if(argc==2&&!strcmp(argv[1],NVRAM_LIST))
	{
		printf("All infos were displayed here:\n");
		printf(all_config);
		return 0;
	}
	else if(argc==2&&!strcmp(argv[1],NVRAM_RESET))
	{
		nvram_init(NVRAM_ID);
		if(!nvram_reset_all())
			printf("Reset %s!\n",NVRAM_SUCCESS);
		else
			printf("Reset %s!\n",NVRAM_ERR);
		nvram_close();
		return 0;
	}
	else if(argc==2&&!strcmp(argv[1],NVRAM_COMMIT))
	{
		nvram_init(NVRAM_ID);
		if(!nvram_commit_all())
			printf("Commit %s!\n",NVRAM_SUCCESS);
		else
			printf("Commit %s!\n",NVRAM_ERR);
		nvram_close();
		return 0;
	}
	else if(argc==2&&!strcmp(argv[1],NVRAM_BACKUP))
	{
		if(! nvram_backup(NVRAM_BACKUP_FILE))
			printf("Backup nvram as %s %s!\n",NVRAM_BACKUP_FILE,NVRAM_SUCCESS);
		else
			printf("Backup nvram %s!\n",NVRAM_ERR);
		return 0;
	}
	else if(argc==3&&!strcmp(argv[1],NVRAM_RESTORE))
	{
		if(!nvram_restore(argv[2]))
			printf("Restore nvram from %s %s!\n",argv[2],NVRAM_SUCCESS);
		else
			printf("Restore nvram %s!\n",NVRAM_ERR);
	}
	else
		return usage(p);
}
int usage(const char *cmd)
{
	printf("usage:\n");
	printf("%s <command> [<file>]\n",cmd);
	printf("command:\n");
	printf("list    - display all infos name\n");
	printf("commit    - write cache to nvram\n");
	printf("reset    - copy nvram to cache\n");
	printf("backup   - backup nvram to a file ,named %s \n",NVRAM_BACKUP_FILE);
	printf("restore   - restore nvram from <file>\n");
	return 0;
}

int get_usage(const char *cmd)
{
	printf("%s  -  display <info>'s data of nvram,default data is ASCII string\n",cmd);
	printf("usage:\n");
	printf("%s [<option>] <info>  [<file>]\n",cmd);
	printf("option:\n");
	printf("-f\n       output result to <file>\n");
	printf("-b\n       output result as binary stream\n");
	return 0;
}

int set_usage(const char *cmd)
{
	printf("%s  -  set data of nvram follow <info>,default data is ASCII string\n",cmd);
	printf("usage:\n");
	printf("%s [<option>] <info>  [<file>]\n",cmd);
	printf("option:\n");
	printf("-f\n       read data from <file>\n");
	printf("-b\n      read data  as binary stream\n");
	return 0;
}

int nvram_backup(const char *file_name)
{
	const size_t nv_size=sizeof(nvram_t);
	FILE *fp=NULL;
	char *file_path=NULL;
	INFO* info_nvram=nvram_get_info("NVRAM");
	void *data=NULL;
	if((data=malloc(nv_size+1))==NULL)
	{
		putsErr("malloc error\n");
		return -1;
	}
	nvram_init(NVRAM_ID);
	nvram_get(info_nvram,data);
	nvram_close();
	
	file_path=get_absolute_path(file_name);
	if((fp=fopen(file_path,"w"))==NULL)
	{
		putsErr("Can't create file %s \n",file_name);
		free(data);
		return -1;
	}
	fwrite(&nv_size,1,sizeof(size_t),fp);
	fwrite(data,1,nv_size,fp);
	fclose(fp);
	free(file_path);
	free(data);
	return 0;
}
int nvram_restore(const char *file_name)
{
	size_t nv_size=0;
	unsigned int nv_id=0;
	FILE *fp=NULL;
	void *data=NULL;
	char *file_path=NULL;
	INFO *info_nvram=NULL;
	char state=CONF_STATE_VALID;

	info_nvram=nvram_get_info("NVRAM");
	info_nvram->offset+=sizeof(unsigned int);
	info_nvram->misc-=sizeof(unsigned int)+sizeof(unsigned long);
	info_nvram->state=&state;
	file_path=get_absolute_path(file_name);
	if((fp=fopen(file_path,"r"))==NULL)
	{
		putsErr("Can't open file %s \n",file_name);
		return -1;
	}
	free(file_path);
	fread(&nv_size,1,sizeof(size_t),fp);
	if(nv_size!=sizeof(nvram_t))
	{
		putsErr("Size mismatch!\n");
		goto nv_err;
	}
	fread(&nv_id,1,sizeof(unsigned int),fp);
	if(nv_id!=NVRAM_ID)
	{
		putsErr("Nvram ID mismatch!\n");
		goto nv_err;
	}
	if((data=malloc(nv_size+1))==NULL)
	{
		putsErr("malloc error\n");
		goto nv_err;
	}

	if(fread(data,1,GET_INFO_SIZE(info_nvram->misc),fp)
		!=GET_INFO_SIZE(info_nvram->misc))
	{
		putsErr("The file %s has been broken!\n",file_name);
		free(data);
		goto nv_err;
	}
	
	nvram_init(NVRAM_ID);
	if(nvram_set(info_nvram,data)!=0)
	{
		putsErr("nvram_set err \n");
		goto nv_err1;
	}
	if(nvram_commit_all()!=0)
	{
		putsErr("nvram_commit  err \n");
		goto nv_err1;
	}
	
	fclose(fp);
	return 0;
nv_err1:
	nvram_close();
	free(data);
nv_err:
	fclose(fp);
	return -1;
}


int nvram_utility_set(int argc,char **argv)
{
	char *options_set="fb";//f : file b : binary
	char options[OPTION_NUM]={0};
	int i=1,j=0;
	FILE *fp=NULL;
	char *file_path = NULL;
	INFO *info_e=NULL;
	size_t info_len=0;
	void *data=NULL;

	for(i;i<argc;i++)
	{
		if(argv[i][0]=='-')
		{
			if(strlen(argv[i])==1)
			{
				fprintf(stderr,"error option '%s' \n",argv[i]);
				set_usage(argv[0]);
				exit(1);
			}
			for(j=1;j<strlen(argv[i]);j++)
			{
				if(argv[i][j]=='f')
					options[0]='f';
				else if(argv[i][j]=='b')
					options[1]='b';
				else 
				{
					fprintf(stderr,"Invalid option '%c' \n",argv[i][j]);
					set_usage(argv[0]);
					exit(1);
				}
			}
		}
		else 
			break;
	}
	
	if(options[0]=='f'&&argc-1!=3||options[0]==0&&options[1]==0&&argc-1!=2)
	{
		fprintf(stderr,"error option parameter num\n");
		set_usage(argv[0]);
		exit(1);
	}
	if(options[0]==0&&options[1]!=0&&argc!=4)
	{
		fprintf(stdout,"This tool only suport set string data to nvram in terminal.\n"
				"But you can save the binay data into a file ,then use option '-bf' to store the file into nvram.\n");
		printf("Set %s !\n",NVRAM_ERR);
		return -1;
	}
	return set_data(argc,argv,options);
}

int set_data(int argc,char **argv,const char *options)
{
	void *data=NULL;
	int i=0;
	INFO *info_e=NULL;
	char *file_path=NULL;
	int ret=-1;
	
	nvram_init(NVRAM_ID);
	info_e = get_info(argv[argc-2]);
	if(!info_e)
	{
		fprintf(stdout,"Have no this info %s. \n"
			"You should use command 'nvram_utility list' to check the info list.\n",argv[argc-2]);
		nvram_close();
		exit(1);
	}
	if(!options[0])
	{
		if(!options[1])
		{
			ret = nvram_set_str(info_e,argv[argc-1]);
		}
		else
		{
			fprintf(stdout,"This tool only suport set string data to nvram in terminal.\n"
				"But you can save the binay data into a file ,then use option '-bf' to store the file into nvram.\n");
		}
	}
	else
	{
		file_path = get_absolute_path(argv[argc-1]);
		if(!options[1])
		{
			ret = nvram_from_cfgfile(info_e,file_path);
		}
		else
		{
			ret = nvram_from_binaryfile(info_e,file_path);
		}
		free(file_path);
	}
	nvram_close();
	if(ret<0)
		printf("Set %s %s!\n",argv[argc-2],NVRAM_ERR);
	else
		printf("Set %s %s!\n",argv[argc-2],NVRAM_SUCCESS);
	return ret;
}

int nvram_from_binaryfile(INFO *info_e,const char *file_name)
{
	size_t info_size=0;
	size_t file_size=0;
	char *file_data=NULL;

	info_size = GET_INFO_SIZE(info_e->misc);
	file_data = get_file_data(file_name,&file_size);
	if(!file_data)
	{
		fprintf(stderr,"Read %s error\n",file_name);
		return -1;
	}
	if(info_size<file_size)
	{
		fprintf(stderr,"This info have no enough size to store the file\n");
		free(file_data);
		return -1;
	}
	nvram_set(info_e,file_data);
	free(file_data);
	return nvram_commit_all();
}

void *get_file_data(const char *file_name,size_t *size)
{
	FILE *fp=NULL;
	size_t file_size=0;
	char *file_data=NULL;
	//char buf[BUFF_LINE];

	if((fp=fopen(file_name,"r"))==NULL)
		return NULL;
	fseek(fp,0,SEEK_END);
	file_size = ftell(fp);
	fseek(fp,0,SEEK_SET);/*get file size*/
	if((file_data=malloc(file_size+1))==NULL)
	{
		fclose(fp);
		return NULL;
	}
	*size = fread(file_data,1,file_size,fp);
	fclose(fp);
	return file_data;
}


int nvram_utility_get(int argc,char **argv)
{
	char *options_set="fb";//f : file b : binary
	char options[OPTION_NUM]={0};
	int i=1,j=0;
	FILE *fp=NULL;
	char *file_path = NULL;
	INFO *info_e=NULL;
	size_t info_len=0;
	void *data=NULL;
	
	char *p=NULL;
	int x=strlen(argv[0])-1;
	for(;x>=0;x--)
	{
		p=argv[0]+x;
		if(*p=='/'&&p++)
			break;
	}

	for(i;i<argc;i++)
	{
		if(argv[i][0]=='-')
		{
			if(strlen(argv[i])==1)
			{
				fprintf(stderr,"error option '%s' \n",argv[i]);
				get_usage(p);
				exit(1);
			}
			for(j=1;j<strlen(argv[i]);j++)
			{
				if(argv[i][j]=='f')
					options[0]='f';
				else if(argv[i][j]=='b')
					options[1]='b';
				else 
				{
					fprintf(stderr,"Invalid option '%c' \n",argv[i][j]);
					get_usage(p);
					exit(1);
				}
			}
		}
		else 
			break;
	}
	
	if(options[0]=='f'&&argc-1!=3||options[0]==0&&options[1]==0&&argc-1!=1
		||options[0]==0&&options[1]!=0&&argc!=3)
	{
		fprintf(stderr,"error option parameter num\n");
		get_usage(p);
		exit(1);
	}
	nvram_init(NVRAM_ID);
	info_e = nvram_get_info(argv[i]);
	if(!info_e)
	{
		fprintf(stdout,"Have no this info %s ,\n"
			"you should use command 'nvram_utility list' to check the info list.\n",argv[i]);
		nvram_close();
		exit(1);
	}
	
	info_len = GET_INFO_SIZE(info_e->misc);
	if((data = malloc(info_len+1))==NULL)
	{
		fprintf(stderr,"%s: %d malloc error \n",__FILE__,__LINE__);
		nvram_close();
		exit(1);
	}
	nvram_get(info_e,data);
	nvram_close();
	
	i++;
	if(options[0])
	{
		printf("%s has been writen in file %s \nSuccess!\n",argv[i-1],argv[i]);
		file_path = get_absolute_path(argv[i]);
		print_data(data,info_len,options,file_path);
		free(file_path);
	}
	else
	{
		printf("%s :  \n",argv[i-1]);
		print_data(data,info_len,options,NULL);
	}
	return 0;
}

INFO* nvram_get_info(const char *conf_name)
{
	return get_info(conf_name);
}

char *get_absolute_path(const char *path)
{
	char *a_path = NULL;
	char *pwd = NULL;
	size_t len=0;

	if(path[0]=='/')
		return strdup(path);
	pwd=getenv("PWD");
	if((a_path=malloc(strlen(path)+strlen(pwd)+2))==NULL)
	{
		fprintf(stderr,"%s: %d malloc error \n",__FILE__,__LINE__);
		exit(1);
	}
	sprintf(a_path,"%s/%s",pwd,path);
	return a_path;
}

int print_data(void *data,size_t data_size,const char *options,const char *file_name)
{
	FILE *fp = NULL;
	unsigned long len=0;
	size_t datasize=data_size;

	if(!options[0])
	{
		if(!options[1])
			puts(data);
		else
		{
			while(datasize>0)
			{
				len+=print_line(data+len,datasize<LINE_LENGTH ? datasize : LINE_LENGTH);
				datasize -=datasize<LINE_LENGTH ? datasize : LINE_LENGTH;
			}
		}
	}
	else
	{
		if((fp=fopen(file_name,"w"))==NULL)
		{
			fprintf(stderr,"Can't open file '%s'",file_name);
			free(data);
			exit(1);
		}
		if(!options[1])
			fputs(data,fp);
		else
			fwrite(data,1,data_size,fp);
		fclose(fp);
	}
}

unsigned long print_line(const void *data,size_t data_size)
{
	int i=0;
	const char *p=(const char * )data;
	for(i=0;i<data_size;i++)
	{
		if(i==LINE_LENGTH>>1)
			fprintf(stdout," ");
		fprintf(stdout,"%2X",p[i]);
	}
	for(i=0;i<LINE_LENGTH-data_size+LINE_SPACE;i++)
		fprintf(stdout," ");
	for(i=0;i<data_size;i++)
	{
		if(i==LINE_LENGTH>>1)
			fprintf(stdout," ");
		if(isprint(p[i]))
			fprintf(stdout,"%c",p[i]);
		else
			fprintf(stdout,".");
	}
	fprintf(stdout,"\n");
	return data_size;
}


