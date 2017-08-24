/*
 * Copyright 2003-2011 NetLogic Microsystems, Inc. (NetLogic). All rights
 * reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the NetLogic
 * license below:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY NETLOGIC ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL NETLOGIC OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/threads.h>

#include <asm/asm.h>
#include <asm/asm-offsets.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <asm/string.h>

#include <asm/netlogic/haldefs.h>
#include <asm/netlogic/common.h>
#include <asm/netlogic/mips-extns.h>

#include <asm/netlogic/xlp-hal/iomap.h>
#include <asm/netlogic/xlp-hal/pic.h>
#include <asm/netlogic/xlp-hal/xlp.h>
#include <asm/netlogic/xlp-hal/sys.h>

static int xlp_wakeup_core(uint64_t sysbase, int node, int core)
{
	uint32_t coremask, value;
	int count;

	coremask = (1 << core);

	/*                  */
	value = nlm_read_sys_reg(sysbase, SYS_CORE_DFS_DIS_CTRL);
	value &= ~coremask;
	nlm_write_sys_reg(sysbase, SYS_CORE_DFS_DIS_CTRL, value);

	/*                  */
	value = nlm_read_sys_reg(sysbase, SYS_CPU_RESET);
	value &= ~coremask;
	nlm_write_sys_reg(sysbase, SYS_CPU_RESET, value);

	/*                                      */
	count = 100000;
	do {
		value = nlm_read_sys_reg(sysbase, SYS_CPU_NONCOHERENT_MODE);
	} while ((value & coremask) != 0 && --count > 0);

	return count != 0;
}

static void xlp_enable_secondary_cores(const cpumask_t *wakeup_mask)
{
	struct nlm_soc_info *nodep;
	uint64_t syspcibase;
	uint32_t syscoremask;
	int core, n, cpu, count, val;

	for (n = 0; n < NLM_NR_NODES; n++) {
		syspcibase = nlm_get_sys_pcibase(n);
		if (nlm_read_reg(syspcibase, 0) == 0xffffffff)
			break;

		/*                              */
		if (n != 0)
			nlm_node_init(n);
		nodep = nlm_get_node(n);
		syscoremask = nlm_read_sys_reg(nodep->sysbase, SYS_CPU_RESET);
		/*              */
		if (n == 0) {
			syscoremask |= 1;
			nodep->coremask = 1;
		}

		for (core = 0; core < NLM_CORES_PER_NODE; core++) {
			/*                             */
			if (n == 0 && core == 0)
				continue;

			/*                        */
			if ((syscoremask & (1 << core)) == 0)
				continue;

			/*                                                */
			cpu = (n * NLM_CORES_PER_NODE + core)
						* NLM_THREADS_PER_CORE;
			if (!cpumask_test_cpu(cpu, wakeup_mask))
				continue;

			/*                  */
			if (!xlp_wakeup_core(nodep->sysbase, n, core))
				continue;

			/*            */
			nodep->coremask |= 1u << core;

			/*                                               */
			count = 0x20000000;
			do {
				val = *(volatile int *)&nlm_cpu_ready[cpu];
			} while (val == 0 && --count > 0);
		}
	}
}

void xlp_wakeup_secondary_cpus()
{
	/*
                                                   
                               
  */
	xlp_boot_core0_siblings();

	/*                                  */
	xlp_enable_secondary_cores(&nlm_cpumask);
}
