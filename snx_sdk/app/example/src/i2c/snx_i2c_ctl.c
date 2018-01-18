/**********************************************************************
 *
 * Function:   MCU 2511 proction IC 
 * How it works: Host issues a i2c write commands with data (0xa8 0x0) to
 *               MCU (slave addr: 0xA8) periodly. 
 *               MCU will issue reset signal to host if no i2c communication
 *               in 6 seconds. 
 *
 **********************************************************************/
 
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <getopt.h>

#define I2C0_BUS 			"/dev/i2c-0"
#define I2C1_BUS 			"/dev/i2c-1"

#define SNX_I2C_R8D8_MODE	(0)
#define SNX_I2C_R8D16_MODE	(1)
#define SNX_I2C_R16D8_MODE	(2)
#define SNX_I2C_R16D16_MODE	(3)

#define SNX_I2C_OP_WRITE	(0)
#define SNX_I2C_OP_READ		(1)
#define SNX_I2C_OP_MCU		(2)

#define MCU_SLAVE_ADDR		0xA8
#define MCU_OP_CODE1		0xA8
#define MCU_OP_CODE2		0x0

//#define DEBUG

int snx_i2c_open(char *dev)
{
	int fd;
	fd = open(dev, O_RDWR);
	if (fd < 0) {
		printf("open %s failed\n", dev);
		exit(1);
	}
	return fd;
}

int snx_i2c_close(int fd)
{
        return close(fd);
}

int snx_i2c_burst_write(int fd, int chip_addr, int start_addr, int len, void *data, int mode)
{
	struct i2c_msg msgs[1];
	struct i2c_rdwr_ioctl_data ioctl_data;
	int ret = 0;
	int i;
	__u8 *val;
	
	if(len <= 0) {
		ret = -1;
		printf("[SNX_I2C] Wrong len (%d)\n", len);
		return ret;
	}

	val = (__u8 *) malloc(sizeof(__u8) * (len + 1) * 2);

	memset(val, 0x0, (sizeof(__u8) * (len + 1) * 2));

	msgs[0].addr = chip_addr;
	msgs[0].flags = 0;

	switch (mode)
	{
		case SNX_I2C_R8D8_MODE:
			msgs[0].len = 1 + len;
			msgs[0].buf = val;
			val[0] = start_addr;
			for (i = 0; i < len; i++) {
				__u8 *new_data = (__u8 *) data;
				val[i+1] = new_data[i];
			}
			break;

		case SNX_I2C_R8D16_MODE:
			msgs[0].len = 1 + (len * 2);
			msgs[0].buf = val;
			val[0] = start_addr;
			for (i = 0; i < len; i++) {
				__u16 *new_data = (__u16 *) data;
				val[i+1] = (__u8)(new_data[i] >> 8);
				val[i+2] = (__u8)(new_data[i] & 0xff);
			}
			break;

		case SNX_I2C_R16D8_MODE:
			msgs[0].len = 2 + len;
			msgs[0].buf = val;
			val[0] = (__u8)(start_addr >> 8);
			val[1] = (__u8)(start_addr & 0xff);

			for (i = 0; i < len; i++) {
				__u8 *new_data = (__u8 *) data;
				val[i+2] = new_data[i];
			}
			break;

		case SNX_I2C_R16D16_MODE:
			msgs[0].len = 2 + (len * 2);
			msgs[0].buf = val;
			val[0] = (__u8)(start_addr >> 8);
			val[1] = (__u8)(start_addr & 0xff);
			for (i = 0; i < len; i++) {
				__u16 *new_data = (__u16 *) data;
				val[i+1] = (__u8)(new_data[i] >> 8);
				val[i+2] = (__u8)(new_data[i] & 0xff);
			}
			break;
		default:
			printf("[SNX_I2C] Wrong Mode (%d)\n", mode);
			break;

	}
	
	
	ioctl_data.nmsgs = 1;
	ioctl_data.msgs = msgs;

#ifdef DEBUG

	printf("----- Writing Data -------\n\n");
	printf("-- \tchipaddr: 0x%x, reg: 0x%x\n", chip_addr, start_addr);
	for (i = 0; i < msgs[0].len ; i ++) {
		printf("-- \t msgs.data[%d] = 0x%x\n", i, val[i]);
	}
#endif

	ret = ioctl(fd, I2C_RDWR, &ioctl_data);
	if (ret < 0) {
		printf("%s: ioctl return: %d\n", __func__, ret);
	}
	

	return ret;
}

int snx_i2c_write(int fd, int chip_addr, int addr, int data, int mode)
{

#if 0
	struct i2c_msg msgs[1];
	struct i2c_rdwr_ioctl_data ioctl_data;
	int ret;
	__u8 buf[2];

	buf[0] = addr;
	buf[1] = data;
	msgs[0].addr = chip_addr;
	msgs[0].flags = 0;				//Write Operation
	msgs[0].len = 2;				
	msgs[0].buf = buf;

	ioctl_data.nmsgs = 1;
	ioctl_data.msgs = msgs;
	ret = ioctl(fd, I2C_RDWR, &ioctl_data);
	if (ret < 0) {
		printf("%s: ioctl return: %d\n", __func__, ret);
	}
#else
	int ret = 0;

	ret = snx_i2c_burst_write(fd, chip_addr, addr, 1, &data, mode);

#endif

	return ret;
}

