/*
 *  drivers/net/arm/snx_mac.c
 *
 *  Author:	Kelvin Cheung
 *  Created:	Jul 18, 2008
 *  Copyright:	 (C) 2008 SONIX Technology, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
//#define NAPI           // alek add

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/netdevice.h>
#include <linux/mii.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/clk.h>

#include <asm/io.h>
#include <asm/irq.h>

#include <mach/regs-mac.h>

#define TX_DESC_NUM		512
#define RX_DESC_NUM		256

#define TX_BUF_SIZE		1518
#define RX_BUF_SIZE		1518

#define NAPI_WEIGHT		64     // if 10M 16, 100M 64

#define	LINK_POLL_INTERVAL	500	/* 500 ms */

/* Realtek RTL8201 PHY */
#define PHYID_RTL8201		0x00008201

/* Davicom 9161 PHY */
#define PHYID_DM9161		0x0181b880
#define PHYID_DM9161A		0x0181b8a0
#define PHYID_DM9161B		0x0181b8b0
#define PHYID_RTL8201FL		0x1cc816
#define PHYID_LAN8720   0x7c0f1   //alek add
#define PHYID_IP101G		0x02430c54
#define OWNBY_SW		0
#define OWNBY_MAC		1

struct snx_mac_txdesc
{
	/* TXDES0 (status) */
	unsigned int TXPKT_LATECOL:1;	/* Transmission abort due to late collision (when FTS = 1): 0 */
	unsigned int TXPKT_EXSCOL:1;	/* Transmission abort due after 16 collisions (when FTS = 1): 1 */
	unsigned int Res1:29;
	unsigned int TXDMA_OWN:1;	/* Transmit descriptor ownership: 31 */

	/* TXDES1 (control) */
	unsigned int tx_buf_size:11;	/* Transmit buffer size in byte: 10:0 */
	unsigned int Res2:16;
	unsigned int LTS:1;		/* Last transmit segment descriptor: 27 */
	unsigned int FTS:1;		/* First transmit segment descriptor: 28 */
	unsigned int TX2FIC:1;		/* Transmit to FIFO interrupt on completion: 29 */
	unsigned int TXIC:1;		/* Transmit interrupt on completion: 30 */
	unsigned int EDOTR:1;		/* End descriptor of transmit ring: 31 */

	/* RXDES2 */
	unsigned int tx_buf_phys;	/* Physical base address of transmit ring buffer */
	unsigned int tx_buf;		/* Virtual base address of transmit ring buffer */
};

struct snx_mac_rxdesc
{
	/* RXDES0 (status) */
	unsigned int RFL:11;		/* Receive Frame Length: 10:0 */
	unsigned int Res1:5;
	unsigned int MULTICAST:1;	/* Multicast frame: 16 */
	unsigned int BROARDCAST:1;	/* Broadcast frame: 17 */
	unsigned int RX_ERR:1;		/* Receive error: 18 */
	unsigned int CRC_ERR:1;		/* CRC error: 19 */
	unsigned int FTL:1;		/* Frame too long: 20 */
	unsigned int RUNT:1;		/* Runt packet: 21 */
	unsigned int RX_ODD_NB:1;	/* Receive odd nibbles: 22 */
	unsigned int Res2:5;
	unsigned int LRS:1;		/* Last receive segment descriptor: 28 */
	unsigned int FRS:1;		/* First receive segment descriptor: 29 */
	unsigned int Res3:1;
	unsigned int RXDMA_OWN:1;	/* Receive descriptor ownership: 31 */

	/* RXDES1 (control) */
	unsigned int rx_buf_size:11;	/* Receive buffer size in byte: 10:0 */
	unsigned int Res4:20;
	unsigned int EDORR:1;		/* End descriptor of receive ring: 31 */

	/* RXDES2 */
	unsigned int rx_buf_phys;	/* Physical base address of receive ring buffer */
	void	     *rx_buf;		/* Virtual base address of receive ring buffer */
};

struct snx_netdev_priv
{
	struct mii_if_info	mii;
	unsigned int		phy_type;		/* PHY ID */
	unsigned int		speed_mode : 1;		/* speed mode: 1:100 Mbps; 0:10 Mbps */
	unsigned int		maccr;
	struct resource		*res;
	struct clk		*clk;

	struct snx_mac_txdesc	*tx_desc;	/* Virtual base address of transmit ring descriptors */
	dma_addr_t		tx_desc_dma;	/* Physical base address of transmit ring descriptors */
	struct snx_mac_rxdesc	*rx_desc;	/* Virtual base address of transmit ring descriptors */
	dma_addr_t		rx_desc_dma;	/* Physical base address of transmit ring descriptors */

	spinlock_t		rx_lock;
	unsigned int		rx_idx;			/* Index of current receive ring descriptor */
	spinlock_t		tx_lock;
	unsigned int		tx_idx;			/* Index of current transmit ring descriptor */
	unsigned int		tx_pending;
	unsigned int		tx_clean_idx;		/* Index of clean ring descriptor */

	struct timer_list	check_timer;	/* Check link timer */
#ifdef NAPI //alek add
  struct napi_struct napi;    
  struct net_device *netdev;
#endif
  
};
static int snx_mac_tx_complete (struct net_device *dev);
#if 0
static void print_packet ( unsigned char * buf, int length )
{
        int i;
        int remainder;
        int lines;

	printk ("length = %d \n", length );

        lines = length >> 4;
        remainder = length & 15;
    
        for ( i = 0; i < lines ; i ++ ) {
                int cur;
                for ( cur = 0; cur < 8; cur ++ ) {
                        unsigned char a, b;
                        a = * (buf ++ );
                        b = * (buf ++ );
                        printk ("%02x%02x ", a, b );
                }
                printk ("\n");
        }
        for ( i = 0; i < remainder/2 ; i++ ) {
                unsigned char a, b;
        
                a = * (buf ++ );
                b = * (buf ++ );
                printk ("%02x%02x ", a, b );
        }
        printk ("\n");
}
#endif

static inline unsigned int eth_in (struct net_device *dev, int offset)
{
	return ioread32 (dev->base_addr + offset);
}

static inline void eth_out (struct net_device *dev, int offset, int value)
{
	iowrite32 (value, dev->base_addr + offset);
}

/*
 * Read value stored in a PHY register.
 */
static int snx_mac_mdio_read (struct net_device *dev, int phy_id, int reg)
{
	struct snx_netdev_priv *priv = netdev_priv (dev);
	unsigned int data = 0;
	unsigned int flag = 0;

	/* Make sure MDIO no other used */
	while ((flag = eth_in(dev, MAC_PHYCR)) & (PHYCR_MIIRD | PHYCR_MIIWR));
	data = PHYCR_MIIRD | ((reg & priv->mii.reg_num_mask) << 21) |
			     ((phy_id & priv->mii.phy_id_mask) << 16);
	eth_out (dev, MAC_PHYCR, data);
	//udelay (1000);	//alek add 			/* wait for MII operation */

	/* Wait for operation finish */
	while ((data = eth_in(dev, MAC_PHYCR)) & PHYCR_MIIRD);

	data &= 0xffff;
	// udelay (1000); //alek modify ori is 100
	return data;
}

