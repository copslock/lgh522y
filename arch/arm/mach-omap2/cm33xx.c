/*
 * AM33XX CM functions
 *
 * Copyright (C) 2011-2012 Texas Instruments Incorporated - http://www.ti.com/
 * Vaibhav Hiremath <hvaibhav@ti.com>
 *
 * Reference taken from from OMAP4 cminst44xx.c
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/io.h>

#include "clockdomain.h"
#include "cm.h"
#include "cm33xx.h"
#include "cm-regbits-34xx.h"
#include "cm-regbits-33xx.h"
#include "prm33xx.h"

/*
                                                                          
  
                                                            
                                                                              
                           
                                                                              
                                                  
                                                            
  
 */
#define CLKCTRL_IDLEST_FUNCTIONAL		0x0
#define CLKCTRL_IDLEST_INTRANSITION		0x1
#define CLKCTRL_IDLEST_INTERFACE_IDLE		0x2
#define CLKCTRL_IDLEST_DISABLED			0x3

/*                   */

/*                                  */
static inline u32 am33xx_cm_read_reg(s16 inst, u16 idx)
{
	return __raw_readl(cm_base + inst + idx);
}

/*                               */
static inline void am33xx_cm_write_reg(u32 val, s16 inst, u16 idx)
{
	__raw_writel(val, cm_base + inst + idx);
}

/*                                    */
static inline u32 am33xx_cm_rmw_reg_bits(u32 mask, u32 bits, s16 inst, s16 idx)
{
	u32 v;

	v = am33xx_cm_read_reg(inst, idx);
	v &= ~mask;
	v |= bits;
	am33xx_cm_write_reg(v, inst, idx);

	return v;
}

static inline u32 am33xx_cm_set_reg_bits(u32 bits, s16 inst, s16 idx)
{
	return am33xx_cm_rmw_reg_bits(bits, bits, inst, idx);
}

static inline u32 am33xx_cm_clear_reg_bits(u32 bits, s16 inst, s16 idx)
{
	return am33xx_cm_rmw_reg_bits(bits, 0x0, inst, idx);
}

static inline u32 am33xx_cm_read_reg_bits(u16 inst, s16 idx, u32 mask)
{
	u32 v;

	v = am33xx_cm_read_reg(inst, idx);
	v &= mask;
	v >>= __ffs(mask);

	return v;
}

/* 
                                                                               
                                                    
                                                        
                                                                        
  
                                                                         
         
 */
static u32 _clkctrl_idlest(u16 inst, s16 cdoffs, u16 clkctrl_offs)
{
	u32 v = am33xx_cm_read_reg(inst, clkctrl_offs);
	v &= AM33XX_IDLEST_MASK;
	v >>= AM33XX_IDLEST_SHIFT;
	return v;
}

/* 
                                                                                
                                                    
                                                        
                                                                        
  
                                                                      
                                                   
 */
static bool _is_module_ready(u16 inst, s16 cdoffs, u16 clkctrl_offs)
{
	u32 v;

	v = _clkctrl_idlest(inst, cdoffs, clkctrl_offs);

	return (v == CLKCTRL_IDLEST_FUNCTIONAL ||
		v == CLKCTRL_IDLEST_INTERFACE_IDLE) ? true : false;
}

/* 
                                                                            
                                                                 
                                                    
                                                        
  
                                                                     
                                
 */
static void _clktrctrl_write(u8 c, s16 inst, u16 cdoffs)
{
	u32 v;

	v = am33xx_cm_read_reg(inst, cdoffs);
	v &= ~AM33XX_CLKTRCTRL_MASK;
	v |= c << AM33XX_CLKTRCTRL_SHIFT;
	am33xx_cm_write_reg(v, inst, cdoffs);
}

/*                  */

/* 
                                                                     
                                                    
                                                        
  
                                                                  
                                                       
 */
bool am33xx_cm_is_clkdm_in_hwsup(s16 inst, u16 cdoffs)
{
	u32 v;

	v = am33xx_cm_read_reg(inst, cdoffs);
	v &= AM33XX_CLKTRCTRL_MASK;
	v >>= AM33XX_CLKTRCTRL_SHIFT;

	return (v == OMAP34XX_CLKSTCTRL_ENABLE_AUTO) ? true : false;
}

/* 
                                                                      
                                                    
                                                        
  
                                                         
                                                   
 */
void am33xx_cm_clkdm_enable_hwsup(s16 inst, u16 cdoffs)
{
	_clktrctrl_write(OMAP34XX_CLKSTCTRL_ENABLE_AUTO, inst, cdoffs);
}

/* 
                                                                       
                                                    
                                                        
  
                                                         
                                                                  
                                                 
 */
void am33xx_cm_clkdm_disable_hwsup(s16 inst, u16 cdoffs)
{
	_clktrctrl_write(OMAP34XX_CLKSTCTRL_DISABLE_AUTO, inst, cdoffs);
}

