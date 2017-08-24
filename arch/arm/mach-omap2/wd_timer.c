/*
 * OMAP2+ MPU WD_TIMER-specific code
 *
 * Copyright (C) 2012 Texas Instruments, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/err.h>

#include <linux/platform_data/omap-wd-timer.h>

#include "omap_hwmod.h"
#include "omap_device.h"
#include "wd_timer.h"
#include "common.h"
#include "prm.h"
#include "soc.h"

/*
                                                                  
                                                                       
                                                                          
                                                                    
                                     
 */
#define OMAP_WDT_WPS		0x34
#define OMAP_WDT_SPR		0x48

int omap2_wd_timer_disable(struct omap_hwmod *oh)
{
	void __iomem *base;

	if (!oh) {
		pr_err("%s: Could not look up wdtimer_hwmod\n", __func__);
		return -EINVAL;
	}

	base = omap_hwmod_get_mpu_rt_va(oh);
	if (!base) {
		pr_err("%s: Could not get the base address for %s\n",
				oh->name, __func__);
		return -EINVAL;
	}

	/*                                       */
	__raw_writel(0xAAAA, base + OMAP_WDT_SPR);
	while (__raw_readl(base + OMAP_WDT_WPS) & 0x10)
		cpu_relax();

	__raw_writel(0x5555, base + OMAP_WDT_SPR);
	while (__raw_readl(base + OMAP_WDT_WPS) & 0x10)
		cpu_relax();

	return 0;
}

/* 
                                                               
                           
  
                                                                      
                                                                  
                                                                   
                                                                     
                                                                      
                                       
  
                                                                  
                                                                  
                                                                        
                                               
 */
int omap2_wd_timer_reset(struct omap_hwmod *oh)
{
	int c = 0;

	/*                            */
	omap_hwmod_softreset(oh);

	/*                       */
	omap_test_timeout((omap_hwmod_read(oh,
					   oh->class->sysc->syss_offs)
			   & SYSS_RESETDONE_MASK),
			  MAX_MODULE_SOFTRESET_WAIT, c);

	if (oh->class->sysc->srst_udelay)
		udelay(oh->class->sysc->srst_udelay);

	if (c == MAX_MODULE_SOFTRESET_WAIT)
		pr_warning("%s: %s: softreset failed (waited %d usec)\n",
			   __func__, oh->name, MAX_MODULE_SOFTRESET_WAIT);
	else
		pr_debug("%s: %s: softreset in %d usec\n", __func__,
			 oh->name, c);

	return (c == MAX_MODULE_SOFTRESET_WAIT) ? -ETIMEDOUT :
		omap2_wd_timer_disable(oh);
}

static int __init omap_init_wdt(void)
{
	int id = -1;
	struct platform_device *pdev;
	struct omap_hwmod *oh;
	char *oh_name = "wd_timer2";
	char *dev_name = "omap_wdt";
	struct omap_wd_timer_platform_data pdata;

	if (!cpu_class_is_omap2() || of_have_populated_dt())
		return 0;

	oh = omap_hwmod_lookup(oh_name);
	if (!oh) {
		pr_err("Could not look up wd_timer%d hwmod\n", id);
		return -EINVAL;
	}

	pdata.read_reset_sources = prm_read_reset_sources;

	pdev = omap_device_build(dev_name, id, oh, &pdata,
				 sizeof(struct omap_wd_timer_platform_data));
	WARN(IS_ERR(pdev), "Can't build omap_device for %s:%s.\n",
	     dev_name, oh->name);
	return 0;
}
omap_subsys_initcall(omap_init_wdt);
