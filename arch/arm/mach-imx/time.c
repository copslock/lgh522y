/*
 *  linux/arch/arm/plat-mxc/time.c
 *
 *  Copyright (C) 2000-2001 Deep Blue Solutions
 *  Copyright (C) 2002 Shane Nay (shane@minirl.com)
 *  Copyright (C) 2006-2007 Pavel Pisa (ppisa@pikron.com)
 *  Copyright (C) 2008 Juergen Beisert (kernel@pengutronix.de)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/clockchips.h>
#include <linux/clk.h>
#include <linux/err.h>

#include <asm/sched_clock.h>
#include <asm/mach/time.h>

#include "common.h"
#include "hardware.h"

/*
                                                                        
                                  
                                          
 */

/*                             */
#define MXC_TCTL		0x00
#define MXC_TCTL_TEN		(1 << 0) /*               */
#define MXC_TPRER		0x04

/*                 */
#define MX1_2_TCTL_CLK_PCLK1	(1 << 1)
#define MX1_2_TCTL_IRQEN	(1 << 4)
#define MX1_2_TCTL_FRR		(1 << 8)
#define MX1_2_TCMP		0x08
#define MX1_2_TCN		0x10
#define MX1_2_TSTAT		0x14

/*            */
#define MX2_TSTAT_CAPT		(1 << 1)
#define MX2_TSTAT_COMP		(1 << 0)

/*                       */
#define V2_TCTL_WAITEN		(1 << 3) /*                  */
#define V2_TCTL_CLK_IPG		(1 << 6)
#define V2_TCTL_CLK_PER		(2 << 6)
#define V2_TCTL_FRR		(1 << 9)
#define V2_IR			0x0c
#define V2_TSTAT		0x08
#define V2_TSTAT_OF1		(1 << 0)
#define V2_TCN			0x24
#define V2_TCMP			0x10

#define timer_is_v1()	(cpu_is_mx1() || cpu_is_mx21() || cpu_is_mx27())
#define timer_is_v2()	(!timer_is_v1())

static struct clock_event_device clockevent_mxc;
static enum clock_event_mode clockevent_mode = CLOCK_EVT_MODE_UNUSED;

static void __iomem *timer_base;

static inline void gpt_irq_disable(void)
{
	unsigned int tmp;

	if (timer_is_v2())
		__raw_writel(0, timer_base + V2_IR);
	else {
		tmp = __raw_readl(timer_base + MXC_TCTL);
		__raw_writel(tmp & ~MX1_2_TCTL_IRQEN, timer_base + MXC_TCTL);
	}
}

static inline void gpt_irq_enable(void)
{
	if (timer_is_v2())
		__raw_writel(1<<0, timer_base + V2_IR);
	else {
		__raw_writel(__raw_readl(timer_base + MXC_TCTL) | MX1_2_TCTL_IRQEN,
			timer_base + MXC_TCTL);
	}
}

static void gpt_irq_acknowledge(void)
{
	if (timer_is_v1()) {
		if (cpu_is_mx1())
			__raw_writel(0, timer_base + MX1_2_TSTAT);
		else
			__raw_writel(MX2_TSTAT_CAPT | MX2_TSTAT_COMP,
				timer_base + MX1_2_TSTAT);
	} else if (timer_is_v2())
		__raw_writel(V2_TSTAT_OF1, timer_base + V2_TSTAT);
}

static void __iomem *sched_clock_reg;

static u32 notrace mxc_read_sched_clock(void)
{
	return sched_clock_reg ? __raw_readl(sched_clock_reg) : 0;
}

static int __init mxc_clocksource_init(struct clk *timer_clk)
{
	unsigned int c = clk_get_rate(timer_clk);
	void __iomem *reg = timer_base + (timer_is_v2() ? V2_TCN : MX1_2_TCN);

	sched_clock_reg = reg;

	setup_sched_clock(mxc_read_sched_clock, 32, c);
	return clocksource_mmio_init(reg, "mxc_timer1", c, 200, 32,
			clocksource_mmio_readl_up);
}

/*             */

static int mx1_2_set_next_event(unsigned long evt,
			      struct clock_event_device *unused)
{
	unsigned long tcmp;

	tcmp = __raw_readl(timer_base + MX1_2_TCN) + evt;

	__raw_writel(tcmp, timer_base + MX1_2_TCMP);

	return (int)(tcmp - __raw_readl(timer_base + MX1_2_TCN)) < 0 ?
				-ETIME : 0;
}

static int v2_set_next_event(unsigned long evt,
			      struct clock_event_device *unused)
{
	unsigned long tcmp;

	tcmp = __raw_readl(timer_base + V2_TCN) + evt;

	__raw_writel(tcmp, timer_base + V2_TCMP);

	return evt < 0x7fffffff &&
		(int)(tcmp - __raw_readl(timer_base + V2_TCN)) < 0 ?
				-ETIME : 0;
}

