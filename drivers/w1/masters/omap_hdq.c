/*
 * drivers/w1/masters/omap_hdq.c
 *
 * Copyright (C) 2007,2012 Texas Instruments, Inc.
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2. This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 *
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/sched.h>
#include <linux/pm_runtime.h>

#include "../w1.h"
#include "../w1_int.h"

#define	MOD_NAME	"OMAP_HDQ:"

#define OMAP_HDQ_REVISION			0x00
#define OMAP_HDQ_TX_DATA			0x04
#define OMAP_HDQ_RX_DATA			0x08
#define OMAP_HDQ_CTRL_STATUS			0x0c
#define OMAP_HDQ_CTRL_STATUS_INTERRUPTMASK	(1<<6)
#define OMAP_HDQ_CTRL_STATUS_CLOCKENABLE	(1<<5)
#define OMAP_HDQ_CTRL_STATUS_GO			(1<<4)
#define OMAP_HDQ_CTRL_STATUS_INITIALIZATION	(1<<2)
#define OMAP_HDQ_CTRL_STATUS_DIR		(1<<1)
#define OMAP_HDQ_CTRL_STATUS_MODE		(1<<0)
#define OMAP_HDQ_INT_STATUS			0x10
#define OMAP_HDQ_INT_STATUS_TXCOMPLETE		(1<<2)
#define OMAP_HDQ_INT_STATUS_RXCOMPLETE		(1<<1)
#define OMAP_HDQ_INT_STATUS_TIMEOUT		(1<<0)
#define OMAP_HDQ_SYSCONFIG			0x14
#define OMAP_HDQ_SYSCONFIG_SOFTRESET		(1<<1)
#define OMAP_HDQ_SYSCONFIG_AUTOIDLE		(1<<0)
#define OMAP_HDQ_SYSSTATUS			0x18
#define OMAP_HDQ_SYSSTATUS_RESETDONE		(1<<0)

#define OMAP_HDQ_FLAG_CLEAR			0
#define OMAP_HDQ_FLAG_SET			1
#define OMAP_HDQ_TIMEOUT			(HZ/5)

#define OMAP_HDQ_MAX_USER			4

static DECLARE_WAIT_QUEUE_HEAD(hdq_wait_queue);
static int w1_id;

struct hdq_data {
	struct device		*dev;
	void __iomem		*hdq_base;
	/*                    */
	struct  mutex		hdq_mutex;
	int			hdq_usecount;
	u8			hdq_irqstatus;
	/*             */
	spinlock_t		hdq_spinlock;
	/*
                                                              
                                                              
                           
  */
	int			init_trans;
};

static int omap_hdq_probe(struct platform_device *pdev);
static int omap_hdq_remove(struct platform_device *pdev);

static struct platform_driver omap_hdq_driver = {
	.probe =	omap_hdq_probe,
	.remove =	omap_hdq_remove,
	.driver =	{
		.name =	"omap_hdq",
	},
};

static u8 omap_w1_read_byte(void *_hdq);
static void omap_w1_write_byte(void *_hdq, u8 byte);
static u8 omap_w1_reset_bus(void *_hdq);
static void omap_w1_search_bus(void *_hdq, struct w1_master *master_dev,
		u8 search_type,	w1_slave_found_callback slave_found);


static struct w1_bus_master omap_w1_master = {
	.read_byte	= omap_w1_read_byte,
	.write_byte	= omap_w1_write_byte,
	.reset_bus	= omap_w1_reset_bus,
	.search		= omap_w1_search_bus,
};

/*                           */
static inline u8 hdq_reg_in(struct hdq_data *hdq_data, u32 offset)
{
	return __raw_readl(hdq_data->hdq_base + offset);
}

static inline void hdq_reg_out(struct hdq_data *hdq_data, u32 offset, u8 val)
{
	__raw_writel(val, hdq_data->hdq_base + offset);
}

static inline u8 hdq_reg_merge(struct hdq_data *hdq_data, u32 offset,
			u8 val, u8 mask)
{
	u8 new_val = (__raw_readl(hdq_data->hdq_base + offset) & ~mask)
			| (val & mask);
	__raw_writel(new_val, hdq_data->hdq_base + offset);

	return new_val;
}

/*
                                            
                                                       
                                                               
                                                             
 */
