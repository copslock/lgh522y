/*
 * NUMA memory policies for Linux.
 * Copyright 2003,2004 Andi Kleen SuSE Labs
 */
#ifndef _UAPI_LINUX_MEMPOLICY_H
#define _UAPI_LINUX_MEMPOLICY_H

#include <linux/errno.h>


/*
                                                                          
                                                                              
                                                                             
 */

/*          */
enum {
	MPOL_DEFAULT,
	MPOL_PREFERRED,
	MPOL_BIND,
	MPOL_INTERLEAVE,
	MPOL_LOCAL,
	MPOL_MAX,	/*                            */
};

enum mpol_rebind_step {
	MPOL_REBIND_ONCE,	/*                                         */
	MPOL_REBIND_STEP1,	/*                                     */
	MPOL_REBIND_STEP2,	/*                                            */
	MPOL_REBIND_NSTEP,
};

/*                         */
#define MPOL_F_STATIC_NODES	(1 << 15)
#define MPOL_F_RELATIVE_NODES	(1 << 14)

/*
                                                                             
                                     
 */
#define MPOL_MODE_FLAGS	(MPOL_F_STATIC_NODES | MPOL_F_RELATIVE_NODES)

/*                         */
#define MPOL_F_NODE	(1<<0)	/*                                          */
#define MPOL_F_ADDR	(1<<1)	/*                           */
#define MPOL_F_MEMS_ALLOWED (1<<2) /*                         */

/*                 */
#define MPOL_MF_STRICT	(1<<0)	/*                                      */
#define MPOL_MF_MOVE	 (1<<1)	/*                                            
                 */
#define MPOL_MF_MOVE_ALL (1<<2)	/*                                      */
#define MPOL_MF_LAZY	 (1<<3)	/*                                         */
#define MPOL_MF_INTERNAL (1<<4)	/*                           */

#define MPOL_MF_VALID	(MPOL_MF_STRICT   | 	\
			 MPOL_MF_MOVE     | 	\
			 MPOL_MF_MOVE_ALL)

/*
                                                                 
                                                                  
                                                            
 */
#define MPOL_F_SHARED  (1 << 0)	/*                          */
#define MPOL_F_LOCAL   (1 << 1)	/*                            */
#define MPOL_F_REBINDING (1 << 2)	/*                                */
#define MPOL_F_MOF	(1 << 3) /*                                    */
#define MPOL_F_MORON	(1 << 4) /*                                       */


#endif /*                         */
