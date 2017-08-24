/*
 * Copyright (C) 2011 Marvell International Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 */

#ifndef __MV_U3D_PHY_H
#define __MV_U3D_PHY_H

#define USB3_POWER_PLL_CONTROL		0x1
#define USB3_KVCO_CALI_CONTROL		0x2
#define USB3_IMPEDANCE_CALI_CTRL	0x3
#define USB3_IMPEDANCE_TX_SSC		0x4
#define USB3_SQUELCH_FFE		0x6
#define USB3_GEN1_SET0			0xD
#define USB3_GEN2_SET0			0xF
#define USB3_GEN2_SET1			0x10
#define USB3_DIGITAL_LOOPBACK_EN	0x23
#define USB3_PHY_ISOLATION_MODE		0x26
#define USB3_TXDETRX			0x48
#define USB3_TX_EMPPH			0x5E
#define USB3_RESET_CONTROL		0x90
#define USB3_PIPE_SM_CTRL		0x91

#define USB3_RESET_CONTROL_RESET_PIPE			0x1
#define USB3_RESET_CONTROL_RESET_PHY			0x2

#define USB3_POWER_PLL_CONTROL_REF_FREF_SEL_MASK	(0x1F << 0)
#define USB3_POWER_PLL_CONTROL_REF_FREF_SEL_SHIFT	0
#define USB3_PLL_25MHZ					0x2
#define USB3_PLL_26MHZ					0x5
#define USB3_POWER_PLL_CONTROL_PHY_MODE_MASK		(0x7 << 5)
#define USB3_POWER_PLL_CONTROL_PHY_MODE_SHIFT		5
#define USB3_POWER_PLL_CONTROL_PU_MASK			(0xF << 12)
#define USB3_POWER_PLL_CONTROL_PU_SHIFT			12
#define USB3_POWER_PLL_CONTROL_PU			(0xF << 12)

#define USB3_KVCO_CALI_CONTROL_USE_MAX_PLL_RATE_MASK	(0x1 << 12)
#define USB3_KVCO_CALI_CONTROL_USE_MAX_PLL_RATE_SHIFT	12
#define USB3_KVCO_CALI_CONTROL_CAL_DONE_SHIFT		14
#define USB3_KVCO_CALI_CONTROL_CAL_START_SHIFT		15

#define USB3_SQUELCH_FFE_FFE_CAP_SEL_MASK		0xF
#define USB3_SQUELCH_FFE_FFE_CAP_SEL_SHIFT		0
#define USB3_SQUELCH_FFE_FFE_RES_SEL_MASK		(0x7 << 4)
#define USB3_SQUELCH_FFE_FFE_RES_SEL_SHIFT		4
#define USB3_SQUELCH_FFE_SQ_THRESH_IN_MASK		(0x1F << 8)
#define USB3_SQUELCH_FFE_SQ_THRESH_IN_SHIFT		8

#define USB3_GEN1_SET0_G1_TX_SLEW_CTRL_EN_MASK		(0x1 << 15)
#define USB3_GEN1_SET0_G1_TX_EMPH_EN_SHIFT		11

#define USB3_GEN2_SET0_G2_TX_AMP_MASK			(0x1F << 1)
#define USB3_GEN2_SET0_G2_TX_AMP_SHIFT			1
#define USB3_GEN2_SET0_G2_TX_AMP_ADJ_SHIFT		6
#define USB3_GEN2_SET0_G2_TX_EMPH_AMP_MASK		(0xF << 7)
#define USB3_GEN2_SET0_G2_TX_EMPH_AMP_SHIFT		7
#define USB3_GEN2_SET0_G2_TX_EMPH_EN_MASK		(0x1 << 11)
#define USB3_GEN2_SET0_G2_TX_EMPH_EN_SHIFT		11
#define USB3_GEN2_SET0_G2_TX_SLEW_CTRL_EN_MASK		(0x1 << 15)
#define USB3_GEN2_SET0_G2_TX_SLEW_CTRL_EN_SHIFT		15

#define USB3_GEN2_SET1_G2_RX_SELMUPI_MASK		(0x7 << 0)
#define USB3_GEN2_SET1_G2_RX_SELMUPI_SHIFT		0
#define USB3_GEN2_SET1_G2_RX_SELMUPF_MASK		(0x7 << 3)
#define USB3_GEN2_SET1_G2_RX_SELMUPF_SHIFT		3
#define USB3_GEN2_SET1_G2_RX_SELMUFI_MASK		(0x3 << 6)
#define USB3_GEN2_SET1_G2_RX_SELMUFI_SHIFT		6
#define USB3_GEN2_SET1_G2_RX_SELMUFF_MASK		(0x3 << 8)
#define USB3_GEN2_SET1_G2_RX_SELMUFF_SHIFT		8

#define USB3_DIGITAL_LOOPBACK_EN_SEL_BITS_MASK		(0x3 << 10)
#define USB3_DIGITAL_LOOPBACK_EN_SEL_BITS_SHIFT		10

#define USB3_IMPEDANCE_CALI_CTRL_IMP_CAL_THR_MASK	(0x7 << 12)
#define USB3_IMPEDANCE_CALI_CTRL_IMP_CAL_THR_SHIFT	12

#define USB3_IMPEDANCE_TX_SSC_SSC_AMP_MASK		(0x3F << 0)
#define USB3_IMPEDANCE_TX_SSC_SSC_AMP_SHIFT		0

#define USB3_PHY_ISOLATION_MODE_PHY_GEN_RX_MASK		0xF
#define USB3_PHY_ISOLATION_MODE_PHY_GEN_RX_SHIFT	0
#define USB3_PHY_ISOLATION_MODE_PHY_GEN_TX_MASK		(0xF << 4)
#define USB3_PHY_ISOLATION_MODE_PHY_GEN_TX_SHIFT	4
#define USB3_PHY_ISOLATION_MODE_TX_DRV_IDLE_MASK	(0x1 << 8)

#define USB3_TXDETRX_VTHSEL_MASK			(0x3 << 4)
#define USB3_TXDETRX_VTHSEL_SHIFT			4

#define USB3_TX_EMPPH_AMP_MASK				(0xF << 0)
#define USB3_TX_EMPPH_AMP_SHIFT				0
#define USB3_TX_EMPPH_EN_MASK				(0x1 << 6)
#define USB3_TX_EMPPH_EN_SHIFT				6
#define USB3_TX_EMPPH_AMP_FORCE_MASK			(0x1 << 7)
#define USB3_TX_EMPPH_AMP_FORCE_SHIFT			7
#define USB3_TX_EMPPH_PAR1_MASK				(0x1F << 8)
#define USB3_TX_EMPPH_PAR1_SHIFT			8
#define USB3_TX_EMPPH_PAR2_MASK				(0x1 << 13)
#define USB3_TX_EMPPH_PAR2_SHIFT			13

#define USB3_PIPE_SM_CTRL_PHY_INIT_DONE			15

#endif /*                */
