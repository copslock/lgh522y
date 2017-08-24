/* pci_schizo.c: SCHIZO/TOMATILLO specific PCI controller support.
 *
 * Copyright (C) 2001, 2002, 2003, 2007, 2008 David S. Miller (davem@davemloft.net)
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/export.h>
#include <linux/interrupt.h>
#include <linux/of_device.h>

#include <asm/iommu.h>
#include <asm/irq.h>
#include <asm/pstate.h>
#include <asm/prom.h>
#include <asm/upa.h>

#include "pci_impl.h"
#include "iommu_common.h"

#define DRIVER_NAME	"schizo"
#define PFX		DRIVER_NAME ": "

/*                                                        
                                                            
                   
  
                                                           
                                                           
                                                             
                                 
 */

/*                                    */
#define SCHIZO_STRBUF_CTRL_LPTR    0x00000000000000f0UL /*                  */
#define SCHIZO_STRBUF_CTRL_LENAB   0x0000000000000008UL /*                 */
#define SCHIZO_STRBUF_CTRL_RRDIS   0x0000000000000004UL /*               */
#define SCHIZO_STRBUF_CTRL_DENAB   0x0000000000000002UL /*                        */
#define SCHIZO_STRBUF_CTRL_ENAB    0x0000000000000001UL /*                         */

/*                         */
#define SCHIZO_IOMMU_CTRL_RESV     0xfffffffff9000000UL /*                               */
#define SCHIZO_IOMMU_CTRL_XLTESTAT 0x0000000006000000UL /*                               */
#define SCHIZO_IOMMU_CTRL_XLTEERR  0x0000000001000000UL /*                               */
#define SCHIZO_IOMMU_CTRL_LCKEN    0x0000000000800000UL /*                               */
#define SCHIZO_IOMMU_CTRL_LCKPTR   0x0000000000780000UL /*                               */
#define SCHIZO_IOMMU_CTRL_TSBSZ    0x0000000000070000UL /*                               */
#define SCHIZO_IOMMU_TSBSZ_1K      0x0000000000000000UL /*                               */
#define SCHIZO_IOMMU_TSBSZ_2K      0x0000000000010000UL /*                               */
#define SCHIZO_IOMMU_TSBSZ_4K      0x0000000000020000UL /*                               */
#define SCHIZO_IOMMU_TSBSZ_8K      0x0000000000030000UL /*                               */
#define SCHIZO_IOMMU_TSBSZ_16K     0x0000000000040000UL /*                               */
#define SCHIZO_IOMMU_TSBSZ_32K     0x0000000000050000UL /*                               */
#define SCHIZO_IOMMU_TSBSZ_64K     0x0000000000060000UL /*                               */
#define SCHIZO_IOMMU_TSBSZ_128K    0x0000000000070000UL /*                               */
#define SCHIZO_IOMMU_CTRL_RESV2    0x000000000000fff8UL /*                               */
#define SCHIZO_IOMMU_CTRL_TBWSZ    0x0000000000000004UL /*                               */
#define SCHIZO_IOMMU_CTRL_DENAB    0x0000000000000002UL /*                               */
#define SCHIZO_IOMMU_CTRL_ENAB     0x0000000000000001UL /*                               */

/*                                                          
                  
  
                                                          
                                                            
                                                            
                                                            
 */
#define SCHIZO_CONFIG_BASE(PBM)	((PBM)->config_space)
#define SCHIZO_CONFIG_ENCODE(BUS, DEVFN, REG)	\
	(((unsigned long)(BUS)   << 16) |	\
	 ((unsigned long)(DEVFN) << 8)  |	\
	 ((unsigned long)(REG)))

static void *schizo_pci_config_mkaddr(struct pci_pbm_info *pbm,
				      unsigned char bus,
				      unsigned int devfn,
				      int where)
{
	if (!pbm)
		return NULL;
	bus -= pbm->pci_first_busno;
	return (void *)
		(SCHIZO_CONFIG_BASE(pbm) |
		 SCHIZO_CONFIG_ENCODE(bus, devfn, where));
}

/*                                */
enum schizo_error_type {
	UE_ERR, CE_ERR, PCI_ERR, SAFARI_ERR
};

static DEFINE_SPINLOCK(stc_buf_lock);
static unsigned long stc_error_buf[128];
static unsigned long stc_tag_buf[16];
static unsigned long stc_line_buf[16];

#define SCHIZO_UE_INO		0x30 /*                         */
#define SCHIZO_CE_INO		0x31 /*                       */
#define SCHIZO_PCIERR_A_INO	0x32 /*                     */
#define SCHIZO_PCIERR_B_INO	0x33 /*                     */
#define SCHIZO_SERR_INO		0x34 /*                        */

#define SCHIZO_STC_ERR	0xb800UL /*            */
#define SCHIZO_STC_TAG	0xba00UL /*            */
#define SCHIZO_STC_LINE	0xbb00UL /*            */

#define SCHIZO_STCERR_WRITE	0x2UL
#define SCHIZO_STCERR_READ	0x1UL

#define SCHIZO_STCTAG_PPN	0x3fffffff00000000UL
#define SCHIZO_STCTAG_VPN	0x00000000ffffe000UL
#define SCHIZO_STCTAG_VALID	0x8000000000000000UL
#define SCHIZO_STCTAG_READ	0x4000000000000000UL

#define SCHIZO_STCLINE_LINDX	0x0000000007800000UL
#define SCHIZO_STCLINE_SPTR	0x000000000007e000UL
#define SCHIZO_STCLINE_LADDR	0x0000000000001fc0UL
#define SCHIZO_STCLINE_EPTR	0x000000000000003fUL
#define SCHIZO_STCLINE_VALID	0x0000000000600000UL
#define SCHIZO_STCLINE_FOFN	0x0000000000180000UL

static void __schizo_check_stc_error_pbm(struct pci_pbm_info *pbm,
					 enum schizo_error_type type)
{
	struct strbuf *strbuf = &pbm->stc;
	unsigned long regbase = pbm->pbm_regs;
	unsigned long err_base, tag_base, line_base;
	u64 control;
	int i;

	err_base = regbase + SCHIZO_STC_ERR;
	tag_base = regbase + SCHIZO_STC_TAG;
	line_base = regbase + SCHIZO_STC_LINE;

	spin_lock(&stc_buf_lock);

	/*                                               
                                                  
                                                   
                                                 
                                                  
                                              
                                                   
                
  */
	control = upa_readq(strbuf->strbuf_control);
	upa_writeq((control | SCHIZO_STRBUF_CTRL_DENAB),
		   strbuf->strbuf_control);
	for (i = 0; i < 128; i++) {
		unsigned long val;

		val = upa_readq(err_base + (i * 8UL));
		upa_writeq(0UL, err_base + (i * 8UL));
		stc_error_buf[i] = val;
	}
	for (i = 0; i < 16; i++) {
		stc_tag_buf[i] = upa_readq(tag_base + (i * 8UL));
		stc_line_buf[i] = upa_readq(line_base + (i * 8UL));
		upa_writeq(0UL, tag_base + (i * 8UL));
		upa_writeq(0UL, line_base + (i * 8UL));
	}

	/*                                            */
	upa_writeq(control, strbuf->strbuf_control);

	for (i = 0; i < 16; i++) {
		int j, saw_error, first, last;

		saw_error = 0;
		first = i * 8;
		last = first + 8;
		for (j = first; j < last; j++) {
			unsigned long errval = stc_error_buf[j];
			if (errval != 0) {
				saw_error++;
				printk("%s: STC_ERR(%d)[wr(%d)rd(%d)]\n",
				       pbm->name,
				       j,
				       (errval & SCHIZO_STCERR_WRITE) ? 1 : 0,
				       (errval & SCHIZO_STCERR_READ) ? 1 : 0);
			}
		}
		if (saw_error != 0) {
			unsigned long tagval = stc_tag_buf[i];
			unsigned long lineval = stc_line_buf[i];
			printk("%s: STC_TAG(%d)[PA(%016lx)VA(%08lx)V(%d)R(%d)]\n",
			       pbm->name,
			       i,
			       ((tagval & SCHIZO_STCTAG_PPN) >> 19UL),
			       (tagval & SCHIZO_STCTAG_VPN),
			       ((tagval & SCHIZO_STCTAG_VALID) ? 1 : 0),
			       ((tagval & SCHIZO_STCTAG_READ) ? 1 : 0));

			/*                                                          */
			printk("%s: STC_LINE(%d)[LIDX(%lx)SP(%lx)LADDR(%lx)EP(%lx)"
			       "V(%d)FOFN(%d)]\n",
			       pbm->name,
			       i,
			       ((lineval & SCHIZO_STCLINE_LINDX) >> 23UL),
			       ((lineval & SCHIZO_STCLINE_SPTR) >> 13UL),
			       ((lineval & SCHIZO_STCLINE_LADDR) >> 6UL),
			       ((lineval & SCHIZO_STCLINE_EPTR) >> 0UL),
			       ((lineval & SCHIZO_STCLINE_VALID) ? 1 : 0),
			       ((lineval & SCHIZO_STCLINE_FOFN) ? 1 : 0));
		}
	}

