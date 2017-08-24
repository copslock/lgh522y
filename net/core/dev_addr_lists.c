/*
 * net/core/dev_addr_lists.c - Functions for handling net device lists
 * Copyright (c) 2010 Jiri Pirko <jpirko@redhat.com>
 *
 * This file contains functions for working with unicast, multicast and device
 * addresses lists.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/netdevice.h>
#include <linux/rtnetlink.h>
#include <linux/export.h>
#include <linux/list.h>

/*
                                  
 */

static int __hw_addr_create_ex(struct netdev_hw_addr_list *list,
			       const unsigned char *addr, int addr_len,
			       unsigned char addr_type, bool global,
			       bool sync)
{
	struct netdev_hw_addr *ha;
	int alloc_size;

	alloc_size = sizeof(*ha);
	if (alloc_size < L1_CACHE_BYTES)
		alloc_size = L1_CACHE_BYTES;
	ha = kmalloc(alloc_size, GFP_ATOMIC);
	if (!ha)
		return -ENOMEM;
	memcpy(ha->addr, addr, addr_len);
	ha->type = addr_type;
	ha->refcount = 1;
	ha->global_use = global;
	ha->synced = sync;
	ha->sync_cnt = 0;
	list_add_tail_rcu(&ha->list, &list->list);
	list->count++;

	return 0;
}

static int __hw_addr_add_ex(struct netdev_hw_addr_list *list,
			    const unsigned char *addr, int addr_len,
			    unsigned char addr_type, bool global, bool sync)
{
	struct netdev_hw_addr *ha;

	if (addr_len > MAX_ADDR_LEN)
		return -EINVAL;

	list_for_each_entry(ha, &list->list, list) {
		if (!memcmp(ha->addr, addr, addr_len) &&
		    ha->type == addr_type) {
			if (global) {
				/*                                         */
				if (ha->global_use)
					return 0;
				else
					ha->global_use = true;
			}
			if (sync) {
				if (ha->synced)
					return -EEXIST;
				else
					ha->synced = true;
			}
			ha->refcount++;
			return 0;
		}
	}

	return __hw_addr_create_ex(list, addr, addr_len, addr_type, global,
				   sync);
}

static int __hw_addr_add(struct netdev_hw_addr_list *list,
			 const unsigned char *addr, int addr_len,
			 unsigned char addr_type)
{
	return __hw_addr_add_ex(list, addr, addr_len, addr_type, false, false);
}

static int __hw_addr_del_entry(struct netdev_hw_addr_list *list,
			       struct netdev_hw_addr *ha, bool global,
			       bool sync)
{
	if (global && !ha->global_use)
		return -ENOENT;

	if (sync && !ha->synced)
		return -ENOENT;

	if (global)
		ha->global_use = false;

	if (sync)
		ha->synced = false;

	if (--ha->refcount)
		return 0;
	list_del_rcu(&ha->list);
	kfree_rcu(ha, rcu_head);
	list->count--;
	return 0;
}

static int __hw_addr_del_ex(struct netdev_hw_addr_list *list,
			    const unsigned char *addr, int addr_len,
			    unsigned char addr_type, bool global, bool sync)
{
	struct netdev_hw_addr *ha;

	list_for_each_entry(ha, &list->list, list) {
		if (!memcmp(ha->addr, addr, addr_len) &&
		    (ha->type == addr_type || !addr_type))
			return __hw_addr_del_entry(list, ha, global, sync);
	}
	return -ENOENT;
}

static int __hw_addr_del(struct netdev_hw_addr_list *list,
			 const unsigned char *addr, int addr_len,
			 unsigned char addr_type)
{
	return __hw_addr_del_ex(list, addr, addr_len, addr_type, false, false);
}

static int __hw_addr_sync_one(struct netdev_hw_addr_list *to_list,
			       struct netdev_hw_addr *ha,
			       int addr_len)
{
	int err;

	err = __hw_addr_add_ex(to_list, ha->addr, addr_len, ha->type,
			       false, true);
	if (err && err != -EEXIST)
		return err;

	if (!err) {
		ha->sync_cnt++;
		ha->refcount++;
	}

	return 0;
}

