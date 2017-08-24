#ifndef _ASM_X86_ATOMIC64_32_H
#define _ASM_X86_ATOMIC64_32_H

#include <linux/compiler.h>
#include <linux/types.h>
#include <asm/processor.h>
//                        

/*                      */

typedef struct {
	u64 __aligned(8) counter;
} atomic64_t;

#define ATOMIC64_INIT(val)	{ (val) }

#define __ATOMIC64_DECL(sym) void atomic64_##sym(atomic64_t *, ...)
#ifndef ATOMIC64_EXPORT
#define ATOMIC64_DECL_ONE __ATOMIC64_DECL
#else
#define ATOMIC64_DECL_ONE(sym) __ATOMIC64_DECL(sym); \
	ATOMIC64_EXPORT(atomic64_##sym)
#endif

#ifdef CONFIG_X86_CMPXCHG64
#define __alternative_atomic64(f, g, out, in...) \
	asm volatile("call %P[func]" \
		     : out : [func] "i" (atomic64_##g##_cx8), ## in)

#define ATOMIC64_DECL(sym) ATOMIC64_DECL_ONE(sym##_cx8)
#else
#define __alternative_atomic64(f, g, out, in...) \
	alternative_call(atomic64_##f##_386, atomic64_##g##_cx8, \
			 X86_FEATURE_CX8, ASM_OUTPUT2(out), ## in)

#define ATOMIC64_DECL(sym) ATOMIC64_DECL_ONE(sym##_cx8); \
	ATOMIC64_DECL_ONE(sym##_386)

ATOMIC64_DECL_ONE(add_386);
ATOMIC64_DECL_ONE(sub_386);
ATOMIC64_DECL_ONE(inc_386);
ATOMIC64_DECL_ONE(dec_386);
#endif

#define alternative_atomic64(f, out, in...) \
	__alternative_atomic64(f, f, ASM_OUTPUT2(out), ## in)

ATOMIC64_DECL(read);
ATOMIC64_DECL(set);
ATOMIC64_DECL(xchg);
ATOMIC64_DECL(add_return);
ATOMIC64_DECL(sub_return);
ATOMIC64_DECL(inc_return);
ATOMIC64_DECL(dec_return);
ATOMIC64_DECL(dec_if_positive);
ATOMIC64_DECL(inc_not_zero);
ATOMIC64_DECL(add_unless);

#undef ATOMIC64_DECL
#undef ATOMIC64_DECL_ONE
#undef __ATOMIC64_DECL
#undef ATOMIC64_EXPORT

/* 
                                               
                                 
                     
                
  
                                                             
                 
 */

static inline long long atomic64_cmpxchg(atomic64_t *v, long long o, long long n)
{
	return cmpxchg64(&v->counter, o, n);
}

/* 
                                         
                                 
                      
  
                                                     
                 
 */
static inline long long atomic64_xchg(atomic64_t *v, long long n)
{
	long long o;
	unsigned high = (unsigned)(n >> 32);
	unsigned low = (unsigned)n;
	alternative_atomic64(xchg, "=&A" (o),
			     "S" (v), "b" (low), "c" (high)
			     : "memory");
	return o;
}

/* 
                                       
                                 
                      
  
                                         
 */
static inline void atomic64_set(atomic64_t *v, long long i)
{
	unsigned high = (unsigned)(i >> 32);
	unsigned low = (unsigned)i;
	alternative_atomic64(set, /*           */,
			     "S" (v), "b" (low), "c" (high)
			     : "eax", "edx", "memory");
}

/* 
                                         
                                 
  
                                                   
 */
static inline long long atomic64_read(const atomic64_t *v)
{
	long long r;
	alternative_atomic64(read, "=&A" (r), "c" (v) : "memory");
	return r;
 }

/* 
                                       
                           
                                 
  
                                                
 */
static inline long long atomic64_add_return(long long i, atomic64_t *v)
{
	alternative_atomic64(add_return,
			     ASM_OUTPUT2("+A" (i), "+c" (v)),
			     ASM_NO_INPUT_CLOBBER("memory"));
	return i;
}

/*
                                                      
 */
static inline long long atomic64_sub_return(long long i, atomic64_t *v)
{
	alternative_atomic64(sub_return,
			     ASM_OUTPUT2("+A" (i), "+c" (v)),
			     ASM_NO_INPUT_CLOBBER("memory"));
	return i;
}

static inline long long atomic64_inc_return(atomic64_t *v)
{
	long long a;
	alternative_atomic64(inc_return, "=&A" (a),
			     "S" (v) : "memory", "ecx");
	return a;
}

static inline long long atomic64_dec_return(atomic64_t *v)
{
	long long a;
	alternative_atomic64(dec_return, "=&A" (a),
			     "S" (v) : "memory", "ecx");
	return a;
}

/* 
                                                  
                           
                                 
  
                            
 */
static inline long long atomic64_add(long long i, atomic64_t *v)
{
	__alternative_atomic64(add, add_return,
			       ASM_OUTPUT2("+A" (i), "+c" (v)),
			       ASM_NO_INPUT_CLOBBER("memory"));
	return i;
}

/* 
                                                
                                
                                 
  
                                   
 */
static inline long long atomic64_sub(long long i, atomic64_t *v)
{
	__alternative_atomic64(sub, sub_return,
			       ASM_OUTPUT2("+A" (i), "+c" (v)),
			       ASM_NO_INPUT_CLOBBER("memory"));
	return i;
}

/* 
                                                                       
                                
                                 
  
                                              
                                               
               
 */
static inline int atomic64_sub_and_test(long long i, atomic64_t *v)
{
	return atomic64_sub_return(i, v) == 0;
}

/* 
                                             
                                 
  
                                 
 */
static inline void atomic64_inc(atomic64_t *v)
{
	__alternative_atomic64(inc, inc_return, /*           */,
			       "S" (v) : "memory", "eax", "ecx", "edx");
}

/* 
                                             
                                 
  
                                 
 */
static inline void atomic64_dec(atomic64_t *v)
{
	__alternative_atomic64(dec, dec_return, /*           */,
			       "S" (v) : "memory", "eax", "ecx", "edx");
}

/* 
                                             
                                 
  
                                    
                                                          
         
 */
static inline int atomic64_dec_and_test(atomic64_t *v)
{
	return atomic64_dec_return(v) == 0;
}

/* 
                                             
                                 
  
                                
                                                           
               
 */
static inline int atomic64_inc_and_test(atomic64_t *v)
{
	return atomic64_inc_return(v) == 0;
}

/* 
                                                   
                           
                                 
  
                                            
                                           
                                           
 */
static inline int atomic64_add_negative(long long i, atomic64_t *v)
{
	return atomic64_add_return(i, v) < 0;
}

/* 
                                                               
                                 
                                
                                 
  
                                                      
                                                        
 */
static inline int atomic64_add_unless(atomic64_t *v, long long a, long long u)
{
	unsigned low = (unsigned)u;
	unsigned high = (unsigned)(u >> 32);
	alternative_atomic64(add_unless,
			     ASM_OUTPUT2("+A" (a), "+c" (low), "+D" (high)),
			     "S" (v) : "memory");
	return (int)a;
}


static inline int atomic64_inc_not_zero(atomic64_t *v)
{
	int r;
	alternative_atomic64(inc_not_zero, "=&a" (r),
			     "S" (v) : "ecx", "edx", "memory");
	return r;
}

static inline long long atomic64_dec_if_positive(atomic64_t *v)
{
	long long r;
	alternative_atomic64(dec_if_positive, "=&A" (r),
			     "S" (v) : "ecx", "memory");
	return r;
}

#undef alternative_atomic64
#undef __alternative_atomic64

#endif /*                        */
