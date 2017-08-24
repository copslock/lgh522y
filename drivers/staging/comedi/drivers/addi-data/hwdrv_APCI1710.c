/**
@verbatim

Copyright (C) 2004,2005  ADDI-DATA GmbH for the source code of this module.

	ADDI-DATA GmbH
	Dieselstrasse 3
	D-77833 Ottersweier
	Tel: +19(0)7223/9493-0
	Fax: +49(0)7223/9493-92
	http://www.addi-data.com
	info@addi-data.com

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

You should also find the complete GPL in the COPYING file accompanying this source code.

@endverbatim
*/
/*
                                                                           
                                                                            
                                                                           
                                                                           
                                                                           
                                                                           
                                                                           
                                                                           
                                                                           
                                                                           
                                                                           
                                                                           
                                                                           
                                                                           
                                                                           
                                                                           
                                                                           
                                                                           
                                                                           
                                                                           
                                                                           
*/

#define COMEDI_SUBD_TTLIO		11	/*                              */
#define COMEDI_SUBD_PWM			12	/*                         */
#define COMEDI_SUBD_SSI			13	/*                              */
#define COMEDI_SUBD_TOR			14	/*             */
#define COMEDI_SUBD_CHRONO		15	/*              */
#define COMEDI_SUBD_PULSEENCODER	16	/*                       */
#define COMEDI_SUBD_INCREMENTALCOUNTER	17	/*                     */

#define APCI1710_BOARD_NAME		"apci1710"
#define APCI1710_BOARD_DEVICE_ID	0x818F
#define APCI1710_ADDRESS_RANGE		256
#define APCI1710_CONFIG_ADDRESS_RANGE	8
#define APCI1710_INCREMENTAL_COUNTER	0x53430000UL
#define APCI1710_SSI_COUNTER		0x53490000UL
#define APCI1710_TTL_IO			0x544C0000UL
#define APCI1710_DIGITAL_IO		0x44490000UL
#define APCI1710_82X54_TIMER		0x49430000UL
#define APCI1710_CHRONOMETER		0x43480000UL
#define APCI1710_PULSE_ENCODER		0x495A0000UL
#define APCI1710_TOR_COUNTER		0x544F0000UL
#define APCI1710_PWM			0x50570000UL
#define APCI1710_ETM			0x45540000UL
#define APCI1710_CDA			0x43440000UL
#define APCI1710_DISABLE		0
#define APCI1710_ENABLE			1
#define APCI1710_SYNCHRONOUS_MODE	1
#define APCI1710_ASYNCHRONOUS_MODE	0

#include "APCI1710_Inp_cpt.c"

#include "APCI1710_Ssi.c"
#include "APCI1710_Tor.c"
#include "APCI1710_Ttl.c"
#include "APCI1710_Dig_io.c"
#include "APCI1710_82x54.c"
#include "APCI1710_Chrono.c"
#include "APCI1710_Pwm.c"
#include "APCI1710_INCCPT.c"

static const struct comedi_lrange range_apci1710_ttl = {
	4, {
		BIP_RANGE(10),
		BIP_RANGE(5),
		BIP_RANGE(2),
		BIP_RANGE(1)
	}
};

static const struct comedi_lrange range_apci1710_ssi = {
	4, {
		BIP_RANGE(10),
		BIP_RANGE(5),
		BIP_RANGE(2),
		BIP_RANGE(1)
	}
};

static const struct comedi_lrange range_apci1710_inccpt = {
	4, {
		BIP_RANGE(10),
		BIP_RANGE(5),
		BIP_RANGE(2),
		BIP_RANGE(1)
	}
};

