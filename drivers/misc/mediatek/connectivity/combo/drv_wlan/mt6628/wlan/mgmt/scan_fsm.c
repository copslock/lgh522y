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

/*                                                                              
                                                
                                                                                
*/

/*                                                                              
                                                 
                                                                                
*/

/*                                                                              
                                                  
                                                                                
*/

/*                                                                              
                                                   
                                                                                
*/
#if DBG
/*                              */
static PUINT_8 apucDebugScanState[SCAN_STATE_NUM] = {
	(PUINT_8) DISP_STRING("SCAN_STATE_IDLE"),
	(PUINT_8) DISP_STRING("SCAN_STATE_SCANNING"),
};

/*              */
#endif				/*     */

/*                                                                              
                                             
                                                                                
*/

/*                                                                              
                                                             
                                                                                
*/

/*                                                                              
                                                
                                                                                
*/
/*                                                                            */
/* 
        
 
            
 
              
*/
/*                                                                            */
VOID scnFsmSteps(IN P_ADAPTER_T prAdapter, IN ENUM_SCAN_STATE_T eNextState)
{
	P_SCAN_INFO_T prScanInfo;
	P_SCAN_PARAM_T prScanParam;
	P_MSG_HDR_T prMsgHdr;

	BOOLEAN fgIsTransition = (BOOLEAN) FALSE;

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &prScanInfo->rScanParam;

	do {

#if DBG
		DBGLOG(SCN, STATE, ("TRANSITION: [%s] -> [%s]\n",
				    apucDebugScanState[prScanInfo->eCurrentState],
				    apucDebugScanState[eNextState]));
#else
		DBGLOG(SCN, STATE, ("[%d] TRANSITION: [%d] -> [%d]\n",
				    DBG_SCN_IDX, prScanInfo->eCurrentState, eNextState));
#endif

		/*                                                                                 */
		prScanInfo->eCurrentState = eNextState;

		fgIsTransition = (BOOLEAN) FALSE;

		switch (prScanInfo->eCurrentState) {
		case SCAN_STATE_IDLE:
			/*                                     */
			if (!LINK_IS_EMPTY(&(prScanInfo->rPendingMsgList))) {
				/*                                                        */
				LINK_REMOVE_HEAD(&(prScanInfo->rPendingMsgList), prMsgHdr,
						 P_MSG_HDR_T);

				if (prMsgHdr->eMsgId == MID_AIS_SCN_SCAN_REQ
				    || prMsgHdr->eMsgId == MID_BOW_SCN_SCAN_REQ
				    || prMsgHdr->eMsgId == MID_P2P_SCN_SCAN_REQ
				    || prMsgHdr->eMsgId == MID_RLM_SCN_SCAN_REQ) {
					scnFsmHandleScanMsg(prAdapter,
							    (P_MSG_SCN_SCAN_REQ) prMsgHdr);
				} else {
					scnFsmHandleScanMsgV2(prAdapter,
							      (P_MSG_SCN_SCAN_REQ_V2) prMsgHdr);
				}

				/*                      */
				eNextState = SCAN_STATE_SCANNING;
				fgIsTransition = TRUE;

				cnmMemFree(prAdapter, prMsgHdr);
			}
			break;

		case SCAN_STATE_SCANNING:
			if (prScanParam->fgIsScanV2 == FALSE) {
				scnSendScanReq(prAdapter);
			} else {
				scnSendScanReqV2(prAdapter);
			}
			break;

		default:
			ASSERT(0);
			break;

		}
	}
	while (fgIsTransition);

	return;
}


