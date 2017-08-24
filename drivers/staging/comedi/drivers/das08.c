/*
 *  comedi/drivers/das08.c
 *  comedi driver for common DAS08 support (used by ISA/PCI/PCMCIA drivers)
 *
 *  COMEDI - Linux Control and Measurement Device Interface
 *  Copyright (C) 2000 David A. Schleef <ds@schleef.org>
 *  Copyright (C) 2001,2002,2003 Frank Mori Hess <fmhess@users.sourceforge.net>
 *  Copyright (C) 2004 Salvador E. Tropea <set@users.sf.net> <set@ieee.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *****************************************************************
 */

/*
                
                                        
                                                                   
                                        
                                           
                
  
                                                                
                                                                 
  
                                                              
                                                    
 */

#include <linux/delay.h>

#include "../comedidev.h"

#include "8255.h"
#include "8253.h"
#include "das08.h"

/*
                 

             

                             
                               
                                         
                   
           
           

                                   

*/

#define DAS08_LSB		0
#define DAS08_MSB		1
#define DAS08_TRIG_12BIT	1
#define DAS08_STATUS		2
#define   DAS08_EOC			(1<<7)
#define   DAS08_IRQ			(1<<3)
#define   DAS08_IP(x)			(((x)>>4)&0x7)
#define DAS08_CONTROL		2
#define   DAS08_MUX_MASK	0x7
#define   DAS08_MUX(x)		((x) & DAS08_MUX_MASK)
#define   DAS08_INTE			(1<<3)
#define   DAS08_DO_MASK		0xf0
#define   DAS08_OP(x)		(((x) << 4) & DAS08_DO_MASK)

/*
                   

               

                        
                               
                 
           
                    
                    
                    
                    

*/

#define DAS08JR_DIO		3
#define DAS08JR_AO_LSB(x)	((x) ? 6 : 4)
#define DAS08JR_AO_MSB(x)	((x) ? 7 : 5)

/*
                     

             
             
             

                             
                               
                                         
                                 
           
                    
                    
                    
                    
      
           
*/

#define DAS08AO_GAIN_CONTROL	3
#define DAS08AO_GAIN_STATUS	3

#define DAS08AO_AO_LSB(x)	((x) ? 0xa : 8)
#define DAS08AO_AO_MSB(x)	((x) ? 0xb : 9)
#define DAS08AO_AO_UPDATE	8

/*                              */

static const struct comedi_lrange range_das08_pgl = { 9, {
							  BIP_RANGE(10),
							  BIP_RANGE(5),
							  BIP_RANGE(2.5),
							  BIP_RANGE(1.25),
							  BIP_RANGE(0.625),
							  UNI_RANGE(10),
							  UNI_RANGE(5),
							  UNI_RANGE(2.5),
							  UNI_RANGE(1.25)
							  }
};

static const struct comedi_lrange range_das08_pgh = { 12, {
							   BIP_RANGE(10),
							   BIP_RANGE(5),
							   BIP_RANGE(1),
							   BIP_RANGE(0.5),
							   BIP_RANGE(0.1),
							   BIP_RANGE(0.05),
							   BIP_RANGE(0.01),
							   BIP_RANGE(0.005),
							   UNI_RANGE(10),
							   UNI_RANGE(1),
							   UNI_RANGE(0.1),
							   UNI_RANGE(0.01),
							   }
};

static const struct comedi_lrange range_das08_pgm = { 9, {
							  BIP_RANGE(10),
							  BIP_RANGE(5),
							  BIP_RANGE(0.5),
							  BIP_RANGE(0.05),
							  BIP_RANGE(0.01),
							  UNI_RANGE(10),
							  UNI_RANGE(1),
							  UNI_RANGE(0.1),
							  UNI_RANGE(0.01)
							  }
};				/*
                      

                    

                                       
                                             
                                    
                                   
                                        
                                        
                                        
                                        

     */

static const struct comedi_lrange *const das08_ai_lranges[] = {
	&range_unknown,
	&range_bipolar5,
	&range_das08_pgh,
	&range_das08_pgl,
	&range_das08_pgm,
};

static const int das08_pgh_gainlist[] = {
	8, 0, 10, 2, 12, 4, 14, 6, 1, 3, 5, 7
};
static const int das08_pgl_gainlist[] = { 8, 0, 2, 4, 6, 1, 3, 5, 7 };
static const int das08_pgm_gainlist[] = { 8, 0, 10, 12, 14, 9, 11, 13, 15 };

