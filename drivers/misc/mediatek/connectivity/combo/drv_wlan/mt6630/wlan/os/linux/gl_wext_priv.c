/*
                                                                                         
*/

/*                      
                                                    
*/



/*
                         
  
                        
                                                
                           
  
                      
                                                         
                                                             
                                                                  
                             
  
                        
                                                                                         
                            
  
                        
                                                                                         
                                             
  
                      
                                                         
                             
  
                      
                                                             
                                                  
                                                                      
  
                   
                                                                                
    
  
                   
                                                                                
                                                                                   
  
                        
       
                       
  
                        
       
                           
                                                      
  
                   
                                                                                                                                                                   
                                                     
  
                      
       
                                                               
  
                   
                                                                                        
                                                                              
  
                   
                                                                                                     
                                                                        
  
                   
                                                                                                     
                                                                                   
  
                   
                                                                                                 
                                                         
                                                                
  
                         
                                                      
              
  
                           
                                                                                                             
    
  
                         
                                                      
                          
  
                         
                                                                                                                    
                                  
  
                           
                                                                                
    
  
                      
                                                                           
                                                  
  
                      
                                                                                    
    
  
                   
                                                                        
                                                                
  
                        
                                                       
                    
  
                        
                                                       
                                       
  
                      
                                                                                          
                                                                       
  
                      
                                                                                          
                                                          
  
                          
                                                               
                                                        
  
                   
                                                                                        
                                          
  
                   
                                                                                                                                                              
                                                  
  
                   
                                                                                 
                                 
  
                   
                                                                                 
                                     
  
                   
       
                                                                                     
  
                   
       
                                                   
  
                   
  
                                                                               
  
                         
                                                
                                   
  
                   
                                                          
                                                         
  
                           
                                             
                               
  
                           
                                             
                                
                                                              
                                                                                                                                         
                                                              
                           
                                                              
                       
                                                              
                       
                                                              
                                           
                                                              
                                                                                                           
                                                              
                                           
                                     
                                 
                                                              
                                                       
                                                              
                                      
                                                              
                                                     
                                                              
                           
                                                              
                                                                   
                                                              
                      
                                                              
                        
                                                              
                                                                   
                                                              
                               
                                                              
                                       
                                                              
                                            
                                                              
                            
                                                              
                                           
                                                              
                  
                                                              
                                               
                                                              
                                            
                                                             
                                   
                                                             
                                           
                                                             
                                         
                                                             
                    
                                                             
                
                                                             
                                                         
*/

/*                                                                              
                                                     
                                                                                
*/

/*                                                                              
                                                          
                                                                                
*/
#include "precomp.h"
#include "gl_os.h"
#include "gl_wext_priv.h"
#if CFG_SUPPORT_WAPI
#include "gl_sec.h"
#endif
#if CFG_ENABLE_WIFI_DIRECT
#include "gl_p2p_os.h"
#endif



/*                                                                              
                                                
                                                                                
*/
#define NUM_SUPPORTED_OIDS      (sizeof(arWlanOidReqTable) / sizeof(WLAN_REQ_ENTRY))

/*                                                                              
                                                            
                                                                                
*/

static int
priv_get_ndis(IN struct net_device *prNetDev,
	      IN NDIS_TRANSPORT_STRUCT * prNdisReq, OUT PUINT_32 pu4OutputLen);

static int
priv_set_ndis(IN struct net_device *prNetDev,
	      IN NDIS_TRANSPORT_STRUCT * prNdisReq, OUT PUINT_32 pu4OutputLen);

static void
priv_driver_get_chip_config_16(PUINT_8 pucStartAddr,
		UINT_32 u4Length, UINT_32 u4Line, int i4TotalLen, INT_32 i4BytesWritten, char *pcCommand);

static void
priv_driver_get_chip_config_4(PUINT_32 pu4StartAddr,
		UINT_32 u4Length, UINT_32 u4Line, int i4TotalLen, INT_32 i4BytesWritten, char *pcCommand);




#if 0				/*                 */
static int
priv_set_appie(IN struct net_device *prNetDev,
	       IN struct iw_request_info *prIwReqInfo,
	       IN union iwreq_data *prIwReqData, OUT char *pcExtra);

static int
priv_set_filter(IN struct net_device *prNetDev,
		IN struct iw_request_info *prIwReqInfo,
		IN union iwreq_data *prIwReqData, OUT char *pcExtra);
#endif				/*                 */

static BOOLEAN reqSearchSupportedOidEntry(IN UINT_32 rOid, OUT P_WLAN_REQ_ENTRY * ppWlanReqEntry);

#if 0
static WLAN_STATUS
reqExtQueryConfiguration(IN P_GLUE_INFO_T prGlueInfo,
			 OUT PVOID pvQueryBuffer,
			 IN UINT_32 u4QueryBufferLen, OUT PUINT_32 pu4QueryInfoLen);

static WLAN_STATUS
reqExtSetConfiguration(IN P_GLUE_INFO_T prGlueInfo,
		       IN PVOID pvSetBuffer, IN UINT_32 u4SetBufferLen, OUT PUINT_32 pu4SetInfoLen);
#endif

static WLAN_STATUS
reqExtSetAcpiDevicePowerState(IN P_GLUE_INFO_T prGlueInfo,
			      IN PVOID pvSetBuffer,
			      IN UINT_32 u4SetBufferLen, OUT PUINT_32 pu4SetInfoLen);

/*                                                                              
                                               
                                                                                
*/
static UINT_8 aucOidBuf[4096] = { 0 };

/*                      */
/*                                                               
                                          */
static WLAN_REQ_ENTRY arWlanOidReqTable[] = {
	/*
                    
                        
                                                                                      
                      
                    
  */
	/*                                     */

	/*                                      */
	{OID_802_3_CURRENT_ADDRESS,
	 DISP_STRING("OID_802_3_CURRENT_ADDRESS"),
	 TRUE, TRUE, ENUM_OID_DRIVER_CORE, 6,
	 (PFN_OID_HANDLER_FUNC_REQ) wlanoidQueryCurrentAddr,
	 NULL},

	/*                          */
	/*                             */
	/*                     */

	/*                               */
	{OID_802_11_SUPPORTED_RATES,
	 DISP_STRING("OID_802_11_SUPPORTED_RATES"),
	 TRUE, FALSE, ENUM_OID_DRIVER_CORE, sizeof(PARAM_RATES_EX),
	 (PFN_OID_HANDLER_FUNC_REQ) wlanoidQuerySupportedRates,
	 NULL}
	,
	/*
                              
                                            
                                                                       
                                                       
                                                      
  */
	{OID_PNP_SET_POWER,
	 DISP_STRING("OID_PNP_SET_POWER"),
	 TRUE, FALSE, ENUM_OID_GLUE_EXTENSION, sizeof(PARAM_DEVICE_POWER_STATE),
	 NULL,
	 (PFN_OID_HANDLER_FUNC_REQ) reqExtSetAcpiDevicePowerState}
	,

	/*             */
	{OID_CUSTOM_OID_INTERFACE_VERSION,
	 DISP_STRING("OID_CUSTOM_OID_INTERFACE_VERSION"),
	 TRUE, FALSE, ENUM_OID_DRIVER_CORE, 4,
	 (PFN_OID_HANDLER_FUNC_REQ) wlanoidQueryOidInterfaceVersion,
	 NULL}
	,

	/*
                   
                                
                                              
                                                                         
         
                                                       
          
  */

	/*
                                         
                                                       
                                          
                                                           
                                                          
                               
                                             
                                                                                         
         
                                                       
                                      
                                                    
                                          
         
                                                     
                           
                                         
                                        
                                                               
                                                              
                       
                                     
                                        
         
                                            
                       
                                     
                                        
         
                                            
  */

	/*
                                 
                    
                                  
                                          
                                                    
                                                 
          

                           
                                         
                                        
                                                          
                                                         
                            
                                          
                                        
         
                                                       
                               
                                             
                                          
                                                         
                                                        

                           
                                        
                                                      
                                          
                                                         
                                                        
          

                           
                                         
                                                                       
         
                                                   
                                
                                              
                                                                       
                                                                 
                                                               
                                         
                                                       
                                         
                                                                     
                                                                    
  */

	/*     */
	/*
                                      
                                                    
                                         
         
                                                             
  */

	{OID_CUSTOM_MCR_RW,
	 DISP_STRING("OID_CUSTOM_MCR_RW"),
	 TRUE, TRUE, ENUM_OID_DRIVER_CORE, sizeof(PARAM_CUSTOM_MCR_RW_STRUC_T),
	 (PFN_OID_HANDLER_FUNC_REQ) wlanoidQueryMcrRead,
	 (PFN_OID_HANDLER_FUNC_REQ) wlanoidSetMcrWrite}
	,

	{OID_CUSTOM_EEPROM_RW,
	 DISP_STRING("OID_CUSTOM_EEPROM_RW"),
	 TRUE, TRUE, ENUM_OID_DRIVER_CORE, sizeof(PARAM_CUSTOM_EEPROM_RW_STRUC_T),
	 (PFN_OID_HANDLER_FUNC_REQ) wlanoidQueryEepromRead,
	 (PFN_OID_HANDLER_FUNC_REQ) wlanoidSetEepromWrite}
	,

	{OID_CUSTOM_SW_CTRL,
	 DISP_STRING("OID_CUSTOM_SW_CTRL"),
	 TRUE, TRUE, ENUM_OID_DRIVER_CORE, sizeof(PARAM_CUSTOM_SW_CTRL_STRUC_T),
	 (PFN_OID_HANDLER_FUNC_REQ) wlanoidQuerySwCtrlRead,
	 (PFN_OID_HANDLER_FUNC_REQ) wlanoidSetSwCtrlWrite}
	,

	{OID_CUSTOM_MEM_DUMP,
	 DISP_STRING("OID_CUSTOM_MEM_DUMP"),
	 TRUE, TRUE, ENUM_OID_DRIVER_CORE, sizeof(PARAM_CUSTOM_MEM_DUMP_STRUC_T),
	 (PFN_OID_HANDLER_FUNC_REQ) wlanoidQueryMemDump,
	 NULL}
	,

	{OID_CUSTOM_TEST_MODE,
	 DISP_STRING("OID_CUSTOM_TEST_MODE"),
	 FALSE, FALSE, ENUM_OID_DRIVER_CORE, 0,
	 NULL,
	 (PFN_OID_HANDLER_FUNC_REQ) wlanoidRftestSetTestMode}
	,

	/*
                               
                                             
                                                                                     
                                                         
          
                               
                                             
                                                                                     
                                                         
          
  */
	{OID_CUSTOM_ABORT_TEST_MODE,
	 DISP_STRING("OID_CUSTOM_ABORT_TEST_MODE"),
	 FALSE, FALSE, ENUM_OID_DRIVER_CORE, 0,
	 NULL,
	 (PFN_OID_HANDLER_FUNC_REQ) wlanoidRftestSetAbortTestMode}
	,
	{OID_CUSTOM_MTK_WIFI_TEST,
	 DISP_STRING("OID_CUSTOM_MTK_WIFI_TEST"),
	 TRUE, TRUE, ENUM_OID_DRIVER_CORE, sizeof(PARAM_MTK_WIFI_TEST_STRUC_T),
	 (PFN_OID_HANDLER_FUNC_REQ) wlanoidRftestQueryAutoTest,
	 (PFN_OID_HANDLER_FUNC_REQ) wlanoidRftestSetAutoTest}
	,
	{OID_CUSTOM_TEST_ICAP_MODE,
	 DISP_STRING("OID_CUSTOM_TEST_ICAP_MODE"),
	 FALSE, FALSE, ENUM_OID_DRIVER_CORE, 0,
	 NULL,
	 (PFN_OID_HANDLER_FUNC_REQ) wlanoidRftestSetTestIcapMode}
	,

	/*                                      */

	/*      */
#if CFG_SUPPORT_BCM && CFG_SUPPORT_BCM_BWCS
	{OID_CUSTOM_BWCS_CMD,
	 DISP_STRING("OID_CUSTOM_BWCS_CMD"),
	 FALSE, FALSE, ENUM_OID_DRIVER_CORE, sizeof(PTA_IPC_T),
	 (PFN_OID_HANDLER_FUNC_REQ) wlanoidQueryBT,
	 (PFN_OID_HANDLER_FUNC_REQ) wlanoidSetBT}
	,
#endif

/*                               
                                          
                                       
                                                       
                                                      
                        
                                   
                                       
                                           
                                          
    */

	{OID_CUSTOM_MTK_NVRAM_RW,
	 DISP_STRING("OID_CUSTOM_MTK_NVRAM_RW"),
	 TRUE, TRUE, ENUM_OID_DRIVER_CORE, sizeof(PARAM_CUSTOM_NVRAM_RW_STRUCT_T),
	 (PFN_OID_HANDLER_FUNC_REQ) wlanoidQueryNvramRead,
	 (PFN_OID_HANDLER_FUNC_REQ) wlanoidSetNvramWrite}
	,

	{OID_CUSTOM_CFG_SRC_TYPE,
	 DISP_STRING("OID_CUSTOM_CFG_SRC_TYPE"),
	 FALSE, FALSE, ENUM_OID_DRIVER_CORE, sizeof(ENUM_CFG_SRC_TYPE_T),
	 (PFN_OID_HANDLER_FUNC_REQ) wlanoidQueryCfgSrcType,
	 NULL}
	,

	{OID_CUSTOM_EEPROM_TYPE,
	 DISP_STRING("OID_CUSTOM_EEPROM_TYPE"),
	 FALSE, FALSE, ENUM_OID_DRIVER_CORE, sizeof(ENUM_EEPROM_TYPE_T),
	 (PFN_OID_HANDLER_FUNC_REQ) wlanoidQueryEepromType,
	 NULL}
	,

#if CFG_SUPPORT_WAPI
	{OID_802_11_WAPI_MODE,
	 DISP_STRING("OID_802_11_WAPI_MODE"),
	 FALSE, TRUE, ENUM_OID_DRIVER_CORE, 4,
	 NULL,
	 (PFN_OID_HANDLER_FUNC_REQ) wlanoidSetWapiMode}
	,
	{OID_802_11_WAPI_ASSOC_INFO,
	 DISP_STRING("OID_802_11_WAPI_ASSOC_INFO"),
	 FALSE, FALSE, ENUM_OID_DRIVER_CORE, 0,
	 NULL,
	 (PFN_OID_HANDLER_FUNC_REQ) wlanoidSetWapiAssocInfo}
	,
	{OID_802_11_SET_WAPI_KEY,
	 DISP_STRING("OID_802_11_SET_WAPI_KEY"),
	 FALSE, FALSE, ENUM_OID_DRIVER_CORE, sizeof(PARAM_WPI_KEY_T),
	 NULL,
	 (PFN_OID_HANDLER_FUNC_REQ) wlanoidSetWapiKey}
	,
#endif

#if CFG_SUPPORT_WPS2
	{OID_802_11_WSC_ASSOC_INFO,
	 DISP_STRING("OID_802_11_WSC_ASSOC_INFO"),
	 FALSE, FALSE, ENUM_OID_DRIVER_CORE, 0,
	 NULL,
	 (PFN_OID_HANDLER_FUNC_REQ) wlanoidSetWSCAssocInfo}
	,
#endif
};

/*                                                                              
                                                
                                                                                
*/

