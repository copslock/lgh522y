/*
 * omap_hwmod implementation for OMAP2/3/4
 *
 * Copyright (C) 2009-2011 Nokia Corporation
 * Copyright (C) 2011-2012 Texas Instruments, Inc.
 *
 * Paul Walmsley, Benoît Cousson, Kevin Hilman
 *
 * Created in collaboration with (alphabetical order): Thara Gopinath,
 * Tony Lindgren, Rajendra Nayak, Vikram Pandita, Sakari Poussa, Anand
 * Sawant, Santosh Shilimkar, Richard Woodruff
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Introduction
 * ------------
 * One way to view an OMAP SoC is as a collection of largely unrelated
 * IP blocks connected by interconnects.  The IP blocks include
 * devices such as ARM processors, audio serial interfaces, UARTs,
 * etc.  Some of these devices, like the DSP, are created by TI;
 * others, like the SGX, largely originate from external vendors.  In
 * TI's documentation, on-chip devices are referred to as "OMAP
 * modules."  Some of these IP blocks are identical across several
 * OMAP versions.  Others are revised frequently.
 *
 * These OMAP modules are tied together by various interconnects.
 * Most of the address and data flow between modules is via OCP-based
 * interconnects such as the L3 and L4 buses; but there are other
 * interconnects that distribute the hardware clock tree, handle idle
 * and reset signaling, supply power, and connect the modules to
 * various pads or balls on the OMAP package.
 *
 * OMAP hwmod provides a consistent way to describe the on-chip
 * hardware blocks and their integration into the rest of the chip.
 * This description can be automatically generated from the TI
 * hardware database.  OMAP hwmod provides a standard, consistent API
 * to reset, enable, idle, and disable these hardware blocks.  And
 * hwmod provides a way for other core code, such as the Linux device
 * code or the OMAP power management and address space mapping code,
 * to query the hardware database.
 *
 * Using hwmod
 * -----------
 * Drivers won't call hwmod functions directly.  That is done by the
 * omap_device code, and in rare occasions, by custom integration code
 * in arch/arm/ *omap*.  The omap_device code includes functions to
 * build a struct platform_device using omap_hwmod data, and that is
 * currently how hwmod data is communicated to drivers and to the
 * Linux driver model.  Most drivers will call omap_hwmod functions only
 * indirectly, via pm_runtime*() functions.
 *
 * From a layering perspective, here is where the OMAP hwmod code
 * fits into the kernel software stack:
 *
 *            +-------------------------------+
 *            |      Device driver code       |
 *            |      (e.g., drivers/)         |
 *            +-------------------------------+
 *            |      Linux driver model       |
 *            |     (platform_device /        |
 *            |  platform_driver data/code)   |
 *            +-------------------------------+
 *            | OMAP core-driver integration  |
 *            |(arch/arm/mach-omap2/devices.c)|
 *            +-------------------------------+
 *            |      omap_device code         |
 *            | (../plat-omap/omap_device.c)  |
 *            +-------------------------------+
 *   ---->    |    omap_hwmod code/data       |    <-----
 *            | (../mach-omap2/omap_hwmod*)   |
 *            +-------------------------------+
 *            | OMAP clock/PRCM/register fns  |
 *            | (__raw_{read,write}l, clk*)   |
 *            +-------------------------------+
 *
 * Device drivers should not contain any OMAP-specific code or data in
 * them.  They should only contain code to operate the IP block that
 * the driver is responsible for.  This is because these IP blocks can
 * also appear in other SoCs, either from TI (such as DaVinci) or from
 * other manufacturers; and drivers should be reusable across other
 * platforms.
 *
 * The OMAP hwmod code also will attempt to reset and idle all on-chip
 * devices upon boot.  The goal here is for the kernel to be
 * completely self-reliant and independent from bootloaders.  This is
 * to ensure a repeatable configuration, both to ensure consistent
 * runtime behavior, and to make it easier for others to reproduce
 * bugs.
 *
 * OMAP module activity states
 * ---------------------------
 * The hwmod code considers modules to be in one of several activity
 * states.  IP blocks start out in an UNKNOWN state, then once they
 * are registered via the hwmod code, proceed to the REGISTERED state.
 * Once their clock names are resolved to clock pointers, the module
 * enters the CLKS_INITED state; and finally, once the module has been
 * reset and the integration registers programmed, the INITIALIZED state
 * is entered.  The hwmod code will then place the module into either
 * the IDLE state to save power, or in the case of a critical system
 * module, the ENABLED state.
 *
 * OMAP core integration code can then call omap_hwmod*() functions
 * directly to move the module between the IDLE, ENABLED, and DISABLED
 * states, as needed.  This is done during both the PM idle loop, and
 * in the OMAP core integration code's implementation of the PM runtime
 * functions.
 *
 * References
 * ----------
 * This is a partial list.
 * - OMAP2420 Multimedia Processor Silicon Revision 2.1.1, 2.2 (SWPU064)
 * - OMAP2430 Multimedia Device POP Silicon Revision 2.1 (SWPU090)
 * - OMAP34xx Multimedia Device Silicon Revision 3.1 (SWPU108)
 * - OMAP4430 Multimedia Device Silicon Revision 1.0 (SWPU140)
 * - Open Core Protocol Specification 2.2
 *
 * To do:
 * - handle IO mapping
 * - bus throughput & module latency measurement code
 *
 * XXX add tests at the beginning of each function to ensure the hwmod is
 * in the appropriate state
 * XXX error return values should be checked to ensure that they are
 * appropriate
 */
#undef DEBUG

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/clk-provider.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/bootmem.h>
#include <linux/cpu.h>
#include <linux/of.h>
#include <linux/of_address.h>

#include <asm/system_misc.h>

#include "clock.h"
#include "omap_hwmod.h"

#include "soc.h"
#include "common.h"
#include "clockdomain.h"
#include "powerdomain.h"
#include "cm2xxx.h"
#include "cm3xxx.h"
#include "cminst44xx.h"
#include "cm33xx.h"
#include "prm.h"
#include "prm3xxx.h"
#include "prm44xx.h"
#include "prm33xx.h"
#include "prminst44xx.h"
#include "mux.h"
#include "pm.h"

/*                                    */
#define MPU_INITIATOR_NAME		"mpu"

/*
                                                      
                                                             
 */
#define LINKS_PER_OCP_IF		2

/* 
                                                                       
                                                               
                                                                 
  
                                                                     
                                                                           
                             
 */
struct omap_hwmod_soc_ops {
	void (*enable_module)(struct omap_hwmod *oh);
	int (*disable_module)(struct omap_hwmod *oh);
	int (*wait_target_ready)(struct omap_hwmod *oh);
	int (*assert_hardreset)(struct omap_hwmod *oh,
				struct omap_hwmod_rst_info *ohri);
	int (*deassert_hardreset)(struct omap_hwmod *oh,
				  struct omap_hwmod_rst_info *ohri);
	int (*is_hardreset_asserted)(struct omap_hwmod *oh,
				     struct omap_hwmod_rst_info *ohri);
	int (*init_clkdm)(struct omap_hwmod *oh);
	void (*update_context_lost)(struct omap_hwmod *oh);
	int (*get_context_lost)(struct omap_hwmod *oh);
};

/*                                                                 */
static struct omap_hwmod_soc_ops soc_ops;

/*                                                            */
static LIST_HEAD(omap_hwmod_list);

/*                                                             */
static struct omap_hwmod *mpu_oh;

/*                                                                    */
static DEFINE_SPINLOCK(io_chain_lock);

/*
                                                                     
                                                             
                                                             
 */
static struct omap_hwmod_link *linkspace;

/*
                                                                  
                                                                    
                                                          
 */
static unsigned short free_ls, max_ls, ls_supp;

/*                                                        */
static bool inited;

/*                   */

/* 
                                                               
                                                                
                                                                       
  
                                                          
                                                                  
                                                                  
          
 */
static struct omap_hwmod_ocp_if *_fetch_next_ocp_if(struct list_head **p,
						    int *i)
{
	struct omap_hwmod_ocp_if *oi;

	oi = list_entry(*p, struct omap_hwmod_link, node)->ocp_if;
	*p = (*p)->next;

	*i = *i + 1;

	return oi;
}

/* 
                                                                           
                           
  
                                                                      
                                                                        
                                            
 */
static int _update_sysc_cache(struct omap_hwmod *oh)
{
	if (!oh->class->sysc) {
		WARN(1, "omap_hwmod: %s: cannot read OCP_SYSCONFIG: not defined on hwmod's class\n", oh->name);
		return -EINVAL;
	}

	/*                                         */

	oh->_sysc_cache = omap_hwmod_read(oh, oh->class->sysc->sysc_offs);

	if (!(oh->class->sysc->sysc_flags & SYSC_NO_CACHE))
		oh->_int_flags |= _HWMOD_SYSCONFIG_LOADED;

	return 0;
}

/* 
                                                                          
                                   
                           
  
                                                                    
                         
 */
static void _write_sysconfig(u32 v, struct omap_hwmod *oh)
{
	if (!oh->class->sysc) {
		WARN(1, "omap_hwmod: %s: cannot write OCP_SYSCONFIG: not defined on hwmod's class\n", oh->name);
		return;
	}

	/*                                         */

	/*                                                                  */
	oh->_sysc_cache = v;
	omap_hwmod_write(v, oh, oh->class->sysc->sysc_offs);
}

/* 
                                                                       
                           
                                     
                                             
  
                                                                   
                                                                   
                                
 */
static int _set_master_standbymode(struct omap_hwmod *oh, u8 standbymode,
				   u32 *v)
{
	u32 mstandby_mask;
	u8 mstandby_shift;

	if (!oh->class->sysc ||
	    !(oh->class->sysc->sysc_flags & SYSC_HAS_MIDLEMODE))
		return -EINVAL;

	if (!oh->class->sysc->sysc_fields) {
		WARN(1, "omap_hwmod: %s: offset struct for sysconfig not provided in class\n", oh->name);
		return -EINVAL;
	}

	mstandby_shift = oh->class->sysc->sysc_fields->midle_shift;
	mstandby_mask = (0x3 << mstandby_shift);

	*v &= ~mstandby_mask;
	*v |= __ffs(standbymode) << mstandby_shift;

	return 0;
}

/* 
                                                                   
                           
                                  
                                             
  
                                                                    
                                                                      
                     
 */
static int _set_slave_idlemode(struct omap_hwmod *oh, u8 idlemode, u32 *v)
{
	u32 sidle_mask;
	u8 sidle_shift;

	if (!oh->class->sysc ||
	    !(oh->class->sysc->sysc_flags & SYSC_HAS_SIDLEMODE))
		return -EINVAL;

	if (!oh->class->sysc->sysc_fields) {
		WARN(1, "omap_hwmod: %s: offset struct for sysconfig not provided in class\n", oh->name);
		return -EINVAL;
	}

	sidle_shift = oh->class->sysc->sysc_fields->sidle_shift;
	sidle_mask = (0x3 << sidle_shift);

	*v &= ~sidle_mask;
	*v |= __ffs(idlemode) << sidle_shift;

	return 0;
}

/* 
                                                                 
                           
                                      
                                             
  
                                                                   
                                                                     
                                                                   
           
 */
static int _set_clockactivity(struct omap_hwmod *oh, u8 clockact, u32 *v)
{
	u32 clkact_mask;
	u8  clkact_shift;

	if (!oh->class->sysc ||
	    !(oh->class->sysc->sysc_flags & SYSC_HAS_CLOCKACTIVITY))
		return -EINVAL;

	if (!oh->class->sysc->sysc_fields) {
		WARN(1, "omap_hwmod: %s: offset struct for sysconfig not provided in class\n", oh->name);
		return -EINVAL;
	}

	clkact_shift = oh->class->sysc->sysc_fields->clkact_shift;
	clkact_mask = (0x3 << clkact_shift);

	*v &= ~clkact_mask;
	*v |= clockact << clkact_shift;

	return 0;
}

/* 
                                                        
                           
                                             
  
                                                                   
                           
 */
static int _set_softreset(struct omap_hwmod *oh, u32 *v)
{
	u32 softrst_mask;

	if (!oh->class->sysc ||
	    !(oh->class->sysc->sysc_flags & SYSC_HAS_SOFTRESET))
		return -EINVAL;

	if (!oh->class->sysc->sysc_fields) {
		WARN(1, "omap_hwmod: %s: offset struct for sysconfig not provided in class\n", oh->name);
		return -EINVAL;
	}

	softrst_mask = (0x1 << oh->class->sysc->sysc_fields->srst_shift);

	*v |= softrst_mask;

	return 0;
}

/* 
                                                            
                           
                                             
  
                                                                     
                           
 */
