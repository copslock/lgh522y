/*
 * DA9055 declarations for DA9055 PMICs.
 *
 * Copyright(c) 2012 Dialog Semiconductor Ltd.
 *
 * Author: David Dajun Chen <dchen@diasemi.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef __DA9055_REG_H
#define __DA9055_REG_H

/*
                 
 */
 /*       */
#define	DA9055_REG_PAGE_CON		0x00

/*                                    */
#define	DA9055_REG_STATUS_A		0x01
#define	DA9055_REG_STATUS_B		0x02
#define	DA9055_REG_FAULT_LOG		0x03
#define	DA9055_REG_EVENT_A		0x04
#define	DA9055_REG_EVENT_B		0x05
#define	DA9055_REG_EVENT_C		0x06
#define	DA9055_REG_IRQ_MASK_A		0x07
#define	DA9055_REG_IRQ_MASK_B		0x08
#define	DA9055_REG_IRQ_MASK_C		0x09
#define	DA9055_REG_CONTROL_A		0x0A
#define	DA9055_REG_CONTROL_B		0x0B
#define	DA9055_REG_CONTROL_C		0x0C
#define	DA9055_REG_CONTROL_D		0x0D
#define	DA9055_REG_CONTROL_E		0x0E
#define	DA9055_REG_PD_DIS		0x0F

/*                        */
#define	DA9055_REG_GPIO0_1		0x10
#define	DA9055_REG_GPIO2		0x11
#define	DA9055_REG_GPIO_MODE0_2		0x12

/*                             */
#define	DA9055_REG_BCORE_CONT		0x13
#define	DA9055_REG_BMEM_CONT		0x14
#define	DA9055_REG_LDO1_CONT		0x15
#define	DA9055_REG_LDO2_CONT		0x16
#define	DA9055_REG_LDO3_CONT		0x17
#define	DA9055_REG_LDO4_CONT		0x18
#define	DA9055_REG_LDO5_CONT		0x19
#define	DA9055_REG_LDO6_CONT		0x1A

/*                          */
#define	DA9055_REG_ADC_MAN		0x1B
#define	DA9055_REG_ADC_CONT		0x1C
#define	DA9055_REG_VSYS_MON		0x1D
#define	DA9055_REG_ADC_RES_L		0x1E
#define	DA9055_REG_ADC_RES_H		0x1F
#define	DA9055_REG_VSYS_RES		0x20
#define	DA9055_REG_ADCIN1_RES		0x21
#define	DA9055_REG_ADCIN2_RES		0x22
#define	DA9055_REG_ADCIN3_RES		0x23

/*                             */
#define	DA9055_REG_EN_32K		0x35

/*                             */
#define	DA9055_REG_BUCK_LIM		0x37
#define	DA9055_REG_BCORE_MODE		0x38
#define	DA9055_REG_VBCORE_A		0x39
#define	DA9055_REG_VBMEM_A		0x3A
#define	DA9055_REG_VLDO1_A		0x3B
#define	DA9055_REG_VLDO2_A		0x3C
#define	DA9055_REG_VLDO3_A		0x3D
#define	DA9055_REG_VLDO4_A		0x3E
#define	DA9055_REG_VLDO5_A		0x3F
#define	DA9055_REG_VLDO6_A		0x40
#define	DA9055_REG_VBCORE_B		0x41
#define	DA9055_REG_VBMEM_B		0x42
#define	DA9055_REG_VLDO1_B		0x43
#define	DA9055_REG_VLDO2_B		0x44
#define	DA9055_REG_VLDO3_B		0x45
#define	DA9055_REG_VLDO4_B		0x46
#define	DA9055_REG_VLDO5_B		0x47
#define	DA9055_REG_VLDO6_B		0x48

/*                            */
#define	DA9055_REG_AUTO1_HIGH		0x49
#define	DA9055_REG_AUTO1_LOW		0x4A
#define	DA9055_REG_AUTO2_HIGH		0x4B
#define	DA9055_REG_AUTO2_LOW		0x4C
#define	DA9055_REG_AUTO3_HIGH		0x4D
#define	DA9055_REG_AUTO3_LOW		0x4E

/*     */
#define	DA9055_REG_OPT_COUNT		0x50
#define	DA9055_REG_OPT_ADDR		0x51
#define	DA9055_REG_OPT_DATA		0x52

