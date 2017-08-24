/*
 *  Siano core API module
 *
 *  This file contains implementation for the interface to sms core component
 *
 *  author: Uri Shkolnik
 *
 *  Copyright (c), 2005-2008 Siano Mobile Silicon, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation;
 *
 *  Software distributed under the License is distributed on an "AS IS"
 *  basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.
 *
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/slab.h>

#include <linux/firmware.h>
#include <linux/wait.h>
#include <asm/byteorder.h>

#include "smscoreapi.h"
#include "sms-cards.h"
#include "smsir.h"

static int sms_dbg;
module_param_named(debug, sms_dbg, int, 0644);
MODULE_PARM_DESC(debug, "set debug level (info=1, adv=2 (or-able))");

struct smscore_device_notifyee_t {
	struct list_head entry;
	hotplug_t hotplug;
};

struct smscore_idlist_t {
	struct list_head entry;
	int		id;
	int		data_type;
};

struct smscore_client_t {
	struct list_head entry;
	struct smscore_device_t *coredev;
	void			*context;
	struct list_head	idlist;
	onresponse_t		onresponse_handler;
	onremove_t		onremove_handler;
};

static char *siano_msgs[] = {
	[MSG_TYPE_BASE_VAL                           - MSG_TYPE_BASE_VAL] = "MSG_TYPE_BASE_VAL",
	[MSG_SMS_GET_VERSION_REQ                     - MSG_TYPE_BASE_VAL] = "MSG_SMS_GET_VERSION_REQ",
	[MSG_SMS_GET_VERSION_RES                     - MSG_TYPE_BASE_VAL] = "MSG_SMS_GET_VERSION_RES",
	[MSG_SMS_MULTI_BRIDGE_CFG                    - MSG_TYPE_BASE_VAL] = "MSG_SMS_MULTI_BRIDGE_CFG",
	[MSG_SMS_GPIO_CONFIG_REQ                     - MSG_TYPE_BASE_VAL] = "MSG_SMS_GPIO_CONFIG_REQ",
	[MSG_SMS_GPIO_CONFIG_RES                     - MSG_TYPE_BASE_VAL] = "MSG_SMS_GPIO_CONFIG_RES",
	[MSG_SMS_GPIO_SET_LEVEL_REQ                  - MSG_TYPE_BASE_VAL] = "MSG_SMS_GPIO_SET_LEVEL_REQ",
	[MSG_SMS_GPIO_SET_LEVEL_RES                  - MSG_TYPE_BASE_VAL] = "MSG_SMS_GPIO_SET_LEVEL_RES",
	[MSG_SMS_GPIO_GET_LEVEL_REQ                  - MSG_TYPE_BASE_VAL] = "MSG_SMS_GPIO_GET_LEVEL_REQ",
	[MSG_SMS_GPIO_GET_LEVEL_RES                  - MSG_TYPE_BASE_VAL] = "MSG_SMS_GPIO_GET_LEVEL_RES",
	[MSG_SMS_EEPROM_BURN_IND                     - MSG_TYPE_BASE_VAL] = "MSG_SMS_EEPROM_BURN_IND",
	[MSG_SMS_LOG_ENABLE_CHANGE_REQ               - MSG_TYPE_BASE_VAL] = "MSG_SMS_LOG_ENABLE_CHANGE_REQ",
	[MSG_SMS_LOG_ENABLE_CHANGE_RES               - MSG_TYPE_BASE_VAL] = "MSG_SMS_LOG_ENABLE_CHANGE_RES",
	[MSG_SMS_SET_MAX_TX_MSG_LEN_REQ              - MSG_TYPE_BASE_VAL] = "MSG_SMS_SET_MAX_TX_MSG_LEN_REQ",
	[MSG_SMS_SET_MAX_TX_MSG_LEN_RES              - MSG_TYPE_BASE_VAL] = "MSG_SMS_SET_MAX_TX_MSG_LEN_RES",
	[MSG_SMS_SPI_HALFDUPLEX_TOKEN_HOST_TO_DEVICE - MSG_TYPE_BASE_VAL] = "MSG_SMS_SPI_HALFDUPLEX_TOKEN_HOST_TO_DEVICE",
	[MSG_SMS_SPI_HALFDUPLEX_TOKEN_DEVICE_TO_HOST - MSG_TYPE_BASE_VAL] = "MSG_SMS_SPI_HALFDUPLEX_TOKEN_DEVICE_TO_HOST",
	[MSG_SMS_BACKGROUND_SCAN_FLAG_CHANGE_REQ     - MSG_TYPE_BASE_VAL] = "MSG_SMS_BACKGROUND_SCAN_FLAG_CHANGE_REQ",
	[MSG_SMS_BACKGROUND_SCAN_FLAG_CHANGE_RES     - MSG_TYPE_BASE_VAL] = "MSG_SMS_BACKGROUND_SCAN_FLAG_CHANGE_RES",
	[MSG_SMS_BACKGROUND_SCAN_SIGNAL_DETECTED_IND - MSG_TYPE_BASE_VAL] = "MSG_SMS_BACKGROUND_SCAN_SIGNAL_DETECTED_IND",
	[MSG_SMS_BACKGROUND_SCAN_NO_SIGNAL_IND       - MSG_TYPE_BASE_VAL] = "MSG_SMS_BACKGROUND_SCAN_NO_SIGNAL_IND",
	[MSG_SMS_CONFIGURE_RF_SWITCH_REQ             - MSG_TYPE_BASE_VAL] = "MSG_SMS_CONFIGURE_RF_SWITCH_REQ",
	[MSG_SMS_CONFIGURE_RF_SWITCH_RES             - MSG_TYPE_BASE_VAL] = "MSG_SMS_CONFIGURE_RF_SWITCH_RES",
	[MSG_SMS_MRC_PATH_DISCONNECT_REQ             - MSG_TYPE_BASE_VAL] = "MSG_SMS_MRC_PATH_DISCONNECT_REQ",
	[MSG_SMS_MRC_PATH_DISCONNECT_RES             - MSG_TYPE_BASE_VAL] = "MSG_SMS_MRC_PATH_DISCONNECT_RES",
	[MSG_SMS_RECEIVE_1SEG_THROUGH_FULLSEG_REQ    - MSG_TYPE_BASE_VAL] = "MSG_SMS_RECEIVE_1SEG_THROUGH_FULLSEG_REQ",
	[MSG_SMS_RECEIVE_1SEG_THROUGH_FULLSEG_RES    - MSG_TYPE_BASE_VAL] = "MSG_SMS_RECEIVE_1SEG_THROUGH_FULLSEG_RES",
	[MSG_SMS_RECEIVE_VHF_VIA_VHF_INPUT_REQ       - MSG_TYPE_BASE_VAL] = "MSG_SMS_RECEIVE_VHF_VIA_VHF_INPUT_REQ",
	[MSG_SMS_RECEIVE_VHF_VIA_VHF_INPUT_RES       - MSG_TYPE_BASE_VAL] = "MSG_SMS_RECEIVE_VHF_VIA_VHF_INPUT_RES",
	[MSG_WR_REG_RFT_REQ                          - MSG_TYPE_BASE_VAL] = "MSG_WR_REG_RFT_REQ",
	[MSG_WR_REG_RFT_RES                          - MSG_TYPE_BASE_VAL] = "MSG_WR_REG_RFT_RES",
	[MSG_RD_REG_RFT_REQ                          - MSG_TYPE_BASE_VAL] = "MSG_RD_REG_RFT_REQ",
	[MSG_RD_REG_RFT_RES                          - MSG_TYPE_BASE_VAL] = "MSG_RD_REG_RFT_RES",
	[MSG_RD_REG_ALL_RFT_REQ                      - MSG_TYPE_BASE_VAL] = "MSG_RD_REG_ALL_RFT_REQ",
	[MSG_RD_REG_ALL_RFT_RES                      - MSG_TYPE_BASE_VAL] = "MSG_RD_REG_ALL_RFT_RES",
	[MSG_HELP_INT                                - MSG_TYPE_BASE_VAL] = "MSG_HELP_INT",
	[MSG_RUN_SCRIPT_INT                          - MSG_TYPE_BASE_VAL] = "MSG_RUN_SCRIPT_INT",
	[MSG_SMS_EWS_INBAND_REQ                      - MSG_TYPE_BASE_VAL] = "MSG_SMS_EWS_INBAND_REQ",
	[MSG_SMS_EWS_INBAND_RES                      - MSG_TYPE_BASE_VAL] = "MSG_SMS_EWS_INBAND_RES",
	[MSG_SMS_RFS_SELECT_REQ                      - MSG_TYPE_BASE_VAL] = "MSG_SMS_RFS_SELECT_REQ",
	[MSG_SMS_RFS_SELECT_RES                      - MSG_TYPE_BASE_VAL] = "MSG_SMS_RFS_SELECT_RES",
	[MSG_SMS_MB_GET_VER_REQ                      - MSG_TYPE_BASE_VAL] = "MSG_SMS_MB_GET_VER_REQ",
	[MSG_SMS_MB_GET_VER_RES                      - MSG_TYPE_BASE_VAL] = "MSG_SMS_MB_GET_VER_RES",
	[MSG_SMS_MB_WRITE_CFGFILE_REQ                - MSG_TYPE_BASE_VAL] = "MSG_SMS_MB_WRITE_CFGFILE_REQ",
	[MSG_SMS_MB_WRITE_CFGFILE_RES                - MSG_TYPE_BASE_VAL] = "MSG_SMS_MB_WRITE_CFGFILE_RES",
	[MSG_SMS_MB_READ_CFGFILE_REQ                 - MSG_TYPE_BASE_VAL] = "MSG_SMS_MB_READ_CFGFILE_REQ",
	[MSG_SMS_MB_READ_CFGFILE_RES                 - MSG_TYPE_BASE_VAL] = "MSG_SMS_MB_READ_CFGFILE_RES",
	[MSG_SMS_RD_MEM_REQ                          - MSG_TYPE_BASE_VAL] = "MSG_SMS_RD_MEM_REQ",
	[MSG_SMS_RD_MEM_RES                          - MSG_TYPE_BASE_VAL] = "MSG_SMS_RD_MEM_RES",
	[MSG_SMS_WR_MEM_REQ                          - MSG_TYPE_BASE_VAL] = "MSG_SMS_WR_MEM_REQ",
	[MSG_SMS_WR_MEM_RES                          - MSG_TYPE_BASE_VAL] = "MSG_SMS_WR_MEM_RES",
	[MSG_SMS_UPDATE_MEM_REQ                      - MSG_TYPE_BASE_VAL] = "MSG_SMS_UPDATE_MEM_REQ",
	[MSG_SMS_UPDATE_MEM_RES                      - MSG_TYPE_BASE_VAL] = "MSG_SMS_UPDATE_MEM_RES",
	[MSG_SMS_ISDBT_ENABLE_FULL_PARAMS_SET_REQ    - MSG_TYPE_BASE_VAL] = "MSG_SMS_ISDBT_ENABLE_FULL_PARAMS_SET_REQ",
	[MSG_SMS_ISDBT_ENABLE_FULL_PARAMS_SET_RES    - MSG_TYPE_BASE_VAL] = "MSG_SMS_ISDBT_ENABLE_FULL_PARAMS_SET_RES",
	[MSG_SMS_RF_TUNE_REQ                         - MSG_TYPE_BASE_VAL] = "MSG_SMS_RF_TUNE_REQ",
	[MSG_SMS_RF_TUNE_RES                         - MSG_TYPE_BASE_VAL] = "MSG_SMS_RF_TUNE_RES",
	[MSG_SMS_ISDBT_ENABLE_HIGH_MOBILITY_REQ      - MSG_TYPE_BASE_VAL] = "MSG_SMS_ISDBT_ENABLE_HIGH_MOBILITY_REQ",
	[MSG_SMS_ISDBT_ENABLE_HIGH_MOBILITY_RES      - MSG_TYPE_BASE_VAL] = "MSG_SMS_ISDBT_ENABLE_HIGH_MOBILITY_RES",
	[MSG_SMS_ISDBT_SB_RECEPTION_REQ              - MSG_TYPE_BASE_VAL] = "MSG_SMS_ISDBT_SB_RECEPTION_REQ",
	[MSG_SMS_ISDBT_SB_RECEPTION_RES              - MSG_TYPE_BASE_VAL] = "MSG_SMS_ISDBT_SB_RECEPTION_RES",
	[MSG_SMS_GENERIC_EPROM_WRITE_REQ             - MSG_TYPE_BASE_VAL] = "MSG_SMS_GENERIC_EPROM_WRITE_REQ",
	[MSG_SMS_GENERIC_EPROM_WRITE_RES             - MSG_TYPE_BASE_VAL] = "MSG_SMS_GENERIC_EPROM_WRITE_RES",
	[MSG_SMS_GENERIC_EPROM_READ_REQ              - MSG_TYPE_BASE_VAL] = "MSG_SMS_GENERIC_EPROM_READ_REQ",
	[MSG_SMS_GENERIC_EPROM_READ_RES              - MSG_TYPE_BASE_VAL] = "MSG_SMS_GENERIC_EPROM_READ_RES",
	[MSG_SMS_EEPROM_WRITE_REQ                    - MSG_TYPE_BASE_VAL] = "MSG_SMS_EEPROM_WRITE_REQ",
	[MSG_SMS_EEPROM_WRITE_RES                    - MSG_TYPE_BASE_VAL] = "MSG_SMS_EEPROM_WRITE_RES",
	[MSG_SMS_CUSTOM_READ_REQ                     - MSG_TYPE_BASE_VAL] = "MSG_SMS_CUSTOM_READ_REQ",
	[MSG_SMS_CUSTOM_READ_RES                     - MSG_TYPE_BASE_VAL] = "MSG_SMS_CUSTOM_READ_RES",
	[MSG_SMS_CUSTOM_WRITE_REQ                    - MSG_TYPE_BASE_VAL] = "MSG_SMS_CUSTOM_WRITE_REQ",
	[MSG_SMS_CUSTOM_WRITE_RES                    - MSG_TYPE_BASE_VAL] = "MSG_SMS_CUSTOM_WRITE_RES",
	[MSG_SMS_INIT_DEVICE_REQ                     - MSG_TYPE_BASE_VAL] = "MSG_SMS_INIT_DEVICE_REQ",
	[MSG_SMS_INIT_DEVICE_RES                     - MSG_TYPE_BASE_VAL] = "MSG_SMS_INIT_DEVICE_RES",
	[MSG_SMS_ATSC_SET_ALL_IP_REQ                 - MSG_TYPE_BASE_VAL] = "MSG_SMS_ATSC_SET_ALL_IP_REQ",
	[MSG_SMS_ATSC_SET_ALL_IP_RES                 - MSG_TYPE_BASE_VAL] = "MSG_SMS_ATSC_SET_ALL_IP_RES",
	[MSG_SMS_ATSC_START_ENSEMBLE_REQ             - MSG_TYPE_BASE_VAL] = "MSG_SMS_ATSC_START_ENSEMBLE_REQ",
	[MSG_SMS_ATSC_START_ENSEMBLE_RES             - MSG_TYPE_BASE_VAL] = "MSG_SMS_ATSC_START_ENSEMBLE_RES",
	[MSG_SMS_SET_OUTPUT_MODE_REQ                 - MSG_TYPE_BASE_VAL] = "MSG_SMS_SET_OUTPUT_MODE_REQ",
	[MSG_SMS_SET_OUTPUT_MODE_RES                 - MSG_TYPE_BASE_VAL] = "MSG_SMS_SET_OUTPUT_MODE_RES",
	[MSG_SMS_ATSC_IP_FILTER_GET_LIST_REQ         - MSG_TYPE_BASE_VAL] = "MSG_SMS_ATSC_IP_FILTER_GET_LIST_REQ",
	[MSG_SMS_ATSC_IP_FILTER_GET_LIST_RES         - MSG_TYPE_BASE_VAL] = "MSG_SMS_ATSC_IP_FILTER_GET_LIST_RES",
	[MSG_SMS_SUB_CHANNEL_START_REQ               - MSG_TYPE_BASE_VAL] = "MSG_SMS_SUB_CHANNEL_START_REQ",
	[MSG_SMS_SUB_CHANNEL_START_RES               - MSG_TYPE_BASE_VAL] = "MSG_SMS_SUB_CHANNEL_START_RES",
	[MSG_SMS_SUB_CHANNEL_STOP_REQ                - MSG_TYPE_BASE_VAL] = "MSG_SMS_SUB_CHANNEL_STOP_REQ",
	[MSG_SMS_SUB_CHANNEL_STOP_RES                - MSG_TYPE_BASE_VAL] = "MSG_SMS_SUB_CHANNEL_STOP_RES",
	[MSG_SMS_ATSC_IP_FILTER_ADD_REQ              - MSG_TYPE_BASE_VAL] = "MSG_SMS_ATSC_IP_FILTER_ADD_REQ",
	[MSG_SMS_ATSC_IP_FILTER_ADD_RES              - MSG_TYPE_BASE_VAL] = "MSG_SMS_ATSC_IP_FILTER_ADD_RES",
	[MSG_SMS_ATSC_IP_FILTER_REMOVE_REQ           - MSG_TYPE_BASE_VAL] = "MSG_SMS_ATSC_IP_FILTER_REMOVE_REQ",
	[MSG_SMS_ATSC_IP_FILTER_REMOVE_RES           - MSG_TYPE_BASE_VAL] = "MSG_SMS_ATSC_IP_FILTER_REMOVE_RES",
	[MSG_SMS_ATSC_IP_FILTER_REMOVE_ALL_REQ       - MSG_TYPE_BASE_VAL] = "MSG_SMS_ATSC_IP_FILTER_REMOVE_ALL_REQ",
	[MSG_SMS_ATSC_IP_FILTER_REMOVE_ALL_RES       - MSG_TYPE_BASE_VAL] = "MSG_SMS_ATSC_IP_FILTER_REMOVE_ALL_RES",
	[MSG_SMS_WAIT_CMD                            - MSG_TYPE_BASE_VAL] = "MSG_SMS_WAIT_CMD",
	[MSG_SMS_ADD_PID_FILTER_REQ                  - MSG_TYPE_BASE_VAL] = "MSG_SMS_ADD_PID_FILTER_REQ",
	[MSG_SMS_ADD_PID_FILTER_RES                  - MSG_TYPE_BASE_VAL] = "MSG_SMS_ADD_PID_FILTER_RES",
	[MSG_SMS_REMOVE_PID_FILTER_REQ               - MSG_TYPE_BASE_VAL] = "MSG_SMS_REMOVE_PID_FILTER_REQ",
	[MSG_SMS_REMOVE_PID_FILTER_RES               - MSG_TYPE_BASE_VAL] = "MSG_SMS_REMOVE_PID_FILTER_RES",
	[MSG_SMS_FAST_INFORMATION_CHANNEL_REQ        - MSG_TYPE_BASE_VAL] = "MSG_SMS_FAST_INFORMATION_CHANNEL_REQ",
	[MSG_SMS_FAST_INFORMATION_CHANNEL_RES        - MSG_TYPE_BASE_VAL] = "MSG_SMS_FAST_INFORMATION_CHANNEL_RES",
	[MSG_SMS_DAB_CHANNEL                         - MSG_TYPE_BASE_VAL] = "MSG_SMS_DAB_CHANNEL",
	[MSG_SMS_GET_PID_FILTER_LIST_REQ             - MSG_TYPE_BASE_VAL] = "MSG_SMS_GET_PID_FILTER_LIST_REQ",
	[MSG_SMS_GET_PID_FILTER_LIST_RES             - MSG_TYPE_BASE_VAL] = "MSG_SMS_GET_PID_FILTER_LIST_RES",
	[MSG_SMS_POWER_DOWN_REQ                      - MSG_TYPE_BASE_VAL] = "MSG_SMS_POWER_DOWN_REQ",
	[MSG_SMS_POWER_DOWN_RES                      - MSG_TYPE_BASE_VAL] = "MSG_SMS_POWER_DOWN_RES",
	[MSG_SMS_ATSC_SLT_EXIST_IND                  - MSG_TYPE_BASE_VAL] = "MSG_SMS_ATSC_SLT_EXIST_IND",
	[MSG_SMS_ATSC_NO_SLT_IND                     - MSG_TYPE_BASE_VAL] = "MSG_SMS_ATSC_NO_SLT_IND",
	[MSG_SMS_GET_STATISTICS_REQ                  - MSG_TYPE_BASE_VAL] = "MSG_SMS_GET_STATISTICS_REQ",
	[MSG_SMS_GET_STATISTICS_RES                  - MSG_TYPE_BASE_VAL] = "MSG_SMS_GET_STATISTICS_RES",
	[MSG_SMS_SEND_DUMP                           - MSG_TYPE_BASE_VAL] = "MSG_SMS_SEND_DUMP",
	[MSG_SMS_SCAN_START_REQ                      - MSG_TYPE_BASE_VAL] = "MSG_SMS_SCAN_START_REQ",
	[MSG_SMS_SCAN_START_RES                      - MSG_TYPE_BASE_VAL] = "MSG_SMS_SCAN_START_RES",
	[MSG_SMS_SCAN_STOP_REQ                       - MSG_TYPE_BASE_VAL] = "MSG_SMS_SCAN_STOP_REQ",
	[MSG_SMS_SCAN_STOP_RES                       - MSG_TYPE_BASE_VAL] = "MSG_SMS_SCAN_STOP_RES",
	[MSG_SMS_SCAN_PROGRESS_IND                   - MSG_TYPE_BASE_VAL] = "MSG_SMS_SCAN_PROGRESS_IND",
	[MSG_SMS_SCAN_COMPLETE_IND                   - MSG_TYPE_BASE_VAL] = "MSG_SMS_SCAN_COMPLETE_IND",
	[MSG_SMS_LOG_ITEM                            - MSG_TYPE_BASE_VAL] = "MSG_SMS_LOG_ITEM",
	[MSG_SMS_DAB_SUBCHANNEL_RECONFIG_REQ         - MSG_TYPE_BASE_VAL] = "MSG_SMS_DAB_SUBCHANNEL_RECONFIG_REQ",
	[MSG_SMS_DAB_SUBCHANNEL_RECONFIG_RES         - MSG_TYPE_BASE_VAL] = "MSG_SMS_DAB_SUBCHANNEL_RECONFIG_RES",
	[MSG_SMS_HO_PER_SLICES_IND                   - MSG_TYPE_BASE_VAL] = "MSG_SMS_HO_PER_SLICES_IND",
	[MSG_SMS_HO_INBAND_POWER_IND                 - MSG_TYPE_BASE_VAL] = "MSG_SMS_HO_INBAND_POWER_IND",
	[MSG_SMS_MANUAL_DEMOD_REQ                    - MSG_TYPE_BASE_VAL] = "MSG_SMS_MANUAL_DEMOD_REQ",
	[MSG_SMS_HO_TUNE_ON_REQ                      - MSG_TYPE_BASE_VAL] = "MSG_SMS_HO_TUNE_ON_REQ",
	[MSG_SMS_HO_TUNE_ON_RES                      - MSG_TYPE_BASE_VAL] = "MSG_SMS_HO_TUNE_ON_RES",
	[MSG_SMS_HO_TUNE_OFF_REQ                     - MSG_TYPE_BASE_VAL] = "MSG_SMS_HO_TUNE_OFF_REQ",
	[MSG_SMS_HO_TUNE_OFF_RES                     - MSG_TYPE_BASE_VAL] = "MSG_SMS_HO_TUNE_OFF_RES",
	[MSG_SMS_HO_PEEK_FREQ_REQ                    - MSG_TYPE_BASE_VAL] = "MSG_SMS_HO_PEEK_FREQ_REQ",
	[MSG_SMS_HO_PEEK_FREQ_RES                    - MSG_TYPE_BASE_VAL] = "MSG_SMS_HO_PEEK_FREQ_RES",
	[MSG_SMS_HO_PEEK_FREQ_IND                    - MSG_TYPE_BASE_VAL] = "MSG_SMS_HO_PEEK_FREQ_IND",
	[MSG_SMS_MB_ATTEN_SET_REQ                    - MSG_TYPE_BASE_VAL] = "MSG_SMS_MB_ATTEN_SET_REQ",
	[MSG_SMS_MB_ATTEN_SET_RES                    - MSG_TYPE_BASE_VAL] = "MSG_SMS_MB_ATTEN_SET_RES",
	[MSG_SMS_ENABLE_STAT_IN_I2C_REQ              - MSG_TYPE_BASE_VAL] = "MSG_SMS_ENABLE_STAT_IN_I2C_REQ",
	[MSG_SMS_ENABLE_STAT_IN_I2C_RES              - MSG_TYPE_BASE_VAL] = "MSG_SMS_ENABLE_STAT_IN_I2C_RES",
	[MSG_SMS_SET_ANTENNA_CONFIG_REQ              - MSG_TYPE_BASE_VAL] = "MSG_SMS_SET_ANTENNA_CONFIG_REQ",
	[MSG_SMS_SET_ANTENNA_CONFIG_RES              - MSG_TYPE_BASE_VAL] = "MSG_SMS_SET_ANTENNA_CONFIG_RES",
	[MSG_SMS_GET_STATISTICS_EX_REQ               - MSG_TYPE_BASE_VAL] = "MSG_SMS_GET_STATISTICS_EX_REQ",
	[MSG_SMS_GET_STATISTICS_EX_RES               - MSG_TYPE_BASE_VAL] = "MSG_SMS_GET_STATISTICS_EX_RES",
	[MSG_SMS_SLEEP_RESUME_COMP_IND               - MSG_TYPE_BASE_VAL] = "MSG_SMS_SLEEP_RESUME_COMP_IND",
	[MSG_SMS_SWITCH_HOST_INTERFACE_REQ           - MSG_TYPE_BASE_VAL] = "MSG_SMS_SWITCH_HOST_INTERFACE_REQ",
	[MSG_SMS_SWITCH_HOST_INTERFACE_RES           - MSG_TYPE_BASE_VAL] = "MSG_SMS_SWITCH_HOST_INTERFACE_RES",
	[MSG_SMS_DATA_DOWNLOAD_REQ                   - MSG_TYPE_BASE_VAL] = "MSG_SMS_DATA_DOWNLOAD_REQ",
	[MSG_SMS_DATA_DOWNLOAD_RES                   - MSG_TYPE_BASE_VAL] = "MSG_SMS_DATA_DOWNLOAD_RES",
	[MSG_SMS_DATA_VALIDITY_REQ                   - MSG_TYPE_BASE_VAL] = "MSG_SMS_DATA_VALIDITY_REQ",
	[MSG_SMS_DATA_VALIDITY_RES                   - MSG_TYPE_BASE_VAL] = "MSG_SMS_DATA_VALIDITY_RES",
	[MSG_SMS_SWDOWNLOAD_TRIGGER_REQ              - MSG_TYPE_BASE_VAL] = "MSG_SMS_SWDOWNLOAD_TRIGGER_REQ",
	[MSG_SMS_SWDOWNLOAD_TRIGGER_RES              - MSG_TYPE_BASE_VAL] = "MSG_SMS_SWDOWNLOAD_TRIGGER_RES",
	[MSG_SMS_SWDOWNLOAD_BACKDOOR_REQ             - MSG_TYPE_BASE_VAL] = "MSG_SMS_SWDOWNLOAD_BACKDOOR_REQ",
	[MSG_SMS_SWDOWNLOAD_BACKDOOR_RES             - MSG_TYPE_BASE_VAL] = "MSG_SMS_SWDOWNLOAD_BACKDOOR_RES",
	[MSG_SMS_GET_VERSION_EX_REQ                  - MSG_TYPE_BASE_VAL] = "MSG_SMS_GET_VERSION_EX_REQ",
	[MSG_SMS_GET_VERSION_EX_RES                  - MSG_TYPE_BASE_VAL] = "MSG_SMS_GET_VERSION_EX_RES",
	[MSG_SMS_CLOCK_OUTPUT_CONFIG_REQ             - MSG_TYPE_BASE_VAL] = "MSG_SMS_CLOCK_OUTPUT_CONFIG_REQ",
	[MSG_SMS_CLOCK_OUTPUT_CONFIG_RES             - MSG_TYPE_BASE_VAL] = "MSG_SMS_CLOCK_OUTPUT_CONFIG_RES",
	[MSG_SMS_I2C_SET_FREQ_REQ                    - MSG_TYPE_BASE_VAL] = "MSG_SMS_I2C_SET_FREQ_REQ",
	[MSG_SMS_I2C_SET_FREQ_RES                    - MSG_TYPE_BASE_VAL] = "MSG_SMS_I2C_SET_FREQ_RES",
	[MSG_SMS_GENERIC_I2C_REQ                     - MSG_TYPE_BASE_VAL] = "MSG_SMS_GENERIC_I2C_REQ",
	[MSG_SMS_GENERIC_I2C_RES                     - MSG_TYPE_BASE_VAL] = "MSG_SMS_GENERIC_I2C_RES",
	[MSG_SMS_DVBT_BDA_DATA                       - MSG_TYPE_BASE_VAL] = "MSG_SMS_DVBT_BDA_DATA",
	[MSG_SW_RELOAD_REQ                           - MSG_TYPE_BASE_VAL] = "MSG_SW_RELOAD_REQ",
	[MSG_SMS_DATA_MSG                            - MSG_TYPE_BASE_VAL] = "MSG_SMS_DATA_MSG",
	[MSG_TABLE_UPLOAD_REQ                        - MSG_TYPE_BASE_VAL] = "MSG_TABLE_UPLOAD_REQ",
	[MSG_TABLE_UPLOAD_RES                        - MSG_TYPE_BASE_VAL] = "MSG_TABLE_UPLOAD_RES",
	[MSG_SW_RELOAD_START_REQ                     - MSG_TYPE_BASE_VAL] = "MSG_SW_RELOAD_START_REQ",
	[MSG_SW_RELOAD_START_RES                     - MSG_TYPE_BASE_VAL] = "MSG_SW_RELOAD_START_RES",
	[MSG_SW_RELOAD_EXEC_REQ                      - MSG_TYPE_BASE_VAL] = "MSG_SW_RELOAD_EXEC_REQ",
	[MSG_SW_RELOAD_EXEC_RES                      - MSG_TYPE_BASE_VAL] = "MSG_SW_RELOAD_EXEC_RES",
	[MSG_SMS_SPI_INT_LINE_SET_REQ                - MSG_TYPE_BASE_VAL] = "MSG_SMS_SPI_INT_LINE_SET_REQ",
	[MSG_SMS_SPI_INT_LINE_SET_RES                - MSG_TYPE_BASE_VAL] = "MSG_SMS_SPI_INT_LINE_SET_RES",
	[MSG_SMS_GPIO_CONFIG_EX_REQ                  - MSG_TYPE_BASE_VAL] = "MSG_SMS_GPIO_CONFIG_EX_REQ",
	[MSG_SMS_GPIO_CONFIG_EX_RES                  - MSG_TYPE_BASE_VAL] = "MSG_SMS_GPIO_CONFIG_EX_RES",
	[MSG_SMS_WATCHDOG_ACT_REQ                    - MSG_TYPE_BASE_VAL] = "MSG_SMS_WATCHDOG_ACT_REQ",
	[MSG_SMS_WATCHDOG_ACT_RES                    - MSG_TYPE_BASE_VAL] = "MSG_SMS_WATCHDOG_ACT_RES",
	[MSG_SMS_LOOPBACK_REQ                        - MSG_TYPE_BASE_VAL] = "MSG_SMS_LOOPBACK_REQ",
	[MSG_SMS_LOOPBACK_RES                        - MSG_TYPE_BASE_VAL] = "MSG_SMS_LOOPBACK_RES",
	[MSG_SMS_RAW_CAPTURE_START_REQ               - MSG_TYPE_BASE_VAL] = "MSG_SMS_RAW_CAPTURE_START_REQ",
	[MSG_SMS_RAW_CAPTURE_START_RES               - MSG_TYPE_BASE_VAL] = "MSG_SMS_RAW_CAPTURE_START_RES",
	[MSG_SMS_RAW_CAPTURE_ABORT_REQ               - MSG_TYPE_BASE_VAL] = "MSG_SMS_RAW_CAPTURE_ABORT_REQ",
	[MSG_SMS_RAW_CAPTURE_ABORT_RES               - MSG_TYPE_BASE_VAL] = "MSG_SMS_RAW_CAPTURE_ABORT_RES",
	[MSG_SMS_RAW_CAPTURE_COMPLETE_IND            - MSG_TYPE_BASE_VAL] = "MSG_SMS_RAW_CAPTURE_COMPLETE_IND",
	[MSG_SMS_DATA_PUMP_IND                       - MSG_TYPE_BASE_VAL] = "MSG_SMS_DATA_PUMP_IND",
	[MSG_SMS_DATA_PUMP_REQ                       - MSG_TYPE_BASE_VAL] = "MSG_SMS_DATA_PUMP_REQ",
	[MSG_SMS_DATA_PUMP_RES                       - MSG_TYPE_BASE_VAL] = "MSG_SMS_DATA_PUMP_RES",
	[MSG_SMS_FLASH_DL_REQ                        - MSG_TYPE_BASE_VAL] = "MSG_SMS_FLASH_DL_REQ",
	[MSG_SMS_EXEC_TEST_1_REQ                     - MSG_TYPE_BASE_VAL] = "MSG_SMS_EXEC_TEST_1_REQ",
	[MSG_SMS_EXEC_TEST_1_RES                     - MSG_TYPE_BASE_VAL] = "MSG_SMS_EXEC_TEST_1_RES",
	[MSG_SMS_ENBALE_TS_INTERFACE_REQ             - MSG_TYPE_BASE_VAL] = "MSG_SMS_ENBALE_TS_INTERFACE_REQ",
	[MSG_SMS_ENBALE_TS_INTERFACE_RES             - MSG_TYPE_BASE_VAL] = "MSG_SMS_ENBALE_TS_INTERFACE_RES",
	[MSG_SMS_SPI_SET_BUS_WIDTH_REQ               - MSG_TYPE_BASE_VAL] = "MSG_SMS_SPI_SET_BUS_WIDTH_REQ",
	[MSG_SMS_SPI_SET_BUS_WIDTH_RES               - MSG_TYPE_BASE_VAL] = "MSG_SMS_SPI_SET_BUS_WIDTH_RES",
	[MSG_SMS_SEND_EMM_REQ                        - MSG_TYPE_BASE_VAL] = "MSG_SMS_SEND_EMM_REQ",
	[MSG_SMS_SEND_EMM_RES                        - MSG_TYPE_BASE_VAL] = "MSG_SMS_SEND_EMM_RES",
	[MSG_SMS_DISABLE_TS_INTERFACE_REQ            - MSG_TYPE_BASE_VAL] = "MSG_SMS_DISABLE_TS_INTERFACE_REQ",
	[MSG_SMS_DISABLE_TS_INTERFACE_RES            - MSG_TYPE_BASE_VAL] = "MSG_SMS_DISABLE_TS_INTERFACE_RES",
	[MSG_SMS_IS_BUF_FREE_REQ                     - MSG_TYPE_BASE_VAL] = "MSG_SMS_IS_BUF_FREE_REQ",
	[MSG_SMS_IS_BUF_FREE_RES                     - MSG_TYPE_BASE_VAL] = "MSG_SMS_IS_BUF_FREE_RES",
	[MSG_SMS_EXT_ANTENNA_REQ                     - MSG_TYPE_BASE_VAL] = "MSG_SMS_EXT_ANTENNA_REQ",
	[MSG_SMS_EXT_ANTENNA_RES                     - MSG_TYPE_BASE_VAL] = "MSG_SMS_EXT_ANTENNA_RES",
	[MSG_SMS_CMMB_GET_NET_OF_FREQ_REQ_OBSOLETE   - MSG_TYPE_BASE_VAL] = "MSG_SMS_CMMB_GET_NET_OF_FREQ_REQ_OBSOLETE",
	[MSG_SMS_CMMB_GET_NET_OF_FREQ_RES_OBSOLETE   - MSG_TYPE_BASE_VAL] = "MSG_SMS_CMMB_GET_NET_OF_FREQ_RES_OBSOLETE",
	[MSG_SMS_BATTERY_LEVEL_REQ                   - MSG_TYPE_BASE_VAL] = "MSG_SMS_BATTERY_LEVEL_REQ",
	[MSG_SMS_BATTERY_LEVEL_RES                   - MSG_TYPE_BASE_VAL] = "MSG_SMS_BATTERY_LEVEL_RES",
	[MSG_SMS_CMMB_INJECT_TABLE_REQ_OBSOLETE      - MSG_TYPE_BASE_VAL] = "MSG_SMS_CMMB_INJECT_TABLE_REQ_OBSOLETE",
	[MSG_SMS_CMMB_INJECT_TABLE_RES_OBSOLETE      - MSG_TYPE_BASE_VAL] = "MSG_SMS_CMMB_INJECT_TABLE_RES_OBSOLETE",
	[MSG_SMS_FM_RADIO_BLOCK_IND                  - MSG_TYPE_BASE_VAL] = "MSG_SMS_FM_RADIO_BLOCK_IND",
	[MSG_SMS_HOST_NOTIFICATION_IND               - MSG_TYPE_BASE_VAL] = "MSG_SMS_HOST_NOTIFICATION_IND",
	[MSG_SMS_CMMB_GET_CONTROL_TABLE_REQ_OBSOLETE - MSG_TYPE_BASE_VAL] = "MSG_SMS_CMMB_GET_CONTROL_TABLE_REQ_OBSOLETE",
	[MSG_SMS_CMMB_GET_CONTROL_TABLE_RES_OBSOLETE - MSG_TYPE_BASE_VAL] = "MSG_SMS_CMMB_GET_CONTROL_TABLE_RES_OBSOLETE",
	[MSG_SMS_CMMB_GET_NETWORKS_REQ               - MSG_TYPE_BASE_VAL] = "MSG_SMS_CMMB_GET_NETWORKS_REQ",
	[MSG_SMS_CMMB_GET_NETWORKS_RES               - MSG_TYPE_BASE_VAL] = "MSG_SMS_CMMB_GET_NETWORKS_RES",
	[MSG_SMS_CMMB_START_SERVICE_REQ              - MSG_TYPE_BASE_VAL] = "MSG_SMS_CMMB_START_SERVICE_REQ",
	[MSG_SMS_CMMB_START_SERVICE_RES              - MSG_TYPE_BASE_VAL] = "MSG_SMS_CMMB_START_SERVICE_RES",
	[MSG_SMS_CMMB_STOP_SERVICE_REQ               - MSG_TYPE_BASE_VAL] = "MSG_SMS_CMMB_STOP_SERVICE_REQ",
	[MSG_SMS_CMMB_STOP_SERVICE_RES               - MSG_TYPE_BASE_VAL] = "MSG_SMS_CMMB_STOP_SERVICE_RES",
	[MSG_SMS_CMMB_ADD_CHANNEL_FILTER_REQ         - MSG_TYPE_BASE_VAL] = "MSG_SMS_CMMB_ADD_CHANNEL_FILTER_REQ",
	[MSG_SMS_CMMB_ADD_CHANNEL_FILTER_RES         - MSG_TYPE_BASE_VAL] = "MSG_SMS_CMMB_ADD_CHANNEL_FILTER_RES",
	[MSG_SMS_CMMB_REMOVE_CHANNEL_FILTER_REQ      - MSG_TYPE_BASE_VAL] = "MSG_SMS_CMMB_REMOVE_CHANNEL_FILTER_REQ",
	[MSG_SMS_CMMB_REMOVE_CHANNEL_FILTER_RES      - MSG_TYPE_BASE_VAL] = "MSG_SMS_CMMB_REMOVE_CHANNEL_FILTER_RES",
	[MSG_SMS_CMMB_START_CONTROL_INFO_REQ         - MSG_TYPE_BASE_VAL] = "MSG_SMS_CMMB_START_CONTROL_INFO_REQ",
	[MSG_SMS_CMMB_START_CONTROL_INFO_RES         - MSG_TYPE_BASE_VAL] = "MSG_SMS_CMMB_START_CONTROL_INFO_RES",
	[MSG_SMS_CMMB_STOP_CONTROL_INFO_REQ          - MSG_TYPE_BASE_VAL] = "MSG_SMS_CMMB_STOP_CONTROL_INFO_REQ",
	[MSG_SMS_CMMB_STOP_CONTROL_INFO_RES          - MSG_TYPE_BASE_VAL] = "MSG_SMS_CMMB_STOP_CONTROL_INFO_RES",
	[MSG_SMS_ISDBT_TUNE_REQ                      - MSG_TYPE_BASE_VAL] = "MSG_SMS_ISDBT_TUNE_REQ",
	[MSG_SMS_ISDBT_TUNE_RES                      - MSG_TYPE_BASE_VAL] = "MSG_SMS_ISDBT_TUNE_RES",
	[MSG_SMS_TRANSMISSION_IND                    - MSG_TYPE_BASE_VAL] = "MSG_SMS_TRANSMISSION_IND",
	[MSG_SMS_PID_STATISTICS_IND                  - MSG_TYPE_BASE_VAL] = "MSG_SMS_PID_STATISTICS_IND",
	[MSG_SMS_POWER_DOWN_IND                      - MSG_TYPE_BASE_VAL] = "MSG_SMS_POWER_DOWN_IND",
	[MSG_SMS_POWER_DOWN_CONF                     - MSG_TYPE_BASE_VAL] = "MSG_SMS_POWER_DOWN_CONF",
	[MSG_SMS_POWER_UP_IND                        - MSG_TYPE_BASE_VAL] = "MSG_SMS_POWER_UP_IND",
	[MSG_SMS_POWER_UP_CONF                       - MSG_TYPE_BASE_VAL] = "MSG_SMS_POWER_UP_CONF",
	[MSG_SMS_POWER_MODE_SET_REQ                  - MSG_TYPE_BASE_VAL] = "MSG_SMS_POWER_MODE_SET_REQ",
	[MSG_SMS_POWER_MODE_SET_RES                  - MSG_TYPE_BASE_VAL] = "MSG_SMS_POWER_MODE_SET_RES",
	[MSG_SMS_DEBUG_HOST_EVENT_REQ                - MSG_TYPE_BASE_VAL] = "MSG_SMS_DEBUG_HOST_EVENT_REQ",
	[MSG_SMS_DEBUG_HOST_EVENT_RES                - MSG_TYPE_BASE_VAL] = "MSG_SMS_DEBUG_HOST_EVENT_RES",
	[MSG_SMS_NEW_CRYSTAL_REQ                     - MSG_TYPE_BASE_VAL] = "MSG_SMS_NEW_CRYSTAL_REQ",
	[MSG_SMS_NEW_CRYSTAL_RES                     - MSG_TYPE_BASE_VAL] = "MSG_SMS_NEW_CRYSTAL_RES",
	[MSG_SMS_CONFIG_SPI_REQ                      - MSG_TYPE_BASE_VAL] = "MSG_SMS_CONFIG_SPI_REQ",
	[MSG_SMS_CONFIG_SPI_RES                      - MSG_TYPE_BASE_VAL] = "MSG_SMS_CONFIG_SPI_RES",
	[MSG_SMS_I2C_SHORT_STAT_IND                  - MSG_TYPE_BASE_VAL] = "MSG_SMS_I2C_SHORT_STAT_IND",
	[MSG_SMS_START_IR_REQ                        - MSG_TYPE_BASE_VAL] = "MSG_SMS_START_IR_REQ",
	[MSG_SMS_START_IR_RES                        - MSG_TYPE_BASE_VAL] = "MSG_SMS_START_IR_RES",
	[MSG_SMS_IR_SAMPLES_IND                      - MSG_TYPE_BASE_VAL] = "MSG_SMS_IR_SAMPLES_IND",
	[MSG_SMS_CMMB_CA_SERVICE_IND                 - MSG_TYPE_BASE_VAL] = "MSG_SMS_CMMB_CA_SERVICE_IND",
	[MSG_SMS_SLAVE_DEVICE_DETECTED               - MSG_TYPE_BASE_VAL] = "MSG_SMS_SLAVE_DEVICE_DETECTED",
	[MSG_SMS_INTERFACE_LOCK_IND                  - MSG_TYPE_BASE_VAL] = "MSG_SMS_INTERFACE_LOCK_IND",
	[MSG_SMS_INTERFACE_UNLOCK_IND                - MSG_TYPE_BASE_VAL] = "MSG_SMS_INTERFACE_UNLOCK_IND",
	[MSG_SMS_SEND_ROSUM_BUFF_REQ                 - MSG_TYPE_BASE_VAL] = "MSG_SMS_SEND_ROSUM_BUFF_REQ",
	[MSG_SMS_SEND_ROSUM_BUFF_RES                 - MSG_TYPE_BASE_VAL] = "MSG_SMS_SEND_ROSUM_BUFF_RES",
	[MSG_SMS_ROSUM_BUFF                          - MSG_TYPE_BASE_VAL] = "MSG_SMS_ROSUM_BUFF",
	[MSG_SMS_SET_AES128_KEY_REQ                  - MSG_TYPE_BASE_VAL] = "MSG_SMS_SET_AES128_KEY_REQ",
	[MSG_SMS_SET_AES128_KEY_RES                  - MSG_TYPE_BASE_VAL] = "MSG_SMS_SET_AES128_KEY_RES",
	[MSG_SMS_MBBMS_WRITE_REQ                     - MSG_TYPE_BASE_VAL] = "MSG_SMS_MBBMS_WRITE_REQ",
	[MSG_SMS_MBBMS_WRITE_RES                     - MSG_TYPE_BASE_VAL] = "MSG_SMS_MBBMS_WRITE_RES",
	[MSG_SMS_MBBMS_READ_IND                      - MSG_TYPE_BASE_VAL] = "MSG_SMS_MBBMS_READ_IND",
	[MSG_SMS_IQ_STREAM_START_REQ                 - MSG_TYPE_BASE_VAL] = "MSG_SMS_IQ_STREAM_START_REQ",
	[MSG_SMS_IQ_STREAM_START_RES                 - MSG_TYPE_BASE_VAL] = "MSG_SMS_IQ_STREAM_START_RES",
	[MSG_SMS_IQ_STREAM_STOP_REQ                  - MSG_TYPE_BASE_VAL] = "MSG_SMS_IQ_STREAM_STOP_REQ",
	[MSG_SMS_IQ_STREAM_STOP_RES                  - MSG_TYPE_BASE_VAL] = "MSG_SMS_IQ_STREAM_STOP_RES",
	[MSG_SMS_IQ_STREAM_DATA_BLOCK                - MSG_TYPE_BASE_VAL] = "MSG_SMS_IQ_STREAM_DATA_BLOCK",
	[MSG_SMS_GET_EEPROM_VERSION_REQ              - MSG_TYPE_BASE_VAL] = "MSG_SMS_GET_EEPROM_VERSION_REQ",
	[MSG_SMS_GET_EEPROM_VERSION_RES              - MSG_TYPE_BASE_VAL] = "MSG_SMS_GET_EEPROM_VERSION_RES",
	[MSG_SMS_SIGNAL_DETECTED_IND                 - MSG_TYPE_BASE_VAL] = "MSG_SMS_SIGNAL_DETECTED_IND",
	[MSG_SMS_NO_SIGNAL_IND                       - MSG_TYPE_BASE_VAL] = "MSG_SMS_NO_SIGNAL_IND",
	[MSG_SMS_MRC_SHUTDOWN_SLAVE_REQ              - MSG_TYPE_BASE_VAL] = "MSG_SMS_MRC_SHUTDOWN_SLAVE_REQ",
	[MSG_SMS_MRC_SHUTDOWN_SLAVE_RES              - MSG_TYPE_BASE_VAL] = "MSG_SMS_MRC_SHUTDOWN_SLAVE_RES",
	[MSG_SMS_MRC_BRINGUP_SLAVE_REQ               - MSG_TYPE_BASE_VAL] = "MSG_SMS_MRC_BRINGUP_SLAVE_REQ",
	[MSG_SMS_MRC_BRINGUP_SLAVE_RES               - MSG_TYPE_BASE_VAL] = "MSG_SMS_MRC_BRINGUP_SLAVE_RES",
	[MSG_SMS_EXTERNAL_LNA_CTRL_REQ               - MSG_TYPE_BASE_VAL] = "MSG_SMS_EXTERNAL_LNA_CTRL_REQ",
	[MSG_SMS_EXTERNAL_LNA_CTRL_RES               - MSG_TYPE_BASE_VAL] = "MSG_SMS_EXTERNAL_LNA_CTRL_RES",
	[MSG_SMS_SET_PERIODIC_STATISTICS_REQ         - MSG_TYPE_BASE_VAL] = "MSG_SMS_SET_PERIODIC_STATISTICS_REQ",
	[MSG_SMS_SET_PERIODIC_STATISTICS_RES         - MSG_TYPE_BASE_VAL] = "MSG_SMS_SET_PERIODIC_STATISTICS_RES",
	[MSG_SMS_CMMB_SET_AUTO_OUTPUT_TS0_REQ        - MSG_TYPE_BASE_VAL] = "MSG_SMS_CMMB_SET_AUTO_OUTPUT_TS0_REQ",
	[MSG_SMS_CMMB_SET_AUTO_OUTPUT_TS0_RES        - MSG_TYPE_BASE_VAL] = "MSG_SMS_CMMB_SET_AUTO_OUTPUT_TS0_RES",
	[LOCAL_TUNE                                  - MSG_TYPE_BASE_VAL] = "LOCAL_TUNE",
	[LOCAL_IFFT_H_ICI                            - MSG_TYPE_BASE_VAL] = "LOCAL_IFFT_H_ICI",
	[MSG_RESYNC_REQ                              - MSG_TYPE_BASE_VAL] = "MSG_RESYNC_REQ",
	[MSG_SMS_CMMB_GET_MRC_STATISTICS_REQ         - MSG_TYPE_BASE_VAL] = "MSG_SMS_CMMB_GET_MRC_STATISTICS_REQ",
	[MSG_SMS_CMMB_GET_MRC_STATISTICS_RES         - MSG_TYPE_BASE_VAL] = "MSG_SMS_CMMB_GET_MRC_STATISTICS_RES",
	[MSG_SMS_LOG_EX_ITEM                         - MSG_TYPE_BASE_VAL] = "MSG_SMS_LOG_EX_ITEM",
	[MSG_SMS_DEVICE_DATA_LOSS_IND                - MSG_TYPE_BASE_VAL] = "MSG_SMS_DEVICE_DATA_LOSS_IND",
	[MSG_SMS_MRC_WATCHDOG_TRIGGERED_IND          - MSG_TYPE_BASE_VAL] = "MSG_SMS_MRC_WATCHDOG_TRIGGERED_IND",
	[MSG_SMS_USER_MSG_REQ                        - MSG_TYPE_BASE_VAL] = "MSG_SMS_USER_MSG_REQ",
	[MSG_SMS_USER_MSG_RES                        - MSG_TYPE_BASE_VAL] = "MSG_SMS_USER_MSG_RES",
	[MSG_SMS_SMART_CARD_INIT_REQ                 - MSG_TYPE_BASE_VAL] = "MSG_SMS_SMART_CARD_INIT_REQ",
	[MSG_SMS_SMART_CARD_INIT_RES                 - MSG_TYPE_BASE_VAL] = "MSG_SMS_SMART_CARD_INIT_RES",
	[MSG_SMS_SMART_CARD_WRITE_REQ                - MSG_TYPE_BASE_VAL] = "MSG_SMS_SMART_CARD_WRITE_REQ",
	[MSG_SMS_SMART_CARD_WRITE_RES                - MSG_TYPE_BASE_VAL] = "MSG_SMS_SMART_CARD_WRITE_RES",
	[MSG_SMS_SMART_CARD_READ_IND                 - MSG_TYPE_BASE_VAL] = "MSG_SMS_SMART_CARD_READ_IND",
	[MSG_SMS_TSE_ENABLE_REQ                      - MSG_TYPE_BASE_VAL] = "MSG_SMS_TSE_ENABLE_REQ",
	[MSG_SMS_TSE_ENABLE_RES                      - MSG_TYPE_BASE_VAL] = "MSG_SMS_TSE_ENABLE_RES",
	[MSG_SMS_CMMB_GET_SHORT_STATISTICS_REQ       - MSG_TYPE_BASE_VAL] = "MSG_SMS_CMMB_GET_SHORT_STATISTICS_REQ",
	[MSG_SMS_CMMB_GET_SHORT_STATISTICS_RES       - MSG_TYPE_BASE_VAL] = "MSG_SMS_CMMB_GET_SHORT_STATISTICS_RES",
	[MSG_SMS_LED_CONFIG_REQ                      - MSG_TYPE_BASE_VAL] = "MSG_SMS_LED_CONFIG_REQ",
	[MSG_SMS_LED_CONFIG_RES                      - MSG_TYPE_BASE_VAL] = "MSG_SMS_LED_CONFIG_RES",
	[MSG_PWM_ANTENNA_REQ                         - MSG_TYPE_BASE_VAL] = "MSG_PWM_ANTENNA_REQ",
	[MSG_PWM_ANTENNA_RES                         - MSG_TYPE_BASE_VAL] = "MSG_PWM_ANTENNA_RES",
	[MSG_SMS_CMMB_SMD_SN_REQ                     - MSG_TYPE_BASE_VAL] = "MSG_SMS_CMMB_SMD_SN_REQ",
	[MSG_SMS_CMMB_SMD_SN_RES                     - MSG_TYPE_BASE_VAL] = "MSG_SMS_CMMB_SMD_SN_RES",
	[MSG_SMS_CMMB_SET_CA_CW_REQ                  - MSG_TYPE_BASE_VAL] = "MSG_SMS_CMMB_SET_CA_CW_REQ",
	[MSG_SMS_CMMB_SET_CA_CW_RES                  - MSG_TYPE_BASE_VAL] = "MSG_SMS_CMMB_SET_CA_CW_RES",
	[MSG_SMS_CMMB_SET_CA_SALT_REQ                - MSG_TYPE_BASE_VAL] = "MSG_SMS_CMMB_SET_CA_SALT_REQ",
	[MSG_SMS_CMMB_SET_CA_SALT_RES                - MSG_TYPE_BASE_VAL] = "MSG_SMS_CMMB_SET_CA_SALT_RES",
	[MSG_SMS_NSCD_INIT_REQ                       - MSG_TYPE_BASE_VAL] = "MSG_SMS_NSCD_INIT_REQ",
	[MSG_SMS_NSCD_INIT_RES                       - MSG_TYPE_BASE_VAL] = "MSG_SMS_NSCD_INIT_RES",
	[MSG_SMS_NSCD_PROCESS_SECTION_REQ            - MSG_TYPE_BASE_VAL] = "MSG_SMS_NSCD_PROCESS_SECTION_REQ",
	[MSG_SMS_NSCD_PROCESS_SECTION_RES            - MSG_TYPE_BASE_VAL] = "MSG_SMS_NSCD_PROCESS_SECTION_RES",
	[MSG_SMS_DBD_CREATE_OBJECT_REQ               - MSG_TYPE_BASE_VAL] = "MSG_SMS_DBD_CREATE_OBJECT_REQ",
	[MSG_SMS_DBD_CREATE_OBJECT_RES               - MSG_TYPE_BASE_VAL] = "MSG_SMS_DBD_CREATE_OBJECT_RES",
	[MSG_SMS_DBD_CONFIGURE_REQ                   - MSG_TYPE_BASE_VAL] = "MSG_SMS_DBD_CONFIGURE_REQ",
	[MSG_SMS_DBD_CONFIGURE_RES                   - MSG_TYPE_BASE_VAL] = "MSG_SMS_DBD_CONFIGURE_RES",
	[MSG_SMS_DBD_SET_KEYS_REQ                    - MSG_TYPE_BASE_VAL] = "MSG_SMS_DBD_SET_KEYS_REQ",
	[MSG_SMS_DBD_SET_KEYS_RES                    - MSG_TYPE_BASE_VAL] = "MSG_SMS_DBD_SET_KEYS_RES",
	[MSG_SMS_DBD_PROCESS_HEADER_REQ              - MSG_TYPE_BASE_VAL] = "MSG_SMS_DBD_PROCESS_HEADER_REQ",
	[MSG_SMS_DBD_PROCESS_HEADER_RES              - MSG_TYPE_BASE_VAL] = "MSG_SMS_DBD_PROCESS_HEADER_RES",
	[MSG_SMS_DBD_PROCESS_DATA_REQ                - MSG_TYPE_BASE_VAL] = "MSG_SMS_DBD_PROCESS_DATA_REQ",
	[MSG_SMS_DBD_PROCESS_DATA_RES                - MSG_TYPE_BASE_VAL] = "MSG_SMS_DBD_PROCESS_DATA_RES",
	[MSG_SMS_DBD_PROCESS_GET_DATA_REQ            - MSG_TYPE_BASE_VAL] = "MSG_SMS_DBD_PROCESS_GET_DATA_REQ",
	[MSG_SMS_DBD_PROCESS_GET_DATA_RES            - MSG_TYPE_BASE_VAL] = "MSG_SMS_DBD_PROCESS_GET_DATA_RES",
	[MSG_SMS_NSCD_OPEN_SESSION_REQ               - MSG_TYPE_BASE_VAL] = "MSG_SMS_NSCD_OPEN_SESSION_REQ",
	[MSG_SMS_NSCD_OPEN_SESSION_RES               - MSG_TYPE_BASE_VAL] = "MSG_SMS_NSCD_OPEN_SESSION_RES",
	[MSG_SMS_SEND_HOST_DATA_TO_DEMUX_REQ         - MSG_TYPE_BASE_VAL] = "MSG_SMS_SEND_HOST_DATA_TO_DEMUX_REQ",
	[MSG_SMS_SEND_HOST_DATA_TO_DEMUX_RES         - MSG_TYPE_BASE_VAL] = "MSG_SMS_SEND_HOST_DATA_TO_DEMUX_RES",
	[MSG_LAST_MSG_TYPE                           - MSG_TYPE_BASE_VAL] = "MSG_LAST_MSG_TYPE",
};

char *smscore_translate_msg(enum msg_types msgtype)
{
	int i = msgtype - MSG_TYPE_BASE_VAL;
	char *msg;

	if (i < 0 || i >= ARRAY_SIZE(siano_msgs))
		return "Unknown msg type";

	msg = siano_msgs[i];

	if (!*msg)
		return "Unknown msg type";

	return msg;
}
EXPORT_SYMBOL_GPL(smscore_translate_msg);

void smscore_set_board_id(struct smscore_device_t *core, int id)
{
	core->board_id = id;
}

int smscore_led_state(struct smscore_device_t *core, int led)
{
	if (led >= 0)
		core->led_state = led;
	return core->led_state;
}
EXPORT_SYMBOL_GPL(smscore_set_board_id);

int smscore_get_board_id(struct smscore_device_t *core)
{
	return core->board_id;
}
EXPORT_SYMBOL_GPL(smscore_get_board_id);

struct smscore_registry_entry_t {
	struct list_head entry;
	char			devpath[32];
	int				mode;
	enum sms_device_type_st	type;
};

static struct list_head g_smscore_notifyees;
static struct list_head g_smscore_devices;
static struct mutex g_smscore_deviceslock;

static struct list_head g_smscore_registry;
static struct mutex g_smscore_registrylock;

static int default_mode = DEVICE_MODE_NONE;

module_param(default_mode, int, 0644);
MODULE_PARM_DESC(default_mode, "default firmware id (device mode)");

static struct smscore_registry_entry_t *smscore_find_registry(char *devpath)
{
	struct smscore_registry_entry_t *entry;
	struct list_head *next;

	kmutex_lock(&g_smscore_registrylock);
	for (next = g_smscore_registry.next;
	     next != &g_smscore_registry;
	     next = next->next) {
		entry = (struct smscore_registry_entry_t *) next;
		if (!strcmp(entry->devpath, devpath)) {
			kmutex_unlock(&g_smscore_registrylock);
			return entry;
		}
	}
	entry = kmalloc(sizeof(struct smscore_registry_entry_t), GFP_KERNEL);
	if (entry) {
		entry->mode = default_mode;
		strcpy(entry->devpath, devpath);
		list_add(&entry->entry, &g_smscore_registry);
	} else
		sms_err("failed to create smscore_registry.");
	kmutex_unlock(&g_smscore_registrylock);
	return entry;
}

int smscore_registry_getmode(char *devpath)
{
	struct smscore_registry_entry_t *entry;

	entry = smscore_find_registry(devpath);
	if (entry)
		return entry->mode;
	else
		sms_err("No registry found.");

	return default_mode;
}
EXPORT_SYMBOL_GPL(smscore_registry_getmode);

static enum sms_device_type_st smscore_registry_gettype(char *devpath)
{
	struct smscore_registry_entry_t *entry;

	entry = smscore_find_registry(devpath);
	if (entry)
		return entry->type;
	else
		sms_err("No registry found.");

	return -EINVAL;
}

static void smscore_registry_setmode(char *devpath, int mode)
{
	struct smscore_registry_entry_t *entry;

	entry = smscore_find_registry(devpath);
	if (entry)
		entry->mode = mode;
	else
		sms_err("No registry found.");
}

static void smscore_registry_settype(char *devpath,
				     enum sms_device_type_st type)
{
	struct smscore_registry_entry_t *entry;

	entry = smscore_find_registry(devpath);
	if (entry)
		entry->type = type;
	else
		sms_err("No registry found.");
}


static void list_add_locked(struct list_head *new, struct list_head *head,
			    spinlock_t *lock)
{
	unsigned long flags;

	spin_lock_irqsave(lock, flags);

	list_add(new, head);

	spin_unlock_irqrestore(lock, flags);
}

/* 
                                                                          
                                                                        
  
                          
  
                                     
 */
