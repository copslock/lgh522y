/*
 * Copyright (c) 2000-2003,2005 Silicon Graphics, Inc.
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
#ifndef	__XFS_LOG_H__
#define __XFS_LOG_H__

/*                */
#define CYCLE_LSN(lsn) ((uint)((lsn)>>32))
#define BLOCK_LSN(lsn) ((uint)(lsn))

/*                                                                    */
#define CYCLE_LSN_DISK(lsn) (((__be32 *)&(lsn))[0])

#ifdef __KERNEL__
/*
                                                                  
                                                                    
 */
static inline xfs_lsn_t	_lsn_cmp(xfs_lsn_t lsn1, xfs_lsn_t lsn2)
{
	if (CYCLE_LSN(lsn1) != CYCLE_LSN(lsn2))
		return (CYCLE_LSN(lsn1)<CYCLE_LSN(lsn2))? -999 : 999;

	if (BLOCK_LSN(lsn1) != BLOCK_LSN(lsn2))
		return (BLOCK_LSN(lsn1)<BLOCK_LSN(lsn2))? -999 : 999;

	return 0;
}

#define	XFS_LSN_CMP(x,y) _lsn_cmp(x,y)

/*
                                                                   
 */

/*
                          
 */
#define XFS_LOG_REL_PERM_RESERV	0x1

/*
                           
  
                                                      
 */
#define XFS_LOG_SYNC		0x1

#endif	/*            */


/*             */
#define XFS_TRANSACTION		0x69
#define XFS_VOLUME		0x2
#define XFS_LOG			0xaa


/*                                 */
#define XLOG_REG_TYPE_BFORMAT		1
#define XLOG_REG_TYPE_BCHUNK		2
#define XLOG_REG_TYPE_EFI_FORMAT	3
#define XLOG_REG_TYPE_EFD_FORMAT	4
#define XLOG_REG_TYPE_IFORMAT		5
#define XLOG_REG_TYPE_ICORE		6
#define XLOG_REG_TYPE_IEXT		7
#define XLOG_REG_TYPE_IBROOT		8
#define XLOG_REG_TYPE_ILOCAL		9
#define XLOG_REG_TYPE_IATTR_EXT		10
#define XLOG_REG_TYPE_IATTR_BROOT	11
#define XLOG_REG_TYPE_IATTR_LOCAL	12
#define XLOG_REG_TYPE_QFORMAT		13
#define XLOG_REG_TYPE_DQUOT		14
#define XLOG_REG_TYPE_QUOTAOFF		15
#define XLOG_REG_TYPE_LRHEADER		16
#define XLOG_REG_TYPE_UNMOUNT		17
#define XLOG_REG_TYPE_COMMIT		18
#define XLOG_REG_TYPE_TRANSHDR		19
#define XLOG_REG_TYPE_MAX		19

typedef struct xfs_log_iovec {
	void		*i_addr;	/*                             */
	int		i_len;		/*                           */
	uint		i_type;		/*                */
} xfs_log_iovec_t;

struct xfs_log_vec {
	struct xfs_log_vec	*lv_next;	/*                       */
	int			lv_niovecs;	/*                        */
	struct xfs_log_iovec	*lv_iovecp;	/*             */
	struct xfs_log_item	*lv_item;	/*       */
	char			*lv_buf;	/*                  */
	int			lv_buf_len;	/*                          */
};

/*
                                                                       
                      
 */
typedef struct xfs_log_callback {
	struct xfs_log_callback	*cb_next;
	void			(*cb_func)(void *, int);
	void			*cb_arg;
} xfs_log_callback_t;


#ifdef __KERNEL__
/*                        */
struct xfs_mount;
struct xlog_in_core;
struct xlog_ticket;
struct xfs_log_item;
struct xfs_item_ops;
struct xfs_trans;

void	xfs_log_item_init(struct xfs_mount	*mp,
			struct xfs_log_item	*item,
			int			type,
			const struct xfs_item_ops *ops);

xfs_lsn_t xfs_log_done(struct xfs_mount *mp,
		       struct xlog_ticket *ticket,
		       struct xlog_in_core **iclog,
		       uint		flags);
int	  _xfs_log_force(struct xfs_mount *mp,
			 uint		flags,
			 int		*log_forced);
void	  xfs_log_force(struct xfs_mount	*mp,
			uint			flags);
int	  _xfs_log_force_lsn(struct xfs_mount *mp,
			     xfs_lsn_t		lsn,
			     uint		flags,
			     int		*log_forced);
void	  xfs_log_force_lsn(struct xfs_mount	*mp,
			    xfs_lsn_t		lsn,
			    uint		flags);
int	  xfs_log_mount(struct xfs_mount	*mp,
			struct xfs_buftarg	*log_target,
			xfs_daddr_t		start_block,
			int		 	num_bblocks);
int	  xfs_log_mount_finish(struct xfs_mount *mp);
xfs_lsn_t xlog_assign_tail_lsn(struct xfs_mount *mp);
xfs_lsn_t xlog_assign_tail_lsn_locked(struct xfs_mount *mp);
void	  xfs_log_space_wake(struct xfs_mount *mp);
int	  xfs_log_notify(struct xfs_mount	*mp,
			 struct xlog_in_core	*iclog,
			 xfs_log_callback_t	*callback_entry);
int	  xfs_log_release_iclog(struct xfs_mount *mp,
			 struct xlog_in_core	 *iclog);
int	  xfs_log_reserve(struct xfs_mount *mp,
			  int		   length,
			  int		   count,
			  struct xlog_ticket **ticket,
			  __uint8_t	   clientid,
			  bool		   permanent,
			  uint		   t_type);
int	  xfs_log_regrant(struct xfs_mount *mp, struct xlog_ticket *tic);
int	  xfs_log_unmount_write(struct xfs_mount *mp);
void      xfs_log_unmount(struct xfs_mount *mp);
int	  xfs_log_force_umount(struct xfs_mount *mp, int logerror);
int	  xfs_log_need_covered(struct xfs_mount *mp);

void	  xlog_iodone(struct xfs_buf *);

struct xlog_ticket *xfs_log_ticket_get(struct xlog_ticket *ticket);
void	  xfs_log_ticket_put(struct xlog_ticket *ticket);

int	xfs_log_commit_cil(struct xfs_mount *mp, struct xfs_trans *tp,
				xfs_lsn_t *commit_lsn, int flags);
bool	xfs_log_item_in_current_chkpt(struct xfs_log_item *lip);

void	xfs_log_work_queue(struct xfs_mount *mp);
void	xfs_log_worker(struct work_struct *work);
void	xfs_log_quiesce(struct xfs_mount *mp);

#endif
#endif	/*               */
