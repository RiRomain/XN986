/*
 * Create at 2011/05/06 by yanjie_yang
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/io.h>

#include <mach/clock.h>
#include <mach/regs-sys.h>

unsigned long snx_apb_clk_rate;

static LIST_HEAD(clocks);
static DEFINE_MUTEX(clocks_mutex);
static DEFINE_SPINLOCK(clocks_lock);

struct clk *clk_get(struct device *dev, const char *id)
{
	struct clk *p, *clk = ERR_PTR(-ENOENT);

	mutex_lock(&clocks_mutex);
	list_for_each_entry(p, &clocks, node) {
		if (strcmp(id, p->name) == 0 && try_module_get(p->owner)) {
			clk = p;
			break;
		}
	}
	mutex_unlock(&clocks_mutex);

	return clk;
}
EXPORT_SYMBOL(clk_get);

void clk_put(struct clk *clk)
{
	module_put(clk->owner);
}
EXPORT_SYMBOL(clk_put);

int clk_enable(struct clk *clk)
{
	unsigned long flags;

	spin_lock_irqsave(&clocks_lock, flags);
	if (clk->enabled++ == 0)
		(clk->enable)(clk);
	spin_unlock_irqrestore(&clocks_lock, flags);

	return 0;
}
EXPORT_SYMBOL(clk_enable);

void clk_disable(struct clk *clk)
{
	unsigned long flags;

	WARN_ON(clk->enabled == 0);

	spin_lock_irqsave(&clocks_lock, flags);
	if (--clk->enabled == 0)
		(clk->disable)(clk);
	spin_unlock_irqrestore(&clocks_lock, flags);
}
EXPORT_SYMBOL(clk_disable);

unsigned long clk_get_rate(struct clk *clk)
{
	if(clk->parent)
	{
		unsigned long parent_rate, rate;
		unsigned int ratio;

		parent_rate = clk_get_rate(clk->parent);
		if(clk->ratio == 0)
		{
			if(clk->get_ratio != NULL)
				ratio = (clk->get_ratio)(clk);
			else
			{
				printk(KERN_ERR "the ratio of '%s' clock is error.\n", clk->name);
				return 0;
			}
		}
		else
			ratio = clk->ratio;
		
		if(clk->rate_raise)
			rate = parent_rate * ratio;
		else
			rate = parent_rate / ratio;

		return rate;
	}
	else if(clk->rate)
		return clk->rate;
	else		// (clk->rate == 0) && (clk->parent == NULL)
	{
		printk(KERN_ERR "'%s' Clock setting is error.\n", clk->name);
		return 0;
	}
}
EXPORT_SYMBOL(clk_get_rate);

int clk_register(struct clk *clk)
{
	mutex_lock(&clocks_mutex);
	list_add(&clk->node, &clocks);
	mutex_unlock(&clocks_mutex);

	return 0;
}
EXPORT_SYMBOL(clk_register);

void clk_unregister(struct clk *clk)
{
	mutex_lock(&clocks_mutex);
	list_del(&clk->node);
	mutex_unlock(&clocks_mutex);
}
EXPORT_SYMBOL(clk_unregister);

/*
 * Routine to safely enable a clock in the PMU
 */
static void snx_clk_enable (struct clk *clk)
{
	unsigned long val, flags;

	local_irq_save(flags);
	val = readl(clk->gate_addr);
	val &= ~clk->gate;
	writel(val, clk->gate_addr);
	local_irq_restore(flags);
}

/*
 * Routine to safely disable a clock in the PMU
 */
static void snx_clk_disable (struct clk *clk)
{
	unsigned long val, flags;

	local_irq_save(flags);
	val = readl(clk->gate_addr);
	val |= clk->gate;
	writel(val, clk->gate_addr);
	local_irq_restore(flags);
}

/*
 * Routine to get ratio between parent clock and clock.
 */
static unsigned int snx_clk_get_ratio (struct clk *clk)
{
	unsigned long flags;
	unsigned int val;

	local_irq_save(flags);
	val = readl(clk->ratio_addr);
	val = (val & clk->ratio_mask) >> clk->ratio_start_bit;
	local_irq_restore(flags);

	return val;
}

