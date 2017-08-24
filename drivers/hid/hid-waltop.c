/*
 *  HID driver for Waltop devices not fully compliant with HID standard
 *
 *  Copyright (c) 2010 Nikolai Kondrashov
 */

/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#include <linux/device.h>
#include <linux/hid.h>
#include <linux/module.h>

#include "hid-ids.h"

/*
                                                                       
                                                                       
                                                                       
  
                                                                            
                                                                         
  
                                                                           
                                                                            
                              
  
                                                                            
                                                                            
                                         
  
                                            
  
                                                                   
  
                       
                      
                   
 */

/*
                                                                             
                                                                         
 */

/*                                                                */
#define SLIM_TABLET_5_8_INCH_RDESC_ORIG_SIZE	222

/*                                       */
static __u8 slim_tablet_5_8_inch_rdesc_fixed[] = {
	0x05, 0x0D,         /*                                      */
	0x09, 0x02,         /*                                      */
	0xA1, 0x01,         /*                                      */
	0x85, 0x10,         /*                                      */
	0x09, 0x20,         /*                                      */
	0xA0,               /*                                      */
	0x09, 0x42,         /*                                      */
	0x09, 0x44,         /*                                      */
	0x09, 0x46,         /*                                      */
	0x15, 0x01,         /*                                      */
	0x25, 0x03,         /*                                      */
	0x75, 0x04,         /*                                      */
	0x95, 0x01,         /*                                      */
	0x80,               /*                                      */
	0x09, 0x32,         /*                                      */
	0x14,               /*                                      */
	0x25, 0x01,         /*                                      */
	0x75, 0x01,         /*                                      */
	0x95, 0x01,         /*                                      */
	0x81, 0x02,         /*                                      */
	0x95, 0x03,         /*                                      */
	0x81, 0x03,         /*                                      */
	0x75, 0x10,         /*                                      */
	0x95, 0x01,         /*                                      */
	0x14,               /*                                      */
	0xA4,               /*                                      */
	0x05, 0x01,         /*                                      */
	0x65, 0x13,         /*                                      */
	0x55, 0xFD,         /*                                      */
	0x34,               /*                                      */
	0x09, 0x30,         /*                                      */
	0x46, 0x88, 0x13,   /*                                      */
	0x26, 0x10, 0x27,   /*                                      */
	0x81, 0x02,         /*                                      */
	0x09, 0x31,         /*                                      */
	0x46, 0xB8, 0x0B,   /*                                      */
	0x26, 0x70, 0x17,   /*                                      */
	0x81, 0x02,         /*                                      */
	0xB4,               /*                                      */
	0x09, 0x30,         /*                                      */
	0x26, 0xFF, 0x03,   /*                                      */
	0x81, 0x02,         /*                                      */
	0xC0,               /*                                      */
	0xC0                /*                                      */
};

/*
                                                                              
                                                                          
 */

/*                                                                 */
#define SLIM_TABLET_12_1_INCH_RDESC_ORIG_SIZE	269

