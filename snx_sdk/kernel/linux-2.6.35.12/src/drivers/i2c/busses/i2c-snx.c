/* file i2c-snx.c
 * SNX i2c driver
 * Functions list:
 * 1. I2C controller communicate with I2c slave.
 * author timing_gong
 * date   2012/04/09
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/i2c-id.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/slab.h>

#include <asm/io.h>
#include <asm/irq.h>

#include <mach/regs-i2c.h>

#define SUPPORT_I2C_GPIO  1

#if SUPPORT_I2C_GPIO
#include <mach/platform.h>
#include <linux/miscdevice.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#define I2C_GPIO_OFFSET 0x1c
#define I2C_MODE_MASK		0x3
#define I2C_SCL_OUTPUT_EN	0x4
#define I2C_SDA_OUTPUT_EN	0x8

enum
{
	GPIO_SCL0_PIN,
	GPIO_SDA0_PIN,
	GPIO_SCL1_PIN,
	GPIO_SDA1_PIN,
};
#define I2C_GPIO_mode 0x2

// ioctl cmd
#define I2C_GPIO_WRITE 	1
#define I2C_GPIO_READ 	0

spinlock_t	i2c_lock;
#endif

/* struct snx_i2c
 * brief sonix i2c device struct
 *
 * lock: i2c spinlock
 * wait: i2c wait queue head
 * msg: the msg of i2c send/receive
 * msg_num : nr of pointer msg
 * msg_idx : index of msg
 * msg_ptr :  record current msg had send bytes
 * i2cclk : i2c clock rate
 * slave_addr : slave address
 * clk : a pointer to i2c clock (struct clk)
 * res : a pionter to i2c resoure
 * mapbase : for ioremap
 * membase : read/write[bwl]
 * irq : irq number
 * icr : i2c control register value
 * adap : i2c_adapter struct
 * irqlog : current log index
 * isrlog : record state register
 * icrlog : record control register
 *
 */
struct snx_i2c {
	spinlock_t		lock;
	wait_queue_head_t	wait;
	struct i2c_msg		*msg;
	unsigned int		msg_num;
	unsigned int		msg_idx;
	unsigned int		msg_ptr;
	unsigned int		i2cclk;
	unsigned int		slave_addr;
	struct clk		*clk;
	struct resource		*res;
	unsigned long		mapbase;
	unsigned char		*membase;
	unsigned int		irq;
	unsigned int		icr;

	struct i2c_adapter	adap;

#ifdef CONFIG_I2C_SNX_SLAVE
	struct i2c_slave_client *slave;
#endif

	unsigned int	irqlogidx;
	u32		isrlog[32];
	u32		icrlog[32];
};

#define I2C_NR			2	/*!< Number of I2C controller */

#define I2C_SNX_SLAVE_ADDR	0x1	/*!< I2C Slave mode address */

#define I2C_STANDARD_CLK	100000
#define I2C_FAST_CLK		400000

#define DEF_TIMEOUT		32
#define BUS_ERROR		(-EREMOTEIO)
#define XFER_NAKED		(-ECONNREFUSED)
#define I2C_RETRY		(-2000)	/* an error has occurred retry transmit */

#ifdef DEBUG

#define i2c_debug		1

struct bits {
	u32	mask;
	const char *set;
	const char *unset;
};

#define BIT(m, s, u) { .mask = m, .set = s, .unset = u }

static const struct bits isr_bits[] = {
	BIT(ISR_RW,     "RX",		 "TX"),
	BIT(ISR_ACKNAK, "NAK",		 "ACK"),
	BIT(ISR_CB,     "Bsy",		 "Rdy"),
	BIT(ISR_BB,     "BusBsy",	 "BusRdy"),
	BIT(ISR_STOP,	"SlaveStop", NULL),
	BIT(ISR_ALD,    "ALD",       NULL),
	BIT(ISR_DT,     "TxEmpty",   NULL),
	BIT(ISR_DR,     "RxFull",    NULL),
	BIT(ISR_GCAD,   "GenCall",   NULL),
	BIT(ISR_SAM,    "SlaveAddr", NULL),
	BIT(ISR_BED,    "BusErr",    NULL),
};

static const struct bits icr_bits[] = {
	BIT(ICR_START,  "START",	NULL),
	BIT(ICR_STOP,   "STOP",		NULL),
	BIT(ICR_ACKNAK, "ACKNAK",	NULL),
	BIT(ICR_TB,     "TB",		NULL),
	BIT(ICR_MA,     "MA",		NULL),
	BIT(ICR_SCLE,   "SCLE",		"scle"),
	BIT(ICR_I2CE,   "IUE",		"iue"),
	BIT(ICR_GCD,    "GCD",		NULL),
	BIT(ICR_ITEIE,  "ITEIE",	NULL),
	BIT(ICR_IRFIE,  "IRFIE",	NULL),
	BIT(ICR_BEIE,   "BEIE",		NULL),
	BIT(ICR_SSDIE,  "SSDIE",	NULL),
	BIT(ICR_ALDIE,  "ALDIE",	NULL),
	BIT(ICR_SADIE,  "SADIE",	NULL),
	BIT(ICR_RST,    "RST",		"ur"),
};

static inline void
decode_bits(const char *prefix, const struct bits *bits, int num, u32 val)
{
	printk("%s %08x: ", prefix, val);
	while (num--) {
		const char *str = val & bits->mask ? bits->set : bits->unset;
		if (str)
			printk("%s ", str);
		bits++;
	}
}

static void decode_ISR(unsigned int val)
{
	decode_bits(KERN_DEBUG "I2C_ISR", isr_bits, ARRAY_SIZE(isr_bits), val);
	printk("\n");
}

static void decode_ICR(unsigned int val)
{
	decode_bits(KERN_DEBUG "I2C_ICR", icr_bits, ARRAY_SIZE(icr_bits), val);
	printk("\n");
}

static void snx_i2c_show_state(struct snx_i2c *i2c, int lno, const char *fname)
{
	dev_dbg(&i2c->adap.dev, "state:%s:%d: I2C_ISR=%08x, I2C_ICR=%08x, I2C_IBMR=%02x\n",
		fname, lno,
		i2c_in(i2c, I2C_ISR_OFFSET),
		i2c_in(i2c, I2C_ICR_OFFSET),
		i2c_in(i2c, I2C_IBMR_OFFSET));
}

#define show_state(i2c) snx_i2c_show_state(i2c, __LINE__, __FUNCTION__)

