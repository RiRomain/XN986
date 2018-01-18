#ifndef __SNX_AHB_DMA_H__
#define __SNX_AHB_DMA_H__

#include <asm/scatterlist.h>
#include <linux/dma-mapping.h>
#include <linux/dmapool.h>

#include <linux/dmaengine.h>


#define SNX_HDMA_MAX_NR_CHANNELS 4

#define SNX_HDMA_ENABLE  (0x1 << 0)
#define SNX_HDMA_DISABLE (0x0 << 0)

#define SNX_HDMA_CHAN_ENABLE  (0x1 << 0)
#define SNX_HDMA_CHAN_DISABLE (0x0 << 0)

#define SNX_HDMA_CHANNEL_ABORT (0x1 << 15)

#define SNX_HDMA_INTR_ABT_OFFSET 0x10

#define SNX_HDMA_BTSIZE_MAX 0x003FFFFF


typedef enum snx_irq_event {
	SNX_DMA_NONE,
	SNX_DMA_TC,
	SNX_DMA_ABT,
	SNX_DMA_ERR,
}snx_irq_event_t;

enum dma_priority {
	DMA_PRIORITY_0,
	DMA_PRIORITY_1,
	DMA_PRIORITY_2,
	DMA_PRIORITY_3,
};

enum dma_burst_size{
	DMA_BURST_1,
	DMA_BURST_4,
	DMA_BURST_8,
	DMA_BURST_16,
	DMA_BURST_32,
	DMA_BURST_64,
	DMA_BURST_128,
	DMA_BURST_256,
};

enum dma_width{
	DMA_WIDTH_8BITS,
	DMA_WIDTH_16BITS,
	DMA_WIDTH_32BITS,
};

enum dma_fifo_threshold {
	DMA_FIFO_TH_1,
	DMA_FIFO_TH_2,
	DMA_FIFO_TH_4,
	DMA_FIFO_TH_8,
	DMA_FIFO_TH_16,
};

enum dma_mode{
	DMA_NORMAL,
	DMA_HARDWARE_HANDSHAKE,
};

enum dma_address_ctl{
	DMA_ADDRESS_INC,
	DMA_ADDRESS_DEC,
	DMA_ADDRESS_FIX,
};

enum dma_AHB_master{
	DMA_MASTER_0,
	DMA_MASTER_1,
};

enum  dma_tc_mask {
	DMA_TC_UNMASK,
	DMA_TC_MASK,
};

/**
 * struct snx_ch_csr - DMA controller register description
 */
 typedef struct snx_ch_csr {
 	u32 enable:1;
	u32 dst_sel:1;
	u32 src_sel:1;
	u32 dst_ctl:2;
	u32 src_ctl:2;
	u32 mode:1;
	u32 dst_width:3;
	u32 src_width:3;
	u32 resered1:1;
	u32 abort:1;
	u32 src_size:3;
	u32 prot:3;
	u32 priority:2;
	u32 ff_th:3;
	u32 reserved0:4;
	u32 tc_msk:1;
}snx_ch_csr_t;

/**
 * struct snx_ch_cfg - DMA Configuration register
 */
 typedef struct snx_ch_cfg {
 	u32 int_tc_msk:1;
	u32 int_err_msk:1;
	u32 int_abt_msk:1;
	u32 src_req:4;
	u32 busy:1;
	u32 dst_rq:4;
	u32 dst_he:1;
	u32 reserved0:2;
	u32 llp_cnt:4;
	u32 reserved1:12;
}snx_ch_cfg_t;

/**
 * struct snx_llp - DMA linked list pointer address
 */
typedef struct snx_llp {
 	u32 llp_master:1;
	u32 llp_reserved:1;
	u32 llp_addr:30;
}snx_llp_t;

/**
 * struct snx_ctl - DMA link list control struct
 */
typedef struct snx_ctl {
	u32 reserved:16;
	u32 dst_sel:1;
	u32 src_sel:1;
	u32 dstaddr_ctl:2;
	u32 srcaddr_ctl:2;
	u32 dst_width:3;
	u32 src_width:3;
	u32 tc_mask:1;
	u32 ff_th:3;
}snx_ctl_t;

/**
 * struct snx_lld - DMA linked list descriptor
 */
struct snx_lld {
	dma_addr_t  srcaddr;
	dma_addr_t  dstaddr;
	snx_llp_t llp; 
	snx_ctl_t ctrl;
	u32         size;
};	

/**
 * struct snx_desc - software descriptor
 */
 struct snx_desc {
 	struct snx_lld lld;
	struct list_head tx_list;
	struct dma_async_tx_descriptor txd;
	struct list_head desc_node;
	size_t tot_size;

	u32 lld_cnt;
	u32 flag;
#define SNX_HDMA_MEMSET (1 << 0)
};
 	