int snx_i2c_burst_read(int fd, int chip_addr, int start_addr, int len, void *data, int mode)
{
	struct i2c_msg msgs[2];
	struct i2c_rdwr_ioctl_data ioctl_data;
	int ret = 0;
	int i;
	__u8 *val;
	
	if(len <= 0) {
		ret = -1;
		printf("[SNX_I2C] Wrong len (%d)\n", len);
		return ret;
	}

	val = (__u8 *) malloc(sizeof(__u8) * len * 2);

	memset(val, 0x0, (sizeof(__u8) * len * 2));

	msgs[0].addr = chip_addr;
	msgs[0].flags = 0;

	switch (mode)
	{
		case SNX_I2C_R8D8_MODE:
		case SNX_I2C_R8D16_MODE:
			msgs[0].len = 1;
			msgs[0].buf = val;
			val[0] = start_addr;
			break;

		case SNX_I2C_R16D8_MODE:
		case SNX_I2C_R16D16_MODE:
			msgs[0].len = 2;
			msgs[0].buf = val;
			val[0] = (__u8)(start_addr >> 8);
			val[1] = (__u8)(start_addr & 0xff);
			break;
		default:
			printf("[SNX_I2C] Wrong Mode (%d)\n", mode);
			break;

	}
	
	msgs[1].addr = chip_addr;
	msgs[1].flags = I2C_M_RD;

	switch (mode)
	{
		case SNX_I2C_R8D8_MODE:
		case SNX_I2C_R16D8_MODE:
		
			msgs[1].len = len;
			msgs[1].buf = val;
			break;

		case SNX_I2C_R8D16_MODE:
		case SNX_I2C_R16D16_MODE:

			msgs[1].len = len * 2;
			msgs[1].buf = val;
			break;
		default:
			printf("[SNX_I2C] Wrong Mode (%d)\n", mode);
			break;

	}

	
	ioctl_data.nmsgs = 2;
	ioctl_data.msgs = msgs;
	ret = ioctl(fd, I2C_RDWR, &ioctl_data);
	if (ret < 0) {
		printf("%s: ioctl return: %d\n", __func__, ret);
	}

	for (i = 0; i < len; i++) {
		switch (mode)
		{
			case SNX_I2C_R8D8_MODE:
			case SNX_I2C_R16D8_MODE:
			
				{
					__u8 *new_data = (__u8 *) data;
				
					new_data[i] = (__u8)val[i];

    			}
				break;

			case SNX_I2C_R8D16_MODE:
			case SNX_I2C_R16D16_MODE:
				{
					__u16 *new_data = (__u16 *) data;
				
					new_data[i] = (__u16)val[i*2];
	    			new_data[i] = (new_data[i]  << 8 )| (__u16)val[ (i*2 + 1)];
    			}
				break;
			default:
				printf("[SNX_I2C] Wrong Mode (%d)\n", mode);
				break;

		}

	}
#ifdef DEBUG
	printf("----- reading Data -------\n\n");
	printf("-- \tchipaddr: 0x%x, reg: 0x%x\n", chip_addr, start_addr);
	for (i = 0; i < msgs[1].len ; i ++) {
		printf("-- \t msgs.data[%d] = 0x%x\n", i, val[i]);
	}
#endif
	return ret;
}

int snx_i2c_read(int fd, int chip_addr, int addr, int mode)
{
	int value = 0;

	snx_i2c_burst_read(fd, chip_addr, addr, 1, &value, mode);

	return value;
}

int snx_mcu_querry(int fd, int chip_addr, void *data)
{
	struct i2c_msg msgs[2];
	struct i2c_rdwr_ioctl_data ioctl_data;
	int ret;

	msgs[0].addr = chip_addr;
	msgs[0].flags = 0;
	msgs[0].len = 2;
	msgs[0].buf = (__u8 *)data;
	
	ioctl_data.nmsgs = 1;
	ioctl_data.msgs = msgs;
	ret = ioctl(fd, I2C_RDWR, &ioctl_data);
	if (ret < 0) {
		printf("%s: ioctl return: %d\n", __func__, ret);
	}
	return ret;
}

