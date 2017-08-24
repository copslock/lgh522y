/*****************************************************************************

            (c) Cambridge Silicon Radio Limited 2011
            All rights reserved and confidential information of CSR

            Refer to LICENSE.txt included with this source for details
            on the license terms.

*****************************************************************************/

/*                                       */


/*                                */

#ifndef CSR_WIFI_HIP_SIGS_H
#define CSR_WIFI_HIP_SIGS_H

typedef s16 csr_place_holding_type;

typedef u16 CSR_ASSOCIATION_ID;

typedef u16 CSR_AUTONOMOUS_SCAN_ID;

typedef u16 CSR_BEACON_PERIODS;

typedef u16 CSR_BLACKOUT_ID;

typedef enum CSR_BLACKOUT_SOURCE
{
    CSR_DOT11_LOCAL                               = 0x0000,
    CSR_DOT11_REMOTE                              = 0x0001,
    CSR_OTHER_RADIO                               = 0x0002,
    CSR_NOT_LINKED                                = 0x0004
} CSR_BLACKOUT_SOURCE;

typedef enum CSR_BLACKOUT_TYPE
{
    CSR_LOCAL_DEVICE_ONLY                         = 0x0001,
    CSR_SPECIFIED_PEER                            = 0x0002,
    CSR_CURRENT_CHANNEL                           = 0x0004,
    CSR_P2P                                       = 0x0008
} CSR_BLACKOUT_TYPE;

typedef enum CSR_BOOT_LOADER_OPERATION
{
    CSR_BOOT_LOADER_IDLE                          = 0x00,
    CSR_BOOT_LOADER_RESTART                       = 0x01,
    CSR_BOOT_LOADER_PATCH                         = 0x02,
    CSR_BOOT_LOADER_IMAGE_0                       = 0x10,
    CSR_BOOT_LOADER_IMAGE_1                       = 0x11,
    CSR_BOOT_LOADER_IMAGE_2                       = 0x12,
    CSR_BOOT_LOADER_IMAGE_3                       = 0x13
} CSR_BOOT_LOADER_OPERATION;

typedef u16 CSR_CAPABILITY_INFORMATION;

typedef u16 CSR_CHANNEL_STARTING_FACTOR;

typedef u32 CSR_CIPHER_SUITE_SELECTOR;

typedef u32 CSR_CLIENT_TAG;

typedef enum CSR_CONNECTION_STATUS
{
    CSR_DISCONNECTED                              = 0x0000,
    CSR_CONNECTED_AWAKE                           = 0x0001
} CSR_CONNECTION_STATUS;

typedef s16 CSR_DECIBELS;

typedef enum CSR_DIRECTION
{
    CSR_TRANSMIT                                  = 0x0000,
    CSR_RECEIVE                                   = 0x0001,
    CSR_BIDIRECTIONAL                             = 0x0003
} CSR_DIRECTION;

typedef enum CSR_FRAME_TYPE
{
    CSR_RESERVED                                  = 0x0000,
    CSR_BEACON                                    = 0x0001,
    CSR_PROBE_RESPONSE                            = 0x0002,
    CSR_BEACON_AND_PROBE_RESPONSE                 = 0x0003,
    CSR_PROBE_REQUEST                             = 0x0004
} CSR_FRAME_TYPE;

typedef u32 CSR_IPV4_ADDRESS;

typedef enum CSR_IFINTERFACE
{
    CSR_INDEX_2G4                                 = 0x0001,
    CSR_INDEX_5G                                  = 0x0002
} CSR_IFINTERFACE;

typedef enum CSR_KEY_TYPE
{
    CSR_GROUP                                     = 0x0000,
    CSR_PAIRWISE                                  = 0x0001,
    CSR_PEER_KEY                                  = 0x0002,
    CSR_IGTK                                      = 0x0003
} CSR_KEY_TYPE;

typedef enum CSR_LOADER_OPERATION
{
    CSR_LOADER_IDLE                               = 0x0000,
    CSR_LOADER_COPY                               = 0x0001
} CSR_LOADER_OPERATION;

typedef struct CSR_MAC_ADDRESS
{
    u8 x[6];
} CSR_MACADDRESS;

typedef enum CSR_MIB_STATUS
{
    CSR_MIB_SUCCESSFUL                            = 0x0000,
    CSR_MIB_INVALID_PARAMETERS                    = 0x0001,
    CSR_MIB_WRITE_ONLY                            = 0x0002,
    CSR_MIB_READ_ONLY                             = 0x0003
} CSR_MIB_STATUS;

typedef enum CSR_MEMORY_SPACE
{
    CSR_NONE                                      = 0x00,
    CSR_SHARED_DATA_MEMORY                        = 0x01,
    CSR_EXTERNAL_FLASH_MEMORY                     = 0x02,
    CSR_EXTERNAL_SRAM                             = 0x03,
    CSR_REGISTERS                                 = 0x04,
    CSR_PHY_PROCESSOR_DATA_MEMORY                 = 0x10,
    CSR_PHY_PROCESSOR_PROGRAM_MEMORY              = 0x11,
    CSR_PHY_PROCESSOR_ROM                         = 0x12,
    CSR_MAC_PROCESSOR_DATA_MEMORY                 = 0x20,
    CSR_MAC_PROCESSOR_PROGRAM_MEMORY              = 0x21,
    CSR_MAC_PROCESSOR_ROM                         = 0x22,
    CSR_BT_PROCESSOR_DATA_MEMORY                  = 0x30,
    CSR_BT_PROCESSOR_PROGRAM_MEMORY               = 0x31,
    CSR_BT_PROCESSOR_ROM                          = 0x32
} CSR_MEMORY_SPACE;

typedef u16 CSR_MICROSECONDS16;

typedef u32 CSR_MICROSECONDS32;

typedef u16 CSR_NATURAL16;

typedef enum CSR_PS_SCHEME
{
    CSR_LEGACY_PS                                 = 0x0001,
    CSR_U_APSD                                    = 0x0002,
    CSR_S_APSD                                    = 0x0004
} CSR_PS_SCHEME;

typedef enum CSR_PACKET_FILTER_MODE
{
    CSR_PFM_OPT_OUT                               = 0x0000,
    CSR_PFM_OPT_IN                                = 0x0003
} CSR_PACKET_FILTER_MODE;

typedef u16 CSR_PERIODIC_ID;

typedef enum CSR_PERIODIC_SCHEDULING_MODE
{
    CSR_PSM_PERIODIC_SCHEDULE_PS_POLL             = 0x0001,
    CSR_PSM_PERIODIC_SCHEDULE_PM_BIT              = 0x0002,
    CSR_PSM_PERIODIC_SCHEDULE_UAPSD               = 0x0004,
    CSR_PSM_PERIODIC_SCHEDULE_SAPSD               = 0x0008
} CSR_PERIODIC_SCHEDULING_MODE;

typedef enum CSR_POWER_MANAGEMENT_MODE
{
    CSR_PMM_ACTIVE_MODE                           = 0x0000,
    CSR_PMM_POWER_SAVE                            = 0x0001,
    CSR_PMM_FAST_POWER_SAVE                       = 0x0002
} CSR_POWER_MANAGEMENT_MODE;

typedef enum CSR_PRIORITY
{
    CSR_QOS_UP0                                   = 0x0000,
    CSR_QOS_UP1                                   = 0x0001,
    CSR_QOS_UP2                                   = 0x0002,
    CSR_QOS_UP3                                   = 0x0003,
    CSR_QOS_UP4                                   = 0x0004,
    CSR_QOS_UP5                                   = 0x0005,
    CSR_QOS_UP6                                   = 0x0006,
    CSR_QOS_UP7                                   = 0x0007,
    CSR_CONTENTION                                = 0x8000,
    CSR_MANAGEMENT                                = 0x8010
} CSR_PRIORITY;

typedef enum CSR_REASON_CODE
{
    CSR_UNSPECIFIED_REASON                        = 0x0001,
    CSR_INVALID_INFORMATION_ELEMENT               = 0x000d,
    CSR_QOS_UNSPECIFIED_REASON                    = 0x0020,
    CSR_QOS_EXCESSIVE_NOT_ACK                     = 0x0022,
    CSR_QOS_TXOP_LIMIT_EXCEEDED                   = 0x0023,
    CSR_QSTA_LEAVING                              = 0x0024,
    CSR_UNKNOWN_BA                                = 0x0026,
    CSR_UNKNOWN_TS                                = 0x0026,
    CSR_TIMEOUT                                   = 0x0027
} CSR_REASON_CODE;