	spin_unlock(&stc_buf_lock);
}

/*                                                              
                           
 */

#define SCHIZO_IOMMU_TAG	0xa580UL
#define SCHIZO_IOMMU_DATA	0xa600UL

#define SCHIZO_IOMMU_TAG_CTXT	0x0000001ffe000000UL
#define SCHIZO_IOMMU_TAG_ERRSTS	0x0000000001800000UL
#define SCHIZO_IOMMU_TAG_ERR	0x0000000000400000UL
#define SCHIZO_IOMMU_TAG_WRITE	0x0000000000200000UL
#define SCHIZO_IOMMU_TAG_STREAM	0x0000000000100000UL
#define SCHIZO_IOMMU_TAG_SIZE	0x0000000000080000UL
#define SCHIZO_IOMMU_TAG_VPAGE	0x000000000007ffffUL

#define SCHIZO_IOMMU_DATA_VALID	0x0000000100000000UL
#define SCHIZO_IOMMU_DATA_CACHE	0x0000000040000000UL
#define SCHIZO_IOMMU_DATA_PPAGE	0x000000003fffffffUL

static void schizo_check_iommu_error_pbm(struct pci_pbm_info *pbm,
					 enum schizo_error_type type)
{
	struct iommu *iommu = pbm->iommu;
	unsigned long iommu_tag[16];
	unsigned long iommu_data[16];
	unsigned long flags;
	u64 control;
	int i;

	spin_lock_irqsave(&iommu->lock, flags);
	control = upa_readq(iommu->iommu_control);
	if (control & SCHIZO_IOMMU_CTRL_XLTEERR) {
		unsigned long base;
		char *type_string;

		/*                                  */
		control &= ~SCHIZO_IOMMU_CTRL_XLTEERR;
		upa_writeq(control, iommu->iommu_control);

		switch((control & SCHIZO_IOMMU_CTRL_XLTESTAT) >> 25UL) {
		case 0:
			type_string = "Protection Error";
			break;
		case 1:
			type_string = "Invalid Error";
			break;
		case 2:
			type_string = "TimeOut Error";
			break;
		case 3:
		default:
			type_string = "ECC Error";
			break;
		}
		printk("%s: IOMMU Error, type[%s]\n",
		       pbm->name, type_string);

		/*                                             
                                            
    
                                                  
                                                   
                                                  
                                                 
                                              
                       
   */
		upa_writeq(control | SCHIZO_IOMMU_CTRL_DENAB,
			   iommu->iommu_control);

		base = pbm->pbm_regs;

		for (i = 0; i < 16; i++) {
			iommu_tag[i] =
				upa_readq(base + SCHIZO_IOMMU_TAG + (i * 8UL));
			iommu_data[i] =
				upa_readq(base + SCHIZO_IOMMU_DATA + (i * 8UL));

			/*                          */
			upa_writeq(0, base + SCHIZO_IOMMU_TAG + (i * 8UL));
			upa_writeq(0, base + SCHIZO_IOMMU_DATA + (i * 8UL));
		}

		/*                        */
		upa_writeq(control, iommu->iommu_control);

		for (i = 0; i < 16; i++) {
			unsigned long tag, data;

			tag = iommu_tag[i];
			if (!(tag & SCHIZO_IOMMU_TAG_ERR))
				continue;

			data = iommu_data[i];
			switch((tag & SCHIZO_IOMMU_TAG_ERRSTS) >> 23UL) {
			case 0:
				type_string = "Protection Error";
				break;
			case 1:
				type_string = "Invalid Error";
				break;
			case 2:
				type_string = "TimeOut Error";
				break;
			case 3:
			default:
				type_string = "ECC Error";
				break;
			}
			printk("%s: IOMMU TAG(%d)[error(%s) ctx(%x) wr(%d) str(%d) "
			       "sz(%dK) vpg(%08lx)]\n",
			       pbm->name, i, type_string,
			       (int)((tag & SCHIZO_IOMMU_TAG_CTXT) >> 25UL),
			       ((tag & SCHIZO_IOMMU_TAG_WRITE) ? 1 : 0),
			       ((tag & SCHIZO_IOMMU_TAG_STREAM) ? 1 : 0),
			       ((tag & SCHIZO_IOMMU_TAG_SIZE) ? 64 : 8),
			       (tag & SCHIZO_IOMMU_TAG_VPAGE) << IOMMU_PAGE_SHIFT);
			printk("%s: IOMMU DATA(%d)[valid(%d) cache(%d) ppg(%016lx)]\n",
			       pbm->name, i,
			       ((data & SCHIZO_IOMMU_DATA_VALID) ? 1 : 0),
			       ((data & SCHIZO_IOMMU_DATA_CACHE) ? 1 : 0),
			       (data & SCHIZO_IOMMU_DATA_PPAGE) << IOMMU_PAGE_SHIFT);
		}
	}
	if (pbm->stc.strbuf_enabled)
		__schizo_check_stc_error_pbm(pbm, type);
	spin_unlock_irqrestore(&iommu->lock, flags);
}

static void schizo_check_iommu_error(struct pci_pbm_info *pbm,
				     enum schizo_error_type type)
{
	schizo_check_iommu_error_pbm(pbm, type);
	if (pbm->sibling)
		schizo_check_iommu_error_pbm(pbm->sibling, type);
}

/*                                           */
#define SCHIZO_UE_AFSR	0x10030UL
#define SCHIZO_UE_AFAR	0x10038UL

#define SCHIZO_UEAFSR_PPIO	0x8000000000000000UL /*        */
#define SCHIZO_UEAFSR_PDRD	0x4000000000000000UL /*                  */
#define SCHIZO_UEAFSR_PDWR	0x2000000000000000UL /*        */
#define SCHIZO_UEAFSR_SPIO	0x1000000000000000UL /*        */
#define SCHIZO_UEAFSR_SDMA	0x0800000000000000UL /*                  */
#define SCHIZO_UEAFSR_ERRPNDG	0x0300000000000000UL /*        */
#define SCHIZO_UEAFSR_BMSK	0x000003ff00000000UL /*        */
#define SCHIZO_UEAFSR_QOFF	0x00000000c0000000UL /*                  */
#define SCHIZO_UEAFSR_AID	0x000000001f000000UL /*                  */
#define SCHIZO_UEAFSR_PARTIAL	0x0000000000800000UL /*        */
#define SCHIZO_UEAFSR_OWNEDIN	0x0000000000400000UL /*        */
#define SCHIZO_UEAFSR_MTAGSYND	0x00000000000f0000UL /*        */
#define SCHIZO_UEAFSR_MTAG	0x000000000000e000UL /*        */
#define SCHIZO_UEAFSR_ECCSYND	0x00000000000001ffUL /*        */