static void __hw_addr_unsync_one(struct netdev_hw_addr_list *to_list,
				 struct netdev_hw_addr_list *from_list,
				 struct netdev_hw_addr *ha,
				 int addr_len)
{
	int err;

	err = __hw_addr_del_ex(to_list, ha->addr, addr_len, ha->type,
			       false, true);
	if (err)
		return;
	ha->sync_cnt--;
	/*                                           */
	__hw_addr_del_entry(from_list, ha, false, false);
}

static int __hw_addr_sync_multiple(struct netdev_hw_addr_list *to_list,
				   struct netdev_hw_addr_list *from_list,
				   int addr_len)
{
	int err = 0;
	struct netdev_hw_addr *ha, *tmp;

	list_for_each_entry_safe(ha, tmp, &from_list->list, list) {
		if (ha->sync_cnt == ha->refcount) {
			__hw_addr_unsync_one(to_list, from_list, ha, addr_len);
		} else {
			err = __hw_addr_sync_one(to_list, ha, addr_len);
			if (err)
				break;
		}
	}
	return err;
}

int __hw_addr_add_multiple(struct netdev_hw_addr_list *to_list,
			   struct netdev_hw_addr_list *from_list,
			   int addr_len, unsigned char addr_type)
{
	int err;
	struct netdev_hw_addr *ha, *ha2;
	unsigned char type;

	list_for_each_entry(ha, &from_list->list, list) {
		type = addr_type ? addr_type : ha->type;
		err = __hw_addr_add(to_list, ha->addr, addr_len, type);
		if (err)
			goto unroll;
	}
	return 0;

unroll:
	list_for_each_entry(ha2, &from_list->list, list) {
		if (ha2 == ha)
			break;
		type = addr_type ? addr_type : ha2->type;
		__hw_addr_del(to_list, ha2->addr, addr_len, type);
	}
	return err;
}
EXPORT_SYMBOL(__hw_addr_add_multiple);

void __hw_addr_del_multiple(struct netdev_hw_addr_list *to_list,
			    struct netdev_hw_addr_list *from_list,
			    int addr_len, unsigned char addr_type)
{
	struct netdev_hw_addr *ha;
	unsigned char type;

	list_for_each_entry(ha, &from_list->list, list) {
		type = addr_type ? addr_type : ha->type;
		__hw_addr_del(to_list, ha->addr, addr_len, type);
	}
}
EXPORT_SYMBOL(__hw_addr_del_multiple);

/*                                                                  
                                                                     
                                                             
                             
 */
int __hw_addr_sync(struct netdev_hw_addr_list *to_list,
		   struct netdev_hw_addr_list *from_list,
		   int addr_len)
{
	int err = 0;
	struct netdev_hw_addr *ha, *tmp;

	list_for_each_entry_safe(ha, tmp, &from_list->list, list) {
		if (!ha->sync_cnt) {
			err = __hw_addr_sync_one(to_list, ha, addr_len);
			if (err)
				break;
		} else if (ha->refcount == 1)
			__hw_addr_unsync_one(to_list, from_list, ha, addr_len);
	}
	return err;
}
EXPORT_SYMBOL(__hw_addr_sync);

void __hw_addr_unsync(struct netdev_hw_addr_list *to_list,
		      struct netdev_hw_addr_list *from_list,
		      int addr_len)
{
	struct netdev_hw_addr *ha, *tmp;

	list_for_each_entry_safe(ha, tmp, &from_list->list, list) {
		if (ha->sync_cnt)
			__hw_addr_unsync_one(to_list, from_list, ha, addr_len);
	}
}
EXPORT_SYMBOL(__hw_addr_unsync);

void __hw_addr_flush(struct netdev_hw_addr_list *list)
{
	struct netdev_hw_addr *ha, *tmp;

	list_for_each_entry_safe(ha, tmp, &list->list, list) {
		list_del_rcu(&ha->list);
		kfree_rcu(ha, rcu_head);
	}
	list->count = 0;
}
EXPORT_SYMBOL(__hw_addr_flush);

void __hw_addr_init(struct netdev_hw_addr_list *list)
{
	INIT_LIST_HEAD(&list->list);
	list->count = 0;
}
EXPORT_SYMBOL(__hw_addr_init);

/*
                                      
 */

/* 
                                             
               
  
                                                  
  
                                       
 */
void dev_addr_flush(struct net_device *dev)
{
	/*                              */

	__hw_addr_flush(&dev->dev_addrs);
	dev->dev_addr = NULL;
}
EXPORT_SYMBOL(dev_addr_flush);

