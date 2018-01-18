#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>
#include <linux/dmapool.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <mach/regs-dmac.h>
#include <linux/platform_device.h>

#include "snx_hdma.h"


/******************* module init param********************/

static unsigned int init_nr_desc_per_channel = 64;
module_param(init_nr_desc_per_channel, uint, 0644);
MODULE_PARM_DESC(init_nr_desc_per_channel, 
	"initial descriptors per channel, default: 64");

/*********************************************************/

static struct snx_desc *
snx_first_active(struct snx_hdma_chan *snx_chan)
{
	return list_first_entry(&snx_chan->active_list,
				struct snx_desc, desc_node);
}

static void snx_chan_clear_intr_status(struct snx_hdma_chan *snx_chan)
{
	/* Clear Terminal count Interrupt Status */
	dmac_writel(snx_chan->device, DMA_INT_TC_CLR,  snx_chan->mask);

	/* Clear Error/Abort Interrupt Status */
	dmac_writel(snx_chan->device, DMA_INT_ERRABT_CLR, 
		snx_chan->mask | snx_chan->mask << SNX_HDMA_INTR_ABT_OFFSET);

	cpu_relax();
}
	
static void snx_chan_init(struct snx_hdma_chan *snx_chan)
{
	memset(&snx_chan->csr, 0, sizeof(snx_chan->csr));
	memset(&snx_chan->cfg, 0, sizeof(snx_chan->cfg));

	snx_chan->csr.csr.src_sel   = DMA_MASTER_1;
	snx_chan->csr.csr.dst_sel   = DMA_MASTER_1;
	snx_chan->csr.csr.src_ctl   = DMA_ADDRESS_INC;
	snx_chan->csr.csr.dst_ctl   = DMA_ADDRESS_INC;
	snx_chan->csr.csr.mode      = DMA_NORMAL;
	snx_chan->csr.csr.src_width = DMA_WIDTH_32BITS;
	snx_chan->csr.csr.dst_width = DMA_WIDTH_32BITS;
	snx_chan->csr.csr.src_size  = DMA_BURST_256;
	snx_chan->csr.csr.priority  = DMA_PRIORITY_0;
	snx_chan->csr.csr.tc_msk    = DMA_TC_MASK;

	channel_writel(snx_chan, DMA_CHANNEL_CSR_OFFSET, snx_chan->csr.u);

	snx_chan->cfg.cfg.int_tc_msk  = 0;
	snx_chan->cfg.cfg.int_err_msk = 0;
	snx_chan->cfg.cfg.int_abt_msk = 0;

	channel_writel(snx_chan, DMA_CHANNEL_CFG_OFFSET, snx_chan->cfg.u);

	snx_chan_clear_intr_status(snx_chan);
	
	cpu_relax();
}

static int snx_chan_enabled(struct snx_hdma_chan *snx_chan)
{
	return dmac_readl(snx_chan->device, DMA_CH_EN) & snx_chan->mask;
}

static void snx_chan_disable(struct snx_hdma_chan *snx_chan)
{
	if(!snx_chan_enabled(snx_chan))
		return;

	/* Wait for channel inactive */
	while(dmac_readl(snx_chan->device, DMA_CH_BUSY) & snx_chan->mask)
		cpu_relax();
	
	/* Disable channel */
	snx_chan->csr.u = channel_readl(snx_chan, DMA_CHANNEL_CSR_OFFSET);
	snx_chan->csr.csr.enable = 0x0;
	channel_writel(snx_chan, DMA_CHANNEL_CSR_OFFSET, snx_chan->csr.u);
	
	/* check channel disabled */
	while(snx_chan_enabled(snx_chan))
		cpu_relax();
}

static void snx_dmac_enable(struct snx_hdma *snx_dma)
{
	/* Enable DMAC
	 * DMA Controller enable 
	 * AHB Master 0 Little-endian
	 * AHB Master 1 Little-endian
	 */

	dmac_writel(snx_dma, DMA_CSR, SNX_HDMA_ENABLE); 
}

static void snx_dmac_disable(struct snx_hdma *snx_dma)
{
	int i;

	/* Disable all channels */	
	for(i = 0; i < snx_dma->dma_common.chancnt; i++)
		snx_chan_disable(&snx_dma->chan[i]);

	/* Disable DMAC */
	dmac_writel(snx_dma, DMA_CSR, SNX_HDMA_DISABLE);
}

/****************************ASYNC DMA API *****************************/

/**
 * snx_assign_cookie - compute and assign new cookie
 * @snx_chan: channel we work on
 * @desc: descriptor to assign cookie for
 *
 * Called with snx_chan->lock held and bh disabled
 */
static dma_cookie_t 
snx_assign_cookie(struct snx_hdma_chan *snx_chan, struct snx_desc *desc)
{
	dma_cookie_t cookie = snx_chan->chan_common.cookie;

	if(++cookie < 0)
		cookie = 1;

	snx_chan->chan_common.cookie = cookie;
	desc->txd.cookie = cookie;

	return cookie;
}

/**
 * snx_do_start - starts the DMA engine for real
 * @snx_chan: the channel we want to start
 * @first: first descriptor in the list we want to begin with
 */
