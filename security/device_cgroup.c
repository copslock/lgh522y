/*
 * device_cgroup.c - device cgroup subsystem
 *
 * Copyright 2007 IBM Corp
 */

#include <linux/device_cgroup.h>
#include <linux/cgroup.h>
#include <linux/ctype.h>
#include <linux/list.h>
#include <linux/uaccess.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/rcupdate.h>
#include <linux/mutex.h>

#define ACC_MKNOD 1
#define ACC_READ  2
#define ACC_WRITE 4
#define ACC_MASK (ACC_MKNOD | ACC_READ | ACC_WRITE)

#define DEV_BLOCK 1
#define DEV_CHAR  2
#define DEV_ALL   4  /*                             */

static DEFINE_MUTEX(devcgroup_mutex);

enum devcg_behavior {
	DEVCG_DEFAULT_NONE,
	DEVCG_DEFAULT_ALLOW,
	DEVCG_DEFAULT_DENY,
};

/*
                                
                                        
                                 
 */

struct dev_exception_item {
	u32 major, minor;
	short type;
	short access;
	struct list_head list;
	struct rcu_head rcu;
};

struct dev_cgroup {
	struct cgroup_subsys_state css;
	struct list_head exceptions;
	enum devcg_behavior behavior;
	/*                                                   */
	struct list_head propagate_pending;
};

static inline struct dev_cgroup *css_to_devcgroup(struct cgroup_subsys_state *s)
{
	return container_of(s, struct dev_cgroup, css);
}

static inline struct dev_cgroup *cgroup_to_devcgroup(struct cgroup *cgroup)
{
	return css_to_devcgroup(cgroup_subsys_state(cgroup, devices_subsys_id));
}

static inline struct dev_cgroup *task_devcgroup(struct task_struct *task)
{
	return css_to_devcgroup(task_subsys_state(task, devices_subsys_id));
}

struct cgroup_subsys devices_subsys;

static int devcgroup_can_attach(struct cgroup *new_cgrp,
				struct cgroup_taskset *set)
{
	struct task_struct *task = cgroup_taskset_first(set);

	if (current != task && !capable(CAP_SYS_ADMIN))
		return -EPERM;
	return 0;
}

/*
                               
 */
static int dev_exceptions_copy(struct list_head *dest, struct list_head *orig)
{
	struct dev_exception_item *ex, *tmp, *new;

	lockdep_assert_held(&devcgroup_mutex);

	list_for_each_entry(ex, orig, list) {
		new = kmemdup(ex, sizeof(*ex), GFP_KERNEL);
		if (!new)
			goto free_and_exit;
		list_add_tail(&new->list, dest);
	}

	return 0;

free_and_exit:
	list_for_each_entry_safe(ex, tmp, dest, list) {
		list_del(&ex->list);
		kfree(ex);
	}
	return -ENOMEM;
}

/*
                               
 */
static int dev_exception_add(struct dev_cgroup *dev_cgroup,
			     struct dev_exception_item *ex)
{
	struct dev_exception_item *excopy, *walk;

	lockdep_assert_held(&devcgroup_mutex);

	excopy = kmemdup(ex, sizeof(*ex), GFP_KERNEL);
	if (!excopy)
		return -ENOMEM;

	list_for_each_entry(walk, &dev_cgroup->exceptions, list) {
		if (walk->type != ex->type)
			continue;
		if (walk->major != ex->major)
			continue;
		if (walk->minor != ex->minor)
			continue;

		walk->access |= ex->access;
		kfree(excopy);
		excopy = NULL;
	}

	if (excopy != NULL)
		list_add_tail_rcu(&excopy->list, &dev_cgroup->exceptions);
	return 0;
}

/*
                               
 */
static void dev_exception_rm(struct dev_cgroup *dev_cgroup,
			     struct dev_exception_item *ex)
{
	struct dev_exception_item *walk, *tmp;

	lockdep_assert_held(&devcgroup_mutex);

	list_for_each_entry_safe(walk, tmp, &dev_cgroup->exceptions, list) {
		if (walk->type != ex->type)
			continue;
		if (walk->major != ex->major)
			continue;
		if (walk->minor != ex->minor)
			continue;

		walk->access &= ~ex->access;
		if (!walk->access) {
			list_del_rcu(&walk->list);
			kfree_rcu(walk, rcu);
		}
	}
}