/* 
                                           
               
  
                                                         
                      
  
                                       
 */
int dev_addr_init(struct net_device *dev)
{
	unsigned char addr[MAX_ADDR_LEN];
	struct netdev_hw_addr *ha;
	int err;

	/*                              */

	__hw_addr_init(&dev->dev_addrs);
	memset(addr, 0, sizeof(addr));
	err = __hw_addr_add(&dev->dev_addrs, addr, sizeof(addr),
			    NETDEV_HW_ADDR_T_LAN);
	if (!err) {
		/*
                                                             
                                               
   */
		ha = list_first_entry(&dev->dev_addrs.list,
				      struct netdev_hw_addr, list);
		dev->dev_addr = ha->addr;
	}
	return err;
}
EXPORT_SYMBOL(dev_addr_init);

/* 
                                      
               
                        
                           
  
                                                                        
                     
  
                                       
 */
int dev_addr_add(struct net_device *dev, const unsigned char *addr,
		 unsigned char addr_type)
{
	int err;

	ASSERT_RTNL();

	err = __hw_addr_add(&dev->dev_addrs, addr, dev->addr_len, addr_type);
	if (!err)
		call_netdevice_notifiers(NETDEV_CHANGEADDR, dev);
	return err;
}
EXPORT_SYMBOL(dev_addr_add);

/* 
                                           
               
                           
                           
  
                                                                      
                                        
  
                                       
 */
int dev_addr_del(struct net_device *dev, const unsigned char *addr,
		 unsigned char addr_type)
{
	int err;
	struct netdev_hw_addr *ha;

	ASSERT_RTNL();

	/*
                                                             
                                 
  */
	ha = list_first_entry(&dev->dev_addrs.list,
			      struct netdev_hw_addr, list);
	if (!memcmp(ha->addr, addr, dev->addr_len) &&
	    ha->type == addr_type && ha->refcount == 1)
		return -ENOENT;

	err = __hw_addr_del(&dev->dev_addrs, addr, dev->addr_len,
			    addr_type);
	if (!err)
		call_netdevice_notifiers(NETDEV_CHANGEADDR, dev);
	return err;
}
EXPORT_SYMBOL(dev_addr_del);

/* 
                                                                   
                                                   
                                                       
                                                                     
  
                                                     
   
                                       
 */
int dev_addr_add_multiple(struct net_device *to_dev,
			  struct net_device *from_dev,
			  unsigned char addr_type)
{
	int err;

	ASSERT_RTNL();

	if (from_dev->addr_len != to_dev->addr_len)
		return -EINVAL;
	err = __hw_addr_add_multiple(&to_dev->dev_addrs, &from_dev->dev_addrs,
				     to_dev->addr_len, addr_type);
	if (!err)
		call_netdevice_notifiers(NETDEV_CHANGEADDR, to_dev);
	return err;
}
EXPORT_SYMBOL(dev_addr_add_multiple);

/* 
                                                                    
                                                      
                                                          
                                                                     
  
                                                                          
  
                                       
 */
int dev_addr_del_multiple(struct net_device *to_dev,
			  struct net_device *from_dev,
			  unsigned char addr_type)
{
	ASSERT_RTNL();

	if (from_dev->addr_len != to_dev->addr_len)
		return -EINVAL;
	__hw_addr_del_multiple(&to_dev->dev_addrs, &from_dev->dev_addrs,
			       to_dev->addr_len, addr_type);
	call_netdevice_notifiers(NETDEV_CHANGEADDR, to_dev);
	return 0;
}
EXPORT_SYMBOL(dev_addr_del_multiple);

/*
                                  
 */

/* 
                                                           
               
                        
 */
int dev_uc_add_excl(struct net_device *dev, const unsigned char *addr)
{
	struct netdev_hw_addr *ha;
	int err;

	netif_addr_lock_bh(dev);
	list_for_each_entry(ha, &dev->uc.list, list) {
		if (!memcmp(ha->addr, addr, dev->addr_len) &&
		    ha->type == NETDEV_HW_ADDR_T_UNICAST) {
			err = -EEXIST;
			goto out;
		}
	}
	err = __hw_addr_create_ex(&dev->uc, addr, dev->addr_len,
				  NETDEV_HW_ADDR_T_UNICAST, true, false);
	if (!err)
		__dev_set_rx_mode(dev);
out:
	netif_addr_unlock_bh(dev);
	return err;
}
EXPORT_SYMBOL(dev_uc_add_excl);

