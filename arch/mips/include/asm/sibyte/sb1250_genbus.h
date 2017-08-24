/*  *********************************************************************
    *  SB1250 Board Support Package
    *
    *  Generic Bus Constants			 File: sb1250_genbus.h
    *
    *  This module contains constants and macros useful for
    *  manipulating the SB1250's Generic Bus interface
    *
    *  SB1250 specification level:  User's manual 10/21/02
    *  BCM1280 specification level: User's Manual 11/14/03
    *
    *********************************************************************
    *
    *  Copyright 2000, 2001, 2002, 2003
    *  Broadcom Corporation. All rights reserved.
    *
    *  This program is free software; you can redistribute it and/or
    *  modify it under the terms of the GNU General Public License as
    *  published by the Free Software Foundation; either version 2 of
    *  the License, or (at your option) any later version.
    *
    *  This program is distributed in the hope that it will be useful,
    *  but WITHOUT ANY WARRANTY; without even the implied warranty of
    *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    *  GNU General Public License for more details.
    *
    *  You should have received a copy of the GNU General Public License
    *  along with this program; if not, write to the Free Software
    *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
    *  MA 02111-1307 USA
    ********************************************************************* */


#ifndef _SB1250_GENBUS_H
#define _SB1250_GENBUS_H

#include <asm/sibyte/sb1250_defs.h>

/*
                                                          
 */

#define S_IO_RDY_ACTIVE		0
#define M_IO_RDY_ACTIVE		_SB_MAKEMASK1(S_IO_RDY_ACTIVE)

#define S_IO_ENA_RDY		1
#define M_IO_ENA_RDY		_SB_MAKEMASK1(S_IO_ENA_RDY)

#define S_IO_WIDTH_SEL		2
#define M_IO_WIDTH_SEL		_SB_MAKEMASK(2, S_IO_WIDTH_SEL)
#define K_IO_WIDTH_SEL_1	0
#define K_IO_WIDTH_SEL_2	1
#if SIBYTE_HDR_FEATURE(1250, PASS2) || SIBYTE_HDR_FEATURE(112x, PASS1) \
    || SIBYTE_HDR_FEATURE_CHIP(1480)
#define K_IO_WIDTH_SEL_1L	2
#endif /*                                  */
#define K_IO_WIDTH_SEL_4	3
#define V_IO_WIDTH_SEL(x)	_SB_MAKEVALUE(x, S_IO_WIDTH_SEL)
#define G_IO_WIDTH_SEL(x)	_SB_GETVALUE(x, S_IO_WIDTH_SEL, M_IO_WIDTH_SEL)

#define S_IO_PARITY_ENA		4
#define M_IO_PARITY_ENA		_SB_MAKEMASK1(S_IO_PARITY_ENA)
#if SIBYTE_HDR_FEATURE(1250, PASS2) || SIBYTE_HDR_FEATURE(112x, PASS1) \
    || SIBYTE_HDR_FEATURE_CHIP(1480)
#define S_IO_BURST_EN		5
#define M_IO_BURST_EN		_SB_MAKEMASK1(S_IO_BURST_EN)
#endif /*                                  */
#define S_IO_PARITY_ODD		6
#define M_IO_PARITY_ODD		_SB_MAKEMASK1(S_IO_PARITY_ODD)
#define S_IO_NONMUX		7
#define M_IO_NONMUX		_SB_MAKEMASK1(S_IO_NONMUX)

#define S_IO_TIMEOUT		8
#define M_IO_TIMEOUT		_SB_MAKEMASK(8, S_IO_TIMEOUT)
#define V_IO_TIMEOUT(x)		_SB_MAKEVALUE(x, S_IO_TIMEOUT)
#define G_IO_TIMEOUT(x)		_SB_GETVALUE(x, S_IO_TIMEOUT, M_IO_TIMEOUT)

/*
                                                
 */