int smscore_register_hotplug(hotplug_t hotplug)
{
	struct smscore_device_notifyee_t *notifyee;
	struct list_head *next, *first;
	int rc = 0;

	kmutex_lock(&g_smscore_deviceslock);

	notifyee = kmalloc(sizeof(struct smscore_device_notifyee_t),
			   GFP_KERNEL);
	if (notifyee) {
		/*                                            */
		first = &g_smscore_devices;
		for (next = first->next;
		     next != first && !rc;
		     next = next->next) {
			struct smscore_device_t *coredev =
				(struct smscore_device_t *) next;
			rc = hotplug(coredev, coredev->device, 1);
		}

		if (rc >= 0) {
			notifyee->hotplug = hotplug;
			list_add(&notifyee->entry, &g_smscore_notifyees);
		} else
			kfree(notifyee);
	} else
		rc = -ENOMEM;

	kmutex_unlock(&g_smscore_deviceslock);

	return rc;
}
EXPORT_SYMBOL_GPL(smscore_register_hotplug);

/* 
                                                                            
  
                          
  
 */
void smscore_unregister_hotplug(hotplug_t hotplug)
{
	struct list_head *next, *first;

	kmutex_lock(&g_smscore_deviceslock);

	first = &g_smscore_notifyees;

	for (next = first->next; next != first;) {
		struct smscore_device_notifyee_t *notifyee =
			(struct smscore_device_notifyee_t *) next;
		next = next->next;

		if (notifyee->hotplug == hotplug) {
			list_del(&notifyee->entry);
			kfree(notifyee);
		}
	}

	kmutex_unlock(&g_smscore_deviceslock);
}
EXPORT_SYMBOL_GPL(smscore_unregister_hotplug);

