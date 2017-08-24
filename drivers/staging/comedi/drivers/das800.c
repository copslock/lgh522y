/*
    comedi/drivers/das800.c
    Driver for Keitley das800 series boards and compatibles
    Copyright (C) 2000 Frank Mori Hess <fmhess@users.sourceforge.net>

    COMEDI - Linux Control and Measurement Device Interface
    Copyright (C) 2000 David A. Schleef <ds@schleef.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

************************************************************************
*/
/*
              
                                                      
                                                      
                                                                   
                    
                                                  
                                                   
                               
                                                                      

                      
                             
                                                                              

      
                                                                         

                                                                   
                                                                   
                       

                                                             
                               

                                                                    
                                                           
*/
/*

                       
                                    
                            
                           
                                      
                                       


*/

#include <linux/interrupt.h>
#include "../comedidev.h"

#include <linux/ioport.h>
#include <linux/delay.h>

#include "8253.h"
#include "comedi_fc.h"

#define DAS800_SIZE           8
#define TIMER_BASE            1000
#define N_CHAN_AI             8	/*                                  */

/*                          */

#define DAS800_LSB            0
#define   FIFO_EMPTY            0x1
#define   FIFO_OVF              0x2
#define DAS800_MSB            1
#define DAS800_CONTROL1       2
#define   CONTROL1_INTE         0x8
#define DAS800_CONV_CONTROL   2
#define   ITE                   0x1
#define   CASC                  0x2
#define   DTEN                  0x4
#define   IEOC                  0x8
#define   EACS                  0x10
#define   CONV_HCEN             0x80
#define DAS800_SCAN_LIMITS    2
#define DAS800_STATUS         2
#define   IRQ                   0x8
#define   BUSY                  0x80
#define DAS800_GAIN           3
#define   CIO_FFOV              0x8   /*                             */
#define   CIO_ENHF              0x90  /*                                      */
#define   CONTROL1              0x80
#define   CONV_CONTROL          0xa0
#define   SCAN_LIMITS           0xc0
#define   ID                    0xe0
#define DAS800_8254           4
#define DAS800_STATUS2        7
#define   STATUS2_HCEN          0x80
#define   STATUS2_INTE          0X20
#define DAS800_ID             7

#define DAS802_16_HALF_FIFO_SZ	128

struct das800_board {
	const char *name;
	int ai_speed;
	const struct comedi_lrange *ai_range;
	int resolution;
};

static const struct comedi_lrange range_das801_ai = {
	9, {
		BIP_RANGE(5),
		BIP_RANGE(10),
		UNI_RANGE(10),
		BIP_RANGE(0.5),
		UNI_RANGE(1),
		BIP_RANGE(0.05),
		UNI_RANGE(0.1),
		BIP_RANGE(0.01),
		UNI_RANGE(0.02)
	}
};

static const struct comedi_lrange range_cio_das801_ai = {
	9, {
		BIP_RANGE(5),
		BIP_RANGE(10),
		UNI_RANGE(10),
		BIP_RANGE(0.5),
		UNI_RANGE(1),
		BIP_RANGE(0.05),
		UNI_RANGE(0.1),
		BIP_RANGE(0.005),
		UNI_RANGE(0.01)
	}
};

static const struct comedi_lrange range_das802_ai = {
	9, {
		BIP_RANGE(5),
		BIP_RANGE(10),
		UNI_RANGE(10),
		BIP_RANGE(2.5),
		UNI_RANGE(5),
		BIP_RANGE(1.25),
		UNI_RANGE(2.5),
		BIP_RANGE(0.625),
		UNI_RANGE(1.25)
	}
};

static const struct comedi_lrange range_das80216_ai = {
	8, {
		BIP_RANGE(10),
		UNI_RANGE(10),
		BIP_RANGE(5),
		UNI_RANGE(5),
		BIP_RANGE(2.5),
		UNI_RANGE(2.5),
		BIP_RANGE(1.25),
		UNI_RANGE(1.25)
	}
};

