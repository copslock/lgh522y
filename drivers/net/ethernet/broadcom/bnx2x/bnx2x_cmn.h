/* bnx2x_cmn.h: Broadcom Everest network driver.
 *
 * Copyright (c) 2007-2013 Broadcom Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 *
 * Maintained by: Eilon Greenstein <eilong@broadcom.com>
 * Written by: Eliezer Tamir
 * Based on code from Michael Chan's bnx2 driver
 * UDP CSUM errata workaround by Arik Gendelman
 * Slowpath and fastpath rework by Vladislav Zolotarov
 * Statistics and Link management by Yitchak Gertner
 *
 */
#ifndef BNX2X_CMN_H
#define BNX2X_CMN_H

#include <linux/types.h>
#include <linux/pci.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>


#include "bnx2x.h"
#include "bnx2x_sriov.h"

/*                                                              */
extern int load_count[2][3]; /*                                      */

extern int num_queues;
extern int int_mode;

/*                                                              */
#define BNX2X_PCI_FREE(x, y, size) \
	do { \
		if (x) { \
			dma_free_coherent(&bp->pdev->dev, size, (void *)x, y); \
			x = NULL; \
			y = 0; \
		} \
	} while (0)

#define BNX2X_FREE(x) \
	do { \
		if (x) { \
			kfree((void *)x); \
			x = NULL; \
		} \
	} while (0)

#define BNX2X_PCI_ALLOC(x, y, size)				\
do {								\
	x = dma_alloc_coherent(&bp->pdev->dev, size, y,		\
			       GFP_KERNEL | __GFP_ZERO);	\
	if (x == NULL)						\
		goto alloc_mem_err;				\
} while (0)

#define BNX2X_ALLOC(x, size) \
	do { \
		x = kzalloc(size, GFP_KERNEL); \
		if (x == NULL) \
			goto alloc_mem_err; \
	} while (0)

/*                                                              
                                                                
 */
/*      */

/* 
                                                            
  
                       
                                                 
  
                                                                
 */
u32 bnx2x_send_unload_req(struct bnx2x *bp, int unload_mode);

/* 
                                                                
  
                      
                                               
 */
void bnx2x_send_unload_done(struct bnx2x *bp, bool keep_link);

/* 
                                                          
  
                       
                               
                                              
                                                         
 */
int bnx2x_config_rss_pf(struct bnx2x *bp, struct bnx2x_rss_config_obj *rss_obj,
			bool config_hash);

/* 
                                              
  
                       
  
                                                       
                                                       
             
 */
void bnx2x__init_func_obj(struct bnx2x *bp);

/* 
                                       
  
                      
                                          
                    
  
 */
int bnx2x_setup_queue(struct bnx2x *bp, struct bnx2x_fastpath *fp,
		       bool leading);

/* 
                                                      
  
                      
 */
int bnx2x_setup_leading(struct bnx2x *bp);

/* 
                                            
  
                      
                    
                              
  
                               
 */
u32 bnx2x_fw_command(struct bnx2x *bp, u32 command, u32 param);

/* 
                                                                           
  
                      
                           
 */
int bnx2x_initial_phy_init(struct bnx2x *bp, int load_mode);

/* 
                                                                        
  
                      
 */
void bnx2x_link_set(struct bnx2x *bp);

/* 
                                                              
                    
  
                      
 */
void bnx2x_force_link_reset(struct bnx2x *bp);

/* 
                                       
  
                      
                   
  
                           
 */
u8 bnx2x_link_test(struct bnx2x *bp, u8 is_serdes);

/* 
                                                
  
                      
  
                                                                
                
 */
void bnx2x_drv_pulse(struct bnx2x *bp);

/* 
                                                      
  
                      
                    
                       
                   
                     
                                 
 */
void bnx2x_igu_ack_sb(struct bnx2x *bp, u8 igu_sb_id, u8 segment,
		      u16 index, u8 op, u8 update);

/*                                        */
void bnx2x_pf_disable(struct bnx2x *bp);
int bnx2x_pretend_func(struct bnx2x *bp, u16 pretend_func_val);

/* 
                                                          
  
                      
 */
void bnx2x__link_status_update(struct bnx2x *bp);

/* 
                                                         
  
                      
 */
void bnx2x_link_report(struct bnx2x *bp);

/*                                            */
void __bnx2x_link_report(struct bnx2x *bp);

/* 
                                           
  
                      
  
                                                             
 */
u16 bnx2x_get_mf_speed(struct bnx2x *bp);

/* 
                                                       
  
                    
                                  
 */
irqreturn_t bnx2x_msix_sp_int(int irq, void *dev_instance);

/* 
                                                
  
                    
                                  
 */
irqreturn_t bnx2x_interrupt(int irq, void *dev_instance);

/* 
                                                  
  
                      
                
 */
int bnx2x_cnic_notify(struct bnx2x *bp, int cmd);

/* 
                                                                 
  
                      
 */
void bnx2x_setup_cnic_irq_info(struct bnx2x *bp);

/* 
                                                          
  
                      
 */
void bnx2x_setup_cnic_info(struct bnx2x *bp);

