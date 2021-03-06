/*
 * usb hub driver head file
 *
 * Copyright (C) 1999 Linus Torvalds
 * Copyright (C) 1999 Johannes Erdfelt
 * Copyright (C) 1999 Gregory P. Smith
 * Copyright (C) 2001 Brad Hards (bhards@bigpond.net.au)
 * Copyright (C) 2012 Intel Corp (tianyu.lan@intel.com)
 *
 *  move struct usb_hub to this file.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <linux/usb.h>
#include <linux/usb/ch11.h>
#include <linux/usb/hcd.h>
#include "usb.h"

struct usb_hub {
	struct device		*intfdev;	/*                        */
	struct usb_device	*hdev;
	struct kref		kref;
	struct urb		*urb;		/*                            */

	/*                                                       */
	u8			(*buffer)[8];
	union {
		struct usb_hub_status	hub;
		struct usb_port_status	port;
	}			*status;	/*                           */
	struct mutex		status_mutex;	/*                       */

	int			error;		/*                     */
	int			nerrors;	/*                          */

	struct list_head	event_list;	/*                           */
	unsigned long		event_bits[1];	/*                       */
	unsigned long		change_bits[1];	/*                           
                     */
	unsigned long		busy_bits[1];	/*                     
               */
	unsigned long		removed_bits[1]; /*                       
                      */
	unsigned long		wakeup_bits[1];	/*                         
                     */
#if USB_MAXCHILDREN > 31 /*                             */
#error event_bits[] is too short!
#endif

	struct usb_hub_descriptor *descriptor;	/*                  */
	struct usb_tt		tt;		/*                        */

	unsigned		mA_per_port;	/*                        */
#ifdef	CONFIG_PM
	unsigned		wakeup_enabled_descendants;
#endif

	unsigned		limited_power:1;
	unsigned		quiescing:1;
	unsigned		disconnected:1;

	unsigned		quirk_check_port_auto_suspend:1;

	unsigned		has_indicators:1;
	u8			indicator[USB_MAXCHILDREN];
	struct delayed_work	leds;
	struct delayed_work	init_work;
	struct usb_port		**ports;
};

/* 
                                                          
                                           
                                 
                            
                                     
                                     
                                   
                                                    
 */
struct usb_port {
	struct usb_device *child;
	struct device dev;
	struct dev_state *port_owner;
	enum usb_port_connect_type connect_type;
	u8 portnum;
	unsigned power_is_on:1;
	unsigned did_runtime_put:1;
};

#define to_usb_port(_dev) \
	container_of(_dev, struct usb_port, dev)

extern int usb_hub_create_port_device(struct usb_hub *hub,
		int port1);
extern void usb_hub_remove_port_device(struct usb_hub *hub,
		int port1);
extern int usb_hub_set_port_power(struct usb_device *hdev,
		int port1, bool set);
extern struct usb_hub *usb_hub_to_struct_hub(struct usb_device *hdev);
extern int hub_port_debounce(struct usb_hub *hub, int port1,
		bool must_be_connected);
extern int usb_clear_port_feature(struct usb_device *hdev,
		int port1, int feature);

static inline int hub_port_debounce_be_connected(struct usb_hub *hub,
		int port1)
{
	return hub_port_debounce(hub, port1, true);
}

static inline int hub_port_debounce_be_stable(struct usb_hub *hub,
		int port1)
{
	return hub_port_debounce(hub, port1, false);
}

