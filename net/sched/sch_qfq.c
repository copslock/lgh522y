/*
 * net/sched/sch_qfq.c         Quick Fair Queueing Plus Scheduler.
 *
 * Copyright (c) 2009 Fabio Checconi, Luigi Rizzo, and Paolo Valente.
 * Copyright (c) 2012 Paolo Valente.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/bitops.h>
#include <linux/errno.h>
#include <linux/netdevice.h>
#include <linux/pkt_sched.h>
#include <net/sch_generic.h>
#include <net/pkt_sched.h>
#include <net/pkt_cls.h>


/*                          
                            

            

                      
                                                              
                                                                 

                    

                                                                       
                                                                    

             
                                           
 */

/*

                                                                 
                                                                    
                                                             
                                                                    
                                                                
                     

                                                                     
                                                                 
                                                                      
                                                               
                                                                      
                                                     

                            

                                                            
                         

                                                                 
                   
                                                                   

                                     

                                                     
                                                     
                       
                        

                                                                

                                                      
                                
                                                             
                                                      

                                  
                                      
                                        
                           
                                          
                                                            
                                 

                                                                 
                                              

 */

/*
                                                                     
                  
 */
#define QFQ_MAX_SLOTS	32

/*
                                                                              
                                                                            
                                                                          
                                    
  
                                                            
                                                       
 */
#define QFQ_MAX_INDEX		24
#define QFQ_MAX_WSHIFT		10

#define	QFQ_MAX_WEIGHT		(1<<QFQ_MAX_WSHIFT) /*                     */
#define QFQ_MAX_WSUM		(64*QFQ_MAX_WEIGHT)

#define FRAC_BITS		30	/*                        */
#define ONE_FP			(1UL << FRAC_BITS)

#define QFQ_MTU_SHIFT		16	/*                    */
#define QFQ_MIN_LMAX		512	/*                     */

#define QFQ_MAX_AGG_CLASSES	8 /*                                       */

/*
                                                                           
                             
 */
enum qfq_state { ER, IR, EB, IB, QFQ_MAX_STATE };

struct qfq_group;

struct qfq_aggregate;

struct qfq_class {
	struct Qdisc_class_common common;

	unsigned int refcnt;
	unsigned int filter_cnt;

	struct gnet_stats_basic_packed bstats;
	struct gnet_stats_queue qstats;
	struct gnet_stats_rate_est rate_est;
	struct Qdisc *qdisc;
	struct list_head alist;		/*                               */
	struct qfq_aggregate *agg;	/*                   */
	int deficit;			/*                      */
};

struct qfq_aggregate {
	struct hlist_node next;	/*                         */
	u64 S, F;		/*                         */

	/*                                                          
                                                          
                             
  */
	struct qfq_group *grp;

	/*                                    */
	u32	class_weight; /*                                         */
	/*                                                              */
	int	lmax;

	u32	inv_w;	    /*                                              */
	u32	budgetmax;  /*                                */
	u32	initial_budget, budget;     /*                             */

	int		  num_classes;	/*                                 */
	struct list_head  active;	/*                              */

	struct hlist_node nonfull_next;	/*                                */
};

struct qfq_group {
	u64 S, F;			/*                            */
	unsigned int slot_shift;	/*             */
	unsigned int index;		/*              */
	unsigned int front;		/*                          */
	unsigned long full_slots;	/*                 */

	/*                                         */
	struct hlist_head slots[QFQ_MAX_SLOTS];
};

struct qfq_sched {
	struct tcf_proto *filter_list;
	struct Qdisc_class_hash clhash;

	u64			oldV, V;	/*                        */
	struct qfq_aggregate	*in_serv_agg;   /*                         */
	u32			num_active_agg; /*                           */
	u32			wsum;		/*            */
	u32			iwsum;		/*                    */

	unsigned long bitmaps[QFQ_MAX_STATE];	    /*                */
	struct qfq_group groups[QFQ_MAX_INDEX + 1]; /*             */
	u32 min_slot_shift;	/*                                          */

	u32 max_agg_classes;		/*                                 */
	struct hlist_head nonfull_aggs; /*                                  */
};

/*
                                                                  
                                                                         
                  
                                                                           
                                      
 */
enum update_reason {enqueue, requeue};

static struct qfq_class *qfq_find_class(struct Qdisc *sch, u32 classid)
{
	struct qfq_sched *q = qdisc_priv(sch);
	struct Qdisc_class_common *clc;

	clc = qdisc_class_find(&q->clhash, classid);
	if (clc == NULL)
		return NULL;
	return container_of(clc, struct qfq_class, common);
}

static void qfq_purge_queue(struct qfq_class *cl)
{
	unsigned int len = cl->qdisc->q.qlen;

	qdisc_reset(cl->qdisc);
	qdisc_tree_decrease_qlen(cl->qdisc, len);
}

static const struct nla_policy qfq_policy[TCA_QFQ_MAX + 1] = {
	[TCA_QFQ_WEIGHT] = { .type = NLA_U32 },
	[TCA_QFQ_LMAX] = { .type = NLA_U32 },
};

/*
                                                                      
                                                                 
                                           
 */
static int qfq_calc_index(u32 inv_w, unsigned int maxlen, u32 min_slot_shift)
{
	u64 slot_size = (u64)maxlen * inv_w;
	unsigned long size_map;
	int index = 0;

	size_map = slot_size >> min_slot_shift;
	if (!size_map)
		goto out;

	index = __fls(size_map) + 1;	/*                   */
	index -= !(slot_size - (1ULL << (index + min_slot_shift - 1)));

	if (index < 0)
		index = 0;
out:
	pr_debug("qfq calc_index: W = %lu, L = %u, I = %d\n",
		 (unsigned long) ONE_FP/inv_w, maxlen, index);

	return index;
}

