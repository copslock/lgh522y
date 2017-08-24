/*
 * Initialize MMU support.
 *
 * Copyright (C) 1998-2003 Hewlett-Packard Co
 *	David Mosberger-Tang <davidm@hpl.hp.com>
 */
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/bootmem.h>
#include <linux/efi.h>
#include <linux/elf.h>
#include <linux/memblock.h>
#include <linux/mm.h>
#include <linux/mmzone.h>
#include <linux/module.h>
#include <linux/personality.h>
#include <linux/reboot.h>
#include <linux/slab.h>
#include <linux/swap.h>
#include <linux/proc_fs.h>
#include <linux/bitops.h>
#include <linux/kexec.h>

#include <asm/dma.h>
#include <asm/io.h>
#include <asm/machvec.h>
#include <asm/numa.h>
#include <asm/patch.h>
#include <asm/pgalloc.h>
#include <asm/sal.h>
#include <asm/sections.h>
#include <asm/tlb.h>
#include <asm/uaccess.h>
#include <asm/unistd.h>
#include <asm/mca.h>
#include <asm/paravirt.h>

extern void ia64_tlb_init (void);

unsigned long MAX_DMA_ADDRESS = PAGE_OFFSET + 0x100000000UL;

#ifdef CONFIG_VIRTUAL_MEM_MAP
unsigned long VMALLOC_END = VMALLOC_END_INIT;
EXPORT_SYMBOL(VMALLOC_END);
struct page *vmem_map;
EXPORT_SYMBOL(vmem_map);
#endif

struct page *zero_page_memmap_ptr;	/*                         */
EXPORT_SYMBOL(zero_page_memmap_ptr);

void
__ia64_sync_icache_dcache (pte_t pte)
{
	unsigned long addr;
	struct page *page;

	page = pte_page(pte);
	addr = (unsigned long) page_address(page);

	if (test_bit(PG_arch_1, &page->flags))
		return;				/*                                          */

	flush_icache_range(addr, addr + (PAGE_SIZE << compound_order(page)));
	set_bit(PG_arch_1, &page->flags);	/*                    */
}

/*
                                                                            
                                                                              
                                                              
 */
void
dma_mark_clean(void *addr, size_t size)
{
	unsigned long pg_addr, end;

	pg_addr = PAGE_ALIGN((unsigned long) addr);
	end = (unsigned long) addr + size;
	while (pg_addr + PAGE_SIZE <= end) {
		struct page *page = virt_to_page(pg_addr);
		set_bit(PG_arch_1, &page->flags);
		pg_addr += PAGE_SIZE;
	}
}

inline void
ia64_set_rbs_bot (void)
{
	unsigned long stack_size = rlimit_max(RLIMIT_STACK) & -16;

	if (stack_size > MAX_USER_STACK_SIZE)
		stack_size = MAX_USER_STACK_SIZE;
	current->thread.rbs_bot = PAGE_ALIGN(current->mm->start_stack - stack_size);
}

/*
                                                                      
                                                                  
                                                                    
                                    
 */
void
ia64_init_addr_space (void)
{
	struct vm_area_struct *vma;

	ia64_set_rbs_bot();

	/*
                                                                                
                                                                                  
                                                            
  */
	vma = kmem_cache_zalloc(vm_area_cachep, GFP_KERNEL);
	if (vma) {
		INIT_LIST_HEAD(&vma->anon_vma_chain);
		vma->vm_mm = current->mm;
		vma->vm_start = current->thread.rbs_bot & PAGE_MASK;
		vma->vm_end = vma->vm_start + PAGE_SIZE;
		vma->vm_flags = VM_DATA_DEFAULT_FLAGS|VM_GROWSUP|VM_ACCOUNT;
		vma->vm_page_prot = vm_get_page_prot(vma->vm_flags);
		down_write(&current->mm->mmap_sem);
		if (insert_vm_struct(current->mm, vma)) {
			up_write(&current->mm->mmap_sem);
			kmem_cache_free(vm_area_cachep, vma);
			return;
		}
		up_write(&current->mm->mmap_sem);
	}

	/*                                                                             */
	if (!(current->personality & MMAP_PAGE_ZERO)) {
		vma = kmem_cache_zalloc(vm_area_cachep, GFP_KERNEL);
		if (vma) {
			INIT_LIST_HEAD(&vma->anon_vma_chain);
			vma->vm_mm = current->mm;
			vma->vm_end = PAGE_SIZE;
			vma->vm_page_prot = __pgprot(pgprot_val(PAGE_READONLY) | _PAGE_MA_NAT);
			vma->vm_flags = VM_READ | VM_MAYREAD | VM_IO |
					VM_DONTEXPAND | VM_DONTDUMP;
			down_write(&current->mm->mmap_sem);
			if (insert_vm_struct(current->mm, vma)) {
				up_write(&current->mm->mmap_sem);
				kmem_cache_free(vm_area_cachep, vma);
				return;
			}
			up_write(&current->mm->mmap_sem);
		}
	}
}

