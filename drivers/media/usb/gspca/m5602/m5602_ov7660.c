/*
 * Driver for the ov7660 sensor
 *
 * Copyright (C) 2009 Erik Andrén
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

#include "m5602_ov7660.h"

static int ov7660_s_ctrl(struct v4l2_ctrl *ctrl);
static void ov7660_dump_registers(struct sd *sd);

static struct v4l2_pix_format ov7660_modes[] = {
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

static const struct v4l2_ctrl_ops ov7660_ctrl_ops = {
	.s_ctrl = ov7660_s_ctrl,
};

int ov7660_probe(struct sd *sd)
{
	int err = 0, i;
	u8 prod_id = 0, ver_id = 0;

	if (force_sensor) {
		if (force_sensor == OV7660_SENSOR) {
			pr_info("Forcing an %s sensor\n", ov7660.name);
			goto sensor_found;
		}
		/*                                    
                              */
		return -ENODEV;
	}

	/*                */
	for (i = 0; i < ARRAY_SIZE(preinit_ov7660) && !err; i++) {
		u8 data[2];

		if (preinit_ov7660[i][0] == BRIDGE) {
			err = m5602_write_bridge(sd,
				preinit_ov7660[i][1],
				preinit_ov7660[i][2]);
		} else {
			data[0] = preinit_ov7660[i][2];
			err = m5602_write_sensor(sd,
				preinit_ov7660[i][1], data, 1);
		}
	}
	if (err < 0)
		return err;

	if (m5602_read_sensor(sd, OV7660_PID, &prod_id, 1))
		return -ENODEV;

	if (m5602_read_sensor(sd, OV7660_VER, &ver_id, 1))
		return -ENODEV;

	pr_info("Sensor reported 0x%x%x\n", prod_id, ver_id);

	if ((prod_id == 0x76) && (ver_id == 0x60)) {
		pr_info("Detected a ov7660 sensor\n");
		goto sensor_found;
	}
	return -ENODEV;

sensor_found:
	sd->gspca_dev.cam.cam_mode = ov7660_modes;
	sd->gspca_dev.cam.nmodes = ARRAY_SIZE(ov7660_modes);

	return 0;
}

int ov7660_init(struct sd *sd)
{
	int i, err;

	/*                 */
	for (i = 0; i < ARRAY_SIZE(init_ov7660); i++) {
		u8 data[2];

		if (init_ov7660[i][0] == BRIDGE) {
			err = m5602_write_bridge(sd,
				init_ov7660[i][1],
				init_ov7660[i][2]);
		} else {
			data[0] = init_ov7660[i][2];
			err = m5602_write_sensor(sd,
				init_ov7660[i][1], data, 1);
		}
		if (err < 0)
			return err;
	}

	if (dump_sensor)
		ov7660_dump_registers(sd);

	return 0;
}

int ov7660_init_controls(struct sd *sd)
{
	struct v4l2_ctrl_handler *hdl = &sd->gspca_dev.ctrl_handler;

	sd->gspca_dev.vdev.ctrl_handler = hdl;
	v4l2_ctrl_handler_init(hdl, 6);

	v4l2_ctrl_new_std(hdl, &ov7660_ctrl_ops, V4L2_CID_AUTO_WHITE_BALANCE,
			  0, 1, 1, 1);
	v4l2_ctrl_new_std_menu(hdl, &ov7660_ctrl_ops,
			  V4L2_CID_EXPOSURE_AUTO, 1, 0, V4L2_EXPOSURE_AUTO);

	sd->autogain = v4l2_ctrl_new_std(hdl, &ov7660_ctrl_ops,
					 V4L2_CID_AUTOGAIN, 0, 1, 1, 1);
	sd->gain = v4l2_ctrl_new_std(hdl, &ov7660_ctrl_ops, V4L2_CID_GAIN, 0,
				     255, 1, OV7660_DEFAULT_GAIN);

	sd->hflip = v4l2_ctrl_new_std(hdl, &ov7660_ctrl_ops, V4L2_CID_HFLIP,
				      0, 1, 1, 0);
	sd->vflip = v4l2_ctrl_new_std(hdl, &ov7660_ctrl_ops, V4L2_CID_VFLIP,
				      0, 1, 1, 0);

	if (hdl->error) {
		pr_err("Could not initialize controls\n");
		return hdl->error;
	}

	v4l2_ctrl_auto_cluster(2, &sd->autogain, 0, false);
	v4l2_ctrl_cluster(2, &sd->hflip);

	return 0;
}

int ov7660_start(struct sd *sd)
{
	return 0;
}

int ov7660_stop(struct sd *sd)
{
	return 0;
}

void ov7660_disconnect(struct sd *sd)
{
	ov7660_stop(sd);

	sd->sensor = NULL;
}

