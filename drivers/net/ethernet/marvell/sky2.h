/*
                                                  
 */
#ifndef _SKY2_H
#define _SKY2_H

#define ETH_JUMBO_MTU		9000	/*                       */

/*                      */
enum {
	PCI_DEV_REG1	= 0x40,
	PCI_DEV_REG2	= 0x44,
	PCI_DEV_STATUS  = 0x7c,
	PCI_DEV_REG3	= 0x80,
	PCI_DEV_REG4	= 0x84,
	PCI_DEV_REG5    = 0x88,
	PCI_CFG_REG_0	= 0x90,
	PCI_CFG_REG_1	= 0x94,

	PSM_CONFIG_REG0  = 0x98,
	PSM_CONFIG_REG1	 = 0x9C,
	PSM_CONFIG_REG2  = 0x160,
	PSM_CONFIG_REG3  = 0x164,
	PSM_CONFIG_REG4  = 0x168,

	PCI_LDO_CTRL    = 0xbc,
};

/*         */
enum pci_dev_reg_1 {
	PCI_Y2_PIG_ENA	 = 1<<31, /*                             */
	PCI_Y2_DLL_DIS	 = 1<<30, /*                           */
	PCI_SW_PWR_ON_RST= 1<<30, /*                              */
	PCI_Y2_PHY2_COMA = 1<<29, /*                                  */
	PCI_Y2_PHY1_COMA = 1<<28, /*                                  */
	PCI_Y2_PHY2_POWD = 1<<27, /*                                   */
	PCI_Y2_PHY1_POWD = 1<<26, /*                                   */
	PCI_Y2_PME_LEGACY= 1<<15, /*                                          */

	PCI_PHY_LNK_TIM_MSK= 3L<<8,/*                                     */
	PCI_ENA_L1_EVENT = 1<<7, /*                     */
	PCI_ENA_GPHY_LNK = 1<<6, /*                                 */
	PCI_FORCE_PEX_L1 = 1<<5, /*                 */
};

enum pci_dev_reg_2 {
	PCI_VPD_WR_THR	= 0xffL<<24,	/*                                 */
	PCI_DEV_SEL	= 0x7fL<<17,	/*                                  */
	PCI_VPD_ROM_SZ	= 7L<<14,	/*                          */

	PCI_PATCH_DIR	= 0xfL<<8,	/*                                  */
	PCI_EXT_PATCHS	= 0xfL<<4,	/*                                  */
	PCI_EN_DUMMY_RD	= 1<<3,		/*                   */
	PCI_REV_DESC	= 1<<2,		/*                     */

	PCI_USEDATA64	= 1<<0,		/*                        */
};

/*                                                       */
enum pci_dev_reg_3 {
	P_CLK_ASF_REGS_DIS	= 1<<18,/*                                */
	P_CLK_COR_REGS_D0_DIS	= 1<<17,/*                            */
	P_CLK_MACSEC_DIS	= 1<<17,/*                                   */
	P_CLK_PCI_REGS_D0_DIS	= 1<<16,/*                            */
	P_CLK_COR_YTB_ARB_DIS	= 1<<15,/*                            */
	P_CLK_MAC_LNK1_D3_DIS	= 1<<14,/*                             */
	P_CLK_COR_LNK1_D0_DIS	= 1<<13,/*                             */
	P_CLK_MAC_LNK1_D0_DIS	= 1<<12,/*                             */
	P_CLK_COR_LNK1_D3_DIS	= 1<<11,/*                             */
	P_CLK_PCI_MST_ARB_DIS	= 1<<10,/*                                */
	P_CLK_COR_REGS_D3_DIS	= 1<<9,	/*                            */
	P_CLK_PCI_REGS_D3_DIS	= 1<<8,	/*                            */
	P_CLK_REF_LNK1_GM_DIS	= 1<<7,	/*                               */
	P_CLK_COR_LNK1_GM_DIS	= 1<<6,	/*                               */
	P_CLK_PCI_COMMON_DIS	= 1<<5,	/*                           */
	P_CLK_COR_COMMON_DIS	= 1<<4,	/*                           */
	P_CLK_PCI_LNK1_BMU_DIS	= 1<<3,	/*                              */
	P_CLK_COR_LNK1_BMU_DIS	= 1<<2,	/*                              */
	P_CLK_PCI_LNK1_BIU_DIS	= 1<<1,	/*                              */
	P_CLK_COR_LNK1_BIU_DIS	= 1<<0,	/*                              */
	PCIE_OUR3_WOL_D3_COLD_SET = P_CLK_ASF_REGS_DIS |
				    P_CLK_COR_REGS_D0_DIS |
				    P_CLK_COR_LNK1_D0_DIS |
				    P_CLK_MAC_LNK1_D0_DIS |
				    P_CLK_PCI_MST_ARB_DIS |
				    P_CLK_COR_COMMON_DIS |
				    P_CLK_COR_LNK1_BMU_DIS,
};

/*                                                       */
enum pci_dev_reg_4 {
				/*                                        */
	P_PEX_LTSSM_STAT_MSK	= 0x7fL<<25,	/*                            */
#define P_PEX_LTSSM_STAT(x)	((x << 25) & P_PEX_LTSSM_STAT_MSK)
	P_PEX_LTSSM_L1_STAT	= 0x34,
	P_PEX_LTSSM_DET_STAT	= 0x01,
	P_TIMER_VALUE_MSK	= 0xffL<<16,	/*                              */
					/*                                 */
	P_FORCE_ASPM_REQUEST	= 1<<15, /*                              */
	P_ASPM_GPHY_LINK_DOWN	= 1<<14, /*                          */
	P_ASPM_INT_FIFO_EMPTY	= 1<<13, /*                               */
	P_ASPM_CLKRUN_REQUEST	= 1<<12, /*                          */

	P_ASPM_FORCE_CLKREQ_ENA	= 1<<4,	/*                                */
	P_ASPM_CLKREQ_PAD_CTL	= 1<<3,	/*                              */
	P_ASPM_A1_MODE_SELECT	= 1<<2,	/*                          */
	P_CLK_GATE_PEX_UNIT_ENA	= 1<<1,	/*                            */
	P_CLK_GATE_ROOT_COR_ENA	= 1<<0,	/*                             */
	P_ASPM_CONTROL_MSK	= P_FORCE_ASPM_REQUEST | P_ASPM_GPHY_LINK_DOWN
				  | P_ASPM_CLKRUN_REQUEST | P_ASPM_INT_FIFO_EMPTY,
};

/*                                                       */
enum pci_dev_reg_5 {
					/*                            */
	P_CTL_DIV_CORE_CLK_ENA	= 1<<31, /*                          */
	P_CTL_SRESET_VMAIN_AV	= 1<<30, /*                                   */
	P_CTL_BYPASS_VMAIN_AV	= 1<<29, /*                                   */
	P_CTL_TIM_VMAIN_AV_MSK	= 3<<27, /*                                 */
					 /*                                    */
	P_REL_PCIE_RST_DE_ASS	= 1<<26, /*                        */
	P_REL_GPHY_REC_PACKET	= 1<<25, /*                      */
	P_REL_INT_FIFO_N_EMPTY	= 1<<24, /*                         */
	P_REL_MAIN_PWR_AVAIL	= 1<<23, /*                      */
	P_REL_CLKRUN_REQ_REL	= 1<<22, /*                        */
	P_REL_PCIE_RESET_ASS	= 1<<21, /*                     */
	P_REL_PME_ASSERTED	= 1<<20, /*              */
	P_REL_PCIE_EXIT_L1_ST	= 1<<19, /*                    */
	P_REL_LOADER_NOT_FIN	= 1<<18, /*                           */
	P_REL_PCIE_RX_EX_IDLE	= 1<<17, /*                                    */
	P_REL_GPHY_LINK_UP	= 1<<16, /*              */

					/*                                 */
	P_GAT_PCIE_RST_ASSERTED	= 1<<10,/*                     */
	P_GAT_GPHY_N_REC_PACKET	= 1<<9, /*                          */
	P_GAT_INT_FIFO_EMPTY	= 1<<8, /*                     */
	P_GAT_MAIN_PWR_N_AVAIL	= 1<<7, /*                          */
	P_GAT_CLKRUN_REQ_REL	= 1<<6, /*                      */
	P_GAT_PCIE_RESET_ASS	= 1<<5, /*                     */
	P_GAT_PME_DE_ASSERTED	= 1<<4, /*                 */
	P_GAT_PCIE_ENTER_L1_ST	= 1<<3, /*                     */
	P_GAT_LOADER_FINISHED	= 1<<2, /*                       */
	P_GAT_PCIE_RX_EL_IDLE	= 1<<1, /*                               */
	P_GAT_GPHY_LINK_DOWN	= 1<<0,	/*                */

	PCIE_OUR5_EVENT_CLK_D3_SET = P_REL_GPHY_REC_PACKET |
				     P_REL_INT_FIFO_N_EMPTY |
				     P_REL_PCIE_EXIT_L1_ST |
				     P_REL_PCIE_RX_EX_IDLE |
				     P_GAT_GPHY_N_REC_PACKET |
				     P_GAT_INT_FIFO_EMPTY |
				     P_GAT_PCIE_ENTER_L1_ST |
				     P_GAT_PCIE_RX_EL_IDLE,
};

/*                                                           */
enum pci_cfg_reg1 {
	P_CF1_DIS_REL_EVT_RST	= 1<<24, /*                                   */
										/*                                    */
	P_CF1_REL_LDR_NOT_FIN	= 1<<23, /*                            */
	P_CF1_REL_VMAIN_AVLBL	= 1<<22, /*                 */
	P_CF1_REL_PCIE_RESET	= 1<<21, /*             */
										/*                                 */
	P_CF1_GAT_LDR_NOT_FIN	= 1<<20, /*                        */
	P_CF1_GAT_PCIE_RX_IDLE	= 1<<19, /*                          */
	P_CF1_GAT_PCIE_RESET	= 1<<18, /*             */
	P_CF1_PRST_PHY_CLKREQ	= 1<<17, /*                                       */
	P_CF1_PCIE_RST_CLKREQ	= 1<<16, /*                                  */

	P_CF1_ENA_CFG_LDR_DONE	= 1<<8, /*                                      */

	P_CF1_ENA_TXBMU_RD_IDLE	= 1<<1, /*                                   */
	P_CF1_ENA_TXBMU_WR_IDLE	= 1<<0, /*                                   */

	PCIE_CFG1_EVENT_CLK_D3_SET = P_CF1_DIS_REL_EVT_RST |
					P_CF1_REL_LDR_NOT_FIN |
					P_CF1_REL_VMAIN_AVLBL |
					P_CF1_REL_PCIE_RESET |
					P_CF1_GAT_LDR_NOT_FIN |
					P_CF1_GAT_PCIE_RESET |
					P_CF1_PRST_PHY_CLKREQ |
					P_CF1_ENA_CFG_LDR_DONE |
					P_CF1_ENA_TXBMU_RD_IDLE |
					P_CF1_ENA_TXBMU_WR_IDLE,
};

/*              */
enum {
	PSM_CONFIG_REG1_AC_PRESENT_STATUS = 1<<31,   /*                   */

	PSM_CONFIG_REG1_PTP_CLK_SEL	  = 1<<29,   /*                  */
	PSM_CONFIG_REG1_PTP_MODE	  = 1<<28,   /*          */

	PSM_CONFIG_REG1_MUX_PHY_LINK	  = 1<<27,   /*                         */

	PSM_CONFIG_REG1_EN_PIN63_AC_PRESENT = 1<<26,  /*                                  */
	PSM_CONFIG_REG1_EN_PCIE_TIMER	  = 1<<25,    /*                   */
	PSM_CONFIG_REG1_EN_SPU_TIMER	  = 1<<24,    /*                  */
	PSM_CONFIG_REG1_POLARITY_AC_PRESENT = 1<<23,  /*                     */

	PSM_CONFIG_REG1_EN_AC_PRESENT	  = 1<<21,    /*                   */

	PSM_CONFIG_REG1_EN_GPHY_INT_PSM	= 1<<20,      /*                         */
	PSM_CONFIG_REG1_DIS_PSM_TIMER	= 1<<19,      /*                   */
};

/*               */
enum {
	PSM_CONFIG_REG1_GPHY_ENERGY_STS	= 1<<31, /*                           */

	PSM_CONFIG_REG1_UART_MODE_MSK	= 3<<29, /*           */
	PSM_CONFIG_REG1_CLK_RUN_ASF	= 1<<28, /*                                             */
	PSM_CONFIG_REG1_UART_CLK_DISABLE= 1<<27, /*                    */
	PSM_CONFIG_REG1_VAUX_ONE	= 1<<26, /*                           */
	PSM_CONFIG_REG1_UART_FC_RI_VAL	= 1<<25, /*                             */
	PSM_CONFIG_REG1_UART_FC_DCD_VAL	= 1<<24, /*                              */
	PSM_CONFIG_REG1_UART_FC_DSR_VAL	= 1<<23, /*                              */
	PSM_CONFIG_REG1_UART_FC_CTS_VAL	= 1<<22, /*                              */
	PSM_CONFIG_REG1_LATCH_VAUX	= 1<<21, /*                                 */
	PSM_CONFIG_REG1_FORCE_TESTMODE_INPUT= 1<<20, /*                                 */
	PSM_CONFIG_REG1_UART_RST	= 1<<19, /*          */
	PSM_CONFIG_REG1_PSM_PCIE_L1_POL	= 1<<18, /*                                */
	PSM_CONFIG_REG1_TIMER_STAT	= 1<<17, /*                  */
	PSM_CONFIG_REG1_GPHY_INT	= 1<<16, /*                 */
	PSM_CONFIG_REG1_FORCE_TESTMODE_ZERO= 1<<15, /*                                 */
	PSM_CONFIG_REG1_EN_INT_ASPM_CLKREQ = 1<<14, /*                                          */
	PSM_CONFIG_REG1_EN_SND_TASK_ASPM_CLKREQ	= 1<<13, /*                                               */
	PSM_CONFIG_REG1_DIS_CLK_GATE_SND_TASK	= 1<<12, /*                                   */
	PSM_CONFIG_REG1_DIS_FF_CHIAN_SND_INTA	= 1<<11, /*                                         */

	PSM_CONFIG_REG1_DIS_LOADER	= 1<<9, /*                                               */
	PSM_CONFIG_REG1_DO_PWDN		= 1<<8, /*                                 */
	PSM_CONFIG_REG1_DIS_PIG		= 1<<7, /*                                                   */
	PSM_CONFIG_REG1_DIS_PERST	= 1<<6, /*                                                         */
	PSM_CONFIG_REG1_EN_REG18_PD	= 1<<5, /*                                 */
	PSM_CONFIG_REG1_EN_PSM_LOAD	= 1<<4, /*                                                   */
	PSM_CONFIG_REG1_EN_PSM_HOT_RST	= 1<<3, /*                               */
	PSM_CONFIG_REG1_EN_PSM_PERST	= 1<<2, /*                                 */
	PSM_CONFIG_REG1_EN_PSM_PCIE_L1	= 1<<1, /*                              */
	PSM_CONFIG_REG1_EN_PSM		= 1<<0, /*                   */
};

