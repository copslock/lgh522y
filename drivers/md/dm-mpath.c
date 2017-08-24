/*
 * Copyright (C) 2003 Sistina Software Limited.
 * Copyright (C) 2004-2005 Red Hat, Inc. All rights reserved.
 *
 * This file is released under the GPL.
 */

#include <linux/device-mapper.h>

#include "dm-path-selector.h"
#include "dm-uevent.h"

#include <linux/ctype.h>
#include <linux/init.h>
#include <linux/mempool.h>
#include <linux/module.h>
#include <linux/pagemap.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <scsi/scsi_dh.h>
#include <linux/atomic.h>

#define DM_MSG_PREFIX "multipath"
#define DM_PG_INIT_DELAY_MSECS 2000
#define DM_PG_INIT_DELAY_DEFAULT ((unsigned) -1)

/*                 */
struct pgpath {
	struct list_head list;

	struct priority_group *pg;	/*           */
	unsigned is_active;		/*             */
	unsigned fail_count;		/*                          */

	struct dm_path path;
	struct delayed_work activate_path;
};

#define path_to_pgpath(__pgp) container_of((__pgp), struct pgpath, path)

/*
                                                                      
                                                                
 */
struct priority_group {
	struct list_head list;

	struct multipath *m;		/*                           */
	struct path_selector ps;

	unsigned pg_num;		/*                  */
	unsigned bypassed;		/*                             */

	unsigned nr_pgpaths;		/*                       */
	struct list_head pgpaths;
};

/*                   */
struct multipath {
	struct list_head list;
	struct dm_target *ti;

	const char *hw_handler_name;
	char *hw_handler_params;

	spinlock_t lock;

	unsigned nr_priority_groups;
	struct list_head priority_groups;

	wait_queue_head_t pg_init_wait;	/*                             */

	unsigned pg_init_required;	/*                        */
	unsigned pg_init_in_progress;	/*                                  */
	unsigned pg_init_delay_retry;	/*                      */

	unsigned nr_valid_paths;	/*                              */
	struct pgpath *current_pgpath;
	struct priority_group *current_pg;
	struct priority_group *next_pg;	/*                          */
	unsigned repeat_count;		/*                                   */

	unsigned queue_io:1;		/*                        */
	unsigned queue_if_no_path:1;	/*                               */
	unsigned saved_queue_if_no_path:1; /*                               */
	unsigned retain_attached_hw_handler:1; /*                                                           */
	unsigned pg_init_disabled:1;	/*                                  */

	unsigned pg_init_retries;	/*                                  */
	unsigned pg_init_count;		/*                                */
	unsigned pg_init_delay_msecs;	/*                                      */

	unsigned queue_size;
	struct work_struct process_queued_ios;
	struct list_head queued_ios;

	struct work_struct trigger_event;

	/*
                                                           
                               
  */
	mempool_t *mpio_pool;

	struct mutex work_mutex;
};

/*
                                                       
 */
struct dm_mpath_io {
	struct pgpath *pgpath;
	size_t nr_bytes;
};

typedef int (*action_fn) (struct pgpath *pgpath);

#define MIN_IOS 256	/*              */

static struct kmem_cache *_mpio_cache;

static struct workqueue_struct *kmultipathd, *kmpath_handlerd;
static void process_queued_ios(struct work_struct *work);
static void trigger_event(struct work_struct *work);
static void activate_path(struct work_struct *work);


/*                                               
                      
                                                 */

static struct pgpath *alloc_pgpath(void)
{
	struct pgpath *pgpath = kzalloc(sizeof(*pgpath), GFP_KERNEL);

	if (pgpath) {
		pgpath->is_active = 1;
		INIT_DELAYED_WORK(&pgpath->activate_path, activate_path);
	}

	return pgpath;
}

static void free_pgpath(struct pgpath *pgpath)
{
	kfree(pgpath);
}

static struct priority_group *alloc_priority_group(void)
{
	struct priority_group *pg;

	pg = kzalloc(sizeof(*pg), GFP_KERNEL);

	if (pg)
		INIT_LIST_HEAD(&pg->pgpaths);

	return pg;
}

static void free_pgpaths(struct list_head *pgpaths, struct dm_target *ti)
{
	struct pgpath *pgpath, *tmp;
	struct multipath *m = ti->private;

	list_for_each_entry_safe(pgpath, tmp, pgpaths, list) {
		list_del(&pgpath->list);
		if (m->hw_handler_name)
			scsi_dh_detach(bdev_get_queue(pgpath->path.dev->bdev));
		dm_put_device(ti, pgpath->path.dev);
		free_pgpath(pgpath);
	}
}

static void free_priority_group(struct priority_group *pg,
				struct dm_target *ti)
{
	struct path_selector *ps = &pg->ps;

	if (ps->type) {
		ps->type->destroy(ps);
		dm_put_path_selector(ps->type);
	}

	free_pgpaths(&pg->pgpaths, ti);
	kfree(pg);
}

static struct multipath *alloc_multipath(struct dm_target *ti)
{
	struct multipath *m;

	m = kzalloc(sizeof(*m), GFP_KERNEL);
	if (m) {
		INIT_LIST_HEAD(&m->priority_groups);
		INIT_LIST_HEAD(&m->queued_ios);
		spin_lock_init(&m->lock);
		m->queue_io = 1;
		m->pg_init_delay_msecs = DM_PG_INIT_DELAY_DEFAULT;
		INIT_WORK(&m->process_queued_ios, process_queued_ios);
		INIT_WORK(&m->trigger_event, trigger_event);
		init_waitqueue_head(&m->pg_init_wait);
		mutex_init(&m->work_mutex);
		m->mpio_pool = mempool_create_slab_pool(MIN_IOS, _mpio_cache);
		if (!m->mpio_pool) {
			kfree(m);
			return NULL;
		}
		m->ti = ti;
		ti->private = m;
	}

	return m;
}

