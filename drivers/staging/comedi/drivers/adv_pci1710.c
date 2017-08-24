/*
                               
  
                                         
  
                                                             
                                
  
                                        
                                                                         
                                                                        
  
           
                                                               
                                                                   
                         
  
*/
/*
                   
                                                                
                                  
                                      
                                                                    
                                             
          
             

                                                  
                                             
                                             

                                                               
                                                                
           

                      
                                    
                                     
                                                      
                     
*/

#include <linux/pci.h>
#include <linux/interrupt.h>

#include "../comedidev.h"

#include "comedi_fc.h"
#include "8253.h"
#include "amcc_s5933.h"

#define PCI171x_PARANOIDCHECK	/*                                            
                                             
              */

/*                             */
#define TYPE_PCI171X	0
#define TYPE_PCI1713	2
#define TYPE_PCI1720	3

#define PCI171x_AD_DATA	 0	/*               */
#define PCI171x_SOFTTRG	 0	/*                           */
#define PCI171x_RANGE	 2	/*                              */
#define PCI171x_MUX	 4	/*                              */
#define PCI171x_STATUS	 6	/*                      */
#define PCI171x_CONTROL	 6	/*                       */
#define PCI171x_CLRINT	 8	/*                               */
#define PCI171x_CLRFIFO	 9	/*                 */
#define PCI171x_DA1	10	/*                   */
#define PCI171x_DA2	12	/*                   */
#define PCI171x_DAREF	14	/*                            */
#define PCI171x_DI	16	/*                  */
#define PCI171x_DO	16	/*                  */
#define PCI171x_CNT0	24	/*                     */
#define PCI171x_CNT1	26	/*                     */
#define PCI171x_CNT2	28	/*                     */
#define PCI171x_CNTCTRL	30	/*                           */

/*                                                                             
        */
#define	Status_FE	0x0100	/*                 */
#define Status_FH	0x0200	/*                     */
#define Status_FF	0x0400	/*                             */
#define Status_IRQ	0x0800	/*                */
/*                                              */
#define Control_CNT0	0x0040	/*                             
                                     */
#define Control_ONEFH	0x0020	/*                                            */
#define Control_IRQEN	0x0010	/*              */
#define Control_GATE	0x0008	/*                                        */
#define Control_EXT	0x0004	/*                           */
#define Control_PACER	0x0002	/*                                       */
#define Control_SW	0x0001	/*                                  */
/*                                                      */
#define Counter_BCD     0x0001	/*                                     */
#define Counter_M0      0x0002	/*                        */
#define Counter_M1      0x0004	/*                                */
#define Counter_M2      0x0008
#define Counter_RW0     0x0010	/*                                */
#define Counter_RW1     0x0020
#define Counter_SC0     0x0040	/*                                   */
#define Counter_SC1     0x0080	/*                      
                                */

#define PCI1720_DA0	 0	/*                     */
#define PCI1720_DA1	 2	/*                     */
#define PCI1720_DA2	 4	/*                     */
#define PCI1720_DA3	 6	/*                     */
#define PCI1720_RANGE	 8	/*                         */
#define PCI1720_SYNCOUT	 9	/*                                       */
#define PCI1720_SYNCONT	15	/*                               */

/*                                            */
#define Syncont_SC0	 1	/*                             */

static const struct comedi_lrange range_pci1710_3 = { 9, {
							  BIP_RANGE(5),
							  BIP_RANGE(2.5),
							  BIP_RANGE(1.25),
							  BIP_RANGE(0.625),
							  BIP_RANGE(10),
							  UNI_RANGE(10),
							  UNI_RANGE(5),
							  UNI_RANGE(2.5),
							  UNI_RANGE(1.25)
							  }
};

static const char range_codes_pci1710_3[] = { 0x00, 0x01, 0x02, 0x03, 0x04,
					      0x10, 0x11, 0x12, 0x13 };

static const struct comedi_lrange range_pci1710hg = { 12, {
							   BIP_RANGE(5),
							   BIP_RANGE(0.5),
							   BIP_RANGE(0.05),
							   BIP_RANGE(0.005),
							   BIP_RANGE(10),
							   BIP_RANGE(1),
							   BIP_RANGE(0.1),
							   BIP_RANGE(0.01),
							   UNI_RANGE(10),
							   UNI_RANGE(1),
							   UNI_RANGE(0.1),
							   UNI_RANGE(0.01)
							   }
};

static const char range_codes_pci1710hg[] = { 0x00, 0x01, 0x02, 0x03, 0x04,
					      0x05, 0x06, 0x07, 0x10, 0x11,
					      0x12, 0x13 };

static const struct comedi_lrange range_pci17x1 = { 5, {
							BIP_RANGE(10),
							BIP_RANGE(5),
							BIP_RANGE(2.5),
							BIP_RANGE(1.25),
							BIP_RANGE(0.625)
							}
};

static const char range_codes_pci17x1[] = { 0x00, 0x01, 0x02, 0x03, 0x04 };

static const struct comedi_lrange range_pci1720 = { 4, {
							UNI_RANGE(5),
							UNI_RANGE(10),
							BIP_RANGE(5),
							BIP_RANGE(10)
							}
};

static const struct comedi_lrange range_pci171x_da = { 2, {
							   UNI_RANGE(5),
							   UNI_RANGE(10),
							   }
};

enum pci1710_boardid {
	BOARD_PCI1710,
	BOARD_PCI1710HG,
	BOARD_PCI1711,
	BOARD_PCI1713,
	BOARD_PCI1720,
	BOARD_PCI1731,
};

struct boardtype {
	const char *name;	/*             */
	char have_irq;		/*                     */
	char cardtype;		/*                          */
	int n_aichan;		/*                   */
	int n_aichand;		/*                                */
	int n_aochan;		/*                   */
	int n_dichan;		/*                  */
	int n_dochan;		/*                  */
	int n_counter;		/*                  */
	int ai_maxdata;		/*                    */
	int ao_maxdata;		/*                    */
	const struct comedi_lrange *rangelist_ai;	/*                    */
	const char *rangecode_ai;	/*                              */
	const struct comedi_lrange *rangelist_ao;	/*                    */
	unsigned int ai_ns_min;	/*                                */
	unsigned int fifo_half_size;	/*                 */
};