static int hdq_wait_for_flag(struct hdq_data *hdq_data, u32 offset,
		u8 flag, u8 flag_set, u8 *status)
{
	int ret = 0;
	unsigned long timeout = jiffies + OMAP_HDQ_TIMEOUT;

	if (flag_set == OMAP_HDQ_FLAG_CLEAR) {
		/*                         */
		while (((*status = hdq_reg_in(hdq_data, offset)) & flag)
			&& time_before(jiffies, timeout)) {
			schedule_timeout_uninterruptible(1);
		}
		if (*status & flag)
			ret = -ETIMEDOUT;
	} else if (flag_set == OMAP_HDQ_FLAG_SET) {
		/*                       */
		while (!((*status = hdq_reg_in(hdq_data, offset)) & flag)
			&& time_before(jiffies, timeout)) {
			schedule_timeout_uninterruptible(1);
		}
		if (!(*status & flag))
			ret = -ETIMEDOUT;
	} else
		return -EINVAL;

	return ret;
}

/*                                                       */
static int hdq_write_byte(struct hdq_data *hdq_data, u8 val, u8 *status)
{
	int ret;
	u8 tmp_status;
	unsigned long irqflags;

	*status = 0;

	spin_lock_irqsave(&hdq_data->hdq_spinlock, irqflags);
	/*                                        */
	hdq_reg_in(hdq_data, OMAP_HDQ_INT_STATUS);
	/*                                  */
	hdq_data->hdq_irqstatus = 0;
	spin_unlock_irqrestore(&hdq_data->hdq_spinlock, irqflags);

	hdq_reg_out(hdq_data, OMAP_HDQ_TX_DATA, val);

	/*                */
	hdq_reg_merge(hdq_data, OMAP_HDQ_CTRL_STATUS, OMAP_HDQ_CTRL_STATUS_GO,
		OMAP_HDQ_CTRL_STATUS_DIR | OMAP_HDQ_CTRL_STATUS_GO);
	/*                             */
	ret = wait_event_timeout(hdq_wait_queue,
		hdq_data->hdq_irqstatus, OMAP_HDQ_TIMEOUT);
	if (ret == 0) {
		dev_dbg(hdq_data->dev, "TX wait elapsed\n");
		ret = -ETIMEDOUT;
		goto out;
	}

	*status = hdq_data->hdq_irqstatus;
	/*                 */
	if (!(*status & OMAP_HDQ_INT_STATUS_TXCOMPLETE)) {
		dev_dbg(hdq_data->dev, "timeout waiting for"
			" TXCOMPLETE/RXCOMPLETE, %x", *status);
		ret = -ETIMEDOUT;
		goto out;
	}

	/*                                    */
	ret = hdq_wait_for_flag(hdq_data, OMAP_HDQ_CTRL_STATUS,
			OMAP_HDQ_CTRL_STATUS_GO,
			OMAP_HDQ_FLAG_CLEAR, &tmp_status);
	if (ret) {
		dev_dbg(hdq_data->dev, "timeout waiting GO bit"
			" return to zero, %x", tmp_status);
	}

out:
	return ret;
}

/*                               */
static irqreturn_t hdq_isr(int irq, void *_hdq)
{
	struct hdq_data *hdq_data = _hdq;
	unsigned long irqflags;

	spin_lock_irqsave(&hdq_data->hdq_spinlock, irqflags);
	hdq_data->hdq_irqstatus = hdq_reg_in(hdq_data, OMAP_HDQ_INT_STATUS);
	spin_unlock_irqrestore(&hdq_data->hdq_spinlock, irqflags);
	dev_dbg(hdq_data->dev, "hdq_isr: %x", hdq_data->hdq_irqstatus);

	if (hdq_data->hdq_irqstatus &
		(OMAP_HDQ_INT_STATUS_TXCOMPLETE | OMAP_HDQ_INT_STATUS_RXCOMPLETE
		| OMAP_HDQ_INT_STATUS_TIMEOUT)) {
		/*                          */
		wake_up(&hdq_wait_queue);
	}

	return IRQ_HANDLED;
}

/*                                 */
static u8 omap_w1_reset_bus(void *_hdq)
{
	return 0;
}

/*                             */
static void omap_w1_search_bus(void *_hdq, struct w1_master *master_dev,
		u8 search_type, w1_slave_found_callback slave_found)
{
	u64 module_id, rn_le, cs, id;

	if (w1_id)
		module_id = w1_id;
	else
		module_id = 0x1;

	rn_le = cpu_to_le64(module_id);
	/*
                                             
                                               
  */
	cs = w1_calc_crc8((u8 *)&rn_le, 7);
	id = (cs << 56) | module_id;

	slave_found(master_dev, id);
}

