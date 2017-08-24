/*
                                                                                      
*/

/*                        
                                                                
*/



/*
                         
  
                        
                                                                
                                              
  
                        
       
                      
  
                        
       
                                     
  
                        
                                                                                                   
                                         
  
                   
       
                                                                             
  
                        
       
                                                                  
  
                        
       
                                    
  
                        
       
                                
  
                        
                                                                                                                                         
                                                                      
                                            
  
                      
                                                                                        
    
  
                        
                                                          
                 

  
                           
       
                                                             
  
                        
                                                                                 
                             
  
                           
                                                                                            
    
  
                   
                                                                                      
                                                   
  
                          
                                                                                         
                                
  
                   
                                                                             
                                     
  
                   
                                                                             
                                   
  
                   
                                                                                       
                                                             
  
                   
                                                                                       
                                                     
  
                   
                                                                                       
                                                                                  
  
                   
                                                                                    
                                   
  
                   
                                                                                                                                 
                                                  
  
                   
                                                                                                                                 
                                                             
                                                                                    
  
                   
       
                                                                     
  
                   
                                                                                                      
                                                      
  
                   
                                                                     
                                      
  
                   
                                                                         
                                               
  
                         
                                                                                 
                                                            
  
                          
       
                           
  
                   
       
                                                                 
  
                   
       
                                                         
  
                        
       
                                    
  
                   
       
                                                                                                          
  
                          
       
                                                                               
  
                   
       
                                                      
  
                   
       
                                                                     
  
                   
       
                                                                     
  
                   
       
                                                
  
                   
  
                                                                               
  
                         
                                                
                                   
  
                   
                                                                     
                                                      
  
                   
                                                                     
                                                                        
  
                   
                                                                     
                             
  
                   
                                                                     
                                                          
  
                   
                                                                     
                                                                                    
  
                   
                                                                     
                      
  
                   
                                                                     
                                                      
  
*/

#ifndef _GL_P2P_IOCTL_H
#define _GL_P2P_IOCTL_H

/*                                                                              
                                                     
                                                                                
*/

/*                                                                              
                                                          
                                                                                
*/
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/wireless.h>
#if CFG_ENABLE_WIFI_DIRECT_CFG_80211
#include <linux/ieee80211.h>
#include <net/cfg80211.h>
#endif

#include "wlan_oid.h"

/*                                                                              
                                                
                                                                                
*/

/*                                          */
#define IOC_P2P_CFG_DEVICE              (SIOCIWFIRSTPRIV+0)
#define IOC_P2P_PROVISION_COMPLETE      (SIOCIWFIRSTPRIV+2)
#define IOC_P2P_START_STOP_DISCOVERY    (SIOCIWFIRSTPRIV+4)
#define IOC_P2P_DISCOVERY_RESULTS       (SIOCIWFIRSTPRIV+5)
#define IOC_P2P_WSC_BEACON_PROBE_RSP_IE (SIOCIWFIRSTPRIV+6)
#define IOC_P2P_GO_WSC_IE               IOC_P2P_WSC_BEACON_PROBE_RSP_IE
#define IOC_P2P_CONNECT_DISCONNECT      (SIOCIWFIRSTPRIV+8)
#define IOC_P2P_PASSWORD_READY          (SIOCIWFIRSTPRIV+10)
/*                                                              */
#define IOC_P2P_SET_INT                 (SIOCIWFIRSTPRIV+12)
#define IOC_P2P_GET_STRUCT              (SIOCIWFIRSTPRIV+13)
#define IOC_P2P_SET_STRUCT              (SIOCIWFIRSTPRIV+14)
#define IOC_P2P_GET_REQ_DEVICE_INFO     (SIOCIWFIRSTPRIV+15)

#define PRIV_CMD_INT_P2P_SET            0

/*                                               */
#define P2P_PROVISIONING_SUCCESS        0
#define P2P_PROVISIONING_FAIL           1

/*                                                 */
#define P2P_STOP_DISCOVERY              0
#define P2P_START_DISCOVERY             1

/*                                               */
#define P2P_CONNECT                     0
#define P2P_DISCONNECT                  1

/*                                          */
#define P2P_SCAN_FULL_AND_FIND          0
#define P2P_SCAN_FULL                   1
#define P2P_SCAN_SEARCH_AND_LISTEN      2
#define P2P_LISTEN                      3

