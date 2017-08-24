/*
 * Functions to access MAX8907 power management chip.
 *
 * Copyright (C) 2010 Gyungoh Yoo <jack.yoo@maxim-ic.com>
 * Copyright (C) 2012, NVIDIA CORPORATION. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __LINUX_MFD_MAX8907_H
#define __LINUX_MFD_MAX8907_H

#include <linux/mutex.h>
#include <linux/pm.h>

#define MAX8907_GEN_I2C_ADDR		(0x78 >> 1)
#define MAX8907_ADC_I2C_ADDR		(0x8e >> 1)
#define MAX8907_RTC_I2C_ADDR		(0xd0 >> 1)

/*                      */
#define MAX8907_REG_SYSENSEL		0x00
#define MAX8907_REG_ON_OFF_IRQ1		0x01
#define MAX8907_REG_ON_OFF_IRQ1_MASK	0x02
#define MAX8907_REG_ON_OFF_STAT		0x03
#define MAX8907_REG_SDCTL1		0x04
#define MAX8907_REG_SDSEQCNT1		0x05
#define MAX8907_REG_SDV1		0x06
#define MAX8907_REG_SDCTL2		0x07
#define MAX8907_REG_SDSEQCNT2		0x08
#define MAX8907_REG_SDV2		0x09
#define MAX8907_REG_SDCTL3		0x0A
#define MAX8907_REG_SDSEQCNT3		0x0B
#define MAX8907_REG_SDV3		0x0C
#define MAX8907_REG_ON_OFF_IRQ2		0x0D
#define MAX8907_REG_ON_OFF_IRQ2_MASK	0x0E
#define MAX8907_REG_RESET_CNFG		0x0F
#define MAX8907_REG_LDOCTL16		0x10
#define MAX8907_REG_LDOSEQCNT16		0x11
#define MAX8907_REG_LDO16VOUT		0x12
#define MAX8907_REG_SDBYSEQCNT		0x13
#define MAX8907_REG_LDOCTL17		0x14
#define MAX8907_REG_LDOSEQCNT17		0x15
#define MAX8907_REG_LDO17VOUT		0x16
#define MAX8907_REG_LDOCTL1		0x18
#define MAX8907_REG_LDOSEQCNT1		0x19
#define MAX8907_REG_LDO1VOUT		0x1A
#define MAX8907_REG_LDOCTL2		0x1C
#define MAX8907_REG_LDOSEQCNT2		0x1D
#define MAX8907_REG_LDO2VOUT		0x1E
#define MAX8907_REG_LDOCTL3		0x20
#define MAX8907_REG_LDOSEQCNT3		0x21
#define MAX8907_REG_LDO3VOUT		0x22
#define MAX8907_REG_LDOCTL4		0x24
#define MAX8907_REG_LDOSEQCNT4		0x25
#define MAX8907_REG_LDO4VOUT		0x26
#define MAX8907_REG_LDOCTL5		0x28
#define MAX8907_REG_LDOSEQCNT5		0x29
#define MAX8907_REG_LDO5VOUT		0x2A
#define MAX8907_REG_LDOCTL6		0x2C
#define MAX8907_REG_LDOSEQCNT6		0x2D
#define MAX8907_REG_LDO6VOUT		0x2E
#define MAX8907_REG_LDOCTL7		0x30
#define MAX8907_REG_LDOSEQCNT7		0x31
#define MAX8907_REG_LDO7VOUT		0x32
#define MAX8907_REG_LDOCTL8		0x34
#define MAX8907_REG_LDOSEQCNT8		0x35
#define MAX8907_REG_LDO8VOUT		0x36
#define MAX8907_REG_LDOCTL9		0x38
#define MAX8907_REG_LDOSEQCNT9		0x39
#define MAX8907_REG_LDO9VOUT		0x3A
#define MAX8907_REG_LDOCTL10		0x3C
#define MAX8907_REG_LDOSEQCNT10		0x3D
#define MAX8907_REG_LDO10VOUT		0x3E
#define MAX8907_REG_LDOCTL11		0x40
#define MAX8907_REG_LDOSEQCNT11		0x41
#define MAX8907_REG_LDO11VOUT		0x42
#define MAX8907_REG_LDOCTL12		0x44
#define MAX8907_REG_LDOSEQCNT12		0x45
#define MAX8907_REG_LDO12VOUT		0x46
#define MAX8907_REG_LDOCTL13		0x48
#define MAX8907_REG_LDOSEQCNT13		0x49
#define MAX8907_REG_LDO13VOUT		0x4A
#define MAX8907_REG_LDOCTL14		0x4C
#define MAX8907_REG_LDOSEQCNT14		0x4D
#define MAX8907_REG_LDO14VOUT		0x4E
#define MAX8907_REG_LDOCTL15		0x50
#define MAX8907_REG_LDOSEQCNT15		0x51
#define MAX8907_REG_LDO15VOUT		0x52
#define MAX8907_REG_OUT5VEN		0x54
#define MAX8907_REG_OUT5VSEQ		0x55
#define MAX8907_REG_OUT33VEN		0x58
#define MAX8907_REG_OUT33VSEQ		0x59
#define MAX8907_REG_LDOCTL19		0x5C
#define MAX8907_REG_LDOSEQCNT19		0x5D
#define MAX8907_REG_LDO19VOUT		0x5E
#define MAX8907_REG_LBCNFG		0x60
#define MAX8907_REG_SEQ1CNFG		0x64
#define MAX8907_REG_SEQ2CNFG		0x65
#define MAX8907_REG_SEQ3CNFG		0x66
#define MAX8907_REG_SEQ4CNFG		0x67
#define MAX8907_REG_SEQ5CNFG		0x68
#define MAX8907_REG_SEQ6CNFG		0x69
#define MAX8907_REG_SEQ7CNFG		0x6A
#define MAX8907_REG_LDOCTL18		0x72
#define MAX8907_REG_LDOSEQCNT18		0x73
#define MAX8907_REG_LDO18VOUT		0x74
#define MAX8907_REG_BBAT_CNFG		0x78
#define MAX8907_REG_CHG_CNTL1		0x7C
#define MAX8907_REG_CHG_CNTL2		0x7D
#define MAX8907_REG_CHG_IRQ1		0x7E
#define MAX8907_REG_CHG_IRQ2		0x7F
#define MAX8907_REG_CHG_IRQ1_MASK	0x80
#define MAX8907_REG_CHG_IRQ2_MASK	0x81
#define MAX8907_REG_CHG_STAT		0x82
#define MAX8907_REG_WLED_MODE_CNTL	0x84
#define MAX8907_REG_ILED_CNTL		0x84
#define MAX8907_REG_II1RR		0x8E
#define MAX8907_REG_II2RR		0x8F
#define MAX8907_REG_LDOCTL20		0x9C
#define MAX8907_REG_LDOSEQCNT20		0x9D
#define MAX8907_REG_LDO20VOUT		0x9E