static int _omap_hdq_reset(struct hdq_data *hdq_data)
{
	int ret;
	u8 tmp_status;

	hdq_reg_out(hdq_data, OMAP_HDQ_SYSCONFIG, OMAP_HDQ_SYSCONFIG_SOFTRESET);
	/*
                                    
                                                                         
                                                                      
              
  */
	hdq_reg_out(hdq_data, OMAP_HDQ_CTRL_STATUS,
		OMAP_HDQ_CTRL_STATUS_CLOCKENABLE |
		OMAP_HDQ_CTRL_STATUS_INTERRUPTMASK);

	/*                            */
	ret = hdq_wait_for_flag(hdq_data, OMAP_HDQ_SYSSTATUS,
		OMAP_HDQ_SYSSTATUS_RESETDONE, OMAP_HDQ_FLAG_SET, &tmp_status);
	if (ret)
		dev_dbg(hdq_data->dev, "timeout waiting HDQ reset, %x",
				tmp_status);
	else {
		hdq_reg_out(hdq_data, OMAP_HDQ_CTRL_STATUS,
			OMAP_HDQ_CTRL_STATUS_CLOCKENABLE |
			OMAP_HDQ_CTRL_STATUS_INTERRUPTMASK);
		hdq_reg_out(hdq_data, OMAP_HDQ_SYSCONFIG,
			OMAP_HDQ_SYSCONFIG_AUTOIDLE);
	}

	return ret;
}

/*                                 */
static int omap_hdq_break(struct hdq_data *hdq_data)
{
	int ret = 0;
	u8 tmp_status;
	unsigned long irqflags;

	ret = mutex_lock_interruptible(&hdq_data->hdq_mutex);
	if (ret < 0) {
		dev_dbg(hdq_data->dev, "Could not acquire mutex\n");
		ret = -EINTR;
		goto rtn;
	}

	spin_lock_irqsave(&hdq_data->hdq_spinlock, irqflags);
	/*                                        */
	hdq_reg_in(hdq_data, OMAP_HDQ_INT_STATUS);
	/*                                  */
	hdq_data->hdq_irqstatus = 0;
	spin_unlock_irqrestore(&hdq_data->hdq_spinlock, irqflags);

	/*                         */
	hdq_reg_merge(hdq_data, OMAP_HDQ_CTRL_STATUS,
		OMAP_HDQ_CTRL_STATUS_INITIALIZATION | OMAP_HDQ_CTRL_STATUS_GO,
		OMAP_HDQ_CTRL_STATUS_DIR | OMAP_HDQ_CTRL_STATUS_INITIALIZATION |
		OMAP_HDQ_CTRL_STATUS_GO);

	/*                          */
	ret = wait_event_timeout(hdq_wait_queue,
		hdq_data->hdq_irqstatus, OMAP_HDQ_TIMEOUT);
	if (ret == 0) {
		dev_dbg(hdq_data->dev, "break wait elapsed\n");
		ret = -EINTR;
		goto out;
	}

	tmp_status = hdq_data->hdq_irqstatus;
	/*                 */
	if (!(tmp_status & OMAP_HDQ_INT_STATUS_TIMEOUT)) {
		dev_dbg(hdq_data->dev, "timeout waiting for TIMEOUT, %x",
				tmp_status);
		ret = -ETIMEDOUT;
		goto out;
	}
	/*
                                                  
                                               
  */
	ret = hdq_wait_for_flag(hdq_data, OMAP_HDQ_CTRL_STATUS,
			OMAP_HDQ_CTRL_STATUS_INITIALIZATION |
			OMAP_HDQ_CTRL_STATUS_GO, OMAP_HDQ_FLAG_CLEAR,
			&tmp_status);
	if (ret)
		dev_dbg(hdq_data->dev, "timeout waiting INIT&GO bits"
			" return to zero, %x", tmp_status);

out:
	mutex_unlock(&hdq_data->hdq_mutex);
rtn:
	return ret;
}

