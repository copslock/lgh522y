/*
 * Copyright (C) 2013 Emilio López <emilio@elopez.com.ar>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Adjustable factor-based clock implementation
 */

#include <linux/clk-provider.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/string.h>

#include <linux/delay.h>

#include "clk-factors.h"

/*
                                                            
  
                        
                                                               
                                                            
                             
                                                                 
                                                    
 */

struct clk_factors {
	struct clk_hw hw;
	void __iomem *reg;
	struct clk_factors_config *config;
	void (*get_factors) (u32 *rate, u32 parent, u8 *n, u8 *k, u8 *m, u8 *p);
	spinlock_t *lock;
};

#define to_clk_factors(_hw) container_of(_hw, struct clk_factors, hw)

#define SETMASK(len, pos)		(((-1U) >> (31-len))  << (pos))
#define CLRMASK(len, pos)		(~(SETMASK(len, pos)))
#define FACTOR_GET(bit, len, reg)	(((reg) & SETMASK(len, bit)) >> (bit))

#define FACTOR_SET(bit, len, reg, val) \
	(((reg) & CLRMASK(len, bit)) | (val << (bit)))

static unsigned long clk_factors_recalc_rate(struct clk_hw *hw,
					     unsigned long parent_rate)
{
	u8 n = 1, k = 0, p = 0, m = 0;
	u32 reg;
	unsigned long rate;
	struct clk_factors *factors = to_clk_factors(hw);
	struct clk_factors_config *config = factors->config;

	/*                          */
	reg = readl(factors->reg);

	/*                                          */
	if (config->nwidth != SUNXI_FACTORS_NOT_APPLICABLE)
		n = FACTOR_GET(config->nshift, config->nwidth, reg);
	if (config->kwidth != SUNXI_FACTORS_NOT_APPLICABLE)
		k = FACTOR_GET(config->kshift, config->kwidth, reg);
	if (config->mwidth != SUNXI_FACTORS_NOT_APPLICABLE)
		m = FACTOR_GET(config->mshift, config->mwidth, reg);
	if (config->pwidth != SUNXI_FACTORS_NOT_APPLICABLE)
		p = FACTOR_GET(config->pshift, config->pwidth, reg);

	/* Calculate the rate */
	rate = (parent_rate * (n + config->n_start) * (k + 1) >> p) / (m + 1);

	return rate;
}

static long clk_factors_round_rate(struct clk_hw *hw, unsigned long rate,
				   unsigned long *parent_rate)
{
	struct clk_factors *factors = to_clk_factors(hw);
	factors->get_factors((u32 *)&rate, (u32)*parent_rate,
			     NULL, NULL, NULL, NULL);

	return rate;
}

static int clk_factors_set_rate(struct clk_hw *hw, unsigned long rate,
				unsigned long parent_rate)
{
	u8 n, k, m, p;
	u32 reg;
	struct clk_factors *factors = to_clk_factors(hw);
	struct clk_factors_config *config = factors->config;
	unsigned long flags = 0;

	factors->get_factors((u32 *)&rate, (u32)parent_rate, &n, &k, &m, &p);

	if (factors->lock)
		spin_lock_irqsave(factors->lock, flags);

	/*                          */
	reg = readl(factors->reg);

	/*                                                                  */
	reg = FACTOR_SET(config->nshift, config->nwidth, reg, n);
	reg = FACTOR_SET(config->kshift, config->kwidth, reg, k);
	reg = FACTOR_SET(config->mshift, config->mwidth, reg, m);
	reg = FACTOR_SET(config->pshift, config->pwidth, reg, p);

	/*                */
	writel(reg, factors->reg);

	/*                               */
	__delay((rate >> 20) * 500 / 2);

	if (factors->lock)
		spin_unlock_irqrestore(factors->lock, flags);

	return 0;
}

static const struct clk_ops clk_factors_ops = {
	.recalc_rate = clk_factors_recalc_rate,
	.round_rate = clk_factors_round_rate,
	.set_rate = clk_factors_set_rate,
};

/* 
                                                       
                      
                                      
                            
                                       
                                   
                                           
                                                    
                                                                        
                                             
 */
struct clk *clk_register_factors(struct device *dev, const char *name,
				 const char *parent_name,
				 unsigned long flags, void __iomem *reg,
				 struct clk_factors_config *config,
				 void (*get_factors)(u32 *rate, u32 parent,
						     u8 *n, u8 *k, u8 *m, u8 *p),
				 spinlock_t *lock)
{
	struct clk_factors *factors;
	struct clk *clk;
	struct clk_init_data init;

	/*                      */
	factors = kzalloc(sizeof(struct clk_factors), GFP_KERNEL);
	if (!factors) {
		pr_err("%s: could not allocate factors clk\n", __func__);
		return ERR_PTR(-ENOMEM);
	}

	init.name = name;
	init.ops = &clk_factors_ops;
	init.flags = flags;
	init.parent_names = (parent_name ? &parent_name : NULL);
	init.num_parents = (parent_name ? 1 : 0);

	/*                                */
	factors->reg = reg;
	factors->config = config;
	factors->lock = lock;
	factors->hw.init = &init;
	factors->get_factors = get_factors;

	/*                    */
	clk = clk_register(dev, &factors->hw);

	if (IS_ERR(clk))
		kfree(factors);

	return clk;
}
