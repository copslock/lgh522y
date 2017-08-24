/*
 * Sleepable Read-Copy Update mechanism for mutual exclusion.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Copyright (C) IBM Corporation, 2006
 * Copyright (C) Fujitsu, 2012
 *
 * Author: Paul McKenney <paulmck@us.ibm.com>
 *	   Lai Jiangshan <laijs@cn.fujitsu.com>
 *
 * For detailed explanation of Read-Copy Update mechanism see -
 * 		Documentation/RCU/ *.txt
 *
 */

#include <linux/export.h>
#include <linux/mutex.h>
#include <linux/percpu.h>
#include <linux/preempt.h>
#include <linux/rcupdate.h>
#include <linux/sched.h>
#include <linux/smp.h>
#include <linux/delay.h>
#include <linux/srcu.h>

#include <trace/events/rcu.h>

#include "rcu.h"

/*
                                              
 */
static inline void rcu_batch_init(struct rcu_batch *b)
{
	b->head = NULL;
	b->tail = &b->head;
}

/*
                                                                         
 */
static inline void rcu_batch_queue(struct rcu_batch *b, struct rcu_head *head)
{
	*b->tail = head;
	b->tail = &head->next;
}

/*
                                              
 */
static inline bool rcu_batch_empty(struct rcu_batch *b)
{
	return b->tail == &b->head;
}

/*
                                                                       
                                                                        
 */
static inline struct rcu_head *rcu_batch_dequeue(struct rcu_batch *b)
{
	struct rcu_head *head;

	if (rcu_batch_empty(b))
		return NULL;

	head = b->head;
	b->head = head->next;
	if (b->tail == &head->next)
		rcu_batch_init(b);

	return head;
}

/*
                                                                         
                                   
 */
static inline void rcu_batch_move(struct rcu_batch *to, struct rcu_batch *from)
{
	if (!rcu_batch_empty(from)) {
		*to->tail = from->head;
		to->tail = from->tail;
		rcu_batch_init(from);
	}
}

static int init_srcu_struct_fields(struct srcu_struct *sp)
{
	sp->completed = 0;
	spin_lock_init(&sp->queue_lock);
	sp->running = false;
	rcu_batch_init(&sp->batch_queue);
	rcu_batch_init(&sp->batch_check0);
	rcu_batch_init(&sp->batch_check1);
	rcu_batch_init(&sp->batch_done);
	INIT_DELAYED_WORK(&sp->work, process_srcu);
	sp->per_cpu_ref = alloc_percpu(struct srcu_struct_array);
	return sp->per_cpu_ref ? 0 : -ENOMEM;
}

#ifdef CONFIG_DEBUG_LOCK_ALLOC

int __init_srcu_struct(struct srcu_struct *sp, const char *name,
		       struct lock_class_key *key)
{
	/*                                              */
	debug_check_no_locks_freed((void *)sp, sizeof(*sp));
	lockdep_init_map(&sp->dep_map, name, key, 0);
	return init_srcu_struct_fields(sp);
}
EXPORT_SYMBOL_GPL(__init_srcu_struct);

#else /*                                */

/* 
                                                      
                                
  
                                                                          
                                                                        
                      
 */
int init_srcu_struct(struct srcu_struct *sp)
{
	return init_srcu_struct_fields(sp);
}
EXPORT_SYMBOL_GPL(init_srcu_struct);

#endif /*                                      */

/*
                                                                   
                                             
 */
static unsigned long srcu_readers_seq_idx(struct srcu_struct *sp, int idx)
{
	int cpu;
	unsigned long sum = 0;
	unsigned long t;

	for_each_possible_cpu(cpu) {
		t = ACCESS_ONCE(per_cpu_ptr(sp->per_cpu_ref, cpu)->seq[idx]);
		sum += t;
	}
	return sum;
}

/*
                                                                     
                                 
 */
static unsigned long srcu_readers_active_idx(struct srcu_struct *sp, int idx)
{
	int cpu;
	unsigned long sum = 0;
	unsigned long t;

	for_each_possible_cpu(cpu) {
		t = ACCESS_ONCE(per_cpu_ptr(sp->per_cpu_ref, cpu)->c[idx]);
		sum += t;
	}
	return sum;
}

/*
                                                                     
                                                                  
                                                                       
                                                                         
                                                                      
                                                                       
                                 
 */
static bool srcu_readers_active_idx_check(struct srcu_struct *sp, int idx)
{
	unsigned long seq;

	seq = srcu_readers_seq_idx(sp, idx);

	/*
                                                                 
                                                        
                                                                 
                                                                       
                                                                 
                                     
   
                                                             
                                                                    
                                    
  */
	smp_mb(); /*   */

	/*
                                                              
                                                               
                                                           
                                                              
                                                           
                                                               
                                                                 
                                                                
                                                                
                                                             
                                                               
                                                          
   
                                                                      
                
  */
	if (srcu_readers_active_idx(sp, idx) != 0)
		return false;

	/*
                                                          
                                                         
                                                               
                                                            
                                                                
                                                      
   
                                                                 
                                                                
                                                                
                                                                
                                                             
                                                             
                                                                 
                                                             
                                                               
                                                         
                                                             
                                                            
                                                               
                        
  */
	smp_mb(); /*   */

	return srcu_readers_seq_idx(sp, idx) == seq;
}

