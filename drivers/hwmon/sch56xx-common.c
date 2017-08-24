/***************************************************************************
 *   Copyright (C) 2010-2012 Hans de Goede <hdegoede@redhat.com>           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/acpi.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/watchdog.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/kref.h>
#include <linux/slab.h>
#include "sch56xx-common.h"

/*                   */
static int nowayout = WATCHDOG_NOWAYOUT;
module_param(nowayout, int, 0);
MODULE_PARM_DESC(nowayout, "Watchdog cannot be stopped once started (default="
	__MODULE_STRING(WATCHDOG_NOWAYOUT) ")");

#define SIO_SCH56XX_LD_EM	0x0C	/*                                  */
#define SIO_UNLOCK_KEY		0x55	/*                         */
#define SIO_LOCK_KEY		0xAA	/*                          */

#define SIO_REG_LDSEL		0x07	/*                       */
#define SIO_REG_DEVID		0x20	/*           */
#define SIO_REG_ENABLE		0x30	/*                       */
#define SIO_REG_ADDR		0x66	/*                                  */

#define SIO_SCH5627_ID		0xC6	/*            */
#define SIO_SCH5636_ID		0xC7	/*            */

#define REGION_LENGTH		10

#define SCH56XX_CMD_READ	0x02
#define SCH56XX_CMD_WRITE	0x03

/*                    */
#define SCH56XX_REG_WDOG_PRESET		0x58B
#define SCH56XX_REG_WDOG_CONTROL	0x58C
#define SCH56XX_WDOG_TIME_BASE_SEC	0x01
#define SCH56XX_REG_WDOG_OUTPUT_ENABLE	0x58E
#define SCH56XX_WDOG_OUTPUT_ENABLE	0x02

struct sch56xx_watchdog_data {
	u16 addr;
	struct mutex *io_lock;
	struct kref kref;
	struct watchdog_info wdinfo;
	struct watchdog_device wddev;
	u8 watchdog_preset;
	u8 watchdog_control;
	u8 watchdog_output_enable;
};

static struct platform_device *sch56xx_pdev;

/*                     */
static inline int superio_inb(int base, int reg)
{
	outb(reg, base);
	return inb(base + 1);
}

static inline int superio_enter(int base)
{
	/*                                                    */
	if (!request_muxed_region(base, 2, "sch56xx")) {
		pr_err("I/O address 0x%04x already in use\n", base);
		return -EBUSY;
	}

	outb(SIO_UNLOCK_KEY, base);

	return 0;
}

static inline void superio_select(int base, int ld)
{
	outb(SIO_REG_LDSEL, base);
	outb(ld, base + 1);
}

static inline void superio_exit(int base)
{
	outb(SIO_LOCK_KEY, base);
	release_region(base, 2);
}

static int sch56xx_send_cmd(u16 addr, u8 cmd, u16 reg, u8 v)
{
	u8 val;
	int i;
	/*
                                                                  
                                                                
                                                              
                                                              
                                    
  */
	const int max_busy_polls = 64;
	const int max_lazy_polls = 32;

	/*                                                        */
	val = inb(addr + 1);
	outb(val, addr + 1);

	/*                                                           */
	outb(0x00, addr + 2);
	outb(0x80, addr + 3);

	/*                             */
	outb(cmd, addr + 4); /*                                       */
	outb(0x01, addr + 5); /*                              */
	outb(0x04, addr + 2); /*                                     */

	/*                   */
	if (cmd == SCH56XX_CMD_WRITE)
		outb(v, addr + 4);

	/*                     */
	outb(reg & 0xff, addr + 6);
	outb(reg >> 8, addr + 7);

	/*                                   */
	outb(0x01, addr); /*                                      */

	/*                                  */
	for (i = 0; i < max_busy_polls + max_lazy_polls; i++) {
		if (i >= max_busy_polls)
			msleep(1);
		/*                                */
		val = inb(addr + 8);
		/*                                       */
		if (val)
			outb(val, addr + 8);
		/*                     */
		if (val & 0x01)
			break;
	}
	if (i == max_busy_polls + max_lazy_polls) {
		pr_err("Max retries exceeded reading virtual register 0x%04hx (%d)\n",
		       reg, 1);
		return -EIO;
	}

	/*
                                                                      
                               
  */
	for (i = 0; i < max_busy_polls; i++) {
		/*                          */
		val = inb(addr + 1);
		/*                     */
		if (val == 0x01)
			break;

		if (i == 0)
			pr_warn("EC reports: 0x%02x reading virtual register 0x%04hx\n",
				(unsigned int)val, reg);
	}
	if (i == max_busy_polls) {
		pr_err("Max retries exceeded reading virtual register 0x%04hx (%d)\n",
		       reg, 2);
		return -EIO;
	}

	/*
                                                    
   
                                                               
                         
                         
   
                                                      
  */

	/*                  */
	if (cmd == SCH56XX_CMD_READ)
		return inb(addr + 4);

	return 0;
}

