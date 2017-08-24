/***********************************************************************************
 CED1401 usb driver. This basic loading is based on the usb-skeleton.c code that is:
 Copyright (C) 2001-2004 Greg Kroah-Hartman (greg@kroah.com)
 Copyright (C) 2012 Alois Schloegl <alois.schloegl@ist.ac.at>
 There is not a great deal of the skeleton left.

 All the remainder dealing specifically with the CED1401 is based on drivers written
 by CED for other systems (mainly Windows) and is:
 Copyright (C) 2010 Cambridge Electronic Design Ltd
 Author Greg P Smith (greg@ced.co.uk)

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

Endpoints
*********
There are 4 endpoints plus the control endpoint in the standard interface
provided by most 1401s. The control endpoint is used for standard USB requests,
plus various CED-specific transactions such as start self test, debug and get
the 1401 status. The other endpoints are:

 1 Characters to the 1401
 2 Characters from the 1401
 3 Block data to the 1401
 4 Block data to the host.

inside the driver these are indexed as an array from 0 to 3, transactions
over the control endpoint are carried out using a separate mechanism. The
use of the endpoints is mostly straightforward, with the driver issuing
IO request packets (IRPs) as required to transfer data to and from the 1401.
The handling of endpoint 2 is different because it is used for characters
from the 1401, which can appear spontaneously and without any other driver
activity - for example to repeatedly request DMA transfers in Spike2. The
desired effect is achieved by using an interrupt endpoint which can be
polled to see if it has data available, and writing the driver so that it
always maintains a pending read IRP from that endpoint which will read the
character data and terminate as soon as the 1401 makes data available. This
works very well, some care is taken with when you kick off this character
read IRP to avoid it being active when it is not wanted but generally it
is running all the time.

In the 2270, there are only three endpoints plus the control endpoint. In
addition to the transactions mentioned above, the control endpoint is used
to transfer character data to the 1401. The other endpoints are used as:

 1 Characters from the 1401
 2 Block data to the 1401
 3 Block data to the host.

The type of interface available is specified by the interface subclass field
in the interface descriptor provided by the 1401. See the USB_INT_ constants
for the values that this field can hold.

****************************************************************************
Linux implementation

Although Linux Device Drivers (3rd Edition) was a major source of information,
it is very out of date. A lot of information was gleaned from the latest
usb_skeleton.c code (you need to download the kernel sources to get this).

To match the Windows version, everything is done using ioctl calls. All the
device state is held in the DEVICE_EXTENSION (named to match Windows use).
Block transfers are done by using get_user_pages() to pin down a list of
pages that we hold a pointer to in the device driver. We also allocate a
coherent transfer buffer of size STAGED_SZ (this must be a multiple of the
bulk endpoint size so that the 1401 does not realise that we break large
transfers down into smaller pieces). We use kmap_atomic() to get a kernel
va for each page, as it is required, for copying; see CopyUserSpace().

All character and data transfers are done using asynchronous IO. All Urbs are
tracked by anchoring them. Status and debug ioctls are implemented with the
synchronous non-Urb based transfers.
*/

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/usb.h>
#include <linux/mutex.h>
#include <linux/mm.h>
#include <linux/highmem.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kref.h>
#include <linux/uaccess.h>

#include "usb1401.h"

/*                                           */
#define USB_CED_VENDOR_ID	0x0525
#define USB_CED_PRODUCT_ID	0xa0f0

/*                                             */
static const struct usb_device_id ced_table[] = {
	{USB_DEVICE(USB_CED_VENDOR_ID, USB_CED_PRODUCT_ID)},
	{}			/*                   */
};

MODULE_DEVICE_TABLE(usb, ced_table);

/*                                                            */
#define USB_CED_MINOR_BASE	192

/*                                                                     */
#define MAX_TRANSFER		(PAGE_SIZE - 512)
/*                                                         
                                                              
                                                            */
#define WRITES_IN_FLIGHT	8
/*                    */

static struct usb_driver ced_driver;

static void ced_delete(struct kref *kref)
{
	DEVICE_EXTENSION *pdx = to_DEVICE_EXTENSION(kref);

	//                                                                                    
	//                                                               
	usb_free_coherent(pdx->udev, OUTBUF_SZ, pdx->pCoherCharOut,
			  pdx->pUrbCharOut->transfer_dma);
	usb_free_urb(pdx->pUrbCharOut);

	//                           
	usb_free_coherent(pdx->udev, INBUF_SZ, pdx->pCoherCharIn,
			  pdx->pUrbCharIn->transfer_dma);
	usb_free_urb(pdx->pUrbCharIn);

	//                                    
	usb_free_coherent(pdx->udev, STAGED_SZ, pdx->pCoherStagedIO,
			  pdx->pStagedUrb->transfer_dma);
	usb_free_urb(pdx->pStagedUrb);

	usb_put_dev(pdx->udev);
	kfree(pdx);
}

//                                                           
static int ced_open(struct inode *inode, struct file *file)
{
	DEVICE_EXTENSION *pdx;
	int retval = 0;
	int subminor = iminor(inode);
	struct usb_interface *interface =
	    usb_find_interface(&ced_driver, subminor);
	if (!interface) {
		pr_err("%s - error, can't find device for minor %d", __func__,
		       subminor);
		retval = -ENODEV;
		goto exit;
	}

	pdx = usb_get_intfdata(interface);
	if (!pdx) {
		retval = -ENODEV;
		goto exit;
	}

	dev_dbg(&interface->dev, "%s got pdx", __func__);

	/*                                          */
	kref_get(&pdx->kref);

	/*                                                   
                  */
	mutex_lock(&pdx->io_mutex);

	if (!pdx->open_count++) {
		retval = usb_autopm_get_interface(interface);
		if (retval) {
			pdx->open_count--;
			mutex_unlock(&pdx->io_mutex);
			kref_put(&pdx->kref, ced_delete);
			goto exit;
		}
	} else {		//                                               
		dev_err(&interface->dev, "%s fail: already open", __func__);
		retval = -EBUSY;
		pdx->open_count--;
		mutex_unlock(&pdx->io_mutex);
		kref_put(&pdx->kref, ced_delete);
		goto exit;
	}
	/*                                             */

	/*                                                 */
	file->private_data = pdx;
	mutex_unlock(&pdx->io_mutex);

exit:
	return retval;
}

static int ced_release(struct inode *inode, struct file *file)
{
	DEVICE_EXTENSION *pdx = file->private_data;
	if (pdx == NULL)
		return -ENODEV;

	dev_dbg(&pdx->interface->dev, "%s called", __func__);
	mutex_lock(&pdx->io_mutex);
	if (!--pdx->open_count && pdx->interface)	//                  
		usb_autopm_put_interface(pdx->interface);
	mutex_unlock(&pdx->io_mutex);

	kref_put(&pdx->kref, ced_delete);	//                                  
	return 0;
}

