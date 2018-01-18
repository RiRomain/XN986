/** \file time.c
 * Sonix Timer driver implement file
 * \n Functions list:
 * \n 1. Support system timer.
 * \n 2. Support special hardware timer.
 * \author yanjie_yang
 * \date   2011/12/01
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/irq.h>

#include <linux/io.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/clocksource.h>
#include <linux/cnt32_to_63.h>
#include <asm/mach/time.h>
#include <asm/div64.h>

#include <mach/regs-irq.h>
#include <mach/regs-timer.h>

#include <linux/sched.h>
#include "generated/snx_sdk_conf.h"

#define TOTAL_COUNT	512
#define MAX_ST58660_APB	20000000	//20M
u64 *timestamp_prt;
int tag_array[TOTAL_COUNT];

static unsigned long timer_clock;

int stamp_cnt = (TOTAL_COUNT - 1);

static cycle_t snx_clksrc_read(struct clocksource *cs)
{
        return cnt32_to_63(readl(TIMER3_COUNT));
}

static struct clocksource clksrc_ftclock = {
        .name           = "ft_clocksource",
        .shift          = 8,
        .rating         = 300,
        .read           = snx_clksrc_read,
        .mask           = CLOCKSOURCE_MASK(32),
        .flags          = CLOCK_SOURCE_IS_CONTINUOUS,
};

int init_timestamp_buf(void)
{
	timestamp_prt = kmalloc(TOTAL_COUNT * 8, GFP_KERNEL);

	if (timestamp_prt == NULL)
		return -1;

	memset(tag_array, 0, sizeof(tag_array));

	return 0;
}

void _snx_log_timestamp(int tag)
{
	*(timestamp_prt + stamp_cnt) = cpu_clock(0);
	tag_array[stamp_cnt] = tag;

	if (stamp_cnt == 0)
		stamp_cnt = TOTAL_COUNT - 1;
	else
		stamp_cnt--;

}
EXPORT_SYMBOL(_snx_log_timestamp);

void _snx_show_timestamp(struct seq_file *f)
{
	int i = 0;
	unsigned long long t;
	unsigned long nanosec_rem;

	seq_printf(f, "Tag\t\tTimestamp\n");

	if (stamp_cnt != 0 && tag_array[stamp_cnt - 1] != 0) {
		for (i = stamp_cnt - 1 ; i >=0 ; i --) {
			t = *(timestamp_prt + i);
			nanosec_rem = do_div(t, 1000000000);
			seq_printf(f, "%d\t\t%5lu.%06lu\n", tag_array[i],
						(unsigned long) t,
						nanosec_rem / 1000);
		}
	}

	for (i = TOTAL_COUNT - 1 ; i > stamp_cnt ; i--) {
		t = *(timestamp_prt + i);
		nanosec_rem = do_div(t, 1000000000);
		seq_printf(f, "%d\t\t%5lu.%06lu\n", tag_array[i],
					(unsigned long) t,
					nanosec_rem / 1000);
	}

	return;
}
EXPORT_SYMBOL(_snx_show_timestamp);


#ifdef CONFIG_SPECIAL_HW_TIMER

#define SPECIAL_TIMER_NUM		1 	/*!< The number of special hardware timer */

/** \struct sonix_hw_timer
 * \brief data struct of sonix special hardware timer
 * \n
 * \n handler:Callback function
 * \n arg:Parameter of callback function
 * \n use:The timer is used. (1 - used, 0 - no used)
 * \n enable:The timer is enable. (1 - enable, 0 - disable)
 * \n measure_mode:The timer is used as time measurement. (1 - measure, 0 - alarm)
 * \n lock: The spin lock of this struct
 *
 */
struct sonix_hw_timer
{
	timer_handler_t handler;
	unsigned long arg;
	int use;
	int enable;
	int measure_mode;
	int second;
	spinlock_t lock;
};

struct sonix_hw_timer snx_hw_timer[SPECIAL_TIMER_NUM];

/**
 * \Microseconds units to tick count
 */
