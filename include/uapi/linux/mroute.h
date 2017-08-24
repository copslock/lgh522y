#ifndef _UAPI__LINUX_MROUTE_H
#define _UAPI__LINUX_MROUTE_H

#include <linux/sockios.h>
#include <linux/types.h>

/*
 *	Based on the MROUTING 3.5 defines primarily to keep
 *	source compatibility with BSD.
 *
 *	See the mrouted code for the original history.
 *
 *      Protocol Independent Multicast (PIM) data structures included
 *      Carlos Picoto (cap@di.fc.ul.pt)
 *
 */

#define MRT_BASE	200
#define MRT_INIT	(MRT_BASE)	/*                                  */
#define MRT_DONE	(MRT_BASE+1)	/*                             */
#define MRT_ADD_VIF	(MRT_BASE+2)	/*                          */
#define MRT_DEL_VIF	(MRT_BASE+3)	/*                             */
#define MRT_ADD_MFC	(MRT_BASE+4)	/*                                  */
#define MRT_DEL_MFC	(MRT_BASE+5)	/*                                     */
#define MRT_VERSION	(MRT_BASE+6)	/*                                  */
#define MRT_ASSERT	(MRT_BASE+7)	/*                           */
#define MRT_PIM		(MRT_BASE+8)	/*                   */
#define MRT_TABLE	(MRT_BASE+9)	/*                          */
#define MRT_ADD_MFC_PROXY	(MRT_BASE+10)	/*                         */
#define MRT_DEL_MFC_PROXY	(MRT_BASE+11)	/*                         */
#define MRT_MAX		(MRT_BASE+11)

#define SIOCGETVIFCNT	SIOCPROTOPRIVATE	/*                      */
#define SIOCGETSGCNT	(SIOCPROTOPRIVATE+1)
#define SIOCGETRPF	(SIOCPROTOPRIVATE+2)

#define MAXVIFS		32	
typedef unsigned long vifbitmap_t;	/*                                    */
typedef unsigned short vifi_t;
#define ALL_VIFS	((vifi_t)(-1))

/*
                      
 */
 
#define VIFM_SET(n,m)	((m)|=(1<<(n)))
#define VIFM_CLR(n,m)	((m)&=~(1<<(n)))
#define VIFM_ISSET(n,m)	((m)&(1<<(n)))
#define VIFM_CLRALL(m)	((m)=0)
#define VIFM_COPY(mfrom,mto)	((mto)=(mfrom))
#define VIFM_SAME(m1,m2)	((m1)==(m2))

/*
                                                          
                                           
 */
 
struct vifctl {
	vifi_t	vifc_vifi;		/*              */
	unsigned char vifc_flags;	/*             */
	unsigned char vifc_threshold;	/*           */
	unsigned int vifc_rate_limit;	/*                          */
	union {
		struct in_addr vifc_lcl_addr;     /*                         */
		int            vifc_lcl_ifindex;  /*                         */
	};
	struct in_addr vifc_rmt_addr;	/*                  */
};

#define VIFF_TUNNEL		0x1	/*             */
#define VIFF_SRCRT		0x2	/*    */
#define VIFF_REGISTER		0x4	/*              */
#define VIFF_USE_IFINDEX	0x8	/*                                
                                           */

/*
                                                     
 */
 
struct mfcctl {
	struct in_addr mfcc_origin;		/*                 */
	struct in_addr mfcc_mcastgrp;		/*                   */
	vifi_t	mfcc_parent;			/*                  */
	unsigned char mfcc_ttls[MAXVIFS];	/*                   */
	unsigned int mfcc_pkt_cnt;		/*                       */
	unsigned int mfcc_byte_cnt;
	unsigned int mfcc_wrong_if;
	int	     mfcc_expire;
};

/* 
                                    
 */
 
struct sioc_sg_req {
	struct in_addr src;
	struct in_addr grp;
	unsigned long pktcnt;
	unsigned long bytecnt;
	unsigned long wrong_if;
};

/*
                           
 */

struct sioc_vif_req {
	vifi_t	vifi;		/*             */
	unsigned long icount;	/*            */
	unsigned long ocount;	/*             */
	unsigned long ibytes;	/*          */
	unsigned long obytes;	/*           */
};

/*
                                                                   
                                                                      
 */
 
struct igmpmsg {
	__u32 unused1,unused2;
	unsigned char im_msgtype;		/*              */
	unsigned char im_mbz;			/*              */
	unsigned char im_vif;			/*                                        */
	unsigned char unused3;
	struct in_addr im_src,im_dst;
};

/*
                            
 */



#define MFC_ASSERT_THRESH (3*HZ)		/*                          */

/*
                                  
 */

#define IGMPMSG_NOCACHE		1		/*                                    */
#define IGMPMSG_WRONGVIF	2		/*                                    */
#define IGMPMSG_WHOLEPKT	3		/*                             */


#endif /*                       */
