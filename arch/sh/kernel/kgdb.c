/*
 * SuperH KGDB support
 *
 * Copyright (C) 2008 - 2012  Paul Mundt
 *
 * Single stepping taken from the old stub by Henry Bell and Jeremy Siegel.
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */
#include <linux/kgdb.h>
#include <linux/kdebug.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/sched.h>
#include <asm/cacheflush.h>
#include <asm/traps.h>

/*                                                   */
#define OPCODE_BT(op)		(((op) & 0xff00) == 0x8900)
#define OPCODE_BF(op)		(((op) & 0xff00) == 0x8b00)
#define OPCODE_BTF_DISP(op)	(((op) & 0x80) ? (((op) | 0xffffff80) << 1) : \
				 (((op) & 0x7f ) << 1))
#define OPCODE_BFS(op)		(((op) & 0xff00) == 0x8f00)
#define OPCODE_BTS(op)		(((op) & 0xff00) == 0x8d00)
#define OPCODE_BRA(op)		(((op) & 0xf000) == 0xa000)
#define OPCODE_BRA_DISP(op)	(((op) & 0x800) ? (((op) | 0xfffff800) << 1) : \
				 (((op) & 0x7ff) << 1))
#define OPCODE_BRAF(op)		(((op) & 0xf0ff) == 0x0023)
#define OPCODE_BRAF_REG(op)	(((op) & 0x0f00) >> 8)
#define OPCODE_BSR(op)		(((op) & 0xf000) == 0xb000)
#define OPCODE_BSR_DISP(op)	(((op) & 0x800) ? (((op) | 0xfffff800) << 1) : \
				 (((op) & 0x7ff) << 1))
#define OPCODE_BSRF(op)		(((op) & 0xf0ff) == 0x0003)
#define OPCODE_BSRF_REG(op)	(((op) >> 8) & 0xf)
#define OPCODE_JMP(op)		(((op) & 0xf0ff) == 0x402b)
#define OPCODE_JMP_REG(op)	(((op) >> 8) & 0xf)
#define OPCODE_JSR(op)		(((op) & 0xf0ff) == 0x400b)
#define OPCODE_JSR_REG(op)	(((op) >> 8) & 0xf)
#define OPCODE_RTS(op)		((op) == 0xb)
#define OPCODE_RTE(op)		((op) == 0x2b)

#define SR_T_BIT_MASK           0x1
#define STEP_OPCODE             0xc33d

/*                                            */
static short *get_step_address(struct pt_regs *linux_regs)
{
	insn_size_t op = __raw_readw(linux_regs->pc);
	long addr;

	/*    */
	if (OPCODE_BT(op)) {
		if (linux_regs->sr & SR_T_BIT_MASK)
			addr = linux_regs->pc + 4 + OPCODE_BTF_DISP(op);
		else
			addr = linux_regs->pc + 2;
	}

	/*     */
	else if (OPCODE_BTS(op)) {
		if (linux_regs->sr & SR_T_BIT_MASK)
			addr = linux_regs->pc + 4 + OPCODE_BTF_DISP(op);
		else
			addr = linux_regs->pc + 4;	/*                   */
	}

	/*    */
	else if (OPCODE_BF(op)) {
		if (!(linux_regs->sr & SR_T_BIT_MASK))
			addr = linux_regs->pc + 4 + OPCODE_BTF_DISP(op);
		else
			addr = linux_regs->pc + 2;
	}

	/*     */
	else if (OPCODE_BFS(op)) {
		if (!(linux_regs->sr & SR_T_BIT_MASK))
			addr = linux_regs->pc + 4 + OPCODE_BTF_DISP(op);
		else
			addr = linux_regs->pc + 4;	/*                   */
	}

	/*     */
	else if (OPCODE_BRA(op))
		addr = linux_regs->pc + 4 + OPCODE_BRA_DISP(op);

	/*      */
	else if (OPCODE_BRAF(op))
		addr = linux_regs->pc + 4
		    + linux_regs->regs[OPCODE_BRAF_REG(op)];

	/*     */
	else if (OPCODE_BSR(op))
		addr = linux_regs->pc + 4 + OPCODE_BSR_DISP(op);

	/*      */
	else if (OPCODE_BSRF(op))
		addr = linux_regs->pc + 4
		    + linux_regs->regs[OPCODE_BSRF_REG(op)];

	/*     */
	else if (OPCODE_JMP(op))
		addr = linux_regs->regs[OPCODE_JMP_REG(op)];

	/*     */
	else if (OPCODE_JSR(op))
		addr = linux_regs->regs[OPCODE_JSR_REG(op)];

	/*     */
	else if (OPCODE_RTS(op))
		addr = linux_regs->pr;

	/*     */
	else if (OPCODE_RTE(op))
		addr = linux_regs->regs[15];

	/*       */
	else
		addr = linux_regs->pc + instruction_size(op);

	flush_icache_range(addr, addr + instruction_size(op));
	return (short *)addr;
}

