/*
 * OMAP2xxx DVFS virtual clock functions
 *
 * Copyright (C) 2005-2008, 2012 Texas Instruments, Inc.
 * Copyright (C) 2004-2010 Nokia Corporation
 *
 * Contacts:
 * Richard Woodruff <r-woodruff2@ti.com>
 * Paul Walmsley
 *
 * Based on earlier work by Tuukka Tikkanen, Tony Lindgren,
 * Gordon McNutt and RidgeRun, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * XXX Some of this code should be replaceable by the upcoming OPP layer
 * code.  However, some notion of "rate set" is probably still necessary
 * for OMAP2xxx at least.  Rate sets should be generalized so they can be
 * used for any OMAP chip, not just OMAP2xxx.  In particular, Richard Woodruff
 * has in the past expressed a preference to use rate sets for OPP changes,
 * rather than dynamically recalculating the clock tree, so if someone wants
 * this badly enough to write the code to handle it, we should support it
 * as an option.
 */
#undef DEBUG

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/cpufreq.h>
#include <linux/slab.h>

#include "soc.h"
#include "clock.h"
#include "clock2xxx.h"
#include "opp2xxx.h"
#include "cm2xxx.h"
#include "cm-regbits-24xx.h"
#include "sdrc.h"
#include "sram.h"

const struct prcm_config *curr_prcm_set;
const struct prcm_config *rate_table;

/*
                                                             
                                                                     
                                           
 */
static unsigned long sys_ck_rate;

/* 
                                                     
                                 
  
                                                                           
 */
unsigned long omap2_table_mpu_recalc(struct clk_hw *clk,
				     unsigned long parent_rate)
{
	return curr_prcm_set->mpu_speed;
}

/*
                                                                                
  
                                                                       
                                                                           
                           
 */
long omap2_round_to_table_rate(struct clk_hw *hw, unsigned long rate,
			       unsigned long *parent_rate)
{
	const struct prcm_config *ptr;
	long highest_rate;

	highest_rate = -EINVAL;

	for (ptr = rate_table; ptr->mpu_speed; ptr++) {
		if (!(ptr->flags & cpu_mask))
			continue;
		if (ptr->xtal_speed != sys_ck_rate)
			continue;

		highest_rate = ptr->mpu_speed;

		/*                                           */
		if (ptr->mpu_speed <= rate)
			break;
	}
	return highest_rate;
}

/*                                               */
int omap2_select_table_rate(struct clk_hw *hw, unsigned long rate,
			    unsigned long parent_rate)
{
	u32 cur_rate, done_rate, bypass = 0, tmp;
	const struct prcm_config *prcm;
	unsigned long found_speed = 0;
	unsigned long flags;

	for (prcm = rate_table; prcm->mpu_speed; prcm++) {
		if (!(prcm->flags & cpu_mask))
			continue;

		if (prcm->xtal_speed != sys_ck_rate)
			continue;

		if (prcm->mpu_speed <= rate) {
			found_speed = prcm->mpu_speed;
			break;
		}
	}

	if (!found_speed) {
		printk(KERN_INFO "Could not set MPU rate to %luMHz\n",
		       rate / 1000000);
		return -EINVAL;
	}

	curr_prcm_set = prcm;
	cur_rate = omap2xxx_clk_get_core_rate();

	if (prcm->dpll_speed == cur_rate / 2) {
		omap2xxx_sdrc_reprogram(CORE_CLK_SRC_DPLL, 1);
	} else if (prcm->dpll_speed == cur_rate * 2) {
		omap2xxx_sdrc_reprogram(CORE_CLK_SRC_DPLL_X2, 1);
	} else if (prcm->dpll_speed != cur_rate) {
		local_irq_save(flags);

		if (prcm->dpll_speed == prcm->xtal_speed)
			bypass = 1;

		if ((prcm->cm_clksel2_pll & OMAP24XX_CORE_CLK_SRC_MASK) ==
		    CORE_CLK_SRC_DPLL_X2)
			done_rate = CORE_CLK_SRC_DPLL_X2;
		else
			done_rate = CORE_CLK_SRC_DPLL;

		/*             */
		omap2_cm_write_mod_reg(prcm->cm_clksel_mpu, MPU_MOD, CM_CLKSEL);

		/*                                    */
		omap2_cm_write_mod_reg(prcm->cm_clksel_dsp,
				 OMAP24XX_DSP_MOD, CM_CLKSEL);

		omap2_cm_write_mod_reg(prcm->cm_clksel_gfx, GFX_MOD, CM_CLKSEL);

		/*                          */
		tmp = omap2_cm_read_mod_reg(CORE_MOD, CM_CLKSEL1) & OMAP24XX_CLKSEL_DSS2_MASK;
		omap2_cm_write_mod_reg(prcm->cm_clksel1_core | tmp, CORE_MOD,
				 CM_CLKSEL1);

		if (cpu_is_omap2430())
			omap2_cm_write_mod_reg(prcm->cm_clksel_mdm,
					 OMAP2430_MDM_MOD, CM_CLKSEL);

		/*                                         */
		omap2xxx_sdrc_reprogram(CORE_CLK_SRC_DPLL_X2, 1);

		omap2_set_prcm(prcm->cm_clksel1_pll, prcm->base_sdrc_rfr,
			       bypass);

		omap2xxx_sdrc_init_params(omap2xxx_sdrc_dll_is_unlocked());
		omap2xxx_sdrc_reprogram(done_rate, 0);

		local_irq_restore(flags);
	}

	return 0;
}

/* 
                                                                        
                                                         
  
                                                                  
                                                                     
                                
 */
void omap2xxx_clkt_vps_check_bootloader_rates(void)
{
	const struct prcm_config *prcm = NULL;
	unsigned long rate;

	rate = omap2xxx_clk_get_core_rate();
	for (prcm = rate_table; prcm->mpu_speed; prcm++) {
		if (!(prcm->flags & cpu_mask))
			continue;
		if (prcm->xtal_speed != sys_ck_rate)
			continue;
		if (prcm->dpll_speed <= rate)
			break;
	}
	curr_prcm_set = prcm;
}

/* 
                                                                
  
                                                                     
                                                                    
                                                                
                                                          
                                  
 */
void omap2xxx_clkt_vps_late_init(void)
{
	struct clk *c;

	c = clk_get(NULL, "sys_ck");
	if (IS_ERR(c)) {
		WARN(1, "could not locate sys_ck\n");
	} else {
		sys_ck_rate = clk_get_rate(c);
		clk_put(c);
	}
}