/*                                       */
#define P2P_SEND_SD_RESPONSE            0
#define P2P_GET_SD_REQUEST              1
#define P2P_SEND_SD_REQUEST             2
#define P2P_GET_SD_RESPONSE             3
#define P2P_TERMINATE_SD_PHASE          4

/*                                                                              
                                                 
                                                                                
*/
/*                                                                            */
/*                                                                            */
/*                                                                            */
typedef struct iw_p2p_cfg_device_type {
	void __user *ssid;
	UINT_8 ssid_len;
	UINT_8 pri_device_type[8];
	UINT_8 snd_device_type[8];
	void __user *device_name;
	UINT_8 device_name_len;
	UINT_8 intend;
	UINT_8 persistence;
	UINT_8 sec_mode;
	UINT_8 ch;
	UINT_8 ch_width;	/*                             */
	UINT_8 max_scb;
} IW_P2P_CFG_DEVICE_TYPE, *P_IW_P2P_CFG_DEVICE_TYPE;

typedef struct iw_p2p_hostapd_param {
	UINT_8 cmd;
	UINT_8 rsv[3];
	UINT_8 sta_addr[6];
	void __user *data;
	UINT_16 len;
} IW_P2P_HOSTAPD_PARAM, *P_IW_P2P_HOSTAPD_PARAM;

typedef struct iw_p2p_req_device_type {
	UINT_8 scan_type;	/*                    
                   
                               
                
                       
     */
	UINT_8 pri_device_type[8];
	void __user *probe_req_ie;
	UINT_16 probe_req_len;
	void __user *probe_rsp_ie;
	UINT_16 probe_rsp_len;
} IW_P2P_REQ_DEVICE_TYPE, *P_IW_P2P_REQ_DEVICE_TYPE;

typedef struct iw_p2p_connect_device {
	UINT_8 sta_addr[6];
	UINT_8 p2pRole;		/*                            */
	UINT_8 needProvision;	/*                                                             */
	UINT_8 authPeer;	/*                                 */
	UINT_8 intend_config_method;	/*                                        */
} IW_P2P_CONNECT_DEVICE, *P_IW_P2P_CONNECT_DEVICE;

typedef struct iw_p2p_password_ready {
	UINT_8 active_config_method;
	void __user *probe_req_ie;
	UINT_16 probe_req_len;
	void __user *probe_rsp_ie;
	UINT_16 probe_rsp_len;
} IW_P2P_PASSWORD_READY, *P_IW_P2P_PASSWORD_READY;

typedef struct iw_p2p_device_req {
	UINT_8 name[33];
	UINT_32 name_len;
	UINT_8 device_addr[6];
	UINT_8 device_type;
	INT_32 config_method;
	INT_32 active_config_method;
} IW_P2P_DEVICE_REQ, *P_IW_P2P_DEVICE_REQ;

typedef struct iw_p2p_transport_struct {
	UINT_32 u4CmdId;
	UINT_32 inBufferLength;
	UINT_32 outBufferLength;
	UINT_8 aucBuffer[16];
} IW_P2P_TRANSPORT_STRUCT, *P_IW_P2P_TRANSPORT_STRUCT;

/*                */
typedef struct iw_p2p_ioctl_invitation_struct {
	UINT_8 aucDeviceID[6];
	UINT_8 aucGroupID[6];	/*       */
	UINT_8 aucSsid[32];
	UINT_32 u4SsidLen;
	UINT_8 ucReinvoke;
} IW_P2P_IOCTL_INVITATION_STRUCT, *P_IW_P2P_IOCTL_INVITATION_STRUCT;

typedef struct iw_p2p_ioctl_abort_invitation {
	UINT_8 dev_addr[6];
} IW_P2P_IOCTL_ABORT_INVITATION, *P_IW_P2P_IOCTL_ABORT_INVITATION;

typedef struct iw_p2p_ioctl_invitation_indicate {
	UINT_8 dev_addr[6];
	UINT_8 group_bssid[6];
	INT_32 config_method;	/*                                     */
	UINT_8 dev_name[32];	/*              */
	UINT_32 name_len;
	UINT_8 operating_channel;	/*                                         */
	UINT_8 invitation_type;	/*                         */
} IW_P2P_IOCTL_INVITATION_INDICATE, *P_IW_P2P_IOCTL_INVITATION_INDICATE;

