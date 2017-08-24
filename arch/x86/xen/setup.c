/*
                                 
  
                                                                  
 */

#include <linux/module.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/pm.h>
#include <linux/memblock.h>
#include <linux/cpuidle.h>
#include <linux/cpufreq.h>

#include <asm/elf.h>
#include <asm/vdso.h>
#include <asm/e820.h>
#include <asm/setup.h>
#include <asm/acpi.h>
#include <asm/numa.h>
#include <asm/xen/hypervisor.h>
#include <asm/xen/hypercall.h>

#include <xen/xen.h>
#include <xen/page.h>
#include <xen/interface/callback.h>
#include <xen/interface/memory.h>
#include <xen/interface/physdev.h>
#include <xen/features.h>
#include "xen-ops.h"
#include "vdso.h"

/*                                                        */
extern const char xen_hypervisor_callback[];
extern const char xen_failsafe_callback[];
extern void xen_sysenter_target(void);
extern void xen_syscall_target(void);
extern void xen_syscall32_target(void);

/*                                                        */
struct xen_memory_region xen_extra_mem[XEN_EXTRA_MEM_MAX_REGIONS] __initdata;

/*                                                       */
unsigned long xen_released_pages;

/* 
                                                                     
                                                                     
                                                             
                                                                 
        
   
                                                                      
                                       
 */
#define EXTRA_MEM_RATIO		(10)

static void __init xen_add_extra_mem(u64 start, u64 size)
{
	unsigned long pfn;
	int i;

	for (i = 0; i < XEN_EXTRA_MEM_MAX_REGIONS; i++) {
		/*                 */
		if (xen_extra_mem[i].size == 0) {
			xen_extra_mem[i].start = start;
			xen_extra_mem[i].size  = size;
			break;
		}
		/*                            */
		if (xen_extra_mem[i].start + xen_extra_mem[i].size == start) {
			xen_extra_mem[i].size += size;
			break;
		}
	}
	if (i == XEN_EXTRA_MEM_MAX_REGIONS)
		printk(KERN_WARNING "Warning: not enough extra memory regions\n");

	memblock_reserve(start, size);

	xen_max_p2m_pfn = PFN_DOWN(start + size);
	for (pfn = PFN_DOWN(start); pfn < xen_max_p2m_pfn; pfn++) {
		unsigned long mfn = pfn_to_mfn(pfn);

		if (WARN(mfn == pfn, "Trying to over-write 1-1 mapping (pfn: %lx)\n", pfn))
			continue;
		WARN(mfn != INVALID_P2M_ENTRY, "Trying to remove %lx which has %lx mfn!\n",
			pfn, mfn);

		__set_phys_to_machine(pfn, INVALID_P2M_ENTRY);
	}
}

static unsigned long __init xen_do_chunk(unsigned long start,
					 unsigned long end, bool release)
{
	struct xen_memory_reservation reservation = {
		.address_bits = 0,
		.extent_order = 0,
		.domid        = DOMID_SELF
	};
	unsigned long len = 0;
	unsigned long pfn;
	int ret;

	for (pfn = start; pfn < end; pfn++) {
		unsigned long frame;
		unsigned long mfn = pfn_to_mfn(pfn);

		if (release) {
			/*                                    */
			if (mfn == INVALID_P2M_ENTRY || mfn_to_pfn(mfn) != pfn)
				continue;
			frame = mfn;
		} else {
			if (mfn != INVALID_P2M_ENTRY)
				continue;
			frame = pfn;
		}
		set_xen_guest_handle(reservation.extent_start, &frame);
		reservation.nr_extents = 1;

		ret = HYPERVISOR_memory_op(release ? XENMEM_decrease_reservation : XENMEM_populate_physmap,
					   &reservation);
		WARN(ret != 1, "Failed to %s pfn %lx err=%d\n",
		     release ? "release" : "populate", pfn, ret);

		if (ret == 1) {
			if (!early_set_phys_to_machine(pfn, release ? INVALID_P2M_ENTRY : frame)) {
				if (release)
					break;
				set_xen_guest_handle(reservation.extent_start, &frame);
				reservation.nr_extents = 1;
				ret = HYPERVISOR_memory_op(XENMEM_decrease_reservation,
							   &reservation);
				break;
			}
			len++;
		} else
			break;
	}
	if (len)
		printk(KERN_INFO "%s %lx-%lx pfn range: %lu pages %s\n",
		       release ? "Freeing" : "Populating",
		       start, end, len,
		       release ? "freed" : "added");

	return len;
}