inline
static unsigned int snx_clk_get_ratio_2to16 (struct clk *clk)
{
	unsigned int val;

	val = snx_clk_get_ratio(clk);
	if((val >= 2) && (val <= 16))
		return val;
	else
		return 16;
}

inline
static unsigned int snx_clk_get_ratio_1to16 (struct clk *clk)
{
	unsigned int val;

	val = snx_clk_get_ratio(clk);
	if((val >= 1) && (val <= 16))
		return val;
	else
		return 16;
}

inline
static unsigned int snx_clk_get_ratio_1to8 (struct clk *clk)
{
	unsigned int val;

	val = snx_clk_get_ratio(clk);
	if((val >= 1) && (val <= 8))
		return val;
	else
		return 8;
}

inline
static unsigned int snx_clk_get_ratio_8to32 (struct clk *clk)
{
	unsigned int val;

	val = snx_clk_get_ratio(clk);
	if((val >= 8) && (val <= 32))
		return val;
	else
		return 32;
}

inline
static unsigned int snx_clk_get_ratio_4to16 (struct clk *clk)
{
	unsigned int val;

	val = snx_clk_get_ratio(clk);
	if((val >= 4) && (val <= 16))
		return val;
	else
		return 16;
}


/* Primary clocks */
static struct clk snx_pll_12m_clk = {
	.name = "pll_12m_clk",
	.parent = NULL,
	.rate = 12 * 1000 * 1000,
};

static struct clk snx_pll_800m_clk = {
	.name = "pll_800m_clk",
	.parent = &snx_pll_12m_clk,
	.rate_raise = 1,
	.ratio = 0,
	.ratio_addr = SYS_MISC,
	.ratio_start_bit = MISC_PLL800_DIV_BIT,
	.ratio_mask = MISC_PLL800_DIV_MASK,
	.get_ratio = snx_clk_get_ratio,
};

static struct clk snx_pll_300m_clk = {
	.name = "pll_300m_clk",
	.parent = &snx_pll_12m_clk,
	.rate_raise = 1,
	.ratio = 0,
	.ratio_addr = SYS_MISC,
	.ratio_start_bit = MISC_PLL300_DIV_BIT,
	.ratio_mask = MISC_PLL300_DIV_MASK,
	.get_ratio = snx_clk_get_ratio,
	.gate = BIT(MISC_PLL300_EN_BIT),
	.gate_addr = SYS_MISC,
	.enable = snx_clk_enable,
	.disable = snx_clk_disable,
};

static struct clk snx_pll_100m_clk = {
	.name = "pll_100m_clk",
	.parent = NULL,
	.rate = 100 * 1000 * 1000,
	.gate = BIT(MISC_PLL100_EN_BIT),
	.gate_addr = SYS_MISC,
	.enable = snx_clk_enable,
	.disable = snx_clk_disable,
};

static struct clk snx_pll_480m_clk = {
	.name = "pll_480m_clk",
	.parent = NULL,
	.rate = 480 * 1000 * 1000,
	.gate = BIT(MISC_PLL480_EN_BIT),
	.gate_addr = SYS_MISC,
	.enable = snx_clk_enable,
	.disable = snx_clk_disable,
};

static struct clk snx_ddr_clk = {
	.name = "ddr_clk",
	.parent = &snx_pll_800m_clk,
	.ratio = 0,
	.ratio_addr = SYS_CLKSRC,
	.ratio_start_bit = CLKSRC_DDR_BIT,
	.ratio_mask = CLKSRC_DDR_MASK,
	.get_ratio = snx_clk_get_ratio_2to16,
	.gate = CLKGATE_DDRC,
	.gate_addr = SYS_CLKGATE,
	.enable = snx_clk_enable,
	.disable = snx_clk_disable,
};

static struct clk snx_cpu_clk = {
	.name = "cpu_clk",
	.parent = &snx_pll_800m_clk,
	.ratio = 0,
	.ratio_addr = SYS_CLKSRC,
	.ratio_start_bit = CLKSRC_CPU_BIT,
	.ratio_mask = CLKSRC_CPU_MASK,
	.get_ratio = snx_clk_get_ratio_2to16,
};

