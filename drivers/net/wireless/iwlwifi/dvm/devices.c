/******************************************************************************
 *
 * Copyright(c) 2008 - 2013 Intel Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 * The full GNU General Public License is included in this distribution in the
 * file called LICENSE.
 *
 * Contact Information:
 *  Intel Linux Wireless <ilw@linux.intel.com>
 * Intel Corporation, 5200 N.E. Elam Young Parkway, Hillsboro, OR 97124-6497
 *
 *****************************************************************************/

/*
                                       
 */
#include "iwl-io.h"
#include "iwl-prph.h"
#include "iwl-eeprom-parse.h"

#include "agn.h"
#include "dev.h"
#include "commands.h"


/*
              
              
 */

/*
                                                                           
                                                        
                                                                            
                                                                   
                                                                          
              
                                                                             
                                
 */
static void iwl1000_set_ct_threshold(struct iwl_priv *priv)
{
	/*              */
	priv->hw_params.ct_kill_threshold = CT_KILL_THRESHOLD_LEGACY;
	priv->hw_params.ct_kill_exit_threshold = CT_KILL_EXIT_THRESHOLD;
}

/*                                   */
static void iwl1000_nic_config(struct iwl_priv *priv)
{
	/*                                            */
	/*                                                          */
	iwl_set_bits_mask_prph(priv->trans, APMG_DIGITAL_SVR_REG,
				APMG_SVR_DIGITAL_VOLTAGE_1_32,
				~APMG_SVR_VOLTAGE_CONFIG_BIT_MSK);
}

/* 
                                                                 
                                              
                                                         
 */
static inline u32 iwl_beacon_time_mask_low(struct iwl_priv *priv,
					   u16 tsf_bits)
{
	return (1 << tsf_bits) - 1;
}

/* 
                                                                   
                                              
                                                         
 */
static inline u32 iwl_beacon_time_mask_high(struct iwl_priv *priv,
					    u16 tsf_bits)
{
	return ((1 << (32 - tsf_bits)) - 1) << tsf_bits;
}

/*
                              
                                                                               
                                         
                                                                   
 */
static u32 iwl_usecs_to_beacons(struct iwl_priv *priv, u32 usec,
				u32 beacon_interval)
{
	u32 quot;
	u32 rem;
	u32 interval = beacon_interval * TIME_UNIT;

	if (!interval || !usec)
		return 0;

	quot = (usec / interval) &
		(iwl_beacon_time_mask_high(priv, IWLAGN_EXT_BEACON_TIME_POS) >>
		IWLAGN_EXT_BEACON_TIME_POS);
	rem = (usec % interval) & iwl_beacon_time_mask_low(priv,
				   IWLAGN_EXT_BEACON_TIME_POS);

	return (quot << IWLAGN_EXT_BEACON_TIME_POS) + rem;
}

/*                                                                 
                                             
 */
static __le32 iwl_add_beacon_time(struct iwl_priv *priv, u32 base,
			   u32 addon, u32 beacon_interval)
{
	u32 base_low = base & iwl_beacon_time_mask_low(priv,
				IWLAGN_EXT_BEACON_TIME_POS);
	u32 addon_low = addon & iwl_beacon_time_mask_low(priv,
				IWLAGN_EXT_BEACON_TIME_POS);
	u32 interval = beacon_interval * TIME_UNIT;
	u32 res = (base & iwl_beacon_time_mask_high(priv,
				IWLAGN_EXT_BEACON_TIME_POS)) +
				(addon & iwl_beacon_time_mask_high(priv,
				IWLAGN_EXT_BEACON_TIME_POS));

	if (base_low > addon_low)
		res += base_low - addon_low;
	else if (base_low < addon_low) {
		res += interval + base_low - addon_low;
		res += (1 << IWLAGN_EXT_BEACON_TIME_POS);
	} else
		res += (1 << IWLAGN_EXT_BEACON_TIME_POS);

	return cpu_to_le32(res);
}