#define S_IO_MULT_SIZE		0
#define M_IO_MULT_SIZE		_SB_MAKEMASK(12, S_IO_MULT_SIZE)
#define V_IO_MULT_SIZE(x)	_SB_MAKEVALUE(x, S_IO_MULT_SIZE)
#define G_IO_MULT_SIZE(x)	_SB_GETVALUE(x, S_IO_MULT_SIZE, M_IO_MULT_SIZE)

#define S_IO_REGSIZE		16	 /*                                   */

/*
                                          
 */

#define S_IO_START_ADDR		0
#define M_IO_START_ADDR		_SB_MAKEMASK(14, S_IO_START_ADDR)
#define V_IO_START_ADDR(x)	_SB_MAKEVALUE(x, S_IO_START_ADDR)
#define G_IO_START_ADDR(x)	_SB_GETVALUE(x, S_IO_START_ADDR, M_IO_START_ADDR)

#define S_IO_ADDRBASE		16	 /*                                   */

#define M_IO_BLK_CACHE		_SB_MAKEMASK1(15)


/*
                                              
 */

#define S_IO_ALE_WIDTH		0
#define M_IO_ALE_WIDTH		_SB_MAKEMASK(3, S_IO_ALE_WIDTH)
#define V_IO_ALE_WIDTH(x)	_SB_MAKEVALUE(x, S_IO_ALE_WIDTH)
#define G_IO_ALE_WIDTH(x)	_SB_GETVALUE(x, S_IO_ALE_WIDTH, M_IO_ALE_WIDTH)

#if SIBYTE_HDR_FEATURE(1250, PASS2) || SIBYTE_HDR_FEATURE(112x, PASS1) \
    || SIBYTE_HDR_FEATURE_CHIP(1480)
#define M_IO_EARLY_CS		_SB_MAKEMASK1(3)
#endif /*                                  */

#define S_IO_ALE_TO_CS		4
#define M_IO_ALE_TO_CS		_SB_MAKEMASK(2, S_IO_ALE_TO_CS)
#define V_IO_ALE_TO_CS(x)	_SB_MAKEVALUE(x, S_IO_ALE_TO_CS)
#define G_IO_ALE_TO_CS(x)	_SB_GETVALUE(x, S_IO_ALE_TO_CS, M_IO_ALE_TO_CS)

#if SIBYTE_HDR_FEATURE(1250, PASS2) || SIBYTE_HDR_FEATURE(112x, PASS1) \
    || SIBYTE_HDR_FEATURE_CHIP(1480)
#define S_IO_BURST_WIDTH	   _SB_MAKE64(6)
#define M_IO_BURST_WIDTH	   _SB_MAKEMASK(2, S_IO_BURST_WIDTH)
#define V_IO_BURST_WIDTH(x)	   _SB_MAKEVALUE(x, S_IO_BURST_WIDTH)
#define G_IO_BURST_WIDTH(x)	   _SB_GETVALUE(x, S_IO_BURST_WIDTH, M_IO_BURST_WIDTH)
#endif /*                                  */

#define S_IO_CS_WIDTH		8
#define M_IO_CS_WIDTH		_SB_MAKEMASK(5, S_IO_CS_WIDTH)
#define V_IO_CS_WIDTH(x)	_SB_MAKEVALUE(x, S_IO_CS_WIDTH)
#define G_IO_CS_WIDTH(x)	_SB_GETVALUE(x, S_IO_CS_WIDTH, M_IO_CS_WIDTH)

#define S_IO_RDY_SMPLE		13
#define M_IO_RDY_SMPLE		_SB_MAKEMASK(3, S_IO_RDY_SMPLE)
#define V_IO_RDY_SMPLE(x)	_SB_MAKEVALUE(x, S_IO_RDY_SMPLE)
#define G_IO_RDY_SMPLE(x)	_SB_GETVALUE(x, S_IO_RDY_SMPLE, M_IO_RDY_SMPLE)


/*
                                              
 */

#define S_IO_ALE_TO_WRITE	0
#define M_IO_ALE_TO_WRITE	_SB_MAKEMASK(3, S_IO_ALE_TO_WRITE)
#define V_IO_ALE_TO_WRITE(x)	_SB_MAKEVALUE(x, S_IO_ALE_TO_WRITE)
#define G_IO_ALE_TO_WRITE(x)	_SB_GETVALUE(x, S_IO_ALE_TO_WRITE, M_IO_ALE_TO_WRITE)