static void snx_i2c_scream_blue_murder(struct snx_i2c *i2c, const char *why)
{
	unsigned int i;

	printk("i2c: error: %s\n", why);

	printk("i2c: msg_num: %d msg_idx: %d msg_ptr: %d\n",
		i2c->msg_num, i2c->msg_idx, i2c->msg_ptr);

	printk("i2c: I2C_ICR: %08x I2C_ISR: %08x\n" "i2c: log: ",
	       i2c_in(i2c, I2C_ICR_OFFSET),
	       i2c_in(i2c, I2C_ISR_OFFSET));

	for (i = 0; i < i2c->irqlogidx; i++)
		printk("[%08x:%08x] ", i2c->isrlog[i], i2c->icrlog[i]);

	printk("\n");
}

static void snx_i2c_register_dump(struct snx_i2c *i2c)
{
	u32 v;
	v = __raw_readl(i2c->membase + I2C_ICR_OFFSET);
	printk("I2C_ICR_OFFSET:%08x\n", v);
	v = __raw_readl(i2c->membase + I2C_ISR_OFFSET);
	printk("I2C_ISR_OFFSET:%08x\n", v);
	v = __raw_readl(i2c->membase + I2C_ICDR_OFFSET);
	printk("I2C_ICDR_OFFSET:%08x\n", v);
	v = __raw_readl(i2c->membase + I2C_IDBR_OFFSET);
	printk("I2C_IDBR_OFFSET:%08x\n", v);
	v = __raw_readl(i2c->membase + I2C_ISAR_OFFSET);
	printk("I2C_ISAR_OFFSET:%08x\n", v);
	v = __raw_readl(i2c->membase + I2C_TGSR_OFFSET);
	printk("I2C_TGSR_OFFSET:%08x\n", v);
}


#else

#define i2c_debug	0

#define show_state(i2c) do { } while (0)
#define decode_ISR(val) do { } while (0)
#define decode_ICR(val) do { } while (0)

static void snx_i2c_scream_blue_murder(struct snx_i2c *i2c, const char *why)
{
}

static void snx_i2c_register_dump(struct snx_i2c *i2c)
{
}

#endif

static struct snx_i2c *snx_i2cs[I2C_NR];


/* fn statuc inline unsigned int i2c_in(struct snx_i2c *i2c, int offset)
 * brief i2c_in - read i2c register
 *
 * param i2c: a pointer to struct snx_i2c
 * param offset: the offset of register
 *
 * return the register value
 */

static inline unsigned int i2c_in(struct snx_i2c *i2c, int offset)
{
	return __raw_readl(i2c->membase + offset);
}

/* fn static inline void i2c_out (struct snx_i2c *i2c, int offset, int value)
 * brief i2_out - write i2c register
 *
 * param i2c: a pointer to struct snx_i2c
 * param offset: the offset of register
 * param value: data which writing to register
 *
 * return NON
 */

static inline void i2c_out(struct snx_i2c *i2c, int offset, int value)
{
	__raw_writel(value, i2c->membase + offset);
}


static inline int snx_i2c_is_slavemode(struct snx_i2c *i2c)
{
	return !(i2c_in(i2c, I2C_ICR_OFFSET) & ICR_SCLE);
}

static void snx_i2c_abort(struct snx_i2c *i2c)
{
	unsigned long timeout = jiffies + HZ/4;

	if (snx_i2c_is_slavemode(i2c)) {
		dev_dbg(&i2c->adap.dev, "%s: called in slave mode\n", __func__);
		return;
	}

	while (time_before(jiffies, timeout) && (i2c_in(i2c, I2C_IBMR_OFFSET) & 0x1) == 0) {

		i2c->icr = i2c_in(i2c, I2C_ICR_OFFSET);
		i2c->icr &= ~ICR_START;
		i2c->icr |= ICR_ACKNAK | ICR_STOP | ICR_TB;
		i2c_out(i2c, I2C_ICR_OFFSET, i2c->icr);

		show_state(i2c);

		msleep(1);
	}

	i2c->icr = i2c_in(i2c, I2C_ICR_OFFSET);
	i2c->icr &= ~(ICR_START | ICR_STOP);
	i2c_out(i2c, I2C_ICR_OFFSET, i2c->icr);
}

/* fn static void snx_i2c_reset(struct snx_i2c *i2c)
 * brief snx_i2c_reset - reset the i2c controller
 *
 * param i2c: snx_i2c
 *
 * return NON
 */

static void snx_i2c_reset(struct snx_i2c *i2c)
{
	unsigned int gsr, cnt;

	pr_debug("Resetting I2C Controller Unit\n");

	/* abort any transfer currently under way */
	snx_i2c_abort(i2c);

	/* reset I2C controller */
	i2c->icr = ICR_RST;
	i2c_out(i2c, I2C_ICR_OFFSET, i2c->icr);

	/* read status register to clear */
	i2c_in(i2c, I2C_ISR_OFFSET);

	/* set control register values
	 * Enable detecting non-ACK responses interrupt
	 * Enable receiveing one data byte interrupt
	 * Enable transmitting one data byte interrupt
	 */
	i2c->icr = ICR_BEIE | ICR_DRIE | ICR_DTIE;
	i2c_out(i2c, I2C_ICR_OFFSET, i2c->icr);

#ifdef CONFIG_I2C_SNX_SLAVE
	dev_info(&i2c->adap.dev, "Enabling slave mode\n");

	i2c_out(i2c, I2C_ISAR_OFFSET, i2c->slave_addr);

	i2c->icr = i2c_in(i2c, I2C_ICR_OFFSET);
	i2c->icr |= ICR_STARTIE | ICR_ALDIE | ICR_SAMIE | ICR_STOPIE;
	i2c_out(i2c, I2C_ICR_OFFSET, i2c->icr);

	snx_i2c_set_slave(i2c, 0);
#endif

	/* set I2C clock */
	/* BIT 12:10 R/W I2C1_GSR[2:0]*/
	gsr = (i2c_in(i2c, I2C_TGSR_OFFSET) & 0x1c00) >> 10;

	i2c->i2cclk = clk_get_rate(i2c->clk);


	cnt = i2c->i2cclk / 10;
	if(cnt > I2C_FAST_CLK)
		cnt = I2C_FAST_CLK;
	else
		cnt = I2C_STANDARD_CLK;

	/*i2c speed 100k HZ*/
	cnt = I2C_STANDARD_CLK;

	cnt = (i2c->i2cclk/cnt - gsr)/2 - 2;

	i2c_out(i2c, I2C_ICDR_OFFSET, cnt);

	/* enable I2C
	 * Enable I2C controller clock output for master mode operation
	 * Enable I2C controller to respond to a general call message as a slave
	 */
	i2c->icr = i2c_in(i2c, I2C_ICR_OFFSET);
	i2c->icr |= ICR_SCLE | ICR_I2CE;
	i2c_out(i2c, I2C_ICR_OFFSET, i2c->icr);

	udelay(100);
}