static int ced_flush(struct file *file, fl_owner_t id)
{
	int res;
	DEVICE_EXTENSION *pdx = file->private_data;
	if (pdx == NULL)
		return -ENODEV;

	dev_dbg(&pdx->interface->dev, "%s char in pend=%d", __func__,
		pdx->bReadCharsPending);

	/*                     */
	mutex_lock(&pdx->io_mutex);
	dev_dbg(&pdx->interface->dev, "%s got io_mutex", __func__);
	ced_draw_down(pdx);

	/*                                                       */
	spin_lock_irq(&pdx->err_lock);
	res = pdx->errors ? (pdx->errors == -EPIPE ? -EPIPE : -EIO) : 0;
	pdx->errors = 0;
	spin_unlock_irq(&pdx->err_lock);

	mutex_unlock(&pdx->io_mutex);
	dev_dbg(&pdx->interface->dev, "%s exit reached", __func__);

	return res;
}

/*                                                                          
                      
                                                                             
                                                                             
                                                   
                                                        
*/
static bool CanAcceptIoRequests(DEVICE_EXTENSION * pdx)
{
	return pdx && pdx->interface;	//                          
}

/*                                                                           
                                                                         
                                
                                                                           */
static void ced_writechar_callback(struct urb *pUrb)
{
	DEVICE_EXTENSION *pdx = pUrb->context;
	int nGot = pUrb->actual_length;	//                    

	if (pUrb->status) {	//                                       
		if (!
		    (pUrb->status == -ENOENT || pUrb->status == -ECONNRESET
		     || pUrb->status == -ESHUTDOWN)) {
			dev_err(&pdx->interface->dev,
				"%s - nonzero write bulk status received: %d",
				__func__, pUrb->status);
		}

		spin_lock(&pdx->err_lock);
		pdx->errors = pUrb->status;
		spin_unlock(&pdx->err_lock);
		nGot = 0;	//                         

		spin_lock(&pdx->charOutLock);	//                     
		pdx->dwOutBuffGet = 0;	//                        
		pdx->dwOutBuffPut = 0;
		pdx->dwNumOutput = 0;	//                     
		pdx->bPipeError[0] = 1;	//                        
		pdx->bSendCharsPending = false;	//                          
		spin_unlock(&pdx->charOutLock);	//                     
		dev_dbg(&pdx->interface->dev,
			"%s - char out done, 0 chars sent", __func__);
	} else {
		dev_dbg(&pdx->interface->dev,
			"%s - char out done, %d chars sent", __func__, nGot);
		spin_lock(&pdx->charOutLock);	//                     
		pdx->dwNumOutput -= nGot;	//                                
		pdx->dwOutBuffGet += nGot;	//                     
		if (pdx->dwOutBuffGet >= OUTBUF_SZ)	//                                                       
			pdx->dwOutBuffGet = 0;

		if (pdx->dwNumOutput > 0)	//                      
		{
			int nPipe = 0;	//                       
			int iReturn;
			char *pDat = &pdx->outputBuffer[pdx->dwOutBuffGet];
			unsigned int dwCount = pdx->dwNumOutput;	//                
			if ((pdx->dwOutBuffGet + dwCount) > OUTBUF_SZ)	//                          
				dwCount = OUTBUF_SZ - pdx->dwOutBuffGet;
			spin_unlock(&pdx->charOutLock);	//                                    
			memcpy(pdx->pCoherCharOut, pDat, dwCount);	//                               
			usb_fill_bulk_urb(pdx->pUrbCharOut, pdx->udev,
					  usb_sndbulkpipe(pdx->udev,
							  pdx->epAddr[0]),
					  pdx->pCoherCharOut, dwCount,
					  ced_writechar_callback, pdx);
			pdx->pUrbCharOut->transfer_flags |=
			    URB_NO_TRANSFER_DMA_MAP;
			usb_anchor_urb(pdx->pUrbCharOut, &pdx->submitted);	//                           
			iReturn = usb_submit_urb(pdx->pUrbCharOut, GFP_ATOMIC);
			dev_dbg(&pdx->interface->dev, "%s n=%d>%s<", __func__,
				dwCount, pDat);
			spin_lock(&pdx->charOutLock);	//                     
			if (iReturn) {
				pdx->bPipeError[nPipe] = 1;	//                                  
				pdx->bSendCharsPending = false;	//                          
				usb_unanchor_urb(pdx->pUrbCharOut);
				dev_err(&pdx->interface->dev,
					"%s usb_submit_urb() returned %d",
					__func__, iReturn);
			}
		} else
			pdx->bSendCharsPending = false;	//                          
		spin_unlock(&pdx->charOutLock);	//                     
	}
}

/*                                                                           
            
                                                                          
                                         
                                                                           */
int SendChars(DEVICE_EXTENSION * pdx)
{
	int iReturn = U14ERR_NOERROR;

	spin_lock_irq(&pdx->charOutLock);	//                  

	if ((!pdx->bSendCharsPending) &&	//                      
	    (pdx->dwNumOutput > 0) &&	//                          
	    (CanAcceptIoRequests(pdx)))	//                            
	{
		unsigned int dwCount = pdx->dwNumOutput;	//                                  
		pdx->bSendCharsPending = true;	//                                   

		dev_dbg(&pdx->interface->dev,
			"Send %d chars to 1401, EP0 flag %d\n", dwCount,
			pdx->nPipes == 3);
		//                                                                                
		if (pdx->nPipes == 3) {
			//                                                                              
			//                                                                              
			unsigned int count = dwCount;	//                   
			unsigned int index = 0;	//                               

			spin_unlock_irq(&pdx->charOutLock);	//                              

			while ((count > 0) && (iReturn == U14ERR_NOERROR)) {
				//                                                                               
				int n = count > 64 ? 64 : count;	//                               
				int nSent = usb_control_msg(pdx->udev,
							    usb_sndctrlpipe(pdx->udev, 0),	//                
							    DB_CHARS,	//         
							    (H_TO_D | VENDOR | DEVREQ),	//                                            
							    0, 0,	//                           
							    &pdx->outputBuffer[index],	//                   
							    n,	//                 
							    1000);	//                   
				if (nSent <= 0) {
					iReturn = nSent ? nSent : -ETIMEDOUT;	//                             
					dev_err(&pdx->interface->dev,
						"Send %d chars by EP0 failed: %d",
						n, iReturn);
				} else {
					dev_dbg(&pdx->interface->dev,
						"Sent %d chars by EP0", n);
					count -= nSent;
					index += nSent;
				}
			}

			spin_lock_irq(&pdx->charOutLock);	//                                              
			pdx->dwOutBuffGet = 0;	//                           
			pdx->dwOutBuffPut = 0;
			pdx->dwNumOutput = 0;	//                           
			pdx->bSendCharsPending = false;	//                          
		} else {	//                                                        
			int nPipe = 0;	//                       
			char *pDat = &pdx->outputBuffer[pdx->dwOutBuffGet];

			if ((pdx->dwOutBuffGet + dwCount) > OUTBUF_SZ)	//                          
				dwCount = OUTBUF_SZ - pdx->dwOutBuffGet;
			spin_unlock_irq(&pdx->charOutLock);	//                                    
			memcpy(pdx->pCoherCharOut, pDat, dwCount);	//                               
			usb_fill_bulk_urb(pdx->pUrbCharOut, pdx->udev,
					  usb_sndbulkpipe(pdx->udev,
							  pdx->epAddr[0]),
					  pdx->pCoherCharOut, dwCount,
					  ced_writechar_callback, pdx);
			pdx->pUrbCharOut->transfer_flags |=
			    URB_NO_TRANSFER_DMA_MAP;
			usb_anchor_urb(pdx->pUrbCharOut, &pdx->submitted);
			iReturn = usb_submit_urb(pdx->pUrbCharOut, GFP_KERNEL);
			spin_lock_irq(&pdx->charOutLock);	//                     
			if (iReturn) {
				pdx->bPipeError[nPipe] = 1;	//                                  
				pdx->bSendCharsPending = false;	//                          
				usb_unanchor_urb(pdx->pUrbCharOut);	//                                
			}
		}
	} else if (pdx->bSendCharsPending && (pdx->dwNumOutput > 0))
		dev_dbg(&pdx->interface->dev,
			"SendChars bSendCharsPending:true");

	dev_dbg(&pdx->interface->dev, "SendChars exit code: %d", iReturn);
	spin_unlock_irq(&pdx->charOutLock);	//                           
	return iReturn;
}

