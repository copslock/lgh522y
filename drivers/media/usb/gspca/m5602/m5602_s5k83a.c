/*
 * Driver for the s5k83a sensor
 *
 * Copyright (C) 2008 Erik Andrén
 * Copyright (C) 2007 Ilyes Gouta. Based on the m5603x Linux Driver Project.
 * Copyright (C) 2005 m5603x Linux Driver Project <m5602@x3ng.com.br>
 *
 * Portions of code to USB interface and ALi driver software,
 * Copyright (c) 2006 Willem Duinker
 * v4l2 interface modeled after the V4L2 driver
 * for SN9C10x PC Camera Controllers
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2.
 *
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kthread.h>
#include "m5602_s5k83a.h"

static int s5k83a_s_ctrl(struct v4l2_ctrl *ctrl);

static const struct v4l2_ctrl_ops s5k83a_ctrl_ops = {
	.s_ctrl = s5k83a_s_ctrl,
};

static struct v4l2_pix_format s5k83a_modes[] = {
	{
		640,
		480,
		V4L2_PIX_FMT_SBGGR8,
		V4L2_FIELD_NONE,
		.sizeimage =
			640 * 480,
		.bytesperline = 640,
		.colorspace = V4L2_COLORSPACE_SRGB,
		.priv = 0
	}
};

static void s5k83a_dump_registers(struct sd *sd);
static int s5k83a_get_rotation(struct sd *sd, u8 *reg_data);
static int s5k83a_set_led_indication(struct sd *sd, u8 val);
static int s5k83a_set_flip_real(struct gspca_dev *gspca_dev,
				__s32 vflip, __s32 hflip);

int s5k83a_probe(struct sd *sd)
{
	u8 prod_id = 0, ver_id = 0;
	int i, err = 0;
	struct gspca_dev *gspca_dev = (struct gspca_dev *)sd;

	if (force_sensor) {
		if (force_sensor == S5K83A_SENSOR) {
			pr_info("Forcing a %s sensor\n", s5k83a.name);
			goto sensor_found;
		}
		/*                                                            
         */
		return -ENODEV;
	}

	PDEBUG(D_PROBE, "Probing for a s5k83a sensor");

	/*                    */
	for (i = 0; i < ARRAY_SIZE(preinit_s5k83a) && !err; i++) {
		u8 data[2] = {preinit_s5k83a[i][2], preinit_s5k83a[i][3]};
		if (preinit_s5k83a[i][0] == SENSOR)
			err = m5602_write_sensor(sd, preinit_s5k83a[i][1],
				data, 2);
		else
			err = m5602_write_bridge(sd, preinit_s5k83a[i][1],
				data[0]);
	}

	/*                                                                 
                                                                       
                         */
	if (m5602_read_sensor(sd, 0x00, &prod_id, 1))
		return -ENODEV;

	if (m5602_read_sensor(sd, 0x01, &ver_id, 1))
		return -ENODEV;

	if ((prod_id == 0xff) || (ver_id == 0xff))
		return -ENODEV;
	else
		pr_info("Detected a s5k83a sensor\n");

sensor_found:
	sd->gspca_dev.cam.cam_mode = s5k83a_modes;
	sd->gspca_dev.cam.nmodes = ARRAY_SIZE(s5k83a_modes);

	/*                                           */
	sd->rotation_thread = NULL;

	return 0;
}

int s5k83a_init(struct sd *sd)
{
	int i, err = 0;

	for (i = 0; i < ARRAY_SIZE(init_s5k83a) && !err; i++) {
		u8 data[2] = {0x00, 0x00};

		switch (init_s5k83a[i][0]) {
		case BRIDGE:
			err = m5602_write_bridge(sd,
					init_s5k83a[i][1],
					init_s5k83a[i][2]);
			break;

		case SENSOR:
			data[0] = init_s5k83a[i][2];
			err = m5602_write_sensor(sd,
				init_s5k83a[i][1], data, 1);
			break;

		case SENSOR_LONG:
			data[0] = init_s5k83a[i][2];
			data[1] = init_s5k83a[i][3];
			err = m5602_write_sensor(sd,
				init_s5k83a[i][1], data, 2);
			break;
		default:
			pr_info("Invalid stream command, exiting init\n");
			return -EINVAL;
		}
	}

	if (dump_sensor)
		s5k83a_dump_registers(sd);

	return err;
}

