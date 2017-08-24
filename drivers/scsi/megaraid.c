/*
 *
 *			Linux MegaRAID device driver
 *
 * Copyright (c) 2002  LSI Logic Corporation.
 *
 *	   This program is free software; you can redistribute it and/or
 *	   modify it under the terms of the GNU General Public License
 *	   as published by the Free Software Foundation; either version
 *	   2 of the License, or (at your option) any later version.
 *
 * Copyright (c) 2002  Red Hat, Inc. All rights reserved.
 *	  - fixes
 *	  - speed-ups (list handling fixes, issued_list, optimizations.)
 *	  - lots of cleanups.
 *
 * Copyright (c) 2003  Christoph Hellwig  <hch@lst.de>
 *	  - new-style, hotplug-aware pci probing and scsi registration
 *
 * Version : v2.00.4 Mon Nov 14 14:02:43 EST 2005 - Seokmann Ju
 * 						<Seokmann.Ju@lsil.com>
 *
 * Description: Linux device driver for LSI Logic MegaRAID controller
 *
 * Supported controllers: MegaRAID 418, 428, 438, 466, 762, 467, 471, 490, 493
 *					518, 520, 531, 532
 *
 * This driver is supported by LSI Logic, with assistance from Red Hat, Dell,
 * and others. Please send updates to the mailing list
 * linux-scsi@vger.kernel.org .
 *
 */

#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/blkdev.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/reboot.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <scsi/scsicam.h>

#include "scsi.h"
#include <scsi/scsi_host.h>

#include "megaraid.h"

#define MEGARAID_MODULE_VERSION "2.00.4"

MODULE_AUTHOR ("sju@lsil.com");
MODULE_DESCRIPTION ("LSI Logic MegaRAID legacy driver");
MODULE_LICENSE ("GPL");
MODULE_VERSION(MEGARAID_MODULE_VERSION);

static DEFINE_MUTEX(megadev_mutex);
static unsigned int max_cmd_per_lun = DEF_CMD_PER_LUN;
module_param(max_cmd_per_lun, uint, 0);
MODULE_PARM_DESC(max_cmd_per_lun, "Maximum number of commands which can be issued to a single LUN (default=DEF_CMD_PER_LUN=63)");

static unsigned short int max_sectors_per_io = MAX_SECTORS_PER_IO;
module_param(max_sectors_per_io, ushort, 0);
MODULE_PARM_DESC(max_sectors_per_io, "Maximum number of sectors per I/O request (default=MAX_SECTORS_PER_IO=128)");


static unsigned short int max_mbox_busy_wait = MBOX_BUSY_WAIT;
module_param(max_mbox_busy_wait, ushort, 0);
MODULE_PARM_DESC(max_mbox_busy_wait, "Maximum wait for mailbox in microseconds if busy (default=MBOX_BUSY_WAIT=10)");

#define RDINDOOR(adapter)	readl((adapter)->mmio_base + 0x20)
#define RDOUTDOOR(adapter)	readl((adapter)->mmio_base + 0x2C)
#define WRINDOOR(adapter,value)	 writel(value, (adapter)->mmio_base + 0x20)
#define WROUTDOOR(adapter,value) writel(value, (adapter)->mmio_base + 0x2C)

/*
                   
 */

static int hba_count;
static adapter_t *hba_soft_state[MAX_CONTROLLERS];
static struct proc_dir_entry *mega_proc_dir_entry;

/*                            */
static struct mega_hbas mega_hbas[MAX_CONTROLLERS];

static long
megadev_unlocked_ioctl(struct file *filep, unsigned int cmd, unsigned long arg);

/*
                                                                             
 */
static const struct file_operations megadev_fops = {
	.owner		= THIS_MODULE,
	.unlocked_ioctl	= megadev_unlocked_ioctl,
	.open		= megadev_open,
	.llseek		= noop_llseek,
};

/*
                                                                              
                                                                            
                        
 */
static struct mcontroller mcontroller[MAX_CONTROLLERS];

/*                            */
static u32 driver_ver = 0x02000000;

/*                                                         */
static int major;

#define IS_RAID_CH(hba, ch)	(((hba)->mega_ch_class >> (ch)) & 0x01)


/*
                                                   
 */
static int trace_level;

/* 
                       
                                       
  
                                                               
 */
static int
mega_setup_mailbox(adapter_t *adapter)
{
	unsigned long	align;

	adapter->una_mbox64 = pci_alloc_consistent(adapter->dev,
			sizeof(mbox64_t), &adapter->una_mbox64_dma);

	if( !adapter->una_mbox64 ) return -1;
		
	adapter->mbox = &adapter->una_mbox64->mbox;

	adapter->mbox = (mbox_t *)((((unsigned long) adapter->mbox) + 15) &
			(~0UL ^ 0xFUL));

	adapter->mbox64 = (mbox64_t *)(((unsigned long)adapter->mbox) - 8);

	align = ((void *)adapter->mbox) - ((void *)&adapter->una_mbox64->mbox);

	adapter->mbox_dma = adapter->una_mbox64_dma + 8 + align;

	/*
                                                                     
  */
	if( adapter->flag & BOARD_IOMAP ) {

		outb(adapter->mbox_dma & 0xFF,
				adapter->host->io_port + MBOX_PORT0);

		outb((adapter->mbox_dma >> 8) & 0xFF,
				adapter->host->io_port + MBOX_PORT1);

		outb((adapter->mbox_dma >> 16) & 0xFF,
				adapter->host->io_port + MBOX_PORT2);

		outb((adapter->mbox_dma >> 24) & 0xFF,
				adapter->host->io_port + MBOX_PORT3);

		outb(ENABLE_MBOX_BYTE,
				adapter->host->io_port + ENABLE_MBOX_REGION);

		irq_ack(adapter);

		irq_enable(adapter);
	}

	return 0;
}


/*
                       
                                       
  
                                                                    
                                                       
 */
static int
mega_query_adapter(adapter_t *adapter)
{
	dma_addr_t	prod_info_dma_handle;
	mega_inquiry3	*inquiry3;
	u8	raw_mbox[sizeof(struct mbox_out)];
	mbox_t	*mbox;
	int	retval;

	/*                                    */

	mbox = (mbox_t *)raw_mbox;

	memset((void *)adapter->mega_buffer, 0, MEGA_BUFFER_SIZE);
	memset(&mbox->m_out, 0, sizeof(raw_mbox));

	/*
                                 
                                                                    
                             
  */
	mbox->m_out.xferaddr = (u32)adapter->buf_dma_handle;

	inquiry3 = (mega_inquiry3 *)adapter->mega_buffer;

	raw_mbox[0] = FC_NEW_CONFIG;		/*                     */
	raw_mbox[2] = NC_SUBOP_ENQUIRY3;	/*           */
	raw_mbox[3] = ENQ3_GET_SOLICITED_FULL;	/*           */

	/*                                      */
	if ((retval = issue_scb_block(adapter, raw_mbox))) {
		/*                                   */

		mraid_ext_inquiry	*ext_inq;
		mraid_inquiry		*inq;
		dma_addr_t		dma_handle;

		ext_inq = pci_alloc_consistent(adapter->dev,
				sizeof(mraid_ext_inquiry), &dma_handle);

		if( ext_inq == NULL ) return -1;

		inq = &ext_inq->raid_inq;

		mbox->m_out.xferaddr = (u32)dma_handle;

		/*                                  */
		mbox->m_out.cmd = MEGA_MBOXCMD_ADPEXTINQ;

		issue_scb_block(adapter, raw_mbox);

		/*
                                                    
                            
   */
		mega_8_to_40ld(inq, inquiry3,
				(mega_product_info *)&adapter->product_info);

		pci_free_consistent(adapter->dev, sizeof(mraid_ext_inquiry),
				ext_inq, dma_handle);

	} else {		/*                      */
		adapter->flag |= BOARD_40LD;

		/*
                                                              
              
   */
		prod_info_dma_handle = pci_map_single(adapter->dev, (void *)
				&adapter->product_info,
				sizeof(mega_product_info), PCI_DMA_FROMDEVICE);

		mbox->m_out.xferaddr = prod_info_dma_handle;

		raw_mbox[0] = FC_NEW_CONFIG;	/*                     */
		raw_mbox[2] = NC_SUBOP_PRODUCT_INFO;	/*           */

		if ((retval = issue_scb_block(adapter, raw_mbox)))
			printk(KERN_WARNING
			"megaraid: Product_info cmd failed with error: %d\n",
				retval);

		pci_unmap_single(adapter->dev, prod_info_dma_handle,
				sizeof(mega_product_info), PCI_DMA_FROMDEVICE);
	}


	/*
                                                      
  */
	adapter->host->max_channel =
		adapter->product_info.nchannels + NVIRT_CHAN -1;

	adapter->host->max_id = 16;	/*                         */

	adapter->host->max_lun = 7;	/*                                   */

	adapter->host->cmd_per_lun = max_cmd_per_lun;

	adapter->numldrv = inquiry3->num_ldrv;

	adapter->max_cmds = adapter->product_info.max_commands;

	if(adapter->max_cmds > MAX_COMMANDS)
		adapter->max_cmds = MAX_COMMANDS;

	adapter->host->can_queue = adapter->max_cmds - 1;

	/*
                                                                       
            
  */
	mega_get_max_sgl(adapter);

	adapter->host->sg_tablesize = adapter->sglen;

	/*                                          
                                                                       
                                                                    
                     */
	if (adapter->product_info.subsysvid == PCI_VENDOR_ID_HP) {
		sprintf (adapter->fw_version, "%c%d%d.%d%d",
			 adapter->product_info.fw_version[2],
			 0,
			 adapter->product_info.fw_version[1] & 0x0f,
			 0,
			 adapter->product_info.fw_version[0] & 0x0f);
		sprintf (adapter->bios_version, "%c%d%d.%d%d",
			 adapter->product_info.bios_version[2],
			 0,
			 adapter->product_info.bios_version[1] & 0x0f,
			 0,
			 adapter->product_info.bios_version[0] & 0x0f);
	} else {
		memcpy(adapter->fw_version,
				(char *)adapter->product_info.fw_version, 4);
		adapter->fw_version[4] = 0;

		memcpy(adapter->bios_version,
				(char *)adapter->product_info.bios_version, 4);

		adapter->bios_version[4] = 0;
	}

	printk(KERN_NOTICE "megaraid: [%s:%s] detected %d logical drives.\n",
		adapter->fw_version, adapter->bios_version, adapter->numldrv);

	/*
                                           
  */
	adapter->support_ext_cdb = mega_support_ext_cdb(adapter);
	if (adapter->support_ext_cdb)
		printk(KERN_NOTICE "megaraid: supports extended CDBs.\n");


	return 0;
}

/* 
                  
                                       
  
                                             
 */
static inline void
mega_runpendq(adapter_t *adapter)
{
	if(!list_empty(&adapter->pending_list))
		__mega_runpendq(adapter);
}

/*
                   
                                  
                                                    
  
                                                     
 */
static int
megaraid_queue_lck(Scsi_Cmnd *scmd, void (*done)(Scsi_Cmnd *))
{
	adapter_t	*adapter;
	scb_t	*scb;
	int	busy=0;
	unsigned long flags;

	adapter = (adapter_t *)scmd->device->host->hostdata;

	scmd->scsi_done = done;


	/*
                                    
                                                               
                                                              
                                                                
                                                                
                          
  */

	spin_lock_irqsave(&adapter->lock, flags);
	scb = mega_build_cmd(adapter, scmd, &busy);
	if (!scb)
		goto out;

	scb->state |= SCB_PENDQ;
	list_add_tail(&scb->list, &adapter->pending_list);

	/*
                                                          
                                                      
                     
  */
	if (atomic_read(&adapter->quiescent) == 0)
		mega_runpendq(adapter);

	busy = 0;
 out:
	spin_unlock_irqrestore(&adapter->lock, flags);
	return busy;
}

static DEF_SCSI_QCMD(megaraid_queue)

/* 
                      
                                       
                                         
  
                                                                         
            
 */
static inline scb_t *
mega_allocate_scb(adapter_t *adapter, Scsi_Cmnd *cmd)
{
	struct list_head *head = &adapter->free_list;
	scb_t	*scb;

	/*                               */
	if( !list_empty(head) ) {

		scb = list_entry(head->next, scb_t, list);

		list_del_init(head->next);

		scb->state = SCB_ACTIVE;
		scb->cmd = cmd;
		scb->dma_type = MEGA_DMA_TYPE_NONE;

		return scb;
	}

	return NULL;
}

/* 
                      
                                       
                                
                                       
  
                                                                              
                          
 */
static inline int
mega_get_ldrv_num(adapter_t *adapter, Scsi_Cmnd *cmd, int channel)
{
	int		tgt;
	int		ldrv_num;

	tgt = cmd->device->id;
	
	if ( tgt > adapter->this_id )
		tgt--;	/*                                         */

	ldrv_num = (channel * 15) + tgt;


	/*
                                                                  
  */
	if( adapter->boot_ldrv_enabled ) {
		if( ldrv_num == 0 ) {
			ldrv_num = adapter->boot_ldrv;
		}
		else {
			if( ldrv_num <= adapter->boot_ldrv ) {
				ldrv_num--;
			}
		}
	}

	/*
                                                                    
                                                                    
   
                                                                        
                                                       
   
                                        
  */

	if (adapter->support_random_del && adapter->read_ldidmap )
		switch (cmd->cmnd[0]) {
		case READ_6:	/*              */
		case WRITE_6:	/*              */
		case READ_10:	/*              */
		case WRITE_10:
			ldrv_num += 0x80;
		}

	return ldrv_num;
}

/* 
                   
                                       
                                         
                                    
  
                                                                              
                                                                      
                                                                   
  
                                                                          
                 
 */
