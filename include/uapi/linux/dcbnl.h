/*
 * Copyright (c) 2008-2011, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307 USA.
 *
 * Author: Lucy Liu <lucy.liu@intel.com>
 */

#ifndef __LINUX_DCBNL_H__
#define __LINUX_DCBNL_H__

#include <linux/types.h>

/*                                    */
#define IEEE_8021QAZ_MAX_TCS	8

#define IEEE_8021QAZ_TSA_STRICT		0
#define IEEE_8021QAZ_TSA_CB_SHAPER	1
#define IEEE_8021QAZ_TSA_ETS		2
#define IEEE_8021QAZ_TSA_VENDOR		255

/*                                                             
  
                                                 
                                                        
                                                    
                                                      
                                                      
                                                          
                                                                      
                                                                         
                                                                          
                                                                              
  
                                                                          
                                
  
       
                                    
                    
                        
                                    
                 
                      
 */
struct ieee_ets {
	__u8	willing;
	__u8	ets_cap;
	__u8	cbs;
	__u8	tc_tx_bw[IEEE_8021QAZ_MAX_TCS];
	__u8	tc_rx_bw[IEEE_8021QAZ_MAX_TCS];
	__u8	tc_tsa[IEEE_8021QAZ_MAX_TCS];
	__u8	prio_tc[IEEE_8021QAZ_MAX_TCS];
	__u8	tc_reco_bw[IEEE_8021QAZ_MAX_TCS];
	__u8	tc_reco_tsa[IEEE_8021QAZ_MAX_TCS];
	__u8	reco_prio_tc[IEEE_8021QAZ_MAX_TCS];
};

/*                                                                      
                  
                                                                          
                               
  
                                                                
 */
struct ieee_maxrate {
	__u64	tc_maxrate[IEEE_8021QAZ_MAX_TCS];
};

/*                                                             
  
                                                                        
                                                 
                                                         
                                        
                                                                       
                   
                                          
                                                 
 */
struct ieee_pfc {
	__u8	pfc_cap;
	__u8	pfc_en;
	__u8	mbc;
	__u16	delay;
	__u64	requests[IEEE_8021QAZ_MAX_TCS];
	__u64	indications[IEEE_8021QAZ_MAX_TCS];
};

/*                               */
#define CEE_DCBX_MAX_PGS	8
#define CEE_DCBX_MAX_PRIO	8

/* 
                                                    
  
                                      
                                  
                                       
                                                      
                                                       
                                                       
 */
struct cee_pg {
	__u8    willing;
	__u8    error;
	__u8    pg_en;
	__u8    tcs_supported;
	__u8    pg_bw[CEE_DCBX_MAX_PGS];
	__u8    prio_pg[CEE_DCBX_MAX_PGS];
};

/* 
                                          
  
                                       
                                   
                                                         
                                                      
 */
struct cee_pfc {
	__u8    willing;
	__u8    error;
	__u8    pfc_en;
	__u8    tcs_supported;
};

/*                                    */
#define IEEE_8021QAZ_APP_SEL_ETHERTYPE	1
#define IEEE_8021QAZ_APP_SEL_STREAM	2
#define IEEE_8021QAZ_APP_SEL_DGRAM	3
#define IEEE_8021QAZ_APP_SEL_ANY	4

/*                                                                   
                                                                      
                       
  
                                      
                                        
                                                        
  
       
                         
             
              
                                            
                                            
                                                        
               
 */
struct dcb_app {
	__u8	selector;
	__u8	priority;
	__u16	protocol;
};

/* 
                                                                      
  
                                            
                                        
  
                                                                      
                                                    
 */
struct dcb_peer_app_info {
	__u8	willing;
	__u8	error;
};

struct dcbmsg {
	__u8               dcb_family;
	__u8               cmd;
	__u16              dcb_pad;
};

/* 
                                               
  
                                                          
                                                          
                                                      
                                                                      
                                                                  
                                                                      
                                                                  
                                                                     
                                                                 
                                                               
                                                                         
                                                                  
                                                            
                                                                          
                                                      
                                                                    
                                                                    
                                                        
                                                        
                                                     
                                                     
                                                
                                                
                                             
                                                         
                                                     
                                                        
 */
enum dcbnl_commands {
	DCB_CMD_UNDEFINED,

	DCB_CMD_GSTATE,
	DCB_CMD_SSTATE,

	DCB_CMD_PGTX_GCFG,
	DCB_CMD_PGTX_SCFG,
	DCB_CMD_PGRX_GCFG,
	DCB_CMD_PGRX_SCFG,

	DCB_CMD_PFC_GCFG,
	DCB_CMD_PFC_SCFG,

