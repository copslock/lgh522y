/*
 * Copyright (c) 2000-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "xfs.h"
#include "xfs_fs.h"
#include "xfs_types.h"
#include "xfs_bit.h"
#include "xfs_log.h"
#include "xfs_trans.h"
#include "xfs_sb.h"
#include "xfs_ag.h"
#include "xfs_mount.h"
#include "xfs_da_btree.h"
#include "xfs_bmap_btree.h"
#include "xfs_attr_sf.h"
#include "xfs_dinode.h"
#include "xfs_inode.h"
#include "xfs_alloc.h"
#include "xfs_inode_item.h"
#include "xfs_bmap.h"
#include "xfs_attr.h"
#include "xfs_attr_leaf.h"
#include "xfs_attr_remote.h"
#include "xfs_error.h"
#include "xfs_quota.h"
#include "xfs_trans_space.h"
#include "xfs_vnodeops.h"
#include "xfs_trace.h"

/*
             
  
                                                             
 */

/*                                                                        
                                      
                                                                          */

/*
                                                               
 */
STATIC int xfs_attr_shortform_addname(xfs_da_args_t *args);

/*
                                                      
 */
STATIC int xfs_attr_leaf_get(xfs_da_args_t *args);
STATIC int xfs_attr_leaf_addname(xfs_da_args_t *args);
STATIC int xfs_attr_leaf_removename(xfs_da_args_t *args);
STATIC int xfs_attr_leaf_list(xfs_attr_list_context_t *context);

/*
                                                                
 */
STATIC int xfs_attr_node_get(xfs_da_args_t *args);
STATIC int xfs_attr_node_addname(xfs_da_args_t *args);
STATIC int xfs_attr_node_removename(xfs_da_args_t *args);
STATIC int xfs_attr_node_list(xfs_attr_list_context_t *context);
STATIC int xfs_attr_fillstate(xfs_da_state_t *state);
STATIC int xfs_attr_refillstate(xfs_da_state_t *state);


STATIC int
xfs_attr_name_to_xname(
	struct xfs_name	*xname,
	const unsigned char *aname)
{
	if (!aname)
		return EINVAL;
	xname->name = aname;
	xname->len = strlen((char *)aname);
	if (xname->len >= MAXNAMELEN)
		return EFAULT;		/*                      */

	return 0;
}

STATIC int
xfs_inode_hasattr(
	struct xfs_inode	*ip)
{
	if (!XFS_IFORK_Q(ip) ||
	    (ip->i_d.di_aformat == XFS_DINODE_FMT_EXTENTS &&
	     ip->i_d.di_anextents == 0))
		return 0;
	return 1;
}

/*                                                                        
                                       
                                                                          */

STATIC int
xfs_attr_get_int(
	struct xfs_inode	*ip,
	struct xfs_name		*name,
	unsigned char		*value,
	int			*valuelenp,
	int			flags)
{
	xfs_da_args_t   args;
	int             error;

	if (!xfs_inode_hasattr(ip))
		return ENOATTR;

	/*
                                               
  */
	memset((char *)&args, 0, sizeof(args));
	args.name = name->name;
	args.namelen = name->len;
	args.value = value;
	args.valuelen = *valuelenp;
	args.flags = flags;
	args.hashval = xfs_da_hashname(args.name, args.namelen);
	args.dp = ip;
	args.whichfork = XFS_ATTR_FORK;

	/*
                                                                 
  */
	if (ip->i_d.di_aformat == XFS_DINODE_FMT_LOCAL) {
		error = xfs_attr_shortform_getvalue(&args);
	} else if (xfs_bmap_one_block(ip, XFS_ATTR_FORK)) {
		error = xfs_attr_leaf_get(&args);
	} else {
		error = xfs_attr_node_get(&args);
	}

	/*
                                                          
  */
	*valuelenp = args.valuelen;

	if (error == EEXIST)
		error = 0;
	return(error);
}

int
xfs_attr_get(
	xfs_inode_t	*ip,
	const unsigned char *name,
	unsigned char	*value,
	int		*valuelenp,
	int		flags)
{
	int		error;
	struct xfs_name	xname;

	XFS_STATS_INC(xs_attr_get);

	if (XFS_FORCED_SHUTDOWN(ip->i_mount))
		return(EIO);

	error = xfs_attr_name_to_xname(&xname, name);
	if (error)
		return error;

	xfs_ilock(ip, XFS_ILOCK_SHARED);
	error = xfs_attr_get_int(ip, &xname, value, valuelenp, flags);
	xfs_iunlock(ip, XFS_ILOCK_SHARED);
	return(error);
}

/*
                                                           
 */
STATIC int
xfs_attr_calc_size(
	struct xfs_inode 	*ip,
	int			namelen,
	int			valuelen,
	int			*local)
{
	struct xfs_mount 	*mp = ip->i_mount;
	int			size;
	int			nblks;

	/*
                                                              
                                                
  */
	size = xfs_attr_leaf_newentsize(namelen, valuelen,
					mp->m_sb.sb_blocksize, local);

	nblks = XFS_DAENTER_SPACE_RES(mp, XFS_ATTR_FORK);
	if (*local) {
		if (size > (mp->m_sb.sb_blocksize >> 1)) {
			/*                       */
			nblks *= 2;
		}
	} else {
		/*
                                                    
                                              
   */
		uint	dblocks = XFS_B_TO_FSB(mp, valuelen);
		nblks += dblocks;
		nblks += XFS_NEXTENTADD_SPACE_RES(mp, dblocks, XFS_ATTR_FORK);
	}

	return nblks;
}

