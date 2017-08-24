#ifndef _ASM_X86_PGTABLE_3LEVEL_H
#define _ASM_X86_PGTABLE_3LEVEL_H

/*
 * Intel Physical Address Extension (PAE) Mode - three-level page
 * tables on PPro+ CPUs.
 *
 * Copyright (C) 1999 Ingo Molnar <mingo@redhat.com>
 */

#define pte_ERROR(e)							\
	pr_err("%s:%d: bad pte %p(%08lx%08lx)\n",			\
	       __FILE__, __LINE__, &(e), (e).pte_high, (e).pte_low)
#define pmd_ERROR(e)							\
	pr_err("%s:%d: bad pmd %p(%016Lx)\n",				\
	       __FILE__, __LINE__, &(e), pmd_val(e))
#define pgd_ERROR(e)							\
	pr_err("%s:%d: bad pgd %p(%016Lx)\n",				\
	       __FILE__, __LINE__, &(e), pgd_val(e))

/*                                                          
                                                           
                                                          
                                                            
                                                 
 */
static inline void native_set_pte(pte_t *ptep, pte_t pte)
{
	ptep->pte_high = pte.pte_high;
	smp_wmb();
	ptep->pte_low = pte.pte_low;
}

#define pmd_read_atomic pmd_read_atomic
/*
                                                                      
                                                                   
                                                                  
                                                                      
                                                          
                                                                     
                                                                  
                                                                    
                                                                  
                                                                
                                                                      
                                                                  
              
  
                                                                    
                                                                  
                                                             
  
                                                                   
                                                                     
                                                             
                                                                     
                                                                    
                                                                      
                                                                    
                                                                    
                                                                      
                                                                 
                                                                     
                                                                    
                                                                    
                                                                     
                                           
 */
static inline pmd_t pmd_read_atomic(pmd_t *pmdp)
{
	pmdval_t ret;
	u32 *tmp = (u32 *)pmdp;

	ret = (pmdval_t) (*tmp);
	if (ret) {
		/*
                                                            
                                         
   */
		smp_rmb();
		ret |= ((pmdval_t)*(tmp + 1)) << 32;
	}

	return (pmd_t) { ret };
}

static inline void native_set_pte_atomic(pte_t *ptep, pte_t pte)
{
	set_64bit((unsigned long long *)(ptep), native_pte_val(pte));
}

static inline void native_set_pmd(pmd_t *pmdp, pmd_t pmd)
{
	set_64bit((unsigned long long *)(pmdp), native_pmd_val(pmd));
}

static inline void native_set_pud(pud_t *pudp, pud_t pud)
{
	set_64bit((unsigned long long *)(pudp), native_pud_val(pud));
}

/*
                                                                              
                                                                             
           
 */
static inline void native_pte_clear(struct mm_struct *mm, unsigned long addr,
				    pte_t *ptep)
{
	ptep->pte_low = 0;
	smp_wmb();
	ptep->pte_high = 0;
}

static inline void native_pmd_clear(pmd_t *pmd)
{
	u32 *tmp = (u32 *)pmd;
	*tmp = 0;
	smp_wmb();
	*(tmp + 1) = 0;
}

static inline void pud_clear(pud_t *pudp)
{
	set_pud(pudp, __pud(0));

	/*
                                                               
                                                             
                                                            
                                                  
   
                                                                
                                                                   
                                                      
  */
}

#ifdef CONFIG_SMP
static inline pte_t native_ptep_get_and_clear(pte_t *ptep)
{
	pte_t res;

	/*                                                            */
	res.pte_low = xchg(&ptep->pte_low, 0);
	res.pte_high = ptep->pte_high;
	ptep->pte_high = 0;

	return res;
}
#else
#define native_ptep_get_and_clear(xp) native_local_ptep_get_and_clear(xp)
#endif

#ifdef CONFIG_SMP
union split_pmd {
	struct {
		u32 pmd_low;
		u32 pmd_high;
	};
	pmd_t pmd;
};
static inline pmd_t native_pmdp_get_and_clear(pmd_t *pmdp)
{
	union split_pmd res, *orig = (union split_pmd *)pmdp;

	/*                                                        */
	res.pmd_low = xchg(&orig->pmd_low, 0);
	res.pmd_high = orig->pmd_high;
	orig->pmd_high = 0;

	return res.pmd;
}
#else
#define native_pmdp_get_and_clear(xp) native_local_pmdp_get_and_clear(xp)
#endif

/*
                                                        
                                                
 */
#define pte_to_pgoff(pte) ((pte).pte_high)
#define pgoff_to_pte(off)						\
	((pte_t) { { .pte_low = _PAGE_FILE, .pte_high = (off) } })
#define PTE_FILE_MAX_BITS       32

/*                                 */
#define MAX_SWAPFILES_CHECK() BUILD_BUG_ON(MAX_SWAPFILES_SHIFT > 5)
#define __swp_type(x)			(((x).val) & 0x1f)
#define __swp_offset(x)			((x).val >> 5)
#define __swp_entry(type, offset)	((swp_entry_t){(type) | (offset) << 5})
#define __pte_to_swp_entry(pte)		((swp_entry_t){ (pte).pte_high })
#define __swp_entry_to_pte(x)		((pte_t){ { .pte_high = (x).val } })

#endif /*                           */