static void free_multipath(struct multipath *m)
{
	struct priority_group *pg, *tmp;

	list_for_each_entry_safe(pg, tmp, &m->priority_groups, list) {
		list_del(&pg->list);
		free_priority_group(pg, m->ti);
	}

	kfree(m->hw_handler_name);
	kfree(m->hw_handler_params);
	mempool_destroy(m->mpio_pool);
	kfree(m);
}

static int set_mapinfo(struct multipath *m, union map_info *info)
{
	struct dm_mpath_io *mpio;

	mpio = mempool_alloc(m->mpio_pool, GFP_ATOMIC);
	if (!mpio)
		return -ENOMEM;

	memset(mpio, 0, sizeof(*mpio));
	info->ptr = mpio;

	return 0;
}

static void clear_mapinfo(struct multipath *m, union map_info *info)
{
	struct dm_mpath_io *mpio = info->ptr;

	info->ptr = NULL;
	mempool_free(mpio, m->mpio_pool);
}

/*                                               
                 
                                                 */

static void __pg_init_all_paths(struct multipath *m)
{
	struct pgpath *pgpath;
	unsigned long pg_init_delay = 0;

	m->pg_init_count++;
	m->pg_init_required = 0;
	if (m->pg_init_delay_retry)
		pg_init_delay = msecs_to_jiffies(m->pg_init_delay_msecs != DM_PG_INIT_DELAY_DEFAULT ?
						 m->pg_init_delay_msecs : DM_PG_INIT_DELAY_MSECS);
	list_for_each_entry(pgpath, &m->current_pg->pgpaths, list) {
		/*                   */
		if (!pgpath->is_active)
			continue;
		if (queue_delayed_work(kmpath_handlerd, &pgpath->activate_path,
				       pg_init_delay))
			m->pg_init_in_progress++;
	}
}

static void __switch_pg(struct multipath *m, struct pgpath *pgpath)
{
	m->current_pg = pgpath->pg;

	/*                                                                 */
	if (m->hw_handler_name) {
		m->pg_init_required = 1;
		m->queue_io = 1;
	} else {
		m->pg_init_required = 0;
		m->queue_io = 0;
	}

	m->pg_init_count = 0;
}

static int __choose_path_in_pg(struct multipath *m, struct priority_group *pg,
			       size_t nr_bytes)
{
	struct dm_path *path;

	path = pg->ps.type->select_path(&pg->ps, &m->repeat_count, nr_bytes);
	if (!path)
		return -ENXIO;

	m->current_pgpath = path_to_pgpath(path);

	if (m->current_pg != pg)
		__switch_pg(m, m->current_pgpath);

	return 0;
}

static void __choose_pgpath(struct multipath *m, size_t nr_bytes)
{
	struct priority_group *pg;
	unsigned bypassed = 1;

	if (!m->nr_valid_paths)
		goto failed;

	/*                                  */
	if (m->next_pg) {
		pg = m->next_pg;
		m->next_pg = NULL;
		if (!__choose_path_in_pg(m, pg, nr_bytes))
			return;
	}

	/*                                                 */
	if (m->current_pg && !__choose_path_in_pg(m, m->current_pg, nr_bytes))
		return;

	/*
                                                            
                                             
                                                        
                                                        
  */
	do {
		list_for_each_entry(pg, &m->priority_groups, list) {
			if (pg->bypassed == bypassed)
				continue;
			if (!__choose_path_in_pg(m, pg, nr_bytes)) {
				if (!bypassed)
					m->pg_init_delay_retry = 1;
				return;
			}
		}
	} while (bypassed--);

failed:
	m->current_pgpath = NULL;
	m->current_pg = NULL;
}

/*
                                                                     
                           
  
                                 
  
                                                                
                                                            
                                                            
                                       
 */
static int __must_push_back(struct multipath *m)
{
	return (m->queue_if_no_path != m->saved_queue_if_no_path &&
		dm_noflush_suspending(m->ti));
}

static int map_io(struct multipath *m, struct request *clone,
		  union map_info *map_context, unsigned was_queued)
{
	int r = DM_MAPIO_REMAPPED;
	size_t nr_bytes = blk_rq_bytes(clone);
	unsigned long flags;
	struct pgpath *pgpath;
	struct block_device *bdev;
	struct dm_mpath_io *mpio = map_context->ptr;

	spin_lock_irqsave(&m->lock, flags);

	/*                                    */
	if (!m->current_pgpath ||
	    (!m->queue_io && (m->repeat_count && --m->repeat_count == 0)))
		__choose_pgpath(m, nr_bytes);

	pgpath = m->current_pgpath;

	if (was_queued)
		m->queue_size--;

	if ((pgpath && m->queue_io) ||
	    (!pgpath && m->queue_if_no_path)) {
		/*                                  */
		list_add_tail(&clone->queuelist, &m->queued_ios);
		m->queue_size++;
		if ((m->pg_init_required && !m->pg_init_in_progress) ||
		    !m->queue_io)
			queue_work(kmultipathd, &m->process_queued_ios);
		pgpath = NULL;
		r = DM_MAPIO_SUBMITTED;
	} else if (pgpath) {
		bdev = pgpath->path.dev->bdev;
		clone->q = bdev_get_queue(bdev);
		clone->rq_disk = bdev->bd_disk;
	} else if (__must_push_back(m))
		r = DM_MAPIO_REQUEUE;
	else
		r = -EIO;	/*        */

	mpio->pgpath = pgpath;
	mpio->nr_bytes = nr_bytes;

	if (r == DM_MAPIO_REMAPPED && pgpath->pg->ps.type->start_io)
		pgpath->pg->ps.type->start_io(&pgpath->pg->ps, &pgpath->path,
					      nr_bytes);

	spin_unlock_irqrestore(&m->lock, flags);

	return r;
}

/*
                                                                  
 */
static int queue_if_no_path(struct multipath *m, unsigned queue_if_no_path,
			    unsigned save_old_value)
{
	unsigned long flags;

	spin_lock_irqsave(&m->lock, flags);

	if (save_old_value)
		m->saved_queue_if_no_path = m->queue_if_no_path;
	else
		m->saved_queue_if_no_path = queue_if_no_path;
	m->queue_if_no_path = queue_if_no_path;
	if (!m->queue_if_no_path && m->queue_size)
		queue_work(kmultipathd, &m->process_queued_ios);

	spin_unlock_irqrestore(&m->lock, flags);

	return 0;
}