static int _clear_softreset(struct omap_hwmod *oh, u32 *v)
{
	u32 softrst_mask;

	if (!oh->class->sysc ||
	    !(oh->class->sysc->sysc_flags & SYSC_HAS_SOFTRESET))
		return -EINVAL;

	if (!oh->class->sysc->sysc_fields) {
		WARN(1,
		     "omap_hwmod: %s: sysc_fields absent for sysconfig class\n",
		     oh->name);
		return -EINVAL;
	}

	softrst_mask = (0x1 << oh->class->sysc->sysc_fields->srst_shift);

	*v &= ~softrst_mask;

	return 0;
}

/* 
                                                                   
                                      
  
                                                                  
                                                                 
                                                                     
                                                                
                                                            
 */
static int _wait_softreset_complete(struct omap_hwmod *oh)
{
	struct omap_hwmod_class_sysconfig *sysc;
	u32 softrst_mask;
	int c = 0;

	sysc = oh->class->sysc;

	if (sysc->sysc_flags & SYSS_HAS_RESET_STATUS)
		omap_test_timeout((omap_hwmod_read(oh, sysc->syss_offs)
				   & SYSS_RESETDONE_MASK),
				  MAX_MODULE_SOFTRESET_WAIT, c);
	else if (sysc->sysc_flags & SYSC_HAS_RESET_STATUS) {
		softrst_mask = (0x1 << sysc->sysc_fields->srst_shift);
		omap_test_timeout(!(omap_hwmod_read(oh, sysc->sysc_offs)
				    & softrst_mask),
				  MAX_MODULE_SOFTRESET_WAIT, c);
	}

	return c;
}

/* 
                                                          
                           
  
                                                                           
                                                                      
                                                                        
                                                                        
  
                                                                    
                           
 */
static int _set_dmadisable(struct omap_hwmod *oh)
{
	u32 v;
	u32 dmadisable_mask;

	if (!oh->class->sysc ||
	    !(oh->class->sysc->sysc_flags & SYSC_HAS_DMADISABLE))
		return -EINVAL;

	if (!oh->class->sysc->sysc_fields) {
		WARN(1, "omap_hwmod: %s: offset struct for sysconfig not provided in class\n", oh->name);
		return -EINVAL;
	}

	/*                                      */
	if (oh->_state != _HWMOD_STATE_ENABLED) {
		pr_warn("omap_hwmod: %s: dma can be disabled only from enabled state\n", oh->name);
		return -EINVAL;
	}

	pr_debug("omap_hwmod: %s: setting DMADISABLE\n", oh->name);

	v = oh->_sysc_cache;
	dmadisable_mask =
		(0x1 << oh->class->sysc->sysc_fields->dmadisable_shift);
	v |= dmadisable_mask;
	_write_sysconfig(v, oh);

	return 0;
}

/* 
                                                                   
                           
                                                      
                                             
  
                                                                   
                                                                
                                                                  
                                                                 
                                                                 
                           
 */
static int _set_module_autoidle(struct omap_hwmod *oh, u8 autoidle,
				u32 *v)
{
	u32 autoidle_mask;
	u8 autoidle_shift;

	if (!oh->class->sysc ||
	    !(oh->class->sysc->sysc_flags & SYSC_HAS_AUTOIDLE))
		return -EINVAL;

	if (!oh->class->sysc->sysc_fields) {
		WARN(1, "omap_hwmod: %s: offset struct for sysconfig not provided in class\n", oh->name);
		return -EINVAL;
	}

	autoidle_shift = oh->class->sysc->sysc_fields->autoidle_shift;
	autoidle_mask = (0x1 << autoidle_shift);

	*v &= ~autoidle_mask;
	*v |= autoidle << autoidle_shift;

	return 0;
}

/* 
                                                                               
                           
                                                                                
  
                                                                  
                                                                     
                                                                
                                                                
                                                                    
                                                                   
                                                                 
                                    
 */
static void _set_idle_ioring_wakeup(struct omap_hwmod *oh, bool set_wake)
{
	struct omap_device_pad *pad;
	bool change = false;
	u16 prev_idle;
	int j;

	if (!oh->mux || !oh->mux->enabled)
		return;

	for (j = 0; j < oh->mux->nr_pads_dynamic; j++) {
		pad = oh->mux->pads_dynamic[j];

		if (!(pad->flags & OMAP_DEVICE_PAD_WAKEUP))
			continue;

		prev_idle = pad->idle;

		if (set_wake)
			pad->idle |= OMAP_WAKEUP_EN;
		else
			pad->idle &= ~OMAP_WAKEUP_EN;

		if (prev_idle != pad->idle)
			change = true;
	}

	if (change && oh->_state == _HWMOD_STATE_IDLE)
		omap_hwmod_mux(oh->mux, _HWMOD_STATE_IDLE);
}

/* 
                                                                  
                           
  
                                                                  
                                
 */
static int _enable_wakeup(struct omap_hwmod *oh, u32 *v)
{
	if (!oh->class->sysc ||
	    !((oh->class->sysc->sysc_flags & SYSC_HAS_ENAWAKEUP) ||
	      (oh->class->sysc->idlemodes & SIDLE_SMART_WKUP) ||
	      (oh->class->sysc->idlemodes & MSTANDBY_SMART_WKUP)))
		return -EINVAL;

	if (!oh->class->sysc->sysc_fields) {
		WARN(1, "omap_hwmod: %s: offset struct for sysconfig not provided in class\n", oh->name);
		return -EINVAL;
	}

	if (oh->class->sysc->sysc_flags & SYSC_HAS_ENAWAKEUP)
		*v |= 0x1 << oh->class->sysc->sysc_fields->enwkup_shift;

	if (oh->class->sysc->idlemodes & SIDLE_SMART_WKUP)
		_set_slave_idlemode(oh, HWMOD_IDLEMODE_SMART_WKUP, v);
	if (oh->class->sysc->idlemodes & MSTANDBY_SMART_WKUP)
		_set_master_standbymode(oh, HWMOD_IDLEMODE_SMART_WKUP, v);

	/*                                                    */

	return 0;
}

/* 
                                                                     
                           
  
                                                                    
                                
 */
static int _disable_wakeup(struct omap_hwmod *oh, u32 *v)
{
	if (!oh->class->sysc ||
	    !((oh->class->sysc->sysc_flags & SYSC_HAS_ENAWAKEUP) ||
	      (oh->class->sysc->idlemodes & SIDLE_SMART_WKUP) ||
	      (oh->class->sysc->idlemodes & MSTANDBY_SMART_WKUP)))
		return -EINVAL;

	if (!oh->class->sysc->sysc_fields) {
		WARN(1, "omap_hwmod: %s: offset struct for sysconfig not provided in class\n", oh->name);
		return -EINVAL;
	}

	if (oh->class->sysc->sysc_flags & SYSC_HAS_ENAWAKEUP)
		*v &= ~(0x1 << oh->class->sysc->sysc_fields->enwkup_shift);

	if (oh->class->sysc->idlemodes & SIDLE_SMART_WKUP)
		_set_slave_idlemode(oh, HWMOD_IDLEMODE_SMART, v);
	if (oh->class->sysc->idlemodes & MSTANDBY_SMART_WKUP)
		_set_master_standbymode(oh, HWMOD_IDLEMODE_SMART, v);

	/*                                                    */

	return 0;
}

static struct clockdomain *_get_clkdm(struct omap_hwmod *oh)
{
	struct clk_hw_omap *clk;

	if (oh->clkdm) {
		return oh->clkdm;
	} else if (oh->_clk) {
		clk = to_clk_hw_omap(__clk_get_hw(oh->_clk));
		return  clk->clkdm;
	}
	return NULL;
}

/* 
                                                                             
                           
  
                                                               
                                                                     
                                                                     
                                                                     
                                                                    
                                                                      
                                                                      
                                                        
 */
static int _add_initiator_dep(struct omap_hwmod *oh, struct omap_hwmod *init_oh)
{
	struct clockdomain *clkdm, *init_clkdm;

	clkdm = _get_clkdm(oh);
	init_clkdm = _get_clkdm(init_oh);

	if (!clkdm || !init_clkdm)
		return -EINVAL;

	if (clkdm && clkdm->flags & CLKDM_NO_AUTODEPS)
		return 0;

	return clkdm_add_sleepdep(clkdm, init_clkdm);
}

/* 
                                                                         
                           
  
                                                                
                                                                      
                                                                    
                                                                      
                                                                    
                                                                      
                                                                  
                                                 
 */
static int _del_initiator_dep(struct omap_hwmod *oh, struct omap_hwmod *init_oh)
{
	struct clockdomain *clkdm, *init_clkdm;

	clkdm = _get_clkdm(oh);
	init_clkdm = _get_clkdm(init_oh);

	if (!clkdm || !init_clkdm)
		return -EINVAL;

	if (clkdm && clkdm->flags & CLKDM_NO_AUTODEPS)
		return 0;

	return clkdm_del_sleepdep(clkdm, init_clkdm);
}

/* 
                                                                              
                           
  
                                                            
                                                                    
                               
 */
static int _init_main_clk(struct omap_hwmod *oh)
{
	int ret = 0;

	if (!oh->main_clk)
		return 0;

	oh->_clk = clk_get(NULL, oh->main_clk);
	if (IS_ERR(oh->_clk)) {
		pr_warning("omap_hwmod: %s: cannot clk_get main_clk %s\n",
			   oh->name, oh->main_clk);
		return -EINVAL;
	}
	/*
                                                                 
                                                       
                                                            
                                                            
                                                        
              
  */
	clk_prepare(oh->_clk);

	if (!_get_clkdm(oh))
		pr_debug("omap_hwmod: %s: missing clockdomain for %s.\n",
			   oh->name, oh->main_clk);

	return ret;
}

/* 
                                                                               
                           
  
                                                                     
                                                             
 */
static int _init_interface_clks(struct omap_hwmod *oh)
{
	struct omap_hwmod_ocp_if *os;
	struct list_head *p;
	struct clk *c;
	int i = 0;
	int ret = 0;

	p = oh->slave_ports.next;

	while (i < oh->slaves_cnt) {
		os = _fetch_next_ocp_if(&p, &i);
		if (!os->clk)
			continue;

		c = clk_get(NULL, os->clk);
		if (IS_ERR(c)) {
			pr_warning("omap_hwmod: %s: cannot clk_get interface_clk %s\n",
				   oh->name, os->clk);
			ret = -EINVAL;
		}
		os->_clk = c;
		/*
                                                                  
                                                        
                                                             
                                                             
                                                         
               
   */
		clk_prepare(os->_clk);
	}

	return ret;
}

/* 
                                                                         
                           
  
                                                                    
                                                             
 */
static int _init_opt_clks(struct omap_hwmod *oh)
{
	struct omap_hwmod_opt_clk *oc;
	struct clk *c;
	int i;
	int ret = 0;

	for (i = oh->opt_clks_cnt, oc = oh->opt_clks; i > 0; i--, oc++) {
		c = clk_get(NULL, oc->clk);
		if (IS_ERR(c)) {
			pr_warning("omap_hwmod: %s: cannot clk_get opt_clk %s\n",
				   oh->name, oc->clk);
			ret = -EINVAL;
		}
		oc->_clk = c;
		/*
                                                                  
                                                        
                                                             
                                                             
                                                         
               
   */
		clk_prepare(oc->_clk);
	}

	return ret;
}

/* 
                                                                
                           
  
                                                                        
                                
 */
static int _enable_clocks(struct omap_hwmod *oh)
{
	struct omap_hwmod_ocp_if *os;
	struct list_head *p;
	int i = 0;

	pr_debug("omap_hwmod: %s: enabling clocks\n", oh->name);

	if (oh->_clk)
		clk_enable(oh->_clk);

	p = oh->slave_ports.next;

	while (i < oh->slaves_cnt) {
		os = _fetch_next_ocp_if(&p, &i);

		if (os->_clk && (os->flags & OCPIF_SWSUP_IDLE))
			clk_enable(os->_clk);
	}

	/*                                                     */

	return 0;
}

/* 
                                                                  
                           
  
                                                                           
 */
static int _disable_clocks(struct omap_hwmod *oh)
{
	struct omap_hwmod_ocp_if *os;
	struct list_head *p;
	int i = 0;

	pr_debug("omap_hwmod: %s: disabling clocks\n", oh->name);

	if (oh->_clk)
		clk_disable(oh->_clk);

	p = oh->slave_ports.next;

	while (i < oh->slaves_cnt) {
		os = _fetch_next_ocp_if(&p, &i);

		if (os->_clk && (os->flags & OCPIF_SWSUP_IDLE))
			clk_disable(os->_clk);
	}

	/*                                                     */

	return 0;
}

