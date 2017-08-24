/*
 * bitmap.c two-level bitmap (C) Peter T. Breuer (ptb@ot.uc3m.es) 2003
 *
 * bitmap_create  - sets up the bitmap structure
 * bitmap_destroy - destroys the bitmap structure
 *
 * additions, Copyright (C) 2003-2004, Paul Clements, SteelEye Technology, Inc.:
 * - added disk storage for bitmap
 * - changes to allow various bitmap chunk sizes
 */

/*
               
  
                                                                     
 */

#include <linux/blkdev.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/file.h>
#include <linux/mount.h>
#include <linux/buffer_head.h>
#include <linux/seq_file.h>
#include "md.h"
#include "bitmap.h"

static inline char *bmname(struct bitmap *bitmap)
{
	return bitmap->mddev ? mdname(bitmap->mddev) : "mdX";
}

/*
                                                                                
  
                                                                           
                                                                       
                                        
  
                                                                         
                                 
 */
static int bitmap_checkpage(struct bitmap_counts *bitmap,
			    unsigned long page, int create)
__releases(bitmap->lock)
__acquires(bitmap->lock)
{
	unsigned char *mappage;

	if (page >= bitmap->pages) {
		/*                                                 
                                                  
                    
   */
		return -EINVAL;
	}

	if (bitmap->bp[page].hijacked) /*                                   */
		return 0;

	if (bitmap->bp[page].map) /*                                        */
		return 0;

	if (!create)
		return -ENOENT;

	/*                                      */

	spin_unlock_irq(&bitmap->lock);
	mappage = kzalloc(PAGE_SIZE, GFP_NOIO);
	spin_lock_irq(&bitmap->lock);

	if (mappage == NULL) {
		pr_debug("md/bitmap: map page allocation failed, hijacking\n");
		/*                                                      
                          */
		if (!bitmap->bp[page].map)
			bitmap->bp[page].hijacked = 1;
	} else if (bitmap->bp[page].map ||
		   bitmap->bp[page].hijacked) {
		/*                                      */
		kfree(mappage);
		return 0;
	} else {

		/*                                                     */

		bitmap->bp[page].map = mappage;
		bitmap->missing_pages--;
	}
	return 0;
}

/*                                                                          */
/*                                                                         */
/*                                             */
static void bitmap_checkfree(struct bitmap_counts *bitmap, unsigned long page)
{
	char *ptr;

	if (bitmap->bp[page].count) /*                    */
		return;

	/*                                              */

	if (bitmap->bp[page].hijacked) { /*                                  */
		bitmap->bp[page].hijacked = 0;
		bitmap->bp[page].map = NULL;
	} else {
		/*                            */
		ptr = bitmap->bp[page].map;
		bitmap->bp[page].map = NULL;
		bitmap->missing_pages++;
		kfree(ptr);
	}
}

/*
                                                                           
 */

/*
                            
 */

/*                                                          */
static int read_sb_page(struct mddev *mddev, loff_t offset,
			struct page *page,
			unsigned long index, int size)
{
	/*                                                 */

	struct md_rdev *rdev;
	sector_t target;

	rdev_for_each(rdev, mddev) {
		if (! test_bit(In_sync, &rdev->flags)
		    || test_bit(Faulty, &rdev->flags))
			continue;

		target = offset + index * (PAGE_SIZE/512);

		if (sync_page_io(rdev, target,
				 roundup(size, bdev_logical_block_size(rdev->bdev)),
				 page, READ, true)) {
			page->index = index;
			return 0;
		}
	}
	return -EIO;
}

static struct md_rdev *next_active_rdev(struct md_rdev *rdev, struct mddev *mddev)
{
	/*                                                                  
                                                                        
                                      
                                                                  
                                                                    
                                                              
                                     
  */
	rcu_read_lock();
	if (rdev == NULL)
		/*                        */
		rdev = list_entry_rcu(&mddev->disks, struct md_rdev, same_set);
	else {
		/*                                                 */
		rdev_dec_pending(rdev, mddev);
	}
	list_for_each_entry_continue_rcu(rdev, &mddev->disks, same_set) {
		if (rdev->raid_disk >= 0 &&
		    !test_bit(Faulty, &rdev->flags)) {
			/*                          */
			atomic_inc(&rdev->nr_pending);
			rcu_read_unlock();
			return rdev;
		}
	}
	rcu_read_unlock();
	return NULL;
}

static int write_sb_page(struct bitmap *bitmap, struct page *page, int wait)
{
	struct md_rdev *rdev = NULL;
	struct block_device *bdev;
	struct mddev *mddev = bitmap->mddev;
	struct bitmap_storage *store = &bitmap->storage;

	while ((rdev = next_active_rdev(rdev, mddev)) != NULL) {
		int size = PAGE_SIZE;
		loff_t offset = mddev->bitmap_info.offset;

		bdev = (rdev->meta_bdev) ? rdev->meta_bdev : rdev->bdev;

		if (page->index == store->file_pages-1) {
			int last_page_size = store->bytes & (PAGE_SIZE-1);
			if (last_page_size == 0)
				last_page_size = PAGE_SIZE;
			size = roundup(last_page_size,
				       bdev_logical_block_size(bdev));
		}
		/*                                            
             
   */
		if (mddev->external) {
			/*                           */
			if (rdev->sb_start + offset + (page->index
						       * (PAGE_SIZE/512))
			    > rdev->data_offset
			    &&
			    rdev->sb_start + offset
			    < (rdev->data_offset + mddev->dev_sectors
			     + (PAGE_SIZE/512)))
				goto bad_alignment;
		} else if (offset < 0) {
			/*                        */
			if (offset
			    + (long)(page->index * (PAGE_SIZE/512))
			    + size/512 > 0)
				/*                            */
				goto bad_alignment;
			if (rdev->data_offset + mddev->dev_sectors
			    > rdev->sb_start + offset)
				/*                        */
				goto bad_alignment;
		} else if (rdev->sb_start < rdev->data_offset) {
			/*                      */
			if (rdev->sb_start
			    + offset
			    + page->index*(PAGE_SIZE/512) + size/512
			    > rdev->data_offset)
				/*                        */
				goto bad_alignment;
		} else {
			/*                                    */
		}
		md_super_write(mddev, rdev,
			       rdev->sb_start + offset
			       + page->index * (PAGE_SIZE/512),
			       size,
			       page);
	}

	if (wait)
		md_super_wait(mddev);
	return 0;

 bad_alignment:
	return -EINVAL;
}

static void bitmap_file_kick(struct bitmap *bitmap);
/*
                             
 */
static void write_page(struct bitmap *bitmap, struct page *page, int wait)
{
	struct buffer_head *bh;

	if (bitmap->storage.file == NULL) {
		switch (write_sb_page(bitmap, page, wait)) {
		case -EINVAL:
			set_bit(BITMAP_WRITE_ERROR, &bitmap->flags);
		}
	} else {

		bh = page_buffers(page);

		while (bh && bh->b_blocknr) {
			atomic_inc(&bitmap->pending_writes);
			set_buffer_locked(bh);
			set_buffer_mapped(bh);
			submit_bh(WRITE | REQ_SYNC, bh);
			bh = bh->b_this_page;
		}

		if (wait)
			wait_event(bitmap->write_wait,
				   atomic_read(&bitmap->pending_writes)==0);
	}
	if (test_bit(BITMAP_WRITE_ERROR, &bitmap->flags))
		bitmap_file_kick(bitmap);
}

static void end_bitmap_write(struct buffer_head *bh, int uptodate)
{
	struct bitmap *bitmap = bh->b_private;

	if (!uptodate)
		set_bit(BITMAP_WRITE_ERROR, &bitmap->flags);
	if (atomic_dec_and_test(&bitmap->pending_writes))
		wake_up(&bitmap->write_wait);
}

/*                      */
static void
__clear_page_buffers(struct page *page)
{
	ClearPagePrivate(page);
	set_page_private(page, 0);
	page_cache_release(page);
}
static void free_buffers(struct page *page)
{
	struct buffer_head *bh;

	if (!PagePrivate(page))
		return;

	bh = page_buffers(page);
	while (bh) {
		struct buffer_head *next = bh->b_this_page;
		free_buffer_head(bh);
		bh = next;
	}
	__clear_page_buffers(page);
	put_page(page);
}

/*                         
                                                                      
                                                                    
                                                                 
                                                                     
                                                                    
 */
static int read_page(struct file *file, unsigned long index,
		     struct bitmap *bitmap,
		     unsigned long count,
		     struct page *page)
{
	int ret = 0;
	struct inode *inode = file_inode(file);
	struct buffer_head *bh;
	sector_t block;

