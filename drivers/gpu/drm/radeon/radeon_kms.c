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
#include "radeon.h"
#include <drm/radeon_drm.h>
#include "radeon_asic.h"

#include <linux/vga_switcheroo.h>
#include <linux/slab.h>

/* 
                                                           
  
                        
  
                                                        
                                                  
                                                  
                                                
                        
 */
int radeon_driver_unload_kms(struct drm_device *dev)
{
	struct radeon_device *rdev = dev->dev_private;

	if (rdev == NULL)
		return 0;
	if (rdev->rmmio == NULL)
		goto done_free;
	radeon_acpi_fini(rdev);
	radeon_modeset_fini(rdev);
	radeon_device_fini(rdev);

done_free:
	kfree(rdev);
	dev->dev_private = NULL;
	return 0;
}

/* 
                                                       
  
                        
                       
  
                                                      
                                                          
                                                          
                                                    
                                           
                                          
 */
int radeon_driver_load_kms(struct drm_device *dev, unsigned long flags)
{
	struct radeon_device *rdev;
	int r, acpi_status;

	rdev = kzalloc(sizeof(struct radeon_device), GFP_KERNEL);
	if (rdev == NULL) {
		return -ENOMEM;
	}
	dev->dev_private = (void *)rdev;

	/*                 */
	if (drm_pci_device_is_agp(dev)) {
		flags |= RADEON_IS_AGP;
	} else if (pci_is_pcie(dev->pdev)) {
		flags |= RADEON_IS_PCIE;
	} else {
		flags |= RADEON_IS_PCI;
	}

	/*                                                  
                                                        
                                                     
                                                        
                   
  */
	r = radeon_device_init(rdev, dev, dev->pdev, flags);
	if (r) {
		dev_err(&dev->pdev->dev, "Fatal error during GPU init\n");
		goto out;
	}

	/*                                                   
                                                      
                       
  */
	r = radeon_modeset_init(rdev);
	if (r)
		dev_err(&dev->pdev->dev, "Fatal error during modeset init\n");

	/*                                        
                            
  */
	if (!r) {
		acpi_status = radeon_acpi_init(rdev);
		if (acpi_status)
		dev_dbg(&dev->pdev->dev,
				"Error during ACPI methods call\n");
	}

out:
	if (r)
		radeon_driver_unload_kms(dev);
	return r;
}

/* 
                                           
  
                        
                   
                     
                
  
                                                   
 */
static void radeon_set_filp_rights(struct drm_device *dev,
				   struct drm_file **owner,
				   struct drm_file *applier,
				   uint32_t *value)
{
	mutex_lock(&dev->struct_mutex);
	if (*value == 1) {
		/*              */
		if (!*owner)
			*owner = applier;
	} else if (*value == 0) {
		/*                */
		if (*owner == applier)
			*owner = NULL;
	}
	*value = *owner == applier ? 1 : 0;
	mutex_unlock(&dev->struct_mutex);
}

/*
                                  
 */
/* 
                                                        
  
                               
                        
                  
  
                                                                            
                                                                            
                    
                                            
 */
