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
#include "distributed-arp-table.h"
#include "hard-interface.h"
#include "soft-interface.h"
#include "send.h"
#include "translation-table.h"
#include "routing.h"
#include "sysfs.h"
#include "originator.h"
#include "hash.h"
#include "bridge_loop_avoidance.h"

#include <linux/if_arp.h>
#include <linux/if_ether.h>

void batadv_hardif_free_rcu(struct rcu_head *rcu)
{
	struct batadv_hard_iface *hard_iface;

	hard_iface = container_of(rcu, struct batadv_hard_iface, rcu);
	dev_put(hard_iface->net_dev);
	kfree(hard_iface);
}

struct batadv_hard_iface *
batadv_hardif_get_by_netdev(const struct net_device *net_dev)
{
	struct batadv_hard_iface *hard_iface;

	rcu_read_lock();
	list_for_each_entry_rcu(hard_iface, &batadv_hardif_list, list) {
		if (hard_iface->net_dev == net_dev &&
		    atomic_inc_not_zero(&hard_iface->refcount))
			goto out;
	}

	hard_iface = NULL;

out:
	rcu_read_unlock();
	return hard_iface;
}

/* 
                                                                             
                                
  
                                                                              
                                                                             
                                                                             
                                                                           
                                                    
  
                                                                              
                                                           
 */
static bool batadv_is_on_batman_iface(const struct net_device *net_dev)
{
	struct net_device *parent_dev;
	bool ret;

	/*                                              */
	if (batadv_softif_is_valid(net_dev))
		return true;

	/*                                 */
	if (net_dev->iflink == net_dev->ifindex)
		return false;

	/*                                */
	parent_dev = dev_get_by_index(&init_net, net_dev->iflink);
	/*                                                         */
	if (WARN(!parent_dev, "Cannot find parent device"))
		return false;

	ret = batadv_is_on_batman_iface(parent_dev);

	if (parent_dev)
		dev_put(parent_dev);
	return ret;
}

static int batadv_is_valid_iface(const struct net_device *net_dev)
{
	if (net_dev->flags & IFF_LOOPBACK)
		return 0;

	if (net_dev->type != ARPHRD_ETHER)
		return 0;

	if (net_dev->addr_len != ETH_ALEN)
		return 0;

	/*                       */
	if (batadv_is_on_batman_iface(net_dev))
		return 0;

	return 1;
}

static struct batadv_hard_iface *
batadv_hardif_get_active(const struct net_device *soft_iface)
{
	struct batadv_hard_iface *hard_iface;

	rcu_read_lock();
	list_for_each_entry_rcu(hard_iface, &batadv_hardif_list, list) {
		if (hard_iface->soft_iface != soft_iface)
			continue;

		if (hard_iface->if_status == BATADV_IF_ACTIVE &&
		    atomic_inc_not_zero(&hard_iface->refcount))
			goto out;
	}

	hard_iface = NULL;

out:
	rcu_read_unlock();
	return hard_iface;
}

static void batadv_primary_if_update_addr(struct batadv_priv *bat_priv,
					  struct batadv_hard_iface *oldif)
{
	struct batadv_vis_packet *vis_packet;
	struct batadv_hard_iface *primary_if;
	struct sk_buff *skb;

	primary_if = batadv_primary_if_get_selected(bat_priv);
	if (!primary_if)
		goto out;

	batadv_dat_init_own_addr(bat_priv, primary_if);

	skb = bat_priv->vis.my_info->skb_packet;
	vis_packet = (struct batadv_vis_packet *)skb->data;
	memcpy(vis_packet->vis_orig, primary_if->net_dev->dev_addr, ETH_ALEN);
	memcpy(vis_packet->sender_orig,
	       primary_if->net_dev->dev_addr, ETH_ALEN);

	batadv_bla_update_orig_address(bat_priv, primary_if, oldif);
out:
	if (primary_if)
		batadv_hardif_free_ref(primary_if);
}

static void batadv_primary_if_select(struct batadv_priv *bat_priv,
				     struct batadv_hard_iface *new_hard_iface)
{
	struct batadv_hard_iface *curr_hard_iface;

	ASSERT_RTNL();

	if (new_hard_iface && !atomic_inc_not_zero(&new_hard_iface->refcount))
		new_hard_iface = NULL;

