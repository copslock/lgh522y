/* linux/drivers/i2c/busses/i2c-s3c2410.c
 *
 * Copyright (C) 2004,2005,2009 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *
 * S3C2410 I2C Controller
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/clk.h>
#include <linux/cpufreq.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/of_i2c.h>
#include <linux/of_gpio.h>
#include <linux/pinctrl/consumer.h>

#include <asm/irq.h>

#include <linux/platform_data/i2c-s3c2410.h>

/*                                                               */

#define S3C2410_IICCON			0x00
#define S3C2410_IICSTAT			0x04
#define S3C2410_IICADD			0x08
#define S3C2410_IICDS			0x0C
#define S3C2440_IICLC			0x10

#define S3C2410_IICCON_ACKEN		(1 << 7)
#define S3C2410_IICCON_TXDIV_16		(0 << 6)
#define S3C2410_IICCON_TXDIV_512	(1 << 6)
#define S3C2410_IICCON_IRQEN		(1 << 5)
#define S3C2410_IICCON_IRQPEND		(1 << 4)
#define S3C2410_IICCON_SCALE(x)		((x) & 0xf)
#define S3C2410_IICCON_SCALEMASK	(0xf)

#define S3C2410_IICSTAT_MASTER_RX	(2 << 6)
#define S3C2410_IICSTAT_MASTER_TX	(3 << 6)
#define S3C2410_IICSTAT_SLAVE_RX	(0 << 6)
#define S3C2410_IICSTAT_SLAVE_TX	(1 << 6)
#define S3C2410_IICSTAT_MODEMASK	(3 << 6)

#define S3C2410_IICSTAT_START		(1 << 5)
#define S3C2410_IICSTAT_BUSBUSY		(1 << 5)
#define S3C2410_IICSTAT_TXRXEN		(1 << 4)
#define S3C2410_IICSTAT_ARBITR		(1 << 3)
#define S3C2410_IICSTAT_ASSLAVE		(1 << 2)
#define S3C2410_IICSTAT_ADDR0		(1 << 1)
#define S3C2410_IICSTAT_LASTBIT		(1 << 0)

#define S3C2410_IICLC_SDA_DELAY0	(0 << 0)
#define S3C2410_IICLC_SDA_DELAY5	(1 << 0)
#define S3C2410_IICLC_SDA_DELAY10	(2 << 0)
#define S3C2410_IICLC_SDA_DELAY15	(3 << 0)
#define S3C2410_IICLC_SDA_DELAY_MASK	(3 << 0)

#define S3C2410_IICLC_FILTER_ON		(1 << 2)

/*                                                                           */
#define QUIRK_S3C2440		(1 << 0)
#define QUIRK_HDMIPHY		(1 << 1)
#define QUIRK_NO_GPIO		(1 << 2)

/*                                                              */
#define S3C2410_IDLE_TIMEOUT	5000

/*                      */
enum s3c24xx_i2c_state {
	STATE_IDLE,
	STATE_START,
	STATE_READ,
	STATE_WRITE,
	STATE_STOP
};

struct s3c24xx_i2c {
	wait_queue_head_t	wait;
	unsigned int            quirks;
	unsigned int		suspended:1;

	struct i2c_msg		*msg;
	unsigned int		msg_num;
	unsigned int		msg_idx;
	unsigned int		msg_ptr;

	unsigned int		tx_setup;
	unsigned int		irq;

	enum s3c24xx_i2c_state	state;
	unsigned long		clkrate;

	void __iomem		*regs;
	struct clk		*clk;
	struct device		*dev;
	struct i2c_adapter	adap;

	struct s3c2410_platform_i2c	*pdata;
	int			gpios[2];
	struct pinctrl          *pctrl;
#ifdef CONFIG_CPU_FREQ
	struct notifier_block	freq_transition;
#endif
};

static struct platform_device_id s3c24xx_driver_ids[] = {
	{
		.name		= "s3c2410-i2c",
		.driver_data	= 0,
	}, {
		.name		= "s3c2440-i2c",
		.driver_data	= QUIRK_S3C2440,
	}, {
		.name		= "s3c2440-hdmiphy-i2c",
		.driver_data	= QUIRK_S3C2440 | QUIRK_HDMIPHY | QUIRK_NO_GPIO,
	}, { },
};
MODULE_DEVICE_TABLE(platform, s3c24xx_driver_ids);

