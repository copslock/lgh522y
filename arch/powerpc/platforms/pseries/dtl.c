/*
 * Virtual Processor Dispatch Trace Log
 *
 * (C) Copyright IBM Corporation 2009
 *
 * Author: Jeremy Kerr <jk@ozlabs.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/init.h>
#include <linux/slab.h>
#include <linux/debugfs.h>
#include <linux/spinlock.h>
#include <asm/smp.h>
#include <asm/uaccess.h>
#include <asm/firmware.h>
#include <asm/lppaca.h>
#include <asm/debug.h>

#include "plpar_wrappers.h"

struct dtl {
	struct dtl_entry	*buf;
	struct dentry		*file;
	int			cpu;
	int			buf_entries;
	u64			last_idx;
	spinlock_t		lock;
};
static DEFINE_PER_CPU(struct dtl, cpu_dtl);

/*
                                 
                                              
                                
                                                 
 */
static u8 dtl_event_mask = 0x7;


/*
                                                                      
                           
 */
static int dtl_buf_entries = N_DISPATCH_LOG;

#ifdef CONFIG_VIRT_CPU_ACCOUNTING_NATIVE
struct dtl_ring {
	u64	write_index;
	struct dtl_entry *write_ptr;
	struct dtl_entry *buf;
	struct dtl_entry *buf_end;
	u8	saved_dtl_mask;
};

static DEFINE_PER_CPU(struct dtl_ring, dtl_rings);

static atomic_t dtl_count;

/*
                                                                   
                                       
 */
static void consume_dtle(struct dtl_entry *dtle, u64 index)
{
	struct dtl_ring *dtlr = &__get_cpu_var(dtl_rings);
	struct dtl_entry *wp = dtlr->write_ptr;
	struct lppaca *vpa = local_paca->lppaca_ptr;

	if (!wp)
		return;

	*wp = *dtle;
	barrier();

	/*                                                                    */
	if (index + N_DISPATCH_LOG < vpa->dtl_idx)
		return;

	++wp;
	if (wp == dtlr->buf_end)
		wp = dtlr->buf;
	dtlr->write_ptr = wp;

	/*                                                      */
	smp_wmb();
	++dtlr->write_index;
}

static int dtl_start(struct dtl *dtl)
{
	struct dtl_ring *dtlr = &per_cpu(dtl_rings, dtl->cpu);

	dtlr->buf = dtl->buf;
	dtlr->buf_end = dtl->buf + dtl->buf_entries;
	dtlr->write_index = 0;

	/*                                                   */
	smp_wmb();
	dtlr->write_ptr = dtl->buf;

	/*                      */
	dtlr->saved_dtl_mask = lppaca_of(dtl->cpu).dtl_enable_mask;
	lppaca_of(dtl->cpu).dtl_enable_mask |= dtl_event_mask;

	dtl_consumer = consume_dtle;
	atomic_inc(&dtl_count);
	return 0;
}

static void dtl_stop(struct dtl *dtl)
{
	struct dtl_ring *dtlr = &per_cpu(dtl_rings, dtl->cpu);

	dtlr->write_ptr = NULL;
	smp_wmb();

	dtlr->buf = NULL;

	/*                         */
	lppaca_of(dtl->cpu).dtl_enable_mask = dtlr->saved_dtl_mask;

	if (atomic_dec_and_test(&dtl_count))
		dtl_consumer = NULL;
}

static u64 dtl_current_index(struct dtl *dtl)
{
	return per_cpu(dtl_rings, dtl->cpu).write_index;
}

#else /*                                   */

static int dtl_start(struct dtl *dtl)
{
	unsigned long addr;
	int ret, hwcpu;

	/*                                                                
                                                              */
	((u32 *)dtl->buf)[1] = DISPATCH_LOG_BYTES;

	hwcpu = get_hard_smp_processor_id(dtl->cpu);
	addr = __pa(dtl->buf);
	ret = register_dtl(hwcpu, addr);
	if (ret) {
		printk(KERN_WARNING "%s: DTL registration for cpu %d (hw %d) "
		       "failed with %d\n", __func__, dtl->cpu, hwcpu, ret);
		return -EIO;
	}

	/*                                */
	lppaca_of(dtl->cpu).dtl_idx = 0;

	/*                                                                  
                                   */
	smp_wmb();

	/*                      */
	lppaca_of(dtl->cpu).dtl_enable_mask = dtl_event_mask;

	return 0;
}

static void dtl_stop(struct dtl *dtl)
{
	int hwcpu = get_hard_smp_processor_id(dtl->cpu);

	lppaca_of(dtl->cpu).dtl_enable_mask = 0x0;

	unregister_dtl(hwcpu);
}

static u64 dtl_current_index(struct dtl *dtl)
{
	return lppaca_of(dtl->cpu).dtl_idx;
}
#endif /*                                   */