/*                                                                            */
/* 
                                                                         
                    
 
                                           
                                                
                                                                         
 
                        
                                              
                           
 
*/
/*                                                                            */
int priv_support_ioctl(IN struct net_device *prNetDev, IN OUT struct ifreq *prIfReq, IN int i4Cmd)
{
	/*                                                          */
	struct iwreq *prIwReq = (struct iwreq *)prIfReq;
	struct iw_request_info rIwReqInfo;

	/*                                                           */

	/*                  */
	rIwReqInfo.cmd = (__u16) i4Cmd;
	rIwReqInfo.flags = 0;

	switch (i4Cmd) {
	case IOCTL_SET_INT:
		/*                                                                             */
		return priv_set_int(prNetDev, &rIwReqInfo, &(prIwReq->u), (char *)&(prIwReq->u));

	case IOCTL_GET_INT:
		/*                                                                             */
		return priv_get_int(prNetDev, &rIwReqInfo, &(prIwReq->u), (char *)&(prIwReq->u));

	case IOCTL_SET_STRUCT:
	case IOCTL_SET_STRUCT_FOR_EM:
		return priv_set_struct(prNetDev, &rIwReqInfo, &prIwReq->u, (char *)&(prIwReq->u));

	case IOCTL_GET_STRUCT:
		return priv_get_struct(prNetDev, &rIwReqInfo, &prIwReq->u, (char *)&(prIwReq->u));

	default:
		return -EOPNOTSUPP;

	}			/*               */

}				/*                    */


#if CFG_SUPPORT_BATCH_SCAN

EVENT_BATCH_RESULT_T g_rEventBatchResult[CFG_BATCH_MAX_MSCAN];

UINT_32 batchChannelNum2Freq(UINT_32 u4ChannelNum)
{
	UINT_32 u4ChannelInMHz;

	if (u4ChannelNum >= 1 && u4ChannelNum <= 13) {
		u4ChannelInMHz = 2412 + (u4ChannelNum - 1) * 5;
	} else if (u4ChannelNum == 14) {
		u4ChannelInMHz = 2484;
	} else if (u4ChannelNum == 133) {
		u4ChannelInMHz = 3665;	/*         */
	} else if (u4ChannelNum == 137) {
		u4ChannelInMHz = 3685;	/*         */
	} else if (u4ChannelNum >= 34 && u4ChannelNum <= 165) {
		u4ChannelInMHz = 5000 + u4ChannelNum * 5;
	} else if (u4ChannelNum >= 183 && u4ChannelNum <= 196) {
		u4ChannelInMHz = 4000 + u4ChannelNum * 5;
	} else {
		u4ChannelInMHz = 0;
	}

	return u4ChannelInMHz;
}

#define TMP_TEXT_LEN_S 40
#define TMP_TEXT_LEN_L 60
static UCHAR text1[TMP_TEXT_LEN_S], text2[TMP_TEXT_LEN_L], text3[TMP_TEXT_LEN_L];	/*            */


WLAN_STATUS
batchConvertResult(IN P_EVENT_BATCH_RESULT_T prEventBatchResult,
		   OUT PVOID pvBuffer, IN UINT_32 u4MaxBufferLen, OUT PUINT_32 pu4RetLen)
{
	CHAR *p = pvBuffer;
	CHAR ssid[ELEM_MAX_LEN_SSID + 1];
	INT_32 nsize, nsize1, nsize2, nsize3, scancount;
	INT_32 i, j, nleft;
	UINT_32 freq;

	P_EVENT_BATCH_RESULT_ENTRY_T prEntry;
	P_EVENT_BATCH_RESULT_T pBr;

	nleft = u4MaxBufferLen - 5;	/*                 */

	pBr = prEventBatchResult;
	scancount = 0;
	for (j = 0; j < CFG_BATCH_MAX_MSCAN; j++) {
		scancount += pBr->ucScanCount;
		pBr++;
	}

	nsize1 = kalSnprintf(text1, TMP_TEXT_LEN_S, "scancount=%ld\nnextcount=%ld\n",
			     scancount, scancount);
	if (nsize1 < nleft) {
		p += nsize1 = kalSprintf(p, "%s", text1);
		nleft -= nsize1;
	} else
		goto short_buf;


	pBr = prEventBatchResult;
	for (j = 0; j < CFG_BATCH_MAX_MSCAN; j++) {
		DBGLOG(SCN, TRACE,
		       ("convert mscan = %d, apcount=%d, nleft=%d\n", j, pBr->ucScanCount, nleft));

		if (pBr->ucScanCount == 0) {
			pBr++;
			continue;
		}

		nleft -= 5;	/*                 */

		/*                                            */
		nsize1 = kalSnprintf(text1, TMP_TEXT_LEN_S, "apcount=%d\n", pBr->ucScanCount);
		if (nsize1 < nleft) {
			p += nsize1 = kalSprintf(p, "%s", text1);
			nleft -= nsize1;
		} else
			goto short_buf;

		for (i = 0; i < pBr->ucScanCount; i++) {
			prEntry = &pBr->arBatchResult[i];

			nsize1 = kalSnprintf(text1, TMP_TEXT_LEN_S, "bssid=" MACSTR "\n",
					     prEntry->aucBssid[0],
					     prEntry->aucBssid[1],
					     prEntry->aucBssid[2],
					     prEntry->aucBssid[3],
					     prEntry->aucBssid[4], prEntry->aucBssid[5]);

			kalMemCopy(ssid,
				   prEntry->aucSSID,
				   (prEntry->ucSSIDLen <
				    ELEM_MAX_LEN_SSID ? prEntry->ucSSIDLen : ELEM_MAX_LEN_SSID));
			ssid[(prEntry->ucSSIDLen <
			      (ELEM_MAX_LEN_SSID - 1) ? prEntry->ucSSIDLen : (ELEM_MAX_LEN_SSID -
									      1))] = '\0';
			nsize2 = kalSnprintf(text2, TMP_TEXT_LEN_L, "ssid=%s\n", ssid);

			freq = batchChannelNum2Freq(prEntry->ucFreq);
			nsize3 =
			    kalSnprintf(text3, TMP_TEXT_LEN_L,
					"freq=%lu\nlevel=%d\ndist=%lu\ndistSd=%lu\n====\n", freq,
					prEntry->cRssi, prEntry->u4Dist, prEntry->u4Distsd);

			nsize = nsize1 + nsize2 + nsize3;
			if (nsize < nleft) {

				kalStrnCpy(p, text1, TMP_TEXT_LEN_S);
				p += nsize1;

				kalStrnCpy(p, text2, TMP_TEXT_LEN_L);
				p += nsize2;

				kalStrnCpy(p, text3, TMP_TEXT_LEN_L);
				p += nsize3;

				nleft -= nsize;
			} else {
				DBGLOG(SCN, TRACE, ("Warning: Early break! (%d)\n", i));
				break;	/*                                           */
			}
		}

		nsize1 = kalSnprintf(text1, TMP_TEXT_LEN_S, "%s", "####\n");
		p += kalSprintf(p, "%s", text1);

		pBr++;
	}

	nsize1 = kalSnprintf(text1, TMP_TEXT_LEN_S, "%s", "----\n");
	kalSprintf(p, "%s", text1);

	*pu4RetLen = u4MaxBufferLen - nleft;
	DBGLOG(SCN, TRACE, ("total len = %d (max len = %d)\n", *pu4RetLen, u4MaxBufferLen));

	return WLAN_STATUS_SUCCESS;

short_buf:
	DBGLOG(SCN, TRACE,
	       ("Short buffer issue! %d > %d, %s\n", u4MaxBufferLen + (nsize - nleft),
		u4MaxBufferLen, pvBuffer));
	return WLAN_STATUS_INVALID_LENGTH;
}
#endif

/*                                                                            */
/* 
                                       
 
                                           
                                                    
                                                                                
                                                
 
                        
                                              
                                             
 
*/
/*                                                                            */
int
priv_set_int(IN struct net_device *prNetDev,
	     IN struct iw_request_info *prIwReqInfo,
	     IN union iwreq_data *prIwReqData, IN char *pcExtra)
{
	UINT_32 u4SubCmd;
	PUINT_32 pu4IntBuf;
	P_NDIS_TRANSPORT_STRUCT prNdisReq;
	P_GLUE_INFO_T prGlueInfo;
	UINT_32 u4BufLen = 0;
	int status = 0;
	P_PTA_IPC_T prPtaIpc;

	ASSERT(prNetDev);
	ASSERT(prIwReqInfo);
	ASSERT(prIwReqData);
	ASSERT(pcExtra);

	if (FALSE == GLUE_CHK_PR3(prNetDev, prIwReqData, pcExtra)) {
		return -EINVAL;
	}
	prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prNetDev));

	u4SubCmd = (UINT_32) prIwReqData->mode;
	pu4IntBuf = (PUINT_32) pcExtra;

	switch (u4SubCmd) {
	case PRIV_CMD_TEST_MODE:
		/*                                         */
		prNdisReq = (P_NDIS_TRANSPORT_STRUCT) &aucOidBuf[0];

		if (pu4IntBuf[1] == PRIV_CMD_TEST_MAGIC_KEY) {
			prNdisReq->ndisOidCmd = OID_CUSTOM_TEST_MODE;
		} else if (pu4IntBuf[1] == 0) {
			prNdisReq->ndisOidCmd = OID_CUSTOM_ABORT_TEST_MODE;
		} else if (pu4IntBuf[1] == PRIV_CMD_TEST_MAGIC_KEY_ICAP) {
			prNdisReq->ndisOidCmd = OID_CUSTOM_TEST_ICAP_MODE;
		} else {
			status = 0;
			break;
		}
		prNdisReq->inNdisOidlength = 0;
		prNdisReq->outNdisOidLength = 0;

		/*                  */
		status = priv_set_ndis(prNetDev, prNdisReq, &u4BufLen);
		break;

	case PRIV_CMD_TEST_CMD:
		/*                                                                    */
		prNdisReq = (P_NDIS_TRANSPORT_STRUCT) &aucOidBuf[0];

		kalMemCopy(&prNdisReq->ndisOidContent[0], &pu4IntBuf[1], 8);

		prNdisReq->ndisOidCmd = OID_CUSTOM_MTK_WIFI_TEST;
		prNdisReq->inNdisOidlength = 8;
		prNdisReq->outNdisOidLength = 8;

		/*                  */
		status = priv_set_ndis(prNetDev, prNdisReq, &u4BufLen);
		break;

#if CFG_SUPPORT_PRIV_MCR_RW
	case PRIV_CMD_ACCESS_MCR:
		/*                                                                     */
		prNdisReq = (P_NDIS_TRANSPORT_STRUCT) &aucOidBuf[0];

		if (!prGlueInfo->fgMcrAccessAllowed) {
			if (pu4IntBuf[1] == PRIV_CMD_TEST_MAGIC_KEY &&
			    pu4IntBuf[2] == PRIV_CMD_TEST_MAGIC_KEY) {
				prGlueInfo->fgMcrAccessAllowed = TRUE;
			}
			status = 0;
			break;
		}

		kalMemCopy(&prNdisReq->ndisOidContent[0], &pu4IntBuf[1], 8);

		prNdisReq->ndisOidCmd = OID_CUSTOM_MCR_RW;
		prNdisReq->inNdisOidlength = 8;
		prNdisReq->outNdisOidLength = 8;

		/*                  */
		status = priv_set_ndis(prNetDev, prNdisReq, &u4BufLen);
		break;
#endif

	case PRIV_CMD_SW_CTRL:
		/*                                                                     */
		prNdisReq = (P_NDIS_TRANSPORT_STRUCT) &aucOidBuf[0];

		kalMemCopy(&prNdisReq->ndisOidContent[0], &pu4IntBuf[1], 8);

		prNdisReq->ndisOidCmd = OID_CUSTOM_SW_CTRL;
		prNdisReq->inNdisOidlength = 8;
		prNdisReq->outNdisOidLength = 8;

		/*                  */
		status = priv_set_ndis(prNetDev, prNdisReq, &u4BufLen);
		break;


#if 0
	case PRIV_CMD_BEACON_PERIOD:
		rStatus = wlanSetInformation(prGlueInfo->prAdapter, wlanoidSetBeaconInterval, (PVOID) & pu4IntBuf[1],	/*                                      */
					     sizeof(UINT_32), &u4BufLen);
		break;
#endif

#if CFG_TCP_IP_CHKSUM_OFFLOAD
	case PRIV_CMD_CSUM_OFFLOAD:
		{
			UINT_32 u4CSUMFlags;


			if (pu4IntBuf[1] == 1) {
				u4CSUMFlags = CSUM_OFFLOAD_EN_ALL;
			} else if (pu4IntBuf[1] == 0) {
				u4CSUMFlags = 0;
			} else {
				return -EINVAL;
			}

			if (kalIoctl(prGlueInfo,
				     wlanoidSetCSUMOffload,
				     (PVOID) & u4CSUMFlags,
				     sizeof(UINT_32),
				     FALSE, FALSE, TRUE, &u4BufLen) == WLAN_STATUS_SUCCESS) {
				if (pu4IntBuf[1] == 1) {
					prNetDev->features |= NETIF_F_HW_CSUM;
				} else if (pu4IntBuf[1] == 0) {
					prNetDev->features &= ~NETIF_F_HW_CSUM;
				}
			}
		}
		break;
#endif				/*                           */

	case PRIV_CMD_POWER_MODE:
		{
			PARAM_POWER_MODE_T rPowerMode;
			P_BSS_INFO_T prBssInfo = prGlueInfo->prAdapter->prAisBssInfo;

			if (!prBssInfo) {
				break;
			}

			rPowerMode.ePowerMode = (PARAM_POWER_MODE) pu4IntBuf[1];
			rPowerMode.ucBssIdx = prBssInfo->ucBssIndex;

			kalIoctl(prGlueInfo, wlanoidSet802dot11PowerSaveProfile, &rPowerMode,	/*                                      */
				 sizeof(PARAM_POWER_MODE_T), FALSE, FALSE, TRUE, &u4BufLen);
		}
		break;

	case PRIV_CMD_WMM_PS:
		{
			PARAM_CUSTOM_WMM_PS_TEST_STRUC_T rWmmPsTest;

			rWmmPsTest.bmfgApsdEnAc = (UINT_8) pu4IntBuf[1];
			rWmmPsTest.ucIsEnterPsAtOnce = (UINT_8) pu4IntBuf[2];
			rWmmPsTest.ucIsDisableUcTrigger = (UINT_8) pu4IntBuf[3];
			rWmmPsTest.reserved = 0;

			kalIoctl(prGlueInfo,
				 wlanoidSetWiFiWmmPsTest,
				 (PVOID) & rWmmPsTest,
				 sizeof(PARAM_CUSTOM_WMM_PS_TEST_STRUC_T),
				 FALSE, FALSE, TRUE, &u4BufLen);
		}
		break;

#if 0
	case PRIV_CMD_ADHOC_MODE:
		rStatus = wlanSetInformation(prGlueInfo->prAdapter, wlanoidSetAdHocMode, (PVOID) & pu4IntBuf[1],	/*                                      */
					     sizeof(UINT_32), &u4BufLen);
		break;
#endif

	case PRIV_CUSTOM_BWCS_CMD:

		DBGLOG(REQ, INFO,
		       ("pu4IntBuf[1] = %x, size of PTA_IPC_T = %d.\n", pu4IntBuf[1],
			sizeof(PARAM_PTA_IPC_T)));

		prPtaIpc = (P_PTA_IPC_T) aucOidBuf;
		prPtaIpc->u.aucBTPParams[0] = (UINT_8) (pu4IntBuf[1] >> 24);
		prPtaIpc->u.aucBTPParams[1] = (UINT_8) (pu4IntBuf[1] >> 16);
		prPtaIpc->u.aucBTPParams[2] = (UINT_8) (pu4IntBuf[1] >> 8);
		prPtaIpc->u.aucBTPParams[3] = (UINT_8) (pu4IntBuf[1]);

		DBGLOG(REQ, INFO,
		       ("BCM BWCS CMD : PRIV_CUSTOM_BWCS_CMD : aucBTPParams[0] = %02x, aucBTPParams[1] = %02x, aucBTPParams[2] = %02x, aucBTPParams[3] = %02x.\n",
			prPtaIpc->u.aucBTPParams[0], prPtaIpc->u.aucBTPParams[1],
			prPtaIpc->u.aucBTPParams[2], prPtaIpc->u.aucBTPParams[3]));

#if 0
		status = wlanSetInformation(prGlueInfo->prAdapter,
					    wlanoidSetBT,
					    (PVOID) & aucOidBuf[0], u4CmdLen, &u4BufLen);
#endif

		status = wlanoidSetBT(prGlueInfo->prAdapter,
				      (PVOID) & aucOidBuf[0], sizeof(PARAM_PTA_IPC_T), &u4BufLen);

		if (WLAN_STATUS_SUCCESS != status) {
			status = -EFAULT;
		}

		break;

	case PRIV_CMD_BAND_CONFIG:
		{
			DBGLOG(INIT, INFO, ("CMD set_band = %lu\n", (UINT_32) pu4IntBuf[1]));
		}
		break;

#if CFG_ENABLE_WIFI_DIRECT
	case PRIV_CMD_P2P_MODE:
		{
			PARAM_CUSTOM_P2P_SET_STRUC_T rSetP2P;
			WLAN_STATUS rWlanStatus = WLAN_STATUS_SUCCESS;

			rSetP2P.u4Enable = pu4IntBuf[1];
			rSetP2P.u4Mode = pu4IntBuf[2];
#if 1
			if (!rSetP2P.u4Enable) {
				p2pNetUnregister(prGlueInfo, TRUE);
			}

			rWlanStatus = kalIoctl(prGlueInfo, wlanoidSetP2pMode, (PVOID) & rSetP2P,	/*                                      */
					       sizeof(PARAM_CUSTOM_P2P_SET_STRUC_T),
					       FALSE, FALSE, TRUE, &u4BufLen);

			if ((rSetP2P.u4Enable) && (rWlanStatus == WLAN_STATUS_SUCCESS)) {
				p2pNetRegister(prGlueInfo, TRUE);
			}
#endif

		}
		break;
#endif

#if (CFG_MET_PACKET_TRACE_SUPPORT == 1)
    case PRIV_CMD_MET_PROFILING:
            {
                //                                                 
                //                                                      
                //                                                   
                //                                                                                                                  
                prGlueInfo->fgMetProfilingEn = (UINT_8)pu4IntBuf[1];
                prGlueInfo->u2MetUdpPort = (UINT_16)pu4IntBuf[2];          
                //                                                                                                    

                
            }
            break;
    
#endif

	default:
		return -EOPNOTSUPP;
	}

	return status;
}


