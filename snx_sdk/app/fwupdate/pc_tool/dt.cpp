/*
* attention 1):this side is used for pc,it's the client side
* attention 2):the board control it's operation
* 
* usage:./dt ip
*/
#include "dt.h"

#define SECURITY_PWD            "sonixrf"
#define FW_UPDATE_DAEMON_PORT   6847
#define PACKET_PREFIX           "SNX"

#define BUFFER_SIZE             1024
 
int main(int argc, char *argv[])
{
    
//    struct hostent *he;
    char ipaddr[20];
    // connector's address information
    struct sockaddr_in their_addr;
    char buffer[BUFFER_SIZE];
    size_t data_len;
    char  filename[100];
    int filesize = 0;
    
#ifdef WINDOWS 
	int wsaret;
	WSADATA wsaData;
	SOCKET sockfd;
#else
	int sockfd;
#endif
     
    // if no command line argument supplied
    if(argc != 3)
    {
        fprintf(stderr, "Client-Usage: %s hostname filename \n", argv[0]);
        // just exit
        exit(1);
    }

#if 0     
    // get the host info
    if((he=gethostbyname(argv[1])) == NULL)
    {
        perror("gethostbyname()");
        exit(1);
    }
    else
        printf("Client-The remote host is: %s\n", argv[1]);
#else
    strcpy(ipaddr, argv[1]);
#endif
    strcpy(filename, argv[2]);
    
    {
        int filefd = 0;
#ifdef WINDOWS 
		struct _stat st_buf;
#else
        struct stat st_buf;
#endif
        if((filefd=open(filename,O_RDONLY))==-1)
        {
            printf("Crate %s failed\n", filename);
            exit(1);
        }
        fstat (filefd, &st_buf);
        filesize = st_buf.st_size;
        printf("Filename : %s, Size: %d\n",filename, filesize);
        if(filefd)
            close(filefd);

    }
    //filesize = atoi(argv[3]);

    
    printf("filename : %s, filesize: %d\n", filename, filesize);

#ifdef WINDOWS 
	//initialize winsock
    wsaret=WSAStartup(MAKEWORD(2,2),&wsaData); 
    if(wsaret)  
    		return -1; 
    
	sockfd=socket(AF_INET,SOCK_STREAM,0);  
	if(sockfd==INVALID_SOCKET)  
	{
    	perror("socket()");
        exit(1);
    } else
        printf("Client-The socket() sockfd is OK...\n");
#else
    //create socket 
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
		perror("socket()");
        exit(1);
    }
    else
        printf("Client-The socket() sockfd is OK...\n");
#endif

    bzero((char *)&their_addr, sizeof(their_addr));
    their_addr.sin_family = AF_INET;
    their_addr.sin_addr.s_addr = inet_addr(ipaddr);
    their_addr.sin_port = htons(FW_UPDATE_DAEMON_PORT);
 

    //connect
    if(connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("connect()");
        exit(1);
    }


    {
        struct strt_pckt strt_pkt_cmd;
        bzero(&strt_pkt_cmd,sizeof(struct strt_pckt));

        strcpy(strt_pkt_cmd.prefix, PACKET_PREFIX);
        strt_pkt_cmd.cmd = FW_UPDT_CMD_START;
        strt_pkt_cmd.fw_type = FW_UPDT_TYPE_MAIN_CODE;
        strt_pkt_cmd.total_size = ntohl(filesize);

        data_len=send(sockfd,(char *)&strt_pkt_cmd,sizeof(struct strt_pckt),0);
        if(data_len<0)
        {  
            perror("send file error\n");
        }

        printf ("Send strt pkt cmd successfully\n");

    }

    {

        int recv_len =0;
        struct strt_pckt_ack *strt_pckt_ack_p = (struct strt_pckt_ack *) malloc( sizeof(struct strt_pckt_ack));
            recv_len = recv(sockfd, (char* )strt_pckt_ack_p, sizeof(struct strt_pckt_ack), 0);
            if (recv_len <= 0) {   
                printf("recv <= 0\n");
                goto Exception;
            }

        if(strt_pckt_ack_p)
            free(strt_pckt_ack_p);
    }
   
    {
        int len = 0;
        FILE *filefd;

#if 1
        if((filefd=fopen(filename,"rb"))==NULL)
        {
            printf("Crate %s failed\n", filename);
            goto Exception;
        }

        while((len=fread(buffer, 1, BUFFER_SIZE , filefd))>0)
        {
        	printf("len: %d\n", len);
            data_len=send(sockfd,buffer,len,0);
            if(data_len<0)
            {  
                    perror ("send file error\n");
                  break;
            }
            bzero(buffer,BUFFER_SIZE);
        }
#endif
        printf("send file sucessfully!\n");
        //close file stream
        if(fclose(filefd))
        {
                perror("file close error\n");
        }     

    }

Exception:
    printf("Client-Closing sockfd\n");
#ifdef WINDOWS 
	if(sockfd)
		closesocket(sockfd);
	WSACleanup();
#else
    if (sockfd)
        close(sockfd);
#endif
    return 0;
}

