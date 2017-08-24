/*
    comedi/drivers/dmm32at.c
    Diamond Systems mm32at code for a Comedi driver

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

*/
/*
               
                                           
        
                                                   
                                     
                    

                                                     
                                                                     
                                                                     
                                                      

                      
                                                 
*/

#include <linux/interrupt.h>
#include "../comedidev.h"
#include <linux/ioport.h>

#include "comedi_fc.h"

/*                          */

#define DMM32AT_MEMSIZE 0x10

#define DMM32AT_CONV 0x00
#define DMM32AT_AILSB 0x00
#define DMM32AT_AUXDOUT 0x01
#define DMM32AT_AIMSB 0x01
#define DMM32AT_AILOW 0x02
#define DMM32AT_AIHIGH 0x03

#define DMM32AT_DACLSB 0x04
#define DMM32AT_DACSTAT 0x04
#define DMM32AT_DACMSB 0x05

#define DMM32AT_FIFOCNTRL 0x07
#define DMM32AT_FIFOSTAT 0x07

#define DMM32AT_CNTRL 0x08
#define DMM32AT_AISTAT 0x08

#define DMM32AT_INTCLOCK 0x09

#define DMM32AT_CNTRDIO 0x0a

#define DMM32AT_AICONF 0x0b
#define DMM32AT_AIRBACK 0x0b

#define DMM32AT_CLK1 0x0d
#define DMM32AT_CLK2 0x0e
#define DMM32AT_CLKCT 0x0f

#define DMM32AT_DIOA 0x0c
#define DMM32AT_DIOB 0x0d
#define DMM32AT_DIOC 0x0e
#define DMM32AT_DIOCONF 0x0f

/*                        */

/*                      */
#define DMM32AT_DACBUSY 0x80

/*                        */
#define DMM32AT_FIFORESET 0x02
#define DMM32AT_SCANENABLE 0x04

/*                    */
#define DMM32AT_RESET 0x20
#define DMM32AT_INTRESET 0x08
#define DMM32AT_CLKACC 0x00
#define DMM32AT_DIOACC 0x01

/*                     */
#define DMM32AT_STATUS 0x80

/*                       */
#define DMM32AT_ADINT 0x80
#define DMM32AT_CLKSEL 0x03

/*                      */
#define DMM32AT_FREQ12 0x80

/*                     */
#define DMM32AT_RANGE_U10 0x0c
#define DMM32AT_RANGE_U5 0x0d
#define DMM32AT_RANGE_B10 0x08
#define DMM32AT_RANGE_B5 0x00
#define DMM32AT_SCINT_20 0x00
#define DMM32AT_SCINT_15 0x10
#define DMM32AT_SCINT_10 0x20
#define DMM32AT_SCINT_5 0x30

/*                    */
#define DMM32AT_CLKCT1 0x56	/*                                       */
#define DMM32AT_CLKCT2 0xb6	/*                                            */

/*                      */
#define DMM32AT_DIENABLE 0x80
#define DMM32AT_DIRA 0x10
#define DMM32AT_DIRB 0x02
#define DMM32AT_DIRCL 0x01
#define DMM32AT_DIRCH 0x08

/*                                     */
static const struct comedi_lrange dmm32at_airanges = {
	4,
	{
	 UNI_RANGE(10),
	 UNI_RANGE(5),
	 BIP_RANGE(10),
	 BIP_RANGE(5),
	 }
};

/*                                  */
static const unsigned char dmm32at_rangebits[] = {
	DMM32AT_RANGE_U10,
	DMM32AT_RANGE_U5,
	DMM32AT_RANGE_B10,
	DMM32AT_RANGE_B5,
};

/*                                                             
                                                                     
 */
static const struct comedi_lrange dmm32at_aoranges = {
	4,
	{
	 UNI_RANGE(10),
	 UNI_RANGE(5),
	 BIP_RANGE(10),
	 BIP_RANGE(5),
	 }
};

struct dmm32at_private {

	int data;
	int ai_inuse;
	unsigned int ai_scans_left;

	/*                      */
	unsigned int ao_readback[4];
	unsigned char dio_config;

};

static int dmm32at_ai_rinsn(struct comedi_device *dev,
			    struct comedi_subdevice *s,
			    struct comedi_insn *insn, unsigned int *data)
{
	int n, i;
	unsigned int d;
	unsigned char status;
	unsigned short msb, lsb;
	unsigned char chan;
	int range;

