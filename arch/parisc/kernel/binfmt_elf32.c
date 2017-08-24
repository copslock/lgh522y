/*
 * Support for 32-bit Linux/Parisc ELF binaries on 64 bit kernels
 *
 * Copyright (C) 2000 John Marvin
 * Copyright (C) 2000 Hewlett Packard Co.
 *
 * Heavily inspired from various other efforts to do the same thing
 * (ia64,sparc64/mips64)
 */

/*                                                         */

#define ELF_CLASS	ELFCLASS32

#define ELF_CORE_COPY_REGS(dst, pt)	\
	memset(dst, 0, sizeof(dst));	/*                              */ \
	{	int i; \
		for (i = 0; i < 32; i++) dst[i] = (elf_greg_t) pt->gr[i]; \
		for (i = 0; i < 8; i++) dst[32 + i] = (elf_greg_t) pt->sr[i]; \
	} \
	dst[40] = (elf_greg_t) pt->iaoq[0]; dst[41] = (elf_greg_t) pt->iaoq[1]; \
	dst[42] = (elf_greg_t) pt->iasq[0]; dst[43] = (elf_greg_t) pt->iasq[1]; \
	dst[44] = (elf_greg_t) pt->sar;   dst[45] = (elf_greg_t) pt->iir; \
	dst[46] = (elf_greg_t) pt->isr;   dst[47] = (elf_greg_t) pt->ior; \
	dst[48] = (elf_greg_t) mfctl(22); dst[49] = (elf_greg_t) mfctl(0); \
	dst[50] = (elf_greg_t) mfctl(24); dst[51] = (elf_greg_t) mfctl(25); \
	dst[52] = (elf_greg_t) mfctl(26); dst[53] = (elf_greg_t) mfctl(27); \
	dst[54] = (elf_greg_t) mfctl(28); dst[55] = (elf_greg_t) mfctl(29); \
	dst[56] = (elf_greg_t) mfctl(30); dst[57] = (elf_greg_t) mfctl(31); \
	dst[58] = (elf_greg_t) mfctl( 8); dst[59] = (elf_greg_t) mfctl( 9); \
	dst[60] = (elf_greg_t) mfctl(12); dst[61] = (elf_greg_t) mfctl(13); \
	dst[62] = (elf_greg_t) mfctl(10); dst[63] = (elf_greg_t) mfctl(15);


typedef unsigned int elf_greg_t;

#include <linux/spinlock.h>
#include <asm/processor.h>
#include <linux/module.h>
#include <linux/elfcore.h>
#include <linux/compat.h>		/*                       */

#define elf_prstatus elf_prstatus32
struct elf_prstatus32
{
	struct elf_siginfo pr_info;	/*                             */
	short	pr_cursig;		/*                */
	unsigned int pr_sigpend;	/*                        */
	unsigned int pr_sighold;	/*                     */
	pid_t	pr_pid;
	pid_t	pr_ppid;
	pid_t	pr_pgrp;
	pid_t	pr_sid;
	struct compat_timeval pr_utime;		/*           */
	struct compat_timeval pr_stime;		/*             */
	struct compat_timeval pr_cutime;	/*                      */
	struct compat_timeval pr_cstime;	/*                        */
	elf_gregset_t pr_reg;	/*              */
	int pr_fpvalid;		/*                                        */
};

#define elf_prpsinfo elf_prpsinfo32
struct elf_prpsinfo32
{
	char	pr_state;	/*                       */
	char	pr_sname;	/*                   */
	char	pr_zomb;	/*        */
	char	pr_nice;	/*          */
	unsigned int pr_flag;	/*       */
	u16	pr_uid;
	u16	pr_gid;
	pid_t	pr_pid, pr_ppid, pr_pgrp, pr_sid;
	/*              */
	char	pr_fname[16];	/*                        */
	char	pr_psargs[ELF_PRARGSZ];	/*                          */
};

#define init_elf_binfmt init_elf32_binfmt

#define ELF_PLATFORM  ("PARISC32\0")

/*
                                                                        
                                                                  
                                                             
 */

#undef SET_PERSONALITY
#define SET_PERSONALITY(ex) \
	set_thread_flag(TIF_32BIT); \
	current->thread.map_base = DEFAULT_MAP_BASE32; \
	current->thread.task_size = DEFAULT_TASK_SIZE32 \

#undef cputime_to_timeval
#define cputime_to_timeval cputime_to_compat_timeval
static __inline__ void
cputime_to_compat_timeval(const cputime_t cputime, struct compat_timeval *value)
{
	unsigned long jiffies = cputime_to_jiffies(cputime);
	value->tv_usec = (jiffies % HZ) * (1000000L / HZ);
	value->tv_sec = jiffies / HZ;
}

#include "../../../fs/binfmt_elf.c"