static int snx_i2c_wait_bus_busy(struct snx_i2c *i2c)
{
	int timeout = DEF_TIMEOUT;

	while (timeout-- && i2c_in(i2c, I2C_ISR_OFFSET) & (ISR_BB | ISR_CB)) {
		if ((i2c_in(i2c, I2C_ISR_OFFSET) & ISR_SAM) != 0)
			timeout += 4;

		udelay(100);
		show_state(i2c);
	}

	if (timeout <= 0)
		show_state(i2c);

	return timeout <= 0 ? I2C_RETRY : 0;
}

static int snx_i2c_wait_master(struct snx_i2c *i2c)
{
	unsigned long timeout = jiffies + HZ*4;

	while (time_before(jiffies, timeout)) {
		if (i2c_debug > 1)
			dev_dbg(&i2c->adap.dev, "%s: %ld: I2C_ISR=%08x, I2C_ICR=%08x, I2C_IBMR=%02x\n",
				__func__, (long)jiffies,
				i2c_in(i2c, I2C_ISR_OFFSET),
				i2c_in(i2c, I2C_ICR_OFFSET),
				i2c_in(i2c, I2C_IBMR_OFFSET));

		if (i2c_in(i2c, I2C_ISR_OFFSET) & ISR_SAM) {
			if (i2c_debug > 0)
				dev_dbg(&i2c->adap.dev, "%s: Slave detected\n", __func__);
			goto out;
		}

		/* wait for unit and bus being not busy, and we also do a
		 * quick check of the i2c lines themselves to ensure they've
		 * gone high...
		 */
		if ((i2c_in(i2c, I2C_ISR_OFFSET) & (ISR_CB | ISR_BB)) == 0
			&& i2c_in(i2c, I2C_IBMR_OFFSET) == 3) {
			if (i2c_debug > 0)
				dev_dbg(&i2c->adap.dev, "%s: done\n", __func__);
			return 1;
		}

		udelay(100);
	}

	if (i2c_debug > 0)
		dev_dbg(&i2c->adap.dev, "%s: did not free\n", __func__);
 out:
	return 0;
}

/* fn static int snx_i2c_set_master(struct snx_i2c *i2c)
 * brief snx_i2c_set_master - set i2c master mode
 *
 * param i2c: address of snx_i2c
 *
 * return zero: success nonzero: fail
 */
static int snx_i2c_set_master(struct snx_i2c *i2c)
{
	if (i2c_debug)
		dev_dbg(&i2c->adap.dev, "setting to bus master\n");

	if ((i2c_in(i2c, I2C_ISR_OFFSET) & (ISR_CB | ISR_BB)) != 0) {

		dev_dbg(&i2c->adap.dev, "%s: unit is busy\n", __func__);

		if (!snx_i2c_wait_master(i2c)) {
			dev_dbg(&i2c->adap.dev, "%s: error: unit busy\n", __func__);
      printk ("%s: error: unit busy\n", __func__);
			return I2C_RETRY;
		}
	}

	/* Enable I2C controller clock output for master mode operation */
	i2c->icr = i2c_in(i2c, I2C_ICR_OFFSET);
	i2c->icr |= ICR_SCLE;
	i2c_out(i2c, I2C_ICR_OFFSET, i2c->icr);

	return 0;
}

/* fn static void snx_i2c_master_complete(struct snx_i2c *i2c, int ret)
 * brief snx_i2c_master_complete - complete the message and wake up.
 *
 * param i2c: address of snx_i2c
 * param ret: transfer return status
 *
 * return NON
 */
static void snx_i2c_master_complete(struct snx_i2c *i2c, int ret)
{
	i2c->msg_ptr = 0;
	i2c->msg = NULL;
	i2c->msg_idx ++;
	i2c->msg_num = 0;

	if (ret)
		i2c->msg_idx = ret;

	wake_up(&i2c->wait);
}

#ifdef CONFIG_I2C_SNX_SLAVE
#if 0
static int snx_i2c_wait_slave(struct snx_i2c *i2c)
{
	unsigned long timeout = jiffies + HZ*1;

	/* wait for stop */

	show_state(i2c);

	while (time_before(jiffies, timeout)) {
		if (i2c_debug > 1)
			dev_dbg(&i2c->adap.dev, "%s: %ld: I2C_ISR=%08x, I2C_ICR=%08x, I2C_IBMR=%02x\n",
				__func__, (long)jiffies,
				i2c_in(i2c, I2C_ISR_OFFSET),
				i2c_in(i2c, I2C_ICR_OFFSET),
				i2c_in(i2c, I2C_IBMR_OFFSET));

		if ((i2c_in(i2c, I2C_ISR_OFFSET) & (ISR_CB|ISR_BB|ISR_SAM)) == ISR_SAM
			|| (i2c_in(i2c, I2C_ICR_OFFSET) & ICR_SCLE) == 0) {
			if (i2c_debug > 1)
				dev_dbg(&i2c->adap.dev, "%s: done\n", __func__);
			return 1;
		}

		msleep(1);
	}

	if (i2c_debug > 0)
		dev_dbg(&i2c->adap.dev, "%s: did not free\n", __func__);

	return 0;
}

/*
 * clear the hold on the bus, and take of anything else
 * that has been configured
 */
static void snx_i2c_set_slave(struct snx_i2c *i2c, int errcode)
{
	show_state(i2c);

	if (errcode < 0) {
		udelay(100);   /* simple delay */
	} else {
		/* we need to wait for the stop condition to end */

		/* if we where in stop, then clear... */
		if (i2c_in(i2c, I2C_ICR_OFFSET) & ICR_STOP) {
			udelay(100);
			i2c->icr = i2c_in(i2c, I2C_ICR_OFFSET);
			i2c->icr &= ~ICR_STOP;
			i2c_out(i2c, I2C_ICR_OFFSET, i2c->icr);
		}

		if (!snx_i2c_wait_slave(i2c)) {
			dev_err(&i2c->adap.dev, "%s: wait timedout\n",
				__func__);
			return;
		}
	}

	i2c->icr = i2c_in(i2c, I2C_ICR_OFFSET);
	i2c->icr &= ~(ICR_STOP|ICR_ACKNAK|ICR_SCLE);
	i2c_out(i2c, I2C_ICR_OFFSET, i2c->icr);

	if (i2c_debug) {
		dev_dbg(&i2c->adap.dev, "I2C_ICR now %08x, I2C_ISR %08x\n",
		i2c_in(i2c, I2C_ICR_OFFSET),
		i2c_in(i2c, I2C_ISR_OFFSET));
		decode_ICR(I2C_ICR);
	}
}