	/*                                  */

	chan = CR_CHAN(insn->chanspec) & (s->n_chan - 1);
	range = CR_RANGE(insn->chanspec);

	/*                                                  */

	/*                                           */
	outb(DMM32AT_FIFORESET, dev->iobase + DMM32AT_FIFOCNTRL);

	/*                                 */
	outb(chan, dev->iobase + DMM32AT_AILOW);
	outb(chan, dev->iobase + DMM32AT_AIHIGH);
	/*                    */
	outb(dmm32at_rangebits[range], dev->iobase + DMM32AT_AICONF);

	/*                            */
	for (i = 0; i < 40000; i++) {
		status = inb(dev->iobase + DMM32AT_AIRBACK);
		if ((status & DMM32AT_STATUS) == 0)
			break;
	}
	if (i == 40000) {
		printk(KERN_WARNING "dmm32at: timeout\n");
		return -ETIMEDOUT;
	}

	/*                   */
	for (n = 0; n < insn->n; n++) {
		/*                    */
		outb(0xff, dev->iobase + DMM32AT_CONV);
		/*                            */
		for (i = 0; i < 40000; i++) {
			status = inb(dev->iobase + DMM32AT_AISTAT);
			if ((status & DMM32AT_STATUS) == 0)
				break;
		}
		if (i == 40000) {
			printk(KERN_WARNING "dmm32at: timeout\n");
			return -ETIMEDOUT;
		}

		/*           */
		lsb = inb(dev->iobase + DMM32AT_AILSB);
		msb = inb(dev->iobase + DMM32AT_AIMSB);

		/*                                                   
                                                 
                                                  
                                                   
                                                    
                    */
		d = ((msb ^ 0x0080) << 8) + lsb;

		data[n] = d;
	}

	/*                                           */
	return n;
}

static int dmm32at_ns_to_timer(unsigned int *ns, int round)
{
	/*               */
	return *ns;
}

static int dmm32at_ai_cmdtest(struct comedi_device *dev,
			      struct comedi_subdevice *s,
			      struct comedi_cmd *cmd)
{
	int err = 0;
	int tmp;
	int start_chan, gain, i;

	/*                                                */

	err |= cfc_check_trigger_src(&cmd->start_src, TRIG_NOW);
	err |= cfc_check_trigger_src(&cmd->scan_begin_src,
					TRIG_TIMER /*           */);
	err |= cfc_check_trigger_src(&cmd->convert_src,
					TRIG_TIMER /*           */);
	err |= cfc_check_trigger_src(&cmd->scan_end_src, TRIG_COUNT);
	err |= cfc_check_trigger_src(&cmd->stop_src, TRIG_COUNT | TRIG_NONE);

	if (err)
		return 1;

	/*                                                */

	err |= cfc_check_trigger_is_unique(cmd->scan_begin_src);
	err |= cfc_check_trigger_is_unique(cmd->convert_src);
	err |= cfc_check_trigger_is_unique(cmd->stop_src);

	/*                                   */

	if (err)
		return 2;

	/*                                                */

	err |= cfc_check_trigger_arg_is(&cmd->start_arg, 0);

#define MAX_SCAN_SPEED	1000000	/*                */
#define MIN_SCAN_SPEED	1000000000	/*                */

	if (cmd->scan_begin_src == TRIG_TIMER) {
		err |= cfc_check_trigger_arg_min(&cmd->scan_begin_arg,
						 MAX_SCAN_SPEED);
		err |= cfc_check_trigger_arg_max(&cmd->scan_begin_arg,
						 MIN_SCAN_SPEED);
	} else {
		/*                  */
		/*                                                */
		/*                                           */
		err |= cfc_check_trigger_arg_max(&cmd->scan_begin_arg, 9);
	}

	if (cmd->convert_src == TRIG_TIMER) {
		if (cmd->convert_arg >= 17500)
			cmd->convert_arg = 20000;
		else if (cmd->convert_arg >= 12500)
			cmd->convert_arg = 15000;
		else if (cmd->convert_arg >= 7500)
			cmd->convert_arg = 10000;
		else
			cmd->convert_arg = 5000;
	} else {
		/*                  */
		/*           */
		err |= cfc_check_trigger_arg_max(&cmd->convert_arg, 9);
	}