static scb_t *
mega_build_cmd(adapter_t *adapter, Scsi_Cmnd *cmd, int *busy)
{
	mega_ext_passthru	*epthru;
	mega_passthru	*pthru;
	scb_t	*scb;
	mbox_t	*mbox;
	u32	seg;
	char	islogical;
	int	max_ldrv_num;
	int	channel = 0;
	int	target = 0;
	int	ldrv_num = 0;   /*                      */


	/*
                                          
  */
	if((cmd->cmnd[0] == MEGA_INTERNAL_CMD))
		return (scb_t *)cmd->host_scribble;

	/*
                                                                      
  */
	islogical = adapter->logdrv_chan[cmd->device->channel];

	/*
                                                                      
                                                                      
                                                                        
                                                                        
                                 
  */
	if( adapter->boot_pdrv_enabled ) {
		if( islogical ) {
			/*                 */
			channel = cmd->device->channel -
				adapter->product_info.nchannels;
		}
		else {
			/*                          */
			channel = cmd->device->channel; 
			target = cmd->device->id;

			/*
                                                      
                                                       
                                                     
    */
			if( target == 0 ) {
				target = adapter->boot_pdrv_tgt;
			}
			else if( target == adapter->boot_pdrv_tgt ) {
				target = 0;
			}
		}
	}
	else {
		if( islogical ) {
			/*                             */
			channel = cmd->device->channel;	
		}
		else {
			/*                  */
			channel = cmd->device->channel - NVIRT_CHAN;	
			target = cmd->device->id;
		}
	}


	if(islogical) {

		/*                                                     */
		if (cmd->device->lun) {
			cmd->result = (DID_BAD_TARGET << 16);
			cmd->scsi_done(cmd);
			return NULL;
		}

		ldrv_num = mega_get_ldrv_num(adapter, cmd, channel);


		max_ldrv_num = (adapter->flag & BOARD_40LD) ?
			MAX_LOGICAL_DRIVES_40LD : MAX_LOGICAL_DRIVES_8LD;

		/*
                                                             
             
   */
		if(adapter->read_ldidmap)
			max_ldrv_num += 0x80;

		if(ldrv_num > max_ldrv_num ) {
			cmd->result = (DID_BAD_TARGET << 16);
			cmd->scsi_done(cmd);
			return NULL;
		}

	}
	else {
		if( cmd->device->lun > 7) {
			/*
                                                   
             
    */
			cmd->result = (DID_BAD_TARGET << 16);
			cmd->scsi_done(cmd);
			return NULL;
		}
	}

	/*
   
                          
   
  */
	if(islogical) {
		switch (cmd->cmnd[0]) {
		case TEST_UNIT_READY:
#if MEGA_HAVE_CLUSTERING
			/*
                                                         
                                  
    */
			if( !adapter->has_cluster ) {
				cmd->result = (DID_OK << 16);
				cmd->scsi_done(cmd);
				return NULL;
			}

			if(!(scb = mega_allocate_scb(adapter, cmd))) {
				*busy = 1;
				return NULL;
			}

			scb->raw_mbox[0] = MEGA_CLUSTER_CMD;
			scb->raw_mbox[2] = MEGA_RESERVATION_STATUS;
			scb->raw_mbox[3] = ldrv_num;

			scb->dma_direction = PCI_DMA_NONE;

			return scb;
#else
			cmd->result = (DID_OK << 16);
			cmd->scsi_done(cmd);
			return NULL;
#endif

		case MODE_SENSE: {
			char *buf;
			struct scatterlist *sg;

			sg = scsi_sglist(cmd);
			buf = kmap_atomic(sg_page(sg)) + sg->offset;

			memset(buf, 0, cmd->cmnd[4]);
			kunmap_atomic(buf - sg->offset);

			cmd->result = (DID_OK << 16);
			cmd->scsi_done(cmd);
			return NULL;
		}

		case READ_CAPACITY:
		case INQUIRY:

			if(!(adapter->flag & (1L << cmd->device->channel))) {

				printk(KERN_NOTICE
					"scsi%d: scanning scsi channel %d ",
						adapter->host->host_no,
						cmd->device->channel);
				printk("for logical drives.\n");

				adapter->flag |= (1L << cmd->device->channel);
			}

			/*                                        */
			if(!(scb = mega_allocate_scb(adapter, cmd))) {
				*busy = 1;
				return NULL;
			}
			pthru = scb->pthru;

			mbox = (mbox_t *)scb->raw_mbox;
			memset(mbox, 0, sizeof(scb->raw_mbox));
			memset(pthru, 0, sizeof(mega_passthru));

			pthru->timeout = 0;
			pthru->ars = 1;
			pthru->reqsenselen = 14;
			pthru->islogical = 1;
			pthru->logdrv = ldrv_num;
			pthru->cdblen = cmd->cmd_len;
			memcpy(pthru->cdb, cmd->cmnd, cmd->cmd_len);

			if( adapter->has_64bit_addr ) {
				mbox->m_out.cmd = MEGA_MBOXCMD_PASSTHRU64;
			}
			else {
				mbox->m_out.cmd = MEGA_MBOXCMD_PASSTHRU;
			}

			scb->dma_direction = PCI_DMA_FROMDEVICE;

			pthru->numsgelements = mega_build_sglist(adapter, scb,
				&pthru->dataxferaddr, &pthru->dataxferlen);

			mbox->m_out.xferaddr = scb->pthru_dma_addr;

			return scb;

		case READ_6:
		case WRITE_6:
		case READ_10:
		case WRITE_10:
		case READ_12:
		case WRITE_12:

			/*                                       */
			if(!(scb = mega_allocate_scb(adapter, cmd))) {
				*busy = 1;
				return NULL;
			}
			mbox = (mbox_t *)scb->raw_mbox;

			memset(mbox, 0, sizeof(scb->raw_mbox));
			mbox->m_out.logdrv = ldrv_num;

			/*
                                                      
                                                     
    */
			if( adapter->has_64bit_addr ) {
				mbox->m_out.cmd = (*cmd->cmnd & 0x02) ?
					MEGA_MBOXCMD_LWRITE64:
					MEGA_MBOXCMD_LREAD64 ;
			}
			else {
				mbox->m_out.cmd = (*cmd->cmnd & 0x02) ?
					MEGA_MBOXCMD_LWRITE:
					MEGA_MBOXCMD_LREAD ;
			}

			/*
                                          
    */
			if( cmd->cmd_len == 6 ) {
				mbox->m_out.numsectors = (u32) cmd->cmnd[4];
				mbox->m_out.lba =
					((u32)cmd->cmnd[1] << 16) |
					((u32)cmd->cmnd[2] << 8) |
					(u32)cmd->cmnd[3];

				mbox->m_out.lba &= 0x1FFFFF;

#if MEGA_HAVE_STATS
				/*
                                                
                                              
                        
     */
				if (*cmd->cmnd == READ_6) {
					adapter->nreads[ldrv_num%0x80]++;
					adapter->nreadblocks[ldrv_num%0x80] +=
						mbox->m_out.numsectors;
				} else {
					adapter->nwrites[ldrv_num%0x80]++;
					adapter->nwriteblocks[ldrv_num%0x80] +=
						mbox->m_out.numsectors;
				}
#endif
			}

			/*
                                           
    */
			if( cmd->cmd_len == 10 ) {
				mbox->m_out.numsectors =
					(u32)cmd->cmnd[8] |
					((u32)cmd->cmnd[7] << 8);
				mbox->m_out.lba =
					((u32)cmd->cmnd[2] << 24) |
					((u32)cmd->cmnd[3] << 16) |
					((u32)cmd->cmnd[4] << 8) |
					(u32)cmd->cmnd[5];

#if MEGA_HAVE_STATS
				if (*cmd->cmnd == READ_10) {
					adapter->nreads[ldrv_num%0x80]++;
					adapter->nreadblocks[ldrv_num%0x80] +=
						mbox->m_out.numsectors;
				} else {
					adapter->nwrites[ldrv_num%0x80]++;
					adapter->nwriteblocks[ldrv_num%0x80] +=
						mbox->m_out.numsectors;
				}
#endif
			}

			/*
                                           
    */
			if( cmd->cmd_len == 12 ) {
				mbox->m_out.lba =
					((u32)cmd->cmnd[2] << 24) |
					((u32)cmd->cmnd[3] << 16) |
					((u32)cmd->cmnd[4] << 8) |
					(u32)cmd->cmnd[5];

				mbox->m_out.numsectors =
					((u32)cmd->cmnd[6] << 24) |
					((u32)cmd->cmnd[7] << 16) |
					((u32)cmd->cmnd[8] << 8) |
					(u32)cmd->cmnd[9];

#if MEGA_HAVE_STATS
				if (*cmd->cmnd == READ_12) {
					adapter->nreads[ldrv_num%0x80]++;
					adapter->nreadblocks[ldrv_num%0x80] +=
						mbox->m_out.numsectors;
				} else {
					adapter->nwrites[ldrv_num%0x80]++;
					adapter->nwriteblocks[ldrv_num%0x80] +=
						mbox->m_out.numsectors;
				}
#endif
			}

			/*
                             
    */
			if( (*cmd->cmnd & 0x0F) == 0x08 ) {
				scb->dma_direction = PCI_DMA_FROMDEVICE;
			}
			else {
				scb->dma_direction = PCI_DMA_TODEVICE;
			}

			/*                               */
			mbox->m_out.numsgelements = mega_build_sglist(adapter, scb,
					(u32 *)&mbox->m_out.xferaddr, &seg);

			return scb;

#if MEGA_HAVE_CLUSTERING
		case RESERVE:	/*              */
		case RELEASE:

			/*
                                                         
    */
			if( ! adapter->has_cluster ) {

				cmd->result = (DID_BAD_TARGET << 16);
				cmd->scsi_done(cmd);
				return NULL;
			}

			/*                                       */
			if(!(scb = mega_allocate_scb(adapter, cmd))) {
				*busy = 1;
				return NULL;
			}

			scb->raw_mbox[0] = MEGA_CLUSTER_CMD;
			scb->raw_mbox[2] = ( *cmd->cmnd == RESERVE ) ?
				MEGA_RESERVE_LD : MEGA_RELEASE_LD;

			scb->raw_mbox[3] = ldrv_num;

			scb->dma_direction = PCI_DMA_NONE;

			return scb;
#endif

		default:
			cmd->result = (DID_BAD_TARGET << 16);
			cmd->scsi_done(cmd);
			return NULL;
		}
	}

	/*
                           
  */
	else {
		/*                                        */
		if(!(scb = mega_allocate_scb(adapter, cmd))) {
			*busy = 1;
			return NULL;
		}

		mbox = (mbox_t *)scb->raw_mbox;
		memset(mbox, 0, sizeof(scb->raw_mbox));

		if( adapter->support_ext_cdb ) {

			epthru = mega_prepare_extpassthru(adapter, scb, cmd,
					channel, target);

			mbox->m_out.cmd = MEGA_MBOXCMD_EXTPTHRU;

			mbox->m_out.xferaddr = scb->epthru_dma_addr;

		}
		else {

			pthru = mega_prepare_passthru(adapter, scb, cmd,
					channel, target);

			/*                    */
			if( adapter->has_64bit_addr ) {
				mbox->m_out.cmd = MEGA_MBOXCMD_PASSTHRU64;
			}
			else {
				mbox->m_out.cmd = MEGA_MBOXCMD_PASSTHRU;
			}

			mbox->m_out.xferaddr = scb->pthru_dma_addr;

		}
		return scb;
	}
	return NULL;
}


/* 
                          
                                       
                                
                                         
                                              
                                         
  
                                                   
 */
static mega_passthru *
mega_prepare_passthru(adapter_t *adapter, scb_t *scb, Scsi_Cmnd *cmd,
		int channel, int target)
{
	mega_passthru *pthru;

	pthru = scb->pthru;
	memset(pthru, 0, sizeof (mega_passthru));

	/*                               */
	pthru->timeout = 2;

	pthru->ars = 1;
	pthru->reqsenselen = 14;
	pthru->islogical = 0;

	pthru->channel = (adapter->flag & BOARD_40LD) ? 0 : channel;

	pthru->target = (adapter->flag & BOARD_40LD) ?
		(channel << 4) | target : target;

	pthru->cdblen = cmd->cmd_len;
	pthru->logdrv = cmd->device->lun;

	memcpy(pthru->cdb, cmd->cmnd, cmd->cmd_len);

	/*                              */
	scb->dma_direction = PCI_DMA_BIDIRECTIONAL;

	/*                                                               */
	switch (cmd->cmnd[0]) {
	case INQUIRY:
	case READ_CAPACITY:
		if(!(adapter->flag & (1L << cmd->device->channel))) {

			printk(KERN_NOTICE
				"scsi%d: scanning scsi channel %d [P%d] ",
					adapter->host->host_no,
					cmd->device->channel, channel);
			printk("for physical devices.\n");

			adapter->flag |= (1L << cmd->device->channel);
		}
		/*              */
	default:
		pthru->numsgelements = mega_build_sglist(adapter, scb,
				&pthru->dataxferaddr, &pthru->dataxferlen);
		break;
	}
	return pthru;
}


/* 
                             
                                       
                                
                                         
                                              
                                         
  
                                                                          
                                                                
 */
static mega_ext_passthru *
mega_prepare_extpassthru(adapter_t *adapter, scb_t *scb, Scsi_Cmnd *cmd,
		int channel, int target)
{
	mega_ext_passthru	*epthru;

	epthru = scb->epthru;
	memset(epthru, 0, sizeof(mega_ext_passthru));

	/*                               */
	epthru->timeout = 2;

	epthru->ars = 1;
	epthru->reqsenselen = 14;
	epthru->islogical = 0;

	epthru->channel = (adapter->flag & BOARD_40LD) ? 0 : channel;
	epthru->target = (adapter->flag & BOARD_40LD) ?
		(channel << 4) | target : target;

	epthru->cdblen = cmd->cmd_len;
	epthru->logdrv = cmd->device->lun;

	memcpy(epthru->cdb, cmd->cmnd, cmd->cmd_len);

	/*                              */
	scb->dma_direction = PCI_DMA_BIDIRECTIONAL;

	switch(cmd->cmnd[0]) {
	case INQUIRY:
	case READ_CAPACITY:
		if(!(adapter->flag & (1L << cmd->device->channel))) {

			printk(KERN_NOTICE
				"scsi%d: scanning scsi channel %d [P%d] ",
					adapter->host->host_no,
					cmd->device->channel, channel);
			printk("for physical devices.\n");

			adapter->flag |= (1L << cmd->device->channel);
		}
		/*              */
	default:
		epthru->numsgelements = mega_build_sglist(adapter, scb,
				&epthru->dataxferaddr, &epthru->dataxferlen);
		break;
	}

	return epthru;
}

static void
__mega_runpendq(adapter_t *adapter)
{
	scb_t *scb;
	struct list_head *pos, *next;

	/*                                        */
	list_for_each_safe(pos, next, &adapter->pending_list) {

		scb = list_entry(pos, scb_t, list);

		if( !(scb->state & SCB_ISSUED) ) {

			if( issue_scb(adapter, scb) != 0 )
				return;
		}
	}

	return;
}


/* 
              
                                       
                            
  
                                                                           
                                                                     
             
 */
static int
issue_scb(adapter_t *adapter, scb_t *scb)
{
	volatile mbox64_t	*mbox64 = adapter->mbox64;
	volatile mbox_t		*mbox = adapter->mbox;
	unsigned int	i = 0;

	if(unlikely(mbox->m_in.busy)) {
		do {
			udelay(1);
			i++;
		} while( mbox->m_in.busy && (i < max_mbox_busy_wait) );

		if(mbox->m_in.busy) return -1;
	}

	/*                                       */
	memcpy((char *)&mbox->m_out, (char *)scb->raw_mbox, 
			sizeof(struct mbox_out));

	mbox->m_out.cmdid = scb->idx;	/*           */
	mbox->m_in.busy = 1;		/*          */


	/*
                                       
  */
	atomic_inc(&adapter->pend_cmds);

	switch (mbox->m_out.cmd) {
	case MEGA_MBOXCMD_LREAD64:
	case MEGA_MBOXCMD_LWRITE64:
	case MEGA_MBOXCMD_PASSTHRU64:
	case MEGA_MBOXCMD_EXTPTHRU:
		mbox64->xfer_segment_lo = mbox->m_out.xferaddr;
		mbox64->xfer_segment_hi = 0;
		mbox->m_out.xferaddr = 0xFFFFFFFF;
		break;
	default:
		mbox64->xfer_segment_lo = 0;
		mbox64->xfer_segment_hi = 0;
	}

	/*
                    
  */
	scb->state |= SCB_ISSUED;

	if( likely(adapter->flag & BOARD_MEMMAP) ) {
		mbox->m_in.poll = 0;
		mbox->m_in.ack = 0;
		WRINDOOR(adapter, adapter->mbox_dma | 0x1);
	}
	else {
		irq_enable(adapter);
		issue_command(adapter);
	}

	return 0;
}

/*
                                                   
 */
static inline int
mega_busywait_mbox (adapter_t *adapter)
{
	if (adapter->mbox->m_in.busy)
		return __mega_busywait_mbox(adapter);
	return 0;
}

/* 
                    
                                       
                          
  
                                                    
 */