static void _enable_optional_clocks(struct omap_hwmod *oh)
{
	struct omap_hwmod_opt_clk *oc;
	int i;

	pr_debug("omap_hwmod: %s: enabling optional clocks\n", oh->name);

	for (i = oh->opt_clks_cnt, oc = oh->opt_clks; i > 0; i--, oc++)
		if (oc->_clk) {
			pr_debug("omap_hwmod: enable %s:%s\n", oc->role,
				 __clk_get_name(oc->_clk));
			clk_enable(oc->_clk);
		}
}

static void _disable_optional_clocks(struct omap_hwmod *oh)
{
	struct omap_hwmod_opt_clk *oc;
	int i;

	pr_debug("omap_hwmod: %s: disabling optional clocks\n", oh->name);

	for (i = oh->opt_clks_cnt, oc = oh->opt_clks; i > 0; i--, oc++)
		if (oc->_clk) {
			pr_debug("omap_hwmod: disable %s:%s\n", oc->role,
				 __clk_get_name(oc->_clk));
			clk_disable(oc->_clk);
		}
}

/* 
                                                            
                           
  
                                                         
                   
 */
static void _omap4_enable_module(struct omap_hwmod *oh)
{
	if (!oh->clkdm || !oh->prcm.omap4.modulemode)
		return;

	pr_debug("omap_hwmod: %s: %s: %d\n",
		 oh->name, __func__, oh->prcm.omap4.modulemode);

	omap4_cminst_module_enable(oh->prcm.omap4.modulemode,
				   oh->clkdm->prcm_partition,
				   oh->clkdm->cm_inst,
				   oh->clkdm->clkdm_offs,
				   oh->prcm.omap4.clkctrl_offs);
}

/* 
                                                              
                           
  
                                                         
                   
 */
static void _am33xx_enable_module(struct omap_hwmod *oh)
{
	if (!oh->clkdm || !oh->prcm.omap4.modulemode)
		return;

	pr_debug("omap_hwmod: %s: %s: %d\n",
		 oh->name, __func__, oh->prcm.omap4.modulemode);

	am33xx_cm_module_enable(oh->prcm.omap4.modulemode, oh->clkdm->cm_inst,
				oh->clkdm->clkdm_offs,
				oh->prcm.omap4.clkctrl_offs);
}

/* 
                                                                         
                           
  
                                                                      
                                                                   
                                                            
                                                 
 */
static int _omap4_wait_target_disable(struct omap_hwmod *oh)
{
	if (!oh)
		return -EINVAL;

	if (oh->_int_flags & _HWMOD_NO_MPU_PORT || !oh->clkdm)
		return 0;

	if (oh->flags & HWMOD_NO_IDLEST)
		return 0;

	return omap4_cminst_wait_module_idle(oh->clkdm->prcm_partition,
					     oh->clkdm->cm_inst,
					     oh->clkdm->clkdm_offs,
					     oh->prcm.omap4.clkctrl_offs);
}

/* 
                                                                           
                           
  
                                                                      
                                                                   
                                                            
                                                 
 */
static int _am33xx_wait_target_disable(struct omap_hwmod *oh)
{
	if (!oh)
		return -EINVAL;

	if (oh->_int_flags & _HWMOD_NO_MPU_PORT)
		return 0;

	if (oh->flags & HWMOD_NO_IDLEST)
		return 0;

	return am33xx_cm_wait_module_idle(oh->clkdm->cm_inst,
					     oh->clkdm->clkdm_offs,
					     oh->prcm.omap4.clkctrl_offs);
}

/* 
                                                                          
                             
  
                                                                    
                                                                    
        
 */
static int _count_mpu_irqs(struct omap_hwmod *oh)
{
	struct omap_hwmod_irq_info *ohii;
	int i = 0;

	if (!oh || !oh->mpu_irqs)
		return 0;

	do {
		ohii = &oh->mpu_irqs[i++];
	} while (ohii->irq != -1);

	return i-1;
}

/* 
                                                                                
                             
  
                                                                    
                                                                    
                  
 */
static int _count_sdma_reqs(struct omap_hwmod *oh)
{
	struct omap_hwmod_dma_info *ohdi;
	int i = 0;

	if (!oh || !oh->sdma_reqs)
		return 0;

	do {
		ohdi = &oh->sdma_reqs[i++];
	} while (ohdi->dma_req != -1);

	return i-1;
}

/* 
                                                                                
                             
  
                                                                      
                                                                    
                  
 */
static int _count_ocp_if_addr_spaces(struct omap_hwmod_ocp_if *os)
{
	struct omap_hwmod_addr_space *mem;
	int i = 0;

	if (!os || !os->addr)
		return 0;

	do {
		mem = &os->addr[i++];
	} while (mem->pa_start != mem->pa_end);

	return i-1;
}

/* 
                                                                 
                                         
                                                                             
                                                                  
  
                                                                    
                                                                      
                                                                    
                                                                    
                                                                     
                                                                 
                                                                
                                                                      
                                                                  
                   
 */
static int _get_mpu_irq_by_name(struct omap_hwmod *oh, const char *name,
				unsigned int *irq)
{
	int i;
	bool found = false;

	if (!oh->mpu_irqs)
		return -ENOENT;

	i = 0;
	while (oh->mpu_irqs[i].irq != -1) {
		if (name == oh->mpu_irqs[i].name ||
		    !strcmp(name, oh->mpu_irqs[i].name)) {
			found = true;
			break;
		}
		i++;
	}

	if (!found)
		return -ENOENT;

	*irq = oh->mpu_irqs[i].irq;

	return 0;
}

/* 
                                                             
                                         
                                                                          
                                                                   
  
                                                                  
                                                                     
                                                                      
                                                                      
                                                                    
                                                                   
                                                                  
                                                                      
                                                    
 */
static int _get_sdma_req_by_name(struct omap_hwmod *oh, const char *name,
				 unsigned int *dma)
{
	int i;
	bool found = false;

	if (!oh->sdma_reqs)
		return -ENOENT;

	i = 0;
	while (oh->sdma_reqs[i].dma_req != -1) {
		if (name == oh->sdma_reqs[i].name ||
		    !strcmp(name, oh->sdma_reqs[i].name)) {
			found = true;
			break;
		}
		i++;
	}

	if (!found)
		return -ENOENT;

	*dma = oh->sdma_reqs[i].dma_req;

	return 0;
}

/* 
                                                                    
                                         
                                                                      
                                                               
                                                           
  
                                                                  
                                                                 
                                                                    
                                                             
                                                                
                                                                    
                                                                    
                                                                   
                                                               
                                  
 */
static int _get_addr_space_by_name(struct omap_hwmod *oh, const char *name,
				   u32 *pa_start, u32 *pa_end)
{
	int i, j;
	struct omap_hwmod_ocp_if *os;
	struct list_head *p = NULL;
	bool found = false;

	p = oh->slave_ports.next;

	i = 0;
	while (i < oh->slaves_cnt) {
		os = _fetch_next_ocp_if(&p, &i);

		if (!os->addr)
			return -ENOENT;

		j = 0;
		while (os->addr[j].pa_start != os->addr[j].pa_end) {
			if (name == os->addr[j].name ||
			    !strcmp(name, os->addr[j].name)) {
				found = true;
				break;
			}
			j++;
		}

		if (found)
			break;
	}

	if (!found)
		return -ENOENT;

	*pa_start = os->addr[j].pa_start;
	*pa_end = os->addr[j].pa_end;

	return 0;
}

/* 
                                                                   
                           
  
                                                                     
                                                                  
                                                                  
         
 */
static void __init _save_mpu_port_index(struct omap_hwmod *oh)
{
	struct omap_hwmod_ocp_if *os = NULL;
	struct list_head *p;
	int i = 0;

	if (!oh)
		return;

	oh->_int_flags |= _HWMOD_NO_MPU_PORT;

	p = oh->slave_ports.next;

	while (i < oh->slaves_cnt) {
		os = _fetch_next_ocp_if(&p, &i);
		if (os->user & OCP_USER_MPU) {
			oh->_mpu_port = os;
			oh->_int_flags &= ~_HWMOD_NO_MPU_PORT;
			break;
		}
	}

	return;
}

/* 
                                                                     
                           
  
                                                                      
                                                                    
                                                                      
                                                                      
                                                                   
                                                                       
                                                                      
            
 */
static struct omap_hwmod_ocp_if *_find_mpu_rt_port(struct omap_hwmod *oh)
{
	if (!oh || oh->_int_flags & _HWMOD_NO_MPU_PORT || oh->slaves_cnt == 0)
		return NULL;

	return oh->_mpu_port;
};

/* 
                                                                             
                           
  
                                                                            
                                                                     
 */
static struct omap_hwmod_addr_space * __init _find_mpu_rt_addr_space(struct omap_hwmod *oh)
{
	struct omap_hwmod_ocp_if *os;
	struct omap_hwmod_addr_space *mem;
	int found = 0, i = 0;

	os = _find_mpu_rt_port(oh);
	if (!os || !os->addr)
		return NULL;

	do {
		mem = &os->addr[i++];
		if (mem->flags & ADDR_TYPE_RT)
			found = 1;
	} while (!found && mem->pa_start != mem->pa_end);

	return (found) ? mem : NULL;
}

/* 
                                                                     
                           
  
                                                                      
                                                                     
                                                                 
                                                                      
                                                                    
                                                     
 */
static void _enable_sysc(struct omap_hwmod *oh)
{
	u8 idlemode, sf;
	u32 v;
	bool clkdm_act;
	struct clockdomain *clkdm;

	if (!oh->class->sysc)
		return;

	/*
                                                            
                                                          
                                                       
                                      
  */
	if (oh->flags & HWMOD_CONTROL_OPT_CLKS_IN_RESET)
		_enable_optional_clocks(oh);
	_wait_softreset_complete(oh);
	if (oh->flags & HWMOD_CONTROL_OPT_CLKS_IN_RESET)
		_disable_optional_clocks(oh);

	v = oh->_sysc_cache;
	sf = oh->class->sysc->sysc_flags;

	clkdm = _get_clkdm(oh);
	if (sf & SYSC_HAS_SIDLEMODE) {
		if (oh->flags & HWMOD_SWSUP_SIDLE ||
		    oh->flags & HWMOD_SWSUP_SIDLE_ACT) {
			idlemode = HWMOD_IDLEMODE_NO;
		} else {
			if (sf & SYSC_HAS_ENAWAKEUP)
				_enable_wakeup(oh, &v);
			if (oh->class->sysc->idlemodes & SIDLE_SMART_WKUP)
				idlemode = HWMOD_IDLEMODE_SMART_WKUP;
			else
				idlemode = HWMOD_IDLEMODE_SMART;
		}

		/*
                                               
                                        
   */
		clkdm_act = (clkdm && clkdm->flags & CLKDM_ACTIVE_WITH_MPU);
		if (clkdm_act && !(oh->class->sysc->idlemodes &
				   (SIDLE_SMART | SIDLE_SMART_WKUP)))
			idlemode = HWMOD_IDLEMODE_FORCE;

		_set_slave_idlemode(oh, idlemode, &v);
	}

	if (sf & SYSC_HAS_MIDLEMODE) {
		if (oh->flags & HWMOD_FORCE_MSTANDBY) {
			idlemode = HWMOD_IDLEMODE_FORCE;
		} else if (oh->flags & HWMOD_SWSUP_MSTANDBY) {
			idlemode = HWMOD_IDLEMODE_NO;
		} else {
			if (sf & SYSC_HAS_ENAWAKEUP)
				_enable_wakeup(oh, &v);
			if (oh->class->sysc->idlemodes & MSTANDBY_SMART_WKUP)
				idlemode = HWMOD_IDLEMODE_SMART_WKUP;
			else
				idlemode = HWMOD_IDLEMODE_SMART;
		}
		_set_master_standbymode(oh, idlemode, &v);
	}

	/*
                                                  
                                                         
                                                       
  */
	if ((oh->flags & HWMOD_SET_DEFAULT_CLOCKACT) &&
	    (sf & SYSC_HAS_CLOCKACTIVITY))
		_set_clockactivity(oh, oh->class->sysc->clockact, &v);

	_write_sysconfig(v, oh);

	/*
                                                             
                                                               
  */
	if (sf & SYSC_HAS_AUTOIDLE) {
		idlemode = (oh->flags & HWMOD_NO_OCP_AUTOIDLE) ?
			0 : 1;
		_set_module_autoidle(oh, idlemode, &v);
		_write_sysconfig(v, oh);
	}
}

/* 
                                                               
                           
  
                                                                  
                                                                     
                                                                      
                                                    
 */