static void 
snx_do_start(struct snx_hdma_chan *snx_chan, struct snx_desc *first)
{
	if(snx_chan_enabled(snx_chan)) {
		dev_err(chan2dev(&snx_chan->chan_common), 
			"BUG: Attempted to start non-idle channel\n");
		return;
	}
	sndma_dbg(chan2dev(&snx_chan->chan_common), 
		"do_start %u descriptor\n", first->txd.cookie);
	
	/* clear any pending interrupt status */
	snx_chan_clear_intr_status(snx_chan);
	
#if 0
	/* Set Source Address Register*/
	channel_writel(snx_chan, DMA_CHANNEL_SRCADDR_OFFSET, first->lld.srcaddr);

	/* Set Destination Address Register */
	channel_writel(snx_chan, DMA_CHANNEL_DSTADDR_OFFSET, first->lld.dstaddr);

	/* Set Linked List Descriptor Pointer Register*/
	channel_writel(snx_chan, DMA_CHANNEL_LLP_OFFSET, *((u32 *)(&first->lld.llp)));

	/* Set Transfer Size Register */
	channel_writel(snx_chan, DMA_CHANNEL_SIZE_OFFSET, first->lld.size);

	snx_chan->csr.csr.dst_sel   = first->lld.ctrl.dst_sel;
	snx_chan->csr.csr.src_sel   = first->lld.ctrl.src_sel;
	snx_chan->csr.csr.dst_ctl   = first->lld.ctrl.dstaddr_ctl;
	snx_chan->csr.csr.src_ctl   = first->lld.ctrl.srcaddr_ctl;
	snx_chan->csr.csr.dst_width = first->lld.ctrl.dst_width;
	snx_chan->csr.csr.src_width = first->lld.ctrl.src_width;
	snx_chan->csr.csr.tc_msk    = first->lld.ctrl.tc_mask;
	snx_chan->csr.csr.ff_th     = first->lld.ctrl.ff_th;
#else
	/* clear any pending interrupt status */
	snx_chan_clear_intr_status(snx_chan);

	/* Set Source Address Register*/
	channel_writel(snx_chan, DMA_CHANNEL_SRCADDR_OFFSET, 0);

	/* Set Destination Address Register */
	channel_writel(snx_chan, DMA_CHANNEL_DSTADDR_OFFSET, 0);

	/* Set Linked List Descriptor Pointer Register*/
	channel_writel(snx_chan, DMA_CHANNEL_LLP_OFFSET, first->txd.phys);

	/* Set Transfer Size Register */
	channel_writel(snx_chan, DMA_CHANNEL_SIZE_OFFSET, 0);
#endif

	channel_writel(snx_chan, DMA_CHANNEL_CSR_OFFSET, 
		snx_chan->csr.u | SNX_HDMA_CHAN_ENABLE);
}

/**
 * snx_tx_submit - set the prepared descriptor(s) to executed by the engine
 * @tx: descriptor at the head of the transaction chain
 */
static dma_cookie_t snx_tx_submit(struct dma_async_tx_descriptor *tx)
{
	struct snx_desc *desc = txd_to_snx_desc(tx);
	struct snx_hdma_chan *snx_chan = to_snx_hdma_chan(tx->chan);

	dma_cookie_t cookie;

	sndma_dbg(chan2dev(tx->chan), "snx_tx_submit %d\n",desc->lld_cnt);
	
	spin_lock_bh(&snx_chan->lock);

	cookie = snx_assign_cookie(snx_chan, desc);

	/* active list is null, indicate channel disabled.
	 * every executing transaction is the first active_list
	 */
	
	if(list_empty(&snx_chan->active_list)) {
		sndma_dbg(chan2dev(tx->chan), "tx_submit: started %u\n",
			desc->txd.cookie);
		
		list_add_tail(&desc->desc_node, &snx_chan->active_list);
		snx_do_start(snx_chan, desc);
	} else {
		sndma_dbg(chan2dev(tx->chan), "tx_submit: queued %u\n",
			desc->txd.cookie);

		list_add_tail(&desc->desc_node, &snx_chan->queue);
	}

	spin_unlock_bh(&snx_chan->lock);

	return cookie;
}

/**
 * static struct snx_desc *snx_alloc_descriptor(struct dma_chan *chan, 
 * 		gfp_t gfp_flags)
 * @chan: the channel to allocate descriptor for
 * @gfp_flags: GFP allocation flags
 */
static struct snx_desc *snx_alloc_descriptor(struct dma_chan *chan,
	gfp_t gfp_flags)
{
	struct snx_desc *desc = NULL;
	struct snx_hdma *snx_dma = to_snx_hdma(chan->device);

	dma_addr_t phys;

	desc = dma_pool_alloc(snx_dma->dma_desc_pool, gfp_flags, &phys);
	if(desc) {
		memset(desc, 0, sizeof(struct snx_desc));

		INIT_LIST_HEAD(&desc->tx_list);
		dma_async_tx_descriptor_init(&desc->txd, chan);
		desc->txd.flags = DMA_CTRL_ACK;
		desc->txd.tx_submit = snx_tx_submit;
		desc->txd.phys = phys;
		desc->flag = 0;
	}

	return desc;
}

/**
 * static struct snx_desc *snx_desc_get(struct snx_hdma_chan *snx_chan)
 * - get an unused descriptor from free_list
 * @snx_chan: Channel we want a new descriptor for
 */
static struct snx_desc *snx_desc_get(struct snx_hdma_chan *snx_chan)
{
	struct snx_desc *desc, *_desc;
	struct snx_desc *retval = NULL;

	unsigned int i = 0;

	LIST_HEAD(tmp_list);

	spin_lock_bh(&snx_chan->lock);
	list_for_each_entry_safe(desc, _desc, &snx_chan->free_list, desc_node) {
		i++;
		if(async_tx_test_ack(&desc->txd)) {
			list_del(&desc->desc_node);
			retval = desc;
			break;
		}
		sndma_dbg(chan2dev(&snx_chan->chan_common), 
			"desc %p not ACKed\n", desc);
	}
	spin_unlock_bh(&snx_chan->lock);
	
	sndma_dbg(chan2dev(&snx_chan->chan_common), 
		"scanned %u descriptors on free_list\n", i);

	if(!retval) {
		retval = snx_alloc_descriptor(&snx_chan->chan_common, GFP_ATOMIC);
		if(retval) {
			spin_lock_bh(&snx_chan->lock);
			snx_chan->descs_allocated++;
			spin_unlock_bh(&snx_chan->lock);
		}
		else { 
			dev_err(chan2dev(&snx_chan->chan_common), 
				"not enough descriptors available\n");
		}
	}
		