int radeon_info_ioctl(struct drm_device *dev, void *data, struct drm_file *filp)
{
	struct radeon_device *rdev = dev->dev_private;
	struct drm_radeon_info *info = data;
	struct radeon_mode_info *minfo = &rdev->mode_info;
	uint32_t *value, value_tmp, *value_ptr, value_size;
	uint64_t value64;
	struct drm_crtc *crtc;
	int i, found;

	value_ptr = (uint32_t *)((unsigned long)info->value);
	value = &value_tmp;
	value_size = sizeof(uint32_t);

	switch (info->request) {
	case RADEON_INFO_DEVICE_ID:
		*value = dev->pci_device;
		break;
	case RADEON_INFO_NUM_GB_PIPES:
		*value = rdev->num_gb_pipes;
		break;
	case RADEON_INFO_NUM_Z_PIPES:
		*value = rdev->num_z_pipes;
		break;
	case RADEON_INFO_ACCEL_WORKING:
		/*                                                                */
		if ((rdev->family >= CHIP_CEDAR) && (rdev->family <= CHIP_HEMLOCK))
			*value = false;
		else
			*value = rdev->accel_working;
		break;
	case RADEON_INFO_CRTC_FROM_ID:
		if (DRM_COPY_FROM_USER(value, value_ptr, sizeof(uint32_t))) {
			DRM_ERROR("copy_from_user %s:%u\n", __func__, __LINE__);
			return -EFAULT;
		}
		for (i = 0, found = 0; i < rdev->num_crtc; i++) {
			crtc = (struct drm_crtc *)minfo->crtcs[i];
			if (crtc && crtc->base.id == *value) {
				struct radeon_crtc *radeon_crtc = to_radeon_crtc(crtc);
				*value = radeon_crtc->crtc_id;
				found = 1;
				break;
			}
		}
		if (!found) {
			DRM_DEBUG_KMS("unknown crtc id %d\n", *value);
			return -EINVAL;
		}
		break;
	case RADEON_INFO_ACCEL_WORKING2:
		*value = rdev->accel_working;
		break;
	case RADEON_INFO_TILING_CONFIG:
		if (rdev->family >= CHIP_TAHITI)
			*value = rdev->config.si.tile_config;
		else if (rdev->family >= CHIP_CAYMAN)
			*value = rdev->config.cayman.tile_config;
		else if (rdev->family >= CHIP_CEDAR)
			*value = rdev->config.evergreen.tile_config;
		else if (rdev->family >= CHIP_RV770)
			*value = rdev->config.rv770.tile_config;
		else if (rdev->family >= CHIP_R600)
			*value = rdev->config.r600.tile_config;
		else {
			DRM_DEBUG_KMS("tiling config is r6xx+ only!\n");
			return -EINVAL;
		}
		break;
	case RADEON_INFO_WANT_HYPERZ:
		/*                                                        
                                                           
                                                              
    
                                                                
                  */
		if (DRM_COPY_FROM_USER(value, value_ptr, sizeof(uint32_t))) {
			DRM_ERROR("copy_from_user %s:%u\n", __func__, __LINE__);
			return -EFAULT;
		}
		if (*value >= 2) {
			DRM_DEBUG_KMS("WANT_HYPERZ: invalid value %d\n", *value);
			return -EINVAL;
		}
		radeon_set_filp_rights(dev, &rdev->hyperz_filp, filp, value);
		break;
	case RADEON_INFO_WANT_CMASK:
		/*                            */
		if (DRM_COPY_FROM_USER(value, value_ptr, sizeof(uint32_t))) {
			DRM_ERROR("copy_from_user %s:%u\n", __func__, __LINE__);
			return -EFAULT;
		}
		if (*value >= 2) {
			DRM_DEBUG_KMS("WANT_CMASK: invalid value %d\n", *value);
			return -EINVAL;
		}
		radeon_set_filp_rights(dev, &rdev->cmask_filp, filp, value);
		break;
	case RADEON_INFO_CLOCK_CRYSTAL_FREQ:
		/*                           */
		if (rdev->asic->get_xclk)
			*value = radeon_get_xclk(rdev) * 10;
		else
			*value = rdev->clock.spll.reference_freq * 10;
		break;
	case RADEON_INFO_NUM_BACKENDS:
		if (rdev->family >= CHIP_TAHITI)
			*value = rdev->config.si.max_backends_per_se *
				rdev->config.si.max_shader_engines;
		else if (rdev->family >= CHIP_CAYMAN)
			*value = rdev->config.cayman.max_backends_per_se *
				rdev->config.cayman.max_shader_engines;
		else if (rdev->family >= CHIP_CEDAR)
			*value = rdev->config.evergreen.max_backends;
		else if (rdev->family >= CHIP_RV770)
			*value = rdev->config.rv770.max_backends;
		else if (rdev->family >= CHIP_R600)
			*value = rdev->config.r600.max_backends;
		else {
			return -EINVAL;
		}
		break;
	case RADEON_INFO_NUM_TILE_PIPES:
		if (rdev->family >= CHIP_TAHITI)
			*value = rdev->config.si.max_tile_pipes;
		else if (rdev->family >= CHIP_CAYMAN)
			*value = rdev->config.cayman.max_tile_pipes;
		else if (rdev->family >= CHIP_CEDAR)
			*value = rdev->config.evergreen.max_tile_pipes;
		else if (rdev->family >= CHIP_RV770)
			*value = rdev->config.rv770.max_tile_pipes;
		else if (rdev->family >= CHIP_R600)
			*value = rdev->config.r600.max_tile_pipes;
		else {
			return -EINVAL;
		}
		break;
	case RADEON_INFO_FUSION_GART_WORKING:
		*value = 1;
		break;
	case RADEON_INFO_BACKEND_MAP:
		if (rdev->family >= CHIP_TAHITI)
			*value = rdev->config.si.backend_map;
		else if (rdev->family >= CHIP_CAYMAN)
			*value = rdev->config.cayman.backend_map;
		else if (rdev->family >= CHIP_CEDAR)
			*value = rdev->config.evergreen.backend_map;
		else if (rdev->family >= CHIP_RV770)
			*value = rdev->config.rv770.backend_map;
		else if (rdev->family >= CHIP_R600)
			*value = rdev->config.r600.backend_map;
		else {
			return -EINVAL;
		}
		break;
	case RADEON_INFO_VA_START:
		/*                                                   */
		if (rdev->family < CHIP_CAYMAN)
			return -EINVAL;
		*value = RADEON_VA_RESERVED_SIZE;
		break;
	case RADEON_INFO_IB_VM_MAX_SIZE:
		/*                                                   */
		if (rdev->family < CHIP_CAYMAN)
			return -EINVAL;
		*value = RADEON_IB_VM_MAX_SIZE;
		break;
	case RADEON_INFO_MAX_PIPES:
		if (rdev->family >= CHIP_TAHITI)
			*value = rdev->config.si.max_cu_per_sh;
		else if (rdev->family >= CHIP_CAYMAN)
			*value = rdev->config.cayman.max_pipes_per_simd;
		else if (rdev->family >= CHIP_CEDAR)
			*value = rdev->config.evergreen.max_pipes;
		else if (rdev->family >= CHIP_RV770)
			*value = rdev->config.rv770.max_pipes;
		else if (rdev->family >= CHIP_R600)
			*value = rdev->config.r600.max_pipes;
		else {
			return -EINVAL;
		}
		break;
	case RADEON_INFO_TIMESTAMP:
		if (rdev->family < CHIP_R600) {
			DRM_DEBUG_KMS("timestamp is r6xx+ only!\n");
			return -EINVAL;
		}
		value = (uint32_t*)&value64;
		value_size = sizeof(uint64_t);
		value64 = radeon_get_gpu_clock_counter(rdev);
		break;
	case RADEON_INFO_MAX_SE:
		if (rdev->family >= CHIP_TAHITI)
			*value = rdev->config.si.max_shader_engines;
		else if (rdev->family >= CHIP_CAYMAN)
			*value = rdev->config.cayman.max_shader_engines;
		else if (rdev->family >= CHIP_CEDAR)
			*value = rdev->config.evergreen.num_ses;
		else
			*value = 1;
		break;
	case RADEON_INFO_MAX_SH_PER_SE:
		if (rdev->family >= CHIP_TAHITI)
			*value = rdev->config.si.max_sh_per_se;
		else
			return -EINVAL;
		break;
	case RADEON_INFO_FASTFB_WORKING:
		*value = rdev->fastfb_working;
		break;
	case RADEON_INFO_RING_WORKING:
		if (DRM_COPY_FROM_USER(value, value_ptr, sizeof(uint32_t))) {
			DRM_ERROR("copy_from_user %s:%u\n", __func__, __LINE__);
			return -EFAULT;
		}
		switch (*value) {
		case RADEON_CS_RING_GFX:
		case RADEON_CS_RING_COMPUTE:
			*value = rdev->ring[RADEON_RING_TYPE_GFX_INDEX].ready;
			break;
		case RADEON_CS_RING_DMA:
			*value = rdev->ring[R600_RING_TYPE_DMA_INDEX].ready;
			*value |= rdev->ring[CAYMAN_RING_TYPE_DMA1_INDEX].ready;
			break;
		case RADEON_CS_RING_UVD:
			*value = rdev->ring[R600_RING_TYPE_UVD_INDEX].ready;
			break;
		default:
			return -EINVAL;
		}
		break;
	case RADEON_INFO_SI_TILE_MODE_ARRAY:
		if (rdev->family < CHIP_TAHITI) {
			DRM_DEBUG_KMS("tile mode array is si only!\n");
			return -EINVAL;
		}
		value = rdev->config.si.tile_mode_array;
		value_size = sizeof(uint32_t)*32;
		break;
	case RADEON_INFO_SI_CP_DMA_COMPUTE:
		*value = 1;
		break;
	default:
		DRM_DEBUG_KMS("Invalid request %d\n", info->request);
		return -EINVAL;
	}
	if (DRM_COPY_TO_USER(value_ptr, (char*)value, value_size)) {
		DRM_ERROR("copy_to_user %s:%u\n", __func__, __LINE__);
		return -EFAULT;
	}
	return 0;
}


