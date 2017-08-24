/*
 * Driver for the Texas Instruments / Burr Brown INA209
 * Bidirectional Current/Power Monitor
 *
 * Copyright (C) 2012 Guenter Roeck <linux@roeck-us.net>
 *
 * Derived from Ira W. Snyder's original driver submission
 *	Copyright (C) 2008 Paul Hays <Paul.Hays@cattail.ca>
 *	Copyright (C) 2008-2009 Ira W. Snyder <iws@ovro.caltech.edu>
 *
 * Aligned with ina2xx driver
 *	Copyright (C) 2012 Lothar Felten <l-felten@ti.com>
 *	Thanks to Jan Volkering
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * Datasheet:
 * http://www.ti.com/lit/gpn/ina209
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/bug.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>

#include <linux/platform_data/ina2xx.h>

/*                      */
#define INA209_CONFIGURATION		0x00
#define INA209_STATUS			0x01
#define INA209_STATUS_MASK		0x02
#define INA209_SHUNT_VOLTAGE		0x03
#define INA209_BUS_VOLTAGE		0x04
#define INA209_POWER			0x05
#define INA209_CURRENT			0x06
#define INA209_SHUNT_VOLTAGE_POS_PEAK	0x07
#define INA209_SHUNT_VOLTAGE_NEG_PEAK	0x08
#define INA209_BUS_VOLTAGE_MAX_PEAK	0x09
#define INA209_BUS_VOLTAGE_MIN_PEAK	0x0a
#define INA209_POWER_PEAK		0x0b
#define INA209_SHUNT_VOLTAGE_POS_WARN	0x0c
#define INA209_SHUNT_VOLTAGE_NEG_WARN	0x0d
#define INA209_POWER_WARN		0x0e
#define INA209_BUS_VOLTAGE_OVER_WARN	0x0f
#define INA209_BUS_VOLTAGE_UNDER_WARN	0x10
#define INA209_POWER_OVER_LIMIT		0x11
#define INA209_BUS_VOLTAGE_OVER_LIMIT	0x12
#define INA209_BUS_VOLTAGE_UNDER_LIMIT	0x13
#define INA209_CRITICAL_DAC_POS		0x14
#define INA209_CRITICAL_DAC_NEG		0x15
#define INA209_CALIBRATION		0x16

#define INA209_REGISTERS		0x17

#define INA209_CONFIG_DEFAULT		0x3c47	/*                   */
#define INA209_SHUNT_DEFAULT		10000	/*      */

struct ina209_data {
	struct device *hwmon_dev;

	struct mutex update_lock;
	bool valid;
	unsigned long last_updated;	/*            */

	u16 regs[INA209_REGISTERS];	/*                    */

	u16 config_orig;		/*                        */
	u16 calibration_orig;		/*                      */
	u16 update_interval;
};

static struct ina209_data *ina209_update_device(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ina209_data *data = i2c_get_clientdata(client);
	struct ina209_data *ret = data;
	s32 val;
	int i;

	mutex_lock(&data->update_lock);

	if (!data->valid ||
	    time_after(jiffies, data->last_updated + data->update_interval)) {
		for (i = 0; i < ARRAY_SIZE(data->regs); i++) {
			val = i2c_smbus_read_word_swapped(client, i);
			if (val < 0) {
				ret = ERR_PTR(val);
				goto abort;
			}
			data->regs[i] = val;
		}
		data->last_updated = jiffies;
		data->valid = true;
	}
abort:
	mutex_unlock(&data->update_lock);
	return ret;
}

/*
                                                            
                          
 */
static long ina209_from_reg(const u8 reg, const u16 val)
{
	switch (reg) {
	case INA209_SHUNT_VOLTAGE:
	case INA209_SHUNT_VOLTAGE_POS_PEAK:
	case INA209_SHUNT_VOLTAGE_NEG_PEAK:
	case INA209_SHUNT_VOLTAGE_POS_WARN:
	case INA209_SHUNT_VOLTAGE_NEG_WARN:
		/*                           */
		return DIV_ROUND_CLOSEST(val, 100);

	case INA209_BUS_VOLTAGE:
	case INA209_BUS_VOLTAGE_MAX_PEAK:
	case INA209_BUS_VOLTAGE_MIN_PEAK:
	case INA209_BUS_VOLTAGE_OVER_WARN:
	case INA209_BUS_VOLTAGE_UNDER_WARN:
	case INA209_BUS_VOLTAGE_OVER_LIMIT:
	case INA209_BUS_VOLTAGE_UNDER_LIMIT:
		/*                              */
		return (val >> 3) * 4;

	case INA209_CRITICAL_DAC_POS:
		/*                               */
		return val >> 8;

	case INA209_CRITICAL_DAC_NEG:
		/*                               */
		return -1 * (val >> 8);

	case INA209_POWER:
	case INA209_POWER_PEAK:
	case INA209_POWER_WARN:
	case INA209_POWER_OVER_LIMIT:
		/*                          */
		return val * 20 * 1000L;

	case INA209_CURRENT:
		/*                               */
		return val;
	}

	/*                   */
	WARN_ON_ONCE(1);
	return 0;
}

