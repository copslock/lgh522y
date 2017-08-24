/* Key permission checking
 *
 * Copyright (C) 2005 Red Hat, Inc. All Rights Reserved.
 * Written by David Howells (dhowells@redhat.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/security.h>
#include "internal.h"

/* 
                                                
                              
                                 
                                       
  
                                                                              
                                               
  
                                                                           
  
                                                                    
                                     
 */
int key_task_permission(const key_ref_t key_ref, const struct cred *cred,
			key_perm_t perm)
{
	struct key *key;
	key_perm_t kperm;
	int ret;

	key = key_ref_to_ptr(key_ref);

	/*                                                               */
	if (uid_eq(key->uid, cred->fsuid)) {
		kperm = key->perm >> 16;
		goto use_these_perms;
	}

	/*                                                                    
                              */
	if (gid_valid(key->gid) && key->perm & KEY_GRP_ALL) {
		if (gid_eq(key->gid, cred->fsgid)) {
			kperm = key->perm >> 8;
			goto use_these_perms;
		}

		ret = groups_search(cred->group_info, key->gid);
		if (ret) {
			kperm = key->perm >> 8;
			goto use_these_perms;
		}
	}

	/*                                            */
	kperm = key->perm;

use_these_perms:

	/*                                                                
                                                               
  */
	if (is_key_possessed(key_ref))
		kperm |= key->perm >> 24;

	kperm = kperm & perm & KEY_ALL;

	if (kperm != perm)
		return -EACCES;

	/*                              */
	return security_key_permission(key_ref, cred, perm);
}
EXPORT_SYMBOL(key_task_permission);

/* 
                                 
                                 
  
                                                                            
                                                                            
                                                                   
 */
int key_validate(const struct key *key)
{
	unsigned long flags = key->flags;

	if (flags & (1 << KEY_FLAG_INVALIDATED))
		return -ENOKEY;

	/*                             */
	if (flags & ((1 << KEY_FLAG_REVOKED) |
		     (1 << KEY_FLAG_DEAD)))
		return -EKEYREVOKED;

	/*                         */
	if (key->expiry) {
		struct timespec now = current_kernel_time();
		if (now.tv_sec >= key->expiry)
			return -EKEYEXPIRED;
	}

	return 0;
}
EXPORT_SYMBOL(key_validate);
