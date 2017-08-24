/*
 * Performance event support - powerpc architecture code
 *
 * Copyright 2008-2009 Paul Mackerras, IBM Corporation.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/perf_event.h>
#include <linux/percpu.h>
#include <linux/hardirq.h>
#include <linux/uaccess.h>
#include <asm/reg.h>
#include <asm/pmc.h>
#include <asm/machdep.h>
#include <asm/firmware.h>
#include <asm/ptrace.h>
#include <asm/code-patching.h>

#define BHRB_MAX_ENTRIES	32
#define BHRB_TARGET		0x0000000000000002
#define BHRB_PREDICTION		0x0000000000000001
#define BHRB_EA			0xFFFFFFFFFFFFFFFC

struct cpu_hw_events {
	int n_events;
	int n_percpu;
	int disabled;
	int n_added;
	int n_limited;
	u8  pmcs_enabled;
	struct perf_event *event[MAX_HWEVENTS];
	u64 events[MAX_HWEVENTS];
	unsigned int flags[MAX_HWEVENTS];
	unsigned long mmcr[3];
	struct perf_event *limited_counter[MAX_LIMITED_HWCOUNTERS];
	u8  limited_hwidx[MAX_LIMITED_HWCOUNTERS];
	u64 alternatives[MAX_HWEVENTS][MAX_EVENT_ALTERNATIVES];
	unsigned long amasks[MAX_HWEVENTS][MAX_EVENT_ALTERNATIVES];
	unsigned long avalues[MAX_HWEVENTS][MAX_EVENT_ALTERNATIVES];

	unsigned int group_flag;
	int n_txn_start;

	/*           */
	u64				bhrb_filter;	/*                       */
	int				bhrb_users;
	void				*bhrb_context;
	struct	perf_branch_stack	bhrb_stack;
	struct	perf_branch_entry	bhrb_entries[BHRB_MAX_ENTRIES];
};

DEFINE_PER_CPU(struct cpu_hw_events, cpu_hw_events);

struct power_pmu *ppmu;

/*
                                                                    
                                                                    
                                                                     
                                                                       
                                                            
 */
static unsigned int freeze_events_kernel = MMCR0_FCS;

/*
                                                    
                                       
 */
#ifdef CONFIG_PPC32

#define MMCR0_FCHV		0
#define MMCR0_PMCjCE		MMCR0_PMCnCE
#define MMCR0_FC56		0
#define MMCR0_PMAO		0

#define SPRN_MMCRA		SPRN_MMCR2
#define MMCRA_SAMPLE_ENABLE	0

static inline unsigned long perf_ip_adjust(struct pt_regs *regs)
{
	return 0;
}
static inline void perf_get_data_addr(struct pt_regs *regs, u64 *addrp) { }
static inline u32 perf_get_misc_flags(struct pt_regs *regs)
{
	return 0;
}
static inline void perf_read_regs(struct pt_regs *regs)
{
	regs->result = 0;
}
static inline int perf_intr_is_nmi(struct pt_regs *regs)
{
	return 0;
}

static inline int siar_valid(struct pt_regs *regs)
{
	return 1;
}

static inline void power_pmu_bhrb_enable(struct perf_event *event) {}
static inline void power_pmu_bhrb_disable(struct perf_event *event) {}
void power_pmu_flush_branch_stack(void) {}
static inline void power_pmu_bhrb_read(struct cpu_hw_events *cpuhw) {}
#endif /*              */

static bool regs_use_siar(struct pt_regs *regs)
{
	return !!regs->result;
}

/*
                                                      
 */
#ifdef CONFIG_PPC64

static inline unsigned long perf_ip_adjust(struct pt_regs *regs)
{
	unsigned long mmcra = regs->dsisr;

	if ((ppmu->flags & PPMU_HAS_SSLOT) && (mmcra & MMCRA_SAMPLE_ENABLE)) {
		unsigned long slot = (mmcra & MMCRA_SLOT) >> MMCRA_SLOT_SHIFT;
		if (slot > 1)
			return 4 * (slot - 1);
	}

	return 0;
}

/*
                                          
                                                              
                                                                      
                                                               
                                                                          
                                                                          
 */
static inline void perf_get_data_addr(struct pt_regs *regs, u64 *addrp)
{
	unsigned long mmcra = regs->dsisr;
	bool sdar_valid;

	if (ppmu->flags & PPMU_HAS_SIER)
		sdar_valid = regs->dar & SIER_SDAR_VALID;
	else {
		unsigned long sdsync;

		if (ppmu->flags & PPMU_SIAR_VALID)
			sdsync = POWER7P_MMCRA_SDAR_VALID;
		else if (ppmu->flags & PPMU_ALT_SIPR)
			sdsync = POWER6_MMCRA_SDSYNC;
		else
			sdsync = MMCRA_SDSYNC;

		sdar_valid = mmcra & sdsync;
	}

	if (!(mmcra & MMCRA_SAMPLE_ENABLE) || sdar_valid)
		*addrp = mfspr(SPRN_SDAR);
}

static bool regs_sihv(struct pt_regs *regs)
{
	unsigned long sihv = MMCRA_SIHV;

	if (ppmu->flags & PPMU_HAS_SIER)
		return !!(regs->dar & SIER_SIHV);

	if (ppmu->flags & PPMU_ALT_SIPR)
		sihv = POWER6_MMCRA_SIHV;

	return !!(regs->dsisr & sihv);
}

static bool regs_sipr(struct pt_regs *regs)
{
	unsigned long sipr = MMCRA_SIPR;

	if (ppmu->flags & PPMU_HAS_SIER)
		return !!(regs->dar & SIER_SIPR);

	if (ppmu->flags & PPMU_ALT_SIPR)
		sipr = POWER6_MMCRA_SIPR;

	return !!(regs->dsisr & sipr);
}

static inline u32 perf_flags_from_msr(struct pt_regs *regs)
{
	if (regs->msr & MSR_PR)
		return PERF_RECORD_MISC_USER;
	if ((regs->msr & MSR_HV) && freeze_events_kernel != MMCR0_FCHV)
		return PERF_RECORD_MISC_HYPERVISOR;
	return PERF_RECORD_MISC_KERNEL;
}