static struct clk snx_ahb_clk = {
	.name = "ahb_clk",
	.parent = &snx_cpu_clk,
	.ratio = 0,
	.ratio_addr = SYS_CLKSRC,
	.ratio_start_bit = CLKSRC_AHB_BIT,
	.ratio_mask = CLKSRC_AHB_MASK,
	.get_ratio = snx_clk_get_ratio_1to16,
};

static struct clk snx_apb_clk = {
	.name = "apb_clk",
	.parent = &snx_ahb_clk,
	.ratio = 0,
	.ratio_addr = SYS_CLKSRC,
	.ratio_start_bit = CLKSRC_APB_BIT,
	.ratio_mask = CLKSRC_APB_MASK,
	.get_ratio = snx_clk_get_ratio_1to8,
};

static struct clk snx_ahb2_clk = {
	.name = "ahb2_clk",
	.parent = &snx_pll_800m_clk,
	.ratio = 0,
	.ratio_addr = SYS_CLKSRC,
	.ratio_start_bit = CLKSRC_AHB2_BIT,
	.ratio_mask = CLKSRC_AHB2_MASK,
	.get_ratio = snx_clk_get_ratio_8to32,
};

static struct clk snx_isp_clk = {
	.name = "isp_clk",
	.parent = &snx_pll_800m_clk,
	.ratio = 0,
	.ratio_addr = SYS_CLKSRC,
	.ratio_start_bit = CLKSRC_ISP_BIT,
	.ratio_mask = CLKSRC_ISP_MASK,
	.get_ratio = snx_clk_get_ratio_4to16,
	.gate = CLKGATE_SENSOR,
	.gate_addr = SYS_CLKGATE,
	.enable = snx_clk_enable,
	.disable = snx_clk_disable,
};

static struct clk snx_lcdtv_clk = {
	.name = "lcdtv_clk",
	.parent = &snx_pll_800m_clk,
	.ratio = 0,
	.ratio_addr = SYS_CLKSRC,
	.ratio_start_bit = CLKSRC_LCD_BIT,
	.ratio_mask = CLKSRC_LCD_MASK,
	.get_ratio = snx_clk_get_ratio_4to16,
	.gate = CLKGATE_LCD,
	.gate_addr = SYS_CLKGATE,
	.enable = snx_clk_enable,
	.disable = snx_clk_disable,
};

static struct clk snx_jpgdec_clk = {
	.name = "jpgdec_clk",
	.parent = &snx_pll_800m_clk,
	.ratio = 0,
	.ratio_addr = SYS_CLKSRC,
	.ratio_start_bit = CLKSRC_JPGDEC_BIT,
	.ratio_mask = CLKSRC_JPGDEC_MASK,
	.get_ratio = snx_clk_get_ratio_4to16,
	.gate = CLKGATE1_JPGDEC,
	.gate_addr = SYS_CLKGATE1,
	.enable = snx_clk_enable,
	.disable = snx_clk_disable,
};

static struct clk snx_img_clk = {
	.name = "img_clk",
	.parent = &snx_pll_800m_clk,
	.ratio = 0,
	.ratio_addr = SYS_CLKSRC1,
	.ratio_start_bit = CLKSRC1_IMG_BIT,
	.ratio_mask = CLKSRC1_IMG_MASK,
	.get_ratio = snx_clk_get_ratio_4to16,
	.gate = CLKGATE1_IMG,
	.gate_addr = SYS_CLKGATE1,
	.enable = snx_clk_enable,
	.disable = snx_clk_disable,
};

static struct clk snx_pll_400m_clk = {
	.name = "pll_400m_clk",
	.parent = &snx_pll_800m_clk,
	.ratio = 2,
};

static struct clk snx_h264_clk = {
	.name = "h264_clk",
	.parent = &snx_pll_300m_clk,
	.ratio = 0,
	.ratio_addr = SYS_CLKSRC1,
	.ratio_start_bit = CLKSRC1_H264_BIT,
	.ratio_mask = CLKSRC1_H264_MASK,
	.get_ratio = snx_clk_get_ratio_1to16,
};

