/*
 * AD5415, AD5426, AD5429, AD5432, AD5439, AD5443, AD5449 Digital to Analog
 * Converter driver.
 *
 * Copyright 2012 Analog Devices Inc.
 *  Author: Lars-Peter Clausen <lars@metafoo.de>
 *
 * Licensed under the GPL-2.
 */

#ifndef __LINUX_PLATFORM_DATA_AD5449_H__
#define __LINUX_PLATFORM_DATA_AD5449_H__

/* 
                                                      
                                                                
                                                                    
                                                                  
                                                                                
                               
 */
enum ad5449_sdo_mode {
	AD5449_SDO_DRIVE_FULL = 0x0,
	AD5449_SDO_DRIVE_WEAK = 0x1,
	AD5449_SDO_OPEN_DRAIN = 0x2,
	AD5449_SDO_DISABLED = 0x3,
};

/* 
                                                                        
                          
                                                                               
                                                        
 */
struct ad5449_platform_data {
	enum ad5449_sdo_mode sdo_mode;
	bool hardware_clear_to_midscale;
};

#endif