/*                                        */
static __u8 slim_tablet_12_1_inch_rdesc_fixed[] = {
	0x05, 0x0D,         /*                                      */
	0x09, 0x02,         /*                                      */
	0xA1, 0x01,         /*                                      */
	0x85, 0x10,         /*                                      */
	0x09, 0x20,         /*                                      */
	0xA0,               /*                                      */
	0x09, 0x42,         /*                                      */
	0x09, 0x44,         /*                                      */
	0x09, 0x46,         /*                                      */
	0x15, 0x01,         /*                                      */
	0x25, 0x03,         /*                                      */
	0x75, 0x04,         /*                                      */
	0x95, 0x01,         /*                                      */
	0x80,               /*                                      */
	0x09, 0x32,         /*                                      */
	0x14,               /*                                      */
	0x25, 0x01,         /*                                      */
	0x75, 0x01,         /*                                      */
	0x95, 0x01,         /*                                      */
	0x81, 0x02,         /*                                      */
	0x95, 0x03,         /*                                      */
	0x81, 0x03,         /*                                      */
	0x75, 0x10,         /*                                      */
	0x95, 0x01,         /*                                      */
	0x14,               /*                                      */
	0xA4,               /*                                      */
	0x05, 0x01,         /*                                      */
	0x65, 0x13,         /*                                      */
	0x55, 0xFD,         /*                                      */
	0x34,               /*                                      */
	0x09, 0x30,         /*                                      */
	0x46, 0x10, 0x27,   /*                                      */
	0x26, 0x20, 0x4E,   /*                                      */
	0x81, 0x02,         /*                                      */
	0x09, 0x31,         /*                                      */
	0x46, 0x6A, 0x18,   /*                                      */
	0x26, 0xD4, 0x30,   /*                                      */
	0x81, 0x02,         /*                                      */
	0xB4,               /*                                      */
	0x09, 0x30,         /*                                      */
	0x26, 0xFF, 0x03,   /*                                      */
	0x81, 0x02,         /*                                      */
	0xC0,               /*                                      */
	0xC0                /*                                      */
};

/*
                                                              
                                                            
 */

/*                                                 */
#define Q_PAD_RDESC_ORIG_SIZE	241

/*                        */
static __u8 q_pad_rdesc_fixed[] = {
	0x05, 0x0D,         /*                                      */
	0x09, 0x02,         /*                                      */
	0xA1, 0x01,         /*                                      */
	0x85, 0x10,         /*                                      */
	0x09, 0x20,         /*                                      */
	0xA0,               /*                                      */
	0x09, 0x42,         /*                                      */
	0x09, 0x44,         /*                                      */
	0x09, 0x46,         /*                                      */
	0x15, 0x01,         /*                                      */
	0x25, 0x03,         /*                                      */
	0x75, 0x04,         /*                                      */
	0x95, 0x01,         /*                                      */
	0x80,               /*                                      */
	0x09, 0x32,         /*                                      */
	0x14,               /*                                      */
	0x25, 0x01,         /*                                      */
	0x75, 0x01,         /*                                      */
	0x95, 0x01,         /*                                      */
	0x81, 0x02,         /*                                      */
	0x95, 0x03,         /*                                      */
	0x81, 0x03,         /*                                      */
	0x75, 0x10,         /*                                      */
	0x95, 0x01,         /*                                      */
	0x14,               /*                                      */
	0xA4,               /*                                      */
	0x05, 0x01,         /*                                      */
	0x65, 0x13,         /*                                      */
	0x55, 0xFD,         /*                                      */
	0x34,               /*                                      */
	0x09, 0x30,         /*                                      */
	0x46, 0x70, 0x17,   /*                                      */
	0x26, 0x00, 0x30,   /*                                      */
	0x81, 0x02,         /*                                      */
	0x09, 0x31,         /*                                      */
	0x46, 0x94, 0x11,   /*                                      */
	0x26, 0x00, 0x24,   /*                                      */
	0x81, 0x02,         /*                                      */
	0xB4,               /*                                      */
	0x09, 0x30,         /*                                      */
	0x26, 0xFF, 0x03,   /*                                      */
	0x81, 0x02,         /*                                      */
	0xC0,               /*                                      */
	0xC0                /*                                      */
};

/*
                                                                                
                                                               
 */

/*                                                                */
#define PID_0038_RDESC_ORIG_SIZE	241

/*
                                                    
 */