typedef struct iw_p2p_ioctl_invitation_status {
	UINT_32 status_code;
} IW_P2P_IOCTL_INVITATION_STATUS, *P_IW_P2P_IOCTL_INVITATION_STATUS;

/*               */
typedef struct iw_p2p_ioctl_start_formation {
	UINT_8 dev_addr[6];	/*       */
	UINT_8 role;		/*                            */
	UINT_8 needProvision;	/*                                                             */
	UINT_8 auth;		/*                                 */
	UINT_8 config_method;	/*                                        */
} IW_P2P_IOCTL_START_FORMATION, *P_IW_P2P_IOCTL_START_FORMATION;

/*                         */
typedef enum _ENUM_P2P_CMD_ID_T {
	P2P_CMD_ID_SEND_SD_RESPONSE = 0,	/*            */
	P2P_CMD_ID_GET_SD_REQUEST,	/*            */
	P2P_CMD_ID_SEND_SD_REQUEST,	/*            */
	P2P_CMD_ID_GET_SD_RESPONSE,	/*            */
	P2P_CMD_ID_TERMINATE_SD_PHASE,	/*            */
#if 1				/*                         */
	P2P_CMD_ID_SEC_CHECK,	/*           */
#endif
	P2P_CMD_ID_INVITATION,	/*            */
	P2P_CMD_ID_INVITATION_INDICATE,	/*            */
	P2P_CMD_ID_INVITATION_STATUS,	/*            */
	P2P_CMD_ID_INVITATION_ABORT,	/*            */
	P2P_CMD_ID_START_FORMATION,	/*            */
	P2P_CMD_ID_P2P_VERSION,	/*                */
	P2P_CMD_ID_GET_CH_LIST = 12,	/*            */
	P2P_CMD_ID_GET_OP_CH = 14	/*            */
} ENUM_P2P_CMD_ID_T, *P_ENUM_P2P_CMD_ID_T;

/*                   */
typedef struct iw_p2p_cmd_send_sd_response {
	PARAM_MAC_ADDRESS rReceiverAddr;
	UINT_8 fgNeedTxDoneIndication;
	UINT_8 ucSeqNum;
	UINT_16 u2PacketLength;
	UINT_8 aucPacketContent[0];	/*              */
} IW_P2P_CMD_SEND_SD_RESPONSE, *P_IW_P2P_CMD_SEND_SD_RESPONSE;

typedef struct iw_p2p_cmd_get_sd_request {
	PARAM_MAC_ADDRESS rTransmitterAddr;
	UINT_16 u2PacketLength;
	UINT_8 aucPacketContent[0];	/*              */
} IW_P2P_CMD_GET_SD_REQUEST, *P_IW_P2P_CMD_GET_SD_REQUEST;

typedef struct iw_p2p_cmd_send_service_discovery_request {
	PARAM_MAC_ADDRESS rReceiverAddr;
	UINT_8 fgNeedTxDoneIndication;
	UINT_8 ucSeqNum;
	UINT_16 u2PacketLength;
	UINT_8 aucPacketContent[0];	/*              */
} IW_P2P_CMD_SEND_SD_REQUEST, *P_IW_P2P_CMD_SEND_SD_REQUEST;

typedef struct iw_p2p_cmd_get_sd_response {
	PARAM_MAC_ADDRESS rTransmitterAddr;
	UINT_16 u2PacketLength;
	UINT_8 aucPacketContent[0];	/*              */
} IW_P2P_CMD_GET_SD_RESPONSE, *P_IW_P2P_CMD_GET_SD_RESPONSE;

typedef struct iw_p2p_cmd_terminate_sd_phase {
	PARAM_MAC_ADDRESS rPeerAddr;
} IW_P2P_CMD_TERMINATE_SD_PHASE, *P_IW_P2P_CMD_TERMINATE_SD_PHASE;

typedef struct iw_p2p_version {
	UINT_32 u4Version;
} IW_P2P_VERSION, *P_IW_P2P_VERSION;

/*                                                                              
                                                  
                                                                                
*/

