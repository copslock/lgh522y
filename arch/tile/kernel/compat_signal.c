/*
 * Copyright 2010 Tilera Corporation. All Rights Reserved.
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation, version 2.
 *
 *   This program is distributed in the hope that it will be useful, but
 *   WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, GOOD TITLE or
 *   NON INFRINGEMENT.  See the GNU General Public License for
 *   more details.
 */

#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/smp.h>
#include <linux/kernel.h>
#include <linux/signal.h>
#include <linux/errno.h>
#include <linux/wait.h>
#include <linux/unistd.h>
#include <linux/stddef.h>
#include <linux/personality.h>
#include <linux/suspend.h>
#include <linux/ptrace.h>
#include <linux/elf.h>
#include <linux/compat.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <asm/processor.h>
#include <asm/ucontext.h>
#include <asm/sigframe.h>
#include <asm/syscalls.h>
#include <arch/interrupts.h>

struct compat_ucontext {
	compat_ulong_t	  uc_flags;
	compat_uptr_t     uc_link;
	struct compat_sigaltstack	  uc_stack;
	struct sigcontext uc_mcontext;
	sigset_t	  uc_sigmask;	/*                             */
};

struct compat_rt_sigframe {
	unsigned char save_area[C_ABI_SAVE_AREA_SIZE]; /*                  */
	struct compat_siginfo info;
	struct compat_ucontext uc;
};

int copy_siginfo_to_user32(struct compat_siginfo __user *to, siginfo_t *from)
{
	int err;

	if (!access_ok(VERIFY_WRITE, to, sizeof(struct compat_siginfo)))
		return -EFAULT;

	/*                                                         
                                   
                                                           
                                                      
                                            */
	err = __put_user(from->si_signo, &to->si_signo);
	err |= __put_user(from->si_errno, &to->si_errno);
	err |= __put_user((short)from->si_code, &to->si_code);

	if (from->si_code < 0) {
		err |= __put_user(from->si_pid, &to->si_pid);
		err |= __put_user(from->si_uid, &to->si_uid);
		err |= __put_user(ptr_to_compat(from->si_ptr), &to->si_ptr);
	} else {
		/*
                                               
                                                       
   */
		err |= __put_user(from->_sifields._pad[0],
				  &to->_sifields._pad[0]);
		switch (from->si_code >> 16) {
		case __SI_FAULT >> 16:
			break;
		case __SI_CHLD >> 16:
			err |= __put_user(from->si_utime, &to->si_utime);
			err |= __put_user(from->si_stime, &to->si_stime);
			err |= __put_user(from->si_status, &to->si_status);
			/*              */
		default:
		case __SI_KILL >> 16:
			err |= __put_user(from->si_uid, &to->si_uid);
			break;
		case __SI_POLL >> 16:
			err |= __put_user(from->si_fd, &to->si_fd);
			break;
		case __SI_TIMER >> 16:
			err |= __put_user(from->si_overrun, &to->si_overrun);
			err |= __put_user(ptr_to_compat(from->si_ptr),
					  &to->si_ptr);
			break;
			 /*                                                 */
		case __SI_RT >> 16:
		case __SI_MESGQ >> 16:
			err |= __put_user(from->si_uid, &to->si_uid);
			err |= __put_user(from->si_int, &to->si_int);
			break;
		}
	}
	return err;
}

int copy_siginfo_from_user32(siginfo_t *to, struct compat_siginfo __user *from)
{
	int err;
	u32 ptr32;

	if (!access_ok(VERIFY_READ, from, sizeof(struct compat_siginfo)))
		return -EFAULT;

	err = __get_user(to->si_signo, &from->si_signo);
	err |= __get_user(to->si_errno, &from->si_errno);
	err |= __get_user(to->si_code, &from->si_code);

	err |= __get_user(to->si_pid, &from->si_pid);
	err |= __get_user(to->si_uid, &from->si_uid);
	err |= __get_user(ptr32, &from->si_ptr);
	to->si_ptr = compat_ptr(ptr32);

	return err;
}

