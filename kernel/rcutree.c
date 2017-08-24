/*
 * Read-Copy Update mechanism for mutual exclusion
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
 * Copyright IBM Corporation, 2008
 *
 * Authors: Dipankar Sarma <dipankar@in.ibm.com>
 *	    Manfred Spraul <manfred@colorfullife.com>
 *	    Paul E. McKenney <paulmck@linux.vnet.ibm.com> Hierarchical version
 *
 * Based on the original work by Paul McKenney <paulmck@us.ibm.com>
 * and inputs from Rusty Russell, Andrea Arcangeli and Andi Kleen.
 *
 * For detailed explanation of Read-Copy Update mechanism see -
 *	Documentation/RCU
 */
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/smp.h>
#include <linux/rcupdate.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/nmi.h>
#include <linux/atomic.h>
#include <linux/bitops.h>
#include <linux/export.h>
#include <linux/completion.h>
#include <linux/moduleparam.h>
#include <linux/percpu.h>
#include <linux/notifier.h>
#include <linux/cpu.h>
#include <linux/mutex.h>
#include <linux/time.h>
#include <linux/kernel_stat.h>
#include <linux/wait.h>
#include <linux/kthread.h>
#include <linux/prefetch.h>
#include <linux/delay.h>
#include <linux/stop_machine.h>
#include <linux/random.h>

#include "rcutree.h"
#include <trace/events/rcu.h>

#include "rcu.h"

/*                  */

static struct lock_class_key rcu_node_class[RCU_NUM_LVLS];
static struct lock_class_key rcu_fqs_class[RCU_NUM_LVLS];

