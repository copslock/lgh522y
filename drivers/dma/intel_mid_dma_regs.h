/*
 *  intel_mid_dma_regs.h - Intel MID DMA Drivers
 *
 *  Copyright (C) 2008-10 Intel Corp
 *  Author: Vinod Koul <vinod.koul@intel.com>
 *  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *
 */
#ifndef __INTEL_MID_DMAC_REGS_H__
#define __INTEL_MID_DMAC_REGS_H__

#include <linux/dmaengine.h>
#include <linux/dmapool.h>
#include <linux/pci_ids.h>

#define INTEL_MID_DMA_DRIVER_VERSION "1.1.0"

#define	REG_BIT0		0x00000001
#define	REG_BIT8		0x00000100
#define INT_MASK_WE		0x8
#define CLEAR_DONE		0xFFFFEFFF
#define UNMASK_INTR_REG(chan_num) \
	((REG_BIT0 << chan_num) | (REG_BIT8 << chan_num))
#define MASK_INTR_REG(chan_num) (REG_BIT8 << chan_num)

#define ENABLE_CHANNEL(chan_num) \
	((REG_BIT0 << chan_num) | (REG_BIT8 << chan_num))

#define DISABLE_CHANNEL(chan_num) \
	(REG_BIT8 << chan_num)

#define DESCS_PER_CHANNEL	16
/*             */
/*                                             */
#define DMA_REG_SIZE		0x400
#define DMA_CH_SIZE		0x58

/*                                    */
#define SAR			0x00 /*                        */
#define DAR			0x08 /*                             */
#define LLP			0x10 /*                             */
#define CTL_LOW			0x18 /*                 */
#define CTL_HIGH		0x1C /*                 */
#define CFG_LOW			0x40 /*                           */
#define CFG_HIGH		0x44 /*                            */

#define STATUS_TFR		0x2E8
#define STATUS_BLOCK		0x2F0
#define STATUS_ERR		0x308

#define RAW_TFR			0x2C0
#define RAW_BLOCK		0x2C8
#define RAW_ERR			0x2E0

#define MASK_TFR		0x310
#define MASK_BLOCK		0x318
#define MASK_SRC_TRAN		0x320
#define MASK_DST_TRAN		0x328
#define MASK_ERR		0x330

#define CLEAR_TFR		0x338
#define CLEAR_BLOCK		0x340
#define CLEAR_SRC_TRAN		0x348
#define CLEAR_DST_TRAN		0x350
#define CLEAR_ERR		0x358

#define INTR_STATUS		0x360
#define DMA_CFG			0x398
#define DMA_CHAN_EN		0x3A0

/*                             */
union intel_mid_dma_ctl_lo {
	struct {
		u32	int_en:1;	/*                            */
					/*           */
		u32	dst_tr_width:3;	/*                          */
					/*                     */
		u32	src_tr_width:3; /*                     */
					/*                     */
		u32	dinc:2;		/*                           */
					/*                                  */
		u32	sinc:2;		/*                                   */
		u32	dst_msize:3;	/*                                    */
					/*                  */
		u32	src_msize:3;	/*                               */
					/*                  */
		u32	reser1:3;
		u32	tt_fc:3;	/*                                 */
					/*         
                
                */
		u32	dms:2;		/*                             */
		u32	sms:2;		/*                        */
		u32	llp_dst_en:1;	/*                                  */
		u32	llp_src_en:1;	/*                             */
		u32	reser2:3;
	} ctlx;
	u32	ctl_lo;
};

union intel_mid_dma_ctl_hi {
	struct {
		u32	block_ts:12;	/*                   */
		u32	done:1;		/*                      */
		u32	reser:19;	/*                  */
	} ctlx;
	u32	ctl_hi;

};

/*                                   */
union intel_mid_dma_cfg_lo {
	struct {
		u32	reser1:5;
		u32	ch_prior:3;	/*                    */
		u32	ch_susp:1;	/*                   */
		u32	fifo_empty:1;	/*                           */
		u32	hs_sel_dst:1;	/*                              */
					/*              */
		u32	hs_sel_src:1;	/*                            */
		u32	reser2:6;
		u32	dst_hs_pol:1;	/*                          */
		u32	src_hs_pol:1;	/*                         */
		u32	max_abrst:10;	/*                                   */
		u32	reload_src:1;	/*                                   */
		u32	reload_dst:1;	/*                             */
	} cfgx;
	u32	cfg_lo;
};