/*
 * SNX I2C Slave mode
 */
static void snx_i2c_slave_txempty(struct snx_i2c *i2c, u32 isr)
{
	if (isr & ISR_BED) {
		/* what should we do here? */
	} else {
		int ret = i2c->slave->read(i2c->slave->data);

		i2c_out(i2c, I2C_IDBR, ret);
		i2c->icr = i2c_in(i2c, I2C_ICR_OFFSET);
		i2c->icr |= ICR_TB;   /* allow next byte */
		i2c_out(i2c, I2C_ICR_OFFSET, i2c->icr);
	}
}

static void snx_i2c_slave_rxfull(struct snx_i2c *i2c, u32 isr)
{
	unsigned int byte = i2c_in(i2c, I2C_IDBR_OFFSET);

	if (i2c->slave != NULL)
		i2c->slave->write(i2c->slave->data, byte);

	i2c->icr = i2c_in(i2c, I2C_ICR_OFFSET);
	i2c->icr |= ICR_TB;
	i2c_out(i2c, I2C_ICR_OFFSET, i2c->icr);
}

static void snx_i2c_slave_start(struct snx_i2c *i2c, u32 isr)
{
	int timeout;

	if (i2c_debug)
		dev_dbg(&i2c->adap.dev, "SAD, mode is slave-%cx\n",
		       (isr & ISR_RW) ? 'r' : 't');

	if (i2c->slave != NULL)
		i2c->slave->event(i2c->slave->data,
				 (isr & ISR_RW) ? I2C_SLAVE_EVENT_START_READ : I2C_SLAVE_EVENT_START_WRITE);

	/*
	 * slave could interrupt in the middle of us generating a
	 * start condition... if this happens, we'd better back off
	 * and stop holding the poor thing up
	 */
	i2c->icr = i2c_in(i2c, I2C_ICR_OFFSET);
	i2c->icr &= ~(ICR_START|ICR_STOP);
	i2c->icr |= ICR_TB;
	i2c_out(i2c, I2C_ICR_OFFSET, i2c->icr);

	timeout = 0x10000;

	while (1) {
		if ((i2c_in(i2c, I2C_IBMR_OFFSET) & 2) == 2)
			break;

		timeout--;

		if (timeout <= 0) {
			dev_err(&i2c->adap.dev, "timeout waiting for SCL high\n");
			break;
		}
	}

	i2c->icr = i2c_in(i2c, I2C_ICR_OFFSET);
	i2c->icr &= ~ICR_SCLE;
	i2c_out(i2c, I2C_ICR_OFFSET, i2c->icr);
}

static void snx_i2c_slave_stop(struct snx_i2c *i2c)
{
	if (i2c_debug > 2)
		dev_dbg(&i2c->adap.dev, "I2C_ISR: SSD (Slave Stop)\n");

	if (i2c->slave != NULL)
		i2c->slave->event(i2c->slave->data, I2C_SLAVE_EVENT_STOP);

	if (i2c_debug > 2)
		dev_dbg(&i2c->adap.dev, "I2C_ISR: SSD (Slave Stop) acked\n");

	/*
	 * If we have a master-mode message waiting,
	 * kick it off now that the slave has completed.
	 */
	if (i2c->msg)
		snx_i2c_master_complete(i2c, I2C_RETRY);
}
#endif
#else

static void snx_i2c_slave_txempty(struct snx_i2c *i2c, u32 isr)
{
	if (isr & ISR_BED) {
		/* what should we do here? */
	} else {
	    	/*
	     	* pull SDA low, response ACK
	     	*/
		i2c_out(i2c, I2C_IDBR_OFFSET, 0);

		i2c->icr = i2c_in(i2c, I2C_ICR_OFFSET);
		i2c->icr |= ICR_TB;
		i2c_out(i2c, I2C_ICR_OFFSET, i2c->icr);
	}
}

static void snx_i2c_slave_rxfull(struct snx_i2c *i2c, u32 isr)
{
	i2c->icr = i2c_in(i2c, I2C_ICR_OFFSET);
	i2c->icr |= ICR_TB | ICR_ACKNAK;
	i2c_out(i2c, I2C_ICR_OFFSET, i2c->icr);
}

static void snx_i2c_slave_start(struct snx_i2c *i2c, u32 isr)
{
	int timeout;

	/*
	 * slave could interrupt in the middle of us generating a
	 * start condition... if this happens, we'd better back off
	 * and stop holding the poor thing up
	 */
	i2c->icr = i2c_in(i2c, I2C_ICR_OFFSET);
	i2c->icr &= ~(ICR_START|ICR_STOP);
	i2c->icr |= ICR_TB | ICR_ACKNAK;
	i2c_out(i2c, I2C_ICR_OFFSET, i2c->icr);

	timeout = 0x10000;

	while (1) {
		if ((i2c_in(i2c, I2C_IBMR_OFFSET) & 2) == 2)
			break;

		timeout--;

		if (timeout <= 0) {
			dev_err(&i2c->adap.dev, "timeout waiting for SCL high\n");
			break;
		}
	}

	i2c->icr = i2c_in(i2c, I2C_ICR_OFFSET);
	i2c->icr &= ~ICR_SCLE;
	i2c_out(i2c, I2C_ICR_OFFSET, i2c->icr);
}

#if 0
static void snx_i2c_slave_stop(struct snx_i2c *i2c)
{
	if (i2c->msg)
		snx_i2c_master_complete(i2c, I2C_RETRY);
}
#endif
#endif

/*
 * SNX I2C Master mode
 */

/* fn static inline unsigned int snx_i2c_addr_byte(struct i2c_msg *msg)
 * brief sn92b_i2c_addr_byte - set slave addr and transfer direction
 *
 * param msg: a pointer to struct i2c_msg
 *
 * return slave address
 */

static inline unsigned int snx_i2c_addr_byte(struct i2c_msg *msg)
{
	//unsigned int addr = (msg->addr & 0x7f) << 1;
	unsigned int addr = msg->addr;

	if (msg->flags & I2C_M_RD)
		addr |= 1;

	return addr;
}

