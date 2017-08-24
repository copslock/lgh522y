/*
 * Copyright (c) 2005-2010 Brocade Communications Systems, Inc.
 * All rights reserved
 * www.brocade.com
 *
 * Linux driver for Brocade Fibre Channel Host Bus Adapter.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (GPL) Version 2 as
 * published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#ifndef __BFI_MS_H__
#define __BFI_MS_H__

#include "bfi.h"
#include "bfa_fc.h"
#include "bfa_defs_svc.h"

#pragma pack(1)

enum bfi_iocfc_h2i_msgs {
	BFI_IOCFC_H2I_CFG_REQ		= 1,
	BFI_IOCFC_H2I_SET_INTR_REQ	= 2,
	BFI_IOCFC_H2I_UPDATEQ_REQ	= 3,
	BFI_IOCFC_H2I_FAA_QUERY_REQ	= 4,
	BFI_IOCFC_H2I_ADDR_REQ		= 5,
};

enum bfi_iocfc_i2h_msgs {
	BFI_IOCFC_I2H_CFG_REPLY		= BFA_I2HM(1),
	BFI_IOCFC_I2H_UPDATEQ_RSP	= BFA_I2HM(3),
	BFI_IOCFC_I2H_FAA_QUERY_RSP	= BFA_I2HM(4),
	BFI_IOCFC_I2H_ADDR_MSG		= BFA_I2HM(5),
};

struct bfi_iocfc_cfg_s {
	u8	num_cqs;	/*                           */
	u8	 sense_buf_len;	/*                        */
	u16	rsvd_1;
	u32	endian_sig;	/*                               */
	u8	rsvd_2;
	u8	single_msix_vec;
	u8	rsvd[2];
	__be16	num_ioim_reqs;
	__be16	num_fwtio_reqs;


	/*
                                                                
                          
  */
	union bfi_addr_u  req_cq_ba[BFI_IOC_MAX_CQS];
	union bfi_addr_u  req_shadow_ci[BFI_IOC_MAX_CQS];
	__be16    req_cq_elems[BFI_IOC_MAX_CQS];
	union bfi_addr_u  rsp_cq_ba[BFI_IOC_MAX_CQS];
	union bfi_addr_u  rsp_shadow_pi[BFI_IOC_MAX_CQS];
	__be16    rsp_cq_elems[BFI_IOC_MAX_CQS];

	union bfi_addr_u  stats_addr;	/*                               */
	union bfi_addr_u  cfgrsp_addr;	/*                               */
	union bfi_addr_u  ioim_snsbase[BFI_IOIM_SNSBUF_SEGS];
					/*                                  */
	struct bfa_iocfc_intr_attr_s intr_attr; /*                           */
};

/*
                                                                             
                                                    
 */
struct bfi_iocfc_bootwwns {
	wwn_t		wwn[BFA_BOOT_BOOTLUN_MAX];
	u8		nwwns;
	u8		rsvd[7];
};

/* 
                                             
 */
struct bfi_iocfc_qreg_s {
	u32	cpe_q_ci_off[BFI_IOC_MAX_CQS];
	u32	cpe_q_pi_off[BFI_IOC_MAX_CQS];
	u32	cpe_qctl_off[BFI_IOC_MAX_CQS];
	u32	rme_q_ci_off[BFI_IOC_MAX_CQS];
	u32	rme_q_pi_off[BFI_IOC_MAX_CQS];
	u32	rme_qctl_off[BFI_IOC_MAX_CQS];
	u8	hw_qid[BFI_IOC_MAX_CQS];
};

struct bfi_iocfc_cfgrsp_s {
	struct bfa_iocfc_fwcfg_s	fwcfg;
	struct bfa_iocfc_intr_attr_s	intr_attr;
	struct bfi_iocfc_bootwwns	bootwwns;
	struct bfi_pbc_s		pbc_cfg;
	struct bfi_iocfc_qreg_s		qreg;
};

/*
                                
 */