static irqreturn_t schizo_ue_intr(int irq, void *dev_id)
{
	struct pci_pbm_info *pbm = dev_id;
	unsigned long afsr_reg = pbm->controller_regs + SCHIZO_UE_AFSR;
	unsigned long afar_reg = pbm->controller_regs + SCHIZO_UE_AFAR;
	unsigned long afsr, afar, error_bits;
	int reported, limit;

	/*                                   */
	afar = upa_readq(afar_reg);

	/*                                                   
                                                       
                                                          
  */
	limit = 1000;
	do {
		afsr = upa_readq(afsr_reg);
	} while ((afsr & SCHIZO_UEAFSR_ERRPNDG) != 0 && --limit);

	/*                                                */
	error_bits = afsr &
		(SCHIZO_UEAFSR_PPIO | SCHIZO_UEAFSR_PDRD | SCHIZO_UEAFSR_PDWR |
		 SCHIZO_UEAFSR_SPIO | SCHIZO_UEAFSR_SDMA);
	if (!error_bits)
		return IRQ_NONE;
	upa_writeq(error_bits, afsr_reg);

	/*                */
	printk("%s: Uncorrectable Error, primary error type[%s]\n",
	       pbm->name,
	       (((error_bits & SCHIZO_UEAFSR_PPIO) ?
		 "PIO" :
		 ((error_bits & SCHIZO_UEAFSR_PDRD) ?
		  "DMA Read" :
		  ((error_bits & SCHIZO_UEAFSR_PDWR) ?
		   "DMA Write" : "???")))));
	printk("%s: bytemask[%04lx] qword_offset[%lx] SAFARI_AID[%02lx]\n",
	       pbm->name,
	       (afsr & SCHIZO_UEAFSR_BMSK) >> 32UL,
	       (afsr & SCHIZO_UEAFSR_QOFF) >> 30UL,
	       (afsr & SCHIZO_UEAFSR_AID) >> 24UL);
	printk("%s: partial[%d] owned_in[%d] mtag[%lx] mtag_synd[%lx] ecc_sync[%lx]\n",
	       pbm->name,
	       (afsr & SCHIZO_UEAFSR_PARTIAL) ? 1 : 0,
	       (afsr & SCHIZO_UEAFSR_OWNEDIN) ? 1 : 0,
	       (afsr & SCHIZO_UEAFSR_MTAG) >> 13UL,
	       (afsr & SCHIZO_UEAFSR_MTAGSYND) >> 16UL,
	       (afsr & SCHIZO_UEAFSR_ECCSYND) >> 0UL);
	printk("%s: UE AFAR [%016lx]\n", pbm->name, afar);
	printk("%s: UE Secondary errors [", pbm->name);
	reported = 0;
	if (afsr & SCHIZO_UEAFSR_SPIO) {
		reported++;
		printk("(PIO)");
	}
	if (afsr & SCHIZO_UEAFSR_SDMA) {
		reported++;
		printk("(DMA)");
	}
	if (!reported)
		printk("(none)");
	printk("]\n");

	/*                                     */
	schizo_check_iommu_error(pbm, UE_ERR);

	return IRQ_HANDLED;
}

#define SCHIZO_CE_AFSR	0x10040UL
#define SCHIZO_CE_AFAR	0x10048UL

#define SCHIZO_CEAFSR_PPIO	0x8000000000000000UL
#define SCHIZO_CEAFSR_PDRD	0x4000000000000000UL
#define SCHIZO_CEAFSR_PDWR	0x2000000000000000UL
#define SCHIZO_CEAFSR_SPIO	0x1000000000000000UL
#define SCHIZO_CEAFSR_SDMA	0x0800000000000000UL
#define SCHIZO_CEAFSR_ERRPNDG	0x0300000000000000UL
#define SCHIZO_CEAFSR_BMSK	0x000003ff00000000UL
#define SCHIZO_CEAFSR_QOFF	0x00000000c0000000UL
#define SCHIZO_CEAFSR_AID	0x000000001f000000UL
#define SCHIZO_CEAFSR_PARTIAL	0x0000000000800000UL
#define SCHIZO_CEAFSR_OWNEDIN	0x0000000000400000UL
#define SCHIZO_CEAFSR_MTAGSYND	0x00000000000f0000UL
#define SCHIZO_CEAFSR_MTAG	0x000000000000e000UL
#define SCHIZO_CEAFSR_ECCSYND	0x00000000000001ffUL

static irqreturn_t schizo_ce_intr(int irq, void *dev_id)
{
	struct pci_pbm_info *pbm = dev_id;
	unsigned long afsr_reg = pbm->controller_regs + SCHIZO_CE_AFSR;
	unsigned long afar_reg = pbm->controller_regs + SCHIZO_CE_AFAR;
	unsigned long afsr, afar, error_bits;
	int reported, limit;

	/*                     */
	afar = upa_readq(afar_reg);

	/*                                                   
                                                       
                                                          
  */
	limit = 1000;
	do {
		afsr = upa_readq(afsr_reg);
	} while ((afsr & SCHIZO_UEAFSR_ERRPNDG) != 0 && --limit);

	/*                                            */
	error_bits = afsr &
		(SCHIZO_CEAFSR_PPIO | SCHIZO_CEAFSR_PDRD | SCHIZO_CEAFSR_PDWR |
		 SCHIZO_CEAFSR_SPIO | SCHIZO_CEAFSR_SDMA);
	if (!error_bits)
		return IRQ_NONE;
	upa_writeq(error_bits, afsr_reg);

	/*                */
	printk("%s: Correctable Error, primary error type[%s]\n",
	       pbm->name,
	       (((error_bits & SCHIZO_CEAFSR_PPIO) ?
		 "PIO" :
		 ((error_bits & SCHIZO_CEAFSR_PDRD) ?
		  "DMA Read" :
		  ((error_bits & SCHIZO_CEAFSR_PDWR) ?
		   "DMA Write" : "???")))));

	/*                                                               
                                          
  */
	printk("%s: bytemask[%04lx] qword_offset[%lx] SAFARI_AID[%02lx]\n",
	       pbm->name,
	       (afsr & SCHIZO_UEAFSR_BMSK) >> 32UL,
	       (afsr & SCHIZO_UEAFSR_QOFF) >> 30UL,
	       (afsr & SCHIZO_UEAFSR_AID) >> 24UL);
	printk("%s: partial[%d] owned_in[%d] mtag[%lx] mtag_synd[%lx] ecc_sync[%lx]\n",
	       pbm->name,
	       (afsr & SCHIZO_UEAFSR_PARTIAL) ? 1 : 0,
	       (afsr & SCHIZO_UEAFSR_OWNEDIN) ? 1 : 0,
	       (afsr & SCHIZO_UEAFSR_MTAG) >> 13UL,
	       (afsr & SCHIZO_UEAFSR_MTAGSYND) >> 16UL,
	       (afsr & SCHIZO_UEAFSR_ECCSYND) >> 0UL);
	printk("%s: CE AFAR [%016lx]\n", pbm->name, afar);
	printk("%s: CE Secondary errors [", pbm->name);
	reported = 0;
	if (afsr & SCHIZO_CEAFSR_SPIO) {
		reported++;
		printk("(PIO)");
	}
	if (afsr & SCHIZO_CEAFSR_SDMA) {
		reported++;
		printk("(DMA)");
	}
	if (!reported)
		printk("(none)");
	printk("]\n");

	return IRQ_HANDLED;
}

#define SCHIZO_PCI_AFSR	0x2010UL
#define SCHIZO_PCI_AFAR	0x2018UL

#define SCHIZO_PCIAFSR_PMA	0x8000000000000000UL /*                  */
#define SCHIZO_PCIAFSR_PTA	0x4000000000000000UL /*                  */
#define SCHIZO_PCIAFSR_PRTRY	0x2000000000000000UL /*                  */
#define SCHIZO_PCIAFSR_PPERR	0x1000000000000000UL /*                  */
#define SCHIZO_PCIAFSR_PTTO	0x0800000000000000UL /*                  */
#define SCHIZO_PCIAFSR_PUNUS	0x0400000000000000UL /*        */
#define SCHIZO_PCIAFSR_SMA	0x0200000000000000UL /*                  */
#define SCHIZO_PCIAFSR_STA	0x0100000000000000UL /*                  */
#define SCHIZO_PCIAFSR_SRTRY	0x0080000000000000UL /*                  */
#define SCHIZO_PCIAFSR_SPERR	0x0040000000000000UL /*                  */
#define SCHIZO_PCIAFSR_STTO	0x0020000000000000UL /*                  */
#define SCHIZO_PCIAFSR_SUNUS	0x0010000000000000UL /*        */
#define SCHIZO_PCIAFSR_BMSK	0x000003ff00000000UL /*                  */
#define SCHIZO_PCIAFSR_BLK	0x0000000080000000UL /*                  */
#define SCHIZO_PCIAFSR_CFG	0x0000000040000000UL /*                  */
#define SCHIZO_PCIAFSR_MEM	0x0000000020000000UL /*                  */
#define SCHIZO_PCIAFSR_IO	0x0000000010000000UL /*                  */

