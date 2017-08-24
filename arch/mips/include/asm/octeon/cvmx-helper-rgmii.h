/***********************license start***************
 * Author: Cavium Networks
 *
 * Contact: support@caviumnetworks.com
 * This file is part of the OCTEON SDK
 *
 * Copyright (c) 2003-2008 Cavium Networks
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, Version 2, as
 * published by the Free Software Foundation.
 *
 * This file is distributed in the hope that it will be useful, but
 * AS-IS and WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, TITLE, or
 * NONINFRINGEMENT.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this file; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 * or visit http://www.gnu.org/licenses/.
 *
 * This file may also be available under a different license from Cavium.
 * Contact Cavium Networks for more information
 ***********************license end**************************************/

/* 
        
  
                                                              
                  
  
 */
#ifndef __CVMX_HELPER_RGMII_H__
#define __CVMX_HELPER_RGMII_H__

/* 
                                                     
  
                                 
  
                                                
 */
extern int __cvmx_helper_rgmii_probe(int interface);
#define __cvmx_helper_rgmii_enumerate __cvmx_helper_rgmii_probe

/* 
                                                                 
                                                               
                                       
  
                                    
 */
extern void cvmx_helper_rgmii_internal_loopback(int port);

/* 
                                                            
                                                      
  
                                                  
  
                          
 */
extern int __cvmx_helper_rgmii_enable(int interface);

/* 
                                                          
                                                              
                                                             
                                           
  
                                   
  
                     
 */
extern cvmx_helper_link_info_t __cvmx_helper_rgmii_link_get(int ipd_port);

/* 
                                                               
                                                                 
                                                                  
                                                        
                                       
  
                                        
                                 
  
                                               
 */
extern int __cvmx_helper_rgmii_link_set(int ipd_port,
					cvmx_helper_link_info_t link_info);

/* 
                                                                            
                                                                              
                                                           
  
                                       
                    
                                             
                    
                                             
  
                                                
 */
extern int __cvmx_helper_rgmii_configure_loopback(int ipd_port,
						  int enable_internal,
						  int enable_external);

#endif