static void qfq_deactivate_agg(struct qfq_sched *, struct qfq_aggregate *);
static void qfq_activate_agg(struct qfq_sched *, struct qfq_aggregate *,
			     enum update_reason);

static void qfq_init_agg(struct qfq_sched *q, struct qfq_aggregate *agg,
			 u32 lmax, u32 weight)
{
	INIT_LIST_HEAD(&agg->active);
	hlist_add_head(&agg->nonfull_next, &q->nonfull_aggs);

	agg->lmax = lmax;
	agg->class_weight = weight;
}

static struct qfq_aggregate *qfq_find_agg(struct qfq_sched *q,
					  u32 lmax, u32 weight)
{
	struct qfq_aggregate *agg;

	hlist_for_each_entry(agg, &q->nonfull_aggs, nonfull_next)
		if (agg->lmax == lmax && agg->class_weight == weight)
			return agg;

	return NULL;
}


/*                                                              */
static void qfq_update_agg(struct qfq_sched *q, struct qfq_aggregate *agg,
			   int new_num_classes)
{
	u32 new_agg_weight;

	if (new_num_classes == q->max_agg_classes)
		hlist_del_init(&agg->nonfull_next);

	if (agg->num_classes > new_num_classes &&
	    new_num_classes == q->max_agg_classes - 1) /*                  */
		hlist_add_head(&agg->nonfull_next, &q->nonfull_aggs);

	/*                            
                                        
                                                                  
  */
	agg->budgetmax = new_num_classes * agg->lmax;
	new_agg_weight = agg->class_weight * new_num_classes;
	agg->inv_w = ONE_FP/new_agg_weight;

	if (agg->grp == NULL) {
		int i = qfq_calc_index(agg->inv_w, agg->budgetmax,
				       q->min_slot_shift);
		agg->grp = &q->groups[i];
	}

	q->wsum +=
		(int) agg->class_weight * (new_num_classes - agg->num_classes);
	q->iwsum = ONE_FP / q->wsum;

	agg->num_classes = new_num_classes;
}

/*                         */
static void qfq_add_to_agg(struct qfq_sched *q,
			   struct qfq_aggregate *agg,
			   struct qfq_class *cl)
{
	cl->agg = agg;

	qfq_update_agg(q, agg, agg->num_classes+1);
	if (cl->qdisc->q.qlen > 0) { /*                        */
		list_add_tail(&cl->alist, &agg->active);
		if (list_first_entry(&agg->active, struct qfq_class, alist) ==
		    cl && q->in_serv_agg != agg) /*                  */
			qfq_activate_agg(q, agg, enqueue); /*              */
	}
}

static struct qfq_aggregate *qfq_choose_next_agg(struct qfq_sched *);

static void qfq_destroy_agg(struct qfq_sched *q, struct qfq_aggregate *agg)
{
	if (!hlist_unhashed(&agg->nonfull_next))
		hlist_del_init(&agg->nonfull_next);
	q->wsum -= agg->class_weight;
	if (q->wsum != 0)
		q->iwsum = ONE_FP / q->wsum;

	if (q->in_serv_agg == agg)
		q->in_serv_agg = qfq_choose_next_agg(q);
	kfree(agg);
}

/*                                                    */
static void qfq_deactivate_class(struct qfq_sched *q, struct qfq_class *cl)
{
	struct qfq_aggregate *agg = cl->agg;


	list_del(&cl->alist); /*                                       */
	if (list_empty(&agg->active)) /*                     */
		qfq_deactivate_agg(q, agg);
}

/*                                         */
static void qfq_rm_from_agg(struct qfq_sched *q, struct qfq_class *cl)
{
	struct qfq_aggregate *agg = cl->agg;

	cl->agg = NULL;
	if (agg->num_classes == 1) { /*                               */
		qfq_destroy_agg(q, agg);
		return;
	}
	qfq_update_agg(q, agg, agg->num_classes-1);
}

/*                                                           */
static void qfq_deact_rm_from_agg(struct qfq_sched *q, struct qfq_class *cl)
{
	if (cl->qdisc->q.qlen > 0) /*                 */
		qfq_deactivate_class(q, cl);

	qfq_rm_from_agg(q, cl);
}

/*                                                                          */
static int qfq_change_agg(struct Qdisc *sch, struct qfq_class *cl, u32 weight,
			   u32 lmax)
{
	struct qfq_sched *q = qdisc_priv(sch);
	struct qfq_aggregate *new_agg = qfq_find_agg(q, lmax, weight);

	if (new_agg == NULL) { /*                      */
		new_agg = kzalloc(sizeof(*new_agg), GFP_ATOMIC);
		if (new_agg == NULL)
			return -ENOBUFS;
		qfq_init_agg(q, new_agg, lmax, weight);
	}
	qfq_deact_rm_from_agg(q, cl);
	qfq_add_to_agg(q, new_agg, cl);

	return 0;
}

static int qfq_change_class(struct Qdisc *sch, u32 classid, u32 parentid,
			    struct nlattr **tca, unsigned long *arg)
{
	struct qfq_sched *q = qdisc_priv(sch);
	struct qfq_class *cl = (struct qfq_class *)*arg;
	bool existing = false;
	struct nlattr *tb[TCA_QFQ_MAX + 1];
	struct qfq_aggregate *new_agg = NULL;
	u32 weight, lmax, inv_w;
	int err;
	int delta_w;

	if (tca[TCA_OPTIONS] == NULL) {
		pr_notice("qfq: no options\n");
		return -EINVAL;
	}

	err = nla_parse_nested(tb, TCA_QFQ_MAX, tca[TCA_OPTIONS], qfq_policy);
	if (err < 0)
		return err;