STATIC int
xfs_attr_set_int(
	struct xfs_inode *dp,
	struct xfs_name	*name,
	unsigned char	*value,
	int		valuelen,
	int		flags)
{
	xfs_da_args_t	args;
	xfs_fsblock_t	firstblock;
	xfs_bmap_free_t flist;
	int		error, err2, committed;
	xfs_mount_t	*mp = dp->i_mount;
	int             rsvd = (flags & ATTR_ROOT) != 0;
	int		local;

	/*
                                   
  */
	error = xfs_qm_dqattach(dp, 0);
	if (error)
		return error;

	/*
                                                         
                                                        
  */
	if (XFS_IFORK_Q(dp) == 0) {
		int sf_size = sizeof(xfs_attr_sf_hdr_t) +
			      XFS_ATTR_SF_ENTSIZE_BYNAME(name->len, valuelen);

		if ((error = xfs_bmap_add_attrfork(dp, sf_size, rsvd)))
			return(error);
	}

	/*
                                               
  */
	memset((char *)&args, 0, sizeof(args));
	args.name = name->name;
	args.namelen = name->len;
	args.value = value;
	args.valuelen = valuelen;
	args.flags = flags;
	args.hashval = xfs_da_hashname(args.name, args.namelen);
	args.dp = dp;
	args.firstblock = &firstblock;
	args.flist = &flist;
	args.whichfork = XFS_ATTR_FORK;
	args.op_flags = XFS_DA_OP_ADDNAME | XFS_DA_OP_OKNOENT;

	/*                                       */
	args.total = xfs_attr_calc_size(dp, name->len, valuelen, &local);

	/*
                                           
   
                                                                  
                                                                     
                                                                      
                                                                   
                                                                 
            
  */
	args.trans = xfs_trans_alloc(mp, XFS_TRANS_ATTR_SET);

	/*
                                                              
                          
  */

	if (rsvd)
		args.trans->t_flags |= XFS_TRANS_RESERVE;

	error = xfs_trans_reserve(args.trans, args.total,
				  XFS_ATTRSETM_LOG_RES(mp) +
				  XFS_ATTRSETRT_LOG_RES(mp) * args.total,
				  0, XFS_TRANS_PERM_LOG_RES,
				  XFS_ATTRSET_LOG_COUNT);
	if (error) {
		xfs_trans_cancel(args.trans, 0);
		return(error);
	}
	xfs_ilock(dp, XFS_ILOCK_EXCL);

	error = xfs_trans_reserve_quota_nblks(args.trans, dp, args.total, 0,
				rsvd ? XFS_QMOPT_RES_REGBLKS | XFS_QMOPT_FORCE_RES :
				       XFS_QMOPT_RES_REGBLKS);
	if (error) {
		xfs_iunlock(dp, XFS_ILOCK_EXCL);
		xfs_trans_cancel(args.trans, XFS_TRANS_RELEASE_LOG_RES);
		return (error);
	}

	xfs_trans_ijoin(args.trans, dp, 0);

	/*
                                                              
                                                     
  */
	if ((dp->i_d.di_aformat == XFS_DINODE_FMT_LOCAL) ||
	    ((dp->i_d.di_aformat == XFS_DINODE_FMT_EXTENTS) &&
	     (dp->i_d.di_anextents == 0))) {

		/*
                                                
   */
		if (dp->i_d.di_aformat == XFS_DINODE_FMT_EXTENTS)
			xfs_attr_shortform_create(&args);

		/*
                                                 
               
   */
		error = xfs_attr_shortform_addname(&args);
		if (error != ENOSPC) {
			/*
                                                
                                                      
    */
			ASSERT(args.trans != NULL);

			/*
                                                    
                                                   
                  
    */
			if (mp->m_flags & XFS_MOUNT_WSYNC) {
				xfs_trans_set_sync(args.trans);
			}

			if (!error && (flags & ATTR_KERNOTIME) == 0) {
				xfs_trans_ichgtime(args.trans, dp,
							XFS_ICHGTIME_CHG);
			}
			err2 = xfs_trans_commit(args.trans,
						 XFS_TRANS_RELEASE_LOG_RES);
			xfs_iunlock(dp, XFS_ILOCK_EXCL);

			return(error == 0 ? err2 : error);
		}

		/*
                                                              
                                                               
   */
		xfs_bmap_init(args.flist, args.firstblock);
		error = xfs_attr_shortform_to_leaf(&args);
		if (!error) {
			error = xfs_bmap_finish(&args.trans, args.flist,
						&committed);
		}
		if (error) {
			ASSERT(committed);
			args.trans = NULL;
			xfs_bmap_cancel(&flist);
			goto out;
		}

		/*
                                                                
                                                             
   */
		if (committed)
			xfs_trans_ijoin(args.trans, dp, 0);

		/*
                                                                 
                                                      
   */

		error = xfs_trans_roll(&args.trans, dp);
		if (error)
			goto out;

	}

	if (xfs_bmap_one_block(dp, XFS_ATTR_FORK)) {
		error = xfs_attr_leaf_addname(&args);
	} else {
		error = xfs_attr_node_addname(&args);
	}
	if (error) {
		goto out;
	}

	/*
                                                      
                                                          
  */
	if (mp->m_flags & XFS_MOUNT_WSYNC) {
		xfs_trans_set_sync(args.trans);
	}

	if ((flags & ATTR_KERNOTIME) == 0)
		xfs_trans_ichgtime(args.trans, dp, XFS_ICHGTIME_CHG);

	/*
                                                    
  */
	xfs_trans_log_inode(args.trans, dp, XFS_ILOG_CORE);
	error = xfs_trans_commit(args.trans, XFS_TRANS_RELEASE_LOG_RES);
	xfs_iunlock(dp, XFS_ILOCK_EXCL);

	return(error);

out:
	if (args.trans)
		xfs_trans_cancel(args.trans,
			XFS_TRANS_RELEASE_LOG_RES|XFS_TRANS_ABORT);
	xfs_iunlock(dp, XFS_ILOCK_EXCL);
	return(error);
}

int
xfs_attr_set(
	xfs_inode_t	*dp,
	const unsigned char *name,
	unsigned char	*value,
	int		valuelen,
	int		flags)
{
	int             error;
	struct xfs_name	xname;

	XFS_STATS_INC(xs_attr_set);

	if (XFS_FORCED_SHUTDOWN(dp->i_mount))
		return (EIO);

	error = xfs_attr_name_to_xname(&xname, name);
	if (error)
		return error;

	return xfs_attr_set_int(dp, &xname, value, valuelen, flags);
}

/*
                                                                   
                                                                   
 */
