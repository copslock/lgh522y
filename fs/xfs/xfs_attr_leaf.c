/*
 * Copyright (c) 2000-2005 Silicon Graphics, Inc.
 * Copyright (c) 2013 Red Hat, Inc.
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
#include "xfs_alloc_btree.h"
#include "xfs_ialloc_btree.h"
#include "xfs_alloc.h"
#include "xfs_btree.h"
#include "xfs_attr_sf.h"
#include "xfs_attr_remote.h"
#include "xfs_dinode.h"
#include "xfs_inode.h"
#include "xfs_inode_item.h"
#include "xfs_bmap.h"
#include "xfs_attr.h"
#include "xfs_attr_leaf.h"
#include "xfs_error.h"
#include "xfs_trace.h"
#include "xfs_buf_item.h"
#include "xfs_cksum.h"


/*
                  
  
                                                                             
 */

/*                                                                        
                                      
                                                                          */

/*
                                       
 */
STATIC int xfs_attr3_leaf_create(struct xfs_da_args *args,
				 xfs_dablk_t which_block, struct xfs_buf **bpp);
STATIC int xfs_attr3_leaf_add_work(struct xfs_buf *leaf_buffer,
				   struct xfs_attr3_icleaf_hdr *ichdr,
				   struct xfs_da_args *args, int freemap_index);
STATIC void xfs_attr3_leaf_compact(struct xfs_da_args *args,
				   struct xfs_attr3_icleaf_hdr *ichdr,
				   struct xfs_buf *leaf_buffer);
STATIC void xfs_attr3_leaf_rebalance(xfs_da_state_t *state,
						   xfs_da_state_blk_t *blk1,
						   xfs_da_state_blk_t *blk2);
STATIC int xfs_attr3_leaf_figure_balance(xfs_da_state_t *state,
			xfs_da_state_blk_t *leaf_blk_1,
			struct xfs_attr3_icleaf_hdr *ichdr1,
			xfs_da_state_blk_t *leaf_blk_2,
			struct xfs_attr3_icleaf_hdr *ichdr2,
			int *number_entries_in_blk1,
			int *number_usedbytes_in_blk1);

/*
                                         
 */
STATIC int xfs_attr3_node_inactive(xfs_trans_t **trans, xfs_inode_t *dp,
				  struct xfs_buf *bp, int level);
STATIC int xfs_attr3_leaf_inactive(xfs_trans_t **trans, xfs_inode_t *dp,
				  struct xfs_buf *bp);
STATIC int xfs_attr3_leaf_freextent(xfs_trans_t **trans, xfs_inode_t *dp,
				   xfs_dablk_t blkno, int blkcnt);

/*
                    
 */
STATIC void xfs_attr3_leaf_moveents(struct xfs_attr_leafblock *src_leaf,
			struct xfs_attr3_icleaf_hdr *src_ichdr, int src_start,
			struct xfs_attr_leafblock *dst_leaf,
			struct xfs_attr3_icleaf_hdr *dst_ichdr, int dst_start,
			int move_count, struct xfs_mount *mp);
STATIC int xfs_attr_leaf_entsize(xfs_attr_leafblock_t *leaf, int index);

void
xfs_attr3_leaf_hdr_from_disk(
	struct xfs_attr3_icleaf_hdr	*to,
	struct xfs_attr_leafblock	*from)
{
	int	i;

	ASSERT(from->hdr.info.magic == cpu_to_be16(XFS_ATTR_LEAF_MAGIC) ||
	       from->hdr.info.magic == cpu_to_be16(XFS_ATTR3_LEAF_MAGIC));

	if (from->hdr.info.magic == cpu_to_be16(XFS_ATTR3_LEAF_MAGIC)) {
		struct xfs_attr3_leaf_hdr *hdr3 = (struct xfs_attr3_leaf_hdr *)from;

		to->forw = be32_to_cpu(hdr3->info.hdr.forw);
		to->back = be32_to_cpu(hdr3->info.hdr.back);
		to->magic = be16_to_cpu(hdr3->info.hdr.magic);
		to->count = be16_to_cpu(hdr3->count);
		to->usedbytes = be16_to_cpu(hdr3->usedbytes);
		to->firstused = be16_to_cpu(hdr3->firstused);
		to->holes = hdr3->holes;

		for (i = 0; i < XFS_ATTR_LEAF_MAPSIZE; i++) {
			to->freemap[i].base = be16_to_cpu(hdr3->freemap[i].base);
			to->freemap[i].size = be16_to_cpu(hdr3->freemap[i].size);
		}
		return;
	}
	to->forw = be32_to_cpu(from->hdr.info.forw);
	to->back = be32_to_cpu(from->hdr.info.back);
	to->magic = be16_to_cpu(from->hdr.info.magic);
	to->count = be16_to_cpu(from->hdr.count);
	to->usedbytes = be16_to_cpu(from->hdr.usedbytes);
	to->firstused = be16_to_cpu(from->hdr.firstused);
	to->holes = from->hdr.holes;

	for (i = 0; i < XFS_ATTR_LEAF_MAPSIZE; i++) {
		to->freemap[i].base = be16_to_cpu(from->hdr.freemap[i].base);
		to->freemap[i].size = be16_to_cpu(from->hdr.freemap[i].size);
	}
}

void
xfs_attr3_leaf_hdr_to_disk(
	struct xfs_attr_leafblock	*to,
	struct xfs_attr3_icleaf_hdr	*from)
{
	int	i;

	ASSERT(from->magic == XFS_ATTR_LEAF_MAGIC ||
	       from->magic == XFS_ATTR3_LEAF_MAGIC);

	if (from->magic == XFS_ATTR3_LEAF_MAGIC) {
		struct xfs_attr3_leaf_hdr *hdr3 = (struct xfs_attr3_leaf_hdr *)to;

		hdr3->info.hdr.forw = cpu_to_be32(from->forw);
		hdr3->info.hdr.back = cpu_to_be32(from->back);
		hdr3->info.hdr.magic = cpu_to_be16(from->magic);
		hdr3->count = cpu_to_be16(from->count);
		hdr3->usedbytes = cpu_to_be16(from->usedbytes);
		hdr3->firstused = cpu_to_be16(from->firstused);
		hdr3->holes = from->holes;
		hdr3->pad1 = 0;

		for (i = 0; i < XFS_ATTR_LEAF_MAPSIZE; i++) {
			hdr3->freemap[i].base = cpu_to_be16(from->freemap[i].base);
			hdr3->freemap[i].size = cpu_to_be16(from->freemap[i].size);
		}
		return;
	}
	to->hdr.info.forw = cpu_to_be32(from->forw);
	to->hdr.info.back = cpu_to_be32(from->back);
	to->hdr.info.magic = cpu_to_be16(from->magic);
	to->hdr.count = cpu_to_be16(from->count);
	to->hdr.usedbytes = cpu_to_be16(from->usedbytes);
	to->hdr.firstused = cpu_to_be16(from->firstused);
	to->hdr.holes = from->holes;
	to->hdr.pad1 = 0;

	for (i = 0; i < XFS_ATTR_LEAF_MAPSIZE; i++) {
		to->hdr.freemap[i].base = cpu_to_be16(from->freemap[i].base);
		to->hdr.freemap[i].size = cpu_to_be16(from->freemap[i].size);
	}
}

static bool
xfs_attr3_leaf_verify(
	struct xfs_buf		*bp)
{
	struct xfs_mount	*mp = bp->b_target->bt_mount;
	struct xfs_attr_leafblock *leaf = bp->b_addr;
	struct xfs_attr3_icleaf_hdr ichdr;

	xfs_attr3_leaf_hdr_from_disk(&ichdr, leaf);

	if (xfs_sb_version_hascrc(&mp->m_sb)) {
		struct xfs_da3_node_hdr *hdr3 = bp->b_addr;

		if (ichdr.magic != XFS_ATTR3_LEAF_MAGIC)
			return false;

		if (!uuid_equal(&hdr3->info.uuid, &mp->m_sb.sb_uuid))
			return false;
		if (be64_to_cpu(hdr3->info.blkno) != bp->b_bn)
			return false;
	} else {
		if (ichdr.magic != XFS_ATTR_LEAF_MAGIC)
			return false;
	}
	if (ichdr.count == 0)
		return false;

	/*                                                     */
	/*                        */

	return true;
}

static void
xfs_attr3_leaf_write_verify(
	struct xfs_buf	*bp)
{
	struct xfs_mount	*mp = bp->b_target->bt_mount;
	struct xfs_buf_log_item	*bip = bp->b_fspriv;
	struct xfs_attr3_leaf_hdr *hdr3 = bp->b_addr;

	if (!xfs_attr3_leaf_verify(bp)) {
		XFS_CORRUPTION_ERROR(__func__, XFS_ERRLEVEL_LOW, mp, bp->b_addr);
		xfs_buf_ioerror(bp, EFSCORRUPTED);
		return;
	}

	if (!xfs_sb_version_hascrc(&mp->m_sb))
		return;

	if (bip)
		hdr3->info.lsn = cpu_to_be64(bip->bli_item.li_lsn);

	xfs_update_cksum(bp->b_addr, BBTOB(bp->b_length), XFS_ATTR3_LEAF_CRC_OFF);
}

/*
                                                                                
                                                                             
                                                                               
                                  
 */
static void
xfs_attr3_leaf_read_verify(
	struct xfs_buf		*bp)
{
	struct xfs_mount	*mp = bp->b_target->bt_mount;

	if ((xfs_sb_version_hascrc(&mp->m_sb) &&
	     !xfs_verify_cksum(bp->b_addr, BBTOB(bp->b_length),
					  XFS_ATTR3_LEAF_CRC_OFF)) ||
	    !xfs_attr3_leaf_verify(bp)) {
		XFS_CORRUPTION_ERROR(__func__, XFS_ERRLEVEL_LOW, mp, bp->b_addr);
		xfs_buf_ioerror(bp, EFSCORRUPTED);
	}
}

const struct xfs_buf_ops xfs_attr3_leaf_buf_ops = {
	.verify_read = xfs_attr3_leaf_read_verify,
	.verify_write = xfs_attr3_leaf_write_verify,
};

int
xfs_attr3_leaf_read(
	struct xfs_trans	*tp,
	struct xfs_inode	*dp,
	xfs_dablk_t		bno,
	xfs_daddr_t		mappedbno,
	struct xfs_buf		**bpp)
{
	int			err;

	err = xfs_da_read_buf(tp, dp, bno, mappedbno, bpp,
				XFS_ATTR_FORK, &xfs_attr3_leaf_buf_ops);
	if (!err && tp)
		xfs_trans_buf_set_type(tp, *bpp, XFS_BLFT_ATTR_LEAF_BUF);
	return err;
}

/*                                                                        
                            
                                                                          */

/*
                                          
                              
 */
STATIC int
xfs_attr_namesp_match(int arg_flags, int ondisk_flags)
{
	return XFS_ATTR_NSP_ONDISK(ondisk_flags) == XFS_ATTR_NSP_ARGS_TO_ONDISK(arg_flags);
}


/*                                                                        
                                                               
                                                                          */

/*
                                                                     
                                              
  
                                                                         
                                                                      
  
                                                                     
                                                                     
 */
int
xfs_attr_shortform_bytesfit(xfs_inode_t *dp, int bytes)
{
	int offset;
	int minforkoff;	/*                                        */
	int maxforkoff;	/*                                        */
	int dsize;
	xfs_mount_t *mp = dp->i_mount;

	/*              */
	offset = (XFS_LITINO(mp, dp->i_d.di_version) - bytes) >> 3;

	switch (dp->i_d.di_format) {
	case XFS_DINODE_FMT_DEV:
		minforkoff = roundup(sizeof(xfs_dev_t), 8) >> 3;
		return (offset >= minforkoff) ? minforkoff : 0;
	case XFS_DINODE_FMT_UUID:
		minforkoff = roundup(sizeof(uuid_t), 8) >> 3;
		return (offset >= minforkoff) ? minforkoff : 0;
	}

	/*
                                                                
                                                      
   
                                                                     
                                                                  
                                                                     
                                                                     
                             
  */
	if (bytes <= XFS_IFORK_ASIZE(dp))
		return dp->i_d.di_forkoff;

	/*
                                                                     
                                                                   
                                      
  */
	if (!(mp->m_flags & XFS_MOUNT_ATTR2))
		return 0;

	dsize = dp->i_df.if_bytes;

	switch (dp->i_d.di_format) {
	case XFS_DINODE_FMT_EXTENTS:
		/*
                                                            
                                                            
                                                       
                                                           
                    
   */
		if (!dp->i_d.di_forkoff && dp->i_df.if_bytes >
		    xfs_default_attroffset(dp))
			dsize = XFS_BMDR_SPACE_CALC(MINDBTPTRS);
		break;
	case XFS_DINODE_FMT_BTREE:
		/*
                                                              
                                                       
                                                             
                             
   */
		if (dp->i_d.di_forkoff) {
			if (offset < dp->i_d.di_forkoff)
				return 0;
			return dp->i_d.di_forkoff;
		}
		dsize = XFS_BMAP_BROOT_SPACE(mp, dp->i_df.if_broot);
		break;
	}

	/*
                                                       
                                                                
  */
	minforkoff = MAX(dsize, XFS_BMDR_SPACE_CALC(MINDBTPTRS));
	minforkoff = roundup(minforkoff, 8) >> 3;

	/*                                                                */
	maxforkoff = XFS_LITINO(mp, dp->i_d.di_version) -
			XFS_BMDR_SPACE_CALC(MINABTPTRS);
	maxforkoff = maxforkoff >> 3;	/*              */

	if (offset >= maxforkoff)
		return maxforkoff;
	if (offset >= minforkoff)
		return offset;
	return 0;
}

/*
                                                              
 */
