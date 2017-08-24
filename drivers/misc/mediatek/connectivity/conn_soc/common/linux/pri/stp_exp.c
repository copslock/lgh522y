/*
* Copyright (C) 2011-2014 MediaTek Inc.
* 
* This program is free software: you can redistribute it and/or modify it under the terms of the 
* GNU General Public License version 2 as published by the Free Software Foundation.
* 
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with this program.
* If not, see <http://www.gnu.org/licenses/>.
*/

/*                                                                              
                                                      
                                                                                
*/

#include <linux/types.h>
#include <linux/major.h>
#include <linux/errno.h>
#include <linux/signal.h>
#include <linux/fcntl.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/tty.h>
#include <linux/timer.h>
#include <linux/ctype.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/bitops.h>
#include <linux/audit.h>
#include <linux/file.h>
#include <linux/module.h>

#include <linux/spinlock.h>
#include <linux/delay.h> /*          */

#include <asm/uaccess.h>

#ifdef CONFIG_MTK_WCN_ARM64
#include <system.h>
#else
#include <asm/system.h>
#endif

#include "osal_typedef.h"
#include "stp_core.h"
#include "stp_exp.h"

/*                                                                              
                                            
                                                                                
*/


/*                                                                              
                                             
                                                                                
*/


/*                                                                              
                                              
                                                                                
*/
/*                                                                              
                                               
                                                                                
*/
static MTK_WCN_STP_IF_TX stp_uart_if_tx = NULL;
static MTK_WCN_STP_IF_TX stp_sdio_if_tx = NULL;
static MTK_WCN_STP_IF_TX stp_btif_if_tx = NULL;
static ENUM_STP_TX_IF_TYPE g_stp_if_type = STP_MAX_IF_TX;
static MTK_WCN_STP_IF_RX stp_if_rx = NULL;
static MTK_WCN_STP_EVENT_CB event_callback_tbl[MTKSTP_MAX_TASK_NUM] = {0x0};
static MTK_WCN_STP_EVENT_CB tx_event_callback_tbl[MTKSTP_MAX_TASK_NUM] = {0x0};

/*                                                                             
                                                             
                                                                               
*/

/*                                                                              
                                            
                                                                                
*/

INT32 mtk_wcn_sys_if_rx(UINT8 *data, INT32 size)
{
    if(stp_if_rx == 0x0)
    {
        return (-1);
    }
    else
    {
        (*stp_if_rx)(data, size);
        return 0;
    }
}

static INT32 mtk_wcn_sys_if_tx (
    const UINT8 *data,
    const UINT32 size,
    UINT32 *written_size
    )
{

    if (STP_UART_IF_TX == g_stp_if_type) {
        return stp_uart_if_tx != NULL ? (*stp_uart_if_tx)(data, size, written_size) : -1;
    }
    else if (STP_SDIO_IF_TX == g_stp_if_type) {
        return stp_sdio_if_tx != NULL ? (*stp_sdio_if_tx)(data, size, written_size) : -1;
    }
	else if (STP_BTIF_IF_TX == g_stp_if_type) {
        return stp_btif_if_tx != NULL ? (*stp_btif_if_tx)(data, size, written_size) : -1;
    }
    else {
        /*                                    */ /*                                      */
        return (-1);
    }
}

static INT32 mtk_wcn_sys_event_set(UINT8 function_type)
{
    if((function_type < MTKSTP_MAX_TASK_NUM) && (event_callback_tbl[function_type] != 0x0))
    {
        (*event_callback_tbl[function_type])();
    }
    else {
        /*                       */
        printk(KERN_INFO "[%s] STP set event fail. It seems the function is not active.\n", __func__);
    }

    return 0;
}

static INT32 mtk_wcn_sys_event_tx_resume(UINT8 winspace)
{
    int type = 0;

    for(type = 0 ;  type < MTKSTP_MAX_TASK_NUM ; type ++ )
    {
        if(tx_event_callback_tbl[type])
        {
            tx_event_callback_tbl[type]();
        }
    }

    return 0;
}

static INT32 mtk_wcn_sys_check_function_status(UINT8 type, UINT8 op){

    /*                                                             */
    if(!(type >= 0 && type < MTKSTP_MAX_TASK_NUM))
    {
        return STATUS_FUNCTION_INVALID;
    }

    if(op == OP_FUNCTION_ACTIVE)
    {
        if(event_callback_tbl[type] != 0x0)
        {
            return STATUS_FUNCTION_ACTIVE;
        }
        else
        {
            return STATUS_FUNCTION_INACTIVE;
        }
    }
    /*                                                                               */

    return STATUS_OP_INVALID;
}

#if STP_EXP_HID_API_EXPORT
INT32 _mtk_wcn_stp_register_if_rx(MTK_WCN_STP_IF_RX func)
#else
INT32 mtk_wcn_stp_register_if_rx(MTK_WCN_STP_IF_RX func)
#endif
{
    stp_if_rx = func;

    return 0;    
}

VOID mtk_wcn_stp_set_if_tx_type (
    ENUM_STP_TX_IF_TYPE stp_if_type
    )
{
	char * ifType[] = {
			"UART",
			"SDIO",
			"BTIF",
			"UNKNOWN"
		};
    g_stp_if_type = stp_if_type;
    printk(KERN_INFO "[%s] set STP_IF_TX to %s.\n",
        __FUNCTION__,ifType[stp_if_type]);
}