/*                                                                            */
/* 
                                                
 
            
 
              
*/
/*                                                                            */
VOID scnSendScanReq(IN P_ADAPTER_T prAdapter)
{
	P_SCAN_INFO_T prScanInfo;
	P_SCAN_PARAM_T prScanParam;
	CMD_SCAN_REQ rCmdScanReq;
	UINT_32 i;

	ASSERT(prAdapter);

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &prScanInfo->rScanParam;

	/*                              */
	kalMemZero(&rCmdScanReq, sizeof(CMD_SCAN_REQ));

	rCmdScanReq.ucSeqNum = prScanParam->ucSeqNum;
	rCmdScanReq.ucNetworkType = (UINT_8) prScanParam->eNetTypeIndex;
	rCmdScanReq.ucScanType = (UINT_8) prScanParam->eScanType;
	rCmdScanReq.ucSSIDType = prScanParam->ucSSIDType;

	if (prScanParam->ucSSIDNum == 1) {
		COPY_SSID(rCmdScanReq.aucSSID,
			  rCmdScanReq.ucSSIDLength,
			  prScanParam->aucSpecifiedSSID[0], prScanParam->ucSpecifiedSSIDLen[0]);
	}

	rCmdScanReq.ucChannelType = (UINT_8) prScanParam->eScanChannel;

	if (prScanParam->eScanChannel == SCAN_CHANNEL_SPECIFIED) {
		/*               
                                                                  
                                                                                                   
   */
		rCmdScanReq.ucChannelListNum = prScanParam->ucChannelListNum;

		for (i = 0; i < rCmdScanReq.ucChannelListNum; i++) {
			rCmdScanReq.arChannelList[i].ucBand =
			    (UINT_8) prScanParam->arChnlInfoList[i].eBand;

			rCmdScanReq.arChannelList[i].ucChannelNum =
			    (UINT_8) prScanParam->arChnlInfoList[i].ucChannelNum;
		}
	}
#if CFG_ENABLE_WIFI_DIRECT
	if (prScanParam->eNetTypeIndex == NETWORK_TYPE_P2P_INDEX) {
		rCmdScanReq.u2ChannelDwellTime = prScanParam->u2PassiveListenInterval;
	}
#endif

	if (prScanParam->u2IELen <= MAX_IE_LENGTH) {
		rCmdScanReq.u2IELen = prScanParam->u2IELen;
	} else {
		rCmdScanReq.u2IELen = MAX_IE_LENGTH;
	}

	if (prScanParam->u2IELen) {
		kalMemCopy(rCmdScanReq.aucIE,
			   prScanParam->aucIE, sizeof(UINT_8) * rCmdScanReq.u2IELen);
	}

	wlanSendSetQueryCmd(prAdapter,
			    CMD_ID_SCAN_REQ,
			    TRUE,
			    FALSE,
			    FALSE,
			    NULL,
			    NULL,
			    OFFSET_OF(CMD_SCAN_REQ, aucIE) + rCmdScanReq.u2IELen,
			    (PUINT_8) &rCmdScanReq, NULL, 0);
}


/*                                                                            */
/* 
                                                   
 
            
 
              
*/
/*                                                                            */
VOID scnSendScanReqV2(IN P_ADAPTER_T prAdapter)
{
	P_SCAN_INFO_T prScanInfo;
	P_SCAN_PARAM_T prScanParam;
	CMD_SCAN_REQ_V2 rCmdScanReq;
	UINT_32 i;

	ASSERT(prAdapter);

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &prScanInfo->rScanParam;

	/*                              */
	kalMemZero(&rCmdScanReq, sizeof(CMD_SCAN_REQ_V2));

	rCmdScanReq.ucSeqNum = prScanParam->ucSeqNum;
	rCmdScanReq.ucNetworkType = (UINT_8) prScanParam->eNetTypeIndex;
	rCmdScanReq.ucScanType = (UINT_8) prScanParam->eScanType;
	rCmdScanReq.ucSSIDType = prScanParam->ucSSIDType;

	for (i = 0; i < prScanParam->ucSSIDNum; i++) {
		COPY_SSID(rCmdScanReq.arSSID[i].aucSsid,
			  rCmdScanReq.arSSID[i].u4SsidLen,
			  prScanParam->aucSpecifiedSSID[i], prScanParam->ucSpecifiedSSIDLen[i]);
	}

	rCmdScanReq.u2ProbeDelayTime = (UINT_8) prScanParam->u2ProbeDelayTime;
	rCmdScanReq.ucChannelType = (UINT_8) prScanParam->eScanChannel;

	if (prScanParam->eScanChannel == SCAN_CHANNEL_SPECIFIED) {
		/*               
                                                                  
                                                                                                   
   */
		rCmdScanReq.ucChannelListNum = prScanParam->ucChannelListNum;

		for (i = 0; i < rCmdScanReq.ucChannelListNum; i++) {
			rCmdScanReq.arChannelList[i].ucBand =
			    (UINT_8) prScanParam->arChnlInfoList[i].eBand;

			rCmdScanReq.arChannelList[i].ucChannelNum =
			    (UINT_8) prScanParam->arChnlInfoList[i].ucChannelNum;
		}
	}
#if CFG_ENABLE_WIFI_DIRECT
	if (prScanParam->eNetTypeIndex == NETWORK_TYPE_P2P_INDEX) {
		rCmdScanReq.u2ChannelDwellTime = prScanParam->u2PassiveListenInterval;
	}
#endif

	if (prScanParam->u2IELen <= MAX_IE_LENGTH) {
		rCmdScanReq.u2IELen = prScanParam->u2IELen;
	} else {
		rCmdScanReq.u2IELen = MAX_IE_LENGTH;
	}

	if (prScanParam->u2IELen) {
		kalMemCopy(rCmdScanReq.aucIE,
			   prScanParam->aucIE, sizeof(UINT_8) * rCmdScanReq.u2IELen);
	}

	wlanSendSetQueryCmd(prAdapter,
			    CMD_ID_SCAN_REQ_V2,
			    TRUE,
			    FALSE,
			    FALSE,
			    NULL,
			    NULL,
			    OFFSET_OF(CMD_SCAN_REQ_V2, aucIE) + rCmdScanReq.u2IELen,
			    (PUINT_8) &rCmdScanReq, NULL, 0);

}


