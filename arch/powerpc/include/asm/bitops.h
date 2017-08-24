/*
 * PowerPC atomic bit operations.
 *
 * Merged version by David Gibson <david@gibson.dropbear.id.au>.
 * Based on ppc64 versions by: Dave Engebretsen, Todd Inglett, Don
 * Reed, Pat McCarthy, Peter Bergner, Anton Blanchard.  They
 * originally took it from the ppc32 code.
 *
 * Within a word, bits are numbered LSB first.  Lot's of places make
 * this assumption by directly testing bits with (val & (1<<nr)).
 * This can cause confusion for large (> 1 word) bitmaps on a
 * big-endian system because, unlike little endian, the number of each
 * bit depends on the word size.
 *
 * The bitop functions are defined to work on unsigned longs, so for a
 * ppc64 system the bits end up numbered:
 *   |63..............0|127............64|191...........128|255...........196|
 * and on ppc32:
 *   |31.....0|63....31|95....64|127...96|159..128|191..160|223..192|255..224|
 *
 * There are a few little-endian macros used mostly for filesystem
 * bitmaps, these work on similar bit arrays layouts, but
 * byte-oriented:
 *   |7...0|15...8|23...16|31...24|39...32|47...40|55...48|63...56|
 *
 * The main difference is that bit 3-5 (64b) or 3-4 (32b) in the bit
 * number field needs to be reversed compared to the big-endian bit
 * fields. This can be achieved by XOR with 0x38 (64b) or 0x18 (32b).
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#ifndef _ASM_POWERPC_BITOPS_H
#define _ASM_POWERPC_BITOPS_H

#ifdef __KERNEL__

#ifndef _LINUX_BITOPS_H
#error only <linux/bitops.h> can be included directly
#endif

#include <linux/compiler.h>
#include <asm/asm-compat.h>
#include <asm/synch.h>

/*
                                           
 */
#define smp_mb__before_clear_bit()	smp_mb()
#define smp_mb__after_clear_bit()	smp_mb()

/*                                               */
#define DEFINE_BITOP(fn, op, prefix)		\
static __inline__ void fn(unsigned long mask,	\
		volatile unsigned long *_p)	\
{						\
	unsigned long old;			\
	unsigned long *p = (unsigned long *)_p;	\
	__asm__ __volatile__ (			\
	prefix					\
"1:"	PPC_LLARX(%0,0,%3,0) "\n"		\
	stringify_in_c(op) "%0,%0,%2\n"		\
	PPC405_ERR77(0,%3)			\
	PPC_STLCX "%0,0,%3\n"			\
	"bne- 1b\n"				\
	: "=&r" (old), "+m" (*p)		\
	: "r" (mask), "r" (p)			\
	: "cc", "memory");			\
}

DEFINE_BITOP(set_bits, or, "")
DEFINE_BITOP(clear_bits, andc, "")
DEFINE_BITOP(clear_bits_unlock, andc, PPC_RELEASE_BARRIER)
DEFINE_BITOP(change_bits, xor, "")

static __inline__ void set_bit(int nr, volatile unsigned long *addr)
{
	set_bits(BIT_MASK(nr), addr + BIT_WORD(nr));
}

static __inline__ void clear_bit(int nr, volatile unsigned long *addr)
{
	clear_bits(BIT_MASK(nr), addr + BIT_WORD(nr));
}

static __inline__ void clear_bit_unlock(int nr, volatile unsigned long *addr)
{
	clear_bits_unlock(BIT_MASK(nr), addr + BIT_WORD(nr));
}

static __inline__ void change_bit(int nr, volatile unsigned long *addr)
{
	change_bits(BIT_MASK(nr), addr + BIT_WORD(nr));
}

/*                                                                          
             */
#define DEFINE_TESTOP(fn, op, prefix, postfix, eh)	\
static __inline__ unsigned long fn(			\
		unsigned long mask,			\
		volatile unsigned long *_p)		\
{							\
	unsigned long old, t;				\
	unsigned long *p = (unsigned long *)_p;		\
	__asm__ __volatile__ (				\
	prefix						\
"1:"	PPC_LLARX(%0,0,%3,eh) "\n"			\
	stringify_in_c(op) "%1,%0,%2\n"			\
	PPC405_ERR77(0,%3)				\
	PPC_STLCX "%1,0,%3\n"				\
	"bne- 1b\n"					\
	postfix						\
	: "=&r" (old), "=&r" (t)			\
	: "r" (mask), "r" (p)				\
	: "cc", "memory");				\
	return (old & mask);				\
}

