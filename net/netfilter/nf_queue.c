/*
 * Rusty Russell (C)2000 -- This code is GPL.
 * Patrick McHardy (c) 2006-2012
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/skbuff.h>
#include <linux/netfilter.h>
#include <linux/seq_file.h>
#include <linux/rcupdate.h>
#include <net/protocol.h>
#include <net/netfilter/nf_queue.h>
#include <net/dst.h>

#include "nf_internals.h"

/*
                                                          
                                                              
  
                                                               
                            
 */
static const struct nf_queue_handler __rcu *queue_handler __read_mostly;

/*                                                                    
                                                            */
void nf_register_queue_handler(const struct nf_queue_handler *qh)
{
	/*                                                                  */
	WARN_ON(rcu_access_pointer(queue_handler));
	rcu_assign_pointer(queue_handler, qh);
}
EXPORT_SYMBOL(nf_register_queue_handler);

/*                                               */
void nf_unregister_queue_handler(void)
{
	RCU_INIT_POINTER(queue_handler, NULL);
	synchronize_rcu();
}
EXPORT_SYMBOL(nf_unregister_queue_handler);

void nf_queue_entry_release_refs(struct nf_queue_entry *entry)
{
	/*                                                        */
	if (entry->indev)
		dev_put(entry->indev);
	if (entry->outdev)
		dev_put(entry->outdev);
#ifdef CONFIG_BRIDGE_NETFILTER
	if (entry->skb->nf_bridge) {
		struct nf_bridge_info *nf_bridge = entry->skb->nf_bridge;

		if (nf_bridge->physindev)
			dev_put(nf_bridge->physindev);
		if (nf_bridge->physoutdev)
			dev_put(nf_bridge->physoutdev);
	}
#endif
	/*                                                  */
	module_put(entry->elem->owner);
}
EXPORT_SYMBOL_GPL(nf_queue_entry_release_refs);

/*                                                        */
bool nf_queue_entry_get_refs(struct nf_queue_entry *entry)
{
	if (!try_module_get(entry->elem->owner))
		return false;

	if (entry->indev)
		dev_hold(entry->indev);
	if (entry->outdev)
		dev_hold(entry->outdev);
#ifdef CONFIG_BRIDGE_NETFILTER
	if (entry->skb->nf_bridge) {
		struct nf_bridge_info *nf_bridge = entry->skb->nf_bridge;
		struct net_device *physdev;

		physdev = nf_bridge->physindev;
		if (physdev)
			dev_hold(physdev);
		physdev = nf_bridge->physoutdev;
		if (physdev)
			dev_hold(physdev);
	}
#endif

	return true;
}
EXPORT_SYMBOL_GPL(nf_queue_entry_get_refs);

/*
                                                          
                         
 */
int nf_queue(struct sk_buff *skb,
		      struct nf_hook_ops *elem,
		      u_int8_t pf, unsigned int hook,
		      struct net_device *indev,
		      struct net_device *outdev,
		      int (*okfn)(struct sk_buff *),
		      unsigned int queuenum)
{
	int status = -ENOENT;
	struct nf_queue_entry *entry = NULL;
	const struct nf_afinfo *afinfo;
	const struct nf_queue_handler *qh;

	/*                                                 */
	rcu_read_lock();

	qh = rcu_dereference(queue_handler);
	if (!qh) {
		status = -ESRCH;
		goto err_unlock;
	}

	afinfo = nf_get_afinfo(pf);
	if (!afinfo)
		goto err_unlock;

	entry = kmalloc(sizeof(*entry) + afinfo->route_key_size, GFP_ATOMIC);
	if (!entry) {
		status = -ENOMEM;
		goto err_unlock;
	}

	*entry = (struct nf_queue_entry) {
		.skb	= skb,
		.elem	= elem,
		.pf	= pf,
		.hook	= hook,
		.indev	= indev,
		.outdev	= outdev,
		.okfn	= okfn,
		.size	= sizeof(*entry) + afinfo->route_key_size,
	};

	if (!nf_queue_entry_get_refs(entry)) {
		status = -ECANCELED;
		goto err_unlock;
	}
	skb_dst_force(skb);
	afinfo->saveroute(skb, entry);
	status = qh->outfn(entry, queuenum);

	rcu_read_unlock();

	if (status < 0) {
		nf_queue_entry_release_refs(entry);
		goto err;
	}

	return 0;

err_unlock:
	rcu_read_unlock();
err:
	kfree(entry);
	return status;
}

void nf_reinject(struct nf_queue_entry *entry, unsigned int verdict)
{
	struct sk_buff *skb = entry->skb;
	struct nf_hook_ops *elem = entry->elem;
	const struct nf_afinfo *afinfo;
	int err;

	rcu_read_lock();

	nf_queue_entry_release_refs(entry);

	/*                                             */
	if (verdict == NF_REPEAT) {
		elem = list_entry(elem->list.prev, struct nf_hook_ops, list);
		verdict = NF_ACCEPT;
	}

	if (verdict == NF_ACCEPT) {
		afinfo = nf_get_afinfo(entry->pf);
		if (!afinfo || afinfo->reroute(skb, entry) < 0)
			verdict = NF_DROP;
	}

	if (verdict == NF_ACCEPT) {
	next_hook:
		verdict = nf_iterate(&nf_hooks[entry->pf][entry->hook],
				     skb, entry->hook,
				     entry->indev, entry->outdev, &elem,
				     entry->okfn, INT_MIN);
	}

	switch (verdict & NF_VERDICT_MASK) {
	case NF_ACCEPT:
	case NF_STOP:
		local_bh_disable();
		entry->okfn(skb);
		local_bh_enable();
		break;
	case NF_QUEUE:
		err = nf_queue(skb, elem, entry->pf, entry->hook,
				entry->indev, entry->outdev, entry->okfn,
				verdict >> NF_VERDICT_QBITS);
		if (err < 0) {
			if (err == -ECANCELED)
				goto next_hook;
			if (err == -ESRCH &&
			   (verdict & NF_VERDICT_FLAG_QUEUE_BYPASS))
				goto next_hook;
			kfree_skb(skb);
		}
		break;
	case NF_STOLEN:
		break;
	default:
		kfree_skb(skb);
	}
	rcu_read_unlock();
	kfree(entry);
}
EXPORT_SYMBOL(nf_reinject);