static const struct boardtype boardtypes[] = {
	[BOARD_PCI1710] = {
		.name		= "pci1710",
		.have_irq	= 1,
		.cardtype	= TYPE_PCI171X,
		.n_aichan	= 16,
		.n_aichand	= 8,
		.n_aochan	= 2,
		.n_dichan	= 16,
		.n_dochan	= 16,
		.n_counter	= 1,
		.ai_maxdata	= 0x0fff,
		.ao_maxdata	= 0x0fff,
		.rangelist_ai	= &range_pci1710_3,
		.rangecode_ai	= range_codes_pci1710_3,
		.rangelist_ao	= &range_pci171x_da,
		.ai_ns_min	= 10000,
		.fifo_half_size	= 2048,
	},
	[BOARD_PCI1710HG] = {
		.name		= "pci1710hg",
		.have_irq	= 1,
		.cardtype	= TYPE_PCI171X,
		.n_aichan	= 16,
		.n_aichand	= 8,
		.n_aochan	= 2,
		.n_dichan	= 16,
		.n_dochan	= 16,
		.n_counter	= 1,
		.ai_maxdata	= 0x0fff,
		.ao_maxdata	= 0x0fff,
		.rangelist_ai	= &range_pci1710hg,
		.rangecode_ai	= range_codes_pci1710hg,
		.rangelist_ao	= &range_pci171x_da,
		.ai_ns_min	= 10000,
		.fifo_half_size	= 2048,
	},
	[BOARD_PCI1711] = {
		.name		= "pci1711",
		.have_irq	= 1,
		.cardtype	= TYPE_PCI171X,
		.n_aichan	= 16,
		.n_aochan	= 2,
		.n_dichan	= 16,
		.n_dochan	= 16,
		.n_counter	= 1,
		.ai_maxdata	= 0x0fff,
		.ao_maxdata	= 0x0fff,
		.rangelist_ai	= &range_pci17x1,
		.rangecode_ai	= range_codes_pci17x1,
		.rangelist_ao	= &range_pci171x_da,
		.ai_ns_min	= 10000,
		.fifo_half_size	= 512,
	},
	[BOARD_PCI1713] = {
		.name		= "pci1713",
		.have_irq	= 1,
		.cardtype	= TYPE_PCI1713,
		.n_aichan	= 32,
		.n_aichand	= 16,
		.ai_maxdata	= 0x0fff,
		.rangelist_ai	= &range_pci1710_3,
		.rangecode_ai	= range_codes_pci1710_3,
		.ai_ns_min	= 10000,
		.fifo_half_size	= 2048,
	},
	[BOARD_PCI1720] = {
		.name		= "pci1720",
		.cardtype	= TYPE_PCI1720,
		.n_aochan	= 4,
		.ao_maxdata	= 0x0fff,
		.rangelist_ao	= &range_pci1720,
	},
	[BOARD_PCI1731] = {
		.name		= "pci1731",
		.have_irq	= 1,
		.cardtype	= TYPE_PCI171X,
		.n_aichan	= 16,
		.n_dichan	= 16,
		.n_dochan	= 16,
		.ai_maxdata	= 0x0fff,
		.rangelist_ai	= &range_pci17x1,
		.rangecode_ai	= range_codes_pci17x1,
		.ai_ns_min	= 10000,
		.fifo_half_size	= 512,
	},
};

struct pci1710_private {
	char neverending_ai;	/*                     */
	unsigned int CntrlReg;	/*                   */
	unsigned int i8254_osc_base;	/*                                 */
	unsigned int ai_do;	/*                                     */
	unsigned int ai_act_scan;	/*                             */
	unsigned int ai_act_chan;	/*                                 */
	unsigned int ai_buf_ptr;	/*                             */
	unsigned char ai_eos;	/*                */
	unsigned char ai_et;
	unsigned int ai_et_CntrlReg;
	unsigned int ai_et_MuxVal;
	unsigned int ai_et_div1, ai_et_div2;
	unsigned int act_chanlist[32];	/*                          */
	unsigned char act_chanlist_len;	/*                  */
	unsigned char act_chanlist_pos;	/*                              */
	unsigned char da_ranges;	/*                                    */
	unsigned int ai_scans;	/*                  */
	unsigned int ai_n_chan;	/*                                */
	unsigned int *ai_chanlist;	/*                  */
	unsigned int ai_flags;	/*           */
	unsigned int ai_data_len;	/*                     */
	short *ai_data;		/*              */
	unsigned int ai_timer1;	/*         */
	unsigned int ai_timer2;
	short ao_data[4];	/*                     */
	unsigned int cnt0_write_wait;	/*                                      
                       */
};

/*                                 */
static const unsigned int muxonechan[] = {
	0x0000, 0x0101, 0x0202, 0x0303, 0x0404, 0x0505, 0x0606, 0x0707,
	0x0808, 0x0909, 0x0a0a, 0x0b0b, 0x0c0c, 0x0d0d, 0x0e0e, 0x0f0f,
	0x1010, 0x1111, 0x1212, 0x1313, 0x1414, 0x1515, 0x1616, 0x1717,
	0x1818, 0x1919, 0x1a1a, 0x1b1b, 0x1c1c, 0x1d1d, 0x1e1e, 0x1f1f
};

/*
                                                                              
                                                   
                                          
                          
*/
static int check_channel_list(struct comedi_device *dev,
			      struct comedi_subdevice *s,
			      unsigned int *chanlist, unsigned int n_chan)
{
	unsigned int chansegment[32];
	unsigned int i, nowmustbechan, seglen, segpos;

	/*                                                              */
	if (n_chan < 1) {
		comedi_error(dev, "range/channel list is empty!");
		return 0;
	}

	if (n_chan == 1)
		return 1; /*          */

	chansegment[0] = chanlist[0]; /*                                 */
	for (i = 1, seglen = 1; i < n_chan; i++, seglen++) {
		if (chanlist[0] == chanlist[i])
			break;	/*                           */
		if ((CR_CHAN(chanlist[i]) & 1) &&
		    (CR_AREF(chanlist[i]) == AREF_DIFF)) {
			comedi_error(dev, "Odd channel cannot be differential input!\n");
			return 0;
		}
		nowmustbechan = (CR_CHAN(chansegment[i - 1]) + 1) % s->n_chan;
		if (CR_AREF(chansegment[i - 1]) == AREF_DIFF)
			nowmustbechan = (nowmustbechan + 1) % s->n_chan;
		if (nowmustbechan != CR_CHAN(chanlist[i])) {
			printk("channel list must be continuous! chanlist[%i]=%d but must be %d or %d!\n",
			       i, CR_CHAN(chanlist[i]), nowmustbechan,
			       CR_CHAN(chanlist[0]));
			return 0;
		}
		chansegment[i] = chanlist[i]; /*                              */
	}

	for (i = 0, segpos = 0; i < n_chan; i++) {
		if (chanlist[i] != chansegment[i % seglen]) {
			printk("bad channel, reference or range number! chanlist[%i]=%d,%d,%d and not %d,%d,%d!\n",
			       i, CR_CHAN(chansegment[i]),
			       CR_RANGE(chansegment[i]),
			       CR_AREF(chansegment[i]),
			       CR_CHAN(chanlist[i % seglen]),
			       CR_RANGE(chanlist[i % seglen]),
			       CR_AREF(chansegment[i % seglen]));
			return 0;
		}
	}
	return seglen;
}