/* 
                                               
               
                        
  
                                                            
                                            
 */
int dev_uc_add(struct net_device *dev, const unsigned char *addr)
{
	int err;

	netif_addr_lock_bh(dev);
	err = __hw_addr_add(&dev->uc, addr, dev->addr_len,
			    NETDEV_HW_ADDR_T_UNICAST);
	if (!err)
		__dev_set_rx_mode(dev);
	netif_addr_unlock_bh(dev);
	return err;
}
EXPORT_SYMBOL(dev_uc_add);

/* 
                                                  
               
                           
  
                                                                 
                                                        
 */
int dev_uc_del(struct net_device *dev, const unsigned char *addr)
{
	int err;

	netif_addr_lock_bh(dev);
	err = __hw_addr_del(&dev->uc, addr, dev->addr_len,
			    NETDEV_HW_ADDR_T_UNICAST);
	if (!err)
		__dev_set_rx_mode(dev);
	netif_addr_unlock_bh(dev);
	return err;
}
EXPORT_SYMBOL(dev_uc_del);

/* 
                                                                    
                          
                       
  
                                                                  
                                                               
                                
  
                                                                   
                                                                    
                                                                      
 */
int dev_uc_sync(struct net_device *to, struct net_device *from)
{
	int err = 0;

	if (to->addr_len != from->addr_len)
		return -EINVAL;

	netif_addr_lock_nested(to);
	err = __hw_addr_sync(&to->uc, &from->uc, to->addr_len);
	if (!err)
		__dev_set_rx_mode(to);
	netif_addr_unlock(to);
	return err;
}
EXPORT_SYMBOL(dev_uc_sync);

/* 
                                                                      
                                                                    
                          
                       
  
                                                                  
                                                                      
                                        
  
                                                                   
                                                                       
                                                       
 */
int dev_uc_sync_multiple(struct net_device *to, struct net_device *from)
{
	int err = 0;

	if (to->addr_len != from->addr_len)
		return -EINVAL;

	netif_addr_lock_nested(to);
	err = __hw_addr_sync_multiple(&to->uc, &from->uc, to->addr_len);
	if (!err)
		__dev_set_rx_mode(to);
	netif_addr_unlock(to);
	return err;
}
EXPORT_SYMBOL(dev_uc_sync_multiple);

/* 
                                                                            
                          
                       
  
                                                                    
                                                                 
                                                  
 */
void dev_uc_unsync(struct net_device *to, struct net_device *from)
{
	if (to->addr_len != from->addr_len)
		return;

	netif_addr_lock_bh(from);
	netif_addr_lock_nested(to);
	__hw_addr_unsync(&to->uc, &from->uc, to->addr_len);
	__dev_set_rx_mode(to);
	netif_addr_unlock(to);
	netif_addr_unlock_bh(from);
}
EXPORT_SYMBOL(dev_uc_unsync);

/* 
                                         
               
  
                           
 */
void dev_uc_flush(struct net_device *dev)
{
	netif_addr_lock_bh(dev);
	__hw_addr_flush(&dev->uc);
	netif_addr_unlock_bh(dev);
}
EXPORT_SYMBOL(dev_uc_flush);

/* 
                                           
               
  
                             
 */
void dev_uc_init(struct net_device *dev)
{
	__hw_addr_init(&dev->uc);
}
EXPORT_SYMBOL(dev_uc_init);

/*
                                    
 */

/* 
                                                             
               
                        
 */
int dev_mc_add_excl(struct net_device *dev, const unsigned char *addr)
{
	struct netdev_hw_addr *ha;
	int err;

	netif_addr_lock_bh(dev);
	list_for_each_entry(ha, &dev->mc.list, list) {
		if (!memcmp(ha->addr, addr, dev->addr_len) &&
		    ha->type == NETDEV_HW_ADDR_T_MULTICAST) {
			err = -EEXIST;
			goto out;
		}
	}
	err = __hw_addr_create_ex(&dev->mc, addr, dev->addr_len,
				  NETDEV_HW_ADDR_T_MULTICAST, true, false);
	if (!err)
		__dev_set_rx_mode(dev);
out:
	netif_addr_unlock_bh(dev);
	return err;
}
EXPORT_SYMBOL(dev_mc_add_excl);

