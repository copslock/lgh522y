/*
 * Signal handling for 32bit PPC and 32bit tasks on 64bit PPC
 *
 *  PowerPC version
 *    Copyright (C) 1995-1996 Gary Thomas (gdt@linuxppc.org)
 * Copyright (C) 2001 IBM
 * Copyright (C) 1997,1998 Jakub Jelinek (jj@sunsite.mff.cuni.cz)
 * Copyright (C) 1997 David S. Miller (davem@caip.rutgers.edu)
 *
 *  Derived from "arch/i386/kernel/signal.c"
 *    Copyright (C) 1991, 1992 Linus Torvalds
 *    1997-11-28  Modified for POSIX.1b signals by Richard Henderson
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 */

#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/smp.h>
#include <linux/kernel.h>
#include <linux/signal.h>
#include <linux/errno.h>
#include <linux/elf.h>
#include <linux/ptrace.h>
#include <linux/ratelimit.h>
#ifdef CONFIG_PPC64
#include <linux/syscalls.h>
#include <linux/compat.h>
#else
#include <linux/wait.h>
#include <linux/unistd.h>
#include <linux/stddef.h>
#include <linux/tty.h>
#include <linux/binfmts.h>
#endif

#include <asm/uaccess.h>
#include <asm/cacheflush.h>
#include <asm/syscalls.h>
#include <asm/sigcontext.h>
#include <asm/vdso.h>
#include <asm/switch_to.h>
#include <asm/tm.h>
#ifdef CONFIG_PPC64
#include "ppc32.h"
#include <asm/unistd.h>
#else
#include <asm/ucontext.h>
#include <asm/pgtable.h>
#endif

#include "signal.h"

#undef DEBUG_SIG

#ifdef CONFIG_PPC64
#define sys_rt_sigreturn	compat_sys_rt_sigreturn
#define sys_swapcontext	compat_sys_swapcontext
#define sys_sigreturn	compat_sys_sigreturn

#define old_sigaction	old_sigaction32
#define sigcontext	sigcontext32
#define mcontext	mcontext32
#define ucontext	ucontext32

#define __save_altstack __compat_save_altstack

/*
                                                                     
                                               
 */
#define UCONTEXTSIZEWITHOUTVSX \
		(sizeof(struct ucontext) - sizeof(elf_vsrreghalf_t32))

/*
                                               
                                            
                                              
                                          
 */

#define GP_REGS_SIZE	min(sizeof(elf_gregset_t32), sizeof(struct pt_regs32))
#undef __SIGNAL_FRAMESIZE
#define __SIGNAL_FRAMESIZE	__SIGNAL_FRAMESIZE32
#undef ELF_NVRREG
#define ELF_NVRREG	ELF_NVRREG32

/*
                                                               
                                                                  
 */
static inline int put_sigset_t(compat_sigset_t __user *uset, sigset_t *set)
{
	compat_sigset_t	cset;

	switch (_NSIG_WORDS) {
	case 4: cset.sig[6] = set->sig[3] & 0xffffffffull;
		cset.sig[7] = set->sig[3] >> 32;
	case 3: cset.sig[4] = set->sig[2] & 0xffffffffull;
		cset.sig[5] = set->sig[2] >> 32;
	case 2: cset.sig[2] = set->sig[1] & 0xffffffffull;
		cset.sig[3] = set->sig[1] >> 32;
	case 1: cset.sig[0] = set->sig[0] & 0xffffffffull;
		cset.sig[1] = set->sig[0] >> 32;
	}
	return copy_to_user(uset, &cset, sizeof(*uset));
}

static inline int get_sigset_t(sigset_t *set,
			       const compat_sigset_t __user *uset)
{
	compat_sigset_t s32;

	if (copy_from_user(&s32, uset, sizeof(*uset)))
		return -EFAULT;

	/*
                                                            
                                                  
  */
	switch (_NSIG_WORDS) {
	case 4: set->sig[3] = s32.sig[6] | (((long)s32.sig[7]) << 32);
	case 3: set->sig[2] = s32.sig[4] | (((long)s32.sig[5]) << 32);
	case 2: set->sig[1] = s32.sig[2] | (((long)s32.sig[3]) << 32);
	case 1: set->sig[0] = s32.sig[0] | (((long)s32.sig[1]) << 32);
	}
	return 0;
}

#define to_user_ptr(p)		ptr_to_compat(p)
#define from_user_ptr(p)	compat_ptr(p)

static inline int save_general_regs(struct pt_regs *regs,
		struct mcontext __user *frame)
{
	elf_greg_t64 *gregs = (elf_greg_t64 *)regs;
	int i;

	WARN_ON(!FULL_REGS(regs));

	for (i = 0; i <= PT_RESULT; i ++) {
		if (i == 14 && !FULL_REGS(regs))
			i = 32;
		if (__put_user((unsigned int)gregs[i], &frame->mc_gregs[i]))
			return -EFAULT;
	}
	return 0;
}

static inline int restore_general_regs(struct pt_regs *regs,
		struct mcontext __user *sr)
{
	elf_greg_t64 *gregs = (elf_greg_t64 *)regs;
	int i;

	for (i = 0; i <= PT_RESULT; i++) {
		if ((i == PT_MSR) || (i == PT_SOFTE))
			continue;
		if (__get_user(gregs[i], &sr->mc_gregs[i]))
			return -EFAULT;
	}
	return 0;
}

#else /*              */

#define GP_REGS_SIZE	min(sizeof(elf_gregset_t), sizeof(struct pt_regs))

static inline int put_sigset_t(sigset_t __user *uset, sigset_t *set)
{
	return copy_to_user(uset, set, sizeof(*uset));
}

static inline int get_sigset_t(sigset_t *set, const sigset_t __user *uset)
{
	return copy_from_user(set, uset, sizeof(*uset));
}

#define to_user_ptr(p)		((unsigned long)(p))
#define from_user_ptr(p)	((void __user *)(p))

static inline int save_general_regs(struct pt_regs *regs,
		struct mcontext __user *frame)
{
	WARN_ON(!FULL_REGS(regs));
	return __copy_to_user(&frame->mc_gregs, regs, GP_REGS_SIZE);
}

