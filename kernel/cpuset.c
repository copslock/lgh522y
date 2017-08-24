/*
 *  kernel/cpuset.c
 *
 *  Processor and Memory placement constraints for sets of tasks.
 *
 *  Copyright (C) 2003 BULL SA.
 *  Copyright (C) 2004-2007 Silicon Graphics, Inc.
 *  Copyright (C) 2006 Google, Inc
 *
 *  Portions derived from Patrick Mochel's sysfs code.
 *  sysfs is Copyright (c) 2001-3 Patrick Mochel
 *
 *  2003-10-10 Written by Simon Derr.
 *  2003-10-22 Updates by Stephen Hemminger.
 *  2004 May-July Rework by Paul Jackson.
 *  2006 Rework by Paul Menage to use generic cgroups
 *  2008 Rework of the scheduler domains and CPU hotplug handling
 *       by Max Krasnyansky
 *
 *  This file is subject to the terms and conditions of the GNU General Public
 *  License.  See the file COPYING in the main directory of the Linux
 *  distribution for more details.
 */

#include <linux/cpu.h>
#include <linux/cpumask.h>
#include <linux/cpuset.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/kmod.h>
#include <linux/list.h>
#include <linux/mempolicy.h>
#include <linux/mm.h>
#include <linux/memory.h>
#include <linux/export.h>
#include <linux/mount.h>
#include <linux/namei.h>
#include <linux/pagemap.h>
#include <linux/proc_fs.h>
#include <linux/rcupdate.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/security.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/stat.h>
#include <linux/string.h>
#include <linux/time.h>
#include <linux/backing-dev.h>
#include <linux/sort.h>

#include <asm/uaccess.h>
#include <linux/atomic.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/cgroup.h>

/*
                                                           
                                                         
                            
 */
int number_of_cpusets __read_mostly;

/*                                   */
struct cgroup_subsys cpuset_subsys;
struct cpuset;

/*                                        */

struct fmeter {
	int cnt;		/*                          */
	int val;		/*                          */
	time_t time;		/*                                */
	spinlock_t lock;	/*                               */
};

struct cpuset {
	struct cgroup_subsys_state css;

	unsigned long flags;		/*                                */
	cpumask_var_t cpus_allowed;	/*                                 */
	nodemask_t mems_allowed;	/*                               */

	struct fmeter fmeter;		/*                        */

	/*
                                                             
                                                                    
  */
	int attach_in_progress;

	/*                                              */
	int pn;

	/*                         */
	int relax_domain_level;

	struct work_struct hotplug_work;
};

/*                                  */
static inline struct cpuset *cgroup_cs(struct cgroup *cont)
{
	return container_of(cgroup_subsys_state(cont, cpuset_subsys_id),
			    struct cpuset, css);
}

/*                                */
static inline struct cpuset *task_cs(struct task_struct *task)
{
	return container_of(task_subsys_state(task, cpuset_subsys_id),
			    struct cpuset, css);
}

static inline struct cpuset *parent_cs(const struct cpuset *cs)
{
	struct cgroup *pcgrp = cs->css.cgroup->parent;

	if (pcgrp)
		return cgroup_cs(pcgrp);
	return NULL;
}

#ifdef CONFIG_NUMA
static inline bool task_has_mempolicy(struct task_struct *task)
{
	return task->mempolicy;
}
#else
static inline bool task_has_mempolicy(struct task_struct *task)
{
	return false;
}
#endif


/*                                   */
typedef enum {
	CS_ONLINE,
	CS_CPU_EXCLUSIVE,
	CS_MEM_EXCLUSIVE,
	CS_MEM_HARDWALL,
	CS_MEMORY_MIGRATE,
	CS_SCHED_LOAD_BALANCE,
	CS_SPREAD_PAGE,
	CS_SPREAD_SLAB,
} cpuset_flagbits_t;

/*                                 */
static inline bool is_cpuset_online(const struct cpuset *cs)
{
	return test_bit(CS_ONLINE, &cs->flags);
}

static inline int is_cpu_exclusive(const struct cpuset *cs)
{
	return test_bit(CS_CPU_EXCLUSIVE, &cs->flags);
}

static inline int is_mem_exclusive(const struct cpuset *cs)
{
	return test_bit(CS_MEM_EXCLUSIVE, &cs->flags);
}

static inline int is_mem_hardwall(const struct cpuset *cs)
{
	return test_bit(CS_MEM_HARDWALL, &cs->flags);
}

static inline int is_sched_load_balance(const struct cpuset *cs)
{
	return test_bit(CS_SCHED_LOAD_BALANCE, &cs->flags);
}

static inline int is_memory_migrate(const struct cpuset *cs)
{
	return test_bit(CS_MEMORY_MIGRATE, &cs->flags);
}

static inline int is_spread_page(const struct cpuset *cs)
{
	return test_bit(CS_SPREAD_PAGE, &cs->flags);
}

static inline int is_spread_slab(const struct cpuset *cs)
{
	return test_bit(CS_SPREAD_SLAB, &cs->flags);
}

static struct cpuset top_cpuset = {
	.flags = ((1 << CS_ONLINE) | (1 << CS_CPU_EXCLUSIVE) |
		  (1 << CS_MEM_EXCLUSIVE)),
};

/* 
                                                               
                                                       
                                
                                                
  
                                                                          
                        
 */
#define cpuset_for_each_child(child_cs, pos_cgrp, parent_cs)		\
	cgroup_for_each_child((pos_cgrp), (parent_cs)->css.cgroup)	\
		if (is_cpuset_online(((child_cs) = cgroup_cs((pos_cgrp)))))

/* 
                                                                            
                                                          
                                
                                              
  
                                                                         
                                                                    
                                                 
 */
#define cpuset_for_each_descendant_pre(des_cs, pos_cgrp, root_cs)	\
	cgroup_for_each_descendant_pre((pos_cgrp), (root_cs)->css.cgroup) \
		if (is_cpuset_online(((des_cs) = cgroup_cs((pos_cgrp)))))

/*
                                                                         
                                                                       
                                                                         
                                                               
  
                                                                    
                                                                           
                                                                      
                                                                         
                                                                         
                                                                           
                                                                         
                                                                          
                 
  
                                                                     
                                                                       
                                                             
                   
  
                                                                  
                     
  
                                                                        
                                                                        
        
  
                                                                         
                                                                     
                          
  
                                                                  
                                                              
 */

static DEFINE_MUTEX(cpuset_mutex);
static DEFINE_MUTEX(callback_mutex);

/*
                                                  
 */
static struct workqueue_struct *cpuset_propagate_hotplug_wq;

static void cpuset_hotplug_workfn(struct work_struct *work);
static void cpuset_propagate_hotplug_workfn(struct work_struct *work);
static void schedule_cpuset_propagate_hotplug(struct cpuset *cs);

static DECLARE_WORK(cpuset_hotplug_work, cpuset_hotplug_workfn);

/*
                                                                    
                                                               
                                               
 */
static struct dentry *cpuset_mount(struct file_system_type *fs_type,
			 int flags, const char *unused_dev_name, void *data)
{
	struct file_system_type *cgroup_fs = get_fs_type("cgroup");
	struct dentry *ret = ERR_PTR(-ENODEV);
	if (cgroup_fs) {
		char mountopts[] =
			"cpuset,noprefix,"
			"release_agent=/sbin/cpuset_release_agent";
		ret = cgroup_fs->mount(cgroup_fs, flags,
					   unused_dev_name, mountopts);
		put_filesystem(cgroup_fs);
	}
	return ret;
}

static struct file_system_type cpuset_fs_type = {
	.name = "cpuset",
	.mount = cpuset_mount,
};

/*
                                                               
                                                                
                                                                
                                                                  
                                                                   
                                
  
                                                                   
                      
  
                                 
 */

static void guarantee_online_cpus(const struct cpuset *cs,
				  struct cpumask *pmask)
{
	while (cs && !cpumask_intersects(cs->cpus_allowed, cpu_online_mask))
		cs = parent_cs(cs);
	if (cs)
		cpumask_and(pmask, cs->cpus_allowed, cpu_online_mask);
	else
		cpumask_copy(pmask, cpu_online_mask);
	BUG_ON(!cpumask_intersects(pmask, cpu_online_mask));
}

