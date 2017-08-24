/*
 * Driver for the Synopsys DesignWare DMA Controller (aka DMACA on
 * AVR32 systems.)
 *
 * Copyright (C) 2007 Atmel Corporation
 * Copyright (C) 2010-2011 ST Microelectronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef DW_DMAC_H
#define DW_DMAC_H

#include <linux/dmaengine.h>

/* 
                                                                      
  
                                                    
                                                                     
                                                                
                                                                 
                                                                 
                                                              
                                                               
 */
struct dw_dma_slave {
	struct device		*dma_dev;
	u32			cfg_hi;
	u32			cfg_lo;
	u8			src_master;
	u8			dst_master;
};

/* 
                                                                    
                                                                 
                                                                           
                                                
                                                                 
                                                                         
                                                              
                                                                 
                                                                       
                                             
                                                          
                                                    
 */
struct dw_dma_platform_data {
	unsigned int	nr_channels;
	bool		is_private;
#define CHAN_ALLOCATION_ASCENDING	0	/*               */
#define CHAN_ALLOCATION_DESCENDING	1	/*               */
	unsigned char	chan_allocation_order;
#define CHAN_PRIORITY_ASCENDING		0	/*               */
#define CHAN_PRIORITY_DESCENDING	1	/*               */
	unsigned char	chan_priority;
	unsigned short	block_size;
	unsigned char	nr_masters;
	unsigned char	data_width[4];
};

/*             */
enum dw_dma_msize {
	DW_DMA_MSIZE_1,
	DW_DMA_MSIZE_4,
	DW_DMA_MSIZE_8,
	DW_DMA_MSIZE_16,
	DW_DMA_MSIZE_32,
	DW_DMA_MSIZE_64,
	DW_DMA_MSIZE_128,
	DW_DMA_MSIZE_256,
};

/*                                      */
#define DWC_CFGH_FCMODE		(1 << 0)
#define DWC_CFGH_FIFO_MODE	(1 << 1)
#define DWC_CFGH_PROTCTL(x)	((x) << 2)
#define DWC_CFGH_SRC_PER(x)	((x) << 7)
#define DWC_CFGH_DST_PER(x)	((x) << 11)

/*                                      */
#define DWC_CFGL_LOCK_CH_XFER	(0 << 12)	/*                  */
#define DWC_CFGL_LOCK_CH_BLOCK	(1 << 12)
#define DWC_CFGL_LOCK_CH_XACT	(2 << 12)
#define DWC_CFGL_LOCK_BUS_XFER	(0 << 14)	/*                   */
#define DWC_CFGL_LOCK_BUS_BLOCK	(1 << 14)
#define DWC_CFGL_LOCK_BUS_XACT	(2 << 14)
#define DWC_CFGL_LOCK_CH	(1 << 15)	/*                 */
#define DWC_CFGL_LOCK_BUS	(1 << 16)	/*                   */
#define DWC_CFGL_HS_DST_POL	(1 << 18)	/*                          */
#define DWC_CFGL_HS_SRC_POL	(1 << 19)	/*                          */

/*                    */
struct dw_cyclic_desc {
	struct dw_desc	**desc;
	unsigned long	periods;
	void		(*period_callback)(void *param);
	void		*period_callback_param;
};

struct dw_cyclic_desc *dw_dma_cyclic_prep(struct dma_chan *chan,
		dma_addr_t buf_addr, size_t buf_len, size_t period_len,
		enum dma_transfer_direction direction);
void dw_dma_cyclic_free(struct dma_chan *chan);
int dw_dma_cyclic_start(struct dma_chan *chan);
void dw_dma_cyclic_stop(struct dma_chan *chan);

dma_addr_t dw_dma_get_src_addr(struct dma_chan *chan);

dma_addr_t dw_dma_get_dst_addr(struct dma_chan *chan);

#endif /*           */
