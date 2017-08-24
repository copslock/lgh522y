/*

SiI8348 Linux Driver

Copyright (C) 2013 Silicon Image, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation version 2.
This program is distributed AS-IS WITHOUT ANY WARRANTY of any
kind, whether express or implied; INCLUDING without the implied warranty
of MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE or NON-INFRINGEMENT.  See 
the GNU General Public License for more details at http://www.gnu.org/licenses/gpl-2.0.html.             

*/

#if !defined(MHL_LINUX_TX_H)
#define MHL_LINUX_TX_H

#include "sii_hal.h"
/*
              
  
 */
/*                          */
#define	MHL_TX_EVENT_NONE		    0x00

/*                              */
#define	MHL_TX_EVENT_DISCONNECTION	0x01

/*                                     */
#define	MHL_TX_EVENT_CONNECTION		0x02

/*                          */
#define	MHL_TX_EVENT_RCP_RECEIVED	0x04

/*                          */
#define	MHL_TX_EVENT_RCPK_RECEIVED	0x05

/*                          */
#define	MHL_TX_EVENT_RCPE_RECEIVED	0x06

/*                            */
#define	MHL_TX_EVENT_UCP_RECEIVED	0x07

/*                          */
#define	MHL_TX_EVENT_UCPK_RECEIVED	0x08

/*                          */
#define	MHL_TX_EVENT_UCPE_RECEIVED	0x09

/*                           */
#define MHL_TX_EVENT_SPAD_RECEIVED	0x0A

/*                                     */
#define	MHL_TX_EVENT_POW_BIT_CHG	0x0B

/*                                                  */
#define MHL_TX_EVENT_RAP_RECEIVED	0x0C

//     
#define MHL_TX_EVENT_SMB_DATA		0x40
#define MHL_TX_EVENT_HPD_CLEAR 		0x41
#define MHL_TX_EVENT_HPD_GOT 		0x42
#define MHL_TX_EVENT_DEV_CAP_UPDATE 0x43
#define MHL_TX_EVENT_EDID_UPDATE 	0x44
#define MHL_TX_EVENT_EDID_DONE 		0x45

#define ADOPTER_ID_SIZE			2
#define MAX_SCRATCH_PAD_TRANSFER_SIZE	16
#define SCRATCH_PAD_SIZE		64

#define SCRATCHPAD_SIZE			16
typedef union {
	MHL2_video_format_data_t	videoFormatData;
	uint8_t				asBytes[SCRATCHPAD_SIZE];
} scratch_pad_u;


struct timer_obj {
	struct list_head		list_link;
	struct work_struct		work_item;
	struct hrtimer			hr_timer;
	struct mhl_dev_context		*dev_context;
	uint8_t				flags;
#define TIMER_OBJ_FLAG_WORK_IP		0x01
#define TIMER_OBJ_FLAG_DEL_REQ		0x02
	void				*callback_param;
	void (*timer_callback_handler)(void *callback_param);
};

/*                                                          */
#define MSEC_TO_NSEC(x)			(x * 1000000UL)

union misc_flags_u {
	struct {
		unsigned	scratchpad_busy						:1;
		unsigned	req_wrt_pending						:1;
		unsigned	write_burst_pending					:1;
		unsigned	rcp_ready						:1;

		unsigned	have_complete_devcap					:1;
		unsigned	sent_dcap_rdy						:1;
		unsigned	sent_path_en						:1;
		unsigned	rap_content_on 						:1;

		unsigned	mhl_hpd							:1;
		unsigned	mhl_rsen						:1;
		unsigned	edid_loop_active					:1;
		unsigned	cbus_abort_delay_active					:1;

		unsigned	reserved						:20;
	} flags;
	uint32_t		as_uint32;
};

/*
                                                 
                                  
 */