static __u8 pid_0038_rdesc_fixed[] = {
	0x05, 0x0D,         /*                                      */
	0x09, 0x02,         /*                                      */
	0xA1, 0x01,         /*                                      */
	0x85, 0x10,         /*                                      */
	0x09, 0x20,         /*                                      */
	0xA0,               /*                                      */
	0x09, 0x42,         /*                                      */
	0x09, 0x44,         /*                                      */
	0x09, 0x46,         /*                                      */
	0x15, 0x01,         /*                                      */
	0x25, 0x03,         /*                                      */
	0x75, 0x04,         /*                                      */
	0x95, 0x01,         /*                                      */
	0x80,               /*                                      */
	0x09, 0x32,         /*                                      */
	0x14,               /*                                      */
	0x25, 0x01,         /*                                      */
	0x75, 0x01,         /*                                      */
	0x95, 0x01,         /*                                      */
	0x81, 0x02,         /*                                      */
	0x95, 0x03,         /*                                      */
	0x81, 0x03,         /*                                      */
	0x75, 0x10,         /*                                      */
	0x95, 0x01,         /*                                      */
	0x14,               /*                                      */
	0xA4,               /*                                      */
	0x05, 0x01,         /*                                      */
	0x65, 0x13,         /*                                      */
	0x55, 0xFD,         /*                                      */
	0x34,               /*                                      */
	0x09, 0x30,         /*                                      */
	0x46, 0x2E, 0x22,   /*                                      */
	0x26, 0x00, 0x46,   /*                                      */
	0x81, 0x02,         /*                                      */
	0x09, 0x31,         /*                                      */
	0x46, 0x82, 0x14,   /*                                      */
	0x26, 0x00, 0x2A,   /*                                      */
	0x81, 0x02,         /*                                      */
	0xB4,               /*                                      */
	0x09, 0x30,         /*                                      */
	0x26, 0xFF, 0x03,   /*                                      */
	0x81, 0x02,         /*                                      */
	0xC0,               /*                                      */
	0xC0                /*                                      */
};

/*
                                                                               
                                                                           
 */

/*                                                                  */
#define MEDIA_TABLET_10_6_INCH_RDESC_ORIG_SIZE	300

