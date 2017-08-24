/*
 * ASIX AX8817X based USB 2.0 Ethernet Devices
 * Copyright (C) 2003-2006 David Hollis <dhollis@davehollis.com>
 * Copyright (C) 2005 Phil Chang <pchang23@sbcglobal.net>
 * Copyright (C) 2006 James Painter <jamie.painter@iname.com>
 * Copyright (c) 2002-2003 TiVo Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _ASIX_H
#define _ASIX_H

//                                                   
//                                            

#include <linux/module.h>
#include <linux/kmod.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/workqueue.h>
#include <linux/mii.h>
#include <linux/usb.h>
#include <linux/crc32.h>
#include <linux/usb/usbnet.h>
#include <linux/slab.h>
#include <linux/if_vlan.h>

#define DRIVER_VERSION "22-Dec-2011"
#define DRIVER_NAME "asix"

/*                                             */

#define AX_CMD_SET_SW_MII		0x06
#define AX_CMD_READ_MII_REG		0x07
#define AX_CMD_WRITE_MII_REG		0x08
#define AX_CMD_SET_HW_MII		0x0a
#define AX_CMD_READ_EEPROM		0x0b
#define AX_CMD_WRITE_EEPROM		0x0c
#define AX_CMD_WRITE_ENABLE		0x0d
#define AX_CMD_WRITE_DISABLE		0x0e
#define AX_CMD_READ_RX_CTL		0x0f
#define AX_CMD_WRITE_RX_CTL		0x10
#define AX_CMD_READ_IPG012		0x11
#define AX_CMD_WRITE_IPG0		0x12
#define AX_CMD_WRITE_IPG1		0x13
#define AX_CMD_READ_NODE_ID		0x13
#define AX_CMD_WRITE_NODE_ID		0x14
#define AX_CMD_WRITE_IPG2		0x14
#define AX_CMD_WRITE_MULTI_FILTER	0x16
#define AX88172_CMD_READ_NODE_ID	0x17
#define AX_CMD_READ_PHY_ID		0x19
#define AX_CMD_READ_MEDIUM_STATUS	0x1a
#define AX_CMD_WRITE_MEDIUM_MODE	0x1b
#define AX_CMD_READ_MONITOR_MODE	0x1c
#define AX_CMD_WRITE_MONITOR_MODE	0x1d
#define AX_CMD_READ_GPIOS		0x1e
#define AX_CMD_WRITE_GPIOS		0x1f
#define AX_CMD_SW_RESET			0x20
#define AX_CMD_SW_PHY_STATUS		0x21
#define AX_CMD_SW_PHY_SELECT		0x22

#define AX_PHY_SELECT_MASK		(BIT(3) | BIT(2))
#define AX_PHY_SELECT_INTERNAL		0
#define AX_PHY_SELECT_EXTERNAL		BIT(2)

#define AX_MONITOR_MODE			0x01
#define AX_MONITOR_LINK			0x02
#define AX_MONITOR_MAGIC		0x04
#define AX_MONITOR_HSFS			0x10

/*                                       */
#define AX88172_MEDIUM_FD		0x02
#define AX88172_MEDIUM_TX		0x04
#define AX88172_MEDIUM_FC		0x10
#define AX88172_MEDIUM_DEFAULT \
		( AX88172_MEDIUM_FD | AX88172_MEDIUM_TX | AX88172_MEDIUM_FC )

#define AX_MCAST_FILTER_SIZE		8
#define AX_MAX_MCAST			64

#define AX_SWRESET_CLEAR		0x00
#define AX_SWRESET_RR			0x01
#define AX_SWRESET_RT			0x02
#define AX_SWRESET_PRTE			0x04
#define AX_SWRESET_PRL			0x08
#define AX_SWRESET_BZ			0x10
#define AX_SWRESET_IPRL			0x20
#define AX_SWRESET_IPPD			0x40

#define AX88772_IPG0_DEFAULT		0x15
#define AX88772_IPG1_DEFAULT		0x0c
#define AX88772_IPG2_DEFAULT		0x12

/*                                        */
#define AX_MEDIUM_PF		0x0080
#define AX_MEDIUM_JFE		0x0040
#define AX_MEDIUM_TFC		0x0020
#define AX_MEDIUM_RFC		0x0010
#define AX_MEDIUM_ENCK		0x0008
#define AX_MEDIUM_AC		0x0004
#define AX_MEDIUM_FD		0x0002
#define AX_MEDIUM_GM		0x0001
#define AX_MEDIUM_SM		0x1000
#define AX_MEDIUM_SBP		0x0800
#define AX_MEDIUM_PS		0x0200
#define AX_MEDIUM_RE		0x0100