STATIC int
xfs_attr_remove_int(xfs_inode_t *dp, struct xfs_name *name, int flags)
{
	xfs_da_args_t	args;
	xfs_fsblock_t	firstblock;
	xfs_bmap_free_t	flist;
	int		error;
	xfs_mount_t	*mp = dp->i_mount;

	/*
                                               
  */
	memset((char *)&args, 0, sizeof(args));
	args.name = name->name;
	args.namelen = name->len;
	args.flags = flags;
	args.hashval = xfs_da_hashname(args.name, args.namelen);
	args.dp = dp;
	args.firstblock = &firstblock;
	args.flist = &flist;
	args.total = 0;
	args.whichfork = XFS_ATTR_FORK;

	/*
                                                                        
                                                                     
                    
  */
	args.op_flags = XFS_DA_OP_OKNOENT;

	/*
                                   
  */
	error = xfs_qm_dqattach(dp, 0);
	if (error)
		return error;

	/*
                                           
   
                                                                  
                                                                     
                                                                      
                                                                   
                                                                 
            
  */
	args.trans = xfs_trans_alloc(mp, XFS_TRANS_ATTR_RM);

	/*
                                                              
                          
  */

	if (flags & ATTR_ROOT)
		args.trans->t_flags |= XFS_TRANS_RESERVE;

	if ((error = xfs_trans_reserve(args.trans,
				      XFS_ATTRRM_SPACE_RES(mp),
				      XFS_ATTRRM_LOG_RES(mp),
				      0, XFS_TRANS_PERM_LOG_RES,
				      XFS_ATTRRM_LOG_COUNT))) {
		xfs_trans_cancel(args.trans, 0);
		return(error);
	}

	xfs_ilock(dp, XFS_ILOCK_EXCL);
	/*
                                                                      
                                           
  */
	xfs_trans_ijoin(args.trans, dp, 0);

	/*
                                                                 
  */
	if (!xfs_inode_hasattr(dp)) {
		error = XFS_ERROR(ENOATTR);
		goto out;
	}
	if (dp->i_d.di_aformat == XFS_DINODE_FMT_LOCAL) {
		ASSERT(dp->i_afp->if_flags & XFS_IFINLINE);
		error = xfs_attr_shortform_remove(&args);
		if (error) {
			goto out;
		}
	} else if (xfs_bmap_one_block(dp, XFS_ATTR_FORK)) {
		error = xfs_attr_leaf_removename(&args);
	} else {
		error = xfs_attr_node_removename(&args);
	}
	if (error) {
		goto out;
	}

	/*
                                                      
                                                          
  */
	if (mp->m_flags & XFS_MOUNT_WSYNC) {
		xfs_trans_set_sync(args.trans);
	}

	if ((flags & ATTR_KERNOTIME) == 0)
		xfs_trans_ichgtime(args.trans, dp, XFS_ICHGTIME_CHG);

	/*
                                                    
  */
	xfs_trans_log_inode(args.trans, dp, XFS_ILOG_CORE);
	error = xfs_trans_commit(args.trans, XFS_TRANS_RELEASE_LOG_RES);
	xfs_iunlock(dp, XFS_ILOCK_EXCL);

	return(error);

out:
	if (args.trans)
		xfs_trans_cancel(args.trans,
			XFS_TRANS_RELEASE_LOG_RES|XFS_TRANS_ABORT);
	xfs_iunlock(dp, XFS_ILOCK_EXCL);
	return(error);
}

int
xfs_attr_remove(
	xfs_inode_t	*dp,
	const unsigned char *name,
	int		flags)
{
	int		error;
	struct xfs_name	xname;

	XFS_STATS_INC(xs_attr_remove);

	if (XFS_FORCED_SHUTDOWN(dp->i_mount))
		return (EIO);

	error = xfs_attr_name_to_xname(&xname, name);
	if (error)
		return error;

	xfs_ilock(dp, XFS_ILOCK_SHARED);
	if (!xfs_inode_hasattr(dp)) {
		xfs_iunlock(dp, XFS_ILOCK_SHARED);
		return XFS_ERROR(ENOATTR);
	}
	xfs_iunlock(dp, XFS_ILOCK_SHARED);

	return xfs_attr_remove_int(dp, &xname, flags);
}

int
xfs_attr_list_int(xfs_attr_list_context_t *context)
{
	int error;
	xfs_inode_t *dp = context->dp;

	XFS_STATS_INC(xs_attr_list);

	if (XFS_FORCED_SHUTDOWN(dp->i_mount))
		return EIO;

	xfs_ilock(dp, XFS_ILOCK_SHARED);

	/*
                                                                 
  */
	if (!xfs_inode_hasattr(dp)) {
		error = 0;
	} else if (dp->i_d.di_aformat == XFS_DINODE_FMT_LOCAL) {
		error = xfs_attr_shortform_list(context);
	} else if (xfs_bmap_one_block(dp, XFS_ATTR_FORK)) {
		error = xfs_attr_leaf_list(context);
	} else {
		error = xfs_attr_node_list(context);
	}

	xfs_iunlock(dp, XFS_ILOCK_SHARED);

	return error;
}

#define	ATTR_ENTBASESIZE		/*                               */ \
	(((struct attrlist_ent *) 0)->a_name - (char *) 0)
#define	ATTR_ENTSIZE(namelen)		/*                              */ \
	((ATTR_ENTBASESIZE + (namelen) + 1 + sizeof(u_int32_t)-1) \
	 & ~(sizeof(u_int32_t)-1))

/*
                                                            
                                                                     
                                                        
 */
/*        */
STATIC int
xfs_attr_put_listent(
	xfs_attr_list_context_t *context,
	int		flags,
	unsigned char	*name,
	int		namelen,
	int		valuelen,
	unsigned char	*value)
{
	struct attrlist *alist = (struct attrlist *)context->alist;
	attrlist_ent_t *aep;
	int arraytop;

	ASSERT(!(context->flags & ATTR_KERNOVAL));
	ASSERT(context->count >= 0);
	ASSERT(context->count < (ATTR_MAX_VALUELEN/8));
	ASSERT(context->firstu >= sizeof(*alist));
	ASSERT(context->firstu <= context->bufsize);

	/*
                                             
  */
	if (((context->flags & ATTR_SECURE) == 0) !=
	    ((flags & XFS_ATTR_SECURE) == 0))
		return 0;
	if (((context->flags & ATTR_ROOT) == 0) !=
	    ((flags & XFS_ATTR_ROOT) == 0))
		return 0;

	arraytop = sizeof(*alist) +
			context->count * sizeof(alist->al_offset[0]);
	context->firstu -= ATTR_ENTSIZE(namelen);
	if (context->firstu < arraytop) {
		trace_xfs_attr_list_full(context);
		alist->al_more = 1;
		context->seen_enough = 1;
		return 1;
	}

	aep = (attrlist_ent_t *)&context->alist[context->firstu];
	aep->a_valuelen = valuelen;
	memcpy(aep->a_name, name, namelen);
	aep->a_name[namelen] = 0;
	alist->al_offset[context->count++] = context->firstu;
	alist->al_count = context->count;
	trace_xfs_attr_list_add(context);
	return 0;
}