#ifdef CONFIG_OF
static const struct of_device_id s3c24xx_i2c_match[] = {
	{ .compatible = "samsung,s3c2410-i2c", .data = (void *)0 },
	{ .compatible = "samsung,s3c2440-i2c", .data = (void *)QUIRK_S3C2440 },
	{ .compatible = "samsung,s3c2440-hdmiphy-i2c",
	  .data = (void *)(QUIRK_S3C2440 | QUIRK_HDMIPHY | QUIRK_NO_GPIO) },
	{ .compatible = "samsung,exynos5440-i2c",
	  .data = (void *)(QUIRK_S3C2440 | QUIRK_NO_GPIO) },
	{},
};
MODULE_DEVICE_TABLE(of, s3c24xx_i2c_match);
#endif

/*                          
  
                                                                          
*/

static inline unsigned int s3c24xx_get_device_quirks(struct platform_device *pdev)
{
	if (pdev->dev.of_node) {
		const struct of_device_id *match;
		match = of_match_node(s3c24xx_i2c_match, pdev->dev.of_node);
		return (unsigned int)match->data;
	}

	return platform_get_device_id(pdev)->driver_data;
}

/*                            
  
                                                                            
                      
*/

static inline void s3c24xx_i2c_master_complete(struct s3c24xx_i2c *i2c, int ret)
{
	dev_dbg(i2c->dev, "master_complete %d\n", ret);

	i2c->msg_ptr = 0;
	i2c->msg = NULL;
	i2c->msg_idx++;
	i2c->msg_num = 0;
	if (ret)
		i2c->msg_idx = ret;

	wake_up(&i2c->wait);
}

static inline void s3c24xx_i2c_disable_ack(struct s3c24xx_i2c *i2c)
{
	unsigned long tmp;

	tmp = readl(i2c->regs + S3C2410_IICCON);
	writel(tmp & ~S3C2410_IICCON_ACKEN, i2c->regs + S3C2410_IICCON);
}

static inline void s3c24xx_i2c_enable_ack(struct s3c24xx_i2c *i2c)
{
	unsigned long tmp;

	tmp = readl(i2c->regs + S3C2410_IICCON);
	writel(tmp | S3C2410_IICCON_ACKEN, i2c->regs + S3C2410_IICCON);
}

/*                              */

static inline void s3c24xx_i2c_disable_irq(struct s3c24xx_i2c *i2c)
{
	unsigned long tmp;

	tmp = readl(i2c->regs + S3C2410_IICCON);
	writel(tmp & ~S3C2410_IICCON_IRQEN, i2c->regs + S3C2410_IICCON);
}

static inline void s3c24xx_i2c_enable_irq(struct s3c24xx_i2c *i2c)
{
	unsigned long tmp;

	tmp = readl(i2c->regs + S3C2410_IICCON);
	writel(tmp | S3C2410_IICCON_IRQEN, i2c->regs + S3C2410_IICCON);
}


/*                          
  
                                          
*/

static void s3c24xx_i2c_message_start(struct s3c24xx_i2c *i2c,
				      struct i2c_msg *msg)
{
	unsigned int addr = (msg->addr & 0x7f) << 1;
	unsigned long stat;
	unsigned long iiccon;

	stat = 0;
	stat |=  S3C2410_IICSTAT_TXRXEN;

	if (msg->flags & I2C_M_RD) {
		stat |= S3C2410_IICSTAT_MASTER_RX;
		addr |= 1;
	} else
		stat |= S3C2410_IICSTAT_MASTER_TX;

	if (msg->flags & I2C_M_REV_DIR_ADDR)
		addr ^= 1;

	/*                                            */
	s3c24xx_i2c_enable_ack(i2c);

	iiccon = readl(i2c->regs + S3C2410_IICCON);
	writel(stat, i2c->regs + S3C2410_IICSTAT);

	dev_dbg(i2c->dev, "START: %08lx to IICSTAT, %02x to DS\n", stat, addr);
	writeb(addr, i2c->regs + S3C2410_IICDS);

	/*                                                           
                                      */

	ndelay(i2c->tx_setup);

	dev_dbg(i2c->dev, "iiccon, %08lx\n", iiccon);
	writel(iiccon, i2c->regs + S3C2410_IICCON);

	stat |= S3C2410_IICSTAT_START;
	writel(stat, i2c->regs + S3C2410_IICSTAT);
}

