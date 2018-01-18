/*
 * Faraday FOTG200 ("FOTG200") USB Device Controller driver
 *
 * Copyright (C) 2004-2005 John
 *      by John Chiang
 * Copyright (C) 2004 Faraday corp. 
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */
//*************************************************************************
//****************************** 1.Name Define ****************************
//*************************************************************************

//////////////////////////////////////////////
//#define	OTG200_Peripheral_Only
////////////////////////////////////////////

#include <mach/platform.h>

#define TRUE			1
#define FALSE			0

#define MASK_F0			0xF0

// Block Size define
#define BLK512BYTE		1
#define BLK1024BYTE		2

#define BLK64BYTE		1
#define BLK128BYTE		2

// Block toggle number define
#define SINGLE_BLK		1
#define DOUBLE_BLK		2
#define TRIBLE_BLK		3

// Endpoint transfer type
#define TF_TYPE_CONTROL		0
#define TF_TYPE_ISOCHRONOUS	1
#define TF_TYPE_BULK		2
#define TF_TYPE_INTERRUPT	3

// Endpoint or FIFO direction define
#define DIRECTION_IN	0//1
#define DIRECTION_OUT	1//0

// FIFO number define
#define FIFO0	        0x0
#define FIFO1	        0x1
#define FIFO2	        0x2
#define FIFO3	        0x3
#define FIFO4	        0x4
#define FIFO5	        0x5
#define FIFO6	        0x6
#define FIFO7	        0x7
#define FIFO8	        0x8
#define FIFO9	        0x9
#define FIFOCX	        0xFF
// Endpoint number define
#define EP0             0x00
#define EP1             0x01
#define EP2             0x02
#define EP3             0x03
#define EP4             0x04
#define EP5             0x05
#define EP6             0x06
#define EP7             0x07
#define EP8             0x08
#define EP9             0x09
#define EP10            0x10
#define EP11            0x11
#define EP12            0x12
#define EP13            0x13
#define EP14            0x14
#define EP15            0x15
	
#define BIT0		0x00000001
#define BIT1		0x00000002
#define BIT2		0x00000004
#define BIT3		0x00000008
#define BIT4		0x00000010
#define BIT5		0x00000020
#define BIT6		0x00000040
#define BIT7		0x00000080

#define BIT8		0x00000100
#define BIT9		0x00000200
#define BIT10		0x00000400
#define BIT11		0x00000800
#define BIT12		0x00001000
#define BIT13		0x00002000
#define BIT14		0x00004000
#define BIT15		0x00008000	
	
#define BIT16		0x00010000
#define BIT17		0x00020000
#define BIT18		0x00040000
#define BIT19		0x00080000
#define BIT20		0x00100000
#define BIT21		0x00200000
#define BIT22		0x00400000
#define BIT23		0x00800000	
	
#define BIT24		0x01000000
#define BIT25		0x02000000
#define BIT26		0x04000000
#define BIT27		0x08000000
#define BIT28		0x10000000
#define BIT29		0x20000000
#define BIT30		0x40000000
#define BIT31		0x80000000	
	
#define mLowByte(u16)	        ((u8)(u16	 ))
#define mHighByte(u16)	        ((u8)(u16 >> 8))

#define	DMA_ADDR_INVALID	(~(dma_addr_t)0)
#define DMA_CHANEL_FREE         0XFF

// Buffer allocation for each EP
#define USB_EPX_BUFSIZ	4096

#define	MAX_FIFO_SIZE	64    // reset value for EP
#define	MAX_EP0_SIZE	0x40  // ep0 fifo size

#define USB_EP0_BUFSIZ	256

// Used for test pattern traffic
#define TEST_J		0x02
#define TEST_K		0x04
#define TEST_SE0_NAK	0x08
#define TEST_PKY	0x10

#define MAX_EP_NUM	8	
#define MAX_FIFO_NUM	4	



/*-------------------------------------------------------------------------*/
//*************************************************************************
//****************************** 2.Structure Define************************
//*************************************************************************

/* DRIVER DATA STRUCTURES and UTILITIES */

struct FTC_ep {
	struct usb_ep	 ep;
	struct FTC_udc	*dev;
	unsigned long 	 irqs;
	u32              wDMA_Set_Length;
	unsigned         num:8,
		         dma:1,
		         is_in:1,
		         stopped:1;
	/* analogous to a host-side qh */
	struct list_head			queue;
	const struct usb_endpoint_descriptor	*desc;
};

struct FTC_request {
	struct usb_request	req;
	struct list_head	queue;
	unsigned		mapped:1;
};

enum ep0state {
	EP0_DISCONNECT,	 /* no host */
	EP0_IDLE,	 /* between STATUS ack and SETUP report */
	EP0_IN, EP0_OUT, /* data stage */
	EP0_STATUS,	 /* status stage */
	EP0_STALL,	 /* data or status stages */
	EP0_SUSPEND,	 /* usb suspend */
};

typedef enum 
{
	CMD_VOID,		// No command
	CMD_GET_DESCRIPTOR,	// Get_Descriptor command
	CMD_SET_DESCRIPTOR,	// Set_Descriptor command
	CMD_TEST_MODE           // Test_Mode command
} CommandType;

typedef enum 
{
    PIO_Mode,
    DMA_Mode,
} DMA_mode;