/* 
                                           
  
                      
 */
void bnx2x_int_enable(struct bnx2x *bp);

/* 
                                               
  
                      
                                            
  
                                          
                                                          
 */
void bnx2x_int_disable_sync(struct bnx2x *bp, int disable_hw);

/* 
                                                        
  
                      
                                       
  
               
           
                   
          
 */
void bnx2x_nic_init_cnic(struct bnx2x *bp);

/* 
                                                 
  
                      
  
               
                     
                    
        
 */
void bnx2x_pre_irq_nic_init(struct bnx2x *bp);

/* 
                                                  
  
                      
                                       
  
               
                   
                    
          
 */
void bnx2x_post_irq_nic_init(struct bnx2x *bp, u32 load_code);
/* 
                                                            
  
                      
 */
int bnx2x_alloc_mem_cnic(struct bnx2x *bp);
/* 
                                              
  
                      
 */
int bnx2x_alloc_mem(struct bnx2x *bp);

/* 
                                                          
  
                      
 */
void bnx2x_free_mem_cnic(struct bnx2x *bp);
/* 
                                            
  
                      
 */
void bnx2x_free_mem(struct bnx2x *bp);

/* 
                                                                 
  
                      
 */
void bnx2x_set_num_queues(struct bnx2x *bp);

/* 
                                               
  
                       
                                       
                                                
  
                               
                    
         
 */
void bnx2x_chip_cleanup(struct bnx2x *bp, int unload_mode, bool keep_link);

/* 
                                           
  
                      
                                           
 */
int bnx2x_acquire_hw_lock(struct bnx2x *bp, u32 resource);

/* 
                                           
  
                      
                                           
 */
int bnx2x_release_hw_lock(struct bnx2x *bp, u32 resource);

/* 
                                                           
  
                      
 */
int bnx2x_release_leader_lock(struct bnx2x *bp);

/* 
                                                          
  
                      
                     
  
                                                         
 */
int bnx2x_set_eth_mac(struct bnx2x *bp, bool set);

/* 
                                                        
  
                  
  
                                             
                                              
                       
 */
void bnx2x_set_rx_mode(struct net_device *dev);

/* 
                                                                   
  
                      
  
                                              
                        
 */
int bnx2x_set_storm_rx_mode(struct bnx2x *bp);

/* 
                                                               
  
                       
                     
                                        
                                            
                                                        
                                      
 */
int bnx2x_set_q_rx_mode(struct bnx2x *bp, u8 cl_id,
			unsigned long rx_mode_flags,
			unsigned long rx_accept_flags,
			unsigned long tx_accept_flags,
			unsigned long ramrod_flags);

/*                       */
void bnx2x_set_pf_load(struct bnx2x *bp);
bool bnx2x_clear_pf_load(struct bnx2x *bp);
bool bnx2x_chk_parity_attn(struct bnx2x *bp, bool *global, bool print);
bool bnx2x_reset_is_done(struct bnx2x *bp, int engine);
void bnx2x_set_reset_in_progress(struct bnx2x *bp);
void bnx2x_set_reset_global(struct bnx2x *bp);
void bnx2x_disable_close_the_gate(struct bnx2x *bp);
int bnx2x_init_hw_func_cnic(struct bnx2x *bp);

/* 
                                              
  
                                      
                      
 */
void bnx2x_sp_event(struct bnx2x_fastpath *fp, union eth_rx_cqe *rr_cqe);

/* 
                                                   
  
                      
 */
void bnx2x_ilt_set_info(struct bnx2x *bp);

/* 
                                                               
          
  
                      
 */
void bnx2x_ilt_set_info_cnic(struct bnx2x *bp);

/* 
                                              
  
                      
 */
void bnx2x_dcbx_init(struct bnx2x *bp, bool update_shmem);

/* 
                                                                  
  
                      
                                     
  
                                             
 */
int bnx2x_set_power_state(struct bnx2x *bp, pci_power_t state);

/* 
                                                                          
  
                      
                    
 */
void bnx2x_update_max_mf_config(struct bnx2x *bp, u32 value);
/*                */
void bnx2x_fw_dump_lvl(struct bnx2x *bp, const char *lvl);

/*                               */
bool bnx2x_test_firmware_version(struct bnx2x *bp, bool is_err);

/*                      */
int bnx2x_nic_unload(struct bnx2x *bp, int unload_mode, bool keep_link);

/*                     */
int bnx2x_nic_load(struct bnx2x *bp, int load_mode);

/*                    */
netdev_tx_t bnx2x_start_xmit(struct sk_buff *skb, struct net_device *dev);

/*                   */
int bnx2x_setup_tc(struct net_device *dev, u8 num_tc);

int bnx2x_get_vf_config(struct net_device *dev, int vf,
			struct ifla_vf_info *ivi);
int bnx2x_set_vf_mac(struct net_device *dev, int queue, u8 *mac);
int bnx2x_set_vf_vlan(struct net_device *netdev, int vf, u16 vlan, u8 qos);

/*                       */
u16 bnx2x_select_queue(struct net_device *dev, struct sk_buff *skb);

