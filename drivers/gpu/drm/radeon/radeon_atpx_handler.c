/*
 * Copyright (c) 2010 Red Hat Inc.
 * Author : Dave Airlie <airlied@redhat.com>
 *
 * Licensed under GPLv2
 *
 * ATPX support for both Intel/ATI
 */
#include <linux/vga_switcheroo.h>
#include <linux/slab.h>
#include <acpi/acpi.h>
#include <acpi/acpi_bus.h>
#include <linux/pci.h>

#include "radeon_acpi.h"

struct radeon_atpx_functions {
	bool px_params;
	bool power_cntl;
	bool disp_mux_cntl;
	bool i2c_mux_cntl;
	bool switch_start;
	bool switch_end;
	bool disp_connectors_mapping;
	bool disp_detetion_ports;
};

struct radeon_atpx {
	acpi_handle handle;
	struct radeon_atpx_functions functions;
};

static struct radeon_atpx_priv {
	bool atpx_detected;
	/*                              */
	acpi_handle dhandle;
	struct radeon_atpx atpx;
} radeon_atpx_priv;

struct atpx_verify_interface {
	u16 size;		/*                                               */
	u16 version;		/*         */
	u32 function_bits;	/*                                */
} __packed;

struct atpx_px_params {
	u16 size;		/*                                               */
	u32 valid_flags;	/*                       */
	u32 flags;		/*       */
} __packed;

struct atpx_power_control {
	u16 size;
	u8 dgpu_state;
} __packed;

struct atpx_mux {
	u16 size;
	u16 mux;
} __packed;

/* 
                                         
  
                       
                                          
                                
  
                                                    
                                               
 */
static union acpi_object *radeon_atpx_call(acpi_handle handle, int function,
					   struct acpi_buffer *params)
{
	acpi_status status;
	union acpi_object atpx_arg_elements[2];
	struct acpi_object_list atpx_arg;
	struct acpi_buffer buffer = { ACPI_ALLOCATE_BUFFER, NULL };

	atpx_arg.count = 2;
	atpx_arg.pointer = &atpx_arg_elements[0];

	atpx_arg_elements[0].type = ACPI_TYPE_INTEGER;
	atpx_arg_elements[0].integer.value = function;

	if (params) {
		atpx_arg_elements[1].type = ACPI_TYPE_BUFFER;
		atpx_arg_elements[1].buffer.length = params->length;
		atpx_arg_elements[1].buffer.pointer = params->pointer;
	} else {
		/*                                 */
		atpx_arg_elements[1].type = ACPI_TYPE_INTEGER;
		atpx_arg_elements[1].integer.value = 0;
	}

	status = acpi_evaluate_object(handle, NULL, &atpx_arg, &buffer);

	/*                                                             */
	if (ACPI_FAILURE(status) && status != AE_NOT_FOUND) {
		printk("failed to evaluate ATPX got %s\n",
		       acpi_format_exception(status));
		kfree(buffer.pointer);
		return NULL;
	}

	return buffer.pointer;
}

/* 
                                                          
  
                                 
                                            
  
                                                      
                                                             
                             
 */
static void radeon_atpx_parse_functions(struct radeon_atpx_functions *f, u32 mask)
{
	f->px_params = mask & ATPX_GET_PX_PARAMETERS_SUPPORTED;
	f->power_cntl = mask & ATPX_POWER_CONTROL_SUPPORTED;
	f->disp_mux_cntl = mask & ATPX_DISPLAY_MUX_CONTROL_SUPPORTED;
	f->i2c_mux_cntl = mask & ATPX_I2C_MUX_CONTROL_SUPPORTED;
	f->switch_start = mask & ATPX_GRAPHICS_DEVICE_SWITCH_START_NOTIFICATION_SUPPORTED;
	f->switch_end = mask & ATPX_GRAPHICS_DEVICE_SWITCH_END_NOTIFICATION_SUPPORTED;
	f->disp_connectors_mapping = mask & ATPX_GET_DISPLAY_CONNECTORS_MAPPING_SUPPORTED;
	f->disp_detetion_ports = mask & ATPX_GET_DISPLAY_DETECTION_PORTS_SUPPORTED;
}

/* 
                                                           
  
                            
  
                                                            
                                          
 */
