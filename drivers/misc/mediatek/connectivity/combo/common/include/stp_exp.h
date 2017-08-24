/*       
                                            

                                                                                           
*/



#ifndef _STP_EXP_H_
#define _STP_EXP_H_

#include "wmt_stp_exp.h"
#include "osal.h"
/*                                                                              
                                                     
                                                                                
*/

/*                                                                              
                                             
                                                                                
*/

#ifndef MTK_WCN_WMT_STP_EXP_SYMBOL_ABSTRACT

#define BT_TASK_INDX        (0)
#define FM_TASK_INDX        (1)
#define GPS_TASK_INDX       (2)
#define WIFI_TASK_INDX      (3)
#define WMT_TASK_INDX       (4)
#define STP_TASK_INDX       (5)
#define INFO_TASK_INDX      (6)
#define ANT_TASK_INDX       (7)
#if CFG_WMT_LTE_COEX_HANDLING
#define COEX_TASK_INDX		(8)
#define MTKSTP_MAX_TASK_NUM (9)
#else
#define MTKSTP_MAX_TASK_NUM	(8)
#endif

#define MTKSTP_BUFFER_SIZE  (16384)	/*                  */

#define STP_EXP_HID_API_EXPORT 0

#else
#define STP_EXP_HID_API_EXPORT 1


#endif

/*                                                                              
                                                          
                                                                                
*/



/*                                                                              
                                                
                                                                                
*/



/*                                                                              
                                                 
                                                                                
*/
#ifndef MTK_WCN_WMT_STP_EXP_SYMBOL_ABSTRACT

typedef VOID (*MTK_WCN_STP_EVENT_CB) (VOID);
typedef INT32(*MTK_WCN_STP_IF_TX) (const PUINT8 data, const UINT32 size, PUINT32 written_size);
/*                       */
typedef VOID (*MTK_WCN_STP_IF_RX) (const PUINT8 data, INT32 size);

typedef enum {
	STP_UART_IF_TX = 0,
	STP_SDIO_IF_TX,
	STP_MAX_IF_TX
} ENUM_STP_TX_IF_TYPE;



/*                                                                              
                                                  
                                                                                
*/

/*                                                                              
                                                   
                                                                                
*/


/*                                                                              
                                                            
                                                                                
*/

/*                                                                            
          
                           
             
                                           
            
                                      
                                             
         
                                                   
                                                                            */
extern INT32 mtk_wcn_stp_receive_data(PUINT8 buffer, UINT32 length, UINT8 type);

/*                                                                            
          
                        
             
                                    
            
                                      
                                             
                                           
         
                                                
                                                                            */
extern INT32 mtk_wcn_stp_send_data(const PUINT8 buffer, const UINT32 length, const UINT8 type);

/*                                                                            
          
                               
             
                             
            
                                           
         
                                                     
                                                                            */
extern MTK_WCN_BOOL mtk_wcn_stp_is_rxqueue_empty(UINT8 type);

/*                                                                            
          
                        
             
                
            
        
         
                                              
                                                                            */
extern MTK_WCN_BOOL mtk_wcn_stp_is_ready(VOID);

/*                                                                            
          
                          
             
                                                       
            
                                      
                                             
         
       
                                                                            */
extern INT32 mtk_wcn_stp_parser_data(PUINT8 buffer, UINT32 length);

/*                                                                            
          
                             
             
                             
            
                    
         
       
                                                                            */
extern void mtk_wcn_stp_set_bluez(MTK_WCN_BOOL sdio_flag);

/*                                                                            
          
                                   
             
                                     
            
       
         
                                 
                                                                            */
extern INT32 mtk_wcn_stp_register_tx_event_cb(INT32 type, MTK_WCN_STP_EVENT_CB func);

/*                                                                            
          
                                
             
                                     
            
       
         
                                 
                                                                            */
extern INT32 mtk_wcn_stp_register_event_cb(INT32 type, MTK_WCN_STP_EVENT_CB func);


/*                                                                            
          
                             
             
                                     
            
                                                 
         
                                 
                                                                            */
extern INT32 mtk_wcn_stp_register_if_tx(ENUM_STP_TX_IF_TYPE stp_if, MTK_WCN_STP_IF_TX func);


/*                                                                            
          
                             
             
                                     
            
                                                 
         
                                 
                                                                            */
extern INT32 mtk_wcn_stp_register_if_rx(MTK_WCN_STP_IF_RX func);

/*                                                                            
          
                                 
             
                                     
            
       
         
                                                                        
                                                                            */
extern INT32 mtk_wcn_stp_coredump_start_get(VOID);


/*                                                                              
                                                
                                                                                
*/
#else
extern INT32 _mtk_wcn_stp_receive_data(PUINT8 buffer, UINT32 length, UINT8 type);
extern INT32 _mtk_wcn_stp_send_data_raw(const PUINT8 buffer, const UINT32 length,
					const UINT8 type);
extern INT32 _mtk_wcn_stp_send_data(const PUINT8 buffer, const UINT32 length, const UINT8 type);
extern MTK_WCN_BOOL _mtk_wcn_stp_is_rxqueue_empty(UINT8 type);
extern MTK_WCN_BOOL _mtk_wcn_stp_is_ready(VOID);
extern INT32 _mtk_wcn_stp_parser_data(PUINT8 buffer, UINT32 length);
extern void _mtk_wcn_stp_set_bluez(MTK_WCN_BOOL sdio_flag);
extern INT32 _mtk_wcn_stp_register_tx_event_cb(INT32 type, MTK_WCN_STP_EVENT_CB func);
extern INT32 _mtk_wcn_stp_register_event_cb(INT32 type, MTK_WCN_STP_EVENT_CB func);
extern INT32 _mtk_wcn_stp_register_if_tx(ENUM_STP_TX_IF_TYPE stp_if, MTK_WCN_STP_IF_TX func);
extern INT32 _mtk_wcn_stp_register_if_rx(MTK_WCN_STP_IF_RX func);
extern INT32 _mtk_wcn_stp_coredump_start_get(VOID);

#endif


#endif				/*             */
