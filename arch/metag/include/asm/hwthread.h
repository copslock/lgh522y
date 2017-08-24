/*
 * Copyright (C) 2008 Imagination Technologies
 */
#ifndef __METAG_HWTHREAD_H
#define __METAG_HWTHREAD_H

#include <linux/bug.h>
#include <linux/io.h>

#include <asm/metag_mem.h>

#define BAD_HWTHREAD_ID		(0xFFU)
#define BAD_CPU_ID		(0xFFU)

extern u8 cpu_2_hwthread_id[];
extern u8 hwthread_id_2_cpu[];

/*
                                                                  
                                                              
  
                                                                   
                               
 */
static inline
void __iomem *__CU_addr(unsigned int thread, unsigned int regnum)
{
	unsigned int base, thread_offset, thread_regnum;

	WARN_ON(thread == BAD_HWTHREAD_ID);

	base = T0UCTREG0;	/*                   */

	thread_offset = TnUCTRX_STRIDE * thread;
	thread_regnum = TXUCTREGn_STRIDE * regnum;

	return (void __iomem *)(base + thread_offset + thread_regnum);
}

#endif /*                    */