/*                                                                 
                                                                   
                                                                 */

static void dispatch_queued_ios(struct multipath *m)
{
	int r;
	unsigned long flags;
	union map_info *info;
	struct request *clone, *n;
	LIST_HEAD(cl);

	spin_lock_irqsave(&m->lock, flags);
	list_splice_init(&m->queued_ios, &cl);
	spin_unlock_irqrestore(&m->lock, flags);

	list_for_each_entry_safe(clone, n, &cl, queuelist) {
		list_del_init(&clone->queuelist);

		info = dm_get_rq_mapinfo(clone);

		r = map_io(m, clone, info, 1);
		if (r < 0) {
			clear_mapinfo(m, info);
			dm_kill_unmapped_request(clone, r);
		} else if (r == DM_MAPIO_REMAPPED)
			dm_dispatch_request(clone);
		else if (r == DM_MAPIO_REQUEUE) {
			clear_mapinfo(m, info);
			dm_requeue_unmapped_request(clone);
		}
	}
}

static void process_queued_ios(struct work_struct *work)
{
	struct multipath *m =
		container_of(work, struct multipath, process_queued_ios);
	struct pgpath *pgpath = NULL;
	unsigned must_queue = 1;
	unsigned long flags;

	spin_lock_irqsave(&m->lock, flags);

	if (!m->current_pgpath)
		__choose_pgpath(m, 0);

	pgpath = m->current_pgpath;

	if ((pgpath && !m->queue_io) ||
	    (!pgpath && !m->queue_if_no_path))
		must_queue = 0;

	if (m->pg_init_required && !m->pg_init_in_progress && pgpath &&
	    !m->pg_init_disabled)
		__pg_init_all_paths(m);

	spin_unlock_irqrestore(&m->lock, flags);
	if (!must_queue)
		dispatch_queued_ios(m);
}

/*
                                                             
                                       
 */
static void trigger_event(struct work_struct *work)
{
	struct multipath *m =
		container_of(work, struct multipath, trigger_event);

	dm_table_event(m->ti->table);
}

/*                                                                 
                                
                                     
                                           
                     
                           
                                            
                                          
                                 
                                                                 */
static int parse_path_selector(struct dm_arg_set *as, struct priority_group *pg,
			       struct dm_target *ti)
{
	int r;
	struct path_selector_type *pst;
	unsigned ps_argc;

	static struct dm_arg _args[] = {
		{0, 1024, "invalid number of path selector args"},
	};

	pst = dm_get_path_selector(dm_shift_arg(as));
	if (!pst) {
		ti->error = "unknown path selector type";
		return -EINVAL;
	}

	r = dm_read_arg_group(_args, as, &ps_argc, &ti->error);
	if (r) {
		dm_put_path_selector(pst);
		return -EINVAL;
	}

	r = pst->create(&pg->ps, ps_argc, as->argv);
	if (r) {
		dm_put_path_selector(pst);
		ti->error = "path selector constructor failed";
		return r;
	}

	pg->ps.type = pst;
	dm_consume_args(as, ps_argc);

	return 0;
}

static struct pgpath *parse_path(struct dm_arg_set *as, struct path_selector *ps,
			       struct dm_target *ti)
{
	int r;
	struct pgpath *p;
	struct multipath *m = ti->private;
	struct request_queue *q = NULL;
	const char *attached_handler_name;

	/*                             */
	if (as->argc < 1) {
		ti->error = "no device given";
		return ERR_PTR(-EINVAL);
	}

	p = alloc_pgpath();
	if (!p)
		return ERR_PTR(-ENOMEM);

	r = dm_get_device(ti, dm_shift_arg(as), dm_table_get_mode(ti->table),
			  &p->path.dev);
	if (r) {
		ti->error = "error getting device";
		goto bad;
	}

	if (m->retain_attached_hw_handler || m->hw_handler_name)
		q = bdev_get_queue(p->path.dev->bdev);

	if (m->retain_attached_hw_handler) {
		attached_handler_name = scsi_dh_attached_handler_name(q, GFP_KERNEL);
		if (attached_handler_name) {
			/*
                                                         
                                                         
                      
     
                                                         
                                                      
    */
			kfree(m->hw_handler_name);
			m->hw_handler_name = attached_handler_name;

			kfree(m->hw_handler_params);
			m->hw_handler_params = NULL;
		}
	}

	if (m->hw_handler_name) {
		/*
                                                     
                              
   */
		r = scsi_dh_attach(q, m->hw_handler_name);
		if (r == -EBUSY) {
			/*
                                               
                                       
    */
			scsi_dh_detach(q);
			r = scsi_dh_attach(q, m->hw_handler_name);
		}

		if (r < 0) {
			ti->error = "error attaching hardware handler";
			dm_put_device(ti, p->path.dev);
			goto bad;
		}

		if (m->hw_handler_params) {
			r = scsi_dh_set_params(q, m->hw_handler_params);
			if (r < 0) {
				ti->error = "unable to set hardware "
							"handler parameters";
				scsi_dh_detach(q);
				dm_put_device(ti, p->path.dev);
				goto bad;
			}
		}
	}

	r = ps->type->add_path(ps, &p->path, as->argc, as->argv, &ti->error);
	if (r) {
		dm_put_device(ti, p->path.dev);
		goto bad;
	}

	return p;

 bad:
	free_pgpath(p);
	return ERR_PTR(r);
}

static struct priority_group *parse_priority_group(struct dm_arg_set *as,
						   struct multipath *m)
{
	static struct dm_arg _args[] = {
		{1, 1024, "invalid number of paths"},
		{0, 1024, "invalid number of selector args"}
	};

	int r;
	unsigned i, nr_selector_args, nr_args;
	struct priority_group *pg;
	struct dm_target *ti = m->ti;