struct bfi_iocfc_cfg_req_s {
	struct bfi_mhdr_s      mh;
	union bfi_addr_u      ioc_cfg_dma_addr;
};


/*
                                  
 */
struct bfi_iocfc_cfg_reply_s {
	struct bfi_mhdr_s  mh;		/*                      */
	u8	 cfg_success;	/*                      */
	u8	 lpu_bm;		/*                             */
	u8	 rsvd[2];
};


/*
                                     
 */
struct bfi_iocfc_set_intr_req_s {
	struct bfi_mhdr_s mh;		/*                     */
	u8		coalesce;	/*                         */
	u8		rsvd[3];
	__be16	delay;		/*                        */
	__be16	latency;	/*                         */
};


/*
                                    
 */
struct bfi_iocfc_updateq_req_s {
	struct bfi_mhdr_s mh;		/*                     */
	u32 reqq_ba;		/*                  */
	u32 rspq_ba;		/*                  */
	u32 reqq_sci;		/*                  */
	u32 rspq_spi;		/*                  */
};


/*
                                    
 */
struct bfi_iocfc_updateq_rsp_s {
	struct bfi_mhdr_s mh;		/*                    */
	u8	status;			/*                  */
	u8	rsvd[3];
};


/*
               
 */
union bfi_iocfc_h2i_msg_u {
	struct bfi_mhdr_s		mh;
	struct bfi_iocfc_cfg_req_s	cfg_req;
	struct bfi_iocfc_updateq_req_s updateq_req;
	u32 mboxmsg[BFI_IOC_MSGSZ];
};


/*
               
 */
union bfi_iocfc_i2h_msg_u {
	struct bfi_mhdr_s		mh;
	struct bfi_iocfc_cfg_reply_s	cfg_reply;
	struct bfi_iocfc_updateq_rsp_s updateq_rsp;
	u32 mboxmsg[BFI_IOC_MSGSZ];
};

/*
                                                                     
 */
struct bfi_faa_en_dis_s {
	struct bfi_mhdr_s mh;	/*                      */
};

struct bfi_faa_addr_msg_s {
	struct  bfi_mhdr_s mh;	/*                   */
	u8	rsvd[4];
	wwn_t	pwwn;		/*                      */
	wwn_t	nwwn;		/*                      */
};

/*
                                      
 */
struct bfi_faa_query_s {
	struct bfi_mhdr_s mh;	/*                      */
	u8	faa_status;	/*                      */
	u8	addr_source;	/*                      */
	u8	rsvd[2];
	wwn_t	faa;		/*                      */
};

/*
                                                                      
 */
struct bfi_faa_en_dis_rsp_s {
	struct bfi_mhdr_s mh;	/*                      */
	u8	status;		/*                      */
	u8	rsvd[3];
};

/*
                                      
 */
#define bfi_faa_query_rsp_t struct bfi_faa_query_s

enum bfi_fcport_h2i {
	BFI_FCPORT_H2I_ENABLE_REQ		= (1),
	BFI_FCPORT_H2I_DISABLE_REQ		= (2),
	BFI_FCPORT_H2I_SET_SVC_PARAMS_REQ	= (3),
	BFI_FCPORT_H2I_STATS_GET_REQ		= (4),
	BFI_FCPORT_H2I_STATS_CLEAR_REQ		= (5),
};


enum bfi_fcport_i2h {
	BFI_FCPORT_I2H_ENABLE_RSP		= BFA_I2HM(1),
	BFI_FCPORT_I2H_DISABLE_RSP		= BFA_I2HM(2),
	BFI_FCPORT_I2H_SET_SVC_PARAMS_RSP	= BFA_I2HM(3),
	BFI_FCPORT_I2H_STATS_GET_RSP		= BFA_I2HM(4),
	BFI_FCPORT_I2H_STATS_CLEAR_RSP		= BFA_I2HM(5),
	BFI_FCPORT_I2H_EVENT			= BFA_I2HM(6),
	BFI_FCPORT_I2H_TRUNK_SCN		= BFA_I2HM(7),
	BFI_FCPORT_I2H_ENABLE_AEN		= BFA_I2HM(8),
	BFI_FCPORT_I2H_DISABLE_AEN		= BFA_I2HM(9),
};


