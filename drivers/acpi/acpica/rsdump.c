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
#include "acresrc.h"

#define _COMPONENT          ACPI_RESOURCES
ACPI_MODULE_NAME("rsdump")
#if defined(ACPI_DEBUG_OUTPUT) || defined(ACPI_DEBUGGER)
/*                  */
static void acpi_rs_out_string(char *title, char *value);

static void acpi_rs_out_integer8(char *title, u8 value);

static void acpi_rs_out_integer16(char *title, u16 value);

static void acpi_rs_out_integer32(char *title, u32 value);

static void acpi_rs_out_integer64(char *title, u64 value);

static void acpi_rs_out_title(char *title);

static void acpi_rs_dump_byte_list(u16 length, u8 *data);

static void acpi_rs_dump_word_list(u16 length, u16 *data);

static void acpi_rs_dump_dword_list(u8 length, u32 *data);

static void acpi_rs_dump_short_byte_list(u8 length, u8 *data);

static void
acpi_rs_dump_resource_source(struct acpi_resource_source *resource_source);

static void acpi_rs_dump_address_common(union acpi_resource_data *resource);

static void
acpi_rs_dump_descriptor(void *resource, struct acpi_rsdump_info *table);

/*                                                                              
  
                                       
  
                                                                    
                                                                        
  
                    
  
                                                                       
  
                                                                              */

static void
acpi_rs_dump_descriptor(void *resource, struct acpi_rsdump_info *table)
{
	u8 *target = NULL;
	u8 *previous_target;
	char *name;
	u8 count;

	/*                                                                      */

	count = table->offset;

	while (count) {
		previous_target = target;
		target = ACPI_ADD_PTR(u8, resource, table->offset);
		name = table->name;

		switch (table->opcode) {
		case ACPI_RSD_TITLE:
			/*
                             
    */
			if (table->name) {
				acpi_os_printf("%s Resource\n", name);
			}
			break;

			/*         */

		case ACPI_RSD_LITERAL:
			acpi_rs_out_string(name,
					   ACPI_CAST_PTR(char, table->pointer));
			break;

		case ACPI_RSD_STRING:
			acpi_rs_out_string(name, ACPI_CAST_PTR(char, target));
			break;

			/*                            */

		case ACPI_RSD_UINT8:
			if (table->pointer) {
				acpi_rs_out_string(name, ACPI_CAST_PTR(char,
								       table->
								       pointer
								       [*target]));
			} else {
				acpi_rs_out_integer8(name, ACPI_GET8(target));
			}
			break;

		case ACPI_RSD_UINT16:
			acpi_rs_out_integer16(name, ACPI_GET16(target));
			break;

		case ACPI_RSD_UINT32:
			acpi_rs_out_integer32(name, ACPI_GET32(target));
			break;

		case ACPI_RSD_UINT64:
			acpi_rs_out_integer64(name, ACPI_GET64(target));
			break;

			/*                                        */

		case ACPI_RSD_1BITFLAG:
			acpi_rs_out_string(name, ACPI_CAST_PTR(char,
							       table->
							       pointer[*target &
								       0x01]));
			break;

		case ACPI_RSD_2BITFLAG:
			acpi_rs_out_string(name, ACPI_CAST_PTR(char,
							       table->
							       pointer[*target &
								       0x03]));
			break;

		case ACPI_RSD_3BITFLAG:
			acpi_rs_out_string(name, ACPI_CAST_PTR(char,
							       table->
							       pointer[*target &
								       0x07]));
			break;

		case ACPI_RSD_SHORTLIST:
			/*
                                                                    
                                                                     
    */
			if (previous_target) {
				acpi_rs_out_title(name);
				acpi_rs_dump_short_byte_list(*previous_target,
							     target);
			}
			break;

		case ACPI_RSD_SHORTLISTX:
			/*
                                                               
                                                                     
    */
			if (previous_target) {
				acpi_rs_out_title(name);
				acpi_rs_dump_short_byte_list(*previous_target,
							     *
							     (ACPI_CAST_INDIRECT_PTR
							      (u8, target)));
			}
			break;

		case ACPI_RSD_LONGLIST:
			/*
                                             
                                                                     
    */
			if (previous_target) {
				acpi_rs_dump_byte_list(ACPI_GET16
						       (previous_target),
						       target);
			}
			break;

		case ACPI_RSD_DWORDLIST:
			/*
                                                 
                                                                     
    */
			if (previous_target) {
				acpi_rs_dump_dword_list(*previous_target,
							ACPI_CAST_PTR(u32,
								      target));
			}
			break;

		case ACPI_RSD_WORDLIST:
			/*
                                  
                                                                     
    */
			if (previous_target) {
				acpi_rs_dump_word_list(*previous_target,
						       *(ACPI_CAST_INDIRECT_PTR
							 (u16, target)));
			}
			break;

		case ACPI_RSD_ADDRESS:
			/*
                                            
    */
			acpi_rs_dump_address_common(ACPI_CAST_PTR
						    (union acpi_resource_data,
						     target));
			break;

		case ACPI_RSD_SOURCE:
			/*
                                                    
    */
			acpi_rs_dump_resource_source(ACPI_CAST_PTR
						     (struct
								   acpi_resource_source,
								   target));
			break;

		default:
			acpi_os_printf("**** Invalid table opcode [%X] ****\n",
				       table->opcode);
			return;
		}

		table++;
		count--;
	}
}