static void usage(FILE * fp, int argc, char ** argv) {   
    fprintf(fp, "Usage: %s [options]/n\n"   
        "Options:\n"
        "-D I2C device 0: i2c-0 1: i2c-1\n"  
        "-C cmd 0:write 1:read \n"  
        "-a address\n"
        "-r register\n"
        "-v value \n"
        "-m operation mode \n"
        "   0: SNX_I2C_R8D8_MODE \n"
        "   1: SNX_I2C_R8D16_MODE \n"
        "   2: SNX_I2C_R16D8_MODE \n"
        "   3: SNX_I2C_R16D16_MODE \n"
        "", argv[0]);   
}  
static const char short_options[] = "D:C:a:r:v:m:";   
static const struct option long_options[] =
{
		{ "help", no_argument, NULL, 'h' },
		{ "device", required_argument, NULL, 'D' },
		{ "command", required_argument, NULL, 'C' },
		{ "address", required_argument, NULL, 'a' }, 
		{ "reg", required_argument, NULL, 'r' },
		{ "value", required_argument, NULL, 'v' }, 
		{ "mode", required_argument, NULL, 'm' },    
		{ 0, 0, 0, 0 }
}; 
/*
* main()
*/
int main(int argc,char *argv[])
{                          

	int reg = -1; 
	int value = -1; 
	int addr = -1;
	int status = -1;
	int fd = 0;
	char opcode[2] = {MCU_OP_CODE1, MCU_OP_CODE2};
	int device = 0;
	int mode = SNX_I2C_R8D8_MODE;
	int len = 1;
	int i;
	addr = MCU_SLAVE_ADDR;

	
  	for (;;)
	{
	    int index;   
	    int c;   

	    c = getopt_long(argc, argv, short_options, long_options, &index);

	    if (-1 == c)   
				break;   
			switch (c)
			{   
				case 0: /* getopt_long() flag */   
					break;
				case 'h':   
					usage(stdout, argc, argv);   
					exit(0);
				case 'D':   
					sscanf(optarg, "%d", &device);
					if ((device < 0) || (device > 1))
					{
						printf("Wrong device number (%d)\n", device);
						exit(0);
					}
					break;   
				case 'C':   
					sscanf(optarg, "%d", &status);
					if ((mode < SNX_I2C_OP_WRITE) || (mode > SNX_I2C_OP_MCU) ) {
						printf("[SNX_I2C] Wrong MODE (%d)\n", mode);
						exit(0);
					}
					break;   
				case 'a':
					sscanf(optarg, "%x", &addr);
					break;
				case 'm':
					sscanf(optarg, "%x", &mode);
					if ((mode < SNX_I2C_R8D8_MODE) || (mode > SNX_I2C_R16D16_MODE) ) {
						printf("[SNX_I2C] Wrong MODE (%d)\n", mode);
						exit(0);
					}
					break;
				case 'r':
					sscanf(optarg, "%x", &reg);
					break;
				case 'v':
					sscanf(optarg, "%x", &value);
					break;
				default:   
					usage(stderr, argc, argv);   
					exit(0);   
			}    
  	}

  	if (device)
  		fd = snx_i2c_open(I2C1_BUS);
  	else
  		fd = snx_i2c_open(I2C0_BUS);

  	printf("I2C device (%d) is open\n",device);
  
	if(fd > 0) {
	
		if (status == SNX_I2C_OP_WRITE) {
			if (addr == -1 || reg == -1 || value == -1)
			{
				usage(stdout, argc, argv);   
				goto EXIT;  
			}

			snx_i2c_write(fd, addr, reg, value, mode);
			printf("addr: %x set 0x%x = 0x%x\n", addr, reg, value);
			goto EXIT;
	    }		
		else if (status == SNX_I2C_OP_READ) {
			if (addr == -1 || reg == -1)
			{
				usage(stdout, argc, argv);   
				goto EXIT;  
			}

			if (len == 1) 
				printf("addr: 0x%x get 0x%x = 0x%x\n", addr, reg, snx_i2c_read(fd, addr, reg, mode));
			else {
				
				{
					void *value;
					switch(mode) {
						case SNX_I2C_R8D8_MODE:
						case SNX_I2C_R16D8_MODE:
							value = (__u8*) malloc(sizeof(__u8)*len);
							break;
						case SNX_I2C_R16D16_MODE:
						case SNX_I2C_R8D16_MODE:
							value = (__u16*) malloc(sizeof(__u16)*len);
							break;
					}
					snx_i2c_burst_read(fd, addr, reg, len, value, mode);
					printf("addr: 0x%x start address: 0x%x len: %d\n", addr, reg, len);
					printf("data: \t");

					for(i = 0; i < len; i++) {

						switch(mode) {
						case SNX_I2C_R8D8_MODE:
						case SNX_I2C_R16D8_MODE:
							printf("0x%x\n", ((__u8*)(value))[i]);
							break;
						case SNX_I2C_R16D16_MODE:
						case SNX_I2C_R8D16_MODE:
							printf("0x%x\n", ((__u16*)(value))[i]);
							break;
						}
						
					}
				}
				
			}
			goto EXIT;
		}
#if 0
		else if (status == SNX_I2C_OP_MCU){
			printf("[SNX MCU CTL] Slave addr: 0x%x, Op code: 0x%x 0x%x\n", MCU_SLAVE_ADDR, MCU_OP_CODE1, MCU_OP_CODE2);
			while (1) {
			
			if(snx_mcu_querry(fd, MCU_SLAVE_ADDR, opcode) < 0)
				break;
				
				usleep(5000000);
			}
	    }
#endif
	    else
	      usage(stdout, argc, argv);     
        
EXIT:
		snx_i2c_close(fd);
  }
	return 0;
}