/*                                  */
#define	DA9055_REG_COUNT_S		0x53
#define	DA9055_REG_COUNT_MI		0x54
#define	DA9055_REG_COUNT_H		0x55
#define	DA9055_REG_COUNT_D		0x56
#define	DA9055_REG_COUNT_MO		0x57
#define	DA9055_REG_COUNT_Y		0x58
#define	DA9055_REG_ALARM_MI		0x59
#define	DA9055_REG_ALARM_H		0x5A
#define	DA9055_REG_ALARM_D		0x5B
#define	DA9055_REG_ALARM_MO		0x5C
#define	DA9055_REG_ALARM_Y		0x5D
#define	DA9055_REG_SECOND_A		0x5E
#define	DA9055_REG_SECOND_B		0x5F
#define	DA9055_REG_SECOND_C		0x60
#define	DA9055_REG_SECOND_D		0x61

/*                                 */
#define	DA9055_REG_T_OFFSET		0x63
#define	DA9055_REG_INTERFACE		0x64
#define	DA9055_REG_CONFIG_A		0x65
#define	DA9055_REG_CONFIG_B		0x66
#define	DA9055_REG_CONFIG_C		0x67
#define	DA9055_REG_CONFIG_D		0x68
#define	DA9055_REG_CONFIG_E		0x69
#define	DA9055_REG_TRIM_CLDR		0x6F

/*                           */
#define	DA9055_REG_GP_ID_0		0x70
#define	DA9055_REG_GP_ID_1		0x71
#define	DA9055_REG_GP_ID_2		0x72
#define	DA9055_REG_GP_ID_3		0x73
#define	DA9055_REG_GP_ID_4		0x74
#define	DA9055_REG_GP_ID_5		0x75
#define	DA9055_REG_GP_ID_6		0x76
#define	DA9055_REG_GP_ID_7		0x77
#define	DA9055_REG_GP_ID_8		0x78
#define	DA9055_REG_GP_ID_9		0x79
#define	DA9055_REG_GP_ID_10		0x7A
#define	DA9055_REG_GP_ID_11		0x7B
#define	DA9055_REG_GP_ID_12		0x7C
#define	DA9055_REG_GP_ID_13		0x7D
#define	DA9055_REG_GP_ID_14		0x7E
#define	DA9055_REG_GP_ID_15		0x7F
#define	DA9055_REG_GP_ID_16		0x80
#define	DA9055_REG_GP_ID_17		0x81
#define	DA9055_REG_GP_ID_18		0x82
#define	DA9055_REG_GP_ID_19		0x83

#define DA9055_MAX_REGISTER_CNT		DA9055_REG_GP_ID_19

/*
                      
 */

/*                                 */
#define	DA9055_PAGE_WRITE_MODE		(0<<6)
#define	DA9055_REPEAT_WRITE_MODE	(1<<6)

/*                                 */
#define	DA9055_NOKEY_STS		0x01
#define	DA9055_WAKE_STS			0x02
#define	DA9055_DVC_BUSY_STS		0x04
#define	DA9055_COMP1V2_STS		0x08
#define	DA9055_NJIG_STS			0x10
#define	DA9055_LDO5_LIM_STS		0x20
#define	DA9055_LDO6_LIM_STS		0x40

/*                                 */
#define	DA9055_GPI0_STS			0x01
#define	DA9055_GPI1_STS			0x02
#define	DA9055_GPI2_STS			0x04

/*                                  */
#define	DA9055_TWD_ERROR_FLG		0x01
#define	DA9055_POR_FLG			0x02
#define	DA9055_VDD_FAULT_FLG		0x04
#define	DA9055_VDD_START_FLG		0x08
#define	DA9055_TEMP_CRIT_FLG		0x10
#define	DA9055_KEY_RESET_FLG		0x20
#define	DA9055_WAIT_SHUT_FLG		0x80

/*                                */
#define	DA9055_NOKEY_EINT		0x01
#define	DA9055_ALARM_EINT		0x02
#define	DA9055_TICK_EINT		0x04
#define	DA9055_ADC_RDY_EINT		0x08
#define	DA9055_SEQ_RDY_EINT		0x10
#define	DA9055_EVENTS_B_EINT		0x20
#define	DA9055_EVENTS_C_EINT		0x40

/*                                */
#define	DA9055_E_WAKE_EINT		0x01
#define	DA9055_E_TEMP_EINT		0x02
#define	DA9055_E_COMP1V2_EINT		0x04
#define	DA9055_E_LDO_LIM_EINT		0x08
#define	DA9055_E_NJIG_EINT		0x20
#define	DA9055_E_VDD_MON_EINT		0x40
#define	DA9055_E_VDD_WARN_EINT		0x80