/*                                                                            */
/* 
        
 
            
 
              
*/
/*                                                                            */
VOID scnFsmMsgStart(IN P_ADAPTER_T prAdapter, IN P_MSG_HDR_T prMsgHdr)
{
	P_SCAN_INFO_T prScanInfo;
	P_SCAN_PARAM_T prScanParam;

	ASSERT(prMsgHdr);

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &prScanInfo->rScanParam;


	if (prScanInfo->eCurrentState == SCAN_STATE_IDLE) {
		if (prMsgHdr->eMsgId == MID_AIS_SCN_SCAN_REQ
		    || prMsgHdr->eMsgId == MID_BOW_SCN_SCAN_REQ
		    || prMsgHdr->eMsgId == MID_P2P_SCN_SCAN_REQ
		    || prMsgHdr->eMsgId == MID_RLM_SCN_SCAN_REQ) {
			scnFsmHandleScanMsg(prAdapter, (P_MSG_SCN_SCAN_REQ) prMsgHdr);
		} else if (prMsgHdr->eMsgId == MID_AIS_SCN_SCAN_REQ_V2
			   || prMsgHdr->eMsgId == MID_BOW_SCN_SCAN_REQ_V2
			   || prMsgHdr->eMsgId == MID_P2P_SCN_SCAN_REQ_V2
			   || prMsgHdr->eMsgId == MID_RLM_SCN_SCAN_REQ_V2) {
			scnFsmHandleScanMsgV2(prAdapter, (P_MSG_SCN_SCAN_REQ_V2) prMsgHdr);
		} else {
			/*                                     */
			ASSERT(0);
		}

		cnmMemFree(prAdapter, prMsgHdr);
		scnFsmSteps(prAdapter, SCAN_STATE_SCANNING);
	} else {
		LINK_INSERT_TAIL(&prScanInfo->rPendingMsgList, &prMsgHdr->rLinkEntry);
	}

	return;
}