void
free_initmem (void)
{
	free_reserved_area((unsigned long)ia64_imva(__init_begin),
			   (unsigned long)ia64_imva(__init_end),
			   0, "unused kernel");
}

void __init
free_initrd_mem (unsigned long start, unsigned long end)
{
	/*
                                                              
                                                                
                                                                
                                                
   
                                                            
                                      
                                  
   
                    
                         
                    
                    
                         
                    
                    
                         
                    
                    
                         
                    
                    
                         
                    
                    
                             
   
                                                                     
                                           
  */
	start = PAGE_ALIGN(start);
	end = end & PAGE_MASK;

	if (start < end)
		printk(KERN_INFO "Freeing initrd memory: %ldkB freed\n", (end - start) >> 10);

	for (; start < end; start += PAGE_SIZE) {
		if (!virt_addr_valid(start))
			continue;
		free_reserved_page(virt_to_page(start));
	}
}

/*
                                                         
 */
static struct page * __init
put_kernel_page (struct page *page, unsigned long address, pgprot_t pgprot)
{
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;

	if (!PageReserved(page))
		printk(KERN_ERR "put_kernel_page: page at 0x%p not in reserved memory\n",
		       page_address(page));

	pgd = pgd_offset_k(address);		/*                                 */

	{
		pud = pud_alloc(&init_mm, pgd, address);
		if (!pud)
			goto out;
		pmd = pmd_alloc(&init_mm, pud, address);
		if (!pmd)
			goto out;
		pte = pte_alloc_kernel(pmd, address);
		if (!pte)
			goto out;
		if (!pte_none(*pte))
			goto out;
		set_pte(pte, mk_pte(page, pgprot));
	}
  out:
	/*                       */
	return page;
}

static void __init
setup_gate (void)
{
	void *gate_section;
	struct page *page;

	/*
                                                             
                                                     
                                  
  */
	gate_section = paravirt_get_gate_section();
	page = virt_to_page(ia64_imva(gate_section));
	put_kernel_page(page, GATE_ADDR, PAGE_READONLY);
#ifdef HAVE_BUGGY_SEGREL
	page = virt_to_page(ia64_imva(gate_section + PAGE_SIZE));
	put_kernel_page(page, GATE_ADDR + PAGE_SIZE, PAGE_GATE);
#else
	put_kernel_page(page, GATE_ADDR + PERCPU_PAGE_SIZE, PAGE_GATE);
	/*                                                       */
	{
		unsigned long addr;

		for (addr = GATE_ADDR + PAGE_SIZE;
		     addr < GATE_ADDR + PERCPU_PAGE_SIZE;
		     addr += PAGE_SIZE)
		{
			put_kernel_page(ZERO_PAGE(0), addr,
					PAGE_READONLY);
			put_kernel_page(ZERO_PAGE(0), addr + PERCPU_PAGE_SIZE,
					PAGE_READONLY);
		}
	}
#endif
	ia64_patch_gate();
}

