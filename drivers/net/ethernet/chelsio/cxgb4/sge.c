/*
 * This file is part of the Chelsio T4 Ethernet driver for Linux.
 *
 * Copyright (c) 2003-2010 Chelsio Communications, Inc. All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/if_vlan.h>
#include <linux/ip.h>
#include <linux/dma-mapping.h>
#include <linux/jiffies.h>
#include <linux/prefetch.h>
#include <linux/export.h>
#include <net/ipv6.h>
#include <net/tcp.h>
#include "cxgb4.h"
#include "t4_regs.h"
#include "t4_msg.h"
#include "t4fw_api.h"

/*
                                                                            
                               
 */
#if PAGE_SHIFT >= 16
# define FL_PG_ORDER 0
#else
# define FL_PG_ORDER (16 - PAGE_SHIFT)
#endif

/*                                        */
#define RX_COPY_THRES    256
#define RX_PULL_LEN      128

/*
                                                                             
                                                                                
 */
#define RX_PKT_SKB_LEN   512

/*
                                                                           
                                                                             
                                                                           
                                                                 
 */
#define MAX_TX_RECLAIM 16

/*
                                                                            
                                         
 */
#define MAX_RX_REFILL 16U

/*
                                                                          
                                                                           
 */
#define RX_QCHECK_PERIOD (HZ / 2)

/*
                                      
 */
#define TX_QCHECK_PERIOD (HZ / 2)

/*
                                                                
 */
#define MAX_TIMER_TX_RECLAIM 100

/*
                                                            
 */
#define NOMEM_TMR_IDX (SGE_NTIMERS - 1)

/*
                                                                              
                        
 */
#define FL_STARVE_THRES 4

/*
                                                                           
                                                            
                             
 */
#define ETHTXQ_STOP_THRES \
	(1 + DIV_ROUND_UP((3 * MAX_SKB_FRAGS) / 2 + (MAX_SKB_FRAGS & 1), 8))

/*
                                                                           
                       
 */
#define TXQ_STOP_THRES (SGE_MAX_WR_LEN / sizeof(struct tx_desc))

/*
                                                                        
             
 */
#define MAX_IMM_TX_PKT_LEN 128

/*
                                                    
 */
#define MAX_CTRL_WR_LEN SGE_MAX_WR_LEN

struct tx_sw_desc {                /*                            */
	struct sk_buff *skb;
	struct ulptx_sgl *sgl;
};

struct rx_sw_desc {                /*                            */
	struct page *page;
	dma_addr_t dma_addr;
};

/*
                                                                             
                                                                              
                                                                          
           
 */
#define FL_MTU_SMALL 1500
#define FL_MTU_LARGE 9000

static inline unsigned int fl_mtu_bufsize(struct adapter *adapter,
					  unsigned int mtu)
{
	struct sge *s = &adapter->sge;

	return ALIGN(s->pktshift + ETH_HLEN + VLAN_HLEN + mtu, s->fl_align);
}

#define FL_MTU_SMALL_BUFSIZE(adapter) fl_mtu_bufsize(adapter, FL_MTU_SMALL)
#define FL_MTU_LARGE_BUFSIZE(adapter) fl_mtu_bufsize(adapter, FL_MTU_LARGE)

/*
                                                                            
                                                                             
                                                                             
                                                                             
                                                                              
                                                                        
                                                                            
                                                                    
 */
enum {
	RX_BUF_FLAGS     = 0x1f,   /*                              */
	RX_BUF_SIZE      = 0x0f,   /*                                     */
	RX_UNMAPPED_BUF  = 0x10,   /*                      */

	/*
                                                               
                                                                
                                                              
                                                                  
                                                         
  */
	RX_SMALL_PG_BUF  = 0x0,   /*                               */
	RX_LARGE_PG_BUF  = 0x1,   /*                                        */

	RX_SMALL_MTU_BUF = 0x2,   /*                  */
	RX_LARGE_MTU_BUF = 0x3,   /*                  */
};

static inline dma_addr_t get_buf_addr(const struct rx_sw_desc *d)
{
	return d->dma_addr & ~(dma_addr_t)RX_BUF_FLAGS;
}

static inline bool is_buf_mapped(const struct rx_sw_desc *d)
{
	return !(d->dma_addr & RX_UNMAPPED_BUF);
}

/* 
                                                                 
                   
  
                                                                         
           
 */
static inline unsigned int txq_avail(const struct sge_txq *q)
{
	return q->size - 1 - q->in_use;
}

/* 
                                                     
              
  
                                                                         
                                                                          
                                 
 */
static inline unsigned int fl_cap(const struct sge_fl *fl)
{
	return fl->size - 8;   /*                          */
}

static inline bool fl_starving(const struct sge_fl *fl)
{
	return fl->avail - fl->pend_cred <= FL_STARVE_THRES;
}

static int map_skb(struct device *dev, const struct sk_buff *skb,
		   dma_addr_t *addr)
{
	const skb_frag_t *fp, *end;
	const struct skb_shared_info *si;

	*addr = dma_map_single(dev, skb->data, skb_headlen(skb), DMA_TO_DEVICE);
	if (dma_mapping_error(dev, *addr))
		goto out_err;

	si = skb_shinfo(skb);
	end = &si->frags[si->nr_frags];

	for (fp = si->frags; fp < end; fp++) {
		*++addr = skb_frag_dma_map(dev, fp, 0, skb_frag_size(fp),
					   DMA_TO_DEVICE);
		if (dma_mapping_error(dev, *addr))
			goto unwind;
	}
	return 0;

unwind:
	while (fp-- > si->frags)
		dma_unmap_page(dev, *--addr, skb_frag_size(fp), DMA_TO_DEVICE);

	dma_unmap_single(dev, addr[-1], skb_headlen(skb), DMA_TO_DEVICE);
out_err:
	return -ENOMEM;
}

#ifdef CONFIG_NEED_DMA_MAP_STATE
static void unmap_skb(struct device *dev, const struct sk_buff *skb,
		      const dma_addr_t *addr)
{
	const skb_frag_t *fp, *end;
	const struct skb_shared_info *si;

	dma_unmap_single(dev, *addr++, skb_headlen(skb), DMA_TO_DEVICE);

	si = skb_shinfo(skb);
	end = &si->frags[si->nr_frags];
	for (fp = si->frags; fp < end; fp++)
		dma_unmap_page(dev, *addr++, skb_frag_size(fp), DMA_TO_DEVICE);
}

/* 
                                                              
                   
  
                                                                        
                                                                         
         
 */
static void deferred_unmap_destructor(struct sk_buff *skb)
{
	unmap_skb(skb->dev->dev.parent, skb, (dma_addr_t *)skb->head);
}
#endif

static void unmap_sgl(struct device *dev, const struct sk_buff *skb,
		      const struct ulptx_sgl *sgl, const struct sge_txq *q)
{
	const struct ulptx_sge_pair *p;
	unsigned int nfrags = skb_shinfo(skb)->nr_frags;

	if (likely(skb_headlen(skb)))
		dma_unmap_single(dev, be64_to_cpu(sgl->addr0), ntohl(sgl->len0),
				 DMA_TO_DEVICE);
	else {
		dma_unmap_page(dev, be64_to_cpu(sgl->addr0), ntohl(sgl->len0),
			       DMA_TO_DEVICE);
		nfrags--;
	}

	/*
                                                                       
                           
  */
	for (p = sgl->sge; nfrags >= 2; nfrags -= 2) {
		if (likely((u8 *)(p + 1) <= (u8 *)q->stat)) {
unmap:			dma_unmap_page(dev, be64_to_cpu(p->addr[0]),
				       ntohl(p->len[0]), DMA_TO_DEVICE);
			dma_unmap_page(dev, be64_to_cpu(p->addr[1]),
				       ntohl(p->len[1]), DMA_TO_DEVICE);
			p++;
		} else if ((u8 *)p == (u8 *)q->stat) {
			p = (const struct ulptx_sge_pair *)q->desc;
			goto unmap;
		} else if ((u8 *)p + 8 == (u8 *)q->stat) {
			const __be64 *addr = (const __be64 *)q->desc;

			dma_unmap_page(dev, be64_to_cpu(addr[0]),
				       ntohl(p->len[0]), DMA_TO_DEVICE);
			dma_unmap_page(dev, be64_to_cpu(addr[1]),
				       ntohl(p->len[1]), DMA_TO_DEVICE);
			p = (const struct ulptx_sge_pair *)&addr[2];
		} else {
			const __be64 *addr = (const __be64 *)q->desc;

			dma_unmap_page(dev, be64_to_cpu(p->addr[0]),
				       ntohl(p->len[0]), DMA_TO_DEVICE);
			dma_unmap_page(dev, be64_to_cpu(addr[0]),
				       ntohl(p->len[1]), DMA_TO_DEVICE);
			p = (const struct ulptx_sge_pair *)&addr[1];
		}
	}
	if (nfrags) {
		__be64 addr;

		if ((u8 *)p == (u8 *)q->stat)
			p = (const struct ulptx_sge_pair *)q->desc;
		addr = (u8 *)p + 16 <= (u8 *)q->stat ? p->addr[0] :
						       *(const __be64 *)q->desc;
		dma_unmap_page(dev, be64_to_cpu(addr), ntohl(p->len[0]),
			       DMA_TO_DEVICE);
	}
}

/* 
                                                           
                        
                                               
                                           
                                                         
  
                                                                        
                                                   
 */
static void free_tx_desc(struct adapter *adap, struct sge_txq *q,
			 unsigned int n, bool unmap)
{
	struct tx_sw_desc *d;
	unsigned int cidx = q->cidx;
	struct device *dev = adap->pdev_dev;

	d = &q->sdesc[cidx];
	while (n--) {
		if (d->skb) {                       /*                   */
			if (unmap)
				unmap_sgl(dev, d->skb, d->sgl, q);
			kfree_skb(d->skb);
			d->skb = NULL;
		}
		++d;
		if (++cidx == q->size) {
			cidx = 0;
			d = q->sdesc;
		}
	}
	q->cidx = cidx;
}

/*
                                                              
 */
static inline int reclaimable(const struct sge_txq *q)
{
	int hw_cidx = ntohs(q->stat->cidx);
	hw_cidx -= q->cidx;
	return hw_cidx < 0 ? hw_cidx + q->size : hw_cidx;
}

/* 
                                                           
                     
                                                         
                                                         
  
                                                                       
                                                                    
                
 */
static inline void reclaim_completed_tx(struct adapter *adap, struct sge_txq *q,
					bool unmap)
{
	int avail = reclaimable(q);

	if (avail) {
		/*
                                                              
                                
   */
		if (avail > MAX_TX_RECLAIM)
			avail = MAX_TX_RECLAIM;

		free_tx_desc(adap, q, avail, unmap);
		q->in_use -= avail;
	}
}

