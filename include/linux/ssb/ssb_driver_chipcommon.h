#ifndef LINUX_SSB_CHIPCO_H_
#define LINUX_SSB_CHIPCO_H_

/* SonicsSiliconBackplane CHIPCOMMON core hardware definitions
 *
 * The chipcommon core provides chip identification, SB control,
 * jtag, 0/1/2 uarts, clock frequency control, a watchdog interrupt timer,
 * gpio interface, extbus, and support for serial and parallel flashes.
 *
 * Copyright 2005, Broadcom Corporation
 * Copyright 2006, Michael Buesch <m@bues.ch>
 *
 * Licensed under the GPL version 2. See COPYING for details.
 */

/*                              */

#define SSB_CHIPCO_CHIPID		0x0000
#define  SSB_CHIPCO_IDMASK		0x0000FFFF
#define  SSB_CHIPCO_REVMASK		0x000F0000
#define  SSB_CHIPCO_REVSHIFT		16
#define  SSB_CHIPCO_PACKMASK		0x00F00000
#define  SSB_CHIPCO_PACKSHIFT		20
#define  SSB_CHIPCO_NRCORESMASK		0x0F000000
#define  SSB_CHIPCO_NRCORESSHIFT	24
#define SSB_CHIPCO_CAP	 		0x0004		/*              */
#define  SSB_CHIPCO_CAP_NRUART		0x00000003	/*            */
#define  SSB_CHIPCO_CAP_MIPSEB		0x00000004	/*                        */
#define  SSB_CHIPCO_CAP_UARTCLK		0x00000018	/*                   */
#define   SSB_CHIPCO_CAP_UARTCLK_INT	0x00000008	/*                                            */
#define  SSB_CHIPCO_CAP_UARTGPIO	0x00000020	/*                     */
#define  SSB_CHIPCO_CAP_EXTBUS		0x000000C0	/*                        */
#define  SSB_CHIPCO_CAP_FLASHT		0x00000700	/*            */
#define   SSB_CHIPCO_FLASHT_NONE	0x00000000	/*          */
#define   SSB_CHIPCO_FLASHT_STSER	0x00000100	/*                 */
#define   SSB_CHIPCO_FLASHT_ATSER	0x00000200	/*                    */
#define	  SSB_CHIPCO_FLASHT_PARA	0x00000700	/*                */
#define  SSB_CHIPCO_CAP_PLLT		0x00038000	/*          */
#define   SSB_PLLTYPE_NONE		0x00000000
#define   SSB_PLLTYPE_1			0x00010000	/*                        */
#define   SSB_PLLTYPE_2			0x00020000	/*                   */
#define   SSB_PLLTYPE_3			0x00030000	/*                   */
#define   SSB_PLLTYPE_4			0x00008000	/*                   */
#define   SSB_PLLTYPE_5			0x00018000	/*                   */
#define   SSB_PLLTYPE_6			0x00028000	/*                         */
#define   SSB_PLLTYPE_7			0x00038000	/*                   */
#define  SSB_CHIPCO_CAP_PCTL		0x00040000	/*               */
#define  SSB_CHIPCO_CAP_OTPS		0x00380000	/*          */
#define  SSB_CHIPCO_CAP_OTPS_SHIFT	19
#define  SSB_CHIPCO_CAP_OTPS_BASE	5
#define  SSB_CHIPCO_CAP_JTAGM		0x00400000	/*                     */
#define  SSB_CHIPCO_CAP_BROM		0x00800000	/*                          */
#define  SSB_CHIPCO_CAP_64BIT		0x08000000	/*                  */
#define  SSB_CHIPCO_CAP_PMU		0x10000000	/*                           */
#define  SSB_CHIPCO_CAP_ECI		0x20000000	/*                           */
#define  SSB_CHIPCO_CAP_SPROM		0x40000000	/*               */
#define SSB_CHIPCO_CORECTL		0x0008
#define  SSB_CHIPCO_CORECTL_UARTCLK0	0x00000001	/*                                */
#define	 SSB_CHIPCO_CORECTL_SE		0x00000002	/*                                    */
#define  SSB_CHIPCO_CORECTL_UARTCLKEN	0x00000008	/*                               */
#define SSB_CHIPCO_BIST			0x000C
#define SSB_CHIPCO_OTPS			0x0010		/*            */
#define	 SSB_CHIPCO_OTPS_PROGFAIL	0x80000000
#define	 SSB_CHIPCO_OTPS_PROTECT	0x00000007
#define	 SSB_CHIPCO_OTPS_HW_PROTECT	0x00000001
#define	 SSB_CHIPCO_OTPS_SW_PROTECT	0x00000002
#define	 SSB_CHIPCO_OTPS_CID_PROTECT	0x00000004
#define SSB_CHIPCO_OTPC			0x0014		/*             */
#define	 SSB_CHIPCO_OTPC_RECWAIT	0xFF000000
#define	 SSB_CHIPCO_OTPC_PROGWAIT	0x00FFFF00
#define	 SSB_CHIPCO_OTPC_PRW_SHIFT	8
#define	 SSB_CHIPCO_OTPC_MAXFAIL	0x00000038
#define	 SSB_CHIPCO_OTPC_VSEL		0x00000006
#define	 SSB_CHIPCO_OTPC_SELVL		0x00000001
#define SSB_CHIPCO_OTPP			0x0018		/*          */
#define	 SSB_CHIPCO_OTPP_COL		0x000000FF
#define	 SSB_CHIPCO_OTPP_ROW		0x0000FF00
#define	 SSB_CHIPCO_OTPP_ROW_SHIFT	8
#define	 SSB_CHIPCO_OTPP_READERR	0x10000000
#define	 SSB_CHIPCO_OTPP_VALUE		0x20000000
#define	 SSB_CHIPCO_OTPP_READ		0x40000000
#define	 SSB_CHIPCO_OTPP_START		0x80000000
#define	 SSB_CHIPCO_OTPP_BUSY		0x80000000
#define SSB_CHIPCO_IRQSTAT		0x0020
#define SSB_CHIPCO_IRQMASK		0x0024
#define	 SSB_CHIPCO_IRQ_GPIO		0x00000001	/*           */
#define	 SSB_CHIPCO_IRQ_EXT		0x00000002	/*                                 */
#define	 SSB_CHIPCO_IRQ_WDRESET		0x80000000	/*                         */
#define SSB_CHIPCO_CHIPCTL		0x0028		/*                */
#define SSB_CHIPCO_CHIPSTAT		0x002C		/*                */
#define SSB_CHIPCO_JCMD			0x0030		/*                */
#define  SSB_CHIPCO_JCMD_START		0x80000000
#define  SSB_CHIPCO_JCMD_BUSY		0x80000000
#define  SSB_CHIPCO_JCMD_PAUSE		0x40000000
#define  SSB_CHIPCO_JCMD0_ACC_MASK	0x0000F000
#define  SSB_CHIPCO_JCMD0_ACC_IRDR	0x00000000
#define  SSB_CHIPCO_JCMD0_ACC_DR	0x00001000
#define  SSB_CHIPCO_JCMD0_ACC_IR	0x00002000
#define  SSB_CHIPCO_JCMD0_ACC_RESET	0x00003000
#define  SSB_CHIPCO_JCMD0_ACC_IRPDR	0x00004000
#define  SSB_CHIPCO_JCMD0_ACC_PDR	0x00005000
#define  SSB_CHIPCO_JCMD0_IRW_MASK	0x00000F00
#define  SSB_CHIPCO_JCMD_ACC_MASK	0x000F0000	/*                        */
#define  SSB_CHIPCO_JCMD_ACC_IRDR	0x00000000
#define  SSB_CHIPCO_JCMD_ACC_DR		0x00010000
#define  SSB_CHIPCO_JCMD_ACC_IR		0x00020000
#define  SSB_CHIPCO_JCMD_ACC_RESET	0x00030000
#define  SSB_CHIPCO_JCMD_ACC_IRPDR	0x00040000
#define  SSB_CHIPCO_JCMD_ACC_PDR	0x00050000
#define  SSB_CHIPCO_JCMD_IRW_MASK	0x00001F00
#define  SSB_CHIPCO_JCMD_IRW_SHIFT	8
#define  SSB_CHIPCO_JCMD_DRW_MASK	0x0000003F
#define SSB_CHIPCO_JIR			0x0034		/*                */
#define SSB_CHIPCO_JDR			0x0038		/*                */
#define SSB_CHIPCO_JCTL			0x003C		/*                */
#define  SSB_CHIPCO_JCTL_FORCE_CLK	4		/*             */
#define  SSB_CHIPCO_JCTL_EXT_EN		2		/*                         */
#define  SSB_CHIPCO_JCTL_EN		1		/*                    */
#define SSB_CHIPCO_FLASHCTL		0x0040
#define  SSB_CHIPCO_FLASHCTL_START	0x80000000
#define  SSB_CHIPCO_FLASHCTL_BUSY	SSB_CHIPCO_FLASHCTL_START
#define SSB_CHIPCO_FLASHADDR		0x0044
#define SSB_CHIPCO_FLASHDATA		0x0048
#define SSB_CHIPCO_BCAST_ADDR		0x0050
#define SSB_CHIPCO_BCAST_DATA		0x0054
#define SSB_CHIPCO_GPIOPULLUP		0x0058		/*                */
#define SSB_CHIPCO_GPIOPULLDOWN		0x005C		/*                */
#define SSB_CHIPCO_GPIOIN		0x0060
#define SSB_CHIPCO_GPIOOUT		0x0064
#define SSB_CHIPCO_GPIOOUTEN		0x0068
#define SSB_CHIPCO_GPIOCTL		0x006C
#define SSB_CHIPCO_GPIOPOL		0x0070
#define SSB_CHIPCO_GPIOIRQ		0x0074
#define SSB_CHIPCO_WATCHDOG		0x0080
#define SSB_CHIPCO_GPIOTIMER		0x0088		/*                               */
#define  SSB_CHIPCO_GPIOTIMER_OFFTIME	0x0000FFFF
#define  SSB_CHIPCO_GPIOTIMER_OFFTIME_SHIFT	0
#define  SSB_CHIPCO_GPIOTIMER_ONTIME	0xFFFF0000
#define  SSB_CHIPCO_GPIOTIMER_ONTIME_SHIFT	16
#define SSB_CHIPCO_GPIOTOUTM		0x008C		/*                               */
#define SSB_CHIPCO_CLOCK_N		0x0090
#define SSB_CHIPCO_CLOCK_SB		0x0094
#define SSB_CHIPCO_CLOCK_PCI		0x0098
#define SSB_CHIPCO_CLOCK_M2		0x009C
#define SSB_CHIPCO_CLOCK_MIPS		0x00A0
#define SSB_CHIPCO_CLKDIV		0x00A4		/*               */
#define	 SSB_CHIPCO_CLKDIV_SFLASH	0x0F000000
#define	 SSB_CHIPCO_CLKDIV_SFLASH_SHIFT	24
#define	 SSB_CHIPCO_CLKDIV_OTP		0x000F0000
#define	 SSB_CHIPCO_CLKDIV_OTP_SHIFT	16
#define	 SSB_CHIPCO_CLKDIV_JTAG		0x00000F00
#define	 SSB_CHIPCO_CLKDIV_JTAG_SHIFT	8
#define	 SSB_CHIPCO_CLKDIV_UART		0x000000FF
#define SSB_CHIPCO_PLLONDELAY		0x00B0		/*               */
#define SSB_CHIPCO_FREFSELDELAY		0x00B4		/*               */
#define SSB_CHIPCO_SLOWCLKCTL		0x00B8		/*                    */
#define  SSB_CHIPCO_SLOWCLKCTL_SRC	0x00000007	/*                        */
#define	  SSB_CHIPCO_SLOWCLKCTL_SRC_LPO		0x00000000	/*                             */
#define   SSB_CHIPCO_SLOWCLKCTL_SRC_XTAL	0x00000001	/*                                 */
#define	  SSB_CHIPCO_SLOECLKCTL_SRC_PCI		0x00000002	/*                             */
#define  SSB_CHIPCO_SLOWCLKCTL_LPOFREQ	0x00000200	/*                                 */
#define  SSB_CHIPCO_SLOWCLKCTL_LPOPD	0x00000400	/*                                                     */
#define  SSB_CHIPCO_SLOWCLKCTL_FSLOW	0x00000800	/*                                                                         */
#define  SSB_CHIPCO_SLOWCLKCTL_IPLL	0x00001000	/*                                                                                       */
#define  SSB_CHIPCO_SLOWCLKCTL_ENXTAL	0x00002000	/*                                                                               */
#define  SSB_CHIPCO_SLOWCLKCTL_XTALPU	0x00004000	/*                                            */
#define  SSB_CHIPCO_SLOWCLKCTL_CLKDIV	0xFFFF0000	/*                                        */
#define  SSB_CHIPCO_SLOWCLKCTL_CLKDIV_SHIFT	16
#define SSB_CHIPCO_SYSCLKCTL		0x00C0		/*               */
#define	 SSB_CHIPCO_SYSCLKCTL_IDLPEN	0x00000001	/*                              */
#define	 SSB_CHIPCO_SYSCLKCTL_ALPEN	0x00000002	/*                                */
#define	 SSB_CHIPCO_SYSCLKCTL_PLLEN	0x00000004	/*            */
#define	 SSB_CHIPCO_SYSCLKCTL_FORCEALP	0x00000008	/*                                      */
#define	 SSB_CHIPCO_SYSCLKCTL_FORCEHT	0x00000010	/*          */
#define  SSB_CHIPCO_SYSCLKCTL_CLKDIV	0xFFFF0000	/*                               */
#define  SSB_CHIPCO_SYSCLKCTL_CLKDIV_SHIFT	16
#define SSB_CHIPCO_CLKSTSTR		0x00C4		/*               */
#define SSB_CHIPCO_PCMCIA_CFG		0x0100
#define SSB_CHIPCO_PCMCIA_MEMWAIT	0x0104
#define SSB_CHIPCO_PCMCIA_ATTRWAIT	0x0108
#define SSB_CHIPCO_PCMCIA_IOWAIT	0x010C
#define SSB_CHIPCO_IDE_CFG		0x0110
#define SSB_CHIPCO_IDE_MEMWAIT		0x0114
#define SSB_CHIPCO_IDE_ATTRWAIT		0x0118
#define SSB_CHIPCO_IDE_IOWAIT		0x011C
#define SSB_CHIPCO_PROG_CFG		0x0120
#define SSB_CHIPCO_PROG_WAITCNT		0x0124
#define SSB_CHIPCO_FLASH_CFG		0x0128
#define SSB_CHIPCO_FLASH_WAITCNT	0x012C
#define SSB_CHIPCO_CLKCTLST		0x01E0 /*                                      */
#define  SSB_CHIPCO_CLKCTLST_FORCEALP	0x00000001 /*                   */
#define  SSB_CHIPCO_CLKCTLST_FORCEHT	0x00000002 /*                  */
#define  SSB_CHIPCO_CLKCTLST_FORCEILP	0x00000004 /*                   */
#define  SSB_CHIPCO_CLKCTLST_HAVEALPREQ	0x00000008 /*                       */
#define  SSB_CHIPCO_CLKCTLST_HAVEHTREQ	0x00000010 /*                      */
#define  SSB_CHIPCO_CLKCTLST_HWCROFF	0x00000020 /*                            */
#define  SSB_CHIPCO_CLKCTLST_HAVEALP	0x00010000 /*               */
#define  SSB_CHIPCO_CLKCTLST_HAVEHT	0x00020000 /*              */
#define  SSB_CHIPCO_CLKCTLST_4328A0_HAVEHT	0x00010000 /*                          */
#define  SSB_CHIPCO_CLKCTLST_4328A0_HAVEALP	0x00020000 /*                          */
#define SSB_CHIPCO_HW_WORKAROUND	0x01E4 /*                                 */
#define SSB_CHIPCO_UART0_DATA		0x0300
#define SSB_CHIPCO_UART0_IMR		0x0304
#define SSB_CHIPCO_UART0_FCR		0x0308
#define SSB_CHIPCO_UART0_LCR		0x030C
#define SSB_CHIPCO_UART0_MCR		0x0310
#define SSB_CHIPCO_UART0_LSR		0x0314
#define SSB_CHIPCO_UART0_MSR		0x0318
#define SSB_CHIPCO_UART0_SCRATCH	0x031C
#define SSB_CHIPCO_UART1_DATA		0x0400
#define SSB_CHIPCO_UART1_IMR		0x0404
#define SSB_CHIPCO_UART1_FCR		0x0408
#define SSB_CHIPCO_UART1_LCR		0x040C
#define SSB_CHIPCO_UART1_MCR		0x0410
#define SSB_CHIPCO_UART1_LSR		0x0414
#define SSB_CHIPCO_UART1_MSR		0x0418
#define SSB_CHIPCO_UART1_SCRATCH	0x041C
/*                           */
#define SSB_CHIPCO_PMU_CTL			0x0600 /*             */
#define  SSB_CHIPCO_PMU_CTL_ILP_DIV		0xFFFF0000 /*              */
#define  SSB_CHIPCO_PMU_CTL_ILP_DIV_SHIFT	16
#define  SSB_CHIPCO_PMU_CTL_PLL_UPD		0x00000400
#define  SSB_CHIPCO_PMU_CTL_NOILPONW		0x00000200 /*                */
#define  SSB_CHIPCO_PMU_CTL_HTREQEN		0x00000100 /*               */
#define  SSB_CHIPCO_PMU_CTL_ALPREQEN		0x00000080 /*                */
#define  SSB_CHIPCO_PMU_CTL_XTALFREQ		0x0000007C /*              */
#define  SSB_CHIPCO_PMU_CTL_XTALFREQ_SHIFT	2
#define  SSB_CHIPCO_PMU_CTL_ILPDIVEN		0x00000002 /*                */
#define  SSB_CHIPCO_PMU_CTL_LPOSEL		0x00000001 /*         */
#define SSB_CHIPCO_PMU_CAP			0x0604 /*                  */
#define  SSB_CHIPCO_PMU_CAP_REVISION		0x000000FF /*               */
#define SSB_CHIPCO_PMU_STAT			0x0608 /*            */
#define  SSB_CHIPCO_PMU_STAT_INTPEND		0x00000040 /*                   */
#define  SSB_CHIPCO_PMU_STAT_SBCLKST		0x00000030 /*                         */
#define  SSB_CHIPCO_PMU_STAT_HAVEALP		0x00000008 /*               */
#define  SSB_CHIPCO_PMU_STAT_HAVEHT		0x00000004 /*              */
#define  SSB_CHIPCO_PMU_STAT_RESINIT		0x00000003 /*          */
#define SSB_CHIPCO_PMU_RES_STAT			0x060C /*                */
#define SSB_CHIPCO_PMU_RES_PEND			0x0610 /*                 */
#define SSB_CHIPCO_PMU_TIMER			0x0614 /*           */
#define SSB_CHIPCO_PMU_MINRES_MSK		0x0618 /*                  */
#define SSB_CHIPCO_PMU_MAXRES_MSK		0x061C /*                  */
#define SSB_CHIPCO_PMU_RES_TABSEL		0x0620 /*                   */
#define SSB_CHIPCO_PMU_RES_DEPMSK		0x0624 /*                  */
#define SSB_CHIPCO_PMU_RES_UPDNTM		0x0628 /*                      */
#define SSB_CHIPCO_PMU_RES_TIMER		0x062C /*               */
#define SSB_CHIPCO_PMU_CLKSTRETCH		0x0630 /*                  */
#define SSB_CHIPCO_PMU_WATCHDOG			0x0634 /*              */
#define SSB_CHIPCO_PMU_RES_REQTS		0x0640 /*                       */
#define SSB_CHIPCO_PMU_RES_REQT			0x0644 /*                   */
#define SSB_CHIPCO_PMU_RES_REQM			0x0648 /*                  */
#define SSB_CHIPCO_CHIPCTL_ADDR			0x0650
#define SSB_CHIPCO_CHIPCTL_DATA			0x0654
#define SSB_CHIPCO_REGCTL_ADDR			0x0658
#define SSB_CHIPCO_REGCTL_DATA			0x065C
#define SSB_CHIPCO_PLLCTL_ADDR			0x0660
#define SSB_CHIPCO_PLLCTL_DATA			0x0664



