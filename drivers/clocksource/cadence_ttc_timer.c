/*
 * This file contains driver for the Cadence Triple Timer Counter Rev 06
 *
 *  Copyright (C) 2011-2013 Xilinx
 *
 * based on arch/mips/kernel/time.c timer driver
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/clk.h>
#include <linux/interrupt.h>
#include <linux/clockchips.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/slab.h>
#include <linux/clk-provider.h>

/*
                                                                  
  
                                                   
                                              
                        
  
                                                                           
                                                                              
                                                            

                                                                         
                                                           
 */

/*
                                                                            
                                   
 */
#define TTC_CLK_CNTRL_OFFSET		0x00 /*                       */
#define TTC_CNT_CNTRL_OFFSET		0x0C /*                         */
#define TTC_COUNT_VAL_OFFSET		0x18 /*                       */
#define TTC_INTR_VAL_OFFSET		0x24 /*                        */
#define TTC_ISR_OFFSET		0x54 /*                          */
#define TTC_IER_OFFSET		0x60 /*                          */

#define TTC_CNT_CNTRL_DISABLE_MASK	0x1

/*
                                                                             
                                                                       
 */
#define PRESCALE_EXPONENT	11	/*                                  */
#define PRESCALE		2048	/*                              */
#define CLK_CNTRL_PRESCALE	((PRESCALE_EXPONENT - 1) << 1)
#define CLK_CNTRL_PRESCALE_EN	1
#define CNT_CNTRL_RESET		(1 << 4)

/* 
                                                                   
  
                                    
                                
                                                            
 */
struct ttc_timer {
	void __iomem *base_addr;
	struct clk *clk;
	struct notifier_block clk_rate_change_nb;
};

#define to_ttc_timer(x) \
		container_of(x, struct ttc_timer, clk_rate_change_nb)

struct ttc_timer_clocksource {
	struct ttc_timer	ttc;
	struct clocksource	cs;
};

#define to_ttc_timer_clksrc(x) \
		container_of(x, struct ttc_timer_clocksource, cs)

struct ttc_timer_clockevent {
	struct ttc_timer		ttc;
	struct clock_event_device	ce;
};

#define to_ttc_timer_clkevent(x) \
		container_of(x, struct ttc_timer_clockevent, ce)

/* 
                                                  
  
                                        
                                
  */
static void ttc_set_interval(struct ttc_timer *timer,
					unsigned long cycles)
{
	u32 ctrl_reg;

	/*                                                                   */
	ctrl_reg = __raw_readl(timer->base_addr + TTC_CNT_CNTRL_OFFSET);
	ctrl_reg |= TTC_CNT_CNTRL_DISABLE_MASK;
	__raw_writel(ctrl_reg, timer->base_addr + TTC_CNT_CNTRL_OFFSET);

	__raw_writel(cycles, timer->base_addr + TTC_INTR_VAL_OFFSET);

	/*
                                                               
                                                  
  */
	ctrl_reg |= CNT_CNTRL_RESET;
	ctrl_reg &= ~TTC_CNT_CNTRL_DISABLE_MASK;
	__raw_writel(ctrl_reg, timer->base_addr + TTC_CNT_CNTRL_OFFSET);
}

/* 
                                                                  
  
                                
                                                  
  
                                        
  */
static irqreturn_t ttc_clock_event_interrupt(int irq, void *dev_id)
{
	struct ttc_timer_clockevent *ttce = dev_id;
	struct ttc_timer *timer = &ttce->ttc;

	/*                                                  */
	__raw_readl(timer->base_addr + TTC_ISR_OFFSET);

	ttce->ce.event_handler(&ttce->ce);

	return IRQ_HANDLED;
}

/* 
                                                            
  
                                                
  */
static cycle_t __ttc_clocksource_read(struct clocksource *cs)
{
	struct ttc_timer *timer = &to_ttc_timer_clksrc(cs)->ttc;

	return (cycle_t)__raw_readl(timer->base_addr +
				TTC_COUNT_VAL_OFFSET);
}

/* 
                                                             
  
                                
                                        
  
                              
  */
static int ttc_set_next_event(unsigned long cycles,
					struct clock_event_device *evt)
{
	struct ttc_timer_clockevent *ttce = to_ttc_timer_clkevent(evt);
	struct ttc_timer *timer = &ttce->ttc;