/*
 * Write value to the a PHY register
 */
static void snx_mac_mdio_write (struct net_device *dev, int phy_id, int reg, int data)
{
	struct snx_netdev_priv *priv = netdev_priv (dev);
	unsigned int flag = 0;

	/* Make sure MDIO no other used */
	while ((flag = eth_in(dev, MAC_PHYCR)) & (PHYCR_MIIRD | PHYCR_MIIWR));
	//udelay (1000);   //alek add
	eth_out (dev, MAC_PHYWDATA, data);
	data = PHYCR_MIIWR | ((reg & priv->mii.reg_num_mask) << 21) |
			     ((phy_id & priv->mii.phy_id_mask) << 16);
	eth_out (dev, MAC_PHYCR, data);
	//udelay (1000);	//alek modify ori is 100	/* wait for MII operation */
}

/*
 * Detect PHY address (phy_id)
 */
static void snx_mac_phy_detect (struct net_device *dev)
{
	struct snx_netdev_priv *priv = netdev_priv (dev);
	int phy_addr;
	unsigned int phy_bmcr, phy_id1, phy_id2;
	int phy_detected = 0;

 	/* Detect PHY address (phy_id) and mode */
	for (phy_addr = 0; phy_addr < 32; phy_addr++) {
		/* Read PHY ID registers (phy_type)*/
		phy_id1 = priv->mii.mdio_read (dev, phy_addr, MII_PHYSID1);
		phy_id2 = priv->mii.mdio_read (dev, phy_addr, MII_PHYSID2);
		priv->phy_type = ((phy_id1 & 0xffff) << 16) | (phy_id2 & 0xffff);

		switch (priv->phy_type) {
		case PHYID_IP101G:
			printk (KERN_INFO "%s: IP101G Ethernet PHY ",
				to_platform_device (dev->dev.parent)->name);
			phy_detected = 1;
			break;

		case PHYID_RTL8201:
			printk (KERN_INFO "%s: Realtek RTL8201 (B)L Ethernet PHY ",
				to_platform_device (dev->dev.parent)->name);
			phy_detected = 1;
			break;

		case PHYID_DM9161:
			printk (KERN_INFO "%s: Davicom DM9161 Ethernet PHY ",
				to_platform_device (dev->dev.parent)->name);
			phy_detected = 1;
			break;

		case PHYID_DM9161A:
			printk (KERN_INFO "%s: Davicom DM9161A Ethernet PHY ",
				to_platform_device (dev->dev.parent)->name);
			phy_detected = 1;
			break;

		case PHYID_RTL8201FL:
			printk (KERN_INFO "%s: Realtek 8201FL Ethernet PHY ",
				to_platform_device (dev->dev.parent)->name);
         
			priv->mii.mdio_write (dev, phy_addr, 31, 7);
			priv->mii.mdio_write (dev, phy_addr, 16, 0x1f5e);
			phy_detected = 1;
			break;

		case PHYID_LAN8720:
			printk (KERN_INFO "%s: LAN8720 Ethernet PHY ",
			to_platform_device (dev->dev.parent)->name);
			phy_detected = 1;
			break;
		}

		if (phy_detected)
			break;
	}

//	printk(KERN_INFO "PHY_ID = %d\n", phy_addr);
	priv->mii.phy_id = phy_addr; 

	/* Read PHY basic mode control register */
	phy_bmcr = priv->mii.mdio_read (dev, phy_addr, MII_BMCR);
	if (phy_bmcr & BMCR_SPEED100)
		priv->speed_mode = 1;
	if (phy_bmcr & BMCR_ANENABLE)
		priv->mii.force_media = 0;
	if (phy_bmcr & BMCR_FULLDPLX)
		priv->mii.full_duplex = 1;
	printk ("%s Mbps %s (%s)\n",
		priv->speed_mode ? "100" : "10",
		priv->mii.full_duplex ? "FullDuplex" : "HalfDuplex",
		priv->mii.force_media ? "Force Media" : "Auto Negotiation");
}

/*
 * Get the link status from the PHY.
 */
static void snx_mac_check_link (unsigned long dev_id)
{
	struct net_device *dev = (struct net_device *) dev_id;
	struct snx_netdev_priv *priv = netdev_priv (dev);
        unsigned int linkstat, carrier;

	linkstat = priv->mii.mdio_read (dev, priv->mii.phy_id, MII_BMSR) & BMSR_LSTATUS;
	carrier = netif_carrier_ok (dev);

#if 0
	printk ("net queue stat:%d TX:%d %d %d\n", netif_queue_stopped (dev), \
		priv->tx_idx, priv->tx_pending, priv->tx_clean_idx);
#endif

//	printk ("%s: linkstat = %x, carrier = %x\n", __func__, linkstat, carrier);

	if (linkstat && !carrier) {
		netif_carrier_on (dev);
		printk (KERN_INFO "%s: link up\n", dev->name);
	} else if (!linkstat && carrier) {
		netif_carrier_off (dev);
		printk (KERN_INFO "%s: link down\n", dev->name);
	}

	mod_timer (&priv->check_timer, jiffies + msecs_to_jiffies (LINK_POLL_INTERVAL));
}

/*
 * Finds the CRC32 of a set of bytes.
 * Again, from Peter Cammaert's code.
 */
static int crc32 (char *s, int length) 
{
	/* indices */
	int perByte;
	int perBit;
	/* crc polynomial for Ethernet */
	const unsigned long poly = 0xedb88320;
	/* crc value - preinitialized to all 1's */
	unsigned long crc_value = 0xffffffff;

	for (perByte = 0; perByte < length; perByte++) {
		unsigned char c;

		c = * (s++);
		for (perBit = 0; perBit < 8; perBit++) {
			crc_value = (crc_value >> 1) ^
				 (((crc_value ^ c) & 0x01) ? poly : 0);
			c >>= 1;
		}
	}
	return crc_value;
}

/*
 .    This sets the internal hardware table to filter out unwanted multicast
 .    packets before they take up memory.
 */
/*
static void snx_mac_set_hashtable (struct net_device *dev) 
{
        struct dev_mc_list *mc_addr;
        int crc_val;
    
        for (mc_addr = dev->mc_list; mc_addr != NULL; mc_addr = mc_addr->next) {
                if ( ! (*mc_addr->dmi_addr & 1) )	//check whether the address is a multicast
                        continue;
                crc_val = crc32 (mc_addr->dmi_addr, 6);
                crc_val = (crc_val >> 26) & 0x3f;
                if (crc_val >= 32)
                        eth_out (dev, MAC_MAHT1, eth_in (dev, MAC_MAHT1) | (1UL << (crc_val-32)));
                else
                        eth_out (dev, MAC_MAHT0, eth_in (dev, MAC_MAHT0) | (1UL << crc_val));
        }
}
*/


