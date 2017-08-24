#ifndef __ARCH_ARM_MACH_OMAP2_CM_REGBITS_24XX_H
#define __ARCH_ARM_MACH_OMAP2_CM_REGBITS_24XX_H

/*
 * OMAP24XX Clock Management register bits
 *
 * Copyright (C) 2007 Texas Instruments, Inc.
 * Copyright (C) 2007 Nokia Corporation
 *
 * Written by Paul Walmsley
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*                               */

/*                                                 */
#define OMAP24XX_EN_CAM_SHIFT				31
#define OMAP24XX_EN_CAM_MASK				(1 << 31)
#define OMAP24XX_EN_WDT4_SHIFT				29
#define OMAP24XX_EN_WDT4_MASK				(1 << 29)
#define OMAP2420_EN_WDT3_SHIFT				28
#define OMAP2420_EN_WDT3_MASK				(1 << 28)
#define OMAP24XX_EN_MSPRO_SHIFT				27
#define OMAP24XX_EN_MSPRO_MASK				(1 << 27)
#define OMAP24XX_EN_FAC_SHIFT				25
#define OMAP24XX_EN_FAC_MASK				(1 << 25)
#define OMAP2420_EN_EAC_SHIFT				24
#define OMAP2420_EN_EAC_MASK				(1 << 24)
#define OMAP24XX_EN_HDQ_SHIFT				23
#define OMAP24XX_EN_HDQ_MASK				(1 << 23)
#define OMAP2420_EN_I2C2_SHIFT				20
#define OMAP2420_EN_I2C2_MASK				(1 << 20)
#define OMAP2420_EN_I2C1_SHIFT				19
#define OMAP2420_EN_I2C1_MASK				(1 << 19)

/*                                                 */
#define OMAP2430_EN_MCBSP5_SHIFT			5
#define OMAP2430_EN_MCBSP5_MASK				(1 << 5)
#define OMAP2430_EN_MCBSP4_SHIFT			4
#define OMAP2430_EN_MCBSP4_MASK				(1 << 4)
#define OMAP2430_EN_MCBSP3_SHIFT			3
#define OMAP2430_EN_MCBSP3_MASK				(1 << 3)
#define OMAP24XX_EN_SSI_SHIFT				1
#define OMAP24XX_EN_SSI_MASK				(1 << 1)

/*                                               */
#define OMAP24XX_EN_MPU_WDT_SHIFT			3
#define OMAP24XX_EN_MPU_WDT_MASK			(1 << 3)

/*                                */

/*               */
/*           */
#define OMAP2430_ST_MPU_MASK				(1 << 0)

/*               */
#define OMAP24XX_CLKSEL_MPU_SHIFT			0
#define OMAP24XX_CLKSEL_MPU_MASK			(0x1f << 0)
#define OMAP24XX_CLKSEL_MPU_WIDTH			5

/*                  */
#define OMAP24XX_AUTOSTATE_MPU_SHIFT			0
#define OMAP24XX_AUTOSTATE_MPU_MASK			(1 << 0)

/*                              */
#define OMAP24XX_EN_TV_SHIFT				2
#define OMAP24XX_EN_TV_MASK				(1 << 2)
#define OMAP24XX_EN_DSS2_SHIFT				1
#define OMAP24XX_EN_DSS2_MASK				(1 << 1)
#define OMAP24XX_EN_DSS1_SHIFT				0
#define OMAP24XX_EN_DSS1_MASK				(1 << 0)

/*                               */
#define OMAP2430_EN_I2CHS2_SHIFT			20
#define OMAP2430_EN_I2CHS2_MASK				(1 << 20)
#define OMAP2430_EN_I2CHS1_SHIFT			19
#define OMAP2430_EN_I2CHS1_MASK				(1 << 19)
#define OMAP2430_EN_MMCHSDB2_SHIFT			17
#define OMAP2430_EN_MMCHSDB2_MASK			(1 << 17)
#define OMAP2430_EN_MMCHSDB1_SHIFT			16
#define OMAP2430_EN_MMCHSDB1_MASK			(1 << 16)

/*                               */
#define OMAP24XX_EN_MAILBOXES_SHIFT			30
#define OMAP24XX_EN_MAILBOXES_MASK			(1 << 30)
#define OMAP24XX_EN_DSS_SHIFT				0
#define OMAP24XX_EN_DSS_MASK				(1 << 0)

