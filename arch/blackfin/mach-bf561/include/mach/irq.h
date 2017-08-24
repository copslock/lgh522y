/*
 * Copyright 2005-2008 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#ifndef _BF561_IRQ_H_
#define _BF561_IRQ_H_

#include <mach-common/irq.h>

#define NR_PERI_INTS		(2 * 32)

#define IRQ_PLL_WAKEUP		BFIN_IRQ(0)	/*                      */
#define IRQ_DMA1_ERROR		BFIN_IRQ(1)	/*                        */
#define IRQ_DMA_ERROR		IRQ_DMA1_ERROR	/*                        */
#define IRQ_DMA2_ERROR		BFIN_IRQ(2)	/*                        */
#define IRQ_IMDMA_ERROR		BFIN_IRQ(3)	/*                        */
#define IRQ_PPI1_ERROR		BFIN_IRQ(4)	/*                        */
#define IRQ_PPI_ERROR		IRQ_PPI1_ERROR	/*                        */
#define IRQ_PPI2_ERROR		BFIN_IRQ(5)	/*                        */
#define IRQ_SPORT0_ERROR	BFIN_IRQ(6)	/*                        */
#define IRQ_SPORT1_ERROR	BFIN_IRQ(7)	/*                        */
#define IRQ_SPI_ERROR		BFIN_IRQ(8)	/*                        */
#define IRQ_UART_ERROR		BFIN_IRQ(9)	/*                        */
#define IRQ_RESERVED_ERROR	BFIN_IRQ(10)	/*          */
#define IRQ_DMA1_0		BFIN_IRQ(11)	/*                         */
#define IRQ_PPI			IRQ_DMA1_0	/*                         */
#define IRQ_PPI0		IRQ_DMA1_0	/*                         */
#define IRQ_DMA1_1		BFIN_IRQ(12)	/*                         */
#define IRQ_PPI1		IRQ_DMA1_1	/*                         */
#define IRQ_DMA1_2		BFIN_IRQ(13)	/*                   */
#define IRQ_DMA1_3		BFIN_IRQ(14)	/*                   */
#define IRQ_DMA1_4		BFIN_IRQ(15)	/*                   */
#define IRQ_DMA1_5		BFIN_IRQ(16)	/*                   */
#define IRQ_DMA1_6		BFIN_IRQ(17)	/*                   */
#define IRQ_DMA1_7		BFIN_IRQ(18)	/*                   */
#define IRQ_DMA1_8		BFIN_IRQ(19)	/*                   */
#define IRQ_DMA1_9		BFIN_IRQ(20)	/*                   */
#define IRQ_DMA1_10		BFIN_IRQ(21)	/*                   */
#define IRQ_DMA1_11		BFIN_IRQ(22)	/*                   */
#define IRQ_DMA2_0		BFIN_IRQ(23)	/*                     */
#define IRQ_SPORT0_RX		IRQ_DMA2_0	/*                     */
#define IRQ_DMA2_1		BFIN_IRQ(24)	/*                     */
#define IRQ_SPORT0_TX		IRQ_DMA2_1	/*                     */
#define IRQ_DMA2_2		BFIN_IRQ(25)	/*                     */
#define IRQ_SPORT1_RX		IRQ_DMA2_2	/*                     */
#define IRQ_DMA2_3		BFIN_IRQ(26)	/*                     */
#define IRQ_SPORT1_TX		IRQ_DMA2_3	/*                     */
#define IRQ_DMA2_4		BFIN_IRQ(27)	/*               */
#define IRQ_SPI			IRQ_DMA2_4	/*               */
#define IRQ_DMA2_5		BFIN_IRQ(28)	/*                   */
#define IRQ_UART_RX		IRQ_DMA2_5	/*                   */
#define IRQ_DMA2_6		BFIN_IRQ(29)	/*                   */
#define IRQ_UART_TX		IRQ_DMA2_6	/*                   */
#define IRQ_DMA2_7		BFIN_IRQ(30)	/*                   */
#define IRQ_DMA2_8		BFIN_IRQ(31)	/*                   */
#define IRQ_DMA2_9		BFIN_IRQ(32)	/*                   */
#define IRQ_DMA2_10		BFIN_IRQ(33)	/*                   */
#define IRQ_DMA2_11		BFIN_IRQ(34)	/*                   */
#define IRQ_TIMER0		BFIN_IRQ(35)	/*                    */
#define IRQ_TIMER1		BFIN_IRQ(36)	/*                    */
#define IRQ_TIMER2		BFIN_IRQ(37)	/*                    */
#define IRQ_TIMER3		BFIN_IRQ(38)	/*                    */
#define IRQ_TIMER4		BFIN_IRQ(39)	/*                    */
#define IRQ_TIMER5		BFIN_IRQ(40)	/*                    */
#define IRQ_TIMER6		BFIN_IRQ(41)	/*                    */
#define IRQ_TIMER7		BFIN_IRQ(42)	/*                    */
#define IRQ_TIMER8		BFIN_IRQ(43)	/*                    */
#define IRQ_TIMER9		BFIN_IRQ(44)	/*                    */
#define IRQ_TIMER10		BFIN_IRQ(45)	/*                    */
#define IRQ_TIMER11		BFIN_IRQ(46)	/*                    */
#define IRQ_PROG0_INTA		BFIN_IRQ(47)	/*                           */
#define IRQ_PROG_INTA		IRQ_PROG0_INTA	/*                           */
#define IRQ_PROG0_INTB		BFIN_IRQ(48)	/*                           */
#define IRQ_PROG_INTB		IRQ_PROG0_INTB	/*                           */
#define IRQ_PROG1_INTA		BFIN_IRQ(49)	/*                           */
#define IRQ_PROG1_INTB		BFIN_IRQ(50)	/*                           */
#define IRQ_PROG2_INTA		BFIN_IRQ(51)	/*                           */
#define IRQ_PROG2_INTB		BFIN_IRQ(52)	/*                           */
#define IRQ_DMA1_WRRD0		BFIN_IRQ(53)	/*                        */
#define IRQ_DMA_WRRD0		IRQ_DMA1_WRRD0	/*                        */
#define IRQ_MEM_DMA0		IRQ_DMA1_WRRD0
#define IRQ_DMA1_WRRD1		BFIN_IRQ(54)	/*                        */
#define IRQ_DMA_WRRD1		IRQ_DMA1_WRRD1	/*                        */
#define IRQ_MEM_DMA1		IRQ_DMA1_WRRD1
#define IRQ_DMA2_WRRD0		BFIN_IRQ(55)	/*                        */
#define IRQ_MEM_DMA2		IRQ_DMA2_WRRD0
#define IRQ_DMA2_WRRD1		BFIN_IRQ(56)	/*                        */
#define IRQ_MEM_DMA3		IRQ_DMA2_WRRD1
#define IRQ_IMDMA_WRRD0		BFIN_IRQ(57)	/*                        */
#define IRQ_IMEM_DMA0		IRQ_IMDMA_WRRD0
#define IRQ_IMDMA_WRRD1		BFIN_IRQ(58)	/*                        */
#define IRQ_IMEM_DMA1		IRQ_IMDMA_WRRD1
#define IRQ_WATCH		BFIN_IRQ(59)	/*                 */
#define IRQ_RESERVED_1		BFIN_IRQ(60)	/*                    */
#define IRQ_RESERVED_2		BFIN_IRQ(61)	/*                    */
#define IRQ_SUPPLE_0		BFIN_IRQ(62)	/*                          */
#define IRQ_SUPPLE_1		BFIN_IRQ(63)	/*                          */