static void _idle_sysc(struct omap_hwmod *oh)
{
	u8 idlemode, sf;
	u32 v;

	if (!oh->class->sysc)
		return;

	v = oh->_sysc_cache;
	sf = oh->class->sysc->sysc_flags;

	if (sf & SYSC_HAS_SIDLEMODE) {
		if (oh->flags & HWMOD_SWSUP_SIDLE) {
			idlemode = HWMOD_IDLEMODE_FORCE;
		} else {
			if (sf & SYSC_HAS_ENAWAKEUP)
				_enable_wakeup(oh, &v);
			if (oh->class->sysc->idlemodes & SIDLE_SMART_WKUP)
				idlemode = HWMOD_IDLEMODE_SMART_WKUP;
			else
				idlemode = HWMOD_IDLEMODE_SMART;
		}
		_set_slave_idlemode(oh, idlemode, &v);
	}

	if (sf & SYSC_HAS_MIDLEMODE) {
		if ((oh->flags & HWMOD_SWSUP_MSTANDBY) ||
		    (oh->flags & HWMOD_FORCE_MSTANDBY)) {
			idlemode = HWMOD_IDLEMODE_FORCE;
		} else {
			if (sf & SYSC_HAS_ENAWAKEUP)
				_enable_wakeup(oh, &v);
			if (oh->class->sysc->idlemodes & MSTANDBY_SMART_WKUP)
				idlemode = HWMOD_IDLEMODE_SMART_WKUP;
			else
				idlemode = HWMOD_IDLEMODE_SMART;
		}
		_set_master_standbymode(oh, idlemode, &v);
	}

	_write_sysconfig(v, oh);
}

/* 
                                                              
                           
  
                                                                 
         
 */
static void _shutdown_sysc(struct omap_hwmod *oh)
{
	u32 v;
	u8 sf;

	if (!oh->class->sysc)
		return;

	v = oh->_sysc_cache;
	sf = oh->class->sysc->sysc_flags;

	if (sf & SYSC_HAS_SIDLEMODE)
		_set_slave_idlemode(oh, HWMOD_IDLEMODE_FORCE, &v);

	if (sf & SYSC_HAS_MIDLEMODE)
		_set_master_standbymode(oh, HWMOD_IDLEMODE_FORCE, &v);

	if (sf & SYSC_HAS_AUTOIDLE)
		_set_module_autoidle(oh, 1, &v);

	_write_sysconfig(v, oh);
}

/* 
                                       
                                    
  
                                                                   
 */
static struct omap_hwmod *_lookup(const char *name)
{
	struct omap_hwmod *oh, *temp_oh;

	oh = NULL;

	list_for_each_entry(temp_oh, &omap_hwmod_list, node) {
		if (!strcmp(name, temp_oh->name)) {
			oh = temp_oh;
			break;
		}
	}

	return oh;
}

/* 
                                                                        
                           
  
                                                                  
                                                               
                                                  
 */
static int _init_clkdm(struct omap_hwmod *oh)
{
	if (!oh->clkdm_name) {
		pr_debug("omap_hwmod: %s: missing clockdomain\n", oh->name);
		return 0;
	}

	oh->clkdm = clkdm_lookup(oh->clkdm_name);
	if (!oh->clkdm) {
		pr_warning("omap_hwmod: %s: could not associate to clkdm %s\n",
			oh->name, oh->clkdm_name);
		return -EINVAL;
	}

	pr_debug("omap_hwmod: %s: associated to clkdm %s\n",
		oh->name, oh->clkdm_name);

	return 0;
}

/* 
                                                                              
                        
                           
                             
  
                                                           
                                                                
                                                
 */
static int _init_clocks(struct omap_hwmod *oh, void *data)
{
	int ret = 0;

	if (oh->_state != _HWMOD_STATE_REGISTERED)
		return 0;

	pr_debug("omap_hwmod: %s: looking up clocks\n", oh->name);

	if (soc_ops.init_clkdm)
		ret |= soc_ops.init_clkdm(oh);

	ret |= _init_main_clk(oh);
	ret |= _init_interface_clks(oh);
	ret |= _init_opt_clks(oh);

	if (!ret)
		oh->_state = _HWMOD_STATE_CLKS_INITED;
	else
		pr_warning("omap_hwmod: %s: cannot _init_clocks\n", oh->name);

	return ret;
}

/* 
                                                                       
                           
                                                             
                                                                      
  
                                                           
                                           
 */
static int _lookup_hardreset(struct omap_hwmod *oh, const char *name,
			     struct omap_hwmod_rst_info *ohri)
{
	int i;

	for (i = 0; i < oh->rst_lines_cnt; i++) {
		const char *rst_line = oh->rst_lines[i].name;
		if (!strcmp(rst_line, name)) {
			ohri->rst_shift = oh->rst_lines[i].rst_shift;
			ohri->st_shift = oh->rst_lines[i].st_shift;
			pr_debug("omap_hwmod: %s: %s: %s: rst %d st %d\n",
				 oh->name, __func__, rst_line, ohri->rst_shift,
				 ohri->st_shift);

			return 0;
		}
	}

	return -ENOENT;
}

/* 
                                                             
                                 
                           
                                                     
  
                                                                    
                                                                      
                                                               
                                                                      
                                                               
                         
 */
static int _assert_hardreset(struct omap_hwmod *oh, const char *name)
{
	struct omap_hwmod_rst_info ohri;
	int ret = -EINVAL;

	if (!oh)
		return -EINVAL;

	if (!soc_ops.assert_hardreset)
		return -ENOSYS;

	ret = _lookup_hardreset(oh, name, &ohri);
	if (ret < 0)
		return ret;

	ret = soc_ops.assert_hardreset(oh, &ohri);

	return ret;
}

/* 
                                                                           
                       
                           
                                                        
  
                                                                    
                                                                      
                                                               
                                                                        
                                                               
                           
 */
static int _deassert_hardreset(struct omap_hwmod *oh, const char *name)
{
	struct omap_hwmod_rst_info ohri;
	int ret = -EINVAL;
	int hwsup = 0;

	if (!oh)
		return -EINVAL;

	if (!soc_ops.deassert_hardreset)
		return -ENOSYS;

	ret = _lookup_hardreset(oh, name, &ohri);
	if (ret < 0)
		return ret;

	if (oh->clkdm) {
		/*
                                                    
                                                       
                                                  
   */
		hwsup = clkdm_in_hwsup(oh->clkdm);
		ret = clkdm_hwmod_enable(oh->clkdm, oh);
		if (ret) {
			WARN(1, "omap_hwmod: %s: could not enable clockdomain %s: %d\n",
			     oh->name, oh->clkdm->name, ret);
			return ret;
		}
	}

	_enable_clocks(oh);
	if (soc_ops.enable_module)
		soc_ops.enable_module(oh);

	ret = soc_ops.deassert_hardreset(oh, &ohri);

	if (soc_ops.disable_module)
		soc_ops.disable_module(oh);
	_disable_clocks(oh);

	if (ret == -EBUSY)
		pr_warning("omap_hwmod: %s: failed to hardreset\n", oh->name);

	if (!ret) {
		/*
                                                      
                                
   */
		if (oh->clkdm && hwsup)
			clkdm_allow_idle(oh->clkdm);
	} else {
		if (oh->clkdm)
			clkdm_hwmod_disable(oh->clkdm, oh);
	}

	return ret;
}

/* 
                                                               
                                
                           
                                                    
  
                                                                 
                                                                
                                                                 
                                                                    
        
 */
static int _read_hardreset(struct omap_hwmod *oh, const char *name)
{
	struct omap_hwmod_rst_info ohri;
	int ret = -EINVAL;

	if (!oh)
		return -EINVAL;

	if (!soc_ops.is_hardreset_asserted)
		return -ENOSYS;

	ret = _lookup_hardreset(oh, name, &ohri);
	if (ret < 0)
		return ret;

	return soc_ops.is_hardreset_asserted(oh, &ohri);
}

/* 
                                                                           
                           
  
                                                                             
                                                                      
                                                       
                                                                      
                                                        
 */
static bool _are_all_hardreset_lines_asserted(struct omap_hwmod *oh)
{
	int i, rst_cnt = 0;

	if (oh->rst_lines_cnt == 0)
		return false;

	for (i = 0; i < oh->rst_lines_cnt; i++)
		if (_read_hardreset(oh, oh->rst_lines[i].name) > 0)
			rst_cnt++;

	if (oh->rst_lines_cnt == rst_cnt)
		return true;

	return false;
}

/* 
                                                                        
             
                           
  
                                                                
                                                                     
                                                                     
                                                                      
                                                        
 */
static bool _are_any_hardreset_lines_asserted(struct omap_hwmod *oh)
{
	int rst_cnt = 0;
	int i;

	for (i = 0; i < oh->rst_lines_cnt && rst_cnt == 0; i++)
		if (_read_hardreset(oh, oh->rst_lines[i].name) > 0)
			rst_cnt++;

	return (rst_cnt) ? true : false;
}

/* 
                                                             
                           
  
                                                         
                                                                             
 */
static int _omap4_disable_module(struct omap_hwmod *oh)
{
	int v;

	if (!oh->clkdm || !oh->prcm.omap4.modulemode)
		return -EINVAL;

	/*
                                                               
                                             
  */
	if (_are_any_hardreset_lines_asserted(oh))
		return 0;

	pr_debug("omap_hwmod: %s: %s\n", oh->name, __func__);

	omap4_cminst_module_disable(oh->clkdm->prcm_partition,
				    oh->clkdm->cm_inst,
				    oh->clkdm->clkdm_offs,
				    oh->prcm.omap4.clkctrl_offs);

	v = _omap4_wait_target_disable(oh);
	if (v)
		pr_warn("omap_hwmod: %s: _wait_target_disable failed\n",
			oh->name);

	return 0;
}

/* 
                                                               
                           
  
                                                         
                                                                             
 */
static int _am33xx_disable_module(struct omap_hwmod *oh)
{
	int v;

	if (!oh->clkdm || !oh->prcm.omap4.modulemode)
		return -EINVAL;

	pr_debug("omap_hwmod: %s: %s\n", oh->name, __func__);

	if (_are_any_hardreset_lines_asserted(oh))
		return 0;

	am33xx_cm_module_disable(oh->clkdm->cm_inst, oh->clkdm->clkdm_offs,
				 oh->prcm.omap4.clkctrl_offs);

	v = _am33xx_wait_target_disable(oh);
	if (v)
		pr_warn("omap_hwmod: %s: _wait_target_disable failed\n",
			oh->name);

	return 0;
}

/* 
                                                                 
                           
  
                                                                     
                                                                    
                                                              
                                                                     
  
                                                                          
                                                                          
                                                         
  
                                                                    
                
 */
static int _ocp_softreset(struct omap_hwmod *oh)
{
	u32 v;
	int c = 0;
	int ret = 0;

	if (!oh->class->sysc ||
	    !(oh->class->sysc->sysc_flags & SYSC_HAS_SOFTRESET))
		return -ENOENT;

	/*                                      */
	if (oh->_state != _HWMOD_STATE_ENABLED) {
		pr_warn("omap_hwmod: %s: reset can only be entered from enabled state\n",
			oh->name);
		return -EINVAL;
	}

	/*                                                                   */
	if (oh->flags & HWMOD_CONTROL_OPT_CLKS_IN_RESET)
		_enable_optional_clocks(oh);

	pr_debug("omap_hwmod: %s: resetting via OCP SOFTRESET\n", oh->name);

	v = oh->_sysc_cache;
	ret = _set_softreset(oh, &v);
	if (ret)
		goto dis_opt_clks;

	_write_sysconfig(v, oh);
	ret = _clear_softreset(oh, &v);
	if (ret)
		goto dis_opt_clks;

	_write_sysconfig(v, oh);

	if (oh->class->sysc->srst_udelay)
		udelay(oh->class->sysc->srst_udelay);

	c = _wait_softreset_complete(oh);
	if (c == MAX_MODULE_SOFTRESET_WAIT)
		pr_warning("omap_hwmod: %s: softreset failed (waited %d usec)\n",
			   oh->name, MAX_MODULE_SOFTRESET_WAIT);
	else
		pr_debug("omap_hwmod: %s: softreset in %d usec\n", oh->name, c);

	/*
                                                                     
                                    
  */

	ret = (c == MAX_MODULE_SOFTRESET_WAIT) ? -ETIMEDOUT : 0;

dis_opt_clks:
	if (oh->flags & HWMOD_CONTROL_OPT_CLKS_IN_RESET)
		_disable_optional_clocks(oh);

	return ret;
}