static int
issue_scb_block(adapter_t *adapter, u_char *raw_mbox)
{
	volatile mbox64_t *mbox64 = adapter->mbox64;
	volatile mbox_t *mbox = adapter->mbox;
	u8	byte;

	/*                            */
	if(mega_busywait_mbox (adapter))
		goto bug_blocked_mailbox;

	/*                                       */
	memcpy((char *) mbox, raw_mbox, sizeof(struct mbox_out));
	mbox->m_out.cmdid = 0xFE;
	mbox->m_in.busy = 1;

	switch (raw_mbox[0]) {
	case MEGA_MBOXCMD_LREAD64:
	case MEGA_MBOXCMD_LWRITE64:
	case MEGA_MBOXCMD_PASSTHRU64:
	case MEGA_MBOXCMD_EXTPTHRU:
		mbox64->xfer_segment_lo = mbox->m_out.xferaddr;
		mbox64->xfer_segment_hi = 0;
		mbox->m_out.xferaddr = 0xFFFFFFFF;
		break;
	default:
		mbox64->xfer_segment_lo = 0;
		mbox64->xfer_segment_hi = 0;
	}

	if( likely(adapter->flag & BOARD_MEMMAP) ) {
		mbox->m_in.poll = 0;
		mbox->m_in.ack = 0;
		mbox->m_in.numstatus = 0xFF;
		mbox->m_in.status = 0xFF;
		WRINDOOR(adapter, adapter->mbox_dma | 0x1);

		while((volatile u8)mbox->m_in.numstatus == 0xFF)
			cpu_relax();

		mbox->m_in.numstatus = 0xFF;

		while( (volatile u8)mbox->m_in.poll != 0x77 )
			cpu_relax();

		mbox->m_in.poll = 0;
		mbox->m_in.ack = 0x77;

		WRINDOOR(adapter, adapter->mbox_dma | 0x2);

		while(RDINDOOR(adapter) & 0x2)
			cpu_relax();
	}
	else {
		irq_disable(adapter);
		issue_command(adapter);

		while (!((byte = irq_state(adapter)) & INTR_VALID))
			cpu_relax();

		set_irq_state(adapter, byte);
		irq_enable(adapter);
		irq_ack(adapter);
	}

	return mbox->m_in.status;

bug_blocked_mailbox:
	printk(KERN_WARNING "megaraid: Blocked mailbox......!!\n");
	udelay (1000);
	return -1;
}


/* 
                          
             
                                    
  
                                                       
                                                                            
                                      
 */
static irqreturn_t
megaraid_isr_iomapped(int irq, void *devp)
{
	adapter_t	*adapter = devp;
	unsigned long	flags;
	u8	status;
	u8	nstatus;
	u8	completed[MAX_FIRMWARE_STATUS];
	u8	byte;
	int	handled = 0;


	/*
                                                       
  */
	spin_lock_irqsave(&adapter->lock, flags);

	do {
		/*                                       */
		byte = irq_state(adapter);
		if( (byte & VALID_INTR_BYTE) == 0 ) {
			/*
                              
    */
			goto out_unlock;
		}
		set_irq_state(adapter, byte);

		while((nstatus = (volatile u8)adapter->mbox->m_in.numstatus)
				== 0xFF)
			cpu_relax();
		adapter->mbox->m_in.numstatus = 0xFF;

		status = adapter->mbox->m_in.status;

		/*
                                        
   */
		atomic_sub(nstatus, &adapter->pend_cmds);

		memcpy(completed, (void *)adapter->mbox->m_in.completed, 
				nstatus);

		/*                       */
		irq_ack(adapter);

		mega_cmd_done(adapter, completed, nstatus, status);

		mega_rundoneq(adapter);

		handled = 1;

		/*                                   */
		if(atomic_read(&adapter->quiescent) == 0) {
			mega_runpendq(adapter);
		}

	} while(1);

 out_unlock:

	spin_unlock_irqrestore(&adapter->lock, flags);

	return IRQ_RETVAL(handled);
}


/* 
                           
             
                                    
  
                                                           
                                                                            
                                      
 */
static irqreturn_t
megaraid_isr_memmapped(int irq, void *devp)
{
	adapter_t	*adapter = devp;
	unsigned long	flags;
	u8	status;
	u32	dword = 0;
	u8	nstatus;
	u8	completed[MAX_FIRMWARE_STATUS];
	int	handled = 0;


	/*
                                                       
  */
	spin_lock_irqsave(&adapter->lock, flags);

	do {
		/*                                       */
		dword = RDOUTDOOR(adapter);
		if(dword != 0x10001234) {
			/*
                              
    */
			goto out_unlock;
		}
		WROUTDOOR(adapter, 0x10001234);

		while((nstatus = (volatile u8)adapter->mbox->m_in.numstatus)
				== 0xFF) {
			cpu_relax();
		}
		adapter->mbox->m_in.numstatus = 0xFF;

		status = adapter->mbox->m_in.status;

		/*
                                        
   */
		atomic_sub(nstatus, &adapter->pend_cmds);

		memcpy(completed, (void *)adapter->mbox->m_in.completed, 
				nstatus);

		/*                       */
		WRINDOOR(adapter, 0x2);

		handled = 1;

		while( RDINDOOR(adapter) & 0x02 )
			cpu_relax();

		mega_cmd_done(adapter, completed, nstatus, status);

		mega_rundoneq(adapter);

		/*                                   */
		if(atomic_read(&adapter->quiescent) == 0) {
			mega_runpendq(adapter);
		}

	} while(1);

 out_unlock:

	spin_unlock_irqrestore(&adapter->lock, flags);

	return IRQ_RETVAL(handled);
}
/* 
                  
                                       
                                                  
                                          
                                                 
  
                                                                    
 */
static void
mega_cmd_done(adapter_t *adapter, u8 completed[], int nstatus, int status)
{
	mega_ext_passthru	*epthru = NULL;
	struct scatterlist	*sgl;
	Scsi_Cmnd	*cmd = NULL;
	mega_passthru	*pthru = NULL;
	mbox_t	*mbox = NULL;
	u8	c;
	scb_t	*scb;
	int	islogical;
	int	cmdid;
	int	i;

	/*
                                                                       
                     
  */
	for( i = 0; i < nstatus; i++ ) {

		cmdid = completed[i];

		if( cmdid == CMDID_INT_CMDS ) { /*                  */
			scb = &adapter->int_scb;
			cmd = scb->cmd;
			mbox = (mbox_t *)scb->raw_mbox;

			/*
                                                         
                                 
    */
			pthru = scb->pthru;

		}
		else {
			scb = &adapter->scb_list[cmdid];

			/*
                                                 
    */
			if( !(scb->state & SCB_ISSUED) || scb->cmd == NULL ) {
				printk(KERN_CRIT
					"megaraid: invalid command ");
				printk("Id %d, scb->state:%x, scsi cmd:%p\n",
					cmdid, scb->state, scb->cmd);

				continue;
			}

			/*
                                         
    */
			if( scb->state & SCB_ABORT ) {

				printk(KERN_WARNING
				"megaraid: aborted cmd [%x] complete.\n",
					scb->idx);

				scb->cmd->result = (DID_ABORT << 16);

				list_add_tail(SCSI_LIST(scb->cmd),
						&adapter->completed_list);

				mega_free_scb(adapter, scb);

				continue;
			}

			/*
                                         
    */
			if( scb->state & SCB_RESET ) {

				printk(KERN_WARNING
				"megaraid: reset cmd [%x] complete.\n",
					scb->idx);

				scb->cmd->result = (DID_RESET << 16);

				list_add_tail(SCSI_LIST(scb->cmd),
						&adapter->completed_list);

				mega_free_scb (adapter, scb);

				continue;
			}

			cmd = scb->cmd;
			pthru = scb->pthru;
			epthru = scb->epthru;
			mbox = (mbox_t *)scb->raw_mbox;

#if MEGA_HAVE_STATS
			{

			int	logdrv = mbox->m_out.logdrv;

			islogical = adapter->logdrv_chan[cmd->channel];
			/*
                                                      
                                                
                
    */
			if( status && islogical && (cmd->cmnd[0] == READ_6 ||
						cmd->cmnd[0] == READ_10 ||
						cmd->cmnd[0] == READ_12)) {
				/*
                                                  
                                 
     */
				adapter->rd_errors[logdrv%0x80]++;
			}

			if( status && islogical && (cmd->cmnd[0] == WRITE_6 ||
						cmd->cmnd[0] == WRITE_10 ||
						cmd->cmnd[0] == WRITE_12)) {
				/*
                                                  
                                 
     */
				adapter->wr_errors[logdrv%0x80]++;
			}

			}
#endif
		}

		/*
                                                               
                                                            
                                                                
       
   */
		islogical = adapter->logdrv_chan[cmd->device->channel];
		if( cmd->cmnd[0] == INQUIRY && !islogical ) {

			sgl = scsi_sglist(cmd);
			if( sg_page(sgl) ) {
				c = *(unsigned char *) sg_virt(&sgl[0]);
			} else {
				printk(KERN_WARNING
				       "megaraid: invalid sg.\n");
				c = 0;
			}

			if(IS_RAID_CH(adapter, cmd->device->channel) &&
					((c & 0x1F ) == TYPE_DISK)) {
				status = 0xF0;
			}
		}

		/*                                                        */
		cmd->result = 0;

		/*                                             */
		switch (status) {
		case 0x00:	/*                                 */
			cmd->result |= (DID_OK << 16);
			break;

		case 0x02:	/*                    
                                   */

			/*                                    */
			if( mbox->m_out.cmd == MEGA_MBOXCMD_PASSTHRU ||
				mbox->m_out.cmd == MEGA_MBOXCMD_PASSTHRU64 ) {

				memcpy(cmd->sense_buffer, pthru->reqsensearea,
						14);

				cmd->result = (DRIVER_SENSE << 24) |
					(DID_OK << 16) |
					(CHECK_CONDITION << 1);
			}
			else {
				if (mbox->m_out.cmd == MEGA_MBOXCMD_EXTPTHRU) {

					memcpy(cmd->sense_buffer,
						epthru->reqsensearea, 14);

					cmd->result = (DRIVER_SENSE << 24) |
						(DID_OK << 16) |
						(CHECK_CONDITION << 1);
				} else {
					cmd->sense_buffer[0] = 0x70;
					cmd->sense_buffer[2] = ABORTED_COMMAND;
					cmd->result |= (CHECK_CONDITION << 1);
				}
			}
			break;

		case 0x08:	/*                            
                        */
			cmd->result |= (DID_BUS_BUSY << 16) | status;
			break;

		default:
#if MEGA_HAVE_CLUSTERING
			/*
                                       
                                    
    */
			if( cmd->cmnd[0] == TEST_UNIT_READY ) {
				cmd->result |= (DID_ERROR << 16) |
					(RESERVATION_CONFLICT << 1);
			}
			else
			/*
                                                    
                                              
    */
			if( status == 1 &&
				(cmd->cmnd[0] == RESERVE ||
					 cmd->cmnd[0] == RELEASE) ) {

				cmd->result |= (DID_ERROR << 16) |
					(RESERVATION_CONFLICT << 1);
			}
			else
#endif
				cmd->result |= (DID_BAD_TARGET << 16)|status;
		}

		/*
                                                         
                                                    
    
                                                             
                                            
   */
		if( cmdid == CMDID_INT_CMDS ) { /*                  */
			cmd->result = status;

			/*
                                                       
    */
			list_del_init(&scb->list);
			scb->state = SCB_FREE;
		}
		else {
			mega_free_scb(adapter, scb);
		}

		/*                                            */
		list_add_tail(SCSI_LIST(cmd), &adapter->completed_list);
	}
}


/*
                  
  
                                                           
 */
static void
mega_rundoneq (adapter_t *adapter)
{
	Scsi_Cmnd *cmd;
	struct list_head *pos;

	list_for_each(pos, &adapter->completed_list) {

		struct scsi_pointer* spos = (struct scsi_pointer *)pos;

		cmd = list_entry(spos, Scsi_Cmnd, SCp);
		cmd->scsi_done(cmd);
	}

	INIT_LIST_HEAD(&adapter->completed_list);
}


/*
                       
                                                                              
 */
static void
mega_free_scb(adapter_t *adapter, scb_t *scb)
{
	switch( scb->dma_type ) {

	case MEGA_DMA_TYPE_NONE:
		break;

	case MEGA_SGLIST:
		scsi_dma_unmap(scb->cmd);
		break;
	default:
		break;
	}

	/*
                                
  */
	list_del_init(&scb->list);

	/*                                  */
	scb->state = SCB_FREE;
	scb->cmd = NULL;

	list_add(&scb->list, &adapter->free_list);
}


static int
__mega_busywait_mbox (adapter_t *adapter)
{
	volatile mbox_t *mbox = adapter->mbox;
	long counter;

	for (counter = 0; counter < 10000; counter++) {
		if (!mbox->m_in.busy)
			return 0;
		udelay(100);
		cond_resched();
	}
	return -1;		/*                        */
}

/*
                        
                                                                             
 */
static int
mega_build_sglist(adapter_t *adapter, scb_t *scb, u32 *buf, u32 *len)
{
	struct scatterlist *sg;
	Scsi_Cmnd	*cmd;
	int	sgcnt;
	int	idx;

	cmd = scb->cmd;

	/*
                                                            
   
                                                                
  */
	sgcnt = scsi_dma_map(cmd);

	scb->dma_type = MEGA_SGLIST;

	BUG_ON(sgcnt > adapter->sglen || sgcnt < 0);

	*len = 0;

	if (scsi_sg_count(cmd) == 1 && !adapter->has_64bit_addr) {
		sg = scsi_sglist(cmd);
		scb->dma_h_bulkdata = sg_dma_address(sg);
		*buf = (u32)scb->dma_h_bulkdata;
		*len = sg_dma_len(sg);
		return 0;
	}

	scsi_for_each_sg(cmd, sg, sgcnt, idx) {
		if (adapter->has_64bit_addr) {
			scb->sgl64[idx].address = sg_dma_address(sg);
			*len += scb->sgl64[idx].length = sg_dma_len(sg);
		} else {
			scb->sgl[idx].address = sg_dma_address(sg);
			*len += scb->sgl[idx].length = sg_dma_len(sg);
		}
	}

	/*                                 */
	*buf = scb->sgl_dma_addr;

	/*                             */
	return sgcnt;
}


/*
                   
  
                                                                              
                                    
 */
static void
mega_8_to_40ld(mraid_inquiry *inquiry, mega_inquiry3 *enquiry3,
		mega_product_info *product_info)
{
	int i;

	product_info->max_commands = inquiry->adapter_info.max_commands;
	enquiry3->rebuild_rate = inquiry->adapter_info.rebuild_rate;
	product_info->nchannels = inquiry->adapter_info.nchannels;

	for (i = 0; i < 4; i++) {
		product_info->fw_version[i] =
			inquiry->adapter_info.fw_version[i];

		product_info->bios_version[i] =
			inquiry->adapter_info.bios_version[i];
	}
	enquiry3->cache_flush_interval =
		inquiry->adapter_info.cache_flush_interval;

	product_info->dram_size = inquiry->adapter_info.dram_size;

	enquiry3->num_ldrv = inquiry->logdrv_info.num_ldrv;

	for (i = 0; i < MAX_LOGICAL_DRIVES_8LD; i++) {
		enquiry3->ldrv_size[i] = inquiry->logdrv_info.ldrv_size[i];
		enquiry3->ldrv_prop[i] = inquiry->logdrv_info.ldrv_prop[i];
		enquiry3->ldrv_state[i] = inquiry->logdrv_info.ldrv_state[i];
	}

	for (i = 0; i < (MAX_PHYSICAL_DRIVES); i++)
		enquiry3->pdrv_state[i] = inquiry->pdrv_info.pdrv_state[i];
}