	curr_hard_iface = rcu_dereference_protected(bat_priv->primary_if, 1);
	rcu_assign_pointer(bat_priv->primary_if, new_hard_iface);

	if (!new_hard_iface)
		goto out;

	bat_priv->bat_algo_ops->bat_primary_iface_set(new_hard_iface);
	batadv_primary_if_update_addr(bat_priv, curr_hard_iface);

out:
	if (curr_hard_iface)
		batadv_hardif_free_ref(curr_hard_iface);
}

static bool
batadv_hardif_is_iface_up(const struct batadv_hard_iface *hard_iface)
{
	if (hard_iface->net_dev->flags & IFF_UP)
		return true;

	return false;
}

static void batadv_check_known_mac_addr(const struct net_device *net_dev)
{
	const struct batadv_hard_iface *hard_iface;

	rcu_read_lock();
	list_for_each_entry_rcu(hard_iface, &batadv_hardif_list, list) {
		if ((hard_iface->if_status != BATADV_IF_ACTIVE) &&
		    (hard_iface->if_status != BATADV_IF_TO_BE_ACTIVATED))
			continue;

		if (hard_iface->net_dev == net_dev)
			continue;

		if (!batadv_compare_eth(hard_iface->net_dev->dev_addr,
					net_dev->dev_addr))
			continue;

		pr_warn("The newly added mac address (%pM) already exists on: %s\n",
			net_dev->dev_addr, hard_iface->net_dev->name);
		pr_warn("It is strongly recommended to keep mac addresses unique to avoid problems!\n");
	}
	rcu_read_unlock();
}

int batadv_hardif_min_mtu(struct net_device *soft_iface)
{
	const struct batadv_priv *bat_priv = netdev_priv(soft_iface);
	const struct batadv_hard_iface *hard_iface;
	/*                                                     
                                      
  */
	int min_mtu = ETH_DATA_LEN;

	if (atomic_read(&bat_priv->fragmentation))
		goto out;

	rcu_read_lock();
	list_for_each_entry_rcu(hard_iface, &batadv_hardif_list, list) {
		if ((hard_iface->if_status != BATADV_IF_ACTIVE) &&
		    (hard_iface->if_status != BATADV_IF_TO_BE_ACTIVATED))
			continue;

		if (hard_iface->soft_iface != soft_iface)
			continue;

		min_mtu = min_t(int,
				hard_iface->net_dev->mtu - BATADV_HEADER_LEN,
				min_mtu);
	}
	rcu_read_unlock();
out:
	return min_mtu;
}

/*                                                                 */
void batadv_update_min_mtu(struct net_device *soft_iface)
{
	int min_mtu;

	min_mtu = batadv_hardif_min_mtu(soft_iface);
	if (soft_iface->mtu != min_mtu)
		soft_iface->mtu = min_mtu;
}

static void
batadv_hardif_activate_interface(struct batadv_hard_iface *hard_iface)
{
	struct batadv_priv *bat_priv;
	struct batadv_hard_iface *primary_if = NULL;

	if (hard_iface->if_status != BATADV_IF_INACTIVE)
		goto out;

	bat_priv = netdev_priv(hard_iface->soft_iface);

	bat_priv->bat_algo_ops->bat_iface_update_mac(hard_iface);
	hard_iface->if_status = BATADV_IF_TO_BE_ACTIVATED;

	/*                                                            
                                                                         
  */
	primary_if = batadv_primary_if_get_selected(bat_priv);
	if (!primary_if)
		batadv_primary_if_select(bat_priv, hard_iface);

	batadv_info(hard_iface->soft_iface, "Interface activated: %s\n",
		    hard_iface->net_dev->name);

	batadv_update_min_mtu(hard_iface->soft_iface);

out:
	if (primary_if)
		batadv_hardif_free_ref(primary_if);
}

static void
batadv_hardif_deactivate_interface(struct batadv_hard_iface *hard_iface)
{
	if ((hard_iface->if_status != BATADV_IF_ACTIVE) &&
	    (hard_iface->if_status != BATADV_IF_TO_BE_ACTIVATED))
		return;

	hard_iface->if_status = BATADV_IF_INACTIVE;

	batadv_info(hard_iface->soft_iface, "Interface deactivated: %s\n",
		    hard_iface->net_dev->name);

	batadv_update_min_mtu(hard_iface->soft_iface);
}

