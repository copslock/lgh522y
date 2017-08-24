/*
 * OMAP5xxx bandgap registers, bitfields and temperature definitions
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 * Contact:
 *   Eduardo Valentin <eduardo.valentin@ti.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */
#ifndef __OMAP5XXX_BANDGAP_H
#define __OMAP5XXX_BANDGAP_H

/* 
                   
  
                                                    
                                                              
 */

/* 
                                
  
                                                    
                                         
  
                                                                         
 */

/*                               */
#define OMAP5430_FUSE_OPP_BGAP_GPU			0x0
#define OMAP5430_TEMP_SENSOR_GPU_OFFSET			0x150
#define OMAP5430_BGAP_THRESHOLD_GPU_OFFSET		0x1A8
#define OMAP5430_BGAP_TSHUT_GPU_OFFSET			0x1B4
#define OMAP5430_BGAP_CUMUL_DTEMP_GPU_OFFSET		0x1C0
#define OMAP5430_BGAP_DTEMP_GPU_0_OFFSET		0x1F4
#define OMAP5430_BGAP_DTEMP_GPU_1_OFFSET		0x1F8
#define OMAP5430_BGAP_DTEMP_GPU_2_OFFSET		0x1FC
#define OMAP5430_BGAP_DTEMP_GPU_3_OFFSET		0x200
#define OMAP5430_BGAP_DTEMP_GPU_4_OFFSET		0x204

/*                               */
#define OMAP5430_FUSE_OPP_BGAP_MPU			0x4
#define OMAP5430_TEMP_SENSOR_MPU_OFFSET			0x14C
#define OMAP5430_BGAP_THRESHOLD_MPU_OFFSET		0x1A4
#define OMAP5430_BGAP_TSHUT_MPU_OFFSET			0x1B0
#define OMAP5430_BGAP_CUMUL_DTEMP_MPU_OFFSET		0x1BC
#define OMAP5430_BGAP_DTEMP_MPU_0_OFFSET		0x1E0
#define OMAP5430_BGAP_DTEMP_MPU_1_OFFSET		0x1E4
#define OMAP5430_BGAP_DTEMP_MPU_2_OFFSET		0x1E8
#define OMAP5430_BGAP_DTEMP_MPU_3_OFFSET		0x1EC
#define OMAP5430_BGAP_DTEMP_MPU_4_OFFSET		0x1F0

/*                               */
#define OMAP5430_FUSE_OPP_BGAP_CORE			0x8
#define OMAP5430_TEMP_SENSOR_CORE_OFFSET		0x154
#define OMAP5430_BGAP_THRESHOLD_CORE_OFFSET		0x1AC
#define OMAP5430_BGAP_TSHUT_CORE_OFFSET			0x1B8
#define OMAP5430_BGAP_CUMUL_DTEMP_CORE_OFFSET		0x1C4
#define OMAP5430_BGAP_DTEMP_CORE_0_OFFSET		0x208
#define OMAP5430_BGAP_DTEMP_CORE_1_OFFSET		0x20C
#define OMAP5430_BGAP_DTEMP_CORE_2_OFFSET		0x210
#define OMAP5430_BGAP_DTEMP_CORE_3_OFFSET		0x214
#define OMAP5430_BGAP_DTEMP_CORE_4_OFFSET		0x218

/*                                  */
#define OMAP5430_BGAP_CTRL_OFFSET			0x1A0
#define OMAP5430_BGAP_STATUS_OFFSET			0x1C8

/* 
                                  
  
                                                     
                                                       
                       
 */

/*                      */
#define OMAP5430_BGAP_TEMP_SENSOR_SOC_MASK		BIT(12)
#define OMAP5430_BGAP_TEMPSOFF_MASK			BIT(11)
#define OMAP5430_BGAP_TEMP_SENSOR_EOCZ_MASK		BIT(10)
#define OMAP5430_BGAP_TEMP_SENSOR_DTEMP_MASK		(0x3ff << 0)