static void smscore_notify_clients(struct smscore_device_t *coredev)
{
	struct smscore_client_t *client;

	/*                                                                    */
	while (!list_empty(&coredev->clients)) {
		client = (struct smscore_client_t *) coredev->clients.next;
		client->onremove_handler(client->context);
	}
}

static int smscore_notify_callbacks(struct smscore_device_t *coredev,
				    struct device *device, int arrival)
{
	struct smscore_device_notifyee_t *elem;
	int rc = 0;

	/*                                          */

	list_for_each_entry(elem, &g_smscore_notifyees, entry) {
		rc = elem->hotplug(coredev, device, arrival);
		if (rc < 0)
			break;
	}

	return rc;
}

static struct
smscore_buffer_t *smscore_createbuffer(u8 *buffer, void *common_buffer,
				       dma_addr_t common_buffer_phys)
{
	struct smscore_buffer_t *cb;

	cb = kzalloc(sizeof(struct smscore_buffer_t), GFP_KERNEL);
	if (!cb) {
		sms_info("kzalloc(...) failed");
		return NULL;
	}

	cb->p = buffer;
	cb->offset_in_common = buffer - (u8 *) common_buffer;
	cb->phys = common_buffer_phys + cb->offset_in_common;

	return cb;
}

/* 
                                                         
                                                                          
  
                                                                         
                             
                                                                         
  
                                     
 */
