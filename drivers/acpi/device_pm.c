/*
 * drivers/acpi/device_pm.c - ACPI device power management routines.
 *
 * Copyright (C) 2012, Intel Corp.
 * Author: Rafael J. Wysocki <rafael.j.wysocki@intel.com>
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

#include <linux/device.h>
#include <linux/export.h>
#include <linux/mutex.h>
#include <linux/pm_qos.h>
#include <linux/pm_runtime.h>

#include <acpi/acpi.h>
#include <acpi/acpi_bus.h>
#include <acpi/acpi_drivers.h>

#include "internal.h"

#define _COMPONENT	ACPI_POWER_COMPONENT
ACPI_MODULE_NAME("device_pm");

/* 
                                                                              
                                                                          
 */
const char *acpi_power_state_string(int state)
{
	switch (state) {
	case ACPI_STATE_D0:
		return "D0";
	case ACPI_STATE_D1:
		return "D1";
	case ACPI_STATE_D2:
		return "D2";
	case ACPI_STATE_D3_HOT:
		return "D3hot";
	case ACPI_STATE_D3_COLD:
		return "D3cold";
	default:
		return "(unknown)";
	}
}

/* 
                                                             
                                             
                                                        
  
                                                                           
                                                                          
                                                            
 */
int acpi_device_get_power(struct acpi_device *device, int *state)
{
	int result = ACPI_STATE_UNKNOWN;

	if (!device || !state)
		return -EINVAL;

	if (!device->flags.power_manageable) {
		/*                                                        */
		*state = device->parent ?
			device->parent->power.state : ACPI_STATE_D0;
		goto out;
	}

	/*
                                                                        
                 
  */
	if (device->power.flags.power_resources) {
		int error = acpi_power_get_inferred_state(device, &result);
		if (error)
			return error;
	}
	if (device->power.flags.explicit_get) {
		acpi_handle handle = device->handle;
		unsigned long long psc;
		acpi_status status;

		status = acpi_evaluate_integer(handle, "_PSC", NULL, &psc);
		if (ACPI_FAILURE(status))
			return -ENODEV;

		/*
                                                            
                                                         
    
                                                           
                                                              
                                                               
                        
   */
		if (psc > result && psc < ACPI_STATE_D3_COLD)
			result = psc;
		else if (result == ACPI_STATE_UNKNOWN)
			result = psc > ACPI_STATE_D2 ? ACPI_STATE_D3_COLD : psc;
	}

	/*
                                                                      
                                                                        
                    
  */
	if (device->parent && device->parent->power.state == ACPI_STATE_UNKNOWN
	    && result == ACPI_STATE_D0)
		device->parent->power.state = ACPI_STATE_D0;

	*state = result;

 out:
	ACPI_DEBUG_PRINT((ACPI_DB_INFO, "Device [%s] power state is %s\n",
			  device->pnp.bus_id, acpi_power_state_string(*state)));

	return 0;
}

static int acpi_dev_pm_explicit_set(struct acpi_device *adev, int state)
{
	if (adev->power.states[state].flags.explicit_set) {
		char method[5] = { '_', 'P', 'S', '0' + state, '\0' };
		acpi_status status;

		status = acpi_evaluate_object(adev->handle, method, NULL, NULL);
		if (ACPI_FAILURE(status))
			return -ENODEV;
	}
	return 0;
}

/* 
                                                             
                                             
                                  
  
                                                                            
            
 */
