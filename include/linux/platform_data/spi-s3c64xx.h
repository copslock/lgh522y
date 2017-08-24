/* linux/arch/arm/plat-samsung/include/plat/s3c64xx-spi.h
 *
 * Copyright (C) 2009 Samsung Electronics Ltd.
 *	Jaswinder Singh <jassi.brar@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __S3C64XX_PLAT_SPI_H
#define __S3C64XX_PLAT_SPI_H

#include <linux/dmaengine.h>

struct platform_device;

/* 
                                                     
                                            
                                                                     
                                           
  
                                                
                                                                
                                              
 */
struct s3c64xx_spi_csinfo {
	u8 fb_delay;
	unsigned line;
};

/* 
                                                              
                                                                     
                                                  
                                                     
 */
struct s3c64xx_spi_info {
	int src_clk_nr;
	int num_cs;
	int (*cfg_gpio)(void);
	dma_filter_fn filter;
};

/* 
                                                                            
                          
                                             
                                                                          
                                                 
  
                                                                
                                 
 */
extern void s3c64xx_spi0_set_platdata(int (*cfg_gpio)(void), int src_clk_nr,
						int num_cs);
extern void s3c64xx_spi1_set_platdata(int (*cfg_gpio)(void), int src_clk_nr,
						int num_cs);
extern void s3c64xx_spi2_set_platdata(int (*cfg_gpio)(void), int src_clk_nr,
						int num_cs);

/*                                           */
extern int s3c64xx_spi0_cfg_gpio(void);
extern int s3c64xx_spi1_cfg_gpio(void);
extern int s3c64xx_spi2_cfg_gpio(void);

extern struct s3c64xx_spi_info s3c64xx_spi0_pdata;
extern struct s3c64xx_spi_info s3c64xx_spi1_pdata;
extern struct s3c64xx_spi_info s3c64xx_spi2_pdata;
#endif /*                      */