/*                                                                          
                
                                                                          
                                                                          
                                                                          
                                                               
                                                                 
  
                                                                          
                                                         
  
                                                                           
                                                         
                                                                          */
static void CopyUserSpace(DEVICE_EXTENSION * pdx, int n)
{
	unsigned int nArea = pdx->StagedId;
	if (nArea < MAX_TRANSAREAS) {
		TRANSAREA *pArea = &pdx->rTransDef[nArea];	//                
		unsigned int dwOffset =
		    pdx->StagedDone + pdx->StagedOffset + pArea->dwBaseOffset;
		char *pCoherBuf = pdx->pCoherStagedIO;	//                
		if (!pArea->bUsed) {
			dev_err(&pdx->interface->dev, "%s area %d unused",
				__func__, nArea);
			return;
		}

		while (n) {
			int nPage = dwOffset >> PAGE_SHIFT;	//                     
			if (nPage < pArea->nPages) {
				char *pvAddress =
				    (char *)kmap_atomic(pArea->pPages[nPage]);
				if (pvAddress) {
					unsigned int uiPageOff = dwOffset & (PAGE_SIZE - 1);	//                     
					size_t uiXfer = PAGE_SIZE - uiPageOff;	//                             
					if (uiXfer > n)	//                             
						uiXfer = n;	//             
					if (pdx->StagedRead)
						memcpy(pvAddress + uiPageOff,
						       pCoherBuf, uiXfer);
					else
						memcpy(pCoherBuf,
						       pvAddress + uiPageOff,
						       uiXfer);
					kunmap_atomic(pvAddress);
					dwOffset += uiXfer;
					pCoherBuf += uiXfer;
					n -= uiXfer;
				} else {
					dev_err(&pdx->interface->dev,
						"%s did not map page %d",
						__func__, nPage);
					return;
				}

			} else {
				dev_err(&pdx->interface->dev,
					"%s exceeded pages %d", __func__,
					nPage);
				return;
			}
		}
	} else
		dev_err(&pdx->interface->dev, "%s bad area %d", __func__,
			nArea);
}

//                                               
static int StageChunk(DEVICE_EXTENSION * pdx);
/*                                                                          
                     
  
                                                     
*/
static void staged_callback(struct urb *pUrb)
{
	DEVICE_EXTENSION *pdx = pUrb->context;
	unsigned int nGot = pUrb->actual_length;	//                    
	bool bCancel = false;
	bool bRestartCharInput;	//                

	spin_lock(&pdx->stagedLock);	//                                                         
	pdx->bStagedUrbPending = false;	//                                      

	if (pUrb->status) {	//                                       
		if (!
		    (pUrb->status == -ENOENT || pUrb->status == -ECONNRESET
		     || pUrb->status == -ESHUTDOWN)) {
			dev_err(&pdx->interface->dev,
				"%s - nonzero write bulk status received: %d",
				__func__, pUrb->status);
		} else
			dev_info(&pdx->interface->dev,
				 "%s - staged xfer cancelled", __func__);

		spin_lock(&pdx->err_lock);
		pdx->errors = pUrb->status;
		spin_unlock(&pdx->err_lock);
		nGot = 0;	//                         
		bCancel = true;
	} else {
		dev_dbg(&pdx->interface->dev, "%s %d chars xferred", __func__,
			nGot);
		if (pdx->StagedRead)	//                               
			CopyUserSpace(pdx, nGot);	//                         
		if (nGot == 0)
			dev_dbg(&pdx->interface->dev, "%s ZLP", __func__);
	}

	//                                                                              
	pdx->StagedDone += nGot;

	dev_dbg(&pdx->interface->dev, "%s, done %d bytes of %d", __func__,
		pdx->StagedDone, pdx->StagedLength);

	if ((pdx->StagedDone == pdx->StagedLength) ||	//                 
	    (bCancel))		//                          
	{
		TRANSAREA *pArea = &pdx->rTransDef[pdx->StagedId];	//                   
		dev_dbg(&pdx->interface->dev,
			"%s transfer done, bytes %d, cancel %d", __func__,
			pdx->StagedDone, bCancel);

		//                                                                                            
		//                                                                                              
		//                                                                                          
		//                                                                      
		if ((pArea->bCircular) && (pArea->bCircToHost) && (!bCancel) &&	//                                       
		    (pdx->StagedRead))	//                                  
		{
			if (pArea->aBlocks[1].dwSize > 0)	//                                          
			{
				if (pdx->StagedOffset ==
				    (pArea->aBlocks[1].dwOffset +
				     pArea->aBlocks[1].dwSize)) {
					pArea->aBlocks[1].dwSize +=
					    pdx->StagedLength;
					dev_dbg(&pdx->interface->dev,
						"RWM_Complete, circ block 1 now %d bytes at %d",
						pArea->aBlocks[1].dwSize,
						pArea->aBlocks[1].dwOffset);
				} else {
					//                                                                                            
					pArea->aBlocks[1].dwOffset =
					    pdx->StagedOffset;
					pArea->aBlocks[1].dwSize =
					    pdx->StagedLength;
					dev_err(&pdx->interface->dev,
						"%s ERROR, circ block 1 re-started %d bytes at %d",
						__func__,
						pArea->aBlocks[1].dwSize,
						pArea->aBlocks[1].dwOffset);
				}
			} else	//                                                 
			{
				if (pArea->aBlocks[0].dwSize > 0)	//                                
				{	//                                      
					if (pdx->StagedOffset ==
					    (pArea->aBlocks[0].dwOffset +
					     pArea->aBlocks[0].dwSize)) {
						pArea->aBlocks[0].dwSize += pdx->StagedLength;	//                          
						dev_dbg(&pdx->interface->dev,
							"RWM_Complete, circ block 0 now %d bytes at %d",
							pArea->aBlocks[0].
							dwSize,
							pArea->aBlocks[0].
							dwOffset);
					} else	//                                           
					{
						pArea->aBlocks[1].dwOffset =
						    pdx->StagedOffset;
						pArea->aBlocks[1].dwSize =
						    pdx->StagedLength;
						dev_dbg(&pdx->interface->dev,
							"RWM_Complete, circ block 1 started %d bytes at %d",
							pArea->aBlocks[1].
							dwSize,
							pArea->aBlocks[1].
							dwOffset);
					}
				} else	//                                         
				{
					pArea->aBlocks[0].dwOffset =
					    pdx->StagedOffset;
					pArea->aBlocks[0].dwSize =
					    pdx->StagedLength;
					dev_dbg(&pdx->interface->dev,
						"RWM_Complete, circ block 0 started %d bytes at %d",
						pArea->aBlocks[0].dwSize,
						pArea->aBlocks[0].dwOffset);
				}
			}
		}

		if (!bCancel)	//                                     
		{
			dev_dbg(&pdx->interface->dev,
				"RWM_Complete,  bCircular %d, bToHost %d, eStart %d, eSize %d",
				pArea->bCircular, pArea->bEventToHost,
				pArea->dwEventSt, pArea->dwEventSz);
			if ((pArea->dwEventSz) &&	//                         
			    (pdx->StagedRead == pArea->bEventToHost))	//                                   
			{
				int iWakeUp = 0;	//       
				//                                                                                 
				//                                                   
				if ((pArea->bCircular) &&	//                                  
				    (pArea->bCircToHost))	//                            
				{	//                                        
					unsigned int dwTotal =
					    pArea->aBlocks[0].dwSize +
					    pArea->aBlocks[1].dwSize;
					iWakeUp = (dwTotal >= pArea->dwEventSz);
				} else {
					unsigned int transEnd =
					    pdx->StagedOffset +
					    pdx->StagedLength;
					unsigned int eventEnd =
					    pArea->dwEventSt + pArea->dwEventSz;
					iWakeUp = (pdx->StagedOffset < eventEnd)
					    && (transEnd > pArea->dwEventSt);
				}

				if (iWakeUp) {
					dev_dbg(&pdx->interface->dev,
						"About to set event to notify app");
					wake_up_interruptible(&pArea->wqEvent);	//                          
					++pArea->iWakeUp;	//                       
				}
			}
		}

		pdx->dwDMAFlag = MODE_CHAR;	//                                                  

		if (!bCancel)	//                                             
		{
			//                                           
			if (pdx->bXFerWaiting)	//                          
			{
				int iReturn;
				dev_info(&pdx->interface->dev,
					 "*** RWM_Complete *** pending transfer will now be set up!!!");
				iReturn =
				    ReadWriteMem(pdx, !pdx->rDMAInfo.bOutWard,
						 pdx->rDMAInfo.wIdent,
						 pdx->rDMAInfo.dwOffset,
						 pdx->rDMAInfo.dwSize);

				if (iReturn)
					dev_err(&pdx->interface->dev,
						"RWM_Complete rw setup failed %d",
						iReturn);
			}
		}

	} else			//                    
		StageChunk(pdx);	//                      

	//                                                                            
	//                                                                                        
	//                                                                      
	bRestartCharInput = !bCancel && (pdx->dwDMAFlag == MODE_CHAR)
	    && !pdx->bXFerWaiting;

	spin_unlock(&pdx->stagedLock);	//                               

	//                                                                                    
	//                                                                                   
	//                                                                  
	if (bRestartCharInput)	//                           
		Allowi(pdx);	//                            
	dev_dbg(&pdx->interface->dev, "%s done", __func__);
}

