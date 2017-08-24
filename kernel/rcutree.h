/*
 * Read-Copy Update mechanism for mutual exclusion (tree-based version)
 * Internal non-public definitions.
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
 * Author: Ingo Molnar <mingo@elte.hu>
 *	   Paul E. McKenney <paulmck@linux.vnet.ibm.com>
 */

#include <linux/cache.h>
#include <linux/spinlock.h>
#include <linux/threads.h>
#include <linux/cpumask.h>
#include <linux/seqlock.h>
#include <linux/irq_work.h>

/*
                                                                     
                          
                                                                         
                                                                   
                                    
 */
#define MAX_RCU_LVLS 4
#define RCU_FANOUT_1	      (CONFIG_RCU_FANOUT_LEAF)
#define RCU_FANOUT_2	      (RCU_FANOUT_1 * CONFIG_RCU_FANOUT)
#define RCU_FANOUT_3	      (RCU_FANOUT_2 * CONFIG_RCU_FANOUT)
#define RCU_FANOUT_4	      (RCU_FANOUT_3 * CONFIG_RCU_FANOUT)

#if NR_CPUS <= RCU_FANOUT_1
#  define RCU_NUM_LVLS	      1
#  define NUM_RCU_LVL_0	      1
#  define NUM_RCU_LVL_1	      (NR_CPUS)
#  define NUM_RCU_LVL_2	      0
#  define NUM_RCU_LVL_3	      0
#  define NUM_RCU_LVL_4	      0
#elif NR_CPUS <= RCU_FANOUT_2
#  define RCU_NUM_LVLS	      2
#  define NUM_RCU_LVL_0	      1
#  define NUM_RCU_LVL_1	      DIV_ROUND_UP(NR_CPUS, RCU_FANOUT_1)
#  define NUM_RCU_LVL_2	      (NR_CPUS)
#  define NUM_RCU_LVL_3	      0
#  define NUM_RCU_LVL_4	      0
#elif NR_CPUS <= RCU_FANOUT_3
#  define RCU_NUM_LVLS	      3
#  define NUM_RCU_LVL_0	      1
#  define NUM_RCU_LVL_1	      DIV_ROUND_UP(NR_CPUS, RCU_FANOUT_2)
#  define NUM_RCU_LVL_2	      DIV_ROUND_UP(NR_CPUS, RCU_FANOUT_1)
#  define NUM_RCU_LVL_3	      (NR_CPUS)
#  define NUM_RCU_LVL_4	      0
#elif NR_CPUS <= RCU_FANOUT_4
#  define RCU_NUM_LVLS	      4
#  define NUM_RCU_LVL_0	      1
#  define NUM_RCU_LVL_1	      DIV_ROUND_UP(NR_CPUS, RCU_FANOUT_3)
#  define NUM_RCU_LVL_2	      DIV_ROUND_UP(NR_CPUS, RCU_FANOUT_2)
#  define NUM_RCU_LVL_3	      DIV_ROUND_UP(NR_CPUS, RCU_FANOUT_1)
#  define NUM_RCU_LVL_4	      (NR_CPUS)
#else
# error "CONFIG_RCU_FANOUT insufficient for NR_CPUS"
#endif /*                               */

#define RCU_SUM (NUM_RCU_LVL_0 + NUM_RCU_LVL_1 + NUM_RCU_LVL_2 + NUM_RCU_LVL_3 + NUM_RCU_LVL_4)
#define NUM_RCU_NODES (RCU_SUM - NR_CPUS)

extern int rcu_num_lvls;
extern int rcu_num_nodes;

/*
                          
 */
struct rcu_dynticks {
	long long dynticks_nesting; /*                                  */
				    /*                                     */
	int dynticks_nmi_nesting;   /*                          */
	atomic_t dynticks;	    /*                                */
#ifdef CONFIG_RCU_FAST_NO_HZ
	bool all_lazy;		    /*                         */
	unsigned long nonlazy_posted;
				    /*                                     */
	unsigned long nonlazy_posted_snap;
				    /*                                      */
	unsigned long last_accelerate;
				    /*                                  */
	int tick_nohz_enabled_snap; /*                                   */
#endif /*                              */
};