/* fn static inline void snx_i2c_start_message(struct snx_i2c *i2c)
 * brief snx_i2c_start_message - start transfer
 *
 * param i2c: a pointer to struct snx_i2c
 *
 * return NON
 */
static inline void snx_i2c_start_message(struct snx_i2c *i2c)
{
	/*
	 * Step 1: target slave address into IDBR
	 */
	i2c_out(i2c, I2C_IDBR_OFFSET, snx_i2c_addr_byte(i2c->msg));

	/*
	 * Step 2: initiate the write.
	 */
	i2c->icr = i2c_in(i2c, I2C_ICR_OFFSET);
	i2c->icr &= ~(ICR_STOP | ICR_ALDIE);
	i2c->icr |= ICR_START | ICR_TB;
	i2c_out(i2c, I2C_ICR_OFFSET, i2c->icr);
}

/*
 * We are protected by the adapter bus mutex.
 */

/* fn static int snx_i2c_do_xfer(struct snx_i2c *i2c, struct i2c_msg *msg, int num)
 * brief snx_i2c_do_xfer - really i2c transfer function
 *
 * param i2c: a pointer to struct snx_i2c
 * param msg: a pointer to struct i2c_msg
 * param num: count of msg
 *
 * return zero: success  nonzero: fail
 */

static int snx_i2c_do_xfer(struct snx_i2c *i2c, struct i2c_msg *msg, int num)
{
	long timeout;
	int ret;

	/*
	 * Wait for the bus to become free.
	 */
	ret = snx_i2c_wait_bus_busy(i2c);
	if (ret) {
//		if(i2c_debug)
			dev_err(&i2c->adap.dev, "snx_i2c: timeout waiting for bus free\n");

		goto out;
	}

	/*
	 * Set master mode.
	 */
	ret = snx_i2c_set_master(i2c);
	if (ret) {
		if(i2c_debug)
			dev_err(&i2c->adap.dev, "snx_i2c_set_master: error %d\n", ret);
		printk("snx_i2c_set_master: error %d\n", ret);
		goto out;
	}

	spin_lock_irq(&i2c->lock);

	i2c->msg = msg;
	i2c->msg_num = num;
	i2c->msg_idx = 0;
	i2c->msg_ptr = 0;
	i2c->irqlogidx = 0;

	snx_i2c_start_message(i2c);

	spin_unlock_irq(&i2c->lock);

	/*
	 * The rest of the processing occurs in the interrupt handler.
	 */


	timeout = wait_event_timeout(i2c->wait, i2c->msg_num == 0, HZ * 1);

	/*
	 * We place the return code in i2c->msg_idx.
	 */
	ret = i2c->msg_idx;

	if (timeout == 0)
	{
		printk("i2c timeout\n");
	  	snx_i2c_scream_blue_murder(i2c, "timeout");
	}

out:
	return ret;
}


/* fn static void snx_i2c_irq_txempty(struct snx_i2c *i2c, u32 isr)
 * brief snx_i2c_irq_txempty - i2c send function
 *
 * param i2c: a pointer to struct snx_i2c
 * param isr: i2c isr register value
 *
 * return NON
 */

static void snx_i2c_irq_txempty(struct snx_i2c *i2c, u32 isr)
{
	i2c->icr = i2c_in(i2c, I2C_ICR_OFFSET)
		& ~(ICR_START | ICR_STOP | ICR_ACKNAK | ICR_TB);

again:
	/*
	 * If ISR_ALD is set, we lost arbitration.
	 */
	if (isr & ISR_ALD) {
		/*
		 * Do we need to do anything here?  The SNX docs
		 * are vague about what happens.
		 */
		snx_i2c_scream_blue_murder(i2c, "ALD set");

		/*
		 * We ignore this error.  We seem to see spurious ALDs
		 * for seemingly no reason.  If we handle them as I think
		 * they should, we end up causing an I2C error, which
		 * is painful for some systems.
		 */
		return; /* ignore */
	}

	if (isr & ISR_BED) {
		int ret = BUS_ERROR;

		/*
		 * I2C bus error - either the device NAK'd us, or
		 * something more serious happened.  If we were NAK'd
		 * on the initial address phase, we can retry.
		 */
		if (isr & ISR_ACKNAK) {
			if (i2c->msg_ptr == 0 && i2c->msg_idx == 0)
				ret = I2C_RETRY;
			else
				ret = XFER_NAKED;
		}

		snx_i2c_master_complete(i2c, ret);

	} else if (isr & ISR_RW) {
		/*
		 * receive mode.  We have just sent the address byte, and
		 * now we must initiate the transfer.
		 */
		if (i2c->msg_ptr == i2c->msg->len - 1
			&& i2c->msg_idx == i2c->msg_num - 1)
			i2c->icr |= ICR_STOP | ICR_ACKNAK;

		i2c->icr |= ICR_ALDIE | ICR_TB;

	} else if (i2c->msg_ptr < i2c->msg->len) {
		/*
		 * transmit mode.  Write the next data byte.
		 */
		i2c_out(i2c, I2C_IDBR_OFFSET, i2c->msg->buf[i2c->msg_ptr++]);

		i2c->icr |= ICR_ALDIE | ICR_TB;

		/*
		 * If this is the last byte of the last message, send
		 * a STOP.
		 */
		if (i2c->msg_ptr == i2c->msg->len
			&& i2c->msg_idx == i2c->msg_num - 1)
			i2c->icr |= ICR_STOP;

	} else if (i2c->msg_idx < i2c->msg_num - 1) {
		/*
		 * Next segment of the message.
		 */
		i2c->msg_ptr = 0;
		i2c->msg_idx ++;
		i2c->msg++;

		/*
		 * If we aren't doing a repeated start and address,
		 * go back and try to send the next byte.  Note that
		 * we do not support switching the R/W direction here.
		 */
		if (i2c->msg->flags & I2C_M_NOSTART)
			goto again;

		/*
		 * Write the next address.
		 */
		i2c_out(i2c, I2C_IDBR_OFFSET, snx_i2c_addr_byte(i2c->msg));

		/*
		 * And trigger a repeated start, and send the byte.
		 */
		i2c->icr &= ~ICR_ALDIE;
		i2c->icr |= ICR_START | ICR_TB;

	} else {
		if (i2c->msg->len == 0) {
			/*
			 * Device probes have a message length of zero
			 * and need the bus to be reset before it can
			 * be used again.
			 */
			snx_i2c_reset(i2c);
		}

		snx_i2c_master_complete(i2c, 0);
	}

	i2c->icrlog[i2c->irqlogidx-1] = i2c->icr;

	i2c_out(i2c, I2C_ICR_OFFSET, i2c->icr);

	show_state(i2c);
}