/*                                         */
static __u8 media_tablet_10_6_inch_rdesc_fixed[] = {
	0x05, 0x0D,         /*                                      */
	0x09, 0x02,         /*                                      */
	0xA1, 0x01,         /*                                      */
	0x85, 0x10,         /*                                      */
	0x09, 0x20,         /*                                      */
	0xA0,               /*                                      */
	0x09, 0x42,         /*                                      */
	0x09, 0x44,         /*                                      */
	0x09, 0x46,         /*                                      */
	0x15, 0x01,         /*                                      */
	0x25, 0x03,         /*                                      */
	0x75, 0x04,         /*                                      */
	0x95, 0x01,         /*                                      */
	0x80,               /*                                      */
	0x75, 0x01,         /*                                      */
	0x09, 0x32,         /*                                      */
	0x14,               /*                                      */
	0x25, 0x01,         /*                                      */
	0x95, 0x01,         /*                                      */
	0x81, 0x02,         /*                                      */
	0x95, 0x03,         /*                                      */
	0x81, 0x03,         /*                                      */
	0x75, 0x10,         /*                                      */
	0x95, 0x01,         /*                                      */
	0x14,               /*                                      */
	0xA4,               /*                                      */
	0x05, 0x01,         /*                                      */
	0x65, 0x13,         /*                                      */
	0x55, 0xFD,         /*                                      */
	0x34,               /*                                      */
	0x09, 0x30,         /*                                      */
	0x46, 0x28, 0x23,   /*                                      */
	0x26, 0x50, 0x46,   /*                                      */
	0x81, 0x02,         /*                                      */
	0x09, 0x31,         /*                                      */
	0x46, 0x7C, 0x15,   /*                                      */
	0x26, 0xF8, 0x2A,   /*                                      */
	0x81, 0x02,         /*                                      */
	0xB4,               /*                                      */
	0x09, 0x30,         /*                                      */
	0x26, 0xFF, 0x03,   /*                                      */
	0x81, 0x02,         /*                                      */
	0xC0,               /*                                      */
	0xC0,               /*                                      */
	0x05, 0x01,         /*                                      */
	0x09, 0x02,         /*                                      */
	0xA1, 0x01,         /*                                      */
	0x85, 0x01,         /*                                      */
	0x09, 0x01,         /*                                      */
	0xA0,               /*                                      */
	0x75, 0x08,         /*                                      */
	0x95, 0x03,         /*                                      */
	0x81, 0x03,         /*                                      */
	0x95, 0x02,         /*                                      */
	0x15, 0xFF,         /*                                      */
	0x25, 0x01,         /*                                      */
	0x09, 0x38,         /*                                      */
	0x0B, 0x38, 0x02,   /*                                      */
		0x0C, 0x00,
	0x81, 0x06,         /*                                      */
	0x95, 0x02,         /*                                      */
	0x81, 0x03,         /*                                      */
	0xC0,               /*                                      */
	0xC0,               /*                                      */
	0x05, 0x0C,         /*                                      */
	0x09, 0x01,         /*                                      */
	0xA1, 0x01,         /*                                      */
	0x85, 0x0D,         /*                                      */
	0x95, 0x01,         /*                                      */
	0x75, 0x10,         /*                                      */
	0x81, 0x03,         /*                                      */
	0x0A, 0x2F, 0x02,   /*                                      */
	0x0A, 0x2E, 0x02,   /*                                      */
	0x0A, 0x2D, 0x02,   /*                                      */
	0x09, 0xB6,         /*                                      */
	0x09, 0xB5,         /*                                      */
	0x08,               /*                                      */
	0x08,               /*                                      */
	0x08,               /*                                      */
	0x08,               /*                                      */
	0x08,               /*                                      */
	0x0A, 0x2E, 0x02,   /*                                      */
	0x0A, 0x2D, 0x02,   /*                                      */
	0x15, 0x0C,         /*                                      */
	0x25, 0x17,         /*                                      */
	0x75, 0x05,         /*                                      */
	0x80,               /*                                      */
	0x75, 0x03,         /*                                      */
	0x81, 0x03,         /*                                      */
	0x75, 0x20,         /*                                      */
	0x81, 0x03,         /*                                      */
	0xC0,               /*                                      */
	0x09, 0x01,         /*                                      */
	0xA1, 0x01,         /*                                      */
	0x85, 0x0C,         /*                                      */
	0x75, 0x01,         /*                                      */
	0x09, 0xE9,         /*                                      */
	0x09, 0xEA,         /*                                      */
	0x09, 0xE2,         /*                                      */
	0x14,               /*                                      */
	0x25, 0x01,         /*                                      */
	0x95, 0x03,         /*                                      */
	0x81, 0x06,         /*                                      */
	0x95, 0x35,         /*                                      */
	0x81, 0x03,         /*                                      */
	0xC0                /*                                      */
};

/*
                                                                               
                                                                           
 */

/*                                                                  */
#define MEDIA_TABLET_14_1_INCH_RDESC_ORIG_SIZE	309