static inline int get_buf_size(struct adapter *adapter,
			       const struct rx_sw_desc *d)
{
	struct sge *s = &adapter->sge;
	unsigned int rx_buf_size_idx = d->dma_addr & RX_BUF_SIZE;
	int buf_size;

	switch (rx_buf_size_idx) {
	case RX_SMALL_PG_BUF:
		buf_size = PAGE_SIZE;
		break;

	case RX_LARGE_PG_BUF:
		buf_size = PAGE_SIZE << s->fl_pg_order;
		break;

	case RX_SMALL_MTU_BUF:
		buf_size = FL_MTU_SMALL_BUFSIZE(adapter);
		break;

	case RX_LARGE_MTU_BUF:
		buf_size = FL_MTU_LARGE_BUFSIZE(adapter);
		break;

	default:
		BUG_ON(1);
	}

	return buf_size;
}

/* 
                                                         
                     
                                             
                               
  
                                                                    
                                                                        
 */
static void free_rx_bufs(struct adapter *adap, struct sge_fl *q, int n)
{
	while (n--) {
		struct rx_sw_desc *d = &q->sdesc[q->cidx];

		if (is_buf_mapped(d))
			dma_unmap_page(adap->pdev_dev, get_buf_addr(d),
				       get_buf_size(adap, d),
				       PCI_DMA_FROMDEVICE);
		put_page(d->page);
		d->page = NULL;
		if (++q->cidx == q->size)
			q->cidx = 0;
		q->avail--;
	}
}

/* 
                                                                 
                     
                        
  
                                                                 
                                                                       
  
                                                                       
                                                                    
 */
static void unmap_rx_buf(struct adapter *adap, struct sge_fl *q)
{
	struct rx_sw_desc *d = &q->sdesc[q->cidx];

	if (is_buf_mapped(d))
		dma_unmap_page(adap->pdev_dev, get_buf_addr(d),
			       get_buf_size(adap, d), PCI_DMA_FROMDEVICE);
	d->page = NULL;
	if (++q->cidx == q->size)
		q->cidx = 0;
	q->avail--;
}

static inline void ring_fl_db(struct adapter *adap, struct sge_fl *q)
{
	u32 val;
	if (q->pend_cred >= 8) {
		val = PIDX(q->pend_cred / 8);
		if (!is_t4(adap->chip))
			val |= DBTYPE(1);
		wmb();
		t4_write_reg(adap, MYPF_REG(SGE_PF_KDOORBELL), DBPRIO(1) |
			     QID(q->cntxt_id) | val);
		q->pend_cred &= 7;
	}
}

static inline void set_rx_sw_desc(struct rx_sw_desc *sd, struct page *pg,
				  dma_addr_t mapping)
{
	sd->page = pg;
	sd->dma_addr = mapping;      /*                        */
}

/* 
                                           
                     
                         
                                            
                                          
  
                                                                          
                                                                      
                                                                       
                                                                          
  
                                           
 */
static unsigned int refill_fl(struct adapter *adap, struct sge_fl *q, int n,
			      gfp_t gfp)
{
	struct sge *s = &adap->sge;
	struct page *pg;
	dma_addr_t mapping;
	unsigned int cred = q->avail;
	__be64 *d = &q->desc[q->pidx];
	struct rx_sw_desc *sd = &q->sdesc[q->pidx];

	gfp |= __GFP_NOWARN | __GFP_COLD;

	if (s->fl_pg_order == 0)
		goto alloc_small_pages;

	/*
                        
  */
	while (n) {
		pg = alloc_pages(gfp | __GFP_COMP, s->fl_pg_order);
		if (unlikely(!pg)) {
			q->large_alloc_failed++;
			break;       /*                           */
		}

		mapping = dma_map_page(adap->pdev_dev, pg, 0,
				       PAGE_SIZE << s->fl_pg_order,
				       PCI_DMA_FROMDEVICE);
		if (unlikely(dma_mapping_error(adap->pdev_dev, mapping))) {
			__free_pages(pg, s->fl_pg_order);
			goto out;   /*                                       */
		}
		mapping |= RX_LARGE_PG_BUF;
		*d++ = cpu_to_be64(mapping);

		set_rx_sw_desc(sd, pg, mapping);
		sd++;

		q->avail++;
		if (++q->pidx == q->size) {
			q->pidx = 0;
			sd = q->sdesc;
			d = q->desc;
		}
		n--;
	}

alloc_small_pages:
	while (n--) {
		pg = __skb_alloc_page(gfp, NULL);
		if (unlikely(!pg)) {
			q->alloc_failed++;
			break;
		}

		mapping = dma_map_page(adap->pdev_dev, pg, 0, PAGE_SIZE,
				       PCI_DMA_FROMDEVICE);
		if (unlikely(dma_mapping_error(adap->pdev_dev, mapping))) {
			put_page(pg);
			goto out;
		}
		*d++ = cpu_to_be64(mapping);

		set_rx_sw_desc(sd, pg, mapping);
		sd++;

		q->avail++;
		if (++q->pidx == q->size) {
			q->pidx = 0;
			sd = q->sdesc;
			d = q->desc;
		}
	}

out:	cred = q->avail - cred;
	q->pend_cred += cred;
	ring_fl_db(adap, q);

	if (unlikely(fl_starving(q))) {
		smp_wmb();
		set_bit(q->cntxt_id - adap->sge.egr_start,
			adap->sge.starving_fl);
	}

	return cred;
}

static inline void __refill_fl(struct adapter *adap, struct sge_fl *fl)
{
	refill_fl(adap, fl, min(MAX_RX_REFILL, fl_cap(fl) - fl->avail),
		  GFP_ATOMIC);
}

/* 
                                                             
                                     
                                    
                                          
                                                                       
                                                    
                                                                    
                                                            
                                               
  
                                                                     
                                                                 
                                                                        
                                                                      
                                                                      
                                                                    
                  
 */
static void *alloc_ring(struct device *dev, size_t nelem, size_t elem_size,
			size_t sw_size, dma_addr_t *phys, void *metadata,
			size_t stat_size, int node)
{
	size_t len = nelem * elem_size + stat_size;
	void *s = NULL;
	void *p = dma_alloc_coherent(dev, len, phys, GFP_KERNEL);

	if (!p)
		return NULL;
	if (sw_size) {
		s = kzalloc_node(nelem * sw_size, GFP_KERNEL, node);

		if (!s) {
			dma_free_coherent(dev, len, p, *phys);
			return NULL;
		}
	}
	if (metadata)
		*(void **)metadata = s;
	memset(p, 0, len);
	return p;
}

/* 
                                                                
                                
  
                                                                       
                                        
 */
static inline unsigned int sgl_len(unsigned int n)
{
	n--;
	return (3 * n) / 2 + (n & 1) + 2;
}

/* 
                                                                        
                          
  
                                                                      
            
 */
static inline unsigned int flits_to_desc(unsigned int n)
{
	BUG_ON(n > SGE_MAX_WR_LEN / 8);
	return DIV_ROUND_UP(n, 8);
}

/* 
                                                                 
                   
  
                                                               
                  
 */
static inline int is_eth_imm(const struct sk_buff *skb)
{
	return skb->len <= MAX_IMM_TX_PKT_LEN - sizeof(struct cpl_tx_pkt);
}

/* 
                                                                   
                   
  
                                                                        
                                                   
 */
static inline unsigned int calc_tx_flits(const struct sk_buff *skb)
{
	unsigned int flits;

	if (is_eth_imm(skb))
		return DIV_ROUND_UP(skb->len + sizeof(struct cpl_tx_pkt), 8);

	flits = sgl_len(skb_shinfo(skb)->nr_frags + 1) + 4;
	if (skb_shinfo(skb)->gso_size)
		flits += 2;
	return flits;
}

/* 
                                                                      
                   
  
                                                                     
                                                   
 */
static inline unsigned int calc_tx_descs(const struct sk_buff *skb)
{
	return flits_to_desc(calc_tx_flits(skb));
}

/* 
                                                          
                   
                                       
                                              
                                              
                                                                     
                                                        
  
                                                                 
                                                                           
                                                                          
                                                                     
                                                                        
                                                                        
                                  
 */
static void write_sgl(const struct sk_buff *skb, struct sge_txq *q,
		      struct ulptx_sgl *sgl, u64 *end, unsigned int start,
		      const dma_addr_t *addr)
{
	unsigned int i, len;
	struct ulptx_sge_pair *to;
	const struct skb_shared_info *si = skb_shinfo(skb);
	unsigned int nfrags = si->nr_frags;
	struct ulptx_sge_pair buf[MAX_SKB_FRAGS / 2 + 1];

	len = skb_headlen(skb) - start;
	if (likely(len)) {
		sgl->len0 = htonl(len);
		sgl->addr0 = cpu_to_be64(addr[0] + start);
		nfrags++;
	} else {
		sgl->len0 = htonl(skb_frag_size(&si->frags[0]));
		sgl->addr0 = cpu_to_be64(addr[1]);
	}

	sgl->cmd_nsge = htonl(ULPTX_CMD(ULP_TX_SC_DSGL) | ULPTX_NSGE(nfrags));
	if (likely(--nfrags == 0))
		return;
	/*
                                                                      
                                                                     
                                                                  
  */
	to = (u8 *)end > (u8 *)q->stat ? buf : sgl->sge;

	for (i = (nfrags != si->nr_frags); nfrags >= 2; nfrags -= 2, to++) {
		to->len[0] = cpu_to_be32(skb_frag_size(&si->frags[i]));
		to->len[1] = cpu_to_be32(skb_frag_size(&si->frags[++i]));
		to->addr[0] = cpu_to_be64(addr[i]);
		to->addr[1] = cpu_to_be64(addr[++i]);
	}
	if (nfrags) {
		to->len[0] = cpu_to_be32(skb_frag_size(&si->frags[i]));
		to->len[1] = cpu_to_be32(0);
		to->addr[0] = cpu_to_be64(addr[i + 1]);
	}
	if (unlikely((u8 *)end > (u8 *)q->stat)) {
		unsigned int part0 = (u8 *)q->stat - (u8 *)sgl->sge, part1;

		if (likely(part0))
			memcpy(sgl->sge, buf, part0);
		part1 = (u8 *)end - (u8 *)q->stat;
		memcpy(q->desc, (u8 *)buf + part0, part1);
		end = (void *)q->desc + part1;
	}
	if ((uintptr_t)end & 8)           /*                         */
		*end = 0;
}

/*                                                       
                                               
                                                                         
 */
static void cxgb_pio_copy(u64 __iomem *dst, u64 *src)
{
	int count = 8;

	while (count) {
		writeq(*src, dst);
		src++;
		dst++;
		count--;
	}
}

/* 
                                                                
                     
                   
                                              
  
                                   
 */
