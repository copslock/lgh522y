/*
 * Copyright (c) 2013 Johannes Berg <johannes@sipsolutions.net>
 *
 *  This file is free software: you may copy, redistribute and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation, either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  This file is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 * Copyright (c) 2012 Qualcomm Atheros, Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <linux/etherdevice.h>
#include <linux/delay.h>
#include <linux/pci.h>
#include <linux/mdio.h>
#include "reg.h"
#include "hw.h"

static inline bool alx_is_rev_a(u8 rev)
{
	return rev == ALX_REV_A0 || rev == ALX_REV_A1;
}

static int alx_wait_mdio_idle(struct alx_hw *hw)
{
	u32 val;
	int i;

	for (i = 0; i < ALX_MDIO_MAX_AC_TO; i++) {
		val = alx_read_mem32(hw, ALX_MDIO);
		if (!(val & ALX_MDIO_BUSY))
			return 0;
		udelay(10);
	}

	return -ETIMEDOUT;
}

static int alx_read_phy_core(struct alx_hw *hw, bool ext, u8 dev,
			     u16 reg, u16 *phy_data)
{
	u32 val, clk_sel;
	int err;

	*phy_data = 0;

	/*                                                */
	clk_sel = hw->link_speed != SPEED_UNKNOWN ?
			ALX_MDIO_CLK_SEL_25MD4 :
			ALX_MDIO_CLK_SEL_25MD128;

	if (ext) {
		val = dev << ALX_MDIO_EXTN_DEVAD_SHIFT |
		      reg << ALX_MDIO_EXTN_REG_SHIFT;
		alx_write_mem32(hw, ALX_MDIO_EXTN, val);

		val = ALX_MDIO_SPRES_PRMBL | ALX_MDIO_START |
		      ALX_MDIO_MODE_EXT | ALX_MDIO_OP_READ |
		      clk_sel << ALX_MDIO_CLK_SEL_SHIFT;
	} else {
		val = ALX_MDIO_SPRES_PRMBL |
		      clk_sel << ALX_MDIO_CLK_SEL_SHIFT |
		      reg << ALX_MDIO_REG_SHIFT |
		      ALX_MDIO_START | ALX_MDIO_OP_READ;
	}
	alx_write_mem32(hw, ALX_MDIO, val);

	err = alx_wait_mdio_idle(hw);
	if (err)
		return err;
	val = alx_read_mem32(hw, ALX_MDIO);
	*phy_data = ALX_GET_FIELD(val, ALX_MDIO_DATA);
	return 0;
}

static int alx_write_phy_core(struct alx_hw *hw, bool ext, u8 dev,
			      u16 reg, u16 phy_data)
{
	u32 val, clk_sel;

	/*                                                */
	clk_sel = hw->link_speed != SPEED_UNKNOWN ?
			ALX_MDIO_CLK_SEL_25MD4 :
			ALX_MDIO_CLK_SEL_25MD128;

	if (ext) {
		val = dev << ALX_MDIO_EXTN_DEVAD_SHIFT |
		      reg << ALX_MDIO_EXTN_REG_SHIFT;
		alx_write_mem32(hw, ALX_MDIO_EXTN, val);

		val = ALX_MDIO_SPRES_PRMBL |
		      clk_sel << ALX_MDIO_CLK_SEL_SHIFT |
		      phy_data << ALX_MDIO_DATA_SHIFT |
		      ALX_MDIO_START | ALX_MDIO_MODE_EXT;
	} else {
		val = ALX_MDIO_SPRES_PRMBL |
		      clk_sel << ALX_MDIO_CLK_SEL_SHIFT |
		      reg << ALX_MDIO_REG_SHIFT |
		      phy_data << ALX_MDIO_DATA_SHIFT |
		      ALX_MDIO_START;
	}
	alx_write_mem32(hw, ALX_MDIO, val);

	return alx_wait_mdio_idle(hw);
}

static int __alx_read_phy_reg(struct alx_hw *hw, u16 reg, u16 *phy_data)
{
	return alx_read_phy_core(hw, false, 0, reg, phy_data);
}

static int __alx_write_phy_reg(struct alx_hw *hw, u16 reg, u16 phy_data)
{
	return alx_write_phy_core(hw, false, 0, reg, phy_data);
}

static int __alx_read_phy_ext(struct alx_hw *hw, u8 dev, u16 reg, u16 *pdata)
{
	return alx_read_phy_core(hw, true, dev, reg, pdata);
}

static int __alx_write_phy_ext(struct alx_hw *hw, u8 dev, u16 reg, u16 data)
{
	return alx_write_phy_core(hw, true, dev, reg, data);
}

static int __alx_read_phy_dbg(struct alx_hw *hw, u16 reg, u16 *pdata)
{
	int err;

	err = __alx_write_phy_reg(hw, ALX_MII_DBG_ADDR, reg);
	if (err)
		return err;

	return __alx_read_phy_reg(hw, ALX_MII_DBG_DATA, pdata);
}

static int __alx_write_phy_dbg(struct alx_hw *hw, u16 reg, u16 data)
{
	int err;

	err = __alx_write_phy_reg(hw, ALX_MII_DBG_ADDR, reg);
	if (err)
		return err;

	return __alx_write_phy_reg(hw, ALX_MII_DBG_DATA, data);
}

int alx_read_phy_reg(struct alx_hw *hw, u16 reg, u16 *phy_data)
{
	int err;

	spin_lock(&hw->mdio_lock);
	err = __alx_read_phy_reg(hw, reg, phy_data);
	spin_unlock(&hw->mdio_lock);

	return err;
}

int alx_write_phy_reg(struct alx_hw *hw, u16 reg, u16 phy_data)
{
	int err;

	spin_lock(&hw->mdio_lock);
	err = __alx_write_phy_reg(hw, reg, phy_data);
	spin_unlock(&hw->mdio_lock);

	return err;
}

int alx_read_phy_ext(struct alx_hw *hw, u8 dev, u16 reg, u16 *pdata)
{
	int err;

	spin_lock(&hw->mdio_lock);
	err = __alx_read_phy_ext(hw, dev, reg, pdata);
	spin_unlock(&hw->mdio_lock);

	return err;
}

int alx_write_phy_ext(struct alx_hw *hw, u8 dev, u16 reg, u16 data)
{
	int err;

	spin_lock(&hw->mdio_lock);
	err = __alx_write_phy_ext(hw, dev, reg, data);
	spin_unlock(&hw->mdio_lock);

	return err;
}