#define SCHIZO_PCI_CTRL		(0x2000UL)
#define SCHIZO_PCICTRL_BUS_UNUS	(1UL << 63UL) /*        */
#define SCHIZO_PCICTRL_DTO_INT	(1UL << 61UL) /*           */
#define SCHIZO_PCICTRL_ARB_PRIO (0x1ff << 52UL) /*           */
#define SCHIZO_PCICTRL_ESLCK	(1UL << 51UL) /*        */
#define SCHIZO_PCICTRL_ERRSLOT	(7UL << 48UL) /*        */
#define SCHIZO_PCICTRL_TTO_ERR	(1UL << 38UL) /*                  */
#define SCHIZO_PCICTRL_RTRY_ERR	(1UL << 37UL) /*                  */
#define SCHIZO_PCICTRL_DTO_ERR	(1UL << 36UL) /*                  */
#define SCHIZO_PCICTRL_SBH_ERR	(1UL << 35UL) /*        */
#define SCHIZO_PCICTRL_SERR	(1UL << 34UL) /*                  */
#define SCHIZO_PCICTRL_PCISPD	(1UL << 33UL) /*        */
#define SCHIZO_PCICTRL_MRM_PREF	(1UL << 30UL) /*           */
#define SCHIZO_PCICTRL_RDO_PREF	(1UL << 29UL) /*           */
#define SCHIZO_PCICTRL_RDL_PREF	(1UL << 28UL) /*           */
#define SCHIZO_PCICTRL_PTO	(3UL << 24UL) /*                  */
#define SCHIZO_PCICTRL_PTO_SHIFT 24UL
#define SCHIZO_PCICTRL_TRWSW	(7UL << 21UL) /*           */
#define SCHIZO_PCICTRL_F_TGT_A	(1UL << 20UL) /*           */
#define SCHIZO_PCICTRL_S_DTO_INT (1UL << 19UL) /*        */
#define SCHIZO_PCICTRL_F_TGT_RT	(1UL << 19UL) /*           */
#define SCHIZO_PCICTRL_SBH_INT	(1UL << 18UL) /*        */
#define SCHIZO_PCICTRL_T_DTO_INT (1UL << 18UL) /*           */
#define SCHIZO_PCICTRL_EEN	(1UL << 17UL) /*                  */
#define SCHIZO_PCICTRL_PARK	(1UL << 16UL) /*                  */
#define SCHIZO_PCICTRL_PCIRST	(1UL <<  8UL) /*        */
#define SCHIZO_PCICTRL_ARB_S	(0x3fUL << 0UL) /*        */
#define SCHIZO_PCICTRL_ARB_T	(0xffUL << 0UL) /*           */

static irqreturn_t schizo_pcierr_intr_other(struct pci_pbm_info *pbm)
{
	unsigned long csr_reg, csr, csr_error_bits;
	irqreturn_t ret = IRQ_NONE;
	u16 stat;

	csr_reg = pbm->pbm_regs + SCHIZO_PCI_CTRL;
	csr = upa_readq(csr_reg);
	csr_error_bits =
		csr & (SCHIZO_PCICTRL_BUS_UNUS |
		       SCHIZO_PCICTRL_TTO_ERR |
		       SCHIZO_PCICTRL_RTRY_ERR |
		       SCHIZO_PCICTRL_DTO_ERR |
		       SCHIZO_PCICTRL_SBH_ERR |
		       SCHIZO_PCICTRL_SERR);
	if (csr_error_bits) {
		/*                    */
		upa_writeq(csr, csr_reg);

		/*           */
		if (csr_error_bits & SCHIZO_PCICTRL_BUS_UNUS)
			printk("%s: Bus unusable error asserted.\n",
			       pbm->name);
		if (csr_error_bits & SCHIZO_PCICTRL_TTO_ERR)
			printk("%s: PCI TRDY# timeout error asserted.\n",
			       pbm->name);
		if (csr_error_bits & SCHIZO_PCICTRL_RTRY_ERR)
			printk("%s: PCI excessive retry error asserted.\n",
			       pbm->name);
		if (csr_error_bits & SCHIZO_PCICTRL_DTO_ERR)
			printk("%s: PCI discard timeout error asserted.\n",
			       pbm->name);
		if (csr_error_bits & SCHIZO_PCICTRL_SBH_ERR)
			printk("%s: PCI streaming byte hole error asserted.\n",
			       pbm->name);
		if (csr_error_bits & SCHIZO_PCICTRL_SERR)
			printk("%s: PCI SERR signal asserted.\n",
			       pbm->name);
		ret = IRQ_HANDLED;
	}
	pci_read_config_word(pbm->pci_bus->self, PCI_STATUS, &stat);
	if (stat & (PCI_STATUS_PARITY |
		    PCI_STATUS_SIG_TARGET_ABORT |
		    PCI_STATUS_REC_TARGET_ABORT |
		    PCI_STATUS_REC_MASTER_ABORT |
		    PCI_STATUS_SIG_SYSTEM_ERROR)) {
		printk("%s: PCI bus error, PCI_STATUS[%04x]\n",
		       pbm->name, stat);
		pci_write_config_word(pbm->pci_bus->self, PCI_STATUS, 0xffff);
		ret = IRQ_HANDLED;
	}
	return ret;
}

static irqreturn_t schizo_pcierr_intr(int irq, void *dev_id)
{
	struct pci_pbm_info *pbm = dev_id;
	unsigned long afsr_reg, afar_reg, base;
	unsigned long afsr, afar, error_bits;
	int reported;

	base = pbm->pbm_regs;

	afsr_reg = base + SCHIZO_PCI_AFSR;
	afar_reg = base + SCHIZO_PCI_AFAR;

	/*                     */
	afar = upa_readq(afar_reg);
	afsr = upa_readq(afsr_reg);

	/*                                            */
	error_bits = afsr &
		(SCHIZO_PCIAFSR_PMA | SCHIZO_PCIAFSR_PTA |
		 SCHIZO_PCIAFSR_PRTRY | SCHIZO_PCIAFSR_PPERR |
		 SCHIZO_PCIAFSR_PTTO | SCHIZO_PCIAFSR_PUNUS |
		 SCHIZO_PCIAFSR_SMA | SCHIZO_PCIAFSR_STA |
		 SCHIZO_PCIAFSR_SRTRY | SCHIZO_PCIAFSR_SPERR |
		 SCHIZO_PCIAFSR_STTO | SCHIZO_PCIAFSR_SUNUS);
	if (!error_bits)
		return schizo_pcierr_intr_other(pbm);
	upa_writeq(error_bits, afsr_reg);

	/*                */
	printk("%s: PCI Error, primary error type[%s]\n",
	       pbm->name,
	       (((error_bits & SCHIZO_PCIAFSR_PMA) ?
		 "Master Abort" :
		 ((error_bits & SCHIZO_PCIAFSR_PTA) ?
		  "Target Abort" :
		  ((error_bits & SCHIZO_PCIAFSR_PRTRY) ?
		   "Excessive Retries" :
		   ((error_bits & SCHIZO_PCIAFSR_PPERR) ?
		    "Parity Error" :
		    ((error_bits & SCHIZO_PCIAFSR_PTTO) ?
		     "Timeout" :
		     ((error_bits & SCHIZO_PCIAFSR_PUNUS) ?
		      "Bus Unusable" : "???"))))))));
	printk("%s: bytemask[%04lx] was_block(%d) space(%s)\n",
	       pbm->name,
	       (afsr & SCHIZO_PCIAFSR_BMSK) >> 32UL,
	       (afsr & SCHIZO_PCIAFSR_BLK) ? 1 : 0,
	       ((afsr & SCHIZO_PCIAFSR_CFG) ?
		"Config" :
		((afsr & SCHIZO_PCIAFSR_MEM) ?
		 "Memory" :
		 ((afsr & SCHIZO_PCIAFSR_IO) ?
		  "I/O" : "???"))));
	printk("%s: PCI AFAR [%016lx]\n",
	       pbm->name, afar);
	printk("%s: PCI Secondary errors [",
	       pbm->name);
	reported = 0;
	if (afsr & SCHIZO_PCIAFSR_SMA) {
		reported++;
		printk("(Master Abort)");
	}
	if (afsr & SCHIZO_PCIAFSR_STA) {
		reported++;
		printk("(Target Abort)");
	}
	if (afsr & SCHIZO_PCIAFSR_SRTRY) {
		reported++;
		printk("(Excessive Retries)");
	}
	if (afsr & SCHIZO_PCIAFSR_SPERR) {
		reported++;
		printk("(Parity Error)");
	}
	if (afsr & SCHIZO_PCIAFSR_STTO) {
		reported++;
		printk("(Timeout)");
	}
	if (afsr & SCHIZO_PCIAFSR_SUNUS) {
		reported++;
		printk("(Bus Unusable)");
	}
	if (!reported)
		printk("(none)");
	printk("]\n");

	/*                                                          
                                      
  */

	/*                                                         
                                                          
                                                          
                                                           
  */
	if (error_bits & (SCHIZO_PCIAFSR_PTA | SCHIZO_PCIAFSR_STA)) {
		schizo_check_iommu_error(pbm, PCI_ERR);
		pci_scan_for_target_abort(pbm, pbm->pci_bus);
	}
	if (error_bits & (SCHIZO_PCIAFSR_PMA | SCHIZO_PCIAFSR_SMA))
		pci_scan_for_master_abort(pbm, pbm->pci_bus);

	/*                                                        
                                                           
                                                          
                                                        
                 
  */

	if (error_bits & (SCHIZO_PCIAFSR_PPERR | SCHIZO_PCIAFSR_SPERR))
		pci_scan_for_parity_error(pbm, pbm->pci_bus);

	return IRQ_HANDLED;
}

