/*
 * ipmi_si.c
 *
 * The interface to the IPMI driver for the system interfaces (KCS, SMIC,
 * BT).
 *
 * Author: MontaVista Software, Inc.
 *         Corey Minyard <minyard@mvista.com>
 *         source@mvista.com
 *
 * Copyright 2002 MontaVista Software Inc.
 * Copyright 2006 IBM Corp., Christian Krafft <krafft@de.ibm.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2 of the License, or (at your
 *  option) any later version.
 *
 *
 *  THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 *  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
                                                                  
                                                                      
                                         
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/timer.h>
#include <linux/errno.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <linux/pci.h>
#include <linux/ioport.h>
#include <linux/notifier.h>
#include <linux/mutex.h>
#include <linux/kthread.h>
#include <asm/irq.h>
#include <linux/interrupt.h>
#include <linux/rcupdate.h>
#include <linux/ipmi.h>
#include <linux/ipmi_smi.h>
#include <asm/io.h>
#include "ipmi_si_sm.h"
#include <linux/init.h>
#include <linux/dmi.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/pnp.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>

#define PFX "ipmi_si: "

/*                                             */
#undef DEBUG_TIMING

/*                   */
#define SI_TIMEOUT_TIME_USEC	10000
#define SI_USEC_PER_JIFFY	(1000000/HZ)
#define SI_TIMEOUT_JIFFIES	(SI_TIMEOUT_TIME_USEC/SI_USEC_PER_JIFFY)
#define SI_SHORT_TIMEOUT_USEC  250 /*                            
                        */

enum si_intf_state {
	SI_NORMAL,
	SI_GETTING_FLAGS,
	SI_GETTING_EVENTS,
	SI_CLEARING_FLAGS,
	SI_CLEARING_FLAGS_THEN_SET_IRQ,
	SI_GETTING_MESSAGES,
	SI_ENABLE_INTERRUPTS1,
	SI_ENABLE_INTERRUPTS2,
	SI_DISABLE_INTERRUPTS1,
	SI_DISABLE_INTERRUPTS2
	/*                             */
};

/*                                        */
#define IPMI_BT_INTMASK_REG		2
#define IPMI_BT_INTMASK_CLEAR_IRQ_BIT	2
#define IPMI_BT_INTMASK_ENABLE_IRQ_BIT	1

enum si_type {
    SI_KCS, SI_SMIC, SI_BT
};
static char *si_to_str[] = { "kcs", "smic", "bt" };

static char *ipmi_addr_src_to_str[] = { NULL, "hotmod", "hardcoded", "SPMI",
					"ACPI", "SMBIOS", "PCI",
					"device-tree", "default" };

#define DEVICE_NAME "ipmi_si"

static struct platform_driver ipmi_driver;

/*
                                          
 */
enum si_stat_indexes {
	/*
                                                                   
                    
  */
	SI_STAT_short_timeouts = 0,

	/*
                                                                     
             
  */
	SI_STAT_long_timeouts,

	/*                                                            */
	SI_STAT_idles,

	/*                                          */
	SI_STAT_interrupts,

	/*                                                          */
	SI_STAT_attentions,

	/*                                                               */
	SI_STAT_flag_fetches,

	/*                                                               */
	SI_STAT_hosed_count,

	/*                               */
	SI_STAT_complete_transactions,

	/*                                                   */
	SI_STAT_events,

	/*                                 */
	SI_STAT_watchdog_pretimeouts,

	/*                                           */
	SI_STAT_incoming_messages,


	/*                                                     */
	SI_NUM_STATS
};

struct smi_info {
	int                    intf_num;
	ipmi_smi_t             intf;
	struct si_sm_data      *si_sm;
	struct si_sm_handlers  *handlers;
	enum si_type           si_type;
	spinlock_t             si_lock;
	struct list_head       xmit_msgs;
	struct list_head       hp_xmit_msgs;
	struct ipmi_smi_msg    *curr_msg;
	enum si_intf_state     si_state;

	/*
                                                               
        
  */
	struct si_sm_io io;
	int (*io_setup)(struct smi_info *info);
	void (*io_cleanup)(struct smi_info *info);
	int (*irq_setup)(struct smi_info *info);
	void (*irq_cleanup)(struct smi_info *info);
	unsigned int io_size;
	enum ipmi_addr_src addr_source; /*                                   */
	void (*addr_source_cleanup)(struct smi_info *info);
	void *addr_source_data;

	/*
                                                           
                                                             
                        
  */
	int (*oem_data_avail_handler)(struct smi_info *smi_info);

	/*
                                                                
                                                                  
                   
  */
#define RECEIVE_MSG_AVAIL	0x01
#define EVENT_MSG_BUFFER_FULL	0x02
#define WDT_PRE_TIMEOUT_INT	0x08
#define OEM0_DATA_AVAIL     0x20
#define OEM1_DATA_AVAIL     0x40
#define OEM2_DATA_AVAIL     0x80
#define OEM_DATA_AVAIL      (OEM0_DATA_AVAIL | \
			     OEM1_DATA_AVAIL | \
			     OEM2_DATA_AVAIL)
	unsigned char       msg_flags;

	/*                                    */
	char		    has_event_buffer;

	/*
                                                              
                          
  */
	atomic_t            req_events;

	/*
                                                              
                                                               
        
  */
	int                 run_to_completion;

	/*                                  */
	int                 port;

	/*
                                                            
                                                                   
                             
  */
	unsigned int        spacing;

	/*                 */
	int                 irq;

	/*                        */
	struct timer_list   si_timer;

	/*                                                                          */
	bool		    timer_running;

	/*                                                     */
	unsigned long       last_timeout_jiffies;

	/*                                                            */
	atomic_t            stop_operation;

	/*
                                                          
                                                            
                                                             
               
  */
	int interrupt_disabled;

	/*                                    */
	struct ipmi_device_id device_id;

	/*                     */
	struct device *dev;
	struct platform_device *pdev;

	/*
                                                          
                              
  */
	int dev_registered;

	/*                                            */
	unsigned char slave_addr;

	/*                                              */
	atomic_t stats[SI_NUM_STATS];

	struct task_struct *thread;

	struct list_head link;
	union ipmi_smi_info_union addr_info;
};