static int alx_read_phy_dbg(struct alx_hw *hw, u16 reg, u16 *pdata)
{
	int err;

	spin_lock(&hw->mdio_lock);
	err = __alx_read_phy_dbg(hw, reg, pdata);
	spin_unlock(&hw->mdio_lock);

	return err;
}

static int alx_write_phy_dbg(struct alx_hw *hw, u16 reg, u16 data)
{
	int err;

	spin_lock(&hw->mdio_lock);
	err = __alx_write_phy_dbg(hw, reg, data);
	spin_unlock(&hw->mdio_lock);

	return err;
}

static u16 alx_get_phy_config(struct alx_hw *hw)
{
	u32 val;
	u16 phy_val;

	val = alx_read_mem32(hw, ALX_PHY_CTRL);
	/*              */
	if ((val & ALX_PHY_CTRL_DSPRST_OUT) == 0)
		return ALX_DRV_PHY_UNKNOWN;

	val = alx_read_mem32(hw, ALX_DRV);
	val = ALX_GET_FIELD(val, ALX_DRV_PHY);
	if (ALX_DRV_PHY_UNKNOWN == val)
		return ALX_DRV_PHY_UNKNOWN;

	alx_read_phy_reg(hw, ALX_MII_DBG_ADDR, &phy_val);
	if (ALX_PHY_INITED == phy_val)
		return val;

	return ALX_DRV_PHY_UNKNOWN;
}

static bool alx_wait_reg(struct alx_hw *hw, u32 reg, u32 wait, u32 *val)
{
	u32 read;
	int i;

	for (i = 0; i < ALX_SLD_MAX_TO; i++) {
		read = alx_read_mem32(hw, reg);
		if ((read & wait) == 0) {
			if (val)
				*val = read;
			return true;
		}
		mdelay(1);
	}

	return false;
}

static bool alx_read_macaddr(struct alx_hw *hw, u8 *addr)
{
	u32 mac0, mac1;

	mac0 = alx_read_mem32(hw, ALX_STAD0);
	mac1 = alx_read_mem32(hw, ALX_STAD1);

	/*                           */
	*(__be32 *)(addr + 2) = cpu_to_be32(mac0);
	*(__be16 *)addr = cpu_to_be16(mac1);

	return is_valid_ether_addr(addr);
}

int alx_get_perm_macaddr(struct alx_hw *hw, u8 *addr)
{
	u32 val;

	/*                                   */
	if (alx_read_macaddr(hw, addr))
		return 0;

	/*                        */
	if (!alx_wait_reg(hw, ALX_SLD, ALX_SLD_STAT | ALX_SLD_START, &val))
		return -EIO;
	alx_write_mem32(hw, ALX_SLD, val | ALX_SLD_START);
	if (!alx_wait_reg(hw, ALX_SLD, ALX_SLD_START, NULL))
		return -EIO;
	if (alx_read_macaddr(hw, addr))
		return 0;

	/*                                            */
	val = alx_read_mem32(hw, ALX_EFLD);
	if (val & (ALX_EFLD_F_EXIST | ALX_EFLD_E_EXIST)) {
		if (!alx_wait_reg(hw, ALX_EFLD,
				  ALX_EFLD_STAT | ALX_EFLD_START, &val))
			return -EIO;
		alx_write_mem32(hw, ALX_EFLD, val | ALX_EFLD_START);
		if (!alx_wait_reg(hw, ALX_EFLD, ALX_EFLD_START, NULL))
			return -EIO;
		if (alx_read_macaddr(hw, addr))
			return 0;
	}

	return -EIO;
}

void alx_set_macaddr(struct alx_hw *hw, const u8 *addr)
{
	u32 val;

	/*                                                             */
	val = be32_to_cpu(*(__be32 *)(addr + 2));
	alx_write_mem32(hw, ALX_STAD0, val);
	val = be16_to_cpu(*(__be16 *)addr);
	alx_write_mem32(hw, ALX_STAD1, val);
}

static void alx_enable_osc(struct alx_hw *hw)
{
	u32 val;

	/*             */
	val = alx_read_mem32(hw, ALX_MISC);
	alx_write_mem32(hw, ALX_MISC, val & ~ALX_MISC_INTNLOSC_OPEN);
	alx_write_mem32(hw, ALX_MISC, val | ALX_MISC_INTNLOSC_OPEN);
}

static void alx_reset_osc(struct alx_hw *hw, u8 rev)
{
	u32 val, val2;

	/*                                                         */
	val = alx_read_mem32(hw, ALX_MISC3);
	alx_write_mem32(hw, ALX_MISC3,
			(val & ~ALX_MISC3_25M_BY_SW) |
			ALX_MISC3_25M_NOTO_INTNL);

	/*                                                           
                                                              
  */
	val = alx_read_mem32(hw, ALX_MISC);
	if (rev >= ALX_REV_B0) {
		/*                                         
                                       
   */
		ALX_SET_FIELD(val, ALX_MISC_PSW_OCP, ALX_MISC_PSW_OCP_DEF);
		/*                                                   */
		val &= ~ALX_MISC_INTNLOSC_OPEN;
		alx_write_mem32(hw, ALX_MISC, val);
		alx_write_mem32(hw, ALX_MISC, val | ALX_MISC_INTNLOSC_OPEN);
		/*                                          */
		val2 = alx_read_mem32(hw, ALX_MSIC2);
		val2 &= ~ALX_MSIC2_CALB_START;
		alx_write_mem32(hw, ALX_MSIC2, val2);
		alx_write_mem32(hw, ALX_MSIC2, val2 | ALX_MSIC2_CALB_START);
	} else {
		val &= ~ALX_MISC_INTNLOSC_OPEN;
		/*                                   */
		if (alx_is_rev_a(rev))
			val &= ~ALX_MISC_ISO_EN;

		alx_write_mem32(hw, ALX_MISC, val | ALX_MISC_INTNLOSC_OPEN);
		alx_write_mem32(hw, ALX_MISC, val);
	}

	udelay(20);
}

