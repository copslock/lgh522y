/*
 * MPC52xx gpio driver
 *
 * Copyright (c) 2008 Sascha Hauer <s.hauer@pengutronix.de>, Pengutronix
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/of.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/of_gpio.h>
#include <linux/io.h>
#include <linux/of_platform.h>
#include <linux/module.h>

#include <asm/gpio.h>
#include <asm/mpc52xx.h>
#include <sysdev/fsl_soc.h>

static DEFINE_SPINLOCK(gpio_lock);

struct mpc52xx_gpiochip {
	struct of_mm_gpio_chip mmchip;
	unsigned int shadow_dvo;
	unsigned int shadow_gpioe;
	unsigned int shadow_ddr;
};

/*
                                                
  
                                                                    
                                       
  
                   
                   
              
              
              
              
              
              
  
 */
static int mpc52xx_wkup_gpio_get(struct gpio_chip *gc, unsigned int gpio)
{
	struct of_mm_gpio_chip *mm_gc = to_of_mm_gpio_chip(gc);
	struct mpc52xx_gpio_wkup __iomem *regs = mm_gc->regs;
	unsigned int ret;

	ret = (in_8(&regs->wkup_ival) >> (7 - gpio)) & 1;

	pr_debug("%s: gpio: %d ret: %d\n", __func__, gpio, ret);

	return ret;
}

static inline void
__mpc52xx_wkup_gpio_set(struct gpio_chip *gc, unsigned int gpio, int val)
{
	struct of_mm_gpio_chip *mm_gc = to_of_mm_gpio_chip(gc);
	struct mpc52xx_gpiochip *chip = container_of(mm_gc,
			struct mpc52xx_gpiochip, mmchip);
	struct mpc52xx_gpio_wkup __iomem *regs = mm_gc->regs;

	if (val)
		chip->shadow_dvo |= 1 << (7 - gpio);
	else
		chip->shadow_dvo &= ~(1 << (7 - gpio));

	out_8(&regs->wkup_dvo, chip->shadow_dvo);
}

static void
mpc52xx_wkup_gpio_set(struct gpio_chip *gc, unsigned int gpio, int val)
{
	unsigned long flags;

	spin_lock_irqsave(&gpio_lock, flags);

	__mpc52xx_wkup_gpio_set(gc, gpio, val);

	spin_unlock_irqrestore(&gpio_lock, flags);

	pr_debug("%s: gpio: %d val: %d\n", __func__, gpio, val);
}

static int mpc52xx_wkup_gpio_dir_in(struct gpio_chip *gc, unsigned int gpio)
{
	struct of_mm_gpio_chip *mm_gc = to_of_mm_gpio_chip(gc);
	struct mpc52xx_gpiochip *chip = container_of(mm_gc,
			struct mpc52xx_gpiochip, mmchip);
	struct mpc52xx_gpio_wkup __iomem *regs = mm_gc->regs;
	unsigned long flags;

	spin_lock_irqsave(&gpio_lock, flags);

	/*                   */
	chip->shadow_ddr &= ~(1 << (7 - gpio));
	out_8(&regs->wkup_ddr, chip->shadow_ddr);

	/*                    */
	chip->shadow_gpioe |= 1 << (7 - gpio);
	out_8(&regs->wkup_gpioe, chip->shadow_gpioe);

	spin_unlock_irqrestore(&gpio_lock, flags);

	return 0;
}

static int
mpc52xx_wkup_gpio_dir_out(struct gpio_chip *gc, unsigned int gpio, int val)
{
	struct of_mm_gpio_chip *mm_gc = to_of_mm_gpio_chip(gc);
	struct mpc52xx_gpio_wkup __iomem *regs = mm_gc->regs;
	struct mpc52xx_gpiochip *chip = container_of(mm_gc,
			struct mpc52xx_gpiochip, mmchip);
	unsigned long flags;

	spin_lock_irqsave(&gpio_lock, flags);

	__mpc52xx_wkup_gpio_set(gc, gpio, val);

	/*                    */
	chip->shadow_ddr |= 1 << (7 - gpio);
	out_8(&regs->wkup_ddr, chip->shadow_ddr);

	/*                        */
	chip->shadow_gpioe |= 1 << (7 - gpio);
	out_8(&regs->wkup_gpioe, chip->shadow_gpioe);

	spin_unlock_irqrestore(&gpio_lock, flags);

	pr_debug("%s: gpio: %d val: %d\n", __func__, gpio, val);

	return 0;
}