static void setup_channel_list(struct comedi_device *dev,
			       struct comedi_subdevice *s,
			       unsigned int *chanlist, unsigned int n_chan,
			       unsigned int seglen)
{
	const struct boardtype *this_board = comedi_board(dev);
	struct pci1710_private *devpriv = dev->private;
	unsigned int i, range, chanprog;

	devpriv->act_chanlist_len = seglen;
	devpriv->act_chanlist_pos = 0;

	for (i = 0; i < seglen; i++) {	/*                           */
		chanprog = muxonechan[CR_CHAN(chanlist[i])];
		outw(chanprog, dev->iobase + PCI171x_MUX); /*                */
		range = this_board->rangecode_ai[CR_RANGE(chanlist[i])];
		if (CR_AREF(chanlist[i]) == AREF_DIFF)
			range |= 0x0020;
		outw(range, dev->iobase + PCI171x_RANGE); /*             */
#ifdef PCI171x_PARANOIDCHECK
		devpriv->act_chanlist[i] =
			(CR_CHAN(chanlist[i]) << 12) & 0xf000;
#endif
	}
#ifdef PCI171x_PARANOIDCHECK
	for ( ; i < n_chan; i++) { /*                                 */
		devpriv->act_chanlist[i] =
			(CR_CHAN(chanlist[i]) << 12) & 0xf000;
	}
#endif

	devpriv->ai_et_MuxVal =
		CR_CHAN(chanlist[0]) | (CR_CHAN(chanlist[seglen - 1]) << 8);
	/*                                 */
	outw(devpriv->ai_et_MuxVal, dev->iobase + PCI171x_MUX);
}

/*
                                                                              
*/
static int pci171x_insn_read_ai(struct comedi_device *dev,
				struct comedi_subdevice *s,
				struct comedi_insn *insn, unsigned int *data)
{
	struct pci1710_private *devpriv = dev->private;
	int n, timeout;
#ifdef PCI171x_PARANOIDCHECK
	const struct boardtype *this_board = comedi_board(dev);
	unsigned int idata;
#endif

	devpriv->CntrlReg &= Control_CNT0;
	devpriv->CntrlReg |= Control_SW;	/*                       */
	outw(devpriv->CntrlReg, dev->iobase + PCI171x_CONTROL);
	outb(0, dev->iobase + PCI171x_CLRFIFO);
	outb(0, dev->iobase + PCI171x_CLRINT);

	setup_channel_list(dev, s, &insn->chanspec, 1, 1);

	for (n = 0; n < insn->n; n++) {
		outw(0, dev->iobase + PCI171x_SOFTTRG);	/*                  */
		/*            */
		timeout = 100;
		while (timeout--) {
			if (!(inw(dev->iobase + PCI171x_STATUS) & Status_FE))
				goto conv_finish;
		}
		comedi_error(dev, "A/D insn timeout");
		outb(0, dev->iobase + PCI171x_CLRFIFO);
		outb(0, dev->iobase + PCI171x_CLRINT);
		data[n] = 0;
		return -ETIME;

conv_finish:
#ifdef PCI171x_PARANOIDCHECK
		idata = inw(dev->iobase + PCI171x_AD_DATA);
		if (this_board->cardtype != TYPE_PCI1713)
			if ((idata & 0xf000) != devpriv->act_chanlist[0]) {
				comedi_error(dev, "A/D insn data droput!");
				return -ETIME;
			}
		data[n] = idata & 0x0fff;
#else
		data[n] = inw(dev->iobase + PCI171x_AD_DATA) & 0x0fff;
#endif

	}

	outb(0, dev->iobase + PCI171x_CLRFIFO);
	outb(0, dev->iobase + PCI171x_CLRINT);

	return n;
}

/*
                                                                              
*/
static int pci171x_insn_write_ao(struct comedi_device *dev,
				 struct comedi_subdevice *s,
				 struct comedi_insn *insn, unsigned int *data)
{
	struct pci1710_private *devpriv = dev->private;
	unsigned int val;
	int n, chan, range, ofs;

	chan = CR_CHAN(insn->chanspec);
	range = CR_RANGE(insn->chanspec);
	if (chan) {
		devpriv->da_ranges &= 0xfb;
		devpriv->da_ranges |= (range << 2);
		outw(devpriv->da_ranges, dev->iobase + PCI171x_DAREF);
		ofs = PCI171x_DA2;
	} else {
		devpriv->da_ranges &= 0xfe;
		devpriv->da_ranges |= range;
		outw(devpriv->da_ranges, dev->iobase + PCI171x_DAREF);
		ofs = PCI171x_DA1;
	}
	val = devpriv->ao_data[chan];

	for (n = 0; n < insn->n; n++) {
		val = data[n];
		outw(val, dev->iobase + ofs);
	}

	devpriv->ao_data[chan] = val;

	return n;

}

/*
                                                                              
*/
static int pci171x_insn_read_ao(struct comedi_device *dev,
				struct comedi_subdevice *s,
				struct comedi_insn *insn, unsigned int *data)
{
	struct pci1710_private *devpriv = dev->private;
	int n, chan;

	chan = CR_CHAN(insn->chanspec);
	for (n = 0; n < insn->n; n++)
		data[n] = devpriv->ao_data[chan];

	return n;
}

/*
                                                                              
*/
static int pci171x_insn_bits_di(struct comedi_device *dev,
				struct comedi_subdevice *s,
				struct comedi_insn *insn, unsigned int *data)
{
	data[1] = inw(dev->iobase + PCI171x_DI);

	return insn->n;
}

/*
                                                                              
*/
static int pci171x_insn_bits_do(struct comedi_device *dev,
				struct comedi_subdevice *s,
				struct comedi_insn *insn, unsigned int *data)
{
	if (data[0]) {
		s->state &= ~data[0];
		s->state |= (data[0] & data[1]);
		outw(s->state, dev->iobase + PCI171x_DO);
	}
	data[1] = s->state;

	return insn->n;
}