static int alx_stop_mac(struct alx_hw *hw)
{
	u32 rxq, txq, val;
	u16 i;

	rxq = alx_read_mem32(hw, ALX_RXQ0);
	alx_write_mem32(hw, ALX_RXQ0, rxq & ~ALX_RXQ0_EN);
	txq = alx_read_mem32(hw, ALX_TXQ0);
	alx_write_mem32(hw, ALX_TXQ0, txq & ~ALX_TXQ0_EN);

	udelay(40);

	hw->rx_ctrl &= ~(ALX_MAC_CTRL_RX_EN | ALX_MAC_CTRL_TX_EN);
	alx_write_mem32(hw, ALX_MAC_CTRL, hw->rx_ctrl);

	for (i = 0; i < ALX_DMA_MAC_RST_TO; i++) {
		val = alx_read_mem32(hw, ALX_MAC_STS);
		if (!(val & ALX_MAC_STS_IDLE))
			return 0;
		udelay(10);
	}

	return -ETIMEDOUT;
}

int alx_reset_mac(struct alx_hw *hw)
{
	u32 val, pmctrl;
	int i, ret;
	u8 rev;
	bool a_cr;

	pmctrl = 0;
	rev = alx_hw_revision(hw);
	a_cr = alx_is_rev_a(rev) && alx_hw_with_cr(hw);

	/*                                 */
	alx_write_mem32(hw, ALX_MSIX_MASK, 0xFFFFFFFF);
	alx_write_mem32(hw, ALX_IMR, 0);
	alx_write_mem32(hw, ALX_ISR, ALX_ISR_DIS);

	ret = alx_stop_mac(hw);
	if (ret)
		return ret;

	/*                     */
	alx_write_mem32(hw, ALX_RFD_PIDX, 1);

	/*                             */
	if (a_cr) {
		pmctrl = alx_read_mem32(hw, ALX_PMCTRL);
		if (pmctrl & (ALX_PMCTRL_L1_EN | ALX_PMCTRL_L0S_EN))
			alx_write_mem32(hw, ALX_PMCTRL,
					pmctrl & ~(ALX_PMCTRL_L1_EN |
						   ALX_PMCTRL_L0S_EN));
	}

	/*                        */
	val = alx_read_mem32(hw, ALX_MASTER);
	alx_write_mem32(hw, ALX_MASTER,
			val | ALX_MASTER_DMA_MAC_RST | ALX_MASTER_OOB_DIS);

	/*                          */
	udelay(10);
	for (i = 0; i < ALX_DMA_MAC_RST_TO; i++) {
		val = alx_read_mem32(hw, ALX_RFD_PIDX);
		if (val == 0)
			break;
		udelay(10);
	}
	for (; i < ALX_DMA_MAC_RST_TO; i++) {
		val = alx_read_mem32(hw, ALX_MASTER);
		if ((val & ALX_MASTER_DMA_MAC_RST) == 0)
			break;
		udelay(10);
	}
	if (i == ALX_DMA_MAC_RST_TO)
		return -EIO;
	udelay(10);

	if (a_cr) {
		alx_write_mem32(hw, ALX_MASTER, val | ALX_MASTER_PCLKSEL_SRDS);
		/*                  */
		if (pmctrl & (ALX_PMCTRL_L1_EN | ALX_PMCTRL_L0S_EN))
			alx_write_mem32(hw, ALX_PMCTRL, pmctrl);
	}

	alx_reset_osc(hw, rev);

	/*                                                         
                                     
  */
	val = alx_read_mem32(hw, ALX_MISC3);
	alx_write_mem32(hw, ALX_MISC3,
			(val & ~ALX_MISC3_25M_BY_SW) |
			ALX_MISC3_25M_NOTO_INTNL);
	val = alx_read_mem32(hw, ALX_MISC);
	val &= ~ALX_MISC_INTNLOSC_OPEN;
	if (alx_is_rev_a(rev))
		val &= ~ALX_MISC_ISO_EN;
	alx_write_mem32(hw, ALX_MISC, val);
	udelay(20);

	/*                                       */
	alx_write_mem32(hw, ALX_MAC_CTRL, hw->rx_ctrl);

	val = alx_read_mem32(hw, ALX_SERDES);
	alx_write_mem32(hw, ALX_SERDES,
			val | ALX_SERDES_MACCLK_SLWDWN |
			ALX_SERDES_PHYCLK_SLWDWN);

	return 0;
}

void alx_reset_phy(struct alx_hw *hw)
{
	int i;
	u32 val;
	u16 phy_val;

	/*                     */
	val = alx_read_mem32(hw, ALX_PHY_CTRL);
	val &= ~(ALX_PHY_CTRL_DSPRST_OUT | ALX_PHY_CTRL_IDDQ |
		 ALX_PHY_CTRL_GATE_25M | ALX_PHY_CTRL_POWER_DOWN |
		 ALX_PHY_CTRL_CLS);
	val |= ALX_PHY_CTRL_RST_ANALOG;

	val |= (ALX_PHY_CTRL_HIB_PULSE | ALX_PHY_CTRL_HIB_EN);
	alx_write_mem32(hw, ALX_PHY_CTRL, val);
	udelay(10);
	alx_write_mem32(hw, ALX_PHY_CTRL, val | ALX_PHY_CTRL_DSPRST_OUT);

	for (i = 0; i < ALX_PHY_CTRL_DSPRST_TO; i++)
		udelay(10);

	/*                        */
	alx_write_phy_dbg(hw, ALX_MIIDBG_LEGCYPS, ALX_LEGCYPS_DEF);
	alx_write_phy_dbg(hw, ALX_MIIDBG_SYSMODCTRL,
			  ALX_SYSMODCTRL_IECHOADJ_DEF);
	alx_write_phy_ext(hw, ALX_MIIEXT_PCS, ALX_MIIEXT_VDRVBIAS,
			  ALX_VDRVBIAS_DEF);

	/*                   */
	val = alx_read_mem32(hw, ALX_LPI_CTRL);
	alx_write_mem32(hw, ALX_LPI_CTRL, val & ~ALX_LPI_CTRL_EN);
	alx_write_phy_ext(hw, ALX_MIIEXT_ANEG, ALX_MIIEXT_LOCAL_EEEADV, 0);

	/*                  */
	alx_write_phy_dbg(hw, ALX_MIIDBG_TST10BTCFG, ALX_TST10BTCFG_DEF);
	alx_write_phy_dbg(hw, ALX_MIIDBG_SRDSYSMOD, ALX_SRDSYSMOD_DEF);
	alx_write_phy_dbg(hw, ALX_MIIDBG_TST100BTCFG, ALX_TST100BTCFG_DEF);
	alx_write_phy_dbg(hw, ALX_MIIDBG_ANACTRL, ALX_ANACTRL_DEF);
	alx_read_phy_dbg(hw, ALX_MIIDBG_GREENCFG2, &phy_val);
	alx_write_phy_dbg(hw, ALX_MIIDBG_GREENCFG2,
			  phy_val & ~ALX_GREENCFG2_GATE_DFSE_EN);
	/*                      */
	alx_write_phy_ext(hw, ALX_MIIEXT_ANEG, ALX_MIIEXT_NLP78,
			  ALX_MIIEXT_NLP78_120M_DEF);
	alx_write_phy_ext(hw, ALX_MIIEXT_ANEG, ALX_MIIEXT_S3DIG10,
			  ALX_MIIEXT_S3DIG10_DEF);

	if (hw->lnk_patch) {
		/*                         */
		alx_read_phy_ext(hw, ALX_MIIEXT_PCS, ALX_MIIEXT_CLDCTRL3,
				 &phy_val);
		alx_write_phy_ext(hw, ALX_MIIEXT_PCS, ALX_MIIEXT_CLDCTRL3,
				  phy_val | ALX_CLDCTRL3_BP_CABLE1TH_DET_GT);
		/*                        */
		alx_read_phy_dbg(hw, ALX_MIIDBG_GREENCFG2, &phy_val);
		alx_write_phy_dbg(hw, ALX_MIIDBG_GREENCFG2,
				  phy_val | ALX_GREENCFG2_BP_GREEN);
		/*                    */
		alx_read_phy_ext(hw, ALX_MIIEXT_PCS, ALX_MIIEXT_CLDCTRL5,
				 &phy_val);
		alx_write_phy_ext(hw, ALX_MIIEXT_PCS, ALX_MIIEXT_CLDCTRL5,
				  phy_val | ALX_CLDCTRL5_BP_VD_HLFBIAS);
	}

	/*                        */
	alx_write_phy_reg(hw, ALX_MII_IER, ALX_IER_LINK_UP | ALX_IER_LINK_DOWN);
}

