/*
 * AM33XX PRM_XXX register bits
 *
 * Copyright (C) 2011-2012 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __ARCH_ARM_MACH_OMAP2_PRM_REGBITS_33XX_H
#define __ARCH_ARM_MACH_OMAP2_PRM_REGBITS_33XX_H

#include "prm.h"

/*                                                         */
#define AM33XX_ABBOFF_ACT_EXPORT_SHIFT			1
#define AM33XX_ABBOFF_ACT_EXPORT_MASK			(1 << 1)

/*                                                         */
#define AM33XX_ABBOFF_SLEEP_EXPORT_SHIFT		2
#define AM33XX_ABBOFF_SLEEP_EXPORT_MASK			(1 << 2)

/*                                                         */
#define AM33XX_AIPOFF_SHIFT				8
#define AM33XX_AIPOFF_MASK				(1 << 8)

/*                         */
#define AM33XX_DEBUGSS_MEM_STATEST_SHIFT		17
#define AM33XX_DEBUGSS_MEM_STATEST_MASK			(0x3 << 17)

/*                                                         */
#define AM33XX_DISABLE_RTA_EXPORT_SHIFT			0
#define AM33XX_DISABLE_RTA_EXPORT_MASK			(1 << 0)

/*                                             */
#define AM33XX_DPLL_CORE_RECAL_EN_SHIFT			12
#define AM33XX_DPLL_CORE_RECAL_EN_MASK			(1 << 12)

/*                                             */
#define AM33XX_DPLL_CORE_RECAL_ST_SHIFT			12
#define AM33XX_DPLL_CORE_RECAL_ST_MASK			(1 << 12)

/*                                             */
#define AM33XX_DPLL_DDR_RECAL_EN_SHIFT			14
#define AM33XX_DPLL_DDR_RECAL_EN_MASK			(1 << 14)

/*                                             */
#define AM33XX_DPLL_DDR_RECAL_ST_SHIFT			14
#define AM33XX_DPLL_DDR_RECAL_ST_MASK			(1 << 14)

/*                                             */
#define AM33XX_DPLL_DISP_RECAL_EN_SHIFT			15
#define AM33XX_DPLL_DISP_RECAL_EN_MASK			(1 << 15)

/*                                             */
#define AM33XX_DPLL_DISP_RECAL_ST_SHIFT			13
#define AM33XX_DPLL_DISP_RECAL_ST_MASK			(1 << 13)

/*                                             */
#define AM33XX_DPLL_MPU_RECAL_EN_SHIFT			11
#define AM33XX_DPLL_MPU_RECAL_EN_MASK			(1 << 11)

/*                                             */
#define AM33XX_DPLL_MPU_RECAL_ST_SHIFT			11
#define AM33XX_DPLL_MPU_RECAL_ST_MASK			(1 << 11)

/*                                             */
#define AM33XX_DPLL_PER_RECAL_EN_SHIFT			13
#define AM33XX_DPLL_PER_RECAL_EN_MASK			(1 << 13)

/*                                             */
#define AM33XX_DPLL_PER_RECAL_ST_SHIFT			15
#define AM33XX_DPLL_PER_RECAL_ST_MASK			(1 << 15)

/*                       */
#define AM33XX_EMULATION_M3_RST_SHIFT			6
#define AM33XX_EMULATION_M3_RST_MASK			(1 << 6)

/*                      */
#define AM33XX_EMULATION_MPU_RST_SHIFT			5
#define AM33XX_EMULATION_MPU_RST_MASK			(1 << 5)

/*                                                         */
#define AM33XX_ENFUNC1_EXPORT_SHIFT			3
#define AM33XX_ENFUNC1_EXPORT_MASK			(1 << 3)

/*                                                         */
#define AM33XX_ENFUNC3_EXPORT_SHIFT			5
#define AM33XX_ENFUNC3_EXPORT_MASK			(1 << 5)

/*                                                         */
#define AM33XX_ENFUNC4_SHIFT				6
#define AM33XX_ENFUNC4_MASK				(1 << 6)

/*                                                         */
#define AM33XX_ENFUNC5_SHIFT				7
#define AM33XX_ENFUNC5_MASK				(1 << 7)

/*                   */
#define AM33XX_EXTERNAL_WARM_RST_SHIFT			5
#define AM33XX_EXTERNAL_WARM_RST_MASK			(1 << 5)

/*                                             */
#define AM33XX_FORCEWKUP_EN_SHIFT			10
#define AM33XX_FORCEWKUP_EN_MASK			(1 << 10)

/*                                             */
#define AM33XX_FORCEWKUP_ST_SHIFT			10
#define AM33XX_FORCEWKUP_ST_MASK			(1 << 10)

/*                          */
#define AM33XX_GFX_MEM_ONSTATE_SHIFT			17
#define AM33XX_GFX_MEM_ONSTATE_MASK			(0x3 << 17)