static const struct iwl_sensitivity_ranges iwl1000_sensitivity = {
	.min_nrg_cck = 95,
	.auto_corr_min_ofdm = 90,
	.auto_corr_min_ofdm_mrc = 170,
	.auto_corr_min_ofdm_x1 = 120,
	.auto_corr_min_ofdm_mrc_x1 = 240,

	.auto_corr_max_ofdm = 120,
	.auto_corr_max_ofdm_mrc = 210,
	.auto_corr_max_ofdm_x1 = 155,
	.auto_corr_max_ofdm_mrc_x1 = 290,

	.auto_corr_min_cck = 125,
	.auto_corr_max_cck = 200,
	.auto_corr_min_cck_mrc = 170,
	.auto_corr_max_cck_mrc = 400,
	.nrg_th_cck = 95,
	.nrg_th_ofdm = 95,

	.barker_corr_th_min = 190,
	.barker_corr_th_min_mrc = 390,
	.nrg_th_cca = 62,
};

static void iwl1000_hw_set_hw_params(struct iwl_priv *priv)
{
	iwl1000_set_ct_threshold(priv);

	/*                                    */
	priv->hw_params.sens = &iwl1000_sensitivity;
}

struct iwl_lib_ops iwl1000_lib = {
	.set_hw_params = iwl1000_hw_set_hw_params,
	.nic_config = iwl1000_nic_config,
	.temperature = iwlagn_temperature,
};


/*
              
              
 */

static void iwl2000_set_ct_threshold(struct iwl_priv *priv)
{
	/*              */
	priv->hw_params.ct_kill_threshold = CT_KILL_THRESHOLD;
	priv->hw_params.ct_kill_exit_threshold = CT_KILL_EXIT_THRESHOLD;
}

/*                                   */
static void iwl2000_nic_config(struct iwl_priv *priv)
{
	iwl_set_bit(priv->trans, CSR_GP_DRIVER_REG,
		    CSR_GP_DRIVER_REG_BIT_RADIO_IQ_INVER);
}

static const struct iwl_sensitivity_ranges iwl2000_sensitivity = {
	.min_nrg_cck = 97,
	.auto_corr_min_ofdm = 80,
	.auto_corr_min_ofdm_mrc = 128,
	.auto_corr_min_ofdm_x1 = 105,
	.auto_corr_min_ofdm_mrc_x1 = 192,

	.auto_corr_max_ofdm = 145,
	.auto_corr_max_ofdm_mrc = 232,
	.auto_corr_max_ofdm_x1 = 110,
	.auto_corr_max_ofdm_mrc_x1 = 232,

	.auto_corr_min_cck = 125,
	.auto_corr_max_cck = 175,
	.auto_corr_min_cck_mrc = 160,
	.auto_corr_max_cck_mrc = 310,
	.nrg_th_cck = 97,
	.nrg_th_ofdm = 100,

	.barker_corr_th_min = 190,
	.barker_corr_th_min_mrc = 390,
	.nrg_th_cca = 62,
};

static void iwl2000_hw_set_hw_params(struct iwl_priv *priv)
{
	iwl2000_set_ct_threshold(priv);

	/*                                    */
	priv->hw_params.sens = &iwl2000_sensitivity;
}

struct iwl_lib_ops iwl2000_lib = {
	.set_hw_params = iwl2000_hw_set_hw_params,
	.nic_config = iwl2000_nic_config,
	.temperature = iwlagn_temperature,
};

struct iwl_lib_ops iwl2030_lib = {
	.set_hw_params = iwl2000_hw_set_hw_params,
	.nic_config = iwl2000_nic_config,
	.temperature = iwlagn_temperature,
};

/*
              
              
 */

/*                                   */
static const struct iwl_sensitivity_ranges iwl5000_sensitivity = {
	.min_nrg_cck = 100,
	.auto_corr_min_ofdm = 90,
	.auto_corr_min_ofdm_mrc = 170,
	.auto_corr_min_ofdm_x1 = 105,
	.auto_corr_min_ofdm_mrc_x1 = 220,

	.auto_corr_max_ofdm = 120,
	.auto_corr_max_ofdm_mrc = 210,
	.auto_corr_max_ofdm_x1 = 120,
	.auto_corr_max_ofdm_mrc_x1 = 240,

	.auto_corr_min_cck = 125,
	.auto_corr_max_cck = 200,
	.auto_corr_min_cck_mrc = 200,
	.auto_corr_max_cck_mrc = 400,
	.nrg_th_cck = 100,
	.nrg_th_ofdm = 100,

	.barker_corr_th_min = 190,
	.barker_corr_th_min_mrc = 390,
	.nrg_th_cca = 62,
};

