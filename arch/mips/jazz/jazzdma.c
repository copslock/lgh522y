/*
 * Mips Jazz DMA controller support
 * Copyright (C) 1995, 1996 by Andreas Busse
 *
 * NOTE: Some of the argument checking could be removed when
 * things have settled down. Also, instead of returning 0xffffffff
 * on failure of vdma_alloc() one could leave page #0 unused
 * and return the more usual NULL pointer as logical address.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/bootmem.h>
#include <linux/spinlock.h>
#include <linux/gfp.h>
#include <asm/mipsregs.h>
#include <asm/jazz.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/dma.h>
#include <asm/jazzdma.h>
#include <asm/pgtable.h>

/*
                                                        
 */
#define CONF_DEBUG_VDMA 0

static VDMA_PGTBL_ENTRY *pgtbl;

static DEFINE_SPINLOCK(vdma_lock);

/*
              
 */
#define vdma_debug     ((CONF_DEBUG_VDMA) ? debuglvl : 0)

static int debuglvl = 3;

/*
                                                        
                                                     
                                                        
                                                     
 */
static inline void vdma_pgtbl_init(void)
{
	unsigned long paddr = 0;
	int i;

	for (i = 0; i < VDMA_PGTBL_ENTRIES; i++) {
		pgtbl[i].frame = paddr;
		pgtbl[i].owner = VDMA_PAGE_EMPTY;
		paddr += VDMA_PAGESIZE;
	}
}

/*
                                           
 */
static int __init vdma_init(void)
{
	/*
                                                                     
                                                                      
           
  */
	pgtbl = (VDMA_PGTBL_ENTRY *)__get_free_pages(GFP_KERNEL | GFP_DMA,
						    get_order(VDMA_PGTBL_SIZE));
	BUG_ON(!pgtbl);
	dma_cache_wback_inv((unsigned long)pgtbl, VDMA_PGTBL_SIZE);
	pgtbl = (VDMA_PGTBL_ENTRY *)KSEG1ADDR(pgtbl);

	/*
                                     
  */
	vdma_pgtbl_init();

	r4030_write_reg32(JAZZ_R4030_TRSTBL_BASE, CPHYSADDR(pgtbl));
	r4030_write_reg32(JAZZ_R4030_TRSTBL_LIM, VDMA_PGTBL_SIZE);
	r4030_write_reg32(JAZZ_R4030_TRSTBL_INV, 0);

	printk(KERN_INFO "VDMA: R4030 DMA pagetables initialized.\n");
	return 0;
}

/*
                                                             
 */
unsigned long vdma_alloc(unsigned long paddr, unsigned long size)
{
	int first, last, pages, frame, i;
	unsigned long laddr, flags;

	/*                 */

	if (paddr > 0x1fffffff) {
		if (vdma_debug)
			printk("vdma_alloc: Invalid physical address: %08lx\n",
			       paddr);
		return VDMA_ERROR;	/*                          */
	}
	if (size > 0x400000 || size == 0) {
		if (vdma_debug)
			printk("vdma_alloc: Invalid size: %08lx\n", size);
		return VDMA_ERROR;	/*                          */
	}

	spin_lock_irqsave(&vdma_lock, flags);
	/*
                   
  */
	pages = VDMA_PAGE(paddr + size) - VDMA_PAGE(paddr) + 1;
	first = 0;
	while (1) {
		while (pgtbl[first].owner != VDMA_PAGE_EMPTY &&
		       first < VDMA_PGTBL_ENTRIES) first++;
		if (first + pages > VDMA_PGTBL_ENTRIES) {	/*              */
			spin_unlock_irqrestore(&vdma_lock, flags);
			return VDMA_ERROR;
		}

		last = first + 1;
		while (pgtbl[last].owner == VDMA_PAGE_EMPTY
		       && last - first < pages)
			last++;

		if (last - first == pages)
			break;	/*       */
		first = last + 1;
	}

	/*
                           
  */
	laddr = (first << 12) + (paddr & (VDMA_PAGESIZE - 1));
	frame = paddr & ~(VDMA_PAGESIZE - 1);

	for (i = first; i < last; i++) {
		pgtbl[i].frame = frame;
		pgtbl[i].owner = laddr;
		frame += VDMA_PAGESIZE;
	}

	/*
                                                             
  */
	r4030_write_reg32(JAZZ_R4030_TRSTBL_INV, 0);

	if (vdma_debug > 1)
		printk("vdma_alloc: Allocated %d pages starting from %08lx\n",
		     pages, laddr);

	if (vdma_debug > 2) {
		printk("LADDR: ");
		for (i = first; i < last; i++)
			printk("%08x ", i << 12);
		printk("\nPADDR: ");
		for (i = first; i < last; i++)
			printk("%08x ", pgtbl[i].frame);
		printk("\nOWNER: ");
		for (i = first; i < last; i++)
			printk("%08x ", pgtbl[i].owner);
		printk("\n");
	}

	spin_unlock_irqrestore(&vdma_lock, flags);

	return laddr;
}

