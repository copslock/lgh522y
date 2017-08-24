/* linux/arch/arm/plat-s3c24xx/s3c244x.c
 *
 * Copyright (c) 2004-2006 Simtec Electronics
 *   Ben Dooks <ben@simtec.co.uk>
 *
 * Samsung S3C2440 and S3C2442 Mobile CPU support (not S3C2443)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/syscore_ops.h>
#include <linux/clk.h>
#include <linux/io.h>

#include <asm/system_misc.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <mach/hardware.h>
#include <asm/irq.h>

#include <plat/cpu-freq.h>

#include <mach/regs-clock.h>
#include <plat/regs-serial.h>
#include <mach/regs-gpio.h>

#include <plat/clock.h>
#include <plat/devs.h>
#include <plat/cpu.h>
#include <plat/pm.h>
#include <plat/pll.h>
#include <plat/nand-core.h>
#include <plat/watchdog-reset.h>

#include "regs-dsc.h"

static struct map_desc s3c244x_iodesc[] __initdata = {
	IODESC_ENT(CLKPWR),
	IODESC_ENT(TIMER),
	IODESC_ENT(WATCHDOG),
};

/*                     */

void __init s3c244x_init_uarts(struct s3c2410_uartcfg *cfg, int no)
{
	s3c24xx_init_uartdevs("s3c2440-uart", s3c2410_uart_resources, cfg, no);
}

void __init s3c244x_map_io(void)
{
	/*                        */

	iotable_init(s3c244x_iodesc, ARRAY_SIZE(s3c244x_iodesc));

	/*                                                        */

	s3c_device_sdi.name  = "s3c2440-sdi";
	s3c_device_i2c0.name  = "s3c2440-i2c";
	s3c_nand_setname("s3c2440-nand");
	s3c_device_ts.name = "s3c2440-ts";
	s3c_device_usbgadget.name = "s3c2440-usbgadget";
}

void __init_or_cpufreq s3c244x_setup_clocks(void)
{
	struct clk *xtal_clk;
	unsigned long clkdiv;
	unsigned long camdiv;
	unsigned long xtal;
	unsigned long hclk, fclk, pclk;
	int hdiv = 1;

	xtal_clk = clk_get(NULL, "xtal");
	xtal = clk_get_rate(xtal_clk);
	clk_put(xtal_clk);

	fclk = s3c24xx_get_pll(__raw_readl(S3C2410_MPLLCON), xtal) * 2;

	clkdiv = __raw_readl(S3C2410_CLKDIVN);
	camdiv = __raw_readl(S3C2440_CAMDIVN);

	/*                         */

	switch (clkdiv & S3C2440_CLKDIVN_HDIVN_MASK) {
	case S3C2440_CLKDIVN_HDIVN_1:
		hdiv = 1;
		break;

	case S3C2440_CLKDIVN_HDIVN_2:
		hdiv = 2;
		break;

	case S3C2440_CLKDIVN_HDIVN_4_8:
		hdiv = (camdiv & S3C2440_CAMDIVN_HCLK4_HALF) ? 8 : 4;
		break;

	case S3C2440_CLKDIVN_HDIVN_3_6:
		hdiv = (camdiv & S3C2440_CAMDIVN_HCLK3_HALF) ? 6 : 3;
		break;
	}

	hclk = fclk / hdiv;
	pclk = hclk / ((clkdiv & S3C2440_CLKDIVN_PDIVN) ? 2 : 1);

	/*                                    */

	printk("S3C244X: core %ld.%03ld MHz, memory %ld.%03ld MHz, peripheral %ld.%03ld MHz\n",
	       print_mhz(fclk), print_mhz(hclk), print_mhz(pclk));

	s3c24xx_setup_clocks(fclk, hclk, pclk);
}

void __init s3c244x_init_clocks(int xtal)
{
	/*                                                           
                                                                     
  */

	s3c24xx_register_baseclocks(xtal);
	s3c244x_setup_clocks();
	s3c2410_baseclk_add();
}

/*                                                                     */

struct bus_type s3c2440_subsys = {
	.name		= "s3c2440-core",
	.dev_name	= "s3c2440-core",
};

struct bus_type s3c2442_subsys = {
	.name		= "s3c2442-core",
	.dev_name	= "s3c2442-core",
};

/*                                                                           
                                                                        
                                                                      
                                                                       
*/

static int __init s3c2440_core_init(void)
{
	return subsys_system_register(&s3c2440_subsys, NULL);
}

core_initcall(s3c2440_core_init);

static int __init s3c2442_core_init(void)
{
	return subsys_system_register(&s3c2442_subsys, NULL);
}

core_initcall(s3c2442_core_init);


#ifdef CONFIG_PM
static struct sleep_save s3c244x_sleep[] = {
	SAVE_ITEM(S3C2440_DSC0),
	SAVE_ITEM(S3C2440_DSC1),
	SAVE_ITEM(S3C2440_GPJDAT),
	SAVE_ITEM(S3C2440_GPJCON),
	SAVE_ITEM(S3C2440_GPJUP)
};

static int s3c244x_suspend(void)
{
	s3c_pm_do_save(s3c244x_sleep, ARRAY_SIZE(s3c244x_sleep));
	return 0;
}

static void s3c244x_resume(void)
{
	s3c_pm_do_restore(s3c244x_sleep, ARRAY_SIZE(s3c244x_sleep));
}
#else
#define s3c244x_suspend NULL
#define s3c244x_resume  NULL
#endif

struct syscore_ops s3c244x_pm_syscore_ops = {
	.suspend	= s3c244x_suspend,
	.resume		= s3c244x_resume,
};

void s3c244x_restart(char mode, const char *cmd)
{
	if (mode == 's')
		soft_restart(0);

	arch_wdt_reset();

	/*                                                 */
	soft_restart(0);
}