/*                                                                              
                                                   
                                                                                
*/

/*                                                                              
                                             
                                                                                
*/
/*                          */
#define RATETAB_ENT(_rate, _rateid, _flags) \
    {                                       \
	.bitrate    = (_rate),              \
	.hw_value   = (_rateid),            \
	.flags      = (_flags),             \
    }

#define CHAN2G(_channel, _freq, _flags)             \
    {                                               \
	.band               = IEEE80211_BAND_2GHZ,  \
	.center_freq        = (_freq),              \
	.hw_value           = (_channel),           \
	.flags              = (_flags),             \
	.max_antenna_gain   = 0,                    \
	.max_power          = 30,                   \
    }

#define CHAN5G(_channel, _flags)                        \
    {                                                   \
	.band               = IEEE80211_BAND_5GHZ,      \
	.center_freq        = 5000 + (5 * (_channel)),  \
	.hw_value           = (_channel),               \
	.flags              = (_flags),                 \
	.max_antenna_gain   = 0,                        \
	.max_power          = 30,                       \
    }

/*                                                                              
                                                            
                                                                                
*/

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32) && (CFG_ENABLE_WIFI_DIRECT_CFG_80211 != 0)
int mtk_p2p_cfg80211_change_iface(struct wiphy *wiphy,
				  struct net_device *ndev,
				  enum nl80211_iftype type, u32 *flags, struct vif_params *params);

int mtk_p2p_cfg80211_add_key(struct wiphy *wiphy,
			     struct net_device *ndev,
			     u8 key_index,
			     bool pairwise, const u8 *mac_addr, struct key_params *params);

int mtk_p2p_cfg80211_get_key(struct wiphy *wiphy,
			     struct net_device *ndev,
			     u8 key_index,
			     bool pairwise,
			     const u8 *mac_addr,
			     void *cookie, void (*callback) (void *cookie, struct key_params *)
    );

int mtk_p2p_cfg80211_del_key(struct wiphy *wiphy,
			     struct net_device *ndev,
			     u8 key_index, bool pairwise, const u8 *mac_addr);

int
mtk_p2p_cfg80211_set_default_key(struct wiphy *wiphy,
				 struct net_device *netdev,
				 u8 key_index, bool unicast, bool multicast);


int mtk_p2p_cfg80211_get_station(struct wiphy *wiphy,
				 struct net_device *ndev, u8 *mac, struct station_info *sinfo);


int mtk_p2p_cfg80211_set_wiphy_params(struct wiphy *wiphy, u32 changed);

int mtk_p2p_cfg80211_connect(struct wiphy *wiphy,
			     struct net_device *dev, struct cfg80211_connect_params *sme);

int mtk_p2p_cfg80211_disconnect(struct wiphy *wiphy, struct net_device *dev, u16 reason_code);

int mtk_p2p_cfg80211_join_ibss(struct wiphy *wiphy,
			       struct net_device *dev, struct cfg80211_ibss_params *params);

int mtk_p2p_cfg80211_leave_ibss(struct wiphy *wiphy, struct net_device *dev);


int mtk_p2p_cfg80211_set_power_mgmt(struct wiphy *wiphy,
				    struct net_device *dev, bool enabled, int timeout);

int
mtk_p2p_cfg80211_change_bss(struct wiphy *wiphy,
			    struct net_device *dev, struct bss_parameters *params);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0)

int mtk_p2p_cfg80211_remain_on_channel(struct wiphy *wiphy,
				       struct wireless_dev *wdev,
				       struct ieee80211_channel *chan,
				       unsigned int duration, u64 *cookie);

int mtk_p2p_cfg80211_scan(struct wiphy *wiphy, struct cfg80211_scan_request *request);

int mtk_p2p_cfg80211_cancel_remain_on_channel(struct wiphy *wiphy,
					      struct wireless_dev *wdev, u64 cookie);

int mtk_p2p_cfg80211_set_txpower(struct wiphy *wiphy,
				 struct wireless_dev *wdev,
				 enum nl80211_tx_power_setting type, int mbm);


int mtk_p2p_cfg80211_get_txpower(struct wiphy *wiphy, struct wireless_dev *wdev, int *dbm);


