/*
 * Faraday FOTG200-Peripheral  USB Device Controller driver
 *
 * Copyright (C) 2004-2005 Lineo
 *      by John Chiang
 * Copyright (C) 2004 Faraday tech corp.
 * 
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

/*
 * This device has ep0 and four bulk/interrupt endpoints.
 *
 *  - Endpoint numbering is fixed: EP1-bulk in, EP2-bulk out, EP3-interrupt in, EP4 interrupt out 
 *  - EP maxpacket (if full speed:64, if high speed:512)
 *  - no DMA supporting in the first version
 *  - support AHB_DMA in the 2nd version
 */

#undef DEBUG
// #define	VERBOSE		/* extra debug messages (success too) */
// #define	USB_TRACE	/* packet-level success messages */

//#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/smp_lock.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/proc_fs.h>

#include <linux/usb/ch9.h>
#include <linux/usb/otg.h>
#include <linux/usb/gadget.h>

#include <asm/byteorder.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/pci.h>
#include <asm/system.h>
#include <asm/unaligned.h>
//#include <asm/arch/cpe_int.h>

#include "FTC_FOTG200_udc.h"

#define	DRIVER_DESC	"FOTG200 USB Peripheral Controller"
#define	DRIVER_VERSION	"15-Feb 2005"

static const char driver_name [] = "FTC_FOTG200_udc";
static const char driver_desc [] = DRIVER_DESC;

static char *names [] = {"ep0","ep1-bulkin","ep2-bulkout","ep3-intin","ep4-intout","ep5","ep6","ep7","ep8","ep9","ep10" };

static struct FTC_udc	*the_controller=0;

MODULE_AUTHOR("Faraday-SW");
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");


extern struct otg_transceiver *FOTG2XX_get_otg_transceiver(void);



/*-------------------------------------------------------------------------*/
//*******************************************************************
// Name:Get_FIFO_Num
// Description:get the FIFO number from ep 
//             FIFO0=0
//             FIFO1=1
//             FIFO2=2
//             FIFO3=3
//             FIFO-CX = 0xFF
//*******************************************************************
static u8 Get_FIFO_Num(struct FTC_ep *ep)
{
    u8  u8fifo_n;  

    if (ep->num==0)
        {u8fifo_n=FIFOCX;
        return(u8fifo_n);
        }
        
    u8fifo_n = mUsbEPMapRd(ep->num);		// get the relatived FIFO number
    if (ep->is_in)
       u8fifo_n &= 0x0F;
    else
       u8fifo_n >>= 4;
    if (u8fifo_n >= MAX_FIFO_NUM)	// over the Max. fifo count ?
       {
       	printk("??? Error ep > FUSB200_MAX_FIFO \n");
       	}
 
  return( u8fifo_n);

}
static void nuke(struct FTC_ep *, int status);

//****************************************************
// Name:FTC_ep_enable
// Description: Enable endpoint 
//              EP0 : has been enabled while driver booting up
//              Need to give this EP's descriptor
// Input:<1>.ep structure point
// Output:none
//****************************************************
static int
FTC_ep_enable(struct usb_ep *_ep, const struct usb_endpoint_descriptor *desc)
{
	struct FTC_udc	*dev;
	struct FTC_ep	*ep;
	u16	max;
	unsigned long flags;

	ep = container_of(_ep, struct FTC_ep, ep);

	DBG_FUNCC("+FTC_ep_enable() : _ep = %x desc = %x ep->desc= %x\n",(u32) _ep, (u32) desc, (u32) ep->desc);

  //<1>.Checking input
	if (!_ep || !desc || ep->desc
			|| desc->bDescriptorType != USB_DT_ENDPOINT)
		return -EINVAL;

	dev = ep->dev; 
	if (ep == &dev->ep[0])  //no EP0 need to be enabled
		return -EINVAL;

	if (!dev->driver || dev->gadget.speed == USB_SPEED_UNKNOWN)
		return -ESHUTDOWN;

	if (ep->num != (desc->bEndpointAddress & 0x0f)) 
		return -EINVAL;

	// EP should be Bulk or intr
	switch (desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) {
	case USB_ENDPOINT_XFER_BULK:
	case USB_ENDPOINT_XFER_INT:
		break;
	default:
		return -EINVAL;
	}

  //<2>.Set the ep data
	/* enabling the no-toggle interrupt mode would need an api hook */
	max = le16_to_cpu(get_unaligned(&desc->wMaxPacketSize));

	// In FOTG200 Only support DMA (PIO not support)
	   ep->dma = 1;

	ep->is_in = (USB_DIR_IN & desc->bEndpointAddress) != 0;

	spin_lock_irqsave(&ep->dev->lock, flags);

	ep->ep.maxpacket = max;
	ep->stopped = 0;
	ep->desc = desc;
	spin_unlock_irqrestore(&ep->dev->lock, flags);

	VDBG(dev, "enable %s %s %s maxpacket %u\n", ep->ep.name,
		ep->is_in ? "IN" : "OUT",
		ep->dma ? "dma" : "pio",
		max);


	INFO(dev,"enable %s %s %s maxpacket %u\n", ep->ep.name,
		ep->is_in ? "IN" : "OUT",
		ep->dma ? "dma" : "pio",
		max);

	return 0;
}
//****************************************************
// Name:ep_reset
// Description: ep reset
//              
// Input:<1>.ep structure point
// Output:none
//****************************************************
static void ep_reset(struct FTC_ep *ep)
{
	DBG_FUNCC("+ep_reset\n");

	ep->ep.maxpacket = MAX_FIFO_SIZE;
	ep->desc = 0;
	ep->stopped = 1;
	ep->irqs = 0;
	ep->dma = 0;
}

//****************************************************
// Name:FTC_ep_disable
// Description: 
//              
// Input:<1>.ep structure point
// Output:none
//****************************************************
static int FTC_ep_disable(struct usb_ep *_ep)
{
	struct FTC_ep	*ep;
	struct FTC_udc	*dev;

	unsigned long	flags;

	DBG_FUNCC("+FTC_ep_disable()\n");
	
	//<1>.Checking input
	ep = container_of(_ep, struct FTC_ep, ep);
	if (!_ep || !ep->desc)
		return -ENODEV;

	dev = ep->dev;
	if (ep == &dev->ep[0])  //john no EP0 need to be enabled
		return -EINVAL;

	VDBG(dev, "disable %s\n", _ep->name);
	
	//<2>.de queue all the requests
	spin_lock_irqsave(&dev->lock, flags);
	nuke(ep, -ESHUTDOWN);
	
	//<3>.reset ep	
	ep_reset(ep);
	spin_unlock_irqrestore(&dev->lock, flags);

	return 0;
}

/*-------------------------------------------------------------------------*/
//****************************************************
// Name:FTC_alloc_request
// Description: allocate request
//              
// Input:<1>.ep structure point
//       <2>.gfp flag
// Output:none
//****************************************************
static struct usb_request *
FTC_alloc_request(struct usb_ep *_ep, gfp_t gfp_flags)
{
	struct FTC_request *req;
    
	DBG_FUNCC("+FTC_alloc_request\n");

  	//<1>.Checking input
	if (!_ep)
		return 0;

  	//<2>.Malloc memory
	req = kmalloc(sizeof *req, gfp_flags);
	if (!req)
		return 0;

	memset(req, 0, sizeof *req);
	req->req.dma = DMA_ADDR_INVALID;
	
  	//<3>.Add to list	
	INIT_LIST_HEAD(&req->queue);

	DBG_FUNCC("-FTC_alloc_request\n");

	return &req->req;
}
//****************************************************
// Name:FTC_free_request
// Description: free request
//              
// Input:<1>.ep structure point
//       <2>.gfp flag
// Output:none
//****************************************************
static void
FTC_free_request(struct usb_ep *_ep, struct usb_request *_req)
{
	struct FTC_request *req;

    	DBG_FUNCC("+FTC_free_request()\n");

	if (!_ep || !_req)
		return;

	req = container_of(_req, struct FTC_request, req);
	WARN_ON(!list_empty(&req->queue));
	kfree(req);
}

/*-------------------------------------------------------------------------*/

/* many common platforms have dma-coherent caches, which means that it's
 * safe to use kmalloc() memory for all i/o buffers without using any
 * cache flushing calls.  (unless you're trying to share cache lines
 * between dma and non-dma activities, which is a slow idea in any case.)
 *
 * other platforms need more care, with 2.6 having a moderately general
 * solution except for the common "buffer is smaller than a page" case.
 */
//#undef USE_KMALLOC //Please use the "pci_alloc_consistent" function call
#define USE_KMALLOC


//****************************************************
// Name:FTC_alloc_buffer
// Description: Allocate the buffer for FOTG200 DMA
//              
// Input:<1>.ep structure point
//       <2>.bytes number
//       <3>.dma address pointer
//       <4>.gfp flag
// Output:none
//****************************************************
/* allocating buffers this way eliminates dma mapping overhead, which
 * on some platforms will mean eliminating a per-io buffer copy.  with
 * some kinds of system caches, further tweaks may still be needed.
 */
static void *
FTC_alloc_buffer(struct usb_ep *_ep, unsigned bytes, dma_addr_t *dma, int  gfp_flags)
{
	void *retval;
	struct FTC_ep *ep;

	DBG_FUNCC("+FTC_alloc_buffer():ep = 0x%x\n",(u32) _ep);

	ep = container_of(_ep, struct FTC_ep, ep);
	if (!_ep)
		return 0;

	*dma = DMA_ADDR_INVALID;

#if defined(USE_KMALLOC)
	retval = kmalloc(bytes, gfp_flags);
	if (retval)
		*dma = virt_to_phys(retval);
#else
	//if (ep->dma) {
		/* one problem with this call is that it wastes memory on
		 * typical 1/N page allocations: it allocates 1-N pages.
		 * another is that it always uses GFP_ATOMIC.
		 */
		retval = consistent_alloc(gfp_flags, bytes, dma);
	//} 
	//else
	//	retval = kmalloc(bytes, gfp_flags);
#endif
	return retval;
}
//****************************************************
// Name:FTC_free_buffer
// Description: Free the buffer
//              
// Input:<1>.ep structure point
//       <2>.bytes number
//       <3>.dma address pointer
//       <4>.gfp flag
// Output:none
//****************************************************
static void
FTC_free_buffer(struct usb_ep *_ep, void *buf, dma_addr_t dma, unsigned bytes)
{
	//struct FTC_ep *ep;
	DBG_FUNCC("+FTC_free_buffer()\n");

	/* free memory into the right allocator */
#ifndef	USE_KMALLOC
	//if (dma != DMA_ADDR_INVALID) {
		ep = container_of(_ep, struct FTC_ep, ep);
		if (!_ep)
			return;
		/* one problem with this call is that some platforms
		 * don't allow it to be used in_irq().
		 */
		
		consistent_free(buf, bytes, dma);//Bruce;;05032005;;
	//} 
    //else
	//	kfree (buf);
#else
		kfree (buf);
#endif
}

/*-------------------------------------------------------------------------*/
//****************************************************
// Name:done
// Description: <1>.DMA memory unmap
//              <2>.Call back complete function
//              
// Input:<1>.ep structure point
//       <2>.req structure point
//       <3>.status
// Output:none
//****************************************************
static void
done(struct FTC_ep *ep, struct FTC_request *req, int status)
{
	struct FTC_udc		*dev;
	unsigned stopped = ep->stopped;
    u32 temp;
   

	DBG_FUNCC("+done()\n");

  //<1>.Check the status
  
	list_del_init(&req->queue);

	if (likely(req->req.status == -EINPROGRESS))
		req->req.status = status;
	else
		status = req->req.status;

	dev = ep->dev;
	
	
	
	
  //<2>.Unmap DMA memory
	if (req->mapped) {
		DBG_CTRLL("....pci_unmap_single len = %d\n",req->req.length);   

		// important : DMA length will set as 16*n bytes
		temp = req->req.length / 16;
		if (req->req.length % 16)
           temp++;
		pci_unmap_single((void *)dev, req->req.dma, temp*16,  //USB_EPX_BUFSIZ,  //req->req.length+32,
			ep->is_in ? PCI_DMA_TODEVICE : PCI_DMA_FROMDEVICE);
		req->req.dma = DMA_ADDR_INVALID;
		req->mapped = 0;
	}

#ifndef USB_TRACE
	if (status && status != -ESHUTDOWN)
#endif
		VDBG(dev, "complete %s req %p stat %d len %u/%u\n",
			ep->ep.name, &req->req, status,
			req->req.actual, req->req.length);

	/* don't modify queue heads during completion callback */
	ep->stopped = 1;
	
	

	
	
  //<3>.call back the complete function call	
    
	spin_unlock(&dev->lock);
	req->req.complete(&ep->ep, &req->req);
	spin_lock(&dev->lock);
    
    
  //<4>.For EP0 => Set done to HW    
    if (ep->num==0)	
       mUsbEP0DoneSet();
	ep->stopped = stopped;


    DBG_CTRLL(">>> (Interrupt Register=0x%x)(IntGroupMask=0x%x)...\n",mUsbIntSrc1MaskRd(),mUsbIntGroupMaskRd());
	DBG_FUNCC("-done() stopped=%d  \n",stopped);
}