/*                    */

/*                         */
#define SSB_PMU0_PLLCTL0			0
#define  SSB_PMU0_PLLCTL0_PDIV_MSK		0x00000001
#define  SSB_PMU0_PLLCTL0_PDIV_FREQ		25000 /*     */
#define SSB_PMU0_PLLCTL1			1
#define  SSB_PMU0_PLLCTL1_WILD_IMSK		0xF0000000 /*                            */
#define  SSB_PMU0_PLLCTL1_WILD_IMSK_SHIFT	28
#define  SSB_PMU0_PLLCTL1_WILD_FMSK		0x0FFFFF00 /*                */
#define  SSB_PMU0_PLLCTL1_WILD_FMSK_SHIFT	8
#define  SSB_PMU0_PLLCTL1_STOPMOD		0x00000040 /*          */
#define SSB_PMU0_PLLCTL2			2
#define  SSB_PMU0_PLLCTL2_WILD_IMSKHI		0x0000000F /*                             */
#define  SSB_PMU0_PLLCTL2_WILD_IMSKHI_SHIFT	0

/*                         */
#define SSB_PMU1_PLLCTL0			0
#define  SSB_PMU1_PLLCTL0_P1DIV			0x00F00000 /*        */
#define  SSB_PMU1_PLLCTL0_P1DIV_SHIFT		20
#define  SSB_PMU1_PLLCTL0_P2DIV			0x0F000000 /*        */
#define  SSB_PMU1_PLLCTL0_P2DIV_SHIFT		24
#define SSB_PMU1_PLLCTL1			1
#define  SSB_PMU1_PLLCTL1_M1DIV			0x000000FF /*        */
#define  SSB_PMU1_PLLCTL1_M1DIV_SHIFT		0
#define  SSB_PMU1_PLLCTL1_M2DIV			0x0000FF00 /*        */
#define  SSB_PMU1_PLLCTL1_M2DIV_SHIFT		8
#define  SSB_PMU1_PLLCTL1_M3DIV			0x00FF0000 /*        */
#define  SSB_PMU1_PLLCTL1_M3DIV_SHIFT		16
#define  SSB_PMU1_PLLCTL1_M4DIV			0xFF000000 /*        */
#define  SSB_PMU1_PLLCTL1_M4DIV_SHIFT		24
#define SSB_PMU1_PLLCTL2			2
#define  SSB_PMU1_PLLCTL2_M5DIV			0x000000FF /*        */
#define  SSB_PMU1_PLLCTL2_M5DIV_SHIFT		0
#define  SSB_PMU1_PLLCTL2_M6DIV			0x0000FF00 /*        */
#define  SSB_PMU1_PLLCTL2_M6DIV_SHIFT		8
#define  SSB_PMU1_PLLCTL2_NDIVMODE		0x000E0000 /*           */
#define  SSB_PMU1_PLLCTL2_NDIVMODE_SHIFT	17
#define  SSB_PMU1_PLLCTL2_NDIVINT		0x1FF00000 /*          */
#define  SSB_PMU1_PLLCTL2_NDIVINT_SHIFT		20
#define SSB_PMU1_PLLCTL3			3
#define  SSB_PMU1_PLLCTL3_NDIVFRAC		0x00FFFFFF /*           */
#define  SSB_PMU1_PLLCTL3_NDIVFRAC_SHIFT	0
#define SSB_PMU1_PLLCTL4			4
#define SSB_PMU1_PLLCTL5			5
#define  SSB_PMU1_PLLCTL5_CLKDRV		0xFFFFFF00 /*         */
#define  SSB_PMU1_PLLCTL5_CLKDRV_SHIFT		8