inline static unsigned int us2tick(unsigned int us)
{
	return us * (timer_clock / 100000) / 10 ;
}

/**
 * \tick count to Microseconds units 
 */
inline static unsigned int tick2us(unsigned int tick)
{
	unsigned int tickof10us;

	/* the procedure of calculate use 10 microseconds precision */
	tickof10us = timer_clock / 100000;

	if (tickof10us)
		return (tick * 10 + tickof10us / 2) / tickof10us;
	else
		return 0;
}


/**
 * \defgroup TIME_C_G1 The interface of special hardware timer
 * @{
 */

/** \fn int request_hw_timer(void)
 * \brief Request one special hardware timer
 * \return The ID of timer. (greater than or equal to 0:success, less than 0:fail)
 */
int request_hw_timer(void)
{
	unsigned long flags;
	int i;

	for (i = 0; i < SPECIAL_TIMER_NUM; i++)
	{
		spin_lock_irqsave (&(snx_hw_timer[i].lock), flags);
		if (snx_hw_timer[i].use == 0)
		{
			snx_hw_timer[i].use = 1;
			spin_unlock_irqrestore (&(snx_hw_timer[i].lock), flags);
			return i;
		}
		spin_unlock_irqrestore (&(snx_hw_timer[i].lock), flags);
	}

	printk (KERN_WARNING "Request hardware timer fail. hardware timers have run out.\n");
	return -EBUSY;
}
EXPORT_SYMBOL (request_hw_timer);

/** \fn int free_hw_timer(int timer_id)
 * \brief Free one special hardware timer
 * param timer_id :The ID of timer
 * \return zero:success, nonzero:fail)
 */
int free_hw_timer (int timer_id)
{
	unsigned long flags;

	if ((timer_id >= SPECIAL_TIMER_NUM) || (timer_id < 0))
		return -EINVAL;
	
	spin_lock_irqsave (&(snx_hw_timer[timer_id].lock), flags);
	if (snx_hw_timer[timer_id].enable)
	{
		spin_unlock_irqrestore (&(snx_hw_timer[timer_id].lock), flags);
		printk (KERN_ERR "Can not free timer when the timer is enable.\n");
		return -EBUSY;
	}
	snx_hw_timer[timer_id].use = 0;
	snx_hw_timer[timer_id].measure_mode = 0;
	snx_hw_timer[timer_id].second = 0;
	snx_hw_timer[timer_id].handler = NULL;
	snx_hw_timer[timer_id].arg = 0;
	spin_unlock_irqrestore (&(snx_hw_timer[timer_id].lock), flags);

	return 0;
}
EXPORT_SYMBOL (free_hw_timer);

/** \fn int set_hw_timer_alarm(int timer_id, unsigned int ms, timer_handler_t handler, unsigned long arg)
 * \brief Set the alarm of special hardware timer
 * param timer_id :The ID of timer
 * param ms :The alarm time (in Microseconds units)
 * param handler :Callback function
 * param arg :the parameter of callback function
 * \return zero:success, nonzero:fail)
 */
