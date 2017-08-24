/*
                                                                                          
*/

/*                        
                                                    

       
*/



/*
                       
  
                      
                                                                                            
                                                                
  
                   
  
                                                                               
  
                         
                                                
                                   
  
                   
                                                          
                                            
                                                                                                             
                                                                            
                           
                                                                            
                                  
                                                                            
                           
                                                                            
                        
                                                                            
                                                                            
                                         
                                                                           
                            
                                                                           
                              
                                                                           
                                                       
                                                                           
                                                                            
                                                                           
                  
                                                                           
                                
                                                                           
                                                 
                                                                           
                    
  
*/

#ifndef _MT6620_REG_H
#define _MT6620_REG_H

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

/*                                                                              
                                                   
                                                                                
*/

/*                                                                              
                                             
                                                                                
*/

/*                                                                              
                                                             
                                                                                
*/

/*                                                                              
                                                
                                                                                
*/

/*                         */

/*                  */

/*                    */
#define MCR_WCIR                            0x0000

/*                                   */
#define MCR_WHLPCR                          0x0004
/*                                                    */


/*                            */
#define MCR_WSDIOCSR                        0x0008
#define MCR_WSPICSR                         0x0008

/*                        */
#define MCR_WHCR                            0x000C

/*                                  */
#define MCR_WHISR                           0x0010

/*                                  */
#define MCR_WHIER                           0x0014

/*                            */
#define MCR_WASR                            0x0018

/*                                            */
#define MCR_WSICR                           0x001C

/*                           */
#define MCR_WTSR0                           0x0020

/*                           */
#define MCR_WTSR1                           0x0024

/*                           */
#define MCR_WTDR0                           0x0028

/*                           */
#define MCR_WTDR1                           0x002C

/*                           */
#define MCR_WRDR0                           0x0030

/*                           */
#define MCR_WRDR1                           0x0034

/*                                          */
#define MCR_H2DSM0R                         0x0038

/*                                          */
#define MCR_H2DSM1R                         0x003c

/*                                             */
#define MCR_D2HRM0R                         0x0040

/*                                             */
#define MCR_D2HRM1R                         0x0044

/*                                  */
#define MCR_WRPLR                           0x0048




/*                                  */
typedef struct _ENHANCE_MODE_DATA_STRUCT_T {
	UINT_32 u4WHISR;
	union {
		struct {
			UINT_8 ucTQ0Cnt;
			UINT_8 ucTQ1Cnt;
			UINT_8 ucTQ2Cnt;
			UINT_8 ucTQ3Cnt;
			UINT_8 ucTQ4Cnt;
			UINT_8 ucTQ5Cnt;
			UINT_16 u2Rsrv;
		} u;
		UINT_32 au4WTSR[2];
	} rTxInfo;
	union {
		struct {
			UINT_16 u2NumValidRx0Len;
			UINT_16 u2NumValidRx1Len;
			UINT_16 au2Rx0Len[16];
			UINT_16 au2Rx1Len[16];
		} u;
		UINT_32 au4RxStatusRaw[17];
	} rRxInfo;
	UINT_32 u4RcvMailbox0;
	UINT_32 u4RcvMailbox1;
} ENHANCE_MODE_DATA_STRUCT_T, *P_ENHANCE_MODE_DATA_STRUCT_T;
/*                                      */ */


/*                               */
/*               */
#define WCIR_WLAN_READY                  BIT(21)
#define WCIR_POR_INDICATOR               BIT(20)
#define WCIR_REVISION_ID                 BITS(16, 19)
#define WCIR_CHIP_ID                     BITS(0, 15)

#define MTK_CHIP_REV                     0x00006620
#define MTK_CHIP_MP_REVERSION_ID         0x0

/*                 */
#define WHLPCR_FW_OWN_REQ_CLR            BIT(9)
#define WHLPCR_FW_OWN_REQ_SET            BIT(8)
#define WHLPCR_IS_DRIVER_OWN             BIT(8)
#define WHLPCR_INT_EN_CLR                BIT(1)
#define WHLPCR_INT_EN_SET                BIT(0)

/*                   */
#define WSDIOCSR_SDIO_RE_INIT_EN         BIT(0)

/*                  */
#define WCSR_SPI_MODE_SEL                BITS(3, 4)
#define WCSR_SPI_ENDIAN_BIG              BIT(2)
#define WCSR_SPI_INT_OUT_MODE            BIT(1)
#define WCSR_SPI_DATA_OUT_MODE           BIT(0)

/*               */
#define WHCR_RX_ENHANCE_MODE_EN         BIT(16)
#define WHCR_MAX_HIF_RX_LEN_NUM         BITS(4, 7)
#define WHCR_W_MAILBOX_RD_CLR_EN        BIT(2)
#define WHCR_W_INT_CLR_CTRL             BIT(1)
#define WHCR_MCU_DBG_EN                 BIT(0)
#define WHCR_OFFSET_MAX_HIF_RX_LEN_NUM  4

/*                */
#define WHISR_D2H_SW_INT                BITS(8, 31)
#define WHISR_D2H_SW_ASSERT_INFO_INT    BIT(31)
#define WHISR_FW_INT_INDICATOR          BIT(7)
#define WHISR_FW_OWN_BACK_INT           BIT(4)
#define WHISR_ABNORMAL_INT              BIT(3)
#define WHISR_RX1_DONE_INT              BIT(2)
#define WHISR_RX0_DONE_INT              BIT(1)
#define WHISR_TX_DONE_INT               BIT(0)


/*                */
#define WHIER_D2H_SW_INT                BITS(8, 31)
#define WHIER_FW_INT_INDICATOR_EN       BIT(7)
#define WHIER_FW_OWN_BACK_INT_EN        BIT(4)
#define WHIER_ABNORMAL_INT_EN           BIT(3)
#define WHIER_RX1_DONE_INT_EN           BIT(2)
#define WHIER_RX0_DONE_INT_EN           BIT(1)
#define WHIER_TX_DONE_INT_EN            BIT(0)
#define WHIER_DEFAULT                   (WHIER_RX0_DONE_INT_EN    | \
					 WHIER_RX1_DONE_INT_EN    | \
					 WHIER_TX_DONE_INT_EN     | \
					 WHIER_ABNORMAL_INT_EN    | \
					 WHIER_D2H_SW_INT           \
					 )


/*               */
#define WASR_FW_OWN_INVALID_ACCESS      BIT(4)
#define WASR_RX1_UNDER_FLOW             BIT(3)
#define WASR_RX0_UNDER_FLOW             BIT(2)
#define WASR_TX1_OVER_FLOW              BIT(1)
#define WASR_TX0_OVER_FLOW              BIT(0)


/*                */
#define WSICR_H2D_SW_INT_SET            BITS(16, 31)


/*                */
#define WRPLR_RX1_PACKET_LENGTH         BITS(16, 31)
#define WRPLR_RX0_PACKET_LENGTH         BITS(0, 15)

#endif				/*               */