static inline u32 perf_get_misc_flags(struct pt_regs *regs)
{
	bool use_siar = regs_use_siar(regs);

	if (!use_siar)
		return perf_flags_from_msr(regs);

	/*
                                                      
                                                    
                                                 
           
  */
	if (ppmu->flags & PPMU_NO_SIPR) {
		unsigned long siar = mfspr(SPRN_SIAR);
		if (siar >= PAGE_OFFSET)
			return PERF_RECORD_MISC_KERNEL;
		return PERF_RECORD_MISC_USER;
	}

	/*                                                      */
	if (regs_sipr(regs))
		return PERF_RECORD_MISC_USER;

	if (regs_sihv(regs) && (freeze_events_kernel != MMCR0_FCHV))
		return PERF_RECORD_MISC_HYPERVISOR;

	return PERF_RECORD_MISC_KERNEL;
}

/*
                                                                      
                     
                                                  
                                                                         
                                             
 */
static inline void perf_read_regs(struct pt_regs *regs)
{
	unsigned long mmcra = mfspr(SPRN_MMCRA);
	int marked = mmcra & MMCRA_SAMPLE_ENABLE;
	int use_siar;

	regs->dsisr = mmcra;

	if (ppmu->flags & PPMU_HAS_SIER)
		regs->dar = mfspr(SPRN_SIER);

	/*
                                                                   
                           
   
                                         
   
                                                                
            
   
                                                        
                                                            
                                                               
                                                             
                                                          
                                                            
                                             
  */
	if (TRAP(regs) != 0xf00)
		use_siar = 0;
	else if (marked)
		use_siar = 1;
	else if ((ppmu->flags & PPMU_NO_CONT_SAMPLING))
		use_siar = 0;
	else if (!(ppmu->flags & PPMU_NO_SIPR) && regs_sipr(regs))
		use_siar = 0;
	else
		use_siar = 1;

	regs->result = use_siar;
}

/*
                                                                      
                
 */
static inline int perf_intr_is_nmi(struct pt_regs *regs)
{
	return !regs->softe;
}

/*
                                                                           
                                                     
  
                                                                              
                                  
 */
static inline int siar_valid(struct pt_regs *regs)
{
	unsigned long mmcra = regs->dsisr;
	int marked = mmcra & MMCRA_SAMPLE_ENABLE;

	if (marked) {
		if (ppmu->flags & PPMU_HAS_SIER)
			return regs->dar & SIER_SIAR_VALID;

		if (ppmu->flags & PPMU_SIAR_VALID)
			return mmcra & POWER7P_MMCRA_SIAR_VALID;
	}

	return 1;
}


/*                                 */
static void power_pmu_bhrb_reset(void)
{
	asm volatile(PPC_CLRBHRB);
}

static void power_pmu_bhrb_enable(struct perf_event *event)
{
	struct cpu_hw_events *cpuhw = &__get_cpu_var(cpu_hw_events);

	if (!ppmu->bhrb_nr)
		return;

	/*                                                           */
	if (event->ctx->task && cpuhw->bhrb_context != event->ctx) {
		power_pmu_bhrb_reset();
		cpuhw->bhrb_context = event->ctx;
	}
	cpuhw->bhrb_users++;
}

static void power_pmu_bhrb_disable(struct perf_event *event)
{
	struct cpu_hw_events *cpuhw = &__get_cpu_var(cpu_hw_events);

	if (!ppmu->bhrb_nr)
		return;

	cpuhw->bhrb_users--;
	WARN_ON_ONCE(cpuhw->bhrb_users < 0);

	if (!cpuhw->disabled && !cpuhw->bhrb_users) {
		/*                                     
                                  
   */

		/*                     */
		cpuhw->bhrb_context = NULL;
	}
}

/*                                                             
                                                                 
 */
void power_pmu_flush_branch_stack(void)
{
	if (ppmu->bhrb_nr)
		power_pmu_bhrb_reset();
}
/*                                       */
static __u64 power_pmu_bhrb_to(u64 addr)
{
	unsigned int instr;
	int ret;
	__u64 target;

	if (is_kernel_addr(addr))
		return branch_target((unsigned int *)addr);

	/*                                                         */
	pagefault_disable();
	ret = __get_user_inatomic(instr, (unsigned int __user *)addr);
	if (ret) {
		pagefault_enable();
		return 0;
	}
	pagefault_enable();

	target = branch_target(&instr);
	if ((!target) || (instr & BRANCH_ABSOLUTE))
		return target;

	/*                                                              */
	return target - (unsigned long)&instr + addr;
}

/*                         */
void power_pmu_bhrb_read(struct cpu_hw_events *cpuhw)
{
	u64 val;
	u64 addr;
	int r_index, u_index, pred;

	r_index = 0;
	u_index = 0;
	while (r_index < ppmu->bhrb_nr) {
		/*                        */
		val = read_bhrb(r_index++);
		if (!val)
			/*                                            */
			break;
		else {
			addr = val & BHRB_EA;
			pred = val & BHRB_PREDICTION;

			if (!addr)
				/*               */
				continue;

			/*                                                     
                              
                                           
                                                    
                                                      
                                                
                                                     
                                    
                                                        
                                                     
                                                         
                                                        
                                                        
                                                        
                                                          
                                                   
    */

			if (val & BHRB_TARGET) {
				/*                                
                                   
     */
				cpuhw->bhrb_entries[u_index].to = addr;
				cpuhw->bhrb_entries[u_index].mispred = pred;
				cpuhw->bhrb_entries[u_index].predicted = ~pred;

				/*                                */
				val = read_bhrb(r_index++);
				addr = val & BHRB_EA;
				if (val & BHRB_TARGET) {
					/*                                
                                        */
					r_index--;
					addr = 0;
				}
				cpuhw->bhrb_entries[u_index].from = addr;
			} else {
				/*                             
                        */
				cpuhw->bhrb_entries[u_index].from = addr;
				cpuhw->bhrb_entries[u_index].to =
					power_pmu_bhrb_to(addr);
				cpuhw->bhrb_entries[u_index].mispred = pred;
				cpuhw->bhrb_entries[u_index].predicted = ~pred;
			}
			u_index++;

		}
	}
	cpuhw->bhrb_stack.nr = u_index;
	return;
}

