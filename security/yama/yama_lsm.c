/*
 * Yama Linux Security Module
 *
 * Author: Kees Cook <keescook@chromium.org>
 *
 * Copyright (C) 2010 Canonical, Ltd.
 * Copyright (C) 2011 The Chromium OS Authors.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2, as
 * published by the Free Software Foundation.
 *
 */

#include <linux/security.h>
#include <linux/sysctl.h>
#include <linux/ptrace.h>
#include <linux/prctl.h>
#include <linux/ratelimit.h>
#include <linux/workqueue.h>

#define YAMA_SCOPE_DISABLED	0
#define YAMA_SCOPE_RELATIONAL	1
#define YAMA_SCOPE_CAPABILITY	2
#define YAMA_SCOPE_NO_ATTACH	3

static int ptrace_scope = YAMA_SCOPE_RELATIONAL;

/*                                                        */
struct ptrace_relation {
	struct task_struct *tracer;
	struct task_struct *tracee;
	bool invalid;
	struct list_head node;
	struct rcu_head rcu;
};

static LIST_HEAD(ptracer_relations);
static DEFINE_SPINLOCK(ptracer_relations_lock);

static void yama_relation_cleanup(struct work_struct *work);
static DECLARE_WORK(yama_relation_work, yama_relation_cleanup);

/* 
                                                                        
  
 */
static void yama_relation_cleanup(struct work_struct *work)
{
	struct ptrace_relation *relation;

	spin_lock(&ptracer_relations_lock);
	rcu_read_lock();
	list_for_each_entry_rcu(relation, &ptracer_relations, node) {
		if (relation->invalid) {
			list_del_rcu(&relation->node);
			kfree_rcu(relation, rcu);
		}
	}
	rcu_read_unlock();
	spin_unlock(&ptracer_relations_lock);
}

/* 
                                                                          
                                                           
                                                        
  
                                                                       
                                                                          
  
                                                     
 */
static int yama_ptracer_add(struct task_struct *tracer,
			    struct task_struct *tracee)
{
	struct ptrace_relation *relation, *added;

	added = kmalloc(sizeof(*added), GFP_KERNEL);
	if (!added)
		return -ENOMEM;

	added->tracee = tracee;
	added->tracer = tracer;
	added->invalid = false;

	spin_lock(&ptracer_relations_lock);
	rcu_read_lock();
	list_for_each_entry_rcu(relation, &ptracer_relations, node) {
		if (relation->invalid)
			continue;
		if (relation->tracee == tracee) {
			list_replace_rcu(&relation->node, &added->node);
			kfree_rcu(relation, rcu);
			goto out;
		}
	}

	list_add_rcu(&added->node, &ptracer_relations);

out:
	rcu_read_unlock();
	spin_unlock(&ptracer_relations_lock);
	return 0;
}

/* 
                                                                  
                                                         
                                                         
 */
static void yama_ptracer_del(struct task_struct *tracer,
			     struct task_struct *tracee)
{
	struct ptrace_relation *relation;
	bool marked = false;

	rcu_read_lock();
	list_for_each_entry_rcu(relation, &ptracer_relations, node) {
		if (relation->invalid)
			continue;
		if (relation->tracee == tracee ||
		    (tracer && relation->tracer == tracer)) {
			relation->invalid = true;
			marked = true;
		}
	}
	rcu_read_unlock();

	if (marked)
		schedule_work(&yama_relation_work);
}

/* 
                                                                    
                            
 */
void yama_task_free(struct task_struct *task)
{
	yama_ptracer_del(task, task);
}

/* 
                                                             
                     
                  
                  
                  
                  
  
                                                                    
                                    
 */
int yama_task_prctl(int option, unsigned long arg2, unsigned long arg3,
			   unsigned long arg4, unsigned long arg5)
{
	int rc;
	struct task_struct *myself = current;

	rc = cap_task_prctl(option, arg2, arg3, arg4, arg5);
	if (rc != -ENOSYS)
		return rc;

	switch (option) {
	case PR_SET_PTRACER:
		/*                                                       
                                                         
                                                           
                                                               
                                        
   */
		rcu_read_lock();
		if (!thread_group_leader(myself))
			myself = rcu_dereference(myself->group_leader);
		get_task_struct(myself);
		rcu_read_unlock();

		if (arg2 == 0) {
			yama_ptracer_del(NULL, myself);
			rc = 0;
		} else if (arg2 == PR_SET_PTRACER_ANY || (int)arg2 == -1) {
			rc = yama_ptracer_add(NULL, myself);
		} else {
			struct task_struct *tracer;

			rcu_read_lock();
			tracer = find_task_by_vpid(arg2);
			if (tracer)
				get_task_struct(tracer);
			else
				rc = -EINVAL;
			rcu_read_unlock();

			if (tracer) {
				rc = yama_ptracer_add(tracer, myself);
				put_task_struct(tracer);
			}
		}

		put_task_struct(myself);
		break;
	}

	return rc;
}

/* 
                                                                         
                                                                      
                                                                     
  
                                                          
 */
static int task_is_descendant(struct task_struct *parent,
			      struct task_struct *child)
{
	int rc = 0;
	struct task_struct *walker = child;

	if (!parent || !child)
		return 0;