static inline void
mega_free_sgl(adapter_t *adapter)
{
	scb_t	*scb;
	int	i;

	for(i = 0; i < adapter->max_cmds; i++) {

		scb = &adapter->scb_list[i];

		if( scb->sgl64 ) {
			pci_free_consistent(adapter->dev,
				sizeof(mega_sgl64) * adapter->sglen,
				scb->sgl64,
				scb->sgl_dma_addr);

			scb->sgl64 = NULL;
		}

		if( scb->pthru ) {
			pci_free_consistent(adapter->dev, sizeof(mega_passthru),
				scb->pthru, scb->pthru_dma_addr);

			scb->pthru = NULL;
		}

		if( scb->epthru ) {
			pci_free_consistent(adapter->dev,
				sizeof(mega_ext_passthru),
				scb->epthru, scb->epthru_dma_addr);

			scb->epthru = NULL;
		}

	}
}


/*
                                        
 */
const char *
megaraid_info(struct Scsi_Host *host)
{
	static char buffer[512];
	adapter_t *adapter;

	adapter = (adapter_t *)host->hostdata;

	sprintf (buffer,
		 "LSI Logic MegaRAID %s %d commands %d targs %d chans %d luns",
		 adapter->fw_version, adapter->product_info.max_commands,
		 adapter->host->max_id, adapter->host->max_channel,
		 adapter->host->max_lun);
	return buffer;
}

/*
                                                                          
                                                             
 */
static int
megaraid_abort(Scsi_Cmnd *cmd)
{
	adapter_t	*adapter;
	int		rval;

	adapter = (adapter_t *)cmd->device->host->hostdata;

	rval =  megaraid_abort_and_reset(adapter, cmd, SCB_ABORT);

	/*
                                                            
                                             
  */
	mega_rundoneq(adapter);

	return rval;
}


static int
megaraid_reset(struct scsi_cmnd *cmd)
{
	adapter_t	*adapter;
	megacmd_t	mc;
	int		rval;

	adapter = (adapter_t *)cmd->device->host->hostdata;

#if MEGA_HAVE_CLUSTERING
	mc.cmd = MEGA_CLUSTER_CMD;
	mc.opcode = MEGA_RESET_RESERVATIONS;

	if( mega_internal_command(adapter, &mc, NULL) != 0 ) {
		printk(KERN_WARNING
				"megaraid: reservation reset failed.\n");
	}
	else {
		printk(KERN_INFO "megaraid: reservation reset.\n");
	}
#endif

	spin_lock_irq(&adapter->lock);

	rval =  megaraid_abort_and_reset(adapter, cmd, SCB_RESET);

	/*
                                                            
                                             
  */
	mega_rundoneq(adapter);
	spin_unlock_irq(&adapter->lock);

	return rval;
}

/* 
                             
                                 
                                             
                             
  
                                                                           
                                                                     
 */
static int
megaraid_abort_and_reset(adapter_t *adapter, Scsi_Cmnd *cmd, int aor)
{
	struct list_head	*pos, *next;
	scb_t			*scb;

	printk(KERN_WARNING "megaraid: %s cmd=%x <c=%d t=%d l=%d>\n",
	     (aor == SCB_ABORT)? "ABORTING":"RESET",
	     cmd->cmnd[0], cmd->device->channel, 
	     cmd->device->id, cmd->device->lun);

	if(list_empty(&adapter->pending_list))
		return FALSE;

	list_for_each_safe(pos, next, &adapter->pending_list) {

		scb = list_entry(pos, scb_t, list);

		if (scb->cmd == cmd) { /*               */

			scb->state |= aor;

			/*
                                                      
                                                     
                                                        
                      
    */
			if( scb->state & SCB_ISSUED ) {

				printk(KERN_WARNING
					"megaraid: %s[%x], fw owner.\n",
					(aor==SCB_ABORT) ? "ABORTING":"RESET",
					scb->idx);

				return FALSE;
			}
			else {

				/*
                                              
           
     */
				printk(KERN_WARNING
					"megaraid: %s-[%x], driver owner.\n",
					(aor==SCB_ABORT) ? "ABORTING":"RESET",
					scb->idx);

				mega_free_scb(adapter, scb);

				if( aor == SCB_ABORT ) {
					cmd->result = (DID_ABORT << 16);
				}
				else {
					cmd->result = (DID_RESET << 16);
				}

				list_add_tail(SCSI_LIST(cmd),
						&adapter->completed_list);

				return TRUE;
			}
		}
	}

	return FALSE;
}

static inline int
make_local_pdev(adapter_t *adapter, struct pci_dev **pdev)
{
	*pdev = alloc_pci_dev();

	if( *pdev == NULL ) return -1;

	memcpy(*pdev, adapter->dev, sizeof(struct pci_dev));

	if( pci_set_dma_mask(*pdev, DMA_BIT_MASK(32)) != 0 ) {
		kfree(*pdev);
		return -1;
	}

	return 0;
}

static inline void
free_local_pdev(struct pci_dev *pdev)
{
	kfree(pdev);
}

/* 
                          
                                                
                               
  
                                         
 */
static inline void *
mega_allocate_inquiry(dma_addr_t *dma_handle, struct pci_dev *pdev)
{
	return pci_alloc_consistent(pdev, sizeof(mega_inquiry3), dma_handle);
}


static inline void
mega_free_inquiry(void *inquiry, dma_addr_t dma_handle, struct pci_dev *pdev)
{
	pci_free_consistent(pdev, sizeof(mega_inquiry3), inquiry, dma_handle);
}


#ifdef CONFIG_PROC_FS
/*                                  */

/* 
                     
                                        
                     
  
                                                          
 */
static int
proc_show_config(struct seq_file *m, void *v)
{

	adapter_t *adapter = m->private;

	seq_puts(m, MEGARAID_VERSION);
	if(adapter->product_info.product_name[0])
		seq_printf(m, "%s\n", adapter->product_info.product_name);

	seq_puts(m, "Controller Type: ");

	if( adapter->flag & BOARD_MEMMAP )
		seq_puts(m, "438/466/467/471/493/518/520/531/532\n");
	else
		seq_puts(m, "418/428/434\n");

	if(adapter->flag & BOARD_40LD)
		seq_puts(m, "Controller Supports 40 Logical Drives\n");

	if(adapter->flag & BOARD_64BIT)
		seq_puts(m, "Controller capable of 64-bit memory addressing\n");
	if( adapter->has_64bit_addr )
		seq_puts(m, "Controller using 64-bit memory addressing\n");
	else
		seq_puts(m, "Controller is not using 64-bit memory addressing\n");

	seq_printf(m, "Base = %08lx, Irq = %d, ",
		   adapter->base, adapter->host->irq);

	seq_printf(m, "Logical Drives = %d, Channels = %d\n",
		   adapter->numldrv, adapter->product_info.nchannels);

	seq_printf(m, "Version =%s:%s, DRAM = %dMb\n",
		   adapter->fw_version, adapter->bios_version,
		   adapter->product_info.dram_size);

	seq_printf(m, "Controller Queue Depth = %d, Driver Queue Depth = %d\n",
		   adapter->product_info.max_commands, adapter->max_cmds);

	seq_printf(m, "support_ext_cdb    = %d\n", adapter->support_ext_cdb);
	seq_printf(m, "support_random_del = %d\n", adapter->support_random_del);
	seq_printf(m, "boot_ldrv_enabled  = %d\n", adapter->boot_ldrv_enabled);
	seq_printf(m, "boot_ldrv          = %d\n", adapter->boot_ldrv);
	seq_printf(m, "boot_pdrv_enabled  = %d\n", adapter->boot_pdrv_enabled);
	seq_printf(m, "boot_pdrv_ch       = %d\n", adapter->boot_pdrv_ch);
	seq_printf(m, "boot_pdrv_tgt      = %d\n", adapter->boot_pdrv_tgt);
	seq_printf(m, "quiescent          = %d\n",
		   atomic_read(&adapter->quiescent));
	seq_printf(m, "has_cluster        = %d\n", adapter->has_cluster);

	seq_puts(m, "\nModule Parameters:\n");
	seq_printf(m, "max_cmd_per_lun    = %d\n", max_cmd_per_lun);
	seq_printf(m, "max_sectors_per_io = %d\n", max_sectors_per_io);
	return 0;
}

/* 
                   
                                        
                     
  
                                                          
 */
static int
proc_show_stat(struct seq_file *m, void *v)
{
	adapter_t *adapter = m->private;
#if MEGA_HAVE_STATS
	int	i;
#endif

	seq_puts(m, "Statistical Information for this controller\n");
	seq_printf(m, "pend_cmds = %d\n", atomic_read(&adapter->pend_cmds));
#if MEGA_HAVE_STATS
	for(i = 0; i < adapter->numldrv; i++) {
		seq_printf(m, "Logical Drive %d:\n", i);
		seq_printf(m, "\tReads Issued = %lu, Writes Issued = %lu\n",
			   adapter->nreads[i], adapter->nwrites[i]);
		seq_printf(m, "\tSectors Read = %lu, Sectors Written = %lu\n",
			   adapter->nreadblocks[i], adapter->nwriteblocks[i]);
		seq_printf(m, "\tRead errors = %lu, Write errors = %lu\n\n",
			   adapter->rd_errors[i], adapter->wr_errors[i]);
	}
#else
	seq_puts(m, "IO and error counters not compiled in driver.\n");
#endif
	return 0;
}


/* 
                   
                                        
                     
  
                                                                            
                         
 */
static int
proc_show_mbox(struct seq_file *m, void *v)
{
	adapter_t	*adapter = m->private;
	volatile mbox_t	*mbox = adapter->mbox;

	seq_puts(m, "Contents of Mail Box Structure\n");
	seq_printf(m, "  Fw Command   = 0x%02x\n", mbox->m_out.cmd);
	seq_printf(m, "  Cmd Sequence = 0x%02x\n", mbox->m_out.cmdid);
	seq_printf(m, "  No of Sectors= %04d\n", mbox->m_out.numsectors);
	seq_printf(m, "  LBA          = 0x%02x\n", mbox->m_out.lba);
	seq_printf(m, "  DTA          = 0x%08x\n", mbox->m_out.xferaddr);
	seq_printf(m, "  Logical Drive= 0x%02x\n", mbox->m_out.logdrv);
	seq_printf(m, "  No of SG Elmt= 0x%02x\n", mbox->m_out.numsgelements);
	seq_printf(m, "  Busy         = %01x\n", mbox->m_in.busy);
	seq_printf(m, "  Status       = 0x%02x\n", mbox->m_in.status);
	return 0;
}


/* 
                           
                                        
                     
  
                               
 */
static int
proc_show_rebuild_rate(struct seq_file *m, void *v)
{
	adapter_t	*adapter = m->private;
	dma_addr_t	dma_handle;
	caddr_t		inquiry;
	struct pci_dev	*pdev;

	if( make_local_pdev(adapter, &pdev) != 0 )
		return 0;

	if( (inquiry = mega_allocate_inquiry(&dma_handle, pdev)) == NULL )
		goto free_pdev;

	if( mega_adapinq(adapter, dma_handle) != 0 ) {
		seq_puts(m, "Adapter inquiry failed.\n");
		printk(KERN_WARNING "megaraid: inquiry failed.\n");
		goto free_inquiry;
	}

	if( adapter->flag & BOARD_40LD )
		seq_printf(m, "Rebuild Rate: [%d%%]\n",
			   ((mega_inquiry3 *)inquiry)->rebuild_rate);
	else
		seq_printf(m, "Rebuild Rate: [%d%%]\n",
			((mraid_ext_inquiry *)
			 inquiry)->raid_inq.adapter_info.rebuild_rate);

free_inquiry:
	mega_free_inquiry(inquiry, dma_handle, pdev);
free_pdev:
	free_local_pdev(pdev);
	return 0;
}


/* 
                      
                                        
                     
  
                                                                  
 */
static int
proc_show_battery(struct seq_file *m, void *v)
{
	adapter_t	*adapter = m->private;
	dma_addr_t	dma_handle;
	caddr_t		inquiry;
	struct pci_dev	*pdev;
	u8	battery_status;

	if( make_local_pdev(adapter, &pdev) != 0 )
		return 0;

	if( (inquiry = mega_allocate_inquiry(&dma_handle, pdev)) == NULL )
		goto free_pdev;

	if( mega_adapinq(adapter, dma_handle) != 0 ) {
		seq_printf(m, "Adapter inquiry failed.\n");
		printk(KERN_WARNING "megaraid: inquiry failed.\n");
		goto free_inquiry;
	}

	if( adapter->flag & BOARD_40LD ) {
		battery_status = ((mega_inquiry3 *)inquiry)->battery_status;
	}
	else {
		battery_status = ((mraid_ext_inquiry *)inquiry)->
			raid_inq.adapter_info.battery_status;
	}

	/*
                             
  */
	seq_printf(m, "Battery Status:[%d]", battery_status);

	if(battery_status == MEGA_BATT_CHARGE_DONE)
		seq_puts(m, " Charge Done");

	if(battery_status & MEGA_BATT_MODULE_MISSING)
		seq_puts(m, " Module Missing");
	
	if(battery_status & MEGA_BATT_LOW_VOLTAGE)
		seq_puts(m, " Low Voltage");
	
	if(battery_status & MEGA_BATT_TEMP_HIGH)
		seq_puts(m, " Temperature High");
	
	if(battery_status & MEGA_BATT_PACK_MISSING)
		seq_puts(m, " Pack Missing");
	
	if(battery_status & MEGA_BATT_CHARGE_INPROG)
		seq_puts(m, " Charge In-progress");
	
	if(battery_status & MEGA_BATT_CHARGE_FAIL)
		seq_puts(m, " Charge Fail");
	
	if(battery_status & MEGA_BATT_CYCLES_EXCEEDED)
		seq_puts(m, " Cycles Exceeded");

	seq_putc(m, '\n');

free_inquiry:
	mega_free_inquiry(inquiry, dma_handle, pdev);
free_pdev:
	free_local_pdev(pdev);
	return 0;
}


/*
                       
 */
static void
mega_print_inquiry(struct seq_file *m, char *scsi_inq)
{
	int	i;

	seq_puts(m, "  Vendor: ");
	seq_write(m, scsi_inq + 8, 8);
	seq_puts(m, "  Model: ");
	seq_write(m, scsi_inq + 16, 16);
	seq_puts(m, "  Rev: ");
	seq_write(m, scsi_inq + 32, 4);
	seq_putc(m, '\n');

	i = scsi_inq[0] & 0x1f;
	seq_printf(m, "  Type:   %s ", scsi_device_type(i));

	seq_printf(m, "                 ANSI SCSI revision: %02x",
		   scsi_inq[2] & 0x07);

	if( (scsi_inq[2] & 0x07) == 1 && (scsi_inq[3] & 0x0f) == 1 )
		seq_puts(m, " CCS\n");
	else
		seq_putc(m, '\n');
}

/* 
                   
                                        
                                      
                                       
  
                                                 
 */