/*                          */
#define AM33XX_GFX_MEM_RETSTATE_SHIFT			6
#define AM33XX_GFX_MEM_RETSTATE_MASK			(1 << 6)

/*                        */
#define AM33XX_GFX_MEM_STATEST_SHIFT			4
#define AM33XX_GFX_MEM_STATEST_MASK			(0x3 << 4)

/*                                      */
#define AM33XX_GFX_RST_SHIFT				0
#define AM33XX_GFX_RST_MASK				(1 << 0)

/*                   */
#define AM33XX_GLOBAL_COLD_RST_SHIFT			0
#define AM33XX_GLOBAL_COLD_RST_MASK			(1 << 0)

/*                   */
#define AM33XX_GLOBAL_WARM_SW_RST_SHIFT			1
#define AM33XX_GLOBAL_WARM_SW_RST_MASK			(1 << 1)

/*                       */
#define AM33XX_ICECRUSHER_M3_RST_SHIFT			7
#define AM33XX_ICECRUSHER_M3_RST_MASK			(1 << 7)

/*                      */
#define AM33XX_ICECRUSHER_MPU_RST_SHIFT			6
#define AM33XX_ICECRUSHER_MPU_RST_MASK			(1 << 6)

/*                   */
#define AM33XX_ICEPICK_RST_SHIFT			9
#define AM33XX_ICEPICK_RST_MASK				(1 << 9)

/*                        */
#define AM33XX_PRUSS_LRST_SHIFT				1
#define AM33XX_PRUSS_LRST_MASK				(1 << 1)

/*                          */
#define AM33XX_PRUSS_MEM_ONSTATE_SHIFT			5
#define AM33XX_PRUSS_MEM_ONSTATE_MASK			(0x3 << 5)

/*                          */
#define AM33XX_PRUSS_MEM_RETSTATE_SHIFT			7
#define AM33XX_PRUSS_MEM_RETSTATE_MASK			(1 << 7)

/*                        */
#define AM33XX_PRUSS_MEM_STATEST_SHIFT			23
#define AM33XX_PRUSS_MEM_STATEST_MASK			(0x3 << 23)

/*
                                                                             
                                  
 */
#define AM33XX_INTRANSITION_SHIFT			20
#define AM33XX_INTRANSITION_MASK			(1 << 20)

/*                           */
#define AM33XX_LASTPOWERSTATEENTERED_SHIFT		24
#define AM33XX_LASTPOWERSTATEENTERED_MASK		(0x3 << 24)

/*                                                              */
#define AM33XX_LOGICRETSTATE_SHIFT			2
#define AM33XX_LOGICRETSTATE_MASK			(1 << 2)

/*                                                                        */
#define AM33XX_LOGICRETSTATE_3_3_SHIFT			3
#define AM33XX_LOGICRETSTATE_3_3_MASK			(1 << 3)

/*
                                                                             
                                  
 */
#define AM33XX_LOGICSTATEST_SHIFT			2
#define AM33XX_LOGICSTATEST_MASK			(1 << 2)

/*
                                                                   
                                                        
 */
#define AM33XX_LOWPOWERSTATECHANGE_SHIFT		4
#define AM33XX_LOWPOWERSTATECHANGE_MASK			(1 << 4)

/*                          */
#define AM33XX_MPU_L1_ONSTATE_SHIFT			18
#define AM33XX_MPU_L1_ONSTATE_MASK			(0x3 << 18)

/*                          */
#define AM33XX_MPU_L1_RETSTATE_SHIFT			22
#define AM33XX_MPU_L1_RETSTATE_MASK			(1 << 22)

/*                        */
#define AM33XX_MPU_L1_STATEST_SHIFT			6
#define AM33XX_MPU_L1_STATEST_MASK			(0x3 << 6)

/*                          */
#define AM33XX_MPU_L2_ONSTATE_SHIFT			20
#define AM33XX_MPU_L2_ONSTATE_MASK			(0x3 << 20)

/*                          */
#define AM33XX_MPU_L2_RETSTATE_SHIFT			23
#define AM33XX_MPU_L2_RETSTATE_MASK			(1 << 23)

/*                        */
#define AM33XX_MPU_L2_STATEST_SHIFT			8
#define AM33XX_MPU_L2_STATEST_MASK			(0x3 << 8)

/*                          */
#define AM33XX_MPU_RAM_ONSTATE_SHIFT			16
#define AM33XX_MPU_RAM_ONSTATE_MASK			(0x3 << 16)

/*                          */
#define AM33XX_MPU_RAM_RETSTATE_SHIFT			24
#define AM33XX_MPU_RAM_RETSTATE_MASK			(1 << 24)

/*                        */
#define AM33XX_MPU_RAM_STATEST_SHIFT			4
#define AM33XX_MPU_RAM_STATEST_MASK			(0x3 << 4)