typedef enum CSR_RECEPTION_STATUS
{
    CSR_RX_SUCCESS                                = 0x0000,
    CSR_RX_FAILURE_UNSPECIFIED                    = 0x0001,
    CSR_MICHAEL_MIC_ERROR                         = 0x0002,
    CSR_DECRYPTION_ERROR                          = 0x0003,
    CSR_NO_TEMPORAL_KEY_AVAILABLE                 = 0x0004,
    CSR_UNSUPPORTED_MODULATION                    = 0x0011,
    CSR_BAD_FCS                                   = 0x0012,
    CSR_BAD_SIGNAL                                = 0x0013
} CSR_RECEPTION_STATUS;

typedef enum CSR_RESULT_CODE
{
    CSR_RC_SUCCESS                                = 0x0000,
    CSR_RC_UNSPECIFIED_FAILURE                    = 0x0001,
    CSR_RC_REFUSED                                = 0x0003,
    CSR_RC_INVALID_PARAMETERS                     = 0x0026,
    CSR_RC_REJECTED_INVALID_IE                    = 0x0028,
    CSR_RC_REJECTED_INVALID_GROUP_CIPHER          = 0x0029,
    CSR_RC_REJECTED_INVALID_PAIRWISE_CIPHER       = 0x002a,
    CSR_RC_TIMEOUT                                = 0x8000,
    CSR_RC_TOO_MANY_SIMULTANEOUS_REQUESTS         = 0x8001,
    CSR_RC_BSS_ALREADY_STARTED_OR_JOINED          = 0x8002,
    CSR_RC_NOT_SUPPORTED                          = 0x8003,
    CSR_RC_TRANSMISSION_FAILURE                   = 0x8004,
    CSR_RC_RESET_REQUIRED_BEFORE_START            = 0x8006,
    CSR_RC_INSUFFICIENT_RESOURCE                  = 0x8007,
    CSR_RC_NO_BUFFERED_BROADCAST_MULTICAST_FRAMES = 0x8008,
    CSR_RC_INVALID_UNICAST_CIPHER                 = 0xf02f,
    CSR_RC_INVALID_MULTICAST_CIPHER               = 0xf030
} CSR_RESULT_CODE;

typedef enum CSR_SCAN_TYPE
{
    CSR_SC_ACTIVE_SCAN                            = 0x0000,
    CSR_SC_PASSIVE_SCAN                           = 0x0001
} CSR_SCAN_TYPE;

typedef enum CSR_SIGNAL_ID
{
    CSR_MA_PACKET_REQUEST_ID                      = 0x0110,
    CSR_MA_PACKET_CONFIRM_ID                      = 0x0111,
    CSR_MA_PACKET_INDICATION_ID                   = 0x0113,
    CSR_MA_PACKET_CANCEL_REQUEST_ID               = 0x0114,
    CSR_MA_VIF_AVAILABILITY_RESPONSE_ID           = 0x0116,
    CSR_MA_VIF_AVAILABILITY_INDICATION_ID         = 0x0117,
    CSR_MA_PACKET_ERROR_INDICATION_ID             = 0x011b,
    CSR_MLME_RESET_REQUEST_ID                     = 0x0200,
    CSR_MLME_RESET_CONFIRM_ID                     = 0x0201,
    CSR_MLME_GET_REQUEST_ID                       = 0x0204,
    CSR_MLME_GET_CONFIRM_ID                       = 0x0205,
    CSR_MLME_SET_REQUEST_ID                       = 0x0208,
    CSR_MLME_SET_CONFIRM_ID                       = 0x0209,
    CSR_MLME_GET_NEXT_REQUEST_ID                  = 0x020c,
    CSR_MLME_GET_NEXT_CONFIRM_ID                  = 0x020d,
    CSR_MLME_POWERMGT_REQUEST_ID                  = 0x0210,
    CSR_MLME_POWERMGT_CONFIRM_ID                  = 0x0211,
    CSR_MLME_SCAN_REQUEST_ID                      = 0x0214,
    CSR_MLME_SCAN_CONFIRM_ID                      = 0x0215,
    CSR_MLME_HL_SYNC_REQUEST_ID                   = 0x0244,
    CSR_MLME_HL_SYNC_CONFIRM_ID                   = 0x0245,
    CSR_MLME_MEASURE_REQUEST_ID                   = 0x0258,
    CSR_MLME_MEASURE_CONFIRM_ID                   = 0x0259,
    CSR_MLME_MEASURE_INDICATION_ID                = 0x025b,
    CSR_MLME_SETKEYS_REQUEST_ID                   = 0x0268,
    CSR_MLME_SETKEYS_CONFIRM_ID                   = 0x0269,
    CSR_MLME_DELETEKEYS_REQUEST_ID                = 0x026c,
    CSR_MLME_DELETEKEYS_CONFIRM_ID                = 0x026d,
    CSR_MLME_AUTONOMOUS_SCAN_LOSS_INDICATION_ID   = 0x0287,
    CSR_MLME_CONNECTED_INDICATION_ID              = 0x028b,
    CSR_MLME_SCAN_CANCEL_REQUEST_ID               = 0x028c,
    CSR_MLME_HL_SYNC_CANCEL_REQUEST_ID            = 0x0298,
    CSR_MLME_HL_SYNC_CANCEL_CONFIRM_ID            = 0x0299,
    CSR_MLME_ADD_PERIODIC_REQUEST_ID              = 0x02a0,
    CSR_MLME_ADD_PERIODIC_CONFIRM_ID              = 0x02a1,
    CSR_MLME_DEL_PERIODIC_REQUEST_ID              = 0x02a4,
    CSR_MLME_DEL_PERIODIC_CONFIRM_ID              = 0x02a5,
    CSR_MLME_ADD_AUTONOMOUS_SCAN_REQUEST_ID       = 0x02a8,
    CSR_MLME_ADD_AUTONOMOUS_SCAN_CONFIRM_ID       = 0x02a9,
    CSR_MLME_DEL_AUTONOMOUS_SCAN_REQUEST_ID       = 0x02ac,
    CSR_MLME_DEL_AUTONOMOUS_SCAN_CONFIRM_ID       = 0x02ad,
    CSR_MLME_SET_PACKET_FILTER_REQUEST_ID         = 0x02b8,
    CSR_MLME_SET_PACKET_FILTER_CONFIRM_ID         = 0x02b9,
    CSR_MLME_STOP_MEASURE_REQUEST_ID              = 0x02bc,
    CSR_MLME_STOP_MEASURE_CONFIRM_ID              = 0x02bd,
    CSR_MLME_PAUSE_AUTONOMOUS_SCAN_REQUEST_ID     = 0x02cc,
    CSR_MLME_PAUSE_AUTONOMOUS_SCAN_CONFIRM_ID     = 0x02cd,
    CSR_MLME_AUTONOMOUS_SCAN_DONE_INDICATION_ID   = 0x02db,
    CSR_MLME_ADD_TRIGGERED_GET_REQUEST_ID         = 0x02dc,
    CSR_MLME_ADD_TRIGGERED_GET_CONFIRM_ID         = 0x02dd,
    CSR_MLME_DEL_TRIGGERED_GET_REQUEST_ID         = 0x02e0,
    CSR_MLME_DEL_TRIGGERED_GET_CONFIRM_ID         = 0x02e1,
    CSR_MLME_TRIGGERED_GET_INDICATION_ID          = 0x02e7,
    CSR_MLME_ADD_BLACKOUT_REQUEST_ID              = 0x02f8,
    CSR_MLME_ADD_BLACKOUT_CONFIRM_ID              = 0x02f9,
    CSR_MLME_BLACKOUT_ENDED_INDICATION_ID         = 0x02fb,
    CSR_MLME_DEL_BLACKOUT_REQUEST_ID              = 0x02fc,
    CSR_MLME_DEL_BLACKOUT_CONFIRM_ID              = 0x02fd,
    CSR_MLME_ADD_RX_TRIGGER_REQUEST_ID            = 0x0304,
    CSR_MLME_ADD_RX_TRIGGER_CONFIRM_ID            = 0x0305,
    CSR_MLME_DEL_RX_TRIGGER_REQUEST_ID            = 0x0308,
    CSR_MLME_DEL_RX_TRIGGER_CONFIRM_ID            = 0x0309,
    CSR_MLME_CONNECT_STATUS_REQUEST_ID            = 0x0310,
    CSR_MLME_CONNECT_STATUS_CONFIRM_ID            = 0x0311,
    CSR_MLME_MODIFY_BSS_PARAMETER_REQUEST_ID      = 0x0314,
    CSR_MLME_MODIFY_BSS_PARAMETER_CONFIRM_ID      = 0x0315,
    CSR_MLME_ADD_TEMPLATE_REQUEST_ID              = 0x0318,
    CSR_MLME_ADD_TEMPLATE_CONFIRM_ID              = 0x0319,
    CSR_MLME_CONFIG_QUEUE_REQUEST_ID              = 0x031c,
    CSR_MLME_CONFIG_QUEUE_CONFIRM_ID              = 0x031d,
    CSR_MLME_ADD_TSPEC_REQUEST_ID                 = 0x0320,
    CSR_MLME_ADD_TSPEC_CONFIRM_ID                 = 0x0321,
    CSR_MLME_DEL_TSPEC_REQUEST_ID                 = 0x0324,
    CSR_MLME_DEL_TSPEC_CONFIRM_ID                 = 0x0325,
    CSR_MLME_START_AGGREGATION_REQUEST_ID         = 0x0328,
    CSR_MLME_START_AGGREGATION_CONFIRM_ID         = 0x0329,
    CSR_MLME_BLOCKACK_ERROR_INDICATION_ID         = 0x032b,
    CSR_MLME_STOP_AGGREGATION_REQUEST_ID          = 0x032c,
    CSR_MLME_STOP_AGGREGATION_CONFIRM_ID          = 0x032d,
    CSR_MLME_SM_START_REQUEST_ID                  = 0x0334,
    CSR_MLME_SM_START_CONFIRM_ID                  = 0x0335,
    CSR_MLME_LEAVE_REQUEST_ID                     = 0x0338,
    CSR_MLME_LEAVE_CONFIRM_ID                     = 0x0339,
    CSR_MLME_SET_TIM_REQUEST_ID                   = 0x033c,
    CSR_MLME_SET_TIM_CONFIRM_ID                   = 0x033d,
    CSR_MLME_GET_KEY_SEQUENCE_REQUEST_ID          = 0x0340,
    CSR_MLME_GET_KEY_SEQUENCE_CONFIRM_ID          = 0x0341,
    CSR_MLME_SET_CHANNEL_REQUEST_ID               = 0x034c,
    CSR_MLME_SET_CHANNEL_CONFIRM_ID               = 0x034d,
    CSR_MLME_ADD_MULTICAST_ADDRESS_REQUEST_ID     = 0x040c,
    CSR_MLME_ADD_MULTICAST_ADDRESS_CONFIRM_ID     = 0x040d,
    CSR_DEBUG_STRING_INDICATION_ID                = 0x0803,
    CSR_DEBUG_WORD16_INDICATION_ID                = 0x0807,
    CSR_DEBUG_GENERIC_REQUEST_ID                  = 0x0808,
    CSR_DEBUG_GENERIC_CONFIRM_ID                  = 0x0809,
    CSR_DEBUG_GENERIC_INDICATION_ID               = 0x080b
} CSR_SIGNAL_ID;