static inline void bnx2x_update_rx_prod(struct bnx2x *bp,
					struct bnx2x_fastpath *fp,
					u16 bd_prod, u16 rx_comp_prod,
					u16 rx_sge_prod)
{
	struct ustorm_eth_rx_producers rx_prods = {0};
	u32 i;

	/*                  */
	rx_prods.bd_prod = bd_prod;
	rx_prods.cqe_prod = rx_comp_prod;
	rx_prods.sge_prod = rx_sge_prod;

	/*                                                                  
                                                                     
               
                                                                    
                                                                   
                                  
  */
	wmb();

	for (i = 0; i < sizeof(rx_prods)/4; i++)
		REG_WR(bp, fp->ustorm_rx_prods_offset + i*4,
		       ((u32 *)&rx_prods)[i]);

	mmiowb(); /*                           */

	DP(NETIF_MSG_RX_STATUS,
	   "queue[%d]:  wrote  bd_prod %u  cqe_prod %u  sge_prod %u\n",
	   fp->index, bd_prod, rx_comp_prod, rx_sge_prod);
}

/*               */
int bnx2x_reload_if_running(struct net_device *dev);

int bnx2x_change_mac_addr(struct net_device *dev, void *p);

/*                   */
int bnx2x_rx_int(struct bnx2x_fastpath *fp, int budget);

/*                   */
int bnx2x_tx_int(struct bnx2x *bp, struct bnx2x_fp_txdata *txdata);

/*                          */
int bnx2x_suspend(struct pci_dev *pdev, pm_message_t state);
int bnx2x_resume(struct pci_dev *pdev);

/*                     */
void bnx2x_free_irq(struct bnx2x *bp);

void bnx2x_free_fp_mem_cnic(struct bnx2x *bp);
void bnx2x_free_fp_mem(struct bnx2x *bp);
int bnx2x_alloc_fp_mem_cnic(struct bnx2x *bp);
int bnx2x_alloc_fp_mem(struct bnx2x *bp);
void bnx2x_init_rx_rings(struct bnx2x *bp);
void bnx2x_init_rx_rings_cnic(struct bnx2x *bp);
void bnx2x_free_skbs_cnic(struct bnx2x *bp);
void bnx2x_free_skbs(struct bnx2x *bp);
void bnx2x_netif_stop(struct bnx2x *bp, int disable_hw);
void bnx2x_netif_start(struct bnx2x *bp);
int bnx2x_load_cnic(struct bnx2x *bp);

/* 
                                              
  
                      
  
                                                         
                                            
 */
int bnx2x_enable_msix(struct bnx2x *bp);

/* 
                                                                             
  
                      
 */
int bnx2x_enable_msi(struct bnx2x *bp);

/* 
                             
  
                        
           
  
 */
int bnx2x_poll(struct napi_struct *napi, int budget);

/* 
                                                                       
  
                      
 */
int bnx2x_alloc_mem_bp(struct bnx2x *bp);

/* 
                                                                     
  
                      
 */
void bnx2x_free_mem_bp(struct bnx2x *bp);

/* 
                                                
  
                   
                          
  
 */
int bnx2x_change_mtu(struct net_device *dev, int new_mtu);

#ifdef NETDEV_FCOE_WWNN
/* 
                                                                    
  
                   
                      
                                                                      
  
 */
int bnx2x_fcoe_get_wwn(struct net_device *dev, u64 *wwn, int type);
#endif

netdev_features_t bnx2x_fix_features(struct net_device *dev,
				     netdev_features_t features);
int bnx2x_set_features(struct net_device *dev, netdev_features_t features);

/* 
                                                
  
                   
 */
void bnx2x_tx_timeout(struct net_device *dev);

/*                                                                */
/*                                                                */
static inline void bnx2x_update_fpsb_idx(struct bnx2x_fastpath *fp)
{
	barrier(); /*                                        */
	fp->fp_hc_idx = fp->sb_running_index[SM_RX_ID];
}

static inline void bnx2x_igu_ack_sb_gen(struct bnx2x *bp, u8 igu_sb_id,
					u8 segment, u16 index, u8 op,
					u8 update, u32 igu_addr)
{
	struct igu_regular cmd_data = {0};

	cmd_data.sb_id_and_flags =
			((index << IGU_REGULAR_SB_INDEX_SHIFT) |
			 (segment << IGU_REGULAR_SEGMENT_ACCESS_SHIFT) |
			 (update << IGU_REGULAR_BUPDATE_SHIFT) |
			 (op << IGU_REGULAR_ENABLE_INT_SHIFT));

	DP(NETIF_MSG_INTR, "write 0x%08x to IGU addr 0x%x\n",
	   cmd_data.sb_id_and_flags, igu_addr);
	REG_WR(bp, igu_addr, cmd_data.sb_id_and_flags);

	/*                               */
	mmiowb();
	barrier();
}