	if (tb[TCA_QFQ_WEIGHT]) {
		weight = nla_get_u32(tb[TCA_QFQ_WEIGHT]);
		if (!weight || weight > (1UL << QFQ_MAX_WSHIFT)) {
			pr_notice("qfq: invalid weight %u\n", weight);
			return -EINVAL;
		}
	} else
		weight = 1;

	if (tb[TCA_QFQ_LMAX]) {
		lmax = nla_get_u32(tb[TCA_QFQ_LMAX]);
		if (lmax < QFQ_MIN_LMAX || lmax > (1UL << QFQ_MTU_SHIFT)) {
			pr_notice("qfq: invalid max length %u\n", lmax);
			return -EINVAL;
		}
	} else
		lmax = psched_mtu(qdisc_dev(sch));

	inv_w = ONE_FP / weight;
	weight = ONE_FP / inv_w;

	if (cl != NULL &&
	    lmax == cl->agg->lmax &&
	    weight == cl->agg->class_weight)
		return 0; /*                   */

	delta_w = weight - (cl ? cl->agg->class_weight : 0);

	if (q->wsum + delta_w > QFQ_MAX_WSUM) {
		pr_notice("qfq: total weight out of range (%d + %u)\n",
			  delta_w, q->wsum);
		return -EINVAL;
	}

	if (cl != NULL) { /*                       */
		if (tca[TCA_RATE]) {
			err = gen_replace_estimator(&cl->bstats, &cl->rate_est,
						    qdisc_root_sleeping_lock(sch),
						    tca[TCA_RATE]);
			if (err)
				return err;
		}
		existing = true;
		goto set_change_agg;
	}

	/*                           */
	cl = kzalloc(sizeof(struct qfq_class), GFP_KERNEL);
	if (cl == NULL)
		return -ENOBUFS;

	cl->refcnt = 1;
	cl->common.classid = classid;
	cl->deficit = lmax;

	cl->qdisc = qdisc_create_dflt(sch->dev_queue,
				      &pfifo_qdisc_ops, classid);
	if (cl->qdisc == NULL)
		cl->qdisc = &noop_qdisc;

	if (tca[TCA_RATE]) {
		err = gen_new_estimator(&cl->bstats, &cl->rate_est,
					qdisc_root_sleeping_lock(sch),
					tca[TCA_RATE]);
		if (err)
			goto destroy_class;
	}

	sch_tree_lock(sch);
	qdisc_class_hash_insert(&q->clhash, &cl->common);
	sch_tree_unlock(sch);

	qdisc_class_hash_grow(sch, &q->clhash);

set_change_agg:
	sch_tree_lock(sch);
	new_agg = qfq_find_agg(q, lmax, weight);
	if (new_agg == NULL) { /*                      */
		sch_tree_unlock(sch);
		new_agg = kzalloc(sizeof(*new_agg), GFP_KERNEL);
		if (new_agg == NULL) {
			err = -ENOBUFS;
			gen_kill_estimator(&cl->bstats, &cl->rate_est);
			goto destroy_class;
		}
		sch_tree_lock(sch);
		qfq_init_agg(q, new_agg, lmax, weight);
	}
	if (existing)
		qfq_deact_rm_from_agg(q, cl);
	qfq_add_to_agg(q, new_agg, cl);
	sch_tree_unlock(sch);

	*arg = (unsigned long)cl;
	return 0;

destroy_class:
	qdisc_destroy(cl->qdisc);
	kfree(cl);
	return err;
}

static void qfq_destroy_class(struct Qdisc *sch, struct qfq_class *cl)
{
	struct qfq_sched *q = qdisc_priv(sch);

	qfq_rm_from_agg(q, cl);
	gen_kill_estimator(&cl->bstats, &cl->rate_est);
	qdisc_destroy(cl->qdisc);
	kfree(cl);
}

static int qfq_delete_class(struct Qdisc *sch, unsigned long arg)
{
	struct qfq_sched *q = qdisc_priv(sch);
	struct qfq_class *cl = (struct qfq_class *)arg;

	if (cl->filter_cnt > 0)
		return -EBUSY;

	sch_tree_lock(sch);

	qfq_purge_queue(cl);
	qdisc_class_hash_remove(&q->clhash, &cl->common);

	BUG_ON(--cl->refcnt == 0);
	/*
                                                                
                                                                    
  */

	sch_tree_unlock(sch);
	return 0;
}

static unsigned long qfq_get_class(struct Qdisc *sch, u32 classid)
{
	struct qfq_class *cl = qfq_find_class(sch, classid);

	if (cl != NULL)
		cl->refcnt++;

	return (unsigned long)cl;
}

static void qfq_put_class(struct Qdisc *sch, unsigned long arg)
{
	struct qfq_class *cl = (struct qfq_class *)arg;

	if (--cl->refcnt == 0)
		qfq_destroy_class(sch, cl);
}

static struct tcf_proto **qfq_tcf_chain(struct Qdisc *sch, unsigned long cl)
{
	struct qfq_sched *q = qdisc_priv(sch);

	if (cl)
		return NULL;

	return &q->filter_list;
}

static unsigned long qfq_bind_tcf(struct Qdisc *sch, unsigned long parent,
				  u32 classid)
{
	struct qfq_class *cl = qfq_find_class(sch, classid);

	if (cl != NULL)
		cl->filter_cnt++;

	return (unsigned long)cl;
}

static void qfq_unbind_tcf(struct Qdisc *sch, unsigned long arg)
{
	struct qfq_class *cl = (struct qfq_class *)arg;

	cl->filter_cnt--;
}

static int qfq_graft_class(struct Qdisc *sch, unsigned long arg,
			   struct Qdisc *new, struct Qdisc **old)
{
	struct qfq_class *cl = (struct qfq_class *)arg;