typedef u16 CSR_SIMPLE_POINTER;

typedef u16 CSR_STARTING_SEQUENCE_NUMBER;

typedef enum CSR_SYMBOL_ID
{
    CSR_SLT_END                                   = 0x0000,
    CSR_SLT_PCI_SLOT_CONFIG                       = 0x0001,
    CSR_SLT_SDIO_SLOT_CONFIG                      = 0x0002,
    CSR_SLT_BUILD_ID_NUMBER                       = 0x0003,
    CSR_SLT_BUILD_ID_STRING                       = 0x0004,
    CSR_SLT_PERSISTENT_STORE_DB                   = 0x0005,
    CSR_SLT_RESET_VECTOR_PHY                      = 0x0006,
    CSR_SLT_RESET_VECTOR_MAC                      = 0x0007,
    CSR_SLT_SDIO_LOADER_CONTROL                   = 0x0008,
    CSR_SLT_TEST_CMD                              = 0x0009,
    CSR_SLT_TEST_ALIVE_COUNTER                    = 0x000a,
    CSR_SLT_TEST_PARAMETERS                       = 0x000b,
    CSR_SLT_TEST_RESULTS                          = 0x000c,
    CSR_SLT_TEST_VERSION                          = 0x000d,
    CSR_SLT_MIB_PSID_RANGES                       = 0x000e,
    CSR_SLT_KIP_TABLE                             = 0x000f,
    CSR_SLT_PANIC_DATA_PHY                        = 0x0010,
    CSR_SLT_PANIC_DATA_MAC                        = 0x0011,
    CSR_SLT_BOOT_LOADER_CONTROL                   = 0x0012,
    CSR_SLT_SOFT_MAC                              = 0x0013
} CSR_SYMBOL_ID;

typedef struct CSR_TSF_TIME
{
    u8 x[8];
} CSR_TSF_TIME;

typedef u16 CSR_TIME_UNITS;

typedef enum CSR_TRANSMISSION_CONTROL
{
    CSR_TRIGGERED                                 = 0x0001,
    CSR_END_OF_SERVICE                            = 0x0002,
    CSR_NO_CONFIRM_REQUIRED                       = 0x0004,
    CSR_ALLOW_BA                                  = 0x0008
} CSR_TRANSMISSION_CONTROL;

typedef enum CSR_TRANSMISSION_STATUS
{
    CSR_TX_SUCCESSFUL                             = 0x0000,
    CSR_TX_RETRY_LIMIT                            = 0x0001,
    CSR_TX_LIFETIME                               = 0x0002,
    CSR_TX_NO_BSS                                 = 0x0003,
    CSR_TX_EXCESSIVE_DATA_LENGTH                  = 0x0004,
    CSR_TX_UNSUPPORTED_PRIORITY                   = 0x0006,
    CSR_TX_UNAVAILABLE_PRIORITY                   = 0x0007,
    CSR_TX_UNAVAILABLE_KEY_MAPPING                = 0x000a,
    CSR_TX_EDCA_TIMEOUT                           = 0x000b,
    CSR_TX_BLOCK_ACK_TIMEOUT                      = 0x000c,
    CSR_TX_FAIL_TRANSMISSION_VIF_INTERRUPTED      = 0x000d,
    CSR_TX_REJECTED_PEER_STATION_SLEEPING         = 0x000e,
    CSR_TX_REJECTED_DTIM_ENDED                    = 0x000f,
    CSR_TX_REJECTED_DTIM_STARTED                  = 0x0010
} CSR_TRANSMISSION_STATUS;

typedef u16 CSR_TRIGGER_ID;

typedef u16 CSR_TRIGGERED_ID;

typedef enum CSR_HIP_VERSIONS
{
    CSR_HIP_ENG_VERSION                           = 0x0001,
    CSR_HIP_VERSION                               = 0x0900
} CSR_HIP_VERSIONS;

typedef u16 CSR_BUFFER_HANDLE;

typedef u16 CSR_CHANNEL_NUMBER;

typedef struct CSR_DATA_REFERENCE
{
    u16 SlotNumber;
    u16 DataLength;
} CSR_DATAREF;

typedef u16 CSR_DIALOG_TOKEN;

typedef struct CSR_GENERIC_POINTER
{
    u32        MemoryOffset;
    CSR_MEMORY_SPACE MemorySpace;
} CSR_GENERIC_POINTER;

typedef struct CSR_MLME_CONFIG_QUEUE_CONFIRM
{
    CSR_DATAREF     Dummydataref1;
    CSR_DATAREF     Dummydataref2;
    CSR_RESULT_CODE ResultCode;
} CSR_MLME_CONFIG_QUEUE_CONFIRM;

typedef struct CSR_MLME_CONFIG_QUEUE_REQUEST
{
    CSR_DATAREF   Dummydataref1;
    CSR_DATAREF   Dummydataref2;
    CSR_NATURAL16 QueueIndex;
    CSR_NATURAL16 Aifs;
    CSR_NATURAL16 Cwmin;
    CSR_NATURAL16 Cwmax;
    CSR_NATURAL16 TxopLimit;
} CSR_MLME_CONFIG_QUEUE_REQUEST;