struct FTC_udc {
	/* each pci device provides one gadget, several endpoints */
	struct usb_gadget	        gadget;
	spinlock_t	                lock;
	struct FTC_ep		        ep[MAX_EP_NUM];
	struct usb_gadget_driver        *driver;
	struct otg_transceiver		*transceiver;    //For OTG
	unsigned			softconnect:1;   //For OTG
	unsigned			vbus_active:1;	 //For OTG
	u16				devstat;	 //For OTG
        struct usb_request              *EP0req;
   	enum ep0state                   ep0state;
	unsigned		        got_irq:1,
					got_region:1,
					req_config:1,
					configured:1,
					enabled:1;
	struct usb_ctrlrequest          ControlCmd;
        u8                              u8UsbConfigValue;
        u8                              u8UsbInterfaceValue;
        u8                              u8UsbInterfaceAlternateSetting;
        CommandType                     eUsbCxCommand;
        DMA_mode                        Dma_Status;
        u8                              bUsbBufferEmpty;
        u8                              u16TxRxCounter;
        u8                              usb_interrupt_level1;
        u8                              usb_interrupt_level1_Save;
        u8                              usb_interrupt_level1_Mask;
        u8                              *pu8DescriptorEX;
        u8                              EPUseDMA;   
	/* statistics... */
	unsigned long			irqs;
        u8                              u8LineCount;

};

/*-------------------------------------------------------------------------*/
//*************************************************************************
//****************************** 3.Debug Info Define************************
//*************************************************************************
#define DBG_OFF 	        0x00
#define DBG_CTRL 	        0x01
#define DBG_BULK 	        0x02
#define DBG_ISO		        0x04
#define DBG_INT		        0x08
#define DBG_FUNC	        0x10
#define DBG_INFO_GENERAL	0x20
#define DBG_TEMP	        0x40

#define USB_DBG 	(DBG_OFF)//General Setting
//#define USB_DBG 	(DBG_TEMP|DBG_INFO)
//#define USB_DBG		(DBG_CTRL | DBG_BULK | DBG_INT | DBG_FUNC | DBG_TEMP)

#define xprintk(dev,level,fmt,args...) printk(level "%s : " fmt , driver_name , ## args)
#define wprintk(level,fmt,args...) printk(level "%s : " fmt , driver_name , ## args)

#ifdef DEBUG
#define DBG(dev,fmt,args...) xprintk(dev , KERN_DEBUG , fmt , ## args)
#else
#define DBG(dev,fmt,args...) do { } while (0)
#endif /* DEBUG */


#define VERBOSE
 
#ifdef VERBOSE
#define VDBG DBG
#else
#define VDBG(dev,fmt,args...) do { } while (0)
#endif	/* VERBOSE */

#define ERROR(dev,fmt,args...) xprintk(dev , KERN_ERR , fmt , ## args)
#define WARN(dev,fmt,args...)  xprintk(dev , KERN_WARNING , fmt , ## args)
#define INFO(dev,fmt,args...)  xprintk(dev , KERN_INFO , fmt , ## args)




#if (USB_DBG & DBG_FUNC)  
#define DBG_FUNCC(fmt,args...) wprintk(KERN_INFO , fmt , ## args)
#else
#define DBG_FUNCC(fmt,args...)
#endif

#if (USB_DBG & DBG_CTRL)  
#define DBG_CTRLL(fmt,args...) wprintk(KERN_INFO , fmt , ## args)
#else
#define DBG_CTRLL(fmt,args...)
#endif

#if (USB_DBG & DBG_BULK)  
#define DBG_BULKK(fmt,args...) wprintk(KERN_INFO , fmt , ## args)
#else
#define DBG_BULKK(fmt,args...)
#endif


#if (USB_DBG & DBG_TEMP)  
#define DBG_TEMP_FUN(fmt,args...) wprintk(KERN_INFO , fmt , ## args)
#else
#define DBG_TEMP_FUN(fmt,args...)
#endif


/*-------------------------------------------------------------------------*/
//*************************************************************************
//****************************** 4.Others Define************************
//*************************************************************************

/* 2.5 stuff that's sometimes missing in 2.4 */

#ifndef container_of
#define	container_of	list_entry
#endif

#ifndef likely
#define likely(x)	(x)
#define unlikely(x)	(x)
#endif

#ifndef BUG_ON
#define BUG_ON(condition) do { if (unlikely((condition)!=0)) BUG(); } while(0)
#endif

#ifndef WARN_ON
#define	WARN_ON(x)	do { } while (0)
#endif

#ifndef	IRQ_RETVAL
typedef void irqreturn_t;
#define IRQ_NONE
#define IRQ_HANDLED
#define IRQ_RETVAL(x)
#endif

//*************************************************************************
//****************************** 5.Function Export Define************************
//*************************************************************************



extern void vUsbFIFO_EPxCfg_FS(void);
extern void vUsbFIFO_EPxCfg_HS(void);
extern int FTC_fifo_status(struct usb_ep *_ep);


//*************************************************************************
//****************************** 6.ED/FIFO Config Define************************
//*************************************************************************

//**************************************
//*** Full Speed HW ED/FIFO Configuration Area ***
#define FULL_ED1_bBLKSIZE    BLK64BYTE
#define FULL_ED1_bBLKNO      SINGLE_BLK
#define FULL_ED1_bDIRECTION  DIRECTION_IN
#define FULL_ED1_bTYPE       TF_TYPE_BULK
#define FULL_ED1_MAXPACKET   64

#define FULL_ED2_bBLKSIZE    BLK64BYTE
#define FULL_ED2_bBLKNO      SINGLE_BLK
#define FULL_ED2_bDIRECTION  DIRECTION_OUT
#define FULL_ED2_bTYPE       TF_TYPE_BULK
#define FULL_ED2_MAXPACKET   64

