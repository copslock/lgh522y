/*
 * syscalls.h - Linux syscall interfaces (arch-specific)
 *
 * Copyright (c) 2008 Jaswinder Singh Rajput
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

#ifndef _ASM_TILE_SYSCALLS_H
#define _ASM_TILE_SYSCALLS_H

#include <linux/compiler.h>
#include <linux/linkage.h>
#include <linux/signal.h>
#include <linux/types.h>
#include <linux/compat.h>

/*
                                                                  
                                                                 
                                                                  
                      
 */

/*              */
ssize_t sys32_readahead(int fd, u32 offset_lo, u32 offset_hi, u32 count);
long sys32_fadvise64(int fd, u32 offset_lo, u32 offset_hi,
		     u32 len, int advice);
int sys32_fadvise64_64(int fd, u32 offset_lo, u32 offset_hi,
		       u32 len_lo, u32 len_hi, int advice);
long sys_cacheflush(unsigned long addr, unsigned long len,
		    unsigned long flags);
#ifndef __tilegx__  /*                                 */
#define sys_mmap sys_mmap
#endif

#ifndef __tilegx__
/*            */
long sys_cmpxchg_badaddr(unsigned long address);
#endif

#ifdef CONFIG_COMPAT
/*                                                                        */
long sys_fcntl64(unsigned int fd, unsigned int cmd, unsigned long arg);
long sys_fstat64(unsigned long fd, struct stat64 __user *statbuf);
long sys_truncate64(const char __user *path, loff_t length);
long sys_ftruncate64(unsigned int fd, loff_t length);
#endif

/*                                                                   */
long sys_rt_sigreturn(void);
#define sys_rt_sigreturn sys_rt_sigreturn

/*                                      */
long _sys_rt_sigreturn(void);
long _sys_clone(unsigned long clone_flags, unsigned long newsp,
		void __user *parent_tid, void __user *child_tid);

#include <asm-generic/syscalls.h>

#endif /*                      */