	ttc_set_interval(timer, cycles);
	return 0;
}

/* 
                                        
  
                        
                                        
  */
static void ttc_set_mode(enum clock_event_mode mode,
					struct clock_event_device *evt)
{
	struct ttc_timer_clockevent *ttce = to_ttc_timer_clkevent(evt);
	struct ttc_timer *timer = &ttce->ttc;
	u32 ctrl_reg;

	switch (mode) {
	case CLOCK_EVT_MODE_PERIODIC:
		ttc_set_interval(timer,
				DIV_ROUND_CLOSEST(clk_get_rate(ttce->ttc.clk),
					PRESCALE * HZ));
		break;
	case CLOCK_EVT_MODE_ONESHOT:
	case CLOCK_EVT_MODE_UNUSED:
	case CLOCK_EVT_MODE_SHUTDOWN:
		ctrl_reg = __raw_readl(timer->base_addr +
					TTC_CNT_CNTRL_OFFSET);
		ctrl_reg |= TTC_CNT_CNTRL_DISABLE_MASK;
		__raw_writel(ctrl_reg,
				timer->base_addr + TTC_CNT_CNTRL_OFFSET);
		break;
	case CLOCK_EVT_MODE_RESUME:
		ctrl_reg = __raw_readl(timer->base_addr +
					TTC_CNT_CNTRL_OFFSET);
		ctrl_reg &= ~TTC_CNT_CNTRL_DISABLE_MASK;
		__raw_writel(ctrl_reg,
				timer->base_addr + TTC_CNT_CNTRL_OFFSET);
		break;
	}
}

static int ttc_rate_change_clocksource_cb(struct notifier_block *nb,
		unsigned long event, void *data)
{
	struct clk_notifier_data *ndata = data;
	struct ttc_timer *ttc = to_ttc_timer(nb);
	struct ttc_timer_clocksource *ttccs = container_of(ttc,
			struct ttc_timer_clocksource, ttc);

	switch (event) {
	case POST_RATE_CHANGE:
		/*
                                                            
    
                                                                 
                                                               
                                                           
    
                                                                
                                                           
                                                                  
    
                                                                 
                                                                
                                                                 
                                                     
   */
		clocksource_unregister(&ttccs->cs);
		clocksource_register_hz(&ttccs->cs,
				ndata->new_rate / PRESCALE);
		/*              */
	case PRE_RATE_CHANGE:
	case ABORT_RATE_CHANGE:
	default:
		return NOTIFY_DONE;
	}
}

static void __init ttc_setup_clocksource(struct clk *clk, void __iomem *base)
{
	struct ttc_timer_clocksource *ttccs;
	int err;

	ttccs = kzalloc(sizeof(*ttccs), GFP_KERNEL);
	if (WARN_ON(!ttccs))
		return;

	ttccs->ttc.clk = clk;

	err = clk_prepare_enable(ttccs->ttc.clk);
	if (WARN_ON(err)) {
		kfree(ttccs);
		return;
	}

	ttccs->ttc.clk_rate_change_nb.notifier_call =
		ttc_rate_change_clocksource_cb;
	ttccs->ttc.clk_rate_change_nb.next = NULL;
	if (clk_notifier_register(ttccs->ttc.clk,
				&ttccs->ttc.clk_rate_change_nb))
		pr_warn("Unable to register clock notifier.\n");

	ttccs->ttc.base_addr = base;
	ttccs->cs.name = "ttc_clocksource";
	ttccs->cs.rating = 200;
	ttccs->cs.read = __ttc_clocksource_read;
	ttccs->cs.mask = CLOCKSOURCE_MASK(16);
	ttccs->cs.flags = CLOCK_SOURCE_IS_CONTINUOUS;

	/*
                                                                
                                                            
                                            
  */
	__raw_writel(0x0,  ttccs->ttc.base_addr + TTC_IER_OFFSET);
	__raw_writel(CLK_CNTRL_PRESCALE | CLK_CNTRL_PRESCALE_EN,
		     ttccs->ttc.base_addr + TTC_CLK_CNTRL_OFFSET);
	__raw_writel(CNT_CNTRL_RESET,
		     ttccs->ttc.base_addr + TTC_CNT_CNTRL_OFFSET);

	err = clocksource_register_hz(&ttccs->cs,
			clk_get_rate(ttccs->ttc.clk) / PRESCALE);
	if (WARN_ON(err)) {
		kfree(ttccs);
		return;
	}
}

