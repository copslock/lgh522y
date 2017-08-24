#ifndef __ASM_SH7757_H__
#define __ASM_SH7757_H__

enum {
	/*     */
	GPIO_PTA0, GPIO_PTA1, GPIO_PTA2, GPIO_PTA3,
	GPIO_PTA4, GPIO_PTA5, GPIO_PTA6, GPIO_PTA7,

	/*     */
	GPIO_PTB0, GPIO_PTB1, GPIO_PTB2, GPIO_PTB3,
	GPIO_PTB4, GPIO_PTB5, GPIO_PTB6, GPIO_PTB7,

	/*     */
	GPIO_PTC0, GPIO_PTC1, GPIO_PTC2, GPIO_PTC3,
	GPIO_PTC4, GPIO_PTC5, GPIO_PTC6, GPIO_PTC7,

	/*     */
	GPIO_PTD0, GPIO_PTD1, GPIO_PTD2, GPIO_PTD3,
	GPIO_PTD4, GPIO_PTD5, GPIO_PTD6, GPIO_PTD7,

	/*     */
	GPIO_PTE0, GPIO_PTE1, GPIO_PTE2, GPIO_PTE3,
	GPIO_PTE4, GPIO_PTE5, GPIO_PTE6, GPIO_PTE7,

	/*     */
	GPIO_PTF0, GPIO_PTF1, GPIO_PTF2, GPIO_PTF3,
	GPIO_PTF4, GPIO_PTF5, GPIO_PTF6, GPIO_PTF7,

	/*     */
	GPIO_PTG0, GPIO_PTG1, GPIO_PTG2, GPIO_PTG3,
	GPIO_PTG4, GPIO_PTG5, GPIO_PTG6, GPIO_PTG7,

	/*     */
	GPIO_PTH0, GPIO_PTH1, GPIO_PTH2, GPIO_PTH3,
	GPIO_PTH4, GPIO_PTH5, GPIO_PTH6, GPIO_PTH7,

	/*     */
	GPIO_PTI0, GPIO_PTI1, GPIO_PTI2, GPIO_PTI3,
	GPIO_PTI4, GPIO_PTI5, GPIO_PTI6, GPIO_PTI7,

	/*     */
	GPIO_PTJ0, GPIO_PTJ1, GPIO_PTJ2, GPIO_PTJ3,
	GPIO_PTJ4, GPIO_PTJ5, GPIO_PTJ6, GPIO_PTJ7_RESV,

	/*     */
	GPIO_PTK0, GPIO_PTK1, GPIO_PTK2, GPIO_PTK3,
	GPIO_PTK4, GPIO_PTK5, GPIO_PTK6, GPIO_PTK7,

	/*     */
	GPIO_PTL0, GPIO_PTL1, GPIO_PTL2, GPIO_PTL3,
	GPIO_PTL4, GPIO_PTL5, GPIO_PTL6, GPIO_PTL7_RESV,

	/*     */
	GPIO_PTM0, GPIO_PTM1, GPIO_PTM2, GPIO_PTM3,
	GPIO_PTM4, GPIO_PTM5, GPIO_PTM6, GPIO_PTM7,

	/*     */
	GPIO_PTN0, GPIO_PTN1, GPIO_PTN2, GPIO_PTN3,
	GPIO_PTN4, GPIO_PTN5, GPIO_PTN6, GPIO_PTN7_RESV,

	/*     */
	GPIO_PTO0, GPIO_PTO1, GPIO_PTO2, GPIO_PTO3,
	GPIO_PTO4, GPIO_PTO5, GPIO_PTO6, GPIO_PTO7,

	/*     */
	GPIO_PTP0, GPIO_PTP1, GPIO_PTP2, GPIO_PTP3,
	GPIO_PTP4, GPIO_PTP5, GPIO_PTP6, GPIO_PTP7,

	/*     */
	GPIO_PTQ0, GPIO_PTQ1, GPIO_PTQ2, GPIO_PTQ3,
	GPIO_PTQ4, GPIO_PTQ5, GPIO_PTQ6, GPIO_PTQ7_RESV,