#if SIBYTE_HDR_FEATURE(1250, PASS2) || SIBYTE_HDR_FEATURE(112x, PASS1) \
    || SIBYTE_HDR_FEATURE_CHIP(1480)
#define M_IO_RDY_SYNC		_SB_MAKEMASK1(3)
#endif /*                                  */

#define S_IO_WRITE_WIDTH	4
#define M_IO_WRITE_WIDTH	_SB_MAKEMASK(4, S_IO_WRITE_WIDTH)
#define V_IO_WRITE_WIDTH(x)	_SB_MAKEVALUE(x, S_IO_WRITE_WIDTH)
#define G_IO_WRITE_WIDTH(x)	_SB_GETVALUE(x, S_IO_WRITE_WIDTH, M_IO_WRITE_WIDTH)

#define S_IO_IDLE_CYCLE		8
#define M_IO_IDLE_CYCLE		_SB_MAKEMASK(4, S_IO_IDLE_CYCLE)
#define V_IO_IDLE_CYCLE(x)	_SB_MAKEVALUE(x, S_IO_IDLE_CYCLE)
#define G_IO_IDLE_CYCLE(x)	_SB_GETVALUE(x, S_IO_IDLE_CYCLE, M_IO_IDLE_CYCLE)

#define S_IO_OE_TO_CS		12
#define M_IO_OE_TO_CS		_SB_MAKEMASK(2, S_IO_OE_TO_CS)
#define V_IO_OE_TO_CS(x)	_SB_MAKEVALUE(x, S_IO_OE_TO_CS)
#define G_IO_OE_TO_CS(x)	_SB_GETVALUE(x, S_IO_OE_TO_CS, M_IO_OE_TO_CS)

#define S_IO_CS_TO_OE		14
#define M_IO_CS_TO_OE		_SB_MAKEMASK(2, S_IO_CS_TO_OE)
#define V_IO_CS_TO_OE(x)	_SB_MAKEVALUE(x, S_IO_CS_TO_OE)
#define G_IO_CS_TO_OE(x)	_SB_GETVALUE(x, S_IO_CS_TO_OE, M_IO_CS_TO_OE)

/*
                                                     
 */

#define M_IO_CS_ERR_INT		_SB_MAKEMASK(0, 8)
#define M_IO_CS0_ERR_INT	_SB_MAKEMASK1(0)
#define M_IO_CS1_ERR_INT	_SB_MAKEMASK1(1)
#define M_IO_CS2_ERR_INT	_SB_MAKEMASK1(2)
#define M_IO_CS3_ERR_INT	_SB_MAKEMASK1(3)
#define M_IO_CS4_ERR_INT	_SB_MAKEMASK1(4)
#define M_IO_CS5_ERR_INT	_SB_MAKEMASK1(5)
#define M_IO_CS6_ERR_INT	_SB_MAKEMASK1(6)
#define M_IO_CS7_ERR_INT	_SB_MAKEMASK1(7)

#define M_IO_RD_PAR_INT		_SB_MAKEMASK1(9)
#define M_IO_TIMEOUT_INT	_SB_MAKEMASK1(10)
#define M_IO_ILL_ADDR_INT	_SB_MAKEMASK1(11)
#define M_IO_MULT_CS_INT	_SB_MAKEMASK1(12)
#if SIBYTE_HDR_FEATURE(1250, PASS2) || SIBYTE_HDR_FEATURE(112x, PASS1) || SIBYTE_HDR_FEATURE_CHIP(1480)
#define M_IO_COH_ERR		_SB_MAKEMASK1(14)
#endif /*                                  */


/*
                                                            
 */

#define S_IO_SLEW0		0
#define M_IO_SLEW0		_SB_MAKEMASK(2, S_IO_SLEW0)
#define V_IO_SLEW0(x)		_SB_MAKEVALUE(x, S_IO_SLEW0)
#define G_IO_SLEW0(x)		_SB_GETVALUE(x, S_IO_SLEW0, M_IO_SLEW0)