static void snx_mac_set_hashtable (struct net_device *dev) 
{
  struct netdev_hw_addr *mc_addr;
  int crc_val;
  netdev_for_each_mc_addr (mc_addr, dev) {	
    crc_val = crc32 (mc_addr->addr, 6);
    crc_val = (crc_val >> 26) & 0x3f;
    if (crc_val >= 32)
      eth_out (dev, MAC_MAHT1, eth_in (dev, MAC_MAHT1) | (1UL << (crc_val-32)));
    else
      eth_out (dev, MAC_MAHT0, eth_in (dev, MAC_MAHT0) | (1UL << crc_val));
  }
}

/*
 . This routine will, depending on the values passed to it,
 . either make it accept multicast packets, go into
 . promiscuous mode ( for TCPDUMP and cousins ) or accept
 . a select set of multicast packets
*/
static void snx_mac_set_multicast (struct net_device *dev)
{
	struct snx_netdev_priv *priv = netdev_priv (dev);
        
	priv->maccr = eth_in (dev, MAC_MACCR);

  if (dev->flags & IFF_PROMISC)			/* Enable promiscuous mode */
    priv->maccr |= MACCR_RX_ALL;
  else {
		priv->maccr &= ~MACCR_RX_ALL;		/* Disable promiscuous mode */
		if (dev->flags & IFF_ALLMULTI)			/* Enable all multicast mode */
			priv->maccr |= MACCR_RX_MUL;
		else {
			priv->maccr &= ~MACCR_RX_MUL;		/* Disable all multicast mode */
			//if (dev->mc_count && IFF_MULTICAST) {	/* Enable specific multicast mode */
			if (!netdev_mc_empty (dev)) {	/* Enable specific multicast mode */
				priv->maccr |= MACCR_MUL_HT;
				snx_mac_set_hashtable (dev);
			}
			else
				priv->maccr &= ~MACCR_MUL_HT;	/* Disable specific multicast mode */
			if (dev->flags & IFF_BROADCAST)	/* Enable broadcast mode */
				priv->maccr |= MACCR_RX_BRO;
			else					/* Disable broadcast mode */
				priv->maccr &= ~MACCR_RX_BRO;
		}
	}
	eth_out (dev, MAC_MACCR, priv->maccr);
}

/*
 * Program the hardware MAC address from dev->dev_addr.
 */
static void update_mac_address (struct net_device *dev)
{
	 eth_out (dev, MAC_MADR, (dev->dev_addr[0] <<  8) |  dev->dev_addr[1]);
	 eth_out (dev, MAC_LADR, (dev->dev_addr[2] << 24) | (dev->dev_addr[3] << 16) |
				   (dev->dev_addr[4] <<  8) | (dev->dev_addr[5]));		
}

/*
 * Store the new hardware address in dev->dev_addr, and update the MAC.
 */
