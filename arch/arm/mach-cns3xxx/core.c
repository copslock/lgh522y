/*
 * Copyright 1999 - 2003 ARM Limited
 * Copyright 2000 Deep Blue Solutions Ltd
 * Copyright 2008 Cavium Networks
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, Version 2, as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/clockchips.h>
#include <linux/io.h>
#include <linux/irqchip/arm-gic.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/usb/ehci_pdriver.h>
#include <linux/usb/ohci_pdriver.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/time.h>
#include <asm/mach/irq.h>
#include <asm/hardware/cache-l2x0.h>
#include "cns3xxx.h"
#include "core.h"
#include "pm.h"

static struct map_desc cns3xxx_io_desc[] __initdata = {
	{
		.virtual	= CNS3XXX_TC11MP_SCU_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_TC11MP_SCU_BASE),
		.length		= SZ_8K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_TIMER1_2_3_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_TIMER1_2_3_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_MISC_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_MISC_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_PM_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_PM_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	},
};

void __init cns3xxx_map_io(void)
{
	iotable_init(cns3xxx_io_desc, ARRAY_SIZE(cns3xxx_io_desc));
}

/*                       */
void __init cns3xxx_init_irq(void)
{
	gic_init(0, 29, IOMEM(CNS3XXX_TC11MP_GIC_DIST_BASE_VIRT),
		 IOMEM(CNS3XXX_TC11MP_GIC_CPU_BASE_VIRT));
}

void cns3xxx_power_off(void)
{
	u32 __iomem *pm_base = IOMEM(CNS3XXX_PM_BASE_VIRT);
	u32 clkctrl;

	printk(KERN_INFO "powering system down...\n");

	clkctrl = readl(pm_base + PM_SYS_CLK_CTRL_OFFSET);
	clkctrl &= 0xfffff1ff;
	clkctrl |= (0x5 << 9);		/*           */
	writel(clkctrl, pm_base + PM_SYS_CLK_CTRL_OFFSET);

}

/*
        
 */
static void __iomem *cns3xxx_tmr1;

static void cns3xxx_timer_set_mode(enum clock_event_mode mode,
				   struct clock_event_device *clk)
{
	unsigned long ctrl = readl(cns3xxx_tmr1 + TIMER1_2_CONTROL_OFFSET);
	int pclk = cns3xxx_cpu_clock() / 8;
	int reload;

	switch (mode) {
	case CLOCK_EVT_MODE_PERIODIC:
		reload = pclk * 20 / (3 * HZ) * 0x25000;
		writel(reload, cns3xxx_tmr1 + TIMER1_AUTO_RELOAD_OFFSET);
		ctrl |= (1 << 0) | (1 << 2) | (1 << 9);
		break;
	case CLOCK_EVT_MODE_ONESHOT:
		/*                                                    */
		ctrl |= (1 << 2) | (1 << 9);
		break;
	case CLOCK_EVT_MODE_UNUSED:
	case CLOCK_EVT_MODE_SHUTDOWN:
	default:
		ctrl = 0;
	}

	writel(ctrl, cns3xxx_tmr1 + TIMER1_2_CONTROL_OFFSET);
}

static int cns3xxx_timer_set_next_event(unsigned long evt,
					struct clock_event_device *unused)
{
	unsigned long ctrl = readl(cns3xxx_tmr1 + TIMER1_2_CONTROL_OFFSET);

	writel(evt, cns3xxx_tmr1 + TIMER1_AUTO_RELOAD_OFFSET);
	writel(ctrl | (1 << 0), cns3xxx_tmr1 + TIMER1_2_CONTROL_OFFSET);

	return 0;
}

static struct clock_event_device cns3xxx_tmr1_clockevent = {
	.name		= "cns3xxx timer1",
	.features       = CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT,
	.set_mode	= cns3xxx_timer_set_mode,
	.set_next_event	= cns3xxx_timer_set_next_event,
	.rating		= 350,
	.cpumask	= cpu_all_mask,
};

static void __init cns3xxx_clockevents_init(unsigned int timer_irq)
{
	cns3xxx_tmr1_clockevent.irq = timer_irq;
	clockevents_config_and_register(&cns3xxx_tmr1_clockevent,
					(cns3xxx_cpu_clock() >> 3) * 1000000,
					0xf, 0xffffffff);
}

/*
                            
 */