/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
//****************************************************
// Name:CX_dma_Directly
// Description: Start DMA directly
//              <1>.For Control Command - Get Stayus (Only for ep0)
// Input:<1>.For Control Command - Get Stayus
//       <2>.status
// Output:none
//****************************************************
static int CX_dma_Directly(u8 *pu8Buffer, u32 u8Num,u8 bdir)
{

	u32            FIFO_Sel,wTemp,wDMABuffer,temp;
    u8             u8fifo_n;
	struct FTC_udc	*dev = the_controller;    
	
	
	DBG_FUNCC("+CX_dma_Directly, start addr = 0x%x\n",(u32)pu8Buffer);


  //<1>.Get the FIFO Select
        u8fifo_n=0;
        FIFO_Sel=FOTG200_DMA2CxFIFO;



  //<2>.Map the DMA Buffer

		temp = u8Num / 16;
		if (u8Num % 16)
           temp++;
		wDMABuffer = pci_map_single((void *)dev, pu8Buffer,  temp*16, //USB_EPX_BUFSIZ 
			bdir ? PCI_DMA_TODEVICE : PCI_DMA_FROMDEVICE);



  //<3>.Init DMA start register
       	// EP=0,1,2,3,4
        if (bdir) 
	        mUsbDmaConfig(u8Num,DIRECTION_IN);
	    else
	        mUsbDmaConfig(u8Num,DIRECTION_OUT);     
	    
	    mUsbDMA2FIFOSel(FIFO_Sel);
	    mUsbDmaAddr((u32)wDMABuffer);

  //<4>.Enable the DMA
  	    mUsbDmaStart();
  	    
  //<5>.Waiting for DMA complete 	    
	while(1)
	{
		wTemp = mUsbIntSrc2Rd();
		if(wTemp & BIT8)
		{
			mUsbIntDmaErrClr();
			printk("??? Cx IN DMA error..");
			break;
		}
		if(wTemp & BIT7)
		{
			mUsbIntDmaFinishClr();
			break;
		}
		if((wTemp & 0x00000007)>0)//If (Resume/Suspend/Reset) exit
		{
			mUsbIntDmaFinishClr();
			printk("???? Cx IN DMA stop because USB Resume/Suspend/Reset..");
			break;
		}
	}
	mUsbDMA2FIFOSel(FOTG200_DMA2FIFO_Non);  
	
	
  //<6>.Unmap the DMA	
		pci_unmap_single((void *)dev,wDMABuffer, temp*16,  //USB_EPX_BUFSIZ,  //req->req.length+32,
			bdir ? PCI_DMA_TODEVICE : PCI_DMA_FROMDEVICE);	
	
		    

	    
  	    
  	    
	return 0;
}
//****************************************************
// Name:start_dma
// Description: Start the DMA
//              For FOTG200-Peripheral HW:
//              <1>.Control-8 byte command => Only PIO
//              <2>.Others => Only DMA 
//              
// Input:<1>.ep structure point
//       <2>.status
// Output:none
//****************************************************
// return:  0 = q running, 1 = q stopped, negative = errno
static int start_dma(struct FTC_ep *ep, struct FTC_request *req)
{
	struct FTC_udc *dev = ep->dev;
	u32		       start = req->req.dma;
	u32            FIFO_Sel=0;
    u8             u8fifo_n;
	DBG_FUNCC("+start_dma, start addr = 0x%x\n",start);



  //<1>.Get the FIFO Select
        u8fifo_n=Get_FIFO_Num(ep);
        if (u8fifo_n==FIFOCX)
            FIFO_Sel=FOTG200_DMA2CxFIFO;
		else
		    FIFO_Sel = 1<<(u8fifo_n);

  //<2>.Init DMA start register
       	// EP=0,1,2,3,4
        if (likely(ep->is_in)) 
	        {
	        ep->wDMA_Set_Length=req->req.length;
	        mUsbDmaConfig(req->req.length,DIRECTION_IN);
	        }
	    else
	        {
	        if ((ep->num)==0)	
	            mUsbDmaConfig(req->req.length,DIRECTION_OUT);     
	        else
	           {//For EP1~EP4
	           	//Read the Byte Counter

	            DBG_CTRLL(">>>start_dma ==>(mUsbFIFOOutByteCount=0x%x(ep->num=0x%x)(MaxPackSize=0x%x))\n"
	                ,mUsbFIFOOutByteCount(((ep->num)-1)),ep->num,mUsbEPMxPtSzRd(((ep->num)),DIRECTION_OUT));
	           	
	           	if (mUsbFIFOOutByteCount(((ep->num)-1))<(mUsbEPMxPtSzRd(((ep->num)),DIRECTION_OUT)))
	                {
	                ep->wDMA_Set_Length=mUsbFIFOOutByteCount(((ep->num)-1));
	                mUsbDmaConfig((ep->wDMA_Set_Length),DIRECTION_OUT);     
	                }
	            else
	                {
	                ep->wDMA_Set_Length=req->req.length;	                
	                mUsbDmaConfig(ep->wDMA_Set_Length,DIRECTION_OUT); 	                

	                }
	           	}
	        
	         }
	    
	    mUsbDMA2FIFOSel(FIFO_Sel);
	    mUsbDmaAddr(start);
	    DBG_CTRLL(">>>(mUsbDmaConfigRd=0x%x(Request Length=0x%x))\n",mUsbDmaConfigRd(),req->req.length);
	    DBG_CTRLL(">>>(mUsbDMA2FIFOSel=0x%x)\n",mUsbDMA2FIFORd());
	    DBG_CTRLL(">>>(mUsbDmaAddr=0x%x)\n",mUsbDmaAddrRd());


  //<3>.Record who use the DMA chanel(In use)
	    dev->EPUseDMA = ep->num;
	    
  //<4>.Disable FIFO-Interrupt
  	    // If use DMA, no FIFO interrupt for FIFO
        //FIFO_Interrupt(ep,0);

  //<5>.Enable the DMA Interrupt
        mUsbIntDmaErrEn();
        mUsbIntDmaFinishEn();
        
  //<6>.Enable the DMA
  	    mUsbDmaStart();
  	    

	    DBG_FUNCC("-start_dma...\n");

	return 0;
}
//****************************************************
// Name:dma_advance
// Description: After finish DMA 
// Input:<1>.dev structure pointer
//       <2>.ep structure pointer
// Output:none
//****************************************************
static void dma_advance(struct FTC_udc *dev, struct FTC_ep *ep)
{

	struct FTC_request	*req;


	DBG_FUNCC("+dma_advance\n");


	if (unlikely(list_empty(&ep->queue))) {
      
stop://Disable DMA)
       mUsbDmaStop();
       mUsbIntDmaErrDis();
       mUsbIntDmaFinishDis();
       mUsbDMA2FIFOSel(FOTG200_DMA2FIFO_Non);
       dev->EPUseDMA =DMA_CHANEL_FREE;
       
       if (unlikely(ep->num))//ep->num>0
          {//Disable the FIFO-Interrupt (ep->Num>0)
          if (likely(ep->is_in))
             mUsbIntFXINDis(((ep->num)-1));
          else
             mUsbIntFXOUTDis(((ep->num)-1));
          }

	   return;
	}
	req = list_entry(ep->queue.next, struct FTC_request, queue);

  //<2>.Get length 
	/* normal hw dma completion (not abort) */
        if (mUsbIntDmaErrRd()==0)
            {
             req->req.actual=ep->wDMA_Set_Length;	
	         DBG_CTRLL(">>> dma_advance=>req->req.actual=0x%x  \n",req->req.actual);
            }
        else    
            {
            printk("??? DMA Error...\n");
            req->req.actual=0; 
            }

#ifdef USB_TRACE
	VDBG(dev, "done %s %s dma, %u/%u bytes, req %p\n",
		ep->ep.name, ep->is_in ? "IN" : "OUT",
		req->req.actual, req->req.length, req);
#endif

  //<3>.Done the request
	done(ep, req, 0);
	if (list_empty(&ep->queue))
		goto stop;
		
  //<4>.Start another req DMA		
        if (ep->num==0)
	       {
	       	req = list_entry(ep->queue.next, struct FTC_request, queue);
	        (void) start_dma(ep, req);
	       }
	    else{
	   

            
            //<1>.Free the DMA resource => Waiting for next DMA-Start 
                  mUsbDmaStop();
                  mUsbIntDmaErrDis();
                  mUsbIntDmaFinishDis();
                  mUsbDMA2FIFOSel(FOTG200_DMA2FIFO_Non);
                  dev->EPUseDMA =DMA_CHANEL_FREE;
            //<2>.open the interrupt
          	 if (likely(ep->is_in))
          	     {
          	     mUsbIntFXINEn(((ep->num)-1));//Enable the Bulk-In
          	     }
          	 else
          	     mUsbIntFXOUTEn(((ep->num)-1));//Enable the Bulk-Out  	    
          	     
          	     
	    

	    }   

}

//****************************************************
// Name:abort_dma
// Description: In FOTG200 abort_dma = reset dma
// Input:<1>.ep structure pointer
//       <2>.Status
// Output:none
//****************************************************
static void abort_dma(struct FTC_ep *ep, int status)
{
	struct FTC_request	*req;
	struct FTC_udc	*dev;
	u8 u8fifo_n;
	
	DBG_FUNCC("+abort_dma\n");

	req = list_entry(ep->queue.next, struct FTC_request, queue);

	/* FIXME using these resets isn't usably documented. this may
	 * not work unless it's followed by disabling the endpoint.
	 *
	 * FIXME the OUT reset path doesn't even behave consistently.
	 */
  
 //<1>.Checking => Finish
	if (mUsbIntDmaFinishRd()>0)
           goto finished;
	req->req.status = status;

	VDBG(ep->dev, "%s %s %s %d/%d\n", __FUNCTION__, ep->ep.name,
		ep->is_in ? "IN" : "OUT",
		req->req.actual, req->req.length);

    mUsbDMAResetSet();
    
    u8fifo_n=Get_FIFO_Num(ep);
	if (u8fifo_n==FIFOCX)
       mUsbCxFClr();
	else
        mUsbFIFOReset(u8fifo_n);

	return;

finished:
	///* dma already completed; no abort needed */
	//command(regs, COMMAND_FIFO_ENABLE, ep->num);
	req->req.actual = req->req.length;
	req->req.status = 0;
	
	mUsbDmaStop();
    mUsbIntDmaErrDis();
    mUsbIntDmaFinishDis();
    mUsbDMA2FIFOSel(FOTG200_DMA2FIFO_Non);
    

    dev = ep->dev; 
    dev->EPUseDMA =DMA_CHANEL_FREE;
	
	
}

//****************************************************
// Name:Ep_ISR
// Description: For Ep-1 In
//             <1>.if queue have data start dma
//             
//
// Input:dev
// Output:none
//****************************************************
void Ep_ISR(struct FTC_udc *dev,u8 epNum)
{
  struct FTC_request	*req;
  struct FTC_ep *ep;
  DBG_FUNCC("+Ep_ISR(epNum=0x%x)\n",epNum);


 //<1>.Checking data in queue ?
       ep=&(dev->ep[epNum]);
       
       if (list_empty(&ep->queue))
          {
          	if (likely(ep->is_in))
          	    mUsbIntFXINDis(((ep->num)-1));//Disable the Bulk--In
          	else
          	    mUsbIntFXOUTDis(((ep->num)-1));//Disable the Bulk-Out
          
          }
       else//data in queue
           {
	        
	       if (dev->EPUseDMA ==DMA_CHANEL_FREE)
              { 
	            //Start the DMA
	            req = list_entry(ep->queue.next, struct FTC_request, queue);
	            (void) start_dma(ep, req);           	
          	  }
         
          	else{
          	
       	//During DMA => Wait for done & disable DMA
          	    if (likely(ep->is_in))
          	        {

          	        mUsbIntFXINDis(((ep->num)-1));//Disable the interrupt-In
          	        }
          	    else
          	        mUsbIntFXOUTDis(((ep->num)-1));//Disable the interrupt-Out          	

          	}

         }   
}