#endif /*              */

static void perf_event_interrupt(struct pt_regs *regs);

void perf_event_print_debug(void)
{
}

/*
                                              
 */
static unsigned long read_pmc(int idx)
{
	unsigned long val;

	switch (idx) {
	case 1:
		val = mfspr(SPRN_PMC1);
		break;
	case 2:
		val = mfspr(SPRN_PMC2);
		break;
	case 3:
		val = mfspr(SPRN_PMC3);
		break;
	case 4:
		val = mfspr(SPRN_PMC4);
		break;
	case 5:
		val = mfspr(SPRN_PMC5);
		break;
	case 6:
		val = mfspr(SPRN_PMC6);
		break;
#ifdef CONFIG_PPC64
	case 7:
		val = mfspr(SPRN_PMC7);
		break;
	case 8:
		val = mfspr(SPRN_PMC8);
		break;
#endif /*              */
	default:
		printk(KERN_ERR "oops trying to read PMC%d\n", idx);
		val = 0;
	}
	return val;
}

/*
                 
 */
static void write_pmc(int idx, unsigned long val)
{
	switch (idx) {
	case 1:
		mtspr(SPRN_PMC1, val);
		break;
	case 2:
		mtspr(SPRN_PMC2, val);
		break;
	case 3:
		mtspr(SPRN_PMC3, val);
		break;
	case 4:
		mtspr(SPRN_PMC4, val);
		break;
	case 5:
		mtspr(SPRN_PMC5, val);
		break;
	case 6:
		mtspr(SPRN_PMC6, val);
		break;
#ifdef CONFIG_PPC64
	case 7:
		mtspr(SPRN_PMC7, val);
		break;
	case 8:
		mtspr(SPRN_PMC8, val);
		break;
#endif /*              */
	default:
		printk(KERN_ERR "oops trying to write PMC%d\n", idx);
	}
}

/*
                                                          
                                                                    
                                                               
                                              
 */
static int power_check_constraints(struct cpu_hw_events *cpuhw,
				   u64 event_id[], unsigned int cflags[],
				   int n_ev)
{
	unsigned long mask, value, nv;
	unsigned long smasks[MAX_HWEVENTS], svalues[MAX_HWEVENTS];
	int n_alt[MAX_HWEVENTS], choice[MAX_HWEVENTS];
	int i, j;
	unsigned long addf = ppmu->add_fields;
	unsigned long tadd = ppmu->test_adder;

	if (n_ev > ppmu->n_counter)
		return -1;

	/*                                          */
	for (i = 0; i < n_ev; ++i) {
		if ((cflags[i] & PPMU_LIMITED_PMC_REQD)
		    && !ppmu->limited_pmc_event(event_id[i])) {
			ppmu->get_alternatives(event_id[i], cflags[i],
					       cpuhw->alternatives[i]);
			event_id[i] = cpuhw->alternatives[i][0];
		}
		if (ppmu->get_constraint(event_id[i], &cpuhw->amasks[i][0],
					 &cpuhw->avalues[i][0]))
			return -1;
	}
	value = mask = 0;
	for (i = 0; i < n_ev; ++i) {
		nv = (value | cpuhw->avalues[i][0]) +
			(value & cpuhw->avalues[i][0] & addf);
		if ((((nv + tadd) ^ value) & mask) != 0 ||
		    (((nv + tadd) ^ cpuhw->avalues[i][0]) &
		     cpuhw->amasks[i][0]) != 0)
			break;
		value = nv;
		mask |= cpuhw->amasks[i][0];
	}
	if (i == n_ev)
		return 0;	/*        */

	/*                                      */
	if (!ppmu->get_alternatives)
		return -1;
	for (i = 0; i < n_ev; ++i) {
		choice[i] = 0;
		n_alt[i] = ppmu->get_alternatives(event_id[i], cflags[i],
						  cpuhw->alternatives[i]);
		for (j = 1; j < n_alt[i]; ++j)
			ppmu->get_constraint(cpuhw->alternatives[i][j],
					     &cpuhw->amasks[i][j],
					     &cpuhw->avalues[i][j]);
	}

	/*                                                      */
	i = 0;
	j = -1;
	value = mask = nv = 0;
	while (i < n_ev) {
		if (j >= 0) {
			/*                                     */
			value = svalues[i];
			mask = smasks[i];
			j = choice[i];
		}
		/*
                                             
                                               
   */
		while (++j < n_alt[i]) {
			nv = (value | cpuhw->avalues[i][j]) +
				(value & cpuhw->avalues[i][j] & addf);
			if ((((nv + tadd) ^ value) & mask) == 0 &&
			    (((nv + tadd) ^ cpuhw->avalues[i][j])
			     & cpuhw->amasks[i][j]) == 0)
				break;
		}
		if (j >= n_alt[i]) {
			/*
                                        
                                                  
                                           
    */
			if (--i < 0)
				return -1;
		} else {
			/*
                                                  
                                                     
                                                
                                   
    */
			choice[i] = j;
			svalues[i] = value;
			smasks[i] = mask;
			value = nv;
			mask |= cpuhw->amasks[i][j];
			++i;
			j = -1;
		}
	}

	/*                                                                  */
	for (i = 0; i < n_ev; ++i)
		event_id[i] = cpuhw->alternatives[i][choice[i]];
	return 0;
}

/*
                                                           
                                                              
                
 */
static int check_excludes(struct perf_event **ctrs, unsigned int cflags[],
			  int n_prev, int n_new)
{
	int eu = 0, ek = 0, eh = 0;
	int i, n, first;
	struct perf_event *event;

	n = n_prev + n_new;
	if (n <= 1)
		return 0;

	first = 1;
	for (i = 0; i < n; ++i) {
		if (cflags[i] & PPMU_LIMITED_PMC_OK) {
			cflags[i] &= ~PPMU_LIMITED_PMC_REQD;
			continue;
		}
		event = ctrs[i];
		if (first) {
			eu = event->attr.exclude_user;
			ek = event->attr.exclude_kernel;
			eh = event->attr.exclude_hv;
			first = 0;
		} else if (event->attr.exclude_user != eu ||
			   event->attr.exclude_kernel != ek ||
			   event->attr.exclude_hv != eh) {
			return -EAGAIN;
		}
	}

	if (eu || ek || eh)
		for (i = 0; i < n; ++i)
			if (cflags[i] & PPMU_LIMITED_PMC_OK)
				cflags[i] |= PPMU_LIMITED_PMC_REQD;

	return 0;
}

