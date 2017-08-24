/*
 * Copyright (c) 2012 Broadcom Corporation
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <linux/netdevice.h>

#include "brcmu_wifi.h"
#include "brcmu_utils.h"

#include "dhd.h"
#include "dhd_dbg.h"
#include "tracepoint.h"
#include "fwsignal.h"
#include "fweh.h"
#include "fwil.h"

/* 
                                                       
  
                                     
                                          
                                
                            
                                      
 */
struct brcm_ethhdr {
	__be16 subtype;
	__be16 length;
	u8 version;
	u8 oui[3];
	__be16 usr_subtype;
} __packed;

struct brcmf_event_msg_be {
	__be16 version;
	__be16 flags;
	__be32 event_type;
	__be32 status;
	__be32 reason;
	__be32 auth_type;
	__be32 datalen;
	u8 addr[ETH_ALEN];
	char ifname[IFNAMSIZ];
	u8 ifidx;
	u8 bsscfgidx;
} __packed;

/* 
                                                          
  
                               
                                        
                                                 
 */
struct brcmf_event {
	struct ethhdr eth;
	struct brcm_ethhdr hdr;
	struct brcmf_event_msg_be msg;
} __packed;

/* 
                                                            
  
                                
                     
                                                 
                                           
                                                          
                                                         
 */
struct brcmf_fweh_queue_item {
	struct list_head q;
	enum brcmf_fweh_event_code code;
	u8 ifidx;
	u8 ifaddr[ETH_ALEN];
	struct brcmf_event_msg_be emsg;
	u8 data[0];
};

/* 
                                                           
 */
struct brcmf_fweh_event_name {
	enum brcmf_fweh_event_code code;
	const char *name;
};

#ifdef DEBUG
#define BRCMF_ENUM_DEF(id, val) \
	{ val, #id },

/*                                      */
static struct brcmf_fweh_event_name fweh_event_names[] = {
	BRCMF_FWEH_EVENT_ENUM_DEFLIST
};
#undef BRCMF_ENUM_DEF

/* 
                                                               
  
                         
 */
static const char *brcmf_fweh_event_name(enum brcmf_fweh_event_code code)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(fweh_event_names); i++) {
		if (fweh_event_names[i].code == code)
			return fweh_event_names[i].name;
	}
	return "unknown";
}
#else
static const char *brcmf_fweh_event_name(enum brcmf_fweh_event_code code)
{
	return "nodebug";
}
#endif

/* 
                                                     
  
                                       
                             
 */
static void brcmf_fweh_queue_event(struct brcmf_fweh_info *fweh,
				   struct brcmf_fweh_queue_item *event)
{
	ulong flags;

	spin_lock_irqsave(&fweh->evt_q_lock, flags);
	list_add_tail(&event->q, &fweh->event_q);
	spin_unlock_irqrestore(&fweh->evt_q_lock, flags);
	schedule_work(&fweh->event_work);
}

static int brcmf_fweh_call_event_handler(struct brcmf_if *ifp,
					 enum brcmf_fweh_event_code code,
					 struct brcmf_event_msg *emsg,
					 void *data)
{
	struct brcmf_fweh_info *fweh;
	int err = -EINVAL;

	if (ifp) {
		fweh = &ifp->drvr->fweh;

		/*                                                 */
		if (fweh->evt_handler[code])
			err = fweh->evt_handler[code](ifp, emsg, data);
		else
			brcmf_err("unhandled event %d ignored\n", code);
	} else {
		brcmf_err("no interface object\n");
	}
	return err;
}

/* 
                                                  
  
                                    
                      
                                                        
 */