void ia64_mmu_init(void *my_cpu_data)
{
	unsigned long pta, impl_va_bits;
	extern void tlb_init(void);

#ifdef CONFIG_DISABLE_VHPT
#	define VHPT_ENABLE_BIT	0
#else
#	define VHPT_ENABLE_BIT	1
#endif

	/*
                                                                                  
                                                                              
                                                                                 
                                                                                  
                                                                                   
                                                                             
                                                                                
                                                                           
                     
  */
#	define pte_bits			3
#	define mapped_space_bits	(3*(PAGE_SHIFT - pte_bits) + PAGE_SHIFT)
	/*
                                                                                   
                                                                               
                                                                    
                                                                                   
                                                                        
  */
#	define vmlpt_bits		(impl_va_bits - PAGE_SHIFT + pte_bits)
#	define POW2(n)			(1ULL << (n))

	impl_va_bits = ffz(~(local_cpu_data->unimpl_va_mask | (7UL << 61)));

	if (impl_va_bits < 51 || impl_va_bits > 61)
		panic("CPU has bogus IMPL_VA_MSB value of %lu!\n", impl_va_bits - 1);
	/*
                                                                       
                                                                     
                                                                 
                                                   
  */
	if ((mapped_space_bits - PAGE_SHIFT > vmlpt_bits - pte_bits) ||
	    (mapped_space_bits > impl_va_bits - 1))
		panic("Cannot build a big enough virtual-linear page table"
		      " to cover mapped address space.\n"
		      " Try using a smaller page size.\n");


	/*                                                              */
	pta = POW2(61) - POW2(vmlpt_bits);

	/*
                                                              
                                                             
                                                           
            
  */
	ia64_set_pta(pta | (0 << 8) | (vmlpt_bits << 2) | VHPT_ENABLE_BIT);

	ia64_tlb_init();

#ifdef	CONFIG_HUGETLB_PAGE
	ia64_set_rr(HPAGE_REGION_BASE, HPAGE_SHIFT << 2);
	ia64_srlz_d();
#endif
}

#ifdef CONFIG_VIRTUAL_MEM_MAP
int vmemmap_find_next_valid_pfn(int node, int i)
{
	unsigned long end_address, hole_next_pfn;
	unsigned long stop_address;
	pg_data_t *pgdat = NODE_DATA(node);

	end_address = (unsigned long) &vmem_map[pgdat->node_start_pfn + i];
	end_address = PAGE_ALIGN(end_address);

	stop_address = (unsigned long) &vmem_map[
		pgdat->node_start_pfn + pgdat->node_spanned_pages];

	do {
		pgd_t *pgd;
		pud_t *pud;
		pmd_t *pmd;
		pte_t *pte;

		pgd = pgd_offset_k(end_address);
		if (pgd_none(*pgd)) {
			end_address += PGDIR_SIZE;
			continue;
		}

		pud = pud_offset(pgd, end_address);
		if (pud_none(*pud)) {
			end_address += PUD_SIZE;
			continue;
		}

		pmd = pmd_offset(pud, end_address);
		if (pmd_none(*pmd)) {
			end_address += PMD_SIZE;
			continue;
		}

		pte = pte_offset_kernel(pmd, end_address);
retry_pte:
		if (pte_none(*pte)) {
			end_address += PAGE_SIZE;
			pte++;
			if ((end_address < stop_address) &&
			    (end_address != ALIGN(end_address, 1UL << PMD_SHIFT)))
				goto retry_pte;
			continue;
		}
		/*                                */
		break;
	} while (end_address < stop_address);

	end_address = min(end_address, stop_address);
	end_address = end_address - (unsigned long) vmem_map + sizeof(struct page) - 1;
	hole_next_pfn = end_address / sizeof(struct page);
	return hole_next_pfn - pgdat->node_start_pfn;
}

int __init create_mem_map_page_table(u64 start, u64 end, void *arg)
{
	unsigned long address, start_page, end_page;
	struct page *map_start, *map_end;
	int node;
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;

	map_start = vmem_map + (__pa(start) >> PAGE_SHIFT);
	map_end   = vmem_map + (__pa(end) >> PAGE_SHIFT);

	start_page = (unsigned long) map_start & PAGE_MASK;
	end_page = PAGE_ALIGN((unsigned long) map_end);
	node = paddr_to_nid(__pa(start));

	for (address = start_page; address < end_page; address += PAGE_SIZE) {
		pgd = pgd_offset_k(address);
		if (pgd_none(*pgd))
			pgd_populate(&init_mm, pgd, alloc_bootmem_pages_node(NODE_DATA(node), PAGE_SIZE));
		pud = pud_offset(pgd, address);

		if (pud_none(*pud))
			pud_populate(&init_mm, pud, alloc_bootmem_pages_node(NODE_DATA(node), PAGE_SIZE));
		pmd = pmd_offset(pud, address);

		if (pmd_none(*pmd))
			pmd_populate_kernel(&init_mm, pmd, alloc_bootmem_pages_node(NODE_DATA(node), PAGE_SIZE));
		pte = pte_offset_kernel(pmd, address);

		if (pte_none(*pte))
			set_pte(pte, pfn_pte(__pa(alloc_bootmem_pages_node(NODE_DATA(node), PAGE_SIZE)) >> PAGE_SHIFT,
					     PAGE_KERNEL));
	}
	return 0;
}