static int radeon_atpx_validate(struct radeon_atpx *atpx)
{
	/*                                          */
	/*                                */
	atpx->functions.power_cntl = true;

	if (atpx->functions.px_params) {
		union acpi_object *info;
		struct atpx_px_params output;
		size_t size;
		u32 valid_bits;

		info = radeon_atpx_call(atpx->handle, ATPX_FUNCTION_GET_PX_PARAMETERS, NULL);
		if (!info)
			return -EIO;

		memset(&output, 0, sizeof(output));

		size = *(u16 *) info->buffer.pointer;
		if (size < 10) {
			printk("ATPX buffer is too small: %zu\n", size);
			kfree(info);
			return -EINVAL;
		}
		size = min(sizeof(output), size);

		memcpy(&output, info->buffer.pointer, size);

		valid_bits = output.flags & output.valid_flags;
		/*                                                        */
		if (valid_bits & ATPX_SEPARATE_MUX_FOR_I2C) {
			atpx->functions.i2c_mux_cntl = true;
			atpx->functions.disp_mux_cntl = true;
		}
		/*                                                     */
		if (valid_bits & (ATPX_CRT1_RGB_SIGNAL_MUXED |
				  ATPX_TV_SIGNAL_MUXED |
				  ATPX_DFP_SIGNAL_MUXED))
			atpx->functions.disp_mux_cntl = true;

		kfree(info);
	}
	return 0;
}

/* 
                                             
  
                            
  
                                                           
                                                               
               
                                          
 */
static int radeon_atpx_verify_interface(struct radeon_atpx *atpx)
{
	union acpi_object *info;
	struct atpx_verify_interface output;
	size_t size;
	int err = 0;

	info = radeon_atpx_call(atpx->handle, ATPX_FUNCTION_VERIFY_INTERFACE, NULL);
	if (!info)
		return -EIO;

	memset(&output, 0, sizeof(output));

	size = *(u16 *) info->buffer.pointer;
	if (size < 8) {
		printk("ATPX buffer is too small: %zu\n", size);
		err = -EINVAL;
		goto out;
	}
	size = min(sizeof(output), size);

	memcpy(&output, info->buffer.pointer, size);

	/*                      */
	printk("ATPX version %u, functions 0x%08x\n",
	       output.version, output.function_bits);

	radeon_atpx_parse_functions(&atpx->functions, output.function_bits);

out:
	kfree(info);
	return err;
}

/* 
                                                              
  
                          
                                                            
  
                                                           
                                              
                                          
 */
static int radeon_atpx_set_discrete_state(struct radeon_atpx *atpx, u8 state)
{
	struct acpi_buffer params;
	union acpi_object *info;
	struct atpx_power_control input;

	if (atpx->functions.power_cntl) {
		input.size = 3;
		input.dgpu_state = state;
		params.length = input.size;
		params.pointer = &input;
		info = radeon_atpx_call(atpx->handle,
					ATPX_FUNCTION_POWER_CONTROL,
					&params);
		if (!info)
			return -EIO;
		kfree(info);
	}
	return 0;
}

/* 
                                                   
  
                          
                                                            
  
                                                                 
                                                                     
               
                                          
 */
static int radeon_atpx_switch_disp_mux(struct radeon_atpx *atpx, u16 mux_id)
{
	struct acpi_buffer params;
	union acpi_object *info;
	struct atpx_mux input;

	if (atpx->functions.disp_mux_cntl) {
		input.size = 4;
		input.mux = mux_id;
		params.length = input.size;
		params.pointer = &input;
		info = radeon_atpx_call(atpx->handle,
					ATPX_FUNCTION_DISPLAY_MUX_CONTROL,
					&params);
		if (!info)
			return -EIO;
		kfree(info);
	}
	return 0;
}

/* 
                                                  
  
                          
                                                            
  
                                                             
                                                                     
               
                                          
 */
static int radeon_atpx_switch_i2c_mux(struct radeon_atpx *atpx, u16 mux_id)
{
	struct acpi_buffer params;
	union acpi_object *info;
	struct atpx_mux input;

	if (atpx->functions.i2c_mux_cntl) {
		input.size = 4;
		input.mux = mux_id;
		params.length = input.size;
		params.pointer = &input;
		info = radeon_atpx_call(atpx->handle,
					ATPX_FUNCTION_I2C_MUX_CONTROL,
					&params);
		if (!info)
			return -EIO;
		kfree(info);
	}
	return 0;
}

/* 
                                                              
  
                          
                                                            
  
                                                                           
                                                                          
                                        
                                          
 */
static int radeon_atpx_switch_start(struct radeon_atpx *atpx, u16 mux_id)
{
	struct acpi_buffer params;
	union acpi_object *info;
	struct atpx_mux input;

	if (atpx->functions.switch_start) {
		input.size = 4;
		input.mux = mux_id;
		params.length = input.size;
		params.pointer = &input;
		info = radeon_atpx_call(atpx->handle,
					ATPX_FUNCTION_GRAPHICS_DEVICE_SWITCH_START_NOTIFICATION,
					&params);
		if (!info)
			return -EIO;
		kfree(info);
	}
	return 0;
}

/* 
                                                            
  
                          
                                                            
  
                                                                         
                                                                          
                                        
                                          
 */