#else
int
mtk_p2p_cfg80211_remain_on_channel(struct wiphy *wiphy,
				   struct net_device *dev,
				   struct ieee80211_channel *chan,
				   enum nl80211_channel_type channel_type,
				   unsigned int duration, u64 *cookie);

int mtk_p2p_cfg80211_scan(struct wiphy *wiphy,
			  struct net_device *ndev, struct cfg80211_scan_request *request);

int
mtk_p2p_cfg80211_cancel_remain_on_channel(struct wiphy *wiphy, struct net_device *dev, u64 cookie);
int mtk_p2p_cfg80211_set_txpower(struct wiphy *wiphy, enum nl80211_tx_power_setting type, int mbm);

int mtk_p2p_cfg80211_get_txpower(struct wiphy *wiphy, int *dbm);

#endif





int
mtk_p2p_cfg80211_deauth(struct wiphy *wiphy,
			struct net_device *dev, struct cfg80211_deauth_request *req
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 2, 0)
			, void *cookie
#endif
    );


int
mtk_p2p_cfg80211_disassoc(struct wiphy *wiphy,
			  struct net_device *dev, struct cfg80211_disassoc_request *req
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 2, 0)
			  , void *cookie
#endif
    );
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 2, 0)

int
mtk_p2p_cfg80211_start_ap(struct wiphy *wiphy,
			  struct net_device *dev, struct cfg80211_ap_settings *settings);


int
mtk_p2p_cfg80211_change_beacon(struct wiphy *wiphy,
			       struct net_device *dev, struct cfg80211_beacon_data *info);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0)

int mtk_p2p_cfg80211_mgmt_tx(struct wiphy *wiphy,
			     struct wireless_dev *wdev,
			     struct ieee80211_channel *chan,
			     bool offchan,
			     unsigned int wait,
			     const u8 *buf,
			     size_t len, bool no_cck, bool dont_wait_for_ack, u64 *cookie);
#else

int mtk_p2p_cfg80211_mgmt_tx(struct wiphy *wiphy,
			     struct net_device *dev,
			     struct ieee80211_channel *chan,
			     bool offchan,
			     enum nl80211_channel_type channel_type,
			     bool channel_type_valid,
			     unsigned int wait,
			     const u8 *buf,
			     size_t len, bool no_cck, bool dont_wait_for_ack, u64 *cookie);
#endif

#else
int
mtk_p2p_cfg80211_add_set_beacon(struct wiphy *wiphy,
				struct net_device *dev, struct beacon_parameters *info);

int
mtk_p2p_cfg80211_mgmt_tx(struct wiphy *wiphy,
			 struct net_device *dev,
			 struct ieee80211_channel *chan,
			 bool offchan,
			 enum nl80211_channel_type channel_type,
			 bool channel_type_valid,
			 unsigned int wait, const u8 *buf, size_t len, u64 *cookie);

#endif


int mtk_p2p_cfg80211_stop_ap(struct wiphy *wiphy, struct net_device *dev);



int mtk_p2p_cfg80211_del_station(struct wiphy *wiphy, struct net_device *dev, u8 *mac);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0)
int mtk_p2p_cfg80211_set_channel(struct wiphy *wiphy, struct cfg80211_chan_def *chandef);

void mtk_p2p_cfg80211_mgmt_frame_register(struct wiphy *wiphy,
					  struct wireless_dev *wdev, u16 frame_type, bool reg);

#else
int
mtk_p2p_cfg80211_set_channel(IN struct wiphy *wiphy,
			     IN struct net_device *dev,
			     IN struct ieee80211_channel *chan,
			     IN enum nl80211_channel_type channel_type);

void
mtk_p2p_cfg80211_mgmt_frame_register(IN struct wiphy *wiphy,
				     IN struct net_device *dev, IN u16 frame_type, IN bool reg);

#endif
int
mtk_p2p_cfg80211_set_bitrate_mask(IN struct wiphy *wiphy,
				  IN struct net_device *dev,
				  IN const u8 *peer, IN const struct cfg80211_bitrate_mask *mask);




#if CONFIG_NL80211_TESTMODE
int mtk_p2p_cfg80211_testmode_cmd(IN struct wiphy *wiphy, IN void *data, IN int len);
int mtk_p2p_cfg80211_testmode_p2p_sigma_pre_cmd(IN struct wiphy *wiphy, IN void *data, IN int len);
int mtk_p2p_cfg80211_testmode_p2p_sigma_cmd(IN struct wiphy *wiphy, IN void *data, IN int len);