	if (new == NULL) {
		new = qdisc_create_dflt(sch->dev_queue,
					&pfifo_qdisc_ops, cl->common.classid);
		if (new == NULL)
			new = &noop_qdisc;
	}

	sch_tree_lock(sch);
	qfq_purge_queue(cl);
	*old = cl->qdisc;
	cl->qdisc = new;
	sch_tree_unlock(sch);
	return 0;
}

static struct Qdisc *qfq_class_leaf(struct Qdisc *sch, unsigned long arg)
{
	struct qfq_class *cl = (struct qfq_class *)arg;

	return cl->qdisc;
}

static int qfq_dump_class(struct Qdisc *sch, unsigned long arg,
			  struct sk_buff *skb, struct tcmsg *tcm)
{
	struct qfq_class *cl = (struct qfq_class *)arg;
	struct nlattr *nest;

	tcm->tcm_parent	= TC_H_ROOT;
	tcm->tcm_handle	= cl->common.classid;
	tcm->tcm_info	= cl->qdisc->handle;

	nest = nla_nest_start(skb, TCA_OPTIONS);
	if (nest == NULL)
		goto nla_put_failure;
	if (nla_put_u32(skb, TCA_QFQ_WEIGHT, cl->agg->class_weight) ||
	    nla_put_u32(skb, TCA_QFQ_LMAX, cl->agg->lmax))
		goto nla_put_failure;
	return nla_nest_end(skb, nest);

nla_put_failure:
	nla_nest_cancel(skb, nest);
	return -EMSGSIZE;
}

static int qfq_dump_class_stats(struct Qdisc *sch, unsigned long arg,
				struct gnet_dump *d)
{
	struct qfq_class *cl = (struct qfq_class *)arg;
	struct tc_qfq_stats xstats;

	memset(&xstats, 0, sizeof(xstats));
	cl->qdisc->qstats.qlen = cl->qdisc->q.qlen;

	xstats.weight = cl->agg->class_weight;
	xstats.lmax = cl->agg->lmax;

	if (gnet_stats_copy_basic(d, &cl->bstats) < 0 ||
	    gnet_stats_copy_rate_est(d, &cl->bstats, &cl->rate_est) < 0 ||
	    gnet_stats_copy_queue(d, &cl->qdisc->qstats) < 0)
		return -1;

	return gnet_stats_copy_app(d, &xstats, sizeof(xstats));
}

static void qfq_walk(struct Qdisc *sch, struct qdisc_walker *arg)
{
	struct qfq_sched *q = qdisc_priv(sch);
	struct qfq_class *cl;
	unsigned int i;

	if (arg->stop)
		return;

	for (i = 0; i < q->clhash.hashsize; i++) {
		hlist_for_each_entry(cl, &q->clhash.hash[i], common.hnode) {
			if (arg->count < arg->skip) {
				arg->count++;
				continue;
			}
			if (arg->fn(sch, (unsigned long)cl, arg) < 0) {
				arg->stop = 1;
				return;
			}
			arg->count++;
		}
	}
}

static struct qfq_class *qfq_classify(struct sk_buff *skb, struct Qdisc *sch,
				      int *qerr)
{
	struct qfq_sched *q = qdisc_priv(sch);
	struct qfq_class *cl;
	struct tcf_result res;
	int result;

	if (TC_H_MAJ(skb->priority ^ sch->handle) == 0) {
		pr_debug("qfq_classify: found %d\n", skb->priority);
		cl = qfq_find_class(sch, skb->priority);
		if (cl != NULL)
			return cl;
	}

	*qerr = NET_XMIT_SUCCESS | __NET_XMIT_BYPASS;
	result = tc_classify(skb, q->filter_list, &res);
	if (result >= 0) {
#ifdef CONFIG_NET_CLS_ACT
		switch (result) {
		case TC_ACT_QUEUED:
		case TC_ACT_STOLEN:
			*qerr = NET_XMIT_SUCCESS | __NET_XMIT_STOLEN;
		case TC_ACT_SHOT:
			return NULL;
		}
#endif
		cl = (struct qfq_class *)res.class;
		if (cl == NULL)
			cl = qfq_find_class(sch, res.classid);
		return cl;
	}

	return NULL;
}

/*                                                   */
static inline int qfq_gt(u64 a, u64 b)
{
	return (s64)(a - b) > 0;
}

/*                                                 */
static inline u64 qfq_round_down(u64 ts, unsigned int shift)
{
	return ts & ~((1ULL << shift) - 1);
}

/*                                                                 */
static inline struct qfq_group *qfq_ffs(struct qfq_sched *q,
					unsigned long bitmap)
{
	int index = __ffs(bitmap);
	return &q->groups[index];
}
/*                                                     */
static inline unsigned long mask_from(unsigned long bitmap, int from)
{
	return bitmap & ~((1UL << from) - 1);
}

/*
                                                         
                                                    
                                                           
 */
static int qfq_calc_state(struct qfq_sched *q, const struct qfq_group *grp)
{
	/*                              */
	unsigned int state = qfq_gt(grp->S, q->V);
	unsigned long mask = mask_from(q->bitmaps[ER], grp->index);
	struct qfq_group *next;

	if (mask) {
		next = qfq_ffs(q, mask);
		if (qfq_gt(grp->F, next->F))
			state |= EB;
	}

	return state;
}


/*
               
                                             
                            
                                          
 */
static inline void qfq_move_groups(struct qfq_sched *q, unsigned long mask,
				   int src, int dst)
{
	q->bitmaps[dst] |= q->bitmaps[src] & mask;
	q->bitmaps[src] &= ~mask;
}

