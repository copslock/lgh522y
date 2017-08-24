/*
 *  linux/arch/arm/common/vic.c
 *
 *  Copyright (C) 1999 - 2003 ARM Limited
 *  Copyright (C) 2000 Deep Blue Solutions Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/export.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/irqdomain.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/syscore_ops.h>
#include <linux/device.h>
#include <linux/amba/bus.h>
#include <linux/irqchip/arm-vic.h>

#include <asm/exception.h>
#include <asm/irq.h>

#include "irqchip.h"

#define VIC_IRQ_STATUS			0x00
#define VIC_FIQ_STATUS			0x04
#define VIC_INT_SELECT			0x0c	/*                  */
#define VIC_INT_SOFT			0x18
#define VIC_INT_SOFT_CLEAR		0x1c
#define VIC_PROTECT			0x20
#define VIC_PL190_VECT_ADDR		0x30	/*            */
#define VIC_PL190_DEF_VECT_ADDR		0x34	/*            */

#define VIC_VECT_ADDR0			0x100	/*                       */
#define VIC_VECT_CNTL0			0x200	/*                       */
#define VIC_ITCR			0x300	/*                           */

#define VIC_VECT_CNTL_ENABLE		(1 << 5)

#define VIC_PL192_VECT_ADDR		0xF00

/* 
                                    
                                                
                                        
                                                
                                                       
                                             
                                        
                                        
                                    
                                  
                                       
 */
struct vic_device {
	void __iomem	*base;
	int		irq;
	u32		valid_sources;
	u32		resume_sources;
	u32		resume_irqs;
	u32		int_select;
	u32		int_enable;
	u32		soft_int;
	u32		protect;
	struct irq_domain *domain;
};

/*                                                              */
static struct vic_device vic_devices[CONFIG_ARM_VIC_NR];

static int vic_id;

static void vic_handle_irq(struct pt_regs *regs);

/* 
                                         
                          
  
                                              
              
*/
static void vic_init2(void __iomem *base)
{
	int i;

	for (i = 0; i < 16; i++) {
		void __iomem *reg = base + VIC_VECT_CNTL0 + (i * 4);
		writel(VIC_VECT_CNTL_ENABLE | i, reg);
	}

	writel(32, base + VIC_PL190_DEF_VECT_ADDR);
}

#ifdef CONFIG_PM
static void resume_one_vic(struct vic_device *vic)
{
	void __iomem *base = vic->base;

	printk(KERN_DEBUG "%s: resuming vic at %p\n", __func__, base);

	/*                               */
	vic_init2(base);

	writel(vic->int_select, base + VIC_INT_SELECT);
	writel(vic->protect, base + VIC_PROTECT);

	/*                                                     */
	writel(vic->int_enable, base + VIC_INT_ENABLE);
	writel(~vic->int_enable, base + VIC_INT_ENABLE_CLEAR);

	/*                                        */

	writel(vic->soft_int, base + VIC_INT_SOFT);
	writel(~vic->soft_int, base + VIC_INT_SOFT_CLEAR);
}

static void vic_resume(void)
{
	int id;

	for (id = vic_id - 1; id >= 0; id--)
		resume_one_vic(vic_devices + id);
}

static void suspend_one_vic(struct vic_device *vic)
{
	void __iomem *base = vic->base;

	printk(KERN_DEBUG "%s: suspending vic at %p\n", __func__, base);

	vic->int_select = readl(base + VIC_INT_SELECT);
	vic->int_enable = readl(base + VIC_INT_ENABLE);
	vic->soft_int = readl(base + VIC_INT_SOFT);
	vic->protect = readl(base + VIC_PROTECT);

	/*                                              
                        */

	writel(vic->resume_irqs, base + VIC_INT_ENABLE);
	writel(~vic->resume_irqs, base + VIC_INT_ENABLE_CLEAR);
}

static int vic_suspend(void)
{
	int id;

	for (id = 0; id < vic_id; id++)
		suspend_one_vic(vic_devices + id);

	return 0;
}

struct syscore_ops vic_syscore_ops = {
	.suspend	= vic_suspend,
	.resume		= vic_resume,
};

/* 
                                             
  
                                                 
                                              
                                    
*/
static int __init vic_pm_init(void)
{
	if (vic_id > 0)
		register_syscore_ops(&vic_syscore_ops);

	return 0;
}
late_initcall(vic_pm_init);
#endif /*           */