static int snx_mac_set_mac (struct net_device *dev, void* addr)
{
	struct sockaddr *address = addr;

	if (!is_valid_ether_addr (address->sa_data))
		return -EADDRNOTAVAIL;

	memcpy (dev->dev_addr, address->sa_data, dev->addr_len);
	update_mac_address (dev);

	printk (KERN_INFO "%s: Setting MAC address to "
		"%02x:%02x:%02x:%02x:%02x:%02x\n", dev->name,
		dev->dev_addr[0], dev->dev_addr[1], dev->dev_addr[2],
		dev->dev_addr[3], dev->dev_addr[4], dev->dev_addr[5]);

	return 0;
}
#ifdef NAPI
static int snx_mac_rx (struct net_device *dev, int budget , bool *process)
{
	int received = 0;
	struct sk_buff *skb;
	int seg_len, pkt_len, acc_len;
	struct snx_mac_rxdesc *cur_rx_desc;
	struct snx_netdev_priv *priv = netdev_priv (dev);
  
	while (received < budget) {
		cur_rx_desc = &priv->rx_desc[priv->rx_idx];

		/* check ownership */
		if (cur_rx_desc->RXDMA_OWN == OWNBY_MAC)
		{
      *process = false;
    	break;
    }
		/* is first receive segment ? */
		if (!cur_rx_desc->FRS) { 
			dev->stats.rx_dropped++;
			goto next;
		}

		/* check error flag */
		if (cur_rx_desc->RX_ERR ||
		cur_rx_desc->CRC_ERR || cur_rx_desc->FTL ||
		cur_rx_desc->RUNT || cur_rx_desc->RX_ODD_NB) {
			if (cur_rx_desc->RX_ERR) {
			}
			if (cur_rx_desc->CRC_ERR) {
				dev->stats.rx_crc_errors++;
			}
			if (cur_rx_desc->FTL || cur_rx_desc->RUNT) {
				dev->stats.rx_length_errors++;
			}
			if (cur_rx_desc->RX_ODD_NB) {
			}
			dev->stats.rx_errors++;
			goto next;
		}

		/* check receive frame length */
		if (!cur_rx_desc->RFL) {
			goto next;
		}
		pkt_len = cur_rx_desc->RFL;

		/* allocate sokect buffer */
		skb = dev_alloc_skb (pkt_len + 2);
		if (skb == NULL) {
			dev->stats.rx_dropped++;
			goto next;
		}
//		skb_reserve (skb, 2);	/* 16 bit alignment */ /* we need this below, not here */
		skb_put (skb, pkt_len);

		for (acc_len = 0; ; acc_len += seg_len) {
//			printk ("rx_idx = %d\n", priv->rx_idx);//////////////kelvin
			cur_rx_desc = &priv->rx_desc[priv->rx_idx];

			/* copy data from receive buffer */
			seg_len = min (pkt_len - acc_len, RX_BUF_SIZE);
			dma_sync_single_for_cpu (NULL, cur_rx_desc->rx_buf_phys,
					cur_rx_desc->FRS ? seg_len + 2 : seg_len, DMA_FROM_DEVICE);
//			eth_copy_and_sum (skb, cur_rx_desc->rx_buf,
//					cur_rx_desc->FRS ? seg_len + 2 : seg_len, 0);	/* for zero copy */
			memcpy (skb->data + acc_len, cur_rx_desc->rx_buf,
				cur_rx_desc->FRS ? seg_len + 2 : seg_len);		/* for zero copy */

			if (cur_rx_desc->FRS) { 
				skb_reserve (skb, 2);					/* for zero copy */
			}

			if (cur_rx_desc->MULTICAST)
				dev->stats.multicast++;

			/* is last receive segment ? */
			if (cur_rx_desc->LRS) 
				break;

			/* reset ownership */
			cur_rx_desc->RXDMA_OWN = OWNBY_MAC;
			/* next descriptor */
			priv->rx_idx = (priv->rx_idx + 1) % RX_DESC_NUM;
		}

		skb->dev = dev;
		skb->protocol = eth_type_trans (skb, dev);
    skb->ip_summed = CHECKSUM_UNNECESSARY;
		netif_receive_skb (skb);
		dev->last_rx = jiffies;
		dev->stats.rx_packets++;
		dev->stats.rx_bytes += pkt_len;
		received++;
   
//		print_packet (skb->data, skb->len);	///////////////////////////////////////kelvin
next:
		/* reset ownership of last descriptor */
    *process = true;
		cur_rx_desc->RXDMA_OWN = OWNBY_MAC;
		/* next descriptor */
		priv->rx_idx = (priv->rx_idx + 1) % RX_DESC_NUM;
	}
  
	return received;
}
#else
static int snx_mac_rx (struct net_device *dev, int budget)
{
	int received = 0;
	struct sk_buff *skb;
	int seg_len, pkt_len, acc_len;
	struct snx_mac_rxdesc *cur_rx_desc;
	struct snx_netdev_priv *priv = netdev_priv (dev);

	while (received < budget) {
		cur_rx_desc = &priv->rx_desc[priv->rx_idx];

		/* check ownership */
		if (cur_rx_desc->RXDMA_OWN == OWNBY_MAC)
			break;

		/* is first receive segment ? */
		if (!cur_rx_desc->FRS) { 
			dev->stats.rx_dropped++;
			goto next;
		}

		/* check error flag */
		if (cur_rx_desc->RX_ERR ||
		cur_rx_desc->CRC_ERR || cur_rx_desc->FTL ||
		cur_rx_desc->RUNT || cur_rx_desc->RX_ODD_NB) {
			if (cur_rx_desc->RX_ERR) {
			}
			if (cur_rx_desc->CRC_ERR) {
				dev->stats.rx_crc_errors++;
			}
			if (cur_rx_desc->FTL || cur_rx_desc->RUNT) {
				dev->stats.rx_length_errors++;
			}
			if (cur_rx_desc->RX_ODD_NB) {
			}
			dev->stats.rx_errors++;
			goto next;
		}

		/* check receive frame length */
		if (!cur_rx_desc->RFL) {
			goto next;
		}
		pkt_len = cur_rx_desc->RFL;

		/* allocate sokect buffer */
		skb = dev_alloc_skb (pkt_len + 2);
		if (skb == NULL) {
			dev->stats.rx_dropped++;
			goto next;
		}
//		skb_reserve (skb, 2);	/* 16 bit alignment */ /* we need this below, not here */
		skb_put (skb, pkt_len);

		for (acc_len = 0; ; acc_len += seg_len) {
//			printk ("rx_idx = %d\n", priv->rx_idx);//////////////kelvin
			cur_rx_desc = &priv->rx_desc[priv->rx_idx];

			/* copy data from receive buffer */
			seg_len = min (pkt_len - acc_len, RX_BUF_SIZE);
			dma_sync_single_for_cpu (NULL, cur_rx_desc->rx_buf_phys,
					cur_rx_desc->FRS ? seg_len + 2 : seg_len, DMA_FROM_DEVICE);
//			eth_copy_and_sum (skb, cur_rx_desc->rx_buf,
//					cur_rx_desc->FRS ? seg_len + 2 : seg_len, 0);	/* for zero copy */
			memcpy (skb->data + acc_len, cur_rx_desc->rx_buf,
				cur_rx_desc->FRS ? seg_len + 2 : seg_len);		/* for zero copy */

			if (cur_rx_desc->FRS) { 
				skb_reserve (skb, 2);					/* for zero copy */
			}

			if (cur_rx_desc->MULTICAST)
				dev->stats.multicast++;

			/* is last receive segment ? */
			if (cur_rx_desc->LRS) 
				break;

			/* reset ownership */
			cur_rx_desc->RXDMA_OWN = OWNBY_MAC;
			/* next descriptor */
			priv->rx_idx = (priv->rx_idx + 1) % RX_DESC_NUM;
		}

		skb->dev = dev;
		//skb->ip_summed = CHECKSUM_UNNECESSARY;
		skb->protocol = eth_type_trans (skb, dev);
#ifdef NAPI
    
		netif_receive_skb (skb);
#else
		netif_rx (skb);
#endif
		dev->last_rx = jiffies;
		dev->stats.rx_packets++;
		dev->stats.rx_bytes += pkt_len;
		received++;
//		print_packet (skb->data, skb->len);	///////////////////////////////////////kelvin
next:
		/* reset ownership of last descriptor */
		cur_rx_desc->RXDMA_OWN = OWNBY_MAC;
		/* next descriptor */
		priv->rx_idx = (priv->rx_idx + 1) % RX_DESC_NUM;
	}

	return received;
}
#endif

#ifdef NAPI      //alek add
static int snx_mac_poll (struct napi_struct *napi, int budget)
{
	struct snx_netdev_priv *priv = container_of (napi, struct snx_netdev_priv, napi);
	//struct net_device *netdev = priv->netdev;

	unsigned int status;
	bool completed = true;
	int rx = 0;
  int process = 0;
	status = eth_in (priv->netdev, MAC_ISR);// (priv->base + FTMAC100_OFFSET_ISR);
  // (FTMAC100_INT_RPKT_FINISH | FTMAC100_INT_NORXBUF)
	if (status & (RPKT_FINISH | NORXBUF) ) {
		/*
		 * FTMAC100_INT_RPKT_FINISH:
		 *	RX DMA has received packets into RX buffer successfully
		 *
		 * FTMAC100_INT_NORXBUF:
		 *	RX buffer unavailable
		 */
		bool retry;
#if 0
		do {
			retry = ftmac100_rx_packet (priv, &rx);
		} while (retry && rx < budget);

		if (retry && rx == budget)
			completed = false;
#endif

    rx = snx_mac_rx (priv->netdev,budget,&process); 
    if (process && rx == budget)
			completed = false;    
	}

	if (status & (XPKT_OK | XPKT_LOST)) {
		/*
		 * FTMAC100_INT_XPKT_OK:
		 *	packet transmitted to ethernet successfully
		 *
		 * FTMAC100_INT_XPKT_LOST:
		 *	packet transmitted to ethernet lost due to late
		 *	collision or excessive collision
		 */
		//ftmac100_tx_complete (priv);
    snx_mac_tx_complete (priv->netdev);
	}

	if (status & (NORXBUF | RPKT_LOST |
		      AHB_ERR | PHYSTS_CHG)) {
		if (net_ratelimit ())
			netdev_info (priv->netdev, "[ISR] = 0x%x: %s%s%s%s\n", status,
				    status & NORXBUF ? "NORXBUF " : "",
				    status & RPKT_LOST ? "RPKT_LOST " : "",
				    status & AHB_ERR ? "AHB_ERR " : "",
				    status & PHYSTS_CHG ? "PHYSTS_CHG" : "");

		if (status & NORXBUF) {
       //FTMAC100_INT_NORXBUF
			/* RX buffer unavailable */
			priv->netdev->stats.rx_over_errors++;
		}

		if (status & RPKT_LOST) {
       //FTMAC100_INT_RPKT_LOST
			/* received packet lost due to RX FIFO full */
			priv->netdev->stats.rx_fifo_errors++;
		}

		if (status & PHYSTS_CHG) {
       //FTMAC100_INT_PHYSTS_CHG
			/* PHY link status change */
			mii_check_link (&priv->mii);
		}
	}

	if (completed) {
		/* stop polling */
		napi_complete (napi);
    /* enable all interrupts */
	  eth_out (priv->netdev, MAC_IER, MAC_ALLINTRS);
		//ftmac100_enable_all_int (priv);
	}

	return rx;
}