static void __dev_exception_clean(struct dev_cgroup *dev_cgroup)
{
	struct dev_exception_item *ex, *tmp;

	list_for_each_entry_safe(ex, tmp, &dev_cgroup->exceptions, list) {
		list_del_rcu(&ex->list);
		kfree_rcu(ex, rcu);
	}
}

/* 
                                                                
                                                                
  
                               
 */
static void dev_exception_clean(struct dev_cgroup *dev_cgroup)
{
	lockdep_assert_held(&devcgroup_mutex);

	__dev_exception_clean(dev_cgroup);
}

static inline bool is_devcg_online(const struct dev_cgroup *devcg)
{
	return (devcg->behavior != DEVCG_DEFAULT_NONE);
}

/* 
                                                                              
                   
                                 
                                                     
 */
static int devcgroup_online(struct cgroup *cgroup)
{
	struct dev_cgroup *dev_cgroup, *parent_dev_cgroup = NULL;
	int ret = 0;

	mutex_lock(&devcgroup_mutex);
	dev_cgroup = cgroup_to_devcgroup(cgroup);
	if (cgroup->parent)
		parent_dev_cgroup = cgroup_to_devcgroup(cgroup->parent);

	if (parent_dev_cgroup == NULL)
		dev_cgroup->behavior = DEVCG_DEFAULT_ALLOW;
	else {
		ret = dev_exceptions_copy(&dev_cgroup->exceptions,
					  &parent_dev_cgroup->exceptions);
		if (!ret)
			dev_cgroup->behavior = parent_dev_cgroup->behavior;
	}
	mutex_unlock(&devcgroup_mutex);

	return ret;
}

static void devcgroup_offline(struct cgroup *cgroup)
{
	struct dev_cgroup *dev_cgroup = cgroup_to_devcgroup(cgroup);

	mutex_lock(&devcgroup_mutex);
	dev_cgroup->behavior = DEVCG_DEFAULT_NONE;
	mutex_unlock(&devcgroup_mutex);
}

/*
                                                       
 */
static struct cgroup_subsys_state *devcgroup_css_alloc(struct cgroup *cgroup)
{
	struct dev_cgroup *dev_cgroup;

	dev_cgroup = kzalloc(sizeof(*dev_cgroup), GFP_KERNEL);
	if (!dev_cgroup)
		return ERR_PTR(-ENOMEM);
	INIT_LIST_HEAD(&dev_cgroup->exceptions);
	INIT_LIST_HEAD(&dev_cgroup->propagate_pending);
	dev_cgroup->behavior = DEVCG_DEFAULT_NONE;

	return &dev_cgroup->css;
}

static void devcgroup_css_free(struct cgroup *cgroup)
{
	struct dev_cgroup *dev_cgroup;

	dev_cgroup = cgroup_to_devcgroup(cgroup);
	__dev_exception_clean(dev_cgroup);
	kfree(dev_cgroup);
}

#define DEVCG_ALLOW 1
#define DEVCG_DENY 2
#define DEVCG_LIST 3

#define MAJMINLEN 13
#define ACCLEN 4

static void set_access(char *acc, short access)
{
	int idx = 0;
	memset(acc, 0, ACCLEN);
	if (access & ACC_READ)
		acc[idx++] = 'r';
	if (access & ACC_WRITE)
		acc[idx++] = 'w';
	if (access & ACC_MKNOD)
		acc[idx++] = 'm';
}

static char type_to_char(short type)
{
	if (type == DEV_ALL)
		return 'a';
	if (type == DEV_CHAR)
		return 'c';
	if (type == DEV_BLOCK)
		return 'b';
	return 'X';
}

static void set_majmin(char *str, unsigned m)
{
	if (m == ~0)
		strcpy(str, "*");
	else
		sprintf(str, "%u", m);
}

static int devcgroup_seq_read(struct cgroup *cgroup, struct cftype *cft,
				struct seq_file *m)
{
	struct dev_cgroup *devcgroup = cgroup_to_devcgroup(cgroup);
	struct dev_exception_item *ex;
	char maj[MAJMINLEN], min[MAJMINLEN], acc[ACCLEN];

