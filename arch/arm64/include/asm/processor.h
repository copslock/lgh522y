/*
 * Based on arch/arm/include/asm/processor.h
 *
 * Copyright (C) 1995-1999 Russell King
 * Copyright (C) 2012 ARM Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __ASM_PROCESSOR_H
#define __ASM_PROCESSOR_H

/*
                                                       
                                           
 */
#define current_text_addr() ({ __label__ _l; _l: &&_l;})

#ifdef __KERNEL__

#include <linux/string.h>

#include <asm/fpsimd.h>
#include <asm/hw_breakpoint.h>
#include <asm/ptrace.h>
#include <asm/types.h>

#ifdef __KERNEL__
#define STACK_TOP_MAX		TASK_SIZE_64
#ifdef CONFIG_COMPAT
#define AARCH32_VECTORS_BASE	0xffff0000
#define STACK_TOP		(test_thread_flag(TIF_32BIT) ? \
				AARCH32_VECTORS_BASE : STACK_TOP_MAX)
#else
#define STACK_TOP		STACK_TOP_MAX
#endif /*               */

#define ARCH_LOW_ADDRESS_LIMIT	PHYS_MASK
#endif /*            */

struct debug_info {
	/*                                           */
	int			suspended_step;
	/*                                                                   */
	int			bps_disabled;
	int			wps_disabled;
	/*                                           */
	struct perf_event	*hbp_break[ARM_MAX_BRP];
	struct perf_event	*hbp_watch[ARM_MAX_WRP];
};

struct cpu_context {
	unsigned long x19;
	unsigned long x20;
	unsigned long x21;
	unsigned long x22;
	unsigned long x23;
	unsigned long x24;
	unsigned long x25;
	unsigned long x26;
	unsigned long x27;
	unsigned long x28;
	unsigned long fp;
	unsigned long sp;
	unsigned long pc;
};

struct thread_struct {
	struct cpu_context	cpu_context;	/*             */
	unsigned long		tp_value;
	struct fpsimd_state	fpsimd_state;
	unsigned long		fault_address;	/*            */
	struct debug_info	debug;		/*           */
};

#define INIT_THREAD  {	}

static inline void start_thread_common(struct pt_regs *regs, unsigned long pc)
{
	memset(regs, 0, sizeof(*regs));
	regs->syscallno = ~0UL;
	regs->pc = pc;
}

static inline void start_thread(struct pt_regs *regs, unsigned long pc,
				unsigned long sp)
{
	start_thread_common(regs, pc);
	regs->pstate = PSR_MODE_EL0t;
	regs->sp = sp;
}

#ifdef CONFIG_COMPAT
static inline void compat_start_thread(struct pt_regs *regs, unsigned long pc,
				       unsigned long sp)
{
	start_thread_common(regs, pc);
	regs->pstate = COMPAT_PSR_MODE_USR;
	if (pc & 1)
		regs->pstate |= COMPAT_PSR_T_BIT;
	regs->compat_sp = sp;
}
#endif

/*                                        */
struct task_struct;

/*                                      */
extern void release_thread(struct task_struct *);

/*                                                       */
#define prepare_to_copy(tsk)	do { } while (0)

unsigned long get_wchan(struct task_struct *p);

#define cpu_relax()			barrier()

/*                  */
extern struct task_struct *cpu_switch_to(struct task_struct *prev,
					 struct task_struct *next);

#define task_pt_regs(p) \
	((struct pt_regs *)(THREAD_START_SP + task_stack_page(p)) - 1)

#define KSTK_EIP(tsk)	((unsigned long)task_pt_regs(tsk)->pc)
#define KSTK_ESP(tsk)	user_stack_pointer(task_pt_regs(tsk))

/*
                      
 */
#define ARCH_HAS_PREFETCH
static inline void prefetch(const void *ptr)
{
	asm volatile("prfm pldl1keep, %a0\n" : : "p" (ptr));
}

#define ARCH_HAS_PREFETCHW
static inline void prefetchw(const void *ptr)
{
	asm volatile("prfm pstl1keep, %a0\n" : : "p" (ptr));
}

#define ARCH_HAS_SPINLOCK_PREFETCH
static inline void spin_lock_prefetch(const void *x)
{
	prefetchw(x);
}

#define HAVE_ARCH_PICK_MMAP_LAYOUT

#endif

#endif /*                   */