#define FULL_ED3_bBLKSIZE    BLK64BYTE
#define FULL_ED3_bBLKNO      SINGLE_BLK
#define FULL_ED3_bDIRECTION  DIRECTION_IN
#define FULL_ED3_bTYPE       TF_TYPE_INTERRUPT
#define FULL_ED3_MAXPACKET   64

#define FULL_ED4_bBLKSIZE    BLK64BYTE
#define FULL_ED4_bBLKNO      SINGLE_BLK
#define FULL_ED4_bDIRECTION  DIRECTION_OUT
#define FULL_ED4_bTYPE       TF_TYPE_INTERRUPT
#define FULL_ED4_MAXPACKET   64
//**************************************************

//**************************************
//*** High Speed HW ED/FIFO Configuration Area ***
#define HIGH_ED1_bBLKSIZE    BLK512BYTE
#define HIGH_ED1_bBLKNO      SINGLE_BLK
#define HIGH_ED1_bDIRECTION  DIRECTION_IN
#define HIGH_ED1_bTYPE       TF_TYPE_BULK
#define HIGH_ED1_MAXPACKET   512

#define HIGH_ED2_bBLKSIZE    BLK512BYTE
#define HIGH_ED2_bBLKNO      SINGLE_BLK
#define HIGH_ED2_bDIRECTION  DIRECTION_OUT
#define HIGH_ED2_bTYPE       TF_TYPE_BULK
#define HIGH_ED2_MAXPACKET   512

#define HIGH_ED3_bBLKSIZE    BLK64BYTE
#define HIGH_ED3_bBLKNO      SINGLE_BLK
#define HIGH_ED3_bDIRECTION  DIRECTION_IN
#define HIGH_ED3_bTYPE       TF_TYPE_INTERRUPT
#define HIGH_ED3_MAXPACKET   64

#define HIGH_ED4_bBLKSIZE    BLK64BYTE
#define HIGH_ED4_bBLKNO      SINGLE_BLK
#define HIGH_ED4_bDIRECTION  DIRECTION_OUT
#define HIGH_ED4_bTYPE       TF_TYPE_INTERRUPT
#define HIGH_ED4_MAXPACKET   64
//**************************************************

#define FULL_ED1_FIFO_START  FIFO0
#define FULL_ED2_FIFO_START  (FULL_ED1_FIFO_START+(FULL_ED1_bBLKNO *FULL_ED1_bBLKSIZE))
#define FULL_ED3_FIFO_START  (FULL_ED2_FIFO_START+(FULL_ED2_bBLKNO *FULL_ED2_bBLKSIZE))
#define FULL_ED4_FIFO_START  (FULL_ED3_FIFO_START+(FULL_ED3_bBLKNO *FULL_ED3_bBLKSIZE))

#define FULL_EP1_Map         (FULL_ED1_FIFO_START |(FULL_ED1_FIFO_START << 4)|(0xF0 >> (4*(FULL_ED1_bDIRECTION))))
#define FULL_EP1_FIFO_Map    (((1-FULL_ED1_bDIRECTION) << 4) | EP1)
#define FULL_EP1_FIFO_Config (FIFOEnBit | ((FULL_ED1_bBLKSIZE - 1) << 4) | ((FULL_ED1_bBLKNO - 1) << 2) | FULL_ED1_bTYPE)

#define FULL_EP2_Map         (FULL_ED2_FIFO_START |(FULL_ED2_FIFO_START << 4)|(0xF0 >> (4*(FULL_ED2_bDIRECTION))))
#define FULL_EP2_FIFO_Map    (((1-FULL_ED2_bDIRECTION) << 4) | EP2)
#define FULL_EP2_FIFO_Config (FIFOEnBit | ((FULL_ED2_bBLKSIZE - 1) << 4) | ((FULL_ED2_bBLKNO - 1) << 2) | FULL_ED2_bTYPE)

#define FULL_EP3_Map         (FULL_ED3_FIFO_START |(FULL_ED3_FIFO_START << 4)|(0xF0 >> (4*(FULL_ED3_bDIRECTION))))
#define FULL_EP3_FIFO_Map    (((1-FULL_ED3_bDIRECTION) << 4) | EP3)
#define FULL_EP3_FIFO_Config (FIFOEnBit | ((FULL_ED3_bBLKSIZE - 1) << 4) | ((FULL_ED3_bBLKNO - 1) << 2) | FULL_ED3_bTYPE)

#define FULL_EP4_Map         (FULL_ED4_FIFO_START |(FULL_ED4_FIFO_START << 4)|(0xF0 >> (4*(FULL_ED4_bDIRECTION))))
#define FULL_EP4_FIFO_Map    (((1-FULL_ED4_bDIRECTION) << 4) | EP4)
#define FULL_EP4_FIFO_Config (FIFOEnBit | ((FULL_ED4_bBLKSIZE - 1) << 4) | ((FULL_ED4_bBLKNO - 1) << 2) | FULL_ED4_bTYPE)


#define HIGH_ED1_FIFO_START  FIFO0
#define HIGH_ED2_FIFO_START  (HIGH_ED1_FIFO_START+(HIGH_ED1_bBLKNO *HIGH_ED1_bBLKSIZE))
#define HIGH_ED3_FIFO_START  (HIGH_ED2_FIFO_START+(HIGH_ED2_bBLKNO *HIGH_ED2_bBLKSIZE))
#define HIGH_ED4_FIFO_START  (HIGH_ED3_FIFO_START+(HIGH_ED3_bBLKNO *HIGH_ED3_bBLKSIZE))