STATIC void
xfs_sbversion_add_attr2(xfs_mount_t *mp, xfs_trans_t *tp)
{
	if ((mp->m_flags & XFS_MOUNT_ATTR2) &&
	    !(xfs_sb_version_hasattr2(&mp->m_sb))) {
		spin_lock(&mp->m_sb_lock);
		if (!xfs_sb_version_hasattr2(&mp->m_sb)) {
			xfs_sb_version_addattr2(&mp->m_sb);
			spin_unlock(&mp->m_sb_lock);
			xfs_mod_sb(tp, XFS_SB_VERSIONNUM | XFS_SB_FEATURES2);
		} else
			spin_unlock(&mp->m_sb_lock);
	}
}

/*
                                                             
 */
void
xfs_attr_shortform_create(xfs_da_args_t *args)
{
	xfs_attr_sf_hdr_t *hdr;
	xfs_inode_t *dp;
	xfs_ifork_t *ifp;

	trace_xfs_attr_sf_create(args);

	dp = args->dp;
	ASSERT(dp != NULL);
	ifp = dp->i_afp;
	ASSERT(ifp != NULL);
	ASSERT(ifp->if_bytes == 0);
	if (dp->i_d.di_aformat == XFS_DINODE_FMT_EXTENTS) {
		ifp->if_flags &= ~XFS_IFEXTENTS;	/*              */
		dp->i_d.di_aformat = XFS_DINODE_FMT_LOCAL;
		ifp->if_flags |= XFS_IFINLINE;
	} else {
		ASSERT(ifp->if_flags & XFS_IFINLINE);
	}
	xfs_idata_realloc(dp, sizeof(*hdr), XFS_ATTR_FORK);
	hdr = (xfs_attr_sf_hdr_t *)ifp->if_u1.if_data;
	hdr->count = 0;
	hdr->totsize = cpu_to_be16(sizeof(*hdr));
	xfs_trans_log_inode(args->trans, dp, XFS_ILOG_CORE | XFS_ILOG_ADATA);
}

/*
                                                         
                                                        
 */
void
xfs_attr_shortform_add(xfs_da_args_t *args, int forkoff)
{
	xfs_attr_shortform_t *sf;
	xfs_attr_sf_entry_t *sfe;
	int i, offset, size;
	xfs_mount_t *mp;
	xfs_inode_t *dp;
	xfs_ifork_t *ifp;

	trace_xfs_attr_sf_add(args);

	dp = args->dp;
	mp = dp->i_mount;
	dp->i_d.di_forkoff = forkoff;

	ifp = dp->i_afp;
	ASSERT(ifp->if_flags & XFS_IFINLINE);
	sf = (xfs_attr_shortform_t *)ifp->if_u1.if_data;
	sfe = &sf->list[0];
	for (i = 0; i < sf->hdr.count; sfe = XFS_ATTR_SF_NEXTENTRY(sfe), i++) {
#ifdef DEBUG
		if (sfe->namelen != args->namelen)
			continue;
		if (memcmp(args->name, sfe->nameval, args->namelen) != 0)
			continue;
		if (!xfs_attr_namesp_match(args->flags, sfe->flags))
			continue;
		ASSERT(0);
#endif
	}

	offset = (char *)sfe - (char *)sf;
	size = XFS_ATTR_SF_ENTSIZE_BYNAME(args->namelen, args->valuelen);
	xfs_idata_realloc(dp, size, XFS_ATTR_FORK);
	sf = (xfs_attr_shortform_t *)ifp->if_u1.if_data;
	sfe = (xfs_attr_sf_entry_t *)((char *)sf + offset);

	sfe->namelen = args->namelen;
	sfe->valuelen = args->valuelen;
	sfe->flags = XFS_ATTR_NSP_ARGS_TO_ONDISK(args->flags);
	memcpy(sfe->nameval, args->name, args->namelen);
	memcpy(&sfe->nameval[args->namelen], args->value, args->valuelen);
	sf->hdr.count++;
	be16_add_cpu(&sf->hdr.totsize, size);
	xfs_trans_log_inode(args->trans, dp, XFS_ILOG_CORE | XFS_ILOG_ADATA);

	xfs_sbversion_add_attr2(mp, args->trans);
}

/*
                                                                       
                                                                
 */
STATIC void
xfs_attr_fork_reset(
	struct xfs_inode	*ip,
	struct xfs_trans	*tp)
{
	xfs_idestroy_fork(ip, XFS_ATTR_FORK);
	ip->i_d.di_forkoff = 0;
	ip->i_d.di_aformat = XFS_DINODE_FMT_EXTENTS;

	ASSERT(ip->i_d.di_anextents == 0);
	ASSERT(ip->i_afp == NULL);

	xfs_trans_log_inode(tp, ip, XFS_ILOG_CORE);
}

/*
                                                                   
 */
int
xfs_attr_shortform_remove(xfs_da_args_t *args)
{
	xfs_attr_shortform_t *sf;
	xfs_attr_sf_entry_t *sfe;
	int base, size=0, end, totsize, i;
	xfs_mount_t *mp;
	xfs_inode_t *dp;

	trace_xfs_attr_sf_remove(args);

	dp = args->dp;
	mp = dp->i_mount;
	base = sizeof(xfs_attr_sf_hdr_t);
	sf = (xfs_attr_shortform_t *)dp->i_afp->if_u1.if_data;
	sfe = &sf->list[0];
	end = sf->hdr.count;
	for (i = 0; i < end; sfe = XFS_ATTR_SF_NEXTENTRY(sfe),
					base += size, i++) {
		size = XFS_ATTR_SF_ENTSIZE(sfe);
		if (sfe->namelen != args->namelen)
			continue;
		if (memcmp(sfe->nameval, args->name, args->namelen) != 0)
			continue;
		if (!xfs_attr_namesp_match(args->flags, sfe->flags))
			continue;
		break;
	}
	if (i == end)
		return(XFS_ERROR(ENOATTR));

	/*
                                                     
  */
	end = base + size;
	totsize = be16_to_cpu(sf->hdr.totsize);
	if (end != totsize)
		memmove(&((char *)sf)[base], &((char *)sf)[end], totsize - end);
	sf->hdr.count--;
	be16_add_cpu(&sf->hdr.totsize, -size);

	/*
                                                 
  */
	totsize -= size;
	if (totsize == sizeof(xfs_attr_sf_hdr_t) &&
	    (mp->m_flags & XFS_MOUNT_ATTR2) &&
	    (dp->i_d.di_format != XFS_DINODE_FMT_BTREE) &&
	    !(args->op_flags & XFS_DA_OP_ADDNAME)) {
		xfs_attr_fork_reset(dp, args->trans);
	} else {
		xfs_idata_realloc(dp, -size, XFS_ATTR_FORK);
		dp->i_d.di_forkoff = xfs_attr_shortform_bytesfit(dp, totsize);
		ASSERT(dp->i_d.di_forkoff);
		ASSERT(totsize > sizeof(xfs_attr_sf_hdr_t) ||
				(args->op_flags & XFS_DA_OP_ADDNAME) ||
				!(mp->m_flags & XFS_MOUNT_ATTR2) ||
				dp->i_d.di_format == XFS_DINODE_FMT_BTREE);
		xfs_trans_log_inode(args->trans, dp,
					XFS_ILOG_CORE | XFS_ILOG_ADATA);
	}

	xfs_sbversion_add_attr2(mp, args->trans);

	return(0);
}

/*
                                                          
 */
/*        */
int
xfs_attr_shortform_lookup(xfs_da_args_t *args)
{
	xfs_attr_shortform_t *sf;
	xfs_attr_sf_entry_t *sfe;
	int i;
	xfs_ifork_t *ifp;

	trace_xfs_attr_sf_lookup(args);

	ifp = args->dp->i_afp;
	ASSERT(ifp->if_flags & XFS_IFINLINE);
	sf = (xfs_attr_shortform_t *)ifp->if_u1.if_data;
	sfe = &sf->list[0];
	for (i = 0; i < sf->hdr.count;
				sfe = XFS_ATTR_SF_NEXTENTRY(sfe), i++) {
		if (sfe->namelen != args->namelen)
			continue;
		if (memcmp(args->name, sfe->nameval, args->namelen) != 0)
			continue;
		if (!xfs_attr_namesp_match(args->flags, sfe->flags))
			continue;
		return(XFS_ERROR(EEXIST));
	}
	return(XFS_ERROR(ENOATTR));
}

/*
                                                          
 */
/*        */
int
xfs_attr_shortform_getvalue(xfs_da_args_t *args)
{
	xfs_attr_shortform_t *sf;
	xfs_attr_sf_entry_t *sfe;
	int i;

	ASSERT(args->dp->i_d.di_aformat == XFS_IFINLINE);
	sf = (xfs_attr_shortform_t *)args->dp->i_afp->if_u1.if_data;
	sfe = &sf->list[0];
	for (i = 0; i < sf->hdr.count;
				sfe = XFS_ATTR_SF_NEXTENTRY(sfe), i++) {
		if (sfe->namelen != args->namelen)
			continue;
		if (memcmp(args->name, sfe->nameval, args->namelen) != 0)
			continue;
		if (!xfs_attr_namesp_match(args->flags, sfe->flags))
			continue;
		if (args->flags & ATTR_KERNOVAL) {
			args->valuelen = sfe->valuelen;
			return(XFS_ERROR(EEXIST));
		}
		if (args->valuelen < sfe->valuelen) {
			args->valuelen = sfe->valuelen;
			return(XFS_ERROR(ERANGE));
		}
		args->valuelen = sfe->valuelen;
		memcpy(args->value, &sfe->nameval[args->namelen],
						    args->valuelen);
		return(XFS_ERROR(EEXIST));
	}
	return(XFS_ERROR(ENOATTR));
}

/*
                                                
 */
int
xfs_attr_shortform_to_leaf(xfs_da_args_t *args)
{
	xfs_inode_t *dp;
	xfs_attr_shortform_t *sf;
	xfs_attr_sf_entry_t *sfe;
	xfs_da_args_t nargs;
	char *tmpbuffer;
	int error, i, size;
	xfs_dablk_t blkno;
	struct xfs_buf *bp;
	xfs_ifork_t *ifp;

	trace_xfs_attr_sf_to_leaf(args);

	dp = args->dp;
	ifp = dp->i_afp;
	sf = (xfs_attr_shortform_t *)ifp->if_u1.if_data;
	size = be16_to_cpu(sf->hdr.totsize);
	tmpbuffer = kmem_alloc(size, KM_SLEEP);
	ASSERT(tmpbuffer != NULL);
	memcpy(tmpbuffer, ifp->if_u1.if_data, size);
	sf = (xfs_attr_shortform_t *)tmpbuffer;

	xfs_idata_realloc(dp, -size, XFS_ATTR_FORK);
	bp = NULL;
	error = xfs_da_grow_inode(args, &blkno);
	if (error) {
		/*
                                                           
                                                           
   */
		if (error == EIO)
			goto out;
		xfs_idata_realloc(dp, size, XFS_ATTR_FORK);	/*            */
		memcpy(ifp->if_u1.if_data, tmpbuffer, size);	/*         */
		goto out;
	}

	ASSERT(blkno == 0);
	error = xfs_attr3_leaf_create(args, blkno, &bp);
	if (error) {
		error = xfs_da_shrink_inode(args, 0, bp);
		bp = NULL;
		if (error)
			goto out;
		xfs_idata_realloc(dp, size, XFS_ATTR_FORK);	/*            */
		memcpy(ifp->if_u1.if_data, tmpbuffer, size);	/*         */
		goto out;
	}

	memset((char *)&nargs, 0, sizeof(nargs));
	nargs.dp = dp;
	nargs.firstblock = args->firstblock;
	nargs.flist = args->flist;
	nargs.total = args->total;
	nargs.whichfork = XFS_ATTR_FORK;
	nargs.trans = args->trans;
	nargs.op_flags = XFS_DA_OP_OKNOENT;

	sfe = &sf->list[0];
	for (i = 0; i < sf->hdr.count; i++) {
		nargs.name = sfe->nameval;
		nargs.namelen = sfe->namelen;
		nargs.value = &sfe->nameval[nargs.namelen];
		nargs.valuelen = sfe->valuelen;
		nargs.hashval = xfs_da_hashname(sfe->nameval,
						sfe->namelen);
		nargs.flags = XFS_ATTR_NSP_ONDISK_TO_ARGS(sfe->flags);
		error = xfs_attr3_leaf_lookup_int(bp, &nargs); /*              */
		ASSERT(error == ENOATTR);
		error = xfs_attr3_leaf_add(bp, &nargs);
		ASSERT(error != ENOSPC);
		if (error)
			goto out;
		sfe = XFS_ATTR_SF_NEXTENTRY(sfe);
	}
	error = 0;

out:
	kmem_free(tmpbuffer);
	return(error);
}

STATIC int
xfs_attr_shortform_compare(const void *a, const void *b)
{
	xfs_attr_sf_sort_t *sa, *sb;

	sa = (xfs_attr_sf_sort_t *)a;
	sb = (xfs_attr_sf_sort_t *)b;
	if (sa->hash < sb->hash) {
		return(-1);
	} else if (sa->hash > sb->hash) {
		return(1);
	} else {
		return(sa->entno - sb->entno);
	}
}


#define XFS_ISRESET_CURSOR(cursor) \
	(!((cursor)->initted) && !((cursor)->hashval) && \
	 !((cursor)->blkno) && !((cursor)->offset))
/*
                                                                 
                                                                    
                                                                     
                                                                    
                                           
 */
