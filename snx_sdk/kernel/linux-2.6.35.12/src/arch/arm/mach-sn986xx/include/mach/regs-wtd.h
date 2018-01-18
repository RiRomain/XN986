/*
 * Create at 2011/05/06 by yanjie_yang
 */

#ifndef __MACH_REGS_WTD_H
#define __MACH_REGS_WTD_H

#include <mach/platform.h>

/*
 * SONiX watch Dog Controller virtual addresses
 */
#define WTD_CNT			IO_ADDRESS(SNX_WTD_CNT)
#define WTD_LOAD		IO_ADDRESS(SNX_WTD_LOAD)
#define WTD_RST			IO_ADDRESS(SNX_WTD_RST)
#define WTD_CTRL		IO_ADDRESS(SNX_WTD_CTRL)
#define WTD_STATUS		IO_ADDRESS(SNX_WTD_STATUS)
#define WTD_CLR			IO_ADDRESS(SNX_WTD_CLR)
#define WTD_INTRLEN		IO_ADDRESS(SNX_WTD_INTRLEN)

/*
 * SONiX Watch Dog Controller physical addresses
 */
#define SNX_WTD_CNT			(SNX_WTD_BASE + WTD_CNT_OFFSET)
#define SNX_WTD_LOAD			(SNX_WTD_BASE + WTD_LOAD_OFFSET)
#define SNX_WTD_RST			(SNX_WTD_BASE + WTD_RST_OFFSET)
#define SNX_WTD_CTRL			(SNX_WTD_BASE + WTD_CTRL_OFFSET)
#define SNX_WTD_STATUS			(SNX_WTD_BASE + WTD_STATUS_OFFSET)
#define SNX_WTD_CLR			(SNX_WTD_BASE + WTD_CLR_OFFSET)
#define SNX_WTD_INTRLEN			(SNX_WTD_BASE + WTD_INTRLEN_OFFSET)

/* Watch dog controller registers offset */
#define WTD_CNT_OFFSET			0x00	/* Watch Dog Timer Counter */
#define WTD_LOAD_OFFSET			0x04	/* Watch Dog Timer Counter Auto Reload */
#define WTD_RST_OFFSET			0x08	/* Watch Dog Timer Counter Restart */
#define WTD_CTRL_OFFSET			0x0c	/* Watch Dog Timer Control */
#define WTD_STATUS_OFFSET		0x10	/* Watch Dog Timer Status */
#define WTD_CLR_OFFSET			0x14	/* Watch Dog Timer Clear */
#define WTD_INTRLEN_OFFSET		0x18	/* Watch Dog Timer Interrupt Length */

#endif