int sch56xx_read_virtual_reg(u16 addr, u16 reg)
{
	return sch56xx_send_cmd(addr, SCH56XX_CMD_READ, reg, 0);
}
EXPORT_SYMBOL(sch56xx_read_virtual_reg);

int sch56xx_write_virtual_reg(u16 addr, u16 reg, u8 val)
{
	return sch56xx_send_cmd(addr, SCH56XX_CMD_WRITE, reg, val);
}
EXPORT_SYMBOL(sch56xx_write_virtual_reg);

int sch56xx_read_virtual_reg16(u16 addr, u16 reg)
{
	int lsb, msb;

	/*                                                                */
	lsb = sch56xx_read_virtual_reg(addr, reg);
	if (lsb < 0)
		return lsb;

	msb = sch56xx_read_virtual_reg(addr, reg + 1);
	if (msb < 0)
		return msb;

	return lsb | (msb << 8);
}
EXPORT_SYMBOL(sch56xx_read_virtual_reg16);

int sch56xx_read_virtual_reg12(u16 addr, u16 msb_reg, u16 lsn_reg,
			       int high_nibble)
{
	int msb, lsn;

	/*                                                                */
	msb = sch56xx_read_virtual_reg(addr, msb_reg);
	if (msb < 0)
		return msb;

	lsn = sch56xx_read_virtual_reg(addr, lsn_reg);
	if (lsn < 0)
		return lsn;

	if (high_nibble)
		return (msb << 4) | (lsn >> 4);
	else
		return (msb << 4) | (lsn & 0x0f);
}
EXPORT_SYMBOL(sch56xx_read_virtual_reg12);

/*
                    
 */

/*                                                      
                                                      */
static void watchdog_release_resources(struct kref *r)
{
	struct sch56xx_watchdog_data *data =
		container_of(r, struct sch56xx_watchdog_data, kref);
	kfree(data);
}

static int watchdog_set_timeout(struct watchdog_device *wddev,
				unsigned int timeout)
{
	struct sch56xx_watchdog_data *data = watchdog_get_drvdata(wddev);
	unsigned int resolution;
	u8 control;
	int ret;

	/*                                   */
	if (timeout <= 255)
		resolution = 1;
	else
		resolution = 60;

	if (timeout < resolution || timeout > (resolution * 255))
		return -EINVAL;

	if (resolution == 1)
		control = data->watchdog_control | SCH56XX_WDOG_TIME_BASE_SEC;
	else
		control = data->watchdog_control & ~SCH56XX_WDOG_TIME_BASE_SEC;

	if (data->watchdog_control != control) {
		mutex_lock(data->io_lock);
		ret = sch56xx_write_virtual_reg(data->addr,
						SCH56XX_REG_WDOG_CONTROL,
						control);
		mutex_unlock(data->io_lock);
		if (ret)
			return ret;

		data->watchdog_control = control;
	}

	/*
                                                                   
                           
  */
	data->watchdog_preset = DIV_ROUND_UP(timeout, resolution);
	wddev->timeout = data->watchdog_preset * resolution;

	return 0;
}

