/*
 * Common Block IO controller cgroup interface
 *
 * Based on ideas and code from CFQ, CFS and BFQ:
 * Copyright (C) 2003 Jens Axboe <axboe@kernel.dk>
 *
 * Copyright (C) 2008 Fabio Checconi <fabio@gandalf.sssup.it>
 *		      Paolo Valente <paolo.valente@unimore.it>
 *
 * Copyright (C) 2009 Vivek Goyal <vgoyal@redhat.com>
 * 	              Nauman Rafique <nauman@google.com>
 */
#include <linux/ioprio.h>
#include <linux/kdev_t.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/blkdev.h>
#include <linux/slab.h>
#include <linux/genhd.h>
#include <linux/delay.h>
#include <linux/atomic.h>
#include "blk-cgroup.h"
#include "blk.h"

#define MAX_KEY_LEN 100

static DEFINE_MUTEX(blkcg_pol_mutex);

struct blkcg blkcg_root = { .cfq_weight = 2 * CFQ_WEIGHT_DEFAULT,
			    .cfq_leaf_weight = 2 * CFQ_WEIGHT_DEFAULT, };
EXPORT_SYMBOL_GPL(blkcg_root);

static struct blkcg_policy *blkcg_policy[BLKCG_MAX_POLS];

static struct blkcg_gq *__blkg_lookup(struct blkcg *blkcg,
				      struct request_queue *q, bool update_hint);

/* 
                                                                        
                                                          
                                
                                              
  
                                                                          
                                                                          
                                                                      
                                                                    
           
 */
#define blkg_for_each_descendant_pre(d_blkg, pos_cgrp, p_blkg)		\
	cgroup_for_each_descendant_pre((pos_cgrp), (p_blkg)->blkcg->css.cgroup) \
		if (((d_blkg) = __blkg_lookup(cgroup_to_blkcg(pos_cgrp), \
					      (p_blkg)->q, false)))

static bool blkcg_policy_enabled(struct request_queue *q,
				 const struct blkcg_policy *pol)
{
	return pol && test_bit(pol->plid, q->blkcg_pols);
}

/* 
                          
                      
  
                                               
 */
static void blkg_free(struct blkcg_gq *blkg)
{
	int i;

	if (!blkg)
		return;

	for (i = 0; i < BLKCG_MAX_POLS; i++) {
		struct blkcg_policy *pol = blkcg_policy[i];
		struct blkg_policy_data *pd = blkg->pd[i];

		if (!pd)
			continue;

		if (pol && pol->pd_exit_fn)
			pol->pd_exit_fn(blkg);

		kfree(pd);
	}

	blk_exit_rl(&blkg->rl);
	kfree(blkg);
}

/* 
                               
                                                       
                                                    
                                    
  
                                                
 */
static struct blkcg_gq *blkg_alloc(struct blkcg *blkcg, struct request_queue *q,
				   gfp_t gfp_mask)
{
	struct blkcg_gq *blkg;
	int i;

	/*                          */
	blkg = kzalloc_node(sizeof(*blkg), gfp_mask, q->node);
	if (!blkg)
		return NULL;

	blkg->q = q;
	INIT_LIST_HEAD(&blkg->q_node);
	blkg->blkcg = blkcg;
	blkg->refcnt = 1;

	/*                                                          */
	if (blkcg != &blkcg_root) {
		if (blk_init_rl(&blkg->rl, q, gfp_mask))
			goto err_free;
		blkg->rl.blkg = blkg;
	}

	for (i = 0; i < BLKCG_MAX_POLS; i++) {
		struct blkcg_policy *pol = blkcg_policy[i];
		struct blkg_policy_data *pd;

		if (!blkcg_policy_enabled(q, pol))
			continue;

		/*                                             */
		pd = kzalloc_node(pol->pd_size, gfp_mask, q->node);
		if (!pd)
			goto err_free;

		blkg->pd[i] = pd;
		pd->blkg = blkg;
		pd->plid = i;

		/*                        */
		if (pol->pd_init_fn)
			pol->pd_init_fn(blkg);
	}

	return blkg;

err_free:
	blkg_free(blkg);
	return NULL;
}

