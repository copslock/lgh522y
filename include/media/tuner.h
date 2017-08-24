/*
    tuner.h - definition for different tuners

    Copyright (C) 1997 Markus Schroeder (schroedm@uni-duesseldorf.de)
    minor modifications by Ralph Metzler (rjkm@thp.uni-koeln.de)

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

#ifndef _TUNER_H
#define _TUNER_H
#ifdef __KERNEL__

#include <linux/videodev2.h>

#define ADDR_UNSET (255)

#define TUNER_TEMIC_PAL			0        /*                          */
#define TUNER_PHILIPS_PAL_I		1
#define TUNER_PHILIPS_NTSC		2
#define TUNER_PHILIPS_SECAM		3	/*                                     */

#define TUNER_ABSENT			4
#define TUNER_PHILIPS_PAL		5
#define TUNER_TEMIC_NTSC		6	/*                                 */
#define TUNER_TEMIC_PAL_I		7	/*                          */

#define TUNER_TEMIC_4036FY5_NTSC	8	/*                                */
#define TUNER_ALPS_TSBH1_NTSC		9
#define TUNER_ALPS_TSBE1_PAL		10
#define TUNER_ALPS_TSBB5_PAL_I		11

#define TUNER_ALPS_TSBE5_PAL		12
#define TUNER_ALPS_TSBC5_PAL		13
#define TUNER_TEMIC_4006FH5_PAL		14	/*                                */
#define TUNER_ALPS_TSHC6_NTSC		15

#define TUNER_TEMIC_PAL_DK		16	/*                          */
#define TUNER_PHILIPS_NTSC_M		17
#define TUNER_TEMIC_4066FY5_PAL_I	18	/*                          */
#define TUNER_TEMIC_4006FN5_MULTI_PAL	19	/*                                                   */

#define TUNER_TEMIC_4009FR5_PAL		20	/*                                      */
#define TUNER_TEMIC_4039FR5_NTSC	21	/*                                      */
#define TUNER_TEMIC_4046FM5		22	/*                                                                            */
#define TUNER_PHILIPS_PAL_DK		23

#define TUNER_PHILIPS_FQ1216ME		24	/*                                            */
#define TUNER_LG_PAL_I_FM		25
#define TUNER_LG_PAL_I			26
#define TUNER_LG_NTSC_FM		27

#define TUNER_LG_PAL_FM			28
#define TUNER_LG_PAL			29
#define TUNER_TEMIC_4009FN5_MULTI_PAL_FM 30	/*                                                   */
#define TUNER_SHARP_2U5JF5540_NTSC	31

#define TUNER_Samsung_PAL_TCPM9091PD27	32
#define TUNER_MT2032			33
#define TUNER_TEMIC_4106FH5		34	/*                          */
#define TUNER_TEMIC_4012FY5		35	/*                          */

#define TUNER_TEMIC_4136FY5		36	/*                          */
#define TUNER_LG_PAL_NEW_TAPC		37
#define TUNER_PHILIPS_FM1216ME_MK3	38
#define TUNER_LG_NTSC_NEW_TAPC		39

#define TUNER_HITACHI_NTSC		40
#define TUNER_PHILIPS_PAL_MK		41
#define TUNER_PHILIPS_FCV1236D		42
#define TUNER_PHILIPS_FM1236_MK3	43

#define TUNER_PHILIPS_4IN1		44	/*                              */
/*                                                                                                   */
#define TUNER_MICROTUNE_4049FM5 	45
#define TUNER_PANASONIC_VP27		46
#define TUNER_LG_NTSC_TAPE		47

#define TUNER_TNF_8831BGFF		48
#define TUNER_MICROTUNE_4042FI5		49	/*                                                */
#define TUNER_TCL_2002N			50
#define TUNER_PHILIPS_FM1256_IH3	51

#define TUNER_THOMSON_DTT7610		52
#define TUNER_PHILIPS_FQ1286		53
#define TUNER_PHILIPS_TDA8290		54
#define TUNER_TCL_2002MB		55	/*                       */

#define TUNER_PHILIPS_FQ1216AME_MK4	56	/*                       */
#define TUNER_PHILIPS_FQ1236A_MK4	57	/*                           */
#define TUNER_YMEC_TVF_8531MF		58
#define TUNER_YMEC_TVF_5533MF		59	/*                          */