/*                                                                              
  
                                            
  
                                                                         
  
                    
  
                                                                               
                                                    
  
                                                                              */

static void
acpi_rs_dump_resource_source(struct acpi_resource_source *resource_source)
{
	ACPI_FUNCTION_ENTRY();

	if (resource_source->index == 0xFF) {
		return;
	}

	acpi_rs_out_integer8("Resource Source Index", resource_source->index);

	acpi_rs_out_string("Resource Source",
			   resource_source->string_ptr ?
			   resource_source->string_ptr : "[Not Specified]");
}

/*                                                                              
  
                                           
  
                                                                            
  
                    
  
                                                                       
                           
  
                                                                              */

static void acpi_rs_dump_address_common(union acpi_resource_data *resource)
{
	ACPI_FUNCTION_ENTRY();

	/*                                */

	switch (resource->address.resource_type) {
	case ACPI_MEMORY_RANGE:

		acpi_rs_dump_descriptor(resource, acpi_rs_dump_memory_flags);
		break;

	case ACPI_IO_RANGE:

		acpi_rs_dump_descriptor(resource, acpi_rs_dump_io_flags);
		break;

	case ACPI_BUS_NUMBER_RANGE:

		acpi_rs_out_string("Resource Type", "Bus Number Range");
		break;

	default:

		acpi_rs_out_integer8("Resource Type",
				     (u8) resource->address.resource_type);
		break;
	}

	/*                          */

	acpi_rs_dump_descriptor(resource, acpi_rs_dump_general_flags);
}

/*                                                                              
  
                                          
  
                                                                           
  
                    
  
                                                                     
  
                                                                              */

void acpi_rs_dump_resource_list(struct acpi_resource *resource_list)
{
	u32 count = 0;
	u32 type;

	ACPI_FUNCTION_ENTRY();

	/*                               */

	if (!ACPI_IS_DEBUG_ENABLED(ACPI_LV_RESOURCES, _COMPONENT)) {
		return;
	}

	/*                                                                  */

	do {
		acpi_os_printf("\n[%02X] ", count);
		count++;

		/*                               */

		type = resource_list->type;
		if (type > ACPI_RESOURCE_TYPE_MAX) {
			acpi_os_printf
			    ("Invalid descriptor type (%X) in resource list\n",
			     resource_list->type);
			return;
		}

		/*                                                                  */

		if (!resource_list->length) {
			acpi_os_printf
			    ("Invalid zero length descriptor in resource list\n");
			return;
		}

		/*                              */

		if (type == ACPI_RESOURCE_TYPE_SERIAL_BUS) {
			acpi_rs_dump_descriptor(&resource_list->data,
						acpi_gbl_dump_serial_bus_dispatch
						[resource_list->data.
						 common_serial_bus.type]);
		} else {
			acpi_rs_dump_descriptor(&resource_list->data,
						acpi_gbl_dump_resource_dispatch
						[type]);
		}

		/*                                      */

		resource_list = ACPI_NEXT_RESOURCE(resource_list);

		/*                                         */

	} while (type != ACPI_RESOURCE_TYPE_END_TAG);
}

