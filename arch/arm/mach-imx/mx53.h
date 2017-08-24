#ifndef __MACH_MX53_H__
#define __MACH_MX53_H__

/*
       
 */
#define MX53_IROM_BASE_ADDR		0x0
#define MX53_IROM_SIZE			SZ_64K

/*      */
#define MX53_TZIC_BASE_ADDR		0x0FFFC000
#define MX53_TZIC_SIZE			SZ_16K

/*
            
 */
#define MX53_SATA_BASE_ADDR		0x10000000

/*
      
 */
#define MX53_NFC_AXI_BASE_ADDR	0xF7FF0000	/*                */
#define MX53_NFC_AXI_SIZE		SZ_64K

/*
       
 */
#define MX53_IRAM_BASE_ADDR	0xF8000000	/*              */
#define MX53_IRAM_PARTITIONS	16
#define MX53_IRAM_SIZE		(MX53_IRAM_PARTITIONS * SZ_8K)	/*       */

/*
                         
 */
#define MX53_IPU_CTRL_BASE_ADDR	0x18000000
#define MX53_GPU2D_BASE_ADDR		0x20000000
#define MX53_GPU_BASE_ADDR		0x30000000
#define MX53_GPU_GMEM_BASE_ADDR	0xF8020000

#define MX53_DEBUG_BASE_ADDR		0x40000000
#define MX53_DEBUG_SIZE		SZ_1M
#define MX53_ETB_BASE_ADDR		(MX53_DEBUG_BASE_ADDR + 0x00001000)
#define MX53_ETM_BASE_ADDR		(MX53_DEBUG_BASE_ADDR + 0x00002000)
#define MX53_TPIU_BASE_ADDR		(MX53_DEBUG_BASE_ADDR + 0x00003000)
#define MX53_CTI0_BASE_ADDR		(MX53_DEBUG_BASE_ADDR + 0x00004000)
#define MX53_CTI1_BASE_ADDR		(MX53_DEBUG_BASE_ADDR + 0x00005000)
#define MX53_CTI2_BASE_ADDR		(MX53_DEBUG_BASE_ADDR + 0x00006000)
#define MX53_CTI3_BASE_ADDR		(MX53_DEBUG_BASE_ADDR + 0x00007000)
#define MX53_CORTEX_DBG_BASE_ADDR	(MX53_DEBUG_BASE_ADDR + 0x00008000)

/*
                                
 */
#define MX53_SPBA0_BASE_ADDR		0x50000000
#define MX53_SPBA0_SIZE		SZ_1M

#define MX53_ESDHC1_BASE_ADDR	(MX53_SPBA0_BASE_ADDR + 0x00004000)
#define MX53_ESDHC2_BASE_ADDR	(MX53_SPBA0_BASE_ADDR + 0x00008000)
#define MX53_UART3_BASE_ADDR		(MX53_SPBA0_BASE_ADDR + 0x0000C000)
#define MX53_ECSPI1_BASE_ADDR		(MX53_SPBA0_BASE_ADDR + 0x00010000)
#define MX53_SSI2_BASE_ADDR		(MX53_SPBA0_BASE_ADDR + 0x00014000)
#define MX53_ESDHC3_BASE_ADDR	(MX53_SPBA0_BASE_ADDR + 0x00020000)
#define MX53_ESDHC4_BASE_ADDR	(MX53_SPBA0_BASE_ADDR + 0x00024000)
#define MX53_SPDIF_BASE_ADDR		(MX53_SPBA0_BASE_ADDR + 0x00028000)
#define MX53_ASRC_BASE_ADDR		(MX53_SPBA0_BASE_ADDR + 0x0002C000)
#define MX53_ATA_DMA_BASE_ADDR	(MX53_SPBA0_BASE_ADDR + 0x00030000)
#define MX53_SLIM_DMA_BASE_ADDR	(MX53_SPBA0_BASE_ADDR + 0x00034000)
#define MX53_HSI2C_DMA_BASE_ADDR	(MX53_SPBA0_BASE_ADDR + 0x00038000)
#define MX53_SPBA_CTRL_BASE_ADDR	(MX53_SPBA0_BASE_ADDR + 0x0003C000)

/*
         
 */
#define MX53_AIPS1_BASE_ADDR	0x53F00000
#define MX53_AIPS1_SIZE		SZ_1M

