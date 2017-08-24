#ifndef __LINUX_SMSCPHY_H__
#define __LINUX_SMSCPHY_H__

#define MII_LAN83C185_ISF 29 /*                        */
#define MII_LAN83C185_IM  30 /*                */
#define MII_LAN83C185_CTRL_STATUS 17 /*                      */
#define MII_LAN83C185_SPECIAL_MODES 18 /*                        */

#define MII_LAN83C185_ISF_INT1 (1<<1) /*                                */
#define MII_LAN83C185_ISF_INT2 (1<<2) /*                          */
#define MII_LAN83C185_ISF_INT3 (1<<3) /*                         */
#define MII_LAN83C185_ISF_INT4 (1<<4) /*           */
#define MII_LAN83C185_ISF_INT5 (1<<5) /*                       */
#define MII_LAN83C185_ISF_INT6 (1<<6) /*                           */
#define MII_LAN83C185_ISF_INT7 (1<<7) /*          */

#define MII_LAN83C185_ISF_INT_ALL (0x0e)

#define MII_LAN83C185_ISF_INT_PHYLIB_EVENTS \
	(MII_LAN83C185_ISF_INT6 | MII_LAN83C185_ISF_INT4 | \
	 MII_LAN83C185_ISF_INT7)

#define MII_LAN83C185_EDPWRDOWN (1 << 13) /*           */
#define MII_LAN83C185_ENERGYON  (1 << 1)  /*          */

#define MII_LAN83C185_MODE_MASK      0xE0
#define MII_LAN83C185_MODE_POWERDOWN 0xC0 /*                 */
#define MII_LAN83C185_MODE_ALL       0xE0 /*                  */

#endif /*                     */