/*
                                                                
                                                                 
                                                                
                                                                   
                                                       
  
                                                                   
                            
  
                                 
 */

static void guarantee_online_mems(const struct cpuset *cs, nodemask_t *pmask)
{
	while (cs && !nodes_intersects(cs->mems_allowed,
					node_states[N_MEMORY]))
		cs = parent_cs(cs);
	if (cs)
		nodes_and(*pmask, cs->mems_allowed,
					node_states[N_MEMORY]);
	else
		*pmask = node_states[N_MEMORY];
	BUG_ON(!nodes_intersects(*pmask, node_states[N_MEMORY]));
}

/*
                                                                     
  
                                               
 */
static void cpuset_update_task_spread_flag(struct cpuset *cs,
					struct task_struct *tsk)
{
	if (is_spread_page(cs))
		tsk->flags |= PF_SPREAD_PAGE;
	else
		tsk->flags &= ~PF_SPREAD_PAGE;
	if (is_spread_slab(cs))
		tsk->flags |= PF_SPREAD_SLAB;
	else
		tsk->flags &= ~PF_SPREAD_SLAB;
}

/*
                                                             
  
                                                                
                                                                  
                                                                   
 */

static int is_cpuset_subset(const struct cpuset *p, const struct cpuset *q)
{
	return	cpumask_subset(p->cpus_allowed, q->cpus_allowed) &&
		nodes_subset(p->mems_allowed, q->mems_allowed) &&
		is_cpu_exclusive(p) <= is_cpu_exclusive(q) &&
		is_mem_exclusive(p) <= is_mem_exclusive(q);
}

/* 
                                               
                                                   
 */
static struct cpuset *alloc_trial_cpuset(const struct cpuset *cs)
{
	struct cpuset *trial;

	trial = kmemdup(cs, sizeof(*cs), GFP_KERNEL);
	if (!trial)
		return NULL;

	if (!alloc_cpumask_var(&trial->cpus_allowed, GFP_KERNEL)) {
		kfree(trial);
		return NULL;
	}
	cpumask_copy(trial->cpus_allowed, cs->cpus_allowed);

	return trial;
}

/* 
                                            
                                       
 */
static void free_trial_cpuset(struct cpuset *trial)
{
	free_cpumask_var(trial->cpus_allowed);
	kfree(trial);
}

/*
                                                                       
                                                    
  
                                                                
                                                             
                                                                   
                     
  
                                                                
                                                                  
                                                    
  
                                                             
                                                                
                                         
  
                                    
 */

static int validate_change(const struct cpuset *cur, const struct cpuset *trial)
{
	struct cgroup *cont;
	struct cpuset *c, *par;
	int ret;

	rcu_read_lock();

	/*                                                  */
	ret = -EBUSY;
	cpuset_for_each_child(c, cont, cur)
		if (!is_cpuset_subset(c, trial))
			goto out;

	/*                                             */
	ret = 0;
	if (cur == &top_cpuset)
		goto out;

	par = parent_cs(cur);

	/*                                          */
	ret = -EACCES;
	if (!is_cpuset_subset(trial, par))
		goto out;

	/*
                                                              
           
  */
	ret = -EINVAL;
	cpuset_for_each_child(c, cont, par) {
		if ((is_cpu_exclusive(trial) || is_cpu_exclusive(c)) &&
		    c != cur &&
		    cpumask_intersects(trial->cpus_allowed, c->cpus_allowed))
			goto out;
		if ((is_mem_exclusive(trial) || is_mem_exclusive(c)) &&
		    c != cur &&
		    nodes_intersects(trial->mems_allowed, c->mems_allowed))
			goto out;
	}

	/*
                                                                 
                                            
  */
	ret = -ENOSPC;
	if ((cgroup_task_count(cur->css.cgroup) || cur->attach_in_progress) &&
	    (cpumask_empty(trial->cpus_allowed) ||
	     nodes_empty(trial->mems_allowed)))
		goto out;

	ret = 0;
out:
	rcu_read_unlock();
	return ret;
}

#ifdef CONFIG_SMP
/*
                                               
                                                       
 */
static int cpusets_overlap(struct cpuset *a, struct cpuset *b)
{
	return cpumask_intersects(a->cpus_allowed, b->cpus_allowed);
}

static void
update_domain_attr(struct sched_domain_attr *dattr, struct cpuset *c)
{
	if (dattr->relax_domain_level < c->relax_domain_level)
		dattr->relax_domain_level = c->relax_domain_level;
	return;
}

static void update_domain_attr_tree(struct sched_domain_attr *dattr,
				    struct cpuset *root_cs)
{
	struct cpuset *cp;
	struct cgroup *pos_cgrp;

	rcu_read_lock();
	cpuset_for_each_descendant_pre(cp, pos_cgrp, root_cs) {
		/*                                                    */
		if (cpumask_empty(cp->cpus_allowed)) {
			pos_cgrp = cgroup_rightmost_descendant(pos_cgrp);
			continue;
		}

		if (is_sched_load_balance(cp))
			update_domain_attr(dattr, cp);
	}
	rcu_read_unlock();
}

/*
                           
  
                                                               
                                                                  
                                 
                                                                   
                                                                        
                                                                      
             
  
                                                                        
                                        
  
                                                                 
                                                                 
                                                                  
                                              
  
                                         
  
                                           
                                                                      
                                                              
                                                          
                                                               
                                                                  
                                                                  
                                                             
                                                                  
                                                            
                                                               
                                                             
                                                               
                                                                      
                                                               
                                                                 
                                                                
                                      
  
                                               
                                                           
                                                               
                                                            
                                                             
                                                                
                                                             
                  
  
                                                      
                                                           
                                                              
                             
 */
static int generate_sched_domains(cpumask_var_t **domains,
			struct sched_domain_attr **attributes)
{
	struct cpuset *cp;	/*         */
	struct cpuset **csa;	/*                          */
	int csn;		/*                                    */
	int i, j, k;		/*                                     */
	cpumask_var_t *doms;	/*                                         */
	struct sched_domain_attr *dattr;  /*                               */
	int ndoms = 0;		/*                                   */
	int nslot;		/*                                       */
	struct cgroup *pos_cgrp;

	doms = NULL;
	dattr = NULL;
	csa = NULL;

	/*                                                                  */
	if (is_sched_load_balance(&top_cpuset)) {
		ndoms = 1;
		doms = alloc_sched_domains(ndoms);
		if (!doms)
			goto done;

		dattr = kmalloc(sizeof(struct sched_domain_attr), GFP_KERNEL);
		if (dattr) {
			*dattr = SD_ATTR_INIT;
			update_domain_attr_tree(dattr, &top_cpuset);
		}
		cpumask_copy(doms[0], top_cpuset.cpus_allowed);

		goto done;
	}

	csa = kmalloc(number_of_cpusets * sizeof(cp), GFP_KERNEL);
	if (!csa)
		goto done;
	csn = 0;

	rcu_read_lock();
	cpuset_for_each_descendant_pre(cp, pos_cgrp, &top_cpuset) {
		/*
                                                             
                                                       
                                                      
                                                       
                                                            
                                    
   */
		if (!cpumask_empty(cp->cpus_allowed) &&
		    !is_sched_load_balance(cp))
			continue;

		if (is_sched_load_balance(cp))
			csa[csn++] = cp;

		/*                    */
		pos_cgrp = cgroup_rightmost_descendant(pos_cgrp);
	}
	rcu_read_unlock();

	for (i = 0; i < csn; i++)
		csa[i]->pn = i;
	ndoms = csn;

restart:
	/*                                                */
	for (i = 0; i < csn; i++) {
		struct cpuset *a = csa[i];
		int apn = a->pn;

		for (j = 0; j < csn; j++) {
			struct cpuset *b = csa[j];
			int bpn = b->pn;

			if (apn != bpn && cpusets_overlap(a, b)) {
				for (k = 0; k < csn; k++) {
					struct cpuset *c = csa[k];

					if (c->pn == bpn)
						c->pn = apn;
				}
				ndoms--;	/*                  */
				goto restart;
			}
		}
	}