/*                                                 */
enum {
						/*                       */
	PSM_CONFIG_REG4_TIMER_PHY_LINK_DETECT_MSK = 0xf<<4,
	PSM_CONFIG_REG4_TIMER_PHY_LINK_DETECT_BASE = 4,

	PSM_CONFIG_REG4_DEBUG_TIMER	    = 1<<1, /*             */
	PSM_CONFIG_REG4_RST_PHY_LINK_DETECT = 1<<0, /*                        */
};


#define PCI_STATUS_ERROR_BITS (PCI_STATUS_DETECTED_PARITY | \
			       PCI_STATUS_SIG_SYSTEM_ERROR | \
			       PCI_STATUS_REC_MASTER_ABORT | \
			       PCI_STATUS_REC_TARGET_ABORT | \
			       PCI_STATUS_PARITY)

enum csr_regs {
	B0_RAP		= 0x0000,
	B0_CTST		= 0x0004,

	B0_POWER_CTRL	= 0x0007,
	B0_ISRC		= 0x0008,
	B0_IMSK		= 0x000c,
	B0_HWE_ISRC	= 0x0010,
	B0_HWE_IMSK	= 0x0014,

	/*                                      */
	B0_Y2_SP_ISRC2	= 0x001c,
	B0_Y2_SP_ISRC3	= 0x0020,
	B0_Y2_SP_EISR	= 0x0024,
	B0_Y2_SP_LISR	= 0x0028,
	B0_Y2_SP_ICR	= 0x002c,

	B2_MAC_1	= 0x0100,
	B2_MAC_2	= 0x0108,
	B2_MAC_3	= 0x0110,
	B2_CONN_TYP	= 0x0118,
	B2_PMD_TYP	= 0x0119,
	B2_MAC_CFG	= 0x011a,
	B2_CHIP_ID	= 0x011b,
	B2_E_0		= 0x011c,

	B2_Y2_CLK_GATE  = 0x011d,
	B2_Y2_HW_RES	= 0x011e,
	B2_E_3		= 0x011f,
	B2_Y2_CLK_CTRL	= 0x0120,

	B2_TI_INI	= 0x0130,
	B2_TI_VAL	= 0x0134,
	B2_TI_CTRL	= 0x0138,
	B2_TI_TEST	= 0x0139,

	B2_TST_CTRL1	= 0x0158,
	B2_TST_CTRL2	= 0x0159,
	B2_GP_IO	= 0x015c,

	B2_I2C_CTRL	= 0x0160,
	B2_I2C_DATA	= 0x0164,
	B2_I2C_IRQ	= 0x0168,
	B2_I2C_SW	= 0x016c,

	Y2_PEX_PHY_DATA = 0x0170,
	Y2_PEX_PHY_ADDR = 0x0172,

	B3_RAM_ADDR	= 0x0180,
	B3_RAM_DATA_LO	= 0x0184,
	B3_RAM_DATA_HI	= 0x0188,

/*                         */
/*                                                    */
/*
                                                                            
                                                                         
                                                 
 */
#define RAM_BUFFER(port, reg)	(reg | (port <<6))

	B3_RI_WTO_R1	= 0x0190,
	B3_RI_WTO_XA1	= 0x0191,
	B3_RI_WTO_XS1	= 0x0192,
	B3_RI_RTO_R1	= 0x0193,
	B3_RI_RTO_XA1	= 0x0194,
	B3_RI_RTO_XS1	= 0x0195,
	B3_RI_WTO_R2	= 0x0196,
	B3_RI_WTO_XA2	= 0x0197,
	B3_RI_WTO_XS2	= 0x0198,
	B3_RI_RTO_R2	= 0x0199,
	B3_RI_RTO_XA2	= 0x019a,
	B3_RI_RTO_XS2	= 0x019b,
	B3_RI_TO_VAL	= 0x019c,
	B3_RI_CTRL	= 0x01a0,
	B3_RI_TEST	= 0x01a2,
	B3_MA_TOINI_RX1	= 0x01b0,
	B3_MA_TOINI_RX2	= 0x01b1,
	B3_MA_TOINI_TX1	= 0x01b2,
	B3_MA_TOINI_TX2	= 0x01b3,
	B3_MA_TOVAL_RX1	= 0x01b4,
	B3_MA_TOVAL_RX2	= 0x01b5,
	B3_MA_TOVAL_TX1	= 0x01b6,
	B3_MA_TOVAL_TX2	= 0x01b7,
	B3_MA_TO_CTRL	= 0x01b8,
	B3_MA_TO_TEST	= 0x01ba,
	B3_MA_RCINI_RX1	= 0x01c0,
	B3_MA_RCINI_RX2	= 0x01c1,
	B3_MA_RCINI_TX1	= 0x01c2,
	B3_MA_RCINI_TX2	= 0x01c3,
	B3_MA_RCVAL_RX1	= 0x01c4,
	B3_MA_RCVAL_RX2	= 0x01c5,
	B3_MA_RCVAL_TX1	= 0x01c6,
	B3_MA_RCVAL_TX2	= 0x01c7,
	B3_MA_RC_CTRL	= 0x01c8,
	B3_MA_RC_TEST	= 0x01ca,
	B3_PA_TOINI_RX1	= 0x01d0,
	B3_PA_TOINI_RX2	= 0x01d4,
	B3_PA_TOINI_TX1	= 0x01d8,
	B3_PA_TOINI_TX2	= 0x01dc,
	B3_PA_TOVAL_RX1	= 0x01e0,
	B3_PA_TOVAL_RX2	= 0x01e4,
	B3_PA_TOVAL_TX1	= 0x01e8,
	B3_PA_TOVAL_TX2	= 0x01ec,
	B3_PA_CTRL	= 0x01f0,
	B3_PA_TEST	= 0x01f2,

	Y2_CFG_SPC	= 0x1c00,	/*                         */
	Y2_CFG_AER      = 0x1d00,	/*                                  */
};

/*                                          */
enum {
	Y2_VMAIN_AVAIL	= 1<<17,/*                                */
	Y2_VAUX_AVAIL	= 1<<16,/*                               */
	Y2_HW_WOL_ON	= 1<<15,/*                                     */
	Y2_HW_WOL_OFF	= 1<<14,/*                                     */
	Y2_ASF_ENABLE	= 1<<13,/*                                */
	Y2_ASF_DISABLE	= 1<<12,/*                                 */
	Y2_CLK_RUN_ENA	= 1<<11,/*                                */
	Y2_CLK_RUN_DIS	= 1<<10,/*                                */
	Y2_LED_STAT_ON	= 1<<9, /*                               */
	Y2_LED_STAT_OFF	= 1<<8, /*                               */

	CS_ST_SW_IRQ	= 1<<7,	/*                    */
	CS_CL_SW_IRQ	= 1<<6,	/*                      */
	CS_STOP_DONE	= 1<<5,	/*                         */
	CS_STOP_MAST	= 1<<4,	/*                                */
	CS_MRST_CLR	= 1<<3,	/*                    */
	CS_MRST_SET	= 1<<2,	/*                  */
	CS_RST_CLR	= 1<<1,	/*                      */
	CS_RST_SET	= 1,	/*                      */
};

/*                                                     */
enum {
	PC_VAUX_ENA	= 1<<7,	/*                     */
	PC_VAUX_DIS	= 1<<6,	/*                     */
	PC_VCC_ENA	= 1<<5,	/*                    */
	PC_VCC_DIS	= 1<<4,	/*                    */
	PC_VAUX_ON	= 1<<3,	/*                 */
	PC_VAUX_OFF	= 1<<2,	/*                 */
	PC_VCC_ON	= 1<<1,	/*                */
	PC_VCC_OFF	= 1<<0,	/*                */
};

/*                                         */

/*                                                      */
/*                                                      */
/*                                    */
/*                                    */
enum {
	Y2_IS_HW_ERR	= 1<<31,	/*                    */
	Y2_IS_STAT_BMU	= 1<<30,	/*                      */
	Y2_IS_ASF	= 1<<29,	/*                         */
	Y2_IS_CPU_TO	= 1<<28,	/*             */
	Y2_IS_POLL_CHK	= 1<<27,	/*                             */
	Y2_IS_TWSI_RDY	= 1<<26,	/*                       */
	Y2_IS_IRQ_SW	= 1<<25,	/*               */
	Y2_IS_TIMINT	= 1<<24,	/*                */

	Y2_IS_IRQ_PHY2	= 1<<12,	/*                      */
	Y2_IS_IRQ_MAC2	= 1<<11,	/*                      */
	Y2_IS_CHK_RX2	= 1<<10,	/*                       */
	Y2_IS_CHK_TXS2	= 1<<9,		/*                        */
	Y2_IS_CHK_TXA2	= 1<<8,		/*                        */

	Y2_IS_PSM_ACK	= 1<<7,		/*                                     */
	Y2_IS_PTP_TIST	= 1<<6,		/*                                    */
	Y2_IS_PHY_QLNK	= 1<<5,		/*                                    */

	Y2_IS_IRQ_PHY1	= 1<<4,		/*                      */
	Y2_IS_IRQ_MAC1	= 1<<3,		/*                      */
	Y2_IS_CHK_RX1	= 1<<2,		/*                       */
	Y2_IS_CHK_TXS1	= 1<<1,		/*                        */
	Y2_IS_CHK_TXA1	= 1<<0,		/*                        */

	Y2_IS_BASE	= Y2_IS_HW_ERR | Y2_IS_STAT_BMU,
	Y2_IS_PORT_1	= Y2_IS_IRQ_PHY1 | Y2_IS_IRQ_MAC1
		          | Y2_IS_CHK_TXA1 | Y2_IS_CHK_RX1,
	Y2_IS_PORT_2	= Y2_IS_IRQ_PHY2 | Y2_IS_IRQ_MAC2
			  | Y2_IS_CHK_TXA2 | Y2_IS_CHK_RX2,
	Y2_IS_ERROR     = Y2_IS_HW_ERR |
			  Y2_IS_IRQ_MAC1 | Y2_IS_CHK_TXA1 | Y2_IS_CHK_RX1 |
			  Y2_IS_IRQ_MAC2 | Y2_IS_CHK_TXA2 | Y2_IS_CHK_RX2,
};

/*                                                     */
enum {
	IS_ERR_MSK	= 0x00003fff,/*                  */

	IS_IRQ_TIST_OV	= 1<<13, /*                                        */
	IS_IRQ_SENSOR	= 1<<12, /*                              */
	IS_IRQ_MST_ERR	= 1<<11, /*                           */
	IS_IRQ_STAT	= 1<<10, /*                      */
	IS_NO_STAT_M1	= 1<<9,	/*                         */
	IS_NO_STAT_M2	= 1<<8,	/*                         */
	IS_NO_TIST_M1	= 1<<7,	/*                          */
	IS_NO_TIST_M2	= 1<<6,	/*                          */
	IS_RAM_RD_PAR	= 1<<5,	/*                        */
	IS_RAM_WR_PAR	= 1<<4,	/*                        */
	IS_M1_PAR_ERR	= 1<<3,	/*                    */
	IS_M2_PAR_ERR	= 1<<2,	/*                    */
	IS_R1_PAR_ERR	= 1<<1,	/*                       */
	IS_R2_PAR_ERR	= 1<<0,	/*                       */
};

/*                                           */
enum {
	Y2_IS_TIST_OV	= 1<<29,/*                                     */
	Y2_IS_SENSOR	= 1<<28, /*                  */
	Y2_IS_MST_ERR	= 1<<27, /*                        */
	Y2_IS_IRQ_STAT	= 1<<26, /*                            */
	Y2_IS_PCI_EXP	= 1<<25, /*                       */
	Y2_IS_PCI_NEXP	= 1<<24, /*                                        */
						/*        */
	Y2_IS_PAR_RD2	= 1<<13, /*                                 */
	Y2_IS_PAR_WR2	= 1<<12, /*                                  */
	Y2_IS_PAR_MAC2	= 1<<11, /*                              */
	Y2_IS_PAR_RX2	= 1<<10, /*                         */
	Y2_IS_TCP_TXS2	= 1<<9, /*                                       */
	Y2_IS_TCP_TXA2	= 1<<8, /*                                        */
						/*        */
	Y2_IS_PAR_RD1	= 1<<5, /*                                 */
	Y2_IS_PAR_WR1	= 1<<4, /*                                  */
	Y2_IS_PAR_MAC1	= 1<<3, /*                              */
	Y2_IS_PAR_RX1	= 1<<2, /*                         */
	Y2_IS_TCP_TXS1	= 1<<1, /*                                       */
	Y2_IS_TCP_TXA1	= 1<<0, /*                                        */

	Y2_HWE_L1_MASK	= Y2_IS_PAR_RD1 | Y2_IS_PAR_WR1 | Y2_IS_PAR_MAC1 |
			  Y2_IS_PAR_RX1 | Y2_IS_TCP_TXS1| Y2_IS_TCP_TXA1,
	Y2_HWE_L2_MASK	= Y2_IS_PAR_RD2 | Y2_IS_PAR_WR2 | Y2_IS_PAR_MAC2 |
			  Y2_IS_PAR_RX2 | Y2_IS_TCP_TXS2| Y2_IS_TCP_TXA2,

	Y2_HWE_ALL_MASK	= Y2_IS_TIST_OV | Y2_IS_MST_ERR | Y2_IS_IRQ_STAT |
			  Y2_HWE_L1_MASK | Y2_HWE_L2_MASK,
};

/*                                                    */
enum {
	DPT_START	= 1<<1,
	DPT_STOP	= 1<<0,
};

/*                                             */
enum {
	TST_FRC_DPERR_MR = 1<<7, /*                          */
	TST_FRC_DPERR_MW = 1<<6, /*                          */
	TST_FRC_DPERR_TR = 1<<5, /*                          */
	TST_FRC_DPERR_TW = 1<<4, /*                          */
	TST_FRC_APERR_M	 = 1<<3, /*                       */
	TST_FRC_APERR_T	 = 1<<2, /*                       */
	TST_CFG_WRITE_ON = 1<<1, /*                       */
	TST_CFG_WRITE_OFF= 1<<0, /*                       */
};

/*          */
enum {
	GLB_GPIO_CLK_DEB_ENA = 1<<31,	/*                    */
	GLB_GPIO_CLK_DBG_MSK = 0xf<<26, /*             */