/*                                   */
#define RCU_KTHREAD_STOPPED  0
#define RCU_KTHREAD_RUNNING  1
#define RCU_KTHREAD_WAITING  2
#define RCU_KTHREAD_OFFCPU   3
#define RCU_KTHREAD_YIELDING 4
#define RCU_KTHREAD_MAX      4

/*
                                                                       
 */
struct rcu_node {
	raw_spinlock_t lock;	/*                                    */
				/*                                         */
	unsigned long gpnum;	/*                                     */
				/*                                      */
				/*                                    */
	unsigned long completed; /*                                  */
				/*                                      */
				/*                                    */
	unsigned long qsmask;	/*                                       */
				/*                                            */
				/*                                            */
				/*                                         */
				/*                                      */
				/*             */
	unsigned long expmask;	/*                               */
				/*                                           */
				/*                                    */
				/*                                        */
	unsigned long qsmaskinit;
				/*                                            */
	unsigned long grpmask;	/*                                 */
				/*                                         */
	int	grplo;		/*                                    */
	int	grphi;		/*                                     */
	u8	grpnum;		/*                                     */
	u8	level;		/*                     */
	struct rcu_node *parent;
	struct list_head blkd_tasks;
				/*                                         */
				/*                                         */
				/*                                         */
	struct list_head *gp_tasks;
				/*                                        */
				/*                                         */
				/*                   */
	struct list_head *exp_tasks;
				/*                                        */
				/*                                          */
				/*                                      */
				/*                                        */
				/*                                          */
#ifdef CONFIG_RCU_BOOST
	struct list_head *boost_tasks;
				/*                                        */
				/*                                           */
				/*                                       */
				/*                                    */
				/*                                         */
				/*                                         */
				/*                             */
	unsigned long boost_time;
				/*                                   */
	struct task_struct *boost_kthread_task;
				/*                                     */
				/*                                        */
	unsigned int boost_kthread_status;
				/*                                          */
	unsigned long n_tasks_boosted;
				/*                                */
	unsigned long n_exp_boosts;
				/*                                           */
	unsigned long n_normal_boosts;
				/*                                        */
	unsigned long n_balk_blkd_tasks;
				/*                                     */
	unsigned long n_balk_exp_gp_tasks;
				/*                                        */
	unsigned long n_balk_boost_tasks;
				/*                                     */
	unsigned long n_balk_notblocked;
				/*                                            */
	unsigned long n_balk_notyet;
				/*                                 */
	unsigned long n_balk_nos;
				/*                                         */
				/*                                          */
#endif /*                         */
#ifdef CONFIG_RCU_NOCB_CPU
	wait_queue_head_t nocb_gp_wq[2];
				/*                                          */
#endif /*                            */
	int need_future_gp[2];
				/*                                       */
	raw_spinlock_t fqslock ____cacheline_internodealigned_in_smp;
} ____cacheline_internodealigned_in_smp;

/*
                                                                  
                                 
 */
#define rcu_for_each_node_breadth_first(rsp, rnp) \
	for ((rnp) = &(rsp)->node[0]; \
	     (rnp) < &(rsp)->node[rcu_num_nodes]; (rnp)++)

/*
                                                                      
                                                                    
                                                                       
 */
#define rcu_for_each_nonleaf_node_breadth_first(rsp, rnp) \
	for ((rnp) = &(rsp)->node[0]; \
	     (rnp) < (rsp)->level[rcu_num_lvls - 1]; (rnp)++)

/*
                                                                        
                                                                       
                                                                         
                                                             
 */
#define rcu_for_each_leaf_node(rsp, rnp) \
	for ((rnp) = (rsp)->level[rcu_num_lvls - 1]; \
	     (rnp) < &(rsp)->node[rcu_num_nodes]; (rnp)++)

/*                                                    */
#define RCU_DONE_TAIL		0	/*                     */
#define RCU_WAIT_TAIL		1	/*                           */
#define RCU_NEXT_READY_TAIL	2	/*                     */
#define RCU_NEXT_TAIL		3
#define RCU_NEXT_SIZE		4