typedef struct CSR_MLME_GET_CONFIRM
{
    CSR_DATAREF    MibAttributeValue;
    CSR_DATAREF    Dummydataref2;
    CSR_MIB_STATUS Status;
    CSR_NATURAL16  ErrorIndex;
} CSR_MLME_GET_CONFIRM;

typedef struct CSR_MLME_GET_REQUEST
{
    CSR_DATAREF MibAttribute;
    CSR_DATAREF Dummydataref2;
} CSR_MLME_GET_REQUEST;

typedef struct CSR_MLME_GET_NEXT_CONFIRM
{
    CSR_DATAREF    MibAttributeValue;
    CSR_DATAREF    Dummydataref2;
    CSR_MIB_STATUS Status;
    CSR_NATURAL16  ErrorIndex;
} CSR_MLME_GET_NEXT_CONFIRM;

typedef struct CSR_MLME_GET_NEXT_REQUEST
{
    CSR_DATAREF MibAttribute;
    CSR_DATAREF Dummydataref2;
} CSR_MLME_GET_NEXT_REQUEST;

typedef struct CSR_MLME_HL_SYNC_CONFIRM
{
    CSR_DATAREF     Dummydataref1;
    CSR_DATAREF     Dummydataref2;
    CSR_MACADDRESS  GroupAddress;
    CSR_RESULT_CODE ResultCode;
} CSR_MLME_HL_SYNC_CONFIRM;

typedef struct CSR_MLME_HL_SYNC_REQUEST
{
    CSR_DATAREF    Dummydataref1;
    CSR_DATAREF    Dummydataref2;
    CSR_MACADDRESS GroupAddress;
} CSR_MLME_HL_SYNC_REQUEST;

typedef struct CSR_MLME_HL_SYNC_CANCEL_CONFIRM
{
    CSR_DATAREF     Dummydataref1;
    CSR_DATAREF     Dummydataref2;
    CSR_RESULT_CODE ResultCode;
} CSR_MLME_HL_SYNC_CANCEL_CONFIRM;

typedef struct CSR_MLME_HL_SYNC_CANCEL_REQUEST
{
    CSR_DATAREF    Dummydataref1;
    CSR_DATAREF    Dummydataref2;
    CSR_MACADDRESS GroupAddress;
} CSR_MLME_HL_SYNC_CANCEL_REQUEST;

typedef struct CSR_MLME_MEASURE_CONFIRM
{
    CSR_DATAREF      Dummydataref1;
    CSR_DATAREF      Dummydataref2;
    CSR_RESULT_CODE  ResultCode;
    CSR_DIALOG_TOKEN DialogToken;
} CSR_MLME_MEASURE_CONFIRM;

typedef struct CSR_MLME_MEASURE_INDICATION
{
    CSR_DATAREF      MeasurementReportSet;
    CSR_DATAREF      Dummydataref2;
    CSR_DIALOG_TOKEN DialogToken;
} CSR_MLME_MEASURE_INDICATION;

typedef struct CSR_MLME_MEASURE_REQUEST
{
    CSR_DATAREF      MeasurementRequestSet;
    CSR_DATAREF      Dummydataref2;
    CSR_DIALOG_TOKEN DialogToken;
} CSR_MLME_MEASURE_REQUEST;

typedef struct CSR_MLME_RESET_CONFIRM
{
    CSR_DATAREF     Dummydataref1;
    CSR_DATAREF     Dummydataref2;
    CSR_RESULT_CODE ResultCode;
} CSR_MLME_RESET_CONFIRM;

typedef struct CSR_MLME_RESET_REQUEST
{
    CSR_DATAREF    Dummydataref1;
    CSR_DATAREF    Dummydataref2;
    CSR_MACADDRESS StaAddress;
    s16       SetDefaultMib;
} CSR_MLME_RESET_REQUEST;

typedef struct CSR_MLME_SET_CONFIRM
{
    CSR_DATAREF    MibAttributeValue;
    CSR_DATAREF    Dummydataref2;
    CSR_MIB_STATUS Status;
    CSR_NATURAL16  ErrorIndex;
} CSR_MLME_SET_CONFIRM;

typedef struct CSR_MLME_SET_REQUEST
{
    CSR_DATAREF MibAttributeValue;
    CSR_DATAREF Dummydataref2;
} CSR_MLME_SET_REQUEST;

typedef struct CSR_MLME_STOP_MEASURE_CONFIRM
{
    CSR_DATAREF      Dummydataref1;
    CSR_DATAREF      Dummydataref2;
    CSR_RESULT_CODE  ResultCode;
    CSR_DIALOG_TOKEN DialogToken;
} CSR_MLME_STOP_MEASURE_CONFIRM;

typedef struct CSR_MLME_STOP_MEASURE_REQUEST
{
    CSR_DATAREF      Dummydataref1;
    CSR_DATAREF      Dummydataref2;
    CSR_DIALOG_TOKEN DialogToken;
} CSR_MLME_STOP_MEASURE_REQUEST;

typedef u16 CSR_PROCESS_ID;

typedef u16 CSR_RATE;

typedef u16 CSR_SEQUENCE_NUMBER;

typedef struct CSR_SIGNAL_PRIMITIVE_HEADER
{
    s16       SignalId;
    CSR_PROCESS_ID ReceiverProcessId;
    CSR_PROCESS_ID SenderProcessId;
} CSR_SIGNAL_PRIMITIVE_HEADER;

typedef u16 CSR_TRAFFIC_WINDOW;

typedef u16 CSR_VIF_IDENTIFIER;

typedef struct CSR_DEBUG_GENERIC_CONFIRM
{
    CSR_DATAREF   DebugVariable;
    CSR_DATAREF   Dummydataref2;
    CSR_NATURAL16 DebugWords[8];
} CSR_DEBUG_GENERIC_CONFIRM;

typedef struct CSR_DEBUG_GENERIC_INDICATION
{
    CSR_DATAREF   DebugVariable;
    CSR_DATAREF   Dummydataref2;
    CSR_NATURAL16 DebugWords[8];
} CSR_DEBUG_GENERIC_INDICATION;

typedef struct CSR_DEBUG_GENERIC_REQUEST
{
    CSR_DATAREF   DebugVariable;
    CSR_DATAREF   Dummydataref2;
    CSR_NATURAL16 DebugWords[8];
} CSR_DEBUG_GENERIC_REQUEST;

typedef struct CSR_DEBUG_STRING_INDICATION
{
    CSR_DATAREF DebugMessage;
    CSR_DATAREF Dummydataref2;
} CSR_DEBUG_STRING_INDICATION;

typedef struct CSR_DEBUG_WORD16_INDICATION
{
    CSR_DATAREF   Dummydataref1;
    CSR_DATAREF   Dummydataref2;
    CSR_NATURAL16 DebugWords[16];
} CSR_DEBUG_WORD16_INDICATION;

typedef struct CSR_MA_PACKET_CONFIRM
{
    CSR_DATAREF             Dummydataref1;
    CSR_DATAREF             Dummydataref2;
    CSR_VIF_IDENTIFIER      VirtualInterfaceIdentifier;
    CSR_TRANSMISSION_STATUS TransmissionStatus;
    CSR_NATURAL16           RetryCount;
    CSR_RATE                Rate;
    CSR_CLIENT_TAG          HostTag;
} CSR_MA_PACKET_CONFIRM;

typedef struct CSR_MA_PACKET_INDICATION
{
    CSR_DATAREF          Data;
    CSR_DATAREF          Dummydataref2;
    CSR_VIF_IDENTIFIER   VirtualInterfaceIdentifier;
    CSR_TSF_TIME         LocalTime;
    CSR_IFINTERFACE      Ifindex;
    CSR_CHANNEL_NUMBER   Channel;
    CSR_RECEPTION_STATUS ReceptionStatus;
    CSR_DECIBELS         Rssi;
    CSR_DECIBELS         Snr;
    CSR_RATE             ReceivedRate;
} CSR_MA_PACKET_INDICATION;

typedef struct CSR_MA_PACKET_REQUEST
{
    CSR_DATAREF              Data;
    CSR_DATAREF              Dummydataref2;
    CSR_VIF_IDENTIFIER       VirtualInterfaceIdentifier;
    CSR_RATE                 TransmitRate;
    CSR_CLIENT_TAG           HostTag;
    CSR_PRIORITY             Priority;
    CSR_MACADDRESS           Ra;
    CSR_TRANSMISSION_CONTROL TransmissionControl;
} CSR_MA_PACKET_REQUEST;