	rcu_read_lock();
	if (!thread_group_leader(parent))
		parent = rcu_dereference(parent->group_leader);
	while (walker->pid > 0) {
		if (!thread_group_leader(walker))
			walker = rcu_dereference(walker->group_leader);
		if (walker == parent) {
			rc = 1;
			break;
		}
		walker = rcu_dereference(walker->real_parent);
	}
	rcu_read_unlock();

	return rc;
}

/* 
                                                                           
                                                            
                                                        
  
                                                                    
 */
static int ptracer_exception_found(struct task_struct *tracer,
				   struct task_struct *tracee)
{
	int rc = 0;
	struct ptrace_relation *relation;
	struct task_struct *parent = NULL;
	bool found = false;

	rcu_read_lock();
	if (!thread_group_leader(tracee))
		tracee = rcu_dereference(tracee->group_leader);
	list_for_each_entry_rcu(relation, &ptracer_relations, node) {
		if (relation->invalid)
			continue;
		if (relation->tracee == tracee) {
			parent = relation->tracer;
			found = true;
			break;
		}
	}

	if (found && (parent == NULL || task_is_descendant(parent, tracer)))
		rc = 1;
	rcu_read_unlock();

	return rc;
}

/* 
                                                          
                                                         
                            
  
                                                              
 */
int yama_ptrace_access_check(struct task_struct *child,
				    unsigned int mode)
{
	int rc;

	/*                                                        
                                      
  */
	rc = cap_ptrace_access_check(child, mode);
	if (rc)
		return rc;

	/*                                                       */
	if (mode == PTRACE_MODE_ATTACH) {
		switch (ptrace_scope) {
		case YAMA_SCOPE_DISABLED:
			/*                             */
			break;
		case YAMA_SCOPE_RELATIONAL:
			rcu_read_lock();
			if (!task_is_descendant(current, child) &&
			    !ptracer_exception_found(current, child) &&
			    !ns_capable(__task_cred(child)->user_ns, CAP_SYS_PTRACE))
				rc = -EPERM;
			rcu_read_unlock();
			break;
		case YAMA_SCOPE_CAPABILITY:
			rcu_read_lock();
			if (!ns_capable(__task_cred(child)->user_ns, CAP_SYS_PTRACE))
				rc = -EPERM;
			rcu_read_unlock();
			break;
		case YAMA_SCOPE_NO_ATTACH:
		default:
			rc = -EPERM;
			break;
		}
	}

	if (rc) {
		printk_ratelimited(KERN_NOTICE
			"ptrace of pid %d was attempted by: %s (pid %d)\n",
			child->pid, current->comm, current->pid);
	}

	return rc;
}

/* 
                                                      
                                                                 
  
                                                              
 */
int yama_ptrace_traceme(struct task_struct *parent)
{
	int rc;

	/*                                                        
                                      
  */
	rc = cap_ptrace_traceme(parent);
	if (rc)
		return rc;

	/*                                                           */
	switch (ptrace_scope) {
	case YAMA_SCOPE_CAPABILITY:
		if (!has_ns_capability(parent, current_user_ns(), CAP_SYS_PTRACE))
			rc = -EPERM;
		break;
	case YAMA_SCOPE_NO_ATTACH:
		rc = -EPERM;
		break;
	}

	if (rc) {
		printk_ratelimited(KERN_NOTICE
			"ptraceme of pid %d was attempted by: %s (pid %d)\n",
			current->pid, parent->comm, parent->pid);
	}

	return rc;
}

#ifndef CONFIG_SECURITY_YAMA_STACKED
static struct security_operations yama_ops = {
	.name =			"yama",

	.ptrace_access_check =	yama_ptrace_access_check,
	.ptrace_traceme =	yama_ptrace_traceme,
	.task_prctl =		yama_task_prctl,
	.task_free =		yama_task_free,
};
#endif

#ifdef CONFIG_SYSCTL
static int yama_dointvec_minmax(struct ctl_table *table, int write,
				void __user *buffer, size_t *lenp, loff_t *ppos)
{
	int rc;

	if (write && !capable(CAP_SYS_PTRACE))
		return -EPERM;

	rc = proc_dointvec_minmax(table, write, buffer, lenp, ppos);
	if (rc)
		return rc;

	/*                                         */
	if (write && *(int *)table->data == *(int *)table->extra2)
		table->extra1 = table->extra2;

	return rc;
}

static int zero;
static int max_scope = YAMA_SCOPE_NO_ATTACH;

struct ctl_path yama_sysctl_path[] = {
	{ .procname = "kernel", },
	{ .procname = "yama", },
	{ }
};

static struct ctl_table yama_sysctl_table[] = {
	{
		.procname       = "ptrace_scope",
		.data           = &ptrace_scope,
		.maxlen         = sizeof(int),
		.mode           = 0644,
		.proc_handler   = yama_dointvec_minmax,
		.extra1         = &zero,
		.extra2         = &max_scope,
	},
	{ }
};
#endif /*               */

static __init int yama_init(void)
{
#ifndef CONFIG_SECURITY_YAMA_STACKED
	if (!security_module_enable(&yama_ops))
		return 0;
#endif

	printk(KERN_INFO "Yama: becoming mindful.\n");

#ifndef CONFIG_SECURITY_YAMA_STACKED
	if (register_security(&yama_ops))
		panic("Yama: kernel registration failed.\n");
#endif

#ifdef CONFIG_SYSCTL
	if (!register_sysctl_paths(yama_sysctl_path, yama_sysctl_table))
		panic("Yama: sysctl registration failed.\n");
#endif

	return 0;
}

security_initcall(yama_init);