static inline void ring_tx_db(struct adapter *adap, struct sge_txq *q, int n)
{
	unsigned int *wr, index;

	wmb();            /*                                     */
	spin_lock(&q->db_lock);
	if (!q->db_disabled) {
		if (is_t4(adap->chip)) {
			t4_write_reg(adap, MYPF_REG(SGE_PF_KDOORBELL),
				     QID(q->cntxt_id) | PIDX(n));
		} else {
			if (n == 1) {
				index = q->pidx ? (q->pidx - 1) : (q->size - 1);
				wr = (unsigned int *)&q->desc[index];
				cxgb_pio_copy((u64 __iomem *)
					      (adap->bar2 + q->udb + 64),
					      (u64 *)wr);
			} else
				writel(n,  adap->bar2 + q->udb + 8);
			wmb();
		}
	}
	q->db_pidx = q->pidx;
	spin_unlock(&q->db_lock);
}

/* 
                                                             
                   
                                                    
                                                                     
  
                                                                       
                                             
                                                                        
                                                 
 */
static void inline_tx_skb(const struct sk_buff *skb, const struct sge_txq *q,
			  void *pos)
{
	u64 *p;
	int left = (void *)q->stat - pos;

	if (likely(skb->len <= left)) {
		if (likely(!skb->data_len))
			skb_copy_from_linear_data(skb, pos, skb->len);
		else
			skb_copy_bits(skb, 0, pos, skb->len);
		pos += skb->len;
	} else {
		skb_copy_bits(skb, 0, pos, left);
		skb_copy_bits(skb, left, q->desc, skb->len - left);
		pos = (void *)q->desc + (skb->len - left);
	}

	/*                         */
	p = PTR_ALIGN(pos, 8);
	if ((uintptr_t)p & 8)
		*p = 0;
}

/*
                                                                            
        
 */
static u64 hwcsum(const struct sk_buff *skb)
{
	int csum_type;
	const struct iphdr *iph = ip_hdr(skb);

	if (iph->version == 4) {
		if (iph->protocol == IPPROTO_TCP)
			csum_type = TX_CSUM_TCPIP;
		else if (iph->protocol == IPPROTO_UDP)
			csum_type = TX_CSUM_UDPIP;
		else {
nocsum:			/*
                                       
                                       
    */
			return TXPKT_L4CSUM_DIS;
		}
	} else {
		/*
                                             
   */
		const struct ipv6hdr *ip6h = (const struct ipv6hdr *)iph;

		if (ip6h->nexthdr == IPPROTO_TCP)
			csum_type = TX_CSUM_TCPIP6;
		else if (ip6h->nexthdr == IPPROTO_UDP)
			csum_type = TX_CSUM_UDPIP6;
		else
			goto nocsum;
	}

	if (likely(csum_type >= TX_CSUM_TCPIP))
		return TXPKT_CSUM_TYPE(csum_type) |
			TXPKT_IPHDR_LEN(skb_network_header_len(skb)) |
			TXPKT_ETHHDR_LEN(skb_network_offset(skb) - ETH_HLEN);
	else {
		int start = skb_transport_offset(skb);

		return TXPKT_CSUM_TYPE(csum_type) | TXPKT_CSUM_START(start) |
			TXPKT_CSUM_LOC(start + skb->csum_offset);
	}
}

static void eth_txq_stop(struct sge_eth_txq *q)
{
	netif_tx_stop_queue(q->txq);
	q->q.stops++;
}

static inline void txq_advance(struct sge_txq *q, unsigned int n)
{
	q->in_use += n;
	q->pidx += n;
	if (q->pidx >= q->size)
		q->pidx -= q->size;
}

/* 
                                                     
                   
                              
  
                                                                          
 */
netdev_tx_t t4_eth_xmit(struct sk_buff *skb, struct net_device *dev)
{
	u32 wr_mid;
	u64 cntrl, *end;
	int qidx, credits;
	unsigned int flits, ndesc;
	struct adapter *adap;
	struct sge_eth_txq *q;
	const struct port_info *pi;
	struct fw_eth_tx_pkt_wr *wr;
	struct cpl_tx_pkt_core *cpl;
	const struct skb_shared_info *ssi;
	dma_addr_t addr[MAX_SKB_FRAGS + 1];

	/*
                                                                    
                                             
  */
	if (unlikely(skb->len < ETH_HLEN)) {
out_free:	dev_kfree_skb(skb);
		return NETDEV_TX_OK;
	}

	pi = netdev_priv(dev);
	adap = pi->adapter;
	qidx = skb_get_queue_mapping(skb);
	q = &adap->sge.ethtxq[qidx + pi->first_qset];

	reclaim_completed_tx(adap, &q->q, true);

	flits = calc_tx_flits(skb);
	ndesc = flits_to_desc(flits);
	credits = txq_avail(&q->q) - ndesc;

	if (unlikely(credits < 0)) {
		eth_txq_stop(q);
		dev_err(adap->pdev_dev,
			"%s: Tx ring %u full while queue awake!\n",
			dev->name, qidx);
		return NETDEV_TX_BUSY;
	}

	if (!is_eth_imm(skb) &&
	    unlikely(map_skb(adap->pdev_dev, skb, addr) < 0)) {
		q->mapping_err++;
		goto out_free;
	}

	wr_mid = FW_WR_LEN16(DIV_ROUND_UP(flits, 2));
	if (unlikely(credits < ETHTXQ_STOP_THRES)) {
		eth_txq_stop(q);
		wr_mid |= FW_WR_EQUEQ | FW_WR_EQUIQ;
	}

	wr = (void *)&q->q.desc[q->q.pidx];
	wr->equiq_to_len16 = htonl(wr_mid);
	wr->r3 = cpu_to_be64(0);
	end = (u64 *)wr + flits;

	ssi = skb_shinfo(skb);
	if (ssi->gso_size) {
		struct cpl_tx_pkt_lso *lso = (void *)wr;
		bool v6 = (ssi->gso_type & SKB_GSO_TCPV6) != 0;
		int l3hdr_len = skb_network_header_len(skb);
		int eth_xtra_len = skb_network_offset(skb) - ETH_HLEN;

		wr->op_immdlen = htonl(FW_WR_OP(FW_ETH_TX_PKT_WR) |
				       FW_WR_IMMDLEN(sizeof(*lso)));
		lso->c.lso_ctrl = htonl(LSO_OPCODE(CPL_TX_PKT_LSO) |
					LSO_FIRST_SLICE | LSO_LAST_SLICE |
					LSO_IPV6(v6) |
					LSO_ETHHDR_LEN(eth_xtra_len / 4) |
					LSO_IPHDR_LEN(l3hdr_len / 4) |
					LSO_TCPHDR_LEN(tcp_hdr(skb)->doff));
		lso->c.ipid_ofst = htons(0);
		lso->c.mss = htons(ssi->gso_size);
		lso->c.seqno_offset = htonl(0);
		lso->c.len = htonl(skb->len);
		cpl = (void *)(lso + 1);
		cntrl = TXPKT_CSUM_TYPE(v6 ? TX_CSUM_TCPIP6 : TX_CSUM_TCPIP) |
			TXPKT_IPHDR_LEN(l3hdr_len) |
			TXPKT_ETHHDR_LEN(eth_xtra_len);
		q->tso++;
		q->tx_cso += ssi->gso_segs;
	} else {
		int len;

		len = is_eth_imm(skb) ? skb->len + sizeof(*cpl) : sizeof(*cpl);
		wr->op_immdlen = htonl(FW_WR_OP(FW_ETH_TX_PKT_WR) |
				       FW_WR_IMMDLEN(len));
		cpl = (void *)(wr + 1);
		if (skb->ip_summed == CHECKSUM_PARTIAL) {
			cntrl = hwcsum(skb) | TXPKT_IPCSUM_DIS;
			q->tx_cso++;
		} else
			cntrl = TXPKT_L4CSUM_DIS | TXPKT_IPCSUM_DIS;
	}

	if (vlan_tx_tag_present(skb)) {
		q->vlan_ins++;
		cntrl |= TXPKT_VLAN_VLD | TXPKT_VLAN(vlan_tx_tag_get(skb));
	}

	cpl->ctrl0 = htonl(TXPKT_OPCODE(CPL_TX_PKT_XT) |
			   TXPKT_INTF(pi->tx_chan) | TXPKT_PF(adap->fn));
	cpl->pack = htons(0);
	cpl->len = htons(skb->len);
	cpl->ctrl1 = cpu_to_be64(cntrl);

	if (is_eth_imm(skb)) {
		inline_tx_skb(skb, &q->q, cpl + 1);
		dev_kfree_skb(skb);
	} else {
		int last_desc;

		write_sgl(skb, &q->q, (struct ulptx_sgl *)(cpl + 1), end, 0,
			  addr);
		skb_orphan(skb);

		last_desc = q->q.pidx + ndesc - 1;
		if (last_desc >= q->q.size)
			last_desc -= q->q.size;
		q->q.sdesc[last_desc].skb = skb;
		q->q.sdesc[last_desc].sgl = (struct ulptx_sgl *)(cpl + 1);
	}

	txq_advance(&q->q, ndesc);

	ring_tx_db(adap, &q->q, ndesc);
	return NETDEV_TX_OK;
}

/* 
                                                                      
                               
  
                                                                         
                                                                        
                                            
 */
static inline void reclaim_completed_tx_imm(struct sge_txq *q)
{
	int hw_cidx = ntohs(q->stat->cidx);
	int reclaim = hw_cidx - q->cidx;

	if (reclaim < 0)
		reclaim += q->size;

	q->in_use -= reclaim;
	q->cidx = hw_cidx;
}

/* 
                                                                
                   
  
                                                                    
 */
static inline int is_imm(const struct sk_buff *skb)
{
	return skb->len <= MAX_CTRL_WR_LEN;
}

/* 
                                                                      
                
                                           
  
                                                                  
                                                                           
                                                                        
                                                 
 */
static void ctrlq_check_stop(struct sge_ctrl_txq *q, struct fw_wr_hdr *wr)
{
	reclaim_completed_tx_imm(&q->q);
	if (unlikely(txq_avail(&q->q) < TXQ_STOP_THRES)) {
		wr->lo |= htonl(FW_WR_EQUEQ | FW_WR_EQUIQ);
		q->q.stops++;
		q->full = 1;
	}
}

/* 
                                                            
                        
                   
  
                                                                       
                                                       
 */
static int ctrl_xmit(struct sge_ctrl_txq *q, struct sk_buff *skb)
{
	unsigned int ndesc;
	struct fw_wr_hdr *wr;

	if (unlikely(!is_imm(skb))) {
		WARN_ON(1);
		dev_kfree_skb(skb);
		return NET_XMIT_DROP;
	}

	ndesc = DIV_ROUND_UP(skb->len, sizeof(struct tx_desc));
	spin_lock(&q->sendq.lock);

	if (unlikely(q->full)) {
		skb->priority = ndesc;                  /*                  */
		__skb_queue_tail(&q->sendq, skb);
		spin_unlock(&q->sendq.lock);
		return NET_XMIT_CN;
	}

	wr = (struct fw_wr_hdr *)&q->q.desc[q->q.pidx];
	inline_tx_skb(skb, &q->q, wr);

	txq_advance(&q->q, ndesc);
	if (unlikely(txq_avail(&q->q) < TXQ_STOP_THRES))
		ctrlq_check_stop(q, wr);

	ring_tx_db(q->adap, &q->q, ndesc);
	spin_unlock(&q->sendq.lock);

	kfree_skb(skb);
	return NET_XMIT_SUCCESS;
}