	pr_debug("read bitmap file (%dB @ %llu)\n", (int)PAGE_SIZE,
		 (unsigned long long)index << PAGE_SHIFT);

	bh = alloc_page_buffers(page, 1<<inode->i_blkbits, 0);
	if (!bh) {
		ret = -ENOMEM;
		goto out;
	}
	attach_page_buffers(page, bh);
	block = index << (PAGE_SHIFT - inode->i_blkbits);
	while (bh) {
		if (count == 0)
			bh->b_blocknr = 0;
		else {
			bh->b_blocknr = bmap(inode, block);
			if (bh->b_blocknr == 0) {
				/*                       */
				ret = -EINVAL;
				goto out;
			}
			bh->b_bdev = inode->i_sb->s_bdev;
			if (count < (1<<inode->i_blkbits))
				count = 0;
			else
				count -= (1<<inode->i_blkbits);

			bh->b_end_io = end_bitmap_write;
			bh->b_private = bitmap;
			atomic_inc(&bitmap->pending_writes);
			set_buffer_locked(bh);
			set_buffer_mapped(bh);
			submit_bh(READ, bh);
		}
		block++;
		bh = bh->b_this_page;
	}
	page->index = index;

	wait_event(bitmap->write_wait,
		   atomic_read(&bitmap->pending_writes)==0);
	if (test_bit(BITMAP_WRITE_ERROR, &bitmap->flags))
		ret = -EIO;
out:
	if (ret)
		printk(KERN_ALERT "md: bitmap read error: (%dB @ %llu): %d\n",
			(int)PAGE_SIZE,
			(unsigned long long)index << PAGE_SHIFT,
			ret);
	return ret;
}

/*
                                    
 */

/*                                                          */
void bitmap_update_sb(struct bitmap *bitmap)
{
	bitmap_super_t *sb;

	if (!bitmap || !bitmap->mddev) /*                          */
		return;
	if (bitmap->mddev->bitmap_info.external)
		return;
	if (!bitmap->storage.sb_page) /*               */
		return;
	sb = kmap_atomic(bitmap->storage.sb_page);
	sb->events = cpu_to_le64(bitmap->mddev->events);
	if (bitmap->mddev->events < bitmap->events_cleared)
		/*                           */
		bitmap->events_cleared = bitmap->mddev->events;
	sb->events_cleared = cpu_to_le64(bitmap->events_cleared);
	sb->state = cpu_to_le32(bitmap->flags);
	/*                                                 */
	sb->daemon_sleep = cpu_to_le32(bitmap->mddev->bitmap_info.daemon_sleep/HZ);
	sb->write_behind = cpu_to_le32(bitmap->mddev->bitmap_info.max_write_behind);
	/*                                           */
	sb->sync_size = cpu_to_le64(bitmap->mddev->resync_max_sectors);
	sb->chunksize = cpu_to_le32(bitmap->mddev->bitmap_info.chunksize);
	sb->sectors_reserved = cpu_to_le32(bitmap->mddev->
					   bitmap_info.space);
	kunmap_atomic(sb);
	write_page(bitmap, bitmap->storage.sb_page, 1);
}

/*                                      */
void bitmap_print_sb(struct bitmap *bitmap)
{
	bitmap_super_t *sb;

	if (!bitmap || !bitmap->storage.sb_page)
		return;
	sb = kmap_atomic(bitmap->storage.sb_page);
	printk(KERN_DEBUG "%s: bitmap file superblock:\n", bmname(bitmap));
	printk(KERN_DEBUG "         magic: %08x\n", le32_to_cpu(sb->magic));
	printk(KERN_DEBUG "       version: %d\n", le32_to_cpu(sb->version));
	printk(KERN_DEBUG "          uuid: %08x.%08x.%08x.%08x\n",
					*(__u32 *)(sb->uuid+0),
					*(__u32 *)(sb->uuid+4),
					*(__u32 *)(sb->uuid+8),
					*(__u32 *)(sb->uuid+12));
	printk(KERN_DEBUG "        events: %llu\n",
			(unsigned long long) le64_to_cpu(sb->events));
	printk(KERN_DEBUG "events cleared: %llu\n",
			(unsigned long long) le64_to_cpu(sb->events_cleared));
	printk(KERN_DEBUG "         state: %08x\n", le32_to_cpu(sb->state));
	printk(KERN_DEBUG "     chunksize: %d B\n", le32_to_cpu(sb->chunksize));
	printk(KERN_DEBUG "  daemon sleep: %ds\n", le32_to_cpu(sb->daemon_sleep));
	printk(KERN_DEBUG "     sync size: %llu KB\n",
			(unsigned long long)le64_to_cpu(sb->sync_size)/2);
	printk(KERN_DEBUG "max write behind: %d\n", le32_to_cpu(sb->write_behind));
	kunmap_atomic(sb);
}

/*
                     
          
  
                                                                           
                                                                              
                                                                        
                                             
  
                                        
 */
static int bitmap_new_disk_sb(struct bitmap *bitmap)
{
	bitmap_super_t *sb;
	unsigned long chunksize, daemon_sleep, write_behind;

	bitmap->storage.sb_page = alloc_page(GFP_KERNEL);
	if (bitmap->storage.sb_page == NULL)
		return -ENOMEM;
	bitmap->storage.sb_page->index = 0;

	sb = kmap_atomic(bitmap->storage.sb_page);

	sb->magic = cpu_to_le32(BITMAP_MAGIC);
	sb->version = cpu_to_le32(BITMAP_MAJOR_HI);

	chunksize = bitmap->mddev->bitmap_info.chunksize;
	BUG_ON(!chunksize);
	if (!is_power_of_2(chunksize)) {
		kunmap_atomic(sb);
		printk(KERN_ERR "bitmap chunksize not a power of 2\n");
		return -EINVAL;
	}
	sb->chunksize = cpu_to_le32(chunksize);

	daemon_sleep = bitmap->mddev->bitmap_info.daemon_sleep;
	if (!daemon_sleep ||
	    (daemon_sleep < 1) || (daemon_sleep > MAX_SCHEDULE_TIMEOUT)) {
		printk(KERN_INFO "Choosing daemon_sleep default (5 sec)\n");
		daemon_sleep = 5 * HZ;
	}
	sb->daemon_sleep = cpu_to_le32(daemon_sleep);
	bitmap->mddev->bitmap_info.daemon_sleep = daemon_sleep;

	/*
                                                          
                                                             
  */
	write_behind = bitmap->mddev->bitmap_info.max_write_behind;
	if (write_behind > COUNTER_MAX)
		write_behind = COUNTER_MAX / 2;
	sb->write_behind = cpu_to_le32(write_behind);
	bitmap->mddev->bitmap_info.max_write_behind = write_behind;

	/*                                                               */
	sb->sync_size = cpu_to_le64(bitmap->mddev->resync_max_sectors);

	memcpy(sb->uuid, bitmap->mddev->uuid, 16);

	set_bit(BITMAP_STALE, &bitmap->flags);
	sb->state = cpu_to_le32(bitmap->flags);
	bitmap->events_cleared = bitmap->mddev->events;
	sb->events_cleared = cpu_to_le64(bitmap->mddev->events);

	kunmap_atomic(sb);

	return 0;
}