/* 
                                                    
                            
                                
                                                                     
  
                                                           
                                                                          
                                                                     
                                                                
 */
static struct blkcg_gq *__blkg_lookup(struct blkcg *blkcg,
				      struct request_queue *q, bool update_hint)
{
	struct blkcg_gq *blkg;

	blkg = rcu_dereference(blkcg->blkg_hint);
	if (blkg && blkg->q == q)
		return blkg;

	/*
                                                                   
                                                                
                                                                  
                                                        
  */
	blkg = radix_tree_lookup(&blkcg->blkg_tree, q->id);
	if (blkg && blkg->q == q) {
		if (update_hint) {
			lockdep_assert_held(q->queue_lock);
			rcu_assign_pointer(blkcg->blkg_hint, blkg);
		}
		return blkg;
	}

	return NULL;
}

/* 
                                                             
                            
                                
  
                                                                        
                                                                           
                                              
 */
struct blkcg_gq *blkg_lookup(struct blkcg *blkcg, struct request_queue *q)
{
	WARN_ON_ONCE(!rcu_read_lock_held());

	if (unlikely(blk_queue_bypass(q)))
		return NULL;
	return __blkg_lookup(blkcg, q, false);
}
EXPORT_SYMBOL_GPL(blkg_lookup);

/*
                                                                      
                                                                        
 */
static struct blkcg_gq *blkg_create(struct blkcg *blkcg,
				    struct request_queue *q,
				    struct blkcg_gq *new_blkg)
{
	struct blkcg_gq *blkg;
	int i, ret;

	WARN_ON_ONCE(!rcu_read_lock_held());
	lockdep_assert_held(q->queue_lock);

	/*                                 */
	if (!css_tryget(&blkcg->css)) {
		ret = -EINVAL;
		goto err_free_blkg;
	}

	/*          */
	if (!new_blkg) {
		new_blkg = blkg_alloc(blkcg, q, GFP_ATOMIC);
		if (unlikely(!new_blkg)) {
			ret = -ENOMEM;
			goto err_put_css;
		}
	}
	blkg = new_blkg;

	/*                        */
	if (blkcg_parent(blkcg)) {
		blkg->parent = __blkg_lookup(blkcg_parent(blkcg), q, false);
		if (WARN_ON_ONCE(!blkg->parent)) {
			blkg = ERR_PTR(-EINVAL);
			goto err_put_css;
		}
		blkg_get(blkg->parent);
	}

	spin_lock(&blkcg->lock);
	ret = radix_tree_insert(&blkcg->blkg_tree, q->id, blkg);
	if (likely(!ret)) {
		hlist_add_head_rcu(&blkg->blkcg_node, &blkcg->blkg_list);
		list_add(&blkg->q_node, &q->blkg_list);

		for (i = 0; i < BLKCG_MAX_POLS; i++) {
			struct blkcg_policy *pol = blkcg_policy[i];

			if (blkg->pd[i] && pol->pd_online_fn)
				pol->pd_online_fn(blkg);
		}
	}
	blkg->online = true;
	spin_unlock(&blkcg->lock);

	if (!ret)
		return blkg;

	/*                                                            */
	blkg_put(blkg);
	return ERR_PTR(ret);

err_put_css:
	css_put(&blkcg->css);
err_free_blkg:
	blkg_free(new_blkg);
	return ERR_PTR(ret);
}

/* 
                                                                   
                            
                                
  
                                                                     
                                                                           
                                                                          
                                                           
  
                                                                         
                                                                          
                                               
 */
struct blkcg_gq *blkg_lookup_create(struct blkcg *blkcg,
				    struct request_queue *q)
{
	struct blkcg_gq *blkg;