int smscore_register_device(struct smsdevice_params_t *params,
			    struct smscore_device_t **coredev)
{
	struct smscore_device_t *dev;
	u8 *buffer;

	dev = kzalloc(sizeof(struct smscore_device_t), GFP_KERNEL);
	if (!dev) {
		sms_info("kzalloc(...) failed");
		return -ENOMEM;
	}

	/*                                                                  */
	INIT_LIST_HEAD(&dev->entry);

	/*             */
	INIT_LIST_HEAD(&dev->clients);
	INIT_LIST_HEAD(&dev->buffers);

	/*            */
	spin_lock_init(&dev->clientslock);
	spin_lock_init(&dev->bufferslock);

	/*                        */
	init_completion(&dev->version_ex_done);
	init_completion(&dev->data_download_done);
	init_completion(&dev->data_validity_done);
	init_completion(&dev->trigger_done);
	init_completion(&dev->init_device_done);
	init_completion(&dev->reload_start_done);
	init_completion(&dev->resume_done);
	init_completion(&dev->gpio_configuration_done);
	init_completion(&dev->gpio_set_level_done);
	init_completion(&dev->gpio_get_level_done);
	init_completion(&dev->ir_init_done);

	/*                   */
	init_waitqueue_head(&dev->buffer_mng_waitq);