static void qfq_unblock_groups(struct qfq_sched *q, int index, u64 old_F)
{
	unsigned long mask = mask_from(q->bitmaps[ER], index + 1);
	struct qfq_group *next;

	if (mask) {
		next = qfq_ffs(q, mask);
		if (!qfq_gt(next->F, old_F))
			return;
	}

	mask = (1UL << index) - 1;
	qfq_move_groups(q, mask, EB, ER);
	qfq_move_groups(q, mask, IB, IR);
}

/*
          
  
               
                             
             
     
  
  
 */
static void qfq_make_eligible(struct qfq_sched *q)
{
	unsigned long vslot = q->V >> q->min_slot_shift;
	unsigned long old_vslot = q->oldV >> q->min_slot_shift;

	if (vslot != old_vslot) {
		unsigned long mask = (1ULL << fls(vslot ^ old_vslot)) - 1;
		qfq_move_groups(q, mask, IR, ER);
		qfq_move_groups(q, mask, IB, EB);
	}
}

/*
                                                                  
                                                                    
                                                                  
                                                                  
                                                              
            
  
                                                                      
                                                                
                                                                  
                                                                      
                                                                      
                                                                    
                                                                    
                                                             
                                                                      
                                               
  
                                                             
                                                            
      
                                                            
                                                        
  
                                                                      
                                                                     
                                                                      
                                                                 
                                                                  
                                                                  
                                                                     
                                                                    
                                                                      
                                                                
                                                             
  
                                                                    
                                                                     
                                                                   
                                                                  
                                                                   
                                                                 
                                                                     
                                                                    
                                                                     
                                                                    
                                                                      
                               
  
                                                                    
                                                                  
                                                                     
              
 */
static void qfq_slot_insert(struct qfq_group *grp, struct qfq_aggregate *agg,
			    u64 roundedS)
{
	u64 slot = (roundedS - grp->S) >> grp->slot_shift;
	unsigned int i; /*                               */

	if (unlikely(slot > QFQ_MAX_SLOTS - 2)) {
		u64 deltaS = roundedS - grp->S -
			((u64)(QFQ_MAX_SLOTS - 2)<<grp->slot_shift);
		agg->S -= deltaS;
		agg->F -= deltaS;
		slot = QFQ_MAX_SLOTS - 2;
	}

	i = (grp->front + slot) % QFQ_MAX_SLOTS;

	hlist_add_head(&agg->next, &grp->slots[i]);
	__set_bit(slot, &grp->full_slots);
}

/*                                     */
static struct qfq_aggregate *qfq_slot_head(struct qfq_group *grp)
{
	return hlist_entry(grp->slots[grp->front].first,
			   struct qfq_aggregate, next);
}

/*
                                 
 */
static void qfq_front_slot_remove(struct qfq_group *grp)
{
	struct qfq_aggregate *agg = qfq_slot_head(grp);

	BUG_ON(!agg);
	hlist_del(&agg->next);
	if (hlist_empty(&grp->slots[grp->front]))
		__clear_bit(0, &grp->full_slots);
}

/*
                                                                   
                                                                
                                                   
 */
static struct qfq_aggregate *qfq_slot_scan(struct qfq_group *grp)
{
	unsigned int i;

	pr_debug("qfq slot_scan: grp %u full %#lx\n",
		 grp->index, grp->full_slots);

	if (grp->full_slots == 0)
		return NULL;

	i = __ffs(grp->full_slots);  /*            */
	if (i > 0) {
		grp->front = (grp->front + i) % QFQ_MAX_SLOTS;
		grp->full_slots >>= i;
	}

	return qfq_slot_head(grp);
}

/*
                                                                    
                                                                    
                                                               
                                                         
                                                                  
                                
                                                      
 */
static void qfq_slot_rotate(struct qfq_group *grp, u64 roundedS)
{
	unsigned int i = (grp->S - roundedS) >> grp->slot_shift;

	grp->full_slots <<= i;
	grp->front = (grp->front - i) % QFQ_MAX_SLOTS;
}

static void qfq_update_eligible(struct qfq_sched *q)
{
	struct qfq_group *grp;
	unsigned long ineligible;

	ineligible = q->bitmaps[IR] | q->bitmaps[IB];
	if (ineligible) {
		if (!q->bitmaps[ER]) {
			grp = qfq_ffs(q, ineligible);
			if (qfq_gt(grp->S, q->V))
				q->V = grp->S;
		}
		qfq_make_eligible(q);
	}
}

/*                                                                          */
static void agg_dequeue(struct qfq_aggregate *agg,
			struct qfq_class *cl, unsigned int len)
{
	qdisc_dequeue_peeked(cl->qdisc);

	cl->deficit -= (int) len;

	if (cl->qdisc->q.qlen == 0) /*                                   */
		list_del(&cl->alist);
	else if (cl->deficit < qdisc_pkt_len(cl->qdisc->ops->peek(cl->qdisc))) {
		cl->deficit += agg->lmax;
		list_move_tail(&cl->alist, &agg->active);
	}
}

static inline struct sk_buff *qfq_peek_skb(struct qfq_aggregate *agg,
					   struct qfq_class **cl,
					   unsigned int *len)
{
	struct sk_buff *skb;

	*cl = list_first_entry(&agg->active, struct qfq_class, alist);
	skb = (*cl)->qdisc->ops->peek((*cl)->qdisc);
	if (skb == NULL)
		WARN_ONCE(1, "qfq_dequeue: non-workconserving leaf\n");
	else
		*len = qdisc_pkt_len(skb);

	return skb;
}

/*                                                                     */
static inline void charge_actual_service(struct qfq_aggregate *agg)
{
	/*                                                           
                                                           
                           
                                                      
  */
	u32 service_received = min(agg->budgetmax,
				   agg->initial_budget - agg->budget);

	agg->F = agg->S + (u64)service_received * agg->inv_w;
}

