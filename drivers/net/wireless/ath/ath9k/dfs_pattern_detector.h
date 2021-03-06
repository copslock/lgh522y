/*
 * Copyright (c) 2012 Neratec Solutions AG
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef DFS_PATTERN_DETECTOR_H
#define DFS_PATTERN_DETECTOR_H

#include <linux/types.h>
#include <linux/list.h>
#include <linux/nl80211.h>

/* 
                                                         
                              
                                  
                               
                             
 */
struct pulse_event {
	u64 ts;
	u16 freq;
	u8 width;
	u8 rssi;
};

/* 
                                                                        
                                                   
                                                
                                                
                                                                            
                                                      
                                                          
                                        
                                                              
                                                                    
 */
struct radar_detector_specs {
	u8 type_id;
	u8 width_min;
	u8 width_max;
	u16 pri_min;
	u16 pri_max;
	u8 num_pri;
	u8 ppb;
	u8 ppb_thresh;
	u8 max_pri_tolerance;
};

/* 
                                                     
                      
                                                                               
                                                                       
                                                          
                                                    
                                                          
                                                        
                                                                
 */
struct dfs_pattern_detector {
	void (*exit)(struct dfs_pattern_detector *dpd);
	bool (*set_dfs_domain)(struct dfs_pattern_detector *dpd,
			   enum nl80211_dfs_regions region);
	bool (*add_pulse)(struct dfs_pattern_detector *dpd,
			  struct pulse_event *pe);

	enum nl80211_dfs_regions region;
	u8 num_radar_types;
	u64 last_pulse_ts;
	/*                      */
	struct ath_hw *ah;

	const struct radar_detector_specs *radar_spec;
	struct list_head channel_detectors;
};

/* 
                                                                       
                                                                             
                                                      
 */
#if defined(CONFIG_ATH9K_DFS_CERTIFIED)
extern struct dfs_pattern_detector *
dfs_pattern_detector_init(struct ath_hw *ah, enum nl80211_dfs_regions region);
#else
static inline struct dfs_pattern_detector *
dfs_pattern_detector_init(struct ath_hw *ah, enum nl80211_dfs_regions region)
{
	return NULL;
}
#endif /*                            */

#endif /*                        */