	/*                     */
	dev->common_buffer_size = params->buffer_size * params->num_buffers;
	dev->common_buffer = dma_alloc_coherent(NULL, dev->common_buffer_size,
						&dev->common_buffer_phys,
						GFP_KERNEL | GFP_DMA);
	if (!dev->common_buffer) {
		smscore_unregister_device(dev);
		return -ENOMEM;
	}

	/*                     */
	for (buffer = dev->common_buffer;
	     dev->num_buffers < params->num_buffers;
	     dev->num_buffers++, buffer += params->buffer_size) {
		struct smscore_buffer_t *cb;

		cb = smscore_createbuffer(buffer, dev->common_buffer,
					  dev->common_buffer_phys);
		if (!cb) {
			smscore_unregister_device(dev);
			return -ENOMEM;
		}

		smscore_putbuffer(dev, cb);
	}

	sms_info("allocated %d buffers", dev->num_buffers);

	dev->mode = DEVICE_MODE_NONE;
	dev->board_id = SMS_BOARD_UNKNOWN;
	dev->context = params->context;
	dev->device = params->device;
	dev->setmode_handler = params->setmode_handler;
	dev->detectmode_handler = params->detectmode_handler;
	dev->sendrequest_handler = params->sendrequest_handler;
	dev->preload_handler = params->preload_handler;
	dev->postload_handler = params->postload_handler;

	dev->device_flags = params->flags;
	strcpy(dev->devpath, params->devpath);

	smscore_registry_settype(dev->devpath, params->device_type);

	/*                            */
	kmutex_lock(&g_smscore_deviceslock);
	list_add(&dev->entry, &g_smscore_devices);
	kmutex_unlock(&g_smscore_deviceslock);

	*coredev = dev;

	sms_info("device %p created", dev);

	return 0;
}
EXPORT_SYMBOL_GPL(smscore_register_device);


static int smscore_sendrequest_and_wait(struct smscore_device_t *coredev,
		void *buffer, size_t size, struct completion *completion) {
	int rc;

	if (completion == NULL)
		return -EINVAL;
	init_completion(completion);

	rc = coredev->sendrequest_handler(coredev->context, buffer, size);
	if (rc < 0) {
		sms_info("sendrequest returned error %d", rc);
		return rc;
	}

	return wait_for_completion_timeout(completion,
			msecs_to_jiffies(SMS_PROTOCOL_MAX_RAOUNDTRIP_MS)) ?
			0 : -ETIME;
}

/* 
                                 
  
                                      
 */
static int smscore_init_ir(struct smscore_device_t *coredev)
{
	int ir_io;
	int rc;
	void *buffer;

	coredev->ir.dev = NULL;
	ir_io = sms_get_board(smscore_get_board_id(coredev))->board_cfg.ir;
	if (ir_io) {/*                                            */
		sms_info("IR loading");
		rc = sms_ir_init(coredev);

		if	(rc != 0)
			sms_err("Error initialization DTV IR sub-module");
		else {
			buffer = kmalloc(sizeof(struct sms_msg_data2) +
						SMS_DMA_ALIGNMENT,
						GFP_KERNEL | GFP_DMA);
			if (buffer) {
				struct sms_msg_data2 *msg =
				(struct sms_msg_data2 *)
				SMS_ALIGN_ADDRESS(buffer);

				SMS_INIT_MSG(&msg->x_msg_header,
						MSG_SMS_START_IR_REQ,
						sizeof(struct sms_msg_data2));
				msg->msg_data[0] = coredev->ir.controller;
				msg->msg_data[1] = coredev->ir.timeout;

				rc = smscore_sendrequest_and_wait(coredev, msg,
						msg->x_msg_header. msg_length,
						&coredev->ir_init_done);

				kfree(buffer);
			} else
				sms_err
				("Sending IR initialization message failed");
		}
	} else
		sms_info("IR port has not been detected");

	return 0;
}

/* 
                                                                         
  
                                                         
                                         
  
                                     
 */
static int smscore_configure_board(struct smscore_device_t *coredev)
{
	struct sms_board *board;

	board = sms_get_board(coredev->board_id);
	if (!board) {
		sms_err("no board configuration exist.");
		return -EINVAL;
	}

	if (board->mtu) {
		struct sms_msg_data mtu_msg;
		sms_debug("set max transmit unit %d", board->mtu);

		mtu_msg.x_msg_header.msg_src_id = 0;
		mtu_msg.x_msg_header.msg_dst_id = HIF_TASK;
		mtu_msg.x_msg_header.msg_flags = 0;
		mtu_msg.x_msg_header.msg_type = MSG_SMS_SET_MAX_TX_MSG_LEN_REQ;
		mtu_msg.x_msg_header.msg_length = sizeof(mtu_msg);
		mtu_msg.msg_data[0] = board->mtu;

		coredev->sendrequest_handler(coredev->context, &mtu_msg,
					     sizeof(mtu_msg));
	}

	if (board->crystal) {
		struct sms_msg_data crys_msg;
		sms_debug("set crystal value %d", board->crystal);

		SMS_INIT_MSG(&crys_msg.x_msg_header,
				MSG_SMS_NEW_CRYSTAL_REQ,
				sizeof(crys_msg));
		crys_msg.msg_data[0] = board->crystal;

		coredev->sendrequest_handler(coredev->context, &crys_msg,
					     sizeof(crys_msg));
	}

	return 0;
}

/* 
                                                                             
  
                                                         
                             
  
                                     
 */