static void i_ADDI_AttachPCI1710(struct comedi_device *dev)
{
	struct comedi_subdevice *s;
	int ret = 0;
	int n_subdevices = 9;

	ret = comedi_alloc_subdevices(dev, n_subdevices);
	if (ret)
		return;

	/*                                                     */
	s = &dev->subdevices[0];

	s->type = COMEDI_SUBD_TIMER;
	s->subdev_flags = SDF_WRITEABLE | SDF_GROUND | SDF_COMMON;
	s->n_chan = 3;
	s->maxdata = 0;
	s->len_chanlist = 3;
	s->range_table = &range_digital;
	s->insn_write = i_APCI1710_InsnWriteEnableDisableTimer;
	s->insn_read = i_APCI1710_InsnReadAllTimerValue;
	s->insn_config = i_APCI1710_InsnConfigInitTimer;
	s->insn_bits = i_APCI1710_InsnBitsTimer;

	/*                                                   */
	s = &dev->subdevices[1];

	s->type = COMEDI_SUBD_DIO;
	s->subdev_flags =
		SDF_WRITEABLE | SDF_READABLE | SDF_GROUND | SDF_COMMON;
	s->n_chan = 7;
	s->maxdata = 1;
	s->len_chanlist = 7;
	s->range_table = &range_digital;
	s->insn_config = i_APCI1710_InsnConfigDigitalIO;
	s->insn_read = i_APCI1710_InsnReadDigitalIOChlValue;
	s->insn_bits = i_APCI1710_InsnBitsDigitalIOPortOnOff;
	s->insn_write = i_APCI1710_InsnWriteDigitalIOChlOnOff;

	/*                                                      */
	s = &dev->subdevices[2];

	s->type = COMEDI_SUBD_CHRONO;
	s->subdev_flags = SDF_WRITEABLE | SDF_GROUND | SDF_COMMON;
	s->n_chan = 4;
	s->maxdata = 0;
	s->len_chanlist = 4;
	s->range_table = &range_digital;
	s->insn_write = i_APCI1710_InsnWriteEnableDisableChrono;
	s->insn_read = i_APCI1710_InsnReadChrono;
	s->insn_config = i_APCI1710_InsnConfigInitChrono;
	s->insn_bits = i_APCI1710_InsnBitsChronoDigitalIO;

	/*                                                   */
	s = &dev->subdevices[3];
	s->type = COMEDI_SUBD_PWM;
	s->subdev_flags =
		SDF_WRITEABLE | SDF_READABLE | SDF_GROUND | SDF_COMMON;
	s->n_chan = 3;
	s->maxdata = 1;
	s->len_chanlist = 3;
	s->range_table = &range_digital;
	s->io_bits = 0;		/*                */
	s->insn_config = i_APCI1710_InsnConfigPWM;
	s->insn_read = i_APCI1710_InsnReadGetPWMStatus;
	s->insn_write = i_APCI1710_InsnWritePWM;
	s->insn_bits = i_APCI1710_InsnBitsReadPWMInterrupt;

	/*                                                     */
	s = &dev->subdevices[4];
	s->type = COMEDI_SUBD_TTLIO;
	s->subdev_flags =
		SDF_WRITEABLE | SDF_READABLE | SDF_GROUND | SDF_COMMON;
	s->n_chan = 8;
	s->maxdata = 1;
	s->len_chanlist = 8;
	s->range_table = &range_apci1710_ttl;	/*                             */
	s->insn_config = i_APCI1710_InsnConfigInitTTLIO;
	s->insn_bits = i_APCI1710_InsnBitsReadTTLIO;
	s->insn_write = i_APCI1710_InsnWriteSetTTLIOChlOnOff;
	s->insn_read = i_APCI1710_InsnReadTTLIOAllPortValue;

	/*                                                   */
	s = &dev->subdevices[5];
	s->type = COMEDI_SUBD_TOR;
	s->subdev_flags =
		SDF_WRITEABLE | SDF_READABLE | SDF_GROUND | SDF_COMMON;
	s->n_chan = 8;
	s->maxdata = 1;
	s->len_chanlist = 8;
	s->range_table = &range_digital;
	s->io_bits = 0;		/*                */
	s->insn_config = i_APCI1710_InsnConfigInitTorCounter;
	s->insn_read = i_APCI1710_InsnReadGetTorCounterInitialisation;
	s->insn_write = i_APCI1710_InsnWriteEnableDisableTorCounter;
	s->insn_bits = i_APCI1710_InsnBitsGetTorCounterProgressStatusAndValue;

	/*                                                   */
	s = &dev->subdevices[6];
	s->type = COMEDI_SUBD_SSI;
	s->subdev_flags =
		SDF_WRITEABLE | SDF_READABLE | SDF_GROUND | SDF_COMMON;
	s->n_chan = 4;
	s->maxdata = 1;
	s->len_chanlist = 4;
	s->range_table = &range_apci1710_ssi;
	s->insn_config = i_APCI1710_InsnConfigInitSSI;
	s->insn_read = i_APCI1710_InsnReadSSIValue;
	s->insn_bits = i_APCI1710_InsnBitsSSIDigitalIO;

	/*                                                            */
	s = &dev->subdevices[7];
	s->type = COMEDI_SUBD_PULSEENCODER;
	s->subdev_flags =
		SDF_WRITEABLE | SDF_READABLE | SDF_GROUND | SDF_COMMON;
	s->n_chan = 4;
	s->maxdata = 1;
	s->len_chanlist = 4;
	s->range_table = &range_digital;
	s->insn_config = i_APCI1710_InsnConfigInitPulseEncoder;
	s->insn_write = i_APCI1710_InsnWriteEnableDisablePulseEncoder;
	s->insn_bits = i_APCI1710_InsnBitsReadWritePulseEncoder;
	s->insn_read = i_APCI1710_InsnReadInterruptPulseEncoder;

	/*                                                                  */
	s = &dev->subdevices[8];
	s->type = COMEDI_SUBD_INCREMENTALCOUNTER;
	s->subdev_flags =
		SDF_WRITEABLE | SDF_READABLE | SDF_GROUND | SDF_COMMON;
	s->n_chan = 500;
	s->maxdata = 1;
	s->len_chanlist = 500;
	s->range_table = &range_apci1710_inccpt;
	s->insn_config = i_APCI1710_InsnConfigINCCPT;
	s->insn_write = i_APCI1710_InsnWriteINCCPT;
	s->insn_read = i_APCI1710_InsnReadINCCPT;
	s->insn_bits = i_APCI1710_InsnBitsINCCPT;
}