	if (as->argc < 2) {
		as->argc = 0;
		ti->error = "not enough priority group arguments";
		return ERR_PTR(-EINVAL);
	}

	pg = alloc_priority_group();
	if (!pg) {
		ti->error = "couldn't allocate priority group";
		return ERR_PTR(-ENOMEM);
	}
	pg->m = m;

	r = parse_path_selector(as, pg, ti);
	if (r)
		goto bad;

	/*
                  
  */
	r = dm_read_arg(_args, as, &pg->nr_pgpaths, &ti->error);
	if (r)
		goto bad;

	r = dm_read_arg(_args + 1, as, &nr_selector_args, &ti->error);
	if (r)
		goto bad;

	nr_args = 1 + nr_selector_args;
	for (i = 0; i < pg->nr_pgpaths; i++) {
		struct pgpath *pgpath;
		struct dm_arg_set path_args;

		if (as->argc < nr_args) {
			ti->error = "not enough path parameters";
			r = -EINVAL;
			goto bad;
		}

		path_args.argc = nr_args;
		path_args.argv = as->argv;

		pgpath = parse_path(&path_args, &pg->ps, ti);
		if (IS_ERR(pgpath)) {
			r = PTR_ERR(pgpath);
			goto bad;
		}

		pgpath->pg = pg;
		list_add_tail(&pgpath->list, &pg->pgpaths);
		dm_consume_args(as, nr_args);
	}

	return pg;

 bad:
	free_priority_group(pg, ti);
	return ERR_PTR(r);
}

static int parse_hw_handler(struct dm_arg_set *as, struct multipath *m)
{
	unsigned hw_argc;
	int ret;
	struct dm_target *ti = m->ti;

	static struct dm_arg _args[] = {
		{0, 1024, "invalid number of hardware handler args"},
	};

	if (dm_read_arg_group(_args, as, &hw_argc, &ti->error))
		return -EINVAL;

	if (!hw_argc)
		return 0;

	m->hw_handler_name = kstrdup(dm_shift_arg(as), GFP_KERNEL);
	if (!try_then_request_module(scsi_dh_handler_exist(m->hw_handler_name),
				     "scsi_dh_%s", m->hw_handler_name)) {
		ti->error = "unknown hardware handler type";
		ret = -EINVAL;
		goto fail;
	}

	if (hw_argc > 1) {
		char *p;
		int i, j, len = 4;

		for (i = 0; i <= hw_argc - 2; i++)
			len += strlen(as->argv[i]) + 1;
		p = m->hw_handler_params = kzalloc(len, GFP_KERNEL);
		if (!p) {
			ti->error = "memory allocation failed";
			ret = -ENOMEM;
			goto fail;
		}
		j = sprintf(p, "%d", hw_argc - 1);
		for (i = 0, p+=j+1; i <= hw_argc - 2; i++, p+=j+1)
			j = sprintf(p, "%s", as->argv[i]);
	}
	dm_consume_args(as, hw_argc - 1);

	return 0;
fail:
	kfree(m->hw_handler_name);
	m->hw_handler_name = NULL;
	return ret;
}

static int parse_features(struct dm_arg_set *as, struct multipath *m)
{
	int r;
	unsigned argc;
	struct dm_target *ti = m->ti;
	const char *arg_name;

	static struct dm_arg _args[] = {
		{0, 6, "invalid number of feature args"},
		{1, 50, "pg_init_retries must be between 1 and 50"},
		{0, 60000, "pg_init_delay_msecs must be between 0 and 60000"},
	};

	r = dm_read_arg_group(_args, as, &argc, &ti->error);
	if (r)
		return -EINVAL;

	if (!argc)
		return 0;

	do {
		arg_name = dm_shift_arg(as);
		argc--;

		if (!strcasecmp(arg_name, "queue_if_no_path")) {
			r = queue_if_no_path(m, 1, 0);
			continue;
		}

		if (!strcasecmp(arg_name, "retain_attached_hw_handler")) {
			m->retain_attached_hw_handler = 1;
			continue;
		}

		if (!strcasecmp(arg_name, "pg_init_retries") &&
		    (argc >= 1)) {
			r = dm_read_arg(_args + 1, as, &m->pg_init_retries, &ti->error);
			argc--;
			continue;
		}

		if (!strcasecmp(arg_name, "pg_init_delay_msecs") &&
		    (argc >= 1)) {
			r = dm_read_arg(_args + 2, as, &m->pg_init_delay_msecs, &ti->error);
			argc--;
			continue;
		}

		ti->error = "Unrecognised multipath feature request";
		r = -EINVAL;
	} while (argc && !r);

	return r;
}

static int multipath_ctr(struct dm_target *ti, unsigned int argc,
			 char **argv)
{
	/*                  */
	static struct dm_arg _args[] = {
		{0, 1024, "invalid number of priority groups"},
		{0, 1024, "invalid initial priority group number"},
	};

	int r;
	struct multipath *m;
	struct dm_arg_set as;
	unsigned pg_count = 0;
	unsigned next_pg_num;

	as.argc = argc;
	as.argv = argv;

	m = alloc_multipath(ti);
	if (!m) {
		ti->error = "can't allocate multipath";
		return -EINVAL;
	}

	r = parse_features(&as, m);
	if (r)
		goto bad;

	r = parse_hw_handler(&as, m);
	if (r)
		goto bad;

	r = dm_read_arg(_args, &as, &m->nr_priority_groups, &ti->error);
	if (r)
		goto bad;

	r = dm_read_arg(_args + 1, &as, &next_pg_num, &ti->error);
	if (r)
		goto bad;

	if ((!m->nr_priority_groups && next_pg_num) ||
	    (m->nr_priority_groups && !next_pg_num)) {
		ti->error = "invalid initial priority group";
		r = -EINVAL;
		goto bad;
	}

	/*                           */
	while (as.argc) {
		struct priority_group *pg;

		pg = parse_priority_group(&as, m);
		if (IS_ERR(pg)) {
			r = PTR_ERR(pg);
			goto bad;
		}

		m->nr_valid_paths += pg->nr_pgpaths;
		list_add_tail(&pg->list, &m->priority_groups);
		pg_count++;
		pg->pg_num = pg_count;
		if (!--next_pg_num)
			m->next_pg = pg;
	}

	if (pg_count != m->nr_priority_groups) {
		ti->error = "priority group count mismatch";
		r = -EINVAL;
		goto bad;
	}

	ti->num_flush_bios = 1;
	ti->num_discard_bios = 1;
	ti->num_write_same_bios = 1;

	return 0;

 bad:
	free_multipath(m);
	return r;
}

