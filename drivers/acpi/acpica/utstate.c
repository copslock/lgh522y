/*                                                                              
  
                                                         
  
                                                                              */

/*
 * Copyright (C) 2000 - 2013, Intel Corp.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions, and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce at minimum a disclaimer
 *    substantially similar to the "NO WARRANTY" disclaimer below
 *    ("Disclaimer") and any redistribution must be conditioned upon
 *    including a substantially similar Disclaimer requirement for further
 *    binary redistribution.
 * 3. Neither the names of the above-listed copyright holders nor the names
 *    of any contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES.
 */

#include <acpi/acpi.h>
#include "accommon.h"

#define _COMPONENT          ACPI_UTILITIES
ACPI_MODULE_NAME("utstate")

/*                                                                              
  
                                                 
  
                                                                     
                                                     
                                                                 
  
                      
  
                                              
  
                                                                              */
acpi_status
acpi_ut_create_pkg_state_and_push(void *internal_object,
				  void *external_object,
				  u16 index,
				  union acpi_generic_state **state_list)
{
	union acpi_generic_state *state;

	ACPI_FUNCTION_ENTRY();

	state =
	    acpi_ut_create_pkg_state(internal_object, external_object, index);
	if (!state) {
		return (AE_NO_MEMORY);
	}

	acpi_ut_push_generic_state(state_list, state);
	return (AE_OK);
}

/*                                                                              
  
                                          
  
                                                             
                                                          
  
                    
  
                                                      
  
                                                                              */

void
acpi_ut_push_generic_state(union acpi_generic_state **list_head,
			   union acpi_generic_state *state)
{
	ACPI_FUNCTION_ENTRY();

	/*                                                          */

	state->common.next = *list_head;
	*list_head = state;
	return;
}

/*                                                                              
  
                                         
  
                                                             
  
                                       
  
                                                     
  
                                                                              */

union acpi_generic_state *acpi_ut_pop_generic_state(union acpi_generic_state
						    **list_head)
{
	union acpi_generic_state *state;

	ACPI_FUNCTION_ENTRY();

	/*                                                         */

	state = *list_head;
	if (state) {

		/*                      */

		*list_head = state->common.next;
	}

	return (state);
}

/*                                                                              
  
                                            
  
                    
  
                                                      
  
                                                                         
                                                                             
  
                                                                              */

union acpi_generic_state *acpi_ut_create_generic_state(void)
{
	union acpi_generic_state *state;

	ACPI_FUNCTION_ENTRY();

	state = acpi_os_acquire_object(acpi_gbl_state_cache);
	if (state) {

		/*            */
		memset(state, 0, sizeof(union acpi_generic_state));
		state->common.descriptor_type = ACPI_DESC_TYPE_STATE;
	}

	return (state);
}

/*                                                                              
  
                                           
  
                    
  
                                                 
  
                                                                            
                                                                
  
                                                                              */

struct acpi_thread_state *acpi_ut_create_thread_state(void)
{
	union acpi_generic_state *state;

	ACPI_FUNCTION_ENTRY();

	/*                                 */

	state = acpi_ut_create_generic_state();
	if (!state) {
		return (NULL);
	}

	/*                                           */

	state->common.descriptor_type = ACPI_DESC_TYPE_STATE_THREAD;
	state->thread.thread_id = acpi_os_get_thread_id();

	/*                                                                      */

	if (!state->thread.thread_id) {
		ACPI_ERROR((AE_INFO, "Invalid zero ID from AcpiOsGetThreadId"));
		state->thread.thread_id = (acpi_thread_id) 1;
	}

	return ((struct acpi_thread_state *)state);
}

/*                                                                              
  
                                           
  
                                                                             
                                                               
  
                                                 
  
                                                                             
                                                                          
                            
  
                                                                              */

union acpi_generic_state *acpi_ut_create_update_state(union acpi_operand_object
						      *object, u16 action)
{
	union acpi_generic_state *state;

	ACPI_FUNCTION_ENTRY();

	/*                                 */

	state = acpi_ut_create_generic_state();
	if (!state) {
		return (NULL);
	}

	/*                                           */

	state->common.descriptor_type = ACPI_DESC_TYPE_STATE_UPDATE;
	state->update.object = object;
	state->update.value = action;
	return (state);
}

/*                                                                              
  
                                        
  
                                                                             
                                                               
  
                                                 
  
                                        
  
                                                                              */

union acpi_generic_state *acpi_ut_create_pkg_state(void *internal_object,
						   void *external_object,
						   u16 index)
{
	union acpi_generic_state *state;

	ACPI_FUNCTION_ENTRY();

	/*                                 */

	state = acpi_ut_create_generic_state();
	if (!state) {
		return (NULL);
	}

	/*                                           */

	state->common.descriptor_type = ACPI_DESC_TYPE_STATE_PACKAGE;
	state->pkg.source_object = (union acpi_operand_object *)internal_object;
	state->pkg.dest_object = external_object;
	state->pkg.index = index;
	state->pkg.num_packages = 1;
	return (state);
}

/*                                                                              
  
                                            
  
                    
  
                                                 
  
                                                                             
                                                                 
  
                                                                              */

union acpi_generic_state *acpi_ut_create_control_state(void)
{
	union acpi_generic_state *state;

	ACPI_FUNCTION_ENTRY();

	/*                                 */

	state = acpi_ut_create_generic_state();
	if (!state) {
		return (NULL);
	}

	/*                                            */

	state->common.descriptor_type = ACPI_DESC_TYPE_STATE_CONTROL;
	state->common.state = ACPI_CONTROL_CONDITIONAL_EXECUTING;
	return (state);
}

/*                                                                              
  
                                            
  
                                                                    
  
                    
  
                                                                             
                            
  
                                                                              */

void acpi_ut_delete_generic_state(union acpi_generic_state *state)
{
	ACPI_FUNCTION_ENTRY();

	/*                   */

	if (state) {
		(void)acpi_os_release_object(acpi_gbl_state_cache, state);
	}
	return;
}