/*                                         */
static __u8 media_tablet_14_1_inch_rdesc_fixed[] = {
	0x05, 0x0D,         /*                                      */
	0x09, 0x02,         /*                                      */
	0xA1, 0x01,         /*                                      */
	0x85, 0x10,         /*                                      */
	0x09, 0x20,         /*                                      */
	0xA0,               /*                                      */
	0x09, 0x42,         /*                                      */
	0x09, 0x44,         /*                                      */
	0x09, 0x46,         /*                                      */
	0x15, 0x01,         /*                                      */
	0x25, 0x03,         /*                                      */
	0x75, 0x04,         /*                                      */
	0x95, 0x01,         /*                                      */
	0x80,               /*                                      */
	0x75, 0x01,         /*                                      */
	0x09, 0x32,         /*                                      */
	0x14,               /*                                      */
	0x25, 0x01,         /*                                      */
	0x95, 0x01,         /*                                      */
	0x81, 0x02,         /*                                      */
	0x95, 0x03,         /*                                      */
	0x81, 0x03,         /*                                      */
	0x75, 0x10,         /*                                      */
	0x95, 0x01,         /*                                      */
	0x14,               /*                                      */
	0xA4,               /*                                      */
	0x05, 0x01,         /*                                      */
	0x65, 0x13,         /*                                      */
	0x55, 0xFD,         /*                                      */
	0x34,               /*                                      */
	0x09, 0x30,         /*                                      */
	0x46, 0xE0, 0x2E,   /*                                      */
	0x26, 0xFF, 0x3F,   /*                                      */
	0x81, 0x02,         /*                                      */
	0x09, 0x31,         /*                                      */
	0x46, 0x52, 0x1C,   /*                                      */
	0x26, 0xFF, 0x3F,   /*                                      */
	0x81, 0x02,         /*                                      */
	0xB4,               /*                                      */
	0x09, 0x30,         /*                                      */
	0x26, 0xFF, 0x03,   /*                                      */
	0x81, 0x02,         /*                                      */
	0xC0,               /*                                      */
	0xC0,               /*                                      */
	0x05, 0x01,         /*                                      */
	0x09, 0x02,         /*                                      */
	0xA1, 0x01,         /*                                      */
	0x85, 0x01,         /*                                      */
	0x09, 0x01,         /*                                      */
	0xA0,               /*                                      */
	0x75, 0x08,         /*                                      */
	0x95, 0x03,         /*                                      */
	0x81, 0x03,         /*                                      */
	0x95, 0x02,         /*                                      */
	0x15, 0xFF,         /*                                      */
	0x25, 0x01,         /*                                      */
	0x09, 0x38,         /*                                      */
	0x0B, 0x38, 0x02,   /*                                      */
		0x0C, 0x00,
	0x81, 0x06,         /*                                      */
	0xC0,               /*                                      */
	0xC0,               /*                                      */
	0x05, 0x0C,         /*                                      */
	0x09, 0x01,         /*                                      */
	0xA1, 0x01,         /*                                      */
	0x85, 0x0D,         /*                                      */
	0x95, 0x01,         /*                                      */
	0x75, 0x10,         /*                                      */
	0x81, 0x03,         /*                                      */
	0x0A, 0x2F, 0x02,   /*                                      */
	0x0A, 0x2E, 0x02,   /*                                      */
	0x0A, 0x2D, 0x02,   /*                                      */
	0x09, 0xB6,         /*                                      */
	0x09, 0xB5,         /*                                      */
	0x08,               /*                                      */
	0x08,               /*                                      */
	0x08,               /*                                      */
	0x08,               /*                                      */
	0x08,               /*                                      */
	0x0A, 0x2E, 0x02,   /*                                      */
	0x0A, 0x2D, 0x02,   /*                                      */
	0x15, 0x0C,         /*                                      */
	0x25, 0x17,         /*                                      */
	0x75, 0x05,         /*                                      */
	0x80,               /*                                      */
	0x75, 0x03,         /*                                      */
	0x81, 0x03,         /*                                      */
	0x75, 0x20,         /*                                      */
	0x81, 0x03,         /*                                      */
	0xC0,               /*                                      */
	0x09, 0x01,         /*                                      */
	0xA1, 0x01,         /*                                      */
	0x85, 0x0C,         /*                                      */
	0x75, 0x01,         /*                                      */
	0x09, 0xE9,         /*                                      */
	0x09, 0xEA,         /*                                      */
	0x09, 0xE2,         /*                                      */
	0x14,               /*                                      */
	0x25, 0x01,         /*                                      */
	0x95, 0x03,         /*                                      */
	0x81, 0x06,         /*                                      */
	0x75, 0x05,         /*                                      */
	0x81, 0x03,         /*                                      */
	0xC0                /*                                      */
};

/*
                                                                                
     
                                                                                 
 */

/*                                                                      */
#define SIRIUS_BATTERY_FREE_TABLET_RDESC_ORIG_SIZE	335