/* 
                               
                           
  
                                                              
                                                                    
                                                                   
                                                                
                                                                     
                                                                      
                                                                   
                                                                    
                                                                
                                                  
  
                                                                  
                                                                
                                                                    
                                                                      
                                                                    
                                                              
                                            
  
                                                                   
                                                                     
                                                                 
                                
  
                                                                    
                                                                 
                                                                  
                                                                     
 */
static int _reset(struct omap_hwmod *oh)
{
	int i, r;

	pr_debug("omap_hwmod: %s: resetting\n", oh->name);

	if (oh->class->reset) {
		r = oh->class->reset(oh);
	} else {
		if (oh->rst_lines_cnt > 0) {
			for (i = 0; i < oh->rst_lines_cnt; i++)
				_assert_hardreset(oh, oh->rst_lines[i].name);
			return 0;
		} else {
			r = _ocp_softreset(oh);
			if (r == -ENOENT)
				r = 0;
		}
	}

	_set_dmadisable(oh);

	/*
                                                      
                                                               
                                              
  */
	if (oh->class->sysc) {
		_update_sysc_cache(oh);
		_enable_sysc(oh);
	}

	return r;
}

/* 
                                                                            
  
                                                                  
                                                                     
                                                                
                                                          
                                              
  
                                                                           
                                                  
 */
static void _reconfigure_io_chain(void)
{
	unsigned long flags;

	spin_lock_irqsave(&io_chain_lock, flags);

	if (cpu_is_omap34xx() && omap3_has_io_chain_ctrl())
		omap3xxx_prm_reconfigure_io_chain();
	else if (cpu_is_omap44xx())
		omap44xx_prm_reconfigure_io_chain();

	spin_unlock_irqrestore(&io_chain_lock, flags);
}

/* 
                                                                       
                                                              
                                       
  
                                                                   
                                                                 
                         
 */
static void _omap4_update_context_lost(struct omap_hwmod *oh)
{
	if (oh->prcm.omap4.flags & HWMOD_OMAP4_NO_CONTEXT_LOSS_BIT)
		return;

	if (!prm_was_any_context_lost_old(oh->clkdm->pwrdm.ptr->prcm_partition,
					  oh->clkdm->pwrdm.ptr->prcm_offs,
					  oh->prcm.omap4.context_offs))
		return;

	oh->prcm.omap4.context_lost_counter++;
	prm_clear_context_loss_flags_old(oh->clkdm->pwrdm.ptr->prcm_partition,
					 oh->clkdm->pwrdm.ptr->prcm_offs,
					 oh->prcm.omap4.context_offs);
}

/* 
                                                                 
                                             
  
                                                          
 */
static int _omap4_get_context_lost(struct omap_hwmod *oh)
{
	return oh->prcm.omap4.context_lost_counter;
}

/* 
                                                                            
                           
  
                                                                    
                                                                     
                                                                      
                                     
 */
static int _enable_preprogram(struct omap_hwmod *oh)
{
	if (!oh->class->enable_preprogram)
		return 0;

	return oh->class->enable_preprogram(oh);
}

/* 
                                 
                           
  
                                                                     
                                                                 
                                                                  
 */
static int _enable(struct omap_hwmod *oh)
{
	int r;
	int hwsup = 0;

	pr_debug("omap_hwmod: %s: enabling\n", oh->name);

	/*
                                                               
                                                               
                                                
  */
	if (oh->_int_flags & _HWMOD_SKIP_ENABLE) {
		/*
                                                         
                                                           
                       
   */
		if (oh->mux)
			omap_hwmod_mux(oh->mux, _HWMOD_STATE_ENABLED);

		oh->_int_flags &= ~_HWMOD_SKIP_ENABLE;
		return 0;
	}

	if (oh->_state != _HWMOD_STATE_INITIALIZED &&
	    oh->_state != _HWMOD_STATE_IDLE &&
	    oh->_state != _HWMOD_STATE_DISABLED) {
		WARN(1, "omap_hwmod: %s: enabled state can only be entered from initialized, idle, or disabled state\n",
			oh->name);
		return -EINVAL;
	}

	/*
                                                              
                                                          
                                                        
                                                            
                                                           
                                                            
           
  */
	if (_are_all_hardreset_lines_asserted(oh))
		return 0;

	/*                                          */
	if (oh->mux && (!oh->mux->enabled ||
			((oh->_state == _HWMOD_STATE_IDLE) &&
			 oh->mux->pads_dynamic))) {
		omap_hwmod_mux(oh->mux, _HWMOD_STATE_ENABLED);
		_reconfigure_io_chain();
	} else if (oh->flags & HWMOD_FORCE_MSTANDBY) {
		_reconfigure_io_chain();
	}

	_add_initiator_dep(oh, mpu_oh);

	if (oh->clkdm) {
		/*
                                                    
                                                      
                                                  
   */
		hwsup = clkdm_in_hwsup(oh->clkdm) &&
			!clkdm_missing_idle_reporting(oh->clkdm);
		r = clkdm_hwmod_enable(oh->clkdm, oh);
		if (r) {
			WARN(1, "omap_hwmod: %s: could not enable clockdomain %s: %d\n",
			     oh->name, oh->clkdm->name, r);
			return r;
		}
	}

	_enable_clocks(oh);
	if (soc_ops.enable_module)
		soc_ops.enable_module(oh);
	if (oh->flags & HWMOD_BLOCK_WFI)
		cpu_idle_poll_ctrl(true);

	if (soc_ops.update_context_lost)
		soc_ops.update_context_lost(oh);

	r = (soc_ops.wait_target_ready) ? soc_ops.wait_target_ready(oh) :
		-EINVAL;
	if (!r) {
		/*
                                                                
                                                 
   */
		if (oh->clkdm && hwsup)
			clkdm_allow_idle(oh->clkdm);

		oh->_state = _HWMOD_STATE_ENABLED;

		/*                                                  */
		if (oh->class->sysc) {
			if (!(oh->_int_flags & _HWMOD_SYSCONFIG_LOADED))
				_update_sysc_cache(oh);
			_enable_sysc(oh);
		}
		r = _enable_preprogram(oh);
	} else {
		if (soc_ops.disable_module)
			soc_ops.disable_module(oh);
		_disable_clocks(oh);
		pr_debug("omap_hwmod: %s: _wait_target_ready: %d\n",
			 oh->name, r);

		if (oh->clkdm)
			clkdm_hwmod_disable(oh->clkdm, oh);
	}

	return r;
}

/* 
                             
                           
  
                                                                     
                                                                 
                      
 */
static int _idle(struct omap_hwmod *oh)
{
	pr_debug("omap_hwmod: %s: idling\n", oh->name);

	if (oh->_state != _HWMOD_STATE_ENABLED) {
		WARN(1, "omap_hwmod: %s: idle state can only be entered from enabled state\n",
			oh->name);
		return -EINVAL;
	}

	if (_are_all_hardreset_lines_asserted(oh))
		return 0;

	if (oh->class->sysc)
		_idle_sysc(oh);
	_del_initiator_dep(oh, mpu_oh);

	if (oh->flags & HWMOD_BLOCK_WFI)
		cpu_idle_poll_ctrl(false);
	if (soc_ops.disable_module)
		soc_ops.disable_module(oh);

	/*
                                                                
                                                                
                                                            
                                    
  */
	_disable_clocks(oh);
	if (oh->clkdm)
		clkdm_hwmod_disable(oh->clkdm, oh);

	/*                                       */
	if (oh->mux && oh->mux->pads_dynamic) {
		omap_hwmod_mux(oh->mux, _HWMOD_STATE_IDLE);
		_reconfigure_io_chain();
	} else if (oh->flags & HWMOD_FORCE_MSTANDBY) {
		_reconfigure_io_chain();
	}

	oh->_state = _HWMOD_STATE_IDLE;

	return 0;
}

/* 
                                     
                           
  
                                                                      
                                                                    
                                                                    
                      
 */
static int _shutdown(struct omap_hwmod *oh)
{
	int ret, i;
	u8 prev_state;

	if (oh->_state != _HWMOD_STATE_IDLE &&
	    oh->_state != _HWMOD_STATE_ENABLED) {
		WARN(1, "omap_hwmod: %s: disabled state can only be entered from idle, or enabled state\n",
			oh->name);
		return -EINVAL;
	}

	if (_are_all_hardreset_lines_asserted(oh))
		return 0;

	pr_debug("omap_hwmod: %s: disabling\n", oh->name);

	if (oh->class->pre_shutdown) {
		prev_state = oh->_state;
		if (oh->_state == _HWMOD_STATE_IDLE)
			_enable(oh);
		ret = oh->class->pre_shutdown(oh);
		if (ret) {
			if (prev_state == _HWMOD_STATE_IDLE)
				_idle(oh);
			return ret;
		}
	}

	if (oh->class->sysc) {
		if (oh->_state == _HWMOD_STATE_IDLE)
			_enable(oh);
		_shutdown_sysc(oh);
	}

	/*                                              */
	if (oh->_state == _HWMOD_STATE_ENABLED) {
		_del_initiator_dep(oh, mpu_oh);
		/*                                                           */
		if (oh->flags & HWMOD_BLOCK_WFI)
			cpu_idle_poll_ctrl(false);
		if (soc_ops.disable_module)
			soc_ops.disable_module(oh);
		_disable_clocks(oh);
		if (oh->clkdm)
			clkdm_hwmod_disable(oh->clkdm, oh);
	}
	/*                                                              */

	for (i = 0; i < oh->rst_lines_cnt; i++)
		_assert_hardreset(oh, oh->rst_lines[i].name);

	/*                                                        */
	if (oh->mux)
		omap_hwmod_mux(oh->mux, _HWMOD_STATE_DISABLED);

	oh->_state = _HWMOD_STATE_DISABLED;

	return 0;
}

/* 
                                                          
                            
                           
  
                                                                     
                                                         
                                                         
 */
static struct device_node *of_dev_hwmod_lookup(struct device_node *np,
						struct omap_hwmod *oh)
{
	struct device_node *np0 = NULL, *np1 = NULL;
	const char *p;

	for_each_child_of_node(np, np0) {
		if (of_find_property(np0, "ti,hwmods", NULL)) {
			p = of_get_property(np0, "ti,hwmods", NULL);
			if (!strcmp(p, oh->name))
				return np0;
			np1 = of_dev_hwmod_lookup(np0, oh);
			if (np1)
				return np1;
		}
	}
	return NULL;
}

/* 
                                                               
                                                         
  
                                                                      
                                                                     
                                                                     
                   
 */
static void __init _init_mpu_rt_base(struct omap_hwmod *oh, void *data)
{
	struct omap_hwmod_addr_space *mem;
	void __iomem *va_start = NULL;
	struct device_node *np;

	if (!oh)
		return;

	_save_mpu_port_index(oh);

	if (oh->_int_flags & _HWMOD_NO_MPU_PORT)
		return;

	mem = _find_mpu_rt_addr_space(oh);
	if (!mem) {
		pr_debug("omap_hwmod: %s: no MPU register target found\n",
			 oh->name);

		/*                                            */
		if (!of_have_populated_dt())
			return;

		np = of_dev_hwmod_lookup(of_find_node_by_name(NULL, "ocp"), oh);
		if (np)
			va_start = of_iomap(np, 0);
	} else {
		va_start = ioremap(mem->pa_start, mem->pa_end - mem->pa_start);
	}

	if (!va_start) {
		pr_err("omap_hwmod: %s: Could not ioremap\n", oh->name);
		return;
	}

	pr_debug("omap_hwmod: %s: MPU register target at va %p\n",
		 oh->name, va_start);

	oh->_mpu_rt_va = va_start;
}

/* 
                                                     
                           
               
  
                                                                     
                                                             
                                                                 
                                                                      
                                                                     
                                                                 
           
 */
static int __init _init(struct omap_hwmod *oh, void *data)
{
	int r;

	if (oh->_state != _HWMOD_STATE_REGISTERED)
		return 0;

	if (oh->class->sysc)
		_init_mpu_rt_base(oh, NULL);

	r = _init_clocks(oh, NULL);
	if (r < 0) {
		WARN(1, "omap_hwmod: %s: couldn't init clocks\n", oh->name);
		return -EINVAL;
	}

	oh->_state = _HWMOD_STATE_INITIALIZED;

	return 0;
}

/* 
                                                                  
                           
  
                                                                           
                                                                           
                                     
 */
static void __init _setup_iclk_autoidle(struct omap_hwmod *oh)
{
	struct omap_hwmod_ocp_if *os;
	struct list_head *p;
	int i = 0;
	if (oh->_state != _HWMOD_STATE_INITIALIZED)
		return;

	p = oh->slave_ports.next;

	while (i < oh->slaves_cnt) {
		os = _fetch_next_ocp_if(&p, &i);
		if (!os->_clk)
			continue;

		if (os->flags & OCPIF_SWSUP_IDLE) {
			/*                             */
		} else {
			/*                              */
			clk_enable(os->_clk);
		}
	}

	return;
}

