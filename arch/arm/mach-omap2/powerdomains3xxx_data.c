/*
 * OMAP3 powerdomain definitions
 *
 * Copyright (C) 2007-2008, 2011 Texas Instruments, Inc.
 * Copyright (C) 2007-2011 Nokia Corporation
 *
 * Paul Walmsley, Jouni Högander
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/bug.h>

#include "soc.h"
#include "powerdomain.h"
#include "powerdomains2xxx_3xxx_data.h"
#include "prcm-common.h"
#include "prm2xxx_3xxx.h"
#include "prm-regbits-34xx.h"
#include "cm2xxx_3xxx.h"
#include "cm-regbits-34xx.h"

/*
                                           
 */

/*
               
 */

static struct powerdomain iva2_pwrdm = {
	.name		  = "iva2_pwrdm",
	.prcm_offs	  = OMAP3430_IVA2_MOD,
	.pwrsts		  = PWRSTS_OFF_RET_ON,
	.pwrsts_logic_ret = PWRSTS_OFF_RET,
	.banks		  = 4,
	.pwrsts_mem_ret	  = {
		[0] = PWRSTS_OFF_RET,
		[1] = PWRSTS_OFF_RET,
		[2] = PWRSTS_OFF_RET,
		[3] = PWRSTS_OFF_RET,
	},
	.pwrsts_mem_on	  = {
		[0] = PWRSTS_ON,
		[1] = PWRSTS_ON,
		[2] = PWRSTS_OFF_ON,
		[3] = PWRSTS_ON,
	},
	.voltdm		  = { .name = "mpu_iva" },
};

static struct powerdomain mpu_3xxx_pwrdm = {
	.name		  = "mpu_pwrdm",
	.prcm_offs	  = MPU_MOD,
	.pwrsts		  = PWRSTS_OFF_RET_ON,
	.pwrsts_logic_ret = PWRSTS_OFF_RET,
	.flags		  = PWRDM_HAS_MPU_QUIRK,
	.banks		  = 1,
	.pwrsts_mem_ret	  = {
		[0] = PWRSTS_OFF_RET,
	},
	.pwrsts_mem_on	  = {
		[0] = PWRSTS_OFF_ON,
	},
	.voltdm		  = { .name = "mpu_iva" },
};

static struct powerdomain mpu_am35x_pwrdm = {
	.name		  = "mpu_pwrdm",
	.prcm_offs	  = MPU_MOD,
	.pwrsts		  = PWRSTS_ON,
	.pwrsts_logic_ret = PWRSTS_ON,
	.flags		  = PWRDM_HAS_MPU_QUIRK,
	.banks		  = 1,
	.pwrsts_mem_ret	  = {
		[0] = PWRSTS_ON,
	},
	.pwrsts_mem_on	  = {
		[0] = PWRSTS_ON,
	},
	.voltdm		  = { .name = "mpu_iva" },
};

/*
                                                     
                                                      
                                       
                                                     
  
                                                           
                                                             
                             
 */
static struct powerdomain core_3xxx_pre_es3_1_pwrdm = {
	.name		  = "core_pwrdm",
	.prcm_offs	  = CORE_MOD,
	.pwrsts		  = PWRSTS_OFF_RET_ON,
	.pwrsts_logic_ret = PWRSTS_OFF_RET,
	.banks		  = 2,
	.pwrsts_mem_ret	  = {
		[0] = PWRSTS_OFF_RET,	 /*              */
		[1] = PWRSTS_OFF_RET,	 /*              */
	},
	.pwrsts_mem_on	  = {
		[0] = PWRSTS_OFF_RET_ON, /*             */
		[1] = PWRSTS_OFF_RET_ON, /*             */
	},
	.voltdm		  = { .name = "core" },
};

static struct powerdomain core_3xxx_es3_1_pwrdm = {
	.name		  = "core_pwrdm",
	.prcm_offs	  = CORE_MOD,
	.pwrsts		  = PWRSTS_OFF_RET_ON,
	.pwrsts_logic_ret = PWRSTS_OFF_RET,
	/*
                                                         
                     
  */
	.flags		  = PWRDM_HAS_HDWR_SAR, /*                 */
	.banks		  = 2,
	.pwrsts_mem_ret	  = {
		[0] = PWRSTS_OFF_RET,	 /*              */
		[1] = PWRSTS_OFF_RET,	 /*              */
	},
	.pwrsts_mem_on	  = {
		[0] = PWRSTS_OFF_RET_ON, /*             */
		[1] = PWRSTS_OFF_RET_ON, /*             */
	},
	.voltdm		  = { .name = "core" },
};

static struct powerdomain core_am35x_pwrdm = {
	.name		  = "core_pwrdm",
	.prcm_offs	  = CORE_MOD,
	.pwrsts		  = PWRSTS_ON,
	.pwrsts_logic_ret = PWRSTS_ON,
	.banks		  = 2,
	.pwrsts_mem_ret	  = {
		[0] = PWRSTS_ON,	 /*              */
		[1] = PWRSTS_ON,	 /*              */
	},
	.pwrsts_mem_on	  = {
		[0] = PWRSTS_ON, /*             */
		[1] = PWRSTS_ON, /*             */
	},
	.voltdm		  = { .name = "core" },
};

