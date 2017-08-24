/*
  * Copyright (C) 2010 Brian King IBM Corporation
  *
  * This program is free software; you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation; either version 2 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
  */

#include <linux/cpu.h>
#include <linux/delay.h>
#include <linux/suspend.h>
#include <linux/stat.h>
#include <asm/firmware.h>
#include <asm/hvcall.h>
#include <asm/machdep.h>
#include <asm/mmu.h>
#include <asm/rtas.h>
#include <asm/topology.h>

static u64 stream_id;
static struct device suspend_dev;
static DECLARE_COMPLETION(suspend_work);
static struct rtas_suspend_me_data suspend_data;
static atomic_t suspending;

/* 
                                                     
  
                                                       
  
                
                                   
  */
static int pseries_suspend_begin(suspend_state_t state)
{
	long vasi_state, rc;
	unsigned long retbuf[PLPAR_HCALL_BUFSIZE];

	/*                              */
	rc = plpar_hcall(H_VASI_STATE, retbuf, stream_id);

	vasi_state = retbuf[0];

	if (rc) {
		pr_err("pseries_suspend_begin: vasi_state returned %ld\n",rc);
		return rc;
	} else if (vasi_state == H_VASI_ENABLED) {
		return -EAGAIN;
	} else if (vasi_state != H_VASI_SUSPENDING) {
		pr_err("pseries_suspend_begin: vasi_state returned state %ld\n",
		       vasi_state);
		return -EIO;
	}

	return 0;
}

/* 
                                             
  
                                           
  
  */
static int pseries_suspend_cpu(void)
{
	if (atomic_read(&suspending))
		return rtas_suspend_cpu(&suspend_data);
	return 0;
}

/* 
                                                     
  
                
                                   
  */
static int pseries_suspend_enter(suspend_state_t state)
{
	int rc = rtas_suspend_last_cpu(&suspend_data);

	atomic_set(&suspending, 0);
	atomic_set(&suspend_data.done, 1);
	return rc;
}

/* 
                                                           
  
                
                                   
  */
static int pseries_prepare_late(void)
{
	atomic_set(&suspending, 1);
	atomic_set(&suspend_data.working, 0);
	atomic_set(&suspend_data.done, 0);
	atomic_set(&suspend_data.error, 0);
	suspend_data.complete = &suspend_work;
	INIT_COMPLETION(suspend_work);
	return 0;
}

/* 
                                                   
                            
                                  
                
                       
  
                                                         
                                       
  
                
                                                        
  */
static ssize_t store_hibernate(struct device *dev,
			       struct device_attribute *attr,
			       const char *buf, size_t count)
{
	cpumask_var_t offline_mask;
	int rc;

	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;

	if (!alloc_cpumask_var(&offline_mask, GFP_TEMPORARY))
		return -ENOMEM;

	stream_id = simple_strtoul(buf, NULL, 16);

	do {
		rc = pseries_suspend_begin(PM_SUSPEND_MEM);
		if (rc == -EAGAIN)
			ssleep(1);
	} while (rc == -EAGAIN);

	if (!rc) {
		/*                                 */
		cpumask_andnot(offline_mask, cpu_present_mask,
				cpu_online_mask);
		rc = rtas_online_cpus_mask(offline_mask);
		if (rc) {
			pr_err("%s: Could not bring present CPUs online.\n",
					__func__);
			goto out;
		}

		stop_topology_update();
		rc = pm_suspend(PM_SUSPEND_MEM);
		start_topology_update();

		/*                                            */
		if (!rtas_offline_cpus_mask(offline_mask))
			pr_warn("%s: Could not restore CPUs to offline "
					"state.\n", __func__);
	}

	stream_id = 0;

	if (!rc)
		rc = count;
out:
	free_cpumask_var(offline_mask);
	return rc;
}

static DEVICE_ATTR(hibernate, S_IWUSR, NULL, store_hibernate);

static struct bus_type suspend_subsys = {
	.name = "power",
	.dev_name = "power",
};

static const struct platform_suspend_ops pseries_suspend_ops = {
	.valid		= suspend_valid_only_mem,
	.begin		= pseries_suspend_begin,
	.prepare_late	= pseries_prepare_late,
	.enter		= pseries_suspend_enter,
};

/* 
                                                       
  
                
                                   
  */
static int pseries_suspend_sysfs_register(struct device *dev)
{
	int rc;

	if ((rc = subsys_system_register(&suspend_subsys, NULL)))
		return rc;

	dev->id = 0;
	dev->bus = &suspend_subsys;

	if ((rc = device_create_file(suspend_subsys.dev_root, &dev_attr_hibernate)))
		goto subsys_unregister;

	return 0;

subsys_unregister:
	bus_unregister(&suspend_subsys);
	return rc;
}

/* 
                                                      
  
                
                                   
  */
static int __init pseries_suspend_init(void)
{
	int rc;

	if (!machine_is(pseries) || !firmware_has_feature(FW_FEATURE_LPAR))
		return 0;

	suspend_data.token = rtas_token("ibm,suspend-me");
	if (suspend_data.token == RTAS_UNKNOWN_SERVICE)
		return 0;

	if ((rc = pseries_suspend_sysfs_register(&suspend_dev)))
		return rc;

	ppc_md.suspend_disable_cpu = pseries_suspend_cpu;
	suspend_set_ops(&pseries_suspend_ops);
	return 0;
}

__initcall(pseries_suspend_init);