/*                                */
#define	DA9055_E_GPI0_EINT		0x01
#define	DA9055_E_GPI1_EINT		0x02
#define	DA9055_E_GPI2_EINT		0x04

/*                                   */
#define	DA9055_M_NONKEY_EINT		0x01
#define	DA9055_M_ALARM_EINT		0x02
#define	DA9055_M_TICK_EINT		0x04
#define	DA9055_M_ADC_RDY_EINT		0x08
#define	DA9055_M_SEQ_RDY_EINT		0x10

/*                                   */
#define	DA9055_M_WAKE_EINT		0x01
#define	DA9055_M_TEMP_EINT		0x02
#define	DA9055_M_COMP_1V2_EINT		0x04
#define	DA9055_M_LDO_LIM_EINT		0x08
#define	DA9055_M_NJIG_EINT		0x20
#define	DA9055_M_VDD_MON_EINT		0x40
#define	DA9055_M_VDD_WARN_EINT		0x80

/*                                   */
#define	DA9055_M_GPI0_EINT		0x01
#define	DA9055_M_GPI1_EINT		0x02
#define	DA9055_M_GPI2_EINT		0x04

/*                                 */
#define	DA9055_DEBOUNCING_SHIFT		0x00
#define	DA9055_DEBOUNCING_MASK		0x07
#define	DA9055_NRES_MODE_SHIFT		0x03
#define	DA9055_NRES_MODE_MASK		0x08
#define	DA9055_SLEW_RATE_SHIFT		0x04
#define	DA9055_SLEW_RATE_MASK		0x30
#define	DA9055_NOKEY_LOCK_SHIFT		0x06
#define	DA9055_NOKEY_LOCK_MASK		0x40

/*                                 */
#define	DA9055_RTC_MODE_PD		0x01
#define	DA9055_RTC_MODE_SD_SHIFT	0x01
#define	DA9055_RTC_MODE_SD		0x02
#define	DA9055_RTC_EN			0x04
#define	DA9055_ECO_MODE_SHIFT		0x03
#define	DA9055_ECO_MODE_MASK		0x08
#define	DA9055_TWDSCALE_SHIFT		4
#define	DA9055_TWDSCALE_MASK		0x70
#define	DA9055_V_LOCK_SHIFT		0x07
#define	DA9055_V_LOCK_MASK		0x80

/*                                 */
#define	DA9055_SYSTEM_EN_SHIFT		0x00
#define	DA9055_SYSTEM_EN_MASK		0x01
#define	DA9055_POWERN_EN_SHIFT		0x01
#define	DA9055_POWERN_EN_MASK		0x02
#define	DA9055_POWER1_EN_SHIFT		0x02
#define	DA9055_POWER1_EN_MASK		0x04

/*                                 */
#define	DA9055_STANDBY_SHIFT		0x02
#define	DA9055_STANDBY_MASK		0x08
#define	DA9055_AUTO_BOOT_SHIFT		0x03
#define	DA9055_AUTO_BOOT_MASK		0x04

/*                                 */
#define	DA9055_WATCHDOG_SHIFT		0x00
#define	DA9055_WATCHDOG_MASK		0x01
#define	DA9055_SHUTDOWN_SHIFT		0x01
#define	DA9055_SHUTDOWN_MASK		0x02
#define	DA9055_WAKE_UP_SHIFT		0x02
#define	DA9055_WAKE_UP_MASK		0x04

/*                                  */
#define	DA9055_GPIO0_PIN_SHIFT		0x00
#define	DA9055_GPIO0_PIN_MASK		0x03
#define	DA9055_GPIO0_TYPE_SHIFT		0x02
#define	DA9055_GPIO0_TYPE_MASK		0x04
#define	DA9055_GPIO0_WEN_SHIFT		0x03
#define	DA9055_GPIO0_WEN_MASK		0x08
#define	DA9055_GPIO1_PIN_SHIFT		0x04
#define	DA9055_GPIO1_PIN_MASK		0x30
#define	DA9055_GPIO1_TYPE_SHIFT		0x06
#define	DA9055_GPIO1_TYPE_MASK		0x40
#define	DA9055_GPIO1_WEN_SHIFT		0x07
#define	DA9055_GPIO1_WEN_MASK		0x80
#define	DA9055_GPIO2_PIN_SHIFT		0x00
#define	DA9055_GPIO2_PIN_MASK		0x30
#define	DA9055_GPIO2_TYPE_SHIFT		0x02
#define	DA9055_GPIO2_TYPE_MASK		0x04
#define	DA9055_GPIO2_WEN_SHIFT		0x03
#define	DA9055_GPIO2_WEN_MASK		0x08

