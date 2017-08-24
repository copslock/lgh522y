/*
 * arch/arm/mach-at91/include/mach/at91_st.h
 *
 * Copyright (C) 2005 Ivan Kokshaysky
 * Copyright (C) SAN People
 *
 * System Timer (ST) - System peripherals registers.
 * Based on AT91RM9200 datasheet revision E.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef AT91_ST_H
#define AT91_ST_H

#ifndef __ASSEMBLY__
extern void __iomem *at91_st_base;

#define at91_st_read(field) \
	__raw_readl(at91_st_base + field)

#define at91_st_write(field, value) \
	__raw_writel(value, at91_st_base + field)
#else
.extern at91_st_base
#endif

#define	AT91_ST_CR		0x00			/*                  */
#define 	AT91_ST_WDRST		(1 << 0)		/*                        */

#define	AT91_ST_PIMR		0x04			/*                               */
#define		AT91_ST_PIV		(0xffff <<  0)		/*                       */

#define	AT91_ST_WDMR		0x08			/*                        */
#define		AT91_ST_WDV		(0xffff <<  0)		/*                        */
#define		AT91_ST_RSTEN		(1	<< 16)		/*              */
#define		AT91_ST_EXTEN		(1	<< 17)		/*                                  */

#define	AT91_ST_RTMR		0x0c			/*                         */
#define		AT91_ST_RTPRES		(0xffff <<  0)		/*                           */

#define	AT91_ST_SR		0x10			/*                 */
#define		AT91_ST_PITS		(1 << 0)		/*                              */
#define		AT91_ST_WDOVF		(1 << 1) 		/*                   */
#define		AT91_ST_RTTINC		(1 << 2) 		/*                           */
#define		AT91_ST_ALMS		(1 << 3) 		/*              */

#define	AT91_ST_IER		0x14			/*                           */
#define	AT91_ST_IDR		0x18			/*                            */
#define	AT91_ST_IMR		0x1c			/*                         */

#define	AT91_ST_RTAR		0x20			/*                          */
#define		AT91_ST_ALMV		(0xfffff << 0)		/*             */

#define	AT91_ST_CRTR		0x24			/*                            */
#define		AT91_ST_CRTV		(0xfffff << 0)		/*                         */

#endif