#define HIGH_EP1_Map         (HIGH_ED1_FIFO_START |(HIGH_ED1_FIFO_START << 4)|(0xF0 >> (4*(HIGH_ED1_bDIRECTION))))
#define HIGH_EP1_FIFO_Map    (((1-HIGH_ED1_bDIRECTION) << 4) | EP1)
#define HIGH_EP1_FIFO_Config (FIFOEnBit | ((HIGH_ED1_bBLKSIZE - 1) << 4) | ((HIGH_ED1_bBLKNO - 1) << 2) | HIGH_ED1_bTYPE)

#define HIGH_EP2_Map         (HIGH_ED2_FIFO_START |(HIGH_ED2_FIFO_START << 4)|(0xF0 >> (4*(HIGH_ED2_bDIRECTION))))
#define HIGH_EP2_FIFO_Map    (((1-HIGH_ED2_bDIRECTION) << 4) | EP2)
#define HIGH_EP2_FIFO_Config (FIFOEnBit | ((HIGH_ED2_bBLKSIZE - 1) << 4) | ((HIGH_ED2_bBLKNO - 1) << 2) | HIGH_ED2_bTYPE)

#define HIGH_EP3_Map         (HIGH_ED3_FIFO_START |(HIGH_ED3_FIFO_START << 4)|(0xF0 >> (4*(HIGH_ED3_bDIRECTION))))
#define HIGH_EP3_FIFO_Map    (((1-HIGH_ED3_bDIRECTION) << 4) | EP3)
#define HIGH_EP3_FIFO_Config (FIFOEnBit | ((HIGH_ED3_bBLKSIZE - 1) << 4) | ((HIGH_ED3_bBLKNO - 1) << 2) | HIGH_ED3_bTYPE)

#define HIGH_EP4_Map         (HIGH_ED4_FIFO_START |(HIGH_ED4_FIFO_START << 4)|(0xF0 >> (4*(HIGH_ED4_bDIRECTION))))
#define HIGH_EP4_FIFO_Map    (((1-HIGH_ED4_bDIRECTION) << 4) | EP4)
#define HIGH_EP4_FIFO_Config (FIFOEnBit | ((HIGH_ED4_bBLKSIZE - 1) << 4) | ((HIGH_ED4_bBLKNO - 1) << 2) | HIGH_ED4_bTYPE)



//*************************************************************************
//****************************** 7.HW Macro Define************************
//*************************************************************************
#define CPE_FOTG200_BASE 0x90A00000

#ifndef __FOTG200_M_H
	#define __FOTG200_M_H
										
	// Macro
//	#define FOTG200_BASE_ADDRESS		(IO_ADDRESS(CPE_FOTG200_BASE)) 
//	#define FOTG200_FIFO_BASE(bOffset)	(IO_ADDRESS(CPE_FOTG200_BASE)+0xC0+(bOffset<<2)) 