	GLB_GPIO_INT_RST_D3_DIS = 1<<15, /*                                       */
	GLB_GPIO_LED_PAD_SPEED_UP = 1<<14, /*                  */
	GLB_GPIO_STAT_RACE_DIS	= 1<<13, /*                     */
	GLB_GPIO_TEST_SEL_MSK	= 3<<11, /*                 */
	GLB_GPIO_TEST_SEL_BASE	= 1<<11,
	GLB_GPIO_RAND_ENA	= 1<<10, /*               */
	GLB_GPIO_RAND_BIT_1	= 1<<9,  /*              */
};

/*                                                      */
enum {
	CFG_CHIP_R_MSK	  = 0xf<<4,	/*                          */
					/*                     */
	CFG_DIS_M2_CLK	  = 1<<1,	/*                           */
	CFG_SNG_MAC	  = 1<<0,	/*                               */
};

/*                                                */
enum {
	CHIP_ID_YUKON_XL   = 0xb3, /*            */
	CHIP_ID_YUKON_EC_U = 0xb4, /*                  */
	CHIP_ID_YUKON_EX   = 0xb5, /*                 */
	CHIP_ID_YUKON_EC   = 0xb6, /*            */
 	CHIP_ID_YUKON_FE   = 0xb7, /*            */
 	CHIP_ID_YUKON_FE_P = 0xb8, /*             */
	CHIP_ID_YUKON_SUPR = 0xb9, /*                 */
	CHIP_ID_YUKON_UL_2 = 0xba, /*                 */
	CHIP_ID_YUKON_OPT  = 0xbc, /*                */
	CHIP_ID_YUKON_PRM  = 0xbd, /*                      */
	CHIP_ID_YUKON_OP_2 = 0xbe, /*                  */
};

enum yukon_xl_rev {
	CHIP_REV_YU_XL_A0  = 0,
	CHIP_REV_YU_XL_A1  = 1,
	CHIP_REV_YU_XL_A2  = 2,
	CHIP_REV_YU_XL_A3  = 3,
};

enum yukon_ec_rev {
	CHIP_REV_YU_EC_A1    = 0,  /*                              */
	CHIP_REV_YU_EC_A2    = 1,  /*                           */
	CHIP_REV_YU_EC_A3    = 2,  /*                           */
};
enum yukon_ec_u_rev {
	CHIP_REV_YU_EC_U_A0  = 1,
	CHIP_REV_YU_EC_U_A1  = 2,
	CHIP_REV_YU_EC_U_B0  = 3,
	CHIP_REV_YU_EC_U_B1  = 5,
};
enum yukon_fe_rev {
	CHIP_REV_YU_FE_A1    = 1,
	CHIP_REV_YU_FE_A2    = 2,
};
enum yukon_fe_p_rev {
	CHIP_REV_YU_FE2_A0   = 0,
};
enum yukon_ex_rev {
	CHIP_REV_YU_EX_A0    = 1,
	CHIP_REV_YU_EX_B0    = 2,
};
enum yukon_supr_rev {
	CHIP_REV_YU_SU_A0    = 0,
	CHIP_REV_YU_SU_B0    = 1,
	CHIP_REV_YU_SU_B1    = 3,
};

enum yukon_prm_rev {
	CHIP_REV_YU_PRM_Z1   = 1,
	CHIP_REV_YU_PRM_A0   = 2,
};

/*                                                   */
enum {
	Y2_STATUS_LNK2_INAC	= 1<<7, /*                                     */
	Y2_CLK_GAT_LNK2_DIS	= 1<<6, /*                             */
	Y2_COR_CLK_LNK2_DIS	= 1<<5, /*                           */
	Y2_PCI_CLK_LNK2_DIS	= 1<<4, /*                          */
	Y2_STATUS_LNK1_INAC	= 1<<3, /*                                     */
	Y2_CLK_GAT_LNK1_DIS	= 1<<2, /*                             */
	Y2_COR_CLK_LNK1_DIS	= 1<<1, /*                           */
	Y2_PCI_CLK_LNK1_DIS	= 1<<0, /*                          */
};

/*                                                */
enum {
	CFG_LED_MODE_MSK	= 7<<2,	/*                           */
	CFG_LINK_2_AVAIL	= 1<<1,	/*                  */
	CFG_LINK_1_AVAIL	= 1<<0,	/*                  */
};
#define CFG_LED_MODE(x)		(((x) & CFG_LED_MODE_MSK) >> 2)
#define CFG_DUAL_MAC_MSK	(CFG_LINK_2_AVAIL | CFG_LINK_1_AVAIL)


/*                                                                     */
enum {
	Y2_CLK_DIV_VAL_MSK	= 0xff<<16,/*                                 */
#define	Y2_CLK_DIV_VAL(x)	(((x)<<16) & Y2_CLK_DIV_VAL_MSK)
	Y2_CLK_DIV_VAL2_MSK	= 7<<21,   /*                                 */
	Y2_CLK_SELECT2_MSK	= 0x1f<<16,/*                          */
#define Y2_CLK_DIV_VAL_2(x)	(((x)<<21) & Y2_CLK_DIV_VAL2_MSK)
#define Y2_CLK_SEL_VAL_2(x)	(((x)<<16) & Y2_CLK_SELECT2_MSK)
	Y2_CLK_DIV_ENA		= 1<<1, /*                             */
	Y2_CLK_DIV_DIS		= 1<<0,	/*                             */
};

/*                                  */
/*                                                  */
enum {
	TIM_START	= 1<<2,	/*             */
	TIM_STOP	= 1<<1,	/*             */
	TIM_CLR_IRQ	= 1<<0,	/*                         */
};

/*                               */
/*                                               */
/*                                                    */
enum {
	TIM_T_ON	= 1<<2,	/*              */
	TIM_T_OFF	= 1<<1,	/*               */
	TIM_T_STEP	= 1<<0,	/*           */
};

/*                                                                    */
enum {
	PEX_RD_ACCESS	= 1<<31, /*                                 */
	PEX_DB_ACCESS	= 1<<30, /*                          */
};

/*                                                   */
					/*                      */
#define RAM_ADR_RAN	0x0007ffffL	/*                               */
/*                         */

/*                                                   */
enum {
	RI_CLR_RD_PERR	= 1<<9,	/*                               */
	RI_CLR_WR_PERR	= 1<<8,	/*                               */

	RI_RST_CLR	= 1<<1,	/*                           */
	RI_RST_SET	= 1<<0,	/*                           */
};

#define SK_RI_TO_53	36		/*                       */


/*                                          */
#define SK_REG(port,reg)	(((port)<<7)+(reg))

/*                                                                */
/*                                                    */
/*                                                 */
/*                                                   */
/*                                                */

#define TXA_MAX_VAL	0x00ffffffUL	/*                                   */

/*                                              */
enum {
	TXA_ENA_FSYNC	= 1<<7,	/*                                */
	TXA_DIS_FSYNC	= 1<<6,	/*                                */
	TXA_ENA_ALLOC	= 1<<5,	/*                                 */
	TXA_DIS_ALLOC	= 1<<4,	/*                                 */
	TXA_START_RC	= 1<<3,	/*                         */
	TXA_STOP_RC	= 1<<2,	/*                         */
	TXA_ENA_ARB	= 1<<1,	/*                    */
	TXA_DIS_ARB	= 1<<0,	/*                    */
};

/*
             
 */
/*                                                                */
enum {
	TXA_ITI_INI	= 0x0200,/*                                      */
	TXA_ITI_VAL	= 0x0204,/*                                    */
	TXA_LIM_INI	= 0x0208,/*                                      */
	TXA_LIM_VAL	= 0x020c,/*                                   */
	TXA_CTRL	= 0x0210,/*                                    */
	TXA_TEST	= 0x0211,/*                                 */
	TXA_STAT	= 0x0212,/*                                   */

	RSS_KEY		= 0x0220, /*               */
	RSS_CFG		= 0x0248, /*                   */
};

enum {
	HASH_TCP_IPV6_EX_CTRL	= 1<<5,
	HASH_IPV6_EX_CTRL	= 1<<4,
	HASH_TCP_IPV6_CTRL	= 1<<3,
	HASH_IPV6_CTRL		= 1<<2,
	HASH_TCP_IPV4_CTRL	= 1<<1,
	HASH_IPV4_CTRL		= 1<<0,

	HASH_ALL		= 0x3f,
};

enum {
	B6_EXT_REG	= 0x0300,/*                                   */
	B7_CFG_SPC	= 0x0380,/*                                    */
	B8_RQ1_REGS	= 0x0400,/*                 */
	B8_RQ2_REGS	= 0x0480,/*                 */
	B8_TS1_REGS	= 0x0600,/*                       */
	B8_TA1_REGS	= 0x0680,/*                        */
	B8_TS2_REGS	= 0x0700,/*                       */
	B8_TA2_REGS	= 0x0780,/*                       */
	B16_RAM_REGS	= 0x0800,/*                      */
};

/*                                                */
enum {
	B8_Q_REGS = 0x0400, /*                         */
	Q_D	= 0x00,	/*                             */
	Q_VLAN  = 0x20, /*                         */
	Q_DONE	= 0x24,	/*                   */
	Q_AC_L	= 0x28,	/*                                          */
	Q_AC_H	= 0x2c,	/*                                           */
	Q_BC	= 0x30,	/*                             */
	Q_CSR	= 0x34,	/*                                    */
	Q_TEST	= 0x38,	/*                              */

/*         */
	Q_WM	= 0x40,	/*                       */
	Q_AL	= 0x42,	/*                       */
	Q_RSP	= 0x44,	/*                                 */
	Q_RSL	= 0x46,	/*                               */
	Q_RP	= 0x48,	/*                          */
	Q_RL	= 0x4a,	/*                        */
	Q_WP	= 0x4c,	/*                           */
	Q_WSP	= 0x4d,	/*                                  */
	Q_WL	= 0x4e,	/*                         */
	Q_WSL	= 0x4f,	/*                                */
};
#define Q_ADDR(reg, offs) (B8_Q_REGS + (reg) + (offs))

/*                                */
enum {
	/*          */
	F_TX_CHK_AUTO_OFF = 1<<31, /*                                      */
	F_TX_CHK_AUTO_ON  = 1<<30, /*                                      */

	/*         */
	F_M_RX_RAM_DIS	= 1<<24, /*                              */

	/*                            */
};

/*                                                                      */
enum {
	Y2_B8_PREF_REGS		= 0x0450,

	PREF_UNIT_CTRL		= 0x00,	/*                         */
	PREF_UNIT_LAST_IDX	= 0x04,	/*                   */
	PREF_UNIT_ADDR_LO	= 0x08,	/*                                  */
	PREF_UNIT_ADDR_HI	= 0x0c,	/*                                  */
	PREF_UNIT_GET_IDX	= 0x10,	/*                  */
	PREF_UNIT_PUT_IDX	= 0x14,	/*                  */
	PREF_UNIT_FIFO_WP	= 0x20,	/*                           */
	PREF_UNIT_FIFO_RP	= 0x24,	/*                          */
	PREF_UNIT_FIFO_WM	= 0x28,	/*                       */
	PREF_UNIT_FIFO_LEV	= 0x2c,	/*                   */

	PREF_UNIT_MASK_IDX	= 0x0fff,
};
#define Y2_QADDR(q,reg)		(Y2_B8_PREF_REGS + (q) + (reg))

/*                             */
enum {

	RB_START	= 0x00,/*                                 */
	RB_END	= 0x04,/*                               */
	RB_WP	= 0x08,/*                                 */
	RB_RP	= 0x0c,/*                                */
	RB_RX_UTPP	= 0x10,/*                                         */
	RB_RX_LTPP	= 0x14,/*                                         */
	RB_RX_UTHP	= 0x18,/*                                      */
	RB_RX_LTHP	= 0x1c,/*                                      */
	/*                                                  */
	RB_PC	= 0x20,/*                                  */
	RB_LEV	= 0x24,/*                                  */
	RB_CTRL	= 0x28,/*                                    */
	RB_TST1	= 0x29,/*                                   */
	RB_TST2	= 0x2a,/*                                   */
};

/*                             */
enum {
	Q_R1	= 0x0000,	/*                 */
	Q_R2	= 0x0080,	/*                 */
	Q_XS1	= 0x0200,	/*                              */
	Q_XA1	= 0x0280,	/*                               */
	Q_XS2	= 0x0300,	/*                              */
	Q_XA2	= 0x0380,	/*                               */
};

/*                     */
enum {
	PHY_ADDR_MARV	= 0,
};

#define RB_ADDR(offs, queue) ((u16) B16_RAM_REGS + (queue) + (offs))


enum {
	LNK_SYNC_INI	= 0x0c30,/*                                 */
	LNK_SYNC_VAL	= 0x0c34,/*                                    */
	LNK_SYNC_CTRL	= 0x0c38,/*                                       */
	LNK_SYNC_TST	= 0x0c39,/*                                    */

	LNK_LED_REG	= 0x0c3c,/*                          */

/*                                       */

	RX_GMF_EA	= 0x0c40,/*                                 */
	RX_GMF_AF_THR	= 0x0c44,/*                                         */
	RX_GMF_CTRL_T	= 0x0c48,/*                                  */
	RX_GMF_FL_MSK	= 0x0c4c,/*                                */
	RX_GMF_FL_THR	= 0x0c50,/*                                     */
	RX_GMF_FL_CTRL	= 0x0c52,/*                                   */
	RX_GMF_TR_THR	= 0x0c54,/*                                          */
	RX_GMF_UP_THR	= 0x0c58,/*                                        */
	RX_GMF_LP_THR	= 0x0c5a,/*                                        */
	RX_GMF_VLAN	= 0x0c5c,/*                                        */
	RX_GMF_WP	= 0x0c60,/*                                   */

	RX_GMF_WLEV	= 0x0c68,/*                                 */

	RX_GMF_RP	= 0x0c70,/*                                  */

	RX_GMF_RLEV	= 0x0c78,/*                                */
};


/*                                    */

/*                              */
/*                                            */
/*                                            */
/*                                                  */
/*                                                   */
/*                                                  */
/*                                                   */
/*                                            */

/*                                             */
enum {
	BMU_IDLE	= 1<<31, /*                */
	BMU_RX_TCP_PKT	= 1<<30, /*                                       */
	BMU_RX_IP_PKT	= 1<<29, /*                                       */

	BMU_ENA_RX_RSS_HASH = 1<<15, /*                     */
	BMU_DIS_RX_RSS_HASH = 1<<14, /*                     */
	BMU_ENA_RX_CHKSUM = 1<<13, /*                                  */
	BMU_DIS_RX_CHKSUM = 1<<12, /*                                  */
	BMU_CLR_IRQ_PAR	= 1<<11, /*                                 */
	BMU_CLR_IRQ_TCP	= 1<<11, /*                                      */
	BMU_CLR_IRQ_CHK	= 1<<10, /*                 */
	BMU_STOP	= 1<<9, /*                   */
	BMU_START	= 1<<8, /*                   */
	BMU_FIFO_OP_ON	= 1<<7, /*                     */
	BMU_FIFO_OP_OFF	= 1<<6, /*                      */
	BMU_FIFO_ENA	= 1<<5, /*             */
	BMU_FIFO_RST	= 1<<4, /*             */
	BMU_OP_ON	= 1<<3, /*                    */
	BMU_OP_OFF	= 1<<2, /*                     */
	BMU_RST_CLR	= 1<<1, /*                          */
	BMU_RST_SET	= 1<<0, /*                 */