/* 
                                                               
                                                                           
  
                                                                      
                                                                      
                                                   
 */
static int srcu_readers_active(struct srcu_struct *sp)
{
	int cpu;
	unsigned long sum = 0;

	for_each_possible_cpu(cpu) {
		sum += ACCESS_ONCE(per_cpu_ptr(sp->per_cpu_ref, cpu)->c[0]);
		sum += ACCESS_ONCE(per_cpu_ptr(sp->per_cpu_ref, cpu)->c[1]);
	}
	return sum;
}

/* 
                                                          
                              
  
                                                                         
                                                                
 */
void cleanup_srcu_struct(struct srcu_struct *sp)
{
	if (WARN_ON(srcu_readers_active(sp)))
		return; /*                                      */
	free_percpu(sp->per_cpu_ref);
	sp->per_cpu_ref = NULL;
}
EXPORT_SYMBOL_GPL(cleanup_srcu_struct);

/*
                                                                  
                                                     
                                                                           
 */
int __srcu_read_lock(struct srcu_struct *sp)
{
	int idx;

	idx = ACCESS_ONCE(sp->completed) & 0x1;
	preempt_disable();
	ACCESS_ONCE(this_cpu_ptr(sp->per_cpu_ref)->c[idx]) += 1;
	smp_mb(); /*   */  /*                                     */
	ACCESS_ONCE(this_cpu_ptr(sp->per_cpu_ref)->seq[idx]) += 1;
	preempt_enable();
	return idx;
}
EXPORT_SYMBOL_GPL(__srcu_read_lock);

/*
                                                                    
                                                                      
                                                                             
                                       
 */
void __srcu_read_unlock(struct srcu_struct *sp, int idx)
{
	smp_mb(); /*   */  /*                                     */
	this_cpu_dec(sp->per_cpu_ref->c[idx]);
}
EXPORT_SYMBOL_GPL(__srcu_read_unlock);

/*
                                                                        
                                                                 
                                                                         
                                                                    
                                                                     
                                                                        
 */
#define SRCU_RETRY_CHECK_DELAY		5
#define SYNCHRONIZE_SRCU_TRYCOUNT	2
#define SYNCHRONIZE_SRCU_EXP_TRYCOUNT	12

/*
                                                                  
                                               
                                                                          
                                  
 */
static bool try_check_zero(struct srcu_struct *sp, int idx, int trycount)
{
	for (;;) {
		if (srcu_readers_active_idx_check(sp, idx))
			return true;
		if (--trycount <= 0)
			return false;
		udelay(SRCU_RETRY_CHECK_DELAY);
	}
}

/*
                                                                     
                                                                   
                                                                   
 */
static void srcu_flip(struct srcu_struct *sp)
{
	sp->completed++;
}

/*
                                                                   
                                                                   
 */
void call_srcu(struct srcu_struct *sp, struct rcu_head *head,
		void (*func)(struct rcu_head *head))
{
	unsigned long flags;

	head->next = NULL;
	head->func = func;
	spin_lock_irqsave(&sp->queue_lock, flags);
	rcu_batch_queue(&sp->batch_queue, head);
	if (!sp->running) {
		sp->running = true;
		schedule_delayed_work(&sp->work, 0);
	}
	spin_unlock_irqrestore(&sp->queue_lock, flags);
}
EXPORT_SYMBOL_GPL(call_srcu);

struct rcu_synchronize {
	struct rcu_head head;
	struct completion completion;
};

/*
                                                                  
                            
 */
static void wakeme_after_rcu(struct rcu_head *head)
{
	struct rcu_synchronize *rcu;

	rcu = container_of(head, struct rcu_synchronize, head);
	complete(&rcu->completion);
}

static void srcu_advance_batches(struct srcu_struct *sp, int trycount);
static void srcu_reschedule(struct srcu_struct *sp);

/*
                                                                           
 */
static void __synchronize_srcu(struct srcu_struct *sp, int trycount)
{
	struct rcu_synchronize rcu;
	struct rcu_head *head = &rcu.head;
	bool done = false;

	rcu_lockdep_assert(!lock_is_held(&sp->dep_map) &&
			   !lock_is_held(&rcu_bh_lock_map) &&
			   !lock_is_held(&rcu_lock_map) &&
			   !lock_is_held(&rcu_sched_lock_map),
			   "Illegal synchronize_srcu() in same-type SRCU (or RCU) read-side critical section");

	might_sleep();
	init_completion(&rcu.completion);

	head->next = NULL;
	head->func = wakeme_after_rcu;
	spin_lock_irq(&sp->queue_lock);
	if (!sp->running) {
		/*                            */
		sp->running = true;
		rcu_batch_queue(&sp->batch_check0, head);
		spin_unlock_irq(&sp->queue_lock);

		srcu_advance_batches(sp, trycount);
		if (!rcu_batch_empty(&sp->batch_done)) {
			BUG_ON(sp->batch_done.head != head);
			rcu_batch_dequeue(&sp->batch_done);
			done = true;
		}
		/*                                          */
		srcu_reschedule(sp);
	} else {
		rcu_batch_queue(&sp->batch_queue, head);
		spin_unlock_irq(&sp->queue_lock);
	}

	if (!done)
		wait_for_completion(&rcu.completion);
}