static inline void qfq_update_agg_ts(struct qfq_sched *q,
				     struct qfq_aggregate *agg,
				     enum update_reason reason);

static void qfq_schedule_agg(struct qfq_sched *q, struct qfq_aggregate *agg);

static struct sk_buff *qfq_dequeue(struct Qdisc *sch)
{
	struct qfq_sched *q = qdisc_priv(sch);
	struct qfq_aggregate *in_serv_agg = q->in_serv_agg;
	struct qfq_class *cl;
	struct sk_buff *skb = NULL;
	/*                                                                   */
	unsigned int len = 0;

	if (in_serv_agg == NULL)
		return NULL;

	if (!list_empty(&in_serv_agg->active))
		skb = qfq_peek_skb(in_serv_agg, &cl, &len);

	/*
                                                               
                                                               
                                                   
  */
	if (len == 0 || in_serv_agg->budget < len) {
		charge_actual_service(in_serv_agg);

		/*                                      */
		in_serv_agg->initial_budget = in_serv_agg->budget =
			in_serv_agg->budgetmax;

		if (!list_empty(&in_serv_agg->active)) {
			/*
                                  
                                                 
                                                 
                                                
                                              
                                             
                                             
                                  
   */
			qfq_update_agg_ts(q, in_serv_agg, requeue);
			qfq_schedule_agg(q, in_serv_agg);
		} else if (sch->q.qlen == 0) { /*                       */
			q->in_serv_agg = NULL;
			return NULL;
		}

		/*
                                                       
                                       
   */
		in_serv_agg = q->in_serv_agg = qfq_choose_next_agg(q);
		skb = qfq_peek_skb(in_serv_agg, &cl, &len);
	}
	if (!skb)
		return NULL;

	sch->q.qlen--;
	qdisc_bstats_update(sch, skb);

	agg_dequeue(in_serv_agg, cl, len);
	/*                                                          
                                                              
                                                   
  */
	if (unlikely(in_serv_agg->budget < len))
		in_serv_agg->budget = 0;
	else
		in_serv_agg->budget -= len;

	q->V += (u64)len * q->iwsum;
	pr_debug("qfq dequeue: len %u F %lld now %lld\n",
		 len, (unsigned long long) in_serv_agg->F,
		 (unsigned long long) q->V);

	return skb;
}

static struct qfq_aggregate *qfq_choose_next_agg(struct qfq_sched *q)
{
	struct qfq_group *grp;
	struct qfq_aggregate *agg, *new_front_agg;
	u64 old_F;

	qfq_update_eligible(q);
	q->oldV = q->V;

	if (!q->bitmaps[ER])
		return NULL;

	grp = qfq_ffs(q, q->bitmaps[ER]);
	old_F = grp->F;

	agg = qfq_slot_head(grp);

	/*                                                  */
	qfq_front_slot_remove(grp);

	new_front_agg = qfq_slot_scan(grp);

	if (new_front_agg == NULL) /*                                       */
		__clear_bit(grp->index, &q->bitmaps[ER]);
	else {
		u64 roundedS = qfq_round_down(new_front_agg->S,
					      grp->slot_shift);
		unsigned int s;

		if (grp->S == roundedS)
			return agg;
		grp->S = roundedS;
		grp->F = roundedS + (2ULL << grp->slot_shift);
		__clear_bit(grp->index, &q->bitmaps[ER]);
		s = qfq_calc_state(q, grp);
		__set_bit(grp->index, &q->bitmaps[s]);
	}

	qfq_unblock_groups(q, grp->index, old_F);

	return agg;
}

/*
                                                                 
                                                          
                                                       
                                                                
  
                                                  
                                                         
                                                             
                                                                    
                                                   
                                                
 */
static void qfq_update_start(struct qfq_sched *q, struct qfq_aggregate *agg)
{
	unsigned long mask;
	u64 limit, roundedF;
	int slot_shift = agg->grp->slot_shift;

	roundedF = qfq_round_down(agg->F, slot_shift);
	limit = qfq_round_down(q->V, slot_shift) + (1ULL << slot_shift);

	if (!qfq_gt(agg->F, q->V) || qfq_gt(roundedF, limit)) {
		/*                     */
		mask = mask_from(q->bitmaps[ER], agg->grp->index);
		if (mask) {
			struct qfq_group *next = qfq_ffs(q, mask);
			if (qfq_gt(roundedF, next->F)) {
				if (qfq_gt(limit, next->F))
					agg->S = next->F;
				else /*                                */
					agg->S = limit;
				return;
			}
		}
		agg->S = q->V;
	} else  /*                        */
		agg->S = agg->F;
}

/*
                                                                     
                                                                 
                                                                
                                                               
 */
static inline void
qfq_update_agg_ts(struct qfq_sched *q,
		    struct qfq_aggregate *agg, enum update_reason reason)
{
	if (reason != requeue)
		qfq_update_start(q, agg);
	else /*                                          */
		agg->S = agg->F;

	agg->F = agg->S + (u64)agg->budgetmax * agg->inv_w;
}

static void qfq_schedule_agg(struct qfq_sched *, struct qfq_aggregate *);

