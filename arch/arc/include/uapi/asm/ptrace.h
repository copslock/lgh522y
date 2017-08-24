/*
 * Copyright (C) 2004, 2007-2010, 2011-2012 Synopsys, Inc. (www.synopsys.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Amit Bhor, Sameer Dhavale: Codito Technologies 2004
 */

#ifndef _UAPI__ASM_ARC_PTRACE_H
#define _UAPI__ASM_ARC_PTRACE_H

#define PTRACE_GET_THREAD_AREA	25

#ifndef __ASSEMBLY__
/*
                                          
                       
                                          
  
                                                                           
                         
                                                                           
                                                               
  
                                                                              
                                    
*/
struct user_regs_struct {

	struct {
		long pad;
		long bta, lp_start, lp_end, lp_count;
		long status32, ret, blink, fp, gp;
		long r12, r11, r10, r9, r8, r7, r6, r5, r4, r3, r2, r1, r0;
		long sp;
	} scratch;
	struct {
		long pad;
		long r25, r24, r23, r22, r21, r20;
		long r19, r18, r17, r16, r15, r14, r13;
	} callee;
	long efa;	/*                                                */
	long stop_pc;	/*                                                  */
};
#endif /*               */

#endif /*                         */
