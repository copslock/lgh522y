/* Copyright (C) 2007-2013 B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
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

#include "main.h"
#include "routing.h"
#include "send.h"
#include "soft-interface.h"
#include "hard-interface.h"
#include "icmp_socket.h"
#include "translation-table.h"
#include "originator.h"
#include "vis.h"
#include "unicast.h"
#include "bridge_loop_avoidance.h"
#include "distributed-arp-table.h"
#include "network-coding.h"

static int batadv_route_unicast_packet(struct sk_buff *skb,
				       struct batadv_hard_iface *recv_if);

void batadv_slide_own_bcast_window(struct batadv_hard_iface *hard_iface)
{
	struct batadv_priv *bat_priv = netdev_priv(hard_iface->soft_iface);
	struct batadv_hashtable *hash = bat_priv->orig_hash;
	struct hlist_head *head;
	struct batadv_orig_node *orig_node;
	unsigned long *word;
	uint32_t i;
	size_t word_index;
	uint8_t *w;

	for (i = 0; i < hash->size; i++) {
		head = &hash->table[i];

		rcu_read_lock();
		hlist_for_each_entry_rcu(orig_node, head, hash_entry) {
			spin_lock_bh(&orig_node->ogm_cnt_lock);
			word_index = hard_iface->if_num * BATADV_NUM_WORDS;
			word = &(orig_node->bcast_own[word_index]);

			batadv_bit_get_packet(bat_priv, word, 1, 0);
			w = &orig_node->bcast_own_sum[hard_iface->if_num];
			*w = bitmap_weight(word, BATADV_TQ_LOCAL_WINDOW_SIZE);
			spin_unlock_bh(&orig_node->ogm_cnt_lock);
		}
		rcu_read_unlock();
	}
}

static void _batadv_update_route(struct batadv_priv *bat_priv,
				 struct batadv_orig_node *orig_node,
				 struct batadv_neigh_node *neigh_node)
{
	struct batadv_neigh_node *curr_router;

	curr_router = batadv_orig_node_get_router(orig_node);

	/*               */
	if ((curr_router) && (!neigh_node)) {
		batadv_dbg(BATADV_DBG_ROUTES, bat_priv,
			   "Deleting route towards: %pM\n", orig_node->orig);
		batadv_tt_global_del_orig(bat_priv, orig_node,
					  "Deleted route towards originator");

	/*             */
	} else if ((!curr_router) && (neigh_node)) {
		batadv_dbg(BATADV_DBG_ROUTES, bat_priv,
			   "Adding route towards: %pM (via %pM)\n",
			   orig_node->orig, neigh_node->addr);
	/*               */
	} else if (neigh_node && curr_router) {
		batadv_dbg(BATADV_DBG_ROUTES, bat_priv,
			   "Changing route towards: %pM (now via %pM - was via %pM)\n",
			   orig_node->orig, neigh_node->addr,
			   curr_router->addr);
	}

	if (curr_router)
		batadv_neigh_node_free_ref(curr_router);

	/*                                        */
	if (neigh_node && !atomic_inc_not_zero(&neigh_node->refcount))
		neigh_node = NULL;

	spin_lock_bh(&orig_node->neigh_list_lock);
	rcu_assign_pointer(orig_node->router, neigh_node);
	spin_unlock_bh(&orig_node->neigh_list_lock);

	/*                                             */
	if (curr_router)
		batadv_neigh_node_free_ref(curr_router);
}

void batadv_update_route(struct batadv_priv *bat_priv,
			 struct batadv_orig_node *orig_node,
			 struct batadv_neigh_node *neigh_node)
{
	struct batadv_neigh_node *router = NULL;

	if (!orig_node)
		goto out;

	router = batadv_orig_node_get_router(orig_node);

	if (router != neigh_node)
		_batadv_update_route(bat_priv, orig_node, neigh_node);

out:
	if (router)
		batadv_neigh_node_free_ref(router);
}

/*                                      */
void batadv_bonding_candidate_del(struct batadv_orig_node *orig_node,
				  struct batadv_neigh_node *neigh_node)
{
	/*                                                 */
	if (list_empty(&neigh_node->bonding_list))
		goto out;

	list_del_rcu(&neigh_node->bonding_list);
	INIT_LIST_HEAD(&neigh_node->bonding_list);
	batadv_neigh_node_free_ref(neigh_node);
	atomic_dec(&orig_node->bond_candidates);

out:
	return;
}

void batadv_bonding_candidate_add(struct batadv_orig_node *orig_node,
				  struct batadv_neigh_node *neigh_node)
{
	struct batadv_neigh_node *tmp_neigh_node, *router = NULL;
	uint8_t interference_candidate = 0;

	spin_lock_bh(&orig_node->neigh_list_lock);

	/*                                                       */
	if (!batadv_compare_eth(orig_node->orig,
				neigh_node->orig_node->primary_addr))
		goto candidate_del;

	router = batadv_orig_node_get_router(orig_node);
	if (!router)
		goto candidate_del;

	/*                                         */
	if (neigh_node->tq_avg < router->tq_avg - BATADV_BONDING_TQ_THRESHOLD)
		goto candidate_del;