/*                                    */
struct rcu_data {
	/*                                                */
	unsigned long	completed;	/*                                */
					/*                             */
	unsigned long	gpnum;		/*                                 */
					/*                              */
	bool		passed_quiesce;	/*                          */
	bool		qs_pending;	/*                              */
	bool		beenonline;	/*                           */
	bool		preemptible;	/*                  */
	struct rcu_node *mynode;	/*                              */
	unsigned long grpmask;		/*                               */
#ifdef CONFIG_RCU_CPU_STALL_INFO
	unsigned long	ticks_this_gp;	/*                                */
					/*                             */
					/*                                  */
					/*                        */
#endif /*                                  */

	/*                   */
	/*
                                                         
                                                           
                                                              
                                                            
                                                               
                               
   
                                       
                                       
                                                         
                                                         
                                                
                                                       
                                                                   
                                                             
                                                         
                                                             
                                                          
                                                       
                                                   
  */
	struct rcu_head *nxtlist;
	struct rcu_head **nxttail[RCU_NEXT_SIZE];
	unsigned long	nxtcompleted[RCU_NEXT_SIZE];
					/*                             */
	long		qlen_lazy;	/*                            */
	long		qlen;		/*                                  */
	long		qlen_last_fqs_check;
					/*                                   */
	unsigned long	n_cbs_invoked;	/*                           */
	unsigned long	n_nocbs_invoked; /*                                  */
	unsigned long   n_cbs_orphaned; /*                               */
	unsigned long   n_cbs_adopted;  /*                                */
	unsigned long	n_force_qs_snap;
					/*                                  */
	long		blimit;		/*                                  */

	/*                        */
	struct rcu_dynticks *dynticks;	/*                                */
	int dynticks_snap;		/*                               */

	/*                                                                  */
	unsigned long dynticks_fqs;	/*                              */
	unsigned long offline_fqs;	/*                              */

	/*                                */
	unsigned long n_rcu_pending;	/*                                 */
	unsigned long n_rp_qs_pending;
	unsigned long n_rp_report_qs;
	unsigned long n_rp_cb_ready;
	unsigned long n_rp_cpu_needs_gp;
	unsigned long n_rp_gp_completed;
	unsigned long n_rp_gp_started;
	unsigned long n_rp_need_nothing;

	/*                                      */
	struct rcu_head barrier_head;
#ifdef CONFIG_RCU_FAST_NO_HZ
	struct rcu_head oom_head;
#endif /*                              */

	/*                         */
#ifdef CONFIG_RCU_NOCB_CPU
	struct rcu_head *nocb_head;	/*                          */
	struct rcu_head **nocb_tail;
	atomic_long_t nocb_q_count;	/*                           */
	atomic_long_t nocb_q_count_lazy; /*                 */
	int nocb_p_count;		/*                                */
	int nocb_p_count_lazy;		/*                 */
	wait_queue_head_t nocb_wq;	/*                                */
	struct task_struct *nocb_kthread;
#endif /*                            */

	/*                        */
#ifdef CONFIG_RCU_CPU_STALL_INFO
	unsigned int softirq_snap;	/*                               */
#endif /*                                  */

	int cpu;
	struct rcu_state *rsp;
};

/*                                                 */
#define RCU_GP_IDLE		0	/*                              */
#define RCU_GP_INIT		1	/*                                 */
#define RCU_SAVE_DYNTICK	2	/*                             */
#define RCU_FORCE_QS		3	/*                                */
#define RCU_SIGNAL_INIT		RCU_SAVE_DYNTICK

#define RCU_JIFFIES_TILL_FORCE_QS	 3	/*                           */

#define RCU_STALL_RAT_DELAY		2	/*                       */
						/*                       */
						/*                       */
						/*                          */

#define rcu_wait(cond)							\
do {									\
	for (;;) {							\
		set_current_state(TASK_INTERRUPTIBLE);			\
		if (cond)						\
			break;						\
		schedule();						\
	}								\
	__set_current_state(TASK_RUNNING);				\
} while (0)

/*
                                                                 
                                                                       
                                                                          
                                                                             
                                                                           
                                                                       
                                                                        
                                   
 */