/*
static int snx_mac_poll (struct net_device *dev, int *budget)
{
	struct snx_netdev_priv *priv = netdev_priv (dev);
	unsigned int work_to_do = *budget;//min (dev->quota, *budget);
	unsigned int work_done = 0;
	int done = 1;
	unsigned long flags;

	spin_lock_irqsave (&priv->rx_lock, flags);
	work_done = snx_mac_rx (dev, work_to_do);
	if (likely (work_done > 0)) {
		*budget -= work_done;
		//dev->quota -= work_done;
		done = (work_done < work_to_do);
	}

	if (done) {
		netif_rx_complete (dev);
		eth_out (dev, MAC_IER, eth_in (dev, MAC_IER) | RPKT_FINISH);
	}
	spin_unlock_irqrestore (&priv->rx_lock, flags);

	return !done;
}
*/
#endif

#ifdef CONFIG_NET_POLL_CONTROLLER
static void snx_mac_poll_controller (struct net_device *dev)
{     
	unsigned long flags;

	local_irq_save (flags);
	snx_mac_irq (dev->irq, dev);
	local_irq_restore (flags);
}
#endif

static int snx_mac_xmit (struct sk_buff *skb, struct net_device *dev)
{
	struct snx_netdev_priv *priv = netdev_priv (dev);
	struct snx_mac_txdesc *cur_tx_desc;
	int pkt_len;
	unsigned long flags;

	spin_lock_irqsave (&priv->tx_lock, flags);
	cur_tx_desc = &priv->tx_desc[priv->tx_idx];

	pkt_len = max (ETH_ZLEN, (int)skb->len);
	pkt_len = min (pkt_len, TX_BUF_SIZE);

#ifdef CONFIG_SNX_MAC_ZERO_COPY
	dma_addr_t map;
	map = dma_map_single(&dev->dev, skb->data, skb_headlen(skb), DMA_TO_DEVICE);
	cur_tx_desc->tx_buf_phys = cpu_to_le32(map);
	cur_tx_desc->tx_buf = (unsigned int)skb;
#else
	/* copy data to transmit buffer */
	skb_copy_and_csum_dev (skb, cur_tx_desc->tx_buf);
	dma_sync_single_for_cpu (NULL, cur_tx_desc->tx_buf_phys,
				skb->len, DMA_TO_DEVICE);
#endif
	/* set transmit descriptor */
	cur_tx_desc->tx_buf_size = pkt_len;
	cur_tx_desc->LTS = 1;
	cur_tx_desc->FTS = 1;
	cur_tx_desc->TXDMA_OWN = OWNBY_MAC;
	
	//spin_lock(&priv->tx_lock);
	priv->tx_pending++;
	if (priv->tx_pending == TX_DESC_NUM)
		netif_stop_queue (dev);

	/*memory barries, prevent compiler disorder the calling code*/
	wmb();
	
	/* trigger MAC to poll transmit descriptor */
	eth_out (dev, MAC_TXPD, 0x1);

	priv->tx_idx = (priv->tx_idx + 1) % TX_DESC_NUM;
	dev->trans_start = jiffies;

	spin_unlock_irqrestore (&priv->tx_lock, flags);

#ifndef CONFIG_SNX_MAC_ZERO_COPY
	//free buffer
	dev_kfree_skb (skb);
#endif

	return NETDEV_TX_OK;
}

static int snx_mac_tx_complete (struct net_device *dev)
{
	unsigned long flags;
	struct snx_mac_txdesc *tx_desc;
	struct snx_netdev_priv *priv = netdev_priv (dev);

	if (priv->tx_pending == 0)
		return 0;

	tx_desc = &priv->tx_desc[priv->tx_clean_idx];
	if (tx_desc->TXDMA_OWN)
		return 0;

	if (unlikely (tx_desc->TXPKT_LATECOL || tx_desc->TXPKT_EXSCOL)){
			dev->stats.tx_aborted_errors++;
	}else{
		dev->stats.tx_packets++;
		dev->stats.tx_bytes += priv->tx_desc[priv->tx_clean_idx].tx_buf_size;
	}

#ifdef CONFIG_SNX_MAC_ZERO_COPY
	struct sk_buff *skb;
	dma_addr_t map;
	skb = (struct sk_buff *)tx_desc->tx_buf;

	map = le32_to_cpu(tx_desc->tx_buf_phys);
	dma_unmap_single(NULL, map, skb_headlen(skb), DMA_TO_DEVICE);

	if (skb)
		dev_kfree_skb_any(skb);
#endif
	//we should clean desc
	tx_desc->TXPKT_LATECOL = 0x0;
	tx_desc->TXPKT_EXSCOL = 0x0;
	tx_desc->tx_buf_size = 0x0;
	tx_desc->LTS = 0x0;
	tx_desc->FTS = 0x0;

	//forword clean index
	priv->tx_clean_idx = (priv->tx_clean_idx + 1) % TX_DESC_NUM;

	spin_lock_irqsave (&priv->tx_lock, flags);
	priv->tx_pending--;
	spin_unlock_irqrestore (&priv->tx_lock, flags);

	netif_wake_queue (dev);
	
	return 1;
}

static irqreturn_t snx_mac_irq (int irq, void *dev_id)
{
#ifdef NAPI
  struct net_device *netdev = dev_id;
	struct snx_netdev_priv *priv = netdev_priv (netdev);

	if (likely (netif_running (netdev))) {
		/* Disable interrupts for polling */
		//ftmac100_disable_all_int (priv);
    eth_out (netdev, MAC_IER, 0);
		napi_schedule (&priv->napi);
	}
	return IRQ_HANDLED;
  
#else
	u32 isr, macsr;
	struct net_device *dev = dev_id;
	//struct snx_netdev_priv *priv = netdev_priv (dev);
	//unsigned long flags;

	//spin_lock_irqsave (&priv->tx_lock, flags);

	isr = eth_in (dev, MAC_ISR);
	macsr = eth_in (dev, MAC_MACSR);
	if (isr == 0)
		return IRQ_NONE;

	if (isr & (RPKT_FINISH |NORXBUF)) {
#ifdef NAPI 
		if (likely (netif_rx_schedule_prep (dev))) {
			eth_out (dev, MAC_IER, eth_in (dev, MAC_IER) & ~RPKT_FINISH);
			__netif_rx_schedule (dev);
		}
#else	
		snx_mac_rx (dev, NAPI_WEIGHT);
#endif
	}

	if (isr & (XPKT_OK|XPKT_LOST)) {
		while (snx_mac_tx_complete (dev))
			;
	}

	if (isr & NORXBUF) {
		dev->stats.rx_dropped++;
	}
#if 0
	if (isr & NOTXBUF) {		/* design bug of IP */
		printk ("%s: No tx buffer\n", __func__);
                priv->stats.tx_dropped++;
	}
#endif
	if (isr & RPKT_LOST) {
		dev->stats.rx_fifo_errors++;
	}

	if (isr & XPKT_LOST) {
              dev->stats.tx_errors++;
		dev->stats.collisions++;
	}

	//spin_unlock_irqrestore (&priv->tx_lock, flags);

	return IRQ_HANDLED;
  
#endif  
}