static inline int restore_general_regs(struct pt_regs *regs,
		struct mcontext __user *sr)
{
	/*                                  */
	if (__copy_from_user(regs, &sr->mc_gregs,
				PT_MSR * sizeof(elf_greg_t)))
		return -EFAULT;
	/*                                                          */
	if (__copy_from_user(&regs->orig_gpr3, &sr->mc_gregs[PT_ORIG_R3],
				GP_REGS_SIZE - PT_ORIG_R3 * sizeof(elf_greg_t)))
		return -EFAULT;
	return 0;
}
#endif

/*
                                                    
                                                          
                         
                     
                      
                                    
  
                                                                             
                                                                    
  
 */
struct sigframe {
	struct sigcontext sctx;		/*                */
	struct mcontext	mctx;		/*                         */
#ifdef CONFIG_PPC_TRANSACTIONAL_MEM
	struct sigcontext sctx_transact;
	struct mcontext	mctx_transact;
#endif
	/*
                                                            
                                                        
  */
	int			abigap[56];
};

/*                                                           */
#define tramp	mc_pad

/*
                                                        
                                                           
                                                        
                                       
                                                           
                                   
  
                                                                
  
 */
struct rt_sigframe {
#ifdef CONFIG_PPC64
	compat_siginfo_t info;
#else
	struct siginfo info;
#endif
	struct ucontext	uc;
#ifdef CONFIG_PPC_TRANSACTIONAL_MEM
	struct ucontext	uc_transact;
#endif
	/*
                                                            
                                                        
  */
	int			abigap[56];
};

#ifdef CONFIG_VSX
unsigned long copy_fpr_to_user(void __user *to,
			       struct task_struct *task)
{
	double buf[ELF_NFPREG];
	int i;

	/*                                                               */
	for (i = 0; i < (ELF_NFPREG - 1) ; i++)
		buf[i] = task->thread.TS_FPR(i);
	memcpy(&buf[i], &task->thread.fpscr, sizeof(double));
	return __copy_to_user(to, buf, ELF_NFPREG * sizeof(double));
}

unsigned long copy_fpr_from_user(struct task_struct *task,
				 void __user *from)
{
	double buf[ELF_NFPREG];
	int i;

	if (__copy_from_user(buf, from, ELF_NFPREG * sizeof(double)))
		return 1;
	for (i = 0; i < (ELF_NFPREG - 1) ; i++)
		task->thread.TS_FPR(i) = buf[i];
	memcpy(&task->thread.fpscr, &buf[i], sizeof(double));

	return 0;
}

unsigned long copy_vsx_to_user(void __user *to,
			       struct task_struct *task)
{
	double buf[ELF_NVSRHALFREG];
	int i;

	/*                                                               */
	for (i = 0; i < ELF_NVSRHALFREG; i++)
		buf[i] = task->thread.fpr[i][TS_VSRLOWOFFSET];
	return __copy_to_user(to, buf, ELF_NVSRHALFREG * sizeof(double));
}

unsigned long copy_vsx_from_user(struct task_struct *task,
				 void __user *from)
{
	double buf[ELF_NVSRHALFREG];
	int i;

	if (__copy_from_user(buf, from, ELF_NVSRHALFREG * sizeof(double)))
		return 1;
	for (i = 0; i < ELF_NVSRHALFREG ; i++)
		task->thread.fpr[i][TS_VSRLOWOFFSET] = buf[i];
	return 0;
}

#ifdef CONFIG_PPC_TRANSACTIONAL_MEM
unsigned long copy_transact_fpr_to_user(void __user *to,
				  struct task_struct *task)
{
	double buf[ELF_NFPREG];
	int i;

	/*                                                               */
	for (i = 0; i < (ELF_NFPREG - 1) ; i++)
		buf[i] = task->thread.TS_TRANS_FPR(i);
	memcpy(&buf[i], &task->thread.transact_fpscr, sizeof(double));
	return __copy_to_user(to, buf, ELF_NFPREG * sizeof(double));
}

unsigned long copy_transact_fpr_from_user(struct task_struct *task,
					  void __user *from)
{
	double buf[ELF_NFPREG];
	int i;

	if (__copy_from_user(buf, from, ELF_NFPREG * sizeof(double)))
		return 1;
	for (i = 0; i < (ELF_NFPREG - 1) ; i++)
		task->thread.TS_TRANS_FPR(i) = buf[i];
	memcpy(&task->thread.transact_fpscr, &buf[i], sizeof(double));

	return 0;
}

unsigned long copy_transact_vsx_to_user(void __user *to,
				  struct task_struct *task)
{
	double buf[ELF_NVSRHALFREG];
	int i;

	/*                                                               */
	for (i = 0; i < ELF_NVSRHALFREG; i++)
		buf[i] = task->thread.transact_fpr[i][TS_VSRLOWOFFSET];
	return __copy_to_user(to, buf, ELF_NVSRHALFREG * sizeof(double));
}

unsigned long copy_transact_vsx_from_user(struct task_struct *task,
					  void __user *from)
{
	double buf[ELF_NVSRHALFREG];
	int i;

	if (__copy_from_user(buf, from, ELF_NVSRHALFREG * sizeof(double)))
		return 1;
	for (i = 0; i < ELF_NVSRHALFREG ; i++)
		task->thread.transact_fpr[i][TS_VSRLOWOFFSET] = buf[i];
	return 0;
}
#endif /*                              */
#else
inline unsigned long copy_fpr_to_user(void __user *to,
				      struct task_struct *task)
{
	return __copy_to_user(to, task->thread.fpr,
			      ELF_NFPREG * sizeof(double));
}

inline unsigned long copy_fpr_from_user(struct task_struct *task,
					void __user *from)
{
	return __copy_from_user(task->thread.fpr, from,
			      ELF_NFPREG * sizeof(double));
}

#ifdef CONFIG_PPC_TRANSACTIONAL_MEM
inline unsigned long copy_transact_fpr_to_user(void __user *to,
					 struct task_struct *task)
{
	return __copy_to_user(to, task->thread.transact_fpr,
			      ELF_NFPREG * sizeof(double));
}