struct interrupt_info {
	uint16_t				flags;
/*                                                      */
#define DRV_INTR_FLAG_MSC_DONE		0x0001	/*                   */
#define DRV_INTR_FLAG_MSC_RECVD		0x0002	/*                      */
#define DRV_INTR_FLAG_MSC_NAK		0x0004	/*                           */
#define DRV_INTR_FLAG_WRITE_STAT	0x0008	/*                         */
#define DRV_INTR_FLAG_SET_INT		0x0010	/*                          */
#define DRV_INTR_FLAG_WRITE_BURST	0x0020	/*                      */
#define DRV_INTR_FLAG_HPD_CHANGE	0x0040	/*                             */
#define DRV_INTR_FLAG_CONNECT		0x0080	/*                            */
#define DRV_INTR_FLAG_DISCONNECT	0x0100	/*                     */
#define DRV_INTR_FLAG_CBUS_ABORT	0x0200	/*                            
                 */

	void					*edid_parser_context;
	uint8_t					msc_done_data;
	uint8_t					hpd_status;		/*                           */
	uint8_t					write_stat[2];		/*                          */
	uint8_t					msc_msg[2];		/*                           */
	uint8_t					int_msg[2];		/*                               */
};

#define NUM_CBUS_EVENT_QUEUE_EVENTS	 50

#define MHL_DEV_CONTEXT_SIGNATURE	( ('M' << 24) | ('H' << 16) | ('L' << 8) | ' ')

struct mhl_dev_context {
	uint32_t				signature;	/*                          
                       */
	struct	mhl_drv_info const 		*drv_info;
	struct	i2c_client			*client;
	struct	cdev				mhl_cdev;
	struct	device				*mhl_dev;
	struct	interrupt_info			intr_info;
	void					*edid_parser_context;
	bool					edid_parse_done;		//                                                       
	u8					dev_flags;
#define DEV_FLAG_SHUTDOWN			0x01	/*                         */
#define DEV_FLAG_COMM_MODE			0x02	/*                                      */

	u16					mhl_flags;				/*                     */
#define MHL_STATE_FLAG_CONNECTED		0x0001	/*               
                                       */
#ifndef RCP_INPUTDEV_SUPPORT
#define MHL_STATE_FLAG_RCP_SENT			0x0002	/*                         
                                */
#define MHL_STATE_FLAG_RCP_RECEIVED		0x0004	/*                         
                                        */
#define MHL_STATE_FLAG_RCP_ACK			0x0008	/*                           
                                 */
#define MHL_STATE_FLAG_RCP_NAK			0x0010	/*                           
                                 */
#endif
#define MHL_STATE_FLAG_UCP_SENT			0x0020	/*                         
                                */
#define MHL_STATE_FLAG_UCP_RECEIVED		0x0040	/*                         
                                        */
#define MHL_STATE_FLAG_UCP_ACK			0x0080	/*                           
                                 */
#define MHL_STATE_FLAG_UCP_NAK			0x0100	/*                           
                                 */
#define MHL_STATE_FLAG_SPAD_SENT		0x0200	/*                    
                                   */
#define MHL_STATE_APPLICATION_RAP_BUSY		0x0400	/*                           
                                   
                              */

	u8				dev_cap_offset;
	u8				rap_sub_command;
	u8				rcp_key_code;
	u8				rcp_err_code;
	u8				rcp_send_status;
	u8				ucp_key_code;
	u8				ucp_err_code;
	u8				ucp_send_status;
	u8				pending_event;		/*                               */
	u8				pending_event_data;	/*                          */
	u8				spad_offset;
	u8				spad_xfer_length;
	u8				spad_send_status;
	u8				debug_i2c_address;
	u8				debug_i2c_offset;
	u8				debug_i2c_xfer_length;

#ifdef MEDIA_DATA_TUNNEL_SUPPORT
	struct mdt_inputdevs		mdt_devs;
#endif

#ifdef RCP_INPUTDEV_SUPPORT
	u8				error_key;
	struct input_dev		*rcp_input_dev;
#endif
	struct semaphore		isr_lock;	/*                                        
                                                
                                     */

	u8				status_0;	/*                                      */
	u8				status_1;
	bool				msc_msg_arrived;
	u8				msc_msg_sub_command;
	u8				msc_msg_data;