static const int *const das08_gainlists[] = {
	NULL,
	NULL,
	das08_pgh_gainlist,
	das08_pgl_gainlist,
	das08_pgm_gainlist,
};

#define TIMEOUT 100000

static int das08_ai_rinsn(struct comedi_device *dev, struct comedi_subdevice *s,
			  struct comedi_insn *insn, unsigned int *data)
{
	const struct das08_board_struct *thisboard = comedi_board(dev);
	struct das08_private_struct *devpriv = dev->private;
	int i, n;
	int chan;
	int range;
	int lsb, msb;

	chan = CR_CHAN(insn->chanspec);
	range = CR_RANGE(insn->chanspec);

	/*            */
	inb(dev->iobase + DAS08_LSB);
	inb(dev->iobase + DAS08_MSB);

	/*                 */
	/*                                           */
	spin_lock(&dev->spinlock);
	devpriv->do_mux_bits &= ~DAS08_MUX_MASK;
	devpriv->do_mux_bits |= DAS08_MUX(chan);
	outb(devpriv->do_mux_bits, dev->iobase + DAS08_CONTROL);
	spin_unlock(&dev->spinlock);

	if (s->range_table->length > 1) {
		/*                */
		range = CR_RANGE(insn->chanspec);
		outb(devpriv->pg_gainlist[range],
		     dev->iobase + DAS08AO_GAIN_CONTROL);
	}

	for (n = 0; n < insn->n; n++) {
		/*                                         */
		if (thisboard->ai_nbits == 16)
			if (inb(dev->iobase + DAS08_MSB) & 0x80)
				dev_info(dev->class_dev, "over-range\n");

		/*                    */
		outb_p(0, dev->iobase + DAS08_TRIG_12BIT);

		for (i = 0; i < TIMEOUT; i++) {
			if (!(inb(dev->iobase + DAS08_STATUS) & DAS08_EOC))
				break;
		}
		if (i == TIMEOUT) {
			dev_err(dev->class_dev, "timeout\n");
			return -ETIME;
		}
		msb = inb(dev->iobase + DAS08_MSB);
		lsb = inb(dev->iobase + DAS08_LSB);
		if (thisboard->ai_encoding == das08_encode12) {
			data[n] = (lsb >> 4) | (msb << 4);
		} else if (thisboard->ai_encoding == das08_pcm_encode12) {
			data[n] = (msb << 8) + lsb;
		} else if (thisboard->ai_encoding == das08_encode16) {
			/*                                       */
			if (msb & 0x80)
				data[n] = (1 << 15) | lsb | ((msb & 0x7f) << 8);
			else
				data[n] = (1 << 15) - (lsb | (msb & 0x7f) << 8);
		} else {
			comedi_error(dev, "bug! unknown ai encoding");
			return -1;
		}
	}

	return n;
}

static int das08_di_rbits(struct comedi_device *dev, struct comedi_subdevice *s,
			  struct comedi_insn *insn, unsigned int *data)
{
	data[0] = 0;
	data[1] = DAS08_IP(inb(dev->iobase + DAS08_STATUS));

	return insn->n;
}

static int das08_do_wbits(struct comedi_device *dev, struct comedi_subdevice *s,
			  struct comedi_insn *insn, unsigned int *data)
{
	struct das08_private_struct *devpriv = dev->private;
	int wbits;

	/*                                               */
	wbits = (devpriv->do_mux_bits >> 4) & 0xf;
	/*                                */
	wbits &= ~data[0];
	/*                     */
	wbits |= data[0] & data[1];
	/*                               */
	/*                                                */
	spin_lock(&dev->spinlock);
	devpriv->do_mux_bits &= ~DAS08_DO_MASK;
	devpriv->do_mux_bits |= DAS08_OP(wbits);
	outb(devpriv->do_mux_bits, dev->iobase + DAS08_CONTROL);
	spin_unlock(&dev->spinlock);

	data[1] = wbits;

	return insn->n;
}

static int das08jr_di_rbits(struct comedi_device *dev,
			    struct comedi_subdevice *s,
			    struct comedi_insn *insn, unsigned int *data)
{
	data[0] = 0;
	data[1] = inb(dev->iobase + DAS08JR_DIO);