inline unsigned long copy_transact_fpr_from_user(struct task_struct *task,
						 void __user *from)
{
	return __copy_from_user(task->thread.transact_fpr, from,
				ELF_NFPREG * sizeof(double));
}
#endif /*                              */
#endif

/*
                                                     
                                                                 
                                          
 */
static int save_user_regs(struct pt_regs *regs, struct mcontext __user *frame,
			  struct mcontext __user *tm_frame, int sigret,
			  int ctx_has_vsx_region)
{
	unsigned long msr = regs->msr;

	/*                                                       */
	flush_fp_to_thread(current);

	/*                        */
	if (save_general_regs(regs, frame))
		return 1;

#ifdef CONFIG_ALTIVEC
	/*                        */
	if (current->thread.used_vr) {
		flush_altivec_to_thread(current);
		if (__copy_to_user(&frame->mc_vregs, current->thread.vr,
				   ELF_NVRREG * sizeof(vector128)))
			return 1;
		/*                                                    
                                         */
		msr |= MSR_VEC;
	}
	/*                                         */

	/*                                                                
                                                                    
                                                                    
                                                     
  */
	if (__put_user(current->thread.vrsave, (u32 __user *)&frame->mc_vregs[32]))
		return 1;
#endif /*                */
	if (copy_fpr_to_user(&frame->mc_fregs, current))
		return 1;

	/*
                                                                      
                                                                       
  */
	msr &= ~MSR_VSX;
#ifdef CONFIG_VSX
	/*
                                                        
                                                              
                                                        
                       
  */
	if (current->thread.used_vsr && ctx_has_vsx_region) {
		__giveup_vsx(current);
		if (copy_vsx_to_user(&frame->mc_vsregs, current))
			return 1;
		msr |= MSR_VSX;
	}
#endif /*            */
#ifdef CONFIG_SPE
	/*                    */
	if (current->thread.used_spe) {
		flush_spe_to_thread(current);
		if (__copy_to_user(&frame->mc_vregs, current->thread.evr,
				   ELF_NEVRREG * sizeof(u32)))
			return 1;
		/*                                                    
                                         */
		msr |= MSR_SPE;
	}
	/*                                         */

	/*                                */
	if (__put_user(current->thread.spefscr, (u32 __user *)&frame->mc_vregs + ELF_NEVRREG))
		return 1;
#endif /*            */

	if (__put_user(msr, &frame->mc_gregs[PT_MSR]))
		return 1;
	/*                                                                  
                                                      
  */
	if (tm_frame && __put_user(0, &tm_frame->mc_gregs[PT_MSR]))
		return 1;

	if (sigret) {
		/*                                                   */
		if (__put_user(0x38000000UL + sigret, &frame->tramp[0])
		    || __put_user(0x44000002UL, &frame->tramp[1]))
			return 1;
		flush_icache_range((unsigned long) &frame->tramp[0],
				   (unsigned long) &frame->tramp[2]);
	}

	return 0;
}

#ifdef CONFIG_PPC_TRANSACTIONAL_MEM
/*
                                                     
                                                                 
                                          
                                                                       
         
  
                                                               
 */
static int save_tm_user_regs(struct pt_regs *regs,
			     struct mcontext __user *frame,
			     struct mcontext __user *tm_frame, int sigret)
{
	unsigned long msr = regs->msr;

	/*                                                       */
	flush_fp_to_thread(current);

	/*                                     */
	if (save_general_regs(&current->thread.ckpt_regs, frame)
	    || save_general_regs(regs, tm_frame))
		return 1;

	/*                                                            
                                                                          
                                                                        
                                                                    
                       
  */
	if (__put_user((msr >> 32), &tm_frame->mc_gregs[PT_MSR]))
		return 1;

#ifdef CONFIG_ALTIVEC
	/*                        */
	if (current->thread.used_vr) {
		flush_altivec_to_thread(current);
		if (__copy_to_user(&frame->mc_vregs, current->thread.vr,
				   ELF_NVRREG * sizeof(vector128)))
			return 1;
		if (msr & MSR_VEC) {
			if (__copy_to_user(&tm_frame->mc_vregs,
					   current->thread.transact_vr,
					   ELF_NVRREG * sizeof(vector128)))
				return 1;
		} else {
			if (__copy_to_user(&tm_frame->mc_vregs,
					   current->thread.vr,
					   ELF_NVRREG * sizeof(vector128)))
				return 1;
		}

		/*                                                    
                                        
   */
		msr |= MSR_VEC;
	}

	/*                                                                
                                                                    
                                                                    
                                                     
  */
	if (__put_user(current->thread.vrsave,
		       (u32 __user *)&frame->mc_vregs[32]))
		return 1;
	if (msr & MSR_VEC) {
		if (__put_user(current->thread.transact_vrsave,
			       (u32 __user *)&tm_frame->mc_vregs[32]))
			return 1;
	} else {
		if (__put_user(current->thread.vrsave,
			       (u32 __user *)&tm_frame->mc_vregs[32]))
			return 1;
	}
#endif /*                */

	if (copy_fpr_to_user(&frame->mc_fregs, current))
		return 1;
	if (msr & MSR_FP) {
		if (copy_transact_fpr_to_user(&tm_frame->mc_fregs, current))
			return 1;
	} else {
		if (copy_fpr_to_user(&tm_frame->mc_fregs, current))
			return 1;
	}

#ifdef CONFIG_VSX
	/*
                                                        
                                                              
                                                        
                       
  */
	if (current->thread.used_vsr) {
		__giveup_vsx(current);
		if (copy_vsx_to_user(&frame->mc_vsregs, current))
			return 1;
		if (msr & MSR_VSX) {
			if (copy_transact_vsx_to_user(&tm_frame->mc_vsregs,
						      current))
				return 1;
		} else {
			if (copy_vsx_to_user(&tm_frame->mc_vsregs, current))
				return 1;
		}

		msr |= MSR_VSX;
	}
#endif /*            */
#ifdef CONFIG_SPE
	/*                                                          
                                           
  */
	if (current->thread.used_spe) {
		flush_spe_to_thread(current);
		if (__copy_to_user(&frame->mc_vregs, current->thread.evr,
				   ELF_NEVRREG * sizeof(u32)))
			return 1;
		/*                                                    
                                         */
		msr |= MSR_SPE;
	}