#define SCHIZO_SAFARI_ERRLOG	0x10018UL

#define SAFARI_ERRLOG_ERROUT	0x8000000000000000UL

#define BUS_ERROR_BADCMD	0x4000000000000000UL /*                  */
#define BUS_ERROR_SSMDIS	0x2000000000000000UL /*        */
#define BUS_ERROR_BADMA		0x1000000000000000UL /*        */
#define BUS_ERROR_BADMB		0x0800000000000000UL /*        */
#define BUS_ERROR_BADMC		0x0400000000000000UL /*        */
#define BUS_ERROR_SNOOP_GR	0x0000000000200000UL /*           */
#define BUS_ERROR_SNOOP_PCI	0x0000000000100000UL /*           */
#define BUS_ERROR_SNOOP_RD	0x0000000000080000UL /*           */
#define BUS_ERROR_SNOOP_RDS	0x0000000000020000UL /*           */
#define BUS_ERROR_SNOOP_RDSA	0x0000000000010000UL /*           */
#define BUS_ERROR_SNOOP_OWN	0x0000000000008000UL /*           */
#define BUS_ERROR_SNOOP_RDO	0x0000000000004000UL /*           */
#define BUS_ERROR_CPU1PS	0x0000000000002000UL /*        */
#define BUS_ERROR_WDATA_PERR	0x0000000000002000UL /*           */
#define BUS_ERROR_CPU1PB	0x0000000000001000UL /*        */
#define BUS_ERROR_CTRL_PERR	0x0000000000001000UL /*           */
#define BUS_ERROR_CPU0PS	0x0000000000000800UL /*        */
#define BUS_ERROR_SNOOP_ERR	0x0000000000000800UL /*           */
#define BUS_ERROR_CPU0PB	0x0000000000000400UL /*        */
#define BUS_ERROR_JBUS_ILL_B	0x0000000000000400UL /*           */
#define BUS_ERROR_CIQTO		0x0000000000000200UL /*        */
#define BUS_ERROR_LPQTO		0x0000000000000100UL /*        */
#define BUS_ERROR_JBUS_ILL_C	0x0000000000000100UL /*           */
#define BUS_ERROR_SFPQTO	0x0000000000000080UL /*        */
#define BUS_ERROR_UFPQTO	0x0000000000000040UL /*        */
#define BUS_ERROR_RD_PERR	0x0000000000000040UL /*           */
#define BUS_ERROR_APERR		0x0000000000000020UL /*                  */
#define BUS_ERROR_UNMAP		0x0000000000000010UL /*                  */
#define BUS_ERROR_BUSERR	0x0000000000000004UL /*                  */
#define BUS_ERROR_TIMEOUT	0x0000000000000002UL /*                  */
#define BUS_ERROR_ILL		0x0000000000000001UL /*        */

/*                                                                 
                                                  
 */
static irqreturn_t schizo_safarierr_intr(int irq, void *dev_id)
{
	struct pci_pbm_info *pbm = dev_id;
	u64 errlog;

	errlog = upa_readq(pbm->controller_regs + SCHIZO_SAFARI_ERRLOG);
	upa_writeq(errlog & ~(SAFARI_ERRLOG_ERROUT),
		   pbm->controller_regs + SCHIZO_SAFARI_ERRLOG);

	if (!(errlog & BUS_ERROR_UNMAP)) {
		printk("%s: Unexpected Safari/JBUS error interrupt, errlog[%016llx]\n",
		       pbm->name, errlog);

		return IRQ_HANDLED;
	}

	printk("%s: Safari/JBUS interrupt, UNMAPPED error, interrogating IOMMUs.\n",
	       pbm->name);
	schizo_check_iommu_error(pbm, SAFARI_ERR);

	return IRQ_HANDLED;
}

/*                                           */
#define SCHIZO_ECC_CTRL		0x10020UL
#define  SCHIZO_ECCCTRL_EE	 0x8000000000000000UL /*                     */
#define  SCHIZO_ECCCTRL_UE	 0x4000000000000000UL /*                      */
#define  SCHIZO_ECCCTRL_CE	 0x2000000000000000UL /*                      */

#define SCHIZO_SAFARI_ERRCTRL	0x10008UL
#define  SCHIZO_SAFERRCTRL_EN	 0x8000000000000000UL
#define SCHIZO_SAFARI_IRQCTRL	0x10010UL
#define  SCHIZO_SAFIRQCTRL_EN	 0x8000000000000000UL

static int pbm_routes_this_ino(struct pci_pbm_info *pbm, u32 ino)
{
	ino &= IMAP_INO;

	if (pbm->ino_bitmap & (1UL << ino))
		return 1;

	return 0;
}

/*                                                                 
  
                                                                     
                                                                         
                                                                      
                                                                    
  
                                                                      
                                     
  
                                                                      
                                                                
                      
 */
