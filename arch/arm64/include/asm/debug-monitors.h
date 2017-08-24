/*
 * Copyright (C) 2012 ARM Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
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
#ifndef __ASM_DEBUG_MONITORS_H
#define __ASM_DEBUG_MONITORS_H

#ifdef __KERNEL__

#define	DBG_ESR_EVT(x)		(((x) >> 27) & 0x7)

/*         */
#define DBG_ESR_EVT_HWBP	0x0
#define DBG_ESR_EVT_HWSS	0x1
#define DBG_ESR_EVT_HWWP	0x2
#define DBG_ESR_EVT_BRK		0x6

/*
                                   
 */
#define BREAK_INSTR_SIZE		4

/*
                                                                   
 */
#define DBG_ESR_VAL_BRK(x)	(0xf2000000 | ((x) & 0xfffff))

/*
                                                    
                                            
                                     
                                          
 */
#define KGDB_DYN_DBG_BRK_IMM		0x400
#define KGDB_COMPILED_DBG_BRK_IMM	0x401


/*
                           
                                                                 
 */
#define AARCH64_BREAK_MON	0xd4200000

/*
                                    
 */
#define KGDB_DYN_DBG_BRK_INS_BYTE(x) \
	((((AARCH64_BREAK_MON) & 0xffe0001f) >> (x * 8)) & 0xff)

/*
                               
 */
#define KGBD_DYN_DBG_BRK_IMM_BYTE(x) \
	(((((KGDB_DYN_DBG_BRK_IMM) & 0xffff) << 5) >> (x * 8)) & 0xff)

#define KGDB_DYN_DBG_BRK_BYTE(x) \
	(KGDB_DYN_DBG_BRK_INS_BYTE(x) | KGBD_DYN_DBG_BRK_IMM_BYTE(x))

#define  KGDB_DYN_BRK_INS_BYTE0  KGDB_DYN_DBG_BRK_BYTE(0)
#define  KGDB_DYN_BRK_INS_BYTE1  KGDB_DYN_DBG_BRK_BYTE(1)
#define  KGDB_DYN_BRK_INS_BYTE2  KGDB_DYN_DBG_BRK_BYTE(2)
#define  KGDB_DYN_BRK_INS_BYTE3  KGDB_DYN_DBG_BRK_BYTE(3)

#define CACHE_FLUSH_IS_SAFE		1

enum debug_el {
	DBG_ACTIVE_EL0 = 0,
	DBG_ACTIVE_EL1,
};

/*         */
#define DBG_ESR_EVT_BKPT	0x4
#define DBG_ESR_EVT_VECC	0x5

#define AARCH32_BREAK_ARM	0x07f001f0
#define AARCH32_BREAK_THUMB	0xde01
#define AARCH32_BREAK_THUMB2_LO	0xf7f0
#define AARCH32_BREAK_THUMB2_HI	0xa000

#ifndef __ASSEMBLY__
struct task_struct;

#define DBG_ARCH_ID_RESERVED	0	/*                                */

#define DBG_HOOK_HANDLED	0
#define DBG_HOOK_ERROR		1

struct step_hook {
	struct list_head node;
	int (*fn)(struct pt_regs *regs, unsigned int esr);
};

void register_step_hook(struct step_hook *hook);
void unregister_step_hook(struct step_hook *hook);

struct break_hook {
	struct list_head node;
	u32 esr_val;
	u32 esr_mask;
	int (*fn)(struct pt_regs *regs, unsigned int esr);
};

void register_break_hook(struct break_hook *hook);
void unregister_break_hook(struct break_hook *hook);

u8 debug_monitors_arch(void);

void enable_debug_monitors(enum debug_el el);
void disable_debug_monitors(enum debug_el el);

void user_rewind_single_step(struct task_struct *task);
void user_fastforward_single_step(struct task_struct *task);

void kernel_enable_single_step(struct pt_regs *regs);
void kernel_disable_single_step(void);
int kernel_active_single_step(void);

#ifdef CONFIG_HAVE_HW_BREAKPOINT
int reinstall_suspended_bps(struct pt_regs *regs);
#else
static inline int reinstall_suspended_bps(struct pt_regs *regs)
{
	return -ENODEV;
}
#endif

#ifdef CONFIG_COMPAT
int aarch32_break_handler(struct pt_regs *regs);
#else
static int aarch32_break_handler(struct pt_regs *regs)
{
	return -EFAULT;
}
#endif

#endif	/*            */
#endif	/*            */
#endif	/*                        */