/*                               */
#define SSB_PMURES_4312_SWITCHER_BURST		0
#define SSB_PMURES_4312_SWITCHER_PWM    	1
#define SSB_PMURES_4312_PA_REF_LDO		2
#define SSB_PMURES_4312_CORE_LDO_BURST		3
#define SSB_PMURES_4312_CORE_LDO_PWM		4
#define SSB_PMURES_4312_RADIO_LDO		5
#define SSB_PMURES_4312_ILP_REQUEST		6
#define SSB_PMURES_4312_BG_FILTBYP		7
#define SSB_PMURES_4312_TX_FILTBYP		8
#define SSB_PMURES_4312_RX_FILTBYP		9
#define SSB_PMURES_4312_XTAL_PU			10
#define SSB_PMURES_4312_ALP_AVAIL		11
#define SSB_PMURES_4312_BB_PLL_FILTBYP		12
#define SSB_PMURES_4312_RF_PLL_FILTBYP		13
#define SSB_PMURES_4312_HT_AVAIL		14

/*                               */
#define SSB_PMURES_4325_BUCK_BOOST_BURST	0
#define SSB_PMURES_4325_CBUCK_BURST		1
#define SSB_PMURES_4325_CBUCK_PWM		2
#define SSB_PMURES_4325_CLDO_CBUCK_BURST	3
#define SSB_PMURES_4325_CLDO_CBUCK_PWM		4
#define SSB_PMURES_4325_BUCK_BOOST_PWM		5
#define SSB_PMURES_4325_ILP_REQUEST		6
#define SSB_PMURES_4325_ABUCK_BURST		7
#define SSB_PMURES_4325_ABUCK_PWM		8
#define SSB_PMURES_4325_LNLDO1_PU		9
#define SSB_PMURES_4325_LNLDO2_PU		10
#define SSB_PMURES_4325_LNLDO3_PU		11
#define SSB_PMURES_4325_LNLDO4_PU		12
#define SSB_PMURES_4325_XTAL_PU			13
#define SSB_PMURES_4325_ALP_AVAIL		14
#define SSB_PMURES_4325_RX_PWRSW_PU		15
#define SSB_PMURES_4325_TX_PWRSW_PU		16
#define SSB_PMURES_4325_RFPLL_PWRSW_PU		17
#define SSB_PMURES_4325_LOGEN_PWRSW_PU		18
#define SSB_PMURES_4325_AFE_PWRSW_PU		19
#define SSB_PMURES_4325_BBPLL_PWRSW_PU		20
#define SSB_PMURES_4325_HT_AVAIL		21