/*
                                                                              
*/
static void start_pacer(struct comedi_device *dev, int mode,
			unsigned int divisor1, unsigned int divisor2)
{
	outw(0xb4, dev->iobase + PCI171x_CNTCTRL);
	outw(0x74, dev->iobase + PCI171x_CNTCTRL);

	if (mode == 1) {
		outw(divisor2 & 0xff, dev->iobase + PCI171x_CNT2);
		outw((divisor2 >> 8) & 0xff, dev->iobase + PCI171x_CNT2);
		outw(divisor1 & 0xff, dev->iobase + PCI171x_CNT1);
		outw((divisor1 >> 8) & 0xff, dev->iobase + PCI171x_CNT1);
	}
}

/*
                                                                              
*/
static int pci171x_insn_counter_read(struct comedi_device *dev,
				     struct comedi_subdevice *s,
				     struct comedi_insn *insn,
				     unsigned int *data)
{
	unsigned int msb, lsb, ccntrl;
	int i;

	ccntrl = 0xD2;		/*            */
	for (i = 0; i < insn->n; i++) {
		outw(ccntrl, dev->iobase + PCI171x_CNTCTRL);

		lsb = inw(dev->iobase + PCI171x_CNT0) & 0xFF;
		msb = inw(dev->iobase + PCI171x_CNT0) & 0xFF;

		data[0] = lsb | (msb << 8);
	}

	return insn->n;
}

/*
                                                                              
*/
static int pci171x_insn_counter_write(struct comedi_device *dev,
				      struct comedi_subdevice *s,
				      struct comedi_insn *insn,
				      unsigned int *data)
{
	struct pci1710_private *devpriv = dev->private;
	uint msb, lsb, ccntrl, status;

	lsb = data[0] & 0x00FF;
	msb = (data[0] & 0xFF00) >> 8;

	/*                     */
	outw(lsb, dev->iobase + PCI171x_CNT0);
	outw(msb, dev->iobase + PCI171x_CNT0);

	if (devpriv->cnt0_write_wait) {
		/*                                     */
		ccntrl = 0xE2;
		do {
			outw(ccntrl, dev->iobase + PCI171x_CNTCTRL);
			status = inw(dev->iobase + PCI171x_CNT0) & 0xFF;
		} while (status & 0x40);
	}

	return insn->n;
}

/*
                                                                              
*/
static int pci171x_insn_counter_config(struct comedi_device *dev,
				       struct comedi_subdevice *s,
				       struct comedi_insn *insn,
				       unsigned int *data)
{
#ifdef unused
	/*                                                       */
	struct pci1710_private *devpriv = dev->private;
	uint ccntrl = 0;

	devpriv->cnt0_write_wait = data[0] & 0x20;

	/*                             */
	if (!(data[0] & 0x10)) {	/*          */
		devpriv->CntrlReg &= ~Control_CNT0;
	} else {
		devpriv->CntrlReg |= Control_CNT0;
	}
	outw(devpriv->CntrlReg, dev->iobase + PCI171x_CONTROL);

	if (data[0] & 0x01)
		ccntrl |= Counter_M0;
	if (data[0] & 0x02)
		ccntrl |= Counter_M1;
	if (data[0] & 0x04)
		ccntrl |= Counter_M2;
	if (data[0] & 0x08)
		ccntrl |= Counter_BCD;
	ccntrl |= Counter_RW0;	/*                     */
	ccntrl |= Counter_RW1;
	outw(ccntrl, dev->iobase + PCI171x_CNTCTRL);
#endif

	return 1;
}

/*
                                                                              
*/
static int pci1720_insn_write_ao(struct comedi_device *dev,
				 struct comedi_subdevice *s,
				 struct comedi_insn *insn, unsigned int *data)
{
	struct pci1710_private *devpriv = dev->private;
	unsigned int val;
	int n, rangereg, chan;

	chan = CR_CHAN(insn->chanspec);
	rangereg = devpriv->da_ranges & (~(0x03 << (chan << 1)));
	rangereg |= (CR_RANGE(insn->chanspec) << (chan << 1));
	if (rangereg != devpriv->da_ranges) {
		outb(rangereg, dev->iobase + PCI1720_RANGE);
		devpriv->da_ranges = rangereg;
	}
	val = devpriv->ao_data[chan];

	for (n = 0; n < insn->n; n++) {
		val = data[n];
		outw(val, dev->iobase + PCI1720_DA0 + (chan << 1));
		outb(0, dev->iobase + PCI1720_SYNCOUT);	/*                 */
	}

	devpriv->ao_data[chan] = val;

	return n;
}

/*
                                                                              
*/
static int pci171x_ai_cancel(struct comedi_device *dev,
			     struct comedi_subdevice *s)
{
	const struct boardtype *this_board = comedi_board(dev);
	struct pci1710_private *devpriv = dev->private;

	switch (this_board->cardtype) {
	default:
		devpriv->CntrlReg &= Control_CNT0;
		devpriv->CntrlReg |= Control_SW;

		outw(devpriv->CntrlReg, dev->iobase + PCI171x_CONTROL);	/*                       */
		start_pacer(dev, -1, 0, 0);
		outb(0, dev->iobase + PCI171x_CLRFIFO);
		outb(0, dev->iobase + PCI171x_CLRINT);
		break;
	}

	devpriv->ai_do = 0;
	devpriv->ai_act_scan = 0;
	s->async->cur_chan = 0;
	devpriv->ai_buf_ptr = 0;
	devpriv->neverending_ai = 0;

	return 0;
}