static inline void bnx2x_hc_ack_sb(struct bnx2x *bp, u8 sb_id,
				   u8 storm, u16 index, u8 op, u8 update)
{
	u32 hc_addr = (HC_REG_COMMAND_REG + BP_PORT(bp)*32 +
		       COMMAND_REG_INT_ACK);
	struct igu_ack_register igu_ack;

	igu_ack.status_block_index = index;
	igu_ack.sb_id_and_flags =
			((sb_id << IGU_ACK_REGISTER_STATUS_BLOCK_ID_SHIFT) |
			 (storm << IGU_ACK_REGISTER_STORM_ID_SHIFT) |
			 (update << IGU_ACK_REGISTER_UPDATE_INDEX_SHIFT) |
			 (op << IGU_ACK_REGISTER_INTERRUPT_MODE_SHIFT));

	REG_WR(bp, hc_addr, (*(u32 *)&igu_ack));

	/*                               */
	mmiowb();
	barrier();
}

static inline void bnx2x_ack_sb(struct bnx2x *bp, u8 igu_sb_id, u8 storm,
				u16 index, u8 op, u8 update)
{
	if (bp->common.int_block == INT_BLOCK_HC)
		bnx2x_hc_ack_sb(bp, igu_sb_id, storm, index, op, update);
	else {
		u8 segment;

		if (CHIP_INT_MODE_IS_BC(bp))
			segment = storm;
		else if (igu_sb_id != bp->igu_dsb_id)
			segment = IGU_SEG_ACCESS_DEF;
		else if (storm == ATTENTION_ID)
			segment = IGU_SEG_ACCESS_ATTN;
		else
			segment = IGU_SEG_ACCESS_DEF;
		bnx2x_igu_ack_sb(bp, igu_sb_id, segment, index, op, update);
	}
}

static inline u16 bnx2x_hc_ack_int(struct bnx2x *bp)
{
	u32 hc_addr = (HC_REG_COMMAND_REG + BP_PORT(bp)*32 +
		       COMMAND_REG_SIMD_MASK);
	u32 result = REG_RD(bp, hc_addr);

	barrier();
	return result;
}

static inline u16 bnx2x_igu_ack_int(struct bnx2x *bp)
{
	u32 igu_addr = (BAR_IGU_INTMEM + IGU_REG_SISR_MDPC_WMASK_LSB_UPPER*8);
	u32 result = REG_RD(bp, igu_addr);

	DP(NETIF_MSG_INTR, "read 0x%08x from IGU addr 0x%x\n",
	   result, igu_addr);

	barrier();
	return result;
}

static inline u16 bnx2x_ack_int(struct bnx2x *bp)
{
	barrier();
	if (bp->common.int_block == INT_BLOCK_HC)
		return bnx2x_hc_ack_int(bp);
	else
		return bnx2x_igu_ack_int(bp);
}

static inline int bnx2x_has_tx_work_unload(struct bnx2x_fp_txdata *txdata)
{
	/*                                                     */
	barrier();
	return txdata->tx_pkt_prod != txdata->tx_pkt_cons;
}

static inline u16 bnx2x_tx_avail(struct bnx2x *bp,
				 struct bnx2x_fp_txdata *txdata)
{
	s16 used;
	u16 prod;
	u16 cons;

	prod = txdata->tx_bd_prod;
	cons = txdata->tx_bd_cons;

	used = SUB_S16(prod, cons);

#ifdef BNX2X_STOP_ON_ERROR
	WARN_ON(used < 0);
	WARN_ON(used > txdata->tx_ring_size);
	WARN_ON((txdata->tx_ring_size - used) > MAX_TX_AVAIL);
#endif

	return (s16)(txdata->tx_ring_size) - used;
}

static inline int bnx2x_tx_queue_has_work(struct bnx2x_fp_txdata *txdata)
{
	u16 hw_cons;

	/*                                                   */
	barrier();
	hw_cons = le16_to_cpu(*txdata->tx_cons_sb);
	return hw_cons != txdata->tx_pkt_cons;
}

static inline bool bnx2x_has_tx_work(struct bnx2x_fastpath *fp)
{
	u8 cos;
	for_each_cos_in_tx_queue(fp, cos)
		if (bnx2x_tx_queue_has_work(fp->txdata_ptr[cos]))
			return true;
	return false;
}

static inline int bnx2x_has_rx_work(struct bnx2x_fastpath *fp)
{
	u16 rx_cons_sb;

	/*                                                   */
	barrier();
	rx_cons_sb = le16_to_cpu(*fp->rx_cons_sb);
	if ((rx_cons_sb & MAX_RCQ_DESC_CNT) == MAX_RCQ_DESC_CNT)
		rx_cons_sb++;
	return (fp->rx_comp_cons != rx_cons_sb);
}

/* 
                                                          
  
                      
 */
static inline void bnx2x_tx_disable(struct bnx2x *bp)
{
	netif_tx_disable(bp->dev);
	netif_carrier_off(bp->dev);
}

static inline void bnx2x_free_rx_sge(struct bnx2x *bp,
				     struct bnx2x_fastpath *fp, u16 index)
{
	struct sw_rx_page *sw_buf = &fp->rx_page_ring[index];
	struct page *page = sw_buf->page;
	struct eth_rx_sge *sge = &fp->rx_sge_ring[index];