static void brcmf_fweh_handle_if_event(struct brcmf_pub *drvr,
				       struct brcmf_event_msg *emsg,
				       void *data)
{
	struct brcmf_if_event *ifevent = data;
	struct brcmf_if *ifp;
	int err = 0;

	brcmf_dbg(EVENT, "action: %u idx: %u bsscfg: %u flags: %u role: %u\n",
		  ifevent->action, ifevent->ifidx, ifevent->bssidx,
		  ifevent->flags, ifevent->role);

	if (ifevent->ifidx >= BRCMF_MAX_IFS) {
		brcmf_err("invalid interface index: %u\n",
			  ifevent->ifidx);
		return;
	}

	ifp = drvr->iflist[ifevent->bssidx];

	if (ifevent->action == BRCMF_E_IF_ADD) {
		brcmf_dbg(EVENT, "adding %s (%pM)\n", emsg->ifname,
			  emsg->addr);
		ifp = brcmf_add_if(drvr, ifevent->bssidx, ifevent->ifidx,
				   emsg->ifname, emsg->addr);
		if (IS_ERR(ifp))
			return;
		brcmf_fws_add_interface(ifp);
		if (!drvr->fweh.evt_handler[BRCMF_E_IF])
			if (brcmf_net_attach(ifp, false) < 0)
				return;
	}

	if (ifevent->action == BRCMF_E_IF_CHANGE)
		brcmf_fws_reset_interface(ifp);

	err = brcmf_fweh_call_event_handler(ifp, emsg->event_code, emsg, data);

	if (ifevent->action == BRCMF_E_IF_DEL) {
		brcmf_fws_del_interface(ifp);
		brcmf_del_if(drvr, ifevent->bssidx);
	}
}

/* 
                                                         
  
                                       
 */
static struct brcmf_fweh_queue_item *
brcmf_fweh_dequeue_event(struct brcmf_fweh_info *fweh)
{
	struct brcmf_fweh_queue_item *event = NULL;
	ulong flags;

	spin_lock_irqsave(&fweh->evt_q_lock, flags);
	if (!list_empty(&fweh->event_q)) {
		event = list_first_entry(&fweh->event_q,
					 struct brcmf_fweh_queue_item, q);
		list_del(&event->q);
	}
	spin_unlock_irqrestore(&fweh->evt_q_lock, flags);

	return event;
}

/* 
                                                     
  
                        
 */
static void brcmf_fweh_event_worker(struct work_struct *work)
{
	struct brcmf_pub *drvr;
	struct brcmf_if *ifp;
	struct brcmf_fweh_info *fweh;
	struct brcmf_fweh_queue_item *event;
	int err = 0;
	struct brcmf_event_msg_be *emsg_be;
	struct brcmf_event_msg emsg;

	fweh = container_of(work, struct brcmf_fweh_info, event_work);
	drvr = container_of(fweh, struct brcmf_pub, fweh);

	while ((event = brcmf_fweh_dequeue_event(fweh))) {
		brcmf_dbg(EVENT, "event %s (%u) ifidx %u bsscfg %u addr %pM\n",
			  brcmf_fweh_event_name(event->code), event->code,
			  event->emsg.ifidx, event->emsg.bsscfgidx,
			  event->emsg.addr);

		/*                       */
		emsg_be = &event->emsg;
		emsg.version = be16_to_cpu(emsg_be->version);
		emsg.flags = be16_to_cpu(emsg_be->flags);
		emsg.event_code = event->code;
		emsg.status = be32_to_cpu(emsg_be->status);
		emsg.reason = be32_to_cpu(emsg_be->reason);
		emsg.auth_type = be32_to_cpu(emsg_be->auth_type);
		emsg.datalen = be32_to_cpu(emsg_be->datalen);
		memcpy(emsg.addr, emsg_be->addr, ETH_ALEN);
		memcpy(emsg.ifname, emsg_be->ifname, sizeof(emsg.ifname));
		emsg.ifidx = emsg_be->ifidx;
		emsg.bsscfgidx = emsg_be->bsscfgidx;

		brcmf_dbg(EVENT, "  version %u flags %u status %u reason %u\n",
			  emsg.version, emsg.flags, emsg.status, emsg.reason);
		brcmf_dbg_hex_dump(BRCMF_EVENT_ON(), event->data,
				   min_t(u32, emsg.datalen, 64),
				   "event payload, len=%d\n", emsg.datalen);

		/*                                     */
		if (event->code == BRCMF_E_IF) {
			brcmf_fweh_handle_if_event(drvr, &emsg, event->data);
			goto event_free;
		}

		ifp = drvr->iflist[emsg.bsscfgidx];
		err = brcmf_fweh_call_event_handler(ifp, event->code, &emsg,
						    event->data);
		if (err) {
			brcmf_err("event handler failed (%d)\n",
				  event->code);
			err = 0;
		}
event_free:
		kfree(event);
	}
}

/* 
                                                            
  
                                    
 */