/*
                                                                              
*/
static void interrupt_pci1710_every_sample(void *d)
{
	struct comedi_device *dev = d;
	struct pci1710_private *devpriv = dev->private;
	struct comedi_subdevice *s = &dev->subdevices[0];
	int m;
#ifdef PCI171x_PARANOIDCHECK
	const struct boardtype *this_board = comedi_board(dev);
	short sampl;
#endif

	m = inw(dev->iobase + PCI171x_STATUS);
	if (m & Status_FE) {
		printk("comedi%d: A/D FIFO empty (%4x)\n", dev->minor, m);
		pci171x_ai_cancel(dev, s);
		s->async->events |= COMEDI_CB_EOA | COMEDI_CB_ERROR;
		comedi_event(dev, s);
		return;
	}
	if (m & Status_FF) {
		printk
		    ("comedi%d: A/D FIFO Full status (Fatal Error!) (%4x)\n",
		     dev->minor, m);
		pci171x_ai_cancel(dev, s);
		s->async->events |= COMEDI_CB_EOA | COMEDI_CB_ERROR;
		comedi_event(dev, s);
		return;
	}

	outb(0, dev->iobase + PCI171x_CLRINT);	/*                        */

	for (; !(inw(dev->iobase + PCI171x_STATUS) & Status_FE);) {
#ifdef PCI171x_PARANOIDCHECK
		sampl = inw(dev->iobase + PCI171x_AD_DATA);
		if (this_board->cardtype != TYPE_PCI1713)
			if ((sampl & 0xf000) !=
			    devpriv->act_chanlist[s->async->cur_chan]) {
				printk
				    ("comedi: A/D data dropout: received data from channel %d, expected %d!\n",
				     (sampl & 0xf000) >> 12,
				     (devpriv->
				      act_chanlist[s->
						   async->cur_chan] & 0xf000) >>
				     12);
				pci171x_ai_cancel(dev, s);
				s->async->events |=
				    COMEDI_CB_EOA | COMEDI_CB_ERROR;
				comedi_event(dev, s);
				return;
			}
		comedi_buf_put(s->async, sampl & 0x0fff);
#else
		comedi_buf_put(s->async,
			       inw(dev->iobase + PCI171x_AD_DATA) & 0x0fff);
#endif
		++s->async->cur_chan;

		if (s->async->cur_chan >= devpriv->ai_n_chan)
			s->async->cur_chan = 0;


		if (s->async->cur_chan == 0) {	/*                */
			devpriv->ai_act_scan++;
			if ((!devpriv->neverending_ai) &&
			    (devpriv->ai_act_scan >= devpriv->ai_scans)) {
				/*                   */
				pci171x_ai_cancel(dev, s);
				s->async->events |= COMEDI_CB_EOA;
				comedi_event(dev, s);
				return;
			}
		}
	}

	outb(0, dev->iobase + PCI171x_CLRINT);	/*                        */

	comedi_event(dev, s);
}

/*
                                                                              
*/
static int move_block_from_fifo(struct comedi_device *dev,
				struct comedi_subdevice *s, int n, int turn)
{
	struct pci1710_private *devpriv = dev->private;
	int i, j;
#ifdef PCI171x_PARANOIDCHECK
	const struct boardtype *this_board = comedi_board(dev);
	int sampl;
#endif

	j = s->async->cur_chan;
	for (i = 0; i < n; i++) {
#ifdef PCI171x_PARANOIDCHECK
		sampl = inw(dev->iobase + PCI171x_AD_DATA);
		if (this_board->cardtype != TYPE_PCI1713)
			if ((sampl & 0xf000) != devpriv->act_chanlist[j]) {
				printk
				    ("comedi%d: A/D  FIFO data dropout: received data from channel %d, expected %d! (%d/%d/%d/%d/%d/%4x)\n",
				     dev->minor, (sampl & 0xf000) >> 12,
				     (devpriv->act_chanlist[j] & 0xf000) >> 12,
				     i, j, devpriv->ai_act_scan, n, turn,
				     sampl);
				pci171x_ai_cancel(dev, s);
				s->async->events |=
				    COMEDI_CB_EOA | COMEDI_CB_ERROR;
				comedi_event(dev, s);
				return 1;
			}
		comedi_buf_put(s->async, sampl & 0x0fff);
#else
		comedi_buf_put(s->async,
			       inw(dev->iobase + PCI171x_AD_DATA) & 0x0fff);
#endif
		j++;
		if (j >= devpriv->ai_n_chan) {
			j = 0;
			devpriv->ai_act_scan++;
		}
	}
	s->async->cur_chan = j;
	return 0;
}

/*
                                                                              
*/
static void interrupt_pci1710_half_fifo(void *d)
{
	struct comedi_device *dev = d;
	const struct boardtype *this_board = comedi_board(dev);
	struct pci1710_private *devpriv = dev->private;
	struct comedi_subdevice *s = &dev->subdevices[0];
	int m, samplesinbuf;

	m = inw(dev->iobase + PCI171x_STATUS);
	if (!(m & Status_FH)) {
		printk("comedi%d: A/D FIFO not half full! (%4x)\n",
		       dev->minor, m);
		pci171x_ai_cancel(dev, s);
		s->async->events |= COMEDI_CB_EOA | COMEDI_CB_ERROR;
		comedi_event(dev, s);
		return;
	}
	if (m & Status_FF) {
		printk
		    ("comedi%d: A/D FIFO Full status (Fatal Error!) (%4x)\n",
		     dev->minor, m);
		pci171x_ai_cancel(dev, s);
		s->async->events |= COMEDI_CB_EOA | COMEDI_CB_ERROR;
		comedi_event(dev, s);
		return;
	}

	samplesinbuf = this_board->fifo_half_size;
	if (samplesinbuf * sizeof(short) >= devpriv->ai_data_len) {
		m = devpriv->ai_data_len / sizeof(short);
		if (move_block_from_fifo(dev, s, m, 0))
			return;
		samplesinbuf -= m;
	}

	if (samplesinbuf) {
		if (move_block_from_fifo(dev, s, samplesinbuf, 1))
			return;
	}

	if (!devpriv->neverending_ai)
		if (devpriv->ai_act_scan >= devpriv->ai_scans) { /*         
                    */
			pci171x_ai_cancel(dev, s);
			s->async->events |= COMEDI_CB_EOA;
			comedi_event(dev, s);
			return;
		}
	outb(0, dev->iobase + PCI171x_CLRINT);	/*                        */

	comedi_event(dev, s);
}

/*
                                                                              
*/
static irqreturn_t interrupt_service_pci1710(int irq, void *d)
{
	struct comedi_device *dev = d;
	struct pci1710_private *devpriv = dev->private;

	if (!dev->attached)	/*                      */
		return IRQ_NONE;	/*           */
	/*                                    */
	if (!(inw(dev->iobase + PCI171x_STATUS) & Status_IRQ))
		return IRQ_NONE;	/*           */

	if (devpriv->ai_et) {	/*                                            */
		devpriv->ai_et = 0;
		devpriv->CntrlReg &= Control_CNT0;
		devpriv->CntrlReg |= Control_SW; /*                      */
		outw(devpriv->CntrlReg, dev->iobase + PCI171x_CONTROL);
		devpriv->CntrlReg = devpriv->ai_et_CntrlReg;
		outb(0, dev->iobase + PCI171x_CLRFIFO);
		outb(0, dev->iobase + PCI171x_CLRINT);
		outw(devpriv->ai_et_MuxVal, dev->iobase + PCI171x_MUX);
		outw(devpriv->CntrlReg, dev->iobase + PCI171x_CONTROL);
		/*              */
		start_pacer(dev, 1, devpriv->ai_et_div1, devpriv->ai_et_div2);
		return IRQ_HANDLED;
	}
	if (devpriv->ai_eos) {	/*                                    */
		interrupt_pci1710_every_sample(d);
	} else {
		interrupt_pci1710_half_fifo(d);
	}
	return IRQ_HANDLED;
}