static struct irq_chip vic_chip;

static int vic_irqdomain_map(struct irq_domain *d, unsigned int irq,
			     irq_hw_number_t hwirq)
{
	struct vic_device *v = d->host_data;

	/*                                                             */
	if (!(v->valid_sources & (1 << hwirq)))
		return -EPERM;
	irq_set_chip_and_handler(irq, &vic_chip, handle_level_irq);
	irq_set_chip_data(irq, v->base);
	set_irq_flags(irq, IRQF_VALID | IRQF_PROBE);
	return 0;
}

/*
                                                                    
                                                                  
                                                                
                                                                     
 */
static int handle_one_vic(struct vic_device *vic, struct pt_regs *regs)
{
	u32 stat, irq;
	int handled = 0;

	while ((stat = readl_relaxed(vic->base + VIC_IRQ_STATUS))) {
		irq = ffs(stat) - 1;
		handle_IRQ(irq_find_mapping(vic->domain, irq), regs);
		handled = 1;
	}

	return handled;
}

/*
                                                                      
              
 */
static asmlinkage void __exception_irq_entry vic_handle_irq(struct pt_regs *regs)
{
	int i, handled;

	do {
		for (i = 0, handled = 0; i < vic_id; ++i)
			handled |= handle_one_vic(&vic_devices[i], regs);
	} while (handled);
}

static struct irq_domain_ops vic_irqdomain_ops = {
	.map = vic_irqdomain_map,
	.xlate = irq_domain_xlate_onetwocell,
};

/* 
                                   
                                      
                                  
                                              
                                                                     
                                                       
  
                                                                          
                                                                         
                                              
  
                                                   
 */
static void __init vic_register(void __iomem *base, unsigned int irq,
				u32 valid_sources, u32 resume_sources,
				struct device_node *node)
{
	struct vic_device *v;
	int i;

	if (vic_id >= ARRAY_SIZE(vic_devices)) {
		printk(KERN_ERR "%s: too few VICs, increase CONFIG_ARM_VIC_NR\n", __func__);
		return;
	}

	v = &vic_devices[vic_id];
	v->base = base;
	v->valid_sources = valid_sources;
	v->resume_sources = resume_sources;
	v->irq = irq;
	set_handle_irq(vic_handle_irq);
	vic_id++;
	v->domain = irq_domain_add_simple(node, fls(valid_sources), irq,
					  &vic_irqdomain_ops, v);
	/*                                          */
	for (i = 0; i < fls(valid_sources); i++)
		if (valid_sources & (1 << i))
			irq_create_mapping(v->domain, i);
}

static void vic_ack_irq(struct irq_data *d)
{
	void __iomem *base = irq_data_get_irq_chip_data(d);
	unsigned int irq = d->hwirq;
	writel(1 << irq, base + VIC_INT_ENABLE_CLEAR);
	/*                                                               */
	writel(1 << irq, base + VIC_INT_SOFT_CLEAR);
}

static void vic_mask_irq(struct irq_data *d)
{
	void __iomem *base = irq_data_get_irq_chip_data(d);
	unsigned int irq = d->hwirq;
	writel(1 << irq, base + VIC_INT_ENABLE_CLEAR);
}

static void vic_unmask_irq(struct irq_data *d)
{
	void __iomem *base = irq_data_get_irq_chip_data(d);
	unsigned int irq = d->hwirq;
	writel(1 << irq, base + VIC_INT_ENABLE);
}

#if defined(CONFIG_PM)
static struct vic_device *vic_from_irq(unsigned int irq)
{
        struct vic_device *v = vic_devices;
	unsigned int base_irq = irq & ~31;
	int id;

	for (id = 0; id < vic_id; id++, v++) {
		if (v->irq == base_irq)
			return v;
	}

	return NULL;
}

static int vic_set_wake(struct irq_data *d, unsigned int on)
{
	struct vic_device *v = vic_from_irq(d->irq);
	unsigned int off = d->hwirq;
	u32 bit = 1 << off;

	if (!v)
		return -EINVAL;

	if (!(bit & v->resume_sources))
		return -EINVAL;

	if (on)
		v->resume_irqs |= bit;
	else
		v->resume_irqs &= ~bit;

	return 0;
}
#else
#define vic_set_wake NULL
#endif /*           */

