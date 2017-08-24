#ifndef _BCACHE_STATS_H_
#define _BCACHE_STATS_H_

struct cache_stat_collector {
	atomic_t cache_hits;
	atomic_t cache_misses;
	atomic_t cache_bypass_hits;
	atomic_t cache_bypass_misses;
	atomic_t cache_readaheads;
	atomic_t cache_miss_collisions;
	atomic_t sectors_bypassed;
};

struct cache_stats {
	struct kobject		kobj;

	unsigned long cache_hits;
	unsigned long cache_misses;
	unsigned long cache_bypass_hits;
	unsigned long cache_bypass_misses;
	unsigned long cache_readaheads;
	unsigned long cache_miss_collisions;
	unsigned long sectors_bypassed;

	unsigned		rescale;
};

struct cache_accounting {
	struct closure		cl;
	struct timer_list	timer;
	atomic_t		closing;

	struct cache_stat_collector collector;

	struct cache_stats total;
	struct cache_stats five_minute;
	struct cache_stats hour;
	struct cache_stats day;
};

struct search;

void bch_cache_accounting_init(struct cache_accounting *acc,
			       struct closure *parent);

int bch_cache_accounting_add_kobjs(struct cache_accounting *acc,
				   struct kobject *parent);

void bch_cache_accounting_clear(struct cache_accounting *acc);

void bch_cache_accounting_destroy(struct cache_accounting *acc);

void bch_mark_cache_accounting(struct search *s, bool hit, bool bypass);
void bch_mark_cache_readahead(struct search *s);
void bch_mark_cache_miss_collision(struct search *s);
void bch_mark_sectors_bypassed(struct search *s, int sectors);

#endif /*                  */