/*                                                                            */
/* 
        
 
            
 
              
*/
/*                                                                            */
VOID scnFsmMsgAbort(IN P_ADAPTER_T prAdapter, IN P_MSG_HDR_T prMsgHdr)
{
	P_MSG_SCN_SCAN_CANCEL prScanCancel;
	P_SCAN_INFO_T prScanInfo;
	P_SCAN_PARAM_T prScanParam;
	CMD_SCAN_CANCEL rCmdScanCancel;

	ASSERT(prMsgHdr);

	prScanCancel = (P_MSG_SCN_SCAN_CANCEL) prMsgHdr;
	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &prScanInfo->rScanParam;

	if (prScanInfo->eCurrentState != SCAN_STATE_IDLE) {
		if (prScanCancel->ucSeqNum == prScanParam->ucSeqNum &&
		    prScanCancel->ucNetTypeIndex == (UINT_8) prScanParam->eNetTypeIndex) {
			/*                                        */
			rCmdScanCancel.ucSeqNum = prScanParam->ucSeqNum;

#if CFG_ENABLE_WIFI_DIRECT
			if (prScanParam->eNetTypeIndex == NETWORK_TYPE_P2P_INDEX) {
				rCmdScanCancel.ucIsExtChannel =
				    (UINT_8) prScanCancel->fgIsChannelExt;
			} else {
				rCmdScanCancel.ucIsExtChannel = (UINT_8) FALSE;
			}
#endif

			wlanSendSetQueryCmd(prAdapter,
					    CMD_ID_SCAN_CANCEL,
					    TRUE,
					    FALSE,
					    FALSE,
					    NULL,
					    NULL,
					    sizeof(CMD_SCAN_CANCEL),
					    (PUINT_8) &rCmdScanCancel, NULL, 0);

			/*                                     */
			scnFsmGenerateScanDoneMsg(prAdapter,
						  prScanParam->ucSeqNum,
						  (UINT_8) prScanParam->eNetTypeIndex,
						  SCAN_STATUS_CANCELLED);

			/*                             */
			scnFsmSteps(prAdapter, SCAN_STATE_IDLE);
		} else {
			scnFsmRemovePendingMsg(prAdapter, prScanCancel->ucSeqNum,
					       prScanCancel->ucNetTypeIndex);
		}
	}

	cnmMemFree(prAdapter, prMsgHdr);

	return;
}


/*                                                                            */
/* 
                                                 
 
            
 
              
*/
/*                                                                            */
VOID scnFsmHandleScanMsg(IN P_ADAPTER_T prAdapter, IN P_MSG_SCN_SCAN_REQ prScanReqMsg)
{
	P_SCAN_INFO_T prScanInfo;
	P_SCAN_PARAM_T prScanParam;
	UINT_32 i;

	ASSERT(prAdapter);
	ASSERT(prScanReqMsg);

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &prScanInfo->rScanParam;

	prScanParam->eScanType = prScanReqMsg->eScanType;
	prScanParam->eNetTypeIndex = (ENUM_NETWORK_TYPE_INDEX_T) prScanReqMsg->ucNetTypeIndex;
	prScanParam->ucSSIDType = prScanReqMsg->ucSSIDType;
	if (prScanParam->ucSSIDType & (SCAN_REQ_SSID_SPECIFIED | SCAN_REQ_SSID_P2P_WILDCARD)) {
		prScanParam->ucSSIDNum = 1;

		COPY_SSID(prScanParam->aucSpecifiedSSID[0],
			  prScanParam->ucSpecifiedSSIDLen[0],
			  prScanReqMsg->aucSSID, prScanReqMsg->ucSSIDLength);

		/*                                                  */
		for (i = 1; i < SCN_SSID_MAX_NUM; i++) {
			prScanParam->ucSpecifiedSSIDLen[i] = 0;
		}
	} else {
		prScanParam->ucSSIDNum = 0;

		for (i = 0; i < SCN_SSID_MAX_NUM; i++) {
			prScanParam->ucSpecifiedSSIDLen[i] = 0;
		}
	}

	prScanParam->u2ProbeDelayTime = 0;
	prScanParam->eScanChannel = prScanReqMsg->eScanChannel;
	if (prScanParam->eScanChannel == SCAN_CHANNEL_SPECIFIED) {
		if (prScanReqMsg->ucChannelListNum <= MAXIMUM_OPERATION_CHANNEL_LIST) {
			prScanParam->ucChannelListNum = prScanReqMsg->ucChannelListNum;
		} else {
			prScanParam->ucChannelListNum = MAXIMUM_OPERATION_CHANNEL_LIST;
		}

		kalMemCopy(prScanParam->arChnlInfoList,
			   prScanReqMsg->arChnlInfoList,
			   sizeof(RF_CHANNEL_INFO_T) * prScanParam->ucChannelListNum);
	}

	if (prScanReqMsg->u2IELen <= MAX_IE_LENGTH) {
		prScanParam->u2IELen = prScanReqMsg->u2IELen;
	} else {
		prScanParam->u2IELen = MAX_IE_LENGTH;
	}

	if (prScanParam->u2IELen) {
		kalMemCopy(prScanParam->aucIE, prScanReqMsg->aucIE, prScanParam->u2IELen);
	}
#if CFG_ENABLE_WIFI_DIRECT
	if (prScanParam->eNetTypeIndex == NETWORK_TYPE_P2P_INDEX) {
		prScanParam->u2PassiveListenInterval = prScanReqMsg->u2ChannelDwellTime;
	}
#endif
	prScanParam->ucSeqNum = prScanReqMsg->ucSeqNum;

	if (prScanReqMsg->rMsgHdr.eMsgId == MID_RLM_SCN_SCAN_REQ) {
		prScanParam->fgIsObssScan = TRUE;
	} else {
		prScanParam->fgIsObssScan = FALSE;
	}

	prScanParam->fgIsScanV2 = FALSE;

	return;
}