#define smi_inc_stat(smi, stat) \
	atomic_inc(&(smi)->stats[SI_STAT_ ## stat])
#define smi_get_stat(smi, stat) \
	((unsigned int) atomic_read(&(smi)->stats[SI_STAT_ ## stat]))

#define SI_MAX_PARMS 4

static int force_kipmid[SI_MAX_PARMS];
static int num_force_kipmid;
#ifdef CONFIG_PCI
static int pci_registered;
#endif
#ifdef CONFIG_ACPI
static int pnp_registered;
#endif

static unsigned int kipmid_max_busy_us[SI_MAX_PARMS];
static int num_max_busy_us;

static int unload_when_empty = 1;

static int add_smi(struct smi_info *smi);
static int try_smi_init(struct smi_info *smi);
static void cleanup_one_si(struct smi_info *to_clean);
static void cleanup_ipmi_si(void);

static ATOMIC_NOTIFIER_HEAD(xaction_notifier_list);
static int register_xaction_notifier(struct notifier_block *nb)
{
	return atomic_notifier_chain_register(&xaction_notifier_list, nb);
}

static void deliver_recv_msg(struct smi_info *smi_info,
			     struct ipmi_smi_msg *msg)
{
	/*                                         */
	ipmi_smi_msg_received(smi_info->intf, msg);
}

static void return_hosed_msg(struct smi_info *smi_info, int cCode)
{
	struct ipmi_smi_msg *msg = smi_info->curr_msg;

	if (cCode < 0 || cCode > IPMI_ERR_UNSPECIFIED)
		cCode = IPMI_ERR_UNSPECIFIED;
	/*                   */

	/*                    */
	msg->rsp[0] = msg->data[0] | 4;
	msg->rsp[1] = msg->data[1];
	msg->rsp[2] = cCode;
	msg->rsp_size = 3;

	smi_info->curr_msg = NULL;
	deliver_recv_msg(smi_info, msg);
}

static enum si_sm_result start_next_msg(struct smi_info *smi_info)
{
	int              rv;
	struct list_head *entry = NULL;
#ifdef DEBUG_TIMING
	struct timeval t;
#endif

	/*                                     */
	if (!list_empty(&(smi_info->hp_xmit_msgs))) {
		entry = smi_info->hp_xmit_msgs.next;
	} else if (!list_empty(&(smi_info->xmit_msgs))) {
		entry = smi_info->xmit_msgs.next;
	}

	if (!entry) {
		smi_info->curr_msg = NULL;
		rv = SI_SM_IDLE;
	} else {
		int err;

		list_del(entry);
		smi_info->curr_msg = list_entry(entry,
						struct ipmi_smi_msg,
						link);
#ifdef DEBUG_TIMING
		do_gettimeofday(&t);
		printk(KERN_DEBUG "**Start2: %d.%9.9d\n", t.tv_sec, t.tv_usec);
#endif
		err = atomic_notifier_call_chain(&xaction_notifier_list,
				0, smi_info);
		if (err & NOTIFY_STOP_MASK) {
			rv = SI_SM_CALL_WITHOUT_DELAY;
			goto out;
		}
		err = smi_info->handlers->start_transaction(
			smi_info->si_sm,
			smi_info->curr_msg->data,
			smi_info->curr_msg->data_size);
		if (err)
			return_hosed_msg(smi_info, err);

		rv = SI_SM_CALL_WITHOUT_DELAY;
	}
 out:
	return rv;
}

static void start_enable_irq(struct smi_info *smi_info)
{
	unsigned char msg[2];

	/*
                                                      
                    
  */
	msg[0] = (IPMI_NETFN_APP_REQUEST << 2);
	msg[1] = IPMI_GET_BMC_GLOBAL_ENABLES_CMD;

	smi_info->handlers->start_transaction(smi_info->si_sm, msg, 2);
	smi_info->si_state = SI_ENABLE_INTERRUPTS1;
}

static void start_disable_irq(struct smi_info *smi_info)
{
	unsigned char msg[2];

	msg[0] = (IPMI_NETFN_APP_REQUEST << 2);
	msg[1] = IPMI_GET_BMC_GLOBAL_ENABLES_CMD;

	smi_info->handlers->start_transaction(smi_info->si_sm, msg, 2);
	smi_info->si_state = SI_DISABLE_INTERRUPTS1;
}

static void start_clear_flags(struct smi_info *smi_info)
{
	unsigned char msg[3];

	/*                                                                */
	msg[0] = (IPMI_NETFN_APP_REQUEST << 2);
	msg[1] = IPMI_CLEAR_MSG_FLAGS_CMD;
	msg[2] = WDT_PRE_TIMEOUT_INT;

	smi_info->handlers->start_transaction(smi_info->si_sm, msg, 3);
	smi_info->si_state = SI_CLEARING_FLAGS;
}

static void smi_mod_timer(struct smi_info *smi_info, unsigned long new_val)
{
	smi_info->last_timeout_jiffies = jiffies;
	mod_timer(&smi_info->si_timer, new_val);
	smi_info->timer_running = true;
}

/*
                                                                 
                                                                      
                                                               
                                           
 */
static inline void disable_si_irq(struct smi_info *smi_info)
{
	if ((smi_info->irq) && (!smi_info->interrupt_disabled)) {
		start_disable_irq(smi_info);
		smi_info->interrupt_disabled = 1;
		if (!atomic_read(&smi_info->stop_operation))
			smi_mod_timer(smi_info, jiffies + SI_TIMEOUT_JIFFIES);
	}
}

static inline void enable_si_irq(struct smi_info *smi_info)
{
	if ((smi_info->irq) && (smi_info->interrupt_disabled)) {
		start_enable_irq(smi_info);
		smi_info->interrupt_disabled = 0;
	}
}

static void handle_flags(struct smi_info *smi_info)
{
 retry:
	if (smi_info->msg_flags & WDT_PRE_TIMEOUT_INT) {
		/*                      */
		smi_inc_stat(smi_info, watchdog_pretimeouts);

		start_clear_flags(smi_info);
		smi_info->msg_flags &= ~WDT_PRE_TIMEOUT_INT;
		ipmi_smi_watchdog_pretimeout(smi_info->intf);
	} else if (smi_info->msg_flags & RECEIVE_MSG_AVAIL) {
		/*                     */
		smi_info->curr_msg = ipmi_alloc_smi_msg();
		if (!smi_info->curr_msg) {
			disable_si_irq(smi_info);
			smi_info->si_state = SI_NORMAL;
			return;
		}
		enable_si_irq(smi_info);

		smi_info->curr_msg->data[0] = (IPMI_NETFN_APP_REQUEST << 2);
		smi_info->curr_msg->data[1] = IPMI_GET_MSG_CMD;
		smi_info->curr_msg->data_size = 2;

		smi_info->handlers->start_transaction(
			smi_info->si_sm,
			smi_info->curr_msg->data,
			smi_info->curr_msg->data_size);
		smi_info->si_state = SI_GETTING_MESSAGES;
	} else if (smi_info->msg_flags & EVENT_MSG_BUFFER_FULL) {
		/*                   */
		smi_info->curr_msg = ipmi_alloc_smi_msg();
		if (!smi_info->curr_msg) {
			disable_si_irq(smi_info);
			smi_info->si_state = SI_NORMAL;
			return;
		}
		enable_si_irq(smi_info);

		smi_info->curr_msg->data[0] = (IPMI_NETFN_APP_REQUEST << 2);
		smi_info->curr_msg->data[1] = IPMI_READ_EVENT_MSG_BUFFER_CMD;
		smi_info->curr_msg->data_size = 2;

		smi_info->handlers->start_transaction(
			smi_info->si_sm,
			smi_info->curr_msg->data,
			smi_info->curr_msg->data_size);
		smi_info->si_state = SI_GETTING_EVENTS;
	} else if (smi_info->msg_flags & OEM_DATA_AVAIL &&
		   smi_info->oem_data_avail_handler) {
		if (smi_info->oem_data_avail_handler(smi_info))
			goto retry;
	} else
		smi_info->si_state = SI_NORMAL;
}

static void handle_transaction_done(struct smi_info *smi_info)
{
	struct ipmi_smi_msg *msg;
#ifdef DEBUG_TIMING
	struct timeval t;

	do_gettimeofday(&t);
	printk(KERN_DEBUG "**Done: %d.%9.9d\n", t.tv_sec, t.tv_usec);
#endif
	switch (smi_info->si_state) {
	case SI_NORMAL:
		if (!smi_info->curr_msg)
			break;

		smi_info->curr_msg->rsp_size
			= smi_info->handlers->get_result(
				smi_info->si_sm,
				smi_info->curr_msg->rsp,
				IPMI_MAX_MSG_LENGTH);

		/*
                                                        
                                                     
                               
   */
		msg = smi_info->curr_msg;
		smi_info->curr_msg = NULL;
		deliver_recv_msg(smi_info, msg);
		break;

	case SI_GETTING_FLAGS:
	{
		unsigned char msg[4];
		unsigned int  len;

		/*                                                 */
		len = smi_info->handlers->get_result(smi_info->si_sm, msg, 4);
		if (msg[2] != 0) {
			/*                                             */
			smi_info->si_state = SI_NORMAL;
		} else if (len < 4) {
			/*
                                                     
                                   
    */
			smi_info->si_state = SI_NORMAL;
		} else {
			smi_info->msg_flags = msg[3];
			handle_flags(smi_info);
		}
		break;
	}

	case SI_CLEARING_FLAGS:
	case SI_CLEARING_FLAGS_THEN_SET_IRQ:
	{
		unsigned char msg[3];

		/*                       */
		smi_info->handlers->get_result(smi_info->si_sm, msg, 3);
		if (msg[2] != 0) {
			/*                      */
			dev_warn(smi_info->dev,
				 "Error clearing flags: %2.2x\n", msg[2]);
		}
		if (smi_info->si_state == SI_CLEARING_FLAGS_THEN_SET_IRQ)
			start_enable_irq(smi_info);
		else
			smi_info->si_state = SI_NORMAL;
		break;
	}

	case SI_GETTING_EVENTS:
	{
		smi_info->curr_msg->rsp_size
			= smi_info->handlers->get_result(
				smi_info->si_sm,
				smi_info->curr_msg->rsp,
				IPMI_MAX_MSG_LENGTH);

		/*
                                                        
                                                     
                               
   */
		msg = smi_info->curr_msg;
		smi_info->curr_msg = NULL;
		if (msg->rsp[2] != 0) {
			/*                                     */
			msg->done(msg);

			/*                          */
			smi_info->msg_flags &= ~EVENT_MSG_BUFFER_FULL;
			handle_flags(smi_info);
		} else {
			smi_inc_stat(smi_info, events);

			/*
                                           
                                                 
                                               
            
    */
			handle_flags(smi_info);

			deliver_recv_msg(smi_info, msg);
		}
		break;
	}

	case SI_GETTING_MESSAGES:
	{
		smi_info->curr_msg->rsp_size
			= smi_info->handlers->get_result(
				smi_info->si_sm,
				smi_info->curr_msg->rsp,
				IPMI_MAX_MSG_LENGTH);

		/*
                                                        
                                                     
                               
   */
		msg = smi_info->curr_msg;
		smi_info->curr_msg = NULL;
		if (msg->rsp[2] != 0) {
			/*                                     */
			msg->done(msg);

			/*                        */
			smi_info->msg_flags &= ~RECEIVE_MSG_AVAIL;
			handle_flags(smi_info);
		} else {
			smi_inc_stat(smi_info, incoming_messages);

			/*
                                           
                                                 
                                               
            
    */
			handle_flags(smi_info);

			deliver_recv_msg(smi_info, msg);
		}
		break;
	}

	case SI_ENABLE_INTERRUPTS1:
	{
		unsigned char msg[4];

		/*                                                 */
		smi_info->handlers->get_result(smi_info->si_sm, msg, 4);
		if (msg[2] != 0) {
			dev_warn(smi_info->dev,
				 "Couldn't get irq info: %x.\n", msg[2]);
			dev_warn(smi_info->dev,
				 "Maybe ok, but ipmi might run very slowly.\n");
			smi_info->si_state = SI_NORMAL;
		} else {
			msg[0] = (IPMI_NETFN_APP_REQUEST << 2);
			msg[1] = IPMI_SET_BMC_GLOBAL_ENABLES_CMD;
			msg[2] = (msg[3] |
				  IPMI_BMC_RCV_MSG_INTR |
				  IPMI_BMC_EVT_MSG_INTR);
			smi_info->handlers->start_transaction(
				smi_info->si_sm, msg, 3);
			smi_info->si_state = SI_ENABLE_INTERRUPTS2;
		}
		break;
	}

	case SI_ENABLE_INTERRUPTS2:
	{
		unsigned char msg[4];

		/*                                                 */
		smi_info->handlers->get_result(smi_info->si_sm, msg, 4);
		if (msg[2] != 0) {
			dev_warn(smi_info->dev,
				 "Couldn't set irq info: %x.\n", msg[2]);
			dev_warn(smi_info->dev,
				 "Maybe ok, but ipmi might run very slowly.\n");
		} else
			smi_info->interrupt_disabled = 0;
		smi_info->si_state = SI_NORMAL;
		break;
	}

	case SI_DISABLE_INTERRUPTS1:
	{
		unsigned char msg[4];

		/*                                                 */
		smi_info->handlers->get_result(smi_info->si_sm, msg, 4);
		if (msg[2] != 0) {
			dev_warn(smi_info->dev, "Could not disable interrupts"
				 ", failed get.\n");
			smi_info->si_state = SI_NORMAL;
		} else {
			msg[0] = (IPMI_NETFN_APP_REQUEST << 2);
			msg[1] = IPMI_SET_BMC_GLOBAL_ENABLES_CMD;
			msg[2] = (msg[3] &
				  ~(IPMI_BMC_RCV_MSG_INTR |
				    IPMI_BMC_EVT_MSG_INTR));
			smi_info->handlers->start_transaction(
				smi_info->si_sm, msg, 3);
			smi_info->si_state = SI_DISABLE_INTERRUPTS2;
		}
		break;
	}

	case SI_DISABLE_INTERRUPTS2:
	{
		unsigned char msg[4];

		/*                                                 */
		smi_info->handlers->get_result(smi_info->si_sm, msg, 4);
		if (msg[2] != 0) {
			dev_warn(smi_info->dev, "Could not disable interrupts"
				 ", failed set.\n");
		}
		smi_info->si_state = SI_NORMAL;
		break;
	}
	}
}

/*
                                                                   
                                                             
                                        
 */
static enum si_sm_result smi_event_handler(struct smi_info *smi_info,
					   int time)
{
	enum si_sm_result si_sm_result;

 restart:
	/*
                                                           
                                                          
                                                                
                                                             
                                                          
                                 
  */
	si_sm_result = smi_info->handlers->event(smi_info->si_sm, time);
	time = 0;
	while (si_sm_result == SI_SM_CALL_WITHOUT_DELAY)
		si_sm_result = smi_info->handlers->event(smi_info->si_sm, 0);

	if (si_sm_result == SI_SM_TRANSACTION_COMPLETE) {
		smi_inc_stat(smi_info, complete_transactions);

		handle_transaction_done(smi_info);
		si_sm_result = smi_info->handlers->event(smi_info->si_sm, 0);
	} else if (si_sm_result == SI_SM_HOSED) {
		smi_inc_stat(smi_info, hosed_count);

		/*
                                                 
                       
   */
		smi_info->si_state = SI_NORMAL;
		if (smi_info->curr_msg != NULL) {
			/*
                                                
                                              
                              
    */
			return_hosed_msg(smi_info, IPMI_ERR_UNSPECIFIED);
		}
		si_sm_result = smi_info->handlers->event(smi_info->si_sm, 0);
	}

	/*
                                                            
                                                               
  */
	if (likely(smi_info->intf) && si_sm_result == SI_SM_ATTN) {
		unsigned char msg[2];

		smi_inc_stat(smi_info, attentions);

		/*
                                                     
                                                     
                                                
                                                    
              
   */
		msg[0] = (IPMI_NETFN_APP_REQUEST << 2);
		msg[1] = IPMI_GET_MSG_FLAGS_CMD;

		smi_info->handlers->start_transaction(
			smi_info->si_sm, msg, 2);
		smi_info->si_state = SI_GETTING_FLAGS;
		goto restart;
	}

	/*                                                          */
	if (si_sm_result == SI_SM_IDLE) {
		smi_inc_stat(smi_info, idles);

		si_sm_result = start_next_msg(smi_info);
		if (si_sm_result != SI_SM_IDLE)
			goto restart;
	}

	if ((si_sm_result == SI_SM_IDLE)
	    && (atomic_read(&smi_info->req_events))) {
		/*
                                                           
                      
   */
		atomic_set(&smi_info->req_events, 0);

		smi_info->curr_msg = ipmi_alloc_smi_msg();
		if (!smi_info->curr_msg)
			goto out;

		smi_info->curr_msg->data[0] = (IPMI_NETFN_APP_REQUEST << 2);
		smi_info->curr_msg->data[1] = IPMI_READ_EVENT_MSG_BUFFER_CMD;
		smi_info->curr_msg->data_size = 2;

		smi_info->handlers->start_transaction(
			smi_info->si_sm,
			smi_info->curr_msg->data,
			smi_info->curr_msg->data_size);
		smi_info->si_state = SI_GETTING_EVENTS;
		goto restart;
	}
 out:
	return si_sm_result;
}

static void sender(void                *send_info,
		   struct ipmi_smi_msg *msg,
		   int                 priority)
{
	struct smi_info   *smi_info = send_info;
	enum si_sm_result result;
	unsigned long     flags;
#ifdef DEBUG_TIMING
	struct timeval    t;
#endif

	if (atomic_read(&smi_info->stop_operation)) {
		msg->rsp[0] = msg->data[0] | 4;
		msg->rsp[1] = msg->data[1];
		msg->rsp[2] = IPMI_ERR_UNSPECIFIED;
		msg->rsp_size = 3;
		deliver_recv_msg(smi_info, msg);
		return;
	}

#ifdef DEBUG_TIMING
	do_gettimeofday(&t);
	printk("**Enqueue: %d.%9.9d\n", t.tv_sec, t.tv_usec);
#endif

	if (smi_info->run_to_completion) {
		/*
                                                      
                                                      
                                          
   */

		/*
                                                       
                    
   */
		list_add_tail(&(msg->link), &(smi_info->xmit_msgs));

		result = smi_event_handler(smi_info, 0);
		while (result != SI_SM_IDLE) {
			udelay(SI_SHORT_TIMEOUT_USEC);
			result = smi_event_handler(smi_info,
						   SI_SHORT_TIMEOUT_USEC);
		}
		return;
	}

	spin_lock_irqsave(&smi_info->si_lock, flags);
	if (priority > 0)
		list_add_tail(&msg->link, &smi_info->hp_xmit_msgs);
	else
		list_add_tail(&msg->link, &smi_info->xmit_msgs);

	if (smi_info->si_state == SI_NORMAL && smi_info->curr_msg == NULL) {
		smi_mod_timer(smi_info, jiffies + SI_TIMEOUT_JIFFIES);

		if (smi_info->thread)
			wake_up_process(smi_info->thread);

		start_next_msg(smi_info);
		smi_event_handler(smi_info, 0);
	}
	spin_unlock_irqrestore(&smi_info->si_lock, flags);
}

static void set_run_to_completion(void *send_info, int i_run_to_completion)
{
	struct smi_info   *smi_info = send_info;
	enum si_sm_result result;

	smi_info->run_to_completion = i_run_to_completion;
	if (i_run_to_completion) {
		result = smi_event_handler(smi_info, 0);
		while (result != SI_SM_IDLE) {
			udelay(SI_SHORT_TIMEOUT_USEC);
			result = smi_event_handler(smi_info,
						   SI_SHORT_TIMEOUT_USEC);
		}
	}
}

/*
                                                                     
                                                                   
                 
 */
static inline void ipmi_si_set_not_busy(struct timespec *ts)
{
	ts->tv_nsec = -1;
}
static inline int ipmi_si_is_busy(struct timespec *ts)
{
	return ts->tv_nsec != -1;
}

static int ipmi_thread_busy_wait(enum si_sm_result smi_result,
				 const struct smi_info *smi_info,
				 struct timespec *busy_until)
{
	unsigned int max_busy_us = 0;

	if (smi_info->intf_num < num_max_busy_us)
		max_busy_us = kipmid_max_busy_us[smi_info->intf_num];
	if (max_busy_us == 0 || smi_result != SI_SM_CALL_WITH_DELAY)
		ipmi_si_set_not_busy(busy_until);
	else if (!ipmi_si_is_busy(busy_until)) {
		getnstimeofday(busy_until);
		timespec_add_ns(busy_until, max_busy_us*NSEC_PER_USEC);
	} else {
		struct timespec now;
		getnstimeofday(&now);
		if (unlikely(timespec_compare(&now, busy_until) > 0)) {
			ipmi_si_set_not_busy(busy_until);
			return 0;
		}
	}
	return 1;
}


/*
                                                      
  
                                                                    
                                                                  
                                                                   
                                                                   
                                      
 */
static int ipmi_thread(void *data)
{
	struct smi_info *smi_info = data;
	unsigned long flags;
	enum si_sm_result smi_result;
	struct timespec busy_until;

	ipmi_si_set_not_busy(&busy_until);
	set_user_nice(current, 19);
	while (!kthread_should_stop()) {
		int busy_wait;

		spin_lock_irqsave(&(smi_info->si_lock), flags);
		smi_result = smi_event_handler(smi_info, 0);

		/*
                                                          
                                                         
                                                       
                                                      
                                              
   */
		if (smi_result != SI_SM_IDLE && !smi_info->timer_running)
			smi_mod_timer(smi_info, jiffies + SI_TIMEOUT_JIFFIES);

		spin_unlock_irqrestore(&(smi_info->si_lock), flags);
		busy_wait = ipmi_thread_busy_wait(smi_result, smi_info,
						  &busy_until);
		if (smi_result == SI_SM_CALL_WITHOUT_DELAY)
			; /*            */
		else if (smi_result == SI_SM_CALL_WITH_DELAY && busy_wait)
			schedule();
		else if (smi_result == SI_SM_IDLE)
			schedule_timeout_interruptible(100);
		else
			schedule_timeout_interruptible(1);
	}
	return 0;
}


static void poll(void *send_info)
{
	struct smi_info *smi_info = send_info;
	unsigned long flags = 0;
	int run_to_completion = smi_info->run_to_completion;

	/*
                                                            
                                          
  */
	udelay(10);
	if (!run_to_completion)
		spin_lock_irqsave(&smi_info->si_lock, flags);
	smi_event_handler(smi_info, 10);
	if (!run_to_completion)
		spin_unlock_irqrestore(&smi_info->si_lock, flags);
}

static void request_events(void *send_info)
{
	struct smi_info *smi_info = send_info;

	if (atomic_read(&smi_info->stop_operation) ||
				!smi_info->has_event_buffer)
		return;

	atomic_set(&smi_info->req_events, 1);
}

static int initialized;

static void smi_timeout(unsigned long data)
{
	struct smi_info   *smi_info = (struct smi_info *) data;
	enum si_sm_result smi_result;
	unsigned long     flags;
	unsigned long     jiffies_now;
	long              time_diff;
	long		  timeout;
#ifdef DEBUG_TIMING
	struct timeval    t;
#endif

	spin_lock_irqsave(&(smi_info->si_lock), flags);
#ifdef DEBUG_TIMING
	do_gettimeofday(&t);
	printk(KERN_DEBUG "**Timer: %d.%9.9d\n", t.tv_sec, t.tv_usec);
#endif
	jiffies_now = jiffies;
	time_diff = (((long)jiffies_now - (long)smi_info->last_timeout_jiffies)
		     * SI_USEC_PER_JIFFY);
	smi_result = smi_event_handler(smi_info, time_diff);

	if ((smi_info->irq) && (!smi_info->interrupt_disabled)) {
		/*                                                 */
		timeout = jiffies + SI_TIMEOUT_JIFFIES;
		smi_inc_stat(smi_info, long_timeouts);
		goto do_mod_timer;
	}

	/*
                                                             
                      
  */
	if (smi_result == SI_SM_CALL_WITH_DELAY) {
		smi_inc_stat(smi_info, short_timeouts);
		timeout = jiffies + 1;
	} else {
		smi_inc_stat(smi_info, long_timeouts);
		timeout = jiffies + SI_TIMEOUT_JIFFIES;
	}

 do_mod_timer:
	if (smi_result != SI_SM_IDLE)
		smi_mod_timer(smi_info, timeout);
	else
		smi_info->timer_running = false;
	spin_unlock_irqrestore(&(smi_info->si_lock), flags);
}

static irqreturn_t si_irq_handler(int irq, void *data)
{
	struct smi_info *smi_info = data;
	unsigned long   flags;
#ifdef DEBUG_TIMING
	struct timeval  t;
#endif

	spin_lock_irqsave(&(smi_info->si_lock), flags);

	smi_inc_stat(smi_info, interrupts);

#ifdef DEBUG_TIMING
	do_gettimeofday(&t);
	printk(KERN_DEBUG "**Interrupt: %d.%9.9d\n", t.tv_sec, t.tv_usec);
#endif
	smi_event_handler(smi_info, 0);
	spin_unlock_irqrestore(&(smi_info->si_lock), flags);
	return IRQ_HANDLED;
}

static irqreturn_t si_bt_irq_handler(int irq, void *data)
{
	struct smi_info *smi_info = data;
	/*                                                     */
	smi_info->io.outputb(&smi_info->io, IPMI_BT_INTMASK_REG,
			     IPMI_BT_INTMASK_CLEAR_IRQ_BIT
			     | IPMI_BT_INTMASK_ENABLE_IRQ_BIT);
	return si_irq_handler(irq, data);
}

static int smi_start_processing(void       *send_info,
				ipmi_smi_t intf)
{
	struct smi_info *new_smi = send_info;
	int             enable = 0;

	new_smi->intf = intf;

	/*                              */
	if (new_smi->irq_setup)
		new_smi->irq_setup(new_smi);

	/*                                             */
	setup_timer(&new_smi->si_timer, smi_timeout, (long)new_smi);
	smi_mod_timer(new_smi, jiffies + SI_TIMEOUT_JIFFIES);

	/*
                                                    
  */
	if (new_smi->intf_num < num_force_kipmid)
		enable = force_kipmid[new_smi->intf_num];
	/*
                                                              
                                                            
  */
	else if ((new_smi->si_type != SI_BT) && (!new_smi->irq))
		enable = 1;

	if (enable) {
		new_smi->thread = kthread_run(ipmi_thread, new_smi,
					      "kipmi%d", new_smi->intf_num);
		if (IS_ERR(new_smi->thread)) {
			dev_notice(new_smi->dev, "Could not start"
				   " kernel thread due to error %ld, only using"
				   " timers to drive the interface\n",
				   PTR_ERR(new_smi->thread));
			new_smi->thread = NULL;
		}
	}

	return 0;
}

static int get_smi_info(void *send_info, struct ipmi_smi_info *data)
{
	struct smi_info *smi = send_info;

	data->addr_src = smi->addr_source;
	data->dev = smi->dev;
	data->addr_info = smi->addr_info;
	get_device(smi->dev);

	return 0;
}

static void set_maintenance_mode(void *send_info, int enable)
{
	struct smi_info   *smi_info = send_info;

	if (!enable)
		atomic_set(&smi_info->req_events, 0);
}

static struct ipmi_smi_handlers handlers = {
	.owner                  = THIS_MODULE,
	.start_processing       = smi_start_processing,
	.get_smi_info		= get_smi_info,
	.sender			= sender,
	.request_events		= request_events,
	.set_maintenance_mode   = set_maintenance_mode,
	.set_run_to_completion  = set_run_to_completion,
	.poll			= poll,
};

/*
                                                                         
                                                                         
 */

static LIST_HEAD(smi_infos);
static DEFINE_MUTEX(smi_infos_lock);
static int smi_num; /*                           */

#define DEFAULT_REGSPACING	1
#define DEFAULT_REGSIZE		1

#ifdef CONFIG_ACPI
static bool          si_tryacpi = 1;
#endif
#ifdef CONFIG_DMI
static bool          si_trydmi = 1;
#endif
static bool          si_tryplatform = 1;
#ifdef CONFIG_PCI
static bool          si_trypci = 1;
#endif
static bool          si_trydefaults = 1;
static char          *si_type[SI_MAX_PARMS];
#define MAX_SI_TYPE_STR 30
static char          si_type_str[MAX_SI_TYPE_STR];
static unsigned long addrs[SI_MAX_PARMS];
static unsigned int num_addrs;
static unsigned int  ports[SI_MAX_PARMS];
static unsigned int num_ports;
static int           irqs[SI_MAX_PARMS];
static unsigned int num_irqs;
static int           regspacings[SI_MAX_PARMS];
static unsigned int num_regspacings;
static int           regsizes[SI_MAX_PARMS];
static unsigned int num_regsizes;
static int           regshifts[SI_MAX_PARMS];
static unsigned int num_regshifts;
static int slave_addrs[SI_MAX_PARMS]; /*                                     */
static unsigned int num_slave_addrs;

#define IPMI_IO_ADDR_SPACE  0
#define IPMI_MEM_ADDR_SPACE 1
static char *addr_space_to_str[] = { "i/o", "mem" };

static int hotmod_handler(const char *val, struct kernel_param *kp);

module_param_call(hotmod, hotmod_handler, NULL, NULL, 0200);
MODULE_PARM_DESC(hotmod, "Add and remove interfaces.  See"
		 " Documentation/IPMI.txt in the kernel sources for the"
		 " gory details.");

#ifdef CONFIG_ACPI
module_param_named(tryacpi, si_tryacpi, bool, 0);
MODULE_PARM_DESC(tryacpi, "Setting this to zero will disable the"
		 " default scan of the interfaces identified via ACPI");
#endif
#ifdef CONFIG_DMI
module_param_named(trydmi, si_trydmi, bool, 0);
MODULE_PARM_DESC(trydmi, "Setting this to zero will disable the"
		 " default scan of the interfaces identified via DMI");
#endif
module_param_named(tryplatform, si_tryplatform, bool, 0);
MODULE_PARM_DESC(tryacpi, "Setting this to zero will disable the"
		 " default scan of the interfaces identified via platform"
		 " interfaces like openfirmware");
#ifdef CONFIG_PCI
module_param_named(trypci, si_trypci, bool, 0);
MODULE_PARM_DESC(tryacpi, "Setting this to zero will disable the"
		 " default scan of the interfaces identified via pci");
#endif
module_param_named(trydefaults, si_trydefaults, bool, 0);
MODULE_PARM_DESC(trydefaults, "Setting this to 'false' will disable the"
		 " default scan of the KCS and SMIC interface at the standard"
		 " address");
module_param_string(type, si_type_str, MAX_SI_TYPE_STR, 0);
MODULE_PARM_DESC(type, "Defines the type of each interface, each"
		 " interface separated by commas.  The types are 'kcs',"
		 " 'smic', and 'bt'.  For example si_type=kcs,bt will set"
		 " the first interface to kcs and the second to bt");
module_param_array(addrs, ulong, &num_addrs, 0);
MODULE_PARM_DESC(addrs, "Sets the memory address of each interface, the"
		 " addresses separated by commas.  Only use if an interface"
		 " is in memory.  Otherwise, set it to zero or leave"
		 " it blank.");
module_param_array(ports, uint, &num_ports, 0);
MODULE_PARM_DESC(ports, "Sets the port address of each interface, the"
		 " addresses separated by commas.  Only use if an interface"
		 " is a port.  Otherwise, set it to zero or leave"
		 " it blank.");
module_param_array(irqs, int, &num_irqs, 0);
MODULE_PARM_DESC(irqs, "Sets the interrupt of each interface, the"
		 " addresses separated by commas.  Only use if an interface"
		 " has an interrupt.  Otherwise, set it to zero or leave"
		 " it blank.");
module_param_array(regspacings, int, &num_regspacings, 0);
MODULE_PARM_DESC(regspacings, "The number of bytes between the start address"
		 " and each successive register used by the interface.  For"
		 " instance, if the start address is 0xca2 and the spacing"
		 " is 2, then the second address is at 0xca4.  Defaults"
		 " to 1.");
module_param_array(regsizes, int, &num_regsizes, 0);
MODULE_PARM_DESC(regsizes, "The size of the specific IPMI register in bytes."
		 " This should generally be 1, 2, 4, or 8 for an 8-bit,"
		 " 16-bit, 32-bit, or 64-bit register.  Use this if you"
		 " the 8-bit IPMI register has to be read from a larger"
		 " register.");
module_param_array(regshifts, int, &num_regshifts, 0);
MODULE_PARM_DESC(regshifts, "The amount to shift the data read from the."
		 " IPMI register, in bits.  For instance, if the data"
		 " is read from a 32-bit word and the IPMI data is in"
		 " bit 8-15, then the shift would be 8");
module_param_array(slave_addrs, int, &num_slave_addrs, 0);
MODULE_PARM_DESC(slave_addrs, "Set the default IPMB slave address for"
		 " the controller.  Normally this is 0x20, but can be"
		 " overridden by this parm.  This is an array indexed"
		 " by interface number.");
module_param_array(force_kipmid, int, &num_force_kipmid, 0);
MODULE_PARM_DESC(force_kipmid, "Force the kipmi daemon to be enabled (1) or"
		 " disabled(0).  Normally the IPMI driver auto-detects"
		 " this, but the value may be overridden by this parm.");
module_param(unload_when_empty, int, 0);
MODULE_PARM_DESC(unload_when_empty, "Unload the module if no interfaces are"
		 " specified or found, default is 1.  Setting to 0"
		 " is useful for hot add of devices using hotmod.");
module_param_array(kipmid_max_busy_us, uint, &num_max_busy_us, 0644);
MODULE_PARM_DESC(kipmid_max_busy_us,
		 "Max time (in microseconds) to busy-wait for IPMI data before"
		 " sleeping. 0 (default) means to wait forever. Set to 100-500"
		 " if kipmid is using up a lot of CPU time.");


static void std_irq_cleanup(struct smi_info *info)
{
	if (info->si_type == SI_BT)
		/*                                            */
		info->io.outputb(&info->io, IPMI_BT_INTMASK_REG, 0);
	free_irq(info->irq, info);
}

static int std_irq_setup(struct smi_info *info)
{
	int rv;

	if (!info->irq)
		return 0;

	if (info->si_type == SI_BT) {
		rv = request_irq(info->irq,
				 si_bt_irq_handler,
				 IRQF_SHARED | IRQF_DISABLED,
				 DEVICE_NAME,
				 info);
		if (!rv)
			/*                                           */
			info->io.outputb(&info->io, IPMI_BT_INTMASK_REG,
					 IPMI_BT_INTMASK_ENABLE_IRQ_BIT);
	} else
		rv = request_irq(info->irq,
				 si_irq_handler,
				 IRQF_SHARED | IRQF_DISABLED,
				 DEVICE_NAME,
				 info);
	if (rv) {
		dev_warn(info->dev, "%s unable to claim interrupt %d,"
			 " running polled\n",
			 DEVICE_NAME, info->irq);
		info->irq = 0;
	} else {
		info->irq_cleanup = std_irq_cleanup;
		dev_info(info->dev, "Using irq %d\n", info->irq);
	}

	return rv;
}

static unsigned char port_inb(struct si_sm_io *io, unsigned int offset)
{
	unsigned int addr = io->addr_data;

	return inb(addr + (offset * io->regspacing));
}

static void port_outb(struct si_sm_io *io, unsigned int offset,
		      unsigned char b)
{
	unsigned int addr = io->addr_data;

	outb(b, addr + (offset * io->regspacing));
}

static unsigned char port_inw(struct si_sm_io *io, unsigned int offset)
{
	unsigned int addr = io->addr_data;

	return (inw(addr + (offset * io->regspacing)) >> io->regshift) & 0xff;
}

static void port_outw(struct si_sm_io *io, unsigned int offset,
		      unsigned char b)
{
	unsigned int addr = io->addr_data;

	outw(b << io->regshift, addr + (offset * io->regspacing));
}

static unsigned char port_inl(struct si_sm_io *io, unsigned int offset)
{
	unsigned int addr = io->addr_data;

	return (inl(addr + (offset * io->regspacing)) >> io->regshift) & 0xff;
}

static void port_outl(struct si_sm_io *io, unsigned int offset,
		      unsigned char b)
{
	unsigned int addr = io->addr_data;

	outl(b << io->regshift, addr+(offset * io->regspacing));
}

static void port_cleanup(struct smi_info *info)
{
	unsigned int addr = info->io.addr_data;
	int          idx;

	if (addr) {
		for (idx = 0; idx < info->io_size; idx++)
			release_region(addr + idx * info->io.regspacing,
				       info->io.regsize);
	}
}

static int port_setup(struct smi_info *info)
{
	unsigned int addr = info->io.addr_data;
	int          idx;

	if (!addr)
		return -ENODEV;

	info->io_cleanup = port_cleanup;

	/*
                                                              
                           
  */
	switch (info->io.regsize) {
	case 1:
		info->io.inputb = port_inb;
		info->io.outputb = port_outb;
		break;
	case 2:
		info->io.inputb = port_inw;
		info->io.outputb = port_outw;
		break;
	case 4:
		info->io.inputb = port_inl;
		info->io.outputb = port_outl;
		break;
	default:
		dev_warn(info->dev, "Invalid register size: %d\n",
			 info->io.regsize);
		return -EINVAL;
	}

	/*
                                                          
                                                             
                                                           
                    
  */
	for (idx = 0; idx < info->io_size; idx++) {
		if (request_region(addr + idx * info->io.regspacing,
				   info->io.regsize, DEVICE_NAME) == NULL) {
			/*                  */
			while (idx--) {
				release_region(addr + idx * info->io.regspacing,
					       info->io.regsize);
			}
			return -EIO;
		}
	}
	return 0;
}

static unsigned char intf_mem_inb(struct si_sm_io *io, unsigned int offset)
{
	return readb((io->addr)+(offset * io->regspacing));
}

static void intf_mem_outb(struct si_sm_io *io, unsigned int offset,
		     unsigned char b)
{
	writeb(b, (io->addr)+(offset * io->regspacing));
}

static unsigned char intf_mem_inw(struct si_sm_io *io, unsigned int offset)
{
	return (readw((io->addr)+(offset * io->regspacing)) >> io->regshift)
		& 0xff;
}

static void intf_mem_outw(struct si_sm_io *io, unsigned int offset,
		     unsigned char b)
{
	writeb(b << io->regshift, (io->addr)+(offset * io->regspacing));
}

static unsigned char intf_mem_inl(struct si_sm_io *io, unsigned int offset)
{
	return (readl((io->addr)+(offset * io->regspacing)) >> io->regshift)
		& 0xff;
}

static void intf_mem_outl(struct si_sm_io *io, unsigned int offset,
		     unsigned char b)
{
	writel(b << io->regshift, (io->addr)+(offset * io->regspacing));
}

#ifdef readq
static unsigned char mem_inq(struct si_sm_io *io, unsigned int offset)
{
	return (readq((io->addr)+(offset * io->regspacing)) >> io->regshift)
		& 0xff;
}

static void mem_outq(struct si_sm_io *io, unsigned int offset,
		     unsigned char b)
{
	writeq(b << io->regshift, (io->addr)+(offset * io->regspacing));
}
#endif

static void mem_cleanup(struct smi_info *info)
{
	unsigned long addr = info->io.addr_data;
	int           mapsize;

	if (info->io.addr) {
		iounmap(info->io.addr);

		mapsize = ((info->io_size * info->io.regspacing)
			   - (info->io.regspacing - info->io.regsize));

		release_mem_region(addr, mapsize);
	}
}

static int mem_setup(struct smi_info *info)
{
	unsigned long addr = info->io.addr_data;
	int           mapsize;

	if (!addr)
		return -ENODEV;

	info->io_cleanup = mem_cleanup;

	/*
                                                                    
                           
  */
	switch (info->io.regsize) {
	case 1:
		info->io.inputb = intf_mem_inb;
		info->io.outputb = intf_mem_outb;
		break;
	case 2:
		info->io.inputb = intf_mem_inw;
		info->io.outputb = intf_mem_outw;
		break;
	case 4:
		info->io.inputb = intf_mem_inl;
		info->io.outputb = intf_mem_outl;
		break;
#ifdef readq
	case 8:
		info->io.inputb = mem_inq;
		info->io.outputb = mem_outq;
		break;
#endif
	default:
		dev_warn(info->dev, "Invalid register size: %d\n",
			 info->io.regsize);
		return -EINVAL;
	}

	/*
                                                              
                                                           
                                                         
                                                         
             
  */
	mapsize = ((info->io_size * info->io.regspacing)
		   - (info->io.regspacing - info->io.regsize));

	if (request_mem_region(addr, mapsize, DEVICE_NAME) == NULL)
		return -EIO;

	info->io.addr = ioremap(addr, mapsize);
	if (info->io.addr == NULL) {
		release_mem_region(addr, mapsize);
		return -EIO;
	}
	return 0;
}

/*
                                                   
                                                                     
               
                     
                  
                   
              
                     
 */
enum hotmod_op { HM_ADD, HM_REMOVE };
struct hotmod_vals {
	char *name;
	int  val;
};
static struct hotmod_vals hotmod_ops[] = {
	{ "add",	HM_ADD },
	{ "remove",	HM_REMOVE },
	{ NULL }
};
static struct hotmod_vals hotmod_si[] = {
	{ "kcs",	SI_KCS },
	{ "smic",	SI_SMIC },
	{ "bt",		SI_BT },
	{ NULL }
};
static struct hotmod_vals hotmod_as[] = {
	{ "mem",	IPMI_MEM_ADDR_SPACE },
	{ "i/o",	IPMI_IO_ADDR_SPACE },
	{ NULL }
};

static int parse_str(struct hotmod_vals *v, int *val, char *name, char **curr)
{
	char *s;
	int  i;

	s = strchr(*curr, ',');
	if (!s) {
		printk(KERN_WARNING PFX "No hotmod %s given.\n", name);
		return -EINVAL;
	}
	*s = '\0';
	s++;
	for (i = 0; hotmod_ops[i].name; i++) {
		if (strcmp(*curr, v[i].name) == 0) {
			*val = v[i].val;
			*curr = s;
			return 0;
		}
	}

	printk(KERN_WARNING PFX "Invalid hotmod %s '%s'\n", name, *curr);
	return -EINVAL;
}

static int check_hotmod_int_op(const char *curr, const char *option,
			       const char *name, int *val)
{
	char *n;

	if (strcmp(curr, name) == 0) {
		if (!option) {
			printk(KERN_WARNING PFX
			       "No option given for '%s'\n",
			       curr);
			return -EINVAL;
		}
		*val = simple_strtoul(option, &n, 0);
		if ((*n != '\0') || (*option == '\0')) {
			printk(KERN_WARNING PFX
			       "Bad option given for '%s'\n",
			       curr);
			return -EINVAL;
		}
		return 1;
	}
	return 0;
}

static struct smi_info *smi_info_alloc(void)
{
	struct smi_info *info = kzalloc(sizeof(*info), GFP_KERNEL);

	if (info)
		spin_lock_init(&info->si_lock);
	return info;
}

static int hotmod_handler(const char *val, struct kernel_param *kp)
{
	char *str = kstrdup(val, GFP_KERNEL);
	int  rv;
	char *next, *curr, *s, *n, *o;
	enum hotmod_op op;
	enum si_type si_type;
	int  addr_space;
	unsigned long addr;
	int regspacing;
	int regsize;
	int regshift;
	int irq;
	int ipmb;
	int ival;
	int len;
	struct smi_info *info;

	if (!str)
		return -ENOMEM;

	/*                                                           */
	len = strlen(str);
	ival = len - 1;
	while ((ival >= 0) && isspace(str[ival])) {
		str[ival] = '\0';
		ival--;
	}

	for (curr = str; curr; curr = next) {
		regspacing = 1;
		regsize = 1;
		regshift = 0;
		irq = 0;
		ipmb = 0; /*                                     */

		next = strchr(curr, ':');
		if (next) {
			*next = '\0';
			next++;
		}

		rv = parse_str(hotmod_ops, &ival, "operation", &curr);
		if (rv)
			break;
		op = ival;

		rv = parse_str(hotmod_si, &ival, "interface type", &curr);
		if (rv)
			break;
		si_type = ival;

		rv = parse_str(hotmod_as, &addr_space, "address space", &curr);
		if (rv)
			break;

		s = strchr(curr, ',');
		if (s) {
			*s = '\0';
			s++;
		}
		addr = simple_strtoul(curr, &n, 0);
		if ((*n != '\0') || (*curr == '\0')) {
			printk(KERN_WARNING PFX "Invalid hotmod address"
			       " '%s'\n", curr);
			break;
		}

		while (s) {
			curr = s;
			s = strchr(curr, ',');
			if (s) {
				*s = '\0';
				s++;
			}
			o = strchr(curr, '=');
			if (o) {
				*o = '\0';
				o++;
			}
			rv = check_hotmod_int_op(curr, o, "rsp", &regspacing);
			if (rv < 0)
				goto out;
			else if (rv)
				continue;
			rv = check_hotmod_int_op(curr, o, "rsi", &regsize);
			if (rv < 0)
				goto out;
			else if (rv)
				continue;
			rv = check_hotmod_int_op(curr, o, "rsh", &regshift);
			if (rv < 0)
				goto out;
			else if (rv)
				continue;
			rv = check_hotmod_int_op(curr, o, "irq", &irq);
			if (rv < 0)
				goto out;
			else if (rv)
				continue;
			rv = check_hotmod_int_op(curr, o, "ipmb", &ipmb);
			if (rv < 0)
				goto out;
			else if (rv)
				continue;

			rv = -EINVAL;
			printk(KERN_WARNING PFX
			       "Invalid hotmod option '%s'\n",
			       curr);
			goto out;
		}

		if (op == HM_ADD) {
			info = smi_info_alloc();
			if (!info) {
				rv = -ENOMEM;
				goto out;
			}

			info->addr_source = SI_HOTMOD;
			info->si_type = si_type;
			info->io.addr_data = addr;
			info->io.addr_type = addr_space;
			if (addr_space == IPMI_MEM_ADDR_SPACE)
				info->io_setup = mem_setup;
			else
				info->io_setup = port_setup;

			info->io.addr = NULL;
			info->io.regspacing = regspacing;
			if (!info->io.regspacing)
				info->io.regspacing = DEFAULT_REGSPACING;
			info->io.regsize = regsize;
			if (!info->io.regsize)
				info->io.regsize = DEFAULT_REGSPACING;
			info->io.regshift = regshift;
			info->irq = irq;
			if (info->irq)
				info->irq_setup = std_irq_setup;
			info->slave_addr = ipmb;

			if (!add_smi(info)) {
				if (try_smi_init(info))
					cleanup_one_si(info);
			} else {
				kfree(info);
			}
		} else {
			/*        */
			struct smi_info *e, *tmp_e;

			mutex_lock(&smi_infos_lock);
			list_for_each_entry_safe(e, tmp_e, &smi_infos, link) {
				if (e->io.addr_type != addr_space)
					continue;
				if (e->si_type != si_type)
					continue;
				if (e->io.addr_data == addr)
					cleanup_one_si(e);
			}
			mutex_unlock(&smi_infos_lock);
		}
	}
	rv = len;
 out:
	kfree(str);
	return rv;
}

static int hardcode_find_bmc(void)
{
	int ret = -ENODEV;
	int             i;
	struct smi_info *info;

	for (i = 0; i < SI_MAX_PARMS; i++) {
		if (!ports[i] && !addrs[i])
			continue;

		info = smi_info_alloc();
		if (!info)
			return -ENOMEM;

		info->addr_source = SI_HARDCODED;
		printk(KERN_INFO PFX "probing via hardcoded address\n");

		if (!si_type[i] || strcmp(si_type[i], "kcs") == 0) {
			info->si_type = SI_KCS;
		} else if (strcmp(si_type[i], "smic") == 0) {
			info->si_type = SI_SMIC;
		} else if (strcmp(si_type[i], "bt") == 0) {
			info->si_type = SI_BT;
		} else {
			printk(KERN_WARNING PFX "Interface type specified "
			       "for interface %d, was invalid: %s\n",
			       i, si_type[i]);
			kfree(info);
			continue;
		}

		if (ports[i]) {
			/*             */
			info->io_setup = port_setup;
			info->io.addr_data = ports[i];
			info->io.addr_type = IPMI_IO_ADDR_SPACE;
		} else if (addrs[i]) {
			/*               */
			info->io_setup = mem_setup;
			info->io.addr_data = addrs[i];
			info->io.addr_type = IPMI_MEM_ADDR_SPACE;
		} else {
			printk(KERN_WARNING PFX "Interface type specified "
			       "for interface %d, but port and address were "
			       "not set or set to zero.\n", i);
			kfree(info);
			continue;
		}

		info->io.addr = NULL;
		info->io.regspacing = regspacings[i];
		if (!info->io.regspacing)
			info->io.regspacing = DEFAULT_REGSPACING;
		info->io.regsize = regsizes[i];
		if (!info->io.regsize)
			info->io.regsize = DEFAULT_REGSPACING;
		info->io.regshift = regshifts[i];
		info->irq = irqs[i];
		if (info->irq)
			info->irq_setup = std_irq_setup;
		info->slave_addr = slave_addrs[i];

		if (!add_smi(info)) {
			if (try_smi_init(info))
				cleanup_one_si(info);
			ret = 0;
		} else {
			kfree(info);
		}
	}
	return ret;
}

#ifdef CONFIG_ACPI

#include <linux/acpi.h>

/*
                                                                    
                                                                      
               
 */
static int acpi_failure;

/*                          */
static u32 ipmi_acpi_gpe(acpi_handle gpe_device,
	u32 gpe_number, void *context)
{
	struct smi_info *smi_info = context;
	unsigned long   flags;
#ifdef DEBUG_TIMING
	struct timeval t;
#endif

	spin_lock_irqsave(&(smi_info->si_lock), flags);

	smi_inc_stat(smi_info, interrupts);

#ifdef DEBUG_TIMING
	do_gettimeofday(&t);
	printk("**ACPI_GPE: %d.%9.9d\n", t.tv_sec, t.tv_usec);
#endif
	smi_event_handler(smi_info, 0);
	spin_unlock_irqrestore(&(smi_info->si_lock), flags);

	return ACPI_INTERRUPT_HANDLED;
}

static void acpi_gpe_irq_cleanup(struct smi_info *info)
{
	if (!info->irq)
		return;

	acpi_remove_gpe_handler(NULL, info->irq, &ipmi_acpi_gpe);
}

static int acpi_gpe_irq_setup(struct smi_info *info)
{
	acpi_status status;

	if (!info->irq)
		return 0;

	/*                                   */
	status = acpi_install_gpe_handler(NULL,
					  info->irq,
					  ACPI_GPE_LEVEL_TRIGGERED,
					  &ipmi_acpi_gpe,
					  info);
	if (status != AE_OK) {
		dev_warn(info->dev, "%s unable to claim ACPI GPE %d,"
			 " running polled\n", DEVICE_NAME, info->irq);
		info->irq = 0;
		return -EINVAL;
	} else {
		info->irq_cleanup = acpi_gpe_irq_cleanup;
		dev_info(info->dev, "Using ACPI GPE %d\n", info->irq);
		return 0;
	}
}

/*
             
                                                                    
 */
struct SPMITable {
	s8	Signature[4];
	u32	Length;
	u8	Revision;
	u8	Checksum;
	s8	OEMID[6];
	s8	OEMTableID[8];
	s8	OEMRevision[4];
	s8	CreatorID[4];
	s8	CreatorRevision[4];
	u8	InterfaceType;
	u8	IPMIlegacy;
	s16	SpecificationRevision;

	/*
                                   
                          
  */
	u8	InterruptType;

	/*
                                                          
                                       
  */
	u8	GPE;

	s16	Reserved;

	/*
                                                          
                         
  */
	u32	GlobalSystemInterrupt;

	/*                              */
	struct acpi_generic_address addr;

	u8	UID[4];

	s8      spmi_id[1]; /*                                      */
};

static int try_init_spmi(struct SPMITable *spmi)
{
	struct smi_info  *info;

	if (spmi->IPMIlegacy != 1) {
		printk(KERN_INFO PFX "Bad SPMI legacy %d\n", spmi->IPMIlegacy);
		return -ENODEV;
	}

	info = smi_info_alloc();
	if (!info) {
		printk(KERN_ERR PFX "Could not allocate SI data (3)\n");
		return -ENOMEM;
	}

	info->addr_source = SI_SPMI;
	printk(KERN_INFO PFX "probing via SPMI\n");

	/*                                */
	switch (spmi->InterfaceType) {
	case 1:	/*     */
		info->si_type = SI_KCS;
		break;
	case 2:	/*      */
		info->si_type = SI_SMIC;
		break;
	case 3:	/*    */
		info->si_type = SI_BT;
		break;
	default:
		printk(KERN_INFO PFX "Unknown ACPI/SPMI SI type %d\n",
		       spmi->InterfaceType);
		kfree(info);
		return -EIO;
	}

	if (spmi->InterruptType & 1) {
		/*                            */
		info->irq = spmi->GPE;
		info->irq_setup = acpi_gpe_irq_setup;
	} else if (spmi->InterruptType & 2) {
		/*                                    */
		info->irq = spmi->GlobalSystemInterrupt;
		info->irq_setup = std_irq_setup;
	} else {
		/*                                    */
		info->irq = 0;
		info->irq_setup = NULL;
	}

	if (spmi->addr.bit_width) {
		/*                                                   */
		info->io.regspacing = spmi->addr.bit_width / 8;
	} else {
		info->io.regspacing = DEFAULT_REGSPACING;
	}
	info->io.regsize = info->io.regspacing;
	info->io.regshift = spmi->addr.bit_offset;

	if (spmi->addr.space_id == ACPI_ADR_SPACE_SYSTEM_MEMORY) {
		info->io_setup = mem_setup;
		info->io.addr_type = IPMI_MEM_ADDR_SPACE;
	} else if (spmi->addr.space_id == ACPI_ADR_SPACE_SYSTEM_IO) {
		info->io_setup = port_setup;
		info->io.addr_type = IPMI_IO_ADDR_SPACE;
	} else {
		kfree(info);
		printk(KERN_WARNING PFX "Unknown ACPI I/O Address type\n");
		return -EIO;
	}
	info->io.addr_data = spmi->addr.address;

	pr_info("ipmi_si: SPMI: %s %#lx regsize %d spacing %d irq %d\n",
		 (info->io.addr_type == IPMI_IO_ADDR_SPACE) ? "io" : "mem",
		 info->io.addr_data, info->io.regsize, info->io.regspacing,
		 info->irq);

	if (add_smi(info))
		kfree(info);

	return 0;
}

static void spmi_find_bmc(void)
{
	acpi_status      status;
	struct SPMITable *spmi;
	int              i;

	if (acpi_disabled)
		return;

	if (acpi_failure)
		return;

	for (i = 0; ; i++) {
		status = acpi_get_table(ACPI_SIG_SPMI, i+1,
					(struct acpi_table_header **)&spmi);
		if (status != AE_OK)
			return;

		try_init_spmi(spmi);
	}
}

static int ipmi_pnp_probe(struct pnp_dev *dev,
				    const struct pnp_device_id *dev_id)
{
	struct acpi_device *acpi_dev;
	struct smi_info *info;
	struct resource *res, *res_second;
	acpi_handle handle;
	acpi_status status;
	unsigned long long tmp;

	acpi_dev = pnp_acpi_device(dev);
	if (!acpi_dev)
		return -ENODEV;

	info = smi_info_alloc();
	if (!info)
		return -ENOMEM;

	info->addr_source = SI_ACPI;
	printk(KERN_INFO PFX "probing via ACPI\n");

	handle = acpi_dev->handle;
	info->addr_info.acpi_info.acpi_handle = handle;

	/*                                                */
	status = acpi_evaluate_integer(handle, "_IFT", NULL, &tmp);
	if (ACPI_FAILURE(status))
		goto err_free;

	switch (tmp) {
	case 1:
		info->si_type = SI_KCS;
		break;
	case 2:
		info->si_type = SI_SMIC;
		break;
	case 3:
		info->si_type = SI_BT;
		break;
	default:
		dev_info(&dev->dev, "unknown IPMI type %lld\n", tmp);
		goto err_free;
	}

	res = pnp_get_resource(dev, IORESOURCE_IO, 0);
	if (res) {
		info->io_setup = port_setup;
		info->io.addr_type = IPMI_IO_ADDR_SPACE;
	} else {
		res = pnp_get_resource(dev, IORESOURCE_MEM, 0);
		if (res) {
			info->io_setup = mem_setup;
			info->io.addr_type = IPMI_MEM_ADDR_SPACE;
		}
	}
	if (!res) {
		dev_err(&dev->dev, "no I/O or memory address\n");
		goto err_free;
	}
	info->io.addr_data = res->start;

	info->io.regspacing = DEFAULT_REGSPACING;
	res_second = pnp_get_resource(dev,
			       (info->io.addr_type == IPMI_IO_ADDR_SPACE) ?
					IORESOURCE_IO : IORESOURCE_MEM,
			       1);
	if (res_second) {
		if (res_second->start > info->io.addr_data)
			info->io.regspacing = res_second->start - info->io.addr_data;
	}
	info->io.regsize = DEFAULT_REGSPACING;
	info->io.regshift = 0;

	/*                                                           */
	status = acpi_evaluate_integer(handle, "_GPE", NULL, &tmp);
	if (ACPI_SUCCESS(status)) {
		info->irq = tmp;
		info->irq_setup = acpi_gpe_irq_setup;
	} else if (pnp_irq_valid(dev, 0)) {
		info->irq = pnp_irq(dev, 0);
		info->irq_setup = std_irq_setup;
	}

	info->dev = &dev->dev;
	pnp_set_drvdata(dev, info);

	dev_info(info->dev, "%pR regsize %d spacing %d irq %d\n",
		 res, info->io.regsize, info->io.regspacing,
		 info->irq);

	if (add_smi(info))
		goto err_free;

	return 0;

err_free:
	kfree(info);
	return -EINVAL;
}

static void ipmi_pnp_remove(struct pnp_dev *dev)
{
	struct smi_info *info = pnp_get_drvdata(dev);

	cleanup_one_si(info);
}

static const struct pnp_device_id pnp_dev_table[] = {
	{"IPI0001", 0},
	{"", 0},
};

static struct pnp_driver ipmi_pnp_driver = {
	.name		= DEVICE_NAME,
	.probe		= ipmi_pnp_probe,
	.remove		= ipmi_pnp_remove,
	.id_table	= pnp_dev_table,
};
#endif

#ifdef CONFIG_DMI
struct dmi_ipmi_data {
	u8   		type;
	u8   		addr_space;
	unsigned long	base_addr;
	u8   		irq;
	u8              offset;
	u8              slave_addr;
};

static int decode_dmi(const struct dmi_header *dm,
				struct dmi_ipmi_data *dmi)
{
	const u8	*data = (const u8 *)dm;
	unsigned long  	base_addr;
	u8		reg_spacing;
	u8              len = dm->length;

	dmi->type = data[4];

	memcpy(&base_addr, data+8, sizeof(unsigned long));
	if (len >= 0x11) {
		if (base_addr & 1) {
			/*     */
			base_addr &= 0xFFFE;
			dmi->addr_space = IPMI_IO_ADDR_SPACE;
		} else
			/*        */
			dmi->addr_space = IPMI_MEM_ADDR_SPACE;

		/*                                                           
             */
		dmi->base_addr = base_addr | ((data[0x10] & 0x10) >> 4);

		dmi->irq = data[0x11];

		/*                                                          */
		reg_spacing = (data[0x10] & 0xC0) >> 6;
		switch (reg_spacing) {
		case 0x00: /*                 */
		    dmi->offset = 1;
		    break;
		case 0x01: /*                   */
		    dmi->offset = 4;
		    break;
		case 0x02: /*                    */
		    dmi->offset = 16;
		    break;
		default:
		    /*                                       */
		    return -EIO;
		}
	} else {
		/*               */
		/*
                                                     
                                                       
                                                        
                                                        
                                                      
                                              
   */
		dmi->base_addr = base_addr & 0xfffe;
		dmi->addr_space = IPMI_IO_ADDR_SPACE;
		dmi->offset = 1;
	}

	dmi->slave_addr = data[6];

	return 0;
}

static void try_init_dmi(struct dmi_ipmi_data *ipmi_data)
{
	struct smi_info *info;

	info = smi_info_alloc();
	if (!info) {
		printk(KERN_ERR PFX "Could not allocate SI data\n");
		return;
	}

	info->addr_source = SI_SMBIOS;
	printk(KERN_INFO PFX "probing via SMBIOS\n");

	switch (ipmi_data->type) {
	case 0x01: /*     */
		info->si_type = SI_KCS;
		break;
	case 0x02: /*      */
		info->si_type = SI_SMIC;
		break;
	case 0x03: /*    */
		info->si_type = SI_BT;
		break;
	default:
		kfree(info);
		return;
	}

	switch (ipmi_data->addr_space) {
	case IPMI_MEM_ADDR_SPACE:
		info->io_setup = mem_setup;
		info->io.addr_type = IPMI_MEM_ADDR_SPACE;
		break;

	case IPMI_IO_ADDR_SPACE:
		info->io_setup = port_setup;
		info->io.addr_type = IPMI_IO_ADDR_SPACE;
		break;

	default:
		kfree(info);
		printk(KERN_WARNING PFX "Unknown SMBIOS I/O Address type: %d\n",
		       ipmi_data->addr_space);
		return;
	}
	info->io.addr_data = ipmi_data->base_addr;

	info->io.regspacing = ipmi_data->offset;
	if (!info->io.regspacing)
		info->io.regspacing = DEFAULT_REGSPACING;
	info->io.regsize = DEFAULT_REGSPACING;
	info->io.regshift = 0;

	info->slave_addr = ipmi_data->slave_addr;

	info->irq = ipmi_data->irq;
	if (info->irq)
		info->irq_setup = std_irq_setup;

	pr_info("ipmi_si: SMBIOS: %s %#lx regsize %d spacing %d irq %d\n",
		 (info->io.addr_type == IPMI_IO_ADDR_SPACE) ? "io" : "mem",
		 info->io.addr_data, info->io.regsize, info->io.regspacing,
		 info->irq);

	if (add_smi(info))
		kfree(info);
}

static void dmi_find_bmc(void)
{
	const struct dmi_device *dev = NULL;
	struct dmi_ipmi_data data;
	int                  rv;

	while ((dev = dmi_find_device(DMI_DEV_TYPE_IPMI, NULL, dev))) {
		memset(&data, 0, sizeof(data));
		rv = decode_dmi((const struct dmi_header *) dev->device_data,
				&data);
		if (!rv)
			try_init_dmi(&data);
	}
}
#endif /*            */

#ifdef CONFIG_PCI

#define PCI_ERMC_CLASSCODE		0x0C0700
#define PCI_ERMC_CLASSCODE_MASK		0xffffff00
#define PCI_ERMC_CLASSCODE_TYPE_MASK	0xff
#define PCI_ERMC_CLASSCODE_TYPE_SMIC	0x00
#define PCI_ERMC_CLASSCODE_TYPE_KCS	0x01
#define PCI_ERMC_CLASSCODE_TYPE_BT	0x02

#define PCI_HP_VENDOR_ID    0x103C
#define PCI_MMC_DEVICE_ID   0x121A
#define PCI_MMC_ADDR_CW     0x10

static void ipmi_pci_cleanup(struct smi_info *info)
{
	struct pci_dev *pdev = info->addr_source_data;

	pci_disable_device(pdev);
}

static int ipmi_pci_probe_regspacing(struct smi_info *info)
{
	if (info->si_type == SI_KCS) {
		unsigned char	status;
		int		regspacing;

		info->io.regsize = DEFAULT_REGSIZE;
		info->io.regshift = 0;
		info->io_size = 2;
		info->handlers = &kcs_smi_handlers;

		/*                             */
		for (regspacing = DEFAULT_REGSPACING; regspacing <= 16;) {
			info->io.regspacing = regspacing;
			if (info->io_setup(info)) {
				dev_err(info->dev,
					"Could not setup I/O space\n");
				return DEFAULT_REGSPACING;
			}
			/*                   */
			info->io.outputb(&info->io, 1, 0x10);
			/*                  */
			status = info->io.inputb(&info->io, 1);
			info->io_cleanup(info);
			if (status)
				return regspacing;
			regspacing *= 4;
		}
	}
	return DEFAULT_REGSPACING;
}

static int ipmi_pci_probe(struct pci_dev *pdev,
				    const struct pci_device_id *ent)
{
	int rv;
	int class_type = pdev->class & PCI_ERMC_CLASSCODE_TYPE_MASK;
	struct smi_info *info;

	info = smi_info_alloc();
	if (!info)
		return -ENOMEM;

	info->addr_source = SI_PCI;
	dev_info(&pdev->dev, "probing via PCI");

	switch (class_type) {
	case PCI_ERMC_CLASSCODE_TYPE_SMIC:
		info->si_type = SI_SMIC;
		break;

	case PCI_ERMC_CLASSCODE_TYPE_KCS:
		info->si_type = SI_KCS;
		break;

	case PCI_ERMC_CLASSCODE_TYPE_BT:
		info->si_type = SI_BT;
		break;

	default:
		kfree(info);
		dev_info(&pdev->dev, "Unknown IPMI type: %d\n", class_type);
		return -ENOMEM;
	}

	rv = pci_enable_device(pdev);
	if (rv) {
		dev_err(&pdev->dev, "couldn't enable PCI device\n");
		kfree(info);
		return rv;
	}

	info->addr_source_cleanup = ipmi_pci_cleanup;
	info->addr_source_data = pdev;

	if (pci_resource_flags(pdev, 0) & IORESOURCE_IO) {
		info->io_setup = port_setup;
		info->io.addr_type = IPMI_IO_ADDR_SPACE;
	} else {
		info->io_setup = mem_setup;
		info->io.addr_type = IPMI_MEM_ADDR_SPACE;
	}
	info->io.addr_data = pci_resource_start(pdev, 0);

	info->io.regspacing = ipmi_pci_probe_regspacing(info);
	info->io.regsize = DEFAULT_REGSIZE;
	info->io.regshift = 0;

	info->irq = pdev->irq;
	if (info->irq)
		info->irq_setup = std_irq_setup;

	info->dev = &pdev->dev;
	pci_set_drvdata(pdev, info);

	dev_info(&pdev->dev, "%pR regsize %d spacing %d irq %d\n",
		&pdev->resource[0], info->io.regsize, info->io.regspacing,
		info->irq);

	if (add_smi(info))
		kfree(info);

	return 0;
}

static void ipmi_pci_remove(struct pci_dev *pdev)
{
	struct smi_info *info = pci_get_drvdata(pdev);
	cleanup_one_si(info);
}

static struct pci_device_id ipmi_pci_devices[] = {
	{ PCI_DEVICE(PCI_HP_VENDOR_ID, PCI_MMC_DEVICE_ID) },
	{ PCI_DEVICE_CLASS(PCI_ERMC_CLASSCODE, PCI_ERMC_CLASSCODE_MASK) },
	{ 0, }
};
MODULE_DEVICE_TABLE(pci, ipmi_pci_devices);

static struct pci_driver ipmi_pci_driver = {
	.name =         DEVICE_NAME,
	.id_table =     ipmi_pci_devices,
	.probe =        ipmi_pci_probe,
	.remove =       ipmi_pci_remove,
};
#endif /*            */

static struct of_device_id ipmi_match[];
static int ipmi_probe(struct platform_device *dev)
{
#ifdef CONFIG_OF
	const struct of_device_id *match;
	struct smi_info *info;
	struct resource resource;
	const __be32 *regsize, *regspacing, *regshift;
	struct device_node *np = dev->dev.of_node;
	int ret;
	int proplen;

	dev_info(&dev->dev, "probing via device tree\n");

	match = of_match_device(ipmi_match, &dev->dev);
	if (!match)
		return -EINVAL;

	ret = of_address_to_resource(np, 0, &resource);
	if (ret) {
		dev_warn(&dev->dev, PFX "invalid address from OF\n");
		return ret;
	}

	regsize = of_get_property(np, "reg-size", &proplen);
	if (regsize && proplen != 4) {
		dev_warn(&dev->dev, PFX "invalid regsize from OF\n");
		return -EINVAL;
	}

	regspacing = of_get_property(np, "reg-spacing", &proplen);
	if (regspacing && proplen != 4) {
		dev_warn(&dev->dev, PFX "invalid regspacing from OF\n");
		return -EINVAL;
	}

	regshift = of_get_property(np, "reg-shift", &proplen);
	if (regshift && proplen != 4) {
		dev_warn(&dev->dev, PFX "invalid regshift from OF\n");
		return -EINVAL;
	}

	info = smi_info_alloc();

	if (!info) {
		dev_err(&dev->dev,
			"could not allocate memory for OF probe\n");
		return -ENOMEM;
	}

	info->si_type		= (enum si_type) match->data;
	info->addr_source	= SI_DEVICETREE;
	info->irq_setup		= std_irq_setup;

	if (resource.flags & IORESOURCE_IO) {
		info->io_setup		= port_setup;
		info->io.addr_type	= IPMI_IO_ADDR_SPACE;
	} else {
		info->io_setup		= mem_setup;
		info->io.addr_type	= IPMI_MEM_ADDR_SPACE;
	}

	info->io.addr_data	= resource.start;

	info->io.regsize	= regsize ? be32_to_cpup(regsize) : DEFAULT_REGSIZE;
	info->io.regspacing	= regspacing ? be32_to_cpup(regspacing) : DEFAULT_REGSPACING;
	info->io.regshift	= regshift ? be32_to_cpup(regshift) : 0;

	info->irq		= irq_of_parse_and_map(dev->dev.of_node, 0);
	info->dev		= &dev->dev;

	dev_dbg(&dev->dev, "addr 0x%lx regsize %d spacing %d irq %d\n",
		info->io.addr_data, info->io.regsize, info->io.regspacing,
		info->irq);

	dev_set_drvdata(&dev->dev, info);

	if (add_smi(info)) {
		kfree(info);
		return -EBUSY;
	}
#endif
	return 0;
}

static int ipmi_remove(struct platform_device *dev)
{
#ifdef CONFIG_OF
	cleanup_one_si(dev_get_drvdata(&dev->dev));
#endif
	return 0;
}

static struct of_device_id ipmi_match[] =
{
	{ .type = "ipmi", .compatible = "ipmi-kcs",
	  .data = (void *)(unsigned long) SI_KCS },
	{ .type = "ipmi", .compatible = "ipmi-smic",
	  .data = (void *)(unsigned long) SI_SMIC },
	{ .type = "ipmi", .compatible = "ipmi-bt",
	  .data = (void *)(unsigned long) SI_BT },
	{},
};

static struct platform_driver ipmi_driver = {
	.driver = {
		.name = DEVICE_NAME,
		.owner = THIS_MODULE,
		.of_match_table = ipmi_match,
	},
	.probe		= ipmi_probe,
	.remove		= ipmi_remove,
};

static int wait_for_msg_done(struct smi_info *smi_info)
{
	enum si_sm_result     smi_result;

	smi_result = smi_info->handlers->event(smi_info->si_sm, 0);
	for (;;) {
		if (smi_result == SI_SM_CALL_WITH_DELAY ||
		    smi_result == SI_SM_CALL_WITH_TICK_DELAY) {
			schedule_timeout_uninterruptible(1);
			smi_result = smi_info->handlers->event(
				smi_info->si_sm, 100);
		} else if (smi_result == SI_SM_CALL_WITHOUT_DELAY) {
			smi_result = smi_info->handlers->event(
				smi_info->si_sm, 0);
		} else
			break;
	}
	if (smi_result == SI_SM_HOSED)
		/*
                                                               
                                                    
   */
		return -ENODEV;

	return 0;
}

static int try_get_dev_id(struct smi_info *smi_info)
{
	unsigned char         msg[2];
	unsigned char         *resp;
	unsigned long         resp_len;
	int                   rv = 0;

	resp = kmalloc(IPMI_MAX_MSG_LENGTH, GFP_KERNEL);
	if (!resp)
		return -ENOMEM;

	/*
                                                             
                
  */
	msg[0] = IPMI_NETFN_APP_REQUEST << 2;
	msg[1] = IPMI_GET_DEVICE_ID_CMD;
	smi_info->handlers->start_transaction(smi_info->si_sm, msg, 2);

	rv = wait_for_msg_done(smi_info);
	if (rv)
		goto out;

	resp_len = smi_info->handlers->get_result(smi_info->si_sm,
						  resp, IPMI_MAX_MSG_LENGTH);

	/*                                                                   */
	rv = ipmi_demangle_device_id(resp, resp_len, &smi_info->device_id);

 out:
	kfree(resp);
	return rv;
}

static int try_enable_event_buffer(struct smi_info *smi_info)
{
	unsigned char         msg[3];
	unsigned char         *resp;
	unsigned long         resp_len;
	int                   rv = 0;

	resp = kmalloc(IPMI_MAX_MSG_LENGTH, GFP_KERNEL);
	if (!resp)
		return -ENOMEM;

	msg[0] = IPMI_NETFN_APP_REQUEST << 2;
	msg[1] = IPMI_GET_BMC_GLOBAL_ENABLES_CMD;
	smi_info->handlers->start_transaction(smi_info->si_sm, msg, 2);

	rv = wait_for_msg_done(smi_info);
	if (rv) {
		printk(KERN_WARNING PFX "Error getting response from get"
		       " global enables command, the event buffer is not"
		       " enabled.\n");
		goto out;
	}

	resp_len = smi_info->handlers->get_result(smi_info->si_sm,
						  resp, IPMI_MAX_MSG_LENGTH);

	if (resp_len < 4 ||
			resp[0] != (IPMI_NETFN_APP_REQUEST | 1) << 2 ||
			resp[1] != IPMI_GET_BMC_GLOBAL_ENABLES_CMD   ||
			resp[2] != 0) {
		printk(KERN_WARNING PFX "Invalid return from get global"
		       " enables command, cannot enable the event buffer.\n");
		rv = -EINVAL;
		goto out;
	}

	if (resp[3] & IPMI_BMC_EVT_MSG_BUFF)
		/*                                           */
		goto out;

	msg[0] = IPMI_NETFN_APP_REQUEST << 2;
	msg[1] = IPMI_SET_BMC_GLOBAL_ENABLES_CMD;
	msg[2] = resp[3] | IPMI_BMC_EVT_MSG_BUFF;
	smi_info->handlers->start_transaction(smi_info->si_sm, msg, 3);

	rv = wait_for_msg_done(smi_info);
	if (rv) {
		printk(KERN_WARNING PFX "Error getting response from set"
		       " global, enables command, the event buffer is not"
		       " enabled.\n");
		goto out;
	}

	resp_len = smi_info->handlers->get_result(smi_info->si_sm,
						  resp, IPMI_MAX_MSG_LENGTH);

	if (resp_len < 3 ||
			resp[0] != (IPMI_NETFN_APP_REQUEST | 1) << 2 ||
			resp[1] != IPMI_SET_BMC_GLOBAL_ENABLES_CMD) {
		printk(KERN_WARNING PFX "Invalid return from get global,"
		       "enables command, not enable the event buffer.\n");
		rv = -EINVAL;
		goto out;
	}

	if (resp[2] != 0)
		/*
                                                     
                                            
   */
		rv = -ENOENT;
 out:
	kfree(resp);
	return rv;
}

static int smi_type_proc_show(struct seq_file *m, void *v)
{
	struct smi_info *smi = m->private;

	return seq_printf(m, "%s\n", si_to_str[smi->si_type]);
}

static int smi_type_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, smi_type_proc_show, PDE_DATA(inode));
}

static const struct file_operations smi_type_proc_ops = {
	.open		= smi_type_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int smi_si_stats_proc_show(struct seq_file *m, void *v)
{
	struct smi_info *smi = m->private;

	seq_printf(m, "interrupts_enabled:    %d\n",
		       smi->irq && !smi->interrupt_disabled);
	seq_printf(m, "short_timeouts:        %u\n",
		       smi_get_stat(smi, short_timeouts));
	seq_printf(m, "long_timeouts:         %u\n",
		       smi_get_stat(smi, long_timeouts));
	seq_printf(m, "idles:                 %u\n",
		       smi_get_stat(smi, idles));
	seq_printf(m, "interrupts:            %u\n",
		       smi_get_stat(smi, interrupts));
	seq_printf(m, "attentions:            %u\n",
		       smi_get_stat(smi, attentions));
	seq_printf(m, "flag_fetches:          %u\n",
		       smi_get_stat(smi, flag_fetches));
	seq_printf(m, "hosed_count:           %u\n",
		       smi_get_stat(smi, hosed_count));
	seq_printf(m, "complete_transactions: %u\n",
		       smi_get_stat(smi, complete_transactions));
	seq_printf(m, "events:                %u\n",
		       smi_get_stat(smi, events));
	seq_printf(m, "watchdog_pretimeouts:  %u\n",
		       smi_get_stat(smi, watchdog_pretimeouts));
	seq_printf(m, "incoming_messages:     %u\n",
		       smi_get_stat(smi, incoming_messages));
	return 0;
}

static int smi_si_stats_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, smi_si_stats_proc_show, PDE_DATA(inode));
}

static const struct file_operations smi_si_stats_proc_ops = {
	.open		= smi_si_stats_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int smi_params_proc_show(struct seq_file *m, void *v)
{
	struct smi_info *smi = m->private;

	return seq_printf(m,
		       "%s,%s,0x%lx,rsp=%d,rsi=%d,rsh=%d,irq=%d,ipmb=%d\n",
		       si_to_str[smi->si_type],
		       addr_space_to_str[smi->io.addr_type],
		       smi->io.addr_data,
		       smi->io.regspacing,
		       smi->io.regsize,
		       smi->io.regshift,
		       smi->irq,
		       smi->slave_addr);
}

static int smi_params_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, smi_params_proc_show, PDE_DATA(inode));
}

static const struct file_operations smi_params_proc_ops = {
	.open		= smi_params_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

/*
                                      
                                                
  
                                                          
                                                      
 */
static int oem_data_avail_to_receive_msg_avail(struct smi_info *smi_info)
{
	smi_info->msg_flags = ((smi_info->msg_flags & ~OEM_DATA_AVAIL) |
			       RECEIVE_MSG_AVAIL);
	return 1;
}

/*
                                        
                                               
  
                                                                  
                                                                      
                                                                   
                                                                   
                                
  
                                                                
                                                                    
                                                                  
                    
                                                              
                         
                                                   
                                              
                                           
                                            
  
                                                                  
                                                                
  
 */
#define DELL_POWEREDGE_8G_BMC_DEVICE_ID  0x20
#define DELL_POWEREDGE_8G_BMC_DEVICE_REV 0x80
#define DELL_POWEREDGE_8G_BMC_IPMI_VERSION 0x51
#define DELL_IANA_MFR_ID 0x0002a2
static void setup_dell_poweredge_oem_data_handler(struct smi_info *smi_info)
{
	struct ipmi_device_id *id = &smi_info->device_id;
	if (id->manufacturer_id == DELL_IANA_MFR_ID) {
		if (id->device_id       == DELL_POWEREDGE_8G_BMC_DEVICE_ID  &&
		    id->device_revision == DELL_POWEREDGE_8G_BMC_DEVICE_REV &&
		    id->ipmi_version   == DELL_POWEREDGE_8G_BMC_IPMI_VERSION) {
			smi_info->oem_data_avail_handler =
				oem_data_avail_to_receive_msg_avail;
		} else if (ipmi_version_major(id) < 1 ||
			   (ipmi_version_major(id) == 1 &&
			    ipmi_version_minor(id) < 5)) {
			smi_info->oem_data_avail_handler =
				oem_data_avail_to_receive_msg_avail;
		}
	}
}

#define CANNOT_RETURN_REQUESTED_LENGTH 0xCA
static void return_hosed_msg_badsize(struct smi_info *smi_info)
{
	struct ipmi_smi_msg *msg = smi_info->curr_msg;

	/*                    */
	msg->rsp[0] = msg->data[0] | 4;
	msg->rsp[1] = msg->data[1];
	msg->rsp[2] = CANNOT_RETURN_REQUESTED_LENGTH;
	msg->rsp_size = 3;
	smi_info->curr_msg = NULL;
	deliver_recv_msg(smi_info, msg);
}

/*
                                    
                                               
  
                                                                    
                                                             
                                                                    
                                                                      
                                                                      
 */

#define STORAGE_NETFN 0x0A
#define STORAGE_CMD_GET_SDR 0x23
static int dell_poweredge_bt_xaction_handler(struct notifier_block *self,
					     unsigned long unused,
					     void *in)
{
	struct smi_info *smi_info = in;
	unsigned char *data = smi_info->curr_msg->data;
	unsigned int size   = smi_info->curr_msg->data_size;
	if (size >= 8 &&
	    (data[0]>>2) == STORAGE_NETFN &&
	    data[1] == STORAGE_CMD_GET_SDR &&
	    data[7] == 0x3A) {
		return_hosed_msg_badsize(smi_info);
		return NOTIFY_STOP;
	}
	return NOTIFY_DONE;
}

static struct notifier_block dell_poweredge_bt_xaction_notifier = {
	.notifier_call	= dell_poweredge_bt_xaction_handler,
};

/*
                                          
                                                       
  
                                                         
                                           
 */
static void
setup_dell_poweredge_bt_xaction_handler(struct smi_info *smi_info)
{
	struct ipmi_device_id *id = &smi_info->device_id;
	if (id->manufacturer_id == DELL_IANA_MFR_ID &&
	    smi_info->si_type == SI_BT)
		register_xaction_notifier(&dell_poweredge_bt_xaction_notifier);
}

/*
                         
                                                       
  
                                                         
                                           
 */

static void setup_oem_data_handler(struct smi_info *smi_info)
{
	setup_dell_poweredge_oem_data_handler(smi_info);
}

static void setup_xaction_handlers(struct smi_info *smi_info)
{
	setup_dell_poweredge_bt_xaction_handler(smi_info);
}

static inline void wait_for_timer_and_thread(struct smi_info *smi_info)
{
	if (smi_info->intf) {
		/*
                                                 
                                                  
   */
		if (smi_info->thread != NULL)
			kthread_stop(smi_info->thread);
		del_timer_sync(&smi_info->si_timer);
	}
}

static struct ipmi_default_vals
{
	int type;
	int port;
} ipmi_defaults[] =
{
	{ .type = SI_KCS, .port = 0xca2 },
	{ .type = SI_SMIC, .port = 0xca9 },
	{ .type = SI_BT, .port = 0xe4 },
	{ .port = 0 }
};

static void default_find_bmc(void)
{
	struct smi_info *info;
	int             i;

	for (i = 0; ; i++) {
		if (!ipmi_defaults[i].port)
			break;
#ifdef CONFIG_PPC
		if (check_legacy_ioport(ipmi_defaults[i].port))
			continue;
#endif
		info = smi_info_alloc();
		if (!info)
			return;

		info->addr_source = SI_DEFAULT;

		info->si_type = ipmi_defaults[i].type;
		info->io_setup = port_setup;
		info->io.addr_data = ipmi_defaults[i].port;
		info->io.addr_type = IPMI_IO_ADDR_SPACE;

		info->io.addr = NULL;
		info->io.regspacing = DEFAULT_REGSPACING;
		info->io.regsize = DEFAULT_REGSPACING;
		info->io.regshift = 0;

		if (add_smi(info) == 0) {
			if ((try_smi_init(info)) == 0) {
				/*              */
				printk(KERN_INFO PFX "Found default %s"
				" state machine at %s address 0x%lx\n",
				si_to_str[info->si_type],
				addr_space_to_str[info->io.addr_type],
				info->io.addr_data);
			} else
				cleanup_one_si(info);
		} else {
			kfree(info);
		}
	}
}

static int is_new_interface(struct smi_info *info)
{
	struct smi_info *e;

	list_for_each_entry(e, &smi_infos, link) {
		if (e->io.addr_type != info->io.addr_type)
			continue;
		if (e->io.addr_data == info->io.addr_data)
			return 0;
	}

	return 1;
}

static int add_smi(struct smi_info *new_smi)
{
	int rv = 0;

	printk(KERN_INFO PFX "Adding %s-specified %s state machine",
			ipmi_addr_src_to_str[new_smi->addr_source],
			si_to_str[new_smi->si_type]);
	mutex_lock(&smi_infos_lock);
	if (!is_new_interface(new_smi)) {
		printk(KERN_CONT " duplicate interface\n");
		rv = -EBUSY;
		goto out_err;
	}

	printk(KERN_CONT "\n");

	/*                                                         */
	new_smi->intf = NULL;
	new_smi->si_sm = NULL;
	new_smi->handlers = NULL;

	list_add_tail(&new_smi->link, &smi_infos);

out_err:
	mutex_unlock(&smi_infos_lock);
	return rv;
}

static int try_smi_init(struct smi_info *new_smi)
{
	int rv = 0;
	int i;

	printk(KERN_INFO PFX "Trying %s-specified %s state"
	       " machine at %s address 0x%lx, slave address 0x%x,"
	       " irq %d\n",
	       ipmi_addr_src_to_str[new_smi->addr_source],
	       si_to_str[new_smi->si_type],
	       addr_space_to_str[new_smi->io.addr_type],
	       new_smi->io.addr_data,
	       new_smi->slave_addr, new_smi->irq);

	switch (new_smi->si_type) {
	case SI_KCS:
		new_smi->handlers = &kcs_smi_handlers;
		break;

	case SI_SMIC:
		new_smi->handlers = &smic_smi_handlers;
		break;

	case SI_BT:
		new_smi->handlers = &bt_smi_handlers;
		break;

	default:
		/*                                   */
		rv = -EIO;
		goto out_err;
	}

	/*                                                      */
	new_smi->si_sm = kmalloc(new_smi->handlers->size(), GFP_KERNEL);
	if (!new_smi->si_sm) {
		printk(KERN_ERR PFX
		       "Could not allocate state machine memory\n");
		rv = -ENOMEM;
		goto out_err;
	}
	new_smi->io_size = new_smi->handlers->init_data(new_smi->si_sm,
							&new_smi->io);

	/*                                                       */
	rv = new_smi->io_setup(new_smi);
	if (rv) {
		printk(KERN_ERR PFX "Could not set up I/O space\n");
		goto out_err;
	}

	/*                               */
	if (new_smi->handlers->detect(new_smi->si_sm)) {
		if (new_smi->addr_source)
			printk(KERN_INFO PFX "Interface detection failed\n");
		rv = -ENODEV;
		goto out_err;
	}

	/*
                                                              
                          
  */
	rv = try_get_dev_id(new_smi);
	if (rv) {
		if (new_smi->addr_source)
			printk(KERN_INFO PFX "There appears to be no BMC"
			       " at this location\n");
		goto out_err;
	}

	setup_oem_data_handler(new_smi);
	setup_xaction_handlers(new_smi);

	INIT_LIST_HEAD(&(new_smi->xmit_msgs));
	INIT_LIST_HEAD(&(new_smi->hp_xmit_msgs));
	new_smi->curr_msg = NULL;
	atomic_set(&new_smi->req_events, 0);
	new_smi->run_to_completion = 0;
	for (i = 0; i < SI_NUM_STATS; i++)
		atomic_set(&new_smi->stats[i], 0);

	new_smi->interrupt_disabled = 1;
	atomic_set(&new_smi->stop_operation, 0);
	new_smi->intf_num = smi_num;
	smi_num++;

	rv = try_enable_event_buffer(new_smi);
	if (rv == 0)
		new_smi->has_event_buffer = 1;

	/*
                                                               
                                         
  */
	start_clear_flags(new_smi);
	/*                                         */
	if (new_smi->irq)
		new_smi->si_state = SI_CLEARING_FLAGS_THEN_SET_IRQ;

	if (!new_smi->dev) {
		/*
                                                     
                                              
   */
		new_smi->pdev = platform_device_alloc("ipmi_si",
						      new_smi->intf_num);
		if (!new_smi->pdev) {
			printk(KERN_ERR PFX
			       "Unable to allocate platform device\n");
			goto out_err;
		}
		new_smi->dev = &new_smi->pdev->dev;
		new_smi->dev->driver = &ipmi_driver.driver;

		rv = platform_device_add(new_smi->pdev);
		if (rv) {
			printk(KERN_ERR PFX
			       "Unable to register system interface device:"
			       " %d\n",
			       rv);
			goto out_err;
		}
		new_smi->dev_registered = 1;
	}

	rv = ipmi_register_smi(&handlers,
			       new_smi,
			       &new_smi->device_id,
			       new_smi->dev,
			       "bmc",
			       new_smi->slave_addr);
	if (rv) {
		dev_err(new_smi->dev, "Unable to register device: error %d\n",
			rv);
		goto out_err_stop_timer;
	}

	rv = ipmi_smi_add_proc_entry(new_smi->intf, "type",
				     &smi_type_proc_ops,
				     new_smi);
	if (rv) {
		dev_err(new_smi->dev, "Unable to create proc entry: %d\n", rv);
		goto out_err_stop_timer;
	}

	rv = ipmi_smi_add_proc_entry(new_smi->intf, "si_stats",
				     &smi_si_stats_proc_ops,
				     new_smi);
	if (rv) {
		dev_err(new_smi->dev, "Unable to create proc entry: %d\n", rv);
		goto out_err_stop_timer;
	}

	rv = ipmi_smi_add_proc_entry(new_smi->intf, "params",
				     &smi_params_proc_ops,
				     new_smi);
	if (rv) {
		dev_err(new_smi->dev, "Unable to create proc entry: %d\n", rv);
		goto out_err_stop_timer;
	}

	dev_info(new_smi->dev, "IPMI %s interface initialized\n",
		 si_to_str[new_smi->si_type]);

	return 0;

 out_err_stop_timer:
	atomic_inc(&new_smi->stop_operation);
	wait_for_timer_and_thread(new_smi);

 out_err:
	new_smi->interrupt_disabled = 1;

	if (new_smi->intf) {
		ipmi_unregister_smi(new_smi->intf);
		new_smi->intf = NULL;
	}

	if (new_smi->irq_cleanup) {
		new_smi->irq_cleanup(new_smi);
		new_smi->irq_cleanup = NULL;
	}

	/*
                                                       
                                                        
              
  */
	synchronize_sched();

	if (new_smi->si_sm) {
		if (new_smi->handlers)
			new_smi->handlers->cleanup(new_smi->si_sm);
		kfree(new_smi->si_sm);
		new_smi->si_sm = NULL;
	}
	if (new_smi->addr_source_cleanup) {
		new_smi->addr_source_cleanup(new_smi);
		new_smi->addr_source_cleanup = NULL;
	}
	if (new_smi->io_cleanup) {
		new_smi->io_cleanup(new_smi);
		new_smi->io_cleanup = NULL;
	}

	if (new_smi->dev_registered) {
		platform_device_unregister(new_smi->pdev);
		new_smi->dev_registered = 0;
	}

	return rv;
}

static int init_ipmi_si(void)
{
	int  i;
	char *str;
	int  rv;
	struct smi_info *e;
	enum ipmi_addr_src type = SI_INVALID;

	if (initialized)
		return 0;
	initialized = 1;

	if (si_tryplatform) {
		rv = platform_driver_register(&ipmi_driver);
		if (rv) {
			printk(KERN_ERR PFX "Unable to register "
			       "driver: %d\n", rv);
			return rv;
		}
	}

	/*                                                   */
	str = si_type_str;
	if (*str != '\0') {
		for (i = 0; (i < SI_MAX_PARMS) && (*str != '\0'); i++) {
			si_type[i] = str;
			str = strchr(str, ',');
			if (str) {
				*str = '\0';
				str++;
			} else {
				break;
			}
		}
	}

	printk(KERN_INFO "IPMI System Interface driver.\n");

	/*                                                                 */
	if (!hardcode_find_bmc())
		return 0;

#ifdef CONFIG_PCI
	if (si_trypci) {
		rv = pci_register_driver(&ipmi_pci_driver);
		if (rv)
			printk(KERN_ERR PFX "Unable to register "
			       "PCI driver: %d\n", rv);
		else
			pci_registered = 1;
	}
#endif

#ifdef CONFIG_ACPI
	if (si_tryacpi) {
		pnp_register_driver(&ipmi_pnp_driver);
		pnp_registered = 1;
	}
#endif

#ifdef CONFIG_DMI
	if (si_trydmi)
		dmi_find_bmc();
#endif

#ifdef CONFIG_ACPI
	if (si_tryacpi)
		spmi_find_bmc();
#endif

	/*                                                                
                                                                     
                                                                    
                                                     */

	mutex_lock(&smi_infos_lock);
	list_for_each_entry(e, &smi_infos, link) {
		/*                                                        
                                                         
                                                                */
		if (e->irq && (!type || e->addr_source == type)) {
			if (!try_smi_init(e)) {
				type = e->addr_source;
			}
		}
	}

	/*                                                                  */
	if (type) {
		mutex_unlock(&smi_infos_lock);
		return 0;
	}

	/*                                   */

	list_for_each_entry(e, &smi_infos, link) {
		if (!e->irq && (!type || e->addr_source == type)) {
			if (!try_smi_init(e)) {
				type = e->addr_source;
			}
		}
	}
	mutex_unlock(&smi_infos_lock);

	if (type)
		return 0;

	if (si_trydefaults) {
		mutex_lock(&smi_infos_lock);
		if (list_empty(&smi_infos)) {
			/*                                 */
			mutex_unlock(&smi_infos_lock);
			default_find_bmc();
		} else
			mutex_unlock(&smi_infos_lock);
	}

	mutex_lock(&smi_infos_lock);
	if (unload_when_empty && list_empty(&smi_infos)) {
		mutex_unlock(&smi_infos_lock);
		cleanup_ipmi_si();
		printk(KERN_WARNING PFX
		       "Unable to find any System Interface(s)\n");
		return -ENODEV;
	} else {
		mutex_unlock(&smi_infos_lock);
		return 0;
	}
}
module_init(init_ipmi_si);

static void cleanup_one_si(struct smi_info *to_clean)
{
	int           rv = 0;
	unsigned long flags;

	if (!to_clean)
		return;

	list_del(&to_clean->link);

	/*                                            */
	atomic_inc(&to_clean->stop_operation);

	/*
                                                               
          
  */
	wait_for_timer_and_thread(to_clean);

	/*
                                                              
                                                            
                       
  */
	spin_lock_irqsave(&to_clean->si_lock, flags);
	while (to_clean->curr_msg || (to_clean->si_state != SI_NORMAL)) {
		spin_unlock_irqrestore(&to_clean->si_lock, flags);
		poll(to_clean);
		schedule_timeout_uninterruptible(1);
		spin_lock_irqsave(&to_clean->si_lock, flags);
	}
	disable_si_irq(to_clean);
	spin_unlock_irqrestore(&to_clean->si_lock, flags);
	while (to_clean->curr_msg || (to_clean->si_state != SI_NORMAL)) {
		poll(to_clean);
		schedule_timeout_uninterruptible(1);
	}

	/*                                                            */
	if (to_clean->irq_cleanup)
		to_clean->irq_cleanup(to_clean);
	while (to_clean->curr_msg || (to_clean->si_state != SI_NORMAL)) {
		poll(to_clean);
		schedule_timeout_uninterruptible(1);
	}

	if (to_clean->intf)
		rv = ipmi_unregister_smi(to_clean->intf);

	if (rv) {
		printk(KERN_ERR PFX "Unable to unregister device: errno=%d\n",
		       rv);
	}

	if (to_clean->handlers)
		to_clean->handlers->cleanup(to_clean->si_sm);

	kfree(to_clean->si_sm);

	if (to_clean->addr_source_cleanup)
		to_clean->addr_source_cleanup(to_clean);
	if (to_clean->io_cleanup)
		to_clean->io_cleanup(to_clean);

	if (to_clean->dev_registered)
		platform_device_unregister(to_clean->pdev);

	kfree(to_clean);
}

static void cleanup_ipmi_si(void)
{
	struct smi_info *e, *tmp_e;

	if (!initialized)
		return;

#ifdef CONFIG_PCI
	if (pci_registered)
		pci_unregister_driver(&ipmi_pci_driver);
#endif
#ifdef CONFIG_ACPI
	if (pnp_registered)
		pnp_unregister_driver(&ipmi_pnp_driver);
#endif

	platform_driver_unregister(&ipmi_driver);

	mutex_lock(&smi_infos_lock);
	list_for_each_entry_safe(e, tmp_e, &smi_infos, link)
		cleanup_one_si(e);
	mutex_unlock(&smi_infos_lock);
}
module_exit(cleanup_ipmi_si);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Corey Minyard <minyard@mvista.com>");
MODULE_DESCRIPTION("Interface to the IPMI driver for the KCS, SMIC, and BT"
		   " system interfaces.");