/* 
                                                    
                                      
  
                                                        
 */
static void restart_ctrlq(unsigned long data)
{
	struct sk_buff *skb;
	unsigned int written = 0;
	struct sge_ctrl_txq *q = (struct sge_ctrl_txq *)data;

	spin_lock(&q->sendq.lock);
	reclaim_completed_tx_imm(&q->q);
	BUG_ON(txq_avail(&q->q) < TXQ_STOP_THRES);  /*                   */

	while ((skb = __skb_dequeue(&q->sendq)) != NULL) {
		struct fw_wr_hdr *wr;
		unsigned int ndesc = skb->priority;     /*                  */

		/*
                                                              
                                                                  
   */
		spin_unlock(&q->sendq.lock);

		wr = (struct fw_wr_hdr *)&q->q.desc[q->q.pidx];
		inline_tx_skb(skb, &q->q, wr);
		kfree_skb(skb);

		written += ndesc;
		txq_advance(&q->q, ndesc);
		if (unlikely(txq_avail(&q->q) < TXQ_STOP_THRES)) {
			unsigned long old = q->q.stops;

			ctrlq_check_stop(q, wr);
			if (q->q.stops != old) {          /*                */
				spin_lock(&q->sendq.lock);
				goto ringdb;
			}
		}
		if (written > 16) {
			ring_tx_db(q->adap, &q->q, written);
			written = 0;
		}
		spin_lock(&q->sendq.lock);
	}
	q->full = 0;
ringdb: if (written)
		ring_tx_db(q->adap, &q->q, written);
	spin_unlock(&q->sendq.lock);
}

/* 
                                         
                     
                                                     
  
                                                     
 */
int t4_mgmt_tx(struct adapter *adap, struct sk_buff *skb)
{
	int ret;

	local_bh_disable();
	ret = ctrl_xmit(&adap->sge.ctrlq[0], skb);
	local_bh_enable();
	return ret;
}

/* 
                                                                     
                   
  
                                                                       
                                                                  
 */
static inline int is_ofld_imm(const struct sk_buff *skb)
{
	return skb->len <= MAX_IMM_TX_PKT_LEN;
}

/* 
                                                                  
                   
  
                                                                   
                                                                        
                 
 */
static inline unsigned int calc_tx_flits_ofld(const struct sk_buff *skb)
{
	unsigned int flits, cnt;

	if (is_ofld_imm(skb))
		return DIV_ROUND_UP(skb->len, 8);

	flits = skb_transport_offset(skb) / 8U;   /*         */
	cnt = skb_shinfo(skb)->nr_frags;
	if (skb->tail != skb->transport_header)
		cnt++;
	return flits + sgl_len(cnt);
}

/* 
                                                              
                     
                        
  
                                                                  
                                                                  
                    
 */
static void txq_stop_maperr(struct sge_ofld_txq *q)
{
	q->mapping_err++;
	q->q.stops++;
	set_bit(q->q.cntxt_id - q->adap->sge.egr_start,
		q->adap->sge.txq_maperr);
}

/* 
                                                               
                        
                                                    
  
                                                                         
                                     
 */
static void ofldtxq_stop(struct sge_ofld_txq *q, struct sk_buff *skb)
{
	struct fw_wr_hdr *wr = (struct fw_wr_hdr *)skb->data;

	wr->lo |= htonl(FW_WR_EQUEQ | FW_WR_EQUIQ);
	q->q.stops++;
	q->full = 1;
}

/* 
                                                    
                        
  
                                                                       
                                                                          
 */
static void service_ofldq(struct sge_ofld_txq *q)
{
	u64 *pos;
	int credits;
	struct sk_buff *skb;
	unsigned int written = 0;
	unsigned int flits, ndesc;

	while ((skb = skb_peek(&q->sendq)) != NULL && !q->full) {
		/*
                                                            
                                                
   */
		spin_unlock(&q->sendq.lock);

		reclaim_completed_tx(q->adap, &q->q, false);

		flits = skb->priority;                /*                  */
		ndesc = flits_to_desc(flits);
		credits = txq_avail(&q->q) - ndesc;
		BUG_ON(credits < 0);
		if (unlikely(credits < TXQ_STOP_THRES))
			ofldtxq_stop(q, skb);

		pos = (u64 *)&q->q.desc[q->q.pidx];
		if (is_ofld_imm(skb))
			inline_tx_skb(skb, &q->q, pos);
		else if (map_skb(q->adap->pdev_dev, skb,
				 (dma_addr_t *)skb->head)) {
			txq_stop_maperr(q);
			spin_lock(&q->sendq.lock);
			break;
		} else {
			int last_desc, hdr_len = skb_transport_offset(skb);

			memcpy(pos, skb->data, hdr_len);
			write_sgl(skb, &q->q, (void *)pos + hdr_len,
				  pos + flits, hdr_len,
				  (dma_addr_t *)skb->head);
#ifdef CONFIG_NEED_DMA_MAP_STATE
			skb->dev = q->adap->port[0];
			skb->destructor = deferred_unmap_destructor;
#endif
			last_desc = q->q.pidx + ndesc - 1;
			if (last_desc >= q->q.size)
				last_desc -= q->q.size;
			q->q.sdesc[last_desc].skb = skb;
		}

		txq_advance(&q->q, ndesc);
		written += ndesc;
		if (unlikely(written > 32)) {
			ring_tx_db(q->adap, &q->q, written);
			written = 0;
		}

		spin_lock(&q->sendq.lock);
		__skb_unlink(skb, &q->sendq);
		if (is_ofld_imm(skb))
			kfree_skb(skb);
	}
	if (likely(written))
		ring_tx_db(q->adap, &q->q, written);
}

/* 
                                                     
                           
                   
  
                                                       
 */
static int ofld_xmit(struct sge_ofld_txq *q, struct sk_buff *skb)
{
	skb->priority = calc_tx_flits_ofld(skb);       /*                  */
	spin_lock(&q->sendq.lock);
	__skb_queue_tail(&q->sendq, skb);
	if (q->sendq.qlen == 1)
		service_ofldq(q);
	spin_unlock(&q->sendq.lock);
	return NET_XMIT_SUCCESS;
}

/* 
                                                    
                                      
  
                                                        
 */
static void restart_ofldq(unsigned long data)
{
	struct sge_ofld_txq *q = (struct sge_ofld_txq *)data;

	spin_lock(&q->sendq.lock);
	q->full = 0;            /*                                            */
	service_ofldq(q);
	spin_unlock(&q->sendq.lock);
}

/* 
                                                             
                   
  
                                                                         
                                      
 */
static inline unsigned int skb_txq(const struct sk_buff *skb)
{
	return skb->queue_mapping >> 1;
}

/* 
                                                                     
                   
  
                                                                 
                                                                
 */
static inline unsigned int is_ctrl_pkt(const struct sk_buff *skb)
{
	return skb->queue_mapping & 1;
}

static inline int ofld_send(struct adapter *adap, struct sk_buff *skb)
{
	unsigned int idx = skb_txq(skb);

	if (unlikely(is_ctrl_pkt(skb)))
		return ctrl_xmit(&adap->sge.ctrlq[idx], skb);
	return ofld_xmit(&adap->sge.ofldtxq[idx], skb);
}

/* 
                                        
                     
                   
  
                                                                          
                                                                      
                                                                    
 */
int t4_ofld_send(struct adapter *adap, struct sk_buff *skb)
{
	int ret;

	local_bh_disable();
	ret = ofld_send(adap, skb);
	local_bh_enable();
	return ret;
}

/* 
                                           
                       
                   
  
                                                                          
                     
 */
int cxgb4_ofld_send(struct net_device *dev, struct sk_buff *skb)
{
	return t4_ofld_send(netdev2adap(dev), skb);
}
EXPORT_SYMBOL(cxgb4_ofld_send);

static inline void copy_frags(struct sk_buff *skb,
			      const struct pkt_gl *gl, unsigned int offset)
{
	int i;

	/*                               */
	__skb_fill_page_desc(skb, 0, gl->frags[0].page,
			     gl->frags[0].offset + offset,
			     gl->frags[0].size - offset);
	skb_shinfo(skb)->nr_frags = gl->nfrags;
	for (i = 1; i < gl->nfrags; i++)
		__skb_fill_page_desc(skb, i, gl->frags[i].page,
				     gl->frags[i].offset,
				     gl->frags[i].size);

	/*                                                   */
	get_page(gl->frags[gl->nfrags - 1].page);
}

/* 
                                                                  
                       
                                                              
                                                               
  
                                                                    
                                                 
 */
struct sk_buff *cxgb4_pktgl_to_skb(const struct pkt_gl *gl,
				   unsigned int skb_len, unsigned int pull_len)
{
	struct sk_buff *skb;

	/*
                                                                         
                                                                  
                                                                    
  */
	if (gl->tot_len <= RX_COPY_THRES) {
		skb = dev_alloc_skb(gl->tot_len);
		if (unlikely(!skb))
			goto out;
		__skb_put(skb, gl->tot_len);
		skb_copy_to_linear_data(skb, gl->va, gl->tot_len);
	} else {
		skb = dev_alloc_skb(skb_len);
		if (unlikely(!skb))
			goto out;
		__skb_put(skb, pull_len);
		skb_copy_to_linear_data(skb, gl->va, pull_len);

		copy_frags(skb, gl, pull_len);
		skb->len = gl->tot_len;
		skb->data_len = skb->len - pull_len;
		skb->truesize += skb->data_len;
	}
out:	return skb;
}
EXPORT_SYMBOL(cxgb4_pktgl_to_skb);

/* 
                                            
                       
  
                                                                      
                                       
 */
static void t4_pktgl_free(const struct pkt_gl *gl)
{
	int n;
	const struct page_frag *p;

	for (p = gl->frags, n = gl->nfrags - 1; n--; p++)
		put_page(p->page);
}

/*
                                                                              
                                                               
 */
static noinline int handle_trace_pkt(struct adapter *adap,
				     const struct pkt_gl *gl)
{
	struct sk_buff *skb;

	skb = cxgb4_pktgl_to_skb(gl, RX_PULL_LEN, RX_PULL_LEN);
	if (unlikely(!skb)) {
		t4_pktgl_free(gl);
		return 0;
	}

	if (is_t4(adap->chip))
		__skb_pull(skb, sizeof(struct cpl_trace_pkt));
	else
		__skb_pull(skb, sizeof(struct cpl_t5_trace_pkt));

	skb_reset_mac_header(skb);
	skb->protocol = htons(0xffff);
	skb->dev = adap->port[0];
	netif_receive_skb(skb);
	return 0;
}

