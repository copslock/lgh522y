/*
 *
 * arch/arm/mach-u300/include/mach/u300-regs.h
 *
 *
 * Copyright (C) 2006-2009 ST-Ericsson AB
 * License terms: GNU General Public License (GPL) version 2
 * Basic register address definitions in physical memory and
 * some block definitions for core devices like the timer.
 * Author: Linus Walleij <linus.walleij@stericsson.com>
 */

#ifndef __MACH_U300_REGS_H
#define __MACH_U300_REGS_H

/*
                                                          
                                                              
 */

/*                */
#define U300_NAND_CS0_PHYS_BASE		0x80000000

/*      */
#define U300_NAND_IF_PHYS_BASE		0x9f800000

/*                               */
#define PLAT_NAND_CLE			(1 << 16)
#define PLAT_NAND_ALE			(1 << 17)

/*                 */
#define U300_AHB_PER_PHYS_BASE		0xa0000000
#define U300_AHB_PER_VIRT_BASE		0xff010000

/*                  */
#define U300_FAST_PER_PHYS_BASE		0xc0000000
#define U300_FAST_PER_VIRT_BASE		0xff020000

/*                  */
#define U300_SLOW_PER_PHYS_BASE		0xc0010000
#define U300_SLOW_PER_VIRT_BASE		0xff000000

/*          */
#define U300_BOOTROM_PHYS_BASE		0xffff0000
#define U300_BOOTROM_VIRT_BASE		0xffff0000

/*                  */
#define U300_SEMI_CONFIG_BASE		0x2FFE0000

/*
                  
 */

/*                                   */
#define U300_AHB_BRIDGE_BASE		(U300_AHB_PER_PHYS_BASE+0x0000)

/*                                                          */
#define U300_INTCON0_BASE		(U300_AHB_PER_PHYS_BASE+0x1000)
#define U300_INTCON0_VBASE		IOMEM(U300_AHB_PER_VIRT_BASE+0x1000)

/*                                                          */
#define U300_INTCON1_BASE		(U300_AHB_PER_PHYS_BASE+0x2000)
#define U300_INTCON1_VBASE		IOMEM(U300_AHB_PER_VIRT_BASE+0x2000)

/*                                     */
#define U300_MSPRO_BASE			(U300_AHB_PER_PHYS_BASE+0x3000)

/*                         */
#define U300_EMIF_CFG_BASE		(U300_AHB_PER_PHYS_BASE+0x4000)


/*
                   
 */

/*                     */
#define U300_FAST_BRIDGE_BASE		(U300_FAST_PER_PHYS_BASE+0x0000)

/*                   */
#define U300_MMCSD_BASE			(U300_FAST_PER_PHYS_BASE+0x1000)

/*                     */
#define U300_PCM_I2S0_BASE		(U300_FAST_PER_PHYS_BASE+0x2000)

/*                     */
#define U300_PCM_I2S1_BASE		(U300_FAST_PER_PHYS_BASE+0x3000)

/*                 */
#define U300_I2C0_BASE			(U300_FAST_PER_PHYS_BASE+0x4000)

/*                 */
#define U300_I2C1_BASE			(U300_FAST_PER_PHYS_BASE+0x5000)

/*                */
#define U300_SPI_BASE			(U300_FAST_PER_PHYS_BASE+0x6000)

/*                         */
#define U300_UART1_BASE			(U300_FAST_PER_PHYS_BASE+0x7000)

/*
                   
 */

/*                     */
#define U300_SLOW_BRIDGE_BASE		(U300_SLOW_PER_PHYS_BASE)

/*        */
#define U300_SYSCON_BASE		(U300_SLOW_PER_PHYS_BASE+0x1000)
#define U300_SYSCON_VBASE		IOMEM(U300_SLOW_PER_VIRT_BASE+0x1000)

/*          */
#define U300_WDOG_BASE			(U300_SLOW_PER_PHYS_BASE+0x2000)

/*       */
#define U300_UART0_BASE			(U300_SLOW_PER_PHYS_BASE+0x3000)

/*                        */
#define U300_TIMER_APP_BASE		(U300_SLOW_PER_PHYS_BASE+0x4000)
#define U300_TIMER_APP_VBASE		IOMEM(U300_SLOW_PER_VIRT_BASE+0x4000)

/*        */
#define U300_KEYPAD_BASE		(U300_SLOW_PER_PHYS_BASE+0x5000)

/*      */
#define U300_GPIO_BASE			(U300_SLOW_PER_PHYS_BASE+0x6000)

/*     */
#define U300_RTC_BASE			(U300_SLOW_PER_PHYS_BASE+0x7000)

/*            */
#define U300_BUSTR_BASE			(U300_SLOW_PER_PHYS_BASE+0x8000)

/*                                */
#define U300_EVHIST_BASE		(U300_SLOW_PER_PHYS_BASE+0x9000)

/*              */
#define U300_TIMER_BASE			(U300_SLOW_PER_PHYS_BASE+0xa000)

/*     */
#define U300_PPM_BASE			(U300_SLOW_PER_PHYS_BASE+0xb000)


/*
                   
 */

/*                              */
#define U300_ISP_BASE			(0xA0008000)

/*                     */
#define U300_DMAC_BASE			(0xC0020000)

/*          */
#define U300_MSL_BASE			(0xc0022000)

/*           */
#define U300_APEX_BASE			(0xc0030000)

/*                    */
#define U300_VIDEOENC_BASE		(0xc0080000)

/*           */
#define U300_XGAM_BASE			(0xd0000000)

#endif