/*
                                                                              
*/
static int pci171x_ai_docmd_and_mode(int mode, struct comedi_device *dev,
				     struct comedi_subdevice *s)
{
	const struct boardtype *this_board = comedi_board(dev);
	struct pci1710_private *devpriv = dev->private;
	unsigned int divisor1 = 0, divisor2 = 0;
	unsigned int seglen;

	start_pacer(dev, -1, 0, 0);	/*             */

	seglen = check_channel_list(dev, s, devpriv->ai_chanlist,
				    devpriv->ai_n_chan);
	if (seglen < 1)
		return -EINVAL;
	setup_channel_list(dev, s, devpriv->ai_chanlist,
			   devpriv->ai_n_chan, seglen);

	outb(0, dev->iobase + PCI171x_CLRFIFO);
	outb(0, dev->iobase + PCI171x_CLRINT);

	devpriv->ai_do = mode;

	devpriv->ai_act_scan = 0;
	s->async->cur_chan = 0;
	devpriv->ai_buf_ptr = 0;
	devpriv->neverending_ai = 0;

	devpriv->CntrlReg &= Control_CNT0;
	/*                                                        */
	if ((devpriv->ai_flags & TRIG_WAKE_EOS)) {
		devpriv->ai_eos = 1;
	} else {
		devpriv->CntrlReg |= Control_ONEFH;
		devpriv->ai_eos = 0;
	}

	if ((devpriv->ai_scans == 0) || (devpriv->ai_scans == -1))
		devpriv->neverending_ai = 1;
	/*                             */
	else
		devpriv->neverending_ai = 0;

	switch (mode) {
	case 1:
	case 2:
		if (devpriv->ai_timer1 < this_board->ai_ns_min)
			devpriv->ai_timer1 = this_board->ai_ns_min;
		devpriv->CntrlReg |= Control_PACER | Control_IRQEN;
		if (mode == 2) {
			devpriv->ai_et_CntrlReg = devpriv->CntrlReg;
			devpriv->CntrlReg &=
			    ~(Control_PACER | Control_ONEFH | Control_GATE);
			devpriv->CntrlReg |= Control_EXT;
			devpriv->ai_et = 1;
		} else {
			devpriv->ai_et = 0;
		}
		i8253_cascade_ns_to_timer(devpriv->i8254_osc_base, &divisor1,
					  &divisor2, &devpriv->ai_timer1,
					  devpriv->ai_flags & TRIG_ROUND_MASK);
		outw(devpriv->CntrlReg, dev->iobase + PCI171x_CONTROL);
		if (mode != 2) {
			/*              */
			start_pacer(dev, mode, divisor1, divisor2);
		} else {
			devpriv->ai_et_div1 = divisor1;
			devpriv->ai_et_div2 = divisor2;
		}
		break;
	case 3:
		devpriv->CntrlReg |= Control_EXT | Control_IRQEN;
		outw(devpriv->CntrlReg, dev->iobase + PCI171x_CONTROL);
		break;
	}

	return 0;
}

/*
                                                                              
*/
static int pci171x_ai_cmdtest(struct comedi_device *dev,
			      struct comedi_subdevice *s,
			      struct comedi_cmd *cmd)
{
	const struct boardtype *this_board = comedi_board(dev);
	struct pci1710_private *devpriv = dev->private;
	int err = 0;
	int tmp;
	unsigned int divisor1 = 0, divisor2 = 0;

	/*                                                */

	err |= cfc_check_trigger_src(&cmd->start_src, TRIG_NOW | TRIG_EXT);
	err |= cfc_check_trigger_src(&cmd->scan_begin_src, TRIG_FOLLOW);
	err |= cfc_check_trigger_src(&cmd->convert_src, TRIG_TIMER | TRIG_EXT);
	err |= cfc_check_trigger_src(&cmd->scan_end_src, TRIG_COUNT);
	err |= cfc_check_trigger_src(&cmd->stop_src, TRIG_COUNT | TRIG_NONE);

	if (err)
		return 1;

	/*                                               */

	err |= cfc_check_trigger_is_unique(cmd->start_src);
	err |= cfc_check_trigger_is_unique(cmd->convert_src);
	err |= cfc_check_trigger_is_unique(cmd->stop_src);

	/*                                  */

	if (err)
		return 2;

	/*                                                */

	err |= cfc_check_trigger_arg_is(&cmd->start_arg, 0);
	err |= cfc_check_trigger_arg_is(&cmd->scan_begin_arg, 0);

	if (cmd->convert_src == TRIG_TIMER)
		err |= cfc_check_trigger_arg_min(&cmd->convert_arg,
						 this_board->ai_ns_min);
	else	/*             */
		err |= cfc_check_trigger_arg_is(&cmd->convert_arg, 0);

	err |= cfc_check_trigger_arg_is(&cmd->scan_end_arg, cmd->chanlist_len);

	if (cmd->stop_src == TRIG_COUNT)
		err |= cfc_check_trigger_arg_min(&cmd->stop_arg, 1);
	else	/*           */
		err |= cfc_check_trigger_arg_is(&cmd->stop_arg, 0);

	if (err)
		return 3;

	/*                              */

	if (cmd->convert_src == TRIG_TIMER) {
		tmp = cmd->convert_arg;
		i8253_cascade_ns_to_timer(devpriv->i8254_osc_base, &divisor1,
					  &divisor2, &cmd->convert_arg,
					  cmd->flags & TRIG_ROUND_MASK);
		if (cmd->convert_arg < this_board->ai_ns_min)
			cmd->convert_arg = this_board->ai_ns_min;
		if (tmp != cmd->convert_arg)
			err++;
	}

	if (err)
		return 4;

	/*                                                        */

	if (cmd->chanlist) {
		if (!check_channel_list(dev, s, cmd->chanlist,
					cmd->chanlist_len))
			return 5;	/*                          */
	}

	return 0;
}

/*
                                                                              
*/
static int pci171x_ai_cmd(struct comedi_device *dev, struct comedi_subdevice *s)
{
	struct pci1710_private *devpriv = dev->private;
	struct comedi_cmd *cmd = &s->async->cmd;

	devpriv->ai_n_chan = cmd->chanlist_len;
	devpriv->ai_chanlist = cmd->chanlist;
	devpriv->ai_flags = cmd->flags;
	devpriv->ai_data_len = s->async->prealloc_bufsz;
	devpriv->ai_data = s->async->prealloc_buf;
	devpriv->ai_timer1 = 0;
	devpriv->ai_timer2 = 0;

	if (cmd->stop_src == TRIG_COUNT)
		devpriv->ai_scans = cmd->stop_arg;
	else
		devpriv->ai_scans = 0;


	if (cmd->scan_begin_src == TRIG_FOLLOW) {	/*               */
		if (cmd->convert_src == TRIG_TIMER) {	/*               */
			devpriv->ai_timer1 = cmd->convert_arg;
			return pci171x_ai_docmd_and_mode(cmd->start_src ==
							 TRIG_EXT ? 2 : 1, dev,
							 s);
		}
		if (cmd->convert_src == TRIG_EXT) {	/*         */
			return pci171x_ai_docmd_and_mode(3, dev, s);
		}
	}

	return -1;
}