	/*                                */
	if (__put_user(current->thread.spefscr, (u32 __user *)&frame->mc_vregs + ELF_NEVRREG))
		return 1;
#endif /*            */

	if (__put_user(msr, &frame->mc_gregs[PT_MSR]))
		return 1;
	if (sigret) {
		/*                                                   */
		if (__put_user(0x38000000UL + sigret, &frame->tramp[0])
		    || __put_user(0x44000002UL, &frame->tramp[1]))
			return 1;
		flush_icache_range((unsigned long) &frame->tramp[0],
				   (unsigned long) &frame->tramp[2]);
	}

	return 0;
}
#endif

/*
                                                                
                    
 */
static long restore_user_regs(struct pt_regs *regs,
			      struct mcontext __user *sr, int sig)
{
	long err;
	unsigned int save_r2 = 0;
	unsigned long msr;
#ifdef CONFIG_VSX
	int i;
#endif

	/*
                                                                  
                                                        
  */
	if (!sig)
		save_r2 = (unsigned int)regs->gpr[2];
	err = restore_general_regs(regs, sr);
	regs->trap = 0;
	err |= __get_user(msr, &sr->mc_gregs[PT_MSR]);
	if (!sig)
		regs->gpr[2] = (unsigned long) save_r2;
	if (err)
		return 1;

	/*                                                                 */
	if (sig)
		regs->msr = (regs->msr & ~MSR_LE) | (msr & MSR_LE);

	/*
                                               
                                                              
                                                           
                                                                
                                          
  */
	discard_lazy_cpu_state();

#ifdef CONFIG_ALTIVEC
	/*
                                                          
                                                          
  */
	regs->msr &= ~MSR_VEC;
	if (msr & MSR_VEC) {
		/*                                          */
		if (__copy_from_user(current->thread.vr, &sr->mc_vregs,
				     sizeof(sr->mc_vregs)))
			return 1;
	} else if (current->thread.used_vr)
		memset(current->thread.vr, 0, ELF_NVRREG * sizeof(vector128));

	/*                        */
	if (__get_user(current->thread.vrsave, (u32 __user *)&sr->mc_vregs[32]))
		return 1;
#endif /*                */
	if (copy_fpr_from_user(current, &sr->mc_fregs))
		return 1;

#ifdef CONFIG_VSX
	/*
                                                      
                                                      
  */
	regs->msr &= ~MSR_VSX;
	if (msr & MSR_VSX) {
		/*
                                                        
                                                     
   */
		if (copy_vsx_from_user(current, &sr->mc_vsregs))
			return 1;
	} else if (current->thread.used_vsr)
		for (i = 0; i < 32 ; i++)
			current->thread.fpr[i][TS_VSRLOWOFFSET] = 0;
#endif /*            */
	/*
                                                     
                                                     
  */
	regs->msr &= ~(MSR_FP | MSR_FE0 | MSR_FE1);

#ifdef CONFIG_SPE
	/*                                                   
                                                       */
	regs->msr &= ~MSR_SPE;
	if (msr & MSR_SPE) {
		/*                                      */
		if (__copy_from_user(current->thread.evr, &sr->mc_vregs,
				     ELF_NEVRREG * sizeof(u32)))
			return 1;
	} else if (current->thread.used_spe)
		memset(current->thread.evr, 0, ELF_NEVRREG * sizeof(u32));

	/*                         */
	if (__get_user(current->thread.spefscr, (u32 __user *)&sr->mc_vregs + ELF_NEVRREG))
		return 1;
#endif /*            */

	return 0;
}

#ifdef CONFIG_PPC_TRANSACTIONAL_MEM
/*
                                                                           
                                                                               
                   
 */
static long restore_tm_user_regs(struct pt_regs *regs,
				 struct mcontext __user *sr,
				 struct mcontext __user *tm_sr)
{
	long err;
	unsigned long msr, msr_hi;
#ifdef CONFIG_VSX
	int i;
#endif

	/*
                                                                  
                                                         
                                                        
                                                                 
                                    
  */
	err = restore_general_regs(regs, tm_sr);
	err |= restore_general_regs(&current->thread.ckpt_regs, sr);

	err |= __get_user(current->thread.tm_tfhar, &sr->mc_gregs[PT_NIP]);

	err |= __get_user(msr, &sr->mc_gregs[PT_MSR]);
	if (err)
		return 1;

	/*                                         */
	regs->msr = (regs->msr & ~MSR_LE) | (msr & MSR_LE);

	/*
                                               
                                                              
                                                           
                                                                
                                          
  */
	discard_lazy_cpu_state();

#ifdef CONFIG_ALTIVEC
	regs->msr &= ~MSR_VEC;
	if (msr & MSR_VEC) {
		/*                                          */
		if (__copy_from_user(current->thread.vr, &sr->mc_vregs,
				     sizeof(sr->mc_vregs)) ||
		    __copy_from_user(current->thread.transact_vr,
				     &tm_sr->mc_vregs,
				     sizeof(sr->mc_vregs)))
			return 1;
	} else if (current->thread.used_vr) {
		memset(current->thread.vr, 0, ELF_NVRREG * sizeof(vector128));
		memset(current->thread.transact_vr, 0,
		       ELF_NVRREG * sizeof(vector128));
	}

	/*                        */
	if (__get_user(current->thread.vrsave,
		       (u32 __user *)&sr->mc_vregs[32]) ||
	    __get_user(current->thread.transact_vrsave,
		       (u32 __user *)&tm_sr->mc_vregs[32]))
		return 1;
#endif /*                */

	regs->msr &= ~(MSR_FP | MSR_FE0 | MSR_FE1);