/*        */
int
xfs_attr_shortform_list(xfs_attr_list_context_t *context)
{
	attrlist_cursor_kern_t *cursor;
	xfs_attr_sf_sort_t *sbuf, *sbp;
	xfs_attr_shortform_t *sf;
	xfs_attr_sf_entry_t *sfe;
	xfs_inode_t *dp;
	int sbsize, nsbuf, count, i;
	int error;

	ASSERT(context != NULL);
	dp = context->dp;
	ASSERT(dp != NULL);
	ASSERT(dp->i_afp != NULL);
	sf = (xfs_attr_shortform_t *)dp->i_afp->if_u1.if_data;
	ASSERT(sf != NULL);
	if (!sf->hdr.count)
		return(0);
	cursor = context->cursor;
	ASSERT(cursor != NULL);

	trace_xfs_attr_list_sf(context);

	/*
                                                                 
                                                                 
                                                                 
         
                                                                  
                                                                 
                                           
  */
	if (context->bufsize == 0 ||
	    (XFS_ISRESET_CURSOR(cursor) &&
             (dp->i_afp->if_bytes + sf->hdr.count * 16) < context->bufsize)) {
		for (i = 0, sfe = &sf->list[0]; i < sf->hdr.count; i++) {
			error = context->put_listent(context,
					   sfe->flags,
					   sfe->nameval,
					   (int)sfe->namelen,
					   (int)sfe->valuelen,
					   &sfe->nameval[sfe->namelen]);

			/*
                                              
                                                
    */
			if (context->seen_enough)
				break;

			if (error)
				return error;
			sfe = XFS_ATTR_SF_NEXTENTRY(sfe);
		}
		trace_xfs_attr_list_sf_all(context);
		return(0);
	}

	/*                                  */
	if (context->bufsize == 0)
		return 0;

	/*
                                                                
  */
	sbsize = sf->hdr.count * sizeof(*sbuf);
	sbp = sbuf = kmem_alloc(sbsize, KM_SLEEP | KM_NOFS);

	/*
                                                                
                                                               
  */
	nsbuf = 0;
	for (i = 0, sfe = &sf->list[0]; i < sf->hdr.count; i++) {
		if (unlikely(
		    ((char *)sfe < (char *)sf) ||
		    ((char *)sfe >= ((char *)sf + dp->i_afp->if_bytes)))) {
			XFS_CORRUPTION_ERROR("xfs_attr_shortform_list",
					     XFS_ERRLEVEL_LOW,
					     context->dp->i_mount, sfe);
			kmem_free(sbuf);
			return XFS_ERROR(EFSCORRUPTED);
		}

		sbp->entno = i;
		sbp->hash = xfs_da_hashname(sfe->nameval, sfe->namelen);
		sbp->name = sfe->nameval;
		sbp->namelen = sfe->namelen;
		/*                                                      */
		sbp->valuelen = sfe->valuelen;
		sbp->flags = sfe->flags;
		sfe = XFS_ATTR_SF_NEXTENTRY(sfe);
		sbp++;
		nsbuf++;
	}

	/*
                                        
  */
	xfs_sort(sbuf, nsbuf, sizeof(*sbuf), xfs_attr_shortform_compare);

	/*
                                         
  */
	count = 0;
	cursor->initted = 1;
	cursor->blkno = 0;
	for (sbp = sbuf, i = 0; i < nsbuf; i++, sbp++) {
		if (sbp->hash == cursor->hashval) {
			if (cursor->offset == count) {
				break;
			}
			count++;
		} else if (sbp->hash > cursor->hashval) {
			break;
		}
	}
	if (i == nsbuf) {
		kmem_free(sbuf);
		return(0);
	}

	/*
                                              
  */
	for ( ; i < nsbuf; i++, sbp++) {
		if (cursor->hashval != sbp->hash) {
			cursor->hashval = sbp->hash;
			cursor->offset = 0;
		}
		error = context->put_listent(context,
					sbp->flags,
					sbp->name,
					sbp->namelen,
					sbp->valuelen,
					&sbp->name[sbp->namelen]);
		if (error)
			return error;
		if (context->seen_enough)
			break;
		cursor->offset++;
	}

	kmem_free(sbuf);
	return(0);
}

/*
                                                                        
                              
 */
int
xfs_attr_shortform_allfit(
	struct xfs_buf		*bp,
	struct xfs_inode	*dp)
{
	struct xfs_attr_leafblock *leaf;
	struct xfs_attr_leaf_entry *entry;
	xfs_attr_leaf_name_local_t *name_loc;
	struct xfs_attr3_icleaf_hdr leafhdr;
	int			bytes;
	int			i;

	leaf = bp->b_addr;
	xfs_attr3_leaf_hdr_from_disk(&leafhdr, leaf);
	entry = xfs_attr3_leaf_entryp(leaf);

	bytes = sizeof(struct xfs_attr_sf_hdr);
	for (i = 0; i < leafhdr.count; entry++, i++) {
		if (entry->flags & XFS_ATTR_INCOMPLETE)
			continue;		/*                            */
		if (!(entry->flags & XFS_ATTR_LOCAL))
			return(0);
		name_loc = xfs_attr3_leaf_name_local(leaf, i);
		if (name_loc->namelen >= XFS_ATTR_SF_ENTSIZE_MAX)
			return(0);
		if (be16_to_cpu(name_loc->valuelen) >= XFS_ATTR_SF_ENTSIZE_MAX)
			return(0);
		bytes += sizeof(struct xfs_attr_sf_entry) - 1
				+ name_loc->namelen
				+ be16_to_cpu(name_loc->valuelen);
	}
	if ((dp->i_mount->m_flags & XFS_MOUNT_ATTR2) &&
	    (dp->i_d.di_format != XFS_DINODE_FMT_BTREE) &&
	    (bytes == sizeof(struct xfs_attr_sf_hdr)))
		return -1;
	return xfs_attr_shortform_bytesfit(dp, bytes);
}

/*
                                                            
 */
int
xfs_attr3_leaf_to_shortform(
	struct xfs_buf		*bp,
	struct xfs_da_args	*args,
	int			forkoff)
{
	struct xfs_attr_leafblock *leaf;
	struct xfs_attr3_icleaf_hdr ichdr;
	struct xfs_attr_leaf_entry *entry;
	struct xfs_attr_leaf_name_local *name_loc;
	struct xfs_da_args	nargs;
	struct xfs_inode	*dp = args->dp;
	char			*tmpbuffer;
	int			error;
	int			i;

	trace_xfs_attr_leaf_to_sf(args);

	tmpbuffer = kmem_alloc(XFS_LBSIZE(dp->i_mount), KM_SLEEP);
	if (!tmpbuffer)
		return ENOMEM;

	memcpy(tmpbuffer, bp->b_addr, XFS_LBSIZE(dp->i_mount));

	leaf = (xfs_attr_leafblock_t *)tmpbuffer;
	xfs_attr3_leaf_hdr_from_disk(&ichdr, leaf);
	entry = xfs_attr3_leaf_entryp(leaf);

	/*                                                              */
	memset(bp->b_addr, 0, XFS_LBSIZE(dp->i_mount));

	/*
                                                       
  */
	error = xfs_da_shrink_inode(args, 0, bp);
	if (error)
		goto out;

	if (forkoff == -1) {
		ASSERT(dp->i_mount->m_flags & XFS_MOUNT_ATTR2);
		ASSERT(dp->i_d.di_format != XFS_DINODE_FMT_BTREE);
		xfs_attr_fork_reset(dp, args->trans);
		goto out;
	}

	xfs_attr_shortform_create(args);

	/*
                       
  */
	memset((char *)&nargs, 0, sizeof(nargs));
	nargs.dp = dp;
	nargs.firstblock = args->firstblock;
	nargs.flist = args->flist;
	nargs.total = args->total;
	nargs.whichfork = XFS_ATTR_FORK;
	nargs.trans = args->trans;
	nargs.op_flags = XFS_DA_OP_OKNOENT;

	for (i = 0; i < ichdr.count; entry++, i++) {
		if (entry->flags & XFS_ATTR_INCOMPLETE)
			continue;	/*                            */
		if (!entry->nameidx)
			continue;
		ASSERT(entry->flags & XFS_ATTR_LOCAL);
		name_loc = xfs_attr3_leaf_name_local(leaf, i);
		nargs.name = name_loc->nameval;
		nargs.namelen = name_loc->namelen;
		nargs.value = &name_loc->nameval[nargs.namelen];
		nargs.valuelen = be16_to_cpu(name_loc->valuelen);
		nargs.hashval = be32_to_cpu(entry->hashval);
		nargs.flags = XFS_ATTR_NSP_ONDISK_TO_ARGS(entry->flags);
		xfs_attr_shortform_add(&nargs, forkoff);
	}
	error = 0;

out:
	kmem_free(tmpbuffer);
	return error;
}

/*
                                                              
 */
int
xfs_attr3_leaf_to_node(
	struct xfs_da_args	*args)
{
	struct xfs_attr_leafblock *leaf;
	struct xfs_attr3_icleaf_hdr icleafhdr;
	struct xfs_attr_leaf_entry *entries;
	struct xfs_da_node_entry *btree;
	struct xfs_da3_icnode_hdr icnodehdr;
	struct xfs_da_intnode	*node;
	struct xfs_inode	*dp = args->dp;
	struct xfs_mount	*mp = dp->i_mount;
	struct xfs_buf		*bp1 = NULL;
	struct xfs_buf		*bp2 = NULL;
	xfs_dablk_t		blkno;
	int			error;

	trace_xfs_attr_leaf_to_node(args);

	error = xfs_da_grow_inode(args, &blkno);
	if (error)
		goto out;
	error = xfs_attr3_leaf_read(args->trans, dp, 0, -1, &bp1);
	if (error)
		goto out;

	error = xfs_da_get_buf(args->trans, dp, blkno, -1, &bp2, XFS_ATTR_FORK);
	if (error)
		goto out;

	/*                                             */
	xfs_trans_buf_set_type(args->trans, bp2, XFS_BLFT_ATTR_LEAF_BUF);
	bp2->b_ops = bp1->b_ops;
	memcpy(bp2->b_addr, bp1->b_addr, XFS_LBSIZE(mp));
	if (xfs_sb_version_hascrc(&mp->m_sb)) {
		struct xfs_da3_blkinfo *hdr3 = bp2->b_addr;
		hdr3->blkno = cpu_to_be64(bp2->b_bn);
	}
	xfs_trans_log_buf(args->trans, bp2, 0, XFS_LBSIZE(mp) - 1);

	/*
                             
  */
	error = xfs_da3_node_create(args, 0, 1, &bp1, XFS_ATTR_FORK);
	if (error)
		goto out;
	node = bp1->b_addr;
	xfs_da3_node_hdr_from_disk(&icnodehdr, node);
	btree = xfs_da3_node_tree_p(node);

	leaf = bp2->b_addr;
	xfs_attr3_leaf_hdr_from_disk(&icleafhdr, leaf);
	entries = xfs_attr3_leaf_entryp(leaf);

	/*                                       */
	btree[0].hashval = entries[icleafhdr.count - 1].hashval;
	btree[0].before = cpu_to_be32(blkno);
	icnodehdr.count = 1;
	xfs_da3_node_hdr_to_disk(node, &icnodehdr);
	xfs_trans_log_buf(args->trans, bp1, 0, XFS_LBSIZE(mp) - 1);
	error = 0;
out:
	return error;
}


/*                                                                        
                                       
                                                                          */

/*
                                                       
                                      
 */
STATIC int
xfs_attr3_leaf_create(
	struct xfs_da_args	*args,
	xfs_dablk_t		blkno,
	struct xfs_buf		**bpp)
{
	struct xfs_attr_leafblock *leaf;
	struct xfs_attr3_icleaf_hdr ichdr;
	struct xfs_inode	*dp = args->dp;
	struct xfs_mount	*mp = dp->i_mount;
	struct xfs_buf		*bp;
	int			error;

	trace_xfs_attr_leaf_create(args);

	error = xfs_da_get_buf(args->trans, args->dp, blkno, -1, &bp,
					    XFS_ATTR_FORK);
	if (error)
		return error;
	bp->b_ops = &xfs_attr3_leaf_buf_ops;
	xfs_trans_buf_set_type(args->trans, bp, XFS_BLFT_ATTR_LEAF_BUF);
	leaf = bp->b_addr;
	memset(leaf, 0, XFS_LBSIZE(mp));

	memset(&ichdr, 0, sizeof(ichdr));
	ichdr.firstused = XFS_LBSIZE(mp);

	if (xfs_sb_version_hascrc(&mp->m_sb)) {
		struct xfs_da3_blkinfo *hdr3 = bp->b_addr;

		ichdr.magic = XFS_ATTR3_LEAF_MAGIC;

		hdr3->blkno = cpu_to_be64(bp->b_bn);
		hdr3->owner = cpu_to_be64(dp->i_ino);
		uuid_copy(&hdr3->uuid, &mp->m_sb.sb_uuid);

		ichdr.freemap[0].base = sizeof(struct xfs_attr3_leaf_hdr);
	} else {
		ichdr.magic = XFS_ATTR_LEAF_MAGIC;
		ichdr.freemap[0].base = sizeof(struct xfs_attr_leaf_hdr);
	}
	ichdr.freemap[0].size = ichdr.firstused - ichdr.freemap[0].base;

	xfs_attr3_leaf_hdr_to_disk(leaf, &ichdr);
	xfs_trans_log_buf(args->trans, bp, 0, XFS_LBSIZE(mp) - 1);

	*bpp = bp;
	return 0;
}

/*
                                                          
 */
int
xfs_attr3_leaf_split(
	struct xfs_da_state	*state,
	struct xfs_da_state_blk	*oldblk,
	struct xfs_da_state_blk	*newblk)
{
	xfs_dablk_t blkno;
	int error;

	trace_xfs_attr_leaf_split(state->args);

	/*
                                       
  */
	ASSERT(oldblk->magic == XFS_ATTR_LEAF_MAGIC);
	error = xfs_da_grow_inode(state->args, &blkno);
	if (error)
		return(error);
	error = xfs_attr3_leaf_create(state->args, blkno, &newblk->bp);
	if (error)
		return(error);
	newblk->blkno = blkno;
	newblk->magic = XFS_ATTR_LEAF_MAGIC;

	/*
                                                
                                                                     
  */
	xfs_attr3_leaf_rebalance(state, oldblk, newblk);
	error = xfs_da3_blk_link(state, oldblk, newblk);
	if (error)
		return(error);

