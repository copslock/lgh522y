/*
 * OMAP36xx-specific clkops
 *
 * Copyright (C) 2010 Texas Instruments, Inc.
 * Copyright (C) 2010 Nokia Corporation
 *
 * Mike Turquette
 * Vijaykumar GN
 * Paul Walmsley
 *
 * Parts of this code are based on code written by
 * Richard Woodruff, Tony Lindgren, Tuukka Tikkanen, Karthik Dasu,
 * Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#undef DEBUG

#include <linux/kernel.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/io.h>

#include "clock.h"
#include "clock36xx.h"
#define to_clk_divider(_hw) container_of(_hw, struct clk_divider, hw)

/* 
                                                                         
                                                                   
                               
  
                                                                 
                                                            
                                                                   
                                                         
                                                              
 */
int omap36xx_pwrdn_clk_enable_with_hsdiv_restore(struct clk_hw *clk)
{
	struct clk_divider *parent;
	struct clk_hw *parent_hw;
	u32 dummy_v, orig_v;
	int ret;

	/*                              */
	ret = omap2_dflt_clk_enable(clk);

	parent_hw = __clk_get_hw(__clk_get_parent(clk->clk));
	parent = to_clk_divider(parent_hw);

	/*                      */
	if (!ret) {
		orig_v = __raw_readl(parent->reg);
		dummy_v = orig_v;

		/*                                                     */
		dummy_v ^= (1 << parent->shift);
		__raw_writel(dummy_v, parent->reg);

		/*                            */
		__raw_writel(orig_v, parent->reg);
	}

	return ret;
}