static inline void s3c24xx_i2c_stop(struct s3c24xx_i2c *i2c, int ret)
{
	unsigned long iicstat = readl(i2c->regs + S3C2410_IICSTAT);

	dev_dbg(i2c->dev, "STOP\n");

	/*
                                                        
                                                       
                                    
                                                   
                                     
   
                                                                
   
                                                         
                                                                
                                                                    
                                                                      
                                               
                                                                 
                                                                   
            
   
                                                                        
                                                                  
                                                                      
                                                                     
                                                          
   
                                                                       
                                                                   
                                                                     
                                                                   
                                                               
   
                                                                  
                                                                    
                                                                      
  */
	if (i2c->quirks & QUIRK_HDMIPHY) {
		/*                           */
		iicstat &= ~S3C2410_IICSTAT_TXRXEN;
	} else {
		/*                   */
		iicstat &= ~S3C2410_IICSTAT_START;
	}
	writel(iicstat, i2c->regs + S3C2410_IICSTAT);

	i2c->state = STATE_STOP;

	s3c24xx_i2c_master_complete(i2c, ret);
	s3c24xx_i2c_disable_irq(i2c);
}

/*                                                              
                           */

/*             
  
                                                             
*/

static inline int is_lastmsg(struct s3c24xx_i2c *i2c)
{
	return i2c->msg_idx >= (i2c->msg_num - 1);
}

/*           
  
                                                                  
*/

static inline int is_msglast(struct s3c24xx_i2c *i2c)
{
	/*                                                             
                                                             
                                       */
	if (i2c->msg->flags & I2C_M_RECV_LEN && i2c->msg->len == 1)
		return 0;

	return i2c->msg_ptr == i2c->msg->len-1;
}

/*          
  
                                                            
*/

static inline int is_msgend(struct s3c24xx_i2c *i2c)
{
	return i2c->msg_ptr >= i2c->msg->len;
}

/*                     
  
                                               
 */

static int i2c_s3c_irq_nextbyte(struct s3c24xx_i2c *i2c, unsigned long iicstat)
{
	unsigned long tmp;
	unsigned char byte;
	int ret = 0;

	switch (i2c->state) {

	case STATE_IDLE:
		dev_err(i2c->dev, "%s: called in STATE_IDLE\n", __func__);
		goto out;

	case STATE_STOP:
		dev_err(i2c->dev, "%s: called in STATE_STOP\n", __func__);
		s3c24xx_i2c_disable_irq(i2c);
		goto out_ack;

	case STATE_START:
		/*                                                    
                                      
   */

		if (iicstat & S3C2410_IICSTAT_LASTBIT &&
		    !(i2c->msg->flags & I2C_M_IGNORE_NAK)) {
			/*                         */

			dev_dbg(i2c->dev, "ack was not received\n");
			s3c24xx_i2c_stop(i2c, -ENXIO);
			goto out_ack;
		}

		if (i2c->msg->flags & I2C_M_RD)
			i2c->state = STATE_READ;
		else
			i2c->state = STATE_WRITE;

		/*                                                 
                                                       */

		if (is_lastmsg(i2c) && i2c->msg->len == 0) {
			s3c24xx_i2c_stop(i2c, 0);
			goto out_ack;
		}

		if (i2c->state == STATE_READ)
			goto prepare_read;

		/*                                                    
                         */

	case STATE_WRITE:
		/*                                                   
                                                       
   */

		if (!(i2c->msg->flags & I2C_M_IGNORE_NAK)) {
			if (iicstat & S3C2410_IICSTAT_LASTBIT) {
				dev_dbg(i2c->dev, "WRITE: No Ack\n");

				s3c24xx_i2c_stop(i2c, -ECONNREFUSED);
				goto out_ack;
			}
		}

 retry_write:

		if (!is_msgend(i2c)) {
			byte = i2c->msg->buf[i2c->msg_ptr++];
			writeb(byte, i2c->regs + S3C2410_IICDS);

			/*                                          
                                                
                                               
                                              
                                            */

			ndelay(i2c->tx_setup);

		} else if (!is_lastmsg(i2c)) {
			/*                                       */

			dev_dbg(i2c->dev, "WRITE: Next Message\n");

			i2c->msg_ptr = 0;
			i2c->msg_idx++;
			i2c->msg++;

			/*                                               */
			if (i2c->msg->flags & I2C_M_NOSTART) {

				if (i2c->msg->flags & I2C_M_RD) {
					/*                               
                                     
                                 */

					s3c24xx_i2c_stop(i2c, -EINVAL);
				}

				goto retry_write;
			} else {
				/*                    */
				s3c24xx_i2c_message_start(i2c, i2c->msg);
				i2c->state = STATE_START;
			}

		} else {
			/*           */

			s3c24xx_i2c_stop(i2c, 0);
		}
		break;

	case STATE_READ:
		/*                                                
                                                        
                                    
   */

		byte = readb(i2c->regs + S3C2410_IICDS);
		i2c->msg->buf[i2c->msg_ptr++] = byte;

		/*                                                */
		if (i2c->msg->flags & I2C_M_RECV_LEN && i2c->msg->len == 1)
			i2c->msg->len += byte;
 prepare_read:
		if (is_msglast(i2c)) {
			/*                     */

			if (is_lastmsg(i2c))
				s3c24xx_i2c_disable_ack(i2c);

		} else if (is_msgend(i2c)) {
			/*                                               
                                     */

			if (is_lastmsg(i2c)) {
				/*                                      */
				dev_dbg(i2c->dev, "READ: Send Stop\n");

				s3c24xx_i2c_stop(i2c, 0);
			} else {
				/*                         */
				dev_dbg(i2c->dev, "READ: Next Transfer\n");

				i2c->msg_ptr = 0;
				i2c->msg_idx++;
				i2c->msg++;
			}
		}

		break;
	}

	/*                                                   */

 out_ack:
	tmp = readl(i2c->regs + S3C2410_IICCON);
	tmp &= ~S3C2410_IICCON_IRQPEND;
	writel(tmp, i2c->regs + S3C2410_IICCON);
 out:
	return ret;
}