#ifdef DEBUG
static const char *clock_event_mode_label[] = {
	[CLOCK_EVT_MODE_PERIODIC] = "CLOCK_EVT_MODE_PERIODIC",
	[CLOCK_EVT_MODE_ONESHOT]  = "CLOCK_EVT_MODE_ONESHOT",
	[CLOCK_EVT_MODE_SHUTDOWN] = "CLOCK_EVT_MODE_SHUTDOWN",
	[CLOCK_EVT_MODE_UNUSED]   = "CLOCK_EVT_MODE_UNUSED",
	[CLOCK_EVT_MODE_RESUME]   = "CLOCK_EVT_MODE_RESUME",
};
#endif /*       */

static void mxc_set_mode(enum clock_event_mode mode,
				struct clock_event_device *evt)
{
	unsigned long flags;

	/*
                                                       
                                                
  */
	local_irq_save(flags);

	/*                                 */
	gpt_irq_disable();

	if (mode != clockevent_mode) {
		/*                                    */
		if (timer_is_v2())
			__raw_writel(__raw_readl(timer_base + V2_TCN) - 3,
					timer_base + V2_TCMP);
		else
			__raw_writel(__raw_readl(timer_base + MX1_2_TCN) - 3,
					timer_base + MX1_2_TCMP);

		/*                         */
		gpt_irq_acknowledge();
	}

#ifdef DEBUG
	printk(KERN_INFO "mxc_set_mode: changing mode from %s to %s\n",
		clock_event_mode_label[clockevent_mode],
		clock_event_mode_label[mode]);
#endif /*       */

	/*                     */
	clockevent_mode = mode;
	local_irq_restore(flags);

	switch (mode) {
	case CLOCK_EVT_MODE_PERIODIC:
		printk(KERN_ERR"mxc_set_mode: Periodic mode is not "
				"supported for i.MX\n");
		break;
	case CLOCK_EVT_MODE_ONESHOT:
	/*
                                                        
                                                      
                                                        
                  
  */
		local_irq_save(flags);
		gpt_irq_enable();
		local_irq_restore(flags);
		break;
	case CLOCK_EVT_MODE_SHUTDOWN:
	case CLOCK_EVT_MODE_UNUSED:
	case CLOCK_EVT_MODE_RESUME:
		/*                                                        */
		break;
	}
}

/*
                            
 */
static irqreturn_t mxc_timer_interrupt(int irq, void *dev_id)
{
	struct clock_event_device *evt = &clockevent_mxc;
	uint32_t tstat;

	if (timer_is_v2())
		tstat = __raw_readl(timer_base + V2_TSTAT);
	else
		tstat = __raw_readl(timer_base + MX1_2_TSTAT);

	gpt_irq_acknowledge();

	evt->event_handler(evt);

	return IRQ_HANDLED;
}

static struct irqaction mxc_timer_irq = {
	.name		= "i.MX Timer Tick",
	.flags		= IRQF_DISABLED | IRQF_TIMER | IRQF_IRQPOLL,
	.handler	= mxc_timer_interrupt,
};

static struct clock_event_device clockevent_mxc = {
	.name		= "mxc_timer1",
	.features	= CLOCK_EVT_FEAT_ONESHOT,
	.set_mode	= mxc_set_mode,
	.set_next_event	= mx1_2_set_next_event,
	.rating		= 200,
};

static int __init mxc_clockevent_init(struct clk *timer_clk)
{
	if (timer_is_v2())
		clockevent_mxc.set_next_event = v2_set_next_event;

	clockevent_mxc.cpumask = cpumask_of(0);
	clockevents_config_and_register(&clockevent_mxc,
					clk_get_rate(timer_clk),
					0xff, 0xfffffffe);

	return 0;
}

void __init mxc_timer_init(void __iomem *base, int irq)
{
	uint32_t tctl_val;
	struct clk *timer_clk;
	struct clk *timer_ipg_clk;

	timer_clk = clk_get_sys("imx-gpt.0", "per");
	if (IS_ERR(timer_clk)) {
		pr_err("i.MX timer: unable to get clk\n");
		return;
	}

	timer_ipg_clk = clk_get_sys("imx-gpt.0", "ipg");
	if (!IS_ERR(timer_ipg_clk))
		clk_prepare_enable(timer_ipg_clk);

	clk_prepare_enable(timer_clk);

	timer_base = base;

	/*
                                                                  
  */

	__raw_writel(0, timer_base + MXC_TCTL);
	__raw_writel(0, timer_base + MXC_TPRER); /*                    */

	if (timer_is_v2())
		tctl_val = V2_TCTL_CLK_PER | V2_TCTL_FRR | V2_TCTL_WAITEN | MXC_TCTL_TEN;
	else
		tctl_val = MX1_2_TCTL_FRR | MX1_2_TCTL_CLK_PCLK1 | MXC_TCTL_TEN;

	__raw_writel(tctl_val, timer_base + MXC_TCTL);

	/*                                              */
	mxc_clocksource_init(timer_clk);
	mxc_clockevent_init(timer_clk);

	/*                  */
	setup_irq(irq, &mxc_timer_irq);
}