/* fn static void snx_i2c_irq_rxfull(struct snx_i2c *i2c, u32 isr)
 * brief snx_i2c_irq_rxfull - i2c receive function
 *
 * param i2c: a pointer to struct snx_i2c
 * param isr: isr register value
 *
 * return NON
 */

static void snx_i2c_irq_rxfull(struct snx_i2c *i2c, u32 isr)
{
	i2c->icr = i2c_in(i2c, I2C_ICR_OFFSET)
		& ~(ICR_START|ICR_STOP|ICR_ACKNAK|ICR_TB);

	/*
	 * Read the byte.
	 */
	i2c->msg->buf[i2c->msg_ptr++] = i2c_in(i2c, I2C_IDBR_OFFSET);

	if (i2c->msg_ptr < i2c->msg->len) {
		/*
		 * If this is the last byte of the last
		 * message, send a STOP.
		 */
		if (i2c->msg_ptr == i2c->msg->len - 1)
			i2c->icr |= ICR_STOP | ICR_ACKNAK;

		i2c->icr |= ICR_ALDIE | ICR_TB;
	} else {
		snx_i2c_master_complete(i2c, 0);
	}

	i2c->icrlog[i2c->irqlogidx-1] = i2c->icr;

	i2c_out(i2c, I2C_ICR_OFFSET, i2c->icr);
}

/* fn static irqreturn_t snx_i2c_irq(int this_irq, void *dev_id)
 * brief snx_i2c_irq - snx i2c interrupt handler
 *
 * param this_irq: interrupt nr
 * param dev_id: address of snx_i2c
 *
 * return IRQ_HANDLED: success
 */

static irqreturn_t snx_i2c_irq(int this_irq, void *dev_id)
{
	struct snx_i2c *i2c = dev_id;
	u32 isr = i2c_in(i2c, I2C_ISR_OFFSET);

	if (i2c_debug > 2) {
		dev_dbg(&i2c->adap.dev, "%s: I2C_ISR=%08x, I2C_ICR=%08x, I2C_IBMR=%02x\n",
			__func__, isr,
			i2c_in(i2c, I2C_ICR_OFFSET),
			i2c_in(i2c, I2C_IBMR_OFFSET));

		decode_ISR(isr);
	}

	if (i2c->irqlogidx < ARRAY_SIZE(i2c->isrlog))
		i2c->isrlog[i2c->irqlogidx++] = isr;

	show_state(i2c);

	/*
	 * Clear all pending IRQs when read isr.
	 */

    	/*
     	* Detect slave address match
     	*/
	if (isr & ISR_SAM)
		snx_i2c_slave_start(i2c, isr);
#if 0
    /*
     * Detect stop condition
     */
	if (isr & ISR_STOP) {
		snx_i2c_slave_stop(i2c);
	}
#endif

	if (snx_i2c_is_slavemode(i2c)) {
		/*
		 * slave mode
		 */
		if (isr & ISR_DT)
			snx_i2c_slave_txempty(i2c, isr);

		if (isr & ISR_DR)
			snx_i2c_slave_rxfull(i2c, isr);

	} else if (i2c->msg) {
		/*
		 * master mode
		 */
		if (isr & ISR_DT)
			/*
			 * The data register has transmitted one
			 * data byte onto the I2C bus.
			 */
			snx_i2c_irq_txempty(i2c, isr);

		if (isr & ISR_DR)
			/*
			 * The data register has received one
			 * data byte onto the I2C bus.
			 */
			snx_i2c_irq_rxfull(i2c, isr);

	} else {
		snx_i2c_scream_blue_murder(i2c, "spurious irq");
	}
	snx_i2c_register_dump(i2c);

	return IRQ_HANDLED;
}

/* fn static int snx_i2c_xfer(struct i2c_adapter *adap, struct i2c_msg msg[], int num)
 * brief snx_i2c_xfer - i2c_algorithm transfer function
 *
 * param adap: a pointer to i2c_adapter
 * param msg: transfer data which wrapped into i2c_msg struct
 * param num: the element count of msg
 *
 * return zero: seccess  nonzero: fail
 */

static int snx_i2c_xfer(struct i2c_adapter *adap, struct i2c_msg msgs[], int num)
{
	struct snx_i2c *i2c = adap->algo_data;
	int ret, i;
	static struct i2c_adapter mcu_adap;
	static struct i2c_msg mcu_msgs;
	static int mcu_num = 0;
	static int mcu_write = 0;

	/* If the I2C controller is disabled we need to reset it (probably due
 	 * to a suspend/resume destroying state). We do this here as we can then
 	 * avoid worrying about resuming the controller before its users.
 	 */
	if (!(i2c_in(i2c, I2C_ICR_OFFSET) & ICR_I2CE))
		snx_i2c_reset(i2c);

	// if MCU and Sensor are shared I2C-1
	if ((snx_i2c_addr_byte(msgs) == 0xA8) && (adap->nr == 0))
	{
		memcpy(&mcu_adap, adap, sizeof(struct i2c_adapter));
		memcpy(&mcu_msgs, msgs, sizeof(struct i2c_msg));
		mcu_num = num;
//		adap->retries = 3;	// MCU retry 3 times
		if (!mcu_write)
		{
			mcu_write = 1;
			return 0;
		}
//		printk(KERN_DEBUG "[I2C] mcu_write failed!\n");
	}
//	else
//		adap->retries = 0;	// Sensor don't retry

	if (mcu_write)
	{
		struct snx_i2c *mcu_i2c = mcu_adap.algo_data;

		if (i2c->mapbase == mcu_i2c->mapbase)
		{
			for (i = 0; i < adap->retries; i++) {
				ret = snx_i2c_do_xfer(mcu_i2c, &mcu_msgs, mcu_num);
				if (ret != I2C_RETRY)
				{
					if (snx_i2c_addr_byte(msgs) != 0xA8)
						mcu_write = 0;	// write OK
					break;
				}

				if (i2c_debug)
					dev_dbg(&adap->dev, "mcu Retrying %d\n", i);

				udelay(100);
			}
		}
	}

	for (i = 0; i < adap->retries; i++) {
		ret = snx_i2c_do_xfer(i2c, msgs, num);
		if (ret != I2C_RETRY)
			goto out;

		if (i2c_debug)
			dev_dbg(&adap->dev, "Retrying transmission\n");

		udelay(100);
	}

	snx_i2c_scream_blue_murder(i2c, "exhausted retries");
	snx_i2c_reset(i2c);
	ret = -EREMOTEIO;
	return ret;

out:	// success
#ifdef CONFIG_I2C_SNX_SLAVE
	snx_i2c_set_slave(i2c, ret);
#endif

	return ret;
}