/*                                                                           
             
  
                                                                
  
                                                                          
                                                                                 
                                                                           */
static int StageChunk(DEVICE_EXTENSION * pdx)
{
	int iReturn = U14ERR_NOERROR;
	unsigned int ChunkSize;
	int nPipe = pdx->StagedRead ? 3 : 2;	//                                           
	if (pdx->nPipes == 3)
		nPipe--;	//                           
	if (nPipe < 0)		//                                       
		return U14ERR_FAIL;

	if (!CanAcceptIoRequests(pdx))	//                   
	{
		dev_info(&pdx->interface->dev, "%s sudden remove, giving up",
			 __func__);
		return U14ERR_FAIL;	//                             
	}

	ChunkSize = (pdx->StagedLength - pdx->StagedDone);	//                          
	if (ChunkSize > STAGED_SZ)	//                        
		ChunkSize = STAGED_SZ;	//                      

	if (!pdx->StagedRead)	//              
		CopyUserSpace(pdx, ChunkSize);	//                             

	usb_fill_bulk_urb(pdx->pStagedUrb, pdx->udev,
			  pdx->StagedRead ? usb_rcvbulkpipe(pdx->udev,
							    pdx->
							    epAddr[nPipe]) :
			  usb_sndbulkpipe(pdx->udev, pdx->epAddr[nPipe]),
			  pdx->pCoherStagedIO, ChunkSize, staged_callback, pdx);
	pdx->pStagedUrb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
	usb_anchor_urb(pdx->pStagedUrb, &pdx->submitted);	//                           
	iReturn = usb_submit_urb(pdx->pStagedUrb, GFP_ATOMIC);
	if (iReturn) {
		usb_unanchor_urb(pdx->pStagedUrb);	//        
		pdx->bPipeError[nPipe] = 1;	//                                  
		dev_err(&pdx->interface->dev, "%s submit urb failed, code %d",
			__func__, iReturn);
	} else
		pdx->bStagedUrbPending = true;	//                                    
	dev_dbg(&pdx->interface->dev, "%s done so far:%d, this size:%d",
		__func__, pdx->StagedDone, ChunkSize);

	return iReturn;
}