	/*     */
	GPIO_PTR0, GPIO_PTR1, GPIO_PTR2, GPIO_PTR3,
	GPIO_PTR4, GPIO_PTR5, GPIO_PTR6, GPIO_PTR7,

	/*     */
	GPIO_PTS0, GPIO_PTS1, GPIO_PTS2, GPIO_PTS3,
	GPIO_PTS4, GPIO_PTS5, GPIO_PTS6, GPIO_PTS7,

	/*     */
	GPIO_PTT0, GPIO_PTT1, GPIO_PTT2, GPIO_PTT3,
	GPIO_PTT4, GPIO_PTT5, GPIO_PTT6, GPIO_PTT7,

	/*     */
	GPIO_PTU0, GPIO_PTU1, GPIO_PTU2, GPIO_PTU3,
	GPIO_PTU4, GPIO_PTU5, GPIO_PTU6, GPIO_PTU7,

	/*     */
	GPIO_PTV0, GPIO_PTV1, GPIO_PTV2, GPIO_PTV3,
	GPIO_PTV4, GPIO_PTV5, GPIO_PTV6, GPIO_PTV7,

	/*     */
	GPIO_PTW0, GPIO_PTW1, GPIO_PTW2, GPIO_PTW3,
	GPIO_PTW4, GPIO_PTW5, GPIO_PTW6, GPIO_PTW7,

	/*     */
	GPIO_PTX0, GPIO_PTX1, GPIO_PTX2, GPIO_PTX3,
	GPIO_PTX4, GPIO_PTX5, GPIO_PTX6, GPIO_PTX7,

	/*     */
	GPIO_PTY0, GPIO_PTY1, GPIO_PTY2, GPIO_PTY3,
	GPIO_PTY4, GPIO_PTY5, GPIO_PTY6, GPIO_PTY7,

	/*     */
	GPIO_PTZ0, GPIO_PTZ1, GPIO_PTZ2, GPIO_PTZ3,
	GPIO_PTZ4, GPIO_PTZ5, GPIO_PTZ6, GPIO_PTZ7,


	/*                           */
	GPIO_FN_BS,	GPIO_FN_RDWR,	GPIO_FN_WE1,	GPIO_FN_RDY,
	GPIO_FN_ET0_MDC,	GPIO_FN_ET0_MDIO,
	GPIO_FN_ET1_MDC,	GPIO_FN_ET1_MDIO,

	/*                               */
	GPIO_FN_IRQ15,	GPIO_FN_IRQ14,	GPIO_FN_IRQ13,	GPIO_FN_IRQ12,
	GPIO_FN_IRQ11,	GPIO_FN_IRQ10,	GPIO_FN_IRQ9,	GPIO_FN_IRQ8,
	GPIO_FN_ON_NRE,	GPIO_FN_ON_NWE,	GPIO_FN_ON_NWP,	GPIO_FN_ON_NCE0,
	GPIO_FN_ON_R_B0,	GPIO_FN_ON_ALE,	GPIO_FN_ON_CLE,
	GPIO_FN_TCLK,

	/*                         */
	GPIO_FN_IRQ7,	GPIO_FN_IRQ6,	GPIO_FN_IRQ5,	GPIO_FN_IRQ4,
	GPIO_FN_IRQ3,	GPIO_FN_IRQ2,	GPIO_FN_IRQ1,	GPIO_FN_IRQ0,
	GPIO_FN_PWMU0,	GPIO_FN_PWMU1,	GPIO_FN_PWMU2,	GPIO_FN_PWMU3,
	GPIO_FN_PWMU4,	GPIO_FN_PWMU5,

	/*                          */
	GPIO_FN_SP0_MOSI,	GPIO_FN_SP0_MISO,	GPIO_FN_SP0_SCK,
	GPIO_FN_SP0_SCK_FB,	GPIO_FN_SP0_SS0,	GPIO_FN_SP0_SS1,
	GPIO_FN_SP0_SS2,	GPIO_FN_SP0_SS3,	GPIO_FN_DREQ0,
	GPIO_FN_DACK0,		GPIO_FN_TEND0,

