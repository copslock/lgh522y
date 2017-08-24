/*
 * Copyright 2008 Cisco Systems, Inc.  All rights reserved.
 * Copyright 2007 Nuova Systems, Inc.  All rights reserved.
 *
 * This program is free software; you may redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
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
#include <linux/errno.h>
#include <linux/pci.h>
#include <linux/slab.h>
#include <linux/skbuff.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/if_ether.h>
#include <linux/if_vlan.h>
#include <linux/workqueue.h>
#include <scsi/fc/fc_fip.h>
#include <scsi/fc/fc_els.h>
#include <scsi/fc/fc_fcoe.h>
#include <scsi/fc_frame.h>
#include <scsi/libfc.h>
#include "fnic_io.h"
#include "fnic.h"
#include "fnic_fip.h"
#include "cq_enet_desc.h"
#include "cq_exch_desc.h"

static u8 fcoe_all_fcfs[ETH_ALEN];
struct workqueue_struct *fnic_fip_queue;
struct workqueue_struct *fnic_event_queue;

static void fnic_set_eth_mode(struct fnic *);
static void fnic_fcoe_send_vlan_req(struct fnic *fnic);
static void fnic_fcoe_start_fcf_disc(struct fnic *fnic);
static void fnic_fcoe_process_vlan_resp(struct fnic *fnic, struct sk_buff *);
static int fnic_fcoe_vlan_check(struct fnic *fnic, u16 flag);
static int fnic_fcoe_handle_fip_frame(struct fnic *fnic, struct sk_buff *skb);

void fnic_handle_link(struct work_struct *work)
{
	struct fnic *fnic = container_of(work, struct fnic, link_work);
	unsigned long flags;
	int old_link_status;
	u32 old_link_down_cnt;

	spin_lock_irqsave(&fnic->fnic_lock, flags);

	if (fnic->stop_rx_link_events) {
		spin_unlock_irqrestore(&fnic->fnic_lock, flags);
		return;
	}

	old_link_down_cnt = fnic->link_down_cnt;
	old_link_status = fnic->link_status;
	fnic->link_status = vnic_dev_link_status(fnic->vdev);
	fnic->link_down_cnt = vnic_dev_link_down_cnt(fnic->vdev);

	if (old_link_status == fnic->link_status) {
		if (!fnic->link_status)
			/*              */
			spin_unlock_irqrestore(&fnic->fnic_lock, flags);
		else {
			if (old_link_down_cnt != fnic->link_down_cnt) {
				/*                  */
				fnic->lport->host_stats.link_failure_count++;
				spin_unlock_irqrestore(&fnic->fnic_lock, flags);
				FNIC_FCS_DBG(KERN_DEBUG, fnic->lport->host,
					     "link down\n");
				fcoe_ctlr_link_down(&fnic->ctlr);
				if (fnic->config.flags & VFCF_FIP_CAPABLE) {
					/*                           */
					fnic_fcoe_send_vlan_req(fnic);
					return;
				}
				FNIC_FCS_DBG(KERN_DEBUG, fnic->lport->host,
					     "link up\n");
				fcoe_ctlr_link_up(&fnic->ctlr);
			} else
				/*          */
				spin_unlock_irqrestore(&fnic->fnic_lock, flags);
		}
	} else if (fnic->link_status) {
		/*            */
		spin_unlock_irqrestore(&fnic->fnic_lock, flags);
		if (fnic->config.flags & VFCF_FIP_CAPABLE) {
			/*                           */
			fnic_fcoe_send_vlan_req(fnic);
			return;
		}
		FNIC_FCS_DBG(KERN_DEBUG, fnic->lport->host, "link up\n");
		fcoe_ctlr_link_up(&fnic->ctlr);
	} else {
		/*            */
		fnic->lport->host_stats.link_failure_count++;
		spin_unlock_irqrestore(&fnic->fnic_lock, flags);
		FNIC_FCS_DBG(KERN_DEBUG, fnic->lport->host, "link down\n");
		fcoe_ctlr_link_down(&fnic->ctlr);
	}

}

/*
                                                       
 */
void fnic_handle_frame(struct work_struct *work)
{
	struct fnic *fnic = container_of(work, struct fnic, frame_work);
	struct fc_lport *lp = fnic->lport;
	unsigned long flags;
	struct sk_buff *skb;
	struct fc_frame *fp;

	while ((skb = skb_dequeue(&fnic->frame_queue))) {

		spin_lock_irqsave(&fnic->fnic_lock, flags);
		if (fnic->stop_rx_link_events) {
			spin_unlock_irqrestore(&fnic->fnic_lock, flags);
			dev_kfree_skb(skb);
			return;
		}
		fp = (struct fc_frame *)skb;

		/*
                                                                
                                                              
   */
		if (fnic->state != FNIC_IN_FC_MODE &&
		    fnic->state != FNIC_IN_ETH_MODE) {
			skb_queue_head(&fnic->frame_queue, skb);
			spin_unlock_irqrestore(&fnic->fnic_lock, flags);
			return;
		}
		spin_unlock_irqrestore(&fnic->fnic_lock, flags);

		fc_exch_recv(lp, fp);
	}
}

void fnic_fcoe_evlist_free(struct fnic *fnic)
{
	struct fnic_event *fevt = NULL;
	struct fnic_event *next = NULL;
	unsigned long flags;

	spin_lock_irqsave(&fnic->fnic_lock, flags);
	if (list_empty(&fnic->evlist)) {
		spin_unlock_irqrestore(&fnic->fnic_lock, flags);
		return;
	}

	list_for_each_entry_safe(fevt, next, &fnic->evlist, list) {
		list_del(&fevt->list);
		kfree(fevt);
	}
	spin_unlock_irqrestore(&fnic->fnic_lock, flags);
}