static unsigned long __init xen_release_chunk(unsigned long start,
					      unsigned long end)
{
	return xen_do_chunk(start, end, true);
}

static unsigned long __init xen_populate_chunk(
	const struct e820entry *list, size_t map_size,
	unsigned long max_pfn, unsigned long *last_pfn,
	unsigned long credits_left)
{
	const struct e820entry *entry;
	unsigned int i;
	unsigned long done = 0;
	unsigned long dest_pfn;

	for (i = 0, entry = list; i < map_size; i++, entry++) {
		unsigned long s_pfn;
		unsigned long e_pfn;
		unsigned long pfns;
		long capacity;

		if (credits_left <= 0)
			break;

		if (entry->type != E820_RAM)
			continue;

		e_pfn = PFN_DOWN(entry->addr + entry->size);

		/*                                                            */
		if (e_pfn <= max_pfn)
			continue;

		s_pfn = PFN_UP(entry->addr);
		/*                                                        
                         
                                                          
   */
		if (s_pfn <= max_pfn) {
			capacity = e_pfn - max_pfn;
			dest_pfn = max_pfn;
		} else {
			capacity = e_pfn - s_pfn;
			dest_pfn = s_pfn;
		}

		if (credits_left < capacity)
			capacity = credits_left;

		pfns = xen_do_chunk(dest_pfn, dest_pfn + capacity, false);
		done += pfns;
		*last_pfn = (dest_pfn + pfns);
		if (pfns < capacity)
			break;
		credits_left -= pfns;
	}
	return done;
}

static void __init xen_set_identity_and_release_chunk(
	unsigned long start_pfn, unsigned long end_pfn, unsigned long nr_pages,
	unsigned long *released, unsigned long *identity)
{
	unsigned long pfn;

	/*
                                                               
                            
  */
	for (pfn = start_pfn; pfn <= max_pfn_mapped && pfn < end_pfn; pfn++)
		(void)HYPERVISOR_update_va_mapping(
			(unsigned long)__va(pfn << PAGE_SHIFT),
			mfn_pte(pfn, PAGE_KERNEL_IO), 0);

	if (start_pfn < nr_pages)
		*released += xen_release_chunk(
			start_pfn, min(end_pfn, nr_pages));

	*identity += set_phys_range_identity(start_pfn, end_pfn);
}

static unsigned long __init xen_set_identity_and_release(
	const struct e820entry *list, size_t map_size, unsigned long nr_pages)
{
	phys_addr_t start = 0;
	unsigned long released = 0;
	unsigned long identity = 0;
	const struct e820entry *entry;
	int i;

	/*
                                                               
                                                        
                                                              
   
                                                              
                                                            
                                                          
                                                               
                        
  */
	for (i = 0, entry = list; i < map_size; i++, entry++) {
		phys_addr_t end = entry->addr + entry->size;
		if (entry->type == E820_RAM || i == map_size - 1) {
			unsigned long start_pfn = PFN_DOWN(start);
			unsigned long end_pfn = PFN_UP(end);

			if (entry->type == E820_RAM)
				end_pfn = PFN_UP(entry->addr);

			if (start_pfn < end_pfn)
				xen_set_identity_and_release_chunk(
					start_pfn, end_pfn, nr_pages,
					&released, &identity);

			start = end;
		}
	}

	if (released)
		printk(KERN_INFO "Released %lu pages of unused memory\n", released);
	if (identity)
		printk(KERN_INFO "Set %ld page(s) to 1-1 mapping\n", identity);

	return released;
}

static unsigned long __init xen_get_max_pages(void)
{
	unsigned long max_pages = MAX_DOMAIN_PAGES;
	domid_t domid = DOMID_SELF;
	int ret;

	/*
                                                            
                     
   
                                                              
                                                               
                                                          
                   
  */
	if (xen_initial_domain()) {
		ret = HYPERVISOR_memory_op(XENMEM_maximum_reservation, &domid);
		if (ret > 0)
			max_pages = ret;
	}

	return min(max_pages, MAX_DOMAIN_PAGES);
}