/*
                                                                              
*/
static int pci171x_reset(struct comedi_device *dev)
{
	const struct boardtype *this_board = comedi_board(dev);
	struct pci1710_private *devpriv = dev->private;

	outw(0x30, dev->iobase + PCI171x_CNTCTRL);
	devpriv->CntrlReg = Control_SW | Control_CNT0;	/*                                  */
	outw(devpriv->CntrlReg, dev->iobase + PCI171x_CONTROL);	/*                       */
	outb(0, dev->iobase + PCI171x_CLRFIFO);	/*             */
	outb(0, dev->iobase + PCI171x_CLRINT);	/*                    */
	start_pacer(dev, -1, 0, 0);	/*            */
	devpriv->da_ranges = 0;
	if (this_board->n_aochan) {
		outb(devpriv->da_ranges, dev->iobase + PCI171x_DAREF);	/*                    */
		outw(0, dev->iobase + PCI171x_DA1);	/*                       */
		devpriv->ao_data[0] = 0x0000;
		if (this_board->n_aochan > 1) {
			outw(0, dev->iobase + PCI171x_DA2);
			devpriv->ao_data[1] = 0x0000;
		}
	}
	outw(0, dev->iobase + PCI171x_DO);	/*                       */
	outb(0, dev->iobase + PCI171x_CLRFIFO);	/*             */
	outb(0, dev->iobase + PCI171x_CLRINT);	/*                    */

	return 0;
}

/*
                                                                              
*/
static int pci1720_reset(struct comedi_device *dev)
{
	struct pci1710_private *devpriv = dev->private;

	outb(Syncont_SC0, dev->iobase + PCI1720_SYNCONT);	/*                              */
	devpriv->da_ranges = 0xAA;
	outb(devpriv->da_ranges, dev->iobase + PCI1720_RANGE);	/*                          */
	outw(0x0800, dev->iobase + PCI1720_DA0);	/*                    */
	outw(0x0800, dev->iobase + PCI1720_DA1);
	outw(0x0800, dev->iobase + PCI1720_DA2);
	outw(0x0800, dev->iobase + PCI1720_DA3);
	outb(0, dev->iobase + PCI1720_SYNCOUT);	/*                 */
	devpriv->ao_data[0] = 0x0800;
	devpriv->ao_data[1] = 0x0800;
	devpriv->ao_data[2] = 0x0800;
	devpriv->ao_data[3] = 0x0800;
	return 0;
}

/*
                                                                              
*/
static int pci1710_reset(struct comedi_device *dev)
{
	const struct boardtype *this_board = comedi_board(dev);

	switch (this_board->cardtype) {
	case TYPE_PCI1720:
		return pci1720_reset(dev);
	default:
		return pci171x_reset(dev);
	}
}

static int pci1710_auto_attach(struct comedi_device *dev,
			       unsigned long context)
{
	struct pci_dev *pcidev = comedi_to_pci_dev(dev);
	const struct boardtype *this_board = NULL;
	struct pci1710_private *devpriv;
	struct comedi_subdevice *s;
	int ret, subdev, n_subdevices;

	if (context < ARRAY_SIZE(boardtypes))
		this_board = &boardtypes[context];
	if (!this_board)
		return -ENODEV;
	dev->board_ptr = this_board;
	dev->board_name = this_board->name;

	devpriv = kzalloc(sizeof(*devpriv), GFP_KERNEL);
	if (!devpriv)
		return -ENOMEM;
	dev->private = devpriv;

	ret = comedi_pci_enable(dev);
	if (ret)
		return ret;
	dev->iobase = pci_resource_start(pcidev, 2);

	n_subdevices = 0;
	if (this_board->n_aichan)
		n_subdevices++;
	if (this_board->n_aochan)
		n_subdevices++;
	if (this_board->n_dichan)
		n_subdevices++;
	if (this_board->n_dochan)
		n_subdevices++;
	if (this_board->n_counter)
		n_subdevices++;

	ret = comedi_alloc_subdevices(dev, n_subdevices);
	if (ret)
		return ret;

	pci1710_reset(dev);

	if (this_board->have_irq && pcidev->irq) {
		ret = request_irq(pcidev->irq, interrupt_service_pci1710,
				  IRQF_SHARED, dev->board_name, dev);
		if (ret == 0)
			dev->irq = pcidev->irq;
	}

	subdev = 0;

	if (this_board->n_aichan) {
		s = &dev->subdevices[subdev];
		dev->read_subdev = s;
		s->type = COMEDI_SUBD_AI;
		s->subdev_flags = SDF_READABLE | SDF_COMMON | SDF_GROUND;
		if (this_board->n_aichand)
			s->subdev_flags |= SDF_DIFF;
		s->n_chan = this_board->n_aichan;
		s->maxdata = this_board->ai_maxdata;
		s->len_chanlist = this_board->n_aichan;
		s->range_table = this_board->rangelist_ai;
		s->cancel = pci171x_ai_cancel;
		s->insn_read = pci171x_insn_read_ai;
		if (dev->irq) {
			s->subdev_flags |= SDF_CMD_READ;
			s->do_cmdtest = pci171x_ai_cmdtest;
			s->do_cmd = pci171x_ai_cmd;
		}
		devpriv->i8254_osc_base = 100;	/*              */
		subdev++;
	}

	if (this_board->n_aochan) {
		s = &dev->subdevices[subdev];
		s->type = COMEDI_SUBD_AO;
		s->subdev_flags = SDF_WRITABLE | SDF_GROUND | SDF_COMMON;
		s->n_chan = this_board->n_aochan;
		s->maxdata = this_board->ao_maxdata;
		s->len_chanlist = this_board->n_aochan;
		s->range_table = this_board->rangelist_ao;
		switch (this_board->cardtype) {
		case TYPE_PCI1720:
			s->insn_write = pci1720_insn_write_ao;
			break;
		default:
			s->insn_write = pci171x_insn_write_ao;
			break;
		}
		s->insn_read = pci171x_insn_read_ao;
		subdev++;
	}

	if (this_board->n_dichan) {
		s = &dev->subdevices[subdev];
		s->type = COMEDI_SUBD_DI;
		s->subdev_flags = SDF_READABLE | SDF_GROUND | SDF_COMMON;
		s->n_chan = this_board->n_dichan;
		s->maxdata = 1;
		s->len_chanlist = this_board->n_dichan;
		s->range_table = &range_digital;
		s->io_bits = 0;	/*                */
		s->insn_bits = pci171x_insn_bits_di;
		subdev++;
	}

	if (this_board->n_dochan) {
		s = &dev->subdevices[subdev];
		s->type = COMEDI_SUBD_DO;
		s->subdev_flags = SDF_WRITABLE | SDF_GROUND | SDF_COMMON;
		s->n_chan = this_board->n_dochan;
		s->maxdata = 1;
		s->len_chanlist = this_board->n_dochan;
		s->range_table = &range_digital;
		/*                 */
		s->io_bits = (1 << this_board->n_dochan) - 1;
		s->state = 0;
		s->insn_bits = pci171x_insn_bits_do;
		subdev++;
	}

	if (this_board->n_counter) {
		s = &dev->subdevices[subdev];
		s->type = COMEDI_SUBD_COUNTER;
		s->subdev_flags = SDF_READABLE | SDF_WRITABLE;
		s->n_chan = this_board->n_counter;
		s->len_chanlist = this_board->n_counter;
		s->maxdata = 0xffff;
		s->range_table = &range_unknown;
		s->insn_read = pci171x_insn_counter_read;
		s->insn_write = pci171x_insn_counter_write;
		s->insn_config = pci171x_insn_counter_config;
		subdev++;
	}

	dev_info(dev->class_dev, "%s attached, irq %sabled\n",
		dev->board_name, dev->irq ? "en" : "dis");

	return 0;
}

