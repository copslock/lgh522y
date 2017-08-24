/*
                            
  
                                                    
              
                                                  
                                    
  
 */

#ifndef _EXT4_EXTENTS_STATUS_H
#define _EXT4_EXTENTS_STATUS_H

/*
                                                                         
 */
#ifdef ES_DEBUG__
#define es_debug(fmt, ...)	printk(fmt, ##__VA_ARGS__)
#else
#define es_debug(fmt, ...)	no_printk(fmt, ##__VA_ARGS__)
#endif

/*
                                                                    
                                       
 */
#define ES_AGGRESSIVE_TEST__

/*
                                                             
 */
#define EXTENT_STATUS_WRITTEN	(1ULL << 63)
#define EXTENT_STATUS_UNWRITTEN (1ULL << 62)
#define EXTENT_STATUS_DELAYED	(1ULL << 61)
#define EXTENT_STATUS_HOLE	(1ULL << 60)

#define EXTENT_STATUS_FLAGS	(EXTENT_STATUS_WRITTEN | \
				 EXTENT_STATUS_UNWRITTEN | \
				 EXTENT_STATUS_DELAYED | \
				 EXTENT_STATUS_HOLE)

struct ext4_extent;

struct extent_status {
	struct rb_node rb_node;
	ext4_lblk_t es_lblk;	/*                                   */
	ext4_lblk_t es_len;	/*                           */
	ext4_fsblk_t es_pblk;	/*                      */
};

struct ext4_es_tree {
	struct rb_root root;
	struct extent_status *cache_es;	/*                          */
};

extern int __init ext4_init_es(void);
extern void ext4_exit_es(void);
extern void ext4_es_init_tree(struct ext4_es_tree *tree);

extern int ext4_es_insert_extent(struct inode *inode, ext4_lblk_t lblk,
				 ext4_lblk_t len, ext4_fsblk_t pblk,
				 unsigned long long status);
extern int ext4_es_remove_extent(struct inode *inode, ext4_lblk_t lblk,
				 ext4_lblk_t len);
extern void ext4_es_find_delayed_extent_range(struct inode *inode,
					ext4_lblk_t lblk, ext4_lblk_t end,
					struct extent_status *es);
extern int ext4_es_lookup_extent(struct inode *inode, ext4_lblk_t lblk,
				 struct extent_status *es);
extern int ext4_es_zeroout(struct inode *inode, struct ext4_extent *ex);

static inline int ext4_es_is_written(struct extent_status *es)
{
	return (es->es_pblk & EXTENT_STATUS_WRITTEN) != 0;
}

static inline int ext4_es_is_unwritten(struct extent_status *es)
{
	return (es->es_pblk & EXTENT_STATUS_UNWRITTEN) != 0;
}

static inline int ext4_es_is_delayed(struct extent_status *es)
{
	return (es->es_pblk & EXTENT_STATUS_DELAYED) != 0;
}

static inline int ext4_es_is_hole(struct extent_status *es)
{
	return (es->es_pblk & EXTENT_STATUS_HOLE) != 0;
}

static inline ext4_fsblk_t ext4_es_status(struct extent_status *es)
{
	return (es->es_pblk & EXTENT_STATUS_FLAGS);
}

static inline ext4_fsblk_t ext4_es_pblock(struct extent_status *es)
{
	return (es->es_pblk & ~EXTENT_STATUS_FLAGS);
}

static inline void ext4_es_store_pblock(struct extent_status *es,
					ext4_fsblk_t pb)
{
	ext4_fsblk_t block;

	block = (pb & ~EXTENT_STATUS_FLAGS) |
		(es->es_pblk & EXTENT_STATUS_FLAGS);
	es->es_pblk = block;
}

static inline void ext4_es_store_status(struct extent_status *es,
					unsigned long long status)
{
	ext4_fsblk_t block;

	block = (status & EXTENT_STATUS_FLAGS) |
		(es->es_pblk & ~EXTENT_STATUS_FLAGS);
	es->es_pblk = block;
}

extern void ext4_es_register_shrinker(struct super_block *sb);
extern void ext4_es_unregister_shrinker(struct super_block *sb);
extern void ext4_es_lru_add(struct inode *inode);
extern void ext4_es_lru_del(struct inode *inode);

#endif /*                        */
