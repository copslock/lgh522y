/*
 *
 * arch/arm/mach-u300/timer.c
 *
 *
 * Copyright (C) 2007-2009 ST-Ericsson AB
 * License terms: GNU General Public License (GPL) version 2
 * Timer COH 901 328, runs the OS timer interrupt.
 * Author: Linus Walleij <linus.walleij@stericsson.com>
 */
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/timex.h>
#include <linux/clockchips.h>
#include <linux/clocksource.h>
#include <linux/types.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/irq.h>

#include <mach/hardware.h>
#include <mach/irqs.h>

/*               */
#include <asm/sched_clock.h>
#include <asm/mach/map.h>
#include <asm/mach/time.h>

#include "timer.h"

/*
                                   
                                                                    
                                         
                                   
                                       
                                       
 */

/*                            */
#define U300_TIMER_APP_ROST					(0x0000)
#define U300_TIMER_APP_ROST_TIMER_RESET				(0x00000000)
/*                             */
#define U300_TIMER_APP_EOST					(0x0004)
#define U300_TIMER_APP_EOST_TIMER_ENABLE			(0x00000000)
/*                              */
#define U300_TIMER_APP_DOST					(0x0008)
#define U300_TIMER_APP_DOST_TIMER_DISABLE			(0x00000000)
/*                                    */
#define U300_TIMER_APP_SOSTM					(0x000c)
#define U300_TIMER_APP_SOSTM_MODE_CONTINUOUS			(0x00000000)
#define U300_TIMER_APP_SOSTM_MODE_ONE_SHOT			(0x00000001)
/*                                      */
#define U300_TIMER_APP_OSTS					(0x0010)
#define U300_TIMER_APP_OSTS_TIMER_STATE_MASK			(0x0000000F)
#define U300_TIMER_APP_OSTS_TIMER_STATE_IDLE			(0x00000001)
#define U300_TIMER_APP_OSTS_TIMER_STATE_ACTIVE			(0x00000002)
#define U300_TIMER_APP_OSTS_ENABLE_IND				(0x00000010)
#define U300_TIMER_APP_OSTS_MODE_MASK				(0x00000020)
#define U300_TIMER_APP_OSTS_MODE_CONTINUOUS			(0x00000000)
#define U300_TIMER_APP_OSTS_MODE_ONE_SHOT			(0x00000020)
#define U300_TIMER_APP_OSTS_IRQ_ENABLED_IND			(0x00000040)
#define U300_TIMER_APP_OSTS_IRQ_PENDING_IND			(0x00000080)
/*                                             */
#define U300_TIMER_APP_OSTCC					(0x0014)
/*                                              */
#define U300_TIMER_APP_OSTTC					(0x0018)
/*                                                */
#define U300_TIMER_APP_OSTIE					(0x001c)
#define U300_TIMER_APP_OSTIE_IRQ_DISABLE			(0x00000000)
#define U300_TIMER_APP_OSTIE_IRQ_ENABLE				(0x00000001)
/*                                                     */
#define U300_TIMER_APP_OSTIA					(0x0020)
#define U300_TIMER_APP_OSTIA_IRQ_ACK				(0x00000080)