#define ALX_PCI_CMD (PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY | PCI_COMMAND_IO)

void alx_reset_pcie(struct alx_hw *hw)
{
	u8 rev = alx_hw_revision(hw);
	u32 val;
	u16 val16;

	/*                                                              */
	pci_read_config_word(hw->pdev, PCI_COMMAND, &val16);
	if (!(val16 & ALX_PCI_CMD) || (val16 & PCI_COMMAND_INTX_DISABLE)) {
		val16 = (val16 | ALX_PCI_CMD) & ~PCI_COMMAND_INTX_DISABLE;
		pci_write_config_word(hw->pdev, PCI_COMMAND, val16);
	}

	/*                          */
	val = alx_read_mem32(hw, ALX_WOL0);
	alx_write_mem32(hw, ALX_WOL0, 0);

	val = alx_read_mem32(hw, ALX_PDLL_TRNS1);
	alx_write_mem32(hw, ALX_PDLL_TRNS1, val & ~ALX_PDLL_TRNS1_D3PLLOFF_EN);

	/*                           */
	val = alx_read_mem32(hw, ALX_UE_SVRT);
	val &= ~(ALX_UE_SVRT_DLPROTERR | ALX_UE_SVRT_FCPROTERR);
	alx_write_mem32(hw, ALX_UE_SVRT, val);

	/*                */
	val = alx_read_mem32(hw, ALX_MASTER);
	if (alx_is_rev_a(rev) && alx_hw_with_cr(hw)) {
		if ((val & ALX_MASTER_WAKEN_25M) == 0 ||
		    (val & ALX_MASTER_PCLKSEL_SRDS) == 0)
			alx_write_mem32(hw, ALX_MASTER,
					val | ALX_MASTER_PCLKSEL_SRDS |
					ALX_MASTER_WAKEN_25M);
	} else {
		if ((val & ALX_MASTER_WAKEN_25M) == 0 ||
		    (val & ALX_MASTER_PCLKSEL_SRDS) != 0)
			alx_write_mem32(hw, ALX_MASTER,
					(val & ~ALX_MASTER_PCLKSEL_SRDS) |
					ALX_MASTER_WAKEN_25M);
	}

	/*              */
	alx_enable_aspm(hw, true, true);

	udelay(10);
}

void alx_start_mac(struct alx_hw *hw)
{
	u32 mac, txq, rxq;

	rxq = alx_read_mem32(hw, ALX_RXQ0);
	alx_write_mem32(hw, ALX_RXQ0, rxq | ALX_RXQ0_EN);
	txq = alx_read_mem32(hw, ALX_TXQ0);
	alx_write_mem32(hw, ALX_TXQ0, txq | ALX_TXQ0_EN);

	mac = hw->rx_ctrl;
	if (hw->link_speed % 10 == DUPLEX_FULL)
		mac |= ALX_MAC_CTRL_FULLD;
	else
		mac &= ~ALX_MAC_CTRL_FULLD;
	ALX_SET_FIELD(mac, ALX_MAC_CTRL_SPEED,
		      hw->link_speed >= SPEED_1000 ? ALX_MAC_CTRL_SPEED_1000 :
						     ALX_MAC_CTRL_SPEED_10_100);
	mac |= ALX_MAC_CTRL_TX_EN | ALX_MAC_CTRL_RX_EN;
	hw->rx_ctrl = mac;
	alx_write_mem32(hw, ALX_MAC_CTRL, mac);
}

void alx_cfg_mac_flowcontrol(struct alx_hw *hw, u8 fc)
{
	if (fc & ALX_FC_RX)
		hw->rx_ctrl |= ALX_MAC_CTRL_RXFC_EN;
	else
		hw->rx_ctrl &= ~ALX_MAC_CTRL_RXFC_EN;

	if (fc & ALX_FC_TX)
		hw->rx_ctrl |= ALX_MAC_CTRL_TXFC_EN;
	else
		hw->rx_ctrl &= ~ALX_MAC_CTRL_TXFC_EN;

	alx_write_mem32(hw, ALX_MAC_CTRL, hw->rx_ctrl);
}

