/*
 * Copyright 2008 Advanced Micro Devices, Inc.
 * Copyright 2008 Red Hat Inc.
 * Copyright 2009 Jerome Glisse.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors: Dave Airlie
 *          Alex Deucher
 *          Jerome Glisse
 */
#include <drm/drmP.h>
#include <drm/drm_crtc_helper.h>
#include <drm/radeon_drm.h>
#include "radeon_reg.h"
#include "radeon.h"
#include "atom.h"

#define RADEON_WAIT_IDLE_TIMEOUT 200

/* 
                                                      
  
                      
  
                                                                 
                                                            
                        
 */
irqreturn_t radeon_driver_irq_handler_kms(DRM_IRQ_ARGS)
{
	struct drm_device *dev = (struct drm_device *) arg;
	struct radeon_device *rdev = dev->dev_private;

	return radeon_irq_process(rdev);
}

/*
                                                              
 */
/* 
                                                          
  
                     
  
                                                       
                                                        
                                                          
                                                         
                                          
 */
static void radeon_hotplug_work_func(struct work_struct *work)
{
	struct radeon_device *rdev = container_of(work, struct radeon_device,
						  hotplug_work);
	struct drm_device *dev = rdev->ddev;
	struct drm_mode_config *mode_config = &dev->mode_config;
	struct drm_connector *connector;

	if (mode_config->num_connector) {
		list_for_each_entry(connector, &mode_config->connector_list, head)
			radeon_connector_hotplug(connector);
	}
	/*                                                             */
	drm_helper_hpd_irq_event(dev);
}

/* 
                                                                 
  
                        
  
                                                
                                                           
 */
void radeon_driver_irq_preinstall_kms(struct drm_device *dev)
{
	struct radeon_device *rdev = dev->dev_private;
	unsigned long irqflags;
	unsigned i;

	spin_lock_irqsave(&rdev->irq.lock, irqflags);
	/*                          */
	for (i = 0; i < RADEON_NUM_RINGS; i++)
		atomic_set(&rdev->irq.ring_int[i], 0);
	for (i = 0; i < RADEON_MAX_HPD_PINS; i++)
		rdev->irq.hpd[i] = false;
	for (i = 0; i < RADEON_MAX_CRTCS; i++) {
		rdev->irq.crtc_vblank_int[i] = false;
		atomic_set(&rdev->irq.pflip[i], 0);
		rdev->irq.afmt[i] = false;
	}
	radeon_irq_set(rdev);
	spin_unlock_irqrestore(&rdev->irq.lock, irqflags);
	/*            */
	radeon_irq_process(rdev);
}

/* 
                                                                  
  
                        
  
                                                            
                        
 */
int radeon_driver_irq_postinstall_kms(struct drm_device *dev)
{
	dev->max_vblank_count = 0x001fffff;
	return 0;
}

/* 
                                                               
  
                        
  
                                                                       
 */
void radeon_driver_irq_uninstall_kms(struct drm_device *dev)
{
	struct radeon_device *rdev = dev->dev_private;
	unsigned long irqflags;
	unsigned i;

	if (rdev == NULL) {
		return;
	}
	spin_lock_irqsave(&rdev->irq.lock, irqflags);
	/*                          */
	for (i = 0; i < RADEON_NUM_RINGS; i++)
		atomic_set(&rdev->irq.ring_int[i], 0);
	for (i = 0; i < RADEON_MAX_HPD_PINS; i++)
		rdev->irq.hpd[i] = false;
	for (i = 0; i < RADEON_MAX_CRTCS; i++) {
		rdev->irq.crtc_vblank_int[i] = false;
		atomic_set(&rdev->irq.pflip[i], 0);
		rdev->irq.afmt[i] = false;
	}
	radeon_irq_set(rdev);
	spin_unlock_irqrestore(&rdev->irq.lock, irqflags);
}

/* 
                                           
  
                               
  
                                                   
                                                           
                                                        
                         
 */