struct rcu_state {
	struct rcu_node node[NUM_RCU_NODES];	/*            */
	struct rcu_node *level[RCU_NUM_LVLS];	/*                   */
	u32 levelcnt[MAX_RCU_LVLS + 1];		/*                        */
	u8 levelspread[RCU_NUM_LVLS];		/*                          */
	struct rcu_data __percpu *rda;		/*                            */
	void (*call)(struct rcu_head *head,	/*                    */
		     void (*func)(struct rcu_head *head));

	/*                                                               */

	u8	fqs_state ____cacheline_internodealigned_in_smp;
						/*                 */
	u8	boost;				/*                            */
	unsigned long gpnum;			/*                    */
	unsigned long completed;		/*                         */
	struct task_struct *gp_kthread;		/*                         */
	wait_queue_head_t gp_wq;		/*                      */
	int gp_flags;				/*                       */

	/*                                                */

	raw_spinlock_t orphan_lock ____cacheline_internodealigned_in_smp;
						/*                           */
	struct rcu_head *orphan_nxtlist;	/*                         */
						/*                       */
	struct rcu_head **orphan_nxttail;	/*                */
	struct rcu_head *orphan_donelist;	/*                         */
						/*                       */
	struct rcu_head **orphan_donetail;	/*                */
	long qlen_lazy;				/*                           */
	long qlen;				/*                            */
	/*                                       */

	struct mutex onoff_mutex;		/*                           */

	struct mutex barrier_mutex;		/*                        */
	atomic_t barrier_cpu_count;		/*                    */
	struct completion barrier_completion;	/*                      */
	unsigned long n_barrier_done;		/*                        */
						/*                  */
	/*                                         */

	atomic_long_t expedited_start;		/*                  */
	atomic_long_t expedited_done;		/*              */
	atomic_long_t expedited_wrap;		/*                        */
	atomic_long_t expedited_tryfail;	/*                         */
	atomic_long_t expedited_workdone1;	/*                      */
	atomic_long_t expedited_workdone2;	/*                      */
	atomic_long_t expedited_normal;		/*                        */
	atomic_long_t expedited_stoppedcpus;	/*                         */
	atomic_long_t expedited_done_tries;	/*                          */
	atomic_long_t expedited_done_lost;	/*                          */
	atomic_long_t expedited_done_exit;	/*                            */

	unsigned long jiffies_force_qs;		/*                         */
						/*                           */
	unsigned long n_force_qs;		/*                    */
						/*                           */
	unsigned long n_force_qs_lh;		/*                          */
						/*                           */
	unsigned long n_force_qs_ngp;		/*                         */
						/*                       */
	unsigned long gp_start;			/*                           */
						/*                  */
	unsigned long jiffies_stall;		/*                        */
						/*                  */
	unsigned long gp_max;			/*                        */
						/*           */
	char *name;				/*                    */
	char abbr;				/*                   */
	struct list_head flavors;		/*                      */
	struct irq_work wakeup_work;		/*                   */
};

/*                                                  */
#define RCU_GP_FLAG_INIT 0x1	/*                                   */
#define RCU_GP_FLAG_FQS  0x2	/*                                            */

extern struct list_head rcu_struct_flavors;

/*                                                            */
#define for_each_rcu_flavor(rsp) \
	list_for_each_entry((rsp), &rcu_struct_flavors, flavors)

/*                                                */

#define RCU_OFL_TASKS_NORM_GP	0x1		/*                       */
						/*                         */
#define RCU_OFL_TASKS_EXP_GP	0x2		/*                          */
						/*                         */

/*
                                            
 */
extern struct rcu_state rcu_sched_state;
DECLARE_PER_CPU(struct rcu_data, rcu_sched_data);

extern struct rcu_state rcu_bh_state;
DECLARE_PER_CPU(struct rcu_data, rcu_bh_data);

#ifdef CONFIG_TREE_PREEMPT_RCU
extern struct rcu_state rcu_preempt_state;
DECLARE_PER_CPU(struct rcu_data, rcu_preempt_data);
#endif /*                                */

#ifdef CONFIG_RCU_BOOST
DECLARE_PER_CPU(unsigned int, rcu_cpu_kthread_status);
DECLARE_PER_CPU(int, rcu_cpu_kthread_cpu);
DECLARE_PER_CPU(unsigned int, rcu_cpu_kthread_loops);
DECLARE_PER_CPU(char, rcu_cpu_has_work);
#endif /*                         */