	err |= cfc_check_trigger_arg_is(&cmd->scan_end_arg, cmd->chanlist_len);

	if (cmd->stop_src == TRIG_COUNT) {
		err |= cfc_check_trigger_arg_max(&cmd->stop_arg, 0xfffffff0);
		err |= cfc_check_trigger_arg_min(&cmd->stop_arg, 1);
	} else {
		/*           */
		err |= cfc_check_trigger_arg_is(&cmd->stop_arg, 0);
	}

	if (err)
		return 3;

	/*                              */

	if (cmd->scan_begin_src == TRIG_TIMER) {
		tmp = cmd->scan_begin_arg;
		dmm32at_ns_to_timer(&cmd->scan_begin_arg,
				    cmd->flags & TRIG_ROUND_MASK);
		if (tmp != cmd->scan_begin_arg)
			err++;
	}
	if (cmd->convert_src == TRIG_TIMER) {
		tmp = cmd->convert_arg;
		dmm32at_ns_to_timer(&cmd->convert_arg,
				    cmd->flags & TRIG_ROUND_MASK);
		if (tmp != cmd->convert_arg)
			err++;
		if (cmd->scan_begin_src == TRIG_TIMER &&
		    cmd->scan_begin_arg <
		    cmd->convert_arg * cmd->scan_end_arg) {
			cmd->scan_begin_arg =
			    cmd->convert_arg * cmd->scan_end_arg;
			err++;
		}
	}

	if (err)
		return 4;

	/*                                                         
                                                         */

	if (cmd->chanlist) {
		gain = CR_RANGE(cmd->chanlist[0]);
		start_chan = CR_CHAN(cmd->chanlist[0]);
		for (i = 1; i < cmd->chanlist_len; i++) {
			if (CR_CHAN(cmd->chanlist[i]) !=
			    (start_chan + i) % s->n_chan) {
				comedi_error(dev,
					     "entries in chanlist must be consecutive channels, counting upwards\n");
				err++;
			}
			if (CR_RANGE(cmd->chanlist[i]) != gain) {
				comedi_error(dev,
					     "entries in chanlist must all have the same gain\n");
				err++;
			}
		}
	}

	if (err)
		return 5;

	return 0;
}

static void dmm32at_setaitimer(struct comedi_device *dev, unsigned int nansec)
{
	unsigned char lo1, lo2, hi2;
	unsigned short both2;

	/*                      */
	lo1 = 200;
	both2 = nansec / 20000;
	hi2 = (both2 & 0xff00) >> 8;
	lo2 = both2 & 0x00ff;

	/*                                    */
	outb(0, dev->iobase + DMM32AT_CNTRDIO);

	/*                              */
	outb(DMM32AT_CLKACC, dev->iobase + DMM32AT_CNTRL);

	/*                                                          */
	outb(DMM32AT_CLKCT1, dev->iobase + DMM32AT_CLKCT);
	outb(lo1, dev->iobase + DMM32AT_CLK1);

	/*                                                               */
	outb(DMM32AT_CLKCT2, dev->iobase + DMM32AT_CLKCT);
	outb(lo2, dev->iobase + DMM32AT_CLK2);
	outb(hi2, dev->iobase + DMM32AT_CLK2);

	/*                                                                 */
	outb(DMM32AT_ADINT | DMM32AT_CLKSEL, dev->iobase + DMM32AT_INTCLOCK);
}