	DCB_CMD_SET_ALL,

	DCB_CMD_GPERM_HWADDR,

	DCB_CMD_GCAP,

	DCB_CMD_GNUMTCS,
	DCB_CMD_SNUMTCS,

	DCB_CMD_PFC_GSTATE,
	DCB_CMD_PFC_SSTATE,

	DCB_CMD_BCN_GCFG,
	DCB_CMD_BCN_SCFG,

	DCB_CMD_GAPP,
	DCB_CMD_SAPP,

	DCB_CMD_IEEE_SET,
	DCB_CMD_IEEE_GET,

	DCB_CMD_GDCBX,
	DCB_CMD_SDCBX,

	DCB_CMD_GFEATCFG,
	DCB_CMD_SFEATCFG,

	DCB_CMD_CEE_GET,
	DCB_CMD_IEEE_DEL,

	__DCB_CMD_ENUM_MAX,
	DCB_CMD_MAX = __DCB_CMD_ENUM_MAX - 1,
};

/* 
                                                      
  
                                                             
                                                                         
                                                              
                                                                  
                                                                      
                                                                               
                                                              
                                                                        
                                                                         
                                                             
                                                                     
                                                                             
                                                                  
                                                                   
                                                      
                                                           
 */
enum dcbnl_attrs {
	DCB_ATTR_UNDEFINED,

	DCB_ATTR_IFNAME,
	DCB_ATTR_STATE,
	DCB_ATTR_PFC_STATE,
	DCB_ATTR_PFC_CFG,
	DCB_ATTR_NUM_TC,
	DCB_ATTR_PG_CFG,
	DCB_ATTR_SET_ALL,
	DCB_ATTR_PERM_HWADDR,
	DCB_ATTR_CAP,
	DCB_ATTR_NUMTCS,
	DCB_ATTR_BCN,
	DCB_ATTR_APP,

	/*                     */
	DCB_ATTR_IEEE,

	DCB_ATTR_DCBX,
	DCB_ATTR_FEATCFG,

	/*                       */
	DCB_ATTR_CEE,

	__DCB_ATTR_ENUM_MAX,
	DCB_ATTR_MAX = __DCB_ATTR_ENUM_MAX - 1,
};

/* 
                                                     
  
                                     
                                                   
                                                   
                                                         
                                                             
                                                             
                                                   
 */
enum ieee_attrs {
	DCB_ATTR_IEEE_UNSPEC,
	DCB_ATTR_IEEE_ETS,
	DCB_ATTR_IEEE_PFC,
	DCB_ATTR_IEEE_APP_TABLE,
	DCB_ATTR_IEEE_PEER_ETS,
	DCB_ATTR_IEEE_PEER_PFC,
	DCB_ATTR_IEEE_PEER_APP,
	DCB_ATTR_IEEE_MAXRATE,
	__DCB_ATTR_IEEE_MAX
};
#define DCB_ATTR_IEEE_MAX (__DCB_ATTR_IEEE_MAX - 1)

enum ieee_attrs_app {
	DCB_ATTR_IEEE_APP_UNSPEC,
	DCB_ATTR_IEEE_APP,
	__DCB_ATTR_IEEE_APP_MAX
};
#define DCB_ATTR_IEEE_APP_MAX (__DCB_ATTR_IEEE_APP_MAX - 1)

/* 
                                            
  
                                    
                                                          
                                                            
                                                        
                                                               
                                                               
                                                          
                                                                  
                                                             
  
                                                                 
 */
enum cee_attrs {
	DCB_ATTR_CEE_UNSPEC,
	DCB_ATTR_CEE_PEER_PG,
	DCB_ATTR_CEE_PEER_PFC,
	DCB_ATTR_CEE_PEER_APP_TABLE,
	DCB_ATTR_CEE_TX_PG,
	DCB_ATTR_CEE_RX_PG,
	DCB_ATTR_CEE_PFC,
	DCB_ATTR_CEE_APP_TABLE,
	DCB_ATTR_CEE_FEAT,
	__DCB_ATTR_CEE_MAX
};
#define DCB_ATTR_CEE_MAX (__DCB_ATTR_CEE_MAX - 1)

enum peer_app_attr {
	DCB_ATTR_CEE_PEER_APP_UNSPEC,
	DCB_ATTR_CEE_PEER_APP_INFO,
	DCB_ATTR_CEE_PEER_APP,
	__DCB_ATTR_CEE_PEER_APP_MAX
};
#define DCB_ATTR_CEE_PEER_APP_MAX (__DCB_ATTR_CEE_PEER_APP_MAX - 1)