	/*                           */
	if (!page)
		return;

	dma_unmap_page(&bp->pdev->dev, dma_unmap_addr(sw_buf, mapping),
		       SGE_PAGES, DMA_FROM_DEVICE);
	__free_pages(page, PAGES_PER_SGE_SHIFT);

	sw_buf->page = NULL;
	sge->addr_hi = 0;
	sge->addr_lo = 0;
}

static inline void bnx2x_add_all_napi_cnic(struct bnx2x *bp)
{
	int i;

	/*                  */
	for_each_rx_queue_cnic(bp, i)
		netif_napi_add(bp->dev, &bnx2x_fp(bp, i, napi),
			       bnx2x_poll, NAPI_POLL_WEIGHT);
}

static inline void bnx2x_add_all_napi(struct bnx2x *bp)
{
	int i;

	/*                  */
	for_each_eth_queue(bp, i)
		netif_napi_add(bp->dev, &bnx2x_fp(bp, i, napi),
			       bnx2x_poll, NAPI_POLL_WEIGHT);
}

static inline void bnx2x_del_all_napi_cnic(struct bnx2x *bp)
{
	int i;

	for_each_rx_queue_cnic(bp, i)
		netif_napi_del(&bnx2x_fp(bp, i, napi));
}

static inline void bnx2x_del_all_napi(struct bnx2x *bp)
{
	int i;

	for_each_eth_queue(bp, i)
		netif_napi_del(&bnx2x_fp(bp, i, napi));
}

int bnx2x_set_int_mode(struct bnx2x *bp);

static inline void bnx2x_disable_msi(struct bnx2x *bp)
{
	if (bp->flags & USING_MSIX_FLAG) {
		pci_disable_msix(bp->pdev);
		bp->flags &= ~(USING_MSIX_FLAG | USING_SINGLE_MSIX_FLAG);
	} else if (bp->flags & USING_MSI_FLAG) {
		pci_disable_msi(bp->pdev);
		bp->flags &= ~USING_MSI_FLAG;
	}
}

static inline int bnx2x_calc_num_queues(struct bnx2x *bp)
{
	return  num_queues ?
		 min_t(int, num_queues, BNX2X_MAX_QUEUES(bp)) :
		 min_t(int, netif_get_num_default_rss_queues(),
		       BNX2X_MAX_QUEUES(bp));
}

static inline void bnx2x_clear_sge_mask_next_elems(struct bnx2x_fastpath *fp)
{
	int i, j;

	for (i = 1; i <= NUM_RX_SGE_PAGES; i++) {
		int idx = RX_SGE_CNT * i - 1;

		for (j = 0; j < 2; j++) {
			BIT_VEC64_CLEAR_BIT(fp->sge_mask, idx);
			idx--;
		}
	}
}

static inline void bnx2x_init_sge_ring_bit_mask(struct bnx2x_fastpath *fp)
{
	/*                                                                    */
	memset(fp->sge_mask, 0xff, sizeof(fp->sge_mask));

	/*                                             
                                                                
                                                            
                      */
	bnx2x_clear_sge_mask_next_elems(fp);
}

/*                                              
                                           
                                     
                                                        
 */
static inline void bnx2x_reuse_rx_data(struct bnx2x_fastpath *fp,
				      u16 cons, u16 prod)
{
	struct sw_rx_bd *cons_rx_buf = &fp->rx_buf_ring[cons];
	struct sw_rx_bd *prod_rx_buf = &fp->rx_buf_ring[prod];
	struct eth_rx_bd *cons_bd = &fp->rx_desc_ring[cons];
	struct eth_rx_bd *prod_bd = &fp->rx_desc_ring[prod];

	dma_unmap_addr_set(prod_rx_buf, mapping,
			   dma_unmap_addr(cons_rx_buf, mapping));
	prod_rx_buf->data = cons_rx_buf->data;
	*prod_bd = *cons_bd;
}

/*                                                                       */

/*                                     */
static inline int func_by_vn(struct bnx2x *bp, int vn)
{
	return 2 * vn + BP_PORT(bp);
}

static inline int bnx2x_config_rss_eth(struct bnx2x *bp, bool config_hash)
{
	return bnx2x_config_rss_pf(bp, &bp->rss_conf_obj, config_hash);
}

/* 
                                   
  
                      
  
                                                                   
 */
static inline int bnx2x_func_start(struct bnx2x *bp)
{
	struct bnx2x_func_state_params func_params = {NULL};
	struct bnx2x_func_start_params *start_params =
		&func_params.params.start;

	/*                                                   */
	__set_bit(RAMROD_COMP_WAIT, &func_params.ramrod_flags);

	func_params.f_obj = &bp->func_obj;
	func_params.cmd = BNX2X_F_CMD_START;

	/*                     */
	start_params->mf_mode = bp->mf_mode;
	start_params->sd_vlan_tag = bp->mf_ov;

	if (CHIP_IS_E2(bp) || CHIP_IS_E3(bp))
		start_params->network_cos_mode = STATIC_COS;
	else /*             */
		start_params->network_cos_mode = FW_WRR;

	start_params->gre_tunnel_mode = IPGRE_TUNNEL;
	start_params->gre_tunnel_rss = GRE_INNER_HEADERS_RSS;

	return bnx2x_func_state_change(bp, &func_params);
}