/*                                                                            */
static int bitmap_read_sb(struct bitmap *bitmap)
{
	char *reason = NULL;
	bitmap_super_t *sb;
	unsigned long chunksize, daemon_sleep, write_behind;
	unsigned long long events;
	unsigned long sectors_reserved = 0;
	int err = -EINVAL;
	struct page *sb_page;

	if (!bitmap->storage.file && !bitmap->mddev->bitmap_info.offset) {
		chunksize = 128 * 1024 * 1024;
		daemon_sleep = 5 * HZ;
		write_behind = 0;
		set_bit(BITMAP_STALE, &bitmap->flags);
		err = 0;
		goto out_no_sb;
	}
	/*                                      */
	sb_page = alloc_page(GFP_KERNEL);
	if (!sb_page)
		return -ENOMEM;
	bitmap->storage.sb_page = sb_page;

	if (bitmap->storage.file) {
		loff_t isize = i_size_read(bitmap->storage.file->f_mapping->host);
		int bytes = isize > PAGE_SIZE ? PAGE_SIZE : isize;

		err = read_page(bitmap->storage.file, 0,
				bitmap, bytes, sb_page);
	} else {
		err = read_sb_page(bitmap->mddev,
				   bitmap->mddev->bitmap_info.offset,
				   sb_page,
				   0, sizeof(bitmap_super_t));
	}
	if (err)
		return err;

	sb = kmap_atomic(sb_page);

	chunksize = le32_to_cpu(sb->chunksize);
	daemon_sleep = le32_to_cpu(sb->daemon_sleep) * HZ;
	write_behind = le32_to_cpu(sb->write_behind);
	sectors_reserved = le32_to_cpu(sb->sectors_reserved);

	/*                                                  */
	if (sb->magic != cpu_to_le32(BITMAP_MAGIC))
		reason = "bad magic";
	else if (le32_to_cpu(sb->version) < BITMAP_MAJOR_LO ||
		 le32_to_cpu(sb->version) > BITMAP_MAJOR_HI)
		reason = "unrecognized superblock version";
	else if (chunksize < 512)
		reason = "bitmap chunksize too small";
	else if (!is_power_of_2(chunksize))
		reason = "bitmap chunksize not a power of 2";
	else if (daemon_sleep < 1 || daemon_sleep > MAX_SCHEDULE_TIMEOUT)
		reason = "daemon sleep period out of range";
	else if (write_behind > COUNTER_MAX)
		reason = "write-behind limit out of range (0 - 16383)";
	if (reason) {
		printk(KERN_INFO "%s: invalid bitmap file superblock: %s\n",
			bmname(bitmap), reason);
		goto out;
	}

	/*                                                               */
	sb->sync_size = cpu_to_le64(bitmap->mddev->resync_max_sectors);

	if (bitmap->mddev->persistent) {
		/*
                                                          
                                                   
   */
		if (memcmp(sb->uuid, bitmap->mddev->uuid, 16)) {
			printk(KERN_INFO
			       "%s: bitmap superblock UUID mismatch\n",
			       bmname(bitmap));
			goto out;
		}
		events = le64_to_cpu(sb->events);
		if (events < bitmap->mddev->events) {
			printk(KERN_INFO
			       "%s: bitmap file is out of date (%llu < %llu) "
			       "-- forcing full recovery\n",
			       bmname(bitmap), events,
			       (unsigned long long) bitmap->mddev->events);
			set_bit(BITMAP_STALE, &bitmap->flags);
		}
	}

	/*                                            */
	bitmap->flags |= le32_to_cpu(sb->state);
	if (le32_to_cpu(sb->version) == BITMAP_MAJOR_HOSTENDIAN)
		set_bit(BITMAP_HOSTENDIAN, &bitmap->flags);
	bitmap->events_cleared = le64_to_cpu(sb->events_cleared);
	err = 0;
out:
	kunmap_atomic(sb);
out_no_sb:
	if (test_bit(BITMAP_STALE, &bitmap->flags))
		bitmap->events_cleared = bitmap->mddev->events;
	bitmap->mddev->bitmap_info.chunksize = chunksize;
	bitmap->mddev->bitmap_info.daemon_sleep = daemon_sleep;
	bitmap->mddev->bitmap_info.max_write_behind = write_behind;
	if (bitmap->mddev->bitmap_info.space == 0 ||
	    bitmap->mddev->bitmap_info.space > sectors_reserved)
		bitmap->mddev->bitmap_info.space = sectors_reserved;
	if (err)
		bitmap_print_sb(bitmap);
	return err;
}

/*
                                 
 */

/*
                  
  
                                                                        
                                                                        
 */
/*                                                        */
static inline unsigned long file_page_index(struct bitmap_storage *store,
					    unsigned long chunk)
{
	if (store->sb_page)
		chunk += sizeof(bitmap_super_t) << 3;
	return chunk >> PAGE_BIT_SHIFT;
}

/*                                                      */
static inline unsigned long file_page_offset(struct bitmap_storage *store,
					     unsigned long chunk)
{
	if (store->sb_page)
		chunk += sizeof(bitmap_super_t) << 3;
	return chunk & (PAGE_BITS - 1);
}

/*
                                                                          
  
                                                                             
                                                                              
              
 */
static inline struct page *filemap_get_page(struct bitmap_storage *store,
					    unsigned long chunk)
{
	if (file_page_index(store, chunk) >= store->file_pages)
		return NULL;
	return store->filemap[file_page_index(store, chunk)
			      - file_page_index(store, 0)];
}

static int bitmap_storage_alloc(struct bitmap_storage *store,
				unsigned long chunks, int with_super)
{
	int pnum;
	unsigned long num_pages;
	unsigned long bytes;

	bytes = DIV_ROUND_UP(chunks, 8);
	if (with_super)
		bytes += sizeof(bitmap_super_t);

	num_pages = DIV_ROUND_UP(bytes, PAGE_SIZE);

	store->filemap = kmalloc(sizeof(struct page *)
				 * num_pages, GFP_KERNEL);
	if (!store->filemap)
		return -ENOMEM;

	if (with_super && !store->sb_page) {
		store->sb_page = alloc_page(GFP_KERNEL|__GFP_ZERO);
		if (store->sb_page == NULL)
			return -ENOMEM;
		store->sb_page->index = 0;
	}
	pnum = 0;
	if (store->sb_page) {
		store->filemap[0] = store->sb_page;
		pnum = 1;
	}
	for ( ; pnum < num_pages; pnum++) {
		store->filemap[pnum] = alloc_page(GFP_KERNEL|__GFP_ZERO);
		if (!store->filemap[pnum]) {
			store->file_pages = pnum;
			return -ENOMEM;
		}
		store->filemap[pnum]->index = pnum;
	}
	store->file_pages = pnum;

	/*                                                  
                             */
	store->filemap_attr = kzalloc(
		roundup(DIV_ROUND_UP(num_pages*4, 8), sizeof(unsigned long)),
		GFP_KERNEL);
	if (!store->filemap_attr)
		return -ENOMEM;

	store->bytes = bytes;

	return 0;
}

static void bitmap_file_unmap(struct bitmap_storage *store)
{
	struct page **map, *sb_page;
	int pages;
	struct file *file;

	file = store->file;
	map = store->filemap;
	pages = store->file_pages;
	sb_page = store->sb_page;

	while (pages--)
		if (map[pages] != sb_page) /*                                */
			free_buffers(map[pages]);
	kfree(map);
	kfree(store->filemap_attr);

	if (sb_page)
		free_buffers(sb_page);

	if (file) {
		struct inode *inode = file_inode(file);
		invalidate_mapping_pages(inode->i_mapping, 0, -1);
		fput(file);
	}
}

/*
                                                                           
                                                                          
                              
 */
static void bitmap_file_kick(struct bitmap *bitmap)
{
	char *path, *ptr = NULL;

	if (!test_and_set_bit(BITMAP_STALE, &bitmap->flags)) {
		bitmap_update_sb(bitmap);

		if (bitmap->storage.file) {
			path = kmalloc(PAGE_SIZE, GFP_KERNEL);
			if (path)
				ptr = d_path(&bitmap->storage.file->f_path,
					     path, PAGE_SIZE);

			printk(KERN_ALERT
			      "%s: kicking failed bitmap file %s from array!\n",
			      bmname(bitmap), IS_ERR(ptr) ? "" : ptr);

			kfree(path);
		} else
			printk(KERN_ALERT
			       "%s: disabling internal bitmap due to errors\n",
			       bmname(bitmap));
	}
}

enum bitmap_page_attr {
	BITMAP_PAGE_DIRTY = 0,     /*                                           */
	BITMAP_PAGE_PENDING = 1,   /*                                       
                                  */
	BITMAP_PAGE_NEEDWRITE = 2, /*                                               */
};

static inline void set_page_attr(struct bitmap *bitmap, int pnum,
				 enum bitmap_page_attr attr)
{
	set_bit((pnum<<2) + attr, bitmap->storage.filemap_attr);
}

static inline void clear_page_attr(struct bitmap *bitmap, int pnum,
				   enum bitmap_page_attr attr)
{
	clear_bit((pnum<<2) + attr, bitmap->storage.filemap_attr);
}

static inline int test_page_attr(struct bitmap *bitmap, int pnum,
				 enum bitmap_page_attr attr)
{
	return test_bit((pnum<<2) + attr, bitmap->storage.filemap_attr);
}

static inline int test_and_clear_page_attr(struct bitmap *bitmap, int pnum,
					   enum bitmap_page_attr attr)
{
	return test_and_clear_bit((pnum<<2) + attr,
				  bitmap->storage.filemap_attr);
}
/*
                                                                           
                                                                   
  
                                                                     
                                                                  
 */
static void bitmap_file_set_bit(struct bitmap *bitmap, sector_t block)
{
	unsigned long bit;
	struct page *page;
	void *kaddr;
	unsigned long chunk = block >> bitmap->counts.chunkshift;

	page = filemap_get_page(&bitmap->storage, chunk);
	if (!page)
		return;
	bit = file_page_offset(&bitmap->storage, chunk);

	/*             */
	kaddr = kmap_atomic(page);
	if (test_bit(BITMAP_HOSTENDIAN, &bitmap->flags))
		set_bit(bit, kaddr);
	else
		set_bit_le(bit, kaddr);
	kunmap_atomic(kaddr);
	pr_debug("set file bit %lu page %lu\n", bit, page->index);
	/*                                                                  */
	set_page_attr(bitmap, page->index, BITMAP_PAGE_DIRTY);
}