static void tomatillo_register_error_handlers(struct pci_pbm_info *pbm)
{
	struct platform_device *op = of_find_device_by_node(pbm->op->dev.of_node);
	u64 tmp, err_mask, err_no_mask;
	int err;

	/*                                  
             
             
             
           
                  
  */

	if (pbm_routes_this_ino(pbm, SCHIZO_UE_INO)) {
		err = request_irq(op->archdata.irqs[1], schizo_ue_intr, 0,
				  "TOMATILLO_UE", pbm);
		if (err)
			printk(KERN_WARNING "%s: Could not register UE, "
			       "err=%d\n", pbm->name, err);
	}
	if (pbm_routes_this_ino(pbm, SCHIZO_CE_INO)) {
		err = request_irq(op->archdata.irqs[2], schizo_ce_intr, 0,
				  "TOMATILLO_CE", pbm);
		if (err)
			printk(KERN_WARNING "%s: Could not register CE, "
			       "err=%d\n", pbm->name, err);
	}
	err = 0;
	if (pbm_routes_this_ino(pbm, SCHIZO_PCIERR_A_INO)) {
		err = request_irq(op->archdata.irqs[0], schizo_pcierr_intr, 0,
				  "TOMATILLO_PCIERR", pbm);
	} else if (pbm_routes_this_ino(pbm, SCHIZO_PCIERR_B_INO)) {
		err = request_irq(op->archdata.irqs[0], schizo_pcierr_intr, 0,
				  "TOMATILLO_PCIERR", pbm);
	}
	if (err)
		printk(KERN_WARNING "%s: Could not register PCIERR, "
		       "err=%d\n", pbm->name, err);

	if (pbm_routes_this_ino(pbm, SCHIZO_SERR_INO)) {
		err = request_irq(op->archdata.irqs[3], schizo_safarierr_intr, 0,
				  "TOMATILLO_SERR", pbm);
		if (err)
			printk(KERN_WARNING "%s: Could not register SERR, "
			       "err=%d\n", pbm->name, err);
	}

	/*                                             */
	upa_writeq((SCHIZO_ECCCTRL_EE |
		    SCHIZO_ECCCTRL_UE |
		    SCHIZO_ECCCTRL_CE), pbm->controller_regs + SCHIZO_ECC_CTRL);

	/*                                            
         
  */
	err_mask = (SCHIZO_PCICTRL_BUS_UNUS |
		    SCHIZO_PCICTRL_TTO_ERR |
		    SCHIZO_PCICTRL_RTRY_ERR |
		    SCHIZO_PCICTRL_SERR |
		    SCHIZO_PCICTRL_EEN);

	err_no_mask = SCHIZO_PCICTRL_DTO_ERR;

	tmp = upa_readq(pbm->pbm_regs + SCHIZO_PCI_CTRL);
	tmp |= err_mask;
	tmp &= ~err_no_mask;
	upa_writeq(tmp, pbm->pbm_regs + SCHIZO_PCI_CTRL);

	err_mask = (SCHIZO_PCIAFSR_PMA | SCHIZO_PCIAFSR_PTA |
		    SCHIZO_PCIAFSR_PRTRY | SCHIZO_PCIAFSR_PPERR |
		    SCHIZO_PCIAFSR_PTTO |
		    SCHIZO_PCIAFSR_SMA | SCHIZO_PCIAFSR_STA |
		    SCHIZO_PCIAFSR_SRTRY | SCHIZO_PCIAFSR_SPERR |
		    SCHIZO_PCIAFSR_STTO);

	upa_writeq(err_mask, pbm->pbm_regs + SCHIZO_PCI_AFSR);

	err_mask = (BUS_ERROR_BADCMD | BUS_ERROR_SNOOP_GR |
		    BUS_ERROR_SNOOP_PCI | BUS_ERROR_SNOOP_RD |
		    BUS_ERROR_SNOOP_RDS | BUS_ERROR_SNOOP_RDSA |
		    BUS_ERROR_SNOOP_OWN | BUS_ERROR_SNOOP_RDO |
		    BUS_ERROR_WDATA_PERR | BUS_ERROR_CTRL_PERR |
		    BUS_ERROR_SNOOP_ERR | BUS_ERROR_JBUS_ILL_B |
		    BUS_ERROR_JBUS_ILL_C | BUS_ERROR_RD_PERR |
		    BUS_ERROR_APERR | BUS_ERROR_UNMAP |
		    BUS_ERROR_BUSERR | BUS_ERROR_TIMEOUT);

	upa_writeq((SCHIZO_SAFERRCTRL_EN | err_mask),
		   pbm->controller_regs + SCHIZO_SAFARI_ERRCTRL);

	upa_writeq((SCHIZO_SAFIRQCTRL_EN | (BUS_ERROR_UNMAP)),
		   pbm->controller_regs + SCHIZO_SAFARI_IRQCTRL);
}

static void schizo_register_error_handlers(struct pci_pbm_info *pbm)
{
	struct platform_device *op = of_find_device_by_node(pbm->op->dev.of_node);
	u64 tmp, err_mask, err_no_mask;
	int err;

	/*                               
             
             
             
           
                  
  */

	if (pbm_routes_this_ino(pbm, SCHIZO_UE_INO)) {
		err = request_irq(op->archdata.irqs[1], schizo_ue_intr, 0,
				  "SCHIZO_UE", pbm);
		if (err)
			printk(KERN_WARNING "%s: Could not register UE, "
			       "err=%d\n", pbm->name, err);
	}
	if (pbm_routes_this_ino(pbm, SCHIZO_CE_INO)) {
		err = request_irq(op->archdata.irqs[2], schizo_ce_intr, 0,
				  "SCHIZO_CE", pbm);
		if (err)
			printk(KERN_WARNING "%s: Could not register CE, "
			       "err=%d\n", pbm->name, err);
	}
	err = 0;
	if (pbm_routes_this_ino(pbm, SCHIZO_PCIERR_A_INO)) {
		err = request_irq(op->archdata.irqs[0], schizo_pcierr_intr, 0,
				  "SCHIZO_PCIERR", pbm);
	} else if (pbm_routes_this_ino(pbm, SCHIZO_PCIERR_B_INO)) {
		err = request_irq(op->archdata.irqs[0], schizo_pcierr_intr, 0,
				  "SCHIZO_PCIERR", pbm);
	}
	if (err)
		printk(KERN_WARNING "%s: Could not register PCIERR, "
		       "err=%d\n", pbm->name, err);

	if (pbm_routes_this_ino(pbm, SCHIZO_SERR_INO)) {
		err = request_irq(op->archdata.irqs[3], schizo_safarierr_intr, 0,
				  "SCHIZO_SERR", pbm);
		if (err)
			printk(KERN_WARNING "%s: Could not register SERR, "
			       "err=%d\n", pbm->name, err);
	}

	/*                                             */
	upa_writeq((SCHIZO_ECCCTRL_EE |
		    SCHIZO_ECCCTRL_UE |
		    SCHIZO_ECCCTRL_CE), pbm->controller_regs + SCHIZO_ECC_CTRL);

	err_mask = (SCHIZO_PCICTRL_BUS_UNUS |
		    SCHIZO_PCICTRL_ESLCK |
		    SCHIZO_PCICTRL_TTO_ERR |
		    SCHIZO_PCICTRL_RTRY_ERR |
		    SCHIZO_PCICTRL_SBH_ERR |
		    SCHIZO_PCICTRL_SERR |
		    SCHIZO_PCICTRL_EEN);

	err_no_mask = (SCHIZO_PCICTRL_DTO_ERR |
		       SCHIZO_PCICTRL_SBH_INT);

	/*                                            
                      
  */
	tmp = upa_readq(pbm->pbm_regs + SCHIZO_PCI_CTRL);
	tmp |= err_mask;
	tmp &= ~err_no_mask;
	upa_writeq(tmp, pbm->pbm_regs + SCHIZO_PCI_CTRL);

	upa_writeq((SCHIZO_PCIAFSR_PMA | SCHIZO_PCIAFSR_PTA |
		    SCHIZO_PCIAFSR_PRTRY | SCHIZO_PCIAFSR_PPERR |
		    SCHIZO_PCIAFSR_PTTO | SCHIZO_PCIAFSR_PUNUS |
		    SCHIZO_PCIAFSR_SMA | SCHIZO_PCIAFSR_STA |
		    SCHIZO_PCIAFSR_SRTRY | SCHIZO_PCIAFSR_SPERR |
		    SCHIZO_PCIAFSR_STTO | SCHIZO_PCIAFSR_SUNUS),
		   pbm->pbm_regs + SCHIZO_PCI_AFSR);

	/*                                                       
                                             
  */
	err_mask = (BUS_ERROR_BADCMD | BUS_ERROR_SSMDIS |
		    BUS_ERROR_BADMA | BUS_ERROR_BADMB |
		    BUS_ERROR_BADMC |
		    BUS_ERROR_CPU1PS | BUS_ERROR_CPU1PB |
		    BUS_ERROR_CPU0PS | BUS_ERROR_CPU0PB |
		    BUS_ERROR_CIQTO |
		    BUS_ERROR_LPQTO | BUS_ERROR_SFPQTO |
		    BUS_ERROR_UFPQTO | BUS_ERROR_APERR |
		    BUS_ERROR_BUSERR | BUS_ERROR_TIMEOUT |
		    BUS_ERROR_ILL);
#if 1
	/*                                                
                                                 
                                                   
                                                  
                                                  
                                    
  */
	err_mask &= ~(BUS_ERROR_CPU1PS | BUS_ERROR_CPU1PB |
		      BUS_ERROR_CPU0PS | BUS_ERROR_CPU0PB);
#endif

	upa_writeq((SCHIZO_SAFERRCTRL_EN | err_mask),
		   pbm->controller_regs + SCHIZO_SAFARI_ERRCTRL);
}

static void pbm_config_busmastering(struct pci_pbm_info *pbm)
{
	u8 *addr;

	/*                                                  
                                       
  */
	addr = schizo_pci_config_mkaddr(pbm, pbm->pci_first_busno,
					0, PCI_CACHE_LINE_SIZE);
	pci_config_write8(addr, 64 / sizeof(u32));

	/*                                         */
	addr = schizo_pci_config_mkaddr(pbm, pbm->pci_first_busno,
					0, PCI_LATENCY_TIMER);
	pci_config_write8(addr, 64);
}