static void multipath_wait_for_pg_init_completion(struct multipath *m)
{
	DECLARE_WAITQUEUE(wait, current);
	unsigned long flags;

	add_wait_queue(&m->pg_init_wait, &wait);

	while (1) {
		set_current_state(TASK_UNINTERRUPTIBLE);

		spin_lock_irqsave(&m->lock, flags);
		if (!m->pg_init_in_progress) {
			spin_unlock_irqrestore(&m->lock, flags);
			break;
		}
		spin_unlock_irqrestore(&m->lock, flags);

		io_schedule();
	}
	set_current_state(TASK_RUNNING);

	remove_wait_queue(&m->pg_init_wait, &wait);
}

static void flush_multipath_work(struct multipath *m)
{
	unsigned long flags;

	spin_lock_irqsave(&m->lock, flags);
	m->pg_init_disabled = 1;
	spin_unlock_irqrestore(&m->lock, flags);

	flush_workqueue(kmpath_handlerd);
	multipath_wait_for_pg_init_completion(m);
	flush_workqueue(kmultipathd);
	flush_work(&m->trigger_event);

	spin_lock_irqsave(&m->lock, flags);
	m->pg_init_disabled = 0;
	spin_unlock_irqrestore(&m->lock, flags);
}

static void multipath_dtr(struct dm_target *ti)
{
	struct multipath *m = ti->private;

	flush_multipath_work(m);
	free_multipath(m);
}

/*
                      
 */
static int multipath_map(struct dm_target *ti, struct request *clone,
			 union map_info *map_context)
{
	int r;
	struct multipath *m = (struct multipath *) ti->private;

	if (set_mapinfo(m, map_context) < 0)
		/*                 */
		return DM_MAPIO_REQUEUE;

	clone->cmd_flags |= REQ_FAILFAST_TRANSPORT;
	r = map_io(m, clone, map_context, 0);
	if (r < 0 || r == DM_MAPIO_REQUEUE)
		clear_mapinfo(m, map_context);

	return r;
}

/*
                          
 */
static int fail_path(struct pgpath *pgpath)
{
	unsigned long flags;
	struct multipath *m = pgpath->pg->m;

	spin_lock_irqsave(&m->lock, flags);

	if (!pgpath->is_active)
		goto out;

	DMWARN("Failing path %s.", pgpath->path.dev->name);

	pgpath->pg->ps.type->fail_path(&pgpath->pg->ps, &pgpath->path);
	pgpath->is_active = 0;
	pgpath->fail_count++;

	m->nr_valid_paths--;

	if (pgpath == m->current_pgpath)
		m->current_pgpath = NULL;

	dm_path_uevent(DM_UEVENT_PATH_FAILED, m->ti,
		      pgpath->path.dev->name, m->nr_valid_paths);

	schedule_work(&m->trigger_event);

out:
	spin_unlock_irqrestore(&m->lock, flags);

	return 0;
}

/*
                                     
 */
static int reinstate_path(struct pgpath *pgpath)
{
	int r = 0;
	unsigned long flags;
	struct multipath *m = pgpath->pg->m;

	spin_lock_irqsave(&m->lock, flags);

	if (pgpath->is_active)
		goto out;

	if (!pgpath->pg->ps.type->reinstate_path) {
		DMWARN("Reinstate path not supported by path selector %s",
		       pgpath->pg->ps.type->name);
		r = -EINVAL;
		goto out;
	}

	r = pgpath->pg->ps.type->reinstate_path(&pgpath->pg->ps, &pgpath->path);
	if (r)
		goto out;

	pgpath->is_active = 1;

	if (!m->nr_valid_paths++ && m->queue_size) {
		m->current_pgpath = NULL;
		queue_work(kmultipathd, &m->process_queued_ios);
	} else if (m->hw_handler_name && (m->current_pg == pgpath->pg)) {
		if (queue_work(kmpath_handlerd, &pgpath->activate_path.work))
			m->pg_init_in_progress++;
	}

	dm_path_uevent(DM_UEVENT_PATH_REINSTATED, m->ti,
		      pgpath->path.dev->name, m->nr_valid_paths);

	schedule_work(&m->trigger_event);

out:
	spin_unlock_irqrestore(&m->lock, flags);

	return r;
}

/*
                                                                     
 */
static int action_dev(struct multipath *m, struct dm_dev *dev,
		      action_fn action)
{
	int r = -EINVAL;
	struct pgpath *pgpath;
	struct priority_group *pg;

	list_for_each_entry(pg, &m->priority_groups, list) {
		list_for_each_entry(pgpath, &pg->pgpaths, list) {
			if (pgpath->path.dev == dev)
				r = action(pgpath);
		}
	}

	return r;
}

/*
                                                          
 */
static void bypass_pg(struct multipath *m, struct priority_group *pg,
		      int bypassed)
{
	unsigned long flags;

	spin_lock_irqsave(&m->lock, flags);

	pg->bypassed = bypassed;
	m->current_pgpath = NULL;
	m->current_pg = NULL;

	spin_unlock_irqrestore(&m->lock, flags);

	schedule_work(&m->trigger_event);
}

/*
                                                                      
 */