enum das800_boardinfo {
	BOARD_DAS800,
	BOARD_CIODAS800,
	BOARD_DAS801,
	BOARD_CIODAS801,
	BOARD_DAS802,
	BOARD_CIODAS802,
	BOARD_CIODAS80216,
};

static const struct das800_board das800_boards[] = {
	[BOARD_DAS800] = {
		.name		= "das-800",
		.ai_speed	= 25000,
		.ai_range	= &range_bipolar5,
		.resolution	= 12,
	},
	[BOARD_CIODAS800] = {
		.name		= "cio-das800",
		.ai_speed	= 20000,
		.ai_range	= &range_bipolar5,
		.resolution	= 12,
	},
	[BOARD_DAS801] = {
		.name		= "das-801",
		.ai_speed	= 25000,
		.ai_range	= &range_das801_ai,
		.resolution	= 12,
	},
	[BOARD_CIODAS801] = {
		.name		= "cio-das801",
		.ai_speed	= 20000,
		.ai_range	= &range_cio_das801_ai,
		.resolution	= 12,
	},
	[BOARD_DAS802] = {
		.name		= "das-802",
		.ai_speed	= 25000,
		.ai_range	= &range_das802_ai,
		.resolution	= 12,
	},
	[BOARD_CIODAS802] = {
		.name		= "cio-das802",
		.ai_speed	= 20000,
		.ai_range	= &range_das802_ai,
		.resolution	= 12,
	},
	[BOARD_CIODAS80216] = {
		.name		= "cio-das802/16",
		.ai_speed	= 10000,
		.ai_range	= &range_das80216_ai,
		.resolution	= 16,
	},
};

struct das800_private {
	unsigned int count;	/*                                        */
	unsigned int divisor1;	/*                                       */
	unsigned int divisor2;	/*                                       */
	unsigned int do_bits;	/*                     */
	bool forever;		/*                                       */
};

static void das800_ind_write(struct comedi_device *dev,
			     unsigned val, unsigned reg)
{
	/*
                                                 
                                
  */
	outb(reg, dev->iobase + DAS800_GAIN);
	outb(val, dev->iobase + 2);
}

static unsigned das800_ind_read(struct comedi_device *dev, unsigned reg)
{
	/*
                                                 
                                 
  */
	outb(reg, dev->iobase + DAS800_GAIN);
	return inb(dev->iobase + 7);
}

static void das800_enable(struct comedi_device *dev)
{
	const struct das800_board *thisboard = comedi_board(dev);
	struct das800_private *devpriv = dev->private;
	unsigned long irq_flags;

	spin_lock_irqsave(&dev->spinlock, irq_flags);
	/*                                                     */
	if (thisboard->resolution == 16)
		outb(CIO_ENHF, dev->iobase + DAS800_GAIN);
	/*                            */
	das800_ind_write(dev, CONV_HCEN, CONV_CONTROL);
	/*                         */
	das800_ind_write(dev, CONTROL1_INTE | devpriv->do_bits, CONTROL1);
	spin_unlock_irqrestore(&dev->spinlock, irq_flags);
}

static void das800_disable(struct comedi_device *dev)
{
	unsigned long irq_flags;

	spin_lock_irqsave(&dev->spinlock, irq_flags);
	/*                                            */
	das800_ind_write(dev, 0x0, CONV_CONTROL);
	spin_unlock_irqrestore(&dev->spinlock, irq_flags);
}

static int das800_set_frequency(struct comedi_device *dev)
{
	struct das800_private *devpriv = dev->private;
	int err = 0;

	if (i8254_load(dev->iobase + DAS800_8254, 0, 1, devpriv->divisor1, 2))
		err++;
	if (i8254_load(dev->iobase + DAS800_8254, 0, 2, devpriv->divisor2, 2))
		err++;
	if (err)
		return -1;

	return 0;
}

static int das800_cancel(struct comedi_device *dev, struct comedi_subdevice *s)
{
	struct das800_private *devpriv = dev->private;

	devpriv->forever = false;
	devpriv->count = 0;
	das800_disable(dev);
	return 0;
}

