/*
 * arch/arm/mach-versatile/include/mach/platform.h
 *
 * Copyright (c) ARM Limited 2003.  All rights reserved.
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

#ifndef __address_h
#define __address_h                     1

/*
                     
 */
#define VERSATILE_BOOT_ROM_LO          0x30000000		/*                   */
#define VERSATILE_BOOT_ROM_HI          0x30000000
#define VERSATILE_BOOT_ROM_BASE        VERSATILE_BOOT_ROM_HI	 /*                  */
#define VERSATILE_BOOT_ROM_SIZE        SZ_64M

#define VERSATILE_SSRAM_BASE           /*                       */
#define VERSATILE_SSRAM_SIZE           SZ_2M

#define VERSATILE_FLASH_BASE           0x34000000
#define VERSATILE_FLASH_SIZE           SZ_64M

/* 
         
 */
#define VERSATILE_SDRAM_BASE           0x00000000

/* 
                           
   
 */


/*                                                                         
                       
                                                                           
   
 */
#define VERSATILE_SYS_ID_OFFSET               0x00
#define VERSATILE_SYS_SW_OFFSET               0x04
#define VERSATILE_SYS_LED_OFFSET              0x08
#define VERSATILE_SYS_OSC0_OFFSET             0x0C

#if defined(CONFIG_ARCH_VERSATILE_PB)
#define VERSATILE_SYS_OSC1_OFFSET             0x10
#define VERSATILE_SYS_OSC2_OFFSET             0x14
#define VERSATILE_SYS_OSC3_OFFSET             0x18
#define VERSATILE_SYS_OSC4_OFFSET             0x1C
#elif defined(CONFIG_MACH_VERSATILE_AB)
#define VERSATILE_SYS_OSC1_OFFSET             0x1C
#endif

#define VERSATILE_SYS_OSCCLCD_OFFSET          0x1c

#define VERSATILE_SYS_LOCK_OFFSET             0x20
#define VERSATILE_SYS_100HZ_OFFSET            0x24
#define VERSATILE_SYS_CFGDATA1_OFFSET         0x28
#define VERSATILE_SYS_CFGDATA2_OFFSET         0x2C
#define VERSATILE_SYS_FLAGS_OFFSET            0x30
#define VERSATILE_SYS_FLAGSSET_OFFSET         0x30
#define VERSATILE_SYS_FLAGSCLR_OFFSET         0x34
#define VERSATILE_SYS_NVFLAGS_OFFSET          0x38
#define VERSATILE_SYS_NVFLAGSSET_OFFSET       0x38
#define VERSATILE_SYS_NVFLAGSCLR_OFFSET       0x3C
#define VERSATILE_SYS_RESETCTL_OFFSET         0x40
#define VERSATILE_SYS_PCICTL_OFFSET           0x44
#define VERSATILE_SYS_MCI_OFFSET              0x48
#define VERSATILE_SYS_FLASH_OFFSET            0x4C
#define VERSATILE_SYS_CLCD_OFFSET             0x50
#define VERSATILE_SYS_CLCDSER_OFFSET          0x54
#define VERSATILE_SYS_BOOTCS_OFFSET           0x58
#define VERSATILE_SYS_24MHz_OFFSET            0x5C
#define VERSATILE_SYS_MISC_OFFSET             0x60
#define VERSATILE_SYS_TEST_OSC0_OFFSET        0x80
#define VERSATILE_SYS_TEST_OSC1_OFFSET        0x84
#define VERSATILE_SYS_TEST_OSC2_OFFSET        0x88
#define VERSATILE_SYS_TEST_OSC3_OFFSET        0x8C
#define VERSATILE_SYS_TEST_OSC4_OFFSET        0x90

#define VERSATILE_SYS_BASE                    0x10000000
#define VERSATILE_SYS_ID                      (VERSATILE_SYS_BASE + VERSATILE_SYS_ID_OFFSET)
#define VERSATILE_SYS_SW                      (VERSATILE_SYS_BASE + VERSATILE_SYS_SW_OFFSET)
#define VERSATILE_SYS_LED                     (VERSATILE_SYS_BASE + VERSATILE_SYS_LED_OFFSET)
#define VERSATILE_SYS_OSC0                    (VERSATILE_SYS_BASE + VERSATILE_SYS_OSC0_OFFSET)
#define VERSATILE_SYS_OSC1                    (VERSATILE_SYS_BASE + VERSATILE_SYS_OSC1_OFFSET)