	WARN_ON_ONCE(!rcu_read_lock_held());
	lockdep_assert_held(q->queue_lock);

	/*
                                                                   
                                                                    
  */
	if (unlikely(blk_queue_bypass(q)))
		return ERR_PTR(blk_queue_dying(q) ? -EINVAL : -EBUSY);

	blkg = __blkg_lookup(blkcg, q, true);
	if (blkg)
		return blkg;

	/*
                                                                    
                                                
  */
	while (true) {
		struct blkcg *pos = blkcg;
		struct blkcg *parent = blkcg_parent(blkcg);

		while (parent && !__blkg_lookup(parent, q, false)) {
			pos = parent;
			parent = blkcg_parent(parent);
		}

		blkg = blkg_create(pos, q, NULL);
		if (pos == blkcg || IS_ERR(blkg))
			return blkg;
	}
}
EXPORT_SYMBOL_GPL(blkg_lookup_create);

static void blkg_destroy(struct blkcg_gq *blkg)
{
	struct blkcg *blkcg = blkg->blkcg;
	int i;

	lockdep_assert_held(blkg->q->queue_lock);
	lockdep_assert_held(&blkcg->lock);

	/*                                                             */
	WARN_ON_ONCE(list_empty(&blkg->q_node));
	WARN_ON_ONCE(hlist_unhashed(&blkg->blkcg_node));

	for (i = 0; i < BLKCG_MAX_POLS; i++) {
		struct blkcg_policy *pol = blkcg_policy[i];

		if (blkg->pd[i] && pol->pd_offline_fn)
			pol->pd_offline_fn(blkg);
	}
	blkg->online = false;

	radix_tree_delete(&blkcg->blkg_tree, blkg->q->id);
	list_del_init(&blkg->q_node);
	hlist_del_init_rcu(&blkg->blkcg_node);

	/*
                                                                   
                                                                  
                                                  
  */
	if (rcu_dereference_raw(blkcg->blkg_hint) == blkg)
		rcu_assign_pointer(blkcg->blkg_hint, NULL);

	/*
                                                                    
                                            
  */
	blkg_put(blkg);
}

/* 
                                                                       
                                
  
                                        
 */
static void blkg_destroy_all(struct request_queue *q)
{
	struct blkcg_gq *blkg, *n;

	lockdep_assert_held(q->queue_lock);

	list_for_each_entry_safe(blkg, n, &q->blkg_list, q_node) {
		struct blkcg *blkcg = blkg->blkcg;

		spin_lock(&blkcg->lock);
		blkg_destroy(blkg);
		spin_unlock(&blkcg->lock);
	}

	/*
                                                         
                                                 
  */
	q->root_blkg = NULL;
	q->root_rl.blkg = NULL;
}

static void blkg_rcu_free(struct rcu_head *rcu_head)
{
	blkg_free(container_of(rcu_head, struct blkcg_gq, rcu_head));
}

void __blkg_release(struct blkcg_gq *blkg)
{
	/*                                                                   */
	css_put(&blkg->blkcg->css);
	if (blkg->parent)
		blkg_put(blkg->parent);

	/*
                                                                   
                                                                    
                                                               
                        
   
                                                                
                                                                 
  */
	call_rcu(&blkg->rcu_head, blkg_rcu_free);
}
EXPORT_SYMBOL_GPL(__blkg_release);

/*
                                                                        
                                                                
 */
struct request_list *__blk_queue_next_rl(struct request_list *rl,
					 struct request_queue *q)
{
	struct list_head *ent;
	struct blkcg_gq *blkg;

	/*
                                                             
                                                              
  */
	if (rl == &q->root_rl) {
		ent = &q->blkg_list;
		/*                                                        */
		if (list_empty(ent))
			return NULL;
	} else {
		blkg = container_of(rl, struct blkcg_gq, rl);
		ent = &blkg->q_node;
	}