/*
                                                                           
 */
/* 
                                                            
  
                        
  
                                          
                        
 */
int radeon_driver_firstopen_kms(struct drm_device *dev)
{
	return 0;
}

/* 
                                                            
  
                        
  
                                                            
 */
void radeon_driver_lastclose_kms(struct drm_device *dev)
{
	vga_switcheroo_process_delayed_switch();
}

/* 
                                                 
  
                        
                       
  
                                                  
                                          
 */
int radeon_driver_open_kms(struct drm_device *dev, struct drm_file *file_priv)
{
	struct radeon_device *rdev = dev->dev_private;

	file_priv->driver_priv = NULL;

	/*                                            */
	if (rdev->family >= CHIP_CAYMAN) {
		struct radeon_fpriv *fpriv;
		struct radeon_bo_va *bo_va;
		int r;

		fpriv = kzalloc(sizeof(*fpriv), GFP_KERNEL);
		if (unlikely(!fpriv)) {
			return -ENOMEM;
		}

		radeon_vm_init(rdev, &fpriv->vm);

		r = radeon_bo_reserve(rdev->ring_tmp_bo.bo, false);
		if (r)
			return r;

		/*                                      
                           */
		bo_va = radeon_vm_bo_add(rdev, &fpriv->vm,
					 rdev->ring_tmp_bo.bo);
		r = radeon_vm_bo_set_addr(rdev, bo_va, RADEON_VA_IB_OFFSET,
					  RADEON_VM_PAGE_READABLE |
					  RADEON_VM_PAGE_SNOOPED);

		radeon_bo_unreserve(rdev->ring_tmp_bo.bo);
		if (r) {
			radeon_vm_fini(rdev, &fpriv->vm);
			kfree(fpriv);
			return r;
		}

		file_priv->driver_priv = fpriv;
	}
	return 0;
}