/*                                             */
static __u8 sirius_battery_free_tablet_rdesc_fixed[] = {
	0x05, 0x0D,         /*                                      */
	0x09, 0x02,         /*                                      */
	0xA1, 0x01,         /*                                      */
	0x85, 0x10,         /*                                      */
	0x09, 0x20,         /*                                      */
	0xA0,               /*                                      */
	0x95, 0x01,         /*                                      */
	0x15, 0x01,         /*                                      */
	0x25, 0x03,         /*                                      */
	0x75, 0x02,         /*                                      */
	0x09, 0x42,         /*                                      */
	0x09, 0x44,         /*                                      */
	0x09, 0x46,         /*                                      */
	0x80,               /*                                      */
	0x14,               /*                                      */
	0x25, 0x01,         /*                                      */
	0x75, 0x01,         /*                                      */
	0x09, 0x3C,         /*                                      */
	0x81, 0x02,         /*                                      */
	0x81, 0x03,         /*                                      */
	0x09, 0x32,         /*                                      */
	0x81, 0x02,         /*                                      */
	0x95, 0x03,         /*                                      */
	0x81, 0x03,         /*                                      */
	0xA4,               /*                                      */
	0x05, 0x01,         /*                                      */
	0x55, 0xFD,         /*                                      */
	0x65, 0x13,         /*                                      */
	0x34,               /*                                      */
	0x14,               /*                                      */
	0x75, 0x10,         /*                                      */
	0x95, 0x01,         /*                                      */
	0x46, 0x10, 0x27,   /*                                      */
	0x26, 0x20, 0x4E,   /*                                      */
	0x09, 0x30,         /*                                      */
	0x81, 0x02,         /*                                      */
	0x46, 0x70, 0x17,   /*                                      */
	0x26, 0xE0, 0x2E,   /*                                      */
	0x09, 0x31,         /*                                      */
	0x81, 0x02,         /*                                      */
	0xB4,               /*                                      */
	0x75, 0x10,         /*                                      */
	0x95, 0x01,         /*                                      */
	0x14,               /*                                      */
	0x26, 0xFF, 0x03,   /*                                      */
	0x09, 0x30,         /*                                      */
	0x81, 0x02,         /*                                      */
	0xA4,               /*                                      */
	0x55, 0xFE,         /*                                      */
	0x65, 0x12,         /*                                      */
	0x35, 0x97,         /*                                      */
	0x45, 0x69,         /*                                      */
	0x15, 0x97,         /*                                      */
	0x25, 0x69,         /*                                      */
	0x75, 0x08,         /*                                      */
	0x95, 0x02,         /*                                      */
	0x09, 0x3D,         /*                                      */
	0x09, 0x3E,         /*                                      */
	0x81, 0x02,         /*                                      */
	0xB4,               /*                                      */
	0xC0,               /*                                      */
	0xC0,               /*                                      */
	0x05, 0x01,         /*                                      */
	0x09, 0x02,         /*                                      */
	0xA1, 0x01,         /*                                      */
	0x85, 0x01,         /*                                      */
	0x09, 0x01,         /*                                      */
	0xA0,               /*                                      */
	0x75, 0x08,         /*                                      */
	0x95, 0x03,         /*                                      */
	0x81, 0x03,         /*                                      */
	0x09, 0x38,         /*                                      */
	0x15, 0xFF,         /*                                      */
	0x25, 0x01,         /*                                      */
	0x75, 0x08,         /*                                      */
	0x95, 0x01,         /*                                      */
	0x81, 0x06,         /*                                      */
	0x75, 0x08,         /*                                      */
	0x95, 0x03,         /*                                      */
	0x81, 0x03,         /*                                      */
	0xC0,               /*                                      */
	0xC0,               /*                                      */
	0x05, 0x01,         /*                                      */
	0x09, 0x06,         /*                                      */
	0xA1, 0x01,         /*                                      */
	0x85, 0x0D,         /*                                      */
	0x05, 0x07,         /*                                      */
	0x19, 0xE0,         /*                                      */
	0x29, 0xE7,         /*                                      */
	0x14,               /*                                      */
	0x25, 0x01,         /*                                      */
	0x75, 0x01,         /*                                      */
	0x95, 0x08,         /*                                      */
	0x81, 0x02,         /*                                      */
	0x75, 0x08,         /*                                      */
	0x95, 0x01,         /*                                      */
	0x81, 0x01,         /*                                      */
	0x18,               /*                                      */
	0x29, 0x65,         /*                                      */
	0x14,               /*                                      */
	0x25, 0x65,         /*                                      */
	0x75, 0x08,         /*                                      */
	0x95, 0x05,         /*                                      */
	0x80,               /*                                      */
	0xC0,               /*                                      */
	0x05, 0x0C,         /*                                      */
	0x09, 0x01,         /*                                      */
	0xA1, 0x01,         /*                                      */
	0x85, 0x0C,         /*                                      */
	0x09, 0xE9,         /*                                      */
	0x09, 0xEA,         /*                                      */
	0x14,               /*                                      */
	0x25, 0x01,         /*                                      */
	0x75, 0x01,         /*                                      */
	0x95, 0x02,         /*                                      */
	0x81, 0x02,         /*                                      */
	0x75, 0x06,         /*                                      */
	0x95, 0x01,         /*                                      */
	0x81, 0x03,         /*                                      */
	0x75, 0x10,         /*                                      */
	0x95, 0x03,         /*                                      */
	0x81, 0x03,         /*                                      */
	0xC0                /*                                      */
};