	/*
                                                                    
                                                                
                                                                   
   
                                                
  */
	if (state->inleaf) {
		trace_xfs_attr_leaf_add_old(state->args);
		error = xfs_attr3_leaf_add(oldblk->bp, state->args);
	} else {
		trace_xfs_attr_leaf_add_new(state->args);
		error = xfs_attr3_leaf_add(newblk->bp, state->args);
	}

	/*
                                                              
  */
	oldblk->hashval = xfs_attr_leaf_lasthash(oldblk->bp, NULL);
	newblk->hashval = xfs_attr_leaf_lasthash(newblk->bp, NULL);
	return(error);
}

/*
                                                   
 */
int
xfs_attr3_leaf_add(
	struct xfs_buf		*bp,
	struct xfs_da_args	*args)
{
	struct xfs_attr_leafblock *leaf;
	struct xfs_attr3_icleaf_hdr ichdr;
	int			tablesize;
	int			entsize;
	int			sum;
	int			tmp;
	int			i;

	trace_xfs_attr_leaf_add(args);

	leaf = bp->b_addr;
	xfs_attr3_leaf_hdr_from_disk(&ichdr, leaf);
	ASSERT(args->index >= 0 && args->index <= ichdr.count);
	entsize = xfs_attr_leaf_newentsize(args->namelen, args->valuelen,
			   args->trans->t_mountp->m_sb.sb_blocksize, NULL);

	/*
                                                            
                                                    
  */
	tablesize = (ichdr.count + 1) * sizeof(xfs_attr_leaf_entry_t)
					+ xfs_attr3_leaf_hdr_size(leaf);
	for (sum = 0, i = XFS_ATTR_LEAF_MAPSIZE - 1; i >= 0; i--) {
		if (tablesize > ichdr.firstused) {
			sum += ichdr.freemap[i].size;
			continue;
		}
		if (!ichdr.freemap[i].size)
			continue;	/*                      */
		tmp = entsize;
		if (ichdr.freemap[i].base < ichdr.firstused)
			tmp += sizeof(xfs_attr_leaf_entry_t);
		if (ichdr.freemap[i].size >= tmp) {
			tmp = xfs_attr3_leaf_add_work(bp, &ichdr, args, i);
			goto out_log_hdr;
		}
		sum += ichdr.freemap[i].size;
	}

	/*
                                                            
                                                                  
                                       
  */
	if (!ichdr.holes && sum < entsize)
		return XFS_ERROR(ENOSPC);

	/*
                                               
                                                                   
  */
	xfs_attr3_leaf_compact(args, &ichdr, bp);

	/*
                                                              
                                                                  
  */
	if (ichdr.freemap[0].size < (entsize + sizeof(xfs_attr_leaf_entry_t))) {
		tmp = ENOSPC;
		goto out_log_hdr;
	}

	tmp = xfs_attr3_leaf_add_work(bp, &ichdr, args, 0);

out_log_hdr:
	xfs_attr3_leaf_hdr_to_disk(leaf, &ichdr);
	xfs_trans_log_buf(args->trans, bp,
		XFS_DA_LOGRANGE(leaf, &leaf->hdr,
				xfs_attr3_leaf_hdr_size(leaf)));
	return tmp;
}

/*
                                                 
 */
STATIC int
xfs_attr3_leaf_add_work(
	struct xfs_buf		*bp,
	struct xfs_attr3_icleaf_hdr *ichdr,
	struct xfs_da_args	*args,
	int			mapindex)
{
	struct xfs_attr_leafblock *leaf;
	struct xfs_attr_leaf_entry *entry;
	struct xfs_attr_leaf_name_local *name_loc;
	struct xfs_attr_leaf_name_remote *name_rmt;
	struct xfs_mount	*mp;
	int			tmp;
	int			i;

	trace_xfs_attr_leaf_add_work(args);

	leaf = bp->b_addr;
	ASSERT(mapindex >= 0 && mapindex < XFS_ATTR_LEAF_MAPSIZE);
	ASSERT(args->index >= 0 && args->index <= ichdr->count);

	/*
                                                            
  */
	entry = &xfs_attr3_leaf_entryp(leaf)[args->index];
	if (args->index < ichdr->count) {
		tmp  = ichdr->count - args->index;
		tmp *= sizeof(xfs_attr_leaf_entry_t);
		memmove(entry + 1, entry, tmp);
		xfs_trans_log_buf(args->trans, bp,
		    XFS_DA_LOGRANGE(leaf, entry, tmp + sizeof(*entry)));
	}
	ichdr->count++;

	/*
                                                              
  */
	mp = args->trans->t_mountp;
	ASSERT(ichdr->freemap[mapindex].base < XFS_LBSIZE(mp));
	ASSERT((ichdr->freemap[mapindex].base & 0x3) == 0);
	ASSERT(ichdr->freemap[mapindex].size >=
		xfs_attr_leaf_newentsize(args->namelen, args->valuelen,
					 mp->m_sb.sb_blocksize, NULL));
	ASSERT(ichdr->freemap[mapindex].size < XFS_LBSIZE(mp));
	ASSERT((ichdr->freemap[mapindex].size & 0x3) == 0);

	ichdr->freemap[mapindex].size -=
			xfs_attr_leaf_newentsize(args->namelen, args->valuelen,
						 mp->m_sb.sb_blocksize, &tmp);

	entry->nameidx = cpu_to_be16(ichdr->freemap[mapindex].base +
				     ichdr->freemap[mapindex].size);
	entry->hashval = cpu_to_be32(args->hashval);
	entry->flags = tmp ? XFS_ATTR_LOCAL : 0;
	entry->flags |= XFS_ATTR_NSP_ARGS_TO_ONDISK(args->flags);
	if (args->op_flags & XFS_DA_OP_RENAME) {
		entry->flags |= XFS_ATTR_INCOMPLETE;
		if ((args->blkno2 == args->blkno) &&
		    (args->index2 <= args->index)) {
			args->index2++;
		}
	}
	xfs_trans_log_buf(args->trans, bp,
			  XFS_DA_LOGRANGE(leaf, entry, sizeof(*entry)));
	ASSERT((args->index == 0) ||
	       (be32_to_cpu(entry->hashval) >= be32_to_cpu((entry-1)->hashval)));
	ASSERT((args->index == ichdr->count - 1) ||
	       (be32_to_cpu(entry->hashval) <= be32_to_cpu((entry+1)->hashval)));

	/*
                                                              
                                                             
                                                                 
                                                                
                                                                
  */
	if (entry->flags & XFS_ATTR_LOCAL) {
		name_loc = xfs_attr3_leaf_name_local(leaf, args->index);
		name_loc->namelen = args->namelen;
		name_loc->valuelen = cpu_to_be16(args->valuelen);
		memcpy((char *)name_loc->nameval, args->name, args->namelen);
		memcpy((char *)&name_loc->nameval[args->namelen], args->value,
				   be16_to_cpu(name_loc->valuelen));
	} else {
		name_rmt = xfs_attr3_leaf_name_remote(leaf, args->index);
		name_rmt->namelen = args->namelen;
		memcpy((char *)name_rmt->name, args->name, args->namelen);
		entry->flags |= XFS_ATTR_INCOMPLETE;
		/*              */
		name_rmt->valuelen = 0;
		name_rmt->valueblk = 0;
		args->rmtblkno = 1;
		args->rmtblkcnt = xfs_attr3_rmt_blocks(mp, args->valuelen);
	}
	xfs_trans_log_buf(args->trans, bp,
	     XFS_DA_LOGRANGE(leaf, xfs_attr3_leaf_name(leaf, args->index),
				   xfs_attr_leaf_entsize(leaf, args->index)));

	/*
                                              
  */
	if (be16_to_cpu(entry->nameidx) < ichdr->firstused)
		ichdr->firstused = be16_to_cpu(entry->nameidx);

	ASSERT(ichdr->firstused >= ichdr->count * sizeof(xfs_attr_leaf_entry_t)
					+ xfs_attr3_leaf_hdr_size(leaf));
	tmp = (ichdr->count - 1) * sizeof(xfs_attr_leaf_entry_t)
					+ xfs_attr3_leaf_hdr_size(leaf);

	for (i = 0; i < XFS_ATTR_LEAF_MAPSIZE; i++) {
		if (ichdr->freemap[i].base == tmp) {
			ichdr->freemap[i].base += sizeof(xfs_attr_leaf_entry_t);
			ichdr->freemap[i].size -= sizeof(xfs_attr_leaf_entry_t);
		}
	}
	ichdr->usedbytes += xfs_attr_leaf_entsize(leaf, args->index);
	return 0;
}

/*
                                                                             
 */
STATIC void
xfs_attr3_leaf_compact(
	struct xfs_da_args	*args,
	struct xfs_attr3_icleaf_hdr *ichdr_dst,
	struct xfs_buf		*bp)
{
	struct xfs_attr_leafblock *leaf_src;
	struct xfs_attr_leafblock *leaf_dst;
	struct xfs_attr3_icleaf_hdr ichdr_src;
	struct xfs_trans	*trans = args->trans;
	struct xfs_mount	*mp = trans->t_mountp;
	char			*tmpbuffer;

	trace_xfs_attr_leaf_compact(args);

	tmpbuffer = kmem_alloc(XFS_LBSIZE(mp), KM_SLEEP);
	memcpy(tmpbuffer, bp->b_addr, XFS_LBSIZE(mp));
	memset(bp->b_addr, 0, XFS_LBSIZE(mp));
	leaf_src = (xfs_attr_leafblock_t *)tmpbuffer;
	leaf_dst = bp->b_addr;

	/*
                                                                      
                                                                    
                                  
  */
	memcpy(bp->b_addr, tmpbuffer, xfs_attr3_leaf_hdr_size(leaf_src));

	/*                               */
	ichdr_src = *ichdr_dst;	/*             */
	ichdr_dst->firstused = XFS_LBSIZE(mp);
	ichdr_dst->usedbytes = 0;
	ichdr_dst->count = 0;
	ichdr_dst->holes = 0;
	ichdr_dst->freemap[0].base = xfs_attr3_leaf_hdr_size(leaf_src);
	ichdr_dst->freemap[0].size = ichdr_dst->firstused -
						ichdr_dst->freemap[0].base;


	/*                                                           */
	xfs_attr3_leaf_hdr_to_disk(leaf_dst, ichdr_dst);

	/*
                                                
                                                         
  */
	xfs_attr3_leaf_moveents(leaf_src, &ichdr_src, 0, leaf_dst, ichdr_dst, 0,
				ichdr_src.count, mp);
	/*
                                                                     
                                                        
  */
	xfs_trans_log_buf(trans, bp, 0, XFS_LBSIZE(mp) - 1);

	kmem_free(tmpbuffer);
}

/*
                                   
                                                
 */
static int
xfs_attr3_leaf_order(
	struct xfs_buf	*leaf1_bp,
	struct xfs_attr3_icleaf_hdr *leaf1hdr,
	struct xfs_buf	*leaf2_bp,
	struct xfs_attr3_icleaf_hdr *leaf2hdr)
{
	struct xfs_attr_leaf_entry *entries1;
	struct xfs_attr_leaf_entry *entries2;

	entries1 = xfs_attr3_leaf_entryp(leaf1_bp->b_addr);
	entries2 = xfs_attr3_leaf_entryp(leaf2_bp->b_addr);
	if (leaf1hdr->count > 0 && leaf2hdr->count > 0 &&
	    ((be32_to_cpu(entries2[0].hashval) <
	      be32_to_cpu(entries1[0].hashval)) ||
	     (be32_to_cpu(entries2[leaf2hdr->count - 1].hashval) <
	      be32_to_cpu(entries1[leaf1hdr->count - 1].hashval)))) {
		return 1;
	}
	return 0;
}

int
xfs_attr_leaf_order(
	struct xfs_buf	*leaf1_bp,
	struct xfs_buf	*leaf2_bp)
{
	struct xfs_attr3_icleaf_hdr ichdr1;
	struct xfs_attr3_icleaf_hdr ichdr2;

	xfs_attr3_leaf_hdr_from_disk(&ichdr1, leaf1_bp->b_addr);
	xfs_attr3_leaf_hdr_from_disk(&ichdr2, leaf2_bp->b_addr);
	return xfs_attr3_leaf_order(leaf1_bp, &ichdr1, leaf2_bp, &ichdr2);
}

/*
                                                                  
                                                 
  
                                                                      
                                                                           
  
                                                                         
                                                                          
                                                                          
                                                             
 */
STATIC void
xfs_attr3_leaf_rebalance(
	struct xfs_da_state	*state,
	struct xfs_da_state_blk	*blk1,
	struct xfs_da_state_blk	*blk2)
{
	struct xfs_da_args	*args;
	struct xfs_attr_leafblock *leaf1;
	struct xfs_attr_leafblock *leaf2;
	struct xfs_attr3_icleaf_hdr ichdr1;
	struct xfs_attr3_icleaf_hdr ichdr2;
	struct xfs_attr_leaf_entry *entries1;
	struct xfs_attr_leaf_entry *entries2;
	int			count;
	int			totallen;
	int			max;
	int			space;
	int			swap;

	/*
                       
  */
	ASSERT(blk1->magic == XFS_ATTR_LEAF_MAGIC);
	ASSERT(blk2->magic == XFS_ATTR_LEAF_MAGIC);
	leaf1 = blk1->bp->b_addr;
	leaf2 = blk2->bp->b_addr;
	xfs_attr3_leaf_hdr_from_disk(&ichdr1, leaf1);
	xfs_attr3_leaf_hdr_from_disk(&ichdr2, leaf2);
	ASSERT(ichdr2.count == 0);
	args = state->args;

	trace_xfs_attr_leaf_rebalance(args);