/*                                                                          
               
  
                                                                      
                                                                               
                                        
  
                                                          
  
             
                                                                  
                                                                           
                                                                       
                                                                             
                        
                                              
*/
int ReadWriteMem(DEVICE_EXTENSION * pdx, bool Read, unsigned short wIdent,
		 unsigned int dwOffs, unsigned int dwLen)
{
	TRANSAREA *pArea = &pdx->rTransDef[wIdent];	//                   

	if (!CanAcceptIoRequests(pdx))	//                                          
	{
		dev_err(&pdx->interface->dev, "%s can't accept requests",
			__func__);
		return U14ERR_FAIL;
	}

	dev_dbg(&pdx->interface->dev,
		"%s xfer %d bytes to %s, offset %d, area %d", __func__, dwLen,
		Read ? "host" : "1401", dwOffs, wIdent);

	//                                                                                           
	//                                                                    
	if (pdx->bStagedUrbPending) {
		pdx->bXFerWaiting = true;	//                    
		dev_info(&pdx->interface->dev,
			 "%s xfer is waiting, as previous staged pending",
			 __func__);
		return U14ERR_NOERROR;
	}

	if (dwLen == 0)		//                                               
	{
		dev_dbg(&pdx->interface->dev,
			"%s OK; zero-len read/write request", __func__);
		return U14ERR_NOERROR;
	}

	if ((pArea->bCircular) &&	//                   
	    (pArea->bCircToHost) && (Read))	//                         
	{			//                                  
		bool bWait = false;	//                                 

		dev_dbg(&pdx->interface->dev,
			"Circular buffers are %d at %d and %d at %d",
			pArea->aBlocks[0].dwSize, pArea->aBlocks[0].dwOffset,
			pArea->aBlocks[1].dwSize, pArea->aBlocks[1].dwOffset);
		if (pArea->aBlocks[1].dwSize > 0)	//                                
		{
			dwOffs = pArea->aBlocks[1].dwOffset + pArea->aBlocks[1].dwSize;	//                      
			bWait = (dwOffs + dwLen) > pArea->aBlocks[0].dwOffset;	//                                
			bWait |= (dwOffs + dwLen) > pArea->dwLength;	//                              
		} else		//                                     
		{
			if (pArea->aBlocks[0].dwSize == 0)	//                            
				pArea->aBlocks[0].dwOffset = 0;
			dwOffs =
			    pArea->aBlocks[0].dwOffset +
			    pArea->aBlocks[0].dwSize;
			if ((dwOffs + dwLen) > pArea->dwLength)	//                           
			{
				pArea->aBlocks[1].dwOffset = 0;	//                           
				dwOffs = 0;
				bWait = (dwOffs + dwLen) > pArea->aBlocks[0].dwOffset;	//                                
				bWait |= (dwOffs + dwLen) > pArea->dwLength;	//                              
			}
		}

		if (bWait)	//                                 
		{
			pdx->bXFerWaiting = true;	//                    
			dev_dbg(&pdx->interface->dev,
				"%s xfer waiting for circular buffer space",
				__func__);
			return U14ERR_NOERROR;
		}

		dev_dbg(&pdx->interface->dev,
			"%s circular xfer, %d bytes starting at %d", __func__,
			dwLen, dwOffs);
	}
	//                                                
	pdx->StagedRead = Read;	//                                  
	pdx->StagedId = wIdent;	//                                       
	pdx->StagedOffset = dwOffs;	//                                  
	pdx->StagedLength = dwLen;
	pdx->StagedDone = 0;	//                          
	pdx->dwDMAFlag = MODE_LINEAR;	//                                
	pdx->bXFerWaiting = false;	//                                   

//                                                                                    
	StageChunk(pdx);	//                         

	return U14ERR_NOERROR;
}

/*                                                                           
  
           
  
                                                  
                                                                 
  
                                                                           */
static bool ReadChar(unsigned char *pChar, char *pBuf, unsigned int *pdDone,
		     unsigned int dGot)
{
	bool bRead = false;
	unsigned int dDone = *pdDone;

	if (dDone < dGot)	//                      
	{
		*pChar = (unsigned char)pBuf[dDone];	//                      
		dDone++;	//                         
		*pdDone = dDone;
		bRead = true;	//                 
	}

	return bRead;
}

#ifdef NOTUSED
/*                                                                           
  
           
  
                                                                            
  
                                                                            */
static bool ReadWord(unsigned short *pWord, char *pBuf, unsigned int *pdDone,
		     unsigned int dGot)
{
	if (ReadChar((unsigned char *)pWord, pBuf, pdDone, dGot))
		return ReadChar(((unsigned char *)pWord) + 1, pBuf, pdDone,
				dGot);
	else
		return false;
}
#endif

/*                                                                           
           
  
                                                   
                                                                           
                                                                          
                                                                            
                                
  
                                                                            */
static bool ReadHuff(volatile unsigned int *pDWord, char *pBuf,
		     unsigned int *pdDone, unsigned int dGot)
{
	unsigned char ucData;	/*                           */
	bool bReturn = true;	/*                        */
	unsigned int dwData = 0;	/*                          */

	if (ReadChar(&ucData, pBuf, pdDone, dGot)) {
		dwData = ucData;	/*               */
		if ((dwData & 0x00000080) != 0) {	/*                         */
			dwData &= 0x0000007F;	/*                        */
			if (ReadChar(&ucData, pBuf, pdDone, dGot)) {
				dwData = (dwData << 8) | ucData;
				if ((dwData & 0x00004000) != 0) {	/*                       */
					dwData &= 0x00003FFF;	/*                        */
					if (ReadChar
					    (&ucData, pBuf, pdDone, dGot))
						dwData = (dwData << 8) | ucData;
					else
						bReturn = false;
				}
			} else
				bReturn = false;	/*                    */
		}
	} else
		bReturn = false;

	*pDWord = dwData;	/*                 */
	return bReturn;
}

/*                                                                          
  
              
  
                                                                            
                                                                           
                                                                          
                                                                           
                                    
  
                                                                          
                                              
  
                                                                            */
static bool ReadDMAInfo(volatile DMADESC * pDmaDesc, DEVICE_EXTENSION * pdx,
			char *pBuf, unsigned int dwCount)
{
	bool bResult = false;	//                        
	unsigned char ucData;
	unsigned int dDone = 0;	//                                  

	dev_dbg(&pdx->interface->dev, "%s", __func__);

	if (ReadChar(&ucData, pBuf, &dDone, dwCount)) {
		unsigned char ucTransCode = (ucData & 0x0F);	//                           
		unsigned short wIdent = ((ucData >> 4) & 0x07);	//                    

		//                                    
		pDmaDesc->wTransType = ucTransCode;	//                 
		pDmaDesc->wIdent = wIdent;	//            
		pDmaDesc->dwSize = 0;	//                      
		pDmaDesc->dwOffset = 0;

		dev_dbg(&pdx->interface->dev, "%s type: %d ident: %d", __func__,
			pDmaDesc->wTransType, pDmaDesc->wIdent);

		pDmaDesc->bOutWard = (ucTransCode != TM_EXTTOHOST);	//                       

		switch (ucTransCode) {
		case TM_EXTTOHOST:	//                                                
		case TM_EXTTO1401:
			{
				bResult =
				    ReadHuff(&(pDmaDesc->dwOffset), pBuf,
					     &dDone, dwCount)
				    && ReadHuff(&(pDmaDesc->dwSize), pBuf,
						&dDone, dwCount);
				if (bResult) {
					dev_dbg(&pdx->interface->dev,
						"%s xfer offset & size %d %d",
						__func__, pDmaDesc->dwOffset,
						pDmaDesc->dwSize);

					if ((wIdent >= MAX_TRANSAREAS) ||	//                           
					    (!pdx->rTransDef[wIdent].bUsed) ||	//                       
					    (pDmaDesc->dwOffset > pdx->rTransDef[wIdent].dwLength) ||	//           
					    ((pDmaDesc->dwOffset +
					      pDmaDesc->dwSize) >
					     (pdx->rTransDef[wIdent].
					      dwLength))) {
						bResult = false;	//                 
						dev_dbg(&pdx->interface->dev,
							"%s bad param - id %d, bUsed %d, offset %d, size %d, area length %d",
							__func__, wIdent,
							pdx->rTransDef[wIdent].
							bUsed,
							pDmaDesc->dwOffset,
							pDmaDesc->dwSize,
							pdx->rTransDef[wIdent].
							dwLength);
					}
				}
				break;
			}
		default:
			break;
		}
	} else
		bResult = false;

	if (!bResult)		//                                  
		dev_err(&pdx->interface->dev, "%s error reading Esc sequence",
			__func__);

	return bResult;
}