static int ov7660_set_gain(struct gspca_dev *gspca_dev, __s32 val)
{
	int err;
	u8 i2c_data = val;
	struct sd *sd = (struct sd *) gspca_dev;

	PDEBUG(D_CONF, "Setting gain to %d", val);

	err = m5602_write_sensor(sd, OV7660_GAIN, &i2c_data, 1);
	return err;
}

static int ov7660_set_auto_white_balance(struct gspca_dev *gspca_dev,
					 __s32 val)
{
	int err;
	u8 i2c_data;
	struct sd *sd = (struct sd *) gspca_dev;

	PDEBUG(D_CONF, "Set auto white balance to %d", val);

	err = m5602_read_sensor(sd, OV7660_COM8, &i2c_data, 1);
	if (err < 0)
		return err;

	i2c_data = ((i2c_data & 0xfd) | ((val & 0x01) << 1));
	err = m5602_write_sensor(sd, OV7660_COM8, &i2c_data, 1);

	return err;
}

static int ov7660_set_auto_gain(struct gspca_dev *gspca_dev, __s32 val)
{
	int err;
	u8 i2c_data;
	struct sd *sd = (struct sd *) gspca_dev;

	PDEBUG(D_CONF, "Set auto gain control to %d", val);

	err = m5602_read_sensor(sd, OV7660_COM8, &i2c_data, 1);
	if (err < 0)
		return err;

	i2c_data = ((i2c_data & 0xfb) | ((val & 0x01) << 2));

	return m5602_write_sensor(sd, OV7660_COM8, &i2c_data, 1);
}

static int ov7660_set_auto_exposure(struct gspca_dev *gspca_dev,
				    __s32 val)
{
	int err;
	u8 i2c_data;
	struct sd *sd = (struct sd *) gspca_dev;

	PDEBUG(D_CONF, "Set auto exposure control to %d", val);

	err = m5602_read_sensor(sd, OV7660_COM8, &i2c_data, 1);
	if (err < 0)
		return err;

	val = (val == V4L2_EXPOSURE_AUTO);
	i2c_data = ((i2c_data & 0xfe) | ((val & 0x01) << 0));

	return m5602_write_sensor(sd, OV7660_COM8, &i2c_data, 1);
}

static int ov7660_set_hvflip(struct gspca_dev *gspca_dev)
{
	int err;
	u8 i2c_data;
	struct sd *sd = (struct sd *) gspca_dev;

	PDEBUG(D_CONF, "Set hvflip to %d, %d", sd->hflip->val, sd->vflip->val);

	i2c_data = (sd->hflip->val << 5) | (sd->vflip->val << 4);

	err = m5602_write_sensor(sd, OV7660_MVFP, &i2c_data, 1);

	return err;
}

static int ov7660_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct gspca_dev *gspca_dev =
		container_of(ctrl->handler, struct gspca_dev, ctrl_handler);
	struct sd *sd = (struct sd *) gspca_dev;
	int err;

	if (!gspca_dev->streaming)
		return 0;

	switch (ctrl->id) {
	case V4L2_CID_AUTO_WHITE_BALANCE:
		err = ov7660_set_auto_white_balance(gspca_dev, ctrl->val);
		break;
	case V4L2_CID_EXPOSURE_AUTO:
		err = ov7660_set_auto_exposure(gspca_dev, ctrl->val);
		break;
	case V4L2_CID_AUTOGAIN:
		err = ov7660_set_auto_gain(gspca_dev, ctrl->val);
		if (err || ctrl->val)
			return err;
		err = ov7660_set_gain(gspca_dev, sd->gain->val);
		break;
	case V4L2_CID_HFLIP:
		err = ov7660_set_hvflip(gspca_dev);
		break;
	default:
		return -EINVAL;
	}

	return err;
}

static void ov7660_dump_registers(struct sd *sd)
{
	int address;
	pr_info("Dumping the ov7660 register state\n");
	for (address = 0; address < 0xa9; address++) {
		u8 value;
		m5602_read_sensor(sd, address, &value, 1);
		pr_info("register 0x%x contains 0x%x\n", address, value);
	}

	pr_info("ov7660 register state dump complete\n");

	pr_info("Probing for which registers that are read/write\n");
	for (address = 0; address < 0xff; address++) {
		u8 old_value, ctrl_value;
		u8 test_value[2] = {0xff, 0xff};

		m5602_read_sensor(sd, address, &old_value, 1);
		m5602_write_sensor(sd, address, test_value, 1);
		m5602_read_sensor(sd, address, &ctrl_value, 1);

		if (ctrl_value == test_value[0])
			pr_info("register 0x%x is writeable\n", address);
		else
			pr_info("register 0x%x is read only\n", address);

		/*                        */
		m5602_write_sensor(sd, address, &old_value, 1);
	}
}
