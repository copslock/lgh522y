#include <linux/types.h>
#include <linux/errno.h>
#include <linux/kmod.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/file.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/wait.h>
#include <linux/bitops.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/ratelimit.h>

#undef LDISC_DEBUG_HANGUP

#ifdef LDISC_DEBUG_HANGUP
#define tty_ldisc_debug(tty, f, args...) ({				       \
	char __b[64];							       \
	printk(KERN_DEBUG "%s: %s: " f, __func__, tty_name(tty, __b), ##args); \
})
#else
#define tty_ldisc_debug(tty, f, args...)
#endif

/*                                           */
enum {
	LDISC_SEM_NORMAL,
	LDISC_SEM_OTHER,
};


/*
                                                             
                                                            
                                                      
 */

static DEFINE_RAW_SPINLOCK(tty_ldiscs_lock);
/*                          */
static struct tty_ldisc_ops *tty_ldiscs[NR_LDISCS];

/* 
                                                 
                      
                                          
  
                                                                 
                                                                  
                           
  
           
                                                      
 */

int tty_register_ldisc(int disc, struct tty_ldisc_ops *new_ldisc)
{
	unsigned long flags;
	int ret = 0;

	if (disc < N_TTY || disc >= NR_LDISCS)
		return -EINVAL;

	raw_spin_lock_irqsave(&tty_ldiscs_lock, flags);
	tty_ldiscs[disc] = new_ldisc;
	new_ldisc->num = disc;
	new_ldisc->refcount = 0;
	raw_spin_unlock_irqrestore(&tty_ldiscs_lock, flags);

	return ret;
}
EXPORT_SYMBOL(tty_register_ldisc);

/* 
                                                  
                      
                                          
  
                                                               
                    
  
           
                                                      
 */

int tty_unregister_ldisc(int disc)
{
	unsigned long flags;
	int ret = 0;

	if (disc < N_TTY || disc >= NR_LDISCS)
		return -EINVAL;

	raw_spin_lock_irqsave(&tty_ldiscs_lock, flags);
	if (tty_ldiscs[disc]->refcount)
		ret = -EBUSY;
	else
		tty_ldiscs[disc] = NULL;
	raw_spin_unlock_irqrestore(&tty_ldiscs_lock, flags);

	return ret;
}
EXPORT_SYMBOL(tty_unregister_ldisc);

static struct tty_ldisc_ops *get_ldops(int disc)
{
	unsigned long flags;
	struct tty_ldisc_ops *ldops, *ret;

	raw_spin_lock_irqsave(&tty_ldiscs_lock, flags);
	ret = ERR_PTR(-EINVAL);
	ldops = tty_ldiscs[disc];
	if (ldops) {
		ret = ERR_PTR(-EAGAIN);
		if (try_module_get(ldops->owner)) {
			ldops->refcount++;
			ret = ldops;
		}
	}
	raw_spin_unlock_irqrestore(&tty_ldiscs_lock, flags);
	return ret;
}

static void put_ldops(struct tty_ldisc_ops *ldops)
{
	unsigned long flags;

	raw_spin_lock_irqsave(&tty_ldiscs_lock, flags);
	ldops->refcount--;
	module_put(ldops->owner);
	raw_spin_unlock_irqrestore(&tty_ldiscs_lock, flags);
}

/* 
                                                
                      
  
                                                                   
                                                                          
                                                                       
            
  
           
                                                      
 */

static struct tty_ldisc *tty_ldisc_get(struct tty_struct *tty, int disc)
{
	struct tty_ldisc *ld;
	struct tty_ldisc_ops *ldops;

	if (disc < N_TTY || disc >= NR_LDISCS)
		return ERR_PTR(-EINVAL);

	/*
                                                                
                              
  */
	ldops = get_ldops(disc);
	if (IS_ERR(ldops)) {
		request_module("tty-ldisc-%d", disc);
		ldops = get_ldops(disc);
		if (IS_ERR(ldops))
			return ERR_CAST(ldops);
	}