/*
                                                             
                                                             
                                                             
                                                             
           
 */
int
xfs_attr_list(
	xfs_inode_t	*dp,
	char		*buffer,
	int		bufsize,
	int		flags,
	attrlist_cursor_kern_t *cursor)
{
	xfs_attr_list_context_t context;
	struct attrlist *alist;
	int error;

	/*
                        
  */
	if (cursor->pad1 || cursor->pad2)
		return(XFS_ERROR(EINVAL));
	if ((cursor->initted == 0) &&
	    (cursor->hashval || cursor->blkno || cursor->offset))
		return XFS_ERROR(EINVAL);

	/*
                                        
  */
	if (((long)buffer) & (sizeof(int)-1))
		return XFS_ERROR(EFAULT);
	if (flags & ATTR_KERNOVAL)
		bufsize = 0;

	/*
                                 
  */
	memset(&context, 0, sizeof(context));
	context.dp = dp;
	context.cursor = cursor;
	context.resynch = 1;
	context.flags = flags;
	context.alist = buffer;
	context.bufsize = (bufsize & ~(sizeof(int)-1));  /*       */
	context.firstu = context.bufsize;
	context.put_listent = xfs_attr_put_listent;

	alist = (struct attrlist *)context.alist;
	alist->al_count = 0;
	alist->al_more = 0;
	alist->al_offset[0] = context.bufsize;

	error = xfs_attr_list_int(&context);
	ASSERT(error >= 0);
	return error;
}

int								/*       */
xfs_attr_inactive(xfs_inode_t *dp)
{
	xfs_trans_t *trans;
	xfs_mount_t *mp;
	int error;

	mp = dp->i_mount;
	ASSERT(! XFS_NOT_DQATTACHED(mp, dp));

	xfs_ilock(dp, XFS_ILOCK_SHARED);
	if (!xfs_inode_hasattr(dp) ||
	    dp->i_d.di_aformat == XFS_DINODE_FMT_LOCAL) {
		xfs_iunlock(dp, XFS_ILOCK_SHARED);
		return 0;
	}
	xfs_iunlock(dp, XFS_ILOCK_SHARED);

	/*
                                           
   
                                                                  
                                                                     
                                                                      
                                                                   
                                                                 
            
  */
	trans = xfs_trans_alloc(mp, XFS_TRANS_ATTRINVAL);
	if ((error = xfs_trans_reserve(trans, 0, XFS_ATTRINVAL_LOG_RES(mp), 0,
				      XFS_TRANS_PERM_LOG_RES,
				      XFS_ATTRINVAL_LOG_COUNT))) {
		xfs_trans_cancel(trans, 0);
		return(error);
	}
	xfs_ilock(dp, XFS_ILOCK_EXCL);

	/*
                                                                      
                                             
  */
	xfs_trans_ijoin(trans, dp, 0);

	/*
                                                                 
  */
	if (!xfs_inode_hasattr(dp) ||
	    dp->i_d.di_aformat == XFS_DINODE_FMT_LOCAL) {
		error = 0;
		goto out;
	}
	error = xfs_attr3_root_inactive(&trans, dp);
	if (error)
		goto out;

	error = xfs_itruncate_extents(&trans, dp, XFS_ATTR_FORK, 0);
	if (error)
		goto out;

	error = xfs_trans_commit(trans, XFS_TRANS_RELEASE_LOG_RES);
	xfs_iunlock(dp, XFS_ILOCK_EXCL);

	return(error);

out:
	xfs_trans_cancel(trans, XFS_TRANS_RELEASE_LOG_RES|XFS_TRANS_ABORT);
	xfs_iunlock(dp, XFS_ILOCK_EXCL);
	return(error);
}



/*                                                                        
                                                            
                                                                          */

/*
                                                       
                                
 */
STATIC int
xfs_attr_shortform_addname(xfs_da_args_t *args)
{
	int newsize, forkoff, retval;

	trace_xfs_attr_sf_addname(args);

	retval = xfs_attr_shortform_lookup(args);
	if ((args->flags & ATTR_REPLACE) && (retval == ENOATTR)) {
		return(retval);
	} else if (retval == EEXIST) {
		if (args->flags & ATTR_CREATE)
			return(retval);
		retval = xfs_attr_shortform_remove(args);
		ASSERT(retval == 0);
	}

	if (args->namelen >= XFS_ATTR_SF_ENTSIZE_MAX ||
	    args->valuelen >= XFS_ATTR_SF_ENTSIZE_MAX)
		return(XFS_ERROR(ENOSPC));

	newsize = XFS_ATTR_SF_TOTSIZE(args->dp);
	newsize += XFS_ATTR_SF_ENTSIZE_BYNAME(args->namelen, args->valuelen);

	forkoff = xfs_attr_shortform_bytesfit(args->dp, newsize);
	if (!forkoff)
		return(XFS_ERROR(ENOSPC));

	xfs_attr_shortform_add(args, forkoff);
	return(0);
}


/*                                                                        
                                                     
                                                                          */

/*
                                                  
  
                                                                          
                                                                         
 */
