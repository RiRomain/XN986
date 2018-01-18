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

static char *name = "unicode16x16.bin";

static unsigned short unicode[] = 
{
	0x0020, 
	0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,0x0038,0x0039,
	0x003a, 0x002d,
	0x677e, 0x7ff0, 0x79d1, 0x6280,
	0x0053, 0x006f, 0x006e, 0x0069, 0x0078,
	0x00,
};

static unsigned short wtxt[] =
{
	0x677e, 0x7ff0, 0x79d1, 0x6280,
	0x0020,
	0x0053, 0x006f, 0x006e, 0x0069, 0x0078,
	0x00,
};

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

	snx_isp_osd_templatew_set(ISP_CH_0, unicode);
	snx_isp_osd_templatew_set(ISP_CH_1, unicode);

	snx_isp_osd_line_txtw_set(ISP_CH_0, ISP_OSD_LINE_2, wtxt);
	snx_isp_osd_line_txtw_set(ISP_CH_1, ISP_OSD_LINE_2, wtxt);

	snx_isp_osd_timestamp_set(ISP_CH_0, 0); //disable auto time(kernel)
	snx_isp_osd_timestamp_set(ISP_CH_1, 0); //disable auto time(kernel)

	while(1){
		char *str = font;
		unsigned short wstr[64];
		time_t timep;
		struct tm *p;
		time(&timep);
		p = localtime(&timep);

		sprintf(str, "%04d-%02d-%02d %02d:%02d:%02d", (1900 + p->tm_year), (p->tm_mon + 1), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);

		memset(wstr, 0x0, sizeof(wstr));

		while(*str){
			wstr[str - font] = *str;
			str++;
		}

		snx_isp_osd_line_txtw_set(ISP_CH_0, ISP_OSD_LINE_1, wstr);
		snx_isp_osd_line_txtw_set(ISP_CH_1, ISP_OSD_LINE_1, wstr);
		sleep(1);
	}

	close(fd);
file_read_error:

	free(font);	
	return 0;
}