	ld = kmalloc(sizeof(struct tty_ldisc), GFP_KERNEL);
	if (ld == NULL) {
		put_ldops(ldops);
		return ERR_PTR(-ENOMEM);
	}

	ld->ops = ldops;
	ld->tty = tty;

	return ld;
}

/* 
                                     
  
                                 
 */
static inline void tty_ldisc_put(struct tty_ldisc *ld)
{
	if (WARN_ON_ONCE(!ld))
		return;

	put_ldops(ld->ops);
	kfree(ld);
}

static void *tty_ldiscs_seq_start(struct seq_file *m, loff_t *pos)
{
	return (*pos < NR_LDISCS) ? pos : NULL;
}

static void *tty_ldiscs_seq_next(struct seq_file *m, void *v, loff_t *pos)
{
	(*pos)++;
	return (*pos < NR_LDISCS) ? pos : NULL;
}

static void tty_ldiscs_seq_stop(struct seq_file *m, void *v)
{
}

static int tty_ldiscs_seq_show(struct seq_file *m, void *v)
{
	int i = *(loff_t *)v;
	struct tty_ldisc_ops *ldops;

	ldops = get_ldops(i);
	if (IS_ERR(ldops))
		return 0;
	seq_printf(m, "%-10s %2d\n", ldops->name ? ldops->name : "???", i);
	put_ldops(ldops);
	return 0;
}

static const struct seq_operations tty_ldiscs_seq_ops = {
	.start	= tty_ldiscs_seq_start,
	.next	= tty_ldiscs_seq_next,
	.stop	= tty_ldiscs_seq_stop,
	.show	= tty_ldiscs_seq_show,
};

static int proc_tty_ldiscs_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &tty_ldiscs_seq_ops);
}

const struct file_operations tty_ldiscs_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= proc_tty_ldiscs_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release,
};

/* 
                                              
                   
  
                                                              
                                                          
                                   
  
                                                                 
                                                                  
                                                                   
                       
  
                                                            
                                                            
 */

struct tty_ldisc *tty_ldisc_ref_wait(struct tty_struct *tty)
{
	ldsem_down_read(&tty->ldisc_sem, MAX_SCHEDULE_TIMEOUT);
	WARN_ON(!tty->ldisc);
	return tty->ldisc;
}
EXPORT_SYMBOL_GPL(tty_ldisc_ref_wait);

/* 
                                     
                   
  
                                                              
                                                          
                                                           
 */

struct tty_ldisc *tty_ldisc_ref(struct tty_struct *tty)
{
	struct tty_ldisc *ld = NULL;

	if (ldsem_down_read_trylock(&tty->ldisc_sem)) {
		ld = tty->ldisc;
		if (!ld)
			ldsem_up_read(&tty->ldisc_sem);
	}
	return ld;
}
EXPORT_SYMBOL_GPL(tty_ldisc_ref);

/* 
                                                
                            
  
                                                                
                            
 */

void tty_ldisc_deref(struct tty_ldisc *ld)
{
	ldsem_up_read(&ld->tty->ldisc_sem);
}
EXPORT_SYMBOL_GPL(tty_ldisc_deref);


static inline int __lockfunc
tty_ldisc_lock(struct tty_struct *tty, unsigned long timeout)
{
	return ldsem_down_write(&tty->ldisc_sem, timeout);
}

static inline int __lockfunc
tty_ldisc_lock_nested(struct tty_struct *tty, unsigned long timeout)
{
	return ldsem_down_write_nested(&tty->ldisc_sem,
				       LDISC_SEM_OTHER, timeout);
}

static inline void tty_ldisc_unlock(struct tty_struct *tty)
{
	return ldsem_up_write(&tty->ldisc_sem);
}