int acpi_device_set_power(struct acpi_device *device, int state)
{
	int result = 0;
	bool cut_power = false;

	if (!device || (state < ACPI_STATE_D0) || (state > ACPI_STATE_D3_COLD))
		return -EINVAL;

	/*                                        */

	if (state == device->power.state) {
		ACPI_DEBUG_PRINT((ACPI_DB_INFO, "Device is already at %s\n",
				  acpi_power_state_string(state)));
		return 0;
	}

	if (!device->power.states[state].flags.valid) {
		printk(KERN_WARNING PREFIX "Device does not support %s\n",
		       acpi_power_state_string(state));
		return -ENODEV;
	}
	if (device->parent && (state < device->parent->power.state)) {
		printk(KERN_WARNING PREFIX
			      "Cannot set device to a higher-powered"
			      " state than parent\n");
		return -ENODEV;
	}

	/*                                                   */
	if (state == ACPI_STATE_D3_COLD
	    && device->power.states[ACPI_STATE_D3_COLD].flags.os_accessible) {
		state = ACPI_STATE_D3_HOT;
		cut_power = true;
	}

	if (state < device->power.state && state != ACPI_STATE_D0
	    && device->power.state >= ACPI_STATE_D3_HOT) {
		printk(KERN_WARNING PREFIX
			"Cannot transition to non-D0 state from D3\n");
		return -ENODEV;
	}

	/*
                    
                    
                                                                    
                                           
  */
	if (device->power.flags.power_resources) {
		result = acpi_power_transition(device, state);
		if (result)
			goto end;
	}
	result = acpi_dev_pm_explicit_set(device, state);
	if (result)
		goto end;

	if (cut_power) {
		device->power.state = state;
		state = ACPI_STATE_D3_COLD;
		result = acpi_power_transition(device, state);
	}

 end:
	if (result) {
		printk(KERN_WARNING PREFIX
			      "Device [%s] failed to transition to %s\n",
			      device->pnp.bus_id,
			      acpi_power_state_string(state));
	} else {
		device->power.state = state;
		ACPI_DEBUG_PRINT((ACPI_DB_INFO,
				  "Device [%s] transitioned to %s\n",
				  device->pnp.bus_id,
				  acpi_power_state_string(state)));
	}

	return result;
}
EXPORT_SYMBOL(acpi_device_set_power);

int acpi_bus_set_power(acpi_handle handle, int state)
{
	struct acpi_device *device;
	int result;

	result = acpi_bus_get_device(handle, &device);
	if (result)
		return result;

	if (!device->flags.power_manageable) {
		ACPI_DEBUG_PRINT((ACPI_DB_INFO,
				"Device [%s] is not power manageable\n",
				dev_name(&device->dev)));
		return -ENODEV;
	}

	return acpi_device_set_power(device, state);
}
EXPORT_SYMBOL(acpi_bus_set_power);

int acpi_bus_init_power(struct acpi_device *device)
{
	int state;
	int result;

	if (!device)
		return -EINVAL;

	device->power.state = ACPI_STATE_UNKNOWN;

	result = acpi_device_get_power(device, &state);
	if (result)
		return result;

	if (state < ACPI_STATE_D3_COLD && device->power.flags.power_resources) {
		result = acpi_power_on_resources(device, state);
		if (result)
			return result;

		result = acpi_dev_pm_explicit_set(device, state);
		if (result)
			return result;
	} else if (state == ACPI_STATE_UNKNOWN) {
		/*
                                                                 
                                                                  
                                                                 
                                      
   */
		state = ACPI_STATE_D0;
	}
	device->power.state = state;
	return 0;
}

/* 
                                                                     
                                                              
  
                                                                              
                                                                              
                                                         
 */
int acpi_device_fix_up_power(struct acpi_device *device)
{
	int ret = 0;

	if (!device->power.flags.power_resources
	    && !device->power.flags.explicit_get
	    && device->power.state == ACPI_STATE_D0)
		ret = acpi_dev_pm_explicit_set(device, ACPI_STATE_D0);

	return ret;
}

int acpi_bus_update_power(acpi_handle handle, int *state_p)
{
	struct acpi_device *device;
	int state;
	int result;

	result = acpi_bus_get_device(handle, &device);
	if (result)
		return result;

	result = acpi_device_get_power(device, &state);
	if (result)
		return result;

	if (state == ACPI_STATE_UNKNOWN) {
		state = ACPI_STATE_D0;
		result = acpi_device_set_power(device, state);
		if (result)
			return result;
	} else {
		if (device->power.flags.power_resources) {
			/*
                                                          
                                                        
    */
			result = acpi_power_transition(device, state);
			if (result)
				return result;
		}
		device->power.state = state;
	}
	if (state_p)
		*state_p = state;

	return 0;
}
EXPORT_SYMBOL_GPL(acpi_bus_update_power);

bool acpi_bus_power_manageable(acpi_handle handle)
{
	struct acpi_device *device;
	int result;

	result = acpi_bus_get_device(handle, &device);
	return result ? false : device->flags.power_manageable;
}
EXPORT_SYMBOL(acpi_bus_power_manageable);

#ifdef CONFIG_PM
static DEFINE_MUTEX(acpi_pm_notifier_lock);

/* 
                                                                     
                                              
                                                                 
  
                                                                              
                                                                             
                                                                          
                                                              
 */