/*                
  
                                  
*/

static irqreturn_t s3c24xx_i2c_irq(int irqno, void *dev_id)
{
	struct s3c24xx_i2c *i2c = dev_id;
	unsigned long status;
	unsigned long tmp;

	status = readl(i2c->regs + S3C2410_IICSTAT);

	if (status & S3C2410_IICSTAT_ARBITR) {
		/*                            */
		dev_err(i2c->dev, "deal with arbitration loss\n");
	}

	if (i2c->state == STATE_IDLE) {
		dev_dbg(i2c->dev, "IRQ: error i2c->state == IDLE\n");

		tmp = readl(i2c->regs + S3C2410_IICCON);
		tmp &= ~S3C2410_IICCON_IRQPEND;
		writel(tmp, i2c->regs +  S3C2410_IICCON);
		goto out;
	}

	/*                                                    
                                                       */

	i2c_s3c_irq_nextbyte(i2c, status);

 out:
	return IRQ_HANDLED;
}


/*                       
  
                                           
*/

static int s3c24xx_i2c_set_master(struct s3c24xx_i2c *i2c)
{
	unsigned long iicstat;
	int timeout = 400;

	while (timeout-- > 0) {
		iicstat = readl(i2c->regs + S3C2410_IICSTAT);

		if (!(iicstat & S3C2410_IICSTAT_BUSBUSY))
			return 0;

		msleep(1);
	}

	return -ETIMEDOUT;
}

/*                      
  
                                       
*/

static void s3c24xx_i2c_wait_idle(struct s3c24xx_i2c *i2c)
{
	unsigned long iicstat;
	ktime_t start, now;
	unsigned long delay;
	int spins;

	/*                                          */

	dev_dbg(i2c->dev, "waiting for bus idle\n");

	start = now = ktime_get();

	/*
                                                                      
                                                                       
                                        
   
                                                                       
                                                                   
  */
	spins = 3;
	iicstat = readl(i2c->regs + S3C2410_IICSTAT);
	while ((iicstat & S3C2410_IICSTAT_START) && --spins) {
		cpu_relax();
		iicstat = readl(i2c->regs + S3C2410_IICSTAT);
	}

	/*
                                                                  
                                                                       
                                                                      
                                                                 
                                    
  */
	delay = 1;
	while ((iicstat & S3C2410_IICSTAT_START) &&
	       ktime_us_delta(now, start) < S3C2410_IDLE_TIMEOUT) {
		usleep_range(delay, 2 * delay);
		if (delay < S3C2410_IDLE_TIMEOUT / 10)
			delay <<= 1;
		now = ktime_get();
		iicstat = readl(i2c->regs + S3C2410_IICSTAT);
	}

	if (iicstat & S3C2410_IICSTAT_START)
		dev_warn(i2c->dev, "timeout waiting for bus idle\n");
}

/*                   
  
                              
*/

static int s3c24xx_i2c_doxfer(struct s3c24xx_i2c *i2c,
			      struct i2c_msg *msgs, int num)
{
	unsigned long timeout;
	int ret;

