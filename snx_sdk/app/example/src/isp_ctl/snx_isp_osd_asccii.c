#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>             /* getopt_long() */
#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>


#include "snx_isp/isp_lib_api.h"

static char *name = "asccii16x16.bin";

static unsigned char *asccii = " 0123456789:-Sonix";

static unsigned char *txt = "Sonix";

int main(void)
{
	int res;
	int fd, sz;
	char *font;
	struct stat st;

	if(stat(name, &st) < 0){
		fprintf(stderr, "can't get %s stat!\n", name);
		return 0;
	}
	sz = st.st_size;

	if(!(font = malloc(sz))){
		fprintf(stderr, "allocate buffer error!\n");
		return 0;
	}

	if((fd = open(name, O_RDONLY)) < 0){
		fprintf(stderr, "open %s error!\n", name);
		goto file_read_error;
	}

	if((res = read(fd, font, sz)) < 0){
		fprintf(stderr, "read %s error!\n", name);
		goto file_read_error;
	}

	snx_isp_osd_font_set(ISP_CH_0, font);
	snx_isp_osd_font_set(ISP_CH_1, font);

	snx_isp_osd_template_set(ISP_CH_0, asccii);
	snx_isp_osd_template_set(ISP_CH_1, asccii);

	snx_isp_osd_line_txt_set(ISP_CH_0, ISP_OSD_LINE_2, txt);
	snx_isp_osd_line_txt_set(ISP_CH_1, ISP_OSD_LINE_2, txt);

	snx_isp_osd_timestamp_set(ISP_CH_0, 0); //disable auto time(kernel)
	snx_isp_osd_timestamp_set(ISP_CH_1, 0); //disable auto time(kernel)

	while(1){
		char *str = font;
		time_t timep;
		struct tm *p;
		time(&timep);
		p = localtime(&timep);

		sprintf(str, "%04d-%02d-%02d %02d:%02d:%02d", (1900 + p->tm_year), (p->tm_mon + 1), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);

		snx_isp_osd_line_txt_set(ISP_CH_0, ISP_OSD_LINE_1, str);
		snx_isp_osd_line_txt_set(ISP_CH_1, ISP_OSD_LINE_1, str);
		sleep(1);
	}

	close(fd);
file_read_error:

	free(font);	
	return 0;
}