typedef struct CSR_MA_PACKET_CANCEL_REQUEST
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_CLIENT_TAG     HostTag;
} CSR_MA_PACKET_CANCEL_REQUEST;

typedef struct CSR_MA_PACKET_ERROR_INDICATION
{
    CSR_DATAREF         Dummydataref1;
    CSR_DATAREF         Dummydataref2;
    CSR_VIF_IDENTIFIER  VirtualInterfaceIdentifier;
    CSR_MACADDRESS      PeerQstaAddress;
    CSR_PRIORITY        UserPriority;
    CSR_SEQUENCE_NUMBER SequenceNumber;
} CSR_MA_PACKET_ERROR_INDICATION;

typedef struct CSR_MA_VIF_AVAILABILITY_INDICATION
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    s16           Multicast;
} CSR_MA_VIF_AVAILABILITY_INDICATION;

typedef struct CSR_MA_VIF_AVAILABILITY_RESPONSE
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_RESULT_CODE    ResultCode;
} CSR_MA_VIF_AVAILABILITY_RESPONSE;

typedef struct CSR_MLME_ADD_AUTONOMOUS_SCAN_CONFIRM
{
    CSR_DATAREF            Dummydataref1;
    CSR_DATAREF            Dummydataref2;
    CSR_VIF_IDENTIFIER     VirtualInterfaceIdentifier;
    CSR_RESULT_CODE        ResultCode;
    CSR_AUTONOMOUS_SCAN_ID AutonomousScanId;
} CSR_MLME_ADD_AUTONOMOUS_SCAN_CONFIRM;

typedef struct CSR_MLME_ADD_AUTONOMOUS_SCAN_REQUEST
{
    CSR_DATAREF                 ChannelList;
    CSR_DATAREF                 InformationElements;
    CSR_VIF_IDENTIFIER          VirtualInterfaceIdentifier;
    CSR_AUTONOMOUS_SCAN_ID      AutonomousScanId;
    CSR_IFINTERFACE             Ifindex;
    CSR_CHANNEL_STARTING_FACTOR ChannelStartingFactor;
    CSR_SCAN_TYPE               ScanType;
    CSR_MICROSECONDS32          ProbeDelay;
    CSR_TIME_UNITS              MinChannelTime;
    CSR_TIME_UNITS              MaxChannelTime;
} CSR_MLME_ADD_AUTONOMOUS_SCAN_REQUEST;

typedef struct CSR_MLME_ADD_BLACKOUT_CONFIRM
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_BLACKOUT_ID    BlackoutId;
    CSR_RESULT_CODE    ResultCode;
} CSR_MLME_ADD_BLACKOUT_CONFIRM;

typedef struct CSR_MLME_ADD_BLACKOUT_REQUEST
{
    CSR_DATAREF         Dummydataref1;
    CSR_DATAREF         Dummydataref2;
    CSR_VIF_IDENTIFIER  VirtualInterfaceIdentifier;
    CSR_BLACKOUT_ID     BlackoutId;
    CSR_BLACKOUT_TYPE   BlackoutType;
    CSR_BLACKOUT_SOURCE BlackoutSource;
    CSR_MICROSECONDS32  BlackoutStartReference;
    CSR_MICROSECONDS32  BlackoutPeriod;
    CSR_MICROSECONDS32  BlackoutDuration;
    CSR_MACADDRESS      PeerStaAddress;
    CSR_NATURAL16       BlackoutCount;
} CSR_MLME_ADD_BLACKOUT_REQUEST;

typedef struct CSR_MLME_ADD_MULTICAST_ADDRESS_CONFIRM
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_RESULT_CODE    ResultCode;
} CSR_MLME_ADD_MULTICAST_ADDRESS_CONFIRM;

typedef struct CSR_MLME_ADD_MULTICAST_ADDRESS_REQUEST
{
    CSR_DATAREF        Data;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_NATURAL16      NumberOfMulticastGroupAddresses;
} CSR_MLME_ADD_MULTICAST_ADDRESS_REQUEST;

typedef struct CSR_MLME_ADD_PERIODIC_CONFIRM
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_PERIODIC_ID    PeriodicId;
    CSR_RESULT_CODE    ResultCode;
} CSR_MLME_ADD_PERIODIC_CONFIRM;

typedef struct CSR_MLME_ADD_PERIODIC_REQUEST
{
    CSR_DATAREF                  Dummydataref1;
    CSR_DATAREF                  Dummydataref2;
    CSR_VIF_IDENTIFIER           VirtualInterfaceIdentifier;
    CSR_PERIODIC_ID              PeriodicId;
    CSR_MICROSECONDS32           MaximumLatency;
    CSR_PERIODIC_SCHEDULING_MODE PeriodicSchedulingMode;
    s16                     WakeHost;
    CSR_PRIORITY                 UserPriority;
} CSR_MLME_ADD_PERIODIC_REQUEST;

typedef struct CSR_MLME_ADD_RX_TRIGGER_CONFIRM
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_TRIGGER_ID     TriggerId;
    CSR_RESULT_CODE    ResultCode;
} CSR_MLME_ADD_RX_TRIGGER_CONFIRM;

typedef struct CSR_MLME_ADD_RX_TRIGGER_REQUEST
{
    CSR_DATAREF        InformationElements;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_TRIGGER_ID     TriggerId;
    CSR_PRIORITY       Priority;
} CSR_MLME_ADD_RX_TRIGGER_REQUEST;

typedef struct CSR_MLME_ADD_TEMPLATE_CONFIRM
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_FRAME_TYPE     FrameType;
    CSR_RESULT_CODE    ResultCode;
} CSR_MLME_ADD_TEMPLATE_CONFIRM;

typedef struct CSR_MLME_ADD_TEMPLATE_REQUEST
{
    CSR_DATAREF        Data1;
    CSR_DATAREF        Data2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_FRAME_TYPE     FrameType;
    CSR_RATE           MinTransmitRate;
} CSR_MLME_ADD_TEMPLATE_REQUEST;

typedef struct CSR_MLME_ADD_TRIGGERED_GET_CONFIRM
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_RESULT_CODE    ResultCode;
    CSR_TRIGGERED_ID   TriggeredId;
} CSR_MLME_ADD_TRIGGERED_GET_CONFIRM;

typedef struct CSR_MLME_ADD_TRIGGERED_GET_REQUEST
{
    CSR_DATAREF        MibAttribute;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_TRIGGERED_ID   TriggeredId;
} CSR_MLME_ADD_TRIGGERED_GET_REQUEST;

typedef struct CSR_MLME_ADD_TSPEC_CONFIRM
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_PRIORITY       UserPriority;
    CSR_RESULT_CODE    ResultCode;
} CSR_MLME_ADD_TSPEC_CONFIRM;

typedef struct CSR_MLME_ADD_TSPEC_REQUEST
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_PRIORITY       UserPriority;
    CSR_DIRECTION      Direction;
    CSR_PS_SCHEME      PsScheme;
    CSR_NATURAL16      MediumTime;
    CSR_MICROSECONDS32 ServiceStartTime;
    CSR_MICROSECONDS32 ServiceInterval;
    CSR_RATE           MinimumDataRate;
} CSR_MLME_ADD_TSPEC_REQUEST;

typedef struct CSR_MLME_AUTONOMOUS_SCAN_DONE_INDICATION
{
    CSR_DATAREF            Dummydataref1;
    CSR_DATAREF            Dummydataref2;
    CSR_VIF_IDENTIFIER     VirtualInterfaceIdentifier;
    CSR_RESULT_CODE        ResultCode;
    CSR_AUTONOMOUS_SCAN_ID AutonomousScanId;
} CSR_MLME_AUTONOMOUS_SCAN_DONE_INDICATION;

typedef struct CSR_MLME_AUTONOMOUS_SCAN_LOSS_INDICATION
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_MACADDRESS     Bssid;
} CSR_MLME_AUTONOMOUS_SCAN_LOSS_INDICATION;

typedef struct CSR_MLME_BLACKOUT_ENDED_INDICATION
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_BLACKOUT_ID    BlackoutId;
} CSR_MLME_BLACKOUT_ENDED_INDICATION;

typedef struct CSR_MLME_BLOCKACK_ERROR_INDICATION
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_REASON_CODE    ResultCode;
    CSR_MACADDRESS     PeerQstaAddress;
} CSR_MLME_BLOCKACK_ERROR_INDICATION;