static void do_gro(struct sge_eth_rxq *rxq, const struct pkt_gl *gl,
		   const struct cpl_rx_pkt *pkt)
{
	struct adapter *adapter = rxq->rspq.adap;
	struct sge *s = &adapter->sge;
	int ret;
	struct sk_buff *skb;

	skb = napi_get_frags(&rxq->rspq.napi);
	if (unlikely(!skb)) {
		t4_pktgl_free(gl);
		rxq->stats.rx_drops++;
		return;
	}

	copy_frags(skb, gl, s->pktshift);
	skb->len = gl->tot_len - s->pktshift;
	skb->data_len = skb->len;
	skb->truesize += skb->data_len;
	skb->ip_summed = CHECKSUM_UNNECESSARY;
	skb_record_rx_queue(skb, rxq->rspq.idx);
	if (rxq->rspq.netdev->features & NETIF_F_RXHASH)
		skb->rxhash = (__force u32)pkt->rsshdr.hash_val;

	if (unlikely(pkt->vlan_ex)) {
		__vlan_hwaccel_put_tag(skb, htons(ETH_P_8021Q), ntohs(pkt->vlan));
		rxq->stats.vlan_ex++;
	}
	ret = napi_gro_frags(&rxq->rspq.napi);
	if (ret == GRO_HELD)
		rxq->stats.lro_pkts++;
	else if (ret == GRO_MERGED || ret == GRO_MERGED_FREE)
		rxq->stats.lro_merged++;
	rxq->stats.pkts++;
	rxq->stats.rx_cso++;
}

/* 
                                                        
                                                  
                                                                 
                                           
  
                                                                  
 */
int t4_ethrx_handler(struct sge_rspq *q, const __be64 *rsp,
		     const struct pkt_gl *si)
{
	bool csum_ok;
	struct sk_buff *skb;
	const struct cpl_rx_pkt *pkt;
	struct sge_eth_rxq *rxq = container_of(q, struct sge_eth_rxq, rspq);
	struct sge *s = &q->adap->sge;
	int cpl_trace_pkt = is_t4(q->adap->chip) ?
			    CPL_TRACE_PKT : CPL_TRACE_PKT_T5;

	if (unlikely(*(u8 *)rsp == cpl_trace_pkt))
		return handle_trace_pkt(q->adap, si);

	pkt = (const struct cpl_rx_pkt *)rsp;
	csum_ok = pkt->csum_calc && !pkt->err_vec;
	if ((pkt->l2info & htonl(RXF_TCP)) &&
	    (q->netdev->features & NETIF_F_GRO) && csum_ok && !pkt->ip_frag) {
		do_gro(rxq, si, pkt);
		return 0;
	}

	skb = cxgb4_pktgl_to_skb(si, RX_PKT_SKB_LEN, RX_PULL_LEN);
	if (unlikely(!skb)) {
		t4_pktgl_free(si);
		rxq->stats.rx_drops++;
		return 0;
	}

	__skb_pull(skb, s->pktshift);      /*                                */
	skb->protocol = eth_type_trans(skb, q->netdev);
	skb_record_rx_queue(skb, q->idx);
	if (skb->dev->features & NETIF_F_RXHASH)
		skb->rxhash = (__force u32)pkt->rsshdr.hash_val;

	rxq->stats.pkts++;

	if (csum_ok && (q->netdev->features & NETIF_F_RXCSUM) &&
	    (pkt->l2info & htonl(RXF_UDP | RXF_TCP))) {
		if (!pkt->ip_frag) {
			skb->ip_summed = CHECKSUM_UNNECESSARY;
			rxq->stats.rx_cso++;
		} else if (pkt->l2info & htonl(RXF_IP)) {
			__sum16 c = (__force __sum16)pkt->csum;
			skb->csum = csum_unfold(c);
			skb->ip_summed = CHECKSUM_COMPLETE;
			rxq->stats.rx_cso++;
		}
	} else
		skb_checksum_none_assert(skb);

	if (unlikely(pkt->vlan_ex)) {
		__vlan_hwaccel_put_tag(skb, htons(ETH_P_8021Q), ntohs(pkt->vlan));
		rxq->stats.vlan_ex++;
	}
	netif_receive_skb(skb);
	return 0;
}

/* 
                                                   
                              
                        
                                          
  
                                                                      
                                                                       
                                      
  
                                                                        
                                                                        
                                                                        
                                                 
 */
static void restore_rx_bufs(const struct pkt_gl *si, struct sge_fl *q,
			    int frags)
{
	struct rx_sw_desc *d;

	while (frags--) {
		if (q->cidx == 0)
			q->cidx = q->size - 1;
		else
			q->cidx--;
		d = &q->sdesc[q->cidx];
		d->page = si->frags[frags].page;
		d->dma_addr |= RX_UNMAPPED_BUF;
		q->avail++;
	}
}

/* 
                                                         
                              
                         
  
                                                                   
            
 */
static inline bool is_new_response(const struct rsp_ctrl *r,
				   const struct sge_rspq *q)
{
	return RSPD_GEN(r->type_gen) == q->gen;
}

/* 
                                                            
                
  
                                                                         
 */
static inline void rspq_next(struct sge_rspq *q)
{
	q->cur_desc = (void *)q->cur_desc + q->iqe_len;
	if (unlikely(++q->cidx == q->size)) {
		q->cidx = 0;
		q->gen ^= 1;
		q->cur_desc = q->desc;
	}
}

/* 
                                                                   
                                   
                                                             
  
                                                                          
                                                                         
         
  
                                                                        
                                                                      
                               
 */
static int process_responses(struct sge_rspq *q, int budget)
{
	int ret, rsp_type;
	int budget_left = budget;
	const struct rsp_ctrl *rc;
	struct sge_eth_rxq *rxq = container_of(q, struct sge_eth_rxq, rspq);
	struct adapter *adapter = q->adap;
	struct sge *s = &adapter->sge;

	while (likely(budget_left)) {
		rc = (void *)q->cur_desc + (q->iqe_len - sizeof(*rc));
		if (!is_new_response(rc, q))
			break;

		rmb();
		rsp_type = RSPD_TYPE(rc->type_gen);
		if (likely(rsp_type == RSP_TYPE_FLBUF)) {
			struct page_frag *fp;
			struct pkt_gl si;
			const struct rx_sw_desc *rsd;
			u32 len = ntohl(rc->pldbuflen_qid), bufsz, frags;

			if (len & RSPD_NEWBUF) {
				if (likely(q->offset > 0)) {
					free_rx_bufs(q->adap, &rxq->fl, 1);
					q->offset = 0;
				}
				len = RSPD_LEN(len);
			}
			si.tot_len = len;

			/*                         */
			for (frags = 0, fp = si.frags; ; frags++, fp++) {
				rsd = &rxq->fl.sdesc[rxq->fl.cidx];
				bufsz = get_buf_size(adapter, rsd);
				fp->page = rsd->page;
				fp->offset = q->offset;
				fp->size = min(bufsz, len);
				len -= fp->size;
				if (!len)
					break;
				unmap_rx_buf(q->adap, &rxq->fl);
			}

			/*
                                                      
                              
    */
			dma_sync_single_for_cpu(q->adap->pdev_dev,
						get_buf_addr(rsd),
						fp->size, DMA_FROM_DEVICE);

			si.va = page_address(si.frags[0].page) +
				si.frags[0].offset;
			prefetch(si.va);

			si.nfrags = frags + 1;
			ret = q->handler(q, q->cur_desc, &si);
			if (likely(ret == 0))
				q->offset += ALIGN(fp->size, s->fl_align);
			else
				restore_rx_bufs(&si, &rxq->fl, frags);
		} else if (likely(rsp_type == RSP_TYPE_CPL)) {
			ret = q->handler(q, q->cur_desc, NULL);
		} else {
			ret = q->handler(q, (const __be64 *)rc, CXGB4_MSG_AN);
		}

		if (unlikely(ret)) {
			/*                                                    */
			q->next_intr_params = QINTR_TIMER_IDX(NOMEM_TMR_IDX);
			break;
		}

		rspq_next(q);
		budget_left--;
	}

	if (q->offset >= 0 && rxq->fl.size - rxq->fl.avail >= 16)
		__refill_fl(q->adap, &rxq->fl);
	return budget - budget_left;
}

/* 
                                                       
                           
                                                         
  
                                                                       
                                                                      
                                                                       
                                                                      
                       
 */
static int napi_rx_handler(struct napi_struct *napi, int budget)
{
	unsigned int params;
	struct sge_rspq *q = container_of(napi, struct sge_rspq, napi);
	int work_done = process_responses(q, budget);

	if (likely(work_done < budget)) {
		napi_complete(napi);
		params = q->next_intr_params;
		q->next_intr_params = q->intr_params;
	} else
		params = QINTR_TIMER_IDX(7);

	t4_write_reg(q->adap, MYPF_REG(SGE_PF_GTS), CIDXINC(work_done) |
		     INGRESSQID((u32)q->cntxt_id) | SEINTARM(params));
	return work_done;
}

/*
                                                         
 */
irqreturn_t t4_sge_intr_msix(int irq, void *cookie)
{
	struct sge_rspq *q = cookie;

	napi_schedule(&q->napi);
	return IRQ_HANDLED;
}

/*
                                                                             
                                                   
 */
static unsigned int process_intrq(struct adapter *adap)
{
	unsigned int credits;
	const struct rsp_ctrl *rc;
	struct sge_rspq *q = &adap->sge.intrq;

	spin_lock(&adap->sge.intrq_lock);
	for (credits = 0; ; credits++) {
		rc = (void *)q->cur_desc + (q->iqe_len - sizeof(*rc));
		if (!is_new_response(rc, q))
			break;

		rmb();
		if (RSPD_TYPE(rc->type_gen) == RSP_TYPE_INTR) {
			unsigned int qid = ntohl(rc->pldbuflen_qid);

			qid -= adap->sge.ingr_start;
			napi_schedule(&adap->sge.ingr_map[qid]->napi);
		}

		rspq_next(q);
	}

	t4_write_reg(adap, MYPF_REG(SGE_PF_GTS), CIDXINC(credits) |
		     INGRESSQID(q->cntxt_id) | SEINTARM(q->intr_params));
	spin_unlock(&adap->sge.intrq_lock);
	return credits;
}

/*
                                                                                
                                                                               
 */
static irqreturn_t t4_intr_msi(int irq, void *cookie)
{
	struct adapter *adap = cookie;

	t4_slow_intr_handler(adap);
	process_intrq(adap);
	return IRQ_HANDLED;
}

/*
                                                
                                                                          
                                                        
 */
static irqreturn_t t4_intr_intx(int irq, void *cookie)
{
	struct adapter *adap = cookie;

	t4_write_reg(adap, MYPF_REG(PCIE_PF_CLI), 0);
	if (t4_slow_intr_handler(adap) | process_intrq(adap))
		return IRQ_HANDLED;
	return IRQ_NONE;             /*                           */
}