//****************************************************
// Name:FTC_queue
// Description: 
// Input:<1>.ep structure point
//       <2>.status
//       <3>.flag
// Output:none
//****************************************************
static int
FTC_queue(struct usb_ep *_ep, struct usb_request *_req, gfp_t gfp_flags)
{
	struct FTC_request	*req;
	struct FTC_ep		*ep;
	struct FTC_udc		*dev;
	unsigned long		 flags;
	int			status;
    	u32 temp; 

	DBG_FUNCC("+FTC_queue()\n");

 	//<1>.Check request & ep & dev
	/* always require a cpu-view buffer so pio works */
	req = container_of(_req, struct FTC_request, req);
	if (unlikely(!_req || !_req->complete || !_req->buf || !list_empty(&req->queue)))
	{
		DBG_CTRLL("??? FTC_queue => return -EINVAL\n");
		return -EINVAL;
	}

	ep = container_of(_ep, struct FTC_ep, ep);
	if (unlikely(!_ep || (!ep->desc && ep->num != 0)))
	{
		 DBG_CTRLL("??? FTC_queue => return -EINVAL\n");
		 return -EINVAL;
        }

  	//Check CX 0 bytes
    	if (req->req.length==0)
       	{
		if (ep->num==0)
       	   	{
			//request Control Transfer 0 byte
       	   	 	mUsbEP0DoneSet();
       	   	 	mUsbCfgSet();//Temp Solution for Set Configuration
       	   	 	DBG_CTRLL(">>> FTC_queue => return (set configuration)\n");
       	   	 	return 0;
       	   	}
       //else => Other ED 0 bytes
       
       }
	dev = ep->dev;
	if (unlikely(!dev->driver || dev->gadget.speed == USB_SPEED_UNKNOWN))
		{
		 DBG_CTRLL("??? FTC_queue => return -ESHUTDOWN\n");
		 return -ESHUTDOWN;
        }


	/* can't touch registers when suspended */
	if (dev->ep0state == EP0_SUSPEND)
		{
		 DBG_CTRLL("??? FTC_queue => return -EBUSY\n");
		 return -EBUSY;
        }		


	if (ep->dma && _req->dma == DMA_ADDR_INVALID) {
		DBG_CTRLL("....pci_map_single len = %d\n",_req->length);

		// important : DMA length will set as 16*n bytes
		temp = _req->length / 16;
		if (_req->length % 16)
           temp++;
		_req->dma = pci_map_single((void *)dev, _req->buf, temp*16, //USB_EPX_BUFSIZ,  
			ep->is_in ? PCI_DMA_TODEVICE : PCI_DMA_FROMDEVICE);
		req->mapped = 1;
	}


#ifdef USB_TRACE
	VDBG(dev, "%s queue req %p, len %u buf %p\n",
			_ep->name, _req, _req->length, _req->buf);
#endif

 //<2>.Set the req's status ...
	
	spin_lock_irqsave(&dev->lock, flags);

	_req->status = -EINPROGRESS;
	_req->actual = 0;


 //<3>.For control-in => Fource short packet

	/* for ep0 IN without premature status, zlp is required and
	 * writing EOP starts the status stage (OUT).
	 */
	if (unlikely(ep->num == 0 && ep->is_in))
		_req->zero = 1;

	/* kickstart this i/o queue? */
	status = 0;
	
	//Bruce;;if (list_empty(&ep->queue) && likely(!ep->stopped)) {
     //In  => Write data to the FIFO directly
     //Out => Only Enable the FIFO-Read interrupt 

		
	if (list_empty(&ep->queue) && likely(!ep->stopped)  ) {
		//ep->num>0 ==> will add to queue until ed-FIFO-Interrupt be issue
		
		/* dma:  done after dma completion IRQ (or error)
		 * pio:  FOTG200 do not support
		 */


        DBG_CTRLL(">>> dev->EPUseDMA = 0x%x (ep->num=%x)\n",dev->EPUseDMA,ep->num);		
		
		//if ((dev->EPUseDMA) ==DMA_CHANEL_FREE)
		   if ((ep->num)==0) 
		       status = start_dma(ep, req);
 

 
		   if (unlikely(status != 0)) {
			  if (status > 0)
				  status = 0;
			  req = 0;
		    }  	   
	   

	} /* else pio or dma irq handler advances the queue. */

 //Add request to queue
	if (likely(req != 0)) {
        DBG_CTRLL(">>> add request to queue ...\n");
		list_add_tail(&req->queue, &ep->queue);
	}

	
	
 //Enable the FIFO Interrupt	
    if (likely((ep->num)>0))//Open the FIFO interrupt
       {
      	if (likely((ep->is_in)==1))//For In-Ep
      	    mUsbIntFXINEn(((ep->num)-1));  
      	else//For Out-Ep   
      	    mUsbIntFXOUTEn(((ep->num)-1));
  
  	    
        DBG_CTRLL(">>> Enable EP-%x Interrupt (Register=0x%x)(Length=0x%x)...\n"
                   ,ep->num,mUsbIntSrc1MaskRd(),req->req.length);
      	    
       }	
	
	
	spin_unlock_irqrestore(&dev->lock, flags);
	
	
	
	return status;
}
//****************************************************
// Name:nuke
// Description: dequeue ALL requests
// Input:<1>.ep structure point
//       <2>.status
// Output:none
//****************************************************
/* dequeue ALL requests */
static void nuke(struct FTC_ep *ep, int status)
{
	struct FTC_request	*req;

    DBG_FUNCC("+nuke() ep addr= 0x%x\n", (u32) ep);

	ep->stopped = 1;
	if (list_empty(&ep->queue))
		return;

	while (!list_empty(&ep->queue)) {
		req = list_entry(ep->queue.next, struct FTC_request, queue);
		printk("release req = %x\n", (u32) req);
		done(ep, req, status);
	}
}
//****************************************************
// Name:FTC_dequeue
// Description: dequeue JUST ONE request
// Input:<1>.ep structure point
//       <2>.request structure pointer
// Output:none
//****************************************************
/* dequeue JUST ONE request */
static int FTC_dequeue(struct usb_ep *_ep, struct usb_request *_req)
{
	struct FTC_request	*req;
	struct FTC_ep		*ep;
	struct FTC_udc		*dev;
	unsigned long		flags;

	DBG_FUNCC("+FTC_dequeue()\n");

  //<1>.Checking the input
	ep = container_of(_ep, struct FTC_ep, ep);
	if (!_ep || !_req || (!ep->desc && ep->num != 0))
		return -EINVAL;
	dev = ep->dev;
	if (!dev->driver)
		return -ESHUTDOWN;

	/* we can't touch (dma) registers when suspended */
	if (dev->ep0state == EP0_SUSPEND)
		return -EBUSY;

	VDBG(dev, "%s %s %s %s %p\n", __FUNCTION__, _ep->name,
		ep->is_in ? "IN" : "OUT",
		ep->dma ? "dma" : "pio",
		_req);

	spin_lock_irqsave(&dev->lock, flags);

	/* make sure it's actually queued on this endpoint */
	list_for_each_entry (req, &ep->queue, queue) {
		if (&req->req == _req)
			break;
	}
	if (&req->req != _req) {
		spin_unlock_irqrestore (&dev->lock, flags);
		return -EINVAL;
	}

  //<2>.dequeue the current req 
    if (ep->dma && ep->queue.next == &req->queue && !ep->stopped) {
		abort_dma(ep, -ECONNRESET);
		done(ep, req, -ECONNRESET);
		dma_advance(dev, ep);
	} 
	else
	 if (!list_empty(&req->queue))
		done(ep, req, -ECONNRESET);
	else
		req = 0;
	spin_unlock_irqrestore(&dev->lock, flags);

	return req ? 0 : -EOPNOTSUPP;
}
//****************************************************
// Name:FTC_clear_halt
// Description: clear the halt ep
// Input:ep structure point
// Output:none
//****************************************************
/*-------------------------------------------------------------------------*/
static void FTC_clear_halt(struct FTC_ep *ep)
{
   struct FTC_request	*req;
	
	DBG_FUNCC("+FTC_clear_halt()(ep->num=%d)\n",ep->num);
   
	VDBG(ep->dev, "%s clear halt\n", ep->ep.name);

 //<1>.Set register(Rst_Toggle)
    if (ep->num>0)
       { if (ep->is_in)	// IN direction ?
	    {
          DBG_BULKK("FTC_udc==>FTC_clear_halt()==>IN direction \n");
        
	       mUsbEPinRsTgSet(ep->num);	// Set Rst_Toggle Bit
	       mUsbEPinRsTgClr(ep->num);	// Clear Rst_Toggle Bit
	       mUsbEPinStallClr(ep->num);	// Clear Stall Bit
	    }
	    else
	    {
           DBG_BULKK("FTC_udc==>FTC_clear_halt()==>OUT direction \n");
        
	       mUsbEPoutRsTgSet(ep->num);	// Set Rst_Toggle Bit
	       mUsbEPoutRsTgClr(ep->num);	// Clear Rst_Toggle Bit
	       mUsbEPoutStallClr(ep->num);	// Clear Stall Bit
	    }
	   }
	DBG_BULKK("FTC_udc==>FTC_clear_halt()==>ep->stopped = %d\n",ep->stopped);

  //<2>.Start next request
	if (ep->stopped) {
		ep->stopped = 0;

		if (list_empty(&ep->queue))
			return;
		req = list_entry(ep->queue.next, struct FTC_request,
						queue);
		(void) start_dma(ep, req);
	}


}
//****************************************************
// Name:FTC_set_halt
// Description:set halt
// Input:ep structure point
// Output:
//****************************************************
static int FTC_set_halt(struct usb_ep *_ep, int value)
{
	struct FTC_ep	*ep;
	unsigned long	flags;
	int		retval = 0;

	DBG_FUNCC("+FTC_set_halt()\n");

  //<1>.Checking input
	if (!_ep)
		return -ENODEV;
	ep = container_of (_ep, struct FTC_ep, ep);
	
	DBG_BULKK("FTC_set_halt()===> (ep->num=%d)(Value=%d)\n",ep->num,value);

  //<2>.Processing ep=0
	if (ep->num == 0) {
		if (value) {
			ep->dev->ep0state = EP0_STALL;
			ep->dev->ep[0].stopped = 1;
		} else
			return -EINVAL;

	/* don't change EPxSTATUS_EP_INVALID to READY */
	} else if (!ep->desc) {
		VDBG(ep->dev, "%s %s inactive?\n", __FUNCTION__, ep->ep.name);
		return -EINVAL;
	}

	spin_lock_irqsave(&ep->dev->lock, flags);
	if (!list_empty(&ep->queue))
		retval = -EAGAIN;
	else if (!value)
		FTC_clear_halt(ep);
	else {
		ep->stopped = 1;
		VDBG(ep->dev, "%s set halt\n", ep->ep.name);
	    if (ep->is_in)	// IN direction ?
	        mUsbEPinStallSet(ep->num);		// Set in Stall Bit
	    else
	        mUsbEPoutStallSet(ep->num);		// Set out Stall Bit
	}
	spin_unlock_irqrestore(&ep->dev->lock, flags);
	return retval;

}
//****************************************************
// Name:FTC_fifo_status
// Description:Get the size of data in FIFO-X
// Input:ep structure point
// Output:size
//****************************************************
int FTC_fifo_status(struct usb_ep *_ep)
{
	struct FTC_ep *ep;

	u32	   size;

	DBG_FUNCC("+FTC_fifo_status() => In FOTG200 => Only support DMA mode... \n");

	if (!_ep)
		return -ENODEV;
	ep = container_of(_ep, struct FTC_ep, ep);

    size =0 ;
	return size;
}
//****************************************************
// Name:FTC_fifo_flush
// Description:Clear the FIFO 
// Input:ep structure pointer
// Output:none
//****************************************************
static void FTC_fifo_flush(struct usb_ep *_ep)
{
	struct FTC_ep *ep;
    u8     u8fifo_n;   //john

	DBG_FUNCC("+FTC_fifo_flush()\n");

	if (!_ep)
		return;
	ep = container_of(_ep, struct FTC_ep, ep);
	VDBG(ep->dev, "%s %s\n", __FUNCTION__, ep->ep.name);

	/* don't change EPxSTATUS_EP_INVALID to READY */
	if (!ep->desc && ep->num != 0) {
		VDBG(ep->dev, "%s %s inactive?\n", __FUNCTION__, ep->ep.name);
		return;
	}

	//For OTG200
	if (ep->num ==0) {   //EP0 
       mUsbCxFClr();
	}
    else {
       u8fifo_n = mUsbEPMapRd(ep->num);		// get the relatived FIFO number
	   if (ep->is_in)
	      u8fifo_n &= 0x0F;
	   else
	      u8fifo_n >>= 4;
       if (u8fifo_n >= MAX_FIFO_NUM)	// over the Max. fifo count ?
          return;

	   // Check the FIFO had been enable ?
	   if ((mUsbFIFOConfigRd(u8fifo_n) & FIFOEnBit) == 0)
	      return;

       mUsbFIFOReset(u8fifo_n);   //reset FIFO
       udelay(10);
       mUsbFIFOResetOK(u8fifo_n);   //reset FIFO finish
	}
	return;
}