static int i_APCI1710_Reset(struct comedi_device *dev)
{
	struct addi_private *devpriv = dev->private;
	int ret;
	unsigned int dw_Dummy;

	/*                               */
	/*                               */
	/*                               */
	ret = inl(devpriv->s_BoardInfos.ui_Address + 60);
	devpriv->s_BoardInfos.dw_MolduleConfiguration[0] = ret;

	ret = inl(devpriv->s_BoardInfos.ui_Address + 124);
	devpriv->s_BoardInfos.dw_MolduleConfiguration[1] = ret;

	ret = inl(devpriv->s_BoardInfos.ui_Address + 188);
	devpriv->s_BoardInfos.dw_MolduleConfiguration[2] = ret;

	ret = inl(devpriv->s_BoardInfos.ui_Address + 252);
	devpriv->s_BoardInfos.dw_MolduleConfiguration[3] = ret;

	/*                                                          */
	outl(0x83838383, devpriv->s_BoardInfos.ui_Address + 0x60);

	devpriv->s_BoardInfos.b_BoardVersion = 1;

	/*                                          */
	dw_Dummy = inl(devpriv->s_BoardInfos.ui_Address + 0x38);
	outl(dw_Dummy | 0x2000, devpriv->s_BoardInfos.ui_Address + 0x38);

	return 0;
}

/*
                                                                              
                                                                              
                                                            
                                                                              
                                                                              
                                                                              
                                                                                       
                                                                              
                                                                              
                                                                              
                                                                              
                                                                              
                                                                              
*/