static int dmm32at_ai_cmd(struct comedi_device *dev, struct comedi_subdevice *s)
{
	struct dmm32at_private *devpriv = dev->private;
	struct comedi_cmd *cmd = &s->async->cmd;
	int i, range;
	unsigned char chanlo, chanhi, status;

	if (!cmd->chanlist)
		return -EINVAL;

	/*                                */
	chanlo = CR_CHAN(cmd->chanlist[0]) & (s->n_chan - 1);
	chanhi = chanlo + cmd->chanlist_len - 1;
	if (chanhi >= s->n_chan)
		return -EINVAL;
	range = CR_RANGE(cmd->chanlist[0]);

	/*            */
	outb(DMM32AT_FIFORESET, dev->iobase + DMM32AT_FIFOCNTRL);

	/*                 */
	outb(DMM32AT_SCANENABLE, dev->iobase + DMM32AT_FIFOCNTRL);

	/*                                 */
	outb(chanlo, dev->iobase + DMM32AT_AILOW);
	outb(chanhi, dev->iobase + DMM32AT_AIHIGH);

	/*                    */
	outb(dmm32at_rangebits[range], dev->iobase + DMM32AT_AICONF);

	/*                                  */
	outb(DMM32AT_INTRESET, dev->iobase + DMM32AT_CNTRL);

	if (cmd->stop_src == TRIG_COUNT)
		devpriv->ai_scans_left = cmd->stop_arg;
	else {			/*           */
		devpriv->ai_scans_left = 0xffffffff; /*                       
                  */
	}

	/*                            */
	for (i = 0; i < 40000; i++) {
		status = inb(dev->iobase + DMM32AT_AIRBACK);
		if ((status & DMM32AT_STATUS) == 0)
			break;
	}
	if (i == 40000) {
		printk(KERN_WARNING "dmm32at: timeout\n");
		return -ETIMEDOUT;
	}

	if (devpriv->ai_scans_left > 1) {
		/*                                           */
		dmm32at_setaitimer(dev, cmd->scan_begin_arg);
	} else {
		/*                                                */
		outb(DMM32AT_ADINT, dev->iobase + DMM32AT_INTCLOCK);
		outb(0xff, dev->iobase + DMM32AT_CONV);
	}

/*                                 */

/*                                  */
/*                                  */

/*                                    */
/*                       */

	return 0;

}

static int dmm32at_ai_cancel(struct comedi_device *dev,
			     struct comedi_subdevice *s)
{
	struct dmm32at_private *devpriv = dev->private;

	devpriv->ai_scans_left = 1;
	return 0;
}

static irqreturn_t dmm32at_isr(int irq, void *d)
{
	struct comedi_device *dev = d;
	struct dmm32at_private *devpriv = dev->private;
	unsigned char intstat;
	unsigned int samp;
	unsigned short msb, lsb;
	int i;

	if (!dev->attached) {
		comedi_error(dev, "spurious interrupt");
		return IRQ_HANDLED;
	}

	intstat = inb(dev->iobase + DMM32AT_INTCLOCK);

	if (intstat & DMM32AT_ADINT) {
		struct comedi_subdevice *s = dev->read_subdev;
		struct comedi_cmd *cmd = &s->async->cmd;

		for (i = 0; i < cmd->chanlist_len; i++) {
			/*           */
			lsb = inb(dev->iobase + DMM32AT_AILSB);
			msb = inb(dev->iobase + DMM32AT_AIMSB);

			/*                                        */
			samp = ((msb ^ 0x0080) << 8) + lsb;
			comedi_buf_put(s->async, samp);
		}

		if (devpriv->ai_scans_left != 0xffffffff) {	/*            */
			devpriv->ai_scans_left--;
			if (devpriv->ai_scans_left == 0) {
				/*                                       */
				outb(0x0, dev->iobase + DMM32AT_INTCLOCK);
				/*                                          */
				s->async->events |= COMEDI_CB_EOA;
			}

		}
		/*                  */
		comedi_event(dev, s);
	}

	/*                     */
	outb(DMM32AT_INTRESET, dev->iobase + DMM32AT_CNTRL);
	return IRQ_HANDLED;
}

static int dmm32at_ao_winsn(struct comedi_device *dev,
			    struct comedi_subdevice *s,
			    struct comedi_insn *insn, unsigned int *data)
{
	struct dmm32at_private *devpriv = dev->private;
	int i;
	int chan = CR_CHAN(insn->chanspec);
	unsigned char hi, lo, status;

	/*                                                          
                                                          */
	for (i = 0; i < insn->n; i++) {

		devpriv->ao_readback[chan] = data[i];

		/*                  */
		lo = data[i] & 0x00ff;
		/*                                        */
		hi = (data[i] >> 8) + chan * (1 << 6);
		/*                                           */
		/*                                            */
		outb(lo, dev->iobase + DMM32AT_DACLSB);
		outb(hi, dev->iobase + DMM32AT_DACMSB);

		/*                            */
		for (i = 0; i < 40000; i++) {
			status = inb(dev->iobase + DMM32AT_DACSTAT);
			if ((status & DMM32AT_DACBUSY) == 0)
				break;
		}
		if (i == 40000) {
			printk(KERN_WARNING "dmm32at: timeout\n");
			return -ETIMEDOUT;
		}
		/*                                         */
		status = inb(dev->iobase + DMM32AT_DACMSB);

	}