/*                                  */
#define	DA9055_GPIO0_MODE_SHIFT		0x00
#define	DA9055_GPIO0_MODE_MASK		0x01
#define	DA9055_GPIO1_MODE_SHIFT		0x01
#define	DA9055_GPIO1_MODE_MASK		0x02
#define	DA9055_GPIO2_MODE_SHIFT		0x02
#define	DA9055_GPIO2_MODE_MASK		0x04

/*                                   */
#define	DA9055_BCORE_EN_SHIFT		0x00
#define	DA9055_BCORE_EN_MASK		0x01
#define	DA9055_BCORE_GPI_SHIFT		0x01
#define	DA9055_BCORE_GPI_MASK		0x02
#define	DA9055_BCORE_PD_DIS_SHIFT	0x03
#define	DA9055_BCORE_PD_DIS_MASK	0x04
#define	DA9055_VBCORE_SEL_SHIFT		0x04
#define	DA9055_SEL_REG_A		0x0
#define	DA9055_SEL_REG_B		0x10
#define	DA9055_VBCORE_SEL_MASK		0x10
#define DA9055_V_GPI_MASK		0x60
#define DA9055_V_GPI_SHIFT		0x05
#define DA9055_E_GPI_MASK		0x06
#define DA9055_E_GPI_SHIFT		0x01
#define	DA9055_VBCORE_GPI_SHIFT		0x05
#define	DA9055_VBCORE_GPI_MASK		0x60
#define	DA9055_BCORE_CONF_SHIFT		0x07
#define	DA9055_BCORE_CONF_MASK		0x80

/*                                  */
#define	DA9055_BMEM_EN_SHIFT		0x00
#define	DA9055_BMEM_EN_MASK		0x01
#define	DA9055_BMEM_GPI_SHIFT		0x01
#define	DA9055_BMEM_GPI_MASK		0x06
#define	DA9055_BMEM_PD_DIS_SHIFT	0x03
#define	DA9055_BMEM_PD_DIS_MASK		0x08
#define	DA9055_VBMEM_SEL_SHIT		0x04
#define	DA9055_VBMEM_SEL_VBMEM_A	(0<<4)
#define	DA9055_VBMEM_SEL_VBMEM_B	(1<<4)
#define	DA9055_VBMEM_SEL_MASK		0x10
#define	DA9055_VBMEM_GPI_SHIFT		0x05
#define	DA9055_VBMEM_GPI_MASK		0x60
#define	DA9055_BMEM_CONF_SHIFT		0x07
#define	DA9055_BMEM_CONF_MASK		0x80

/*                                      */
#define	DA9055_LDO_EN_SHIFT		0x00
#define	DA9055_LDO_EN_MASK		0x01
#define	DA9055_LDO_GPI_SHIFT		0x01
#define	DA9055_LDO_GPI_MASK		0x06
#define	DA9055_LDO_PD_DIS_SHIFT		0x03
#define	DA9055_LDO_PD_DIS_MASK		0x08
#define	DA9055_VLDO_SEL_SHIFT		0x04
#define	DA9055_VLDO_SEL_MASK		0x10
#define	DA9055_VLDO_SEL_VLDO_A		0x00
#define	DA9055_VLDO_SEL_VLDO_B		0x01
#define	DA9055_VLDO_GPI_SHIFT		0x05
#define	DA9055_VLDO_GPI_MASK		0x60
#define	DA9055_LDO_CONF_SHIFT		0x07
#define	DA9055_LDO_CONF_MASK		0x80
#define	DA9055_REGUALTOR_SET_A		0x00
#define	DA9055_REGUALTOR_SET_B		0x10

/*                                */
#define	DA9055_ADC_MUX_SHIFT		0
#define	DA9055_ADC_MUX_MASK		0xF
#define	DA9055_ADC_MUX_VSYS		0x0
#define	DA9055_ADC_MUX_ADCIN1		0x01
#define	DA9055_ADC_MUX_ADCIN2		0x02
#define	DA9055_ADC_MUX_ADCIN3		0x03
#define	DA9055_ADC_MUX_T_SENSE		0x04
#define	DA9055_ADC_MAN_SHIFT		0x04
#define	DA9055_ADC_MAN_CONV		0x10
#define DA9055_ADC_LSB_MASK		0X03
#define DA9055_ADC_MODE_MASK		0x20
#define	DA9055_ADC_MODE_SHIFT		5
#define	DA9055_ADC_MODE_1MS		(1<<5)
#define	DA9055_COMP1V2_EN_SHIFT		7