	/*                                                                
                                                                  
                          
  */
	hlist_for_each_entry_rcu(tmp_neigh_node,
				 &orig_node->neigh_list, list) {
		if (tmp_neigh_node == neigh_node)
			continue;

		/*                                            
                             
   */
		if (list_empty(&tmp_neigh_node->bonding_list))
			continue;

		if ((neigh_node->if_incoming == tmp_neigh_node->if_incoming) ||
		    (batadv_compare_eth(neigh_node->addr,
					tmp_neigh_node->addr))) {
			interference_candidate = 1;
			break;
		}
	}

	/*                                                       */
	if (interference_candidate)
		goto candidate_del;

	/*                                                     */
	if (!list_empty(&neigh_node->bonding_list))
		goto out;

	if (!atomic_inc_not_zero(&neigh_node->refcount))
		goto out;

	list_add_rcu(&neigh_node->bonding_list, &orig_node->bond_list);
	atomic_inc(&orig_node->bond_candidates);
	goto out;

candidate_del:
	batadv_bonding_candidate_del(orig_node, neigh_node);

out:
	spin_unlock_bh(&orig_node->neigh_list_lock);

	if (router)
		batadv_neigh_node_free_ref(router);
}

/*                                  */
void
batadv_bonding_save_primary(const struct batadv_orig_node *orig_node,
			    struct batadv_orig_node *orig_neigh_node,
			    const struct batadv_ogm_packet *batman_ogm_packet)
{
	if (!(batman_ogm_packet->flags & BATADV_PRIMARIES_FIRST_HOP))
		return;

	memcpy(orig_neigh_node->primary_addr, orig_node->orig, ETH_ALEN);
}

/*                                                                 
           
                                     
                                     
 */
int batadv_window_protected(struct batadv_priv *bat_priv, int32_t seq_num_diff,
			    unsigned long *last_reset)
{
	if (seq_num_diff <= -BATADV_TQ_LOCAL_WINDOW_SIZE ||
	    seq_num_diff >= BATADV_EXPECTED_SEQNO_RANGE) {
		if (!batadv_has_timed_out(*last_reset,
					  BATADV_RESET_PROTECTION_MS))
			return 1;

		*last_reset = jiffies;
		batadv_dbg(BATADV_DBG_BATMAN, bat_priv,
			   "old packet received, start protection\n");
	}

	return 0;
}

bool batadv_check_management_packet(struct sk_buff *skb,
				    struct batadv_hard_iface *hard_iface,
				    int header_len)
{
	struct ethhdr *ethhdr;

	/*                                                  */
	if (unlikely(!pskb_may_pull(skb, header_len)))
		return false;

	ethhdr = (struct ethhdr *)skb_mac_header(skb);

	/*                                                        */
	if (!is_broadcast_ether_addr(ethhdr->h_dest))
		return false;

	/*                                      */
	if (is_broadcast_ether_addr(ethhdr->h_source))
		return false;

	/*                                                    */
	if (skb_cow(skb, 0) < 0)
		return false;

	/*                 */
	if (skb_linearize(skb) < 0)
		return false;

	return true;
}

static int batadv_recv_my_icmp_packet(struct batadv_priv *bat_priv,
				      struct sk_buff *skb, size_t icmp_len)
{
	struct batadv_hard_iface *primary_if = NULL;
	struct batadv_orig_node *orig_node = NULL;
	struct batadv_icmp_packet_rr *icmp_packet;
	int ret = NET_RX_DROP;

	icmp_packet = (struct batadv_icmp_packet_rr *)skb->data;

	/*                          */
	if (icmp_packet->msg_type != BATADV_ECHO_REQUEST) {
		batadv_socket_receive_packet(icmp_packet, icmp_len);
		goto out;
	}

	primary_if = batadv_primary_if_get_selected(bat_priv);
	if (!primary_if)
		goto out;

	/*                            */
	/*                         */
	orig_node = batadv_orig_hash_find(bat_priv, icmp_packet->orig);
	if (!orig_node)
		goto out;

	/*                                                    */
	if (skb_cow(skb, ETH_HLEN) < 0)
		goto out;

	icmp_packet = (struct batadv_icmp_packet_rr *)skb->data;

	memcpy(icmp_packet->dst, icmp_packet->orig, ETH_ALEN);
	memcpy(icmp_packet->orig, primary_if->net_dev->dev_addr, ETH_ALEN);
	icmp_packet->msg_type = BATADV_ECHO_REPLY;
	icmp_packet->header.ttl = BATADV_TTL;

	if (batadv_send_skb_to_orig(skb, orig_node, NULL))
		ret = NET_RX_SUCCESS;

out:
	if (primary_if)
		batadv_hardif_free_ref(primary_if);
	if (orig_node)
		batadv_orig_node_free_ref(orig_node);
	return ret;
}

static int batadv_recv_icmp_ttl_exceeded(struct batadv_priv *bat_priv,
					 struct sk_buff *skb)
{
	struct batadv_hard_iface *primary_if = NULL;
	struct batadv_orig_node *orig_node = NULL;
	struct batadv_icmp_packet *icmp_packet;
	int ret = NET_RX_DROP;