/*
                                                                     
                            
 */
static int ina209_to_reg(u8 reg, u16 old, long val)
{
	switch (reg) {
	case INA209_SHUNT_VOLTAGE_POS_WARN:
	case INA209_SHUNT_VOLTAGE_NEG_WARN:
		/*                               */
		return clamp_val(val, -320, 320) * 100;

	case INA209_BUS_VOLTAGE_OVER_WARN:
	case INA209_BUS_VOLTAGE_UNDER_WARN:
	case INA209_BUS_VOLTAGE_OVER_LIMIT:
	case INA209_BUS_VOLTAGE_UNDER_LIMIT:
		/*
                                  
    
                                                            
                                           
   */
		return (DIV_ROUND_CLOSEST(clamp_val(val, 0, 32000), 4) << 3)
		  | (old & 0x7);

	case INA209_CRITICAL_DAC_NEG:
		/*
                                 
                                                           
    
                                                       
                                           
   */
		return (clamp_val(-val, 0, 255) << 8) | (old & 0xff);

	case INA209_CRITICAL_DAC_POS:
		/*
                                
    
                                                       
                                           
   */
		return (clamp_val(val, 0, 255) << 8) | (old & 0xff);

	case INA209_POWER_WARN:
	case INA209_POWER_OVER_LIMIT:
		/*           */
		return DIV_ROUND_CLOSEST(val, 20 * 1000);
	}

	/*                                                    */
	return -EACCES;
}

static int ina209_interval_from_reg(u16 reg)
{
	return 68 >> (15 - ((reg >> 3) & 0x0f));
}

static u16 ina209_reg_from_interval(u16 config, long interval)
{
	int i, adc;

	if (interval <= 0) {
		adc = 8;
	} else {
		adc = 15;
		for (i = 34 + 34 / 2; i; i >>= 1) {
			if (i < interval)
				break;
			adc--;
		}
	}
	return (config & 0xf807) | (adc << 3) | (adc << 7);
}

static ssize_t ina209_set_interval(struct device *dev,
				   struct device_attribute *da,
				   const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ina209_data *data = ina209_update_device(dev);
	long val;
	u16 regval;
	int ret;

	if (IS_ERR(data))
		return PTR_ERR(data);

	ret = kstrtol(buf, 10, &val);
	if (ret < 0)
		return ret;

	mutex_lock(&data->update_lock);
	regval = ina209_reg_from_interval(data->regs[INA209_CONFIGURATION],
					  val);
	i2c_smbus_write_word_swapped(client, INA209_CONFIGURATION, regval);
	data->regs[INA209_CONFIGURATION] = regval;
	data->update_interval = ina209_interval_from_reg(regval);
	mutex_unlock(&data->update_lock);
	return count;
}

static ssize_t ina209_show_interval(struct device *dev,
				    struct device_attribute *da, char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ina209_data *data = i2c_get_clientdata(client);

	return snprintf(buf, PAGE_SIZE, "%d\n", data->update_interval);
}

/*
                                                                            
                                                                      
                                                                           
                                
 */
static u16 ina209_reset_history_regs[] = {
	INA209_SHUNT_VOLTAGE_POS_PEAK,
	INA209_SHUNT_VOLTAGE_NEG_PEAK,
	INA209_BUS_VOLTAGE_MAX_PEAK,
	INA209_BUS_VOLTAGE_MIN_PEAK,
	INA209_POWER_PEAK
};

static ssize_t ina209_reset_history(struct device *dev,
				    struct device_attribute *da,
				    const char *buf,
				    size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ina209_data *data = i2c_get_clientdata(client);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	u32 mask = attr->index;
	long val;
	int i, ret;

	ret = kstrtol(buf, 10, &val);
	if (ret < 0)
		return ret;

	mutex_lock(&data->update_lock);
	for (i = 0; i < ARRAY_SIZE(ina209_reset_history_regs); i++) {
		if (mask & (1 << i))
			i2c_smbus_write_word_swapped(client,
					ina209_reset_history_regs[i], 1);
	}
	data->valid = false;
	mutex_unlock(&data->update_lock);
	return count;
}