/* 
                                                                                
                                                   
                                                         
  
                                                                              
                                                                
                                                                         
 */
static int batadv_master_del_slave(struct batadv_hard_iface *slave,
				   struct net_device *master)
{
	int ret;

	if (!master)
		return 0;

	ret = -EBUSY;
	if (master->netdev_ops->ndo_del_slave)
		ret = master->netdev_ops->ndo_del_slave(master, slave->net_dev);

	return ret;
}

int batadv_hardif_enable_interface(struct batadv_hard_iface *hard_iface,
				   const char *iface_name)
{
	struct batadv_priv *bat_priv;
	struct net_device *soft_iface, *master;
	__be16 ethertype = __constant_htons(ETH_P_BATMAN);
	int ret;

	if (hard_iface->if_status != BATADV_IF_NOT_IN_USE)
		goto out;

	if (!atomic_inc_not_zero(&hard_iface->refcount))
		goto out;

	soft_iface = dev_get_by_name(&init_net, iface_name);

	if (!soft_iface) {
		soft_iface = batadv_softif_create(iface_name);

		if (!soft_iface) {
			ret = -ENOMEM;
			goto err;
		}

		/*                                                          */
		dev_hold(soft_iface);
	}

	if (!batadv_softif_is_valid(soft_iface)) {
		pr_err("Can't create batman mesh interface %s: already exists as regular interface\n",
		       soft_iface->name);
		ret = -EINVAL;
		goto err_dev;
	}

	/*                                                              
                                
  */
	master = netdev_master_upper_dev_get(hard_iface->net_dev);
	ret = batadv_master_del_slave(hard_iface, master);
	if (ret)
		goto err_dev;

	hard_iface->soft_iface = soft_iface;
	bat_priv = netdev_priv(hard_iface->soft_iface);

	ret = netdev_master_upper_dev_link(hard_iface->net_dev, soft_iface);
	if (ret)
		goto err_dev;

	ret = bat_priv->bat_algo_ops->bat_iface_enable(hard_iface);
	if (ret < 0)
		goto err_upper;

	hard_iface->if_num = bat_priv->num_ifaces;
	bat_priv->num_ifaces++;
	hard_iface->if_status = BATADV_IF_INACTIVE;
	ret = batadv_orig_hash_add_if(hard_iface, bat_priv->num_ifaces);
	if (ret < 0) {
		bat_priv->bat_algo_ops->bat_iface_disable(hard_iface);
		bat_priv->num_ifaces--;
		hard_iface->if_status = BATADV_IF_NOT_IN_USE;
		goto err_upper;
	}

	hard_iface->batman_adv_ptype.type = ethertype;
	hard_iface->batman_adv_ptype.func = batadv_batman_skb_recv;
	hard_iface->batman_adv_ptype.dev = hard_iface->net_dev;
	dev_add_pack(&hard_iface->batman_adv_ptype);

	atomic_set(&hard_iface->frag_seqno, 1);
	batadv_info(hard_iface->soft_iface, "Adding interface: %s\n",
		    hard_iface->net_dev->name);

	if (atomic_read(&bat_priv->fragmentation) &&
	    hard_iface->net_dev->mtu < ETH_DATA_LEN + BATADV_HEADER_LEN)
		batadv_info(hard_iface->soft_iface,
			    "The MTU of interface %s is too small (%i) to handle the transport of batman-adv packets. Packets going over this interface will be fragmented on layer2 which could impact the performance. Setting the MTU to %zi would solve the problem.\n",
			    hard_iface->net_dev->name, hard_iface->net_dev->mtu,
			    ETH_DATA_LEN + BATADV_HEADER_LEN);

	if (!atomic_read(&bat_priv->fragmentation) &&
	    hard_iface->net_dev->mtu < ETH_DATA_LEN + BATADV_HEADER_LEN)
		batadv_info(hard_iface->soft_iface,
			    "The MTU of interface %s is too small (%i) to handle the transport of batman-adv packets. If you experience problems getting traffic through try increasing the MTU to %zi.\n",
			    hard_iface->net_dev->name, hard_iface->net_dev->mtu,
			    ETH_DATA_LEN + BATADV_HEADER_LEN);

	if (batadv_hardif_is_iface_up(hard_iface))
		batadv_hardif_activate_interface(hard_iface);
	else
		batadv_err(hard_iface->soft_iface,
			   "Not using interface %s (retrying later): interface not active\n",
			   hard_iface->net_dev->name);

	/*                                                        */
	batadv_schedule_bat_ogm(hard_iface);

out:
	return 0;

err_upper:
	netdev_upper_dev_unlink(hard_iface->net_dev, soft_iface);
err_dev:
	hard_iface->soft_iface = NULL;
	dev_put(soft_iface);
err:
	batadv_hardif_free_ref(hard_iface);
	return ret;
}