#define MX53_OTG_BASE_ADDR	(MX53_AIPS1_BASE_ADDR + 0x00080000)
#define MX53_GPIO1_BASE_ADDR	(MX53_AIPS1_BASE_ADDR + 0x00084000)
#define MX53_GPIO2_BASE_ADDR	(MX53_AIPS1_BASE_ADDR + 0x00088000)
#define MX53_GPIO3_BASE_ADDR	(MX53_AIPS1_BASE_ADDR + 0x0008C000)
#define MX53_GPIO4_BASE_ADDR	(MX53_AIPS1_BASE_ADDR + 0x00090000)
#define MX53_KPP_BASE_ADDR	(MX53_AIPS1_BASE_ADDR + 0x00094000)
#define MX53_WDOG1_BASE_ADDR	(MX53_AIPS1_BASE_ADDR + 0x00098000)
#define MX53_WDOG2_BASE_ADDR	(MX53_AIPS1_BASE_ADDR + 0x0009C000)
#define MX53_GPT1_BASE_ADDR	(MX53_AIPS1_BASE_ADDR + 0x000A0000)
#define MX53_SRTC_BASE_ADDR	(MX53_AIPS1_BASE_ADDR + 0x000A4000)
#define MX53_IOMUXC_BASE_ADDR	(MX53_AIPS1_BASE_ADDR + 0x000A8000)
#define MX53_EPIT1_BASE_ADDR	(MX53_AIPS1_BASE_ADDR + 0x000AC000)
#define MX53_EPIT2_BASE_ADDR	(MX53_AIPS1_BASE_ADDR + 0x000B0000)
#define MX53_PWM1_BASE_ADDR	(MX53_AIPS1_BASE_ADDR + 0x000B4000)
#define MX53_PWM2_BASE_ADDR	(MX53_AIPS1_BASE_ADDR + 0x000B8000)
#define MX53_UART1_BASE_ADDR	(MX53_AIPS1_BASE_ADDR + 0x000BC000)
#define MX53_UART2_BASE_ADDR	(MX53_AIPS1_BASE_ADDR + 0x000C0000)
#define MX53_SRC_BASE_ADDR	(MX53_AIPS1_BASE_ADDR + 0x000D0000)
#define MX53_CCM_BASE_ADDR	(MX53_AIPS1_BASE_ADDR + 0x000D4000)
#define MX53_GPC_BASE_ADDR	(MX53_AIPS1_BASE_ADDR + 0x000D8000)
#define MX53_GPIO5_BASE_ADDR	(MX53_AIPS1_BASE_ADDR + 0x000DC000)
#define MX53_GPIO6_BASE_ADDR	(MX53_AIPS1_BASE_ADDR + 0x000E0000)
#define MX53_GPIO7_BASE_ADDR	(MX53_AIPS1_BASE_ADDR + 0x000E4000)
#define MX53_ATA_BASE_ADDR	(MX53_AIPS1_BASE_ADDR + 0x000E8000)
#define MX53_I2C3_BASE_ADDR	(MX53_AIPS1_BASE_ADDR + 0x000EC000)
#define MX53_UART4_BASE_ADDR	(MX53_AIPS1_BASE_ADDR + 0x000F0000)

/*
         
 */
#define MX53_AIPS2_BASE_ADDR		0x63F00000
#define MX53_AIPS2_SIZE			SZ_1M