static int hdq_read_byte(struct hdq_data *hdq_data, u8 *val)
{
	int ret = 0;
	u8 status;

	ret = mutex_lock_interruptible(&hdq_data->hdq_mutex);
	if (ret < 0) {
		ret = -EINTR;
		goto rtn;
	}

	if (!hdq_data->hdq_usecount) {
		ret = -EINVAL;
		goto out;
	}

	if (!(hdq_data->hdq_irqstatus & OMAP_HDQ_INT_STATUS_RXCOMPLETE)) {
		hdq_reg_merge(hdq_data, OMAP_HDQ_CTRL_STATUS,
			OMAP_HDQ_CTRL_STATUS_DIR | OMAP_HDQ_CTRL_STATUS_GO,
			OMAP_HDQ_CTRL_STATUS_DIR | OMAP_HDQ_CTRL_STATUS_GO);
		/*
                                       
   */
		wait_event_timeout(hdq_wait_queue,
				   (hdq_data->hdq_irqstatus
				    & OMAP_HDQ_INT_STATUS_RXCOMPLETE),
				   OMAP_HDQ_TIMEOUT);

		hdq_reg_merge(hdq_data, OMAP_HDQ_CTRL_STATUS, 0,
			OMAP_HDQ_CTRL_STATUS_DIR);
		status = hdq_data->hdq_irqstatus;
		/*                 */
		if (!(status & OMAP_HDQ_INT_STATUS_RXCOMPLETE)) {
			dev_dbg(hdq_data->dev, "timeout waiting for"
				" RXCOMPLETE, %x", status);
			ret = -ETIMEDOUT;
			goto out;
		}
	}
	/*                                */
	*val = hdq_reg_in(hdq_data, OMAP_HDQ_RX_DATA);
out:
	mutex_unlock(&hdq_data->hdq_mutex);
rtn:
	return ret;

}

/*                                                  */
static int omap_hdq_get(struct hdq_data *hdq_data)
{
	int ret = 0;

	ret = mutex_lock_interruptible(&hdq_data->hdq_mutex);
	if (ret < 0) {
		ret = -EINTR;
		goto rtn;
	}

	if (OMAP_HDQ_MAX_USER == hdq_data->hdq_usecount) {
		dev_dbg(hdq_data->dev, "attempt to exceed the max use count");
		ret = -EINVAL;
		goto out;
	} else {
		hdq_data->hdq_usecount++;
		try_module_get(THIS_MODULE);
		if (1 == hdq_data->hdq_usecount) {

			pm_runtime_get_sync(hdq_data->dev);

			/*                               */
			if (!(hdq_reg_in(hdq_data, OMAP_HDQ_SYSSTATUS) &
				OMAP_HDQ_SYSSTATUS_RESETDONE)) {
				ret = _omap_hdq_reset(hdq_data);
				if (ret)
					/*                   */
					hdq_data->hdq_usecount--;
			} else {
				/*                                 */
				hdq_reg_out(hdq_data, OMAP_HDQ_CTRL_STATUS,
					OMAP_HDQ_CTRL_STATUS_CLOCKENABLE |
					OMAP_HDQ_CTRL_STATUS_INTERRUPTMASK);
				hdq_reg_out(hdq_data, OMAP_HDQ_SYSCONFIG,
					OMAP_HDQ_SYSCONFIG_AUTOIDLE);
				hdq_reg_in(hdq_data, OMAP_HDQ_INT_STATUS);
			}
		}
	}

out:
	mutex_unlock(&hdq_data->hdq_mutex);
rtn:
	return ret;
}

/*                              */
static int omap_hdq_put(struct hdq_data *hdq_data)
{
	int ret = 0;

	ret = mutex_lock_interruptible(&hdq_data->hdq_mutex);
	if (ret < 0)
		return -EINTR;

	if (0 == hdq_data->hdq_usecount) {
		dev_dbg(hdq_data->dev, "attempt to decrement use count"
			" when it is zero");
		ret = -EINVAL;
	} else {
		hdq_data->hdq_usecount--;
		module_put(THIS_MODULE);
		if (0 == hdq_data->hdq_usecount)
			pm_runtime_put_sync(hdq_data->dev);
	}
	mutex_unlock(&hdq_data->hdq_mutex);

	return ret;
}

/*                                     */
static u8 omap_w1_read_byte(void *_hdq)
{
	struct hdq_data *hdq_data = _hdq;
	u8 val = 0;
	int ret;

	ret = hdq_read_byte(hdq_data, &val);
	if (ret) {
		ret = mutex_lock_interruptible(&hdq_data->hdq_mutex);
		if (ret < 0) {
			dev_dbg(hdq_data->dev, "Could not acquire mutex\n");
			return -EINTR;
		}
		hdq_data->init_trans = 0;
		mutex_unlock(&hdq_data->hdq_mutex);
		omap_hdq_put(hdq_data);
		return -1;
	}

	/*                                              */
	if (hdq_data->init_trans) {
		ret = mutex_lock_interruptible(&hdq_data->hdq_mutex);
		if (ret < 0) {
			dev_dbg(hdq_data->dev, "Could not acquire mutex\n");
			return -EINTR;
		}
		hdq_data->init_trans = 0;
		mutex_unlock(&hdq_data->hdq_mutex);
		omap_hdq_put(hdq_data);
	}

	return val;
}