int set_hw_timer_alarm (int timer_id, unsigned int ms, timer_handler_t handler, unsigned long arg)
{
	unsigned long flags;
	unsigned int value;
	unsigned int second;

	if ((timer_id >= SPECIAL_TIMER_NUM) || (timer_id < 0))
		return -EINVAL;

	if (ms == 0)
		return -EINVAL;

	spin_lock_irqsave (&(snx_hw_timer[timer_id].lock), flags);
	if (snx_hw_timer[timer_id].use == 0)
	{
		spin_unlock_irqrestore (&(snx_hw_timer[timer_id].lock), flags);
		printk (KERN_ERR "Request the hardware timer before the first use.\n");
		return -EINVAL;
	}

	if (snx_hw_timer[timer_id].enable)
	{
		spin_unlock_irqrestore (&(snx_hw_timer[timer_id].lock), flags);
		printk (KERN_ERR "Can not set timer interval when the timer is enable.\n");
		return -EBUSY;
	}

	if (snx_hw_timer[timer_id].measure_mode)
	{
		spin_unlock_irqrestore (&(snx_hw_timer[timer_id].lock), flags);
		printk (KERN_ERR "Can not set timer interval when timer work in the measurement mode.\n");
		return -EINVAL;
	}

	second = ms / 1000000;
	value = ms % 1000000;
	if (value == 0)
	{
		second--;
		value = timer_clock;
	}
	else
		value = us2tick (value);


	if (timer_id == 0)
	{
		//timer 2
		writel (value, TIMER2_COUNT);
		writel (timer_clock, TIMER2_LOAD);
	}
	else //if(timer_id == 1)
	{
		//timer 3
		writel (value, TIMER3_COUNT);
		writel (timer_clock, TIMER3_LOAD);
	}
	snx_hw_timer[timer_id].handler = handler;
	snx_hw_timer[timer_id].arg = arg;
	snx_hw_timer[timer_id].second = second;
	spin_unlock_irqrestore (&(snx_hw_timer[timer_id].lock), flags);

	return 0;
}
EXPORT_SYMBOL (set_hw_timer_alarm);

/** \fn int get_hw_timer_time(int timer_id, struct timeval* tv)
 * \brief Get the time of special hardware timer
 * param timer_id :The ID of timer
 * param tv :time value
 * \return zero:success, nonzero:fail)
 */
int get_hw_timer_time (int timer_id, struct timeval* tv)
{
	unsigned long flags;
	unsigned int value;

	if ((timer_id >= SPECIAL_TIMER_NUM) || (timer_id < 0))
		return -EINVAL;

	spin_lock_irqsave (&(snx_hw_timer[timer_id].lock), flags);
	if (snx_hw_timer[timer_id].use == 0)
	{
		spin_unlock_irqrestore (&(snx_hw_timer[timer_id].lock), flags);
		printk (KERN_ERR "Request the hardware timer before the first use.\n");
		return -EINVAL;
	}

	if (snx_hw_timer[timer_id].measure_mode == 0)
	{
		spin_unlock_irqrestore (&(snx_hw_timer[timer_id].lock), flags);
		printk (KERN_ERR "The timer don't work in the measurement mode.\n");
		return -1;
	}

	if (snx_hw_timer[timer_id].enable == 0)
	{
		spin_unlock_irqrestore (&(snx_hw_timer[timer_id].lock), flags);
		printk (KERN_ERR "Please enable the timer first.\n");
		return -2;
	}

	if (timer_id == 0)
	{
		value = readl (TIMER2_LOAD);
		value -= readl (TIMER2_COUNT);
	}
	else //if(timer_id == 1)
	{
		value = readl (TIMER3_LOAD);
		value -= readl (TIMER3_COUNT);
	}

	tv->tv_usec = tick2us (value);
	tv->tv_sec = snx_hw_timer[timer_id].second;
	spin_unlock_irqrestore (&(snx_hw_timer[timer_id].lock), flags);

	return 0;
}
EXPORT_SYMBOL (get_hw_timer_time);

/** \fn int enable_hw_timer_measure_mode(int timer_id)
 * \brief Enable the measure mode of special hardware timer
 * param timer_id :The ID of timer
 * \return zero:success, nonzero:fail)
 */