static void v_APCI1710_Interrupt(int irq, void *d)
{
	struct comedi_device *dev = d;
	struct addi_private *devpriv = dev->private;
	unsigned char b_ModuleCpt = 0;
	unsigned char b_InterruptFlag = 0;
	unsigned char b_PWMCpt = 0;
	unsigned char b_TorCounterCpt = 0;
	unsigned char b_PulseIncoderCpt = 0;
	unsigned int ui_16BitValue;
	unsigned int ul_InterruptLatchReg = 0;
	unsigned int ul_LatchRegisterValue = 0;
	unsigned int ul_82X54InterruptStatus;
	unsigned int ul_StatusRegister;

	union str_ModuleInfo *ps_ModuleInfo;

	printk("APCI1710 Interrupt\n");
	for (b_ModuleCpt = 0; b_ModuleCpt < 4; b_ModuleCpt++, ps_ModuleInfo++) {

		 /*                        */
		/*                        */
		 /*                        */
		ps_ModuleInfo = &devpriv->s_ModuleInfo[b_ModuleCpt];

		 /*                     */
		/*                     */
		 /*                     */

		if ((devpriv->s_BoardInfos.
				dw_MolduleConfiguration[b_ModuleCpt] &
				0xFFFF0000UL) == APCI1710_82X54_TIMER) {

			/*                                       */
			ul_82X54InterruptStatus = inl(devpriv->s_BoardInfos.
				ui_Address + 12 + (64 * b_ModuleCpt));

		    /*                         */
			/*                         */
		    /*                         */

			if ((ul_82X54InterruptStatus & ps_ModuleInfo->
					s_82X54ModuleInfo.
					b_InterruptMask) != 0) {
				devpriv->
					s_InterruptParameters.
					s_FIFOInterruptParameters[devpriv->
					s_InterruptParameters.
					ui_Write].
					ul_OldInterruptMask =
					(ul_82X54InterruptStatus &
					ps_ModuleInfo->s_82X54ModuleInfo.
					b_InterruptMask) << 4;

				devpriv->
					s_InterruptParameters.
					s_FIFOInterruptParameters[devpriv->
					s_InterruptParameters.
					ui_Write].
					b_OldModuleMask = 1 << b_ModuleCpt;

				devpriv->
					s_InterruptParameters.
					s_FIFOInterruptParameters[devpriv->
					s_InterruptParameters.
					ui_Write].ul_OldCounterLatchValue = 0;

				devpriv->
					s_InterruptParameters.
					ul_InterruptOccur++;

		       /*                          */
				/*                          */
		       /*                          */

				devpriv->
					s_InterruptParameters.
					ui_Write = (devpriv->
					s_InterruptParameters.
					ui_Write + 1) % APCI1710_SAVE_INTERRUPT;

				b_InterruptFlag = 1;

			     /*                    */
				/*                    */
			     /*                    */
				/*                                            */
				send_sig(SIGIO, devpriv->tsk_Current, 0);

			}	/*                                            */
		}		/*              */

		 /*                         */
		/*                         */
		 /*                         */

		if ((devpriv->s_BoardInfos.
				dw_MolduleConfiguration[b_ModuleCpt] &
				0xFFFF0000UL) == APCI1710_INCREMENTAL_COUNTER) {

			ul_InterruptLatchReg = inl(devpriv->s_BoardInfos.
				ui_Address + (64 * b_ModuleCpt));

		    /*                   */
			/*                   */
		    /*                   */

			if ((ul_InterruptLatchReg & 0x22) && (ps_ModuleInfo->
					s_SiemensCounterInfo.
					s_ModeRegister.
					s_ByteModeRegister.
					b_ModeRegister2 & 0x80)) {
		       /*                                  */
				/*                                  */
		       /*                                  */

				if (ul_InterruptLatchReg & 2) {
					ul_LatchRegisterValue =
						inl(devpriv->s_BoardInfos.
						ui_Address + 4 +
						(64 * b_ModuleCpt));

					devpriv->
						s_InterruptParameters.
						s_FIFOInterruptParameters
						[devpriv->s_InterruptParameters.
						ui_Write].ul_OldInterruptMask =
						1UL;

					devpriv->
						s_InterruptParameters.
						s_FIFOInterruptParameters
						[devpriv->s_InterruptParameters.
						ui_Write].b_OldModuleMask =
						1 << b_ModuleCpt;

					devpriv->
						s_InterruptParameters.
						s_FIFOInterruptParameters
						[devpriv->s_InterruptParameters.
						ui_Write].
						ul_OldCounterLatchValue =
						ul_LatchRegisterValue;

					devpriv->
						s_InterruptParameters.
						ul_InterruptOccur++;

			  /*                          */
					/*                          */
			  /*                          */
					/*                          */
		      /*                          */

					devpriv->
						s_InterruptParameters.
						ui_Write = (devpriv->
						s_InterruptParameters.
						ui_Write +
						1) % APCI1710_SAVE_INTERRUPT;

					b_InterruptFlag = 1;

				/*                    */
					/*                    */
				/*                    */
					/*                                            */
					send_sig(SIGIO, devpriv->tsk_Current,
						0);

				}

		       /*                                   */
				/*                                   */
		       /*                                   */

				if (ul_InterruptLatchReg & 0x20) {

					ul_LatchRegisterValue =
						inl(devpriv->s_BoardInfos.
						ui_Address + 8 +
						(64 * b_ModuleCpt));

					devpriv->
						s_InterruptParameters.
						s_FIFOInterruptParameters
						[devpriv->s_InterruptParameters.
						ui_Write].ul_OldInterruptMask =
						2UL;

					devpriv->
						s_InterruptParameters.
						s_FIFOInterruptParameters
						[devpriv->s_InterruptParameters.
						ui_Write].b_OldModuleMask =
						1 << b_ModuleCpt;

					devpriv->
						s_InterruptParameters.
						s_FIFOInterruptParameters
						[devpriv->s_InterruptParameters.
						ui_Write].
						ul_OldCounterLatchValue =
						ul_LatchRegisterValue;

					devpriv->
						s_InterruptParameters.
						ul_InterruptOccur++;

			  /*                          */
					/*                          */
			  /*                          */
					/*                          */
			  /*                          */

					devpriv->
						s_InterruptParameters.
						ui_Write = (devpriv->
						s_InterruptParameters.
						ui_Write +
						1) % APCI1710_SAVE_INTERRUPT;

					b_InterruptFlag = 1;

			    /*                    */
					/*                    */
				/*                    */
					/*                                            */
					send_sig(SIGIO, devpriv->tsk_Current,
						0);

				}
			}

			ul_InterruptLatchReg = inl(devpriv->s_BoardInfos.
				ui_Address + 24 + (64 * b_ModuleCpt));

		    /*                         */
			/*                         */
		    /*                         */

			if (ul_InterruptLatchReg & 0x8) {
				ps_ModuleInfo->
					s_SiemensCounterInfo.
					s_InitFlag.b_IndexInterruptOccur = 1;

				if (ps_ModuleInfo->
					s_SiemensCounterInfo.
					s_ModeRegister.
					s_ByteModeRegister.
					b_ModeRegister2 &
					APCI1710_INDEX_AUTO_MODE) {

					outl(ps_ModuleInfo->
						s_SiemensCounterInfo.
						s_ModeRegister.
						dw_ModeRegister1_2_3_4,
						devpriv->s_BoardInfos.
						ui_Address + 20 +
						(64 * b_ModuleCpt));
				}

		       /*                           */
				/*                           */
		       /*                           */

				if ((ps_ModuleInfo->
						s_SiemensCounterInfo.
						s_ModeRegister.
						s_ByteModeRegister.
						b_ModeRegister3 &
						APCI1710_ENABLE_INDEX_INT) ==
					APCI1710_ENABLE_INDEX_INT) {
					devpriv->s_InterruptParameters.
						s_FIFOInterruptParameters
						[devpriv->s_InterruptParameters.
						ui_Write].ul_OldInterruptMask =
						4UL;

					devpriv->
						s_InterruptParameters.
						s_FIFOInterruptParameters
						[devpriv->s_InterruptParameters.
						ui_Write].b_OldModuleMask =
						1 << b_ModuleCpt;

					devpriv->
						s_InterruptParameters.
						s_FIFOInterruptParameters
						[devpriv->s_InterruptParameters.
						ui_Write].
						ul_OldCounterLatchValue =
						ul_LatchRegisterValue;

					devpriv->
						s_InterruptParameters.
						ul_InterruptOccur++;

			  /*                          */
					/*                          */
			  /*                          */
					/*                          */
			  /*                          */

					devpriv->
						s_InterruptParameters.
						ui_Write = (devpriv->
						s_InterruptParameters.
						ui_Write +
						1) % APCI1710_SAVE_INTERRUPT;

					b_InterruptFlag = 1;

				/*                    */
					/*                    */
				/*                    */
					/*                                            */
					send_sig(SIGIO, devpriv->tsk_Current,
						0);

				}
			}

		    /*                           */
			/*                           */
		    /*                           */

			if (ul_InterruptLatchReg & 0x10) {
		       /*                           */
				/*                           */
		       /*                           */

				if ((ps_ModuleInfo->
						s_SiemensCounterInfo.
						s_ModeRegister.
						s_ByteModeRegister.
						b_ModeRegister3 &
						APCI1710_ENABLE_COMPARE_INT) ==
					APCI1710_ENABLE_COMPARE_INT) {
					devpriv->s_InterruptParameters.
						s_FIFOInterruptParameters
						[devpriv->s_InterruptParameters.
						ui_Write].ul_OldInterruptMask =
						8UL;

					devpriv->
						s_InterruptParameters.
						s_FIFOInterruptParameters
						[devpriv->s_InterruptParameters.
						ui_Write].b_OldModuleMask =
						1 << b_ModuleCpt;

					devpriv->
						s_InterruptParameters.
						s_FIFOInterruptParameters
						[devpriv->s_InterruptParameters.
						ui_Write].
						ul_OldCounterLatchValue =
						ul_LatchRegisterValue;

					devpriv->
						s_InterruptParameters.
						ul_InterruptOccur++;

			  /*                          */
					/*                          */
			  /*                          */
					/*                          */
		      /*                          */

					devpriv->
						s_InterruptParameters.
						ui_Write = (devpriv->
						s_InterruptParameters.
						ui_Write +
						1) % APCI1710_SAVE_INTERRUPT;

					b_InterruptFlag = 1;

				/*                    */
					/*                    */
				/*                    */
					/*                                            */
					send_sig(SIGIO, devpriv->tsk_Current,
						0);

				}
			}

		    /*                                         */
			/*                                         */
		    /*                                         */

			if (ul_InterruptLatchReg & 0x20) {
		       /*                 */
				/*                 */
		       /*                 */

				ul_StatusRegister = inl(devpriv->s_BoardInfos.
					ui_Address + 32 + (64 * b_ModuleCpt));

		       /*                */
				/*                */
		       /*                */

				ul_LatchRegisterValue =
					inl(devpriv->s_BoardInfos.ui_Address +
					28 + (64 * b_ModuleCpt));

				switch ((ul_StatusRegister >> 1) & 3) {
				case 0:
			       /*                       */
					/*                       */
			       /*                       */

					if ((devpriv->s_ModuleInfo[b_ModuleCpt].
							s_SiemensCounterInfo.
							s_ModeRegister.
							s_ByteModeRegister.
							b_ModeRegister1 &
							APCI1710_16BIT_COUNTER)
						== APCI1710_16BIT_COUNTER) {
				  /*                                      */
						/*                                      */
				  /*                                      */

						if ((ul_LatchRegisterValue &
								0xFFFFU) != 0) {
							ui_16BitValue =
								(unsigned int)
								ul_LatchRegisterValue
								& 0xFFFFU;
							ul_LatchRegisterValue =
								(ul_LatchRegisterValue
								& 0xFFFF0000UL)
								| (0xFFFFU -
								ui_16BitValue);
						}

				  /*                                      */
						/*                                      */
				  /*                                      */

						if ((ul_LatchRegisterValue &
								0xFFFF0000UL) !=
							0) {
							ui_16BitValue =
								(unsigned int) (
								(ul_LatchRegisterValue
									>> 16) &
								0xFFFFU);
							ul_LatchRegisterValue =
								(ul_LatchRegisterValue
								& 0xFFFFUL) |
								((0xFFFFU -
									ui_16BitValue)
								<< 16);
						}
					} else {
						if (ul_LatchRegisterValue != 0) {
							ul_LatchRegisterValue =
								0xFFFFFFFFUL -
								ul_LatchRegisterValue;
						}
					}
					break;

				case 1:
			       /*                                      */
					/*                                      */
			       /*                                      */

					if ((ul_LatchRegisterValue &
							0xFFFF0000UL) != 0) {
						ui_16BitValue =
							(unsigned int) (
							(ul_LatchRegisterValue
								>> 16) &
							0xFFFFU);
						ul_LatchRegisterValue =
							(ul_LatchRegisterValue &
							0xFFFFUL) | ((0xFFFFU -
								ui_16BitValue)
							<< 16);
					}
					break;

				case 2:
			       /*                                      */
					/*                                      */
			       /*                                      */

					if ((ul_LatchRegisterValue & 0xFFFFU) !=
						0) {
						ui_16BitValue =
							(unsigned int)
							ul_LatchRegisterValue &
							0xFFFFU;
						ul_LatchRegisterValue =
							(ul_LatchRegisterValue &
							0xFFFF0000UL) | (0xFFFFU
							- ui_16BitValue);
					}
					break;
				}

				devpriv->
					s_InterruptParameters.
					s_FIFOInterruptParameters[devpriv->
					s_InterruptParameters.
					ui_Write].
					ul_OldInterruptMask = 0x10000UL;

				devpriv->
					s_InterruptParameters.
					s_FIFOInterruptParameters[devpriv->
					s_InterruptParameters.
					ui_Write].
					b_OldModuleMask = 1 << b_ModuleCpt;

				devpriv->
					s_InterruptParameters.
					s_FIFOInterruptParameters[devpriv->
					s_InterruptParameters.
					ui_Write].
					ul_OldCounterLatchValue =
					ul_LatchRegisterValue;

				devpriv->
					s_InterruptParameters.
					ul_InterruptOccur++;

		       /*                          */
				/*                          */
		       /*                          */
				/*                          */
		       /*                          */

				devpriv->
					s_InterruptParameters.
					ui_Write = (devpriv->
					s_InterruptParameters.
					ui_Write + 1) % APCI1710_SAVE_INTERRUPT;

				b_InterruptFlag = 1;

			     /*                    */
				/*                    */
			     /*                    */
				/*                                            */
				send_sig(SIGIO, devpriv->tsk_Current, 0);

			}
		}		/*                      */

		 /*             */
		/*             */
		 /*             */

		if ((devpriv->s_BoardInfos.
				dw_MolduleConfiguration[b_ModuleCpt] &
				0xFFFF0000UL) == APCI1710_CDA) {
		    /*                                        */
			/*                                        */
		    /*                                        */

			if ((devpriv->s_ModuleInfo[b_ModuleCpt].
					s_CDAModuleInfo.
					b_CDAEnable == APCI1710_ENABLE)
				&& (devpriv->s_ModuleInfo[b_ModuleCpt].
					s_CDAModuleInfo.b_FctSelection == 0)) {
		       /*                          */
				/*                          */
		       /*                          */

				ul_StatusRegister = inl(devpriv->s_BoardInfos.
					ui_Address + 16 + (64 * b_ModuleCpt));
		       /*                         */
				/*                         */
		       /*                         */

				if (ul_StatusRegister & 1) {
					devpriv->
						s_InterruptParameters.
						s_FIFOInterruptParameters
						[devpriv->s_InterruptParameters.
						ui_Write].ul_OldInterruptMask =
						0x80000UL;

					devpriv->
						s_InterruptParameters.
						s_FIFOInterruptParameters
						[devpriv->s_InterruptParameters.
						ui_Write].b_OldModuleMask =
						1 << b_ModuleCpt;

					devpriv->
						s_InterruptParameters.
						s_FIFOInterruptParameters
						[devpriv->s_InterruptParameters.
						ui_Write].
						ul_OldCounterLatchValue = 0;

					devpriv->
						s_InterruptParameters.
						ul_InterruptOccur++;

			  /*                          */
					/*                          */
			  /*                          */

					devpriv->
						s_InterruptParameters.
						ui_Write = (devpriv->
						s_InterruptParameters.
						ui_Write +
						1) % APCI1710_SAVE_INTERRUPT;

					b_InterruptFlag = 1;

				/*                    */
					/*                    */
				/*                    */

					/*                                            */
					send_sig(SIGIO, devpriv->tsk_Current,
						0);

				}	/*                             */

			}
		}		/*      */

		 /*                     */
		/*                     */
		 /*                     */

		if ((devpriv->s_BoardInfos.
				dw_MolduleConfiguration[b_ModuleCpt] &
				0xFFFF0000UL) == APCI1710_PWM) {
			for (b_PWMCpt = 0; b_PWMCpt < 2; b_PWMCpt++) {
		       /*                                   */
				/*                                   */
		       /*                                   */

				if (devpriv->
					s_ModuleInfo[b_ModuleCpt].
					s_PWMModuleInfo.
					s_PWMInfo[b_PWMCpt].
					b_InterruptEnable == APCI1710_ENABLE) {
			  /*                           */
					/*                           */
			  /*                           */

					ul_StatusRegister =
						inl(devpriv->s_BoardInfos.
						ui_Address + 16 +
						(20 * b_PWMCpt) +
						(64 * b_ModuleCpt));

			  /*                         */
					/*                         */
			  /*                         */

					if (ul_StatusRegister & 0x1) {
						devpriv->
							s_InterruptParameters.
							s_FIFOInterruptParameters
							[devpriv->
							s_InterruptParameters.
							ui_Write].
							ul_OldInterruptMask =
							0x4000UL << b_PWMCpt;

						devpriv->
							s_InterruptParameters.
							s_FIFOInterruptParameters
							[devpriv->
							s_InterruptParameters.
							ui_Write].
							b_OldModuleMask =
							1 << b_ModuleCpt;

						devpriv->
							s_InterruptParameters.
							ul_InterruptOccur++;

			     /*                          */
						/*                          */
			     /*                          */

						devpriv->
							s_InterruptParameters.
							ui_Write = (devpriv->
							s_InterruptParameters.
							ui_Write +
							1) %
							APCI1710_SAVE_INTERRUPT;

						b_InterruptFlag = 1;

				   /*                    */
						/*                    */
				   /*                    */
						/*                                            */
						send_sig(SIGIO,
							devpriv->tsk_Current,
							0);

					}	/*                               */
				}	/*                       */
			}	/*                                                 */
		}		/*              */

		 /*                     */
		/*                     */
		 /*                     */

		if ((devpriv->s_BoardInfos.
				dw_MolduleConfiguration[b_ModuleCpt] &
				0xFFFF0000UL) == APCI1710_TOR_COUNTER) {
			for (b_TorCounterCpt = 0; b_TorCounterCpt < 2;
				b_TorCounterCpt++) {
		       /*                                   */
				/*                                   */
		       /*                                   */

				if (devpriv->
					s_ModuleInfo[b_ModuleCpt].
					s_TorCounterModuleInfo.
					s_TorCounterInfo[b_TorCounterCpt].
					b_InterruptEnable == APCI1710_ENABLE) {
			  /*                           */
					/*                           */
			  /*                           */

					ul_StatusRegister =
						inl(devpriv->s_BoardInfos.
						ui_Address + 12 +
						(16 * b_TorCounterCpt) +
						(64 * b_ModuleCpt));

			  /*                         */
					/*                         */
			  /*                         */

					if (ul_StatusRegister & 0x1) {
			     /*                            */
						/*                            */
			     /*                            */

						ul_LatchRegisterValue =
							inl(devpriv->
							s_BoardInfos.
							ui_Address + 0 +
							(16 * b_TorCounterCpt) +
							(64 * b_ModuleCpt));

						devpriv->
							s_InterruptParameters.
							s_FIFOInterruptParameters
							[devpriv->
							s_InterruptParameters.
							ui_Write].
							ul_OldInterruptMask =
							0x1000UL <<
							b_TorCounterCpt;

						devpriv->
							s_InterruptParameters.
							s_FIFOInterruptParameters
							[devpriv->
							s_InterruptParameters.
							ui_Write].
							b_OldModuleMask =
							1 << b_ModuleCpt;

						devpriv->
							s_InterruptParameters.
							s_FIFOInterruptParameters
							[devpriv->
							s_InterruptParameters.
							ui_Write].
							ul_OldCounterLatchValue
							= ul_LatchRegisterValue;

						devpriv->
							s_InterruptParameters.
							ul_InterruptOccur++;

			     /*                          */
						/*                          */
			     /*                          */

						devpriv->
							s_InterruptParameters.
							ui_Write = (devpriv->
							s_InterruptParameters.
							ui_Write +
							1) %
							APCI1710_SAVE_INTERRUPT;

						b_InterruptFlag = 1;

				   /*                    */
						/*                    */
				   /*                    */

						/*                                            */
						send_sig(SIGIO,
							devpriv->tsk_Current,
							0);
					}	/*                               */
				}	/*                       */
			}	/*                                                                      */
		}		/*              */

		 /*                     */
		/*                     */
		 /*                     */

		if ((devpriv->s_BoardInfos.
				dw_MolduleConfiguration[b_ModuleCpt] &
				0xFFFF0000UL) == APCI1710_CHRONOMETER) {

			/*                                        */
		    /*                           */
			/*                           */
		    /*                           */

			ul_InterruptLatchReg = inl(devpriv->s_BoardInfos.
				ui_Address + 12 + (64 * b_ModuleCpt));

		    /*                         */
			/*                         */
		    /*                         */

			if ((ul_InterruptLatchReg & 0x8) == 0x8) {
		       /*                          */
				/*                          */
		       /*                          */

				outl(0, devpriv->s_BoardInfos.
					ui_Address + 32 + (64 * b_ModuleCpt));

		       /*                         */
				/*                         */
		       /*                         */

				if (ps_ModuleInfo->
					s_ChronoModuleInfo.
					b_CycleMode == APCI1710_ENABLE) {
			  /*                  */
					/*                  */
			  /*                  */

					outl(0, devpriv->s_BoardInfos.
						ui_Address + 36 +
						(64 * b_ModuleCpt));
				}

		       /*                       */
				/*                       */
		       /*                       */

				ul_LatchRegisterValue =
					inl(devpriv->s_BoardInfos.ui_Address +
					4 + (64 * b_ModuleCpt));

		       /*                           */
				/*                           */
		       /*                           */

				if (ps_ModuleInfo->
					s_ChronoModuleInfo.b_InterruptMask) {
					devpriv->
						s_InterruptParameters.
						s_FIFOInterruptParameters
						[devpriv->s_InterruptParameters.
						ui_Write].ul_OldInterruptMask =
						0x80;

					devpriv->
						s_InterruptParameters.
						s_FIFOInterruptParameters
						[devpriv->s_InterruptParameters.
						ui_Write].b_OldModuleMask =
						1 << b_ModuleCpt;

					devpriv->
						s_InterruptParameters.
						s_FIFOInterruptParameters
						[devpriv->s_InterruptParameters.
						ui_Write].
						ul_OldCounterLatchValue =
						ul_LatchRegisterValue;

					devpriv->
						s_InterruptParameters.
						ul_InterruptOccur++;

			  /*                          */
					/*                          */
		      /*                          */

					devpriv->
						s_InterruptParameters.
						ui_Write = (devpriv->
						s_InterruptParameters.
						ui_Write +
						1) % APCI1710_SAVE_INTERRUPT;

					b_InterruptFlag = 1;

				/*                    */
					/*                    */
				/*                    */
					/*                                            */
					send_sig(SIGIO, devpriv->tsk_Current,
						0);

				}
			}
		}		/*              */

		 /*                       */
		/*                       */
		 /*                       */

		if ((devpriv->s_BoardInfos.
				dw_MolduleConfiguration[b_ModuleCpt] &
				0xFFFF0000UL) == APCI1710_PULSE_ENCODER) {
		    /*                          */
			/*                          */
		    /*                          */

			ul_StatusRegister = inl(devpriv->s_BoardInfos.
				ui_Address + 20 + (64 * b_ModuleCpt));

			if (ul_StatusRegister & 0xF) {
				for (b_PulseIncoderCpt = 0;
					b_PulseIncoderCpt < 4;
					b_PulseIncoderCpt++) {
			  /*                                   */
					/*                                   */
			  /*                                   */

					if ((ps_ModuleInfo->
							s_PulseEncoderModuleInfo.
							s_PulseEncoderInfo
							[b_PulseIncoderCpt].
							b_PulseEncoderInit == 1)
						&& (((ps_ModuleInfo->s_PulseEncoderModuleInfo.dw_SetRegister >> b_PulseIncoderCpt) & 1) == 1) && (((ul_StatusRegister >> (b_PulseIncoderCpt)) & 1) == 1)) {
						devpriv->s_InterruptParameters.
							s_FIFOInterruptParameters
							[devpriv->
							s_InterruptParameters.
							ui_Write].
							ul_OldInterruptMask =
							0x100UL <<
							b_PulseIncoderCpt;

						devpriv->
							s_InterruptParameters.
							s_FIFOInterruptParameters
							[devpriv->
							s_InterruptParameters.
							ui_Write].
							b_OldModuleMask =
							1 << b_ModuleCpt;

						devpriv->
							s_InterruptParameters.
							s_FIFOInterruptParameters
							[devpriv->
							s_InterruptParameters.
							ui_Write].
							ul_OldCounterLatchValue
							= ul_LatchRegisterValue;

						devpriv->
							s_InterruptParameters.
							ul_InterruptOccur++;

			     /*                          */
						/*                          */
			     /*                          */
						/*                          */
			     /*                          */

						devpriv->
							s_InterruptParameters.
							ui_Write = (devpriv->
							s_InterruptParameters.
							ui_Write +
							1) %
							APCI1710_SAVE_INTERRUPT;

						b_InterruptFlag = 1;

				   /*                    */
						/*                    */
				   /*                    */
						/*                                            */
						send_sig(SIGIO,
							devpriv->tsk_Current,
							0);

					}
				}
			}
		}		/*               */

	}
	return;

}
