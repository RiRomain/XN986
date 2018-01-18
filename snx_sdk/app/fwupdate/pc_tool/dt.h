#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

#include <signal.h>

#ifdef WINDOWS 
#include "winsock2.h"
#include "sys/stat.h"
#else 
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <netdb.h>
#endif


#include <errno.h>

#define SNX_PRFIX_LEN   3
#define CMD_LEN         1

#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)

enum fwupdt_cmd {
    FW_UPDT_CMD_START = 0x00,
    FW_UPDT_CMD_STOP  = 0x01,

    FW_UPDT_CMD_ACK   = 0x10,    
};

enum fwupdt_type {
    FW_UPDT_TYPE_MAIN_CODE,
    FW_UPDT_TYPE_KERNEL_CODE,
    FW_UPDT_TYPE_ROOTFS_CODE,

    FW_UPDT_TYPE_NUM,
};

struct strt_pckt
{
    char prefix[SNX_PRFIX_LEN];
    unsigned char cmd;
    int total_size;
    unsigned char fw_type;
}__attribute__ ((packed));

struct strt_pckt_ack
{
    char prefix[SNX_PRFIX_LEN];
    unsigned char cmd;
}__attribute__ ((packed));