static ssize_t ina209_set_value(struct device *dev,
				struct device_attribute *da,
				const char *buf,
				size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct ina209_data *data = ina209_update_device(dev);
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	int reg = attr->index;
	long val;
	int ret;

	if (IS_ERR(data))
		return PTR_ERR(data);

	ret = kstrtol(buf, 10, &val);
	if (ret < 0)
		return ret;

	mutex_lock(&data->update_lock);
	ret = ina209_to_reg(reg, data->regs[reg], val);
	if (ret < 0) {
		count = ret;
		goto abort;
	}
	i2c_smbus_write_word_swapped(client, reg, ret);
	data->regs[reg] = ret;
abort:
	mutex_unlock(&data->update_lock);
	return count;
}

static ssize_t ina209_show_value(struct device *dev,
				 struct device_attribute *da,
				 char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct ina209_data *data = ina209_update_device(dev);
	long val;

	if (IS_ERR(data))
		return PTR_ERR(data);

	val = ina209_from_reg(attr->index, data->regs[attr->index]);
	return snprintf(buf, PAGE_SIZE, "%ld\n", val);
}

static ssize_t ina209_show_alarm(struct device *dev,
				 struct device_attribute *da,
				 char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct ina209_data *data = ina209_update_device(dev);
	const unsigned int mask = attr->index;
	u16 status;

	if (IS_ERR(data))
		return PTR_ERR(data);

	status = data->regs[INA209_STATUS];

	/*
                                                                 
                                                       
  */
	return snprintf(buf, PAGE_SIZE, "%u\n", !!(status & mask));
}

/*                                        */
static SENSOR_DEVICE_ATTR(in0_input, S_IRUGO, ina209_show_value, NULL,
			  INA209_SHUNT_VOLTAGE);
static SENSOR_DEVICE_ATTR(in0_input_highest, S_IRUGO, ina209_show_value, NULL,
			  INA209_SHUNT_VOLTAGE_POS_PEAK);
static SENSOR_DEVICE_ATTR(in0_input_lowest, S_IRUGO, ina209_show_value, NULL,
			  INA209_SHUNT_VOLTAGE_NEG_PEAK);
static SENSOR_DEVICE_ATTR(in0_reset_history, S_IWUSR, NULL,
			  ina209_reset_history, (1 << 0) | (1 << 1));
static SENSOR_DEVICE_ATTR(in0_max, S_IRUGO | S_IWUSR, ina209_show_value,
			  ina209_set_value, INA209_SHUNT_VOLTAGE_POS_WARN);
static SENSOR_DEVICE_ATTR(in0_min, S_IRUGO | S_IWUSR, ina209_show_value,
			  ina209_set_value, INA209_SHUNT_VOLTAGE_NEG_WARN);
static SENSOR_DEVICE_ATTR(in0_crit_max, S_IRUGO | S_IWUSR, ina209_show_value,
			  ina209_set_value, INA209_CRITICAL_DAC_POS);
static SENSOR_DEVICE_ATTR(in0_crit_min, S_IRUGO | S_IWUSR, ina209_show_value,
			  ina209_set_value, INA209_CRITICAL_DAC_NEG);

static SENSOR_DEVICE_ATTR(in0_min_alarm,  S_IRUGO, ina209_show_alarm, NULL,
			  1 << 11);
static SENSOR_DEVICE_ATTR(in0_max_alarm, S_IRUGO, ina209_show_alarm, NULL,
			  1 << 12);
static SENSOR_DEVICE_ATTR(in0_crit_min_alarm, S_IRUGO, ina209_show_alarm, NULL,
			  1 << 6);
static SENSOR_DEVICE_ATTR(in0_crit_max_alarm, S_IRUGO, ina209_show_alarm, NULL,
			  1 << 7);

/*                                      */
static SENSOR_DEVICE_ATTR(in1_input, S_IRUGO, ina209_show_value, NULL,
			  INA209_BUS_VOLTAGE);
static SENSOR_DEVICE_ATTR(in1_input_highest, S_IRUGO, ina209_show_value, NULL,
			  INA209_BUS_VOLTAGE_MAX_PEAK);
static SENSOR_DEVICE_ATTR(in1_input_lowest, S_IRUGO, ina209_show_value, NULL,
			  INA209_BUS_VOLTAGE_MIN_PEAK);
static SENSOR_DEVICE_ATTR(in1_reset_history, S_IWUSR, NULL,
			  ina209_reset_history, (1 << 2) | (1 << 3));
static SENSOR_DEVICE_ATTR(in1_max, S_IRUGO | S_IWUSR, ina209_show_value,
			  ina209_set_value, INA209_BUS_VOLTAGE_OVER_WARN);