static int switch_pg_num(struct multipath *m, const char *pgstr)
{
	struct priority_group *pg;
	unsigned pgnum;
	unsigned long flags;
	char dummy;

	if (!pgstr || (sscanf(pgstr, "%u%c", &pgnum, &dummy) != 1) || !pgnum ||
	    (pgnum > m->nr_priority_groups)) {
		DMWARN("invalid PG number supplied to switch_pg_num");
		return -EINVAL;
	}

	spin_lock_irqsave(&m->lock, flags);
	list_for_each_entry(pg, &m->priority_groups, list) {
		pg->bypassed = 0;
		if (--pgnum)
			continue;

		m->current_pgpath = NULL;
		m->current_pg = NULL;
		m->next_pg = pg;
	}
	spin_unlock_irqrestore(&m->lock, flags);

	schedule_work(&m->trigger_event);
	return 0;
}

/*
                                     
                                                                   
 */
static int bypass_pg_num(struct multipath *m, const char *pgstr, int bypassed)
{
	struct priority_group *pg;
	unsigned pgnum;
	char dummy;

	if (!pgstr || (sscanf(pgstr, "%u%c", &pgnum, &dummy) != 1) || !pgnum ||
	    (pgnum > m->nr_priority_groups)) {
		DMWARN("invalid PG number supplied to bypass_pg");
		return -EINVAL;
	}

	list_for_each_entry(pg, &m->priority_groups, list) {
		if (!--pgnum)
			break;
	}

	bypass_pg(m, pg, bypassed);
	return 0;
}

/*
                                       
 */
static int pg_init_limit_reached(struct multipath *m, struct pgpath *pgpath)
{
	unsigned long flags;
	int limit_reached = 0;

	spin_lock_irqsave(&m->lock, flags);

	if (m->pg_init_count <= m->pg_init_retries && !m->pg_init_disabled)
		m->pg_init_required = 1;
	else
		limit_reached = 1;

	spin_unlock_irqrestore(&m->lock, flags);

	return limit_reached;
}

static void pg_init_done(void *data, int errors)
{
	struct pgpath *pgpath = data;
	struct priority_group *pg = pgpath->pg;
	struct multipath *m = pg->m;
	unsigned long flags;
	unsigned delay_retry = 0;

	/*                           */
	switch (errors) {
	case SCSI_DH_OK:
		break;
	case SCSI_DH_NOSYS:
		if (!m->hw_handler_name) {
			errors = 0;
			break;
		}
		DMERR("Could not failover the device: Handler scsi_dh_%s "
		      "Error %d.", m->hw_handler_name, errors);
		/*
                                              
   */
		fail_path(pgpath);
		break;
	case SCSI_DH_DEV_TEMP_BUSY:
		/*
                                                    
                                    
   */
		bypass_pg(m, pg, 1);
		break;
	case SCSI_DH_RETRY:
		/*                       */
		delay_retry = 1;
	case SCSI_DH_IMM_RETRY:
	case SCSI_DH_RES_TEMP_UNAVAIL:
		if (pg_init_limit_reached(m, pgpath))
			fail_path(pgpath);
		errors = 0;
		break;
	default:
		/*
                                                          
                                                      
                                              
   */
		fail_path(pgpath);
	}

	spin_lock_irqsave(&m->lock, flags);
	if (errors) {
		if (pgpath == m->current_pgpath) {
			DMERR("Could not failover device. Error %d.", errors);
			m->current_pgpath = NULL;
			m->current_pg = NULL;
		}
	} else if (!m->pg_init_required)
		pg->bypassed = 0;

	if (--m->pg_init_in_progress)
		/*                                               */
		goto out;

	if (!m->pg_init_required)
		m->queue_io = 0;

	m->pg_init_delay_retry = delay_retry;
	queue_work(kmultipathd, &m->process_queued_ios);

	/*
                                          
  */
	wake_up(&m->pg_init_wait);

out:
	spin_unlock_irqrestore(&m->lock, flags);
}

static void activate_path(struct work_struct *work)
{
	struct pgpath *pgpath =
		container_of(work, struct pgpath, activate_path.work);

	scsi_dh_activate(bdev_get_queue(pgpath->path.dev->bdev),
				pg_init_done, pgpath);
}

/*
                  
 */
static int do_end_io(struct multipath *m, struct request *clone,
		     int error, struct dm_mpath_io *mpio)
{
	/*
                                                                
                                                                  
                                                              
                                                                
                                                                     
                            
                                                                     
                                                               
                                            
  */
	int r = DM_ENDIO_REQUEUE;
	unsigned long flags;

	if (!error && !clone->errors)
		return 0;	/*              */

	if (error == -EOPNOTSUPP || error == -EREMOTEIO || error == -EILSEQ) {
		if ((clone->cmd_flags & REQ_WRITE_SAME) &&
		    !clone->q->limits.max_write_same_sectors) {
			struct queue_limits *limits;

			/*                                                      */
			limits = dm_get_queue_limits(dm_table_get_md(m->ti->table));
			limits->max_write_same_sectors = 0;
		}
		return error;
	}

	if (mpio->pgpath)
		fail_path(mpio->pgpath);

	spin_lock_irqsave(&m->lock, flags);
	if (!m->nr_valid_paths) {
		if (!m->queue_if_no_path) {
			if (!__must_push_back(m))
				r = -EIO;
		} else {
			if (error == -EBADE)
				r = error;
		}
	}
	spin_unlock_irqrestore(&m->lock, flags);

	return r;
}

static int multipath_end_io(struct dm_target *ti, struct request *clone,
			    int error, union map_info *map_context)
{
	struct multipath *m = ti->private;
	struct dm_mpath_io *mpio = map_context->ptr;
	struct pgpath *pgpath;
	struct path_selector *ps;
	int r;

	BUG_ON(!mpio);

	r  = do_end_io(m, clone, error, mpio);
	pgpath = mpio->pgpath;
	if (pgpath) {
		ps = &pgpath->pg->ps;
		if (ps->type->end_io)
			ps->type->end_io(ps, &pgpath->path, mpio->nr_bytes);
	}
	clear_mapinfo(m, map_context);

	return r;
}

/*
                                                              
                                                       
                                                           
                                                              
 */