static struct usb_ep_ops FTC_ep_ops = {
	.enable		= FTC_ep_enable,
	.disable	= FTC_ep_disable,

	.alloc_request	= FTC_alloc_request,
	.free_request	= FTC_free_request,

	//.alloc_buffer	= FTC_alloc_buffer,
	//.free_buffer	= FTC_free_buffer,

	.queue		= FTC_queue,
	.dequeue	= FTC_dequeue,

	.set_halt	= FTC_set_halt,
	.fifo_status    = FTC_fifo_status,
	.fifo_flush	= FTC_fifo_flush,
};

/*-------------------------------------------------------------------------*/

static int FTC_get_frame(struct usb_gadget *_gadget)
{
	struct FTC_udc	*dev;
	u16 retval;
	unsigned long	flags;

	DBG_FUNCC("+FTC_get_frame()\n");

	if (!_gadget)
		return -ENODEV;
	dev = container_of (_gadget, struct FTC_udc, gadget);
	spin_lock_irqsave (&dev->lock, flags);
	retval = mUsbFrameNo();
	spin_unlock_irqrestore (&dev->lock, flags);

	return retval;
}

static int FTC_wakeup(struct usb_gadget *_gadget)
{
	struct FTC_udc	*dev;
	unsigned long	flags;
 
    DBG_FUNCC("+FTC_wakeup()\n");

	if (!_gadget)
		return -ENODEV;
	dev = container_of (_gadget, struct FTC_udc, gadget);
	spin_lock_irqsave (&dev->lock, flags);
        
        DBG_TEMP_FUN(">>> dev->devstat=%d\n",dev->devstat);
//For OTG Start
	
	if (mUsb_TGC_Control_A_VBUS_VLD_Rd()) {
		/* NOTE:  OTG spec erratum says that OTG devices may
		 * issue wakeups without host enable.
		 */
		//if (dev->devstat & (UDC_B_HNP_ENABLE|UDC_R_WK_OK)) {
		INFO(dev,"remote wakeup...\n");
 	        // Set "Device_Remote_Wakeup", Turn on the"RMWKUP" bit in Mode Register
	        mUsbRmWkupSet();

		

	/* NOTE:  non-OTG systems may use SRP TOO... */
	} else if (dev->transceiver)
		   otg_start_srp(dev->transceiver);
	

//For OTG End

	spin_unlock_irqrestore (&dev->lock, flags);

	return 0;
}

static int FTC_set_selfpowered(struct usb_gadget *_gadget, int value)
{
	DBG_FUNCC("+FTC_set_selfpowered()\n");

	return -EOPNOTSUPP;
}

static int FTC_ioctl(struct usb_gadget *_gadget, unsigned code, unsigned long param)
{
	unsigned long	flags;
	struct FTC_udc	*dev;
	struct FTC_ep	*ep;
	struct usb_ep   *_ep;

	DBG_FUNCC("+FTC_ioctl()\n");

	if (!_gadget)
		return -ENODEV;
	dev = container_of (_gadget, struct FTC_udc, gadget);
	spin_lock_irqsave (&dev->lock, flags);

	switch (code) {
    case 1:   //DMA enable from others
       _ep = (struct usb_ep *)param;       
	   ep = container_of(_ep, struct FTC_ep, ep);
       ep->dma=1;
       break;
    case 2:   //DMA disable from others
       _ep = (struct usb_ep *)param;       
	   ep = container_of(_ep, struct FTC_ep, ep);
	   printk("??? Error => FOTG200 only support DMA Mode...(DMA can not be disabled)\n");
       ep->dma=1;
       break;
    default:
	   break;
	}

	spin_unlock_irqrestore (&dev->lock, flags);

	return -EOPNOTSUPP;
}

static const struct usb_gadget_ops FTC_ops = {
	.get_frame	     = FTC_get_frame,
	.wakeup		     = FTC_wakeup,
	.set_selfpowered = FTC_set_selfpowered,
	.ioctl           = FTC_ioctl,
};

/*-------------------------------------------------------------------------*/

/////////////////////////////////////////////////////
//		clrFIFORegister(void)
//		Description:
//		input: none
//		output: none
/////////////////////////////////////////////////////
static void clrFIFORegister(void)
{

	u8 u8ep;	
	
    mUsbEPMapAllClr();
	mUsbFIFOMapAllClr();
	mUsbFIFOConfigAllClr();
	

	for (u8ep = 1; u8ep <= MAX_FIFO_NUM; u8ep ++)
	{
		mUsbEPMxPtSzClr(u8ep,DIRECTION_IN);	
		mUsbEPMxPtSzClr(u8ep,DIRECTION_OUT);	
	}		
	


}
//////////////////////////////////////////////////////////////////////////////////////////////////
// config FIFO
//-----------------------------------------------------------------------
/////////////////////////////////////////////////////
//		vUsbFIFO_EPxCfg_HS(void)
//		Description:
//			1. Configure the FIFO and EPx map
//		input: none
//		output: none
/////////////////////////////////////////////////////
void vUsbFIFO_EPxCfg_FS(void)
{
	int i;

	DBG_FUNCC("+vUsbFIFO_EPxCfg_FS()\n");

    DBG_CTRLL("FIFO-Start:0~3\n");  
    DBG_CTRLL("Dir:Out=>1 / In =>0\n"); 
    DBG_CTRLL("BLKSize:1=>64bytes / 2 =>128 bytes\n"); 
    DBG_CTRLL("MaxPackSize:Max=64 bytes\n"); 
    DBG_CTRLL("IFO-Use-Num:1=>Single / 2=>Double / 3=>TRIBLE\n"); 
    DBG_CTRLL("FULL_ED4_bTYPE:0=>Control / 1=>ISO / 2=>Bulk / 3=>Interrupt\n"); 

	
	clrFIFORegister();
	
	//EP4
	   mUsbEPMap(EP4, FULL_EP4_Map);
	   mUsbFIFOMap(FULL_ED4_FIFO_START, FULL_EP4_FIFO_Map);
	   mUsbFIFOConfig(FULL_ED4_FIFO_START,FULL_EP4_FIFO_Config);
	   								
	   for(i = (FULL_ED4_FIFO_START + 1) ;
	   	i < (FULL_ED4_FIFO_START + (FULL_ED4_bBLKNO *FULL_ED4_bBLKSIZE)) ;
	       i ++)
	   {
	   	mUsbFIFOConfig(i, (FULL_EP4_FIFO_Config & (~BIT7)) );
	   }
   
	   mUsbEPMxPtSz(EP4, FULL_ED4_bDIRECTION, (FULL_ED4_MAXPACKET & 0x7ff) );
	   mUsbEPinHighBandSet(EP4 , FULL_ED4_bDIRECTION ,FULL_ED4_MAXPACKET);
       DBG_CTRLL("EP4 Config = (FIFO-Start=0x%x) (Dir=0x%x)(BLKSize=0x%x)(MaxPackSize=0x%x)\n"
                 ,FULL_ED4_FIFO_START,FULL_ED4_bDIRECTION,FULL_ED4_bBLKSIZE,FULL_ED4_MAXPACKET);  
       DBG_CTRLL("             (FIFO-Use-Num=0x%x) (Type=0x%x)\n"
                 ,FULL_ED4_bBLKNO,FULL_ED4_bTYPE);                   

       DBG_CTRLL("Register Dump (mUsbEPMap=0x%x) (mUsbFIFOMap=0x%x)(mUsbFIFOConfig=0x%x)\n"
                 ,mUsbEPMap1_4Rd(),mUsbFIFOMapAllRd(),mUsbFIFOConfigAllRd());  


	//EP3
	    mUsbEPMap(EP3, FULL_EP3_Map);
	    mUsbFIFOMap(FULL_ED3_FIFO_START, FULL_EP3_FIFO_Map);
	    mUsbFIFOConfig(FULL_ED3_FIFO_START, FULL_EP3_FIFO_Config);
	    
	   for(i = FULL_ED3_FIFO_START + 1 ;
	   	i < FULL_ED3_FIFO_START + (FULL_ED3_bBLKNO *FULL_ED3_bBLKSIZE) ;
	       i ++)
	   {
	   	mUsbFIFOConfig(i, (FULL_EP3_FIFO_Config & (~BIT7)) );
	   }
	   
	   mUsbEPMxPtSz(EP3, FULL_ED3_bDIRECTION, (FULL_ED3_MAXPACKET & 0x7ff) );
	   mUsbEPinHighBandSet(EP3 , FULL_ED3_bDIRECTION , FULL_ED3_MAXPACKET);
       DBG_CTRLL("EP3 Config = (FIFO-Start=0x%x) (Dir=0x%x)(BLKSize=0x%x)(MaxPackSize=0x%x)\n"
                 ,FULL_ED3_FIFO_START,FULL_ED3_bDIRECTION,FULL_ED3_bBLKSIZE,FULL_ED3_MAXPACKET);  
       DBG_CTRLL("             (FIFO-Use-Num=0x%x) (Type=0x%x)\n"
                 ,FULL_ED3_bBLKNO,FULL_ED3_bTYPE);                   


       DBG_CTRLL("Register Dump (mUsbEPMap=0x%x) (mUsbFIFOMap=0x%x)(mUsbFIFOConfig=0x%x)\n"
                 ,mUsbEPMap1_4Rd(),mUsbFIFOMapAllRd(),mUsbFIFOConfigAllRd());  

	
    //EP2
	    mUsbEPMap(EP2, FULL_EP2_Map);
	    mUsbFIFOMap(FULL_ED2_FIFO_START, FULL_EP2_FIFO_Map);
	    mUsbFIFOConfig(FULL_ED2_FIFO_START, FULL_EP2_FIFO_Config);
	    
	   for(i = FULL_ED2_FIFO_START + 1 ;
	   	i < FULL_ED2_FIFO_START + (FULL_ED2_bBLKNO *FULL_ED2_bBLKSIZE) ;
	       i ++)
	   {
	   	mUsbFIFOConfig(i, (FULL_EP2_FIFO_Config & (~BIT7)) );
	   }
	   
	   mUsbEPMxPtSz(EP2, FULL_ED2_bDIRECTION, (FULL_ED2_MAXPACKET & 0x7ff) );
	   mUsbEPinHighBandSet(EP2 , FULL_ED2_bDIRECTION , FULL_ED2_MAXPACKET);
       DBG_CTRLL("EP2 Config = (FIFO-Start=0x%x) (Dir=0x%x)(BLKSize=0x%x)(MaxPackSize=0x%x)\n"
                 ,FULL_ED2_FIFO_START,FULL_ED2_bDIRECTION,FULL_ED2_bBLKSIZE,FULL_ED2_MAXPACKET);  
       DBG_CTRLL("             (FIFO-Use-Num=0x%x) (Type=0x%x)\n"
                 ,FULL_ED2_bBLKNO,FULL_ED2_bTYPE);                   

       DBG_CTRLL("Register Dump (mUsbEPMap=0x%x) (mUsbFIFOMap=0x%x)(mUsbFIFOConfig=0x%x)\n"
                 ,mUsbEPMap1_4Rd(),mUsbFIFOMapAllRd(),mUsbFIFOConfigAllRd());  




   //EP1
	    mUsbEPMap(EP1, FULL_EP1_Map);
	    mUsbFIFOMap(FULL_ED1_FIFO_START, FULL_EP1_FIFO_Map);
	    mUsbFIFOConfig(FULL_ED1_FIFO_START, FULL_EP1_FIFO_Config);
	    
	   for(i = FULL_ED1_FIFO_START + 1 ;
	   	i < FULL_ED1_FIFO_START + (FULL_ED1_bBLKNO *FULL_ED1_bBLKSIZE) ;
	       i ++)
	   {
	   	mUsbFIFOConfig(i, (FULL_EP1_FIFO_Config & (~BIT7)) );
	   }
	   
	   mUsbEPMxPtSz(EP1, FULL_ED1_bDIRECTION, (FULL_ED1_MAXPACKET & 0x7ff) );
	   mUsbEPinHighBandSet(EP1 , FULL_ED1_bDIRECTION , FULL_ED1_MAXPACKET);
       DBG_CTRLL("EP1 Config = (FIFO-Start=0x%x) (Dir=0x%x)(BLKSize=0x%x)(MaxPackSize=0x%x)\n"
                 ,FULL_ED1_FIFO_START,FULL_ED1_bDIRECTION,FULL_ED1_bBLKSIZE,FULL_ED1_MAXPACKET);  
       DBG_CTRLL("             (FIFO-Use-Num=0x%x) (Type=0x%x)\n"
                 ,FULL_ED1_bBLKNO,FULL_ED1_bTYPE);                   
				
       DBG_CTRLL("Register Dump (mUsbEPMap=0x%x) (mUsbFIFOMap=0x%x)(mUsbFIFOConfig=0x%x)\n"
                 ,mUsbEPMap1_4Rd(),mUsbFIFOMapAllRd(),mUsbFIFOConfigAllRd());  
       
     

}