static u64 check_and_compute_delta(u64 prev, u64 val)
{
	u64 delta = (val - prev) & 0xfffffffful;

	/*
                                                                    
                                                                      
                                                                       
                                                                         
                                                                   
                                                                
             
  */
	if (prev > val && (prev - val) < 256)
		delta = 0;

	return delta;
}

static void power_pmu_read(struct perf_event *event)
{
	s64 val, delta, prev;

	if (event->hw.state & PERF_HES_STOPPED)
		return;

	if (!event->hw.idx)
		return;
	/*
                                                            
                                                              
                                      
  */
	do {
		prev = local64_read(&event->hw.prev_count);
		barrier();
		val = read_pmc(event->hw.idx);
		delta = check_and_compute_delta(prev, val);
		if (!delta)
			return;
	} while (local64_cmpxchg(&event->hw.prev_count, prev, val) != prev);

	local64_add(delta, &event->count);

	/*
                                                                       
                                                                       
                                                                     
                                                                  
              
   
                                                             
  */
	do {
		prev = local64_read(&event->hw.period_left);
		val = prev - delta;
		if (val < 1)
			val = 1;
	} while (local64_cmpxchg(&event->hw.period_left, prev, val) != prev);
}

/*
                                                                  
                                                                    
                                     
 */
static int is_limited_pmc(int pmcnum)
{
	return (ppmu->flags & PPMU_LIMITED_PMC5_6)
		&& (pmcnum == 5 || pmcnum == 6);
}

static void freeze_limited_counters(struct cpu_hw_events *cpuhw,
				    unsigned long pmc5, unsigned long pmc6)
{
	struct perf_event *event;
	u64 val, prev, delta;
	int i;

	for (i = 0; i < cpuhw->n_limited; ++i) {
		event = cpuhw->limited_counter[i];
		if (!event->hw.idx)
			continue;
		val = (event->hw.idx == 5) ? pmc5 : pmc6;
		prev = local64_read(&event->hw.prev_count);
		event->hw.idx = 0;
		delta = check_and_compute_delta(prev, val);
		if (delta)
			local64_add(delta, &event->count);
	}
}

static void thaw_limited_counters(struct cpu_hw_events *cpuhw,
				  unsigned long pmc5, unsigned long pmc6)
{
	struct perf_event *event;
	u64 val, prev;
	int i;

	for (i = 0; i < cpuhw->n_limited; ++i) {
		event = cpuhw->limited_counter[i];
		event->hw.idx = cpuhw->limited_hwidx[i];
		val = (event->hw.idx == 5) ? pmc5 : pmc6;
		prev = local64_read(&event->hw.prev_count);
		if (check_and_compute_delta(prev, val))
			local64_set(&event->hw.prev_count, val);
		perf_event_update_userpage(event);
	}
}

/*
                                                               
                                                                 
                                                            
                                                            
                                                                   
                                                          
                                                            
                                                               
                                                   
 */
static void write_mmcr0(struct cpu_hw_events *cpuhw, unsigned long mmcr0)
{
	unsigned long pmc5, pmc6;

	if (!cpuhw->n_limited) {
		mtspr(SPRN_MMCR0, mmcr0);
		return;
	}

	/*
                                                     
                                                          
                                                          
                                                        
                                     
  */
	asm volatile("mtspr %3,%2; mfspr %0,%4; mfspr %1,%5"
		     : "=&r" (pmc5), "=&r" (pmc6)
		     : "r" (mmcr0 & ~(MMCR0_PMC1CE | MMCR0_PMCjCE)),
		       "i" (SPRN_MMCR0),
		       "i" (SPRN_PMC5), "i" (SPRN_PMC6));

	if (mmcr0 & MMCR0_FC)
		freeze_limited_counters(cpuhw, pmc5, pmc6);
	else
		thaw_limited_counters(cpuhw, pmc5, pmc6);

	/*
                                                               
                              
  */
	if (mmcr0 & (MMCR0_PMC1CE | MMCR0_PMCjCE))
		mtspr(SPRN_MMCR0, mmcr0);
}

/*
                                                            
                                 
 */
static void power_pmu_disable(struct pmu *pmu)
{
	struct cpu_hw_events *cpuhw;
	unsigned long flags, val;

	if (!ppmu)
		return;
	local_irq_save(flags);
	cpuhw = &__get_cpu_var(cpu_hw_events);

	if (!cpuhw->disabled) {
		/*
                                                  
   */
		if (!cpuhw->pmcs_enabled) {
			ppc_enable_pmcs();
			cpuhw->pmcs_enabled = 1;
		}

		/*
                                                    
   */
		val  = mfspr(SPRN_MMCR0);
		val |= MMCR0_FC;
		val &= ~(MMCR0_PMAO | MMCR0_FC56);

		/*
                                                   
                                                    
                      
   */
		write_mmcr0(cpuhw, val);
		mb();

		/*
                                                   
   */
		if (cpuhw->mmcr[2] & MMCRA_SAMPLE_ENABLE) {
			mtspr(SPRN_MMCRA,
			      cpuhw->mmcr[2] & ~MMCRA_SAMPLE_ENABLE);
			mb();
		}

		cpuhw->disabled = 1;
		cpuhw->n_added = 0;
	}
	local_irq_restore(flags);
}

/*
                                        
                                                             
                                 
 */