/*                            */
#define U300_TIMER_APP_RDDT					(0x0040)
#define U300_TIMER_APP_RDDT_TIMER_RESET				(0x00000000)
/*                             */
#define U300_TIMER_APP_EDDT					(0x0044)
#define U300_TIMER_APP_EDDT_TIMER_ENABLE			(0x00000000)
/*                              */
#define U300_TIMER_APP_DDDT					(0x0048)
#define U300_TIMER_APP_DDDT_TIMER_DISABLE			(0x00000000)
/*                                    */
#define U300_TIMER_APP_SDDTM					(0x004c)
#define U300_TIMER_APP_SDDTM_MODE_CONTINUOUS			(0x00000000)
#define U300_TIMER_APP_SDDTM_MODE_ONE_SHOT			(0x00000001)
/*                                      */
#define U300_TIMER_APP_DDTS					(0x0050)
#define U300_TIMER_APP_DDTS_TIMER_STATE_MASK			(0x0000000F)
#define U300_TIMER_APP_DDTS_TIMER_STATE_IDLE			(0x00000001)
#define U300_TIMER_APP_DDTS_TIMER_STATE_ACTIVE			(0x00000002)
#define U300_TIMER_APP_DDTS_ENABLE_IND				(0x00000010)
#define U300_TIMER_APP_DDTS_MODE_MASK				(0x00000020)
#define U300_TIMER_APP_DDTS_MODE_CONTINUOUS			(0x00000000)
#define U300_TIMER_APP_DDTS_MODE_ONE_SHOT			(0x00000020)
#define U300_TIMER_APP_DDTS_IRQ_ENABLED_IND			(0x00000040)
#define U300_TIMER_APP_DDTS_IRQ_PENDING_IND			(0x00000080)
/*                                             */
#define U300_TIMER_APP_DDTCC					(0x0054)
/*                                              */
#define U300_TIMER_APP_DDTTC					(0x0058)
/*                                                */
#define U300_TIMER_APP_DDTIE					(0x005c)
#define U300_TIMER_APP_DDTIE_IRQ_DISABLE			(0x00000000)
#define U300_TIMER_APP_DDTIE_IRQ_ENABLE				(0x00000001)
/*                                                     */
#define U300_TIMER_APP_DDTIA					(0x0060)
#define U300_TIMER_APP_DDTIA_IRQ_ACK				(0x00000080)

/*                             */
#define U300_TIMER_APP_RGPT1					(0x0080)
#define U300_TIMER_APP_RGPT1_TIMER_RESET			(0x00000000)
/*                              */
#define U300_TIMER_APP_EGPT1					(0x0084)
#define U300_TIMER_APP_EGPT1_TIMER_ENABLE			(0x00000000)
/*                               */
#define U300_TIMER_APP_DGPT1					(0x0088)
#define U300_TIMER_APP_DGPT1_TIMER_DISABLE			(0x00000000)
/*                                     */
#define U300_TIMER_APP_SGPT1M					(0x008c)
#define U300_TIMER_APP_SGPT1M_MODE_CONTINUOUS			(0x00000000)
#define U300_TIMER_APP_SGPT1M_MODE_ONE_SHOT			(0x00000001)
/*                                       */
#define U300_TIMER_APP_GPT1S					(0x0090)
#define U300_TIMER_APP_GPT1S_TIMER_STATE_MASK			(0x0000000F)
#define U300_TIMER_APP_GPT1S_TIMER_STATE_IDLE			(0x00000001)
#define U300_TIMER_APP_GPT1S_TIMER_STATE_ACTIVE			(0x00000002)
#define U300_TIMER_APP_GPT1S_ENABLE_IND				(0x00000010)
#define U300_TIMER_APP_GPT1S_MODE_MASK				(0x00000020)
#define U300_TIMER_APP_GPT1S_MODE_CONTINUOUS			(0x00000000)
#define U300_TIMER_APP_GPT1S_MODE_ONE_SHOT			(0x00000020)
#define U300_TIMER_APP_GPT1S_IRQ_ENABLED_IND			(0x00000040)
#define U300_TIMER_APP_GPT1S_IRQ_PENDING_IND			(0x00000080)
/*                                              */
#define U300_TIMER_APP_GPT1CC					(0x0094)
/*                                               */
#define U300_TIMER_APP_GPT1TC					(0x0098)
/*                                                 */
#define U300_TIMER_APP_GPT1IE					(0x009c)
#define U300_TIMER_APP_GPT1IE_IRQ_DISABLE			(0x00000000)
#define U300_TIMER_APP_GPT1IE_IRQ_ENABLE			(0x00000001)
/*                                                      */
#define U300_TIMER_APP_GPT1IA					(0x00a0)
#define U300_TIMER_APP_GPT1IA_IRQ_ACK				(0x00000080)