static int dtl_enable(struct dtl *dtl)
{
	long int n_entries;
	long int rc;
	struct dtl_entry *buf = NULL;

	if (!dtl_cache)
		return -ENOMEM;

	/*                       */
	if (dtl->buf)
		return -EBUSY;

	n_entries = dtl_buf_entries;
	buf = kmem_cache_alloc_node(dtl_cache, GFP_KERNEL, cpu_to_node(dtl->cpu));
	if (!buf) {
		printk(KERN_WARNING "%s: buffer alloc failed for cpu %d\n",
				__func__, dtl->cpu);
		return -ENOMEM;
	}

	spin_lock(&dtl->lock);
	rc = -EBUSY;
	if (!dtl->buf) {
		/*                                                        */
		dtl->buf_entries = n_entries;
		dtl->buf = buf;
		dtl->last_idx = 0;
		rc = dtl_start(dtl);
		if (rc)
			dtl->buf = NULL;
	}
	spin_unlock(&dtl->lock);

	if (rc)
		kmem_cache_free(dtl_cache, buf);
	return rc;
}

static void dtl_disable(struct dtl *dtl)
{
	spin_lock(&dtl->lock);
	dtl_stop(dtl);
	kmem_cache_free(dtl_cache, dtl->buf);
	dtl->buf = NULL;
	dtl->buf_entries = 0;
	spin_unlock(&dtl->lock);
}

/*                */

static int dtl_file_open(struct inode *inode, struct file *filp)
{
	struct dtl *dtl = inode->i_private;
	int rc;

	rc = dtl_enable(dtl);
	if (rc)
		return rc;

	filp->private_data = dtl;
	return 0;
}

static int dtl_file_release(struct inode *inode, struct file *filp)
{
	struct dtl *dtl = inode->i_private;
	dtl_disable(dtl);
	return 0;
}

static ssize_t dtl_file_read(struct file *filp, char __user *buf, size_t len,
		loff_t *pos)
{
	long int rc, n_read, n_req, read_size;
	struct dtl *dtl;
	u64 cur_idx, last_idx, i;

	if ((len % sizeof(struct dtl_entry)) != 0)
		return -EINVAL;

	dtl = filp->private_data;

	/*                                     */
	n_req = len / sizeof(struct dtl_entry);

	/*                               */
	n_read = 0;

	spin_lock(&dtl->lock);

	cur_idx = dtl_current_index(dtl);
	last_idx = dtl->last_idx;

	if (last_idx + dtl->buf_entries <= cur_idx)
		last_idx = cur_idx - dtl->buf_entries + 1;

	if (last_idx + n_req > cur_idx)
		n_req = cur_idx - last_idx;

	if (n_req > 0)
		dtl->last_idx = last_idx + n_req;

	spin_unlock(&dtl->lock);

	if (n_req <= 0)
		return 0;

	i = last_idx % dtl->buf_entries;

	/*                                              */
	if (i + n_req > dtl->buf_entries) {
		read_size = dtl->buf_entries - i;

		rc = copy_to_user(buf, &dtl->buf[i],
				read_size * sizeof(struct dtl_entry));
		if (rc)
			return -EFAULT;

		i = 0;
		n_req -= read_size;
		n_read += read_size;
		buf += read_size * sizeof(struct dtl_entry);
	}

	/*                     */
	rc = copy_to_user(buf, &dtl->buf[i], n_req * sizeof(struct dtl_entry));
	if (rc)
		return -EFAULT;

	n_read += n_req;

	return n_read * sizeof(struct dtl_entry);
}

static const struct file_operations dtl_fops = {
	.open		= dtl_file_open,
	.release	= dtl_file_release,
	.read		= dtl_file_read,
	.llseek		= no_llseek,
};

static struct dentry *dtl_dir;

static int dtl_setup_file(struct dtl *dtl)
{
	char name[10];

	sprintf(name, "cpu-%d", dtl->cpu);

	dtl->file = debugfs_create_file(name, 0400, dtl_dir, dtl, &dtl_fops);
	if (!dtl->file)
		return -ENOMEM;

	return 0;
}

static int dtl_init(void)
{
	struct dentry *event_mask_file, *buf_entries_file;
	int rc, i;

	if (!firmware_has_feature(FW_FEATURE_SPLPAR))
		return -ENODEV;

	/*                                 */

	rc = -ENOMEM;
	dtl_dir = debugfs_create_dir("dtl", powerpc_debugfs_root);
	if (!dtl_dir) {
		printk(KERN_WARNING "%s: can't create dtl root dir\n",
				__func__);
		goto err;
	}

	event_mask_file = debugfs_create_x8("dtl_event_mask", 0600,
				dtl_dir, &dtl_event_mask);
	buf_entries_file = debugfs_create_u32("dtl_buf_entries", 0400,
				dtl_dir, &dtl_buf_entries);

	if (!event_mask_file || !buf_entries_file) {
		printk(KERN_WARNING "%s: can't create dtl files\n", __func__);
		goto err_remove_dir;
	}

	/*                                   */
	for_each_possible_cpu(i) {
		struct dtl *dtl = &per_cpu(cpu_dtl, i);
		spin_lock_init(&dtl->lock);
		dtl->cpu = i;

		rc = dtl_setup_file(dtl);
		if (rc)
			goto err_remove_dir;
	}

	return 0;

err_remove_dir:
	debugfs_remove_recursive(dtl_dir);
err:
	return rc;
}
arch_initcall(dtl_init);
