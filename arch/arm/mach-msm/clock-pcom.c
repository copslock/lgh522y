/*
 * Copyright (C) 2007 Google, Inc.
 * Copyright (c) 2007-2010, Code Aurora Forum. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 */

#include <linux/err.h>
#include <linux/ctype.h>
#include <linux/stddef.h>
#include <mach/clk.h>

#include "proc_comm.h"
#include "clock.h"
#include "clock-pcom.h"

/*
                                   
 */
static int pc_clk_enable(unsigned id)
{
	int rc = msm_proc_comm(PCOM_CLKCTL_RPC_ENABLE, &id, NULL);
	if (rc < 0)
		return rc;
	else
		return (int)id < 0 ? -EINVAL : 0;
}

static void pc_clk_disable(unsigned id)
{
	msm_proc_comm(PCOM_CLKCTL_RPC_DISABLE, &id, NULL);
}

int pc_clk_reset(unsigned id, enum clk_reset_action action)
{
	int rc;

	if (action == CLK_RESET_ASSERT)
		rc = msm_proc_comm(PCOM_CLKCTL_RPC_RESET_ASSERT, &id, NULL);
	else
		rc = msm_proc_comm(PCOM_CLKCTL_RPC_RESET_DEASSERT, &id, NULL);

	if (rc < 0)
		return rc;
	else
		return (int)id < 0 ? -EINVAL : 0;
}

static int pc_clk_set_rate(unsigned id, unsigned rate)
{
	/*                                                                
                                                                    
                                             
  */
	int rc = msm_proc_comm(PCOM_CLKCTL_RPC_SET_RATE, &id, &rate);
	if (rc < 0)
		return rc;
	else
		return (int)id < 0 ? -EINVAL : 0;
}

static int pc_clk_set_min_rate(unsigned id, unsigned rate)
{
	int rc = msm_proc_comm(PCOM_CLKCTL_RPC_MIN_RATE, &id, &rate);
	if (rc < 0)
		return rc;
	else
		return (int)id < 0 ? -EINVAL : 0;
}

static int pc_clk_set_max_rate(unsigned id, unsigned rate)
{
	int rc = msm_proc_comm(PCOM_CLKCTL_RPC_MAX_RATE, &id, &rate);
	if (rc < 0)
		return rc;
	else
		return (int)id < 0 ? -EINVAL : 0;
}

static int pc_clk_set_flags(unsigned id, unsigned flags)
{
	int rc = msm_proc_comm(PCOM_CLKCTL_RPC_SET_FLAGS, &id, &flags);
	if (rc < 0)
		return rc;
	else
		return (int)id < 0 ? -EINVAL : 0;
}

static unsigned pc_clk_get_rate(unsigned id)
{
	if (msm_proc_comm(PCOM_CLKCTL_RPC_RATE, &id, NULL))
		return 0;
	else
		return id;
}

static unsigned pc_clk_is_enabled(unsigned id)
{
	if (msm_proc_comm(PCOM_CLKCTL_RPC_ENABLED, &id, NULL))
		return 0;
	else
		return id;
}

static long pc_clk_round_rate(unsigned id, unsigned rate)
{

	/*                                                                    */
	return rate;
}

static bool pc_clk_is_local(unsigned id)
{
	return false;
}

struct clk_ops clk_ops_pcom = {
	.enable = pc_clk_enable,
	.disable = pc_clk_disable,
	.auto_off = pc_clk_disable,
	.reset = pc_clk_reset,
	.set_rate = pc_clk_set_rate,
	.set_min_rate = pc_clk_set_min_rate,
	.set_max_rate = pc_clk_set_max_rate,
	.set_flags = pc_clk_set_flags,
	.get_rate = pc_clk_get_rate,
	.is_enabled = pc_clk_is_enabled,
	.round_rate = pc_clk_round_rate,
	.is_local = pc_clk_is_local,
};