/*                                                                            */
/* 
                                       
 
                                       
                                                
                                                                               
                                                          
 
                        
                                              
                           
 
*/
/*                                                                            */
int
priv_get_int(IN struct net_device *prNetDev,
	     IN struct iw_request_info *prIwReqInfo,
	     IN union iwreq_data *prIwReqData, IN OUT char *pcExtra)
{
	UINT_32 u4SubCmd;
	PUINT_32 pu4IntBuf;
	P_GLUE_INFO_T prGlueInfo;
	UINT_32 u4BufLen = 0;
	int status = 0;
	P_NDIS_TRANSPORT_STRUCT prNdisReq;
	INT_32 ch[50];

	ASSERT(prNetDev);
	ASSERT(prIwReqInfo);
	ASSERT(prIwReqData);
	ASSERT(pcExtra);
	if (FALSE == GLUE_CHK_PR3(prNetDev, prIwReqData, pcExtra)) {
		return -EINVAL;
	}
	prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prNetDev));

	u4SubCmd = (UINT_32) prIwReqData->mode;
	pu4IntBuf = (PUINT_32) pcExtra;

	switch (u4SubCmd) {
	case PRIV_CMD_TEST_CMD:
		/*                                                                    */
		prNdisReq = (P_NDIS_TRANSPORT_STRUCT) &aucOidBuf[0];

		kalMemCopy(&prNdisReq->ndisOidContent[0], &pu4IntBuf[1], 8);

		prNdisReq->ndisOidCmd = OID_CUSTOM_MTK_WIFI_TEST;
		prNdisReq->inNdisOidlength = 8;
		prNdisReq->outNdisOidLength = 8;

		status = priv_get_ndis(prNetDev, prNdisReq, &u4BufLen);
		if (status == 0) {
			/*                                                                   */
			prIwReqData->mode = *(PUINT_32) &prNdisReq->ndisOidContent[4];
			/*
                                                 
                                          
                                                                         
                     
       
    */
		}
		return status;

#if CFG_SUPPORT_PRIV_MCR_RW
	case PRIV_CMD_ACCESS_MCR:
		/*                                         */
		prNdisReq = (P_NDIS_TRANSPORT_STRUCT) &aucOidBuf[0];

		if (!prGlueInfo->fgMcrAccessAllowed) {
			status = 0;
			return status;
		}

		kalMemCopy(&prNdisReq->ndisOidContent[0], &pu4IntBuf[1], 8);

		prNdisReq->ndisOidCmd = OID_CUSTOM_MCR_RW;
		prNdisReq->inNdisOidlength = 8;
		prNdisReq->outNdisOidLength = 8;

		status = priv_get_ndis(prNetDev, prNdisReq, &u4BufLen);
		if (status == 0) {
			/*                                                                   */
			prIwReqData->mode = *(PUINT_32) &prNdisReq->ndisOidContent[4];
		}
		return status;
#endif

	case PRIV_CMD_DUMP_MEM:
		prNdisReq = (P_NDIS_TRANSPORT_STRUCT) &aucOidBuf[0];

#if 1
		if (!prGlueInfo->fgMcrAccessAllowed) {
			status = 0;
			return status;
		}
#endif
		kalMemCopy(&prNdisReq->ndisOidContent[0], &pu4IntBuf[1], 8);

		prNdisReq->ndisOidCmd = OID_CUSTOM_MEM_DUMP;
		prNdisReq->inNdisOidlength = sizeof(PARAM_CUSTOM_MEM_DUMP_STRUC_T);
		prNdisReq->outNdisOidLength = sizeof(PARAM_CUSTOM_MEM_DUMP_STRUC_T);

		status = priv_get_ndis(prNetDev, prNdisReq, &u4BufLen);
		if (status == 0) {
			prIwReqData->mode = *(PUINT_32) &prNdisReq->ndisOidContent[0];
		}
		return status;

	case PRIV_CMD_SW_CTRL:
		/*                                          */

		prNdisReq = (P_NDIS_TRANSPORT_STRUCT) &aucOidBuf[0];

		kalMemCopy(&prNdisReq->ndisOidContent[0], &pu4IntBuf[1], 8);

		prNdisReq->ndisOidCmd = OID_CUSTOM_SW_CTRL;
		prNdisReq->inNdisOidlength = 8;
		prNdisReq->outNdisOidLength = 8;

		status = priv_get_ndis(prNetDev, prNdisReq, &u4BufLen);
		if (status == 0) {
			/*                                                                   */
			prIwReqData->mode = *(PUINT_32) &prNdisReq->ndisOidContent[4];
		}
		return status;

#if 0
	case PRIV_CMD_BEACON_PERIOD:
		status = wlanQueryInformation(prGlueInfo->prAdapter,
					      wlanoidQueryBeaconInterval,
					      (PVOID) pu4IntBuf, sizeof(UINT_32), &u4BufLen);
		return status;

	case PRIV_CMD_POWER_MODE:
		status = wlanQueryInformation(prGlueInfo->prAdapter,
					      wlanoidQuery802dot11PowerSaveProfile,
					      (PVOID) pu4IntBuf, sizeof(UINT_32), &u4BufLen);
		return status;

	case PRIV_CMD_ADHOC_MODE:
		status = wlanQueryInformation(prGlueInfo->prAdapter,
					      wlanoidQueryAdHocMode,
					      (PVOID) pu4IntBuf, sizeof(UINT_32), &u4BufLen);
		return status;
#endif

	case PRIV_CMD_BAND_CONFIG:
		DBGLOG(INIT, INFO, ("CMD get_band=\n"));
		prIwReqData->mode = 0;
		return status;

	default:
		break;
	}

	u4SubCmd = (UINT_32) prIwReqData->data.flags;

	switch (u4SubCmd) {
	case PRIV_CMD_GET_CH_LIST:
		{
			UINT_16 i, j = 0;
			UINT_8 NumOfChannel = 50;
			UINT_8 ucMaxChannelNum = 50;
			RF_CHANNEL_INFO_T aucChannelList[50];

			kalGetChannelList(prGlueInfo, BAND_NULL, ucMaxChannelNum, &NumOfChannel,
					  aucChannelList);
			if (NumOfChannel > 50)
				NumOfChannel = 50;

			if (kalIsAPmode(prGlueInfo)) {
				for (i = 0; i < NumOfChannel; i++) {
					if ((aucChannelList[i].ucChannelNum <= 13) ||
					    (aucChannelList[i].ucChannelNum == 36
					     || aucChannelList[i].ucChannelNum == 40
					     || aucChannelList[i].ucChannelNum == 44
					     || aucChannelList[i].ucChannelNum == 48)) {
						ch[j] = (INT_32) aucChannelList[i].ucChannelNum;
						j++;
					}
				}
			} else {
				for (j = 0; j < NumOfChannel; j++) {
					ch[j] = (INT_32) aucChannelList[j].ucChannelNum;
				}
			}

			prIwReqData->data.length = j;
			if (copy_to_user
			    (prIwReqData->data.pointer, ch, NumOfChannel * sizeof(INT_32))) {
				return -EFAULT;
			} else
				return status;
		}
	default:
		return -EOPNOTSUPP;
	}

	return status;
}				/*              */


/*                                                                            */
/* 
                                             
 
                                           
                                                    
                                                                                
                                                
 
                        
                                              
                                             
 
*/
/*                                                                            */
int
priv_set_ints(IN struct net_device *prNetDev,
	      IN struct iw_request_info *prIwReqInfo,
	      IN union iwreq_data *prIwReqData, IN char *pcExtra)
{
	UINT_32 u4SubCmd, u4BufLen;
	P_GLUE_INFO_T prGlueInfo;
	int status = 0;
	WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
	P_SET_TXPWR_CTRL_T prTxpwr;

	ASSERT(prNetDev);
	ASSERT(prIwReqInfo);
	ASSERT(prIwReqData);
	ASSERT(pcExtra);

	if (FALSE == GLUE_CHK_PR3(prNetDev, prIwReqData, pcExtra)) {
		return -EINVAL;
	}
	prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prNetDev));

	u4SubCmd = (UINT_32) prIwReqData->data.flags;

	switch (u4SubCmd) {
	case PRIV_CMD_SET_TX_POWER:
		{
			INT_32 *setting = prIwReqData->data.pointer;
			UINT_16 i;

#if 0
			printk("Tx power num = %d\n", prIwReqData->data.length);

			printk("Tx power setting = %d %d %d %d\n",
			       setting[0], setting[1], setting[2], setting[3]);
#endif
			prTxpwr = &prGlueInfo->rTxPwr;
			if (setting[0] == 0 && prIwReqData->data.length == 4 /*          */) {
				/*                                                                              */
				if (setting[1] == 1 || setting[1] == 0) {
					if (setting[2] == 0 || setting[2] == 1)
						prTxpwr->c2GLegacyStaPwrOffset = setting[3];
					if (setting[2] == 0 || setting[2] == 2)
						prTxpwr->c5GLegacyStaPwrOffset = setting[3];
				}
				if (setting[1] == 2 || setting[1] == 0) {
					if (setting[2] == 0 || setting[2] == 1)
						prTxpwr->c2GHotspotPwrOffset = setting[3];
					if (setting[2] == 0 || setting[2] == 2)
						prTxpwr->c5GHotspotPwrOffset = setting[3];
				}
				if (setting[1] == 3 || setting[1] == 0) {
					if (setting[2] == 0 || setting[2] == 1)
						prTxpwr->c2GP2pPwrOffset = setting[3];
					if (setting[2] == 0 || setting[2] == 2)
						prTxpwr->c5GP2pPwrOffset = setting[3];
				}
				if (setting[1] == 4 || setting[1] == 0) {
					if (setting[2] == 0 || setting[2] == 1)
						prTxpwr->c2GBowPwrOffset = setting[3];
					if (setting[2] == 0 || setting[2] == 2)
						prTxpwr->c5GBowPwrOffset = setting[3];
				}
			} else if (setting[0] == 1 && prIwReqData->data.length == 2) {
				prTxpwr->ucConcurrencePolicy = setting[1];
			} else if (setting[0] == 2 && prIwReqData->data.length == 3) {
				if (setting[1] == 0) {
					for (i = 0; i < 14; i++)
						prTxpwr->acTxPwrLimit2G[i] = setting[2];
				} else if (setting[1] <= 14)
					prTxpwr->acTxPwrLimit2G[setting[1] - 1] = setting[2];
			} else if (setting[0] == 3 && prIwReqData->data.length == 3) {
				if (setting[1] == 0) {
					for (i = 0; i < 4; i++)
						prTxpwr->acTxPwrLimit5G[i] = setting[2];
				} else if (setting[1] <= 4)
					prTxpwr->acTxPwrLimit5G[setting[1] - 1] = setting[2];
			} else if (setting[0] == 4 && prIwReqData->data.length == 2) {
				if (setting[1] == 0) {
					wlanDefTxPowerCfg(prGlueInfo->prAdapter);
				}
				rStatus = kalIoctl(prGlueInfo,
						   wlanoidSetTxPower,
						   prTxpwr,
						   sizeof(SET_TXPWR_CTRL_T),
						   TRUE, FALSE, FALSE, &u4BufLen);
			} else
				return -EFAULT;
		}
		return status;
	default:
		break;
	}

	return status;
}


/*                                                                            */
/* 
                                             
 
                                       
                                                
                                                                               
                                                          
 
                        
                                              
                           
 
*/
/*                                                                            */
int
priv_get_ints(IN struct net_device *prNetDev,
	      IN struct iw_request_info *prIwReqInfo,
	      IN union iwreq_data *prIwReqData, IN OUT char *pcExtra)
{
	UINT_32 u4SubCmd;
	P_GLUE_INFO_T prGlueInfo;
	int status = 0;
	INT_32 ch[50];

	ASSERT(prNetDev);
	ASSERT(prIwReqInfo);
	ASSERT(prIwReqData);
	ASSERT(pcExtra);
	if (FALSE == GLUE_CHK_PR3(prNetDev, prIwReqData, pcExtra)) {
		return -EINVAL;
	}
	prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prNetDev));

	u4SubCmd = (UINT_32) prIwReqData->data.flags;

	switch (u4SubCmd) {
	case PRIV_CMD_GET_CH_LIST:
		{
			UINT_16 i;
			UINT_8 NumOfChannel = 50;
			UINT_8 ucMaxChannelNum = 50;
			RF_CHANNEL_INFO_T aucChannelList[50];

			kalGetChannelList(prGlueInfo, BAND_NULL, ucMaxChannelNum, &NumOfChannel,
					  aucChannelList);
			if (NumOfChannel > 50)
				NumOfChannel = 50;

			for (i = 0; i < NumOfChannel; i++) {
				ch[i] = (INT_32) aucChannelList[i].ucChannelNum;
			}

			prIwReqData->data.length = NumOfChannel;
			if (copy_to_user
			    (prIwReqData->data.pointer, ch, NumOfChannel * sizeof(INT_32))) {
				return -EFAULT;
			} else
				return status;
		}
	default:
		break;
	}

	return status;
}				/*              */