#define RCU_STATE_INITIALIZER(sname, sabbr, cr) { \
	.level = { &sname##_state.node[0] }, \
	.call = cr, \
	.fqs_state = RCU_GP_IDLE, \
	.gpnum = 0UL - 300UL, \
	.completed = 0UL - 300UL, \
	.orphan_lock = __RAW_SPIN_LOCK_UNLOCKED(&sname##_state.orphan_lock), \
	.orphan_nxttail = &sname##_state.orphan_nxtlist, \
	.orphan_donetail = &sname##_state.orphan_donelist, \
	.barrier_mutex = __MUTEX_INITIALIZER(sname##_state.barrier_mutex), \
	.onoff_mutex = __MUTEX_INITIALIZER(sname##_state.onoff_mutex), \
	.name = #sname, \
	.abbr = sabbr, \
}

struct rcu_state rcu_sched_state =
	RCU_STATE_INITIALIZER(rcu_sched, 's', call_rcu_sched);
DEFINE_PER_CPU(struct rcu_data, rcu_sched_data);

struct rcu_state rcu_bh_state = RCU_STATE_INITIALIZER(rcu_bh, 'b', call_rcu_bh);
DEFINE_PER_CPU(struct rcu_data, rcu_bh_data);

static struct rcu_state *rcu_state;
LIST_HEAD(rcu_struct_flavors);

/*                                                                      */
static int rcu_fanout_leaf = CONFIG_RCU_FANOUT_LEAF;
module_param(rcu_fanout_leaf, int, 0444);
int rcu_num_lvls __read_mostly = RCU_NUM_LVLS;
static int num_rcu_lvl[] = {  /*                                         */
	NUM_RCU_LVL_0,
	NUM_RCU_LVL_1,
	NUM_RCU_LVL_2,
	NUM_RCU_LVL_3,
	NUM_RCU_LVL_4,
};
int rcu_num_nodes __read_mostly = NUM_RCU_NODES; /*                           */

/*
                                                                      
                                                                        
                                                                       
                                                                          
                                                                         
                                                                         
                                             
 */
int rcu_scheduler_active __read_mostly;
EXPORT_SYMBOL_GPL(rcu_scheduler_active);

/*
                                                                       
                                                                       
                                                                     
                                                                        
                                                                    
                                                                          
  
                                                                          
                                                                         
          
 */
static int rcu_scheduler_fully_active __read_mostly;

#ifdef CONFIG_RCU_BOOST

/*
                                                                  
                             
 */
static DEFINE_PER_CPU(struct task_struct *, rcu_cpu_kthread_task);
DEFINE_PER_CPU(unsigned int, rcu_cpu_kthread_status);
DEFINE_PER_CPU(unsigned int, rcu_cpu_kthread_loops);
DEFINE_PER_CPU(char, rcu_cpu_has_work);

#endif /*                         */

static void rcu_boost_kthread_setaffinity(struct rcu_node *rnp, int outgoingcpu);
static void invoke_rcu_core(void);
static void invoke_rcu_callbacks(struct rcu_state *rsp, struct rcu_data *rdp);

/*
                                                                   
                                                                     
                                                                  
                                                                
                                                                       
                                                                
                           
 */
unsigned long rcutorture_testseq;
unsigned long rcutorture_vernum;

/*
                                                                         
                                                                       
                                                                      
 */
static int rcu_gp_in_progress(struct rcu_state *rsp)
{
	return ACCESS_ONCE(rsp->completed) != ACCESS_ONCE(rsp->gpnum);
}

/*
                                                          
                                                               
                                                                  
                                            
 */
void rcu_sched_qs(int cpu)
{
	struct rcu_data *rdp = &per_cpu(rcu_sched_data, cpu);

	if (rdp->passed_quiesce == 0)
		trace_rcu_grace_period("rcu_sched", rdp->gpnum, "cpuqs");
	rdp->passed_quiesce = 1;
}

void rcu_bh_qs(int cpu)
{
	struct rcu_data *rdp = &per_cpu(rcu_bh_data, cpu);

	if (rdp->passed_quiesce == 0)
		trace_rcu_grace_period("rcu_bh", rdp->gpnum, "cpuqs");
	rdp->passed_quiesce = 1;
}

/*
                                                                   
                                                     
                                            
 */
void rcu_note_context_switch(int cpu)
{
	trace_rcu_utilization("Start context switch");
	rcu_sched_qs(cpu);
	rcu_preempt_note_context_switch(cpu);
	trace_rcu_utilization("End context switch");
}
EXPORT_SYMBOL_GPL(rcu_note_context_switch);

DEFINE_PER_CPU(struct rcu_dynticks, rcu_dynticks) = {
	.dynticks_nesting = DYNTICK_TASK_EXIT_IDLE,
	.dynticks = ATOMIC_INIT(1),
};

static long blimit = 10;	/*                                     */
static long qhimark = 10000;	/*                                      */
static long qlowmark = 100;	/*                                          */

module_param(blimit, long, 0444);
module_param(qhimark, long, 0444);
module_param(qlowmark, long, 0444);

static ulong jiffies_till_first_fqs = RCU_JIFFIES_TILL_FORCE_QS;
static ulong jiffies_till_next_fqs = RCU_JIFFIES_TILL_FORCE_QS;

module_param(jiffies_till_first_fqs, ulong, 0644);
module_param(jiffies_till_next_fqs, ulong, 0644);

static void rcu_start_gp_advanced(struct rcu_state *rsp, struct rcu_node *rnp,
				  struct rcu_data *rdp);
static void force_qs_rnp(struct rcu_state *rsp, int (*f)(struct rcu_data *));
static void force_quiescent_state(struct rcu_state *rsp);
static int rcu_pending(int cpu);

/*
                                                                               
 */
long rcu_batches_completed_sched(void)
{
	return rcu_sched_state.completed;
}
EXPORT_SYMBOL_GPL(rcu_batches_completed_sched);

/*
                                                                            
 */
long rcu_batches_completed_bh(void)
{
	return rcu_bh_state.completed;
}
EXPORT_SYMBOL_GPL(rcu_batches_completed_bh);

/*
                                      
 */
void rcu_bh_force_quiescent_state(void)
{
	force_quiescent_state(&rcu_bh_state);
}
EXPORT_SYMBOL_GPL(rcu_bh_force_quiescent_state);

/*
                                                                      
                                                                       
                                                                         
                                                                      
                                         
 */
void rcutorture_record_test_transition(void)
{
	rcutorture_testseq++;
	rcutorture_vernum = 0;
}
EXPORT_SYMBOL_GPL(rcutorture_record_test_transition);

/*
                                                                          
                                                                           
            
 */
void rcutorture_record_progress(unsigned long vernum)
{
	rcutorture_vernum++;
}
EXPORT_SYMBOL_GPL(rcutorture_record_progress);

/*
                                         
 */
void rcu_sched_force_quiescent_state(void)
{
	force_quiescent_state(&rcu_sched_state);
}
EXPORT_SYMBOL_GPL(rcu_sched_force_quiescent_state);

/*
                                                   
 */
static int
cpu_has_callbacks_ready_to_invoke(struct rcu_data *rdp)
{
	return &rdp->nxtlist != rdp->nxttail[RCU_DONE_TAIL] &&
	       rdp->nxttail[RCU_DONE_TAIL] != NULL;
}

/*
                                                               
                                                                 
                            
 */
static int
cpu_needs_another_gp(struct rcu_state *rsp, struct rcu_data *rdp)
{
	int i;

	if (rcu_gp_in_progress(rsp))
		return 0;  /*                                            */
	if (rcu_nocb_needs_gp(rsp))
		return 1;  /*                              */
	if (!rdp->nxttail[RCU_NEXT_TAIL])
		return 0;  /*                                        */
	if (*rdp->nxttail[RCU_NEXT_READY_TAIL])
		return 1;  /*                                               */
	for (i = RCU_WAIT_TAIL; i < RCU_NEXT_TAIL; i++)
		if (rdp->nxttail[i - 1] != rdp->nxttail[i] &&
		    ULONG_CMP_LT(ACCESS_ONCE(rsp->completed),
				 rdp->nxtcompleted[i]))
			return 1;  /*                                   */
	return 0; /*                         */
}

/*
                                                             
 */
static struct rcu_node *rcu_get_root(struct rcu_state *rsp)
{
	return &rsp->node[0];
}

/*
                                                                                
  
                                                                  
                                                                       
                                            
 */
static void rcu_eqs_enter_common(struct rcu_dynticks *rdtp, long long oldval,
				bool user)
{
	trace_rcu_dyntick("Start", oldval, rdtp->dynticks_nesting);
	if (!user && !is_idle_task(current)) {
		struct task_struct *idle = idle_task(smp_processor_id());

		trace_rcu_dyntick("Error on entry: not idle task", oldval, 0);
		ftrace_dump(DUMP_ORIG);
		WARN_ONCE(1, "Current pid: %d comm: %s / Idle pid: %d comm: %s",
			  current->pid, current->comm,
			  idle->pid, idle->comm); /*                    */
	}
	rcu_prepare_for_idle(smp_processor_id());
	/*                                                                  */
	smp_mb__before_atomic_inc();  /*            */
	atomic_inc(&rdtp->dynticks);
	smp_mb__after_atomic_inc();  /*                                   */
	WARN_ON_ONCE(atomic_read(&rdtp->dynticks) & 0x1);

	/*
                                                            
                                         
  */
	rcu_lockdep_assert(!lock_is_held(&rcu_lock_map),
			   "Illegal idle entry in RCU read-side critical section.");
	rcu_lockdep_assert(!lock_is_held(&rcu_bh_lock_map),
			   "Illegal idle entry in RCU-bh read-side critical section.");
	rcu_lockdep_assert(!lock_is_held(&rcu_sched_lock_map),
			   "Illegal idle entry in RCU-sched read-side critical section.");
}

/*
                                                                 
                                                     
 */
static void rcu_eqs_enter(bool user)
{
	long long oldval;
	struct rcu_dynticks *rdtp;

	rdtp = &__get_cpu_var(rcu_dynticks);
	oldval = rdtp->dynticks_nesting;
	WARN_ON_ONCE((oldval & DYNTICK_TASK_NEST_MASK) == 0);
	if ((oldval & DYNTICK_TASK_NEST_MASK) == DYNTICK_TASK_NEST_VALUE)
		rdtp->dynticks_nesting = 0;
	else
		rdtp->dynticks_nesting -= DYNTICK_TASK_NEST_VALUE;
	rcu_eqs_enter_common(rdtp, oldval, user);
}

/* 
                                                                
  
                                                                 
                                                                
                                                                     
                                          
  
                                                               
                                                                 
                                                           
 */
void rcu_idle_enter(void)
{
	unsigned long flags;

	local_irq_save(flags);
	rcu_eqs_enter(false);
	local_irq_restore(flags);
}
EXPORT_SYMBOL_GPL(rcu_idle_enter);

#ifdef CONFIG_RCU_USER_QS
/* 
                                                              
  
                                                                      
                                                                   
                                                                     
                                  
 */
void rcu_user_enter(void)
{
	rcu_eqs_enter(1);
}

/* 
                                                                              
                                 
  
                                                                          
                                                                     
           
 */
void rcu_user_enter_after_irq(void)
{
	unsigned long flags;
	struct rcu_dynticks *rdtp;

	local_irq_save(flags);
	rdtp = &__get_cpu_var(rcu_dynticks);
	/*                                                        */
	WARN_ON_ONCE(!(rdtp->dynticks_nesting & DYNTICK_TASK_MASK));
	rdtp->dynticks_nesting = 1;
	local_irq_restore(flags);
}
#endif /*                    */

/* 
                                                                         
  
                                                                          
                                                                          
                      
  
                                                                      
                                                                     
                                                                    
                                                                     
  
                                                              
  
                        
 */
void rcu_irq_exit(void)
{
	unsigned long flags;
	long long oldval;
	struct rcu_dynticks *rdtp;

	local_irq_save(flags);
	rdtp = &__get_cpu_var(rcu_dynticks);
	oldval = rdtp->dynticks_nesting;
	rdtp->dynticks_nesting--;
	WARN_ON_ONCE(rdtp->dynticks_nesting < 0);
	if (rdtp->dynticks_nesting)
		trace_rcu_dyntick("--=", oldval, rdtp->dynticks_nesting);
	else
		rcu_eqs_enter_common(rdtp, oldval, true);
	local_irq_restore(flags);
}

/*
                                                                              
  
                                                                          
                                                                      
                                            
 */
static void rcu_eqs_exit_common(struct rcu_dynticks *rdtp, long long oldval,
			       int user)
{
	smp_mb__before_atomic_inc();  /*                                    */
	atomic_inc(&rdtp->dynticks);
	/*                                                                  */
	smp_mb__after_atomic_inc();  /*            */
	WARN_ON_ONCE(!(atomic_read(&rdtp->dynticks) & 0x1));
	rcu_cleanup_after_idle(smp_processor_id());
	trace_rcu_dyntick("End", oldval, rdtp->dynticks_nesting);
	if (!user && !is_idle_task(current)) {
		struct task_struct *idle = idle_task(smp_processor_id());

		trace_rcu_dyntick("Error on exit: not idle task",
				  oldval, rdtp->dynticks_nesting);
		ftrace_dump(DUMP_ORIG);
		WARN_ONCE(1, "Current pid: %d comm: %s / Idle pid: %d comm: %s",
			  current->pid, current->comm,
			  idle->pid, idle->comm); /*                    */
	}
}

/*
                                                                
                                                     
 */
static void rcu_eqs_exit(bool user)
{
	struct rcu_dynticks *rdtp;
	long long oldval;

	rdtp = &__get_cpu_var(rcu_dynticks);
	oldval = rdtp->dynticks_nesting;
	WARN_ON_ONCE(oldval < 0);
	if (oldval & DYNTICK_TASK_NEST_MASK)
		rdtp->dynticks_nesting += DYNTICK_TASK_NEST_VALUE;
	else
		rdtp->dynticks_nesting = DYNTICK_TASK_EXIT_IDLE;
	rcu_eqs_exit_common(rdtp, oldval, user);
}

/* 
                                                              
  
                                                                
                                         
  
                                                                  
                                                                     
                                                                 
                
 */
void rcu_idle_exit(void)
{
	unsigned long flags;

	local_irq_save(flags);
	rcu_eqs_exit(false);
	local_irq_restore(flags);
}
EXPORT_SYMBOL_GPL(rcu_idle_exit);

#ifdef CONFIG_RCU_USER_QS
/* 
                                                            
  
                                                              
                                                
 */
void rcu_user_exit(void)
{
	rcu_eqs_exit(1);
}

/* 
                                                                         
                                                       
  
                                                                   
                                                                        
                                                                           
                                           
 */
void rcu_user_exit_after_irq(void)
{
	unsigned long flags;
	struct rcu_dynticks *rdtp;

	local_irq_save(flags);
	rdtp = &__get_cpu_var(rcu_dynticks);
	/*                                              */
	WARN_ON_ONCE(rdtp->dynticks_nesting & DYNTICK_TASK_NEST_MASK);
	rdtp->dynticks_nesting += DYNTICK_TASK_EXIT_IDLE;
	local_irq_restore(flags);
}
#endif /*                    */

/* 
                                                                             
  
                                                                     
                                                                           
                      
  
                                                                       
                                                                 
                                                                         
                                                                          
                                                                         
                                                                      
                                                       
  
                                                              
  
                        
 */
void rcu_irq_enter(void)
{
	unsigned long flags;
	struct rcu_dynticks *rdtp;
	long long oldval;

	local_irq_save(flags);
	rdtp = &__get_cpu_var(rcu_dynticks);
	oldval = rdtp->dynticks_nesting;
	rdtp->dynticks_nesting++;
	WARN_ON_ONCE(rdtp->dynticks_nesting == 0);
	if (oldval)
		trace_rcu_dyntick("++=", oldval, rdtp->dynticks_nesting);
	else
		rcu_eqs_exit_common(rdtp, oldval, true);
	local_irq_restore(flags);
}

/* 
                                                     
  
                                                                 
                                                                  
                                                         
 */
void rcu_nmi_enter(void)
{
	struct rcu_dynticks *rdtp = &__get_cpu_var(rcu_dynticks);

	if (rdtp->dynticks_nmi_nesting == 0 &&
	    (atomic_read(&rdtp->dynticks) & 0x1))
		return;
	rdtp->dynticks_nmi_nesting++;
	smp_mb__before_atomic_inc();  /*                               */
	atomic_inc(&rdtp->dynticks);
	/*                                                                  */
	smp_mb__after_atomic_inc();  /*            */
	WARN_ON_ONCE(!(atomic_read(&rdtp->dynticks) & 0x1));
}

/* 
                                                     
  
                                                                 
                                                                  
                                                                   
 */
void rcu_nmi_exit(void)
{
	struct rcu_dynticks *rdtp = &__get_cpu_var(rcu_dynticks);

	if (rdtp->dynticks_nmi_nesting == 0 ||
	    --rdtp->dynticks_nmi_nesting != 0)
		return;
	/*                                                                  */
	smp_mb__before_atomic_inc();  /*            */
	atomic_inc(&rdtp->dynticks);
	smp_mb__after_atomic_inc();  /*                            */
	WARN_ON_ONCE(atomic_read(&rdtp->dynticks) & 0x1);
}

/* 
                                                                   
  
                                                                        
                               
 */
int rcu_is_cpu_idle(void)
{
	int ret;

	preempt_disable();
	ret = (atomic_read(&__get_cpu_var(rcu_dynticks).dynticks) & 0x1) == 0;
	preempt_enable();
	return ret;
}
EXPORT_SYMBOL(rcu_is_cpu_idle);

#if defined(CONFIG_PROVE_RCU) && defined(CONFIG_HOTPLUG_CPU)

/*
                                                                          
                                                                           
                                                                       
                                                                       
                                                                         
                                                                     
                                                                           
                                                                      
                                                                    
                                                                        
                                                                       
                                                                          
             
  
                                                               
                                                              
  
                                                                        
                                   
 */
bool rcu_lockdep_current_cpu_online(void)
{
	struct rcu_data *rdp;
	struct rcu_node *rnp;
	bool ret;

	if (in_nmi())
		return 1;
	preempt_disable();
	rdp = &__get_cpu_var(rcu_sched_data);
	rnp = rdp->mynode;
	ret = (rdp->grpmask & rnp->qsmaskinit) ||
	      !rcu_scheduler_fully_active;
	preempt_enable();
	return ret;
}
EXPORT_SYMBOL_GPL(rcu_lockdep_current_cpu_online);

#endif /*                                                              */

/* 
                                                                                
  
                                                                      
                                                                   
                       
 */
static int rcu_is_cpu_rrupt_from_idle(void)
{
	return __get_cpu_var(rcu_dynticks).dynticks_nesting <= 1;
}

/*
                                                                     
                                                                      
                                                                  
 */
static int dyntick_save_progress_counter(struct rcu_data *rdp)
{
	rdp->dynticks_snap = atomic_add_return(0, &rdp->dynticks->dynticks);
	return (rdp->dynticks_snap & 0x1) == 0;
}

/*
                                                                  
                                                                   
                                                                    
                                                          
 */
static int rcu_implicit_dynticks_qs(struct rcu_data *rdp)
{
	unsigned int curr;
	unsigned int snap;

	curr = (unsigned int)atomic_add_return(0, &rdp->dynticks->dynticks);
	snap = (unsigned int)rdp->dynticks_snap;

	/*
                                                                   
                                                                       
                                                                
                                                             
                                                                
                                    
  */
	if ((curr & 0x1) == 0 || UINT_CMP_GE(curr, snap + 2)) {
		trace_rcu_fqs(rdp->rsp->name, rdp->gpnum, rdp->cpu, "dti");
		rdp->dynticks_fqs++;
		return 1;
	}

	/*
                                                                 
                                                                 
                                                                
                    
   
                                                              
                                                                 
                                                                   
             
  */
	if (ULONG_CMP_GE(rdp->rsp->gp_start + 2, jiffies))
		return 0;  /*                                 */
	barrier();
	if (cpu_is_offline(rdp->cpu)) {
		trace_rcu_fqs(rdp->rsp->name, rdp->gpnum, rdp->cpu, "ofl");
		rdp->offline_fqs++;
		return 1;
	}

	/*
                                                             
                                                                   
                                                               
                                                              
                         
  */
	rcu_kick_nohz_cpu(rdp->cpu);

	return 0;
}

static void record_gp_stall_check_time(struct rcu_state *rsp)
{
	rsp->gp_start = jiffies;
	rsp->jiffies_stall = jiffies + rcu_jiffies_till_stall_check();
}

/*
                                                                        
                                                                       
                                                                    
                             
 */
static void rcu_dump_cpu_stacks(struct rcu_state *rsp)
{
	int cpu;
	unsigned long flags;
	struct rcu_node *rnp;

	rcu_for_each_leaf_node(rsp, rnp) {
		raw_spin_lock_irqsave(&rnp->lock, flags);
		if (rnp->qsmask != 0) {
			for (cpu = 0; cpu <= rnp->grphi - rnp->grplo; cpu++)
				if (rnp->qsmask & (1UL << cpu))
					dump_cpu_task(rnp->grplo + cpu);
		}
		raw_spin_unlock_irqrestore(&rnp->lock, flags);
	}
}

static void print_other_cpu_stall(struct rcu_state *rsp)
{
	int cpu;
	long delta;
	unsigned long flags;
	int ndetected = 0;
	struct rcu_node *rnp = rcu_get_root(rsp);
	long totqlen = 0;

	/*                                                           */

	raw_spin_lock_irqsave(&rnp->lock, flags);
	delta = jiffies - rsp->jiffies_stall;
	if (delta < RCU_STALL_RAT_DELAY || !rcu_gp_in_progress(rsp)) {
		raw_spin_unlock_irqrestore(&rnp->lock, flags);
		return;
	}
	rsp->jiffies_stall = jiffies + 3 * rcu_jiffies_till_stall_check() + 3;
	raw_spin_unlock_irqrestore(&rnp->lock, flags);

	/*
                                   
                                                                
                           
  */
	printk(KERN_ERR "INFO: %s detected stalls on CPUs/tasks:",
	       rsp->name);
	print_cpu_stall_info_begin();
	rcu_for_each_leaf_node(rsp, rnp) {
		raw_spin_lock_irqsave(&rnp->lock, flags);
		ndetected += rcu_print_task_stall(rnp);
		if (rnp->qsmask != 0) {
			for (cpu = 0; cpu <= rnp->grphi - rnp->grplo; cpu++)
				if (rnp->qsmask & (1UL << cpu)) {
					print_cpu_stall_info(rsp,
							     rnp->grplo + cpu);
					ndetected++;
				}
		}
		raw_spin_unlock_irqrestore(&rnp->lock, flags);
	}

	/*
                                                                
                         
  */
	rnp = rcu_get_root(rsp);
	raw_spin_lock_irqsave(&rnp->lock, flags);
	ndetected += rcu_print_task_stall(rnp);
	raw_spin_unlock_irqrestore(&rnp->lock, flags);

	print_cpu_stall_info_end();
	for_each_possible_cpu(cpu)
		totqlen += per_cpu_ptr(rsp->rda, cpu)->qlen;
	pr_cont("(detected by %d, t=%ld jiffies, g=%lu, c=%lu, q=%lu)\n",
	       smp_processor_id(), (long)(jiffies - rsp->gp_start),
	       rsp->gpnum, rsp->completed, totqlen);
	if (ndetected == 0)
		printk(KERN_ERR "INFO: Stall ended before state dump start\n");
	else if (!trigger_all_cpu_backtrace())
		rcu_dump_cpu_stacks(rsp);

	/*                                                 */

	rcu_print_detail_task_stall(rsp);

	force_quiescent_state(rsp);  /*                */
}

static void print_cpu_stall(struct rcu_state *rsp)
{
	int cpu;
	unsigned long flags;
	struct rcu_node *rnp = rcu_get_root(rsp);
	long totqlen = 0;

	/*
                                   
                                                                
                           
  */
	printk(KERN_ERR "INFO: %s self-detected stall on CPU", rsp->name);
	print_cpu_stall_info_begin();
	print_cpu_stall_info(rsp, smp_processor_id());
	print_cpu_stall_info_end();
	for_each_possible_cpu(cpu)
		totqlen += per_cpu_ptr(rsp->rda, cpu)->qlen;
	pr_cont(" (t=%lu jiffies g=%lu c=%lu q=%lu)\n",
		jiffies - rsp->gp_start, rsp->gpnum, rsp->completed, totqlen);
	if (!trigger_all_cpu_backtrace())
		dump_stack();

	raw_spin_lock_irqsave(&rnp->lock, flags);
	if (ULONG_CMP_GE(jiffies, rsp->jiffies_stall))
		rsp->jiffies_stall = jiffies +
				     3 * rcu_jiffies_till_stall_check() + 3;
	raw_spin_unlock_irqrestore(&rnp->lock, flags);

	set_need_resched();  /*                                     */
}

static void check_cpu_stall(struct rcu_state *rsp, struct rcu_data *rdp)
{
	unsigned long j;
	unsigned long js;
	struct rcu_node *rnp;

	if (rcu_cpu_stall_suppress)
		return;
	j = ACCESS_ONCE(jiffies);
	js = ACCESS_ONCE(rsp->jiffies_stall);
	rnp = rdp->mynode;
	if (rcu_gp_in_progress(rsp) &&
	    (ACCESS_ONCE(rnp->qsmask) & rdp->grpmask) && ULONG_CMP_GE(j, js)) {

		/*                                          */
		print_cpu_stall(rsp);

	} else if (rcu_gp_in_progress(rsp) &&
		   ULONG_CMP_GE(j, js + RCU_STALL_RAT_DELAY)) {

		/*                                                       */
		print_other_cpu_stall(rsp);
	}
}

/* 
                                                                               
  
                                                                         
                                                                          
                     
  
                                     
 */
void rcu_cpu_stall_reset(void)
{
	struct rcu_state *rsp;

	for_each_rcu_flavor(rsp)
		rsp->jiffies_stall = jiffies + ULONG_MAX / 2;
}

/*
                                                                            
                                                                        
                                                                        
                                                                          
                                
 */
static void __note_new_gpnum(struct rcu_state *rsp, struct rcu_node *rnp, struct rcu_data *rdp)
{
	if (rdp->gpnum != rnp->gpnum) {
		/*
                                                         
                                                        
                        
   */
		rdp->gpnum = rnp->gpnum;
		trace_rcu_grace_period(rsp->name, rdp->gpnum, "cpustart");
		rdp->passed_quiesce = 0;
		rdp->qs_pending = !!(rnp->qsmask & rdp->grpmask);
		zero_cpu_stall_ticks(rdp);
	}
}

static void note_new_gpnum(struct rcu_state *rsp, struct rcu_data *rdp)
{
	unsigned long flags;
	struct rcu_node *rnp;

	local_irq_save(flags);
	rnp = rdp->mynode;
	if (rdp->gpnum == ACCESS_ONCE(rnp->gpnum) || /*               */
	    !raw_spin_trylock(&rnp->lock)) { /*                             */
		local_irq_restore(flags);
		return;
	}
	__note_new_gpnum(rsp, rnp, rdp);
	raw_spin_unlock_irqrestore(&rnp->lock, flags);
}

/*
                                                                    
                                                                    
                                   
 */
static int
check_for_new_grace_period(struct rcu_state *rsp, struct rcu_data *rdp)
{
	unsigned long flags;
	int ret = 0;

	local_irq_save(flags);
	if (rdp->gpnum != rsp->gpnum) {
		note_new_gpnum(rsp, rdp);
		ret = 1;
	}
	local_irq_restore(flags);
	return ret;
}

/*
                                                                        
 */
static void init_callback_list(struct rcu_data *rdp)
{
	int i;

	if (init_nocb_callback_list(rdp))
		return;
	rdp->nxtlist = NULL;
	for (i = 0; i < RCU_NEXT_SIZE; i++)
		rdp->nxttail[i] = &rdp->nxtlist;
}

/*
                                                                   
                                                                       
                                                                      
                                                                    
                               
  
                                                           
 */
static unsigned long rcu_cbs_completed(struct rcu_state *rsp,
				       struct rcu_node *rnp)
{
	/*
                                                           
                                                              
                                                            
                                                             
                                                            
  */
	if (rcu_get_root(rsp) == rnp && rnp->gpnum == rnp->completed)
		return rnp->completed + 1;

	/*
                                                           
                                          
  */
	return rnp->completed + 2;
}

/*
                                                            
                      
 */
static void trace_rcu_future_gp(struct rcu_node *rnp, struct rcu_data *rdp,
				unsigned long c, char *s)
{
	trace_rcu_future_grace_period(rdp->rsp->name, rnp->gpnum,
				      rnp->completed, c, rnp->level,
				      rnp->grplo, rnp->grphi, s);
}

/*
                                                                    
                                                                     
                                               
  
                                                                  
 */
static unsigned long __maybe_unused
rcu_start_future_gp(struct rcu_node *rnp, struct rcu_data *rdp)
{
	unsigned long c;
	int i;
	struct rcu_node *rnp_root = rcu_get_root(rdp->rsp);

	/*
                                                           
                                                                   
  */
	c = rcu_cbs_completed(rdp->rsp, rnp);
	trace_rcu_future_gp(rnp, rdp, c, "Startleaf");
	if (rnp->need_future_gp[c & 0x1]) {
		trace_rcu_future_gp(rnp, rdp, c, "Prestartleaf");
		return c;
	}

	/*
                                                                    
                                                                 
                                                                
                                                                    
                                 
  */
	if (rnp->gpnum != rnp->completed ||
	    ACCESS_ONCE(rnp->gpnum) != ACCESS_ONCE(rnp->completed)) {
		rnp->need_future_gp[c & 0x1]++;
		trace_rcu_future_gp(rnp, rdp, c, "Startedleaf");
		return c;
	}

	/*
                                                                    
                                                                   
                          
  */
	if (rnp != rnp_root)
		raw_spin_lock(&rnp_root->lock);

	/*
                                                               
                                                                   
                                                                
                                                                   
  */
	c = rcu_cbs_completed(rdp->rsp, rnp_root);
	for (i = RCU_DONE_TAIL; i < RCU_NEXT_TAIL; i++)
		if (ULONG_CMP_LT(c, rdp->nxtcompleted[i]))
			rdp->nxtcompleted[i] = c;

	/*
                                                          
                              
  */
	if (rnp_root->need_future_gp[c & 0x1]) {
		trace_rcu_future_gp(rnp, rdp, c, "Prestartedroot");
		goto unlock_out;
	}

	/*                                              */
	rnp_root->need_future_gp[c & 0x1]++;

	/*                                                          */
	if (rnp_root->gpnum != rnp_root->completed) {
		trace_rcu_future_gp(rnp, rdp, c, "Startedleafroot");
	} else {
		trace_rcu_future_gp(rnp, rdp, c, "Startedroot");
		rcu_start_gp_advanced(rdp->rsp, rnp_root, rdp);
	}
unlock_out:
	if (rnp != rnp_root)
		raw_spin_unlock(&rnp_root->lock);
	return c;
}

/*
                                                                          
                                                                         
                                                                      
                                             
 */
static int rcu_future_gp_cleanup(struct rcu_state *rsp, struct rcu_node *rnp)
{
	int c = rnp->completed;
	int needmore;
	struct rcu_data *rdp = this_cpu_ptr(rsp->rda);

	rcu_nocb_gp_cleanup(rsp, rnp);
	rnp->need_future_gp[c & 0x1] = 0;
	needmore = rnp->need_future_gp[(c + 1) & 0x1];
	trace_rcu_future_gp(rnp, rdp, c, needmore ? "CleanupMore" : "Cleanup");
	return needmore;
}

/*
                                                                    
                                                                     
                                                                        
                                                                         
                                                                         
                                                                          
                                  
  
                                                           
 */
static void rcu_accelerate_cbs(struct rcu_state *rsp, struct rcu_node *rnp,
			       struct rcu_data *rdp)
{
	unsigned long c;
	int i;

	/*                                             */
	if (!rdp->nxttail[RCU_NEXT_TAIL] || !*rdp->nxttail[RCU_DONE_TAIL])
		return;

	/*
                                                           
                                                                     
                                                                     
                                                                
                                                                
                                                                   
   
                                                               
                                                                 
                                                                 
                                                                   
                                       
  */
	c = rcu_cbs_completed(rsp, rnp);
	for (i = RCU_NEXT_TAIL - 1; i > RCU_DONE_TAIL; i--)
		if (rdp->nxttail[i] != rdp->nxttail[i - 1] &&
		    !ULONG_CMP_GE(rdp->nxtcompleted[i], c))
			break;

	/*
                                                            
                                                               
                                                                   
                    
  */
	if (++i >= RCU_NEXT_TAIL)
		return;

	/*
                                                                   
                                                                 
                   
  */
	for (; i <= RCU_NEXT_TAIL; i++) {
		rdp->nxttail[i] = rdp->nxttail[RCU_NEXT_TAIL];
		rdp->nxtcompleted[i] = c;
	}
	/*                                             */
	rcu_start_future_gp(rnp, rdp);

	/*                                                         */
	if (!*rdp->nxttail[RCU_WAIT_TAIL])
		trace_rcu_grace_period(rsp->name, rdp->gpnum, "AccWaitCB");
	else
		trace_rcu_grace_period(rsp->name, rdp->gpnum, "AccReadyCB");
}

/*
                                                             
                                                                 
                                                                   
                                                                
                                                                     
  
                                                           
 */
static void rcu_advance_cbs(struct rcu_state *rsp, struct rcu_node *rnp,
			    struct rcu_data *rdp)
{
	int i, j;

	/*                                             */
	if (!rdp->nxttail[RCU_NEXT_TAIL] || !*rdp->nxttail[RCU_DONE_TAIL])
		return;

	/*
                                                                   
                                                                     
  */
	for (i = RCU_WAIT_TAIL; i < RCU_NEXT_TAIL; i++) {
		if (ULONG_CMP_LT(rnp->completed, rdp->nxtcompleted[i]))
			break;
		rdp->nxttail[RCU_DONE_TAIL] = rdp->nxttail[i];
	}
	/*                                                                */
	for (j = RCU_WAIT_TAIL; j < i; j++)
		rdp->nxttail[j] = rdp->nxttail[RCU_DONE_TAIL];

	/*                                                */
	for (j = RCU_WAIT_TAIL; i < RCU_NEXT_TAIL; i++, j++) {
		if (rdp->nxttail[j] == rdp->nxttail[RCU_NEXT_TAIL])
			break;
		rdp->nxttail[j] = rdp->nxttail[i];
		rdp->nxtcompleted[j] = rdp->nxtcompleted[i];
	}

	/*                                   */
	rcu_accelerate_cbs(rsp, rnp, rdp);
}

/*
                                                                     
                                                                   
                                                                     
                                                         
 */
static void
__rcu_process_gp_end(struct rcu_state *rsp, struct rcu_node *rnp, struct rcu_data *rdp)
{
	/*                               */
	if (rdp->completed == rnp->completed) {

		/*                                          */
		rcu_accelerate_cbs(rsp, rnp, rdp);

	} else {

		/*                    */
		rcu_advance_cbs(rsp, rnp, rdp);

		/*                                                    */
		rdp->completed = rnp->completed;
		trace_rcu_grace_period(rsp->name, rdp->gpnum, "cpuend");

		/*
                                                           
                                                          
                                                         
                                                         
                                                        
                                                            
                                                    
   */
		if (ULONG_CMP_LT(rdp->gpnum, rdp->completed)) {
			rdp->gpnum = rdp->completed;
			rdp->passed_quiesce = 0;
		}

		/*
                                                          
                                                             
   */
		if ((rnp->qsmask & rdp->grpmask) == 0)
			rdp->qs_pending = 0;
	}
}

/*
                                                                     
                                                                   
           
 */
static void
rcu_process_gp_end(struct rcu_state *rsp, struct rcu_data *rdp)
{
	unsigned long flags;
	struct rcu_node *rnp;

	local_irq_save(flags);
	rnp = rdp->mynode;
	if (rdp->completed == ACCESS_ONCE(rnp->completed) || /*               */
	    !raw_spin_trylock(&rnp->lock)) { /*                             */
		local_irq_restore(flags);
		return;
	}
	__rcu_process_gp_end(rsp, rnp, rdp);
	raw_spin_unlock_irqrestore(&rnp->lock, flags);
}

/*
                                                                      
                                                                     
            
 */
static void
rcu_start_gp_per_cpu(struct rcu_state *rsp, struct rcu_node *rnp, struct rcu_data *rdp)
{
	/*                                                                 */
	__rcu_process_gp_end(rsp, rnp, rdp);

	/*                                                                  */
	__note_new_gpnum(rsp, rnp, rdp);
}

/*
                                 
 */
static int rcu_gp_init(struct rcu_state *rsp)
{
	struct rcu_data *rdp;
	struct rcu_node *rnp = rcu_get_root(rsp);

	raw_spin_lock_irq(&rnp->lock);
	rsp->gp_flags = 0; /*                                    */

	if (rcu_gp_in_progress(rsp)) {
		/*                                                         */
		raw_spin_unlock_irq(&rnp->lock);
		return 0;
	}

	/*                                                     */
	rsp->gpnum++;
	trace_rcu_grace_period(rsp->name, rsp->gpnum, "start");
	record_gp_stall_check_time(rsp);
	raw_spin_unlock_irq(&rnp->lock);

	/*                                                */
	mutex_lock(&rsp->onoff_mutex);

	/*
                                                           
                                                                    
                                                                    
                                                                   
                                                                     
                                                                 
                                                                  
                           
   
                                                             
                                                        
  */
	rcu_for_each_node_breadth_first(rsp, rnp) {
		raw_spin_lock_irq(&rnp->lock);
		rdp = this_cpu_ptr(rsp->rda);
		rcu_preempt_check_blocked_tasks(rnp);
		rnp->qsmask = rnp->qsmaskinit;
		ACCESS_ONCE(rnp->gpnum) = rsp->gpnum;
		WARN_ON_ONCE(rnp->completed != rsp->completed);
		ACCESS_ONCE(rnp->completed) = rsp->completed;
		if (rnp == rdp->mynode)
			rcu_start_gp_per_cpu(rsp, rnp, rdp);
		rcu_preempt_boost_start_gp(rnp);
		trace_rcu_grace_period_init(rsp->name, rnp->gpnum,
					    rnp->level, rnp->grplo,
					    rnp->grphi, rnp->qsmask);
		raw_spin_unlock_irq(&rnp->lock);
#ifdef CONFIG_PROVE_RCU_DELAY
		if ((prandom_u32() % (rcu_num_nodes + 1)) == 0 &&
		    system_state == SYSTEM_RUNNING)
			udelay(200);
#endif /*                               */
		cond_resched();
	}

	mutex_unlock(&rsp->onoff_mutex);
	return 1;
}

/*
                                           
 */
int rcu_gp_fqs(struct rcu_state *rsp, int fqs_state_in)
{
	int fqs_state = fqs_state_in;
	struct rcu_node *rnp = rcu_get_root(rsp);

	rsp->n_force_qs++;
	if (fqs_state == RCU_SAVE_DYNTICK) {
		/*                                 */
		force_qs_rnp(rsp, dyntick_save_progress_counter);
		fqs_state = RCU_FORCE_QS;
	} else {
		/*                                       */
		force_qs_rnp(rsp, rcu_implicit_dynticks_qs);
	}
	/*                                           */
	if (ACCESS_ONCE(rsp->gp_flags) & RCU_GP_FLAG_FQS) {
		raw_spin_lock_irq(&rnp->lock);
		rsp->gp_flags &= ~RCU_GP_FLAG_FQS;
		raw_spin_unlock_irq(&rnp->lock);
	}
	return fqs_state;
}

/*
                                       
 */
static void rcu_gp_cleanup(struct rcu_state *rsp)
{
	unsigned long gp_duration;
	int nocb = 0;
	struct rcu_data *rdp;
	struct rcu_node *rnp = rcu_get_root(rsp);

	raw_spin_lock_irq(&rnp->lock);
	gp_duration = jiffies - rsp->gp_start;
	if (gp_duration > rsp->gp_max)
		rsp->gp_max = gp_duration;

	/*
                                                              
                                                            
                                                             
                                                             
                                                           
                                                          
  */
	raw_spin_unlock_irq(&rnp->lock);

	/*
                                                             
                                                                  
                                                              
                                                               
                                                                    
                                                                   
                                                               
  */
	rcu_for_each_node_breadth_first(rsp, rnp) {
		raw_spin_lock_irq(&rnp->lock);
		ACCESS_ONCE(rnp->completed) = rsp->gpnum;
		rdp = this_cpu_ptr(rsp->rda);
		if (rnp == rdp->mynode)
			__rcu_process_gp_end(rsp, rnp, rdp);
		nocb += rcu_future_gp_cleanup(rsp, rnp);
		raw_spin_unlock_irq(&rnp->lock);
		cond_resched();
	}
	rnp = rcu_get_root(rsp);
	raw_spin_lock_irq(&rnp->lock);
	rcu_nocb_gp_set(rnp, nocb);

	rsp->completed = rsp->gpnum; /*                            */
	trace_rcu_grace_period(rsp->name, rsp->completed, "end");
	rsp->fqs_state = RCU_GP_IDLE;
	rdp = this_cpu_ptr(rsp->rda);
	rcu_advance_cbs(rsp, rnp, rdp);  /*                               */
	if (cpu_needs_another_gp(rsp, rdp))
		rsp->gp_flags = 1;
	raw_spin_unlock_irq(&rnp->lock);
}

/*
                                              
 */
static int __noreturn rcu_gp_kthread(void *arg)
{
	int fqs_state;
	unsigned long j;
	int ret;
	struct rcu_state *rsp = arg;
	struct rcu_node *rnp = rcu_get_root(rsp);

	for (;;) {

		/*                            */
		for (;;) {
			wait_event_interruptible(rsp->gp_wq,
						 rsp->gp_flags &
						 RCU_GP_FLAG_INIT);
			if ((rsp->gp_flags & RCU_GP_FLAG_INIT) &&
			    rcu_gp_init(rsp))
				break;
			cond_resched();
			flush_signals(current);
		}

		/*                                 */
		fqs_state = RCU_SAVE_DYNTICK;
		j = jiffies_till_first_fqs;
		if (j > HZ) {
			j = HZ;
			jiffies_till_first_fqs = HZ;
		}
		for (;;) {
			rsp->jiffies_force_qs = jiffies + j;
			ret = wait_event_interruptible_timeout(rsp->gp_wq,
					(rsp->gp_flags & RCU_GP_FLAG_FQS) ||
					(!ACCESS_ONCE(rnp->qsmask) &&
					 !rcu_preempt_blocked_readers_cgp(rnp)),
					j);
			/*                                   */
			if (!ACCESS_ONCE(rnp->qsmask) &&
			    !rcu_preempt_blocked_readers_cgp(rnp))
				break;
			/*                                             */
			if (ret == 0 || (rsp->gp_flags & RCU_GP_FLAG_FQS)) {
				fqs_state = rcu_gp_fqs(rsp, fqs_state);
				cond_resched();
			} else {
				/*                         */
				cond_resched();
				flush_signals(current);
			}
			j = jiffies_till_next_fqs;
			if (j > HZ) {
				j = HZ;
				jiffies_till_next_fqs = HZ;
			} else if (j < 1) {
				j = 1;
				jiffies_till_next_fqs = 1;
			}
		}

		/*                          */
		rcu_gp_cleanup(rsp);
	}
}

static void rsp_wakeup(struct irq_work *work)
{
	struct rcu_state *rsp = container_of(work, struct rcu_state, wakeup_work);

	/*                                                     */
	wake_up(&rsp->gp_wq);
}

/*
                                                                           
                                                                            
                                                         
  
                                                                        
                                                                        
                   
 */
static void
rcu_start_gp_advanced(struct rcu_state *rsp, struct rcu_node *rnp,
		      struct rcu_data *rdp)
{
	if (!rsp->gp_kthread || !cpu_needs_another_gp(rsp, rdp)) {
		/*
                                                    
                                                       
                                              
                                                
   */
		return;
	}
	rsp->gp_flags = RCU_GP_FLAG_INIT;

	/*
                                                            
                                                           
                                    
  */
	irq_work_queue(&rsp->wakeup_work);
}

/*
                                                                         
                                                                          
                                                                      
                                                                         
                                  
 */
static void
rcu_start_gp(struct rcu_state *rsp)
{
	struct rcu_data *rdp = this_cpu_ptr(rsp->rda);
	struct rcu_node *rnp = rcu_get_root(rsp);

	/*
                                                          
                                                               
                                                                 
                                                              
                                                                
                                
  */
	rcu_advance_cbs(rsp, rnp, rdp);
	rcu_start_gp_advanced(rsp, rnp, rdp);
}

/*
                                                                   
                                                                   
                                                                   
                                                                     
                             
 */
static void rcu_report_qs_rsp(struct rcu_state *rsp, unsigned long flags)
	__releases(rcu_get_root(rsp)->lock)
{
	WARN_ON_ONCE(!rcu_gp_in_progress(rsp));
	raw_spin_unlock_irqrestore(&rcu_get_root(rsp)->lock, flags);
	wake_up(&rsp->gp_wq);  /*                                           */
}

/*
                                                                     
                                                                       
                                                                        
                                                                        
                                                                         
                                                                  
 */
static void
rcu_report_qs_rnp(unsigned long mask, struct rcu_state *rsp,
		  struct rcu_node *rnp, unsigned long flags)
	__releases(rnp->lock)
{
	struct rcu_node *rnp_c;

	/*                                 */
	for (;;) {
		if (!(rnp->qsmask & mask)) {

			/*                                            */
			raw_spin_unlock_irqrestore(&rnp->lock, flags);
			return;
		}
		rnp->qsmask &= ~mask;
		trace_rcu_quiescent_state_report(rsp->name, rnp->gpnum,
						 mask, rnp->qsmask, rnp->level,
						 rnp->grplo, rnp->grphi,
						 !!rnp->gp_tasks);
		if (rnp->qsmask != 0 || rcu_preempt_blocked_readers_cgp(rnp)) {

			/*                                              */
			raw_spin_unlock_irqrestore(&rnp->lock, flags);
			return;
		}
		mask = rnp->grpmask;
		if (rnp->parent == NULL) {

			/*                                               */

			break;
		}
		raw_spin_unlock_irqrestore(&rnp->lock, flags);
		rnp_c = rnp;
		rnp = rnp->parent;
		raw_spin_lock_irqsave(&rnp->lock, flags);
		WARN_ON_ONCE(rnp_c->qsmask);
	}

	/*
                                                               
                                                            
                                                                 
  */
	rcu_report_qs_rsp(rsp, flags); /*                     */
}

/*
                                                                        
                                                                    
                                                                       
                                                                          
                                                                        
                                                                           
                                                                 
 */
static void
rcu_report_qs_rdp(int cpu, struct rcu_state *rsp, struct rcu_data *rdp)
{
	unsigned long flags;
	unsigned long mask;
	struct rcu_node *rnp;

	rnp = rdp->mynode;
	raw_spin_lock_irqsave(&rnp->lock, flags);
	if (rdp->passed_quiesce == 0 || rdp->gpnum != rnp->gpnum ||
	    rnp->completed == rnp->gpnum) {

		/*
                                                       
                                                    
                                                         
                                     
   */
		rdp->passed_quiesce = 0;	/*                     */
		raw_spin_unlock_irqrestore(&rnp->lock, flags);
		return;
	}
	mask = rdp->grpmask;
	if ((rnp->qsmask & mask) == 0) {
		raw_spin_unlock_irqrestore(&rnp->lock, flags);
	} else {
		rdp->qs_pending = 0;

		/*
                                                         
                                                   
   */
		rcu_accelerate_cbs(rsp, rnp, rdp);

		rcu_report_qs_rnp(mask, rsp, rnp, flags); /*                 */
	}
}

/*
                                                                
                                                                   
                                                               
                                                                     
 */
static void
rcu_check_quiescent_state(struct rcu_state *rsp, struct rcu_data *rdp)
{
	/*                                                        */
	if (check_for_new_grace_period(rsp, rdp))
		return;

	/*
                                                                     
                                                               
  */
	if (!rdp->qs_pending)
		return;

	/*
                                                                
                                                        
  */
	if (!rdp->passed_quiesce)
		return;

	/*
                                                             
                   
  */
	rcu_report_qs_rdp(rdp->cpu, rsp, rdp);
}

#ifdef CONFIG_HOTPLUG_CPU

/*
                                                                
                                                              
                 
 */
static void
rcu_send_cbs_to_orphanage(int cpu, struct rcu_state *rsp,
			  struct rcu_node *rnp, struct rcu_data *rdp)
{
	/*                                               */
	if (rcu_is_nocb_cpu(rdp->cpu))
		return;

	/*
                                                                 
                                                                 
                                                               
  */
	if (rdp->nxtlist != NULL) {
		rsp->qlen_lazy += rdp->qlen_lazy;
		rsp->qlen += rdp->qlen;
		rdp->n_cbs_orphaned += rdp->qlen;
		rdp->qlen_lazy = 0;
		ACCESS_ONCE(rdp->qlen) = 0;
	}

	/*
                                                              
                                                          
                                                                 
                                                                   
                                                                  
                                                               
                                           
  */
	if (*rdp->nxttail[RCU_DONE_TAIL] != NULL) {
		*rsp->orphan_nxttail = *rdp->nxttail[RCU_DONE_TAIL];
		rsp->orphan_nxttail = rdp->nxttail[RCU_NEXT_TAIL];
		*rdp->nxttail[RCU_DONE_TAIL] = NULL;
	}

	/*
                                                             
                                                              
                                                                
  */
	if (rdp->nxtlist != NULL) {
		*rsp->orphan_donetail = rdp->nxtlist;
		rsp->orphan_donetail = rdp->nxttail[RCU_DONE_TAIL];
	}

	/*                                                              */
	init_callback_list(rdp);
}

/*
                                                                   
                                                      
 */
static void rcu_adopt_orphan_cbs(struct rcu_state *rsp)
{
	int i;
	struct rcu_data *rdp = __this_cpu_ptr(rsp->rda);

	/*                                    */
	if (rcu_nocb_adopt_orphan_cbs(rsp, rdp))
		return;

	/*                          */
	rdp->qlen_lazy += rsp->qlen_lazy;
	rdp->qlen += rsp->qlen;
	rdp->n_cbs_adopted += rsp->qlen;
	if (rsp->qlen_lazy != rsp->qlen)
		rcu_idle_count_callbacks_posted();
	rsp->qlen_lazy = 0;
	rsp->qlen = 0;

	/*
                                                                
                                                             
                                            
  */

	/*                                            */
	if (rsp->orphan_donelist != NULL) {
		*rsp->orphan_donetail = *rdp->nxttail[RCU_DONE_TAIL];
		*rdp->nxttail[RCU_DONE_TAIL] = rsp->orphan_donelist;
		for (i = RCU_NEXT_SIZE - 1; i >= RCU_DONE_TAIL; i--)
			if (rdp->nxttail[i] == rdp->nxttail[RCU_DONE_TAIL])
				rdp->nxttail[i] = rsp->orphan_donetail;
		rsp->orphan_donelist = NULL;
		rsp->orphan_donetail = &rsp->orphan_donelist;
	}

	/*                                                              */
	if (rsp->orphan_nxtlist != NULL) {
		*rdp->nxttail[RCU_NEXT_TAIL] = rsp->orphan_nxtlist;
		rdp->nxttail[RCU_NEXT_TAIL] = rsp->orphan_nxttail;
		rsp->orphan_nxtlist = NULL;
		rsp->orphan_nxttail = &rsp->orphan_nxtlist;
	}
}

/*
                                                 
 */
static void rcu_cleanup_dying_cpu(struct rcu_state *rsp)
{
	RCU_TRACE(unsigned long mask);
	RCU_TRACE(struct rcu_data *rdp = this_cpu_ptr(rsp->rda));
	RCU_TRACE(struct rcu_node *rnp = rdp->mynode);

	RCU_TRACE(mask = rdp->grpmask);
	trace_rcu_grace_period(rsp->name,
			       rnp->gpnum + 1 - !!(rnp->qsmask & mask),
			       "cpuofl");
}

/*
                                                                       
                                                                    
                                                                 
                                                                         
                                                                    
 */
static void rcu_cleanup_dead_cpu(int cpu, struct rcu_state *rsp)
{
	unsigned long flags;
	unsigned long mask;
	int need_report = 0;
	struct rcu_data *rdp = per_cpu_ptr(rsp->rda, cpu);
	struct rcu_node *rnp = rdp->mynode;  /*                           */

	/*                                       */
	rcu_boost_kthread_setaffinity(rnp, -1);

	/*                                                                  */

	/*                                                   */
	mutex_lock(&rsp->onoff_mutex);
	raw_spin_lock_irqsave(&rsp->orphan_lock, flags);

	/*                                                                 */
	rcu_send_cbs_to_orphanage(cpu, rsp, rnp, rdp);
	rcu_adopt_orphan_cbs(rsp);

	/*                                                                   */
	mask = rdp->grpmask;	/*                         */
	do {
		raw_spin_lock(&rnp->lock);	/*                        */
		rnp->qsmaskinit &= ~mask;
		if (rnp->qsmaskinit != 0) {
			if (rnp != rdp->mynode)
				raw_spin_unlock(&rnp->lock); /*                       */
			break;
		}
		if (rnp == rdp->mynode)
			need_report = rcu_preempt_offline_tasks(rsp, rnp, rdp);
		else
			raw_spin_unlock(&rnp->lock); /*                       */
		mask = rnp->grpmask;
		rnp = rnp->parent;
	} while (rnp != NULL);

	/*
                                                            
                                                               
                                                                   
                           
  */
	raw_spin_unlock(&rsp->orphan_lock); /*                       */
	rnp = rdp->mynode;
	if (need_report & RCU_OFL_TASKS_NORM_GP)
		rcu_report_unblock_qs_rnp(rnp, flags);
	else
		raw_spin_unlock_irqrestore(&rnp->lock, flags);
	if (need_report & RCU_OFL_TASKS_EXP_GP)
		rcu_report_exp_rnp(rsp, rnp, true);
	WARN_ONCE(rdp->qlen != 0 || rdp->nxtlist != NULL,
		  "rcu_cleanup_dead_cpu: Callbacks on offline CPU %d: qlen=%lu, nxtlist=%p\n",
		  cpu, rdp->qlen, rdp->nxtlist);
	init_callback_list(rdp);
	/*                                         */
	rdp->nxttail[RCU_NEXT_TAIL] = NULL;
	mutex_unlock(&rsp->onoff_mutex);
}

#else /*                           */

static void rcu_cleanup_dying_cpu(struct rcu_state *rsp)
{
}

static void rcu_cleanup_dead_cpu(int cpu, struct rcu_state *rsp)
{
}

#endif /*                                 */

/*
                                                                       
                                                
 */
static void rcu_do_batch(struct rcu_state *rsp, struct rcu_data *rdp)
{
	unsigned long flags;
	struct rcu_head *next, *list, **tail;
	long bl, count, count_lazy;
	int i;

	/*                                         */
	if (!cpu_has_callbacks_ready_to_invoke(rdp)) {
		trace_rcu_batch_start(rsp->name, rdp->qlen_lazy, rdp->qlen, 0);
		trace_rcu_batch_end(rsp->name, 0, !!ACCESS_ONCE(rdp->nxtlist),
				    need_resched(), is_idle_task(current),
				    rcu_is_callbacks_kthread());
		return;
	}

	/*
                                                             
                                                  
  */
	local_irq_save(flags);
	WARN_ON_ONCE(cpu_is_offline(smp_processor_id()));
	bl = rdp->blimit;
	trace_rcu_batch_start(rsp->name, rdp->qlen_lazy, rdp->qlen, bl);
	list = rdp->nxtlist;
	rdp->nxtlist = *rdp->nxttail[RCU_DONE_TAIL];
	*rdp->nxttail[RCU_DONE_TAIL] = NULL;
	tail = rdp->nxttail[RCU_DONE_TAIL];
	for (i = RCU_NEXT_SIZE - 1; i >= 0; i--)
		if (rdp->nxttail[i] == rdp->nxttail[RCU_DONE_TAIL])
			rdp->nxttail[i] = &rdp->nxtlist;
	local_irq_restore(flags);

	/*                   */
	count = count_lazy = 0;
	while (list) {
		next = list->next;
		prefetch(next);
		debug_rcu_head_unqueue(list);
		if (__rcu_reclaim(rsp->name, list))
			count_lazy++;
		list = next;
		/*                                                         */
		if (++count >= bl &&
		    (need_resched() ||
		     (!is_idle_task(current) && !rcu_is_callbacks_kthread())))
			break;
	}

	local_irq_save(flags);
	trace_rcu_batch_end(rsp->name, count, !!list, need_resched(),
			    is_idle_task(current),
			    rcu_is_callbacks_kthread());

	/*                                                    */
	if (list != NULL) {
		*tail = rdp->nxtlist;
		rdp->nxtlist = list;
		for (i = 0; i < RCU_NEXT_SIZE; i++)
			if (&rdp->nxtlist == rdp->nxttail[i])
				rdp->nxttail[i] = tail;
			else
				break;
	}
	smp_mb(); /*                                                  */
	rdp->qlen_lazy -= count_lazy;
	ACCESS_ONCE(rdp->qlen) -= count;
	rdp->n_cbs_invoked += count;

	/*                                                          */
	if (rdp->blimit == LONG_MAX && rdp->qlen <= qlowmark)
		rdp->blimit = blimit;

	/*                                                                 */
	if (rdp->qlen == 0 && rdp->qlen_last_fqs_check != 0) {
		rdp->qlen_last_fqs_check = 0;
		rdp->n_force_qs_snap = rsp->n_force_qs;
	} else if (rdp->qlen < rdp->qlen_last_fqs_check - qhimark)
		rdp->qlen_last_fqs_check = rdp->qlen;
	WARN_ON_ONCE((rdp->nxtlist == NULL) != (rdp->qlen == 0));

	local_irq_restore(flags);

	/*                                                                 */
	if (cpu_has_callbacks_ready_to_invoke(rdp))
		invoke_rcu_core();
}

/*
                                                                      
                                                                      
                                     
  
                                                                     
                                                                       
                                                              
 */
void rcu_check_callbacks(int cpu, int user)
{
	trace_rcu_utilization("Start scheduler-tick");
	increment_cpu_stall_ticks();
	if (user || rcu_is_cpu_rrupt_from_idle()) {

		/*
                                                      
                                                     
                                                   
                                   
    
                                                    
                                                            
                                                         
                                                        
   */

		rcu_sched_qs(cpu);
		rcu_bh_qs(cpu);

	} else if (!in_softirq()) {

		/*
                                                         
                                                       
                                                         
                                  
   */

		rcu_bh_qs(cpu);
	}
	rcu_preempt_check_callbacks(cpu);
	if (rcu_pending(cpu))
		invoke_rcu_core();
	trace_rcu_utilization("End scheduler-tick");
}

/*
                                                                           
                                                                            
                                                                       
  
                                                              
 */
static void force_qs_rnp(struct rcu_state *rsp, int (*f)(struct rcu_data *))
{
	unsigned long bit;
	int cpu;
	unsigned long flags;
	unsigned long mask;
	struct rcu_node *rnp;

	rcu_for_each_leaf_node(rsp, rnp) {
		cond_resched();
		mask = 0;
		raw_spin_lock_irqsave(&rnp->lock, flags);
		if (!rcu_gp_in_progress(rsp)) {
			raw_spin_unlock_irqrestore(&rnp->lock, flags);
			return;
		}
		if (rnp->qsmask == 0) {
			rcu_initiate_boost(rnp, flags); /*                    */
			continue;
		}
		cpu = rnp->grplo;
		bit = 1;
		for (; cpu <= rnp->grphi; cpu++, bit <<= 1) {
			if ((rnp->qsmask & bit) != 0 &&
			    f(per_cpu_ptr(rsp->rda, cpu)))
				mask |= bit;
		}
		if (mask != 0) {

			/*                                         */
			rcu_report_qs_rnp(mask, rsp, rnp, flags);
			continue;
		}
		raw_spin_unlock_irqrestore(&rnp->lock, flags);
	}
	rnp = rcu_get_root(rsp);
	if (rnp->qsmask == 0) {
		raw_spin_lock_irqsave(&rnp->lock, flags);
		rcu_initiate_boost(rnp, flags); /*                     */
	}
}

/*
                                                                  
                                 
 */
static void force_quiescent_state(struct rcu_state *rsp)
{
	unsigned long flags;
	bool ret;
	struct rcu_node *rnp;
	struct rcu_node *rnp_old = NULL;

	/*                                                       */
	rnp = per_cpu_ptr(rsp->rda, raw_smp_processor_id())->mynode;
	for (; rnp != NULL; rnp = rnp->parent) {
		ret = (ACCESS_ONCE(rsp->gp_flags) & RCU_GP_FLAG_FQS) ||
		      !raw_spin_trylock(&rnp->fqslock);
		if (rnp_old != NULL)
			raw_spin_unlock(&rnp_old->fqslock);
		if (ret) {
			rsp->n_force_qs_lh++;
			return;
		}
		rnp_old = rnp;
	}
	/*                                            */

	/*                                                      */
	raw_spin_lock_irqsave(&rnp_old->lock, flags);
	raw_spin_unlock(&rnp_old->fqslock);
	if (ACCESS_ONCE(rsp->gp_flags) & RCU_GP_FLAG_FQS) {
		rsp->n_force_qs_lh++;
		raw_spin_unlock_irqrestore(&rnp_old->lock, flags);
		return;  /*                        */
	}
	rsp->gp_flags |= RCU_GP_FLAG_FQS;
	raw_spin_unlock_irqrestore(&rnp_old->lock, flags);
	wake_up(&rsp->gp_wq);  /*                                           */
}

/*
                                                                     
                                                                    
                        
 */
static void
__rcu_process_callbacks(struct rcu_state *rsp)
{
	unsigned long flags;
	struct rcu_data *rdp = __this_cpu_ptr(rsp->rda);

	WARN_ON_ONCE(rdp->beenonline == 0);

	/*                                                              */
	rcu_process_gp_end(rsp, rdp);

	/*                                                        */
	rcu_check_quiescent_state(rsp, rdp);

	/*                                                       */
	local_irq_save(flags);
	if (cpu_needs_another_gp(rsp, rdp)) {
		raw_spin_lock(&rcu_get_root(rsp)->lock); /*                */
		rcu_start_gp(rsp);
		raw_spin_unlock_irqrestore(&rcu_get_root(rsp)->lock, flags);
	} else {
		local_irq_restore(flags);
	}

	/*                                            */
	if (cpu_has_callbacks_ready_to_invoke(rdp))
		invoke_rcu_callbacks(rsp, rdp);
}

/*
                                              
 */
static void rcu_process_callbacks(struct softirq_action *unused)
{
	struct rcu_state *rsp;

	if (cpu_is_offline(smp_processor_id()))
		return;
	trace_rcu_utilization("Start RCU core");
	for_each_rcu_flavor(rsp)
		__rcu_process_callbacks(rsp);
	trace_rcu_utilization("End RCU core");
}

/*
                                                                  
                                                                 
                                                                      
                                                               
                                                           
 */
static void invoke_rcu_callbacks(struct rcu_state *rsp, struct rcu_data *rdp)
{
	if (unlikely(!ACCESS_ONCE(rcu_scheduler_fully_active)))
		return;
	if (likely(!rsp->boost)) {
		rcu_do_batch(rsp, rdp);
		return;
	}
	invoke_rcu_callbacks_kthread();
}

static void invoke_rcu_core(void)
{
	if (cpu_online(smp_processor_id()))
		raise_softirq(RCU_SOFTIRQ);
}

/*
                                                                      
 */
static void __call_rcu_core(struct rcu_state *rsp, struct rcu_data *rdp,
			    struct rcu_head *head, unsigned long flags)
{
	/*
                                                              
                                                             
  */
	if (rcu_is_cpu_idle() && cpu_online(smp_processor_id()))
		invoke_rcu_core();

	/*                                                                    */
	if (irqs_disabled_flags(flags) || cpu_is_offline(smp_processor_id()))
		return;

	/*
                                                                     
                                                                
                                                               
                                                                   
                                                           
  */
	if (unlikely(rdp->qlen > rdp->qlen_last_fqs_check + qhimark)) {

		/*                                           */
		rcu_process_gp_end(rsp, rdp);
		check_for_new_grace_period(rsp, rdp);

		/*                                                      */
		if (!rcu_gp_in_progress(rsp)) {
			struct rcu_node *rnp_root = rcu_get_root(rsp);

			raw_spin_lock(&rnp_root->lock);
			rcu_start_gp(rsp);
			raw_spin_unlock(&rnp_root->lock);
		} else {
			/*                               */
			rdp->blimit = LONG_MAX;
			if (rsp->n_force_qs == rdp->n_force_qs_snap &&
			    *rdp->nxttail[RCU_DONE_TAIL] != head)
				force_quiescent_state(rsp);
			rdp->n_force_qs_snap = rsp->n_force_qs;
			rdp->qlen_last_fqs_check = rdp->qlen;
		}
	}
}

/*
                                                                     
                                                                      
                                                                          
                                
 */
static void
__call_rcu(struct rcu_head *head, void (*func)(struct rcu_head *rcu),
	   struct rcu_state *rsp, int cpu, bool lazy)
{
	unsigned long flags;
	struct rcu_data *rdp;

	WARN_ON_ONCE((unsigned long)head & 0x3); /*                      */
	debug_rcu_head_queue(head);
	head->func = func;
	head->next = NULL;

	/*
                                                               
                                                            
                                                                 
                                   
  */
	local_irq_save(flags);
	rdp = this_cpu_ptr(rsp->rda);

	/*                               */
	if (unlikely(rdp->nxttail[RCU_NEXT_TAIL] == NULL) || cpu != -1) {
		int offline;

		if (cpu != -1)
			rdp = per_cpu_ptr(rsp->rda, cpu);
		offline = !__call_rcu_nocb(rdp, head, lazy);
		WARN_ON_ONCE(offline);
		/*                                                           */
		local_irq_restore(flags);
		return;
	}
	ACCESS_ONCE(rdp->qlen)++;
	if (lazy)
		rdp->qlen_lazy++;
	else
		rcu_idle_count_callbacks_posted();
	smp_mb();  /*                                                 */
	*rdp->nxttail[RCU_NEXT_TAIL] = head;
	rdp->nxttail[RCU_NEXT_TAIL] = &head->next;

	if (__is_kfree_rcu_offset((unsigned long)func))
		trace_rcu_kfree_callback(rsp->name, head, (unsigned long)func,
					 rdp->qlen_lazy, rdp->qlen);
	else
		trace_rcu_callback(rsp->name, head, rdp->qlen_lazy, rdp->qlen);

	/*                                             */
	__call_rcu_core(rsp, rdp, head, flags);
	local_irq_restore(flags);
}

/*
                                                                   
 */
void call_rcu_sched(struct rcu_head *head, void (*func)(struct rcu_head *rcu))
{
	__call_rcu(head, func, &rcu_sched_state, -1, 0);
}
EXPORT_SYMBOL_GPL(call_rcu_sched);

/*
                                                                     
 */
void call_rcu_bh(struct rcu_head *head, void (*func)(struct rcu_head *rcu))
{
	__call_rcu(head, func, &rcu_bh_state, -1, 0);
}
EXPORT_SYMBOL_GPL(call_rcu_bh);

/*
                                                                       
                                                                      
                                                                     
                                                                      
                                                                        
                                                                    
                                               
 */
static inline int rcu_blocking_is_gp(void)
{
	int ret;

	might_sleep();  /*                                           */
	preempt_disable();
	ret = num_online_cpus() <= 1;
	preempt_enable();
	return ret;
}

/* 
                                                                        
  
                                                                     
                                                                         
                                                                          
                                                               
                                                                            
                                                         
                         
  
                                                                        
                                                                      
                                                                        
                                                                     
                                                                     
  
                                                                       
                                                                       
                                                                          
                                                                       
                                                                          
                                                                        
                                                                           
                                                                         
                                                                           
                                                                          
                                    
  
                                                                    
                                                                   
                                                                 
                                                                       
                                                   
  
                                                                   
                                                                 
                                                                
                                                              
                                                            
 */
void synchronize_sched(void)
{
	rcu_lockdep_assert(!lock_is_held(&rcu_bh_lock_map) &&
			   !lock_is_held(&rcu_lock_map) &&
			   !lock_is_held(&rcu_sched_lock_map),
			   "Illegal synchronize_sched() in RCU-sched read-side critical section");
	if (rcu_blocking_is_gp())
		return;
	if (rcu_expedited)
		synchronize_sched_expedited();
	else
		wait_rcu_gp(call_rcu_sched);
}
EXPORT_SYMBOL_GPL(synchronize_sched);

/* 
                                                                      
  
                                                                        
                                                                          
                                                                      
                                                                         
                     
  
                                                                           
                                 
 */
void synchronize_rcu_bh(void)
{
	rcu_lockdep_assert(!lock_is_held(&rcu_bh_lock_map) &&
			   !lock_is_held(&rcu_lock_map) &&
			   !lock_is_held(&rcu_sched_lock_map),
			   "Illegal synchronize_rcu_bh() in RCU-bh read-side critical section");
	if (rcu_blocking_is_gp())
		return;
	if (rcu_expedited)
		synchronize_rcu_bh_expedited();
	else
		wait_rcu_gp(call_rcu_bh);
}
EXPORT_SYMBOL_GPL(synchronize_rcu_bh);

static int synchronize_sched_expedited_cpu_stop(void *data)
{
	/*
                                                            
                                                           
                         
   
                                                          
                                                           
                                                         
                                                        
                                                     
  */
	smp_mb(); /*                          */
	return 0;
}

/* 
                                                                   
  
                                                                       
                                                                    
                                                                         
                                                                         
                                                                   
                                                                     
                               
  
                                                                       
                                                                           
                                                                         
                                             
  
                                                                    
                                                        
                                                              
                                                            
                                                                       
                                                                   
                                                                     
                                                                 
                                                                
                                                                   
  
                                                                  
                                                             
                                                                      
                                                                   
                                                                   
                                                                     
                         
  
                                                                           
 */
void synchronize_sched_expedited(void)
{
	long firstsnap, s, snap;
	int trycount = 0;
	struct rcu_state *rsp = &rcu_sched_state;

	/*
                                                                     
                                                                    
                                                                   
                                                                
                                                                    
                                          
  */
	if (ULONG_CMP_GE((ulong)atomic_long_read(&rsp->expedited_start),
			 (ulong)atomic_long_read(&rsp->expedited_done) +
			 ULONG_MAX / 8)) {
		synchronize_sched();
		atomic_long_inc(&rsp->expedited_wrap);
		return;
	}

	/*
                                                           
                        
  */
	snap = atomic_long_inc_return(&rsp->expedited_start);
	firstsnap = snap;
	get_online_cpus();
	WARN_ON_ONCE(cpu_is_offline(raw_smp_processor_id()));

	/*
                                                            
                               
  */
	while (try_stop_cpus(cpu_online_mask,
			     synchronize_sched_expedited_cpu_stop,
			     NULL) == -EAGAIN) {
		put_online_cpus();
		atomic_long_inc(&rsp->expedited_tryfail);

		/*                                                   */
		s = atomic_long_read(&rsp->expedited_done);
		if (ULONG_CMP_GE((ulong)s, (ulong)firstsnap)) {
			/*                                         */
			smp_mb__before_atomic_inc(); /*     */
			atomic_long_inc(&rsp->expedited_workdone1);
			return;
		}

		/*                                                        */
		if (trycount++ < 10) {
			udelay(trycount * num_online_cpus());
		} else {
			wait_rcu_gp(call_rcu_sched);
			atomic_long_inc(&rsp->expedited_normal);
			return;
		}

		/*                                                     */
		s = atomic_long_read(&rsp->expedited_done);
		if (ULONG_CMP_GE((ulong)s, (ulong)firstsnap)) {
			/*                                         */
			smp_mb__before_atomic_inc(); /*     */
			atomic_long_inc(&rsp->expedited_workdone2);
			return;
		}

		/*
                                                         
                                                        
                                                            
                                                         
                         
   */
		get_online_cpus();
		snap = atomic_long_read(&rsp->expedited_start);
		smp_mb(); /*                                        */
	}
	atomic_long_inc(&rsp->expedited_stoppedcpus);

	/*
                                                                
                                                              
                                                              
                                         
  */
	do {
		atomic_long_inc(&rsp->expedited_done_tries);
		s = atomic_long_read(&rsp->expedited_done);
		if (ULONG_CMP_GE((ulong)s, (ulong)snap)) {
			/*                                         */
			smp_mb__before_atomic_inc(); /*     */
			atomic_long_inc(&rsp->expedited_done_lost);
			break;
		}
	} while (atomic_long_cmpxchg(&rsp->expedited_done, s, snap) != s);
	atomic_long_inc(&rsp->expedited_done_exit);

	put_online_cpus();
}
EXPORT_SYMBOL_GPL(synchronize_sched_expedited);

/*
                                                                     
                                                                        
                                                                    
                                                                     
                                                                      
 */
static int __rcu_pending(struct rcu_state *rsp, struct rcu_data *rdp)
{
	struct rcu_node *rnp = rdp->mynode;

	rdp->n_rcu_pending++;

	/*                                   */
	check_cpu_stall(rsp, rdp);

	/*                                                              */
	if (rcu_scheduler_fully_active &&
	    rdp->qs_pending && !rdp->passed_quiesce) {
		rdp->n_rp_qs_pending++;
	} else if (rdp->qs_pending && rdp->passed_quiesce) {
		rdp->n_rp_report_qs++;
		return 1;
	}

	/*                                               */
	if (cpu_has_callbacks_ready_to_invoke(rdp)) {
		rdp->n_rp_cb_ready++;
		return 1;
	}

	/*                                                               */
	if (cpu_needs_another_gp(rsp, rdp)) {
		rdp->n_rp_cpu_needs_gp++;
		return 1;
	}

	/*                                          */
	if (ACCESS_ONCE(rnp->completed) != rdp->completed) { /*              */
		rdp->n_rp_gp_completed++;
		return 1;
	}

	/*                                     */
	if (ACCESS_ONCE(rnp->gpnum) != rdp->gpnum) { /*              */
		rdp->n_rp_gp_started++;
		return 1;
	}

	/*               */
	rdp->n_rp_need_nothing++;
	return 0;
}

/*
                                                                     
                                                                       
                                                                     
 */
static int rcu_pending(int cpu)
{
	struct rcu_state *rsp;

	for_each_rcu_flavor(rsp)
		if (__rcu_pending(rsp, per_cpu_ptr(rsp->rda, cpu)))
			return 1;
	return 0;
}

/*
                                                                     
                                                                   
                                                                  
 */
static int rcu_cpu_has_callbacks(int cpu, bool *all_lazy)
{
	bool al = true;
	bool hc = false;
	struct rcu_data *rdp;
	struct rcu_state *rsp;

	for_each_rcu_flavor(rsp) {
		rdp = per_cpu_ptr(rsp->rda, cpu);
		if (rdp->qlen != rdp->qlen_lazy)
			al = false;
		if (rdp->nxtlist)
			hc = true;
	}
	if (all_lazy)
		*all_lazy = al;
	return hc;
}

/*
                                                                       
                                                  
 */
static void _rcu_barrier_trace(struct rcu_state *rsp, char *s,
			       int cpu, unsigned long done)
{
	trace_rcu_barrier(rsp->name, s, cpu,
			  atomic_read(&rsp->barrier_cpu_count), done);
}

/*
                                                                  
                                        
 */
static void rcu_barrier_callback(struct rcu_head *rhp)
{
	struct rcu_data *rdp = container_of(rhp, struct rcu_data, barrier_head);
	struct rcu_state *rsp = rdp->rsp;

	if (atomic_dec_and_test(&rsp->barrier_cpu_count)) {
		_rcu_barrier_trace(rsp, "LastCB", -1, rsp->n_barrier_done);
		complete(&rsp->barrier_completion);
	} else {
		_rcu_barrier_trace(rsp, "CB", -1, rsp->n_barrier_done);
	}
}

/*
                                                                   
 */
static void rcu_barrier_func(void *type)
{
	struct rcu_state *rsp = type;
	struct rcu_data *rdp = __this_cpu_ptr(rsp->rda);

	_rcu_barrier_trace(rsp, "IRQ", -1, rsp->n_barrier_done);
	atomic_inc(&rsp->barrier_cpu_count);
	rsp->call(&rdp->barrier_head, rcu_barrier_callback);
}

/*
                                                                 
                                                   
 */
static void _rcu_barrier(struct rcu_state *rsp)
{
	int cpu;
	struct rcu_data *rdp;
	unsigned long snap = ACCESS_ONCE(rsp->n_barrier_done);
	unsigned long snap_done;

	_rcu_barrier_trace(rsp, "Begin", -1, snap);

	/*                                                            */
	mutex_lock(&rsp->barrier_mutex);

	/*
                                                                    
                                                    
  */
	smp_mb();  /*                          */

	/*
                                                                  
                                                                   
                                                                   
                                                                   
  */
	snap_done = ACCESS_ONCE(rsp->n_barrier_done);
	_rcu_barrier_trace(rsp, "Check", -1, snap_done);
	if (ULONG_CMP_GE(snap_done, ((snap + 1) & ~0x1) + 2)) {
		_rcu_barrier_trace(rsp, "EarlyExit", -1, snap_done);
		smp_mb(); /*                                             */
		mutex_unlock(&rsp->barrier_mutex);
		return;
	}

	/*
                                                            
                                                          
                                                  
  */
	ACCESS_ONCE(rsp->n_barrier_done)++;
	WARN_ON_ONCE((rsp->n_barrier_done & 0x1) != 1);
	_rcu_barrier_trace(rsp, "Inc1", -1, rsp->n_barrier_done);
	smp_mb(); /*                                                        */

	/*
                                                               
                                                                   
                                                                 
                                                       
  */
	init_completion(&rsp->barrier_completion);
	atomic_set(&rsp->barrier_cpu_count, 1);
	get_online_cpus();

	/*
                                                             
                                                               
                                                              
  */
	for_each_possible_cpu(cpu) {
		if (!cpu_online(cpu) && !rcu_is_nocb_cpu(cpu))
			continue;
		rdp = per_cpu_ptr(rsp->rda, cpu);
		if (rcu_is_nocb_cpu(cpu)) {
			_rcu_barrier_trace(rsp, "OnlineNoCB", cpu,
					   rsp->n_barrier_done);
			atomic_inc(&rsp->barrier_cpu_count);
			__call_rcu(&rdp->barrier_head, rcu_barrier_callback,
				   rsp, cpu, 0);
		} else if (ACCESS_ONCE(rdp->qlen)) {
			_rcu_barrier_trace(rsp, "OnlineQ", cpu,
					   rsp->n_barrier_done);
			smp_call_function_single(cpu, rcu_barrier_func, rsp, 1);
		} else {
			_rcu_barrier_trace(rsp, "OnlineNQ", cpu,
					   rsp->n_barrier_done);
		}
	}
	put_online_cpus();

	/*
                                                               
                                                         
  */
	if (atomic_dec_and_test(&rsp->barrier_cpu_count))
		complete(&rsp->barrier_completion);

	/*                                                       */
	smp_mb(); /*                                       */
	ACCESS_ONCE(rsp->n_barrier_done)++;
	WARN_ON_ONCE((rsp->n_barrier_done & 0x1) != 0);
	_rcu_barrier_trace(rsp, "Inc2", -1, rsp->n_barrier_done);
	smp_mb(); /*                                                 */

	/*                                                              */
	wait_for_completion(&rsp->barrier_completion);

	/*                                                         */
	mutex_unlock(&rsp->barrier_mutex);
}

/* 
                                                                              
 */
void rcu_barrier_bh(void)
{
	_rcu_barrier(&rcu_bh_state);
}
EXPORT_SYMBOL_GPL(rcu_barrier_bh);

/* 
                                                                     
 */
void rcu_barrier_sched(void)
{
	_rcu_barrier(&rcu_sched_state);
}
EXPORT_SYMBOL_GPL(rcu_barrier_sched);

/*
                                                           
 */
static void __init
rcu_boot_init_percpu_data(int cpu, struct rcu_state *rsp)
{
	unsigned long flags;
	struct rcu_data *rdp = per_cpu_ptr(rsp->rda, cpu);
	struct rcu_node *rnp = rcu_get_root(rsp);

	/*                                                               */
	raw_spin_lock_irqsave(&rnp->lock, flags);
	rdp->grpmask = 1UL << (cpu - rdp->mynode->grplo);
	init_callback_list(rdp);
	rdp->qlen_lazy = 0;
	ACCESS_ONCE(rdp->qlen) = 0;
	rdp->dynticks = &per_cpu(rcu_dynticks, cpu);
	WARN_ON_ONCE(rdp->dynticks->dynticks_nesting != DYNTICK_TASK_EXIT_IDLE);
	WARN_ON_ONCE(atomic_read(&rdp->dynticks->dynticks) != 1);
	rdp->cpu = cpu;
	rdp->rsp = rsp;
	rcu_boot_init_nocb_percpu_data(rdp);
	raw_spin_unlock_irqrestore(&rnp->lock, flags);
}

/*
                                                                     
                                                                     
                                                                    
                                                                      
 */
static void __cpuinit
rcu_init_percpu_data(int cpu, struct rcu_state *rsp, int preemptible)
{
	unsigned long flags;
	unsigned long mask;
	struct rcu_data *rdp = per_cpu_ptr(rsp->rda, cpu);
	struct rcu_node *rnp = rcu_get_root(rsp);

	/*                            */
	mutex_lock(&rsp->onoff_mutex);

	/*                                                               */
	raw_spin_lock_irqsave(&rnp->lock, flags);
	rdp->beenonline = 1;	 /*                          */
	rdp->preemptible = preemptible;
	rdp->qlen_last_fqs_check = 0;
	rdp->n_force_qs_snap = rsp->n_force_qs;
	rdp->blimit = blimit;
	init_callback_list(rdp);  /*                                  */
	rdp->dynticks->dynticks_nesting = DYNTICK_TASK_EXIT_IDLE;
	atomic_set(&rdp->dynticks->dynticks,
		   (atomic_read(&rdp->dynticks->dynticks) & ~0x1) + 1);
	raw_spin_unlock(&rnp->lock);		/*                       */

	/*                               */
	rnp = rdp->mynode;
	mask = rdp->grpmask;
	do {
		/*                                                          */
		raw_spin_lock(&rnp->lock);	/*                        */
		rnp->qsmaskinit |= mask;
		mask = rnp->grpmask;
		if (rnp == rdp->mynode) {
			/*
                                                     
                                                
                    
    */
			rdp->gpnum = rnp->completed;
			rdp->completed = rnp->completed;
			rdp->passed_quiesce = 0;
			rdp->qs_pending = 0;
			trace_rcu_grace_period(rsp->name, rdp->gpnum, "cpuonl");
		}
		raw_spin_unlock(&rnp->lock); /*                        */
		rnp = rnp->parent;
	} while (rnp != NULL && !(rnp->qsmaskinit & mask));
	local_irq_restore(flags);

	mutex_unlock(&rsp->onoff_mutex);
}

static void __cpuinit rcu_prepare_cpu(int cpu)
{
	struct rcu_state *rsp;

	for_each_rcu_flavor(rsp)
		rcu_init_percpu_data(cpu, rsp,
				     strcmp(rsp->name, "rcu_preempt") == 0);
}

/*
                                                 
 */
static int __cpuinit rcu_cpu_notify(struct notifier_block *self,
				    unsigned long action, void *hcpu)
{
	long cpu = (long)hcpu;
	struct rcu_data *rdp = per_cpu_ptr(rcu_state->rda, cpu);
	struct rcu_node *rnp = rdp->mynode;
	struct rcu_state *rsp;

	trace_rcu_utilization("Start CPU hotplug");
	switch (action) {
	case CPU_UP_PREPARE:
	case CPU_UP_PREPARE_FROZEN:
		rcu_prepare_cpu(cpu);
		rcu_prepare_kthreads(cpu);
		break;
	case CPU_ONLINE:
	case CPU_DOWN_FAILED:
		rcu_boost_kthread_setaffinity(rnp, -1);
		break;
	case CPU_DOWN_PREPARE:
		rcu_boost_kthread_setaffinity(rnp, cpu);
		break;
	case CPU_DYING:
	case CPU_DYING_FROZEN:
		for_each_rcu_flavor(rsp)
			rcu_cleanup_dying_cpu(rsp);
		break;
	case CPU_DEAD:
	case CPU_DEAD_FROZEN:
	case CPU_UP_CANCELED:
	case CPU_UP_CANCELED_FROZEN:
		for_each_rcu_flavor(rsp)
			rcu_cleanup_dead_cpu(cpu, rsp);
		break;
	default:
		break;
	}
	trace_rcu_utilization("End CPU hotplug");
	return NOTIFY_OK;
}

/*
                                                                  
 */
static int __init rcu_spawn_gp_kthread(void)
{
	unsigned long flags;
	struct rcu_node *rnp;
	struct rcu_state *rsp;
	struct task_struct *t;

	for_each_rcu_flavor(rsp) {
		t = kthread_run(rcu_gp_kthread, rsp, rsp->name);
		BUG_ON(IS_ERR(t));
		rnp = rcu_get_root(rsp);
		raw_spin_lock_irqsave(&rnp->lock, flags);
		rsp->gp_kthread = t;
		raw_spin_unlock_irqrestore(&rnp->lock, flags);
		rcu_spawn_nocb_kthreads(rsp);
	}
	return 0;
}
early_initcall(rcu_spawn_gp_kthread);

/*
                                                                             
                                                               
                                                                
                                                                   
                                                                   
                                                              
 */
void rcu_scheduler_starting(void)
{
	WARN_ON(num_online_cpus() != 1);
	WARN_ON(nr_context_switches() > 0);
	rcu_scheduler_active = 1;
}

/*
                                                                        
                                                               
 */
#ifdef CONFIG_RCU_FANOUT_EXACT
static void __init rcu_init_levelspread(struct rcu_state *rsp)
{
	int i;

	for (i = rcu_num_lvls - 1; i > 0; i--)
		rsp->levelspread[i] = CONFIG_RCU_FANOUT;
	rsp->levelspread[0] = rcu_fanout_leaf;
}
#else /*                                */
static void __init rcu_init_levelspread(struct rcu_state *rsp)
{
	int ccur;
	int cprv;
	int i;

	cprv = nr_cpu_ids;
	for (i = rcu_num_lvls - 1; i >= 0; i--) {
		ccur = rsp->levelcnt[i];
		rsp->levelspread[i] = (cprv + ccur - 1) / ccur;
		cprv = ccur;
	}
}
#endif /*                                      */

/*
                                                                           
 */
static void __init rcu_init_one(struct rcu_state *rsp,
		struct rcu_data __percpu *rda)
{
	static char *buf[] = { "rcu_node_0",
			       "rcu_node_1",
			       "rcu_node_2",
			       "rcu_node_3" };  /*                    */
	static char *fqs[] = { "rcu_node_fqs_0",
			       "rcu_node_fqs_1",
			       "rcu_node_fqs_2",
			       "rcu_node_fqs_3" };  /*                    */
	int cpustride = 1;
	int i;
	int j;
	struct rcu_node *rnp;

	BUILD_BUG_ON(MAX_RCU_LVLS > ARRAY_SIZE(buf));  /*                 */

	/*                                                         */
	if (rcu_num_lvls > RCU_NUM_LVLS)
		panic("rcu_init_one: rcu_num_lvls overflow");

	/*                                       */

	for (i = 0; i < rcu_num_lvls; i++)
		rsp->levelcnt[i] = num_rcu_lvl[i];
	for (i = 1; i < rcu_num_lvls; i++)
		rsp->level[i] = rsp->level[i - 1] + rsp->levelcnt[i - 1];
	rcu_init_levelspread(rsp);

	/*                                                               */

	for (i = rcu_num_lvls - 1; i >= 0; i--) {
		cpustride *= rsp->levelspread[i];
		rnp = rsp->level[i];
		for (j = 0; j < rsp->levelcnt[i]; j++, rnp++) {
			raw_spin_lock_init(&rnp->lock);
			lockdep_set_class_and_name(&rnp->lock,
						   &rcu_node_class[i], buf[i]);
			raw_spin_lock_init(&rnp->fqslock);
			lockdep_set_class_and_name(&rnp->fqslock,
						   &rcu_fqs_class[i], fqs[i]);
			rnp->gpnum = rsp->gpnum;
			rnp->completed = rsp->completed;
			rnp->qsmask = 0;
			rnp->qsmaskinit = 0;
			rnp->grplo = j * cpustride;
			rnp->grphi = (j + 1) * cpustride - 1;
			if (rnp->grphi >= NR_CPUS)
				rnp->grphi = NR_CPUS - 1;
			if (i == 0) {
				rnp->grpnum = 0;
				rnp->grpmask = 0;
				rnp->parent = NULL;
			} else {
				rnp->grpnum = j % rsp->levelspread[i - 1];
				rnp->grpmask = 1UL << rnp->grpnum;
				rnp->parent = rsp->level[i - 1] +
					      j / rsp->levelspread[i - 1];
			}
			rnp->level = i;
			INIT_LIST_HEAD(&rnp->blkd_tasks);
			rcu_init_one_nocb(rnp);
		}
	}

	rsp->rda = rda;
	init_waitqueue_head(&rsp->gp_wq);
	init_irq_work(&rsp->wakeup_work, rsp_wakeup);
	rnp = rsp->level[rcu_num_lvls - 1];
	for_each_possible_cpu(i) {
		while (i > rnp->grphi)
			rnp++;
		per_cpu_ptr(rsp->rda, i)->mynode = rnp;
		rcu_boot_init_percpu_data(i, rsp);
	}
	list_add(&rsp->flavors, &rcu_struct_flavors);
}

/*
                                                                          
                                                                        
                                               
 */
static void __init rcu_init_geometry(void)
{
	int i;
	int j;
	int n = nr_cpu_ids;
	int rcu_capacity[MAX_RCU_LVLS + 1];

	/*                                                      */
	if (rcu_fanout_leaf == CONFIG_RCU_FANOUT_LEAF &&
	    nr_cpu_ids == NR_CPUS)
		return;

	/*
                                                                
                                                                   
                                  
  */
	rcu_capacity[0] = 1;
	rcu_capacity[1] = rcu_fanout_leaf;
	for (i = 2; i <= MAX_RCU_LVLS; i++)
		rcu_capacity[i] = rcu_capacity[i - 1] * CONFIG_RCU_FANOUT;

	/*
                                                             
                                                                   
                                                             
                                                                      
                                                                 
                                                     
  */
	if (rcu_fanout_leaf < CONFIG_RCU_FANOUT_LEAF ||
	    rcu_fanout_leaf > sizeof(unsigned long) * 8 ||
	    n > rcu_capacity[MAX_RCU_LVLS]) {
		WARN_ON(1);
		return;
	}

	/*                                                              */
	for (i = 1; i <= MAX_RCU_LVLS; i++)
		if (n <= rcu_capacity[i]) {
			for (j = 0; j <= i; j++)
				num_rcu_lvl[j] =
					DIV_ROUND_UP(n, rcu_capacity[i - j]);
			rcu_num_lvls = i;
			for (j = i + 1; j <= MAX_RCU_LVLS; j++)
				num_rcu_lvl[j] = 0;
			break;
		}

	/*                                                    */
	rcu_num_nodes = 0;
	for (i = 0; i <= MAX_RCU_LVLS; i++)
		rcu_num_nodes += num_rcu_lvl[i];
	rcu_num_nodes -= n;
}

void __init rcu_init(void)
{
	int cpu;

	rcu_bootup_announce();
	rcu_init_geometry();
	rcu_init_one(&rcu_sched_state, &rcu_sched_data);
	rcu_init_one(&rcu_bh_state, &rcu_bh_data);
	__rcu_init_preempt();
	open_softirq(RCU_SOFTIRQ, rcu_process_callbacks);

	/*
                                                             
                                                          
                                     
  */
	cpu_notifier(rcu_cpu_notify, 0);
	for_each_online_cpu(cpu)
		rcu_cpu_notify(NULL, CPU_UP_PREPARE, (void *)(long)cpu);
}

#include "rcutree_plugin.h"