	return retval;		
		
}

/**
 * static void snx_desc_put(struct snx_hdma_chan *snx_chan, 
 * 		struct snx_desc *desc)
 * @snx_chan: channel we work on
 * @desc: descriptor, at the head of a chain, to move to free list
 */
static void snx_desc_put(struct snx_hdma_chan *snx_chan, struct snx_desc *desc)
{
	if(desc) {
		struct snx_desc *child;

		spin_lock_bh(&snx_chan->lock);
		list_for_each_entry(child, &desc->tx_list, desc_node)
			sndma_dbg(chan2dev(&snx_chan->chan_common), 
				"moving child desc %p to freelist\n", child);

		list_splice_init(&desc->tx_list, &snx_chan->free_list);

		sndma_dbg(chan2dev(&snx_chan->chan_common),
			"moving desc %p to freelist\n", desc);

		list_add(&desc->desc_node, &snx_chan->free_list);
		spin_lock_bh(&snx_chan->lock);
	}
}

/**
 * snx_chain_complete - finish work for one transaction chain
 * @snx_chan: channel we work on
 * @desc: descriptor at the head of the chain we want do complete
 *
 * Called with snx_chan->lock held and bh disabled
 */
static void snx_chain_complete(struct snx_hdma_chan *snx_chan,
	struct snx_desc *desc)
{
	dma_async_tx_callback callback;
	void *param;

	struct dma_async_tx_descriptor *txd = &desc->txd;

	sndma_dbg(chan2dev(&snx_chan->chan_common), 
		"descriptor %u complete\n", txd->cookie);

	snx_chan->completed_cookie = txd->cookie;

	callback = txd->callback;
	param = txd->callback_param;

	/* move children to free_list */
	list_splice_init(&desc->tx_list, &snx_chan->free_list);
	/* move myself to free_list */
	list_move(&desc->desc_node, &snx_chan->free_list);

	/* unmap DMA addresses */
	if(!snx_chan->chan_common.private) {
		struct device *parent = chan2parent(&snx_chan->chan_common);
		
		if(!(txd->flags & DMA_COMPL_SKIP_DEST_UNMAP)) {
			if(txd->flags & DMA_COMPL_DEST_UNMAP_SINGLE)
				dma_unmap_single(parent, desc->lld.dstaddr, desc->tot_size,
					DMA_FROM_DEVICE);
			else 
				dma_unmap_page(parent, desc->lld.dstaddr, desc->tot_size,
					DMA_FROM_DEVICE);
		}

		if(!(txd->flags & DMA_COMPL_SKIP_SRC_UNMAP)) {
			if(txd->flags & DMA_COMPL_SRC_UNMAP_SINGLE)
				dma_unmap_single(parent, desc->lld.srcaddr, desc->tot_size,
					DMA_TO_DEVICE);
			else 
				dma_unmap_page(parent, desc->lld.srcaddr, desc->tot_size,
					DMA_TO_DEVICE);
		}
	}

	/* for memset */
	if(desc->flag & SNX_HDMA_MEMSET)
		kfree(dma_to_virt(chan2dev(&snx_chan->chan_common), desc->lld.srcaddr));

	if(callback)
		callback(param);

	dma_run_dependencies(txd);
}

/**
 * snx_cleanup_descriptors - cleanup up finished descriptors in active_list
 * @snx_chan: channel to be clean up
 * 
 * Called with snx_chan->lock held and bh disabled
 */
static void snx_cleanup_descriptors(struct snx_hdma_chan *snx_chan)
{
#if 0
	struct snx_desc *desc, *_desc;
	struct snx_desc *child;

	sndma_dbg(chan2dev(&snx_chan->chan_common), "clean descriptors\n");

	list_for_each_entry_safe(desc, _desc, &snx_chan->active_list, desc_node) {

		if(!(1 /*desc completed */))
			return;
		
		list_for_each_entry(child, &desc->tx_list, desc_node) {
			if(!(1 /* child completed */))
				return;
		}
		snx_chain_complete(snx_chan, desc);
	}
#endif

}

/**
 * snx_complete_all - finish work for all transactions
 * @snx_chan: channel to complete transactions for
 *
 * Called with snx_chan->lock held and bh disabled
 */
static void snx_complete_all(struct snx_hdma_chan *snx_chan)
{
	struct snx_desc *desc, *_desc;

	LIST_HEAD(list);

	sndma_dbg(chan2dev(&snx_chan->chan_common), "complete_all\n");

	BUG_ON(snx_chan_enabled(snx_chan));


	/* empty active_list now it is completed */
	list_splice_init(&snx_chan->active_list, &list);

	/* Submit queue descriptors */
	if(!list_empty(&snx_chan->queue)) {
		sndma_dbg(chan2dev(&snx_chan->chan_common),
			"queue not empty\n");
		
		/* empty queue list by moving descriptors to active_list */
		list_splice_init(&snx_chan->queue, &snx_chan->active_list);
		snx_do_start(snx_chan, snx_first_active(snx_chan));
	}

	list_for_each_entry_safe(desc, _desc, &list, desc_node)
		snx_chain_complete(snx_chan, desc);
}

/**
 * snx_advance_work - at the end of a transaction, move forward
 * @snx: channel where the the transaction ended
 * 
 * Called with snx_chan->lock held and bh disabled
 */