static __u8 *waltop_report_fixup(struct hid_device *hdev, __u8 *rdesc,
		unsigned int *rsize)
{
	switch (hdev->product) {
	case USB_DEVICE_ID_WALTOP_SLIM_TABLET_5_8_INCH:
		if (*rsize == SLIM_TABLET_5_8_INCH_RDESC_ORIG_SIZE) {
			rdesc = slim_tablet_5_8_inch_rdesc_fixed;
			*rsize = sizeof(slim_tablet_5_8_inch_rdesc_fixed);
		}
		break;
	case USB_DEVICE_ID_WALTOP_SLIM_TABLET_12_1_INCH:
		if (*rsize == SLIM_TABLET_12_1_INCH_RDESC_ORIG_SIZE) {
			rdesc = slim_tablet_12_1_inch_rdesc_fixed;
			*rsize = sizeof(slim_tablet_12_1_inch_rdesc_fixed);
		}
		break;
	case USB_DEVICE_ID_WALTOP_Q_PAD:
		if (*rsize == Q_PAD_RDESC_ORIG_SIZE) {
			rdesc = q_pad_rdesc_fixed;
			*rsize = sizeof(q_pad_rdesc_fixed);
		}
		break;
	case USB_DEVICE_ID_WALTOP_PID_0038:
		if (*rsize == PID_0038_RDESC_ORIG_SIZE) {
			rdesc = pid_0038_rdesc_fixed;
			*rsize = sizeof(pid_0038_rdesc_fixed);
		}
		break;
	case USB_DEVICE_ID_WALTOP_MEDIA_TABLET_10_6_INCH:
		if (*rsize == MEDIA_TABLET_10_6_INCH_RDESC_ORIG_SIZE) {
			rdesc = media_tablet_10_6_inch_rdesc_fixed;
			*rsize = sizeof(media_tablet_10_6_inch_rdesc_fixed);
		}
		break;
	case USB_DEVICE_ID_WALTOP_MEDIA_TABLET_14_1_INCH:
		if (*rsize == MEDIA_TABLET_14_1_INCH_RDESC_ORIG_SIZE) {
			rdesc = media_tablet_14_1_inch_rdesc_fixed;
			*rsize = sizeof(media_tablet_14_1_inch_rdesc_fixed);
		}
		break;
	case USB_DEVICE_ID_WALTOP_SIRIUS_BATTERY_FREE_TABLET:
		if (*rsize == SIRIUS_BATTERY_FREE_TABLET_RDESC_ORIG_SIZE) {
			rdesc = sirius_battery_free_tablet_rdesc_fixed;
			*rsize = sizeof(sirius_battery_free_tablet_rdesc_fixed);
		}
		break;
	}
	return rdesc;
}