void vUsbFIFO_EPxCfg_HS(void)
{
    int i;
	DBG_FUNCC("+vUsbFIFO_EPxCfg_HS()\n");
    
    DBG_CTRLL("FIFO-Start:0~3\n");  
    DBG_CTRLL("Dir:Out=>1 / In =>0\n"); 
    DBG_CTRLL("BLKSize:1=>512bytes / 2 =>1024 bytes\n"); 
    DBG_CTRLL("MaxPackSize:Max=1023 bytes\n"); 
    DBG_CTRLL("IFO-Use-Num:1=>Single / 2=>Double / 3=>TRIBLE\n"); 
    DBG_CTRLL("FULL_ED4_bTYPE:0=>Control / 1=>ISO / 2=>Bulk / 3=>Interrupt\n"); 				
    
    clrFIFORegister();
    
	//EP4
	   mUsbEPMap(EP4, HIGH_EP4_Map);
	   mUsbFIFOMap(HIGH_ED4_FIFO_START, HIGH_EP4_FIFO_Map);
	   mUsbFIFOConfig(HIGH_ED4_FIFO_START,HIGH_EP4_FIFO_Config);
	   								
	   for(i = HIGH_ED4_FIFO_START + 1 ;
	   	i < HIGH_ED4_FIFO_START + (HIGH_ED4_bBLKNO *HIGH_ED4_bBLKSIZE) ;
	       i ++)
	   {
	   	mUsbFIFOConfig(i, (HIGH_EP4_FIFO_Config & (~BIT7)) );
	   }
	   
	   mUsbEPMxPtSz(EP4, HIGH_ED4_bDIRECTION, (HIGH_ED4_MAXPACKET & 0x7ff) );
	   mUsbEPinHighBandSet(EP4 , HIGH_ED4_bDIRECTION , HIGH_ED4_MAXPACKET);
       DBG_CTRLL("EP4 Config = (FIFO-Start=0x%x) (Dir=0x%x)(BLKSize=0x%x)(MaxPackSize=0x%x)\n"
                 ,HIGH_ED4_FIFO_START,HIGH_ED4_bDIRECTION,HIGH_ED4_bBLKSIZE,HIGH_ED4_MAXPACKET);  
       DBG_CTRLL("             (FIFO-Use-Num=0x%x) (Type=0x%x)\n"
                 ,HIGH_ED4_bBLKNO,HIGH_ED4_bTYPE);                   

	//EP3
	    mUsbEPMap(EP3, HIGH_EP3_Map);
	    mUsbFIFOMap(HIGH_ED3_FIFO_START, HIGH_EP3_FIFO_Map);
	    mUsbFIFOConfig(HIGH_ED3_FIFO_START, HIGH_EP3_FIFO_Config);
	    
	   for(i = HIGH_ED3_FIFO_START + 1 ;
	   	i < HIGH_ED3_FIFO_START + (HIGH_ED3_bBLKNO *HIGH_ED3_bBLKSIZE) ;
	       i ++)
	   {
	   	mUsbFIFOConfig(i, (HIGH_EP3_FIFO_Config & (~BIT7)) );
	   }
	   
	   mUsbEPMxPtSz(EP3, HIGH_ED3_bDIRECTION, (HIGH_ED3_MAXPACKET & 0x7ff) );
	   mUsbEPinHighBandSet(EP3 , HIGH_ED3_bDIRECTION ,HIGH_ED3_MAXPACKET);
       DBG_CTRLL("EP3 Config = (FIFO-Start=0x%x) (Dir=0x%x)(BLKSize=0x%x)(MaxPackSize=0x%x)\n"
                 ,HIGH_ED3_FIFO_START,HIGH_ED3_bDIRECTION,HIGH_ED3_bBLKSIZE,HIGH_ED3_MAXPACKET);  
       DBG_CTRLL("             (FIFO-Use-Num=0x%x) (Type=0x%x)\n"
                 ,HIGH_ED3_bBLKNO,HIGH_ED3_bTYPE);                   

	
    //EP2
	    mUsbEPMap(EP2, HIGH_EP2_Map);
	    mUsbFIFOMap(HIGH_ED2_FIFO_START, HIGH_EP2_FIFO_Map);
	    mUsbFIFOConfig(HIGH_ED2_FIFO_START, HIGH_EP2_FIFO_Config);
	    
	   for(i = HIGH_ED2_FIFO_START + 1 ;
	   	i < HIGH_ED2_FIFO_START + (HIGH_ED2_bBLKNO *HIGH_ED2_bBLKSIZE) ;
	       i ++)
	   {
	   	mUsbFIFOConfig(i, (HIGH_EP2_FIFO_Config & (~BIT7)) );
	   }
	   
	   mUsbEPMxPtSz(EP2, HIGH_ED2_bDIRECTION, (HIGH_ED2_MAXPACKET & 0x7ff) );
	   mUsbEPinHighBandSet(EP2 , HIGH_ED2_bDIRECTION , HIGH_ED2_MAXPACKET);
       DBG_CTRLL("EP2 Config = (FIFO-Start=0x%x) (Dir=0x%x)(BLKSize=0x%x)(MaxPackSize=0x%x)\n"
                 ,HIGH_ED2_FIFO_START,HIGH_ED2_bDIRECTION,HIGH_ED2_bBLKSIZE,HIGH_ED2_MAXPACKET);  
       DBG_CTRLL("             (FIFO-Use-Num=0x%x) (Type=0x%x)\n"
                 ,HIGH_ED2_bBLKNO,HIGH_ED2_bTYPE);                   

   //EP1
	    mUsbEPMap(EP1, HIGH_EP1_Map);
	    mUsbFIFOMap(HIGH_ED1_FIFO_START, HIGH_EP1_FIFO_Map);
	    mUsbFIFOConfig(HIGH_ED1_FIFO_START, HIGH_EP1_FIFO_Config);
	    
	   for(i = HIGH_ED1_FIFO_START + 1 ;
	   	i < HIGH_ED1_FIFO_START + (HIGH_ED1_bBLKNO *HIGH_ED1_bBLKSIZE) ;
	       i ++)
	   {
	   	mUsbFIFOConfig(i, (HIGH_EP1_FIFO_Config & (~BIT7)) );
	   }
	   
	   mUsbEPMxPtSz(EP1, HIGH_ED1_bDIRECTION, (HIGH_ED1_MAXPACKET & 0x7ff) );
	   mUsbEPinHighBandSet(EP1 , HIGH_ED1_bDIRECTION , HIGH_ED1_MAXPACKET);
       DBG_CTRLL("EP1 Config = (FIFO-Start=0x%x) (Dir=0x%x)(BLKSize=0x%x)(MaxPackSize=0x%x)\n"
                 ,HIGH_ED1_FIFO_START,HIGH_ED1_bDIRECTION,HIGH_ED1_bBLKSIZE,HIGH_ED1_MAXPACKET);  
       DBG_CTRLL("             (FIFO-Use-Num=0x%x) (Type=0x%x)\n"
                 ,HIGH_ED1_bBLKNO,HIGH_ED1_bTYPE);                   
       DBG_CTRLL("Register Dump (mUsbEPMap=0x%x) (mUsbFIFOMap=0x%x)(mUsbFIFOConfig=0x%x)\n"
                 ,mUsbEPMap1_4Rd(),mUsbFIFOMapAllRd(),mUsbFIFOConfigAllRd());  
 
}

/*-------------------------------------------------------------------------*/
// Faraday USB initial code

///////////////////////////////////////////////////////////////////////////////
//		vFOTG200_Dev_Init()
//		Description:
//			1. Turn on the "Global Interrupt Enable" bit of FOTG200-P
//			2. Turn on the "Chip Enable" bit of FOTG200
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
static void vFOTG200_Dev_Init(void)
{
	DBG_FUNCC("+vFOTG200_Dev_Init()\n");

	// Clear interrupt
	mUsbIntBusRstClr();
	mUsbIntSuspClr();
	mUsbIntResmClr();
	
	// Disable all fifo interrupt
	mUsbIntFIFO0_3OUTDis();
	mUsbIntFIFO0_3INDis();

	
	// Soft Reset
	mUsbSoftRstSet(); 			// All circuit change to which state after Soft Reset?
	mUsbSoftRstClr();
	
	// Clear all fifo
	mUsbClrAllFIFOSet();		// will be cleared after one cycle.

	//Bruce;;Clear mUsbIntEP0EndDis
	mUsbIntEP0EndDis();
    mUsbIntEP0InDis();//We will use DMA-finish to instead of it
    mUsbIntEP0OutDis();//We will use DMA-finish to instead of it
	
}

///////////////////////////////////////////////////////////////////////////////
//		vUsbInit(struct FTC_udc *dev)
//		Description:
//			1. Configure the FIFO and EPx map.
//			2. Init FOTG200-Peripheral.
//			3. Set the usb interrupt source as edge-trigger.
//			4. Enable Usb interrupt.
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
static void vUsbInit(struct FTC_udc *dev)
{
	DBG_FUNCC("+vUsbInit()\n");

	// init variables
	dev->u16TxRxCounter = 0;
	dev->eUsbCxCommand = CMD_VOID;
	dev->u8UsbConfigValue = 0;
	dev->u8UsbInterfaceValue = 0;
	dev->u8UsbInterfaceAlternateSetting = 0;

	// init hardware
	vFOTG200_Dev_Init();
}

/*-------------------------------------------------------------------------*/
//******************************************************************************
// Name:udc_reinit
// Description:
// 
//
//******************************************************************************
static void udc_reinit (struct FTC_udc *dev)
{
	unsigned i;

	DBG_FUNCC("+udc_reinit()\n");

	INIT_LIST_HEAD (&dev->gadget.ep_list);
	dev->gadget.ep0 = &dev->ep [0].ep;
	dev->gadget.speed = USB_SPEED_UNKNOWN;
	dev->ep0state = EP0_DISCONNECT;
	dev->irqs = 0;

	for (i = 0; i <= MAX_EP_NUM; i++) {
		struct FTC_ep	*ep = &dev->ep[i];

		ep->num = i;
		ep->ep.name = names[i];

        DBG_CTRLL("EP%d Name = %s\n",i, ep->ep.name);

		ep->ep.ops = &FTC_ep_ops;
		list_add_tail (&ep->ep.ep_list, &dev->gadget.ep_list);
		ep->dev = dev;
		INIT_LIST_HEAD (&ep->queue);

		ep_reset(ep);
	}

	for (i = 0; i <= MAX_EP_NUM; i++) 
		dev->ep[i].irqs = 0;

	dev->ep[0].ep.maxpacket = MAX_EP0_SIZE;
	list_del_init (&dev->ep[0].ep.ep_list);
}