#define MX53_PLL1_BASE_ADDR	(MX53_AIPS2_BASE_ADDR + 0x00080000)
#define MX53_PLL2_BASE_ADDR	(MX53_AIPS2_BASE_ADDR + 0x00084000)
#define MX53_PLL3_BASE_ADDR	(MX53_AIPS2_BASE_ADDR + 0x00088000)
#define MX53_PLL4_BASE_ADDR	(MX53_AIPS2_BASE_ADDR + 0x0008C000)
#define MX53_UART5_BASE_ADDR	(MX53_AIPS2_BASE_ADDR + 0x00090000)
#define MX53_AHBMAX_BASE_ADDR	(MX53_AIPS2_BASE_ADDR + 0x00094000)
#define MX53_IIM_BASE_ADDR	(MX53_AIPS2_BASE_ADDR + 0x00098000)
#define MX53_CSU_BASE_ADDR	(MX53_AIPS2_BASE_ADDR + 0x0009C000)
#define MX53_ARM_BASE_ADDR	(MX53_AIPS2_BASE_ADDR + 0x000A0000)
#define MX53_OWIRE_BASE_ADDR	(MX53_AIPS2_BASE_ADDR + 0x000A4000)
#define MX53_FIRI_BASE_ADDR	(MX53_AIPS2_BASE_ADDR + 0x000A8000)
#define MX53_ECSPI2_BASE_ADDR	(MX53_AIPS2_BASE_ADDR + 0x000AC000)
#define MX53_SDMA_BASE_ADDR	(MX53_AIPS2_BASE_ADDR + 0x000B0000)
#define MX53_SCC_BASE_ADDR	(MX53_AIPS2_BASE_ADDR + 0x000B4000)
#define MX53_ROMCP_BASE_ADDR	(MX53_AIPS2_BASE_ADDR + 0x000B8000)
#define MX53_RTIC_BASE_ADDR	(MX53_AIPS2_BASE_ADDR + 0x000BC000)
#define MX53_CSPI_BASE_ADDR	(MX53_AIPS2_BASE_ADDR + 0x000C0000)
#define MX53_I2C2_BASE_ADDR	(MX53_AIPS2_BASE_ADDR + 0x000C4000)
#define MX53_I2C1_BASE_ADDR	(MX53_AIPS2_BASE_ADDR + 0x000C8000)
#define MX53_SSI1_BASE_ADDR	(MX53_AIPS2_BASE_ADDR + 0x000CC000)
#define MX53_AUDMUX_BASE_ADDR	(MX53_AIPS2_BASE_ADDR + 0x000D0000)
#define MX53_RTC_BASE_ADDR	(MX53_AIPS2_BASE_ADDR + 0x000D4000)
#define MX53_M4IF_BASE_ADDR	(MX53_AIPS2_BASE_ADDR + 0x000D8000)
#define MX53_ESDCTL_BASE_ADDR	(MX53_AIPS2_BASE_ADDR + 0x000D9000)
#define MX53_WEIM_BASE_ADDR	(MX53_AIPS2_BASE_ADDR + 0x000DA000)
#define MX53_NFC_BASE_ADDR	(MX53_AIPS2_BASE_ADDR + 0x000DB000)
#define MX53_EMI_BASE_ADDR	(MX53_AIPS2_BASE_ADDR + 0x000DBF00)
#define MX53_MIPI_HSC_BASE_ADDR	(MX53_AIPS2_BASE_ADDR + 0x000DC000)
#define MX53_MLB_BASE_ADDR	(MX53_AIPS2_BASE_ADDR + 0x000E4000)
#define MX53_SSI3_BASE_ADDR	(MX53_AIPS2_BASE_ADDR + 0x000E8000)
#define MX53_FEC_BASE_ADDR	(MX53_AIPS2_BASE_ADDR + 0x000EC000)
#define MX53_TVE_BASE_ADDR	(MX53_AIPS2_BASE_ADDR + 0x000F0000)
#define MX53_VPU_BASE_ADDR	(MX53_AIPS2_BASE_ADDR + 0x000F4000)
#define MX53_SAHARA_BASE_ADDR	(MX53_AIPS2_BASE_ADDR + 0x000F8000)
#define MX53_PTP_BASE_ADDR	(MX53_AIPS2_BASE_ADDR + 0x000FC000)

/*
                        
 */
#define MX53_CSD0_BASE_ADDR		0x70000000
#define MX53_CSD1_BASE_ADDR		0xB0000000
#define MX53_CS0_BASE_ADDR		0xF0000000
#define MX53_CS1_32MB_BASE_ADDR	0xF2000000
#define MX53_CS1_64MB_BASE_ADDR		0xF4000000
#define MX53_CS2_64MB_BASE_ADDR		0xF4000000
#define MX53_CS2_96MB_BASE_ADDR		0xF6000000
#define MX53_CS3_BASE_ADDR		0xF6000000

#define MX53_IO_P2V(x)			IMX_IO_P2V(x)
#define MX53_IO_ADDRESS(x)		IOMEM(MX53_IO_P2V(x))

/*
                           
 */