	/*                                             */
	ent = ent->next;
	if (ent == &q->root_blkg->q_node)
		ent = ent->next;
	if (ent == &q->blkg_list)
		return NULL;

	blkg = container_of(ent, struct blkcg_gq, q_node);
	return &blkg->rl;
}

static int blkcg_reset_stats(struct cgroup *cgroup, struct cftype *cftype,
			     u64 val)
{
	struct blkcg *blkcg = cgroup_to_blkcg(cgroup);
	struct blkcg_gq *blkg;
	int i;

	mutex_lock(&blkcg_pol_mutex);
	spin_lock_irq(&blkcg->lock);

	/*
                                                                 
                                                                
                                             
  */
	hlist_for_each_entry(blkg, &blkcg->blkg_list, blkcg_node) {
		for (i = 0; i < BLKCG_MAX_POLS; i++) {
			struct blkcg_policy *pol = blkcg_policy[i];

			if (blkcg_policy_enabled(blkg->q, pol) &&
			    pol->pd_reset_stats_fn)
				pol->pd_reset_stats_fn(blkg);
		}
	}

	spin_unlock_irq(&blkcg->lock);
	mutex_unlock(&blkcg_pol_mutex);
	return 0;
}

static const char *blkg_dev_name(struct blkcg_gq *blkg)
{
	/*                                                               */
	if (blkg->q->backing_dev_info.dev)
		return dev_name(blkg->q->backing_dev_info.dev);
	return NULL;
}

/* 
                                                        
                            
                            
                                             
                           
                                      
                                                               
  
                                                                     
                                                                     
                                                                          
                                                                      
                            
  
                                                      
                                  
 */
void blkcg_print_blkgs(struct seq_file *sf, struct blkcg *blkcg,
		       u64 (*prfill)(struct seq_file *,
				     struct blkg_policy_data *, int),
		       const struct blkcg_policy *pol, int data,
		       bool show_total)
{
	struct blkcg_gq *blkg;
	u64 total = 0;

	rcu_read_lock();
	hlist_for_each_entry_rcu(blkg, &blkcg->blkg_list, blkcg_node) {
		spin_lock_irq(blkg->q->queue_lock);
		if (blkcg_policy_enabled(blkg->q, pol))
			total += prfill(sf, blkg->pd[pol->plid], data);
		spin_unlock_irq(blkg->q->queue_lock);
	}
	rcu_read_unlock();

	if (show_total)
		seq_printf(sf, "Total %llu\n", (unsigned long long)total);
}
EXPORT_SYMBOL_GPL(blkcg_print_blkgs);

/* 
                                                           
                            
                                       
                     
  
                                                      
 */
u64 __blkg_prfill_u64(struct seq_file *sf, struct blkg_policy_data *pd, u64 v)
{
	const char *dname = blkg_dev_name(pd->blkg);

	if (!dname)
		return 0;

	seq_printf(sf, "%s %llu\n", dname, (unsigned long long)v);
	return v;
}
EXPORT_SYMBOL_GPL(__blkg_prfill_u64);

/* 
                                                         
                            
                                       
                           
  
                                                           
 */
u64 __blkg_prfill_rwstat(struct seq_file *sf, struct blkg_policy_data *pd,
			 const struct blkg_rwstat *rwstat)
{
	static const char *rwstr[] = {
		[BLKG_RWSTAT_READ]	= "Read",
		[BLKG_RWSTAT_WRITE]	= "Write",
		[BLKG_RWSTAT_SYNC]	= "Sync",
		[BLKG_RWSTAT_ASYNC]	= "Async",
	};
	const char *dname = blkg_dev_name(pd->blkg);
	u64 v;
	int i;

	if (!dname)
		return 0;

	for (i = 0; i < BLKG_RWSTAT_NR; i++)
		seq_printf(sf, "%s %s %llu\n", dname, rwstr[i],
			   (unsigned long long)rwstat->cnt[i]);

	v = rwstat->cnt[BLKG_RWSTAT_READ] + rwstat->cnt[BLKG_RWSTAT_WRITE];
	seq_printf(sf, "%s Total %llu\n", dname, (unsigned long long)v);
	return v;
}
EXPORT_SYMBOL_GPL(__blkg_prfill_rwstat);