	BMU_CLR_RESET	= BMU_FIFO_RST | BMU_OP_OFF | BMU_RST_CLR,
	BMU_OPER_INIT	= BMU_CLR_IRQ_PAR | BMU_CLR_IRQ_CHK | BMU_START |
			  BMU_FIFO_ENA | BMU_OP_ON,

	BMU_WM_DEFAULT = 0x600,
	BMU_WM_PEX     = 0x80,
};

/*                                             */
								/*                        */
enum {
	BMU_TX_IPIDINCR_ON	= 1<<13, /*                         */
	BMU_TX_IPIDINCR_OFF	= 1<<12, /*                         */
	BMU_TX_CLR_IRQ_TCP	= 1<<11, /*                                          */
};

/*                                               */
enum {
	TBMU_TEST_BMU_TX_CHK_AUTO_OFF		= 1<<31, /*                                          */
	TBMU_TEST_BMU_TX_CHK_AUTO_ON		= 1<<30, /*                                         */
	TBMU_TEST_HOME_ADD_PAD_FIX1_EN		= 1<<29, /*                                   */
	TBMU_TEST_HOME_ADD_PAD_FIX1_DIS		= 1<<28, /*                                    */
	TBMU_TEST_ROUTING_ADD_FIX_EN		= 1<<27, /*                            */
	TBMU_TEST_ROUTING_ADD_FIX_DIS		= 1<<26, /*                             */
	TBMU_TEST_HOME_ADD_FIX_EN		= 1<<25, /*                                  */
	TBMU_TEST_HOME_ADD_FIX_DIS		= 1<<24, /*                                   */

	TBMU_TEST_TEST_RSPTR_ON			= 1<<22, /*                             */
	TBMU_TEST_TEST_RSPTR_OFF		= 1<<21, /*                              */
	TBMU_TEST_TESTSTEP_RSPTR		= 1<<20, /*                          */

	TBMU_TEST_TEST_RPTR_ON			= 1<<18, /*                      */
	TBMU_TEST_TEST_RPTR_OFF			= 1<<17, /*                       */
	TBMU_TEST_TESTSTEP_RPTR			= 1<<16, /*                   */

	TBMU_TEST_TEST_WSPTR_ON			= 1<<14, /*                              */
	TBMU_TEST_TEST_WSPTR_OFF		= 1<<13, /*                               */
	TBMU_TEST_TESTSTEP_WSPTR		= 1<<12, /*                           */

	TBMU_TEST_TEST_WPTR_ON			= 1<<10, /*                       */
	TBMU_TEST_TEST_WPTR_OFF			= 1<<9, /*                        */
	TBMU_TEST_TESTSTEP_WPTR			= 1<<8,			/*                    */

	TBMU_TEST_TEST_REQ_NB_ON		= 1<<6, /*                             */
	TBMU_TEST_TEST_REQ_NB_OFF		= 1<<5, /*                              */
	TBMU_TEST_TESTSTEP_REQ_NB		= 1<<4, /*                          */

	TBMU_TEST_TEST_DONE_IDX_ON		= 1<<2, /*                        */
	TBMU_TEST_TEST_DONE_IDX_OFF		= 1<<1, /*                         */
	TBMU_TEST_TESTSTEP_DONE_IDX		= 1<<0,	/*                     */
};

/*                                                                      */
/*                                                 */
enum {
	PREF_UNIT_OP_ON		= 1<<3,	/*                           */
	PREF_UNIT_OP_OFF	= 1<<2,	/*                               */
	PREF_UNIT_RST_CLR	= 1<<1,	/*                           */
	PREF_UNIT_RST_SET	= 1<<0,	/*                           */
};

/*                                                                 */
/*                                           */
/*                                        */
/*                                         */
/*                                        */
/*                                                   */
/*                                                   */
/*                                                  */
/*                                                  */
/*                                          */
/*                                           */

#define RB_MSK	0x0007ffff	/*                                     */
/*                                             */
/*                                             */

/*                                              */
enum {
	RB_ENA_STFWD	= 1<<5,	/*                         */
	RB_DIS_STFWD	= 1<<4,	/*                         */
	RB_ENA_OP_MD	= 1<<3,	/*                        */
	RB_DIS_OP_MD	= 1<<2,	/*                        */
	RB_RST_CLR	= 1<<1,	/*                         */
	RB_RST_SET	= 1<<0,	/*                         */
};


/*                                 */
enum {
	TX_GMF_EA	= 0x0d40,/*                                 */
	TX_GMF_AE_THR	= 0x0d44,/*                                         */
	TX_GMF_CTRL_T	= 0x0d48,/*                                  */

	TX_GMF_WP	= 0x0d60,/*                                    */
	TX_GMF_WSP	= 0x0d64,/*                                        */
	TX_GMF_WLEV	= 0x0d68,/*                                  */

	TX_GMF_RP	= 0x0d70,/*                                   */
	TX_GMF_RSTP	= 0x0d74,/*                                      */
	TX_GMF_RLEV	= 0x0d78,/*                                 */

	/*                                                 */
	ECU_AE_THR	= 0x0070, /*                        */
	ECU_TXFF_LEV	= 0x01a0, /*                   */
	ECU_JUMBO_WM	= 0x0080, /*                      */
};

/*                                 */
enum {
	B28_DPT_INI	= 0x0e00,/*                                       */
	B28_DPT_VAL	= 0x0e04,/*                                       */
	B28_DPT_CTRL	= 0x0e08,/*                                       */

	B28_DPT_TST	= 0x0e0a,/*                                       */
};

/*                                         */
enum {
	GMAC_TI_ST_VAL	= 0x0e14,/*                                  */
	GMAC_TI_ST_CTRL	= 0x0e18,/*                                  */
	GMAC_TI_ST_TST	= 0x0e1a,/*                                  */
};

/*                                       */
enum {
	POLL_CTRL	= 0x0e20, /*                                 */
	POLL_LAST_IDX	= 0x0e24,/*                                     */

	POLL_LIST_ADDR_LO= 0x0e28,/*                                    */
	POLL_LIST_ADDR_HI= 0x0e2c,/*                                     */
};

enum {
	SMB_CFG		 = 0x0e40, /*                              */
	SMB_CSR		 = 0x0e44, /*                                      */
};

enum {
	CPU_WDOG	 = 0x0e48, /*                           */
	CPU_CNTR	 = 0x0e4C, /*                          */
	CPU_TIM		 = 0x0e50,/*                                */
	CPU_AHB_ADDR	 = 0x0e54, /*                                 */
	CPU_AHB_WDATA	 = 0x0e58, /*                                 */
	CPU_AHB_RDATA	 = 0x0e5C, /*                                 */
	HCU_MAP_BASE	 = 0x0e60, /*                           */
	CPU_AHB_CTRL	 = 0x0e64, /*                                 */
	HCU_CCSR	 = 0x0e68, /*                                        */
	HCU_HCSR	 = 0x0e6C, /*                                         */
};

/*                                        */
enum {
	B28_Y2_SMB_CONFIG  = 0x0e40,/*                                  */
	B28_Y2_SMB_CSD_REG = 0x0e44,/*                                    */
	B28_Y2_ASF_IRQ_V_BASE=0x0e60,/*                            */

	B28_Y2_ASF_STAT_CMD= 0x0e68,/*                                   */
	B28_Y2_ASF_HOST_COM= 0x0e6c,/*                                   */
	B28_Y2_DATA_REG_1  = 0x0e70,/*                                 */
	B28_Y2_DATA_REG_2  = 0x0e74,/*                                 */
	B28_Y2_DATA_REG_3  = 0x0e78,/*                                 */
	B28_Y2_DATA_REG_4  = 0x0e7c,/*                                 */
};

/*                                    */
enum {
	STAT_CTRL	= 0x0e80,/*                               */
	STAT_LAST_IDX	= 0x0e84,/*                              */

	STAT_LIST_ADDR_LO= 0x0e88,/*                                     */
	STAT_LIST_ADDR_HI= 0x0e8c,/*                                      */
	STAT_TXA1_RIDX	= 0x0e90,/*                                     */
	STAT_TXS1_RIDX	= 0x0e92,/*                                     */
	STAT_TXA2_RIDX	= 0x0e94,/*                                     */
	STAT_TXS2_RIDX	= 0x0e96,/*                                     */
	STAT_TX_IDX_TH	= 0x0e98,/*                                      */
	STAT_PUT_IDX	= 0x0e9c,/*                             */

/*                                             */
	STAT_FIFO_WP	= 0x0ea0,/*                                      */
	STAT_FIFO_RP	= 0x0ea4,/*                                     */
	STAT_FIFO_RSP	= 0x0ea6,/*                                    */
	STAT_FIFO_LEVEL	= 0x0ea8,/*                              */
	STAT_FIFO_SHLVL	= 0x0eaa,/*                                     */
	STAT_FIFO_WM	= 0x0eac,/*                                  */
	STAT_FIFO_ISR_WM= 0x0ead,/*                                      */

/*                                             */
	STAT_LEV_TIMER_INI= 0x0eb0,/*                                    */
	STAT_LEV_TIMER_CNT= 0x0eb4,/*                                */
	STAT_LEV_TIMER_CTRL= 0x0eb8,/*                                */
	STAT_LEV_TIMER_TEST= 0x0eb9,/*                             */
	STAT_TX_TIMER_INI  = 0x0ec0,/*                                 */
	STAT_TX_TIMER_CNT  = 0x0ec4,/*                             */
	STAT_TX_TIMER_CTRL = 0x0ec8,/*                             */
	STAT_TX_TIMER_TEST = 0x0ec9,/*                          */
	STAT_ISR_TIMER_INI = 0x0ed0,/*                                  */
	STAT_ISR_TIMER_CNT = 0x0ed4,/*                              */
	STAT_ISR_TIMER_CTRL= 0x0ed8,/*                              */
	STAT_ISR_TIMER_TEST= 0x0ed9,/*                           */
};

enum {
	LINKLED_OFF 	     = 0x01,
	LINKLED_ON  	     = 0x02,
	LINKLED_LINKSYNC_OFF = 0x04,
	LINKLED_LINKSYNC_ON  = 0x08,
	LINKLED_BLINK_OFF    = 0x10,
	LINKLED_BLINK_ON     = 0x20,
};

/*                                              */
enum {
	GMAC_CTRL	= 0x0f00,/*                         */
	GPHY_CTRL	= 0x0f04,/*                         */
	GMAC_IRQ_SRC	= 0x0f08,/*                                  */
	GMAC_IRQ_MSK	= 0x0f0c,/*                                */
	GMAC_LINK_CTRL	= 0x0f10,/*                         */

/*                                                            */
	WOL_CTRL_STAT	= 0x0f20,/*                               */
	WOL_MATCH_CTL	= 0x0f22,/*                              */
	WOL_MATCH_RES	= 0x0f23,/*                             */
	WOL_MAC_ADDR	= 0x0f24,/*                        */
	WOL_PATT_RPTR	= 0x0f2c,/*                                 */

/*                                           */
	WOL_PATT_LEN_LO	= 0x0f30,/*                                */
	WOL_PATT_LEN_HI	= 0x0f34,/*                                */

/*                                            */
	WOL_PATT_CNT_0	= 0x0f38,/*                                 */
	WOL_PATT_CNT_4	= 0x0f3c,/*                                 */
};
#define WOL_REGS(port, x)	(x + (port)*0x80)

enum {
	WOL_PATT_RAM_1	= 0x1000,/*                         */
	WOL_PATT_RAM_2	= 0x1400,/*                         */
};
#define WOL_PATT_RAM_BASE(port)	(WOL_PATT_RAM_1 + (port)*0x400)

enum {
	BASE_GMAC_1	= 0x2800,/*                  */
	BASE_GMAC_2	= 0x3800,/*                  */
};

/*
                                                     
 */
enum {
	PHY_MARV_CTRL		= 0x00,/*                                 */
	PHY_MARV_STAT		= 0x01,/*                                */
	PHY_MARV_ID0		= 0x02,/*                             */
	PHY_MARV_ID1		= 0x03,/*                             */
	PHY_MARV_AUNE_ADV	= 0x04,/*                                    */
	PHY_MARV_AUNE_LP	= 0x05,/*                                  */
	PHY_MARV_AUNE_EXP	= 0x06,/*                                    */
	PHY_MARV_NEPG		= 0x07,/*                               */
	PHY_MARV_NEPG_LP	= 0x08,/*                                   */
	/*                           */
	PHY_MARV_1000T_CTRL	= 0x09,/*                                   */
	PHY_MARV_1000T_STAT	= 0x0a,/*                                  */
	PHY_MARV_EXT_STAT	= 0x0f,/*                                */
	PHY_MARV_PHY_CTRL	= 0x10,/*                                  */
	PHY_MARV_PHY_STAT	= 0x11,/*                                  */
	PHY_MARV_INT_MASK	= 0x12,/*                               */
	PHY_MARV_INT_STAT	= 0x13,/*                                 */
	PHY_MARV_EXT_CTRL	= 0x14,/*                                   */
	PHY_MARV_RXE_CNT	= 0x15,/*                                  */
	PHY_MARV_EXT_ADR	= 0x16,/*                                     */
	PHY_MARV_PORT_IRQ	= 0x17,/*                                      */
	PHY_MARV_LED_CTRL	= 0x18,/*                            */
	PHY_MARV_LED_OVER	= 0x19,/*                                    */
	PHY_MARV_EXT_CTRL_2	= 0x1a,/*                                     */
	PHY_MARV_EXT_P_STAT	= 0x1b,/*                                    */
	PHY_MARV_CABLE_DIAG	= 0x1c,/*                                 */
	PHY_MARV_PAGE_ADDR	= 0x1d,/*                                      */
	PHY_MARV_PAGE_DATA	= 0x1e,/*                                   */

/*                                             */
	PHY_MARV_FE_LED_PAR	= 0x16,/*                                     */
	PHY_MARV_FE_LED_SER	= 0x17,/*                                     */
	PHY_MARV_FE_VCT_TX	= 0x1a,/*                                    */
	PHY_MARV_FE_VCT_RX	= 0x1b,/*                                    */
	PHY_MARV_FE_SPEC_2	= 0x1c,/*                                    */
};