/*
                   
 */
struct bfi_fcport_req_s {
	struct bfi_mhdr_s  mh;		/*                   */
	u32	   msgtag;	/*                        */
};

/*
                   
 */
struct bfi_fcport_rsp_s {
	struct bfi_mhdr_s  mh;		/*                         */
	u8		   status;	/*                          */
	u8		   rsvd[3];
	struct	bfa_port_cfg_s port_cfg;/*                    */
	u32	msgtag;			/*                  */
};

/*
                            
 */
struct bfi_fcport_enable_req_s {
	struct bfi_mhdr_s  mh;		/*                   */
	u32	   rsvd1;
	wwn_t		   nwwn;	/*                                */
	wwn_t		   pwwn;	/*                                */
	struct bfa_port_cfg_s port_cfg; /*                         */
	union bfi_addr_u   stats_dma_addr; /*                            */
	u32	   msgtag;	/*                        */
	u8	use_flash_cfg;	/*                         */
	u8	rsvd2[3];
};

/*
                                    
 */
struct bfi_fcport_set_svc_params_req_s {
	struct bfi_mhdr_s  mh;		/*             */
	__be16	   tx_bbcredit;	/*             */
	u8	bb_scn;		/*                          */
	u8	rsvd;
};

/*
                       
 */
struct bfi_fcport_event_s {
	struct bfi_mhdr_s	mh;	/*                    */
	struct bfa_port_link_s	link_state;
};

/*
                           
 */
struct bfi_fcport_trunk_link_s {
	wwn_t			trunk_wwn;
	u8			fctl;		/*                       */
	u8			state;		/*                        */
	u8			speed;		/*                  */
	u8			rsvd;
	__be32		deskew;
};

#define BFI_FCPORT_MAX_LINKS	2
struct bfi_fcport_trunk_scn_s {
	struct bfi_mhdr_s	mh;
	u8			trunk_state;	/*                   */
	u8			trunk_speed;	/*                  */
	u8			rsvd_a[2];
	struct bfi_fcport_trunk_link_s tlink[BFI_FCPORT_MAX_LINKS];
};

/*
                     
 */
union bfi_fcport_h2i_msg_u {
	struct bfi_mhdr_s			*mhdr;
	struct bfi_fcport_enable_req_s		*penable;
	struct bfi_fcport_req_s			*pdisable;
	struct bfi_fcport_set_svc_params_req_s	*psetsvcparams;
	struct bfi_fcport_req_s			*pstatsget;
	struct bfi_fcport_req_s			*pstatsclear;
};

/*
                     
 */
union bfi_fcport_i2h_msg_u {
	struct bfi_msg_s			*msg;
	struct bfi_fcport_rsp_s			*penable_rsp;
	struct bfi_fcport_rsp_s			*pdisable_rsp;
	struct bfi_fcport_rsp_s			*psetsvcparams_rsp;
	struct bfi_fcport_rsp_s			*pstatsget_rsp;
	struct bfi_fcport_rsp_s			*pstatsclear_rsp;
	struct bfi_fcport_event_s		*event;
	struct bfi_fcport_trunk_scn_s		*trunk_scn;
};

enum bfi_fcxp_h2i {
	BFI_FCXP_H2I_SEND_REQ = 1,
};

enum bfi_fcxp_i2h {
	BFI_FCXP_I2H_SEND_RSP = BFA_I2HM(1),
};

#define BFA_FCXP_MAX_SGES	2

/*
                              
 */