#if STP_EXP_HID_API_EXPORT
INT32 _mtk_wcn_stp_register_if_tx (
    ENUM_STP_TX_IF_TYPE stp_if,
    MTK_WCN_STP_IF_TX func
    )
#else
INT32 mtk_wcn_stp_register_if_tx (
    ENUM_STP_TX_IF_TYPE stp_if,
    MTK_WCN_STP_IF_TX func
    )
#endif
{
    if (STP_UART_IF_TX == stp_if) 
    {
        stp_uart_if_tx = func;
    }
    else if (STP_SDIO_IF_TX == stp_if) 
    {
        stp_sdio_if_tx = func;
    }
	else if (STP_BTIF_IF_TX == stp_if) 
    {
        stp_btif_if_tx = func;
    }
    else 
    {
        printk(KERN_WARNING "[%s] STP_IF_TX(%d) out of boundary.\n", __FUNCTION__, stp_if);
        return -1;
    }

    return 0;
}

#if STP_EXP_HID_API_EXPORT
INT32 _mtk_wcn_stp_register_event_cb(INT32 type, MTK_WCN_STP_EVENT_CB func)
#else
INT32 mtk_wcn_stp_register_event_cb(INT32 type, MTK_WCN_STP_EVENT_CB func)
#endif
{
    if (type < MTKSTP_MAX_TASK_NUM)
    {
        event_callback_tbl[type] = func;

        /*              */
        printk("Flush type = %d Rx Queue\n", type);
        mtk_wcn_stp_flush_rx_queue(type);
    }

    return 0;
}

#if STP_EXP_HID_API_EXPORT
INT32 _mtk_wcn_stp_register_tx_event_cb(INT32 type, MTK_WCN_STP_EVENT_CB func)
#else
INT32 mtk_wcn_stp_register_tx_event_cb(INT32 type, MTK_WCN_STP_EVENT_CB func)
#endif
{
    if(type < MTKSTP_MAX_TASK_NUM)
    {
        tx_event_callback_tbl[type] = func;
    }
    else
    {
        BUG_ON(0);
    }

    return 0;
}

INT32 stp_drv_init(VOID)
{
	INT32 ret = 0;
	
    mtkstp_callback cb =
    {
        .cb_if_tx           = mtk_wcn_sys_if_tx,
        .cb_event_set       = mtk_wcn_sys_event_set,
        .cb_event_tx_resume = mtk_wcn_sys_event_tx_resume,
        .cb_check_funciton_status = mtk_wcn_sys_check_function_status
    };

#ifdef MTK_WCN_WMT_STP_EXP_SYMBOL_ABSTRACT
	MTK_WCN_STP_EXP_CB_INFO stpExpCb = 
	{	
		.stp_send_data_cb		= _mtk_wcn_stp_send_data,
		.stp_send_data_raw_cb	= _mtk_wcn_stp_send_data_raw,
		.stp_parser_data_cb		= _mtk_wcn_stp_parser_data,
		.stp_receive_data_cb	= _mtk_wcn_stp_receive_data,
		.stp_is_rxqueue_empty_cb = _mtk_wcn_stp_is_rxqueue_empty,
		.stp_is_ready_cb		= _mtk_wcn_stp_is_ready,
		.stp_set_bluez_cb		= _mtk_wcn_stp_set_bluez,
		.stp_if_tx_cb			= _mtk_wcn_stp_register_if_tx,
		.stp_if_rx_cb			= _mtk_wcn_stp_register_if_rx,
		.stp_reg_event_cb		= _mtk_wcn_stp_register_event_cb,
		.stp_reg_tx_event_cb	= _mtk_wcn_stp_register_tx_event_cb,
		.stp_coredump_start_get_cb = _mtk_wcn_stp_coredump_start_get,
	};
#endif

    ret = mtk_wcn_stp_init(&cb);
#ifdef MTK_WCN_WMT_STP_EXP_SYMBOL_ABSTRACT
	mtk_wcn_stp_exp_cb_reg(&stpExpCb);
#endif
	return ret;
}

VOID stp_drv_exit(VOID)
{
    mtk_wcn_stp_deinit();
	
#ifdef MTK_WCN_WMT_STP_EXP_SYMBOL_ABSTRACT
	mtk_wcn_stp_exp_cb_unreg();
#endif
		
    return;
}

#ifndef MTK_WCN_WMT_STP_EXP_SYMBOL_ABSTRACT
EXPORT_SYMBOL(mtk_wcn_stp_register_if_tx);
EXPORT_SYMBOL(mtk_wcn_stp_register_if_rx);
EXPORT_SYMBOL(mtk_wcn_stp_register_event_cb);
EXPORT_SYMBOL(mtk_wcn_stp_register_tx_event_cb);
EXPORT_SYMBOL(mtk_wcn_stp_parser_data);
EXPORT_SYMBOL(mtk_wcn_stp_send_data);
EXPORT_SYMBOL(mtk_wcn_stp_send_data_raw);
EXPORT_SYMBOL(mtk_wcn_stp_receive_data);
EXPORT_SYMBOL(mtk_wcn_stp_is_rxqueue_empty);
EXPORT_SYMBOL(mtk_wcn_stp_set_bluez);
EXPORT_SYMBOL(mtk_wcn_stp_is_ready);
EXPORT_SYMBOL(mtk_wcn_stp_dbg_log_ctrl);

#endif