static void schizo_scan_bus(struct pci_pbm_info *pbm, struct device *parent)
{
	pbm_config_busmastering(pbm);
	pbm->is_66mhz_capable =
		(of_find_property(pbm->op->dev.of_node, "66mhz-capable", NULL)
		 != NULL);

	pbm->pci_bus = pci_scan_one_pbm(pbm, parent);

	if (pbm->chip_type == PBM_CHIP_TYPE_TOMATILLO)
		tomatillo_register_error_handlers(pbm);
	else
		schizo_register_error_handlers(pbm);
}

#define SCHIZO_STRBUF_CONTROL		(0x02800UL)
#define SCHIZO_STRBUF_FLUSH		(0x02808UL)
#define SCHIZO_STRBUF_FSYNC		(0x02810UL)
#define SCHIZO_STRBUF_CTXFLUSH		(0x02818UL)
#define SCHIZO_STRBUF_CTXMATCH		(0x10000UL)

static void schizo_pbm_strbuf_init(struct pci_pbm_info *pbm)
{
	unsigned long base = pbm->pbm_regs;
	u64 control;

	if (pbm->chip_type == PBM_CHIP_TYPE_TOMATILLO) {
		/*                                   */
		return;
	}

	/*                              */
	pbm->stc.strbuf_control		= base + SCHIZO_STRBUF_CONTROL;
	pbm->stc.strbuf_pflush		= base + SCHIZO_STRBUF_FLUSH;
	pbm->stc.strbuf_fsync		= base + SCHIZO_STRBUF_FSYNC;
	pbm->stc.strbuf_ctxflush	= base + SCHIZO_STRBUF_CTXFLUSH;
	pbm->stc.strbuf_ctxmatch_base	= base + SCHIZO_STRBUF_CTXMATCH;

	pbm->stc.strbuf_flushflag = (volatile unsigned long *)
		((((unsigned long)&pbm->stc.__flushflag_buf[0])
		  + 63UL)
		 & ~63UL);
	pbm->stc.strbuf_flushflag_pa = (unsigned long)
		__pa(pbm->stc.strbuf_flushflag);

	/*                                               
                                                
                               
  */
	control = upa_readq(pbm->stc.strbuf_control);
	control &= ~(SCHIZO_STRBUF_CTRL_LPTR |
		     SCHIZO_STRBUF_CTRL_LENAB |
		     SCHIZO_STRBUF_CTRL_DENAB);
	control |= SCHIZO_STRBUF_CTRL_ENAB;
	upa_writeq(control, pbm->stc.strbuf_control);

	pbm->stc.strbuf_enabled = 1;
}

#define SCHIZO_IOMMU_CONTROL		(0x00200UL)
#define SCHIZO_IOMMU_TSBBASE		(0x00208UL)
#define SCHIZO_IOMMU_FLUSH		(0x00210UL)
#define SCHIZO_IOMMU_CTXFLUSH		(0x00218UL)

static int schizo_pbm_iommu_init(struct pci_pbm_info *pbm)
{
	static const u32 vdma_default[] = { 0xc0000000, 0x40000000 };
	unsigned long i, tagbase, database;
	struct iommu *iommu = pbm->iommu;
	int tsbsize, err;
	const u32 *vdma;
	u32 dma_mask;
	u64 control;

	vdma = of_get_property(pbm->op->dev.of_node, "virtual-dma", NULL);
	if (!vdma)
		vdma = vdma_default;

	dma_mask = vdma[0];
	switch (vdma[1]) {
		case 0x20000000:
			dma_mask |= 0x1fffffff;
			tsbsize = 64;
			break;

		case 0x40000000:
			dma_mask |= 0x3fffffff;
			tsbsize = 128;
			break;

		case 0x80000000:
			dma_mask |= 0x7fffffff;
			tsbsize = 128;
			break;

		default:
			printk(KERN_ERR PFX "Strange virtual-dma size.\n");
			return -EINVAL;
	}

	/*                                                    */
	iommu->iommu_control  = pbm->pbm_regs + SCHIZO_IOMMU_CONTROL;
	iommu->iommu_tsbbase  = pbm->pbm_regs + SCHIZO_IOMMU_TSBBASE;
	iommu->iommu_flush    = pbm->pbm_regs + SCHIZO_IOMMU_FLUSH;
	iommu->iommu_tags     = iommu->iommu_flush + (0xa580UL - 0x0210UL);
	iommu->iommu_ctxflush = pbm->pbm_regs + SCHIZO_IOMMU_CTXFLUSH;

	/*                                                               
                        
  */
	iommu->write_complete_reg = pbm->controller_regs + 0x10000UL;

	/*
                           
  */
	control = upa_readq(iommu->iommu_control);
	control |= SCHIZO_IOMMU_CTRL_DENAB;
	upa_writeq(control, iommu->iommu_control);

	tagbase = SCHIZO_IOMMU_TAG, database = SCHIZO_IOMMU_DATA;

	for (i = 0; i < 16; i++) {
		upa_writeq(0, pbm->pbm_regs + tagbase + (i * 8UL));
		upa_writeq(0, pbm->pbm_regs + database + (i * 8UL));
	}

	/*                                               
                  
  */
	err = iommu_table_init(iommu, tsbsize * 8 * 1024, vdma[0], dma_mask,
			       pbm->numa_node);
	if (err) {
		printk(KERN_ERR PFX "iommu_table_init() fails with %d\n", err);
		return err;
	}

	upa_writeq(__pa(iommu->page_table), iommu->iommu_tsbbase);

	control = upa_readq(iommu->iommu_control);
	control &= ~(SCHIZO_IOMMU_CTRL_TSBSZ | SCHIZO_IOMMU_CTRL_TBWSZ);
	switch (tsbsize) {
	case 64:
		control |= SCHIZO_IOMMU_TSBSZ_64K;
		break;
	case 128:
		control |= SCHIZO_IOMMU_TSBSZ_128K;
		break;
	}

	control |= SCHIZO_IOMMU_CTRL_ENAB;
	upa_writeq(control, iommu->iommu_control);

	return 0;
}

#define SCHIZO_PCI_IRQ_RETRY	(0x1a00UL)
#define  SCHIZO_IRQ_RETRY_INF	 0xffUL

#define SCHIZO_PCI_DIAG			(0x2020UL)
#define  SCHIZO_PCIDIAG_D_BADECC	(1UL << 10UL) /*                                 */
#define  SCHIZO_PCIDIAG_D_BYPASS	(1UL <<  9UL) /*                                            */
#define  SCHIZO_PCIDIAG_D_TTO		(1UL <<  8UL) /*                                       */
#define  SCHIZO_PCIDIAG_D_RTRYARB	(1UL <<  7UL) /*                                    */
#define  SCHIZO_PCIDIAG_D_RETRY		(1UL <<  6UL) /*                                        */
#define  SCHIZO_PCIDIAG_D_INTSYNC	(1UL <<  5UL) /*                                                */
#define  SCHIZO_PCIDIAG_I_DMA_PARITY	(1UL <<  3UL) /*                                      */
#define  SCHIZO_PCIDIAG_I_PIOD_PARITY	(1UL <<  2UL) /*                                           */
#define  SCHIZO_PCIDIAG_I_PIOA_PARITY	(1UL <<  1UL) /*                                              */

#define TOMATILLO_PCI_IOC_CSR		(0x2248UL)
#define TOMATILLO_IOC_PART_WPENAB	0x0000000000080000UL
#define TOMATILLO_IOC_RDMULT_PENAB	0x0000000000040000UL
#define TOMATILLO_IOC_RDONE_PENAB	0x0000000000020000UL
#define TOMATILLO_IOC_RDLINE_PENAB	0x0000000000010000UL
#define TOMATILLO_IOC_RDMULT_PLEN	0x000000000000c000UL
#define TOMATILLO_IOC_RDMULT_PLEN_SHIFT	14UL
#define TOMATILLO_IOC_RDONE_PLEN	0x0000000000003000UL
#define TOMATILLO_IOC_RDONE_PLEN_SHIFT	12UL
#define TOMATILLO_IOC_RDLINE_PLEN	0x0000000000000c00UL
#define TOMATILLO_IOC_RDLINE_PLEN_SHIFT	10UL
#define TOMATILLO_IOC_PREF_OFF		0x00000000000003f8UL
#define TOMATILLO_IOC_PREF_OFF_SHIFT	3UL
#define TOMATILLO_IOC_RDMULT_CPENAB	0x0000000000000004UL
#define TOMATILLO_IOC_RDONE_CPENAB	0x0000000000000002UL
#define TOMATILLO_IOC_RDLINE_CPENAB	0x0000000000000001UL

