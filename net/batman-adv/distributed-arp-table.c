/* Copyright (C) 2011-2013 B.A.T.M.A.N. contributors:
 *
 * Antonio Quartulli
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 */

#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <net/arp.h>

#include "main.h"
#include "hash.h"
#include "distributed-arp-table.h"
#include "hard-interface.h"
#include "originator.h"
#include "send.h"
#include "types.h"
#include "translation-table.h"
#include "unicast.h"

static void batadv_dat_purge(struct work_struct *work);

/* 
                                                              
                                                                  
 */
static void batadv_dat_start_timer(struct batadv_priv *bat_priv)
{
	INIT_DELAYED_WORK(&bat_priv->dat.work, batadv_dat_purge);
	queue_delayed_work(batadv_event_workqueue, &bat_priv->dat.work,
			   msecs_to_jiffies(10000));
}

/* 
                                                                               
          
                                 
 */
static void batadv_dat_entry_free_ref(struct batadv_dat_entry *dat_entry)
{
	if (atomic_dec_and_test(&dat_entry->refcount))
		kfree_rcu(dat_entry, rcu);
}

/* 
                                                                           
                                 
  
                                                                  
 */
static bool batadv_dat_to_purge(struct batadv_dat_entry *dat_entry)
{
	return batadv_has_timed_out(dat_entry->last_update,
				    BATADV_DAT_ENTRY_TIMEOUT);
}

/* 
                                                                 
                                                                  
                                                                               
                                                                      
                                                                      
                        
  
                                                                              
                                                        
 */
static void __batadv_dat_purge(struct batadv_priv *bat_priv,
			       bool (*to_purge)(struct batadv_dat_entry *))
{
	spinlock_t *list_lock; /*                                         */
	struct batadv_dat_entry *dat_entry;
	struct hlist_node *node_tmp;
	struct hlist_head *head;
	uint32_t i;

	if (!bat_priv->dat.hash)
		return;

	for (i = 0; i < bat_priv->dat.hash->size; i++) {
		head = &bat_priv->dat.hash->table[i];
		list_lock = &bat_priv->dat.hash->list_locks[i];

		spin_lock_bh(list_lock);
		hlist_for_each_entry_safe(dat_entry, node_tmp, head,
					  hash_entry) {
			/*                                                    
                                                 
    */
			if (to_purge && !to_purge(dat_entry))
				continue;

			hlist_del_rcu(&dat_entry->hash_entry);
			batadv_dat_entry_free_ref(dat_entry);
		}
		spin_unlock_bh(list_lock);
	}
}

/* 
                                                                               
             
                            
 */
static void batadv_dat_purge(struct work_struct *work)
{
	struct delayed_work *delayed_work;
	struct batadv_priv_dat *priv_dat;
	struct batadv_priv *bat_priv;

	delayed_work = container_of(work, struct delayed_work, work);
	priv_dat = container_of(delayed_work, struct batadv_priv_dat, work);
	bat_priv = container_of(priv_dat, struct batadv_priv, dat);

	__batadv_dat_purge(bat_priv, batadv_dat_to_purge);
	batadv_dat_start_timer(bat_priv);
}

/* 
                                                                           
                                 
                                               
  
                                                       
 */
static int batadv_compare_dat(const struct hlist_node *node, const void *data2)
{
	const void *data1 = container_of(node, struct batadv_dat_entry,
					 hash_entry);

	return (memcmp(data1, data2, sizeof(__be32)) == 0 ? 1 : 0);
}

/* 
                                                                  
                   
                                                               
  
                                                          
 */
static uint8_t *batadv_arp_hw_src(struct sk_buff *skb, int hdr_size)
{
	uint8_t *addr;

	addr = (uint8_t *)(skb->data + hdr_size);
	addr += ETH_HLEN + sizeof(struct arphdr);

	return addr;
}

/* 
                                                                  
                   
                                                               
  
                                                          
 */