	/*
                                                                 
   
                                                           
                                                    
  */
	swap = 0;
	if (xfs_attr3_leaf_order(blk1->bp, &ichdr1, blk2->bp, &ichdr2)) {
		struct xfs_da_state_blk	*tmp_blk;
		struct xfs_attr3_icleaf_hdr tmp_ichdr;

		tmp_blk = blk1;
		blk1 = blk2;
		blk2 = tmp_blk;

		/*                                                     */
		tmp_ichdr = ichdr1;
		ichdr1 = ichdr2;
		ichdr2 = tmp_ichdr;

		leaf1 = blk1->bp->b_addr;
		leaf2 = blk2->bp->b_addr;
		swap = 1;
	}

	/*
                                                              
                                                             
                                                             
   
                                                                   
                                                               
  */
	state->inleaf = xfs_attr3_leaf_figure_balance(state, blk1, &ichdr1,
						      blk2, &ichdr2,
						      &count, &totallen);
	if (swap)
		state->inleaf = !state->inleaf;

	/*
                                                
  */
	if (count < ichdr1.count) {
		/*
                                                                
   */
		/*                            */
		count = ichdr1.count - count;
		space  = ichdr1.usedbytes - totallen;
		space += count * sizeof(xfs_attr_leaf_entry_t);

		/*
                                                            
   */
		max  = ichdr2.firstused - xfs_attr3_leaf_hdr_size(leaf1);
		max -= ichdr2.count * sizeof(xfs_attr_leaf_entry_t);
		if (space > max)
			xfs_attr3_leaf_compact(args, &ichdr2, blk2->bp);

		/*
                                                      
   */
		xfs_attr3_leaf_moveents(leaf1, &ichdr1, ichdr1.count - count,
				leaf2, &ichdr2, 0, count, state->mp);

	} else if (count > ichdr1.count) {
		/*
                                                     
                                                   
   */
		ASSERT(0);

		/*
                                                                
   */
		/*                            */
		count -= ichdr1.count;
		space  = totallen - ichdr1.usedbytes;
		space += count * sizeof(xfs_attr_leaf_entry_t);

		/*
                                                            
   */
		max  = ichdr1.firstused - xfs_attr3_leaf_hdr_size(leaf1);
		max -= ichdr1.count * sizeof(xfs_attr_leaf_entry_t);
		if (space > max)
			xfs_attr3_leaf_compact(args, &ichdr1, blk1->bp);

		/*
                                                      
   */
		xfs_attr3_leaf_moveents(leaf2, &ichdr2, 0, leaf1, &ichdr1,
					ichdr1.count, count, state->mp);
	}

	xfs_attr3_leaf_hdr_to_disk(leaf1, &ichdr1);
	xfs_attr3_leaf_hdr_to_disk(leaf2, &ichdr2);
	xfs_trans_log_buf(args->trans, blk1->bp, 0, state->blocksize-1);
	xfs_trans_log_buf(args->trans, blk2->bp, 0, state->blocksize-1);

	/*
                                                        
  */
	entries1 = xfs_attr3_leaf_entryp(leaf1);
	entries2 = xfs_attr3_leaf_entryp(leaf2);
	blk1->hashval = be32_to_cpu(entries1[ichdr1.count - 1].hashval);
	blk2->hashval = be32_to_cpu(entries2[ichdr2.count - 1].hashval);

	/*
                                            
                                                               
                                      
   
                                                                 
                                                               
                                                                 
                                                                
                                                                
                                                            
  */
	if (blk1->index > ichdr1.count) {
		ASSERT(state->inleaf == 0);
		blk2->index = blk1->index - ichdr1.count;
		args->index = args->index2 = blk2->index;
		args->blkno = args->blkno2 = blk2->blkno;
	} else if (blk1->index == ichdr1.count) {
		if (state->inleaf) {
			args->index = blk1->index;
			args->blkno = blk1->blkno;
			args->index2 = 0;
			args->blkno2 = blk2->blkno;
		} else {
			/*
                                                        
                                                  
                                                
    */
			blk2->index = blk1->index - ichdr1.count;
			args->index = blk2->index;
			args->blkno = blk2->blkno;
			if (!state->extravalid) {
				/*
                                                 
                                              
                                            
     */
				args->index2 = blk2->index;
				args->blkno2 = blk2->blkno;
			}
		}
	} else {
		ASSERT(state->inleaf == 1);
		args->index = args->index2 = blk1->index;
		args->blkno = args->blkno2 = blk1->blkno;
	}
}

/*
                                                             
                                                  
                                                                         
                                                                          
                                         
 */
STATIC int
xfs_attr3_leaf_figure_balance(
	struct xfs_da_state		*state,
	struct xfs_da_state_blk		*blk1,
	struct xfs_attr3_icleaf_hdr	*ichdr1,
	struct xfs_da_state_blk		*blk2,
	struct xfs_attr3_icleaf_hdr	*ichdr2,
	int				*countarg,
	int				*usedbytesarg)
{
	struct xfs_attr_leafblock	*leaf1 = blk1->bp->b_addr;
	struct xfs_attr_leafblock	*leaf2 = blk2->bp->b_addr;
	struct xfs_attr_leaf_entry	*entry;
	int				count;
	int				max;
	int				index;
	int				totallen = 0;
	int				half;
	int				lastdelta;
	int				foundit = 0;
	int				tmp;

	/*
                                                              
                                                   
  */
	max = ichdr1->count + ichdr2->count;
	half = (max + 1) * sizeof(*entry);
	half += ichdr1->usedbytes + ichdr2->usedbytes +
			xfs_attr_leaf_newentsize(state->args->namelen,
						 state->args->valuelen,
						 state->blocksize, NULL);
	half /= 2;
	lastdelta = state->blocksize;
	entry = xfs_attr3_leaf_entryp(leaf1);
	for (count = index = 0; count < max; entry++, index++, count++) {

#define XFS_ATTR_ABS(A)	(((A) < 0) ? -(A) : (A))
		/*
                                                         
   */
		if (count == blk1->index) {
			tmp = totallen + sizeof(*entry) +
				xfs_attr_leaf_newentsize(
						state->args->namelen,
						state->args->valuelen,
						state->blocksize, NULL);
			if (XFS_ATTR_ABS(half - tmp) > lastdelta)
				break;
			lastdelta = XFS_ATTR_ABS(half - tmp);
			totallen = tmp;
			foundit = 1;
		}

		/*
                                                    
   */
		if (count == ichdr1->count) {
			leaf1 = leaf2;
			entry = xfs_attr3_leaf_entryp(leaf1);
			index = 0;
		}

		/*
                                                     
   */
		tmp = totallen + sizeof(*entry) + xfs_attr_leaf_entsize(leaf1,
									index);
		if (XFS_ATTR_ABS(half - tmp) > lastdelta)
			break;
		lastdelta = XFS_ATTR_ABS(half - tmp);
		totallen = tmp;
#undef XFS_ATTR_ABS
	}

	/*
                                                                      
                                                      
  */
	totallen -= count * sizeof(*entry);
	if (foundit) {
		totallen -= sizeof(*entry) +
				xfs_attr_leaf_newentsize(
						state->args->namelen,
						state->args->valuelen,
						state->blocksize, NULL);
	}

	*countarg = count;
	*usedbytesarg = totallen;
	return foundit;
}

/*                                                                        
                                         
                                                                          */

/*
                                                                     
                                                                   
                                 
                                                                         
                                                                   
                                                                    
                                    
  
                                                     
 */
int
xfs_attr3_leaf_toosmall(
	struct xfs_da_state	*state,
	int			*action)
{
	struct xfs_attr_leafblock *leaf;
	struct xfs_da_state_blk	*blk;
	struct xfs_attr3_icleaf_hdr ichdr;
	struct xfs_buf		*bp;
	xfs_dablk_t		blkno;
	int			bytes;
	int			forward;
	int			error;
	int			retval;
	int			i;

	trace_xfs_attr_leaf_toosmall(state->args);

	/*
                                                                   
                                                                 
                               
  */
	blk = &state->path.blk[ state->path.active-1 ];
	leaf = blk->bp->b_addr;
	xfs_attr3_leaf_hdr_from_disk(&ichdr, leaf);
	bytes = xfs_attr3_leaf_hdr_size(leaf) +
		ichdr.count * sizeof(xfs_attr_leaf_entry_t) +
		ichdr.usedbytes;
	if (bytes > (state->blocksize >> 1)) {
		*action = 0;	/*                                 */
		return(0);
	}

	/*
                                                           
                                                             
                                                              
                                                      
  */
	if (ichdr.count == 0) {
		/*
                                                        
                                                        
   */
		forward = (ichdr.forw != 0);
		memcpy(&state->altpath, &state->path, sizeof(state->path));
		error = xfs_da3_path_shift(state, &state->altpath, forward,
						 0, &retval);
		if (error)
			return(error);
		if (retval) {
			*action = 0;
		} else {
			*action = 2;
		}
		return 0;
	}

	/*
                                                             
                                                            
                                                            
                                                              
                                          
  */
	/*                            */
	forward = ichdr.forw < ichdr.back;
	for (i = 0; i < 2; forward = !forward, i++) {
		struct xfs_attr3_icleaf_hdr ichdr2;
		if (forward)
			blkno = ichdr.forw;
		else
			blkno = ichdr.back;
		if (blkno == 0)
			continue;
		error = xfs_attr3_leaf_read(state->args->trans, state->args->dp,
					blkno, -1, &bp);
		if (error)
			return(error);

		xfs_attr3_leaf_hdr_from_disk(&ichdr2, bp->b_addr);

		bytes = state->blocksize - (state->blocksize >> 2) -
			ichdr.usedbytes - ichdr2.usedbytes -
			((ichdr.count + ichdr2.count) *
					sizeof(xfs_attr_leaf_entry_t)) -
			xfs_attr3_leaf_hdr_size(leaf);

		xfs_trans_brelse(state->args->trans, bp);
		if (bytes >= 0)
			break;	/*                                 */
	}
	if (i >= 2) {
		*action = 0;
		return(0);
	}

	/*
                                                              
                                                                
  */
	memcpy(&state->altpath, &state->path, sizeof(state->path));
	if (blkno < blk->blkno) {
		error = xfs_da3_path_shift(state, &state->altpath, forward,
						 0, &retval);
	} else {
		error = xfs_da3_path_shift(state, &state->path, forward,
						 0, &retval);
	}
	if (error)
		return(error);
	if (retval) {
		*action = 0;
	} else {
		*action = 1;
	}
	return(0);
}

/*
                                                        
  
                                                            
                                                                      
 */
int
xfs_attr3_leaf_remove(
	struct xfs_buf		*bp,
	struct xfs_da_args	*args)
{
	struct xfs_attr_leafblock *leaf;
	struct xfs_attr3_icleaf_hdr ichdr;
	struct xfs_attr_leaf_entry *entry;
	struct xfs_mount	*mp = args->trans->t_mountp;
	int			before;
	int			after;
	int			smallest;
	int			entsize;
	int			tablesize;
	int			tmp;
	int			i;

	trace_xfs_attr_leaf_remove(args);

	leaf = bp->b_addr;
	xfs_attr3_leaf_hdr_from_disk(&ichdr, leaf);

	ASSERT(ichdr.count > 0 && ichdr.count < XFS_LBSIZE(mp) / 8);
	ASSERT(args->index >= 0 && args->index < ichdr.count);
	ASSERT(ichdr.firstused >= ichdr.count * sizeof(*entry) +
					xfs_attr3_leaf_hdr_size(leaf));

	entry = &xfs_attr3_leaf_entryp(leaf)[args->index];

	ASSERT(be16_to_cpu(entry->nameidx) >= ichdr.firstused);
	ASSERT(be16_to_cpu(entry->nameidx) < XFS_LBSIZE(mp));

	/*
                                   
                                                                
                                                               
                                                   
  */
	tablesize = ichdr.count * sizeof(xfs_attr_leaf_entry_t)
					+ xfs_attr3_leaf_hdr_size(leaf);
	tmp = ichdr.freemap[0].size;
	before = after = -1;
	smallest = XFS_ATTR_LEAF_MAPSIZE - 1;
	entsize = xfs_attr_leaf_entsize(leaf, args->index);
	for (i = 0; i < XFS_ATTR_LEAF_MAPSIZE; i++) {
		ASSERT(ichdr.freemap[i].base < XFS_LBSIZE(mp));
		ASSERT(ichdr.freemap[i].size < XFS_LBSIZE(mp));
		if (ichdr.freemap[i].base == tablesize) {
			ichdr.freemap[i].base -= sizeof(xfs_attr_leaf_entry_t);
			ichdr.freemap[i].size += sizeof(xfs_attr_leaf_entry_t);
		}

		if (ichdr.freemap[i].base + ichdr.freemap[i].size ==
				be16_to_cpu(entry->nameidx)) {
			before = i;
		} else if (ichdr.freemap[i].base ==
				(be16_to_cpu(entry->nameidx) + entsize)) {
			after = i;
		} else if (ichdr.freemap[i].size < tmp) {
			tmp = ichdr.freemap[i].size;
			smallest = i;
		}
	}

	/*
                                      
                                   
  */
	if ((before >= 0) || (after >= 0)) {
		if ((before >= 0) && (after >= 0)) {
			ichdr.freemap[before].size += entsize;
			ichdr.freemap[before].size += ichdr.freemap[after].size;
			ichdr.freemap[after].base = 0;
			ichdr.freemap[after].size = 0;
		} else if (before >= 0) {
			ichdr.freemap[before].size += entsize;
		} else {
			ichdr.freemap[after].base = be16_to_cpu(entry->nameidx);
			ichdr.freemap[after].size += entsize;
		}
	} else {
		/*
                                                                 
   */
		if (ichdr.freemap[smallest].size < entsize) {
			ichdr.freemap[smallest].base = be16_to_cpu(entry->nameidx);
			ichdr.freemap[smallest].size = entsize;
		}
	}

	/*
                                  
  */
	if (be16_to_cpu(entry->nameidx) == ichdr.firstused)
		smallest = 1;
	else
		smallest = 0;