/*                   */
#define AM33XX_MPU_SECURITY_VIOL_RST_SHIFT		2
#define AM33XX_MPU_SECURITY_VIOL_RST_MASK		(1 << 2)

/*                        */
#define AM33XX_PCHARGECNT_VALUE_SHIFT			0
#define AM33XX_PCHARGECNT_VALUE_MASK			(0x3f << 0)

/*                        */
#define AM33XX_PCI_LRST_SHIFT				0
#define AM33XX_PCI_LRST_MASK				(1 << 0)

/*                                            */
#define AM33XX_PCI_LRST_5_5_SHIFT			5
#define AM33XX_PCI_LRST_5_5_MASK			(1 << 5)

/*                          */
#define AM33XX_PER_MEM_ONSTATE_SHIFT			25
#define AM33XX_PER_MEM_ONSTATE_MASK			(0x3 << 25)

/*                          */
#define AM33XX_PER_MEM_RETSTATE_SHIFT			29
#define AM33XX_PER_MEM_RETSTATE_MASK			(1 << 29)

/*                        */
#define AM33XX_PER_MEM_STATEST_SHIFT			17
#define AM33XX_PER_MEM_STATEST_MASK			(0x3 << 17)

/*
                                                                   
                   
 */
#define AM33XX_POWERSTATE_SHIFT				0
#define AM33XX_POWERSTATE_MASK				(0x3 << 0)

/*                                                                           */
#define AM33XX_POWERSTATEST_SHIFT			0
#define AM33XX_POWERSTATEST_MASK			(0x3 << 0)

/*                          */
#define AM33XX_RAM_MEM_ONSTATE_SHIFT			30
#define AM33XX_RAM_MEM_ONSTATE_MASK			(0x3 << 30)

/*                          */
#define AM33XX_RAM_MEM_RETSTATE_SHIFT			27
#define AM33XX_RAM_MEM_RETSTATE_MASK			(1 << 27)

/*                        */
#define AM33XX_RAM_MEM_STATEST_SHIFT			21
#define AM33XX_RAM_MEM_STATEST_MASK			(0x3 << 21)

/*                                                       */
#define AM33XX_RETMODE_ENABLE_SHIFT			0
#define AM33XX_RETMODE_ENABLE_MASK			(1 << 0)

/*                      */
#define AM33XX_REV_SHIFT				0
#define AM33XX_REV_MASK					(0xff << 0)

/*                     */
#define AM33XX_RSTTIME1_SHIFT				0
#define AM33XX_RSTTIME1_MASK				(0xff << 0)

/*                     */
#define AM33XX_RSTTIME2_SHIFT				8
#define AM33XX_RSTTIME2_MASK				(0x1f << 8)

/*                     */
#define AM33XX_RST_GLOBAL_COLD_SW_SHIFT			1
#define AM33XX_RST_GLOBAL_COLD_SW_MASK			(1 << 1)

/*                     */
#define AM33XX_RST_GLOBAL_WARM_SW_SHIFT			0
#define AM33XX_RST_GLOBAL_WARM_SW_MASK			(1 << 0)

/*                        */
#define AM33XX_SLPCNT_VALUE_SHIFT			16
#define AM33XX_SLPCNT_VALUE_MASK			(0xff << 16)

/*                                                       */
#define AM33XX_SRAMLDO_STATUS_SHIFT			8
#define AM33XX_SRAMLDO_STATUS_MASK			(1 << 8)

/*                                                       */
#define AM33XX_SRAM_IN_TRANSITION_SHIFT			9
#define AM33XX_SRAM_IN_TRANSITION_MASK			(1 << 9)

/*                        */
#define AM33XX_STARTUP_COUNT_SHIFT			24
#define AM33XX_STARTUP_COUNT_MASK			(0xff << 24)

/*                                             */
#define AM33XX_TRANSITION_EN_SHIFT			8
#define AM33XX_TRANSITION_EN_MASK			(1 << 8)

/*                                             */
#define AM33XX_TRANSITION_ST_SHIFT			8
#define AM33XX_TRANSITION_ST_MASK			(1 << 8)

/*                        */
#define AM33XX_VSETUPCNT_VALUE_SHIFT			8
#define AM33XX_VSETUPCNT_VALUE_MASK			(0xff << 8)

/*                   */
#define AM33XX_WDT0_RST_SHIFT				3
#define AM33XX_WDT0_RST_MASK				(1 << 3)

/*                   */
#define AM33XX_WDT1_RST_SHIFT				4
#define AM33XX_WDT1_RST_MASK				(1 << 4)

/*                         */
#define AM33XX_WKUP_M3_LRST_SHIFT			3
#define AM33XX_WKUP_M3_LRST_MASK			(1 << 3)

/*                                                 */
#define AM33XX_WKUP_M3_LRST_5_5_SHIFT			5
#define AM33XX_WKUP_M3_LRST_5_5_MASK			(1 << 5)

#endif
