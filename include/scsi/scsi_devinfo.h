#ifndef _SCSI_SCSI_DEVINFO_H
#define _SCSI_SCSI_DEVINFO_H
/*
                                                     
 */
#define BLIST_NOLUN     	0x001	/*                 */
#define BLIST_FORCELUN  	0x002	/*                                    
                                   */
#define BLIST_BORKEN    	0x004	/*                             */
#define BLIST_KEY       	0x008	/*                           */
#define BLIST_SINGLELUN 	0x010	/*                             */
#define BLIST_NOTQ		0x020	/*                              */
#define BLIST_SPARSELUN 	0x040	/*                               */
#define BLIST_MAX5LUN		0x080	/*                 */
#define BLIST_ISROM     	0x100	/*                             */
#define BLIST_LARGELUN		0x200	/*                                */
#define BLIST_INQUIRY_36	0x400	/*                                  */
#define BLIST_INQUIRY_58	0x800	/*                                  */
#define BLIST_NOSTARTONADD	0x1000	/*                                  */
#define BLIST_MS_SKIP_PAGE_08	0x2000	/*                          */
#define BLIST_MS_SKIP_PAGE_3F	0x4000	/*                          */
#define BLIST_USE_10_BYTE_MS	0x8000	/*                                 */
#define BLIST_MS_192_BYTES_FOR_3F	0x10000	/*                                */
#define BLIST_REPORTLUN2	0x20000	/*                                     
                                            */
#define BLIST_NOREPORTLUN	0x40000	/*                                          */
#define BLIST_NOT_LOCKABLE	0x80000	/*                                  */
#define BLIST_NO_ULD_ATTACH	0x100000 /*                                    */
#define BLIST_SELECT_NO_ATN	0x200000 /*                    */
#define BLIST_RETRY_HWERROR	0x400000 /*                      */
#define BLIST_MAX_512		0x800000 /*                               */
#define BLIST_ATTACH_PQ3	0x1000000 /*                             */
#define BLIST_NO_DIF		0x2000000 /*                      */
#endif