void fnic_handle_event(struct work_struct *work)
{
	struct fnic *fnic = container_of(work, struct fnic, event_work);
	struct fnic_event *fevt = NULL;
	struct fnic_event *next = NULL;
	unsigned long flags;

	spin_lock_irqsave(&fnic->fnic_lock, flags);
	if (list_empty(&fnic->evlist)) {
		spin_unlock_irqrestore(&fnic->fnic_lock, flags);
		return;
	}

	list_for_each_entry_safe(fevt, next, &fnic->evlist, list) {
		if (fnic->stop_rx_link_events) {
			list_del(&fevt->list);
			kfree(fevt);
			spin_unlock_irqrestore(&fnic->fnic_lock, flags);
			return;
		}
		/*
                                                                
                                                              
   */
		if (fnic->state != FNIC_IN_FC_MODE &&
		    fnic->state != FNIC_IN_ETH_MODE) {
			spin_unlock_irqrestore(&fnic->fnic_lock, flags);
			return;
		}

		list_del(&fevt->list);
		switch (fevt->event) {
		case FNIC_EVT_START_VLAN_DISC:
			spin_unlock_irqrestore(&fnic->fnic_lock, flags);
			fnic_fcoe_send_vlan_req(fnic);
			spin_lock_irqsave(&fnic->fnic_lock, flags);
			break;
		case FNIC_EVT_START_FCF_DISC:
			FNIC_FCS_DBG(KERN_DEBUG, fnic->lport->host,
				  "Start FCF Discovery\n");
			fnic_fcoe_start_fcf_disc(fnic);
			break;
		default:
			FNIC_FCS_DBG(KERN_DEBUG, fnic->lport->host,
				  "Unknown event 0x%x\n", fevt->event);
			break;
		}
		kfree(fevt);
	}
	spin_unlock_irqrestore(&fnic->fnic_lock, flags);
}

/* 
                                                    
                                                    
                               
  
                                                                      
                                         
 */
static inline int is_fnic_fip_flogi_reject(struct fcoe_ctlr *fip,
					 struct sk_buff *skb)
{
	struct fc_lport *lport = fip->lp;
	struct fip_header *fiph;
	struct fc_frame_header *fh = NULL;
	struct fip_desc *desc;
	struct fip_encaps *els;
	enum fip_desc_type els_dtype = 0;
	u16 op;
	u8 els_op;
	u8 sub;

	size_t els_len = 0;
	size_t rlen;
	size_t dlen = 0;

	if (skb_linearize(skb))
		return 0;

	if (skb->len < sizeof(*fiph))
		return 0;

	fiph = (struct fip_header *)skb->data;
	op = ntohs(fiph->fip_op);
	sub = fiph->fip_subcode;

	if (op != FIP_OP_LS)
		return 0;

	if (sub != FIP_SC_REP)
		return 0;

	rlen = ntohs(fiph->fip_dl_len) * 4;
	if (rlen + sizeof(*fiph) > skb->len)
		return 0;

	desc = (struct fip_desc *)(fiph + 1);
	dlen = desc->fip_dlen * FIP_BPW;

	if (desc->fip_dtype == FIP_DT_FLOGI) {

		shost_printk(KERN_DEBUG, lport->host,
			  " FIP TYPE FLOGI: fab name:%llx "
			  "vfid:%d map:%x\n",
			  fip->sel_fcf->fabric_name, fip->sel_fcf->vfid,
			  fip->sel_fcf->fc_map);
		if (dlen < sizeof(*els) + sizeof(*fh) + 1)
			return 0;

		els_len = dlen - sizeof(*els);
		els = (struct fip_encaps *)desc;
		fh = (struct fc_frame_header *)(els + 1);
		els_dtype = desc->fip_dtype;

		if (!fh)
			return 0;

		/*
                                                                 
                                                  
   */
		els_op = *(u8 *)(fh + 1);
		if (els_op == ELS_LS_RJT) {
			shost_printk(KERN_INFO, lport->host,
				  "Flogi Request Rejected by Switch\n");
			return 1;
		}
		shost_printk(KERN_INFO, lport->host,
				"Flogi Request Accepted by Switch\n");
	}
	return 0;
}

static void fnic_fcoe_send_vlan_req(struct fnic *fnic)
{
	struct fcoe_ctlr *fip = &fnic->ctlr;
	struct sk_buff *skb;
	char *eth_fr;
	int fr_len;
	struct fip_vlan *vlan;
	u64 vlan_tov;

	fnic_fcoe_reset_vlans(fnic);
	fnic->set_vlan(fnic, 0);
	FNIC_FCS_DBG(KERN_INFO, fnic->lport->host,
		  "Sending VLAN request...\n");
	skb = dev_alloc_skb(sizeof(struct fip_vlan));
	if (!skb)
		return;

	fr_len = sizeof(*vlan);
	eth_fr = (char *)skb->data;
	vlan = (struct fip_vlan *)eth_fr;

	memset(vlan, 0, sizeof(*vlan));
	memcpy(vlan->eth.h_source, fip->ctl_src_addr, ETH_ALEN);
	memcpy(vlan->eth.h_dest, fcoe_all_fcfs, ETH_ALEN);
	vlan->eth.h_proto = htons(ETH_P_FIP);

	vlan->fip.fip_ver = FIP_VER_ENCAPS(FIP_VER);
	vlan->fip.fip_op = htons(FIP_OP_VLAN);
	vlan->fip.fip_subcode = FIP_SC_VL_REQ;
	vlan->fip.fip_dl_len = htons(sizeof(vlan->desc) / FIP_BPW);

	vlan->desc.mac.fd_desc.fip_dtype = FIP_DT_MAC;
	vlan->desc.mac.fd_desc.fip_dlen = sizeof(vlan->desc.mac) / FIP_BPW;
	memcpy(&vlan->desc.mac.fd_mac, fip->ctl_src_addr, ETH_ALEN);

	vlan->desc.wwnn.fd_desc.fip_dtype = FIP_DT_NAME;
	vlan->desc.wwnn.fd_desc.fip_dlen = sizeof(vlan->desc.wwnn) / FIP_BPW;
	put_unaligned_be64(fip->lp->wwnn, &vlan->desc.wwnn.fd_wwn);

	skb_put(skb, sizeof(*vlan));
	skb->protocol = htons(ETH_P_FIP);
	skb_reset_mac_header(skb);
	skb_reset_network_header(skb);
	fip->send(fip, skb);

	/*                                                       */
	vlan_tov = jiffies + msecs_to_jiffies(FCOE_CTLR_FIPVLAN_TOV);
	mod_timer(&fnic->fip_timer, round_jiffies(vlan_tov));
}