static void snx_mac_free_ring (struct snx_netdev_priv *priv)
{
	int i;

	for (i = 0; i < RX_DESC_NUM; i += 2) {
		dma_addr_t d;

		d = priv->rx_desc[i].rx_buf_phys;
		if (d)
			dma_unmap_single (NULL, d, PAGE_SIZE, DMA_FROM_DEVICE);

		if (priv->rx_desc[i].rx_buf != NULL)
			free_page ((unsigned long)priv->rx_desc[i].rx_buf);
	}
#ifndef CONFIG_SNX_MAC_ZERO_COPY
	for (i = 0; i < TX_DESC_NUM; i += 2) {
		dma_addr_t d;

		d = priv->tx_desc[i].tx_buf_phys;
		if (d)
			dma_unmap_single (NULL, d, PAGE_SIZE, DMA_TO_DEVICE);

		if (priv->tx_desc[i].tx_buf != NULL)
			free_page ((unsigned long)priv->tx_desc[i].tx_buf);
	}
#endif
	if (priv->rx_desc)
		dma_free_coherent (NULL, sizeof (struct snx_mac_rxdesc)*RX_DESC_NUM,
				priv->rx_desc,priv->rx_desc_dma);
	if (priv->tx_desc)
		dma_free_coherent (NULL, sizeof (struct snx_mac_txdesc)*TX_DESC_NUM,
				priv->tx_desc,priv->tx_desc_dma);
}

/*
 * The hardware enforces a sub-2K maximum packet size, so we put
 * two buffers on every hardware page.
 */
static int snx_mac_alloc_ring (struct snx_netdev_priv *priv)
{
	int i, ret = 0;

	/* allocate transmit ring descriptor */
	priv->tx_desc = dma_alloc_coherent (NULL, sizeof (struct snx_mac_txdesc)*TX_DESC_NUM,
					&priv->tx_desc_dma, GFP_KERNEL | GFP_DMA);
	if (priv->tx_desc == NULL) {
		ret = 1;
		goto out;
	}
        memset (priv->tx_desc, 0, sizeof (struct snx_mac_txdesc)*TX_DESC_NUM);

	/* allocate receive ring descriptor */
	priv->rx_desc = dma_alloc_coherent (NULL, sizeof (struct snx_mac_rxdesc)*RX_DESC_NUM,
					&priv->rx_desc_dma, GFP_KERNEL | GFP_DMA);
	if (priv->rx_desc == NULL) {
		ret = 1;
		goto out;
	}
        memset (priv->rx_desc, 0, sizeof (struct snx_mac_rxdesc)*RX_DESC_NUM);

	/* allocate transmit ring buffer, and initialize transmit ring descriptor */
	for (i = 0; i < TX_DESC_NUM; i += 2) {
#ifndef CONFIG_SNX_MAC_ZERO_COPY
		void *page;
		dma_addr_t d;

		page = (void *)__get_free_page (GFP_KERNEL | GFP_DMA);
		if (page == NULL) {
			ret = 1;
			goto err;
		}

		d = dma_map_single (NULL, page, PAGE_SIZE, DMA_TO_DEVICE);
		if (dma_mapping_error (NULL, d)) {
			free_page ((unsigned long)page);
			ret = 1;
			goto err;
		}

		/* initialize transmit ring descriptor */
		priv->tx_desc[i].tx_buf = page;	/* set physical base address of transmit ring buffer */
		priv->tx_desc[i].tx_buf_phys = d;	/* set virtual base address of transmit ring buffer */

		priv->tx_desc[i + 1].tx_buf = page + PAGE_SIZE / 2;
		priv->tx_desc[i + 1].tx_buf_phys = d + PAGE_SIZE / 2;
#endif
		priv->tx_desc[i].TX2FIC = 0;
		priv->tx_desc[i].TXIC = 0;
		priv->tx_desc[i].EDOTR = 0;
		priv->tx_desc[i].TXDMA_OWN = OWNBY_SW;	/* initialize TXDMA ownership */

		priv->tx_desc[i + 1].TX2FIC = 0;
		priv->tx_desc[i + 1].TXIC = 0;
		priv->tx_desc[i + 1].EDOTR = 0;
		priv->tx_desc[i + 1].TXDMA_OWN = OWNBY_SW;
	}
	priv->tx_desc[TX_DESC_NUM - 1].EDOTR = 1;	/* last descriptor of transmit ring */

	/* allocate receive ring buffer , and initialize receive ring descriptor */
	for (i = 0; i < RX_DESC_NUM; i += 2) {
		void *page;
		dma_addr_t d;

		page = (void *)__get_free_page (GFP_KERNEL | GFP_DMA);
		if (page == NULL) {
			ret = 1;
			goto err;
		}

		d = dma_map_single (NULL, page, PAGE_SIZE, DMA_FROM_DEVICE);
		if (dma_mapping_error (NULL, d)) {
			free_page ((unsigned long)page);
			ret = 1;
			goto err;
		}

		/* initialize receive ring descriptor */
		priv->rx_desc[i].rx_buf = page;	/* set physical base address of receive ring buffer */
		priv->rx_desc[i].rx_buf_phys = d;	/* set virtual base address of receive ring buffer */
		priv->rx_desc[i].rx_buf_size = RX_BUF_SIZE; /* set receive buffer size */
		priv->rx_desc[i].EDORR = 0;
		priv->rx_desc[i].RXDMA_OWN = OWNBY_MAC;	/* initialize RXDMA ownership */

		priv->rx_desc[i + 1].rx_buf = page + PAGE_SIZE / 2;
		priv->rx_desc[i + 1].rx_buf_phys = d + PAGE_SIZE / 2;
		priv->rx_desc[i + 1].rx_buf_size = RX_BUF_SIZE;
		priv->rx_desc[i + 1].EDORR = 0;
		priv->rx_desc[i + 1].RXDMA_OWN = OWNBY_MAC;
	}
	priv->rx_desc[RX_DESC_NUM - 1].EDORR = 1;	/* last descriptor of receive ring */

	goto out;

err:
	snx_mac_free_ring (priv);
out:
	return ret;
}