#ifndef RCU_TREE_NONCORE

/*                                           */
static void rcu_bootup_announce(void);
long rcu_batches_completed(void);
static void rcu_preempt_note_context_switch(int cpu);
static int rcu_preempt_blocked_readers_cgp(struct rcu_node *rnp);
#ifdef CONFIG_HOTPLUG_CPU
static void rcu_report_unblock_qs_rnp(struct rcu_node *rnp,
				      unsigned long flags);
#endif /*                           */
static void rcu_print_detail_task_stall(struct rcu_state *rsp);
static int rcu_print_task_stall(struct rcu_node *rnp);
static void rcu_preempt_check_blocked_tasks(struct rcu_node *rnp);
#ifdef CONFIG_HOTPLUG_CPU
static int rcu_preempt_offline_tasks(struct rcu_state *rsp,
				     struct rcu_node *rnp,
				     struct rcu_data *rdp);
#endif /*                           */
static void rcu_preempt_check_callbacks(int cpu);
void call_rcu(struct rcu_head *head, void (*func)(struct rcu_head *rcu));
#if defined(CONFIG_HOTPLUG_CPU) || defined(CONFIG_TREE_PREEMPT_RCU)
static void rcu_report_exp_rnp(struct rcu_state *rsp, struct rcu_node *rnp,
			       bool wake);
#endif /*                                                                     */
static void __init __rcu_init_preempt(void);
static void rcu_initiate_boost(struct rcu_node *rnp, unsigned long flags);
static void rcu_preempt_boost_start_gp(struct rcu_node *rnp);
static void invoke_rcu_callbacks_kthread(void);
static bool rcu_is_callbacks_kthread(void);
#ifdef CONFIG_RCU_BOOST
static void rcu_preempt_do_callbacks(void);
static int __cpuinit rcu_spawn_one_boost_kthread(struct rcu_state *rsp,
						 struct rcu_node *rnp);
#endif /*                         */
static void __cpuinit rcu_prepare_kthreads(int cpu);
static void rcu_cleanup_after_idle(int cpu);
static void rcu_prepare_for_idle(int cpu);
static void rcu_idle_count_callbacks_posted(void);
static void print_cpu_stall_info_begin(void);
static void print_cpu_stall_info(struct rcu_state *rsp, int cpu);
static void print_cpu_stall_info_end(void);
static void zero_cpu_stall_ticks(struct rcu_data *rdp);
static void increment_cpu_stall_ticks(void);
static int rcu_nocb_needs_gp(struct rcu_state *rsp);
static void rcu_nocb_gp_set(struct rcu_node *rnp, int nrq);
static void rcu_nocb_gp_cleanup(struct rcu_state *rsp, struct rcu_node *rnp);
static void rcu_init_one_nocb(struct rcu_node *rnp);
static bool __call_rcu_nocb(struct rcu_data *rdp, struct rcu_head *rhp,
			    bool lazy);
static bool rcu_nocb_adopt_orphan_cbs(struct rcu_state *rsp,
				      struct rcu_data *rdp);
static void rcu_boot_init_nocb_percpu_data(struct rcu_data *rdp);
static void rcu_spawn_nocb_kthreads(struct rcu_state *rsp);
static void rcu_kick_nohz_cpu(int cpu);
static bool init_nocb_callback_list(struct rcu_data *rdp);

#endif /*                          */

#ifdef CONFIG_RCU_TRACE
#ifdef CONFIG_RCU_NOCB_CPU
/*                                   */
static inline void rcu_nocb_q_lengths(struct rcu_data *rdp, long *ql, long *qll)
{
	*ql = atomic_long_read(&rdp->nocb_q_count) + rdp->nocb_p_count;
	*qll = atomic_long_read(&rdp->nocb_q_count_lazy) + rdp->nocb_p_count_lazy;
}
#else /*                            */
static inline void rcu_nocb_q_lengths(struct rcu_data *rdp, long *ql, long *qll)
{
	*ql = 0;
	*qll = 0;
}
#endif /*                                  */
#endif /*                         */