/*                                    */
static void omap_w1_write_byte(void *_hdq, u8 byte)
{
	struct hdq_data *hdq_data = _hdq;
	int ret;
	u8 status;

	/*                                        */
	if (hdq_data->init_trans == 0)
		omap_hdq_get(hdq_data);

	ret = mutex_lock_interruptible(&hdq_data->hdq_mutex);
	if (ret < 0) {
		dev_dbg(hdq_data->dev, "Could not acquire mutex\n");
		return;
	}
	hdq_data->init_trans++;
	mutex_unlock(&hdq_data->hdq_mutex);

	ret = hdq_write_byte(hdq_data, byte, &status);
	if (ret < 0) {
		dev_dbg(hdq_data->dev, "TX failure:Ctrl status %x\n", status);
		return;
	}

	/*                                                    */
	if (hdq_data->init_trans > 1) {
		omap_hdq_put(hdq_data);
		ret = mutex_lock_interruptible(&hdq_data->hdq_mutex);
		if (ret < 0) {
			dev_dbg(hdq_data->dev, "Could not acquire mutex\n");
			return;
		}
		hdq_data->init_trans = 0;
		mutex_unlock(&hdq_data->hdq_mutex);
	}
}

static int omap_hdq_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct hdq_data *hdq_data;
	struct resource *res;
	int ret, irq;
	u8 rev;

	hdq_data = devm_kzalloc(dev, sizeof(*hdq_data), GFP_KERNEL);
	if (!hdq_data) {
		dev_dbg(&pdev->dev, "unable to allocate memory\n");
		return -ENOMEM;
	}

	hdq_data->dev = dev;
	platform_set_drvdata(pdev, hdq_data);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	hdq_data->hdq_base = devm_ioremap_resource(dev, res);
	if (IS_ERR(hdq_data->hdq_base))
		return PTR_ERR(hdq_data->hdq_base);

	hdq_data->hdq_usecount = 0;
	mutex_init(&hdq_data->hdq_mutex);

	pm_runtime_enable(&pdev->dev);
	pm_runtime_get_sync(&pdev->dev);

	rev = hdq_reg_in(hdq_data, OMAP_HDQ_REVISION);
	dev_info(&pdev->dev, "OMAP HDQ Hardware Rev %c.%c. Driver in %s mode\n",
		(rev >> 4) + '0', (rev & 0x0f) + '0', "Interrupt");

	spin_lock_init(&hdq_data->hdq_spinlock);

	irq = platform_get_irq(pdev, 0);
	if (irq	< 0) {
		ret = -ENXIO;
		goto err_irq;
	}

	ret = devm_request_irq(dev, irq, hdq_isr, IRQF_DISABLED,
			"omap_hdq", hdq_data);
	if (ret < 0) {
		dev_dbg(&pdev->dev, "could not request irq\n");
		goto err_irq;
	}

	omap_hdq_break(hdq_data);

	pm_runtime_put_sync(&pdev->dev);

	omap_w1_master.data = hdq_data;

	ret = w1_add_master_device(&omap_w1_master);
	if (ret) {
		dev_dbg(&pdev->dev, "Failure in registering w1 master\n");
		goto err_w1;
	}

	return 0;

err_irq:
	pm_runtime_put_sync(&pdev->dev);
err_w1:
	pm_runtime_disable(&pdev->dev);

	return ret;
}

static int omap_hdq_remove(struct platform_device *pdev)
{
	struct hdq_data *hdq_data = platform_get_drvdata(pdev);

	mutex_lock(&hdq_data->hdq_mutex);

	if (hdq_data->hdq_usecount) {
		dev_dbg(&pdev->dev, "removed when use count is not zero\n");
		mutex_unlock(&hdq_data->hdq_mutex);
		return -EBUSY;
	}

	mutex_unlock(&hdq_data->hdq_mutex);

	/*                          */
	pm_runtime_disable(&pdev->dev);

	return 0;
}

module_platform_driver(omap_hdq_driver);

module_param(w1_id, int, S_IRUSR);
MODULE_PARM_DESC(w1_id, "1-wire id for the slave detection");

MODULE_AUTHOR("Texas Instruments");
MODULE_DESCRIPTION("HDQ driver Library");
MODULE_LICENSE("GPL");