#define S_IO_DRV_A		2
#define M_IO_DRV_A		_SB_MAKEMASK(2, S_IO_DRV_A)
#define V_IO_DRV_A(x)		_SB_MAKEVALUE(x, S_IO_DRV_A)
#define G_IO_DRV_A(x)		_SB_GETVALUE(x, S_IO_DRV_A, M_IO_DRV_A)

#define S_IO_DRV_B		6
#define M_IO_DRV_B		_SB_MAKEMASK(2, S_IO_DRV_B)
#define V_IO_DRV_B(x)		_SB_MAKEVALUE(x, S_IO_DRV_B)
#define G_IO_DRV_B(x)		_SB_GETVALUE(x, S_IO_DRV_B, M_IO_DRV_B)

#define S_IO_DRV_C		10
#define M_IO_DRV_C		_SB_MAKEMASK(2, S_IO_DRV_C)
#define V_IO_DRV_C(x)		_SB_MAKEVALUE(x, S_IO_DRV_C)
#define G_IO_DRV_C(x)		_SB_GETVALUE(x, S_IO_DRV_C, M_IO_DRV_C)

#define S_IO_DRV_D		14
#define M_IO_DRV_D		_SB_MAKEMASK(2, S_IO_DRV_D)
#define V_IO_DRV_D(x)		_SB_MAKEVALUE(x, S_IO_DRV_D)
#define G_IO_DRV_D(x)		_SB_GETVALUE(x, S_IO_DRV_D, M_IO_DRV_D)

/*
                                                            
 */

#define S_IO_DRV_E		2
#define M_IO_DRV_E		_SB_MAKEMASK(2, S_IO_DRV_E)
#define V_IO_DRV_E(x)		_SB_MAKEVALUE(x, S_IO_DRV_E)
#define G_IO_DRV_E(x)		_SB_GETVALUE(x, S_IO_DRV_E, M_IO_DRV_E)

#define S_IO_DRV_F		6
#define M_IO_DRV_F		_SB_MAKEMASK(2, S_IO_DRV_F)
#define V_IO_DRV_F(x)		_SB_MAKEVALUE(x, S_IO_DRV_F)
#define G_IO_DRV_F(x)		_SB_GETVALUE(x, S_IO_DRV_F, M_IO_DRV_F)

#define S_IO_SLEW1		8
#define M_IO_SLEW1		_SB_MAKEMASK(2, S_IO_SLEW1)
#define V_IO_SLEW1(x)		_SB_MAKEVALUE(x, S_IO_SLEW1)
#define G_IO_SLEW1(x)		_SB_GETVALUE(x, S_IO_SLEW1, M_IO_SLEW1)

#define S_IO_DRV_G		10
#define M_IO_DRV_G		_SB_MAKEMASK(2, S_IO_DRV_G)
#define V_IO_DRV_G(x)		_SB_MAKEVALUE(x, S_IO_DRV_G)
#define G_IO_DRV_G(x)		_SB_GETVALUE(x, S_IO_DRV_G, M_IO_DRV_G)

#define S_IO_SLEW2		12
#define M_IO_SLEW2		_SB_MAKEMASK(2, S_IO_SLEW2)
#define V_IO_SLEW2(x)		_SB_MAKEVALUE(x, S_IO_SLEW2)
#define G_IO_SLEW2(x)		_SB_GETVALUE(x, S_IO_SLEW2, M_IO_SLEW2)

#define S_IO_DRV_H		14
#define M_IO_DRV_H		_SB_MAKEMASK(2, S_IO_DRV_H)
#define V_IO_DRV_H(x)		_SB_MAKEVALUE(x, S_IO_DRV_H)
#define G_IO_DRV_H(x)		_SB_GETVALUE(x, S_IO_DRV_H, M_IO_DRV_H)

/*
                                                            
 */