acpi_status acpi_add_pm_notifier(struct acpi_device *adev,
				 acpi_notify_handler handler, void *context)
{
	acpi_status status = AE_ALREADY_EXISTS;

	mutex_lock(&acpi_pm_notifier_lock);

	if (adev->wakeup.flags.notifier_present)
		goto out;

	status = acpi_install_notify_handler(adev->handle,
					     ACPI_SYSTEM_NOTIFY,
					     handler, context);
	if (ACPI_FAILURE(status))
		goto out;

	adev->wakeup.flags.notifier_present = true;

 out:
	mutex_unlock(&acpi_pm_notifier_lock);
	return status;
}

/* 
                                                                           
                                                  
 */
acpi_status acpi_remove_pm_notifier(struct acpi_device *adev,
				    acpi_notify_handler handler)
{
	acpi_status status = AE_BAD_PARAMETER;

	mutex_lock(&acpi_pm_notifier_lock);

	if (!adev->wakeup.flags.notifier_present)
		goto out;

	status = acpi_remove_notify_handler(adev->handle,
					    ACPI_SYSTEM_NOTIFY,
					    handler);
	if (ACPI_FAILURE(status))
		goto out;

	adev->wakeup.flags.notifier_present = false;

 out:
	mutex_unlock(&acpi_pm_notifier_lock);
	return status;
}

bool acpi_bus_can_wakeup(acpi_handle handle)
{
	struct acpi_device *device;
	int result;

	result = acpi_bus_get_device(handle, &device);
	return result ? false : device->wakeup.flags.valid;
}
EXPORT_SYMBOL(acpi_bus_can_wakeup);

/* 
                                                                      
                                                             
                                                 
                                                                   
                                                                 
                                                                           
                                                                        
                                                                      
  
                                                                          
                                                                   
                                                                               
                                                                             
                                    
  
                                                                            
                                                           
 */
int acpi_device_power_state(struct device *dev, struct acpi_device *adev,
			    u32 target_state, int d_max_in, int *d_min_p)
{
	char acpi_method[] = "_SxD";
	unsigned long long d_min, d_max;
	bool wakeup = false;

	if (d_max_in < ACPI_STATE_D0 || d_max_in > ACPI_STATE_D3)
		return -EINVAL;

	if (d_max_in > ACPI_STATE_D3_HOT) {
		enum pm_qos_flags_status stat;

		stat = dev_pm_qos_flags(dev, PM_QOS_FLAG_NO_POWER_OFF);
		if (stat == PM_QOS_FLAGS_ALL)
			d_max_in = ACPI_STATE_D3_HOT;
	}

	acpi_method[2] = '0' + target_state;
	/*
                                                               
                                                               
                                                              
                                            
  */
	d_min = ACPI_STATE_D0;
	d_max = ACPI_STATE_D3;

	/*
                                                                      
                                                                     
                                     
   
                                                                       
                                                            
  */
	if (target_state > ACPI_STATE_S0) {
		acpi_evaluate_integer(adev->handle, acpi_method, NULL, &d_min);
		wakeup = device_may_wakeup(dev) && adev->wakeup.flags.valid
			&& adev->wakeup.sleep_state >= target_state;
	} else if (dev_pm_qos_flags(dev, PM_QOS_FLAG_REMOTE_WAKEUP) !=
			PM_QOS_FLAGS_NONE) {
		wakeup = adev->wakeup.flags.valid;
	}

	/*
                                                                       
                                                                    
                                                               
                                                                        
                                                 
  */
	if (wakeup) {
		acpi_status status;

		acpi_method[3] = 'W';
		status = acpi_evaluate_integer(adev->handle, acpi_method, NULL,
						&d_max);
		if (ACPI_FAILURE(status)) {
			if (target_state != ACPI_STATE_S0 ||
			    status != AE_NOT_FOUND)
				d_max = d_min;
		} else if (d_max < d_min) {
			/*                                  */
			printk(KERN_WARNING "ACPI: Wrong value from %s\n",
				acpi_method);
			/*             */
			d_min = d_max;
		}
	}

	if (d_max_in < d_min)
		return -EINVAL;
	if (d_min_p)
		*d_min_p = d_min;
	/*                                                          */
	if (d_max > d_max_in) {
		for (d_max = d_max_in; d_max > d_min; d_max--) {
			if (adev->power.states[d_max].flags.valid)
				break;
		}
	}
	return d_max;
}
EXPORT_SYMBOL_GPL(acpi_device_power_state);

/* 
                                                                         
                                                             
                                                                           
                                                                 
                                                                        
                                                                      
  
                                                                        
 */