/* 
                                                            
                           
  
                                                                     
                                                                    
                                                               
           
 */
static int __init _setup_reset(struct omap_hwmod *oh)
{
	int r;

	if (oh->_state != _HWMOD_STATE_INITIALIZED)
		return -EINVAL;

	if (oh->flags & HWMOD_EXT_OPT_MAIN_CLK)
		return -EPERM;

	if (oh->rst_lines_cnt == 0) {
		r = _enable(oh);
		if (r) {
			pr_warning("omap_hwmod: %s: cannot be enabled for reset (%d)\n",
				   oh->name, oh->_state);
			return -EINVAL;
		}
	}

	if (!(oh->flags & HWMOD_INIT_NO_RESET))
		r = _reset(oh);

	return r;
}

/* 
                                                                      
                           
  
                                                                    
                                                                      
                                                                  
                                                  
                                                                
                                                                     
            
  
                                                                      
                                                           
                                                               
                                                                   
                                                                 
                                                                      
                                                                    
                                                                     
                          
  
                                                                 
                                                                   
                                                                      
                                                                   
                                                                  
                                                                     
                                                                 
                                                           
                                                                   
                                                                   
                         
  
                   
 */
static void __init _setup_postsetup(struct omap_hwmod *oh)
{
	u8 postsetup_state;

	if (oh->rst_lines_cnt > 0)
		return;

	postsetup_state = oh->_postsetup_state;
	if (postsetup_state == _HWMOD_STATE_UNKNOWN)
		postsetup_state = _HWMOD_STATE_ENABLED;

	/*
                                                          
                                                                      
  */
	if ((oh->flags & HWMOD_INIT_NO_IDLE) &&
	    (postsetup_state == _HWMOD_STATE_IDLE)) {
		oh->_int_flags |= _HWMOD_SKIP_ENABLE;
		postsetup_state = _HWMOD_STATE_ENABLED;
	}

	if (postsetup_state == _HWMOD_STATE_IDLE)
		_idle(oh);
	else if (postsetup_state == _HWMOD_STATE_DISABLED)
		_shutdown(oh);
	else if (postsetup_state != _HWMOD_STATE_ENABLED)
		WARN(1, "hwmod: %s: unknown postsetup state %d! defaulting to enabled\n",
		     oh->name, postsetup_state);

	return;
}

/* 
                                             
                           
                          
  
                                                               
                                                             
                                                                     
                                                                    
                                                                  
                                                                     
                                                                      
                                                                   
                                                                
                                            
 */
static int __init _setup(struct omap_hwmod *oh, void *data)
{
	if (oh->_state != _HWMOD_STATE_INITIALIZED)
		return 0;

	_setup_iclk_autoidle(oh);

	if (!_setup_reset(oh))
		_setup_postsetup(oh);

	return 0;
}

/* 
                                           
                           
  
                                                                  
                                                               
                                                           
                                                                   
                                                                  
           
  
                                                                   
                                                                      
                                                                       
                                                                            
                    
 */
static int __init _register(struct omap_hwmod *oh)
{
	if (!oh || !oh->name || !oh->class || !oh->class->name ||
	    (oh->_state != _HWMOD_STATE_UNKNOWN))
		return -EINVAL;

	pr_debug("omap_hwmod: %s: registering\n", oh->name);

	if (_lookup(oh->name))
		return -EEXIST;

	list_add_tail(&oh->node, &omap_hwmod_list);

	INIT_LIST_HEAD(&oh->master_ports);
	INIT_LIST_HEAD(&oh->slave_ports);
	spin_lock_init(&oh->_lock);

	oh->_state = _HWMOD_STATE_REGISTERED;

	/*
                                                             
                                                              
  */
	if (!strcmp(oh->name, MPU_INITIATOR_NAME))
		mpu_oh = oh;

	return 0;
}

/* 
                                                         
                                                                 
                                                                
  
                                                                 
                                                                     
                                                                     
                                                                   
                                                               
                                                              
                       
 */
static int __init _alloc_links(struct omap_hwmod_link **ml,
			       struct omap_hwmod_link **sl)
{
	unsigned int sz;

	if ((free_ls + LINKS_PER_OCP_IF) <= max_ls) {
		*ml = &linkspace[free_ls++];
		*sl = &linkspace[free_ls++];
		return 0;
	}

	sz = sizeof(struct omap_hwmod_link) * LINKS_PER_OCP_IF;

	*sl = NULL;
	*ml = alloc_bootmem(sz);

	memset(*ml, 0, sz);

	*sl = (void *)(*ml) + sizeof(struct omap_hwmod_link);

	ls_supp++;
	pr_debug("omap_hwmod: supplemental link allocations needed: %d\n",
		 ls_supp * LINKS_PER_OCP_IF);

	return 0;
};

/* 
                                                        
                                                    
  
                                                                    
                                                                     
                                                                      
                                                                 
                                                                 
                                  
 */
static int __init _add_link(struct omap_hwmod_ocp_if *oi)
{
	struct omap_hwmod_link *ml, *sl;

	pr_debug("omap_hwmod: %s -> %s: adding link\n", oi->master->name,
		 oi->slave->name);

	_alloc_links(&ml, &sl);

	ml->ocp_if = oi;
	INIT_LIST_HEAD(&ml->node);
	list_add(&ml->node, &oi->master->master_ports);
	oi->master->masters_cnt++;

	sl->ocp_if = oi;
	INIT_LIST_HEAD(&sl->node);
	list_add(&sl->node, &oi->slave->slave_ports);
	oi->slave->slaves_cnt++;

	return 0;
}

/* 
                                                       
                                  
  
                                                                     
                                                                
                                                                 
           
  
                                                                   
                                                                      
                                                                 
 */
static int __init _register_link(struct omap_hwmod_ocp_if *oi)
{
	if (!oi || !oi->master || !oi->slave || !oi->user)
		return -EINVAL;

	if (oi->_int_flags & _OCPIF_INT_FLAGS_REGISTERED)
		return -EEXIST;

	pr_debug("omap_hwmod: registering link from %s to %s\n",
		 oi->master->name, oi->slave->name);

	/*
                                                       
                      
  */
	if (oi->master->_state != _HWMOD_STATE_REGISTERED)
		_register(oi->master);

	if (oi->slave->_state != _HWMOD_STATE_REGISTERED)
		_register(oi->slave);

	_add_link(oi);

	oi->_int_flags |= _OCPIF_INT_FLAGS_REGISTERED;

	return 0;
}

/* 
                                                         
                                                                         
  
                                                                  
                                                                    
                                                              
                                                                      
                                                                    
                                                                   
                                                                   
                                                                    
                                                                          
                     
 */
static int __init _alloc_linkspace(struct omap_hwmod_ocp_if **ois)
{
	unsigned int i = 0;
	unsigned int sz;

	if (linkspace) {
		WARN(1, "linkspace already allocated\n");
		return -EEXIST;
	}

	if (max_ls == 0)
		while (ois[i++])
			max_ls += LINKS_PER_OCP_IF;

	sz = sizeof(struct omap_hwmod_link) * max_ls;

	pr_debug("omap_hwmod: %s: allocating %d byte linkspace (%d links)\n",
		 __func__, sz, max_ls);

	linkspace = alloc_bootmem(sz);

	memset(linkspace, 0, sz);

	return 0;
}

/*                                                                           */

/* 
                                                                      
                           
  
                                                                      
                                                                   
                                                            
                                                  
 */
static int _omap2xxx_wait_target_ready(struct omap_hwmod *oh)
{
	if (!oh)
		return -EINVAL;

	if (oh->flags & HWMOD_NO_IDLEST)
		return 0;

	if (!_find_mpu_rt_port(oh))
		return 0;

	/*                                                              */

	return omap2xxx_cm_wait_module_ready(oh->prcm.omap2.module_offs,
					     oh->prcm.omap2.idlest_reg_id,
					     oh->prcm.omap2.idlest_idle_bit);
}

/* 
                                                                      
                           
  
                                                                      
                                                                   
                                                            
                                                  
 */
static int _omap3xxx_wait_target_ready(struct omap_hwmod *oh)
{
	if (!oh)
		return -EINVAL;

	if (oh->flags & HWMOD_NO_IDLEST)
		return 0;

	if (!_find_mpu_rt_port(oh))
		return 0;

	/*                                                              */

	return omap3xxx_cm_wait_module_ready(oh->prcm.omap2.module_offs,
					     oh->prcm.omap2.idlest_reg_id,
					     oh->prcm.omap2.idlest_idle_bit);
}

/* 
                                                                   
                           
  
                                                                      
                                                                   
                                                            
                                                  
 */
static int _omap4_wait_target_ready(struct omap_hwmod *oh)
{
	if (!oh)
		return -EINVAL;

	if (oh->flags & HWMOD_NO_IDLEST || !oh->clkdm)
		return 0;

	if (!_find_mpu_rt_port(oh))
		return 0;

	/*                                              */

	return omap4_cminst_wait_module_ready(oh->clkdm->prcm_partition,
					      oh->clkdm->cm_inst,
					      oh->clkdm->clkdm_offs,
					      oh->prcm.omap4.clkctrl_offs);
}

/* 
                                                                    
                           
  
                                                                      
                                                                   
                                                            
                                                  
 */
static int _am33xx_wait_target_ready(struct omap_hwmod *oh)
{
	if (!oh || !oh->clkdm)
		return -EINVAL;

	if (oh->flags & HWMOD_NO_IDLEST)
		return 0;

	if (!_find_mpu_rt_port(oh))
		return 0;

	/*                                              */

	return am33xx_cm_wait_module_ready(oh->clkdm->cm_inst,
					      oh->clkdm->clkdm_offs,
					      oh->prcm.omap4.clkctrl_offs);
}

/* 
                                                                        
                                               
                             
  
                                                                   
                                                                      
                                                                     
                                                                     
                                                        
 */
static int _omap2_assert_hardreset(struct omap_hwmod *oh,
				   struct omap_hwmod_rst_info *ohri)
{
	return omap2_prm_assert_hardreset(oh->prcm.omap2.module_offs,
					  ohri->rst_shift);
}

/* 
                                                                          
                                                 
                             
  
                                                                     
                                                                      
                                                                     
                                                             
                                                                  
 */
static int _omap2_deassert_hardreset(struct omap_hwmod *oh,
				     struct omap_hwmod_rst_info *ohri)
{
	return omap2_prm_deassert_hardreset(oh->prcm.omap2.module_offs,
					    ohri->rst_shift,
					    ohri->st_shift);
}

/* 
                                                                             
                                             
                             
  
                                                                   
                                                              
                                                                     
                                                                 
                                                                    
            
 */
static int _omap2_is_hardreset_asserted(struct omap_hwmod *oh,
					struct omap_hwmod_rst_info *ohri)
{
	return omap2_prm_is_hardreset_asserted(oh->prcm.omap2.module_offs,
					       ohri->st_shift);
}

/* 
                                                                        
                                               
                             
  
                                                                  
                                                              
                                                                     
                                                                
                                                                    
            
 */
static int _omap4_assert_hardreset(struct omap_hwmod *oh,
				   struct omap_hwmod_rst_info *ohri)
{
	if (!oh->clkdm)
		return -EINVAL;

	return omap4_prminst_assert_hardreset(ohri->rst_shift,
				oh->clkdm->pwrdm.ptr->prcm_partition,
				oh->clkdm->pwrdm.ptr->prcm_offs,
				oh->prcm.omap4.rstctrl_offs);
}

/* 
                                                                          
                                                 
                             
  
                                                                    
                                                              
                                                                     
                                                                  
                                                                    
            
 */
static int _omap4_deassert_hardreset(struct omap_hwmod *oh,
				     struct omap_hwmod_rst_info *ohri)
{
	if (!oh->clkdm)
		return -EINVAL;

	if (ohri->st_shift)
		pr_err("omap_hwmod: %s: %s: hwmod data error: OMAP4 does not support st_shift\n",
		       oh->name, ohri->name);
	return omap4_prminst_deassert_hardreset(ohri->rst_shift,
				oh->clkdm->pwrdm.ptr->prcm_partition,
				oh->clkdm->pwrdm.ptr->prcm_offs,
				oh->prcm.omap4.rstctrl_offs);
}

/* 
                                                                             
                                             
                             
  
                                                             
                                                                  
                                                                      
                                                                    
                                                                    
                 
 */