/* 
                                                   
                            
                                       
                                       
  
                                            
 */
u64 blkg_prfill_stat(struct seq_file *sf, struct blkg_policy_data *pd, int off)
{
	return __blkg_prfill_u64(sf, pd, blkg_stat_read((void *)pd + off));
}
EXPORT_SYMBOL_GPL(blkg_prfill_stat);

/* 
                                                       
                            
                                       
                                         
  
                                              
 */
u64 blkg_prfill_rwstat(struct seq_file *sf, struct blkg_policy_data *pd,
		       int off)
{
	struct blkg_rwstat rwstat = blkg_rwstat_read((void *)pd + off);

	return __blkg_prfill_rwstat(sf, pd, &rwstat);
}
EXPORT_SYMBOL_GPL(blkg_prfill_rwstat);

/* 
                                                           
                                       
                                       
  
                                                                      
                                                                        
                         
 */
u64 blkg_stat_recursive_sum(struct blkg_policy_data *pd, int off)
{
	struct blkcg_policy *pol = blkcg_policy[pd->plid];
	struct blkcg_gq *pos_blkg;
	struct cgroup *pos_cgrp;
	u64 sum;

	lockdep_assert_held(pd->blkg->q->queue_lock);

	sum = blkg_stat_read((void *)pd + off);

	rcu_read_lock();
	blkg_for_each_descendant_pre(pos_blkg, pos_cgrp, pd_to_blkg(pd)) {
		struct blkg_policy_data *pos_pd = blkg_to_pd(pos_blkg, pol);
		struct blkg_stat *stat = (void *)pos_pd + off;

		if (pos_blkg->online)
			sum += blkg_stat_read(stat);
	}
	rcu_read_unlock();

	return sum;
}
EXPORT_SYMBOL_GPL(blkg_stat_recursive_sum);

/* 
                                                               
                                       
                                       
  
                                                                        
                                                                        
                         
 */
struct blkg_rwstat blkg_rwstat_recursive_sum(struct blkg_policy_data *pd,
					     int off)
{
	struct blkcg_policy *pol = blkcg_policy[pd->plid];
	struct blkcg_gq *pos_blkg;
	struct cgroup *pos_cgrp;
	struct blkg_rwstat sum;
	int i;

	lockdep_assert_held(pd->blkg->q->queue_lock);

	sum = blkg_rwstat_read((void *)pd + off);

	rcu_read_lock();
	blkg_for_each_descendant_pre(pos_blkg, pos_cgrp, pd_to_blkg(pd)) {
		struct blkg_policy_data *pos_pd = blkg_to_pd(pos_blkg, pol);
		struct blkg_rwstat *rwstat = (void *)pos_pd + off;
		struct blkg_rwstat tmp;

		if (!pos_blkg->online)
			continue;

		tmp = blkg_rwstat_read(rwstat);

		for (i = 0; i < BLKG_RWSTAT_NR; i++)
			sum.cnt[i] += tmp.cnt[i];
	}
	rcu_read_unlock();

	return sum;
}
EXPORT_SYMBOL_GPL(blkg_rwstat_recursive_sum);

/* 
                                                                
                              
                      
                       
                                   
  
                                                                        
                                                                           
                                                                           
                                          
 */
