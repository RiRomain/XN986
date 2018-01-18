/**********************************************************************
 *
 * Function:    SPI to GPIO for IR-Cut/LED control program    
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
#include "snx_gpio.h"


/* -------------------------------------------------------------------

NOTE: 
    snx_ir_ctl is an example application to describe how to control a 

    IR CUT. We take MS1_1, MS1_2 as the IR CUT P/N pins,  MS1_3 as the 

    Day/Night mode detection pin. You can modified them to fit your 

    spec. The detail GPIO control methods, please refer to the 

    GPIO example code.

----------------------------------------------------------------------*/
#if SN9866X
#include "snx_aud_gpio.h"
#define DEVICE_NAME_AUD_GPIO "/dev/snx_aud_gpio"

#define IR_CUT_P_PIN		MS1_GPIO_PIN2
#define IR_CUT_N_PIN		AUD_GPIO_PIN_1
#define IR_CUT_DN_PIN		AUD_GPIO_PIN_2
#define LED_PIN				AUD_GPIO_PIN_0

#else

#define IR_CUT_P_PIN		2
#define IR_CUT_N_PIN		4
#define IR_CUT_DN_PIN		3
#define LED_PIN				5

#endif

#define IR_DAY_MODE			1
#define IR_NIGHT_MODE		0

gpio_pin_info ircut_p, ircut_n, ir_dn, led_pin;

#if SN9866X
int open_file(char *filename)
{
	int fd;
	
	fd = open(filename, O_RDWR);
	if (fd == -1) {
		printf("[AUD_GPIO] Open file is fail!\n");
	}
	return (fd);
}

void close_file(int fd)
{
	if (close(fd) != 0) {
		printf("[AUD_GPIO] Close file is fail\n");
	}	
}

void write_file(int fd, struct cmd_buffer val)
{
	int ret;
	//printf("write: 0x%x, 0x%x\n",val.id,  val.data);
	ret = write(fd, &val, 1);
	if (ret <= 0) {
		printf("[AUD_GPIO] Write file is fail\n");
	}
}
#endif

int spi_gpio_init(void)
{
#if SN9866X
	int fd;
	struct cmd_buffer cmd;
#endif
 	snx_ms1_gpio_open();
 	ircut_p.pinumber = IR_CUT_P_PIN;
 	ircut_p.mode 	 = 1; //output mode
 	ircut_p.value 	 = 0; //initialize
 	if (snx_ms1_gpio_write(ircut_p) == GPIO_FAIL)
    	printf ("msi%d gpio fail\n",ircut_p.pinumber); 

#if SN9866X
    cmd.id = AUD_GPIO;
	cmd.data = 1 << AUD_GPIO_DIR_MASK;
	cmd.data += IR_CUT_N_PIN;
	cmd.data |= 0 << AUD_GPIO_VAL_MASK;
	fd = open_file(DEVICE_NAME_AUD_GPIO);
	write_file(fd,cmd);
	close_file(fd);
	
	// Init DN PIN to high first
	cmd.id = AUD_GPIO;
	cmd.data = 1 << AUD_GPIO_DIR_MASK;
	cmd.data += IR_CUT_DN_PIN;
	cmd.data |= 1 << AUD_GPIO_VAL_MASK;
	fd = open_file(DEVICE_NAME_AUD_GPIO);
	write_file(fd,cmd);
	close_file(fd);

	usleep(30000);

	cmd.id = AUD_GPIO;
	cmd.data = 0 << AUD_GPIO_DIR_MASK;
	cmd.data += IR_CUT_DN_PIN;
	cmd.data |= 0 << AUD_GPIO_VAL_MASK;
	fd = open_file(DEVICE_NAME_AUD_GPIO);
	write_file(fd,cmd);
	close_file(fd);

	cmd.id = AUD_GPIO;
	cmd.data = 1 << AUD_GPIO_DIR_MASK;
	cmd.data += LED_PIN;
	cmd.data |= 0 << AUD_GPIO_VAL_MASK;
	fd = open_file(DEVICE_NAME_AUD_GPIO);
	write_file(fd,cmd);
	close_file(fd);
#else

 	ircut_n.pinumber = IR_CUT_N_PIN;
 	ircut_n.mode 	 = 1; //output mode
 	ircut_n.value 	 = 0; //initialize
 	if (snx_ms1_gpio_write(ircut_n) == GPIO_FAIL)
    	printf ("msi%d gpio fail\n",ircut_n.pinumber); 


	// Init DN PIN to high first
    ir_dn.pinumber 		= IR_CUT_DN_PIN;
 	ir_dn.mode 	 		= 1; //output mode
 	ir_dn.value 	 	= 1; 
 	if (snx_ms1_gpio_write(ir_dn) == GPIO_FAIL)
    	printf ("msi%d gpio fail\n",ir_dn.pinumber); 
    
	usleep(30000);
	
	ir_dn.pinumber 		= IR_CUT_DN_PIN;
 	ir_dn.mode 	 		= 0; //input mode
 	ir_dn.value 	 	= 0; 
 	if (snx_ms1_gpio_write(ir_dn) == GPIO_FAIL)
    	printf ("msi%d gpio fail\n",ir_dn.pinumber); 
    
	led_pin.pinumber 		= LED_PIN;
 	led_pin.mode 	 		= 1; //output mode
 	led_pin.value 	 	= 0; 
 	if (snx_ms1_gpio_write(led_pin) == GPIO_FAIL)
    	printf ("msi%d gpio fail\n",led_pin.pinumber); 
#endif
	return 0;
}

