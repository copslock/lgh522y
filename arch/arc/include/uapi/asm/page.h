/*
 * Copyright (C) 2004, 2007-2010, 2011-2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _UAPI__ASM_ARC_PAGE_H
#define _UAPI__ASM_ARC_PAGE_H

/*                                     */
#if defined(CONFIG_ARC_PAGE_SIZE_16K)
#define PAGE_SHIFT 14
#elif defined(CONFIG_ARC_PAGE_SIZE_4K)
#define PAGE_SHIFT 12
#else
/*
             
                                                                         
                                                                       
                                                                       
                
 */
#define PAGE_SHIFT 13
#endif

#ifdef __ASSEMBLY__
#define PAGE_SIZE	(1 << PAGE_SHIFT)
#define PAGE_OFFSET	(0x80000000)
#else
#define PAGE_SIZE	(1UL << PAGE_SHIFT)	/*            */
#define PAGE_OFFSET	(0x80000000UL)	/*                             */
#endif

#define PAGE_MASK	(~(PAGE_SIZE-1))


#endif /*                       */