/*                               */

/*                 */
/*           */
#define OMAP2430_EN_SDRC_SHIFT				2
#define OMAP2430_EN_SDRC_MASK				(1 << 2)

/*                 */
#define OMAP24XX_EN_PKA_SHIFT				4
#define OMAP24XX_EN_PKA_MASK				(1 << 4)
#define OMAP24XX_EN_AES_SHIFT				3
#define OMAP24XX_EN_AES_MASK				(1 << 3)
#define OMAP24XX_EN_RNG_SHIFT				2
#define OMAP24XX_EN_RNG_MASK				(1 << 2)
#define OMAP24XX_EN_SHA_SHIFT				1
#define OMAP24XX_EN_SHA_MASK				(1 << 1)
#define OMAP24XX_EN_DES_SHIFT				0
#define OMAP24XX_EN_DES_MASK				(1 << 0)

/*                               */
#define OMAP24XX_ST_MAILBOXES_SHIFT			30
#define OMAP24XX_ST_MAILBOXES_MASK			(1 << 30)
#define OMAP24XX_ST_WDT4_SHIFT				29
#define OMAP24XX_ST_WDT4_MASK				(1 << 29)
#define OMAP2420_ST_WDT3_SHIFT				28
#define OMAP2420_ST_WDT3_MASK				(1 << 28)
#define OMAP24XX_ST_MSPRO_SHIFT				27
#define OMAP24XX_ST_MSPRO_MASK				(1 << 27)
#define OMAP24XX_ST_FAC_SHIFT				25
#define OMAP24XX_ST_FAC_MASK				(1 << 25)
#define OMAP2420_ST_EAC_SHIFT				24
#define OMAP2420_ST_EAC_MASK				(1 << 24)
#define OMAP24XX_ST_HDQ_SHIFT				23
#define OMAP24XX_ST_HDQ_MASK				(1 << 23)
#define OMAP2420_ST_I2C2_SHIFT				20
#define OMAP2420_ST_I2C2_MASK				(1 << 20)
#define OMAP2430_ST_I2CHS1_SHIFT			19
#define OMAP2430_ST_I2CHS1_MASK				(1 << 19)
#define OMAP2420_ST_I2C1_SHIFT				19
#define OMAP2420_ST_I2C1_MASK				(1 << 19)
#define OMAP2430_ST_I2CHS2_SHIFT			20
#define OMAP2430_ST_I2CHS2_MASK				(1 << 20)
#define OMAP24XX_ST_MCBSP2_SHIFT			16
#define OMAP24XX_ST_MCBSP2_MASK				(1 << 16)
#define OMAP24XX_ST_MCBSP1_SHIFT			15
#define OMAP24XX_ST_MCBSP1_MASK				(1 << 15)
#define OMAP24XX_ST_DSS_SHIFT				0
#define OMAP24XX_ST_DSS_MASK				(1 << 0)

/*                 */
#define OMAP2430_ST_MCBSP5_SHIFT			5
#define OMAP2430_ST_MCBSP5_MASK				(1 << 5)
#define OMAP2430_ST_MCBSP4_SHIFT			4
#define OMAP2430_ST_MCBSP4_MASK				(1 << 4)
#define OMAP2430_ST_MCBSP3_SHIFT			3
#define OMAP2430_ST_MCBSP3_MASK				(1 << 3)
#define OMAP24XX_ST_SSI_SHIFT				1
#define OMAP24XX_ST_SSI_MASK				(1 << 1)

/*                 */
/*           */
#define OMAP2430_ST_SDRC_MASK				(1 << 2)

/*                 */
#define OMAP24XX_ST_PKA_SHIFT				4
#define OMAP24XX_ST_PKA_MASK				(1 << 4)
#define OMAP24XX_ST_AES_SHIFT				3
#define OMAP24XX_ST_AES_MASK				(1 << 3)
#define OMAP24XX_ST_RNG_SHIFT				2
#define OMAP24XX_ST_RNG_MASK				(1 << 2)
#define OMAP24XX_ST_SHA_SHIFT				1
#define OMAP24XX_ST_SHA_MASK				(1 << 1)
#define OMAP24XX_ST_DES_SHIFT				0
#define OMAP24XX_ST_DES_MASK				(1 << 0)