static struct irq_chip vic_chip = {
	.name		= "VIC",
	.irq_ack	= vic_ack_irq,
	.irq_mask	= vic_mask_irq,
	.irq_unmask	= vic_unmask_irq,
	.irq_set_wake	= vic_set_wake,
};

static void __init vic_disable(void __iomem *base)
{
	writel(0, base + VIC_INT_SELECT);
	writel(0, base + VIC_INT_ENABLE);
	writel(~0, base + VIC_INT_ENABLE_CLEAR);
	writel(0, base + VIC_ITCR);
	writel(~0, base + VIC_INT_SOFT_CLEAR);
}

static void __init vic_clear_interrupts(void __iomem *base)
{
	unsigned int i;

	writel(0, base + VIC_PL190_VECT_ADDR);
	for (i = 0; i < 19; i++) {
		unsigned int value;

		value = readl(base + VIC_PL190_VECT_ADDR);
		writel(value, base + VIC_PL190_VECT_ADDR);
	}
}

/*
                                                                           
                                                                      
                                                                
                                                                  
                                                         
 */
static void __init vic_init_st(void __iomem *base, unsigned int irq_start,
			       u32 vic_sources, struct device_node *node)
{
	unsigned int i;
	int vic_2nd_block = ((unsigned long)base & ~PAGE_MASK) != 0;

	/*                                   */
	vic_disable(base);

	/*
                                                                    
                                                                 
                                                                
                                                      
  */
	if (vic_2nd_block) {
		vic_clear_interrupts(base);

		/*                                                            */
		for (i = 0; i < 16; i++) {
			void __iomem *reg = base + VIC_VECT_CNTL0 + (i * 4);
			writel(0, reg);
		}

		writel(32, base + VIC_PL190_DEF_VECT_ADDR);
	}

	vic_register(base, irq_start, vic_sources, 0, node);
}

void __init __vic_init(void __iomem *base, int irq_start,
			      u32 vic_sources, u32 resume_sources,
			      struct device_node *node)
{
	unsigned int i;
	u32 cellid = 0;
	enum amba_vendor vendor;

	/*                                                        */
	for (i = 0; i < 4; i++) {
		void __iomem *addr;
		addr = (void __iomem *)((u32)base & PAGE_MASK) + 0xfe0 + (i * 4);
		cellid |= (readl(addr) & 0xff) << (8 * i);
	}
	vendor = (cellid >> 12) & 0xff;
	printk(KERN_INFO "VIC @%p: id 0x%08x, vendor 0x%02x\n",
	       base, cellid, vendor);

	switch(vendor) {
	case AMBA_VENDOR_ST:
		vic_init_st(base, irq_start, vic_sources, node);
		return;
	default:
		printk(KERN_WARNING "VIC: unknown vendor, continuing anyways\n");
		/*              */
	case AMBA_VENDOR_ARM:
		break;
	}

	/*                                   */
	vic_disable(base);

	/*                                            */
	vic_clear_interrupts(base);

	vic_init2(base);

	vic_register(base, irq_start, vic_sources, resume_sources, node);
}

/* 
                                                          
                            
                                                               
                                                      
                                                                    
 */
void __init vic_init(void __iomem *base, unsigned int irq_start,
		     u32 vic_sources, u32 resume_sources)
{
	__vic_init(base, irq_start, vic_sources, resume_sources, NULL);
}

#ifdef CONFIG_OF
int __init vic_of_init(struct device_node *node, struct device_node *parent)
{
	void __iomem *regs;

	if (WARN(parent, "non-root VICs are not supported"))
		return -EINVAL;

	regs = of_iomap(node, 0);
	if (WARN_ON(!regs))
		return -EIO;

	/*
                                                                       
  */
	__vic_init(regs, 0, ~0, ~0, node);

	return 0;
}
IRQCHIP_DECLARE(arm_pl190_vic, "arm,pl190-vic", vic_of_init);
IRQCHIP_DECLARE(arm_pl192_vic, "arm,pl192-vic", vic_of_init);
IRQCHIP_DECLARE(arm_versatile_vic, "arm,versatile-vic", vic_of_init);
#endif /*           */