static void bitmap_file_clear_bit(struct bitmap *bitmap, sector_t block)
{
	unsigned long bit;
	struct page *page;
	void *paddr;
	unsigned long chunk = block >> bitmap->counts.chunkshift;

	page = filemap_get_page(&bitmap->storage, chunk);
	if (!page)
		return;
	bit = file_page_offset(&bitmap->storage, chunk);
	paddr = kmap_atomic(page);
	if (test_bit(BITMAP_HOSTENDIAN, &bitmap->flags))
		clear_bit(bit, paddr);
	else
		clear_bit_le(bit, paddr);
	kunmap_atomic(paddr);
	if (!test_page_attr(bitmap, page->index, BITMAP_PAGE_NEEDWRITE)) {
		set_page_attr(bitmap, page->index, BITMAP_PAGE_PENDING);
		bitmap->allclean = 0;
	}
}

/*                                                                      
                                                                        
                                                   */
void bitmap_unplug(struct bitmap *bitmap)
{
	unsigned long i;
	int dirty, need_write;
	int wait = 0;

	if (!bitmap || !bitmap->storage.filemap ||
	    test_bit(BITMAP_STALE, &bitmap->flags))
		return;

	/*                                                                   
                        */
	for (i = 0; i < bitmap->storage.file_pages; i++) {
		if (!bitmap->storage.filemap)
			return;
		dirty = test_and_clear_page_attr(bitmap, i, BITMAP_PAGE_DIRTY);
		need_write = test_and_clear_page_attr(bitmap, i,
						      BITMAP_PAGE_NEEDWRITE);
		if (dirty || need_write) {
			clear_page_attr(bitmap, i, BITMAP_PAGE_PENDING);
			write_page(bitmap, bitmap->storage.filemap[i], 0);
		}
		if (dirty)
			wait = 1;
	}
	if (wait) { /*                                                       */
		if (bitmap->storage.file)
			wait_event(bitmap->write_wait,
				   atomic_read(&bitmap->pending_writes)==0);
		else
			md_super_wait(bitmap->mddev);
	}
	if (test_bit(BITMAP_WRITE_ERROR, &bitmap->flags))
		bitmap_file_kick(bitmap);
}
EXPORT_SYMBOL(bitmap_unplug);

static void bitmap_set_memory_bits(struct bitmap *bitmap, sector_t offset, int needed);
/*                                                                      
                                                                    
                                    
                 
                                                              
                                                              
                                         
  
                                                                
                                                     
 */
static int bitmap_init_from_disk(struct bitmap *bitmap, sector_t start)
{
	unsigned long i, chunks, index, oldindex, bit;
	struct page *page = NULL;
	unsigned long bit_cnt = 0;
	struct file *file;
	unsigned long offset;
	int outofdate;
	int ret = -ENOSPC;
	void *paddr;
	struct bitmap_storage *store = &bitmap->storage;

	chunks = bitmap->counts.chunks;
	file = store->file;

	if (!file && !bitmap->mddev->bitmap_info.offset) {
		/*                                       */
		store->filemap = NULL;
		store->file_pages = 0;
		for (i = 0; i < chunks ; i++) {
			/*                                            */
			int needed = ((sector_t)(i+1) << (bitmap->counts.chunkshift)
				      >= start);
			bitmap_set_memory_bits(bitmap,
					       (sector_t)i << bitmap->counts.chunkshift,
					       needed);
		}
		return 0;
	}

	outofdate = test_bit(BITMAP_STALE, &bitmap->flags);
	if (outofdate)
		printk(KERN_INFO "%s: bitmap file is out of date, doing full "
			"recovery\n", bmname(bitmap));

	if (file && i_size_read(file->f_mapping->host) < store->bytes) {
		printk(KERN_INFO "%s: bitmap file too short %lu < %lu\n",
		       bmname(bitmap),
		       (unsigned long) i_size_read(file->f_mapping->host),
		       store->bytes);
		goto err;
	}

	oldindex = ~0L;
	offset = 0;
	if (!bitmap->mddev->bitmap_info.external)
		offset = sizeof(bitmap_super_t);

	for (i = 0; i < chunks; i++) {
		int b;
		index = file_page_index(&bitmap->storage, i);
		bit = file_page_offset(&bitmap->storage, i);
		if (index != oldindex) { /*                                */
			int count;
			/*                                        */
			if (index == store->file_pages-1)
				count = store->bytes - index * PAGE_SIZE;
			else
				count = PAGE_SIZE;
			page = store->filemap[index];
			if (file)
				ret = read_page(file, index, bitmap,
						count, page);
			else
				ret = read_sb_page(
					bitmap->mddev,
					bitmap->mddev->bitmap_info.offset,
					page,
					index, count);

			if (ret)
				goto err;

			oldindex = index;

			if (outofdate) {
				/*
                                          
                                  
     */
				paddr = kmap_atomic(page);
				memset(paddr + offset, 0xff,
				       PAGE_SIZE - offset);
				kunmap_atomic(paddr);
				write_page(bitmap, page, 1);

				ret = -EIO;
				if (test_bit(BITMAP_WRITE_ERROR,
					     &bitmap->flags))
					goto err;
			}
		}
		paddr = kmap_atomic(page);
		if (test_bit(BITMAP_HOSTENDIAN, &bitmap->flags))
			b = test_bit(bit, paddr);
		else
			b = test_bit_le(bit, paddr);
		kunmap_atomic(paddr);
		if (b) {
			/*                                            */
			int needed = ((sector_t)(i+1) << bitmap->counts.chunkshift
				      >= start);
			bitmap_set_memory_bits(bitmap,
					       (sector_t)i << bitmap->counts.chunkshift,
					       needed);
			bit_cnt++;
		}
		offset = 0;
	}

	printk(KERN_INFO "%s: bitmap initialized from disk: "
	       "read %lu pages, set %lu of %lu bits\n",
	       bmname(bitmap), store->file_pages,
	       bit_cnt, chunks);

	return 0;

 err:
	printk(KERN_INFO "%s: bitmap initialisation failed: %d\n",
	       bmname(bitmap), ret);
	return ret;
}

void bitmap_write_all(struct bitmap *bitmap)
{
	/*                                                
                                           
  */
	int i;

	if (!bitmap || !bitmap->storage.filemap)
		return;
	if (bitmap->storage.file)
		/*                                  */
		return;

	for (i = 0; i < bitmap->storage.file_pages; i++)
		set_page_attr(bitmap, i,
			      BITMAP_PAGE_NEEDWRITE);
	bitmap->allclean = 0;
}

static void bitmap_count_page(struct bitmap_counts *bitmap,
			      sector_t offset, int inc)
{
	sector_t chunk = offset >> bitmap->chunkshift;
	unsigned long page = chunk >> PAGE_COUNTER_SHIFT;
	bitmap->bp[page].count += inc;
	bitmap_checkfree(bitmap, page);
}

static void bitmap_set_pending(struct bitmap_counts *bitmap, sector_t offset)
{
	sector_t chunk = offset >> bitmap->chunkshift;
	unsigned long page = chunk >> PAGE_COUNTER_SHIFT;
	struct bitmap_page *bp = &bitmap->bp[page];

	if (!bp->pending)
		bp->pending = 1;
}

static bitmap_counter_t *bitmap_get_counter(struct bitmap_counts *bitmap,
					    sector_t offset, sector_t *blocks,
					    int create);

/*
                                                                       
                
 */