static int __dev_mc_add(struct net_device *dev, const unsigned char *addr,
			bool global)
{
	int err;

	netif_addr_lock_bh(dev);
	err = __hw_addr_add_ex(&dev->mc, addr, dev->addr_len,
			       NETDEV_HW_ADDR_T_MULTICAST, global, false);
	if (!err)
		__dev_set_rx_mode(dev);
	netif_addr_unlock_bh(dev);
	return err;
}
/* 
                                       
               
                        
  
                                                    
                                            
 */
int dev_mc_add(struct net_device *dev, const unsigned char *addr)
{
	return __dev_mc_add(dev, addr, false);
}
EXPORT_SYMBOL(dev_mc_add);

/* 
                                                     
               
                        
  
                                                
 */
int dev_mc_add_global(struct net_device *dev, const unsigned char *addr)
{
	return __dev_mc_add(dev, addr, true);
}
EXPORT_SYMBOL(dev_mc_add_global);

static int __dev_mc_del(struct net_device *dev, const unsigned char *addr,
			bool global)
{
	int err;

	netif_addr_lock_bh(dev);
	err = __hw_addr_del_ex(&dev->mc, addr, dev->addr_len,
			       NETDEV_HW_ADDR_T_MULTICAST, global, false);
	if (!err)
		__dev_set_rx_mode(dev);
	netif_addr_unlock_bh(dev);
	return err;
}

/* 
                                           
               
                           
  
                                                         
                                                        
 */
int dev_mc_del(struct net_device *dev, const unsigned char *addr)
{
	return __dev_mc_del(dev, addr, false);
}
EXPORT_SYMBOL(dev_mc_del);

/* 
                                                         
               
                           
  
                                                         
                                                        
 */
int dev_mc_del_global(struct net_device *dev, const unsigned char *addr)
{
	return __dev_mc_del(dev, addr, true);
}
EXPORT_SYMBOL(dev_mc_del_global);

/* 
                                                                    
                          
                       
  
                                                                  
                                                               
                                
  
                                                                  
                                        
 */
int dev_mc_sync(struct net_device *to, struct net_device *from)
{
	int err = 0;

	if (to->addr_len != from->addr_len)
		return -EINVAL;

	netif_addr_lock_nested(to);
	err = __hw_addr_sync(&to->mc, &from->mc, to->addr_len);
	if (!err)
		__dev_set_rx_mode(to);
	netif_addr_unlock(to);
	return err;
}
EXPORT_SYMBOL(dev_mc_sync);

/* 
                                                                      
                                                                    
                          
                       
  
                                                                  
                                                               
                                
  
                                                                  
                                                                
                                                              
 */
int dev_mc_sync_multiple(struct net_device *to, struct net_device *from)
{
	int err = 0;

	if (to->addr_len != from->addr_len)
		return -EINVAL;

	netif_addr_lock_nested(to);
	err = __hw_addr_sync_multiple(&to->mc, &from->mc, to->addr_len);
	if (!err)
		__dev_set_rx_mode(to);
	netif_addr_unlock(to);
	return err;
}
EXPORT_SYMBOL(dev_mc_sync_multiple);

/* 
                                                                            
                          
                       
  
                                                                    
                                                                 
                                                  
 */
void dev_mc_unsync(struct net_device *to, struct net_device *from)
{
	if (to->addr_len != from->addr_len)
		return;

	netif_addr_lock_bh(from);
	netif_addr_lock_nested(to);
	__hw_addr_unsync(&to->mc, &from->mc, to->addr_len);
	__dev_set_rx_mode(to);
	netif_addr_unlock(to);
	netif_addr_unlock_bh(from);
}
EXPORT_SYMBOL(dev_mc_unsync);

/* 
                                           
               
  
                             
 */
void dev_mc_flush(struct net_device *dev)
{
	netif_addr_lock_bh(dev);
	__hw_addr_flush(&dev->mc);
	netif_addr_unlock_bh(dev);
}
EXPORT_SYMBOL(dev_mc_flush);

/* 
                                             
               
  
                               
 */
void dev_mc_init(struct net_device *dev)
{
	__hw_addr_init(&dev->mc);
}
EXPORT_SYMBOL(dev_mc_init);