	/*
                                           
                                                               
  */
	doms = alloc_sched_domains(ndoms);
	if (!doms)
		goto done;

	/*
                                                                
                                                      
  */
	dattr = kmalloc(ndoms * sizeof(struct sched_domain_attr), GFP_KERNEL);

	for (nslot = 0, i = 0; i < csn; i++) {
		struct cpuset *a = csa[i];
		struct cpumask *dp;
		int apn = a->pn;

		if (apn < 0) {
			/*                           */
			continue;
		}

		dp = doms[nslot];

		if (nslot == ndoms) {
			static int warnings = 10;
			if (warnings) {
				printk(KERN_WARNING
				 "rebuild_sched_domains confused:"
				  " nslot %d, ndoms %d, csn %d, i %d,"
				  " apn %d\n",
				  nslot, ndoms, csn, i, apn);
				warnings--;
			}
			continue;
		}

		cpumask_clear(dp);
		if (dattr)
			*(dattr + nslot) = SD_ATTR_INIT;
		for (j = i; j < csn; j++) {
			struct cpuset *b = csa[j];

			if (apn == b->pn) {
				cpumask_or(dp, dp, b->cpus_allowed);
				if (dattr)
					update_domain_attr_tree(dattr + nslot, b);

				/*                          */
				b->pn = -1;
			}
		}
		nslot++;
	}
	BUG_ON(nslot != ndoms);

done:
	kfree(csa);

	/*
                                                       
                                              
  */
	if (doms == NULL)
		ndoms = 1;

	*domains    = doms;
	*attributes = dattr;
	return ndoms;
}

/*
                             
  
                                                                
                                                                 
                                                                 
                                                           
                                     
  
                                                         
 */
static void rebuild_sched_domains_locked(void)
{
	struct sched_domain_attr *attr;
	cpumask_var_t *doms;
	int ndoms;

	lockdep_assert_held(&cpuset_mutex);
	get_online_cpus();

	/*
                                                              
                                                                
                                                          
  */
	if (!cpumask_equal(top_cpuset.cpus_allowed, cpu_active_mask))
		goto out;

	/*                                 */
	ndoms = generate_sched_domains(&doms, &attr);

	/*                                    */
	partition_sched_domains(ndoms, doms, attr);
out:
	put_online_cpus();
}
#else /*             */
static void rebuild_sched_domains_locked(void)
{
}
#endif /*            */

void rebuild_sched_domains(void)
{
	mutex_lock(&cpuset_mutex);
	rebuild_sched_domains_locked();
	mutex_unlock(&cpuset_mutex);
}

/* 
                                                                       
                     
                                                                              
  
                                                                     
                                                           
                                                                               
                                                         
 */
static int cpuset_test_cpumask(struct task_struct *tsk,
			       struct cgroup_scanner *scan)
{
	return !cpumask_equal(&tsk->cpus_allowed,
			(cgroup_cs(scan->cg))->cpus_allowed);
}

/* 
                                                                              
                     
                                                                 
  
                                                                
                                         
  
                                                                          
                                      
 */
static void cpuset_change_cpumask(struct task_struct *tsk,
				  struct cgroup_scanner *scan)
{
	set_cpus_allowed_ptr(tsk, ((cgroup_cs(scan->cg))->cpus_allowed));
}

/* 
                                                                     
                                                                             
                                                                      
  
                                
  
                                                                        
                                       
  
                                                                             
                    
 */
static void update_tasks_cpumask(struct cpuset *cs, struct ptr_heap *heap)
{
	struct cgroup_scanner scan;

	scan.cg = cs->css.cgroup;
	scan.test_task = cpuset_test_cpumask;
	scan.process_task = cpuset_change_cpumask;
	scan.heap = heap;
	cgroup_scan_tasks(&scan);
}

/* 
                                                                                
                              
                                                     
 */
static int update_cpumask(struct cpuset *cs, struct cpuset *trialcs,
			  const char *buf)
{
	struct ptr_heap heap;
	int retval;
	int is_load_balanced;

	/*                                                                */
	if (cs == &top_cpuset)
		return -EACCES;

	/*
                                                                
                                                                 
                                                                  
                         
  */
	if (!*buf) {
		cpumask_clear(trialcs->cpus_allowed);
	} else {
		retval = cpulist_parse(buf, trialcs->cpus_allowed);
		if (retval < 0)
			return retval;

		if (!cpumask_subset(trialcs->cpus_allowed, cpu_active_mask))
			return -EINVAL;
	}
	retval = validate_change(cs, trialcs);
	if (retval < 0)
		return retval;

	/*                                         */
	if (cpumask_equal(cs->cpus_allowed, trialcs->cpus_allowed))
		return 0;

	retval = heap_init(&heap, PAGE_SIZE, GFP_KERNEL, NULL);
	if (retval)
		return retval;

	is_load_balanced = is_sched_load_balance(trialcs);

	mutex_lock(&callback_mutex);
	cpumask_copy(cs->cpus_allowed, trialcs->cpus_allowed);
	mutex_unlock(&callback_mutex);

	/*
                                                            
                        
  */
	update_tasks_cpumask(cs, &heap);

	heap_free(&heap);

	if (is_load_balanced)
		rebuild_sched_domains_locked();
	return 0;
}

/*
                    
  
                                                             
  
                                                                       
                                                                   
  
                                                                 
                                                                     
                                                                  
                                                                    
                        
  
                                                                 
                                                                  
                                                                     
                              
 */

static void cpuset_migrate_mm(struct mm_struct *mm, const nodemask_t *from,
							const nodemask_t *to)
{
	struct task_struct *tsk = current;

	tsk->mems_allowed = *to;

	do_migrate_pages(mm, from, to, MPOL_MF_MOVE_ALL);

	guarantee_online_mems(task_cs(tsk),&tsk->mems_allowed);
}

/*
                                                                         
                           
                                                
  
                                                                           
                                                                             
                   
 */
static void cpuset_change_task_nodemask(struct task_struct *tsk,
					nodemask_t *newmems)
{
	bool need_loop;

	/*
                                                                     
                                           
  */
	if (unlikely(test_thread_flag(TIF_MEMDIE)))
		return;
	if (current->flags & PF_EXITING) /*                            */
		return;

	task_lock(tsk);
	/*
                                                               
                                                                   
                                                                     
                                                     
  */
	need_loop = task_has_mempolicy(tsk) ||
			!nodes_intersects(*newmems, tsk->mems_allowed);

	if (need_loop) {
		local_irq_disable();
		write_seqcount_begin(&tsk->mems_allowed_seq);
	}

	nodes_or(tsk->mems_allowed, tsk->mems_allowed, *newmems);
	mpol_rebind_task(tsk, newmems, MPOL_REBIND_STEP1);

	mpol_rebind_task(tsk, newmems, MPOL_REBIND_STEP2);
	tsk->mems_allowed = *newmems;

	if (need_loop) {
		write_seqcount_end(&tsk->mems_allowed_seq);
		local_irq_enable();
	}

	task_unlock(tsk);
}

/*
                                                                          
                                                                        
                                                             
 */
static void cpuset_change_nodemask(struct task_struct *p,
				   struct cgroup_scanner *scan)
{
	struct mm_struct *mm;
	struct cpuset *cs;
	int migrate;
	const nodemask_t *oldmem = scan->data;
	static nodemask_t newmems;	/*                           */

	cs = cgroup_cs(scan->cg);
	guarantee_online_mems(cs, &newmems);

	cpuset_change_task_nodemask(p, &newmems);

	mm = get_task_mm(p);
	if (!mm)
		return;

	migrate = is_memory_migrate(cs);

	mpol_rebind_mm(mm, &cs->mems_allowed);
	if (migrate)
		cpuset_migrate_mm(mm, oldmem, &cs->mems_allowed);
	mmput(mm);
}

static void *cpuset_being_rebound;