#define MX53_SPBA_SDHC1	0x04
#define MX53_SPBA_SDHC2	0x08
#define MX53_SPBA_UART3	0x0C
#define MX53_SPBA_CSPI1	0x10
#define MX53_SPBA_SSI2		0x14
#define MX53_SPBA_SDHC3	0x20
#define MX53_SPBA_SDHC4	0x24
#define MX53_SPBA_SPDIF	0x28
#define MX53_SPBA_ATA		0x30
#define MX53_SPBA_SLIM		0x34
#define MX53_SPBA_HSI2C	0x38
#define MX53_SPBA_CTRL		0x3C

/*
                          
 */
#define MX53_DMA_REQ_SSI3_TX0		47
#define MX53_DMA_REQ_SSI3_RX0		46
#define MX53_DMA_REQ_SSI3_TX1		45
#define MX53_DMA_REQ_SSI3_RX1		44
#define MX53_DMA_REQ_UART3_TX	43
#define MX53_DMA_REQ_UART3_RX	42
#define MX53_DMA_REQ_ESAI_TX		41
#define MX53_DMA_REQ_ESAI_RX		40
#define MX53_DMA_REQ_CSPI_TX		39
#define MX53_DMA_REQ_CSPI_RX		38
#define MX53_DMA_REQ_ASRC_DMA6	37
#define MX53_DMA_REQ_ASRC_DMA5	36
#define MX53_DMA_REQ_ASRC_DMA4	35
#define MX53_DMA_REQ_ASRC_DMA3	34
#define MX53_DMA_REQ_ASRC_DMA2	33
#define MX53_DMA_REQ_ASRC_DMA1	32
#define MX53_DMA_REQ_EMI_WR		31
#define MX53_DMA_REQ_EMI_RD		30
#define MX53_DMA_REQ_SSI1_TX0		29
#define MX53_DMA_REQ_SSI1_RX0		28
#define MX53_DMA_REQ_SSI1_TX1		27
#define MX53_DMA_REQ_SSI1_RX1		26
#define MX53_DMA_REQ_SSI2_TX0		25
#define MX53_DMA_REQ_SSI2_RX0		24
#define MX53_DMA_REQ_SSI2_TX1		23
#define MX53_DMA_REQ_SSI2_RX1		22
#define MX53_DMA_REQ_I2C2_SDHC2	21
#define MX53_DMA_REQ_I2C1_SDHC1	20
#define MX53_DMA_REQ_UART1_TX	19
#define MX53_DMA_REQ_UART1_RX	18
#define MX53_DMA_REQ_UART5_TX	17
#define MX53_DMA_REQ_UART5_RX	16
#define MX53_DMA_REQ_SPDIF_TX		15
#define MX53_DMA_REQ_SPDIF_RX		14
#define MX53_DMA_REQ_UART2_FIRI_TX	13
#define MX53_DMA_REQ_UART2_FIRI_RX	12
#define MX53_DMA_REQ_SDHC4		11
#define MX53_DMA_REQ_I2C3_SDHC3	10
#define MX53_DMA_REQ_CSPI2_TX		9
#define MX53_DMA_REQ_CSPI2_RX		8
#define MX53_DMA_REQ_CSPI1_TX		7
#define MX53_DMA_REQ_CSPI1_RX		6
#define MX53_DMA_REQ_IPU		5
#define MX53_DMA_REQ_ATA_TX_END	4
#define MX53_DMA_REQ_ATA_UART4_TX	3
#define MX53_DMA_REQ_ATA_UART4_RX	2
#define MX53_DMA_REQ_GPC		1
#define MX53_DMA_REQ_VPU		0

/*
                    
 */