/*                                                                              
  
                                     
  
                                                                       
  
                    
  
                                       
  
                                                                              */

void acpi_rs_dump_irq_list(u8 * route_table)
{
	struct acpi_pci_routing_table *prt_element;
	u8 count;

	ACPI_FUNCTION_ENTRY();

	/*                               */

	if (!ACPI_IS_DEBUG_ENABLED(ACPI_LV_RESOURCES, _COMPONENT)) {
		return;
	}

	prt_element = ACPI_CAST_PTR(struct acpi_pci_routing_table, route_table);

	/*                                                      */

	for (count = 0; prt_element->length; count++) {
		acpi_os_printf("\n[%02X] PCI IRQ Routing Table Package\n",
			       count);
		acpi_rs_dump_descriptor(prt_element, acpi_rs_dump_prt);

		prt_element = ACPI_ADD_PTR(struct acpi_pci_routing_table,
					   prt_element, prt_element->length);
	}
}

/*                                                                              
  
                            
  
                                                        
                                                         
  
                    
  
                                                                         
                                                    
  
                                                                              */

static void acpi_rs_out_string(char *title, char *value)
{
	acpi_os_printf("%27s : %s", title, value);
	if (!*value) {
		acpi_os_printf("[NULL NAMESTRING]");
	}
	acpi_os_printf("\n");
}

static void acpi_rs_out_integer8(char *title, u8 value)
{
	acpi_os_printf("%27s : %2.2X\n", title, value);
}

static void acpi_rs_out_integer16(char *title, u16 value)
{
	acpi_os_printf("%27s : %4.4X\n", title, value);
}

static void acpi_rs_out_integer32(char *title, u32 value)
{
	acpi_os_printf("%27s : %8.8X\n", title, value);
}

static void acpi_rs_out_integer64(char *title, u64 value)
{
	acpi_os_printf("%27s : %8.8X%8.8X\n", title, ACPI_FORMAT_UINT64(value));
}

static void acpi_rs_out_title(char *title)
{
	acpi_os_printf("%27s : ", title);
}

/*                                                                              
  
                                 
  
                                                            
                                               
  
                    
  
                                                                 
  
                                                                              */

static void acpi_rs_dump_byte_list(u16 length, u8 * data)
{
	u8 i;

	for (i = 0; i < length; i++) {
		acpi_os_printf("%25s%2.2X : %2.2X\n", "Byte", i, data[i]);
	}
}

static void acpi_rs_dump_short_byte_list(u8 length, u8 * data)
{
	u8 i;

	for (i = 0; i < length; i++) {
		acpi_os_printf("%X ", data[i]);
	}
	acpi_os_printf("\n");
}

static void acpi_rs_dump_dword_list(u8 length, u32 * data)
{
	u8 i;

	for (i = 0; i < length; i++) {
		acpi_os_printf("%25s%2.2X : %8.8X\n", "Dword", i, data[i]);
	}
}

static void acpi_rs_dump_word_list(u16 length, u16 *data)
{
	u16 i;

	for (i = 0; i < length; i++) {
		acpi_os_printf("%25s%2.2X : %4.4X\n", "Word", i, data[i]);
	}
}

#endif