static int mpc52xx_wkup_gpiochip_probe(struct platform_device *ofdev)
{
	struct mpc52xx_gpiochip *chip;
	struct mpc52xx_gpio_wkup __iomem *regs;
	struct gpio_chip *gc;
	int ret;

	chip = kzalloc(sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	gc = &chip->mmchip.gc;

	gc->ngpio            = 8;
	gc->direction_input  = mpc52xx_wkup_gpio_dir_in;
	gc->direction_output = mpc52xx_wkup_gpio_dir_out;
	gc->get              = mpc52xx_wkup_gpio_get;
	gc->set              = mpc52xx_wkup_gpio_set;

	ret = of_mm_gpiochip_add(ofdev->dev.of_node, &chip->mmchip);
	if (ret)
		return ret;

	regs = chip->mmchip.regs;
	chip->shadow_gpioe = in_8(&regs->wkup_gpioe);
	chip->shadow_ddr = in_8(&regs->wkup_ddr);
	chip->shadow_dvo = in_8(&regs->wkup_dvo);

	return 0;
}

static int mpc52xx_gpiochip_remove(struct platform_device *ofdev)
{
	return -EBUSY;
}

static const struct of_device_id mpc52xx_wkup_gpiochip_match[] = {
	{ .compatible = "fsl,mpc5200-gpio-wkup", },
	{}
};

static struct platform_driver mpc52xx_wkup_gpiochip_driver = {
	.driver = {
		.name = "mpc5200-gpio-wkup",
		.owner = THIS_MODULE,
		.of_match_table = mpc52xx_wkup_gpiochip_match,
	},
	.probe = mpc52xx_wkup_gpiochip_probe,
	.remove = mpc52xx_gpiochip_remove,
};

/*
                                               
  
                                                                     
                                       
                                                                  
  
                    
                
                
                    
               
                    
                
                
                
 */
static int mpc52xx_simple_gpio_get(struct gpio_chip *gc, unsigned int gpio)
{
	struct of_mm_gpio_chip *mm_gc = to_of_mm_gpio_chip(gc);
	struct mpc52xx_gpio __iomem *regs = mm_gc->regs;
	unsigned int ret;

	ret = (in_be32(&regs->simple_ival) >> (31 - gpio)) & 1;

	return ret;
}

static inline void
__mpc52xx_simple_gpio_set(struct gpio_chip *gc, unsigned int gpio, int val)
{
	struct of_mm_gpio_chip *mm_gc = to_of_mm_gpio_chip(gc);
	struct mpc52xx_gpiochip *chip = container_of(mm_gc,
			struct mpc52xx_gpiochip, mmchip);
	struct mpc52xx_gpio __iomem *regs = mm_gc->regs;

	if (val)
		chip->shadow_dvo |= 1 << (31 - gpio);
	else
		chip->shadow_dvo &= ~(1 << (31 - gpio));
	out_be32(&regs->simple_dvo, chip->shadow_dvo);
}

static void
mpc52xx_simple_gpio_set(struct gpio_chip *gc, unsigned int gpio, int val)
{
	unsigned long flags;

	spin_lock_irqsave(&gpio_lock, flags);

	__mpc52xx_simple_gpio_set(gc, gpio, val);

	spin_unlock_irqrestore(&gpio_lock, flags);

	pr_debug("%s: gpio: %d val: %d\n", __func__, gpio, val);
}

static int mpc52xx_simple_gpio_dir_in(struct gpio_chip *gc, unsigned int gpio)
{
	struct of_mm_gpio_chip *mm_gc = to_of_mm_gpio_chip(gc);
	struct mpc52xx_gpiochip *chip = container_of(mm_gc,
			struct mpc52xx_gpiochip, mmchip);
	struct mpc52xx_gpio __iomem *regs = mm_gc->regs;
	unsigned long flags;

	spin_lock_irqsave(&gpio_lock, flags);

	/*                   */
	chip->shadow_ddr &= ~(1 << (31 - gpio));
	out_be32(&regs->simple_ddr, chip->shadow_ddr);

	/*                    */
	chip->shadow_gpioe |= 1 << (31 - gpio);
	out_be32(&regs->simple_gpioe, chip->shadow_gpioe);

	spin_unlock_irqrestore(&gpio_lock, flags);

	return 0;
}

static int
mpc52xx_simple_gpio_dir_out(struct gpio_chip *gc, unsigned int gpio, int val)
{
	struct of_mm_gpio_chip *mm_gc = to_of_mm_gpio_chip(gc);
	struct mpc52xx_gpiochip *chip = container_of(mm_gc,
			struct mpc52xx_gpiochip, mmchip);
	struct mpc52xx_gpio __iomem *regs = mm_gc->regs;
	unsigned long flags;

	spin_lock_irqsave(&gpio_lock, flags);

	/*                         */
	__mpc52xx_simple_gpio_set(gc, gpio, val);

	/*                    */
	chip->shadow_ddr |= 1 << (31 - gpio);
	out_be32(&regs->simple_ddr, chip->shadow_ddr);

	/*                        */
	chip->shadow_gpioe |= 1 << (31 - gpio);
	out_be32(&regs->simple_gpioe, chip->shadow_gpioe);

	spin_unlock_irqrestore(&gpio_lock, flags);

	pr_debug("%s: gpio: %d val: %d\n", __func__, gpio, val);

	return 0;
}

static int mpc52xx_simple_gpiochip_probe(struct platform_device *ofdev)
{
	struct mpc52xx_gpiochip *chip;
	struct gpio_chip *gc;
	struct mpc52xx_gpio __iomem *regs;
	int ret;

	chip = kzalloc(sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	gc = &chip->mmchip.gc;

	gc->ngpio            = 32;
	gc->direction_input  = mpc52xx_simple_gpio_dir_in;
	gc->direction_output = mpc52xx_simple_gpio_dir_out;
	gc->get              = mpc52xx_simple_gpio_get;
	gc->set              = mpc52xx_simple_gpio_set;

	ret = of_mm_gpiochip_add(ofdev->dev.of_node, &chip->mmchip);
	if (ret)
		return ret;

	regs = chip->mmchip.regs;
	chip->shadow_gpioe = in_be32(&regs->simple_gpioe);
	chip->shadow_ddr = in_be32(&regs->simple_ddr);
	chip->shadow_dvo = in_be32(&regs->simple_dvo);

	return 0;
}

static const struct of_device_id mpc52xx_simple_gpiochip_match[] = {
	{ .compatible = "fsl,mpc5200-gpio", },
	{}
};

static struct platform_driver mpc52xx_simple_gpiochip_driver = {
	.driver = {
		.name = "mpc5200-gpio",
		.owner = THIS_MODULE,
		.of_match_table = mpc52xx_simple_gpiochip_match,
	},
	.probe = mpc52xx_simple_gpiochip_probe,
	.remove = mpc52xx_gpiochip_remove,
};

static int __init mpc52xx_gpio_init(void)
{
	if (platform_driver_register(&mpc52xx_wkup_gpiochip_driver))
		printk(KERN_ERR "Unable to register wakeup GPIO driver\n");

	if (platform_driver_register(&mpc52xx_simple_gpiochip_driver))
		printk(KERN_ERR "Unable to register simple GPIO driver\n");

	return 0;
}


/*                                                                 */
subsys_initcall(mpc52xx_gpio_init);

/*                                                                  */

MODULE_DESCRIPTION("Freescale MPC52xx gpio driver");
MODULE_AUTHOR("Sascha Hauer <s.hauer@pengutronix.de");
MODULE_LICENSE("GPL v2");