static __be32 batadv_arp_ip_src(struct sk_buff *skb, int hdr_size)
{
	return *(__be32 *)(batadv_arp_hw_src(skb, hdr_size) + ETH_ALEN);
}

/* 
                                                                  
                   
                                                               
  
                                                          
 */
static uint8_t *batadv_arp_hw_dst(struct sk_buff *skb, int hdr_size)
{
	return batadv_arp_hw_src(skb, hdr_size) + ETH_ALEN + 4;
}

/* 
                                                                  
                   
                                                               
  
                                                          
 */
static __be32 batadv_arp_ip_dst(struct sk_buff *skb, int hdr_size)
{
	return *(__be32 *)(batadv_arp_hw_src(skb, hdr_size) + ETH_ALEN * 2 + 4);
}

/* 
                                                             
                      
                                
  
                                                                  
 */
static uint32_t batadv_hash_dat(const void *data, uint32_t size)
{
	const unsigned char *key = data;
	uint32_t hash = 0;
	size_t i;

	for (i = 0; i < 4; i++) {
		hash += key[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash % size;
}

/* 
                                                                             
        
                                                                  
                  
  
                                                 
 */
static struct batadv_dat_entry *
batadv_dat_entry_hash_find(struct batadv_priv *bat_priv, __be32 ip)
{
	struct hlist_head *head;
	struct batadv_dat_entry *dat_entry, *dat_entry_tmp = NULL;
	struct batadv_hashtable *hash = bat_priv->dat.hash;
	uint32_t index;

	if (!hash)
		return NULL;

	index = batadv_hash_dat(&ip, hash->size);
	head = &hash->table[index];

	rcu_read_lock();
	hlist_for_each_entry_rcu(dat_entry, head, hash_entry) {
		if (dat_entry->ip != ip)
			continue;

		if (!atomic_inc_not_zero(&dat_entry->refcount))
			continue;

		dat_entry_tmp = dat_entry;
		break;
	}
	rcu_read_unlock();

	return dat_entry_tmp;
}

/* 
                                                                            
                                                                  
                        
                                                     
 */
static void batadv_dat_entry_add(struct batadv_priv *bat_priv, __be32 ip,
				 uint8_t *mac_addr)
{
	struct batadv_dat_entry *dat_entry;
	int hash_added;

	dat_entry = batadv_dat_entry_hash_find(bat_priv, ip);
	/*                                                */
	if (dat_entry) {
		if (!batadv_compare_eth(dat_entry->mac_addr, mac_addr))
			memcpy(dat_entry->mac_addr, mac_addr, ETH_ALEN);
		dat_entry->last_update = jiffies;
		batadv_dbg(BATADV_DBG_DAT, bat_priv,
			   "Entry updated: %pI4 %pM\n", &dat_entry->ip,
			   dat_entry->mac_addr);
		goto out;
	}

	dat_entry = kmalloc(sizeof(*dat_entry), GFP_ATOMIC);
	if (!dat_entry)
		goto out;

	dat_entry->ip = ip;
	memcpy(dat_entry->mac_addr, mac_addr, ETH_ALEN);
	dat_entry->last_update = jiffies;
	atomic_set(&dat_entry->refcount, 2);

	hash_added = batadv_hash_add(bat_priv->dat.hash, batadv_compare_dat,
				     batadv_hash_dat, &dat_entry->ip,
				     &dat_entry->hash_entry);

	if (unlikely(hash_added != 0)) {
		/*                                   */
		batadv_dat_entry_free_ref(dat_entry);
		goto out;
	}

	batadv_dbg(BATADV_DBG_DAT, bat_priv, "New entry added: %pI4 %pM\n",
		   &dat_entry->ip, dat_entry->mac_addr);

out:
	if (dat_entry)
		batadv_dat_entry_free_ref(dat_entry);
}

#ifdef CONFIG_BATMAN_ADV_DEBUG

/* 
                                                                               
                                                                  
                   
                  
                                                               
                                                                 
 */
static void batadv_dbg_arp(struct batadv_priv *bat_priv, struct sk_buff *skb,
			   uint16_t type, int hdr_size, char *msg)
{
	struct batadv_unicast_4addr_packet *unicast_4addr_packet;
	struct batadv_bcast_packet *bcast_pkt;
	uint8_t *orig_addr;
	__be32 ip_src, ip_dst;

	if (msg)
		batadv_dbg(BATADV_DBG_DAT, bat_priv, "%s\n", msg);

	ip_src = batadv_arp_ip_src(skb, hdr_size);
	ip_dst = batadv_arp_ip_dst(skb, hdr_size);
	batadv_dbg(BATADV_DBG_DAT, bat_priv,
		   "ARP MSG = [src: %pM-%pI4 dst: %pM-%pI4]\n",
		   batadv_arp_hw_src(skb, hdr_size), &ip_src,
		   batadv_arp_hw_dst(skb, hdr_size), &ip_dst);

	if (hdr_size == 0)
		return;

	/*                                                                  
                       
  */
	unicast_4addr_packet = (struct batadv_unicast_4addr_packet *)skb->data;

	switch (unicast_4addr_packet->u.header.packet_type) {
	case BATADV_UNICAST:
		batadv_dbg(BATADV_DBG_DAT, bat_priv,
			   "* encapsulated within a UNICAST packet\n");
		break;
	case BATADV_UNICAST_4ADDR:
		batadv_dbg(BATADV_DBG_DAT, bat_priv,
			   "* encapsulated within a UNICAST_4ADDR packet (src: %pM)\n",
			   unicast_4addr_packet->src);
		switch (unicast_4addr_packet->subtype) {
		case BATADV_P_DAT_DHT_PUT:
			batadv_dbg(BATADV_DBG_DAT, bat_priv, "* type: DAT_DHT_PUT\n");
			break;
		case BATADV_P_DAT_DHT_GET:
			batadv_dbg(BATADV_DBG_DAT, bat_priv, "* type: DAT_DHT_GET\n");
			break;
		case BATADV_P_DAT_CACHE_REPLY:
			batadv_dbg(BATADV_DBG_DAT, bat_priv,
				   "* type: DAT_CACHE_REPLY\n");
			break;
		case BATADV_P_DATA:
			batadv_dbg(BATADV_DBG_DAT, bat_priv, "* type: DATA\n");
			break;
		default:
			batadv_dbg(BATADV_DBG_DAT, bat_priv, "* type: Unknown (%u)!\n",
				   unicast_4addr_packet->u.header.packet_type);
		}
		break;
	case BATADV_BCAST:
		bcast_pkt = (struct batadv_bcast_packet *)unicast_4addr_packet;
		orig_addr = bcast_pkt->orig;
		batadv_dbg(BATADV_DBG_DAT, bat_priv,
			   "* encapsulated within a BCAST packet (src: %pM)\n",
			   orig_addr);
		break;
	default:
		batadv_dbg(BATADV_DBG_DAT, bat_priv,
			   "* encapsulated within an unknown packet type (0x%x)\n",
			   unicast_4addr_packet->u.header.packet_type);
	}
}

#else

static void batadv_dbg_arp(struct batadv_priv *bat_priv, struct sk_buff *skb,
			   uint16_t type, int hdr_size, char *msg)
{
}

#endif /*                         */

/* 
                                                                             
                                                       
                                                 
                                                    
                                  
                                                    
                                         
                                          
  
                                                                                
 */
static bool batadv_is_orig_node_eligible(struct batadv_dat_candidate *res,
					 int select, batadv_dat_addr_t tmp_max,
					 batadv_dat_addr_t max,
					 batadv_dat_addr_t last_max,
					 struct batadv_orig_node *candidate,
					 struct batadv_orig_node *max_orig_node)
{
	bool ret = false;
	int j;

	/*                                                 */
	for (j = 0; j < select; j++)
		if (res[j].orig_node == candidate)
			break;
	/*                        */
	if (j < select)
		goto out;
	/*                                                                    */
	if (tmp_max > last_max)
		goto out;
	/*                                                               
                                  
  */
	if (tmp_max < max)
		goto out;
	/*                                                                   
                                   
  */
	if ((tmp_max == max) && max_orig_node &&
	    (batadv_compare_eth(candidate->orig, max_orig_node->orig) > 0))
		goto out;

	ret = true;
out:
	return ret;
}

/* 
                                                               
                                                                  
                           
                                                             
                                     
                                                                               
 */
static void batadv_choose_next_candidate(struct batadv_priv *bat_priv,
					 struct batadv_dat_candidate *cands,
					 int select, batadv_dat_addr_t ip_key,
					 batadv_dat_addr_t *last_max)
{
	batadv_dat_addr_t max = 0, tmp_max = 0;
	struct batadv_orig_node *orig_node, *max_orig_node = NULL;
	struct batadv_hashtable *hash = bat_priv->orig_hash;
	struct hlist_head *head;
	int i;

	/*                                                                 
             
  */
	cands[select].type = BATADV_DAT_CANDIDATE_NOT_FOUND;

	/*                                                                
                                               
  */
	for (i = 0; i < hash->size; i++) {
		head = &hash->table[i];

		rcu_read_lock();
		hlist_for_each_entry_rcu(orig_node, head, hash_entry) {
			/*                                                    */
			tmp_max = BATADV_DAT_ADDR_MAX - orig_node->dat_addr +
				  ip_key;

			if (!batadv_is_orig_node_eligible(cands, select,
							  tmp_max, max,
							  *last_max, orig_node,
							  max_orig_node))
				continue;

			if (!atomic_inc_not_zero(&orig_node->refcount))
				continue;

			max = tmp_max;
			if (max_orig_node)
				batadv_orig_node_free_ref(max_orig_node);
			max_orig_node = orig_node;
		}
		rcu_read_unlock();
	}
	if (max_orig_node) {
		cands[select].type = BATADV_DAT_CANDIDATE_ORIG;
		cands[select].orig_node = max_orig_node;
		batadv_dbg(BATADV_DBG_DAT, bat_priv,
			   "dat_select_candidates() %d: selected %pM addr=%u dist=%u\n",
			   select, max_orig_node->orig, max_orig_node->dat_addr,
			   max);
	}
	*last_max = max;
}

/* 
                                                                                
             
                                                                  
                                      
  
                                                                              
                                                                           
                                       
  
                                                               
 */
static struct batadv_dat_candidate *
batadv_dat_select_candidates(struct batadv_priv *bat_priv, __be32 ip_dst)
{
	int select;
	batadv_dat_addr_t last_max = BATADV_DAT_ADDR_MAX, ip_key;
	struct batadv_dat_candidate *res;

	if (!bat_priv->orig_hash)
		return NULL;

	res = kmalloc(BATADV_DAT_CANDIDATES_NUM * sizeof(*res), GFP_ATOMIC);
	if (!res)
		return NULL;

	ip_key = (batadv_dat_addr_t)batadv_hash_dat(&ip_dst,
						    BATADV_DAT_ADDR_MAX);

	batadv_dbg(BATADV_DBG_DAT, bat_priv,
		   "dat_select_candidates(): IP=%pI4 hash(IP)=%u\n", &ip_dst,
		   ip_key);

	for (select = 0; select < BATADV_DAT_CANDIDATES_NUM; select++)
		batadv_choose_next_candidate(bat_priv, res, select, ip_key,
					     &last_max);

	return res;
}

/* 
                                                                   
                                                                  
                        
                   
                                                      
  
                                                                            
                                                    
  
                                                                                
 */
static bool batadv_dat_send_data(struct batadv_priv *bat_priv,
				 struct sk_buff *skb, __be32 ip,
				 int packet_subtype)
{
	int i;
	bool ret = false;
	int send_status;
	struct batadv_neigh_node *neigh_node = NULL;
	struct sk_buff *tmp_skb;
	struct batadv_dat_candidate *cand;

	cand = batadv_dat_select_candidates(bat_priv, ip);
	if (!cand)
		goto out;

	batadv_dbg(BATADV_DBG_DAT, bat_priv, "DHT_SEND for %pI4\n", &ip);

	for (i = 0; i < BATADV_DAT_CANDIDATES_NUM; i++) {
		if (cand[i].type == BATADV_DAT_CANDIDATE_NOT_FOUND)
			continue;

		neigh_node = batadv_orig_node_get_router(cand[i].orig_node);
		if (!neigh_node)
			goto free_orig;

		tmp_skb = pskb_copy(skb, GFP_ATOMIC);
		if (!batadv_unicast_4addr_prepare_skb(bat_priv, tmp_skb,
						      cand[i].orig_node,
						      packet_subtype)) {
			kfree_skb(tmp_skb);
			goto free_neigh;
		}

		send_status = batadv_send_skb_packet(tmp_skb,
						     neigh_node->if_incoming,
						     neigh_node->addr);
		if (send_status == NET_XMIT_SUCCESS) {
			/*                       */
			switch (packet_subtype) {
			case BATADV_P_DAT_DHT_GET:
				batadv_inc_counter(bat_priv,
						   BATADV_CNT_DAT_GET_TX);
				break;
			case BATADV_P_DAT_DHT_PUT:
				batadv_inc_counter(bat_priv,
						   BATADV_CNT_DAT_PUT_TX);
				break;
			}

			/*                                         */
			ret = true;
		}
free_neigh:
		batadv_neigh_node_free_ref(neigh_node);
free_orig:
		batadv_orig_node_free_ref(cand[i].orig_node);
	}

out:
	kfree(cand);
	return ret;
}

/* 
                                                       
                                                                  
 */
static void batadv_dat_hash_free(struct batadv_priv *bat_priv)
{
	if (!bat_priv->dat.hash)
		return;

	__batadv_dat_purge(bat_priv, NULL);

	batadv_hash_destroy(bat_priv->dat.hash);

	bat_priv->dat.hash = NULL;
}

/* 
                                                 
                                                                  
 */
int batadv_dat_init(struct batadv_priv *bat_priv)
{
	if (bat_priv->dat.hash)
		return 0;

	bat_priv->dat.hash = batadv_hash_new(1024);

	if (!bat_priv->dat.hash)
		return -ENOMEM;

	batadv_dat_start_timer(bat_priv);

	return 0;
}

/* 
                                           
                                                                  
 */
void batadv_dat_free(struct batadv_priv *bat_priv)
{
	cancel_delayed_work_sync(&bat_priv->dat.work);

	batadv_dat_hash_free(bat_priv);
}

/* 
                                                                   
                             
                    
 */
int batadv_dat_cache_seq_print_text(struct seq_file *seq, void *offset)
{
	struct net_device *net_dev = (struct net_device *)seq->private;
	struct batadv_priv *bat_priv = netdev_priv(net_dev);
	struct batadv_hashtable *hash = bat_priv->dat.hash;
	struct batadv_dat_entry *dat_entry;
	struct batadv_hard_iface *primary_if;
	struct hlist_head *head;
	unsigned long last_seen_jiffies;
	int last_seen_msecs, last_seen_secs, last_seen_mins;
	uint32_t i;

	primary_if = batadv_seq_print_text_primary_if_get(seq);
	if (!primary_if)
		goto out;

	seq_printf(seq, "Distributed ARP Table (%s):\n", net_dev->name);
	seq_printf(seq, "          %-7s          %-13s %5s\n", "IPv4", "MAC",
		   "last-seen");

	for (i = 0; i < hash->size; i++) {
		head = &hash->table[i];

		rcu_read_lock();
		hlist_for_each_entry_rcu(dat_entry, head, hash_entry) {
			last_seen_jiffies = jiffies - dat_entry->last_update;
			last_seen_msecs = jiffies_to_msecs(last_seen_jiffies);
			last_seen_mins = last_seen_msecs / 60000;
			last_seen_msecs = last_seen_msecs % 60000;
			last_seen_secs = last_seen_msecs / 1000;

			seq_printf(seq, " * %15pI4 %14pM %6i:%02i\n",
				   &dat_entry->ip, dat_entry->mac_addr,
				   last_seen_mins, last_seen_secs);
		}
		rcu_read_unlock();
	}

out:
	if (primary_if)
		batadv_hardif_free_ref(primary_if);
	return 0;
}

/* 
                                                              
                                                                  
                          
                                                                          
  
                                                                           
 */
static uint16_t batadv_arp_get_type(struct batadv_priv *bat_priv,
				    struct sk_buff *skb, int hdr_size)
{
	struct arphdr *arphdr;
	struct ethhdr *ethhdr;
	__be32 ip_src, ip_dst;
	uint8_t *hw_src, *hw_dst;
	uint16_t type = 0;

	/*                          */
	if (unlikely(!pskb_may_pull(skb, hdr_size + ETH_HLEN)))
		goto out;

	ethhdr = (struct ethhdr *)(skb->data + hdr_size);

	if (ethhdr->h_proto != htons(ETH_P_ARP))
		goto out;

	/*                      */
	if (unlikely(!pskb_may_pull(skb, hdr_size + ETH_HLEN +
				    arp_hdr_len(skb->dev))))
		goto out;

	arphdr = (struct arphdr *)(skb->data + hdr_size + ETH_HLEN);

	/*                                             
                  
  */
	if (arphdr->ar_hrd != htons(ARPHRD_ETHER))
		goto out;

	if (arphdr->ar_pro != htons(ETH_P_IP))
		goto out;

	if (arphdr->ar_hln != ETH_ALEN)
		goto out;

	if (arphdr->ar_pln != 4)
		goto out;

	/*                                                                 
                         
  */
	ip_src = batadv_arp_ip_src(skb, hdr_size);
	ip_dst = batadv_arp_ip_dst(skb, hdr_size);
	if (ipv4_is_loopback(ip_src) || ipv4_is_multicast(ip_src) ||
	    ipv4_is_loopback(ip_dst) || ipv4_is_multicast(ip_dst) ||
	    ipv4_is_zeronet(ip_src) || ipv4_is_lbcast(ip_src) ||
	    ipv4_is_zeronet(ip_dst) || ipv4_is_lbcast(ip_dst))
		goto out;

	hw_src = batadv_arp_hw_src(skb, hdr_size);
	if (is_zero_ether_addr(hw_src) || is_multicast_ether_addr(hw_src))
		goto out;

	/*                                                                 */
	if (arphdr->ar_op != htons(ARPOP_REQUEST)) {
		hw_dst = batadv_arp_hw_dst(skb, hdr_size);
		if (is_zero_ether_addr(hw_dst) ||
		    is_multicast_ether_addr(hw_dst))
			goto out;
	}

	type = ntohs(arphdr->ar_op);
out:
	return type;
}

/* 
                                                                           
                   
                                                                  
                        
  
                                                                         
                                                                          
           
 */
bool batadv_dat_snoop_outgoing_arp_request(struct batadv_priv *bat_priv,
					   struct sk_buff *skb)
{
	uint16_t type = 0;
	__be32 ip_dst, ip_src;
	uint8_t *hw_src;
	bool ret = false;
	struct batadv_dat_entry *dat_entry = NULL;
	struct sk_buff *skb_new;

	if (!atomic_read(&bat_priv->distributed_arp_table))
		goto out;

	type = batadv_arp_get_type(bat_priv, skb, 0);
	/*                                                                 
                                          
  */
	if (type != ARPOP_REQUEST)
		goto out;

	batadv_dbg_arp(bat_priv, skb, type, 0, "Parsing outgoing ARP REQUEST");

	ip_src = batadv_arp_ip_src(skb, 0);
	hw_src = batadv_arp_hw_src(skb, 0);
	ip_dst = batadv_arp_ip_dst(skb, 0);

	batadv_dat_entry_add(bat_priv, ip_src, hw_src);

	dat_entry = batadv_dat_entry_hash_find(bat_priv, ip_dst);
	if (dat_entry) {
		/*                                                            
                                                         
                      
    
                                                                  
                                                            
                                         
   */
		if (batadv_is_my_client(bat_priv, dat_entry->mac_addr)) {
			ret = true;
			goto out;
		}

		skb_new = arp_create(ARPOP_REPLY, ETH_P_ARP, ip_src,
				     bat_priv->soft_iface, ip_dst, hw_src,
				     dat_entry->mac_addr, hw_src);
		if (!skb_new)
			goto out;

		skb_reset_mac_header(skb_new);
		skb_new->protocol = eth_type_trans(skb_new,
						   bat_priv->soft_iface);
		bat_priv->stats.rx_packets++;
		bat_priv->stats.rx_bytes += skb->len + ETH_HLEN;
		bat_priv->soft_iface->last_rx = jiffies;

		netif_rx(skb_new);
		batadv_dbg(BATADV_DBG_DAT, bat_priv, "ARP request replied locally\n");
		ret = true;
	} else {
		/*                             */
		ret = batadv_dat_send_data(bat_priv, skb, ip_dst,
					   BATADV_P_DAT_DHT_GET);
	}
out:
	if (dat_entry)
		batadv_dat_entry_free_ref(dat_entry);
	return ret;
}

/* 
                                                                           
                                     
                                                                  
                        
                                              
  
                                                                 
 */
bool batadv_dat_snoop_incoming_arp_request(struct batadv_priv *bat_priv,
					   struct sk_buff *skb, int hdr_size)
{
	uint16_t type;
	__be32 ip_src, ip_dst;
	uint8_t *hw_src;
	struct sk_buff *skb_new;
	struct batadv_dat_entry *dat_entry = NULL;
	bool ret = false;
	int err;

	if (!atomic_read(&bat_priv->distributed_arp_table))
		goto out;

	type = batadv_arp_get_type(bat_priv, skb, hdr_size);
	if (type != ARPOP_REQUEST)
		goto out;

	hw_src = batadv_arp_hw_src(skb, hdr_size);
	ip_src = batadv_arp_ip_src(skb, hdr_size);
	ip_dst = batadv_arp_ip_dst(skb, hdr_size);

	batadv_dbg_arp(bat_priv, skb, type, hdr_size,
		       "Parsing incoming ARP REQUEST");

	batadv_dat_entry_add(bat_priv, ip_src, hw_src);

	dat_entry = batadv_dat_entry_hash_find(bat_priv, ip_dst);
	if (!dat_entry)
		goto out;

	skb_new = arp_create(ARPOP_REPLY, ETH_P_ARP, ip_src,
			     bat_priv->soft_iface, ip_dst, hw_src,
			     dat_entry->mac_addr, hw_src);

	if (!skb_new)
		goto out;

	/*                                                                 
                                                                       
                                                                     
               
  */
	if (hdr_size == sizeof(struct batadv_unicast_4addr_packet))
		err = batadv_unicast_4addr_send_skb(bat_priv, skb_new,
						    BATADV_P_DAT_CACHE_REPLY);
	else
		err = batadv_unicast_send_skb(bat_priv, skb_new);

	if (!err) {
		batadv_inc_counter(bat_priv, BATADV_CNT_DAT_CACHED_REPLY_TX);
		ret = true;
	}
out:
	if (dat_entry)
		batadv_dat_entry_free_ref(dat_entry);
	if (ret)
		kfree_skb(skb);
	return ret;
}

/* 
                                                                             
                                                                  
                        
 */
void batadv_dat_snoop_outgoing_arp_reply(struct batadv_priv *bat_priv,
					 struct sk_buff *skb)
{
	uint16_t type;
	__be32 ip_src, ip_dst;
	uint8_t *hw_src, *hw_dst;

	if (!atomic_read(&bat_priv->distributed_arp_table))
		return;

	type = batadv_arp_get_type(bat_priv, skb, 0);
	if (type != ARPOP_REPLY)
		return;

	batadv_dbg_arp(bat_priv, skb, type, 0, "Parsing outgoing ARP REPLY");

	hw_src = batadv_arp_hw_src(skb, 0);
	ip_src = batadv_arp_ip_src(skb, 0);
	hw_dst = batadv_arp_hw_dst(skb, 0);
	ip_dst = batadv_arp_ip_dst(skb, 0);

	batadv_dat_entry_add(bat_priv, ip_src, hw_src);
	batadv_dat_entry_add(bat_priv, ip_dst, hw_dst);

	/*                                                                    
                                     
  */
	batadv_dat_send_data(bat_priv, skb, ip_src, BATADV_P_DAT_DHT_PUT);
	batadv_dat_send_data(bat_priv, skb, ip_dst, BATADV_P_DAT_DHT_PUT);
}
/* 
                                                                               
                   
                                                                  
                        
                                               
 */
bool batadv_dat_snoop_incoming_arp_reply(struct batadv_priv *bat_priv,
					 struct sk_buff *skb, int hdr_size)
{
	uint16_t type;
	__be32 ip_src, ip_dst;
	uint8_t *hw_src, *hw_dst;
	bool ret = false;

	if (!atomic_read(&bat_priv->distributed_arp_table))
		goto out;

	type = batadv_arp_get_type(bat_priv, skb, hdr_size);
	if (type != ARPOP_REPLY)
		goto out;

	batadv_dbg_arp(bat_priv, skb, type, hdr_size,
		       "Parsing incoming ARP REPLY");

	hw_src = batadv_arp_hw_src(skb, hdr_size);
	ip_src = batadv_arp_ip_src(skb, hdr_size);
	hw_dst = batadv_arp_hw_dst(skb, hdr_size);
	ip_dst = batadv_arp_ip_dst(skb, hdr_size);

	/*                                                                  
                        
  */
	batadv_dat_entry_add(bat_priv, ip_src, hw_src);
	batadv_dat_entry_add(bat_priv, ip_dst, hw_dst);

	/*                                                                 
                           
  */
	ret = !batadv_is_my_client(bat_priv, hw_dst);
out:
	if (ret)
		kfree_skb(skb);
	/*                                                                */
	return ret;
}

/* 
                                                                               
                                                              
                                                                  
                                     
  
                                                                
 */
bool batadv_dat_drop_broadcast_packet(struct batadv_priv *bat_priv,
				      struct batadv_forw_packet *forw_packet)
{
	uint16_t type;
	__be32 ip_dst;
	struct batadv_dat_entry *dat_entry = NULL;
	bool ret = false;
	const size_t bcast_len = sizeof(struct batadv_bcast_packet);

	if (!atomic_read(&bat_priv->distributed_arp_table))
		goto out;

	/*                                                              
                                                                       
  */
	if (forw_packet->num_packets)
		goto out;

	type = batadv_arp_get_type(bat_priv, forw_packet->skb, bcast_len);
	if (type != ARPOP_REQUEST)
		goto out;

	ip_dst = batadv_arp_ip_dst(forw_packet->skb, bcast_len);
	dat_entry = batadv_dat_entry_hash_find(bat_priv, ip_dst);
	/*                                          */
	if (!dat_entry) {
		batadv_dbg(BATADV_DBG_DAT, bat_priv,
			   "ARP Request for %pI4: fallback\n", &ip_dst);
		goto out;
	}

	batadv_dbg(BATADV_DBG_DAT, bat_priv,
		   "ARP Request for %pI4: fallback prevented\n", &ip_dst);
	ret = true;

out:
	if (dat_entry)
		batadv_dat_entry_free_ref(dat_entry);
	return ret;
}
