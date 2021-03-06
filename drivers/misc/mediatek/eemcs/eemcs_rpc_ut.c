/*                                                                              
            
            
                   
  
           
           
         
  
               
               
                                                
  
          
          
  
                                                                             
        
  
                       
                                                              
                    
  
                       
                                                           
                                      
                               
  
                       
                                                                        
                                   
  
                                                                            */

#include <linux/module.h>
#include <linux/limits.h>
#include <linux/nls.h>
#include <linux/wait.h>

#include "eemcs_kal.h"
#include "eemcs_ccci.h"
#include "eemcs_debug.h"
#include "eemcs_char.h"
#include "eemcs_file_ops.h"
#include "eemcs_rpc_ut.h"
#include "eemcs_rpc.h"

void dump_rpc_skb_data(void *data)
{
    pIPC_RPC_StreamBuffer_T stream = NULL;
  //                             

    stream = (pIPC_RPC_StreamBuffer_T)data;
   //                                            

    DBGLOG(RPCD, DBG, "[RPC][SKB] Stream Header = 0x%p", stream);
    //                                                           
    //                                                               
    //                                                               

//                                                                  
    //                                                                         
  //                                                              
    //                                                                             
}

inline KAL_INT32 eemcs_rpc_ut_UL_write_skb_to_swq(CCCI_CHANNEL_T chn, struct sk_buff *skb)
{
	pIPC_RPC_StreamBuffer_T temp_rpc_buffer;
	kal_uint8* ptr;
    KAL_UINT32 tx_queue_idx;

	DBGLOG(RPCD, DBG, "[RPCUT] enter rpcdut_UL_write_skb_to_swq");

    //                        
    
	temp_rpc_buffer = (pIPC_RPC_StreamBuffer_T)skb->data;
    if(NULL != skb){
        
		DBGLOG(RPCD, DBG, "[RPCUT] rpc_buf: ccci header: data[0]=0x%08X, data[1]=0x%08X, channel=0x%08X, reserved=0x%08X", 
				temp_rpc_buffer->ccci_header.data[0],temp_rpc_buffer->ccci_header.data[1] ,temp_rpc_buffer->ccci_header.channel,temp_rpc_buffer->ccci_header.reserved);
		DBGLOG(RPCD, DBG, "[RPCUT] rpc_buf: opid=0x%08X, num_para=0x%08X", 
				temp_rpc_buffer->rpc_opid, temp_rpc_buffer->num_para);
		KAL_ASSERT(temp_rpc_buffer->ccci_header.channel == chn); 
        KAL_ASSERT(temp_rpc_buffer->ccci_header.channel < CH_NUM_MAX); 
    }else{
        DBGLOG(RPCD,DBG, "[RPCUT] rpcdut_UL_write_skb_to_swq write NULL skb to kick DF process!!");
    }
	
	KAL_ASSERT(temp_rpc_buffer->ccci_header.reserved == md_gpd_idx_ut_val);
	KAL_ASSERT(temp_rpc_buffer->ccci_header.data[1] == sizeof(CCCI_BUFF_T) + 2*sizeof(kal_uint32) + 4*sizeof(kal_uint32)); //      
	KAL_ASSERT(temp_rpc_buffer->rpc_opid == (IPC_RPC_API_RESP_ID |IPC_RPC_UT_OP));
    KAL_ASSERT(temp_rpc_buffer->num_para == num_para_from_ap_ut_val);
	
	//                 
	ptr = (kal_uint8*)(temp_rpc_buffer) + sizeof(CCCI_BUFF_T) +2*sizeof(kal_uint32);
	//                                
	KAL_ASSERT(*ptr == sizeof(kal_uint32));
	DBGLOG(RPCD, DBG, "[RPCUT] LV[0]  len = 0x%08X", *((kal_uint32 *)ptr));
	ptr += sizeof(kal_uint32);
	KAL_ASSERT(*((kal_uint32 *)ptr) == RPC_UT_SUCCESS);  //                                  
	DBGLOG(RPCD, DBG, "[RPCUT] LV[0]  data= 0x%08X", *((kal_uint32 *)ptr));
	//                
	ptr += sizeof(kal_uint32);
	//           
	KAL_ASSERT(*((kal_uint32 *)ptr) == sizeof(kal_uint32));
	DBGLOG(RPCD, DBG, "[RPCUT] LV[1]  len = 0x%08X", *((kal_uint32 *)ptr));
	ptr += sizeof(kal_uint32);
	//                              
	KAL_ASSERT(*((kal_uint32 *)ptr) == ut_ret_val);
	DBGLOG(RPCD, DBG, "[RPCUT] LV[1]  data= 0x%08X", *((kal_uint32 *)ptr));

//               
    tx_queue_idx = (ccci_get_port_info(ccci_ch_to_port(chn)))->txq_id;
	DBGLOG(RPCD, DBG, "[RPCUT] tx_queue_idx = 0x%08X", tx_queue_idx);

	DBGLOG(RPCD, DBG, "[RPCUT] leave rpcdut_UL_write_skb_to_swq !!");
	return KAL_SUCCESS;

}


