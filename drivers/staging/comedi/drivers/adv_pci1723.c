/*******************************************************************************
   comedi/drivers/pci1723.c

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

*******************************************************************************/
/*
                   
                               
                                                                   
                                           
                                        
             

                      
                                    
                                     

                                                   
                                

                                                    

                                                                         
                                                                         
                                          

     

                                                                            
                                                                             
                          
                         
*/

#include <linux/pci.h>

#include "../comedidev.h"

/*                                         */
#define PCI1723_DA(N)   ((N)<<1)	/*                            */

#define PCI1723_SYN_SET  0x12		/*                           */
#define PCI1723_ALL_CHNNELE_SYN_STROBE  0x12
					/*                              */

#define PCI1723_RANGE_CALIBRATION_MODE 0x14
					/*                            */
#define PCI1723_RANGE_CALIBRATION_STATUS 0x14
					/*                              */

#define PCI1723_CONTROL_CMD_CALIBRATION_FUN 0x16
						/*
                                 
                             
       */
#define PCI1723_STATUS_CMD_CALIBRATION_FUN 0x16
						/*
                                
                             
       */

#define PCI1723_CALIBRATION_PARA_STROBE 0x18
					/*                              */

#define PCI1723_DIGITAL_IO_PORT_SET 0x1A	/*                          */
#define PCI1723_DIGITAL_IO_PORT_MODE 0x1A	/*                       */

#define PCI1723_WRITE_DIGITAL_OUTPUT_CMD 0x1C
					/*                              */
#define PCI1723_READ_DIGITAL_INPUT_DATA 0x1C	/*                         */

#define PCI1723_WRITE_CAL_CMD 0x1E		/*                           */
#define PCI1723_READ_CAL_STATUS 0x1E		/*                         */

#define PCI1723_SYN_STROBE 0x20			/*                     */

#define PCI1723_RESET_ALL_CHN_STROBE 0x22
					/*                               */

#define PCI1723_RESET_CAL_CONTROL_STROBE 0x24
						/*
                              
                          
       */

#define PCI1723_CHANGE_CHA_OUTPUT_TYPE_STROBE 0x26
						/*
                                   
                    
       */

#define PCI1723_SELECT_CALIBRATION 0x28	/*                              */

struct pci1723_private {
	unsigned char da_range[8];	/*                                   */
	short ao_data[8];	/*                    */
};

/*
                          
 */
static int pci1723_reset(struct comedi_device *dev)
{
	struct pci1723_private *devpriv = dev->private;
	int i;

	outw(0x01, dev->iobase + PCI1723_SYN_SET);
					       /*                             */

	for (i = 0; i < 8; i++) {
		/*                       */
		devpriv->ao_data[i] = 0x8000;
		outw(devpriv->ao_data[i], dev->iobase + PCI1723_DA(i));
		/*                           */
		devpriv->da_range[i] = 0;
		outw(((devpriv->da_range[i] << 4) | i),
		     PCI1723_RANGE_CALIBRATION_MODE);
	}

	outw(0, dev->iobase + PCI1723_CHANGE_CHA_OUTPUT_TYPE_STROBE);
							    /*               */
	outw(0, dev->iobase + PCI1723_SYN_STROBE);	    /*                */

	/*                              */
	outw(0, dev->iobase + PCI1723_SYN_SET);

	return 0;
}

static int pci1723_insn_read_ao(struct comedi_device *dev,
				struct comedi_subdevice *s,
				struct comedi_insn *insn, unsigned int *data)
{
	struct pci1723_private *devpriv = dev->private;
	int n, chan;

	chan = CR_CHAN(insn->chanspec);
	for (n = 0; n < insn->n; n++)
		data[n] = devpriv->ao_data[chan];

	return n;
}

/*
                     
*/
static int pci1723_ao_write_winsn(struct comedi_device *dev,
				  struct comedi_subdevice *s,
				  struct comedi_insn *insn, unsigned int *data)
{
	struct pci1723_private *devpriv = dev->private;
	int n, chan;
	chan = CR_CHAN(insn->chanspec);

	for (n = 0; n < insn->n; n++) {

		devpriv->ao_data[chan] = data[n];
		outw(data[n], dev->iobase + PCI1723_DA(chan));
	}

	return n;
}

/*
                          
*/
static int pci1723_dio_insn_config(struct comedi_device *dev,
				   struct comedi_subdevice *s,
				   struct comedi_insn *insn, unsigned int *data)
{
	unsigned int mask;
	unsigned int bits;
	unsigned short dio_mode;

	mask = 1 << CR_CHAN(insn->chanspec);
	if (mask & 0x00FF)
		bits = 0x00FF;
	else
		bits = 0xFF00;

	switch (data[0]) {
	case INSN_CONFIG_DIO_INPUT:
		s->io_bits &= ~bits;
		break;
	case INSN_CONFIG_DIO_OUTPUT:
		s->io_bits |= bits;
		break;
	case INSN_CONFIG_DIO_QUERY:
		data[1] = (s->io_bits & bits) ? COMEDI_OUTPUT : COMEDI_INPUT;
		return insn->n;
	default:
		return -EINVAL;
	}

