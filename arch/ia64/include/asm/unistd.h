/*
 * IA-64 Linux syscall numbers and inline-functions.
 *
 * Copyright (C) 1998-2005 Hewlett-Packard Co
 *	David Mosberger-Tang <davidm@hpl.hp.com>
 */
#ifndef _ASM_IA64_UNISTD_H
#define _ASM_IA64_UNISTD_H

#include <uapi/asm/unistd.h>



#define NR_syscalls			312 /*                         */

/*
                                                                             
                                                                         
                                       
 */
#define __IGNORE_fork		/*         */
#define __IGNORE_time		/*                */
#define __IGNORE_alarm		/*                            */
#define __IGNORE_pause		/*                                   */
#define __IGNORE_utime		/*          */
#define __IGNORE_getpgrp	/*           */
#define __IGNORE_vfork		/*         */
#define __IGNORE_umount2	/*          */

#if !defined(__ASSEMBLY__) && !defined(ASSEMBLER)

#include <linux/types.h>
#include <linux/linkage.h>
#include <linux/compiler.h>

extern long __ia64_syscall (long a0, long a1, long a2, long a3, long a4, long nr);

asmlinkage unsigned long sys_mmap(
				unsigned long addr, unsigned long len,
				int prot, int flags,
				int fd, long off);
asmlinkage unsigned long sys_mmap2(
				unsigned long addr, unsigned long len,
				int prot, int flags,
				int fd, long pgoff);
struct pt_regs;
asmlinkage long sys_ia64_pipe(void);

#endif /*               */
#endif /*                    */