static struct powerdomain dss_pwrdm = {
	.name		  = "dss_pwrdm",
	.prcm_offs	  = OMAP3430_DSS_MOD,
	.pwrsts		  = PWRSTS_OFF_RET_ON,
	.pwrsts_logic_ret = PWRSTS_RET,
	.banks		  = 1,
	.pwrsts_mem_ret	  = {
		[0] = PWRSTS_RET, /*             */
	},
	.pwrsts_mem_on	  = {
		[0] = PWRSTS_ON,  /*            */
	},
	.voltdm		  = { .name = "core" },
};

static struct powerdomain dss_am35x_pwrdm = {
	.name		  = "dss_pwrdm",
	.prcm_offs	  = OMAP3430_DSS_MOD,
	.pwrsts		  = PWRSTS_ON,
	.pwrsts_logic_ret = PWRSTS_ON,
	.banks		  = 1,
	.pwrsts_mem_ret	  = {
		[0] = PWRSTS_ON, /*             */
	},
	.pwrsts_mem_on	  = {
		[0] = PWRSTS_ON,  /*            */
	},
	.voltdm		  = { .name = "core" },
};

/*
                                                                    
                                                                  
             
 */
static struct powerdomain sgx_pwrdm = {
	.name		  = "sgx_pwrdm",
	.prcm_offs	  = OMAP3430ES2_SGX_MOD,
	/*                                                        */
	.pwrsts		  = PWRSTS_OFF_ON,
	.pwrsts_logic_ret = PWRSTS_RET,
	.banks		  = 1,
	.pwrsts_mem_ret	  = {
		[0] = PWRSTS_RET, /*             */
	},
	.pwrsts_mem_on	  = {
		[0] = PWRSTS_ON,  /*            */
	},
	.voltdm		  = { .name = "core" },
};

static struct powerdomain sgx_am35x_pwrdm = {
	.name		  = "sgx_pwrdm",
	.prcm_offs	  = OMAP3430ES2_SGX_MOD,
	.pwrsts		  = PWRSTS_ON,
	.pwrsts_logic_ret = PWRSTS_ON,
	.banks		  = 1,
	.pwrsts_mem_ret	  = {
		[0] = PWRSTS_ON, /*             */
	},
	.pwrsts_mem_on	  = {
		[0] = PWRSTS_ON,  /*            */
	},
	.voltdm		  = { .name = "core" },
};

static struct powerdomain cam_pwrdm = {
	.name		  = "cam_pwrdm",
	.prcm_offs	  = OMAP3430_CAM_MOD,
	.pwrsts		  = PWRSTS_OFF_RET_ON,
	.pwrsts_logic_ret = PWRSTS_RET,
	.banks		  = 1,
	.pwrsts_mem_ret	  = {
		[0] = PWRSTS_RET, /*             */
	},
	.pwrsts_mem_on	  = {
		[0] = PWRSTS_ON,  /*            */
	},
	.voltdm		  = { .name = "core" },
};

static struct powerdomain per_pwrdm = {
	.name		  = "per_pwrdm",
	.prcm_offs	  = OMAP3430_PER_MOD,
	.pwrsts		  = PWRSTS_OFF_RET_ON,
	.pwrsts_logic_ret = PWRSTS_OFF_RET,
	.banks		  = 1,
	.pwrsts_mem_ret	  = {
		[0] = PWRSTS_RET, /*             */
	},
	.pwrsts_mem_on	  = {
		[0] = PWRSTS_ON,  /*            */
	},
	.voltdm		  = { .name = "core" },
};

static struct powerdomain per_am35x_pwrdm = {
	.name		  = "per_pwrdm",
	.prcm_offs	  = OMAP3430_PER_MOD,
	.pwrsts		  = PWRSTS_ON,
	.pwrsts_logic_ret = PWRSTS_ON,
	.banks		  = 1,
	.pwrsts_mem_ret	  = {
		[0] = PWRSTS_ON, /*             */
	},
	.pwrsts_mem_on	  = {
		[0] = PWRSTS_ON,  /*            */
	},
	.voltdm		  = { .name = "core" },
};

static struct powerdomain emu_pwrdm = {
	.name		= "emu_pwrdm",
	.prcm_offs	= OMAP3430_EMU_MOD,
	.voltdm		  = { .name = "core" },
};

static struct powerdomain neon_pwrdm = {
	.name		  = "neon_pwrdm",
	.prcm_offs	  = OMAP3430_NEON_MOD,
	.pwrsts		  = PWRSTS_OFF_RET_ON,
	.pwrsts_logic_ret = PWRSTS_RET,
	.voltdm		  = { .name = "mpu_iva" },
};

static struct powerdomain neon_am35x_pwrdm = {
	.name		  = "neon_pwrdm",
	.prcm_offs	  = OMAP3430_NEON_MOD,
	.pwrsts		  = PWRSTS_ON,
	.pwrsts_logic_ret = PWRSTS_ON,
	.voltdm		  = { .name = "mpu_iva" },
};