	/*                                           */
	return i;
}

static int dmm32at_ao_rinsn(struct comedi_device *dev,
			    struct comedi_subdevice *s,
			    struct comedi_insn *insn, unsigned int *data)
{
	struct dmm32at_private *devpriv = dev->private;
	int i;
	int chan = CR_CHAN(insn->chanspec);

	for (i = 0; i < insn->n; i++)
		data[i] = devpriv->ao_readback[chan];

	return i;
}

static int dmm32at_dio_insn_bits(struct comedi_device *dev,
				 struct comedi_subdevice *s,
				 struct comedi_insn *insn, unsigned int *data)
{
	struct dmm32at_private *devpriv = dev->private;
	unsigned char diobits;

	/*                                                    
                                                     */
	if (data[0]) {
		s->state &= ~data[0];
		s->state |= data[0] & data[1];
		/*                                        */
		/*                                           */
	}

	/*                            */
	outb(DMM32AT_DIOACC, dev->iobase + DMM32AT_CNTRL);

	/*                                         */
	if (((devpriv->dio_config & DMM32AT_DIRCL) == 0) ||
	    ((devpriv->dio_config & DMM32AT_DIRCH) == 0)) {
		diobits = (s->state & 0x00ff0000) >> 16;
		outb(diobits, dev->iobase + DMM32AT_DIOC);
	}
	if ((devpriv->dio_config & DMM32AT_DIRB) == 0) {
		diobits = (s->state & 0x0000ff00) >> 8;
		outb(diobits, dev->iobase + DMM32AT_DIOB);
	}
	if ((devpriv->dio_config & DMM32AT_DIRA) == 0) {
		diobits = (s->state & 0x000000ff);
		outb(diobits, dev->iobase + DMM32AT_DIOA);
	}

	/*                            */
	s->state = inb(dev->iobase + DMM32AT_DIOC);
	s->state <<= 8;
	s->state |= inb(dev->iobase + DMM32AT_DIOB);
	s->state <<= 8;
	s->state |= inb(dev->iobase + DMM32AT_DIOA);
	data[1] = s->state;

	/*                                                     
                            */
	/*                                         */
	/*                                                                  
                                             */
	/*                   */

	return insn->n;
}

static int dmm32at_dio_insn_config(struct comedi_device *dev,
				   struct comedi_subdevice *s,
				   struct comedi_insn *insn, unsigned int *data)
{
	struct dmm32at_private *devpriv = dev->private;
	unsigned char chanbit;
	int chan = CR_CHAN(insn->chanspec);

	if (insn->n != 1)
		return -EINVAL;

	if (chan < 8)
		chanbit = DMM32AT_DIRA;
	else if (chan < 16)
		chanbit = DMM32AT_DIRB;
	else if (chan < 20)
		chanbit = DMM32AT_DIRCL;
	else
		chanbit = DMM32AT_DIRCH;

	/*                                                          
                                                              
                                                                
                                         */

	/*                                           */
	if (data[0] == COMEDI_OUTPUT)
		devpriv->dio_config &= ~chanbit;
	else
		devpriv->dio_config |= chanbit;
	/*                            */
	outb(DMM32AT_DIOACC, dev->iobase + DMM32AT_CNTRL);
	/*                                                */
	outb(devpriv->dio_config, dev->iobase + DMM32AT_DIOCONF);

	return 1;
}

static int dmm32at_attach(struct comedi_device *dev,
			  struct comedi_devconfig *it)
{
	struct dmm32at_private *devpriv;
	int ret;
	struct comedi_subdevice *s;
	unsigned char aihi, ailo, fifostat, aistat, intstat, airback;
	unsigned int irq;

	irq = it->options[1];

	ret = comedi_request_region(dev, it->options[0], DMM32AT_MEMSIZE);
	if (ret)
		return ret;

	/*                                                          
                        */

	/*                 */
	outb(DMM32AT_RESET, dev->iobase + DMM32AT_CNTRL);

	/*                              */
	udelay(1000);

	/*                            */
	outb(0x0, dev->iobase + DMM32AT_FIFOCNTRL);

	/*                                  */
	outb(0x0, dev->iobase + DMM32AT_INTCLOCK);

	/*                                                         */
	outb(0x80, dev->iobase + DMM32AT_AILOW);
	outb(0xff, dev->iobase + DMM32AT_AIHIGH);