/* fn static u32 snx_i2c_functionality(struct i2c_adapter *adap)
 * brief snx_i2c_functionality - return the i2c_adapter functionalities
 *
 * param adap: a pointer to i2c_adapter
 *
 * return the i2c_adapter functionalities
 */

static u32 snx_i2c_functionality(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL;
}

static struct i2c_algorithm snx_i2c_algorithm = {
	.master_xfer	= snx_i2c_xfer,
	.functionality	= snx_i2c_functionality,
};


#if SUPPORT_I2C_GPIO
void inline i2c_set_gpio_mode(unsigned int BASE_I2C)
{
	u32 v;

	v = __raw_readl(BASE_I2C + I2C_GPIO_OFFSET);
	v |= I2C_GPIO_mode;
	__raw_writel(v,BASE_I2C + I2C_GPIO_OFFSET);
}

void i2c_gpio_write(int pin, int mode, int value)
{
	u32 v;
	unsigned int BASE_I2C;
	unsigned char *pMembase;

	if (pin <= 1 )
		pMembase = (void *) io_p2v(SNX_I2C0_BASE);
	else
		pMembase = (void *) io_p2v(SNX_I2C1_BASE);

	BASE_I2C = (unsigned int)pMembase;

	spin_lock_irq(&i2c_lock);
	// set i2c into GPIO mode
	i2c_set_gpio_mode(BASE_I2C);

	v = __raw_readl(BASE_I2C + I2C_GPIO_OFFSET);
	switch (pin) {
		case GPIO_SCL0_PIN:
		case GPIO_SCL1_PIN:
			if (mode == 1)
			{
				v |= (I2C_SCL_OUTPUT_EN);
				v &= ~(0x1 << 4);
				v |= value << 4;
				  break;
			}
			else
			{
				v &= ~(I2C_SCL_OUTPUT_EN);
				v &= ~(0x1 << 4);
			}
			break;
		case GPIO_SDA0_PIN:
		case GPIO_SDA1_PIN:
			if (mode == 1)
			{
				v |= (I2C_SDA_OUTPUT_EN);
				v &= ~(0x1 << 5);
				v |= value << 5;
			}
			else
			{
				v &= ~(I2C_SDA_OUTPUT_EN);
				v &= ~(0x1 << 5);
			}
			break;
		default:
			printk ("unknown pin\n");
			break;
	}

	__raw_writel(v, BASE_I2C + I2C_GPIO_OFFSET);
	spin_unlock_irq(&i2c_lock);
}

void i2c_gpio_read(int pin,int *mode, int *value)
{
	u32 v = 0;
	unsigned int BASE_I2C;
	unsigned char *pMembase;

	if (pin <= 1 )
		pMembase = (void *) io_p2v(SNX_I2C0_BASE);
	else
		pMembase = (void *) io_p2v(SNX_I2C1_BASE);

	BASE_I2C = (unsigned int)pMembase;

	v = __raw_readl (BASE_I2C + I2C_GPIO_OFFSET);

	switch (pin) {
		case GPIO_SCL0_PIN:
		case GPIO_SCL1_PIN:
			*mode = (v>>2)&0x1;

			if (*mode == 1)
				*value =  (v >> 4) & 0x1;
			else
				*value = (v >> 6) & 0x1;

			break;
		case GPIO_SDA0_PIN:
		case GPIO_SDA1_PIN:
			*mode = (v>>3)&0x1;

			if (*mode == 1)
				*value =  (v >> 5) & 0x1;
			else
				*value = (v >> 7) & 0x1;

			break;
		default:
			printk ("unknown pin\n");
			break;
	}
}

void i2c_io_release(void)
{
	unsigned char *pMembase1;
	unsigned char *pMembase2;

	pMembase1 = (void *) io_p2v(SNX_I2C0_BASE);
	pMembase2 = (void *) io_p2v(SNX_I2C1_BASE);

	spin_lock_irq(&i2c_lock);
	*((volatile unsigned *)(((unsigned int)pMembase1) + I2C_GPIO_OFFSET)) = 0;
	*((volatile unsigned *)(((unsigned int)pMembase2) + I2C_GPIO_OFFSET)) = 0;
	spin_unlock_irq(&i2c_lock);
}

static int snx_i2cgpio_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg);
static int snx_i2cgpio_open(struct inode *inode, struct file *file);
static int snx_i2cgpio_release(struct inode *inode, struct file *file);

 static struct file_operations snx_i2cgpio_fops=
{
  	.owner		= THIS_MODULE,
  	.ioctl		= snx_i2cgpio_ioctl,
  	.open		= snx_i2cgpio_open,
  	.release	= snx_i2cgpio_release,
};

static struct miscdevice snx_i2cgpio_miscdev=
{
	.minor	= MISC_DYNAMIC_MINOR,
	.name		= "i2cgpio",
	.fops		= &snx_i2cgpio_fops,
};

struct gpio_pin_info{
	unsigned int  pinumber; //pin number
	unsigned int  mode;		 //0:input 1:output
	unsigned int  value; 	//0:low 1:high
};

static int snx_i2cgpio_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	struct gpio_pin_info info;

	if (copy_from_user (&info, (void __user *)arg, sizeof(info)))
			return 1;

	switch(cmd){
		case I2C_GPIO_WRITE:
			i2c_gpio_write (info.pinumber ,info.mode ,info.value);
			break;
		case I2C_GPIO_READ:
			i2c_gpio_read(info.pinumber,&info.mode,&info.value);

			if (copy_to_user ((void __user *)arg, &info, sizeof(info)))
			  return 1;

			break;
		 default:
			return 1;
	}

   	return 0;
}

/** \fn static int snx_i2cgpio_open(struct inode *inode, struct file *filp)
 * \brief Open device
 */
static int snx_i2cgpio_open(struct inode *inode, struct file *file)
{
	//need not switch pin mux
	//i2c_set_gpio_mode(SNX_I2C1_BASE);
	return 0;
}

/** \fn static int snx_i2cgpio_release(struct inode *inode, struct file *filp)
 * \brief Close device.
 */
static int snx_i2cgpio_release(struct inode *inode, struct file *file)
{
	i2c_io_release();
	return 0;
}
#endif