/*                             */
#define U300_TIMER_APP_RGPT2					(0x00c0)
#define U300_TIMER_APP_RGPT2_TIMER_RESET			(0x00000000)
/*                              */
#define U300_TIMER_APP_EGPT2					(0x00c4)
#define U300_TIMER_APP_EGPT2_TIMER_ENABLE			(0x00000000)
/*                               */
#define U300_TIMER_APP_DGPT2					(0x00c8)
#define U300_TIMER_APP_DGPT2_TIMER_DISABLE			(0x00000000)
/*                                     */
#define U300_TIMER_APP_SGPT2M					(0x00cc)
#define U300_TIMER_APP_SGPT2M_MODE_CONTINUOUS			(0x00000000)
#define U300_TIMER_APP_SGPT2M_MODE_ONE_SHOT			(0x00000001)
/*                                       */
#define U300_TIMER_APP_GPT2S					(0x00d0)
#define U300_TIMER_APP_GPT2S_TIMER_STATE_MASK			(0x0000000F)
#define U300_TIMER_APP_GPT2S_TIMER_STATE_IDLE			(0x00000001)
#define U300_TIMER_APP_GPT2S_TIMER_STATE_ACTIVE			(0x00000002)
#define U300_TIMER_APP_GPT2S_ENABLE_IND				(0x00000010)
#define U300_TIMER_APP_GPT2S_MODE_MASK				(0x00000020)
#define U300_TIMER_APP_GPT2S_MODE_CONTINUOUS			(0x00000000)
#define U300_TIMER_APP_GPT2S_MODE_ONE_SHOT			(0x00000020)
#define U300_TIMER_APP_GPT2S_IRQ_ENABLED_IND			(0x00000040)
#define U300_TIMER_APP_GPT2S_IRQ_PENDING_IND			(0x00000080)
/*                                              */
#define U300_TIMER_APP_GPT2CC					(0x00d4)
/*                                               */
#define U300_TIMER_APP_GPT2TC					(0x00d8)
/*                                                 */
#define U300_TIMER_APP_GPT2IE					(0x00dc)
#define U300_TIMER_APP_GPT2IE_IRQ_DISABLE			(0x00000000)
#define U300_TIMER_APP_GPT2IE_IRQ_ENABLE			(0x00000001)
/*                                                      */
#define U300_TIMER_APP_GPT2IA					(0x00e0)
#define U300_TIMER_APP_GPT2IA_IRQ_ACK				(0x00000080)

/*                                                  */
#define U300_TIMER_APP_CRC					(0x100)
#define U300_TIMER_APP_CRC_CLOCK_REQUEST_ENABLE			(0x00000001)

#define TICKS_PER_JIFFY ((CLOCK_TICK_RATE + (HZ/2)) / HZ)
#define US_PER_TICK ((1000000 + (HZ/2)) / HZ)

/*
                                                             
                                                             
                                                     
 */
