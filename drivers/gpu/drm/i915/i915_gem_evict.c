/*
 * Copyright © 2008-2010 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *    Chris Wilson <chris@chris-wilson.co.uuk>
 *
 */

#include <drm/drmP.h>
#include "i915_drv.h"
#include <drm/i915_drm.h>
#include "i915_trace.h"

static bool
mark_free(struct drm_i915_gem_object *obj, struct list_head *unwind)
{
	if (obj->pin_count)
		return false;

	list_add(&obj->exec_list, unwind);
	return drm_mm_scan_add_block(obj->gtt_space);
}

int
i915_gem_evict_something(struct drm_device *dev, int min_size,
			 unsigned alignment, unsigned cache_level,
			 bool mappable, bool nonblocking)
{
	drm_i915_private_t *dev_priv = dev->dev_private;
	struct list_head eviction_list, unwind_list;
	struct drm_i915_gem_object *obj;
	int ret = 0;

	trace_i915_gem_evict(dev, min_size, alignment, mappable);

	/*
                                                                   
                                                                    
                                                                      
                                                                     
                                                               
                                                                 
                                                                     
                                                                   
                                                                    
                                                               
                                                     
   
                                    
                                           
                             
                      
                              
   
                                                                      
                       
  */

	INIT_LIST_HEAD(&unwind_list);
	if (mappable)
		drm_mm_init_scan_with_range(&dev_priv->mm.gtt_space,
					    min_size, alignment, cache_level,
					    0, dev_priv->gtt.mappable_end);
	else
		drm_mm_init_scan(&dev_priv->mm.gtt_space,
				 min_size, alignment, cache_level);

	/*                                                                */
	list_for_each_entry(obj, &dev_priv->mm.inactive_list, mm_list) {
		if (mark_free(obj, &unwind_list))
			goto found;
	}

	if (nonblocking)
		goto none;

	/*                                                */
	list_for_each_entry(obj, &dev_priv->mm.active_list, mm_list) {
		if (mark_free(obj, &unwind_list))
			goto found;
	}

none:
	/*                                       */
	while (!list_empty(&unwind_list)) {
		obj = list_first_entry(&unwind_list,
				       struct drm_i915_gem_object,
				       exec_list);

		ret = drm_mm_scan_remove_block(obj->gtt_space);
		BUG_ON(ret);

		list_del_init(&obj->exec_list);
	}

	/*                                                                    
                                                          
  */
	return -ENOSPC;

found:
	/*                                                      
                                                        
                    */
	INIT_LIST_HEAD(&eviction_list);
	while (!list_empty(&unwind_list)) {
		obj = list_first_entry(&unwind_list,
				       struct drm_i915_gem_object,
				       exec_list);
		if (drm_mm_scan_remove_block(obj->gtt_space)) {
			list_move(&obj->exec_list, &eviction_list);
			drm_gem_object_reference(&obj->base);
			continue;
		}
		list_del_init(&obj->exec_list);
	}

	/*                                          */
	while (!list_empty(&eviction_list)) {
		obj = list_first_entry(&eviction_list,
				       struct drm_i915_gem_object,
				       exec_list);
		if (ret == 0)
			ret = i915_gem_object_unbind(obj);

		list_del_init(&obj->exec_list);
		drm_gem_object_unreference(&obj->base);
	}

	return ret;
}

int
i915_gem_evict_everything(struct drm_device *dev)
{
	drm_i915_private_t *dev_priv = dev->dev_private;
	struct drm_i915_gem_object *obj, *next;
	bool lists_empty;
	int ret;

	lists_empty = (list_empty(&dev_priv->mm.inactive_list) &&
		       list_empty(&dev_priv->mm.active_list));
	if (lists_empty)
		return -ENOSPC;

	trace_i915_gem_evict_everything(dev);

	/*                                                              
                                                                 
                         
  */
	ret = i915_gpu_idle(dev);
	if (ret)
		return ret;

	i915_gem_retire_requests(dev);

	/*                                                                 */
	list_for_each_entry_safe(obj, next,
				 &dev_priv->mm.inactive_list, mm_list)
		if (obj->pin_count == 0)
			WARN_ON(i915_gem_object_unbind(obj));

	return 0;
}