int smscore_start_device(struct smscore_device_t *coredev)
{
	int rc;
	int board_id = smscore_get_board_id(coredev);
	int mode = smscore_registry_getmode(coredev->devpath);

	/*                                           */
	if (board_id != SMS_BOARD_UNKNOWN && mode == DEVICE_MODE_NONE)
		mode = sms_get_board(board_id)->default_mode;

	rc = smscore_set_device_mode(coredev, mode);
	if (rc < 0) {
		sms_info("set device mode faile , rc %d", rc);
		return rc;
	}
	rc = smscore_configure_board(coredev);
	if (rc < 0) {
		sms_info("configure board failed , rc %d", rc);
		return rc;
	}

	kmutex_lock(&g_smscore_deviceslock);

	rc = smscore_notify_callbacks(coredev, coredev->device, 1);
	smscore_init_ir(coredev);

	sms_info("device %p started, rc %d", coredev, rc);

	kmutex_unlock(&g_smscore_deviceslock);

	return rc;
}
EXPORT_SYMBOL_GPL(smscore_start_device);


static int smscore_load_firmware_family2(struct smscore_device_t *coredev,
					 void *buffer, size_t size)
{
	struct sms_firmware *firmware = (struct sms_firmware *) buffer;
	struct sms_msg_data4 *msg;
	u32 mem_address,  calc_checksum = 0;
	u32 i, *ptr;
	u8 *payload = firmware->payload;
	int rc = 0;
	firmware->start_address = le32_to_cpu(firmware->start_address);
	firmware->length = le32_to_cpu(firmware->length);

	mem_address = firmware->start_address;

	sms_info("loading FW to addr 0x%x size %d",
		 mem_address, firmware->length);
	if (coredev->preload_handler) {
		rc = coredev->preload_handler(coredev->context);
		if (rc < 0)
			return rc;
	}

	/*                                                  */
	msg = kmalloc(PAGE_SIZE, GFP_KERNEL | GFP_DMA);
	if (!msg)
		return -ENOMEM;

	if (coredev->mode != DEVICE_MODE_NONE) {
		sms_debug("sending reload command.");
		SMS_INIT_MSG(&msg->x_msg_header, MSG_SW_RELOAD_START_REQ,
			     sizeof(struct sms_msg_hdr));
		rc = smscore_sendrequest_and_wait(coredev, msg,
						  msg->x_msg_header.msg_length,
						  &coredev->reload_start_done);
		if (rc < 0) {
			sms_err("device reload failed, rc %d", rc);
			goto exit_fw_download;
		}
		mem_address = *(u32 *) &payload[20];
	}

	for (i = 0, ptr = (u32 *)firmware->payload; i < firmware->length/4 ;
	     i++, ptr++)
		calc_checksum += *ptr;

	while (size && rc >= 0) {
		struct sms_data_download *data_msg =
			(struct sms_data_download *) msg;
		int payload_size = min_t(int, size, SMS_MAX_PAYLOAD_SIZE);

		SMS_INIT_MSG(&msg->x_msg_header, MSG_SMS_DATA_DOWNLOAD_REQ,
			     (u16)(sizeof(struct sms_msg_hdr) +
				      sizeof(u32) + payload_size));

		data_msg->mem_addr = mem_address;
		memcpy(data_msg->payload, payload, payload_size);

		rc = smscore_sendrequest_and_wait(coredev, data_msg,
				data_msg->x_msg_header.msg_length,
				&coredev->data_download_done);

		payload += payload_size;
		size -= payload_size;
		mem_address += payload_size;
	}

	if (rc < 0)
		goto exit_fw_download;

	sms_err("sending MSG_SMS_DATA_VALIDITY_REQ expecting 0x%x",
		calc_checksum);
	SMS_INIT_MSG(&msg->x_msg_header, MSG_SMS_DATA_VALIDITY_REQ,
			sizeof(msg->x_msg_header) +
			sizeof(u32) * 3);
	msg->msg_data[0] = firmware->start_address;
		/*             */
	msg->msg_data[1] = firmware->length;
	msg->msg_data[2] = 0; /*                 */
	rc = smscore_sendrequest_and_wait(coredev, msg,
					  msg->x_msg_header.msg_length,
					  &coredev->data_validity_done);
	if (rc < 0)
		goto exit_fw_download;

	if (coredev->mode == DEVICE_MODE_NONE) {
		struct sms_msg_data *trigger_msg =
			(struct sms_msg_data *) msg;

		sms_debug("sending MSG_SMS_SWDOWNLOAD_TRIGGER_REQ");
		SMS_INIT_MSG(&msg->x_msg_header,
				MSG_SMS_SWDOWNLOAD_TRIGGER_REQ,
				sizeof(struct sms_msg_hdr) +
				sizeof(u32) * 5);

		trigger_msg->msg_data[0] = firmware->start_address;
					/*             */
		trigger_msg->msg_data[1] = 6; /*          */
		trigger_msg->msg_data[2] = 0x200; /*            */
		trigger_msg->msg_data[3] = 0; /*           */
		trigger_msg->msg_data[4] = 4; /*         */

		rc = smscore_sendrequest_and_wait(coredev, trigger_msg,
					trigger_msg->x_msg_header.msg_length,
					&coredev->trigger_done);
	} else {
		SMS_INIT_MSG(&msg->x_msg_header, MSG_SW_RELOAD_EXEC_REQ,
				sizeof(struct sms_msg_hdr));
		rc = coredev->sendrequest_handler(coredev->context, msg,
				msg->x_msg_header.msg_length);
	}

	if (rc < 0)
		goto exit_fw_download;

	/*
                                                          
                        
  */
	msleep(400);

exit_fw_download:
	kfree(msg);

	if (coredev->postload_handler) {
		sms_debug("rc=%d, postload=0x%p", rc, coredev->postload_handler);
		if (rc >= 0)
			return coredev->postload_handler(coredev->context);
	}

	sms_debug("rc=%d", rc);
	return rc;
}

static char *smscore_fw_lkup[][DEVICE_MODE_MAX] = {
	[SMS_NOVA_A0] = {
		[DEVICE_MODE_DVBT]		= SMS_FW_DVB_NOVA_12MHZ,
		[DEVICE_MODE_DVBH]		= SMS_FW_DVB_NOVA_12MHZ,
		[DEVICE_MODE_DAB_TDMB]		= SMS_FW_TDMB_NOVA_12MHZ,
		[DEVICE_MODE_DVBT_BDA]		= SMS_FW_DVB_NOVA_12MHZ,
		[DEVICE_MODE_ISDBT]		= SMS_FW_ISDBT_NOVA_12MHZ,
		[DEVICE_MODE_ISDBT_BDA]		= SMS_FW_ISDBT_NOVA_12MHZ,
	},
	[SMS_NOVA_B0] = {
		[DEVICE_MODE_DVBT]		= SMS_FW_DVB_NOVA_12MHZ_B0,
		[DEVICE_MODE_DVBH]		= SMS_FW_DVB_NOVA_12MHZ_B0,
		[DEVICE_MODE_DAB_TDMB]		= SMS_FW_TDMB_NOVA_12MHZ_B0,
		[DEVICE_MODE_DVBT_BDA]		= SMS_FW_DVB_NOVA_12MHZ_B0,
		[DEVICE_MODE_ISDBT]		= SMS_FW_ISDBT_NOVA_12MHZ_B0,
		[DEVICE_MODE_ISDBT_BDA]		= SMS_FW_ISDBT_NOVA_12MHZ_B0,
		[DEVICE_MODE_FM_RADIO]		= SMS_FW_FM_RADIO,
		[DEVICE_MODE_FM_RADIO_BDA]	= SMS_FW_FM_RADIO,
	},
	[SMS_VEGA] = {
		[DEVICE_MODE_CMMB]		= SMS_FW_CMMB_VEGA_12MHZ,
	},
	[SMS_VENICE] = {
		[DEVICE_MODE_CMMB]		= SMS_FW_CMMB_VENICE_12MHZ,
	},
	[SMS_MING] = {
		[DEVICE_MODE_CMMB]		= SMS_FW_CMMB_MING_APP,
	},
	[SMS_PELE] = {
		[DEVICE_MODE_ISDBT]		= SMS_FW_ISDBT_PELE,
		[DEVICE_MODE_ISDBT_BDA]		= SMS_FW_ISDBT_PELE,
	},
	[SMS_RIO] = {
		[DEVICE_MODE_DVBT]		= SMS_FW_DVB_RIO,
		[DEVICE_MODE_DVBH]		= SMS_FW_DVBH_RIO,
		[DEVICE_MODE_DVBT_BDA]		= SMS_FW_DVB_RIO,
		[DEVICE_MODE_ISDBT]		= SMS_FW_ISDBT_RIO,
		[DEVICE_MODE_ISDBT_BDA]		= SMS_FW_ISDBT_RIO,
		[DEVICE_MODE_FM_RADIO]		= SMS_FW_FM_RADIO_RIO,
		[DEVICE_MODE_FM_RADIO_BDA]	= SMS_FW_FM_RADIO_RIO,
	},
	[SMS_DENVER_1530] = {
		[DEVICE_MODE_ATSC]		= SMS_FW_ATSC_DENVER,
	},
	[SMS_DENVER_2160] = {
		[DEVICE_MODE_DAB_TDMB]		= SMS_FW_TDMB_DENVER,
	},
};

/* 
                                                                        
                   
                                                         
                             
                                          
                                                                      
                                                 
  
                                     
 */
static char *smscore_get_fw_filename(struct smscore_device_t *coredev,
			      int mode)
{
	char **fw;
	int board_id = smscore_get_board_id(coredev);
	enum sms_device_type_st type;

	type = smscore_registry_gettype(coredev->devpath);

	/*                                                   */
	if (type <= SMS_UNKNOWN_TYPE || type >= SMS_NUM_OF_DEVICE_TYPES)
		return NULL;
	if (mode <= DEVICE_MODE_NONE || mode >= DEVICE_MODE_MAX)
		return NULL;

	sms_debug("trying to get fw name from sms_boards board_id %d mode %d",
		  board_id, mode);
	fw = sms_get_board(board_id)->fw;
	if (!fw || !fw[mode]) {
		sms_debug("cannot find fw name in sms_boards, getting from lookup table mode %d type %d",
			  mode, type);
		return smscore_fw_lkup[type][mode];
	}

	return fw[mode];
}

/* 
                                                                               
  
                                                         
                                         
                                                                      
                                                                 
  
                                     
 */
static int smscore_load_firmware_from_file(struct smscore_device_t *coredev,
					   int mode,
					   loadfirmware_t loadfirmware_handler)
{
	int rc = -ENOENT;
	u8 *fw_buf;
	u32 fw_buf_size;
	const struct firmware *fw;

	char *fw_filename = smscore_get_fw_filename(coredev, mode);
	if (!fw_filename) {
		sms_info("mode %d not supported on this device", mode);
		return -ENOENT;
	}
	sms_debug("Firmware name: %s", fw_filename);

	if (loadfirmware_handler == NULL && !(coredev->device_flags
			& SMS_DEVICE_FAMILY2))
		return -EINVAL;

	rc = request_firmware(&fw, fw_filename, coredev->device);
	if (rc < 0) {
		sms_info("failed to open \"%s\"", fw_filename);
		return rc;
	}
	sms_info("read fw %s, buffer size=0x%zx", fw_filename, fw->size);
	fw_buf = kmalloc(ALIGN(fw->size, SMS_ALLOC_ALIGNMENT),
			 GFP_KERNEL | GFP_DMA);
	if (!fw_buf) {
		sms_info("failed to allocate firmware buffer");
		return -ENOMEM;
	}
	memcpy(fw_buf, fw->data, fw->size);
	fw_buf_size = fw->size;

	rc = (coredev->device_flags & SMS_DEVICE_FAMILY2) ?
		smscore_load_firmware_family2(coredev, fw_buf, fw_buf_size)
		: loadfirmware_handler(coredev->context, fw_buf,
		fw_buf_size);

	kfree(fw_buf);
	release_firmware(fw);

	return rc;
}

/* 
                                                                      
                                       
  
                                                         
                                         
  
                                     
 */
void smscore_unregister_device(struct smscore_device_t *coredev)
{
	struct smscore_buffer_t *cb;
	int num_buffers = 0;
	int retry = 0;

	kmutex_lock(&g_smscore_deviceslock);

	/*                                     */
	sms_ir_exit(coredev);

	smscore_notify_clients(coredev);
	smscore_notify_callbacks(coredev, NULL, 0);

	/*                                         
                                        */

	while (1) {
		while (!list_empty(&coredev->buffers)) {
			cb = (struct smscore_buffer_t *) coredev->buffers.next;
			list_del(&cb->entry);
			kfree(cb);
			num_buffers++;
		}
		if (num_buffers == coredev->num_buffers)
			break;
		if (++retry > 10) {
			sms_info("exiting although not all buffers released.");
			break;
		}

		sms_info("waiting for %d buffer(s)",
			 coredev->num_buffers - num_buffers);
		kmutex_unlock(&g_smscore_deviceslock);
		msleep(100);
		kmutex_lock(&g_smscore_deviceslock);
	}

	sms_info("freed %d buffers", num_buffers);

	if (coredev->common_buffer)
		dma_free_coherent(NULL, coredev->common_buffer_size,
			coredev->common_buffer, coredev->common_buffer_phys);

	kfree(coredev->fw_buf);

	list_del(&coredev->entry);
	kfree(coredev);

	kmutex_unlock(&g_smscore_deviceslock);

	sms_info("device %p destroyed", coredev);
}
EXPORT_SYMBOL_GPL(smscore_unregister_device);

static int smscore_detect_mode(struct smscore_device_t *coredev)
{
	void *buffer = kmalloc(sizeof(struct sms_msg_hdr) + SMS_DMA_ALIGNMENT,
			       GFP_KERNEL | GFP_DMA);
	struct sms_msg_hdr *msg =
		(struct sms_msg_hdr *) SMS_ALIGN_ADDRESS(buffer);
	int rc;

	if (!buffer)
		return -ENOMEM;

	SMS_INIT_MSG(msg, MSG_SMS_GET_VERSION_EX_REQ,
		     sizeof(struct sms_msg_hdr));

	rc = smscore_sendrequest_and_wait(coredev, msg, msg->msg_length,
					  &coredev->version_ex_done);
	if (rc == -ETIME) {
		sms_err("MSG_SMS_GET_VERSION_EX_REQ failed first try");

		if (wait_for_completion_timeout(&coredev->resume_done,
						msecs_to_jiffies(5000))) {
			rc = smscore_sendrequest_and_wait(
				coredev, msg, msg->msg_length,
				&coredev->version_ex_done);
			if (rc < 0)
				sms_err("MSG_SMS_GET_VERSION_EX_REQ failed second try, rc %d",
					rc);
		} else
			rc = -ETIME;
	}

	kfree(buffer);

	return rc;
}

