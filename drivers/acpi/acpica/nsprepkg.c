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
#include "acnamesp.h"
#include "acpredef.h"

#define _COMPONENT          ACPI_NAMESPACE
ACPI_MODULE_NAME("nsprepkg")

/*                  */
static acpi_status
acpi_ns_check_package_list(struct acpi_predefined_data *data,
			   const union acpi_predefined_info *package,
			   union acpi_operand_object **elements, u32 count);

static acpi_status
acpi_ns_check_package_elements(struct acpi_predefined_data *data,
			       union acpi_operand_object **elements,
			       u8 type1,
			       u32 count1,
			       u8 type2, u32 count2, u32 start_index);

/*                                                                              
  
                                     
  
                                                                          
                                                                             
                                                                      
  
                      
  
                                                                         
                                                
  
                                                                              */

acpi_status
acpi_ns_check_package(struct acpi_predefined_data *data,
		      union acpi_operand_object **return_object_ptr)
{
	union acpi_operand_object *return_object = *return_object_ptr;
	const union acpi_predefined_info *package;
	union acpi_operand_object **elements;
	acpi_status status = AE_OK;
	u32 expected_count;
	u32 count;
	u32 i;

	ACPI_FUNCTION_NAME(ns_check_package);

	/*                                                           */

	package = data->predefined + 1;

	ACPI_DEBUG_PRINT((ACPI_DB_NAMES,
			  "%s Validating return Package of Type %X, Count %X\n",
			  data->pathname, package->ret_info.type,
			  return_object->package.count));

	/*
                                                                   
                                      
  */
	acpi_ns_remove_null_elements(data, package->ret_info.type,
				     return_object);

	/*                                          */

	elements = return_object->package.elements;
	count = return_object->package.count;

	/*
                                                                    
                                                     
  */
	if (!count) {
		if (package->ret_info.type == ACPI_PTYPE1_VAR) {
			return (AE_OK);
		}

		ACPI_WARN_PREDEFINED((AE_INFO, data->pathname, data->node_flags,
				      "Return Package has no elements (empty)"));

		return (AE_AML_OPERAND_VALUE);
	}