void alx_enable_aspm(struct alx_hw *hw, bool l0s_en, bool l1_en)
{
	u32 pmctrl;
	u8 rev = alx_hw_revision(hw);

	pmctrl = alx_read_mem32(hw, ALX_PMCTRL);

	ALX_SET_FIELD(pmctrl, ALX_PMCTRL_LCKDET_TIMER,
		      ALX_PMCTRL_LCKDET_TIMER_DEF);
	pmctrl |= ALX_PMCTRL_RCVR_WT_1US |
		  ALX_PMCTRL_L1_CLKSW_EN |
		  ALX_PMCTRL_L1_SRDSRX_PWD;
	ALX_SET_FIELD(pmctrl, ALX_PMCTRL_L1REQ_TO, ALX_PMCTRL_L1REG_TO_DEF);
	ALX_SET_FIELD(pmctrl, ALX_PMCTRL_L1_TIMER, ALX_PMCTRL_L1_TIMER_16US);
	pmctrl &= ~(ALX_PMCTRL_L1_SRDS_EN |
		    ALX_PMCTRL_L1_SRDSPLL_EN |
		    ALX_PMCTRL_L1_BUFSRX_EN |
		    ALX_PMCTRL_SADLY_EN |
		    ALX_PMCTRL_HOTRST_WTEN|
		    ALX_PMCTRL_L0S_EN |
		    ALX_PMCTRL_L1_EN |
		    ALX_PMCTRL_ASPM_FCEN |
		    ALX_PMCTRL_TXL1_AFTER_L0S |
		    ALX_PMCTRL_RXL1_AFTER_L0S);
	if (alx_is_rev_a(rev) && alx_hw_with_cr(hw))
		pmctrl |= ALX_PMCTRL_L1_SRDS_EN | ALX_PMCTRL_L1_SRDSPLL_EN;

	if (l0s_en)
		pmctrl |= (ALX_PMCTRL_L0S_EN | ALX_PMCTRL_ASPM_FCEN);
	if (l1_en)
		pmctrl |= (ALX_PMCTRL_L1_EN | ALX_PMCTRL_ASPM_FCEN);

	alx_write_mem32(hw, ALX_PMCTRL, pmctrl);
}


static u32 ethadv_to_hw_cfg(struct alx_hw *hw, u32 ethadv_cfg)
{
	u32 cfg = 0;

	if (ethadv_cfg & ADVERTISED_Autoneg) {
		cfg |= ALX_DRV_PHY_AUTO;
		if (ethadv_cfg & ADVERTISED_10baseT_Half)
			cfg |= ALX_DRV_PHY_10;
		if (ethadv_cfg & ADVERTISED_10baseT_Full)
			cfg |= ALX_DRV_PHY_10 | ALX_DRV_PHY_DUPLEX;
		if (ethadv_cfg & ADVERTISED_100baseT_Half)
			cfg |= ALX_DRV_PHY_100;
		if (ethadv_cfg & ADVERTISED_100baseT_Full)
			cfg |= ALX_DRV_PHY_100 | ALX_DRV_PHY_DUPLEX;
		if (ethadv_cfg & ADVERTISED_1000baseT_Half)
			cfg |= ALX_DRV_PHY_1000;
		if (ethadv_cfg & ADVERTISED_1000baseT_Full)
			cfg |= ALX_DRV_PHY_100 | ALX_DRV_PHY_DUPLEX;
		if (ethadv_cfg & ADVERTISED_Pause)
			cfg |= ADVERTISE_PAUSE_CAP;
		if (ethadv_cfg & ADVERTISED_Asym_Pause)
			cfg |= ADVERTISE_PAUSE_ASYM;
	} else {
		switch (ethadv_cfg) {
		case ADVERTISED_10baseT_Half:
			cfg |= ALX_DRV_PHY_10;
			break;
		case ADVERTISED_100baseT_Half:
			cfg |= ALX_DRV_PHY_100;
			break;
		case ADVERTISED_10baseT_Full:
			cfg |= ALX_DRV_PHY_10 | ALX_DRV_PHY_DUPLEX;
			break;
		case ADVERTISED_100baseT_Full:
			cfg |= ALX_DRV_PHY_100 | ALX_DRV_PHY_DUPLEX;
			break;
		}
	}

	return cfg;
}

int alx_setup_speed_duplex(struct alx_hw *hw, u32 ethadv, u8 flowctrl)
{
	u16 adv, giga, cr;
	u32 val;
	int err = 0;

	alx_write_phy_reg(hw, ALX_MII_DBG_ADDR, 0);
	val = alx_read_mem32(hw, ALX_DRV);
	ALX_SET_FIELD(val, ALX_DRV_PHY, 0);

	if (ethadv & ADVERTISED_Autoneg) {
		adv = ADVERTISE_CSMA;
		adv |= ethtool_adv_to_mii_adv_t(ethadv);

		if (flowctrl & ALX_FC_ANEG) {
			if (flowctrl & ALX_FC_RX) {
				adv |= ADVERTISED_Pause;
				if (!(flowctrl & ALX_FC_TX))
					adv |= ADVERTISED_Asym_Pause;
			} else if (flowctrl & ALX_FC_TX) {
				adv |= ADVERTISED_Asym_Pause;
			}
		}
		giga = 0;
		if (alx_hw_giga(hw))
			giga = ethtool_adv_to_mii_ctrl1000_t(ethadv);

		cr = BMCR_RESET | BMCR_ANENABLE | BMCR_ANRESTART;

		if (alx_write_phy_reg(hw, MII_ADVERTISE, adv) ||
		    alx_write_phy_reg(hw, MII_CTRL1000, giga) ||
		    alx_write_phy_reg(hw, MII_BMCR, cr))
			err = -EBUSY;
	} else {
		cr = BMCR_RESET;
		if (ethadv == ADVERTISED_100baseT_Half ||
		    ethadv == ADVERTISED_100baseT_Full)
			cr |= BMCR_SPEED100;
		if (ethadv == ADVERTISED_10baseT_Full ||
		    ethadv == ADVERTISED_100baseT_Full)
			cr |= BMCR_FULLDPLX;

		err = alx_write_phy_reg(hw, MII_BMCR, cr);
	}

	if (!err) {
		alx_write_phy_reg(hw, ALX_MII_DBG_ADDR, ALX_PHY_INITED);
		val |= ethadv_to_hw_cfg(hw, ethadv);
	}

	alx_write_mem32(hw, ALX_DRV, val);

	return err;
}