#define AX88178_MEDIUM_DEFAULT	\
	(AX_MEDIUM_PS | AX_MEDIUM_FD | AX_MEDIUM_AC | \
	 AX_MEDIUM_RFC | AX_MEDIUM_TFC | AX_MEDIUM_JFE | \
	 AX_MEDIUM_RE)

#define AX88772_MEDIUM_DEFAULT	\
	(AX_MEDIUM_FD | AX_MEDIUM_RFC | \
	 AX_MEDIUM_TFC | AX_MEDIUM_PS | \
	 AX_MEDIUM_AC | AX_MEDIUM_RE)

/*                                 */
#define AX_RX_CTL_SO		0x0080
#define AX_RX_CTL_AP		0x0020
#define AX_RX_CTL_AM		0x0010
#define AX_RX_CTL_AB		0x0008
#define AX_RX_CTL_SEP		0x0004
#define AX_RX_CTL_AMALL		0x0002
#define AX_RX_CTL_PRO		0x0001
#define AX_RX_CTL_MFB_2048	0x0000
#define AX_RX_CTL_MFB_4096	0x0100
#define AX_RX_CTL_MFB_8192	0x0200
#define AX_RX_CTL_MFB_16384	0x0300

#define AX_DEFAULT_RX_CTL	(AX_RX_CTL_SO | AX_RX_CTL_AB)

/*                     */
#define AX_GPIO_GPO0EN		0x01	/*                     */
#define AX_GPIO_GPO_0		0x02	/*                    */
#define AX_GPIO_GPO1EN		0x04	/*                     */
#define AX_GPIO_GPO_1		0x08	/*                    */
#define AX_GPIO_GPO2EN		0x10	/*                     */
#define AX_GPIO_GPO_2		0x20	/*                    */
#define AX_GPIO_RESERVED	0x40	/*          */
#define AX_GPIO_RSE		0x80	/*                      */

#define AX_EEPROM_MAGIC		0xdeadbeef
#define AX_EEPROM_LEN		0x200

/*                                                                     */
struct asix_data {
	u8 multi_filter[AX_MCAST_FILTER_SIZE];
	u8 mac_addr[ETH_ALEN];
	u8 phymode;
	u8 ledmode;
	u8 res;
};

struct asix_rx_fixup_info {
	struct sk_buff *ax_skb;
	u32 header;
	u16 size;
	bool split_head;
};

struct asix_common_private {
	struct asix_rx_fixup_info rx_fixup_info;
};

/*                     */
#define FLAG_EEPROM_MAC		(1UL << 0)  /*                             */

int asix_read_cmd(struct usbnet *dev, u8 cmd, u16 value, u16 index,
		  u16 size, void *data);

int asix_write_cmd(struct usbnet *dev, u8 cmd, u16 value, u16 index,
		   u16 size, void *data);

void asix_write_cmd_async(struct usbnet *dev, u8 cmd, u16 value,
			  u16 index, u16 size, void *data);

int asix_rx_fixup_internal(struct usbnet *dev, struct sk_buff *skb,
			   struct asix_rx_fixup_info *rx);
int asix_rx_fixup_common(struct usbnet *dev, struct sk_buff *skb);

struct sk_buff *asix_tx_fixup(struct usbnet *dev, struct sk_buff *skb,
			      gfp_t flags);

int asix_set_sw_mii(struct usbnet *dev);
int asix_set_hw_mii(struct usbnet *dev);

int asix_read_phy_addr(struct usbnet *dev, int internal);
int asix_get_phy_addr(struct usbnet *dev);

int asix_sw_reset(struct usbnet *dev, u8 flags);

u16 asix_read_rx_ctl(struct usbnet *dev);
int asix_write_rx_ctl(struct usbnet *dev, u16 mode);

u16 asix_read_medium_status(struct usbnet *dev);
int asix_write_medium_mode(struct usbnet *dev, u16 mode);

int asix_write_gpio(struct usbnet *dev, u16 value, int sleep);

void asix_set_multicast(struct net_device *net);

int asix_mdio_read(struct net_device *netdev, int phy_id, int loc);
void asix_mdio_write(struct net_device *netdev, int phy_id, int loc, int val);

void asix_get_wol(struct net_device *net, struct ethtool_wolinfo *wolinfo);
int asix_set_wol(struct net_device *net, struct ethtool_wolinfo *wolinfo);

int asix_get_eeprom_len(struct net_device *net);
int asix_get_eeprom(struct net_device *net, struct ethtool_eeprom *eeprom,
		    u8 *data);
int asix_set_eeprom(struct net_device *net, struct ethtool_eeprom *eeprom,
		    u8 *data);

void asix_get_drvinfo(struct net_device *net, struct ethtool_drvinfo *info);

int asix_set_mac_address(struct net_device *net, void *p);

#endif /*         */