int enable_hw_timer_measure_mode (int timer_id)
{
	unsigned long flags;
	
	if((timer_id >= SPECIAL_TIMER_NUM) || (timer_id < 0))
		return -EINVAL;

	spin_lock_irqsave (&(snx_hw_timer[timer_id].lock), flags);
	if (snx_hw_timer[timer_id].use == 0)
	{
		spin_unlock_irqrestore (&(snx_hw_timer[timer_id].lock), flags);
		printk (KERN_ERR "Request the hardware timer before the first use.\n");
		return -EINVAL;
	}
	
	if (snx_hw_timer[timer_id].enable)
	{
		spin_unlock_irqrestore (&(snx_hw_timer[timer_id].lock), flags);
		printk (KERN_ERR "Can not set timer mode when the timer is enable.\n");
		return -EBUSY;
	}

	if (timer_id == 0)
	{
		//timer 2
		writel (timer_clock, TIMER2_COUNT);
		writel (timer_clock, TIMER2_LOAD);
	}
	else //if(timer_id == 1)
	{
		//timer 3
		writel (timer_clock, TIMER3_COUNT);
		writel (timer_clock, TIMER3_LOAD);
	}
	
	snx_hw_timer[timer_id].measure_mode = 1;
	snx_hw_timer[timer_id].second = 0;
	snx_hw_timer[timer_id].handler = NULL;
	snx_hw_timer[timer_id].arg = 0;

	spin_unlock_irqrestore (&(snx_hw_timer[timer_id].lock), flags);

	return 0;
}
EXPORT_SYMBOL (enable_hw_timer_measure_mode);

/** \fn int disable_hw_timer_measure_mode(int timer_id)
 * \brief Disable the measure mode of special hardware timer
 * param timer_id :The ID of timer
 * \return zero:success, nonzero:fail)
 */
int disable_hw_timer_measure_mode (int timer_id)
{
	unsigned long flags;
	
	if((timer_id >= SPECIAL_TIMER_NUM) || (timer_id < 0))
		return -EINVAL;

	spin_lock_irqsave (&(snx_hw_timer[timer_id].lock), flags);
	if (snx_hw_timer[timer_id].use == 0)
	{
		spin_unlock_irqrestore (&(snx_hw_timer[timer_id].lock), flags);
		printk (KERN_ERR "Request the hardware timer before the first use.\n");
		return -EINVAL;
	}
	
	if (snx_hw_timer[timer_id].enable)
	{
		spin_unlock_irqrestore (&(snx_hw_timer[timer_id].lock), flags);
		printk (KERN_ERR "Can not set timer mode when the timer is enable.\n");
		return -EBUSY;
	}

	snx_hw_timer[timer_id].measure_mode = 0;
	spin_unlock_irqrestore (&(snx_hw_timer[timer_id].lock), flags);

	return 0;
}
EXPORT_SYMBOL (disable_hw_timer_measure_mode);

/** \fn int enable_hw_timer(int timer_id)
 * \brief Enable special hardware timer
 * param timer_id :The ID of timer
 * \return zero:success, nonzero:fail)
 */
int enable_hw_timer (int timer_id)
{
	unsigned long flags;
	unsigned int value;
	
	if((timer_id >= SPECIAL_TIMER_NUM) || (timer_id < 0))
		return -EINVAL;

	if (snx_hw_timer[timer_id].use == 0)
	{
		printk (KERN_ERR "Request the hardware timer before the first use.\n");
		return -EINVAL;
	}

	spin_lock_irqsave (&(snx_hw_timer[timer_id].lock), flags);
	snx_hw_timer[timer_id].enable = 1;
	value = readl (TIMER_CTRL);
//#ifdef CONFIG_SYSTEM_PLATFORM_ST58660FPGA
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
	if (timer_id == 0) {
		if (timer_clock >= MAX_ST58660_APB)
			value |= (TM2ENABLE | TM2CLOCK | TM2OFENABLE);
		else
			value |= ((TM2ENABLE | TM2OFENABLE) & (~TM2CLOCK));
	} else {
		if (timer_clock >= MAX_ST58660_APB)
			value |= (TM3ENABLE | TM3OFENABLE | TM3CLOCK);
		else
			value |= ((TM3ENABLE | TM3OFENABLE) & (~TM3CLOCK));
	}
#else
	if (timer_id == 0)
		value |= ((TM2ENABLE | TM2OFENABLE) & (~TM2CLOCK));
	else
		value |= ((TM3ENABLE | TM3OFENABLE) & (~TM3CLOCK));
#endif

	writel (value, TIMER_CTRL);
	spin_unlock_irqrestore (&(snx_hw_timer[timer_id].lock), flags);

	return 0;
}
EXPORT_SYMBOL (enable_hw_timer);