static void fnic_fcoe_process_vlan_resp(struct fnic *fnic, struct sk_buff *skb)
{
	struct fcoe_ctlr *fip = &fnic->ctlr;
	struct fip_header *fiph;
	struct fip_desc *desc;
	u16 vid;
	size_t rlen;
	size_t dlen;
	struct fcoe_vlan *vlan;
	u64 sol_time;
	unsigned long flags;

	FNIC_FCS_DBG(KERN_INFO, fnic->lport->host,
		  "Received VLAN response...\n");

	fiph = (struct fip_header *) skb->data;

	FNIC_FCS_DBG(KERN_INFO, fnic->lport->host,
		  "Received VLAN response... OP 0x%x SUB_OP 0x%x\n",
		  ntohs(fiph->fip_op), fiph->fip_subcode);

	rlen = ntohs(fiph->fip_dl_len) * 4;
	fnic_fcoe_reset_vlans(fnic);
	spin_lock_irqsave(&fnic->vlans_lock, flags);
	desc = (struct fip_desc *)(fiph + 1);
	while (rlen > 0) {
		dlen = desc->fip_dlen * FIP_BPW;
		switch (desc->fip_dtype) {
		case FIP_DT_VLAN:
			vid = ntohs(((struct fip_vlan_desc *)desc)->fd_vlan);
			shost_printk(KERN_INFO, fnic->lport->host,
				  "process_vlan_resp: FIP VLAN %d\n", vid);
			vlan = kmalloc(sizeof(*vlan),
							GFP_ATOMIC);
			if (!vlan) {
				/*                  */
				spin_unlock_irqrestore(&fnic->vlans_lock,
							flags);
				goto out;
			}
			memset(vlan, 0, sizeof(struct fcoe_vlan));
			vlan->vid = vid & 0x0fff;
			vlan->state = FIP_VLAN_AVAIL;
			list_add_tail(&vlan->list, &fnic->vlans);
			break;
		}
		desc = (struct fip_desc *)((char *)desc + dlen);
		rlen -= dlen;
	}

	/*                                */
	if (list_empty(&fnic->vlans)) {
		/*                  */
		FNIC_FCS_DBG(KERN_INFO, fnic->lport->host,
			  "No VLAN descriptors in FIP VLAN response\n");
		spin_unlock_irqrestore(&fnic->vlans_lock, flags);
		goto out;
	}

	vlan = list_first_entry(&fnic->vlans, struct fcoe_vlan, list);
	fnic->set_vlan(fnic, vlan->vid);
	vlan->state = FIP_VLAN_SENT; /*          */
	vlan->sol_count++;
	spin_unlock_irqrestore(&fnic->vlans_lock, flags);

	/*                        */
	fcoe_ctlr_link_up(fip);

	sol_time = jiffies + msecs_to_jiffies(FCOE_CTLR_START_DELAY);
	mod_timer(&fnic->fip_timer, round_jiffies(sol_time));
out:
	return;
}

static void fnic_fcoe_start_fcf_disc(struct fnic *fnic)
{
	unsigned long flags;
	struct fcoe_vlan *vlan;
	u64 sol_time;

	spin_lock_irqsave(&fnic->vlans_lock, flags);
	vlan = list_first_entry(&fnic->vlans, struct fcoe_vlan, list);
	fnic->set_vlan(fnic, vlan->vid);
	vlan->state = FIP_VLAN_SENT; /*          */
	vlan->sol_count = 1;
	spin_unlock_irqrestore(&fnic->vlans_lock, flags);

	/*                        */
	fcoe_ctlr_link_up(&fnic->ctlr);

	sol_time = jiffies + msecs_to_jiffies(FCOE_CTLR_START_DELAY);
	mod_timer(&fnic->fip_timer, round_jiffies(sol_time));
}

static int fnic_fcoe_vlan_check(struct fnic *fnic, u16 flag)
{
	unsigned long flags;
	struct fcoe_vlan *fvlan;

	spin_lock_irqsave(&fnic->vlans_lock, flags);
	if (list_empty(&fnic->vlans)) {
		spin_unlock_irqrestore(&fnic->vlans_lock, flags);
		return -EINVAL;
	}

	fvlan = list_first_entry(&fnic->vlans, struct fcoe_vlan, list);
	if (fvlan->state == FIP_VLAN_USED) {
		spin_unlock_irqrestore(&fnic->vlans_lock, flags);
		return 0;
	}

	if (fvlan->state == FIP_VLAN_SENT) {
		fvlan->state = FIP_VLAN_USED;
		spin_unlock_irqrestore(&fnic->vlans_lock, flags);
		return 0;
	}
	spin_unlock_irqrestore(&fnic->vlans_lock, flags);
	return -EINVAL;
}

static void fnic_event_enq(struct fnic *fnic, enum fnic_evt ev)
{
	struct fnic_event *fevt;
	unsigned long flags;

	fevt = kmalloc(sizeof(*fevt), GFP_ATOMIC);
	if (!fevt)
		return;

	fevt->fnic = fnic;
	fevt->event = ev;

	spin_lock_irqsave(&fnic->fnic_lock, flags);
	list_add_tail(&fevt->list, &fnic->evlist);
	spin_unlock_irqrestore(&fnic->fnic_lock, flags);

	schedule_work(&fnic->event_work);
}

