#ifndef __UTIL_H_
#define __UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif
//#include <ifaddrs.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "snx_lib.h"


int file_exist(const char *filename);
int snx_get_file_string(char *path, char *str);
int card_async_operation(char* cmd, char* target,  char* search);
#ifdef __cplusplus
}

#endif
#endif