	if (i2c->suspended)
		return -EIO;

	ret = s3c24xx_i2c_set_master(i2c);
	if (ret != 0) {
		dev_err(i2c->dev, "cannot get bus (error %d)\n", ret);
		ret = -EAGAIN;
		goto out;
	}

	i2c->msg     = msgs;
	i2c->msg_num = num;
	i2c->msg_ptr = 0;
	i2c->msg_idx = 0;
	i2c->state   = STATE_START;

	s3c24xx_i2c_enable_irq(i2c);
	s3c24xx_i2c_message_start(i2c, msgs);

	timeout = wait_event_timeout(i2c->wait, i2c->msg_num == 0, HZ * 5);

	ret = i2c->msg_idx;

	/*                                                   
                                  */

	if (timeout == 0)
		dev_dbg(i2c->dev, "timeout\n");
	else if (ret != num)
		dev_dbg(i2c->dev, "incomplete xfer (%d)\n", ret);

	/*                                            */
	if (i2c->quirks & QUIRK_HDMIPHY)
		goto out;

	s3c24xx_i2c_wait_idle(i2c);

 out:
	return ret;
}

/*                 
  
                                                                 
                                   
*/

static int s3c24xx_i2c_xfer(struct i2c_adapter *adap,
			struct i2c_msg *msgs, int num)
{
	struct s3c24xx_i2c *i2c = (struct s3c24xx_i2c *)adap->algo_data;
	int retry;
	int ret;

	pm_runtime_get_sync(&adap->dev);
	clk_prepare_enable(i2c->clk);

	for (retry = 0; retry < adap->retries; retry++) {

		ret = s3c24xx_i2c_doxfer(i2c, msgs, num);

		if (ret != -EAGAIN) {
			clk_disable_unprepare(i2c->clk);
			pm_runtime_put(&adap->dev);
			return ret;
		}

		dev_dbg(i2c->dev, "Retrying transmission (%d)\n", retry);

		udelay(100);
	}

	clk_disable_unprepare(i2c->clk);
	pm_runtime_put(&adap->dev);
	return -EREMOTEIO;
}

/*                               */
static u32 s3c24xx_i2c_func(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL | I2C_FUNC_NOSTART |
		I2C_FUNC_PROTOCOL_MANGLING;
}

/*                           */

static const struct i2c_algorithm s3c24xx_i2c_algorithm = {
	.master_xfer		= s3c24xx_i2c_xfer,
	.functionality		= s3c24xx_i2c_func,
};

/*                        
  
                                                    
*/

static int s3c24xx_i2c_calcdivisor(unsigned long clkin, unsigned int wanted,
				   unsigned int *div1, unsigned int *divs)
{
	unsigned int calc_divs = clkin / wanted;
	unsigned int calc_div1;

	if (calc_divs > (16*16))
		calc_div1 = 512;
	else
		calc_div1 = 16;

	calc_divs += calc_div1-1;
	calc_divs /= calc_div1;

	if (calc_divs == 0)
		calc_divs = 1;
	if (calc_divs > 17)
		calc_divs = 17;

	*divs = calc_divs;
	*div1 = calc_div1;

	return clkin / (calc_divs * calc_div1);
}

/*                      
  
                                                               
                                                                
                                                
*/

static int s3c24xx_i2c_clockrate(struct s3c24xx_i2c *i2c, unsigned int *got)
{
	struct s3c2410_platform_i2c *pdata = i2c->pdata;
	unsigned long clkin = clk_get_rate(i2c->clk);
	unsigned int divs, div1;
	unsigned long target_frequency;
	u32 iiccon;
	int freq;

	i2c->clkrate = clkin;
	clkin /= 1000;		/*                  */

	dev_dbg(i2c->dev, "pdata desired frequency %lu\n", pdata->frequency);

	target_frequency = pdata->frequency ? pdata->frequency : 100000;

	target_frequency /= 1000; /*                             */

	freq = s3c24xx_i2c_calcdivisor(clkin, target_frequency, &div1, &divs);

	if (freq > target_frequency) {
		dev_err(i2c->dev,
			"Unable to achieve desired frequency %luKHz."	\
			" Lowest achievable %dKHz\n", target_frequency, freq);
		return -EINVAL;
	}

	*got = freq;

	iiccon = readl(i2c->regs + S3C2410_IICCON);
	iiccon &= ~(S3C2410_IICCON_SCALEMASK | S3C2410_IICCON_TXDIV_512);
	iiccon |= (divs-1);

	if (div1 == 512)
		iiccon |= S3C2410_IICCON_TXDIV_512;

	writel(iiccon, i2c->regs + S3C2410_IICCON);

	if (i2c->quirks & QUIRK_S3C2440) {
		unsigned long sda_delay;

		if (pdata->sda_delay) {
			sda_delay = clkin * pdata->sda_delay;
			sda_delay = DIV_ROUND_UP(sda_delay, 1000000);
			sda_delay = DIV_ROUND_UP(sda_delay, 5);
			if (sda_delay > 3)
				sda_delay = 3;
			sda_delay |= S3C2410_IICLC_FILTER_ON;
		} else
			sda_delay = 0;

		dev_dbg(i2c->dev, "IICLC=%08lx\n", sda_delay);
		writel(sda_delay, i2c->regs + S3C2440_IICLC);
	}

	return 0;
}