#include <asm/irq.h>
#define MX53_INT_RESV0		(NR_IRQS_LEGACY + 0)
#define MX53_INT_ESDHC1		(NR_IRQS_LEGACY + 1)
#define MX53_INT_ESDHC2		(NR_IRQS_LEGACY + 2)
#define MX53_INT_ESDHC3		(NR_IRQS_LEGACY + 3)
#define MX53_INT_ESDHC4		(NR_IRQS_LEGACY + 4)
#define MX53_INT_DAP		(NR_IRQS_LEGACY + 5)
#define MX53_INT_SDMA		(NR_IRQS_LEGACY + 6)
#define MX53_INT_IOMUX		(NR_IRQS_LEGACY + 7)
#define MX53_INT_NFC		(NR_IRQS_LEGACY + 8)
#define MX53_INT_VPU		(NR_IRQS_LEGACY + 9)
#define MX53_INT_IPU_ERR	(NR_IRQS_LEGACY + 10)
#define MX53_INT_IPU_SYN	(NR_IRQS_LEGACY + 11)
#define MX53_INT_GPU		(NR_IRQS_LEGACY + 12)
#define MX53_INT_UART4		(NR_IRQS_LEGACY + 13)
#define MX53_INT_USB_H1		(NR_IRQS_LEGACY + 14)
#define MX53_INT_EMI		(NR_IRQS_LEGACY + 15)
#define MX53_INT_USB_H2		(NR_IRQS_LEGACY + 16)
#define MX53_INT_USB_H3		(NR_IRQS_LEGACY + 17)
#define MX53_INT_USB_OTG	(NR_IRQS_LEGACY + 18)
#define MX53_INT_SAHARA_H0	(NR_IRQS_LEGACY + 19)
#define MX53_INT_SAHARA_H1	(NR_IRQS_LEGACY + 20)
#define MX53_INT_SCC_SMN	(NR_IRQS_LEGACY + 21)
#define MX53_INT_SCC_STZ	(NR_IRQS_LEGACY + 22)
#define MX53_INT_SCC_SCM	(NR_IRQS_LEGACY + 23)
#define MX53_INT_SRTC_NTZ	(NR_IRQS_LEGACY + 24)
#define MX53_INT_SRTC_TZ	(NR_IRQS_LEGACY + 25)
#define MX53_INT_RTIC		(NR_IRQS_LEGACY + 26)
#define MX53_INT_CSU		(NR_IRQS_LEGACY + 27)
#define MX53_INT_SATA		(NR_IRQS_LEGACY + 28)
#define MX53_INT_SSI1		(NR_IRQS_LEGACY + 29)
#define MX53_INT_SSI2		(NR_IRQS_LEGACY + 30)
#define MX53_INT_UART1		(NR_IRQS_LEGACY + 31)
#define MX53_INT_UART2		(NR_IRQS_LEGACY + 32)
#define MX53_INT_UART3		(NR_IRQS_LEGACY + 33)
#define MX53_INT_RTC		(NR_IRQS_LEGACY + 34)
#define MX53_INT_PTP		(NR_IRQS_LEGACY + 35)
#define MX53_INT_ECSPI1		(NR_IRQS_LEGACY + 36)
#define MX53_INT_ECSPI2		(NR_IRQS_LEGACY + 37)
#define MX53_INT_CSPI		(NR_IRQS_LEGACY + 38)
#define MX53_INT_GPT		(NR_IRQS_LEGACY + 39)
#define MX53_INT_EPIT1		(NR_IRQS_LEGACY + 40)
#define MX53_INT_EPIT2		(NR_IRQS_LEGACY + 41)
#define MX53_INT_GPIO1_INT7	(NR_IRQS_LEGACY + 42)
#define MX53_INT_GPIO1_INT6	(NR_IRQS_LEGACY + 43)
#define MX53_INT_GPIO1_INT5	(NR_IRQS_LEGACY + 44)
#define MX53_INT_GPIO1_INT4	(NR_IRQS_LEGACY + 45)
#define MX53_INT_GPIO1_INT3	(NR_IRQS_LEGACY + 46)
#define MX53_INT_GPIO1_INT2	(NR_IRQS_LEGACY + 47)
#define MX53_INT_GPIO1_INT1	(NR_IRQS_LEGACY + 48)
#define MX53_INT_GPIO1_INT0	(NR_IRQS_LEGACY + 49)
#define MX53_INT_GPIO1_LOW	(NR_IRQS_LEGACY + 50)
#define MX53_INT_GPIO1_HIGH	(NR_IRQS_LEGACY + 51)
#define MX53_INT_GPIO2_LOW	(NR_IRQS_LEGACY + 52)
#define MX53_INT_GPIO2_HIGH	(NR_IRQS_LEGACY + 53)
#define MX53_INT_GPIO3_LOW	(NR_IRQS_LEGACY + 54)
#define MX53_INT_GPIO3_HIGH	(NR_IRQS_LEGACY + 55)
#define MX53_INT_GPIO4_LOW	(NR_IRQS_LEGACY + 56)
#define MX53_INT_GPIO4_HIGH	(NR_IRQS_LEGACY + 57)
#define MX53_INT_WDOG1		(NR_IRQS_LEGACY + 58)
#define MX53_INT_WDOG2		(NR_IRQS_LEGACY + 59)
#define MX53_INT_KPP		(NR_IRQS_LEGACY + 60)
#define MX53_INT_PWM1		(NR_IRQS_LEGACY + 61)
#define MX53_INT_I2C1		(NR_IRQS_LEGACY + 62)
#define MX53_INT_I2C2		(NR_IRQS_LEGACY + 63)
#define MX53_INT_I2C3		(NR_IRQS_LEGACY + 64)
#define MX53_INT_MLB		(NR_IRQS_LEGACY + 65)
#define MX53_INT_ASRC		(NR_IRQS_LEGACY + 66)
#define MX53_INT_SPDIF		(NR_IRQS_LEGACY + 67)
#define MX53_INT_SIM_DAT	(NR_IRQS_LEGACY + 68)
#define MX53_INT_IIM		(NR_IRQS_LEGACY + 69)
#define MX53_INT_ATA		(NR_IRQS_LEGACY + 70)
#define MX53_INT_CCM1		(NR_IRQS_LEGACY + 71)
#define MX53_INT_CCM2		(NR_IRQS_LEGACY + 72)
#define MX53_INT_GPC1		(NR_IRQS_LEGACY + 73)
#define MX53_INT_GPC2		(NR_IRQS_LEGACY + 74)
#define MX53_INT_SRC		(NR_IRQS_LEGACY + 75)
#define MX53_INT_NM		(NR_IRQS_LEGACY + 76)
#define MX53_INT_PMU		(NR_IRQS_LEGACY + 77)
#define MX53_INT_CTI_IRQ	(NR_IRQS_LEGACY + 78)
#define MX53_INT_CTI1_TG0	(NR_IRQS_LEGACY + 79)
#define MX53_INT_CTI1_TG1	(NR_IRQS_LEGACY + 80)
#define MX53_INT_ESAI		(NR_IRQS_LEGACY + 81)
#define MX53_INT_CAN1		(NR_IRQS_LEGACY + 82)
#define MX53_INT_CAN2		(NR_IRQS_LEGACY + 83)
#define MX53_INT_GPU2_IRQ	(NR_IRQS_LEGACY + 84)
#define MX53_INT_GPU2_BUSY	(NR_IRQS_LEGACY + 85)
#define MX53_INT_UART5		(NR_IRQS_LEGACY + 86)
#define MX53_INT_FEC		(NR_IRQS_LEGACY + 87)
#define MX53_INT_OWIRE		(NR_IRQS_LEGACY + 88)
#define MX53_INT_CTI1_TG2	(NR_IRQS_LEGACY + 89)
#define MX53_INT_SJC		(NR_IRQS_LEGACY + 90)
#define MX53_INT_TVE		(NR_IRQS_LEGACY + 92)
#define MX53_INT_FIRI		(NR_IRQS_LEGACY + 93)
#define MX53_INT_PWM2		(NR_IRQS_LEGACY + 94)
#define MX53_INT_SLIM_EXP	(NR_IRQS_LEGACY + 95)
#define MX53_INT_SSI3		(NR_IRQS_LEGACY + 96)
#define MX53_INT_EMI_BOOT	(NR_IRQS_LEGACY + 97)
#define MX53_INT_CTI1_TG3	(NR_IRQS_LEGACY + 98)
#define MX53_INT_SMC_RX		(NR_IRQS_LEGACY + 99)
#define MX53_INT_VPU_IDLE	(NR_IRQS_LEGACY + 100)
#define MX53_INT_EMI_NFC	(NR_IRQS_LEGACY + 101)
#define MX53_INT_GPU_IDLE	(NR_IRQS_LEGACY + 102)
#define MX53_INT_GPIO5_LOW	(NR_IRQS_LEGACY + 103)
#define MX53_INT_GPIO5_HIGH	(NR_IRQS_LEGACY + 104)
#define MX53_INT_GPIO6_LOW	(NR_IRQS_LEGACY + 105)
#define MX53_INT_GPIO6_HIGH	(NR_IRQS_LEGACY + 106)
#define MX53_INT_GPIO7_LOW	(NR_IRQS_LEGACY + 107)
#define MX53_INT_GPIO7_HIGH	(NR_IRQS_LEGACY + 108)

#endif /*                        */