#define SYS_IRQS		71

#define IRQ_PF0			73
#define IRQ_PF1			74
#define IRQ_PF2			75
#define IRQ_PF3			76
#define IRQ_PF4			77
#define IRQ_PF5			78
#define IRQ_PF6			79
#define IRQ_PF7			80
#define IRQ_PF8			81
#define IRQ_PF9			82
#define IRQ_PF10		83
#define IRQ_PF11		84
#define IRQ_PF12		85
#define IRQ_PF13		86
#define IRQ_PF14		87
#define IRQ_PF15		88
#define IRQ_PF16		89
#define IRQ_PF17		90
#define IRQ_PF18		91
#define IRQ_PF19		92
#define IRQ_PF20		93
#define IRQ_PF21		94
#define IRQ_PF22		95
#define IRQ_PF23		96
#define IRQ_PF24		97
#define IRQ_PF25		98
#define IRQ_PF26		99
#define IRQ_PF27		100
#define IRQ_PF28		101
#define IRQ_PF29		102
#define IRQ_PF30		103
#define IRQ_PF31		104
#define IRQ_PF32		105
#define IRQ_PF33		106
#define IRQ_PF34		107
#define IRQ_PF35		108
#define IRQ_PF36		109
#define IRQ_PF37		110
#define IRQ_PF38		111
#define IRQ_PF39		112
#define IRQ_PF40		113
#define IRQ_PF41		114
#define IRQ_PF42		115
#define IRQ_PF43		116
#define IRQ_PF44		117
#define IRQ_PF45		118
#define IRQ_PF46		119
#define IRQ_PF47		120