/* 
                                                 
  
                                                         
                                         
                                          
  
                                     
 */
static int smscore_init_device(struct smscore_device_t *coredev, int mode)
{
	void *buffer;
	struct sms_msg_data *msg;
	int rc = 0;

	buffer = kmalloc(sizeof(struct sms_msg_data) +
			SMS_DMA_ALIGNMENT, GFP_KERNEL | GFP_DMA);
	if (!buffer) {
		sms_err("Could not allocate buffer for init device message.");
		return -ENOMEM;
	}

	msg = (struct sms_msg_data *)SMS_ALIGN_ADDRESS(buffer);
	SMS_INIT_MSG(&msg->x_msg_header, MSG_SMS_INIT_DEVICE_REQ,
			sizeof(struct sms_msg_data));
	msg->msg_data[0] = mode;

	rc = smscore_sendrequest_and_wait(coredev, msg,
			msg->x_msg_header. msg_length,
			&coredev->init_device_done);

	kfree(buffer);
	return rc;
}

/* 
                                                   
                                                      
  
                                                         
                                         
                                          
  
                                     
 */
int smscore_set_device_mode(struct smscore_device_t *coredev, int mode)
{
	int rc = 0;

	sms_debug("set device mode to %d", mode);
	if (coredev->device_flags & SMS_DEVICE_FAMILY2) {
		if (mode <= DEVICE_MODE_NONE || mode >= DEVICE_MODE_MAX) {
			sms_err("invalid mode specified %d", mode);
			return -EINVAL;
		}

		smscore_registry_setmode(coredev->devpath, mode);

		if (!(coredev->device_flags & SMS_DEVICE_NOT_READY)) {
			rc = smscore_detect_mode(coredev);
			if (rc < 0) {
				sms_err("mode detect failed %d", rc);
				return rc;
			}
		}

		if (coredev->mode == mode) {
			sms_info("device mode %d already set", mode);
			return 0;
		}

		if (!(coredev->modes_supported & (1 << mode))) {
			rc = smscore_load_firmware_from_file(coredev,
							     mode, NULL);
			if (rc >= 0)
				sms_info("firmware download success");
		} else {
			sms_info("mode %d is already supported by running firmware",
				 mode);
		}
		if (coredev->fw_version >= 0x800) {
			rc = smscore_init_device(coredev, mode);
			if (rc < 0)
				sms_err("device init failed, rc %d.", rc);
		}
	} else {
		if (mode <= DEVICE_MODE_NONE || mode >= DEVICE_MODE_MAX) {
			sms_err("invalid mode specified %d", mode);
			return -EINVAL;
		}

		smscore_registry_setmode(coredev->devpath, mode);

		if (coredev->detectmode_handler)
			coredev->detectmode_handler(coredev->context,
						    &coredev->mode);

		if (coredev->mode != mode && coredev->setmode_handler)
			rc = coredev->setmode_handler(coredev->context, mode);
	}

	if (rc >= 0) {
		char *buffer;
		coredev->mode = mode;
		coredev->device_flags &= ~SMS_DEVICE_NOT_READY;

		buffer = kmalloc(sizeof(struct sms_msg_data) +
				 SMS_DMA_ALIGNMENT, GFP_KERNEL | GFP_DMA);
		if (buffer) {
			struct sms_msg_data *msg = (struct sms_msg_data *) SMS_ALIGN_ADDRESS(buffer);

			SMS_INIT_MSG(&msg->x_msg_header, MSG_SMS_INIT_DEVICE_REQ,
				     sizeof(struct sms_msg_data));
			msg->msg_data[0] = mode;

			rc = smscore_sendrequest_and_wait(
				coredev, msg, msg->x_msg_header.msg_length,
				&coredev->init_device_done);

			kfree(buffer);
		}
	}

	if (rc < 0)
		sms_err("return error code %d.", rc);
	else
		sms_debug("Success setting device mode.");

	return rc;
}

/* 
                                                        
  
                                                         
                                         
  
                       
 */
int smscore_get_device_mode(struct smscore_device_t *coredev)
{
	return coredev->mode;
}
EXPORT_SYMBOL_GPL(smscore_get_device_mode);

/* 
                                                             
                                
  
                                                         
                                         
                                                                  
                                                 
  
 */
static struct
smscore_client_t *smscore_find_client(struct smscore_device_t *coredev,
				      int data_type, int id)
{
	struct list_head *first;
	struct smscore_client_t *client;
	unsigned long flags;
	struct list_head *firstid;
	struct smscore_idlist_t *client_id;

	spin_lock_irqsave(&coredev->clientslock, flags);
	first = &coredev->clients;
	list_for_each_entry(client, first, entry) {
		firstid = &client->idlist;
		list_for_each_entry(client_id, firstid, entry) {
			if ((client_id->id == id) &&
			    (client_id->data_type == data_type ||
			    (client_id->data_type == 0)))
				goto found;
		}
	}
	client = NULL;
found:
	spin_unlock_irqrestore(&coredev->clientslock, flags);
	return client;
}

/* 
                                                                   
                                 
  
                                                         
                                         
                                                  
  
 */
void smscore_onresponse(struct smscore_device_t *coredev,
		struct smscore_buffer_t *cb) {
	struct sms_msg_hdr *phdr = (struct sms_msg_hdr *) ((u8 *) cb->p
			+ cb->offset);
	struct smscore_client_t *client;
	int rc = -EBUSY;
	static unsigned long last_sample_time; /*      */
	static int data_total; /*      */
	unsigned long time_now = jiffies_to_msecs(jiffies);

	if (!last_sample_time)
		last_sample_time = time_now;

	if (time_now - last_sample_time > 10000) {
		sms_debug("data rate %d bytes/secs",
			  (int)((data_total * 1000) /
				(time_now - last_sample_time)));

		last_sample_time = time_now;
		data_total = 0;
	}

	data_total += cb->size;
	/*                         */
	if ((phdr->msg_type == MSG_SMS_HO_PER_SLICES_IND) ||
			(phdr->msg_type == MSG_SMS_TRANSMISSION_IND)) {
		if (coredev->mode == DEVICE_MODE_DVBT_BDA)
			phdr->msg_dst_id = DVBT_BDA_CONTROL_MSG_ID;
	}


	client = smscore_find_client(coredev, phdr->msg_type, phdr->msg_dst_id);

	/*                                       
                                                          */
	if (client)
		rc = client->onresponse_handler(client->context, cb);

	if (rc < 0) {
		switch (phdr->msg_type) {
		case MSG_SMS_ISDBT_TUNE_RES:
			break;
		case MSG_SMS_RF_TUNE_RES:
			break;
		case MSG_SMS_SIGNAL_DETECTED_IND:
			break;
		case MSG_SMS_NO_SIGNAL_IND:
			break;
		case MSG_SMS_SPI_INT_LINE_SET_RES:
			break;
		case MSG_SMS_INTERFACE_LOCK_IND:
			break;
		case MSG_SMS_INTERFACE_UNLOCK_IND:
			break;
		case MSG_SMS_GET_VERSION_EX_RES:
		{
			struct sms_version_res *ver =
				(struct sms_version_res *) phdr;
			sms_debug("Firmware id %d prots 0x%x ver %d.%d",
				  ver->firmware_id, ver->supported_protocols,
				  ver->rom_ver_major, ver->rom_ver_minor);

			coredev->mode = ver->firmware_id == 255 ?
				DEVICE_MODE_NONE : ver->firmware_id;
			coredev->modes_supported = ver->supported_protocols;
			coredev->fw_version = ver->rom_ver_major << 8 |
					      ver->rom_ver_minor;

			complete(&coredev->version_ex_done);
			break;
		}
		case MSG_SMS_INIT_DEVICE_RES:
			complete(&coredev->init_device_done);
			break;
		case MSG_SW_RELOAD_START_RES:
			complete(&coredev->reload_start_done);
			break;
		case MSG_SMS_DATA_VALIDITY_RES:
		{
			struct sms_msg_data *validity = (struct sms_msg_data *) phdr;

			sms_err("MSG_SMS_DATA_VALIDITY_RES, checksum = 0x%x",
				validity->msg_data[0]);
			complete(&coredev->data_validity_done);
			break;
		}
		case MSG_SMS_DATA_DOWNLOAD_RES:
			complete(&coredev->data_download_done);
			break;
		case MSG_SW_RELOAD_EXEC_RES:
			break;
		case MSG_SMS_SWDOWNLOAD_TRIGGER_RES:
			complete(&coredev->trigger_done);
			break;
		case MSG_SMS_SLEEP_RESUME_COMP_IND:
			complete(&coredev->resume_done);
			break;
		case MSG_SMS_GPIO_CONFIG_EX_RES:
			complete(&coredev->gpio_configuration_done);
			break;
		case MSG_SMS_GPIO_SET_LEVEL_RES:
			complete(&coredev->gpio_set_level_done);
			break;
		case MSG_SMS_GPIO_GET_LEVEL_RES:
		{
			u32 *msgdata = (u32 *) phdr;
			coredev->gpio_get_res = msgdata[1];
			sms_debug("gpio level %d",
					coredev->gpio_get_res);
			complete(&coredev->gpio_get_level_done);
			break;
		}
		case MSG_SMS_START_IR_RES:
			complete(&coredev->ir_init_done);
			break;
		case MSG_SMS_IR_SAMPLES_IND:
			sms_ir_event(coredev,
				(const char *)
				((char *)phdr
				+ sizeof(struct sms_msg_hdr)),
				(int)phdr->msg_length
				- sizeof(struct sms_msg_hdr));
			break;

		case MSG_SMS_DVBT_BDA_DATA:
			/*
                                                 
                                                        
                                                         
                                                      
    */
			break;

		default:
			sms_debug("message %s(%d) not handled.",
				  smscore_translate_msg(phdr->msg_type),
				  phdr->msg_type);
			break;
		}
		smscore_putbuffer(coredev, cb);
	}
}
EXPORT_SYMBOL_GPL(smscore_onresponse);

/* 
                                                               
  
                                                         
                                         
  
                                                           
 */

static struct smscore_buffer_t *get_entry(struct smscore_device_t *coredev)
{
	struct smscore_buffer_t *cb = NULL;
	unsigned long flags;

	spin_lock_irqsave(&coredev->bufferslock, flags);
	if (!list_empty(&coredev->buffers)) {
		cb = (struct smscore_buffer_t *) coredev->buffers.next;
		list_del(&cb->entry);
	}
	spin_unlock_irqrestore(&coredev->bufferslock, flags);
	return cb;
}

struct smscore_buffer_t *smscore_getbuffer(struct smscore_device_t *coredev)
{
	struct smscore_buffer_t *cb = NULL;

	wait_event(coredev->buffer_mng_waitq, (cb = get_entry(coredev)));

	return cb;
}
EXPORT_SYMBOL_GPL(smscore_getbuffer);

/* 
                                     
  
                                                         
                                         
                                      
  
 */
void smscore_putbuffer(struct smscore_device_t *coredev,
		struct smscore_buffer_t *cb) {
	wake_up_interruptible(&coredev->buffer_mng_waitq);
	list_add_locked(&cb->entry, &coredev->buffers, &coredev->bufferslock);
}
EXPORT_SYMBOL_GPL(smscore_putbuffer);

static int smscore_validate_client(struct smscore_device_t *coredev,
				   struct smscore_client_t *client,
				   int data_type, int id)
{
	struct smscore_idlist_t *listentry;
	struct smscore_client_t *registered_client;

	if (!client) {
		sms_err("bad parameter.");
		return -EINVAL;
	}
	registered_client = smscore_find_client(coredev, data_type, id);
	if (registered_client == client)
		return 0;

	if (registered_client) {
		sms_err("The msg ID already registered to another client.");
		return -EEXIST;
	}
	listentry = kzalloc(sizeof(struct smscore_idlist_t), GFP_KERNEL);
	if (!listentry) {
		sms_err("Can't allocate memory for client id.");
		return -ENOMEM;
	}
	listentry->id = id;
	listentry->data_type = data_type;
	list_add_locked(&listentry->entry, &client->idlist,
			&coredev->clientslock);
	return 0;
}

/* 
                                                                     
  
                                                                  
                                                                           
                                                                          
                                                             
                                                      
                                                                               
                                         
                                                                          
  
                                     
 */
int smscore_register_client(struct smscore_device_t *coredev,
			    struct smsclient_params_t *params,
			    struct smscore_client_t **client)
{
	struct smscore_client_t *newclient;
	/*                                                         */
	if (smscore_find_client(coredev, params->data_type,
				params->initial_id)) {
		sms_err("Client already exist.");
		return -EEXIST;
	}

	newclient = kzalloc(sizeof(struct smscore_client_t), GFP_KERNEL);
	if (!newclient) {
		sms_err("Failed to allocate memory for client.");
		return -ENOMEM;
	}

	INIT_LIST_HEAD(&newclient->idlist);
	newclient->coredev = coredev;
	newclient->onresponse_handler = params->onresponse_handler;
	newclient->onremove_handler = params->onremove_handler;
	newclient->context = params->context;
	list_add_locked(&newclient->entry, &coredev->clients,
			&coredev->clientslock);
	smscore_validate_client(coredev, newclient, params->data_type,
				params->initial_id);
	*client = newclient;
	sms_debug("%p %d %d", params->context, params->data_type,
		  params->initial_id);

	return 0;
}
EXPORT_SYMBOL_GPL(smscore_register_client);

/* 
                                                               
  
                                                        
                                        
  
 */
void smscore_unregister_client(struct smscore_client_t *client)
{
	struct smscore_device_t *coredev = client->coredev;
	unsigned long flags;

	spin_lock_irqsave(&coredev->clientslock, flags);


	while (!list_empty(&client->idlist)) {
		struct smscore_idlist_t *identry =
			(struct smscore_idlist_t *) client->idlist.next;
		list_del(&identry->entry);
		kfree(identry);
	}

	sms_info("%p", client->context);

	list_del(&client->entry);
	kfree(client);

	spin_unlock_irqrestore(&coredev->clientslock, flags);
}
EXPORT_SYMBOL_GPL(smscore_unregister_client);

/* 
                                                          
                                                      
  
                                                        
                                        
                                            
                                                
  
                                     
 */