STATIC int
xfs_attr_leaf_addname(xfs_da_args_t *args)
{
	xfs_inode_t *dp;
	struct xfs_buf *bp;
	int retval, error, committed, forkoff;

	trace_xfs_attr_leaf_addname(args);

	/*
                                                   
  */
	dp = args->dp;
	args->blkno = 0;
	error = xfs_attr3_leaf_read(args->trans, args->dp, args->blkno, -1, &bp);
	if (error)
		return error;

	/*
                                                                 
                                                                  
  */
	retval = xfs_attr3_leaf_lookup_int(bp, args);
	if ((args->flags & ATTR_REPLACE) && (retval == ENOATTR)) {
		xfs_trans_brelse(args->trans, bp);
		return retval;
	} else if (retval == EEXIST) {
		if (args->flags & ATTR_CREATE) {	/*                */
			xfs_trans_brelse(args->trans, bp);
			return retval;
		}

		trace_xfs_attr_leaf_replace(args);

		args->op_flags |= XFS_DA_OP_RENAME;	/*                  */
		args->blkno2 = args->blkno;		/*                   */
		args->index2 = args->index;
		args->rmtblkno2 = args->rmtblkno;
		args->rmtblkcnt2 = args->rmtblkcnt;
	}

	/*
                                                                 
                
  */
	retval = xfs_attr3_leaf_add(bp, args);
	if (retval == ENOSPC) {
		/*
                                                         
                                                            
                                     
   */
		xfs_bmap_init(args->flist, args->firstblock);
		error = xfs_attr3_leaf_to_node(args);
		if (!error) {
			error = xfs_bmap_finish(&args->trans, args->flist,
						&committed);
		}
		if (error) {
			ASSERT(committed);
			args->trans = NULL;
			xfs_bmap_cancel(args->flist);
			return(error);
		}

		/*
                                                                
                                                             
   */
		if (committed)
			xfs_trans_ijoin(args->trans, dp, 0);

		/*
                                                             
               
   */
		error = xfs_trans_roll(&args->trans, dp);
		if (error)
			return (error);

		/*
                                                             
   */
		error = xfs_attr_node_addname(args);
		return(error);
	}

	/*
                                                           
                                                     
  */
	error = xfs_trans_roll(&args->trans, dp);
	if (error)
		return (error);

	/*
                                                             
                                                                
                                                               
                                                        
  */
	if (args->rmtblkno > 0) {
		error = xfs_attr_rmtval_set(args);
		if (error)
			return(error);
	}

	/*
                                                             
                                                                 
                                                               
                                               
  */
	if (args->op_flags & XFS_DA_OP_RENAME) {
		/*
                                                              
                                                                
   */
		error = xfs_attr3_leaf_flipflags(args);
		if (error)
			return(error);

		/*
                                                         
                                     
   */
		args->index = args->index2;
		args->blkno = args->blkno2;
		args->rmtblkno = args->rmtblkno2;
		args->rmtblkcnt = args->rmtblkcnt2;
		if (args->rmtblkno) {
			error = xfs_attr_rmtval_remove(args);
			if (error)
				return(error);
		}

		/*
                                                      
                                                       
   */
		error = xfs_attr3_leaf_read(args->trans, args->dp, args->blkno,
					   -1, &bp);
		if (error)
			return error;

		xfs_attr3_leaf_remove(bp, args);

		/*
                                                                 
   */
		if ((forkoff = xfs_attr_shortform_allfit(bp, dp))) {
			xfs_bmap_init(args->flist, args->firstblock);
			error = xfs_attr3_leaf_to_shortform(bp, args, forkoff);
			/*                                       */
			if (!error) {
				error = xfs_bmap_finish(&args->trans,
							args->flist,
							&committed);
			}
			if (error) {
				ASSERT(committed);
				args->trans = NULL;
				xfs_bmap_cancel(args->flist);
				return(error);
			}

			/*
                                                     
                                                     
                          
    */
			if (committed)
				xfs_trans_ijoin(args->trans, dp, 0);
		}

		/*
                                                          
   */
		error = xfs_trans_roll(&args->trans, dp);

	} else if (args->rmtblkno > 0) {
		/*
                                                            
   */
		error = xfs_attr3_leaf_clearflag(args);
	}
	return error;
}

/*
                                                       
  
                                                                          
                                                                         
 */
STATIC int
xfs_attr_leaf_removename(xfs_da_args_t *args)
{
	xfs_inode_t *dp;
	struct xfs_buf *bp;
	int error, committed, forkoff;

	trace_xfs_attr_leaf_removename(args);

	/*
                         
  */
	dp = args->dp;
	args->blkno = 0;
	error = xfs_attr3_leaf_read(args->trans, args->dp, args->blkno, -1, &bp);
	if (error)
		return error;

	error = xfs_attr3_leaf_lookup_int(bp, args);
	if (error == ENOATTR) {
		xfs_trans_brelse(args->trans, bp);
		return error;
	}

	xfs_attr3_leaf_remove(bp, args);

	/*
                                                                
  */
	if ((forkoff = xfs_attr_shortform_allfit(bp, dp))) {
		xfs_bmap_init(args->flist, args->firstblock);
		error = xfs_attr3_leaf_to_shortform(bp, args, forkoff);
		/*                                       */
		if (!error) {
			error = xfs_bmap_finish(&args->trans, args->flist,
						&committed);
		}
		if (error) {
			ASSERT(committed);
			args->trans = NULL;
			xfs_bmap_cancel(args->flist);
			return error;
		}

		/*
                                                                
                                                             
   */
		if (committed)
			xfs_trans_ijoin(args->trans, dp, 0);
	}
	return 0;
}

/*
                                                     
  
                                                                          
                                                                         
 */
STATIC int
xfs_attr_leaf_get(xfs_da_args_t *args)
{
	struct xfs_buf *bp;
	int error;

	trace_xfs_attr_leaf_get(args);

	args->blkno = 0;
	error = xfs_attr3_leaf_read(args->trans, args->dp, args->blkno, -1, &bp);
	if (error)
		return error;

	error = xfs_attr3_leaf_lookup_int(bp, args);
	if (error != EEXIST)  {
		xfs_trans_brelse(args->trans, bp);
		return error;
	}
	error = xfs_attr3_leaf_getvalue(bp, args);
	xfs_trans_brelse(args->trans, bp);
	if (!error && (args->rmtblkno > 0) && !(args->flags & ATTR_KERNOVAL)) {
		error = xfs_attr_rmtval_get(args);
	}
	return error;
}

/*
                                                                        
 */