/*                  */
#define MAX8907_REG_RTC_SEC		0x00
#define MAX8907_REG_RTC_MIN		0x01
#define MAX8907_REG_RTC_HOURS		0x02
#define MAX8907_REG_RTC_WEEKDAY		0x03
#define MAX8907_REG_RTC_DATE		0x04
#define MAX8907_REG_RTC_MONTH		0x05
#define MAX8907_REG_RTC_YEAR1		0x06
#define MAX8907_REG_RTC_YEAR2		0x07
#define MAX8907_REG_ALARM0_SEC		0x08
#define MAX8907_REG_ALARM0_MIN		0x09
#define MAX8907_REG_ALARM0_HOURS	0x0A
#define MAX8907_REG_ALARM0_WEEKDAY	0x0B
#define MAX8907_REG_ALARM0_DATE		0x0C
#define MAX8907_REG_ALARM0_MONTH	0x0D
#define MAX8907_REG_ALARM0_YEAR1	0x0E
#define MAX8907_REG_ALARM0_YEAR2	0x0F
#define MAX8907_REG_ALARM1_SEC		0x10
#define MAX8907_REG_ALARM1_MIN		0x11
#define MAX8907_REG_ALARM1_HOURS	0x12
#define MAX8907_REG_ALARM1_WEEKDAY	0x13
#define MAX8907_REG_ALARM1_DATE		0x14
#define MAX8907_REG_ALARM1_MONTH	0x15
#define MAX8907_REG_ALARM1_YEAR1	0x16
#define MAX8907_REG_ALARM1_YEAR2	0x17
#define MAX8907_REG_ALARM0_CNTL		0x18
#define MAX8907_REG_ALARM1_CNTL		0x19
#define MAX8907_REG_RTC_STATUS		0x1A
#define MAX8907_REG_RTC_CNTL		0x1B
#define MAX8907_REG_RTC_IRQ		0x1C
#define MAX8907_REG_RTC_IRQ_MASK	0x1D
#define MAX8907_REG_MPL_CNTL		0x1E

