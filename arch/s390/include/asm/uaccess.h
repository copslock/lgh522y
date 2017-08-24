/*
 *  S390 version
 *    Copyright IBM Corp. 1999, 2000
 *    Author(s): Hartmut Penner (hp@de.ibm.com),
 *               Martin Schwidefsky (schwidefsky@de.ibm.com)
 *
 *  Derived from "include/asm-i386/uaccess.h"
 */
#ifndef __S390_UACCESS_H
#define __S390_UACCESS_H

/*
                                     
 */
#include <linux/sched.h>
#include <linux/errno.h>
#include <asm/ctl_reg.h>

#define VERIFY_READ     0
#define VERIFY_WRITE    1


/*
                                                                       
                                                                         
                                               
  
                                                             
 */

#define MAKE_MM_SEG(a)  ((mm_segment_t) { (a) })


#define KERNEL_DS       MAKE_MM_SEG(0)
#define USER_DS         MAKE_MM_SEG(1)

#define get_ds()        (KERNEL_DS)
#define get_fs()        (current->thread.mm_segment)

#define set_fs(x) \
({									\
	unsigned long __pto;						\
	current->thread.mm_segment = (x);				\
	__pto = current->thread.mm_segment.ar4 ?			\
		S390_lowcore.user_asce : S390_lowcore.kernel_asce;	\
	__ctl_load(__pto, 7, 7);					\
})

#define segment_eq(a,b) ((a).ar4 == (b).ar4)

static inline int __range_ok(unsigned long addr, unsigned long size)
{
	return 1;
}

#define __access_ok(addr, size)				\
({							\
	__chk_user_ptr(addr);				\
	__range_ok((unsigned long)(addr), (size));	\
})

#define access_ok(type, addr, size) __access_ok(addr, size)

/*
                                                                       
                                                                        
                                                                      
                                                                        
              
  
                                                                     
                                                                       
                                                                      
                               
 */

struct exception_table_entry
{
	int insn, fixup;
};

static inline unsigned long extable_insn(const struct exception_table_entry *x)
{
	return (unsigned long)&x->insn + x->insn;
}

static inline unsigned long extable_fixup(const struct exception_table_entry *x)
{
	return (unsigned long)&x->fixup + x->fixup;
}

#define ARCH_HAS_SORT_EXTABLE
#define ARCH_HAS_SEARCH_EXTABLE

struct uaccess_ops {
	size_t (*copy_from_user)(size_t, const void __user *, void *);
	size_t (*copy_from_user_small)(size_t, const void __user *, void *);
	size_t (*copy_to_user)(size_t, void __user *, const void *);
	size_t (*copy_to_user_small)(size_t, void __user *, const void *);
	size_t (*copy_in_user)(size_t, void __user *, const void __user *);
	size_t (*clear_user)(size_t, void __user *);
	size_t (*strnlen_user)(size_t, const char __user *);
	size_t (*strncpy_from_user)(size_t, const char __user *, char *);
	int (*futex_atomic_op)(int op, u32 __user *, int oparg, int *old);
	int (*futex_atomic_cmpxchg)(u32 *, u32 __user *, u32 old, u32 new);
};

extern struct uaccess_ops uaccess;
extern struct uaccess_ops uaccess_std;
extern struct uaccess_ops uaccess_mvcos;
extern struct uaccess_ops uaccess_mvcos_switch;
extern struct uaccess_ops uaccess_pt;

extern int __handle_fault(unsigned long, unsigned long, int);

static inline int __put_user_fn(size_t size, void __user *ptr, void *x)
{
	size = uaccess.copy_to_user_small(size, ptr, x);
	return size ? -EFAULT : size;
}

static inline int __get_user_fn(size_t size, const void __user *ptr, void *x)
{
	size = uaccess.copy_from_user_small(size, ptr, x);
	return size ? -EFAULT : size;
}

/*
                                                                         
                                                             
 */
#define __put_user(x, ptr) \
({								\
	__typeof__(*(ptr)) __x = (x);				\
	int __pu_err = -EFAULT;					\
        __chk_user_ptr(ptr);                                    \
	switch (sizeof (*(ptr))) {				\
	case 1:							\
	case 2:							\
	case 4:							\
	case 8:							\
		__pu_err = __put_user_fn(sizeof (*(ptr)),	\
					 ptr, &__x);		\
		break;						\
	default:						\
		__put_user_bad();				\
		break;						\
	 }							\
	__pu_err;						\
})

#define put_user(x, ptr)					\
({								\
	might_fault();						\
	__put_user(x, ptr);					\
})


extern int __put_user_bad(void) __attribute__((noreturn));

