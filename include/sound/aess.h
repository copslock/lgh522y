/*
 * AESS IP block reset
 *
 * Copyright (C) 2012 Texas Instruments, Inc.
 * Paul Walmsley
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */
#ifndef __SOUND_AESS_H__
#define __SOUND_AESS_H__

#include <linux/kernel.h>
#include <linux/io.h>

/*
                                                                 
                                                                      
                   
 */
#define AESS_AUTO_GATING_ENABLE_OFFSET			0x07c

/*                                                               */
#define AESS_AUTO_GATING_ENABLE_SHIFT			0

/* 
                                                           
                           
  
                                                                   
                                                         
 */
static inline void aess_enable_autogating(void __iomem *base)
{
	u32 v;

	/*                                                           */
	v = 1 << AESS_AUTO_GATING_ENABLE_SHIFT;
	writel(v, base + AESS_AUTO_GATING_ENABLE_OFFSET);
}

#endif /*                  */