static int
proc_show_pdrv(struct seq_file *m, adapter_t *adapter, int channel)
{
	dma_addr_t	dma_handle;
	char		*scsi_inq;
	dma_addr_t	scsi_inq_dma_handle;
	caddr_t		inquiry;
	struct pci_dev	*pdev;
	u8	*pdrv_state;
	u8	state;
	int	tgt;
	int	max_channels;
	int	i;

	if( make_local_pdev(adapter, &pdev) != 0 )
		return 0;

	if( (inquiry = mega_allocate_inquiry(&dma_handle, pdev)) == NULL )
		goto free_pdev;

	if( mega_adapinq(adapter, dma_handle) != 0 ) {
		seq_puts(m, "Adapter inquiry failed.\n");
		printk(KERN_WARNING "megaraid: inquiry failed.\n");
		goto free_inquiry;
	}


	scsi_inq = pci_alloc_consistent(pdev, 256, &scsi_inq_dma_handle);
	if( scsi_inq == NULL ) {
		seq_puts(m, "memory not available for scsi inq.\n");
		goto free_inquiry;
	}

	if( adapter->flag & BOARD_40LD ) {
		pdrv_state = ((mega_inquiry3 *)inquiry)->pdrv_state;
	}
	else {
		pdrv_state = ((mraid_ext_inquiry *)inquiry)->
			raid_inq.pdrv_info.pdrv_state;
	}

	max_channels = adapter->product_info.nchannels;

	if( channel >= max_channels ) {
		goto free_pci;
	}

	for( tgt = 0; tgt <= MAX_TARGET; tgt++ ) {

		i = channel*16 + tgt;

		state = *(pdrv_state + i);
		switch( state & 0x0F ) {
		case PDRV_ONLINE:
			seq_printf(m, "Channel:%2d Id:%2d State: Online",
				   channel, tgt);
			break;

		case PDRV_FAILED:
			seq_printf(m, "Channel:%2d Id:%2d State: Failed",
				   channel, tgt);
			break;

		case PDRV_RBLD:
			seq_printf(m, "Channel:%2d Id:%2d State: Rebuild",
				   channel, tgt);
			break;

		case PDRV_HOTSPARE:
			seq_printf(m, "Channel:%2d Id:%2d State: Hot spare",
				   channel, tgt);
			break;

		default:
			seq_printf(m, "Channel:%2d Id:%2d State: Un-configured",
				   channel, tgt);
			break;
		}

		/*
                                                      
                                                   
                                                  
   */
		memset(scsi_inq, 0, 256);
		if( mega_internal_dev_inquiry(adapter, channel, tgt,
				scsi_inq_dma_handle) ||
				(scsi_inq[0] & 0x1F) != TYPE_DISK ) {
			continue;
		}

		/*
                                               
                           
   */
		seq_puts(m, ".\n");
		mega_print_inquiry(m, scsi_inq);
	}

free_pci:
	pci_free_consistent(pdev, 256, scsi_inq, scsi_inq_dma_handle);
free_inquiry:
	mega_free_inquiry(inquiry, dma_handle, pdev);
free_pdev:
	free_local_pdev(pdev);
	return 0;
}

/* 
                       
                                        
                     
  
                                                                       
 */
static int
proc_show_pdrv_ch0(struct seq_file *m, void *v)
{
	return proc_show_pdrv(m, m->private, 0);
}


/* 
                       
                                        
                     
  
                                                                       
 */
static int
proc_show_pdrv_ch1(struct seq_file *m, void *v)
{
	return proc_show_pdrv(m, m->private, 1);
}


/* 
                       
                                        
                     
  
                                                                       
 */
static int
proc_show_pdrv_ch2(struct seq_file *m, void *v)
{
	return proc_show_pdrv(m, m->private, 2);
}


/* 
                       
                                        
                     
  
                                                                       
 */
static int
proc_show_pdrv_ch3(struct seq_file *m, void *v)
{
	return proc_show_pdrv(m, m->private, 3);
}


/* 
                   
                                        
                                       
                                             
                                         
  
                                                                              
                            
 */
static int
proc_show_rdrv(struct seq_file *m, adapter_t *adapter, int start, int end )
{
	dma_addr_t	dma_handle;
	logdrv_param	*lparam;
	megacmd_t	mc;
	char		*disk_array;
	dma_addr_t	disk_array_dma_handle;
	caddr_t		inquiry;
	struct pci_dev	*pdev;
	u8	*rdrv_state;
	int	num_ldrv;
	u32	array_sz;
	int	i;

	if( make_local_pdev(adapter, &pdev) != 0 )
		return 0;

	if( (inquiry = mega_allocate_inquiry(&dma_handle, pdev)) == NULL )
		goto free_pdev;

	if( mega_adapinq(adapter, dma_handle) != 0 ) {
		seq_puts(m, "Adapter inquiry failed.\n");
		printk(KERN_WARNING "megaraid: inquiry failed.\n");
		goto free_inquiry;
	}

	memset(&mc, 0, sizeof(megacmd_t));

	if( adapter->flag & BOARD_40LD ) {
		array_sz = sizeof(disk_array_40ld);

		rdrv_state = ((mega_inquiry3 *)inquiry)->ldrv_state;

		num_ldrv = ((mega_inquiry3 *)inquiry)->num_ldrv;
	}
	else {
		array_sz = sizeof(disk_array_8ld);

		rdrv_state = ((mraid_ext_inquiry *)inquiry)->
			raid_inq.logdrv_info.ldrv_state;

		num_ldrv = ((mraid_ext_inquiry *)inquiry)->
			raid_inq.logdrv_info.num_ldrv;
	}

	disk_array = pci_alloc_consistent(pdev, array_sz,
			&disk_array_dma_handle);

	if( disk_array == NULL ) {
		seq_puts(m, "memory not available.\n");
		goto free_inquiry;
	}

	mc.xferaddr = (u32)disk_array_dma_handle;

	if( adapter->flag & BOARD_40LD ) {
		mc.cmd = FC_NEW_CONFIG;
		mc.opcode = OP_DCMD_READ_CONFIG;

		if( mega_internal_command(adapter, &mc, NULL) ) {
			seq_puts(m, "40LD read config failed.\n");
			goto free_pci;
		}

	}
	else {
		mc.cmd = NEW_READ_CONFIG_8LD;

		if( mega_internal_command(adapter, &mc, NULL) ) {
			mc.cmd = READ_CONFIG_8LD;
			if( mega_internal_command(adapter, &mc, NULL) ) {
				seq_puts(m, "8LD read config failed.\n");
				goto free_pci;
			}
		}
	}

	for( i = start; i < ( (end+1 < num_ldrv) ? end+1 : num_ldrv ); i++ ) {

		if( adapter->flag & BOARD_40LD ) {
			lparam =
			&((disk_array_40ld *)disk_array)->ldrv[i].lparam;
		}
		else {
			lparam =
			&((disk_array_8ld *)disk_array)->ldrv[i].lparam;
		}

		/*
                                                              
                                          
   */
		seq_printf(m, "Logical drive:%2d:, ", i);

		switch( rdrv_state[i] & 0x0F ) {
		case RDRV_OFFLINE:
			seq_puts(m, "state: offline");
			break;
		case RDRV_DEGRADED:
			seq_puts(m, "state: degraded");
			break;
		case RDRV_OPTIMAL:
			seq_puts(m, "state: optimal");
			break;
		case RDRV_DELETED:
			seq_puts(m, "state: deleted");
			break;
		default:
			seq_puts(m, "state: unknown");
			break;
		}

		/*
                                                             
                            
   */
		if( (rdrv_state[i] & 0xF0) == 0x20 )
			seq_puts(m, ", check-consistency in progress");
		else if( (rdrv_state[i] & 0xF0) == 0x10 )
			seq_puts(m, ", initialization in progress");
		
		seq_putc(m, '\n');

		seq_printf(m, "Span depth:%3d, ", lparam->span_depth);
		seq_printf(m, "RAID level:%3d, ", lparam->level);
		seq_printf(m, "Stripe size:%3d, ",
			   lparam->stripe_sz ? lparam->stripe_sz/2: 128);
		seq_printf(m, "Row size:%3d\n", lparam->row_size);

		seq_puts(m, "Read Policy: ");
		switch(lparam->read_ahead) {
		case NO_READ_AHEAD:
			seq_puts(m, "No read ahead, ");
			break;
		case READ_AHEAD:
			seq_puts(m, "Read ahead, ");
			break;
		case ADAP_READ_AHEAD:
			seq_puts(m, "Adaptive, ");
			break;

		}

		seq_puts(m, "Write Policy: ");
		switch(lparam->write_mode) {
		case WRMODE_WRITE_THRU:
			seq_puts(m, "Write thru, ");
			break;
		case WRMODE_WRITE_BACK:
			seq_puts(m, "Write back, ");
			break;
		}

		seq_puts(m, "Cache Policy: ");
		switch(lparam->direct_io) {
		case CACHED_IO:
			seq_puts(m, "Cached IO\n\n");
			break;
		case DIRECT_IO:
			seq_puts(m, "Direct IO\n\n");
			break;
		}
	}

free_pci:
	pci_free_consistent(pdev, array_sz, disk_array,
			disk_array_dma_handle);
free_inquiry:
	mega_free_inquiry(inquiry, dma_handle, pdev);
free_pdev:
	free_local_pdev(pdev);
	return 0;
}

/* 
                      
                                        
                     
  
                                                                      
 */
static int
proc_show_rdrv_10(struct seq_file *m, void *v)
{
	return proc_show_rdrv(m, m->private, 0, 9);
}


/* 
                      
                                        
                     
  
                                                                      
 */
static int
proc_show_rdrv_20(struct seq_file *m, void *v)
{
	return proc_show_rdrv(m, m->private, 10, 19);
}


/* 
                      
                                        
                     
  
                                                                      
 */
static int
proc_show_rdrv_30(struct seq_file *m, void *v)
{
	return proc_show_rdrv(m, m->private, 20, 29);
}


/* 
                      
                                        
                     
  
                                                                      
 */
static int
proc_show_rdrv_40(struct seq_file *m, void *v)
{
	return proc_show_rdrv(m, m->private, 30, 39);
}


/*
                                                
 */
static int mega_proc_open(struct inode *inode, struct file *file)
{
	adapter_t *adapter = proc_get_parent_data(inode);
	int (*show)(struct seq_file *, void *) = PDE_DATA(inode);

	return single_open(file, show, adapter);
}