/*                               */
#define SSB_PMURES_4328_EXT_SWITCHER_PWM	0
#define SSB_PMURES_4328_BB_SWITCHER_PWM		1
#define SSB_PMURES_4328_BB_SWITCHER_BURST	2
#define SSB_PMURES_4328_BB_EXT_SWITCHER_BURST	3
#define SSB_PMURES_4328_ILP_REQUEST		4
#define SSB_PMURES_4328_RADIO_SWITCHER_PWM	5
#define SSB_PMURES_4328_RADIO_SWITCHER_BURST	6
#define SSB_PMURES_4328_ROM_SWITCH		7
#define SSB_PMURES_4328_PA_REF_LDO		8
#define SSB_PMURES_4328_RADIO_LDO		9
#define SSB_PMURES_4328_AFE_LDO			10
#define SSB_PMURES_4328_PLL_LDO			11
#define SSB_PMURES_4328_BG_FILTBYP		12
#define SSB_PMURES_4328_TX_FILTBYP		13
#define SSB_PMURES_4328_RX_FILTBYP		14
#define SSB_PMURES_4328_XTAL_PU			15
#define SSB_PMURES_4328_XTAL_EN			16
#define SSB_PMURES_4328_BB_PLL_FILTBYP		17
#define SSB_PMURES_4328_RF_PLL_FILTBYP		18
#define SSB_PMURES_4328_BB_PLL_PU		19