static void snx_advance_work(struct snx_hdma_chan *snx_chan)
{
	sndma_dbg(chan2dev(&snx_chan->chan_common), "advance_work\n");

	if(list_empty(&snx_chan->active_list) ||
		list_is_singular(&snx_chan->active_list)) {
		if(list_empty(&snx_chan->active_list))
			sndma_dbg(chan2dev(&snx_chan->chan_common),
				"ist_empty\n");
		
		if(list_is_singular(&snx_chan->active_list))
			sndma_dbg(chan2dev(&snx_chan->chan_common),
				"list_is_singular\n");
		
		snx_complete_all(snx_chan);
	} else {
		snx_chain_complete(snx_chan, snx_first_active(snx_chan));
		snx_do_start(snx_chan, snx_first_active(snx_chan));
	}
}

/**
 * snx_handle_error - handle errors reported by DMA controller
 * @snx_chan: channel where error occurs
 * 
 * Called with snx_chan->lock held and bh disabled
 */
static void snx_handle_error(struct snx_hdma_chan *snx_chan)
{
	struct snx_desc *bad_desc;
	struct snx_desc *child;

	/*
	 * The  descriptor currently at the head of the active list is
	 * broked.  Since we don't have any way to report errors, we'll
	 * just have to scream loudly and try to carry on.
	 */

	bad_desc = snx_first_active(snx_chan);
	list_del_init(&bad_desc->desc_node);

	/* As we are stopped, take advantage to push queued descriptors 
	 * in active_list.
	 */

	list_splice_init(&snx_chan->queue, snx_chan->active_list.prev);

	/* Try to restart the controller */
	if(!list_empty(&snx_chan->active_list))
		snx_do_start(snx_chan, snx_first_active(snx_chan));

	/*
	 * KERN_CRITICAL may seem harsh, but since this only happens
	 * when someone submits a bad physical address in a 
	 * descriptor, we should consider ourslves lucky that the 
	 * controller flagged an error instead of scribbling over 
	 * random memory locations.
	 */

	dev_crit(chan2dev(&snx_chan->chan_common),
		"Bad descriptor submitted for DMA!\n");
	dev_crit(chan2dev(&snx_chan->chan_common),
		" cookie: %d\n", bad_desc->txd.cookie);

	dump_lld(snx_chan, &bad_desc->lld);

	list_for_each_entry(child, &bad_desc->tx_list, desc_node)
		dump_lld(snx_chan, &child->lld);

	/* Pretend the descriptor completed successfully */
	snx_chain_complete(snx_chan, bad_desc);
}
	 

/**
 * static snx_ahbdam_alloc_chan_resources(struct dma_chan *chan)
 * @chan: allocate descriptor resources for this channel
 */
static int snx_hdma_alloc_chan_resources(struct dma_chan *chan)
{
	struct snx_hdma_chan *snx_chan = to_snx_hdma_chan(chan);
	struct snx_hdma      *snx_dma  = to_snx_hdma(chan->device);
	struct snx_desc *desc;
	int i;

	LIST_HEAD(tmp_list);

	if(snx_chan_enabled(snx_chan)) {
		sndma_dbg(chan2dev(chan), "DMA channel not idle ?\n");
		return -EIO;
	}

	if(!list_empty(&snx_chan->free_list))
		return snx_chan->descs_allocated;

	for(i = 0; i < init_nr_desc_per_channel; i++) {
		desc = snx_alloc_descriptor(chan, GFP_KERNEL);
		if(!desc) {
			dev_err(snx_dma->dma_common.dev, 
				"only %d initial descriptors\n", i);
			break;
		}
		list_add_tail(&desc->desc_node, &tmp_list);
	}

	spin_lock_bh(&snx_chan->lock);
	snx_chan->descs_allocated = i;
	list_splice(&tmp_list, &snx_chan->free_list);
	spin_unlock_bh(&snx_chan->lock);

	sndma_dbg(chan2dev(chan), "alloc_chan_resources: allocated %d descriptor\n", 
		snx_chan->descs_allocated);

	return snx_chan->descs_allocated;
}
	
/**
 * static snx_hdma_free_chan_resources(struct dma_chan *chan)
 * @chan: DMA channel
 */
 
static void snx_hdma_free_chan_resources(struct dma_chan *chan)
{
	struct snx_hdma_chan *snx_chan = to_snx_hdma_chan(chan);
	struct snx_hdma      *snx_dma  = to_snx_hdma(chan->device);

	struct snx_desc *desc, *_desc;

	LIST_HEAD(list);
	
	sndma_dbg(chan2dev(chan), "free_chan_resources: descs allocated %u\n",
		snx_chan->descs_allocated);

	BUG_ON(!list_empty(&snx_chan->active_list));
	BUG_ON(!list_empty(&snx_chan->queue));
	BUG_ON(snx_chan_enabled(snx_chan));

	list_for_each_entry_safe(desc, _desc, &snx_chan->free_list, desc_node) {
		sndma_dbg(chan2dev(chan), "freeing descriptor %p\n", desc);

		list_del(&desc->desc_node);
		dma_pool_free(snx_dma->dma_desc_pool, desc, desc->txd.phys);
	}
	list_splice_init(&snx_chan->free_list, &list);
	snx_chan->descs_allocated = 0;

	sndma_dbg(chan2dev(chan), "free_chan_resources: done\n");
}