	/*                               */
	outb(DMM32AT_RANGE_U10, dev->iobase + DMM32AT_AICONF);

	/*                                               */
	udelay(100);

	/*                      */
	ailo = inb(dev->iobase + DMM32AT_AILOW);
	aihi = inb(dev->iobase + DMM32AT_AIHIGH);
	fifostat = inb(dev->iobase + DMM32AT_FIFOSTAT);
	aistat = inb(dev->iobase + DMM32AT_AISTAT);
	intstat = inb(dev->iobase + DMM32AT_INTCLOCK);
	airback = inb(dev->iobase + DMM32AT_AIRBACK);

	printk(KERN_DEBUG "dmm32at: lo=0x%02x hi=0x%02x fifostat=0x%02x\n",
	       ailo, aihi, fifostat);
	printk(KERN_DEBUG
	       "dmm32at: aistat=0x%02x intstat=0x%02x airback=0x%02x\n",
	       aistat, intstat, airback);

	if ((ailo != 0x00) || (aihi != 0x1f) || (fifostat != 0x80) ||
	    (aistat != 0x60 || (intstat != 0x00) || airback != 0x0c)) {
		printk(KERN_ERR "dmmat32: board detection failed\n");
		return -EIO;
	}

	/*                                    */
	if (irq) {
		ret = request_irq(irq, dmm32at_isr, 0, dev->board_name, dev);
		if (ret < 0) {
			printk(KERN_ERR "dmm32at: irq conflict\n");
			return ret;
		}
		dev->irq = irq;
	}

	devpriv = kzalloc(sizeof(*devpriv), GFP_KERNEL);
	if (!devpriv)
		return -ENOMEM;
	dev->private = devpriv;

	ret = comedi_alloc_subdevices(dev, 3);
	if (ret)
		return ret;

	s = &dev->subdevices[0];
	dev->read_subdev = s;
	/*                        */
	s->type = COMEDI_SUBD_AI;
	/*                                                   */
	s->subdev_flags = SDF_READABLE | SDF_GROUND | SDF_DIFF | SDF_CMD_READ;
	s->n_chan = 32;
	s->maxdata = 0xffff;
	s->range_table = &dmm32at_airanges;
	s->len_chanlist = 32;	/*                                         
                            */
	s->insn_read = dmm32at_ai_rinsn;
	s->do_cmd = dmm32at_ai_cmd;
	s->do_cmdtest = dmm32at_ai_cmdtest;
	s->cancel = dmm32at_ai_cancel;

	s = &dev->subdevices[1];
	/*                         */
	s->type = COMEDI_SUBD_AO;
	s->subdev_flags = SDF_WRITABLE;
	s->n_chan = 4;
	s->maxdata = 0x0fff;
	s->range_table = &dmm32at_aoranges;
	s->insn_write = dmm32at_ao_winsn;
	s->insn_read = dmm32at_ao_rinsn;

	s = &dev->subdevices[2];
	/*                       */

	/*                            */
	outb(DMM32AT_DIOACC, dev->iobase + DMM32AT_CNTRL);
	/*                                            */
	devpriv->dio_config = DMM32AT_DIRA | DMM32AT_DIRB |
		DMM32AT_DIRCL | DMM32AT_DIRCH | DMM32AT_DIENABLE;
	outb(devpriv->dio_config, dev->iobase + DMM32AT_DIOCONF);

	/*                      */
	s->type = COMEDI_SUBD_DIO;
	s->subdev_flags = SDF_READABLE | SDF_WRITABLE;
	s->n_chan = 24;
	s->maxdata = 1;
	s->state = 0;
	s->range_table = &range_digital;
	s->insn_bits = dmm32at_dio_insn_bits;
	s->insn_config = dmm32at_dio_insn_config;

	/*         */
	printk(KERN_INFO "comedi%d: dmm32at: attached\n", dev->minor);

	return 1;

}

static struct comedi_driver dmm32at_driver = {
	.driver_name	= "dmm32at",
	.module		= THIS_MODULE,
	.attach		= dmm32at_attach,
	.detach		= comedi_legacy_detach,
};
module_comedi_driver(dmm32at_driver);

MODULE_AUTHOR("Comedi http://www.comedi.org");
MODULE_DESCRIPTION("Comedi low-level driver");
MODULE_LICENSE("GPL");