/*                                                                            */
/* 
                                             
 
                                       
                                                         
 
                        
                                              
                                             
 
*/
/*                                                                            */
int
priv_set_struct(IN struct net_device *prNetDev,
		IN struct iw_request_info *prIwReqInfo,
		IN union iwreq_data *prIwReqData, IN char *pcExtra)
{
	UINT_32 u4SubCmd = 0;
	int status = 0;
	/*                                            */
	UINT_32 u4CmdLen = 0;
	P_NDIS_TRANSPORT_STRUCT prNdisReq;
	PUINT_32 pu4IntBuf = NULL;

	P_GLUE_INFO_T prGlueInfo = NULL;
	UINT_32 u4BufLen = 0;

	ASSERT(prNetDev);
	/*                      */
	ASSERT(prIwReqData);
	/*                  */

	kalMemZero(&aucOidBuf[0], sizeof(aucOidBuf));

	if (FALSE == GLUE_CHK_PR2(prNetDev, prIwReqData)) {
		return -EINVAL;
	}
	prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prNetDev));

	u4SubCmd = (UINT_32) prIwReqData->data.flags;

#if 0
	printk(KERN_INFO DRV_NAME "priv_set_struct(): prIwReqInfo->cmd(0x%X), u4SubCmd(%ld)\n",
	       prIwReqInfo->cmd, u4SubCmd);
#endif

	switch (u4SubCmd) {
#if 0				/*             */
	case PRIV_CMD_BT_COEXIST:
		u4CmdLen = prIwReqData->data.length * sizeof(UINT_32);
		ASSERT(sizeof(PARAM_CUSTOM_BT_COEXIST_T) >= u4CmdLen);
		if (sizeof(PARAM_CUSTOM_BT_COEXIST_T) < u4CmdLen) {
			return -EFAULT;
		}

		if (copy_from_user(&aucOidBuf[0], prIwReqData->data.pointer, u4CmdLen)) {
			status = -EFAULT;	/*                 */
			break;
		}

		rStatus = wlanSetInformation(prGlueInfo->prAdapter,
					     wlanoidSetBtCoexistCtrl,
					     (PVOID) & aucOidBuf[0], u4CmdLen, &u4BufLen);
		if (WLAN_STATUS_SUCCESS != rStatus) {
			status = -EFAULT;
		}
		break;
#endif

	case PRIV_CUSTOM_BWCS_CMD:
		u4CmdLen = prIwReqData->data.length * sizeof(UINT_32);
		ASSERT(sizeof(PARAM_PTA_IPC_T) >= u4CmdLen);
		if (sizeof(PARAM_PTA_IPC_T) < u4CmdLen) {
			return -EFAULT;
		}
#if CFG_SUPPORT_BCM && CFG_SUPPORT_BCM_BWCS && CFG_SUPPORT_BCM_BWCS_DEBUG
		DBGLOG(REQ, INFO,
		       ("ucCmdLen = %d, size of PTA_IPC_T = %d, prIwReqData->data = 0x%x.\n",
			u4CmdLen, sizeof(PARAM_PTA_IPC_T), prIwReqData->data));

		DBGLOG(REQ, INFO, ("priv_set_struct(): prIwReqInfo->cmd(0x%X), u4SubCmd(%ld)\n",
				   prIwReqInfo->cmd, u4SubCmd));

		DBGLOG(REQ, INFO, ("*pcExtra = 0x%x\n", *pcExtra));
#endif

		if (copy_from_user(&aucOidBuf[0], prIwReqData->data.pointer, u4CmdLen)) {
			status = -EFAULT;	/*                 */
			break;
		}
#if CFG_SUPPORT_BCM && CFG_SUPPORT_BCM_BWCS && CFG_SUPPORT_BCM_BWCS_DEBUG
		DBGLOG(REQ, INFO, ("priv_set_struct(): BWCS CMD = %02x%02x%02x%02x\n",
				   aucOidBuf[2], aucOidBuf[3], aucOidBuf[4], aucOidBuf[5]));
#endif

#if 0
		status = wlanSetInformation(prGlueInfo->prAdapter,
					    wlanoidSetBT,
					    (PVOID) & aucOidBuf[0], u4CmdLen, &u4BufLen);
#endif

#if 1
		status = wlanoidSetBT(prGlueInfo->prAdapter,
				      (PVOID) & aucOidBuf[0], u4CmdLen, &u4BufLen);
#endif

		if (WLAN_STATUS_SUCCESS != status) {
			status = -EFAULT;
		}

		break;

#if CFG_SUPPORT_WPS2
	case PRIV_CMD_WSC_PROBE_REQ:
		{
			/*                               */
			if (prIwReqData->data.length > 0) {
				if (copy_from_user(prGlueInfo->aucWSCIE, prIwReqData->data.pointer,
						   prIwReqData->data.length)) {
					status = -EFAULT;
					break;
				}
				prGlueInfo->u2WSCIELen = prIwReqData->data.length;
			} else {
				prGlueInfo->u2WSCIELen = 0;
			}
		}
		break;
#endif
	case PRIV_CMD_OID:
		if (copy_from_user(&aucOidBuf[0],
				   prIwReqData->data.pointer, prIwReqData->data.length)) {
			status = -EFAULT;
			break;
		}
		if (!kalMemCmp(&aucOidBuf[0], pcExtra, prIwReqData->data.length)) {
			DBGLOG(REQ, INFO, ("pcExtra buffer is valid\n"));
		} else
			DBGLOG(REQ, INFO, ("pcExtra 0x%p\n", pcExtra));

		/*                  */
		status =
		    priv_set_ndis(prNetDev, (P_NDIS_TRANSPORT_STRUCT) &aucOidBuf[0], &u4BufLen);
		/*                           */
		((P_NDIS_TRANSPORT_STRUCT) &aucOidBuf[0])->outNdisOidLength = u4BufLen;

		if (copy_to_user(prIwReqData->data.pointer,
				 &aucOidBuf[0], OFFSET_OF(NDIS_TRANSPORT_STRUCT, ndisOidContent))) {
			DBGLOG(REQ, INFO, ("copy_to_user oidBuf fail\n"));
			status = -EFAULT;
		}

		break;

	case PRIV_CMD_SW_CTRL:
		pu4IntBuf = (PUINT_32) prIwReqData->data.pointer;
		prNdisReq = (P_NDIS_TRANSPORT_STRUCT) &aucOidBuf[0];

		/*                                                                          */
		if (copy_from_user(&prNdisReq->ndisOidContent[0],
				   prIwReqData->data.pointer, prIwReqData->data.length)) {
			status = -EFAULT;
			break;
		}
		prNdisReq->ndisOidCmd = OID_CUSTOM_SW_CTRL;
		prNdisReq->inNdisOidlength = 8;
		prNdisReq->outNdisOidLength = 8;

		/*                  */
		status = priv_set_ndis(prNetDev, prNdisReq, &u4BufLen);
		break;

	default:
		return -EOPNOTSUPP;
	}

	return status;
}

/*                                                                            */
/* 
                                          
 
                                       
                                                
                                     
 
                        
                                                      
                                                     
 
*/
/*                                                                            */
int
priv_get_struct(IN struct net_device *prNetDev,
		IN struct iw_request_info *prIwReqInfo,
		IN union iwreq_data *prIwReqData, IN OUT char *pcExtra)
{
	UINT_32 u4SubCmd = 0;
	P_NDIS_TRANSPORT_STRUCT prNdisReq = NULL;

	P_GLUE_INFO_T prGlueInfo = NULL;
	UINT_32 u4BufLen = 0;
	PUINT_32 pu4IntBuf = NULL;
	int status = 0;

	kalMemZero(&aucOidBuf[0], sizeof(aucOidBuf));

	ASSERT(prNetDev);
	ASSERT(prIwReqData);
	if (!prNetDev || !prIwReqData) {
		DBGLOG(REQ, INFO, ("priv_get_struct(): invalid param(0x%p, 0x%p)\n",
				   prNetDev, prIwReqData));
		return -EINVAL;
	}

	u4SubCmd = (UINT_32) prIwReqData->data.flags;
	prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prNetDev));
	ASSERT(prGlueInfo);
	if (!prGlueInfo) {
		DBGLOG(REQ, INFO, ("priv_get_struct(): invalid prGlueInfo(0x%p, 0x%p)\n",
				   prNetDev, *((P_GLUE_INFO_T *) netdev_priv(prNetDev))));
		return -EINVAL;
	}
#if 0
	printk(KERN_INFO DRV_NAME "priv_get_struct(): prIwReqInfo->cmd(0x%X), u4SubCmd(%ld)\n",
	       prIwReqInfo->cmd, u4SubCmd);
#endif
	memset(aucOidBuf, 0, sizeof(aucOidBuf));

	switch (u4SubCmd) {
	case PRIV_CMD_OID:
		if (copy_from_user(&aucOidBuf[0],
				   prIwReqData->data.pointer, sizeof(NDIS_TRANSPORT_STRUCT))) {
			DBGLOG(REQ, INFO, ("priv_get_struct() copy_from_user oidBuf fail\n"));
			return -EFAULT;
		}

		prNdisReq = (P_NDIS_TRANSPORT_STRUCT) &aucOidBuf[0];
#if 0
		printk(KERN_NOTICE "\n priv_get_struct cmd 0x%02x len:%d OID:0x%08x OID Len:%d\n",
		       cmd, pIwReq->u.data.length, ndisReq->ndisOidCmd, ndisReq->inNdisOidlength);
#endif
		if (priv_get_ndis(prNetDev, prNdisReq, &u4BufLen) == 0) {
			prNdisReq->outNdisOidLength = u4BufLen;
			if (copy_to_user(prIwReqData->data.pointer,
					 &aucOidBuf[0],
					 u4BufLen + sizeof(NDIS_TRANSPORT_STRUCT) -
					 sizeof(prNdisReq->ndisOidContent))) {
				DBGLOG(REQ, INFO,
				       ("priv_get_struct() copy_to_user oidBuf fail(1)\n"));
				return -EFAULT;
			}
			return 0;
		} else {
			prNdisReq->outNdisOidLength = u4BufLen;
			if (copy_to_user(prIwReqData->data.pointer,
					 &aucOidBuf[0],
					 OFFSET_OF(NDIS_TRANSPORT_STRUCT, ndisOidContent))) {
				DBGLOG(REQ, INFO,
				       ("priv_get_struct() copy_to_user oidBuf fail(2)\n"));
			}
			return -EFAULT;
		}
		break;

	case PRIV_CMD_SW_CTRL:
		pu4IntBuf = (PUINT_32) prIwReqData->data.pointer;
		prNdisReq = (P_NDIS_TRANSPORT_STRUCT) &aucOidBuf[0];

		if (copy_from_user(&prNdisReq->ndisOidContent[0],
				   prIwReqData->data.pointer, prIwReqData->data.length)) {
			DBGLOG(REQ, INFO, ("priv_get_struct() copy_from_user oidBuf fail\n"));
			return -EFAULT;
		}

		prNdisReq->ndisOidCmd = OID_CUSTOM_SW_CTRL;
		prNdisReq->inNdisOidlength = 8;
		prNdisReq->outNdisOidLength = 8;

		status = priv_get_ndis(prNetDev, prNdisReq, &u4BufLen);
		if (status == 0) {
			prNdisReq->outNdisOidLength = u4BufLen;
			/*                                                                                      */

			if (copy_to_user(prIwReqData->data.pointer,
					 &prNdisReq->ndisOidContent[4],
					 4 /*                                                  */)){
				DBGLOG(REQ, INFO,
				       ("priv_get_struct() copy_to_user oidBuf fail(2)\n"));
			}
		}
		return 0;
		break;
	default:
		DBGLOG(REQ, WARN, ("get struct cmd:0x%lx\n", u4SubCmd));
		return -EOPNOTSUPP;
	}
}				/*                 */

/*                                                                            */
/* 
                                                              
 
                                       
                                                                 
                                                                          
                                                                     
                                                                        
                                                                        
 
                       
                                              
 
*/
/*                                                                            */
static int
priv_set_ndis(IN struct net_device *prNetDev,
	      IN NDIS_TRANSPORT_STRUCT * prNdisReq, OUT PUINT_32 pu4OutputLen)
{
	P_WLAN_REQ_ENTRY prWlanReqEntry = NULL;
	WLAN_STATUS status = WLAN_STATUS_SUCCESS;
	P_GLUE_INFO_T prGlueInfo = NULL;
	UINT_32 u4SetInfoLen = 0;

	ASSERT(prNetDev);
	ASSERT(prNdisReq);
	ASSERT(pu4OutputLen);

	if (!prNetDev || !prNdisReq || !pu4OutputLen) {
		DBGLOG(REQ, INFO, ("priv_set_ndis(): invalid param(0x%p, 0x%p, 0x%p)\n",
				   prNetDev, prNdisReq, pu4OutputLen));
		return -EINVAL;
	}

	prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prNetDev));
	ASSERT(prGlueInfo);
	if (!prGlueInfo) {
		DBGLOG(REQ, INFO, ("priv_set_ndis(): invalid prGlueInfo(0x%p, 0x%p)\n",
				   prNetDev, *((P_GLUE_INFO_T *) netdev_priv(prNetDev))));
		return -EINVAL;
	}
#if 0
	printk(KERN_INFO DRV_NAME "priv_set_ndis(): prNdisReq->ndisOidCmd(0x%lX)\n",
	       prNdisReq->ndisOidCmd);
#endif

	if (FALSE == reqSearchSupportedOidEntry(prNdisReq->ndisOidCmd, &prWlanReqEntry)) {
		/*                                                                   */
		return -EOPNOTSUPP;
	}

	if (NULL == prWlanReqEntry->pfOidSetHandler) {
		/*                                                                      */
		return -EOPNOTSUPP;
	}
#if 0
	printk(KERN_INFO DRV_NAME "priv_set_ndis(): %s\n", prWlanReqEntry->pucOidName);