static void pci1710_detach(struct comedi_device *dev)
{
	if (dev->iobase)
		pci1710_reset(dev);
	if (dev->irq)
		free_irq(dev->irq, dev);
	comedi_pci_disable(dev);
}

static struct comedi_driver adv_pci1710_driver = {
	.driver_name	= "adv_pci1710",
	.module		= THIS_MODULE,
	.auto_attach	= pci1710_auto_attach,
	.detach		= pci1710_detach,
};

static int adv_pci1710_pci_probe(struct pci_dev *dev,
				 const struct pci_device_id *id)
{
	return comedi_pci_auto_config(dev, &adv_pci1710_driver,
				      id->driver_data);
}

static DEFINE_PCI_DEVICE_TABLE(adv_pci1710_pci_table) = {
	{
		PCI_DEVICE_SUB(PCI_VENDOR_ID_ADVANTECH, 0x1710,
			       PCI_VENDOR_ID_PLX, PCI_DEVICE_ID_PLX_9050),
		.driver_data = BOARD_PCI1710,
	}, {
		PCI_DEVICE_SUB(PCI_VENDOR_ID_ADVANTECH, 0x1710,
			       PCI_VENDOR_ID_ADVANTECH, 0x0000),
		.driver_data = BOARD_PCI1710,
	}, {
		PCI_DEVICE_SUB(PCI_VENDOR_ID_ADVANTECH, 0x1710,
			       PCI_VENDOR_ID_ADVANTECH, 0xb100),
		.driver_data = BOARD_PCI1710,
	}, {
		PCI_DEVICE_SUB(PCI_VENDOR_ID_ADVANTECH, 0x1710,
			       PCI_VENDOR_ID_ADVANTECH, 0xb200),
		.driver_data = BOARD_PCI1710,
	}, {
		PCI_DEVICE_SUB(PCI_VENDOR_ID_ADVANTECH, 0x1710,
			       PCI_VENDOR_ID_ADVANTECH, 0xc100),
		.driver_data = BOARD_PCI1710,
	}, {
		PCI_DEVICE_SUB(PCI_VENDOR_ID_ADVANTECH, 0x1710,
			       PCI_VENDOR_ID_ADVANTECH, 0xc200),
		.driver_data = BOARD_PCI1710,
	}, {
		PCI_DEVICE_SUB(PCI_VENDOR_ID_ADVANTECH, 0x1710, 0x1000, 0xd100),
		.driver_data = BOARD_PCI1710,
	}, {
		PCI_DEVICE_SUB(PCI_VENDOR_ID_ADVANTECH, 0x1710,
			       PCI_VENDOR_ID_ADVANTECH, 0x0002),
		.driver_data = BOARD_PCI1710HG,
	}, {
		PCI_DEVICE_SUB(PCI_VENDOR_ID_ADVANTECH, 0x1710,
			       PCI_VENDOR_ID_ADVANTECH, 0xb102),
		.driver_data = BOARD_PCI1710HG,
	}, {
		PCI_DEVICE_SUB(PCI_VENDOR_ID_ADVANTECH, 0x1710,
			       PCI_VENDOR_ID_ADVANTECH, 0xb202),
		.driver_data = BOARD_PCI1710HG,
	}, {
		PCI_DEVICE_SUB(PCI_VENDOR_ID_ADVANTECH, 0x1710,
			       PCI_VENDOR_ID_ADVANTECH, 0xc102),
		.driver_data = BOARD_PCI1710HG,
	}, {
		PCI_DEVICE_SUB(PCI_VENDOR_ID_ADVANTECH, 0x1710,
			       PCI_VENDOR_ID_ADVANTECH, 0xc202),
		.driver_data = BOARD_PCI1710HG,
	}, {
		PCI_DEVICE_SUB(PCI_VENDOR_ID_ADVANTECH, 0x1710, 0x1000, 0xd102),
		.driver_data = BOARD_PCI1710HG,
	},
	{ PCI_VDEVICE(ADVANTECH, 0x1711), BOARD_PCI1711 },
	{ PCI_VDEVICE(ADVANTECH, 0x1713), BOARD_PCI1713 },
	{ PCI_VDEVICE(ADVANTECH, 0x1720), BOARD_PCI1720 },
	{ PCI_VDEVICE(ADVANTECH, 0x1731), BOARD_PCI1731 },
	{ 0 }
};
MODULE_DEVICE_TABLE(pci, adv_pci1710_pci_table);

static struct pci_driver adv_pci1710_pci_driver = {
	.name		= "adv_pci1710",
	.id_table	= adv_pci1710_pci_table,
	.probe		= adv_pci1710_pci_probe,
	.remove		= comedi_pci_auto_unconfig,
};
module_comedi_pci_driver(adv_pci1710_driver, adv_pci1710_pci_driver);

MODULE_AUTHOR("Comedi http://www.comedi.org");
MODULE_DESCRIPTION("Comedi low-level driver");
MODULE_LICENSE("GPL");