typedef struct CSR_MLME_CONNECTED_INDICATION
{
    CSR_DATAREF           Dummydataref1;
    CSR_DATAREF           Dummydataref2;
    CSR_VIF_IDENTIFIER    VirtualInterfaceIdentifier;
    CSR_CONNECTION_STATUS ConnectionStatus;
    CSR_MACADDRESS        PeerMacAddress;
} CSR_MLME_CONNECTED_INDICATION;

typedef struct CSR_MLME_CONNECT_STATUS_CONFIRM
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_RESULT_CODE    ResultCode;
} CSR_MLME_CONNECT_STATUS_CONFIRM;

typedef struct CSR_MLME_CONNECT_STATUS_REQUEST
{
    CSR_DATAREF                InformationElements;
    CSR_DATAREF                Dummydataref2;
    CSR_VIF_IDENTIFIER         VirtualInterfaceIdentifier;
    CSR_CONNECTION_STATUS      ConnectionStatus;
    CSR_MACADDRESS             StaAddress;
    CSR_ASSOCIATION_ID         AssociationId;
    CSR_CAPABILITY_INFORMATION AssociationCapabilityInformation;
} CSR_MLME_CONNECT_STATUS_REQUEST;

typedef struct CSR_MLME_DELETEKEYS_CONFIRM
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_RESULT_CODE    ResultCode;
} CSR_MLME_DELETEKEYS_CONFIRM;

typedef struct CSR_MLME_DELETEKEYS_REQUEST
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_NATURAL16      KeyId;
    CSR_KEY_TYPE       KeyType;
    CSR_MACADDRESS     Address;
} CSR_MLME_DELETEKEYS_REQUEST;

typedef struct CSR_MLME_DEL_AUTONOMOUS_SCAN_CONFIRM
{
    CSR_DATAREF            Dummydataref1;
    CSR_DATAREF            Dummydataref2;
    CSR_VIF_IDENTIFIER     VirtualInterfaceIdentifier;
    CSR_RESULT_CODE        ResultCode;
    CSR_AUTONOMOUS_SCAN_ID AutonomousScanId;
} CSR_MLME_DEL_AUTONOMOUS_SCAN_CONFIRM;

typedef struct CSR_MLME_DEL_AUTONOMOUS_SCAN_REQUEST
{
    CSR_DATAREF            Dummydataref1;
    CSR_DATAREF            Dummydataref2;
    CSR_VIF_IDENTIFIER     VirtualInterfaceIdentifier;
    CSR_AUTONOMOUS_SCAN_ID AutonomousScanId;
} CSR_MLME_DEL_AUTONOMOUS_SCAN_REQUEST;

typedef struct CSR_MLME_DEL_BLACKOUT_CONFIRM
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_BLACKOUT_ID    BlackoutId;
    CSR_RESULT_CODE    ResultCode;
} CSR_MLME_DEL_BLACKOUT_CONFIRM;

typedef struct CSR_MLME_DEL_BLACKOUT_REQUEST
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_BLACKOUT_ID    BlackoutId;
} CSR_MLME_DEL_BLACKOUT_REQUEST;

typedef struct CSR_MLME_DEL_PERIODIC_CONFIRM
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_PERIODIC_ID    PeriodicId;
    CSR_RESULT_CODE    ResultCode;
} CSR_MLME_DEL_PERIODIC_CONFIRM;

typedef struct CSR_MLME_DEL_PERIODIC_REQUEST
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_PERIODIC_ID    PeriodicId;
} CSR_MLME_DEL_PERIODIC_REQUEST;

typedef struct CSR_MLME_DEL_RX_TRIGGER_CONFIRM
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_TRIGGER_ID     TriggerId;
    CSR_RESULT_CODE    ResultCode;
} CSR_MLME_DEL_RX_TRIGGER_CONFIRM;

typedef struct CSR_MLME_DEL_RX_TRIGGER_REQUEST
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_TRIGGER_ID     TriggerId;
} CSR_MLME_DEL_RX_TRIGGER_REQUEST;

typedef struct CSR_MLME_DEL_TRIGGERED_GET_CONFIRM
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_RESULT_CODE    ResultCode;
    CSR_TRIGGERED_ID   TriggeredId;
} CSR_MLME_DEL_TRIGGERED_GET_CONFIRM;

typedef struct CSR_MLME_DEL_TRIGGERED_GET_REQUEST
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_TRIGGERED_ID   TriggeredId;
} CSR_MLME_DEL_TRIGGERED_GET_REQUEST;

typedef struct CSR_MLME_DEL_TSPEC_CONFIRM
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_PRIORITY       UserPriority;
    CSR_RESULT_CODE    ResultCode;
} CSR_MLME_DEL_TSPEC_CONFIRM;

typedef struct CSR_MLME_DEL_TSPEC_REQUEST
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_PRIORITY       UserPriority;
    CSR_DIRECTION      Direction;
} CSR_MLME_DEL_TSPEC_REQUEST;

typedef struct CSR_MLME_GET_KEY_SEQUENCE_CONFIRM
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_RESULT_CODE    ResultCode;
    CSR_NATURAL16      SequenceNumber[8];
} CSR_MLME_GET_KEY_SEQUENCE_CONFIRM;

typedef struct CSR_MLME_GET_KEY_SEQUENCE_REQUEST
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_NATURAL16      KeyId;
    CSR_KEY_TYPE       KeyType;
    CSR_MACADDRESS     Address;
} CSR_MLME_GET_KEY_SEQUENCE_REQUEST;

typedef struct CSR_MLME_LEAVE_CONFIRM
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_RESULT_CODE    ResultCode;
} CSR_MLME_LEAVE_CONFIRM;

typedef struct CSR_MLME_LEAVE_REQUEST
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
} CSR_MLME_LEAVE_REQUEST;

typedef struct CSR_MLME_MODIFY_BSS_PARAMETER_CONFIRM
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_RESULT_CODE    ResultCode;
} CSR_MLME_MODIFY_BSS_PARAMETER_CONFIRM;

typedef struct CSR_MLME_MODIFY_BSS_PARAMETER_REQUEST
{
    CSR_DATAREF                Data;
    CSR_DATAREF                Dummydataref2;
    CSR_VIF_IDENTIFIER         VirtualInterfaceIdentifier;
    CSR_TIME_UNITS             BeaconPeriod;
    CSR_BEACON_PERIODS         DtimPeriod;
    CSR_CAPABILITY_INFORMATION CapabilityInformation;
    CSR_MACADDRESS             Bssid;
    CSR_NATURAL16              RtsThreshold;
} CSR_MLME_MODIFY_BSS_PARAMETER_REQUEST;

typedef struct CSR_MLME_PAUSE_AUTONOMOUS_SCAN_CONFIRM
{
    CSR_DATAREF            Dummydataref1;
    CSR_DATAREF            Dummydataref2;
    CSR_VIF_IDENTIFIER     VirtualInterfaceIdentifier;
    CSR_RESULT_CODE        ResultCode;
    CSR_AUTONOMOUS_SCAN_ID AutonomousScanId;
} CSR_MLME_PAUSE_AUTONOMOUS_SCAN_CONFIRM;

typedef struct CSR_MLME_PAUSE_AUTONOMOUS_SCAN_REQUEST
{
    CSR_DATAREF            Dummydataref1;
    CSR_DATAREF            Dummydataref2;
    CSR_VIF_IDENTIFIER     VirtualInterfaceIdentifier;
    CSR_AUTONOMOUS_SCAN_ID AutonomousScanId;
    s16               Pause;
} CSR_MLME_PAUSE_AUTONOMOUS_SCAN_REQUEST;

typedef struct CSR_MLME_POWERMGT_CONFIRM
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_RESULT_CODE    ResultCode;
} CSR_MLME_POWERMGT_CONFIRM;

typedef struct CSR_MLME_POWERMGT_REQUEST
{
    CSR_DATAREF               Dummydataref1;
    CSR_DATAREF               Dummydataref2;
    CSR_VIF_IDENTIFIER        VirtualInterfaceIdentifier;
    CSR_POWER_MANAGEMENT_MODE PowerManagementMode;
    s16                  ReceiveDtims;
    CSR_BEACON_PERIODS        ListenInterval;
    CSR_TRAFFIC_WINDOW        TrafficWindow;
} CSR_MLME_POWERMGT_REQUEST;

