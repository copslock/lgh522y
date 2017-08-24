#ifndef _LINUX_IF_MACVLAN_H
#define _LINUX_IF_MACVLAN_H

#include <linux/if_link.h>
#include <linux/list.h>
#include <linux/netdevice.h>
#include <linux/netlink.h>
#include <net/netlink.h>
#include <linux/u64_stats_sync.h>

#if defined(CONFIG_MACVTAP) || defined(CONFIG_MACVTAP_MODULE)
struct socket *macvtap_get_socket(struct file *);
#else
#include <linux/err.h>
#include <linux/errno.h>
struct file;
struct socket;
static inline struct socket *macvtap_get_socket(struct file *f)
{
	return ERR_PTR(-EINVAL);
}
#endif /*                */

struct macvlan_port;
struct macvtap_queue;

/* 
                                                   
                                          
                                      
                                                      
                                             
                                         
                                                   
                                  
                                            
 */
struct macvlan_pcpu_stats {
	u64			rx_packets;
	u64			rx_bytes;
	u64			rx_multicast;
	u64			tx_packets;
	u64			tx_bytes;
	struct u64_stats_sync	syncp;
	u32			rx_errors;
	u32			tx_dropped;
};

/*
                                                                    
                                                                     
 */
#define MAX_MACVTAP_QUEUES	(NR_CPUS < 16 ? NR_CPUS : 16)

#define MACVLAN_MC_FILTER_BITS	8
#define MACVLAN_MC_FILTER_SZ	(1 << MACVLAN_MC_FILTER_BITS)

struct macvlan_dev {
	struct net_device	*dev;
	struct list_head	list;
	struct hlist_node	hlist;
	struct macvlan_port	*port;
	struct net_device	*lowerdev;
	struct macvlan_pcpu_stats __percpu *pcpu_stats;

	DECLARE_BITMAP(mc_filter, MACVLAN_MC_FILTER_SZ);

	enum macvlan_mode	mode;
	u16			flags;
	int (*receive)(struct sk_buff *skb);
	int (*forward)(struct net_device *dev, struct sk_buff *skb);
	struct macvtap_queue	*taps[MAX_MACVTAP_QUEUES];
	int			numvtaps;
	int			minor;
};

static inline void macvlan_count_rx(const struct macvlan_dev *vlan,
				    unsigned int len, bool success,
				    bool multicast)
{
	if (likely(success)) {
		struct macvlan_pcpu_stats *pcpu_stats;

		pcpu_stats = this_cpu_ptr(vlan->pcpu_stats);
		u64_stats_update_begin(&pcpu_stats->syncp);
		pcpu_stats->rx_packets++;
		pcpu_stats->rx_bytes += len;
		if (multicast)
			pcpu_stats->rx_multicast++;
		u64_stats_update_end(&pcpu_stats->syncp);
	} else {
		this_cpu_inc(vlan->pcpu_stats->rx_errors);
	}
}

extern void macvlan_common_setup(struct net_device *dev);

extern int macvlan_common_newlink(struct net *src_net, struct net_device *dev,
				  struct nlattr *tb[], struct nlattr *data[],
				  int (*receive)(struct sk_buff *skb),
				  int (*forward)(struct net_device *dev,
						 struct sk_buff *skb));

extern void macvlan_count_rx(const struct macvlan_dev *vlan,
			     unsigned int len, bool success,
			     bool multicast);

extern void macvlan_dellink(struct net_device *dev, struct list_head *head);

extern int macvlan_link_register(struct rtnl_link_ops *ops);

extern netdev_tx_t macvlan_start_xmit(struct sk_buff *skb,
				      struct net_device *dev);

#endif /*                     */