/** \fn int disable_hw_timer(int timer_id)
 * \brief Disable special hardware timer
 * param timer_id :The ID of timer
 * \return zero:success, nonzero:fail)
 */
int disable_hw_timer (int timer_id)
{
	unsigned long flags;
	unsigned int value;

	if((timer_id >= SPECIAL_TIMER_NUM) || (timer_id < 0))
		return -EINVAL;

	spin_lock_irqsave (&(snx_hw_timer[timer_id].lock), flags);
	snx_hw_timer[timer_id].enable = 0;
	value = readl (TIMER_CTRL);
	if (timer_id == 0)
		value &= (~(TM2ENABLE | TM2OFENABLE));
	else
		value &= (~(TM3ENABLE | TM3OFENABLE));
	
	writel (value, TIMER_CTRL);
	spin_unlock_irqrestore (&(snx_hw_timer[timer_id].lock), flags);

	return 0;
}
EXPORT_SYMBOL (disable_hw_timer);
/** @} */

/** \fn static int __init hw_timer_init(void)
 * \brief Initialize special hardware timer
 * \return zero:success, nonzero:fail)
 */
static int __init hw_timer_init (void)
{
	unsigned int value;
	int i;

	for (i = 0; i < SPECIAL_TIMER_NUM; i++)
	{
		snx_hw_timer[i].handler = NULL;
		snx_hw_timer[i].arg = 0;
		snx_hw_timer[i].enable = 0;
		snx_hw_timer[i].use = 0;
		snx_hw_timer[i].second = 0;
		snx_hw_timer[i].measure_mode = 0;

		spin_lock_init (&(snx_hw_timer[i].lock));
	}
#if 0
	value = readl (TIMER_CTRL);
	value &= (~(TM2ENABLE | TM3ENABLE | TM2OFENABLE | TM3OFENABLE));
	value &= (~(TM2CLOCK | TM3CLOCK | TM2UPDOWN | TM3UPDOWN));
	writel (value, TIMER_CTRL);

	value = readl (TIMER_INTRMASK);
	value &= (~(TM2OF | TM3OF));
	writel (value, TIMER_INTRMASK);
#endif

	value = readl (TIMER_CTRL);
	value &= (~(TM2ENABLE | TM2OFENABLE));
	value &= (~(TM2CLOCK | TM2UPDOWN));
	writel (value, TIMER_CTRL);

	value = readl (TIMER_INTRMASK);
	value &= (~(TM2OF));
	writel (value, TIMER_INTRMASK);
	return 0;
}
#endif

/** \fn static unsigned long snx_gettimeoffset(void)
 * \brief Returns number of ms since last clock interrupt.
 * \return The number of ms
 */
static unsigned long snx_gettimeoffset (void)
{
	unsigned long load, count, elapsed, usec, irqpend;

	load = readl (TIMER1_LOAD);

        /* check to see if there is an interrupt pending */
	irqpend = readl (IRQ_FLAG);
	if (irqpend & IRQ_TIMER) {
		count =  readl (TIMER1_COUNT);
       		elapsed = load - count;

       		if (count != 0)
       			elapsed += load;
	} else {
		/* Get ticks before next timer match */
		elapsed = load - readl (TIMER1_COUNT);
	}
	/* Now, convert them to usec */
	usec = (unsigned long)(elapsed * jiffies_to_usecs(1)) / LATCH;

	return usec;
}

/** \fn static irqreturn_t snx_timer_interrupt(int irq, void *dev_id)
 * \brief IRQ handler of timer
 */