int blkg_conf_prep(struct blkcg *blkcg, const struct blkcg_policy *pol,
		   const char *input, struct blkg_conf_ctx *ctx)
	__acquires(rcu) __acquires(disk->queue->queue_lock)
{
	struct gendisk *disk;
	struct blkcg_gq *blkg;
	unsigned int major, minor;
	unsigned long long v;
	int part, ret;

	if (sscanf(input, "%u:%u %llu", &major, &minor, &v) != 3)
		return -EINVAL;

	disk = get_gendisk(MKDEV(major, minor), &part);
	if (!disk || part)
		return -EINVAL;

	rcu_read_lock();
	spin_lock_irq(disk->queue->queue_lock);

	if (blkcg_policy_enabled(disk->queue, pol))
		blkg = blkg_lookup_create(blkcg, disk->queue);
	else
		blkg = ERR_PTR(-EINVAL);

	if (IS_ERR(blkg)) {
		ret = PTR_ERR(blkg);
		rcu_read_unlock();
		spin_unlock_irq(disk->queue->queue_lock);
		put_disk(disk);
		/*
                                                            
                                                           
                                                           
                        
   */
		if (ret == -EBUSY) {
			msleep(10);
			ret = restart_syscall();
		}
		return ret;
	}

	ctx->disk = disk;
	ctx->blkg = blkg;
	ctx->v = v;
	return 0;
}
EXPORT_SYMBOL_GPL(blkg_conf_prep);

/* 
                                                      
                                                      
  
                                                                        
                         
 */
void blkg_conf_finish(struct blkg_conf_ctx *ctx)
	__releases(ctx->disk->queue->queue_lock) __releases(rcu)
{
	spin_unlock_irq(ctx->disk->queue->queue_lock);
	rcu_read_unlock();
	put_disk(ctx->disk);
}
EXPORT_SYMBOL_GPL(blkg_conf_finish);

struct cftype blkcg_files[] = {
	{
		.name = "reset_stats",
		.write_u64 = blkcg_reset_stats,
	},
	{ }	/*           */
};

/* 
                                                  
                              
  
                                                                           
                                                                        
                                                                         
                                                                     
  
                                                     
 */
static void blkcg_css_offline(struct cgroup *cgroup)
{
	struct blkcg *blkcg = cgroup_to_blkcg(cgroup);

	spin_lock_irq(&blkcg->lock);

	while (!hlist_empty(&blkcg->blkg_list)) {
		struct blkcg_gq *blkg = hlist_entry(blkcg->blkg_list.first,
						struct blkcg_gq, blkcg_node);
		struct request_queue *q = blkg->q;

		if (spin_trylock(q->queue_lock)) {
			blkg_destroy(blkg);
			spin_unlock(q->queue_lock);
		} else {
			spin_unlock_irq(&blkcg->lock);
			cpu_relax();
			spin_lock_irq(&blkcg->lock);
		}
	}

	spin_unlock_irq(&blkcg->lock);
}

static void blkcg_css_free(struct cgroup *cgroup)
{
	struct blkcg *blkcg = cgroup_to_blkcg(cgroup);

	if (blkcg != &blkcg_root)
		kfree(blkcg);
}

static struct cgroup_subsys_state *blkcg_css_alloc(struct cgroup *cgroup)
{
	static atomic64_t id_seq = ATOMIC64_INIT(0);
	struct blkcg *blkcg;
	struct cgroup *parent = cgroup->parent;

	if (!parent) {
		blkcg = &blkcg_root;
		goto done;
	}

	blkcg = kzalloc(sizeof(*blkcg), GFP_KERNEL);
	if (!blkcg)
		return ERR_PTR(-ENOMEM);

	blkcg->cfq_weight = CFQ_WEIGHT_DEFAULT;
	blkcg->cfq_leaf_weight = CFQ_WEIGHT_DEFAULT;
	blkcg->id = atomic64_inc_return(&id_seq); /*                         */
done:
	spin_lock_init(&blkcg->lock);
	INIT_RADIX_TREE(&blkcg->blkg_tree, GFP_ATOMIC);
	INIT_HLIST_HEAD(&blkcg->blkg_list);

	return &blkcg->css;
}