	/*
                                                    
   
                                          
                                        
  */
	switch (package->ret_info.type) {
	case ACPI_PTYPE1_FIXED:

		/*
                                                             
    
                                   
                                                                   
   */
		expected_count =
		    package->ret_info.count1 + package->ret_info.count2;
		if (count < expected_count) {
			goto package_too_small;
		} else if (count > expected_count) {
			ACPI_DEBUG_PRINT((ACPI_DB_REPAIR,
					  "%s: Return Package is larger than needed - "
					  "found %u, expected %u\n",
					  data->pathname, count,
					  expected_count));
		}

		/*                                               */

		status = acpi_ns_check_package_elements(data, elements,
							package->ret_info.
							object_type1,
							package->ret_info.
							count1,
							package->ret_info.
							object_type2,
							package->ret_info.
							count2, 0);
		break;

	case ACPI_PTYPE1_VAR:

		/*
                                                                      
                                      
   */
		for (i = 0; i < count; i++) {
			status = acpi_ns_check_object_type(data, elements,
							   package->ret_info.
							   object_type1, i);
			if (ACPI_FAILURE(status)) {
				return (status);
			}
			elements++;
		}
		break;

	case ACPI_PTYPE1_OPTION:

		/*
                                                                        
                                                                  
                       
    
                                                                  
   */
		expected_count = package->ret_info3.count;
		if (count < expected_count) {
			goto package_too_small;
		}

		/*                                */

		for (i = 0; i < count; i++) {
			if (i < package->ret_info3.count) {

				/*                                                      */

				status =
				    acpi_ns_check_object_type(data, elements,
							      package->
							      ret_info3.
							      object_type[i],
							      i);
				if (ACPI_FAILURE(status)) {
					return (status);
				}
			} else {
				/*                                         */

				status =
				    acpi_ns_check_object_type(data, elements,
							      package->
							      ret_info3.
							      tail_object_type,
							      i);
				if (ACPI_FAILURE(status)) {
					return (status);
				}
			}
			elements++;
		}
		break;

	case ACPI_PTYPE2_REV_FIXED:

		/*                                         */

		status = acpi_ns_check_object_type(data, elements,
						   ACPI_RTYPE_INTEGER, 0);
		if (ACPI_FAILURE(status)) {
			return (status);
		}

		elements++;
		count--;

		/*                          */

		status =
		    acpi_ns_check_package_list(data, package, elements, count);
		break;

	case ACPI_PTYPE2_PKG_COUNT:

		/*                                                                */

		status = acpi_ns_check_object_type(data, elements,
						   ACPI_RTYPE_INTEGER, 0);
		if (ACPI_FAILURE(status)) {
			return (status);
		}

		/*
                                                                        
                                                          
   */
		expected_count = (u32)(*elements)->integer.value;
		if (expected_count >= count) {
			goto package_too_small;
		}

		count = expected_count;
		elements++;

		/*                          */

		status =
		    acpi_ns_check_package_list(data, package, elements, count);
		break;

	case ACPI_PTYPE2:
	case ACPI_PTYPE2_FIXED:
	case ACPI_PTYPE2_MIN:
	case ACPI_PTYPE2_COUNT:
	case ACPI_PTYPE2_FIX_VAR:

		/*
                                                               
                                     
    
                                                                   
                                                                  
                                                                
                                                               
                                                            
   */
		if (*elements
		    && ((*elements)->common.type != ACPI_TYPE_PACKAGE)) {

			/*                                              */

			status =
			    acpi_ns_wrap_with_package(data, return_object,
						      return_object_ptr);
			if (ACPI_FAILURE(status)) {
				return (status);
			}

			/*                                                          */

			return_object = *return_object_ptr;
			elements = return_object->package.elements;
			count = 1;
		}

		/*                          */

		status =
		    acpi_ns_check_package_list(data, package, elements, count);
		break;

	default:

		/*                                                         */

		ACPI_WARN_PREDEFINED((AE_INFO, data->pathname, data->node_flags,
				      "Invalid internal return type in table entry: %X",
				      package->ret_info.type));

		return (AE_AML_INTERNAL);
	}

	return (status);

      package_too_small:

	/*                                                         */

	ACPI_WARN_PREDEFINED((AE_INFO, data->pathname, data->node_flags,
			      "Return Package is too small - found %u elements, expected %u",
			      count, expected_count));

	return (AE_AML_OPERAND_VALUE);
}

/*                                                                              
  
                                          
  
                                                                      
                                                                             
                                                                              
                                                                         
                                                      
  
                      
  
                                             
  
                                                                              */

static acpi_status
acpi_ns_check_package_list(struct acpi_predefined_data *data,
			   const union acpi_predefined_info *package,
			   union acpi_operand_object **elements, u32 count)
{
	union acpi_operand_object *sub_package;
	union acpi_operand_object **sub_elements;
	acpi_status status;
	u32 expected_count;
	u32 i;
	u32 j;