STATIC int
xfs_attr_leaf_list(xfs_attr_list_context_t *context)
{
	int error;
	struct xfs_buf *bp;

	trace_xfs_attr_leaf_list(context);

	context->cursor->blkno = 0;
	error = xfs_attr3_leaf_read(NULL, context->dp, 0, -1, &bp);
	if (error)
		return XFS_ERROR(error);

	error = xfs_attr3_leaf_list_int(bp, context);
	xfs_trans_brelse(NULL, bp);
	return XFS_ERROR(error);
}


/*                                                                        
                                                               
                                                                          */

/*
                                               
  
                                                                      
                                                                       
                                                          
  
                                                                           
                                                       
 */
STATIC int
xfs_attr_node_addname(xfs_da_args_t *args)
{
	xfs_da_state_t *state;
	xfs_da_state_blk_t *blk;
	xfs_inode_t *dp;
	xfs_mount_t *mp;
	int committed, retval, error;

	trace_xfs_attr_node_addname(args);

	/*
                                                                
  */
	dp = args->dp;
	mp = dp->i_mount;
restart:
	state = xfs_da_state_alloc();
	state->args = args;
	state->mp = mp;
	state->blocksize = state->mp->m_sb.sb_blocksize;
	state->node_ents = state->mp->m_attr_node_ents;

	/*
                                                                
                          
  */
	error = xfs_da3_node_lookup_int(state, &retval);
	if (error)
		goto out;
	blk = &state->path.blk[ state->path.active-1 ];
	ASSERT(blk->magic == XFS_ATTR_LEAF_MAGIC);
	if ((args->flags & ATTR_REPLACE) && (retval == ENOATTR)) {
		goto out;
	} else if (retval == EEXIST) {
		if (args->flags & ATTR_CREATE)
			goto out;

		trace_xfs_attr_node_replace(args);

		args->op_flags |= XFS_DA_OP_RENAME;	/*                  */
		args->blkno2 = args->blkno;		/*                   */
		args->index2 = args->index;
		args->rmtblkno2 = args->rmtblkno;
		args->rmtblkcnt2 = args->rmtblkcnt;
		args->rmtblkno = 0;
		args->rmtblkcnt = 0;
	}

	retval = xfs_attr3_leaf_add(blk->bp, state->args);
	if (retval == ENOSPC) {
		if (state->path.active == 1) {
			/*
                                               
                                                     
                         
    */
			xfs_da_state_free(state);
			xfs_bmap_init(args->flist, args->firstblock);
			error = xfs_attr3_leaf_to_node(args);
			if (!error) {
				error = xfs_bmap_finish(&args->trans,
							args->flist,
							&committed);
			}
			if (error) {
				ASSERT(committed);
				args->trans = NULL;
				xfs_bmap_cancel(args->flist);
				goto out;
			}

			/*
                                                     
                                                     
                          
    */
			if (committed)
				xfs_trans_ijoin(args->trans, dp, 0);

			/*
                                                   
                         
    */
			error = xfs_trans_roll(&args->trans, dp);
			if (error)
				goto out;

			goto restart;
		}

		/*
                                              
                                                     
                                                     
                                                      
   */
		xfs_bmap_init(args->flist, args->firstblock);
		error = xfs_da3_split(state);
		if (!error) {
			error = xfs_bmap_finish(&args->trans, args->flist,
						&committed);
		}
		if (error) {
			ASSERT(committed);
			args->trans = NULL;
			xfs_bmap_cancel(args->flist);
			goto out;
		}

		/*
                                                                
                                                             
   */
		if (committed)
			xfs_trans_ijoin(args->trans, dp, 0);
	} else {
		/*
                                               
   */
		xfs_da3_fixhashpath(state, &state->path);
	}

	/*
                                                            
                                         
  */
	xfs_da_state_free(state);
	state = NULL;

	/*
                                                              
                       
  */
	error = xfs_trans_roll(&args->trans, dp);
	if (error)
		goto out;

	/*
                                                             
                                                                
                                                               
                                                        
  */
	if (args->rmtblkno > 0) {
		error = xfs_attr_rmtval_set(args);
		if (error)
			return(error);
	}

	/*
                                                             
                                                                 
                                                               
                                               
  */
	if (args->op_flags & XFS_DA_OP_RENAME) {
		/*
                                                              
                                                                
   */
		error = xfs_attr3_leaf_flipflags(args);
		if (error)
			goto out;

		/*
                                                         
                                     
   */
		args->index = args->index2;
		args->blkno = args->blkno2;
		args->rmtblkno = args->rmtblkno2;
		args->rmtblkcnt = args->rmtblkcnt2;
		if (args->rmtblkno) {
			error = xfs_attr_rmtval_remove(args);
			if (error)
				return(error);
		}

		/*
                                                           
                                                          
                             
   */
		args->flags |= XFS_ATTR_INCOMPLETE;
		state = xfs_da_state_alloc();
		state->args = args;
		state->mp = mp;
		state->blocksize = state->mp->m_sb.sb_blocksize;
		state->node_ents = state->mp->m_attr_node_ents;
		state->inleaf = 0;
		error = xfs_da3_node_lookup_int(state, &retval);
		if (error)
			goto out;

		/*
                                                         
   */
		blk = &state->path.blk[ state->path.active-1 ];
		ASSERT(blk->magic == XFS_ATTR_LEAF_MAGIC);
		error = xfs_attr3_leaf_remove(blk->bp, args);
		xfs_da3_fixhashpath(state, &state->path);

		/*
                                                    
   */
		if (retval && (state->path.active > 1)) {
			xfs_bmap_init(args->flist, args->firstblock);
			error = xfs_da3_join(state);
			if (!error) {
				error = xfs_bmap_finish(&args->trans,
							args->flist,
							&committed);
			}
			if (error) {
				ASSERT(committed);
				args->trans = NULL;
				xfs_bmap_cancel(args->flist);
				goto out;
			}

			/*
                                                     
                                                     
                          
    */
			if (committed)
				xfs_trans_ijoin(args->trans, dp, 0);
		}

		/*
                                                  
   */
		error = xfs_trans_roll(&args->trans, dp);
		if (error)
			goto out;

	} else if (args->rmtblkno > 0) {
		/*
                                                            
   */
		error = xfs_attr3_leaf_clearflag(args);
		if (error)
			goto out;
	}
	retval = error = 0;

out:
	if (state)
		xfs_da_state_free(state);
	if (error)
		return(error);
	return(retval);
}