typedef struct CSR_MLME_SCAN_CONFIRM
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_RESULT_CODE    ResultCode;
} CSR_MLME_SCAN_CONFIRM;

typedef struct CSR_MLME_SCAN_REQUEST
{
    CSR_DATAREF        ChannelList;
    CSR_DATAREF        InformationElements;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_IFINTERFACE    Ifindex;
    CSR_SCAN_TYPE      ScanType;
    CSR_MICROSECONDS32 ProbeDelay;
    CSR_TIME_UNITS     MinChannelTime;
    CSR_TIME_UNITS     MaxChannelTime;
} CSR_MLME_SCAN_REQUEST;

typedef struct CSR_MLME_SCAN_CANCEL_REQUEST
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
} CSR_MLME_SCAN_CANCEL_REQUEST;

typedef struct CSR_MLME_SETKEYS_CONFIRM
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_RESULT_CODE    ResultCode;
} CSR_MLME_SETKEYS_CONFIRM;

typedef struct CSR_MLME_SETKEYS_REQUEST
{
    CSR_DATAREF               Key;
    CSR_DATAREF               Dummydataref2;
    CSR_VIF_IDENTIFIER        VirtualInterfaceIdentifier;
    CSR_NATURAL16             Length;
    CSR_NATURAL16             KeyId;
    CSR_KEY_TYPE              KeyType;
    CSR_MACADDRESS            Address;
    CSR_NATURAL16             SequenceNumber[8];
    CSR_CIPHER_SUITE_SELECTOR CipherSuiteSelector;
} CSR_MLME_SETKEYS_REQUEST;

typedef struct CSR_MLME_SET_CHANNEL_CONFIRM
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_RESULT_CODE    ResultCode;
} CSR_MLME_SET_CHANNEL_CONFIRM;

typedef struct CSR_MLME_SET_CHANNEL_REQUEST
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_IFINTERFACE    Ifindex;
    CSR_CHANNEL_NUMBER Channel;
    CSR_MACADDRESS     Address;
    CSR_TIME_UNITS     AvailabilityDuration;
    CSR_TIME_UNITS     AvailabilityInterval;
} CSR_MLME_SET_CHANNEL_REQUEST;

typedef struct CSR_MLME_SET_PACKET_FILTER_CONFIRM
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_RESULT_CODE    ResultCode;
} CSR_MLME_SET_PACKET_FILTER_CONFIRM;

typedef struct CSR_MLME_SET_PACKET_FILTER_REQUEST
{
    CSR_DATAREF            InformationElements;
    CSR_DATAREF            Dummydataref2;
    CSR_VIF_IDENTIFIER     VirtualInterfaceIdentifier;
    CSR_PACKET_FILTER_MODE PacketFilterMode;
    CSR_IPV4_ADDRESS       ArpFilterAddress;
} CSR_MLME_SET_PACKET_FILTER_REQUEST;

typedef struct CSR_MLME_SET_TIM_CONFIRM
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_RESULT_CODE    ResultCode;
} CSR_MLME_SET_TIM_CONFIRM;

typedef struct CSR_MLME_SET_TIM_REQUEST
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_ASSOCIATION_ID AssociationId;
    s16           TimValue;
} CSR_MLME_SET_TIM_REQUEST;

typedef struct CSR_MLME_SM_START_CONFIRM
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_RESULT_CODE    ResultCode;
} CSR_MLME_SM_START_CONFIRM;

typedef struct CSR_MLME_SM_START_REQUEST
{
    CSR_DATAREF                Beacon;
    CSR_DATAREF                BssParameters;
    CSR_VIF_IDENTIFIER         VirtualInterfaceIdentifier;
    CSR_IFINTERFACE            Ifindex;
    CSR_CHANNEL_NUMBER         Channel;
    CSR_MACADDRESS             InterfaceAddress;
    CSR_MACADDRESS             Bssid;
    CSR_TIME_UNITS             BeaconPeriod;
    CSR_BEACON_PERIODS         DtimPeriod;
    CSR_CAPABILITY_INFORMATION CapabilityInformation;
} CSR_MLME_SM_START_REQUEST;

typedef struct CSR_MLME_START_AGGREGATION_CONFIRM
{
    CSR_DATAREF         Dummydataref1;
    CSR_DATAREF         Dummydataref2;
    CSR_VIF_IDENTIFIER  VirtualInterfaceIdentifier;
    CSR_MACADDRESS      PeerQstaAddress;
    CSR_PRIORITY        UserPriority;
    CSR_DIRECTION       Direction;
    CSR_RESULT_CODE     ResultCode;
    CSR_SEQUENCE_NUMBER SequenceNumber;
} CSR_MLME_START_AGGREGATION_CONFIRM;

typedef struct CSR_MLME_START_AGGREGATION_REQUEST
{
    CSR_DATAREF                  Dummydataref1;
    CSR_DATAREF                  Dummydataref2;
    CSR_VIF_IDENTIFIER           VirtualInterfaceIdentifier;
    CSR_MACADDRESS               PeerQstaAddress;
    CSR_PRIORITY                 UserPriority;
    CSR_DIRECTION                Direction;
    CSR_STARTING_SEQUENCE_NUMBER StartingSequenceNumber;
    CSR_NATURAL16                BufferSize;
    CSR_TIME_UNITS               BlockAckTimeout;
} CSR_MLME_START_AGGREGATION_REQUEST;

typedef struct CSR_MLME_STOP_AGGREGATION_CONFIRM
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_MACADDRESS     PeerQstaAddress;
    CSR_PRIORITY       UserPriority;
    CSR_DIRECTION      Direction;
    CSR_RESULT_CODE    ResultCode;
} CSR_MLME_STOP_AGGREGATION_CONFIRM;

typedef struct CSR_MLME_STOP_AGGREGATION_REQUEST
{
    CSR_DATAREF        Dummydataref1;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_MACADDRESS     PeerQstaAddress;
    CSR_PRIORITY       UserPriority;
    CSR_DIRECTION      Direction;
} CSR_MLME_STOP_AGGREGATION_REQUEST;

typedef struct CSR_MLME_TRIGGERED_GET_INDICATION
{
    CSR_DATAREF        MibAttributeValue;
    CSR_DATAREF        Dummydataref2;
    CSR_VIF_IDENTIFIER VirtualInterfaceIdentifier;
    CSR_MIB_STATUS     Status;
    CSR_NATURAL16      ErrorIndex;
    CSR_TRIGGERED_ID   TriggeredId;
} CSR_MLME_TRIGGERED_GET_INDICATION;