int smsclient_sendrequest(struct smscore_client_t *client,
			  void *buffer, size_t size)
{
	struct smscore_device_t *coredev;
	struct sms_msg_hdr *phdr = (struct sms_msg_hdr *) buffer;
	int rc;

	if (client == NULL) {
		sms_err("Got NULL client");
		return -EINVAL;
	}

	coredev = client->coredev;

	/*                                                 */
	if (coredev == NULL) {
		sms_err("Got NULL coredev");
		return -EINVAL;
	}

	rc = smscore_validate_client(client->coredev, client, 0,
				     phdr->msg_src_id);
	if (rc < 0)
		return rc;

	return coredev->sendrequest_handler(coredev->context, buffer, size);
}
EXPORT_SYMBOL_GPL(smsclient_sendrequest);


/*                                     */
int smscore_configure_gpio(struct smscore_device_t *coredev, u32 pin,
			   struct smscore_config_gpio *pinconfig)
{
	struct {
		struct sms_msg_hdr hdr;
		u32 data[6];
	} msg;

	if (coredev->device_flags & SMS_DEVICE_FAMILY2) {
		msg.hdr.msg_src_id = DVBT_BDA_CONTROL_MSG_ID;
		msg.hdr.msg_dst_id = HIF_TASK;
		msg.hdr.msg_flags = 0;
		msg.hdr.msg_type  = MSG_SMS_GPIO_CONFIG_EX_REQ;
		msg.hdr.msg_length = sizeof(msg);

		msg.data[0] = pin;
		msg.data[1] = pinconfig->pullupdown;

		/*                                                        */
		msg.data[2] = pinconfig->outputslewrate == 0 ? 3 : 0;

		switch (pinconfig->outputdriving) {
		case SMS_GPIO_OUTPUTDRIVING_S_16mA:
			msg.data[3] = 7; /*             */
			break;
		case SMS_GPIO_OUTPUTDRIVING_S_12mA:
			msg.data[3] = 5; /*             */
			break;
		case SMS_GPIO_OUTPUTDRIVING_S_8mA:
			msg.data[3] = 3; /*            */
			break;
		case SMS_GPIO_OUTPUTDRIVING_S_4mA:
		default:
			msg.data[3] = 2; /*            */
			break;
		}

		msg.data[4] = pinconfig->direction;
		msg.data[5] = 0;
	} else /*                          */
		return -EINVAL;

	return coredev->sendrequest_handler(coredev->context,
					    &msg, sizeof(msg));
}

int smscore_set_gpio(struct smscore_device_t *coredev, u32 pin, int level)
{
	struct {
		struct sms_msg_hdr hdr;
		u32 data[3];
	} msg;

	if (pin > MAX_GPIO_PIN_NUMBER)
		return -EINVAL;

	msg.hdr.msg_src_id = DVBT_BDA_CONTROL_MSG_ID;
	msg.hdr.msg_dst_id = HIF_TASK;
	msg.hdr.msg_flags = 0;
	msg.hdr.msg_type  = MSG_SMS_GPIO_SET_LEVEL_REQ;
	msg.hdr.msg_length = sizeof(msg);

	msg.data[0] = pin;
	msg.data[1] = level ? 1 : 0;
	msg.data[2] = 0;

	return coredev->sendrequest_handler(coredev->context,
					    &msg, sizeof(msg));
}

/*                                    */
static int get_gpio_pin_params(u32 pin_num, u32 *p_translatedpin_num,
		u32 *p_group_num, u32 *p_group_cfg) {

	*p_group_cfg = 1;

	if (pin_num <= 1)	{
		*p_translatedpin_num = 0;
		*p_group_num = 9;
		*p_group_cfg = 2;
	} else if (pin_num >= 2 && pin_num <= 6) {
		*p_translatedpin_num = 2;
		*p_group_num = 0;
		*p_group_cfg = 2;
	} else if (pin_num >= 7 && pin_num <= 11) {
		*p_translatedpin_num = 7;
		*p_group_num = 1;
	} else if (pin_num >= 12 && pin_num <= 15) {
		*p_translatedpin_num = 12;
		*p_group_num = 2;
		*p_group_cfg = 3;
	} else if (pin_num == 16) {
		*p_translatedpin_num = 16;
		*p_group_num = 23;
	} else if (pin_num >= 17 && pin_num <= 24) {
		*p_translatedpin_num = 17;
		*p_group_num = 3;
	} else if (pin_num == 25) {
		*p_translatedpin_num = 25;
		*p_group_num = 6;
	} else if (pin_num >= 26 && pin_num <= 28) {
		*p_translatedpin_num = 26;
		*p_group_num = 4;
	} else if (pin_num == 29) {
		*p_translatedpin_num = 29;
		*p_group_num = 5;
		*p_group_cfg = 2;
	} else if (pin_num == 30) {
		*p_translatedpin_num = 30;
		*p_group_num = 8;
	} else if (pin_num == 31) {
		*p_translatedpin_num = 31;
		*p_group_num = 17;
	} else
		return -1;

	*p_group_cfg <<= 24;

	return 0;
}

int smscore_gpio_configure(struct smscore_device_t *coredev, u8 pin_num,
		struct smscore_config_gpio *p_gpio_config) {

	u32 total_len;
	u32 translatedpin_num = 0;
	u32 group_num = 0;
	u32 electric_char;
	u32 group_cfg;
	void *buffer;
	int rc;

	struct set_gpio_msg {
		struct sms_msg_hdr x_msg_header;
		u32 msg_data[6];
	} *p_msg;


	if (pin_num > MAX_GPIO_PIN_NUMBER)
		return -EINVAL;

	if (p_gpio_config == NULL)
		return -EINVAL;

	total_len = sizeof(struct sms_msg_hdr) + (sizeof(u32) * 6);

	buffer = kmalloc(total_len + SMS_DMA_ALIGNMENT,
			GFP_KERNEL | GFP_DMA);
	if (!buffer)
		return -ENOMEM;

	p_msg = (struct set_gpio_msg *) SMS_ALIGN_ADDRESS(buffer);

	p_msg->x_msg_header.msg_src_id = DVBT_BDA_CONTROL_MSG_ID;
	p_msg->x_msg_header.msg_dst_id = HIF_TASK;
	p_msg->x_msg_header.msg_flags = 0;
	p_msg->x_msg_header.msg_length = (u16) total_len;
	p_msg->msg_data[0] = pin_num;

	if (!(coredev->device_flags & SMS_DEVICE_FAMILY2)) {
		p_msg->x_msg_header.msg_type = MSG_SMS_GPIO_CONFIG_REQ;
		if (get_gpio_pin_params(pin_num, &translatedpin_num, &group_num,
				&group_cfg) != 0) {
			rc = -EINVAL;
			goto free;
		}

		p_msg->msg_data[1] = translatedpin_num;
		p_msg->msg_data[2] = group_num;
		electric_char = (p_gpio_config->pullupdown)
				| (p_gpio_config->inputcharacteristics << 2)
				| (p_gpio_config->outputslewrate << 3)
				| (p_gpio_config->outputdriving << 4);
		p_msg->msg_data[3] = electric_char;
		p_msg->msg_data[4] = p_gpio_config->direction;
		p_msg->msg_data[5] = group_cfg;
	} else {
		p_msg->x_msg_header.msg_type = MSG_SMS_GPIO_CONFIG_EX_REQ;
		p_msg->msg_data[1] = p_gpio_config->pullupdown;
		p_msg->msg_data[2] = p_gpio_config->outputslewrate;
		p_msg->msg_data[3] = p_gpio_config->outputdriving;
		p_msg->msg_data[4] = p_gpio_config->direction;
		p_msg->msg_data[5] = 0;
	}

	rc = smscore_sendrequest_and_wait(coredev, p_msg, total_len,
			&coredev->gpio_configuration_done);

	if (rc != 0) {
		if (rc == -ETIME)
			sms_err("smscore_gpio_configure timeout");
		else
			sms_err("smscore_gpio_configure error");
	}
free:
	kfree(buffer);

	return rc;
}

int smscore_gpio_set_level(struct smscore_device_t *coredev, u8 pin_num,
		u8 new_level) {

	u32 total_len;
	int rc;
	void *buffer;

	struct set_gpio_msg {
		struct sms_msg_hdr x_msg_header;
		u32 msg_data[3]; /*             */
	} *p_msg;

	if ((new_level > 1) || (pin_num > MAX_GPIO_PIN_NUMBER))
		return -EINVAL;

	total_len = sizeof(struct sms_msg_hdr) +
			(3 * sizeof(u32)); /*             */

	buffer = kmalloc(total_len + SMS_DMA_ALIGNMENT,
			GFP_KERNEL | GFP_DMA);
	if (!buffer)
		return -ENOMEM;

	p_msg = (struct set_gpio_msg *) SMS_ALIGN_ADDRESS(buffer);

	p_msg->x_msg_header.msg_src_id = DVBT_BDA_CONTROL_MSG_ID;
	p_msg->x_msg_header.msg_dst_id = HIF_TASK;
	p_msg->x_msg_header.msg_flags = 0;
	p_msg->x_msg_header.msg_type = MSG_SMS_GPIO_SET_LEVEL_REQ;
	p_msg->x_msg_header.msg_length = (u16) total_len;
	p_msg->msg_data[0] = pin_num;
	p_msg->msg_data[1] = new_level;

	/*                     */
	rc = smscore_sendrequest_and_wait(coredev, p_msg, total_len,
			&coredev->gpio_set_level_done);

	if (rc != 0) {
		if (rc == -ETIME)
			sms_err("smscore_gpio_set_level timeout");
		else
			sms_err("smscore_gpio_set_level error");
	}
	kfree(buffer);

	return rc;
}

int smscore_gpio_get_level(struct smscore_device_t *coredev, u8 pin_num,
		u8 *level) {

	u32 total_len;
	int rc;
	void *buffer;

	struct set_gpio_msg {
		struct sms_msg_hdr x_msg_header;
		u32 msg_data[2];
	} *p_msg;


	if (pin_num > MAX_GPIO_PIN_NUMBER)
		return -EINVAL;

	total_len = sizeof(struct sms_msg_hdr) + (2 * sizeof(u32));

	buffer = kmalloc(total_len + SMS_DMA_ALIGNMENT,
			GFP_KERNEL | GFP_DMA);
	if (!buffer)
		return -ENOMEM;

	p_msg = (struct set_gpio_msg *) SMS_ALIGN_ADDRESS(buffer);

	p_msg->x_msg_header.msg_src_id = DVBT_BDA_CONTROL_MSG_ID;
	p_msg->x_msg_header.msg_dst_id = HIF_TASK;
	p_msg->x_msg_header.msg_flags = 0;
	p_msg->x_msg_header.msg_type = MSG_SMS_GPIO_GET_LEVEL_REQ;
	p_msg->x_msg_header.msg_length = (u16) total_len;
	p_msg->msg_data[0] = pin_num;
	p_msg->msg_data[1] = 0;

	/*                     */
	rc = smscore_sendrequest_and_wait(coredev, p_msg, total_len,
			&coredev->gpio_get_level_done);

	if (rc != 0) {
		if (rc == -ETIME)
			sms_err("smscore_gpio_get_level timeout");
		else
			sms_err("smscore_gpio_get_level error");
	}
	kfree(buffer);

	/*                                                                     
                                                                      
  */
	*level = coredev->gpio_get_res;

	return rc;
}

static int __init smscore_module_init(void)
{
	int rc = 0;

	INIT_LIST_HEAD(&g_smscore_notifyees);
	INIT_LIST_HEAD(&g_smscore_devices);
	kmutex_init(&g_smscore_deviceslock);

	INIT_LIST_HEAD(&g_smscore_registry);
	kmutex_init(&g_smscore_registrylock);

	return rc;
}

static void __exit smscore_module_exit(void)
{
	kmutex_lock(&g_smscore_deviceslock);
	while (!list_empty(&g_smscore_notifyees)) {
		struct smscore_device_notifyee_t *notifyee =
			(struct smscore_device_notifyee_t *)
				g_smscore_notifyees.next;

		list_del(&notifyee->entry);
		kfree(notifyee);
	}
	kmutex_unlock(&g_smscore_deviceslock);

	kmutex_lock(&g_smscore_registrylock);
	while (!list_empty(&g_smscore_registry)) {
		struct smscore_registry_entry_t *entry =
			(struct smscore_registry_entry_t *)
				g_smscore_registry.next;

		list_del(&entry->entry);
		kfree(entry);
	}
	kmutex_unlock(&g_smscore_registrylock);

	sms_debug("");
}

module_init(smscore_module_init);
module_exit(smscore_module_exit);

MODULE_DESCRIPTION("Siano MDTV Core module");
MODULE_AUTHOR("Siano Mobile Silicon, Inc. (uris@siano-ms.com)");
MODULE_LICENSE("GPL");

/*                                                  */
MODULE_FIRMWARE(SMS_FW_ATSC_DENVER);
MODULE_FIRMWARE(SMS_FW_CMMB_MING_APP);
MODULE_FIRMWARE(SMS_FW_CMMB_VEGA_12MHZ);
MODULE_FIRMWARE(SMS_FW_CMMB_VENICE_12MHZ);
MODULE_FIRMWARE(SMS_FW_DVBH_RIO);
MODULE_FIRMWARE(SMS_FW_DVB_NOVA_12MHZ_B0);
MODULE_FIRMWARE(SMS_FW_DVB_NOVA_12MHZ);
MODULE_FIRMWARE(SMS_FW_DVB_RIO);
MODULE_FIRMWARE(SMS_FW_FM_RADIO);
MODULE_FIRMWARE(SMS_FW_FM_RADIO_RIO);
MODULE_FIRMWARE(SMS_FW_DVBT_HCW_55XXX);
MODULE_FIRMWARE(SMS_FW_ISDBT_HCW_55XXX);
MODULE_FIRMWARE(SMS_FW_ISDBT_NOVA_12MHZ_B0);
MODULE_FIRMWARE(SMS_FW_ISDBT_NOVA_12MHZ);
MODULE_FIRMWARE(SMS_FW_ISDBT_PELE);
MODULE_FIRMWARE(SMS_FW_ISDBT_RIO);
MODULE_FIRMWARE(SMS_FW_DVBT_NOVA_A);
MODULE_FIRMWARE(SMS_FW_DVBT_NOVA_B);
MODULE_FIRMWARE(SMS_FW_DVBT_STELLAR);
MODULE_FIRMWARE(SMS_FW_TDMB_DENVER);
MODULE_FIRMWARE(SMS_FW_TDMB_NOVA_12MHZ_B0);
MODULE_FIRMWARE(SMS_FW_TDMB_NOVA_12MHZ);