/* 
                                                            
  
                        
                       
  
                                                             
 */
void radeon_driver_postclose_kms(struct drm_device *dev,
				 struct drm_file *file_priv)
{
	struct radeon_device *rdev = dev->dev_private;

	/*                                            */
	if (rdev->family >= CHIP_CAYMAN && file_priv->driver_priv) {
		struct radeon_fpriv *fpriv = file_priv->driver_priv;
		struct radeon_bo_va *bo_va;
		int r;

		r = radeon_bo_reserve(rdev->ring_tmp_bo.bo, false);
		if (!r) {
			bo_va = radeon_vm_bo_find(&fpriv->vm,
						  rdev->ring_tmp_bo.bo);
			if (bo_va)
				radeon_vm_bo_rmv(rdev, bo_va);
			radeon_bo_unreserve(rdev->ring_tmp_bo.bo);
		}

		radeon_vm_fini(rdev, &fpriv->vm);
		kfree(fpriv);
		file_priv->driver_priv = NULL;
	}
}

/* 
                                                          
  
                        
                       
  
                                                                     
               
 */
void radeon_driver_preclose_kms(struct drm_device *dev,
				struct drm_file *file_priv)
{
	struct radeon_device *rdev = dev->dev_private;
	if (rdev->hyperz_filp == file_priv)
		rdev->hyperz_filp = NULL;
	if (rdev->cmask_filp == file_priv)
		rdev->cmask_filp = NULL;
	radeon_uvd_free_handles(rdev, file_priv);
}