static int __lockfunc
tty_ldisc_lock_pair_timeout(struct tty_struct *tty, struct tty_struct *tty2,
			    unsigned long timeout)
{
	int ret;

	if (tty < tty2) {
		ret = tty_ldisc_lock(tty, timeout);
		if (ret) {
			ret = tty_ldisc_lock_nested(tty2, timeout);
			if (!ret)
				tty_ldisc_unlock(tty);
		}
	} else {
		/*                                                  */
		WARN_ON_ONCE(tty == tty2);
		if (tty2 && tty != tty2) {
			ret = tty_ldisc_lock(tty2, timeout);
			if (ret) {
				ret = tty_ldisc_lock_nested(tty, timeout);
				if (!ret)
					tty_ldisc_unlock(tty2);
			}
		} else
			ret = tty_ldisc_lock(tty, timeout);
	}

	if (!ret)
		return -EBUSY;

	set_bit(TTY_LDISC_HALTED, &tty->flags);
	if (tty2)
		set_bit(TTY_LDISC_HALTED, &tty2->flags);
	return 0;
}

static void __lockfunc
tty_ldisc_lock_pair(struct tty_struct *tty, struct tty_struct *tty2)
{
	tty_ldisc_lock_pair_timeout(tty, tty2, MAX_SCHEDULE_TIMEOUT);
}

static void __lockfunc tty_ldisc_unlock_pair(struct tty_struct *tty,
					     struct tty_struct *tty2)
{
	tty_ldisc_unlock(tty);
	if (tty2)
		tty_ldisc_unlock(tty2);
}

static void __lockfunc tty_ldisc_enable_pair(struct tty_struct *tty,
					     struct tty_struct *tty2)
{
	clear_bit(TTY_LDISC_HALTED, &tty->flags);
	if (tty2)
		clear_bit(TTY_LDISC_HALTED, &tty2->flags);

	tty_ldisc_unlock_pair(tty, tty2);
}

/* 
                                                
            
  
                                                                  
                                                
 */

void tty_ldisc_flush(struct tty_struct *tty)
{
	struct tty_ldisc *ld = tty_ldisc_ref(tty);
	if (ld) {
		if (ld->ops->flush_buffer)
			ld->ops->flush_buffer(tty);
		tty_ldisc_deref(ld);
	}
	tty_buffer_flush(tty);
}
EXPORT_SYMBOL_GPL(tty_ldisc_flush);

/* 
                                           
                      
                               
  
                                                          
                                                            
            
  
                               
 */

static void tty_set_termios_ldisc(struct tty_struct *tty, int num)
{
	mutex_lock(&tty->termios_mutex);
	tty->termios.c_line = num;
	mutex_unlock(&tty->termios_mutex);
}

/* 
                                           
                                        
                          
  
                                                                 
         
  
                                                
 */

static int tty_ldisc_open(struct tty_struct *tty, struct tty_ldisc *ld)
{
	WARN_ON(test_and_set_bit(TTY_LDISC_OPEN, &tty->flags));
	if (ld->ops->open) {
		int ret;
                /*                                      */
		ret = ld->ops->open(tty);
		if (ret)
			clear_bit(TTY_LDISC_OPEN, &tty->flags);
		return ret;
	}
	return 0;
}

/* 
                                             
                                        
                           
  
                                                               
         
 */

static void tty_ldisc_close(struct tty_struct *tty, struct tty_ldisc *ld)
{
	WARN_ON(!test_bit(TTY_LDISC_OPEN, &tty->flags));
	clear_bit(TTY_LDISC_OPEN, &tty->flags);
	if (ld->ops->close)
		ld->ops->close(tty);
}

/* 
                                                  
                       
                       
  
                                                                       
                                    
 */

static void tty_ldisc_restore(struct tty_struct *tty, struct tty_ldisc *old)
{
	char buf[64];
	struct tty_ldisc *new_ldisc;
	int r;

	/*                                                        */
	old = tty_ldisc_get(tty, old->ops->num);
	WARN_ON(IS_ERR(old));
	tty->ldisc = old;
	tty_set_termios_ldisc(tty, old->ops->num);
	if (tty_ldisc_open(tty, old) < 0) {
		tty_ldisc_put(old);
		/*                               */
		new_ldisc = tty_ldisc_get(tty, N_TTY);
		if (IS_ERR(new_ldisc))
			panic("n_tty: get");
		tty->ldisc = new_ldisc;
		tty_set_termios_ldisc(tty, N_TTY);
		r = tty_ldisc_open(tty, new_ldisc);
		if (r < 0)
			panic("Couldn't open N_TTY ldisc for "
			      "%s --- error %d.",
			      tty_name(tty, buf), r);
	}
}