EXPORT_SYMBOL(vdma_alloc);

/*
                                                  
                                                        
                                            
 */
int vdma_free(unsigned long laddr)
{
	int i;

	i = laddr >> 12;

	if (pgtbl[i].owner != laddr) {
		printk
		    ("vdma_free: trying to free other's dma pages, laddr=%8lx\n",
		     laddr);
		return -1;
	}

	while (i < VDMA_PGTBL_ENTRIES && pgtbl[i].owner == laddr) {
		pgtbl[i].owner = VDMA_PAGE_EMPTY;
		i++;
	}

	if (vdma_debug > 1)
		printk("vdma_free: freed %ld pages starting from %08lx\n",
		       i - (laddr >> 12), laddr);

	return 0;
}

EXPORT_SYMBOL(vdma_free);

/*
                                                   
                                                 
 */
int vdma_remap(unsigned long laddr, unsigned long paddr, unsigned long size)
{
	int first, pages;

	if (laddr > 0xffffff) {
		if (vdma_debug)
			printk
			    ("vdma_map: Invalid logical address: %08lx\n",
			     laddr);
		return -EINVAL; /*                         */
	}
	if (paddr > 0x1fffffff) {
		if (vdma_debug)
			printk
			    ("vdma_map: Invalid physical address: %08lx\n",
			     paddr);
		return -EINVAL; /*                          */
	}

	pages = (((paddr & (VDMA_PAGESIZE - 1)) + size) >> 12) + 1;
	first = laddr >> 12;
	if (vdma_debug)
		printk("vdma_remap: first=%x, pages=%x\n", first, pages);
	if (first + pages > VDMA_PGTBL_ENTRIES) {
		if (vdma_debug)
			printk("vdma_alloc: Invalid size: %08lx\n", size);
		return -EINVAL;
	}

	paddr &= ~(VDMA_PAGESIZE - 1);
	while (pages > 0 && first < VDMA_PGTBL_ENTRIES) {
		if (pgtbl[first].owner != laddr) {
			if (vdma_debug)
				printk("Trying to remap other's pages.\n");
			return -EPERM;	/*           */
		}
		pgtbl[first].frame = paddr;
		paddr += VDMA_PAGESIZE;
		first++;
		pages--;
	}

	/*
                            
  */
	r4030_write_reg32(JAZZ_R4030_TRSTBL_INV, 0);

	if (vdma_debug > 2) {
		int i;
		pages = (((paddr & (VDMA_PAGESIZE - 1)) + size) >> 12) + 1;
		first = laddr >> 12;
		printk("LADDR: ");
		for (i = first; i < first + pages; i++)
			printk("%08x ", i << 12);
		printk("\nPADDR: ");
		for (i = first; i < first + pages; i++)
			printk("%08x ", pgtbl[i].frame);
		printk("\nOWNER: ");
		for (i = first; i < first + pages; i++)
			printk("%08x ", pgtbl[i].owner);
		printk("\n");
	}

	return 0;
}

/*
                                                     
                                                    
         
 */
unsigned long vdma_phys2log(unsigned long paddr)
{
	int i;
	int frame;

	frame = paddr & ~(VDMA_PAGESIZE - 1);

	for (i = 0; i < VDMA_PGTBL_ENTRIES; i++) {
		if (pgtbl[i].frame == frame)
			break;
	}

	if (i == VDMA_PGTBL_ENTRIES)
		return ~0UL;

	return (i << 12) + (paddr & (VDMA_PAGESIZE - 1));
}