enum cee_attrs_app {
	DCB_ATTR_CEE_APP_UNSPEC,
	DCB_ATTR_CEE_APP,
	__DCB_ATTR_CEE_APP_MAX
};
#define DCB_ATTR_CEE_APP_MAX (__DCB_ATTR_CEE_APP_MAX - 1)

/* 
                                                                              
  
                                                                    
                                                                               
                                                                               
                                                                               
                                                                               
                                                                               
                                                                               
                                                                               
                                                                               
                                                                   
                                                                            
  
 */
enum dcbnl_pfc_up_attrs {
	DCB_PFC_UP_ATTR_UNDEFINED,

	DCB_PFC_UP_ATTR_0,
	DCB_PFC_UP_ATTR_1,
	DCB_PFC_UP_ATTR_2,
	DCB_PFC_UP_ATTR_3,
	DCB_PFC_UP_ATTR_4,
	DCB_PFC_UP_ATTR_5,
	DCB_PFC_UP_ATTR_6,
	DCB_PFC_UP_ATTR_7,
	DCB_PFC_UP_ATTR_ALL,

	__DCB_PFC_UP_ATTR_ENUM_MAX,
	DCB_PFC_UP_ATTR_MAX = __DCB_PFC_UP_ATTR_ENUM_MAX - 1,
};

/* 
                                                      
  
                                                                
                                                                               
                                                                               
                                                                               
                                                                               
                                                                               
                                                                               
                                                                               
                                                                               
                                                                  
                                                                 
                                                                                
                                                                                
                                                                                
                                                                                
                                                                                
                                                                                
                                                                                
                                                                                
                                                                     
                                                                  
  
 */
enum dcbnl_pg_attrs {
	DCB_PG_ATTR_UNDEFINED,

	DCB_PG_ATTR_TC_0,
	DCB_PG_ATTR_TC_1,
	DCB_PG_ATTR_TC_2,
	DCB_PG_ATTR_TC_3,
	DCB_PG_ATTR_TC_4,
	DCB_PG_ATTR_TC_5,
	DCB_PG_ATTR_TC_6,
	DCB_PG_ATTR_TC_7,
	DCB_PG_ATTR_TC_MAX,
	DCB_PG_ATTR_TC_ALL,

	DCB_PG_ATTR_BW_ID_0,
	DCB_PG_ATTR_BW_ID_1,
	DCB_PG_ATTR_BW_ID_2,
	DCB_PG_ATTR_BW_ID_3,
	DCB_PG_ATTR_BW_ID_4,
	DCB_PG_ATTR_BW_ID_5,
	DCB_PG_ATTR_BW_ID_6,
	DCB_PG_ATTR_BW_ID_7,
	DCB_PG_ATTR_BW_ID_MAX,
	DCB_PG_ATTR_BW_ID_ALL,

	__DCB_PG_ATTR_ENUM_MAX,
	DCB_PG_ATTR_MAX = __DCB_PG_ATTR_ENUM_MAX - 1,
};

/* 
                                                     
  
                                                                      
                                                                                
                                                  
                                                                             
                                                                           
                                                            
                                                                   
                                           
                                                   
                                                  
                                                                                
                                                                         
                                                                        
                                                                          
                                                                  
  
 */
enum dcbnl_tc_attrs {
	DCB_TC_ATTR_PARAM_UNDEFINED,

	DCB_TC_ATTR_PARAM_PGID,
	DCB_TC_ATTR_PARAM_UP_MAPPING,
	DCB_TC_ATTR_PARAM_STRICT_PRIO,
	DCB_TC_ATTR_PARAM_BW_PCT,
	DCB_TC_ATTR_PARAM_ALL,

	__DCB_TC_ATTR_PARAM_ENUM_MAX,
	DCB_TC_ATTR_PARAM_MAX = __DCB_TC_ATTR_PARAM_ENUM_MAX - 1,
};

/* 
                                                   
  
                                                                 
                                                          
                                                             
                                                                    
                                                                 
                                                      
                                                                    
                                                                      
                                                                              
                                                                     
                                                                              
                                                                              
                                                                    
                                                                   
                                           
                                                           
  
 */
enum dcbnl_cap_attrs {
	DCB_CAP_ATTR_UNDEFINED,
	DCB_CAP_ATTR_ALL,
	DCB_CAP_ATTR_PG,
	DCB_CAP_ATTR_PFC,
	DCB_CAP_ATTR_UP2TC,
	DCB_CAP_ATTR_PG_TCS,
	DCB_CAP_ATTR_PFC_TCS,
	DCB_CAP_ATTR_GSP,
	DCB_CAP_ATTR_BCN,
	DCB_CAP_ATTR_DCBX,