enum {
	PHY_CT_RESET	= 1<<15, /*                                         */
	PHY_CT_LOOP	= 1<<14, /*                                  */
	PHY_CT_SPS_LSB	= 1<<13, /*                                 */
	PHY_CT_ANE	= 1<<12, /*                                  */
	PHY_CT_PDOWN	= 1<<11, /*                         */
	PHY_CT_ISOL	= 1<<10, /*                      */
	PHY_CT_RE_CFG	= 1<<9, /*                                       */
	PHY_CT_DUP_MD	= 1<<8, /*                     */
	PHY_CT_COL_TST	= 1<<7, /*                                */
	PHY_CT_SPS_MSB	= 1<<6, /*                                 */
};

enum {
	PHY_CT_SP1000	= PHY_CT_SPS_MSB, /*                           */
	PHY_CT_SP100	= PHY_CT_SPS_LSB, /*                           */
	PHY_CT_SP10	= 0,		  /*                           */
};

enum {
	PHY_ST_EXT_ST	= 1<<8, /*                                 */

	PHY_ST_PRE_SUP	= 1<<6, /*                              */
	PHY_ST_AN_OVER	= 1<<5, /*                               */
	PHY_ST_REM_FLT	= 1<<4, /*                                         */
	PHY_ST_AN_CAP	= 1<<3, /*                                     */
	PHY_ST_LSYNC	= 1<<2, /*                           */
	PHY_ST_JAB_DET	= 1<<1, /*                         */
	PHY_ST_EXT_REG	= 1<<0, /*                                     */
};

enum {
	PHY_I1_OUI_MSK	= 0x3f<<10, /*                                    */
	PHY_I1_MOD_NUM	= 0x3f<<4, /*                          */
	PHY_I1_REV_MSK	= 0xf, /*                             */
};

/*                           */
enum {
	PHY_MARV_ID0_VAL= 0x0141, /*                           */

	PHY_BCOM_ID1_A1	= 0x6041,
	PHY_BCOM_ID1_B2	= 0x6043,
	PHY_BCOM_ID1_C0	= 0x6044,
	PHY_BCOM_ID1_C5	= 0x6047,

	PHY_MARV_ID1_B0	= 0x0C23, /*                      */
	PHY_MARV_ID1_B2	= 0x0C25, /*                          */
	PHY_MARV_ID1_C2	= 0x0CC2, /*                        */
	PHY_MARV_ID1_Y2	= 0x0C91, /*                       */
	PHY_MARV_ID1_FE = 0x0C83, /*                                 */
	PHY_MARV_ID1_ECU= 0x0CB0, /*                                  */
};

/*                             */
enum {
	PHY_AN_NXT_PG	= 1<<15, /*                           */
	PHY_AN_ACK	= 1<<14, /*                                   */
	PHY_AN_RF	= 1<<13, /*                           */

	PHY_AN_PAUSE_ASYM = 1<<11,/*                            */
	PHY_AN_PAUSE_CAP = 1<<10, /*                       */
	PHY_AN_100BASE4	= 1<<9, /*                                   */
	PHY_AN_100FULL	= 1<<8, /*                                    */
	PHY_AN_100HALF	= 1<<7, /*                                    */
	PHY_AN_10FULL	= 1<<6, /*                                   */
	PHY_AN_10HALF	= 1<<5, /*                                   */
	PHY_AN_CSMA	= 1<<0, /*                                */
	PHY_AN_SEL	= 0x1f, /*                                         */
	PHY_AN_FULL	= PHY_AN_100FULL | PHY_AN_10FULL | PHY_AN_CSMA,
	PHY_AN_ALL	= PHY_AN_10HALF | PHY_AN_10FULL |
		  	  PHY_AN_100HALF | PHY_AN_100FULL,
};

/*                                                               */
/*                                                               */
enum {
	PHY_B_1000S_MSF	= 1<<15, /*                            */
	PHY_B_1000S_MSR	= 1<<14, /*                             */
	PHY_B_1000S_LRS	= 1<<13, /*                               */
	PHY_B_1000S_RRS	= 1<<12, /*                                */
	PHY_B_1000S_LP_FD	= 1<<11, /*                             */
	PHY_B_1000S_LP_HD	= 1<<10, /*                             */
									/*                     */
	PHY_B_1000S_IEC	= 0xff, /*                             */
};

/*                   */
enum {
	PHY_M_AN_NXT_PG	= 1<<15, /*                   */
	PHY_M_AN_ACK	= 1<<14, /*                           */
	PHY_M_AN_RF	= 1<<13, /*              */

	PHY_M_AN_ASP	= 1<<11, /*                  */
	PHY_M_AN_PC	= 1<<10, /*                       */
	PHY_M_AN_100_T4	= 1<<9, /*                                */
	PHY_M_AN_100_FD	= 1<<8, /*                                  */
	PHY_M_AN_100_HD	= 1<<7, /*                                  */
	PHY_M_AN_10_FD	= 1<<6, /*                                 */
	PHY_M_AN_10_HD	= 1<<5, /*                                 */
	PHY_M_AN_SEL_MSK =0x1f<<4,	/*                                 */
};

/*                                           */
enum {
	PHY_M_AN_ASP_X	= 1<<8, /*                  */
	PHY_M_AN_PC_X	= 1<<7, /*                       */
	PHY_M_AN_1000X_AHD	= 1<<6, /*                                   */
	PHY_M_AN_1000X_AFD	= 1<<5, /*                                   */
};

/*                                                        */
enum {
	PHY_M_P_NO_PAUSE_X	= 0<<7,/*                           */
	PHY_M_P_SYM_MD_X	= 1<<7, /*                                  */
	PHY_M_P_ASYM_MD_X	= 2<<7,/*                                   */
	PHY_M_P_BOTH_MD_X	= 3<<7,/*                             */
};

/*                                                                */
enum {
	PHY_M_1000C_TEST	= 7<<13,/*                        */
	PHY_M_1000C_MSE	= 1<<12, /*                            */
	PHY_M_1000C_MSC	= 1<<11, /*                              */
	PHY_M_1000C_MPD	= 1<<10, /*                   */
	PHY_M_1000C_AFD	= 1<<9, /*                       */
	PHY_M_1000C_AHD	= 1<<8, /*                       */
};

/*                                                             */
enum {
	PHY_M_PC_TX_FFD_MSK	= 3<<14,/*                                */
	PHY_M_PC_RX_FFD_MSK	= 3<<12,/*                                */
	PHY_M_PC_ASS_CRS_TX	= 1<<11, /*                        */
	PHY_M_PC_FL_GOOD	= 1<<10, /*                 */
	PHY_M_PC_EN_DET_MSK	= 3<<8,/*                                */
	PHY_M_PC_ENA_EXT_D	= 1<<7, /*                             */
	PHY_M_PC_MDIX_MSK	= 3<<5,/*                                   */
	PHY_M_PC_DIS_125CLK	= 1<<4, /*                 */
	PHY_M_PC_MAC_POW_UP	= 1<<3, /*              */
	PHY_M_PC_SQE_T_ENA	= 1<<2, /*                  */
	PHY_M_PC_POL_R_DIS	= 1<<1, /*                            */
	PHY_M_PC_DIS_JABBER	= 1<<0, /*                */
};

enum {
	PHY_M_PC_EN_DET		= 2<<8,	/*                        */
	PHY_M_PC_EN_DET_PLUS	= 3<<8, /*                             */
};

#define PHY_M_PC_MDI_XMODE(x)	(((u16)(x)<<5) & PHY_M_PC_MDIX_MSK)

enum {
	PHY_M_PC_MAN_MDI	= 0, /*                               */
	PHY_M_PC_MAN_MDIX	= 1, /*                                */
	PHY_M_PC_ENA_AUTO	= 3, /*                                 */
};

/*                                                        */
enum {
	PHY_M_PC_COP_TX_DIS	= 1<<3, /*                            */
	PHY_M_PC_POW_D_ENA	= 1<<2,	/*                   */
};

/*                                             */
enum {
	PHY_M_PC_ENA_DTE_DT	= 1<<15, /*                                        */
	PHY_M_PC_ENA_ENE_DT	= 1<<14, /*                                      */
	PHY_M_PC_DIS_NLP_CK	= 1<<13, /*                                      */
	PHY_M_PC_ENA_LIP_NP	= 1<<12, /*                                    */
	PHY_M_PC_DIS_NLP_GN	= 1<<11, /*                                     */

	PHY_M_PC_DIS_SCRAMB	= 1<<9, /*                   */
	PHY_M_PC_DIS_FEFI	= 1<<8, /*                                     */

	PHY_M_PC_SH_TP_SEL	= 1<<6, /*                              */
	PHY_M_PC_RX_FD_MSK	= 3<<2,/*                                */
};

/*                                                               */
enum {
	PHY_M_PS_SPEED_MSK	= 3<<14, /*                        */
	PHY_M_PS_SPEED_1000	= 1<<15, /*                 */
	PHY_M_PS_SPEED_100	= 1<<14, /*                 */
	PHY_M_PS_SPEED_10	= 0,	 /*                 */
	PHY_M_PS_FULL_DUP	= 1<<13, /*             */
	PHY_M_PS_PAGE_REC	= 1<<12, /*               */
	PHY_M_PS_SPDUP_RES	= 1<<11, /*                         */
	PHY_M_PS_LINK_UP	= 1<<10, /*         */
	PHY_M_PS_CABLE_MSK	= 7<<7,  /*                               */
	PHY_M_PS_MDI_X_STAT	= 1<<6,  /*                             */
	PHY_M_PS_DOWNS_STAT	= 1<<5,  /*                              */
	PHY_M_PS_ENDET_STAT	= 1<<4,  /*                              */
	PHY_M_PS_TX_P_EN	= 1<<3,  /*                  */
	PHY_M_PS_RX_P_EN	= 1<<2,  /*                  */
	PHY_M_PS_POL_REV	= 1<<1,  /*                   */
	PHY_M_PS_JABBER		= 1<<0,  /*        */
};

#define PHY_M_PS_PAUSE_MSK	(PHY_M_PS_TX_P_EN | PHY_M_PS_RX_P_EN)

/*                                             */
enum {
	PHY_M_PS_DTE_DETECT	= 1<<15, /*                                        */
	PHY_M_PS_RES_SPEED	= 1<<14, /*                                       */
};

enum {
	PHY_M_IS_AN_ERROR	= 1<<15, /*                        */
	PHY_M_IS_LSP_CHANGE	= 1<<14, /*                    */
	PHY_M_IS_DUP_CHANGE	= 1<<13, /*                     */
	PHY_M_IS_AN_PR		= 1<<12, /*               */
	PHY_M_IS_AN_COMPL	= 1<<11, /*                            */
	PHY_M_IS_LST_CHANGE	= 1<<10, /*                     */
	PHY_M_IS_SYMB_ERROR	= 1<<9, /*              */
	PHY_M_IS_FALSE_CARR	= 1<<8, /*               */
	PHY_M_IS_FIFO_ERROR	= 1<<7, /*                              */
	PHY_M_IS_MDI_CHANGE	= 1<<6, /*                       */
	PHY_M_IS_DOWNSH_DET	= 1<<5, /*                    */
	PHY_M_IS_END_CHANGE	= 1<<4, /*                       */

	PHY_M_IS_DTE_CHANGE	= 1<<2, /*                               */
	PHY_M_IS_POL_CHANGE	= 1<<1, /*                  */
	PHY_M_IS_JABBER		= 1<<0, /*        */

	PHY_M_DEF_MSK		= PHY_M_IS_LSP_CHANGE | PHY_M_IS_LST_CHANGE
				 | PHY_M_IS_DUP_CHANGE,
	PHY_M_AN_MSK	       = PHY_M_IS_AN_ERROR | PHY_M_IS_AN_COMPL,
};


/*                                                              */
enum {
	PHY_M_EC_ENA_BC_EXT = 1<<15, /*                                        */
	PHY_M_EC_ENA_LIN_LB = 1<<14, /*                                     */

	PHY_M_EC_DIS_LINK_P = 1<<12, /*                                    */
	PHY_M_EC_M_DSC_MSK  = 3<<10, /*                                      */
					/*                */
	PHY_M_EC_S_DSC_MSK  = 3<<8,/*                                      */
				       /*                */
	PHY_M_EC_M_DSC_MSK2 = 7<<9,/*                                      */
					/*                */
	PHY_M_EC_DOWN_S_ENA = 1<<8, /*                                 */
					/*                                   */
	PHY_M_EC_RX_TIM_CT  = 1<<7, /*                        */
	PHY_M_EC_MAC_S_MSK  = 7<<4,/*                                      */
	PHY_M_EC_FIB_AN_ENA = 1<<3, /*                                        */
	PHY_M_EC_DTE_D_ENA  = 1<<2, /*                                  */
	PHY_M_EC_TX_TIM_CT  = 1<<1, /*                         */
	PHY_M_EC_TRANS_DIS  = 1<<0, /*                                    */

	PHY_M_10B_TE_ENABLE = 1<<7, /*                                      */
};
#define PHY_M_EC_M_DSC(x)	((u16)(x)<<10 & PHY_M_EC_M_DSC_MSK)
					/*                            */
#define PHY_M_EC_S_DSC(x)	((u16)(x)<<8 & PHY_M_EC_S_DSC_MSK)
					/*                             */
#define PHY_M_EC_DSC_2(x)	((u16)(x)<<9 & PHY_M_EC_M_DSC_MSK2)
					/*                                */
#define PHY_M_EC_MAC_S(x)	((u16)(x)<<4 & PHY_M_EC_MAC_S_MSK)
					/*                              */

/*                                                 */
enum {
	PHY_M_PC_DIS_LINK_Pa	= 1<<15,/*                     */
	PHY_M_PC_DSC_MSK	= 7<<12,/*                               */
	PHY_M_PC_DOWN_S_ENA	= 1<<11,/*                  */
};
/*                                   */

#define PHY_M_PC_DSC(x)			(((u16)(x)<<12) & PHY_M_PC_DSC_MSK)
											/*                                */
enum {
	MAC_TX_CLK_0_MHZ	= 2,
	MAC_TX_CLK_2_5_MHZ	= 6,
	MAC_TX_CLK_25_MHZ 	= 7,
};

/*                                                       */
enum {
	PHY_M_LEDC_DIS_LED	= 1<<15, /*             */
	PHY_M_LEDC_PULS_MSK	= 7<<12,/*                                */
	PHY_M_LEDC_F_INT	= 1<<11, /*                 */
	PHY_M_LEDC_BL_R_MSK	= 7<<8,/*                             */
	PHY_M_LEDC_DP_C_LSB	= 1<<7, /*                                    */
	PHY_M_LEDC_TX_C_LSB	= 1<<6, /*                                */
	PHY_M_LEDC_LK_C_MSK	= 7<<3,/*                               */
					/*                */
};

