/* Driver for Realtek PCI-Express card reader
 *
 * Copyright(c) 2009-2013 Realtek Semiconductor Corp. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * Author:
 *   Wei WANG <wei_wang@realsil.com.cn>
 *   No. 128, West Shenhu Road, Suzhou Industry Park, Suzhou, China
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/mfd/rtsx_pci.h>

#include "rtsx_pcr.h"

static u8 rts5249_get_ic_version(struct rtsx_pcr *pcr)
{
	u8 val;

	rtsx_pci_read_register(pcr, DUMMY_REG_RESET_0, &val);
	return val & 0x0F;
}

static int rts5249_extra_init_hw(struct rtsx_pcr *pcr)
{
	rtsx_pci_init_cmd(pcr);

	/*                          */
	rtsx_pci_add_cmd(pcr, WRITE_REG_CMD, GPIO_CTL, 0x02, 0x02);
	/*                                             */
	rtsx_pci_add_cmd(pcr, WRITE_REG_CMD, LDO_PWR_SEL, 0x03, 0x00);
	rtsx_pci_add_cmd(pcr, WRITE_REG_CMD, LDO_PWR_SEL, 0x03, 0x01);
	/*                                                    */
	rtsx_pci_add_cmd(pcr, WRITE_REG_CMD, OLT_LED_CTL, 0x0F, 0x02);
	/*                 */
	rtsx_pci_add_cmd(pcr, WRITE_REG_CMD,
			SD30_CLK_DRIVE_SEL, 0xFF, 0x99);
	rtsx_pci_add_cmd(pcr, WRITE_REG_CMD,
			SD30_CMD_DRIVE_SEL, 0xFF, 0x99);
	rtsx_pci_add_cmd(pcr, WRITE_REG_CMD,
			SD30_DAT_DRIVE_SEL, 0xFF, 0x92);

	return rtsx_pci_send_cmd(pcr, 100);
}

static int rts5249_optimize_phy(struct rtsx_pcr *pcr)
{
	int err;

	err = rtsx_pci_write_phy_register(pcr, PHY_REG_REV, 0xFE46);
	if (err < 0)
		return err;

	msleep(1);

	return rtsx_pci_write_phy_register(pcr, PHY_BPCR, 0x05C0);
}

static int rts5249_turn_on_led(struct rtsx_pcr *pcr)
{
	return rtsx_pci_write_register(pcr, GPIO_CTL, 0x02, 0x02);
}

static int rts5249_turn_off_led(struct rtsx_pcr *pcr)
{
	return rtsx_pci_write_register(pcr, GPIO_CTL, 0x02, 0x00);
}

static int rts5249_enable_auto_blink(struct rtsx_pcr *pcr)
{
	return rtsx_pci_write_register(pcr, OLT_LED_CTL, 0x08, 0x08);
}

static int rts5249_disable_auto_blink(struct rtsx_pcr *pcr)
{
	return rtsx_pci_write_register(pcr, OLT_LED_CTL, 0x08, 0x00);
}

static int rts5249_card_power_on(struct rtsx_pcr *pcr, int card)
{
	int err;

	rtsx_pci_init_cmd(pcr);
	rtsx_pci_add_cmd(pcr, WRITE_REG_CMD, CARD_PWR_CTL,
			SD_POWER_MASK, SD_VCC_PARTIAL_POWER_ON);
	rtsx_pci_add_cmd(pcr, WRITE_REG_CMD, PWR_GATE_CTRL,
			LDO3318_PWR_MASK, 0x02);
	err = rtsx_pci_send_cmd(pcr, 100);
	if (err < 0)
		return err;

	msleep(5);

	rtsx_pci_init_cmd(pcr);
	rtsx_pci_add_cmd(pcr, WRITE_REG_CMD, CARD_PWR_CTL,
			SD_POWER_MASK, SD_VCC_POWER_ON);
	rtsx_pci_add_cmd(pcr, WRITE_REG_CMD, PWR_GATE_CTRL,
			LDO3318_PWR_MASK, 0x06);
	err = rtsx_pci_send_cmd(pcr, 100);
	if (err < 0)
		return err;

	return 0;
}

static int rts5249_card_power_off(struct rtsx_pcr *pcr, int card)
{
	rtsx_pci_init_cmd(pcr);
	rtsx_pci_add_cmd(pcr, WRITE_REG_CMD, CARD_PWR_CTL,
			SD_POWER_MASK, SD_POWER_OFF);
	rtsx_pci_add_cmd(pcr, WRITE_REG_CMD, PWR_GATE_CTRL,
			LDO3318_PWR_MASK, 0x00);
	return rtsx_pci_send_cmd(pcr, 100);
}