#if CFG_SUPPORT_WFD
int mtk_p2p_cfg80211_testmode_wfd_update_cmd(IN struct wiphy *wiphy, IN void *data, IN int len);
#endif

int
mtk_p2p_cfg80211_testmode_hotspot_block_cmd(IN struct wiphy *wiphy, IN void *data, IN int len);
#else
#error "Please ENABLE kernel config (CONFIG_NL80211_TESTMODE) to support Wi-Fi Direct"
#endif

#endif

/*                      */

int
mtk_p2p_wext_get_priv(IN struct net_device *prDev,
		      IN struct iw_request_info *info,
		      IN OUT union iwreq_data *wrqu, IN OUT char *extra);

int
mtk_p2p_wext_reconnect(IN struct net_device *prDev,
		       IN struct iw_request_info *info,
		       IN OUT union iwreq_data *wrqu, IN OUT char *extra);

int
mtk_p2p_wext_set_auth(IN struct net_device *prDev,
		      IN struct iw_request_info *info,
		      IN OUT union iwreq_data *wrqu, IN OUT char *extra);

int
mtk_p2p_wext_set_key(IN struct net_device *prDev,
		     IN struct iw_request_info *info,
		     IN OUT union iwreq_data *wrqu, IN OUT char *extra);

int
mtk_p2p_wext_mlme_handler(IN struct net_device *prDev,
			  IN struct iw_request_info *info,
			  IN OUT union iwreq_data *wrqu, IN OUT char *extra);

int
mtk_p2p_wext_set_powermode(IN struct net_device *prDev,
			   IN struct iw_request_info *info,
			   IN OUT union iwreq_data *wrqu, IN OUT char *extra);

int
mtk_p2p_wext_get_powermode(IN struct net_device *prDev,
			   IN struct iw_request_info *info,
			   IN OUT union iwreq_data *wrqu, IN OUT char *extra);

/*                                                       */
int
mtk_p2p_wext_set_local_dev_info(IN struct net_device *prDev,
				IN struct iw_request_info *info,
				IN OUT union iwreq_data *wrqu, IN OUT char *extra);

int
mtk_p2p_wext_set_provision_complete(IN struct net_device *prDev,
				    IN struct iw_request_info *info,
				    IN OUT union iwreq_data *wrqu, IN OUT char *extra);

int
mtk_p2p_wext_start_stop_discovery(IN struct net_device *prDev,
				  IN struct iw_request_info *info,
				  IN OUT union iwreq_data *wrqu, IN OUT char *extra);

int
mtk_p2p_wext_discovery_results(IN struct net_device *prDev,
			       IN struct iw_request_info *info,
			       IN OUT union iwreq_data *wrqu, IN OUT char *extra);

int
mtk_p2p_wext_wsc_ie(IN struct net_device *prDev,
		    IN struct iw_request_info *info,
		    IN OUT union iwreq_data *wrqu, IN OUT char *extra);

int
mtk_p2p_wext_connect_disconnect(IN struct net_device *prDev,
				IN struct iw_request_info *info,
				IN OUT union iwreq_data *wrqu, IN OUT char *extra);

int
mtk_p2p_wext_password_ready(IN struct net_device *prDev,
			    IN struct iw_request_info *info,
			    IN OUT union iwreq_data *wrqu, IN OUT char *extra);

int
mtk_p2p_wext_request_dev_info(IN struct net_device *prDev,
			      IN struct iw_request_info *info,
			      IN OUT union iwreq_data *wrqu, IN OUT char *extra);

int
mtk_p2p_wext_invitation_indicate(IN struct net_device *prDev,
				 IN struct iw_request_info *info,
				 IN OUT union iwreq_data *wrqu, IN OUT char *extra);

int
mtk_p2p_wext_invitation_status(IN struct net_device *prDev,
			       IN struct iw_request_info *info,
			       IN OUT union iwreq_data *wrqu, IN OUT char *extra);

int
mtk_p2p_wext_set_pm_param(IN struct net_device *prDev,
			  IN struct iw_request_info *info,
			  IN OUT union iwreq_data *wrqu, IN OUT char *extra);