static void power_pmu_enable(struct pmu *pmu)
{
	struct perf_event *event;
	struct cpu_hw_events *cpuhw;
	unsigned long flags;
	long i;
	unsigned long val;
	s64 left;
	unsigned int hwc_index[MAX_HWEVENTS];
	int n_lim;
	int idx;

	if (!ppmu)
		return;

	local_irq_save(flags);

	cpuhw = &__get_cpu_var(cpu_hw_events);
	if (!cpuhw->disabled)
		goto out;

	if (cpuhw->n_events == 0) {
		ppc_set_pmu_inuse(0);
		goto out;
	}

	cpuhw->disabled = 0;

	/*
                                                         
                                                             
                                                         
                                             
  */
	if (!cpuhw->n_added) {
		mtspr(SPRN_MMCRA, cpuhw->mmcr[2] & ~MMCRA_SAMPLE_ENABLE);
		mtspr(SPRN_MMCR1, cpuhw->mmcr[1]);
		goto out_enable;
	}

	/*
                                                  
  */
	if (ppmu->compute_mmcr(cpuhw->events, cpuhw->n_events, hwc_index,
			       cpuhw->mmcr)) {
		/*                         */
		printk(KERN_ERR "oops compute_mmcr failed\n");
		goto out;
	}

	/*
                                                 
                                            
                                                    
                                                  
  */
	event = cpuhw->event[0];
	if (event->attr.exclude_user)
		cpuhw->mmcr[0] |= MMCR0_FCP;
	if (event->attr.exclude_kernel)
		cpuhw->mmcr[0] |= freeze_events_kernel;
	if (event->attr.exclude_hv)
		cpuhw->mmcr[0] |= MMCR0_FCHV;

	/*
                                                        
                                                                
                             
  */
	ppc_set_pmu_inuse(1);
	mtspr(SPRN_MMCRA, cpuhw->mmcr[2] & ~MMCRA_SAMPLE_ENABLE);
	mtspr(SPRN_MMCR1, cpuhw->mmcr[1]);
	mtspr(SPRN_MMCR0, (cpuhw->mmcr[0] & ~(MMCR0_PMC1CE | MMCR0_PMCjCE))
				| MMCR0_FC);

	/*
                                                      
                   
  */
	for (i = 0; i < cpuhw->n_events; ++i) {
		event = cpuhw->event[i];
		if (event->hw.idx && event->hw.idx != hwc_index[i] + 1) {
			power_pmu_read(event);
			write_pmc(event->hw.idx, 0);
			event->hw.idx = 0;
		}
	}

	/*
                                                         
  */
	cpuhw->n_limited = n_lim = 0;
	for (i = 0; i < cpuhw->n_events; ++i) {
		event = cpuhw->event[i];
		if (event->hw.idx)
			continue;
		idx = hwc_index[i] + 1;
		if (is_limited_pmc(idx)) {
			cpuhw->limited_counter[n_lim] = event;
			cpuhw->limited_hwidx[n_lim] = idx;
			++n_lim;
			continue;
		}
		val = 0;
		if (event->hw.sample_period) {
			left = local64_read(&event->hw.period_left);
			if (left < 0x80000000L)
				val = 0x80000000L - left;
		}
		local64_set(&event->hw.prev_count, val);
		event->hw.idx = idx;
		if (event->hw.state & PERF_HES_STOPPED)
			val = 0;
		write_pmc(idx, val);
		perf_event_update_userpage(event);
	}
	cpuhw->n_limited = n_lim;
	cpuhw->mmcr[0] |= MMCR0_PMXE | MMCR0_FCECE;

 out_enable:
	mb();
	write_mmcr0(cpuhw, cpuhw->mmcr[0]);

	/*
                                            
  */
	if (cpuhw->mmcr[2] & MMCRA_SAMPLE_ENABLE) {
		mb();
		mtspr(SPRN_MMCRA, cpuhw->mmcr[2]);
	}

 out:
	if (cpuhw->bhrb_users)
		ppmu->config_bhrb(cpuhw->bhrb_filter);

	local_irq_restore(flags);
}

static int collect_events(struct perf_event *group, int max_count,
			  struct perf_event *ctrs[], u64 *events,
			  unsigned int *flags)
{
	int n = 0;
	struct perf_event *event;

	if (!is_software_event(group)) {
		if (n >= max_count)
			return -1;
		ctrs[n] = group;
		flags[n] = group->hw.event_base;
		events[n++] = group->hw.config;
	}
	list_for_each_entry(event, &group->sibling_list, group_entry) {
		if (!is_software_event(event) &&
		    event->state != PERF_EVENT_STATE_OFF) {
			if (n >= max_count)
				return -1;
			ctrs[n] = event;
			flags[n] = event->hw.event_base;
			events[n++] = event->hw.config;
		}
	}
	return n;
}

/*
                          
                                                            
                                                             
                                        
 */
static int power_pmu_add(struct perf_event *event, int ef_flags)
{
	struct cpu_hw_events *cpuhw;
	unsigned long flags;
	int n0;
	int ret = -EAGAIN;

	local_irq_save(flags);
	perf_pmu_disable(event->pmu);

	/*
                                                
                                                      
  */
	cpuhw = &__get_cpu_var(cpu_hw_events);
	n0 = cpuhw->n_events;
	if (n0 >= ppmu->n_counter)
		goto out;
	cpuhw->event[n0] = event;
	cpuhw->events[n0] = event->hw.config;
	cpuhw->flags[n0] = event->hw.event_base;

	/*
                                                                     
                                                                    
                                                                  
                               
  */
	if (!(ef_flags & PERF_EF_START))
		event->hw.state = PERF_HES_STOPPED | PERF_HES_UPTODATE;
	else
		event->hw.state = 0;

	/*
                                                       
                                                           
                                           
  */
	if (cpuhw->group_flag & PERF_EVENT_TXN)
		goto nocheck;

	if (check_excludes(cpuhw->event, cpuhw->flags, n0, 1))
		goto out;
	if (power_check_constraints(cpuhw, cpuhw->events, cpuhw->flags, n0 + 1))
		goto out;
	event->hw.config = cpuhw->events[n0];

nocheck:
	++cpuhw->n_events;
	++cpuhw->n_added;

	ret = 0;
 out:
	if (has_branch_stack(event))
		power_pmu_bhrb_enable(event);

	perf_pmu_enable(event->pmu);
	local_irq_restore(flags);
	return ret;
}

/*
                               
 */