static struct iwl_sensitivity_ranges iwl5150_sensitivity = {
	.min_nrg_cck = 95,
	.auto_corr_min_ofdm = 90,
	.auto_corr_min_ofdm_mrc = 170,
	.auto_corr_min_ofdm_x1 = 105,
	.auto_corr_min_ofdm_mrc_x1 = 220,

	.auto_corr_max_ofdm = 120,
	.auto_corr_max_ofdm_mrc = 210,
	/*                                           */
	.auto_corr_max_ofdm_x1 = 105,
	.auto_corr_max_ofdm_mrc_x1 = 220,

	.auto_corr_min_cck = 125,
	.auto_corr_max_cck = 200,
	.auto_corr_min_cck_mrc = 170,
	.auto_corr_max_cck_mrc = 400,
	.nrg_th_cck = 95,
	.nrg_th_ofdm = 95,

	.barker_corr_th_min = 190,
	.barker_corr_th_min_mrc = 390,
	.nrg_th_cca = 62,
};

#define IWL_5150_VOLTAGE_TO_TEMPERATURE_COEFF	(-5)

static s32 iwl_temp_calib_to_offset(struct iwl_priv *priv)
{
	u16 temperature, voltage;

	temperature = le16_to_cpu(priv->nvm_data->kelvin_temperature);
	voltage = le16_to_cpu(priv->nvm_data->kelvin_voltage);

	/*                              */
	return (s32)(temperature -
			voltage / IWL_5150_VOLTAGE_TO_TEMPERATURE_COEFF);
}

static void iwl5150_set_ct_threshold(struct iwl_priv *priv)
{
	const s32 volt2temp_coef = IWL_5150_VOLTAGE_TO_TEMPERATURE_COEFF;
	s32 threshold = (s32)CELSIUS_TO_KELVIN(CT_KILL_THRESHOLD_LEGACY) -
			iwl_temp_calib_to_offset(priv);

	priv->hw_params.ct_kill_threshold = threshold * volt2temp_coef;
}

static void iwl5000_set_ct_threshold(struct iwl_priv *priv)
{
	/*              */
	priv->hw_params.ct_kill_threshold = CT_KILL_THRESHOLD_LEGACY;
}

static void iwl5000_hw_set_hw_params(struct iwl_priv *priv)
{
	iwl5000_set_ct_threshold(priv);

	/*                                    */
	priv->hw_params.sens = &iwl5000_sensitivity;
}

static void iwl5150_hw_set_hw_params(struct iwl_priv *priv)
{
	iwl5150_set_ct_threshold(priv);

	/*                                    */
	priv->hw_params.sens = &iwl5150_sensitivity;
}

static void iwl5150_temperature(struct iwl_priv *priv)
{
	u32 vt = 0;
	s32 offset =  iwl_temp_calib_to_offset(priv);

	vt = le32_to_cpu(priv->statistics.common.temperature);
	vt = vt / IWL_5150_VOLTAGE_TO_TEMPERATURE_COEFF + offset;
	/*                                       */
	priv->temperature = KELVIN_TO_CELSIUS(vt);
	iwl_tt_handler(priv);
}

static int iwl5000_hw_channel_switch(struct iwl_priv *priv,
				     struct ieee80211_channel_switch *ch_switch)
{
	/*
               
                                  
  */
	struct iwl_rxon_context *ctx = &priv->contexts[IWL_RXON_CTX_BSS];
	struct iwl5000_channel_switch_cmd cmd;
	u32 switch_time_in_usec, ucode_switch_time;
	u16 ch;
	u32 tsf_low;
	u8 switch_count;
	u16 beacon_interval = le16_to_cpu(ctx->timing.beacon_interval);
	struct ieee80211_vif *vif = ctx->vif;
	struct iwl_host_cmd hcmd = {
		.id = REPLY_CHANNEL_SWITCH,
		.len = { sizeof(cmd), },
		.flags = CMD_SYNC,
		.data = { &cmd, },
	};