/* 
                                                                       
                                                                             
                                         
                                                                      
  
                                
                                                                             
                    
 */
static void update_tasks_nodemask(struct cpuset *cs, const nodemask_t *oldmem,
				 struct ptr_heap *heap)
{
	struct cgroup_scanner scan;

	cpuset_being_rebound = cs;		/*                          */

	scan.cg = cs->css.cgroup;
	scan.test_task = NULL;
	scan.process_task = cpuset_change_nodemask;
	scan.heap = heap;
	scan.data = (nodemask_t *)oldmem;

	/*
                                                               
                                                             
                                                                
                                                                
                                                                
                                                                    
                                                            
                                                               
  */
	cgroup_scan_tasks(&scan);

	/*                                                              */
	cpuset_being_rebound = NULL;
}

/*
                                                            
                                                          
                                                         
                                                              
                                                            
                                             
  
                                                                     
                                                                 
                                                               
                                                     
 */
static int update_nodemask(struct cpuset *cs, struct cpuset *trialcs,
			   const char *buf)
{
	NODEMASK_ALLOC(nodemask_t, oldmem, GFP_KERNEL);
	int retval;
	struct ptr_heap heap;

	if (!oldmem)
		return -ENOMEM;

	/*
                                                        
                  
  */
	if (cs == &top_cpuset) {
		retval = -EACCES;
		goto done;
	}

	/*
                                                                     
                                                                  
                                                                  
                           
  */
	if (!*buf) {
		nodes_clear(trialcs->mems_allowed);
	} else {
		retval = nodelist_parse(buf, trialcs->mems_allowed);
		if (retval < 0)
			goto done;

		if (!nodes_subset(trialcs->mems_allowed,
				node_states[N_MEMORY])) {
			retval =  -EINVAL;
			goto done;
		}
	}
	*oldmem = cs->mems_allowed;
	if (nodes_equal(*oldmem, trialcs->mems_allowed)) {
		retval = 0;		/*                          */
		goto done;
	}
	retval = validate_change(cs, trialcs);
	if (retval < 0)
		goto done;

	retval = heap_init(&heap, PAGE_SIZE, GFP_KERNEL, NULL);
	if (retval < 0)
		goto done;

	mutex_lock(&callback_mutex);
	cs->mems_allowed = trialcs->mems_allowed;
	mutex_unlock(&callback_mutex);

	update_tasks_nodemask(cs, oldmem, &heap);

	heap_free(&heap);
done:
	NODEMASK_FREE(oldmem);
	return retval;
}

int current_cpuset_is_being_rebound(void)
{
	int ret;

	rcu_read_lock();
	ret = task_cs(current) == cpuset_being_rebound;
	rcu_read_unlock();

	return ret;
}

static int update_relax_domain_level(struct cpuset *cs, s64 val)
{
#ifdef CONFIG_SMP
	if (val < -1 || val >= sched_domain_level_max)
		return -EINVAL;
#endif

	if (val != cs->relax_domain_level) {
		cs->relax_domain_level = val;
		if (!cpumask_empty(cs->cpus_allowed) &&
		    is_sched_load_balance(cs))
			rebuild_sched_domains_locked();
	}

	return 0;
}

/*
                                                                           
                           
                                                                 
  
                                                           
  
                                                                          
                                      
 */
static void cpuset_change_flag(struct task_struct *tsk,
				struct cgroup_scanner *scan)
{
	cpuset_update_task_spread_flag(cgroup_cs(scan->cg), tsk);
}

/*
                                                                       
                                                                        
                                                                      
  
                                
  
                                                                        
                                       
  
                                                                             
                    
 */
static void update_tasks_flags(struct cpuset *cs, struct ptr_heap *heap)
{
	struct cgroup_scanner scan;

	scan.cg = cs->css.cgroup;
	scan.test_task = NULL;
	scan.process_task = cpuset_change_flag;
	scan.heap = heap;
	cgroup_scan_tasks(&scan);
}

/*
                                                                     
                                                  
                            
                                                        
  
                               
 */

static int update_flag(cpuset_flagbits_t bit, struct cpuset *cs,
		       int turning_on)
{
	struct cpuset *trialcs;
	int balance_flag_changed;
	int spread_flag_changed;
	struct ptr_heap heap;
	int err;

	trialcs = alloc_trial_cpuset(cs);
	if (!trialcs)
		return -ENOMEM;

	if (turning_on)
		set_bit(bit, &trialcs->flags);
	else
		clear_bit(bit, &trialcs->flags);

	err = validate_change(cs, trialcs);
	if (err < 0)
		goto out;

	err = heap_init(&heap, PAGE_SIZE, GFP_KERNEL, NULL);
	if (err < 0)
		goto out;

	balance_flag_changed = (is_sched_load_balance(cs) !=
				is_sched_load_balance(trialcs));

	spread_flag_changed = ((is_spread_slab(cs) != is_spread_slab(trialcs))
			|| (is_spread_page(cs) != is_spread_page(trialcs)));

	mutex_lock(&callback_mutex);
	cs->flags = trialcs->flags;
	mutex_unlock(&callback_mutex);

	if (!cpumask_empty(trialcs->cpus_allowed) && balance_flag_changed)
		rebuild_sched_domains_locked();

	if (spread_flag_changed)
		update_tasks_flags(cs, &heap);
	heap_free(&heap);
out:
	free_trial_cpuset(trialcs);
	return err;
}

/*
                                                      
  
                                                                   
                                                   
                                                  
                                                             
                                                               
                                                              
  
                                                               
                                                                  
                                          
  
                                                                 
                                                                     
                                                                   
                                                               
  
                                                                  
                                                                 
                                                              
                                                                   
  
                                                                     
                                                                   
                                                                 
                  
  
                                                                     
                                                      
  
                                                                    
                                                                  
                                                                  
                                                                     
                                                                   
                                                               
                                                                   
                                                                  
                                                                
                                                                      
              
 */

#define FM_COEF 933		/*                                      */
#define FM_MAXTICKS ((time_t)99) /*                                        */
#define FM_MAXCNT 1000000	/*                             */
#define FM_SCALE 1000		/*                        */

/*                              */
static void fmeter_init(struct fmeter *fmp)
{
	fmp->cnt = 0;
	fmp->val = 0;
	fmp->time = 0;
	spin_lock_init(&fmp->lock);
}

/*                                                             */
static void fmeter_update(struct fmeter *fmp)
{
	time_t now = get_seconds();
	time_t ticks = now - fmp->time;

	if (ticks == 0)
		return;

	ticks = min(FM_MAXTICKS, ticks);
	while (ticks-- > 0)
		fmp->val = (FM_COEF * fmp->val) / FM_SCALE;
	fmp->time = now;

	fmp->val += ((FM_SCALE - FM_COEF) * fmp->cnt) / FM_SCALE;
	fmp->cnt = 0;
}

/*                                                                 */
static void fmeter_markevent(struct fmeter *fmp)
{
	spin_lock(&fmp->lock);
	fmeter_update(fmp);
	fmp->cnt = min(FM_MAXCNT, fmp->cnt + FM_SCALE);
	spin_unlock(&fmp->lock);
}

/*                                                        */
static int fmeter_getrate(struct fmeter *fmp)
{
	int val;

	spin_lock(&fmp->lock);
	fmeter_update(fmp);
	val = fmp->val;
	spin_unlock(&fmp->lock);
	return val;
}

/*                                                                         */
static int cpuset_can_attach(struct cgroup *cgrp, struct cgroup_taskset *tset)
{
	struct cpuset *cs = cgroup_cs(cgrp);
	struct task_struct *task;
	int ret;

	mutex_lock(&cpuset_mutex);

	ret = -ENOSPC;
	if (cpumask_empty(cs->cpus_allowed) || nodes_empty(cs->mems_allowed))
		goto out_unlock;

	cgroup_taskset_for_each(task, cgrp, tset) {
		/*
                                                           
                                                       
                                                        
                                                         
                                                             
                                                            
                                        
   */
		ret = -EINVAL;
		if (task->flags & PF_NO_SETAFFINITY)
			goto out_unlock;
		ret = security_task_setscheduler(task);
		if (ret)
			goto out_unlock;
	}

	/*
                                                                  
                                         
  */
	cs->attach_in_progress++;
	ret = 0;
out_unlock:
	mutex_unlock(&cpuset_mutex);
	return ret;
}

