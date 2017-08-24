/*
 * AD5791 SPI DAC driver
 *
 * Copyright 2011 Analog Devices Inc.
 *
 * Licensed under the GPL-2.
 */

#ifndef SPI_AD5791_H_
#define SPI_AD5791_H_

/* 
                                                              
                                                        
                                                        
                                                                         
 */

struct ad5791_platform_data {
	u16				vref_pos_mv;
	u16				vref_neg_mv;
	bool				use_rbuf_gain2;
};

#endif /*               */