struct memmap_init_callback_data {
	struct page *start;
	struct page *end;
	int nid;
	unsigned long zone;
};

static int __meminit
virtual_memmap_init(u64 start, u64 end, void *arg)
{
	struct memmap_init_callback_data *args;
	struct page *map_start, *map_end;

	args = (struct memmap_init_callback_data *) arg;
	map_start = vmem_map + (__pa(start) >> PAGE_SHIFT);
	map_end   = vmem_map + (__pa(end) >> PAGE_SHIFT);

	if (map_start < args->start)
		map_start = args->start;
	if (map_end > args->end)
		map_end = args->end;

	/*
                                                                                  
                                                                                   
                                                         
  */
	map_start -= ((unsigned long) map_start & (PAGE_SIZE - 1)) / sizeof(struct page);
	map_end += ((PAGE_ALIGN((unsigned long) map_end) - (unsigned long) map_end)
		    / sizeof(struct page));

	if (map_start < map_end)
		memmap_init_zone((unsigned long)(map_end - map_start),
				 args->nid, args->zone, page_to_pfn(map_start),
				 MEMMAP_EARLY);
	return 0;
}

void __meminit
memmap_init (unsigned long size, int nid, unsigned long zone,
	     unsigned long start_pfn)
{
	if (!vmem_map)
		memmap_init_zone(size, nid, zone, start_pfn, MEMMAP_EARLY);
	else {
		struct page *start;
		struct memmap_init_callback_data args;

		start = pfn_to_page(start_pfn);
		args.start = start;
		args.end = start + size;
		args.nid = nid;
		args.zone = zone;

		efi_memmap_walk(virtual_memmap_init, &args);
	}
}

int
ia64_pfn_valid (unsigned long pfn)
{
	char byte;
	struct page *pg = pfn_to_page(pfn);

	return     (__get_user(byte, (char __user *) pg) == 0)
		&& ((((u64)pg & PAGE_MASK) == (((u64)(pg + 1) - 1) & PAGE_MASK))
			|| (__get_user(byte, (char __user *) (pg + 1) - 1) == 0));
}
EXPORT_SYMBOL(ia64_pfn_valid);

int __init find_largest_hole(u64 start, u64 end, void *arg)
{
	u64 *max_gap = arg;

	static u64 last_end = PAGE_OFFSET;

	/*                                                          */

	if (*max_gap < (start - last_end))
		*max_gap = start - last_end;
	last_end = end;
	return 0;
}

#endif /*                        */

int __init register_active_ranges(u64 start, u64 len, int nid)
{
	u64 end = start + len;

#ifdef CONFIG_KEXEC
	if (start > crashk_res.start && start < crashk_res.end)
		start = crashk_res.end;
	if (end > crashk_res.start && end < crashk_res.end)
		end = crashk_res.start;
#endif

	if (start < end)
		memblock_add_node(__pa(start), end - start, nid);
	return 0;
}

static int __init
count_reserved_pages(u64 start, u64 end, void *arg)
{
	unsigned long num_reserved = 0;
	unsigned long *count = arg;

	for (; start < end; start += PAGE_SIZE)
		if (PageReserved(virt_to_page(start)))
			++num_reserved;
	*count += num_reserved;
	return 0;
}

int
find_max_min_low_pfn (u64 start, u64 end, void *arg)
{
	unsigned long pfn_start, pfn_end;
#ifdef CONFIG_FLATMEM
	pfn_start = (PAGE_ALIGN(__pa(start))) >> PAGE_SHIFT;
	pfn_end = (PAGE_ALIGN(__pa(end - 1))) >> PAGE_SHIFT;
#else
	pfn_start = GRANULEROUNDDOWN(__pa(start)) >> PAGE_SHIFT;
	pfn_end = GRANULEROUNDUP(__pa(end - 1)) >> PAGE_SHIFT;
#endif
	min_low_pfn = min(min_low_pfn, pfn_start);
	max_low_pfn = max(max_low_pfn, pfn_end);
	return 0;
}