static void udc_reset(struct FTC_udc *dev)
{
	u8 i;
	
	DBG_FUNCC("+udc_reset()\n");

	//default value
	dev->Dma_Status = DMA_Mode;
	
	dev->u8LineCount = 0;
	INFO(dev,"***** FOTG200 Peripheral 2.0 Test program *****\n");
	INFO(dev,"L%x: System initial, Please wait...\n", dev->u8LineCount ++);

   
    mUsbDMAResetSet();

    for (i=0;i<4;i++)
        mUsbFIFOReset(i);


	// initial Reg setup
	mUsbUnPLGClr();             // 0x08 BIT0

 	vUsbInit(dev);
	dev->EPUseDMA = DMA_CHANEL_FREE;
	
	INFO(dev,"L%x: System is ready(dev->EPUseDMA=0x%x)...\n", dev->u8LineCount ++,dev->EPUseDMA);

}

static void udc_enable(struct FTC_udc *dev)
{
	DBG_FUNCC("+udc_enable()\n");

	// Enable usb200 global interrupt
	mUsbGlobIntEnSet();
	mUsbChipEnSet();
}

/*-------------------------------------------------------------------------*/

/* keeping it simple:
 * - one bus driver, initted first;
 * - one function driver, initted second
 */

/* when a driver is successfully registered, it will receive
 * control requests including set_configuration(), which enables
 * non-control requests.  then usb traffic follows until a
 * disconnect is reported.  then a host may connect again, or
 * the driver might get unbound.
 */
int usb_gadget_register_driver(struct usb_gadget_driver *driver)
{
	struct FTC_udc	*dev = the_controller;
	int	   retval,status;
    //u8 i;
 
	DBG_FUNCC("+usb_gadget_register_driver()\n");

	if (!driver
			|| driver->speed != USB_SPEED_HIGH
			|| !driver->bind
			|| !driver->unbind
			|| !driver->disconnect
			|| !driver->setup)
		return -EINVAL;
	if (!dev)
		return -ENODEV;
	if (dev->driver)
		return -EBUSY;


	/* hook up the driver */
	dev->driver = driver;
	retval = driver->bind(&dev->gadget);
	if (retval) {
		DBG(dev, "bind to driver %s --> error %d\n",
				driver->driver.name, retval);
		dev->driver = 0;
		return retval;
	}

	/* then enable host detection and ep0; and we're ready
	 * for set_configuration as well as eventual disconnect.
	 */
	udc_enable(dev);

//For OTG;;Start
	if (dev->transceiver) {
		status = otg_set_peripheral(dev->transceiver, &dev->gadget);
		if (status < 0) {
			ERROR(dev,"can't bind to transceiver\n");
			driver->unbind (&dev->gadget);

			dev->driver = 0;
		    return status;
		}
	}
//For OTG;;End

	DBG(dev, "registered gadget driver '%s'\n", driver->driver.name);

	DBG_FUNCC("-usb_gadget_register_driver()\n");

	return 0;
}
EXPORT_SYMBOL(usb_gadget_register_driver);

/*
static void
stop_activity(struct goku_udc *dev, struct usb_gadget_driver *driver)
{
	unsigned	i;

	DBG (dev, "%s\n", __FUNCTION__);

	if (dev->gadget.speed == USB_SPEED_UNKNOWN)
		driver = 0;

	// disconnect gadget driver after quiesceing hw and the driver 
	udc_reset (dev);
	for (i = 0; i <= MAX_EP_NUM; i++)
		nuke(&dev->ep [i], -ESHUTDOWN);
	if (driver) {
		spin_unlock(&dev->lock);
		driver->disconnect(&dev->gadget);
		spin_lock(&dev->lock);
	}

	if (dev->driver)
		udc_enable(dev);
}
*/

int usb_gadget_unregister_driver(struct usb_gadget_driver *driver)
{
	struct FTC_udc	*dev = the_controller;
	unsigned long	flags;

	DBG_FUNCC("+usb_gadget_unregister_driver()\n");

	if (!dev)
		return -ENODEV;
	if (!driver || driver != dev->driver)
		return -EINVAL;

	spin_lock_irqsave(&dev->lock, flags);

//For OTG;;Start	
    if (dev->transceiver)
		(void) otg_set_peripheral(dev->transceiver, 0);
//For OTG;;End	
	
	dev->driver = 0;
	//john stop_activity(dev, driver);
	spin_unlock_irqrestore(&dev->lock, flags);

	driver->unbind(&dev->gadget);

	DBG(dev, "unregistered driver '%s'\n", driver->driver.name);
	return 0;
}
EXPORT_SYMBOL(usb_gadget_unregister_driver);


/*-------------------------------------------------------------------------*/

///////////////////////////////////////////////////////////////////////////////
//		vUsb_rst(struct FTC_udc	*dev)
//		Description:
//			1. Change descriptor table (High or Full speed).
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
static void vUsb_rst(struct FTC_udc	*dev)
{

	DBG_FUNCC("+vUsb_rst()\n");		

// stop
	INFO(dev,"L%x, Bus reset\n", dev->u8LineCount ++);
	 
	mUsbIntBusRstClr();
	dev->gadget.speed = USB_SPEED_UNKNOWN;
	
	

	
}

///////////////////////////////////////////////////////////////////////////////
//		vUsb_suspend(dev)
//		Description:
//			1. .
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
static void vUsb_suspend(struct FTC_udc	*dev)
{
	DBG_FUNCC("+vUsb_suspend()\n");

	INFO(dev,"L%x, Bus suspend\n", dev->u8LineCount ++);
	// uP must do-over everything it should handle and do before into the suspend mode
	// Go Suspend status



	mUsbIntSuspClr();
	//Bruce;;mUsbGoSuspend();
    dev->gadget.b_hnp_enable = 0;  
    dev->ep0state = EP0_SUSPEND;
}

///////////////////////////////////////////////////////////////////////////////
//		vUsb_resm(struct FTC_udc	*dev)
//		Description:
//			1. Change descriptor table (High or Full speed).
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
static void vUsb_resm(struct FTC_udc	*dev)
{
	DBG_FUNCC("+vUsb_resm()\n");

	INFO(dev,"L%x, Bus resume\n", dev->u8LineCount ++);
	// uP must do-over everything it should handle and do before into the suspend mode
	// uP must wakeup immediately
	mUsbIntResmClr();

    dev->ep0state = EP0_IDLE;
}



void vUsbClrEPx(void)
{
	u8 u8ep;

	DBG_FUNCC("+vUsbClrEPx()\n");

	// Clear All EPx Toggle Bit
	for (u8ep = 1; u8ep <= MAX_EP_NUM; u8ep ++)
	{
		mUsbEPinRsTgSet(u8ep);
		mUsbEPinRsTgClr(u8ep);
	}
	for (u8ep = 1; u8ep <= MAX_EP_NUM; u8ep ++)
	{
		mUsbEPoutRsTgSet(u8ep);
		mUsbEPoutRsTgClr(u8ep);
	}
}


///////////////////////////////////////////////////////////////////////////////
//		bGet_status(struct FTC_udc *dev)
//		Description:
//			1. Send 2 bytes status to host.
//		input: none
//		output: TRUE or FALSE (u8)
///////////////////////////////////////////////////////////////////////////////
static u8 bGet_status(struct FTC_udc *dev,const struct usb_ctrlrequest *ctrl)
{
    
	u8 u8ep_n,u8fifo_n,RecipientStatusLow, RecipientStatusHigh;
	u8 u8Tmp[2];
	u8 bdir;
	
	DBG_FUNCC("+bGet_status()  \n");

	RecipientStatusLow = 0;
	RecipientStatusHigh = 0;
	switch((ctrl->bRequestType)&0x3)  // Judge which recipient type is at first
	{
    		case 0:					// Device
        	// Return 2-byte's Device status (Bit1:Remote_Wakeup, Bit0:Self_Powered) to Host
        	// Notice that the programe sequence of RecipientStatus
			RecipientStatusLow = mUsbRmWkupST() << 1;
			// Bit0: Self_Powered--> DescriptorTable[0x23], D6(Bit 6)
			// Now we force device return data as self power. (Andrew)
			RecipientStatusLow |= ((USB_CONFIG_ATT_SELFPOWER >> 6) & 0x01);
        	break;
 		case 1:					// Interface
			// Return 2-byte ZEROs Interface status to Host
    		break;

		case 2:					// Endpoint
			if(ctrl->wIndex == 0x00)
			{
				if(dev->ep0state == EP0_STALL)
	       			RecipientStatusLow = TRUE;
			}
			else
			{
				u8ep_n = (u8)ctrl->wIndex  & 0x7F;		// which ep will be clear
				bdir = (u8)ctrl->wIndex >> 7;			// the direction of this ep
				if (u8ep_n > MAX_EP_NUM)			// over the Max. ep count ?
					{return FALSE;
				    
					}
				else
				{
					u8fifo_n = mUsbEPMapRd(u8ep_n);		// get the relatived FIFO number
					if (bdir == 1)
						u8fifo_n &= 0x0F;
					else
						u8fifo_n >>= 4;
					if (u8fifo_n >= MAX_FIFO_NUM)	// over the Max. fifo count ?
						{
						 
							return FALSE;
						}

														// Check the FIFO had been enable ?

					if (bdir == 1)						// IN direction ?
						RecipientStatusLow = mUsbEPinStallST(u8ep_n);
					else
						RecipientStatusLow = mUsbEPoutStallST(u8ep_n);
				}
			}
        	break;
		default :
		   
			return FALSE;
	}

	// return RecipientStatus;
	u8Tmp[0] = RecipientStatusLow;
	u8Tmp[1] = RecipientStatusHigh;
		
	//Use DMA to transfer data
	CX_dma_Directly(u8Tmp,2,1);

	
	mUsbEP0DoneSet();	
	
	return TRUE; 
}

///////////////////////////////////////////////////////////////////////////////
//		bClear_feature(struct FTC_udc *dev)
//		Description:
//			1. Send 2 bytes status to host.
//		input: none
//		output: TRUE or FALSE (u8)
///////////////////////////////////////////////////////////////////////////////
static u8 bClear_feature(struct FTC_udc *dev,const struct usb_ctrlrequest *ctrl)
{

   u8 u8ep_n;
   u8 u8fifo_n;
   u8 bdir;

   DBG_FUNCC("+bClear_feature()\n");

   switch (ctrl->wValue)		// FeatureSelector
   {
      case 0:		// ENDPOINT_HALE
		 // Clear "Endpoint_Halt", Turn off the "STALL" bit in Endpoint Control Function Register
		 if(ctrl->wIndex == 0x00)
		   u8ep_n=0;  //Sp0 ed clear feature
		 else
		 {
		    u8ep_n = ctrl->wIndex & 0x7F;		// which ep will be clear
			bdir = ctrl->wIndex >> 7;			// the direction of this ep
			if (u8ep_n > MAX_EP_NUM)			// over the Max. ep count ?
			   return FALSE;
			else
			{
		 	   u8fifo_n = Get_FIFO_Num(&dev->ep[u8ep_n]);		// get the relatived FIFO number
			 
			   if (u8fifo_n<MAX_FIFO_NUM)
			       if ((mUsbFIFOConfigRd(u8fifo_n) & FIFOEnBit) == 0) // Check the FIFO had been enable ?
			           return FALSE;

			}


		 }
			FTC_clear_halt(&dev->ep[u8ep_n]);
 

         break;
            		
 	  case 1 :   		// Device Remote Wakeup
	 	 // Clear "Device_Remote_Wakeup", Turn off the"RMWKUP" bit in Main Control Register
		 mUsbRmWkupClr();
         break;

	  case 2 :   		// Test Mode
	 	 return FALSE;


          		
	  default :
		 return FALSE;
	}

	mUsbEP0DoneSet();	

	
	return TRUE; 
}



