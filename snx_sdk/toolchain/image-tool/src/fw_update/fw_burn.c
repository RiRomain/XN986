#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <linux/ioctl.h>
#include <mtd/mtd-abi.h>
#include <mtd/mtd-user.h>
#include "generated/snx_sdk_conf.h"

#define PROC_MTD			"/proc/mtd"

#ifdef 	CONFIG_ENABLE_BURNBIN_LOG
#define debug(fmt, args...) printf(fmt, ##args)
#else 
#define debug(fmt, args...)
#endif

typedef struct _image_entry
{
	unsigned int res;		//reservations
	unsigned int offset;
	unsigned int size;
	unsigned int start_addr;
	unsigned int end_addr;
} image_entry_st;


//typedef struct _mtd_info mtd_info_st;
typedef struct _mtd_info
{
	unsigned int start_addr;
	unsigned int end_addr;
	char *name;
	struct _mtd_info *next;
} mtd_info_st;

extern const unsigned int image_table;

int snx_destroy_mtd_info(mtd_info_st *pmtd_info)
{
	mtd_info_st *p, *pnext;

	p = pmtd_info;
	while(p != NULL)
	{
		pnext = p->next;
		free(p->name);
		free(p);
		p = pnext;
	}

	return 0;
}

int snx_create_mtd_info(mtd_info_st **pmtd_info)
{
	mtd_info_st *p, *plast;
	char part_name[16];
	FILE *fp = NULL;
	unsigned int size, index, start_addr = 0;
	int retval = 0;

	if((fp = fopen(PROC_MTD, "r")) == NULL)
	{
		printf("Open file '%s' failed.\n", PROC_MTD);
		return -1;
	}

	while(fgetc(fp) != '\n')
		;

	*pmtd_info = p = plast = NULL;
	index = 0;
	while(fgetc(fp) != EOF)
	{
		while(fgetc(fp) != ' ')
			;
		fscanf(fp, "%x", &size);
		while(fgetc(fp) != '\n')
			;

		p = malloc(sizeof(mtd_info_st));
		if(p == NULL)
		{
			printf("Allocate memory failed!\n");
			retval = -2;
			goto exit;
		}
		p->start_addr = start_addr;
		p->end_addr = start_addr + size;
		sprintf(part_name, "/dev/mtd%d", index);
		p->name = strdup(part_name);
		p->next = NULL;
		if(plast == NULL)
			*pmtd_info = plast = p;
		else
			plast->next = p;

		start_addr += size;
		plast = p;
		index++;
	}

exit:
	if(retval)
	{
		snx_destroy_mtd_info(*pmtd_info);
		*pmtd_info = NULL;
	}

	if(fp)
	{
		fclose(fp);
		fp = NULL;
	}
	return retval;
}

int snx_flash_write(char *dev_name, unsigned int start_offset, unsigned int end_offset,
	        const unsigned char *pdata, unsigned int data_offset, unsigned int data_size)
{
	int fd;
	int i, count;
	struct mtd_info_user mtd;
	struct erase_info_user erase;
	int retval = 0, image_size = 0, part_size = 0;

	fd = open(dev_name, O_SYNC | O_RDWR, 0666);
	if(fd < 0)
	{
		printf("Open '%s' error.\n", dev_name);
		return -1;
	}

	if(ioctl(fd, MEMGETINFO, &mtd) < 0)
	{
		printf("'%s' isn't a MTD Flash device\n", dev_name);
		retval = -2;
		goto exit;
	}

	if((start_offset % mtd.erasesize) || (end_offset % mtd.erasesize))
	{
		printf("erase section (0x%08x, 0x%08x) don't align\n", start_offset, end_offset);
		retval = -3;
		goto exit;
	}

//	count = (end_offset - start_offset) / mtd.erasesize;
//	erase.length = mtd.erasesize;
	part_size = end_offset - start_offset;
	image_size = (data_size + mtd.erasesize - 1) & ~(mtd.erasesize - 1);
	erase.length = (part_size > image_size ? image_size : part_size); 
	erase.start = start_offset;
	
	count = erase.length / mtd.erasesize;
	erase.length = mtd.erasesize;
//	printf ("# begin erase start: 0x%x, erase length: 0x%x\n", erase.start, erase.length);
	for(i = 0; i < count; i++)
	{
		if(ioctl(fd, MEMERASE, &erase) < 0)
		{
			printf("erase error at 0x%08x on %s\n", erase.start, dev_name);
			retval = -4;
			goto exit;
		}
		erase.start += mtd.erasesize;
	}
	printf ("@ stop erase start: 0x%x, datasize=0x%x\n", erase.start, data_size);

	if(data_size > 0)
	{
		unsigned int block_size, rw_size, ret_rw_size;
		char *buf = NULL;

		block_size = mtd.erasesize;
		buf = malloc(block_size);
		if(buf == NULL)
		{
			printf("Allocate memory failed!\n");
			retval = -5;
			goto exit;
		}

		pdata += data_offset;
		while(data_size > 0)
		{
			lseek(fd, start_offset, SEEK_SET);
			rw_size = data_size < block_size ? data_size : block_size;
			ret_rw_size = write(fd, pdata, rw_size);
			if(ret_rw_size != rw_size)
//			ret_rw_size = write(fd, pdata, data_size);
//			if(ret_rw_size != data_size)
			{
				printf("write error at 0x%08x on %s, write returned %d, rw_size= %d\n",
						start_offset, dev_name, ret_rw_size, rw_size);
				retval = -6;
				break;
			}
			start_offset += rw_size;
			pdata += rw_size;
			data_size -= rw_size;
#if 0
			lseek(fd, start_offset, SEEK_SET);
			read(fd, buf, rw_size);
			if(memcmp(buf, pdata, rw_size))
			{
				printf("verification mismatch at 0x%08x on %s\n", start_offset, dev_name);
				retval = -7;
				break;
			}

		}
#endif
		}
		free(buf);
	}