#endif

	if (prWlanReqEntry->fgSetBufLenChecking) {
		if (prNdisReq->inNdisOidlength != prWlanReqEntry->u4InfoBufLen) {
			DBGLOG(REQ, WARN, ("Set %s: Invalid length (current=%ld, needed=%ld)\n",
					   prWlanReqEntry->pucOidName,
					   prNdisReq->inNdisOidlength,
					   prWlanReqEntry->u4InfoBufLen));

			*pu4OutputLen = prWlanReqEntry->u4InfoBufLen;
			return -EINVAL;
		}
	}

	if (prWlanReqEntry->eOidMethod == ENUM_OID_GLUE_ONLY) {
		/*                   */
		status = prWlanReqEntry->pfOidSetHandler(prGlueInfo,
							 prNdisReq->ndisOidContent,
							 prNdisReq->inNdisOidlength, &u4SetInfoLen);
	} else if (prWlanReqEntry->eOidMethod == ENUM_OID_GLUE_EXTENSION) {
		/*                        */
		status = prWlanReqEntry->pfOidSetHandler(prGlueInfo,
							 prNdisReq->ndisOidContent,
							 prNdisReq->inNdisOidlength, &u4SetInfoLen);
	} else if (prWlanReqEntry->eOidMethod == ENUM_OID_DRIVER_CORE) {
		/*             */

		status = kalIoctl(prGlueInfo,
				  (PFN_OID_HANDLER_FUNC) prWlanReqEntry->pfOidSetHandler,
				  prNdisReq->ndisOidContent,
				  prNdisReq->inNdisOidlength, FALSE, FALSE, TRUE, &u4SetInfoLen);
	} else {
		DBGLOG(REQ, INFO, ("priv_set_ndis(): unsupported OID method:0x%x\n",
				   prWlanReqEntry->eOidMethod));
		return -EOPNOTSUPP;
	}

	*pu4OutputLen = u4SetInfoLen;

	switch (status) {
	case WLAN_STATUS_SUCCESS:
		break;

	case WLAN_STATUS_INVALID_LENGTH:
		/*                                                                */
		/*                             */
		/*                             */
		/*                 */
		break;
	}

	if (WLAN_STATUS_SUCCESS != status) {
		return -EFAULT;
	}

	return 0;
}				/*               */

/*                                                                            */
/* 
                                                                             
                                                                      
 
                                       
                                                                 
                                                                          
                                                                    
                                                                       
                                                                       
 
                       
                                              
                                          
 
*/
/*                                                                            */
static int
priv_get_ndis(IN struct net_device *prNetDev,
	      IN NDIS_TRANSPORT_STRUCT * prNdisReq, OUT PUINT_32 pu4OutputLen)
{
	P_WLAN_REQ_ENTRY prWlanReqEntry = NULL;
	UINT_32 u4BufLen = 0;
	WLAN_STATUS status = WLAN_STATUS_SUCCESS;
	P_GLUE_INFO_T prGlueInfo = NULL;

	ASSERT(prNetDev);
	ASSERT(prNdisReq);
	ASSERT(pu4OutputLen);

	if (!prNetDev || !prNdisReq || !pu4OutputLen) {
		DBGLOG(REQ, INFO, ("priv_get_ndis(): invalid param(0x%p, 0x%p, 0x%p)\n",
				   prNetDev, prNdisReq, pu4OutputLen));
		return -EINVAL;
	}

	prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prNetDev));
	ASSERT(prGlueInfo);
	if (!prGlueInfo) {
		DBGLOG(REQ, INFO, ("priv_get_ndis(): invalid prGlueInfo(0x%p, 0x%p)\n",
				   prNetDev, *((P_GLUE_INFO_T *) netdev_priv(prNetDev))));
		return -EINVAL;
	}
#if 0
	printk(KERN_INFO DRV_NAME "priv_get_ndis(): prNdisReq->ndisOidCmd(0x%lX)\n",
	       prNdisReq->ndisOidCmd);
#endif

	if (FALSE == reqSearchSupportedOidEntry(prNdisReq->ndisOidCmd, &prWlanReqEntry)) {
		/*                                                                     */
		return -EOPNOTSUPP;
	}


	if (NULL == prWlanReqEntry->pfOidQueryHandler) {
		/*                                                                          */
		return -EOPNOTSUPP;
	}
#if 0
	printk(KERN_INFO DRV_NAME "priv_get_ndis(): %s\n", prWlanReqEntry->pucOidName);
#endif

	if (prWlanReqEntry->fgQryBufLenChecking) {
		if (prNdisReq->inNdisOidlength < prWlanReqEntry->u4InfoBufLen) {
			/*                                            */
			/*                                                                    */
			/*                             */
			/*                             */
			/*                                 */

			*pu4OutputLen = prWlanReqEntry->u4InfoBufLen;

			status = WLAN_STATUS_INVALID_LENGTH;
			return -EINVAL;
		}
	}


	if (prWlanReqEntry->eOidMethod == ENUM_OID_GLUE_ONLY) {
		/*                   */
		status = prWlanReqEntry->pfOidQueryHandler(prGlueInfo,
							   prNdisReq->ndisOidContent,
							   prNdisReq->inNdisOidlength, &u4BufLen);
	} else if (prWlanReqEntry->eOidMethod == ENUM_OID_GLUE_EXTENSION) {
		/*                        */
		status = prWlanReqEntry->pfOidQueryHandler(prGlueInfo,
							   prNdisReq->ndisOidContent,
							   prNdisReq->inNdisOidlength, &u4BufLen);
	} else if (prWlanReqEntry->eOidMethod == ENUM_OID_DRIVER_CORE) {
		/*             */

		status = kalIoctl(prGlueInfo,
				  (PFN_OID_HANDLER_FUNC) prWlanReqEntry->pfOidQueryHandler,
				  prNdisReq->ndisOidContent,
				  prNdisReq->inNdisOidlength, TRUE, TRUE, TRUE, &u4BufLen);
	} else {
		DBGLOG(REQ, INFO, ("priv_set_ndis(): unsupported OID method:0x%x\n",
				   prWlanReqEntry->eOidMethod));
		return -EOPNOTSUPP;
	}

	*pu4OutputLen = u4BufLen;

	switch (status) {
	case WLAN_STATUS_SUCCESS:
		break;

	case WLAN_STATUS_INVALID_LENGTH:
		/*                                                                */
		/*                             */
		/*                             */
		/*             */
		break;
	}

	if (WLAN_STATUS_SUCCESS != status) {
		return -EOPNOTSUPP;
	}

	return 0;
}				/*               */

/*                                                                            */
/* 
                                                      
 
                                                
                                                          
 
                                    
                                        
*/
/*                                                                            */
static BOOLEAN reqSearchSupportedOidEntry(IN UINT_32 rOid, OUT P_WLAN_REQ_ENTRY *ppWlanReqEntry)
{
	INT_32 i, j, k;

	i = 0;
	j = NUM_SUPPORTED_OIDS - 1;

	while (i <= j) {
		k = (i + j) / 2;

		if (rOid == arWlanOidReqTable[k].rOid) {
			*ppWlanReqEntry = &arWlanOidReqTable[k];
			return TRUE;
		} else if (rOid < arWlanOidReqTable[k].rOid) {
			j = k - 1;
		} else {
			i = k + 1;
		}
	}

	return FALSE;
}				/*                            */



/*                                                                            */
/* 
                                      
 
                                       
                                                
                                     
 
                        
                                                      
                                                     
 
*/
/*                                                                            */
int
priv_set_driver(IN struct net_device *prNetDev,
		IN struct iw_request_info *prIwReqInfo,
		IN union iwreq_data *prIwReqData, IN OUT char *pcExtra)
{
	UINT_32 u4SubCmd = 0;
	UINT_16 u2Cmd = 0;

	P_GLUE_INFO_T prGlueInfo = NULL;
	INT_32 i4BytesWritten = 0;

	ASSERT(prNetDev);
	ASSERT(prIwReqData);
	if (!prNetDev || !prIwReqData) {
		DBGLOG(REQ, INFO, ("priv_set_driver(): invalid param(0x%p, 0x%p)\n",
				   prNetDev, prIwReqData));
		return -EINVAL;
	}

	u2Cmd = prIwReqInfo->cmd;
	DBGLOG(REQ, INFO, ("prIwReqInfo->cmd %u\n", u2Cmd));

	u4SubCmd = (UINT_32) prIwReqData->data.flags;
	prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prNetDev));
	ASSERT(prGlueInfo);
	if (!prGlueInfo) {
		DBGLOG(REQ, INFO, ("priv_set_driver(): invalid prGlueInfo(0x%p, 0x%p)\n",
				   prNetDev, *((P_GLUE_INFO_T *) netdev_priv(prNetDev))));
		return -EINVAL;
	}

	/*                                                                 */
	/*                                                                                */

	DBGLOG(REQ, INFO, ("prIwReqData->data.length %u\n", prIwReqData->data.length));

	/*                                             */

	ASSERT(IW_IS_GET(u2Cmd));
	if (prIwReqData->data.length != 0) {
		if (!access_ok(VERIFY_READ, prIwReqData->data.pointer, prIwReqData->data.length)) {
			DBGLOG(REQ, INFO,
			       ("%s access_ok Read fail written = %d\n", __func__,
				i4BytesWritten));
			return -EFAULT;
		}
		if (copy_from_user(pcExtra, prIwReqData->data.pointer, prIwReqData->data.length)) {
			DBGLOG(REQ, INFO,
			       ("%s copy_form_user fail written = %d\n", __func__,
				prIwReqData->data.length));
			return -EFAULT;
		}
	}

	if (pcExtra) {
		DBGLOG(REQ, INFO, ("pcExtra %s\n", pcExtra));
		/*                                         */
		DBGLOG(REQ, INFO,
		       ("%s prIwReqData->data.length = %d\n", __func__,
			prIwReqData->data.length));
		i4BytesWritten =
		    priv_driver_cmds(prNetDev, pcExtra, 2000 /*                         */);
		DBGLOG(REQ, INFO, ("%s i4BytesWritten = %d\n", __func__, i4BytesWritten));
	}

	DBGLOG(REQ, INFO, ("pcExtra done\n"));

	if (i4BytesWritten > 0) {

		if (i4BytesWritten > 2000) {
			i4BytesWritten = 2000;
		}
		prIwReqData->data.length = i4BytesWritten;	/*                                */

	} else if (i4BytesWritten == 0) {
		prIwReqData->data.length = i4BytesWritten;
	}

#if 0
	/*                                                                 */
	/*                                                                                  */
	ASSERT(IW_IS_SET(u2Cmd));
	if (!access_ok(VERIFY_WRITE, prIwReqData->data.pointer, i4BytesWritten)) {
		DBGLOG(REQ, INFO,
		       ("%s access_ok Write fail written = %d\n", __func__, i4BytesWritten));
		return -EFAULT;
	}
	if (copy_to_user(prIwReqData->data.pointer, pcExtra, i4BytesWritten)) {
		DBGLOG(REQ, INFO,
		       ("%s copy_to_user fail written = %d\n", __func__, i4BytesWritten));
		return -EFAULT;
	}
#endif

	return 0;

}				/*                 */


#if 0
/*                                                                            */
/* 
                                                                             
                               
 
                                                                     
                                                                                         
                                                               
                                                                                
                                                                                
                                                                                 
                                                                     
 
                             
                                    
*/
/*                                                                            */
static WLAN_STATUS
reqExtQueryConfiguration(IN P_GLUE_INFO_T prGlueInfo,
			 OUT PVOID pvQueryBuffer,
			 IN UINT_32 u4QueryBufferLen, OUT PUINT_32 pu4QueryInfoLen)
{
	P_PARAM_802_11_CONFIG_T prQueryConfig = (P_PARAM_802_11_CONFIG_T) pvQueryBuffer;
	WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
	UINT_32 u4QueryInfoLen = 0;

	DEBUGFUNC("wlanoidQueryConfiguration");


	ASSERT(prGlueInfo);
	ASSERT(pu4QueryInfoLen);

	*pu4QueryInfoLen = sizeof(PARAM_802_11_CONFIG_T);
	if (u4QueryBufferLen < sizeof(PARAM_802_11_CONFIG_T)) {
		return WLAN_STATUS_INVALID_LENGTH;
	}

	ASSERT(pvQueryBuffer);

	kalMemZero(prQueryConfig, sizeof(PARAM_802_11_CONFIG_T));

	/*                                         */
	prQueryConfig->u4Length = sizeof(PARAM_802_11_CONFIG_T);

#if defined(_HIF_SDIO)
	rStatus = sdio_io_ctrl(prGlueInfo,
			       wlanoidSetBeaconInterval,
			       &prQueryConfig->u4BeaconPeriod,
			       sizeof(UINT_32), TRUE, TRUE, &u4QueryInfoLen);
#else
	rStatus = wlanQueryInformation(prGlueInfo->prAdapter,
				       wlanoidQueryBeaconInterval,
				       &prQueryConfig->u4BeaconPeriod,
				       sizeof(UINT_32), &u4QueryInfoLen);
#endif
	if (rStatus != WLAN_STATUS_SUCCESS) {
		return rStatus;
	}
#if defined(_HIF_SDIO)
	rStatus = sdio_io_ctrl(prGlueInfo,
			       wlanoidQueryAtimWindow,
			       &prQueryConfig->u4ATIMWindow,
			       sizeof(UINT_32), TRUE, TRUE, &u4QueryInfoLen);
#else
	rStatus = wlanQueryInformation(prGlueInfo->prAdapter,
				       wlanoidQueryAtimWindow,
				       &prQueryConfig->u4ATIMWindow,
				       sizeof(UINT_32), &u4QueryInfoLen);
#endif
	if (rStatus != WLAN_STATUS_SUCCESS) {
		return rStatus;
	}
#if defined(_HIF_SDIO)
	rStatus = sdio_io_ctrl(prGlueInfo,
			       wlanoidQueryFrequency,
			       &prQueryConfig->u4DSConfig,
			       sizeof(UINT_32), TRUE, TRUE, &u4QueryInfoLen);
#else
	rStatus = wlanQueryInformation(prGlueInfo->prAdapter,
				       wlanoidQueryFrequency,
				       &prQueryConfig->u4DSConfig,
				       sizeof(UINT_32), &u4QueryInfoLen);
#endif
	if (rStatus != WLAN_STATUS_SUCCESS) {
		return rStatus;
	}

	prQueryConfig->rFHConfig.u4Length = sizeof(PARAM_802_11_CONFIG_FH_T);

	return rStatus;

}				/*                                   */


/*                                                                            */
/* 
                                                                           
              
 
                                                                 
                                                                                  
                                                         
                                                                            
                                                                              
                                                                            
                                                         
 
                             
                                    
                                  
*/
/*                                                                            */
static WLAN_STATUS
reqExtSetConfiguration(IN P_GLUE_INFO_T prGlueInfo,
		       IN PVOID pvSetBuffer, IN UINT_32 u4SetBufferLen, OUT PUINT_32 pu4SetInfoLen)
{
	WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
	P_PARAM_802_11_CONFIG_T prNewConfig = (P_PARAM_802_11_CONFIG_T) pvSetBuffer;
	UINT_32 u4SetInfoLen = 0;

	DEBUGFUNC("wlanoidSetConfiguration");


	ASSERT(prGlueInfo);
	ASSERT(pu4SetInfoLen);

	*pu4SetInfoLen = sizeof(PARAM_802_11_CONFIG_T);

	if (u4SetBufferLen < *pu4SetInfoLen) {
		return WLAN_STATUS_INVALID_LENGTH;
	}

	/*                                                                          */
	if (prGlueInfo->eParamMediaStateIndicated == PARAM_MEDIA_STATE_CONNECTED) {
		return WLAN_STATUS_NOT_ACCEPTED;
	}

	ASSERT(pvSetBuffer);

#if defined(_HIF_SDIO)
	rStatus = sdio_io_ctrl(prGlueInfo,
			       wlanoidSetBeaconInterval,
			       &prNewConfig->u4BeaconPeriod,
			       sizeof(UINT_32), FALSE, TRUE, &u4SetInfoLen);
#else
	rStatus = wlanSetInformation(prGlueInfo->prAdapter,
				     wlanoidSetBeaconInterval,
				     &prNewConfig->u4BeaconPeriod, sizeof(UINT_32), &u4SetInfoLen);
#endif
	if (rStatus != WLAN_STATUS_SUCCESS) {
		return rStatus;
	}
#if defined(_HIF_SDIO)
	rStatus = sdio_io_ctrl(prGlueInfo,
			       wlanoidSetAtimWindow,
			       &prNewConfig->u4ATIMWindow,
			       sizeof(UINT_32), FALSE, TRUE, &u4SetInfoLen);
#else
	rStatus = wlanSetInformation(prGlueInfo->prAdapter,
				     wlanoidSetAtimWindow,
				     &prNewConfig->u4ATIMWindow, sizeof(UINT_32), &u4SetInfoLen);
#endif
	if (rStatus != WLAN_STATUS_SUCCESS) {
		return rStatus;
	}
#if defined(_HIF_SDIO)
	rStatus = sdio_io_ctrl(prGlueInfo,
			       wlanoidSetFrequency,
			       &prNewConfig->u4DSConfig,
			       sizeof(UINT_32), FALSE, TRUE, &u4SetInfoLen);
#else
	rStatus = wlanSetInformation(prGlueInfo->prAdapter,
				     wlanoidSetFrequency,
				     &prNewConfig->u4DSConfig, sizeof(UINT_32), &u4SetInfoLen);
#endif

	if (rStatus != WLAN_STATUS_SUCCESS) {
		return rStatus;
	}

	return rStatus;

}				/*                                 */