	icmp_packet = (struct batadv_icmp_packet *)skb->data;

	/*                                                             */
	if (icmp_packet->msg_type != BATADV_ECHO_REQUEST) {
		pr_debug("Warning - can't forward icmp packet from %pM to %pM: ttl exceeded\n",
			 icmp_packet->orig, icmp_packet->dst);
		goto out;
	}

	primary_if = batadv_primary_if_get_selected(bat_priv);
	if (!primary_if)
		goto out;

	/*                         */
	orig_node = batadv_orig_hash_find(bat_priv, icmp_packet->orig);
	if (!orig_node)
		goto out;

	/*                                                    */
	if (skb_cow(skb, ETH_HLEN) < 0)
		goto out;

	icmp_packet = (struct batadv_icmp_packet *)skb->data;

	memcpy(icmp_packet->dst, icmp_packet->orig, ETH_ALEN);
	memcpy(icmp_packet->orig, primary_if->net_dev->dev_addr, ETH_ALEN);
	icmp_packet->msg_type = BATADV_TTL_EXCEEDED;
	icmp_packet->header.ttl = BATADV_TTL;

	if (batadv_send_skb_to_orig(skb, orig_node, NULL))
		ret = NET_RX_SUCCESS;

out:
	if (primary_if)
		batadv_hardif_free_ref(primary_if);
	if (orig_node)
		batadv_orig_node_free_ref(orig_node);
	return ret;
}


int batadv_recv_icmp_packet(struct sk_buff *skb,
			    struct batadv_hard_iface *recv_if)
{
	struct batadv_priv *bat_priv = netdev_priv(recv_if->soft_iface);
	struct batadv_icmp_packet_rr *icmp_packet;
	struct ethhdr *ethhdr;
	struct batadv_orig_node *orig_node = NULL;
	int hdr_size = sizeof(struct batadv_icmp_packet);
	int ret = NET_RX_DROP;

	/*                                                                    */
	if (skb->len >= sizeof(struct batadv_icmp_packet_rr))
		hdr_size = sizeof(struct batadv_icmp_packet_rr);

	/*                                                  */
	if (unlikely(!pskb_may_pull(skb, hdr_size)))
		goto out;

	ethhdr = (struct ethhdr *)skb_mac_header(skb);

	/*                                                        */
	if (is_broadcast_ether_addr(ethhdr->h_dest))
		goto out;

	/*                                      */
	if (is_broadcast_ether_addr(ethhdr->h_source))
		goto out;

	/*            */
	if (!batadv_is_my_mac(bat_priv, ethhdr->h_dest))
		goto out;

	icmp_packet = (struct batadv_icmp_packet_rr *)skb->data;

	/*                                          */
	if ((hdr_size == sizeof(struct batadv_icmp_packet_rr)) &&
	    (icmp_packet->rr_cur < BATADV_RR_LEN)) {
		memcpy(&(icmp_packet->rr[icmp_packet->rr_cur]),
		       ethhdr->h_dest, ETH_ALEN);
		icmp_packet->rr_cur++;
	}

	/*               */
	if (batadv_is_my_mac(bat_priv, icmp_packet->dst))
		return batadv_recv_my_icmp_packet(bat_priv, skb, hdr_size);

	/*              */
	if (icmp_packet->header.ttl < 2)
		return batadv_recv_icmp_ttl_exceeded(bat_priv, skb);

	/*                         */
	orig_node = batadv_orig_hash_find(bat_priv, icmp_packet->dst);
	if (!orig_node)
		goto out;

	/*                                                    */
	if (skb_cow(skb, ETH_HLEN) < 0)
		goto out;

	icmp_packet = (struct batadv_icmp_packet_rr *)skb->data;

	/*               */
	icmp_packet->header.ttl--;

	/*          */
	if (batadv_send_skb_to_orig(skb, orig_node, recv_if))
		ret = NET_RX_SUCCESS;

out:
	if (orig_node)
		batadv_orig_node_free_ref(orig_node);
	return ret;
}

/*                                                 
                                               
  
                                                         
                              
 */
static struct batadv_neigh_node *
batadv_find_bond_router(struct batadv_orig_node *primary_orig,
			const struct batadv_hard_iface *recv_if)
{
	struct batadv_neigh_node *tmp_neigh_node;
	struct batadv_neigh_node *router = NULL, *first_candidate = NULL;

	rcu_read_lock();
	list_for_each_entry_rcu(tmp_neigh_node, &primary_orig->bond_list,
				bonding_list) {
		if (!first_candidate)
			first_candidate = tmp_neigh_node;

		/*                                    */
		if (tmp_neigh_node->if_incoming == recv_if)
			continue;

		if (!atomic_inc_not_zero(&tmp_neigh_node->refcount))
			continue;

		router = tmp_neigh_node;
		break;
	}

	/*                                               */
	if (!router && first_candidate &&
	    atomic_inc_not_zero(&first_candidate->refcount))
		router = first_candidate;

	if (!router)
		goto out;

	/*                                          
                            
  */
	spin_lock_bh(&primary_orig->neigh_list_lock);
	/*                                           
                                 
  */
	list_del_rcu(&primary_orig->bond_list);
	list_add_rcu(&primary_orig->bond_list,
		     &router->bonding_list);
	spin_unlock_bh(&primary_orig->neigh_list_lock);

out:
	rcu_read_unlock();
	return router;
}