void alx_post_phy_link(struct alx_hw *hw)
{
	u16 phy_val, len, agc;
	u8 revid = alx_hw_revision(hw);
	bool adj_th = revid == ALX_REV_B0;
	int speed;

	if (hw->link_speed == SPEED_UNKNOWN)
		speed = SPEED_UNKNOWN;
	else
		speed = hw->link_speed - hw->link_speed % 10;

	if (revid != ALX_REV_B0 && !alx_is_rev_a(revid))
		return;

	/*                               */
	if (speed != SPEED_UNKNOWN) {
		alx_read_phy_ext(hw, ALX_MIIEXT_PCS, ALX_MIIEXT_CLDCTRL6,
				 &phy_val);
		len = ALX_GET_FIELD(phy_val, ALX_CLDCTRL6_CAB_LEN);
		alx_read_phy_dbg(hw, ALX_MIIDBG_AGC, &phy_val);
		agc = ALX_GET_FIELD(phy_val, ALX_AGC_2_VGA);

		if ((speed == SPEED_1000 &&
		     (len > ALX_CLDCTRL6_CAB_LEN_SHORT1G ||
		      (len == 0 && agc > ALX_AGC_LONG1G_LIMT))) ||
		    (speed == SPEED_100 &&
		     (len > ALX_CLDCTRL6_CAB_LEN_SHORT100M ||
		      (len == 0 && agc > ALX_AGC_LONG100M_LIMT)))) {
			alx_write_phy_dbg(hw, ALX_MIIDBG_AZ_ANADECT,
					  ALX_AZ_ANADECT_LONG);
			alx_read_phy_ext(hw, ALX_MIIEXT_ANEG, ALX_MIIEXT_AFE,
					 &phy_val);
			alx_write_phy_ext(hw, ALX_MIIEXT_ANEG, ALX_MIIEXT_AFE,
					  phy_val | ALX_AFE_10BT_100M_TH);
		} else {
			alx_write_phy_dbg(hw, ALX_MIIDBG_AZ_ANADECT,
					  ALX_AZ_ANADECT_DEF);
			alx_read_phy_ext(hw, ALX_MIIEXT_ANEG,
					 ALX_MIIEXT_AFE, &phy_val);
			alx_write_phy_ext(hw, ALX_MIIEXT_ANEG, ALX_MIIEXT_AFE,
					  phy_val & ~ALX_AFE_10BT_100M_TH);
		}

		/*                  */
		if (adj_th && hw->lnk_patch) {
			if (speed == SPEED_100) {
				alx_write_phy_dbg(hw, ALX_MIIDBG_MSE16DB,
						  ALX_MSE16DB_UP);
			} else if (speed == SPEED_1000) {
				/*
                                                  
                
     */
				alx_read_phy_dbg(hw, ALX_MIIDBG_MSE20DB,
						 &phy_val);
				ALX_SET_FIELD(phy_val, ALX_MSE20DB_TH,
					      ALX_MSE20DB_TH_HI);
				alx_write_phy_dbg(hw, ALX_MIIDBG_MSE20DB,
						  phy_val);
			}
		}
	} else {
		alx_read_phy_ext(hw, ALX_MIIEXT_ANEG, ALX_MIIEXT_AFE,
				 &phy_val);
		alx_write_phy_ext(hw, ALX_MIIEXT_ANEG, ALX_MIIEXT_AFE,
				  phy_val & ~ALX_AFE_10BT_100M_TH);

		if (adj_th && hw->lnk_patch) {
			alx_write_phy_dbg(hw, ALX_MIIDBG_MSE16DB,
					  ALX_MSE16DB_DOWN);
			alx_read_phy_dbg(hw, ALX_MIIDBG_MSE20DB, &phy_val);
			ALX_SET_FIELD(phy_val, ALX_MSE20DB_TH,
				      ALX_MSE20DB_TH_DEF);
			alx_write_phy_dbg(hw, ALX_MIIDBG_MSE20DB, phy_val);
		}
	}
}


/*      
                                                                  
                                                                         
 */
int alx_pre_suspend(struct alx_hw *hw, int speed)
{
	u32 master, mac, phy, val;
	int err = 0;

	master = alx_read_mem32(hw, ALX_MASTER);
	master &= ~ALX_MASTER_PCLKSEL_SRDS;
	mac = hw->rx_ctrl;
	/*             */
	ALX_SET_FIELD(mac, ALX_MAC_CTRL_SPEED,  ALX_MAC_CTRL_SPEED_10_100);
	mac &= ~(ALX_MAC_CTRL_FULLD | ALX_MAC_CTRL_RX_EN | ALX_MAC_CTRL_TX_EN);

	phy = alx_read_mem32(hw, ALX_PHY_CTRL);
	phy &= ~(ALX_PHY_CTRL_DSPRST_OUT | ALX_PHY_CTRL_CLS);
	phy |= ALX_PHY_CTRL_RST_ANALOG | ALX_PHY_CTRL_HIB_PULSE |
	       ALX_PHY_CTRL_HIB_EN;

	/*                       */
	if (!(hw->sleep_ctrl & ALX_SLEEP_ACTIVE)) {
		err = alx_write_phy_reg(hw, ALX_MII_IER, 0);
		if (err)
			return err;
		phy |= ALX_PHY_CTRL_IDDQ | ALX_PHY_CTRL_POWER_DOWN;
	} else {
		if (hw->sleep_ctrl & (ALX_SLEEP_WOL_MAGIC | ALX_SLEEP_CIFS))
			mac |= ALX_MAC_CTRL_RX_EN | ALX_MAC_CTRL_BRD_EN;
		if (hw->sleep_ctrl & ALX_SLEEP_CIFS)
			mac |= ALX_MAC_CTRL_TX_EN;
		if (speed % 10 == DUPLEX_FULL)
			mac |= ALX_MAC_CTRL_FULLD;
		if (speed >= SPEED_1000)
			ALX_SET_FIELD(mac, ALX_MAC_CTRL_SPEED,
				      ALX_MAC_CTRL_SPEED_1000);
		phy |= ALX_PHY_CTRL_DSPRST_OUT;
		err = alx_write_phy_ext(hw, ALX_MIIEXT_ANEG,
					ALX_MIIEXT_S3DIG10,
					ALX_MIIEXT_S3DIG10_SL);
		if (err)
			return err;
	}

	alx_enable_osc(hw);
	hw->rx_ctrl = mac;
	alx_write_mem32(hw, ALX_MASTER, master);
	alx_write_mem32(hw, ALX_MAC_CTRL, mac);
	alx_write_mem32(hw, ALX_PHY_CTRL, phy);

	/*                          */
	val = alx_read_mem32(hw, ALX_PDLL_TRNS1);
	val |= ALX_PDLL_TRNS1_D3PLLOFF_EN;
	alx_write_mem32(hw, ALX_PDLL_TRNS1, val);

	return 0;
}