static struct dma_async_tx_descriptor * 
snx_hdma_prep_dma_memcpy(struct dma_chan *chan, dma_addr_t dest, 
	dma_addr_t src, size_t len, unsigned long flags)
{
	struct snx_hdma_chan *snx_chan = to_snx_hdma_chan(chan);
	struct snx_desc *desc  = NULL;
	struct snx_desc *first = NULL;
	struct snx_desc *prev  = NULL;
	
	size_t xfer_count;
	size_t offset;

	snx_ctl_t ctrl ;

	sndma_dbg(chan2dev(chan), "prep_dma_memcpy: DST 0x%08x SRC 0x%08X "
		"len 0x%08x Flags 0x%08x\n", dest, src, len, (int)flags);

	if(unlikely(!len)) {
		sndma_dbg(chan2dev(chan), "prep_dma_memcpy: length is zero!\n");
		return NULL;
	}

	ctrl.reserved    = 0;
	ctrl.dst_sel     = DMA_MASTER_1;
	ctrl.src_sel     = DMA_MASTER_1;
	ctrl.dstaddr_ctl = DMA_ADDRESS_INC;
	ctrl.srcaddr_ctl = DMA_ADDRESS_INC;
	ctrl.tc_mask     = DMA_TC_MASK;
	ctrl.ff_th       = DMA_FIFO_TH_1;
	
	if(!((src | dest | len) & 0x3))
		ctrl.src_width = ctrl.dst_width = DMA_WIDTH_32BITS;
	else if(!((src | dest | len) & 0x1))
		ctrl.src_width = ctrl.dst_width = DMA_WIDTH_16BITS;
	else 
		ctrl.src_width = ctrl.dst_width = DMA_WIDTH_8BITS;

	sndma_dbg(chan2dev(chan), "dma width %d\n", ctrl.src_width);

	for(offset = 0; offset < len; offset += xfer_count << ctrl.src_width) {
		xfer_count = min_t(size_t, (len - offset) >> ctrl.src_width,
			SNX_HDMA_BTSIZE_MAX);
		
		desc = snx_desc_get(snx_chan);
		if(!desc)
			goto err_desc_get;
		
		desc->lld.srcaddr = src  + offset;
		desc->lld.dstaddr = dest + offset;
		desc->lld.size = xfer_count;
		desc->lld.ctrl = ctrl;

		if(!first)
			first = desc;
		else {
			prev->lld.llp.llp_master = DMA_MASTER_1;
			prev->lld.llp.llp_reserved = 0;
			prev->lld.llp.llp_addr = (desc->txd.phys) >> 2;
			list_add_tail(&desc->desc_node, &first->tx_list);
		}
		prev = desc;
	}

	first->txd.cookie = -EBUSY;
	first->tot_size = len;

	desc->lld.ctrl.tc_mask = 0; 
	desc->lld.llp.llp_master = DMA_MASTER_1;
	desc->lld.llp.llp_reserved = 0;
	desc->lld.llp.llp_addr = 0;

	desc->txd.flags = flags;

	return &first->txd;

err_desc_get:
	snx_desc_put(snx_chan, first);

	return NULL;
}


#if 0
static struct dma_async_tx_descriptor *
snx_hdma_prep_dma_xor(struct dma_chan *chan, dma_addr_t dest, 
	dma_addr_t src,unsigned int src_cnt, size_t len, unsigned long flags)
{
	printk("Not implement this function: %s\n", __func__);
	return NULL;
}

static struct dma_async_tx_descriptor *
snx_hdma_prep_dma_xor_val(struct dma_chan *chan, dma_addr_t *src, 
	unsigned int src_cnt, size_t len, enum sum_check_flags *result, 
	unsigned long flags)
{
	printk("Not implement this function: %s\n", __func__);
	return NULL;
}

static struct dma_async_tx_descriptor *
snx_hdma_prep_dma_pq(struct dma_chan *chan, dma_addr_t *dst, 
	dma_addr_t *src, unsigned int src_cnt, const unsigned char *scf, 
	size_t len, unsigned long flags)
{
	printk("Not implement this function: %s\n", __func__);
	return NULL;
}

static struct dma_async_tx_descriptor *
snx_hdma_prep_dma_pq_val(struct dma_chan *chan, dma_addr_t *pq, 
	dma_addr_t *src, unsigned int src_cnt, const unsigned char *scf, 
	size_t len, enum sum_check_flags *pqres, unsigned long flags)
{
	printk("Not implement this function: %s\n", __func__);
	return NULL;
}
#endif

static struct dma_async_tx_descriptor *
snx_hdma_prep_dma_memset(struct dma_chan *chan, dma_addr_t dest, 
	int value, size_t len, unsigned long flags)
{
	struct snx_hdma_chan *snx_chan = to_snx_hdma_chan(chan);
	struct snx_desc *desc  = NULL;
	struct snx_desc *first = NULL;
	struct snx_desc *prev  = NULL;

	u32 *src_addr;

	size_t xfer_count;
	size_t offset;

	snx_ctl_t ctrl ;

	sndma_dbg(chan2dev(chan), "prep_dma_memset: DST 0x%08x VAL 0x%08X "
		"len 0x%08x Flags 0x%08x\n", dest, value, len, (int)flags);

	if(unlikely(!len)) {
		sndma_dbg(chan2dev(chan), "prep_dma_memcpy: length is zero!\n");
		return NULL;
	}

	src_addr = (u32 *)kmalloc(sizeof(u32), GFP_DMA | GFP_KERNEL);

	*src_addr = 0x01010101 * (value & 0xff);

	ctrl.reserved    = 0;
	ctrl.dst_sel     = DMA_MASTER_1;
	ctrl.src_sel     = DMA_MASTER_1;
	ctrl.dstaddr_ctl = DMA_ADDRESS_INC;
	ctrl.srcaddr_ctl = DMA_ADDRESS_FIX;
	ctrl.tc_mask     = DMA_TC_MASK;
	ctrl.ff_th       = DMA_FIFO_TH_1;
	
	if(!((dest | len) & 0x3))
		ctrl.src_width = ctrl.dst_width = DMA_WIDTH_32BITS;
	else if(!((dest | len) & 0x1))
		ctrl.src_width = ctrl.dst_width = DMA_WIDTH_16BITS;
	else 
		ctrl.src_width = ctrl.dst_width = DMA_WIDTH_8BITS;

	for(offset = 0; offset < len; offset += xfer_count << ctrl.src_width) {
		xfer_count = min_t(size_t, (len - offset) >> ctrl.src_width,
			SNX_HDMA_BTSIZE_MAX);
		
		desc = snx_desc_get(snx_chan);
		if(!desc)
			goto err_desc_get;

		desc->lld.srcaddr = virt_to_dma(chan2dev(chan),src_addr);
		desc->lld.dstaddr = dest + offset;
		desc->lld.size = xfer_count;
		desc->lld.ctrl = ctrl;

		if(!first)
			first = desc;
		else {
			prev->lld.llp.llp_master = DMA_MASTER_1;
			prev->lld.llp.llp_reserved = 0;
			prev->lld.llp.llp_addr = (desc->txd.phys) >> 2;
			list_add_tail(&desc->desc_node, &first->tx_list);
		}
		prev = desc;
	}
	
	first->txd.cookie = -EBUSY;
	first->tot_size = len;
	first->flag = SNX_HDMA_MEMSET;
	
	desc->lld.ctrl.tc_mask = 0; 
	desc->lld.llp.llp_master = DMA_MASTER_1;
	desc->lld.llp.llp_reserved = 0;
	desc->lld.llp.llp_addr = 0;

	desc->txd.flags = flags | DMA_COMPL_SKIP_SRC_UNMAP;

	return &first->txd;

err_desc_get:
	snx_desc_put(snx_chan, first);

	return NULL;
}