/*
                            
 */
/* 
                                                  
  
                        
                                          
  
                                                          
                                                      
 */
u32 radeon_get_vblank_counter_kms(struct drm_device *dev, int crtc)
{
	struct radeon_device *rdev = dev->dev_private;

	if (crtc < 0 || crtc >= rdev->num_crtc) {
		DRM_ERROR("Invalid crtc %d\n", crtc);
		return -EINVAL;
	}

	return radeon_get_vblank_counter(rdev, crtc);
}

/* 
                                                     
  
                        
                                             
  
                                                          
                                            
 */
int radeon_enable_vblank_kms(struct drm_device *dev, int crtc)
{
	struct radeon_device *rdev = dev->dev_private;
	unsigned long irqflags;
	int r;

	if (crtc < 0 || crtc >= rdev->num_crtc) {
		DRM_ERROR("Invalid crtc %d\n", crtc);
		return -EINVAL;
	}

	spin_lock_irqsave(&rdev->irq.lock, irqflags);
	rdev->irq.crtc_vblank_int[crtc] = true;
	r = radeon_irq_set(rdev);
	spin_unlock_irqrestore(&rdev->irq.lock, irqflags);
	return r;
}

/* 
                                                       
  
                        
                                              
  
                                                           
 */
void radeon_disable_vblank_kms(struct drm_device *dev, int crtc)
{
	struct radeon_device *rdev = dev->dev_private;
	unsigned long irqflags;

	if (crtc < 0 || crtc >= rdev->num_crtc) {
		DRM_ERROR("Invalid crtc %d\n", crtc);
		return;
	}

	spin_lock_irqsave(&rdev->irq.lock, irqflags);
	rdev->irq.crtc_vblank_int[crtc] = false;
	radeon_irq_set(rdev);
	spin_unlock_irqrestore(&rdev->irq.lock, irqflags);
}

/* 
                                                         
  
                        
                                       
                        
                           
                                     
  
                                                        
                                  
                                                                      
 */
int radeon_get_vblank_timestamp_kms(struct drm_device *dev, int crtc,
				    int *max_error,
				    struct timeval *vblank_time,
				    unsigned flags)
{
	struct drm_crtc *drmcrtc;
	struct radeon_device *rdev = dev->dev_private;

	if (crtc < 0 || crtc >= dev->num_crtcs) {
		DRM_ERROR("Invalid crtc %d\n", crtc);
		return -EINVAL;
	}

	/*                          */
	drmcrtc = &rdev->mode_info.crtcs[crtc]->base;
	if (!drmcrtc)
		return -EINVAL;

	/*                                               */
	return drm_calc_vbltimestamp_from_scanoutpos(dev, crtc, max_error,
						     vblank_time, flags,
						     drmcrtc);
}

/*
         
 */
int radeon_dma_ioctl_kms(struct drm_device *dev, void *data,
			 struct drm_file *file_priv)
{
	/*                   */
	return -EINVAL;
}

#define KMS_INVALID_IOCTL(name)						\
int name(struct drm_device *dev, void *data, struct drm_file *file_priv)\
{									\
	DRM_ERROR("invalid ioctl with kms %s\n", __func__);		\
	return -EINVAL;							\
}

/*
                                             
 */
