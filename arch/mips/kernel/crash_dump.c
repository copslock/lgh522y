#include <linux/highmem.h>
#include <linux/bootmem.h>
#include <linux/crash_dump.h>
#include <asm/uaccess.h>
#include <linux/slab.h>

static int __init parse_savemaxmem(char *p)
{
	if (p)
		saved_max_pfn = (memparse(p, &p) >> PAGE_SHIFT) - 1;

	return 1;
}
__setup("savemaxmem=", parse_savemaxmem);


static void *kdump_buf_page;

/* 
                                                 
                                       
                                                                          
                                             
                                  
                                                                          
                                                                       
                                                           
  
                                                                   
                         
  
                                                                         
                                                                           
                               
 */
ssize_t copy_oldmem_page(unsigned long pfn, char *buf,
			 size_t csize, unsigned long offset, int userbuf)
{
	void  *vaddr;

	if (!csize)
		return 0;

	vaddr = kmap_atomic_pfn(pfn);

	if (!userbuf) {
		memcpy(buf, (vaddr + offset), csize);
		kunmap_atomic(vaddr);
	} else {
		if (!kdump_buf_page) {
			pr_warning("Kdump: Kdump buffer page not allocated\n");

			return -EFAULT;
		}
		copy_page(kdump_buf_page, vaddr);
		kunmap_atomic(vaddr);
		if (copy_to_user(buf, (kdump_buf_page + offset), csize))
			return -EFAULT;
	}

	return csize;
}

static int __init kdump_buf_page_init(void)
{
	int ret = 0;

	kdump_buf_page = kmalloc(PAGE_SIZE, GFP_KERNEL);
	if (!kdump_buf_page) {
		pr_warning("Kdump: Failed to allocate kdump buffer page\n");
		ret = -ENOMEM;
	}

	return ret;
}
arch_initcall(kdump_buf_page_init);