struct bfi_fcxp_send_req_s {
	struct bfi_mhdr_s  mh;		/*                         */
	__be16	fcxp_tag;	/*                          */
	__be16	max_frmsz;	/*                          */
	__be16	vf_id;		/*                             */
	u16	rport_fw_hndl;	/*                                 */
	u8	 class;		/*                                */
	u8	 rsp_timeout;	/*                                 */
	u8	 cts;		/*                         */
	u8	 lp_fwtag;	/*                  */
	struct fchs_s	fchs;	/*                                 */
	__be32	req_len;	/*                             */
	__be32	rsp_maxlen;	/*                                 */
	struct bfi_alen_s req_alen;	/*                */
	struct bfi_alen_s rsp_alen;	/*                 */
};

/*
                               
 */
struct bfi_fcxp_send_rsp_s {
	struct bfi_mhdr_s  mh;		/*                         */
	__be16	fcxp_tag;	/*                        */
	u8	 req_status;	/*                      */
	u8	 rsvd;
	__be32	rsp_len;	/*                             */
	__be32	residue_len;	/*                               */
	struct fchs_s	fchs;	/*                                 */
};

enum bfi_uf_h2i {
	BFI_UF_H2I_BUF_POST = 1,
};

enum bfi_uf_i2h {
	BFI_UF_I2H_FRM_RCVD = BFA_I2HM(1),
};

#define BFA_UF_MAX_SGES	2

struct bfi_uf_buf_post_s {
	struct bfi_mhdr_s  mh;		/*                     */
	u16	buf_tag;	/*               */
	__be16	buf_len;	/*                      */
	struct bfi_alen_s alen;	/*                         */
};

struct bfi_uf_frm_rcvd_s {
	struct bfi_mhdr_s  mh;		/*                     */
	u16	buf_tag;	/*               */
	u16	rsvd;
	u16	frm_len;	/*                        */
	u16	xfr_len;	/*                     */
};

enum bfi_lps_h2i_msgs {
	BFI_LPS_H2I_LOGIN_REQ	= 1,
	BFI_LPS_H2I_LOGOUT_REQ	= 2,
	BFI_LPS_H2I_N2N_PID_REQ = 3,
};

enum bfi_lps_i2h_msgs {
	BFI_LPS_I2H_LOGIN_RSP	= BFA_I2HM(1),
	BFI_LPS_I2H_LOGOUT_RSP	= BFA_I2HM(2),
	BFI_LPS_I2H_CVL_EVENT	= BFA_I2HM(3),
};

struct bfi_lps_login_req_s {
	struct bfi_mhdr_s  mh;		/*                     */
	u8		bfa_tag;
	u8		alpa;
	__be16		pdu_size;
	wwn_t		pwwn;
	wwn_t		nwwn;
	u8		fdisc;
	u8		auth_en;
	u8		lps_role;
	u8		bb_scn;
	u32		vvl_flag;
};

struct bfi_lps_login_rsp_s {
	struct bfi_mhdr_s  mh;		/*                     */
	u8		fw_tag;
	u8		status;
	u8		lsrjt_rsn;
	u8		lsrjt_expl;
	wwn_t		port_name;
	wwn_t		node_name;
	__be16		bb_credit;
	u8		f_port;
	u8		npiv_en;
	u32	lp_pid:24;
	u32	auth_req:8;
	mac_t		lp_mac;
	mac_t		fcf_mac;
	u8		ext_status;
	u8		brcd_switch;	/*                               */
	u8		bb_scn;		/*                        */
	u8		bfa_tag;
};

struct bfi_lps_logout_req_s {
	struct bfi_mhdr_s  mh;		/*                     */
	u8		fw_tag;
	u8		rsvd[3];
	wwn_t		port_name;
};

struct bfi_lps_logout_rsp_s {
	struct bfi_mhdr_s  mh;		/*                     */
	u8		bfa_tag;
	u8		status;
	u8		rsvd[2];
};

struct bfi_lps_cvl_event_s {
	struct bfi_mhdr_s  mh;		/*                     */
	u8		bfa_tag;
	u8		rsvd[3];
};

struct bfi_lps_n2n_pid_req_s {
	struct bfi_mhdr_s	mh;	/*                     */
	u8	fw_tag;
	u32	lp_pid:24;
};