/*                                                                           
  
                
  
                                                                         
                                                                               
                                                     
  
                 
  
                                                                             
                                                                     
  
                                                                           */
static int Handle1401Esc(DEVICE_EXTENSION * pdx, char *pCh,
			 unsigned int dwCount)
{
	int iReturn = U14ERR_FAIL;

	//                                                                                
	//                                                                                
	//                                                                                 
	if (pCh[0] == '?')	//                                
	{			//                               
	} else {
		spin_lock(&pdx->stagedLock);	//                

		if (ReadDMAInfo(&pdx->rDMAInfo, pdx, pCh, dwCount))	//                   
		{
			unsigned short wTransType = pdx->rDMAInfo.wTransType;	//                    

			dev_dbg(&pdx->interface->dev,
				"%s xfer to %s, offset %d, length %d", __func__,
				pdx->rDMAInfo.bOutWard ? "1401" : "host",
				pdx->rDMAInfo.dwOffset, pdx->rDMAInfo.dwSize);

			if (pdx->bXFerWaiting)	//                                      
			{	//                              
				dev_err(&pdx->interface->dev,
					"ERROR: DMA setup while transfer still waiting");
				spin_unlock(&pdx->stagedLock);
			} else {
				if ((wTransType == TM_EXTTOHOST)
				    || (wTransType == TM_EXTTO1401)) {
					iReturn =
					    ReadWriteMem(pdx,
							 !pdx->rDMAInfo.
							 bOutWard,
							 pdx->rDMAInfo.wIdent,
							 pdx->rDMAInfo.dwOffset,
							 pdx->rDMAInfo.dwSize);
					if (iReturn != U14ERR_NOERROR)
						dev_err(&pdx->interface->dev,
							"%s ReadWriteMem() failed %d",
							__func__, iReturn);
				} else	//                                      
					dev_err(&pdx->interface->dev,
						"%s Unknown block xfer type %d",
						__func__, wTransType);
			}
		} else		//                          
			dev_err(&pdx->interface->dev, "%s ReadDMAInfo() fail",
				__func__);

		spin_unlock(&pdx->stagedLock);	//        
	}

	dev_dbg(&pdx->interface->dev, "%s returns %d", __func__, iReturn);

	return iReturn;
}

/*                                                                           
                                                    
                                                                           */
static void ced_readchar_callback(struct urb *pUrb)
{
	DEVICE_EXTENSION *pdx = pUrb->context;
	int nGot = pUrb->actual_length;	//                    

	if (pUrb->status)	//                                
	{
		int nPipe = pdx->nPipes == 4 ? 1 : 0;	//                                 
		//                                                                                
		if (!
		    (pUrb->status == -ENOENT || pUrb->status == -ECONNRESET
		     || pUrb->status == -ESHUTDOWN)) {
			dev_err(&pdx->interface->dev,
				"%s - nonzero write bulk status received: %d",
				__func__, pUrb->status);
		} else
			dev_dbg(&pdx->interface->dev,
				"%s - 0 chars pUrb->status=%d (shutdown?)",
				__func__, pUrb->status);

		spin_lock(&pdx->err_lock);
		pdx->errors = pUrb->status;
		spin_unlock(&pdx->err_lock);
		nGot = 0;	//                         

		spin_lock(&pdx->charInLock);	//                     
		pdx->bPipeError[nPipe] = 1;	//                        
	} else {
		if ((nGot > 1) && ((pdx->pCoherCharIn[0] & 0x7f) == 0x1b))	//              
		{
			Handle1401Esc(pdx, &pdx->pCoherCharIn[1], nGot - 1);	//          
			spin_lock(&pdx->charInLock);	//                     
		} else {
			spin_lock(&pdx->charInLock);	//                     
			if (nGot > 0) {
				unsigned int i;
				if (nGot < INBUF_SZ) {
					pdx->pCoherCharIn[nGot] = 0;	//                
					dev_dbg(&pdx->interface->dev,
						"%s got %d chars >%s<",
						__func__, nGot,
						pdx->pCoherCharIn);
				}
				//                                                           
				for (i = 0; i < nGot; i++) {
					pdx->inputBuffer[pdx->dwInBuffPut++] =
					    pdx->pCoherCharIn[i] & 0x7F;
					if (pdx->dwInBuffPut >= INBUF_SZ)
						pdx->dwInBuffPut = 0;
				}

				if ((pdx->dwNumInput + nGot) <= INBUF_SZ)
					pdx->dwNumInput += nGot;	//                                    
			} else
				dev_dbg(&pdx->interface->dev, "%s read ZLP",
					__func__);
		}
	}

	pdx->bReadCharsPending = false;	//                              
	spin_unlock(&pdx->charInLock);	//                     

	Allowi(pdx);	//                              
}

/*                                                                           
         
  
                                                                             
                                                                               
                                                 
                                                                           */
int Allowi(DEVICE_EXTENSION * pdx)
{
	int iReturn = U14ERR_NOERROR;
	unsigned long flags;
	spin_lock_irqsave(&pdx->charInLock, flags);	//                                   

	//                                                                               
	//                                                                              
	//                                                                                
	//                                                             
	if (!pdx->bInDrawDown &&	//              
	    !pdx->bReadCharsPending &&	//                               
	    (pdx->dwNumInput < (INBUF_SZ / 2)) &&	//                         
	    (pdx->dwDMAFlag == MODE_CHAR) &&	//                   
	    (!pdx->bXFerWaiting) &&	//                          
	    (CanAcceptIoRequests(pdx)))	//                              
	{			//                
		unsigned int nMax = INBUF_SZ - pdx->dwNumInput;	//                  
		int nPipe = pdx->nPipes == 4 ? 1 : 0;	//                       

		dev_dbg(&pdx->interface->dev, "%s %d chars in input buffer",
			__func__, pdx->dwNumInput);

		usb_fill_int_urb(pdx->pUrbCharIn, pdx->udev,
				 usb_rcvintpipe(pdx->udev, pdx->epAddr[nPipe]),
				 pdx->pCoherCharIn, nMax, ced_readchar_callback,
				 pdx, pdx->bInterval);
		pdx->pUrbCharIn->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;	//                              
		usb_anchor_urb(pdx->pUrbCharIn, &pdx->submitted);	//                           
		iReturn = usb_submit_urb(pdx->pUrbCharIn, GFP_ATOMIC);
		if (iReturn) {
			usb_unanchor_urb(pdx->pUrbCharIn);	//                                
			pdx->bPipeError[nPipe] = 1;	//                                  
			dev_err(&pdx->interface->dev,
				"%s submit urb failed: %d", __func__, iReturn);
		} else
			pdx->bReadCharsPending = true;	//                             
	}

	spin_unlock_irqrestore(&pdx->charInLock, flags);

	return iReturn;

}

