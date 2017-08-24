#ifndef _SPARC64_TLBFLUSH_H
#define _SPARC64_TLBFLUSH_H

#include <linux/mm.h>
#include <asm/mmu_context.h>

/*                       */

#define TLB_BATCH_NR	192

struct tlb_batch {
	struct mm_struct *mm;
	unsigned long tlb_nr;
	unsigned long active;
	unsigned long vaddrs[TLB_BATCH_NR];
};

extern void flush_tsb_kernel_range(unsigned long start, unsigned long end);
extern void flush_tsb_user(struct tlb_batch *tb);
extern void flush_tsb_user_page(struct mm_struct *mm, unsigned long vaddr);

/*                       */

static inline void flush_tlb_mm(struct mm_struct *mm)
{
}

static inline void flush_tlb_page(struct vm_area_struct *vma,
				  unsigned long vmaddr)
{
}

static inline void flush_tlb_range(struct vm_area_struct *vma,
				   unsigned long start, unsigned long end)
{
}

void flush_tlb_kernel_range(unsigned long start, unsigned long end);

#define __HAVE_ARCH_ENTER_LAZY_MMU_MODE

extern void flush_tlb_pending(void);
extern void arch_enter_lazy_mmu_mode(void);
extern void arch_leave_lazy_mmu_mode(void);
#define arch_flush_lazy_mmu_mode()      do {} while (0)

/*                  */
extern void __flush_tlb_all(void);
extern void __flush_tlb_page(unsigned long context, unsigned long vaddr);
extern void __flush_tlb_kernel_range(unsigned long start, unsigned long end);

#ifndef CONFIG_SMP

static inline void global_flush_tlb_page(struct mm_struct *mm, unsigned long vaddr)
{
	__flush_tlb_page(CTX_HWBITS(mm->context), vaddr);
}

#else /*            */

extern void smp_flush_tlb_kernel_range(unsigned long start, unsigned long end);
extern void smp_flush_tlb_page(struct mm_struct *mm, unsigned long vaddr);

#define global_flush_tlb_page(mm, vaddr) \
	smp_flush_tlb_page(mm, vaddr)

#endif /*              */

#endif /*                     */