static void u300_set_mode(enum clock_event_mode mode,
			  struct clock_event_device *evt)
{
	switch (mode) {
	case CLOCK_EVT_MODE_PERIODIC:
		/*                            */
		writel(U300_TIMER_APP_GPT1IE_IRQ_DISABLE,
		       U300_TIMER_APP_VBASE + U300_TIMER_APP_GPT1IE);
		/*                                           */
		writel(U300_TIMER_APP_DGPT1_TIMER_DISABLE,
		       U300_TIMER_APP_VBASE + U300_TIMER_APP_DGPT1);
		/*
                                                           
           
   */
		writel(TICKS_PER_JIFFY,
		       U300_TIMER_APP_VBASE + U300_TIMER_APP_GPT1TC);
		/*
                                                       
                
   */
		writel(U300_TIMER_APP_SGPT1M_MODE_CONTINUOUS,
		       U300_TIMER_APP_VBASE + U300_TIMER_APP_SGPT1M);
		/*                         */
		writel(U300_TIMER_APP_GPT1IE_IRQ_ENABLE,
		       U300_TIMER_APP_VBASE + U300_TIMER_APP_GPT1IE);
		/*                                */
		writel(U300_TIMER_APP_EGPT1_TIMER_ENABLE,
		       U300_TIMER_APP_VBASE + U300_TIMER_APP_EGPT1);
		break;
	case CLOCK_EVT_MODE_ONESHOT:
		/*                   */
		/*
                                                                
                                                             
                   
   */
		/*                            */
		writel(U300_TIMER_APP_GPT1IE_IRQ_DISABLE,
		       U300_TIMER_APP_VBASE + U300_TIMER_APP_GPT1IE);
		/*                                           */
		writel(U300_TIMER_APP_DGPT1_TIMER_DISABLE,
		       U300_TIMER_APP_VBASE + U300_TIMER_APP_DGPT1);
		/*
                                                            
                   
   */
		writel(0xFFFFFFFF, U300_TIMER_APP_VBASE + U300_TIMER_APP_GPT1TC);
		/*                                */
		writel(U300_TIMER_APP_SGPT1M_MODE_ONE_SHOT,
		       U300_TIMER_APP_VBASE + U300_TIMER_APP_SGPT1M);
		/*                                  */
		writel(U300_TIMER_APP_GPT1IE_IRQ_ENABLE,
		       U300_TIMER_APP_VBASE + U300_TIMER_APP_GPT1IE);
		/*              */
		writel(U300_TIMER_APP_EGPT1_TIMER_ENABLE,
		       U300_TIMER_APP_VBASE + U300_TIMER_APP_EGPT1);
		break;
	case CLOCK_EVT_MODE_UNUSED:
	case CLOCK_EVT_MODE_SHUTDOWN:
		/*                           */
		writel(U300_TIMER_APP_GPT1IE_IRQ_DISABLE,
		       U300_TIMER_APP_VBASE + U300_TIMER_APP_GPT1IE);
		/*             */
		writel(U300_TIMER_APP_DGPT1_TIMER_DISABLE,
		       U300_TIMER_APP_VBASE + U300_TIMER_APP_DGPT1);
		break;
	case CLOCK_EVT_MODE_RESUME:
		/*                  */
		break;
	}
}

/*
                                                                  
                                                                        
                                                                       
                                                                     
                                                                         
                                           
 */
static int u300_set_next_event(unsigned long cycles,
			       struct clock_event_device *evt)

{
	/*                            */
	writel(U300_TIMER_APP_GPT1IE_IRQ_DISABLE,
	       U300_TIMER_APP_VBASE + U300_TIMER_APP_GPT1IE);
	/*                                           */
	writel(U300_TIMER_APP_DGPT1_TIMER_DISABLE,
	       U300_TIMER_APP_VBASE + U300_TIMER_APP_DGPT1);
	/*                                    */
	writel(U300_TIMER_APP_RGPT1_TIMER_RESET,
	       U300_TIMER_APP_VBASE + U300_TIMER_APP_RGPT1);
	/*                   */
	writel(cycles, U300_TIMER_APP_VBASE + U300_TIMER_APP_GPT1TC);
	/*
                                                                     
                                      
  */
	writel(U300_TIMER_APP_SGPT1M_MODE_ONE_SHOT,
	       U300_TIMER_APP_VBASE + U300_TIMER_APP_SGPT1M);
	/*                         */
	writel(U300_TIMER_APP_GPT1IE_IRQ_ENABLE,
	       U300_TIMER_APP_VBASE + U300_TIMER_APP_GPT1IE);
	/*                                */
	writel(U300_TIMER_APP_EGPT1_TIMER_ENABLE,
	       U300_TIMER_APP_VBASE + U300_TIMER_APP_EGPT1);
	return 0;
}


/*                                            */
static struct clock_event_device clockevent_u300_1mhz = {
	.name		= "GPT1",
	.rating		= 300, /*                                          */
	.features	= CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT,
	.set_next_event	= u300_set_next_event,
	.set_mode	= u300_set_mode,
};