KMS_INVALID_IOCTL(radeon_cp_init_kms)
KMS_INVALID_IOCTL(radeon_cp_start_kms)
KMS_INVALID_IOCTL(radeon_cp_stop_kms)
KMS_INVALID_IOCTL(radeon_cp_reset_kms)
KMS_INVALID_IOCTL(radeon_cp_idle_kms)
KMS_INVALID_IOCTL(radeon_cp_resume_kms)
KMS_INVALID_IOCTL(radeon_engine_reset_kms)
KMS_INVALID_IOCTL(radeon_fullscreen_kms)
KMS_INVALID_IOCTL(radeon_cp_swap_kms)
KMS_INVALID_IOCTL(radeon_cp_clear_kms)
KMS_INVALID_IOCTL(radeon_cp_vertex_kms)
KMS_INVALID_IOCTL(radeon_cp_indices_kms)
KMS_INVALID_IOCTL(radeon_cp_texture_kms)
KMS_INVALID_IOCTL(radeon_cp_stipple_kms)
KMS_INVALID_IOCTL(radeon_cp_indirect_kms)
KMS_INVALID_IOCTL(radeon_cp_vertex2_kms)
KMS_INVALID_IOCTL(radeon_cp_cmdbuf_kms)
KMS_INVALID_IOCTL(radeon_cp_getparam_kms)
KMS_INVALID_IOCTL(radeon_cp_flip_kms)
KMS_INVALID_IOCTL(radeon_mem_alloc_kms)
KMS_INVALID_IOCTL(radeon_mem_free_kms)
KMS_INVALID_IOCTL(radeon_mem_init_heap_kms)
KMS_INVALID_IOCTL(radeon_irq_emit_kms)
KMS_INVALID_IOCTL(radeon_irq_wait_kms)
KMS_INVALID_IOCTL(radeon_cp_setparam_kms)
KMS_INVALID_IOCTL(radeon_surface_alloc_kms)
KMS_INVALID_IOCTL(radeon_surface_free_kms)