/* 
                                                             
  
                                
                                  
                                
                               
 */
static inline void bnx2x_set_fw_mac_addr(__le16 *fw_hi, __le16 *fw_mid,
					 __le16 *fw_lo, u8 *mac)
{
	((u8 *)fw_hi)[0]  = mac[1];
	((u8 *)fw_hi)[1]  = mac[0];
	((u8 *)fw_mid)[0] = mac[3];
	((u8 *)fw_mid)[1] = mac[2];
	((u8 *)fw_lo)[0]  = mac[5];
	((u8 *)fw_lo)[1]  = mac[4];
}

static inline void bnx2x_free_rx_sge_range(struct bnx2x *bp,
					   struct bnx2x_fastpath *fp, int last)
{
	int i;

	if (fp->disable_tpa)
		return;

	for (i = 0; i < last; i++)
		bnx2x_free_rx_sge(bp, fp, i);
}

static inline void bnx2x_set_next_page_rx_bd(struct bnx2x_fastpath *fp)
{
	int i;

	for (i = 1; i <= NUM_RX_RINGS; i++) {
		struct eth_rx_bd *rx_bd;

		rx_bd = &fp->rx_desc_ring[RX_DESC_CNT * i - 2];
		rx_bd->addr_hi =
			cpu_to_le32(U64_HI(fp->rx_desc_mapping +
				    BCM_PAGE_SIZE*(i % NUM_RX_RINGS)));
		rx_bd->addr_lo =
			cpu_to_le32(U64_LO(fp->rx_desc_mapping +
				    BCM_PAGE_SIZE*(i % NUM_RX_RINGS)));
	}
}

/*                                                                         
        
 */
static inline u8 bnx2x_stats_id(struct bnx2x_fastpath *fp)
{
	struct bnx2x *bp = fp->bp;
	if (!CHIP_IS_E1x(bp)) {
		/*                                                         */
		if (IS_FCOE_FP(fp))
			return bp->cnic_base_cl_id + (bp->pf_num >> 1);
		return fp->cl_id;
	}
	return fp->cl_id + BP_PORT(bp) * FP_SB_MAX_E1x;
}

static inline void bnx2x_init_vlan_mac_fp_objs(struct bnx2x_fastpath *fp,
					       bnx2x_obj_type obj_type)
{
	struct bnx2x *bp = fp->bp;

	/*                              */
	bnx2x_init_mac_obj(bp, &bnx2x_sp_obj(bp, fp).mac_obj, fp->cl_id,
			   fp->cid, BP_FUNC(bp), bnx2x_sp(bp, mac_rdata),
			   bnx2x_sp_mapping(bp, mac_rdata),
			   BNX2X_FILTER_MAC_PENDING,
			   &bp->sp_state, obj_type,
			   &bp->macs_pool);
}

/* 
                                                           
  
                      
  
                                                                
                
 */
static inline u8 bnx2x_get_path_func_num(struct bnx2x *bp)
{
	u8 func_num = 0, i;

	/*                                      */
	if (CHIP_IS_E1(bp))
		return 1;

	/*                                                       
              
  */
	if (CHIP_REV_IS_SLOW(bp)) {
		if (IS_MF(bp))
			func_num = 4;
		else
			func_num = 2;
	} else {
		for (i = 0; i < E1H_FUNC_MAX / 2; i++) {
			u32 func_config =
				MF_CFG_RD(bp,
					  func_mf_config[BP_PORT(bp) + 2 * i].
					  config);
			func_num +=
				((func_config & FUNC_MF_CFG_FUNC_HIDE) ? 0 : 1);
		}
	}

	WARN_ON(!func_num);

	return func_num;
}

static inline void bnx2x_init_bp_objs(struct bnx2x *bp)
{
	/*                            */
	bnx2x_init_rx_mode_obj(bp, &bp->rx_mode_obj);

	/*                                            */
	bnx2x_init_mcast_obj(bp, &bp->mcast_obj, bp->fp->cl_id, bp->fp->cid,
			     BP_FUNC(bp), BP_FUNC(bp),
			     bnx2x_sp(bp, mcast_rdata),
			     bnx2x_sp_mapping(bp, mcast_rdata),
			     BNX2X_FILTER_MCAST_PENDING, &bp->sp_state,
			     BNX2X_OBJ_TYPE_RX);

	/*                        */
	bnx2x_init_mac_credit_pool(bp, &bp->macs_pool, BP_FUNC(bp),
				   bnx2x_get_path_func_num(bp));

	bnx2x_init_vlan_credit_pool(bp, &bp->vlans_pool, BP_ABS_FUNC(bp)>>1,
				    bnx2x_get_path_func_num(bp));

	/*                          */
	bnx2x_init_rss_config_obj(bp, &bp->rss_conf_obj, bp->fp->cl_id,
				  bp->fp->cid, BP_FUNC(bp), BP_FUNC(bp),
				  bnx2x_sp(bp, rss_rdata),
				  bnx2x_sp_mapping(bp, rss_rdata),
				  BNX2X_FILTER_RSS_CONF_PENDING, &bp->sp_state,
				  BNX2X_OBJ_TYPE_RX);
}

