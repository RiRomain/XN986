#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/timex.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define DEVICE_NAME			"snx_timer_test"

struct snx_timer_test_t
{
	struct cdev dev;
	int timer_finish;
	unsigned long timer_alarm_time;

};

int snx_timer_test_major = 255;
int snx_timer_test_minor = 0;
module_param(snx_timer_test_major, int, 0644);
MODULE_PARM_DESC(snx_timer_test_major, "major number of device");

struct snx_timer_test_t snx_timer_test;

static int snx_timer_test_open(struct inode *node, struct file *flip)
{
	return 0;
}

static int snx_timer_test_release(struct inode *node, struct file *filp)
{
	return 0;
}

static void snx_timer_handler(unsigned long arg)
{
	snx_timer_test.timer_finish = 1;
}



static int snx_timer_alarm_test(void)
{
	int retval;
	int timer_id;
	int i;

	printk(KERN_INFO "timer alarm test start.\n");

	timer_id = request_hw_timer();
	if(timer_id < 0)
	{
		printk(KERN_ERR "Request hardware timer fail.\n");
		return -1;
	}

	retval = set_hw_timer_alarm(timer_id, snx_timer_test.timer_alarm_time, snx_timer_handler, 0);
	if(retval < 0)
	{
		printk(KERN_ERR "Set hardware timer alarm fail.\n");
		retval = -2;
		goto err1;
	}

	snx_timer_test.timer_finish = 0;
	retval = enable_hw_timer(timer_id);
	if(retval < 0)
	{
		printk(KERN_ERR "Enable hardware timer fail.\n");
		retval = -3;
		goto err1;
	}
	
	i = 0;
	while(snx_timer_test.timer_finish == 0)
	{
		udelay(snx_timer_test.timer_alarm_time);
		i++;
		if(i > 3)
		{
			printk(KERN_ERR "Timeout!\n");
			retval = -4;
			goto err2;
		}
	}

	retval = free_hw_timer(timer_id);
	if(retval < 0)
	{
		printk(KERN_ERR "Free hardware timer fail.\n");
		return -5;
	}

	return 0;

err2:
	disable_hw_timer(timer_id);
err1:
	free_hw_timer(timer_id);
	return retval;
}

static int snx_timer_measure_test(void)
{
	struct timeval start_time, end_time;
	struct timeval hw_start_time, hw_end_time;
	unsigned long hw_time, time;
	int retval;
	int timer_id;

	printk(KERN_INFO "timer measure test start.\n");

	timer_id = request_hw_timer();
	if(timer_id < 0)
	{
		printk(KERN_ERR "Request hardware timer fail.\n");
		return -1;
	}

	retval = enable_hw_timer_measure_mode(timer_id);
	if(retval < 0)
	{
		printk(KERN_ERR "Enable hardware timer measure mode fail.\n");
		retval = -2;
		goto err1;
	}

	retval = enable_hw_timer(timer_id);
	if(retval < 0)
	{
		printk(KERN_ERR "Enable hardware timer fail.\n");
		retval = -3;
		goto err1;
	}

	do_gettimeofday(&start_time);
	udelay(1000);
	do_gettimeofday(&end_time);
	
	time = (end_time.tv_sec - start_time.tv_sec) * 1000000;
	time += end_time.tv_usec - start_time.tv_usec;

	//printk("end_time %ld %ld, start_time %ld %ld\n",end_time.tv_sec, end_time.tv_usec, start_time.tv_sec, start_time.tv_usec);

	retval = get_hw_timer_time(timer_id, &hw_start_time);
	if(retval < 0)
	{
		printk(KERN_ERR "Get hardware timer time fail.\n");
		retval = -4;
		goto err2;
	}
	udelay(1000);
	retval = get_hw_timer_time(timer_id, &hw_end_time);
	if(retval < 0)
	{
		printk(KERN_ERR "Get hardware timer time fail.\n");
		retval = -5;
		goto err2;
	}

	hw_time = (hw_end_time.tv_sec - hw_start_time.tv_sec) * 1000000;
	hw_time += hw_end_time.tv_usec - hw_start_time.tv_usec;
	//printk("end_time %ld %ld, start_time %ld %ld\n",hw_end_time.tv_sec, hw_end_time.tv_usec, hw_start_time.tv_sec, hw_start_time.tv_usec);

	printk(KERN_ERR "do_gettimeoofday measure time %ldus\n", time);
	printk(KERN_ERR "hw_timer measure time %ldus\n", hw_time);
	
	
	retval = disable_hw_timer(timer_id);
	if(retval < 0)
	{
		printk(KERN_ERR "Disable hardware timer fail.\n");
		return -7;
	}

	retval = free_hw_timer(timer_id);
	if(retval < 0)
	{
		printk(KERN_ERR "Free hardware timer fail.\n");
		return -8;
	}

	return 0;

err2:
	disable_hw_timer(timer_id);
err1:
	free_hw_timer(timer_id);
	return retval;
}