/*                                 */
#define	DA9055_ADC_AUTO_VSYS_EN_SHIFT	0
#define	DA9055_ADC_AUTO_AD1_EN_SHIFT	1
#define	DA9055_ADC_AUTO_AD2_EN_SHIFT	2
#define	DA9055_ADC_AUTO_AD3_EN_SHIFT	3
#define	DA9055_ADC_ISRC_EN_SHIFT	4
#define	DA9055_ADC_ADCIN1_DEB_SHIFT	5
#define	DA9055_ADC_ADCIN2_DEB_SHIFT	6
#define	DA9055_ADC_ADCIN3_DEB_SHIFT	7
#define DA9055_AD1_ISRC_MASK		0x10
#define DA9055_AD1_ISRC_SHIFT		4

/*                                 */
#define	DA9055_VSYS_VAL_SHIFT		0
#define	DA9055_VSYS_VAL_MASK		0xFF
#define	DA9055_VSYS_VAL_BASE		0x00
#define	DA9055_VSYS_VAL_MAX		DA9055_VSYS_VAL_MASK
#define	DA9055_VSYS_VOLT_BASE		2500
#define	DA9055_VSYS_VOLT_INC		10
#define	DA9055_VSYS_STEPS		255
#define	DA9055_VSYS_VOLT_MIN		2500

/*                                     */
#define	DA9055_ADC_VAL_SHIFT		0
#define	DA9055_ADC_VAL_MASK		0xFF
#define	DA9055_ADC_VAL_BASE		0x00
#define	DA9055_ADC_VAL_MAX		DA9055_ADC_VAL_MASK
#define	DA9055_ADC_VOLT_BASE		0
#define	DA9055_ADC_VSYS_VOLT_BASE	2500
#define	DA9055_ADC_VOLT_INC		10
#define	DA9055_ADC_VSYS_VOLT_INC	12
#define	DA9055_ADC_STEPS		255

/*                               */
#define	DA9055_STARTUP_TIME_MASK	0x07
#define	DA9055_STARTUP_TIME_0S		0x0
#define	DA9055_STARTUP_TIME_0_52S	0x1
#define	DA9055_STARTUP_TIME_1S		0x2
#define	DA9055_CRYSTAL_EN		0x08
#define	DA9055_DELAY_MODE_EN		0x10
#define	DA9055_OUT_CLCK_GATED		0x20
#define	DA9055_RTC_CLOCK_GATED		0x40
#define	DA9055_EN_32KOUT_BUF		0x80

/*                              */
/*                       */
#define	DA9055_RESET_TIMER_VAL_SHIFT	0
#define	DA9055_RESET_LOW_VAL_MASK	0x3F
#define	DA9055_RESET_LOW_VAL_BASE	0
#define	DA9055_RESET_LOW_VAL_MAX	DA9055_RESET_LOW_VAL_MASK
#define	DA9055_RESET_US_LOW_BASE	1024 /*                        */
#define	DA9055_RESET_US_LOW_INC		1024 /*                        */
#define	DA9055_RESET_US_LOW_STEP	30

/*                        */
#define	DA9055_RESET_HIGH_VAL_MASK	0x3F
#define	DA9055_RESET_HIGH_VAL_BASE	0
#define	DA9055_RESET_HIGH_VAL_MAX	DA9055_RESET_HIGH_VAL_MASK
#define	DA9055_RESET_US_HIGH_BASE	32768 /*                        */
#define	DA9055_RESET_US_HIGH_INC	32768 /*                        */
#define	DA9055_RESET_US_HIGH_STEP	31

/*                                 */
#define	DA9055_BMEM_ILIM_SHIFT		0
#define	DA9055_ILIM_MASK		0x3
#define	DA9055_ILIM_500MA		0x0
#define	DA9055_ILIM_600MA		0x1
#define	DA9055_ILIM_700MA		0x2
#define	DA9055_ILIM_800MA		0x3
#define	DA9055_BCORE_ILIM_SHIFT		2