/*                                     */
static irqreturn_t u300_timer_interrupt(int irq, void *dev_id)
{
	struct clock_event_device *evt = &clockevent_u300_1mhz;
	/*                                            */
	writel(U300_TIMER_APP_GPT1IA_IRQ_ACK,
		U300_TIMER_APP_VBASE + U300_TIMER_APP_GPT1IA);
	evt->event_handler(evt);
	return IRQ_HANDLED;
}

static struct irqaction u300_timer_irq = {
	.name		= "U300 Timer Tick",
	.flags		= IRQF_DISABLED | IRQF_TIMER | IRQF_IRQPOLL,
	.handler	= u300_timer_interrupt,
};

/*
                                                        
                                                              
                                                               
                                                              
                                            
 */

static u32 notrace u300_read_sched_clock(void)
{
	return readl(U300_TIMER_APP_VBASE + U300_TIMER_APP_GPT2CC);
}


/*
                                                                
 */
void __init u300_timer_init(void)
{
	struct clk *clk;
	unsigned long rate;

	/*                                */
	clk = clk_get_sys("apptimer", NULL);
	BUG_ON(IS_ERR(clk));
	clk_prepare_enable(clk);
	rate = clk_get_rate(clk);

	setup_sched_clock(u300_read_sched_clock, 32, rate);

	/*
                                                                      
                                                            
  */
	writel(U300_TIMER_APP_CRC_CLOCK_REQUEST_ENABLE,
		U300_TIMER_APP_VBASE + U300_TIMER_APP_CRC);
	writel(U300_TIMER_APP_ROST_TIMER_RESET,
		U300_TIMER_APP_VBASE + U300_TIMER_APP_ROST);
	writel(U300_TIMER_APP_DOST_TIMER_DISABLE,
		U300_TIMER_APP_VBASE + U300_TIMER_APP_DOST);
	writel(U300_TIMER_APP_RDDT_TIMER_RESET,
		U300_TIMER_APP_VBASE + U300_TIMER_APP_RDDT);
	writel(U300_TIMER_APP_DDDT_TIMER_DISABLE,
		U300_TIMER_APP_VBASE + U300_TIMER_APP_DDDT);

	/*                                    */
	writel(U300_TIMER_APP_RGPT1_TIMER_RESET,
		U300_TIMER_APP_VBASE + U300_TIMER_APP_RGPT1);

	/*                        */
	setup_irq(IRQ_U300_TIMER_APP_GP1, &u300_timer_irq);

	/*                                   */
	writel(U300_TIMER_APP_RGPT2_TIMER_RESET,
		U300_TIMER_APP_VBASE + U300_TIMER_APP_RGPT2);
	/*                                      */
	writel(0xFFFFFFFFU, U300_TIMER_APP_VBASE + U300_TIMER_APP_GPT2TC);
	/*                                        */
	writel(U300_TIMER_APP_SGPT2M_MODE_CONTINUOUS,
	       U300_TIMER_APP_VBASE + U300_TIMER_APP_SGPT2M);
	/*                          */
	writel(U300_TIMER_APP_GPT2IE_IRQ_DISABLE,
		U300_TIMER_APP_VBASE + U300_TIMER_APP_GPT2IE);
	/*                                                               */
	writel(U300_TIMER_APP_EGPT2_TIMER_ENABLE,
		U300_TIMER_APP_VBASE + U300_TIMER_APP_EGPT2);

	/*                                             */
	if (clocksource_mmio_init(U300_TIMER_APP_VBASE + U300_TIMER_APP_GPT2CC,
			"GPT2", rate, 300, 32, clocksource_mmio_readl_up))
		pr_err("timer: failed to initialize U300 clock source\n");

	/*                                       */
	clockevents_config_and_register(&clockevent_u300_1mhz, rate,
					1, 0xffffffff);

	/*
                                                                   
                     
  */
}