#define __get_user(x, ptr)					\
({								\
	int __gu_err = -EFAULT;					\
	__chk_user_ptr(ptr);					\
	switch (sizeof(*(ptr))) {				\
	case 1: {						\
		unsigned char __x;				\
		__gu_err = __get_user_fn(sizeof (*(ptr)),	\
					 ptr, &__x);		\
		(x) = *(__force __typeof__(*(ptr)) *) &__x;	\
		break;						\
	};							\
	case 2: {						\
		unsigned short __x;				\
		__gu_err = __get_user_fn(sizeof (*(ptr)),	\
					 ptr, &__x);		\
		(x) = *(__force __typeof__(*(ptr)) *) &__x;	\
		break;						\
	};							\
	case 4: {						\
		unsigned int __x;				\
		__gu_err = __get_user_fn(sizeof (*(ptr)),	\
					 ptr, &__x);		\
		(x) = *(__force __typeof__(*(ptr)) *) &__x;	\
		break;						\
	};							\
	case 8: {						\
		unsigned long long __x;				\
		__gu_err = __get_user_fn(sizeof (*(ptr)),	\
					 ptr, &__x);		\
		(x) = *(__force __typeof__(*(ptr)) *) &__x;	\
		break;						\
	};							\
	default:						\
		__get_user_bad();				\
		break;						\
	}							\
	__gu_err;						\
})

#define get_user(x, ptr)					\
({								\
	might_fault();						\
	__get_user(x, ptr);					\
})

extern int __get_user_bad(void) __attribute__((noreturn));

#define __put_user_unaligned __put_user
#define __get_user_unaligned __get_user

/* 
                                                                              
                                             
                                          
                                  
  
                                                        
  
                                                                
                                                                     
  
                                                    
                                 
 */
static inline unsigned long __must_check
__copy_to_user(void __user *to, const void *from, unsigned long n)
{
	if (__builtin_constant_p(n) && (n <= 256))
		return uaccess.copy_to_user_small(n, to, from);
	else
		return uaccess.copy_to_user(n, to, from);
}

#define __copy_to_user_inatomic __copy_to_user
#define __copy_from_user_inatomic __copy_from_user

/* 
                                                        
                                             
                                          
                                  
  
                                                        
  
                                             
  
                                                    
                                 
 */
static inline unsigned long __must_check
copy_to_user(void __user *to, const void *from, unsigned long n)
{
	might_fault();
	return __copy_to_user(to, from, n);
}

/* 
                                                                                
                                               
                                        
                                  
  
                                                        
  
                                                                
                                                                     
  
                                                    
                                 
  
                                                                      
                                               
 */
static inline unsigned long __must_check
__copy_from_user(void *to, const void __user *from, unsigned long n)
{
	if (__builtin_constant_p(n) && (n <= 256))
		return uaccess.copy_from_user_small(n, from, to);
	else
		return uaccess.copy_from_user(n, from, to);
}

extern void copy_from_user_overflow(void)
#ifdef CONFIG_DEBUG_STRICT_USER_COPY_CHECKS
__compiletime_warning("copy_from_user() buffer size is not provably correct")
#endif
;

/* 
                                                          
                                               
                                        
                                  
  
                                                        
  
                                             
  
                                                    
                                 
  
                                                                      
                                               
 */
static inline unsigned long __must_check
copy_from_user(void *to, const void __user *from, unsigned long n)
{
	unsigned int sz = __compiletime_object_size(to);

	might_fault();
	if (unlikely(sz != -1 && sz < n)) {
		copy_from_user_overflow();
		return n;
	}
	return __copy_from_user(to, from, n);
}

static inline unsigned long __must_check
__copy_in_user(void __user *to, const void __user *from, unsigned long n)
{
	return uaccess.copy_in_user(n, to, from);
}

static inline unsigned long __must_check
copy_in_user(void __user *to, const void __user *from, unsigned long n)
{
	might_fault();
	return __copy_in_user(to, from, n);
}

/*
                                                
 */
static inline long __must_check
strncpy_from_user(char *dst, const char __user *src, long count)
{
	might_fault();
	return uaccess.strncpy_from_user(count, src, dst);
}

static inline unsigned long
strnlen_user(const char __user * src, unsigned long n)
{
	might_fault();
	return uaccess.strnlen_user(n, src);
}

/* 
                                                         
                               
  
                                                        
  
                                                         
  
                                                                
                           
  
                                                                       
                                         
 */
#define strlen_user(str) strnlen_user(str, ~0UL)

/*
                 
 */

static inline unsigned long __must_check
__clear_user(void __user *to, unsigned long n)
{
	return uaccess.clear_user(n, to);
}

static inline unsigned long __must_check
clear_user(void __user *to, unsigned long n)
{
	might_fault();
	return uaccess.clear_user(n, to);
}

extern int copy_to_user_real(void __user *dest, void *src, size_t count);
extern int copy_from_user_real(void *dest, void __user *src, size_t count);

#endif /*                  */
