/*
 * Renesas Emma Mobile 8250 driver
 *
 *  Copyright (C) 2012 Magnus Damm
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License
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

#include <linux/device.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/serial_8250.h>
#include <linux/serial_core.h>
#include <linux/serial_reg.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/slab.h>

#include "8250.h"

#define UART_DLL_EM 9
#define UART_DLM_EM 10

struct serial8250_em_priv {
	struct clk *sclk;
	int line;
};

static void serial8250_em_serial_out(struct uart_port *p, int offset, int value)
{
	switch (offset) {
	case UART_TX: /*           */
		writeb(value, p->membase);
		break;
	case UART_FCR: /*                 */
	case UART_LCR: /*                 */
	case UART_MCR: /*                 */
	case UART_SCR: /*                 */
		writel(value, p->membase + ((offset + 1) << 2));
		break;
	case UART_IER: /*            */
		value &= 0x0f; /*                                */
		/*              */
	case UART_DLL_EM: /*                 */
	case UART_DLM_EM: /*                 */
		writel(value, p->membase + (offset << 2));
	}
}

static unsigned int serial8250_em_serial_in(struct uart_port *p, int offset)
{
	switch (offset) {
	case UART_RX: /*           */
		return readb(p->membase);
	case UART_MCR: /*                 */
	case UART_LSR: /*                 */
	case UART_MSR: /*                 */
	case UART_SCR: /*                 */
		return readl(p->membase + ((offset + 1) << 2));
	case UART_IER: /*            */
	case UART_IIR: /*            */
	case UART_DLL_EM: /*                 */
	case UART_DLM_EM: /*                 */
		return readl(p->membase + (offset << 2));
	}
	return 0;
}

static int serial8250_em_serial_dl_read(struct uart_8250_port *up)
{
	return serial_in(up, UART_DLL_EM) | serial_in(up, UART_DLM_EM) << 8;
}

static void serial8250_em_serial_dl_write(struct uart_8250_port *up, int value)
{
	serial_out(up, UART_DLL_EM, value & 0xff);
	serial_out(up, UART_DLM_EM, value >> 8 & 0xff);
}

static int serial8250_em_probe(struct platform_device *pdev)
{
	struct resource *regs = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	struct resource *irq = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	struct serial8250_em_priv *priv;
	struct uart_8250_port up;
	int ret = -EINVAL;

	if (!regs || !irq) {
		dev_err(&pdev->dev, "missing registers or irq\n");
		goto err0;
	}

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		dev_err(&pdev->dev, "unable to allocate private data\n");
		ret = -ENOMEM;
		goto err0;
	}

	priv->sclk = clk_get(&pdev->dev, "sclk");
	if (IS_ERR(priv->sclk)) {
		dev_err(&pdev->dev, "unable to get clock\n");
		ret = PTR_ERR(priv->sclk);
		goto err1;
	}

	memset(&up, 0, sizeof(up));
	up.port.mapbase = regs->start;
	up.port.irq = irq->start;
	up.port.type = PORT_UNKNOWN;
	up.port.flags = UPF_BOOT_AUTOCONF | UPF_FIXED_PORT | UPF_IOREMAP;
	up.port.dev = &pdev->dev;
	up.port.private_data = priv;

	clk_enable(priv->sclk);
	up.port.uartclk = clk_get_rate(priv->sclk);

	up.port.iotype = UPIO_MEM32;
	up.port.serial_in = serial8250_em_serial_in;
	up.port.serial_out = serial8250_em_serial_out;
	up.dl_read = serial8250_em_serial_dl_read;
	up.dl_write = serial8250_em_serial_dl_write;

	ret = serial8250_register_8250_port(&up);
	if (ret < 0) {
		dev_err(&pdev->dev, "unable to register 8250 port\n");
		goto err2;
	}

	priv->line = ret;
	platform_set_drvdata(pdev, priv);
	return 0;

 err2:
	clk_disable(priv->sclk);
	clk_put(priv->sclk);
 err1:
	kfree(priv);
 err0:
	return ret;
}

static int serial8250_em_remove(struct platform_device *pdev)
{
	struct serial8250_em_priv *priv = platform_get_drvdata(pdev);

	serial8250_unregister_port(priv->line);
	clk_disable(priv->sclk);
	clk_put(priv->sclk);
	kfree(priv);
	return 0;
}

static const struct of_device_id serial8250_em_dt_ids[] = {
	{ .compatible = "renesas,em-uart", },
	{},
};
MODULE_DEVICE_TABLE(of, serial8250_em_dt_ids);

static struct platform_driver serial8250_em_platform_driver = {
	.driver = {
		.name		= "serial8250-em",
		.of_match_table = serial8250_em_dt_ids,
		.owner		= THIS_MODULE,
	},
	.probe			= serial8250_em_probe,
	.remove			= serial8250_em_remove,
};

module_platform_driver(serial8250_em_platform_driver);

MODULE_AUTHOR("Magnus Damm");
MODULE_DESCRIPTION("Renesas Emma Mobile 8250 Driver");
MODULE_LICENSE("GPL v2");