	if (copy_fpr_from_user(current, &sr->mc_fregs) ||
	    copy_transact_fpr_from_user(current, &tm_sr->mc_fregs))
		return 1;

#ifdef CONFIG_VSX
	regs->msr &= ~MSR_VSX;
	if (msr & MSR_VSX) {
		/*
                                                        
                                                     
   */
		if (copy_vsx_from_user(current, &sr->mc_vsregs) ||
		    copy_transact_vsx_from_user(current, &tm_sr->mc_vsregs))
			return 1;
	} else if (current->thread.used_vsr)
		for (i = 0; i < 32 ; i++) {
			current->thread.fpr[i][TS_VSRLOWOFFSET] = 0;
			current->thread.transact_fpr[i][TS_VSRLOWOFFSET] = 0;
		}
#endif /*            */

#ifdef CONFIG_SPE
	/*                                                          
                                              
  */
	regs->msr &= ~MSR_SPE;
	if (msr & MSR_SPE) {
		if (__copy_from_user(current->thread.evr, &sr->mc_vregs,
				     ELF_NEVRREG * sizeof(u32)))
			return 1;
	} else if (current->thread.used_spe)
		memset(current->thread.evr, 0, ELF_NEVRREG * sizeof(u32));

	/*                         */
	if (__get_user(current->thread.spefscr, (u32 __user *)&sr->mc_vregs
		       + ELF_NEVRREG))
		return 1;
#endif /*            */

	/*                                                                  
                                                                   
                                            
  */
	tm_enable();
	/*                                               */
	current->thread.tm_texasr |= TEXASR_FS;
	/*                                                   */
	tm_recheckpoint(&current->thread, msr);
	/*                             */
	if (__get_user(msr_hi, &tm_sr->mc_gregs[PT_MSR]))
		return 1;
	/*                                  */
	regs->msr = (regs->msr & ~MSR_TS_MASK) | ((msr_hi<<32) & MSR_TS_MASK);

	/*                                                  */
	if (msr & MSR_FP) {
		do_load_up_transact_fpu(&current->thread);
		regs->msr |= (MSR_FP | current->thread.fpexc_mode);
	}
#ifdef CONFIG_ALTIVEC
	if (msr & MSR_VEC) {
		do_load_up_transact_altivec(&current->thread);
		regs->msr |= MSR_VEC;
	}
#endif

	return 0;
}
#endif

#ifdef CONFIG_PPC64
int copy_siginfo_to_user32(struct compat_siginfo __user *d, siginfo_t *s)
{
	int err;

	if (!access_ok (VERIFY_WRITE, d, sizeof(*d)))
		return -EFAULT;

	/*                                                  
                                   
                                                           
                                                      
                                          
                                                                 
                     
  */
	err = __put_user(s->si_signo, &d->si_signo);
	err |= __put_user(s->si_errno, &d->si_errno);
	err |= __put_user((short)s->si_code, &d->si_code);
	if (s->si_code < 0)
		err |= __copy_to_user(&d->_sifields._pad, &s->_sifields._pad,
				      SI_PAD_SIZE32);
	else switch(s->si_code >> 16) {
	case __SI_CHLD >> 16:
		err |= __put_user(s->si_pid, &d->si_pid);
		err |= __put_user(s->si_uid, &d->si_uid);
		err |= __put_user(s->si_utime, &d->si_utime);
		err |= __put_user(s->si_stime, &d->si_stime);
		err |= __put_user(s->si_status, &d->si_status);
		break;
	case __SI_FAULT >> 16:
		err |= __put_user((unsigned int)(unsigned long)s->si_addr,
				  &d->si_addr);
		break;
	case __SI_POLL >> 16:
		err |= __put_user(s->si_band, &d->si_band);
		err |= __put_user(s->si_fd, &d->si_fd);
		break;
	case __SI_TIMER >> 16:
		err |= __put_user(s->si_tid, &d->si_tid);
		err |= __put_user(s->si_overrun, &d->si_overrun);
		err |= __put_user(s->si_int, &d->si_int);
		break;
	case __SI_RT >> 16: /*                                                 */
	case __SI_MESGQ >> 16:
		err |= __put_user(s->si_int, &d->si_int);
		/*             */
	case __SI_KILL >> 16:
	default:
		err |= __put_user(s->si_pid, &d->si_pid);
		err |= __put_user(s->si_uid, &d->si_uid);
		break;
	}
	return err;
}

#define copy_siginfo_to_user	copy_siginfo_to_user32

int copy_siginfo_from_user32(siginfo_t *to, struct compat_siginfo __user *from)
{
	memset(to, 0, sizeof *to);

	if (copy_from_user(to, from, 3*sizeof(int)) ||
	    copy_from_user(to->_sifields._pad,
			   from->_sifields._pad, SI_PAD_SIZE32))
		return -EFAULT;

	return 0;
}
#endif /*              */

/*
                                                         
                            
 */
int handle_rt_signal32(unsigned long sig, struct k_sigaction *ka,
		siginfo_t *info, sigset_t *oldset,
		struct pt_regs *regs)
{
	struct rt_sigframe __user *rt_sf;
	struct mcontext __user *frame;
	struct mcontext __user *tm_frame = NULL;
	void __user *addr;
	unsigned long newsp = 0;
	int sigret;
	unsigned long tramp;

	/*                     */
	/*                                    */
	rt_sf = get_sigframe(ka, get_tm_stackpointer(regs), sizeof(*rt_sf), 1);
	addr = rt_sf;
	if (unlikely(rt_sf == NULL))
		goto badframe;

	/*                                                */
	if (copy_siginfo_to_user(&rt_sf->info, info)
	    || __put_user(0, &rt_sf->uc.uc_flags)
	    || __save_altstack(&rt_sf->uc.uc_stack, regs->gpr[1])
	    || __put_user(to_user_ptr(&rt_sf->uc.uc_mcontext),
		    &rt_sf->uc.uc_regs)
	    || put_sigset_t(&rt_sf->uc.uc_sigmask, oldset))
		goto badframe;