static int watchdog_start(struct watchdog_device *wddev)
{
	struct sch56xx_watchdog_data *data = watchdog_get_drvdata(wddev);
	int ret;
	u8 val;

	/*
                                                             
                                                             
                                                                  
   
                                                                 
                                                                  
                           
   
                                                                     
                                                                     
                                                                 
                                                                      
   
                                                                     
                                                                    
          
  */

	mutex_lock(data->io_lock);

	/*                                         */
	ret = sch56xx_write_virtual_reg(data->addr, SCH56XX_REG_WDOG_PRESET,
					data->watchdog_preset);
	if (ret)
		goto leave;

	/*                  */
	val = data->watchdog_output_enable | SCH56XX_WDOG_OUTPUT_ENABLE;
	ret = sch56xx_write_virtual_reg(data->addr,
					SCH56XX_REG_WDOG_OUTPUT_ENABLE, val);
	if (ret)
		goto leave;

	data->watchdog_output_enable = val;

	/*                                        */
	val = inb(data->addr + 9);
	if (val & 0x01)
		outb(0x01, data->addr + 9);

leave:
	mutex_unlock(data->io_lock);
	return ret;
}

static int watchdog_trigger(struct watchdog_device *wddev)
{
	struct sch56xx_watchdog_data *data = watchdog_get_drvdata(wddev);
	int ret;

	/*                                      */
	mutex_lock(data->io_lock);
	ret = sch56xx_write_virtual_reg(data->addr, SCH56XX_REG_WDOG_PRESET,
					data->watchdog_preset);
	mutex_unlock(data->io_lock);

	return ret;
}

static int watchdog_stop(struct watchdog_device *wddev)
{
	struct sch56xx_watchdog_data *data = watchdog_get_drvdata(wddev);
	int ret = 0;
	u8 val;

	val = data->watchdog_output_enable & ~SCH56XX_WDOG_OUTPUT_ENABLE;
	mutex_lock(data->io_lock);
	ret = sch56xx_write_virtual_reg(data->addr,
					SCH56XX_REG_WDOG_OUTPUT_ENABLE, val);
	mutex_unlock(data->io_lock);
	if (ret)
		return ret;

	data->watchdog_output_enable = val;
	return 0;
}

static void watchdog_ref(struct watchdog_device *wddev)
{
	struct sch56xx_watchdog_data *data = watchdog_get_drvdata(wddev);

	kref_get(&data->kref);
}

static void watchdog_unref(struct watchdog_device *wddev)
{
	struct sch56xx_watchdog_data *data = watchdog_get_drvdata(wddev);

	kref_put(&data->kref, watchdog_release_resources);
}

static const struct watchdog_ops watchdog_ops = {
	.owner		= THIS_MODULE,
	.start		= watchdog_start,
	.stop		= watchdog_stop,
	.ping		= watchdog_trigger,
	.set_timeout	= watchdog_set_timeout,
	.ref		= watchdog_ref,
	.unref		= watchdog_unref,
};

struct sch56xx_watchdog_data *sch56xx_watchdog_register(struct device *parent,
	u16 addr, u32 revision, struct mutex *io_lock, int check_enabled)
{
	struct sch56xx_watchdog_data *data;
	int err, control, output_enable;

	/*                              */
	mutex_lock(io_lock);
	control =
		sch56xx_read_virtual_reg(addr, SCH56XX_REG_WDOG_CONTROL);
	output_enable =
		sch56xx_read_virtual_reg(addr, SCH56XX_REG_WDOG_OUTPUT_ENABLE);
	mutex_unlock(io_lock);

	if (control < 0)
		return NULL;
	if (output_enable < 0)
		return NULL;
	if (check_enabled && !(output_enable & SCH56XX_WDOG_OUTPUT_ENABLE)) {
		pr_warn("Watchdog not enabled by BIOS, not registering\n");
		return NULL;
	}

	data = kzalloc(sizeof(struct sch56xx_watchdog_data), GFP_KERNEL);
	if (!data)
		return NULL;

	data->addr = addr;
	data->io_lock = io_lock;
	kref_init(&data->kref);

	strlcpy(data->wdinfo.identity, "sch56xx watchdog",
		sizeof(data->wdinfo.identity));
	data->wdinfo.firmware_version = revision;
	data->wdinfo.options = WDIOF_KEEPALIVEPING | WDIOF_SETTIMEOUT;
	if (!nowayout)
		data->wdinfo.options |= WDIOF_MAGICCLOSE;

	data->wddev.info = &data->wdinfo;
	data->wddev.ops = &watchdog_ops;
	data->wddev.parent = parent;
	data->wddev.timeout = 60;
	data->wddev.min_timeout = 1;
	data->wddev.max_timeout = 255 * 60;
	if (nowayout)
		set_bit(WDOG_NO_WAY_OUT, &data->wddev.status);
	if (output_enable & SCH56XX_WDOG_OUTPUT_ENABLE)
		set_bit(WDOG_ACTIVE, &data->wddev.status);

	/*                                                                   
                                                        
                                                          */
	if (control & SCH56XX_WDOG_TIME_BASE_SEC)
		data->watchdog_preset = 60; /*         */
	else
		data->watchdog_preset = 1; /*        */

	data->watchdog_control = control;
	data->watchdog_output_enable = output_enable;

	watchdog_set_drvdata(&data->wddev, data);
	err = watchdog_register_device(&data->wddev);
	if (err) {
		pr_err("Registering watchdog chardev: %d\n", err);
		kfree(data);
		return NULL;
	}

	return data;
}
EXPORT_SYMBOL(sch56xx_watchdog_register);