/*                   */
#define OMAP24XX_AUTO_CAM_MASK				(1 << 31)
#define OMAP24XX_AUTO_MAILBOXES_MASK			(1 << 30)
#define OMAP24XX_AUTO_WDT4_MASK				(1 << 29)
#define OMAP2420_AUTO_WDT3_MASK				(1 << 28)
#define OMAP24XX_AUTO_MSPRO_MASK			(1 << 27)
#define OMAP2420_AUTO_MMC_MASK				(1 << 26)
#define OMAP24XX_AUTO_FAC_MASK				(1 << 25)
#define OMAP2420_AUTO_EAC_MASK				(1 << 24)
#define OMAP24XX_AUTO_HDQ_MASK				(1 << 23)
#define OMAP24XX_AUTO_UART2_MASK			(1 << 22)
#define OMAP24XX_AUTO_UART1_MASK			(1 << 21)
#define OMAP24XX_AUTO_I2C2_MASK				(1 << 20)
#define OMAP24XX_AUTO_I2C1_MASK				(1 << 19)
#define OMAP24XX_AUTO_MCSPI2_MASK			(1 << 18)
#define OMAP24XX_AUTO_MCSPI1_MASK			(1 << 17)
#define OMAP24XX_AUTO_MCBSP2_MASK			(1 << 16)
#define OMAP24XX_AUTO_MCBSP1_MASK			(1 << 15)
#define OMAP24XX_AUTO_GPT12_MASK			(1 << 14)
#define OMAP24XX_AUTO_GPT11_MASK			(1 << 13)
#define OMAP24XX_AUTO_GPT10_MASK			(1 << 12)
#define OMAP24XX_AUTO_GPT9_MASK				(1 << 11)
#define OMAP24XX_AUTO_GPT8_MASK				(1 << 10)
#define OMAP24XX_AUTO_GPT7_MASK				(1 << 9)
#define OMAP24XX_AUTO_GPT6_MASK				(1 << 8)
#define OMAP24XX_AUTO_GPT5_MASK				(1 << 7)
#define OMAP24XX_AUTO_GPT4_MASK				(1 << 6)
#define OMAP24XX_AUTO_GPT3_MASK				(1 << 5)
#define OMAP24XX_AUTO_GPT2_MASK				(1 << 4)
#define OMAP2420_AUTO_VLYNQ_MASK			(1 << 3)
#define OMAP24XX_AUTO_DSS_MASK				(1 << 0)

/*                   */
#define OMAP2430_AUTO_MDM_INTC_MASK			(1 << 11)
#define OMAP2430_AUTO_GPIO5_MASK			(1 << 10)
#define OMAP2430_AUTO_MCSPI3_MASK			(1 << 9)
#define OMAP2430_AUTO_MMCHS2_MASK			(1 << 8)
#define OMAP2430_AUTO_MMCHS1_MASK			(1 << 7)
#define OMAP2430_AUTO_USBHS_MASK			(1 << 6)
#define OMAP2430_AUTO_MCBSP5_MASK			(1 << 5)
#define OMAP2430_AUTO_MCBSP4_MASK			(1 << 4)
#define OMAP2430_AUTO_MCBSP3_MASK			(1 << 3)
#define OMAP24XX_AUTO_UART3_MASK			(1 << 2)
#define OMAP24XX_AUTO_SSI_MASK				(1 << 1)
#define OMAP24XX_AUTO_USB_MASK				(1 << 0)

/*                   */
#define OMAP24XX_AUTO_SDRC_SHIFT			2
#define OMAP24XX_AUTO_SDRC_MASK				(1 << 2)
#define OMAP24XX_AUTO_GPMC_SHIFT			1
#define OMAP24XX_AUTO_GPMC_MASK				(1 << 1)
#define OMAP24XX_AUTO_SDMA_SHIFT			0
#define OMAP24XX_AUTO_SDMA_MASK				(1 << 0)

/*                   */
#define OMAP24XX_AUTO_PKA_MASK				(1 << 4)
#define OMAP24XX_AUTO_AES_MASK				(1 << 3)
#define OMAP24XX_AUTO_RNG_MASK				(1 << 2)
#define OMAP24XX_AUTO_SHA_MASK				(1 << 1)
#define OMAP24XX_AUTO_DES_MASK				(1 << 0)