static int fnic_fcoe_handle_fip_frame(struct fnic *fnic, struct sk_buff *skb)
{
	struct fip_header *fiph;
	int ret = 1;
	u16 op;
	u8 sub;

	if (!skb || !(skb->data))
		return -1;

	if (skb_linearize(skb))
		goto drop;

	fiph = (struct fip_header *)skb->data;
	op = ntohs(fiph->fip_op);
	sub = fiph->fip_subcode;

	if (FIP_VER_DECAPS(fiph->fip_ver) != FIP_VER)
		goto drop;

	if (ntohs(fiph->fip_dl_len) * FIP_BPW + sizeof(*fiph) > skb->len)
		goto drop;

	if (op == FIP_OP_DISC && sub == FIP_SC_ADV) {
		if (fnic_fcoe_vlan_check(fnic, ntohs(fiph->fip_flags)))
			goto drop;
		/*                    */
		ret = 1;
	} else if (op == FIP_OP_VLAN && sub == FIP_SC_VL_REP) {
		/*                      */
		fnic_fcoe_process_vlan_resp(fnic, skb);
		ret = 0;
	} else if (op == FIP_OP_CTRL && sub == FIP_SC_CLR_VLINK) {
		/*                                         */
		fnic_event_enq(fnic, FNIC_EVT_START_VLAN_DISC);
		/*                    */
		ret = 1;
	}
drop:
	return ret;
}

void fnic_handle_fip_frame(struct work_struct *work)
{
	struct fnic *fnic = container_of(work, struct fnic, fip_frame_work);
	unsigned long flags;
	struct sk_buff *skb;
	struct ethhdr *eh;

	while ((skb = skb_dequeue(&fnic->fip_frame_queue))) {
		spin_lock_irqsave(&fnic->fnic_lock, flags);
		if (fnic->stop_rx_link_events) {
			spin_unlock_irqrestore(&fnic->fnic_lock, flags);
			dev_kfree_skb(skb);
			return;
		}
		/*
                                                                
                                                              
   */
		if (fnic->state != FNIC_IN_FC_MODE &&
		    fnic->state != FNIC_IN_ETH_MODE) {
			skb_queue_head(&fnic->fip_frame_queue, skb);
			spin_unlock_irqrestore(&fnic->fnic_lock, flags);
			return;
		}
		spin_unlock_irqrestore(&fnic->fnic_lock, flags);
		eh = (struct ethhdr *)skb->data;
		if (eh->h_proto == htons(ETH_P_FIP)) {
			skb_pull(skb, sizeof(*eh));
			if (fnic_fcoe_handle_fip_frame(fnic, skb) <= 0) {
				dev_kfree_skb(skb);
				continue;
			}
			/*
                                          
                                  
    */
			if (is_fnic_fip_flogi_reject(&fnic->ctlr, skb)) {
				shost_printk(KERN_INFO, fnic->lport->host,
					  "Trigger a Link down - VLAN Disc\n");
				fcoe_ctlr_link_down(&fnic->ctlr);
				/*                           */
				fnic_fcoe_send_vlan_req(fnic);
				dev_kfree_skb(skb);
				continue;
			}
			fcoe_ctlr_recv(&fnic->ctlr, skb);
			continue;
		}
	}
}

/* 
                                                                
                        
                        
 */
static inline int fnic_import_rq_eth_pkt(struct fnic *fnic, struct sk_buff *skb)
{
	struct fc_frame *fp;
	struct ethhdr *eh;
	struct fcoe_hdr *fcoe_hdr;
	struct fcoe_crc_eof *ft;

	/*
                                       
  */
	eh = (struct ethhdr *)skb->data;
	if (eh->h_proto == htons(ETH_P_8021Q)) {
		memmove((u8 *)eh + VLAN_HLEN, eh, ETH_ALEN * 2);
		eh = (struct ethhdr *)skb_pull(skb, VLAN_HLEN);
		skb_reset_mac_header(skb);
	}
	if (eh->h_proto == htons(ETH_P_FIP)) {
		if (!(fnic->config.flags & VFCF_FIP_CAPABLE)) {
			printk(KERN_ERR "Dropped FIP frame, as firmware "
					"uses non-FIP mode, Enable FIP "
					"using UCSM\n");
			goto drop;
		}
		skb_queue_tail(&fnic->fip_frame_queue, skb);
		queue_work(fnic_fip_queue, &fnic->fip_frame_work);
		return 1;		/*                                 */
	}
	if (eh->h_proto != htons(ETH_P_FCOE))
		goto drop;
	skb_set_network_header(skb, sizeof(*eh));
	skb_pull(skb, sizeof(*eh));

	fcoe_hdr = (struct fcoe_hdr *)skb->data;
	if (FC_FCOE_DECAPS_VER(fcoe_hdr) != FC_FCOE_VER)
		goto drop;

	fp = (struct fc_frame *)skb;
	fc_frame_init(fp);
	fr_sof(fp) = fcoe_hdr->fcoe_sof;
	skb_pull(skb, sizeof(struct fcoe_hdr));
	skb_reset_transport_header(skb);

	ft = (struct fcoe_crc_eof *)(skb->data + skb->len - sizeof(*ft));
	fr_eof(fp) = ft->fcoe_eof;
	skb_trim(skb, skb->len - sizeof(*ft));
	return 0;
drop:
	dev_kfree_skb_irq(skb);
	return -1;
}

/* 
                                                               
                        
                                         
  
                                  
 */