void bitmap_daemon_work(struct mddev *mddev)
{
	struct bitmap *bitmap;
	unsigned long j;
	unsigned long nextpage;
	sector_t blocks;
	struct bitmap_counts *counts;

	/*                                         
                   
  */
	mutex_lock(&mddev->bitmap_info.mutex);
	bitmap = mddev->bitmap;
	if (bitmap == NULL) {
		mutex_unlock(&mddev->bitmap_info.mutex);
		return;
	}
	if (time_before(jiffies, bitmap->daemon_lastrun
			+ mddev->bitmap_info.daemon_sleep))
		goto done;

	bitmap->daemon_lastrun = jiffies;
	if (bitmap->allclean) {
		mddev->thread->timeout = MAX_SCHEDULE_TIMEOUT;
		goto done;
	}
	bitmap->allclean = 1;

	/*                                                        
                                                                    
                     
  */
	for (j = 0; j < bitmap->storage.file_pages; j++)
		if (test_and_clear_page_attr(bitmap, j,
					     BITMAP_PAGE_PENDING))
			set_page_attr(bitmap, j,
				      BITMAP_PAGE_NEEDWRITE);

	if (bitmap->need_sync &&
	    mddev->bitmap_info.external == 0) {
		/*                                         
                   */
		bitmap_super_t *sb;
		bitmap->need_sync = 0;
		if (bitmap->storage.filemap) {
			sb = kmap_atomic(bitmap->storage.sb_page);
			sb->events_cleared =
				cpu_to_le64(bitmap->events_cleared);
			kunmap_atomic(sb);
			set_page_attr(bitmap, 0,
				      BITMAP_PAGE_NEEDWRITE);
		}
	}
	/*                                                           
                                     
  */
	counts = &bitmap->counts;
	spin_lock_irq(&counts->lock);
	nextpage = 0;
	for (j = 0; j < counts->chunks; j++) {
		bitmap_counter_t *bmc;
		sector_t  block = (sector_t)j << counts->chunkshift;

		if (j == nextpage) {
			nextpage += PAGE_COUNTER_RATIO;
			if (!counts->bp[j >> PAGE_COUNTER_SHIFT].pending) {
				j |= PAGE_COUNTER_MASK;
				continue;
			}
			counts->bp[j >> PAGE_COUNTER_SHIFT].pending = 0;
		}
		bmc = bitmap_get_counter(counts,
					 block,
					 &blocks, 0);

		if (!bmc) {
			j |= PAGE_COUNTER_MASK;
			continue;
		}
		if (*bmc == 1 && !bitmap->need_sync) {
			/*                      */
			*bmc = 0;
			bitmap_count_page(counts, block, -1);
			bitmap_file_clear_bit(bitmap, block);
		} else if (*bmc && *bmc <= 2) {
			*bmc = 1;
			bitmap_set_pending(counts, block);
			bitmap->allclean = 0;
		}
	}
	spin_unlock_irq(&counts->lock);

	/*                                                              
                                                                  
             
                                                                 
                                                             
                                                                    
                                                            
  */
	for (j = 0;
	     j < bitmap->storage.file_pages
		     && !test_bit(BITMAP_STALE, &bitmap->flags);
	     j++) {

		if (test_page_attr(bitmap, j,
				   BITMAP_PAGE_DIRTY))
			/*                                    */
			break;
		if (test_and_clear_page_attr(bitmap, j,
					     BITMAP_PAGE_NEEDWRITE)) {
			write_page(bitmap, bitmap->storage.filemap[j], 0);
		}
	}

 done:
	if (bitmap->allclean == 0)
		mddev->thread->timeout =
			mddev->bitmap_info.daemon_sleep;
	mutex_unlock(&mddev->bitmap_info.mutex);
}

static bitmap_counter_t *bitmap_get_counter(struct bitmap_counts *bitmap,
					    sector_t offset, sector_t *blocks,
					    int create)
__releases(bitmap->lock)
__acquires(bitmap->lock)
{
	/*                                                       
                                                          
                                          
  */
	sector_t chunk = offset >> bitmap->chunkshift;
	unsigned long page = chunk >> PAGE_COUNTER_SHIFT;
	unsigned long pageoff = (chunk & PAGE_COUNTER_MASK) << COUNTER_BYTE_SHIFT;
	sector_t csize;
	int err;

	err = bitmap_checkpage(bitmap, page, create);

	if (bitmap->bp[page].hijacked ||
	    bitmap->bp[page].map == NULL)
		csize = ((sector_t)1) << (bitmap->chunkshift +
					  PAGE_COUNTER_SHIFT - 1);
	else
		csize = ((sector_t)1) << bitmap->chunkshift;
	*blocks = csize - (offset & (csize - 1));

	if (err < 0)
		return NULL;

	/*                */

	if (bitmap->bp[page].hijacked) { /*                  */
		/*                                                
                              */
		int hi = (pageoff > PAGE_COUNTER_MASK);
		return  &((bitmap_counter_t *)
			  &bitmap->bp[page].map)[hi];
	} else /*                   */
		return (bitmap_counter_t *)
			&(bitmap->bp[page].map[pageoff]);
}

int bitmap_startwrite(struct bitmap *bitmap, sector_t offset, unsigned long sectors, int behind)
{
	if (!bitmap)
		return 0;

	if (behind) {
		int bw;
		atomic_inc(&bitmap->behind_writes);
		bw = atomic_read(&bitmap->behind_writes);
		if (bw > bitmap->behind_writes_used)
			bitmap->behind_writes_used = bw;

		pr_debug("inc write-behind count %d/%lu\n",
			 bw, bitmap->mddev->bitmap_info.max_write_behind);
	}

	while (sectors) {
		sector_t blocks;
		bitmap_counter_t *bmc;

		spin_lock_irq(&bitmap->counts.lock);
		bmc = bitmap_get_counter(&bitmap->counts, offset, &blocks, 1);
		if (!bmc) {
			spin_unlock_irq(&bitmap->counts.lock);
			return 0;
		}

		if (unlikely(COUNTER(*bmc) == COUNTER_MAX)) {
			DEFINE_WAIT(__wait);
			/*                                               
                                                        
                   
    */
			prepare_to_wait(&bitmap->overflow_wait, &__wait,
					TASK_UNINTERRUPTIBLE);
			spin_unlock_irq(&bitmap->counts.lock);
			schedule();
			finish_wait(&bitmap->overflow_wait, &__wait);
			continue;
		}

		switch (*bmc) {
		case 0:
			bitmap_file_set_bit(bitmap, offset);
			bitmap_count_page(&bitmap->counts, offset, 1);
			/*              */
		case 1:
			*bmc = 2;
		}

		(*bmc)++;

		spin_unlock_irq(&bitmap->counts.lock);

		offset += blocks;
		if (sectors > blocks)
			sectors -= blocks;
		else
			sectors = 0;
	}
	return 0;
}
EXPORT_SYMBOL(bitmap_startwrite);

void bitmap_endwrite(struct bitmap *bitmap, sector_t offset, unsigned long sectors,
		     int success, int behind)
{
	if (!bitmap)
		return;
	if (behind) {
		if (atomic_dec_and_test(&bitmap->behind_writes))
			wake_up(&bitmap->behind_wait);
		pr_debug("dec write-behind count %d/%lu\n",
			 atomic_read(&bitmap->behind_writes),
			 bitmap->mddev->bitmap_info.max_write_behind);
	}

	while (sectors) {
		sector_t blocks;
		unsigned long flags;
		bitmap_counter_t *bmc;

		spin_lock_irqsave(&bitmap->counts.lock, flags);
		bmc = bitmap_get_counter(&bitmap->counts, offset, &blocks, 0);
		if (!bmc) {
			spin_unlock_irqrestore(&bitmap->counts.lock, flags);
			return;
		}

		if (success && !bitmap->mddev->degraded &&
		    bitmap->events_cleared < bitmap->mddev->events) {
			bitmap->events_cleared = bitmap->mddev->events;
			bitmap->need_sync = 1;
			sysfs_notify_dirent_safe(bitmap->sysfs_can_clear);
		}

		if (!success && !NEEDED(*bmc))
			*bmc |= NEEDED_MASK;

		if (COUNTER(*bmc) == COUNTER_MAX)
			wake_up(&bitmap->overflow_wait);

		(*bmc)--;
		if (*bmc <= 2) {
			bitmap_set_pending(&bitmap->counts, offset);
			bitmap->allclean = 0;
		}
		spin_unlock_irqrestore(&bitmap->counts.lock, flags);
		offset += blocks;
		if (sectors > blocks)
			sectors -= blocks;
		else
			sectors = 0;
	}
}
EXPORT_SYMBOL(bitmap_endwrite);

static int __bitmap_start_sync(struct bitmap *bitmap, sector_t offset, sector_t *blocks,
			       int degraded)
{
	bitmap_counter_t *bmc;
	int rv;
	if (bitmap == NULL) {/*                                 */
		*blocks = 1024;
		return 1; /*                            */
	}
	spin_lock_irq(&bitmap->counts.lock);
	bmc = bitmap_get_counter(&bitmap->counts, offset, blocks, 0);
	rv = 0;
	if (bmc) {
		/*        */
		if (RESYNC(*bmc))
			rv = 1;
		else if (NEEDED(*bmc)) {
			rv = 1;
			if (!degraded) { /*                                  */
				*bmc |= RESYNC_MASK;
				*bmc &= ~NEEDED_MASK;
			}
		}
	}
	spin_unlock_irq(&bitmap->counts.lock);
	return rv;
}