static int snx_timer_measure_and_alarm_test(void)
{
	struct timeval start_time, end_time;
	struct timeval hw_start_time, hw_end_time;
	unsigned long hw_time, time;
	int retval;
	int timer_id;
	int i;

	printk(KERN_INFO "timer measure and alarm test start.\n");

	timer_id = request_hw_timer();
	if(timer_id < 0)
	{
		printk(KERN_ERR "Request hardware timer fail.\n");
		return -1;
	}

	retval = enable_hw_timer_measure_mode(timer_id);
	if(retval < 0)
	{
		printk(KERN_ERR "Enable hardware timer measure mode fail.\n");
		retval = -2;
		goto err1;
	}

	retval = enable_hw_timer(timer_id);
	if(retval < 0)
	{
		printk(KERN_ERR "Enable hardware timer fail.\n");
		retval = -3;
		goto err1;
	}
	
	do_gettimeofday(&start_time);
	udelay(1000);
	do_gettimeofday(&end_time);
	
	time = (end_time.tv_sec - start_time.tv_sec) * 1000000;
	time += end_time.tv_usec - start_time.tv_usec;
	//printk("end_time %ld %ld, start_time %ld %ld\n",end_time.tv_sec, end_time.tv_usec, start_time.tv_sec, start_time.tv_usec);

	retval = get_hw_timer_time(timer_id, &hw_start_time);
	if(retval < 0)
	{
		printk(KERN_ERR "Get hardware timer time fail.\n");
		retval = -4;
		goto err2;
	}

	udelay(1000);

	retval = get_hw_timer_time(timer_id, &hw_end_time);
	if(retval < 0)
	{
		printk(KERN_ERR "Get hardware timer time fail.\n");
		retval = -5;
		goto err2;
	}

	hw_time = (hw_end_time.tv_sec - hw_start_time.tv_sec) * 1000000;
	hw_time += hw_end_time.tv_usec - hw_start_time.tv_usec;
	//printk("end_time %ld %ld, start_time %ld %ld\n",hw_end_time.tv_sec, hw_end_time.tv_usec, hw_start_time.tv_sec, hw_start_time.tv_usec);


	printk(KERN_ERR "do_gettimeofday time %ldus\n", time);
	printk(KERN_ERR "hw_timer time %ldus\n", hw_time);


	retval = disable_hw_timer(timer_id);
	if(retval < 0)
	{
		printk(KERN_ERR "Disable hardware timer fail.\n");
		return -7;
	}

	retval = disable_hw_timer_measure_mode(timer_id);
	if(retval < 0)
	{
		printk(KERN_ERR "Disable hardware timer measure mode fail.\n");
		retval = -8;
		goto err1;
	}

	retval = set_hw_timer_alarm(timer_id, snx_timer_test.timer_alarm_time, snx_timer_handler, 0);
	if(retval < 0)
	{
		printk(KERN_ERR "Set hardware timer alarm fail.\n");
		retval = -9;
		goto err1;
	}

	snx_timer_test.timer_finish = 0;
	retval = enable_hw_timer(timer_id);
	if(retval < 0)
	{
		printk(KERN_ERR "Enable hardware timer fail.\n");
		retval = -10;
		goto err1;
	}

	while(snx_timer_test.timer_finish == 0)
	{
		udelay(snx_timer_test.timer_alarm_time);
		i++;
		if(i > 3)
		{
			printk(KERN_ERR "Timeout!\n");
			retval = -11;
			goto err2;
		}
	}

	retval = free_hw_timer(timer_id);
	if(retval < 0)
	{
		printk(KERN_ERR "Free hardware timer fail.\n");
		return -12;
	}

	return 0;

err2:
	disable_hw_timer(timer_id);

err1:
	free_hw_timer(timer_id);
	return retval;
}
static int snx_timer_cpu_clock_test(void)
{
  long long timeS,timeE; 
  timeS = cpu_clock(0);
  udelay (1000);
  timeE = cpu_clock(0);
  printk ("cpu_clock test = %lld ns",timeE - timeS);
  return 0;
}