/*                                                                            */
/* 
                                                                        
 
            
 
              
*/
/*                                                                            */
VOID scnFsmHandleScanMsgV2(IN P_ADAPTER_T prAdapter, IN P_MSG_SCN_SCAN_REQ_V2 prScanReqMsg)
{
	P_SCAN_INFO_T prScanInfo;
	P_SCAN_PARAM_T prScanParam;
	UINT_32 i;

	ASSERT(prAdapter);
	ASSERT(prScanReqMsg);
	ASSERT(prScanReqMsg->ucSSIDNum <= SCN_SSID_MAX_NUM);

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &prScanInfo->rScanParam;

	prScanParam->eScanType = prScanReqMsg->eScanType;
	prScanParam->eNetTypeIndex = (ENUM_NETWORK_TYPE_INDEX_T) prScanReqMsg->ucNetTypeIndex;
	prScanParam->ucSSIDType = prScanReqMsg->ucSSIDType;
	prScanParam->ucSSIDNum = prScanReqMsg->ucSSIDNum;

	for (i = 0; i < prScanReqMsg->ucSSIDNum; i++) {
		COPY_SSID(prScanParam->aucSpecifiedSSID[i],
			  prScanParam->ucSpecifiedSSIDLen[i],
			  prScanReqMsg->prSsid[i].aucSsid,
			  (UINT_8) prScanReqMsg->prSsid[i].u4SsidLen);
	}

	prScanParam->u2ProbeDelayTime = prScanReqMsg->u2ProbeDelay;
	prScanParam->eScanChannel = prScanReqMsg->eScanChannel;
	if (prScanParam->eScanChannel == SCAN_CHANNEL_SPECIFIED) {
		if (prScanReqMsg->ucChannelListNum <= MAXIMUM_OPERATION_CHANNEL_LIST) {
			prScanParam->ucChannelListNum = prScanReqMsg->ucChannelListNum;
		} else {
			prScanParam->ucChannelListNum = MAXIMUM_OPERATION_CHANNEL_LIST;
		}

		kalMemCopy(prScanParam->arChnlInfoList,
			   prScanReqMsg->arChnlInfoList,
			   sizeof(RF_CHANNEL_INFO_T) * prScanParam->ucChannelListNum);
	}

	if (prScanReqMsg->u2IELen <= MAX_IE_LENGTH) {
		prScanParam->u2IELen = prScanReqMsg->u2IELen;
	} else {
		prScanParam->u2IELen = MAX_IE_LENGTH;
	}

	if (prScanParam->u2IELen) {
		kalMemCopy(prScanParam->aucIE, prScanReqMsg->aucIE, prScanParam->u2IELen);
	}
#if CFG_ENABLE_WIFI_DIRECT
	if (prScanParam->eNetTypeIndex == NETWORK_TYPE_P2P_INDEX) {
		prScanParam->u2PassiveListenInterval = prScanReqMsg->u2ChannelDwellTime;
	}
#endif
	prScanParam->ucSeqNum = prScanReqMsg->ucSeqNum;

	if (prScanReqMsg->rMsgHdr.eMsgId == MID_RLM_SCN_SCAN_REQ) {
		prScanParam->fgIsObssScan = TRUE;
	} else {
		prScanParam->fgIsObssScan = FALSE;
	}

	prScanParam->fgIsScanV2 = TRUE;

	return;
}