int acpi_pm_device_sleep_state(struct device *dev, int *d_min_p, int d_max_in)
{
	acpi_handle handle = DEVICE_ACPI_HANDLE(dev);
	struct acpi_device *adev;

	if (!handle || acpi_bus_get_device(handle, &adev)) {
		dev_dbg(dev, "ACPI handle without context in %s!\n", __func__);
		return -ENODEV;
	}

	return acpi_device_power_state(dev, adev, acpi_target_system_state(),
				       d_max_in, d_min_p);
}
EXPORT_SYMBOL(acpi_pm_device_sleep_state);

#ifdef CONFIG_PM_RUNTIME
/* 
                                                                     
                                                              
                                      
                                             
 */
static void acpi_wakeup_device(acpi_handle handle, u32 event, void *context)
{
	struct device *dev = context;

	if (event == ACPI_NOTIFY_DEVICE_WAKE && dev) {
		pm_wakeup_event(dev, 0);
		pm_runtime_resume(dev);
	}
}

/* 
                                                                            
                                                              
                                                                  
  
                                                                       
                                                                            
                                      
  
                                                                              
                 
 */
int __acpi_device_run_wake(struct acpi_device *adev, bool enable)
{
	struct acpi_device_wakeup *wakeup = &adev->wakeup;

	if (enable) {
		acpi_status res;
		int error;

		error = acpi_enable_wakeup_device_power(adev, ACPI_STATE_S0);
		if (error)
			return error;

		res = acpi_enable_gpe(wakeup->gpe_device, wakeup->gpe_number);
		if (ACPI_FAILURE(res)) {
			acpi_disable_wakeup_device_power(adev);
			return -EIO;
		}
	} else {
		acpi_disable_gpe(wakeup->gpe_device, wakeup->gpe_number);
		acpi_disable_wakeup_device_power(adev);
	}
	return 0;
}

/* 
                                                                           
                                                          
                                                                  
 */
int acpi_pm_device_run_wake(struct device *phys_dev, bool enable)
{
	struct acpi_device *adev;
	acpi_handle handle;

	if (!device_run_wake(phys_dev))
		return -EINVAL;

	handle = DEVICE_ACPI_HANDLE(phys_dev);
	if (!handle || acpi_bus_get_device(handle, &adev)) {
		dev_dbg(phys_dev, "ACPI handle without context in %s!\n",
			__func__);
		return -ENODEV;
	}

	return __acpi_device_run_wake(adev, enable);
}
EXPORT_SYMBOL(acpi_pm_device_run_wake);
#else
static inline void acpi_wakeup_device(acpi_handle handle, u32 event,
				      void *context) {}
#endif /*                   */

#ifdef CONFIG_PM_SLEEP
/* 
                                                                             
                                                        
                                                                      
                                                                    
 */
int __acpi_device_sleep_wake(struct acpi_device *adev, u32 target_state,
			     bool enable)
{
	return enable ?
		acpi_enable_wakeup_device_power(adev, target_state) :
		acpi_disable_wakeup_device_power(adev);
}

/* 
                                                                              
                                                                          
                                                                    
 */
int acpi_pm_device_sleep_wake(struct device *dev, bool enable)
{
	acpi_handle handle;
	struct acpi_device *adev;
	int error;

	if (!device_can_wakeup(dev))
		return -EINVAL;

	handle = DEVICE_ACPI_HANDLE(dev);
	if (!handle || acpi_bus_get_device(handle, &adev)) {
		dev_dbg(dev, "ACPI handle without context in %s!\n", __func__);
		return -ENODEV;
	}

	error = __acpi_device_sleep_wake(adev, acpi_target_system_state(),
					 enable);
	if (!error)
		dev_info(dev, "System wakeup %s by ACPI\n",
				enable ? "enabled" : "disabled");

	return error;
}
#endif /*                 */

/* 
                                                                             
                                         
 */
struct acpi_device *acpi_dev_pm_get_node(struct device *dev)
{
	acpi_handle handle = DEVICE_ACPI_HANDLE(dev);
	struct acpi_device *adev;

	return handle && !acpi_bus_get_device(handle, &adev) ? adev : NULL;
}

/* 
                                                                  
                                              
                                                 
                                                              
 */
