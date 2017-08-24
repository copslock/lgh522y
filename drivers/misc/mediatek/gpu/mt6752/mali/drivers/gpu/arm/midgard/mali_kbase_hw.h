/*
 *
 * (C) COPYRIGHT ARM Limited. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 *
 * A copy of the licence is included with the program, and can also be obtained
 * from Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 */





/* 
        
                                
 */

#ifndef _KBASE_HW_H_
#define _KBASE_HW_H_

#include "mali_kbase_defs.h"

/* 
                                                      
 */
#define kbase_hw_has_issue(kbdev, issue)\
	test_bit(issue, &(kbdev)->hw_issues_mask[0])

/* 
                                             
 */
#define kbase_hw_has_feature(kbdev, feature)\
	test_bit(feature, &(kbdev)->hw_features_mask[0])

/* 
                                                        
 */
mali_error kbase_hw_set_issues_mask(struct kbase_device *kbdev);

/* 
                                                       
 */
void kbase_hw_set_features_mask(struct kbase_device *kbdev);

#endif				/*              */