	/*
                                                                  
  */
	memset(xfs_attr3_leaf_name(leaf, args->index), 0, entsize);
	ichdr.usedbytes -= entsize;
	xfs_trans_log_buf(args->trans, bp,
	     XFS_DA_LOGRANGE(leaf, xfs_attr3_leaf_name(leaf, args->index),
				   entsize));

	tmp = (ichdr.count - args->index) * sizeof(xfs_attr_leaf_entry_t);
	memmove(entry, entry + 1, tmp);
	ichdr.count--;
	xfs_trans_log_buf(args->trans, bp,
	    XFS_DA_LOGRANGE(leaf, entry, tmp + sizeof(xfs_attr_leaf_entry_t)));

	entry = &xfs_attr3_leaf_entryp(leaf)[ichdr.count];
	memset(entry, 0, sizeof(xfs_attr_leaf_entry_t));

	/*
                                                              
                                                                  
                                                           
                      
  */
	if (smallest) {
		tmp = XFS_LBSIZE(mp);
		entry = xfs_attr3_leaf_entryp(leaf);
		for (i = ichdr.count - 1; i >= 0; entry++, i--) {
			ASSERT(be16_to_cpu(entry->nameidx) >= ichdr.firstused);
			ASSERT(be16_to_cpu(entry->nameidx) < XFS_LBSIZE(mp));

			if (be16_to_cpu(entry->nameidx) < tmp)
				tmp = be16_to_cpu(entry->nameidx);
		}
		ichdr.firstused = tmp;
		if (!ichdr.firstused)
			ichdr.firstused = tmp - XFS_ATTR_LEAF_NAME_ALIGN;
	} else {
		ichdr.holes = 1;	/*                            */
	}
	xfs_attr3_leaf_hdr_to_disk(leaf, &ichdr);
	xfs_trans_log_buf(args->trans, bp,
			  XFS_DA_LOGRANGE(leaf, &leaf->hdr,
					  xfs_attr3_leaf_hdr_size(leaf)));

	/*
                                                           
                                         
  */
	tmp = ichdr.usedbytes + xfs_attr3_leaf_hdr_size(leaf) +
	      ichdr.count * sizeof(xfs_attr_leaf_entry_t);

	return tmp < mp->m_attr_magicpct; /*                    */
}

/*
                                                                     
 */
void
xfs_attr3_leaf_unbalance(
	struct xfs_da_state	*state,
	struct xfs_da_state_blk	*drop_blk,
	struct xfs_da_state_blk	*save_blk)
{
	struct xfs_attr_leafblock *drop_leaf = drop_blk->bp->b_addr;
	struct xfs_attr_leafblock *save_leaf = save_blk->bp->b_addr;
	struct xfs_attr3_icleaf_hdr drophdr;
	struct xfs_attr3_icleaf_hdr savehdr;
	struct xfs_attr_leaf_entry *entry;
	struct xfs_mount	*mp = state->mp;

	trace_xfs_attr_leaf_unbalance(state->args);

	drop_leaf = drop_blk->bp->b_addr;
	save_leaf = save_blk->bp->b_addr;
	xfs_attr3_leaf_hdr_from_disk(&drophdr, drop_leaf);
	xfs_attr3_leaf_hdr_from_disk(&savehdr, save_leaf);
	entry = xfs_attr3_leaf_entryp(drop_leaf);

	/*
                                                             
  */
	drop_blk->hashval = be32_to_cpu(entry[drophdr.count - 1].hashval);

	/*
                                                             
                                                             
                                                                  
  */
	if (savehdr.holes == 0) {
		/*
                                                       
                                          
   */
		if (xfs_attr3_leaf_order(save_blk->bp, &savehdr,
					 drop_blk->bp, &drophdr)) {
			xfs_attr3_leaf_moveents(drop_leaf, &drophdr, 0,
						save_leaf, &savehdr, 0,
						drophdr.count, mp);
		} else {
			xfs_attr3_leaf_moveents(drop_leaf, &drophdr, 0,
						save_leaf, &savehdr,
						savehdr.count, drophdr.count, mp);
		}
	} else {
		/*
                                                       
                                           
   */
		struct xfs_attr_leafblock *tmp_leaf;
		struct xfs_attr3_icleaf_hdr tmphdr;

		tmp_leaf = kmem_zalloc(state->blocksize, KM_SLEEP);

		/*
                                                             
                                                                
                                      
   */
		memcpy(tmp_leaf, save_leaf, xfs_attr3_leaf_hdr_size(save_leaf));

		memset(&tmphdr, 0, sizeof(tmphdr));
		tmphdr.magic = savehdr.magic;
		tmphdr.forw = savehdr.forw;
		tmphdr.back = savehdr.back;
		tmphdr.firstused = state->blocksize;

		/*                                                      */
		xfs_attr3_leaf_hdr_to_disk(tmp_leaf, &tmphdr);

		if (xfs_attr3_leaf_order(save_blk->bp, &savehdr,
					 drop_blk->bp, &drophdr)) {
			xfs_attr3_leaf_moveents(drop_leaf, &drophdr, 0,
						tmp_leaf, &tmphdr, 0,
						drophdr.count, mp);
			xfs_attr3_leaf_moveents(save_leaf, &savehdr, 0,
						tmp_leaf, &tmphdr, tmphdr.count,
						savehdr.count, mp);
		} else {
			xfs_attr3_leaf_moveents(save_leaf, &savehdr, 0,
						tmp_leaf, &tmphdr, 0,
						savehdr.count, mp);
			xfs_attr3_leaf_moveents(drop_leaf, &drophdr, 0,
						tmp_leaf, &tmphdr, tmphdr.count,
						drophdr.count, mp);
		}
		memcpy(save_leaf, tmp_leaf, state->blocksize);
		savehdr = tmphdr; /*             */
		kmem_free(tmp_leaf);
	}

	xfs_attr3_leaf_hdr_to_disk(save_leaf, &savehdr);
	xfs_trans_log_buf(state->args->trans, save_blk->bp, 0,
					   state->blocksize - 1);

	/*
                                                        
  */
	entry = xfs_attr3_leaf_entryp(save_leaf);
	save_blk->hashval = be32_to_cpu(entry[savehdr.count - 1].hashval);
}

/*                                                                        
                                                 
                                                                          */

/*
                                                     
                                                             
  
                                                                  
                                                                        
  
                                                                   
                                                                      
               
  
                                                             
 */
int
xfs_attr3_leaf_lookup_int(
	struct xfs_buf		*bp,
	struct xfs_da_args	*args)
{
	struct xfs_attr_leafblock *leaf;
	struct xfs_attr3_icleaf_hdr ichdr;
	struct xfs_attr_leaf_entry *entry;
	struct xfs_attr_leaf_entry *entries;
	struct xfs_attr_leaf_name_local *name_loc;
	struct xfs_attr_leaf_name_remote *name_rmt;
	xfs_dahash_t		hashval;
	int			probe;
	int			span;

	trace_xfs_attr_leaf_lookup(args);

	leaf = bp->b_addr;
	xfs_attr3_leaf_hdr_from_disk(&ichdr, leaf);
	entries = xfs_attr3_leaf_entryp(leaf);
	ASSERT(ichdr.count < XFS_LBSIZE(args->dp->i_mount) / 8);

	/*
                                                            
  */
	hashval = args->hashval;
	probe = span = ichdr.count / 2;
	for (entry = &entries[probe]; span > 4; entry = &entries[probe]) {
		span /= 2;
		if (be32_to_cpu(entry->hashval) < hashval)
			probe += span;
		else if (be32_to_cpu(entry->hashval) > hashval)
			probe -= span;
		else
			break;
	}
	ASSERT(probe >= 0 && (!ichdr.count || probe < ichdr.count));
	ASSERT(span <= 4 || be32_to_cpu(entry->hashval) == hashval);

	/*
                                                                  
                        
  */
	while (probe > 0 && be32_to_cpu(entry->hashval) >= hashval) {
		entry--;
		probe--;
	}
	while (probe < ichdr.count &&
	       be32_to_cpu(entry->hashval) < hashval) {
		entry++;
		probe++;
	}
	if (probe == ichdr.count || be32_to_cpu(entry->hashval) != hashval) {
		args->index = probe;
		return XFS_ERROR(ENOATTR);
	}

	/*
                                                                     
  */
	for (; probe < ichdr.count && (be32_to_cpu(entry->hashval) == hashval);
			entry++, probe++) {
/*
                                               
 */
		/*
                                                               
                                                             
   */
		if ((args->flags & XFS_ATTR_INCOMPLETE) !=
		    (entry->flags & XFS_ATTR_INCOMPLETE)) {
			continue;
		}
		if (entry->flags & XFS_ATTR_LOCAL) {
			name_loc = xfs_attr3_leaf_name_local(leaf, probe);
			if (name_loc->namelen != args->namelen)
				continue;
			if (memcmp(args->name, name_loc->nameval,
							args->namelen) != 0)
				continue;
			if (!xfs_attr_namesp_match(args->flags, entry->flags))
				continue;
			args->index = probe;
			return XFS_ERROR(EEXIST);
		} else {
			name_rmt = xfs_attr3_leaf_name_remote(leaf, probe);
			if (name_rmt->namelen != args->namelen)
				continue;
			if (memcmp(args->name, name_rmt->name,
							args->namelen) != 0)
				continue;
			if (!xfs_attr_namesp_match(args->flags, entry->flags))
				continue;
			args->index = probe;
			args->valuelen = be32_to_cpu(name_rmt->valuelen);
			args->rmtblkno = be32_to_cpu(name_rmt->valueblk);
			args->rmtblkcnt = xfs_attr3_rmt_blocks(
							args->dp->i_mount,
							args->valuelen);
			return XFS_ERROR(EEXIST);
		}
	}
	args->index = probe;
	return XFS_ERROR(ENOATTR);
}

/*
                                                                        
                  
 */
int
xfs_attr3_leaf_getvalue(
	struct xfs_buf		*bp,
	struct xfs_da_args	*args)
{
	struct xfs_attr_leafblock *leaf;
	struct xfs_attr3_icleaf_hdr ichdr;
	struct xfs_attr_leaf_entry *entry;
	struct xfs_attr_leaf_name_local *name_loc;
	struct xfs_attr_leaf_name_remote *name_rmt;
	int			valuelen;

	leaf = bp->b_addr;
	xfs_attr3_leaf_hdr_from_disk(&ichdr, leaf);
	ASSERT(ichdr.count < XFS_LBSIZE(args->dp->i_mount) / 8);
	ASSERT(args->index < ichdr.count);

	entry = &xfs_attr3_leaf_entryp(leaf)[args->index];
	if (entry->flags & XFS_ATTR_LOCAL) {
		name_loc = xfs_attr3_leaf_name_local(leaf, args->index);
		ASSERT(name_loc->namelen == args->namelen);
		ASSERT(memcmp(args->name, name_loc->nameval, args->namelen) == 0);
		valuelen = be16_to_cpu(name_loc->valuelen);
		if (args->flags & ATTR_KERNOVAL) {
			args->valuelen = valuelen;
			return 0;
		}
		if (args->valuelen < valuelen) {
			args->valuelen = valuelen;
			return XFS_ERROR(ERANGE);
		}
		args->valuelen = valuelen;
		memcpy(args->value, &name_loc->nameval[args->namelen], valuelen);
	} else {
		name_rmt = xfs_attr3_leaf_name_remote(leaf, args->index);
		ASSERT(name_rmt->namelen == args->namelen);
		ASSERT(memcmp(args->name, name_rmt->name, args->namelen) == 0);
		valuelen = be32_to_cpu(name_rmt->valuelen);
		args->rmtblkno = be32_to_cpu(name_rmt->valueblk);
		args->rmtblkcnt = xfs_attr3_rmt_blocks(args->dp->i_mount,
						       valuelen);
		if (args->flags & ATTR_KERNOVAL) {
			args->valuelen = valuelen;
			return 0;
		}
		if (args->valuelen < valuelen) {
			args->valuelen = valuelen;
			return XFS_ERROR(ERANGE);
		}
		args->valuelen = valuelen;
	}
	return 0;
}

/*                                                                        
                    
                                                                          */

/*
                                                       
                                                                  
 */
/*        */
STATIC void
xfs_attr3_leaf_moveents(
	struct xfs_attr_leafblock	*leaf_s,
	struct xfs_attr3_icleaf_hdr	*ichdr_s,
	int				start_s,
	struct xfs_attr_leafblock	*leaf_d,
	struct xfs_attr3_icleaf_hdr	*ichdr_d,
	int				start_d,
	int				count,
	struct xfs_mount		*mp)
{
	struct xfs_attr_leaf_entry	*entry_s;
	struct xfs_attr_leaf_entry	*entry_d;
	int				desti;
	int				tmp;
	int				i;

	/*
                            
  */
	if (count == 0)
		return;

	/*
                       
  */
	ASSERT(ichdr_s->magic == XFS_ATTR_LEAF_MAGIC ||
	       ichdr_s->magic == XFS_ATTR3_LEAF_MAGIC);
	ASSERT(ichdr_s->magic == ichdr_d->magic);
	ASSERT(ichdr_s->count > 0 && ichdr_s->count < XFS_LBSIZE(mp) / 8);
	ASSERT(ichdr_s->firstused >= (ichdr_s->count * sizeof(*entry_s))
					+ xfs_attr3_leaf_hdr_size(leaf_s));
	ASSERT(ichdr_d->count < XFS_LBSIZE(mp) / 8);
	ASSERT(ichdr_d->firstused >= (ichdr_d->count * sizeof(*entry_d))
					+ xfs_attr3_leaf_hdr_size(leaf_d));

	ASSERT(start_s < ichdr_s->count);
	ASSERT(start_d <= ichdr_d->count);
	ASSERT(count <= ichdr_s->count);