#define GPIO_IRQ_BASE		IRQ_PF0

#define NR_MACH_IRQS		(IRQ_PF47 + 1)

/*                 */
#define IRQ_PLL_WAKEUP_POS	0
#define IRQ_DMA1_ERROR_POS	4
#define IRQ_DMA2_ERROR_POS	8
#define IRQ_IMDMA_ERROR_POS	12
#define IRQ_PPI0_ERROR_POS	16
#define IRQ_PPI1_ERROR_POS	20
#define IRQ_SPORT0_ERROR_POS	24
#define IRQ_SPORT1_ERROR_POS	28

/*                 */
#define IRQ_SPI_ERROR_POS	0
#define IRQ_UART_ERROR_POS	4
#define IRQ_RESERVED_ERROR_POS	8
#define IRQ_DMA1_0_POS		12
#define IRQ_DMA1_1_POS		16
#define IRQ_DMA1_2_POS		20
#define IRQ_DMA1_3_POS		24
#define IRQ_DMA1_4_POS		28

/*                 */
#define IRQ_DMA1_5_POS		0
#define IRQ_DMA1_6_POS		4
#define IRQ_DMA1_7_POS		8
#define IRQ_DMA1_8_POS		12
#define IRQ_DMA1_9_POS		16
#define IRQ_DMA1_10_POS		20
#define IRQ_DMA1_11_POS		24
#define IRQ_DMA2_0_POS		28

/*                 */
#define IRQ_DMA2_1_POS		0
#define IRQ_DMA2_2_POS		4
#define IRQ_DMA2_3_POS		8
#define IRQ_DMA2_4_POS		12
#define IRQ_DMA2_5_POS		16
#define IRQ_DMA2_6_POS		20
#define IRQ_DMA2_7_POS		24
#define IRQ_DMA2_8_POS		28

/*                 */
#define IRQ_DMA2_9_POS		0
#define IRQ_DMA2_10_POS		4
#define IRQ_DMA2_11_POS		8
#define IRQ_TIMER0_POS		12
#define IRQ_TIMER1_POS		16
#define IRQ_TIMER2_POS		20
#define IRQ_TIMER3_POS		24
#define IRQ_TIMER4_POS		28

/*                 */
#define IRQ_TIMER5_POS		0
#define IRQ_TIMER6_POS		4
#define IRQ_TIMER7_POS		8
#define IRQ_TIMER8_POS		12
#define IRQ_TIMER9_POS		16
#define IRQ_TIMER10_POS		20
#define IRQ_TIMER11_POS		24
#define IRQ_PROG0_INTA_POS	28

/*                 */
#define IRQ_PROG0_INTB_POS	0
#define IRQ_PROG1_INTA_POS	4
#define IRQ_PROG1_INTB_POS	8
#define IRQ_PROG2_INTA_POS	12
#define IRQ_PROG2_INTB_POS	16
#define IRQ_DMA1_WRRD0_POS	20
#define IRQ_DMA1_WRRD1_POS	24
#define IRQ_DMA2_WRRD0_POS	28

/*                 */
#define IRQ_DMA2_WRRD1_POS	0
#define IRQ_IMDMA_WRRD0_POS	4
#define IRQ_IMDMA_WRRD1_POS	8
#define IRQ_WDTIMER_POS		12
#define IRQ_RESERVED_1_POS	16
#define IRQ_RESERVED_2_POS	20
#define IRQ_SUPPLE_0_POS	24
#define IRQ_SUPPLE_1_POS	28

#endif