/*
                                  
         
           
          
           
 */
void eemcs_rpc_ut_trigger(void)
{
	struct sk_buff *new_skb = NULL;
	pIPC_RPC_StreamBuffer_T temp_rpc_buffer;
	kal_uint8* ptr;
	//             
	DBGLOG(RPCD, DBG, "[RPCUT] enter eemcs_rpc_ut_trigger");
	//                                            

    //                        

	new_skb = dev_alloc_skb(IPC_RPC_MAX_BUF_SIZE);
	if (new_skb == NULL) {
		DBGLOG(RPCD, ERR, "[RPCUT] Failed to alloc skb !!");
		goto _fail;
	}
	temp_rpc_buffer = (pIPC_RPC_StreamBuffer_T)skb_put(new_skb, IPC_RPC_MAX_BUF_SIZE);
	memset(new_skb->data, 0, IPC_RPC_MAX_BUF_SIZE);
	//                                                                                
	//                                       

	//                    
	temp_rpc_buffer->ccci_header.reserved = md_gpd_idx_ut_val;
	temp_rpc_buffer->ccci_header.channel = CH_RPC_RX;
	temp_rpc_buffer->ccci_header.data[1] = sizeof(CCCI_BUFF_T) +6*sizeof(kal_uint32); //      
	//                                                   
	temp_rpc_buffer->rpc_opid = IPC_RPC_UT_OP;
	temp_rpc_buffer->num_para = num_para_from_md_ut_val;

	DBGLOG(RPCD, DBG, "[RPCUT] rpc_buf: ccci header: data[0]=0x%08X, data[1]=0x%08X, channel=0x%08X, reserved=0x%08X", 
		temp_rpc_buffer->ccci_header.data[0],temp_rpc_buffer->ccci_header.data[1] ,temp_rpc_buffer->ccci_header.channel,temp_rpc_buffer->ccci_header.reserved);
	DBGLOG(RPCD, DBG, "[RPCUT] rpc_buf: opid=0x%08X, num_para=0x%08X", 
		temp_rpc_buffer->rpc_opid, temp_rpc_buffer->num_para);
#if 0

	
	//                 
	pkt_info = (kal_uint32)(temp_rpc_buffer) + sizeof(CCCI_BUFF_T) +2*sizeof(kal_uint32);
	//           
	pkt_info[0]->len= sizeof(kal_uint32);
	pkt_info[0]->buf= 0xbeef;
	//           
	pkt_info[1]->len= sizeof(kal_uint32);
	pkt_info[1]->buf= 0xbeef;
	for(i = 0; i < 2; i++)
	{
		DBGLOG(RPCD, DBG, "[RPC] LV[%d]  len= 0x%08X, value= 0x%08X", i, pkt_info[i].len, pkt_info[i].buf);
	}


#endif

	//                 
	ptr = (kal_uint8*)(temp_rpc_buffer) + sizeof(CCCI_BUFF_T) +2*sizeof(kal_uint32);
	//           
	*((kal_uint32 *)ptr) = sizeof(kal_uint32);
	DBGLOG(RPCD, DBG, "[RPCUT] LV[0]  len= 0x%08X", *((kal_uint32 *)ptr));
	ptr += sizeof(kal_uint32);
	//                              
	*((kal_uint32 *)ptr) = 0xbeef;
	DBGLOG(RPCD, DBG, "[RPCUT] LV[0]  value= 0x%08X", *((kal_uint32 *)ptr));
	//                
	ptr += sizeof(kal_uint32);
	//           
	*((kal_uint32 *)ptr) = sizeof(kal_uint32);
	DBGLOG(RPCD, DBG, "[RPCUT] LV[1]  len= 0x%08X", *((kal_uint32 *)ptr));
	ptr += sizeof(kal_uint32);
	*((kal_uint32 *)ptr) = 0xbeef;
	DBGLOG(RPCD, DBG, "[RPCUT] LV[1]  value= 0x%08X", *((kal_uint32 *)ptr));
	

    //                             
    
    eemcs_rpc_callback(new_skb, 0);
	
	DBGLOG(RPCD, DBG, "[RPCUT] leave eemcs_rpc_ut_trigger");

    return ;
	
_fail:
	KAL_ASSERT(0);
}
