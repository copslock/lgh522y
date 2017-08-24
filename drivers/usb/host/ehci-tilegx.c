/*
 * Copyright 2012 Tilera Corporation. All Rights Reserved.
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation, version 2.
 *
 *   This program is distributed in the hope that it will be useful, but
 *   WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, GOOD TITLE or
 *   NON INFRINGEMENT.  See the GNU General Public License for
 *   more details.
 */

/*
                                                  
 */

#include <linux/irq.h>
#include <linux/platform_device.h>
#include <linux/usb/tilegx.h>
#include <linux/usb.h>

#include <asm/homecache.h>

#include <gxio/iorpc_usb_host.h>
#include <gxio/usb_host.h>

static void tilegx_start_ehc(void)
{
}

static void tilegx_stop_ehc(void)
{
}

static int tilegx_ehci_setup(struct usb_hcd *hcd)
{
	int ret = ehci_init(hcd);

	/*
                    
   
                                               
                                 
   
                                                                 
                                                              
                                     
  */

	return ret;
}

static const struct hc_driver ehci_tilegx_hc_driver = {
	.description		= hcd_name,
	.product_desc		= "Tile-Gx EHCI",
	.hcd_priv_size		= sizeof(struct ehci_hcd),

	/*
                             
  */
	.irq			= ehci_irq,
	.flags			= HCD_MEMORY | HCD_USB2,

	/*
                               
  */
	.reset			= tilegx_ehci_setup,
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
	.bus_suspend		= ehci_bus_suspend,
	.bus_resume		= ehci_bus_resume,
	.relinquish_port	= ehci_relinquish_port,
	.port_handed_over	= ehci_port_handed_over,

	.clear_tt_buffer_complete	= ehci_clear_tt_buffer_complete,
};

static int ehci_hcd_tilegx_drv_probe(struct platform_device *pdev)
{
	struct usb_hcd *hcd;
	struct ehci_hcd *ehci;
	struct tilegx_usb_platform_data *pdata = pdev->dev.platform_data;
	pte_t pte = { 0 };
	int my_cpu = smp_processor_id();
	int ret;

	if (usb_disabled())
		return -ENODEV;

	/*
                                                               
                  
  */
	if (gxio_usb_host_init(&pdata->usb_ctx, pdata->dev_index, 1) != 0)
		return -ENXIO;

	hcd = usb_create_hcd(&ehci_tilegx_hc_driver, &pdev->dev,
			     dev_name(&pdev->dev));
	if (!hcd) {
          ret = -ENOMEM;
          goto err_hcd;
        }

	/*
                                                                   
                                                               
  */
	hcd->rsrc_start =
		(ulong) gxio_usb_host_get_reg_start(&pdata->usb_ctx);
	hcd->rsrc_len = gxio_usb_host_get_reg_len(&pdata->usb_ctx);
	hcd->regs = gxio_usb_host_get_reg_start(&pdata->usb_ctx);

	tilegx_start_ehc();

	ehci = hcd_to_ehci(hcd);
	ehci->caps = hcd->regs;
	ehci->regs =
		hcd->regs + HC_LENGTH(ehci, readl(&ehci->caps->hc_capbase));
	/*                                               */
	ehci->hcs_params = readl(&ehci->caps->hcs_params);

	/*                                    */
	pdata->irq = create_irq();
	if (pdata->irq < 0) {
		ret = -ENXIO;
		goto err_no_irq;
	}

	tile_irq_activate(pdata->irq, TILE_IRQ_PERCPU);

	/*                       */
	ret = gxio_usb_host_cfg_interrupt(&pdata->usb_ctx,
					  cpu_x(my_cpu), cpu_y(my_cpu),
					  KERNEL_PL, pdata->irq);
	if (ret) {
		ret = -ENXIO;
		goto err_have_irq;
	}

	/*                             */
	pte = pte_set_home(pte, PAGE_HOME_HASH);
	ret = gxio_usb_host_register_client_memory(&pdata->usb_ctx, pte, 0);
	if (ret) {
		ret = -ENXIO;
		goto err_have_irq;
	}

	ret = usb_add_hcd(hcd, pdata->irq, IRQF_SHARED);
	if (ret == 0) {
		platform_set_drvdata(pdev, hcd);
		return ret;
	}

err_have_irq:
	destroy_irq(pdata->irq);
err_no_irq:
	tilegx_stop_ehc();
	usb_put_hcd(hcd);
err_hcd:
	gxio_usb_host_destroy(&pdata->usb_ctx);
	return ret;
}

static int ehci_hcd_tilegx_drv_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);
	struct tilegx_usb_platform_data *pdata = pdev->dev.platform_data;

	usb_remove_hcd(hcd);
	usb_put_hcd(hcd);
	tilegx_stop_ehc();
	gxio_usb_host_destroy(&pdata->usb_ctx);
	destroy_irq(pdata->irq);
	platform_set_drvdata(pdev, NULL);

	return 0;
}

static void ehci_hcd_tilegx_drv_shutdown(struct platform_device *pdev)
{
	usb_hcd_platform_shutdown(pdev);
	ehci_hcd_tilegx_drv_remove(pdev);
}

static struct platform_driver ehci_hcd_tilegx_driver = {
	.probe		= ehci_hcd_tilegx_drv_probe,
	.remove		= ehci_hcd_tilegx_drv_remove,
	.shutdown	= ehci_hcd_tilegx_drv_shutdown,
	.driver = {
		.name	= "tilegx-ehci",
		.owner	= THIS_MODULE,
	}
};

MODULE_ALIAS("platform:tilegx-ehci");
