/* Driver for USB Mass Storage compliant devices
 * Unusual Devices File
 *
 * Current development and maintenance by:
 *   (c) 2000-2002 Matthew Dharm (mdharm-usb@one-eyed-alien.net)
 *
 * Initial work by:
 *   (c) 2000 Adam J. Richter (adam@yggdrasil.com), Yggdrasil Computing, Inc.
 *
 * Please see http://www.one-eyed-alien.net/~mdharm/linux-usb for more
 * information about this driver.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*                                                                      
                                      
                                                                       
                                
 */

/*                                                                       
                     
  
                                                                    
                         
                                                                
                                                            
                                              
                                                                
                             
                                                                  
                                                                      
                                                                            
                                         
 */

/*                                                                     
                                                               
                                                                   
                                                                
 */

/*                                                                   
                                                                  
                                                    
  
                                                                     
                                                           
 */

#if !defined(CONFIG_USB_STORAGE_SDDR09) && \
		!defined(CONFIG_USB_STORAGE_SDDR09_MODULE)
#define NO_SDDR09
#endif

/*                                                           
 */
UNUSUAL_DEV(  0x03eb, 0x2002, 0x0100, 0x0100,
		"ATMEL",
		"SND1 Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE),

/*                                                 */
UNUSUAL_DEV(  0x03ee, 0x6906, 0x0003, 0x0003,
		"VIA Technologies Inc.",
		"Mitsumi multi cardreader",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE ),

UNUSUAL_DEV(  0x03f0, 0x0107, 0x0200, 0x0200,
		"HP",
		"CD-Writer+",
		USB_SC_8070, USB_PR_CB, NULL, 0),

/*                                           */
UNUSUAL_DEV(  0x03f0, 0x070c, 0x0000, 0x0000,
		"HP",
		"Personal Media Drive",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_SANE_SENSE ),

/*                                                       
                                                                  
 */
UNUSUAL_DEV(  0x03f0, 0x4002, 0x0001, 0x0001,
		"HP",
		"PhotoSmart R707",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL, US_FL_FIX_CAPACITY),

/*                                                        
                                                                       
                                                         
 */
UNUSUAL_DEV(  0x0409, 0x0040, 0x0000, 0x9999,
		"NEC",
		"NEC USB UF000x",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_SINGLE_LUN ),

/*                                                           */
UNUSUAL_DEV(  0x040d, 0x6205, 0x0003, 0x0003,
		"VIA Technologies Inc.",
		"USB 2.0 Card Reader",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE ),

/*                                                
                                                                            
                                   
 */
UNUSUAL_DEV(  0x0411, 0x001c, 0x0113, 0x0113,
		"Buffalo",
		"DUB-P40G HDD",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_INQUIRY ),

/*                                                         */
UNUSUAL_DEV(  0x0419, 0x0100, 0x0100, 0x0100,
		"Samsung Info. Systems America, Inc.",
		"MP3 Player",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE ),

/*                                             */
UNUSUAL_DEV(  0x0419, 0xaace, 0x0100, 0x0100,
		"Samsung", "MP3 Player",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE ),

/*                                                  */
UNUSUAL_DEV(  0x0419, 0xaaf5, 0x0100, 0x0100,
		"TrekStor",
		"i.Beat 115 2.0",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE | US_FL_NOT_LOCKABLE ),

/*                                            */
UNUSUAL_DEV(  0x0419, 0xaaf6, 0x0100, 0x0100,
		"TrekStor",
		"i.Beat Joy 2.0",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE ),

/*                                                          */
UNUSUAL_DEV(  0x0420, 0x0001, 0x0100, 0x0100,
		"GENERIC", "MP3 PLAYER", /*                                */
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE ),

/*                                         
                                                                        */
UNUSUAL_DEV(  0x0421, 0x0019, 0x0592, 0x0610,
		"Nokia",
		"Nokia 6288",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_MAX_SECTORS_64 ),

/*                                               */
UNUSUAL_DEV(  0x0421, 0x042e, 0x0100, 0x0100,
		"Nokia",
		"Nokia 3250",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE | US_FL_FIX_CAPACITY ),

/*                                 */
UNUSUAL_DEV(  0x0421, 0x0433, 0x0100, 0x0100,
		"Nokia",
		"E70",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE | US_FL_FIX_CAPACITY ),

/*                                        */
UNUSUAL_DEV(  0x0421, 0x0434, 0x0100, 0x0100,
		"Nokia",
		"E60",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY | US_FL_IGNORE_RESIDUE ),

/*                                                       
                                               */
UNUSUAL_DEV(  0x0421, 0x0444, 0x0100, 0x0100,
		"Nokia",
		"N91",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE | US_FL_FIX_CAPACITY ),

/*                                                 
                                        */
UNUSUAL_DEV(  0x0421, 0x0446, 0x0100, 0x0100,
		"Nokia",
		"N80",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE | US_FL_FIX_CAPACITY ),

/*                                                    */
UNUSUAL_DEV(  0x0421, 0x044e, 0x0100, 0x0100,
		"Nokia",
		"E61",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE | US_FL_FIX_CAPACITY ),

/*                                                       */
UNUSUAL_DEV(  0x0421, 0x047c, 0x0370, 0x0610,
		"Nokia",
		"6131",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_MAX_SECTORS_64 ),

/*                                                         */
UNUSUAL_DEV( 0x0421, 0x0492, 0x0452, 0x9999,
		"Nokia",
		"Nokia 6233",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_MAX_SECTORS_64 ),

/*                                               */
UNUSUAL_DEV(  0x0421, 0x0495, 0x0370, 0x0370,
		"Nokia",
		"6234",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_MAX_SECTORS_64 ),

/*                                              */
UNUSUAL_DEV(  0x0421, 0x04b9, 0x0350, 0x0350,
		"Nokia",
		"5300",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_MAX_SECTORS_64 ),

/*                                                                  */
UNUSUAL_DEV(  0x0421, 0x05af, 0x0742, 0x0742,
		"Nokia",
		"305",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_MAX_SECTORS_64),

/*                                                            */
UNUSUAL_DEV(  0x0421, 0x06aa, 0x1110, 0x1110,
		"Nokia",
		"502",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_MAX_SECTORS_64 ),

#ifdef NO_SDDR09
UNUSUAL_DEV(  0x0436, 0x0005, 0x0100, 0x0100,
		"Microtech",
		"CameraMate",
		USB_SC_SCSI, USB_PR_CB, NULL,
		US_FL_SINGLE_LUN ),
#endif

/*                                                 
                                                                         */
UNUSUAL_DEV(  0x0451, 0x5416, 0x0100, 0x0100,
		"Neuros Audio",
		"USB 2.0 HD 2.5",
		USB_SC_DEVICE, USB_PR_BULK, NULL,
		US_FL_NEED_OVERRIDE ),

/*
                                                                          
                                                                   
                                                    
 */
UNUSUAL_DEV(  0x0457, 0x0150, 0x0100, 0x0100,
		"USBest Technology",	/*                   */
		"USB Mass Storage Device",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL, US_FL_NOT_LOCKABLE ),

/*
                                       
                                                      
                               
*/
UNUSUAL_DEV(  0x0457, 0x0151, 0x0100, 0x0100,
		"USB 2.0",
		"Flash Disk",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_NOT_LOCKABLE ),

/*                                                  
                                                         
                                                 
                                                                
                                                                
 */
UNUSUAL_DEV(  0x045e, 0xffff, 0x0000, 0x0000,
		"Mitac",
		"GPS",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_MAX_SECTORS_64 ),

/*
                                                                       
                                                
                                                                     
                                                                 
 */
UNUSUAL_DEV(  0x046b, 0xff40, 0x0100, 0x0100,
		"AMI",
		"Virtual Floppy",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_NO_WP_DETECT),

/*                                                        */
UNUSUAL_DEV(  0x0482, 0x0100, 0x0100, 0x0100,
		"Kyocera",
		"Finecam S3x",
		USB_SC_8070, USB_PR_CB, NULL, US_FL_FIX_INQUIRY),

/*                                                        */
UNUSUAL_DEV(  0x0482, 0x0101, 0x0100, 0x0100,
		"Kyocera",
		"Finecam S4",
		USB_SC_8070, USB_PR_CB, NULL, US_FL_FIX_INQUIRY),

/*                                                              */
UNUSUAL_DEV(  0x0482, 0x0103, 0x0100, 0x0100,
		"Kyocera",
		"Finecam S5",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL, US_FL_FIX_INQUIRY),

/*                                                               */
UNUSUAL_DEV(  0x0482, 0x0107, 0x0100, 0x0100,
		"Kyocera",
		"CONTAX SL300R T*",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY | US_FL_NOT_LOCKABLE),

/*                                                
                                                          */
UNUSUAL_DEV(  0x04a4, 0x0004, 0x0001, 0x0001,
		"Hitachi",
		"DVD-CAM DZ-MV100A Camcorder",
		USB_SC_SCSI, USB_PR_CB, NULL, US_FL_SINGLE_LUN),

/*            
                                                     
                                            */
UNUSUAL_DEV(  0x04a5, 0x3010, 0x0100, 0x0100,
		"Tekom Technologies, Inc",
		"300_CAMERA",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE ),

/*                             
                                                      */
UNUSUAL_DEV(  0x04b0, 0x0301, 0x0010, 0x0010,
		"NIKON",
		"NIKON DSC E2000",
		USB_SC_DEVICE, USB_PR_DEVICE,NULL,
		US_FL_NOT_LOCKABLE ),

/*                                             */
UNUSUAL_DEV(  0x04b3, 0x4001, 0x0110, 0x0110,
		"IBM",
		"IBM RSA2",
		USB_SC_DEVICE, USB_PR_CB, NULL,
		US_FL_MAX_SECTORS_MIN),

/*                                            
                                         */
UNUSUAL_DEV(  0x04b8, 0x0601, 0x0100, 0x0100,
		"Epson",
		"875DC Storage",
		USB_SC_SCSI, USB_PR_CB, NULL, US_FL_FIX_INQUIRY),

/*                                                
                                                          */
UNUSUAL_DEV(  0x04b8, 0x0602, 0x0110, 0x0110,
		"Epson",
		"785EPX Storage",
		USB_SC_SCSI, USB_PR_BULK, NULL, US_FL_SINGLE_LUN),

/*                                          
                                                                       
                 */
UNUSUAL_DEV(  0x04cb, 0x0100, 0x0000, 0x2210,
		"Fujifilm",
		"FinePix 1400Zoom",
		USB_SC_UFI, USB_PR_DEVICE, NULL, US_FL_FIX_INQUIRY | US_FL_SINGLE_LUN),

/*                                                     
                                                                             
 */
UNUSUAL_DEV(  0x04ce, 0x0002, 0x026c, 0x026c,
		"ScanLogic",
		"SL11R-IDE",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY),

/*                                                  
                                                        
                                                     
                                                          
 */
UNUSUAL_DEV(  0x04da, 0x0901, 0x0100, 0x0200,
		"Panasonic",
		"LS-120 Camera",
		USB_SC_UFI, USB_PR_DEVICE, NULL, 0),

/*                                            
                                        */
UNUSUAL_DEV(  0x04da, 0x0d05, 0x0000, 0x0000,
		"Sharp CE-CW05",
		"CD-R/RW Drive",
		USB_SC_8070, USB_PR_CB, NULL, 0),

/*                                                  */
UNUSUAL_DEV(  0x04da, 0x2372, 0x0000, 0x9999,
		"Panasonic",
		"DMC-LCx Camera",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY | US_FL_NOT_LOCKABLE ),

/*                                                       */
UNUSUAL_DEV(  0x04da, 0x2373, 0x0000, 0x9999,
		"LEICA",
		"D-LUX Camera",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY | US_FL_NOT_LOCKABLE ),

/*                                                              
                        
 */
UNUSUAL_DEV(  0x04e6, 0x0001, 0x0200, 0x0200,
		"Matshita",
		"LS-120",
		USB_SC_8020, USB_PR_CB, NULL, 0),

UNUSUAL_DEV(  0x04e6, 0x0002, 0x0100, 0x0100,
		"Shuttle",
		"eUSCSI Bridge",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_euscsi_init,
		US_FL_SCM_MULT_TARG ),

#ifdef NO_SDDR09
UNUSUAL_DEV(  0x04e6, 0x0005, 0x0100, 0x0208,
		"SCM Microsystems",
		"eUSB CompactFlash Adapter",
		USB_SC_SCSI, USB_PR_CB, NULL,
		US_FL_SINGLE_LUN),
#endif

/*                                                               */
UNUSUAL_DEV(  0x04e6, 0x0006, 0x0100, 0x0100,
		"SCM Microsystems Inc.",
		"eUSB MMC Adapter",
		USB_SC_SCSI, USB_PR_CB, NULL,
		US_FL_SINGLE_LUN),

/*                                                */
UNUSUAL_DEV(  0x04e6, 0x0006, 0x0205, 0x0205,
		"Shuttle",
		"eUSB MMC Adapter",
		USB_SC_SCSI, USB_PR_DEVICE, NULL,
		US_FL_SINGLE_LUN),

UNUSUAL_DEV(  0x04e6, 0x0007, 0x0100, 0x0200,
		"Sony",
		"Hifd",
		USB_SC_SCSI, USB_PR_CB, NULL,
		US_FL_SINGLE_LUN),

UNUSUAL_DEV(  0x04e6, 0x0009, 0x0200, 0x0200,
		"Shuttle",
		"eUSB ATA/ATAPI Adapter",
		USB_SC_8020, USB_PR_CB, NULL, 0),

UNUSUAL_DEV(  0x04e6, 0x000a, 0x0200, 0x0200,
		"Shuttle",
		"eUSB CompactFlash Adapter",
		USB_SC_8020, USB_PR_CB, NULL, 0),

UNUSUAL_DEV(  0x04e6, 0x000B, 0x0100, 0x0100,
		"Shuttle",
		"eUSCSI Bridge",
		USB_SC_SCSI, USB_PR_BULK, usb_stor_euscsi_init,
		US_FL_SCM_MULT_TARG ), 

UNUSUAL_DEV(  0x04e6, 0x000C, 0x0100, 0x0100,
		"Shuttle",
		"eUSCSI Bridge",
		USB_SC_SCSI, USB_PR_BULK, usb_stor_euscsi_init,
		US_FL_SCM_MULT_TARG ),

UNUSUAL_DEV(  0x04e6, 0x0101, 0x0200, 0x0200,
		"Shuttle",
		"CD-RW Device",
		USB_SC_8020, USB_PR_CB, NULL, 0),

/*                                                   */
UNUSUAL_DEV(  0x04e8, 0x507c, 0x0220, 0x0220,
		"Samsung",
		"YP-U3",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_MAX_SECTORS_64),

/*                                                  */
UNUSUAL_DEV(  0x04e8, 0x5122, 0x0000, 0x9999,
		"Samsung",
		"YP-CP3",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_MAX_SECTORS_64 | US_FL_BULK_IGNORE_TAG),

/*                                               */
UNUSUAL_DEV(  0x04e8, 0x5136, 0x0000, 0x9999,
		"Samsung",
		"YP-Z3",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_MAX_SECTORS_64),

/*                                                                     
                                                                          
                                                                             
 */
UNUSUAL_DEV(  0x04fc, 0x80c2, 0x0100, 0x0100,
		"Kobian Mercury",
		"Binocam DCB-132",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_BULK32),

/*                                                                 */
UNUSUAL_DEV(  0x050d, 0x0115, 0x0133, 0x0133,
		"Belkin",
		"USB SCSI Adaptor",
		USB_SC_SCSI, USB_PR_BULK, usb_stor_euscsi_init,
		US_FL_SCM_MULT_TARG ),

/*                    
                                                     
                                                
 */
UNUSUAL_DEV(  0x0525, 0xa140, 0x0100, 0x0100,
		"Iomega",
		"USB Clik! 40",
		USB_SC_8070, USB_PR_DEVICE, NULL,
		US_FL_FIX_INQUIRY ),

/*                                                 */
COMPLIANT_DEV(0x0525, 0xa4a5, 0x0000, 0x9999,
		"Linux",
		"File-backed Storage Gadget",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_CAPACITY_OK ),

/*                     
                                                           */
UNUSUAL_DEV(  0x052b, 0x1801, 0x0100, 0x0100,
		"Tekom Technologies, Inc",
		"300_CAMERA",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE ),

/*                       
                                                               */
UNUSUAL_DEV(  0x052b, 0x1804, 0x0100, 0x0100,
		"Tekom Technologies, Inc",
		"300_CAMERA",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE ),

/*                                                    */
UNUSUAL_DEV(  0x052b, 0x1807, 0x0100, 0x0100,
		"Tekom Technologies, Inc",
		"300_CAMERA",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE ),

/*                     
                                                   */
UNUSUAL_DEV(  0x052b, 0x1905, 0x0100, 0x0100,
		"Tekom Technologies, Inc",
		"400_CAMERA",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE ),

/*                                            
                                                                     */
UNUSUAL_DEV(  0x052b, 0x1911, 0x0100, 0x0100,
		"Tekom Technologies, Inc",
		"400_CAMERA",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE ),

UNUSUAL_DEV(  0x054c, 0x0010, 0x0106, 0x0450,
		"Sony",
		"DSC-S30/S70/S75/505V/F505/F707/F717/P8",
		USB_SC_SCSI, USB_PR_DEVICE, NULL,
		US_FL_SINGLE_LUN | US_FL_NOT_LOCKABLE | US_FL_NO_WP_DETECT ),

/*                                                    
                                                          */
UNUSUAL_DEV(  0x054c, 0x0010, 0x0500, 0x0610,
		"Sony",
		"DSC-T1/T5/H5",
		USB_SC_8070, USB_PR_DEVICE, NULL,
		US_FL_SINGLE_LUN ),


/*                          */
UNUSUAL_DEV(  0x054c, 0x0025, 0x0100, 0x0100,
		"Sony",
		"Memorystick NW-MS7",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_SINGLE_LUN ),

/*                                                              */
UNUSUAL_DEV(  0x054c, 0x002c, 0x0501, 0x2000,
		"Sony",
		"USB Floppy Drive",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_SINGLE_LUN ),

UNUSUAL_DEV(  0x054c, 0x002d, 0x0100, 0x0100,
		"Sony",
		"Memorystick MSAC-US1",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_SINGLE_LUN ),

/*                                                     */
UNUSUAL_DEV(  0x054c, 0x002e, 0x0106, 0x0310,
		"Sony",
		"Handycam",
		USB_SC_SCSI, USB_PR_DEVICE, NULL,
		US_FL_SINGLE_LUN ),

/*                                                      */
UNUSUAL_DEV(  0x054c, 0x002e, 0x0500, 0x0500,
		"Sony",
		"Handycam HC-85",
		USB_SC_UFI, USB_PR_DEVICE, NULL,
		US_FL_SINGLE_LUN ),

UNUSUAL_DEV(  0x054c, 0x0032, 0x0000, 0x9999,
		"Sony",
		"Memorystick MSC-U01N",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_SINGLE_LUN ),

/*                                               */
UNUSUAL_DEV(  0x054c, 0x0058, 0x0000, 0x9999,
		"Sony",
		"PEG N760c Memorystick",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_INQUIRY ),
		
UNUSUAL_DEV(  0x054c, 0x0069, 0x0000, 0x9999,
		"Sony",
		"Memorystick MSC-U03",
		USB_SC_UFI, USB_PR_CB, NULL,
		US_FL_SINGLE_LUN ),

/*                                            */
UNUSUAL_DEV(  0x054c, 0x006d, 0x0000, 0x9999,
		"Sony",
		"PEG Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_INQUIRY ),

/*                                                    */
UNUSUAL_DEV(  0x054c, 0x0099, 0x0000, 0x9999,
		"Sony",
		"PEG Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_INQUIRY ),

/*                                                       */
UNUSUAL_DEV(  0x054c, 0x016a, 0x0000, 0x9999,
		"Sony",
		"PEG Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_INQUIRY ),

/*                                                       */
UNUSUAL_DEV(  0x054c, 0x02a5, 0x0100, 0x0100,
		"Sony Corp.",
		"MicroVault Flash Drive",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_NO_READ_CAPACITY_16 ),

/*                              */
UNUSUAL_DEV(  0x055d, 0x2020, 0x0000, 0x0210,
		"SAMSUNG",
		"SFD-321U [FW 0C]",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_SINGLE_LUN ),

/*                                                                           */
UNUSUAL_DEV(  0x057b, 0x0000, 0x0000, 0x0299,
		"Y-E Data",
		"Flashbuster-U",
		USB_SC_DEVICE,  USB_PR_CB, NULL,
		US_FL_SINGLE_LUN),

/*                                                  
                                                       
                                           
 */
UNUSUAL_DEV(  0x057b, 0x0022, 0x0000, 0x9999,
		"Y-E Data",
		"Silicon Media R/W",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL, 0),

/*                                      */
UNUSUAL_DEV(  0x058f, 0x6387, 0x0141, 0x0141,
		"JetFlash",
		"TS1GJF2A/120",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_MAX_SECTORS_64 ),

/*                                    */
UNUSUAL_DEV(  0x0595, 0x4343, 0x0000, 0x2210,
		"Fujifilm",
		"Digital Camera EX-20 DSC",
		USB_SC_8070, USB_PR_DEVICE, NULL, 0 ),

/*                                             
                                                                      
                                                                       
                                                                      
                               
 */
UNUSUAL_DEV(  0x059b, 0x0001, 0x0100, 0x0100,
		"Iomega",
		"ZIP 100",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_SINGLE_LUN ),

/*                                       */
UNUSUAL_DEV(  0x059f, 0x0643, 0x0000, 0x0000,
		"LaCie",
		"DVD+-RW",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_GO_SLOW ),

/*                                                  
                                                                        
                      
 */
UNUSUAL_DEV(  0x05ab, 0x0060, 0x1104, 0x1110,
		"In-System",
		"PyroGate External CD-ROM Enclosure (FCD-523)",
		USB_SC_SCSI, USB_PR_BULK, NULL,
		US_FL_NEED_OVERRIDE ),

/*                                                    
                                                                           
                                                                          
                                                                          
                
 */
UNUSUAL_DEV( 0x05ac, 0x1202, 0x0000, 0x9999,
		"Apple",
		"iPod",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY ),

/*                                         */
UNUSUAL_DEV( 0x05ac, 0x1203, 0x0000, 0x9999,
		"Apple",
		"iPod",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY ),

UNUSUAL_DEV( 0x05ac, 0x1204, 0x0000, 0x9999,
		"Apple",
		"iPod",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY | US_FL_NOT_LOCKABLE ),

UNUSUAL_DEV( 0x05ac, 0x1205, 0x0000, 0x9999,
		"Apple",
		"iPod",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY ),

/*
                                               
                                             
 */
UNUSUAL_DEV( 0x05ac, 0x120a, 0x0000, 0x9999,
		"Apple",
		"iPod",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY ),

/*                                           
                                      
                                                              
 */

/*                                                                  */
UNUSUAL_DEV(  0x05c6, 0x1000, 0x0000, 0x9999,
		"Option N.V.",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, option_ms_init,
		0),

/*                                                 */
UNUSUAL_DEV(  0x05dc, 0xb002, 0x0000, 0x0113,
		"Lexar",
		"USB CF Reader",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_INQUIRY ),

/*                                                       
                                                         
                                                            
                                                  
                                                                 
                
  
                                                                
                                                          
                                             
 */
UNUSUAL_DEV(  0x05e3, 0x0701, 0x0000, 0xffff,
		"Genesys Logic",
		"USB to IDE Optical",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_GO_SLOW | US_FL_MAX_SECTORS_64 | US_FL_IGNORE_RESIDUE ),

UNUSUAL_DEV(  0x05e3, 0x0702, 0x0000, 0xffff,
		"Genesys Logic",
		"USB to IDE Disk",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_GO_SLOW | US_FL_MAX_SECTORS_64 | US_FL_IGNORE_RESIDUE ),

/*                                           */
UNUSUAL_DEV(  0x05e3, 0x0723, 0x9451, 0x9451,
		"Genesys Logic",
		"USB to SATA",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_SANE_SENSE ),

/*                                       
                                 */
UNUSUAL_DEV(  0x0636, 0x0003, 0x0000, 0x9999,
		"Vivitar",
		"Vivicam 35Xx",
		USB_SC_SCSI, USB_PR_BULK, NULL,
		US_FL_FIX_INQUIRY ),

UNUSUAL_DEV(  0x0644, 0x0000, 0x0100, 0x0100,
		"TEAC",
		"Floppy Drive",
		USB_SC_UFI, USB_PR_CB, NULL, 0 ),

/*                                                     */
UNUSUAL_DEV( 0x066f, 0x8000, 0x0001, 0x0001,
		"SigmaTel",
		"USBMSC Audio Player",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY ),

/*                                                   */
UNUSUAL_DEV( 0x067b, 0x1063, 0x0100, 0x0100,
		"Prolific Technology, Inc.",
		"Prolific Storage Gadget",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_BAD_SENSE ),

/*                                               */
UNUSUAL_DEV( 0x067b, 0x2317, 0x0001, 0x001,
		"Prolific Technology, Inc.",
		"Mass Storage Device",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_NOT_LOCKABLE ),

/*                                                      */
/*                                                      
                                             */
UNUSUAL_DEV( 0x067b, 0x2507, 0x0001, 0x0100,
		"Prolific Technology Inc.",
		"Mass Storage Device",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY | US_FL_GO_SLOW ),

/*                                                        */
UNUSUAL_DEV( 0x067b, 0x3507, 0x0001, 0x0101,
		"Prolific Technology Inc.",
		"ATAPI-6 Bridge Controller",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY | US_FL_GO_SLOW ),

/*                                                       */
UNUSUAL_DEV( 0x0686, 0x4011, 0x0001, 0x0001,
		"Minolta",
		"Dimage F300",
		USB_SC_SCSI, USB_PR_BULK, NULL, 0 ),

/*                                               */
UNUSUAL_DEV(  0x0686, 0x4017, 0x0001, 0x0001,
		"Minolta",
		"DIMAGE E223",
		USB_SC_SCSI, USB_PR_DEVICE, NULL, 0 ),

UNUSUAL_DEV(  0x0693, 0x0005, 0x0100, 0x0100,
		"Hagiwara",
		"Flashgate",
		USB_SC_SCSI, USB_PR_BULK, NULL, 0 ),

/*                                                        */
UNUSUAL_DEV(  0x069b, 0x3004, 0x0001, 0x0001,
		"Thomson Multimedia Inc.",
		"RCA RD1080 MP3 Player",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY ),

/*                                                */
UNUSUAL_DEV(  0x071b, 0x3203, 0x0000, 0x0000,
		"RockChip",
		"MP3",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_NO_WP_DETECT | US_FL_MAX_SECTORS_64 |
		US_FL_NO_READ_CAPACITY_16),

/*                                                   
                                  
                            
 */
UNUSUAL_DEV(  0x071b, 0x32bb, 0x0000, 0x0000,
		"RockChip",
		"MTP",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_NO_WP_DETECT | US_FL_MAX_SECTORS_64),

/*                                                                    
                                                                        
                                                                          
                                      
                                          
                           
                          
                          
                                 
 */
UNUSUAL_DEV(  0x071b, 0x3203, 0x0100, 0x0100,
		"RockChip",
		"ROCK MP3",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_MAX_SECTORS_64),

/*                                                  */
UNUSUAL_DEV(  0x0727, 0x0306, 0x0100, 0x0100,
		"ATMEL",
		"SND1 Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE),

/*                                            */
UNUSUAL_DEV(  0x0781, 0x0001, 0x0200, 0x0200,
		"Sandisk",
		"ImageMate SDDR-05a",
		USB_SC_SCSI, USB_PR_CB, NULL,
		US_FL_SINGLE_LUN ),

UNUSUAL_DEV(  0x0781, 0x0002, 0x0009, 0x0009,
		"SanDisk Corporation",
		"ImageMate CompactFlash USB",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY ),

UNUSUAL_DEV(  0x0781, 0x0100, 0x0100, 0x0100,
		"Sandisk",
		"ImageMate SDDR-12",
		USB_SC_SCSI, USB_PR_CB, NULL,
		US_FL_SINGLE_LUN ),

/*                                                  */
UNUSUAL_DEV(  0x07ab, 0xfccd, 0x0000, 0x9999,
		"Freecom Technologies",
		"FHD-Classic",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY),

UNUSUAL_DEV(  0x07af, 0x0004, 0x0100, 0x0133,
		"Microtech",
		"USB-SCSI-DB25",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_euscsi_init,
		US_FL_SCM_MULT_TARG ), 

UNUSUAL_DEV(  0x07af, 0x0005, 0x0100, 0x0100,
		"Microtech",
		"USB-SCSI-HD50",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_euscsi_init,
		US_FL_SCM_MULT_TARG ),

#ifdef NO_SDDR09
UNUSUAL_DEV(  0x07af, 0x0006, 0x0100, 0x0100,
		"Microtech",
		"CameraMate",
		USB_SC_SCSI, USB_PR_CB, NULL,
		US_FL_SINGLE_LUN ),
#endif

/*                                                                 
                                                                
                                                                      
                                                           
                                                       
 */
UNUSUAL_DEV(  0x07c4, 0xa400, 0x0000, 0xffff,
		"Datafab",
		"KECF-USB",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_INQUIRY | US_FL_FIX_CAPACITY ),

/*                                             
                                                                 
 */
UNUSUAL_DEV(  0x07c4, 0xa4a5, 0x0000, 0xffff,
		"Simple Tech/Datafab",
		"CF+SM Reader",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE | US_FL_MAX_SECTORS_64 ),

/*                                                                      
                                                
                                                                        
                                     
                                                                        
                                 
                                                                 
                                                                             
                                            
                                                                    
 */
UNUSUAL_DEV( 0x07cf, 0x1001, 0x1000, 0x9999,
		"Casio",
		"QV DigitalCamera",
		USB_SC_8070, USB_PR_CB, NULL,
		US_FL_NEED_OVERRIDE | US_FL_FIX_INQUIRY ),

/*                                                       */
UNUSUAL_DEV( 0x07cf, 0x1167, 0x0100, 0x0100,
		"Casio",
		"EX-N1 DigitalCamera",
		USB_SC_8070, USB_PR_DEVICE, NULL, 0),

/*                                           */
UNUSUAL_DEV( 0x0839, 0x000a, 0x0001, 0x0001,
		"Samsung",
		"Digimax 410",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_INQUIRY),

/*                                                 */
UNUSUAL_DEV( 0x0840, 0x0082, 0x0001, 0x0001,
		"Argosy",
		"Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY),

/*                                                             */
UNUSUAL_DEV( 0x0840, 0x0084, 0x0001, 0x0001,
		"Argosy",
		"Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY),

/*                                                       */
UNUSUAL_DEV( 0x0840, 0x0085, 0x0001, 0x0001,
		"Argosy",
		"Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY),

/*                                                                     
                                                                         
                                                                           
                                                                        
 */

UNUSUAL_DEV(  0x084d, 0x0011, 0x0110, 0x0110,
		"Grandtech",
		"DC2MEGA",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_BULK32),

/*                              
                                                                  
                                 
 */
UNUSUAL_DEV(  0x0851, 0x1542, 0x0002, 0x0002,
		"MagicPixel",
		"FW_Omega2",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL, 0),

/*                             
                                                                       
            
                                                                   
*/
UNUSUAL_DEV(  0x0851, 0x1543, 0x0200, 0x0200,
		"PanDigital",
		"Photo Frame",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_NOT_LOCKABLE),

/*                                            */
UNUSUAL_DEV(  0x08bd, 0x1100, 0x0000, 0x0000,
		"CITIZEN",
		"X1DE-USB",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_SINGLE_LUN),

/*                                            
                              
 */
UNUSUAL_DEV(  0x08ca, 0x3103, 0x0100, 0x0100,
		"AIPTEK",
		"Aiptek USB Keychain MP3 Player",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE),

/*                                                               
                                                                       
                                                    
                                                                
 */
UNUSUAL_DEV(  0x090a, 0x1001, 0x0100, 0x0100,
		"Trumpion",
		"t33520 USB Flash Card Controller",
		USB_SC_DEVICE, USB_PR_BULK, NULL,
		US_FL_NEED_OVERRIDE ),

/*                                                  
                                                        
 */
UNUSUAL_DEV(  0x090a, 0x1050, 0x0100, 0x0100,
		"Trumpion Microelectronics, Inc.",
		"33520 USB Digital Voice Recorder",
		USB_SC_UFI, USB_PR_DEVICE, NULL,
		0),

/*                                                                    */
UNUSUAL_DEV( 0x090a, 0x1200, 0x0000, 0x9999,
		"Trumpion",
		"MP3 player",
		USB_SC_RBC, USB_PR_BULK, NULL,
		0 ),

/*     */
UNUSUAL_DEV( 0x090c, 0x1132, 0x0000, 0xffff,
		"Feiya",
		"5-in-1 Card Reader",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY ),

/*                                                        
                                                                   
                                                                     
 */
UNUSUAL_DEV(  0x090c, 0x6000, 0x0100, 0x0100,
		"Feiya",
		"SD/SDHC Card Reader",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_INITIAL_READ10 ),

/*                                           
                                      
                                                                         
                                 
                                
                                                                  
 */
UNUSUAL_DEV( 0x0a17, 0x0004, 0x1000, 0x1000,
		"Pentax",
		"Optio 2/3/400",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_INQUIRY ),

/*                                                                
                                             */
UNUSUAL_DEV( 0x0ace, 0x2011, 0x0101, 0x0101,
		"ZyXEL",
		"G-220F USB-WLAN Install",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_DEVICE ),

UNUSUAL_DEV( 0x0ace, 0x20ff, 0x0101, 0x0101,
		"SiteCom",
		"WL-117 USB-WLAN Install",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_DEVICE ),

/*                                           
                                      
                                                              
 */

/*          */
UNUSUAL_DEV(  0x0af0, 0x6971, 0x0000, 0x9999,
		"Option N.V.",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, option_ms_init,
		0),

/*                                        
                                                                 
                                                                   
                                    */
UNUSUAL_DEV( 0x0af0, 0x7401, 0x0000, 0x0000,
		"Option",
		"GI 0401 SD-Card",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		0 ),

/*                                           
                                                                    
                                                                         
                                    */
UNUSUAL_DEV( 0x0af0, 0x7501, 0x0000, 0x0000,
		"Option",
		"GI 0431 SD-Card",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		0 ),

UNUSUAL_DEV( 0x0af0, 0x7701, 0x0000, 0x0000,
		"Option",
		"GI 0451 SD-Card",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		0 ),

UNUSUAL_DEV( 0x0af0, 0x7706, 0x0000, 0x0000,
		"Option",
		"GI 0451 SD-Card",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		0 ),

UNUSUAL_DEV( 0x0af0, 0x7901, 0x0000, 0x0000,
		"Option",
		"GI 0452 SD-Card",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		0 ),

UNUSUAL_DEV( 0x0af0, 0x7A01, 0x0000, 0x0000,
		"Option",
		"GI 0461 SD-Card",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		0 ),

UNUSUAL_DEV( 0x0af0, 0x7A05, 0x0000, 0x0000,
		"Option",
		"GI 0461 SD-Card",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		0 ),

UNUSUAL_DEV( 0x0af0, 0x8300, 0x0000, 0x0000,
		"Option",
		"GI 033x SD-Card",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		0 ),

UNUSUAL_DEV( 0x0af0, 0x8302, 0x0000, 0x0000,
		"Option",
		"GI 033x SD-Card",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		0 ),

UNUSUAL_DEV( 0x0af0, 0x8304, 0x0000, 0x0000,
		"Option",
		"GI 033x SD-Card",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		0 ),

UNUSUAL_DEV( 0x0af0, 0xc100, 0x0000, 0x0000,
		"Option",
		"GI 070x SD-Card",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		0 ),

UNUSUAL_DEV( 0x0af0, 0xd057, 0x0000, 0x0000,
		"Option",
		"GI 1505 SD-Card",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		0 ),

UNUSUAL_DEV( 0x0af0, 0xd058, 0x0000, 0x0000,
		"Option",
		"GI 1509 SD-Card",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		0 ),

UNUSUAL_DEV( 0x0af0, 0xd157, 0x0000, 0x0000,
		"Option",
		"GI 1515 SD-Card",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		0 ),

UNUSUAL_DEV( 0x0af0, 0xd257, 0x0000, 0x0000,
		"Option",
		"GI 1215 SD-Card",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		0 ),

UNUSUAL_DEV( 0x0af0, 0xd357, 0x0000, 0x0000,
		"Option",
		"GI 1505 SD-Card",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		0 ),

/*                                                   */
UNUSUAL_DEV(0x0bc2, 0x2300, 0x0000, 0x9999,
		"Seagate",
		"Portable HDD",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL, US_FL_WRITE_CACHE),

/*                                           */
UNUSUAL_DEV( 0x0bc2, 0x3010, 0x0000, 0x0000,
		"Seagate",
		"FreeAgent Pro",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_SANE_SENSE ),

UNUSUAL_DEV(  0x0d49, 0x7310, 0x0000, 0x9999,
		"Maxtor",
		"USB to SATA",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_SANE_SENSE),

/*
                                               
                                                               
 */
UNUSUAL_DEV( 0x0c45, 0x1060, 0x0100, 0x0100,
		"Unknown",
		"Unknown",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_SINGLE_LUN ),

/*                                               */
UNUSUAL_DEV( 0x0d96, 0x410a, 0x0001, 0xffff,
		"Medion",
		"MD 7425",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_INQUIRY),

/*
                               
  
                          
 */
UNUSUAL_DEV(  0x0d96, 0x5200, 0x0001, 0x0200,
		"Jenoptik",
		"JD 5200 z3",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL, US_FL_FIX_INQUIRY),

/*                                               */
UNUSUAL_DEV(  0x0dc4, 0x0073, 0x0000, 0x0000,
		"Macpower Technology Co.LTD.",
		"USB 2.0 3.5\" DEVICE",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY),

/*                                               
                                                                            
                                                                            
            
 */
UNUSUAL_DEV( 0x0dd8, 0x1060, 0x0000, 0xffff,
		"Netac",
		"USB-CF-Card",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_INQUIRY ),

/*                                                               
                                                       */
UNUSUAL_DEV( 0x0dd8, 0xd202, 0x0000, 0x9999,
		"Netac",
		"USB Flash Disk",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE ),


/*                                                 
                                     */
UNUSUAL_DEV( 0x0dda, 0x0001, 0x0012, 0x0012,
		"WINWARD",
		"Music Disk",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE ),

/*                                                     */
UNUSUAL_DEV(  0x0dda, 0x0301, 0x0012, 0x0012,
		"PNP_MP3",
		"PNP_MP3 PLAYER",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE ),

/*                                              */
UNUSUAL_DEV( 0x0e21, 0x0520, 0x0100, 0x0100,
		"Cowon Systems",
		"iAUDIO M5",
		USB_SC_DEVICE, USB_PR_BULK, NULL,
		US_FL_NEED_OVERRIDE ),

/*                                                          */
UNUSUAL_DEV( 0x0ed1, 0x6660, 0x0100, 0x0300,
		"USB",
		"Solid state disk",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_INQUIRY ),

/*                                           
                                          */
UNUSUAL_DEV(  0x0ea0, 0x2168, 0x0110, 0x0110,
		"Ours Technology",
		"Flash Disk",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE ),

/*                                                    */
UNUSUAL_DEV(  0x0ea0, 0x6828, 0x0110, 0x0110,
		"USB",
		"Flash Disk",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE ),

/*                                              
                                        */
UNUSUAL_DEV(  0x0ed1, 0x7636, 0x0103, 0x0103,
		"Typhoon",
		"My DJ 1820",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE | US_FL_GO_SLOW | US_FL_MAX_SECTORS_64),

/*                                           
                                                            
                                                     
                                                        
 */
UNUSUAL_DEV(  0x0f19, 0x0103, 0x0100, 0x0100,
		"Oracom Co., Ltd",
		"ORC-200M",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE ),

/*                                  
                                                                         
                               
 */
UNUSUAL_DEV(  0x0f19, 0x0105, 0x0100, 0x0100,
		"C-MEX",
		"A-VOX",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE ),

/*                            */
UNUSUAL_DEV( 0x0f88, 0x042e, 0x0100, 0x0100,
		"VTech",
		"Kidizoom",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY ),

/*                                                                         */
UNUSUAL_DEV(  0x0fca, 0x8004, 0x0201, 0x0201,
		"Research In Motion",
		"BlackBerry Bold 9000",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_MAX_SECTORS_64 ),

/*                                                       */
UNUSUAL_DEV(  0x0fce, 0xd008, 0x0000, 0x0000,
		"Sony Ericsson",
		"V800-Vodafone 802",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_NO_WP_DETECT ),

/*                                                */
UNUSUAL_DEV(  0x0fce, 0xd0e1, 0x0000, 0x0000,
		"Sony Ericsson",
		"MD400",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_DEVICE),

/*                                          
                                            */
UNUSUAL_DEV(  0x0fce, 0xe030, 0x0000, 0x0000,
		"Sony Ericsson",
		"P990i",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY | US_FL_IGNORE_RESIDUE ),

/*                                                   */
UNUSUAL_DEV(  0x0fce, 0xe031, 0x0000, 0x0000,
		"Sony Ericsson",
		"M600i",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE | US_FL_FIX_CAPACITY ),

/*                                                     */
UNUSUAL_DEV(  0x0fce, 0xe092, 0x0000, 0x0000,
		"Sony Ericsson",
		"P1i",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE ),

/*                                                        
                                   
                                                              
                                                         
                                    
 */
UNUSUAL_DEV(  0x1019, 0x0c55, 0x0000, 0x0110,
		"Desknote",
		"UCR-61S2B",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_ucr61s2b_init,
		0 ),

UNUSUAL_DEV(  0x1058, 0x0704, 0x0000, 0x9999,
		"Western Digital",
		"External HDD",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_SANE_SENSE),

/*                                                   */
UNUSUAL_DEV(0x1058, 0x070a, 0x0000, 0x9999,
		"Western Digital",
		"My Passport HDD",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL, US_FL_WRITE_CACHE),

/*                                               
                                                     
 */
UNUSUAL_DEV(  0x10d6, 0x2200, 0x0100, 0x0100,
		"Actions Semiconductor",
		"Mtp device",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		0),

/*                                                 
                                                              
 */
UNUSUAL_DEV(  0x1186, 0x3e04, 0x0000, 0x0000,
           "D-Link",
           "USB Mass Storage",
           USB_SC_DEVICE, USB_PR_DEVICE, option_ms_init, US_FL_IGNORE_DEVICE),

/*                                                   
                                                         
                                                
          
 */
UNUSUAL_DEV(  0x1199, 0x0fff, 0x0000, 0x9999,
		"Sierra Wireless",
		"USB MMC Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, sierra_ms_init,
		0),

/*                                          
                                                                          
                                                                
 */
UNUSUAL_DEV(  0x1210, 0x0003, 0x0100, 0x0100,
		"Digitech HMG",
		"DigiTech Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE ),

/*                                              
                                                                
 */
UNUSUAL_DEV(  0x12d1, 0x1001, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1003, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1004, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1401, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1402, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1403, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1404, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1405, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1406, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1407, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1408, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1409, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x140A, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x140B, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x140C, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x140D, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x140E, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x140F, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1410, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1411, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1412, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1413, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1414, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1415, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1416, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1417, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1418, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1419, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x141A, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x141B, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x141C, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x141D, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x141E, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x141F, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1420, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1421, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1422, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1423, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1424, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1425, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1426, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1427, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1428, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1429, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x142A, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x142B, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x142C, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x142D, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x142E, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x142F, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1430, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1431, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1432, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1433, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1434, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1435, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1436, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1437, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1438, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x1439, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x143A, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x143B, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x143C, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x143D, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x143E, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
UNUSUAL_DEV(  0x12d1, 0x143F, 0x0000, 0x0000,
		"HUAWEI MOBILE",
		"Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_e220_init,
		0),
#if 1		
HW_UNUSUAL_DEV ( 0x12d1, 0x08, 0x06, 0x50,
		"HUAWEI",
		"HUAWEI MOBILE Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_huawei_init,
		0),

HW_UNUSUAL_DEV ( 0x19d2, 0x08, 0x06, 0x50,
		"ZTE",
		"ZTE MOBILE Mass Storage",
		USB_SC_DEVICE, USB_PR_DEVICE, usb_stor_zte_init,
		0),
HW_UNUSUAL_DEV(  0x2001, 0xa708, 0x06, 0x50,
		"D-LINK",
		"CD-ROM",
		USB_SC_8020, USB_PR_DEVICE, usb_stor_dlink_scsi_init,
		0),
#endif
/*                                                          */
UNUSUAL_DEV(  0x132b, 0x000b, 0x0001, 0x0001,
		"Minolta",
		"Dimage Z10",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		0 ),

/*                                              */
UNUSUAL_DEV(  0x1370, 0x6828, 0x0110, 0x0110,
		"SWISSBIT",
		"Black Silver",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE ),

/*                                            */
UNUSUAL_DEV(  0x13fe, 0x3600, 0x0100, 0x0100,
		"Kingston",
		"DT 101 G2",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_BULK_IGNORE_TAG ),

/*                                                    */
UNUSUAL_DEV(  0x14cd, 0x6600, 0x0201, 0x0201,
		"Super Top",
		"IDE DEVICE",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE ),

/*                                                      
                                                               
                                                         */
UNUSUAL_DEV(  0x152d, 0x2329, 0x0100, 0x0100,
		"JMicron",
		"USB to ATA/ATAPI Bridge",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE | US_FL_SANE_SENSE ),

/*                                                
                                                                      */
UNUSUAL_DEV(  0x1652, 0x6600, 0x0201, 0x0201,
		"Teac",
		"HD-35PUK-B",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE ),

/*                                              */
UNUSUAL_DEV(  0x174c, 0x55aa, 0x0100, 0x0100,
		"ASMedia",
		"AS2105",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_NEEDS_CAP16),

/*                                                 */
UNUSUAL_DEV(  0x177f, 0x0400, 0x0000, 0x0000,
		"Yarvik",
		"PMP400",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_BULK_IGNORE_TAG | US_FL_MAX_SECTORS_64 ),

/*                                                
                                                                         
                                                                         
                                                                        */
UNUSUAL_DEV( 0x1908, 0x1315, 0x0000, 0x0000,
		"BUILDWIN",
		"Photo Frame",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_BAD_SENSE ),
UNUSUAL_DEV( 0x1908, 0x1320, 0x0000, 0x0000,
		"BUILDWIN",
		"Photo Frame",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_BAD_SENSE ),
UNUSUAL_DEV( 0x1908, 0x3335, 0x0200, 0x0200,
		"BUILDWIN",
		"Photo Frame",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_NO_READ_DISC_INFO ),

/*                                                
                                                                        
 */
UNUSUAL_DEV(  0x1b1c, 0x1ab5, 0x0200, 0x0200,
		"Corsair",
		"Padlock v2",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_INITIAL_READ10 ),

/*                                               
                                                               
                                                         */
UNUSUAL_DEV(  0x1e68, 0x001b, 0x0000, 0x0000,
		"TrekStor GmbH & Co. KG",
		"DataStation maxi g.u",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE | US_FL_SANE_SENSE ),

/*                                                             */
UNUSUAL_DEV( 0x1e74, 0x4621, 0x0000, 0x0000,
		"Coby Electronics",
		"MP3 Player",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_BULK_IGNORE_TAG | US_FL_MAX_SECTORS_64 ),

UNUSUAL_DEV( 0x2116, 0x0320, 0x0001, 0x0001,
		"ST",
		"2A",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY),

/*                                                                
                                       
 */
UNUSUAL_DEV(  0x22b8, 0x3010, 0x0001, 0x0001,
		"Motorola",
		"RAZR V3x",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_CAPACITY | US_FL_IGNORE_RESIDUE ),

/*
                                               
                              
                    
 */
UNUSUAL_DEV(  0x22b8, 0x6426, 0x0101, 0x0101,
		"Motorola",
		"MSnc.",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_FIX_INQUIRY | US_FL_FIX_CAPACITY | US_FL_BULK_IGNORE_TAG),

/*                                                                */
UNUSUAL_DEV(  0x2735, 0x100b, 0x0000, 0x9999,
		"MPIO",
		"HS200",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_GO_SLOW ),

/*                                                               
               
 */
UNUSUAL_DEV(  0x3340, 0xffff, 0x0000, 0x0000,
		"Mitac",
		"Mio DigiWalker USB Sync",
		USB_SC_DEVICE,USB_PR_DEVICE,NULL,
		US_FL_MAX_SECTORS_64 ),

/*                                                    */
UNUSUAL_DEV(  0x4102, 0x1020, 0x0100,  0x0100,
		"iRiver",
		"MP3 T10",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_IGNORE_RESIDUE ),

/*                                          */
UNUSUAL_DEV(  0x4102, 0x1059, 0x0000,  0x0000,
               "iRiver",
               "P7K",
               USB_SC_DEVICE, USB_PR_DEVICE, NULL,
               US_FL_MAX_SECTORS_64 ),

/*
                                   
                                                                       
 */
UNUSUAL_DEV(  0x4146, 0xba01, 0x0100, 0x0100,
		"Iomega",
		"Micro Mini 1GB",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL, US_FL_NOT_LOCKABLE ),

/*
                                         
                                                        
 */
UNUSUAL_DEV(  0xc251, 0x4003, 0x0100, 0x0100,
		"Keil Software, Inc.",
		"V2M MotherBoard",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_NOT_LOCKABLE),

/*                                                       */
UNUSUAL_DEV(  0xed06, 0x4500, 0x0001, 0x0001,
		"DataStor",
		"USB4500 FW1.04",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL,
		US_FL_CAPACITY_HEURISTICS),

/*                                                     */
UNUSUAL_DEV( 0xed10, 0x7636, 0x0001, 0x0001,
		"TGE",
		"Digital MP3 Audio Player",
		USB_SC_DEVICE, USB_PR_DEVICE, NULL, US_FL_NOT_LOCKABLE ),

/*                                                */
USUAL_DEV(USB_SC_RBC, USB_PR_CB),
USUAL_DEV(USB_SC_8020, USB_PR_CB),
USUAL_DEV(USB_SC_QIC, USB_PR_CB),
USUAL_DEV(USB_SC_UFI, USB_PR_CB),
USUAL_DEV(USB_SC_8070, USB_PR_CB),
USUAL_DEV(USB_SC_SCSI, USB_PR_CB),

/*                                                          */
USUAL_DEV(USB_SC_RBC, USB_PR_CBI),
USUAL_DEV(USB_SC_8020, USB_PR_CBI),
USUAL_DEV(USB_SC_QIC, USB_PR_CBI),
USUAL_DEV(USB_SC_UFI, USB_PR_CBI),
USUAL_DEV(USB_SC_8070, USB_PR_CBI),
USUAL_DEV(USB_SC_SCSI, USB_PR_CBI),

/*                                             */
USUAL_DEV(USB_SC_RBC, USB_PR_BULK),
USUAL_DEV(USB_SC_8020, USB_PR_BULK),
USUAL_DEV(USB_SC_QIC, USB_PR_BULK),
USUAL_DEV(USB_SC_UFI, USB_PR_BULK),
USUAL_DEV(USB_SC_8070, USB_PR_BULK),
USUAL_DEV(USB_SC_SCSI, USB_PR_BULK),