static int _omap4_is_hardreset_asserted(struct omap_hwmod *oh,
					struct omap_hwmod_rst_info *ohri)
{
	if (!oh->clkdm)
		return -EINVAL;

	return omap4_prminst_is_hardreset_asserted(ohri->rst_shift,
				oh->clkdm->pwrdm.ptr->prcm_partition,
				oh->clkdm->pwrdm.ptr->prcm_offs,
				oh->prcm.omap4.rstctrl_offs);
}

/* 
                                                                          
                                               
                             
  
                                                                   
                                                              
                                                                     
                                                                 
                                                                    
            
 */
static int _am33xx_assert_hardreset(struct omap_hwmod *oh,
				   struct omap_hwmod_rst_info *ohri)

{
	return am33xx_prm_assert_hardreset(ohri->rst_shift,
				oh->clkdm->pwrdm.ptr->prcm_offs,
				oh->prcm.omap4.rstctrl_offs);
}

/* 
                                                                            
                                                 
                             
  
                                                                     
                                                              
                                                                     
                                                                   
                                                                    
            
 */
static int _am33xx_deassert_hardreset(struct omap_hwmod *oh,
				     struct omap_hwmod_rst_info *ohri)
{
	return am33xx_prm_deassert_hardreset(ohri->rst_shift,
				ohri->st_shift,
				oh->clkdm->pwrdm.ptr->prcm_offs,
				oh->prcm.omap4.rstctrl_offs,
				oh->prcm.omap4.rstst_offs);
}

/* 
                                                                               
                                             
                             
  
                                                              
                                                                  
                                                                      
                                                                     
                                                                    
                 
 */
static int _am33xx_is_hardreset_asserted(struct omap_hwmod *oh,
					struct omap_hwmod_rst_info *ohri)
{
	return am33xx_prm_is_hardreset_asserted(ohri->rst_shift,
				oh->clkdm->pwrdm.ptr->prcm_offs,
				oh->prcm.omap4.rstctrl_offs);
}

/*                  */

u32 omap_hwmod_read(struct omap_hwmod *oh, u16 reg_offs)
{
	if (oh->flags & HWMOD_16BIT_REG)
		return __raw_readw(oh->_mpu_rt_va + reg_offs);
	else
		return __raw_readl(oh->_mpu_rt_va + reg_offs);
}

void omap_hwmod_write(u32 v, struct omap_hwmod *oh, u16 reg_offs)
{
	if (oh->flags & HWMOD_16BIT_REG)
		__raw_writew(v, oh->_mpu_rt_va + reg_offs);
	else
		__raw_writel(v, oh->_mpu_rt_va + reg_offs);
}

/* 
                                                                    
                           
  
                                                                            
                                                                            
                                                                           
                                       
 */
int omap_hwmod_softreset(struct omap_hwmod *oh)
{
	u32 v;
	int ret;

	if (!oh || !(oh->_sysc_cache))
		return -EINVAL;

	v = oh->_sysc_cache;
	ret = _set_softreset(oh, &v);
	if (ret)
		goto error;
	_write_sysconfig(v, oh);

	ret = _clear_softreset(oh, &v);
	if (ret)
		goto error;
	_write_sysconfig(v, oh);

error:
	return ret;
}

/* 
                                                              
                                           
  
                                                                     
                                           
 */
struct omap_hwmod *omap_hwmod_lookup(const char *name)
{
	struct omap_hwmod *oh;

	if (!name)
		return NULL;

	oh = _lookup(name);

	return oh;
}

/* 
                                                                     
                                      
                                                  
  
                                                                 
                                                                  
                                                                      
                                                                
                                                       
                              
 */
int omap_hwmod_for_each(int (*fn)(struct omap_hwmod *oh, void *data),
			void *data)
{
	struct omap_hwmod *temp_oh;
	int ret = 0;

	if (!fn)
		return -EINVAL;

	list_for_each_entry(temp_oh, &omap_hwmod_list, node) {
		ret = (*fn)(temp_oh, data);
		if (ret)
			break;
	}

	return ret;
}

/* 
                                                               
                                                             
  
                                                                    
                                                                   
                                                                   
                                                                     
                                                                
           
 */
int __init omap_hwmod_register_links(struct omap_hwmod_ocp_if **ois)
{
	int r, i;

	if (!inited)
		return -EINVAL;

	if (!ois)
		return 0;

	if (!linkspace) {
		if (_alloc_linkspace(ois)) {
			pr_err("omap_hwmod: could not allocate link space\n");
			return -ENOMEM;
		}
	}

	i = 0;
	do {
		r = _register_link(ois[i]);
		WARN(r && r != -EEXIST,
		     "omap_hwmod: _register_link(%s -> %s) returned %d\n",
		     ois[i]->master->name, ois[i]->slave->name, r);
	} while (ois[++i]);

	return 0;
}

/* 
                                                                             
                                                                         
  
                                                                
                                                                   
                                                                     
                                                                      
                
 */
static void __init _ensure_mpu_hwmod_is_setup(struct omap_hwmod *oh)
{
	if (!mpu_oh || mpu_oh->_state == _HWMOD_STATE_UNKNOWN)
		pr_err("omap_hwmod: %s: MPU initiator hwmod %s not yet registered\n",
		       __func__, MPU_INITIATOR_NAME);
	else if (mpu_oh->_state == _HWMOD_STATE_REGISTERED && oh != mpu_oh)
		omap_hwmod_setup_one(MPU_INITIATOR_NAME);
}

/* 
                                               
                                                                        
  
                                                                   
                                                                      
                                                               
                                                                
                                                                      
                                        
 */
int __init omap_hwmod_setup_one(const char *oh_name)
{
	struct omap_hwmod *oh;

	pr_debug("omap_hwmod: %s: %s\n", oh_name, __func__);

	oh = _lookup(oh_name);
	if (!oh) {
		WARN(1, "omap_hwmod: %s: hwmod not yet registered\n", oh_name);
		return -EINVAL;
	}

	_ensure_mpu_hwmod_is_setup(oh);

	_init(oh, NULL);
	_setup(oh, NULL);

	return 0;
}

/* 
                                                         
  
                                                                      
                                                                  
                                                                     
                                                         
 */
static int __init omap_hwmod_setup_all(void)
{
	_ensure_mpu_hwmod_is_setup(NULL);

	omap_hwmod_for_each(_init, NULL);
	omap_hwmod_for_each(_setup, NULL);

	return 0;
}
omap_core_initcall(omap_hwmod_setup_all);

/* 
                                           
                           
  
                                                                            
                                                                            
 */
int omap_hwmod_enable(struct omap_hwmod *oh)
{
	int r;
	unsigned long flags;

	if (!oh)
		return -EINVAL;

	spin_lock_irqsave(&oh->_lock, flags);
	r = _enable(oh);
	spin_unlock_irqrestore(&oh->_lock, flags);

	return r;
}

/* 
                                       
                           
  
                                                                        
                                                                          
 */
int omap_hwmod_idle(struct omap_hwmod *oh)
{
	unsigned long flags;

	if (!oh)
		return -EINVAL;

	spin_lock_irqsave(&oh->_lock, flags);
	_idle(oh);
	spin_unlock_irqrestore(&oh->_lock, flags);

	return 0;
}

/* 
                                               
                           
  
                                                        
                                                                    
                                     
 */
int omap_hwmod_shutdown(struct omap_hwmod *oh)
{
	unsigned long flags;

	if (!oh)
		return -EINVAL;

	spin_lock_irqsave(&oh->_lock, flags);
	_shutdown(oh);
	spin_unlock_irqrestore(&oh->_lock, flags);

	return 0;
}

/* 
                                                                   
                             
  
                                                 
 */
int omap_hwmod_enable_clocks(struct omap_hwmod *oh)
{
	unsigned long flags;

	spin_lock_irqsave(&oh->_lock, flags);
	_enable_clocks(oh);
	spin_unlock_irqrestore(&oh->_lock, flags);

	return 0;
}

/* 
                                                                     
                             
  
                                                 
 */
int omap_hwmod_disable_clocks(struct omap_hwmod *oh)
{
	unsigned long flags;

	spin_lock_irqsave(&oh->_lock, flags);
	_disable_clocks(oh);
	spin_unlock_irqrestore(&oh->_lock, flags);

	return 0;
}

/* 
                                                                                
                             
  
                                                                 
                                                             
                                                               
                 
  
                                                    
 */
void omap_hwmod_ocp_barrier(struct omap_hwmod *oh)
{
	BUG_ON(!oh);

	if (!oh->class->sysc || !oh->class->sysc->sysc_flags) {
		WARN(1, "omap_device: %s: OCP barrier impossible due to device configuration\n",
			oh->name);
		return;
	}

	/*
                                                               
                   
  */
	omap_hwmod_read(oh, oh->class->sysc->sysc_offs);
}

/* 
                                     
                           
  
                                                                       
                                                                          
                                  
 */
int omap_hwmod_reset(struct omap_hwmod *oh)
{
	int r;
	unsigned long flags;

	if (!oh)
		return -EINVAL;

	spin_lock_irqsave(&oh->_lock, flags);
	r = _reset(oh);
	spin_unlock_irqrestore(&oh->_lock, flags);

	return r;
}

/*
                                    
 */

/* 
                                                                                
                           
                                                                   
  
                                                                  
                                                                   
                                                                     
                                                                
                                                                     
                                  
  
                                                                      
                
  
 */
int omap_hwmod_count_resources(struct omap_hwmod *oh, unsigned long flags)
{
	int ret = 0;

	if (flags & IORESOURCE_IRQ)
		ret += _count_mpu_irqs(oh);

	if (flags & IORESOURCE_DMA)
		ret += _count_sdma_reqs(oh);

	if (flags & IORESOURCE_MEM) {
		int i = 0;
		struct omap_hwmod_ocp_if *os;
		struct list_head *p = oh->slave_ports.next;

		while (i < oh->slaves_cnt) {
			os = _fetch_next_ocp_if(&p, &i);
			ret += _count_ocp_if_addr_spaces(os);
		}
	}

	return ret;
}

/* 
                                                                         
                           
                                                                            
  
                                                                  
                                                                
                                                                     
                                   
 */
int omap_hwmod_fill_resources(struct omap_hwmod *oh, struct resource *res)
{
	struct omap_hwmod_ocp_if *os;
	struct list_head *p;
	int i, j, mpu_irqs_cnt, sdma_reqs_cnt, addr_cnt;
	int r = 0;

	/*                                               */

	mpu_irqs_cnt = _count_mpu_irqs(oh);
	for (i = 0; i < mpu_irqs_cnt; i++) {
		(res + r)->name = (oh->mpu_irqs + i)->name;
		(res + r)->start = (oh->mpu_irqs + i)->irq;
		(res + r)->end = (oh->mpu_irqs + i)->irq;
		(res + r)->flags = IORESOURCE_IRQ;
		r++;
	}

	sdma_reqs_cnt = _count_sdma_reqs(oh);
	for (i = 0; i < sdma_reqs_cnt; i++) {
		(res + r)->name = (oh->sdma_reqs + i)->name;
		(res + r)->start = (oh->sdma_reqs + i)->dma_req;
		(res + r)->end = (oh->sdma_reqs + i)->dma_req;
		(res + r)->flags = IORESOURCE_DMA;
		r++;
	}

	p = oh->slave_ports.next;

	i = 0;
	while (i < oh->slaves_cnt) {
		os = _fetch_next_ocp_if(&p, &i);
		addr_cnt = _count_ocp_if_addr_spaces(os);

		for (j = 0; j < addr_cnt; j++) {
			(res + r)->name = (os->addr + j)->name;
			(res + r)->start = (os->addr + j)->pa_start;
			(res + r)->end = (os->addr + j)->pa_end;
			(res + r)->flags = IORESOURCE_MEM;
			r++;
		}
	}

	return r;
}

/* 
                                                                           
                           
                                                        
  
                                                                      
                                                                
                                                                     
                                   
 */
int omap_hwmod_fill_dma_resources(struct omap_hwmod *oh, struct resource *res)
{
	int i, sdma_reqs_cnt;
	int r = 0;

	sdma_reqs_cnt = _count_sdma_reqs(oh);
	for (i = 0; i < sdma_reqs_cnt; i++) {
		(res + r)->name = (oh->sdma_reqs + i)->name;
		(res + r)->start = (oh->sdma_reqs + i)->dma_req;
		(res + r)->end = (oh->sdma_reqs + i)->dma_req;
		(res + r)->flags = IORESOURCE_DMA;
		r++;
	}

	return r;
}

/* 
                                                                           
                                         
                                                                       
                                                             
                                                               
  
                                                                  
                                                                    
                                                                 
                                                                     
                                                                      
                                                                   
                                                                    
                                                                      
                                                                  
                                                                 
                                                               
                                                                      
                                                                  
                                               
 */