/* 
                                       
                            
                              
  
                                                                  
                                                                    
                                                                      
                                                                  
 */

int tty_set_ldisc(struct tty_struct *tty, int ldisc)
{
	int retval;
	struct tty_ldisc *o_ldisc, *new_ldisc;
	struct tty_struct *o_tty = tty->link;

	new_ldisc = tty_ldisc_get(tty, ldisc);
	if (IS_ERR(new_ldisc))
		return PTR_ERR(new_ldisc);

	retval = tty_ldisc_lock_pair_timeout(tty, o_tty, 5 * HZ);
	if (retval) {
		tty_ldisc_put(new_ldisc);
		return retval;
	}

	/*
                        
  */

	if (tty->ldisc->ops->num == ldisc) {
		tty_ldisc_enable_pair(tty, o_tty);
		tty_ldisc_put(new_ldisc);
		return 0;
	}

	/*                                                    */
	tty->receive_room = 0;

	o_ldisc = tty->ldisc;
	tty_lock(tty);

	/*                         */
	WARN_ON(test_bit(TTY_HUPPED, &tty->flags));

	if (test_bit(TTY_HUPPING, &tty->flags)) {
		/*                                                         
                                              */
		tty_ldisc_enable_pair(tty, o_tty);
		tty_ldisc_put(new_ldisc);
		tty_unlock(tty);
		return -EIO;
	}

	/*                                  */
	tty_ldisc_close(tty, o_ldisc);

	/*                                     */
	tty->ldisc = new_ldisc;
	tty_set_termios_ldisc(tty, ldisc);

	retval = tty_ldisc_open(tty, new_ldisc);
	if (retval < 0) {
		/*                                          */
		tty_ldisc_put(new_ldisc);
		tty_ldisc_restore(tty, o_ldisc);
	}

	/*                                                         
                                                              
                                                       */

	if (tty->ldisc->ops->num != o_ldisc->ops->num && tty->ops->set_ldisc)
		tty->ops->set_ldisc(tty);

	tty_ldisc_put(o_ldisc);

	/*
                                          
  */
	tty_ldisc_enable_pair(tty, o_tty);

	/*                                                                  
                    */
	schedule_work(&tty->port->buf.work);
	if (o_tty)
		schedule_work(&o_tty->port->buf.work);

	tty_unlock(tty);
	return retval;
}

/* 
                                           
                     
  
                                                  
 */

static void tty_reset_termios(struct tty_struct *tty)
{
	mutex_lock(&tty->termios_mutex);
	tty->termios = tty->driver->init_termios;
	tty->termios.c_ispeed = tty_termios_input_baud_rate(&tty->termios);
	tty->termios.c_ospeed = tty_termios_baud_rate(&tty->termios);
	mutex_unlock(&tty->termios_mutex);
}


/* 
                                                
                      
                                          
  
                                                          
               
 */

static int tty_ldisc_reinit(struct tty_struct *tty, int ldisc)
{
	struct tty_ldisc *ld = tty_ldisc_get(tty, ldisc);

	if (IS_ERR(ld))
		return -1;

	tty_ldisc_close(tty, tty->ldisc);
	tty_ldisc_put(tty->ldisc);
	/*
                                   
  */
	tty->ldisc = ld;
	tty_set_termios_ldisc(tty, ldisc);

	return 0;
}

/* 
                                         
                          
  
                                                                  
                                                                      
                                    
  
                                                                  
                             
  
                                                              
                                                        
 */