	/*
                                                   
   
                                                                 
                                                              
                                    
  */
	for (i = 0; i < count; i++) {
		sub_package = *elements;
		sub_elements = sub_package->package.elements;
		data->parent_package = sub_package;

		/*                                         */

		status = acpi_ns_check_object_type(data, &sub_package,
						   ACPI_RTYPE_PACKAGE, i);
		if (ACPI_FAILURE(status)) {
			return (status);
		}

		/*                                                      */

		data->parent_package = sub_package;
		switch (package->ret_info.type) {
		case ACPI_PTYPE2:
		case ACPI_PTYPE2_PKG_COUNT:
		case ACPI_PTYPE2_REV_FIXED:

			/*                                                */

			expected_count =
			    package->ret_info.count1 + package->ret_info.count2;
			if (sub_package->package.count < expected_count) {
				goto package_too_small;
			}

			status =
			    acpi_ns_check_package_elements(data, sub_elements,
							   package->ret_info.
							   object_type1,
							   package->ret_info.
							   count1,
							   package->ret_info.
							   object_type2,
							   package->ret_info.
							   count2, 0);
			if (ACPI_FAILURE(status)) {
				return (status);
			}
			break;

		case ACPI_PTYPE2_FIX_VAR:
			/*
                                                           
                      
    */
			expected_count =
			    package->ret_info.count1 + package->ret_info.count2;
			if (sub_package->package.count < expected_count) {
				goto package_too_small;
			}

			status =
			    acpi_ns_check_package_elements(data, sub_elements,
							   package->ret_info.
							   object_type1,
							   package->ret_info.
							   count1,
							   package->ret_info.
							   object_type2,
							   sub_package->package.
							   count -
							   package->ret_info.
							   count1, 0);
			if (ACPI_FAILURE(status)) {
				return (status);
			}
			break;

		case ACPI_PTYPE2_FIXED:

			/*                                     */

			expected_count = package->ret_info2.count;
			if (sub_package->package.count < expected_count) {
				goto package_too_small;
			}

			/*                                            */

			for (j = 0; j < expected_count; j++) {
				status =
				    acpi_ns_check_object_type(data,
							      &sub_elements[j],
							      package->
							      ret_info2.
							      object_type[j],
							      j);
				if (ACPI_FAILURE(status)) {
					return (status);
				}
			}
			break;

		case ACPI_PTYPE2_MIN:

			/*                                                    */

			expected_count = package->ret_info.count1;
			if (sub_package->package.count < expected_count) {
				goto package_too_small;
			}

			/*                                            */

			status =
			    acpi_ns_check_package_elements(data, sub_elements,
							   package->ret_info.
							   object_type1,
							   sub_package->package.
							   count, 0, 0, 0);
			if (ACPI_FAILURE(status)) {
				return (status);
			}
			break;

		case ACPI_PTYPE2_COUNT:

			/*
                                                                 
                                                     
    */
			status = acpi_ns_check_object_type(data, sub_elements,
							   ACPI_RTYPE_INTEGER,
							   0);
			if (ACPI_FAILURE(status)) {
				return (status);
			}

			/*
                                                            
                                     
    */
			expected_count = (u32)(*sub_elements)->integer.value;
			if (sub_package->package.count < expected_count) {
				goto package_too_small;
			}
			if (sub_package->package.count <
			    package->ret_info.count1) {
				expected_count = package->ret_info.count1;
				goto package_too_small;
			}
			if (expected_count == 0) {
				/*
                                                                   
                                                               
                                                                 
                                     
     */
				expected_count = sub_package->package.count;
				(*sub_elements)->integer.value = expected_count;
			}

			/*                                            */

			status =
			    acpi_ns_check_package_elements(data,
							   (sub_elements + 1),
							   package->ret_info.
							   object_type1,
							   (expected_count - 1),
							   0, 0, 1);
			if (ACPI_FAILURE(status)) {
				return (status);
			}
			break;

		default:	/*                                                   */

			return (AE_AML_INTERNAL);
		}

		elements++;
	}

	return (AE_OK);

      package_too_small:

	/*                                                 */

	ACPI_WARN_PREDEFINED((AE_INFO, data->pathname, data->node_flags,
			      "Return Sub-Package[%u] is too small - found %u elements, expected %u",
			      i, sub_package->package.count, expected_count));

	return (AE_AML_OPERAND_VALUE);
}

/*                                                                              
  
                                              
  
                                                                      
                                                                       
                                                             
                                                       
                                                              
                                                        
                                                                      
  
                      
  
                                                                              
                                                                          
  
                                                                              */

static acpi_status
acpi_ns_check_package_elements(struct acpi_predefined_data *data,
			       union acpi_operand_object **elements,
			       u8 type1,
			       u32 count1,
			       u8 type2, u32 count2, u32 start_index)
{
	union acpi_operand_object **this_element = elements;
	acpi_status status;
	u32 i;

	/*
                                                                  
                                                                   
                                              
  */
	for (i = 0; i < count1; i++) {
		status = acpi_ns_check_object_type(data, this_element,
						   type1, i + start_index);
		if (ACPI_FAILURE(status)) {
			return (status);
		}
		this_element++;
	}

	for (i = 0; i < count2; i++) {
		status = acpi_ns_check_object_type(data, this_element,
						   type2,
						   (i + count1 + start_index));
		if (ACPI_FAILURE(status)) {
			return (status);
		}
		this_element++;
	}

	return (AE_OK);
}
