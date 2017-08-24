/* $Id: isdnif.h,v 1.43.2.2 2004/01/12 23:08:35 keil Exp $
 *
 * Linux ISDN subsystem
 * Definition of the interface between the subsystem and its low-level drivers.
 *
 * Copyright 1994,95,96 by Fritz Elfert (fritz@isdn4linux.de)
 * Copyright 1995,96    Thinking Objects Software GmbH Wuerzburg
 * 
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 *
 */

#ifndef _UAPI__ISDNIF_H__
#define _UAPI__ISDNIF_H__


/*
                                        
 */
#define ISDN_PTYPE_UNKNOWN   0   /*                      */
#define ISDN_PTYPE_1TR6      1   /*                      */
#define ISDN_PTYPE_EURO      2   /*                      */
#define ISDN_PTYPE_LEASED    3   /*                      */
#define ISDN_PTYPE_NI1       4   /*                      */
#define ISDN_PTYPE_MAX       7   /*                      */

/*
                                        
 */
#define ISDN_PROTO_L2_X75I   0   /*                                   */
#define ISDN_PROTO_L2_X75UI  1   /*                                   */
#define ISDN_PROTO_L2_X75BUI 2   /*                                   */
#define ISDN_PROTO_L2_HDLC   3   /*                                   */
#define ISDN_PROTO_L2_TRANS  4   /*                                   */
#define ISDN_PROTO_L2_X25DTE 5   /*                                   */
#define ISDN_PROTO_L2_X25DCE 6   /*                                   */
#define ISDN_PROTO_L2_V11096 7   /*                                   */
#define ISDN_PROTO_L2_V11019 8   /*                                   */
#define ISDN_PROTO_L2_V11038 9   /*                                   */
#define ISDN_PROTO_L2_MODEM  10  /*                       */
#define ISDN_PROTO_L2_FAX    11  /*                       */
#define ISDN_PROTO_L2_HDLC_56K 12   /*                                   */
#define ISDN_PROTO_L2_MAX    15  /*                                   */

/*
                                        
 */
#define ISDN_PROTO_L3_TRANS	0	/*             */
#define ISDN_PROTO_L3_TRANSDSP	1	/*                      */
#define ISDN_PROTO_L3_FCLASS2	2	/*                       */
#define ISDN_PROTO_L3_FCLASS1	3	/*                       */
#define ISDN_PROTO_L3_MAX	7	/*                  */


#endif /*                   */