/*                 */
#define OMAP24XX_CLKSEL_USB_SHIFT			25
#define OMAP24XX_CLKSEL_USB_MASK			(0x7 << 25)
#define OMAP24XX_CLKSEL_SSI_SHIFT			20
#define OMAP24XX_CLKSEL_SSI_MASK			(0x1f << 20)
#define OMAP2420_CLKSEL_VLYNQ_SHIFT			15
#define OMAP2420_CLKSEL_VLYNQ_MASK			(0x1f << 15)
#define OMAP24XX_CLKSEL_DSS2_SHIFT			13
#define OMAP24XX_CLKSEL_DSS2_MASK			(0x1 << 13)
#define OMAP24XX_CLKSEL_DSS1_SHIFT			8
#define OMAP24XX_CLKSEL_DSS1_MASK			(0x1f << 8)
#define OMAP24XX_CLKSEL_L4_SHIFT			5
#define OMAP24XX_CLKSEL_L4_MASK				(0x3 << 5)
#define OMAP24XX_CLKSEL_L4_WIDTH			2
#define OMAP24XX_CLKSEL_L3_SHIFT			0
#define OMAP24XX_CLKSEL_L3_MASK				(0x1f << 0)
#define OMAP24XX_CLKSEL_L3_WIDTH			5

/*                 */
#define OMAP24XX_CLKSEL_GPT12_SHIFT			22
#define OMAP24XX_CLKSEL_GPT12_MASK			(0x3 << 22)
#define OMAP24XX_CLKSEL_GPT11_SHIFT			20
#define OMAP24XX_CLKSEL_GPT11_MASK			(0x3 << 20)
#define OMAP24XX_CLKSEL_GPT10_SHIFT			18
#define OMAP24XX_CLKSEL_GPT10_MASK			(0x3 << 18)
#define OMAP24XX_CLKSEL_GPT9_SHIFT			16
#define OMAP24XX_CLKSEL_GPT9_MASK			(0x3 << 16)
#define OMAP24XX_CLKSEL_GPT8_SHIFT			14
#define OMAP24XX_CLKSEL_GPT8_MASK			(0x3 << 14)
#define OMAP24XX_CLKSEL_GPT7_SHIFT			12
#define OMAP24XX_CLKSEL_GPT7_MASK			(0x3 << 12)
#define OMAP24XX_CLKSEL_GPT6_SHIFT			10
#define OMAP24XX_CLKSEL_GPT6_MASK			(0x3 << 10)
#define OMAP24XX_CLKSEL_GPT5_SHIFT			8
#define OMAP24XX_CLKSEL_GPT5_MASK			(0x3 << 8)
#define OMAP24XX_CLKSEL_GPT4_SHIFT			6
#define OMAP24XX_CLKSEL_GPT4_MASK			(0x3 << 6)
#define OMAP24XX_CLKSEL_GPT3_SHIFT			4
#define OMAP24XX_CLKSEL_GPT3_MASK			(0x3 << 4)
#define OMAP24XX_CLKSEL_GPT2_SHIFT			2
#define OMAP24XX_CLKSEL_GPT2_MASK			(0x3 << 2)

/*                   */
#define OMAP24XX_AUTOSTATE_DSS_SHIFT			2
#define OMAP24XX_AUTOSTATE_DSS_MASK			(1 << 2)
#define OMAP24XX_AUTOSTATE_L4_SHIFT			1
#define OMAP24XX_AUTOSTATE_L4_MASK			(1 << 1)
#define OMAP24XX_AUTOSTATE_L3_SHIFT			0
#define OMAP24XX_AUTOSTATE_L3_MASK			(1 << 0)

/*               */
#define OMAP24XX_EN_3D_SHIFT				2
#define OMAP24XX_EN_3D_MASK				(1 << 2)
#define OMAP24XX_EN_2D_SHIFT				1
#define OMAP24XX_EN_2D_MASK				(1 << 1)

/*                             */

/*                             */

/*                             */

/*                  */
#define OMAP24XX_AUTOSTATE_GFX_SHIFT			0
#define OMAP24XX_AUTOSTATE_GFX_MASK			(1 << 0)

/*                              */