	/*                    */
	GPIO_FN_RMII0_CRS_DV,	GPIO_FN_RMII0_TXD1,	GPIO_FN_RMII0_TXD0,
	GPIO_FN_RMII0_TXEN,	GPIO_FN_RMII0_REFCLK,	GPIO_FN_RMII0_RXD1,
	GPIO_FN_RMII0_RXD0,	GPIO_FN_RMII0_RX_ER,

	/*                            */
	GPIO_FN_RMII1_CRS_DV,	GPIO_FN_RMII1_TXD1,	GPIO_FN_RMII1_TXD0,
	GPIO_FN_RMII1_TXEN,	GPIO_FN_RMII1_REFCLK,	GPIO_FN_RMII1_RXD1,
	GPIO_FN_RMII1_RXD0,	GPIO_FN_RMII1_RX_ER,	GPIO_FN_RAC_RI,

	/*                                                 */
	GPIO_FN_BOOTFMS,		GPIO_FN_BOOTWP,
	GPIO_FN_A25,	GPIO_FN_A24,	GPIO_FN_SERIRQ,	GPIO_FN_WDTOVF,
	GPIO_FN_LPCPD,	GPIO_FN_LDRQ,	GPIO_FN_MMCCLK,	GPIO_FN_MMCCMD,

	/*                                    */
	GPIO_FN_SP1_MOSI,		GPIO_FN_SP1_MISO,
	GPIO_FN_SP1_SCK,		GPIO_FN_SP1_SCK_FB,
	GPIO_FN_SP1_SS0,		GPIO_FN_SP1_SS1,
	GPIO_FN_WP,	GPIO_FN_FMS0,	GPIO_FN_TEND1,	GPIO_FN_DREQ1,
	GPIO_FN_DACK1,	GPIO_FN_ADTRG1,	GPIO_FN_ADTRG0,

	/*                          */
	GPIO_FN_D15,	GPIO_FN_D14,	GPIO_FN_D13,	GPIO_FN_D12,
	GPIO_FN_D11,	GPIO_FN_D10,	GPIO_FN_D9,	GPIO_FN_D8,
	GPIO_FN_SD_WP,	GPIO_FN_SD_CD,	GPIO_FN_SD_CLK,	GPIO_FN_SD_CMD,
	GPIO_FN_SD_D3,	GPIO_FN_SD_D2,	GPIO_FN_SD_D1,	GPIO_FN_SD_D0,

	/*                       */
	GPIO_FN_RTS3,	GPIO_FN_CTS3,	GPIO_FN_TXD3,	GPIO_FN_RXD3,
	GPIO_FN_RTS4,	GPIO_FN_RXD4,	GPIO_FN_TXD4,

	/*                                  */
	GPIO_FN_COM2_TXD,	GPIO_FN_COM2_RXD,	GPIO_FN_COM2_RTS,
	GPIO_FN_COM2_CTS,	GPIO_FN_COM2_DTR,	GPIO_FN_COM2_DSR,
	GPIO_FN_COM2_DCD,	GPIO_FN_CLKOUT,
	GPIO_FN_SCK2,		GPIO_FN_SCK4,	GPIO_FN_SCK3,

	/*                                       */
	GPIO_FN_RAC_RXD,	GPIO_FN_RAC_RTS,	GPIO_FN_RAC_CTS,
	GPIO_FN_RAC_DTR,	GPIO_FN_RAC_DSR,	GPIO_FN_RAC_DCD,
	GPIO_FN_RAC_TXD,	GPIO_FN_RXD2,		GPIO_FN_CS5,
	GPIO_FN_CS6,		GPIO_FN_AUDSYNC,	GPIO_FN_AUDCK,
	GPIO_FN_TXD2,

	/*                         */
	GPIO_FN_CS4,	GPIO_FN_RD,	GPIO_FN_WE0,	GPIO_FN_CS0,
	GPIO_FN_SDA6,	GPIO_FN_SCL6,	GPIO_FN_SDA7,	GPIO_FN_SCL7,

