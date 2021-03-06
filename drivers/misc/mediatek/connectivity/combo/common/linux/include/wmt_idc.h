
#ifndef _WMT_IDC_H_
#define _WMT_IDC_H_


#include "osal.h"

#if CFG_WMT_LTE_COEX_HANDLING
#include "conn_md_exp.h"


#define LTE_IDC_BUFFER_MAX_SIZE 1024
/*                                                                       */
#define WMT_IDC_RX_MAX_LEN 384
#define LTE_MSG_ID_OFFSET 0x30

typedef enum {
	WMT_IDC_TX_OPCODE_MIN = 0,
	WMT_IDC_TX_OPCODE_LTE_PARA = 0x0a,
	WMT_IDC_TX_OPCODE_LTE_FREQ = 0x0b,
	WMT_IDC_TX_OPCODE_WIFI_MAX_POWER = 0x0c,
	WMT_IDC_TX_OPCODE_DEBUG_MONITOR = 0x0e,
	WMT_IDC_TX_OPCODE_SPLIT_FILTER = 0x0f,
	WMT_IDC_TX_OPCODE_LTE_CONNECTION_STAS = 0x16,
	WMT_IDC_TX_OPCODE_LTE_HW_IF_INDICATION = 0x17,
	WMT_IDC_TX_OPCODE_LTE_INDICATION = 0x20,
	WMT_IDC_TX_OPCODE_MAX
} WMT_IDC_TX_OPCODE;

typedef enum {
	WMT_IDC_RX_OPCODE_BTWF_DEF_PARA = 0x0,
	WMT_IDC_RX_OPCODE_BTWF_CHAN_RAN = 0x1,
	/*                                   */
	WMT_IDC_RX_OPCODE_DEBUG_MONITOR = 0x02,
	WMT_IDC_RX_OPCODE_LTE_FREQ_IDX_TABLE = 0x03,
	WMT_IDC_RX_OPCODE_BTWF_PROFILE_IND = 0x04,
	WMT_IDC_RX_OPCODE_UART_PIN_SEL = 0x05,
	WMT_IDC_RX_OPCODE_MAX
} WMT_IDC_RX_OPCODE;

#if (CFG_WMT_LTE_ENABLE_MSGID_MAPPING == 0)
typedef enum
{
    IPC_L4C_MSG_ID_INVALID = IPC_L4C_MSG_ID_BEGIN,
    IPC_L4C_MSG_ID_END,
    IPC_EL1_MSG_ID_INVALID = IPC_EL1_MSG_ID_BEGIN,
	//                                       
	IPC_MSG_ID_EL1_LTE_TX_ALLOW_IND,
	IPC_MSG_ID_EL1_WIFIBT_OPER_DEFAULT_PARAM_IND,
	IPC_MSG_ID_EL1_WIFIBT_OPER_FREQ_IND,
	IPC_MSG_ID_EL1_WIFIBT_FREQ_IDX_TABLE_IND,
	IPC_MSG_ID_EL1_WIFIBT_PROFILE_IND,
	
	//                                 
	IPC_MSG_ID_EL1_LTE_DEFAULT_PARAM_IND,
	IPC_MSG_ID_EL1_LTE_OPER_FREQ_PARAM_IND,
	IPC_MSG_ID_EL1_WIFI_MAX_PWR_IND,
	IPC_MSG_ID_EL1_LTE_TX_IND,
    IPC_EL1_MSG_ID_END,    
}IPC_MSG_ID_CODE;
#endif

typedef struct _MTK_WCN_WMT_IDC_INFO_ {
	ipc_ilm_t iit;
	CONN_MD_BRIDGE_OPS ops;
	UINT8 buffer[LTE_IDC_BUFFER_MAX_SIZE];

} MTK_WCN_WMT_IDC_INFO, *P_MTK_WCN_WMT_IDC_INFO;

extern INT32 wmt_idc_init(VOID);
extern INT32 wmt_idc_deinit(VOID);
extern INT32 wmt_idc_msg_to_lte_handing(VOID);
extern UINT32 wmt_idc_msg_to_lte_handing_for_test(PUINT8 p_buf, UINT32 len);

#else

#if CFG_WMT_LTE_COEX_HANDLING
INT32 wmt_idc_init(VOID)
{
	return 0;
}

INT32 wmt_idc_deinit(VOID)
{
	return 0;
}

INT32 wmt_idc_msg_to_lte_handing(VOID)
{
	return 0;
}

UINT32 wmt_idc_msg_to_lte_handing_for_test(PUINT8 p_buf, UINT32 len)
{
	return 0;
}

#endif

#endif

#endif
