
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h> 		   
#include <fcntl.h>			   
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/fb.h>


static void *do_mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset)
{
	void *ret;
	ret = mmap(start, length, prot, flags, fd, offset);
	if ( ret == (char *)-1 && (flags & MAP_SHARED) ) {
		ret = mmap(start, length, prot,
		           (flags & ~MAP_SHARED) | MAP_PRIVATE, fd, offset);
	}
	return ret;
}

int main (int argc,char ** argv)
{
	int i, fd;
	void *mem;
	int w, h, bits;
	long offset, memlen;
	unsigned long Rmask, Gmask, Bmask;
	struct fb_fix_screeninfo finfo;
	struct fb_var_screeninfo vinfo;	
	const int pagesize = getpagesize();

	fd = open("/dev/fb0", O_RDWR, 0);
	if(fd < 0){
		fprintf(stderr, "Couldn't open device\n");
		return (-1);
	}

	/* Determine the current screen info */
	if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo) < 0) {
		fprintf(stderr, "Couldn't get console pixel format\n");
		return(-1);
	}	
	w = vinfo.xres;
	h = vinfo.yres;
	bits = vinfo.bits_per_pixel;
  memlen = w * h * bits/8;
  printf ("w= %d h= %d,bits=%d\n",w,h,bits);
	Rmask = 0;
	for(i = 0; i < vinfo.red.length; ++i){
		Rmask <<= 1;
		Rmask |= (0x00000001 << vinfo.red.offset);
	}
	Gmask = 0;
	for (i = 0; i<vinfo.green.length; ++i){
		Gmask <<= 1;
		Gmask |= (0x00000001<<vinfo.green.offset);
	}
	Bmask = 0;
	for (i = 0; i<vinfo.blue.length; ++i){
		Bmask <<= 1;
		Bmask |= (0x00000001<<vinfo.blue.offset);
	}

	/* Get the type of video hardware */
	if (ioctl(fd, FBIOGET_FSCREENINFO, &finfo) < 0 ) {
		fprintf(stderr, "Couldn't get console hardware info\n");
		return(-1);
	}

	/* Memory map the device*/
//	offset = (((long)finfo.smem_start) - (((long)finfo.smem_start)&~(pagesize-1)));
//	printf ("finfo.smem_len = %x %x %x\n",finfo.smem_len,offset,finfo.line_length);
//  memlen = finfo.smem_len + offset;
	mem = do_mmap(NULL, memlen, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if (mem == (char *)-1 ) {
		fprintf(stderr, "Unable to memory map the framebuffer hardware\n");
		return(-1);
	}

	/*update framebuffer data*/
	for(i = 0; i < 255; i++){
		int r, g, b;
		struct timeval tv;
		tv.tv_sec = 1;
		tv.tv_usec = 0;

		r = (i&Rmask)>>vinfo.red.offset;
		g = (i&Gmask)>>vinfo.green.offset;
		b = (i&Bmask)>>vinfo.blue.offset;

		fprintf(stderr, "r:%d g:%d b:%d\n", r, g, b);

		memset(mem, i, memlen);
		select(0, NULL, NULL, NULL, &tv);
	}

	munmap(mem, memlen);
	close(fd);
	return 0;
}