	/*
                                                               
  */
	if (start_d < ichdr_d->count) {
		tmp  = ichdr_d->count - start_d;
		tmp *= sizeof(xfs_attr_leaf_entry_t);
		entry_s = &xfs_attr3_leaf_entryp(leaf_d)[start_d];
		entry_d = &xfs_attr3_leaf_entryp(leaf_d)[start_d + count];
		memmove(entry_d, entry_s, tmp);
	}

	/*
                                                
                                                       
  */
	entry_s = &xfs_attr3_leaf_entryp(leaf_s)[start_s];
	entry_d = &xfs_attr3_leaf_entryp(leaf_d)[start_d];
	desti = start_d;
	for (i = 0; i < count; entry_s++, entry_d++, desti++, i++) {
		ASSERT(be16_to_cpu(entry_s->nameidx) >= ichdr_s->firstused);
		tmp = xfs_attr_leaf_entsize(leaf_s, start_s + i);
#ifdef GROT
		/*
                                                             
                                                              
                                            
   */
		if (entry_s->flags & XFS_ATTR_INCOMPLETE) { /*                */
			memset(xfs_attr3_leaf_name(leaf_s, start_s + i), 0, tmp);
			ichdr_s->usedbytes -= tmp;
			ichdr_s->count -= 1;
			entry_d--;	/*                                  */
			desti--;
			if ((start_s + i) < offset)
				result++;	/*                            */
		} else {
#endif /*      */
			ichdr_d->firstused -= tmp;
			/*                                       */
			entry_d->hashval = entry_s->hashval;
			entry_d->nameidx = cpu_to_be16(ichdr_d->firstused);
			entry_d->flags = entry_s->flags;
			ASSERT(be16_to_cpu(entry_d->nameidx) + tmp
							<= XFS_LBSIZE(mp));
			memmove(xfs_attr3_leaf_name(leaf_d, desti),
				xfs_attr3_leaf_name(leaf_s, start_s + i), tmp);
			ASSERT(be16_to_cpu(entry_s->nameidx) + tmp
							<= XFS_LBSIZE(mp));
			memset(xfs_attr3_leaf_name(leaf_s, start_s + i), 0, tmp);
			ichdr_s->usedbytes -= tmp;
			ichdr_d->usedbytes += tmp;
			ichdr_s->count -= 1;
			ichdr_d->count += 1;
			tmp = ichdr_d->count * sizeof(xfs_attr_leaf_entry_t)
					+ xfs_attr3_leaf_hdr_size(leaf_d);
			ASSERT(ichdr_d->firstused >= tmp);
#ifdef GROT
		}
#endif /*      */
	}

	/*
                                        
  */
	if (start_s == ichdr_s->count) {
		tmp = count * sizeof(xfs_attr_leaf_entry_t);
		entry_s = &xfs_attr3_leaf_entryp(leaf_s)[start_s];
		ASSERT(((char *)entry_s + tmp) <=
		       ((char *)leaf_s + XFS_LBSIZE(mp)));
		memset(entry_s, 0, tmp);
	} else {
		/*
                                                      
                                      
   */
		tmp  = (ichdr_s->count - count) * sizeof(xfs_attr_leaf_entry_t);
		entry_s = &xfs_attr3_leaf_entryp(leaf_s)[start_s + count];
		entry_d = &xfs_attr3_leaf_entryp(leaf_s)[start_s];
		memmove(entry_d, entry_s, tmp);

		tmp = count * sizeof(xfs_attr_leaf_entry_t);
		entry_s = &xfs_attr3_leaf_entryp(leaf_s)[ichdr_s->count];
		ASSERT(((char *)entry_s + tmp) <=
		       ((char *)leaf_s + XFS_LBSIZE(mp)));
		memset(entry_s, 0, tmp);
	}

	/*
                                   
  */
	ichdr_d->freemap[0].base = xfs_attr3_leaf_hdr_size(leaf_d);
	ichdr_d->freemap[0].base += ichdr_d->count * sizeof(xfs_attr_leaf_entry_t);
	ichdr_d->freemap[0].size = ichdr_d->firstused - ichdr_d->freemap[0].base;
	ichdr_d->freemap[1].base = 0;
	ichdr_d->freemap[2].base = 0;
	ichdr_d->freemap[1].size = 0;
	ichdr_d->freemap[2].size = 0;
	ichdr_s->holes = 1;	/*                         */
}

/*
                                                
 */
xfs_dahash_t
xfs_attr_leaf_lasthash(
	struct xfs_buf	*bp,
	int		*count)
{
	struct xfs_attr3_icleaf_hdr ichdr;
	struct xfs_attr_leaf_entry *entries;

	xfs_attr3_leaf_hdr_from_disk(&ichdr, bp->b_addr);
	entries = xfs_attr3_leaf_entryp(bp->b_addr);
	if (count)
		*count = ichdr.count;
	if (!ichdr.count)
		return 0;
	return be32_to_cpu(entries[ichdr.count - 1].hashval);
}

/*
                                                                      
                                                                
 */
STATIC int
xfs_attr_leaf_entsize(xfs_attr_leafblock_t *leaf, int index)
{
	struct xfs_attr_leaf_entry *entries;
	xfs_attr_leaf_name_local_t *name_loc;
	xfs_attr_leaf_name_remote_t *name_rmt;
	int size;

	entries = xfs_attr3_leaf_entryp(leaf);
	if (entries[index].flags & XFS_ATTR_LOCAL) {
		name_loc = xfs_attr3_leaf_name_local(leaf, index);
		size = xfs_attr_leaf_entsize_local(name_loc->namelen,
						   be16_to_cpu(name_loc->valuelen));
	} else {
		name_rmt = xfs_attr3_leaf_name_remote(leaf, index);
		size = xfs_attr_leaf_entsize_remote(name_rmt->namelen);
	}
	return size;
}

/*
                                                                        
                                                                          
                                                                      
                                     
 */
int
xfs_attr_leaf_newentsize(int namelen, int valuelen, int blocksize, int *local)
{
	int size;

	size = xfs_attr_leaf_entsize_local(namelen, valuelen);
	if (size < xfs_attr_leaf_entsize_local_max(blocksize)) {
		if (local) {
			*local = 1;
		}
	} else {
		size = xfs_attr_leaf_entsize_remote(namelen);
		if (local) {
			*local = 0;
		}
	}
	return size;
}

/*
                                                                             
 */
int
xfs_attr3_leaf_list_int(
	struct xfs_buf			*bp,
	struct xfs_attr_list_context	*context)
{
	struct attrlist_cursor_kern	*cursor;
	struct xfs_attr_leafblock	*leaf;
	struct xfs_attr3_icleaf_hdr	ichdr;
	struct xfs_attr_leaf_entry	*entries;
	struct xfs_attr_leaf_entry	*entry;
	int				retval;
	int				i;

	trace_xfs_attr_list_leaf(context);

	leaf = bp->b_addr;
	xfs_attr3_leaf_hdr_from_disk(&ichdr, leaf);
	entries = xfs_attr3_leaf_entryp(leaf);

	cursor = context->cursor;
	cursor->initted = 1;

	/*
                                                                 
  */
	if (context->resynch) {
		entry = &entries[0];
		for (i = 0; i < ichdr.count; entry++, i++) {
			if (be32_to_cpu(entry->hashval) == cursor->hashval) {
				if (cursor->offset == context->dupcnt) {
					context->dupcnt = 0;
					break;
				}
				context->dupcnt++;
			} else if (be32_to_cpu(entry->hashval) >
					cursor->hashval) {
				context->dupcnt = 0;
				break;
			}
		}
		if (i == ichdr.count) {
			trace_xfs_attr_list_notfound(context);
			return 0;
		}
	} else {
		entry = &entries[0];
		i = 0;
	}
	context->resynch = 0;

	/*
                                                                  
  */
	retval = 0;
	for (; i < ichdr.count; entry++, i++) {
		if (be32_to_cpu(entry->hashval) != cursor->hashval) {
			cursor->hashval = be32_to_cpu(entry->hashval);
			cursor->offset = 0;
		}

		if (entry->flags & XFS_ATTR_INCOMPLETE)
			continue;		/*                         */

		if (entry->flags & XFS_ATTR_LOCAL) {
			xfs_attr_leaf_name_local_t *name_loc =
				xfs_attr3_leaf_name_local(leaf, i);

			retval = context->put_listent(context,
						entry->flags,
						name_loc->nameval,
						(int)name_loc->namelen,
						be16_to_cpu(name_loc->valuelen),
						&name_loc->nameval[name_loc->namelen]);
			if (retval)
				return retval;
		} else {
			xfs_attr_leaf_name_remote_t *name_rmt =
				xfs_attr3_leaf_name_remote(leaf, i);

			int valuelen = be32_to_cpu(name_rmt->valuelen);

			if (context->put_value) {
				xfs_da_args_t args;

				memset((char *)&args, 0, sizeof(args));
				args.dp = context->dp;
				args.whichfork = XFS_ATTR_FORK;
				args.valuelen = valuelen;
				args.value = kmem_alloc(valuelen, KM_SLEEP | KM_NOFS);
				args.rmtblkno = be32_to_cpu(name_rmt->valueblk);
				args.rmtblkcnt = xfs_attr3_rmt_blocks(
							args.dp->i_mount, valuelen);
				retval = xfs_attr_rmtval_get(&args);
				if (retval)
					return retval;
				retval = context->put_listent(context,
						entry->flags,
						name_rmt->name,
						(int)name_rmt->namelen,
						valuelen,
						args.value);
				kmem_free(args.value);
			} else {
				retval = context->put_listent(context,
						entry->flags,
						name_rmt->name,
						(int)name_rmt->namelen,
						valuelen,
						NULL);
			}
			if (retval)
				return retval;
		}
		if (context->seen_enough)
			break;
		cursor->offset++;
	}
	trace_xfs_attr_list_leaf_end(context);
	return retval;
}


/*                                                                        
                                             
                                                                          */

/*
                                                         
 */
int
xfs_attr3_leaf_clearflag(
	struct xfs_da_args	*args)
{
	struct xfs_attr_leafblock *leaf;
	struct xfs_attr_leaf_entry *entry;
	struct xfs_attr_leaf_name_remote *name_rmt;
	struct xfs_buf		*bp;
	int			error;
#ifdef DEBUG
	struct xfs_attr3_icleaf_hdr ichdr;
	xfs_attr_leaf_name_local_t *name_loc;
	int namelen;
	char *name;
#endif /*       */

	trace_xfs_attr_leaf_clearflag(args);
	/*
                         
  */
	error = xfs_attr3_leaf_read(args->trans, args->dp, args->blkno, -1, &bp);
	if (error)
		return(error);

	leaf = bp->b_addr;
	entry = &xfs_attr3_leaf_entryp(leaf)[args->index];
	ASSERT(entry->flags & XFS_ATTR_INCOMPLETE);

#ifdef DEBUG
	xfs_attr3_leaf_hdr_from_disk(&ichdr, leaf);
	ASSERT(args->index < ichdr.count);
	ASSERT(args->index >= 0);

	if (entry->flags & XFS_ATTR_LOCAL) {
		name_loc = xfs_attr3_leaf_name_local(leaf, args->index);
		namelen = name_loc->namelen;
		name = (char *)name_loc->nameval;
	} else {
		name_rmt = xfs_attr3_leaf_name_remote(leaf, args->index);
		namelen = name_rmt->namelen;
		name = (char *)name_rmt->name;
	}
	ASSERT(be32_to_cpu(entry->hashval) == args->hashval);
	ASSERT(namelen == args->namelen);
	ASSERT(memcmp(name, args->name, namelen) == 0);
#endif /*       */

	entry->flags &= ~XFS_ATTR_INCOMPLETE;
	xfs_trans_log_buf(args->trans, bp,
			 XFS_DA_LOGRANGE(leaf, entry, sizeof(*entry)));

	if (args->rmtblkno) {
		ASSERT((entry->flags & XFS_ATTR_LOCAL) == 0);
		name_rmt = xfs_attr3_leaf_name_remote(leaf, args->index);
		name_rmt->valueblk = cpu_to_be32(args->rmtblkno);
		name_rmt->valuelen = cpu_to_be32(args->valuelen);
		xfs_trans_log_buf(args->trans, bp,
			 XFS_DA_LOGRANGE(leaf, name_rmt, sizeof(*name_rmt)));
	}

	/*
                                                                    
  */
	return xfs_trans_roll(&args->trans, args->dp);
}

/*
                                                       
 */
int
xfs_attr3_leaf_setflag(
	struct xfs_da_args	*args)
{
	struct xfs_attr_leafblock *leaf;
	struct xfs_attr_leaf_entry *entry;
	struct xfs_attr_leaf_name_remote *name_rmt;
	struct xfs_buf		*bp;
	int error;
#ifdef DEBUG
	struct xfs_attr3_icleaf_hdr ichdr;
#endif

	trace_xfs_attr_leaf_setflag(args);

	/*
                         
  */
	error = xfs_attr3_leaf_read(args->trans, args->dp, args->blkno, -1, &bp);
	if (error)
		return(error);

	leaf = bp->b_addr;
#ifdef DEBUG
	xfs_attr3_leaf_hdr_from_disk(&ichdr, leaf);
	ASSERT(args->index < ichdr.count);
	ASSERT(args->index >= 0);
#endif
	entry = &xfs_attr3_leaf_entryp(leaf)[args->index];

	ASSERT((entry->flags & XFS_ATTR_INCOMPLETE) == 0);
	entry->flags |= XFS_ATTR_INCOMPLETE;
	xfs_trans_log_buf(args->trans, bp,
			XFS_DA_LOGRANGE(leaf, entry, sizeof(*entry)));
	if ((entry->flags & XFS_ATTR_LOCAL) == 0) {
		name_rmt = xfs_attr3_leaf_name_remote(leaf, args->index);
		name_rmt->valueblk = 0;
		name_rmt->valuelen = 0;
		xfs_trans_log_buf(args->trans, bp,
			 XFS_DA_LOGRANGE(leaf, name_rmt, sizeof(*name_rmt)));
	}