DEFINE_TESTOP(test_and_set_bits, or, PPC_ATOMIC_ENTRY_BARRIER,
	      PPC_ATOMIC_EXIT_BARRIER, 0)
DEFINE_TESTOP(test_and_set_bits_lock, or, "",
	      PPC_ACQUIRE_BARRIER, 1)
DEFINE_TESTOP(test_and_clear_bits, andc, PPC_ATOMIC_ENTRY_BARRIER,
	      PPC_ATOMIC_EXIT_BARRIER, 0)
DEFINE_TESTOP(test_and_change_bits, xor, PPC_ATOMIC_ENTRY_BARRIER,
	      PPC_ATOMIC_EXIT_BARRIER, 0)

static __inline__ int test_and_set_bit(unsigned long nr,
				       volatile unsigned long *addr)
{
	return test_and_set_bits(BIT_MASK(nr), addr + BIT_WORD(nr)) != 0;
}

static __inline__ int test_and_set_bit_lock(unsigned long nr,
				       volatile unsigned long *addr)
{
	return test_and_set_bits_lock(BIT_MASK(nr),
				addr + BIT_WORD(nr)) != 0;
}

static __inline__ int test_and_clear_bit(unsigned long nr,
					 volatile unsigned long *addr)
{
	return test_and_clear_bits(BIT_MASK(nr), addr + BIT_WORD(nr)) != 0;
}

static __inline__ int test_and_change_bit(unsigned long nr,
					  volatile unsigned long *addr)
{
	return test_and_change_bits(BIT_MASK(nr), addr + BIT_WORD(nr)) != 0;
}

#include <asm-generic/bitops/non-atomic.h>

static __inline__ void __clear_bit_unlock(int nr, volatile unsigned long *addr)
{
	__asm__ __volatile__(PPC_RELEASE_BARRIER "" ::: "memory");
	__clear_bit(nr, addr);
}

/*
                                                                    
                                               
 */
static __inline__ __attribute__((const))
int __ilog2(unsigned long x)
{
	int lz;

	asm (PPC_CNTLZL "%0,%1" : "=r" (lz) : "r" (x));
	return BITS_PER_LONG - 1 - lz;
}

static inline __attribute__((const))
int __ilog2_u32(u32 n)
{
	int bit;
	asm ("cntlzw %0,%1" : "=r" (bit) : "r" (n));
	return 31 - bit;
}

#ifdef __powerpc64__
static inline __attribute__((const))
int __ilog2_u64(u64 n)
{
	int bit;
	asm ("cntlzd %0,%1" : "=r" (bit) : "r" (n));
	return 63 - bit;
}
#endif

/*
                                                                    
                                                           
                                                        
 */
static __inline__ unsigned long ffz(unsigned long x)
{
	/*                                             */
	if ((x = ~x) == 0)
		return BITS_PER_LONG;

	/*
                                                                    
                                                                         
                                                                      
                                                                    
  */
	return __ilog2(x & -x);
}

static __inline__ int __ffs(unsigned long x)
{
	return __ilog2(x & -x);
}

/*
                                                           
                                                        
                                                  
 */
static __inline__ int ffs(int x)
{
	unsigned long i = (unsigned long)x;
	return __ilog2(i & -i) + 1;
}

/*
                                             
                                                     
 */
static __inline__ int fls(unsigned int x)
{
	int lz;

	asm ("cntlzw %0,%1" : "=r" (lz) : "r" (x));
	return 32 - lz;
}

static __inline__ unsigned long __fls(unsigned long x)
{
	return __ilog2(x);
}

/*
                                                                        
                                                                     
                    
 */
#ifdef __powerpc64__
static __inline__ int fls64(__u64 x)
{
	int lz;

	asm ("cntlzd %0,%1" : "=r" (lz) : "r" (x));
	return 64 - lz;
}
#else
#include <asm-generic/bitops/fls64.h>
#endif /*               */

#ifdef CONFIG_PPC64
unsigned int __arch_hweight8(unsigned int w);
unsigned int __arch_hweight16(unsigned int w);
unsigned int __arch_hweight32(unsigned int w);
unsigned long __arch_hweight64(__u64 w);
#include <asm-generic/bitops/const_hweight.h>
#else
#include <asm-generic/bitops/hweight.h>
#endif

#include <asm-generic/bitops/find.h>

/*                        */
#include <asm-generic/bitops/le.h>

/*                                          */

#include <asm-generic/bitops/ext2-atomic-setbit.h>

#include <asm-generic/bitops/sched.h>

#endif /*            */

#endif /*                       */