#define TOMATILLO_PCI_IOC_TDIAG		(0x2250UL)
#define TOMATILLO_PCI_IOC_DDIAG		(0x2290UL)

static void schizo_pbm_hw_init(struct pci_pbm_info *pbm)
{
	u64 tmp;

	upa_writeq(5, pbm->pbm_regs + SCHIZO_PCI_IRQ_RETRY);

	tmp = upa_readq(pbm->pbm_regs + SCHIZO_PCI_CTRL);

	/*                                    */
	tmp |= 0xff;

	if (pbm->chip_type == PBM_CHIP_TYPE_TOMATILLO &&
	    pbm->chip_version >= 0x2)
		tmp |= 0x3UL << SCHIZO_PCICTRL_PTO_SHIFT;

	if (!of_find_property(pbm->op->dev.of_node, "no-bus-parking", NULL))
		tmp |= SCHIZO_PCICTRL_PARK;
	else
		tmp &= ~SCHIZO_PCICTRL_PARK;

	if (pbm->chip_type == PBM_CHIP_TYPE_TOMATILLO &&
	    pbm->chip_version <= 0x1)
		tmp |= SCHIZO_PCICTRL_DTO_INT;
	else
		tmp &= ~SCHIZO_PCICTRL_DTO_INT;

	if (pbm->chip_type == PBM_CHIP_TYPE_TOMATILLO)
		tmp |= (SCHIZO_PCICTRL_MRM_PREF |
			SCHIZO_PCICTRL_RDO_PREF |
			SCHIZO_PCICTRL_RDL_PREF);

	upa_writeq(tmp, pbm->pbm_regs + SCHIZO_PCI_CTRL);

	tmp = upa_readq(pbm->pbm_regs + SCHIZO_PCI_DIAG);
	tmp &= ~(SCHIZO_PCIDIAG_D_RTRYARB |
		 SCHIZO_PCIDIAG_D_RETRY |
		 SCHIZO_PCIDIAG_D_INTSYNC);
	upa_writeq(tmp, pbm->pbm_regs + SCHIZO_PCI_DIAG);

	if (pbm->chip_type == PBM_CHIP_TYPE_TOMATILLO) {
		/*                                              
                
   */
		tmp = (TOMATILLO_IOC_PART_WPENAB |
		       (1 << TOMATILLO_IOC_PREF_OFF_SHIFT) |
		       TOMATILLO_IOC_RDMULT_CPENAB |
		       TOMATILLO_IOC_RDONE_CPENAB |
		       TOMATILLO_IOC_RDLINE_CPENAB);

		upa_writeq(tmp, pbm->pbm_regs + TOMATILLO_PCI_IOC_CSR);
	}
}

static int schizo_pbm_init(struct pci_pbm_info *pbm,
			   struct platform_device *op, u32 portid,
			   int chip_type)
{
	const struct linux_prom64_registers *regs;
	struct device_node *dp = op->dev.of_node;
	const char *chipset_name;
	int err;

	switch (chip_type) {
	case PBM_CHIP_TYPE_TOMATILLO:
		chipset_name = "TOMATILLO";
		break;

	case PBM_CHIP_TYPE_SCHIZO_PLUS:
		chipset_name = "SCHIZO+";
		break;

	case PBM_CHIP_TYPE_SCHIZO:
	default:
		chipset_name = "SCHIZO";
		break;
	}

	/*                            
                          
                                                            
                           
   
                                 
                          
                                          
                           
                 
  */
	regs = of_get_property(dp, "reg", NULL);

	pbm->next = pci_pbm_root;
	pci_pbm_root = pbm;

	pbm->numa_node = -1;

	pbm->pci_ops = &sun4u_pci_ops;
	pbm->config_space_reg_bits = 8;

	pbm->index = pci_num_pbms++;

	pbm->portid = portid;
	pbm->op = op;

	pbm->chip_type = chip_type;
	pbm->chip_version = of_getintprop_default(dp, "version#", 0);
	pbm->chip_revision = of_getintprop_default(dp, "module-version#", 0);

	pbm->pbm_regs = regs[0].phys_addr;
	pbm->controller_regs = regs[1].phys_addr - 0x10000UL;

	if (chip_type == PBM_CHIP_TYPE_TOMATILLO)
		pbm->sync_reg = regs[3].phys_addr + 0x1a18UL;

	pbm->name = dp->full_name;

	printk("%s: %s PCI Bus Module ver[%x:%x]\n",
	       pbm->name, chipset_name,
	       pbm->chip_version, pbm->chip_revision);

	schizo_pbm_hw_init(pbm);

	pci_determine_mem_io_space(pbm);

	pci_get_pbm_props(pbm);

	err = schizo_pbm_iommu_init(pbm);
	if (err)
		return err;

	schizo_pbm_strbuf_init(pbm);

	schizo_scan_bus(pbm, &op->dev);

	return 0;
}

static inline int portid_compare(u32 x, u32 y, int chip_type)
{
	if (chip_type == PBM_CHIP_TYPE_TOMATILLO) {
		if (x == (y ^ 1))
			return 1;
		return 0;
	}
	return (x == y);
}

static struct pci_pbm_info *schizo_find_sibling(u32 portid, int chip_type)
{
	struct pci_pbm_info *pbm;

	for (pbm = pci_pbm_root; pbm; pbm = pbm->next) {
		if (portid_compare(pbm->portid, portid, chip_type))
			return pbm;
	}
	return NULL;
}

static int __schizo_init(struct platform_device *op, unsigned long chip_type)
{
	struct device_node *dp = op->dev.of_node;
	struct pci_pbm_info *pbm;
	struct iommu *iommu;
	u32 portid;
	int err;

	portid = of_getintprop_default(dp, "portid", 0xff);

	err = -ENOMEM;
	pbm = kzalloc(sizeof(*pbm), GFP_KERNEL);
	if (!pbm) {
		printk(KERN_ERR PFX "Cannot allocate pci_pbm_info.\n");
		goto out_err;
	}

	pbm->sibling = schizo_find_sibling(portid, chip_type);

	iommu = kzalloc(sizeof(struct iommu), GFP_KERNEL);
	if (!iommu) {
		printk(KERN_ERR PFX "Cannot allocate PBM A iommu.\n");
		goto out_free_pbm;
	}

	pbm->iommu = iommu;

	if (schizo_pbm_init(pbm, op, portid, chip_type))
		goto out_free_iommu;

	if (pbm->sibling)
		pbm->sibling->sibling = pbm;

	dev_set_drvdata(&op->dev, pbm);

	return 0;

out_free_iommu:
	kfree(pbm->iommu);

out_free_pbm:
	kfree(pbm);

out_err:
	return err;
}

static const struct of_device_id schizo_match[];
static int schizo_probe(struct platform_device *op)
{
	const struct of_device_id *match;

	match = of_match_device(schizo_match, &op->dev);
	if (!match)
		return -EINVAL;
	return __schizo_init(op, (unsigned long)match->data);
}

/*                                                              
                                                                 
                                                                
         
 */
static const struct of_device_id schizo_match[] = {
	{
		.name = "pci",
		.compatible = "pci108e,a801",
		.data = (void *) PBM_CHIP_TYPE_TOMATILLO,
	},
	{
		.name = "pci",
		.compatible = "pci108e,8002",
		.data = (void *) PBM_CHIP_TYPE_SCHIZO_PLUS,
	},
	{
		.name = "pci",
		.compatible = "pci108e,8001",
		.data = (void *) PBM_CHIP_TYPE_SCHIZO,
	},
	{},
};

static struct platform_driver schizo_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
		.of_match_table = schizo_match,
	},
	.probe		= schizo_probe,
};

static int __init schizo_init(void)
{
	return platform_driver_register(&schizo_driver);
}

subsys_initcall(schizo_init);