static void power_pmu_del(struct perf_event *event, int ef_flags)
{
	struct cpu_hw_events *cpuhw;
	long i;
	unsigned long flags;

	local_irq_save(flags);
	perf_pmu_disable(event->pmu);

	power_pmu_read(event);

	cpuhw = &__get_cpu_var(cpu_hw_events);
	for (i = 0; i < cpuhw->n_events; ++i) {
		if (event == cpuhw->event[i]) {
			while (++i < cpuhw->n_events) {
				cpuhw->event[i-1] = cpuhw->event[i];
				cpuhw->events[i-1] = cpuhw->events[i];
				cpuhw->flags[i-1] = cpuhw->flags[i];
			}
			--cpuhw->n_events;
			ppmu->disable_pmc(event->hw.idx - 1, cpuhw->mmcr);
			if (event->hw.idx) {
				write_pmc(event->hw.idx, 0);
				event->hw.idx = 0;
			}
			perf_event_update_userpage(event);
			break;
		}
	}
	for (i = 0; i < cpuhw->n_limited; ++i)
		if (event == cpuhw->limited_counter[i])
			break;
	if (i < cpuhw->n_limited) {
		while (++i < cpuhw->n_limited) {
			cpuhw->limited_counter[i-1] = cpuhw->limited_counter[i];
			cpuhw->limited_hwidx[i-1] = cpuhw->limited_hwidx[i];
		}
		--cpuhw->n_limited;
	}
	if (cpuhw->n_events == 0) {
		/*                                             */
		cpuhw->mmcr[0] &= ~(MMCR0_PMXE | MMCR0_FCECE);
	}

	if (has_branch_stack(event))
		power_pmu_bhrb_disable(event);

	perf_pmu_enable(event->pmu);
	local_irq_restore(flags);
}

/*
                                                                  
                                                                            
 */

static void power_pmu_start(struct perf_event *event, int ef_flags)
{
	unsigned long flags;
	s64 left;
	unsigned long val;

	if (!event->hw.idx || !event->hw.sample_period)
		return;

	if (!(event->hw.state & PERF_HES_STOPPED))
		return;

	if (ef_flags & PERF_EF_RELOAD)
		WARN_ON_ONCE(!(event->hw.state & PERF_HES_UPTODATE));

	local_irq_save(flags);
	perf_pmu_disable(event->pmu);

	event->hw.state = 0;
	left = local64_read(&event->hw.period_left);

	val = 0;
	if (left < 0x80000000L)
		val = 0x80000000L - left;

	write_pmc(event->hw.idx, val);

	perf_event_update_userpage(event);
	perf_pmu_enable(event->pmu);
	local_irq_restore(flags);
}

static void power_pmu_stop(struct perf_event *event, int ef_flags)
{
	unsigned long flags;

	if (!event->hw.idx || !event->hw.sample_period)
		return;

	if (event->hw.state & PERF_HES_STOPPED)
		return;

	local_irq_save(flags);
	perf_pmu_disable(event->pmu);

	power_pmu_read(event);
	event->hw.state |= PERF_HES_STOPPED | PERF_HES_UPTODATE;
	write_pmc(event->hw.idx, 0);

	perf_event_update_userpage(event);
	perf_pmu_enable(event->pmu);
	local_irq_restore(flags);
}

/*
                                            
                                                     
                                                           
 */
void power_pmu_start_txn(struct pmu *pmu)
{
	struct cpu_hw_events *cpuhw = &__get_cpu_var(cpu_hw_events);

	perf_pmu_disable(pmu);
	cpuhw->group_flag |= PERF_EVENT_TXN;
	cpuhw->n_txn_start = cpuhw->n_events;
}

/*
                                           
                                                    
                       
 */
void power_pmu_cancel_txn(struct pmu *pmu)
{
	struct cpu_hw_events *cpuhw = &__get_cpu_var(cpu_hw_events);

	cpuhw->group_flag &= ~PERF_EVENT_TXN;
	perf_pmu_enable(pmu);
}

/*
                                             
                                                   
                      
 */
int power_pmu_commit_txn(struct pmu *pmu)
{
	struct cpu_hw_events *cpuhw;
	long i, n;

	if (!ppmu)
		return -EAGAIN;
	cpuhw = &__get_cpu_var(cpu_hw_events);
	n = cpuhw->n_events;
	if (check_excludes(cpuhw->event, cpuhw->flags, 0, n))
		return -EAGAIN;
	i = power_check_constraints(cpuhw, cpuhw->events, cpuhw->flags, n);
	if (i < 0)
		return -EAGAIN;

	for (i = cpuhw->n_txn_start; i < n; ++i)
		cpuhw->event[i]->hw.config = cpuhw->events[i];

	cpuhw->group_flag &= ~PERF_EVENT_TXN;
	perf_pmu_enable(pmu);
	return 0;
}

/*
                                                              
               
                                                              
                                                                
                                      
 */
static int can_go_on_limited_pmc(struct perf_event *event, u64 ev,
				 unsigned int flags)
{
	int n;
	u64 alt[MAX_EVENT_ALTERNATIVES];

	if (event->attr.exclude_user
	    || event->attr.exclude_kernel
	    || event->attr.exclude_hv
	    || event->attr.sample_period)
		return 0;

	if (ppmu->limited_pmc_event(ev))
		return 1;

	if (ppmu->flags & PPMU_ARCH_207S)
		mtspr(SPRN_MMCR2, 0);

	/*
                                                          
                                                      
  */
	if (!ppmu->get_alternatives)
		return 0;

	flags |= PPMU_LIMITED_PMC_OK | PPMU_LIMITED_PMC_REQD;
	n = ppmu->get_alternatives(ev, flags, alt);

	return n > 0;
}

/*
                                                                       
                                                                      
                                                            
 */
static u64 normal_pmc_alternative(u64 ev, unsigned long flags)
{
	u64 alt[MAX_EVENT_ALTERNATIVES];
	int n;

	flags &= ~(PPMU_LIMITED_PMC_OK | PPMU_LIMITED_PMC_REQD);
	n = ppmu->get_alternatives(ev, flags, alt);
	if (!n)
		return 0;
	return alt[0];
}

/*                                                */
static atomic_t num_events;
/*                                                             */
static DEFINE_MUTEX(pmc_reserve_mutex);

/*
                                                  
 */