int bitmap_start_sync(struct bitmap *bitmap, sector_t offset, sector_t *blocks,
		      int degraded)
{
	/*                                                           
                                                                
                 
                                                            
                                             
                                  
  */
	int rv = 0;
	sector_t blocks1;

	*blocks = 0;
	while (*blocks < (PAGE_SIZE>>9)) {
		rv |= __bitmap_start_sync(bitmap, offset,
					  &blocks1, degraded);
		offset += blocks1;
		*blocks += blocks1;
	}
	return rv;
}
EXPORT_SYMBOL(bitmap_start_sync);

void bitmap_end_sync(struct bitmap *bitmap, sector_t offset, sector_t *blocks, int aborted)
{
	bitmap_counter_t *bmc;
	unsigned long flags;

	if (bitmap == NULL) {
		*blocks = 1024;
		return;
	}
	spin_lock_irqsave(&bitmap->counts.lock, flags);
	bmc = bitmap_get_counter(&bitmap->counts, offset, blocks, 0);
	if (bmc == NULL)
		goto unlock;
	/*        */
	if (RESYNC(*bmc)) {
		*bmc &= ~RESYNC_MASK;

		if (!NEEDED(*bmc) && aborted)
			*bmc |= NEEDED_MASK;
		else {
			if (*bmc <= 2) {
				bitmap_set_pending(&bitmap->counts, offset);
				bitmap->allclean = 0;
			}
		}
	}
 unlock:
	spin_unlock_irqrestore(&bitmap->counts.lock, flags);
}
EXPORT_SYMBOL(bitmap_end_sync);

void bitmap_close_sync(struct bitmap *bitmap)
{
	/*                                                             
                                                              
                                      
  */
	sector_t sector = 0;
	sector_t blocks;
	if (!bitmap)
		return;
	while (sector < bitmap->mddev->resync_max_sectors) {
		bitmap_end_sync(bitmap, sector, &blocks, 0);
		sector += blocks;
	}
}
EXPORT_SYMBOL(bitmap_close_sync);

void bitmap_cond_end_sync(struct bitmap *bitmap, sector_t sector)
{
	sector_t s = 0;
	sector_t blocks;

	if (!bitmap)
		return;
	if (sector == 0) {
		bitmap->last_end_sync = jiffies;
		return;
	}
	if (time_before(jiffies, (bitmap->last_end_sync
				  + bitmap->mddev->bitmap_info.daemon_sleep)))
		return;
	wait_event(bitmap->mddev->recovery_wait,
		   atomic_read(&bitmap->mddev->recovery_active) == 0);

	bitmap->mddev->curr_resync_completed = sector;
	set_bit(MD_CHANGE_CLEAN, &bitmap->mddev->flags);
	sector &= ~((1ULL << bitmap->counts.chunkshift) - 1);
	s = 0;
	while (s < sector && s < bitmap->mddev->resync_max_sectors) {
		bitmap_end_sync(bitmap, s, &blocks, 0);
		s += blocks;
	}
	bitmap->last_end_sync = jiffies;
	sysfs_notify(&bitmap->mddev->kobj, NULL, "sync_completed");
}
EXPORT_SYMBOL(bitmap_cond_end_sync);

static void bitmap_set_memory_bits(struct bitmap *bitmap, sector_t offset, int needed)
{
	/*                                                        
                                                                 
                      
  */

	sector_t secs;
	bitmap_counter_t *bmc;
	spin_lock_irq(&bitmap->counts.lock);
	bmc = bitmap_get_counter(&bitmap->counts, offset, &secs, 1);
	if (!bmc) {
		spin_unlock_irq(&bitmap->counts.lock);
		return;
	}
	if (!*bmc) {
		*bmc = 2 | (needed ? NEEDED_MASK : 0);
		bitmap_count_page(&bitmap->counts, offset, 1);
		bitmap_set_pending(&bitmap->counts, offset);
		bitmap->allclean = 0;
	}
	spin_unlock_irq(&bitmap->counts.lock);
}

/*                                                             */
void bitmap_dirty_bits(struct bitmap *bitmap, unsigned long s, unsigned long e)
{
	unsigned long chunk;

	for (chunk = s; chunk <= e; chunk++) {
		sector_t sec = (sector_t)chunk << bitmap->counts.chunkshift;
		bitmap_set_memory_bits(bitmap, sec, 1);
		bitmap_file_set_bit(bitmap, sec);
		if (sec < bitmap->mddev->recovery_cp)
			/*                                          
                                             
                                         
    */
			bitmap->mddev->recovery_cp = sec;
	}
}

/*
                                
 */
void bitmap_flush(struct mddev *mddev)
{
	struct bitmap *bitmap = mddev->bitmap;
	long sleep;

	if (!bitmap) /*                     */
		return;

	/*                                                               
               
  */
	sleep = mddev->bitmap_info.daemon_sleep * 2;
	bitmap->daemon_lastrun -= sleep;
	bitmap_daemon_work(mddev);
	bitmap->daemon_lastrun -= sleep;
	bitmap_daemon_work(mddev);
	bitmap->daemon_lastrun -= sleep;
	bitmap_daemon_work(mddev);
	bitmap_update_sb(bitmap);
}

/*
                                 
 */
static void bitmap_free(struct bitmap *bitmap)
{
	unsigned long k, pages;
	struct bitmap_page *bp;

	if (!bitmap) /*                     */
		return;

	/*                                            */
	wait_event(bitmap->write_wait,
		   atomic_read(&bitmap->pending_writes) == 0);

	/*                          */
	bitmap_file_unmap(&bitmap->storage);

	bp = bitmap->counts.bp;
	pages = bitmap->counts.pages;

	/*                           */

	if (bp) /*                            */
		for (k = 0; k < pages; k++)
			if (bp[k].map && !bp[k].hijacked)
				kfree(bp[k].map);
	kfree(bp);
	kfree(bitmap);
}

void bitmap_destroy(struct mddev *mddev)
{
	struct bitmap *bitmap = mddev->bitmap;

	if (!bitmap) /*                     */
		return;

	mutex_lock(&mddev->bitmap_info.mutex);
	mddev->bitmap = NULL; /*                               */
	mutex_unlock(&mddev->bitmap_info.mutex);
	if (mddev->thread)
		mddev->thread->timeout = MAX_SCHEDULE_TIMEOUT;

	if (bitmap->sysfs_can_clear)
		sysfs_put(bitmap->sysfs_can_clear);

	bitmap_free(bitmap);
}

/*
                                  
                                                                         
 */
int bitmap_create(struct mddev *mddev)
{
	struct bitmap *bitmap;
	sector_t blocks = mddev->resync_max_sectors;
	struct file *file = mddev->bitmap_info.file;
	int err;
	struct sysfs_dirent *bm = NULL;

	BUILD_BUG_ON(sizeof(bitmap_super_t) != 256);

	BUG_ON(file && mddev->bitmap_info.offset);

	bitmap = kzalloc(sizeof(*bitmap), GFP_KERNEL);
	if (!bitmap)
		return -ENOMEM;

	spin_lock_init(&bitmap->counts.lock);
	atomic_set(&bitmap->pending_writes, 0);
	init_waitqueue_head(&bitmap->write_wait);
	init_waitqueue_head(&bitmap->overflow_wait);
	init_waitqueue_head(&bitmap->behind_wait);

	bitmap->mddev = mddev;

	if (mddev->kobj.sd)
		bm = sysfs_get_dirent(mddev->kobj.sd, NULL, "bitmap");
	if (bm) {
		bitmap->sysfs_can_clear = sysfs_get_dirent(bm, NULL, "can_clear");
		sysfs_put(bm);
	} else
		bitmap->sysfs_can_clear = NULL;

	bitmap->storage.file = file;
	if (file) {
		get_file(file);
		/*                                               
                                                     
           
   */
		vfs_fsync(file, 1);
	}
	/*                                                                           */
	if (!mddev->bitmap_info.external) {
		/*
                                                          
                                                            
   */
		if (test_and_clear_bit(MD_ARRAY_FIRST_USE, &mddev->flags))
			err = bitmap_new_disk_sb(bitmap);
		else
			err = bitmap_read_sb(bitmap);
	} else {
		err = 0;
		if (mddev->bitmap_info.chunksize == 0 ||
		    mddev->bitmap_info.daemon_sleep == 0)
			/*                                   
                 */
			err = -EINVAL;
	}
	if (err)
		goto error;

	bitmap->daemon_lastrun = jiffies;
	err = bitmap_resize(bitmap, blocks, mddev->bitmap_info.chunksize, 1);
	if (err)
		goto error;

	printk(KERN_INFO "created bitmap (%lu pages) for device %s\n",
	       bitmap->counts.pages, bmname(bitmap));

	mddev->bitmap = bitmap;
	return test_bit(BITMAP_WRITE_ERROR, &bitmap->flags) ? -EIO : 0;

 error:
	bitmap_free(bitmap);
	return err;
}