static irqreturn_t cns3xxx_timer_interrupt(int irq, void *dev_id)
{
	struct clock_event_device *evt = &cns3xxx_tmr1_clockevent;
	u32 __iomem *stat = cns3xxx_tmr1 + TIMER1_2_INTERRUPT_STATUS_OFFSET;
	u32 val;

	/*                     */
	val = readl(stat);
	writel(val & ~(1 << 2), stat);

	evt->event_handler(evt);

	return IRQ_HANDLED;
}

static struct irqaction cns3xxx_timer_irq = {
	.name		= "timer",
	.flags		= IRQF_DISABLED | IRQF_TIMER | IRQF_IRQPOLL,
	.handler	= cns3xxx_timer_interrupt,
};

/*
                                                   
 */
static void __init __cns3xxx_timer_init(unsigned int timer_irq)
{
	u32 val;
	u32 irq_mask;

	/*
                                                
  */

	/*                           */
	writel(0, cns3xxx_tmr1 + TIMER1_2_CONTROL_OFFSET);
	/*                          */
	writel(0, cns3xxx_tmr1 + TIMER_FREERUN_CONTROL_OFFSET);

	/*        */
	writel(0x5C800, cns3xxx_tmr1 + TIMER1_COUNTER_OFFSET);
	writel(0x5C800, cns3xxx_tmr1 + TIMER1_AUTO_RELOAD_OFFSET);

	writel(0, cns3xxx_tmr1 + TIMER1_MATCH_V1_OFFSET);
	writel(0, cns3xxx_tmr1 + TIMER1_MATCH_V2_OFFSET);

	/*                                    */
	irq_mask = readl(cns3xxx_tmr1 + TIMER1_2_INTERRUPT_MASK_OFFSET);
	irq_mask &= ~(1 << 2);
	irq_mask |= 0x03;
	writel(irq_mask, cns3xxx_tmr1 + TIMER1_2_INTERRUPT_MASK_OFFSET);

	/*              */
	val = readl(cns3xxx_tmr1 + TIMER1_2_CONTROL_OFFSET);
	val |= (1 << 9);
	writel(val, cns3xxx_tmr1 + TIMER1_2_CONTROL_OFFSET);

	/*        */
	writel(0, cns3xxx_tmr1 + TIMER2_MATCH_V1_OFFSET);
	writel(0, cns3xxx_tmr1 + TIMER2_MATCH_V2_OFFSET);

	/*          */
	irq_mask = readl(cns3xxx_tmr1 + TIMER1_2_INTERRUPT_MASK_OFFSET);
	irq_mask |= ((1 << 3) | (1 << 4) | (1 << 5));
	writel(irq_mask, cns3xxx_tmr1 + TIMER1_2_INTERRUPT_MASK_OFFSET);

	/*              */
	val = readl(cns3xxx_tmr1 + TIMER1_2_CONTROL_OFFSET);
	val |= (1 << 10);
	writel(val, cns3xxx_tmr1 + TIMER1_2_CONTROL_OFFSET);

	/*                                       */
	setup_irq(timer_irq, &cns3xxx_timer_irq);

	cns3xxx_clockevents_init(timer_irq);
}

void __init cns3xxx_timer_init(void)
{
	cns3xxx_tmr1 = IOMEM(CNS3XXX_TIMER1_2_3_BASE_VIRT);

	__cns3xxx_timer_init(IRQ_CNS3XXX_TIMER0);
}

#ifdef CONFIG_CACHE_L2X0

void __init cns3xxx_l2x0_init(void)
{
	void __iomem *base = ioremap(CNS3XXX_L2C_BASE, SZ_4K);
	u32 val;

	if (WARN_ON(!base))
		return;

	/*
                            
   
                                                 
                                               
                                       
   
                                                         
  */
	val = readl(base + L2X0_TAG_LATENCY_CTRL);
	val &= 0xfffff888;
	writel(val, base + L2X0_TAG_LATENCY_CTRL);

	/*
                             
   
                                                  
                                                
                                       
   
                                                         
  */
	val = readl(base + L2X0_DATA_LATENCY_CTRL);
	val &= 0xfffff888;
	writel(val, base + L2X0_DATA_LATENCY_CTRL);

	/*                               */
	l2x0_init(base, 0x00540000, 0xfe000fff);
}

#endif /*                   */