	rcu_read_lock();
	/*
                                  
                                                                     
                                                               
                                                          
  */
	if (devcgroup->behavior == DEVCG_DEFAULT_ALLOW) {
		set_access(acc, ACC_MASK);
		set_majmin(maj, ~0);
		set_majmin(min, ~0);
		seq_printf(m, "%c %s:%s %s\n", type_to_char(DEV_ALL),
			   maj, min, acc);
	} else {
		list_for_each_entry_rcu(ex, &devcgroup->exceptions, list) {
			set_access(acc, ex->access);
			set_majmin(maj, ex->major);
			set_majmin(min, ex->minor);
			seq_printf(m, "%c %s:%s %s\n", type_to_char(ex->type),
				   maj, min, acc);
		}
	}
	rcu_read_unlock();

	return 0;
}

/* 
                                                                      
                                                 
                                                        
                                                    
                                          
                                               
                        
                                       
 */
static bool may_access(struct dev_cgroup *dev_cgroup,
		       struct dev_exception_item *refex,
		       enum devcg_behavior behavior)
{
	struct dev_exception_item *ex;
	bool match = false;

	rcu_lockdep_assert(rcu_read_lock_held() ||
			   lockdep_is_held(&devcgroup_mutex),
			   "device_cgroup::may_access() called without proper synchronization");

	list_for_each_entry_rcu(ex, &dev_cgroup->exceptions, list) {
		if ((refex->type & DEV_BLOCK) && !(ex->type & DEV_BLOCK))
			continue;
		if ((refex->type & DEV_CHAR) && !(ex->type & DEV_CHAR))
			continue;
		if (ex->major != ~0 && ex->major != refex->major)
			continue;
		if (ex->minor != ~0 && ex->minor != refex->minor)
			continue;
		if (refex->access & (~ex->access))
			continue;
		match = true;
		break;
	}

	if (dev_cgroup->behavior == DEVCG_DEFAULT_ALLOW) {
		if (behavior == DEVCG_DEFAULT_ALLOW) {
			/*                                                   */
			return true;
		} else {
			/*                                                    */
			if (match)
				/*
                                                
                                  
     */
				return false;
			return true;
		}
	} else {
		/*                                                  */
		if (match)
			/*                                                   */
			return true;
		else
			return false;
	}
	return false;
}

/*
                   
                                                                    
                                       
 */
static int parent_has_perm(struct dev_cgroup *childcg,
				  struct dev_exception_item *ex)
{
	struct cgroup *pcg = childcg->css.cgroup->parent;
	struct dev_cgroup *parent;

	if (!pcg)
		return 1;
	parent = cgroup_to_devcgroup(pcg);
	return may_access(parent, ex, childcg->behavior);
}

/* 
                                                                    
                                     
                                  
                                                  
 */
static inline int may_allow_all(struct dev_cgroup *parent)
{
	if (!parent)
		return 1;
	return parent->behavior == DEVCG_DEFAULT_ALLOW;
}

/* 
                                                                             
                                                     
                                                     
                                             
                                          
                                                  
  
                                                                       
                                                                         
                                                 
                                                               
 */
static void revalidate_active_exceptions(struct dev_cgroup *devcg)
{
	struct dev_exception_item *ex;
	struct list_head *this, *tmp;

	list_for_each_safe(this, tmp, &devcg->exceptions) {
		ex = container_of(this, struct dev_exception_item, list);
		if (!parent_has_perm(devcg, ex))
			dev_exception_rm(devcg, ex);
	}
}

/* 
                                                                            
                 
                                       
                                                       
  
                                                            
                                                                          
                                                                 
                                                              
                                                               
 */
static void get_online_devcg(struct cgroup *root, struct list_head *online)
{
	struct cgroup *pos;
	struct dev_cgroup *devcg;

	lockdep_assert_held(&devcgroup_mutex);

	rcu_read_lock();
	cgroup_for_each_descendant_pre(pos, root) {
		devcg = cgroup_to_devcgroup(pos);
		if (is_devcg_online(devcg))
			list_add_tail(&devcg->propagate_pending, online);
	}
	rcu_read_unlock();
}