#if 0
static struct dma_async_tx_descriptor *
snx_hdma_prep_dma_interrupt(struct dma_chan *chan, unsigned long flags)
{
	printk("Not implement this function: %s\n", __func__);
	return NULL;
}

static struct dma_async_tx_descriptor *
snx_hdma_prep_slave_sg(struct dma_chan *chan, unsigned long flags)
{
	printk("Not implement this function: %s\n", __func__);
	return NULL;
}


static int 
snx_hdma_control(struct dma_chan *chan, enum dma_ctrl_cmd cmd, 
	unsigned long arg)
{
	struct snx_hdma_chan *snx_chan = to_snx_hdma_chan(chan);
	struct snx_hdma      *snx_dma = to_snx_hdma(chan->device);

	struct snx_desc *desc, *_desc;

	LIST_HEAD(list);

	switch(cmd) {
		case DMA_TERMINATE_ALL:
			spin_lock_bh(&snx_chan->lock);

			channel_writel(snx_chan, DMA_CHANNEL_CSR_OFFSET, 
				SNX_HDMA_CHANNEL_ABORT);

			/* confirm that this channel is disable */
			while(!(dmac_readl(snx_dma, DMA_CH_EN) & snx_chan->mask))
				cpu_relax();

			list_splice_init(&snx_chan->queue, &list);
			list_splice_init(&snx_chan->active_list, &list);

			spin_unlock_bh(&snx_chan->lock);

			/* Flush all pending and queued descriptors */
			list_for_each_entry_safe(desc, _desc, &list, desc_node)
				snx_chain_complete(snx_chan, desc);
		
			break;
		
		case DMA_PAUSE:
			break;
		
		case DMA_RESUME:
			break;

		default:
			dev_err(chan2dev(chan), "DMA CTRL CMD not support\n");
	}
	
	return 0;
}
#endif

/**
 * snx_hdma_tx_status - poll for transaction completion
 * @chan: DMA channel
 * @cookie: transaction identifier to check status of
 * @txstate: if not %NULL updated with transaction state
 */
static enum dma_status snx_hdma_tx_status(struct dma_chan *chan, 
	dma_cookie_t cookie, struct dma_tx_state *txstate)
{
	struct snx_hdma_chan *snx_chan = to_snx_hdma_chan(chan);

	dma_cookie_t last_used;
	dma_cookie_t last_completed;

	enum dma_status ret;

	spin_lock_bh(&snx_chan->lock);

	last_completed = snx_chan->completed_cookie;
	last_used = chan->cookie;

	ret = dma_async_is_complete(cookie, last_completed, last_used);
	if(ret != DMA_SUCCESS) {
		snx_cleanup_descriptors(snx_chan);

		last_completed = snx_chan->completed_cookie;
		last_used = chan->cookie;

		ret = dma_async_is_complete(cookie, last_completed, last_used);
	}

	spin_unlock_bh(&snx_chan->lock);

	dma_set_tx_state(txstate, last_completed, last_used, 0);

	sndma_dbg(chan2dev(chan), "tx_status: %d last_complete %d last_used %d\n",
		cookie, last_completed ? last_completed : 0, last_used ? last_used : 0);

	return ret;
	
}

/**
 * snx_hdma_issue_pending - try to finish work
 * @chan: target DMA channel
 */
 
static void snx_hdma_issue_pending(struct dma_chan *chan)
{
#if 0
	struct snx_hdma_chan *snx_chan = to_snx_hdma_chan(chan);

	sndma_dbg(chan2dev(chan), "issue_pending\n");

	if(!snx_chan_enabled(snx_chan)) {
		spin_lock_bh(&snx_chan->lock);
		snx_advance_work(snx_chan);
		spin_unlock_bh(&snx_chan->lock);
	}	
#endif
}

/********************** IRQ & tasklet ***************************************/

static void snx_hdma_tasklet(unsigned long data)
{
	struct snx_hdma_chan *snx_chan = (struct snx_hdma_chan *) data;

	sndma_dbg(chan2dev(&snx_chan->chan_common), "snx_hdma_tasklet\n");

	dump_regs(snx_chan);
	
	if (snx_chan_enabled(snx_chan)) {
		dev_err(chan2dev(&snx_chan->chan_common), 
			"BUG: channel enable in tasklet\n");
		return;
	}
	
	spin_lock(&snx_chan->lock);
	switch(snx_chan->irq_event) {
	case SNX_DMA_ERR:
		snx_handle_error(snx_chan);
		break;

	case SNX_DMA_TC:
	case SNX_DMA_ABT:
		snx_advance_work(snx_chan);
		break; 
	case SNX_DMA_NONE:
		break;
	}
	spin_unlock(&snx_chan->lock);
}