/*                       */
#define OMAP5430_MASK_SIDLEMODE_MASK			(0x3 << 30)
#define OMAP5430_MASK_COUNTER_DELAY_MASK		(0x7 << 27)
#define OMAP5430_MASK_FREEZE_CORE_MASK			BIT(23)
#define OMAP5430_MASK_FREEZE_GPU_MASK			BIT(22)
#define OMAP5430_MASK_FREEZE_MPU_MASK			BIT(21)
#define OMAP5430_MASK_CLEAR_CORE_MASK			BIT(20)
#define OMAP5430_MASK_CLEAR_GPU_MASK			BIT(19)
#define OMAP5430_MASK_CLEAR_MPU_MASK			BIT(18)
#define OMAP5430_MASK_CLEAR_ACCUM_CORE_MASK		BIT(17)
#define OMAP5430_MASK_CLEAR_ACCUM_GPU_MASK		BIT(16)
#define OMAP5430_MASK_CLEAR_ACCUM_MPU_MASK		BIT(15)
#define OMAP5430_MASK_HOT_CORE_MASK			BIT(5)
#define OMAP5430_MASK_COLD_CORE_MASK			BIT(4)
#define OMAP5430_MASK_HOT_GPU_MASK			BIT(3)
#define OMAP5430_MASK_COLD_GPU_MASK			BIT(2)
#define OMAP5430_MASK_HOT_MPU_MASK			BIT(1)
#define OMAP5430_MASK_COLD_MPU_MASK			BIT(0)

/*                          */
#define OMAP5430_COUNTER_MASK				(0xffffff << 0)

/*                            */
#define OMAP5430_T_HOT_MASK				(0x3ff << 16)
#define OMAP5430_T_COLD_MASK				(0x3ff << 0)

/*                          */
#define OMAP5430_TSHUT_HOT_MASK				(0x3ff << 16)
#define OMAP5430_TSHUT_COLD_MASK			(0x3ff << 0)

/*                                  */
#define OMAP5430_CUMUL_DTEMP_MPU_MASK			(0xffffffff << 0)

/*                                  */
#define OMAP5430_CUMUL_DTEMP_GPU_MASK			(0xffffffff << 0)

/*                                   */
#define OMAP5430_CUMUL_DTEMP_CORE_MASK			(0xffffffff << 0)

/*                         */
#define OMAP5430_BGAP_ALERT_MASK			BIT(31)
#define OMAP5430_HOT_CORE_FLAG_MASK			BIT(5)
#define OMAP5430_COLD_CORE_FLAG_MASK			BIT(4)
#define OMAP5430_HOT_GPU_FLAG_MASK			BIT(3)
#define OMAP5430_COLD_GPU_FLAG_MASK			BIT(2)
#define OMAP5430_HOT_MPU_FLAG_MASK			BIT(1)
#define OMAP5430_COLD_MPU_FLAG_MASK			BIT(0)

/* 
                                                 
  
                                                         
                                                           
                                                       
                         
 */

/*                                         */
/*                             */
#define OMAP5430_ADC_START_VALUE			540
#define OMAP5430_ADC_END_VALUE				945

/*                                      */
/*                      */
#define OMAP5430_GPU_MAX_FREQ				1500000
#define OMAP5430_GPU_MIN_FREQ				1000000
/*               */
#define OMAP5430_GPU_MIN_TEMP				-40000
#define OMAP5430_GPU_MAX_TEMP				125000
#define OMAP5430_GPU_HYST_VAL				5000
/*                       */
#define OMAP5430_GPU_TSHUT_HOT				915
#define OMAP5430_GPU_TSHUT_COLD				900
#define OMAP5430_GPU_T_HOT				800
#define OMAP5430_GPU_T_COLD				795

/*                                      */
/*                      */
#define OMAP5430_MPU_MAX_FREQ				1500000
#define OMAP5430_MPU_MIN_FREQ				1000000
/*               */
#define OMAP5430_MPU_MIN_TEMP				-40000
#define OMAP5430_MPU_MAX_TEMP				125000
#define OMAP5430_MPU_HYST_VAL				5000
/*                       */
#define OMAP5430_MPU_TSHUT_HOT				915
#define OMAP5430_MPU_TSHUT_COLD				900
#define OMAP5430_MPU_T_HOT				800
#define OMAP5430_MPU_T_COLD				795

/*                                       */
/*                      */
#define OMAP5430_CORE_MAX_FREQ				1500000
#define OMAP5430_CORE_MIN_FREQ				1000000
/*               */
#define OMAP5430_CORE_MIN_TEMP				-40000
#define OMAP5430_CORE_MAX_TEMP				125000
#define OMAP5430_CORE_HYST_VAL				5000
/*                       */
#define OMAP5430_CORE_TSHUT_HOT				915
#define OMAP5430_CORE_TSHUT_COLD			900
#define OMAP5430_CORE_T_HOT				800
#define OMAP5430_CORE_T_COLD				795

#endif /*                      */
