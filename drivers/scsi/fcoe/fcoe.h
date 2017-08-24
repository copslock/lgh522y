/*
 * Copyright(c) 2009 Intel Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Maintained at www.Open-FCoE.org
 */

#ifndef _FCOE_H_
#define _FCOE_H_

#include <linux/skbuff.h>
#include <linux/kthread.h>

#define FCOE_MAX_QUEUE_DEPTH	256
#define FCOE_MIN_QUEUE_DEPTH	32

#define FCOE_WORD_TO_BYTE	4

#define FCOE_VERSION	"0.1"
#define FCOE_NAME	"fcoe"
#define FCOE_VENDOR	"Open-FCoE.org"

#define FCOE_MAX_LUN		0xFFFF
#define FCOE_MAX_FCP_TARGET	256

#define FCOE_MAX_OUTSTANDING_COMMANDS	1024

#define FCOE_MIN_XID		0x0000	/*                                  */
#define FCOE_MAX_XID		0x0FFF	/*                                  */

extern unsigned int fcoe_debug_logging;

#define FCOE_LOGGING	    0x01 /*                                  */
#define FCOE_NETDEV_LOGGING 0x02 /*                   */

#define FCOE_CHECK_LOGGING(LEVEL, CMD)					\
do {                                                            	\
	if (unlikely(fcoe_debug_logging & LEVEL))			\
		do {							\
			CMD;						\
		} while (0);						\
} while (0)

#define FCOE_DBG(fmt, args...)						\
	FCOE_CHECK_LOGGING(FCOE_LOGGING,				\
			   pr_info("fcoe: " fmt, ##args);)

#define FCOE_NETDEV_DBG(netdev, fmt, args...)			\
	FCOE_CHECK_LOGGING(FCOE_NETDEV_LOGGING,			\
			   pr_info("fcoe: %s: " fmt,		\
				   netdev->name, ##args);)

/* 
                                           
                                                    
                                           
                                      
                                     
                                                              
                                             
                                                                   
                                                      
                                           
 */
struct fcoe_interface {
	struct list_head   list;
	struct net_device  *netdev;
	struct net_device  *realdev;
	struct packet_type fcoe_packet_type;
	struct packet_type fip_packet_type;
	struct fc_exch_mgr *oem;
	u8	removed;
	u8	priority;
};

#define fcoe_to_ctlr(x)						\
	(struct fcoe_ctlr *)(((struct fcoe_ctlr *)(x)) - 1)

#define fcoe_from_ctlr(x)			\
	((struct fcoe_interface *)((x) + 1))

/* 
                                                                     
                                                    
 */
static inline struct net_device *fcoe_netdev(const struct fc_lport *lport)
{
	return ((struct fcoe_interface *)
			((struct fcoe_port *)lport_priv(lport))->priv)->netdev;
}

#endif /*          */