///////////////////////////////////////////////////////////////////////////////
//		bSet_feature(struct FTC_udc *dev)
//		Description:
//			1. Send 2 bytes status to host.
//		input: none
//		output: TRUE or FALSE (u8)
///////////////////////////////////////////////////////////////////////////////
static u8 bSet_feature(struct FTC_udc *dev,const struct usb_ctrlrequest *ctrl)
{


	u8 i;
	u8 u8ep_n;
	u8 u8fifo_n;
	u8 u8Tmp[52];
	u8 * pp;
	u8 bdir;

	DBG_FUNCC("+bSet_feature()\n");

	
	switch (ctrl->wValue)		// FeatureSelector
	{
		case 0:	// ENDPOINT_HALE
			// Set "Endpoint_Halt", Turn on the "STALL" bit in Endpoint Control Function Register
			if(ctrl->wIndex == 0x00)
			   FTC_set_halt(dev->gadget.ep0, 1);  // Return EP0_Stall
			else
			{
 			 u8ep_n = ctrl->wIndex & 0x7F;		// which ep will be clear
 		     bdir = ctrl->wIndex >> 7;			// the direction of this ep
			 u8fifo_n = Get_FIFO_Num(&dev->ep[u8ep_n]);		// get the relatived FIFO number
			 if (u8fifo_n<MAX_FIFO_NUM)											// Check the FIFO had been enable ?
			    if ((mUsbFIFOConfigRd(u8fifo_n) & FIFOEnBit) == 0)
   			       return FALSE;
		
			
			if (bdir == 1)						// IN direction ?
				mUsbEPinStallSet(u8ep_n);		// Clear Stall Bit
			else
				mUsbEPoutStallSet(u8ep_n);		// Set Stall Bit

			}
			break;
 		case 1 :   		// Device Remote Wakeup
 			// Set "Device_Remote_Wakeup", Turn on the"RMWKUP" bit in Mode Register
			mUsbRmWkupSet();

            break;



		case 2 :   		// Test Mode
			switch ((ctrl->wIndex >> 8))	// TestSelector
			{
				case 0x1:	// Test_J
					mUsbTsMdWr(TEST_J);
					break;
				
				case 0x2:	// Test_K
					mUsbTsMdWr(TEST_K);
					break;
						
				case 0x3:	// TEST_SE0_NAK
					mUsbTsMdWr(TEST_SE0_NAK);
					break;
						
				case 0x4:	// Test_Packet
					mUsbTsMdWr(TEST_PKY);
					mUsbEP0DoneSet();			// special case: follow the test sequence
					//////////////////////////////////////////////
					// Jay ask to modify, 91-6-5 (Begin)		//
					//////////////////////////////////////////////
					pp = u8Tmp;
					for (i=0; i<9; i++)			// JKJKJKJK x 9
					{
						(*pp) = (0x00);
						pp ++;
					}

					(*pp) = (0xAA);
					pp ++;
					(*pp) = (0x00);
					pp ++;		
					
					for (i=0; i<8; i++)			// 8*AA
					{
						(*pp) = (0xAA);
						pp ++;
					}
					
					for (i=0; i<8; i++)			// 8*EE
					{
						(*pp) = (0xEE);
						pp ++;
					}
					(*pp) = (0xFE);
					pp ++;	
					
					for (i=0; i<11; i++)		// 11*FF
					{
						(*pp) = (0xFF);
						pp ++;
					}
					
					(*pp) = (0x7F);
					pp ++;
					(*pp) = (0xBF);
					pp ++;
					(*pp) = (0xDF);
					pp ++;
					(*pp) = (0xEF);
					pp ++;
					(*pp) = (0xF7);
					pp ++;
					(*pp) = (0xFB);
					pp ++;
					(*pp) = (0xFD);
					pp ++;
					(*pp) = (0xFC);
					pp ++;
					(*pp) = (0x7E);
					pp ++;
					(*pp) = (0xBF);
					pp ++;
					(*pp) = (0xDF);
					pp ++;
					(*pp) = (0xFB);
					pp ++;
					(*pp) = (0xFD);
					pp ++;
					(*pp) = (0xFB);
					pp ++;
					(*pp) = (0xFD);
					pp ++;
					(*pp) = (0x7E);
		            CX_dma_Directly(u8Tmp,52,1);
					//////////////////////////////////////////////
					// Jay ask to modify, 91-6-5 (End)			//
					//////////////////////////////////////////////

					// Turn on "r_test_packet_done" bit(flag) (Bit 5)
					mUsbTsPkDoneSet();
					break;
				
				case 0x5:	// Test_Force_Enable
					//FUSBPort[0x08] = 0x20;	//Start Test_Force_Enable
					break;
			
				default:
					return FALSE;		
			}
	         	break;

 		case 3 :   		//For OTG => b_hnp_enable
			
	          dev->gadget.b_hnp_enable = 1;  	
  		//<1>.Set b_Bus_Request
                  mUsb_OTGC_Control_B_BUS_REQ_Set();
                  
  	        //<2>.Set the HNP enable
	          mUsb_OTGC_Control_B_HNP_EN_Set();

            break;			

 		case 4 :   		//For OTG => b_hnp_enable
			
	          dev->gadget.a_hnp_support = 1;  			  
 			  
            break;		
 		case 5 :   		//For OTG => b_hnp_enable
		
	          dev->gadget.a_alt_hnp_support = 1;
  			 printk(">>> Please Connect to an alternate port on the A-device for HNP...\n");

 			  
            break;		

		default :
			return FALSE;
	}
	
   mUsbEP0DoneSet();   
	
	return TRUE;

}


///////////////////////////////////////////////////////////////////////////////
//		bSynch_frame(struct FTC_udc *dev)
//		Description:
//			1. If the EP is a Iso EP, then return the 2 bytes Frame number.
//				 else stall this command
//		input: none
//		output: TRUE or FALSE
///////////////////////////////////////////////////////////////////////////////
static u8 bSynch_frame(struct FTC_udc *dev,const struct usb_ctrlrequest *ctrl)
{


	DBG_FUNCC("+bSynch_frame()  ==> add by Andrew\n");

	if((ctrl->wIndex==0)||(ctrl->wIndex>4))
		return FALSE;
	
	// Does the Endpoint support Isochronous transfer type? 
	   mUsbEP0DoneSet();   
		return TRUE;	
}


///////////////////////////////////////////////////////////////////////////////
//		bSet_address(struct FTC_udc *dev)
//		Description:
//			1. Set addr to FUSB200 register.
//		input: none
//		output: TRUE or FALSE (u8)
///////////////////////////////////////////////////////////////////////////////
static u8 bSet_address(struct FTC_udc *dev,const struct usb_ctrlrequest *ctrl)
{
	DBG_FUNCC("+bSet_address() = %d\n", ctrl->wValue);

	if (ctrl->wValue >= 0x0100)
		return FALSE;
	else
	{
		mUsbDevAddrSet(ctrl->wValue);
		mUsbEP0DoneSet();
		return TRUE;
	}
}


///////////////////////////////////////////////////////////////////////////////
//		vUsb_ep0setup(struct FTC_udc *dev)
//		Description:
//			1. Read the speed
//			2. Read 8-byte setup packet.
//          3. Process the standard command:
//             <1>.bSet_address
//
//
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
static void vUsb_ep0setup(struct FTC_udc *dev)
{
	u8  u8UsbCmd[8];
	struct usb_ctrlrequest	ctrl;
	int	tmp;
	u32 u32UsbCmd[2];


	DBG_FUNCC("+vUsb_ep0setup()\n");

	//<1>.Read the speed
	if(dev->gadget.speed == USB_SPEED_UNKNOWN)
	{

		// first ep0 command after usb reset, means we can check usb speed right now.
		if (mUsbHighSpeedST())					// First we should judge HS or FS
		{
			INFO(dev,"L%x, high speed mode\n", dev->u8LineCount ++);
	        dev->gadget.speed = USB_SPEED_HIGH;
	        vUsbFIFO_EPxCfg_HS();//Set the FIFO information
		}
		else   
		{
			INFO(dev,"L%x, full speed mode\n", dev->u8LineCount ++);
	        dev->gadget.speed = USB_SPEED_FULL;
	        vUsbFIFO_EPxCfg_FS();//Set the FIFO information
		}
	    dev->ep0state = EP0_IDLE;
	}

	//<2>.Dequeue ALL requests
	nuke(&dev->ep[0], 0);
	dev->ep[0].stopped = 0;

	//<3>.Read 8-byte setup packet from FIFO
	
	// Read 8-byte setup packet from FIFO
	mUsbDMA2FIFOSel(FOTG200_DMA2CxFIFO);
	u32UsbCmd[0] = mUsbEP0CmdDataRdDWord();
	u32UsbCmd[1] = mUsbEP0CmdDataRdDWord();
	mUsbDMA2FIFOSel(FOTG200_DMA2FIFO_Non);
	memcpy(u8UsbCmd,u32UsbCmd,8);

   	DBG_CTRLL("L%x, EP0Cmd:%02x %02x %02x %02x %02x %02x %02x %02x\n", dev->u8LineCount ++,
		      u8UsbCmd[0],u8UsbCmd[1],u8UsbCmd[2],u8UsbCmd[3],u8UsbCmd[4],u8UsbCmd[5],u8UsbCmd[6],u8UsbCmd[7]);

	/* read SETUP packet and enter DATA stage */
	ctrl.bRequestType = u8UsbCmd[0];  
	ctrl.bRequest = u8UsbCmd[1];
	ctrl.wValue  =(u8UsbCmd[3]  << 8) | u8UsbCmd[2];
	ctrl.wIndex  = (u8UsbCmd[5]  << 8) | u8UsbCmd[4]; 
	ctrl.wLength = (u8UsbCmd[7]  << 8) | u8UsbCmd[6];



	if (likely(ctrl.bRequestType & USB_DIR_IN)) {
		dev->ep[0].is_in = 1;
		dev->ep0state = EP0_IN;
	} 
	else {
		dev->ep[0].is_in = 0;
		dev->ep0state = EP0_OUT;
	}
   
  //Parsing the Standard Command 
	switch (ctrl.bRequest) // by Standard Request codes
   	{

		case USB_REQ_CLEAR_FEATURE:		// clear feature
			if (bClear_feature(dev, &(ctrl))==FALSE)
   			   goto stall;

   			break;

   		case USB_REQ_SET_ADDRESS:		// set address
   		     if (dev->ep0state == EP0_STALL)
   			   goto stall;   		     
			if (bSet_address(dev,&(ctrl))==FALSE)		
   			   goto stall;

   			break;

		case USB_REQ_SET_FEATURE:		// clear feature
			if (bSet_feature(dev, &(ctrl))==FALSE)
   			   goto stall;

   			break;


		case USB_REQ_GET_STATUS:		// clear feature
			if (bGet_status(dev, &(ctrl))==FALSE)
   			   goto stall;

   			break;

		case USB_REQ_SYNCH_FRAME:		// clear feature
   		     if (dev->ep0state == EP0_STALL)
   			   goto stall;   		     
			if (bSynch_frame(dev, &(ctrl))==FALSE)
   			   goto stall;


   			break;




		default:/* pass to gadget driver */
   		     if (dev->ep0state == EP0_STALL)
   			   goto stall;   		     
			
 	         spin_unlock (&dev->lock);
	         tmp = dev->driver->setup(&dev->gadget, &(ctrl));
	         spin_lock (&dev->lock);	
	         DBG_CTRLL(">>>Exit Driver call back setup function...\n");

	         if (unlikely(tmp < 0)) 	
   			     goto stall;			
	
			break;
   	}


//Normal Exit
  return ;


//Stall the command
stall:
#ifdef USB_TRACE
		  VDBG(dev, "req %02x.%02x protocol STALL; err %d\n",
				ctrl.bRequestType, ctrl.bRequest, tmp);
#endif
		  INFO(dev,"Set STALL in vUsb_ep0setup\n");
	      FTC_set_halt(dev->gadget.ep0, 1);  // Return EP0_Stall
		dev->ep[0].stopped = 1;
		dev->ep0state = EP0_STALL;	      



}


///////////////////////////////////////////////////////////////////////////////
//		vUsb_ep0end(struct FTC_udc *dev)
//		Description:
//			1. End this transfer.
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
static void vUsb_ep0end(struct FTC_udc *dev)
{ 
	DBG_FUNCC("+vUsb_ep0end()\n");

	dev->eUsbCxCommand = CMD_VOID;


}

///////////////////////////////////////////////////////////////////////////////
//		vUsb_ep0fail(struct FTC_udc *dev)
//		Description:
//			1. Stall this transfer.
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
static void vUsb_ep0fail(struct FTC_udc *dev)
{ 
	DBG_FUNCC("+vUsb_ep0fail()\n");

	INFO(dev,"L%x, EP0 fail\n", dev->u8LineCount ++);

	FTC_set_halt(dev->gadget.ep0, 1);  // Return EP0_Stall
}