/*                                           
                                           
                  
  
                                           
 */
static struct batadv_neigh_node *
batadv_find_ifalter_router(struct batadv_orig_node *primary_orig,
			   const struct batadv_hard_iface *recv_if)
{
	struct batadv_neigh_node *tmp_neigh_node;
	struct batadv_neigh_node *router = NULL, *first_candidate = NULL;

	rcu_read_lock();
	list_for_each_entry_rcu(tmp_neigh_node, &primary_orig->bond_list,
				bonding_list) {
		if (!first_candidate)
			first_candidate = tmp_neigh_node;

		/*                                    */
		if (tmp_neigh_node->if_incoming == recv_if)
			continue;

		if (router && tmp_neigh_node->tq_avg <= router->tq_avg)
			continue;

		if (!atomic_inc_not_zero(&tmp_neigh_node->refcount))
			continue;

		/*                                                  */
		if (router)
			batadv_neigh_node_free_ref(router);

		/*                                                         */
		router = tmp_neigh_node;
	}

	/*                                               */
	if (!router && first_candidate &&
	    atomic_inc_not_zero(&first_candidate->refcount))
		router = first_candidate;

	rcu_read_unlock();
	return router;
}

/* 
                                                                    
                                                                  
                        
                                    
  
                                                                             
                                                                            
                                                                               
                                                       
 */
static int batadv_check_unicast_packet(struct batadv_priv *bat_priv,
				       struct sk_buff *skb, int hdr_size)
{
	struct ethhdr *ethhdr;

	/*                                                  */
	if (unlikely(!pskb_may_pull(skb, hdr_size)))
		return -ENODATA;

	ethhdr = (struct ethhdr *)skb_mac_header(skb);

	/*                                                        */
	if (is_broadcast_ether_addr(ethhdr->h_dest))
		return -EBADR;

	/*                                      */
	if (is_broadcast_ether_addr(ethhdr->h_source))
		return -EBADR;

	/*            */
	if (!batadv_is_my_mac(bat_priv, ethhdr->h_dest))
		return -EREMOTE;

	return 0;
}

int batadv_recv_tt_query(struct sk_buff *skb, struct batadv_hard_iface *recv_if)
{
	struct batadv_priv *bat_priv = netdev_priv(recv_if->soft_iface);
	struct batadv_tt_query_packet *tt_query;
	uint16_t tt_size;
	int hdr_size = sizeof(*tt_query);
	char tt_flag;
	size_t packet_size;

	if (batadv_check_unicast_packet(bat_priv, skb, hdr_size) < 0)
		return NET_RX_DROP;

	/*                           */
	if (skb_cow(skb, sizeof(struct batadv_tt_query_packet)) < 0)
		goto out;

	tt_query = (struct batadv_tt_query_packet *)skb->data;

	switch (tt_query->flags & BATADV_TT_QUERY_TYPE_MASK) {
	case BATADV_TT_REQUEST:
		batadv_inc_counter(bat_priv, BATADV_CNT_TT_REQUEST_RX);

		/*                                                 
              
   */
		if (!batadv_send_tt_response(bat_priv, tt_query)) {
			if (tt_query->flags & BATADV_TT_FULL_TABLE)
				tt_flag = 'F';
			else
				tt_flag = '.';

			batadv_dbg(BATADV_DBG_TT, bat_priv,
				   "Routing TT_REQUEST to %pM [%c]\n",
				   tt_query->dst,
				   tt_flag);
			return batadv_route_unicast_packet(skb, recv_if);
		}
		break;
	case BATADV_TT_RESPONSE:
		batadv_inc_counter(bat_priv, BATADV_CNT_TT_RESPONSE_RX);

		if (batadv_is_my_mac(bat_priv, tt_query->dst)) {
			/*                                               
             
    */
			if (skb_linearize(skb) < 0)
				goto out;
			/*                                            */
			tt_query = (struct batadv_tt_query_packet *)skb->data;

			tt_size = batadv_tt_len(ntohs(tt_query->tt_data));

			/*                                     */
			packet_size = sizeof(struct batadv_tt_query_packet);
			packet_size += tt_size;
			if (unlikely(skb_headlen(skb) < packet_size))
				goto out;

			batadv_handle_tt_response(bat_priv, tt_query);
		} else {
			if (tt_query->flags & BATADV_TT_FULL_TABLE)
				tt_flag =  'F';
			else
				tt_flag = '.';
			batadv_dbg(BATADV_DBG_TT, bat_priv,
				   "Routing TT_RESPONSE to %pM [%c]\n",
				   tt_query->dst,
				   tt_flag);
			return batadv_route_unicast_packet(skb, recv_if);
		}
		break;
	}

out:
	/*                                                                   */
	return NET_RX_DROP;
}