static void hw_perf_event_destroy(struct perf_event *event)
{
	if (!atomic_add_unless(&num_events, -1, 1)) {
		mutex_lock(&pmc_reserve_mutex);
		if (atomic_dec_return(&num_events) == 0)
			release_pmc_hardware();
		mutex_unlock(&pmc_reserve_mutex);
	}
}

/*
                                                                    
 */
static int hw_perf_cache_event(u64 config, u64 *eventp)
{
	unsigned long type, op, result;
	int ev;

	if (!ppmu->cache_events)
		return -EINVAL;

	/*               */
	type = config & 0xff;
	op = (config >> 8) & 0xff;
	result = (config >> 16) & 0xff;

	if (type >= PERF_COUNT_HW_CACHE_MAX ||
	    op >= PERF_COUNT_HW_CACHE_OP_MAX ||
	    result >= PERF_COUNT_HW_CACHE_RESULT_MAX)
		return -EINVAL;

	ev = (*ppmu->cache_events)[type][op][result];
	if (ev == 0)
		return -EOPNOTSUPP;
	if (ev == -1)
		return -EINVAL;
	*eventp = ev;
	return 0;
}

static int power_pmu_event_init(struct perf_event *event)
{
	u64 ev;
	unsigned long flags;
	struct perf_event *ctrs[MAX_HWEVENTS];
	u64 events[MAX_HWEVENTS];
	unsigned int cflags[MAX_HWEVENTS];
	int n;
	int err;
	struct cpu_hw_events *cpuhw;

	if (!ppmu)
		return -ENOENT;

	if (has_branch_stack(event)) {
	        /*                      */
		if (!(ppmu->flags & PPMU_ARCH_207S))
			return -EOPNOTSUPP;
	}

	switch (event->attr.type) {
	case PERF_TYPE_HARDWARE:
		ev = event->attr.config;
		if (ev >= ppmu->n_generic || ppmu->generic_events[ev] == 0)
			return -EOPNOTSUPP;
		ev = ppmu->generic_events[ev];
		break;
	case PERF_TYPE_HW_CACHE:
		err = hw_perf_cache_event(event->attr.config, &ev);
		if (err)
			return err;
		break;
	case PERF_TYPE_RAW:
		ev = event->attr.config;
		break;
	default:
		return -ENOENT;
	}

	event->hw.config_base = ev;
	event->hw.idx = 0;

	/*
                                                    
                                                  
                       
  */
	if (!firmware_has_feature(FW_FEATURE_LPAR))
		event->attr.exclude_hv = 0;

	/*
                                                
                                                        
                                                   
                                                    
  */
	flags = 0;
	if (event->attach_state & PERF_ATTACH_TASK)
		flags |= PPMU_ONLY_COUNT_RUN;

	/*
                                                          
                                         
  */
	if (ppmu->flags & PPMU_LIMITED_PMC5_6) {
		if (can_go_on_limited_pmc(event, ev, flags)) {
			flags |= PPMU_LIMITED_PMC_OK;
		} else if (ppmu->limited_pmc_event(ev)) {
			/*
                                                 
                                                
                                       
    */
			ev = normal_pmc_alternative(ev, flags);
			if (!ev)
				return -EINVAL;
		}
	}

	/*
                                                             
                                                            
                                                                    
  */
	n = 0;
	if (event->group_leader != event) {
		n = collect_events(event->group_leader, ppmu->n_counter - 1,
				   ctrs, events, cflags);
		if (n < 0)
			return -EINVAL;
	}
	events[n] = ev;
	ctrs[n] = event;
	cflags[n] = flags;
	if (check_excludes(ctrs, cflags, n, 1))
		return -EINVAL;

	cpuhw = &get_cpu_var(cpu_hw_events);
	err = power_check_constraints(cpuhw, events, cflags, n + 1);

	if (has_branch_stack(event)) {
		cpuhw->bhrb_filter = ppmu->bhrb_filter_map(
					event->attr.branch_sample_type);

		if(cpuhw->bhrb_filter == -1)
			return -EOPNOTSUPP;
	}

	put_cpu_var(cpu_hw_events);
	if (err)
		return -EINVAL;

	event->hw.config = events[n];
	event->hw.event_base = cflags[n];
	event->hw.last_period = event->hw.sample_period;
	local64_set(&event->hw.period_left, event->hw.last_period);

	/*
                                      
                                                             
                                                              
                                                 
  */
	err = 0;
	if (!atomic_inc_not_zero(&num_events)) {
		mutex_lock(&pmc_reserve_mutex);
		if (atomic_read(&num_events) == 0 &&
		    reserve_pmc_hardware(perf_event_interrupt))
			err = -EBUSY;
		else
			atomic_inc(&num_events);
		mutex_unlock(&pmc_reserve_mutex);
	}
	event->destroy = hw_perf_event_destroy;

	return err;
}

static int power_pmu_event_idx(struct perf_event *event)
{
	return event->hw.idx;
}

ssize_t power_events_sysfs_show(struct device *dev,
				struct device_attribute *attr, char *page)
{
	struct perf_pmu_events_attr *pmu_attr;

	pmu_attr = container_of(attr, struct perf_pmu_events_attr, attr);

	return sprintf(page, "event=0x%02llx\n", pmu_attr->id);
}

struct pmu power_pmu = {
	.pmu_enable	= power_pmu_enable,
	.pmu_disable	= power_pmu_disable,
	.event_init	= power_pmu_event_init,
	.add		= power_pmu_add,
	.del		= power_pmu_del,
	.start		= power_pmu_start,
	.stop		= power_pmu_stop,
	.read		= power_pmu_read,
	.start_txn	= power_pmu_start_txn,
	.cancel_txn	= power_pmu_cancel_txn,
	.commit_txn	= power_pmu_commit_txn,
	.event_idx	= power_pmu_event_idx,
	.flush_branch_stack = power_pmu_flush_branch_stack,
};

/*
                                                        
                                                               
                                                        
 */
static void record_and_restart(struct perf_event *event, unsigned long val,
			       struct pt_regs *regs)
{
	u64 period = event->hw.sample_period;
	s64 prev, delta, left;
	int record = 0;

	if (event->hw.state & PERF_HES_STOPPED) {
		write_pmc(event->hw.idx, 0);
		return;
	}