static int das800_ai_do_cmdtest(struct comedi_device *dev,
				struct comedi_subdevice *s,
				struct comedi_cmd *cmd)
{
	const struct das800_board *thisboard = comedi_board(dev);
	struct das800_private *devpriv = dev->private;
	int err = 0;

	/*                                                */

	err |= cfc_check_trigger_src(&cmd->start_src, TRIG_NOW | TRIG_EXT);
	err |= cfc_check_trigger_src(&cmd->scan_begin_src, TRIG_FOLLOW);
	err |= cfc_check_trigger_src(&cmd->convert_src, TRIG_TIMER | TRIG_EXT);
	err |= cfc_check_trigger_src(&cmd->scan_end_src, TRIG_COUNT);
	err |= cfc_check_trigger_src(&cmd->stop_src, TRIG_COUNT | TRIG_NONE);

	if (err)
		return 1;

	/*                                                */

	err |= cfc_check_trigger_is_unique(cmd->start_src);
	err |= cfc_check_trigger_is_unique(cmd->convert_src);
	err |= cfc_check_trigger_is_unique(cmd->stop_src);

	/*                                   */

	if (err)
		return 2;

	/*                                                */

	err |= cfc_check_trigger_arg_is(&cmd->start_arg, 0);

	if (cmd->convert_src == TRIG_TIMER)
		err |= cfc_check_trigger_arg_min(&cmd->convert_arg,
						 thisboard->ai_speed);

	err |= cfc_check_trigger_arg_min(&cmd->chanlist_len, 1);
	err |= cfc_check_trigger_arg_is(&cmd->scan_end_arg, cmd->chanlist_len);

	if (cmd->stop_src == TRIG_COUNT)
		err |= cfc_check_trigger_arg_min(&cmd->stop_arg, 1);
	else	/*           */
		err |= cfc_check_trigger_arg_is(&cmd->stop_arg, 0);

	if (err)
		return 3;

	/*                              */

	if (cmd->convert_src == TRIG_TIMER) {
		int tmp = cmd->convert_arg;

		/*                                                   */
		i8253_cascade_ns_to_timer_2div(TIMER_BASE,
					       &devpriv->divisor1,
					       &devpriv->divisor2,
					       &cmd->convert_arg,
					       cmd->flags & TRIG_ROUND_MASK);
		if (tmp != cmd->convert_arg)
			err++;
	}

	if (err)
		return 4;

	/*                                                     */
	if (cmd->chanlist) {
		unsigned int chan = CR_CHAN(cmd->chanlist[0]);
		unsigned int range = CR_RANGE(cmd->chanlist[0]);
		unsigned int next;
		int i;

		for (i = 1; i < cmd->chanlist_len; i++) {
			next = cmd->chanlist[i];
			if (CR_CHAN(next) != (chan + i) % N_CHAN_AI) {
				dev_err(dev->class_dev,
					"chanlist must be consecutive, counting upwards\n");
				err++;
			}
			if (CR_RANGE(next) != range) {
				dev_err(dev->class_dev,
					"chanlist must all have the same gain\n");
				err++;
			}
		}
	}

	if (err)
		return 5;

	return 0;
}

static int das800_ai_do_cmd(struct comedi_device *dev,
			    struct comedi_subdevice *s)
{
	const struct das800_board *thisboard = comedi_board(dev);
	struct das800_private *devpriv = dev->private;
	struct comedi_async *async = s->async;
	unsigned int gain = CR_RANGE(async->cmd.chanlist[0]);
	unsigned int start_chan = CR_CHAN(async->cmd.chanlist[0]);
	unsigned int end_chan = (start_chan + async->cmd.chanlist_len - 1) % 8;
	unsigned int scan_chans = (end_chan << 3) | start_chan;
	int conv_bits;
	unsigned long irq_flags;

	das800_disable(dev);

	spin_lock_irqsave(&dev->spinlock, irq_flags);
	/*                 */
	das800_ind_write(dev, scan_chans, SCAN_LIMITS);
	spin_unlock_irqrestore(&dev->spinlock, irq_flags);