static struct clk *sn98600_primary_clks[] = {
	&snx_pll_12m_clk,
	&snx_pll_800m_clk,
	&snx_pll_300m_clk,
	&snx_pll_100m_clk,
	&snx_pll_480m_clk,
	&snx_ddr_clk, 
	&snx_cpu_clk, 
	&snx_ahb_clk, 
	&snx_apb_clk, 
	&snx_ahb2_clk, 
	&snx_isp_clk, 
	&snx_lcdtv_clk, 
	&snx_jpgdec_clk, 
	&snx_img_clk, 
	&snx_h264_clk,
};

/* device clocks */
//ahb
static struct clk snx_aes_clk = {
	.name = "aes_clk",
	.parent = &snx_ahb_clk,
	.ratio = 1,
	.gate = CLKGATE1_AES,
	.gate_addr = SYS_CLKGATE1,
};

static struct clk snx_dma_clk = {
	.name = "ahbdma_clk",
	.parent = &snx_ahb_clk,
	.ratio = 1,
	.gate = CLKGATE_DMAC,
	.gate_addr = SYS_CLKGATE,
};

static struct clk snx_usb_clk = {
	.name = "usb_clk",
	.parent = &snx_ahb_clk,
	.ratio = 1,
	.gate = CLKGATE1_USB,
	.gate_addr = SYS_CLKGATE1,
};

//ahb2
static struct clk snx_dlc_spi_clk = {
	.name = "dlc_spi_clk",
	.parent = &snx_ahb2_clk,
	.ratio = 1,
	.gate = CLKGATE1_DLC_SPI,
	.gate_addr = SYS_CLKGATE1,
};

static struct clk snx_audio_clk = {
	.name = "audio_clk",
	.parent = &snx_ahb2_clk,
	.ratio = 1,
	.gate = CLKGATE_AUDIO,
	.gate_addr = SYS_CLKGATE,
};

static struct clk snx_ms1_clk = {
	.name = "ms1_clk",
	.parent = &snx_ahb2_clk,
	.ratio = 1,
	.gate = CLKGATE_MS1,
	.gate_addr = SYS_CLKGATE,
};

static struct clk snx_ms2_clk = {
	.name = "ms2_clk",
	.parent = &snx_ahb2_clk,
	.ratio = 1,
	.gate = CLKGATE_MS2,
	.gate_addr = SYS_CLKGATE,
};

static struct clk snx_mac_clk = {
	.name = "mac_clk",
	.parent = &snx_ahb2_clk,
	.ratio = 1,
	.gate = CLKGATE_MAC,
	.gate_addr = SYS_CLKGATE,
};

//apb
static struct clk snx_uart0_clk = {
	.name = "uart0_clk",
	.parent = &snx_apb_clk,
	.ratio = 1,
	.gate = CLKGATE_UART0,
	.gate_addr = SYS_CLKGATE,
};

static struct clk snx_uart1_clk = {
	.name = "uart1_clk",
	.parent = &snx_apb_clk,
	.ratio = 1,
	.gate = CLKGATE_UART1,
	.gate_addr = SYS_CLKGATE,
};

static struct clk snx_i2c0_clk = {
	.name = "i2c0_clk",
	.parent = &snx_apb_clk,
	.ratio = 1,
	.gate = CLKGATE_I2C0,
	.gate_addr = SYS_CLKGATE,
};

static struct clk snx_i2c1_clk = {
	.name = "i2c1_clk",
	.parent = &snx_apb_clk,
	.ratio = 1,
	.gate = CLKGATE_I2C1,
	.gate_addr = SYS_CLKGATE,
};

static struct clk snx_ssp1_clk = {
	.name = "ssp1_clk",
	.parent = &snx_apb_clk,
	.ratio = 1,
	.gate = CLKGATE_SSP1,
	.gate_addr = SYS_CLKGATE,
};

static struct clk snx_ssp2_clk = {
	.name = "ssp2_clk",
	.parent = &snx_apb_clk,
	.ratio = 1,
	.gate = CLKGATE_SSP2,
	.gate_addr = SYS_CLKGATE,
};

static struct clk snx_pwm1_clk = {
	.name = "pwm1_clk",
	.parent = &snx_apb_clk,
	.ratio = 1,
	.gate = CLKGATE1_PWM1,
	.gate_addr = SYS_CLKGATE1,
};

