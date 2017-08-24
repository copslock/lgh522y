#ifndef _NF_QUEUE_H
#define _NF_QUEUE_H

/*                                                     */
struct nf_queue_entry {
	struct list_head	list;
	struct sk_buff		*skb;
	unsigned int		id;

	struct nf_hook_ops	*elem;
	u_int8_t		pf;
	u16			size; /*                                  */
	unsigned int		hook;
	struct net_device	*indev;
	struct net_device	*outdev;
	int			(*okfn)(struct sk_buff *);

	/*                                 */
};

#define nf_queue_entry_reroute(x) ((void *)x + sizeof(struct nf_queue_entry))

/*                */
struct nf_queue_handler {
	int			(*outfn)(struct nf_queue_entry *entry,
					 unsigned int queuenum);
};

void nf_register_queue_handler(const struct nf_queue_handler *qh);
void nf_unregister_queue_handler(void);
extern void nf_reinject(struct nf_queue_entry *entry, unsigned int verdict);

bool nf_queue_entry_get_refs(struct nf_queue_entry *entry);
void nf_queue_entry_release_refs(struct nf_queue_entry *entry);

#endif /*             */