static inline u8 bnx2x_fp_qzone_id(struct bnx2x_fastpath *fp)
{
	if (CHIP_IS_E1x(fp->bp))
		return fp->cl_id + BP_PORT(fp->bp) * ETH_MAX_RX_CLIENTS_E1H;
	else
		return fp->cl_id;
}

u32 bnx2x_rx_ustorm_prods_offset(struct bnx2x_fastpath *fp);

static inline void bnx2x_init_txdata(struct bnx2x *bp,
				     struct bnx2x_fp_txdata *txdata, u32 cid,
				     int txq_index, __le16 *tx_cons_sb,
				     struct bnx2x_fastpath *fp)
{
	txdata->cid = cid;
	txdata->txq_index = txq_index;
	txdata->tx_cons_sb = tx_cons_sb;
	txdata->parent_fp = fp;
	txdata->tx_ring_size = IS_FCOE_FP(fp) ? MAX_TX_AVAIL : bp->tx_ring_size;

	DP(NETIF_MSG_IFUP, "created tx data cid %d, txq %d\n",
	   txdata->cid, txdata->txq_index);
}

static inline u8 bnx2x_cnic_eth_cl_id(struct bnx2x *bp, u8 cl_idx)
{
	return bp->cnic_base_cl_id + cl_idx +
		(bp->pf_num >> 1) * BNX2X_MAX_CNIC_ETH_CL_ID_IDX;
}

static inline u8 bnx2x_cnic_fw_sb_id(struct bnx2x *bp)
{

	/*                                          */
	return bp->base_fw_ndsb;
}

static inline u8 bnx2x_cnic_igu_sb_id(struct bnx2x *bp)
{
	return bp->igu_base_sb;
}


static inline void bnx2x_init_fcoe_fp(struct bnx2x *bp)
{
	struct bnx2x_fastpath *fp = bnx2x_fcoe_fp(bp);
	unsigned long q_type = 0;

	bnx2x_fcoe(bp, rx_queue) = BNX2X_NUM_ETH_QUEUES(bp);
	bnx2x_fcoe(bp, cl_id) = bnx2x_cnic_eth_cl_id(bp,
						     BNX2X_FCOE_ETH_CL_ID_IDX);
	bnx2x_fcoe(bp, cid) = BNX2X_FCOE_ETH_CID(bp);
	bnx2x_fcoe(bp, fw_sb_id) = DEF_SB_ID;
	bnx2x_fcoe(bp, igu_sb_id) = bp->igu_dsb_id;
	bnx2x_fcoe(bp, rx_cons_sb) = BNX2X_FCOE_L2_RX_INDEX;
	bnx2x_init_txdata(bp, bnx2x_fcoe(bp, txdata_ptr[0]),
			  fp->cid, FCOE_TXQ_IDX(bp), BNX2X_FCOE_L2_TX_INDEX,
			  fp);

	DP(NETIF_MSG_IFUP, "created fcoe tx data (fp index %d)\n", fp->index);

	/*                                            */
	bnx2x_fcoe(bp, cl_qzone_id) = bnx2x_fp_qzone_id(fp);
	/*               */
	bnx2x_fcoe(bp, ustorm_rx_prods_offset) =
		bnx2x_rx_ustorm_prods_offset(fp);

	/*                              */
	__set_bit(BNX2X_Q_TYPE_HAS_RX, &q_type);
	__set_bit(BNX2X_Q_TYPE_HAS_TX, &q_type);

	/*                                 */
	BUG_ON(fp->max_cos != 1);

	bnx2x_init_queue_obj(bp, &bnx2x_sp_obj(bp, fp).q_obj, fp->cl_id,
			     &fp->cid, 1, BP_FUNC(bp), bnx2x_sp(bp, q_rdata),
			     bnx2x_sp_mapping(bp, q_rdata), q_type);

	DP(NETIF_MSG_IFUP,
	   "queue[%d]: bnx2x_init_sb(%p,%p) cl_id %d fw_sb %d igu_sb %d\n",
	   fp->index, bp, fp->status_blk.e2_sb, fp->cl_id, fp->fw_sb_id,
	   fp->igu_sb_id);
}

static inline int bnx2x_clean_tx_queue(struct bnx2x *bp,
				       struct bnx2x_fp_txdata *txdata)
{
	int cnt = 1000;

	while (bnx2x_has_tx_work_unload(txdata)) {
		if (!cnt) {
			BNX2X_ERR("timeout waiting for queue[%d]: txdata->tx_pkt_prod(%d) != txdata->tx_pkt_cons(%d)\n",
				  txdata->txq_index, txdata->tx_pkt_prod,
				  txdata->tx_pkt_cons);
#ifdef BNX2X_STOP_ON_ERROR
			bnx2x_panic();
			return -EBUSY;
#else
			break;
#endif
		}
		cnt--;
		usleep_range(1000, 2000);
	}