void fnic_update_mac_locked(struct fnic *fnic, u8 *new)
{
	u8 *ctl = fnic->ctlr.ctl_src_addr;
	u8 *data = fnic->data_src_addr;

	if (is_zero_ether_addr(new))
		new = ctl;
	if (!compare_ether_addr(data, new))
		return;
	FNIC_FCS_DBG(KERN_DEBUG, fnic->lport->host, "update_mac %pM\n", new);
	if (!is_zero_ether_addr(data) && compare_ether_addr(data, ctl))
		vnic_dev_del_addr(fnic->vdev, data);
	memcpy(data, new, ETH_ALEN);
	if (compare_ether_addr(new, ctl))
		vnic_dev_add_addr(fnic->vdev, new);
}

/* 
                                                        
                      
                                         
 */
void fnic_update_mac(struct fc_lport *lport, u8 *new)
{
	struct fnic *fnic = lport_priv(lport);

	spin_lock_irq(&fnic->fnic_lock);
	fnic_update_mac_locked(fnic, new);
	spin_unlock_irq(&fnic->fnic_lock);
}

/* 
                                                               
                      
                            
                                                            
  
                                                                
                                                                        
                     
  
                                                        
  
                                                         
 */
void fnic_set_port_id(struct fc_lport *lport, u32 port_id, struct fc_frame *fp)
{
	struct fnic *fnic = lport_priv(lport);
	u8 *mac;
	int ret;

	FNIC_FCS_DBG(KERN_DEBUG, lport->host, "set port_id %x fp %p\n",
		     port_id, fp);

	/*
                                                                
                                    
  */
	if (!port_id) {
		fnic_update_mac(lport, fnic->ctlr.ctl_src_addr);
		fnic_set_eth_mode(fnic);
		return;
	}

	if (fp) {
		mac = fr_cb(fp)->granted_mac;
		if (is_zero_ether_addr(mac)) {
			/*                                                  */
			fcoe_ctlr_recv_flogi(&fnic->ctlr, lport, fp);
		}
		fnic_update_mac(lport, mac);
	}

	/*                                               */
	spin_lock_irq(&fnic->fnic_lock);
	if (fnic->state == FNIC_IN_ETH_MODE || fnic->state == FNIC_IN_FC_MODE)
		fnic->state = FNIC_IN_ETH_TRANS_FC_MODE;
	else {
		FNIC_FCS_DBG(KERN_DEBUG, fnic->lport->host,
			     "Unexpected fnic state %s while"
			     " processing flogi resp\n",
			     fnic_state_to_str(fnic->state));
		spin_unlock_irq(&fnic->fnic_lock);
		return;
	}
	spin_unlock_irq(&fnic->fnic_lock);

	/*
                                                          
                                                               
  */
	ret = fnic_flogi_reg_handler(fnic, port_id);

	if (ret < 0) {
		spin_lock_irq(&fnic->fnic_lock);
		if (fnic->state == FNIC_IN_ETH_TRANS_FC_MODE)
			fnic->state = FNIC_IN_ETH_MODE;
		spin_unlock_irq(&fnic->fnic_lock);
	}
}

static void fnic_rq_cmpl_frame_recv(struct vnic_rq *rq, struct cq_desc
				    *cq_desc, struct vnic_rq_buf *buf,
				    int skipped __attribute__((unused)),
				    void *opaque)
{
	struct fnic *fnic = vnic_dev_priv(rq->vdev);
	struct sk_buff *skb;
	struct fc_frame *fp;
	unsigned int eth_hdrs_stripped;
	u8 type, color, eop, sop, ingress_port, vlan_stripped;
	u8 fcoe = 0, fcoe_sof, fcoe_eof;
	u8 fcoe_fc_crc_ok = 1, fcoe_enc_error = 0;
	u8 tcp_udp_csum_ok, udp, tcp, ipv4_csum_ok;
	u8 ipv6, ipv4, ipv4_fragment, rss_type, csum_not_calc;
	u8 fcs_ok = 1, packet_error = 0;
	u16 q_number, completed_index, bytes_written = 0, vlan, checksum;
	u32 rss_hash;
	u16 exchange_id, tmpl;
	u8 sof = 0;
	u8 eof = 0;
	u32 fcp_bytes_written = 0;
	unsigned long flags;

	pci_unmap_single(fnic->pdev, buf->dma_addr, buf->len,
			 PCI_DMA_FROMDEVICE);
	skb = buf->os_buf;
	fp = (struct fc_frame *)skb;
	buf->os_buf = NULL;

	cq_desc_dec(cq_desc, &type, &color, &q_number, &completed_index);
	if (type == CQ_DESC_TYPE_RQ_FCP) {
		cq_fcp_rq_desc_dec((struct cq_fcp_rq_desc *)cq_desc,
				   &type, &color, &q_number, &completed_index,
				   &eop, &sop, &fcoe_fc_crc_ok, &exchange_id,
				   &tmpl, &fcp_bytes_written, &sof, &eof,
				   &ingress_port, &packet_error,
				   &fcoe_enc_error, &fcs_ok, &vlan_stripped,
				   &vlan);
		eth_hdrs_stripped = 1;
		skb_trim(skb, fcp_bytes_written);
		fr_sof(fp) = sof;
		fr_eof(fp) = eof;

	} else if (type == CQ_DESC_TYPE_RQ_ENET) {
		cq_enet_rq_desc_dec((struct cq_enet_rq_desc *)cq_desc,
				    &type, &color, &q_number, &completed_index,
				    &ingress_port, &fcoe, &eop, &sop,
				    &rss_type, &csum_not_calc, &rss_hash,
				    &bytes_written, &packet_error,
				    &vlan_stripped, &vlan, &checksum,
				    &fcoe_sof, &fcoe_fc_crc_ok,
				    &fcoe_enc_error, &fcoe_eof,
				    &tcp_udp_csum_ok, &udp, &tcp,
				    &ipv4_csum_ok, &ipv6, &ipv4,
				    &ipv4_fragment, &fcs_ok);
		eth_hdrs_stripped = 0;
		skb_trim(skb, bytes_written);
		if (!fcs_ok) {
			FNIC_FCS_DBG(KERN_DEBUG, fnic->lport->host,
				     "fcs error.  dropping packet.\n");
			goto drop;
		}
		if (fnic_import_rq_eth_pkt(fnic, skb))
			return;

	} else {
		/*              */
		shost_printk(KERN_ERR, fnic->lport->host,
			     "fnic rq_cmpl wrong cq type x%x\n", type);
		goto drop;
	}

	if (!fcs_ok || packet_error || !fcoe_fc_crc_ok || fcoe_enc_error) {
		FNIC_FCS_DBG(KERN_DEBUG, fnic->lport->host,
			     "fnic rq_cmpl fcoe x%x fcsok x%x"
			     " pkterr x%x fcoe_fc_crc_ok x%x, fcoe_enc_err"
			     " x%x\n",
			     fcoe, fcs_ok, packet_error,
			     fcoe_fc_crc_ok, fcoe_enc_error);
		goto drop;
	}

	spin_lock_irqsave(&fnic->fnic_lock, flags);
	if (fnic->stop_rx_link_events) {
		spin_unlock_irqrestore(&fnic->fnic_lock, flags);
		goto drop;
	}
	fr_dev(fp) = fnic->lport;
	spin_unlock_irqrestore(&fnic->fnic_lock, flags);

	skb_queue_tail(&fnic->frame_queue, skb);
	queue_work(fnic_event_queue, &fnic->frame_work);

	return;
drop:
	dev_kfree_skb_irq(skb);
}