static struct powerdomain usbhost_pwrdm = {
	.name		  = "usbhost_pwrdm",
	.prcm_offs	  = OMAP3430ES2_USBHOST_MOD,
	.pwrsts		  = PWRSTS_OFF_RET_ON,
	.pwrsts_logic_ret = PWRSTS_RET,
	/*
                                                                  
                                                              
                                                                     
                      
  */
	/*                              */ /*                        */
	.banks		  = 1,
	.pwrsts_mem_ret	  = {
		[0] = PWRSTS_RET, /*             */
	},
	.pwrsts_mem_on	  = {
		[0] = PWRSTS_ON,  /*            */
	},
	.voltdm		  = { .name = "core" },
};

static struct powerdomain dpll1_pwrdm = {
	.name		= "dpll1_pwrdm",
	.prcm_offs	= MPU_MOD,
	.voltdm		  = { .name = "mpu_iva" },
};

static struct powerdomain dpll2_pwrdm = {
	.name		= "dpll2_pwrdm",
	.prcm_offs	= OMAP3430_IVA2_MOD,
	.voltdm		  = { .name = "mpu_iva" },
};

static struct powerdomain dpll3_pwrdm = {
	.name		= "dpll3_pwrdm",
	.prcm_offs	= PLL_MOD,
	.voltdm		  = { .name = "core" },
};

static struct powerdomain dpll4_pwrdm = {
	.name		= "dpll4_pwrdm",
	.prcm_offs	= PLL_MOD,
	.voltdm		  = { .name = "core" },
};

static struct powerdomain dpll5_pwrdm = {
	.name		= "dpll5_pwrdm",
	.prcm_offs	= PLL_MOD,
	.voltdm		  = { .name = "core" },
};

/*                                                                            */
static struct powerdomain *powerdomains_omap3430_common[] __initdata = {
	&wkup_omap2_pwrdm,
	&iva2_pwrdm,
	&mpu_3xxx_pwrdm,
	&neon_pwrdm,
	&cam_pwrdm,
	&dss_pwrdm,
	&per_pwrdm,
	&emu_pwrdm,
	&dpll1_pwrdm,
	&dpll2_pwrdm,
	&dpll3_pwrdm,
	&dpll4_pwrdm,
	NULL
};

static struct powerdomain *powerdomains_omap3430es1[] __initdata = {
	&gfx_omap2_pwrdm,
	&core_3xxx_pre_es3_1_pwrdm,
	NULL
};

/*                         */
static struct powerdomain *powerdomains_omap3430es2_es3_0[] __initdata = {
	&core_3xxx_pre_es3_1_pwrdm,
	&sgx_pwrdm,
	&usbhost_pwrdm,
	&dpll5_pwrdm,
	NULL
};

/*                          */
static struct powerdomain *powerdomains_omap3430es3_1plus[] __initdata = {
	&core_3xxx_es3_1_pwrdm,
	&sgx_pwrdm,
	&usbhost_pwrdm,
	&dpll5_pwrdm,
	NULL
};

static struct powerdomain *powerdomains_am35x[] __initdata = {
	&wkup_omap2_pwrdm,
	&mpu_am35x_pwrdm,
	&neon_am35x_pwrdm,
	&core_am35x_pwrdm,
	&sgx_am35x_pwrdm,
	&dss_am35x_pwrdm,
	&per_am35x_pwrdm,
	&emu_pwrdm,
	&dpll1_pwrdm,
	&dpll3_pwrdm,
	&dpll4_pwrdm,
	&dpll5_pwrdm,
	NULL
};

void __init omap3xxx_powerdomains_init(void)
{
	unsigned int rev;

	if (!cpu_is_omap34xx())
		return;

	pwrdm_register_platform_funcs(&omap3_pwrdm_operations);

	rev = omap_rev();

	if (rev == AM35XX_REV_ES1_0 || rev == AM35XX_REV_ES1_1) {
		pwrdm_register_pwrdms(powerdomains_am35x);
	} else {
		pwrdm_register_pwrdms(powerdomains_omap3430_common);

		switch (rev) {
		case OMAP3430_REV_ES1_0:
			pwrdm_register_pwrdms(powerdomains_omap3430es1);
			break;
		case OMAP3430_REV_ES2_0:
		case OMAP3430_REV_ES2_1:
		case OMAP3430_REV_ES3_0:
		case OMAP3630_REV_ES1_0:
			pwrdm_register_pwrdms(powerdomains_omap3430es2_es3_0);
			break;
		case OMAP3430_REV_ES3_1:
		case OMAP3430_REV_ES3_1_2:
		case OMAP3630_REV_ES1_1:
		case OMAP3630_REV_ES1_2:
			pwrdm_register_pwrdms(powerdomains_omap3430es3_1plus);
			break;
		default:
			WARN(1, "OMAP3 powerdomain init: unknown chip type\n");
		}
	}

	pwrdm_complete_init();
}