union bfi_lps_h2i_msg_u {
	struct bfi_mhdr_s		*msg;
	struct bfi_lps_login_req_s	*login_req;
	struct bfi_lps_logout_req_s	*logout_req;
	struct bfi_lps_n2n_pid_req_s	*n2n_pid_req;
};

union bfi_lps_i2h_msg_u {
	struct bfi_msg_s		*msg;
	struct bfi_lps_login_rsp_s	*login_rsp;
	struct bfi_lps_logout_rsp_s	*logout_rsp;
	struct bfi_lps_cvl_event_s	*cvl_event;
};

enum bfi_rport_h2i_msgs {
	BFI_RPORT_H2I_CREATE_REQ = 1,
	BFI_RPORT_H2I_DELETE_REQ = 2,
	BFI_RPORT_H2I_SET_SPEED_REQ  = 3,
};

enum bfi_rport_i2h_msgs {
	BFI_RPORT_I2H_CREATE_RSP = BFA_I2HM(1),
	BFI_RPORT_I2H_DELETE_RSP = BFA_I2HM(2),
	BFI_RPORT_I2H_QOS_SCN    = BFA_I2HM(3),
	BFI_RPORT_I2H_LIP_SCN_ONLINE =	BFA_I2HM(4),
	BFI_RPORT_I2H_LIP_SCN_OFFLINE = BFA_I2HM(5),
	BFI_RPORT_I2H_NO_DEV	= BFA_I2HM(6),
};

struct bfi_rport_create_req_s {
	struct bfi_mhdr_s  mh;		/*                     */
	u16	bfa_handle;	/*                     */
	__be16	max_frmsz;	/*                    */
	u32	pid:24,	/*                  */
		lp_fwtag:8;	/*                  */
	u32	local_pid:24,	/*                 */
		cisc:8;
	u8	fc_class;	/*                       */
	u8	vf_en;		/*                        */
	u16	vf_id;		/*                     */
};

struct bfi_rport_create_rsp_s {
	struct bfi_mhdr_s  mh;		/*                     */
	u8		status;		/*                        */
	u8		rsvd[3];
	u16	bfa_handle;	/*                     */
	u16	fw_handle;	/*                        */
	struct bfa_rport_qos_attr_s qos_attr;  /*                */
};

struct bfa_rport_speed_req_s {
	struct bfi_mhdr_s  mh;		/*                     */
	u16	fw_handle;	/*                        */
	u8		speed;		/*                         */
	u8		rsvd;
};

struct bfi_rport_delete_req_s {
	struct bfi_mhdr_s  mh;		/*                     */
	u16	fw_handle;	/*                        */
	u16	rsvd;
};

struct bfi_rport_delete_rsp_s {
	struct bfi_mhdr_s  mh;		/*                     */
	u16	bfa_handle;	/*                     */
	u8		status;		/*                        */
	u8		rsvd;
};

struct bfi_rport_qos_scn_s {
	struct bfi_mhdr_s  mh;		/*                     */
	u16	bfa_handle;	/*                     */
	u16	rsvd;
	struct bfa_rport_qos_attr_s old_qos_attr;  /*                    */
	struct bfa_rport_qos_attr_s new_qos_attr;  /*                    */
};

struct bfi_rport_lip_scn_s {
	struct bfi_mhdr_s  mh;		/*                     */
	u16	bfa_handle;	/*                     */
	u8		status;		/*                     */
	u8		rsvd;
	struct bfa_fcport_loop_info_s	loop_info;
};

union bfi_rport_h2i_msg_u {
	struct bfi_msg_s		*msg;
	struct bfi_rport_create_req_s	*create_req;
	struct bfi_rport_delete_req_s	*delete_req;
	struct bfi_rport_speed_req_s	*speed_req;
};

union bfi_rport_i2h_msg_u {
	struct bfi_msg_s		*msg;
	struct bfi_rport_create_rsp_s	*create_rsp;
	struct bfi_rport_delete_rsp_s	*delete_rsp;
	struct bfi_rport_qos_scn_s	*qos_scn_evt;
	struct bfi_rport_lip_scn_s	*lip_scn;
};