static void cpuset_cancel_attach(struct cgroup *cgrp,
				 struct cgroup_taskset *tset)
{
	mutex_lock(&cpuset_mutex);
	cgroup_cs(cgrp)->attach_in_progress--;
	mutex_unlock(&cpuset_mutex);
}

/*
                                                                          
                                                                    
                               
 */
static cpumask_var_t cpus_attach;

static void cpuset_attach(struct cgroup *cgrp, struct cgroup_taskset *tset)
{
	/*                                       */
	static nodemask_t cpuset_attach_nodemask_from;
	static nodemask_t cpuset_attach_nodemask_to;
	struct mm_struct *mm;
	struct task_struct *task;
	struct task_struct *leader = cgroup_taskset_first(tset);
	struct cgroup *oldcgrp = cgroup_taskset_cur_cgroup(tset);
	struct cpuset *cs = cgroup_cs(cgrp);
	struct cpuset *oldcs = cgroup_cs(oldcgrp);

	mutex_lock(&cpuset_mutex);

	/*                    */
	if (cs == &top_cpuset)
		cpumask_copy(cpus_attach, cpu_possible_mask);
	else
		guarantee_online_cpus(cs, cpus_attach);

	guarantee_online_mems(cs, &cpuset_attach_nodemask_to);

	cgroup_taskset_for_each(task, cgrp, tset) {
		/*
                                                             
                                                          
   */
		WARN_ON_ONCE(set_cpus_allowed_ptr(task, cpus_attach));

		cpuset_change_task_nodemask(task, &cpuset_attach_nodemask_to);
		cpuset_update_task_spread_flag(cs, task);
	}

	/*
                                                                      
                            
  */
	cpuset_attach_nodemask_from = oldcs->mems_allowed;
	cpuset_attach_nodemask_to = cs->mems_allowed;
	mm = get_task_mm(leader);
	if (mm) {
		mpol_rebind_mm(mm, &cpuset_attach_nodemask_to);
		if (is_memory_migrate(cs))
			cpuset_migrate_mm(mm, &cpuset_attach_nodemask_from,
					  &cpuset_attach_nodemask_to);
		mmput(mm);
	}

	cs->attach_in_progress--;

	/*
                                                                 
                                                                    
                                                                  
  */
	if (cpumask_empty(cs->cpus_allowed) || nodes_empty(cs->mems_allowed))
		schedule_cpuset_propagate_hotplug(cs);

	mutex_unlock(&cpuset_mutex);
}

/*                                                                    */

typedef enum {
	FILE_MEMORY_MIGRATE,
	FILE_CPULIST,
	FILE_MEMLIST,
	FILE_CPU_EXCLUSIVE,
	FILE_MEM_EXCLUSIVE,
	FILE_MEM_HARDWALL,
	FILE_SCHED_LOAD_BALANCE,
	FILE_SCHED_RELAX_DOMAIN_LEVEL,
	FILE_MEMORY_PRESSURE_ENABLED,
	FILE_MEMORY_PRESSURE,
	FILE_SPREAD_PAGE,
	FILE_SPREAD_SLAB,
} cpuset_filetype_t;

static int cpuset_write_u64(struct cgroup *cgrp, struct cftype *cft, u64 val)
{
	struct cpuset *cs = cgroup_cs(cgrp);
	cpuset_filetype_t type = cft->private;
	int retval = 0;

	mutex_lock(&cpuset_mutex);
	if (!is_cpuset_online(cs)) {
		retval = -ENODEV;
		goto out_unlock;
	}

	switch (type) {
	case FILE_CPU_EXCLUSIVE:
		retval = update_flag(CS_CPU_EXCLUSIVE, cs, val);
		break;
	case FILE_MEM_EXCLUSIVE:
		retval = update_flag(CS_MEM_EXCLUSIVE, cs, val);
		break;
	case FILE_MEM_HARDWALL:
		retval = update_flag(CS_MEM_HARDWALL, cs, val);
		break;
	case FILE_SCHED_LOAD_BALANCE:
		retval = update_flag(CS_SCHED_LOAD_BALANCE, cs, val);
		break;
	case FILE_MEMORY_MIGRATE:
		retval = update_flag(CS_MEMORY_MIGRATE, cs, val);
		break;
	case FILE_MEMORY_PRESSURE_ENABLED:
		cpuset_memory_pressure_enabled = !!val;
		break;
	case FILE_MEMORY_PRESSURE:
		retval = -EACCES;
		break;
	case FILE_SPREAD_PAGE:
		retval = update_flag(CS_SPREAD_PAGE, cs, val);
		break;
	case FILE_SPREAD_SLAB:
		retval = update_flag(CS_SPREAD_SLAB, cs, val);
		break;
	default:
		retval = -EINVAL;
		break;
	}
out_unlock:
	mutex_unlock(&cpuset_mutex);
	return retval;
}

static int cpuset_write_s64(struct cgroup *cgrp, struct cftype *cft, s64 val)
{
	struct cpuset *cs = cgroup_cs(cgrp);
	cpuset_filetype_t type = cft->private;
	int retval = -ENODEV;

	mutex_lock(&cpuset_mutex);
	if (!is_cpuset_online(cs))
		goto out_unlock;

	switch (type) {
	case FILE_SCHED_RELAX_DOMAIN_LEVEL:
		retval = update_relax_domain_level(cs, val);
		break;
	default:
		retval = -EINVAL;
		break;
	}
out_unlock:
	mutex_unlock(&cpuset_mutex);
	return retval;
}

/*
                                                          
 */
static int cpuset_write_resmask(struct cgroup *cgrp, struct cftype *cft,
				const char *buf)
{
	struct cpuset *cs = cgroup_cs(cgrp);
	struct cpuset *trialcs;
	int retval = -ENODEV;

	/*
                                                           
                                                                    
                                                                 
                      
   
                                                             
                                                                  
                                                                 
                                           
   
                                                                 
                                                           
                                                        
  */
	flush_work(&cpuset_hotplug_work);
	flush_workqueue(cpuset_propagate_hotplug_wq);

	mutex_lock(&cpuset_mutex);
	if (!is_cpuset_online(cs))
		goto out_unlock;

	trialcs = alloc_trial_cpuset(cs);
	if (!trialcs) {
		retval = -ENOMEM;
		goto out_unlock;
	}

	switch (cft->private) {
	case FILE_CPULIST:
		retval = update_cpumask(cs, trialcs, buf);
		break;
	case FILE_MEMLIST:
		retval = update_nodemask(cs, trialcs, buf);
		break;
	default:
		retval = -EINVAL;
		break;
	}

	free_trial_cpuset(trialcs);
out_unlock:
	mutex_unlock(&cpuset_mutex);
	return retval;
}

/*
                                                                     
                                                                  
                                                                        
                                                                  
                                                                    
                                                              
                                                                  
                                                                     
                       
 */

static size_t cpuset_sprintf_cpulist(char *page, struct cpuset *cs)
{
	size_t count;

	mutex_lock(&callback_mutex);
	count = cpulist_scnprintf(page, PAGE_SIZE, cs->cpus_allowed);
	mutex_unlock(&callback_mutex);

	return count;
}

static size_t cpuset_sprintf_memlist(char *page, struct cpuset *cs)
{
	size_t count;

	mutex_lock(&callback_mutex);
	count = nodelist_scnprintf(page, PAGE_SIZE, cs->mems_allowed);
	mutex_unlock(&callback_mutex);

	return count;
}

