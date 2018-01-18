/**********************************************************************
 *
 * Function:   MCU 2711 Light Sensor, IR-CUT, IR LED Control
 * 
 * 
 * 
 * 
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

#include "snx_isp/isp_lib_api.h"

#define I2C_BUS 			"/dev/i2c-1"
//#define I2C_BUS 			"/dev/i2c-0"
#define MCU_SLAVE_ADDR		0xA8
#define MCU_OP_CODE1		0xA8
#define MCU_OP_CODE2		0x0

#define MCU_CONNECTION_PERIOD   2000000
#define MCU_CONNECTION_RETRY    5

#define TRY_COUNTS				5


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

int snx_i2c_burst_read(int fd, int chip_addr, int start_addr, int end_addr, __u16 *data)
{
	struct i2c_msg msgs[2];
	struct i2c_rdwr_ioctl_data ioctl_data;
	int ret;
	__u8 val[2];

	msgs[0].addr = chip_addr;
	msgs[0].flags = 0;
	msgs[0].len = 1;
	msgs[0].buf = val;
	val[0] = start_addr;
	
	msgs[1].addr = chip_addr;
	msgs[1].flags = I2C_M_RD;
	msgs[1].len = end_addr-start_addr+2;
	msgs[1].buf = val;
	
	ioctl_data.nmsgs = 2;
	ioctl_data.msgs = msgs;
	ret = ioctl(fd, I2C_RDWR, &ioctl_data);
	if (ret < 0) {
		printf("%s: ioctl return: %d\n", __func__, ret);
	}

	*data = (__u16)val[0];
    *data = (*data  << 8 )| (__u16)val[1];

	return ret;
}

int snx_i2c_read(int fd, int chip_addr, int addr, __u16 *data)
{
	//__u8 value = -1;
	if(data==NULL)
		return -1;

	snx_i2c_burst_read(fd, chip_addr, addr, addr, data);

	return 0;
}

/*
* main()
*/
int main(int argc,char *argv[])
{
	int fd;
	unsigned int usecs=MCU_CONNECTION_PERIOD;
	__u16 light_sensor_brightness=0;
	int ret=0;
	int Hight_Bound=0x260;
	int Low_Bound=0x90;
	int Enter_Hight_Bound_Times;
	int Enter_Low_Bound_Times;
	
	if(argc!=3){
		printf("\n");
		printf("Usage : snx_mcu2711_ctrl Hight_Bound Low_Bound\n");
		printf("Hight_Bound Range : 0x000~0x3FF \n");
		printf("Low_Bound   Range : 0x000~0x3FF \n");
		printf("Example :\n");
		printf("snx_mcu2711_ctrl 0x260 0x90\n");
		printf("\n");
		return 0;
	}
	else{
		Hight_Bound = strtoul(argv[1],NULL,16);
		printf("Hight_Bound=0x%08X\n",Hight_Bound);
		Low_Bound   = strtoul(argv[2],NULL,16);
		printf("Low_Bound=0x%08X\n",Low_Bound);		
	}


	system("echo out > /sys/class/gpio/gpio4/direction");
	system("echo out > /sys/class/gpio/gpio6/direction");	
	Enter_Hight_Bound_Times=0;
	Enter_Low_Bound_Times=0;

	fd = snx_i2c_open(I2C_BUS);
	//printf("[SNX MCU CTL] Slave addr: 0x%x\n", MCU_SLAVE_ADDR);
	while (1) {

		usleep(usecs);
//		printf("usleep=%d\n",usecs);
		ret=snx_i2c_read(fd, MCU_SLAVE_ADDR, 0xA9, &light_sensor_brightness);
		if(ret)
			printf("error when i2c read\n");
		printf("light_sensor_brightness=0x%08X\n",light_sensor_brightness);
		
		if(light_sensor_brightness>=Hight_Bound){/* IR_DAY_MODE */
			Enter_Hight_Bound_Times++;
			if(Enter_Hight_Bound_Times==2){
				printf("\nIR_DAY_MODE\n");
				system("echo 1 > /etc/ir_cut");
				system("echo 0 > /proc/isp/iq/nrn");				
				Enter_Low_Bound_Times=0;
				system("echo 0 > /sys/class/gpio/gpio4/value;echo 0 > /sys/class/gpio/gpio6/value");
				usleep(120000);
				system("echo 0 > /sys/class/gpio/gpio4/value;echo 1 > /sys/class/gpio/gpio6/value");
				usleep(120000);
				system("echo 0 > /sys/class/gpio/gpio4/value;echo 0 > /sys/class/gpio/gpio6/value");
				usleep(120000);
				system("gpio_led -n 0 -m 1 -v 0");

				/* Black-White mode to Color mode */
				snx_isp_filter_saturation_set(0x40);
				printf("\n");
				printf("Color mode!!\n");
				printf("\n");
			}
			else if(Enter_Hight_Bound_Times>=3){
				Enter_Hight_Bound_Times=3;
				Enter_Low_Bound_Times=0;
			}
		}
		else
		if(light_sensor_brightness<=Low_Bound){/* IR_NIGHT_MODE */
			Enter_Low_Bound_Times++;
			if(Enter_Low_Bound_Times==2){
				printf("\nIR_NIGHT_MODE\n");
				system("echo 0 > /etc/ir_cut");
				system("echo 1 > /proc/isp/iq/nrn");								
				Enter_Hight_Bound_Times=0;

				/* Color mode to Black-White mode */
				snx_isp_filter_saturation_set(0x0);
				printf("\n");
				printf("Black-White mode!!\n");
				printf("\n");
				
				system("echo 0 > /sys/class/gpio/gpio4/value;echo 0 > /sys/class/gpio/gpio6/value");
				usleep(120000);
				system("echo 1 > /sys/class/gpio/gpio4/value;echo 0 > /sys/class/gpio/gpio6/value");
				usleep(120000);				
				system("echo 0 > /sys/class/gpio/gpio4/value;echo 0 > /sys/class/gpio/gpio6/value");
				usleep(120000);
				system("gpio_led -n 0 -m 1 -v 1");

			}
			else if(Enter_Low_Bound_Times>=3){
				Enter_Low_Bound_Times=3;
				Enter_Hight_Bound_Times=0;
				system("gpio_led -n 0 -m 1 -v 1");
			}			
		}		
		else{
			Enter_Hight_Bound_Times=0;
			Enter_Low_Bound_Times=0;		
		}
		
		
	}
	snx_i2c_close(fd);
	return 0;
}