static const struct file_operations mega_proc_fops = {
	.open		= mega_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

/*
                                         
 */
struct mega_proc_file {
	const char *name;
	unsigned short ptr_offset;
	int (*show) (struct seq_file *m, void *v);
};

static const struct mega_proc_file mega_proc_files[] = {
	{ "config",	      offsetof(adapter_t, proc_read), proc_show_config },
	{ "stat",	      offsetof(adapter_t, proc_stat), proc_show_stat },
	{ "mailbox",	      offsetof(adapter_t, proc_mbox), proc_show_mbox },
#if MEGA_HAVE_ENH_PROC
	{ "rebuild-rate",     offsetof(adapter_t, proc_rr), proc_show_rebuild_rate },
	{ "battery-status",   offsetof(adapter_t, proc_battery), proc_show_battery },
	{ "diskdrives-ch0",   offsetof(adapter_t, proc_pdrvstat[0]), proc_show_pdrv_ch0 },
	{ "diskdrives-ch1",   offsetof(adapter_t, proc_pdrvstat[1]), proc_show_pdrv_ch1 },
	{ "diskdrives-ch2",   offsetof(adapter_t, proc_pdrvstat[2]), proc_show_pdrv_ch2 },
	{ "diskdrives-ch3",   offsetof(adapter_t, proc_pdrvstat[3]), proc_show_pdrv_ch3 },
	{ "raiddrives-0-9",   offsetof(adapter_t, proc_rdrvstat[0]), proc_show_rdrv_10 },
	{ "raiddrives-10-19", offsetof(adapter_t, proc_rdrvstat[1]), proc_show_rdrv_20 },
	{ "raiddrives-20-29", offsetof(adapter_t, proc_rdrvstat[2]), proc_show_rdrv_30 },
	{ "raiddrives-30-39", offsetof(adapter_t, proc_rdrvstat[3]), proc_show_rdrv_40 },
#endif
	{ NULL }
};

/* 
                           
                                     
                                             
  
                                             
 */
static void
mega_create_proc_entry(int index, struct proc_dir_entry *parent)
{
	const struct mega_proc_file *f;
	adapter_t	*adapter = hba_soft_state[index];
	struct proc_dir_entry	*dir, *de, **ppde;
	u8		string[16];

	sprintf(string, "hba%d", adapter->host->host_no);

	dir = adapter->controller_proc_dir_entry =
		proc_mkdir_data(string, 0, parent, adapter);
	if(!dir) {
		printk(KERN_WARNING "\nmegaraid: proc_mkdir failed\n");
		return;
	}

	for (f = mega_proc_files; f->name; f++) {
		de = proc_create_data(f->name, S_IRUSR, dir, &mega_proc_fops,
				      f->show);
		if (!de) {
			printk(KERN_WARNING "\nmegaraid: proc_create failed\n");
			return;
		}

		ppde = (void *)adapter + f->ptr_offset;
		*ppde = de;
	}
}

#else
static inline void mega_create_proc_entry(int index, struct proc_dir_entry *parent)
{
}
#endif


/* 
                       
  
                                                 
 */
static int
megaraid_biosparam(struct scsi_device *sdev, struct block_device *bdev,
		    sector_t capacity, int geom[])
{
	adapter_t	*adapter;
	unsigned char	*bh;
	int	heads;
	int	sectors;
	int	cylinders;
	int	rval;

	/*                                      */
	adapter = (adapter_t *)sdev->host->hostdata;

	if (IS_RAID_CH(adapter, sdev->channel)) {
			/*                                   */
			heads = 64;
			sectors = 32;
			cylinders = (ulong)capacity / (heads * sectors);

			/*
                                                         
           
    */
			if ((ulong)capacity >= 0x200000) {
				heads = 255;
				sectors = 63;
				cylinders = (ulong)capacity / (heads * sectors);
			}

			/*               */
			geom[0] = heads;
			geom[1] = sectors;
			geom[2] = cylinders;
	}
	else {
		bh = scsi_bios_ptable(bdev);

		if( bh ) {
			rval = scsi_partsize(bh, capacity,
					    &geom[2], &geom[0], &geom[1]);
			kfree(bh);
			if( rval != -1 )
				return rval;
		}

		printk(KERN_INFO
		"megaraid: invalid partition on this disk on channel %d\n",
				sdev->channel);

		/*                                   */
		heads = 64;
		sectors = 32;
		cylinders = (ulong)capacity / (heads * sectors);

		/*                                                           */
		if ((ulong)capacity >= 0x200000) {
			heads = 255;
			sectors = 63;
			cylinders = (ulong)capacity / (heads * sectors);
		}

		/*               */
		geom[0] = heads;
		geom[1] = sectors;
		geom[2] = cylinders;
	}

	return 0;
}

/* 
                  
                                       
  
                                                                  
                                                                        
            
 */
static int
mega_init_scb(adapter_t *adapter)
{
	scb_t	*scb;
	int	i;

	for( i = 0; i < adapter->max_cmds; i++ ) {

		scb = &adapter->scb_list[i];

		scb->sgl64 = NULL;
		scb->sgl = NULL;
		scb->pthru = NULL;
		scb->epthru = NULL;
	}

	for( i = 0; i < adapter->max_cmds; i++ ) {

		scb = &adapter->scb_list[i];

		scb->idx = i;

		scb->sgl64 = pci_alloc_consistent(adapter->dev,
				sizeof(mega_sgl64) * adapter->sglen,
				&scb->sgl_dma_addr);

		scb->sgl = (mega_sglist *)scb->sgl64;

		if( !scb->sgl ) {
			printk(KERN_WARNING "RAID: Can't allocate sglist.\n");
			mega_free_sgl(adapter);
			return -1;
		}

		scb->pthru = pci_alloc_consistent(adapter->dev,
				sizeof(mega_passthru),
				&scb->pthru_dma_addr);

		if( !scb->pthru ) {
			printk(KERN_WARNING "RAID: Can't allocate passthru.\n");
			mega_free_sgl(adapter);
			return -1;
		}

		scb->epthru = pci_alloc_consistent(adapter->dev,
				sizeof(mega_ext_passthru),
				&scb->epthru_dma_addr);

		if( !scb->epthru ) {
			printk(KERN_WARNING
				"Can't allocate extended passthru.\n");
			mega_free_sgl(adapter);
			return -1;
		}


		scb->dma_type = MEGA_DMA_TYPE_NONE;

		/*
                      
                                                             
                                 
   */
		scb->state = SCB_FREE;
		scb->cmd = NULL;
		list_add(&scb->list, &adapter->free_list);
	}

	return 0;
}


/* 
                 
                  
                  
  
                                                                             
                    
 */
static int
megadev_open (struct inode *inode, struct file *filep)
{
	/*
                                                          
  */
	if( !capable(CAP_SYS_ADMIN) ) return -EACCES;

	return 0;
}


/* 
                  
                            
                  
                       
                     
  
                                                                              
                                                                          
                                                                      
              
 */
static int
megadev_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
	adapter_t	*adapter;
	nitioctl_t	uioc;
	int		adapno;
	int		rval;
	mega_passthru	__user *upthru;	/*                           */
	mega_passthru	*pthru;		/*                         */
	dma_addr_t	pthru_dma_hndl;
	void		*data = NULL;	/*                        */
	dma_addr_t	data_dma_hndl;	/*                               */
	megacmd_t	mc;
	megastat_t	__user *ustats;
	int		num_ldrv;
	u32		uxferaddr = 0;
	struct pci_dev	*pdev;

	ustats = NULL; /*                            */
	num_ldrv = 0;

	/*
                                                              
                                                        
  */
	if( (_IOC_TYPE(cmd) != MEGAIOC_MAGIC) && (cmd != USCSICMD) ) {
		return -EINVAL;
	}

	/*
                                                             
                                                                   
                       
                                                                       
                                                                  
              
  */
	memset(&uioc, 0, sizeof(nitioctl_t));
	if( (rval = mega_m_to_n( (void __user *)arg, &uioc)) != 0 )
		return rval;


	switch( uioc.opcode ) {

	case GET_DRIVER_VER:
		if( put_user(driver_ver, (u32 __user *)uioc.uioc_uaddr) )
			return (-EFAULT);

		break;

	case GET_N_ADAP:
		if( put_user(hba_count, (u32 __user *)uioc.uioc_uaddr) )
			return (-EFAULT);

		/*
                                                               
                                                              
                                     
   */
		return hba_count;

	case GET_ADAP_INFO:

		/*
                  
   */
		if( (adapno = GETADAP(uioc.adapno)) >= hba_count )
			return (-ENODEV);

		if( copy_to_user(uioc.uioc_uaddr, mcontroller+adapno,
				sizeof(struct mcontroller)) )
			return (-EFAULT);
		break;

#if MEGA_HAVE_STATS

	case GET_STATS:
		/*
                  
   */
		if( (adapno = GETADAP(uioc.adapno)) >= hba_count )
			return (-ENODEV);

		adapter = hba_soft_state[adapno];

		ustats = uioc.uioc_uaddr;

		if( copy_from_user(&num_ldrv, &ustats->num_ldrv, sizeof(int)) )
			return (-EFAULT);

		/*
                                                       
   */
		if( num_ldrv >= MAX_LOGICAL_DRIVES_40LD ) return -EINVAL;

		if( copy_to_user(ustats->nreads, adapter->nreads,
					num_ldrv*sizeof(u32)) )
			return -EFAULT;

		if( copy_to_user(ustats->nreadblocks, adapter->nreadblocks,
					num_ldrv*sizeof(u32)) )
			return -EFAULT;

		if( copy_to_user(ustats->nwrites, adapter->nwrites,
					num_ldrv*sizeof(u32)) )
			return -EFAULT;

		if( copy_to_user(ustats->nwriteblocks, adapter->nwriteblocks,
					num_ldrv*sizeof(u32)) )
			return -EFAULT;

		if( copy_to_user(ustats->rd_errors, adapter->rd_errors,
					num_ldrv*sizeof(u32)) )
			return -EFAULT;

		if( copy_to_user(ustats->wr_errors, adapter->wr_errors,
					num_ldrv*sizeof(u32)) )
			return -EFAULT;

		return 0;

#endif
	case MBOX_CMD:

		/*
                  
   */
		if( (adapno = GETADAP(uioc.adapno)) >= hba_count )
			return (-ENODEV);

		adapter = hba_soft_state[adapno];

		/*
                                                             
                                                       
   */
		if( uioc.uioc_rmbox[0] == FC_DEL_LOGDRV &&
				uioc.uioc_rmbox[2] == OP_DEL_LOGDRV ) {

			/*
                                
    */
			if( !adapter->support_random_del ) {
				printk(KERN_WARNING "megaraid: logdrv ");
				printk("delete on non-supporting F/W.\n");

				return (-EINVAL);
			}

			rval = mega_del_logdrv( adapter, uioc.uioc_rmbox[3] );

			if( rval == 0 ) {
				memset(&mc, 0, sizeof(megacmd_t));

				mc.status = rval;

				rval = mega_n_to_m((void __user *)arg, &mc);
			}

			return rval;
		}
		/*
                                                               
                                                 
   */
		if( uioc.uioc_rmbox[0] == MEGA_MBOXCMD_PASSTHRU64 ||
			uioc.uioc_rmbox[0] == MEGA_MBOXCMD_EXTPTHRU ) {

			printk(KERN_WARNING "megaraid: rejected passthru.\n");

			return (-EINVAL);
		}

		/*
                                                               
                       
   */
		if( make_local_pdev(adapter, &pdev) != 0 )
			return -EIO;

		/*                                    */
		if( uioc.uioc_rmbox[0] == MEGA_MBOXCMD_PASSTHRU ) {
			/*                   */

			pthru = pci_alloc_consistent(pdev,
					sizeof(mega_passthru),
					&pthru_dma_hndl);

			if( pthru == NULL ) {
				free_local_pdev(pdev);
				return (-ENOMEM);
			}

			/*
                                 
    */
			upthru = (mega_passthru __user *)(unsigned long)MBOX(uioc)->xferaddr;

			/*
                                     
    */
			if( copy_from_user(pthru, upthru,
						sizeof(mega_passthru)) ) {

				pci_free_consistent(pdev,
						sizeof(mega_passthru), pthru,
						pthru_dma_hndl);

				free_local_pdev(pdev);

				return (-EFAULT);
			}

			/*
                              
    */
			if( pthru->dataxferlen ) {
				data = pci_alloc_consistent(pdev,
						pthru->dataxferlen,
						&data_dma_hndl);

				if( data == NULL ) {
					pci_free_consistent(pdev,
							sizeof(mega_passthru),
							pthru,
							pthru_dma_hndl);

					free_local_pdev(pdev);

					return (-ENOMEM);
				}

				/*
                                                 
                                       
     */
				uxferaddr = pthru->dataxferaddr;
				pthru->dataxferaddr = data_dma_hndl;
			}


			/*
                                
    */
			if( pthru->dataxferlen && (uioc.flags & UIOC_WR) ) {
				/*
                        
     */
				if( copy_from_user(data, (char __user *)(unsigned long) uxferaddr,
							pthru->dataxferlen) ) {
					rval = (-EFAULT);
					goto freemem_and_return;
				}
			}

			memset(&mc, 0, sizeof(megacmd_t));

			mc.cmd = MEGA_MBOXCMD_PASSTHRU;
			mc.xferaddr = (u32)pthru_dma_hndl;

			/*
                       
    */
			mega_internal_command(adapter, &mc, pthru);

			rval = mega_n_to_m((void __user *)arg, &mc);

			if( rval ) goto freemem_and_return;


			/*
                             
    */
			if( pthru->dataxferlen && (uioc.flags & UIOC_RD) ) {
				if( copy_to_user((char __user *)(unsigned long) uxferaddr, data,
							pthru->dataxferlen) ) {
					rval = (-EFAULT);
				}
			}

			/*
                                                       
                                               
    */
			if (copy_to_user(upthru->reqsensearea,
					pthru->reqsensearea, 14))
				rval = -EFAULT;

freemem_and_return:
			if( pthru->dataxferlen ) {
				pci_free_consistent(pdev,
						pthru->dataxferlen, data,
						data_dma_hndl);
			}

			pci_free_consistent(pdev, sizeof(mega_passthru),
					pthru, pthru_dma_hndl);

			free_local_pdev(pdev);

			return rval;
		}
		else {
			/*               */

			/*
                              
    */
			if( uioc.xferlen ) {
				data = pci_alloc_consistent(pdev,
						uioc.xferlen, &data_dma_hndl);

				if( data == NULL ) {
					free_local_pdev(pdev);
					return (-ENOMEM);
				}

				uxferaddr = MBOX(uioc)->xferaddr;
			}

			/*
                                
    */
			if( uioc.xferlen && (uioc.flags & UIOC_WR) ) {
				/*
                        
     */
				if( copy_from_user(data, (char __user *)(unsigned long) uxferaddr,
							uioc.xferlen) ) {

					pci_free_consistent(pdev,
							uioc.xferlen,
							data, data_dma_hndl);

					free_local_pdev(pdev);

					return (-EFAULT);
				}
			}

			memcpy(&mc, MBOX(uioc), sizeof(megacmd_t));

			mc.xferaddr = (u32)data_dma_hndl;

			/*
                       
    */
			mega_internal_command(adapter, &mc, NULL);

			rval = mega_n_to_m((void __user *)arg, &mc);

			if( rval ) {
				if( uioc.xferlen ) {
					pci_free_consistent(pdev,
							uioc.xferlen, data,
							data_dma_hndl);
				}

				free_local_pdev(pdev);

				return rval;
			}

			/*
                             
    */
			if( uioc.xferlen && (uioc.flags & UIOC_RD) ) {
				if( copy_to_user((char __user *)(unsigned long) uxferaddr, data,
							uioc.xferlen) ) {

					rval = (-EFAULT);
				}
			}

			if( uioc.xferlen ) {
				pci_free_consistent(pdev,
						uioc.xferlen, data,
						data_dma_hndl);
			}

			free_local_pdev(pdev);

			return rval;
		}

	default:
		return (-EINVAL);
	}

	return 0;
}

static long
megadev_unlocked_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
	int ret;

	mutex_lock(&megadev_mutex);
	ret = megadev_ioctl(filep, cmd, arg);
	mutex_unlock(&megadev_mutex);

	return ret;
}

/* 
                
                      
                              
  
                                                                            
            
  
                                                                 
 */
static int
mega_m_to_n(void __user *arg, nitioctl_t *uioc)
{
	struct uioctl_t	uioc_mimd;
	char	signature[8] = {0};
	u8	opcode;
	u8	subopcode;


	/*
                                                                       
                 
                                                                
                               
  */

	if( copy_from_user(signature, arg, 7) )
		return (-EFAULT);

	if( memcmp(signature, "MEGANIT", 7) == 0 ) {

		/*
                                                            
                                                              
                                                                
                                                      
   */
		return -EINVAL;
#if 0
		if( copy_from_user(uioc, arg, sizeof(nitioctl_t)) )
			return (-EFAULT);
		return 0;
#endif
	}

	/*
                                                                   
   
                                
  */
	if( copy_from_user(&uioc_mimd, arg, sizeof(struct uioctl_t)) )
		return (-EFAULT);


	/*
                                                 
  */
	opcode = uioc_mimd.ui.fcs.opcode;
	subopcode = uioc_mimd.ui.fcs.subopcode;

	switch (opcode) {
	case 0x82:

		switch (subopcode) {

		case MEGAIOC_QDRVRVER:	/*                      */
			uioc->opcode = GET_DRIVER_VER;
			uioc->uioc_uaddr = uioc_mimd.data;
			break;

		case MEGAIOC_QNADAP:	/*                   */
			uioc->opcode = GET_N_ADAP;
			uioc->uioc_uaddr = uioc_mimd.data;
			break;

		case MEGAIOC_QADAPINFO:	/*                         */
			uioc->opcode = GET_ADAP_INFO;
			uioc->adapno = uioc_mimd.ui.fcs.adapno;
			uioc->uioc_uaddr = uioc_mimd.data;
			break;

		default:
			return(-EINVAL);
		}

		break;


	case 0x81:

		uioc->opcode = MBOX_CMD;
		uioc->adapno = uioc_mimd.ui.fcs.adapno;

		memcpy(uioc->uioc_rmbox, uioc_mimd.mbox, 18);

		uioc->xferlen = uioc_mimd.ui.fcs.length;

		if( uioc_mimd.outlen ) uioc->flags = UIOC_RD;
		if( uioc_mimd.inlen ) uioc->flags |= UIOC_WR;

		break;

	case 0x80:

		uioc->opcode = MBOX_CMD;
		uioc->adapno = uioc_mimd.ui.fcs.adapno;

		memcpy(uioc->uioc_rmbox, uioc_mimd.mbox, 18);

		/*
                                                       
   */
		uioc->xferlen = uioc_mimd.outlen > uioc_mimd.inlen ?
			uioc_mimd.outlen : uioc_mimd.inlen;

		if( uioc_mimd.outlen ) uioc->flags = UIOC_RD;
		if( uioc_mimd.inlen ) uioc->flags |= UIOC_WR;

		break;

	default:
		return (-EINVAL);

	}

	return 0;
}

/*
                
                      
                        
  
                                                                              
                                                                      
 */
static int
mega_n_to_m(void __user *arg, megacmd_t *mc)
{
	nitioctl_t	__user *uiocp;
	megacmd_t	__user *umc;
	mega_passthru	__user *upthru;
	struct uioctl_t	__user *uioc_mimd;
	char	signature[8] = {0};

	/*
                                             
  */
	if( copy_from_user(signature, arg, 7) )
		return -EFAULT;

	if( memcmp(signature, "MEGANIT", 7) == 0 ) {

		uiocp = arg;

		if( put_user(mc->status, (u8 __user *)&MBOX_P(uiocp)->status) )
			return (-EFAULT);

		if( mc->cmd == MEGA_MBOXCMD_PASSTHRU ) {

			umc = MBOX_P(uiocp);

			if (get_user(upthru, (mega_passthru __user * __user *)&umc->xferaddr))
				return -EFAULT;

			if( put_user(mc->status, (u8 __user *)&upthru->scsistatus))
				return (-EFAULT);
		}
	}
	else {
		uioc_mimd = arg;

		if( put_user(mc->status, (u8 __user *)&uioc_mimd->mbox[17]) )
			return (-EFAULT);

		if( mc->cmd == MEGA_MBOXCMD_PASSTHRU ) {

			umc = (megacmd_t __user *)uioc_mimd->mbox;

			if (get_user(upthru, (mega_passthru __user * __user *)&umc->xferaddr))
				return (-EFAULT);

			if( put_user(mc->status, (u8 __user *)&upthru->scsistatus) )
				return (-EFAULT);
		}
	}

	return 0;
}


/*
                          
 */

/* 
                         
                                       
  
                                                                       
 */
static int
mega_is_bios_enabled(adapter_t *adapter)
{
	unsigned char	raw_mbox[sizeof(struct mbox_out)];
	mbox_t	*mbox;
	int	ret;

	mbox = (mbox_t *)raw_mbox;

	memset(&mbox->m_out, 0, sizeof(raw_mbox));

	memset((void *)adapter->mega_buffer, 0, MEGA_BUFFER_SIZE);

	mbox->m_out.xferaddr = (u32)adapter->buf_dma_handle;

	raw_mbox[0] = IS_BIOS_ENABLED;
	raw_mbox[2] = GET_BIOS;


	ret = issue_scb_block(adapter, raw_mbox);

	return *(char *)adapter->mega_buffer;
}