/* 
                                                                   
                                                        
                                      
  
                                                       
 */
static int propagate_exception(struct dev_cgroup *devcg_root,
			       struct dev_exception_item *ex)
{
	struct cgroup *root = devcg_root->css.cgroup;
	struct dev_cgroup *devcg, *parent, *tmp;
	int rc = 0;
	LIST_HEAD(pending);

	get_online_devcg(root, &pending);

	list_for_each_entry_safe(devcg, tmp, &pending, propagate_pending) {
		parent = cgroup_to_devcgroup(devcg->css.cgroup->parent);

		/*
                                                           
                                                   
   */
		if (devcg_root->behavior == DEVCG_DEFAULT_ALLOW &&
		    devcg->behavior == DEVCG_DEFAULT_ALLOW) {
			rc = dev_exception_add(devcg, ex);
			if (rc)
				break;
		} else {
			/*
                                  
                                           
                                          
                                   
    */
			dev_exception_rm(devcg, ex);
		}
		revalidate_active_exceptions(devcg);

		list_del_init(&devcg->propagate_pending);
	}
	return rc;
}

static inline bool has_children(struct dev_cgroup *devcgroup)
{
	struct cgroup *cgrp = devcgroup->css.cgroup;

	return !list_empty(&cgrp->children);
}

/*
                                                    
                                                                           
                                                                        
                             
                                                                        
                                                                       
                                                                             
  
                                                                       
                                                                        
                                                  
 */
static int devcgroup_update_access(struct dev_cgroup *devcgroup,
				   int filetype, const char *buffer)
{
	const char *b;
	char temp[12];		/*                                    */
	int count, rc = 0;
	struct dev_exception_item ex;
	struct cgroup *p = devcgroup->css.cgroup;
	struct dev_cgroup *parent = NULL;

	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;

	if (p->parent)
		parent = cgroup_to_devcgroup(p->parent);

	memset(&ex, 0, sizeof(ex));
	b = buffer;

	switch (*b) {
	case 'a':
		switch (filetype) {
		case DEVCG_ALLOW:
			if (has_children(devcgroup))
				return -EINVAL;

			if (!may_allow_all(parent))
				return -EPERM;
			dev_exception_clean(devcgroup);
			devcgroup->behavior = DEVCG_DEFAULT_ALLOW;
			if (!parent)
				break;

			rc = dev_exceptions_copy(&devcgroup->exceptions,
						 &parent->exceptions);
			if (rc)
				return rc;
			break;
		case DEVCG_DENY:
			if (has_children(devcgroup))
				return -EINVAL;

			dev_exception_clean(devcgroup);
			devcgroup->behavior = DEVCG_DEFAULT_DENY;
			break;
		default:
			return -EINVAL;
		}
		return 0;
	case 'b':
		ex.type = DEV_BLOCK;
		break;
	case 'c':
		ex.type = DEV_CHAR;
		break;
	default:
		return -EINVAL;
	}
	b++;
	if (!isspace(*b))
		return -EINVAL;
	b++;
	if (*b == '*') {
		ex.major = ~0;
		b++;
	} else if (isdigit(*b)) {
		memset(temp, 0, sizeof(temp));
		for (count = 0; count < sizeof(temp) - 1; count++) {
			temp[count] = *b;
			b++;
			if (!isdigit(*b))
				break;
		}
		rc = kstrtou32(temp, 10, &ex.major);
		if (rc)
			return -EINVAL;
	} else {
		return -EINVAL;
	}
	if (*b != ':')
		return -EINVAL;
	b++;

	/*            */
	if (*b == '*') {
		ex.minor = ~0;
		b++;
	} else if (isdigit(*b)) {
		memset(temp, 0, sizeof(temp));
		for (count = 0; count < sizeof(temp) - 1; count++) {
			temp[count] = *b;
			b++;
			if (!isdigit(*b))
				break;
		}
		rc = kstrtou32(temp, 10, &ex.minor);
		if (rc)
			return -EINVAL;
	} else {
		return -EINVAL;
	}
	if (!isspace(*b))
		return -EINVAL;
	for (b++, count = 0; count < 3; count++, b++) {
		switch (*b) {
		case 'r':
			ex.access |= ACC_READ;
			break;
		case 'w':
			ex.access |= ACC_WRITE;
			break;
		case 'm':
			ex.access |= ACC_MKNOD;
			break;
		case '\n':
		case '\0':
			count = 3;
			break;
		default:
			return -EINVAL;
		}
	}

	switch (filetype) {
	case DEVCG_ALLOW:
		if (!parent_has_perm(devcgroup, &ex))
			return -EPERM;
		/*
                                                                
                                                              
                                      
   */
		if (devcgroup->behavior == DEVCG_DEFAULT_ALLOW) {
			dev_exception_rm(devcgroup, &ex);
			return 0;
		}
		rc = dev_exception_add(devcgroup, &ex);
		break;
	case DEVCG_DENY:
		/*
                                                               
                                                              
                                      
   */
		if (devcgroup->behavior == DEVCG_DEFAULT_DENY)
			dev_exception_rm(devcgroup, &ex);
		else
			rc = dev_exception_add(devcgroup, &ex);

		if (rc)
			break;
		/*                                    */
		rc = propagate_exception(devcgroup, &ex);
		break;
	default:
		rc = -EINVAL;
	}
	return rc;
}