int batadv_recv_roam_adv(struct sk_buff *skb, struct batadv_hard_iface *recv_if)
{
	struct batadv_priv *bat_priv = netdev_priv(recv_if->soft_iface);
	struct batadv_roam_adv_packet *roam_adv_packet;
	struct batadv_orig_node *orig_node;

	if (batadv_check_unicast_packet(bat_priv, skb,
					sizeof(*roam_adv_packet)) < 0)
		goto out;

	batadv_inc_counter(bat_priv, BATADV_CNT_TT_ROAM_ADV_RX);

	roam_adv_packet = (struct batadv_roam_adv_packet *)skb->data;

	if (!batadv_is_my_mac(bat_priv, roam_adv_packet->dst))
		return batadv_route_unicast_packet(skb, recv_if);

	/*                                                   
                                                     
                       
  */
	if (batadv_bla_is_backbone_gw_orig(bat_priv, roam_adv_packet->src))
		goto out;

	orig_node = batadv_orig_hash_find(bat_priv, roam_adv_packet->src);
	if (!orig_node)
		goto out;

	batadv_dbg(BATADV_DBG_TT, bat_priv,
		   "Received ROAMING_ADV from %pM (client %pM)\n",
		   roam_adv_packet->src, roam_adv_packet->client);

	batadv_tt_global_add(bat_priv, orig_node, roam_adv_packet->client,
			     BATADV_TT_CLIENT_ROAM,
			     atomic_read(&orig_node->last_ttvn) + 1);

	batadv_orig_node_free_ref(orig_node);
out:
	/*                                                                   */
	return NET_RX_DROP;
}

/*                                                    
                                                     
            
 */
struct batadv_neigh_node *
batadv_find_router(struct batadv_priv *bat_priv,
		   struct batadv_orig_node *orig_node,
		   const struct batadv_hard_iface *recv_if)
{
	struct batadv_orig_node *primary_orig_node;
	struct batadv_orig_node *router_orig;
	struct batadv_neigh_node *router;
	static uint8_t zero_mac[ETH_ALEN] = {0, 0, 0, 0, 0, 0};
	int bonding_enabled;
	uint8_t *primary_addr;

	if (!orig_node)
		return NULL;

	router = batadv_orig_node_get_router(orig_node);
	if (!router)
		goto err;

	/*                                       
                                     
  */
	bonding_enabled = atomic_read(&bat_priv->bonding);

	rcu_read_lock();
	/*                                 */
	router_orig = router->orig_node;
	if (!router_orig)
		goto err_unlock;

	if ((!recv_if) && (!bonding_enabled))
		goto return_router;

	primary_addr = router_orig->primary_addr;

	/*                                                        
                                      
  */
	if (batadv_compare_eth(primary_addr, zero_mac))
		goto return_router;

	/*                                                          
                                                     
  */
	if (batadv_compare_eth(primary_addr, router_orig->orig)) {
		primary_orig_node = router_orig;
	} else {
		primary_orig_node = batadv_orig_hash_find(bat_priv,
							  primary_addr);
		if (!primary_orig_node)
			goto return_router;

		batadv_orig_node_free_ref(primary_orig_node);
	}

	/*                                             
                                           
  */
	if (atomic_read(&primary_orig_node->bond_candidates) < 2)
		goto return_router;

	/*                                                  
                                                    
       
  */
	batadv_neigh_node_free_ref(router);

	if (bonding_enabled)
		router = batadv_find_bond_router(primary_orig_node, recv_if);
	else
		router = batadv_find_ifalter_router(primary_orig_node, recv_if);

return_router:
	if (router && router->if_incoming->if_status != BATADV_IF_ACTIVE)
		goto err_unlock;

	rcu_read_unlock();
	return router;
err_unlock:
	rcu_read_unlock();
err:
	if (router)
		batadv_neigh_node_free_ref(router);
	return NULL;
}

static int batadv_route_unicast_packet(struct sk_buff *skb,
				       struct batadv_hard_iface *recv_if)
{
	struct batadv_priv *bat_priv = netdev_priv(recv_if->soft_iface);
	struct batadv_orig_node *orig_node = NULL;
	struct batadv_neigh_node *neigh_node = NULL;
	struct batadv_unicast_packet *unicast_packet;
	struct ethhdr *ethhdr = (struct ethhdr *)skb_mac_header(skb);
	int ret = NET_RX_DROP;
	struct sk_buff *new_skb;

	unicast_packet = (struct batadv_unicast_packet *)skb->data;

	/*              */
	if (unicast_packet->header.ttl < 2) {
		pr_debug("Warning - can't forward unicast packet from %pM to %pM: ttl exceeded\n",
			 ethhdr->h_source, unicast_packet->dest);
		goto out;
	}

	/*                         */
	orig_node = batadv_orig_hash_find(bat_priv, unicast_packet->dest);

	if (!orig_node)
		goto out;

	/*                                                        */
	neigh_node = batadv_find_router(bat_priv, orig_node, recv_if);

	if (!neigh_node)
		goto out;

	/*                                                    */
	if (skb_cow(skb, ETH_HLEN) < 0)
		goto out;