static ssize_t cpuset_common_file_read(struct cgroup *cont,
				       struct cftype *cft,
				       struct file *file,
				       char __user *buf,
				       size_t nbytes, loff_t *ppos)
{
	struct cpuset *cs = cgroup_cs(cont);
	cpuset_filetype_t type = cft->private;
	char *page;
	ssize_t retval = 0;
	char *s;

	if (!(page = (char *)__get_free_page(GFP_TEMPORARY)))
		return -ENOMEM;

	s = page;

	switch (type) {
	case FILE_CPULIST:
		s += cpuset_sprintf_cpulist(s, cs);
		break;
	case FILE_MEMLIST:
		s += cpuset_sprintf_memlist(s, cs);
		break;
	default:
		retval = -EINVAL;
		goto out;
	}
	*s++ = '\n';

	retval = simple_read_from_buffer(buf, nbytes, ppos, page, s - page);
out:
	free_page((unsigned long)page);
	return retval;
}

static u64 cpuset_read_u64(struct cgroup *cont, struct cftype *cft)
{
	struct cpuset *cs = cgroup_cs(cont);
	cpuset_filetype_t type = cft->private;
	switch (type) {
	case FILE_CPU_EXCLUSIVE:
		return is_cpu_exclusive(cs);
	case FILE_MEM_EXCLUSIVE:
		return is_mem_exclusive(cs);
	case FILE_MEM_HARDWALL:
		return is_mem_hardwall(cs);
	case FILE_SCHED_LOAD_BALANCE:
		return is_sched_load_balance(cs);
	case FILE_MEMORY_MIGRATE:
		return is_memory_migrate(cs);
	case FILE_MEMORY_PRESSURE_ENABLED:
		return cpuset_memory_pressure_enabled;
	case FILE_MEMORY_PRESSURE:
		return fmeter_getrate(&cs->fmeter);
	case FILE_SPREAD_PAGE:
		return is_spread_page(cs);
	case FILE_SPREAD_SLAB:
		return is_spread_slab(cs);
	default:
		BUG();
	}

	/*                                 */
	return 0;
}

static s64 cpuset_read_s64(struct cgroup *cont, struct cftype *cft)
{
	struct cpuset *cs = cgroup_cs(cont);
	cpuset_filetype_t type = cft->private;
	switch (type) {
	case FILE_SCHED_RELAX_DOMAIN_LEVEL:
		return cs->relax_domain_level;
	default:
		BUG();
	}

	/*                                */
	return 0;
}


/*
                                                             
 */

static struct cftype files[] = {
	{
		.name = "cpus",
		.read = cpuset_common_file_read,
		.write_string = cpuset_write_resmask,
		.max_write_len = (100U + 6 * NR_CPUS),
		.private = FILE_CPULIST,
	},

	{
		.name = "mems",
		.read = cpuset_common_file_read,
		.write_string = cpuset_write_resmask,
		.max_write_len = (100U + 6 * MAX_NUMNODES),
		.private = FILE_MEMLIST,
	},

	{
		.name = "cpu_exclusive",
		.read_u64 = cpuset_read_u64,
		.write_u64 = cpuset_write_u64,
		.private = FILE_CPU_EXCLUSIVE,
	},

	{
		.name = "mem_exclusive",
		.read_u64 = cpuset_read_u64,
		.write_u64 = cpuset_write_u64,
		.private = FILE_MEM_EXCLUSIVE,
	},

	{
		.name = "mem_hardwall",
		.read_u64 = cpuset_read_u64,
		.write_u64 = cpuset_write_u64,
		.private = FILE_MEM_HARDWALL,
	},

	{
		.name = "sched_load_balance",
		.read_u64 = cpuset_read_u64,
		.write_u64 = cpuset_write_u64,
		.private = FILE_SCHED_LOAD_BALANCE,
	},

	{
		.name = "sched_relax_domain_level",
		.read_s64 = cpuset_read_s64,
		.write_s64 = cpuset_write_s64,
		.private = FILE_SCHED_RELAX_DOMAIN_LEVEL,
	},

	{
		.name = "memory_migrate",
		.read_u64 = cpuset_read_u64,
		.write_u64 = cpuset_write_u64,
		.private = FILE_MEMORY_MIGRATE,
	},

	{
		.name = "memory_pressure",
		.read_u64 = cpuset_read_u64,
		.write_u64 = cpuset_write_u64,
		.private = FILE_MEMORY_PRESSURE,
		.mode = S_IRUGO,
	},

	{
		.name = "memory_spread_page",
		.read_u64 = cpuset_read_u64,
		.write_u64 = cpuset_write_u64,
		.private = FILE_SPREAD_PAGE,
	},

	{
		.name = "memory_spread_slab",
		.read_u64 = cpuset_read_u64,
		.write_u64 = cpuset_write_u64,
		.private = FILE_SPREAD_SLAB,
	},

	{
		.name = "memory_pressure_enabled",
		.flags = CFTYPE_ONLY_ON_ROOT,
		.read_u64 = cpuset_read_u64,
		.write_u64 = cpuset_write_u64,
		.private = FILE_MEMORY_PRESSURE_ENABLED,
	},

	{ }	/*           */
};

/*
                                           
                                                          
 */

static struct cgroup_subsys_state *cpuset_css_alloc(struct cgroup *cont)
{
	struct cpuset *cs;

	if (!cont->parent)
		return &top_cpuset.css;

	cs = kzalloc(sizeof(*cs), GFP_KERNEL);
	if (!cs)
		return ERR_PTR(-ENOMEM);
	if (!alloc_cpumask_var(&cs->cpus_allowed, GFP_KERNEL)) {
		kfree(cs);
		return ERR_PTR(-ENOMEM);
	}

	set_bit(CS_SCHED_LOAD_BALANCE, &cs->flags);
	cpumask_clear(cs->cpus_allowed);
	nodes_clear(cs->mems_allowed);
	fmeter_init(&cs->fmeter);
	INIT_WORK(&cs->hotplug_work, cpuset_propagate_hotplug_workfn);
	cs->relax_domain_level = -1;

	return &cs->css;
}

static int cpuset_css_online(struct cgroup *cgrp)
{
	struct cpuset *cs = cgroup_cs(cgrp);
	struct cpuset *parent = parent_cs(cs);
	struct cpuset *tmp_cs;
	struct cgroup *pos_cg;

	if (!parent)
		return 0;

	mutex_lock(&cpuset_mutex);

	set_bit(CS_ONLINE, &cs->flags);
	if (is_spread_page(parent))
		set_bit(CS_SPREAD_PAGE, &cs->flags);
	if (is_spread_slab(parent))
		set_bit(CS_SPREAD_SLAB, &cs->flags);

	number_of_cpusets++;

	if (!test_bit(CGRP_CPUSET_CLONE_CHILDREN, &cgrp->flags))
		goto out_unlock;

	/*
                                                                  
                                                              
                                                               
   
                                                                    
                                                                    
                                                             
                                                                   
                                                             
                                                                
                                              
  */
	rcu_read_lock();
	cpuset_for_each_child(tmp_cs, pos_cg, parent) {
		if (is_mem_exclusive(tmp_cs) || is_cpu_exclusive(tmp_cs)) {
			rcu_read_unlock();
			goto out_unlock;
		}
	}
	rcu_read_unlock();

	mutex_lock(&callback_mutex);
	cs->mems_allowed = parent->mems_allowed;
	cpumask_copy(cs->cpus_allowed, parent->cpus_allowed);
	mutex_unlock(&callback_mutex);
out_unlock:
	mutex_unlock(&cpuset_mutex);
	return 0;
}

static void cpuset_css_offline(struct cgroup *cgrp)
{
	struct cpuset *cs = cgroup_cs(cgrp);

	mutex_lock(&cpuset_mutex);

	if (is_sched_load_balance(cs))
		update_flag(CS_SCHED_LOAD_BALANCE, cs, 0);

	number_of_cpusets--;
	clear_bit(CS_ONLINE, &cs->flags);

	mutex_unlock(&cpuset_mutex);
}

/*
                                                                
                                                               
                                            
 */

static void cpuset_css_free(struct cgroup *cont)
{
	struct cpuset *cs = cgroup_cs(cont);

	free_cpumask_var(cs->cpus_allowed);
	kfree(cs);
}