/*
                                                                    
                                                                       
                                                                         
                                                                      
                                                                   
                                                  
 */

static unsigned long stepped_address;
static insn_size_t stepped_opcode;

static void do_single_step(struct pt_regs *linux_regs)
{
	/*                                                        */
	unsigned short *addr = get_step_address(linux_regs);

	stepped_address = (int)addr;

	/*            */
	stepped_opcode = __raw_readw((long)addr);
	*addr = STEP_OPCODE;

	/*                  */
	flush_icache_range((long)addr, (long)addr +
			   instruction_size(stepped_opcode));
}

/*                    */
static void undo_single_step(struct pt_regs *linux_regs)
{
	/*                                                  */
	/*                                                  */
	if (stepped_opcode != 0) {
		__raw_writew(stepped_opcode, stepped_address);
		flush_icache_range(stepped_address, stepped_address + 2);
	}

	stepped_opcode = 0;
}

struct dbg_reg_def_t dbg_reg_def[DBG_MAX_REG_NUM] = {
	{ "r0",		GDB_SIZEOF_REG, offsetof(struct pt_regs, regs[0]) },
	{ "r1",		GDB_SIZEOF_REG, offsetof(struct pt_regs, regs[1]) },
	{ "r2",		GDB_SIZEOF_REG, offsetof(struct pt_regs, regs[2]) },
	{ "r3",		GDB_SIZEOF_REG, offsetof(struct pt_regs, regs[3]) },
	{ "r4",		GDB_SIZEOF_REG, offsetof(struct pt_regs, regs[4]) },
	{ "r5",		GDB_SIZEOF_REG, offsetof(struct pt_regs, regs[5]) },
	{ "r6",		GDB_SIZEOF_REG, offsetof(struct pt_regs, regs[6]) },
	{ "r7",		GDB_SIZEOF_REG, offsetof(struct pt_regs, regs[7]) },
	{ "r8",		GDB_SIZEOF_REG, offsetof(struct pt_regs, regs[8]) },
	{ "r9",		GDB_SIZEOF_REG, offsetof(struct pt_regs, regs[9]) },
	{ "r10",	GDB_SIZEOF_REG, offsetof(struct pt_regs, regs[10]) },
	{ "r11",	GDB_SIZEOF_REG, offsetof(struct pt_regs, regs[11]) },
	{ "r12",	GDB_SIZEOF_REG, offsetof(struct pt_regs, regs[12]) },
	{ "r13",	GDB_SIZEOF_REG, offsetof(struct pt_regs, regs[13]) },
	{ "r14",	GDB_SIZEOF_REG, offsetof(struct pt_regs, regs[14]) },
	{ "r15",	GDB_SIZEOF_REG, offsetof(struct pt_regs, regs[15]) },
	{ "pc",		GDB_SIZEOF_REG, offsetof(struct pt_regs, pc) },
	{ "pr",		GDB_SIZEOF_REG, offsetof(struct pt_regs, pr) },
	{ "sr",		GDB_SIZEOF_REG, offsetof(struct pt_regs, sr) },
	{ "gbr",	GDB_SIZEOF_REG, offsetof(struct pt_regs, gbr) },
	{ "mach",	GDB_SIZEOF_REG, offsetof(struct pt_regs, mach) },
	{ "macl",	GDB_SIZEOF_REG, offsetof(struct pt_regs, macl) },
	{ "vbr",	GDB_SIZEOF_REG, -1 },
};

int dbg_set_reg(int regno, void *mem, struct pt_regs *regs)
{
	if (regno < 0 || regno >= DBG_MAX_REG_NUM)
		return -EINVAL;

	if (dbg_reg_def[regno].offset != -1)
		memcpy((void *)regs + dbg_reg_def[regno].offset, mem,
		       dbg_reg_def[regno].size);

	return 0;
}

char *dbg_get_reg(int regno, void *mem, struct pt_regs *regs)
{
	if (regno >= DBG_MAX_REG_NUM || regno < 0)
		return NULL;

	if (dbg_reg_def[regno].size != -1)
		memcpy(mem, (void *)regs + dbg_reg_def[regno].offset,
		       dbg_reg_def[regno].size);

	switch (regno) {
	case GDB_VBR:
		__asm__ __volatile__ ("stc vbr, %0" : "=r" (mem));
		break;
	}

	return dbg_reg_def[regno].name;
}