void tty_ldisc_hangup(struct tty_struct *tty)
{
	struct tty_ldisc *ld;
	int reset = tty->driver->flags & TTY_DRIVER_RESET_TERMIOS;
	int err = 0;

	tty_ldisc_debug(tty, "closing ldisc: %p\n", tty->ldisc);

	ld = tty_ldisc_ref(tty);
	if (ld != NULL) {
		if (ld->ops->flush_buffer)
			ld->ops->flush_buffer(tty);
		tty_driver_flush_buffer(tty);
		if ((test_bit(TTY_DO_WRITE_WAKEUP, &tty->flags)) &&
		    ld->ops->write_wakeup)
			ld->ops->write_wakeup(tty);
		if (ld->ops->hangup)
			ld->ops->hangup(tty);
		tty_ldisc_deref(ld);
	}

	wake_up_interruptible_poll(&tty->write_wait, POLLOUT);
	wake_up_interruptible_poll(&tty->read_wait, POLLIN);

	tty_unlock(tty);

	/*
                                                         
                     
   
                                               
  */
	tty_ldisc_lock_pair(tty, tty->link);
	tty_lock(tty);

	if (tty->ldisc) {

		/*                                                              
                                                              
                                                               
             */
		if (reset == 0) {

			if (!tty_ldisc_reinit(tty, tty->termios.c_line))
				err = tty_ldisc_open(tty, tty->ldisc);
			else
				err = 1;
		}
		/*                                                       
                            */
		if (reset || err) {
			BUG_ON(tty_ldisc_reinit(tty, N_TTY));
			WARN_ON(tty_ldisc_open(tty, tty->ldisc));
		}
	}
	tty_ldisc_enable_pair(tty, tty->link);
	if (reset)
		tty_reset_termios(tty);

	tty_ldisc_debug(tty, "re-opened ldisc: %p\n", tty->ldisc);
}

/* 
                                           
                            
                                     
  
                                                                          
                                                                        
                                  
 */

int tty_ldisc_setup(struct tty_struct *tty, struct tty_struct *o_tty)
{
	struct tty_ldisc *ld = tty->ldisc;
	int retval;

	retval = tty_ldisc_open(tty, ld);
	if (retval)
		return retval;

	if (o_tty) {
		retval = tty_ldisc_open(o_tty, o_tty->ldisc);
		if (retval) {
			tty_ldisc_close(tty, ld);
			return retval;
		}
	}
	return 0;
}

static void tty_ldisc_kill(struct tty_struct *tty)
{
	/*
                          
  */
	tty_ldisc_close(tty, tty->ldisc);
	tty_ldisc_put(tty->ldisc);
	/*                                  */
	tty->ldisc = NULL;

	/*                                               */
	tty_set_termios_ldisc(tty, N_TTY);
}

/* 
                                               
                            
                                     
  
                                                                        
                                                                        
                             
 */

void tty_ldisc_release(struct tty_struct *tty, struct tty_struct *o_tty)
{
	/*
                                                              
                                                  
  */

	tty_ldisc_debug(tty, "closing ldisc: %p\n", tty->ldisc);

	tty_ldisc_lock_pair(tty, o_tty);
	tty_lock_pair(tty, o_tty);

	tty_ldisc_kill(tty);
	if (o_tty)
		tty_ldisc_kill(o_tty);

	tty_unlock_pair(tty, o_tty);
	tty_ldisc_unlock_pair(tty, o_tty);

	/*                                                              
                                        */

	tty_ldisc_debug(tty, "ldisc closed\n");
}

/* 
                                            
                            
  
                                                                          
                                                                     
 */

void tty_ldisc_init(struct tty_struct *tty)
{
	struct tty_ldisc *ld = tty_ldisc_get(tty, N_TTY);
	if (IS_ERR(ld))
		panic("n_tty: init_tty");
	tty->ldisc = ld;
}

/* 
                                              
                                        
  
                                                                        
                          
 */
void tty_ldisc_deinit(struct tty_struct *tty)
{
	tty_ldisc_put(tty->ldisc);
	tty->ldisc = NULL;
}

void tty_ldisc_begin(void)
{
	/*                                        */
	(void) tty_register_ldisc(N_TTY, &tty_ldisc_N_TTY);
}