struct cgroup_subsys cpuset_subsys = {
	.name = "cpuset",
	.css_alloc = cpuset_css_alloc,
	.css_online = cpuset_css_online,
	.css_offline = cpuset_css_offline,
	.css_free = cpuset_css_free,
	.can_attach = cpuset_can_attach,
	.cancel_attach = cpuset_cancel_attach,
	.attach = cpuset_attach,
	.subsys_id = cpuset_subsys_id,
	.base_cftypes = files,
	.early_init = 1,
};

/* 
                                                  
  
                                                                          
  */

int __init cpuset_init(void)
{
	int err = 0;

	if (!alloc_cpumask_var(&top_cpuset.cpus_allowed, GFP_KERNEL))
		BUG();

	cpumask_setall(top_cpuset.cpus_allowed);
	nodes_setall(top_cpuset.mems_allowed);

	fmeter_init(&top_cpuset.fmeter);
	set_bit(CS_SCHED_LOAD_BALANCE, &top_cpuset.flags);
	top_cpuset.relax_domain_level = -1;

	err = register_filesystem(&cpuset_fs_type);
	if (err < 0)
		return err;

	if (!alloc_cpumask_var(&cpus_attach, GFP_KERNEL))
		BUG();

	number_of_cpusets = 1;
	return 0;
}

/*
                                                                
                                                              
                                                                   
                                                                   
                                               
 */
static void remove_tasks_in_empty_cpuset(struct cpuset *cs)
{
	struct cpuset *parent;

	/*
                                                       
                                        
  */
	parent = parent_cs(cs);
	while (cpumask_empty(parent->cpus_allowed) ||
			nodes_empty(parent->mems_allowed))
		parent = parent_cs(parent);

	if (cgroup_transfer_tasks(parent->css.cgroup, cs->css.cgroup)) {
		rcu_read_lock();
		printk(KERN_ERR "cpuset: failed to transfer tasks out of empty cpuset %s\n",
		       cgroup_name(cs->css.cgroup));
		rcu_read_unlock();
	}
}

/* 
                                                                             
                          
  
                                                                           
                                                                          
                                                                       
 */
static void cpuset_propagate_hotplug_workfn(struct work_struct *work)
{
	static cpumask_t off_cpus;
	static nodemask_t off_mems, tmp_mems;
	struct cpuset *cs = container_of(work, struct cpuset, hotplug_work);
	bool is_empty;

	mutex_lock(&cpuset_mutex);

	cpumask_andnot(&off_cpus, cs->cpus_allowed, top_cpuset.cpus_allowed);
	nodes_andnot(off_mems, cs->mems_allowed, top_cpuset.mems_allowed);

	/*                              */
	if (!cpumask_empty(&off_cpus)) {
		mutex_lock(&callback_mutex);
		cpumask_andnot(cs->cpus_allowed, cs->cpus_allowed, &off_cpus);
		mutex_unlock(&callback_mutex);
		update_tasks_cpumask(cs, NULL);
	}

	/*                              */
	if (!nodes_empty(off_mems)) {
		tmp_mems = cs->mems_allowed;
		mutex_lock(&callback_mutex);
		nodes_andnot(cs->mems_allowed, cs->mems_allowed, off_mems);
		mutex_unlock(&callback_mutex);
		update_tasks_nodemask(cs, &tmp_mems, NULL);
	}

	is_empty = cpumask_empty(cs->cpus_allowed) ||
		nodes_empty(cs->mems_allowed);

	mutex_unlock(&cpuset_mutex);

	/*
                                                                
                                                                  
                                                                 
  */
	if (is_empty)
		remove_tasks_in_empty_cpuset(cs);

	/*                                                          */
	css_put(&cs->css);
}

/* 
                                                                               
                          
  
                                                                       
                                        
 */
static void schedule_cpuset_propagate_hotplug(struct cpuset *cs)
{
	/*
                                                            
                       
  */
	if (!css_tryget(&cs->css))
		return;

	/*
                                                                   
                                                               
                                                
  */
	if (!queue_work(cpuset_propagate_hotplug_wq, &cs->hotplug_work))
		css_put(&cs->css);
}

/* 
                                                                   
  
                                                                       
                                                                    
                                                                      
                                                                       
                                                                  
  
                                                                          
                                                                           
               
  
                                                                      
                                               
 */
static void cpuset_hotplug_workfn(struct work_struct *work)
{
	static cpumask_t new_cpus, tmp_cpus;
	static nodemask_t new_mems, tmp_mems;
	bool cpus_updated, mems_updated;
	bool cpus_offlined, mems_offlined;

	mutex_lock(&cpuset_mutex);

	/*                                                              */
	cpumask_copy(&new_cpus, cpu_active_mask);
	new_mems = node_states[N_MEMORY];

	cpus_updated = !cpumask_equal(top_cpuset.cpus_allowed, &new_cpus);
	cpus_offlined = cpumask_andnot(&tmp_cpus, top_cpuset.cpus_allowed,
				       &new_cpus);

	mems_updated = !nodes_equal(top_cpuset.mems_allowed, new_mems);
	nodes_andnot(tmp_mems, top_cpuset.mems_allowed, new_mems);
	mems_offlined = !nodes_empty(tmp_mems);

	/*                                             */
	if (cpus_updated) {
		mutex_lock(&callback_mutex);
		cpumask_copy(top_cpuset.cpus_allowed, &new_cpus);
		mutex_unlock(&callback_mutex);
		/*                                                    */
	}

	/*                                      */
	if (mems_updated) {
		tmp_mems = top_cpuset.mems_allowed;
		mutex_lock(&callback_mutex);
		top_cpuset.mems_allowed = new_mems;
		mutex_unlock(&callback_mutex);
		update_tasks_nodemask(&top_cpuset, &tmp_mems, NULL);
	}

	/*                                                                */
	if (cpus_offlined || mems_offlined) {
		struct cpuset *cs;
		struct cgroup *pos_cgrp;

		rcu_read_lock();
		cpuset_for_each_descendant_pre(cs, pos_cgrp, &top_cpuset)
			schedule_cpuset_propagate_hotplug(cs);
		rcu_read_unlock();
	}

	mutex_unlock(&cpuset_mutex);

	/*                                 */
	flush_workqueue(cpuset_propagate_hotplug_wq);

	/*                                                   */
	if (cpus_updated)
		rebuild_sched_domains();
}

void cpuset_update_active_cpus(bool cpu_online)
{
	/*
                                                                
                                                                    
                                                  
   
                                                                
                                                                   
                                                      
                                                         
  */
	partition_sched_domains(1, NULL, NULL);
	schedule_work(&cpuset_hotplug_work);
}

/*
                                                               
                                                                 
                                                            
 */
static int cpuset_track_online_nodes(struct notifier_block *self,
				unsigned long action, void *arg)
{
	schedule_work(&cpuset_hotplug_work);
	return NOTIFY_OK;
}

static struct notifier_block cpuset_track_online_nodes_nb = {
	.notifier_call = cpuset_track_online_nodes,
	.priority = 10,		/*     */
};

/* 
                                            
  
                                                                      
 */
void __init cpuset_init_smp(void)
{
	cpumask_copy(top_cpuset.cpus_allowed, cpu_active_mask);
	top_cpuset.mems_allowed = node_states[N_MEMORY];

	register_hotmemory_notifier(&cpuset_track_online_nodes_nb);

	cpuset_propagate_hotplug_wq =
		alloc_ordered_workqueue("cpuset_hotplug", 0);
	BUG_ON(!cpuset_propagate_hotplug_wq);
}

/* 
                                                                      
                                                                          
                                                                          
  
                                                                    
                                                                       
                                                                  
                
  */

void cpuset_cpus_allowed(struct task_struct *tsk, struct cpumask *pmask)
{
	mutex_lock(&callback_mutex);
	task_lock(tsk);
	guarantee_online_cpus(task_cs(tsk), pmask);
	task_unlock(tsk);
	mutex_unlock(&callback_mutex);
}