EXPORT_SYMBOL(vdma_phys2log);

/*
                                                        
 */
unsigned long vdma_log2phys(unsigned long laddr)
{
	return pgtbl[laddr >> 12].frame + (laddr & (VDMA_PAGESIZE - 1));
}

EXPORT_SYMBOL(vdma_log2phys);

/*
                       
 */
void vdma_stats(void)
{
	int i;

	printk("vdma_stats: CONFIG: %08x\n",
	       r4030_read_reg32(JAZZ_R4030_CONFIG));
	printk("R4030 translation table base: %08x\n",
	       r4030_read_reg32(JAZZ_R4030_TRSTBL_BASE));
	printk("R4030 translation table limit: %08x\n",
	       r4030_read_reg32(JAZZ_R4030_TRSTBL_LIM));
	printk("vdma_stats: INV_ADDR: %08x\n",
	       r4030_read_reg32(JAZZ_R4030_INV_ADDR));
	printk("vdma_stats: R_FAIL_ADDR: %08x\n",
	       r4030_read_reg32(JAZZ_R4030_R_FAIL_ADDR));
	printk("vdma_stats: M_FAIL_ADDR: %08x\n",
	       r4030_read_reg32(JAZZ_R4030_M_FAIL_ADDR));
	printk("vdma_stats: IRQ_SOURCE: %08x\n",
	       r4030_read_reg32(JAZZ_R4030_IRQ_SOURCE));
	printk("vdma_stats: I386_ERROR: %08x\n",
	       r4030_read_reg32(JAZZ_R4030_I386_ERROR));
	printk("vdma_chnl_modes:   ");
	for (i = 0; i < 8; i++)
		printk("%04x ",
		       (unsigned) r4030_read_reg32(JAZZ_R4030_CHNL_MODE +
						   (i << 5)));
	printk("\n");
	printk("vdma_chnl_enables: ");
	for (i = 0; i < 8; i++)
		printk("%04x ",
		       (unsigned) r4030_read_reg32(JAZZ_R4030_CHNL_ENABLE +
						   (i << 5)));
	printk("\n");
}

/*
                         
 */

/*
                                                         
 */
void vdma_enable(int channel)
{
	int status;

	if (vdma_debug)
		printk("vdma_enable: channel %d\n", channel);

	/*
                                
  */
	status = r4030_read_reg32(JAZZ_R4030_CHNL_ENABLE + (channel << 5));
	if (status & 0x400)
		printk("VDMA: Channel %d: Address error!\n", channel);
	if (status & 0x200)
		printk("VDMA: Channel %d: Memory error!\n", channel);

	/*
                             
  */
	r4030_write_reg32(JAZZ_R4030_CHNL_ENABLE + (channel << 5),
			  r4030_read_reg32(JAZZ_R4030_CHNL_ENABLE +
					   (channel << 5)) | R4030_TC_INTR
			  | R4030_MEM_INTR | R4030_ADDR_INTR);

	/*
                              
  */
	r4030_write_reg32(JAZZ_R4030_CHNL_ENABLE + (channel << 5),
			  r4030_read_reg32(JAZZ_R4030_CHNL_ENABLE +
					   (channel << 5)) |
			  R4030_CHNL_ENABLE);
}

EXPORT_SYMBOL(vdma_enable);

/*
                        
 */
void vdma_disable(int channel)
{
	if (vdma_debug) {
		int status =
		    r4030_read_reg32(JAZZ_R4030_CHNL_ENABLE +
				     (channel << 5));

		printk("vdma_disable: channel %d\n", channel);
		printk("VDMA: channel %d status: %04x (%s) mode: "
		       "%02x addr: %06x count: %06x\n",
		       channel, status,
		       ((status & 0x600) ? "ERROR" : "OK"),
		       (unsigned) r4030_read_reg32(JAZZ_R4030_CHNL_MODE +
						   (channel << 5)),
		       (unsigned) r4030_read_reg32(JAZZ_R4030_CHNL_ADDR +
						   (channel << 5)),
		       (unsigned) r4030_read_reg32(JAZZ_R4030_CHNL_COUNT +
						   (channel << 5)));
	}

	r4030_write_reg32(JAZZ_R4030_CHNL_ENABLE + (channel << 5),
			  r4030_read_reg32(JAZZ_R4030_CHNL_ENABLE +
					   (channel << 5)) &
			  ~R4030_CHNL_ENABLE);

	/*
                                                                 
                                                                       
  */
	*((volatile unsigned int *) JAZZ_DUMMY_DEVICE);
}