	return 0;
}

int bnx2x_get_link_cfg_idx(struct bnx2x *bp);

static inline void __storm_memset_struct(struct bnx2x *bp,
					 u32 addr, size_t size, u32 *data)
{
	int i;
	for (i = 0; i < size/4; i++)
		REG_WR(bp, addr + (i * 4), data[i]);
}

/* 
                                                             
  
                      
                                      
 */
static inline bool bnx2x_wait_sp_comp(struct bnx2x *bp, unsigned long mask)
{
	int tout = 5000; /*                      */

	while (tout--) {
		smp_mb();
		netif_addr_lock_bh(bp->dev);
		if (!(bp->sp_state & mask)) {
			netif_addr_unlock_bh(bp->dev);
			return true;
		}
		netif_addr_unlock_bh(bp->dev);

		usleep_range(1000, 2000);
	}

	smp_mb();

	netif_addr_lock_bh(bp->dev);
	if (bp->sp_state & mask) {
		BNX2X_ERR("Filtering completion timed out. sp_state 0x%lx, mask 0x%lx\n",
			  bp->sp_state, mask);
		netif_addr_unlock_bh(bp->dev);
		return false;
	}
	netif_addr_unlock_bh(bp->dev);

	return true;
}

/* 
                                                               
  
                      
                                                     
                                                  
 */
void bnx2x_set_ctx_validation(struct bnx2x *bp, struct eth_context *cxt,
			      u32 cid);

void bnx2x_update_coalesce_sb_index(struct bnx2x *bp, u8 fw_sb_id,
				    u8 sb_index, u8 disable, u16 usec);
void bnx2x_acquire_phy_lock(struct bnx2x *bp);
void bnx2x_release_phy_lock(struct bnx2x *bp);

/* 
                                                                     
  
                      
                            
  
 */
static inline u16 bnx2x_extract_max_cfg(struct bnx2x *bp, u32 mf_cfg)
{
	u16 max_cfg = (mf_cfg & FUNC_MF_CFG_MAX_BW_MASK) >>
			      FUNC_MF_CFG_MAX_BW_SHIFT;
	if (!max_cfg) {
		DP(NETIF_MSG_IFUP | BNX2X_MSG_ETHTOOL,
		   "Max BW configured to 0 - using 100 instead\n");
		max_cfg = 100;
	}
	return max_cfg;
}

/*                                         */
static inline bool bnx2x_mtu_allows_gro(int mtu)
{
	/*                    */
	int fpp = SGE_PAGE_SIZE / (mtu - ETH_MAX_TPA_HEADER_SIZE);

	/*
                                                          
                             
  */
	return mtu <= SGE_PAGE_SIZE && (U_ETH_SGL_SIZE * fpp) <= MAX_SKB_FRAGS;
}

/* 
                                                                          
  
                      
  
 */
void bnx2x_get_iscsi_info(struct bnx2x *bp);

/* 
                                                                 
  
                      
  
 */
static inline void bnx2x_link_sync_notify(struct bnx2x *bp)
{
	int func;
	int vn;

	/*                                                          */
	for (vn = VN_0; vn < BP_MAX_VN_NUM(bp); vn++) {
		if (vn == BP_VN(bp))
			continue;

		func = func_by_vn(bp, vn);
		REG_WR(bp, MISC_REG_AEU_GENERAL_ATTN_0 +
		       (LINK_SYNC_ATTENTION_BIT_FUNC_0 + func)*4, 1);
	}
}

/* 
                                                 
  
                      
                          
                     
  
 */
static inline void bnx2x_update_drv_flags(struct bnx2x *bp, u32 flags, u32 set)
{
	if (SHMEM2_HAS(bp, drv_flags)) {
		u32 drv_flags;
		bnx2x_acquire_hw_lock(bp, HW_LOCK_RESOURCE_DRV_FLAGS);
		drv_flags = SHMEM2_RD(bp, drv_flags);

		if (set)
			SET_FLAGS(drv_flags, flags);
		else
			RESET_FLAGS(drv_flags, flags);

		SHMEM2_WR(bp, drv_flags, drv_flags);
		DP(NETIF_MSG_IFUP, "drv_flags 0x%08x\n", drv_flags);
		bnx2x_release_hw_lock(bp, HW_LOCK_RESOURCE_DRV_FLAGS);
	}
}

static inline bool bnx2x_is_valid_ether_addr(struct bnx2x *bp, u8 *addr)
{
	if (is_valid_ether_addr(addr) ||
	    (is_zero_ether_addr(addr) &&
	     (IS_MF_STORAGE_SD(bp) || IS_MF_FCOE_AFEX(bp))))
		return true;

	return false;
}

/* 
                                                         
  
                            
                                                        
                                         
  
 */
void bnx2x_fill_fw_str(struct bnx2x *bp, char *buf, size_t buf_len);

int bnx2x_drain_tx_queues(struct bnx2x *bp);
void bnx2x_squeeze_objects(struct bnx2x *bp);

#endif /*             */