#define S_IO_DRV_J		2
#define M_IO_DRV_J		_SB_MAKEMASK(2, S_IO_DRV_J)
#define V_IO_DRV_J(x)		_SB_MAKEVALUE(x, S_IO_DRV_J)
#define G_IO_DRV_J(x)		_SB_GETVALUE(x, S_IO_DRV_J, M_IO_DRV_J)

#define S_IO_DRV_K		6
#define M_IO_DRV_K		_SB_MAKEMASK(2, S_IO_DRV_K)
#define V_IO_DRV_K(x)		_SB_MAKEVALUE(x, S_IO_DRV_K)
#define G_IO_DRV_K(x)		_SB_GETVALUE(x, S_IO_DRV_K, M_IO_DRV_K)

#define S_IO_DRV_L		10
#define M_IO_DRV_L		_SB_MAKEMASK(2, S_IO_DRV_L)
#define V_IO_DRV_L(x)		_SB_MAKEVALUE(x, S_IO_DRV_L)
#define G_IO_DRV_L(x)		_SB_GETVALUE(x, S_IO_DRV_L, M_IO_DRV_L)

#define S_IO_DRV_M		14
#define M_IO_DRV_M		_SB_MAKEMASK(2, S_IO_DRV_M)
#define V_IO_DRV_M(x)		_SB_MAKEVALUE(x, S_IO_DRV_M)
#define G_IO_DRV_M(x)		_SB_GETVALUE(x, S_IO_DRV_M, M_IO_DRV_M)

/*
                                                            
 */

#define S_IO_SLEW3		0
#define M_IO_SLEW3		_SB_MAKEMASK(2, S_IO_SLEW3)
#define V_IO_SLEW3(x)		_SB_MAKEVALUE(x, S_IO_SLEW3)
#define G_IO_SLEW3(x)		_SB_GETVALUE(x, S_IO_SLEW3, M_IO_SLEW3)

#define S_IO_DRV_N		2
#define M_IO_DRV_N		_SB_MAKEMASK(2, S_IO_DRV_N)
#define V_IO_DRV_N(x)		_SB_MAKEVALUE(x, S_IO_DRV_N)
#define G_IO_DRV_N(x)		_SB_GETVALUE(x, S_IO_DRV_N, M_IO_DRV_N)

#define S_IO_DRV_P		6
#define M_IO_DRV_P		_SB_MAKEMASK(2, S_IO_DRV_P)
#define V_IO_DRV_P(x)		_SB_MAKEVALUE(x, S_IO_DRV_P)
#define G_IO_DRV_P(x)		_SB_GETVALUE(x, S_IO_DRV_P, M_IO_DRV_P)

#define S_IO_DRV_Q		10
#define M_IO_DRV_Q		_SB_MAKEMASK(2, S_IO_DRV_Q)
#define V_IO_DRV_Q(x)		_SB_MAKEVALUE(x, S_IO_DRV_Q)
#define G_IO_DRV_Q(x)		_SB_GETVALUE(x, S_IO_DRV_Q, M_IO_DRV_Q)

#define S_IO_DRV_R		14
#define M_IO_DRV_R		_SB_MAKEMASK(2, S_IO_DRV_R)
#define V_IO_DRV_R(x)		_SB_MAKEVALUE(x, S_IO_DRV_R)
#define G_IO_DRV_R(x)		_SB_GETVALUE(x, S_IO_DRV_R, M_IO_DRV_R)


/*
                                             
 */

#define M_PCMCIA_CFG_ATTRMEM	_SB_MAKEMASK1(0)
#define M_PCMCIA_CFG_3VEN	_SB_MAKEMASK1(1)
#define M_PCMCIA_CFG_5VEN	_SB_MAKEMASK1(2)
#define M_PCMCIA_CFG_VPPEN	_SB_MAKEMASK1(3)
#define M_PCMCIA_CFG_RESET	_SB_MAKEMASK1(4)
#define M_PCMCIA_CFG_APWRONEN	_SB_MAKEMASK1(5)
#define M_PCMCIA_CFG_CDMASK	_SB_MAKEMASK1(6)
#define M_PCMCIA_CFG_WPMASK	_SB_MAKEMASK1(7)
#define M_PCMCIA_CFG_RDYMASK	_SB_MAKEMASK1(8)
#define M_PCMCIA_CFG_PWRCTL	_SB_MAKEMASK1(9)