static void xen_align_and_add_e820_region(u64 start, u64 size, int type)
{
	u64 end = start + size;

	/*                                       */
	if (type == E820_RAM) {
		start = PAGE_ALIGN(start);
		end &= ~((u64)PAGE_SIZE - 1);
	}

	e820_add_region(start, end - start, type);
}

void xen_ignore_unusable(struct e820entry *list, size_t map_size)
{
	struct e820entry *entry;
	unsigned int i;

	for (i = 0, entry = list; i < map_size; i++, entry++) {
		if (entry->type == E820_UNUSABLE)
			entry->type = E820_RAM;
	}
}

/* 
                                                                          
  */
char * __init xen_memory_setup(void)
{
	static struct e820entry map[E820MAX] __initdata;

	unsigned long max_pfn = xen_start_info->nr_pages;
	unsigned long long mem_end;
	int rc;
	struct xen_memory_map memmap;
	unsigned long max_pages;
	unsigned long last_pfn = 0;
	unsigned long extra_pages = 0;
	unsigned long populated;
	int i;
	int op;

	max_pfn = min(MAX_DOMAIN_PAGES, max_pfn);
	mem_end = PFN_PHYS(max_pfn);

	memmap.nr_entries = E820MAX;
	set_xen_guest_handle(memmap.buffer, map);

	op = xen_initial_domain() ?
		XENMEM_machine_memory_map :
		XENMEM_memory_map;
	rc = HYPERVISOR_memory_op(op, &memmap);
	if (rc == -ENOSYS) {
		BUG_ON(xen_initial_domain());
		memmap.nr_entries = 1;
		map[0].addr = 0ULL;
		map[0].size = mem_end;
		/*                                             */
		map[0].size += 8ULL << 20;
		map[0].type = E820_RAM;
		rc = 0;
	}
	BUG_ON(rc);

	/*
                                                           
                                                               
                                                      
   
                                                           
                          
  */
	if (xen_initial_domain())
		xen_ignore_unusable(map, memmap.nr_entries);

	/*                                                        */
	sanitize_e820_map(map, memmap.nr_entries, &memmap.nr_entries);

	max_pages = xen_get_max_pages();
	if (max_pages > max_pfn)
		extra_pages += max_pages - max_pfn;

	/*
                                                              
                                                               
                            
  */
	xen_released_pages = xen_set_identity_and_release(
		map, memmap.nr_entries, max_pfn);

	/*
                                                               
              */
	populated = xen_populate_chunk(map, memmap.nr_entries,
			max_pfn, &last_pfn, xen_released_pages);

	xen_released_pages -= populated;
	extra_pages += xen_released_pages;

	if (last_pfn > max_pfn) {
		max_pfn = min(MAX_DOMAIN_PAGES, last_pfn);
		mem_end = PFN_PHYS(max_pfn);
	}
	/*
                                                         
                                                           
                                                             
                                                            
                          
   
                                                              
                                                         
                                                    
  */
	extra_pages = min(EXTRA_MEM_RATIO * min(max_pfn, PFN_DOWN(MAXMEM)),
			  extra_pages);
	i = 0;
	while (i < memmap.nr_entries) {
		u64 addr = map[i].addr;
		u64 size = map[i].size;
		u32 type = map[i].type;

		if (type == E820_RAM) {
			if (addr < mem_end) {
				size = min(size, mem_end - addr);
			} else if (extra_pages) {
				size = min(size, (u64)extra_pages * PAGE_SIZE);
				extra_pages -= size / PAGE_SIZE;
				xen_add_extra_mem(addr, size);
			} else
				type = E820_UNUSABLE;
		}

		xen_align_and_add_e820_region(addr, size, type);

		map[i].addr += size;
		map[i].size -= size;
		if (map[i].size == 0)
			i++;
	}

	/*
                                                            
                                                          
                   
  */
	e820_add_region(ISA_START_ADDRESS, ISA_END_ADDRESS - ISA_START_ADDRESS,
			E820_RESERVED);

	/*
                     
               
                     
                                                                  
                                                               
                                                                
                                                               
                                                              
                                                             
                                                              
                                                                     
                                                                   
                                                             
                                                                       
                                                                 
                                                                 
                                                                  
                                                         
                                                                   
                                                              
                                                               
                                          
  */
	memblock_reserve(__pa(xen_start_info->mfn_list),
			 xen_start_info->pt_base - xen_start_info->mfn_list);

	sanitize_e820_map(e820.map, ARRAY_SIZE(e820.map), &e820.nr_map);

	return "Xen";
}