static int snx_mac_start_hw (struct net_device *dev)
{
	struct snx_netdev_priv *priv = netdev_priv (dev);
	int i;

	/* reset MAC controller */
	eth_out (dev, MAC_MACCR, MACCR_RST);
	for (i = 0; i < 10; i++) {
		if ((eth_in (dev, MAC_MACCR) & MACCR_RST) == 0)
			break;
		msleep (1);
	}
	if (i == 10) {
		printk (KERN_CRIT "%s: hw failed to reset\n", dev->name);
		return 1;
	}

	/* 
 	 * Add delay that refer Faraday FTMAC100 source code on Linux 3.10 
 	 * to fix MAC_TXBADR can't write because MAC reset not complete.
 	 */
	udelay(500);

	/* disable all interrupts */
	eth_out (dev, MAC_IER, 0);

	/* set transmit ring descriptor base address */
	eth_out (dev, MAC_TXRBADR, priv->tx_desc_dma);
	
	/* set receive ring descriptor base address  */
	eth_out (dev, MAC_RXRBADR, priv->rx_desc_dma);

	/* set MAC address */
	update_mac_address (dev);

	/* set interrupt timer control register */
	eth_out (dev, MAC_ITC, 0x00001010);	/* recommended value */

	/* set automatic polling timer control register */
	eth_out (dev, MAC_APTC, 0x00000001);	/* recommended value */

	/* set DMA burst length and arbitration control register */
	eth_out (dev, MAC_DBLAC, 0x00000390);	/* recommended value */

	/* enable flow control */
	eth_out (dev, MAC_FCR, eth_in (dev, MAC_FCR)|FC_EN);

	/* enable back pressure */
	eth_out (dev, MAC_BPR, eth_in (dev, MAC_BPR)|BP_EN);

	/* enable all interrupts */
	eth_out (dev, MAC_IER, MAC_ALLINTRS);

	/* enable MAC controller */
	priv->maccr = MACCR_CRC_APD | MACCR_RX_EN | MACCR_TX_EN	| MACCR_RXDMA_EN | MACCR_TXDMA_EN;
	if (priv->speed_mode)
		priv->maccr |= MACCR_SPD;  
	if (priv->mii.full_duplex)
		priv->maccr |= MACCR_FD; 
	eth_out (dev, MAC_MACCR, priv->maccr);

	return 0;
}

static void snx_mac_stop_hw (struct net_device *dev)
{
	int i;

	/* disable all interrupts */
	eth_out (dev, MAC_IER, 0);

	/* reset MAC controller */
	eth_out (dev, MAC_MACCR, MACCR_RST);
	for (i = 0; i < 10; i++) {
		if ((eth_in (dev, MAC_MACCR) & MACCR_RST) == 0)
			break;
		msleep (1);
	}
	if (i == 10)
		printk (KERN_CRIT "%s: hw failed to reset\n", dev->name);
}

static int snx_mac_open (struct net_device *dev)
{
	struct snx_netdev_priv *priv = netdev_priv (dev);
	int err;

	err = snx_mac_alloc_ring (priv);
	if (err)
		return -ENOMEM;
#ifdef NAPI
  err = request_irq (dev->irq, snx_mac_irq, 0,
			to_platform_device (dev->dev.parent)->name, dev);
#else
	err = request_irq (dev->irq, snx_mac_irq, IRQF_SHARED,
			to_platform_device (dev->dev.parent)->name, dev);
#endif      
	if (err) {
		snx_mac_free_ring (priv);
		return err;
	}

	err = snx_mac_start_hw (dev);  
	if (err) {
		free_irq (dev->irq, dev);
		snx_mac_free_ring (priv);
		return -EIO;
	}

	spin_lock_init (&priv->rx_lock);
	priv->rx_idx = 0;
	spin_lock_init (&priv->tx_lock);
	priv->tx_idx = 0;
	priv->tx_pending = 0;
	priv->tx_clean_idx = 0;
#ifdef NAPI
  napi_enable (&priv->napi);
#endif
	netif_start_queue (dev);
  
  eth_out (dev, MAC_IER, MAC_ALLINTRS);
	return 0;
}

static int snx_mac_close (struct net_device *dev)
{
	struct snx_netdev_priv *priv = netdev_priv (dev);

	netif_stop_queue (dev);
#ifdef NAPI   
  napi_disable (&priv->napi);
#endif  
	free_irq (dev->irq, dev);
	snx_mac_stop_hw (dev);
	snx_mac_free_ring (priv);

	return 0;
}

static int snx_mac_ioctl (struct net_device *dev, struct ifreq *ifr, int cmd)
{
	struct snx_netdev_priv *priv = netdev_priv (dev);
	struct mii_ioctl_data *data = if_mii (ifr);

	return generic_mii_ioctl (&priv->mii, data, cmd, NULL);
}

static int snx_miitool_get_settings (struct net_device *dev, struct ethtool_cmd *cmd)
{
	struct snx_netdev_priv *priv = netdev_priv (dev);
	return mii_ethtool_gset (&priv->mii, cmd);
}

static int snx_miitool_set_settings (struct net_device *dev, struct ethtool_cmd *cmd)
{
	struct snx_netdev_priv *priv = netdev_priv (dev);
	return mii_ethtool_sset (&priv->mii, cmd);
}

static void snx_miitool_get_drvinfo (struct net_device *dev, struct ethtool_drvinfo *info)
{
	strncpy (info->driver, "snx_eth", sizeof (info->driver) - 1);
	strncpy (info->version, "__DATE__ __TIME__", sizeof (info->version) - 1);
	//strncpy (info->bus_info, dev->dev.parent->bus_id, sizeof (info->bus_info));
	strncpy (info->bus_info, to_platform_device (dev->dev.parent)->name, sizeof (info->bus_info) - 1);
}

static int snx_miitool_nway_reset (struct net_device *dev)
{
	struct snx_netdev_priv *priv = netdev_priv (dev);
	return mii_nway_restart (&priv->mii);
}

static u32 snx_miitool_get_link (struct net_device *dev)
{
	struct snx_netdev_priv *priv = netdev_priv (dev);
	return mii_link_ok (&priv->mii);
}

static struct ethtool_ops snx_miitool_ops = {
	.get_settings		= snx_miitool_get_settings,
	.set_settings		= snx_miitool_set_settings,
	.get_drvinfo		= snx_miitool_get_drvinfo,
	.nway_reset		= snx_miitool_nway_reset,
	.get_link		= snx_miitool_get_link,
};

static struct net_device_ops snx_netdev_ops = {
	.ndo_open		= snx_mac_open,
	.ndo_stop		= snx_mac_close,
	.ndo_start_xmit		= snx_mac_xmit,
	.ndo_set_multicast_list	= snx_mac_set_multicast,
	.ndo_set_mac_address	= snx_mac_set_mac,
	.ndo_do_ioctl		= snx_mac_ioctl,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_change_mtu		= eth_change_mtu,

#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller	= snx_mac_poll_controller,
#endif
};

#ifdef CONFIG_PM
static int snx_mac_suspend (struct platform_device *pdev, pm_message_t mesg)
{
	struct net_device *dev = platform_get_drvdata (pdev);
	struct snx_netdev_priv *priv = netdev_priv (dev);

	if (netif_running (dev)) {
		netif_stop_queue (dev);
		netif_device_detach (dev);

		clk_disable (priv->clk);
	}
	return 0;
}

