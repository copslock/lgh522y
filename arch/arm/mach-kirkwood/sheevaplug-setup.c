/*
 * arch/arm/mach-kirkwood/sheevaplug-setup.c
 *
 * Marvell SheevaPlug Reference Board Setup
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/ata_platform.h>
#include <linux/mtd/partitions.h>
#include <linux/mv643xx_eth.h>
#include <linux/gpio.h>
#include <linux/leds.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <mach/kirkwood.h>
#include <linux/platform_data/mmc-mvsdio.h>
#include "common.h"
#include "mpp.h"

static struct mtd_partition sheevaplug_nand_parts[] = {
	{
		.name = "u-boot",
		.offset = 0,
		.size = SZ_1M
	}, {
		.name = "uImage",
		.offset = MTDPART_OFS_NXTBLK,
		.size = SZ_4M
	}, {
		.name = "root",
		.offset = MTDPART_OFS_NXTBLK,
		.size = MTDPART_SIZ_FULL
	},
};

static struct mv643xx_eth_platform_data sheevaplug_ge00_data = {
	.phy_addr	= MV643XX_ETH_PHY_ADDR(0),
};

static struct mv_sata_platform_data sheeva_esata_sata_data = {
	.n_ports	= 2,
};

static struct mvsdio_platform_data sheevaplug_mvsdio_data = {
	/*                                                    */
};

static struct mvsdio_platform_data sheeva_esata_mvsdio_data = {
	.gpio_write_protect = 44, /*                                */
	.gpio_card_detect = 47,	  /*                              */
};

static struct gpio_led sheevaplug_led_pins[] = {
	{
		.name			= "plug:red:misc",
		.default_trigger	= "none",
		.gpio			= 46,
		.active_low		= 1,
	},
	{
		.name			= "plug:green:health",
		.default_trigger	= "default-on",
		.gpio			= 49,
		.active_low		= 1,
	},
};

static struct gpio_led_platform_data sheevaplug_led_data = {
	.leds		= sheevaplug_led_pins,
	.num_leds	= ARRAY_SIZE(sheevaplug_led_pins),
};

static struct platform_device sheevaplug_leds = {
	.name	= "leds-gpio",
	.id	= -1,
	.dev	= {
		.platform_data	= &sheevaplug_led_data,
	}
};

static unsigned int sheevaplug_mpp_config[] __initdata = {
	MPP29_GPIO,	/*                  */
	MPP46_GPIO,	/*         */
	MPP49_GPIO,	/*     */
	0
};

static unsigned int sheeva_esata_mpp_config[] __initdata = {
	MPP29_GPIO,	/*                  */
	MPP44_GPIO,	/*                  */
	MPP47_GPIO,	/*                */
	MPP49_GPIO,	/*           */
	0
};

static void __init sheevaplug_init(void)
{
	/*
                                          
  */
	kirkwood_init();

	/*                       */
	if (machine_is_esata_sheevaplug())
		kirkwood_mpp_conf(sheeva_esata_mpp_config);
	else
		kirkwood_mpp_conf(sheevaplug_mpp_config);

	kirkwood_uart0_init();
	kirkwood_nand_init(ARRAY_AND_SIZE(sheevaplug_nand_parts), 25);

	if (gpio_request(29, "USB Power Enable") != 0 ||
	    gpio_direction_output(29, 1) != 0)
		pr_err("can't set up GPIO 29 (USB Power Enable)\n");
	kirkwood_ehci_init();

	kirkwood_ge00_init(&sheevaplug_ge00_data);

	/*                                                        */
	if (machine_is_esata_sheevaplug())
		kirkwood_sata_init(&sheeva_esata_sata_data);

	/*                                            */
	if (machine_is_esata_sheevaplug())
		kirkwood_sdio_init(&sheeva_esata_mvsdio_data);
	else
		kirkwood_sdio_init(&sheevaplug_mvsdio_data);

	platform_device_register(&sheevaplug_leds);
}

#ifdef CONFIG_MACH_SHEEVAPLUG
MACHINE_START(SHEEVAPLUG, "Marvell SheevaPlug Reference Board")
	/*                                               */
	.atag_offset	= 0x100,
	.init_machine	= sheevaplug_init,
	.map_io		= kirkwood_map_io,
	.init_early	= kirkwood_init_early,
	.init_irq	= kirkwood_init_irq,
	.init_time	= kirkwood_timer_init,
	.restart	= kirkwood_restart,
MACHINE_END
#endif

#ifdef CONFIG_MACH_ESATA_SHEEVAPLUG
MACHINE_START(ESATA_SHEEVAPLUG, "Marvell eSATA SheevaPlug Reference Board")
	.atag_offset	= 0x100,
	.init_machine	= sheevaplug_init,
	.map_io		= kirkwood_map_io,
	.init_early	= kirkwood_init_early,
	.init_irq	= kirkwood_init_irq,
	.init_time	= kirkwood_timer_init,
	.restart	= kirkwood_restart,
MACHINE_END
#endif