/*                              */
#define OMAP2430_EN_ICR_SHIFT				6
#define OMAP2430_EN_ICR_MASK				(1 << 6)
#define OMAP24XX_EN_OMAPCTRL_SHIFT			5
#define OMAP24XX_EN_OMAPCTRL_MASK			(1 << 5)
#define OMAP24XX_EN_WDT1_SHIFT				4
#define OMAP24XX_EN_WDT1_MASK				(1 << 4)
#define OMAP24XX_EN_32KSYNC_SHIFT			1
#define OMAP24XX_EN_32KSYNC_MASK			(1 << 1)

/*                              */
#define OMAP2430_ST_ICR_SHIFT				6
#define OMAP2430_ST_ICR_MASK				(1 << 6)
#define OMAP24XX_ST_OMAPCTRL_SHIFT			5
#define OMAP24XX_ST_OMAPCTRL_MASK			(1 << 5)
#define OMAP24XX_ST_WDT1_SHIFT				4
#define OMAP24XX_ST_WDT1_MASK				(1 << 4)
#define OMAP24XX_ST_MPU_WDT_SHIFT			3
#define OMAP24XX_ST_MPU_WDT_MASK			(1 << 3)
#define OMAP24XX_ST_32KSYNC_SHIFT			1
#define OMAP24XX_ST_32KSYNC_MASK			(1 << 1)

/*                  */
#define OMAP24XX_AUTO_OMAPCTRL_MASK			(1 << 5)
#define OMAP24XX_AUTO_WDT1_MASK				(1 << 4)
#define OMAP24XX_AUTO_MPU_WDT_MASK			(1 << 3)
#define OMAP24XX_AUTO_GPIOS_MASK			(1 << 2)
#define OMAP24XX_AUTO_32KSYNC_MASK			(1 << 1)
#define OMAP24XX_AUTO_GPT1_MASK				(1 << 0)

/*                */
#define OMAP24XX_CLKSEL_GPT1_SHIFT			0
#define OMAP24XX_CLKSEL_GPT1_MASK			(0x3 << 0)

/*              */
#define OMAP24XX_EN_54M_PLL_SHIFT			6
#define OMAP24XX_EN_54M_PLL_MASK			(0x3 << 6)
#define OMAP24XX_EN_96M_PLL_SHIFT			2
#define OMAP24XX_EN_96M_PLL_MASK			(0x3 << 2)
#define OMAP24XX_EN_DPLL_SHIFT				0
#define OMAP24XX_EN_DPLL_MASK				(0x3 << 0)

/*                 */
#define OMAP24XX_ST_54M_APLL_SHIFT			9
#define OMAP24XX_ST_54M_APLL_MASK			(1 << 9)
#define OMAP24XX_ST_96M_APLL_SHIFT			8
#define OMAP24XX_ST_96M_APLL_MASK			(1 << 8)
#define OMAP24XX_ST_54M_CLK_MASK			(1 << 6)
#define OMAP24XX_ST_12M_CLK_MASK			(1 << 5)
#define OMAP24XX_ST_48M_CLK_MASK			(1 << 4)
#define OMAP24XX_ST_96M_CLK_MASK			(1 << 2)
#define OMAP24XX_ST_CORE_CLK_SHIFT			0
#define OMAP24XX_ST_CORE_CLK_MASK			(0x3 << 0)

/*                 */
#define OMAP24XX_AUTO_54M_SHIFT				6
#define OMAP24XX_AUTO_54M_MASK				(0x3 << 6)
#define OMAP24XX_AUTO_96M_SHIFT				2
#define OMAP24XX_AUTO_96M_MASK				(0x3 << 2)
#define OMAP24XX_AUTO_DPLL_SHIFT			0
#define OMAP24XX_AUTO_DPLL_MASK				(0x3 << 0)