#if SIBYTE_HDR_FEATURE_CHIP(1480)
#define S_PCMCIA_MODE		16
#define M_PCMCIA_MODE		_SB_MAKEMASK(3, S_PCMCIA_MODE)
#define V_PCMCIA_MODE(x)	_SB_MAKEVALUE(x, S_PCMCIA_MODE)
#define G_PCMCIA_MODE(x)	_SB_GETVALUE(x, S_PCMCIA_MODE, M_PCMCIA_MODE)

#define K_PCMCIA_MODE_PCMA_NOB	0	/*                             */
#define K_PCMCIA_MODE_IDEA_NOB	1	/*                 */
#define K_PCMCIA_MODE_PCMIOA_NOB 2	/*                             */
#define K_PCMCIA_MODE_PCMA_PCMB 4	/*                                          */
#define K_PCMCIA_MODE_IDEA_PCMB 5	/*                              */
#define K_PCMCIA_MODE_PCMA_IDEB 6	/*                              */
#define K_PCMCIA_MODE_IDEA_IDEB 7	/*                  */
#endif


/*
                                      
 */

#define M_PCMCIA_STATUS_CD1	_SB_MAKEMASK1(0)
#define M_PCMCIA_STATUS_CD2	_SB_MAKEMASK1(1)
#define M_PCMCIA_STATUS_VS1	_SB_MAKEMASK1(2)
#define M_PCMCIA_STATUS_VS2	_SB_MAKEMASK1(3)
#define M_PCMCIA_STATUS_WP	_SB_MAKEMASK1(4)
#define M_PCMCIA_STATUS_RDY	_SB_MAKEMASK1(5)
#define M_PCMCIA_STATUS_3VEN	_SB_MAKEMASK1(6)
#define M_PCMCIA_STATUS_5VEN	_SB_MAKEMASK1(7)
#define M_PCMCIA_STATUS_CDCHG	_SB_MAKEMASK1(8)
#define M_PCMCIA_STATUS_WPCHG	_SB_MAKEMASK1(9)
#define M_PCMCIA_STATUS_RDYCHG	_SB_MAKEMASK1(10)

/*
                                            
 */

#define K_GPIO_INTR_DISABLE	0
#define K_GPIO_INTR_EDGE	1
#define K_GPIO_INTR_LEVEL	2
#define K_GPIO_INTR_SPLIT	3

#define S_GPIO_INTR_TYPEX(n)	(((n)/2)*2)
#define M_GPIO_INTR_TYPEX(n)	_SB_MAKEMASK(2, S_GPIO_INTR_TYPEX(n))
#define V_GPIO_INTR_TYPEX(n, x) _SB_MAKEVALUE(x, S_GPIO_INTR_TYPEX(n))
#define G_GPIO_INTR_TYPEX(n, x) _SB_GETVALUE(x, S_GPIO_INTR_TYPEX(n), M_GPIO_INTR_TYPEX(n))

#define S_GPIO_INTR_TYPE0	0
#define M_GPIO_INTR_TYPE0	_SB_MAKEMASK(2, S_GPIO_INTR_TYPE0)
#define V_GPIO_INTR_TYPE0(x)	_SB_MAKEVALUE(x, S_GPIO_INTR_TYPE0)
#define G_GPIO_INTR_TYPE0(x)	_SB_GETVALUE(x, S_GPIO_INTR_TYPE0, M_GPIO_INTR_TYPE0)

#define S_GPIO_INTR_TYPE2	2
#define M_GPIO_INTR_TYPE2	_SB_MAKEMASK(2, S_GPIO_INTR_TYPE2)
#define V_GPIO_INTR_TYPE2(x)	_SB_MAKEVALUE(x, S_GPIO_INTR_TYPE2)
#define G_GPIO_INTR_TYPE2(x)	_SB_GETVALUE(x, S_GPIO_INTR_TYPE2, M_GPIO_INTR_TYPE2)