typedef struct CSR_SIGNAL_PRIMITIVE
{
    CSR_SIGNAL_PRIMITIVE_HEADER SignalPrimitiveHeader;
    union
    {
        CSR_MA_PACKET_REQUEST                    MaPacketRequest;
        CSR_MA_PACKET_CONFIRM                    MaPacketConfirm;
        CSR_MA_PACKET_INDICATION                 MaPacketIndication;
        CSR_MA_PACKET_CANCEL_REQUEST             MaPacketCancelRequest;
        CSR_MA_VIF_AVAILABILITY_RESPONSE         MaVifAvailabilityResponse;
        CSR_MA_VIF_AVAILABILITY_INDICATION       MaVifAvailabilityIndication;
        CSR_MA_PACKET_ERROR_INDICATION           MaPacketErrorIndication;
        CSR_MLME_RESET_REQUEST                   MlmeResetRequest;
        CSR_MLME_RESET_CONFIRM                   MlmeResetConfirm;
        CSR_MLME_GET_REQUEST                     MlmeGetRequest;
        CSR_MLME_GET_CONFIRM                     MlmeGetConfirm;
        CSR_MLME_SET_REQUEST                     MlmeSetRequest;
        CSR_MLME_SET_CONFIRM                     MlmeSetConfirm;
        CSR_MLME_GET_NEXT_REQUEST                MlmeGetNextRequest;
        CSR_MLME_GET_NEXT_CONFIRM                MlmeGetNextConfirm;
        CSR_MLME_POWERMGT_REQUEST                MlmePowermgtRequest;
        CSR_MLME_POWERMGT_CONFIRM                MlmePowermgtConfirm;
        CSR_MLME_SCAN_REQUEST                    MlmeScanRequest;
        CSR_MLME_SCAN_CONFIRM                    MlmeScanConfirm;
        CSR_MLME_HL_SYNC_REQUEST                 MlmeHlSyncRequest;
        CSR_MLME_HL_SYNC_CONFIRM                 MlmeHlSyncConfirm;
        CSR_MLME_MEASURE_REQUEST                 MlmeMeasureRequest;
        CSR_MLME_MEASURE_CONFIRM                 MlmeMeasureConfirm;
        CSR_MLME_MEASURE_INDICATION              MlmeMeasureIndication;
        CSR_MLME_SETKEYS_REQUEST                 MlmeSetkeysRequest;
        CSR_MLME_SETKEYS_CONFIRM                 MlmeSetkeysConfirm;
        CSR_MLME_DELETEKEYS_REQUEST              MlmeDeletekeysRequest;
        CSR_MLME_DELETEKEYS_CONFIRM              MlmeDeletekeysConfirm;
        CSR_MLME_AUTONOMOUS_SCAN_LOSS_INDICATION MlmeAutonomousScanLossIndication;
        CSR_MLME_CONNECTED_INDICATION            MlmeConnectedIndication;
        CSR_MLME_SCAN_CANCEL_REQUEST             MlmeScanCancelRequest;
        CSR_MLME_HL_SYNC_CANCEL_REQUEST          MlmeHlSyncCancelRequest;
        CSR_MLME_HL_SYNC_CANCEL_CONFIRM          MlmeHlSyncCancelConfirm;
        CSR_MLME_ADD_PERIODIC_REQUEST            MlmeAddPeriodicRequest;
        CSR_MLME_ADD_PERIODIC_CONFIRM            MlmeAddPeriodicConfirm;
        CSR_MLME_DEL_PERIODIC_REQUEST            MlmeDelPeriodicRequest;
        CSR_MLME_DEL_PERIODIC_CONFIRM            MlmeDelPeriodicConfirm;
        CSR_MLME_ADD_AUTONOMOUS_SCAN_REQUEST     MlmeAddAutonomousScanRequest;
        CSR_MLME_ADD_AUTONOMOUS_SCAN_CONFIRM     MlmeAddAutonomousScanConfirm;
        CSR_MLME_DEL_AUTONOMOUS_SCAN_REQUEST     MlmeDelAutonomousScanRequest;
        CSR_MLME_DEL_AUTONOMOUS_SCAN_CONFIRM     MlmeDelAutonomousScanConfirm;
        CSR_MLME_SET_PACKET_FILTER_REQUEST       MlmeSetPacketFilterRequest;
        CSR_MLME_SET_PACKET_FILTER_CONFIRM       MlmeSetPacketFilterConfirm;
        CSR_MLME_STOP_MEASURE_REQUEST            MlmeStopMeasureRequest;
        CSR_MLME_STOP_MEASURE_CONFIRM            MlmeStopMeasureConfirm;
        CSR_MLME_PAUSE_AUTONOMOUS_SCAN_REQUEST   MlmePauseAutonomousScanRequest;
        CSR_MLME_PAUSE_AUTONOMOUS_SCAN_CONFIRM   MlmePauseAutonomousScanConfirm;
        CSR_MLME_AUTONOMOUS_SCAN_DONE_INDICATION MlmeAutonomousScanDoneIndication;
        CSR_MLME_ADD_TRIGGERED_GET_REQUEST       MlmeAddTriggeredGetRequest;
        CSR_MLME_ADD_TRIGGERED_GET_CONFIRM       MlmeAddTriggeredGetConfirm;
        CSR_MLME_DEL_TRIGGERED_GET_REQUEST       MlmeDelTriggeredGetRequest;
        CSR_MLME_DEL_TRIGGERED_GET_CONFIRM       MlmeDelTriggeredGetConfirm;
        CSR_MLME_TRIGGERED_GET_INDICATION        MlmeTriggeredGetIndication;
        CSR_MLME_ADD_BLACKOUT_REQUEST            MlmeAddBlackoutRequest;
        CSR_MLME_ADD_BLACKOUT_CONFIRM            MlmeAddBlackoutConfirm;
        CSR_MLME_BLACKOUT_ENDED_INDICATION       MlmeBlackoutEndedIndication;
        CSR_MLME_DEL_BLACKOUT_REQUEST            MlmeDelBlackoutRequest;
        CSR_MLME_DEL_BLACKOUT_CONFIRM            MlmeDelBlackoutConfirm;
        CSR_MLME_ADD_RX_TRIGGER_REQUEST          MlmeAddRxTriggerRequest;
        CSR_MLME_ADD_RX_TRIGGER_CONFIRM          MlmeAddRxTriggerConfirm;
        CSR_MLME_DEL_RX_TRIGGER_REQUEST          MlmeDelRxTriggerRequest;
        CSR_MLME_DEL_RX_TRIGGER_CONFIRM          MlmeDelRxTriggerConfirm;
        CSR_MLME_CONNECT_STATUS_REQUEST          MlmeConnectStatusRequest;
        CSR_MLME_CONNECT_STATUS_CONFIRM          MlmeConnectStatusConfirm;
        CSR_MLME_MODIFY_BSS_PARAMETER_REQUEST    MlmeModifyBssParameterRequest;
        CSR_MLME_MODIFY_BSS_PARAMETER_CONFIRM    MlmeModifyBssParameterConfirm;
        CSR_MLME_ADD_TEMPLATE_REQUEST            MlmeAddTemplateRequest;
        CSR_MLME_ADD_TEMPLATE_CONFIRM            MlmeAddTemplateConfirm;
        CSR_MLME_CONFIG_QUEUE_REQUEST            MlmeConfigQueueRequest;
        CSR_MLME_CONFIG_QUEUE_CONFIRM            MlmeConfigQueueConfirm;
        CSR_MLME_ADD_TSPEC_REQUEST               MlmeAddTspecRequest;
        CSR_MLME_ADD_TSPEC_CONFIRM               MlmeAddTspecConfirm;
        CSR_MLME_DEL_TSPEC_REQUEST               MlmeDelTspecRequest;
        CSR_MLME_DEL_TSPEC_CONFIRM               MlmeDelTspecConfirm;
        CSR_MLME_START_AGGREGATION_REQUEST       MlmeStartAggregationRequest;
        CSR_MLME_START_AGGREGATION_CONFIRM       MlmeStartAggregationConfirm;
        CSR_MLME_BLOCKACK_ERROR_INDICATION       MlmeBlockackErrorIndication;
        CSR_MLME_STOP_AGGREGATION_REQUEST        MlmeStopAggregationRequest;
        CSR_MLME_STOP_AGGREGATION_CONFIRM        MlmeStopAggregationConfirm;
        CSR_MLME_SM_START_REQUEST                MlmeSmStartRequest;
        CSR_MLME_SM_START_CONFIRM                MlmeSmStartConfirm;
        CSR_MLME_LEAVE_REQUEST                   MlmeLeaveRequest;
        CSR_MLME_LEAVE_CONFIRM                   MlmeLeaveConfirm;
        CSR_MLME_SET_TIM_REQUEST                 MlmeSetTimRequest;
        CSR_MLME_SET_TIM_CONFIRM                 MlmeSetTimConfirm;
        CSR_MLME_GET_KEY_SEQUENCE_REQUEST        MlmeGetKeySequenceRequest;
        CSR_MLME_GET_KEY_SEQUENCE_CONFIRM        MlmeGetKeySequenceConfirm;
        CSR_MLME_SET_CHANNEL_REQUEST             MlmeSetChannelRequest;
        CSR_MLME_SET_CHANNEL_CONFIRM             MlmeSetChannelConfirm;
        CSR_MLME_ADD_MULTICAST_ADDRESS_REQUEST   MlmeAddMulticastAddressRequest;
        CSR_MLME_ADD_MULTICAST_ADDRESS_CONFIRM   MlmeAddMulticastAddressConfirm;
        CSR_DEBUG_STRING_INDICATION              DebugStringIndication;
        CSR_DEBUG_WORD16_INDICATION              DebugWord16Indication;
        CSR_DEBUG_GENERIC_REQUEST                DebugGenericRequest;
        CSR_DEBUG_GENERIC_CONFIRM                DebugGenericConfirm;
        CSR_DEBUG_GENERIC_INDICATION             DebugGenericIndication;
    } u;
} CSR_SIGNAL;

#define SIG_FILTER_SIZE 6

u32 SigGetFilterPos(u16 aSigID);

#endif