	/*          */
	if (thisboard->resolution == 12 && gain > 0)
		gain += 0x7;
	gain &= 0xf;
	outb(gain, dev->iobase + DAS800_GAIN);

	switch (async->cmd.stop_src) {
	case TRIG_COUNT:
		devpriv->count = async->cmd.stop_arg * async->cmd.chanlist_len;
		devpriv->forever = false;
		break;
	case TRIG_NONE:
		devpriv->forever = true;
		devpriv->count = 0;
		break;
	default:
		break;
	}

	/*                                                               
                                                
  */
	conv_bits = 0;
	conv_bits |= EACS | IEOC;
	if (async->cmd.start_src == TRIG_EXT)
		conv_bits |= DTEN;
	switch (async->cmd.convert_src) {
	case TRIG_TIMER:
		conv_bits |= CASC | ITE;
		/*                          */
		if (das800_set_frequency(dev) < 0) {
			comedi_error(dev, "Error setting up counters");
			return -1;
		}
		break;
	case TRIG_EXT:
		break;
	default:
		break;
	}

	spin_lock_irqsave(&dev->spinlock, irq_flags);
	das800_ind_write(dev, conv_bits, CONV_CONTROL);
	spin_unlock_irqrestore(&dev->spinlock, irq_flags);

	async->events = 0;
	das800_enable(dev);
	return 0;
}

static unsigned int das800_ai_get_sample(struct comedi_device *dev)
{
	unsigned int lsb = inb(dev->iobase + DAS800_LSB);
	unsigned int msb = inb(dev->iobase + DAS800_MSB);

	return (msb << 8) | lsb;
}

static irqreturn_t das800_interrupt(int irq, void *d)
{
	struct comedi_device *dev = d;
	struct das800_private *devpriv = dev->private;
	struct comedi_subdevice *s = dev->read_subdev;
	struct comedi_async *async = s ? s->async : NULL;
	unsigned long irq_flags;
	unsigned int status;
	unsigned int val;
	bool fifo_empty;
	bool fifo_overflow;
	int i;

	status = inb(dev->iobase + DAS800_STATUS);
	if (!(status & IRQ))
		return IRQ_NONE;
	if (!dev->attached)
		return IRQ_HANDLED;

	spin_lock_irqsave(&dev->spinlock, irq_flags);
	status = das800_ind_read(dev, CONTROL1) & STATUS2_HCEN;
	/*
                                                         
                                              
  */

	/*                                                    */
	if (status == 0) {
		spin_unlock_irqrestore(&dev->spinlock, irq_flags);
		return IRQ_HANDLED;
	}

	for (i = 0; i < DAS802_16_HALF_FIFO_SZ; i++) {
		val = das800_ai_get_sample(dev);
		if (s->maxdata == 0x0fff) {
			fifo_empty = !!(val & FIFO_EMPTY);
			fifo_overflow = !!(val & FIFO_OVF);
		} else {
			/*                                            */
			fifo_empty = false;
			fifo_overflow = !!(inb(dev->iobase + DAS800_GAIN) &
						CIO_FFOV);
		}
		if (fifo_empty || fifo_overflow)
			break;

		if (s->maxdata == 0x0fff)
			val >>= 4;	/*               */

		/*                                          */
		if (devpriv->count > 0 || devpriv->forever) {
			/*                            */
			cfc_write_to_buffer(s, val & s->maxdata);
			devpriv->count--;
		}
	}
	async->events |= COMEDI_CB_BLOCK;

	if (fifo_overflow) {
		spin_unlock_irqrestore(&dev->spinlock, irq_flags);
		das800_cancel(dev, s);
		async->events |= COMEDI_CB_ERROR | COMEDI_CB_EOA;
		comedi_event(dev, s);
		async->events = 0;
		return IRQ_HANDLED;
	}

	if (devpriv->count > 0 || devpriv->forever) {
		/*                            
                                                              */
		das800_ind_write(dev, CONTROL1_INTE | devpriv->do_bits,
				 CONTROL1);
		spin_unlock_irqrestore(&dev->spinlock, irq_flags);
	} else {
		/*                             */
		spin_unlock_irqrestore(&dev->spinlock, irq_flags);
		das800_disable(dev);
		async->events |= COMEDI_CB_EOA;
	}
	comedi_event(dev, s);
	async->events = 0;
	return IRQ_HANDLED;
}