/*                                              */
#define MAX8907_CTL			0
#define MAX8907_SEQCNT			1
#define MAX8907_VOUT			2

/*                 */
#define MAX8907_MASK_LDO_SEQ		0x1C
#define MAX8907_MASK_LDO_EN		0x01
#define MAX8907_MASK_VBBATTCV		0x03
#define MAX8907_MASK_OUT5V_VINEN	0x10
#define MAX8907_MASK_OUT5V_ENSRC	0x0E
#define MAX8907_MASK_OUT5V_EN		0x01
#define MAX8907_MASK_POWER_OFF		0x40

/*               */
#define MAX8907_MBATT	0
#define MAX8907_SD1	1
#define MAX8907_SD2	2
#define MAX8907_SD3	3
#define MAX8907_LDO1	4
#define MAX8907_LDO2	5
#define MAX8907_LDO3	6
#define MAX8907_LDO4	7
#define MAX8907_LDO5	8
#define MAX8907_LDO6	9
#define MAX8907_LDO7	10
#define MAX8907_LDO8	11
#define MAX8907_LDO9	12
#define MAX8907_LDO10	13
#define MAX8907_LDO11	14
#define MAX8907_LDO12	15
#define MAX8907_LDO13	16
#define MAX8907_LDO14	17
#define MAX8907_LDO15	18
#define MAX8907_LDO16	19
#define MAX8907_LDO17	20
#define MAX8907_LDO18	21
#define MAX8907_LDO19	22
#define MAX8907_LDO20	23
#define MAX8907_OUT5V	24
#define MAX8907_OUT33V	25
#define MAX8907_BBAT	26
#define MAX8907_SDBY	27
#define MAX8907_VRTC	28
#define MAX8907_NUM_REGULATORS (MAX8907_VRTC + 1)

/*                 */
enum {
	MAX8907_IRQ_VCHG_DC_OVP = 0,
	MAX8907_IRQ_VCHG_DC_F,
	MAX8907_IRQ_VCHG_DC_R,
	MAX8907_IRQ_VCHG_THM_OK_R,
	MAX8907_IRQ_VCHG_THM_OK_F,
	MAX8907_IRQ_VCHG_MBATTLOW_F,
	MAX8907_IRQ_VCHG_MBATTLOW_R,
	MAX8907_IRQ_VCHG_RST,
	MAX8907_IRQ_VCHG_DONE,
	MAX8907_IRQ_VCHG_TOPOFF,
	MAX8907_IRQ_VCHG_TMR_FAULT,

	MAX8907_IRQ_GPM_RSTIN = 0,
	MAX8907_IRQ_GPM_MPL,
	MAX8907_IRQ_GPM_SW_3SEC,
	MAX8907_IRQ_GPM_EXTON_F,
	MAX8907_IRQ_GPM_EXTON_R,
	MAX8907_IRQ_GPM_SW_1SEC,
	MAX8907_IRQ_GPM_SW_F,
	MAX8907_IRQ_GPM_SW_R,
	MAX8907_IRQ_GPM_SYSCKEN_F,
	MAX8907_IRQ_GPM_SYSCKEN_R,

	MAX8907_IRQ_RTC_ALARM1 = 0,
	MAX8907_IRQ_RTC_ALARM0,
};

struct max8907_platform_data {
	struct regulator_init_data *init_data[MAX8907_NUM_REGULATORS];
	bool pm_off;
};

struct regmap_irq_chips_data;

struct max8907 {
	struct device			*dev;
	struct mutex			irq_lock;
	struct i2c_client		*i2c_gen;
	struct i2c_client		*i2c_rtc;
	struct regmap			*regmap_gen;
	struct regmap			*regmap_rtc;
	struct regmap_irq_chip_data	*irqc_chg;
	struct regmap_irq_chip_data	*irqc_on_off;
	struct regmap_irq_chip_data	*irqc_rtc;
};

#endif