int omap_hwmod_get_resource_byname(struct omap_hwmod *oh, unsigned int type,
				   const char *name, struct resource *rsrc)
{
	int r;
	unsigned int irq, dma;
	u32 pa_start, pa_end;

	if (!oh || !rsrc)
		return -EINVAL;

	if (type == IORESOURCE_IRQ) {
		r = _get_mpu_irq_by_name(oh, name, &irq);
		if (r)
			return r;

		rsrc->start = irq;
		rsrc->end = irq;
	} else if (type == IORESOURCE_DMA) {
		r = _get_sdma_req_by_name(oh, name, &dma);
		if (r)
			return r;

		rsrc->start = dma;
		rsrc->end = dma;
	} else if (type == IORESOURCE_MEM) {
		r = _get_addr_space_by_name(oh, name, &pa_start, &pa_end);
		if (r)
			return r;

		rsrc->start = pa_start;
		rsrc->end = pa_end;
	} else {
		return -EINVAL;
	}

	rsrc->flags = type;
	rsrc->name = name;

	return 0;
}

/* 
                                                                          
                           
  
                                                                 
                                                                 
                                                                      
                                                                
                                                                 
           
 */
struct powerdomain *omap_hwmod_get_pwrdm(struct omap_hwmod *oh)
{
	struct clk *c;
	struct omap_hwmod_ocp_if *oi;
	struct clockdomain *clkdm;
	struct clk_hw_omap *clk;

	if (!oh)
		return NULL;

	if (oh->clkdm)
		return oh->clkdm->pwrdm.ptr;

	if (oh->_clk) {
		c = oh->_clk;
	} else {
		oi = _find_mpu_rt_port(oh);
		if (!oi)
			return NULL;
		c = oi->_clk;
	}

	clk = to_clk_hw_omap(__clk_get_hw(c));
	clkdm = clk->clkdm;
	if (!clkdm)
		return NULL;

	return clkdm->pwrdm.ptr;
}

/* 
                                                                            
                           
  
                                                                    
                                                                     
                                                                        
              
 */
void __iomem *omap_hwmod_get_mpu_rt_va(struct omap_hwmod *oh)
{
	if (!oh)
		return NULL;

	if (oh->_int_flags & _HWMOD_NO_MPU_PORT)
		return NULL;

	if (oh->_state == _HWMOD_STATE_UNKNOWN)
		return NULL;

	return oh->_mpu_rt_va;
}

/* 
                                                                   
                           
                                            
  
                                                                 
                                                                     
                                                                     
                                                           
                                                                  
                        
  
                                              
 */
int omap_hwmod_add_initiator_dep(struct omap_hwmod *oh,
				 struct omap_hwmod *init_oh)
{
	return _add_initiator_dep(oh, init_oh);
}

/*
                                                                     
                                       
 */

/* 
                                                                      
                           
                                            
  
                                                                    
                                                                     
                                                                     
                                                           
                                                                  
                        
  
                                              
 */
int omap_hwmod_del_initiator_dep(struct omap_hwmod *oh,
				 struct omap_hwmod *init_oh)
{
	return _del_initiator_dep(oh, init_oh);
}

/* 
                                                                
                           
  
                                                                  
                                                                  
                                                                
                                                                
                                                                  
                                                                      
                                                               
                                                                 
 */
int omap_hwmod_enable_wakeup(struct omap_hwmod *oh)
{
	unsigned long flags;
	u32 v;

	spin_lock_irqsave(&oh->_lock, flags);

	if (oh->class->sysc &&
	    (oh->class->sysc->sysc_flags & SYSC_HAS_ENAWAKEUP)) {
		v = oh->_sysc_cache;
		_enable_wakeup(oh, &v);
		_write_sysconfig(v, oh);
	}

	_set_idle_ioring_wakeup(oh, true);
	spin_unlock_irqrestore(&oh->_lock, flags);

	return 0;
}

/* 
                                                                    
                           
  
                                                                   
                                                                
                                                                      
                                                                      
                                                                  
                                                                      
                                                               
                                                                 
 */
int omap_hwmod_disable_wakeup(struct omap_hwmod *oh)
{
	unsigned long flags;
	u32 v;

	spin_lock_irqsave(&oh->_lock, flags);

	if (oh->class->sysc &&
	    (oh->class->sysc->sysc_flags & SYSC_HAS_ENAWAKEUP)) {
		v = oh->_sysc_cache;
		_disable_wakeup(oh, &v);
		_write_sysconfig(v, oh);
	}

	_set_idle_ioring_wakeup(oh, false);
	spin_unlock_irqrestore(&oh->_lock, flags);

	return 0;
}

/* 
                                                                       
                                 
                           
                                                     
  
                                                              
                                                                    
                                                                     
                                                                       
                            
 */
int omap_hwmod_assert_hardreset(struct omap_hwmod *oh, const char *name)
{
	int ret;
	unsigned long flags;

	if (!oh)
		return -EINVAL;

	spin_lock_irqsave(&oh->_lock, flags);
	ret = _assert_hardreset(oh, name);
	spin_unlock_irqrestore(&oh->_lock, flags);

	return ret;
}

/* 
                                                                           
                                 
                           
                                                        
  
                                                              
                                                                    
                                                                     
                                                                       
                              
 */
int omap_hwmod_deassert_hardreset(struct omap_hwmod *oh, const char *name)
{
	int ret;
	unsigned long flags;

	if (!oh)
		return -EINVAL;

	spin_lock_irqsave(&oh->_lock, flags);
	ret = _deassert_hardreset(oh, name);
	spin_unlock_irqrestore(&oh->_lock, flags);

	return ret;
}

/* 
                                                                         
                                
                           
                                                    
  
                                                                      
                                                            
                                                                         
                                
 */
int omap_hwmod_read_hardreset(struct omap_hwmod *oh, const char *name)
{
	int ret;
	unsigned long flags;

	if (!oh)
		return -EINVAL;

	spin_lock_irqsave(&oh->_lock, flags);
	ret = _read_hardreset(oh, name);
	spin_unlock_irqrestore(&oh->_lock, flags);

	return ret;
}


/* 
                                                                             
                                                         
                                                                            
                                                                 
  
                                                     
                                                        
                                                                       
                                                                       
                                                                         
 */
int omap_hwmod_for_each_by_class(const char *classname,
				 int (*fn)(struct omap_hwmod *oh,
					   void *user),
				 void *user)
{
	struct omap_hwmod *temp_oh;
	int ret = 0;

	if (!classname || !fn)
		return -EINVAL;

	pr_debug("omap_hwmod: %s: looking for modules of class %s\n",
		 __func__, classname);

	list_for_each_entry(temp_oh, &omap_hwmod_list, node) {
		if (!strcmp(temp_oh->class->name, classname)) {
			pr_debug("omap_hwmod: %s: %s: calling callback fn\n",
				 __func__, temp_oh->name);
			ret = (*fn)(temp_oh, user);
			if (ret)
				break;
		}
	}

	if (ret)
		pr_debug("omap_hwmod: %s: iterator terminated early: %d\n",
			 __func__, ret);

	return ret;
}

/* 
                                                                              
                           
                                                        
  
                                                                  
                                                                
                                                            
                                                                      
                      
 */
int omap_hwmod_set_postsetup_state(struct omap_hwmod *oh, u8 state)
{
	int ret;
	unsigned long flags;

	if (!oh)
		return -EINVAL;

	if (state != _HWMOD_STATE_DISABLED &&
	    state != _HWMOD_STATE_ENABLED &&
	    state != _HWMOD_STATE_IDLE)
		return -EINVAL;

	spin_lock_irqsave(&oh->_lock, flags);

	if (oh->_state != _HWMOD_STATE_REGISTERED) {
		ret = -EINVAL;
		goto ohsps_unlock;
	}

	oh->_postsetup_state = state;
	ret = 0;

ohsps_unlock:
	spin_unlock_irqrestore(&oh->_lock, flags);

	return ret;
}

/* 
                                                             
                           
  
                                                   
                                                              
  
                                                              
                                                                
                                            
 */
int omap_hwmod_get_context_loss_count(struct omap_hwmod *oh)
{
	struct powerdomain *pwrdm;
	int ret = 0;

	if (soc_ops.get_context_lost)
		return soc_ops.get_context_lost(oh);

	pwrdm = omap_hwmod_get_pwrdm(oh);
	if (pwrdm)
		ret = pwrdm_get_context_loss_count(pwrdm);

	return ret;
}

/* 
                                                                          
                           
  
                                                                   
                                                                  
                                                                    
                                                                   
           
 */
int omap_hwmod_no_setup_reset(struct omap_hwmod *oh)
{
	if (!oh)
		return -EINVAL;

	if (oh->_state != _HWMOD_STATE_REGISTERED) {
		pr_err("omap_hwmod: %s: cannot prevent setup reset; in wrong state\n",
			oh->name);
		return -EINVAL;
	}

	oh->flags |= HWMOD_INIT_NO_RESET;

	return 0;
}

/* 
                                                                             
                                                        
                                                                          
                                                                           
  
                                                                     
                                                                 
                                                                     
                                                                
                                                                    
                                                                     
                                                                
                                                                     
                                                                   
                                                                     
                                                                     
                                                                 
                                                                    
  
                                                                   
                                                                   
                                                                     
               
 */
int omap_hwmod_pad_route_irq(struct omap_hwmod *oh, int pad_idx, int irq_idx)
{
	int nr_irqs;

	might_sleep();

	if (!oh || !oh->mux || !oh->mpu_irqs || pad_idx < 0 ||
	    pad_idx >= oh->mux->nr_pads_dynamic)
		return -EINVAL;

	/*                                        */
	for (nr_irqs = 0; oh->mpu_irqs[nr_irqs].irq >= 0; nr_irqs++)
		;

	if (irq_idx >= nr_irqs)
		return -EINVAL;

	if (!oh->mux->irqs) {
		/*                      */
		oh->mux->irqs = kzalloc(sizeof(int) * oh->mux->nr_pads_dynamic,
			GFP_KERNEL);
		if (!oh->mux->irqs)
			return -ENOMEM;
	}
	oh->mux->irqs[pad_idx] = irq_idx;

	return 0;
}

/* 
                                              
  
                                                                            
                                                                       
                                                      
 */
void __init omap_hwmod_init(void)
{
	if (cpu_is_omap24xx()) {
		soc_ops.wait_target_ready = _omap2xxx_wait_target_ready;
		soc_ops.assert_hardreset = _omap2_assert_hardreset;
		soc_ops.deassert_hardreset = _omap2_deassert_hardreset;
		soc_ops.is_hardreset_asserted = _omap2_is_hardreset_asserted;
	} else if (cpu_is_omap34xx()) {
		soc_ops.wait_target_ready = _omap3xxx_wait_target_ready;
		soc_ops.assert_hardreset = _omap2_assert_hardreset;
		soc_ops.deassert_hardreset = _omap2_deassert_hardreset;
		soc_ops.is_hardreset_asserted = _omap2_is_hardreset_asserted;
	} else if (cpu_is_omap44xx() || soc_is_omap54xx()) {
		soc_ops.enable_module = _omap4_enable_module;
		soc_ops.disable_module = _omap4_disable_module;
		soc_ops.wait_target_ready = _omap4_wait_target_ready;
		soc_ops.assert_hardreset = _omap4_assert_hardreset;
		soc_ops.deassert_hardreset = _omap4_deassert_hardreset;
		soc_ops.is_hardreset_asserted = _omap4_is_hardreset_asserted;
		soc_ops.init_clkdm = _init_clkdm;
		soc_ops.update_context_lost = _omap4_update_context_lost;
		soc_ops.get_context_lost = _omap4_get_context_lost;
	} else if (soc_is_am33xx()) {
		soc_ops.enable_module = _am33xx_enable_module;
		soc_ops.disable_module = _am33xx_disable_module;
		soc_ops.wait_target_ready = _am33xx_wait_target_ready;
		soc_ops.assert_hardreset = _am33xx_assert_hardreset;
		soc_ops.deassert_hardreset = _am33xx_deassert_hardreset;
		soc_ops.is_hardreset_asserted = _am33xx_is_hardreset_asserted;
		soc_ops.init_clkdm = _init_clkdm;
	} else {
		WARN(1, "omap_hwmod: unknown SoC type\n");
	}

	inited = true;
}

/* 
                                                           
                           
  
                                                               
                          
 */
const char *omap_hwmod_get_main_clk(struct omap_hwmod *oh)
{
	if (!oh)
		return NULL;

	return oh->main_clk;
}
