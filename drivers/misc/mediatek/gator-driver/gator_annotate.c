/**
 * Copyright (C) ARM Limited 2010-2014. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <asm/current.h>
#include <linux/spinlock.h>

static DEFINE_SPINLOCK(annotate_lock);
static bool collect_annotations;

static int annotate_copy(struct file *file, char const __user *buf, size_t count)
{
	int cpu = 0;
	int write = per_cpu(gator_buffer_write, cpu)[ANNOTATE_BUF];

	if (file == NULL) {
		/*                  */
		memcpy(&per_cpu(gator_buffer, cpu)[ANNOTATE_BUF][write], buf, count);
	} else {
		/*                      */
		if (copy_from_user(&per_cpu(gator_buffer, cpu)[ANNOTATE_BUF][write], buf, count) != 0)
			return -1;
	}
	per_cpu(gator_buffer_write, cpu)[ANNOTATE_BUF] = (write + count) & gator_buffer_mask[ANNOTATE_BUF];

	return 0;
}

static ssize_t annotate_write(struct file *file, char const __user *buf, size_t count_orig, loff_t *offset)
{
	int pid, cpu, header_size, available, contiguous, length1, length2, size, count = count_orig & 0x7fffffff;
	bool interrupt_context;

	if (*offset)
		return -EINVAL;

	interrupt_context = in_interrupt();
	/*                                                                 
                                                                    
                                                                     
         
  */
	if (interrupt_context) {
		pr_warning("gator: Annotations are not supported in interrupt context. Edit gator_annotate.c in the gator driver to enable annotations in interrupt context.\n");
		return -EINVAL;
	}

 retry:
	/*                                                        */
	spin_lock(&annotate_lock);

	if (!collect_annotations) {
		/*                                                                    */
		size = count_orig;
		goto annotate_write_out;
	}

	/*                                                                                         */
	cpu = 0;

	if (current == NULL)
		pid = 0;
	else
		pid = current->pid;

	/*                                     */
	header_size = MAXSIZE_PACK32 * 3 + MAXSIZE_PACK64;
	available = buffer_bytes_available(cpu, ANNOTATE_BUF) - header_size;
	size = count < available ? count : available;

	if (size <= 0) {
		/*                                               */
		spin_unlock(&annotate_lock);

		/*                                                                     */
		if (interrupt_context)
			return -EINVAL;

		wait_event_interruptible(gator_annotate_wait, buffer_bytes_available(cpu, ANNOTATE_BUF) > header_size || !collect_annotations);

		/*                                     */
		if (signal_pending(current))
			return -EINTR;

		goto retry;
	}

	/*                                                          */
	if (per_cpu(gator_buffer, cpu)[ANNOTATE_BUF]) {
		u64 time = gator_get_time();

		gator_buffer_write_packed_int(cpu, ANNOTATE_BUF, get_physical_cpu());
		gator_buffer_write_packed_int(cpu, ANNOTATE_BUF, pid);
		gator_buffer_write_packed_int64(cpu, ANNOTATE_BUF, time);
		gator_buffer_write_packed_int(cpu, ANNOTATE_BUF, size);

		/*                                                                   */
		contiguous = contiguous_space_available(cpu, ANNOTATE_BUF);
		if (size < contiguous) {
			length1 = size;
			length2 = 0;
		} else {
			length1 = contiguous;
			length2 = size - contiguous;
		}

		if (annotate_copy(file, buf, length1) != 0) {
			size = -EINVAL;
			goto annotate_write_out;
		}

		if (length2 > 0 && annotate_copy(file, &buf[length1], length2) != 0) {
			size = -EINVAL;
			goto annotate_write_out;
		}

		/*                                                                  */
		buffer_check(cpu, ANNOTATE_BUF, time);
	}

annotate_write_out:
	spin_unlock(&annotate_lock);

	/*                                    */
	return size;
}

#include "gator_annotate_kernel.c"

static int annotate_release(struct inode *inode, struct file *file)
{
	int cpu = 0;

	/*                           */
	spin_lock(&annotate_lock);

	if (per_cpu(gator_buffer, cpu)[ANNOTATE_BUF] && buffer_check_space(cpu, ANNOTATE_BUF, MAXSIZE_PACK64 + 3 * MAXSIZE_PACK32)) {
		uint32_t pid = current->pid;

		gator_buffer_write_packed_int(cpu, ANNOTATE_BUF, get_physical_cpu());
		gator_buffer_write_packed_int(cpu, ANNOTATE_BUF, pid);
		/*      */
		gator_buffer_write_packed_int64(cpu, ANNOTATE_BUF, 0);
		/*      */
		gator_buffer_write_packed_int(cpu, ANNOTATE_BUF, 0);
	}

	/*                                                                  */
	buffer_check(cpu, ANNOTATE_BUF, gator_get_time());

	spin_unlock(&annotate_lock);

	return 0;
}

static const struct file_operations annotate_fops = {
	.write = annotate_write,
	.release = annotate_release
};

static int gator_annotate_create_files(struct super_block *sb, struct dentry *root)
{
	return gatorfs_create_file_perm(sb, root, "annotate", &annotate_fops, 0666);
}

static int gator_annotate_start(void)
{
	collect_annotations = true;
	return 0;
}

static void gator_annotate_stop(void)
{
	/*                                                                                                        */
	spin_lock(&annotate_lock);
	collect_annotations = false;
	wake_up(&gator_annotate_wait);
	spin_unlock(&annotate_lock);
}