static ssize_t snx_timer_test_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp)
{
	int retval = 0;
	char cmd;

	if(copy_from_user(&(cmd), buff, 1))
	{
		return -EFAULT;
	}
	
	switch(cmd)
	{

			
		case 1:
			retval = snx_timer_measure_test();
			break;

    	case 2:
			snx_timer_test.timer_alarm_time = 100;
			retval = snx_timer_alarm_test();
			break;
      
		case 3:
			snx_timer_test.timer_alarm_time = 150;
			retval = snx_timer_measure_and_alarm_test();
			break;
		case 4:
     		snx_timer_cpu_clock_test ();
     		break; 
		default:
			printk(KERN_ERR "command error.\n");
			return 0;
	}

	if(retval)
		printk(KERN_INFO "Fail --- %d\n", retval);
	else
		printk(KERN_INFO "Pass\n");

	return 1;
}

struct file_operations snx_timer_test_fops = 
{
	.owner   = THIS_MODULE,
	.open    = snx_timer_test_open,
	.release = snx_timer_test_release,
	.write   = snx_timer_test_write,
};


static int snx_timer_test_init(void)
{
	dev_t dev_num;
	int retval = 0;

	if(snx_timer_test_major)
	{
		dev_num = MKDEV(snx_timer_test_major, snx_timer_test_minor);
		retval = register_chrdev_region(dev_num, 1, DEVICE_NAME); 
	}
	else
	{
		retval = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
		snx_timer_test_major = MAJOR(dev_num);
	}

	if(retval < 0)
		goto error_device_num;

	cdev_init(&snx_timer_test.dev, &snx_timer_test_fops);
	snx_timer_test.dev.owner = THIS_MODULE;
	snx_timer_test.dev.ops = &snx_timer_test_fops;
	retval = cdev_add(&snx_timer_test.dev, dev_num, 1);
	if(retval)
		goto error_add_device;


	printk(KERN_INFO "snx_timer_test module insert.\n");
	return retval;

error_add_device:
	printk(KERN_ERR "snx_timer_test: can't add test timer %d", dev_num);
	return retval;

error_device_num:
	printk(KERN_ERR "snx_timer_test: can't get major %d\n", snx_timer_test_major);
	return retval;
}


static void snx_timer_test_exit(void)
{
	cdev_del(&snx_timer_test.dev);
	printk(KERN_INFO "snx_timer_test module remove.\n");
}

module_init(snx_timer_test_init);
module_exit(snx_timer_test_exit);

MODULE_AUTHOR("yanjie_yang");
MODULE_DESCRIPTION("test timer driver");
MODULE_LICENSE("Dual BSD/GPL");