/*
                                                                                        
                                                                                          
                                                                                       
                                                                                         
            
 */

static int nolwsys __initdata;

static int __init
nolwsys_setup (char *s)
{
	nolwsys = 1;
	return 1;
}

__setup("nolwsys", nolwsys_setup);

void __init
mem_init (void)
{
	long reserved_pages, codesize, datasize, initsize;
	pg_data_t *pgdat;
	int i;

	BUG_ON(PTRS_PER_PGD * sizeof(pgd_t) != PAGE_SIZE);
	BUG_ON(PTRS_PER_PMD * sizeof(pmd_t) != PAGE_SIZE);
	BUG_ON(PTRS_PER_PTE * sizeof(pte_t) != PAGE_SIZE);

#ifdef CONFIG_PCI
	/*
                                                                                 
                                                                                  
               
  */
	platform_dma_init();
#endif

#ifdef CONFIG_FLATMEM
	BUG_ON(!mem_map);
	max_mapnr = max_low_pfn;
#endif

	high_memory = __va(max_low_pfn * PAGE_SIZE);

	for_each_online_pgdat(pgdat)
		if (pgdat->bdata->node_bootmem_map)
			totalram_pages += free_all_bootmem_node(pgdat);

	reserved_pages = 0;
	efi_memmap_walk(count_reserved_pages, &reserved_pages);

	codesize =  (unsigned long) _etext - (unsigned long) _stext;
	datasize =  (unsigned long) _edata - (unsigned long) _etext;
	initsize =  (unsigned long) __init_end - (unsigned long) __init_begin;

	printk(KERN_INFO "Memory: %luk/%luk available (%luk code, %luk reserved, "
	       "%luk data, %luk init)\n", nr_free_pages() << (PAGE_SHIFT - 10),
	       num_physpages << (PAGE_SHIFT - 10), codesize >> 10,
	       reserved_pages << (PAGE_SHIFT - 10), datasize >> 10, initsize >> 10);


	/*
                                                                          
                                                                               
                             
  */
	for (i = 0; i < NR_syscalls; ++i) {
		extern unsigned long sys_call_table[NR_syscalls];
		unsigned long *fsyscall_table = paravirt_get_fsyscall_table();

		if (!fsyscall_table[i] || nolwsys)
			fsyscall_table[i] = sys_call_table[i] | 1;
	}
	setup_gate();
}

#ifdef CONFIG_MEMORY_HOTPLUG
int arch_add_memory(int nid, u64 start, u64 size)
{
	pg_data_t *pgdat;
	struct zone *zone;
	unsigned long start_pfn = start >> PAGE_SHIFT;
	unsigned long nr_pages = size >> PAGE_SHIFT;
	int ret;

	pgdat = NODE_DATA(nid);

	zone = pgdat->node_zones + ZONE_NORMAL;
	ret = __add_pages(nid, zone, start_pfn, nr_pages);

	if (ret)
		printk("%s: Problem encountered in __add_pages() as ret=%d\n",
		       __func__,  ret);

	return ret;
}

#ifdef CONFIG_MEMORY_HOTREMOVE
int arch_remove_memory(u64 start, u64 size)
{
	unsigned long start_pfn = start >> PAGE_SHIFT;
	unsigned long nr_pages = size >> PAGE_SHIFT;
	struct zone *zone;
	int ret;

	zone = page_zone(pfn_to_page(start_pfn));
	ret = __remove_pages(zone, start_pfn, nr_pages);
	if (ret)
		pr_warn("%s: Problem encountered in __remove_pages() as"
			" ret=%d\n", __func__,  ret);

	return ret;
}
#endif
#endif

/*
                                                     
                                                    
                                                     
                                                            
                     
 */
static struct exec_domain ia32_exec_domain;

static int __init
per_linux32_init(void)
{
	ia32_exec_domain.name = "Linux/x86";
	ia32_exec_domain.handler = NULL;
	ia32_exec_domain.pers_low = PER_LINUX32;
	ia32_exec_domain.pers_high = PER_LINUX32;
	ia32_exec_domain.signal_map = default_exec_domain.signal_map;
	ia32_exec_domain.signal_invmap = default_exec_domain.signal_invmap;
	register_exec_domain(&ia32_exec_domain);

	return 0;
}

__initcall(per_linux32_init);