	unicast_packet = (struct batadv_unicast_packet *)skb->data;

	if (unicast_packet->header.packet_type == BATADV_UNICAST &&
	    atomic_read(&bat_priv->fragmentation) &&
	    skb->len > neigh_node->if_incoming->net_dev->mtu) {
		ret = batadv_frag_send_skb(skb, bat_priv,
					   neigh_node->if_incoming,
					   neigh_node->addr);
		goto out;
	}

	if (unicast_packet->header.packet_type == BATADV_UNICAST_FRAG &&
	    batadv_frag_can_reassemble(skb,
				       neigh_node->if_incoming->net_dev->mtu)) {
		ret = batadv_frag_reassemble_skb(skb, bat_priv, &new_skb);

		if (ret == NET_RX_DROP)
			goto out;

		/*                                    */
		if (!new_skb) {
			ret = NET_RX_SUCCESS;
			goto out;
		}

		skb = new_skb;
		unicast_packet = (struct batadv_unicast_packet *)skb->data;
	}

	/*               */
	unicast_packet->header.ttl--;

	/*                                 */
	if (batadv_nc_skb_forward(skb, neigh_node, ethhdr)) {
		ret = NET_RX_SUCCESS;
	} else if (batadv_send_skb_to_orig(skb, orig_node, recv_if)) {
		ret = NET_RX_SUCCESS;

		/*                      */
		batadv_inc_counter(bat_priv, BATADV_CNT_FORWARD);
		batadv_add_counter(bat_priv, BATADV_CNT_FORWARD_BYTES,
				   skb->len + ETH_HLEN);
	}

out:
	if (neigh_node)
		batadv_neigh_node_free_ref(neigh_node);
	if (orig_node)
		batadv_orig_node_free_ref(orig_node);
	return ret;
}

/* 
                                                                           
                                                                  
                                                    
                                     
  
                                                                               
                                                                              
                                          
  
                                                                      
 */
static bool
batadv_reroute_unicast_packet(struct batadv_priv *bat_priv,
			      struct batadv_unicast_packet *unicast_packet,
			      uint8_t *dst_addr)
{
	struct batadv_orig_node *orig_node = NULL;
	struct batadv_hard_iface *primary_if = NULL;
	bool ret = false;
	uint8_t *orig_addr, orig_ttvn;

	if (batadv_is_my_client(bat_priv, dst_addr)) {
		primary_if = batadv_primary_if_get_selected(bat_priv);
		if (!primary_if)
			goto out;
		orig_addr = primary_if->net_dev->dev_addr;
		orig_ttvn = (uint8_t)atomic_read(&bat_priv->tt.vn);
	} else {
		orig_node = batadv_transtable_search(bat_priv, NULL, dst_addr);
		if (!orig_node)
			goto out;

		if (batadv_compare_eth(orig_node->orig, unicast_packet->dest))
			goto out;

		orig_addr = orig_node->orig;
		orig_ttvn = (uint8_t)atomic_read(&orig_node->last_ttvn);
	}

	/*                          */
	memcpy(unicast_packet->dest, orig_addr, ETH_ALEN);
	unicast_packet->ttvn = orig_ttvn;

	ret = true;
out:
	if (primary_if)
		batadv_hardif_free_ref(primary_if);
	if (orig_node)
		batadv_orig_node_free_ref(orig_node);

	return ret;
}

static int batadv_check_unicast_ttvn(struct batadv_priv *bat_priv,
				     struct sk_buff *skb, int hdr_len) {
	uint8_t curr_ttvn, old_ttvn;
	struct batadv_orig_node *orig_node;
	struct ethhdr *ethhdr;
	struct batadv_hard_iface *primary_if;
	struct batadv_unicast_packet *unicast_packet;
	int is_old_ttvn;

	/*                                                   */
	if (pskb_may_pull(skb, hdr_len + ETH_HLEN) < 0)
		return 0;

	/*                                                                    */
	if (skb_cow(skb, sizeof(*unicast_packet)) < 0)
		return 0;

	unicast_packet = (struct batadv_unicast_packet *)skb->data;
	ethhdr = (struct ethhdr *)(skb->data + hdr_len);

	/*                                                                      
                                                                    
                                                                         
                 
  */
	if (batadv_tt_local_client_is_roaming(bat_priv, ethhdr->h_dest)) {
		if (batadv_reroute_unicast_packet(bat_priv, unicast_packet,
						  ethhdr->h_dest))
			net_ratelimited_function(batadv_dbg, BATADV_DBG_TT,
						 bat_priv,
						 "Rerouting unicast packet to %pM (dst=%pM): Local Roaming\n",
						 unicast_packet->dest,
						 ethhdr->h_dest);
		/*                                                    
                                                                
                                                              
                                     
   */
		return 1;
	}

	/*                                                                      
                                                                     
                                                           
  */
	curr_ttvn = (uint8_t)atomic_read(&bat_priv->tt.vn);
	if (!batadv_is_my_mac(bat_priv, unicast_packet->dest)) {
		orig_node = batadv_orig_hash_find(bat_priv,
						  unicast_packet->dest);
		/*                                                             
                                                                  
                                  
   */
		if (!orig_node)
			return 0;

		curr_ttvn = (uint8_t)atomic_read(&orig_node->last_ttvn);
		batadv_orig_node_free_ref(orig_node);
	}

	/*                                                                   
              
  */
	is_old_ttvn = batadv_seq_before(unicast_packet->ttvn, curr_ttvn);
	if (!is_old_ttvn)
		return 1;

	old_ttvn = unicast_packet->ttvn;
	/*                                                                 
                                                                     
               
  */
	if (batadv_reroute_unicast_packet(bat_priv, unicast_packet,
					  ethhdr->h_dest)) {
		net_ratelimited_function(batadv_dbg, BATADV_DBG_TT, bat_priv,
					 "Rerouting unicast packet to %pM (dst=%pM): TTVN mismatch old_ttvn=%u new_ttvn=%u\n",
					 unicast_packet->dest, ethhdr->h_dest,
					 old_ttvn, curr_ttvn);
		return 1;
	}

	/*                                                             
                                                                       
                                     
  */
	if (!batadv_is_my_client(bat_priv, ethhdr->h_dest))
		return 0;

	/*                                                                  
                         
  */
	primary_if = batadv_primary_if_get_selected(bat_priv);
	if (!primary_if)
		return 0;

	memcpy(unicast_packet->dest, primary_if->net_dev->dev_addr, ETH_ALEN);

	batadv_hardif_free_ref(primary_if);

	unicast_packet->ttvn = curr_ttvn;

	return 1;
}