void batadv_hardif_disable_interface(struct batadv_hard_iface *hard_iface,
				     enum batadv_hard_if_cleanup autodel)
{
	struct batadv_priv *bat_priv = netdev_priv(hard_iface->soft_iface);
	struct batadv_hard_iface *primary_if = NULL;

	if (hard_iface->if_status == BATADV_IF_ACTIVE)
		batadv_hardif_deactivate_interface(hard_iface);

	if (hard_iface->if_status != BATADV_IF_INACTIVE)
		goto out;

	batadv_info(hard_iface->soft_iface, "Removing interface: %s\n",
		    hard_iface->net_dev->name);
	dev_remove_pack(&hard_iface->batman_adv_ptype);

	bat_priv->num_ifaces--;
	batadv_orig_hash_del_if(hard_iface, bat_priv->num_ifaces);

	primary_if = batadv_primary_if_get_selected(bat_priv);
	if (hard_iface == primary_if) {
		struct batadv_hard_iface *new_if;

		new_if = batadv_hardif_get_active(hard_iface->soft_iface);
		batadv_primary_if_select(bat_priv, new_if);

		if (new_if)
			batadv_hardif_free_ref(new_if);
	}

	bat_priv->bat_algo_ops->bat_iface_disable(hard_iface);
	hard_iface->if_status = BATADV_IF_NOT_IN_USE;

	/*                                          */
	batadv_purge_orig_ref(bat_priv);
	batadv_purge_outstanding_packets(bat_priv, hard_iface);
	dev_put(hard_iface->soft_iface);

	/*                                    */
	if (!bat_priv->num_ifaces && autodel == BATADV_IF_CLEANUP_AUTO)
		batadv_softif_destroy_sysfs(hard_iface->soft_iface);

	netdev_upper_dev_unlink(hard_iface->net_dev, hard_iface->soft_iface);
	hard_iface->soft_iface = NULL;
	batadv_hardif_free_ref(hard_iface);

out:
	if (primary_if)
		batadv_hardif_free_ref(primary_if);
}

/* 
                                                                            
                         
  
                                                                      
                                              
 */
static void batadv_hardif_remove_interface_finish(struct work_struct *work)
{
	struct batadv_hard_iface *hard_iface;

	hard_iface = container_of(work, struct batadv_hard_iface,
				  cleanup_work);

	batadv_sysfs_del_hardif(&hard_iface->hardif_obj);
	batadv_hardif_free_ref(hard_iface);
}

static struct batadv_hard_iface *
batadv_hardif_add_interface(struct net_device *net_dev)
{
	struct batadv_hard_iface *hard_iface;
	int ret;

	ASSERT_RTNL();

	ret = batadv_is_valid_iface(net_dev);
	if (ret != 1)
		goto out;

	dev_hold(net_dev);

	hard_iface = kmalloc(sizeof(*hard_iface), GFP_ATOMIC);
	if (!hard_iface)
		goto release_dev;

	ret = batadv_sysfs_add_hardif(&hard_iface->hardif_obj, net_dev);
	if (ret)
		goto free_if;

	hard_iface->if_num = -1;
	hard_iface->net_dev = net_dev;
	hard_iface->soft_iface = NULL;
	hard_iface->if_status = BATADV_IF_NOT_IN_USE;
	INIT_LIST_HEAD(&hard_iface->list);
	INIT_WORK(&hard_iface->cleanup_work,
		  batadv_hardif_remove_interface_finish);

	/*                            */
	atomic_set(&hard_iface->refcount, 2);

	batadv_check_known_mac_addr(hard_iface->net_dev);
	list_add_tail_rcu(&hard_iface->list, &batadv_hardif_list);

	/*                                                     
                            
  */
	atomic_set(&hard_iface->bat_iv.ogm_seqno, 1);
	hard_iface->bat_iv.ogm_buff = NULL;

	return hard_iface;

free_if:
	kfree(hard_iface);
release_dev:
	dev_put(net_dev);
out:
	return NULL;
}