static int snx_mac_resume (struct platform_device *pdev)
{
	struct net_device *dev = platform_get_drvdata (pdev);
	struct snx_netdev_priv *priv = netdev_priv (dev);

	if (netif_running (dev)) {
		clk_enable (priv->clk);

		netif_device_attach (dev);
		netif_start_queue (dev);
	}
	return 0;
}
#else
#define snx_mac_suspend	NULL
#define snx_mac_resume	NULL
#endif

static int __devexit snx_mac_remove (struct platform_device *pdev)
{
	struct net_device *dev = platform_get_drvdata (pdev);
	struct snx_netdev_priv *priv;

	if (dev != NULL) {
#ifdef NPAI
    netif_napi_del (&priv->napi);
#endif  
		platform_set_drvdata (pdev, NULL);
		priv = netdev_priv (dev);
		del_timer (&priv->check_timer);
		unregister_netdev (dev);

		if (priv->clk != NULL) {
			clk_disable (priv->clk);
			clk_put (priv->clk);
		}
#if 0
		if ((void *)dev->base_addr != NULL)
			iounmap ((void *)dev->base_addr); 

		if (priv->res != NULL) {
			release_resource (priv->res);
			kfree (priv->res);
		}
#endif
		free_netdev (dev);
	}
	return 0;
}

static int __devinit snx_mac_probe (struct platform_device *pdev)
{
//	struct snx_mac_data *data = pdev->dev.platform_data;
	char * ethaddr = (char *) &system_serial_low;
	struct net_device *dev;
	struct snx_netdev_priv *priv;
	struct resource *res;
	int err;

	res = platform_get_resource (pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		err = -EINVAL;
		goto out;
	}

	dev = alloc_etherdev (sizeof (struct snx_netdev_priv));
	if (dev == NULL) {
		err = -ENOMEM;
		goto err;
	}
	priv = netdev_priv (dev);

#if 0
	/* already done in the platform_device_register */
	priv->res = request_mem_region (res->start, PAGE_SIZE, pdev->dev.bus_id);
	if (priv->res == NULL) {
	err = -EBUSY;
		goto err;
	}
#endif
	dev->base_addr		= io_p2v (res->start);
#if 0
	dev->base_addr = (unsigned long)ioremap (res->start,
				res->end - res->start + 1);
	if ((void *)dev->base_addr == NULL) {
		err = -ENOMEM;
		goto err;
	}
#endif
	dev->irq 		= platform_get_irq (pdev, 0);
	dev->features		|= NETIF_F_SG;
#ifndef CONFIG_SNX_MAC_ZERO_COPY
	dev->features		|= NETIF_F_HW_CSUM;
#else
	dev->features		|= NETIF_F_GSO;
#endif
//	dev->get_stats		= snx_mac_get_stats;
	dev->ethtool_ops	= &snx_miitool_ops;
	dev->netdev_ops		= &snx_netdev_ops;
	ether_setup (dev);
	dev->flags 		|= IFF_ALLMULTI;	/* receive mode: refer to snx_mac_set_multicast () */
 
//	dev->hard_start_xmit	= snx_mac_xmit;
//	dev->open		= snx_mac_open;
//	dev->stop		= snx_mac_close;
//	dev->set_multicast_list	= snx_mac_set_multicast;
//	dev->set_mac_address	= snx_mac_set_mac;
//	dev->do_ioctl		= snx_mac_ioctl;

//	if (data != NULL) {
//		if (is_valid_ether_addr (data->dev_addr))
//			memcpy (dev->dev_addr, data->dev_addr, dev->addr_len);
	if (is_valid_ether_addr (ethaddr)) {
	//	if (is_valid_ether_addr (ethaddr))   //alek modify do again
			memcpy (dev->dev_addr, ethaddr, dev->addr_len);
	} else {
		random_ether_addr (dev->dev_addr);
		dev->dev_addr[0] &= 0x0;		/* ensure valid ether addr */
		printk (KERN_INFO "%s : generated random MAC address "
			"%.2x:%.2x:%.2x:%.2x:%.2x:%.2x.\n", pdev->name,
			dev->dev_addr[0], dev->dev_addr[1], dev->dev_addr[2],
			dev->dev_addr[3], dev->dev_addr[4], dev->dev_addr[5]);
	}

	SET_NETDEV_DEV (dev, &pdev->dev);
#ifdef NAPI
  priv->netdev = dev; 
  printk ("SNX  NAPI POLL ADD\n");
  netif_napi_add (dev, &priv->napi, snx_mac_poll, NAPI_WEIGHT);
#endif
//	priv->mii.phy_id	= data->phy_id;	/* manually specify the PHY address by default value */
	priv->mii.phy_id_mask	= 0x1f;
	priv->mii.reg_num_mask	= 0x1f;
	priv->mii.dev		= dev;
	priv->mii.mdio_read	= snx_mac_mdio_read;
	priv->mii.mdio_write	= snx_mac_mdio_write;

	priv->clk = clk_get (&pdev->dev, "mac_clk");
	if (priv->clk == NULL) {
		err = PTR_ERR (priv->clk);
		goto err;
	}
	clk_enable (priv->clk);

	err = register_netdev (dev);
	if (!err) {
		platform_set_drvdata (pdev, dev);
		printk (KERN_INFO "%s: SNX Ethernet MAC controller at 0x%lx (irq = %d) "
				 "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x.\n",
				pdev->name, io_v2p (dev->base_addr), dev->irq,
				dev->dev_addr[0], dev->dev_addr[1], dev->dev_addr[2],
				dev->dev_addr[3], dev->dev_addr[4], dev->dev_addr[5]);
		snx_mac_phy_detect (dev);		/* auto dectect PHY address, get PHY ID */

		init_timer (&priv->check_timer);
		priv->check_timer.data = (unsigned long)dev;
		priv->check_timer.function = snx_mac_check_link;
		priv->check_timer.expires = jiffies + msecs_to_jiffies (LINK_POLL_INTERVAL);
		add_timer (&priv->check_timer);

		goto out;
	}

err:
	snx_mac_remove (pdev);  
out:
	return err;
}

static struct platform_driver snx_mac_driver = {
	.probe		= snx_mac_probe,
	.remove		= __devexit_p (snx_mac_remove),
	.suspend	= snx_mac_suspend,
	.resume		= snx_mac_resume,
	.driver		= {
		.name	= "snx_mac",
		.owner	= THIS_MODULE,
	},
};

static int __init snx_mac_init (void)
{
	printk (KERN_INFO "SONiX Ethernet driver, (c) 2013 Sonix\n");
	return platform_driver_register (&snx_mac_driver);
}

static void __exit snx_mac_exit (void)
{
	platform_driver_unregister (&snx_mac_driver);
}

module_init (snx_mac_init);
module_exit (snx_mac_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Kelvin Cheung");
MODULE_DESCRIPTION ("SNX MAC Ethernet driver");