/*                                                                            
                                                                        
                                                        
                                                              
                                                                                              
                                        
                                                                            */
static long ced_ioctl(struct file *file, unsigned int cmd, unsigned long ulArg)
{
	int err = 0;
	DEVICE_EXTENSION *pdx = file->private_data;
	if (!CanAcceptIoRequests(pdx))	//                     
		return -ENODEV;

	//                                                                                            
	//                                              
	if (_IOC_DIR(cmd) & _IOC_READ)	//                                   
		err = !access_ok(VERIFY_WRITE, (void __user *)ulArg, _IOC_SIZE(cmd));	//                
	else if (_IOC_DIR(cmd) & _IOC_WRITE)	//                                        
		err = !access_ok(VERIFY_READ, (void __user *)ulArg, _IOC_SIZE(cmd));	//               
	if (err)
		return -EFAULT;

	switch (_IOC_NR(cmd)) {
	case _IOC_NR(IOCTL_CED_SENDSTRING(0)):
		return SendString(pdx, (const char __user *)ulArg,
				  _IOC_SIZE(cmd));

	case _IOC_NR(IOCTL_CED_RESET1401):
		return Reset1401(pdx);

	case _IOC_NR(IOCTL_CED_GETCHAR):
		return GetChar(pdx);

	case _IOC_NR(IOCTL_CED_SENDCHAR):
		return SendChar(pdx, (char)ulArg);

	case _IOC_NR(IOCTL_CED_STAT1401):
		return Stat1401(pdx);

	case _IOC_NR(IOCTL_CED_LINECOUNT):
		return LineCount(pdx);

	case _IOC_NR(IOCTL_CED_GETSTRING(0)):
		return GetString(pdx, (char __user *)ulArg, _IOC_SIZE(cmd));

	case _IOC_NR(IOCTL_CED_SETTRANSFER):
		return SetTransfer(pdx, (TRANSFERDESC __user *) ulArg);

	case _IOC_NR(IOCTL_CED_UNSETTRANSFER):
		return UnsetTransfer(pdx, (int)ulArg);

	case _IOC_NR(IOCTL_CED_SETEVENT):
		return SetEvent(pdx, (TRANSFEREVENT __user *) ulArg);

	case _IOC_NR(IOCTL_CED_GETOUTBUFSPACE):
		return GetOutBufSpace(pdx);

	case _IOC_NR(IOCTL_CED_GETBASEADDRESS):
		return -1;

	case _IOC_NR(IOCTL_CED_GETDRIVERREVISION):
		return (2 << 24) | (DRIVERMAJREV << 16) | DRIVERMINREV;	//                    

	case _IOC_NR(IOCTL_CED_GETTRANSFER):
		return GetTransfer(pdx, (TGET_TX_BLOCK __user *) ulArg);

	case _IOC_NR(IOCTL_CED_KILLIO1401):
		return KillIO1401(pdx);

	case _IOC_NR(IOCTL_CED_STATEOF1401):
		return StateOf1401(pdx);

	case _IOC_NR(IOCTL_CED_GRAB1401):
	case _IOC_NR(IOCTL_CED_FREE1401):
		return U14ERR_NOERROR;

	case _IOC_NR(IOCTL_CED_STARTSELFTEST):
		return StartSelfTest(pdx);

	case _IOC_NR(IOCTL_CED_CHECKSELFTEST):
		return CheckSelfTest(pdx, (TGET_SELFTEST __user *) ulArg);

	case _IOC_NR(IOCTL_CED_TYPEOF1401):
		return TypeOf1401(pdx);

	case _IOC_NR(IOCTL_CED_TRANSFERFLAGS):
		return TransferFlags(pdx);

	case _IOC_NR(IOCTL_CED_DBGPEEK):
		return DbgPeek(pdx, (TDBGBLOCK __user *) ulArg);

	case _IOC_NR(IOCTL_CED_DBGPOKE):
		return DbgPoke(pdx, (TDBGBLOCK __user *) ulArg);

	case _IOC_NR(IOCTL_CED_DBGRAMPDATA):
		return DbgRampData(pdx, (TDBGBLOCK __user *) ulArg);

	case _IOC_NR(IOCTL_CED_DBGRAMPADDR):
		return DbgRampAddr(pdx, (TDBGBLOCK __user *) ulArg);

	case _IOC_NR(IOCTL_CED_DBGGETDATA):
		return DbgGetData(pdx, (TDBGBLOCK __user *) ulArg);

	case _IOC_NR(IOCTL_CED_DBGSTOPLOOP):
		return DbgStopLoop(pdx);

	case _IOC_NR(IOCTL_CED_FULLRESET):
		pdx->bForceReset = true;	//                            
		break;

	case _IOC_NR(IOCTL_CED_SETCIRCULAR):
		return SetCircular(pdx, (TRANSFERDESC __user *) ulArg);

	case _IOC_NR(IOCTL_CED_GETCIRCBLOCK):
		return GetCircBlock(pdx, (TCIRCBLOCK __user *) ulArg);

	case _IOC_NR(IOCTL_CED_FREECIRCBLOCK):
		return FreeCircBlock(pdx, (TCIRCBLOCK __user *) ulArg);

	case _IOC_NR(IOCTL_CED_WAITEVENT):
		return WaitEvent(pdx, (int)(ulArg & 0xff), (int)(ulArg >> 8));

	case _IOC_NR(IOCTL_CED_TESTEVENT):
		return TestEvent(pdx, (int)ulArg);

	default:
		return U14ERR_NO_SUCH_FN;
	}
	return U14ERR_NOERROR;
}

static const struct file_operations ced_fops = {
	.owner = THIS_MODULE,
	.open = ced_open,
	.release = ced_release,
	.flush = ced_flush,
	.llseek = noop_llseek,
	.unlocked_ioctl = ced_ioctl,
};

/*
                                                                          
                                                         
 */
static struct usb_class_driver ced_class = {
	.name = "cedusb%d",
	.fops = &ced_fops,
	.minor_base = USB_CED_MINOR_BASE,
};