	return insn->n;
}

static int das08jr_do_wbits(struct comedi_device *dev,
			    struct comedi_subdevice *s,
			    struct comedi_insn *insn, unsigned int *data)
{
	struct das08_private_struct *devpriv = dev->private;

	/*                                */
	devpriv->do_bits &= ~data[0];
	/*                     */
	devpriv->do_bits |= data[0] & data[1];
	outb(devpriv->do_bits, dev->iobase + DAS08JR_DIO);

	data[1] = devpriv->do_bits;

	return insn->n;
}

static void das08_ao_set_data(struct comedi_device *dev,
			      unsigned int chan, unsigned int data)
{
	const struct das08_board_struct *thisboard = comedi_board(dev);
	struct das08_private_struct *devpriv = dev->private;
	unsigned char lsb;
	unsigned char msb;

	lsb = data & 0xff;
	msb = (data >> 8) & 0xff;
	if (thisboard->is_jr) {
		outb(lsb, dev->iobase + DAS08JR_AO_LSB(chan));
		outb(msb, dev->iobase + DAS08JR_AO_MSB(chan));
		/*           */
		inb(dev->iobase + DAS08JR_DIO);
	} else {
		outb(lsb, dev->iobase + DAS08AO_AO_LSB(chan));
		outb(msb, dev->iobase + DAS08AO_AO_MSB(chan));
		/*           */
		inb(dev->iobase + DAS08AO_AO_UPDATE);
	}
	devpriv->ao_readback[chan] = data;
}

static void das08_ao_initialize(struct comedi_device *dev,
				struct comedi_subdevice *s)
{
	int n;
	unsigned int data;

	data = s->maxdata / 2;	/*                         */
	for (n = 0; n < s->n_chan; n++)
		das08_ao_set_data(dev, n, data);
}

static int das08_ao_winsn(struct comedi_device *dev,
			  struct comedi_subdevice *s,
			  struct comedi_insn *insn, unsigned int *data)
{
	unsigned int n;
	unsigned int chan;

	chan = CR_CHAN(insn->chanspec);

	for (n = 0; n < insn->n; n++)
		das08_ao_set_data(dev, chan, *data);

	return n;
}

static int das08_ao_rinsn(struct comedi_device *dev,
			  struct comedi_subdevice *s,
			  struct comedi_insn *insn, unsigned int *data)
{
	struct das08_private_struct *devpriv = dev->private;
	unsigned int n;
	unsigned int chan;

	chan = CR_CHAN(insn->chanspec);

	for (n = 0; n < insn->n; n++)
		data[n] = devpriv->ao_readback[chan];

	return n;
}

static void i8254_initialize(struct comedi_device *dev)
{
	const struct das08_board_struct *thisboard = comedi_board(dev);
	unsigned long i8254_iobase = dev->iobase + thisboard->i8254_offset;
	unsigned int mode = I8254_MODE0 | I8254_BINARY;
	int i;

	for (i = 0; i < 3; ++i)
		i8254_set_mode(i8254_iobase, 0, i, mode);
}

static int das08_counter_read(struct comedi_device *dev,
			      struct comedi_subdevice *s,
			      struct comedi_insn *insn, unsigned int *data)
{
	const struct das08_board_struct *thisboard = comedi_board(dev);
	unsigned long i8254_iobase = dev->iobase + thisboard->i8254_offset;
	int chan = insn->chanspec;

	data[0] = i8254_read(i8254_iobase, 0, chan);
	return 1;
}

static int das08_counter_write(struct comedi_device *dev,
			       struct comedi_subdevice *s,
			       struct comedi_insn *insn, unsigned int *data)
{
	const struct das08_board_struct *thisboard = comedi_board(dev);
	unsigned long i8254_iobase = dev->iobase + thisboard->i8254_offset;
	int chan = insn->chanspec;

	i8254_write(i8254_iobase, 0, chan, data[0]);
	return 1;
}

static int das08_counter_config(struct comedi_device *dev,
				struct comedi_subdevice *s,
				struct comedi_insn *insn, unsigned int *data)
{
	const struct das08_board_struct *thisboard = comedi_board(dev);
	unsigned long i8254_iobase = dev->iobase + thisboard->i8254_offset;
	int chan = insn->chanspec;