static int waltop_raw_event(struct hid_device *hdev, struct hid_report *report,
		     u8 *data, int size)
{
	/*                               */
	if (report->type == HID_INPUT_REPORT && report->id == 16 && size >= 8) {
		/*
                                                              
                                  
   */

		/*                               */
		if ((data[1] & 0xF) > 1) {
			/*                      */
			data[6] = 0;
			data[7] = 0;
		}
	}

	/*                                                             */
	if (hdev->product == USB_DEVICE_ID_WALTOP_SIRIUS_BATTERY_FREE_TABLET &&
	    report->type == HID_INPUT_REPORT &&
	    report->id == 16 &&
	    size == 10) {
		/*
                                                              
              
    
                                                                
                                                                
   */
		static const s8 tilt_to_radians[] = {
			0, 5, 10, 14, 19, 24, 29, 34, 40, 45,
			50, 56, 62, 68, 74, 81, 88, 96, 105
		};

		s8 tilt_x = (s8)data[8];
		s8 tilt_y = (s8)data[9];
		s8 sign_x = tilt_x >= 0 ? 1 : -1;
		s8 sign_y = tilt_y >= 0 ? 1 : -1;

		tilt_x *= sign_x;
		tilt_y *= sign_y;

		/*
                                                               
                                                              
                      
   */
		sign_y *= -1;

		/*
                                                              
                                
   */
		if (tilt_x > ARRAY_SIZE(tilt_to_radians) - 1)
			tilt_x = ARRAY_SIZE(tilt_to_radians) - 1;
		if (tilt_y > ARRAY_SIZE(tilt_to_radians) - 1)
			tilt_y = ARRAY_SIZE(tilt_to_radians) - 1;

		data[8] = tilt_to_radians[tilt_x] * sign_x;
		data[9] = tilt_to_radians[tilt_y] * sign_y;
	}

	return 0;
}

static const struct hid_device_id waltop_devices[] = {
	{ HID_USB_DEVICE(USB_VENDOR_ID_WALTOP,
				USB_DEVICE_ID_WALTOP_SLIM_TABLET_5_8_INCH) },
	{ HID_USB_DEVICE(USB_VENDOR_ID_WALTOP,
				USB_DEVICE_ID_WALTOP_SLIM_TABLET_12_1_INCH) },
	{ HID_USB_DEVICE(USB_VENDOR_ID_WALTOP,
				USB_DEVICE_ID_WALTOP_Q_PAD) },
	{ HID_USB_DEVICE(USB_VENDOR_ID_WALTOP,
				USB_DEVICE_ID_WALTOP_PID_0038) },
	{ HID_USB_DEVICE(USB_VENDOR_ID_WALTOP,
				USB_DEVICE_ID_WALTOP_MEDIA_TABLET_10_6_INCH) },
	{ HID_USB_DEVICE(USB_VENDOR_ID_WALTOP,
				USB_DEVICE_ID_WALTOP_MEDIA_TABLET_14_1_INCH) },
	{ HID_USB_DEVICE(USB_VENDOR_ID_WALTOP,
			 USB_DEVICE_ID_WALTOP_SIRIUS_BATTERY_FREE_TABLET) },
	{ }
};
MODULE_DEVICE_TABLE(hid, waltop_devices);

static struct hid_driver waltop_driver = {
	.name = "waltop",
	.id_table = waltop_devices,
	.report_fixup = waltop_report_fixup,
	.raw_event = waltop_raw_event,
};
module_hid_driver(waltop_driver);

MODULE_LICENSE("GPL");