static irqreturn_t snx_hdma_interrupt(int irq, void *dev_id)
{
	int i, retval;
	unsigned int pending, errabt;
	struct snx_hdma *snx_dma = (struct snx_hdma *)dev_id;
	struct snx_hdma_chan *snx_chan;
	
	retval = IRQ_NONE;
	
	do {
		pending = dmac_readl(snx_dma, DMA_INT) & snx_dma->all_chan_mask;

		if(!pending)
			break;

		for(i = 0; i < snx_dma->dma_common.chancnt; i++) {
			snx_chan = &snx_dma->chan[i];

			if(!(pending & snx_chan->mask))
				continue;
			
			sndma_dbg(chan2dev(&snx_chan->chan_common), "interrupt\n");
			
			if(dmac_readl(snx_dma, DMA_INT_TC) & snx_chan->mask) {
				/* The DMA terminal count interrupts after masking */
				
				dmac_writel(snx_dma, DMA_INT_TC_CLR, snx_chan->mask);
				snx_chan->irq_event = SNX_DMA_TC;
	
				retval = IRQ_HANDLED;
			}else {
				errabt = dmac_readl(snx_dma, DMA_INT_ERRABT) 
					& (snx_dma->all_chan_mask 
					| (snx_dma->all_chan_mask << SNX_HDMA_INTR_ABT_OFFSET));
					
				
				if(errabt & snx_chan->mask) {
					/* The DMA error interrupts after masking */
					
					dmac_writel(snx_dma, DMA_INT_ERRABT_CLR, snx_chan->mask);
					snx_chan->irq_event = SNX_DMA_ERR;
					
					retval = IRQ_HANDLED;
				} else if(errabt & 
					(snx_chan->mask << SNX_HDMA_INTR_ABT_OFFSET)) {
					/* The DMA abort interrupts after masking */
					
					dmac_writel(snx_dma, DMA_INT_ERRABT_CLR, 
						snx_chan->mask << SNX_HDMA_INTR_ABT_OFFSET);
					snx_chan->irq_event = SNX_DMA_ABT;
					
					retval = IRQ_HANDLED;
				}
			}
			
			if(retval == IRQ_HANDLED)
				tasklet_schedule(&snx_chan->tasklet);
		}
	} while(pending);

	return retval;
}


static int __init snx_hdma_probe(struct platform_device *pdev)
{
	int i, err, irq;
	size_t size;
	struct resource *res;
	struct snx_hdma *snx_dma;

	

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(!res)
		return -EINVAL;

	irq = platform_get_irq(pdev, 0);
	if(irq < 0)
		return irq;

	size  = sizeof(struct snx_hdma);
	size += SNX_HDMA_MAX_NR_CHANNELS * sizeof(struct snx_hdma_chan);
	
	snx_dma = kmalloc(size, GFP_KERNEL); 
	if(!snx_dma)
		return -ENOMEM;

	memset(snx_dma, 0, size);

	dma_cap_set(DMA_MEMCPY, snx_dma->dma_common.cap_mask);
	dma_cap_set(DMA_MEMSET, snx_dma->dma_common.cap_mask);

	snx_dma->all_chan_mask = (1 << SNX_HDMA_MAX_NR_CHANNELS) -1;

	size = res->end - res->start + 1;
	if(!request_mem_region(res->start, size, pdev->dev.driver->name)) {
		err = -EBUSY;
		goto err_kfree;
	}
	
	snx_dma->regs = ioremap(res->start, size);
	if(!snx_dma->regs) {
			err = -ENOMEM;
			goto err_release_region;
	}
	
	snx_dma->clk = clk_get(&pdev->dev, "ahbdma_clk");
	if(IS_ERR(snx_dma->clk)) {
		err = PTR_ERR(snx_dma->clk);
		goto err_clk;
	}
	clk_enable(snx_dma->clk);
	
	err = request_irq(irq, snx_hdma_interrupt, IRQF_DISABLED, 
		"snx_hdma", snx_dma);
	if(err)
		goto err_irq;

	platform_set_drvdata(pdev, snx_dma);

	/* create a pool of consistent memory blocks for hardware descriptors */
	snx_dma->dma_desc_pool = dma_pool_create("snx_hdma_desc_pool", 
		&pdev->dev, sizeof(struct snx_desc), 4, 0);
	
	if(!snx_dma->dma_desc_pool) {
		dev_err(&pdev->dev, "No memory for descriptors dma pool\n");
		err = -ENOMEM;
		goto err_pool_create;
	}

	INIT_LIST_HEAD(&snx_dma->dma_common.channels);
	for(i = 0; i < SNX_HDMA_MAX_NR_CHANNELS; i++, snx_dma->dma_common.chancnt++) {
		struct snx_hdma_chan *snx_chan = &snx_dma->chan[i];

		snx_chan->chan_common.device = &snx_dma->dma_common;
		snx_chan->chan_common.cookie = snx_chan->completed_cookie = 1;
		snx_chan->chan_common.chan_id = i;
		
		list_add_tail(&snx_chan->chan_common.device_node,
			&snx_dma->dma_common.channels);
		
		snx_chan->ch_regs = snx_dma->regs + DMA_CHANNEL0_BASE 
			+ i * DMA_CHANNEL_OFFSET;
		
		snx_chan->mask = 1 << i;
		snx_chan->irq_event = SNX_DMA_NONE;
		snx_chan->device = snx_dma;
		spin_lock_init(&snx_chan->lock);

		INIT_LIST_HEAD(&snx_chan->active_list);
		INIT_LIST_HEAD(&snx_chan->queue);
		INIT_LIST_HEAD(&snx_chan->free_list);
		
		tasklet_init(&snx_chan->tasklet, snx_hdma_tasklet, 
			(unsigned long)snx_chan);
		
		snx_chan_init(snx_chan);
	}

	/* set base routines */
	snx_dma->dma_common.device_alloc_chan_resources = snx_hdma_alloc_chan_resources;
	snx_dma->dma_common.device_free_chan_resources = snx_hdma_free_chan_resources;
	snx_dma->dma_common.device_tx_status = snx_hdma_tx_status;
	snx_dma->dma_common.device_issue_pending = snx_hdma_issue_pending;
	//snx_dma->dma_common.device_control = snx_hdma_control;
	snx_dma->dma_common.dev = &pdev->dev;

	/* set prep routines based on capability */
	if(dma_has_cap(DMA_MEMCPY, snx_dma->dma_common.cap_mask))
		snx_dma->dma_common.device_prep_dma_memcpy = snx_hdma_prep_dma_memcpy;

	if(dma_has_cap(DMA_MEMSET, snx_dma->dma_common.cap_mask))
		snx_dma->dma_common.device_prep_dma_memset = snx_hdma_prep_dma_memset;

	snx_dmac_enable(snx_dma);

	dev_info(&pdev->dev, "SNX AHB DMA Controller (%s %s), %d channels\n", 
		dma_has_cap(DMA_MEMCPY, snx_dma->dma_common.cap_mask) ? "memcpy" : "",
		dma_has_cap(DMA_MEMSET, snx_dma->dma_common.cap_mask) ? "memset" : "",
		snx_dma->dma_common.chancnt);

	err = dma_async_device_register(&snx_dma->dma_common);
	if(err < 0) {
		dev_err(&pdev->dev, "dma_async_device_register failed\n");
		goto dma_async_device_register_err;
	}

	return 0;

dma_async_device_register_err:
	snx_dmac_disable(snx_dma);
err_pool_create:
	platform_set_drvdata(pdev, NULL);
	free_irq(irq, snx_dma);
err_irq:
	clk_disable(snx_dma->clk);
	clk_put(snx_dma->clk);
err_clk:
	iounmap(snx_dma->regs);
	snx_dma->regs = NULL;
err_release_region:
	release_mem_region(res->start, size);
err_kfree:
	kfree(snx_dma);
	
	return err;
}