	/*                                  */
	frame = &rt_sf->uc.uc_mcontext;
	addr = frame;
	if (vdso32_rt_sigtramp && current->mm->context.vdso_base) {
		sigret = 0;
		tramp = current->mm->context.vdso_base + vdso32_rt_sigtramp;
	} else {
		sigret = __NR_rt_sigreturn;
		tramp = (unsigned long) frame->tramp;
	}

#ifdef CONFIG_PPC_TRANSACTIONAL_MEM
	tm_frame = &rt_sf->uc_transact.uc_mcontext;
	if (MSR_TM_ACTIVE(regs->msr)) {
		if (save_tm_user_regs(regs, frame, tm_frame, sigret))
			goto badframe;
	}
	else
#endif
	{
		if (save_user_regs(regs, frame, tm_frame, sigret, 1))
			goto badframe;
	}
	regs->link = tramp;

#ifdef CONFIG_PPC_TRANSACTIONAL_MEM
	if (MSR_TM_ACTIVE(regs->msr)) {
		if (__put_user((unsigned long)&rt_sf->uc_transact,
			       &rt_sf->uc.uc_link)
		    || __put_user((unsigned long)tm_frame, &rt_sf->uc_transact.uc_regs))
			goto badframe;
	}
	else
#endif
		if (__put_user(0, &rt_sf->uc.uc_link))
			goto badframe;

	current->thread.fpscr.val = 0;	/*                            */

	/*                                                    */
	newsp = ((unsigned long)rt_sf) - (__SIGNAL_FRAMESIZE + 16);
	addr = (void __user *)regs->gpr[1];
	if (put_user(regs->gpr[1], (u32 __user *)newsp))
		goto badframe;

	/*                                   */
	regs->gpr[1] = newsp;
	regs->gpr[3] = sig;
	regs->gpr[4] = (unsigned long) &rt_sf->info;
	regs->gpr[5] = (unsigned long) &rt_sf->uc;
	regs->gpr[6] = (unsigned long) rt_sf;
	regs->nip = (unsigned long) ka->sa.sa_handler;
	/*                                             */
	regs->msr &= ~MSR_LE;
#ifdef CONFIG_PPC_TRANSACTIONAL_MEM
	/*                                                             
                                                                       
                                                
  */
	regs->msr &= ~MSR_TS_MASK;
#endif
	return 1;

badframe:
#ifdef DEBUG_SIG
	printk("badframe in handle_rt_signal, regs=%p frame=%p newsp=%lx\n",
	       regs, frame, newsp);
#endif
	if (show_unhandled_signals)
		printk_ratelimited(KERN_INFO
				   "%s[%d]: bad frame in handle_rt_signal32: "
				   "%p nip %08lx lr %08lx\n",
				   current->comm, current->pid,
				   addr, regs->nip, regs->link);

	force_sigsegv(sig, current);
	return 0;
}

static int do_setcontext(struct ucontext __user *ucp, struct pt_regs *regs, int sig)
{
	sigset_t set;
	struct mcontext __user *mcp;

	if (get_sigset_t(&set, &ucp->uc_sigmask))
		return -EFAULT;
#ifdef CONFIG_PPC64
	{
		u32 cmcp;

		if (__get_user(cmcp, &ucp->uc_regs))
			return -EFAULT;
		mcp = (struct mcontext __user *)(u64)cmcp;
		/*                                                  */
	}
#else
	if (__get_user(mcp, &ucp->uc_regs))
		return -EFAULT;
	if (!access_ok(VERIFY_READ, mcp, sizeof(*mcp)))
		return -EFAULT;
#endif
	set_current_blocked(&set);
	if (restore_user_regs(regs, mcp, sig))
		return -EFAULT;

	return 0;
}

#ifdef CONFIG_PPC_TRANSACTIONAL_MEM
static int do_setcontext_tm(struct ucontext __user *ucp,
			    struct ucontext __user *tm_ucp,
			    struct pt_regs *regs)
{
	sigset_t set;
	struct mcontext __user *mcp;
	struct mcontext __user *tm_mcp;
	u32 cmcp;
	u32 tm_cmcp;

	if (get_sigset_t(&set, &ucp->uc_sigmask))
		return -EFAULT;

	if (__get_user(cmcp, &ucp->uc_regs) ||
	    __get_user(tm_cmcp, &tm_ucp->uc_regs))
		return -EFAULT;
	mcp = (struct mcontext __user *)(u64)cmcp;
	tm_mcp = (struct mcontext __user *)(u64)tm_cmcp;
	/*                                                  */

	set_current_blocked(&set);
	if (restore_tm_user_regs(regs, mcp, tm_mcp))
		return -EFAULT;

	return 0;
}
#endif

long sys_swapcontext(struct ucontext __user *old_ctx,
		     struct ucontext __user *new_ctx,
		     int ctx_size, int r6, int r7, int r8, struct pt_regs *regs)
{
	unsigned char tmp;
	int ctx_has_vsx_region = 0;

#ifdef CONFIG_PPC64
	unsigned long new_msr = 0;

	if (new_ctx) {
		struct mcontext __user *mcp;
		u32 cmcp;

		/*
                                                   
                                               
              
   */
		if (__get_user(cmcp, &new_ctx->uc_regs))
			return -EFAULT;
		mcp = (struct mcontext __user *)(u64)cmcp;
		if (__get_user(new_msr, &mcp->mc_gregs[PT_MSR]))
			return -EFAULT;
	}
	/*
                                                           
                                   
  */
	if (ctx_size < UCONTEXTSIZEWITHOUTVSX)
		return -EINVAL;
	/*
                                                      
                                 
  */
	if ((ctx_size < sizeof(struct ucontext)) &&
	    (new_msr & MSR_VSX))
		return -EINVAL;
	/*                                                      */
	if (ctx_size >= sizeof(struct ucontext))
		ctx_has_vsx_region = 1;
#else
	/*                                                             
                                         
  */
	if (ctx_size < sizeof(struct ucontext))
		return -EINVAL;
#endif
	if (old_ctx != NULL) {
		struct mcontext __user *mctx;

		/*
                                                   
                                               
                                               
                                                       
                                                      
   */
		mctx = (struct mcontext __user *)
			((unsigned long) &old_ctx->uc_mcontext & ~0xfUL);
		if (!access_ok(VERIFY_WRITE, old_ctx, ctx_size)
		    || save_user_regs(regs, mctx, NULL, 0, ctx_has_vsx_region)
		    || put_sigset_t(&old_ctx->uc_sigmask, &current->blocked)
		    || __put_user(to_user_ptr(mctx), &old_ctx->uc_regs))
			return -EFAULT;
	}
	if (new_ctx == NULL)
		return 0;
	if (!access_ok(VERIFY_READ, new_ctx, ctx_size)
	    || __get_user(tmp, (u8 __user *) new_ctx)
	    || __get_user(tmp, (u8 __user *) new_ctx + ctx_size - 1))
		return -EFAULT;

	/*
                                                           
                                                               
                                                                 
                                                           
                                                           
                                                                
                                                                
                                                                  
                                                      
  */
	if (do_setcontext(new_ctx, regs, 0))
		do_exit(SIGSEGV);

	set_thread_flag(TIF_RESTOREALL);
	return 0;
}

