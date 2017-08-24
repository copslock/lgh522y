/*
 * net/tipc/node.h: Include file for TIPC node management routines
 *
 * Copyright (c) 2000-2006, Ericsson AB
 * Copyright (c) 2005, 2010-2011, Wind River Systems
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the names of the copyright holders nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _TIPC_NODE_H
#define _TIPC_NODE_H

#include "node_subscr.h"
#include "addr.h"
#include "net.h"
#include "bearer.h"

/*
                                        
 */
#define INVALID_NODE_SIG 0x10000

/*                                                                          */
#define WAIT_PEER_DOWN	0x0001	/*                                        */
#define WAIT_NAMES_GONE	0x0002	/*                                           */
#define WAIT_NODE_DOWN	0x0004	/*                                       */

/* 
                                         
                                 
                                                
                                                        
                                                                   
                                                           
                                                  
                                        
                                                                            
                                                                             
                                     
                                                                          
                                       
                                  
                                                                             
                                                                                
                                                                
                                                                
                                                                     
                                                                  
                                                                  
                                                                                
                                                                         
 */
struct tipc_node {
	u32 addr;
	spinlock_t lock;
	struct hlist_node hash;
	struct list_head list;
	struct list_head nsub;
	struct tipc_link *active_links[2];
	struct tipc_link *links[MAX_BEARERS];
	int link_cnt;
	int working_links;
	int block_setup;
	int permit_changeover;
	u32 signature;
	struct {
		u32 acked;
		u32 last_in;
		u32 last_sent;
		u32 oos_state;
		u32 deferred_size;
		struct sk_buff *deferred_head;
		struct sk_buff *deferred_tail;
		struct sk_buff *defragm;
		bool recv_permitted;
	} bclink;
};

extern struct list_head tipc_node_list;

struct tipc_node *tipc_node_find(u32 addr);
struct tipc_node *tipc_node_create(u32 addr);
void tipc_node_delete(struct tipc_node *n_ptr);
void tipc_node_attach_link(struct tipc_node *n_ptr, struct tipc_link *l_ptr);
void tipc_node_detach_link(struct tipc_node *n_ptr, struct tipc_link *l_ptr);
void tipc_node_link_down(struct tipc_node *n_ptr, struct tipc_link *l_ptr);
void tipc_node_link_up(struct tipc_node *n_ptr, struct tipc_link *l_ptr);
int tipc_node_active_links(struct tipc_node *n_ptr);
int tipc_node_redundant_links(struct tipc_node *n_ptr);
int tipc_node_is_up(struct tipc_node *n_ptr);
struct sk_buff *tipc_node_get_links(const void *req_tlv_area, int req_tlv_space);
struct sk_buff *tipc_node_get_nodes(const void *req_tlv_area, int req_tlv_space);

static inline void tipc_node_lock(struct tipc_node *n_ptr)
{
	spin_lock_bh(&n_ptr->lock);
}

static inline void tipc_node_unlock(struct tipc_node *n_ptr)
{
	spin_unlock_bh(&n_ptr->lock);
}

#endif