	u8				msc_msg_last_data;
	u8				msc_save_rcp_key_code;
	u8				msc_save_ucp_key_code;

	u8				link_mode;	/*                                    */
	bool				mhl_connection_event;
	u8				mhl_connected;
	struct workqueue_struct		*timer_work_queue;
	struct list_head		timer_list;
	struct list_head		cbus_queue;
	struct list_head		cbus_free_list;
	struct cbus_req			cbus_req_entries[NUM_CBUS_EVENT_QUEUE_EVENTS];
	struct cbus_req			*current_cbus_req;
	void				*cbus_abort_timer;
	void				*cbus_dpi_timer;
	MHLDevCap_u			dev_cap_cache;
	MHLDevCap_u			dev_cap_cache_new;
	u8				dev_cap_cache_index;
	u8				preferred_clk_mode;
	bool				scratch_pad_read_done;
	scratch_pad_u			incoming_scratch_pad;
	scratch_pad_u			outgoing_scratch_pad;

	union misc_flags_u		misc_flags;

	uint8_t				numEdidExtensions;

	void				*drv_context;	/*                                                              */
};

#define PACKED_PIXEL_AVAILABLE(dev_context)						\
	((MHL_DEV_VID_LINK_SUPP_PPIXEL &						\
	 dev_context->dev_cap_cache.devcap_cache[DEVCAP_OFFSET_VID_LINK_MODE]) &&	\
	 (MHL_DEV_VID_LINK_SUPP_PPIXEL & DEVCAP_VAL_VID_LINK_MODE))

enum scratch_pad_status {
	SCRATCHPAD_FAIL= -4
	,SCRATCHPAD_BAD_PARAM = -3
	,SCRATCHPAD_NOT_SUPPORTED = -2
	,SCRATCHPAD_BUSY = -1
	,SCRATCHPAD_SUCCESS = 0
};

struct drv_hw_context;

struct mhl_drv_info {
	int drv_context_size;
	struct {
		uint8_t	major	: 4;
		uint8_t	minor	: 4;
	} mhl_version_support;

	/*                                                                       */
	int (* mhl_device_initialize) (struct drv_hw_context *hw_context);
	void (*mhl_device_isr) (struct drv_hw_context *hw_context, struct interrupt_info *intr_info);
	int (* mhl_device_dbg_i2c_reg_xfer) (void *dev_context, u8 page, u8 offset, u8 count, bool rw_flag, u8 *buffer);
	void (*mhl_start_video) (struct drv_hw_context *hw_context);
};


/*                                                            */
int mhl_tx_init(struct mhl_drv_info const *drv_info, struct i2c_client *client);

int mhl_tx_remove(struct i2c_client *client);

void mhl_event_notify(struct mhl_dev_context *dev_context, u32 event, u32 event_param, void *data);

struct mhl_dev_context *get_mhl_device_context(void *context);

uint8_t calculate_generic_checksum(uint8_t *info_frame_data, uint8_t checksum, uint8_t length);

void *si_mhl_tx_get_drv_context(void *dev_context);

int mhl_tx_create_timer(void *context,
		void (*callback_handler)(void *callback_param),
		void *callback_param,
		void **timer_handle);

int mhl_tx_delete_timer(void *context, void *timer_handle);

int mhl_tx_start_timer(void *context, void *timer_handle, uint32_t time_msec);

int mhl_tx_stop_timer(void *context, void *timer_handle);

void si_mhl_tx_request_first_edid_block(struct mhl_dev_context *dev_context);
void si_mhl_tx_handle_atomic_hw_edid_read_complete(edid_3d_data_p mhl_edid_3d_data,struct cbus_req *req);

/*                                                 */
uint8_t si_mhl_tx_get_peer_dev_cap_entry(struct mhl_dev_context *dev_context, uint8_t index, uint8_t *data);

enum scratch_pad_status si_get_scratch_pad_vector(
		struct mhl_dev_context *dev_context,
		uint8_t offset,uint8_t length,
		uint8_t *data);
#endif /*                             */
