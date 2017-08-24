#ifndef _ASM_M32R_SIGNAL_H
#define _ASM_M32R_SIGNAL_H

#include <uapi/asm/signal.h>

/*                                                                     
                                 */

#define _NSIG		64
#define _NSIG_BPW	32
#define _NSIG_WORDS	(_NSIG / _NSIG_BPW)

typedef unsigned long old_sigset_t;		/*                  */

typedef struct {
	unsigned long sig[_NSIG_WORDS];
} sigset_t;

#define __ARCH_HAS_SA_RESTORER
#include <asm/sigcontext.h>

#undef __HAVE_ARCH_SIG_BITOPS

#endif  /*                    */