/* 
                                                                   
                                                    
                                                        
  
                                                              
                   
 */
void am33xx_cm_clkdm_force_sleep(s16 inst, u16 cdoffs)
{
	_clktrctrl_write(OMAP34XX_CLKSTCTRL_FORCE_SLEEP, inst, cdoffs);
}

/* 
                                                                       
                                                    
                                                        
  
                                                                  
                                  
 */
void am33xx_cm_clkdm_force_wakeup(s16 inst, u16 cdoffs)
{
	_clktrctrl_write(OMAP34XX_CLKSTCTRL_FORCE_WAKEUP, inst, cdoffs);
}

/*
  
 */

/* 
                                                                        
                                                    
                                                        
                                                                        
  
                                                                           
                                                                          
                                                                       
                  
 */
int am33xx_cm_wait_module_ready(u16 inst, s16 cdoffs, u16 clkctrl_offs)
{
	int i = 0;

	omap_test_timeout(_is_module_ready(inst, cdoffs, clkctrl_offs),
			  MAX_MODULE_READY_TIME, i);

	return (i < MAX_MODULE_READY_TIME) ? 0 : -EBUSY;
}

/* 
                                                                     
        
                                                    
                                                        
                                                                        
  
                                                                   
                                                                   
                               
 */
int am33xx_cm_wait_module_idle(u16 inst, s16 cdoffs, u16 clkctrl_offs)
{
	int i = 0;

	if (!clkctrl_offs)
		return 0;

	omap_test_timeout((_clkctrl_idlest(inst, cdoffs, clkctrl_offs) ==
				CLKCTRL_IDLEST_DISABLED),
				MAX_MODULE_READY_TIME, i);

	return (i < MAX_MODULE_READY_TIME) ? 0 : -EBUSY;
}

/* 
                                                                 
                                
                                                    
                                                        
                                                                        
  
                   
 */
void am33xx_cm_module_enable(u8 mode, u16 inst, s16 cdoffs, u16 clkctrl_offs)
{
	u32 v;

	v = am33xx_cm_read_reg(inst, clkctrl_offs);
	v &= ~AM33XX_MODULEMODE_MASK;
	v |= mode << AM33XX_MODULEMODE_SHIFT;
	am33xx_cm_write_reg(v, inst, clkctrl_offs);
}

/* 
                                                               
                                                    
                                                        
                                                                        
  
                   
 */
void am33xx_cm_module_disable(u16 inst, s16 cdoffs, u16 clkctrl_offs)
{
	u32 v;

	v = am33xx_cm_read_reg(inst, clkctrl_offs);
	v &= ~AM33XX_MODULEMODE_MASK;
	am33xx_cm_write_reg(v, inst, clkctrl_offs);
}

/*
                                  
 */

static int am33xx_clkdm_sleep(struct clockdomain *clkdm)
{
	am33xx_cm_clkdm_force_sleep(clkdm->cm_inst, clkdm->clkdm_offs);
	return 0;
}

static int am33xx_clkdm_wakeup(struct clockdomain *clkdm)
{
	am33xx_cm_clkdm_force_wakeup(clkdm->cm_inst, clkdm->clkdm_offs);
	return 0;
}

static void am33xx_clkdm_allow_idle(struct clockdomain *clkdm)
{
	am33xx_cm_clkdm_enable_hwsup(clkdm->cm_inst, clkdm->clkdm_offs);
}

static void am33xx_clkdm_deny_idle(struct clockdomain *clkdm)
{
	am33xx_cm_clkdm_disable_hwsup(clkdm->cm_inst, clkdm->clkdm_offs);
}

static int am33xx_clkdm_clk_enable(struct clockdomain *clkdm)
{
	if (clkdm->flags & CLKDM_CAN_FORCE_WAKEUP)
		return am33xx_clkdm_wakeup(clkdm);

	return 0;
}

static int am33xx_clkdm_clk_disable(struct clockdomain *clkdm)
{
	bool hwsup = false;

	hwsup = am33xx_cm_is_clkdm_in_hwsup(clkdm->cm_inst, clkdm->clkdm_offs);

	if (!hwsup && (clkdm->flags & CLKDM_CAN_FORCE_SLEEP))
		am33xx_clkdm_sleep(clkdm);

	return 0;
}

struct clkdm_ops am33xx_clkdm_operations = {
	.clkdm_sleep		= am33xx_clkdm_sleep,
	.clkdm_wakeup		= am33xx_clkdm_wakeup,
	.clkdm_allow_idle	= am33xx_clkdm_allow_idle,
	.clkdm_deny_idle	= am33xx_clkdm_deny_idle,
	.clkdm_clk_enable	= am33xx_clkdm_clk_enable,
	.clkdm_clk_disable	= am33xx_clkdm_clk_disable,
};
