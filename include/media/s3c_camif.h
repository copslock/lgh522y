/*
 * s3c24xx/s3c64xx SoC series Camera Interface (CAMIF) driver
 *
 * Copyright (C) 2012 Sylwester Nawrocki <sylvester.nawrocki@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef MEDIA_S3C_CAMIF_
#define MEDIA_S3C_CAMIF_

#include <linux/i2c.h>
#include <media/v4l2-mediabus.h>

/* 
                                                             
                                                                 
                                                                         
                             
                                                             
                                                                         
                                                                    
 */
struct s3c_camif_sensor_info {
	struct i2c_board_info i2c_board_info;
	unsigned long clock_frequency;
	enum v4l2_mbus_type mbus_type;
	u16 i2c_bus_num;
	u16 flags;
	u8 use_field;
};

struct s3c_camif_plat_data {
	struct s3c_camif_sensor_info sensor;
	int (*gpio_get)(void);
	int (*gpio_put)(void);
};

/*                                   */
int s3c_camif_gpio_get(void);
int s3c_camif_gpio_put(void);

#endif /*                  */
