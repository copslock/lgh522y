/*
 * EHCI HCD (Host Controller Driver) for USB.
 *
 * Bus Glue for Xilinx EHCI core on the of_platform bus
 *
 * Copyright (c) 2009 Xilinx, Inc.
 *
 * Based on "ehci-ppc-of.c" by Valentine Barshak <vbarshak@ru.mvista.com>
 * and "ehci-ppc-soc.c" by Stefan Roese <sr@denx.de>
 * and "ohci-ppc-of.c" by Sylvain Munaut <tnt@246tNt.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <linux/err.h>
#include <linux/signal.h>

#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>

/* 
                                                                          
                                                                         
                                                        
  
                                                                             
                                                                          
                                                                       
                 
  
                                                                            
                                                                          
                                                                          
 */
static int ehci_xilinx_port_handed_over(struct usb_hcd *hcd, int portnum)
{
	dev_warn(hcd->self.controller, "port %d cannot be enabled\n", portnum);
	if (hcd->has_tt) {
		dev_warn(hcd->self.controller,
			"Maybe you have connected a low speed device?\n");

		dev_warn(hcd->self.controller,
			"We do not support low speed devices\n");
	} else {
		dev_warn(hcd->self.controller,
			"Maybe your device is not a high speed device?\n");
		dev_warn(hcd->self.controller,
			"The USB host controller does not support full speed "
			"nor low speed devices\n");
		dev_warn(hcd->self.controller,
			"You can reconfigure the host controller to have "
			"full speed support\n");
	}

	return 0;
}


static const struct hc_driver ehci_xilinx_of_hc_driver = {
	.description		= hcd_name,
	.product_desc		= "OF EHCI",
	.hcd_priv_size		= sizeof(struct ehci_hcd),

	/*
                            
  */
	.irq			= ehci_irq,
	.flags			= HCD_MEMORY | HCD_USB2,

	/*
                              
  */
	.reset			= ehci_setup,
	.start			= ehci_run,
	.stop			= ehci_stop,
	.shutdown		= ehci_shutdown,

	/*
                                                         
  */
	.urb_enqueue		= ehci_urb_enqueue,
	.urb_dequeue		= ehci_urb_dequeue,
	.endpoint_disable	= ehci_endpoint_disable,
	.endpoint_reset		= ehci_endpoint_reset,

	/*
                      
  */
	.get_frame_number	= ehci_get_frame,

	/*
                    
  */
	.hub_status_data	= ehci_hub_status_data,
	.hub_control		= ehci_hub_control,
#ifdef	CONFIG_PM
	.bus_suspend		= ehci_bus_suspend,
	.bus_resume		= ehci_bus_resume,
#endif
	.relinquish_port	= NULL,
	.port_handed_over	= ehci_xilinx_port_handed_over,

	.clear_tt_buffer_complete = ehci_clear_tt_buffer_complete,
};

/* 
                                                                      
                                                                    
  
                                                                              
                                                                            
                                                                           
                                                        
 */
static int ehci_hcd_xilinx_of_probe(struct platform_device *op)
{
	struct device_node *dn = op->dev.of_node;
	struct usb_hcd *hcd;
	struct ehci_hcd	*ehci;
	struct resource res;
	int irq;
	int rv;
	int *value;

	if (usb_disabled())
		return -ENODEV;

	dev_dbg(&op->dev, "initializing XILINX-OF USB Controller\n");

	rv = of_address_to_resource(dn, 0, &res);
	if (rv)
		return rv;

	hcd = usb_create_hcd(&ehci_xilinx_of_hc_driver, &op->dev,
				"XILINX-OF USB");
	if (!hcd)
		return -ENOMEM;

	hcd->rsrc_start = res.start;
	hcd->rsrc_len = resource_size(&res);

	irq = irq_of_parse_and_map(dn, 0);
	if (!irq) {
		printk(KERN_ERR "%s: irq_of_parse_and_map failed\n", __FILE__);
		rv = -EBUSY;
		goto err_irq;
	}

	hcd->regs = devm_ioremap_resource(&op->dev, &res);
	if (IS_ERR(hcd->regs)) {
		rv = PTR_ERR(hcd->regs);
		goto err_irq;
	}

	ehci = hcd_to_ehci(hcd);

	/*                                                            
                                  
  */
	ehci->big_endian_mmio = 1;
	ehci->big_endian_desc = 1;

	/*                                                                 
  */
	value = (int *)of_get_property(dn, "xlnx,support-usb-fs", NULL);
	if (value && (*value == 1)) {
		ehci_dbg(ehci, "USB host controller supports FS devices\n");
		hcd->has_tt = 1;
	} else {
		ehci_dbg(ehci,
			"USB host controller is HS only\n");
		hcd->has_tt = 0;
	}

	/*                                              
  */
	ehci->caps = hcd->regs + 0x100;

	rv = usb_add_hcd(hcd, irq, 0);
	if (rv == 0)
		return 0;

err_irq:
	usb_put_hcd(hcd);

	return rv;
}

/* 
                                                                 
                                                                   
  
                                                                          
                
 */
static int ehci_hcd_xilinx_of_remove(struct platform_device *op)
{
	struct usb_hcd *hcd = dev_get_drvdata(&op->dev);
	dev_set_drvdata(&op->dev, NULL);

	dev_dbg(&op->dev, "stopping XILINX-OF USB Controller\n");

	usb_remove_hcd(hcd);

	usb_put_hcd(hcd);

	return 0;
}

/* 
                                                 
                                                                   
  
                                                             
 */
static void ehci_hcd_xilinx_of_shutdown(struct platform_device *op)
{
	struct usb_hcd *hcd = dev_get_drvdata(&op->dev);

	if (hcd->driver->shutdown)
		hcd->driver->shutdown(hcd);
}


static const struct of_device_id ehci_hcd_xilinx_of_match[] = {
		{.compatible = "xlnx,xps-usb-host-1.00.a",},
	{},
};
MODULE_DEVICE_TABLE(of, ehci_hcd_xilinx_of_match);

static struct platform_driver ehci_hcd_xilinx_of_driver = {
	.probe		= ehci_hcd_xilinx_of_probe,
	.remove		= ehci_hcd_xilinx_of_remove,
	.shutdown	= ehci_hcd_xilinx_of_shutdown,
	.driver = {
		.name = "xilinx-of-ehci",
		.owner = THIS_MODULE,
		.of_match_table = ehci_hcd_xilinx_of_match,
	},
};