static int qfq_enqueue(struct sk_buff *skb, struct Qdisc *sch)
{
	struct qfq_sched *q = qdisc_priv(sch);
	struct qfq_class *cl;
	struct qfq_aggregate *agg;
	int err = 0;

	cl = qfq_classify(skb, sch, &err);
	if (cl == NULL) {
		if (err & __NET_XMIT_BYPASS)
			sch->qstats.drops++;
		kfree_skb(skb);
		return err;
	}
	pr_debug("qfq_enqueue: cl = %x\n", cl->common.classid);

	if (unlikely(cl->agg->lmax < qdisc_pkt_len(skb))) {
		pr_debug("qfq: increasing maxpkt from %u to %u for class %u",
			 cl->agg->lmax, qdisc_pkt_len(skb), cl->common.classid);
		err = qfq_change_agg(sch, cl, cl->agg->class_weight,
				     qdisc_pkt_len(skb));
		if (err)
			return err;
	}

	err = qdisc_enqueue(skb, cl->qdisc);
	if (unlikely(err != NET_XMIT_SUCCESS)) {
		pr_debug("qfq_enqueue: enqueue failed %d\n", err);
		if (net_xmit_drop_count(err)) {
			cl->qstats.drops++;
			sch->qstats.drops++;
		}
		return err;
	}

	bstats_update(&cl->bstats, skb);
	++sch->q.qlen;

	agg = cl->agg;
	/*                                            */
	if (cl->qdisc->q.qlen != 1) {
		if (unlikely(skb == cl->qdisc->ops->peek(cl->qdisc)) &&
		    list_first_entry(&agg->active, struct qfq_class, alist)
		    == cl && cl->deficit < qdisc_pkt_len(skb))
			list_move_tail(&cl->alist, &agg->active);

		return err;
	}

	/*                                                 */
	cl->deficit = agg->lmax;
	list_add_tail(&cl->alist, &agg->active);

	if (list_first_entry(&agg->active, struct qfq_class, alist) != cl ||
	    q->in_serv_agg == agg)
		return err; /*                                             */

	qfq_activate_agg(q, agg, enqueue);

	return err;
}

/*
                                                  
 */
static void qfq_schedule_agg(struct qfq_sched *q, struct qfq_aggregate *agg)
{
	struct qfq_group *grp = agg->grp;
	u64 roundedS;
	int s;

	roundedS = qfq_round_down(agg->S, grp->slot_shift);

	/*
                                     
                                                   
                                                     
                                                     
                                                           
                                                            
                                    
  */
	if (grp->full_slots) {
		if (!qfq_gt(grp->S, agg->S))
			goto skip_update;

		/*                               */
		qfq_slot_rotate(grp, roundedS);
		/*                                     */
		__clear_bit(grp->index, &q->bitmaps[IR]);
		__clear_bit(grp->index, &q->bitmaps[IB]);
	} else if (!q->bitmaps[ER] && qfq_gt(roundedS, q->V) &&
		   q->in_serv_agg == NULL)
		q->V = roundedS;

	grp->S = roundedS;
	grp->F = roundedS + (2ULL << grp->slot_shift);
	s = qfq_calc_state(q, grp);
	__set_bit(grp->index, &q->bitmaps[s]);

	pr_debug("qfq enqueue: new state %d %#lx S %lld F %lld V %lld\n",
		 s, q->bitmaps[s],
		 (unsigned long long) agg->S,
		 (unsigned long long) agg->F,
		 (unsigned long long) q->V);

skip_update:
	qfq_slot_insert(grp, agg, roundedS);
}


/*                                            */
static void qfq_activate_agg(struct qfq_sched *q, struct qfq_aggregate *agg,
			     enum update_reason reason)
{
	agg->initial_budget = agg->budget = agg->budgetmax; /*                */

	qfq_update_agg_ts(q, agg, reason);
	if (q->in_serv_agg == NULL) { /*                                  */
		q->in_serv_agg = agg; /*                              */
		 /*                                                  */
		q->oldV = q->V = agg->S;
	} else if (agg != q->in_serv_agg)
		qfq_schedule_agg(q, agg);
}

static void qfq_slot_remove(struct qfq_sched *q, struct qfq_group *grp,
			    struct qfq_aggregate *agg)
{
	unsigned int i, offset;
	u64 roundedS;

	roundedS = qfq_round_down(agg->S, grp->slot_shift);
	offset = (roundedS - grp->S) >> grp->slot_shift;

	i = (grp->front + offset) % QFQ_MAX_SLOTS;

	hlist_del(&agg->next);
	if (hlist_empty(&grp->slots[i]))
		__clear_bit(offset, &grp->full_slots);
}

/*
                                                                   
                                                                    
                                                                     
                
                                            
 */
static void qfq_deactivate_agg(struct qfq_sched *q, struct qfq_aggregate *agg)
{
	struct qfq_group *grp = agg->grp;
	unsigned long mask;
	u64 roundedS;
	int s;

	if (agg == q->in_serv_agg) {
		charge_actual_service(agg);
		q->in_serv_agg = qfq_choose_next_agg(q);
		return;
	}

	agg->F = agg->S;
	qfq_slot_remove(q, grp, agg);

	if (!grp->full_slots) {
		__clear_bit(grp->index, &q->bitmaps[IR]);
		__clear_bit(grp->index, &q->bitmaps[EB]);
		__clear_bit(grp->index, &q->bitmaps[IB]);

		if (test_bit(grp->index, &q->bitmaps[ER]) &&
		    !(q->bitmaps[ER] & ~((1UL << grp->index) - 1))) {
			mask = q->bitmaps[ER] & ((1UL << grp->index) - 1);
			if (mask)
				mask = ~((1UL << __fls(mask)) - 1);
			else
				mask = ~0UL;
			qfq_move_groups(q, mask, EB, ER);
			qfq_move_groups(q, mask, IB, IR);
		}
		__clear_bit(grp->index, &q->bitmaps[ER]);
	} else if (hlist_empty(&grp->slots[grp->front])) {
		agg = qfq_slot_scan(grp);
		roundedS = qfq_round_down(agg->S, grp->slot_shift);
		if (grp->S != roundedS) {
			__clear_bit(grp->index, &q->bitmaps[ER]);
			__clear_bit(grp->index, &q->bitmaps[IR]);
			__clear_bit(grp->index, &q->bitmaps[EB]);
			__clear_bit(grp->index, &q->bitmaps[IB]);
			grp->S = roundedS;
			grp->F = roundedS + (2ULL << grp->slot_shift);
			s = qfq_calc_state(q, grp);
			__set_bit(grp->index, &q->bitmaps[s]);
		}
	}
}