/*
                                              
  
                                                                    
                                                                     
                                                          
 */
STATIC int
xfs_attr_node_removename(xfs_da_args_t *args)
{
	xfs_da_state_t *state;
	xfs_da_state_blk_t *blk;
	xfs_inode_t *dp;
	struct xfs_buf *bp;
	int retval, error, committed, forkoff;

	trace_xfs_attr_node_removename(args);

	/*
                                                             
  */
	dp = args->dp;
	state = xfs_da_state_alloc();
	state->args = args;
	state->mp = dp->i_mount;
	state->blocksize = state->mp->m_sb.sb_blocksize;
	state->node_ents = state->mp->m_attr_node_ents;

	/*
                                                               
  */
	error = xfs_da3_node_lookup_int(state, &retval);
	if (error || (retval != EEXIST)) {
		if (error == 0)
			error = retval;
		goto out;
	}

	/*
                                                             
                                                                
                                                                     
  */
	blk = &state->path.blk[ state->path.active-1 ];
	ASSERT(blk->bp != NULL);
	ASSERT(blk->magic == XFS_ATTR_LEAF_MAGIC);
	if (args->rmtblkno > 0) {
		/*
                                                      
                                                        
                                                 
   */
		error = xfs_attr_fillstate(state);
		if (error)
			goto out;

		/*
                                                         
                  
   */
		error = xfs_attr3_leaf_setflag(args);
		if (error)
			goto out;
		error = xfs_attr_rmtval_remove(args);
		if (error)
			goto out;

		/*
                                                             
                          
   */
		error = xfs_attr_refillstate(state);
		if (error)
			goto out;
	}

	/*
                                                        
  */
	blk = &state->path.blk[ state->path.active-1 ];
	ASSERT(blk->magic == XFS_ATTR_LEAF_MAGIC);
	retval = xfs_attr3_leaf_remove(blk->bp, args);
	xfs_da3_fixhashpath(state, &state->path);

	/*
                                                   
  */
	if (retval && (state->path.active > 1)) {
		xfs_bmap_init(args->flist, args->firstblock);
		error = xfs_da3_join(state);
		if (!error) {
			error = xfs_bmap_finish(&args->trans, args->flist,
						&committed);
		}
		if (error) {
			ASSERT(committed);
			args->trans = NULL;
			xfs_bmap_cancel(args->flist);
			goto out;
		}

		/*
                                                                
                                                             
   */
		if (committed)
			xfs_trans_ijoin(args->trans, dp, 0);

		/*
                                                           
   */
		error = xfs_trans_roll(&args->trans, dp);
		if (error)
			goto out;
	}

	/*
                                                              
  */
	if (xfs_bmap_one_block(dp, XFS_ATTR_FORK)) {
		/*
                                                            
   */
		ASSERT(state->path.active == 1);
		ASSERT(state->path.blk[0].bp);
		state->path.blk[0].bp = NULL;

		error = xfs_attr3_leaf_read(args->trans, args->dp, 0, -1, &bp);
		if (error)
			goto out;

		if ((forkoff = xfs_attr_shortform_allfit(bp, dp))) {
			xfs_bmap_init(args->flist, args->firstblock);
			error = xfs_attr3_leaf_to_shortform(bp, args, forkoff);
			/*                                       */
			if (!error) {
				error = xfs_bmap_finish(&args->trans,
							args->flist,
							&committed);
			}
			if (error) {
				ASSERT(committed);
				args->trans = NULL;
				xfs_bmap_cancel(args->flist);
				goto out;
			}

			/*
                                                     
                                                     
                          
    */
			if (committed)
				xfs_trans_ijoin(args->trans, dp, 0);
		} else
			xfs_trans_brelse(args->trans, bp);
	}
	error = 0;

out:
	xfs_da_state_free(state);
	return(error);
}

/*
                                                                        
                                            
                                                                          
                                                                     
 */
STATIC int
xfs_attr_fillstate(xfs_da_state_t *state)
{
	xfs_da_state_path_t *path;
	xfs_da_state_blk_t *blk;
	int level;

	trace_xfs_attr_fillstate(state->args);

	/*
                                                                    
                                                 
  */
	path = &state->path;
	ASSERT((path->active >= 0) && (path->active < XFS_DA_NODE_MAXDEPTH));
	for (blk = path->blk, level = 0; level < path->active; blk++, level++) {
		if (blk->bp) {
			blk->disk_blkno = XFS_BUF_ADDR(blk->bp);
			blk->bp = NULL;
		} else {
			blk->disk_blkno = 0;
		}
	}

	/*
                                                                       
                                                    
  */
	path = &state->altpath;
	ASSERT((path->active >= 0) && (path->active < XFS_DA_NODE_MAXDEPTH));
	for (blk = path->blk, level = 0; level < path->active; blk++, level++) {
		if (blk->bp) {
			blk->disk_blkno = XFS_BUF_ADDR(blk->bp);
			blk->bp = NULL;
		} else {
			blk->disk_blkno = 0;
		}
	}

	return(0);
}

/*
                                                                      
                                         
                                                                         
                         
 */
STATIC int
xfs_attr_refillstate(xfs_da_state_t *state)
{
	xfs_da_state_path_t *path;
	xfs_da_state_blk_t *blk;
	int level, error;

	trace_xfs_attr_refillstate(state->args);

	/*
                                                                    
                                                 
  */
	path = &state->path;
	ASSERT((path->active >= 0) && (path->active < XFS_DA_NODE_MAXDEPTH));
	for (blk = path->blk, level = 0; level < path->active; blk++, level++) {
		if (blk->disk_blkno) {
			error = xfs_da3_node_read(state->args->trans,
						state->args->dp,
						blk->blkno, blk->disk_blkno,
						&blk->bp, XFS_ATTR_FORK);
			if (error)
				return(error);
		} else {
			blk->bp = NULL;
		}
	}

	/*
                                                                       
                                                    
  */
	path = &state->altpath;
	ASSERT((path->active >= 0) && (path->active < XFS_DA_NODE_MAXDEPTH));
	for (blk = path->blk, level = 0; level < path->active; blk++, level++) {
		if (blk->disk_blkno) {
			error = xfs_da3_node_read(state->args->trans,
						state->args->dp,
						blk->blkno, blk->disk_blkno,
						&blk->bp, XFS_ATTR_FORK);
			if (error)
				return(error);
		} else {
			blk->bp = NULL;
		}
	}

	return(0);
}