//        #define FOTG200_BASE_ADDRESS            (0xF90A0000)
//      #define FOTG200_FIFO_BASE(bOffset)      (0xF90A0000+(bOffset<<2))

        #define FOTG200_BASE_ADDRESS            (IO_ADDRESS(0x90800000))
        #define FOTG200_FIFO_BASE(bOffset)      (IO_ADDRESS(0x90800000)+(bOffset<<2))


	#define bFOTGPeri_Port(bOffset)		*((volatile u8 *) ( FOTG200_BASE_ADDRESS | (u32)(bOffset)))
	#define hwFOTGPeri_Port(bOffset)	*((volatile u16 *) ( FOTG200_BASE_ADDRESS | (u32)(bOffset)))
	#define wFOTGPeri_Port(bOffset)		*((volatile u32 *) ( FOTG200_BASE_ADDRESS | (u32)(bOffset)))

	
	// Macro
	
	// Main control register(0x100)
	#define mUsbRmWkupST()			(wFOTGPeri_Port(0x100) & BIT0)
	#define mUsbRmWkupSet()		        (wFOTGPeri_Port(0x100) |= BIT0)
	#define mUsbRmWkupClr()			(wFOTGPeri_Port(0x100) &= ~BIT0)
	
	#define mUsbTstHalfSpeedEn()		(wFOTGPeri_Port(0x100) |= BIT1)
	#define mUsbTstHalfSpeedDis()	        (wFOTGPeri_Port(0x100) &= ~BIT1)

	#define mUsbGlobIntEnRd()		(wFOTGPeri_Port(0x100) & BIT2)
	#define mUsbGlobIntEnSet()		(wFOTGPeri_Port(0x100) |= BIT2)
	#define mUsbGlobIntDis()		(wFOTGPeri_Port(0x100) &= ~BIT2)
	
	#define mUsbGoSuspend()			(wFOTGPeri_Port(0x100) |=  BIT3)
	
	#define mUsbSoftRstSet()		(wFOTGPeri_Port(0x100) |=  BIT4)
	#define mUsbSoftRstClr()		(wFOTGPeri_Port(0x100) &= ~BIT4)

	#define mUsbChipEnSet()			(wFOTGPeri_Port(0x100) |= BIT5)
	#define mUsbHighSpeedST()		(wFOTGPeri_Port(0x100) & BIT6)
	#define mUsbDMAResetSet()		(wFOTGPeri_Port(0x100) |= BIT8)
	
	// Device address register(0x104)
	#define mUsbDevAddrSet(Value)	        (wFOTGPeri_Port(0x104) = (u32)Value)
	#define mUsbCfgST()			(wFOTGPeri_Port(0x104) & BIT7)
	#define mUsbCfgSet()			(wFOTGPeri_Port(0x104) |= BIT7)
	#define mUsbCfgClr()			(wFOTGPeri_Port(0x104) &= ~BIT7)

	// Test register(0x108)
	#define mUsbClrAllFIFOSet()		(wFOTGPeri_Port(0x108) |= BIT0)
	#define mUsbClrAllFIFOClr()		(wFOTGPeri_Port(0x108) &= ~BIT0)

	// SOF Frame Number register(0x10C)
	#define mUsbFrameNo()			(u16)(wFOTGPeri_Port(0x10C) & 0x7FF)
	#define mUsbMicroFrameNo()		(u8)((wFOTGPeri_Port(0x10C) & 0x3800)>>11)

	// SOF Mask register(0x110)
	#define mUsbSOFMaskHS()		        (wFOTGPeri_Port(0x110) = 0x44c)
	#define mUsbSOFMaskFS()		        (wFOTGPeri_Port(0x110) = 0x2710)

	// PHY Test Mode Selector register(0x114)
	#define mUsbTsMdWr(item)		(wFOTGPeri_Port(0x114) = (u32)item)
	#define mUsbUnPLGClr()			(wFOTGPeri_Port(0x114) &= ~BIT0)
	#define mUsbUnPLGSet()			(wFOTGPeri_Port(0x114) |= BIT0)
	// Vendor Specific IO Control register(0x118)

	// Cx configuration and status register(0x11C)

	// Cx configuration and FIFO Empty Status register(0x120)
	#define mUsbEP0DoneSet()		(wFOTGPeri_Port(0x120) |= BIT0)
	#define mUsbTsPkDoneSet()		(wFOTGPeri_Port(0x120) |= BIT1)
	#define mUsbEP0StallSet()		(wFOTGPeri_Port(0x120) |= BIT2)
	#define mUsbCxFClr()			(wFOTGPeri_Port(0x120) |= BIT3)

	#define mUsbCxFFull()			(wFOTGPeri_Port(0x120) & BIT4)
	#define mUsbCxFEmpty()			(wFOTGPeri_Port(0x120) & BIT5)
	#define mUsbCxFByteCnt()		(u8)((wFOTGPeri_Port(0x120) & 0x7F000000)>>24)
	
	// IDLE Counter register(0x124)
	#define mUsbIdleCnt(time)		(wFOTGPeri_Port(0x124) = (u32)time)

	// Mask of interrupt group(0x130)
	#define mUsbIntGrp0Dis()		(wFOTGPeri_Port(0x130) |= BIT0)
	#define mUsbIntGrp1Dis()		(wFOTGPeri_Port(0x130) |= BIT1)
	#define mUsbIntGrp2Dis()		(wFOTGPeri_Port(0x130) |= BIT2)

	#define mUsbIntGroupMaskRd()	        (wFOTGPeri_Port(0x130))
	
	// Mask of interrupt source group 0(0x134)
	#define mUsbIntEP0SetupDis()		(wFOTGPeri_Port(0x134) |= BIT0)
	#define mUsbIntEP0InDis()		(wFOTGPeri_Port(0x134) |= BIT1)
	#define mUsbIntEP0OutDis()		(wFOTGPeri_Port(0x134) |= BIT2)
	#define mUsbIntEP0EndDis()		(wFOTGPeri_Port(0x134) |= BIT3)
	#define mUsbIntEP0FailDis()		(wFOTGPeri_Port(0x134) |= BIT4)

	#define mUsbIntEP0SetupEn()		(wFOTGPeri_Port(0x134) &= ~(BIT0))
	#define mUsbIntEP0InEn()		(wFOTGPeri_Port(0x134) &= ~(BIT1))
	#define mUsbIntEP0OutEn()		(wFOTGPeri_Port(0x134) &= ~(BIT2))
	#define mUsbIntEP0EndEn()		(wFOTGPeri_Port(0x134) &= ~(BIT3))
	#define mUsbIntEP0FailEn()		(wFOTGPeri_Port(0x134) &= ~(BIT4))

	#define mUsbIntSrc0MaskRd()		(wFOTGPeri_Port(0x134))
	
	// Mask of interrupt source group 1(0x138)
	#define mUsbIntFIFO0_3OUTDis()	        (wFOTGPeri_Port(0x138) |= 0xFF)
	#define mUsbIntFIFO0_3INDis()	        (wFOTGPeri_Port(0x138) |= 0xF0000)
	#define mUsbIntFIFO0_3Set(wTemp)	(wFOTGPeri_Port(0x138)|= wTemp)
	#define mUsbIntFIFO0_3Dis(wTemp)	(wFOTGPeri_Port(0x138)&= ~wTemp)
	
	#define mUsbIntF0OUTEn()		(wFOTGPeri_Port(0x138) &= ~(BIT1 | BIT0))	
	#define mUsbIntF0OUTDis()		(wFOTGPeri_Port(0x138) |= (BIT1 | BIT0))	
	#define mUsbIntF1OUTEn()		(wFOTGPeri_Port(0x138) &= ~(BIT3 | BIT2))
	#define mUsbIntF1OUTDis()		(wFOTGPeri_Port(0x138) |= (BIT3 | BIT2))
	#define mUsbIntF2OUTEn()		(wFOTGPeri_Port(0x138) &= ~(BIT5 | BIT4))
	#define mUsbIntF2OUTDis()		(wFOTGPeri_Port(0x138) |= (BIT5 | BIT4))
	#define mUsbIntF3OUTEn()		(wFOTGPeri_Port(0x138) &= ~(BIT7 | BIT6))
	#define mUsbIntF3OUTDis()		(wFOTGPeri_Port(0x138) |= (BIT7 | BIT6))
	
	#define mUsbIntFXOUTEn(bnum)	        (wFOTGPeri_Port(0x138) &= ~((BIT0<<((bnum)*2+1)) | (BIT0<<((bnum)*2))))
	#define mUsbIntFXOUTDis(bnum)     	(wFOTGPeri_Port(0x138) |= ((BIT0<<((bnum)*2+1)) | (BIT0<<((bnum)*2))))

	#define mUsbIntF0INEn()			(wFOTGPeri_Port(0x138) &= ~BIT16)
	#define mUsbIntF0INDis()		(wFOTGPeri_Port(0x138) |= BIT16)
	#define mUsbIntF1INEn()			(wFOTGPeri_Port(0x138) &= ~BIT17)
	#define mUsbIntF1INDis()		(wFOTGPeri_Port(0x138) |= BIT17)
	#define mUsbIntF2INEn()			(wFOTGPeri_Port(0x138) &= ~BIT18)
	#define mUsbIntF2INDis()		(wFOTGPeri_Port(0x138) |= BIT18)
	#define mUsbIntF3INEn()			(wFOTGPeri_Port(0x138) &= ~BIT19)
	#define mUsbIntF3INDis()		(wFOTGPeri_Port(0x138) |= BIT19)

	#define mUsbIntFXINEn(bnum)		(wFOTGPeri_Port(0x138) &= ~(BIT0<<(bnum+16)))
	#define mUsbIntFXINDis(bnum)	        (wFOTGPeri_Port(0x138) |= (BIT0<<(bnum+16)))
	
	#define mUsbIntSrc1MaskRd()		(wFOTGPeri_Port(0x138))
	
	// Mask of interrupt source group 2(DMA int mask)(0x13C)
	#define mUsbIntSuspDis()		(wFOTGPeri_Port(0x13C) |= BIT1)		
	#define mUsbIntDmaErrDis()		(wFOTGPeri_Port(0x13C) |= BIT8)
	#define mUsbIntDmaFinishDis()	        (wFOTGPeri_Port(0x13C) |= BIT7)

	#define mUsbIntSuspEn()			(wFOTGPeri_Port(0x13C) &= ~(BIT1))		
	#define mUsbIntDmaErrEn()		(wFOTGPeri_Port(0x13C) &= ~(BIT8))
	#define mUsbIntDmaFinishEn()	        (wFOTGPeri_Port(0x13C) &= ~(BIT7))

	#define mUsbIntSrc2MaskRd()		(wFOTGPeri_Port(0x13C))

	// Interrupt group (0x140)
	#define mUsbIntGroupRegRd()		(wFOTGPeri_Port(0x140))
	#define mUsbIntGroupRegSet(wValue)	(wFOTGPeri_Port(0x140) |= wValue)
	
	// Interrupt source group 0(0x144)
	#define mUsbIntSrc0Rd()			(wFOTGPeri_Port(0x144))	
	#define mUsbIntEP0AbortClr()	        (wFOTGPeri_Port(0x144) &= ~(BIT5))	
	#define mUsbIntSrc0Clr()		(wFOTGPeri_Port(0x144) = 0)
	#define mUsbIntSrc0Set(wValue)	        (wFOTGPeri_Port(0x144) |= wValue)
	
	// Interrupt source group 1(0x148)
	#define mUsbIntSrc1Rd()			(wFOTGPeri_Port(0x148))
	#define mUsbIntSrc1Set(wValue)	        (wFOTGPeri_Port(0x148) |= wValue)
	
	// Interrupt source group 2(0x14C)
	#define mUsbIntSrc2Rd()			(wFOTGPeri_Port(0x14C))
	#define mUsbIntSrc2Set(wValue)	        (wFOTGPeri_Port(0x14C) |= wValue)
	
	
	#define mUsbIntBusRstClr()		(wFOTGPeri_Port(0x14C) &= ~BIT0)		
	#define mUsbIntSuspClr()		(wFOTGPeri_Port(0x14C) &= ~BIT1)		
	#define mUsbIntResmClr()		(wFOTGPeri_Port(0x14C) &= ~BIT2)		
	#define mUsbIntIsoSeqErrClr()	        (wFOTGPeri_Port(0x14C) &= ~BIT3)			
	#define mUsbIntIsoSeqAbortClr()	        (wFOTGPeri_Port(0x14C) &= ~BIT4)			
	#define mUsbIntTX0ByteClr()		(wFOTGPeri_Port(0x14C) &= ~BIT5)			
	#define mUsbIntRX0ByteClr()		(wFOTGPeri_Port(0x14C) &= ~BIT6)			
	#define mUsbIntDmaFinishClr()	        (wFOTGPeri_Port(0x14C) &= ~BIT7)			
	#define mUsbIntDmaErrClr()		(wFOTGPeri_Port(0x14C) &= ~BIT8)			

	#define mUsbIntDmaFinishRd()		(wFOTGPeri_Port(0x14C) & BIT7)			
	#define mUsbIntDmaErrRd()		(wFOTGPeri_Port(0x14C) & BIT8)			
	
	// Rx 0 byte packet register(0x150)
	#define mUsbIntRX0ByteRd()		(u8)(wFOTGPeri_Port(0x150))
	#define mUsbIntRX0ByteSetClr(set)	(wFOTGPeri_Port(0x150) &= ~((u32)set))
	
	// Tx 0 byte packet register(0x154)
	#define mUsbIntTX0ByteRd()		(u8)(wFOTGPeri_Port(0x154))
	#define mUsbIntTX0ByteSetClr(data)	(wFOTGPeri_Port(0x154) &= ~((u32)data))

	// ISO sequential Error/Abort register(0x158)
	#define mUsbIntIsoSeqErrRd()		(u8)((wFOTGPeri_Port(0x158) & 0xff0000)>>16)
	#define mUsbIntIsoSeqErrSetClr(data)	(wFOTGPeri_Port(0x158) &= ~(((u32)data)<<16))

	#define mUsbIntIsoSeqAbortRd()	        (u8)(wFOTGPeri_Port(0x158) & 0xff)
	#define mUsbIntIsoSeqAbortSetClr(data)	(wFOTGPeri_Port(0x158) &= ~((u32)data))

	// IN Endpoint MaxPacketSize register(0x160,0x164,...,0x17C)
	#define mUsbEPinHighBandSet(EPn, dir , size )	(wFOTGPeri_Port(0x160 + ((EPn - 1) << 2)) &= ~(BIT14 |BIT13));  (wFOTGPeri_Port(0x160 + ((EPn - 1) << 2)) |= ((((u8)(size >> 11)+1) << 13)*(1 - dir)) )
	#define mUsbEPMxPtSz(EPn, dir, size)	        (wFOTGPeri_Port(0x160 + (dir * 0x20) + ((EPn - 1) << 2)) = (u16)(size))
	#define mUsbEPMxPtSzClr(EPn, dir)	        (wFOTGPeri_Port(0x160 + (dir * 0x20) + ((EPn - 1) << 2)) = 0)

	#define mUsbEPMxPtSzRd(EPn, dir)	(wFOTGPeri_Port(0x160 + (dir * 0x20) + ((EPn - 1) << 2)))

	#define mUsbEPinMxPtSz(EPn)		(wFOTGPeri_Port(0x160 + ((EPn - 1) << 2)) & 0x7ff)
	#define mUsbEPinStallST(EPn)		((wFOTGPeri_Port(0x160 + ((EPn - 1) << 2)) & BIT11) >> 11)
	#define mUsbEPinStallClr(EPn)		(wFOTGPeri_Port(0x160 + ((EPn - 1) << 2)) &= ~BIT11)
	#define mUsbEPinStallSet(EPn)		(wFOTGPeri_Port(0x160 + ((EPn - 1) << 2)) |=  BIT11)
	#define mUsbEPinRsTgClr(EPn)		(wFOTGPeri_Port(0x160 + ((EPn - 1) << 2)) &= ~BIT12)
	#define mUsbEPinRsTgSet(EPn)	        (wFOTGPeri_Port(0x160 + ((EPn - 1) << 2)) |=  BIT12)

	// OUT Endpoint MaxPacketSize register(0x180,0x164,...,0x19C)
	#define mUsbEPoutMxPtSz(EPn)	        ((wFOTGPeri_Port(0x180 + ((EPn - 1) << 2))) & 0x7ff)
	#define mUsbEPoutStallST(EPn)	        ((wFOTGPeri_Port(0x180 + ((EPn - 1) << 2)) & BIT11) >> 11)
	#define mUsbEPoutStallClr(EPn)	        (wFOTGPeri_Port(0x180 + ((EPn - 1) << 2)) &= ~BIT11)
	#define mUsbEPoutStallSet(EPn)	        (wFOTGPeri_Port(0x180 + ((EPn - 1) << 2)) |=  BIT11)
	#define mUsbEPoutRsTgClr(EPn)	        (wFOTGPeri_Port(0x180 + ((EPn - 1) << 2)) &= ~BIT12)
	#define mUsbEPoutRsTgSet(EPn)	        (wFOTGPeri_Port(0x180 + ((EPn - 1) << 2)) |=  BIT12)

	// Endpoint & FIFO Configuration
	// Endpoint 1~4 Map register(0x1a0), Endpoint 5~8 Map register(0x1a4)
	#define mUsbEPMap(EPn, MAP)	        (bFOTGPeri_Port(0x1a0 + (EPn-1)) = MAP)
	#define mUsbEPMapRd(EPn)		(bFOTGPeri_Port(0x1a0+ (EPn-1)))
	#define mUsbEPMapAllClr()		(wFOTGPeri_Port(0x1a0) = 0);(wFOTGPeri_Port(0x1a4) = 0)
	#define mUsbEPMap1_4Rd()                (wFOTGPeri_Port(0x1a0))
	
	
	// FIFO Map register(0x1a8)
	#define mUsbFIFOMap(FIFOn, MAP)	        (bFOTGPeri_Port(0x1a8 + FIFOn) = MAP)
	#define mUsbFIFOMapRd(FIFOn)	        (bFOTGPeri_Port(0x1a8 + FIFOn))
	#define mUsbFIFOMapAllClr()		(wFOTGPeri_Port(0x1a8) = 0)
	#define mUsbFIFOMapAllRd()              (wFOTGPeri_Port(0x1a8))
	// FIFO Configuration register(0x1ac)
	#define mUsbFIFOConfig(FIFOn, CONFIG)	(bFOTGPeri_Port(0x1ac + FIFOn) = CONFIG)
	#define mUsbFIFOConfigRd(FIFOn)		(bFOTGPeri_Port(0x1ac + FIFOn))
	#define mUsbFIFOConfigAllClr()		(bFOTGPeri_Port(0x1ac) = 0)
	#define FIFOEnBit			0x20
	#define mUsbFIFOConfigAllRd()           (wFOTGPeri_Port(0x1ac))
	// FIFO byte count register(0x1b0)
	#define mUsbFIFOOutByteCount(fifo_num)	(((wFOTGPeri_Port(0x1b0+(fifo_num)*4)&0x7ff)))
	#define mUsbFIFODone(fifo_num)		(wFOTGPeri_Port(0x1b0+(fifo_num)*4) |= BIT11)
	#define mUsbFIFOReset(fifo_num)		(wFOTGPeri_Port(0x1b0+(fifo_num)*4) |=  BIT12) //john
	#define mUsbFIFOResetOK(fifo_num)	(wFOTGPeri_Port(0x1b0+(fifo_num)*4) &= ~BIT12) //john	
	// DMA target FIFO register(0x1c0)
	#define FOTG200_DMA2FIFO_Non 		0
	#define FOTG200_DMA2FIFO0 		BIT0
	#define FOTG200_DMA2FIFO1 		BIT1
	#define FOTG200_DMA2FIFO2 		BIT2
	#define FOTG200_DMA2FIFO3 		BIT3
	#define FOTG200_DMA2CxFIFO 		BIT4
	
	#define mUsbDMA2FIFOSel(sel)		(wFOTGPeri_Port(0x1c0) = sel)
	#define mUsbDMA2FIFORd()		(wFOTGPeri_Port(0x1c0))
	
	// DMA parameter set 1 (0x1c8)	
	#define mUsbDmaConfig(len,Dir)		(wFOTGPeri_Port(0x1c8) = (((u32)len)<<8)|((1-Dir)<<1))
	#define mUsbDmaLenRd()			((wFOTGPeri_Port(0x1c8) & 0x1fff0000) >> 8)
	#define mUsbDmaConfigRd()		(wFOTGPeri_Port(0x1c8))
	#define mUsbDmaConfigSet(set)		(wFOTGPeri_Port(0x1c8) = set)
	
	#define mUsbDmaStart()			(wFOTGPeri_Port(0x1c8) |= BIT0)
	#define mUsbDmaStop()			(wFOTGPeri_Port(0x1c8) &= ~BIT0)

	// DMA parameter set 2 (0x1cc)	
	#define mUsbDmaAddr(addr)		(wFOTGPeri_Port(0x1cc) = addr)
	#define mUsbDmaAddrRd()			(wFOTGPeri_Port(0x1cc))
	
	// 8 byte command data port(0x1d0)
	#define mUsbEP0CmdDataRdDWord()	        (wFOTGPeri_Port(0x1d0))
	
	
        //For OTG Definition;;0x80	
        #define mUsb_OTGC_Control_B_BUS_REQ_Rd()	(wFOTGPeri_Port(0x80)& BIT0)	
	#define mUsb_OTGC_Control_B_BUS_REQ_Set()  	(wFOTGPeri_Port(0x80) |=  BIT0)  
	#define mUsb_OTGC_Control_B_BUS_REQ_Clr()  	(wFOTGPeri_Port(0x80) &=  (~BIT0)) 	

        #define mUsb_OTGC_Control_B_HNP_EN_Rd()		(wFOTGPeri_Port(0x80)& BIT1)	
	#define mUsb_OTGC_Control_B_HNP_EN_Set()	(wFOTGPeri_Port(0x80) |=  BIT1)   
	#define mUsb_OTGC_Control_B_HNP_EN_Clr()  	(wFOTGPeri_Port(0x80) &=  (~BIT1))  	

        #define mUsb_OTGC_Control_B_DSCHG_VBUS_Rd()	(wFOTGPeri_Port(0x80)& BIT2)	
	#define mUsb_OTGC_Control_B_DSCHG_VBUS_Set()	(wFOTGPeri_Port(0x80) |=  BIT2)    
	#define mUsb_OTGC_Control_B_DSCHG_VBUS_Clr() 	(wFOTGPeri_Port(0x80) &=  (~BIT2))  

        #define mUsb_OTGC_Control_B_SESS_END_Rd()	(wFOTGPeri_Port(0x80)& BIT16)	  
        #define mUsb_TGC_Control_B_SESS_VLD_Rd()	(wFOTGPeri_Port(0x80)& BIT17)	  
        #define mUsb_TGC_Control_A_SESS_VLD_Rd()	(wFOTGPeri_Port(0x80)& BIT18)	  
        #define mUsb_TGC_Control_A_VBUS_VLD_Rd()	(wFOTGPeri_Port(0x80)& BIT19)  

        #define mUsb_OTGC_Control_CROLE_Rd()	        (wFOTGPeri_Port(0x80)& BIT20) //0:Host 1:Peripheral


        #define mUsb_dwOTGC_INT_STS_ROLE_CHG_Rd()       (wFOTGPeri_Port(0x84)& BIT8)	
	
	
	
	
	//For Host Port Reset setting (Timing Critical)
	#define mUsb_mwHost20_PORTSC_PortReset_Rd()	(wFOTGPeri_Port(0x30) &=  BIT8) 	     	
	#define mUsb_mwHost20_PORTSC_PortReset_Set()	(wFOTGPeri_Port(0x30) |=  BIT8)      	
	#define mUsb_mwHost20_PORTSC_PortReset_Clr()	(wFOTGPeri_Port(0x30) &=  ~BIT8)   	
	
	
	
	
#endif /* __FOTG200_M_H  */