#if defined(CONFIG_ARCH_VERSATILE_PB)
#define VERSATILE_SYS_OSC2                    (VERSATILE_SYS_BASE + VERSATILE_SYS_OSC2_OFFSET)
#define VERSATILE_SYS_OSC3                    (VERSATILE_SYS_BASE + VERSATILE_SYS_OSC3_OFFSET)
#define VERSATILE_SYS_OSC4                    (VERSATILE_SYS_BASE + VERSATILE_SYS_OSC4_OFFSET)
#endif

#define VERSATILE_SYS_LOCK                    (VERSATILE_SYS_BASE + VERSATILE_SYS_LOCK_OFFSET)
#define VERSATILE_SYS_100HZ                   (VERSATILE_SYS_BASE + VERSATILE_SYS_100HZ_OFFSET)
#define VERSATILE_SYS_CFGDATA1                (VERSATILE_SYS_BASE + VERSATILE_SYS_CFGDATA1_OFFSET)
#define VERSATILE_SYS_CFGDATA2                (VERSATILE_SYS_BASE + VERSATILE_SYS_CFGDATA2_OFFSET)
#define VERSATILE_SYS_FLAGS                   (VERSATILE_SYS_BASE + VERSATILE_SYS_FLAGS_OFFSET)
#define VERSATILE_SYS_FLAGSSET                (VERSATILE_SYS_BASE + VERSATILE_SYS_FLAGSSET_OFFSET)
#define VERSATILE_SYS_FLAGSCLR                (VERSATILE_SYS_BASE + VERSATILE_SYS_FLAGSCLR_OFFSET)
#define VERSATILE_SYS_NVFLAGS                 (VERSATILE_SYS_BASE + VERSATILE_SYS_NVFLAGS_OFFSET)
#define VERSATILE_SYS_NVFLAGSSET              (VERSATILE_SYS_BASE + VERSATILE_SYS_NVFLAGSSET_OFFSET)
#define VERSATILE_SYS_NVFLAGSCLR              (VERSATILE_SYS_BASE + VERSATILE_SYS_NVFLAGSCLR_OFFSET)
#define VERSATILE_SYS_RESETCTL                (VERSATILE_SYS_BASE + VERSATILE_SYS_RESETCTL_OFFSET)
#define VERSATILE_SYS_PCICTL                  (VERSATILE_SYS_BASE + VERSATILE_SYS_PCICTL_OFFSET)
#define VERSATILE_SYS_MCI                     (VERSATILE_SYS_BASE + VERSATILE_SYS_MCI_OFFSET)
#define VERSATILE_SYS_FLASH                   (VERSATILE_SYS_BASE + VERSATILE_SYS_FLASH_OFFSET)
#define VERSATILE_SYS_CLCD                    (VERSATILE_SYS_BASE + VERSATILE_SYS_CLCD_OFFSET)
#define VERSATILE_SYS_CLCDSER                 (VERSATILE_SYS_BASE + VERSATILE_SYS_CLCDSER_OFFSET)
#define VERSATILE_SYS_BOOTCS                  (VERSATILE_SYS_BASE + VERSATILE_SYS_BOOTCS_OFFSET)
#define VERSATILE_SYS_24MHz                   (VERSATILE_SYS_BASE + VERSATILE_SYS_24MHz_OFFSET)
#define VERSATILE_SYS_MISC                    (VERSATILE_SYS_BASE + VERSATILE_SYS_MISC_OFFSET)
#define VERSATILE_SYS_TEST_OSC0               (VERSATILE_SYS_BASE + VERSATILE_SYS_TEST_OSC0_OFFSET)
#define VERSATILE_SYS_TEST_OSC1               (VERSATILE_SYS_BASE + VERSATILE_SYS_TEST_OSC1_OFFSET)
#define VERSATILE_SYS_TEST_OSC2               (VERSATILE_SYS_BASE + VERSATILE_SYS_TEST_OSC2_OFFSET)
#define VERSATILE_SYS_TEST_OSC3               (VERSATILE_SYS_BASE + VERSATILE_SYS_TEST_OSC3_OFFSET)
#define VERSATILE_SYS_TEST_OSC4               (VERSATILE_SYS_BASE + VERSATILE_SYS_TEST_OSC4_OFFSET)