int batadv_recv_unicast_packet(struct sk_buff *skb,
			       struct batadv_hard_iface *recv_if)
{
	struct batadv_priv *bat_priv = netdev_priv(recv_if->soft_iface);
	struct batadv_unicast_packet *unicast_packet;
	struct batadv_unicast_4addr_packet *unicast_4addr_packet;
	uint8_t *orig_addr;
	struct batadv_orig_node *orig_node = NULL;
	int check, hdr_size = sizeof(*unicast_packet);
	bool is4addr;

	unicast_packet = (struct batadv_unicast_packet *)skb->data;
	unicast_4addr_packet = (struct batadv_unicast_4addr_packet *)skb->data;

	is4addr = unicast_packet->header.packet_type == BATADV_UNICAST_4ADDR;
	/*                                                        */
	if (is4addr)
		hdr_size = sizeof(*unicast_4addr_packet);

	/*                                                   */
	check = batadv_check_unicast_packet(bat_priv, skb, hdr_size);

	/*                                                                  
                                          
  */
	if (check == -EREMOTE)
		batadv_nc_skb_store_sniffed_unicast(bat_priv, skb);

	if (check < 0)
		return NET_RX_DROP;
	if (!batadv_check_unicast_ttvn(bat_priv, skb, hdr_size))
		return NET_RX_DROP;

	/*               */
	if (batadv_is_my_mac(bat_priv, unicast_packet->dest)) {
		if (is4addr) {
			batadv_dat_inc_counter(bat_priv,
					       unicast_4addr_packet->subtype);
			orig_addr = unicast_4addr_packet->src;
			orig_node = batadv_orig_hash_find(bat_priv, orig_addr);
		}

		if (batadv_dat_snoop_incoming_arp_request(bat_priv, skb,
							  hdr_size))
			goto rx_success;
		if (batadv_dat_snoop_incoming_arp_reply(bat_priv, skb,
							hdr_size))
			goto rx_success;

		batadv_interface_rx(recv_if->soft_iface, skb, recv_if, hdr_size,
				    orig_node);

rx_success:
		if (orig_node)
			batadv_orig_node_free_ref(orig_node);

		return NET_RX_SUCCESS;
	}

	return batadv_route_unicast_packet(skb, recv_if);
}

int batadv_recv_ucast_frag_packet(struct sk_buff *skb,
				  struct batadv_hard_iface *recv_if)
{
	struct batadv_priv *bat_priv = netdev_priv(recv_if->soft_iface);
	struct batadv_unicast_frag_packet *unicast_packet;
	int hdr_size = sizeof(*unicast_packet);
	struct sk_buff *new_skb = NULL;
	int ret;

	if (batadv_check_unicast_packet(bat_priv, skb, hdr_size) < 0)
		return NET_RX_DROP;

	if (!batadv_check_unicast_ttvn(bat_priv, skb, hdr_size))
		return NET_RX_DROP;

	unicast_packet = (struct batadv_unicast_frag_packet *)skb->data;

	/*               */
	if (batadv_is_my_mac(bat_priv, unicast_packet->dest)) {
		ret = batadv_frag_reassemble_skb(skb, bat_priv, &new_skb);

		if (ret == NET_RX_DROP)
			return NET_RX_DROP;

		/*                                    */
		if (!new_skb)
			return NET_RX_SUCCESS;

		if (batadv_dat_snoop_incoming_arp_request(bat_priv, new_skb,
							  hdr_size))
			goto rx_success;
		if (batadv_dat_snoop_incoming_arp_reply(bat_priv, new_skb,
							hdr_size))
			goto rx_success;

		batadv_interface_rx(recv_if->soft_iface, new_skb, recv_if,
				    sizeof(struct batadv_unicast_packet), NULL);

rx_success:
		return NET_RX_SUCCESS;
	}