///////////////////////////////////////////////////////////////////////////////
//		vUsbHandler(struct FTC_udc	*dev)
//		Description:
//			1. Service all Usb events
//			2. ReEnable Usb interrupt.
//		input: none
//		output: none
///////////////////////////////////////////////////////////////////////////////
static void vUsbHandler(struct FTC_udc	*dev)//FOTG200.ok
{
	u32 usb_interrupt_level2;
	u32 usb_interrupt_Mask;
	u32 usb_interrupt_Origan;
	 
	DBG_FUNCC("+vUsbHandler()\n");

	DBG_CTRLL("usb_interrupt_level1:0x%x\n",dev->usb_interrupt_level1);

 //----- Group Byte 2 ---------

	if (dev->usb_interrupt_level1 & BIT2)				
	{
		usb_interrupt_Origan = mUsbIntSrc2Rd();
		usb_interrupt_Mask = mUsbIntSrc2MaskRd();
		usb_interrupt_level2 = usb_interrupt_Origan & ~usb_interrupt_Mask;
	DBG_CTRLL("usb_interrupt_Origan:0x%x\n",usb_interrupt_Origan);		
	DBG_CTRLL("usb_interrupt_Mask:0x%x\n",usb_interrupt_Mask);
	DBG_CTRLL("usb_interrupt_level2:0x%x\n",usb_interrupt_level2);
				
		if (usb_interrupt_level2 & BIT0)
			vUsb_rst(dev);
		if (usb_interrupt_level2 & BIT1)
			vUsb_suspend(dev);
		if (usb_interrupt_level2 & BIT2)
			vUsb_resm(dev);
		if (usb_interrupt_level2 & BIT3)
		{
			mUsbIntIsoSeqErrClr();
			printk("??? ISO sequence error...\n");
		}
		if (usb_interrupt_level2 & BIT4)
		{
			mUsbIntIsoSeqAbortClr();
			printk("??? ISO sequence error...\n");
		}
		if (usb_interrupt_level2 & BIT5)
			mUsbIntTX0ByteClr();
			
		if (usb_interrupt_level2 & BIT6)
			mUsbIntRX0ByteClr();		

		if (usb_interrupt_level2 & BIT7)
		{	mUsbIntDmaFinishClr();
     		dma_advance(dev, &(dev->ep[dev->EPUseDMA]));
		}	
		if (usb_interrupt_level2 & BIT8)
		{
			mUsbIntDmaErrClr();
			printk("??? DMA error Interrupt \n");

		}
		
	}

 //----- Group Byte 0 ---------
	if (dev->usb_interrupt_level1 & BIT0)				
	{
		usb_interrupt_Origan = mUsbIntSrc0Rd();
		usb_interrupt_Mask = mUsbIntSrc0MaskRd();
		usb_interrupt_level2 = usb_interrupt_Origan & ~usb_interrupt_Mask;

		DBG_CTRLL("IntSCR0:0x%x\n",usb_interrupt_level2);
        dev->ep[0].irqs++;
		//	Stop APB DMA if DMA is still running 
		//	record buffer counter, and clear buffer. Later  
		//	will re-input data use DMA.	
		if (usb_interrupt_level2 & BIT0){
			DBG_CTRLL("USB ep0 Setup\n");			
			vUsb_ep0setup(dev);
		}	
		else if (usb_interrupt_level2 & BIT3){
			DBG_CTRLL("USB ep0 end\n");
			vUsb_ep0end(dev);
		}	
		if (usb_interrupt_level2 & BIT1){
			DBG_CTRLL("USB ep0 TX\n");

		}	
		if (usb_interrupt_level2 & BIT2){
			DBG_CTRLL("USB ep0 RX\n");

		}	
		if (usb_interrupt_level2 & BIT4){
			WARN(dev,"USB ep0 fail\n");	
			vUsb_ep0fail(dev);
		}
		if (usb_interrupt_level2 & BIT5){
			mUsbIntEP0AbortClr();	
			printk("??? Command Abort Interrupt ...\n");

		}
	}

 //----- Group Byte 1 ---------
	if (dev->usb_interrupt_level1 & BIT1)			
	{
       usb_interrupt_Origan = mUsbIntSrc1Rd();
	   usb_interrupt_Mask = mUsbIntSrc1MaskRd();
	   usb_interrupt_level2 = usb_interrupt_Origan & (~usb_interrupt_Mask);

	   DBG_CTRLL("(IntSCR1:0x%x)(Mask1:0x%x)(usb_interrupt_level2=0x%x)\n"
	            ,usb_interrupt_Origan,mUsbIntSrc1MaskRd(),usb_interrupt_level2);
  
	   
		//FIFO0 => Ep-1 In  Bulk
		//FIFO1 => Ep-2	Out	Bulk
		//FIFO2 => Ep-3	In	Interrupt
		//FIFO3 => Ep-4	Out	Interrupt
		// use FIFO1 for ep2( bulk out)
	   	if (usb_interrupt_level2 & BIT3)			// short packet
			Ep_ISR(dev,2);	 
		else if (usb_interrupt_level2 & BIT2)		// full packet
			Ep_ISR(dev,2);
		
		// use FIFO0 for ep1( bulk in)
		if (usb_interrupt_level2 & BIT16)
			Ep_ISR(dev,1);
	   
		// use FIFO3 for ep4( Interrupt out)
	   	if (usb_interrupt_level2 & BIT7)			// short packet
			Ep_ISR(dev,4);	 
		else if (usb_interrupt_level2 & BIT6)		// full packet
			Ep_ISR(dev,4);
		
		// use FIFO2 for ep3( Interrupt in)
		if (usb_interrupt_level2 & BIT18)
			Ep_ISR(dev,3);	   
	   

	}


}
#ifdef OTG200_Peripheral_Only

static irqreturn_t FTC_irq(int irq, void *_dev, struct pt_regs *r)//FOTG200.ok
{
	struct FTC_udc	*dev = _dev;
	u32	   handled = 0;
	DBG_FUNCC("+FTC_irq()\n");
	spin_lock(&dev->lock);

	dev->usb_interrupt_level1_Save = mUsbIntGroupRegRd();
	dev->usb_interrupt_level1_Mask = mUsbIntGroupMaskRd();
	
	DBG_CTRLL("dev->usb_interrupt_level1_Save = 0x%x\n",(dev->usb_interrupt_level1_Save));
	DBG_CTRLL("dev->usb_interrupt_level1_Mask = 0x%x\n",(dev->usb_interrupt_level1_Mask));

	
	dev->usb_interrupt_level1 = dev->usb_interrupt_level1_Save & ~dev->usb_interrupt_level1_Mask;
	
	if (dev->usb_interrupt_level1 != 0)
	{		
		dev->irqs++;
		handled = 1;

	    vUsbHandler(dev);
	   // Clear usb interrupt flags
	   dev->usb_interrupt_level1 = 0;
	}

	spin_unlock(&dev->lock);

	return IRQ_RETVAL(handled);
}

#else

static int udc_irq_for_OTG(void)
{
	struct FTC_udc	*dev = the_controller;
	
	DBG_FUNCC("+udc_irq_for_OTG()\n");
	spin_lock(&dev->lock);

	dev->usb_interrupt_level1_Save = mUsbIntGroupRegRd();
	dev->usb_interrupt_level1_Mask = mUsbIntGroupMaskRd();
	
	DBG_CTRLL("dev->usb_interrupt_level1_Save = 0x%x\n",(dev->usb_interrupt_level1_Save));
	DBG_CTRLL("dev->usb_interrupt_level1_Mask = 0x%x\n",(dev->usb_interrupt_level1_Mask));

	
	dev->usb_interrupt_level1 = dev->usb_interrupt_level1_Save & ~dev->usb_interrupt_level1_Mask;
	
	if (dev->usb_interrupt_level1 != 0)
	{		
		dev->irqs++;


	    vUsbHandler(dev);
	   // Clear usb interrupt flags
	   dev->usb_interrupt_level1 = 0;
	}

	spin_unlock(&dev->lock);

	return 0;
}


#endif


/*-------------------------------------------------------------------------*/

/* tear down the binding between this driver and the pci device */
static void FTC_usb_remove(void)//FOTG200.ok
{
	DBG_FUNCC("+FTC_usb_remove()\n");


	/* start with the driver above us */
	if (the_controller->driver) {
		/* should have been done already by driver model core */
		WARN(dev,"remove driver '%s' is still registered\n",
				the_controller->driver->driver.name);
		usb_gadget_unregister_driver(the_controller->driver);
	}

	udc_reset(the_controller);

#ifdef OTG200_Peripheral_Only  
     if (the_controller->got_irq)  //Andrew update
         free_irq(IRQ_FOTG200, the_controller);//Andrew update
#endif



	// free EP0 req, buffer
	//Andrew update;;FTC_free_buffer (&the_controller->ep[0].ep, 
	//Andrew update;;                 the_controller->EP0req->buf, 
	//Andrew update;;				 the_controller->EP0req->dma, the_controller->EP0req->length);
	//Andrew update;;FTC_free_request (&the_controller->ep[0].ep, the_controller->EP0req);

   kfree(the_controller); //Andrew update
	the_controller = 0;

	INFO(dev,"USB device unbind\n");
}

/* wrap this driver around the specified pci device, but
 * don't respond over USB until a gadget driver binds to us.
 */
static int FTC_usb_probe(void)//FOTG200.ok
{
	int			     retval=0;
    u32              wFound=0;
	struct otg_transceiver	*xceiv = 0;    
    
	DBG_FUNCC("+FTC_usb_probe()\n");

  //<1>.Checking FOTG200
    if (wFOTGPeri_Port(0x00)==0x01000010)
        if (wFOTGPeri_Port(0x04)==0x00000001)
           if(wFOTGPeri_Port(0x08)==0x00000006) 
              wFound=1;

    DBG_FUNCC(">>> Checking FOTG200 ...(0x00=0x%x)\n",wFOTGPeri_Port(0x00));              
    
    if (wFound==1)
       	DBG_FUNCC(">>> Found FOTG200 ...\n");
    else
       {
      	DBG_FUNCC("??? Not Found FOTG200 ...(0x00=0x%x)\n",wFOTGPeri_Port(0x00));
       	return(-EBUSY);
       	}

  //Temp Solution for FOTG200-Only-Peripheral => Disable Host & Control interrupt
#ifdef OTG200_Peripheral_Only  
    wFOTGPeri_Port(0xC4)|=0x06; //Disable Host & Control interrupt
#endif
    
  //<2>.Init "the_controller" structure
  
	/* if you want to support more than one controller in a system,
	 * usb_gadget_driver_{register,unregister}() must change.
	 */
	if (the_controller) {
		WARN(dev,"ignoring : more than one device\n");
		return -EBUSY;
	}

	/* alloc, and start init */
	the_controller = kmalloc (sizeof *the_controller, GFP_KERNEL);
	if (the_controller == NULL){
		pr_debug("enomem USB device\n");
		retval = -ENOMEM;
		goto done;
	}

	memset(the_controller, 0, sizeof *the_controller);
	spin_lock_init(&the_controller->lock);
	the_controller->gadget.ops = &FTC_ops;

	/* the "gadget" abstracts/virtualizes the controller */
//	the_controller->gadget.dev.bus_id = "gadget";
	the_controller->gadget.name = driver_name;
//	the_controller->gadget.udc_isr = udc_irq_for_OTG;
	the_controller->enabled = 1;


	the_controller->EPUseDMA = DMA_CHANEL_FREE;   //reset

 //<3>. udc Reset/udc reinit
	/* init to known state, then setup irqs */
	udc_reset(the_controller);
	udc_reinit(the_controller);


 //<4>.Init interrupt
#ifdef OTG200_Peripheral_Only   
    wFOTGPeri_Port(0xC4)|=0x08;//Fource FOTG200-Interrupt to High-Active
    
    cpe_int_set_irq(IRQ_FOTG200, LEVEL, H_ACTIVE);
    
	if (request_irq(IRQ_FOTG200, FTC_irq, SA_INTERRUPT /*|SA_SAMPLE_RANDOM*/,
		            driver_name, the_controller) != 0) 
	{
		DBG(dev, "request interrupt failed\n");
		retval = -EBUSY;
		goto done;
	}
#endif

	the_controller->got_irq = 1;

#ifndef OTG200_Peripheral_Only 
// jingdar mark for test  20091016     xceiv = FOTG2XX_get_otg_transceiver();//For OTG
    the_controller->transceiver = xceiv;//For OTG
    the_controller->gadget.is_otg = 1;
#endif    
    
    
	/* done */
	return 0;

done:

	if (the_controller)
		FTC_usb_remove();
	return retval;

}


/*-------------------------------------------------------------------------*/
static int __init init (void)//FOTG200.ok
{
	INFO(dev,"Init USB device Lower driver\n");

	DBG_CTRLL("FOTG200_BASE_ADDRESS = 0x%x\n", FOTG200_BASE_ADDRESS);
	return FTC_usb_probe();
}
module_init (init);
static void __exit cleanup (void) //FOTG200.ok
{
	INFO(dev,"Remove USB device Lower driver\n");

	return FTC_usb_remove();
}
module_exit (cleanup);