/*                                   */
#define	DA9055_BMEM_MODE_SHIFT		0
#define	DA9055_MODE_MASK		0x3
#define	DA9055_MODE_AB			0x0
#define	DA9055_MODE_SLEEP		0x1
#define	DA9055_MODE_SYNCHRO		0x2
#define	DA9055_MODE_AUTO		0x3
#define	DA9055_BCORE_MODE_SHIFT		2

/*                                       */
#define	DA9055_VBCORE_VAL_SHIFT		0
#define	DA9055_VBCORE_VAL_MASK		0x3F
#define	DA9055_VBCORE_VAL_BASE		0x09
#define	DA9055_VBCORE_VAL_MAX		DA9055_VBCORE_VAL_MASK
#define	DA9055_VBCORE_VOLT_BASE		750
#define	DA9055_VBCORE_VOLT_INC		25
#define	DA9055_VBCORE_STEPS		53
#define	DA9055_VBCORE_VOLT_MIN		DA9055_VBCORE_VOLT_BASE
#define	DA9055_BCORE_SL_SYNCHRO		(0<<7)
#define	DA9055_BCORE_SL_SLEEP		(1<<7)

/*                                      */
#define	DA9055_VBMEM_VAL_SHIFT		0
#define	DA9055_VBMEM_VAL_MASK		0x3F
#define	DA9055_VBMEM_VAL_BASE		0x00
#define	DA9055_VBMEM_VAL_MAX		DA9055_VBMEM_VAL_MASK
#define	DA9055_VBMEM_VOLT_BASE		925
#define	DA9055_VBMEM_VOLT_INC		25
#define	DA9055_VBMEM_STEPS		63
#define	DA9055_VBMEM_VOLT_MIN		DA9055_VBMEM_VOLT_BASE
#define	DA9055_BCMEM_SL_SYNCHRO		(0<<7)
#define	DA9055_BCMEM_SL_SLEEP		(1<<7)


/*                                           */
#define	DA9055_VLDO_VAL_SHIFT		0
#define	DA9055_VLDO_VAL_MASK		0x3F
#define	DA9055_VLDO6_VAL_MASK		0x7F
#define	DA9055_VLDO_VAL_BASE		0x02
#define	DA9055_VLDO2_VAL_BASE		0x03
#define	DA9055_VLDO6_VAL_BASE		0x00
#define	DA9055_VLDO_VAL_MAX		DA9055_VLDO_VAL_MASK
#define	DA9055_VLDO6_VAL_MAX		DA9055_VLDO6_VAL_MASK
#define	DA9055_VLDO_VOLT_BASE		900
#define	DA9055_VLDO_VOLT_INC		50
#define	DA9055_VLDO6_VOLT_INC		20
#define	DA9055_VLDO_STEPS		48
#define	DA9055_VLDO5_STEPS		37
#define	DA9055_VLDO6_STEPS		120
#define	DA9055_VLDO_VOLT_MIN		DA9055_VLDO_VOLT_BASE
#define	DA9055_LDO_MODE_SHIFT		7
#define	DA9055_LDO_SL_NORMAL		0
#define	DA9055_LDO_SL_SLEEP		1

/*                                 */
#define	DA9055_OTP_TIM_NORMAL		(0<<0)
#define	DA9055_OTP_TIM_MARGINAL		(1<<0)
#define	DA9055_OTP_GP_RD_SHIFT		1
#define	DA9055_OTP_APPS_RD_SHIFT	2
#define	DA9055_PC_DONE_SHIFT		3
#define	DA9055_OTP_GP_LOCK_SHIFT	4
#define	DA9055_OTP_APPS_LOCK_SHIFT	5
#define	DA9055_OTP_CONF_LOCK_SHIFT	6
#define	DA9055_OTP_WRITE_DIS_SHIFT	7

/*                                */
#define	DA9055_RTC_SEC			0x3F
#define	DA9055_RTC_MONITOR_EN		0x40
#define	DA9055_RTC_READ			0x80

/*                                 */
#define	DA9055_RTC_MIN			0x3F

/*                                */
#define	DA9055_RTC_HOUR			0x1F

/*                                */
#define	DA9055_RTC_DAY			0x1F

/*                                 */
#define	DA9055_RTC_MONTH		0x0F

/*                                */
#define	DA9055_RTC_YEAR			0x3F
#define	DA9055_RTC_YEAR_BASE		2000