static void multipath_presuspend(struct dm_target *ti)
{
	struct multipath *m = (struct multipath *) ti->private;

	queue_if_no_path(m, 0, 1);
}

static void multipath_postsuspend(struct dm_target *ti)
{
	struct multipath *m = ti->private;

	mutex_lock(&m->work_mutex);
	flush_multipath_work(m);
	mutex_unlock(&m->work_mutex);
}

/*
                                        
 */
static void multipath_resume(struct dm_target *ti)
{
	struct multipath *m = (struct multipath *) ti->private;
	unsigned long flags;

	spin_lock_irqsave(&m->lock, flags);
	m->queue_if_no_path = m->saved_queue_if_no_path;
	spin_unlock_irqrestore(&m->lock, flags);
}

/*
                                        
                                                       
                                                 
                               
                                                         
                                          
                                                              
  
                                                                               
                                    
                                                 
                               
                                                     
                                                                    
 */
static void multipath_status(struct dm_target *ti, status_type_t type,
			     unsigned status_flags, char *result, unsigned maxlen)
{
	int sz = 0;
	unsigned long flags;
	struct multipath *m = (struct multipath *) ti->private;
	struct priority_group *pg;
	struct pgpath *p;
	unsigned pg_num;
	char state;

	spin_lock_irqsave(&m->lock, flags);

	/*          */
	if (type == STATUSTYPE_INFO)
		DMEMIT("2 %u %u ", m->queue_size, m->pg_init_count);
	else {
		DMEMIT("%u ", m->queue_if_no_path +
			      (m->pg_init_retries > 0) * 2 +
			      (m->pg_init_delay_msecs != DM_PG_INIT_DELAY_DEFAULT) * 2 +
			      m->retain_attached_hw_handler);
		if (m->queue_if_no_path)
			DMEMIT("queue_if_no_path ");
		if (m->pg_init_retries)
			DMEMIT("pg_init_retries %u ", m->pg_init_retries);
		if (m->pg_init_delay_msecs != DM_PG_INIT_DELAY_DEFAULT)
			DMEMIT("pg_init_delay_msecs %u ", m->pg_init_delay_msecs);
		if (m->retain_attached_hw_handler)
			DMEMIT("retain_attached_hw_handler ");
	}

	if (!m->hw_handler_name || type == STATUSTYPE_INFO)
		DMEMIT("0 ");
	else
		DMEMIT("1 %s ", m->hw_handler_name);

	DMEMIT("%u ", m->nr_priority_groups);

	if (m->next_pg)
		pg_num = m->next_pg->pg_num;
	else if (m->current_pg)
		pg_num = m->current_pg->pg_num;
	else
		pg_num = (m->nr_priority_groups ? 1 : 0);

	DMEMIT("%u ", pg_num);

	switch (type) {
	case STATUSTYPE_INFO:
		list_for_each_entry(pg, &m->priority_groups, list) {
			if (pg->bypassed)
				state = 'D';	/*          */
			else if (pg == m->current_pg)
				state = 'A';	/*                  */
			else
				state = 'E';	/*         */

			DMEMIT("%c ", state);

			if (pg->ps.type->status)
				sz += pg->ps.type->status(&pg->ps, NULL, type,
							  result + sz,
							  maxlen - sz);
			else
				DMEMIT("0 ");

			DMEMIT("%u %u ", pg->nr_pgpaths,
			       pg->ps.type->info_args);

			list_for_each_entry(p, &pg->pgpaths, list) {
				DMEMIT("%s %s %u ", p->path.dev->name,
				       p->is_active ? "A" : "F",
				       p->fail_count);
				if (pg->ps.type->status)
					sz += pg->ps.type->status(&pg->ps,
					      &p->path, type, result + sz,
					      maxlen - sz);
			}
		}
		break;

	case STATUSTYPE_TABLE:
		list_for_each_entry(pg, &m->priority_groups, list) {
			DMEMIT("%s ", pg->ps.type->name);

			if (pg->ps.type->status)
				sz += pg->ps.type->status(&pg->ps, NULL, type,
							  result + sz,
							  maxlen - sz);
			else
				DMEMIT("0 ");

			DMEMIT("%u %u ", pg->nr_pgpaths,
			       pg->ps.type->table_args);

			list_for_each_entry(p, &pg->pgpaths, list) {
				DMEMIT("%s ", p->path.dev->name);
				if (pg->ps.type->status)
					sz += pg->ps.type->status(&pg->ps,
					      &p->path, type, result + sz,
					      maxlen - sz);
			}
		}
		break;
	}

	spin_unlock_irqrestore(&m->lock, flags);
}

static int multipath_message(struct dm_target *ti, unsigned argc, char **argv)
{
	int r = -EINVAL;
	struct dm_dev *dev;
	struct multipath *m = (struct multipath *) ti->private;
	action_fn action;

	mutex_lock(&m->work_mutex);

	if (dm_suspended(ti)) {
		r = -EBUSY;
		goto out;
	}

	if (argc == 1) {
		if (!strcasecmp(argv[0], "queue_if_no_path")) {
			r = queue_if_no_path(m, 1, 0);
			goto out;
		} else if (!strcasecmp(argv[0], "fail_if_no_path")) {
			r = queue_if_no_path(m, 0, 0);
			goto out;
		}
	}

	if (argc != 2) {
		DMWARN("Unrecognised multipath message received.");
		goto out;
	}

	if (!strcasecmp(argv[0], "disable_group")) {
		r = bypass_pg_num(m, argv[1], 1);
		goto out;
	} else if (!strcasecmp(argv[0], "enable_group")) {
		r = bypass_pg_num(m, argv[1], 0);
		goto out;
	} else if (!strcasecmp(argv[0], "switch_group")) {
		r = switch_pg_num(m, argv[1]);
		goto out;
	} else if (!strcasecmp(argv[0], "reinstate_path"))
		action = reinstate_path;
	else if (!strcasecmp(argv[0], "fail_path"))
		action = fail_path;
	else {
		DMWARN("Unrecognised multipath message received.");
		goto out;
	}

	r = dm_get_device(ti, argv[1], dm_table_get_mode(ti->table), &dev);
	if (r) {
		DMWARN("message: error getting device %s",
		       argv[1]);
		goto out;
	}

	r = action_dev(m, dev, action);

	dm_put_device(ti, dev);

out:
	mutex_unlock(&m->work_mutex);
	return r;
}