int ir_dn_check(void)
{

	int ret;

#if SN9866X

	unsigned char val;
	int fd;
	fd = open_file(DEVICE_NAME_AUD_GPIO);
	ret = read(fd, &val, 1);
	if (!ret) 
			printf("read file is fail\n");
	close_file(fd);
	//printf("%d\n",(val & (1 << pin)) >> pin);
	val = (val & (1 << IR_CUT_DN_PIN)) >> IR_CUT_DN_PIN;
	return val;

#else

	snx_ms1_gpio_read(&ir_dn);
	if (ir_dn.value == 1)
		ret = IR_DAY_MODE;
	else
		ret = IR_NIGHT_MODE;
	return ret;

#endif
}

int ir_cut_set(int op)
{
#if SN9866X
	int fd;
	struct cmd_buffer cmd, cmd_l;
	cmd.id = AUD_GPIO;
	cmd.data = 1 << AUD_GPIO_DIR_MASK;
	cmd.data += IR_CUT_N_PIN;
	
	cmd_l.id = AUD_GPIO;
	cmd_l.data = 1 << AUD_GPIO_DIR_MASK;
	cmd_l.data += LED_PIN;
#endif

	if (op == IR_DAY_MODE) {

#if SN9866X
		ircut_p.value 	 = 0;
		cmd.data |= 1 << AUD_GPIO_VAL_MASK;
		cmd_l.data |= 0 << AUD_GPIO_VAL_MASK;
#else
		ircut_p.value 	 = 0;
		ircut_n.value 	 = 1;
		led_pin.value	 = 0;
#endif
		system("echo 0x40 > /proc/isp/filter/saturation");
        printf("[IR-CTL] Day mode\n");
	} else if ( op == IR_NIGHT_MODE ) {

#if SN9866X
		ircut_p.value 	 = 1;
		cmd.data |= 0 << AUD_GPIO_VAL_MASK;
		cmd_l.data |= 1 << AUD_GPIO_VAL_MASK;
#else
		ircut_p.value 	 = 1;
		ircut_n.value 	 = 0;
		led_pin.value	 = 1;
#endif
		system("echo 0x0 > /proc/isp/filter/saturation");
        printf("[IR-CTL] Night mode\n");
	} else {
		printf("[IR-Cut] Wrong operation!\n");
	}

	if (snx_ms1_gpio_write(ircut_p) == GPIO_FAIL)
		printf ("msi%d gpio fail\n",ircut_p.pinumber);
#if SN9866X
	fd = open_file(DEVICE_NAME_AUD_GPIO);
	write_file(fd,cmd);
	close_file(fd);

    fd = open_file(DEVICE_NAME_AUD_GPIO);
	write_file(fd,cmd_l);
	close_file(fd);
#else
    if (snx_ms1_gpio_write(ircut_n) == GPIO_FAIL)
		printf ("msi%d gpio fail\n",ircut_n.pinumber);
    if (snx_ms1_gpio_write(led_pin) == GPIO_FAIL)
		printf ("msi%d gpio fail\n",led_pin.pinumber);
#endif

	usleep(120000);

if (op == IR_DAY_MODE) {

#if SN9866X
		ircut_p.value 	 = 0;
		cmd.data &= ~(1 << AUD_GPIO_VAL_MASK);
#else
		ircut_p.value 	 = 0;
		ircut_n.value 	 = 0;
#endif
	} else if ( op == IR_NIGHT_MODE ) {

#if SN9866X
		ircut_p.value 	 = 0;
		cmd.data |= 0 << AUD_GPIO_VAL_MASK;
#else
		ircut_p.value 	 = 0;
		ircut_n.value 	 = 0;
#endif
	} else {
		printf("[IR-Cut] Wrong operation!\n");
	}

	if (snx_ms1_gpio_write(ircut_p) == GPIO_FAIL)
		printf ("msi%d gpio fail\n",ircut_p.pinumber);
#if SN9866X
	fd = open_file(DEVICE_NAME_AUD_GPIO);
	write_file(fd,cmd);
	close_file(fd);
#else
    if (snx_ms1_gpio_write(ircut_n) == GPIO_FAIL)
		printf ("msi%d gpio fail\n",ircut_n.pinumber);
#endif
	return 0;

}

int spi_gpio_close(void)
{

	snx_ms1_gpio_close();
	return 0;
}
/*
* main()
*/
int main(int argc,char *argv[])
{
	int dn;
	int pre_dn;
	dn = pre_dn = IR_DAY_MODE;
	spi_gpio_init();
	ir_cut_set(IR_DAY_MODE);
	if (argc == 1) {
		while (1) {
		
			dn=ir_dn_check();
			if(dn != pre_dn) {
				ir_cut_set(dn);
			}
			pre_dn = dn;
			usleep(1000000); //wait one second for the next check
		}
	} else if (argc == 2) {

		if(atoi(argv[1]) == 0) {
	        ir_cut_set(IR_DAY_MODE);
	    } else if (atoi(argv[1]) == 1)
	        ir_cut_set(IR_NIGHT_MODE);
	     else
	     	printf("%s 0/1\n",argv[0]);
	} else {

		printf("%s [0/1]\n",argv[0]);
	}

	spi_gpio_close();
	return 0;
}