int bitmap_load(struct mddev *mddev)
{
	int err = 0;
	sector_t start = 0;
	sector_t sector = 0;
	struct bitmap *bitmap = mddev->bitmap;

	if (!bitmap)
		goto out;

	/*                                                              
                                                                
                                        
                                                         
  */
	while (sector < mddev->resync_max_sectors) {
		sector_t blocks;
		bitmap_start_sync(bitmap, sector, &blocks, 0);
		sector += blocks;
	}
	bitmap_close_sync(bitmap);

	if (mddev->degraded == 0
	    || bitmap->events_cleared == mddev->events)
		/*                                         
                                */
		start = mddev->recovery_cp;

	mutex_lock(&mddev->bitmap_info.mutex);
	err = bitmap_init_from_disk(bitmap, start);
	mutex_unlock(&mddev->bitmap_info.mutex);

	if (err)
		goto out;
	clear_bit(BITMAP_STALE, &bitmap->flags);

	/*                                         */
	set_bit(MD_RECOVERY_NEEDED, &bitmap->mddev->recovery);

	mddev->thread->timeout = mddev->bitmap_info.daemon_sleep;
	md_wakeup_thread(mddev->thread);

	bitmap_update_sb(bitmap);

	if (test_bit(BITMAP_WRITE_ERROR, &bitmap->flags))
		err = -EIO;
out:
	return err;
}
EXPORT_SYMBOL_GPL(bitmap_load);

void bitmap_status(struct seq_file *seq, struct bitmap *bitmap)
{
	unsigned long chunk_kb;
	struct bitmap_counts *counts;

	if (!bitmap)
		return;

	counts = &bitmap->counts;

	chunk_kb = bitmap->mddev->bitmap_info.chunksize >> 10;
	seq_printf(seq, "bitmap: %lu/%lu pages [%luKB], "
		   "%lu%s chunk",
		   counts->pages - counts->missing_pages,
		   counts->pages,
		   (counts->pages - counts->missing_pages)
		   << (PAGE_SHIFT - 10),
		   chunk_kb ? chunk_kb : bitmap->mddev->bitmap_info.chunksize,
		   chunk_kb ? "KB" : "B");
	if (bitmap->storage.file) {
		seq_printf(seq, ", file: ");
		seq_path(seq, &bitmap->storage.file->f_path, " \t\n");
	}

	seq_printf(seq, "\n");
}

int bitmap_resize(struct bitmap *bitmap, sector_t blocks,
		  int chunksize, int init)
{
	/*                                                      
                                             
                                                         
   
                                                             
                                                       
                                                           
                                                            
                                                  
  */
	struct bitmap_storage store;
	struct bitmap_counts old_counts;
	unsigned long chunks;
	sector_t block;
	sector_t old_blocks, new_blocks;
	int chunkshift;
	int ret = 0;
	long pages;
	struct bitmap_page *new_bp;

	if (chunksize == 0) {
		/*                                                          
                                                                
   */
		long bytes;
		long space = bitmap->mddev->bitmap_info.space;

		if (space == 0) {
			/*                                                
                                   
    */
			bytes = DIV_ROUND_UP(bitmap->counts.chunks, 8);
			if (!bitmap->mddev->bitmap_info.external)
				bytes += sizeof(bitmap_super_t);
			space = DIV_ROUND_UP(bytes, 512);
			bitmap->mddev->bitmap_info.space = space;
		}
		chunkshift = bitmap->counts.chunkshift;
		chunkshift--;
		do {
			/*                                                     */
			chunkshift++;
			chunks = DIV_ROUND_UP_SECTOR_T(blocks, 1 << chunkshift);
			bytes = DIV_ROUND_UP(chunks, 8);
			if (!bitmap->mddev->bitmap_info.external)
				bytes += sizeof(bitmap_super_t);
		} while (bytes > (space << 9));
	} else
		chunkshift = ffz(~chunksize) - BITMAP_BLOCK_SHIFT;

	chunks = DIV_ROUND_UP_SECTOR_T(blocks, 1 << chunkshift);
	memset(&store, 0, sizeof(store));
	if (bitmap->mddev->bitmap_info.offset || bitmap->mddev->bitmap_info.file)
		ret = bitmap_storage_alloc(&store, chunks,
					   !bitmap->mddev->bitmap_info.external);
	if (ret)
		goto err;

	pages = DIV_ROUND_UP(chunks, PAGE_COUNTER_RATIO);

	new_bp = kzalloc(pages * sizeof(*new_bp), GFP_KERNEL);
	ret = -ENOMEM;
	if (!new_bp) {
		bitmap_file_unmap(&store);
		goto err;
	}

	if (!init)
		bitmap->mddev->pers->quiesce(bitmap->mddev, 1);

	store.file = bitmap->storage.file;
	bitmap->storage.file = NULL;

	if (store.sb_page && bitmap->storage.sb_page)
		memcpy(page_address(store.sb_page),
		       page_address(bitmap->storage.sb_page),
		       sizeof(bitmap_super_t));
	bitmap_file_unmap(&bitmap->storage);
	bitmap->storage = store;

	old_counts = bitmap->counts;
	bitmap->counts.bp = new_bp;
	bitmap->counts.pages = pages;
	bitmap->counts.missing_pages = pages;
	bitmap->counts.chunkshift = chunkshift;
	bitmap->counts.chunks = chunks;
	bitmap->mddev->bitmap_info.chunksize = 1 << (chunkshift +
						     BITMAP_BLOCK_SHIFT);

	blocks = min(old_counts.chunks << old_counts.chunkshift,
		     chunks << chunkshift);

	spin_lock_irq(&bitmap->counts.lock);
	for (block = 0; block < blocks; ) {
		bitmap_counter_t *bmc_old, *bmc_new;
		int set;

		bmc_old = bitmap_get_counter(&old_counts, block,
					     &old_blocks, 0);
		set = bmc_old && NEEDED(*bmc_old);

		if (set) {
			bmc_new = bitmap_get_counter(&bitmap->counts, block,
						     &new_blocks, 1);
			if (*bmc_new == 0) {
				/*                               */
				sector_t end = block + new_blocks;
				sector_t start = block >> chunkshift;
				start <<= chunkshift;
				while (start < end) {
					bitmap_file_set_bit(bitmap, block);
					start += 1 << chunkshift;
				}
				*bmc_new = 2;
				bitmap_count_page(&bitmap->counts,
						  block, 1);
				bitmap_set_pending(&bitmap->counts,
						   block);
			}
			*bmc_new |= NEEDED_MASK;
			if (new_blocks < old_blocks)
				old_blocks = new_blocks;
		}
		block += old_blocks;
	}

	if (!init) {
		int i;
		while (block < (chunks << chunkshift)) {
			bitmap_counter_t *bmc;
			bmc = bitmap_get_counter(&bitmap->counts, block,
						 &new_blocks, 1);
			if (bmc) {
				/*                                        
                          
     */
				if (*bmc == 0) {
					*bmc = NEEDED_MASK | 2;
					bitmap_count_page(&bitmap->counts,
							  block, 1);
					bitmap_set_pending(&bitmap->counts,
							   block);
				}
			}
			block += new_blocks;
		}
		for (i = 0; i < bitmap->storage.file_pages; i++)
			set_page_attr(bitmap, i, BITMAP_PAGE_DIRTY);
	}
	spin_unlock_irq(&bitmap->counts.lock);

	if (!init) {
		bitmap_unplug(bitmap);
		bitmap->mddev->pers->quiesce(bitmap->mddev, 0);
	}
	ret = 0;
err:
	return ret;
}
EXPORT_SYMBOL_GPL(bitmap_resize);

static ssize_t
location_show(struct mddev *mddev, char *page)
{
	ssize_t len;
	if (mddev->bitmap_info.file)
		len = sprintf(page, "file");
	else if (mddev->bitmap_info.offset)
		len = sprintf(page, "%+lld", (long long)mddev->bitmap_info.offset);
	else
		len = sprintf(page, "none");
	len += sprintf(page+len, "\n");
	return len;
}