static int multipath_ioctl(struct dm_target *ti, unsigned int cmd,
			   unsigned long arg)
{
	struct multipath *m = ti->private;
	struct pgpath *pgpath;
	struct block_device *bdev;
	fmode_t mode;
	unsigned long flags;
	int r;

	bdev = NULL;
	mode = 0;
	r = 0;

	spin_lock_irqsave(&m->lock, flags);

	if (!m->current_pgpath)
		__choose_pgpath(m, 0);

	pgpath = m->current_pgpath;

	if (pgpath) {
		bdev = pgpath->path.dev->bdev;
		mode = pgpath->path.dev->mode;
	}

	if ((pgpath && m->queue_io) || (!pgpath && m->queue_if_no_path))
		r = -ENOTCONN;
	else if (!bdev)
		r = -EIO;

	spin_unlock_irqrestore(&m->lock, flags);

	/*
                                                               
  */
	if (!bdev || ti->len != i_size_read(bdev->bd_inode) >> SECTOR_SHIFT) {
		int err = scsi_verify_blk_ioctl(NULL, cmd);
		if (err)
			r = err;
	}

	if (r == -ENOTCONN && !fatal_signal_pending(current))
		queue_work(kmultipathd, &m->process_queued_ios);

	return r ? : __blkdev_driver_ioctl(bdev, mode, cmd, arg);
}

static int multipath_iterate_devices(struct dm_target *ti,
				     iterate_devices_callout_fn fn, void *data)
{
	struct multipath *m = ti->private;
	struct priority_group *pg;
	struct pgpath *p;
	int ret = 0;

	list_for_each_entry(pg, &m->priority_groups, list) {
		list_for_each_entry(p, &pg->pgpaths, list) {
			ret = fn(ti, p->path.dev, ti->begin, ti->len, data);
			if (ret)
				goto out;
		}
	}

out:
	return ret;
}

static int __pgpath_busy(struct pgpath *pgpath)
{
	struct request_queue *q = bdev_get_queue(pgpath->path.dev->bdev);

	return dm_underlying_device_busy(q);
}

/*
                                                                     
                                                              
                         
                                                                  
                                                                 
                                                               
 */
static int multipath_busy(struct dm_target *ti)
{
	int busy = 0, has_active = 0;
	struct multipath *m = ti->private;
	struct priority_group *pg;
	struct pgpath *pgpath;
	unsigned long flags;

	spin_lock_irqsave(&m->lock, flags);

	/*                                                              */
	if (unlikely(!m->current_pgpath && m->next_pg))
		pg = m->next_pg;
	else if (likely(m->current_pg))
		pg = m->current_pg;
	else
		/*
                                                              
                                                             
                                   
                                                                 
                                                            
   */
		goto out;

	/*
                                                                    
                                                                    
  */
	busy = 1;
	list_for_each_entry(pgpath, &pg->pgpaths, list)
		if (pgpath->is_active) {
			has_active = 1;

			if (!__pgpath_busy(pgpath)) {
				busy = 0;
				break;
			}
		}

	if (!has_active)
		/*
                                                            
                                                         
                                            
   */
		busy = 0;

out:
	spin_unlock_irqrestore(&m->lock, flags);

	return busy;
}

/*                                                                 
               
                                                                 */
static struct target_type multipath_target = {
	.name = "multipath",
	.version = {1, 6, 0},
	.module = THIS_MODULE,
	.ctr = multipath_ctr,
	.dtr = multipath_dtr,
	.map_rq = multipath_map,
	.rq_end_io = multipath_end_io,
	.presuspend = multipath_presuspend,
	.postsuspend = multipath_postsuspend,
	.resume = multipath_resume,
	.status = multipath_status,
	.message = multipath_message,
	.ioctl  = multipath_ioctl,
	.iterate_devices = multipath_iterate_devices,
	.busy = multipath_busy,
};

static int __init dm_multipath_init(void)
{
	int r;

	/*                                */
	_mpio_cache = KMEM_CACHE(dm_mpath_io, 0);
	if (!_mpio_cache)
		return -ENOMEM;

	r = dm_register_target(&multipath_target);
	if (r < 0) {
		DMERR("register failed %d", r);
		kmem_cache_destroy(_mpio_cache);
		return -EINVAL;
	}

	kmultipathd = alloc_workqueue("kmpathd", WQ_MEM_RECLAIM, 0);
	if (!kmultipathd) {
		DMERR("failed to create workqueue kmpathd");
		dm_unregister_target(&multipath_target);
		kmem_cache_destroy(_mpio_cache);
		return -ENOMEM;
	}

	/*
                                                              
                                                            
                                                       
                                                   
  */
	kmpath_handlerd = alloc_ordered_workqueue("kmpath_handlerd",
						  WQ_MEM_RECLAIM);
	if (!kmpath_handlerd) {
		DMERR("failed to create workqueue kmpath_handlerd");
		destroy_workqueue(kmultipathd);
		dm_unregister_target(&multipath_target);
		kmem_cache_destroy(_mpio_cache);
		return -ENOMEM;
	}

	DMINFO("version %u.%u.%u loaded",
	       multipath_target.version[0], multipath_target.version[1],
	       multipath_target.version[2]);

	return r;
}

static void __exit dm_multipath_exit(void)
{
	destroy_workqueue(kmpath_handlerd);
	destroy_workqueue(kmultipathd);

	dm_unregister_target(&multipath_target);
	kmem_cache_destroy(_mpio_cache);
}

module_init(dm_multipath_init);
module_exit(dm_multipath_exit);

MODULE_DESCRIPTION(DM_NAME " multipath target");
MODULE_AUTHOR("Sistina Software <dm-devel@redhat.com>");
MODULE_LICENSE("GPL");