	/*
                                                                    
  */
	return xfs_trans_roll(&args->trans, args->dp);
}

/*
                                                                       
                                                                     
                                      
  
                                                                     
 */
int
xfs_attr3_leaf_flipflags(
	struct xfs_da_args	*args)
{
	struct xfs_attr_leafblock *leaf1;
	struct xfs_attr_leafblock *leaf2;
	struct xfs_attr_leaf_entry *entry1;
	struct xfs_attr_leaf_entry *entry2;
	struct xfs_attr_leaf_name_remote *name_rmt;
	struct xfs_buf		*bp1;
	struct xfs_buf		*bp2;
	int error;
#ifdef DEBUG
	struct xfs_attr3_icleaf_hdr ichdr1;
	struct xfs_attr3_icleaf_hdr ichdr2;
	xfs_attr_leaf_name_local_t *name_loc;
	int namelen1, namelen2;
	char *name1, *name2;
#endif /*       */

	trace_xfs_attr_leaf_flipflags(args);

	/*
                                            
  */
	error = xfs_attr3_leaf_read(args->trans, args->dp, args->blkno, -1, &bp1);
	if (error)
		return error;

	/*
                                                                
  */
	if (args->blkno2 != args->blkno) {
		error = xfs_attr3_leaf_read(args->trans, args->dp, args->blkno2,
					   -1, &bp2);
		if (error)
			return error;
	} else {
		bp2 = bp1;
	}

	leaf1 = bp1->b_addr;
	entry1 = &xfs_attr3_leaf_entryp(leaf1)[args->index];

	leaf2 = bp2->b_addr;
	entry2 = &xfs_attr3_leaf_entryp(leaf2)[args->index2];

#ifdef DEBUG
	xfs_attr3_leaf_hdr_from_disk(&ichdr1, leaf1);
	ASSERT(args->index < ichdr1.count);
	ASSERT(args->index >= 0);

	xfs_attr3_leaf_hdr_from_disk(&ichdr2, leaf2);
	ASSERT(args->index2 < ichdr2.count);
	ASSERT(args->index2 >= 0);

	if (entry1->flags & XFS_ATTR_LOCAL) {
		name_loc = xfs_attr3_leaf_name_local(leaf1, args->index);
		namelen1 = name_loc->namelen;
		name1 = (char *)name_loc->nameval;
	} else {
		name_rmt = xfs_attr3_leaf_name_remote(leaf1, args->index);
		namelen1 = name_rmt->namelen;
		name1 = (char *)name_rmt->name;
	}
	if (entry2->flags & XFS_ATTR_LOCAL) {
		name_loc = xfs_attr3_leaf_name_local(leaf2, args->index2);
		namelen2 = name_loc->namelen;
		name2 = (char *)name_loc->nameval;
	} else {
		name_rmt = xfs_attr3_leaf_name_remote(leaf2, args->index2);
		namelen2 = name_rmt->namelen;
		name2 = (char *)name_rmt->name;
	}
	ASSERT(be32_to_cpu(entry1->hashval) == be32_to_cpu(entry2->hashval));
	ASSERT(namelen1 == namelen2);
	ASSERT(memcmp(name1, name2, namelen1) == 0);
#endif /*       */

	ASSERT(entry1->flags & XFS_ATTR_INCOMPLETE);
	ASSERT((entry2->flags & XFS_ATTR_INCOMPLETE) == 0);

	entry1->flags &= ~XFS_ATTR_INCOMPLETE;
	xfs_trans_log_buf(args->trans, bp1,
			  XFS_DA_LOGRANGE(leaf1, entry1, sizeof(*entry1)));
	if (args->rmtblkno) {
		ASSERT((entry1->flags & XFS_ATTR_LOCAL) == 0);
		name_rmt = xfs_attr3_leaf_name_remote(leaf1, args->index);
		name_rmt->valueblk = cpu_to_be32(args->rmtblkno);
		name_rmt->valuelen = cpu_to_be32(args->valuelen);
		xfs_trans_log_buf(args->trans, bp1,
			 XFS_DA_LOGRANGE(leaf1, name_rmt, sizeof(*name_rmt)));
	}

	entry2->flags |= XFS_ATTR_INCOMPLETE;
	xfs_trans_log_buf(args->trans, bp2,
			  XFS_DA_LOGRANGE(leaf2, entry2, sizeof(*entry2)));
	if ((entry2->flags & XFS_ATTR_LOCAL) == 0) {
		name_rmt = xfs_attr3_leaf_name_remote(leaf2, args->index2);
		name_rmt->valueblk = 0;
		name_rmt->valuelen = 0;
		xfs_trans_log_buf(args->trans, bp2,
			 XFS_DA_LOGRANGE(leaf2, name_rmt, sizeof(*name_rmt)));
	}

	/*
                                                                    
  */
	error = xfs_trans_roll(&args->trans, args->dp);

	return error;
}

/*                                                                        
                                                    
                                                                          */

/*
                                                                    
                                                                         
 */
int
xfs_attr3_root_inactive(
	struct xfs_trans	**trans,
	struct xfs_inode	*dp)
{
	struct xfs_da_blkinfo	*info;
	struct xfs_buf		*bp;
	xfs_daddr_t		blkno;
	int			error;

	/*
                                                  
                                                        
                                                      
                                
  */
	error = xfs_da3_node_read(*trans, dp, 0, -1, &bp, XFS_ATTR_FORK);
	if (error)
		return error;
	blkno = bp->b_bn;

	/*
                                                                        
                                    
  */
	info = bp->b_addr;
	switch (info->magic) {
	case cpu_to_be16(XFS_DA_NODE_MAGIC):
	case cpu_to_be16(XFS_DA3_NODE_MAGIC):
		error = xfs_attr3_node_inactive(trans, dp, bp, 1);
		break;
	case cpu_to_be16(XFS_ATTR_LEAF_MAGIC):
	case cpu_to_be16(XFS_ATTR3_LEAF_MAGIC):
		error = xfs_attr3_leaf_inactive(trans, dp, bp);
		break;
	default:
		error = XFS_ERROR(EIO);
		xfs_trans_brelse(*trans, bp);
		break;
	}
	if (error)
		return error;

	/*
                                                 
  */
	error = xfs_da_get_buf(*trans, dp, 0, blkno, &bp, XFS_ATTR_FORK);
	if (error)
		return error;
	xfs_trans_binval(*trans, bp);	/*                   */
	/*
                                                         
  */
	error = xfs_trans_roll(trans, dp);

	return error;
}

/*
                                                                    
                                                                         
 */
STATIC int
xfs_attr3_node_inactive(
	struct xfs_trans **trans,
	struct xfs_inode *dp,
	struct xfs_buf	*bp,
	int		level)
{
	xfs_da_blkinfo_t *info;
	xfs_da_intnode_t *node;
	xfs_dablk_t child_fsb;
	xfs_daddr_t parent_blkno, child_blkno;
	int error, i;
	struct xfs_buf *child_bp;
	struct xfs_da_node_entry *btree;
	struct xfs_da3_icnode_hdr ichdr;

	/*
                                                                   
  */
	if (level > XFS_DA_NODE_MAXDEPTH) {
		xfs_trans_brelse(*trans, bp);	/*                          */
		return XFS_ERROR(EIO);
	}

	node = bp->b_addr;
	xfs_da3_node_hdr_from_disk(&ichdr, node);
	parent_blkno = bp->b_bn;
	if (!ichdr.count) {
		xfs_trans_brelse(*trans, bp);
		return 0;
	}
	btree = xfs_da3_node_tree_p(node);
	child_fsb = be32_to_cpu(btree[0].before);
	xfs_trans_brelse(*trans, bp);	/*                          */

	/*
                                                                
                                                               
                                  
  */
	for (i = 0; i < ichdr.count; i++) {
		/*
                                                                
                                                           
                                                          
                                     
   */
		error = xfs_da3_node_read(*trans, dp, child_fsb, -2, &child_bp,
						XFS_ATTR_FORK);
		if (error)
			return(error);
		if (child_bp) {
						/*                        */
			child_blkno = XFS_BUF_ADDR(child_bp);

			/*
                                                 
    */
			info = child_bp->b_addr;
			switch (info->magic) {
			case cpu_to_be16(XFS_DA_NODE_MAGIC):
			case cpu_to_be16(XFS_DA3_NODE_MAGIC):
				error = xfs_attr3_node_inactive(trans, dp,
							child_bp, level + 1);
				break;
			case cpu_to_be16(XFS_ATTR_LEAF_MAGIC):
			case cpu_to_be16(XFS_ATTR3_LEAF_MAGIC):
				error = xfs_attr3_leaf_inactive(trans, dp,
							child_bp);
				break;
			default:
				error = XFS_ERROR(EIO);
				xfs_trans_brelse(*trans, child_bp);
				break;
			}
			if (error)
				return error;

			/*
                                                
                       
    */
			error = xfs_da_get_buf(*trans, dp, 0, child_blkno,
				&child_bp, XFS_ATTR_FORK);
			if (error)
				return error;
			xfs_trans_binval(*trans, child_bp);
		}

		/*
                                                          
                        
   */
		if (i + 1 < ichdr.count) {
			error = xfs_da3_node_read(*trans, dp, 0, parent_blkno,
						 &bp, XFS_ATTR_FORK);
			if (error)
				return error;
			child_fsb = be32_to_cpu(btree[i + 1].before);
			xfs_trans_brelse(*trans, bp);
		}
		/*
                                                  
   */
		error = xfs_trans_roll(trans, dp);
		if (error)
			return  error;
	}

	return 0;
}

/*
                                                                          
              
                                                                      
                                                                         
 */
STATIC int
xfs_attr3_leaf_inactive(
	struct xfs_trans	**trans,
	struct xfs_inode	*dp,
	struct xfs_buf		*bp)
{
	struct xfs_attr_leafblock *leaf;
	struct xfs_attr3_icleaf_hdr ichdr;
	struct xfs_attr_leaf_entry *entry;
	struct xfs_attr_leaf_name_remote *name_rmt;
	struct xfs_attr_inactive_list *list;
	struct xfs_attr_inactive_list *lp;
	int			error;
	int			count;
	int			size;
	int			tmp;
	int			i;

	leaf = bp->b_addr;
	xfs_attr3_leaf_hdr_from_disk(&ichdr, leaf);

	/*
                                               
  */
	count = 0;
	entry = xfs_attr3_leaf_entryp(leaf);
	for (i = 0; i < ichdr.count; entry++, i++) {
		if (be16_to_cpu(entry->nameidx) &&
		    ((entry->flags & XFS_ATTR_LOCAL) == 0)) {
			name_rmt = xfs_attr3_leaf_name_remote(leaf, i);
			if (name_rmt->valueblk)
				count++;
		}
	}

	/*
                                                
  */
	if (count == 0) {
		xfs_trans_brelse(*trans, bp);
		return 0;
	}

	/*
                                                                  
  */
	size = count * sizeof(xfs_attr_inactive_list_t);
	list = kmem_alloc(size, KM_SLEEP);

	/*
                                                
  */
	lp = list;
	entry = xfs_attr3_leaf_entryp(leaf);
	for (i = 0; i < ichdr.count; entry++, i++) {
		if (be16_to_cpu(entry->nameidx) &&
		    ((entry->flags & XFS_ATTR_LOCAL) == 0)) {
			name_rmt = xfs_attr3_leaf_name_remote(leaf, i);
			if (name_rmt->valueblk) {
				lp->valueblk = be32_to_cpu(name_rmt->valueblk);
				lp->valuelen = xfs_attr3_rmt_blocks(dp->i_mount,
						    be32_to_cpu(name_rmt->valuelen));
				lp++;
			}
		}
	}
	xfs_trans_brelse(*trans, bp);	/*                                  */

	/*
                                                  
  */
	error = 0;
	for (lp = list, i = 0; i < count; i++, lp++) {
		tmp = xfs_attr3_leaf_freextent(trans, dp,
				lp->valueblk, lp->valuelen);

		if (error == 0)
			error = tmp;	/*                         */
	}

	kmem_free(list);
	return error;
}

/*
                                                   
                                                          
 */
STATIC int
xfs_attr3_leaf_freextent(
	struct xfs_trans	**trans,
	struct xfs_inode	*dp,
	xfs_dablk_t		blkno,
	int			blkcnt)
{
	struct xfs_bmbt_irec	map;
	struct xfs_buf		*bp;
	xfs_dablk_t		tblkno;
	xfs_daddr_t		dblkno;
	int			tblkcnt;
	int			dblkcnt;
	int			nmap;
	int			error;

	/*
                                                                
           
  */
	tblkno = blkno;
	tblkcnt = blkcnt;
	while (tblkcnt > 0) {
		/*
                                                       
   */
		nmap = 1;
		error = xfs_bmapi_read(dp, (xfs_fileoff_t)tblkno, tblkcnt,
				       &map, &nmap, XFS_BMAPI_ATTRFORK);
		if (error) {
			return(error);
		}
		ASSERT(nmap == 1);
		ASSERT(map.br_startblock != DELAYSTARTBLOCK);

		/*
                                               
                                      
   */
		if (map.br_startblock != HOLESTARTBLOCK) {

			dblkno = XFS_FSB_TO_DADDR(dp->i_mount,
						  map.br_startblock);
			dblkcnt = XFS_FSB_TO_BB(dp->i_mount,
						map.br_blockcount);
			bp = xfs_trans_get_buf(*trans,
					dp->i_mount->m_ddev_targp,
					dblkno, dblkcnt, 0);
			if (!bp)
				return ENOMEM;
			xfs_trans_binval(*trans, bp);
			/*
                               
    */
			error = xfs_trans_roll(trans, dp);
			if (error)
				return (error);
		}

		tblkno += map.br_blockcount;
		tblkcnt -= map.br_blockcount;
	}

	return(0);
}