static int rts5249_switch_output_voltage(struct rtsx_pcr *pcr, u8 voltage)
{
	int err;
	u8 clk_drive, cmd_drive, dat_drive;

	if (voltage == OUTPUT_3V3) {
		err = rtsx_pci_write_phy_register(pcr, PHY_TUNE, 0x4FC0 | 0x24);
		if (err < 0)
			return err;
		clk_drive = 0x99;
		cmd_drive = 0x99;
		dat_drive = 0x92;
	} else if (voltage == OUTPUT_1V8) {
		err = rtsx_pci_write_phy_register(pcr, PHY_BACR, 0x3C02);
		if (err < 0)
			return err;
		err = rtsx_pci_write_phy_register(pcr, PHY_TUNE, 0x4C40 | 0x24);
		if (err < 0)
			return err;
		clk_drive = 0xb3;
		cmd_drive = 0xb3;
		dat_drive = 0xb3;
	} else {
		return -EINVAL;
	}

	/*               */
	rtsx_pci_init_cmd(pcr);
	rtsx_pci_add_cmd(pcr, WRITE_REG_CMD, SD30_CLK_DRIVE_SEL,
			0xFF, clk_drive);
	rtsx_pci_add_cmd(pcr, WRITE_REG_CMD, SD30_CMD_DRIVE_SEL,
			0xFF, cmd_drive);
	rtsx_pci_add_cmd(pcr, WRITE_REG_CMD, SD30_DAT_DRIVE_SEL,
			0xFF, dat_drive);
	return rtsx_pci_send_cmd(pcr, 100);
}

static const struct pcr_ops rts5249_pcr_ops = {
	.extra_init_hw = rts5249_extra_init_hw,
	.optimize_phy = rts5249_optimize_phy,
	.turn_on_led = rts5249_turn_on_led,
	.turn_off_led = rts5249_turn_off_led,
	.enable_auto_blink = rts5249_enable_auto_blink,
	.disable_auto_blink = rts5249_disable_auto_blink,
	.card_power_on = rts5249_card_power_on,
	.card_power_off = rts5249_card_power_off,
	.switch_output_voltage = rts5249_switch_output_voltage,
};

/*                        
                              
                              
                              
                              
                                
 */
static const u32 rts5249_sd_pull_ctl_enable_tbl[] = {
	RTSX_REG_PAIR(CARD_PULL_CTL1, 0x66),
	RTSX_REG_PAIR(CARD_PULL_CTL2, 0xAA),
	RTSX_REG_PAIR(CARD_PULL_CTL3, 0xE9),
	RTSX_REG_PAIR(CARD_PULL_CTL4, 0xAA),
	0,
};

/*                         
                                
                              
                                
                                
                                
 */
static const u32 rts5249_sd_pull_ctl_disable_tbl[] = {
	RTSX_REG_PAIR(CARD_PULL_CTL1, 0x66),
	RTSX_REG_PAIR(CARD_PULL_CTL2, 0x55),
	RTSX_REG_PAIR(CARD_PULL_CTL3, 0xD5),
	RTSX_REG_PAIR(CARD_PULL_CTL4, 0x55),
	0,
};

/*                        
                              
                                
 */
static const u32 rts5249_ms_pull_ctl_enable_tbl[] = {
	RTSX_REG_PAIR(CARD_PULL_CTL4, 0x55),
	RTSX_REG_PAIR(CARD_PULL_CTL5, 0x55),
	RTSX_REG_PAIR(CARD_PULL_CTL6, 0x15),
	0,
};

/*                         
                              
                                
 */
static const u32 rts5249_ms_pull_ctl_disable_tbl[] = {
	RTSX_REG_PAIR(CARD_PULL_CTL4, 0x55),
	RTSX_REG_PAIR(CARD_PULL_CTL5, 0x55),
	RTSX_REG_PAIR(CARD_PULL_CTL6, 0x15),
	0,
};

void rts5249_init_params(struct rtsx_pcr *pcr)
{
	pcr->extra_caps = EXTRA_CAPS_SD_SDR50 | EXTRA_CAPS_SD_SDR104;
	pcr->num_slots = 2;
	pcr->ops = &rts5249_pcr_ops;

	pcr->ic_version = rts5249_get_ic_version(pcr);
	pcr->sd_pull_ctl_enable_tbl = rts5249_sd_pull_ctl_enable_tbl;
	pcr->sd_pull_ctl_disable_tbl = rts5249_sd_pull_ctl_disable_tbl;
	pcr->ms_pull_ctl_enable_tbl = rts5249_ms_pull_ctl_enable_tbl;
	pcr->ms_pull_ctl_disable_tbl = rts5249_ms_pull_ctl_disable_tbl;
}