/* 
                                      
 */
#define VERSATILE_SYS_CTRL_RESET_CONFIGCLR    0x01
#define VERSATILE_SYS_CTRL_RESET_CONFIGINIT   0x02
#define VERSATILE_SYS_CTRL_RESET_DLLRESET     0x03
#define VERSATILE_SYS_CTRL_RESET_PLLRESET     0x04
#define VERSATILE_SYS_CTRL_RESET_POR          0x05
#define VERSATILE_SYS_CTRL_RESET_DoC          0x06

#define VERSATILE_SYS_CTRL_LED         (1 << 0)


/*                                                                         
                               
                                                                           
 */

/* 
                    
  
                                    
                                                                  
                                         
                      
                                              
 */

/*
                     
                                                               
                          
 */
#define VERSATILE_SYS_LOCK_LOCKED    (1 << 16)
#define VERSATILE_SYS_LOCKVAL_MASK	0xFFFF		/*                                     */

/*
                      
 */
#define VERSATILE_FLASHPROG_FLVPPEN	(1 << 0)	/*                         */

/*
                   
                                                                  
 */
#define VERSATILE_INTREG_WPROT        0x00    /*                                                */
#define VERSATILE_INTREG_RI0          0x01    /*                                                */
#define VERSATILE_INTREG_CARDIN       0x08    /*                                                */
                                                /*                                                */
#define VERSATILE_INTREG_RI1          0x02    /*                                                */
#define VERSATILE_INTREG_CARDINSERT   0x03    /*                                                */

/*
                                 
 */
#define VERSATILE_PCI_CORE_BASE        0x10001000	/*                  */
#define VERSATILE_I2C_BASE             0x10002000	/*             */
#define VERSATILE_SIC_BASE             0x10003000	/*                                */
#define VERSATILE_AACI_BASE            0x10004000	/*       */
#define VERSATILE_MMCI0_BASE           0x10005000	/*               */
#define VERSATILE_KMI0_BASE            0x10006000	/*               */
#define VERSATILE_KMI1_BASE            0x10007000	/*                   */
#define VERSATILE_CHAR_LCD_BASE        0x10008000	/*               */
#define VERSATILE_UART3_BASE           0x10009000	/*        */
#define VERSATILE_SCI1_BASE            0x1000A000
#define VERSATILE_MMCI1_BASE           0x1000B000    /*               */
	/*                                    */
#define VERSATILE_ETH_BASE             0x10010000	/*          */
#define VERSATILE_USB_BASE             0x10020000	/*     */
	/*                                    */
#define VERSATILE_SMC_BASE             0x10100000	/*     */
#define VERSATILE_MPMC_BASE            0x10110000	/*      */
#define VERSATILE_CLCD_BASE            0x10120000	/*      */
#define VERSATILE_DMAC_BASE            0x10130000	/*                */
#define VERSATILE_VIC_BASE             0x10140000	/*                               */
#define VERSATILE_PERIPH_BASE          0x10150000	/*                                 */
                                                /*                         */
#define VERSATILE_AHBM_BASE            0x101D0000	/*             */
#define VERSATILE_SCTL_BASE            0x101E0000	/*                   */
#define VERSATILE_WATCHDOG_BASE        0x101E1000	/*          */
#define VERSATILE_TIMER0_1_BASE        0x101E2000	/*               */
#define VERSATILE_TIMER2_3_BASE        0x101E3000	/*               */
#define VERSATILE_GPIO0_BASE           0x101E4000	/*             */
#define VERSATILE_GPIO1_BASE           0x101E5000	/*             */
#define VERSATILE_GPIO2_BASE           0x101E6000	/*             */
#define VERSATILE_GPIO3_BASE           0x101E7000	/*             */
#define VERSATILE_RTC_BASE             0x101E8000	/*                 */
	/*                       */