enum {
	PHY_M_LEDC_LINK_MSK	= 3<<3,/*                               */
									/*                */
	PHY_M_LEDC_DP_CTRL	= 1<<2, /*                */
	PHY_M_LEDC_DP_C_MSB	= 1<<2, /*                                    */
	PHY_M_LEDC_RX_CTRL	= 1<<1, /*                    */
	PHY_M_LEDC_TX_CTRL	= 1<<0, /*                    */
	PHY_M_LEDC_TX_C_MSB	= 1<<0, /*                                */
};

#define PHY_M_LED_PULS_DUR(x)	(((u16)(x)<<12) & PHY_M_LEDC_PULS_MSK)

/*                                                                     */
enum {
	PHY_M_POLC_LS1M_MSK	= 0xf<<12, /*                                  */
	PHY_M_POLC_IS0M_MSK	= 0xf<<8,  /*                                   */
	PHY_M_POLC_LOS_MSK	= 0x3<<6,  /*                                 */
	PHY_M_POLC_INIT_MSK	= 0x3<<4,  /*                                  */
	PHY_M_POLC_STA1_MSK	= 0x3<<2,  /*                                   */
	PHY_M_POLC_STA0_MSK	= 0x3,     /*                                   */
};

#define PHY_M_POLC_LS1_P_MIX(x)	(((x)<<12) & PHY_M_POLC_LS1M_MSK)
#define PHY_M_POLC_IS0_P_MIX(x)	(((x)<<8) & PHY_M_POLC_IS0M_MSK)
#define PHY_M_POLC_LOS_CTRL(x)	(((x)<<6) & PHY_M_POLC_LOS_MSK)
#define PHY_M_POLC_INIT_CTRL(x)	(((x)<<4) & PHY_M_POLC_INIT_MSK)
#define PHY_M_POLC_STA1_CTRL(x)	(((x)<<2) & PHY_M_POLC_STA1_MSK)
#define PHY_M_POLC_STA0_CTRL(x)	(((x)<<0) & PHY_M_POLC_STA0_MSK)

enum {
	PULS_NO_STR	= 0,/*                     */
	PULS_21MS	= 1,/*                */
	PULS_42MS	= 2,/*                */
	PULS_84MS	= 3,/*                 */
	PULS_170MS	= 4,/*                  */
	PULS_340MS	= 5,/*                  */
	PULS_670MS	= 6,/*                 */
	PULS_1300MS	= 7,/*                */
};

#define PHY_M_LED_BLINK_RT(x)	(((u16)(x)<<8) & PHY_M_LEDC_BL_R_MSK)

enum {
	BLINK_42MS	= 0,/*       */
	BLINK_84MS	= 1,/*       */
	BLINK_170MS	= 2,/*        */
	BLINK_340MS	= 3,/*        */
	BLINK_670MS	= 4,/*        */
};

/*                                                               */
#define PHY_M_LED_MO_SGMII(x)	((x)<<14)	/*                             */

#define PHY_M_LED_MO_DUP(x)	((x)<<10)	/*                     */
#define PHY_M_LED_MO_10(x)	((x)<<8)	/*                      */
#define PHY_M_LED_MO_100(x)	((x)<<6)	/*                       */
#define PHY_M_LED_MO_1000(x)	((x)<<4)	/*                        */
#define PHY_M_LED_MO_RX(x)	((x)<<2)	/*                 */
#define PHY_M_LED_MO_TX(x)	((x)<<0)	/*                 */

enum led_mode {
	MO_LED_NORM  = 0,
	MO_LED_BLINK = 1,
	MO_LED_OFF   = 2,
	MO_LED_ON    = 3,
};

/*                                                                  */
enum {
	PHY_M_EC2_FI_IMPED	= 1<<6, /*                        */
	PHY_M_EC2_FO_IMPED	= 1<<5, /*                        */
	PHY_M_EC2_FO_M_CLK	= 1<<4, /*                         */
	PHY_M_EC2_FO_BOOST	= 1<<3, /*                    */
	PHY_M_EC2_FO_AM_MSK	= 7,/*                                    */
};

/*                                                                  */
enum {
	PHY_M_FC_AUTO_SEL	= 1<<15, /*                             */
	PHY_M_FC_AN_REG_ACC	= 1<<14, /*                             */
	PHY_M_FC_RESOLUTION	= 1<<13, /*                         */
	PHY_M_SER_IF_AN_BP	= 1<<12, /*                          */
	PHY_M_SER_IF_BP_ST	= 1<<11, /*                          */
	PHY_M_IRQ_POLARITY	= 1<<10, /*              */
	PHY_M_DIS_AUT_MED	= 1<<9, /*                                    */
	/*                */

	PHY_M_UNDOC1		= 1<<7, /*                     */
	PHY_M_DTE_POW_STAT	= 1<<4, /*                                 */
	PHY_M_MODE_MASK	= 0xf, /*                                     */
};

/*                                             */
/*                                                                   */
									/*                                        */
enum {
	PHY_M_FELP_LED2_MSK = 0xf<<8,	/*                              */
	PHY_M_FELP_LED1_MSK = 0xf<<4,	/*                             */
	PHY_M_FELP_LED0_MSK = 0xf, /*                               */
};

#define PHY_M_FELP_LED2_CTRL(x)	(((u16)(x)<<8) & PHY_M_FELP_LED2_MSK)
#define PHY_M_FELP_LED1_CTRL(x)	(((u16)(x)<<4) & PHY_M_FELP_LED1_MSK)
#define PHY_M_FELP_LED0_CTRL(x)	(((u16)(x)<<0) & PHY_M_FELP_LED0_MSK)

enum {
	LED_PAR_CTRL_COLX	= 0x00,
	LED_PAR_CTRL_ERROR	= 0x01,
	LED_PAR_CTRL_DUPLEX	= 0x02,
	LED_PAR_CTRL_DP_COL	= 0x03,
	LED_PAR_CTRL_SPEED	= 0x04,
	LED_PAR_CTRL_LINK	= 0x05,
	LED_PAR_CTRL_TX		= 0x06,
	LED_PAR_CTRL_RX		= 0x07,
	LED_PAR_CTRL_ACT	= 0x08,
	LED_PAR_CTRL_LNK_RX	= 0x09,
	LED_PAR_CTRL_LNK_AC	= 0x0a,
	LED_PAR_CTRL_ACT_BL	= 0x0b,
	LED_PAR_CTRL_TX_BL	= 0x0c,
	LED_PAR_CTRL_RX_BL	= 0x0d,
	LED_PAR_CTRL_COL_BL	= 0x0e,
	LED_PAR_CTRL_INACT	= 0x0f
};

/*                                                                */
enum {
	PHY_M_FESC_DIS_WAIT	= 1<<2, /*                            */
	PHY_M_FESC_ENA_MCLK	= 1<<1, /*                                   */
	PHY_M_FESC_SEL_CL_A	= 1<<0, /*                                 */
};

/*                                                 */
/*                                                                     */
enum {
	PHY_M_FIB_FORCE_LNK	= 1<<10,/*                 */
	PHY_M_FIB_SIGD_POL	= 1<<9,	/*                 */
	PHY_M_FIB_TX_DIS	= 1<<3,	/*                     */
};

/*                                                 */
/*                                                                   */
enum {
	PHY_M_MAC_MD_MSK	= 7<<7, /*                              */
	PHY_M_MAC_GMIF_PUP	= 1<<3,	/*                              */
	PHY_M_MAC_MD_AUTO	= 3,/*                        */
	PHY_M_MAC_MD_COPPER	= 5,/*             */
	PHY_M_MAC_MD_1000BX	= 7,/*                 */
};
#define PHY_M_MAC_MODE_SEL(x)	(((x)<<7) & PHY_M_MAC_MD_MSK)

/*                                                                  */
enum {
	PHY_M_LEDC_LOS_MSK	= 0xf<<12,/*                                */
	PHY_M_LEDC_INIT_MSK	= 0xf<<8, /*                                 */
	PHY_M_LEDC_STA1_MSK	= 0xf<<4,/*                                  */
	PHY_M_LEDC_STA0_MSK	= 0xf, /*                                  */
};

#define PHY_M_LEDC_LOS_CTRL(x)	(((x)<<12) & PHY_M_LEDC_LOS_MSK)
#define PHY_M_LEDC_INIT_CTRL(x)	(((x)<<8) & PHY_M_LEDC_INIT_MSK)
#define PHY_M_LEDC_STA1_CTRL(x)	(((x)<<4) & PHY_M_LEDC_STA1_MSK)
#define PHY_M_LEDC_STA0_CTRL(x)	(((x)<<0) & PHY_M_LEDC_STA0_MSK)

/*                 */
/*                */
enum {
	GM_GP_STAT	= 0x0000,	/*                                   */
	GM_GP_CTRL	= 0x0004,	/*                                    */
	GM_TX_CTRL	= 0x0008,	/*                                  */
	GM_RX_CTRL	= 0x000c,	/*                                 */
	GM_TX_FLOW_CTRL	= 0x0010,	/*                                  */
	GM_TX_PARAM	= 0x0014,	/*                                    */
	GM_SERIAL_MODE	= 0x0018,	/*                                 */
/*                          */
	GM_SRC_ADDR_1L	= 0x001c,	/*                                   */
	GM_SRC_ADDR_1M	= 0x0020,	/*                                      */
	GM_SRC_ADDR_1H	= 0x0024,	/*                                    */
	GM_SRC_ADDR_2L	= 0x0028,	/*                                   */
	GM_SRC_ADDR_2M	= 0x002c,	/*                                      */
	GM_SRC_ADDR_2H	= 0x0030,	/*                                    */

/*                                  */
	GM_MC_ADDR_H1	= 0x0034,	/*                                     */
	GM_MC_ADDR_H2	= 0x0038,	/*                                     */
	GM_MC_ADDR_H3	= 0x003c,	/*                                     */
	GM_MC_ADDR_H4	= 0x0040,	/*                                     */

/*                            */
	GM_TX_IRQ_SRC	= 0x0044,	/*                                   */
	GM_RX_IRQ_SRC	= 0x0048,	/*                                   */
	GM_TR_IRQ_SRC	= 0x004c,	/*                                   */

/*                          */
	GM_TX_IRQ_MSK	= 0x0050,	/*                                 */
	GM_RX_IRQ_MSK	= 0x0054,	/*                                 */
	GM_TR_IRQ_MSK	= 0x0058,	/*                                 */

/*                                             */
	GM_SMI_CTRL	= 0x0080,	/*                                 */
	GM_SMI_DATA	= 0x0084,	/*                              */
	GM_PHY_ADDR	= 0x0088,	/*                                  */
/*              */
	GM_MIB_CNT_BASE	= 0x0100,	/*                              */
	GM_MIB_CNT_END	= 0x025C,	/*                  */
};


/*
                                                     
                                                    
 */
enum {
	GM_RXF_UC_OK    = GM_MIB_CNT_BASE + 0,	/*                            */
	GM_RXF_BC_OK	= GM_MIB_CNT_BASE + 8,	/*                              */
	GM_RXF_MPAUSE	= GM_MIB_CNT_BASE + 16,	/*                                */
	GM_RXF_MC_OK	= GM_MIB_CNT_BASE + 24,	/*                              */
	GM_RXF_FCS_ERR	= GM_MIB_CNT_BASE + 32,	/*                           */

	GM_RXO_OK_LO	= GM_MIB_CNT_BASE + 48,	/*                        */
	GM_RXO_OK_HI	= GM_MIB_CNT_BASE + 56,	/*                         */
	GM_RXO_ERR_LO	= GM_MIB_CNT_BASE + 64,	/*                             */
	GM_RXO_ERR_HI	= GM_MIB_CNT_BASE + 72,	/*                              */
	GM_RXF_SHT	= GM_MIB_CNT_BASE + 80,	/*                             */
	GM_RXE_FRAG	= GM_MIB_CNT_BASE + 88,	/*                                       */
	GM_RXF_64B	= GM_MIB_CNT_BASE + 96,	/*                  */
	GM_RXF_127B	= GM_MIB_CNT_BASE + 104,/*                      */
	GM_RXF_255B	= GM_MIB_CNT_BASE + 112,/*                       */
	GM_RXF_511B	= GM_MIB_CNT_BASE + 120,/*                       */
	GM_RXF_1023B	= GM_MIB_CNT_BASE + 128,/*                        */
	GM_RXF_1518B	= GM_MIB_CNT_BASE + 136,/*                         */
	GM_RXF_MAX_SZ	= GM_MIB_CNT_BASE + 144,/*                            */
	GM_RXF_LNG_ERR	= GM_MIB_CNT_BASE + 152,/*                         */
	GM_RXF_JAB_PKT	= GM_MIB_CNT_BASE + 160,/*                        */

	GM_RXE_FIFO_OV	= GM_MIB_CNT_BASE + 176,/*                        */
	GM_TXF_UC_OK	= GM_MIB_CNT_BASE + 192,/*                           */
	GM_TXF_BC_OK	= GM_MIB_CNT_BASE + 200,/*                             */
	GM_TXF_MPAUSE	= GM_MIB_CNT_BASE + 208,/*                               */
	GM_TXF_MC_OK	= GM_MIB_CNT_BASE + 216,/*                             */
	GM_TXO_OK_LO	= GM_MIB_CNT_BASE + 224,/*                           */
	GM_TXO_OK_HI	= GM_MIB_CNT_BASE + 232,/*                            */
	GM_TXF_64B	= GM_MIB_CNT_BASE + 240,/*                  */
	GM_TXF_127B	= GM_MIB_CNT_BASE + 248,/*                      */
	GM_TXF_255B	= GM_MIB_CNT_BASE + 256,/*                       */
	GM_TXF_511B	= GM_MIB_CNT_BASE + 264,/*                       */
	GM_TXF_1023B	= GM_MIB_CNT_BASE + 272,/*                        */
	GM_TXF_1518B	= GM_MIB_CNT_BASE + 280,/*                         */
	GM_TXF_MAX_SZ	= GM_MIB_CNT_BASE + 288,/*                            */

	GM_TXF_COL	= GM_MIB_CNT_BASE + 304,/*              */
	GM_TXF_LAT_COL	= GM_MIB_CNT_BASE + 312,/*                   */
	GM_TXF_ABO_COL	= GM_MIB_CNT_BASE + 320,/*                               */
	GM_TXF_MUL_COL	= GM_MIB_CNT_BASE + 328,/*                       */
	GM_TXF_SNG_COL	= GM_MIB_CNT_BASE + 336,/*                     */
	GM_TXE_FIFO_UR	= GM_MIB_CNT_BASE + 344,/*                        */
};

/*                      */
/*                                                       */
enum {
	GM_GPSR_SPEED		= 1<<15, /*                                   */
	GM_GPSR_DUPLEX		= 1<<14, /*                                */
	GM_GPSR_FC_TX_DIS	= 1<<13, /*                                       */
	GM_GPSR_LINK_UP		= 1<<12, /*                        */
	GM_GPSR_PAUSE		= 1<<11, /*                     */
	GM_GPSR_TX_ACTIVE	= 1<<10, /*                        */
	GM_GPSR_EXC_COL		= 1<<9,	/*                                       */
	GM_GPSR_LAT_COL		= 1<<8,	/*                                  */