#define S_GPIO_INTR_TYPE4	4
#define M_GPIO_INTR_TYPE4	_SB_MAKEMASK(2, S_GPIO_INTR_TYPE4)
#define V_GPIO_INTR_TYPE4(x)	_SB_MAKEVALUE(x, S_GPIO_INTR_TYPE4)
#define G_GPIO_INTR_TYPE4(x)	_SB_GETVALUE(x, S_GPIO_INTR_TYPE4, M_GPIO_INTR_TYPE4)

#define S_GPIO_INTR_TYPE6	6
#define M_GPIO_INTR_TYPE6	_SB_MAKEMASK(2, S_GPIO_INTR_TYPE6)
#define V_GPIO_INTR_TYPE6(x)	_SB_MAKEVALUE(x, S_GPIO_INTR_TYPE6)
#define G_GPIO_INTR_TYPE6(x)	_SB_GETVALUE(x, S_GPIO_INTR_TYPE6, M_GPIO_INTR_TYPE6)

#define S_GPIO_INTR_TYPE8	8
#define M_GPIO_INTR_TYPE8	_SB_MAKEMASK(2, S_GPIO_INTR_TYPE8)
#define V_GPIO_INTR_TYPE8(x)	_SB_MAKEVALUE(x, S_GPIO_INTR_TYPE8)
#define G_GPIO_INTR_TYPE8(x)	_SB_GETVALUE(x, S_GPIO_INTR_TYPE8, M_GPIO_INTR_TYPE8)

#define S_GPIO_INTR_TYPE10	10
#define M_GPIO_INTR_TYPE10	_SB_MAKEMASK(2, S_GPIO_INTR_TYPE10)
#define V_GPIO_INTR_TYPE10(x)	_SB_MAKEVALUE(x, S_GPIO_INTR_TYPE10)
#define G_GPIO_INTR_TYPE10(x)	_SB_GETVALUE(x, S_GPIO_INTR_TYPE10, M_GPIO_INTR_TYPE10)

#define S_GPIO_INTR_TYPE12	12
#define M_GPIO_INTR_TYPE12	_SB_MAKEMASK(2, S_GPIO_INTR_TYPE12)
#define V_GPIO_INTR_TYPE12(x)	_SB_MAKEVALUE(x, S_GPIO_INTR_TYPE12)
#define G_GPIO_INTR_TYPE12(x)	_SB_GETVALUE(x, S_GPIO_INTR_TYPE12, M_GPIO_INTR_TYPE12)

#define S_GPIO_INTR_TYPE14	14
#define M_GPIO_INTR_TYPE14	_SB_MAKEMASK(2, S_GPIO_INTR_TYPE14)
#define V_GPIO_INTR_TYPE14(x)	_SB_MAKEVALUE(x, S_GPIO_INTR_TYPE14)
#define G_GPIO_INTR_TYPE14(x)	_SB_GETVALUE(x, S_GPIO_INTR_TYPE14, M_GPIO_INTR_TYPE14)

#if SIBYTE_HDR_FEATURE_CHIP(1480)

/*
                                          
 */

#define K_GPIO_INTR_BOTHEDGE	0
#define K_GPIO_INTR_RISEEDGE	1
#define K_GPIO_INTR_UNPRED1	2
#define K_GPIO_INTR_UNPRED2	3

#define S_GPIO_INTR_ATYPEX(n)	(((n)/2)*2)
#define M_GPIO_INTR_ATYPEX(n)	_SB_MAKEMASK(2, S_GPIO_INTR_ATYPEX(n))
#define V_GPIO_INTR_ATYPEX(n, x)	_SB_MAKEVALUE(x, S_GPIO_INTR_ATYPEX(n))
#define G_GPIO_INTR_ATYPEX(n, x)	_SB_GETVALUE(x, S_GPIO_INTR_ATYPEX(n), M_GPIO_INTR_ATYPEX(n))