//	printf ("write end!\n");
	

exit:
	close(fd);
	return retval;
}

int snx_burn_image(image_entry_st *pimage_entry, const unsigned char *pimage, mtd_info_st *pmtd_info)
{
	unsigned int start_addr, end_addr, offset, size;
	int ret = 0;

	debug("reservations:0x%08x\n", pimage_entry->res);
	debug("offset:0x%08x\n", pimage_entry->offset);
	debug("size:0x%08x\n", pimage_entry->size);
	debug("flash start address:0x%08x\n", pimage_entry->start_addr);
	debug("flash end address:0x%08x\n", pimage_entry->end_addr);
	debug("\n\n");

	start_addr = pimage_entry->start_addr;
	end_addr = pimage_entry->end_addr + 1;
	offset = pimage_entry->offset;
	size = pimage_entry->size;
	while(pmtd_info != NULL)
	{
		if(
			(start_addr >= pmtd_info->start_addr) &&
			(start_addr < pmtd_info->end_addr) &&
			(end_addr <= pmtd_info->end_addr)
		)
		{
			ret = snx_flash_write(pmtd_info->name, start_addr - pmtd_info->start_addr,
					end_addr - pmtd_info->start_addr, pimage, offset, size);
			break;
		}

		if(
			(start_addr >= pmtd_info->start_addr) &&
			(start_addr < pmtd_info->end_addr) &&
			(end_addr > pmtd_info->end_addr)
		)
		{
			unsigned int erase_size;

			erase_size = pmtd_info->end_addr - start_addr;
			ret = snx_flash_write(pmtd_info->name, start_addr - pmtd_info->start_addr,
					pmtd_info->end_addr - pmtd_info->start_addr, pimage, offset,
					size <= erase_size ? size : erase_size);
			if(size <= erase_size)
			{
				size = 0;
			}
			else
			{
				size -= erase_size;
				offset += erase_size;
			}
			start_addr = pmtd_info->end_addr;
		}

		pmtd_info = pmtd_info->next;
	}

	if(pmtd_info == NULL)
	{
		printf("Can't find the partition of image.\n");
		return -1;
	}

	return ret;
}

#define IMAGE_TABLE_SIZE	0x200
int main(void)
{
	const unsigned int *p;
	const unsigned char *pimage;
	unsigned int image_table_size, index, image_size;
	image_entry_st *pimage_entry;
	mtd_info_st *pmtd_info = NULL;
	int retval = 0;

	retval = snx_create_mtd_info(&pmtd_info);
	if(retval)
	{
		printf("Create MTD information failed!\n");
		return retval;
	}

	p = &image_table;
//	image_size = *p;
//	printf("image size:0x%08x\n", image_size);
//	p++;

	/* crc check */

	pimage = (const unsigned char *)((unsigned int)p + IMAGE_TABLE_SIZE);
	image_table_size = *p;
	debug("image table size:0x%08x\n", image_table_size);
	p++;

	pimage_entry = (image_entry_st *)p;
	for(index = 0; index < image_table_size; index += 20)
	{
		debug("index:%d\n", index);

		retval = snx_burn_image(pimage_entry, pimage, pmtd_info);
		if(retval)
			break;

		pimage_entry++;
	}
	
	snx_destroy_mtd_info(pmtd_info);
	printf ("fwupdate end!\n");
	return retval;
}