	/*                                    */
	GPIO_FN_VBUS_EN,	GPIO_FN_VBUS_OC,	GPIO_FN_JMCTCK,
	GPIO_FN_JMCTMS,		GPIO_FN_JMCTDO,		GPIO_FN_JMCTDI,
	GPIO_FN_JMCTRST,
	GPIO_FN_SGPIO1_CLK,	GPIO_FN_SGPIO1_LOAD,	GPIO_FN_SGPIO1_DI,
	GPIO_FN_SGPIO1_DO,	GPIO_FN_SUB_CLKIN,

	/*                             */
	GPIO_FN_SGPIO0_CLK,	GPIO_FN_SGPIO0_LOAD,	GPIO_FN_SGPIO0_DI,
	GPIO_FN_SGPIO0_DO,	GPIO_FN_SGPIO2_CLK,	GPIO_FN_SGPIO2_LOAD,
	GPIO_FN_SGPIO2_DI,	GPIO_FN_SGPIO2_DO,	GPIO_FN_COM1_TXD,
	GPIO_FN_COM1_RXD,	GPIO_FN_COM1_RTS,	GPIO_FN_COM1_CTS,

	/*                   */
	GPIO_FN_LAD3,	GPIO_FN_LAD2,	GPIO_FN_LAD1,	GPIO_FN_LAD0,
	GPIO_FN_LFRAME,	GPIO_FN_LRESET,	GPIO_FN_LCLK,

	/*                        */
	GPIO_FN_DDC3,	GPIO_FN_DDC2,	GPIO_FN_SDA2,	GPIO_FN_SCL2,
	GPIO_FN_SDA1,	GPIO_FN_SCL1,	GPIO_FN_SDA0,	GPIO_FN_SCL0,
	GPIO_FN_SDA8,	GPIO_FN_SCL8,

	/*                        */
	GPIO_FN_DDC1,	GPIO_FN_DDC0,	GPIO_FN_SDA5,	GPIO_FN_SCL5,
	GPIO_FN_SDA4,	GPIO_FN_SCL4,	GPIO_FN_SDA3,	GPIO_FN_SCL3,
	GPIO_FN_SDA9,	GPIO_FN_SCL9,

	/*                         */
	GPIO_FN_PWMX7,	GPIO_FN_PWMX6,	GPIO_FN_PWMX5,	GPIO_FN_PWMX4,
	GPIO_FN_PWMX3,	GPIO_FN_PWMX2,	GPIO_FN_PWMX1,	GPIO_FN_PWMX0,
	GPIO_FN_AUDATA3,	GPIO_FN_AUDATA2,	GPIO_FN_AUDATA1,
	GPIO_FN_AUDATA0,	GPIO_FN_STATUS1,	GPIO_FN_STATUS0,

	/*                        */
	GPIO_FN_LGPIO7,	GPIO_FN_LGPIO6,	GPIO_FN_LGPIO5,	GPIO_FN_LGPIO4,
	GPIO_FN_LGPIO3,	GPIO_FN_LGPIO2,	GPIO_FN_LGPIO1,	GPIO_FN_LGPIO0,
	GPIO_FN_APMONCTL_O,	GPIO_FN_APMPWBTOUT_O,	GPIO_FN_APMSCI_O,
	GPIO_FN_APMVDDON,	GPIO_FN_APMSLPBTN,	GPIO_FN_APMPWRBTN,
	GPIO_FN_APMS5N,		GPIO_FN_APMS3N,

	/*                                             */
	GPIO_FN_A23,	GPIO_FN_A22,	GPIO_FN_A21,	GPIO_FN_A20,
	GPIO_FN_A19,	GPIO_FN_A18,	GPIO_FN_A17,	GPIO_FN_A16,
	GPIO_FN_COM2_RI,	GPIO_FN_R_SPI_MOSI,	GPIO_FN_R_SPI_MISO,
	GPIO_FN_R_SPI_RSPCK,	GPIO_FN_R_SPI_SSL0,	GPIO_FN_R_SPI_SSL1,
	GPIO_FN_EVENT7,		GPIO_FN_EVENT6,		GPIO_FN_VBIOS_DI,
	GPIO_FN_VBIOS_DO,	GPIO_FN_VBIOS_CLK,	GPIO_FN_VBIOS_CS,