static int __exit snx_hdma_remove(struct platform_device *pdev)
{
	struct resource *res;
	struct dma_chan *chan, *_chan;
	struct snx_hdma *snx_dma;

	int i;

	snx_dma = platform_get_drvdata(pdev);

	snx_dmac_disable(snx_dma);

	for(i = 0; i < snx_dma->dma_common.chancnt; i++) 
		dma_release_channel(&snx_dma->chan[i].chan_common);

	dma_async_device_unregister(&snx_dma->dma_common);

	dma_pool_destroy(snx_dma->dma_desc_pool);
	platform_set_drvdata(pdev, NULL);
	free_irq(platform_get_irq(pdev, 0), snx_dma);
	
	list_for_each_entry_safe(chan, _chan, &snx_dma->dma_common.channels,
		device_node) {
		struct snx_hdma_chan *snx_chan = to_snx_hdma_chan(chan);

		snx_chan_disable(snx_chan);
		tasklet_disable(&snx_chan->tasklet);
		
		tasklet_kill(&snx_chan->tasklet);
		list_del(&chan->device_node);
	}

	clk_disable(snx_dma->clk);
	clk_put(snx_dma->clk);

	iounmap(snx_dma->regs);
	snx_dma->regs = NULL;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	release_mem_region(res->start, res->end - res->start + 1);

	kfree(snx_dma);

	return 0;
}

#if 0
static void snx_hdma_shutdown(struct platform_device *pdev)
{
	struct snx_hdma *snx_dma = platform_get_drvdata(pdev);

	snx_dmac_disable(snx_dma);
	clk_disable(snx_dma->clk);
}
#endif

static struct platform_driver snx_hdma_driver = {
		.probe    = snx_hdma_probe,
		.remove   = __exit_p(snx_hdma_remove),
		.driver   = {
				.name = "snx_hdma",
		},
};

/************************************************************************/
/*
static u64 snx_hdmamask = DMA_BIT_MASK(32);

static struct snx_hdma_platform_data soninx_hdma_pdata = {
			.nr_channels	= 4,
};

static void snx_hdma_release(struct device * dev)
{
    return ;
}

static struct resource snx_hdma_resources[] = {
	[0] = {
		.start	= SNX_DMAC_BASE,
		.end	= SNX_DMAC_BASE + SZ_512 - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= INT_DMAC,
		.end	= INT_DMAC,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device snx_hdma_device = {
	.name = "snx_hdma",
	.id	  = -1,
	.dev  = {
		.dma_mask		= &snx_hdmamask,
		.coherent_dma_mask	= DMA_BIT_MASK(32),
		.platform_data		= &soninx_hdma_pdata,
		.release = snx_hdma_release,
	},
	.resource	= snx_hdma_resources,
	.num_resources	= ARRAY_SIZE(snx_hdma_resources),
};
*/
/********************************************************************/

static int __init snx_hdma_init(void)
{
	int retval;
	
	retval = platform_driver_register(&snx_hdma_driver);
	if(retval < 0) {
		printk(KERN_ERR "SNX AHB DMA driver register failed\n");
		return retval;
	}

	printk(KERN_INFO "SNX AHB DMA driver register\n");
	
	return 0;
}

static void __exit snx_hdma_exit(void)
{
	platform_driver_unregister(&snx_hdma_driver);
	printk(KERN_INFO "SNX AHB DMA drevice unregister\n");
}

module_init(snx_hdma_init);
module_exit(snx_hdma_exit);

MODULE_DESCRIPTION("SNX AHB DMA Controller Driver");
MODULE_AUTHOR("Timing Gong <timing_gong@cdmail.snx.com.cn>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("snx_hdma");
