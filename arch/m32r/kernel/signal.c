/*
 *  linux/arch/m32r/kernel/signal.c
 *
 *  Copyright (c) 2003  Hitoshi Yamamoto
 *
 *  Taken from i386 version.
 *  Copyright (C) 1991, 1992  Linus Torvalds
 *
 *  1997-11-28  Modified for POSIX.1b signals by Richard Henderson
 *  2000-06-20  Pentium III FXSR, SSE support by Gareth Hughes
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
#include <linux/tracehook.h>
#include <asm/cacheflush.h>
#include <asm/ucontext.h>
#include <asm/uaccess.h>

#define DEBUG_SIG 0

/*
                                             
 */

struct rt_sigframe
{
	int sig;
	struct siginfo __user *pinfo;
	void __user *puc;
	struct siginfo info;
	struct ucontext uc;
//                         
};

static int
restore_sigcontext(struct pt_regs *regs, struct sigcontext __user *sc,
		   int *r0_p)
{
	unsigned int err = 0;

	/*                                                              */
	current_thread_info()->restart_block.fn = do_no_restart_syscall;

#define COPY(x)		err |= __get_user(regs->x, &sc->sc_##x)
	COPY(r4);
	COPY(r5);
	COPY(r6);
	COPY(pt_regs);
	/*                   */
	COPY(r1);
	COPY(r2);
	COPY(r3);
	COPY(r7);
	COPY(r8);
	COPY(r9);
	COPY(r10);
	COPY(r11);
	COPY(r12);
	COPY(acc0h);
	COPY(acc0l);
	COPY(acc1h);		/*                     */
	COPY(acc1l);		/*                     */
	COPY(psw);
	COPY(bpc);
	COPY(bbpsw);
	COPY(bbpc);
	COPY(spu);
	COPY(fp);
	COPY(lr);
	COPY(spi);
#undef COPY

	regs->syscall_nr = -1;	/*                        */
	err |= __get_user(*r0_p, &sc->sc_r0);

	return err;
}

asmlinkage int
sys_rt_sigreturn(unsigned long r0, unsigned long r1,
		 unsigned long r2, unsigned long r3, unsigned long r4,
		 unsigned long r5, unsigned long r6, struct pt_regs *regs)
{
	struct rt_sigframe __user *frame = (struct rt_sigframe __user *)regs->spu;
	sigset_t set;
	int result;

	if (!access_ok(VERIFY_READ, frame, sizeof(*frame)))
		goto badframe;
	if (__copy_from_user(&set, &frame->uc.uc_sigmask, sizeof(set)))
		goto badframe;

	set_current_blocked(&set);

	if (restore_sigcontext(regs, &frame->uc.uc_mcontext, &result))
		goto badframe;

	if (restore_altstack(&frame->uc.uc_stack))
		goto badframe;

	return result;

badframe:
	force_sig(SIGSEGV, current);
	return 0;
}

/*
                         
 */

static int
setup_sigcontext(struct sigcontext __user *sc, struct pt_regs *regs,
	         unsigned long mask)
{
	int err = 0;

#define COPY(x)	err |= __put_user(regs->x, &sc->sc_##x)
	COPY(r4);
	COPY(r5);
	COPY(r6);
	COPY(pt_regs);
	COPY(r0);
	COPY(r1);
	COPY(r2);
	COPY(r3);
	COPY(r7);
	COPY(r8);
	COPY(r9);
	COPY(r10);
	COPY(r11);
	COPY(r12);
	COPY(acc0h);
	COPY(acc0l);
	COPY(acc1h);		/*                     */
	COPY(acc1l);		/*                     */
	COPY(psw);
	COPY(bpc);
	COPY(bbpsw);
	COPY(bbpc);
	COPY(spu);
	COPY(fp);
	COPY(lr);
	COPY(spi);
#undef COPY

	err |= __put_user(mask, &sc->oldmask);

	return err;
}

/*
                                 
 */
static inline void __user *
get_sigframe(struct k_sigaction *ka, unsigned long sp, size_t frame_size)
{
	/*                                                        */
	if (ka->sa.sa_flags & SA_ONSTACK) {
		if (sas_ss_flags(sp) == 0)
			sp = current->sas_ss_sp + current->sas_ss_size;
	}

	return (void __user *)((sp - frame_size) & -8ul);
}

