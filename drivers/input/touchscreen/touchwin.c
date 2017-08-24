/*
 * Touchwindow serial touchscreen driver
 *
 * Copyright (c) 2006 Rick Koch <n1gp@hotmail.com>
 *
 * Based on MicroTouch driver (drivers/input/touchscreen/mtouch.c)
 * Copyright (c) 2004 Vojtech Pavlik
 * and Dan Streetman <ddstreet@ieee.org>
 */

/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

/*
                        
                                                       
                                                             
                                                    
 */

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/serio.h>
#include <linux/init.h>

#define DRIVER_DESC	"Touchwindow serial touchscreen driver"

MODULE_AUTHOR("Rick Koch <n1gp@hotmail.com>");
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

/*
                               
 */

#define TW_LENGTH 3

#define TW_MIN_XC 0
#define TW_MAX_XC 0xff
#define TW_MIN_YC 0
#define TW_MAX_YC 0xff

/*
                        
 */

struct tw {
	struct input_dev *dev;
	struct serio *serio;
	int idx;
	int touched;
	unsigned char data[TW_LENGTH];
	char phys[32];
};

static irqreturn_t tw_interrupt(struct serio *serio,
		unsigned char data, unsigned int flags)
{
	struct tw *tw = serio_get_drvdata(serio);
	struct input_dev *dev = tw->dev;

	if (data) {		/*       */
		tw->touched = 1;
		tw->data[tw->idx++] = data;
		/*                                                 */
		if (tw->idx == TW_LENGTH && tw->data[1] == tw->data[2]) {
			input_report_abs(dev, ABS_X, tw->data[0]);
			input_report_abs(dev, ABS_Y, tw->data[1]);
			input_report_key(dev, BTN_TOUCH, 1);
			input_sync(dev);
			tw->idx = 0;
		}
	} else if (tw->touched) {	/*         */
		input_report_key(dev, BTN_TOUCH, 0);
		input_sync(dev);
		tw->idx = 0;
		tw->touched = 0;
	}

	return IRQ_HANDLED;
}

/*
                                                  
 */

static void tw_disconnect(struct serio *serio)
{
	struct tw *tw = serio_get_drvdata(serio);

	input_get_device(tw->dev);
	input_unregister_device(tw->dev);
	serio_close(serio);
	serio_set_drvdata(serio, NULL);
	input_put_device(tw->dev);
	kfree(tw);
}

/*
                                                                 
                                                                           
                   
 */

static int tw_connect(struct serio *serio, struct serio_driver *drv)
{
	struct tw *tw;
	struct input_dev *input_dev;
	int err;

	tw = kzalloc(sizeof(struct tw), GFP_KERNEL);
	input_dev = input_allocate_device();
	if (!tw || !input_dev) {
		err = -ENOMEM;
		goto fail1;
	}

	tw->serio = serio;
	tw->dev = input_dev;
	snprintf(tw->phys, sizeof(tw->phys), "%s/input0", serio->phys);

	input_dev->name = "Touchwindow Serial TouchScreen";
	input_dev->phys = tw->phys;
	input_dev->id.bustype = BUS_RS232;
	input_dev->id.vendor = SERIO_TOUCHWIN;
	input_dev->id.product = 0;
	input_dev->id.version = 0x0100;
	input_dev->dev.parent = &serio->dev;
	input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);
	input_dev->keybit[BIT_WORD(BTN_TOUCH)] = BIT_MASK(BTN_TOUCH);
	input_set_abs_params(tw->dev, ABS_X, TW_MIN_XC, TW_MAX_XC, 0, 0);
	input_set_abs_params(tw->dev, ABS_Y, TW_MIN_YC, TW_MAX_YC, 0, 0);

	serio_set_drvdata(serio, tw);

	err = serio_open(serio, drv);
	if (err)
		goto fail2;

	err = input_register_device(tw->dev);
	if (err)
		goto fail3;

	return 0;

 fail3:	serio_close(serio);
 fail2:	serio_set_drvdata(serio, NULL);
 fail1:	input_free_device(input_dev);
	kfree(tw);
	return err;
}

/*
                              
 */

static struct serio_device_id tw_serio_ids[] = {
	{
		.type	= SERIO_RS232,
		.proto	= SERIO_TOUCHWIN,
		.id	= SERIO_ANY,
		.extra	= SERIO_ANY,
	},
	{ 0 }
};

MODULE_DEVICE_TABLE(serio, tw_serio_ids);

static struct serio_driver tw_drv = {
	.driver		= {
		.name	= "touchwin",
	},
	.description	= DRIVER_DESC,
	.id_table	= tw_serio_ids,
	.interrupt	= tw_interrupt,
	.connect	= tw_connect,
	.disconnect	= tw_disconnect,
};

module_serio_driver(tw_drv);