static struct clk snx_pwm2_clk = {
	.name = "pwm2_clk",
	.parent = &snx_apb_clk,
	.ratio = 1,
	.gate = CLKGATE1_PWM2,
	.gate_addr = SYS_CLKGATE1,
};

#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
static struct clk snx_pwm3_clk = {
	.name = "pwm3_clk",
	.parent = &snx_apb_clk,
	.ratio = 1,
	.gate = CLKGATE1_PWM3,
	.gate_addr = SYS_CLKGATE1,
};
#endif

static struct clk snx_ahb_mon_clk = {
	.name = "ahb_mon_clk",
	.parent = &snx_apb_clk,
	.ratio = 1,
	.gate = CLKGATE1_AHB_MON,
	.gate_addr = SYS_CLKGATE1,
};

//isp
static struct clk snx_mipi_clk = {
	.name = "mipi_clk",
	.parent = &snx_isp_clk,
	.ratio = 1,
	.gate = CLKGATE1_MIPI,
	.gate_addr = SYS_CLKGATE1,
};



static struct clk *sn98600_device_clks[] = {
	//ahb
	&snx_usb_clk,
	&snx_aes_clk,
	&snx_dma_clk,
	//ahb2
	&snx_dlc_spi_clk,
#ifdef CONFIG_SNX_MAC
	&snx_mac_clk, 
#endif
	&snx_audio_clk, 

        &snx_ms1_clk,
        &snx_ms2_clk,

	//apb
#ifdef CONFIG_SERIAL_SNX
	&snx_uart0_clk, 
	&snx_uart1_clk, 
#endif

#ifdef CONFIG_I2C_SNX
	&snx_i2c0_clk, 
	&snx_i2c1_clk, 
#endif
	&snx_ssp1_clk,
	&snx_ssp2_clk,
	&snx_pwm1_clk,
	&snx_pwm2_clk,
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
	&snx_pwm3_clk,
#endif
	&snx_ahb_mon_clk,

	//isp
	&snx_mipi_clk,
};

int __init snx_clock_init(void)
{
	int i, asic_id;

	asic_id = readl(SYS_ASIC_ID);
	if(asic_id)		//ASIC board
	{
		int val;

		val = readl(SYS_CTRL2);
		val = (val & BIT(CTRL2_H264_CLK_SRC_BIT)) >> CTRL2_H264_CLK_SRC_BIT;
		if(val)
			snx_h264_clk.parent = &snx_pll_400m_clk;
	}
	else		//FPGA board
	{
		snx_pll_800m_clk.parent = NULL;
		snx_pll_800m_clk.rate = 100 * 1000 * 1000;

		snx_h264_clk.parent = NULL;
		snx_h264_clk.rate = 37.5 * 1000 * 1000;
	}

	snx_apb_clk_rate = clk_get_rate(&snx_apb_clk);
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
	if((snx_apb_clk_rate < 20000000) && (snx_apb_clk_rate != CLOCK_TICK_RATE))
		printk(KERN_WARNING "\nTick clock frequency(%d) isn't equal to APB clock frequency(%ld)! \
				\n\tPlease check whether the setting of 'CONFIG_TICK_CLOCK_RATIO' is correct.\n",
				CLOCK_TICK_RATE, snx_apb_clk_rate);

#else
	if(snx_apb_clk_rate != CLOCK_TICK_RATE)
		printk(KERN_WARNING "\nTick clock frequency(%d) isn't equal to APB clock frequency(%ld)! \
				\n\tPlease check whether the setting of 'CONFIG_TICK_CLOCK_RATIO' is correct.\n",
				CLOCK_TICK_RATE, snx_apb_clk_rate);
#endif

	/* Register primary clocks */
	for (i = 0; i < ARRAY_SIZE(sn98600_primary_clks); i++) 
		clk_register(sn98600_primary_clks[i]);

	/* Initialize and Register device clocks */
	for (i = 0; i < ARRAY_SIZE(sn98600_device_clks); i++) 
	{
		if (!sn98600_device_clks[i]->enable)
			sn98600_device_clks[i]->enable = snx_clk_enable;
		if (!sn98600_device_clks[i]->disable)
			sn98600_device_clks[i]->disable = snx_clk_disable;
		clk_register(sn98600_device_clks[i]);
	}


	return 0;
}