int
mtk_p2p_wext_set_ps_profile(IN struct net_device *prDev,
			    IN struct iw_request_info *info,
			    IN OUT union iwreq_data *wrqu, IN OUT char *extra);

int
mtk_p2p_wext_set_network_address(IN struct net_device *prDev,
				 IN struct iw_request_info *info,
				 IN OUT union iwreq_data *wrqu, IN OUT char *extra);

int
mtk_p2p_wext_set_int(IN struct net_device *prDev,
		     IN struct iw_request_info *info,
		     IN OUT union iwreq_data *wrqu, IN OUT char *extra);

/*                                                                 */
int
mtk_p2p_wext_set_struct(IN struct net_device *prDev,
			IN struct iw_request_info *info,
			IN OUT union iwreq_data *wrqu, IN OUT char *extra);

int
mtk_p2p_wext_get_struct(IN struct net_device *prDev,
			IN struct iw_request_info *info,
			IN OUT union iwreq_data *wrqu, IN OUT char *extra);

/*                                                  */
int
mtk_p2p_wext_get_service_discovery_request(IN struct net_device *prDev,
					   IN struct iw_request_info *info,
					   IN OUT union iwreq_data *wrqu, IN OUT char *extra);

int
mtk_p2p_wext_get_service_discovery_response(IN struct net_device *prDev,
					    IN struct iw_request_info *info,
					    IN OUT union iwreq_data *wrqu, IN OUT char *extra);

int
mtk_p2p_wext_send_service_discovery_request(IN struct net_device *prDev,
					    IN struct iw_request_info *info,
					    IN OUT union iwreq_data *wrqu, IN OUT char *extra);

int
mtk_p2p_wext_send_service_discovery_response(IN struct net_device *prDev,
					     IN struct iw_request_info *info,
					     IN OUT union iwreq_data *wrqu, IN OUT char *extra);

int
mtk_p2p_wext_terminate_service_discovery_phase(IN struct net_device *prDev,
					       IN struct iw_request_info *info,
					       IN OUT union iwreq_data *wrqu, IN OUT char *extra);

#if CFG_SUPPORT_ANTI_PIRACY
int
mtk_p2p_wext_set_sec_check_request(IN struct net_device *prDev,
				   IN struct iw_request_info *info,
				   IN OUT union iwreq_data *wrqu, IN OUT char *extra);

int
mtk_p2p_wext_get_sec_check_response(IN struct net_device *prDev,
				    IN struct iw_request_info *info,
				    IN OUT union iwreq_data *wrqu, IN OUT char *extra);
#endif

int
mtk_p2p_wext_set_noa_param(IN struct net_device *prDev,
			   IN struct iw_request_info *info,
			   IN OUT union iwreq_data *wrqu, IN OUT char *extra);

int
mtk_p2p_wext_set_oppps_param(IN struct net_device *prDev,
			     IN struct iw_request_info *info,
			     IN OUT union iwreq_data *wrqu, IN OUT char *extra);

int
mtk_p2p_wext_set_p2p_version(IN struct net_device *prDev,
			     IN struct iw_request_info *info,
			     IN OUT union iwreq_data *wrqu, IN OUT char *extra);

int
mtk_p2p_wext_get_p2p_version(IN struct net_device *prDev,
			     IN struct iw_request_info *info,
			     IN OUT union iwreq_data *wrqu, IN OUT char *extra);


void mtk_p2p_wext_set_Multicastlist(IN P_GLUE_INFO_T prGlueInfo);

#if CFG_SUPPORT_P2P_RSSI_QUERY
int
mtk_p2p_wext_get_rssi(IN struct net_device *prDev,
		      IN struct iw_request_info *info,
		      IN OUT union iwreq_data *wrqu, IN OUT char *extra);

struct iw_statistics *mtk_p2p_wext_get_wireless_stats(struct net_device *prDev);

#endif

int
mtk_p2p_wext_set_txpow(IN struct net_device *prDev,
		       IN struct iw_request_info *prIwrInfo,
		       IN OUT union iwreq_data *prTxPow, IN char *pcExtra);


/*                                                                              
                                                
                                                                                
*/

#endif				/*                 */