/* 
                        
                                       
  
                                                                    
                                                                          
                                     
 */
static void
mega_enum_raid_scsi(adapter_t *adapter)
{
	unsigned char raw_mbox[sizeof(struct mbox_out)];
	mbox_t *mbox;
	int i;

	mbox = (mbox_t *)raw_mbox;

	memset(&mbox->m_out, 0, sizeof(raw_mbox));

	/*
                                                         
  */
	raw_mbox[0] = CHNL_CLASS;
	raw_mbox[2] = GET_CHNL_CLASS;

	memset((void *)adapter->mega_buffer, 0, MEGA_BUFFER_SIZE);

	mbox->m_out.xferaddr = (u32)adapter->buf_dma_handle;

	/*
                                                        
                      
  */
	adapter->mega_ch_class = 0xFF;

	if(!issue_scb_block(adapter, raw_mbox)) {
		adapter->mega_ch_class = *((char *)adapter->mega_buffer);

	}

	for( i = 0; i < adapter->product_info.nchannels; i++ ) { 
		if( (adapter->mega_ch_class >> i) & 0x01 ) {
			printk(KERN_INFO "megaraid: channel[%d] is raid.\n",
					i);
		}
		else {
			printk(KERN_INFO "megaraid: channel[%d] is scsi.\n",
					i);
		}
	}

	return;
}


/* 
                      
                                       
  
                                                                           
                                                                     
 */
static void
mega_get_boot_drv(adapter_t *adapter)
{
	struct private_bios_data	*prv_bios_data;
	unsigned char	raw_mbox[sizeof(struct mbox_out)];
	mbox_t	*mbox;
	u16	cksum = 0;
	u8	*cksum_p;
	u8	boot_pdrv;
	int	i;

	mbox = (mbox_t *)raw_mbox;

	memset(&mbox->m_out, 0, sizeof(raw_mbox));

	raw_mbox[0] = BIOS_PVT_DATA;
	raw_mbox[2] = GET_BIOS_PVT_DATA;

	memset((void *)adapter->mega_buffer, 0, MEGA_BUFFER_SIZE);

	mbox->m_out.xferaddr = (u32)adapter->buf_dma_handle;

	adapter->boot_ldrv_enabled = 0;
	adapter->boot_ldrv = 0;

	adapter->boot_pdrv_enabled = 0;
	adapter->boot_pdrv_ch = 0;
	adapter->boot_pdrv_tgt = 0;

	if(issue_scb_block(adapter, raw_mbox) == 0) {
		prv_bios_data =
			(struct private_bios_data *)adapter->mega_buffer;

		cksum = 0;
		cksum_p = (char *)prv_bios_data;
		for (i = 0; i < 14; i++ ) {
			cksum += (u16)(*cksum_p++);
		}

		if (prv_bios_data->cksum == (u16)(0-cksum) ) {

			/*
                                                    
            
    */
			if( prv_bios_data->boot_drv & 0x80 ) {
				adapter->boot_pdrv_enabled = 1;
				boot_pdrv = prv_bios_data->boot_drv & 0x7F;
				adapter->boot_pdrv_ch = boot_pdrv / 16;
				adapter->boot_pdrv_tgt = boot_pdrv % 16;
			}
			else {
				adapter->boot_ldrv_enabled = 1;
				adapter->boot_ldrv = prv_bios_data->boot_drv;
			}
		}
	}

}

/* 
                            
                                       
  
                                                                       
                 
 */
static int
mega_support_random_del(adapter_t *adapter)
{
	unsigned char raw_mbox[sizeof(struct mbox_out)];
	mbox_t *mbox;
	int rval;

	mbox = (mbox_t *)raw_mbox;

	memset(&mbox->m_out, 0, sizeof(raw_mbox));

	/*
                 
  */
	raw_mbox[0] = FC_DEL_LOGDRV;
	raw_mbox[2] = OP_SUP_DEL_LOGDRV;

	rval = issue_scb_block(adapter, raw_mbox);

	return !rval;
}


/* 
                         
                                       
  
                                                
 */
static int
mega_support_ext_cdb(adapter_t *adapter)
{
	unsigned char raw_mbox[sizeof(struct mbox_out)];
	mbox_t *mbox;
	int rval;

	mbox = (mbox_t *)raw_mbox;

	memset(&mbox->m_out, 0, sizeof(raw_mbox));
	/*
                                                                   
  */
	raw_mbox[0] = 0xA4;
	raw_mbox[2] = 0x16;

	rval = issue_scb_block(adapter, raw_mbox);

	return !rval;
}


/* 
                    
                                       
                                        
  
                                                                           
                                               
 */
static int
mega_del_logdrv(adapter_t *adapter, int logdrv)
{
	unsigned long flags;
	scb_t *scb;
	int rval;

	/*
                                                                   
                                                        
  */
	atomic_set(&adapter->quiescent, 1);

	/*
                                                                   
                                 
  */
	while (atomic_read(&adapter->pend_cmds) > 0 ||
	       !list_empty(&adapter->pending_list))
		msleep(1000);	/*              */

	rval = mega_do_del_logdrv(adapter, logdrv);

	spin_lock_irqsave(&adapter->lock, flags);

	/*
                                                                     
                                          
  */
	if (adapter->read_ldidmap) {
		struct list_head *pos;
		list_for_each(pos, &adapter->pending_list) {
			scb = list_entry(pos, scb_t, list);
			if (scb->pthru->logdrv < 0x80 )
				scb->pthru->logdrv += 0x80;
		}
	}

	atomic_set(&adapter->quiescent, 0);

	mega_runpendq(adapter);

	spin_unlock_irqrestore(&adapter->lock, flags);

	return rval;
}


static int
mega_do_del_logdrv(adapter_t *adapter, int logdrv)
{
	megacmd_t	mc;
	int	rval;

	memset( &mc, 0, sizeof(megacmd_t));

	mc.cmd = FC_DEL_LOGDRV;
	mc.opcode = OP_DEL_LOGDRV;
	mc.subopcode = logdrv;

	rval = mega_internal_command(adapter, &mc, NULL);

	/*                */
	if(rval) {
		printk(KERN_WARNING "megaraid: Delete LD-%d failed.", logdrv);
		return rval;
	}

	/*
                                                                  
                                                     
  */
	adapter->read_ldidmap = 1;

	return rval;
}


/* 
                     
                                       
  
                                                                           
                          
 */
static void
mega_get_max_sgl(adapter_t *adapter)
{
	unsigned char	raw_mbox[sizeof(struct mbox_out)];
	mbox_t	*mbox;

	mbox = (mbox_t *)raw_mbox;

	memset(mbox, 0, sizeof(raw_mbox));

	memset((void *)adapter->mega_buffer, 0, MEGA_BUFFER_SIZE);

	mbox->m_out.xferaddr = (u32)adapter->buf_dma_handle;

	raw_mbox[0] = MAIN_MISC_OPCODE;
	raw_mbox[2] = GET_MAX_SG_SUPPORT;


	if( issue_scb_block(adapter, raw_mbox) ) {
		/*
                                                                
   */
		adapter->sglen = MIN_SGLIST;
	}
	else {
		adapter->sglen = *((char *)adapter->mega_buffer);
		
		/*
                                                         
                         
   */
		if ( adapter->sglen > MAX_SGLIST )
			adapter->sglen = MAX_SGLIST;
	}

	return;
}


/* 
                         
                                       
  
                                                   
 */
static int
mega_support_cluster(adapter_t *adapter)
{
	unsigned char	raw_mbox[sizeof(struct mbox_out)];
	mbox_t	*mbox;

	mbox = (mbox_t *)raw_mbox;

	memset(mbox, 0, sizeof(raw_mbox));

	memset((void *)adapter->mega_buffer, 0, MEGA_BUFFER_SIZE);

	mbox->m_out.xferaddr = (u32)adapter->buf_dma_handle;

	/*
                                                                  
                                        
  */
	raw_mbox[0] = MEGA_GET_TARGET_ID;

	if( issue_scb_block(adapter, raw_mbox) == 0 ) {

		/*
                                                            
                                  
   */
		adapter->this_id = *(u32 *)adapter->mega_buffer;
		adapter->host->this_id = adapter->this_id;

		return 1;
	}

	return 0;
}

#ifdef CONFIG_PROC_FS
/* 
                 
                                       
                                          
  
                                                          
                                                                        
                                                              
 */
static int
mega_adapinq(adapter_t *adapter, dma_addr_t dma_handle)
{
	megacmd_t	mc;

	memset(&mc, 0, sizeof(megacmd_t));

	if( adapter->flag & BOARD_40LD ) {
		mc.cmd = FC_NEW_CONFIG;
		mc.opcode = NC_SUBOP_ENQUIRY3;
		mc.subopcode = ENQ3_GET_SOLICITED_FULL;
	}
	else {
		mc.cmd = MEGA_MBOXCMD_ADPEXTINQ;
	}

	mc.xferaddr = (u32)dma_handle;

	if ( mega_internal_command(adapter, &mc, NULL) != 0 ) {
		return -1;
	}

	return 0;
}


/*                             
                                       
                                
                           
                                              
  
                                                   
 */
static int
mega_internal_dev_inquiry(adapter_t *adapter, u8 ch, u8 tgt,
		dma_addr_t buf_dma_handle)
{
	mega_passthru	*pthru;
	dma_addr_t	pthru_dma_handle;
	megacmd_t	mc;
	int		rval;
	struct pci_dev	*pdev;


	/*
                                                                   
                 
  */
	if( make_local_pdev(adapter, &pdev) != 0 ) return -1;

	pthru = pci_alloc_consistent(pdev, sizeof(mega_passthru),
			&pthru_dma_handle);

	if( pthru == NULL ) {
		free_local_pdev(pdev);
		return -1;
	}

	pthru->timeout = 2;
	pthru->ars = 1;
	pthru->reqsenselen = 14;
	pthru->islogical = 0;

	pthru->channel = (adapter->flag & BOARD_40LD) ? 0 : ch;

	pthru->target = (adapter->flag & BOARD_40LD) ? (ch << 4)|tgt : tgt;

	pthru->cdblen = 6;

	pthru->cdb[0] = INQUIRY;
	pthru->cdb[1] = 0;
	pthru->cdb[2] = 0;
	pthru->cdb[3] = 0;
	pthru->cdb[4] = 255;
	pthru->cdb[5] = 0;


	pthru->dataxferaddr = (u32)buf_dma_handle;
	pthru->dataxferlen = 256;

	memset(&mc, 0, sizeof(megacmd_t));

	mc.cmd = MEGA_MBOXCMD_PASSTHRU;
	mc.xferaddr = (u32)pthru_dma_handle;

	rval = mega_internal_command(adapter, &mc, pthru);

	pci_free_consistent(pdev, sizeof(mega_passthru), pthru,
			pthru_dma_handle);

	free_local_pdev(pdev);

	return rval;
}
#endif

/* 
                          
                                       
                            
                                                
  
                                                 
                                                                            
                                    
  
                                                                           
                                                                
  
                                                             
 */
static int
mega_internal_command(adapter_t *adapter, megacmd_t *mc, mega_passthru *pthru)
{
	Scsi_Cmnd	*scmd;
	struct	scsi_device *sdev;
	scb_t	*scb;
	int	rval;

	scmd = scsi_allocate_command(GFP_KERNEL);
	if (!scmd)
		return -ENOMEM;

	/*
                                                            
                                                                       
                                               
  */
	mutex_lock(&adapter->int_mtx);

	scb = &adapter->int_scb;
	memset(scb, 0, sizeof(scb_t));

	sdev = kzalloc(sizeof(struct scsi_device), GFP_KERNEL);
	scmd->device = sdev;

	memset(adapter->int_cdb, 0, sizeof(adapter->int_cdb));
	scmd->cmnd = adapter->int_cdb;
	scmd->device->host = adapter->host;
	scmd->host_scribble = (void *)scb;
	scmd->cmnd[0] = MEGA_INTERNAL_CMD;

	scb->state |= SCB_ACTIVE;
	scb->cmd = scmd;

	memcpy(scb->raw_mbox, mc, sizeof(megacmd_t));

	/*
                            
  */
	if( mc->cmd == MEGA_MBOXCMD_PASSTHRU ) {

		scb->pthru = pthru;
	}

	scb->idx = CMDID_INT_CMDS;

	megaraid_queue_lck(scmd, mega_internal_done);

	wait_for_completion(&adapter->int_waitq);

	rval = scmd->result;
	mc->status = scmd->result;
	kfree(sdev);

	/*
                                                                       
                     
  */
	if( scmd->result && trace_level ) {
		printk("megaraid: cmd [%x, %x, %x] status:[%x]\n",
			mc->cmd, mc->opcode, mc->subopcode, scmd->result);
	}

	mutex_unlock(&adapter->int_mtx);

	scsi_free_command(GFP_KERNEL, scmd);

	return rval;
}


/* 
                       
                                
  
                                          
 */
static void
mega_internal_done(Scsi_Cmnd *scmd)
{
	adapter_t	*adapter;

	adapter = (adapter_t *)scmd->device->host->hostdata;

	complete(&adapter->int_waitq);

}


static struct scsi_host_template megaraid_template = {
	.module				= THIS_MODULE,
	.name				= "MegaRAID",
	.proc_name			= "megaraid_legacy",
	.info				= megaraid_info,
	.queuecommand			= megaraid_queue,	
	.bios_param			= megaraid_biosparam,
	.max_sectors			= MAX_SECTORS_PER_IO,
	.can_queue			= MAX_COMMANDS,
	.this_id			= DEFAULT_INITIATOR_ID,
	.sg_tablesize			= MAX_SGLIST,
	.cmd_per_lun			= DEF_CMD_PER_LUN,
	.use_clustering			= ENABLE_CLUSTERING,
	.eh_abort_handler		= megaraid_abort,
	.eh_device_reset_handler	= megaraid_reset,
	.eh_bus_reset_handler		= megaraid_reset,
	.eh_host_reset_handler		= megaraid_reset,
	.no_write_same			= 1,
};