static bool radeon_msi_ok(struct radeon_device *rdev)
{
	/*                                             */
	if (rdev->family < CHIP_RV380)
		return false;

	/*                        */
	if (rdev->flags & RADEON_IS_AGP)
		return false;

	/*              */
	if (radeon_msi == 1)
		return true;
	else if (radeon_msi == 0)
		return false;

	/*        */
	/*                                        */
	if ((rdev->pdev->device == 0x791f) &&
	    (rdev->pdev->subsystem_vendor == 0x103c) &&
	    (rdev->pdev->subsystem_device == 0x30c2))
		return true;

	/*                                          */
	if ((rdev->pdev->device == 0x791f) &&
	    (rdev->pdev->subsystem_vendor == 0x1028) &&
	    (rdev->pdev->subsystem_device == 0x01fc))
		return true;

	/*                                          */
	if ((rdev->pdev->device == 0x791f) &&
	    (rdev->pdev->subsystem_vendor == 0x1028) &&
	    (rdev->pdev->subsystem_device == 0x01fd))
		return true;

	/*                                             */
	if ((rdev->pdev->device == 0x791f) &&
	    (rdev->pdev->subsystem_vendor == 0x107b) &&
	    (rdev->pdev->subsystem_device == 0x0185))
		return true;

	/*                                              */
	if (rdev->family == CHIP_RS690)
		return true;

	/*                                              
                                                               
                          
  */
	if (rdev->family == CHIP_RV515)
		return false;
	if (rdev->flags & RADEON_IS_IGP) {
		/*                          */
		if (rdev->family >= CHIP_PALM)
			return true;
		/*                                      */
		return false;
	}

	return true;
}

/* 
                                                   
  
                               
  
                                                                      
                                            
 */
int radeon_irq_kms_init(struct radeon_device *rdev)
{
	int r = 0;

	spin_lock_init(&rdev->irq.lock);
	r = drm_vblank_init(rdev->ddev, rdev->num_crtc);
	if (r) {
		return r;
	}
	/*            */
	rdev->msi_enabled = 0;

	if (radeon_msi_ok(rdev)) {
		int ret = pci_enable_msi(rdev->pdev);
		if (!ret) {
			rdev->msi_enabled = 1;
			dev_info(rdev->dev, "radeon: using MSI.\n");
		}
	}
	rdev->irq.installed = true;
	r = drm_irq_install(rdev->ddev);
	if (r) {
		rdev->irq.installed = false;
		return r;
	}

	INIT_WORK(&rdev->hotplug_work, radeon_hotplug_work_func);
	INIT_WORK(&rdev->audio_work, r600_audio_update_hdmi);

	DRM_INFO("radeon: irq initialized.\n");
	return 0;
}

/* 
                                                        
  
                               
  
                                                                             
 */
void radeon_irq_kms_fini(struct radeon_device *rdev)
{
	drm_vblank_cleanup(rdev->ddev);
	if (rdev->irq.installed) {
		drm_irq_uninstall(rdev->ddev);
		rdev->irq.installed = false;
		if (rdev->msi_enabled)
			pci_disable_msi(rdev->pdev);
		flush_work(&rdev->hotplug_work);
	}
}

/* 
                                                        
  
                               
                                                 
  
                                                                  
                                                                
                     
 */
void radeon_irq_kms_sw_irq_get(struct radeon_device *rdev, int ring)
{
	unsigned long irqflags;

	if (!rdev->ddev->irq_enabled)
		return;

	if (atomic_inc_return(&rdev->irq.ring_int[ring]) == 1) {
		spin_lock_irqsave(&rdev->irq.lock, irqflags);
		radeon_irq_set(rdev);
		spin_unlock_irqrestore(&rdev->irq.lock, irqflags);
	}
}

/* 
                                                         
  
                               
                                                  
  
                                                                   
                                                                
                     
 */
