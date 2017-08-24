#include <linux/compiler.h>
#include <linux/mm.h>
#include <linux/signal.h>
#include <linux/smp.h>

#include <asm/asm.h>
#include <asm/bootinfo.h>
#include <asm/byteorder.h>
#include <asm/cpu.h>
#include <asm/inst.h>
#include <asm/processor.h>
#include <asm/uaccess.h>
#include <asm/branch.h>
#include <asm/mipsregs.h>
#include <asm/cacheflush.h>

#include <asm/fpu_emulator.h>

#include "ieee754.h"

/*                                                  */

#ifdef __mips
#undef __mips
#endif
#define __mips 4

/*
                                                                        
                                                                      
                                            
  
                         
                                  
                                     
                                                                  
                                                    
 */

/*
                                                           
                                                  
 */

struct emuframe {
	mips_instruction	emul;
	mips_instruction	badinst;
	mips_instruction	cookie;
	unsigned long		epc;
};

int mips_dsemul(struct pt_regs *regs, mips_instruction ir, unsigned long cpc)
{
	extern asmlinkage void handle_dsemulret(void);
	struct emuframe __user *fr;
	int err;

	if ((get_isa16_mode(regs->cp0_epc) && ((ir >> 16) == MM_NOP16)) ||
		(ir == 0)) {
		/*             */
		regs->cp0_epc = cpc;
		regs->cp0_cause &= ~CAUSEF_BD;
		return 0;
	}
#ifdef DSEMUL_TRACE
	printk("dsemul %lx %lx\n", regs->cp0_epc, cpc);

#endif

	/*
                                                               
                                                          
                                                        
                            
   
                                                    
                                                      
                                                        
                                                    
                                                             
   
                                                         
                                                                 
                                                               
                                 
  */

	/*                                                             */
	fr = (struct emuframe __user *)
		((regs->regs[29] - sizeof(struct emuframe)) & ~0x7);

	/*                                                       */
	if (unlikely(!access_ok(VERIFY_WRITE, fr, sizeof(struct emuframe))))
		return SIGBUS;

	if (get_isa16_mode(regs->cp0_epc)) {
		err = __put_user(ir >> 16, (u16 __user *)(&fr->emul));
		err |= __put_user(ir & 0xffff, (u16 __user *)((long)(&fr->emul) + 2));
		err |= __put_user(BREAK_MATH >> 16, (u16 __user *)(&fr->badinst));
		err |= __put_user(BREAK_MATH & 0xffff, (u16 __user *)((long)(&fr->badinst) + 2));
	} else {
		err = __put_user(ir, &fr->emul);
		err |= __put_user((mips_instruction)BREAK_MATH, &fr->badinst);
	}

	err |= __put_user((mips_instruction)BD_COOKIE, &fr->cookie);
	err |= __put_user(cpc, &fr->epc);

	if (unlikely(err)) {
		MIPS_FPU_EMU_INC_STATS(errors);
		return SIGBUS;
	}

	regs->cp0_epc = ((unsigned long) &fr->emul) |
		get_isa16_mode(regs->cp0_epc);

	flush_cache_sigtramp((unsigned long)&fr->badinst);

	return SIGILL;		/*                             */
}

int do_dsemulret(struct pt_regs *xcp)
{
	struct emuframe __user *fr;
	unsigned long epc;
	u32 insn, cookie;
	int err = 0;
	u16 instr[2];

	fr = (struct emuframe __user *)
		(msk_isa16_mode(xcp->cp0_epc) - sizeof(mips_instruction));

	/*
                                                                        
                                      
  */
	if (!access_ok(VERIFY_READ, fr, sizeof(struct emuframe)))
		return 0;

	/*
                                              
   
                                                              
                                                  
  */
	if (get_isa16_mode(xcp->cp0_epc)) {
		err = __get_user(instr[0], (u16 __user *)(&fr->badinst));
		err |= __get_user(instr[1], (u16 __user *)((long)(&fr->badinst) + 2));
		insn = (instr[0] << 16) | instr[1];
	} else {
		err = __get_user(insn, &fr->badinst);
	}
	err |= __get_user(cookie, &fr->cookie);

	if (unlikely(err || (insn != BREAK_MATH) || (cookie != BD_COOKIE))) {
		MIPS_FPU_EMU_INC_STATS(errors);
		return 0;
	}

	/*
                                                                        
                                                                
                                                                     
                                                              
                                                                      
                                                                   
                                           
  */

#ifdef DSEMUL_TRACE
	printk("dsemulret\n");
#endif
	if (__get_user(epc, &fr->epc)) {		/*           */
		/*                                       */
		force_sig(SIGBUS, current);

		return 0;
	}

	/*                                              */
	xcp->cp0_epc = epc;

	return 1;
}