	/*                               */
	GPIO_FN_A15,	GPIO_FN_A14,	GPIO_FN_A13,	GPIO_FN_A12,
	GPIO_FN_A11,	GPIO_FN_A10,	GPIO_FN_A9,	GPIO_FN_A8,
	GPIO_FN_EVENT5,	GPIO_FN_EVENT4,	GPIO_FN_EVENT3,	GPIO_FN_EVENT2,
	GPIO_FN_EVENT1,	GPIO_FN_EVENT0,	GPIO_FN_CTS4,	GPIO_FN_CTS2,

	/*                               */
	GPIO_FN_A7,	GPIO_FN_A6,	GPIO_FN_A5,	GPIO_FN_A4,
	GPIO_FN_A3,	GPIO_FN_A2,	GPIO_FN_A1,	GPIO_FN_A0,
	GPIO_FN_RTS2,	GPIO_FN_SIM_D,	GPIO_FN_SIM_CLK, GPIO_FN_SIM_RST,

	/*                    */
	GPIO_FN_D7,	GPIO_FN_D6,	GPIO_FN_D5,	GPIO_FN_D4,
	GPIO_FN_D3,	GPIO_FN_D2,	GPIO_FN_D1,	GPIO_FN_D0,

	/*                          */
	GPIO_FN_MMCDAT7,	GPIO_FN_MMCDAT6,	GPIO_FN_MMCDAT5,
	GPIO_FN_MMCDAT4,	GPIO_FN_MMCDAT3,	GPIO_FN_MMCDAT2,
	GPIO_FN_MMCDAT1,	GPIO_FN_MMCDAT0,
	GPIO_FN_ON_DQ7,	GPIO_FN_ON_DQ6,	GPIO_FN_ON_DQ5,	GPIO_FN_ON_DQ4,
	GPIO_FN_ON_DQ3,	GPIO_FN_ON_DQ2,	GPIO_FN_ON_DQ1,	GPIO_FN_ON_DQ0,
};

enum {
	SHDMA_SLAVE_INVALID,
	SHDMA_SLAVE_SDHI_TX,
	SHDMA_SLAVE_SDHI_RX,
	SHDMA_SLAVE_MMCIF_TX,
	SHDMA_SLAVE_MMCIF_RX,
	SHDMA_SLAVE_SCIF2_TX,
	SHDMA_SLAVE_SCIF2_RX,
	SHDMA_SLAVE_SCIF3_TX,
	SHDMA_SLAVE_SCIF3_RX,
	SHDMA_SLAVE_SCIF4_TX,
	SHDMA_SLAVE_SCIF4_RX,
	SHDMA_SLAVE_RIIC0_TX,
	SHDMA_SLAVE_RIIC0_RX,
	SHDMA_SLAVE_RIIC1_TX,
	SHDMA_SLAVE_RIIC1_RX,
	SHDMA_SLAVE_RIIC2_TX,
	SHDMA_SLAVE_RIIC2_RX,
	SHDMA_SLAVE_RIIC3_TX,
	SHDMA_SLAVE_RIIC3_RX,
	SHDMA_SLAVE_RIIC4_TX,
	SHDMA_SLAVE_RIIC4_RX,
	SHDMA_SLAVE_RIIC5_TX,
	SHDMA_SLAVE_RIIC5_RX,
	SHDMA_SLAVE_RIIC6_TX,
	SHDMA_SLAVE_RIIC6_RX,
	SHDMA_SLAVE_RIIC7_TX,
	SHDMA_SLAVE_RIIC7_RX,
	SHDMA_SLAVE_RIIC8_TX,
	SHDMA_SLAVE_RIIC8_RX,
	SHDMA_SLAVE_RIIC9_TX,
	SHDMA_SLAVE_RIIC9_RX,
	SHDMA_SLAVE_RSPI_TX,
	SHDMA_SLAVE_RSPI_RX,
};
#endif /*                  */