/*                               */
#define SSB_PMURES_5354_EXT_SWITCHER_PWM	0
#define SSB_PMURES_5354_BB_SWITCHER_PWM		1
#define SSB_PMURES_5354_BB_SWITCHER_BURST	2
#define SSB_PMURES_5354_BB_EXT_SWITCHER_BURST	3
#define SSB_PMURES_5354_ILP_REQUEST		4
#define SSB_PMURES_5354_RADIO_SWITCHER_PWM	5
#define SSB_PMURES_5354_RADIO_SWITCHER_BURST	6
#define SSB_PMURES_5354_ROM_SWITCH		7
#define SSB_PMURES_5354_PA_REF_LDO		8
#define SSB_PMURES_5354_RADIO_LDO		9
#define SSB_PMURES_5354_AFE_LDO			10
#define SSB_PMURES_5354_PLL_LDO			11
#define SSB_PMURES_5354_BG_FILTBYP		12
#define SSB_PMURES_5354_TX_FILTBYP		13
#define SSB_PMURES_5354_RX_FILTBYP		14
#define SSB_PMURES_5354_XTAL_PU			15
#define SSB_PMURES_5354_XTAL_EN			16
#define SSB_PMURES_5354_BB_PLL_FILTBYP		17
#define SSB_PMURES_5354_RF_PLL_FILTBYP		18
#define SSB_PMURES_5354_BB_PLL_PU		19



