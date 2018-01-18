// Create at 2011/05/06 by yanjie_yang

#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/sysdev.h>
#include <linux/io.h>

#include <asm/irq.h>
#include <asm/mach/irq.h>

#include <mach/regs-irq.h>

extern void snx_init_extint(void);

/*
 * All IO addresses are mapped onto VA 0xFFFx.xxxx, where x.xxxx
 * is the (PA >> 12).
 *
 * Setup a VA for the Versatile Vectored Interrupt Controller.
 */
#define __io_address(n)		__io(IO_ADDRESS(n))

static void snx_ack_irq(unsigned int irq)
{
	u32 val;

	if (irq < FIQ_NR_BASE) 
	{
		val = 1 << (irq - IRQ_NR_BASE);
		writel(val, IRQ_CLR);
	}
       	else 
	{
		val = 1 << (irq - FIQ_NR_BASE);
		writel(val, FIQ_CLR);
	}
}

static void snx_mask_irq(unsigned int irq)
{
	u32 val;

	if (irq < FIQ_NR_BASE) 
	{
		val = readl(IRQ_MASK);
		val &= ~(1 << (irq - IRQ_NR_BASE));
		writel(val, IRQ_MASK);
	}
       	else 
	{
		val = readl(FIQ_MASK);
		val &= ~(1 << (irq - FIQ_NR_BASE));
		writel(val, FIQ_MASK);
	}
}

static void snx_unmask_irq(unsigned int irq)
{
	u32 val;

	if (irq < FIQ_NR_BASE) 
	{
		val = readl(IRQ_MASK);
		val |= 1 << (irq - IRQ_NR_BASE);
		writel(val, IRQ_MASK);
	}
       	else 
	{
		val = readl(FIQ_MASK);
		val |= 1 << (irq - FIQ_NR_BASE);
		writel(val, FIQ_MASK);
	}
}


static struct irq_chip snx_irq_chip = 
{
	.name	= "snx_intc",
	.ack	= snx_ack_irq,
	.mask	= snx_mask_irq,
	.unmask	= snx_unmask_irq,
};

/*
 * Initialize the interrupt controller.
 */
void __init snx_intc_setup(unsigned int irq_mode, unsigned int irq_level,
				    unsigned int fiq_mode, unsigned int fiq_level)
{
	/* Initialize the IRQ interrupt controller */
	writel(0x0, IRQ_MASK);			/* disable all IRQs */
	writel(0xffffffff, IRQ_CLR);		/* clear all IRQs */
	writel(irq_mode, IRQ_TRG_MODE);		/* level trigger or edge trigger */
	writel(irq_level, IRQ_TRG_LEVEL);		/* high level or rising edge */

	/* Initialize the FIQ interrupt controller */
	writel(0x0, FIQ_MASK);			/* disable all FIQs */
	writel(0xffffffff, FIQ_CLR);		/* clear all FIQs */
	writel(fiq_mode, FIQ_TRG_MODE);		/* level trigger or edge trigger */
	writel(fiq_level, FIQ_TRG_LEVEL);		/* high level or rising edge */
}

void __init sn98600_init_irq(void)
{
	unsigned int i, edge;

	/* setup interrupt controller */
	snx_intc_setup(SNX_IRQ_TRIGGER_MODE, SNX_IRQ_TRIGGER_LEVEL,
	SNX_FIQ_TRIGGER_MODE, SNX_FIQ_TRIGGER_LEVEL);

	/* Register all IRQs */
	for(i = IRQ_NR_BASE, edge = 1; i < IRQ_NR_BASE + SNX_IRQ_NR; i++, edge <<= 1)
	{
		set_irq_chip(i, &snx_irq_chip);	
		if (SNX_IRQ_TRIGGER_MODE & edge) 
		{
			set_irq_handler(i, handle_edge_irq);	
		}
	       	else 
		{
			set_irq_handler(i, handle_level_irq);
		}
		set_irq_flags(i, IRQF_VALID | IRQF_PROBE);
	}

	/* Register all FIQs */
	for (i=FIQ_NR_BASE, edge=1; i<FIQ_NR_BASE+SNX_FIQ_NR; i++, edge<<=1)
	{
		set_irq_chip(i, &snx_irq_chip);
		if (SNX_FIQ_TRIGGER_MODE & edge) 
		{
			set_irq_handler(i, handle_edge_irq);
		}
	       	else 
		{
			set_irq_handler(i, handle_level_irq);
		}
		set_irq_flags(i, IRQF_VALID | IRQF_PROBE);
	}

	snx_init_extint();
}
