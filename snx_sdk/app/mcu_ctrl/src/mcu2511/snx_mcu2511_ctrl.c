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
#include <sys/stat.h>

#include <stdarg.h>
#include <sys/time.h>

#define I2C_BUS 			"/dev/i2c-0"
#define MCU_SLAVE_ADDR		0xA8
#define MCU_OP_CODE1		0xA8
#define MCU_OP_CODE2		0x0

#define MCU_CONNECTION_PERIOD   2000000
#define MCU_CONNECTION_RETRY    5

#define TRY_COUNTS				5
#define MPTOOL_DETECT_FLAG		"Sx4Oi3Nn2Io1Xs0"

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

int snx_i2c_write(int fd, int chip_addr, int addr, int data)
{
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

	return ret;
}

int snx_i2c_burst_read(int fd, int chip_addr, int start_addr, int end_addr, void *data)
{
	struct i2c_msg msgs[2];
	struct i2c_rdwr_ioctl_data ioctl_data;
	int ret;

	msgs[0].addr = chip_addr;
	msgs[0].flags = 0;
	msgs[0].len = 1;
	msgs[0].buf = data;
	((__u8 *)data)[0] = start_addr;
	
	msgs[1].addr = chip_addr;
	msgs[1].flags = I2C_M_RD;
	msgs[1].len = end_addr-start_addr+1;
	msgs[1].buf = data;
	
	ioctl_data.nmsgs = 2;
	ioctl_data.msgs = msgs;
	ret = ioctl(fd, I2C_RDWR, &ioctl_data);
	if (ret < 0) {
		printf("%s: ioctl return: %d\n", __func__, ret);
	}

	return ret;
}

int snx_i2c_read(int fd, int chip_addr, int addr)
{
	__u8 value = -1;

	snx_i2c_burst_read(fd, chip_addr, addr, addr, &value);

	return value;
}

int snx_mcu_querry(int fd, int chip_addr, void *data)
{
	struct i2c_msg msgs[2];
	struct i2c_rdwr_ioctl_data ioctl_data;
	int ret = 0;

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


void LOG(const char *fmt, ...)
{
	if (fmt == NULL)return;

	char szBuffer [4096];
	szBuffer[4095] = 0;

	va_list	ap;
	va_start(ap, fmt);
	vsnprintf( szBuffer, sizeof (szBuffer) - 1, fmt, ap) ;
	va_end(ap);
	
	struct timeval tv;
	struct tm now;
	int ms = 0;
	
	memset(&tv, 0, sizeof(tv));
	gettimeofday(&tv, NULL);

	memset(&now, 0, sizeof(now));
	localtime_r(&tv.tv_sec, &now);
	ms = tv.tv_usec/1000;

	printf("%02d-%02d %02d:%02d:%02d.%03u %s\n", 
		now.tm_mon + 1, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec, ms,
		szBuffer);
}

int snx_mcu_querry_try(int fd, int chip_addr, void *data, int trycnts)
{
	int i = 0, ret = 0, has_error = 0;
	
	do{
		if(has_error)
		{
			LOG("snx_mcu_query retrying %d", i);
		}		
		
		ret = snx_mcu_querry(fd, chip_addr, data);
		
		if(ret < 0)
		{
			LOG("snx_mcu_query failed %d", ret);
			has_error = 1;
		}
		else{
			//LOG("snx_mcu_query OK %d", ret);
		}
		
		usleep(10000);
	}while((ret < 0) && (i++<trycnts));
  
	if(has_error)
	{
		LOG("snx_mcu_query finshed ret=%d, try=%d", ret, i);
	}	
		  
	if((ret  < 0) || (i>= trycnts))
		return ret;
	return 0;
}

/*
* main()
*/
int main(int argc,char *argv[])
{
	int fd;
	char opcode[2] = {MCU_OP_CODE1, MCU_OP_CODE2};
	int mcu_querry_fail=0;
	unsigned int usecs=MCU_CONNECTION_PERIOD;
	char sys_cmd[128];
	struct stat tst;

	memset(sys_cmd, 0x0, 128);
	sprintf(sys_cmd,"/media/mmcblk0p1/MPTool/%s", MPTOOL_DETECT_FLAG);
 	
	if (stat(sys_cmd, &tst) != -1) {
		printf("MPTOOL_DETECT_FLAG Detected\n");
		return 0;
	}

	memset(sys_cmd, 0x0, 128);
	sprintf(sys_cmd,"/media/mmcblk0/MPTool/%s", MPTOOL_DETECT_FLAG);
	
	if (stat(sys_cmd, &tst) != -1) {
		printf("MPTOOL_DETECT_FLAG Detected\n");
		return 0;
	}

	
	fd = snx_i2c_open(I2C_BUS);
	//printf("[SNX MCU CTL] Slave addr: 0x%x, Op code: 0x%x 0x%x\n", MCU_SLAVE_ADDR, MCU_OP_CODE1, MCU_OP_CODE2);
	while (1) {

#if 1
			if(snx_mcu_querry_try(fd, MCU_SLAVE_ADDR, opcode, TRY_COUNTS) < 0){
				printf("%s: usleep(%d) mcu_querry_fail(%d)\n", __func__, usecs, mcu_querry_fail);			
				mcu_querry_fail++;
			}
			else{
				if (mcu_querry_fail >0)
				    printf("%s: usleep(%d) mcu_querry_fail(OK)\n", __func__, usecs);
				mcu_querry_fail=0;
			}

            if(mcu_querry_fail  > MCU_CONNECTION_RETRY) {
                printf("%s: mcu retry failed -- no MCU connection\n", __func__);
                break;
            }
#endif
		usleep(usecs);
	}
	snx_i2c_close(fd);
	return 0;
}