/* 
                                                           
                     
  
                                                                          
                         
 */
irq_handler_t t4_intr_handler(struct adapter *adap)
{
	if (adap->flags & USING_MSIX)
		return t4_sge_intr_msix;
	if (adap->flags & USING_MSI)
		return t4_intr_msi;
	return t4_intr_intx;
}

static void sge_rx_timer_cb(unsigned long data)
{
	unsigned long m;
	unsigned int i, cnt[2];
	struct adapter *adap = (struct adapter *)data;
	struct sge *s = &adap->sge;

	for (i = 0; i < ARRAY_SIZE(s->starving_fl); i++)
		for (m = s->starving_fl[i]; m; m &= m - 1) {
			struct sge_eth_rxq *rxq;
			unsigned int id = __ffs(m) + i * BITS_PER_LONG;
			struct sge_fl *fl = s->egr_map[id];

			clear_bit(id, s->starving_fl);
			smp_mb__after_clear_bit();

			if (fl_starving(fl)) {
				rxq = container_of(fl, struct sge_eth_rxq, fl);
				if (napi_reschedule(&rxq->rspq.napi))
					fl->starving++;
				else
					set_bit(id, s->starving_fl);
			}
		}

	t4_write_reg(adap, SGE_DEBUG_INDEX, 13);
	cnt[0] = t4_read_reg(adap, SGE_DEBUG_DATA_HIGH);
	cnt[1] = t4_read_reg(adap, SGE_DEBUG_DATA_LOW);

	for (i = 0; i < 2; i++)
		if (cnt[i] >= s->starve_thres) {
			if (s->idma_state[i] || cnt[i] == 0xffffffff)
				continue;
			s->idma_state[i] = 1;
			t4_write_reg(adap, SGE_DEBUG_INDEX, 11);
			m = t4_read_reg(adap, SGE_DEBUG_DATA_LOW) >> (i * 16);
			dev_warn(adap->pdev_dev,
				 "SGE idma%u starvation detected for "
				 "queue %lu\n", i, m & 0xffff);
		} else if (s->idma_state[i])
			s->idma_state[i] = 0;

	mod_timer(&s->rx_timer, jiffies + RX_QCHECK_PERIOD);
}

static void sge_tx_timer_cb(unsigned long data)
{
	unsigned long m;
	unsigned int i, budget;
	struct adapter *adap = (struct adapter *)data;
	struct sge *s = &adap->sge;

	for (i = 0; i < ARRAY_SIZE(s->txq_maperr); i++)
		for (m = s->txq_maperr[i]; m; m &= m - 1) {
			unsigned long id = __ffs(m) + i * BITS_PER_LONG;
			struct sge_ofld_txq *txq = s->egr_map[id];

			clear_bit(id, s->txq_maperr);
			tasklet_schedule(&txq->qresume_tsk);
		}

	budget = MAX_TIMER_TX_RECLAIM;
	i = s->ethtxq_rover;
	do {
		struct sge_eth_txq *q = &s->ethtxq[i];

		if (q->q.in_use &&
		    time_after_eq(jiffies, q->txq->trans_start + HZ / 100) &&
		    __netif_tx_trylock(q->txq)) {
			int avail = reclaimable(&q->q);

			if (avail) {
				if (avail > budget)
					avail = budget;

				free_tx_desc(adap, &q->q, avail, true);
				q->q.in_use -= avail;
				budget -= avail;
			}
			__netif_tx_unlock(q->txq);
		}

		if (++i >= s->ethqsets)
			i = 0;
	} while (budget && i != s->ethtxq_rover);
	s->ethtxq_rover = i;
	mod_timer(&s->tx_timer, jiffies + (budget ? TX_QCHECK_PERIOD : 2));
}

int t4_sge_alloc_rxq(struct adapter *adap, struct sge_rspq *iq, bool fwevtq,
		     struct net_device *dev, int intr_idx,
		     struct sge_fl *fl, rspq_handler_t hnd)
{
	int ret, flsz = 0;
	struct fw_iq_cmd c;
	struct sge *s = &adap->sge;
	struct port_info *pi = netdev_priv(dev);

	/*                                                          */
	iq->size = roundup(iq->size, 16);

	iq->desc = alloc_ring(adap->pdev_dev, iq->size, iq->iqe_len, 0,
			      &iq->phys_addr, NULL, 0, NUMA_NO_NODE);
	if (!iq->desc)
		return -ENOMEM;

	memset(&c, 0, sizeof(c));
	c.op_to_vfn = htonl(FW_CMD_OP(FW_IQ_CMD) | FW_CMD_REQUEST |
			    FW_CMD_WRITE | FW_CMD_EXEC |
			    FW_IQ_CMD_PFN(adap->fn) | FW_IQ_CMD_VFN(0));
	c.alloc_to_len16 = htonl(FW_IQ_CMD_ALLOC | FW_IQ_CMD_IQSTART(1) |
				 FW_LEN16(c));
	c.type_to_iqandstindex = htonl(FW_IQ_CMD_TYPE(FW_IQ_TYPE_FL_INT_CAP) |
		FW_IQ_CMD_IQASYNCH(fwevtq) | FW_IQ_CMD_VIID(pi->viid) |
		FW_IQ_CMD_IQANDST(intr_idx < 0) | FW_IQ_CMD_IQANUD(1) |
		FW_IQ_CMD_IQANDSTINDEX(intr_idx >= 0 ? intr_idx :
							-intr_idx - 1));
	c.iqdroprss_to_iqesize = htons(FW_IQ_CMD_IQPCIECH(pi->tx_chan) |
		FW_IQ_CMD_IQGTSMODE |
		FW_IQ_CMD_IQINTCNTTHRESH(iq->pktcnt_idx) |
		FW_IQ_CMD_IQESIZE(ilog2(iq->iqe_len) - 4));
	c.iqsize = htons(iq->size);
	c.iqaddr = cpu_to_be64(iq->phys_addr);

	if (fl) {
		fl->size = roundup(fl->size, 8);
		fl->desc = alloc_ring(adap->pdev_dev, fl->size, sizeof(__be64),
				      sizeof(struct rx_sw_desc), &fl->addr,
				      &fl->sdesc, s->stat_len, NUMA_NO_NODE);
		if (!fl->desc)
			goto fl_nomem;

		flsz = fl->size / 8 + s->stat_len / sizeof(struct tx_desc);
		c.iqns_to_fl0congen = htonl(FW_IQ_CMD_FL0PACKEN(1) |
					    FW_IQ_CMD_FL0FETCHRO(1) |
					    FW_IQ_CMD_FL0DATARO(1) |
					    FW_IQ_CMD_FL0PADEN(1));
		c.fl0dcaen_to_fl0cidxfthresh = htons(FW_IQ_CMD_FL0FBMIN(2) |
				FW_IQ_CMD_FL0FBMAX(3));
		c.fl0size = htons(flsz);
		c.fl0addr = cpu_to_be64(fl->addr);
	}

	ret = t4_wr_mbox(adap, adap->fn, &c, sizeof(c), &c);
	if (ret)
		goto err;

	netif_napi_add(dev, &iq->napi, napi_rx_handler, 64);
	iq->cur_desc = iq->desc;
	iq->cidx = 0;
	iq->gen = 1;
	iq->next_intr_params = iq->intr_params;
	iq->cntxt_id = ntohs(c.iqid);
	iq->abs_id = ntohs(c.physiqid);
	iq->size--;                           /*                       */
	iq->adap = adap;
	iq->netdev = dev;
	iq->handler = hnd;

	/*                                                           */
	iq->offset = fl ? 0 : -1;

	adap->sge.ingr_map[iq->cntxt_id - adap->sge.ingr_start] = iq;

	if (fl) {
		fl->cntxt_id = ntohs(c.fl0id);
		fl->avail = fl->pend_cred = 0;
		fl->pidx = fl->cidx = 0;
		fl->alloc_failed = fl->large_alloc_failed = fl->starving = 0;
		adap->sge.egr_map[fl->cntxt_id - adap->sge.egr_start] = fl;
		refill_fl(adap, fl, fl_cap(fl), GFP_KERNEL);
	}
	return 0;

fl_nomem:
	ret = -ENOMEM;
err:
	if (iq->desc) {
		dma_free_coherent(adap->pdev_dev, iq->size * iq->iqe_len,
				  iq->desc, iq->phys_addr);
		iq->desc = NULL;
	}
	if (fl && fl->desc) {
		kfree(fl->sdesc);
		fl->sdesc = NULL;
		dma_free_coherent(adap->pdev_dev, flsz * sizeof(struct tx_desc),
				  fl->desc, fl->addr);
		fl->desc = NULL;
	}
	return ret;
}

static void init_txq(struct adapter *adap, struct sge_txq *q, unsigned int id)
{
	q->cntxt_id = id;
	if (!is_t4(adap->chip)) {
		unsigned int s_qpp;
		unsigned short udb_density;
		unsigned long qpshift;
		int page;

		s_qpp = QUEUESPERPAGEPF1 * adap->fn;
		udb_density = 1 << QUEUESPERPAGEPF0_GET((t4_read_reg(adap,
				SGE_EGRESS_QUEUES_PER_PAGE_PF) >> s_qpp));
		qpshift = PAGE_SHIFT - ilog2(udb_density);
		q->udb = q->cntxt_id << qpshift;
		q->udb &= PAGE_MASK;
		page = q->udb / PAGE_SIZE;
		q->udb += (q->cntxt_id - (page * udb_density)) * 128;
	}

	q->in_use = 0;
	q->cidx = q->pidx = 0;
	q->stops = q->restarts = 0;
	q->stat = (void *)&q->desc[q->size];
	spin_lock_init(&q->db_lock);
	adap->sge.egr_map[id - adap->sge.egr_start] = q;
}