/*
IR LED : PWM2
~ # gpio_led -n 0 -m 1 -v 0
~ # gpio_led -n 0 -m 1 -v 1

Green LED : PWM1
~ # gpio_led -n 1 -m 1 -v 1
~ # gpio_led -n 1 -m 1 -v 0

Red LED : GPIO3
~ # echo out > /sys/class/gpio/gpio3/direction
~ # echo 1 > /sys/class/gpio/gpio3/value
~ # gpio_led -n 3 -m 1 -v 1
~ # gpio_led -n 3 -m 1 -v 0

Blue LED : AUD_IO0(=PWM3)
~ # gpio_aud write 1 0 1
~ # gpio_aud write 1 0 0

~ # gpio_led -n 4 -m 1 -v 1 (Forever ON)
~ # gpio_led -n 4 -m 1 -v 0 (Forever OFF)

~ # snx_pwm_period 4 500 1000 (Blinking 500ms/1000ms)
~ # gpio_led -n 4 -m 1 -v 0

~ # snx_pwm_period 4 1 10  (Forever ON with min  brightness)
~ # snx_pwm_period 4 5 10  (Forever ON with half brightness)
~ # snx_pwm_period 4 10 10 (Forever ON with max  brightness)


IR-CUT : GPIO4 & GPIO6
echo out > /sys/class/gpio/gpio4/direction
echo out > /sys/class/gpio/gpio6/direction

echo 0 > /sys/class/gpio/gpio4/value;echo 0 > /sys/class/gpio/gpio6/value
echo 1 > /sys/class/gpio/gpio4/value;echo 0 > /sys/class/gpio/gpio6/value no 650um 
echo 0 > /sys/class/gpio/gpio4/value;echo 1 > /sys/class/gpio/gpio6/value 650um

*/