/*                                                                            */
/* 
                                               
 
            
 
              
*/
/*                                                                            */
VOID scnFsmRemovePendingMsg(IN P_ADAPTER_T prAdapter, IN UINT_8 ucSeqNum, IN UINT_8 ucNetTypeIndex)
{
	P_SCAN_INFO_T prScanInfo;
	P_SCAN_PARAM_T prScanParam;
	P_MSG_HDR_T prPendingMsgHdr, prPendingMsgHdrNext, prRemoveMsgHdr = NULL;
	P_LINK_ENTRY_T prRemoveLinkEntry = NULL;

	ASSERT(prAdapter);

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &prScanInfo->rScanParam;

	/*                                              */
	LINK_FOR_EACH_ENTRY_SAFE(prPendingMsgHdr,
				 prPendingMsgHdrNext,
				 &(prScanInfo->rPendingMsgList), rLinkEntry, MSG_HDR_T) {
		if (prPendingMsgHdr->eMsgId == MID_AIS_SCN_SCAN_REQ
		    || prPendingMsgHdr->eMsgId == MID_BOW_SCN_SCAN_REQ
		    || prPendingMsgHdr->eMsgId == MID_P2P_SCN_SCAN_REQ
		    || prPendingMsgHdr->eMsgId == MID_RLM_SCN_SCAN_REQ) {
			P_MSG_SCN_SCAN_REQ prScanReqMsg = (P_MSG_SCN_SCAN_REQ) prPendingMsgHdr;

			if (ucSeqNum == prScanReqMsg->ucSeqNum &&
			    ucNetTypeIndex == prScanReqMsg->ucNetTypeIndex) {
				prRemoveLinkEntry = &(prScanReqMsg->rMsgHdr.rLinkEntry);
				prRemoveMsgHdr = prPendingMsgHdr;
			}
		} else if (prPendingMsgHdr->eMsgId == MID_AIS_SCN_SCAN_REQ_V2
			   || prPendingMsgHdr->eMsgId == MID_BOW_SCN_SCAN_REQ_V2
			   || prPendingMsgHdr->eMsgId == MID_P2P_SCN_SCAN_REQ_V2
			   || prPendingMsgHdr->eMsgId == MID_RLM_SCN_SCAN_REQ_V2) {
			P_MSG_SCN_SCAN_REQ_V2 prScanReqMsgV2 =
			    (P_MSG_SCN_SCAN_REQ_V2) prPendingMsgHdr;

			if (ucSeqNum == prScanReqMsgV2->ucSeqNum &&
			    ucNetTypeIndex == prScanReqMsgV2->ucNetTypeIndex) {
				prRemoveLinkEntry = &(prScanReqMsgV2->rMsgHdr.rLinkEntry);
				prRemoveMsgHdr = prPendingMsgHdr;
			}
		}

		if (prRemoveLinkEntry) {
			/*                                     */
			scnFsmGenerateScanDoneMsg(prAdapter,
						  ucSeqNum, ucNetTypeIndex, SCAN_STATUS_CANCELLED);

			/*                          */
			LINK_REMOVE_KNOWN_ENTRY(&(prScanInfo->rPendingMsgList), prRemoveLinkEntry);
			cnmMemFree(prAdapter, prRemoveMsgHdr);

			break;
		}
	}

	return;
}


/*                                                                            */
/* 
        
 
            
 
              
*/
/*                                                                            */
VOID scnEventScanDone(IN P_ADAPTER_T prAdapter, IN P_EVENT_SCAN_DONE prScanDone)
{
	P_SCAN_INFO_T prScanInfo;
	P_SCAN_PARAM_T prScanParam;

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &prScanInfo->rScanParam;

	/*                                  */
	if (prScanParam->eScanChannel == SCAN_CHANNEL_FULL
	    || prScanParam->eScanChannel == SCAN_CHANNEL_2G4) {
		if (prScanDone->ucSparseChannelValid) {
			prScanInfo->fgIsSparseChannelValid = TRUE;
			prScanInfo->rSparseChannel.eBand =
			    (ENUM_BAND_T) prScanDone->rSparseChannel.ucBand;
			prScanInfo->rSparseChannel.ucChannelNum =
			    prScanDone->rSparseChannel.ucChannelNum;
		} else {
			prScanInfo->fgIsSparseChannelValid = FALSE;
		}
	}

	if (prScanInfo->eCurrentState == SCAN_STATE_SCANNING &&
	    prScanDone->ucSeqNum == prScanParam->ucSeqNum) {
		/*                                     */
		scnFsmGenerateScanDoneMsg(prAdapter,
					  prScanParam->ucSeqNum,
					  (UINT_8) prScanParam->eNetTypeIndex, SCAN_STATUS_DONE);

		/*                             */
		scnFsmSteps(prAdapter, SCAN_STATE_IDLE);
	} else {
		DBGLOG(SCN, LOUD, ("Unexpected SCAN-DONE event: SeqNum = %d, Current State = %d\n",
				   prScanDone->ucSeqNum, prScanInfo->eCurrentState));
	}

	return;
}				/*                         */