long sys_rt_sigreturn(int r3, int r4, int r5, int r6, int r7, int r8,
		     struct pt_regs *regs)
{
	struct rt_sigframe __user *rt_sf;
#ifdef CONFIG_PPC_TRANSACTIONAL_MEM
	struct ucontext __user *uc_transact;
	unsigned long msr_hi;
	unsigned long tmp;
	int tm_restore = 0;
#endif
	/*                                                              */
	current_thread_info()->restart_block.fn = do_no_restart_syscall;

	rt_sf = (struct rt_sigframe __user *)
		(regs->gpr[1] + __SIGNAL_FRAMESIZE + 16);
	if (!access_ok(VERIFY_READ, rt_sf, sizeof(*rt_sf)))
		goto bad;
#ifdef CONFIG_PPC_TRANSACTIONAL_MEM
	if (__get_user(tmp, &rt_sf->uc.uc_link))
		goto bad;
	uc_transact = (struct ucontext __user *)(uintptr_t)tmp;
	if (uc_transact) {
		u32 cmcp;
		struct mcontext __user *mcp;

		if (__get_user(cmcp, &uc_transact->uc_regs))
			return -EFAULT;
		mcp = (struct mcontext __user *)(u64)cmcp;
		/*                                                            
               */
		if (__get_user(msr_hi, &mcp->mc_gregs[PT_MSR]))
			goto bad;

		if (MSR_TM_ACTIVE(msr_hi<<32)) {
			/*                                        
                  
    */
			tm_restore = 1;
			if (do_setcontext_tm(&rt_sf->uc, uc_transact, regs))
				goto bad;
		}
	}
	if (!tm_restore)
		/*                                  */
#endif
	if (do_setcontext(&rt_sf->uc, regs, 1))
		goto bad;

	/*
                                                             
                                                            
                                                               
                                                               
                         
  */
#ifdef CONFIG_PPC64
	if (compat_restore_altstack(&rt_sf->uc.uc_stack))
		goto bad;
#else
	if (restore_altstack(&rt_sf->uc.uc_stack))
		goto bad;
#endif
	set_thread_flag(TIF_RESTOREALL);
	return 0;

 bad:
	if (show_unhandled_signals)
		printk_ratelimited(KERN_INFO
				   "%s[%d]: bad frame in sys_rt_sigreturn: "
				   "%p nip %08lx lr %08lx\n",
				   current->comm, current->pid,
				   rt_sf, regs->nip, regs->link);

	force_sig(SIGSEGV, current);
	return 0;
}

#ifdef CONFIG_PPC32
int sys_debug_setcontext(struct ucontext __user *ctx,
			 int ndbg, struct sig_dbg_op __user *dbg,
			 int r6, int r7, int r8,
			 struct pt_regs *regs)
{
	struct sig_dbg_op op;
	int i;
	unsigned char tmp;
	unsigned long new_msr = regs->msr;
#ifdef CONFIG_PPC_ADV_DEBUG_REGS
	unsigned long new_dbcr0 = current->thread.dbcr0;
#endif

	for (i=0; i<ndbg; i++) {
		if (copy_from_user(&op, dbg + i, sizeof(op)))
			return -EFAULT;
		switch (op.dbg_type) {
		case SIG_DBG_SINGLE_STEPPING:
#ifdef CONFIG_PPC_ADV_DEBUG_REGS
			if (op.dbg_value) {
				new_msr |= MSR_DE;
				new_dbcr0 |= (DBCR0_IDM | DBCR0_IC);
			} else {
				new_dbcr0 &= ~DBCR0_IC;
				if (!DBCR_ACTIVE_EVENTS(new_dbcr0,
						current->thread.dbcr1)) {
					new_msr &= ~MSR_DE;
					new_dbcr0 &= ~DBCR0_IDM;
				}
			}
#else
			if (op.dbg_value)
				new_msr |= MSR_SE;
			else
				new_msr &= ~MSR_SE;
#endif
			break;
		case SIG_DBG_BRANCH_TRACING:
#ifdef CONFIG_PPC_ADV_DEBUG_REGS
			return -EINVAL;
#else
			if (op.dbg_value)
				new_msr |= MSR_BE;
			else
				new_msr &= ~MSR_BE;
#endif
			break;

		default:
			return -EINVAL;
		}
	}

	/*                                                         
                                                          
                                                              
                                                               
                                              */
	regs->msr = new_msr;
#ifdef CONFIG_PPC_ADV_DEBUG_REGS
	current->thread.dbcr0 = new_dbcr0;
#endif

	if (!access_ok(VERIFY_READ, ctx, sizeof(*ctx))
	    || __get_user(tmp, (u8 __user *) ctx)
	    || __get_user(tmp, (u8 __user *) (ctx + 1) - 1))
		return -EFAULT;

	/*
                                                           
                                                               
                                                                 
                                                           
                                                           
                                                                
                                                                
                                                                  
                                                      
  */
	if (do_setcontext(ctx, regs, 1)) {
		if (show_unhandled_signals)
			printk_ratelimited(KERN_INFO "%s[%d]: bad frame in "
					   "sys_debug_setcontext: %p nip %08lx "
					   "lr %08lx\n",
					   current->comm, current->pid,
					   ctx, regs->nip, regs->link);

		force_sig(SIGSEGV, current);
		goto out;
	}