struct drm_ioctl_desc radeon_ioctls_kms[] = {
	DRM_IOCTL_DEF_DRV(RADEON_CP_INIT, radeon_cp_init_kms, DRM_AUTH|DRM_MASTER|DRM_ROOT_ONLY),
	DRM_IOCTL_DEF_DRV(RADEON_CP_START, radeon_cp_start_kms, DRM_AUTH|DRM_MASTER|DRM_ROOT_ONLY),
	DRM_IOCTL_DEF_DRV(RADEON_CP_STOP, radeon_cp_stop_kms, DRM_AUTH|DRM_MASTER|DRM_ROOT_ONLY),
	DRM_IOCTL_DEF_DRV(RADEON_CP_RESET, radeon_cp_reset_kms, DRM_AUTH|DRM_MASTER|DRM_ROOT_ONLY),
	DRM_IOCTL_DEF_DRV(RADEON_CP_IDLE, radeon_cp_idle_kms, DRM_AUTH),
	DRM_IOCTL_DEF_DRV(RADEON_CP_RESUME, radeon_cp_resume_kms, DRM_AUTH),
	DRM_IOCTL_DEF_DRV(RADEON_RESET, radeon_engine_reset_kms, DRM_AUTH),
	DRM_IOCTL_DEF_DRV(RADEON_FULLSCREEN, radeon_fullscreen_kms, DRM_AUTH),
	DRM_IOCTL_DEF_DRV(RADEON_SWAP, radeon_cp_swap_kms, DRM_AUTH),
	DRM_IOCTL_DEF_DRV(RADEON_CLEAR, radeon_cp_clear_kms, DRM_AUTH),
	DRM_IOCTL_DEF_DRV(RADEON_VERTEX, radeon_cp_vertex_kms, DRM_AUTH),
	DRM_IOCTL_DEF_DRV(RADEON_INDICES, radeon_cp_indices_kms, DRM_AUTH),
	DRM_IOCTL_DEF_DRV(RADEON_TEXTURE, radeon_cp_texture_kms, DRM_AUTH),
	DRM_IOCTL_DEF_DRV(RADEON_STIPPLE, radeon_cp_stipple_kms, DRM_AUTH),
	DRM_IOCTL_DEF_DRV(RADEON_INDIRECT, radeon_cp_indirect_kms, DRM_AUTH|DRM_MASTER|DRM_ROOT_ONLY),
	DRM_IOCTL_DEF_DRV(RADEON_VERTEX2, radeon_cp_vertex2_kms, DRM_AUTH),
	DRM_IOCTL_DEF_DRV(RADEON_CMDBUF, radeon_cp_cmdbuf_kms, DRM_AUTH),
	DRM_IOCTL_DEF_DRV(RADEON_GETPARAM, radeon_cp_getparam_kms, DRM_AUTH),
	DRM_IOCTL_DEF_DRV(RADEON_FLIP, radeon_cp_flip_kms, DRM_AUTH),
	DRM_IOCTL_DEF_DRV(RADEON_ALLOC, radeon_mem_alloc_kms, DRM_AUTH),
	DRM_IOCTL_DEF_DRV(RADEON_FREE, radeon_mem_free_kms, DRM_AUTH),
	DRM_IOCTL_DEF_DRV(RADEON_INIT_HEAP, radeon_mem_init_heap_kms, DRM_AUTH|DRM_MASTER|DRM_ROOT_ONLY),
	DRM_IOCTL_DEF_DRV(RADEON_IRQ_EMIT, radeon_irq_emit_kms, DRM_AUTH),
	DRM_IOCTL_DEF_DRV(RADEON_IRQ_WAIT, radeon_irq_wait_kms, DRM_AUTH),
	DRM_IOCTL_DEF_DRV(RADEON_SETPARAM, radeon_cp_setparam_kms, DRM_AUTH),
	DRM_IOCTL_DEF_DRV(RADEON_SURF_ALLOC, radeon_surface_alloc_kms, DRM_AUTH),
	DRM_IOCTL_DEF_DRV(RADEON_SURF_FREE, radeon_surface_free_kms, DRM_AUTH),
	/*     */
	DRM_IOCTL_DEF_DRV(RADEON_GEM_INFO, radeon_gem_info_ioctl, DRM_AUTH|DRM_UNLOCKED),
	DRM_IOCTL_DEF_DRV(RADEON_GEM_CREATE, radeon_gem_create_ioctl, DRM_AUTH|DRM_UNLOCKED),
	DRM_IOCTL_DEF_DRV(RADEON_GEM_MMAP, radeon_gem_mmap_ioctl, DRM_AUTH|DRM_UNLOCKED),
	DRM_IOCTL_DEF_DRV(RADEON_GEM_SET_DOMAIN, radeon_gem_set_domain_ioctl, DRM_AUTH|DRM_UNLOCKED),
	DRM_IOCTL_DEF_DRV(RADEON_GEM_PREAD, radeon_gem_pread_ioctl, DRM_AUTH|DRM_UNLOCKED),
	DRM_IOCTL_DEF_DRV(RADEON_GEM_PWRITE, radeon_gem_pwrite_ioctl, DRM_AUTH|DRM_UNLOCKED),
	DRM_IOCTL_DEF_DRV(RADEON_GEM_WAIT_IDLE, radeon_gem_wait_idle_ioctl, DRM_AUTH|DRM_UNLOCKED),
	DRM_IOCTL_DEF_DRV(RADEON_CS, radeon_cs_ioctl, DRM_AUTH|DRM_UNLOCKED),
	DRM_IOCTL_DEF_DRV(RADEON_INFO, radeon_info_ioctl, DRM_AUTH|DRM_UNLOCKED),
	DRM_IOCTL_DEF_DRV(RADEON_GEM_SET_TILING, radeon_gem_set_tiling_ioctl, DRM_AUTH|DRM_UNLOCKED),
	DRM_IOCTL_DEF_DRV(RADEON_GEM_GET_TILING, radeon_gem_get_tiling_ioctl, DRM_AUTH|DRM_UNLOCKED),
	DRM_IOCTL_DEF_DRV(RADEON_GEM_BUSY, radeon_gem_busy_ioctl, DRM_AUTH|DRM_UNLOCKED),
	DRM_IOCTL_DEF_DRV(RADEON_GEM_VA, radeon_gem_va_ioctl, DRM_AUTH|DRM_UNLOCKED),
};
int radeon_max_kms_ioctl = DRM_ARRAY_SIZE(radeon_ioctls_kms);
