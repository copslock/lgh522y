/*
 * An I2C driver for the Intersil ISL 12022
 *
 * Author: Roman Fietze <roman.fietze@telemotive.de>
 *
 * Based on the Philips PCF8563 RTC
 * by Alessandro Zummo <a.zummo@towertech.it>.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 */

#include <linux/i2c.h>
#include <linux/bcd.h>
#include <linux/rtc.h>
#include <linux/slab.h>
#include <linux/module.h>

#define DRV_VERSION "0.1"

/*                      */
#define ISL12022_REG_SC		0x00
#define ISL12022_REG_MN		0x01
#define ISL12022_REG_HR		0x02
#define ISL12022_REG_DT		0x03
#define ISL12022_REG_MO		0x04
#define ISL12022_REG_YR		0x05
#define ISL12022_REG_DW		0x06

#define ISL12022_REG_SR		0x07
#define ISL12022_REG_INT	0x08

/*                   */
#define ISL12022_HR_MIL		(1 << 7)	/*                          */

#define ISL12022_SR_LBAT85	(1 << 2)
#define ISL12022_SR_LBAT75	(1 << 1)

#define ISL12022_INT_WRTC	(1 << 6)


static struct i2c_driver isl12022_driver;

struct isl12022 {
	struct rtc_device *rtc;

	bool write_enabled;	/*                             */
};


static int isl12022_read_regs(struct i2c_client *client, uint8_t reg,
			      uint8_t *data, size_t n)
{
	struct i2c_msg msgs[] = {
		{
			.addr	= client->addr,
			.flags	= 0,
			.len	= 1,
			.buf	= data
		},		/*                */
		{
			.addr	= client->addr,
			.flags	= I2C_M_RD,
			.len	= n,
			.buf	= data
		}
	};

	int ret;

	data[0] = reg;
	ret = i2c_transfer(client->adapter, msgs, ARRAY_SIZE(msgs));
	if (ret != ARRAY_SIZE(msgs)) {
		dev_err(&client->dev, "%s: read error, ret=%d\n",
			__func__, ret);
		return -EIO;
	}

	return 0;
}


static int isl12022_write_reg(struct i2c_client *client,
			      uint8_t reg, uint8_t val)
{
	uint8_t data[2] = { reg, val };
	int err;

	err = i2c_master_send(client, data, sizeof(data));
	if (err != sizeof(data)) {
		dev_err(&client->dev,
			"%s: err=%d addr=%02x, data=%02x\n",
			__func__, err, data[0], data[1]);
		return -EIO;
	}

	return 0;
}


/*
                                                                        
                                                               
 */
static int isl12022_get_datetime(struct i2c_client *client, struct rtc_time *tm)
{
	uint8_t buf[ISL12022_REG_INT + 1];
	int ret;

	ret = isl12022_read_regs(client, ISL12022_REG_SC, buf, sizeof(buf));
	if (ret)
		return ret;

	if (buf[ISL12022_REG_SR] & (ISL12022_SR_LBAT85 | ISL12022_SR_LBAT75)) {
		dev_warn(&client->dev,
			 "voltage dropped below %u%%, "
			 "date and time is not reliable.\n",
			 buf[ISL12022_REG_SR] & ISL12022_SR_LBAT85 ? 85 : 75);
	}

	dev_dbg(&client->dev,
		"%s: raw data is sec=%02x, min=%02x, hr=%02x, "
		"mday=%02x, mon=%02x, year=%02x, wday=%02x, "
		"sr=%02x, int=%02x",
		__func__,
		buf[ISL12022_REG_SC],
		buf[ISL12022_REG_MN],
		buf[ISL12022_REG_HR],
		buf[ISL12022_REG_DT],
		buf[ISL12022_REG_MO],
		buf[ISL12022_REG_YR],
		buf[ISL12022_REG_DW],
		buf[ISL12022_REG_SR],
		buf[ISL12022_REG_INT]);

	tm->tm_sec = bcd2bin(buf[ISL12022_REG_SC] & 0x7F);
	tm->tm_min = bcd2bin(buf[ISL12022_REG_MN] & 0x7F);
	tm->tm_hour = bcd2bin(buf[ISL12022_REG_HR] & 0x3F);
	tm->tm_mday = bcd2bin(buf[ISL12022_REG_DT] & 0x3F);
	tm->tm_wday = buf[ISL12022_REG_DW] & 0x07;
	tm->tm_mon = bcd2bin(buf[ISL12022_REG_MO] & 0x1F) - 1;
	tm->tm_year = bcd2bin(buf[ISL12022_REG_YR]) + 100;

	dev_dbg(&client->dev, "%s: secs=%d, mins=%d, hours=%d, "
		"mday=%d, mon=%d, year=%d, wday=%d\n",
		__func__,
		tm->tm_sec, tm->tm_min, tm->tm_hour,
		tm->tm_mday, tm->tm_mon, tm->tm_year, tm->tm_wday);

	/*                                                              
                                                                     */
	if (rtc_valid_tm(tm) < 0)
		dev_err(&client->dev, "retrieved date and time is invalid.\n");

	return 0;
}