static int acpi_dev_pm_low_power(struct device *dev, struct acpi_device *adev,
				 u32 system_state)
{
	int power_state;

	if (!acpi_device_power_manageable(adev))
		return 0;

	power_state = acpi_device_power_state(dev, adev, system_state,
					      ACPI_STATE_D3, NULL);
	if (power_state < ACPI_STATE_D0 || power_state > ACPI_STATE_D3)
		return -EIO;

	return acpi_device_set_power(adev, power_state);
}

/* 
                                                                      
                                                            
 */
static int acpi_dev_pm_full_power(struct acpi_device *adev)
{
	return acpi_device_power_manageable(adev) ?
		acpi_device_set_power(adev, ACPI_STATE_D0) : 0;
}

#ifdef CONFIG_PM_RUNTIME
/* 
                                                                           
                                              
  
                                                                              
                                                                           
                                                                              
                                 
 */
int acpi_dev_runtime_suspend(struct device *dev)
{
	struct acpi_device *adev = acpi_dev_pm_get_node(dev);
	bool remote_wakeup;
	int error;

	if (!adev)
		return 0;

	remote_wakeup = dev_pm_qos_flags(dev, PM_QOS_FLAG_REMOTE_WAKEUP) >
				PM_QOS_FLAGS_NONE;
	error = __acpi_device_run_wake(adev, remote_wakeup);
	if (remote_wakeup && error)
		return -EAGAIN;

	error = acpi_dev_pm_low_power(dev, adev, ACPI_STATE_S0);
	if (error)
		__acpi_device_run_wake(adev, false);

	return error;
}
EXPORT_SYMBOL_GPL(acpi_dev_runtime_suspend);

/* 
                                                                             
                                                 
  
                                                                         
                                                                           
                         
 */
int acpi_dev_runtime_resume(struct device *dev)
{
	struct acpi_device *adev = acpi_dev_pm_get_node(dev);
	int error;

	if (!adev)
		return 0;

	error = acpi_dev_pm_full_power(adev);
	__acpi_device_run_wake(adev, false);
	return error;
}
EXPORT_SYMBOL_GPL(acpi_dev_runtime_resume);

/* 
                                                           
                           
  
                                                                               
                                     
 */
int acpi_subsys_runtime_suspend(struct device *dev)
{
	int ret = pm_generic_runtime_suspend(dev);
	return ret ? ret : acpi_dev_runtime_suspend(dev);
}
EXPORT_SYMBOL_GPL(acpi_subsys_runtime_suspend);

/* 
                                                         
                          
  
                                                                               
                                           
 */
int acpi_subsys_runtime_resume(struct device *dev)
{
	int ret = acpi_dev_runtime_resume(dev);
	return ret ? ret : pm_generic_runtime_resume(dev);
}
EXPORT_SYMBOL_GPL(acpi_subsys_runtime_resume);
#endif /*                   */

#ifdef CONFIG_PM_SLEEP
/* 
                                                                        
                                              
  
                                                                            
                                                                          
                                                                          
                                                                          
 */
int acpi_dev_suspend_late(struct device *dev)
{
	struct acpi_device *adev = acpi_dev_pm_get_node(dev);
	u32 target_state;
	bool wakeup;
	int error;

	if (!adev)
		return 0;

	target_state = acpi_target_system_state();
	wakeup = device_may_wakeup(dev);
	error = __acpi_device_sleep_wake(adev, target_state, wakeup);
	if (wakeup && error)
		return error;

	error = acpi_dev_pm_low_power(dev, adev, target_state);
	if (error)
		__acpi_device_sleep_wake(adev, ACPI_STATE_UNKNOWN, false);

	return error;
}
EXPORT_SYMBOL_GPL(acpi_dev_suspend_late);

/* 
                                                                           
                                                 
  
                                                                         
                                                                          
                                                            
 */
int acpi_dev_resume_early(struct device *dev)
{
	struct acpi_device *adev = acpi_dev_pm_get_node(dev);
	int error;

	if (!adev)
		return 0;

	error = acpi_dev_pm_full_power(adev);
	__acpi_device_sleep_wake(adev, ACPI_STATE_UNKNOWN, false);
	return error;
}
EXPORT_SYMBOL_GPL(acpi_dev_resume_early);

/* 
                                                                               
                           
 */
int acpi_subsys_prepare(struct device *dev)
{
	/*
                                                                      
                                   
  */
	pm_runtime_resume(dev);
	return pm_generic_prepare(dev);
}
EXPORT_SYMBOL_GPL(acpi_subsys_prepare);

/* 
                                                        
                           
  
                                                                            
                                                                         
 */