	/*
                                                             
                                                            
                                                               
                                                               
                         
  */
	restore_altstack(&ctx->uc_stack);

	set_thread_flag(TIF_RESTOREALL);
 out:
	return 0;
}
#endif

/*
                               
 */
int handle_signal32(unsigned long sig, struct k_sigaction *ka,
		    siginfo_t *info, sigset_t *oldset, struct pt_regs *regs)
{
	struct sigcontext __user *sc;
	struct sigframe __user *frame;
	struct mcontext __user *tm_mctx = NULL;
	unsigned long newsp = 0;
	int sigret;
	unsigned long tramp;

	/*                     */
	frame = get_sigframe(ka, get_tm_stackpointer(regs), sizeof(*frame), 1);
	if (unlikely(frame == NULL))
		goto badframe;
	sc = (struct sigcontext __user *) &frame->sctx;

#if _NSIG != 64
#error "Please adjust handle_signal()"
#endif
	if (__put_user(to_user_ptr(ka->sa.sa_handler), &sc->handler)
	    || __put_user(oldset->sig[0], &sc->oldmask)
#ifdef CONFIG_PPC64
	    || __put_user((oldset->sig[0] >> 32), &sc->_unused[3])
#else
	    || __put_user(oldset->sig[1], &sc->_unused[3])
#endif
	    || __put_user(to_user_ptr(&frame->mctx), &sc->regs)
	    || __put_user(sig, &sc->signal))
		goto badframe;

	if (vdso32_sigtramp && current->mm->context.vdso_base) {
		sigret = 0;
		tramp = current->mm->context.vdso_base + vdso32_sigtramp;
	} else {
		sigret = __NR_sigreturn;
		tramp = (unsigned long) frame->mctx.tramp;
	}

#ifdef CONFIG_PPC_TRANSACTIONAL_MEM
	tm_mctx = &frame->mctx_transact;
	if (MSR_TM_ACTIVE(regs->msr)) {
		if (save_tm_user_regs(regs, &frame->mctx, &frame->mctx_transact,
				      sigret))
			goto badframe;
	}
	else
#endif
	{
		if (save_user_regs(regs, &frame->mctx, tm_mctx, sigret, 1))
			goto badframe;
	}

	regs->link = tramp;

	current->thread.fpscr.val = 0;	/*                            */

	/*                                                    */
	newsp = ((unsigned long)frame) - __SIGNAL_FRAMESIZE;
	if (put_user(regs->gpr[1], (u32 __user *)newsp))
		goto badframe;

	regs->gpr[1] = newsp;
	regs->gpr[3] = sig;
	regs->gpr[4] = (unsigned long) sc;
	regs->nip = (unsigned long) ka->sa.sa_handler;
	/*                                             */
	regs->msr &= ~MSR_LE;
#ifdef CONFIG_PPC_TRANSACTIONAL_MEM
	/*                                                             
                                                                       
                                                
  */
	regs->msr &= ~MSR_TS_MASK;
#endif
	return 1;

badframe:
#ifdef DEBUG_SIG
	printk("badframe in handle_signal, regs=%p frame=%p newsp=%lx\n",
	       regs, frame, newsp);
#endif
	if (show_unhandled_signals)
		printk_ratelimited(KERN_INFO
				   "%s[%d]: bad frame in handle_signal32: "
				   "%p nip %08lx lr %08lx\n",
				   current->comm, current->pid,
				   frame, regs->nip, regs->link);

	force_sigsegv(sig, current);
	return 0;
}

/*
                                             
 */
long sys_sigreturn(int r3, int r4, int r5, int r6, int r7, int r8,
		       struct pt_regs *regs)
{
	struct sigframe __user *sf;
	struct sigcontext __user *sc;
	struct sigcontext sigctx;
	struct mcontext __user *sr;
	void __user *addr;
	sigset_t set;
#ifdef CONFIG_PPC_TRANSACTIONAL_MEM
	struct mcontext __user *mcp, *tm_mcp;
	unsigned long msr_hi;
#endif

	/*                                                              */
	current_thread_info()->restart_block.fn = do_no_restart_syscall;

	sf = (struct sigframe __user *)(regs->gpr[1] + __SIGNAL_FRAMESIZE);
	sc = &sf->sctx;
	addr = sc;
	if (copy_from_user(&sigctx, sc, sizeof(sigctx)))
		goto badframe;

#ifdef CONFIG_PPC64
	/*
                                                                
                                        
  */
	set.sig[0] = sigctx.oldmask + ((long)(sigctx._unused[3]) << 32);
#else
	set.sig[0] = sigctx.oldmask;
	set.sig[1] = sigctx._unused[3];
#endif
	set_current_blocked(&set);

#ifdef CONFIG_PPC_TRANSACTIONAL_MEM
	mcp = (struct mcontext __user *)&sf->mctx;
	tm_mcp = (struct mcontext __user *)&sf->mctx_transact;
	if (__get_user(msr_hi, &tm_mcp->mc_gregs[PT_MSR]))
		goto badframe;
	if (MSR_TM_ACTIVE(msr_hi<<32)) {
		if (!cpu_has_feature(CPU_FTR_TM))
			goto badframe;
		if (restore_tm_user_regs(regs, mcp, tm_mcp))
			goto badframe;
	} else
#endif
	{
		sr = (struct mcontext __user *)from_user_ptr(sigctx.regs);
		addr = sr;
		if (!access_ok(VERIFY_READ, sr, sizeof(*sr))
		    || restore_user_regs(regs, sr, 1))
			goto badframe;
	}

	set_thread_flag(TIF_RESTOREALL);
	return 0;

badframe:
	if (show_unhandled_signals)
		printk_ratelimited(KERN_INFO
				   "%s[%d]: bad frame in sys_sigreturn: "
				   "%p nip %08lx lr %08lx\n",
				   current->comm, current->pid,
				   addr, regs->nip, regs->link);

	force_sig(SIGSEGV, current);
	return 0;
}