/*                                 */
#define	DA9055_RTC_ALM_MIN		0x3F
#define	DA9055_ALARM_STATUS_SHIFT	6
#define	DA9055_ALARM_STATUS_MASK	0x3
#define	DA9055_ALARM_STATUS_NO_ALARM	0x0
#define	DA9055_ALARM_STATUS_TICK	0x1
#define	DA9055_ALARM_STATUS_TIMER_ALARM	0x2
#define	DA9055_ALARM_STATUS_BOTH	0x3

/*                                */
#define	DA9055_RTC_ALM_HOUR		0x1F

/*                                */
#define	DA9055_RTC_ALM_DAY		0x1F

/*                                 */
#define	DA9055_RTC_ALM_MONTH		0x0F
#define	DA9055_RTC_TICK_WAKE_MASK	0x20
#define	DA9055_RTC_TICK_WAKE_SHIFT	5
#define	DA9055_RTC_TICK_TYPE		0x10
#define	DA9055_RTC_TICK_TYPE_SHIFT	0x4
#define	DA9055_RTC_TICK_SEC		0x0
#define	DA9055_RTC_TICK_MIN		0x1
#define	DA9055_ALARAM_TICK_WAKE		0x20

/*                                */
#define	DA9055_RTC_TICK_EN		0x80
#define	DA9055_RTC_ALM_EN		0x40
#define	DA9055_RTC_TICK_ALM_MASK	0xC0
#define	DA9055_RTC_ALM_YEAR		0x3F

/*                                  */
#define	DA9055_TRIM_32K_SHIFT		0
#define	DA9055_TRIM_32K_MASK		0x7F
#define	DA9055_TRIM_DECREMENT		(1<<7)
#define	DA9055_TRIM_INCREMENT		(0<<7)
#define	DA9055_TRIM_VAL_BASE		0x0
#define	DA9055_TRIM_PPM_BASE		0x0 /*                            */
#define	DA9055_TRIM_PPM_INC		19 /*                            */
#define	DA9055_TRIM_STEPS		127

/*                                 */
#define	DA9055_PM_I_V_VDDCORE		(0<<0)
#define	DA9055_PM_I_V_VDD_IO		(1<<0)
#define	DA9055_VDD_FAULT_TYPE_ACT_LOW	(0<<1)
#define	DA9055_VDD_FAULT_TYPE_ACT_HIGH	(1<<1)
#define	DA9055_PM_O_TYPE_PUSH_PULL	(0<<2)
#define	DA9055_PM_O_TYPE_OPEN_DRAIN	(1<<2)
#define	DA9055_IRQ_TYPE_ACT_LOW		(0<<3)
#define	DA9055_IRQ_TYPE_ACT_HIGH	(1<<3)
#define	DA9055_NIRQ_MODE_IMM		(0<<4)
#define	DA9055_NIRQ_MODE_ACTIVE		(1<<4)
#define	DA9055_GPI_V_VDDCORE		(0<<5)
#define	DA9055_GPI_V_VDD_IO		(1<<5)
#define	DA9055_PM_IF_V_VDDCORE		(0<<6)
#define	DA9055_PM_IF_V_VDD_IO		(1<<6)

/*                                 */
#define	DA9055_VDD_FAULT_VAL_SHIFT	0
#define	DA9055_VDD_FAULT_VAL_MASK	0xF
#define	DA9055_VDD_FAULT_VAL_BASE	0x0
#define	DA9055_VDD_FAULT_VAL_MAX	DA9055_VDD_FAULT_VAL_MASK
#define	DA9055_VDD_FAULT_VOLT_BASE	2500
#define	DA9055_VDD_FAULT_VOLT_INC	50
#define	DA9055_VDD_FAULT_STEPS		15

#define	DA9055_VDD_HYST_VAL_SHIFT	4
#define	DA9055_VDD_HYST_VAL_MASK	0x7
#define	DA9055_VDD_HYST_VAL_BASE	0x0
#define	DA9055_VDD_HYST_VAL_MAX		DA9055_VDD_HYST_VAL_MASK
#define	DA9055_VDD_HYST_VOLT_BASE	100
#define	DA9055_VDD_HYST_VOLT_INC	50
#define	DA9055_VDD_HYST_STEPS		7
#define	DA9055_VDD_HYST_VOLT_MIN	DA9055_VDD_HYST_VOLT_BASE

#define	DA9055_VDD_FAULT_EN_SHIFT	7