bool alx_phy_configured(struct alx_hw *hw)
{
	u32 cfg, hw_cfg;

	cfg = ethadv_to_hw_cfg(hw, hw->adv_cfg);
	cfg = ALX_GET_FIELD(cfg, ALX_DRV_PHY);
	hw_cfg = alx_get_phy_config(hw);

	if (hw_cfg == ALX_DRV_PHY_UNKNOWN)
		return false;

	return cfg == hw_cfg;
}

int alx_get_phy_link(struct alx_hw *hw, int *speed)
{
	struct pci_dev *pdev = hw->pdev;
	u16 bmsr, giga;
	int err;

	err = alx_read_phy_reg(hw, MII_BMSR, &bmsr);
	if (err)
		return err;

	err = alx_read_phy_reg(hw, MII_BMSR, &bmsr);
	if (err)
		return err;

	if (!(bmsr & BMSR_LSTATUS)) {
		*speed = SPEED_UNKNOWN;
		return 0;
	}

	/*                                                              */
	err = alx_read_phy_reg(hw, ALX_MII_GIGA_PSSR, &giga);
	if (err)
		return err;

	if (!(giga & ALX_GIGA_PSSR_SPD_DPLX_RESOLVED))
		goto wrong_speed;

	switch (giga & ALX_GIGA_PSSR_SPEED) {
	case ALX_GIGA_PSSR_1000MBS:
		*speed = SPEED_1000;
		break;
	case ALX_GIGA_PSSR_100MBS:
		*speed = SPEED_100;
		break;
	case ALX_GIGA_PSSR_10MBS:
		*speed = SPEED_10;
		break;
	default:
		goto wrong_speed;
	}

	*speed += (giga & ALX_GIGA_PSSR_DPLX) ? DUPLEX_FULL : DUPLEX_HALF;
	return 1;

wrong_speed:
	dev_err(&pdev->dev, "invalid PHY speed/duplex: 0x%x\n", giga);
	return -EINVAL;
}

int alx_clear_phy_intr(struct alx_hw *hw)
{
	u16 isr;

	/*                                      */
	return alx_read_phy_reg(hw, ALX_MII_ISR, &isr);
}

int alx_config_wol(struct alx_hw *hw)
{
	u32 wol = 0;
	int err = 0;

	/*                            */
	if (hw->sleep_ctrl & ALX_SLEEP_WOL_MAGIC)
		wol |= ALX_WOL0_MAGIC_EN | ALX_WOL0_PME_MAGIC_EN;

	/*                       */
	if (hw->sleep_ctrl & ALX_SLEEP_WOL_PHY) {
		wol |=  ALX_WOL0_LINK_EN | ALX_WOL0_PME_LINK;
		/*                          */
		err = alx_write_phy_reg(hw, ALX_MII_IER, ALX_IER_LINK_UP);
	}
	alx_write_mem32(hw, ALX_WOL0, wol);

	return err;
}

void alx_disable_rss(struct alx_hw *hw)
{
	u32 ctrl = alx_read_mem32(hw, ALX_RXQ0);

	ctrl &= ~ALX_RXQ0_RSS_HASH_EN;
	alx_write_mem32(hw, ALX_RXQ0, ctrl);
}