/* 
                                                            
                                  
  
                                                                         
                                
  
           
                                   
 */
int blkcg_init_queue(struct request_queue *q)
{
	might_sleep();

	return blk_throtl_init(q);
}

/* 
                                                        
                             
  
                                                                       
 */
void blkcg_drain_queue(struct request_queue *q)
{
	lockdep_assert_held(q->queue_lock);

	/*
                                                               
                                                                
  */
	if (!q->root_blkg)
		return;

	blk_throtl_drain(q);
}

/* 
                                                                  
                                   
  
                                                                        
 */
void blkcg_exit_queue(struct request_queue *q)
{
	spin_lock_irq(q->queue_lock);
	blkg_destroy_all(q);
	spin_unlock_irq(q->queue_lock);

	blk_throtl_exit(q);
}

/*
                                                                      
                                                                           
                                                                      
                                                     
 */
static int blkcg_can_attach(struct cgroup *cgrp, struct cgroup_taskset *tset)
{
	struct task_struct *task;
	struct io_context *ioc;
	int ret = 0;

	/*                                                             */
	cgroup_taskset_for_each(task, cgrp, tset) {
		task_lock(task);
		ioc = task->io_context;
		if (ioc && atomic_read(&ioc->nr_tasks) > 1)
			ret = -EINVAL;
		task_unlock(task);
		if (ret)
			break;
	}
	return ret;
}

struct cgroup_subsys blkio_subsys = {
	.name = "blkio",
	.css_alloc = blkcg_css_alloc,
	.css_offline = blkcg_css_offline,
	.css_free = blkcg_css_free,
	.can_attach = blkcg_can_attach,
	.subsys_id = blkio_subsys_id,
	.base_cftypes = blkcg_files,
	.module = THIS_MODULE,

	/*
                                                                    
                                                             
                                                                    
                                                           
  */
	.broken_hierarchy = true,
};
EXPORT_SYMBOL_GPL(blkio_subsys);

/* 
                                                                     
                                
                                 
  
                                                                       
                                                               
  
                                                                          
                                                                          
                                                                          
                                               
  
                                                                         
                                                                
 */
int blkcg_activate_policy(struct request_queue *q,
			  const struct blkcg_policy *pol)
{
	LIST_HEAD(pds);
	struct blkcg_gq *blkg, *new_blkg;
	struct blkg_policy_data *pd, *n;
	int cnt = 0, ret;
	bool preloaded;

	if (blkcg_policy_enabled(q, pol))
		return 0;

	/*                              */
	new_blkg = blkg_alloc(&blkcg_root, q, GFP_KERNEL);
	if (!new_blkg)
		return -ENOMEM;

	blk_queue_bypass_start(q);

	preloaded = !radix_tree_preload(GFP_KERNEL);

	/*
                                                                    
                                                                
                        
  */
	spin_lock_irq(q->queue_lock);

	rcu_read_lock();
	blkg = __blkg_lookup(&blkcg_root, q, false);
	if (blkg)
		blkg_free(new_blkg);
	else
		blkg = blkg_create(&blkcg_root, q, new_blkg);
	rcu_read_unlock();

	if (preloaded)
		radix_tree_preload_end();

	if (IS_ERR(blkg)) {
		ret = PTR_ERR(blkg);
		goto out_unlock;
	}
	q->root_blkg = blkg;
	q->root_rl.blkg = blkg;

	list_for_each_entry(blkg, &q->blkg_list, q_node)
		cnt++;

	spin_unlock_irq(q->queue_lock);

	/*                                             */
	while (cnt--) {
		pd = kzalloc_node(pol->pd_size, GFP_KERNEL, q->node);
		if (!pd) {
			ret = -ENOMEM;
			goto out_free;
		}
		list_add_tail(&pd->alloc_node, &pds);
	}

	/*
                                                              
                                                              
  */
	spin_lock_irq(q->queue_lock);

	list_for_each_entry(blkg, &q->blkg_list, q_node) {
		if (WARN_ON(list_empty(&pds))) {
			/*                                          */
			ret = -ENOMEM;
			goto out_unlock;
		}
		pd = list_first_entry(&pds, struct blkg_policy_data, alloc_node);
		list_del_init(&pd->alloc_node);

		/*                                                   */
		spin_lock(&blkg->blkcg->lock);

		blkg->pd[pol->plid] = pd;
		pd->blkg = blkg;
		pd->plid = pol->plid;
		pol->pd_init_fn(blkg);

		spin_unlock(&blkg->blkcg->lock);
	}

	__set_bit(pol->plid, q->blkcg_pols);
	ret = 0;
out_unlock:
	spin_unlock_irq(q->queue_lock);
out_free:
	blk_queue_bypass_end(q);
	list_for_each_entry_safe(pd, n, &pds, alloc_node)
		kfree(pd);
	return ret;
}
EXPORT_SYMBOL_GPL(blkcg_activate_policy);