static int ttc_rate_change_clockevent_cb(struct notifier_block *nb,
		unsigned long event, void *data)
{
	struct clk_notifier_data *ndata = data;
	struct ttc_timer *ttc = to_ttc_timer(nb);
	struct ttc_timer_clockevent *ttcce = container_of(ttc,
			struct ttc_timer_clockevent, ttc);

	switch (event) {
	case POST_RATE_CHANGE:
	{
		unsigned long flags;

		/*
                                                                  
                                                               
                                                            
           
   */
		local_irq_save(flags);
		clockevents_update_freq(&ttcce->ce,
				ndata->new_rate / PRESCALE);
		local_irq_restore(flags);

		/*              */
	}
	case PRE_RATE_CHANGE:
	case ABORT_RATE_CHANGE:
	default:
		return NOTIFY_DONE;
	}
}

static void __init ttc_setup_clockevent(struct clk *clk,
						void __iomem *base, u32 irq)
{
	struct ttc_timer_clockevent *ttcce;
	int err;

	ttcce = kzalloc(sizeof(*ttcce), GFP_KERNEL);
	if (WARN_ON(!ttcce))
		return;

	ttcce->ttc.clk = clk;

	err = clk_prepare_enable(ttcce->ttc.clk);
	if (WARN_ON(err)) {
		kfree(ttcce);
		return;
	}

	ttcce->ttc.clk_rate_change_nb.notifier_call =
		ttc_rate_change_clockevent_cb;
	ttcce->ttc.clk_rate_change_nb.next = NULL;
	if (clk_notifier_register(ttcce->ttc.clk,
				&ttcce->ttc.clk_rate_change_nb))
		pr_warn("Unable to register clock notifier.\n");

	ttcce->ttc.base_addr = base;
	ttcce->ce.name = "ttc_clockevent";
	ttcce->ce.features = CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT;
	ttcce->ce.set_next_event = ttc_set_next_event;
	ttcce->ce.set_mode = ttc_set_mode;
	ttcce->ce.rating = 200;
	ttcce->ce.irq = irq;
	ttcce->ce.cpumask = cpu_possible_mask;

	/*
                                                             
                                                             
                     
  */
	__raw_writel(0x23, ttcce->ttc.base_addr + TTC_CNT_CNTRL_OFFSET);
	__raw_writel(CLK_CNTRL_PRESCALE | CLK_CNTRL_PRESCALE_EN,
		     ttcce->ttc.base_addr + TTC_CLK_CNTRL_OFFSET);
	__raw_writel(0x1,  ttcce->ttc.base_addr + TTC_IER_OFFSET);

	err = request_irq(irq, ttc_clock_event_interrupt,
			  IRQF_DISABLED | IRQF_TIMER,
			  ttcce->ce.name, ttcce);
	if (WARN_ON(err)) {
		kfree(ttcce);
		return;
	}

	clockevents_config_and_register(&ttcce->ce,
			clk_get_rate(ttcce->ttc.clk) / PRESCALE, 1, 0xfffe);
}

/* 
                                        
  
                                                                               
                                           
 */
static void __init ttc_timer_init(struct device_node *timer)
{
	unsigned int irq;
	void __iomem *timer_baseaddr;
	struct clk *clk;
	static int initialized;

	if (initialized)
		return;

	initialized = 1;

	/*
                                                                     
                                                                         
                                              
  */
	timer_baseaddr = of_iomap(timer, 0);
	if (!timer_baseaddr) {
		pr_err("ERROR: invalid timer base address\n");
		BUG();
	}

	irq = irq_of_parse_and_map(timer, 1);
	if (irq <= 0) {
		pr_err("ERROR: invalid interrupt number\n");
		BUG();
	}

	clk = of_clk_get_by_name(timer, "cpu_1x");
	if (IS_ERR(clk)) {
		pr_err("ERROR: timer input clock not found\n");
		BUG();
	}

	ttc_setup_clocksource(clk, timer_baseaddr);
	ttc_setup_clockevent(clk, timer_baseaddr + 4, irq);

	pr_info("%s #0 at %p, irq=%d\n", timer->name, timer_baseaddr, irq);
}

CLOCKSOURCE_OF_DECLARE(ttc, "cdns,ttc", ttc_timer_init);