	cmd.band = priv->band == IEEE80211_BAND_2GHZ;
	ch = ch_switch->chandef.chan->hw_value;
	IWL_DEBUG_11H(priv, "channel switch from %d to %d\n",
		      ctx->active.channel, ch);
	cmd.channel = cpu_to_le16(ch);
	cmd.rxon_flags = ctx->staging.flags;
	cmd.rxon_filter_flags = ctx->staging.filter_flags;
	switch_count = ch_switch->count;
	tsf_low = ch_switch->timestamp & 0x0ffffffff;
	/*
                                           
                                                      
  */
	if ((priv->ucode_beacon_time > tsf_low) && beacon_interval) {
		if (switch_count > ((priv->ucode_beacon_time - tsf_low) /
		    beacon_interval)) {
			switch_count -= (priv->ucode_beacon_time -
				tsf_low) / beacon_interval;
		} else
			switch_count = 0;
	}
	if (switch_count <= 1)
		cmd.switch_time = cpu_to_le32(priv->ucode_beacon_time);
	else {
		switch_time_in_usec =
			vif->bss_conf.beacon_int * switch_count * TIME_UNIT;
		ucode_switch_time = iwl_usecs_to_beacons(priv,
							 switch_time_in_usec,
							 beacon_interval);
		cmd.switch_time = iwl_add_beacon_time(priv,
						      priv->ucode_beacon_time,
						      ucode_switch_time,
						      beacon_interval);
	}
	IWL_DEBUG_11H(priv, "uCode time for the switch is 0x%x\n",
		      cmd.switch_time);
	cmd.expect_beacon =
		ch_switch->chandef.chan->flags & IEEE80211_CHAN_RADAR;

	return iwl_dvm_send_cmd(priv, &hcmd);
}

struct iwl_lib_ops iwl5000_lib = {
	.set_hw_params = iwl5000_hw_set_hw_params,
	.set_channel_switch = iwl5000_hw_channel_switch,
	.temperature = iwlagn_temperature,
};

struct iwl_lib_ops iwl5150_lib = {
	.set_hw_params = iwl5150_hw_set_hw_params,
	.set_channel_switch = iwl5000_hw_channel_switch,
	.temperature = iwl5150_temperature,
};



/*
              
              
 */

static void iwl6000_set_ct_threshold(struct iwl_priv *priv)
{
	/*              */
	priv->hw_params.ct_kill_threshold = CT_KILL_THRESHOLD;
	priv->hw_params.ct_kill_exit_threshold = CT_KILL_EXIT_THRESHOLD;
}

/*                                   */
static void iwl6000_nic_config(struct iwl_priv *priv)
{
	switch (priv->cfg->device_family) {
	case IWL_DEVICE_FAMILY_6005:
	case IWL_DEVICE_FAMILY_6030:
	case IWL_DEVICE_FAMILY_6000:
		break;
	case IWL_DEVICE_FAMILY_6000i:
		/*                  */
		iwl_write32(priv->trans, CSR_GP_DRIVER_REG,
			     CSR_GP_DRIVER_REG_BIT_RADIO_SKU_2x2_IPA);
		break;
	case IWL_DEVICE_FAMILY_6050:
		/*                                        */
		if (priv->nvm_data->calib_version >= 6)
			iwl_set_bit(priv->trans, CSR_GP_DRIVER_REG,
					CSR_GP_DRIVER_REG_BIT_CALIB_VERSION6);
		break;
	case IWL_DEVICE_FAMILY_6150:
		/*                                        */
		if (priv->nvm_data->calib_version >= 6)
			iwl_set_bit(priv->trans, CSR_GP_DRIVER_REG,
					CSR_GP_DRIVER_REG_BIT_CALIB_VERSION6);
		iwl_set_bit(priv->trans, CSR_GP_DRIVER_REG,
			    CSR_GP_DRIVER_REG_BIT_6050_1x2);
		break;
	default:
		WARN_ON(1);
	}
}