int s5k83a_init_controls(struct sd *sd)
{
	struct v4l2_ctrl_handler *hdl = &sd->gspca_dev.ctrl_handler;

	sd->gspca_dev.vdev.ctrl_handler = hdl;
	v4l2_ctrl_handler_init(hdl, 6);

	v4l2_ctrl_new_std(hdl, &s5k83a_ctrl_ops, V4L2_CID_BRIGHTNESS,
			  0, 255, 1, S5K83A_DEFAULT_BRIGHTNESS);

	v4l2_ctrl_new_std(hdl, &s5k83a_ctrl_ops, V4L2_CID_EXPOSURE,
			  0, S5K83A_MAXIMUM_EXPOSURE, 1,
			  S5K83A_DEFAULT_EXPOSURE);

	v4l2_ctrl_new_std(hdl, &s5k83a_ctrl_ops, V4L2_CID_GAIN,
			  0, 255, 1, S5K83A_DEFAULT_GAIN);

	sd->hflip = v4l2_ctrl_new_std(hdl, &s5k83a_ctrl_ops, V4L2_CID_HFLIP,
				      0, 1, 1, 0);
	sd->vflip = v4l2_ctrl_new_std(hdl, &s5k83a_ctrl_ops, V4L2_CID_VFLIP,
				      0, 1, 1, 0);

	if (hdl->error) {
		pr_err("Could not initialize controls\n");
		return hdl->error;
	}

	v4l2_ctrl_cluster(2, &sd->hflip);

	return 0;
}

static int rotation_thread_function(void *data)
{
	struct sd *sd = (struct sd *) data;
	u8 reg, previous_rotation = 0;
	__s32 vflip, hflip;

	set_current_state(TASK_INTERRUPTIBLE);
	while (!schedule_timeout(100)) {
		if (mutex_lock_interruptible(&sd->gspca_dev.usb_lock))
			break;

		s5k83a_get_rotation(sd, &reg);
		if (previous_rotation != reg) {
			previous_rotation = reg;
			pr_info("Camera was flipped\n");

			hflip = sd->hflip->val;
			vflip = sd->vflip->val;

			if (reg) {
				vflip = !vflip;
				hflip = !hflip;
			}
			s5k83a_set_flip_real((struct gspca_dev *) sd,
					      vflip, hflip);
		}

		mutex_unlock(&sd->gspca_dev.usb_lock);
		set_current_state(TASK_INTERRUPTIBLE);
	}

	/*                        */
	if (previous_rotation) {
		hflip = sd->hflip->val;
		vflip = sd->vflip->val;
		s5k83a_set_flip_real((struct gspca_dev *) sd, vflip, hflip);
	}

	sd->rotation_thread = NULL;
	return 0;
}

int s5k83a_start(struct sd *sd)
{
	int i, err = 0;

	/*                                                                     
                                                                        
                                                                */
	sd->rotation_thread = kthread_create(rotation_thread_function,
					     sd, "rotation thread");
	wake_up_process(sd->rotation_thread);

	/*                    */
	for (i = 0; i < ARRAY_SIZE(start_s5k83a) && !err; i++) {
		u8 data[2] = {start_s5k83a[i][2], start_s5k83a[i][3]};
		if (start_s5k83a[i][0] == SENSOR)
			err = m5602_write_sensor(sd, start_s5k83a[i][1],
				data, 2);
		else
			err = m5602_write_bridge(sd, start_s5k83a[i][1],
				data[0]);
	}
	if (err < 0)
		return err;

	return s5k83a_set_led_indication(sd, 1);
}

int s5k83a_stop(struct sd *sd)
{
	if (sd->rotation_thread)
		kthread_stop(sd->rotation_thread);

	return s5k83a_set_led_indication(sd, 0);
}

void s5k83a_disconnect(struct sd *sd)
{
	s5k83a_stop(sd);

	sd->sensor = NULL;
}

static int s5k83a_set_gain(struct gspca_dev *gspca_dev, __s32 val)
{
	int err;
	u8 data[2];
	struct sd *sd = (struct sd *) gspca_dev;

	data[0] = 0x00;
	data[1] = 0x20;
	err = m5602_write_sensor(sd, 0x14, data, 2);
	if (err < 0)
		return err;

	data[0] = 0x01;
	data[1] = 0x00;
	err = m5602_write_sensor(sd, 0x0d, data, 2);
	if (err < 0)
		return err;

	/*                                                               
                       */
	data[0] = val >> 3; /*                   */
	data[1] = val >> 1; /*                   */
	err = m5602_write_sensor(sd, S5K83A_GAIN, data, 2);

	return err;
}

static int s5k83a_set_brightness(struct gspca_dev *gspca_dev, __s32 val)
{
	int err;
	u8 data[1];
	struct sd *sd = (struct sd *) gspca_dev;

	data[0] = val;
	err = m5602_write_sensor(sd, S5K83A_BRIGHTNESS, data, 1);
	return err;
}

static int s5k83a_set_exposure(struct gspca_dev *gspca_dev, __s32 val)
{
	int err;
	u8 data[2];
	struct sd *sd = (struct sd *) gspca_dev;

	data[0] = 0;
	data[1] = val;
	err = m5602_write_sensor(sd, S5K83A_EXPOSURE, data, 2);
	return err;
}