static ssize_t
location_store(struct mddev *mddev, const char *buf, size_t len)
{

	if (mddev->pers) {
		if (!mddev->pers->quiesce)
			return -EBUSY;
		if (mddev->recovery || mddev->sync_thread)
			return -EBUSY;
	}

	if (mddev->bitmap || mddev->bitmap_info.file ||
	    mddev->bitmap_info.offset) {
		/*                                                        */
		if (strncmp(buf, "none", 4) != 0)
			return -EBUSY;
		if (mddev->pers) {
			mddev->pers->quiesce(mddev, 1);
			bitmap_destroy(mddev);
			mddev->pers->quiesce(mddev, 0);
		}
		mddev->bitmap_info.offset = 0;
		if (mddev->bitmap_info.file) {
			struct file *f = mddev->bitmap_info.file;
			mddev->bitmap_info.file = NULL;
			restore_bitmap_write_access(f);
			fput(f);
		}
	} else {
		/*                                 */
		long long offset;
		if (strncmp(buf, "none", 4) == 0)
			/*                    */;
		else if (strncmp(buf, "file:", 5) == 0) {
			/*                   */
			return -EINVAL;
		} else {
			int rv;
			if (buf[0] == '+')
				rv = strict_strtoll(buf+1, 10, &offset);
			else
				rv = strict_strtoll(buf, 10, &offset);
			if (rv)
				return rv;
			if (offset == 0)
				return -EINVAL;
			if (mddev->bitmap_info.external == 0 &&
			    mddev->major_version == 0 &&
			    offset != mddev->bitmap_info.default_offset)
				return -EINVAL;
			mddev->bitmap_info.offset = offset;
			if (mddev->pers) {
				mddev->pers->quiesce(mddev, 1);
				rv = bitmap_create(mddev);
				if (!rv)
					rv = bitmap_load(mddev);
				if (rv) {
					bitmap_destroy(mddev);
					mddev->bitmap_info.offset = 0;
				}
				mddev->pers->quiesce(mddev, 0);
				if (rv)
					return rv;
			}
		}
	}
	if (!mddev->external) {
		/*                                    
                       
   */
		set_bit(MD_CHANGE_DEVS, &mddev->flags);
		md_wakeup_thread(mddev->thread);
	}
	return len;
}

static struct md_sysfs_entry bitmap_location =
__ATTR(location, S_IRUGO|S_IWUSR, location_show, location_store);

/*                                                            
                                                             
                                              
 */
static ssize_t
space_show(struct mddev *mddev, char *page)
{
	return sprintf(page, "%lu\n", mddev->bitmap_info.space);
}

static ssize_t
space_store(struct mddev *mddev, const char *buf, size_t len)
{
	unsigned long sectors;
	int rv;

	rv = kstrtoul(buf, 10, &sectors);
	if (rv)
		return rv;

	if (sectors == 0)
		return -EINVAL;

	if (mddev->bitmap &&
	    sectors < (mddev->bitmap->storage.bytes + 511) >> 9)
		return -EFBIG; /*                                        */

	/*                                                        
                                          
  */
	mddev->bitmap_info.space = sectors;
	return len;
}

static struct md_sysfs_entry bitmap_space =
__ATTR(space, S_IRUGO|S_IWUSR, space_show, space_store);

static ssize_t
timeout_show(struct mddev *mddev, char *page)
{
	ssize_t len;
	unsigned long secs = mddev->bitmap_info.daemon_sleep / HZ;
	unsigned long jifs = mddev->bitmap_info.daemon_sleep % HZ;

	len = sprintf(page, "%lu", secs);
	if (jifs)
		len += sprintf(page+len, ".%03u", jiffies_to_msecs(jifs));
	len += sprintf(page+len, "\n");
	return len;
}

static ssize_t
timeout_store(struct mddev *mddev, const char *buf, size_t len)
{
	/*                                */
	unsigned long timeout;
	int rv = strict_strtoul_scaled(buf, &timeout, 4);
	if (rv)
		return rv;

	/*                                        */
	if (timeout >= LONG_MAX / HZ)
		return -EINVAL;

	timeout = timeout * HZ / 10000;

	if (timeout >= MAX_SCHEDULE_TIMEOUT)
		timeout = MAX_SCHEDULE_TIMEOUT-1;
	if (timeout < 1)
		timeout = 1;
	mddev->bitmap_info.daemon_sleep = timeout;
	if (mddev->thread) {
		/*                                                 
                                                 
                                 
   */
		if (mddev->thread->timeout < MAX_SCHEDULE_TIMEOUT) {
			mddev->thread->timeout = timeout;
			md_wakeup_thread(mddev->thread);
		}
	}
	return len;
}

static struct md_sysfs_entry bitmap_timeout =
__ATTR(time_base, S_IRUGO|S_IWUSR, timeout_show, timeout_store);

static ssize_t
backlog_show(struct mddev *mddev, char *page)
{
	return sprintf(page, "%lu\n", mddev->bitmap_info.max_write_behind);
}

static ssize_t
backlog_store(struct mddev *mddev, const char *buf, size_t len)
{
	unsigned long backlog;
	int rv = strict_strtoul(buf, 10, &backlog);
	if (rv)
		return rv;
	if (backlog > COUNTER_MAX)
		return -EINVAL;
	mddev->bitmap_info.max_write_behind = backlog;
	return len;
}

static struct md_sysfs_entry bitmap_backlog =
__ATTR(backlog, S_IRUGO|S_IWUSR, backlog_show, backlog_store);

static ssize_t
chunksize_show(struct mddev *mddev, char *page)
{
	return sprintf(page, "%lu\n", mddev->bitmap_info.chunksize);
}

static ssize_t
chunksize_store(struct mddev *mddev, const char *buf, size_t len)
{
	/*                                              */
	int rv;
	unsigned long csize;
	if (mddev->bitmap)
		return -EBUSY;
	rv = strict_strtoul(buf, 10, &csize);
	if (rv)
		return rv;
	if (csize < 512 ||
	    !is_power_of_2(csize))
		return -EINVAL;
	mddev->bitmap_info.chunksize = csize;
	return len;
}

static struct md_sysfs_entry bitmap_chunksize =
__ATTR(chunksize, S_IRUGO|S_IWUSR, chunksize_show, chunksize_store);

static ssize_t metadata_show(struct mddev *mddev, char *page)
{
	return sprintf(page, "%s\n", (mddev->bitmap_info.external
				      ? "external" : "internal"));
}

static ssize_t metadata_store(struct mddev *mddev, const char *buf, size_t len)
{
	if (mddev->bitmap ||
	    mddev->bitmap_info.file ||
	    mddev->bitmap_info.offset)
		return -EBUSY;
	if (strncmp(buf, "external", 8) == 0)
		mddev->bitmap_info.external = 1;
	else if (strncmp(buf, "internal", 8) == 0)
		mddev->bitmap_info.external = 0;
	else
		return -EINVAL;
	return len;
}

static struct md_sysfs_entry bitmap_metadata =
__ATTR(metadata, S_IRUGO|S_IWUSR, metadata_show, metadata_store);

static ssize_t can_clear_show(struct mddev *mddev, char *page)
{
	int len;
	if (mddev->bitmap)
		len = sprintf(page, "%s\n", (mddev->bitmap->need_sync ?
					     "false" : "true"));
	else
		len = sprintf(page, "\n");
	return len;
}

static ssize_t can_clear_store(struct mddev *mddev, const char *buf, size_t len)
{
	if (mddev->bitmap == NULL)
		return -ENOENT;
	if (strncmp(buf, "false", 5) == 0)
		mddev->bitmap->need_sync = 1;
	else if (strncmp(buf, "true", 4) == 0) {
		if (mddev->degraded)
			return -EBUSY;
		mddev->bitmap->need_sync = 0;
	} else
		return -EINVAL;
	return len;
}

static struct md_sysfs_entry bitmap_can_clear =
__ATTR(can_clear, S_IRUGO|S_IWUSR, can_clear_show, can_clear_store);

static ssize_t
behind_writes_used_show(struct mddev *mddev, char *page)
{
	if (mddev->bitmap == NULL)
		return sprintf(page, "0\n");
	return sprintf(page, "%lu\n",
		       mddev->bitmap->behind_writes_used);
}

static ssize_t
behind_writes_used_reset(struct mddev *mddev, const char *buf, size_t len)
{
	if (mddev->bitmap)
		mddev->bitmap->behind_writes_used = 0;
	return len;
}

static struct md_sysfs_entry max_backlog_used =
__ATTR(max_backlog_used, S_IRUGO | S_IWUSR,
       behind_writes_used_show, behind_writes_used_reset);

static struct attribute *md_bitmap_attrs[] = {
	&bitmap_location.attr,
	&bitmap_space.attr,
	&bitmap_timeout.attr,
	&bitmap_backlog.attr,
	&bitmap_chunksize.attr,
	&bitmap_metadata.attr,
	&bitmap_can_clear.attr,
	&max_backlog_used.attr,
	NULL
};
struct attribute_group md_bitmap_group = {
	.name = "bitmap",
	.attrs = md_bitmap_attrs,
};

