/*
 * include/asm-xtensa/bitops.h
 *
 * Atomic operations that C can't guarantee us.Useful for resource counting etc.
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2001 - 2007 Tensilica Inc.
 */

#ifndef _XTENSA_BITOPS_H
#define _XTENSA_BITOPS_H

#ifdef __KERNEL__

#ifndef _LINUX_BITOPS_H
#error only <linux/bitops.h> can be included directly
#endif

#include <asm/processor.h>
#include <asm/byteorder.h>

#ifdef CONFIG_SMP
# error SMP not supported on this architecture
#endif

#define smp_mb__before_clear_bit()	barrier()
#define smp_mb__after_clear_bit()	barrier()

#include <asm-generic/bitops/non-atomic.h>

#if XCHAL_HAVE_NSA

static inline unsigned long __cntlz (unsigned long x)
{
	int lz;
	asm ("nsau %0, %1" : "=r" (lz) : "r" (x));
	return lz;
}

/*
                                                             
                                                           
 */

static inline int ffz(unsigned long x)
{
	return 31 - __cntlz(~x & -~x);
}

/*
                                                        
 */

static inline int __ffs(unsigned long x)
{
	return 31 - __cntlz(x & -x);
}

/*
                                                                   
                                                        
                                                  
 */

static inline int ffs(unsigned long x)
{
	return 32 - __cntlz(x & -x);
}

/*
                                                     
                                                     
 */

static inline int fls (unsigned int x)
{
	return 32 - __cntlz(x);
}

/* 
                                                              
                            
  
                                                                        
 */
static inline unsigned long __fls(unsigned long word)
{
	return 31 - __cntlz(word);
}
#else

/*                                                                            */

# include <asm-generic/bitops/ffs.h>
# include <asm-generic/bitops/__ffs.h>
# include <asm-generic/bitops/ffz.h>
# include <asm-generic/bitops/fls.h>
# include <asm-generic/bitops/__fls.h>

#endif

#include <asm-generic/bitops/fls64.h>

#if XCHAL_HAVE_S32C1I

static inline void set_bit(unsigned int bit, volatile unsigned long *p)
{
	unsigned long tmp, value;
	unsigned long mask = 1UL << (bit & 31);

	p += bit >> 5;

	__asm__ __volatile__(
			"1:     l32i    %1, %3, 0\n"
			"       wsr     %1, scompare1\n"
			"       or      %0, %1, %2\n"
			"       s32c1i  %0, %3, 0\n"
			"       bne     %0, %1, 1b\n"
			: "=&a" (tmp), "=&a" (value)
			: "a" (mask), "a" (p)
			: "memory");
}

static inline void clear_bit(unsigned int bit, volatile unsigned long *p)
{
	unsigned long tmp, value;
	unsigned long mask = 1UL << (bit & 31);

	p += bit >> 5;

	__asm__ __volatile__(
			"1:     l32i    %1, %3, 0\n"
			"       wsr     %1, scompare1\n"
			"       and     %0, %1, %2\n"
			"       s32c1i  %0, %3, 0\n"
			"       bne     %0, %1, 1b\n"
			: "=&a" (tmp), "=&a" (value)
			: "a" (~mask), "a" (p)
			: "memory");
}

static inline void change_bit(unsigned int bit, volatile unsigned long *p)
{
	unsigned long tmp, value;
	unsigned long mask = 1UL << (bit & 31);

	p += bit >> 5;

	__asm__ __volatile__(
			"1:     l32i    %1, %3, 0\n"
			"       wsr     %1, scompare1\n"
			"       xor     %0, %1, %2\n"
			"       s32c1i  %0, %3, 0\n"
			"       bne     %0, %1, 1b\n"
			: "=&a" (tmp), "=&a" (value)
			: "a" (mask), "a" (p)
			: "memory");
}

static inline int
test_and_set_bit(unsigned int bit, volatile unsigned long *p)
{
	unsigned long tmp, value;
	unsigned long mask = 1UL << (bit & 31);

	p += bit >> 5;

	__asm__ __volatile__(
			"1:     l32i    %1, %3, 0\n"
			"       wsr     %1, scompare1\n"
			"       or      %0, %1, %2\n"
			"       s32c1i  %0, %3, 0\n"
			"       bne     %0, %1, 1b\n"
			: "=&a" (tmp), "=&a" (value)
			: "a" (mask), "a" (p)
			: "memory");

	return tmp & mask;
}

static inline int
test_and_clear_bit(unsigned int bit, volatile unsigned long *p)
{
	unsigned long tmp, value;
	unsigned long mask = 1UL << (bit & 31);

	p += bit >> 5;

	__asm__ __volatile__(
			"1:     l32i    %1, %3, 0\n"
			"       wsr     %1, scompare1\n"
			"       and     %0, %1, %2\n"
			"       s32c1i  %0, %3, 0\n"
			"       bne     %0, %1, 1b\n"
			: "=&a" (tmp), "=&a" (value)
			: "a" (~mask), "a" (p)
			: "memory");

	return tmp & mask;
}

static inline int
test_and_change_bit(unsigned int bit, volatile unsigned long *p)
{
	unsigned long tmp, value;
	unsigned long mask = 1UL << (bit & 31);

	p += bit >> 5;

	__asm__ __volatile__(
			"1:     l32i    %1, %3, 0\n"
			"       wsr     %1, scompare1\n"
			"       xor     %0, %1, %2\n"
			"       s32c1i  %0, %3, 0\n"
			"       bne     %0, %1, 1b\n"
			: "=&a" (tmp), "=&a" (value)
			: "a" (mask), "a" (p)
			: "memory");

	return tmp & mask;
}

#else

#include <asm-generic/bitops/atomic.h>

#endif /*                   */

#include <asm-generic/bitops/find.h>
#include <asm-generic/bitops/le.h>

#include <asm-generic/bitops/ext2-atomic-setbit.h>

#include <asm-generic/bitops/hweight.h>
#include <asm-generic/bitops/lock.h>
#include <asm-generic/bitops/sched.h>

#endif	/*            */

#endif	/*                  */