static int radeon_atpx_switch_end(struct radeon_atpx *atpx, u16 mux_id)
{
	struct acpi_buffer params;
	union acpi_object *info;
	struct atpx_mux input;

	if (atpx->functions.switch_end) {
		input.size = 4;
		input.mux = mux_id;
		params.length = input.size;
		params.pointer = &input;
		info = radeon_atpx_call(atpx->handle,
					ATPX_FUNCTION_GRAPHICS_DEVICE_SWITCH_END_NOTIFICATION,
					&params);
		if (!info)
			return -EIO;
		kfree(info);
	}
	return 0;
}

/* 
                                                     
  
                        
  
                                                                              
                              
                                          
 */
static int radeon_atpx_switchto(enum vga_switcheroo_client_id id)
{
	u16 gpu_id;

	if (id == VGA_SWITCHEROO_IGD)
		gpu_id = ATPX_INTEGRATED_GPU;
	else
		gpu_id = ATPX_DISCRETE_GPU;

	radeon_atpx_switch_start(&radeon_atpx_priv.atpx, gpu_id);
	radeon_atpx_switch_disp_mux(&radeon_atpx_priv.atpx, gpu_id);
	radeon_atpx_switch_i2c_mux(&radeon_atpx_priv.atpx, gpu_id);
	radeon_atpx_switch_end(&radeon_atpx_priv.atpx, gpu_id);

	return 0;
}

/* 
                                                            
  
                            
                                                  
  
                                                                        
               
                                          
 */
static int radeon_atpx_power_state(enum vga_switcheroo_client_id id,
				   enum vga_switcheroo_state state)
{
	/*                                           */
	if (id == VGA_SWITCHEROO_IGD)
		return 0;

	radeon_atpx_set_discrete_state(&radeon_atpx_priv.atpx, state);
	return 0;
}

/* 
                                                         
  
                    
  
                                        
                                                       
 */
static bool radeon_atpx_pci_probe_handle(struct pci_dev *pdev)
{
	acpi_handle dhandle, atpx_handle;
	acpi_status status;

	dhandle = DEVICE_ACPI_HANDLE(&pdev->dev);
	if (!dhandle)
		return false;

	status = acpi_get_handle(dhandle, "ATPX", &atpx_handle);
	if (ACPI_FAILURE(status))
		return false;

	radeon_atpx_priv.dhandle = dhandle;
	radeon_atpx_priv.atpx.handle = atpx_handle;
	return true;
}

/* 
                                               
  
                                         
                                          
 */
static int radeon_atpx_init(void)
{
	int r;

	/*                        */
	r = radeon_atpx_verify_interface(&radeon_atpx_priv.atpx);
	if (r)
		return r;

	/*                         */
	r = radeon_atpx_validate(&radeon_atpx_priv.atpx);
	if (r)
		return r;

	return 0;
}

/* 
                                                
  
                    
  
                                                                     
                         
 */
static int radeon_atpx_get_client_id(struct pci_dev *pdev)
{
	if (radeon_atpx_priv.dhandle == DEVICE_ACPI_HANDLE(&pdev->dev))
		return VGA_SWITCHEROO_IGD;
	else
		return VGA_SWITCHEROO_DIS;
}

static struct vga_switcheroo_handler radeon_atpx_handler = {
	.switchto = radeon_atpx_switchto,
	.power_state = radeon_atpx_power_state,
	.init = radeon_atpx_init,
	.get_client_id = radeon_atpx_get_client_id,
};

/* 
                                                 
  
                                            
                                                     
 */
static bool radeon_atpx_detect(void)
{
	char acpi_method_name[255] = { 0 };
	struct acpi_buffer buffer = {sizeof(acpi_method_name), acpi_method_name};
	struct pci_dev *pdev = NULL;
	bool has_atpx = false;
	int vga_count = 0;

	while ((pdev = pci_get_class(PCI_CLASS_DISPLAY_VGA << 8, pdev)) != NULL) {
		vga_count++;

		has_atpx |= (radeon_atpx_pci_probe_handle(pdev) == true);
	}

	/*                                                                 */
	while ((pdev = pci_get_class(PCI_CLASS_DISPLAY_OTHER << 8, pdev)) != NULL) {
		vga_count++;

		has_atpx |= (radeon_atpx_pci_probe_handle(pdev) == true);
	}

	if (has_atpx && vga_count == 2) {
		acpi_get_name(radeon_atpx_priv.atpx.handle, ACPI_FULL_PATHNAME, &buffer);
		printk(KERN_INFO "VGA switcheroo: detected switching method %s handle\n",
		       acpi_method_name);
		radeon_atpx_priv.atpx_detected = true;
		return true;
	}
	return false;
}

/* 
                                                              
  
                                                             
 */
void radeon_register_atpx_handler(void)
{
	bool r;

	/*                                                  */
	r = radeon_atpx_detect();
	if (!r)
		return;

	vga_switcheroo_register_handler(&radeon_atpx_handler);
}

/* 
                                                                  
  
                                                               
 */
void radeon_unregister_atpx_handler(void)
{
	vga_switcheroo_unregister_handler();
}