static void batadv_hardif_remove_interface(struct batadv_hard_iface *hard_iface)
{
	ASSERT_RTNL();

	/*                            */
	if (hard_iface->if_status != BATADV_IF_NOT_IN_USE)
		batadv_hardif_disable_interface(hard_iface,
						BATADV_IF_CLEANUP_AUTO);

	if (hard_iface->if_status != BATADV_IF_NOT_IN_USE)
		return;

	hard_iface->if_status = BATADV_IF_TO_BE_REMOVED;
	queue_work(batadv_event_workqueue, &hard_iface->cleanup_work);
}

void batadv_hardif_remove_interfaces(void)
{
	struct batadv_hard_iface *hard_iface, *hard_iface_tmp;

	rtnl_lock();
	list_for_each_entry_safe(hard_iface, hard_iface_tmp,
				 &batadv_hardif_list, list) {
		list_del_rcu(&hard_iface->list);
		batadv_hardif_remove_interface(hard_iface);
	}
	rtnl_unlock();
}

static int batadv_hard_if_event(struct notifier_block *this,
				unsigned long event, void *ptr)
{
	struct net_device *net_dev = ptr;
	struct batadv_hard_iface *hard_iface;
	struct batadv_hard_iface *primary_if = NULL;
	struct batadv_priv *bat_priv;

	if (batadv_softif_is_valid(net_dev) && event == NETDEV_REGISTER) {
		batadv_sysfs_add_meshif(net_dev);
		return NOTIFY_DONE;
	}

	hard_iface = batadv_hardif_get_by_netdev(net_dev);
	if (!hard_iface && event == NETDEV_REGISTER)
		hard_iface = batadv_hardif_add_interface(net_dev);

	if (!hard_iface)
		goto out;

	switch (event) {
	case NETDEV_UP:
		batadv_hardif_activate_interface(hard_iface);
		break;
	case NETDEV_GOING_DOWN:
	case NETDEV_DOWN:
		batadv_hardif_deactivate_interface(hard_iface);
		break;
	case NETDEV_UNREGISTER:
		list_del_rcu(&hard_iface->list);

		batadv_hardif_remove_interface(hard_iface);
		break;
	case NETDEV_CHANGEMTU:
		if (hard_iface->soft_iface)
			batadv_update_min_mtu(hard_iface->soft_iface);
		break;
	case NETDEV_CHANGEADDR:
		if (hard_iface->if_status == BATADV_IF_NOT_IN_USE)
			goto hardif_put;

		batadv_check_known_mac_addr(hard_iface->net_dev);

		bat_priv = netdev_priv(hard_iface->soft_iface);
		bat_priv->bat_algo_ops->bat_iface_update_mac(hard_iface);

		primary_if = batadv_primary_if_get_selected(bat_priv);
		if (!primary_if)
			goto hardif_put;

		if (hard_iface == primary_if)
			batadv_primary_if_update_addr(bat_priv, NULL);
		break;
	default:
		break;
	}

hardif_put:
	batadv_hardif_free_ref(hard_iface);
out:
	if (primary_if)
		batadv_hardif_free_ref(primary_if);
	return NOTIFY_DONE;
}

/*                                                                        
                         
 */
bool batadv_is_wifi_iface(int ifindex)
{
	struct net_device *net_device = NULL;
	bool ret = false;

	if (ifindex == BATADV_NULL_IFINDEX)
		goto out;

	net_device = dev_get_by_index(&init_net, ifindex);
	if (!net_device)
		goto out;

#ifdef CONFIG_WIRELESS_EXT
	/*                                                                  
                                       
  */
	if (net_device->wireless_handlers)
		ret = true;
	else
#endif
		/*                                            */
		if (net_device->ieee80211_ptr)
			ret = true;
out:
	if (net_device)
		dev_put(net_device);
	return ret;
}

struct notifier_block batadv_hard_if_notifier = {
	.notifier_call = batadv_hard_if_event,
};