/*                                                                          */
long compat_sys_rt_sigreturn(void)
{
	struct pt_regs *regs = current_pt_regs();
	struct compat_rt_sigframe __user *frame =
		(struct compat_rt_sigframe __user *) compat_ptr(regs->sp);
	sigset_t set;

	if (!access_ok(VERIFY_READ, frame, sizeof(*frame)))
		goto badframe;
	if (__copy_from_user(&set, &frame->uc.uc_sigmask, sizeof(set)))
		goto badframe;

	set_current_blocked(&set);

	if (restore_sigcontext(regs, &frame->uc.uc_mcontext))
		goto badframe;

	if (compat_restore_altstack(&frame->uc.uc_stack))
		goto badframe;

	return 0;

badframe:
	signal_fault("bad sigreturn frame", regs, frame, 0);
	return 0;
}

/*
                                 
 */
static inline void __user *compat_get_sigframe(struct k_sigaction *ka,
					       struct pt_regs *regs,
					       size_t frame_size)
{
	unsigned long sp;

	/*                               */
	sp = (unsigned long)compat_ptr(regs->sp);

	/*
                                                              
                                                            
                          
  */
	if (on_sig_stack(sp) && !likely(on_sig_stack(sp - frame_size)))
		return (void __user __force *)-1UL;

	/*                                                        */
	if (ka->sa.sa_flags & SA_ONSTACK) {
		if (sas_ss_flags(sp) == 0)
			sp = current->sas_ss_sp + current->sas_ss_size;
	}

	sp -= frame_size;
	/*
                                                      
                                                  
  */
	sp &= -16UL;
	return (void __user *) sp;
}

int compat_setup_rt_frame(int sig, struct k_sigaction *ka, siginfo_t *info,
			  sigset_t *set, struct pt_regs *regs)
{
	unsigned long restorer;
	struct compat_rt_sigframe __user *frame;
	int err = 0;
	int usig;

	frame = compat_get_sigframe(ka, regs, sizeof(*frame));

	if (!access_ok(VERIFY_WRITE, frame, sizeof(*frame)))
		goto give_sigsegv;

	usig = current_thread_info()->exec_domain
		&& current_thread_info()->exec_domain->signal_invmap
		&& sig < 32
		? current_thread_info()->exec_domain->signal_invmap[sig]
		: sig;

	/*                                                                   */
	if (ka->sa.sa_flags & SA_SIGINFO) {
		/*                                                           */
		err |= copy_siginfo_to_user32(&frame->info, info);
		regs->flags |= PT_FLAGS_RESTORE_REGS;
	} else {
		err |= __put_user(info->si_signo, &frame->info.si_signo);
	}

	/*                       */
	err |= __clear_user(&frame->save_area, sizeof(frame->save_area));
	err |= __put_user(0, &frame->uc.uc_flags);
	err |= __put_user(0, &frame->uc.uc_link);
	err |= __compat_save_altstack(&frame->uc.uc_stack, regs->sp);
	err |= setup_sigcontext(&frame->uc.uc_mcontext, regs);
	err |= __copy_to_user(&frame->uc.uc_sigmask, set, sizeof(*set));
	if (err)
		goto give_sigsegv;

	restorer = VDSO_BASE;
	if (ka->sa.sa_flags & SA_RESTORER)
		restorer = ptr_to_compat_reg(ka->sa.sa_restorer);

	/*
                                        
                                                               
                                              
                                                                  
                                                                   
  */
	regs->pc = ptr_to_compat_reg(ka->sa.sa_handler);
	regs->ex1 = PL_ICS_EX1(USER_PL, 1); /*                         */
	regs->sp = ptr_to_compat_reg(frame);
	regs->lr = restorer;
	regs->regs[0] = (unsigned long) usig;
	regs->regs[1] = ptr_to_compat_reg(&frame->info);
	regs->regs[2] = ptr_to_compat_reg(&frame->uc);
	regs->flags |= PT_FLAGS_CALLER_SAVES;
	return 0;

give_sigsegv:
	signal_fault("bad setup frame", regs, frame, sig);
	return -EFAULT;
}