void radeon_irq_kms_sw_irq_put(struct radeon_device *rdev, int ring)
{
	unsigned long irqflags;

	if (!rdev->ddev->irq_enabled)
		return;

	if (atomic_dec_and_test(&rdev->irq.ring_int[ring])) {
		spin_lock_irqsave(&rdev->irq.lock, irqflags);
		radeon_irq_set(rdev);
		spin_unlock_irqrestore(&rdev->irq.lock, irqflags);
	}
}

/* 
                                                           
  
                               
                                                 
  
                                                                  
                                                    
 */
void radeon_irq_kms_pflip_irq_get(struct radeon_device *rdev, int crtc)
{
	unsigned long irqflags;

	if (crtc < 0 || crtc >= rdev->num_crtc)
		return;

	if (!rdev->ddev->irq_enabled)
		return;

	if (atomic_inc_return(&rdev->irq.pflip[crtc]) == 1) {
		spin_lock_irqsave(&rdev->irq.lock, irqflags);
		radeon_irq_set(rdev);
		spin_unlock_irqrestore(&rdev->irq.lock, irqflags);
	}
}

/* 
                                                            
  
                               
                                                  
  
                                                                   
                                                    
 */
void radeon_irq_kms_pflip_irq_put(struct radeon_device *rdev, int crtc)
{
	unsigned long irqflags;

	if (crtc < 0 || crtc >= rdev->num_crtc)
		return;

	if (!rdev->ddev->irq_enabled)
		return;

	if (atomic_dec_and_test(&rdev->irq.pflip[crtc])) {
		spin_lock_irqsave(&rdev->irq.lock, irqflags);
		radeon_irq_set(rdev);
		spin_unlock_irqrestore(&rdev->irq.lock, irqflags);
	}
}

/* 
                                                                    
  
                               
                                                        
  
                                                                           
 */
void radeon_irq_kms_enable_afmt(struct radeon_device *rdev, int block)
{
	unsigned long irqflags;

	if (!rdev->ddev->irq_enabled)
		return;

	spin_lock_irqsave(&rdev->irq.lock, irqflags);
	rdev->irq.afmt[block] = true;
	radeon_irq_set(rdev);
	spin_unlock_irqrestore(&rdev->irq.lock, irqflags);

}

/* 
                                                                      
  
                               
                                                         
  
                                                                            
 */
void radeon_irq_kms_disable_afmt(struct radeon_device *rdev, int block)
{
	unsigned long irqflags;

	if (!rdev->ddev->irq_enabled)
		return;

	spin_lock_irqsave(&rdev->irq.lock, irqflags);
	rdev->irq.afmt[block] = false;
	radeon_irq_set(rdev);
	spin_unlock_irqrestore(&rdev->irq.lock, irqflags);
}

/* 
                                                              
  
                               
                                                  
  
                                                                           
 */
void radeon_irq_kms_enable_hpd(struct radeon_device *rdev, unsigned hpd_mask)
{
	unsigned long irqflags;
	int i;

	if (!rdev->ddev->irq_enabled)
		return;

	spin_lock_irqsave(&rdev->irq.lock, irqflags);
	for (i = 0; i < RADEON_MAX_HPD_PINS; ++i)
		rdev->irq.hpd[i] |= !!(hpd_mask & (1 << i));
	radeon_irq_set(rdev);
	spin_unlock_irqrestore(&rdev->irq.lock, irqflags);
}

/* 
                                                                
  
                               
                                                   
  
                                                                            
 */
void radeon_irq_kms_disable_hpd(struct radeon_device *rdev, unsigned hpd_mask)
{
	unsigned long irqflags;
	int i;

	if (!rdev->ddev->irq_enabled)
		return;

	spin_lock_irqsave(&rdev->irq.lock, irqflags);
	for (i = 0; i < RADEON_MAX_HPD_PINS; ++i)
		rdev->irq.hpd[i] &= !(hpd_mask & (1 << i));
	radeon_irq_set(rdev);
	spin_unlock_irqrestore(&rdev->irq.lock, irqflags);
}