static int setup_rt_frame(int sig, struct k_sigaction *ka, siginfo_t *info,
			   sigset_t *set, struct pt_regs *regs)
{
	struct rt_sigframe __user *frame;
	int err = 0;
	int signal;

	frame = get_sigframe(ka, regs->spu, sizeof(*frame));

	if (!access_ok(VERIFY_WRITE, frame, sizeof(*frame)))
		goto give_sigsegv;

	signal = current_thread_info()->exec_domain
		&& current_thread_info()->exec_domain->signal_invmap
		&& sig < 32
		? current_thread_info()->exec_domain->signal_invmap[sig]
		: sig;

	err |= __put_user(signal, &frame->sig);
	if (err)
		goto give_sigsegv;

	err |= __put_user(&frame->info, &frame->pinfo);
	err |= __put_user(&frame->uc, &frame->puc);
	err |= copy_siginfo_to_user(&frame->info, info);
	if (err)
		goto give_sigsegv;

	/*                       */
	err |= __put_user(0, &frame->uc.uc_flags);
	err |= __put_user(0, &frame->uc.uc_link);
	err |= __save_altstack(&frame->uc.uc_stack, regs->spu);
	err |= setup_sigcontext(&frame->uc.uc_mcontext, regs, set->sig[0]);
	err |= __copy_to_user(&frame->uc.uc_sigmask, set, sizeof(*set));
	if (err)
		goto give_sigsegv;

	/*                                   */
	regs->lr = (unsigned long)ka->sa.sa_restorer;

	/*                                     */
	regs->spu = (unsigned long)frame;
	regs->r0 = signal;	/*                        */
	regs->r1 = (unsigned long)&frame->info;
	regs->r2 = (unsigned long)&frame->uc;
	regs->bpc = (unsigned long)ka->sa.sa_handler;

	set_fs(USER_DS);

#if DEBUG_SIG
	printk("SIG deliver (%s:%d): sp=%p pc=%p\n",
		current->comm, current->pid, frame, regs->pc);
#endif

	return 0;

give_sigsegv:
	force_sigsegv(sig, current);
	return -EFAULT;
}

static int prev_insn(struct pt_regs *regs)
{
	u16 inst;
	if (get_user(inst, (u16 __user *)(regs->bpc - 2)))
		return -EFAULT;
	if ((inst & 0xfff0) == 0x10f0)	/*        */
		regs->bpc -= 2;
	else
		regs->bpc -= 4;
	regs->syscall_nr = -1;
	return 0;
}

/*
                               
 */

static void
handle_signal(unsigned long sig, struct k_sigaction *ka, siginfo_t *info,
	      struct pt_regs *regs)
{
	/*                            */
	if (regs->syscall_nr >= 0) {
		/*                                       */
		switch (regs->r0) {
		        case -ERESTART_RESTARTBLOCK:
			case -ERESTARTNOHAND:
				regs->r0 = -EINTR;
				break;

			case -ERESTARTSYS:
				if (!(ka->sa.sa_flags & SA_RESTART)) {
					regs->r0 = -EINTR;
					break;
				}
			/*             */
			case -ERESTARTNOINTR:
				regs->r0 = regs->orig_r0;
				if (prev_insn(regs) < 0)
					return;
		}
	}

	/*                        */
	if (setup_rt_frame(sig, ka, info, sigmask_to_save(), regs))
		return;

	signal_delivered(sig, info, ka, regs, 0);
}

/*
                                                                           
                                                                        
           
 */
static void do_signal(struct pt_regs *regs)
{
	siginfo_t info;
	int signr;
	struct k_sigaction ka;

	/*
                                             
                                                
                                                   
          
  */
	if (!user_mode(regs))
		return;

	signr = get_signal_to_deliver(&info, &ka, regs, NULL);
	if (signr > 0) {
		/*                                                
                                                      
                                                  
                       
   */

		/*                                      */
		handle_signal(signr, &ka, &info, regs);

		return;
	}

	/*                                 */
	if (regs->syscall_nr >= 0) {
		/*                                               */
		if (regs->r0 == -ERESTARTNOHAND ||
		    regs->r0 == -ERESTARTSYS ||
		    regs->r0 == -ERESTARTNOINTR) {
			regs->r0 = regs->orig_r0;
			prev_insn(regs);
		} else if (regs->r0 == -ERESTART_RESTARTBLOCK){
			regs->r0 = regs->orig_r0;
			regs->r7 = __NR_restart_syscall;
			prev_insn(regs);
		}
	}
	restore_saved_sigmask();
}

/*
                                                 
                                             
 */
void do_notify_resume(struct pt_regs *regs, __u32 thread_info_flags)
{
	/*                      */
	if (thread_info_flags & _TIF_SINGLESTEP)
		clear_thread_flag(TIF_SINGLESTEP);

	/*                                   */
	if (thread_info_flags & _TIF_SIGPENDING)
		do_signal(regs);

	if (thread_info_flags & _TIF_NOTIFY_RESUME) {
		clear_thread_flag(TIF_NOTIFY_RESUME);
		tracehook_notify_resume(regs);
	}
}