static int fnic_rq_cmpl_handler_cont(struct vnic_dev *vdev,
				     struct cq_desc *cq_desc, u8 type,
				     u16 q_number, u16 completed_index,
				     void *opaque)
{
	struct fnic *fnic = vnic_dev_priv(vdev);

	vnic_rq_service(&fnic->rq[q_number], cq_desc, completed_index,
			VNIC_RQ_RETURN_DESC, fnic_rq_cmpl_frame_recv,
			NULL);
	return 0;
}

int fnic_rq_cmpl_handler(struct fnic *fnic, int rq_work_to_do)
{
	unsigned int tot_rq_work_done = 0, cur_work_done;
	unsigned int i;
	int err;

	for (i = 0; i < fnic->rq_count; i++) {
		cur_work_done = vnic_cq_service(&fnic->cq[i], rq_work_to_do,
						fnic_rq_cmpl_handler_cont,
						NULL);
		if (cur_work_done) {
			err = vnic_rq_fill(&fnic->rq[i], fnic_alloc_rq_frame);
			if (err)
				shost_printk(KERN_ERR, fnic->lport->host,
					     "fnic_alloc_rq_frame can't alloc"
					     " frame\n");
		}
		tot_rq_work_done += cur_work_done;
	}

	return tot_rq_work_done;
}

/*
                                                                    
                                                                        
                                                       
 */
int fnic_alloc_rq_frame(struct vnic_rq *rq)
{
	struct fnic *fnic = vnic_dev_priv(rq->vdev);
	struct sk_buff *skb;
	u16 len;
	dma_addr_t pa;

	len = FC_FRAME_HEADROOM + FC_MAX_FRAME + FC_FRAME_TAILROOM;
	skb = dev_alloc_skb(len);
	if (!skb) {
		FNIC_FCS_DBG(KERN_DEBUG, fnic->lport->host,
			     "Unable to allocate RQ sk_buff\n");
		return -ENOMEM;
	}
	skb_reset_mac_header(skb);
	skb_reset_transport_header(skb);
	skb_reset_network_header(skb);
	skb_put(skb, len);
	pa = pci_map_single(fnic->pdev, skb->data, len, PCI_DMA_FROMDEVICE);
	fnic_queue_rq_desc(rq, skb, pa, len);
	return 0;
}

void fnic_free_rq_buf(struct vnic_rq *rq, struct vnic_rq_buf *buf)
{
	struct fc_frame *fp = buf->os_buf;
	struct fnic *fnic = vnic_dev_priv(rq->vdev);

	pci_unmap_single(fnic->pdev, buf->dma_addr, buf->len,
			 PCI_DMA_FROMDEVICE);

	dev_kfree_skb(fp_skb(fp));
	buf->os_buf = NULL;
}

/* 
                                         
                            
                                                         
 */
void fnic_eth_send(struct fcoe_ctlr *fip, struct sk_buff *skb)
{
	struct fnic *fnic = fnic_from_ctlr(fip);
	struct vnic_wq *wq = &fnic->wq[0];
	dma_addr_t pa;
	struct ethhdr *eth_hdr;
	struct vlan_ethhdr *vlan_hdr;
	unsigned long flags;

	if (!fnic->vlan_hw_insert) {
		eth_hdr = (struct ethhdr *)skb_mac_header(skb);
		vlan_hdr = (struct vlan_ethhdr *)skb_push(skb,
				sizeof(*vlan_hdr) - sizeof(*eth_hdr));
		memcpy(vlan_hdr, eth_hdr, 2 * ETH_ALEN);
		vlan_hdr->h_vlan_proto = htons(ETH_P_8021Q);
		vlan_hdr->h_vlan_encapsulated_proto = eth_hdr->h_proto;
		vlan_hdr->h_vlan_TCI = htons(fnic->vlan_id);
	}

	pa = pci_map_single(fnic->pdev, skb->data, skb->len, PCI_DMA_TODEVICE);

	spin_lock_irqsave(&fnic->wq_lock[0], flags);
	if (!vnic_wq_desc_avail(wq)) {
		pci_unmap_single(fnic->pdev, pa, skb->len, PCI_DMA_TODEVICE);
		spin_unlock_irqrestore(&fnic->wq_lock[0], flags);
		kfree_skb(skb);
		return;
	}

	fnic_queue_wq_eth_desc(wq, skb, pa, skb->len,
			       0 /*                      */,
			       fnic->vlan_id, 1);
	spin_unlock_irqrestore(&fnic->wq_lock[0], flags);
}