	__DCB_CAP_ATTR_ENUM_MAX,
	DCB_CAP_ATTR_MAX = __DCB_CAP_ATTR_ENUM_MAX - 1,
};

/* 
                        
  
                                                                            
                                                                           
                                                
  
                                                                               
                                               
                                                                     
                                                   
                                                                           
                                                       
  
                                                                          
                                                          
  
                                                                           
                                                            
  
                                                                         
                                                                     
                                                                             
                                                   
  
 */
#define DCB_CAP_DCBX_HOST		0x01
#define DCB_CAP_DCBX_LLD_MANAGED	0x02
#define DCB_CAP_DCBX_VER_CEE		0x04
#define DCB_CAP_DCBX_VER_IEEE		0x08
#define DCB_CAP_DCBX_STATIC		0x10

/* 
                                                      
  
                                                                    
                                                                
                                                                   
                                                
                                                                     
                                                               
 */
enum dcbnl_numtcs_attrs {
	DCB_NUMTCS_ATTR_UNDEFINED,
	DCB_NUMTCS_ATTR_ALL,
	DCB_NUMTCS_ATTR_PG,
	DCB_NUMTCS_ATTR_PFC,

	__DCB_NUMTCS_ATTR_ENUM_MAX,
	DCB_NUMTCS_ATTR_MAX = __DCB_NUMTCS_ATTR_ENUM_MAX - 1,
};

enum dcbnl_bcn_attrs{
	DCB_BCN_ATTR_UNDEFINED = 0,

	DCB_BCN_ATTR_RP_0,
	DCB_BCN_ATTR_RP_1,
	DCB_BCN_ATTR_RP_2,
	DCB_BCN_ATTR_RP_3,
	DCB_BCN_ATTR_RP_4,
	DCB_BCN_ATTR_RP_5,
	DCB_BCN_ATTR_RP_6,
	DCB_BCN_ATTR_RP_7,
	DCB_BCN_ATTR_RP_ALL,

	DCB_BCN_ATTR_BCNA_0,
	DCB_BCN_ATTR_BCNA_1,
	DCB_BCN_ATTR_ALPHA,
	DCB_BCN_ATTR_BETA,
	DCB_BCN_ATTR_GD,
	DCB_BCN_ATTR_GI,
	DCB_BCN_ATTR_TMAX,
	DCB_BCN_ATTR_TD,
	DCB_BCN_ATTR_RMIN,
	DCB_BCN_ATTR_W,
	DCB_BCN_ATTR_RD,
	DCB_BCN_ATTR_RU,
	DCB_BCN_ATTR_WRTT,
	DCB_BCN_ATTR_RI,
	DCB_BCN_ATTR_C,
	DCB_BCN_ATTR_ALL,

	__DCB_BCN_ATTR_ENUM_MAX,
	DCB_BCN_ATTR_MAX = __DCB_BCN_ATTR_ENUM_MAX - 1,
};

/* 
                                                              
  
                                                                            
  
 */
enum dcb_general_attr_values {
	DCB_ATTR_VALUE_UNDEFINED = 0xff
};

#define DCB_APP_IDTYPE_ETHTYPE	0x00
#define DCB_APP_IDTYPE_PORTNUM	0x01
enum dcbnl_app_attrs {
	DCB_APP_ATTR_UNDEFINED,

	DCB_APP_ATTR_IDTYPE,
	DCB_APP_ATTR_ID,
	DCB_APP_ATTR_PRIORITY,

	__DCB_APP_ATTR_ENUM_MAX,
	DCB_APP_ATTR_MAX = __DCB_APP_ATTR_ENUM_MAX - 1,
};

/* 
                                                           
  
                                                                     
                                                                          
                                                                         
                                                                   
                                               
                                                                          
  
 */
#define DCB_FEATCFG_ERROR	0x01	/*                             */
#define DCB_FEATCFG_ENABLE	0x02	/*                */
#define DCB_FEATCFG_WILLING	0x04	/*                    */
#define DCB_FEATCFG_ADVERTISE	0x08	/*                   */
enum dcbnl_featcfg_attrs {
	DCB_FEATCFG_ATTR_UNDEFINED,
	DCB_FEATCFG_ATTR_ALL,
	DCB_FEATCFG_ATTR_PG,
	DCB_FEATCFG_ATTR_PFC,
	DCB_FEATCFG_ATTR_APP,

	__DCB_FEATCFG_ATTR_ENUM_MAX,
	DCB_FEATCFG_ATTR_MAX = __DCB_FEATCFG_ATTR_ENUM_MAX - 1,
};

#endif /*                   */
