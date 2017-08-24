/*
 *
 * Copyright 1999 Digi International (www.digi.com)
 *     James Puzzo <jamesp at digi dot com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, EXPRESS OR IMPLIED; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 */

/*
  
             
  
                    
  
                
  
                                                                     
                                                                   
                                      
  
           
  
                     
  
 */

#include <linux/errno.h>
#include <linux/tty.h>
#include <linux/sched.h>
#include <linux/cred.h>

#include "dgrp_common.h"

/* 
                                                         
                          
 */
void dgrp_carrier(struct ch_struct *ch)
{
	struct nd_struct *nd;

	int virt_carrier = 0;
	int phys_carrier = 0;

	/*                                           */

	if (!ch)
		return;
	nd  = ch->ch_nd;
	if (!nd)
		return;

	/*
                                                                     
                                                                  
                                                                    
                                                                      
                                                                     
                                                                  
  */
	if (ch->ch_expect & RR_STATUS)
		return;

	/*
                                                                      
                                              
                                                              
  */
	if ((ch->ch_flag & CH_HANGUP) &&
	    (ch->ch_tun.un_open_count > 0))
		tty_hangup(ch->ch_tun.un_tty);

	/*
                                                                 
                       
  */

	if (ch->ch_s_mlast & DM_CD)
		phys_carrier = 1;

	if ((ch->ch_s_mlast & DM_CD) ||
	    (ch->ch_digi.digi_flags & DIGI_FORCEDCD) ||
	    (ch->ch_flag & CH_CLOCAL))
		virt_carrier = 1;

	/*
                                                   
   
                                                              
                                                               
                                  
  */
	if (((ch->ch_flag & CH_HANGUP)  == 0) &&
	    ((ch->ch_flag & CH_VIRT_CD) == 0) &&
	    (virt_carrier == 1)) {
		/*
                                                 
                                     
   */
		nd->nd_tx_work = 1;

		if (waitqueue_active(&ch->ch_flag_wait))
			wake_up_interruptible(&ch->ch_flag_wait);
	}

	/*
                                                                
                                                                    
                         
   
                                                                
                                                               
                                          
  */
	if ((virt_carrier == 0) &&
	    ((ch->ch_flag & CH_PHYS_CD) != 0) &&
	    (phys_carrier == 0)) {
		/*
                        
    
                                              
    
                                      
    
                                                      
                       
    
                                             
    
                               
   */

		nd->nd_tx_work = 1;

		ch->ch_flag &= ~(CH_LOW | CH_EMPTY | CH_DRAIN | CH_INPUT);

		if (waitqueue_active(&ch->ch_flag_wait))
			wake_up_interruptible(&ch->ch_flag_wait);

		if (ch->ch_tun.un_open_count > 0)
			tty_hangup(ch->ch_tun.un_tty);

		if (ch->ch_pun.un_open_count > 0)
			tty_hangup(ch->ch_pun.un_tty);
	}

	/*
                                                                  
  */
	if (virt_carrier == 1)
		ch->ch_flag |= CH_VIRT_CD;
	else
		ch->ch_flag &= ~CH_VIRT_CD;

	if (phys_carrier == 1)
		ch->ch_flag |= CH_PHYS_CD;
	else
		ch->ch_flag &= ~CH_PHYS_CD;

}
