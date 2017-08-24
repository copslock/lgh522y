/***********************************************************************
* linux/kernel/time/jiffies.c
*
* This file contains the jiffies based clocksource.
*
* Copyright (C) 2004, 2005 IBM, John Stultz (johnstul@us.ibm.com)
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
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
************************************************************************/
#include <linux/clocksource.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/init.h>

#include "tick-internal.h"

/*                                                   
                                                    
                                                    
                                                  
                                              
                                             
                                              
                                                 
                           
 */
#define NSEC_PER_JIFFY	((NSEC_PER_SEC+HZ/2)/HZ)

/*                                                      
                                                      
                                                         
                                                       
                                                    
                                                     
  
                                                        
                                                          
                                                            
          
 */
#if HZ < 34
#define JIFFIES_SHIFT	6
#elif HZ < 67
#define JIFFIES_SHIFT	7
#else
#define JIFFIES_SHIFT	8
#endif

static cycle_t jiffies_read(struct clocksource *cs)
{
	return (cycle_t) jiffies;
}

static struct clocksource clocksource_jiffies = {
	.name		= "jiffies",
	.rating		= 1, /*                    */
	.read		= jiffies_read,
	.mask		= 0xffffffff, /*      */
	.mult		= NSEC_PER_JIFFY << JIFFIES_SHIFT, /*               */
	.shift		= JIFFIES_SHIFT,
};

__cacheline_aligned_in_smp DEFINE_SEQLOCK(jiffies_lock);

#if (BITS_PER_LONG < 64)
u64 get_jiffies_64(void)
{
	unsigned long seq;
	u64 ret;

	do {
		seq = read_seqbegin(&jiffies_lock);
		ret = jiffies_64;
	} while (read_seqretry(&jiffies_lock, seq));
	return ret;
}
EXPORT_SYMBOL(get_jiffies_64);
#endif

EXPORT_SYMBOL(jiffies);

static int __init init_jiffies_clocksource(void)
{
	return clocksource_register(&clocksource_jiffies);
}

core_initcall(init_jiffies_clocksource);

struct clocksource * __init __weak clocksource_default_clock(void)
{
	return &clocksource_jiffies;
}

struct clocksource refined_jiffies;

int register_refined_jiffies(long cycles_per_second)
{
	u64 nsec_per_tick, shift_hz;
	long cycles_per_tick;



	refined_jiffies = clocksource_jiffies;
	refined_jiffies.name = "refined-jiffies";
	refined_jiffies.rating++;

	/*                      */
	cycles_per_tick = (cycles_per_second + HZ/2)/HZ;
	/*                                          */
	shift_hz = (u64)cycles_per_second << 8;
	shift_hz += cycles_per_tick/2;
	do_div(shift_hz, cycles_per_tick);
	/*                                        */
	nsec_per_tick = (u64)NSEC_PER_SEC << 8;
	nsec_per_tick += (u32)shift_hz/2;
	do_div(nsec_per_tick, (u32)shift_hz);

	refined_jiffies.mult = ((u32)nsec_per_tick) << JIFFIES_SHIFT;

	clocksource_register(&refined_jiffies);
	return 0;
}
