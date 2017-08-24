/*
 * Copyright 2008 Cavium Networks
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, Version 2, as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/atomic.h>
#include "cns3xxx.h"
#include "pm.h"
#include "core.h"

void cns3xxx_pwr_clk_en(unsigned int block)
{
	u32 reg = __raw_readl(PM_CLK_GATE_REG);

	reg |= (block & PM_CLK_GATE_REG_MASK);
	__raw_writel(reg, PM_CLK_GATE_REG);
}
EXPORT_SYMBOL(cns3xxx_pwr_clk_en);

void cns3xxx_pwr_clk_dis(unsigned int block)
{
	u32 reg = __raw_readl(PM_CLK_GATE_REG);

	reg &= ~(block & PM_CLK_GATE_REG_MASK);
	__raw_writel(reg, PM_CLK_GATE_REG);
}
EXPORT_SYMBOL(cns3xxx_pwr_clk_dis);

void cns3xxx_pwr_power_up(unsigned int block)
{
	u32 reg = __raw_readl(PM_PLL_HM_PD_CTRL_REG);

	reg &= ~(block & CNS3XXX_PWR_PLL_ALL);
	__raw_writel(reg, PM_PLL_HM_PD_CTRL_REG);

	/*                                                 */
	udelay(300);
};
EXPORT_SYMBOL(cns3xxx_pwr_power_up);

void cns3xxx_pwr_power_down(unsigned int block)
{
	u32 reg = __raw_readl(PM_PLL_HM_PD_CTRL_REG);

	/*                         */
	reg |= (block & CNS3XXX_PWR_PLL_ALL);
	__raw_writel(reg, PM_PLL_HM_PD_CTRL_REG);
};
EXPORT_SYMBOL(cns3xxx_pwr_power_down);

static void cns3xxx_pwr_soft_rst_force(unsigned int block)
{
	u32 reg = __raw_readl(PM_SOFT_RST_REG);

	/*
                                          
                                            
  */
	if (block & 0x30000001) {
		reg &= ~(block & PM_SOFT_RST_REG_MASK);
	} else {
		reg &= ~(block & PM_SOFT_RST_REG_MASK);
		__raw_writel(reg, PM_SOFT_RST_REG);
		reg |= (block & PM_SOFT_RST_REG_MASK);
	}

	__raw_writel(reg, PM_SOFT_RST_REG);
}
EXPORT_SYMBOL(cns3xxx_pwr_soft_rst_force);

void cns3xxx_pwr_soft_rst(unsigned int block)
{
	static unsigned int soft_reset;

	if (soft_reset & block) {
		/*                                              */
		return;
	} else {
		soft_reset |= block;
	}
	cns3xxx_pwr_soft_rst_force(block);
}
EXPORT_SYMBOL(cns3xxx_pwr_soft_rst);

void cns3xxx_restart(char mode, const char *cmd)
{
	/*
                                                
                       
  */
	cns3xxx_pwr_soft_rst(CNS3XXX_PWR_SOFTWARE_RST(GLOBAL));
}

/*
                                          
                     
                     
                     
 */
int cns3xxx_cpu_clock(void)
{
	u32 reg = __raw_readl(PM_CLK_CTRL_REG);
	int cpu;
	int cpu_sel;
	int div_sel;

	cpu_sel = (reg >> PM_CLK_CTRL_REG_OFFSET_PLL_CPU_SEL) & 0xf;
	div_sel = (reg >> PM_CLK_CTRL_REG_OFFSET_CPU_CLK_DIV) & 0x3;

	cpu = (300 + ((cpu_sel / 3) * 100) + ((cpu_sel % 3) * 33)) >> div_sel;

	return cpu;
}
EXPORT_SYMBOL(cns3xxx_cpu_clock);

atomic_t usb_pwr_ref = ATOMIC_INIT(0);
EXPORT_SYMBOL(usb_pwr_ref);