int t4_sge_alloc_eth_txq(struct adapter *adap, struct sge_eth_txq *txq,
			 struct net_device *dev, struct netdev_queue *netdevq,
			 unsigned int iqid)
{
	int ret, nentries;
	struct fw_eq_eth_cmd c;
	struct sge *s = &adap->sge;
	struct port_info *pi = netdev_priv(dev);

	/*                    */
	nentries = txq->q.size + s->stat_len / sizeof(struct tx_desc);

	txq->q.desc = alloc_ring(adap->pdev_dev, txq->q.size,
			sizeof(struct tx_desc), sizeof(struct tx_sw_desc),
			&txq->q.phys_addr, &txq->q.sdesc, s->stat_len,
			netdev_queue_numa_node_read(netdevq));
	if (!txq->q.desc)
		return -ENOMEM;

	memset(&c, 0, sizeof(c));
	c.op_to_vfn = htonl(FW_CMD_OP(FW_EQ_ETH_CMD) | FW_CMD_REQUEST |
			    FW_CMD_WRITE | FW_CMD_EXEC |
			    FW_EQ_ETH_CMD_PFN(adap->fn) | FW_EQ_ETH_CMD_VFN(0));
	c.alloc_to_len16 = htonl(FW_EQ_ETH_CMD_ALLOC |
				 FW_EQ_ETH_CMD_EQSTART | FW_LEN16(c));
	c.viid_pkd = htonl(FW_EQ_ETH_CMD_VIID(pi->viid));
	c.fetchszm_to_iqid = htonl(FW_EQ_ETH_CMD_HOSTFCMODE(2) |
				   FW_EQ_ETH_CMD_PCIECHN(pi->tx_chan) |
				   FW_EQ_ETH_CMD_FETCHRO(1) |
				   FW_EQ_ETH_CMD_IQID(iqid));
	c.dcaen_to_eqsize = htonl(FW_EQ_ETH_CMD_FBMIN(2) |
				  FW_EQ_ETH_CMD_FBMAX(3) |
				  FW_EQ_ETH_CMD_CIDXFTHRESH(5) |
				  FW_EQ_ETH_CMD_EQSIZE(nentries));
	c.eqaddr = cpu_to_be64(txq->q.phys_addr);

	ret = t4_wr_mbox(adap, adap->fn, &c, sizeof(c), &c);
	if (ret) {
		kfree(txq->q.sdesc);
		txq->q.sdesc = NULL;
		dma_free_coherent(adap->pdev_dev,
				  nentries * sizeof(struct tx_desc),
				  txq->q.desc, txq->q.phys_addr);
		txq->q.desc = NULL;
		return ret;
	}

	init_txq(adap, &txq->q, FW_EQ_ETH_CMD_EQID_GET(ntohl(c.eqid_pkd)));
	txq->txq = netdevq;
	txq->tso = txq->tx_cso = txq->vlan_ins = 0;
	txq->mapping_err = 0;
	return 0;
}

int t4_sge_alloc_ctrl_txq(struct adapter *adap, struct sge_ctrl_txq *txq,
			  struct net_device *dev, unsigned int iqid,
			  unsigned int cmplqid)
{
	int ret, nentries;
	struct fw_eq_ctrl_cmd c;
	struct sge *s = &adap->sge;
	struct port_info *pi = netdev_priv(dev);

	/*                    */
	nentries = txq->q.size + s->stat_len / sizeof(struct tx_desc);

	txq->q.desc = alloc_ring(adap->pdev_dev, nentries,
				 sizeof(struct tx_desc), 0, &txq->q.phys_addr,
				 NULL, 0, NUMA_NO_NODE);
	if (!txq->q.desc)
		return -ENOMEM;

	c.op_to_vfn = htonl(FW_CMD_OP(FW_EQ_CTRL_CMD) | FW_CMD_REQUEST |
			    FW_CMD_WRITE | FW_CMD_EXEC |
			    FW_EQ_CTRL_CMD_PFN(adap->fn) |
			    FW_EQ_CTRL_CMD_VFN(0));
	c.alloc_to_len16 = htonl(FW_EQ_CTRL_CMD_ALLOC |
				 FW_EQ_CTRL_CMD_EQSTART | FW_LEN16(c));
	c.cmpliqid_eqid = htonl(FW_EQ_CTRL_CMD_CMPLIQID(cmplqid));
	c.physeqid_pkd = htonl(0);
	c.fetchszm_to_iqid = htonl(FW_EQ_CTRL_CMD_HOSTFCMODE(2) |
				   FW_EQ_CTRL_CMD_PCIECHN(pi->tx_chan) |
				   FW_EQ_CTRL_CMD_FETCHRO |
				   FW_EQ_CTRL_CMD_IQID(iqid));
	c.dcaen_to_eqsize = htonl(FW_EQ_CTRL_CMD_FBMIN(2) |
				  FW_EQ_CTRL_CMD_FBMAX(3) |
				  FW_EQ_CTRL_CMD_CIDXFTHRESH(5) |
				  FW_EQ_CTRL_CMD_EQSIZE(nentries));
	c.eqaddr = cpu_to_be64(txq->q.phys_addr);

	ret = t4_wr_mbox(adap, adap->fn, &c, sizeof(c), &c);
	if (ret) {
		dma_free_coherent(adap->pdev_dev,
				  nentries * sizeof(struct tx_desc),
				  txq->q.desc, txq->q.phys_addr);
		txq->q.desc = NULL;
		return ret;
	}

	init_txq(adap, &txq->q, FW_EQ_CTRL_CMD_EQID_GET(ntohl(c.cmpliqid_eqid)));
	txq->adap = adap;
	skb_queue_head_init(&txq->sendq);
	tasklet_init(&txq->qresume_tsk, restart_ctrlq, (unsigned long)txq);
	txq->full = 0;
	return 0;
}

int t4_sge_alloc_ofld_txq(struct adapter *adap, struct sge_ofld_txq *txq,
			  struct net_device *dev, unsigned int iqid)
{
	int ret, nentries;
	struct fw_eq_ofld_cmd c;
	struct sge *s = &adap->sge;
	struct port_info *pi = netdev_priv(dev);

	/*                    */
	nentries = txq->q.size + s->stat_len / sizeof(struct tx_desc);

	txq->q.desc = alloc_ring(adap->pdev_dev, txq->q.size,
			sizeof(struct tx_desc), sizeof(struct tx_sw_desc),
			&txq->q.phys_addr, &txq->q.sdesc, s->stat_len,
			NUMA_NO_NODE);
	if (!txq->q.desc)
		return -ENOMEM;

	memset(&c, 0, sizeof(c));
	c.op_to_vfn = htonl(FW_CMD_OP(FW_EQ_OFLD_CMD) | FW_CMD_REQUEST |
			    FW_CMD_WRITE | FW_CMD_EXEC |
			    FW_EQ_OFLD_CMD_PFN(adap->fn) |
			    FW_EQ_OFLD_CMD_VFN(0));
	c.alloc_to_len16 = htonl(FW_EQ_OFLD_CMD_ALLOC |
				 FW_EQ_OFLD_CMD_EQSTART | FW_LEN16(c));
	c.fetchszm_to_iqid = htonl(FW_EQ_OFLD_CMD_HOSTFCMODE(2) |
				   FW_EQ_OFLD_CMD_PCIECHN(pi->tx_chan) |
				   FW_EQ_OFLD_CMD_FETCHRO(1) |
				   FW_EQ_OFLD_CMD_IQID(iqid));
	c.dcaen_to_eqsize = htonl(FW_EQ_OFLD_CMD_FBMIN(2) |
				  FW_EQ_OFLD_CMD_FBMAX(3) |
				  FW_EQ_OFLD_CMD_CIDXFTHRESH(5) |
				  FW_EQ_OFLD_CMD_EQSIZE(nentries));
	c.eqaddr = cpu_to_be64(txq->q.phys_addr);

	ret = t4_wr_mbox(adap, adap->fn, &c, sizeof(c), &c);
	if (ret) {
		kfree(txq->q.sdesc);
		txq->q.sdesc = NULL;
		dma_free_coherent(adap->pdev_dev,
				  nentries * sizeof(struct tx_desc),
				  txq->q.desc, txq->q.phys_addr);
		txq->q.desc = NULL;
		return ret;
	}

	init_txq(adap, &txq->q, FW_EQ_OFLD_CMD_EQID_GET(ntohl(c.eqid_pkd)));
	txq->adap = adap;
	skb_queue_head_init(&txq->sendq);
	tasklet_init(&txq->qresume_tsk, restart_ofldq, (unsigned long)txq);
	txq->full = 0;
	txq->mapping_err = 0;
	return 0;
}

static void free_txq(struct adapter *adap, struct sge_txq *q)
{
	struct sge *s = &adap->sge;

	dma_free_coherent(adap->pdev_dev,
			  q->size * sizeof(struct tx_desc) + s->stat_len,
			  q->desc, q->phys_addr);
	q->cntxt_id = 0;
	q->sdesc = NULL;
	q->desc = NULL;
}

static void free_rspq_fl(struct adapter *adap, struct sge_rspq *rq,
			 struct sge_fl *fl)
{
	struct sge *s = &adap->sge;
	unsigned int fl_id = fl ? fl->cntxt_id : 0xffff;

	adap->sge.ingr_map[rq->cntxt_id - adap->sge.ingr_start] = NULL;
	t4_iq_free(adap, adap->fn, adap->fn, 0, FW_IQ_TYPE_FL_INT_CAP,
		   rq->cntxt_id, fl_id, 0xffff);
	dma_free_coherent(adap->pdev_dev, (rq->size + 1) * rq->iqe_len,
			  rq->desc, rq->phys_addr);
	netif_napi_del(&rq->napi);
	rq->netdev = NULL;
	rq->cntxt_id = rq->abs_id = 0;
	rq->desc = NULL;

	if (fl) {
		free_rx_bufs(adap, fl, fl->avail);
		dma_free_coherent(adap->pdev_dev, fl->size * 8 + s->stat_len,
				  fl->desc, fl->addr);
		kfree(fl->sdesc);
		fl->sdesc = NULL;
		fl->cntxt_id = 0;
		fl->desc = NULL;
	}
}

/* 
                                             
                     
  
                                              
 */
void t4_free_sge_resources(struct adapter *adap)
{
	int i;
	struct sge_eth_rxq *eq = adap->sge.ethrxq;
	struct sge_eth_txq *etq = adap->sge.ethtxq;
	struct sge_ofld_rxq *oq = adap->sge.ofldrxq;

	/*                                */
	for (i = 0; i < adap->sge.ethqsets; i++, eq++, etq++) {
		if (eq->rspq.desc)
			free_rspq_fl(adap, &eq->rspq, &eq->fl);
		if (etq->q.desc) {
			t4_eth_eq_free(adap, adap->fn, adap->fn, 0,
				       etq->q.cntxt_id);
			free_tx_desc(adap, &etq->q, etq->q.in_use, true);
			kfree(etq->q.sdesc);
			free_txq(adap, &etq->q);
		}
	}

	/*                                   */
	for (i = 0; i < adap->sge.ofldqsets; i++, oq++) {
		if (oq->rspq.desc)
			free_rspq_fl(adap, &oq->rspq, &oq->fl);
	}
	for (i = 0, oq = adap->sge.rdmarxq; i < adap->sge.rdmaqs; i++, oq++) {
		if (oq->rspq.desc)
			free_rspq_fl(adap, &oq->rspq, &oq->fl);
	}

	/*                            */
	for (i = 0; i < ARRAY_SIZE(adap->sge.ofldtxq); i++) {
		struct sge_ofld_txq *q = &adap->sge.ofldtxq[i];

		if (q->q.desc) {
			tasklet_kill(&q->qresume_tsk);
			t4_ofld_eq_free(adap, adap->fn, adap->fn, 0,
					q->q.cntxt_id);
			free_tx_desc(adap, &q->q, q->q.in_use, false);
			kfree(q->q.sdesc);
			__skb_queue_purge(&q->sendq);
			free_txq(adap, &q->q);
		}
	}

	/*                            */
	for (i = 0; i < ARRAY_SIZE(adap->sge.ctrlq); i++) {
		struct sge_ctrl_txq *cq = &adap->sge.ctrlq[i];

		if (cq->q.desc) {
			tasklet_kill(&cq->qresume_tsk);
			t4_ctrl_eq_free(adap, adap->fn, adap->fn, 0,
					cq->q.cntxt_id);
			__skb_queue_purge(&cq->sendq);
			free_txq(adap, &cq->q);
		}
	}

	if (adap->sge.fw_evtq.desc)
		free_rspq_fl(adap, &adap->sge.fw_evtq, NULL);

	if (adap->sge.intrq.desc)
		free_rspq_fl(adap, &adap->sge.intrq, NULL);

	/*                                    */
	memset(adap->sge.egr_map, 0, sizeof(adap->sge.egr_map));
}