/*                                               */
#define SSB_CHIPCO_CHST_4322_SPROM_EXISTS	0x00000040 /*               */
#define SSB_CHIPCO_CHST_4325_SPROM_OTP_SEL	0x00000003
#define SSB_CHIPCO_CHST_4325_DEFCIS_SEL		0 /*                                           */
#define SSB_CHIPCO_CHST_4325_SPROM_SEL		1 /*                                     */
#define SSB_CHIPCO_CHST_4325_OTP_SEL		2 /*                             */
#define SSB_CHIPCO_CHST_4325_OTP_PWRDN		3 /*                                       */
#define SSB_CHIPCO_CHST_4325_SDIO_USB_MODE	0x00000004
#define SSB_CHIPCO_CHST_4325_SDIO_USB_MODE_SHIFT  2
#define SSB_CHIPCO_CHST_4325_RCAL_VALID		0x00000008
#define SSB_CHIPCO_CHST_4325_RCAL_VALID_SHIFT	3
#define SSB_CHIPCO_CHST_4325_RCAL_VALUE		0x000001F0
#define SSB_CHIPCO_CHST_4325_RCAL_VALUE_SHIFT	4
#define SSB_CHIPCO_CHST_4325_PMUTOP_2B 		0x00000200 /*                       */

/*                                                                    */
#define SSB_CHIPCO_CHST_4312_SPROM_PRESENT(status) \
	((status & SSB_CHIPCO_CHST_4325_SPROM_OTP_SEL) != \
		SSB_CHIPCO_CHST_4325_OTP_SEL)
#define SSB_CHIPCO_CHST_4322_SPROM_PRESENT(status) \
	(status & SSB_CHIPCO_CHST_4322_SPROM_EXISTS)
#define SSB_CHIPCO_CHST_4325_SPROM_PRESENT(status) \
	(((status & SSB_CHIPCO_CHST_4325_SPROM_OTP_SEL) != \
		SSB_CHIPCO_CHST_4325_DEFCIS_SEL) && \
	 ((status & SSB_CHIPCO_CHST_4325_SPROM_OTP_SEL) != \
		SSB_CHIPCO_CHST_4325_OTP_SEL))



/*                                 */

/*                    */
#define	SSB_CHIPCO_CLK_N1		0x0000003F	/*            */
#define	SSB_CHIPCO_CLK_N2		0x00003F00	/*            */
#define	SSB_CHIPCO_CLK_N2_SHIFT		8
#define	SSB_CHIPCO_CLK_PLLC		0x000F0000	/*             */
#define	SSB_CHIPCO_CLK_PLLC_SHIFT	16

/*                              */
#define	SSB_CHIPCO_CLK_M1		0x0000003F	/*            */
#define	SSB_CHIPCO_CLK_M2		0x00003F00	/*            */
#define	SSB_CHIPCO_CLK_M2_SHIFT		8
#define	SSB_CHIPCO_CLK_M3		0x003F0000	/*            */
#define	SSB_CHIPCO_CLK_M3_SHIFT		16
#define	SSB_CHIPCO_CLK_MC		0x1F000000	/*             */
#define	SSB_CHIPCO_CLK_MC_SHIFT		24

/*                                      */
#define	SSB_CHIPCO_CLK_F6_2		0x02		/*                  */
#define	SSB_CHIPCO_CLK_F6_3		0x03		/*                   */
#define	SSB_CHIPCO_CLK_F6_4		0x05		/*              */
#define	SSB_CHIPCO_CLK_F6_5		0x09
#define	SSB_CHIPCO_CLK_F6_6		0x11
#define	SSB_CHIPCO_CLK_F6_7		0x21

#define	SSB_CHIPCO_CLK_F5_BIAS		5		/*                             */

#define	SSB_CHIPCO_CLK_MC_BYPASS	0x08
#define	SSB_CHIPCO_CLK_MC_M1		0x04
#define	SSB_CHIPCO_CLK_MC_M1M2		0x02
#define	SSB_CHIPCO_CLK_MC_M1M2M3	0x01
#define	SSB_CHIPCO_CLK_MC_M1M3		0x11

/*                                         */
#define	SSB_CHIPCO_CLK_T2_BIAS		2		/*                      */
#define	SSB_CHIPCO_CLK_T2M2_BIAS	3		/*         */

#define	SSB_CHIPCO_CLK_T2MC_M1BYP	1
#define	SSB_CHIPCO_CLK_T2MC_M2BYP	2
#define	SSB_CHIPCO_CLK_T2MC_M3BYP	4