static int das800_wait_for_conv(struct comedi_device *dev, int timeout)
{
	int i;

	for (i = 0; i < timeout; i++) {
		if (!(inb(dev->iobase + DAS800_STATUS) & BUSY))
			return 0;
	}
	return -ETIME;
}

static int das800_ai_insn_read(struct comedi_device *dev,
			       struct comedi_subdevice *s,
			       struct comedi_insn *insn,
			       unsigned int *data)
{
	struct das800_private *devpriv = dev->private;
	unsigned int chan = CR_CHAN(insn->chanspec);
	unsigned int range = CR_RANGE(insn->chanspec);
	unsigned long irq_flags;
	unsigned int val;
	int ret;
	int i;

	das800_disable(dev);

	/*                 */
	spin_lock_irqsave(&dev->spinlock, irq_flags);
	das800_ind_write(dev, chan | devpriv->do_bits, CONTROL1);
	spin_unlock_irqrestore(&dev->spinlock, irq_flags);

	/*                  */
	if (s->maxdata == 0x0fff && range)
		range += 0x7;
	range &= 0xf;
	outb(range, dev->iobase + DAS800_GAIN);

	udelay(5);

	for (i = 0; i < insn->n; i++) {
		/*                    */
		outb_p(0, dev->iobase + DAS800_MSB);

		ret = das800_wait_for_conv(dev, 1000);
		if (ret)
			return ret;

		val = das800_ai_get_sample(dev);
		if (s->maxdata == 0x0fff)
			val >>= 4;	/*               */
		data[i] = val & s->maxdata;
	}

	return insn->n;
}

static int das800_di_insn_bits(struct comedi_device *dev,
			       struct comedi_subdevice *s,
			       struct comedi_insn *insn,
			       unsigned int *data)
{
	data[1] = (inb(dev->iobase + DAS800_STATUS) >> 4) & 0x7;

	return insn->n;
}

static int das800_do_insn_bits(struct comedi_device *dev,
			       struct comedi_subdevice *s,
			       struct comedi_insn *insn,
			       unsigned int *data)
{
	struct das800_private *devpriv = dev->private;
	unsigned int mask = data[0];
	unsigned int bits = data[1];
	unsigned long irq_flags;

	if (mask) {
		s->state &= ~mask;
		s->state |= (bits & mask);
		devpriv->do_bits = s->state << 4;

		spin_lock_irqsave(&dev->spinlock, irq_flags);
		das800_ind_write(dev, CONTROL1_INTE | devpriv->do_bits,
				 CONTROL1);
		spin_unlock_irqrestore(&dev->spinlock, irq_flags);
	}

	data[1] = s->state;

	return insn->n;
}

static int das800_probe(struct comedi_device *dev)
{
	const struct das800_board *thisboard = comedi_board(dev);
	int board = thisboard ? thisboard - das800_boards : -EINVAL;
	int id_bits;
	unsigned long irq_flags;

	spin_lock_irqsave(&dev->spinlock, irq_flags);
	id_bits = das800_ind_read(dev, ID) & 0x3;
	spin_unlock_irqrestore(&dev->spinlock, irq_flags);

	switch (id_bits) {
	case 0x0:
		if (board == BOARD_DAS800 || board == BOARD_CIODAS800)
			break;
		dev_dbg(dev->class_dev, "Board model (probed): DAS-800\n");
		board = BOARD_DAS800;
		break;
	case 0x2:
		if (board == BOARD_DAS801 || board == BOARD_CIODAS801)
			break;
		dev_dbg(dev->class_dev, "Board model (probed): DAS-801\n");
		board = BOARD_DAS801;
		break;
	case 0x3:
		if (board == BOARD_DAS802 || board == BOARD_CIODAS802 ||
		    board == BOARD_CIODAS80216)
			break;
		dev_dbg(dev->class_dev, "Board model (probed): DAS-802\n");
		board = BOARD_DAS802;
		break;
	default:
		dev_dbg(dev->class_dev, "Board model: 0x%x (unknown)\n",
			id_bits);
		board = -EINVAL;
		break;
	}
	return board;
}