/*                                                                            */
/* 
        
 
            
 
              
*/
/*                                                                            */
VOID
scnFsmGenerateScanDoneMsg(IN P_ADAPTER_T prAdapter,
			  IN UINT_8 ucSeqNum,
			  IN UINT_8 ucNetTypeIndex, IN ENUM_SCAN_STATUS eScanStatus)
{
	P_SCAN_INFO_T prScanInfo;
	P_SCAN_PARAM_T prScanParam;
	P_MSG_SCN_SCAN_DONE prScanDoneMsg;

	ASSERT(prAdapter);

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);
	prScanParam = &prScanInfo->rScanParam;

	prScanDoneMsg =
	    (P_MSG_SCN_SCAN_DONE) cnmMemAlloc(prAdapter, RAM_TYPE_MSG, sizeof(MSG_SCN_SCAN_DONE));
	if (!prScanDoneMsg) {
		ASSERT(0);	/*                                  */
		return;
	}

	if (prScanParam->fgIsObssScan == TRUE) {
		prScanDoneMsg->rMsgHdr.eMsgId = MID_SCN_RLM_SCAN_DONE;
	} else {
		switch ((ENUM_NETWORK_TYPE_INDEX_T) ucNetTypeIndex) {
		case NETWORK_TYPE_AIS_INDEX:
			prScanDoneMsg->rMsgHdr.eMsgId = MID_SCN_AIS_SCAN_DONE;
			break;

#if CFG_ENABLE_WIFI_DIRECT
		case NETWORK_TYPE_P2P_INDEX:
			prScanDoneMsg->rMsgHdr.eMsgId = MID_SCN_P2P_SCAN_DONE;
			break;
#endif

#if CFG_ENABLE_BT_OVER_WIFI
		case NETWORK_TYPE_BOW_INDEX:
			prScanDoneMsg->rMsgHdr.eMsgId = MID_SCN_BOW_SCAN_DONE;
			break;
#endif

		default:
			DBGLOG(SCN, LOUD, ("Unexpected Network Type: %d\n", ucNetTypeIndex));
			ASSERT(0);
			break;
		}
	}

	prScanDoneMsg->ucSeqNum = ucSeqNum;
	prScanDoneMsg->ucNetTypeIndex = ucNetTypeIndex;
	prScanDoneMsg->eScanStatus = eScanStatus;

	mboxSendMsg(prAdapter, MBOX_ID_0, (P_MSG_HDR_T) prScanDoneMsg, MSG_SEND_METHOD_BUF);

}				/*                                    */


/*                                                                            */
/* 
                                             
 
            
 
              
*/
/*                                                                            */
BOOLEAN
scnQuerySparseChannel(IN P_ADAPTER_T prAdapter,
		      P_ENUM_BAND_T prSparseBand, PUINT_8 pucSparseChannel)
{
	P_SCAN_INFO_T prScanInfo;

	ASSERT(prAdapter);

	prScanInfo = &(prAdapter->rWifiVar.rScanInfo);

	if (prScanInfo->fgIsSparseChannelValid == TRUE) {
		if (prSparseBand) {
			*prSparseBand = prScanInfo->rSparseChannel.eBand;
		}

		if (pucSparseChannel) {
			*pucSparseChannel = prScanInfo->rSparseChannel.ucChannelNum;
		}

		return TRUE;
	} else {
		return FALSE;
	}
}