	switch (data[0]) {
	case INSN_CONFIG_SET_COUNTER_MODE:
		i8254_set_mode(i8254_iobase, 0, chan, data[1]);
		break;
	case INSN_CONFIG_8254_READ_STATUS:
		data[1] = i8254_status(i8254_iobase, 0, chan);
		break;
	default:
		return -EINVAL;
		break;
	}
	return 2;
}

int das08_common_attach(struct comedi_device *dev, unsigned long iobase)
{
	const struct das08_board_struct *thisboard = comedi_board(dev);
	struct das08_private_struct *devpriv = dev->private;
	struct comedi_subdevice *s;
	int ret;

	dev->iobase = iobase;

	dev->board_name = thisboard->name;

	ret = comedi_alloc_subdevices(dev, 6);
	if (ret)
		return ret;

	s = &dev->subdevices[0];
	/*    */
	if (thisboard->ai_nbits) {
		s->type = COMEDI_SUBD_AI;
		/*                                           
                                    
                                               
                         
   */
		s->subdev_flags = SDF_READABLE | SDF_GROUND;
		s->n_chan = 8;
		s->maxdata = (1 << thisboard->ai_nbits) - 1;
		s->range_table = das08_ai_lranges[thisboard->ai_pg];
		s->insn_read = das08_ai_rinsn;
		devpriv->pg_gainlist = das08_gainlists[thisboard->ai_pg];
	} else {
		s->type = COMEDI_SUBD_UNUSED;
	}

	s = &dev->subdevices[1];
	/*    */
	if (thisboard->ao_nbits) {
		s->type = COMEDI_SUBD_AO;
		s->subdev_flags = SDF_WRITABLE;
		s->n_chan = 2;
		s->maxdata = (1 << thisboard->ao_nbits) - 1;
		s->range_table = &range_bipolar5;
		s->insn_write = das08_ao_winsn;
		s->insn_read = das08_ao_rinsn;
		das08_ao_initialize(dev, s);
	} else {
		s->type = COMEDI_SUBD_UNUSED;
	}

	s = &dev->subdevices[2];
	/*    */
	if (thisboard->di_nchan) {
		s->type = COMEDI_SUBD_DI;
		s->subdev_flags = SDF_READABLE;
		s->n_chan = thisboard->di_nchan;
		s->maxdata = 1;
		s->range_table = &range_digital;
		s->insn_bits =
			thisboard->is_jr ? das08jr_di_rbits : das08_di_rbits;
	} else {
		s->type = COMEDI_SUBD_UNUSED;
	}

	s = &dev->subdevices[3];
	/*    */
	if (thisboard->do_nchan) {
		s->type = COMEDI_SUBD_DO;
		s->subdev_flags = SDF_WRITABLE | SDF_READABLE;
		s->n_chan = thisboard->do_nchan;
		s->maxdata = 1;
		s->range_table = &range_digital;
		s->insn_bits =
			thisboard->is_jr ? das08jr_do_wbits : das08_do_wbits;
	} else {
		s->type = COMEDI_SUBD_UNUSED;
	}

	s = &dev->subdevices[4];
	/*      */
	if (thisboard->i8255_offset != 0) {
		subdev_8255_init(dev, s, NULL, (unsigned long)(dev->iobase +
							       thisboard->
							       i8255_offset));
	} else {
		s->type = COMEDI_SUBD_UNUSED;
	}

	s = &dev->subdevices[5];
	/*      */
	if (thisboard->i8254_offset != 0) {
		s->type = COMEDI_SUBD_COUNTER;
		s->subdev_flags = SDF_WRITABLE | SDF_READABLE;
		s->n_chan = 3;
		s->maxdata = 0xFFFF;
		s->insn_read = das08_counter_read;
		s->insn_write = das08_counter_write;
		s->insn_config = das08_counter_config;
		i8254_initialize(dev);
	} else {
		s->type = COMEDI_SUBD_UNUSED;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(das08_common_attach);

void das08_common_detach(struct comedi_device *dev)
{
	comedi_spriv_free(dev, 4);
}
EXPORT_SYMBOL_GPL(das08_common_detach);

static int __init das08_init(void)
{
	return 0;
}
module_init(das08_init);

static void __exit das08_exit(void)
{
}
module_exit(das08_exit);

MODULE_AUTHOR("Comedi http://www.comedi.org");
MODULE_DESCRIPTION("Comedi low-level driver");
MODULE_LICENSE("GPL");