static irqreturn_t snx_timer_interrupt (int irq, void *dev_id)
{
#ifdef CONFIG_SPECIAL_HW_TIMER
	unsigned long val;
#endif

//	BUG_ON(!irqs_disabled());	// mingfeng

	writel (CLR_TM1OF, TIMER_INTRFLAG);
	timer_tick ();

#ifdef CONFIG_SPECIAL_HW_TIMER
	/* clear the interrupt of timer overflow */
	val = readl (TIMER_INTRFLAG);

	if (val & TM2OF) {
		writel (CLR_TM2OF, TIMER_INTRFLAG);
		if (snx_hw_timer[0].measure_mode) {
			snx_hw_timer[0].second++;
		} else {
			if (snx_hw_timer[0].second == 0) {
				disable_hw_timer (0);
				if (snx_hw_timer[0].handler != NULL)
					(snx_hw_timer[0].handler)(snx_hw_timer[0].arg);
			} else {
				snx_hw_timer[0].second--;
			}
		}
	}
#endif

	return IRQ_HANDLED;
}

static struct irqaction snx_timer_irq = {
	.name		= "snx-timer",
	.flags		= IRQF_DISABLED | IRQF_TIMER,
	.handler	= snx_timer_interrupt,
};

/** \fn static void __init snx_timer_setup(void)
 * \brief Initialize the timer controller
 */
static void __init snx_timer_setup (void)
{
	/* disalble timer1/2/3 */
	writel (0x0, TIMER_CTRL);
	/* clear timer1/2/3 */
	writel (CLR_TM_INT_STS, TIMER_INTRFLAG);

	/* setup counter value for the first time */
	writel (LATCH, TIMER1_COUNT);
	/* setup load value */
	writel (LATCH, TIMER1_LOAD);
	/* enable timer1 overflow interrupt only */
	writel (~TM1OF, TIMER_INTRMASK);

	/* enbale timer1, overflow, count down */
	//writel ((TM1ENABLE | TM1OFENABLE | TM3OFENABLE | TM3ENABLE | TM3UPDOWN) & (~(TM1CLOCK | TM1UPDOWN)), TIMER_CTRL);
//#ifdef CONFIG_SYSTEM_PLATFORM_ST58660FPGA
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
	if (APB_CLK >= MAX_ST58660_APB)
		writel ((((TM1ENABLE | TM1OFENABLE | TM3ENABLE | TM3UPDOWN) & (~ TM1UPDOWN)) | (TM1CLOCK | TM3CLOCK)), TIMER_CTRL);
	else
#endif
		writel ((TM1ENABLE | TM1OFENABLE | TM3ENABLE | TM3UPDOWN) & (~(TM1CLOCK | TM1UPDOWN | TM3CLOCK)), TIMER_CTRL);
}

/** \fn static void __init snx_timer_init(void)
 * \brief Initialize the timer controller and setup timer interrupt.
 */
static void __init snx_timer_init (void)
{
//#ifdef CONFIG_SYSTEM_PLATFORM_ST58660FPGA
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
	if (APB_CLK >= MAX_ST58660_APB)
		timer_clock = 10000000;	//10M
	else
#endif
		timer_clock = APB_CLK;
	
	clksrc_ftclock.mult = clocksource_hz2mult((LATCH * HZ), clksrc_ftclock.shift);
	/*
         * We want an even value to automatically clear the top bit
         * returned by cnt32_to_63() without an additional run time
         * instruction. So if the LSB is 1 then round it up.
         */
	clksrc_ftclock.mult &= ~0x1;
	clocksource_register(&clksrc_ftclock);

	snx_clock_init();

	/* setup timer */
	snx_timer_setup ();

	/* Init log timestamp buffer */
	init_timestamp_buf();

#ifdef CONFIG_SPECIAL_HW_TIMER
	hw_timer_init ();
#endif

	/* Make irqs happen for the system timer in INTC*/
	setup_irq(INT_TIMER, &snx_timer_irq);
}

struct sys_timer snx_timer = 
{
	.init		= snx_timer_init,
	.offset	= snx_gettimeoffset
};

unsigned long long sched_clock(void)
{
	unsigned long long v = cnt32_to_63(readl(TIMER3_COUNT));

	return clocksource_cyc2ns(v, clksrc_ftclock.mult ,clksrc_ftclock.shift);
}