#define VERSATILE_SCI_BASE             0x101F0000	/*                       */
#define VERSATILE_UART0_BASE           0x101F1000	/*        */
#define VERSATILE_UART1_BASE           0x101F2000	/*        */
#define VERSATILE_UART2_BASE           0x101F3000	/*        */
#define VERSATILE_SSP_BASE             0x101F4000	/*                         */

#define VERSATILE_SSMC_BASE            0x20000000	/*      */
#define VERSATILE_IB2_BASE             0x24000000	/*            */
#define VERSATILE_MBX_BASE             0x40000000	/*     */

/*           */
#define VERSATILE_PCI_BASE             0x41000000	/*               */
#define VERSATILE_PCI_CFG_BASE	       0x42000000
#define VERSATILE_PCI_IO_BASE          0x43000000
#define VERSATILE_PCI_MEM_BASE0        0x44000000
#define VERSATILE_PCI_MEM_BASE1        0x50000000
#define VERSATILE_PCI_MEM_BASE2        0x60000000
/*                     */
#define VERSATILE_PCI_BASE_SIZE	       0x01000000
#define VERSATILE_PCI_CFG_BASE_SIZE    0x02000000
#define VERSATILE_PCI_IO_BASE_SIZE     0x01000000
#define VERSATILE_PCI_MEM_BASE0_SIZE   0x0c000000	/*      */
#define VERSATILE_PCI_MEM_BASE1_SIZE   0x10000000	/*       */
#define VERSATILE_PCI_MEM_BASE2_SIZE   0x10000000	/*       */

#define VERSATILE_SDRAM67_BASE         0x70000000	/*                     */
#define VERSATILE_LT_BASE              0x80000000	/*                      */

/*
               
 */
#define VERSATILE_DOC_BASE             0x2C000000
#define VERSATILE_DOC_SIZE             (16 << 20)
#define VERSATILE_DOC_PAGE_SIZE        512
#define VERSATILE_DOC_TOTAL_PAGES     (DOC_SIZE / PAGE_SIZE)

#define ERASE_UNIT_PAGES    32
#define START_PAGE          0x80

/* 
                            
 */
#define VERSATILE_SYS_LED0             (1 << 0)
#define VERSATILE_SYS_LED1             (1 << 1)
#define VERSATILE_SYS_LED2             (1 << 2)
#define VERSATILE_SYS_LED3             (1 << 3)
#define VERSATILE_SYS_LED4             (1 << 4)
#define VERSATILE_SYS_LED5             (1 << 5)
#define VERSATILE_SYS_LED6             (1 << 6)
#define VERSATILE_SYS_LED7             (1 << 7)

#define ALL_LEDS                  0xFF

#define LED_BANK                  VERSATILE_SYS_LED

/* 
                    
 */
#define VERSATILE_IDFIELD_OFFSET	0x0	/*                             */
#define VERSATILE_FLASHPROG_OFFSET	0x4	/*               */
#define VERSATILE_INTREG_OFFSET		0x8	/*                   */
#define VERSATILE_DECODE_OFFSET		0xC	/*                      */


/*                                                                         
                                                      
                                                                           
   
                                           
   
                                                  
   
                     
   
                                            
   
                     
   
 */
/*                                                   */

#define SIC_IRQ_STATUS                  0
#define SIC_IRQ_RAW_STATUS              0x04
#define SIC_IRQ_ENABLE                  0x08
#define SIC_IRQ_ENABLE_SET              0x08
#define SIC_IRQ_ENABLE_CLEAR            0x0C
#define SIC_INT_SOFT_SET                0x10
#define SIC_INT_SOFT_CLEAR              0x14
#define SIC_INT_PIC_ENABLE              0x20	/*                                  */
#define SIC_INT_PIC_ENABLES             0x20	/*                                 */
#define SIC_INT_PIC_ENABLEC             0x24	/*                                   */

/*                                                                         
                                         
                                                                           
 */