//                                                                                 
//                                 
static int ced_probe(struct usb_interface *interface,
		     const struct usb_device_id *id)
{
	DEVICE_EXTENSION *pdx;
	struct usb_host_interface *iface_desc;
	struct usb_endpoint_descriptor *endpoint;
	int i, bcdDevice;
	int retval = -ENOMEM;

	//                                                           
	pdx = kzalloc(sizeof(*pdx), GFP_KERNEL);
	if (!pdx)
		goto error;

	for (i = 0; i < MAX_TRANSAREAS; ++i)	//                           
	{
		init_waitqueue_head(&pdx->rTransDef[i].wqEvent);
	}

	//                                                                      
	//                               
	spin_lock_init(&pdx->charOutLock);
	spin_lock_init(&pdx->charInLock);
	spin_lock_init(&pdx->stagedLock);

	//                                    
	kref_init(&pdx->kref);
	mutex_init(&pdx->io_mutex);
	spin_lock_init(&pdx->err_lock);
	init_usb_anchor(&pdx->submitted);

	pdx->udev = usb_get_dev(interface_to_usbdev(interface));
	pdx->interface = interface;

	//                               
	bcdDevice = pdx->udev->descriptor.bcdDevice;
	i = (bcdDevice >> 8);
	if (i == 0)
		pdx->s1401Type = TYPEU1401;
	else if ((i >= 1) && (i <= 23))
		pdx->s1401Type = i + 2;
	else {
		dev_err(&interface->dev, "%s Unknown device. bcdDevice = %d",
			__func__, bcdDevice);
		goto error;
	}
	//                                                                        
	//                                                
	iface_desc = interface->cur_altsetting;
	pdx->nPipes = iface_desc->desc.bNumEndpoints;
	dev_info(&interface->dev, "1401Type=%d with %d End Points",
		 pdx->s1401Type, pdx->nPipes);
	if ((pdx->nPipes < 3) || (pdx->nPipes > 4))
		goto error;

	//                                                   
	pdx->pUrbCharOut = usb_alloc_urb(0, GFP_KERNEL);	//                     
	pdx->pUrbCharIn = usb_alloc_urb(0, GFP_KERNEL);	//                    
	pdx->pStagedUrb = usb_alloc_urb(0, GFP_KERNEL);	//                   
	if (!pdx->pUrbCharOut || !pdx->pUrbCharIn || !pdx->pStagedUrb) {
		dev_err(&interface->dev, "%s URB alloc failed", __func__);
		goto error;
	}

	pdx->pCoherStagedIO =
	    usb_alloc_coherent(pdx->udev, STAGED_SZ, GFP_KERNEL,
			       &pdx->pStagedUrb->transfer_dma);
	pdx->pCoherCharOut =
	    usb_alloc_coherent(pdx->udev, OUTBUF_SZ, GFP_KERNEL,
			       &pdx->pUrbCharOut->transfer_dma);
	pdx->pCoherCharIn =
	    usb_alloc_coherent(pdx->udev, INBUF_SZ, GFP_KERNEL,
			       &pdx->pUrbCharIn->transfer_dma);
	if (!pdx->pCoherCharOut || !pdx->pCoherCharIn || !pdx->pCoherStagedIO) {
		dev_err(&interface->dev, "%s Coherent buffer alloc failed",
			__func__);
		goto error;
	}

	for (i = 0; i < pdx->nPipes; ++i) {
		endpoint = &iface_desc->endpoint[i].desc;
		pdx->epAddr[i] = endpoint->bEndpointAddress;
		dev_info(&interface->dev, "Pipe %d, ep address %02x", i,
			 pdx->epAddr[i]);
		if (((pdx->nPipes == 3) && (i == 0)) ||	//                        
		    ((pdx->nPipes == 4) && (i == 1))) {
			pdx->bInterval = endpoint->bInterval;	//                                     
			dev_info(&interface->dev, "Pipe %d, bInterval = %d", i,
				 pdx->bInterval);
		}
		//                                                  
		if (i == pdx->nPipes - 1)	//                              
		{
			pdx->bIsUSB2 =
			    le16_to_cpu(endpoint->wMaxPacketSize) > 64;
			dev_info(&pdx->interface->dev, "USB%d",
				 pdx->bIsUSB2 + 1);
		}
	}

	/*                                                */
	usb_set_intfdata(interface, pdx);

	/*                                                */
	retval = usb_register_dev(interface, &ced_class);
	if (retval) {
		/*                                                     */
		dev_err(&interface->dev,
			"Not able to get a minor for this device.\n");
		usb_set_intfdata(interface, NULL);
		goto error;
	}

	/*                                                            */
	dev_info(&interface->dev,
		 "USB CEDUSB device now attached to cedusb #%d",
		 interface->minor);
	return 0;

error:
	if (pdx)
		kref_put(&pdx->kref, ced_delete);	//                       
	return retval;
}

static void ced_disconnect(struct usb_interface *interface)
{
	DEVICE_EXTENSION *pdx = usb_get_intfdata(interface);
	int minor = interface->minor;
	int i;

	usb_set_intfdata(interface, NULL);	//                                  
	usb_deregister_dev(interface, &ced_class);	//                                  

	mutex_lock(&pdx->io_mutex);	//                                
	ced_draw_down(pdx);	//                             
	for (i = 0; i < MAX_TRANSAREAS; ++i) {
		int iErr = ClearArea(pdx, i);	//                           
		if (iErr == U14ERR_UNLOCKFAIL)
			dev_err(&pdx->interface->dev, "%s Area %d was in used",
				__func__, i);
	}
	pdx->interface = NULL;	//                                 
	mutex_unlock(&pdx->io_mutex);

	usb_kill_anchored_urbs(&pdx->submitted);

	kref_put(&pdx->kref, ced_delete);	//                          

	dev_info(&interface->dev, "USB cedusb #%d now disconnected", minor);
}

//                                                                         
//                                                                       
//                                                                         
//            
void ced_draw_down(DEVICE_EXTENSION * pdx)
{
	int time;
	dev_dbg(&pdx->interface->dev, "%s called", __func__);

	pdx->bInDrawDown = true;
	time = usb_wait_anchor_empty_timeout(&pdx->submitted, 3000);
	if (!time) {		//                                 
		usb_kill_anchored_urbs(&pdx->submitted);
		dev_err(&pdx->interface->dev, "%s timed out", __func__);
	}
	pdx->bInDrawDown = false;
}

static int ced_suspend(struct usb_interface *intf, pm_message_t message)
{
	DEVICE_EXTENSION *pdx = usb_get_intfdata(intf);
	if (!pdx)
		return 0;
	ced_draw_down(pdx);

	dev_dbg(&pdx->interface->dev, "%s called", __func__);
	return 0;
}

static int ced_resume(struct usb_interface *intf)
{
	DEVICE_EXTENSION *pdx = usb_get_intfdata(intf);
	if (!pdx)
		return 0;
	dev_dbg(&pdx->interface->dev, "%s called", __func__);
	return 0;
}

static int ced_pre_reset(struct usb_interface *intf)
{
	DEVICE_EXTENSION *pdx = usb_get_intfdata(intf);
	dev_dbg(&pdx->interface->dev, "%s", __func__);
	mutex_lock(&pdx->io_mutex);
	ced_draw_down(pdx);
	return 0;
}

static int ced_post_reset(struct usb_interface *intf)
{
	DEVICE_EXTENSION *pdx = usb_get_intfdata(intf);
	dev_dbg(&pdx->interface->dev, "%s", __func__);

	/*                                                    */
	pdx->errors = -EPIPE;
	mutex_unlock(&pdx->io_mutex);

	return 0;
}

static struct usb_driver ced_driver = {
	.name = "cedusb",
	.probe = ced_probe,
	.disconnect = ced_disconnect,
	.suspend = ced_suspend,
	.resume = ced_resume,
	.pre_reset = ced_pre_reset,
	.post_reset = ced_post_reset,
	.id_table = ced_table,
	.supports_autosuspend = 1,
};

module_usb_driver(ced_driver);
MODULE_LICENSE("GPL");