union intel_mid_dma_cfg_hi {
	struct {
		u32	fcmode:1;	/*                     */
		u32	fifo_mode:1;	/*                    */
		u32	protctl:3;	/*                      */
		u32	rsvd:2;
		u32	src_per:4;	/*                   */
		u32	dst_per:4;	/*                    */
		u32	reser2:17;
	} cfgx;
	u32	cfg_hi;
};


/* 
                                                                           
                                                      
                                                            
                                                         
                         
                          
                                           
                                        
                                       
                              
                                                          
                                     
                                                             
                                                    
                                       
                                           
 */
struct intel_mid_dma_chan {
	struct dma_chan		chan;
	void __iomem		*ch_regs;
	void __iomem		*dma_base;
	int			ch_id;
	spinlock_t		lock;
	struct list_head	active_list;
	struct list_head	queue;
	struct list_head	free_list;
	unsigned int		descs_allocated;
	struct middma_device	*dma;
	bool			busy;
	bool			in_use;
	u32			raw_tfr;
	u32			raw_block;
	struct intel_mid_dma_slave *mid_slave;
};

static inline struct intel_mid_dma_chan *to_intel_mid_dma_chan(
						struct dma_chan *chan)
{
	return container_of(chan, struct intel_mid_dma_chan, chan);
}

enum intel_mid_dma_state {
	RUNNING = 0,
	SUSPENDED,
};
/* 
                                                                 
                    
                                                
                                            
                                      
                                                  
                        
                             
                                        
                                              
                                                    
                                                         
                                                                    
                                                                         
                              
 */
struct middma_device {
	struct pci_dev		*pdev;
	void __iomem		*dma_base;
	struct pci_pool		*dma_pool;
	struct dma_device	common;
	struct tasklet_struct   tasklet;
	struct intel_mid_dma_chan ch[MAX_CHAN];
	unsigned int		pci_id;
	unsigned int		intr_mask;
	void __iomem		*mask_reg;
	int			chan_base;
	int			max_chan;
	int			block_size;
	unsigned int		pimr_mask;
	enum intel_mid_dma_state state;
};

static inline struct middma_device *to_middma_device(struct dma_device *common)
{
	return container_of(common, struct middma_device, common);
}

struct intel_mid_dma_desc {
	void __iomem			*block; /*      */
	struct list_head		desc_node;
	struct dma_async_tx_descriptor	txd;
	size_t				len;
	dma_addr_t			sar;
	dma_addr_t			dar;
	u32				cfg_hi;
	u32				cfg_lo;
	u32				ctl_lo;
	u32				ctl_hi;
	struct pci_pool			*lli_pool;
	struct intel_mid_dma_lli	*lli;
	dma_addr_t			lli_phys;
	unsigned int			lli_length;
	unsigned int			current_lli;
	dma_addr_t			next;
	enum dma_transfer_direction		dirn;
	enum dma_status			status;
	enum dma_slave_buswidth		width; /*                */
	enum intel_mid_dma_mode		cfg_mode; /*                  */

};

struct intel_mid_dma_lli {
	dma_addr_t			sar;
	dma_addr_t			dar;
	dma_addr_t			llp;
	u32				ctl_lo;
	u32				ctl_hi;
} __attribute__ ((packed));

static inline int test_ch_en(void __iomem *dma, u32 ch_no)
{
	u32 en_reg = ioread32(dma + DMA_CHAN_EN);
	return (en_reg >> ch_no) & 0x1;
}

static inline struct intel_mid_dma_desc *to_intel_mid_dma_desc
		(struct dma_async_tx_descriptor *txd)
{
	return container_of(txd, struct intel_mid_dma_desc, txd);
}

static inline struct intel_mid_dma_slave *to_intel_mid_dma_slave
		(struct dma_slave_config *slave)
{
	return container_of(slave, struct intel_mid_dma_slave, dma_slave);
}


int dma_resume(struct device *dev);

#endif /*                         */