void alx_configure_basic(struct alx_hw *hw)
{
	u32 val, raw_mtu, max_payload;
	u16 val16;
	u8 chip_rev = alx_hw_revision(hw);

	alx_set_macaddr(hw, hw->mac_addr);

	alx_write_mem32(hw, ALX_CLK_GATE, ALX_CLK_GATE_ALL);

	/*                                 */
	if (chip_rev >= ALX_REV_B0)
		alx_write_mem32(hw, ALX_IDLE_DECISN_TIMER,
				ALX_IDLE_DECISN_TIMER_DEF);

	alx_write_mem32(hw, ALX_SMB_TIMER, hw->smb_timer * 500UL);

	val = alx_read_mem32(hw, ALX_MASTER);
	val |= ALX_MASTER_IRQMOD2_EN |
	       ALX_MASTER_IRQMOD1_EN |
	       ALX_MASTER_SYSALVTIMER_EN;
	alx_write_mem32(hw, ALX_MASTER, val);
	alx_write_mem32(hw, ALX_IRQ_MODU_TIMER,
			(hw->imt >> 1) << ALX_IRQ_MODU_TIMER1_SHIFT);
	/*                      */
	alx_write_mem32(hw, ALX_INT_RETRIG, ALX_INT_RETRIG_TO);
	/*                           */
	alx_write_mem32(hw, ALX_TINT_TPD_THRSHLD, hw->ith_tpd);
	alx_write_mem32(hw, ALX_TINT_TIMER, hw->imt);

	raw_mtu = hw->mtu + ETH_HLEN;
	alx_write_mem32(hw, ALX_MTU, raw_mtu + 8);
	if (raw_mtu > ALX_MTU_JUMBO_TH)
		hw->rx_ctrl &= ~ALX_MAC_CTRL_FAST_PAUSE;

	if ((raw_mtu + 8) < ALX_TXQ1_JUMBO_TSO_TH)
		val = (raw_mtu + 8 + 7) >> 3;
	else
		val = ALX_TXQ1_JUMBO_TSO_TH >> 3;
	alx_write_mem32(hw, ALX_TXQ1, val | ALX_TXQ1_ERRLGPKT_DROP_EN);

	max_payload = pcie_get_readrq(hw->pdev) >> 8;
	/*
                                                        
                               
  */
	if (max_payload < ALX_DEV_CTRL_MAXRRS_MIN)
		pcie_set_readrq(hw->pdev, 128 << ALX_DEV_CTRL_MAXRRS_MIN);

	val = ALX_TXQ_TPD_BURSTPREF_DEF << ALX_TXQ0_TPD_BURSTPREF_SHIFT |
	      ALX_TXQ0_MODE_ENHANCE | ALX_TXQ0_LSO_8023_EN |
	      ALX_TXQ0_SUPT_IPOPT |
	      ALX_TXQ_TXF_BURST_PREF_DEF << ALX_TXQ0_TXF_BURST_PREF_SHIFT;
	alx_write_mem32(hw, ALX_TXQ0, val);
	val = ALX_TXQ_TPD_BURSTPREF_DEF << ALX_HQTPD_Q1_NUMPREF_SHIFT |
	      ALX_TXQ_TPD_BURSTPREF_DEF << ALX_HQTPD_Q2_NUMPREF_SHIFT |
	      ALX_TXQ_TPD_BURSTPREF_DEF << ALX_HQTPD_Q3_NUMPREF_SHIFT |
	      ALX_HQTPD_BURST_EN;
	alx_write_mem32(hw, ALX_HQTPD, val);

	/*                   */
	val = alx_read_mem32(hw, ALX_SRAM5);
	val = ALX_GET_FIELD(val, ALX_SRAM_RXF_LEN) << 3;
	if (val > ALX_SRAM_RXF_LEN_8K) {
		val16 = ALX_MTU_STD_ALGN >> 3;
		val = (val - ALX_RXQ2_RXF_FLOW_CTRL_RSVD) >> 3;
	} else {
		val16 = ALX_MTU_STD_ALGN >> 3;
		val = (val - ALX_MTU_STD_ALGN) >> 3;
	}
	alx_write_mem32(hw, ALX_RXQ2,
			val16 << ALX_RXQ2_RXF_XOFF_THRESH_SHIFT |
			val << ALX_RXQ2_RXF_XON_THRESH_SHIFT);
	val = ALX_RXQ0_NUM_RFD_PREF_DEF << ALX_RXQ0_NUM_RFD_PREF_SHIFT |
	      ALX_RXQ0_RSS_MODE_DIS << ALX_RXQ0_RSS_MODE_SHIFT |
	      ALX_RXQ0_IDT_TBL_SIZE_DEF << ALX_RXQ0_IDT_TBL_SIZE_SHIFT |
	      ALX_RXQ0_RSS_HSTYP_ALL | ALX_RXQ0_RSS_HASH_EN |
	      ALX_RXQ0_IPV6_PARSE_EN;

	if (alx_hw_giga(hw))
		ALX_SET_FIELD(val, ALX_RXQ0_ASPM_THRESH,
			      ALX_RXQ0_ASPM_THRESH_100M);

	alx_write_mem32(hw, ALX_RXQ0, val);

	val = alx_read_mem32(hw, ALX_DMA);
	val = ALX_DMA_RORDER_MODE_OUT << ALX_DMA_RORDER_MODE_SHIFT |
	      ALX_DMA_RREQ_PRI_DATA |
	      max_payload << ALX_DMA_RREQ_BLEN_SHIFT |
	      ALX_DMA_WDLY_CNT_DEF << ALX_DMA_WDLY_CNT_SHIFT |
	      ALX_DMA_RDLY_CNT_DEF << ALX_DMA_RDLY_CNT_SHIFT |
	      (hw->dma_chnl - 1) << ALX_DMA_RCHNL_SEL_SHIFT;
	alx_write_mem32(hw, ALX_DMA, val);

	/*                            */
	val = ALX_WRR_PRI_RESTRICT_NONE << ALX_WRR_PRI_SHIFT |
	      4 << ALX_WRR_PRI0_SHIFT |
	      4 << ALX_WRR_PRI1_SHIFT |
	      4 << ALX_WRR_PRI2_SHIFT |
	      4 << ALX_WRR_PRI3_SHIFT;
	alx_write_mem32(hw, ALX_WRR, val);
}

static inline u32 alx_speed_to_ethadv(int speed)
{
	switch (speed) {
	case SPEED_1000 + DUPLEX_FULL:
		return ADVERTISED_1000baseT_Full;
	case SPEED_100 + DUPLEX_FULL:
		return ADVERTISED_100baseT_Full;
	case SPEED_100 + DUPLEX_HALF:
		return ADVERTISED_10baseT_Half;
	case SPEED_10 + DUPLEX_FULL:
		return ADVERTISED_10baseT_Full;
	case SPEED_10 + DUPLEX_HALF:
		return ADVERTISED_10baseT_Half;
	default:
		return 0;
	}
}

int alx_select_powersaving_speed(struct alx_hw *hw, int *speed)
{
	int i, err, spd;
	u16 lpa;

	err = alx_get_phy_link(hw, &spd);
	if (err < 0)
		return err;

	if (spd == SPEED_UNKNOWN)
		return 0;

	err = alx_read_phy_reg(hw, MII_LPA, &lpa);
	if (err)
		return err;

	if (!(lpa & LPA_LPACK)) {
		*speed = spd;
		return 0;
	}

	if (lpa & LPA_10FULL)
		*speed = SPEED_10 + DUPLEX_FULL;
	else if (lpa & LPA_10HALF)
		*speed = SPEED_10 + DUPLEX_HALF;
	else if (lpa & LPA_100FULL)
		*speed = SPEED_100 + DUPLEX_FULL;
	else
		*speed = SPEED_100 + DUPLEX_HALF;

	if (*speed != spd) {
		err = alx_write_phy_reg(hw, ALX_MII_IER, 0);
		if (err)
			return err;
		err = alx_setup_speed_duplex(hw,
					     alx_speed_to_ethadv(*speed) |
					     ADVERTISED_Autoneg,
					     ALX_FC_ANEG | ALX_FC_RX |
					     ALX_FC_TX);
		if (err)
			return err;

		/*                 */
		for (i = 0; i < ALX_MAX_SETUP_LNK_CYCLE; i++) {
			int speed2;

			msleep(100);

			err = alx_get_phy_link(hw, &speed2);
			if (err < 0)
				return err;
			if (speed2 != SPEED_UNKNOWN)
				break;
		}
		if (i == ALX_MAX_SETUP_LNK_CYCLE)
			return -ETIMEDOUT;
	}

	return 0;
}

bool alx_get_phy_info(struct alx_hw *hw)
{
	u16  devs1, devs2;

	if (alx_read_phy_reg(hw, MII_PHYSID1, &hw->phy_id[0]) ||
	    alx_read_phy_reg(hw, MII_PHYSID2, &hw->phy_id[1]))
		return false;

	/*                                                    
                                                 
                               
  */
	if (alx_read_phy_ext(hw, 3, MDIO_DEVS1, &devs1) ||
	    alx_read_phy_ext(hw, 3, MDIO_DEVS2, &devs2))
		return false;
	hw->mdio.mmds = devs1 | devs2 << 16;

	return true;
}
