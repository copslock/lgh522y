/*
 * Copyright (C) 2007 Google, Inc.
 * Copyright (c) 2008-2011 Code Aurora Forum. All rights reserved.
 * Author: Brian Swetland <swetland@google.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *
 * The MSM peripherals are spread all over across 768MB of physical
 * space, which makes just having a simple IO_ADDRESS macro to slide
 * them into the right virtual location rough.  Instead, we will
 * provide a master phys->virt mapping for peripherals here.
 *
 */

#ifndef __ASM_ARCH_MSM_IOMAP_8X50_H
#define __ASM_ARCH_MSM_IOMAP_8X50_H

/*                                               
                                                                
  
                                                               
                                                         
  
                                                             
                                                              
           
  
 */

#define MSM_VIC_BASE          IOMEM(0xE0000000)
#define MSM_VIC_PHYS          0xAC000000
#define MSM_VIC_SIZE          SZ_4K

#define QSD8X50_CSR_PHYS      0xAC100000
#define QSD8X50_CSR_SIZE      SZ_4K

#define MSM_DMOV_BASE         IOMEM(0xE0002000)
#define MSM_DMOV_PHYS         0xA9700000
#define MSM_DMOV_SIZE         SZ_4K

#define QSD8X50_GPIO1_PHYS        0xA9000000
#define QSD8X50_GPIO1_SIZE        SZ_4K

#define QSD8X50_GPIO2_PHYS        0xA9100000
#define QSD8X50_GPIO2_SIZE        SZ_4K

#define MSM_CLK_CTL_BASE      IOMEM(0xE0005000)
#define MSM_CLK_CTL_PHYS      0xA8600000
#define MSM_CLK_CTL_SIZE      SZ_4K

#define MSM_SIRC_BASE         IOMEM(0xE1006000)
#define MSM_SIRC_PHYS         0xAC200000
#define MSM_SIRC_SIZE         SZ_4K

#define MSM_SCPLL_BASE        IOMEM(0xE1007000)
#define MSM_SCPLL_PHYS        0xA8800000
#define MSM_SCPLL_SIZE        SZ_4K

#ifdef CONFIG_MSM_SOC_REV_A
#define MSM_SMI_BASE 0xE0000000
#else
#define MSM_SMI_BASE 0x00000000
#endif

#define MSM_SHARED_RAM_BASE   IOMEM(0xE0100000)
#define MSM_SHARED_RAM_PHYS (MSM_SMI_BASE + 0x00100000)
#define MSM_SHARED_RAM_SIZE   SZ_1M

#define MSM_UART1_PHYS        0xA9A00000
#define MSM_UART1_SIZE        SZ_4K

#define MSM_UART2_PHYS        0xA9B00000
#define MSM_UART2_SIZE        SZ_4K

#define MSM_UART3_PHYS        0xA9C00000
#define MSM_UART3_SIZE        SZ_4K

#define MSM_MDC_BASE	      IOMEM(0xE0200000)
#define MSM_MDC_PHYS	      0xAA500000
#define MSM_MDC_SIZE	      SZ_1M

#define MSM_AD5_BASE          IOMEM(0xE0300000)
#define MSM_AD5_PHYS          0xAC000000
#define MSM_AD5_SIZE          (SZ_1M*13)


#define MSM_I2C_SIZE          SZ_4K
#define MSM_I2C_PHYS          0xA9900000

#define MSM_HSUSB_PHYS        0xA0800000
#define MSM_HSUSB_SIZE        SZ_1K

#define MSM_NAND_PHYS           0xA0A00000


#define MSM_TSIF_PHYS        (0xa0100000)
#define MSM_TSIF_SIZE        (0x200)

#define MSM_TSSC_PHYS         0xAA300000

#define MSM_UART1DM_PHYS      0xA0200000
#define MSM_UART2DM_PHYS      0xA0900000


#define MSM_SDC1_PHYS          0xA0300000
#define MSM_SDC1_SIZE          SZ_4K

#define MSM_SDC2_PHYS          0xA0400000
#define MSM_SDC2_SIZE          SZ_4K

#define MSM_SDC3_PHYS          0xA0500000
#define MSM_SDC3_SIZE           SZ_4K

#define MSM_SDC4_PHYS          0xA0600000
#define MSM_SDC4_SIZE          SZ_4K

#endif