#endif
/*                                                                            */
/* 
                                                                                     
                                                                                     
 
                                                       
                                                                              
                                                        
                                                                            
                                                                               
                                                         
 
                             
                                                                 
                                    
 
*/
/*                                                                            */
static WLAN_STATUS
reqExtSetAcpiDevicePowerState(IN P_GLUE_INFO_T prGlueInfo,
			      IN PVOID pvSetBuffer,
			      IN UINT_32 u4SetBufferLen, OUT PUINT_32 pu4SetInfoLen)
{
	WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;

	ASSERT(prGlueInfo);
	ASSERT(pvSetBuffer);
	ASSERT(pu4SetInfoLen);

	/*                                                                           */

	/*                                                     */
	/*                                 */
	/*              */
	/*                 */
	/*                 */
	return rStatus;
}

#define CMD_START				"START"
#define CMD_STOP				"STOP"
#define CMD_SCAN_ACTIVE			"SCAN-ACTIVE"
#define CMD_SCAN_PASSIVE		"SCAN-PASSIVE"
#define CMD_RSSI				"RSSI"
#define CMD_LINKSPEED			"LINKSPEED"
#define CMD_RXFILTER_START		"RXFILTER-START"
#define CMD_RXFILTER_STOP		"RXFILTER-STOP"
#define CMD_RXFILTER_ADD		"RXFILTER-ADD"
#define CMD_RXFILTER_REMOVE		"RXFILTER-REMOVE"
#define CMD_BTCOEXSCAN_START	"BTCOEXSCAN-START"
#define CMD_BTCOEXSCAN_STOP		"BTCOEXSCAN-STOP"
#define CMD_BTCOEXMODE			"BTCOEXMODE"
#define CMD_SETSUSPENDOPT		"SETSUSPENDOPT"
#define CMD_SETSUSPENDMODE		"SETSUSPENDMODE"
#define CMD_P2P_DEV_ADDR		"P2P_DEV_ADDR"
#define CMD_SETFWPATH			"SETFWPATH"
#define CMD_SETBAND				"SETBAND"
#define CMD_GETBAND				"GETBAND"
#define CMD_COUNTRY				"COUNTRY"
#define CMD_P2P_SET_NOA			"P2P_SET_NOA"
#define CMD_P2P_GET_NOA			"P2P_GET_NOA"
#define CMD_P2P_SET_PS			"P2P_SET_PS"
#define CMD_SET_AP_WPS_P2P_IE	"SET_AP_WPS_P2P_IE"
#define CMD_SETROAMMODE	"SETROAMMODE"
#define CMD_MIRACAST		"MIRACAST"


#define CMD_PNOSSIDCLR_SET	"PNOSSIDCLR"
#define CMD_PNOSETUP_SET	"PNOSETUP "
#define CMD_PNOENABLE_SET	"PNOFORCE"
#define CMD_PNODEBUG_SET	"PNODEBUG"
#define CMD_WLS_BATCHING	"WLS_BATCHING"

#define CMD_OKC_SET_PMK		"SET_PMK"
#define CMD_OKC_ENABLE		"OKC_ENABLE"

/*                             */
#define MIRACAST_MODE_OFF	0
#define MIRACAST_MODE_SOURCE	1
#define MIRACAST_MODE_SINK	2

#ifndef MIRACAST_AMPDU_SIZE
#define MIRACAST_AMPDU_SIZE	8
#endif

#ifndef MIRACAST_MCHAN_ALGO
#define MIRACAST_MCHAN_ALGO     1
#endif

#ifndef MIRACAST_MCHAN_BW
#define MIRACAST_MCHAN_BW       25
#endif

#define	CMD_BAND_AUTO	0
#define	CMD_BAND_5G		1
#define	CMD_BAND_2G		2
#define	CMD_BAND_ALL	3

/*                          */

#define CMD_SET_SW_CTRL	        "SET_SW_CTRL"
#define CMD_GET_SW_CTRL         "GET_SW_CTRL"
#define CMD_SET_CFG             "SET_CFG"
#define CMD_GET_CFG             "GET_CFG"
#define CMD_SET_CHIP            "SET_CHIP"
#define CMD_GET_CHIP            "GET_CHIP"
#define CMD_SET_DBG_LEVEL       "SET_DBG_LEVEL"
#define CMD_GET_DBG_LEVEL       "GET_DBG_LEVEL"


static UINT_8 g_ucMiracastMode = MIRACAST_MODE_OFF;


typedef struct cmd_tlv {
	char prefix;
	char version;
	char subver;
	char reserved;
} cmd_tlv_t;

typedef struct priv_driver_cmd_s {
	char *buf;
	int used_len;
	int total_len;
} priv_driver_cmd_t;


int priv_driver_set_dbg_level(IN struct net_device *prNetDev, IN char *pcCommand, IN int i4TotalLen)
{
	P_GLUE_INFO_T prGlueInfo = NULL;
	INT_32 i4BytesWritten = 0;
	INT_32 i4Argc = 0;
	PCHAR apcArgv[WLAN_CFG_ARGV_MAX];
	UINT_32 u4DbgIdx, u4DbgMask;
	BOOLEAN fgIsCmdAccept = FALSE;

	ASSERT(prNetDev);
	if (FALSE == GLUE_CHK_PR2(prNetDev, pcCommand)) {
		return -1;
	}
	prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prNetDev));

	DBGLOG(REQ, LOUD, ("command is %s\n", pcCommand));
	wlanCfgParseArgument(pcCommand, &i4Argc, apcArgv);
	DBGLOG(REQ, LOUD, ("argc is %i\n", i4Argc));


	if (i4Argc >= 3) {
		u4DbgIdx = kalStrtoul(apcArgv[1], NULL, 0);
		u4DbgMask = kalStrtoul(apcArgv[2], NULL, 0);

		/*                           */
		if (u4DbgIdx == 0xFFFFFFFF) {
			fgIsCmdAccept = TRUE;
			wlanSetDebugLevel(DBG_ALL_MODULE_IDX, u4DbgMask);
			i4BytesWritten =
			    snprintf(pcCommand, i4TotalLen,
				     "Set ALL DBG module log level to [0x%02x]!",
				     (UINT_8) u4DbgMask);
		} else if (u4DbgIdx == 0xFFFFFFFE) {
			fgIsCmdAccept = TRUE;
			wlanDebugInit();
			i4BytesWritten =
			    snprintf(pcCommand, i4TotalLen,
				     "Reset ALL DBG module log level to DEFAULT!");
		} else if (u4DbgIdx < DBG_MODULE_NUM) {
			fgIsCmdAccept = TRUE;
			wlanSetDebugLevel(u4DbgIdx, u4DbgMask);
			i4BytesWritten =
			    snprintf(pcCommand, i4TotalLen,
				     "Set DBG module[%lu] log level to [0x%02x]!", u4DbgIdx,
				     (UINT_8) u4DbgMask);
		}
	}

	if (!fgIsCmdAccept) {
		i4BytesWritten =
		    snprintf(pcCommand, i4TotalLen, "Set DBG module log level failed!");
	}

	return i4BytesWritten;

}				/*                         */

int priv_driver_get_dbg_level(IN struct net_device *prNetDev, IN char *pcCommand, IN int i4TotalLen)
{
	P_GLUE_INFO_T prGlueInfo = NULL;
	INT_32 i4BytesWritten = 0;
	INT_32 i4Argc = 0;
	PCHAR apcArgv[WLAN_CFG_ARGV_MAX];
	UINT_32 u4DbgIdx, u4DbgMask;
	BOOLEAN fgIsCmdAccept = FALSE;

	ASSERT(prNetDev);
	if (FALSE == GLUE_CHK_PR2(prNetDev, pcCommand)) {
		return -1;
	}
	prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prNetDev));

	DBGLOG(REQ, LOUD, ("command is %s\n", pcCommand));
	wlanCfgParseArgument(pcCommand, &i4Argc, apcArgv);
	DBGLOG(REQ, LOUD, ("argc is %i\n", i4Argc));

	if (i4Argc >= 2) {
		u4DbgIdx = kalStrtoul(apcArgv[1], NULL, 0);

		if (wlanGetDebugLevel(u4DbgIdx, &u4DbgMask) == WLAN_STATUS_SUCCESS) {
			fgIsCmdAccept = TRUE;
			i4BytesWritten =
			    snprintf(pcCommand, i4TotalLen,
				     "Get DBG module[%lu] log level => [0x%02x]!", u4DbgIdx,
				     (UINT_8) u4DbgMask);
		}
	}

	if (!fgIsCmdAccept) {
		i4BytesWritten =
		    snprintf(pcCommand, i4TotalLen, "Get DBG module log level failed!");
	}

	return i4BytesWritten;

}				/*                         */

static int
priv_driver_get_sw_ctrl(IN struct net_device *prNetDev, IN char *pcCommand, IN int i4TotalLen)
{
	P_GLUE_INFO_T prGlueInfo = NULL;
	WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
	UINT_32 u4BufLen = 0;
	INT_32 i4BytesWritten = 0;
	INT_32 i4Argc = 0;
	PCHAR apcArgv[WLAN_CFG_ARGV_MAX];

	PARAM_CUSTOM_SW_CTRL_STRUC_T rSwCtrlInfo;

	ASSERT(prNetDev);
	if (FALSE == GLUE_CHK_PR2(prNetDev, pcCommand)) {
		return -1;
	}
	prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prNetDev));

	DBGLOG(REQ, LOUD, ("command is %s\n", pcCommand));
	wlanCfgParseArgument(pcCommand, &i4Argc, apcArgv);
	DBGLOG(REQ, LOUD, ("argc is %i\n", i4Argc));

	if (i4Argc >= 2) {
		rSwCtrlInfo.u4Id = kalStrtoul(apcArgv[1], NULL, 0);
		rSwCtrlInfo.u4Data = 0;

		DBGLOG(REQ, LOUD, ("id is %x\n", rSwCtrlInfo.u4Id));

		rStatus = kalIoctl(prGlueInfo,
				   wlanoidQuerySwCtrlRead,
				   &rSwCtrlInfo, sizeof(rSwCtrlInfo), TRUE, TRUE, TRUE, &u4BufLen);

		DBGLOG(REQ, LOUD, ("rStatus %u\n", rStatus));
		if (rStatus != WLAN_STATUS_SUCCESS) {
			return -1;
		}

		i4BytesWritten =
		    snprintf(pcCommand, i4TotalLen, "0x%08x", (unsigned int)rSwCtrlInfo.u4Data);
		DBGLOG(REQ, INFO, ("%s: command result is %s\n", __func__, pcCommand));
	}

	return i4BytesWritten;

}				/*                         */


int priv_driver_set_sw_ctrl(IN struct net_device *prNetDev, IN char *pcCommand, IN int i4TotalLen)
{
	P_GLUE_INFO_T prGlueInfo = NULL;
	WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
	UINT_32 u4BufLen = 0;
	INT_32 i4BytesWritten = 0;
	INT_32 i4Argc = 0;
	PCHAR apcArgv[WLAN_CFG_ARGV_MAX] = { 0 };

	PARAM_CUSTOM_SW_CTRL_STRUC_T rSwCtrlInfo;

	ASSERT(prNetDev);
	if (FALSE == GLUE_CHK_PR2(prNetDev, pcCommand)) {
		return -1;
	}
	prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prNetDev));

	DBGLOG(REQ, LOUD, ("command is %s\n", pcCommand));
	wlanCfgParseArgument(pcCommand, &i4Argc, apcArgv);
	DBGLOG(REQ, LOUD, ("argc is %i\n", i4Argc));

	if (i4Argc >= 3) {
		rSwCtrlInfo.u4Id = kalStrtoul(apcArgv[1], NULL, 0);
		rSwCtrlInfo.u4Data = kalStrtoul(apcArgv[2], NULL, 0);

		rStatus = kalIoctl(prGlueInfo,
				   wlanoidSetSwCtrlWrite,
				   &rSwCtrlInfo,
				   sizeof(rSwCtrlInfo), FALSE, FALSE, TRUE, &u4BufLen);

		if (rStatus != WLAN_STATUS_SUCCESS) {
			return -1;
		}

	}

	return i4BytesWritten;

}				/*                         */

int priv_driver_set_cfg(IN struct net_device *prNetDev, IN char *pcCommand, IN int i4TotalLen)
{
	WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
	P_GLUE_INFO_T prGlueInfo = NULL;
	P_ADAPTER_T prAdapter = NULL;
	UINT_32 u4BufLen = 0;
	INT_32 i4BytesWritten = 0;
	INT_32 i4Argc = 0;
	PCHAR apcArgv[WLAN_CFG_ARGV_MAX] = { 0 };

	PARAM_CUSTOM_KEY_CFG_STRUC_T rKeyCfgInfo;

	ASSERT(prNetDev);
	if (FALSE == GLUE_CHK_PR2(prNetDev, pcCommand)) {
		return -1;
	}
	prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prNetDev));

	DBGLOG(REQ, LOUD, ("command is %s\n", pcCommand));
	wlanCfgParseArgument(pcCommand, &i4Argc, apcArgv);
	DBGLOG(REQ, LOUD, ("argc is %i\n", i4Argc));
	prAdapter = prGlueInfo->prAdapter;

	kalMemZero(&rKeyCfgInfo, sizeof(rKeyCfgInfo));

	if (i4Argc >= 3) {
		/*                                                   */
		/*                                                            */
		kalStrnCpy(rKeyCfgInfo.aucKey, apcArgv[1], WLAN_CFG_KEY_LEN_MAX);
		kalStrnCpy(rKeyCfgInfo.aucValue, apcArgv[2], WLAN_CFG_KEY_LEN_MAX);
		rStatus = kalIoctl(prGlueInfo,
				   wlanoidSetKeyCfg,
				   &rKeyCfgInfo,
				   sizeof(rKeyCfgInfo), FALSE, FALSE, TRUE, &u4BufLen);

		if (rStatus != WLAN_STATUS_SUCCESS) {
			return -1;
		}
	}

	return i4BytesWritten;

}				/*                      */