/*
                                              
 */

enum bfi_itn_h2i {
	BFI_ITN_H2I_CREATE_REQ = 1,	/*                     */
	BFI_ITN_H2I_DELETE_REQ = 2,	/*                     */
};

enum bfi_itn_i2h {
	BFI_ITN_I2H_CREATE_RSP = BFA_I2HM(1),
	BFI_ITN_I2H_DELETE_RSP = BFA_I2HM(2),
	BFI_ITN_I2H_SLER_EVENT = BFA_I2HM(3),
};

struct bfi_itn_create_req_s {
	struct bfi_mhdr_s  mh;		/*                      */
	u16	fw_handle;	/*                        */
	u8	class;		/*                    */
	u8	seq_rec;	/*                             */
	u8	msg_no;		/*                      */
	u8	role;
};

struct bfi_itn_create_rsp_s {
	struct bfi_mhdr_s  mh;		/*                      */
	u16	bfa_handle;	/*                        */
	u8	status;		/*                       */
	u8	seq_id;		/*                      */
};

struct bfi_itn_delete_req_s {
	struct bfi_mhdr_s  mh;		/*                      */
	u16	fw_handle;	/*                     */
	u8	seq_id;		/*                      */
	u8	rsvd;
};

struct bfi_itn_delete_rsp_s {
	struct bfi_mhdr_s  mh;		/*                      */
	u16	bfa_handle;	/*                        */
	u8	status;		/*                       */
	u8	seq_id;		/*                      */
};

struct bfi_itn_sler_event_s {
	struct bfi_mhdr_s  mh;		/*                      */
	u16	bfa_handle;	/*                        */
	u16	rsvd;
};

union bfi_itn_h2i_msg_u {
	struct bfi_itn_create_req_s *create_req;
	struct bfi_itn_delete_req_s *delete_req;
	struct bfi_msg_s	*msg;
};

union bfi_itn_i2h_msg_u {
	struct bfi_itn_create_rsp_s *create_rsp;
	struct bfi_itn_delete_rsp_s *delete_rsp;
	struct bfi_itn_sler_event_s *sler_event;
	struct bfi_msg_s	*msg;
};

/*
                                       
 */

enum bfi_ioim_h2i {
	BFI_IOIM_H2I_IOABORT_REQ = 1,	/*                    */
	BFI_IOIM_H2I_IOCLEANUP_REQ = 2,	/*                      */
};

enum bfi_ioim_i2h {
	BFI_IOIM_I2H_IO_RSP = BFA_I2HM(1),	/*                      */
	BFI_IOIM_I2H_IOABORT_RSP = BFA_I2HM(2),	/*             */
};

/*
                      
 */
struct bfi_ioim_dif_s {
	u32	dif_info[4];
};

/*
                           
  
        
                                          
                                                              
             
  
 */
struct bfi_ioim_req_s {
	struct bfi_mhdr_s  mh;		/*                      */
	__be16	io_tag;		/*             */
	u16	rport_hdl;	/*                              */
	struct fcp_cmnd_s	cmnd;	/*                  */

	/*
                                                               
                                                                        
  */
	struct bfi_sge_s	sges[BFI_SGE_INLINE_MAX];
	u8	io_timeout;
	u8	dif_en;
	u8	rsvd_a[2];
	struct bfi_ioim_dif_s  dif;
};

/*
                                                                   
                                                                     
                  
  
                                                         
                        
                             
  
                                                         
                          
  
                                                                 
                     
                                     
  
                                                      
                          
                                     
  
                                                                
                          
                                     
  
                                                               
                                  
                                      
                                        
                                   
  
                                                                   
                                               
                                           
                                 
                                   
  
                                                              
                                             
                                            
                                           
                                             
                                           
                                
  
                                                             
                                             
                                            
                                             
                 
                              
  
                                                              
                             
                              
  
                                                               
                                      
                                           
                               
                                   
  
                                                              
                                             
                              
  
                                                              
                                         
                                
  
                                                         
                
                              
 */