static int devcgroup_access_write(struct cgroup *cgrp, struct cftype *cft,
				  const char *buffer)
{
	int retval;

	mutex_lock(&devcgroup_mutex);
	retval = devcgroup_update_access(cgroup_to_devcgroup(cgrp),
					 cft->private, buffer);
	mutex_unlock(&devcgroup_mutex);
	return retval;
}

static struct cftype dev_cgroup_files[] = {
	{
		.name = "allow",
		.write_string  = devcgroup_access_write,
		.private = DEVCG_ALLOW,
	},
	{
		.name = "deny",
		.write_string = devcgroup_access_write,
		.private = DEVCG_DENY,
	},
	{
		.name = "list",
		.read_seq_string = devcgroup_seq_read,
		.private = DEVCG_LIST,
	},
	{ }	/*           */
};

struct cgroup_subsys devices_subsys = {
	.name = "devices",
	.can_attach = devcgroup_can_attach,
	.css_alloc = devcgroup_css_alloc,
	.css_free = devcgroup_css_free,
	.css_online = devcgroup_online,
	.css_offline = devcgroup_offline,
	.subsys_id = devices_subsys_id,
	.base_cftypes = dev_cgroup_files,
};

/* 
                                                                           
                                                   
                     
                              
                              
                                                            
  
                                                                   
 */
static int __devcgroup_check_permission(short type, u32 major, u32 minor,
				        short access)
{
	struct dev_cgroup *dev_cgroup;
	struct dev_exception_item ex;
	int rc;

	memset(&ex, 0, sizeof(ex));
	ex.type = type;
	ex.major = major;
	ex.minor = minor;
	ex.access = access;

	rcu_read_lock();
	dev_cgroup = task_devcgroup(current);
	rc = may_access(dev_cgroup, &ex, dev_cgroup->behavior);
	rcu_read_unlock();

	if (!rc)
		return -EPERM;

	return 0;
}

int __devcgroup_inode_permission(struct inode *inode, int mask)
{
	short type, access = 0;

	if (S_ISBLK(inode->i_mode))
		type = DEV_BLOCK;
	if (S_ISCHR(inode->i_mode))
		type = DEV_CHAR;
	if (mask & MAY_WRITE)
		access |= ACC_WRITE;
	if (mask & MAY_READ)
		access |= ACC_READ;

	return __devcgroup_check_permission(type, imajor(inode), iminor(inode),
			access);
}

int devcgroup_inode_mknod(int mode, dev_t dev)
{
	short type;

	if (!S_ISBLK(mode) && !S_ISCHR(mode))
		return 0;

	if (S_ISBLK(mode))
		type = DEV_BLOCK;
	else
		type = DEV_CHAR;

	return __devcgroup_check_permission(type, MAJOR(dev), MINOR(dev),
			ACC_MKNOD);

}