static SENSOR_DEVICE_ATTR(in1_min, S_IRUGO | S_IWUSR, ina209_show_value,
			  ina209_set_value, INA209_BUS_VOLTAGE_UNDER_WARN);
static SENSOR_DEVICE_ATTR(in1_crit_max, S_IRUGO | S_IWUSR, ina209_show_value,
			  ina209_set_value, INA209_BUS_VOLTAGE_OVER_LIMIT);
static SENSOR_DEVICE_ATTR(in1_crit_min, S_IRUGO | S_IWUSR, ina209_show_value,
			  ina209_set_value, INA209_BUS_VOLTAGE_UNDER_LIMIT);

static SENSOR_DEVICE_ATTR(in1_min_alarm, S_IRUGO, ina209_show_alarm, NULL,
			  1 << 14);
static SENSOR_DEVICE_ATTR(in1_max_alarm, S_IRUGO, ina209_show_alarm, NULL,
			  1 << 15);
static SENSOR_DEVICE_ATTR(in1_crit_min_alarm, S_IRUGO, ina209_show_alarm, NULL,
			  1 << 9);
static SENSOR_DEVICE_ATTR(in1_crit_max_alarm, S_IRUGO, ina209_show_alarm, NULL,
			  1 << 10);

/*       */
static SENSOR_DEVICE_ATTR(power1_input, S_IRUGO, ina209_show_value, NULL,
			  INA209_POWER);
static SENSOR_DEVICE_ATTR(power1_input_highest, S_IRUGO, ina209_show_value,
			  NULL, INA209_POWER_PEAK);
static SENSOR_DEVICE_ATTR(power1_reset_history, S_IWUSR, NULL,
			  ina209_reset_history, 1 << 4);
static SENSOR_DEVICE_ATTR(power1_max, S_IRUGO | S_IWUSR, ina209_show_value,
			  ina209_set_value, INA209_POWER_WARN);
static SENSOR_DEVICE_ATTR(power1_crit, S_IRUGO | S_IWUSR, ina209_show_value,
			  ina209_set_value, INA209_POWER_OVER_LIMIT);

static SENSOR_DEVICE_ATTR(power1_max_alarm, S_IRUGO, ina209_show_alarm, NULL,
			  1 << 13);
static SENSOR_DEVICE_ATTR(power1_crit_alarm, S_IRUGO, ina209_show_alarm, NULL,
			  1 << 8);

/*         */
static SENSOR_DEVICE_ATTR(curr1_input, S_IRUGO, ina209_show_value, NULL,
			  INA209_CURRENT);

static SENSOR_DEVICE_ATTR(update_interval, S_IRUGO | S_IWUSR,
			  ina209_show_interval, ina209_set_interval, 0);

/*
                                                                           
                                       
 */
static struct attribute *ina209_attributes[] = {
	&sensor_dev_attr_in0_input.dev_attr.attr,
	&sensor_dev_attr_in0_input_highest.dev_attr.attr,
	&sensor_dev_attr_in0_input_lowest.dev_attr.attr,
	&sensor_dev_attr_in0_reset_history.dev_attr.attr,
	&sensor_dev_attr_in0_max.dev_attr.attr,
	&sensor_dev_attr_in0_min.dev_attr.attr,
	&sensor_dev_attr_in0_crit_max.dev_attr.attr,
	&sensor_dev_attr_in0_crit_min.dev_attr.attr,
	&sensor_dev_attr_in0_max_alarm.dev_attr.attr,
	&sensor_dev_attr_in0_min_alarm.dev_attr.attr,
	&sensor_dev_attr_in0_crit_max_alarm.dev_attr.attr,
	&sensor_dev_attr_in0_crit_min_alarm.dev_attr.attr,

	&sensor_dev_attr_in1_input.dev_attr.attr,
	&sensor_dev_attr_in1_input_highest.dev_attr.attr,
	&sensor_dev_attr_in1_input_lowest.dev_attr.attr,
	&sensor_dev_attr_in1_reset_history.dev_attr.attr,
	&sensor_dev_attr_in1_max.dev_attr.attr,
	&sensor_dev_attr_in1_min.dev_attr.attr,
	&sensor_dev_attr_in1_crit_max.dev_attr.attr,
	&sensor_dev_attr_in1_crit_min.dev_attr.attr,
	&sensor_dev_attr_in1_max_alarm.dev_attr.attr,
	&sensor_dev_attr_in1_min_alarm.dev_attr.attr,
	&sensor_dev_attr_in1_crit_max_alarm.dev_attr.attr,
	&sensor_dev_attr_in1_crit_min_alarm.dev_attr.attr,