#define S_GPIO_INTR_ATYPE0	0
#define M_GPIO_INTR_ATYPE0	_SB_MAKEMASK(2, S_GPIO_INTR_ATYPE0)
#define V_GPIO_INTR_ATYPE0(x)	_SB_MAKEVALUE(x, S_GPIO_INTR_ATYPE0)
#define G_GPIO_INTR_ATYPE0(x)	_SB_GETVALUE(x, S_GPIO_INTR_ATYPE0, M_GPIO_INTR_ATYPE0)

#define S_GPIO_INTR_ATYPE2	2
#define M_GPIO_INTR_ATYPE2	_SB_MAKEMASK(2, S_GPIO_INTR_ATYPE2)
#define V_GPIO_INTR_ATYPE2(x)	_SB_MAKEVALUE(x, S_GPIO_INTR_ATYPE2)
#define G_GPIO_INTR_ATYPE2(x)	_SB_GETVALUE(x, S_GPIO_INTR_ATYPE2, M_GPIO_INTR_ATYPE2)

#define S_GPIO_INTR_ATYPE4	4
#define M_GPIO_INTR_ATYPE4	_SB_MAKEMASK(2, S_GPIO_INTR_ATYPE4)
#define V_GPIO_INTR_ATYPE4(x)	_SB_MAKEVALUE(x, S_GPIO_INTR_ATYPE4)
#define G_GPIO_INTR_ATYPE4(x)	_SB_GETVALUE(x, S_GPIO_INTR_ATYPE4, M_GPIO_INTR_ATYPE4)

#define S_GPIO_INTR_ATYPE6	6
#define M_GPIO_INTR_ATYPE6	_SB_MAKEMASK(2, S_GPIO_INTR_ATYPE6)
#define V_GPIO_INTR_ATYPE6(x)	_SB_MAKEVALUE(x, S_GPIO_INTR_ATYPE6)
#define G_GPIO_INTR_ATYPE6(x)	_SB_GETVALUE(x, S_GPIO_INTR_ATYPE6, M_GPIO_INTR_ATYPE6)

#define S_GPIO_INTR_ATYPE8	8
#define M_GPIO_INTR_ATYPE8	_SB_MAKEMASK(2, S_GPIO_INTR_ATYPE8)
#define V_GPIO_INTR_ATYPE8(x)	_SB_MAKEVALUE(x, S_GPIO_INTR_ATYPE8)
#define G_GPIO_INTR_ATYPE8(x)	_SB_GETVALUE(x, S_GPIO_INTR_ATYPE8, M_GPIO_INTR_ATYPE8)

#define S_GPIO_INTR_ATYPE10	10
#define M_GPIO_INTR_ATYPE10	_SB_MAKEMASK(2, S_GPIO_INTR_ATYPE10)
#define V_GPIO_INTR_ATYPE10(x)	_SB_MAKEVALUE(x, S_GPIO_INTR_ATYPE10)
#define G_GPIO_INTR_ATYPE10(x)	_SB_GETVALUE(x, S_GPIO_INTR_ATYPE10, M_GPIO_INTR_ATYPE10)

#define S_GPIO_INTR_ATYPE12	12
#define M_GPIO_INTR_ATYPE12	_SB_MAKEMASK(2, S_GPIO_INTR_ATYPE12)
#define V_GPIO_INTR_ATYPE12(x)	_SB_MAKEVALUE(x, S_GPIO_INTR_ATYPE12)
#define G_GPIO_INTR_ATYPE12(x)	_SB_GETVALUE(x, S_GPIO_INTR_ATYPE12, M_GPIO_INTR_ATYPE12)

#define S_GPIO_INTR_ATYPE14	14
#define M_GPIO_INTR_ATYPE14	_SB_MAKEMASK(2, S_GPIO_INTR_ATYPE14)
#define V_GPIO_INTR_ATYPE14(x)	_SB_MAKEVALUE(x, S_GPIO_INTR_ATYPE14)
#define G_GPIO_INTR_ATYPE14(x)	_SB_GETVALUE(x, S_GPIO_INTR_ATYPE14, M_GPIO_INTR_ATYPE14)
#endif


#endif