static int isl12022_set_datetime(struct i2c_client *client, struct rtc_time *tm)
{
	struct isl12022 *isl12022 = i2c_get_clientdata(client);
	size_t i;
	int ret;
	uint8_t buf[ISL12022_REG_DW + 1];

	dev_dbg(&client->dev, "%s: secs=%d, mins=%d, hours=%d, "
		"mday=%d, mon=%d, year=%d, wday=%d\n",
		__func__,
		tm->tm_sec, tm->tm_min, tm->tm_hour,
		tm->tm_mday, tm->tm_mon, tm->tm_year, tm->tm_wday);

	if (!isl12022->write_enabled) {

		ret = isl12022_read_regs(client, ISL12022_REG_INT, buf, 1);
		if (ret)
			return ret;

		/*                                                           
                 */
		if (!(buf[0] & ISL12022_INT_WRTC)) {
			dev_info(&client->dev,
				 "init write enable and 24 hour format\n");

			/*                           */
			ret = isl12022_write_reg(client,
						 ISL12022_REG_INT,
						 buf[0] | ISL12022_INT_WRTC);
			if (ret)
				return ret;

			/*                                                   
                                                         
              */
			ret = isl12022_read_regs(client, ISL12022_REG_HR,
						 buf, 1);
			if (ret)
				return ret;

			ret = isl12022_write_reg(client,
						 ISL12022_REG_HR,
						 buf[0] | ISL12022_HR_MIL);
			if (ret)
				return ret;
		}

		isl12022->write_enabled = 1;
	}

	/*                            */
	buf[ISL12022_REG_SC] = bin2bcd(tm->tm_sec);
	buf[ISL12022_REG_MN] = bin2bcd(tm->tm_min);
	buf[ISL12022_REG_HR] = bin2bcd(tm->tm_hour) | ISL12022_HR_MIL;

	buf[ISL12022_REG_DT] = bin2bcd(tm->tm_mday);

	/*               */
	buf[ISL12022_REG_MO] = bin2bcd(tm->tm_mon + 1);

	/*                  */
	buf[ISL12022_REG_YR] = bin2bcd(tm->tm_year % 100);

	buf[ISL12022_REG_DW] = tm->tm_wday & 0x07;

	/*                       */
	for (i = 0; i < ARRAY_SIZE(buf); i++) {
		ret = isl12022_write_reg(client, ISL12022_REG_SC + i,
					 buf[ISL12022_REG_SC + i]);
		if (ret)
			return -EIO;
	}

	return 0;
}

static int isl12022_rtc_read_time(struct device *dev, struct rtc_time *tm)
{
	return isl12022_get_datetime(to_i2c_client(dev), tm);
}

static int isl12022_rtc_set_time(struct device *dev, struct rtc_time *tm)
{
	return isl12022_set_datetime(to_i2c_client(dev), tm);
}

static const struct rtc_class_ops isl12022_rtc_ops = {
	.read_time	= isl12022_rtc_read_time,
	.set_time	= isl12022_rtc_set_time,
};

static int isl12022_probe(struct i2c_client *client,
			  const struct i2c_device_id *id)
{
	struct isl12022 *isl12022;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
		return -ENODEV;

	isl12022 = devm_kzalloc(&client->dev, sizeof(struct isl12022),
				GFP_KERNEL);
	if (!isl12022)
		return -ENOMEM;

	dev_dbg(&client->dev, "chip found, driver version " DRV_VERSION "\n");

	i2c_set_clientdata(client, isl12022);

	isl12022->rtc = devm_rtc_device_register(&client->dev,
					isl12022_driver.driver.name,
					&isl12022_rtc_ops, THIS_MODULE);
	if (IS_ERR(isl12022->rtc))
		return PTR_ERR(isl12022->rtc);

	return 0;
}

static int isl12022_remove(struct i2c_client *client)
{
	return 0;
}

static const struct i2c_device_id isl12022_id[] = {
	{ "isl12022", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, isl12022_id);

static struct i2c_driver isl12022_driver = {
	.driver		= {
		.name	= "rtc-isl12022",
	},
	.probe		= isl12022_probe,
	.remove		= isl12022_remove,
	.id_table	= isl12022_id,
};

module_i2c_driver(isl12022_driver);

MODULE_AUTHOR("roman.fietze@telemotive.de");
MODULE_DESCRIPTION("ISL 12022 RTC driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);