/*
                 
 */
static int fnic_send_frame(struct fnic *fnic, struct fc_frame *fp)
{
	struct vnic_wq *wq = &fnic->wq[0];
	struct sk_buff *skb;
	dma_addr_t pa;
	struct ethhdr *eth_hdr;
	struct vlan_ethhdr *vlan_hdr;
	struct fcoe_hdr *fcoe_hdr;
	struct fc_frame_header *fh;
	u32 tot_len, eth_hdr_len;
	int ret = 0;
	unsigned long flags;

	fh = fc_frame_header_get(fp);
	skb = fp_skb(fp);

	if (unlikely(fh->fh_r_ctl == FC_RCTL_ELS_REQ) &&
	    fcoe_ctlr_els_send(&fnic->ctlr, fnic->lport, skb))
		return 0;

	if (!fnic->vlan_hw_insert) {
		eth_hdr_len = sizeof(*vlan_hdr) + sizeof(*fcoe_hdr);
		vlan_hdr = (struct vlan_ethhdr *)skb_push(skb, eth_hdr_len);
		eth_hdr = (struct ethhdr *)vlan_hdr;
		vlan_hdr->h_vlan_proto = htons(ETH_P_8021Q);
		vlan_hdr->h_vlan_encapsulated_proto = htons(ETH_P_FCOE);
		vlan_hdr->h_vlan_TCI = htons(fnic->vlan_id);
		fcoe_hdr = (struct fcoe_hdr *)(vlan_hdr + 1);
	} else {
		eth_hdr_len = sizeof(*eth_hdr) + sizeof(*fcoe_hdr);
		eth_hdr = (struct ethhdr *)skb_push(skb, eth_hdr_len);
		eth_hdr->h_proto = htons(ETH_P_FCOE);
		fcoe_hdr = (struct fcoe_hdr *)(eth_hdr + 1);
	}

	if (fnic->ctlr.map_dest)
		fc_fcoe_set_mac(eth_hdr->h_dest, fh->fh_d_id);
	else
		memcpy(eth_hdr->h_dest, fnic->ctlr.dest_addr, ETH_ALEN);
	memcpy(eth_hdr->h_source, fnic->data_src_addr, ETH_ALEN);

	tot_len = skb->len;
	BUG_ON(tot_len % 4);

	memset(fcoe_hdr, 0, sizeof(*fcoe_hdr));
	fcoe_hdr->fcoe_sof = fr_sof(fp);
	if (FC_FCOE_VER)
		FC_FCOE_ENCAPS_VER(fcoe_hdr, FC_FCOE_VER);

	pa = pci_map_single(fnic->pdev, eth_hdr, tot_len, PCI_DMA_TODEVICE);

	spin_lock_irqsave(&fnic->wq_lock[0], flags);

	if (!vnic_wq_desc_avail(wq)) {
		pci_unmap_single(fnic->pdev, pa,
				 tot_len, PCI_DMA_TODEVICE);
		ret = -1;
		goto fnic_send_frame_end;
	}

	fnic_queue_wq_desc(wq, skb, pa, tot_len, fr_eof(fp),
			   0 /*                      */,
			   fnic->vlan_id, 1, 1, 1);
fnic_send_frame_end:
	spin_unlock_irqrestore(&fnic->wq_lock[0], flags);

	if (ret)
		dev_kfree_skb_any(fp_skb(fp));

	return ret;
}

/*
            
                              
 */
int fnic_send(struct fc_lport *lp, struct fc_frame *fp)
{
	struct fnic *fnic = lport_priv(lp);
	unsigned long flags;

	if (fnic->in_remove) {
		dev_kfree_skb(fp_skb(fp));
		return -1;
	}

	/*
                                           
                                                                        
  */
	spin_lock_irqsave(&fnic->fnic_lock, flags);
	if (fnic->state != FNIC_IN_FC_MODE && fnic->state != FNIC_IN_ETH_MODE) {
		skb_queue_tail(&fnic->tx_queue, fp_skb(fp));
		spin_unlock_irqrestore(&fnic->fnic_lock, flags);
		return 0;
	}
	spin_unlock_irqrestore(&fnic->fnic_lock, flags);

	return fnic_send_frame(fnic, fp);
}

/* 
                                        
                     
  
                                                                  
                                                                         
                                                                      
  
                                 
 */
void fnic_flush_tx(struct fnic *fnic)
{
	struct sk_buff *skb;
	struct fc_frame *fp;

	while ((skb = skb_dequeue(&fnic->tx_queue))) {
		fp = (struct fc_frame *)skb;
		fnic_send_frame(fnic, fp);
	}
}

/* 
                                                     
                     
  
                                 
 */
static void fnic_set_eth_mode(struct fnic *fnic)
{
	unsigned long flags;
	enum fnic_state old_state;
	int ret;

	spin_lock_irqsave(&fnic->fnic_lock, flags);
again:
	old_state = fnic->state;
	switch (old_state) {
	case FNIC_IN_FC_MODE:
	case FNIC_IN_ETH_TRANS_FC_MODE:
	default:
		fnic->state = FNIC_IN_FC_TRANS_ETH_MODE;
		spin_unlock_irqrestore(&fnic->fnic_lock, flags);

		ret = fnic_fw_reset_handler(fnic);

		spin_lock_irqsave(&fnic->fnic_lock, flags);
		if (fnic->state != FNIC_IN_FC_TRANS_ETH_MODE)
			goto again;
		if (ret)
			fnic->state = old_state;
		break;

	case FNIC_IN_FC_TRANS_ETH_MODE:
	case FNIC_IN_ETH_MODE:
		break;
	}
	spin_unlock_irqrestore(&fnic->fnic_lock, flags);
}

