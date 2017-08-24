/*
 * Copyright 2010 Tilera Corporation. All Rights Reserved.
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License
 *   as published by the Free Software Foundation, version 2.
 *
 *   This program is distributed in the hope that it will be useful, but
 *   WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, GOOD TITLE or
 *   NON INFRINGEMENT.  See the GNU General Public License for
 *   more details.
 * Arch specific extensions to struct device
 */

#ifndef _ASM_TILE_DEVICE_H
#define _ASM_TILE_DEVICE_H

struct dev_archdata {
	/*                               */
        struct dma_map_ops	*dma_ops;

	/*                                        */
	dma_addr_t		dma_offset;

	/*                                                           */
	dma_addr_t		max_direct_dma_addr;
};

struct pdev_archdata {
};

#endif /*                    */
