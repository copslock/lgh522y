/*
 * linux/arch/arm/mach-at91/generic.h
 *
 *  Copyright (C) 2005 David Brownell
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/clkdev.h>
#include <linux/of.h>

 /*        */
extern void __init at91_map_io(void);
extern void __init at91_init_sram(int bank, unsigned long base,
				  unsigned int length);

 /*            */
extern void __init at91rm9200_set_type(int type);
extern void __init at91_initialize(unsigned long main_clock);
extern void __init at91x40_initialize(unsigned long main_clock);
extern void __init at91rm9200_dt_initialize(void);
extern void __init at91_dt_initialize(void);

 /*            */
extern void __init at91_init_irq_default(void);
extern void __init at91_init_interrupts(unsigned int priority[]);
extern void __init at91x40_init_interrupts(unsigned int priority[]);
extern void __init at91_aic_init(unsigned int priority[],
				 unsigned int ext_irq_mask);
extern int  __init at91_aic_of_init(struct device_node *node,
				    struct device_node *parent);
extern int  __init at91_aic5_of_init(struct device_node *node,
				    struct device_node *parent);
extern void __init at91_sysirq_mask_rtc(u32 rtc_base);
extern void __init at91_sysirq_mask_rtt(u32 rtt_base);


 /*       */
extern void at91rm9200_ioremap_st(u32 addr);
extern void at91rm9200_timer_init(void);
extern void at91sam926x_ioremap_pit(u32 addr);
extern void at91sam926x_pit_init(void);
extern void at91x40_timer_init(void);

 /*        */
#ifdef CONFIG_AT91_PMC_UNIT
extern int __init at91_clock_init(unsigned long main_clock);
extern int __init at91_dt_clock_init(void);
#else
static int inline at91_clock_init(unsigned long main_clock) { return 0; }
#endif
struct device;

 /*                  */
extern void at91_irq_suspend(void);
extern void at91_irq_resume(void);

/*      */
extern void at91sam9_idle(void);

/*       */
extern void at91_ioremap_rstc(u32 base_addr);
extern void at91sam9_alt_restart(char, const char *);
extern void at91sam9g45_restart(char, const char *);

/*          */
extern void at91_ioremap_shdwc(u32 base_addr);

/*        */
extern void at91_ioremap_matrix(u32 base_addr);

/*               */
extern void at91_ioremap_ramc(int id, u32 addr, u32 size);

 /*      */
#define AT91RM9200_PQFP		3	/*                                     */
#define AT91RM9200_BGA		4	/*                                    */

struct at91_gpio_bank {
	unsigned short id;		/*               */
	unsigned long regbase;		/*                                    */
};
extern void __init at91_gpio_init(struct at91_gpio_bank *, int nr_banks);
extern void __init at91_gpio_irq_setup(void);
extern int  __init at91_gpio_of_irq_setup(struct device_node *node,
					  struct device_node *parent);

extern int at91_extern_irq;