int acpi_subsys_suspend_late(struct device *dev)
{
	int ret = pm_generic_suspend_late(dev);
	return ret ? ret : acpi_dev_suspend_late(dev);
}
EXPORT_SYMBOL_GPL(acpi_subsys_suspend_late);

/* 
                                                       
                          
  
                                                                               
                                                                          
                 
 */
int acpi_subsys_resume_early(struct device *dev)
{
	int ret = acpi_dev_resume_early(dev);
	return ret ? ret : pm_generic_resume_early(dev);
}
EXPORT_SYMBOL_GPL(acpi_subsys_resume_early);
#endif /*                 */

static struct dev_pm_domain acpi_general_pm_domain = {
	.ops = {
#ifdef CONFIG_PM_RUNTIME
		.runtime_suspend = acpi_subsys_runtime_suspend,
		.runtime_resume = acpi_subsys_runtime_resume,
		.runtime_idle = pm_generic_runtime_idle,
#endif
#ifdef CONFIG_PM_SLEEP
		.prepare = acpi_subsys_prepare,
		.suspend_late = acpi_subsys_suspend_late,
		.resume_early = acpi_subsys_resume_early,
		.poweroff_late = acpi_subsys_suspend_late,
		.restore_early = acpi_subsys_resume_early,
#endif
	},
};

/* 
                                                                 
                           
                                                    
  
                                                                             
                                                                           
                                                                              
                                                             
  
                                                                                
                                                          
  
                                                                         
                        
 */
int acpi_dev_pm_attach(struct device *dev, bool power_on)
{
	struct acpi_device *adev = acpi_dev_pm_get_node(dev);

	if (!adev)
		return -ENODEV;

	if (dev->pm_domain)
		return -EEXIST;

	acpi_add_pm_notifier(adev, acpi_wakeup_device, dev);
	dev->pm_domain = &acpi_general_pm_domain;
	if (power_on) {
		acpi_dev_pm_full_power(adev);
		__acpi_device_run_wake(adev, false);
	}
	return 0;
}
EXPORT_SYMBOL_GPL(acpi_dev_pm_attach);

/* 
                                                                     
                                
                                                                     
  
                                                                          
                                                                                
            
  
                                                                         
                        
 */
void acpi_dev_pm_detach(struct device *dev, bool power_off)
{
	struct acpi_device *adev = acpi_dev_pm_get_node(dev);

	if (adev && dev->pm_domain == &acpi_general_pm_domain) {
		dev->pm_domain = NULL;
		acpi_remove_pm_notifier(adev, acpi_wakeup_device);
		if (power_off) {
			/*
                                                          
                                                      
                                                         
                                                           
    */
			dev_pm_qos_hide_latency_limit(dev);
			dev_pm_qos_hide_flags(dev);
			__acpi_device_run_wake(adev, false);
			acpi_dev_pm_low_power(dev, adev, ACPI_STATE_S0);
		}
	}
}
EXPORT_SYMBOL_GPL(acpi_dev_pm_detach);

/* 
                                                                    
                                       
                                                 
 */
void acpi_dev_pm_add_dependent(acpi_handle handle, struct device *depdev)
{
	struct acpi_device_physical_node *dep;
	struct acpi_device *adev;

	if (!depdev || acpi_bus_get_device(handle, &adev))
		return;

	mutex_lock(&adev->physical_node_lock);

	list_for_each_entry(dep, &adev->power_dependent, node)
		if (dep->dev == depdev)
			goto out;

	dep = kzalloc(sizeof(*dep), GFP_KERNEL);
	if (dep) {
		dep->dev = depdev;
		list_add_tail(&dep->node, &adev->power_dependent);
	}

 out:
	mutex_unlock(&adev->physical_node_lock);
}
EXPORT_SYMBOL_GPL(acpi_dev_pm_add_dependent);

/* 
                                                                          
                                       
                                                 
 */
void acpi_dev_pm_remove_dependent(acpi_handle handle, struct device *depdev)
{
	struct acpi_device_physical_node *dep;
	struct acpi_device *adev;

	if (!depdev || acpi_bus_get_device(handle, &adev))
		return;

	mutex_lock(&adev->physical_node_lock);

	list_for_each_entry(dep, &adev->power_dependent, node)
		if (dep->dev == depdev) {
			list_del(&dep->node);
			kfree(dep);
			break;
		}

	mutex_unlock(&adev->physical_node_lock);
}
EXPORT_SYMBOL_GPL(acpi_dev_pm_remove_dependent);
#endif /*           */