void t4_sge_start(struct adapter *adap)
{
	adap->sge.ethtxq_rover = 0;
	mod_timer(&adap->sge.rx_timer, jiffies + RX_QCHECK_PERIOD);
	mod_timer(&adap->sge.tx_timer, jiffies + TX_QCHECK_PERIOD);
}

/* 
                                      
                     
  
                                                                      
                                                                       
                                
 */
void t4_sge_stop(struct adapter *adap)
{
	int i;
	struct sge *s = &adap->sge;

	if (in_interrupt())  /*                               */
		return;

	if (s->rx_timer.function)
		del_timer_sync(&s->rx_timer);
	if (s->tx_timer.function)
		del_timer_sync(&s->tx_timer);

	for (i = 0; i < ARRAY_SIZE(s->ofldtxq); i++) {
		struct sge_ofld_txq *q = &s->ofldtxq[i];

		if (q->q.desc)
			tasklet_kill(&q->qresume_tsk);
	}
	for (i = 0; i < ARRAY_SIZE(s->ctrlq); i++) {
		struct sge_ctrl_txq *cq = &s->ctrlq[i];

		if (cq->q.desc)
			tasklet_kill(&cq->qresume_tsk);
	}
}

/* 
                               
                     
  
                                                                    
                                                                  
                                            
  
                                 
  
                                                                   
                                                                 
                                                                   
                            
  
                                                                   
                                                                   
                                                                  
                                                                  
 */

static int t4_sge_init_soft(struct adapter *adap)
{
	struct sge *s = &adap->sge;
	u32 fl_small_pg, fl_large_pg, fl_small_mtu, fl_large_mtu;
	u32 timer_value_0_and_1, timer_value_2_and_3, timer_value_4_and_5;
	u32 ingress_rx_threshold;

	/*
                                                               
                                                                 
               
  */
	if ((t4_read_reg(adap, SGE_CONTROL) & RXPKTCPLMODE_MASK) !=
	    RXPKTCPLMODE(X_RXPKTCPLMODE_SPLIT)) {
		dev_err(adap->pdev_dev, "bad SGE CPL MODE\n");
		return -EINVAL;
	}

	/*
                                                                   
           
   
                                                                    
                                                                     
                       
  */
	#define READ_FL_BUF(x) \
		t4_read_reg(adap, SGE_FL_BUFFER_SIZE0+(x)*sizeof(u32))

	fl_small_pg = READ_FL_BUF(RX_SMALL_PG_BUF);
	fl_large_pg = READ_FL_BUF(RX_LARGE_PG_BUF);
	fl_small_mtu = READ_FL_BUF(RX_SMALL_MTU_BUF);
	fl_large_mtu = READ_FL_BUF(RX_LARGE_MTU_BUF);

	#undef READ_FL_BUF

	if (fl_small_pg != PAGE_SIZE ||
	    (fl_large_pg != 0 && (fl_large_pg <= fl_small_pg ||
				  (fl_large_pg & (fl_large_pg-1)) != 0))) {
		dev_err(adap->pdev_dev, "bad SGE FL page buffer sizes [%d, %d]\n",
			fl_small_pg, fl_large_pg);
		return -EINVAL;
	}
	if (fl_large_pg)
		s->fl_pg_order = ilog2(fl_large_pg) - PAGE_SHIFT;

	if (fl_small_mtu < FL_MTU_SMALL_BUFSIZE(adap) ||
	    fl_large_mtu < FL_MTU_LARGE_BUFSIZE(adap)) {
		dev_err(adap->pdev_dev, "bad SGE FL MTU sizes [%d, %d]\n",
			fl_small_mtu, fl_large_mtu);
		return -EINVAL;
	}

	/*
                                                              
                                             
  */
	timer_value_0_and_1 = t4_read_reg(adap, SGE_TIMER_VALUE_0_AND_1);
	timer_value_2_and_3 = t4_read_reg(adap, SGE_TIMER_VALUE_2_AND_3);
	timer_value_4_and_5 = t4_read_reg(adap, SGE_TIMER_VALUE_4_AND_5);
	s->timer_val[0] = core_ticks_to_us(adap,
		TIMERVALUE0_GET(timer_value_0_and_1));
	s->timer_val[1] = core_ticks_to_us(adap,
		TIMERVALUE1_GET(timer_value_0_and_1));
	s->timer_val[2] = core_ticks_to_us(adap,
		TIMERVALUE2_GET(timer_value_2_and_3));
	s->timer_val[3] = core_ticks_to_us(adap,
		TIMERVALUE3_GET(timer_value_2_and_3));
	s->timer_val[4] = core_ticks_to_us(adap,
		TIMERVALUE4_GET(timer_value_4_and_5));
	s->timer_val[5] = core_ticks_to_us(adap,
		TIMERVALUE5_GET(timer_value_4_and_5));

	ingress_rx_threshold = t4_read_reg(adap, SGE_INGRESS_RX_THRESHOLD);
	s->counter_val[0] = THRESHOLD_0_GET(ingress_rx_threshold);
	s->counter_val[1] = THRESHOLD_1_GET(ingress_rx_threshold);
	s->counter_val[2] = THRESHOLD_2_GET(ingress_rx_threshold);
	s->counter_val[3] = THRESHOLD_3_GET(ingress_rx_threshold);

	return 0;
}

static int t4_sge_init_hard(struct adapter *adap)
{
	struct sge *s = &adap->sge;

	/*
                                                                    
                                           
  */
	t4_set_reg_field(adap, SGE_CONTROL, RXPKTCPLMODE_MASK,
			 RXPKTCPLMODE_MASK);

	/*
                                                                   
                                                                 
  */
	if (is_t4(adap->chip)) {
		t4_set_reg_field(adap, A_SGE_DBFIFO_STATUS,
				 V_HP_INT_THRESH(M_HP_INT_THRESH) |
				 V_LP_INT_THRESH(M_LP_INT_THRESH),
				 V_HP_INT_THRESH(dbfifo_int_thresh) |
				 V_LP_INT_THRESH(dbfifo_int_thresh));
	} else {
		t4_set_reg_field(adap, A_SGE_DBFIFO_STATUS,
				 V_LP_INT_THRESH_T5(M_LP_INT_THRESH_T5),
				 V_LP_INT_THRESH_T5(dbfifo_int_thresh));
		t4_set_reg_field(adap, SGE_DBFIFO_STATUS2,
				 V_HP_INT_THRESH_T5(M_HP_INT_THRESH_T5),
				 V_HP_INT_THRESH_T5(dbfifo_int_thresh));
	}
	t4_set_reg_field(adap, A_SGE_DOORBELL_CONTROL, F_ENABLE_DROP,
			F_ENABLE_DROP);

	/*
                                                      
                           
  */
	s->fl_pg_order = FL_PG_ORDER;
	if (s->fl_pg_order)
		t4_write_reg(adap,
			     SGE_FL_BUFFER_SIZE0+RX_LARGE_PG_BUF*sizeof(u32),
			     PAGE_SIZE << FL_PG_ORDER);
	t4_write_reg(adap, SGE_FL_BUFFER_SIZE0+RX_SMALL_MTU_BUF*sizeof(u32),
		     FL_MTU_SMALL_BUFSIZE(adap));
	t4_write_reg(adap, SGE_FL_BUFFER_SIZE0+RX_LARGE_MTU_BUF*sizeof(u32),
		     FL_MTU_LARGE_BUFSIZE(adap));

	/*
                                                                  
                                                        
  */
	t4_write_reg(adap, SGE_INGRESS_RX_THRESHOLD,
		     THRESHOLD_0(s->counter_val[0]) |
		     THRESHOLD_1(s->counter_val[1]) |
		     THRESHOLD_2(s->counter_val[2]) |
		     THRESHOLD_3(s->counter_val[3]));
	t4_write_reg(adap, SGE_TIMER_VALUE_0_AND_1,
		     TIMERVALUE0(us_to_core_ticks(adap, s->timer_val[0])) |
		     TIMERVALUE1(us_to_core_ticks(adap, s->timer_val[1])));
	t4_write_reg(adap, SGE_TIMER_VALUE_2_AND_3,
		     TIMERVALUE2(us_to_core_ticks(adap, s->timer_val[2])) |
		     TIMERVALUE3(us_to_core_ticks(adap, s->timer_val[3])));
	t4_write_reg(adap, SGE_TIMER_VALUE_4_AND_5,
		     TIMERVALUE4(us_to_core_ticks(adap, s->timer_val[4])) |
		     TIMERVALUE5(us_to_core_ticks(adap, s->timer_val[5])));

	return 0;
}

int t4_sge_init(struct adapter *adap)
{
	struct sge *s = &adap->sge;
	u32 sge_control;
	int ret;

	/*
                                                                      
                           
  */
	sge_control = t4_read_reg(adap, SGE_CONTROL);
	s->pktshift = PKTSHIFT_GET(sge_control);
	s->stat_len = (sge_control & EGRSTATUSPAGESIZE_MASK) ? 128 : 64;
	s->fl_align = 1 << (INGPADBOUNDARY_GET(sge_control) +
			    X_INGPADBOUNDARY_SHIFT);

	if (adap->flags & USING_SOFT_PARAMS)
		ret = t4_sge_init_soft(adap);
	else
		ret = t4_sge_init_hard(adap);
	if (ret < 0)
		return ret;

	/*
                                                                   
                                                                      
                                                                    
                                                                    
                                                                
                                                              
  */
	s->fl_starve_thres
		= EGRTHRESHOLD_GET(t4_read_reg(adap, SGE_CONM_CTRL))*2 + 1;

	setup_timer(&s->rx_timer, sge_rx_timer_cb, (unsigned long)adap);
	setup_timer(&s->tx_timer, sge_tx_timer_cb, (unsigned long)adap);
	s->starve_thres = core_ticks_per_usec(adap) * 1000000;  /*     */
	s->idma_state[0] = s->idma_state[1] = 0;
	spin_lock_init(&s->intrq_lock);

	return 0;
}