/* 
                                                                               
                                              
  
                                                                    
                                                                       
                                                               
                                                                         
  
                                                  
  
                                                                            
                                                                     
                                                                   
                                                                        
 */
void synchronize_srcu(struct srcu_struct *sp)
{
	__synchronize_srcu(sp, rcu_expedited
			   ? SYNCHRONIZE_SRCU_EXP_TRYCOUNT
			   : SYNCHRONIZE_SRCU_TRYCOUNT);
}
EXPORT_SYMBOL_GPL(synchronize_srcu);

/* 
                                                             
                                              
  
                                                                        
                                              
  
                                                                    
                                                          
                                                                    
                                                                    
                                                             
                                                  
 */
void synchronize_srcu_expedited(struct srcu_struct *sp)
{
	__synchronize_srcu(sp, SYNCHRONIZE_SRCU_EXP_TRYCOUNT);
}
EXPORT_SYMBOL_GPL(synchronize_srcu_expedited);

/* 
                                                                          
 */
void srcu_barrier(struct srcu_struct *sp)
{
	synchronize_srcu(sp);
}
EXPORT_SYMBOL_GPL(srcu_barrier);

/* 
                                                     
                                                        
  
                                                                     
                                                                        
 */
long srcu_batches_completed(struct srcu_struct *sp)
{
	return sp->completed;
}
EXPORT_SYMBOL_GPL(srcu_batches_completed);

#define SRCU_CALLBACK_BATCH	10
#define SRCU_INTERVAL		1

/*
                                                                   
                   
 */
static void srcu_collect_new(struct srcu_struct *sp)
{
	if (!rcu_batch_empty(&sp->batch_queue)) {
		spin_lock_irq(&sp->queue_lock);
		rcu_batch_move(&sp->batch_check0, &sp->batch_queue);
		spin_unlock_irq(&sp->queue_lock);
	}
}

/*
                                                                     
                                                            
 */
static void srcu_advance_batches(struct srcu_struct *sp, int trycount)
{
	int idx = 1 ^ (sp->completed & 1);

	/*
                                                                 
                                                                    
                                                                   
                                                                   
                        
  */

	if (rcu_batch_empty(&sp->batch_check0) &&
	    rcu_batch_empty(&sp->batch_check1))
		return; /*                                  */

	if (!try_check_zero(sp, idx, trycount))
		return; /*                                                 */

	/*
                                                                
                                                             
                                                                      
                                                           
                                                                  
                                                                     
  */
	rcu_batch_move(&sp->batch_done, &sp->batch_check1);

	if (rcu_batch_empty(&sp->batch_check0))
		return; /*                                  */
	srcu_flip(sp);

	/*
                                                       
                                                             
                                         
  */
	rcu_batch_move(&sp->batch_check1, &sp->batch_check0);

	/*
                                                                 
                                                    
  */
	trycount = trycount < 2 ? 2 : trycount;
	if (!try_check_zero(sp, idx^1, trycount))
		return; /*                                                 */

	/*
                                                           
                                                                   
                                                  
  */
	rcu_batch_move(&sp->batch_done, &sp->batch_check1);
}

/*
                                                                     
                                                                     
                 
 */
static void srcu_invoke_callbacks(struct srcu_struct *sp)
{
	int i;
	struct rcu_head *head;

	for (i = 0; i < SRCU_CALLBACK_BATCH; i++) {
		head = rcu_batch_dequeue(&sp->batch_done);
		if (!head)
			break;
		local_bh_disable();
		head->func(head);
		local_bh_enable();
	}
}

/*
                                                                       
                                                                         
 */
static void srcu_reschedule(struct srcu_struct *sp)
{
	bool pending = true;

	if (rcu_batch_empty(&sp->batch_done) &&
	    rcu_batch_empty(&sp->batch_check1) &&
	    rcu_batch_empty(&sp->batch_check0) &&
	    rcu_batch_empty(&sp->batch_queue)) {
		spin_lock_irq(&sp->queue_lock);
		if (rcu_batch_empty(&sp->batch_done) &&
		    rcu_batch_empty(&sp->batch_check1) &&
		    rcu_batch_empty(&sp->batch_check0) &&
		    rcu_batch_empty(&sp->batch_queue)) {
			sp->running = false;
			pending = false;
		}
		spin_unlock_irq(&sp->queue_lock);
	}

	if (pending)
		schedule_delayed_work(&sp->work, SRCU_INTERVAL);
}

/*
                                                                   
 */
void process_srcu(struct work_struct *work)
{
	struct srcu_struct *sp;

	sp = container_of(work, struct srcu_struct, work.work);

	srcu_collect_new(sp);
	srcu_advance_batches(sp, 1);
	srcu_invoke_callbacks(sp);
	srcu_reschedule(sp);
}
EXPORT_SYMBOL_GPL(process_srcu);