enum bfi_ioim_status {
	BFI_IOIM_STS_OK = 0,
	BFI_IOIM_STS_HOST_ABORTED = 1,
	BFI_IOIM_STS_ABORTED = 2,
	BFI_IOIM_STS_TIMEDOUT = 3,
	BFI_IOIM_STS_RES_FREE = 4,
	BFI_IOIM_STS_SQER_NEEDED = 5,
	BFI_IOIM_STS_PROTO_ERR = 6,
	BFI_IOIM_STS_UTAG = 7,
	BFI_IOIM_STS_PATHTOV = 8,
};

/*
                       
 */
struct bfi_ioim_rsp_s {
	struct bfi_mhdr_s	mh;	/*                     */
	__be16	io_tag;		/*                     */
	u16	bfa_rport_hndl;	/*                         */
	u8	io_status;	/*                        */
	u8	reuse_io_tag;	/*                       */
	u16	abort_tag;	/*                         */
	u8		scsi_status;	/*                           */
	u8		sns_len;	/*                      */
	u8		resid_flags;	/*                     */
	u8		rsvd_a;
	__be32	residue;	/*                              */
	u32	rsvd_b[3];
};

struct bfi_ioim_abort_req_s {
	struct bfi_mhdr_s  mh;	/*                     */
	__be16	io_tag;	/*          */
	u16	abort_tag;	/*                     */
};

/*
                                                            
 */

enum bfi_tskim_h2i {
	BFI_TSKIM_H2I_TM_REQ	= 1, /*                    */
	BFI_TSKIM_H2I_ABORT_REQ = 2, /*                    */
};

enum bfi_tskim_i2h {
	BFI_TSKIM_I2H_TM_RSP = BFA_I2HM(1),
};

struct bfi_tskim_req_s {
	struct bfi_mhdr_s  mh;	/*                    */
	__be16	tsk_tag;	/*                      */
	u16	itn_fhdl;	/*                      */
	struct 	scsi_lun lun;	/*            */
	u8	tm_flags;	/*                       */
	u8	t_secs;	/*                           */
	u8	rsvd[2];
};

struct bfi_tskim_abortreq_s {
	struct bfi_mhdr_s  mh;	/*                    */
	__be16	tsk_tag;	/*                      */
	u16	rsvd;
};

enum bfi_tskim_status {
	/*
                                                  
                           
  */
	BFI_TSKIM_STS_OK	= 0,
	BFI_TSKIM_STS_NOT_SUPP = 4,
	BFI_TSKIM_STS_FAILED	= 5,

	/*
                  
  */
	BFI_TSKIM_STS_TIMEOUT  = 10,	/*                      */
	BFI_TSKIM_STS_ABORTED  = 11,	/*                          */
	BFI_TSKIM_STS_UTAG     = 12,	/*                          */
};

struct bfi_tskim_rsp_s {
	struct bfi_mhdr_s  mh;		/*                      */
	__be16	tsk_tag;	/*                       */
	u8	tsk_status;	/*                        */
	u8	rsvd;
};

#pragma pack()

/*
                                    
 */
enum {
	BFI_MSIX_CPE_QMIN_CB = 0,
	BFI_MSIX_CPE_QMAX_CB = 7,
	BFI_MSIX_RME_QMIN_CB = 8,
	BFI_MSIX_RME_QMAX_CB = 15,
	BFI_MSIX_CB_MAX = 22,
};

/*
                                       
 */
enum {
	BFI_MSIX_LPU_ERR_CT = 0,
	BFI_MSIX_CPE_QMIN_CT = 1,
	BFI_MSIX_CPE_QMAX_CT = 4,
	BFI_MSIX_RME_QMIN_CT = 5,
	BFI_MSIX_RME_QMAX_CT = 8,
	BFI_MSIX_CT_MAX = 9,
};

#endif /*              */