static int das800_attach(struct comedi_device *dev, struct comedi_devconfig *it)
{
	const struct das800_board *thisboard = comedi_board(dev);
	struct das800_private *devpriv;
	struct comedi_subdevice *s;
	unsigned int irq = it->options[1];
	unsigned long irq_flags;
	int board;
	int ret;

	devpriv = kzalloc(sizeof(*devpriv), GFP_KERNEL);
	if (!devpriv)
		return -ENOMEM;
	dev->private = devpriv;

	ret = comedi_request_region(dev, it->options[0], DAS800_SIZE);
	if (ret)
		return ret;

	board = das800_probe(dev);
	if (board < 0) {
		dev_dbg(dev->class_dev, "unable to determine board type\n");
		return -ENODEV;
	}
	dev->board_ptr = das800_boards + board;
	thisboard = comedi_board(dev);
	dev->board_name = thisboard->name;

	if (irq > 1 && irq <= 7) {
		ret = request_irq(irq, das800_interrupt, 0, dev->board_name,
				  dev);
		if (ret == 0)
			dev->irq = irq;
	}

	ret = comedi_alloc_subdevices(dev, 3);
	if (ret)
		return ret;

	/*                        */
	s = &dev->subdevices[0];
	dev->read_subdev = s;
	s->type		= COMEDI_SUBD_AI;
	s->subdev_flags	= SDF_READABLE | SDF_GROUND;
	s->n_chan	= 8;
	s->maxdata	= (1 << thisboard->resolution) - 1;
	s->range_table	= thisboard->ai_range;
	s->insn_read	= das800_ai_insn_read;
	if (dev->irq) {
		s->subdev_flags	|= SDF_CMD_READ;
		s->len_chanlist	= 8;
		s->do_cmdtest	= das800_ai_do_cmdtest;
		s->do_cmd	= das800_ai_do_cmd;
		s->cancel	= das800_cancel;
	}

	/*                         */
	s = &dev->subdevices[1];
	s->type		= COMEDI_SUBD_DI;
	s->subdev_flags	= SDF_READABLE;
	s->n_chan	= 3;
	s->maxdata	= 1;
	s->range_table	= &range_digital;
	s->insn_bits	= das800_di_insn_bits;

	/*                          */
	s = &dev->subdevices[2];
	s->type		= COMEDI_SUBD_DO;
	s->subdev_flags	= SDF_WRITABLE | SDF_READABLE;
	s->n_chan	= 4;
	s->maxdata	= 1;
	s->range_table	= &range_digital;
	s->insn_bits	= das800_do_insn_bits;

	das800_disable(dev);

	/*                                 */
	spin_lock_irqsave(&dev->spinlock, irq_flags);
	das800_ind_write(dev, CONTROL1_INTE | devpriv->do_bits, CONTROL1);
	spin_unlock_irqrestore(&dev->spinlock, irq_flags);

	return 0;
};

static struct comedi_driver driver_das800 = {
	.driver_name	= "das800",
	.module		= THIS_MODULE,
	.attach		= das800_attach,
	.detach		= comedi_legacy_detach,
	.num_names	= ARRAY_SIZE(das800_boards),
	.board_name	= &das800_boards[0].name,
	.offset		= sizeof(struct das800_board),
};
module_comedi_driver(driver_das800);

MODULE_AUTHOR("Comedi http://www.comedi.org");
MODULE_DESCRIPTION("Comedi low-level driver");
MODULE_LICENSE("GPL");
