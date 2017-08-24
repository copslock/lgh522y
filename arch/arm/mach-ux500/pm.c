/*
 * Copyright (C) ST-Ericsson SA 2010-2013
 * Author: Rickard Andersson <rickard.andersson@stericsson.com> for
 *         ST-Ericsson.
 * Author: Daniel Lezcano <daniel.lezcano@linaro.org> for Linaro.
 * License terms: GNU General Public License (GPL) version 2
 *
 */

#include <linux/kernel.h>
#include <linux/irqchip/arm-gic.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/platform_data/arm-ux500-pm.h>

#include "db8500-regs.h"

/*                                 */
#define PRCM_ARM_WFI_STANDBY    (prcmu_base + 0x130)
#define PRCM_ARM_WFI_STANDBY_WFI0		0x08
#define PRCM_ARM_WFI_STANDBY_WFI1		0x10
#define PRCM_IOCR		(prcmu_base + 0x310)
#define PRCM_IOCR_IOFORCE			0x1

/*                                                  */
#define PRCM_A9_MASK_REQ	(prcmu_base + 0x328)
#define PRCM_A9_MASK_REQ_PRCM_A9_MASK_REQ	0x1

#define PRCM_A9_MASK_ACK	(prcmu_base + 0x32c)
#define PRCM_ARMITMSK31TO0	(prcmu_base + 0x11c)
#define PRCM_ARMITMSK63TO32	(prcmu_base + 0x120)
#define PRCM_ARMITMSK95TO64	(prcmu_base + 0x124)
#define PRCM_ARMITMSK127TO96	(prcmu_base + 0x128)
#define PRCM_POWER_STATE_VAL	(prcmu_base + 0x25C)
#define PRCM_ARMITVAL31TO0	(prcmu_base + 0x260)
#define PRCM_ARMITVAL63TO32	(prcmu_base + 0x264)
#define PRCM_ARMITVAL95TO64	(prcmu_base + 0x268)
#define PRCM_ARMITVAL127TO96	(prcmu_base + 0x26C)

static void __iomem *prcmu_base;

/*                                               */
int prcmu_gic_decouple(void)
{
	u32 val = readl(PRCM_A9_MASK_REQ);

	/*                               */
	writel(val | PRCM_A9_MASK_REQ_PRCM_A9_MASK_REQ,
	       PRCM_A9_MASK_REQ);

	/*                                   */
	readl(PRCM_A9_MASK_REQ);

	/*                                               */
	udelay(1);

	return 0;
}

/*                                               */
int prcmu_gic_recouple(void)
{
	u32 val = readl(PRCM_A9_MASK_REQ);

	/*                               */
	writel(val & ~PRCM_A9_MASK_REQ_PRCM_A9_MASK_REQ, PRCM_A9_MASK_REQ);

	return 0;
}

#define PRCMU_GIC_NUMBER_REGS 5

/*
                                                                    
                                                            
                                                                  
                                                                    
                                                                  
                                                           
                                                                    
          
 */
bool prcmu_gic_pending_irq(void)
{
	u32 pr; /*                  */
	u32 er; /*                 */
	void __iomem *dist_base = __io_address(U8500_GIC_DIST_BASE);
	int i;

	/*                                    */
	for (i = 0; i < PRCMU_GIC_NUMBER_REGS; i++) {

		pr = readl_relaxed(dist_base + GIC_DIST_PENDING_SET + i * 4);
		er = readl_relaxed(dist_base + GIC_DIST_ENABLE_SET + i * 4);

		if (pr & er)
			return true; /*                              */
	}

	return false;
}

/*
                                                             
                                                              
                                           
 */
bool prcmu_pending_irq(void)
{
	u32 it, im;
	int i;

	for (i = 0; i < PRCMU_GIC_NUMBER_REGS - 1; i++) {
		it = readl(PRCM_ARMITVAL31TO0 + i * 4);
		im = readl(PRCM_ARMITMSK31TO0 + i * 4);
		if (it & im)
			return true; /*                              */
	}

	return false;
}

/*
                                                                     
                                                                              
                                                                       
                         
 */
bool prcmu_is_cpu_in_wfi(int cpu)
{
	return readl(PRCM_ARM_WFI_STANDBY) & cpu ? PRCM_ARM_WFI_STANDBY_WFI1 :
		     PRCM_ARM_WFI_STANDBY_WFI0;
}

/*
                                                                     
                                                                     
 */
int prcmu_copy_gic_settings(void)
{
	u32 er; /*                 */
	void __iomem *dist_base = __io_address(U8500_GIC_DIST_BASE);
	int i;

	/*                         */
	for (i = 0; i < PRCMU_GIC_NUMBER_REGS - 1; i++) {
		er = readl_relaxed(dist_base +
				   GIC_DIST_ENABLE_SET + (i + 1) * 4);
		writel(er, PRCM_ARMITMSK31TO0 + i * 4);
	}

	return 0;
}

void __init ux500_pm_init(u32 phy_base, u32 size)
{
	prcmu_base = ioremap(phy_base, size);
	if (!prcmu_base) {
		pr_err("could not remap PRCMU for PM functions\n");
		return;
	}
	/*
                                                          
                                                             
  */
	prcmu_gic_recouple();
}
