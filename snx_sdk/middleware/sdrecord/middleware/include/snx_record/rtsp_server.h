#ifndef __RTSP_SERVER__H__
#define __RTSP_SERVER__H__

#include <libavformat/rtsp.h>
#include <libavutil/lfg.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <xmllib.h>
#undef  malloc
#undef  free
#undef  realloc
#undef  time
#undef  sprintf
#undef  strcat
#undef  strncpy
#undef  exit
#undef  printf
#undef  fprintf

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/vfs.h>
#include <errno.h>
#define  CONF_XML  "/etc/rtsp_server/conf.xml"
int init_rtsp_server();
int destroy_rtsp_server();
int send_rtp_data(const char *id, char *data, int len);

#endif