#ifdef CONFIG_CPU_FREQ

#define freq_to_i2c(_n) container_of(_n, struct s3c24xx_i2c, freq_transition)

static int s3c24xx_i2c_cpufreq_transition(struct notifier_block *nb,
					  unsigned long val, void *data)
{
	struct s3c24xx_i2c *i2c = freq_to_i2c(nb);
	unsigned int got;
	int delta_f;
	int ret;

	delta_f = clk_get_rate(i2c->clk) - i2c->clkrate;

	/*                                                         
                                                             
                                                  
  */

	if ((val == CPUFREQ_POSTCHANGE && delta_f < 0) ||
	    (val == CPUFREQ_PRECHANGE && delta_f > 0)) {
		i2c_lock_adapter(&i2c->adap);
		ret = s3c24xx_i2c_clockrate(i2c, &got);
		i2c_unlock_adapter(&i2c->adap);

		if (ret < 0)
			dev_err(i2c->dev, "cannot find frequency\n");
		else
			dev_info(i2c->dev, "setting freq %d\n", got);
	}

	return 0;
}

static inline int s3c24xx_i2c_register_cpufreq(struct s3c24xx_i2c *i2c)
{
	i2c->freq_transition.notifier_call = s3c24xx_i2c_cpufreq_transition;

	return cpufreq_register_notifier(&i2c->freq_transition,
					 CPUFREQ_TRANSITION_NOTIFIER);
}

static inline void s3c24xx_i2c_deregister_cpufreq(struct s3c24xx_i2c *i2c)
{
	cpufreq_unregister_notifier(&i2c->freq_transition,
				    CPUFREQ_TRANSITION_NOTIFIER);
}

#else
static inline int s3c24xx_i2c_register_cpufreq(struct s3c24xx_i2c *i2c)
{
	return 0;
}

static inline void s3c24xx_i2c_deregister_cpufreq(struct s3c24xx_i2c *i2c)
{
}
#endif

#ifdef CONFIG_OF
static int s3c24xx_i2c_parse_dt_gpio(struct s3c24xx_i2c *i2c)
{
	int idx, gpio, ret;

	if (i2c->quirks & QUIRK_NO_GPIO)
		return 0;

	for (idx = 0; idx < 2; idx++) {
		gpio = of_get_gpio(i2c->dev->of_node, idx);
		if (!gpio_is_valid(gpio)) {
			dev_err(i2c->dev, "invalid gpio[%d]: %d\n", idx, gpio);
			goto free_gpio;
		}
		i2c->gpios[idx] = gpio;

		ret = gpio_request(gpio, "i2c-bus");
		if (ret) {
			dev_err(i2c->dev, "gpio [%d] request failed\n", gpio);
			goto free_gpio;
		}
	}
	return 0;

free_gpio:
	while (--idx >= 0)
		gpio_free(i2c->gpios[idx]);
	return -EINVAL;
}

static void s3c24xx_i2c_dt_gpio_free(struct s3c24xx_i2c *i2c)
{
	unsigned int idx;

	if (i2c->quirks & QUIRK_NO_GPIO)
		return;

	for (idx = 0; idx < 2; idx++)
		gpio_free(i2c->gpios[idx]);
}
#else
static int s3c24xx_i2c_parse_dt_gpio(struct s3c24xx_i2c *i2c)
{
	return 0;
}

static void s3c24xx_i2c_dt_gpio_free(struct s3c24xx_i2c *i2c)
{
}
#endif

/*                 
  
                                                            
*/