	&sensor_dev_attr_power1_input.dev_attr.attr,
	&sensor_dev_attr_power1_input_highest.dev_attr.attr,
	&sensor_dev_attr_power1_reset_history.dev_attr.attr,
	&sensor_dev_attr_power1_max.dev_attr.attr,
	&sensor_dev_attr_power1_crit.dev_attr.attr,
	&sensor_dev_attr_power1_max_alarm.dev_attr.attr,
	&sensor_dev_attr_power1_crit_alarm.dev_attr.attr,

	&sensor_dev_attr_curr1_input.dev_attr.attr,

	&sensor_dev_attr_update_interval.dev_attr.attr,

	NULL,
};

static const struct attribute_group ina209_group = {
	.attrs = ina209_attributes,
};

static void ina209_restore_conf(struct i2c_client *client,
				struct ina209_data *data)
{
	/*                               */
	i2c_smbus_write_word_swapped(client, INA209_CONFIGURATION,
				     data->config_orig);
	i2c_smbus_write_word_swapped(client, INA209_CALIBRATION,
				     data->calibration_orig);
}

static int ina209_init_client(struct i2c_client *client,
			      struct ina209_data *data)
{
	struct ina2xx_platform_data *pdata = dev_get_platdata(&client->dev);
	u32 shunt;
	int reg;

	reg = i2c_smbus_read_word_swapped(client, INA209_CALIBRATION);
	if (reg < 0)
		return reg;
	data->calibration_orig = reg;

	reg = i2c_smbus_read_word_swapped(client, INA209_CONFIGURATION);
	if (reg < 0)
		return reg;
	data->config_orig = reg;

	if (pdata) {
		if (pdata->shunt_uohms <= 0)
			return -EINVAL;
		shunt = pdata->shunt_uohms;
	} else if (!of_property_read_u32(client->dev.of_node, "shunt-resistor",
					 &shunt)) {
		if (shunt == 0)
			return -EINVAL;
	} else {
		shunt = data->calibration_orig ?
		  40960000 / data->calibration_orig : INA209_SHUNT_DEFAULT;
	}

	i2c_smbus_write_word_swapped(client, INA209_CONFIGURATION,
				     INA209_CONFIG_DEFAULT);
	data->update_interval = ina209_interval_from_reg(INA209_CONFIG_DEFAULT);

	/*
                                                    
                                 
  */
	i2c_smbus_write_word_swapped(client, INA209_CALIBRATION,
				     clamp_val(40960000 / shunt, 1, 65535));

	/*                       */
	i2c_smbus_read_word_swapped(client, INA209_STATUS);

	return 0;
}

static int ina209_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = client->adapter;
	struct ina209_data *data;
	int ret;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_WORD_DATA))
		return -ENODEV;

	data = devm_kzalloc(&client->dev, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	i2c_set_clientdata(client, data);
	mutex_init(&data->update_lock);

	ret = ina209_init_client(client, data);
	if (ret)
		return ret;

	/*                      */
	ret = sysfs_create_group(&client->dev.kobj, &ina209_group);
	if (ret)
		goto out_restore_conf;

	data->hwmon_dev = hwmon_device_register(&client->dev);
	if (IS_ERR(data->hwmon_dev)) {
		ret = PTR_ERR(data->hwmon_dev);
		goto out_hwmon_device_register;
	}

	return 0;

out_hwmon_device_register:
	sysfs_remove_group(&client->dev.kobj, &ina209_group);
out_restore_conf:
	ina209_restore_conf(client, data);
	return ret;
}

static int ina209_remove(struct i2c_client *client)
{
	struct ina209_data *data = i2c_get_clientdata(client);

	hwmon_device_unregister(data->hwmon_dev);
	sysfs_remove_group(&client->dev.kobj, &ina209_group);
	ina209_restore_conf(client, data);

	return 0;
}

static const struct i2c_device_id ina209_id[] = {
	{ "ina209", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, ina209_id);

/*                                          */
static struct i2c_driver ina209_driver = {
	.class		= I2C_CLASS_HWMON,
	.driver = {
		.name	= "ina209",
	},
	.probe		= ina209_probe,
	.remove		= ina209_remove,
	.id_table	= ina209_id,
};

module_i2c_driver(ina209_driver);

MODULE_AUTHOR("Ira W. Snyder <iws@ovro.caltech.edu>, Paul Hays <Paul.Hays@cattail.ca>, Guenter Roeck <linux@roeck-us.net>");
MODULE_DESCRIPTION("INA209 driver");
MODULE_LICENSE("GPL");