#ifdef CONFIG_PM
static int snx_i2c_suspend(struct platform_device *pdev, pm_message_t mesg)
{
	struct snx_i2c *i2c = platform_get_drvdata(pdev);

	clk_disable(i2c->clk);
	return 0;
}

static int snx_i2c_resume(struct platform_device *pdev)
{
	struct snx_i2c *i2c = platform_get_drvdata(pdev);

	return clk_enable(i2c->clk);
}
#else
#define snx_i2c_suspend NULL
#define snx_i2c_resume NULL
#endif


/* fn static int __devexit snx_i2c_remove(struct platform_device *pdev)
 * brief snx_i2c_remove - i2c controller remove function
 *
 * param pdev: address of platform device
 *
 * return zero: success  nonzero: fail
 */

static int __devexit snx_i2c_remove(struct platform_device *pdev)
{
	struct snx_i2c *i2c = platform_get_drvdata(pdev);

	snx_i2c_reset(i2c);

	if (i2c != NULL) {
		snx_i2cs[pdev->id] = NULL;
		platform_set_drvdata(pdev, NULL);
		i2c_del_adapter(&i2c->adap);

		if (i2c->clk != NULL) {
			clk_disable(i2c->clk);
			clk_put(i2c->clk);
		}

		free_irq(i2c->irq, i2c);

#if 0
		if (i2c->membase != NULL)
			iounmap(i2c->membase);

		if (i2c->res == NULL) {
			release_resource(i2c->res);
			kfree(i2c->res);
		}
#endif

		kfree(i2c);
	}

	return 0;
}

/* fn static int __devinit snx_i2c_probe(struct platform_device *pdev)
 * brief snx_i2c_probe - i2c controller probe function
 *
 * param pdev: address of platform device
 *
 * return zero: success  nonzero: fail
 */

static int __devinit snx_i2c_probe(struct platform_device *pdev)
{
	struct snx_i2c *i2c;
	struct resource *res;
	int ret;

#ifdef CONFIG_I2C_SNX_SLAVE
	struct snx_i2c_platform_data *pdata = pdev->dev.platform_data;
	i2c->slave_addr = I2C_SNX_SLAVE_ADDR;
	i2c->slave = &eeprom_client;
	if (pdata) {
		i2c->slave_addr = pdata->slave_addr;
		if (pdata->slave)
			i2c->slave = pdata->slave;
	}
#endif

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		ret = -EINVAL;
		goto out;
	}

	i2c = kzalloc(sizeof(struct snx_i2c), GFP_KERNEL);
	if (i2c == NULL) {
		ret = -ENOMEM;
		goto err;
	}

#if 0
	/* already done in the platform_device_register */
	i2c->res = request_mem_region(res->start, PAGE_SIZE, pdev->dev.bus_id);
	if (i2c->res == NULL) {
		ret = -EBUSY;
		goto err;
	}
#endif

	i2c->mapbase = res->start;
	i2c->membase = (void *) io_p2v(i2c->mapbase);

#if 0
	i2c->membase = ioremap(i2c->mapbase, PAGE_SIZE);
	if (i2c->membase == NULL) {
		ret = -ENOMEM;
		goto err;
	}
#endif

	i2c->irq = platform_get_irq(pdev, 0);

	ret = request_irq(i2c->irq, snx_i2c_irq, IRQF_DISABLED,
			  dev_name(&pdev->dev), i2c);
	if (ret)
		goto err;

	i2c->adap.owner      = THIS_MODULE;
	i2c->adap.algo       = &snx_i2c_algorithm;
	i2c->adap.algo_data  = i2c;
	i2c->adap.retries    = 3;
	i2c->adap.dev.parent = &pdev->dev;

	strlcpy(i2c->adap.name, pdev->name, I2C_NAME_SIZE);

	i2c->lock	= SPIN_LOCK_UNLOCKED;
	init_waitqueue_head(&i2c->wait);

	if (pdev->id == 0)
		i2c->clk = clk_get(&pdev->dev, "i2c0_clk");
	if (pdev->id == 1)
		i2c->clk = clk_get(&pdev->dev, "i2c1_clk");
	if (i2c->clk == NULL) {
		ret = PTR_ERR(i2c->clk);
		goto err;
	}
	clk_enable(i2c->clk);

	snx_i2c_reset(i2c);

	i2c->adap.nr = pdev->id;

	ret = i2c_add_numbered_adapter(&i2c->adap);
	if (!ret) {
		platform_set_drvdata(pdev, i2c);

		snx_i2cs[pdev->id] = i2c;

#ifdef CONFIG_I2C_SNX_SLAVE
		printk(KERN_INFO "%s: SNX I2C%d controller, slave address %d at 0x%lx (irq = %d)\n",
		   dev_name(&pdev->dev), pdev->id, i2c->slave_addr, i2c->mapbase, i2c->irq);
#else
		printk(KERN_INFO "%s: SNX I2C%d controller at 0x%lx (irq = %d)\n",
		   dev_name(&pdev->dev), pdev->id, i2c->mapbase, i2c->irq);
#endif
#if SUPPORT_I2C_GPIO
		if (pdev->id == 0){
			ret = misc_register(&snx_i2cgpio_miscdev);
			if (ret)
				goto err;
			printk ("I2C GPIO driver INIT\n");
		}
#endif
		goto out;
	}

err:
	printk(KERN_INFO "I2C: Failed to add bus\n");
	snx_i2c_remove(pdev);
out:
	return ret;
}

static struct platform_driver snx_i2c_driver = {
	.probe		= snx_i2c_probe,
	.remove		= __devexit_p(snx_i2c_remove),
	.suspend	= snx_i2c_suspend,
	.resume		= snx_i2c_resume,
	.driver		= {
		.name	= "snx_i2c",
		.owner	= THIS_MODULE,
	},
};

/* fn static int __init snx_i2c_init(void)
 * brief This function is called when the module is installed to kernel
 * return zero: success  nonzero: fail
 */

static int __init snx_i2c_init(void)
{
	printk(KERN_INFO "SONIX SNX I2C adapter driver, (c) 2012 Sonix\n");
	return platform_driver_register(&snx_i2c_driver);
}

/* fn static void __eixt snx_i2c_exit(void)
 * brief This function is called when the module is removed from kernel
 */

static void __exit snx_i2c_exit(void)
{
	platform_driver_unregister(&snx_i2c_driver);
}

module_init(snx_i2c_init);
module_exit(snx_i2c_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kelvin Cheung");
MODULE_DESCRIPTION("SONIX SNX I2C adapter driver");