static const struct iwl_sensitivity_ranges iwl6000_sensitivity = {
	.min_nrg_cck = 110,
	.auto_corr_min_ofdm = 80,
	.auto_corr_min_ofdm_mrc = 128,
	.auto_corr_min_ofdm_x1 = 105,
	.auto_corr_min_ofdm_mrc_x1 = 192,

	.auto_corr_max_ofdm = 145,
	.auto_corr_max_ofdm_mrc = 232,
	.auto_corr_max_ofdm_x1 = 110,
	.auto_corr_max_ofdm_mrc_x1 = 232,

	.auto_corr_min_cck = 125,
	.auto_corr_max_cck = 175,
	.auto_corr_min_cck_mrc = 160,
	.auto_corr_max_cck_mrc = 310,
	.nrg_th_cck = 110,
	.nrg_th_ofdm = 110,

	.barker_corr_th_min = 190,
	.barker_corr_th_min_mrc = 336,
	.nrg_th_cca = 62,
};

static void iwl6000_hw_set_hw_params(struct iwl_priv *priv)
{
	iwl6000_set_ct_threshold(priv);

	/*                                    */
	priv->hw_params.sens = &iwl6000_sensitivity;

}

static int iwl6000_hw_channel_switch(struct iwl_priv *priv,
				     struct ieee80211_channel_switch *ch_switch)
{
	/*
               
                                  
  */
	struct iwl_rxon_context *ctx = &priv->contexts[IWL_RXON_CTX_BSS];
	struct iwl6000_channel_switch_cmd *cmd;
	u32 switch_time_in_usec, ucode_switch_time;
	u16 ch;
	u32 tsf_low;
	u8 switch_count;
	u16 beacon_interval = le16_to_cpu(ctx->timing.beacon_interval);
	struct ieee80211_vif *vif = ctx->vif;
	struct iwl_host_cmd hcmd = {
		.id = REPLY_CHANNEL_SWITCH,
		.len = { sizeof(*cmd), },
		.flags = CMD_SYNC,
		.dataflags[0] = IWL_HCMD_DFL_NOCOPY,
	};
	int err;

	cmd = kzalloc(sizeof(*cmd), GFP_KERNEL);
	if (!cmd)
		return -ENOMEM;

	hcmd.data[0] = cmd;

	cmd->band = priv->band == IEEE80211_BAND_2GHZ;
	ch = ch_switch->chandef.chan->hw_value;
	IWL_DEBUG_11H(priv, "channel switch from %u to %u\n",
		      ctx->active.channel, ch);
	cmd->channel = cpu_to_le16(ch);
	cmd->rxon_flags = ctx->staging.flags;
	cmd->rxon_filter_flags = ctx->staging.filter_flags;
	switch_count = ch_switch->count;
	tsf_low = ch_switch->timestamp & 0x0ffffffff;
	/*
                                           
                                                      
  */
	if ((priv->ucode_beacon_time > tsf_low) && beacon_interval) {
		if (switch_count > ((priv->ucode_beacon_time - tsf_low) /
		    beacon_interval)) {
			switch_count -= (priv->ucode_beacon_time -
				tsf_low) / beacon_interval;
		} else
			switch_count = 0;
	}
	if (switch_count <= 1)
		cmd->switch_time = cpu_to_le32(priv->ucode_beacon_time);
	else {
		switch_time_in_usec =
			vif->bss_conf.beacon_int * switch_count * TIME_UNIT;
		ucode_switch_time = iwl_usecs_to_beacons(priv,
							 switch_time_in_usec,
							 beacon_interval);
		cmd->switch_time = iwl_add_beacon_time(priv,
						       priv->ucode_beacon_time,
						       ucode_switch_time,
						       beacon_interval);
	}
	IWL_DEBUG_11H(priv, "uCode time for the switch is 0x%x\n",
		      cmd->switch_time);
	cmd->expect_beacon =
		ch_switch->chandef.chan->flags & IEEE80211_CHAN_RADAR;

	err = iwl_dvm_send_cmd(priv, &hcmd);
	kfree(cmd);
	return err;
}

struct iwl_lib_ops iwl6000_lib = {
	.set_hw_params = iwl6000_hw_set_hw_params,
	.set_channel_switch = iwl6000_hw_channel_switch,
	.nic_config = iwl6000_nic_config,
	.temperature = iwlagn_temperature,
};

struct iwl_lib_ops iwl6030_lib = {
	.set_hw_params = iwl6000_hw_set_hw_params,
	.set_channel_switch = iwl6000_hw_channel_switch,
	.nic_config = iwl6000_nic_config,
	.temperature = iwlagn_temperature,
};