static int s3c24xx_i2c_init(struct s3c24xx_i2c *i2c)
{
	unsigned long iicon = S3C2410_IICCON_IRQEN | S3C2410_IICCON_ACKEN;
	struct s3c2410_platform_i2c *pdata;
	unsigned int freq;

	/*                      */

	pdata = i2c->pdata;

	/*                     */

	writeb(pdata->slave_addr, i2c->regs + S3C2410_IICADD);

	dev_info(i2c->dev, "slave address 0x%02x\n", pdata->slave_addr);

	writel(iicon, i2c->regs + S3C2410_IICCON);

	/*                                                   */

	if (s3c24xx_i2c_clockrate(i2c, &freq) != 0) {
		writel(0, i2c->regs + S3C2410_IICCON);
		dev_err(i2c->dev, "cannot meet bus frequency required\n");
		return -EINVAL;
	}

	/*                                                               */

	dev_info(i2c->dev, "bus frequency set to %d KHz\n", freq);
	dev_dbg(i2c->dev, "S3C2410_IICCON=0x%02lx\n", iicon);

	return 0;
}

#ifdef CONFIG_OF
/*                     
  
                                                             
*/

static void
s3c24xx_i2c_parse_dt(struct device_node *np, struct s3c24xx_i2c *i2c)
{
	struct s3c2410_platform_i2c *pdata = i2c->pdata;

	if (!np)
		return;

	pdata->bus_num = -1; /*                                        */
	of_property_read_u32(np, "samsung,i2c-sda-delay", &pdata->sda_delay);
	of_property_read_u32(np, "samsung,i2c-slave-addr", &pdata->slave_addr);
	of_property_read_u32(np, "samsung,i2c-max-bus-freq",
				(u32 *)&pdata->frequency);
}
#else
static void
s3c24xx_i2c_parse_dt(struct device_node *np, struct s3c24xx_i2c *i2c)
{
	return;
}
#endif

/*                  
  
                                                           
*/

static int s3c24xx_i2c_probe(struct platform_device *pdev)
{
	struct s3c24xx_i2c *i2c;
	struct s3c2410_platform_i2c *pdata = NULL;
	struct resource *res;
	int ret;

	if (!pdev->dev.of_node) {
		pdata = pdev->dev.platform_data;
		if (!pdata) {
			dev_err(&pdev->dev, "no platform data\n");
			return -EINVAL;
		}
	}

	i2c = devm_kzalloc(&pdev->dev, sizeof(struct s3c24xx_i2c), GFP_KERNEL);
	if (!i2c) {
		dev_err(&pdev->dev, "no memory for state\n");
		return -ENOMEM;
	}

	i2c->pdata = devm_kzalloc(&pdev->dev, sizeof(*pdata), GFP_KERNEL);
	if (!i2c->pdata) {
		dev_err(&pdev->dev, "no memory for platform data\n");
		return -ENOMEM;
	}

	i2c->quirks = s3c24xx_get_device_quirks(pdev);
	if (pdata)
		memcpy(i2c->pdata, pdata, sizeof(*pdata));
	else
		s3c24xx_i2c_parse_dt(pdev->dev.of_node, i2c);

	strlcpy(i2c->adap.name, "s3c2410-i2c", sizeof(i2c->adap.name));
	i2c->adap.owner   = THIS_MODULE;
	i2c->adap.algo    = &s3c24xx_i2c_algorithm;
	i2c->adap.retries = 2;
	i2c->adap.class   = I2C_CLASS_HWMON | I2C_CLASS_SPD;
	i2c->tx_setup     = 50;

	init_waitqueue_head(&i2c->wait);

	/*                              */

	i2c->dev = &pdev->dev;
	i2c->clk = devm_clk_get(&pdev->dev, "i2c");
	if (IS_ERR(i2c->clk)) {
		dev_err(&pdev->dev, "cannot get clock\n");
		return -ENOENT;
	}

	dev_dbg(&pdev->dev, "clock source %p\n", i2c->clk);


	/*                   */

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	i2c->regs = devm_ioremap_resource(&pdev->dev, res);

	if (IS_ERR(i2c->regs))
		return PTR_ERR(i2c->regs);

	dev_dbg(&pdev->dev, "registers %p (%p)\n",
		i2c->regs, res);

	/*                                   */

	i2c->adap.algo_data = i2c;
	i2c->adap.dev.parent = &pdev->dev;

	i2c->pctrl = devm_pinctrl_get_select_default(i2c->dev);

	/*                                */

	if (i2c->pdata->cfg_gpio) {
		i2c->pdata->cfg_gpio(to_platform_device(i2c->dev));
	} else if (IS_ERR(i2c->pctrl) && s3c24xx_i2c_parse_dt_gpio(i2c)) {
		return -EINVAL;
	}

	/*                               */

	clk_prepare_enable(i2c->clk);
	ret = s3c24xx_i2c_init(i2c);
	clk_disable_unprepare(i2c->clk);
	if (ret != 0) {
		dev_err(&pdev->dev, "I2C controller init failed\n");
		return ret;
	}
	/*                                                                  
                                  
  */

	i2c->irq = ret = platform_get_irq(pdev, 0);
	if (ret <= 0) {
		dev_err(&pdev->dev, "cannot find IRQ\n");
		return ret;
	}

	ret = devm_request_irq(&pdev->dev, i2c->irq, s3c24xx_i2c_irq, 0,
			       dev_name(&pdev->dev), i2c);

	if (ret != 0) {
		dev_err(&pdev->dev, "cannot claim IRQ %d\n", i2c->irq);
		return ret;
	}

	ret = s3c24xx_i2c_register_cpufreq(i2c);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to register cpufreq notifier\n");
		return ret;
	}

	/*                                                             
                                                                
                                                                
                
  */

	i2c->adap.nr = i2c->pdata->bus_num;
	i2c->adap.dev.of_node = pdev->dev.of_node;

	ret = i2c_add_numbered_adapter(&i2c->adap);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to add bus to i2c core\n");
		s3c24xx_i2c_deregister_cpufreq(i2c);
		return ret;
	}

	of_i2c_register_devices(&i2c->adap);
	platform_set_drvdata(pdev, i2c);

	pm_runtime_enable(&pdev->dev);
	pm_runtime_enable(&i2c->adap.dev);

	dev_info(&pdev->dev, "%s: S3C I2C adapter\n", dev_name(&i2c->adap.dev));
	return 0;
}

