/* linux/arch/arm/plat-samsung/include/plat/irqs.h
 *
 * Copyright (c) 2009 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * S5P Common IRQ support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __PLAT_SAMSUNG_IRQS_H
#define __PLAT_SAMSUNG_IRQS_H __FILE__

/*                                                      
                                                      
                                                          
                           
  
                                                        
                                                  
 */

#define S5P_IRQ_OFFSET		(32)

#define S5P_IRQ(x)		((x) + S5P_IRQ_OFFSET)

#define S5P_VIC0_BASE		S5P_IRQ(0)
#define S5P_VIC1_BASE		S5P_IRQ(32)
#define S5P_VIC2_BASE		S5P_IRQ(64)
#define S5P_VIC3_BASE		S5P_IRQ(96)

#define VIC_BASE(x)		(S5P_VIC0_BASE + ((x)*32))

#define IRQ_VIC0_BASE		S5P_VIC0_BASE
#define IRQ_VIC1_BASE		S5P_VIC1_BASE
#define IRQ_VIC2_BASE		S5P_VIC2_BASE

/*                */

#define S5P_IRQ_VIC0(x)		(S5P_VIC0_BASE + (x))
#define S5P_IRQ_VIC1(x)		(S5P_VIC1_BASE + (x))
#define S5P_IRQ_VIC2(x)		(S5P_VIC2_BASE + (x))
#define S5P_IRQ_VIC3(x)		(S5P_VIC3_BASE + (x))

#define S5P_TIMER_IRQ(x)	(IRQ_TIMER_BASE + (x))

#define IRQ_TIMER0		S5P_TIMER_IRQ(0)
#define IRQ_TIMER1		S5P_TIMER_IRQ(1)
#define IRQ_TIMER2		S5P_TIMER_IRQ(2)
#define IRQ_TIMER3		S5P_TIMER_IRQ(3)
#define IRQ_TIMER4		S5P_TIMER_IRQ(4)
#define IRQ_TIMER_COUNT		(5)

#define IRQ_EINT(x)		((x) < 16 ? ((x) + S5P_EINT_BASE1) \
					: ((x) - 16 + S5P_EINT_BASE2))

#define EINT_OFFSET(irq)	((irq) < S5P_EINT_BASE2 ? \
						((irq) - S5P_EINT_BASE1) : \
						((irq) + 16 - S5P_EINT_BASE2))

#define IRQ_EINT_BIT(x)		EINT_OFFSET(x)

/*                                                                
                                                               
                                                           
                                                                      
                                                                    */
#define S5P_GPIOINT_GROUP_COUNT 4
#define S5P_GPIOINT_GROUP_SIZE	8
#define S5P_GPIOINT_COUNT	(S5P_GPIOINT_GROUP_COUNT * S5P_GPIOINT_GROUP_SIZE)

/*                                        */
#define S5P_IRQ_TYPE_LEVEL_LOW		(0x00)
#define S5P_IRQ_TYPE_LEVEL_HIGH		(0x01)
#define S5P_IRQ_TYPE_EDGE_FALLING	(0x02)
#define S5P_IRQ_TYPE_EDGE_RISING	(0x03)
#define S5P_IRQ_TYPE_EDGE_BOTH		(0x04)

#endif /*                       */