void sch56xx_watchdog_unregister(struct sch56xx_watchdog_data *data)
{
	watchdog_unregister_device(&data->wddev);
	kref_put(&data->kref, watchdog_release_resources);
	/*                                                       */
}
EXPORT_SYMBOL(sch56xx_watchdog_unregister);

/*
                                              
 */

static int __init sch56xx_find(int sioaddr, const char **name)
{
	u8 devid;
	unsigned short address;
	int err;

	err = superio_enter(sioaddr);
	if (err)
		return err;

	devid = superio_inb(sioaddr, SIO_REG_DEVID);
	switch (devid) {
	case SIO_SCH5627_ID:
		*name = "sch5627";
		break;
	case SIO_SCH5636_ID:
		*name = "sch5636";
		break;
	default:
		pr_debug("Unsupported device id: 0x%02x\n",
			 (unsigned int)devid);
		err = -ENODEV;
		goto exit;
	}

	superio_select(sioaddr, SIO_SCH56XX_LD_EM);

	if (!(superio_inb(sioaddr, SIO_REG_ENABLE) & 0x01)) {
		pr_warn("Device not activated\n");
		err = -ENODEV;
		goto exit;
	}

	/*
                                                                    
                                      
  */
	address = superio_inb(sioaddr, SIO_REG_ADDR) |
		   superio_inb(sioaddr, SIO_REG_ADDR + 1) << 8;
	if (address == 0) {
		pr_warn("Base address not set\n");
		err = -ENODEV;
		goto exit;
	}
	err = address;

exit:
	superio_exit(sioaddr);
	return err;
}

static int __init sch56xx_device_add(int address, const char *name)
{
	struct resource res = {
		.start	= address,
		.end	= address + REGION_LENGTH - 1,
		.flags	= IORESOURCE_IO,
	};
	int err;

	sch56xx_pdev = platform_device_alloc(name, address);
	if (!sch56xx_pdev)
		return -ENOMEM;

	res.name = sch56xx_pdev->name;
	err = acpi_check_resource_conflict(&res);
	if (err)
		goto exit_device_put;

	err = platform_device_add_resources(sch56xx_pdev, &res, 1);
	if (err) {
		pr_err("Device resource addition failed\n");
		goto exit_device_put;
	}

	err = platform_device_add(sch56xx_pdev);
	if (err) {
		pr_err("Device addition failed\n");
		goto exit_device_put;
	}

	return 0;

exit_device_put:
	platform_device_put(sch56xx_pdev);

	return err;
}

static int __init sch56xx_init(void)
{
	int address;
	const char *name = NULL;

	address = sch56xx_find(0x4e, &name);
	if (address < 0)
		address = sch56xx_find(0x2e, &name);
	if (address < 0)
		return address;

	return sch56xx_device_add(address, name);
}

static void __exit sch56xx_exit(void)
{
	platform_device_unregister(sch56xx_pdev);
}

MODULE_DESCRIPTION("SMSC SCH56xx Hardware Monitoring Common Code");
MODULE_AUTHOR("Hans de Goede <hdegoede@redhat.com>");
MODULE_LICENSE("GPL");

module_init(sch56xx_init);
module_exit(sch56xx_exit);