/*                                         */
#define	SSB_CHIPCO_CLK_T6_MMASK		1		/*                       */
#define	SSB_CHIPCO_CLK_T6_M0		120000000	/*                    */
#define	SSB_CHIPCO_CLK_T6_M1		100000000	/*                    */
#define	SSB_CHIPCO_CLK_SB2MIPS_T6(sb)	(2 * (sb))

/*                   */
#define	SSB_CHIPCO_CLK_BASE1		24000000	/*                     */
#define SSB_CHIPCO_CLK_BASE2		12500000	/*                                 */

/*                                         */
#define	SSB_CHIPCO_CLK_5350_N		0x0311
#define	SSB_CHIPCO_CLK_5350_M		0x04020009


/*                                */

#define	SSB_CHIPCO_CFG_EN		0x0001		/*        */
#define	SSB_CHIPCO_CFG_EXTM		0x000E		/*            */
#define	 SSB_CHIPCO_CFG_EXTM_ASYNC	0x0002		/*                      */
#define	 SSB_CHIPCO_CFG_EXTM_SYNC	0x0004		/*             */
#define	 SSB_CHIPCO_CFG_EXTM_PCMCIA	0x0008		/*        */
#define	 SSB_CHIPCO_CFG_EXTM_IDE	0x000A		/*     */
#define	SSB_CHIPCO_CFG_DS16		0x0010		/*                            */
#define	SSB_CHIPCO_CFG_CLKDIV		0x0060		/*                     */
#define	SSB_CHIPCO_CFG_CLKEN		0x0080		/*                    */
#define	SSB_CHIPCO_CFG_BSTRO		0x0100		/*                       */


/*                                       */

/*                                     */
#define SSB_CHIPCO_FLASHCTL_ST_WREN	0x0006		/*              */
#define SSB_CHIPCO_FLASHCTL_ST_WRDIS	0x0004		/*               */
#define SSB_CHIPCO_FLASHCTL_ST_RDSR	0x0105		/*                      */
#define SSB_CHIPCO_FLASHCTL_ST_WRSR	0x0101		/*                       */
#define SSB_CHIPCO_FLASHCTL_ST_READ	0x0303		/*                 */
#define SSB_CHIPCO_FLASHCTL_ST_PP	0x0302		/*              */
#define SSB_CHIPCO_FLASHCTL_ST_SE	0x02D8		/*              */
#define SSB_CHIPCO_FLASHCTL_ST_BE	0x00C7		/*            */
#define SSB_CHIPCO_FLASHCTL_ST_DP	0x00B9		/*                 */
#define SSB_CHIPCO_FLASHCTL_ST_RES	0x03AB		/*                           */
#define SSB_CHIPCO_FLASHCTL_ST_CSA	0x1000		/*                           */
#define SSB_CHIPCO_FLASHCTL_ST_SSE	0x0220		/*                  */

/*                                     */
#define SSB_CHIPCO_FLASHSTA_ST_WIP	0x01		/*                   */
#define SSB_CHIPCO_FLASHSTA_ST_WEL	0x02		/*                    */
#define SSB_CHIPCO_FLASHSTA_ST_BP	0x1C		/*               */
#define SSB_CHIPCO_FLASHSTA_ST_BP_SHIFT	2
#define SSB_CHIPCO_FLASHSTA_ST_SRWD	0x80		/*                               */

/*                                        */
#define SSB_CHIPCO_FLASHCTL_AT_READ		0x07E8
#define SSB_CHIPCO_FLASHCTL_AT_PAGE_READ	0x07D2
#define SSB_CHIPCO_FLASHCTL_AT_BUF1_READ	/*       */
#define SSB_CHIPCO_FLASHCTL_AT_BUF2_READ	/*       */
#define SSB_CHIPCO_FLASHCTL_AT_STATUS		0x01D7
#define SSB_CHIPCO_FLASHCTL_AT_BUF1_WRITE	0x0384
#define SSB_CHIPCO_FLASHCTL_AT_BUF2_WRITE	0x0387
#define SSB_CHIPCO_FLASHCTL_AT_BUF1_ERASE_PRGM	0x0283	/*               */
#define SSB_CHIPCO_FLASHCTL_AT_BUF2_ERASE_PRGM	0x0286	/*               */
#define SSB_CHIPCO_FLASHCTL_AT_BUF1_PROGRAM	0x0288
#define SSB_CHIPCO_FLASHCTL_AT_BUF2_PROGRAM	0x0289
#define SSB_CHIPCO_FLASHCTL_AT_PAGE_ERASE	0x0281
#define SSB_CHIPCO_FLASHCTL_AT_BLOCK_ERASE	0x0250
#define SSB_CHIPCO_FLASHCTL_AT_BUF1_WRER_PRGM	0x0382	/*                     */
#define SSB_CHIPCO_FLASHCTL_AT_BUF2_WRER_PRGM	0x0385	/*                     */
#define SSB_CHIPCO_FLASHCTL_AT_BUF1_LOAD	0x0253
#define SSB_CHIPCO_FLASHCTL_AT_BUF2_LOAD	0x0255
#define SSB_CHIPCO_FLASHCTL_AT_BUF1_COMPARE	0x0260
#define SSB_CHIPCO_FLASHCTL_AT_BUF2_COMPARE	0x0261
#define SSB_CHIPCO_FLASHCTL_AT_BUF1_REPROGRAM	0x0258
#define SSB_CHIPCO_FLASHCTL_AT_BUF2_REPROGRAM	0x0259

/*                                        */
#define SSB_CHIPCO_FLASHSTA_AT_READY	0x80
#define SSB_CHIPCO_FLASHSTA_AT_MISMATCH	0x40
#define SSB_CHIPCO_FLASHSTA_AT_ID	0x38
#define SSB_CHIPCO_FLASHSTA_AT_ID_SHIFT	3


/*       */