/*
                                               
  
                                                                         
                                                                        
                                         
 */
STATIC int
xfs_attr_node_get(xfs_da_args_t *args)
{
	xfs_da_state_t *state;
	xfs_da_state_blk_t *blk;
	int error, retval;
	int i;

	trace_xfs_attr_node_get(args);

	state = xfs_da_state_alloc();
	state->args = args;
	state->mp = args->dp->i_mount;
	state->blocksize = state->mp->m_sb.sb_blocksize;
	state->node_ents = state->mp->m_attr_node_ents;

	/*
                                                               
  */
	error = xfs_da3_node_lookup_int(state, &retval);
	if (error) {
		retval = error;
	} else if (retval == EEXIST) {
		blk = &state->path.blk[ state->path.active-1 ];
		ASSERT(blk->bp != NULL);
		ASSERT(blk->magic == XFS_ATTR_LEAF_MAGIC);

		/*
                                     
   */
		retval = xfs_attr3_leaf_getvalue(blk->bp, args);
		if (!retval && (args->rmtblkno > 0)
		    && !(args->flags & ATTR_KERNOVAL)) {
			retval = xfs_attr_rmtval_get(args);
		}
	}

	/*
                                                                
  */
	for (i = 0; i < state->path.active; i++) {
		xfs_trans_brelse(args->trans, state->path.blk[i].bp);
		state->path.blk[i].bp = NULL;
	}

	xfs_da_state_free(state);
	return(retval);
}

STATIC int							/*       */
xfs_attr_node_list(xfs_attr_list_context_t *context)
{
	attrlist_cursor_kern_t *cursor;
	xfs_attr_leafblock_t *leaf;
	xfs_da_intnode_t *node;
	struct xfs_attr3_icleaf_hdr leafhdr;
	struct xfs_da3_icnode_hdr nodehdr;
	struct xfs_da_node_entry *btree;
	int error, i;
	struct xfs_buf *bp;

	trace_xfs_attr_node_list(context);

	cursor = context->cursor;
	cursor->initted = 1;

	/*
                                                                 
                                                                   
                                 
  */
	bp = NULL;
	if (cursor->blkno > 0) {
		error = xfs_da3_node_read(NULL, context->dp, cursor->blkno, -1,
					      &bp, XFS_ATTR_FORK);
		if ((error != 0) && (error != EFSCORRUPTED))
			return(error);
		if (bp) {
			struct xfs_attr_leaf_entry *entries;

			node = bp->b_addr;
			switch (be16_to_cpu(node->hdr.info.magic)) {
			case XFS_DA_NODE_MAGIC:
			case XFS_DA3_NODE_MAGIC:
				trace_xfs_attr_list_wrong_blk(context);
				xfs_trans_brelse(NULL, bp);
				bp = NULL;
				break;
			case XFS_ATTR_LEAF_MAGIC:
			case XFS_ATTR3_LEAF_MAGIC:
				leaf = bp->b_addr;
				xfs_attr3_leaf_hdr_from_disk(&leafhdr, leaf);
				entries = xfs_attr3_leaf_entryp(leaf);
				if (cursor->hashval > be32_to_cpu(
						entries[leafhdr.count - 1].hashval)) {
					trace_xfs_attr_list_wrong_blk(context);
					xfs_trans_brelse(NULL, bp);
					bp = NULL;
				} else if (cursor->hashval <= be32_to_cpu(
						entries[0].hashval)) {
					trace_xfs_attr_list_wrong_blk(context);
					xfs_trans_brelse(NULL, bp);
					bp = NULL;
				}
				break;
			default:
				trace_xfs_attr_list_wrong_blk(context);
				xfs_trans_brelse(NULL, bp);
				bp = NULL;
			}
		}
	}

	/*
                                                                 
                                                                   
                                                                 
  */
	if (bp == NULL) {
		cursor->blkno = 0;
		for (;;) {
			__uint16_t magic;

			error = xfs_da3_node_read(NULL, context->dp,
						      cursor->blkno, -1, &bp,
						      XFS_ATTR_FORK);
			if (error)
				return(error);
			node = bp->b_addr;
			magic = be16_to_cpu(node->hdr.info.magic);
			if (magic == XFS_ATTR_LEAF_MAGIC ||
			    magic == XFS_ATTR3_LEAF_MAGIC)
				break;
			if (magic != XFS_DA_NODE_MAGIC &&
			    magic != XFS_DA3_NODE_MAGIC) {
				XFS_CORRUPTION_ERROR("xfs_attr_node_list(3)",
						     XFS_ERRLEVEL_LOW,
						     context->dp->i_mount,
						     node);
				xfs_trans_brelse(NULL, bp);
				return XFS_ERROR(EFSCORRUPTED);
			}

			xfs_da3_node_hdr_from_disk(&nodehdr, node);
			btree = xfs_da3_node_tree_p(node);
			for (i = 0; i < nodehdr.count; btree++, i++) {
				if (cursor->hashval
						<= be32_to_cpu(btree->hashval)) {
					cursor->blkno = be32_to_cpu(btree->before);
					trace_xfs_attr_list_node_descend(context,
									 btree);
					break;
				}
			}
			if (i == nodehdr.count) {
				xfs_trans_brelse(NULL, bp);
				return 0;
			}
			xfs_trans_brelse(NULL, bp);
		}
	}
	ASSERT(bp != NULL);

	/*
                                                                 
                                                                
                           
  */
	for (;;) {
		leaf = bp->b_addr;
		error = xfs_attr3_leaf_list_int(bp, context);
		if (error) {
			xfs_trans_brelse(NULL, bp);
			return error;
		}
		xfs_attr3_leaf_hdr_from_disk(&leafhdr, leaf);
		if (context->seen_enough || leafhdr.forw == 0)
			break;
		cursor->blkno = leafhdr.forw;
		xfs_trans_brelse(NULL, bp);
		error = xfs_attr3_leaf_read(NULL, context->dp, cursor->blkno, -1,
					   &bp);
		if (error)
			return error;
	}
	xfs_trans_brelse(NULL, bp);
	return 0;
}