	GM_GPSR_PHY_ST_CH	= 1<<5,	/*                           */
	GM_GPSR_GIG_SPEED	= 1<<4,	/*                                       */
	GM_GPSR_PART_MODE	= 1<<3,	/*                        */
	GM_GPSR_FC_RX_DIS	= 1<<2,	/*                                       */
	GM_GPSR_PROM_EN		= 1<<1,	/*                                  */
};

/*                                                        */
enum {
	GM_GPCR_PROM_ENA	= 1<<14,	/*                                 */
	GM_GPCR_FC_TX_DIS	= 1<<13, /*                                      */
	GM_GPCR_TX_ENA		= 1<<12, /*                         */
	GM_GPCR_RX_ENA		= 1<<11, /*                        */
	GM_GPCR_BURST_ENA	= 1<<10, /*                           */
	GM_GPCR_LOOP_ENA	= 1<<9,	/*                                  */
	GM_GPCR_PART_ENA	= 1<<8,	/*                               */
	GM_GPCR_GIGS_ENA	= 1<<7,	/*                                   */
	GM_GPCR_FL_PASS		= 1<<6,	/*                         */
	GM_GPCR_DUP_FULL	= 1<<5,	/*                          */
	GM_GPCR_FC_RX_DIS	= 1<<4,	/*                                      */
	GM_GPCR_SPEED_100	= 1<<3,   /*                             */
	GM_GPCR_AU_DUP_DIS	= 1<<2,	/*                                    */
	GM_GPCR_AU_FCT_DIS	= 1<<1,	/*                                     */
	GM_GPCR_AU_SPD_DIS	= 1<<0,	/*                                   */
};

#define GM_GPCR_SPEED_1000	(GM_GPCR_GIGS_ENA | GM_GPCR_SPEED_100)

/*                                                   */
enum {
	GM_TXCR_FORCE_JAM	= 1<<15, /*                                  */
	GM_TXCR_CRC_DIS		= 1<<14, /*                                  */
	GM_TXCR_PAD_DIS		= 1<<13, /*                                    */
	GM_TXCR_COL_THR_MSK	= 7<<10, /*                                 */
};

#define TX_COL_THR(x)		(((x)<<10) & GM_TXCR_COL_THR_MSK)
#define TX_COL_DEF		0x04

/*                                                  */
enum {
	GM_RXCR_UCF_ENA	= 1<<15, /*                                  */
	GM_RXCR_MCF_ENA	= 1<<14, /*                                    */
	GM_RXCR_CRC_DIS	= 1<<13, /*                           */
	GM_RXCR_PASS_FC	= 1<<12, /*                                 */
};

/*                                                     */
enum {
	GM_TXPA_JAMLEN_MSK	= 0x03<<14,	/*                        */
	GM_TXPA_JAMIPG_MSK	= 0x1f<<9,	/*                    */
	GM_TXPA_JAMDAT_MSK	= 0x1f<<4,	/*                            */
	GM_TXPA_BO_LIM_MSK	= 0x0f,		/*                                */

	TX_JAM_LEN_DEF		= 0x03,
	TX_JAM_IPG_DEF		= 0x0b,
	TX_IPG_JAM_DEF		= 0x1c,
	TX_BOF_LIM_DEF		= 0x04,
};

#define TX_JAM_LEN_VAL(x)	(((x)<<14) & GM_TXPA_JAMLEN_MSK)
#define TX_JAM_IPG_VAL(x)	(((x)<<9)  & GM_TXPA_JAMIPG_MSK)
#define TX_IPG_JAM_DATA(x)	(((x)<<4)  & GM_TXPA_JAMDAT_MSK)
#define TX_BACK_OFF_LIM(x)	((x) & GM_TXPA_BO_LIM_MSK)


/*                                                  */
enum {
	GM_SMOD_DATABL_MSK	= 0x1f<<11, /*                                */
	GM_SMOD_LIMIT_4		= 1<<10, /*                         */
	GM_SMOD_VLAN_ENA	= 1<<9,	 /*                               */
	GM_SMOD_JUMBO_ENA	= 1<<8,	 /*                               */

	GM_NEW_FLOW_CTRL	= 1<<6,	 /*                         */

	GM_SMOD_IPG_MSK		= 0x1f	 /*                                  */
};

#define DATA_BLIND_VAL(x)	(((x)<<11) & GM_SMOD_DATABL_MSK)
#define IPG_DATA_VAL(x)		(x & GM_SMOD_IPG_MSK)

#define DATA_BLIND_DEF		0x04
#define IPG_DATA_DEF_1000	0x1e
#define IPG_DATA_DEF_10_100	0x18

/*                                               */
enum {
	GM_SMI_CT_PHY_A_MSK	= 0x1f<<11,/*                                */
	GM_SMI_CT_REG_A_MSK	= 0x1f<<6,/*                                  */
	GM_SMI_CT_OP_RD		= 1<<5,	/*                              */
	GM_SMI_CT_RD_VAL	= 1<<4,	/*                                     */
	GM_SMI_CT_BUSY		= 1<<3,	/*                                      */
};

#define GM_SMI_CT_PHY_AD(x)	(((u16)(x)<<11) & GM_SMI_CT_PHY_A_MSK)
#define GM_SMI_CT_REG_AD(x)	(((u16)(x)<<6) & GM_SMI_CT_REG_A_MSK)

/*                                                 */
enum {
	GM_PAR_MIB_CLR	= 1<<5,	/*                                    */
	GM_PAR_MIB_TST	= 1<<4,	/*                                      */
};

/*                               */
enum {
	GMR_FS_LEN	= 0x7fff<<16, /*                             */
	GMR_FS_VLAN	= 1<<13, /*             */
	GMR_FS_JABBER	= 1<<12, /*               */
	GMR_FS_UN_SIZE	= 1<<11, /*                  */
	GMR_FS_MC	= 1<<10, /*                  */
	GMR_FS_BC	= 1<<9,  /*                  */
	GMR_FS_RX_OK	= 1<<8,  /*                          */
	GMR_FS_GOOD_FC	= 1<<7,  /*                          */
	GMR_FS_BAD_FC	= 1<<6,  /*                          */
	GMR_FS_MII_ERR	= 1<<5,  /*           */
	GMR_FS_LONG_ERR	= 1<<4,  /*                 */
	GMR_FS_FRAGMENT	= 1<<3,  /*          */

	GMR_FS_CRC_ERR	= 1<<1,  /*           */
	GMR_FS_RX_FF_OV	= 1<<0,  /*                  */

	GMR_FS_ANY_ERR	= GMR_FS_RX_FF_OV | GMR_FS_CRC_ERR |
			  GMR_FS_FRAGMENT | GMR_FS_LONG_ERR |
		  	  GMR_FS_MII_ERR | GMR_FS_BAD_FC |
			  GMR_FS_UN_SIZE | GMR_FS_JABBER,
};

/*                                                */
enum {
	RX_GCLKMAC_ENA	= 1<<31,	/*                            */
	RX_GCLKMAC_OFF	= 1<<30,

	RX_STFW_DIS	= 1<<29,	/*                             */
	RX_STFW_ENA	= 1<<28,

	RX_TRUNC_ON	= 1<<27,  	/*                           */
	RX_TRUNC_OFF	= 1<<26, 	/*                           */
	RX_VLAN_STRIP_ON = 1<<25,	/*                        */
	RX_VLAN_STRIP_OFF = 1<<24,	/*                        */

	RX_MACSEC_FLUSH_ON  = 1<<23,
	RX_MACSEC_FLUSH_OFF = 1<<22,
	RX_MACSEC_ASF_FLUSH_ON = 1<<21,
	RX_MACSEC_ASF_FLUSH_OFF = 1<<20,

	GMF_RX_OVER_ON      = 1<<19,	/*                                    */
	GMF_RX_OVER_OFF     = 1<<18,	/*                                     */
	GMF_ASF_RX_OVER_ON  = 1<<17,	/*                                     */
	GMF_ASF_RX_OVER_OFF = 1<<16,	/*                                      */

	GMF_WP_TST_ON	= 1<<14,	/*                       */
	GMF_WP_TST_OFF	= 1<<13,	/*                        */
	GMF_WP_STEP	= 1<<12,	/*                              */

	GMF_RP_TST_ON	= 1<<10,	/*                      */
	GMF_RP_TST_OFF	= 1<<9,		/*                       */
	GMF_RP_STEP	= 1<<8,		/*                             */
	GMF_RX_F_FL_ON	= 1<<7,		/*                       */
	GMF_RX_F_FL_OFF	= 1<<6,		/*                        */
	GMF_CLI_RX_FO	= 1<<5,		/*                           */
	GMF_CLI_RX_C	= 1<<4,		/*                             */

	GMF_OPER_ON	= 1<<3,		/*                     */
	GMF_OPER_OFF	= 1<<2,		/*                      */
	GMF_RST_CLR	= 1<<1,		/*                       */
	GMF_RST_SET	= 1<<0,		/*                       */

	RX_GMF_FL_THR_DEF = 0xa,	/*                           */

	GMF_RX_CTRL_DEF	= GMF_OPER_ON | GMF_RX_F_FL_ON,
};

/*                                                                  */
enum {
	RX_IPV6_SA_MOB_ENA	= 1<<9,	/*                                 */
	RX_IPV6_SA_MOB_DIS	= 1<<8,	/*                                  */
	RX_IPV6_DA_MOB_ENA	= 1<<7,	/*                                 */
	RX_IPV6_DA_MOB_DIS	= 1<<6,	/*                                  */
	RX_PTR_SYNCDLY_ENA	= 1<<5,	/*                             */
	RX_PTR_SYNCDLY_DIS	= 1<<4,	/*                              */
	RX_ASF_NEWFLAG_ENA	= 1<<3,	/*                              */
	RX_ASF_NEWFLAG_DIS	= 1<<2,	/*                               */
	RX_FLSH_MISSPKT_ENA	= 1<<1,	/*                             */
	RX_FLSH_MISSPKT_DIS	= 1<<0,	/*                              */
};

/*                                            */
enum {
	TX_DYN_WM_ENA	= 3,	/*                    */
};

/*                                                */
enum {
	TX_STFW_DIS	= 1<<31,/*                         */
	TX_STFW_ENA	= 1<<30,/*                         */

	TX_VLAN_TAG_ON	= 1<<25,/*                      */
	TX_VLAN_TAG_OFF	= 1<<24,/*                      */

	TX_PCI_JUM_ENA  = 1<<23,/*                       */
	TX_PCI_JUM_DIS  = 1<<22,/*                       */

	GMF_WSP_TST_ON	= 1<<18,/*                              */
	GMF_WSP_TST_OFF	= 1<<17,/*                               */
	GMF_WSP_STEP	= 1<<16,/*                                     */

	GMF_CLI_TX_FU	= 1<<6,	/*                            */
	GMF_CLI_TX_FC	= 1<<5,	/*                             */
	GMF_CLI_TX_PE	= 1<<4,	/*                           */
};

/*                                                               */
enum {
	GMT_ST_START	= 1<<2,	/*                        */
	GMT_ST_STOP	= 1<<1,	/*                        */
	GMT_ST_CLR_IRQ	= 1<<0,	/*                            */
};

/*                                                        */
enum {
	Y2_ASF_OS_PRES	= 1<<4,	/*                              */
	Y2_ASF_RESET	= 1<<3,	/*                           */
	Y2_ASF_RUNNING	= 1<<2,	/*                        */
	Y2_ASF_CLR_HSTI = 1<<1,	/*               */
	Y2_ASF_IRQ	= 1<<0,	/*                            */

	Y2_ASF_UC_STATE = 3<<2,	/*              */
	Y2_ASF_CLK_HALT	= 0,	/*                          */
};

/*                                                       */
enum {
	Y2_ASF_CLR_ASFI = 1<<1,	/*                */
	Y2_ASF_HOST_IRQ = 1<<0,	/*                             */
};
/*                                          */
enum {
	HCU_CCSR_SMBALERT_MONITOR= 1<<27, /*                      */
	HCU_CCSR_CPU_SLEEP	= 1<<26, /*                  */
	/*                          */
	HCU_CCSR_CS_TO		= 1<<25,
	HCU_CCSR_WDOG		= 1<<24, /*                */

	HCU_CCSR_CLR_IRQ_HOST	= 1<<17, /*                */
	HCU_CCSR_SET_IRQ_HCU	= 1<<16, /*             */

	HCU_CCSR_AHB_RST	= 1<<9, /*                  */
	HCU_CCSR_CPU_RST_MODE	= 1<<8, /*                */

	HCU_CCSR_SET_SYNC_CPU	= 1<<5,
	HCU_CCSR_CPU_CLK_DIVIDE_MSK = 3<<3,/*                  */
	HCU_CCSR_CPU_CLK_DIVIDE_BASE= 1<<3,
	HCU_CCSR_OS_PRSNT	= 1<<2, /*                */
/*                       */
	HCU_CCSR_UC_STATE_MSK	= 3,
	HCU_CCSR_UC_STATE_BASE	= 1<<0,
	HCU_CCSR_ASF_RESET	= 0,
	HCU_CCSR_ASF_HALTED	= 1<<1,
	HCU_CCSR_ASF_RUNNING	= 1<<0,
};

/*                                           */
enum {
	HCU_HCSR_SET_IRQ_CPU	= 1<<16, /*             */

	HCU_HCSR_CLR_IRQ_HCU	= 1<<1, /*               */
	HCU_HCSR_SET_IRQ_HOST	= 1<<0,	/*              */
};

/*                                                              */
enum {
	SC_STAT_CLR_IRQ	= 1<<4,	/*                        */
	SC_STAT_OP_ON	= 1<<3,	/*                     */
	SC_STAT_OP_OFF	= 1<<2,	/*                      */
	SC_STAT_RST_CLR	= 1<<1,	/*                                  */
	SC_STAT_RST_SET	= 1<<0,	/*                         */
};

/*                                                 */
enum {
	GMC_SET_RST	    = 1<<15,/*             */
	GMC_SEC_RST_OFF     = 1<<14,/*                 */
	GMC_BYP_MACSECRX_ON = 1<<13,/*                  */
	GMC_BYP_MACSECRX_OFF= 1<<12,/*                      */
	GMC_BYP_MACSECTX_ON = 1<<11,/*                  */
	GMC_BYP_MACSECTX_OFF= 1<<10,/*                      */
	GMC_BYP_RETR_ON	= 1<<9, /*                           */
	GMC_BYP_RETR_OFF= 1<<8, /*                            */

