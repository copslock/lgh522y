/*
 * Tegra host1x Job
 *
 * Copyright (c) 2011-2013, NVIDIA Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __HOST1X_JOB_H
#define __HOST1X_JOB_H

struct host1x_job_gather {
	u32 words;
	dma_addr_t base;
	struct host1x_bo *bo;
	int offset;
	bool handled;
};

struct host1x_cmdbuf {
	u32 handle;
	u32 offset;
	u32 words;
	u32 pad;
};

struct host1x_reloc {
	struct host1x_bo *cmdbuf;
	u32 cmdbuf_offset;
	struct host1x_bo *target;
	u32 target_offset;
	u32 shift;
	u32 pad;
};

struct host1x_waitchk {
	struct host1x_bo *bo;
	u32 offset;
	u32 syncpt_id;
	u32 thresh;
};

struct host1x_job_unpin_data {
	struct host1x_bo *bo;
	struct sg_table *sgt;
};

/*
                                          
 */
struct host1x_job {
	/*                                              */
	struct kref ref;

	/*            */
	struct list_head list;

	/*                                   */
	struct host1x_channel *channel;

	u32 client;

	/*                          */
	struct host1x_job_gather *gathers;
	unsigned int num_gathers;

	/*                                            */
	struct host1x_waitchk *waitchk;
	unsigned int num_waitchk;
	u32 waitchk_mask;

	/*                                          */
	struct host1x_reloc *relocarray;
	unsigned int num_relocs;
	struct host1x_job_unpin_data *unpins;
	unsigned int num_unpins;

	dma_addr_t *addr_phys;
	dma_addr_t *gather_addr_phys;
	dma_addr_t *reloc_addr_phys;

	/*                                                                   */
	u32 syncpt_id;
	u32 syncpt_incrs;
	u32 syncpt_end;

	/*                                   */
	unsigned int timeout;

	/*                                                   */
	unsigned int first_get;
	unsigned int num_slots;

	/*                 */
	size_t gather_copy_size;
	dma_addr_t gather_copy;
	u8 *gather_copy_mapped;

	/*                                               */
	int (*is_addr_reg)(struct device *dev, u32 reg, u32 class);

	/*                                  */
	u32 class;

	/*                                                 */
	bool serialize;
};
/*
                                                                     
                         
 */
struct host1x_job *host1x_job_alloc(struct host1x_channel *ch,
				    u32 num_cmdbufs, u32 num_relocs,
				    u32 num_waitchks);

/*
                         
 */
void host1x_job_add_gather(struct host1x_job *job, struct host1x_bo *mem_id,
			   u32 words, u32 offset);

/*
                                           
 */
struct host1x_job *host1x_job_get(struct host1x_job *job);

/*
                                                 
 */
void host1x_job_put(struct host1x_job *job);

/*
                                                                         
                                                                            
                                       
  
                                                                           
               
 */
int host1x_job_pin(struct host1x_job *job, struct device *dev);

/*
                               
 */
void host1x_job_unpin(struct host1x_job *job);

/*
                                        
 */
void host1x_job_dump(struct device *dev, struct host1x_job *job);

#endif