static int s5k83a_set_flip_real(struct gspca_dev *gspca_dev,
				__s32 vflip, __s32 hflip)
{
	int err;
	u8 data[1];
	struct sd *sd = (struct sd *) gspca_dev;

	data[0] = 0x05;
	err = m5602_write_sensor(sd, S5K83A_PAGE_MAP, data, 1);
	if (err < 0)
		return err;

	/*                                  */
	data[0] = S5K83A_FLIP_MASK;
	data[0] = (vflip) ? data[0] | 0x40 : data[0];
	data[0] = (hflip) ? data[0] | 0x80 : data[0];

	err = m5602_write_sensor(sd, S5K83A_FLIP, data, 1);
	if (err < 0)
		return err;

	data[0] = (vflip) ? 0x0b : 0x0a;
	err = m5602_write_sensor(sd, S5K83A_VFLIP_TUNE, data, 1);
	if (err < 0)
		return err;

	data[0] = (hflip) ? 0x0a : 0x0b;
	err = m5602_write_sensor(sd, S5K83A_HFLIP_TUNE, data, 1);
	return err;
}

static int s5k83a_set_hvflip(struct gspca_dev *gspca_dev)
{
	int err;
	u8 reg;
	struct sd *sd = (struct sd *) gspca_dev;
	int hflip = sd->hflip->val;
	int vflip = sd->vflip->val;

	err = s5k83a_get_rotation(sd, &reg);
	if (err < 0)
		return err;
	if (reg) {
		hflip = !hflip;
		vflip = !vflip;
	}

	err = s5k83a_set_flip_real(gspca_dev, vflip, hflip);
	return err;
}

static int s5k83a_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct gspca_dev *gspca_dev =
		container_of(ctrl->handler, struct gspca_dev, ctrl_handler);
	int err;

	if (!gspca_dev->streaming)
		return 0;

	switch (ctrl->id) {
	case V4L2_CID_BRIGHTNESS:
		err = s5k83a_set_brightness(gspca_dev, ctrl->val);
		break;
	case V4L2_CID_EXPOSURE:
		err = s5k83a_set_exposure(gspca_dev, ctrl->val);
		break;
	case V4L2_CID_GAIN:
		err = s5k83a_set_gain(gspca_dev, ctrl->val);
		break;
	case V4L2_CID_HFLIP:
		err = s5k83a_set_hvflip(gspca_dev);
		break;
	default:
		return -EINVAL;
	}

	return err;
}

static int s5k83a_set_led_indication(struct sd *sd, u8 val)
{
	int err = 0;
	u8 data[1];

	err = m5602_read_bridge(sd, M5602_XB_GPIO_DAT, data);
	if (err < 0)
		return err;

	if (val)
		data[0] = data[0] | S5K83A_GPIO_LED_MASK;
	else
		data[0] = data[0] & ~S5K83A_GPIO_LED_MASK;

	err = m5602_write_bridge(sd, M5602_XB_GPIO_DAT, data[0]);

	return err;
}

/*                                       */
static int s5k83a_get_rotation(struct sd *sd, u8 *reg_data)
{
	int err = m5602_read_bridge(sd, M5602_XB_GPIO_DAT, reg_data);
	*reg_data = (*reg_data & S5K83A_GPIO_ROTATION_MASK) ? 0 : 1;
	return err;
}

static void s5k83a_dump_registers(struct sd *sd)
{
	int address;
	u8 page, old_page;
	m5602_read_sensor(sd, S5K83A_PAGE_MAP, &old_page, 1);

	for (page = 0; page < 16; page++) {
		m5602_write_sensor(sd, S5K83A_PAGE_MAP, &page, 1);
		pr_info("Dumping the s5k83a register state for page 0x%x\n",
			page);
		for (address = 0; address <= 0xff; address++) {
			u8 val = 0;
			m5602_read_sensor(sd, address, &val, 1);
			pr_info("register 0x%x contains 0x%x\n", address, val);
		}
	}
	pr_info("s5k83a register state dump complete\n");

	for (page = 0; page < 16; page++) {
		m5602_write_sensor(sd, S5K83A_PAGE_MAP, &page, 1);
		pr_info("Probing for which registers that are read/write for page 0x%x\n",
			page);
		for (address = 0; address <= 0xff; address++) {
			u8 old_val, ctrl_val, test_val = 0xff;

			m5602_read_sensor(sd, address, &old_val, 1);
			m5602_write_sensor(sd, address, &test_val, 1);
			m5602_read_sensor(sd, address, &ctrl_val, 1);

			if (ctrl_val == test_val)
				pr_info("register 0x%x is writeable\n",
					address);
			else
				pr_info("register 0x%x is read only\n",
					address);

			/*                      */
			m5602_write_sensor(sd, address, &old_val, 1);
		}
	}
	pr_info("Read/write register probing complete\n");
	m5602_write_sensor(sd, S5K83A_PAGE_MAP, &old_page, 1);
}