/*                                 */
#define	DA9055_BCORE_CLK_INV_SHIFT	0
#define	DA9055_BMEM_CLK_INV_SHIFT	1
#define	DA9055_NFAULT_CONF_SHIFT	2
#define	DA9055_LDO_SD_SHIFT		4
#define	DA9055_LDO5_BYP_SHIFT		6
#define	DA9055_LDO6_BYP_SHIFT		7

/*                                 */
#define	DA9055_NONKEY_PIN_SHIFT		0
#define	DA9055_NONKEY_PIN_MASK		0x3
#define	DA9055_NONKEY_PIN_PORT_MODE	0x0
#define	DA9055_NONKEY_PIN_KEY_MODE	0x1
#define	DA9055_NONKEY_PIN_MULTI_FUNC	0x2
#define	DA9055_NONKEY_PIN_DEDICT	0x3
#define	DA9055_NONKEY_SD_SHIFT		2
#define	DA9055_KEY_DELAY_SHIFT		3
#define	DA9055_KEY_DELAY_MASK		0x3
#define	DA9055_KEY_DELAY_4S		0x0
#define	DA9055_KEY_DELAY_6S		0x1
#define	DA9055_KEY_DELAY_8S		0x2
#define	DA9055_KEY_DELAY_10S		0x3

/*                                 */
#define	DA9055_GPIO_PUPD_PULL_UP	0x0
#define	DA9055_GPIO_PUPD_OPEN_DRAIN	0x1
#define	DA9055_GPIO0_PUPD_SHIFT		0
#define	DA9055_GPIO1_PUPD_SHIFT		1
#define	DA9055_GPIO2_PUPD_SHIFT		2
#define	DA9055_UVOV_DELAY_SHIFT		4
#define	DA9055_UVOV_DELAY_MASK		0x3
#define	DA9055_RESET_DURATION_SHIFT	6
#define	DA9055_RESET_DURATION_MASK	0x3
#define	DA9055_RESET_DURATION_0MS	0x0
#define	DA9055_RESET_DURATION_100MS	0x1
#define	DA9055_RESET_DURATION_500MS	0x2
#define	DA9055_RESET_DURATION_1000MS	0x3

/*                                  */
#define	DA9055_MON_THRES_SHIFT		0
#define	DA9055_MON_THRES_MASK		0x3
#define	DA9055_MON_RES_SHIFT		2
#define	DA9055_MON_DEB_SHIFT		3
#define	DA9055_MON_MODE_SHIFT		4
#define	DA9055_MON_MODE_MASK		0x3
#define	DA9055_START_MAX_SHIFT		6
#define	DA9055_START_MAX_MASK		0x3

/*                                  */
#define	DA9055_LDO1_MON_EN_SHIFT	0
#define	DA9055_LDO2_MON_EN_SHIFT	1
#define	DA9055_LDO3_MON_EN_SHIFT	2
#define	DA9055_LDO4_MON_EN_SHIFT	3
#define	DA9055_LDO5_MON_EN_SHIFT	4
#define	DA9055_LDO6_MON_EN_SHIFT	5
#define	DA9055_BCORE_MON_EN_SHIFT	6
#define	DA9055_BMEM_MON_EN_SHIFT	7

/*                                 */
#define	DA9055_LDO1_DEF_SHIFT		0
#define	DA9055_LDO2_DEF_SHIFT		1
#define	DA9055_LDO3_DEF_SHIFT		2
#define	DA9055_LDO4_DEF_SHIFT		3
#define	DA9055_LDO5_DEF_SHIFT		4
#define	DA9055_LDO6_DEF_SHIFT		5
#define	DA9055_BCORE_DEF_SHIFT		6
#define	DA9055_BMEM_DEF_SHIFT		7

/*                                  */
#define	DA9055_MON_A8_IDX_SHIFT		0
#define	DA9055_MON_A89_IDX_MASK		0x3
#define	DA9055_MON_A89_IDX_NONE		0x0
#define	DA9055_MON_A89_IDX_BUCKCORE	0x1
#define	DA9055_MON_A89_IDX_LDO3		0x2
#define	DA9055_MON_A9_IDX_SHIFT		5

/*                                  */
#define	DA9055_MON_A10_IDX_SHIFT	0
#define	DA9055_MON_A10_IDX_MASK		0x3
#define	DA9055_MON_A10_IDX_NONE		0x0
#define	DA9055_MON_A10_IDX_LDO1		0x1
#define	DA9055_MON_A10_IDX_LDO2		0x2
#define	DA9055_MON_A10_IDX_LDO5		0x3
#define	DA9055_MON_A10_IDX_LDO6		0x4

#endif /*                */
