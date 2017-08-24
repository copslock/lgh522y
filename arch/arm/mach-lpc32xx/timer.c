/*
 * arch/arm/mach-lpc32xx/timer.c
 *
 * Author: Kevin Wells <kevin.wells@nxp.com>
 *
 * Copyright (C) 2009 - 2010 NXP Semiconductors
 * Copyright (C) 2009 Fontys University of Applied Sciences, Eindhoven
 *                    Ed Schouten <e.schouten@fontys.nl>
 *                    Laurens Timmermans <l.timmermans@fontys.nl>
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
 */

#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/time.h>
#include <linux/err.h>
#include <linux/clockchips.h>

#include <asm/mach/time.h>

#include <mach/hardware.h>
#include <mach/platform.h>
#include "common.h"

static int lpc32xx_clkevt_next_event(unsigned long delta,
    struct clock_event_device *dev)
{
	__raw_writel(LPC32XX_TIMER_CNTR_TCR_RESET,
		LPC32XX_TIMER_TCR(LPC32XX_TIMER0_BASE));
	__raw_writel(delta, LPC32XX_TIMER_PR(LPC32XX_TIMER0_BASE));
	__raw_writel(LPC32XX_TIMER_CNTR_TCR_EN,
		LPC32XX_TIMER_TCR(LPC32XX_TIMER0_BASE));

	return 0;
}

static void lpc32xx_clkevt_mode(enum clock_event_mode mode,
    struct clock_event_device *dev)
{
	switch (mode) {
	case CLOCK_EVT_MODE_PERIODIC:
		WARN_ON(1);
		break;

	case CLOCK_EVT_MODE_ONESHOT:
	case CLOCK_EVT_MODE_SHUTDOWN:
		/*
                                                        
                                                    
                      
   */
		__raw_writel(0, LPC32XX_TIMER_TCR(LPC32XX_TIMER0_BASE));
		break;

	case CLOCK_EVT_MODE_UNUSED:
	case CLOCK_EVT_MODE_RESUME:
		break;
	}
}

static struct clock_event_device lpc32xx_clkevt = {
	.name		= "lpc32xx_clkevt",
	.features	= CLOCK_EVT_FEAT_ONESHOT,
	.rating		= 300,
	.set_next_event	= lpc32xx_clkevt_next_event,
	.set_mode	= lpc32xx_clkevt_mode,
};

static irqreturn_t lpc32xx_timer_interrupt(int irq, void *dev_id)
{
	struct clock_event_device *evt = &lpc32xx_clkevt;

	/*             */
	__raw_writel(LPC32XX_TIMER_CNTR_MTCH_BIT(0),
		LPC32XX_TIMER_IR(LPC32XX_TIMER0_BASE));

	evt->event_handler(evt);

	return IRQ_HANDLED;
}

static struct irqaction lpc32xx_timer_irq = {
	.name		= "LPC32XX Timer Tick",
	.flags		= IRQF_DISABLED | IRQF_TIMER | IRQF_IRQPOLL,
	.handler	= lpc32xx_timer_interrupt,
};

/*
                                                                      
                                                                     
                                  
 */
void __init lpc32xx_timer_init(void)
{
	u32 clkrate, pllreg;

	/*                    */
	__raw_writel(LPC32XX_CLKPWR_TMRPWMCLK_TIMER0_EN |
		LPC32XX_CLKPWR_TMRPWMCLK_TIMER1_EN,
		LPC32XX_CLKPWR_TIMERS_PWMS_CLK_CTRL_1);

	/*
                                                                     
                                                                        
                                                                        
                                                              
  */
	if (clk_is_sysclk_mainosc() != 0)
		clkrate = LPC32XX_MAIN_OSC_FREQ;
	else
		clkrate = 397 * LPC32XX_CLOCK_OSC_FREQ;

	/*                                                          */
	pllreg = __raw_readl(LPC32XX_CLKPWR_HCLKPLL_CTRL) & 0x1FFFF;
	clkrate = clk_get_pllrate_from_reg(clkrate, pllreg);

	/*                                                                   */
	clkrate = clkrate / clk_get_pclk_div();

	/*                     */
	__raw_writel(0, LPC32XX_TIMER_TCR(LPC32XX_TIMER0_BASE));
	__raw_writel(LPC32XX_TIMER_CNTR_MTCH_BIT(0),
		LPC32XX_TIMER_IR(LPC32XX_TIMER0_BASE));
	__raw_writel(1, LPC32XX_TIMER_MR0(LPC32XX_TIMER0_BASE));
	__raw_writel(LPC32XX_TIMER_CNTR_MCR_MTCH(0) |
		LPC32XX_TIMER_CNTR_MCR_STOP(0) |
		LPC32XX_TIMER_CNTR_MCR_RESET(0),
		LPC32XX_TIMER_MCR(LPC32XX_TIMER0_BASE));

	/*                      */
	setup_irq(IRQ_LPC32XX_TIMER0, &lpc32xx_timer_irq);

	/*                                 */
	lpc32xx_clkevt.cpumask = cpumask_of(0);
	clockevents_config_and_register(&lpc32xx_clkevt, clkrate, 1, -1);

	/*                             */
	__raw_writel(LPC32XX_TIMER_CNTR_TCR_RESET,
		LPC32XX_TIMER_TCR(LPC32XX_TIMER1_BASE));
	__raw_writel(0, LPC32XX_TIMER_PR(LPC32XX_TIMER1_BASE));
	__raw_writel(0, LPC32XX_TIMER_MCR(LPC32XX_TIMER1_BASE));
	__raw_writel(LPC32XX_TIMER_CNTR_TCR_EN,
		LPC32XX_TIMER_TCR(LPC32XX_TIMER1_BASE));

	clocksource_mmio_init(LPC32XX_TIMER_TC(LPC32XX_TIMER1_BASE),
		"lpc32xx_clksrc", clkrate, 300, 32, clocksource_mmio_readl_up);
}