	GMC_H_BURST_ON	= 1<<7,	/*                           */
	GMC_H_BURST_OFF	= 1<<6,	/*                            */
	GMC_F_LOOPB_ON	= 1<<5,	/*                  */
	GMC_F_LOOPB_OFF	= 1<<4,	/*                   */
	GMC_PAUSE_ON	= 1<<3,	/*          */
	GMC_PAUSE_OFF	= 1<<2,	/*           */
	GMC_RST_CLR	= 1<<1,	/*                  */
	GMC_RST_SET	= 1<<0,	/*                  */
};

/*                                                 */
enum {
	GPC_TX_PAUSE	= 1<<30, /*                       */
	GPC_RX_PAUSE	= 1<<29, /*                       */
	GPC_SPEED	= 3<<27, /*                */
	GPC_LINK	= 1<<26, /*              */
	GPC_DUPLEX	= 1<<25, /*             */
	GPC_CLOCK	= 1<<24, /*                          */

	GPC_PDOWN	= 1<<23, /*                                   */
	GPC_TSTMODE	= 1<<22, /*           */
	GPC_REG18	= 1<<21, /*                  */
	GPC_REG12SEL	= 3<<19, /*                     */
	GPC_REG18SEL	= 3<<17, /*                     */
	GPC_SPILOCK	= 1<<16, /*                */

	GPC_LEDMUX	= 3<<14, /*         */
	GPC_INTPOL	= 1<<13, /*                    */
	GPC_DETECT	= 1<<12, /*               */
	GPC_1000HD	= 1<<11, /*                    */
	GPC_SLAVE	= 1<<10, /*            */
	GPC_PAUSE	= 1<<9, /*              */
	GPC_LEDCTL	= 3<<6, /*           */

	GPC_RST_CLR	= 1<<1,	/*                  */
	GPC_RST_SET	= 1<<0,	/*                  */
};

/*                                                            */
/*                                                            */
enum {
	GM_IS_TX_CO_OV	= 1<<5,	/*                               */
	GM_IS_RX_CO_OV	= 1<<4,	/*                              */
	GM_IS_TX_FF_UR	= 1<<3,	/*                        */
	GM_IS_TX_COMPL	= 1<<2,	/*                             */
	GM_IS_RX_FF_OR	= 1<<1,	/*                      */
	GM_IS_RX_COMPL	= 1<<0,	/*                          */

#define GMAC_DEF_MSK     (GM_IS_TX_FF_UR | GM_IS_RX_FF_OR)
};

/*                                                          */
enum {						/*                       */
	GMLC_RST_CLR	= 1<<1,	/*                       */
	GMLC_RST_SET	= 1<<0,	/*                       */
};


/*                                             */
enum {
	WOL_CTL_LINK_CHG_OCC		= 1<<15,
	WOL_CTL_MAGIC_PKT_OCC		= 1<<14,
	WOL_CTL_PATTERN_OCC		= 1<<13,
	WOL_CTL_CLEAR_RESULT		= 1<<12,
	WOL_CTL_ENA_PME_ON_LINK_CHG	= 1<<11,
	WOL_CTL_DIS_PME_ON_LINK_CHG	= 1<<10,
	WOL_CTL_ENA_PME_ON_MAGIC_PKT	= 1<<9,
	WOL_CTL_DIS_PME_ON_MAGIC_PKT	= 1<<8,
	WOL_CTL_ENA_PME_ON_PATTERN	= 1<<7,
	WOL_CTL_DIS_PME_ON_PATTERN	= 1<<6,
	WOL_CTL_ENA_LINK_CHG_UNIT	= 1<<5,
	WOL_CTL_DIS_LINK_CHG_UNIT	= 1<<4,
	WOL_CTL_ENA_MAGIC_PKT_UNIT	= 1<<3,
	WOL_CTL_DIS_MAGIC_PKT_UNIT	= 1<<2,
	WOL_CTL_ENA_PATTERN_UNIT	= 1<<1,
	WOL_CTL_DIS_PATTERN_UNIT	= 1<<0,
};


/*               */
enum {
	UDPTCP	= 1<<0,
	CALSUM	= 1<<1,
	WR_SUM	= 1<<2,
	INIT_SUM= 1<<3,
	LOCK_SUM= 1<<4,
	INS_VLAN= 1<<5,
	EOP	= 1<<7,
};

enum {
	HW_OWNER 	= 1<<7,
	OP_TCPWRITE	= 0x11,
	OP_TCPSTART	= 0x12,
	OP_TCPINIT	= 0x14,
	OP_TCPLCK	= 0x18,
	OP_TCPCHKSUM	= OP_TCPSTART,
	OP_TCPIS	= OP_TCPINIT | OP_TCPSTART,
	OP_TCPLW	= OP_TCPLCK | OP_TCPWRITE,
	OP_TCPLSW	= OP_TCPLCK | OP_TCPSTART | OP_TCPWRITE,
	OP_TCPLISW	= OP_TCPLCK | OP_TCPINIT | OP_TCPSTART | OP_TCPWRITE,

	OP_ADDR64	= 0x21,
	OP_VLAN		= 0x22,
	OP_ADDR64VLAN	= OP_ADDR64 | OP_VLAN,
	OP_LRGLEN	= 0x24,
	OP_LRGLENVLAN	= OP_LRGLEN | OP_VLAN,
	OP_MSS		= 0x28,
	OP_MSSVLAN	= OP_MSS | OP_VLAN,

	OP_BUFFER	= 0x40,
	OP_PACKET	= 0x41,
	OP_LARGESEND	= 0x43,
	OP_LSOV2	= 0x45,

/*                                */
	OP_RXSTAT	= 0x60,
	OP_RXTIMESTAMP	= 0x61,
	OP_RXVLAN	= 0x62,
	OP_RXCHKS	= 0x64,
	OP_RXCHKSVLAN	= OP_RXCHKS | OP_RXVLAN,
	OP_RXTIMEVLAN	= OP_RXTIMESTAMP | OP_RXVLAN,
	OP_RSS_HASH	= 0x65,
	OP_TXINDEXLE	= 0x68,
	OP_MACSEC	= 0x6c,
	OP_PUTIDX	= 0x70,
};

enum status_css {
	CSS_TCPUDPCSOK	= 1<<7,	/*                          */
	CSS_ISUDP	= 1<<6, /*                        */
	CSS_ISTCP	= 1<<5, /*                        */
	CSS_ISIPFRAG	= 1<<4, /*                                            */
	CSS_ISIPV6	= 1<<3, /*                         */
	CSS_IPV4CSUMOK	= 1<<2, /*                                  */
	CSS_ISIPV4	= 1<<1, /*                         */
	CSS_LINK_BIT	= 1<<0, /*                      */
};

/*                            */
struct sky2_tx_le {
	__le32	addr;
	__le16	length;	/*                                 */
	u8	ctrl;
	u8	opcode;
} __packed;

struct sky2_rx_le {
	__le32	addr;
	__le16	length;
	u8	ctrl;
	u8	opcode;
} __packed;

struct sky2_status_le {
	__le32	status;	/*               */
	__le16	length;	/*               */
	u8	css;
	u8	opcode;
} __packed;

struct tx_ring_info {
	struct sk_buff	*skb;
	unsigned long flags;
#define TX_MAP_SINGLE   0x0001
#define TX_MAP_PAGE     0x0002
	DEFINE_DMA_UNMAP_ADDR(mapaddr);
	DEFINE_DMA_UNMAP_LEN(maplen);
};

struct rx_ring_info {
	struct sk_buff	*skb;
	dma_addr_t	data_addr;
	DEFINE_DMA_UNMAP_LEN(data_size);
	dma_addr_t	frag_addr[ETH_JUMBO_MTU >> PAGE_SHIFT];
};

enum flow_control {
	FC_NONE	= 0,
	FC_TX	= 1,
	FC_RX	= 2,
	FC_BOTH	= 3,
};

struct sky2_stats {
	struct u64_stats_sync syncp;
	u64		packets;
	u64		bytes;
};

struct sky2_port {
	struct sky2_hw	     *hw;
	struct net_device    *netdev;
	unsigned	     port;
	u32		     msg_enable;
	spinlock_t	     phy_lock;

	struct tx_ring_info  *tx_ring;
	struct sky2_tx_le    *tx_le;
	struct sky2_stats    tx_stats;

	u16		     tx_ring_size;
	u16		     tx_cons;		/*                  */
	u16		     tx_prod;		/*                */
	u16		     tx_next;		/*            */

	u16		     tx_pending;
	u16		     tx_last_mss;
	u32		     tx_last_upper;
	u32		     tx_tcpsum;

	struct rx_ring_info  *rx_ring ____cacheline_aligned_in_smp;
	struct sky2_rx_le    *rx_le;
	struct sky2_stats    rx_stats;

	u16		     rx_next;		/*                  */
	u16		     rx_put;		/*                      */
	u16		     rx_pending;
	u16		     rx_data_size;
	u16		     rx_nfrags;

	struct {
		unsigned long last;
		u32	mac_rp;
		u8	mac_lev;
		u8	fifo_rp;
		u8	fifo_lev;
	} check;

	dma_addr_t	     rx_le_map;
	dma_addr_t	     tx_le_map;

	u16		     advertising;	/*                  */
	u16		     speed;		/*                            */
	u8		     wol;		/*            */
	u8		     duplex;		/*                          */
	u16		     flags;
#define SKY2_FLAG_AUTO_SPEED		0x0002
#define SKY2_FLAG_AUTO_PAUSE		0x0004

 	enum flow_control    flow_mode;
 	enum flow_control    flow_status;

#ifdef CONFIG_SKY2_DEBUG
	struct dentry	     *debugfs;
#endif
};

struct sky2_hw {
	void __iomem  	     *regs;
	struct pci_dev	     *pdev;
	struct napi_struct   napi;
	struct net_device    *dev[2];
	unsigned long	     flags;
#define SKY2_HW_USE_MSI		0x00000001
#define SKY2_HW_FIBRE_PHY	0x00000002
#define SKY2_HW_GIGABIT		0x00000004
#define SKY2_HW_NEWER_PHY	0x00000008
#define SKY2_HW_RAM_BUFFER	0x00000010
#define SKY2_HW_NEW_LE		0x00000020	/*                  */
#define SKY2_HW_AUTO_TX_SUM	0x00000040	/*                      */
#define SKY2_HW_ADV_POWER_CTL	0x00000080	/*                           */
#define SKY2_HW_RSS_BROKEN	0x00000100
#define SKY2_HW_VLAN_BROKEN     0x00000200
#define SKY2_HW_RSS_CHKSUM	0x00000400	/*                     */
#define SKY2_HW_IRQ_SETUP	0x00000800

	u8	     	     chip_id;
	u8		     chip_rev;
	u8		     pmd_type;
	u8		     ports;

	struct sky2_status_le *st_le;
	u32		     st_size;
	u32		     st_idx;
	dma_addr_t   	     st_dma;

	struct timer_list    watchdog_timer;
	struct work_struct   restart_work;
	wait_queue_head_t    msi_wait;

	char		     irq_name[0];
};

static inline int sky2_is_copper(const struct sky2_hw *hw)
{
	return !(hw->flags & SKY2_HW_FIBRE_PHY);
}

/*                                            */
static inline u32 sky2_read32(const struct sky2_hw *hw, unsigned reg)
{
	return readl(hw->regs + reg);
}

static inline u16 sky2_read16(const struct sky2_hw *hw, unsigned reg)
{
	return readw(hw->regs + reg);
}

static inline u8 sky2_read8(const struct sky2_hw *hw, unsigned reg)
{
	return readb(hw->regs + reg);
}

static inline void sky2_write32(const struct sky2_hw *hw, unsigned reg, u32 val)
{
	writel(val, hw->regs + reg);
}

static inline void sky2_write16(const struct sky2_hw *hw, unsigned reg, u16 val)
{
	writew(val, hw->regs + reg);
}

static inline void sky2_write8(const struct sky2_hw *hw, unsigned reg, u8 val)
{
	writeb(val, hw->regs + reg);
}

/*                             */
#define SK_GMAC_REG(port,reg) \
	(BASE_GMAC_1 + (port) * (BASE_GMAC_2-BASE_GMAC_1) + (reg))
#define GM_PHY_RETRIES	100

static inline u16 gma_read16(const struct sky2_hw *hw, unsigned port, unsigned reg)
{
	return sky2_read16(hw, SK_GMAC_REG(port,reg));
}

static inline u32 gma_read32(struct sky2_hw *hw, unsigned port, unsigned reg)
{
	unsigned base = SK_GMAC_REG(port, reg);
	return (u32) sky2_read16(hw, base)
		| (u32) sky2_read16(hw, base+4) << 16;
}

static inline u64 gma_read64(struct sky2_hw *hw, unsigned port, unsigned reg)
{
	unsigned base = SK_GMAC_REG(port, reg);

	return (u64) sky2_read16(hw, base)
		| (u64) sky2_read16(hw, base+4) << 16
		| (u64) sky2_read16(hw, base+8) << 32
		| (u64) sky2_read16(hw, base+12) << 48;
}

/*                                                                    */
static inline u32 get_stats32(struct sky2_hw *hw, unsigned port, unsigned reg)
{
	u32 val;

	do {
		val = gma_read32(hw, port, reg);
	} while (gma_read32(hw, port, reg) != val);

	return val;
}

static inline u64 get_stats64(struct sky2_hw *hw, unsigned port, unsigned reg)
{
	u64 val;

	do {
		val = gma_read64(hw, port, reg);
	} while (gma_read64(hw, port, reg) != val);

	return val;
}

static inline void gma_write16(const struct sky2_hw *hw, unsigned port, int r, u16 v)
{
	sky2_write16(hw, SK_GMAC_REG(port,r), v);
}

static inline void gma_set_addr(struct sky2_hw *hw, unsigned port, unsigned reg,
				    const u8 *addr)
{
	gma_write16(hw, port, reg,  (u16) addr[0] | ((u16) addr[1] << 8));
	gma_write16(hw, port, reg+4,(u16) addr[2] | ((u16) addr[3] << 8));
	gma_write16(hw, port, reg+8,(u16) addr[4] | ((u16) addr[5] << 8));
}

/*                         */
static inline u32 sky2_pci_read32(const struct sky2_hw *hw, unsigned reg)
{
	return sky2_read32(hw, Y2_CFG_SPC + reg);
}

static inline u16 sky2_pci_read16(const struct sky2_hw *hw, unsigned reg)
{
	return sky2_read16(hw, Y2_CFG_SPC + reg);
}

static inline void sky2_pci_write32(struct sky2_hw *hw, unsigned reg, u32 val)
{
	sky2_write32(hw, Y2_CFG_SPC + reg, val);
}

static inline void sky2_pci_write16(struct sky2_hw *hw, unsigned reg, u16 val)
{
	sky2_write16(hw, Y2_CFG_SPC + reg, val);
}
#endif