static inline struct snx_desc *
txd_to_snx_desc(struct dma_async_tx_descriptor *txd)
{
	return container_of(txd, struct snx_desc, txd);
}


/**
 * struct snx_hdma_chan - internal representation of an snx hdma channel
 * @chan_common: common dmaengine channel object members
 * @hdmac: AHB dmac
 * @chan_regs_base: memory mapped register base
 * @cham_mask: channel index in a mask
 * @error_status: transmit error status information from irq handler to tasklet
 * @tasklet: bottorn half to finish transaction work
 * @lock: serialize enqueue/dequeue operations to descriptors lists
 * @complete_cookie: identifier for the most recently completed operation
 * @active_list: list of descriptors dmaengine is being running on
 * @queue: list of descriptors ready to be submitted to engine
 * @free_list: list of descriptors usable by the channel
 * @descs_allocated: records the actual size of the descriptor pool
 */


struct snx_hdma_chan
{
	struct dma_chan        chan_common;
	struct snx_hdma  *device;
	
	void __iomem	*ch_regs;
	u8               mask;
	
	snx_irq_event_t irq_event;
	struct tasklet_struct tasklet;

	spinlock_t lock;

	dma_cookie_t completed_cookie;

	struct list_head active_list;
	struct list_head queue;
	struct list_head free_list;
	unsigned int descs_allocated;

	union {
		snx_ch_csr_t csr;
		u32 u;
	} csr;
	union {
		snx_ch_cfg_t cfg;
		u32 u;
	} cfg;
};

#define channel_readl(snx_chan, offset) \
	__raw_readl((snx_chan)->ch_regs + offset)
	
#define channel_writel(snx_chan, offset, val) \
	__raw_writel((val), (snx_chan)->ch_regs + offset)

static inline struct snx_hdma_chan *
to_snx_hdma_chan(struct dma_chan *dchan)
{
	return container_of(dchan, struct snx_hdma_chan, chan_common);
}

/**
 * struct snx_hdma - internal representation of am SNX AHB DMA Controller
 * @dma_common:  common dmaengine dma_device object members
 * @regs_base: memory mapped register base;
 * @clk: DMA Controller clock
 * @all_chan_mask: all channels availlable in a mask
 * @dma_desc_pool: base of DMA descriptor region
 * @chan: channels table to store snx_adbdma_chan structers
 */

struct snx_hdma
{
	struct dma_device  dma_common;
	void __iomem	   *regs;
	struct clk  *clk;
	u8	        all_chan_mask;
	struct dma_pool	*dma_desc_pool;
	struct snx_hdma_chan chan[0];
};

#define dmac_readl(snx_dmac, offset) \
	__raw_readl((snx_dmac)->regs + offset)

#define dmac_writel(snx_dmac, offset, val) \
	__raw_writel((val), (snx_dmac)->regs + offset)

static inline struct snx_hdma* 
to_snx_hdma(struct dma_device *ddev)
{
	return container_of(ddev, struct snx_hdma, dma_common);
}

static struct device *chan2dev(struct dma_chan *chan)
{
	return &chan->dev->device;
}

static struct device *chan2parent(struct dma_chan *chan)
{
	return chan->dev->device.parent;
}

/*****************************************************************/
//#define SNX_HDMA_DEBUG

#ifdef SNX_HDMA_DEBUG
#define sndma_dbg(dev, format, arg...) \
	dev_printk(KERN_INFO , dev , format , ## arg)
#else 
#define sndma_dbg(dev, format, arg...)
#endif

#ifdef SNX_HDMA_DEBUG
static void dump_regs(struct snx_hdma_chan *snx_chan)
{
	sndma_dbg(chan2dev(&snx_chan->chan_common),
		"SRCADDR 0x%08x DSTADDR 0x%08x SIZE 0x%08x, CSR 0x%08x\n",
		channel_readl(snx_chan, DMA_CHANNEL_SRCADDR_OFFSET),
		channel_readl(snx_chan, DMA_CHANNEL_DSTADDR_OFFSET),
		channel_readl(snx_chan, DMA_CHANNEL_SIZE_OFFSET),
		channel_readl(snx_chan, DMA_CHANNEL_CSR_OFFSET));
}
#else 
static void dump_regs(struct snx_hdma_chan *snx_chan) {}
#endif

static void dump_lld(struct snx_hdma_chan *snx_chan, 
	struct snx_lld *lld)
{
	dev_printk(KERN_CRIT, chan2dev(&snx_chan->chan_common),
			"  desc: srcaddr 0x%08x dstaddr 0x%08x llp 0x%08x "
			"ctrl 0x%08x tot size 0x%08x\n",lld->srcaddr, lld->dstaddr,
			*((u32 *)(&lld->llp)), *((u32 *)(&lld->ctrl)), lld->size);
}

#endif //__SNX_AHB_DMA_H__
