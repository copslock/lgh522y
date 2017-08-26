/*
                                                                  
                                                              
               
 */

#ifndef __LINUX_USB_QUIRKS_H
#define __LINUX_USB_QUIRKS_H

/*                                                              */
#define USB_QUIRK_STRING_FETCH_255	0x00000001

/*                                                   */
#define USB_QUIRK_RESET_RESUME		0x00000002

/*                                            */
#define USB_QUIRK_NO_SET_INTF		0x00000004

/*                                                            */
#define USB_QUIRK_CONFIG_INTF_STRINGS	0x00000008

/*                                                           */
#define USB_QUIRK_RESET			0x00000010

/*                                                                      
                                                */
#define USB_QUIRK_HONOR_BNUMINTERFACES	0x00000020

/*                                                                     
              */
#define USB_QUIRK_DELAY_INIT		0x00000040

/* device generates spurious wakeup, ignore remote wakeup capability */
#define USB_QUIRK_IGNORE_REMOTE_WAKEUP	0x00000200

#endif /* __LINUX_USB_QUIRKS_H */