/*                   
  
                                             
*/

static int s3c24xx_i2c_remove(struct platform_device *pdev)
{
	struct s3c24xx_i2c *i2c = platform_get_drvdata(pdev);

	pm_runtime_disable(&i2c->adap.dev);
	pm_runtime_disable(&pdev->dev);

	s3c24xx_i2c_deregister_cpufreq(i2c);

	i2c_del_adapter(&i2c->adap);

	clk_disable_unprepare(i2c->clk);

	if (pdev->dev.of_node && IS_ERR(i2c->pctrl))
		s3c24xx_i2c_dt_gpio_free(i2c);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int s3c24xx_i2c_suspend_noirq(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct s3c24xx_i2c *i2c = platform_get_drvdata(pdev);

	i2c->suspended = 1;

	return 0;
}

static int s3c24xx_i2c_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct s3c24xx_i2c *i2c = platform_get_drvdata(pdev);

	clk_prepare_enable(i2c->clk);
	s3c24xx_i2c_init(i2c);
	clk_disable_unprepare(i2c->clk);
	i2c->suspended = 0;

	return 0;
}
#endif

#ifdef CONFIG_PM
static const struct dev_pm_ops s3c24xx_i2c_dev_pm_ops = {
#ifdef CONFIG_PM_SLEEP
	.suspend_noirq = s3c24xx_i2c_suspend_noirq,
	.resume = s3c24xx_i2c_resume,
#endif
};

#define S3C24XX_DEV_PM_OPS (&s3c24xx_i2c_dev_pm_ops)
#else
#define S3C24XX_DEV_PM_OPS NULL
#endif

/*                                     */

static struct platform_driver s3c24xx_i2c_driver = {
	.probe		= s3c24xx_i2c_probe,
	.remove		= s3c24xx_i2c_remove,
	.id_table	= s3c24xx_driver_ids,
	.driver		= {
		.owner	= THIS_MODULE,
		.name	= "s3c-i2c",
		.pm	= S3C24XX_DEV_PM_OPS,
		.of_match_table = of_match_ptr(s3c24xx_i2c_match),
	},
};

static int __init i2c_adap_s3c_init(void)
{
	return platform_driver_register(&s3c24xx_i2c_driver);
}
subsys_initcall(i2c_adap_s3c_init);

static void __exit i2c_adap_s3c_exit(void)
{
	platform_driver_unregister(&s3c24xx_i2c_driver);
}
module_exit(i2c_adap_s3c_exit);

MODULE_DESCRIPTION("S3C24XX I2C Bus driver");
MODULE_AUTHOR("Ben Dooks, <ben@simtec.co.uk>");
MODULE_LICENSE("GPL");