/*                */
#define OMAP2430_MAXDPLLFASTLOCK_SHIFT			28
#define OMAP2430_MAXDPLLFASTLOCK_MASK			(0x7 << 28)
#define OMAP24XX_APLLS_CLKIN_SHIFT			23
#define OMAP24XX_APLLS_CLKIN_MASK			(0x7 << 23)
#define OMAP24XX_DPLL_MULT_SHIFT			12
#define OMAP24XX_DPLL_MULT_MASK				(0x3ff << 12)
#define OMAP24XX_DPLL_DIV_SHIFT				8
#define OMAP24XX_DPLL_DIV_MASK				(0xf << 8)
#define OMAP24XX_54M_SOURCE_SHIFT			5
#define OMAP24XX_54M_SOURCE_MASK			(1 << 5)
#define OMAP24XX_54M_SOURCE_WIDTH			1
#define OMAP2430_96M_SOURCE_SHIFT			4
#define OMAP2430_96M_SOURCE_MASK			(1 << 4)
#define OMAP2430_96M_SOURCE_WIDTH			1
#define OMAP24XX_48M_SOURCE_SHIFT			3
#define OMAP24XX_48M_SOURCE_MASK			(1 << 3)
#define OMAP2430_ALTCLK_SOURCE_SHIFT			0
#define OMAP2430_ALTCLK_SOURCE_MASK			(0x7 << 0)

/*                */
#define OMAP24XX_CORE_CLK_SRC_SHIFT			0
#define OMAP24XX_CORE_CLK_SRC_MASK			(0x3 << 0)

/*               */
#define OMAP2420_EN_IVA_COP_SHIFT			10
#define OMAP2420_EN_IVA_COP_MASK			(1 << 10)
#define OMAP2420_EN_IVA_MPU_SHIFT			8
#define OMAP2420_EN_IVA_MPU_MASK			(1 << 8)
#define OMAP24XX_CM_FCLKEN_DSP_EN_DSP_SHIFT		0
#define OMAP24XX_CM_FCLKEN_DSP_EN_DSP_MASK		(1 << 0)

/*               */
#define OMAP2420_EN_DSP_IPI_SHIFT			1
#define OMAP2420_EN_DSP_IPI_MASK			(1 << 1)

/*               */
#define OMAP2420_ST_IVA_MASK				(1 << 8)
#define OMAP2420_ST_IPI_MASK				(1 << 1)
#define OMAP24XX_ST_DSP_MASK				(1 << 0)

/*                 */
#define OMAP2420_AUTO_DSP_IPI_MASK			(1 << 1)

/*               */
#define OMAP2420_SYNC_IVA_MASK				(1 << 13)
#define OMAP2420_CLKSEL_IVA_SHIFT			8
#define OMAP2420_CLKSEL_IVA_MASK			(0x1f << 8)
#define OMAP24XX_SYNC_DSP_MASK				(1 << 7)
#define OMAP24XX_CLKSEL_DSP_IF_SHIFT			5
#define OMAP24XX_CLKSEL_DSP_IF_MASK			(0x3 << 5)
#define OMAP24XX_CLKSEL_DSP_SHIFT			0
#define OMAP24XX_CLKSEL_DSP_MASK			(0x1f << 0)

/*                  */
#define OMAP2420_AUTOSTATE_IVA_SHIFT			8
#define OMAP2420_AUTOSTATE_IVA_MASK			(1 << 8)
#define OMAP24XX_AUTOSTATE_DSP_SHIFT			0
#define OMAP24XX_AUTOSTATE_DSP_MASK			(1 << 0)

/*               */
/*           */
#define OMAP2430_EN_OSC_SHIFT				1
#define OMAP2430_EN_OSC_MASK				(1 << 1)

/*               */
/*           */
#define OMAP2430_CM_ICLKEN_MDM_EN_MDM_SHIFT		0
#define OMAP2430_CM_ICLKEN_MDM_EN_MDM_MASK		(1 << 0)

/*                             */
/*           */

/*                 */
/*           */
#define OMAP2430_AUTO_OSC_MASK				(1 << 1)
#define OMAP2430_AUTO_MDM_MASK				(1 << 0)

/*               */
/*           */
#define OMAP2430_SYNC_MDM_MASK				(1 << 4)
#define OMAP2430_CLKSEL_MDM_SHIFT			0
#define OMAP2430_CLKSEL_MDM_MASK			(0xf << 0)

/*                  */
/*           */
#define OMAP2430_AUTOSTATE_MDM_SHIFT			0
#define OMAP2430_AUTOSTATE_MDM_MASK			(1 << 0)

/*                                                         */
#define OMAP24XX_CLKSTCTRL_DISABLE_AUTO		0x0
#define OMAP24XX_CLKSTCTRL_ENABLE_AUTO		0x1


#endif