/*
                                                                     
                                                                      
                                                                 
 */
static void __init fiddle_vdso(void)
{
#ifdef CONFIG_X86_32
	u32 *mask;
	mask = VDSO32_SYMBOL(&vdso32_int80_start, NOTE_MASK);
	*mask |= 1 << VDSO_NOTE_NONEGSEG_BIT;
	mask = VDSO32_SYMBOL(&vdso32_sysenter_start, NOTE_MASK);
	*mask |= 1 << VDSO_NOTE_NONEGSEG_BIT;
#endif
}

static int __cpuinit register_callback(unsigned type, const void *func)
{
	struct callback_register callback = {
		.type = type,
		.address = XEN_CALLBACK(__KERNEL_CS, func),
		.flags = CALLBACKF_mask_events,
	};

	return HYPERVISOR_callback_op(CALLBACKOP_register, &callback);
}

void __cpuinit xen_enable_sysenter(void)
{
	int ret;
	unsigned sysenter_feature;

#ifdef CONFIG_X86_32
	sysenter_feature = X86_FEATURE_SEP;
#else
	sysenter_feature = X86_FEATURE_SYSENTER32;
#endif

	if (!boot_cpu_has(sysenter_feature))
		return;

	ret = register_callback(CALLBACKTYPE_sysenter, xen_sysenter_target);
	if(ret != 0)
		setup_clear_cpu_cap(sysenter_feature);
}

void __cpuinit xen_enable_syscall(void)
{
#ifdef CONFIG_X86_64
	int ret;

	ret = register_callback(CALLBACKTYPE_syscall, xen_syscall_target);
	if (ret != 0) {
		printk(KERN_ERR "Failed to set syscall callback: %d\n", ret);
		/*                                            
                             */
	}

	if (boot_cpu_has(X86_FEATURE_SYSCALL32)) {
		ret = register_callback(CALLBACKTYPE_syscall32,
					xen_syscall32_target);
		if (ret != 0)
			setup_clear_cpu_cap(X86_FEATURE_SYSCALL32);
	}
#endif /*               */
}

void __init xen_arch_setup(void)
{
	xen_panic_handler_init();

	HYPERVISOR_vm_assist(VMASST_CMD_enable, VMASST_TYPE_4gb_segments);
	HYPERVISOR_vm_assist(VMASST_CMD_enable, VMASST_TYPE_writable_pagetables);

	if (!xen_feature(XENFEAT_auto_translated_physmap))
		HYPERVISOR_vm_assist(VMASST_CMD_enable,
				     VMASST_TYPE_pae_extended_cr3);

	if (register_callback(CALLBACKTYPE_event, xen_hypervisor_callback) ||
	    register_callback(CALLBACKTYPE_failsafe, xen_failsafe_callback))
		BUG();

	xen_enable_sysenter();
	xen_enable_syscall();

#ifdef CONFIG_ACPI
	if (!(xen_start_info->flags & SIF_INITDOMAIN)) {
		printk(KERN_INFO "ACPI in unprivileged domain disabled\n");
		disable_acpi();
	}
#endif

	memcpy(boot_command_line, xen_start_info->cmd_line,
	       MAX_GUEST_CMDLINE > COMMAND_LINE_SIZE ?
	       COMMAND_LINE_SIZE : MAX_GUEST_CMDLINE);

	/*                                                    */
	disable_cpuidle();
	disable_cpufreq();
	WARN_ON(xen_set_default_idle());
	fiddle_vdso();
#ifdef CONFIG_NUMA
	numa_off = 1;
#endif
}