static void fnic_wq_complete_frame_send(struct vnic_wq *wq,
					struct cq_desc *cq_desc,
					struct vnic_wq_buf *buf, void *opaque)
{
	struct sk_buff *skb = buf->os_buf;
	struct fc_frame *fp = (struct fc_frame *)skb;
	struct fnic *fnic = vnic_dev_priv(wq->vdev);

	pci_unmap_single(fnic->pdev, buf->dma_addr,
			 buf->len, PCI_DMA_TODEVICE);
	dev_kfree_skb_irq(fp_skb(fp));
	buf->os_buf = NULL;
}

static int fnic_wq_cmpl_handler_cont(struct vnic_dev *vdev,
				     struct cq_desc *cq_desc, u8 type,
				     u16 q_number, u16 completed_index,
				     void *opaque)
{
	struct fnic *fnic = vnic_dev_priv(vdev);
	unsigned long flags;

	spin_lock_irqsave(&fnic->wq_lock[q_number], flags);
	vnic_wq_service(&fnic->wq[q_number], cq_desc, completed_index,
			fnic_wq_complete_frame_send, NULL);
	spin_unlock_irqrestore(&fnic->wq_lock[q_number], flags);

	return 0;
}

int fnic_wq_cmpl_handler(struct fnic *fnic, int work_to_do)
{
	unsigned int wq_work_done = 0;
	unsigned int i;

	for (i = 0; i < fnic->raw_wq_count; i++) {
		wq_work_done  += vnic_cq_service(&fnic->cq[fnic->rq_count+i],
						 work_to_do,
						 fnic_wq_cmpl_handler_cont,
						 NULL);
	}

	return wq_work_done;
}


void fnic_free_wq_buf(struct vnic_wq *wq, struct vnic_wq_buf *buf)
{
	struct fc_frame *fp = buf->os_buf;
	struct fnic *fnic = vnic_dev_priv(wq->vdev);

	pci_unmap_single(fnic->pdev, buf->dma_addr,
			 buf->len, PCI_DMA_TODEVICE);

	dev_kfree_skb(fp_skb(fp));
	buf->os_buf = NULL;
}

void fnic_fcoe_reset_vlans(struct fnic *fnic)
{
	unsigned long flags;
	struct fcoe_vlan *vlan;
	struct fcoe_vlan *next;

	/*
                                                             
                                                               
                     
  */
	spin_lock_irqsave(&fnic->vlans_lock, flags);
	if (!list_empty(&fnic->vlans)) {
		list_for_each_entry_safe(vlan, next, &fnic->vlans, list) {
			list_del(&vlan->list);
			kfree(vlan);
		}
	}
	spin_unlock_irqrestore(&fnic->vlans_lock, flags);
}

void fnic_handle_fip_timer(struct fnic *fnic)
{
	unsigned long flags;
	struct fcoe_vlan *vlan;
	u64 sol_time;

	spin_lock_irqsave(&fnic->fnic_lock, flags);
	if (fnic->stop_rx_link_events) {
		spin_unlock_irqrestore(&fnic->fnic_lock, flags);
		return;
	}
	spin_unlock_irqrestore(&fnic->fnic_lock, flags);

	if (fnic->ctlr.mode == FIP_ST_NON_FIP)
		return;

	spin_lock_irqsave(&fnic->vlans_lock, flags);
	if (list_empty(&fnic->vlans)) {
		/*                               */
		FNIC_FCS_DBG(KERN_DEBUG, fnic->lport->host,
			  "Start VLAN Discovery\n");
		spin_unlock_irqrestore(&fnic->vlans_lock, flags);
		fnic_event_enq(fnic, FNIC_EVT_START_VLAN_DISC);
		return;
	}

	vlan = list_first_entry(&fnic->vlans, struct fcoe_vlan, list);
	shost_printk(KERN_DEBUG, fnic->lport->host,
		  "fip_timer: vlan %d state %d sol_count %d\n",
		  vlan->vid, vlan->state, vlan->sol_count);
	switch (vlan->state) {
	case FIP_VLAN_USED:
		FNIC_FCS_DBG(KERN_DEBUG, fnic->lport->host,
			  "FIP VLAN is selected for FC transaction\n");
		spin_unlock_irqrestore(&fnic->vlans_lock, flags);
		break;
	case FIP_VLAN_FAILED:
		/*                                                     */
		FNIC_FCS_DBG(KERN_DEBUG, fnic->lport->host,
			  "Start VLAN Discovery\n");
		spin_unlock_irqrestore(&fnic->vlans_lock, flags);
		fnic_event_enq(fnic, FNIC_EVT_START_VLAN_DISC);
		break;
	case FIP_VLAN_SENT:
		if (vlan->sol_count >= FCOE_CTLR_MAX_SOL) {
			/*
                                                      
                       
    */
			shost_printk(KERN_INFO, fnic->lport->host,
				  "Dequeue this VLAN ID %d from list\n",
				  vlan->vid);
			list_del(&vlan->list);
			kfree(vlan);
			vlan = NULL;
			if (list_empty(&fnic->vlans)) {
				/*                                           */
				spin_unlock_irqrestore(&fnic->vlans_lock,
							flags);
				shost_printk(KERN_INFO, fnic->lport->host,
					  "fip_timer: vlan list empty, "
					  "trigger vlan disc\n");
				fnic_event_enq(fnic, FNIC_EVT_START_VLAN_DISC);
				return;
			}
			/*                     */
			vlan = list_first_entry(&fnic->vlans, struct fcoe_vlan,
							list);
			fnic->set_vlan(fnic, vlan->vid);
			vlan->state = FIP_VLAN_SENT; /*          */
		}
		spin_unlock_irqrestore(&fnic->vlans_lock, flags);
		vlan->sol_count++;
		sol_time = jiffies + msecs_to_jiffies
					(FCOE_CTLR_START_DELAY);
		mod_timer(&fnic->fip_timer, round_jiffies(sol_time));
		break;
	}
}