	/*                          */
	dio_mode = 0x0000;	/*                                   */
	if ((s->io_bits & 0x00FF) == 0)
		dio_mode |= 0x0001;	/*                */
	if ((s->io_bits & 0xFF00) == 0)
		dio_mode |= 0x0002;	/*                 */
	outw(dio_mode, dev->iobase + PCI1723_DIGITAL_IO_PORT_SET);
	return 1;
}

/*
                             
*/
static int pci1723_dio_insn_bits(struct comedi_device *dev,
				 struct comedi_subdevice *s,
				 struct comedi_insn *insn, unsigned int *data)
{
	if (data[0]) {
		s->state &= ~data[0];
		s->state |= (data[0] & data[1]);
		outw(s->state, dev->iobase + PCI1723_WRITE_DIGITAL_OUTPUT_CMD);
	}
	data[1] = inw(dev->iobase + PCI1723_READ_DIGITAL_INPUT_DATA);
	return insn->n;
}

static int pci1723_auto_attach(struct comedi_device *dev,
					 unsigned long context_unused)
{
	struct pci_dev *pcidev = comedi_to_pci_dev(dev);
	struct pci1723_private *devpriv;
	struct comedi_subdevice *s;
	int ret;

	devpriv = kzalloc(sizeof(*devpriv), GFP_KERNEL);
	if (!devpriv)
		return -ENOMEM;
	dev->private = devpriv;

	ret = comedi_pci_enable(dev);
	if (ret)
		return ret;
	dev->iobase = pci_resource_start(pcidev, 2);

	ret = comedi_alloc_subdevices(dev, 2);
	if (ret)
		return ret;

	s = &dev->subdevices[0];
	dev->write_subdev = s;
	s->type		= COMEDI_SUBD_AO;
	s->subdev_flags	= SDF_WRITEABLE | SDF_GROUND | SDF_COMMON;
	s->n_chan	= 8;
	s->maxdata	= 0xffff;
	s->len_chanlist	= 8;
	s->range_table	= &range_bipolar10;
	s->insn_write	= pci1723_ao_write_winsn;
	s->insn_read	= pci1723_insn_read_ao;

	s = &dev->subdevices[1];
	s->type		= COMEDI_SUBD_DIO;
	s->subdev_flags	= SDF_READABLE | SDF_WRITABLE;
	s->n_chan	= 16;
	s->maxdata	= 1;
	s->len_chanlist	= 16;
	s->range_table	= &range_digital;
	s->insn_config	= pci1723_dio_insn_config;
	s->insn_bits	= pci1723_dio_insn_bits;

	/*                 */
	switch (inw(dev->iobase + PCI1723_DIGITAL_IO_PORT_MODE) & 0x03) {
	case 0x00:	/*                                   */
		s->io_bits = 0xFFFF;
		break;
	case 0x01:	/*                                  */
		s->io_bits = 0xFF00;
		break;
	case 0x02:	/*                                  */
		s->io_bits = 0x00FF;
		break;
	case 0x03:	/*                                 */
		s->io_bits = 0x0000;
		break;
	}
	/*                     */
	s->state = inw(dev->iobase + PCI1723_READ_DIGITAL_INPUT_DATA);

	pci1723_reset(dev);

	dev_info(dev->class_dev, "%s attached\n", dev->board_name);

	return 0;
}

static void pci1723_detach(struct comedi_device *dev)
{
	if (dev->iobase)
		pci1723_reset(dev);
	comedi_pci_disable(dev);
}

static struct comedi_driver adv_pci1723_driver = {
	.driver_name	= "adv_pci1723",
	.module		= THIS_MODULE,
	.auto_attach	= pci1723_auto_attach,
	.detach		= pci1723_detach,
};

static int adv_pci1723_pci_probe(struct pci_dev *dev,
				 const struct pci_device_id *id)
{
	return comedi_pci_auto_config(dev, &adv_pci1723_driver,
				      id->driver_data);
}

static DEFINE_PCI_DEVICE_TABLE(adv_pci1723_pci_table) = {
	{ PCI_DEVICE(PCI_VENDOR_ID_ADVANTECH, 0x1723) },
	{ 0 }
};
MODULE_DEVICE_TABLE(pci, adv_pci1723_pci_table);

static struct pci_driver adv_pci1723_pci_driver = {
	.name		= "adv_pci1723",
	.id_table	= adv_pci1723_pci_table,
	.probe		= adv_pci1723_pci_probe,
	.remove		= comedi_pci_auto_unconfig,
};
module_comedi_pci_driver(adv_pci1723_driver, adv_pci1723_pci_driver);

MODULE_AUTHOR("Comedi http://www.comedi.org");
MODULE_DESCRIPTION("Comedi low-level driver");
MODULE_LICENSE("GPL");