static int
megaraid_probe_one(struct pci_dev *pdev, const struct pci_device_id *id)
{
	struct Scsi_Host *host;
	adapter_t *adapter;
	unsigned long mega_baseport, tbase, flag = 0;
	u16 subsysid, subsysvid;
	u8 pci_bus, pci_dev_func;
	int irq, i, j;
	int error = -ENODEV;

	if (pci_enable_device(pdev))
		goto out;
	pci_set_master(pdev);

	pci_bus = pdev->bus->number;
	pci_dev_func = pdev->devfn;

	/*
                                                                     
                                     
  */
	if (pdev->vendor == PCI_VENDOR_ID_INTEL) {
		u16 magic;
		/*
                                                               
                   
   */
		if (pdev->subsystem_vendor == PCI_VENDOR_ID_COMPAQ &&
		    pdev->subsystem_device == 0xC000)
		   	return -ENODEV;
		/*                                    */
		pci_read_config_word(pdev, PCI_CONF_AMISIG, &magic);
		if (magic != HBA_SIGNATURE_471 && magic != HBA_SIGNATURE)
			return -ENODEV;
		/*                              */
	}

	/*
                                                              
                                
  */
	if (id->driver_data & BOARD_64BIT)
		flag |= BOARD_64BIT;
	else {
		u32 magic64;

		pci_read_config_dword(pdev, PCI_CONF_AMISIG64, &magic64);
		if (magic64 == HBA_SIGNATURE_64BIT)
			flag |= BOARD_64BIT;
	}

	subsysvid = pdev->subsystem_vendor;
	subsysid = pdev->subsystem_device;

	printk(KERN_NOTICE "megaraid: found 0x%4.04x:0x%4.04x:bus %d:",
		id->vendor, id->device, pci_bus);

	printk("slot %d:func %d\n",
		PCI_SLOT(pci_dev_func), PCI_FUNC(pci_dev_func));

	/*                                     */
	mega_baseport = pci_resource_start(pdev, 0);
	irq = pdev->irq;

	tbase = mega_baseport;
	if (pci_resource_flags(pdev, 0) & IORESOURCE_MEM) {
		flag |= BOARD_MEMMAP;

		if (!request_mem_region(mega_baseport, 128, "megaraid")) {
			printk(KERN_WARNING "megaraid: mem region busy!\n");
			goto out_disable_device;
		}

		mega_baseport = (unsigned long)ioremap(mega_baseport, 128);
		if (!mega_baseport) {
			printk(KERN_WARNING
			       "megaraid: could not map hba memory\n");
			goto out_release_region;
		}
	} else {
		flag |= BOARD_IOMAP;
		mega_baseport += 0x10;

		if (!request_region(mega_baseport, 16, "megaraid"))
			goto out_disable_device;
	}

	/*                                */
	host = scsi_host_alloc(&megaraid_template, sizeof(adapter_t));
	if (!host)
		goto out_iounmap;

	adapter = (adapter_t *)host->hostdata;
	memset(adapter, 0, sizeof(adapter_t));

	printk(KERN_NOTICE
		"scsi%d:Found MegaRAID controller at 0x%lx, IRQ:%d\n",
		host->host_no, mega_baseport, irq);

	adapter->base = mega_baseport;
	if (flag & BOARD_MEMMAP)
		adapter->mmio_base = (void __iomem *) mega_baseport;

	INIT_LIST_HEAD(&adapter->free_list);
	INIT_LIST_HEAD(&adapter->pending_list);
	INIT_LIST_HEAD(&adapter->completed_list);

	adapter->flag = flag;
	spin_lock_init(&adapter->lock);

	host->cmd_per_lun = max_cmd_per_lun;
	host->max_sectors = max_sectors_per_io;

	adapter->dev = pdev;
	adapter->host = host;

	adapter->host->irq = irq;

	if (flag & BOARD_MEMMAP)
		adapter->host->base = tbase;
	else {
		adapter->host->io_port = tbase;
		adapter->host->n_io_port = 16;
	}

	adapter->host->unique_id = (pci_bus << 8) | pci_dev_func;

	/*
                                               
  */
	adapter->mega_buffer = pci_alloc_consistent(adapter->dev,
		MEGA_BUFFER_SIZE, &adapter->buf_dma_handle);
	if (!adapter->mega_buffer) {
		printk(KERN_WARNING "megaraid: out of RAM.\n");
		goto out_host_put;
	}

	adapter->scb_list = kmalloc(sizeof(scb_t) * MAX_COMMANDS, GFP_KERNEL);
	if (!adapter->scb_list) {
		printk(KERN_WARNING "megaraid: out of RAM.\n");
		goto out_free_cmd_buffer;
	}

	if (request_irq(irq, (adapter->flag & BOARD_MEMMAP) ?
				megaraid_isr_memmapped : megaraid_isr_iomapped,
					IRQF_SHARED, "megaraid", adapter)) {
		printk(KERN_WARNING
			"megaraid: Couldn't register IRQ %d!\n", irq);
		goto out_free_scb_list;
	}

	if (mega_setup_mailbox(adapter))
		goto out_free_irq;

	if (mega_query_adapter(adapter))
		goto out_free_mbox;

	/*
                                  
  */
	if ((subsysid == 0x1111) && (subsysvid == 0x1111)) {
		/*
                   
   */
		if (!strcmp(adapter->fw_version, "3.00") ||
				!strcmp(adapter->fw_version, "3.01")) {

			printk( KERN_WARNING
				"megaraid: Your  card is a Dell PERC "
				"2/SC RAID controller with  "
				"firmware\nmegaraid: 3.00 or 3.01.  "
				"This driver is known to have "
				"corruption issues\nmegaraid: with "
				"those firmware versions on this "
				"specific card.  In order\nmegaraid: "
				"to protect your data, please upgrade "
				"your firmware to version\nmegaraid: "
				"3.10 or later, available from the "
				"Dell Technical Support web\n"
				"megaraid: site at\nhttp://support."
				"dell.com/us/en/filelib/download/"
				"index.asp?fileid=2940\n"
			);
		}
	}

	/*
                                                         
                                                         
                                                     
              
  */
	if ((subsysvid == PCI_VENDOR_ID_HP) &&
	    ((subsysid == 0x60E7) || (subsysid == 0x60E8))) {
		/*
                   
   */
		if (!strcmp(adapter->fw_version, "H01.07") ||
		    !strcmp(adapter->fw_version, "H01.08") ||
		    !strcmp(adapter->fw_version, "H01.09") ) {
			printk(KERN_WARNING
				"megaraid: Firmware H.01.07, "
				"H.01.08, and H.01.09 on 1M/2M "
				"controllers\n"
				"megaraid: do not support 64 bit "
				"addressing.\nmegaraid: DISABLING "
				"64 bit support.\n");
			adapter->flag &= ~BOARD_64BIT;
		}
	}

	if (mega_is_bios_enabled(adapter))
		mega_hbas[hba_count].is_bios_enabled = 1;
	mega_hbas[hba_count].hostdata_addr = adapter;

	/*
                                                             
                     
  */
	mega_enum_raid_scsi(adapter);

	/*
                                                            
                                                            
                                                            
                                                              
                                                              
                               
  */
	mega_get_boot_drv(adapter);

	if (adapter->boot_pdrv_enabled) {
		j = adapter->product_info.nchannels;
		for( i = 0; i < j; i++ )
			adapter->logdrv_chan[i] = 0;
		for( i = j; i < NVIRT_CHAN + j; i++ )
			adapter->logdrv_chan[i] = 1;
	} else {
		for (i = 0; i < NVIRT_CHAN; i++)
			adapter->logdrv_chan[i] = 1;
		for (i = NVIRT_CHAN; i < MAX_CHANNELS+NVIRT_CHAN; i++)
			adapter->logdrv_chan[i] = 0;
		adapter->mega_ch_class <<= NVIRT_CHAN;
	}

	/*
                                                         
          
  */
	adapter->read_ldidmap = 0;	/*                          
                    */
	adapter->support_random_del = mega_support_random_del(adapter);

	/*                 */
	if (mega_init_scb(adapter))
		goto out_free_mbox;

	/*
                                      
  */
	atomic_set(&adapter->pend_cmds, 0);

	/*
                                    
  */
	atomic_set(&adapter->quiescent, 0);

	hba_soft_state[hba_count] = adapter;

	/*
                                                              
                                                              
                
  */
	i = hba_count;

	mcontroller[i].base = mega_baseport;
	mcontroller[i].irq = irq;
	mcontroller[i].numldrv = adapter->numldrv;
	mcontroller[i].pcibus = pci_bus;
	mcontroller[i].pcidev = id->device;
	mcontroller[i].pcifun = PCI_FUNC (pci_dev_func);
	mcontroller[i].pciid = -1;
	mcontroller[i].pcivendor = id->vendor;
	mcontroller[i].pcislot = PCI_SLOT(pci_dev_func);
	mcontroller[i].uid = (pci_bus << 8) | pci_dev_func;


	/*                                                */
	if ((adapter->flag & BOARD_64BIT) && (sizeof(dma_addr_t) == 8)) {
		pci_set_dma_mask(pdev, DMA_BIT_MASK(64));
		adapter->has_64bit_addr = 1;
	} else  {
		pci_set_dma_mask(pdev, DMA_BIT_MASK(32));
		adapter->has_64bit_addr = 0;
	}
		
	mutex_init(&adapter->int_mtx);
	init_completion(&adapter->int_waitq);

	adapter->this_id = DEFAULT_INITIATOR_ID;
	adapter->host->this_id = DEFAULT_INITIATOR_ID;

#if MEGA_HAVE_CLUSTERING
	/*
                                                 
                                                            
                                                              
                                                             
                                    
  */
	adapter->has_cluster = mega_support_cluster(adapter);
	if (adapter->has_cluster) {
		printk(KERN_NOTICE
			"megaraid: Cluster driver, initiator id:%d\n",
			adapter->this_id);
	}
#endif

	pci_set_drvdata(pdev, host);

	mega_create_proc_entry(hba_count, mega_proc_dir_entry);

	error = scsi_add_host(host, &pdev->dev);
	if (error)
		goto out_free_mbox;

	scsi_scan_host(host);
	hba_count++;
	return 0;

 out_free_mbox:
	pci_free_consistent(adapter->dev, sizeof(mbox64_t),
			adapter->una_mbox64, adapter->una_mbox64_dma);
 out_free_irq:
	free_irq(adapter->host->irq, adapter);
 out_free_scb_list:
	kfree(adapter->scb_list);
 out_free_cmd_buffer:
	pci_free_consistent(adapter->dev, MEGA_BUFFER_SIZE,
			adapter->mega_buffer, adapter->buf_dma_handle);
 out_host_put:
	scsi_host_put(host);
 out_iounmap:
	if (flag & BOARD_MEMMAP)
		iounmap((void *)mega_baseport);
 out_release_region:
	if (flag & BOARD_MEMMAP)
		release_mem_region(tbase, 128);
	else
		release_region(mega_baseport, 16);
 out_disable_device:
	pci_disable_device(pdev);
 out:
	return error;
}

static void
__megaraid_shutdown(adapter_t *adapter)
{
	u_char	raw_mbox[sizeof(struct mbox_out)];
	mbox_t	*mbox = (mbox_t *)raw_mbox;
	int	i;

	/*                     */
	memset(&mbox->m_out, 0, sizeof(raw_mbox));
	raw_mbox[0] = FLUSH_ADAPTER;

	free_irq(adapter->host->irq, adapter);

	/*                                                            */
	issue_scb_block(adapter, raw_mbox);

	/*                   */
	memset(&mbox->m_out, 0, sizeof(raw_mbox));
	raw_mbox[0] = FLUSH_SYSTEM;

	/*                                                            */
	issue_scb_block(adapter, raw_mbox);
	
	if (atomic_read(&adapter->pend_cmds) > 0)
		printk(KERN_WARNING "megaraid: pending commands!!\n");

	/*
                                                          
                     
  */
	for (i = 0; i <= 10; i++)
		mdelay(1000);
}

static void
megaraid_remove_one(struct pci_dev *pdev)
{
	struct Scsi_Host *host = pci_get_drvdata(pdev);
	adapter_t *adapter = (adapter_t *)host->hostdata;

	scsi_remove_host(host);

	__megaraid_shutdown(adapter);

	/*                    */
	if (adapter->flag & BOARD_MEMMAP) {
		iounmap((void *)adapter->base);
		release_mem_region(adapter->host->base, 128);
	} else
		release_region(adapter->base, 16);

	mega_free_sgl(adapter);

#ifdef CONFIG_PROC_FS
	if (adapter->controller_proc_dir_entry) {
		remove_proc_entry("stat", adapter->controller_proc_dir_entry);
		remove_proc_entry("config",
				adapter->controller_proc_dir_entry);
		remove_proc_entry("mailbox",
				adapter->controller_proc_dir_entry);
#if MEGA_HAVE_ENH_PROC
		remove_proc_entry("rebuild-rate",
				adapter->controller_proc_dir_entry);
		remove_proc_entry("battery-status",
				adapter->controller_proc_dir_entry);

		remove_proc_entry("diskdrives-ch0",
				adapter->controller_proc_dir_entry);
		remove_proc_entry("diskdrives-ch1",
				adapter->controller_proc_dir_entry);
		remove_proc_entry("diskdrives-ch2",
				adapter->controller_proc_dir_entry);
		remove_proc_entry("diskdrives-ch3",
				adapter->controller_proc_dir_entry);

		remove_proc_entry("raiddrives-0-9",
				adapter->controller_proc_dir_entry);
		remove_proc_entry("raiddrives-10-19",
				adapter->controller_proc_dir_entry);
		remove_proc_entry("raiddrives-20-29",
				adapter->controller_proc_dir_entry);
		remove_proc_entry("raiddrives-30-39",
				adapter->controller_proc_dir_entry);
#endif
		{
			char	buf[12] = { 0 };
			sprintf(buf, "hba%d", adapter->host->host_no);
			remove_proc_entry(buf, mega_proc_dir_entry);
		}
	}
#endif

	pci_free_consistent(adapter->dev, MEGA_BUFFER_SIZE,
			adapter->mega_buffer, adapter->buf_dma_handle);
	kfree(adapter->scb_list);
	pci_free_consistent(adapter->dev, sizeof(mbox64_t),
			adapter->una_mbox64, adapter->una_mbox64_dma);

	scsi_host_put(host);
	pci_disable_device(pdev);

	hba_count--;
}

static void
megaraid_shutdown(struct pci_dev *pdev)
{
	struct Scsi_Host *host = pci_get_drvdata(pdev);
	adapter_t *adapter = (adapter_t *)host->hostdata;

	__megaraid_shutdown(adapter);
}

static struct pci_device_id megaraid_pci_tbl[] = {
	{PCI_VENDOR_ID_AMI, PCI_DEVICE_ID_AMI_MEGARAID,
		PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
	{PCI_VENDOR_ID_AMI, PCI_DEVICE_ID_AMI_MEGARAID2,
		PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
	{PCI_VENDOR_ID_INTEL, PCI_DEVICE_ID_AMI_MEGARAID3,
		PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
	{0,}
};
MODULE_DEVICE_TABLE(pci, megaraid_pci_tbl);

static struct pci_driver megaraid_pci_driver = {
	.name		= "megaraid_legacy",
	.id_table	= megaraid_pci_tbl,
	.probe		= megaraid_probe_one,
	.remove		= megaraid_remove_one,
	.shutdown	= megaraid_shutdown,
};

static int __init megaraid_init(void)
{
	int error;

	if ((max_cmd_per_lun <= 0) || (max_cmd_per_lun > MAX_CMD_PER_LUN))
		max_cmd_per_lun = MAX_CMD_PER_LUN;
	if (max_mbox_busy_wait > MBOX_BUSY_WAIT)
		max_mbox_busy_wait = MBOX_BUSY_WAIT;

#ifdef CONFIG_PROC_FS
	mega_proc_dir_entry = proc_mkdir("megaraid", NULL);
	if (!mega_proc_dir_entry) {
		printk(KERN_WARNING
				"megaraid: failed to create megaraid root\n");
	}
#endif
	error = pci_register_driver(&megaraid_pci_driver);
	if (error) {
#ifdef CONFIG_PROC_FS
		remove_proc_entry("megaraid", NULL);
#endif
		return error;
	}

	/*
                                                               
                            
                                                               
                            
  */
	major = register_chrdev(0, "megadev_legacy", &megadev_fops);
	if (!major) {
		printk(KERN_WARNING
				"megaraid: failed to register char device\n");
	}

	return 0;
}

static void __exit megaraid_exit(void)
{
	/*
                                                            
  */
	unregister_chrdev(major, "megadev_legacy");

	pci_unregister_driver(&megaraid_pci_driver);

#ifdef CONFIG_PROC_FS
	remove_proc_entry("megaraid", NULL);
#endif
}

module_init(megaraid_init);
module_exit(megaraid_exit);

/*                          */