#define TUNER_THOMSON_DTT761X		60	/*                                                */
#define TUNER_TENA_9533_DI		61
#define TUNER_TEA5767			62	/*                     */
#define TUNER_PHILIPS_FMD1216ME_MK3	63

#define TUNER_LG_TDVS_H06XF		64	/*                          */
#define TUNER_YMEC_TVF66T5_B_DFF	65	/*             */
#define TUNER_LG_TALN			66
#define TUNER_PHILIPS_TD1316		67

#define TUNER_PHILIPS_TUV1236D		68	/*                 */
#define TUNER_TNF_5335MF                69	/*                 */
#define TUNER_SAMSUNG_TCPN_2121P30A     70 	/*                           */
#define TUNER_XC2028			71

#define TUNER_THOMSON_FE6600		72	/*                               */
#define TUNER_SAMSUNG_TCPG_6121P30A     73 	/*                       */
#define TUNER_TDA9887                   74      /*                                           */
#define TUNER_TEA5761			75	/*                     */
#define TUNER_XC5000			76	/*                      */
#define TUNER_TCL_MF02GIP_5N		77	/*                */
#define TUNER_PHILIPS_FMD1216MEX_MK3	78
#define TUNER_PHILIPS_FM1216MK5		79
#define TUNER_PHILIPS_FQ1216LME_MK3	80	/*                           */

#define TUNER_PARTSNIC_PTI_5NF05	81
#define TUNER_PHILIPS_CU1216L           82
#define TUNER_NXP_TDA18271		83
#define TUNER_SONY_BTF_PXN01Z		84
#define TUNER_PHILIPS_FQ1236_MK5	85	/*                            */
#define TUNER_TENA_TNF_5337		86

#define TUNER_XC4000			87	/*                      */
#define TUNER_XC5000C			88	/*                      */

#define TUNER_SONY_BTF_PG472Z		89	/*           */
#define TUNER_SONY_BTF_PK467Z		90	/*         */
#define TUNER_SONY_BTF_PB463Z		91	/*      */

/*                  */
#define TDA9887_PRESENT 		(1<<0)
#define TDA9887_PORT1_INACTIVE 		(1<<1)
#define TDA9887_PORT2_INACTIVE 		(1<<2)
#define TDA9887_QSS 			(1<<3)
#define TDA9887_INTERCARRIER 		(1<<4)
#define TDA9887_PORT1_ACTIVE 		(1<<5)
#define TDA9887_PORT2_ACTIVE 		(1<<6)
#define TDA9887_INTERCARRIER_NTSC 	(1<<7)
/*                                                          */
#define TDA9887_TOP_MASK 		(0x3f << 8)
#define TDA9887_TOP_SET 		(1 << 13)
#define TDA9887_TOP(top) 		(TDA9887_TOP_SET | (((16 + (top)) & 0x1f) << 8))

/*                */
#define TDA9887_DEEMPHASIS_MASK 	(3<<16)
#define TDA9887_DEEMPHASIS_NONE 	(1<<16)
#define TDA9887_DEEMPHASIS_50 		(2<<16)
#define TDA9887_DEEMPHASIS_75 		(3<<16)
#define TDA9887_AUTOMUTE 		(1<<18)
#define TDA9887_GATING_18		(1<<19)
#define TDA9887_GAIN_NORMAL		(1<<20)
#define TDA9887_RIF_41_3		(1<<21)  /*                        */

enum tuner_mode {
	T_RADIO		= 1 << V4L2_TUNER_RADIO,
	T_ANALOG_TV     = 1 << V4L2_TUNER_ANALOG_TV,
	/*                                                                     */
};

/*                                                                     
                                                                      
                                                                       
                  

                                                                      
                                                                      
                                  

                                                                       
                                                                     
                                                                        
                                                                       
                                    
 */

struct tuner_setup {
	unsigned short	addr; 	/*             */
	unsigned int	type;   /*            */
	unsigned int	mode_mask;  /*                     */
	void		*config;    /*                                      */
	int (*tuner_callback) (void *dev, int component, int cmd, int arg);
};

#endif /*            */

#endif /*          */