#define INT_WDOGINT                     0	/*                */
#define INT_SOFTINT                     1	/*                    */
#define INT_COMMRx                      2	/*                         */
#define INT_COMMTx                      3	/*                         */
#define INT_TIMERINT0_1                 4	/*               */
#define INT_TIMERINT2_3                 5	/*               */
#define INT_GPIOINT0                    6	/*        */
#define INT_GPIOINT1                    7	/*        */
#define INT_GPIOINT2                    8	/*        */
#define INT_GPIOINT3                    9	/*        */
#define INT_RTCINT                      10	/*                 */
#define INT_SSPINT                      11	/*                         */
#define INT_UARTINT0                    12	/*                            */
#define INT_UARTINT1                    13	/*                            */
#define INT_UARTINT2                    14	/*                            */
#define INT_SCIINT                      15	/*                      */
#define INT_CLCDINT                     16	/*                 */
#define INT_DMAINT                      17	/*                */
#define INT_PWRFAILINT                  18	/*               */
#define INT_MBXINT                      19	/*                    */
#define INT_GNDINT                      20	/*          */
	/*                                                                     */
#define INT_VICSOURCE21                 21	/*              */
#define INT_VICSOURCE22                 22	/*       */
#define INT_VICSOURCE23                 23	/*       */
#define INT_VICSOURCE24                 24	/*      */
#define INT_VICSOURCE25                 25	/*          */
#define INT_VICSOURCE26                 26	/*     */
#define INT_VICSOURCE27                 27	/*       */
#define INT_VICSOURCE28                 28	/*       */
#define INT_VICSOURCE29                 29	/*       */
#define INT_VICSOURCE30                 30	/*       */
#define INT_VICSOURCE31                 31	/*            */

#define VERSATILE_SC_VALID_INT               0x003FFFFF

#define MAXIRQNUM                       31
#define MAXFIQNUM                       31
#define MAXSWINUM                       31

/*                                                                         
                                           
                                                                           
 */
#define SIC_INT_MMCI0B                  1	/*                    */
#define SIC_INT_MMCI1B                  2	/*                    */
#define SIC_INT_KMI0                    3	/*                       */
#define SIC_INT_KMI1                    4	/*                       */
#define SIC_INT_SCI3                    5	/*                      */
#define SIC_INT_UART3                   6	/*                                */
#define SIC_INT_CLCD                    7	/*               */
#define SIC_INT_TOUCH                   8	/*             */
#define SIC_INT_KEYPAD                  9	/*                               */
	/*                  */
#define SIC_INT_DoC                     21	/*                                */
#define SIC_INT_MMCI0A                  22	/*        */
#define SIC_INT_MMCI1A                  23	/*        */
#define SIC_INT_AACI                    24	/*             */
#define SIC_INT_ETH                     25	/*                     */
#define SIC_INT_USB                     26	/*                */
#define SIC_INT_PCI0                    27
#define SIC_INT_PCI1                    28
#define SIC_INT_PCI2                    29
#define SIC_INT_PCI3                    30


/*
                                   
 */
#define VERSATILE_REFCLK	0
#define VERSATILE_TIMCLK	1

#define VERSATILE_TIMER1_EnSel	15
#define VERSATILE_TIMER2_EnSel	17
#define VERSATILE_TIMER3_EnSel	19
#define VERSATILE_TIMER4_EnSel	21


#define VERSATILE_CSR_BASE             0x10000000
#define VERSATILE_CSR_SIZE             0x10000000

#ifdef CONFIG_MACH_VERSATILE_AB
/*
                                               
 */
#define VERSATILE_IB2_CAMERA_BANK	VERSATILE_IB2_BASE
#define VERSATILE_IB2_KBD_DATAREG	(VERSATILE_IB2_BASE + 0x01000000)

/*                */
#define VERSATILE_IB2_INT_BASE		(VERSATILE_IB2_BASE + 0x02000000)
#define VERSATILE_IB2_IER		(VERSATILE_IB2_INT_BASE + 0)
#define VERSATILE_IB2_ISR		(VERSATILE_IB2_INT_BASE + 4)

#define VERSATILE_IB2_CTL_BASE		(VERSATILE_IB2_BASE + 0x03000000)
#define VERSATILE_IB2_CTRL		(VERSATILE_IB2_CTL_BASE + 0)
#define VERSATILE_IB2_STAT		(VERSATILE_IB2_CTL_BASE + 4)
#endif

#endif
