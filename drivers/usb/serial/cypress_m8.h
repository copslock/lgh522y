#ifndef CYPRESS_M8_H
#define CYPRESS_M8_H

/*
                                                                         
             
 */

/*
                                                                            
                                                                               
 */
#define HID_REQ_GET_REPORT	0x01
#define HID_REQ_SET_REPORT	0x09

/*                                                                             */

/*                                      */
#define VENDOR_ID_DELORME		0x1163
#define PRODUCT_ID_EARTHMATEUSB		0x0100
#define PRODUCT_ID_EARTHMATEUSB_LT20	0x0200

/*                                */
#define VENDOR_ID_CYPRESS		0x04b4
#define PRODUCT_ID_CYPHIDCOM		0x5500

/*                                  */
#define VENDOR_ID_FRWD			0x6737
#define PRODUCT_ID_CYPHIDCOM_FRWD	0x0001

/*                              */
#define VENDOR_ID_POWERCOM		0x0d9f
#define PRODUCT_ID_UPS			0x0002

/*                                 */
#define VENDOR_ID_DAZZLE		0x07d0
#define PRODUCT_ID_CA42			0x4101
/*                       */

/*                                                    */
#define CYPRESS_SET_CONFIG	0x01
#define CYPRESS_GET_CONFIG	0x02

/*                           */
#define THROTTLED		0x1
#define ACTUALLY_THROTTLED	0x2

/*
                                                                               
                             
 */
#define CT_EARTHMATE	0x01
#define CT_CYPHIDCOM	0x02
#define CT_CA42V2	0x03
#define CT_GENERIC	0x0F
/*                             */

/*                                                       */
/*                                                                 */
/*                                                                            */

#define CONTROL_DTR	0x20	/*                                                     */
#define UART_DSR	0x20	/*                                                */
#define CONTROL_RTS	0x10	/*                                                 */
#define UART_CTS	0x10	/*                                               */
#define UART_RI		0x80	/*                                         */
#define UART_CD		0x40	/*                                         */
#define CYP_ERROR	0x08	/*                                             */
/*                                                                    */
#define CONTROL_RESET	0x08	/*                                          */

/*                                    */

#endif /*              */