void brcmf_fweh_attach(struct brcmf_pub *drvr)
{
	struct brcmf_fweh_info *fweh = &drvr->fweh;
	INIT_WORK(&fweh->event_work, brcmf_fweh_event_worker);
	spin_lock_init(&fweh->evt_q_lock);
	INIT_LIST_HEAD(&fweh->event_q);
}

/* 
                                                         
  
                                    
 */
void brcmf_fweh_detach(struct brcmf_pub *drvr)
{
	struct brcmf_fweh_info *fweh = &drvr->fweh;
	struct brcmf_if *ifp = drvr->iflist[0];
	s8 eventmask[BRCMF_EVENTING_MASK_LEN];

	if (ifp) {
		/*                  */
		memset(eventmask, 0, BRCMF_EVENTING_MASK_LEN);
		(void)brcmf_fil_iovar_data_set(ifp, "event_msgs",
					       eventmask,
					       BRCMF_EVENTING_MASK_LEN);
	}
	/*                   */
	cancel_work_sync(&fweh->event_work);
	WARN_ON(!list_empty(&fweh->event_q));
	memset(fweh->evt_handler, 0, sizeof(fweh->evt_handler));
}

/* 
                                                                 
  
                                    
                     
                                              
 */
int brcmf_fweh_register(struct brcmf_pub *drvr, enum brcmf_fweh_event_code code,
			brcmf_fweh_handler_t handler)
{
	if (drvr->fweh.evt_handler[code]) {
		brcmf_err("event code %d already registered\n", code);
		return -ENOSPC;
	}
	drvr->fweh.evt_handler[code] = handler;
	brcmf_dbg(TRACE, "event handler registered for %s\n",
		  brcmf_fweh_event_name(code));
	return 0;
}

/* 
                                                           
  
                                    
                     
 */
void brcmf_fweh_unregister(struct brcmf_pub *drvr,
			   enum brcmf_fweh_event_code code)
{
	brcmf_dbg(TRACE, "event handler cleared for %s\n",
		  brcmf_fweh_event_name(code));
	drvr->fweh.evt_handler[code] = NULL;
}

/* 
                                                                     
  
                                  
 */
int brcmf_fweh_activate_events(struct brcmf_if *ifp)
{
	int i, err;
	s8 eventmask[BRCMF_EVENTING_MASK_LEN];

	for (i = 0; i < BRCMF_E_LAST; i++) {
		if (ifp->drvr->fweh.evt_handler[i]) {
			brcmf_dbg(EVENT, "enable event %s\n",
				  brcmf_fweh_event_name(i));
			setbit(eventmask, i);
		}
	}

	/*                                 */
	brcmf_dbg(EVENT, "enable event IF\n");
	setbit(eventmask, BRCMF_E_IF);

	err = brcmf_fil_iovar_data_set(ifp, "event_msgs",
				       eventmask, BRCMF_EVENTING_MASK_LEN);
	if (err)
		brcmf_err("Set event_msgs error (%d)\n", err);

	return err;
}

/* 
                                                              
  
                                    
                                          
  
                                                                 
                                                             
 */
void brcmf_fweh_process_event(struct brcmf_pub *drvr,
			      struct brcmf_event *event_packet)
{
	enum brcmf_fweh_event_code code;
	struct brcmf_fweh_info *fweh = &drvr->fweh;
	struct brcmf_fweh_queue_item *event;
	gfp_t alloc_flag = GFP_KERNEL;
	void *data;
	u32 datalen;

	/*                */
	code = get_unaligned_be32(&event_packet->msg.event_type);
	datalen = get_unaligned_be32(&event_packet->msg.datalen);
	data = &event_packet[1];

	if (code >= BRCMF_E_LAST)
		return;

	if (code != BRCMF_E_IF && !fweh->evt_handler[code])
		return;

	if (in_interrupt())
		alloc_flag = GFP_ATOMIC;

	event = kzalloc(sizeof(*event) + datalen, alloc_flag);
	if (!event)
		return;

	event->code = code;
	event->ifidx = event_packet->msg.ifidx;

	/*                                         */
	memcpy(&event->emsg, &event_packet->msg, sizeof(event->emsg));
	memcpy(event->data, data, datalen);
	memcpy(event->ifaddr, event_packet->eth.h_dest, ETH_ALEN);

	brcmf_fweh_queue_event(fweh, event);
}