static int csn3xxx_usb_power_on(struct platform_device *pdev)
{
	/*
                                                 
                                                              
                                                           
                                       
   
                                 
  */
	if (atomic_inc_return(&usb_pwr_ref) == 1) {
		cns3xxx_pwr_power_up(1 << PM_PLL_HM_PD_CTRL_REG_OFFSET_PLL_USB);
		cns3xxx_pwr_clk_en(1 << PM_CLK_GATE_REG_OFFSET_USB_HOST);
		cns3xxx_pwr_soft_rst(1 << PM_SOFT_RST_REG_OFFST_USB_HOST);
		__raw_writel((__raw_readl(MISC_CHIP_CONFIG_REG) | (0X2 << 24)),
			MISC_CHIP_CONFIG_REG);
	}

	return 0;
}

static void csn3xxx_usb_power_off(struct platform_device *pdev)
{
	/*
                                                 
                                                              
                                                           
                                       
  */
	if (atomic_dec_return(&usb_pwr_ref) == 0)
		cns3xxx_pwr_clk_dis(1 << PM_CLK_GATE_REG_OFFSET_USB_HOST);
}

static struct usb_ehci_pdata cns3xxx_usb_ehci_pdata = {
	.power_on	= csn3xxx_usb_power_on,
	.power_off	= csn3xxx_usb_power_off,
};

static struct usb_ohci_pdata cns3xxx_usb_ohci_pdata = {
	.num_ports	= 1,
	.power_on	= csn3xxx_usb_power_on,
	.power_off	= csn3xxx_usb_power_off,
};

static struct of_dev_auxdata cns3xxx_auxdata[] __initconst = {
	{ "intel,usb-ehci", CNS3XXX_USB_BASE, "ehci-platform", &cns3xxx_usb_ehci_pdata },
	{ "intel,usb-ohci", CNS3XXX_USB_OHCI_BASE, "ohci-platform", &cns3xxx_usb_ohci_pdata },
	{ "cavium,cns3420-ahci", CNS3XXX_SATA2_BASE, "ahci", NULL },
	{ "cavium,cns3420-sdhci", CNS3XXX_SDIO_BASE, "ahci", NULL },
	{},
};

static void __init cns3xxx_init(void)
{
	struct device_node *dn;

	cns3xxx_l2x0_init();

	dn = of_find_compatible_node(NULL, NULL, "cavium,cns3420-ahci");
	if (of_device_is_available(dn)) {
		u32 tmp;
	
		tmp = __raw_readl(MISC_SATA_POWER_MODE);
		tmp |= 0x1 << 16; /*                                      */
		tmp |= 0x1 << 17; /*                                      */
		__raw_writel(tmp, MISC_SATA_POWER_MODE);
	
		/*                 */
		cns3xxx_pwr_power_up(0x1 << PM_PLL_HM_PD_CTRL_REG_OFFSET_SATA_PHY0);
		cns3xxx_pwr_power_up(0x1 << PM_PLL_HM_PD_CTRL_REG_OFFSET_SATA_PHY1);
	
		/*                   */
		cns3xxx_pwr_clk_en(0x1 << PM_CLK_GATE_REG_OFFSET_SATA);
	
		/*                      */
		cns3xxx_pwr_soft_rst(CNS3XXX_PWR_SOFTWARE_RST(SATA));
	}

	dn = of_find_compatible_node(NULL, NULL, "cavium,cns3420-sdhci");
	if (of_device_is_available(dn)) {
		u32 __iomem *gpioa = IOMEM(CNS3XXX_MISC_BASE_VIRT + 0x0014);
		u32 gpioa_pins = __raw_readl(gpioa);
	
		/*                              */
		gpioa_pins |= 0x1fff0004;
		__raw_writel(gpioa_pins, gpioa);
	
		cns3xxx_pwr_clk_en(CNS3XXX_PWR_CLK_EN(SDIO));
		cns3xxx_pwr_soft_rst(CNS3XXX_PWR_SOFTWARE_RST(SDIO));
	}

	pm_power_off = cns3xxx_power_off;

	of_platform_populate(NULL, of_default_bus_match_table,
                        cns3xxx_auxdata, NULL);
}

static const char *cns3xxx_dt_compat[] __initdata = {
	"cavium,cns3410",
	"cavium,cns3420",
	NULL,
};

DT_MACHINE_START(CNS3XXX_DT, "Cavium Networks CNS3xxx")
	.dt_compat	= cns3xxx_dt_compat,
	.nr_irqs	= NR_IRQS_CNS3XXX,
	.map_io		= cns3xxx_map_io,
	.init_irq	= cns3xxx_init_irq,
	.init_time	= cns3xxx_timer_init,
	.init_machine	= cns3xxx_init,
	.restart	= cns3xxx_restart,
MACHINE_END