/*             */
#define	SSB_CHIPCO_OTP_HW_REGION	SSB_CHIPCO_OTPS_HW_PROTECT
#define	SSB_CHIPCO_OTP_SW_REGION	SSB_CHIPCO_OTPS_SW_PROTECT
#define	SSB_CHIPCO_OTP_CID_REGION	SSB_CHIPCO_OTPS_CID_PROTECT

/*                                          */
#define	SSB_CHIPCO_OTP_SWLIM_OFF	(-8)
#define	SSB_CHIPCO_OTP_CIDBASE_OFF	0
#define	SSB_CHIPCO_OTP_CIDLIM_OFF	8

/*                                                  */
#define	SSB_CHIPCO_OTP_BOUNDARY_OFF	(-4)
#define	SSB_CHIPCO_OTP_HWSIGN_OFF	(-3)
#define	SSB_CHIPCO_OTP_SWSIGN_OFF	(-2)
#define	SSB_CHIPCO_OTP_CIDSIGN_OFF	(-1)

#define	SSB_CHIPCO_OTP_CID_OFF		0
#define	SSB_CHIPCO_OTP_PKG_OFF		1
#define	SSB_CHIPCO_OTP_FID_OFF		2
#define	SSB_CHIPCO_OTP_RSV_OFF		3
#define	SSB_CHIPCO_OTP_LIM_OFF		4

#define	SSB_CHIPCO_OTP_SIGNATURE	0x578A
#define	SSB_CHIPCO_OTP_MAGIC		0x4E56


struct ssb_device;
struct ssb_serial_port;

/*                                
                                                                                       
 */
struct ssb_chipcommon_pmu {
	u8 rev;			/*              */
	u32 crystalfreq;	/*                                       */
};

struct ssb_chipcommon {
	struct ssb_device *dev;
	u32 capabilities;
	u32 status;
	/*                             */
	u16 fast_pwrup_delay;
	spinlock_t gpio_lock;
	struct ssb_chipcommon_pmu pmu;
	u32 ticks_per_ms;
	u32 max_timer_ms;
};

static inline bool ssb_chipco_available(struct ssb_chipcommon *cc)
{
	return (cc->dev != NULL);
}

/*                 */
#define chipco_read32(cc, offset)	ssb_read32((cc)->dev, offset)
#define chipco_write32(cc, offset, val)	ssb_write32((cc)->dev, offset, val)

#define chipco_mask32(cc, offset, mask) \
		chipco_write32(cc, offset, chipco_read32(cc, offset) & (mask))
#define chipco_set32(cc, offset, set) \
		chipco_write32(cc, offset, chipco_read32(cc, offset) | (set))
#define chipco_maskset32(cc, offset, mask, set) \
		chipco_write32(cc, offset, (chipco_read32(cc, offset) & (mask)) | (set))

extern void ssb_chipcommon_init(struct ssb_chipcommon *cc);

extern void ssb_chipco_suspend(struct ssb_chipcommon *cc);
extern void ssb_chipco_resume(struct ssb_chipcommon *cc);

extern void ssb_chipco_get_clockcpu(struct ssb_chipcommon *cc,
                                    u32 *plltype, u32 *n, u32 *m);
extern void ssb_chipco_get_clockcontrol(struct ssb_chipcommon *cc,
					u32 *plltype, u32 *n, u32 *m);
extern void ssb_chipco_timing_init(struct ssb_chipcommon *cc,
				   unsigned long ns_per_cycle);

enum ssb_clkmode {
	SSB_CLKMODE_SLOW,
	SSB_CLKMODE_FAST,
	SSB_CLKMODE_DYNAMIC,
};

extern void ssb_chipco_set_clockmode(struct ssb_chipcommon *cc,
				     enum ssb_clkmode mode);

extern u32 ssb_chipco_watchdog_timer_set(struct ssb_chipcommon *cc, u32 ticks);

void ssb_chipco_irq_mask(struct ssb_chipcommon *cc, u32 mask, u32 value);

u32 ssb_chipco_irq_status(struct ssb_chipcommon *cc, u32 mask);

/*                             */
u32 ssb_chipco_gpio_in(struct ssb_chipcommon *cc, u32 mask);
u32 ssb_chipco_gpio_out(struct ssb_chipcommon *cc, u32 mask, u32 value);
u32 ssb_chipco_gpio_outen(struct ssb_chipcommon *cc, u32 mask, u32 value);
u32 ssb_chipco_gpio_control(struct ssb_chipcommon *cc, u32 mask, u32 value);
u32 ssb_chipco_gpio_intmask(struct ssb_chipcommon *cc, u32 mask, u32 value);
u32 ssb_chipco_gpio_polarity(struct ssb_chipcommon *cc, u32 mask, u32 value);
u32 ssb_chipco_gpio_pullup(struct ssb_chipcommon *cc, u32 mask, u32 value);
u32 ssb_chipco_gpio_pulldown(struct ssb_chipcommon *cc, u32 mask, u32 value);

#ifdef CONFIG_SSB_SERIAL
extern int ssb_chipco_serial_init(struct ssb_chipcommon *cc,
				  struct ssb_serial_port *ports);
#endif /*                   */

/*             */
extern void ssb_pmu_init(struct ssb_chipcommon *cc);

enum ssb_pmu_ldo_volt_id {
	LDO_PAREF = 0,
	LDO_VOLT1,
	LDO_VOLT2,
	LDO_VOLT3,
};

void ssb_pmu_set_ldo_voltage(struct ssb_chipcommon *cc,
			     enum ssb_pmu_ldo_volt_id id, u32 voltage);
void ssb_pmu_set_ldo_paref(struct ssb_chipcommon *cc, bool on);
void ssb_pmu_spuravoid_pllupdate(struct ssb_chipcommon *cc, int spuravoid);

#endif /*                     */
