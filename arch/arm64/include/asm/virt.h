/*
 * Copyright (C) 2012 ARM Ltd.
 * Author: Marc Zyngier <marc.zyngier@arm.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __ASM__VIRT_H
#define __ASM__VIRT_H

#define BOOT_CPU_MODE_EL2	(0x0e12b007)

#ifndef __ASSEMBLY__
#include <asm/cacheflush.h>

/*
                                                         
                                                                           
                                                                      
                                                                              
  
                                                                           
                                                                             
 */
extern u32 __boot_cpu_mode[2];

void __hyp_set_vectors(phys_addr_t phys_vector_base);
phys_addr_t __hyp_get_vectors(void);

static inline void sync_boot_mode(void)
{
	/*
                                                                    
                                                                       
                    
  */
	__flush_dcache_area(__boot_cpu_mode, sizeof(__boot_cpu_mode));
}

/*                                      */
static inline bool is_hyp_mode_available(void)
{
	sync_boot_mode();
	return (__boot_cpu_mode[0] == BOOT_CPU_MODE_EL2 &&
		__boot_cpu_mode[1] == BOOT_CPU_MODE_EL2);
}

/*                                                            */
static inline bool is_hyp_mode_mismatched(void)
{
	sync_boot_mode();
	return __boot_cpu_mode[0] != __boot_cpu_mode[1];
}

#endif /*              */

#endif /*                 */