void cpuset_cpus_allowed_fallback(struct task_struct *tsk)
{
	const struct cpuset *cs;

	rcu_read_lock();
	cs = task_cs(tsk);
	if (cs)
		do_set_cpus_allowed(tsk, cs->cpus_allowed);
	rcu_read_unlock();

	/*
                                                            
   
                                                            
                                                              
                                                              
                                                              
                               
   
                                                              
                                                                 
                                                            
                                                       
   
                                                                      
                
  */
}

void cpuset_init_current_mems_allowed(void)
{
	nodes_setall(current->mems_allowed);
}

/* 
                                                                      
                                                                          
  
                                                                 
                                                                       
                                                                        
                
  */

nodemask_t cpuset_mems_allowed(struct task_struct *tsk)
{
	nodemask_t mask;

	mutex_lock(&callback_mutex);
	task_lock(tsk);
	guarantee_online_mems(task_cs(tsk), &mask);
	task_unlock(tsk);
	mutex_unlock(&callback_mutex);

	return mask;
}

/* 
                                                                               
                                        
  
                                                                         
 */
int cpuset_nodemask_valid_mems_allowed(nodemask_t *nodemask)
{
	return nodes_intersects(*nodemask, current->mems_allowed);
}

/*
                                                                     
                                                               
                                                                   
                                                            
 */
static const struct cpuset *nearest_hardwall_ancestor(const struct cpuset *cs)
{
	while (!(is_mem_exclusive(cs) || is_mem_hardwall(cs)) && parent_cs(cs))
		cs = parent_cs(cs);
	return cs;
}

/* 
                                                                   
                                  
                                     
  
                                                                            
                                                                            
                                                                             
                                                                               
                                                                              
             
                 
  
                                                                      
                                                                             
                                                                
  
                                                                           
                             
  
                                                                  
                                                                    
                                                                   
                                                                   
                                                                   
  
                                                               
                                                                
                                                               
                                                                 
                                                
  
                                                           
                                                                  
                                                                  
                                                                  
                                                                     
                                                                      
         
  
                                                                  
                                                                  
                                                                   
                            
  
                                                                     
                                                                   
                                                                  
                                                                     
               
                                                               
                             
                             
                                                            
                                                              
  
        
                                                                            
                                                                     
                                                             
 */
int __cpuset_node_allowed_softwall(int node, gfp_t gfp_mask)
{
	const struct cpuset *cs;	/*                          */
	int allowed;			/*                                  */

	if (in_interrupt() || (gfp_mask & __GFP_THISNODE))
		return 1;
	might_sleep_if(!(gfp_mask & __GFP_HARDWALL));
	if (node_isset(node, current->mems_allowed))
		return 1;
	/*
                                                                     
                                           
  */
	if (unlikely(test_thread_flag(TIF_MEMDIE)))
		return 1;
	if (gfp_mask & __GFP_HARDWALL)	/*                                */
		return 0;

	if (current->flags & PF_EXITING) /*                            */
		return 1;

	/*                                                             */
	mutex_lock(&callback_mutex);

	task_lock(current);
	cs = nearest_hardwall_ancestor(task_cs(current));
	allowed = node_isset(node, cs->mems_allowed);
	task_unlock(current);

	mutex_unlock(&callback_mutex);
	return allowed;
}

/*
                                                                   
                                  
                                     
  
                                                                            
                                                                            
                                                                             
                                         
                 
  
                                                                  
                                                                    
                                                                   
                                                                   
                                                                   
  
                                                            
                                                               
                                                                    
                                                                   
                   
 */
int __cpuset_node_allowed_hardwall(int node, gfp_t gfp_mask)
{
	if (in_interrupt() || (gfp_mask & __GFP_THISNODE))
		return 1;
	if (node_isset(node, current->mems_allowed))
		return 1;
	/*
                                                                     
                                           
  */
	if (unlikely(test_thread_flag(TIF_MEMDIE)))
		return 1;
	return 0;
}

/* 
                                                                           
                                                                            
  
                                                               
                                                                
                                                             
                                                              
                                                               
                                                                   
                                                                 
                                            
  
                                                               
                                                                 
  
                                                              
                                                               
                                                             
                                                                  
                                                               
                                                                  
                                                                 
                                                                  
                               
 */

static int cpuset_spread_node(int *rotor)
{
	int node;

	node = next_node(*rotor, current->mems_allowed);
	if (node == MAX_NUMNODES)
		node = first_node(current->mems_allowed);
	*rotor = node;
	return node;
}

int cpuset_mem_spread_node(void)
{
	if (current->cpuset_mem_spread_rotor == NUMA_NO_NODE)
		current->cpuset_mem_spread_rotor =
			node_random(&current->mems_allowed);

	return cpuset_spread_node(&current->cpuset_mem_spread_rotor);
}

int cpuset_slab_spread_node(void)
{
	if (current->cpuset_slab_spread_rotor == NUMA_NO_NODE)
		current->cpuset_slab_spread_rotor =
			node_random(&current->mems_allowed);

	return cpuset_spread_node(&current->cpuset_slab_spread_rotor);
}

EXPORT_SYMBOL_GPL(cpuset_mem_spread_node);

/* 
                                                                                
                                              
                                                    
  
                                                                  
                                                                 
                                                                   
                
  */

int cpuset_mems_allowed_intersects(const struct task_struct *tsk1,
				   const struct task_struct *tsk2)
{
	return nodes_intersects(tsk1->mems_allowed, tsk2->mems_allowed);
}

#define CPUSET_NODELIST_LEN	(256)

/* 
                                                                         
                                              
  
                                                                        
                                                                      
                               
 */
void cpuset_print_task_mems_allowed(struct task_struct *tsk)
{
	 /*                                                     */
	static char cpuset_nodelist[CPUSET_NODELIST_LEN];
	static DEFINE_SPINLOCK(cpuset_buffer_lock);

	struct cgroup *cgrp = task_cs(tsk)->css.cgroup;

	rcu_read_lock();
	spin_lock(&cpuset_buffer_lock);

	nodelist_scnprintf(cpuset_nodelist, CPUSET_NODELIST_LEN,
			   tsk->mems_allowed);
	printk(KERN_INFO "%s cpuset=%s mems_allowed=%s\n",
	       tsk->comm, cgroup_name(cgrp), cpuset_nodelist);

	spin_unlock(&cpuset_buffer_lock);
	rcu_read_unlock();
}

/*
                                                     
                                                     
                                                            
 */

int cpuset_memory_pressure_enabled __read_mostly;

/* 
                                                                   
  
                                                             
                                                          
  
                                                            
                                                            
                                                             
                                                             
                          
  
                                                         
                                                    
                                                             
                                                            
  */

void __cpuset_memory_pressure_bump(void)
{
	task_lock(current);
	fmeter_markevent(&task_cs(current)->fmeter);
	task_unlock(current);
}

#ifdef CONFIG_PROC_PID_CPUSET
/*
                     
                                            
                                  
                                                                    
                                                                    
                                                                        
             
 */
int proc_cpuset_show(struct seq_file *m, void *unused_v)
{
	struct pid *pid;
	struct task_struct *tsk;
	char *buf;
	struct cgroup_subsys_state *css;
	int retval;

	retval = -ENOMEM;
	buf = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!buf)
		goto out;

	retval = -ESRCH;
	pid = m->private;
	tsk = get_pid_task(pid, PIDTYPE_PID);
	if (!tsk)
		goto out_free;

	rcu_read_lock();
	css = task_subsys_state(tsk, cpuset_subsys_id);
	retval = cgroup_path(css->cgroup, buf, PAGE_SIZE);
	rcu_read_unlock();
	if (retval < 0)
		goto out_put_task;
	seq_puts(m, buf);
	seq_putc(m, '\n');
out_put_task:
	put_task_struct(tsk);
out_free:
	kfree(buf);
out:
	return retval;
}
#endif /*                        */

/*                                                       */
void cpuset_task_status_allowed(struct seq_file *m, struct task_struct *task)
{
	seq_printf(m, "Mems_allowed:\t");
	seq_nodemask(m, &task->mems_allowed);
	seq_printf(m, "\n");
	seq_printf(m, "Mems_allowed_list:\t");
	seq_nodemask_list(m, &task->mems_allowed);
	seq_printf(m, "\n");
}