	return batadv_route_unicast_packet(skb, recv_if);
}


int batadv_recv_bcast_packet(struct sk_buff *skb,
			     struct batadv_hard_iface *recv_if)
{
	struct batadv_priv *bat_priv = netdev_priv(recv_if->soft_iface);
	struct batadv_orig_node *orig_node = NULL;
	struct batadv_bcast_packet *bcast_packet;
	struct ethhdr *ethhdr;
	int hdr_size = sizeof(*bcast_packet);
	int ret = NET_RX_DROP;
	int32_t seq_diff;

	/*                                                  */
	if (unlikely(!pskb_may_pull(skb, hdr_size)))
		goto out;

	ethhdr = (struct ethhdr *)skb_mac_header(skb);

	/*                                                        */
	if (!is_broadcast_ether_addr(ethhdr->h_dest))
		goto out;

	/*                                      */
	if (is_broadcast_ether_addr(ethhdr->h_source))
		goto out;

	/*                                  */
	if (batadv_is_my_mac(bat_priv, ethhdr->h_source))
		goto out;

	bcast_packet = (struct batadv_bcast_packet *)skb->data;

	/*                                        */
	if (batadv_is_my_mac(bat_priv, bcast_packet->orig))
		goto out;

	if (bcast_packet->header.ttl < 2)
		goto out;

	orig_node = batadv_orig_hash_find(bat_priv, bcast_packet->orig);

	if (!orig_node)
		goto out;

	spin_lock_bh(&orig_node->bcast_seqno_lock);

	/*                                         */
	if (batadv_test_bit(orig_node->bcast_bits, orig_node->last_bcast_seqno,
			    ntohl(bcast_packet->seqno)))
		goto spin_unlock;

	seq_diff = ntohl(bcast_packet->seqno) - orig_node->last_bcast_seqno;

	/*                                                              */
	if (batadv_window_protected(bat_priv, seq_diff,
				    &orig_node->bcast_seqno_reset))
		goto spin_unlock;

	/*                                                        
                
  */
	if (batadv_bit_get_packet(bat_priv, orig_node->bcast_bits, seq_diff, 1))
		orig_node->last_bcast_seqno = ntohl(bcast_packet->seqno);

	spin_unlock_bh(&orig_node->bcast_seqno_lock);

	/*                                                               */
	if (batadv_bla_check_bcast_duplist(bat_priv, skb))
		goto out;

	/*                    */
	batadv_add_bcast_packet_to_list(bat_priv, skb, 1);

	/*                                                        
                           
  */
	if (batadv_bla_is_backbone_gw(skb, orig_node, hdr_size))
		goto out;

	if (batadv_dat_snoop_incoming_arp_request(bat_priv, skb, hdr_size))
		goto rx_success;
	if (batadv_dat_snoop_incoming_arp_reply(bat_priv, skb, hdr_size))
		goto rx_success;

	/*                  */
	batadv_interface_rx(recv_if->soft_iface, skb, recv_if, hdr_size,
			    orig_node);

rx_success:
	ret = NET_RX_SUCCESS;
	goto out;

spin_unlock:
	spin_unlock_bh(&orig_node->bcast_seqno_lock);
out:
	if (orig_node)
		batadv_orig_node_free_ref(orig_node);
	return ret;
}

int batadv_recv_vis_packet(struct sk_buff *skb,
			   struct batadv_hard_iface *recv_if)
{
	struct batadv_vis_packet *vis_packet;
	struct ethhdr *ethhdr;
	struct batadv_priv *bat_priv = netdev_priv(recv_if->soft_iface);
	int hdr_size = sizeof(*vis_packet);

	/*                 */
	if (skb_linearize(skb) < 0)
		return NET_RX_DROP;

	if (unlikely(!pskb_may_pull(skb, hdr_size)))
		return NET_RX_DROP;

	vis_packet = (struct batadv_vis_packet *)skb->data;
	ethhdr = (struct ethhdr *)skb_mac_header(skb);

	/*            */
	if (!batadv_is_my_mac(bat_priv, ethhdr->h_dest))
		return NET_RX_DROP;

	/*                    */
	if (batadv_is_my_mac(bat_priv, vis_packet->vis_orig))
		return NET_RX_DROP;

	if (batadv_is_my_mac(bat_priv, vis_packet->sender_orig))
		return NET_RX_DROP;

	switch (vis_packet->vis_type) {
	case BATADV_VIS_TYPE_SERVER_SYNC:
		batadv_receive_server_sync_packet(bat_priv, vis_packet,
						  skb_headlen(skb));
		break;

	case BATADV_VIS_TYPE_CLIENT_UPDATE:
		batadv_receive_client_update_packet(bat_priv, vis_packet,
						    skb_headlen(skb));
		break;

	default:	/*                       */
		break;
	}

	/*                                                       
                          
  */
	return NET_RX_DROP;
}