/* 
                                                                         
                                
                                   
  
                                                                    
                           
 */
void blkcg_deactivate_policy(struct request_queue *q,
			     const struct blkcg_policy *pol)
{
	struct blkcg_gq *blkg;

	if (!blkcg_policy_enabled(q, pol))
		return;

	blk_queue_bypass_start(q);
	spin_lock_irq(q->queue_lock);

	__clear_bit(pol->plid, q->blkcg_pols);

	/*                                                           */
	if (bitmap_empty(q->blkcg_pols, BLKCG_MAX_POLS))
		blkg_destroy_all(q);

	list_for_each_entry(blkg, &q->blkg_list, q_node) {
		/*                                                   */
		spin_lock(&blkg->blkcg->lock);

		if (pol->pd_offline_fn)
			pol->pd_offline_fn(blkg);
		if (pol->pd_exit_fn)
			pol->pd_exit_fn(blkg);

		kfree(blkg->pd[pol->plid]);
		blkg->pd[pol->plid] = NULL;

		spin_unlock(&blkg->blkcg->lock);
	}

	spin_unlock_irq(q->queue_lock);
	blk_queue_bypass_end(q);
}
EXPORT_SYMBOL_GPL(blkcg_deactivate_policy);

/* 
                                                  
                                 
  
                                                                          
                                                                        
 */
int blkcg_policy_register(struct blkcg_policy *pol)
{
	int i, ret;

	if (WARN_ON(pol->pd_size < sizeof(struct blkg_policy_data)))
		return -EINVAL;

	mutex_lock(&blkcg_pol_mutex);

	/*                    */
	ret = -ENOSPC;
	for (i = 0; i < BLKCG_MAX_POLS; i++)
		if (!blkcg_policy[i])
			break;
	if (i >= BLKCG_MAX_POLS)
		goto out_unlock;

	/*                           */
	pol->plid = i;
	blkcg_policy[i] = pol;

	/*                                                           */
	if (pol->cftypes)
		WARN_ON(cgroup_add_cftypes(&blkio_subsys, pol->cftypes));
	ret = 0;
out_unlock:
	mutex_unlock(&blkcg_pol_mutex);
	return ret;
}
EXPORT_SYMBOL_GPL(blkcg_policy_register);

/* 
                                                      
                                   
  
                                                  
 */
void blkcg_policy_unregister(struct blkcg_policy *pol)
{
	mutex_lock(&blkcg_pol_mutex);

	if (WARN_ON(blkcg_policy[pol->plid] != pol))
		goto out_unlock;

	/*                           */
	if (pol->cftypes)
		cgroup_rm_cftypes(&blkio_subsys, pol->cftypes);

	/*                             */
	blkcg_policy[pol->plid] = NULL;
out_unlock:
	mutex_unlock(&blkcg_pol_mutex);
}
EXPORT_SYMBOL_GPL(blkcg_policy_unregister);
