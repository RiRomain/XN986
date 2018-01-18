
#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/sysdev.h>
#include <linux/io.h>

#include <asm/irq.h>
#include <asm/mach/irq.h>

#include <mach/regs-gpio.h>

static void snx_extint_ack_irq(unsigned int irqno)
{
	unsigned long val;

	irqno -= INT_EXT_BASE;
	val = __raw_readl(GPIO_INTR_CLR);
	val |= (0x1<<irqno);
	__raw_writel(val, GPIO_INTR_CLR);
}

static void snx_extint_mask_irq(unsigned int irqno)
{
	unsigned long val;

	irqno -= INT_EXT_BASE;

	val = __raw_readl(GPIO_INTR_ENB);
	val &= ~(0x1<<irqno);
	__raw_writel(val, GPIO_INTR_ENB);

	val = __raw_readl(GPIO_INTR_MASK);
	val |= (0x1<<irqno);
	__raw_writel(val, GPIO_INTR_MASK);
}

static void snx_extint_unmask_irq(unsigned int irqno)
{
	unsigned long val;

	irqno -= INT_EXT_BASE;
	val = __raw_readl(GPIO_INTR_MASK);
	val &= ~(0x1<<irqno);
	__raw_writel(val, GPIO_INTR_MASK);

	val = __raw_readl(GPIO_INTR_ENB);
	val |= (0x1<<irqno);
	__raw_writel(val, GPIO_INTR_ENB);
}

static int snx_extint_set_type_irq(unsigned int irq, unsigned int type)
{
	unsigned long val;
	int mode = 0, edge = 0, level  = 0;

	irq -= INT_EXT_BASE;

	switch (type)
	{
		case IRQ_TYPE_NONE:
			printk(KERN_WARNING "No edge setting!\n");
			break;
		case IRQ_TYPE_EDGE_RISING:
			mode = GPIO_INT_MODE_EDGE;
			edge = GPIO_INT_EDGE_SINGLE;
			level = GPIO_INT_LEVEL_HIGHT;
			break;
		case IRQ_TYPE_EDGE_FALLING:
			mode = GPIO_INT_MODE_EDGE;
			edge = GPIO_INT_EDGE_SINGLE;
			level = GPIO_INT_LEVEL_LOW;
			break;
		case IRQ_TYPE_EDGE_BOTH:
			mode = GPIO_INT_MODE_EDGE;
			edge = GPIO_INT_EDGE_BOTH;
			level = GPIO_INT_LEVEL_HIGHT;
			break;
		case IRQ_TYPE_LEVEL_LOW:
			mode = GPIO_INT_MODE_LEVEL;
			edge = GPIO_INT_EDGE_SINGLE;
			level = GPIO_INT_LEVEL_LOW;
			break;
		case IRQ_TYPE_LEVEL_HIGH:
			mode = GPIO_INT_MODE_LEVEL;
			edge = GPIO_INT_EDGE_SINGLE;
			level = GPIO_INT_LEVEL_HIGHT;
			break;
		default:
			printk(KERN_ERR "No such irq type %d", type);
			return -1;
	}
	
	val = __raw_readl(GPIO_INTR_MODE);
	val &= ~(0x1<<irq);
	val |= (mode<<irq);
	__raw_writel(val, GPIO_INTR_MODE);

	val = __raw_readl(GPIO_INTR_EDGE);
	val &= ~(0x1<<irq);
	val |= (edge<<irq);
	__raw_writel(val, GPIO_INTR_EDGE);

	val = __raw_readl(GPIO_INTR_LEVEL);
	val &= ~(0x1<<irq);
	val |= (level<<irq);
	__raw_writel(val, GPIO_INTR_LEVEL);

	return 0;
}


static struct irq_chip snx_extint_chip = 
{
	.name = "ext-intc",
	.ack = snx_extint_ack_irq,
	.mask = snx_extint_mask_irq,
	.unmask = snx_extint_unmask_irq,
	.set_type = snx_extint_set_type_irq,
};

static void snx_extint_demux(unsigned int irq, struct irq_desc *desc)
{
	unsigned long eintpnd = __raw_readl(GPIO_INTR_MSKD);

	/* we may as well handle all the pending IRQs here */
	
	while (eintpnd) {
		irq = __ffs(eintpnd);
		eintpnd &= ~(1<<irq);
		irq += INT_EXT_BASE;
		generic_handle_irq(irq);
	}
}

static void __init snx_extint_setup(void)
{
	__raw_writel(0x0, GPIO_INTR_ENB);
	__raw_writel(0xffffffff, GPIO_INTR_MASK);
	__raw_writel(0xffffffff, GPIO_INTR_CLR);
	__raw_writel(0x0, GPIO_INTR_LEVEL);
	__raw_writel(0x0, GPIO_INTR_MODE);
	__raw_writel(0x0, GPIO_INTR_EDGE);
	__raw_writel(0xffffffff, GPIO_BOUNCE_ENABLE);
	__raw_writel(0xfffff, GPIO_BOUNCE_PRESCALE);
}

void __init snx_init_extint(void)
{
	int i;

	/*gpio based interrupts*/
	snx_extint_setup();
	set_irq_chained_handler(INT_GPIO, snx_extint_demux);

	for (i = INT_EXT_BASE; i < (INT_EXT_BASE + SNX_EXT_INT_NR); i++) {
		set_irq_chip(i, &snx_extint_chip);
		set_irq_handler(i, handle_edge_irq);
		set_irq_flags(i, IRQF_VALID);
	}
}