int priv_driver_get_cfg(IN struct net_device *prNetDev, IN char *pcCommand, IN int i4TotalLen)
{
	P_GLUE_INFO_T prGlueInfo = NULL;
	P_ADAPTER_T prAdapter = NULL;
	INT_32 i4BytesWritten = 0;
	INT_32 i4Argc = 0;
	PCHAR apcArgv[WLAN_CFG_ARGV_MAX] = { 0 };
	CHAR aucValue[WLAN_CFG_VALUE_LEN_MAX];

	ASSERT(prNetDev);
	if (FALSE == GLUE_CHK_PR2(prNetDev, pcCommand)) {
		return -1;
	}

	prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prNetDev));

	DBGLOG(REQ, LOUD, ("command is %s\n", pcCommand));
	wlanCfgParseArgument(pcCommand, &i4Argc, apcArgv);
	DBGLOG(REQ, LOUD, ("argc is %i\n", i4Argc));
	prAdapter = prGlueInfo->prAdapter;

	if (i4Argc >= 2) {
		/*              */
		if (wlanCfgGet(prAdapter, apcArgv[1], aucValue, "", 0) == WLAN_STATUS_SUCCESS) {
			kalStrnCpy(pcCommand, aucValue, WLAN_CFG_VALUE_LEN_MAX);
			i4BytesWritten = kalStrnLen(pcCommand, WLAN_CFG_VALUE_LEN_MAX);
		}
	}

	return i4BytesWritten;

}				/*                      */


int
priv_driver_set_chip_config(IN struct net_device *prNetDev, IN char *pcCommand, IN int i4TotalLen)
{
	WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
	P_GLUE_INFO_T prGlueInfo = NULL;
	P_ADAPTER_T prAdapter = NULL;
	UINT_32 u4BufLen = 0;
	INT_32 i4BytesWritten = 0;
	UINT_32 u4CmdLen = 0;
	UINT_32 u4PrefixLen = 0;
	/*                    */
	/*                                          */

	PARAM_CUSTOM_CHIP_CONFIG_STRUC_T rChipConfigInfo;

	ASSERT(prNetDev);
	if (FALSE == GLUE_CHK_PR2(prNetDev, pcCommand)) {
		return -1;
	}
	prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prNetDev));
	prAdapter = prGlueInfo->prAdapter;

	DBGLOG(REQ, LOUD, ("command is %s\n", pcCommand));
	/*                                                    */
	/*                                            */
	/*  */
	u4CmdLen = kalStrnLen(pcCommand, i4TotalLen);
	u4PrefixLen = kalStrLen(CMD_SET_CHIP) + 1 /*      */;

	kalMemZero(&rChipConfigInfo, sizeof(rChipConfigInfo));

	/*                   */
	if (u4CmdLen > u4PrefixLen) {

		rChipConfigInfo.ucType = CHIP_CONFIG_TYPE_WO_RESPONSE;
		/*                                                                           */
		rChipConfigInfo.u2MsgSize = u4CmdLen - u4PrefixLen;
		/*                                                                      */
		kalStrnCpy(rChipConfigInfo.aucCmd, pcCommand + u4PrefixLen, CHIP_CONFIG_RESP_SIZE);

		rStatus = kalIoctl(prGlueInfo,
				   wlanoidSetChipConfig,
				   &rChipConfigInfo,
				   sizeof(rChipConfigInfo), FALSE, FALSE, TRUE, &u4BufLen);

		if (rStatus != WLAN_STATUS_SUCCESS) {
			DBGLOG(REQ, INFO, ("%s: kalIoctl ret=%d\n", __func__, rStatus));
			i4BytesWritten = -1;
		}
	}

	return i4BytesWritten;

}				/*                              */

int
priv_driver_get_chip_config(IN struct net_device *prNetDev, IN char *pcCommand, IN int i4TotalLen)
{
	WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
	P_GLUE_INFO_T prGlueInfo = NULL;
	P_ADAPTER_T prAdapter = NULL;
	INT_32 i4BytesWritten = 0;
	UINT_32 u4BufLen = 0;
	UINT_32 u2MsgSize = 0;
	UINT_32 u4CmdLen = 0;
	UINT_32 u4PrefixLen = 0;
	/*                    */
	/*                                    */

	PARAM_CUSTOM_CHIP_CONFIG_STRUC_T rChipConfigInfo;

	ASSERT(prNetDev);
	if (FALSE == GLUE_CHK_PR2(prNetDev, pcCommand)) {
		return -1;
	}

	prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prNetDev));
	prAdapter = prGlueInfo->prAdapter;

	DBGLOG(REQ, LOUD, ("command is %s\n", pcCommand));
	/*                                                    */
	/*                                            */

	u4CmdLen = kalStrnLen(pcCommand, i4TotalLen);
	u4PrefixLen = kalStrLen(CMD_GET_CHIP) + 1 /*      */;

	/*                   */
	if (u4CmdLen > u4PrefixLen) {
		rChipConfigInfo.ucType = CHIP_CONFIG_TYPE_ASCII;
		/*                                                                           */
		rChipConfigInfo.u2MsgSize = u4CmdLen - u4PrefixLen;
		/*                                                                      */
		kalStrnCpy(rChipConfigInfo.aucCmd, pcCommand + u4PrefixLen, CHIP_CONFIG_RESP_SIZE);
		rStatus = kalIoctl(prGlueInfo,
				   wlanoidQueryChipConfig,
				   &rChipConfigInfo,
				   sizeof(rChipConfigInfo), TRUE, TRUE, TRUE, &u4BufLen);

		if (rStatus != WLAN_STATUS_SUCCESS) {
			DBGLOG(REQ, INFO, ("%s: kalIoctl ret=%d\n", __func__, rStatus));
			return -1;
		}

		/*                */
		u2MsgSize = rChipConfigInfo.u2MsgSize;
		DBGLOG(REQ, INFO, ("%s: RespTyep  %u\n", __func__, rChipConfigInfo.ucRespType));
		DBGLOG(REQ, INFO, ("%s: u2MsgSize %u\n", __func__, rChipConfigInfo.u2MsgSize));

		if (u2MsgSize > sizeof(rChipConfigInfo.aucCmd)) {
			DBGLOG(REQ, INFO,
			       ("%s: u2MsgSize error ret=%u\n", __func__,
				rChipConfigInfo.u2MsgSize));
			return -1;
		}

		if (u2MsgSize > 0) {

			if (rChipConfigInfo.ucRespType == CHIP_CONFIG_TYPE_ASCII) {
				i4BytesWritten =
				    snprintf(pcCommand + i4BytesWritten, i4TotalLen, "%s",
					     rChipConfigInfo.aucCmd);
			} else {
				UINT_32 u4Length;
				UINT_32 u4Line;

				if (rChipConfigInfo.ucRespType == CHIP_CONFIG_TYPE_MEM8) {
					PUINT_8 pucStartAddr = NULL;
					pucStartAddr = (PUINT_8) rChipConfigInfo.aucCmd;
					/*                                                   */
					u4Length = (((u2MsgSize + 15) >> 4)) << 4;
					u4Line = 0;
					priv_driver_get_chip_config_16(pucStartAddr, u4Length, u4Line, i4TotalLen, i4BytesWritten, pcCommand);
				} else {
					PUINT_32 pu4StartAddr = NULL;
					pu4StartAddr = (PUINT_32) rChipConfigInfo.aucCmd;
					/*                                                   */
					u4Length = (((u2MsgSize + 15) >> 4)) << 4;
					u4Line = 0;

					if (IS_ALIGN_4((ULONG) pu4StartAddr)) {
						priv_driver_get_chip_config_4(pu4StartAddr, u4Length, u4Line, i4TotalLen, i4BytesWritten, pcCommand);
					} else {
						DBGLOG(REQ, INFO,
						       ("%s: rChipConfigInfo.aucCmd is not 4 bytes alignment %p\n",
							__func__, rChipConfigInfo.aucCmd));
					}
				}	/*                           */
			}
		}
		/*               */
		DBGLOG(REQ, INFO, ("%s: command result is %s\n", __func__, pcCommand));
	}
	/*        */
	return i4BytesWritten;

}				/*                              */

static void
priv_driver_get_chip_config_16(PUINT_8 pucStartAddr, UINT_32 u4Length, UINT_32 u4Line, int i4TotalLen, INT_32 i4BytesWritten, char *pcCommand)
{

	while (u4Length >= 16) {
		if (i4TotalLen > i4BytesWritten) {
			i4BytesWritten +=
			    snprintf(pcCommand + i4BytesWritten,
				     i4TotalLen - i4BytesWritten,
				     "%04lx %02x %02x %02x %02x  %02x %02x %02x %02x - %02x %02x %02x %02x  %02x %02x %02x %02x\n",
				     u4Line, pucStartAddr[0],
				     pucStartAddr[1],
				     pucStartAddr[2],
				     pucStartAddr[3],
				     pucStartAddr[4],
				     pucStartAddr[5],
				     pucStartAddr[6],
				     pucStartAddr[7],
				     pucStartAddr[8],
				     pucStartAddr[9],
				     pucStartAddr[10],
				     pucStartAddr[11],
				     pucStartAddr[12],
				     pucStartAddr[13],
				     pucStartAddr[14],
				     pucStartAddr[15]);
		}

		pucStartAddr += 16;
		u4Length -= 16;
		u4Line += 16;
	}	/*          */
}

static void
priv_driver_get_chip_config_4(PUINT_32 pu4StartAddr, UINT_32 u4Length, UINT_32 u4Line, int i4TotalLen, INT_32 i4BytesWritten, char *pcCommand)
{
	while (u4Length >= 16) {
		if (i4TotalLen > i4BytesWritten) {
			i4BytesWritten +=
			    snprintf(pcCommand +
				     i4BytesWritten,
				     i4TotalLen -
				     i4BytesWritten,
				     "%04lx %08lx %08lx %08lx %08lx\n",
				     u4Line,
				     pu4StartAddr[0],
				     pu4StartAddr[1],
				     pu4StartAddr[2],
				     pu4StartAddr[3]);
		}

		pu4StartAddr += 4;
		u4Length -= 16;
		u4Line += 4;
	}	/*          */
}

int priv_driver_get_linkspeed(IN struct net_device *prNetDev, IN char *pcCommand, IN int i4TotalLen)
{
	P_GLUE_INFO_T prGlueInfo = NULL;
	WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
	UINT_32 u4BufLen = 0;
	UINT_32 u4Rate = 0;
	UINT_32 u4LinkSpeed = 0;
	INT_32 i4BytesWritten = 0;

	ASSERT(prNetDev);
	if (FALSE == GLUE_CHK_PR2(prNetDev, pcCommand)) {
		return -1;
	}
	prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prNetDev));

	if (!netif_carrier_ok(prNetDev)) {
		return -1;
	}

	rStatus = kalIoctl(prGlueInfo,
			   wlanoidQueryLinkSpeed,
			   &u4Rate, sizeof(u4Rate), TRUE, TRUE, TRUE, &u4BufLen);

	if (rStatus != WLAN_STATUS_SUCCESS) {
		return -1;
	}

	u4LinkSpeed = u4Rate * 100;
	i4BytesWritten = snprintf(pcCommand, i4TotalLen, "LinkSpeed %u", (unsigned int)u4LinkSpeed);
	DBGLOG(REQ, INFO, ("%s: command result is %s\n", __func__, pcCommand));
	return i4BytesWritten;

}				/*                           */


int priv_driver_set_band(IN struct net_device *prNetDev, IN char *pcCommand, IN int i4TotalLen)
{
	P_ADAPTER_T prAdapter = NULL;
	P_GLUE_INFO_T prGlueInfo = NULL;
	INT_32 i4Argc = 0;
	UINT_8 ucBand = 0;
	UINT_8 ucBssIndex;
	ENUM_BAND_T eBand = BAND_NULL;
	PCHAR apcArgv[WLAN_CFG_ARGV_MAX];

	ASSERT(prNetDev);
	if (FALSE == GLUE_CHK_PR2(prNetDev, pcCommand)) {
		return -1;
	}
	prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prNetDev));

	DBGLOG(REQ, LOUD, ("command is %s\n", pcCommand));
	wlanCfgParseArgument(pcCommand, &i4Argc, apcArgv);
	DBGLOG(REQ, LOUD, ("argc is %i\n", i4Argc));

	prAdapter = prGlueInfo->prAdapter;
	if (i4Argc >= 2) {
		ucBand = kalStrtoul(apcArgv[1], NULL, 0);
		ucBssIndex = wlanGetAisBssIndex(prGlueInfo->prAdapter);
		eBand = BAND_NULL;
		if (ucBand == CMD_BAND_5G)
			eBand = BAND_5G;
		else if (ucBand == CMD_BAND_2G)
			eBand = BAND_2G4;
		prAdapter->aePreferBand[ucBssIndex] = eBand;
		/*                                                                  */
		/*                                                            */
	}

	return 0;
}


int priv_driver_set_country(IN struct net_device *prNetDev, IN char *pcCommand, IN int i4TotalLen)
{

	P_GLUE_INFO_T prGlueInfo = NULL;
	WLAN_STATUS rStatus = WLAN_STATUS_SUCCESS;
	UINT_32 u4BufLen = 0;
	INT_32 i4Argc = 0;
	PCHAR apcArgv[WLAN_CFG_ARGV_MAX];
	UINT_8 aucCountry[2];

	ASSERT(prNetDev);
	if (FALSE == GLUE_CHK_PR2(prNetDev, pcCommand)) {
		return -1;
	}
	prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prNetDev));

	DBGLOG(REQ, LOUD, ("command is %s\n", pcCommand));
	wlanCfgParseArgument(pcCommand, &i4Argc, apcArgv);
	DBGLOG(REQ, LOUD, ("argc is %i\n", i4Argc));

	if (i4Argc >= 2) {
		/*                                                          */
		aucCountry[0] = apcArgv[1][0];
		aucCountry[1] = apcArgv[1][1];

		rStatus = kalIoctl(prGlueInfo,
				   wlanoidSetCountryCode,
				   &aucCountry[0], 2, FALSE, FALSE, TRUE, &u4BufLen);

		if (rStatus != WLAN_STATUS_SUCCESS)
			return -1;
	}
	return 0;
}