	/*                                              */
	prev = local64_read(&event->hw.prev_count);
	delta = check_and_compute_delta(prev, val);
	local64_add(delta, &event->count);

	/*
                                                       
                                   
  */
	val = 0;
	left = local64_read(&event->hw.period_left) - delta;
	if (delta == 0)
		left++;
	if (period) {
		if (left <= 0) {
			left += period;
			if (left <= 0)
				left = period;
			record = siar_valid(regs);
			event->hw.last_period = event->hw.sample_period;
		}
		if (left < 0x80000000LL)
			val = 0x80000000LL - left;
	}

	write_pmc(event->hw.idx, val);
	local64_set(&event->hw.prev_count, val);
	local64_set(&event->hw.period_left, left);
	perf_event_update_userpage(event);

	/*
                                     
  */
	if (record) {
		struct perf_sample_data data;

		perf_sample_data_init(&data, ~0ULL, event->hw.last_period);

		if (event->attr.sample_type & PERF_SAMPLE_ADDR)
			perf_get_data_addr(regs, &data.addr);

		if (event->attr.sample_type & PERF_SAMPLE_BRANCH_STACK) {
			struct cpu_hw_events *cpuhw;
			cpuhw = &__get_cpu_var(cpu_hw_events);
			power_pmu_bhrb_read(cpuhw);
			data.br_stack = &cpuhw->bhrb_stack;
		}

		if (perf_event_overflow(event, &data, regs))
			power_pmu_stop(event, 0);
	}
}

/*
                                                                       
                   
 */
unsigned long perf_misc_flags(struct pt_regs *regs)
{
	u32 flags = perf_get_misc_flags(regs);

	if (flags)
		return flags;
	return user_mode(regs) ? PERF_RECORD_MISC_USER :
		PERF_RECORD_MISC_KERNEL;
}

/*
                                                          
                   
 */
unsigned long perf_instruction_pointer(struct pt_regs *regs)
{
	bool use_siar = regs_use_siar(regs);

	if (use_siar && siar_valid(regs))
		return mfspr(SPRN_SIAR) + perf_ip_adjust(regs);
	else if (use_siar)
		return 0;		//                             
	else
		return regs->nip;
}

static bool pmc_overflow_power7(unsigned long val)
{
	/*
                                                                 
                                                                   
                                                                   
                                                                     
                         
   
                                                                   
                                                                  
                                        
  */
	if ((0x80000000 - val) <= 256)
		return true;

	return false;
}

static bool pmc_overflow(unsigned long val)
{
	if ((int)val < 0)
		return true;

	return false;
}

/*
                                      
 */
static void perf_event_interrupt(struct pt_regs *regs)
{
	int i, j;
	struct cpu_hw_events *cpuhw = &__get_cpu_var(cpu_hw_events);
	struct perf_event *event;
	unsigned long val[8];
	int found, active;
	int nmi;

	if (cpuhw->n_limited)
		freeze_limited_counters(cpuhw, mfspr(SPRN_PMC5),
					mfspr(SPRN_PMC6));

	perf_read_regs(regs);

	nmi = perf_intr_is_nmi(regs);
	if (nmi)
		nmi_enter();
	else
		irq_enter();

	/*                                                          */
	for (i = 0; i < ppmu->n_counter; ++i)
		val[i] = read_pmc(i + 1);

	/*                                 */
	found = 0;
	for (i = 0; i < ppmu->n_counter; ++i) {
		if (!pmc_overflow(val[i]))
			continue;
		if (is_limited_pmc(i + 1))
			continue; /*                           */
		/*
                                                   
                                                
                                         
   */
		found = 1;
		active = 0;
		for (j = 0; j < cpuhw->n_events; ++j) {
			event = cpuhw->event[j];
			if (event->hw.idx == (i + 1)) {
				active = 1;
				record_and_restart(event, val[i], regs);
				break;
			}
		}
		if (!active)
			/*                                                */
			write_pmc(i + 1, 0);
	}
	if (!found && pvr_version_is(PVR_POWER7)) {
		/*                                                     */
		for (i = 0; i < cpuhw->n_events; ++i) {
			event = cpuhw->event[i];
			if (!event->hw.idx || is_limited_pmc(event->hw.idx))
				continue;
			if (pmc_overflow_power7(val[event->hw.idx - 1])) {
				/*                                    */
				found = 1;
				record_and_restart(event,
						   val[event->hw.idx - 1],
						   regs);
			}
		}
	}
	if (!found && !nmi && printk_ratelimit())
		printk(KERN_WARNING "Can't find PMC that caused IRQ\n");

	/*
                                                            
                                                                 
                                             
                                                                
                                      
  */
	write_mmcr0(cpuhw, cpuhw->mmcr[0]);

	if (nmi)
		nmi_exit();
	else
		irq_exit();
}

static void power_pmu_setup(int cpu)
{
	struct cpu_hw_events *cpuhw = &per_cpu(cpu_hw_events, cpu);

	if (!ppmu)
		return;
	memset(cpuhw, 0, sizeof(*cpuhw));
	cpuhw->mmcr[0] = MMCR0_FC;
}

static int __cpuinit
power_pmu_notifier(struct notifier_block *self, unsigned long action, void *hcpu)
{
	unsigned int cpu = (long)hcpu;

	switch (action & ~CPU_TASKS_FROZEN) {
	case CPU_UP_PREPARE:
		power_pmu_setup(cpu);
		break;

	default:
		break;
	}

	return NOTIFY_OK;
}

int __cpuinit register_power_pmu(struct power_pmu *pmu)
{
	if (ppmu)
		return -EBUSY;		/*                                */

	ppmu = pmu;
	pr_info("%s performance monitor hardware support registered\n",
		pmu->name);

	power_pmu.attr_groups = ppmu->attr_groups;

#ifdef MSR_HV
	/*
                                                      
  */
	if (mfmsr() & MSR_HV)
		freeze_events_kernel = MMCR0_FCHV;
#endif /*              */

	perf_pmu_register(&power_pmu, "cpu", PERF_TYPE_RAW);
	perf_cpu_notifier(power_pmu_notifier);

	return 0;
}