EXPORT_SYMBOL(vdma_disable);

/*
                                                           
                                                         
                                                          
          
                                                          
                                                             
           
 */
void vdma_set_mode(int channel, int mode)
{
	if (vdma_debug)
		printk("vdma_set_mode: channel %d, mode 0x%x\n", channel,
		       mode);

	switch (channel) {
	case JAZZ_SCSI_DMA:	/*      */
		r4030_write_reg32(JAZZ_R4030_CHNL_MODE + (channel << 5),
/*                       */
/*                        */
				  R4030_MODE_INTR_EN |
				  R4030_MODE_WIDTH_16 |
				  R4030_MODE_ATIME_80);
		break;

	case JAZZ_FLOPPY_DMA:	/*        */
		r4030_write_reg32(JAZZ_R4030_CHNL_MODE + (channel << 5),
/*                       */
/*                        */
				  R4030_MODE_INTR_EN |
				  R4030_MODE_WIDTH_8 |
				  R4030_MODE_ATIME_120);
		break;

	case JAZZ_AUDIOL_DMA:
	case JAZZ_AUDIOR_DMA:
		printk("VDMA: Audio DMA not supported yet.\n");
		break;

	default:
		printk
		    ("VDMA: vdma_set_mode() called with unsupported channel %d!\n",
		     channel);
	}

	switch (mode) {
	case DMA_MODE_READ:
		r4030_write_reg32(JAZZ_R4030_CHNL_ENABLE + (channel << 5),
				  r4030_read_reg32(JAZZ_R4030_CHNL_ENABLE +
						   (channel << 5)) &
				  ~R4030_CHNL_WRITE);
		break;

	case DMA_MODE_WRITE:
		r4030_write_reg32(JAZZ_R4030_CHNL_ENABLE + (channel << 5),
				  r4030_read_reg32(JAZZ_R4030_CHNL_ENABLE +
						   (channel << 5)) |
				  R4030_CHNL_WRITE);
		break;

	default:
		printk
		    ("VDMA: vdma_set_mode() called with unknown dma mode 0x%x\n",
		     mode);
	}
}

EXPORT_SYMBOL(vdma_set_mode);

/*
                       
 */
void vdma_set_addr(int channel, long addr)
{
	if (vdma_debug)
		printk("vdma_set_addr: channel %d, addr %lx\n", channel,
		       addr);

	r4030_write_reg32(JAZZ_R4030_CHNL_ADDR + (channel << 5), addr);
}

EXPORT_SYMBOL(vdma_set_addr);

/*
                     
 */
void vdma_set_count(int channel, int count)
{
	if (vdma_debug)
		printk("vdma_set_count: channel %d, count %08x\n", channel,
		       (unsigned) count);

	r4030_write_reg32(JAZZ_R4030_CHNL_COUNT + (channel << 5), count);
}

EXPORT_SYMBOL(vdma_set_count);

/*
               
 */
int vdma_get_residue(int channel)
{
	int residual;

	residual = r4030_read_reg32(JAZZ_R4030_CHNL_COUNT + (channel << 5));

	if (vdma_debug)
		printk("vdma_get_residual: channel %d: residual=%d\n",
		       channel, residual);

	return residual;
}

/*
                                  
 */
int vdma_get_enable(int channel)
{
	int enable;

	enable = r4030_read_reg32(JAZZ_R4030_CHNL_ENABLE + (channel << 5));

	if (vdma_debug)
		printk("vdma_get_enable: channel %d: enable=%d\n", channel,
		       enable);

	return enable;
}

arch_initcall(vdma_init);