int priv_driver_set_miracast(IN struct net_device *prNetDev, IN char *pcCommand, IN int i4TotalLen)
{

	P_ADAPTER_T prAdapter = NULL;
	P_GLUE_INFO_T prGlueInfo = NULL;
	UINT_32 i4BytesWritten = 0;
	/*                                            */
	/*                       */
	INT_32 i4Argc = 0;
	UINT_8 ucMode = 0;
	P_WFD_CFG_SETTINGS_T prWfdCfgSettings = (P_WFD_CFG_SETTINGS_T) NULL;
	P_MSG_WFD_CONFIG_SETTINGS_CHANGED_T prMsgWfdCfgUpdate =
	    (P_MSG_WFD_CONFIG_SETTINGS_CHANGED_T) NULL;
	PCHAR apcArgv[WLAN_CFG_ARGV_MAX];

	ASSERT(prNetDev);
	if (FALSE == GLUE_CHK_PR2(prNetDev, pcCommand)) {
		return -1;
	}



	prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prNetDev));

	DBGLOG(REQ, LOUD, ("command is %s\n", pcCommand));
	wlanCfgParseArgument(pcCommand, &i4Argc, apcArgv);
	DBGLOG(REQ, LOUD, ("argc is %i\n", i4Argc));


	prAdapter = prGlueInfo->prAdapter;
	if (i4Argc >= 2) {

		ucMode = kalStrtoul(apcArgv[1], NULL, 0);

		if (g_ucMiracastMode == ucMode) {
			/*                       */
		}

		g_ucMiracastMode = ucMode;
		prMsgWfdCfgUpdate =
		    cnmMemAlloc(prAdapter, RAM_TYPE_MSG, sizeof(MSG_WFD_CONFIG_SETTINGS_CHANGED_T));

		if (prMsgWfdCfgUpdate != NULL) {

			prWfdCfgSettings = &(prAdapter->rWifiVar.rWfdConfigureSettings);
			prMsgWfdCfgUpdate->rMsgHdr.eMsgId = MID_MNY_P2P_WFD_CFG_UPDATE;
			prMsgWfdCfgUpdate->prWfdCfgSettings = prWfdCfgSettings;

			if (ucMode == MIRACAST_MODE_OFF) {
				prWfdCfgSettings->ucWfdEnable = 0;
				snprintf(pcCommand, i4TotalLen, CMD_SET_CHIP " mira 0");
			} else if (ucMode == MIRACAST_MODE_SOURCE) {
				prWfdCfgSettings->ucWfdEnable = 1;
				snprintf(pcCommand, i4TotalLen, CMD_SET_CHIP " mira 1");
			} else if (ucMode == MIRACAST_MODE_SINK) {
				prWfdCfgSettings->ucWfdEnable = 2;
				snprintf(pcCommand, i4TotalLen, CMD_SET_CHIP " mira 2");
			} else {
				prWfdCfgSettings->ucWfdEnable = 0;
				snprintf(pcCommand, i4TotalLen, CMD_SET_CHIP " mira 0");
			}

			mboxSendMsg(prAdapter,
				    MBOX_ID_0,
				    (P_MSG_HDR_T) prMsgWfdCfgUpdate, MSG_SEND_METHOD_BUF);

			priv_driver_set_chip_config(prNetDev, pcCommand, i4TotalLen);
		} /*                   */
		else {
			ASSERT(FALSE);
			i4BytesWritten = -1;
		}
	}

	/*        */
	return i4BytesWritten;
}

int
priv_driver_set_suspend_mode(IN struct net_device *prNetDev, IN char *pcCommand, IN int i4TotalLen)
{
	P_GLUE_INFO_T prGlueInfo = NULL;
	INT_32 i4Argc = 0;
	PCHAR apcArgv[WLAN_CFG_ARGV_MAX];
	BOOLEAN fgEnable;

	ASSERT(prNetDev);
	if (FALSE == GLUE_CHK_PR2(prNetDev, pcCommand)) {
		return -1;
	}
	prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prNetDev));

	DBGLOG(REQ, LOUD, ("command is %s\n", pcCommand));
	wlanCfgParseArgument(pcCommand, &i4Argc, apcArgv);
	DBGLOG(REQ, LOUD, ("argc is %i\n", i4Argc));

	if (i4Argc >= 2) {
		fgEnable = (kalStrtoul(apcArgv[1], NULL, 0) == 1) ? TRUE : FALSE;

		DBGLOG(REQ, INFO, ("%s: Set suspend mode [%u]\n", __func__, fgEnable));

		if (prGlueInfo->fgIsInSuspendMode == fgEnable) {
			DBGLOG(REQ, INFO, ("%s: Already in suspend mode, SKIP!\n", __func__));
			return 0;
		}

		prGlueInfo->fgIsInSuspendMode = fgEnable;

		wlanSetSuspendMode(prGlueInfo, fgEnable);
		p2pSetSuspendMode(prGlueInfo, fgEnable);
	}

	return 0;
}


#if CFG_SUPPORT_BATCH_SCAN
#define CMD_BATCH_SET           "WLS_BATCHING SET"
#define CMD_BATCH_GET           "WLS_BATCHING GET"
#define CMD_BATCH_STOP          "WLS_BATCHING STOP"
#endif


INT_32 priv_driver_cmds(IN struct net_device *prNetDev, IN PCHAR pcCommand, IN INT_32 i4TotalLen)
{
	P_GLUE_INFO_T prGlueInfo = NULL;
	INT_32 i4BytesWritten = 0;
	INT_32 i4CmdFound = 0;

	if (FALSE == GLUE_CHK_PR2(prNetDev, pcCommand)) {
		return -1;
	}
	prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prNetDev));

	if (i4CmdFound == 0) {
		i4CmdFound = 1;
		if (strnicmp(pcCommand, CMD_RSSI, strlen(CMD_RSSI)) == 0) {
			/*                                                                 */
		} else if (strnicmp(pcCommand, CMD_LINKSPEED, strlen(CMD_LINKSPEED)) == 0) {
			i4BytesWritten = priv_driver_get_linkspeed(prNetDev, pcCommand, i4TotalLen);
		} else if (strnicmp(pcCommand, CMD_PNOSSIDCLR_SET, strlen(CMD_PNOSSIDCLR_SET)) == 0) {
		} else if (strnicmp(pcCommand, CMD_PNOSETUP_SET, strlen(CMD_PNOSETUP_SET)) == 0) {
		} else if (strnicmp(pcCommand, CMD_PNOENABLE_SET, strlen(CMD_PNOENABLE_SET)) == 0) {
		} else if (strnicmp(pcCommand, CMD_SETSUSPENDOPT, strlen(CMD_SETSUSPENDOPT)) == 0) {
			/*                                                                         */
		} else if (strnicmp(pcCommand, CMD_SETSUSPENDMODE, strlen(CMD_SETSUSPENDMODE)) == 0) {
			i4BytesWritten =
			    priv_driver_set_suspend_mode(prNetDev, pcCommand, i4TotalLen);
		} else if (strnicmp(pcCommand, CMD_SETBAND, strlen(CMD_SETBAND)) == 0) {
			i4BytesWritten = priv_driver_set_band(prNetDev, pcCommand, i4TotalLen);
		} else if (strnicmp(pcCommand, CMD_GETBAND, strlen(CMD_GETBAND)) == 0) {
			/*                                                                   */
		} else if (strnicmp(pcCommand, CMD_COUNTRY, strlen(CMD_COUNTRY)) == 0) {
			i4BytesWritten = priv_driver_set_country(prNetDev, pcCommand, i4TotalLen);
		} else if (strnicmp(pcCommand, CMD_MIRACAST, strlen(CMD_MIRACAST)) == 0) {
			i4BytesWritten = priv_driver_set_miracast(prNetDev, pcCommand, i4TotalLen);
		}
		/*                           */
		else if (strnicmp(pcCommand, CMD_SET_SW_CTRL, strlen(CMD_SET_SW_CTRL)) == 0) {
			i4BytesWritten = priv_driver_set_sw_ctrl(prNetDev, pcCommand, i4TotalLen);
		} else if (strnicmp(pcCommand, CMD_GET_SW_CTRL, strlen(CMD_GET_SW_CTRL)) == 0) {
			i4BytesWritten = priv_driver_get_sw_ctrl(prNetDev, pcCommand, i4TotalLen);
		} else if (strnicmp(pcCommand, CMD_SET_CFG, strlen(CMD_SET_CFG)) == 0) {
			i4BytesWritten = priv_driver_set_cfg(prNetDev, pcCommand, i4TotalLen);
		} else if (strnicmp(pcCommand, CMD_GET_CFG, strlen(CMD_GET_CFG)) == 0) {
			i4BytesWritten = priv_driver_get_cfg(prNetDev, pcCommand, i4TotalLen);
		} else if (strnicmp(pcCommand, CMD_SET_CHIP, strlen(CMD_SET_CHIP)) == 0) {
			i4BytesWritten =
			    priv_driver_set_chip_config(prNetDev, pcCommand, i4TotalLen);
		} else if (strnicmp(pcCommand, CMD_GET_CHIP, strlen(CMD_GET_CHIP)) == 0) {
			i4BytesWritten =
			    priv_driver_get_chip_config(prNetDev, pcCommand, i4TotalLen);
		} else if (strnicmp(pcCommand, CMD_SET_DBG_LEVEL, strlen(CMD_SET_DBG_LEVEL)) == 0) {
			i4BytesWritten = priv_driver_set_dbg_level(prNetDev, pcCommand, i4TotalLen);
		} else if (strnicmp(pcCommand, CMD_GET_DBG_LEVEL, strlen(CMD_GET_DBG_LEVEL)) == 0) {
			i4BytesWritten = priv_driver_get_dbg_level(prNetDev, pcCommand, i4TotalLen);
		}
#if CFG_SUPPORT_BATCH_SCAN
		else if (strnicmp(pcCommand, CMD_BATCH_SET, strlen(CMD_BATCH_SET)) == 0) {
			kalIoctl(prGlueInfo,
				 wlanoidSetBatchScanReq,
				 (PVOID) pcCommand,
				 i4TotalLen, FALSE, FALSE, TRUE, &i4BytesWritten);
		} else if (strnicmp(pcCommand, CMD_BATCH_GET, strlen(CMD_BATCH_GET)) == 0) {
			/*                                                     */
			/*                                                               */
			/*                                                                               */

			UINT_32 u4BufLen;
			int i;
			/*             */

			for (i = 0; i < CFG_BATCH_MAX_MSCAN; i++) {
				g_rEventBatchResult[i].ucScanCount = i + 1;	/*                     */
				kalIoctl(prGlueInfo,
					 wlanoidQueryBatchScanResult,
					 (PVOID) & g_rEventBatchResult[i],
					 sizeof(EVENT_BATCH_RESULT_T), TRUE, TRUE, TRUE, &u4BufLen);
			}

#if 0
			DBGLOG(SCN, INFO,
			       ("Batch Scan Results, scan count = %u\n",
				g_rEventBatchResult.ucScanCount));
			for (i = 0; i < g_rEventBatchResult.ucScanCount; i++) {
				prEntry = &g_rEventBatchResult.arBatchResult[i];
				DBGLOG(SCN, INFO, ("Entry %u\n", i));
				DBGLOG(SCN, INFO,
				       ("	 BSSID = " MACSTR "\n",
					MAC2STR(prEntry->aucBssid)));
				DBGLOG(SCN, INFO, ("	 SSID = %s\n", prEntry->aucSSID));
				DBGLOG(SCN, INFO, ("	 SSID len = %u\n", prEntry->ucSSIDLen));
				DBGLOG(SCN, INFO, ("	 RSSI = %d\n", prEntry->cRssi));
				DBGLOG(SCN, INFO, ("	 Freq = %u\n", prEntry->ucFreq));
			}
#endif

			batchConvertResult(&g_rEventBatchResult[0], pcCommand, i4TotalLen,
					   &i4BytesWritten);

			/*                */
			/*                                                                                                  */

		} else if (strnicmp(pcCommand, CMD_BATCH_STOP, strlen(CMD_BATCH_STOP)) == 0) {
			kalIoctl(prGlueInfo,
				 wlanoidSetBatchScanReq,
				 (PVOID) pcCommand,
				 i4TotalLen, FALSE, FALSE, TRUE, &i4BytesWritten);
		}
#endif
		else {
			i4CmdFound = 0;
		}
	}
	/*            */
	if (i4CmdFound == 0) {
		DBGLOG(REQ, INFO, ("Unknown driver command %s - ignored\n", pcCommand));
	}

	if (i4BytesWritten >= 0) {
		if ((i4BytesWritten == 0) && (i4TotalLen > 0)) {
			/*                          */
			pcCommand[0] = '\0';
		}

		if (i4BytesWritten >= i4TotalLen) {
			DBGLOG(REQ, INFO,
			       ("%s: i4BytesWritten %d > i4TotalLen < %d\n", __func__,
				i4BytesWritten, i4TotalLen));
			i4BytesWritten = i4TotalLen;
		} else {
			pcCommand[i4BytesWritten] = '\0';
			i4BytesWritten++;
		}
	}



	return i4BytesWritten;

}				/*                  */


int
priv_support_driver_cmd(IN struct net_device *prNetDev, IN OUT struct ifreq *prReq, IN int i4Cmd)
{
	P_GLUE_INFO_T prGlueInfo = NULL;
	int ret = 0;
	char *pcCommand = NULL;
	int i4BytesWritten = 0;
	int i4TotalLen = 0;
	priv_driver_cmd_t priv_cmd;

	if (!prReq->ifr_data) {
		ret = -EINVAL;
		goto exit;
	}

	prGlueInfo = *((P_GLUE_INFO_T *) netdev_priv(prNetDev));
	ASSERT(prGlueInfo);
	if (!prGlueInfo) {
		DBGLOG(REQ, WARN, ("No glue info\n"));
		ret = -EFAULT;
		goto exit;
	}
	if (prGlueInfo->u4ReadyFlag == 0) {
		ret = -EINVAL;
		goto exit;
	}

	if (copy_from_user(&priv_cmd, prReq->ifr_data, sizeof(priv_driver_cmd_t))) {
		DBGLOG(REQ, INFO, ("%s: copy_from_user fail\n", __func__));
		ret = -EFAULT;
		goto exit;
	}

	i4TotalLen = priv_cmd.total_len;

	if (i4TotalLen <= 0) {
		ret = -EINVAL;
		goto exit;
	}

	if (!access_ok(VERIFY_READ, priv_cmd.buf, i4TotalLen)) {
		DBGLOG(REQ, INFO, ("%s: copy_from_user fail %d\n", __func__, i4TotalLen));
		ret = -ENOMEM;
		goto exit;
	}

	pcCommand = kmalloc(i4TotalLen, GFP_KERNEL);
	if (!pcCommand) {
		DBGLOG(REQ, INFO,
		       ("%s: failed to allocate memory size %d\n", __func__, i4TotalLen));
		ret = -ENOMEM;
		goto exit;
	}


	if (copy_from_user(pcCommand, priv_cmd.buf, i4TotalLen)) {
		ret = -EFAULT;
		goto exit;
	}

	DBGLOG(REQ, INFO,
	       ("%s: driver cmd \"%s\" on %s\n", __func__, pcCommand, prReq->ifr_name));

	i4BytesWritten = priv_driver_cmds(prNetDev, pcCommand, i4TotalLen);

	if (i4BytesWritten < 0) {
		DBGLOG(REQ, INFO,
		       ("%s: command %s Written is %d\n", __func__, pcCommand, i4BytesWritten));
		if (i4TotalLen >= 3) {
			snprintf(pcCommand, 3, "OK");
			i4BytesWritten = strlen("OK");
		}
	}

	if (i4BytesWritten >= 0) {
		if ((i4BytesWritten == 0) && (i4TotalLen > 0)) {
			/*                           */
			pcCommand[0] = '\0';
		}
		if (i4BytesWritten >= i4TotalLen) {
			DBGLOG(REQ, INFO,
			       ("%s: i4BytesWritten %d > i4TotalLen < %d\n", __func__,
				i4BytesWritten, i4TotalLen));
			i4BytesWritten = i4TotalLen;
		} else {
			pcCommand[i4BytesWritten] = '\0';
			i4BytesWritten++;
		}
		priv_cmd.used_len = i4BytesWritten;
		if (copy_to_user(priv_cmd.buf, pcCommand, i4BytesWritten)) {
			DBGLOG(REQ, WARN, ("failed to copy data to user buffer\n"));
			ret = -EFAULT;
		}
		if (copy_to_user(prReq->ifr_data, &priv_cmd, sizeof(priv_driver_cmd_t))) {
			DBGLOG(REQ, WARN, ("failed to copy driver_cmd to user buffer\n"));
			ret = -EFAULT;
		}

	}

exit:
	if (pcCommand) {
		kfree(pcCommand);
	}

	return ret;
}				/*                         */