void sleeping_thread_to_gdb_regs(unsigned long *gdb_regs, struct task_struct *p)
{
	struct pt_regs *thread_regs = task_pt_regs(p);
	int reg;

	/*                    */
	for (reg = 0; reg < DBG_MAX_REG_NUM; reg++)
		gdb_regs[reg] = 0;

	/*
                             
   
                                                                 
                                                                
                                        
  */
	for (reg = GDB_R8; reg < GDB_R15; reg++)
		gdb_regs[reg] = thread_regs->regs[reg];

	gdb_regs[GDB_R15] = p->thread.sp;
	gdb_regs[GDB_PC] = p->thread.pc;

	/*
                                            
  */
	gdb_regs[GDB_PR] = thread_regs->pr;
	gdb_regs[GDB_GBR] = thread_regs->gbr;
}

int kgdb_arch_handle_exception(int e_vector, int signo, int err_code,
			       char *remcomInBuffer, char *remcomOutBuffer,
			       struct pt_regs *linux_regs)
{
	unsigned long addr;
	char *ptr;

	/*                                    */
	undo_single_step(linux_regs);

	switch (remcomInBuffer[0]) {
	case 'c':
	case 's':
		/*                                                         */
		ptr = &remcomInBuffer[1];
		if (kgdb_hex2long(&ptr, &addr))
			linux_regs->pc = addr;
	case 'D':
	case 'k':
		atomic_set(&kgdb_cpu_doing_single_step, -1);

		if (remcomInBuffer[0] == 's') {
			do_single_step(linux_regs);
			kgdb_single_step = 1;

			atomic_set(&kgdb_cpu_doing_single_step,
				   raw_smp_processor_id());
		}

		return 0;
	}

	/*                                                          */
	return -1;
}

unsigned long kgdb_arch_pc(int exception, struct pt_regs *regs)
{
	if (exception == 60)
		return instruction_pointer(regs) - 2;
	return instruction_pointer(regs);
}

void kgdb_arch_set_pc(struct pt_regs *regs, unsigned long ip)
{
	regs->pc = ip;
}

/*
                                                                  
 */
BUILD_TRAP_HANDLER(singlestep)
{
	unsigned long flags;
	TRAP_HANDLER_DECL;

	local_irq_save(flags);
	regs->pc -= instruction_size(__raw_readw(regs->pc - 4));
	kgdb_handle_exception(0, SIGTRAP, 0, regs);
	local_irq_restore(flags);
}

static void kgdb_call_nmi_hook(void *ignored)
{
	kgdb_nmicallback(raw_smp_processor_id(), get_irq_regs());
}

void kgdb_roundup_cpus(unsigned long flags)
{
	local_irq_enable();
	smp_call_function(kgdb_call_nmi_hook, NULL, 0);
	local_irq_disable();
}

static int __kgdb_notify(struct die_args *args, unsigned long cmd)
{
	int ret;

	switch (cmd) {
	case DIE_BREAKPOINT:
		/*
                                                
                                          
   */
		if (test_thread_flag(TIF_SINGLESTEP))
			return NOTIFY_DONE;

		ret = kgdb_handle_exception(args->trapnr & 0xff, args->signr,
					    args->err, args->regs);
		if (ret)
			return NOTIFY_DONE;

		break;
	}

	return NOTIFY_STOP;
}

static int
kgdb_notify(struct notifier_block *self, unsigned long cmd, void *ptr)
{
	unsigned long flags;
	int ret;

	local_irq_save(flags);
	ret = __kgdb_notify(ptr, cmd);
	local_irq_restore(flags);

	return ret;
}

static struct notifier_block kgdb_notifier = {
	.notifier_call	= kgdb_notify,

	/*
                                                               
  */
	.priority	= -INT_MAX,
};

int kgdb_arch_init(void)
{
	return register_die_notifier(&kgdb_notifier);
}

void kgdb_arch_exit(void)
{
	unregister_die_notifier(&kgdb_notifier);
}

struct kgdb_arch arch_kgdb_ops = {
	/*                                     */
#ifdef CONFIG_CPU_LITTLE_ENDIAN
	.gdb_bpt_instr		= { 0x3c, 0xc3 },
#else
	.gdb_bpt_instr		= { 0xc3, 0x3c },
#endif
};