static void qfq_qlen_notify(struct Qdisc *sch, unsigned long arg)
{
	struct qfq_sched *q = qdisc_priv(sch);
	struct qfq_class *cl = (struct qfq_class *)arg;

	if (cl->qdisc->q.qlen == 0)
		qfq_deactivate_class(q, cl);
}

static unsigned int qfq_drop_from_slot(struct qfq_sched *q,
				       struct hlist_head *slot)
{
	struct qfq_aggregate *agg;
	struct qfq_class *cl;
	unsigned int len;

	hlist_for_each_entry(agg, slot, next) {
		list_for_each_entry(cl, &agg->active, alist) {

			if (!cl->qdisc->ops->drop)
				continue;

			len = cl->qdisc->ops->drop(cl->qdisc);
			if (len > 0) {
				if (cl->qdisc->q.qlen == 0)
					qfq_deactivate_class(q, cl);

				return len;
			}
		}
	}
	return 0;
}

static unsigned int qfq_drop(struct Qdisc *sch)
{
	struct qfq_sched *q = qdisc_priv(sch);
	struct qfq_group *grp;
	unsigned int i, j, len;

	for (i = 0; i <= QFQ_MAX_INDEX; i++) {
		grp = &q->groups[i];
		for (j = 0; j < QFQ_MAX_SLOTS; j++) {
			len = qfq_drop_from_slot(q, &grp->slots[j]);
			if (len > 0) {
				sch->q.qlen--;
				return len;
			}
		}

	}

	return 0;
}

static int qfq_init_qdisc(struct Qdisc *sch, struct nlattr *opt)
{
	struct qfq_sched *q = qdisc_priv(sch);
	struct qfq_group *grp;
	int i, j, err;
	u32 max_cl_shift, maxbudg_shift, max_classes;

	err = qdisc_class_hash_init(&q->clhash);
	if (err < 0)
		return err;

	if (qdisc_dev(sch)->tx_queue_len + 1 > QFQ_MAX_AGG_CLASSES)
		max_classes = QFQ_MAX_AGG_CLASSES;
	else
		max_classes = qdisc_dev(sch)->tx_queue_len + 1;
	/*                                          */
	max_cl_shift = __fls(max_classes);
	q->max_agg_classes = 1<<max_cl_shift;

	/*                                                     */
	maxbudg_shift = QFQ_MTU_SHIFT + max_cl_shift;
	q->min_slot_shift = FRAC_BITS + maxbudg_shift - QFQ_MAX_INDEX;

	for (i = 0; i <= QFQ_MAX_INDEX; i++) {
		grp = &q->groups[i];
		grp->index = i;
		grp->slot_shift = q->min_slot_shift + i;
		for (j = 0; j < QFQ_MAX_SLOTS; j++)
			INIT_HLIST_HEAD(&grp->slots[j]);
	}

	INIT_HLIST_HEAD(&q->nonfull_aggs);

	return 0;
}

static void qfq_reset_qdisc(struct Qdisc *sch)
{
	struct qfq_sched *q = qdisc_priv(sch);
	struct qfq_class *cl;
	unsigned int i;

	for (i = 0; i < q->clhash.hashsize; i++) {
		hlist_for_each_entry(cl, &q->clhash.hash[i], common.hnode) {
			if (cl->qdisc->q.qlen > 0)
				qfq_deactivate_class(q, cl);

			qdisc_reset(cl->qdisc);
		}
	}
	sch->q.qlen = 0;
}

static void qfq_destroy_qdisc(struct Qdisc *sch)
{
	struct qfq_sched *q = qdisc_priv(sch);
	struct qfq_class *cl;
	struct hlist_node *next;
	unsigned int i;

	tcf_destroy_chain(&q->filter_list);

	for (i = 0; i < q->clhash.hashsize; i++) {
		hlist_for_each_entry_safe(cl, next, &q->clhash.hash[i],
					  common.hnode) {
			qfq_destroy_class(sch, cl);
		}
	}
	qdisc_class_hash_destroy(&q->clhash);
}

static const struct Qdisc_class_ops qfq_class_ops = {
	.change		= qfq_change_class,
	.delete		= qfq_delete_class,
	.get		= qfq_get_class,
	.put		= qfq_put_class,
	.tcf_chain	= qfq_tcf_chain,
	.bind_tcf	= qfq_bind_tcf,
	.unbind_tcf	= qfq_unbind_tcf,
	.graft		= qfq_graft_class,
	.leaf		= qfq_class_leaf,
	.qlen_notify	= qfq_qlen_notify,
	.dump		= qfq_dump_class,
	.dump_stats	= qfq_dump_class_stats,
	.walk		= qfq_walk,
};

static struct Qdisc_ops qfq_qdisc_ops __read_mostly = {
	.cl_ops		= &qfq_class_ops,
	.id		= "qfq",
	.priv_size	= sizeof(struct qfq_sched),
	.enqueue	= qfq_enqueue,
	.dequeue	= qfq_dequeue,
	.peek		= qdisc_peek_dequeued,
	.drop		= qfq_drop,
	.init		= qfq_init_qdisc,
	.reset		= qfq_reset_qdisc,
	.destroy	= qfq_destroy_qdisc,
	.owner		= THIS_MODULE,
};

static int __init qfq_init(void)
{
	return register_qdisc(&qfq_qdisc_ops);
}

static void __exit qfq_exit(void)
{
	unregister_qdisc(&qfq_qdisc_ops);
}

module_init(qfq_init);
module_exit(qfq_exit);
MODULE_LICENSE("GPL");
