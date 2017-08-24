/*
 * Copyright 2000 by Hans Reiser, licensing governed by reiserfs/README
 */

/*                                                                     */
/*                                                                     */
/*                                                            */
/*                                                          */
/*                                                */

/* 
                            
                
              
   
  */

#include <asm/uaccess.h>
#include <linux/time.h>
#include "reiserfs.h"
#include <linux/buffer_head.h>
#include <linux/kernel.h>

static inline void buffer_info_init_left(struct tree_balance *tb,
                                         struct buffer_info *bi)
{
	bi->tb          = tb;
	bi->bi_bh       = tb->L[0];
	bi->bi_parent   = tb->FL[0];
	bi->bi_position = get_left_neighbor_position(tb, 0);
}

static inline void buffer_info_init_right(struct tree_balance *tb,
                                          struct buffer_info *bi)
{
	bi->tb          = tb;
	bi->bi_bh       = tb->R[0];
	bi->bi_parent   = tb->FR[0];
	bi->bi_position = get_right_neighbor_position(tb, 0);
}

static inline void buffer_info_init_tbS0(struct tree_balance *tb,
                                         struct buffer_info *bi)
{
	bi->tb          = tb;
	bi->bi_bh        = PATH_PLAST_BUFFER(tb->tb_path);
	bi->bi_parent   = PATH_H_PPARENT(tb->tb_path, 0);
	bi->bi_position = PATH_H_POSITION(tb->tb_path, 1);
}

static inline void buffer_info_init_bh(struct tree_balance *tb,
                                       struct buffer_info *bi,
                                       struct buffer_head *bh)
{
	bi->tb          = tb;
	bi->bi_bh       = bh;
	bi->bi_parent   = NULL;
	bi->bi_position = 0;
}

inline void do_balance_mark_leaf_dirty(struct tree_balance *tb,
				       struct buffer_head *bh, int flag)
{
	journal_mark_dirty(tb->transaction_handle,
			   tb->transaction_handle->t_super, bh);
}

#define do_balance_mark_internal_dirty do_balance_mark_leaf_dirty
#define do_balance_mark_sb_dirty do_balance_mark_leaf_dirty

/*         
                                                 
                                                            
     
                                                           
                                                            
                                                              
                                                              
                                                  

                                                                  
                                                                  
                                                                      
                                                                 
                                                                       

                                                                   
                                                                      
                                                                      
                           

      */

/*                                                               
  
                                   
                                                   
                                                         
                                                                                     
 */
static int balance_leaf_when_delete(struct tree_balance *tb, int flag)
{
	struct buffer_head *tbS0 = PATH_PLAST_BUFFER(tb->tb_path);
	int item_pos = PATH_LAST_POSITION(tb->tb_path);
	int pos_in_item = tb->tb_path->pos_in_item;
	struct buffer_info bi;
	int n;
	struct item_head *ih;

	RFALSE(tb->FR[0] && B_LEVEL(tb->FR[0]) != DISK_LEAF_NODE_LEVEL + 1,
	       "vs- 12000: level: wrong FR %z", tb->FR[0]);
	RFALSE(tb->blknum[0] > 1,
	       "PAP-12005: tb->blknum == %d, can not be > 1", tb->blknum[0]);
	RFALSE(!tb->blknum[0] && !PATH_H_PPARENT(tb->tb_path, 0),
	       "PAP-12010: tree can not be empty");

	ih = B_N_PITEM_HEAD(tbS0, item_pos);
	buffer_info_init_tbS0(tb, &bi);

	/*                             */

	switch (flag) {
	case M_DELETE:		/*                     */

		RFALSE(ih_item_len(ih) + IH_SIZE != -tb->insert_size[0],
		       "vs-12013: mode Delete, insert size %d, ih to be deleted %h",
		       -tb->insert_size[0], ih);

		leaf_delete_items(&bi, 0, item_pos, 1, -1);

		if (!item_pos && tb->CFL[0]) {
			if (B_NR_ITEMS(tbS0)) {
				replace_key(tb, tb->CFL[0], tb->lkey[0], tbS0,
					    0);
			} else {
				if (!PATH_H_POSITION(tb->tb_path, 1))
					replace_key(tb, tb->CFL[0], tb->lkey[0],
						    PATH_H_PPARENT(tb->tb_path,
								   0), 0);
			}
		}

		RFALSE(!item_pos && !tb->CFL[0],
		       "PAP-12020: tb->CFL[0]==%p, tb->L[0]==%p", tb->CFL[0],
		       tb->L[0]);

		break;

	case M_CUT:{		/*                  */
			if (is_direntry_le_ih(ih)) {

				/*                                                                                       */
				/*                                                                                         */
				tb->insert_size[0] = -1;
				leaf_cut_from_buffer(&bi, item_pos, pos_in_item,
						     -tb->insert_size[0]);

				RFALSE(!item_pos && !pos_in_item && !tb->CFL[0],
				       "PAP-12030: can not change delimiting key. CFL[0]=%p",
				       tb->CFL[0]);

				if (!item_pos && !pos_in_item && tb->CFL[0]) {
					replace_key(tb, tb->CFL[0], tb->lkey[0],
						    tbS0, 0);
				}
			} else {
				leaf_cut_from_buffer(&bi, item_pos, pos_in_item,
						     -tb->insert_size[0]);

				RFALSE(!ih_item_len(ih),
				       "PAP-12035: cut must leave non-zero dynamic length of item");
			}
			break;
		}

	default:
		print_cur_tb("12040");
		reiserfs_panic(tb->tb_sb, "PAP-12040",
			       "unexpected mode: %s(%d)",
			       (flag ==
				M_PASTE) ? "PASTE" : ((flag ==
						       M_INSERT) ? "INSERT" :
						      "UNKNOWN"), flag);
	}

	/*                                                                            */
	n = B_NR_ITEMS(tbS0);
	if (tb->lnum[0]) {	/*                              */
		if (tb->lnum[0] == -1) {	/*                               */
			if (tb->rnum[0] == -1) {	/*                                    */
				if (tb->FR[0] == PATH_H_PPARENT(tb->tb_path, 0)) {
					/*                                                   */
					if (PATH_H_POSITION(tb->tb_path, 1) == 0
					    && 1 < B_NR_ITEMS(tb->FR[0]))
						replace_key(tb, tb->CFL[0],
							    tb->lkey[0],
							    tb->FR[0], 1);

					leaf_move_items(LEAF_FROM_S_TO_L, tb, n,
							-1, NULL);
					leaf_move_items(LEAF_FROM_R_TO_L, tb,
							B_NR_ITEMS(tb->R[0]),
							-1, NULL);

					reiserfs_invalidate_buffer(tb, tbS0);
					reiserfs_invalidate_buffer(tb,
								   tb->R[0]);

					return 0;
				}
				/*                                                   */
				leaf_move_items(LEAF_FROM_S_TO_R, tb, n, -1,
						NULL);
				leaf_move_items(LEAF_FROM_L_TO_R, tb,
						B_NR_ITEMS(tb->L[0]), -1, NULL);

				/*                                         */
				replace_key(tb, tb->CFR[0], tb->rkey[0],
					    tb->R[0], 0);

				reiserfs_invalidate_buffer(tb, tbS0);
				reiserfs_invalidate_buffer(tb, tb->L[0]);

				return -1;
			}

			RFALSE(tb->rnum[0] != 0,
			       "PAP-12045: rnum must be 0 (%d)", tb->rnum[0]);
			/*                                               */
			leaf_shift_left(tb, n, -1);

			reiserfs_invalidate_buffer(tb, tbS0);

			return 0;
		}
		/*                                                                                      */

		RFALSE((tb->lnum[0] + tb->rnum[0] < n) ||
		       (tb->lnum[0] + tb->rnum[0] > n + 1),
		       "PAP-12050: rnum(%d) and lnum(%d) and item number(%d) in S[0] are not consistent",
		       tb->rnum[0], tb->lnum[0], n);
		RFALSE((tb->lnum[0] + tb->rnum[0] == n) &&
		       (tb->lbytes != -1 || tb->rbytes != -1),
		       "PAP-12055: bad rbytes (%d)/lbytes (%d) parameters when items are not split",
		       tb->rbytes, tb->lbytes);
		RFALSE((tb->lnum[0] + tb->rnum[0] == n + 1) &&
		       (tb->lbytes < 1 || tb->rbytes != -1),
		       "PAP-12060: bad rbytes (%d)/lbytes (%d) parameters when items are split",
		       tb->rbytes, tb->lbytes);

		leaf_shift_left(tb, tb->lnum[0], tb->lbytes);
		leaf_shift_right(tb, tb->rnum[0], tb->rbytes);

		reiserfs_invalidate_buffer(tb, tbS0);

		return 0;
	}

	if (tb->rnum[0] == -1) {
		/*                                               */
		leaf_shift_right(tb, n, -1);
		reiserfs_invalidate_buffer(tb, tbS0);
		return 0;
	}

	RFALSE(tb->rnum[0],
	       "PAP-12065: bad rnum parameter must be 0 (%d)", tb->rnum[0]);
	return 0;
}

static int balance_leaf(struct tree_balance *tb, struct item_head *ih,	/*                                                         */
			const char *body,	/*                                          */
			int flag,	/*                                           
                                    */
			struct item_head *insert_key,	/*                                                           
                                                                      
                                                               
                   */
			struct buffer_head **insert_ptr	/*                                       */
    )
{
	struct buffer_head *tbS0 = PATH_PLAST_BUFFER(tb->tb_path);
	int item_pos = PATH_LAST_POSITION(tb->tb_path);	/*                                              
                               */
	struct buffer_info bi;
	struct buffer_head *S_new[2];	/*                                                       */
	int snum[2];		/*                                    
                                             
              */
	int sbytes[2];		/*                                                
                                
                                                                            
           
                                                                          
     */
	int n, i;
	int ret_val;
	int pos_in_item;
	int zeros_num;

	PROC_INFO_INC(tb->tb_sb, balance_at[0]);

	/*                                         */
	if (tb->insert_size[0] < 0)
		return balance_leaf_when_delete(tb, flag);

	zeros_num = 0;
	if (flag == M_INSERT && !body)
		zeros_num = ih_item_len(ih);

	pos_in_item = tb->tb_path->pos_in_item;
	/*                                                              
                                   */
	if (flag != M_INSERT
	    && is_indirect_le_ih(B_N_PITEM_HEAD(tbS0, item_pos)))
		pos_in_item *= UNFM_P_SIZE;

	if (tb->lnum[0] > 0) {
		/*                                                         */
		if (item_pos < tb->lnum[0]) {
			/*                                                 */
			n = B_NR_ITEMS(tb->L[0]);

			switch (flag) {
			case M_INSERT:	/*                       */

				if (item_pos == tb->lnum[0] - 1
				    && tb->lbytes != -1) {
					/*                                  */
					int new_item_len;
					int version;

					ret_val =
					    leaf_shift_left(tb, tb->lnum[0] - 1,
							    -1);

					/*                                         */
					new_item_len =
					    ih_item_len(ih) - tb->lbytes;
					/*                                                   */
					put_ih_item_len(ih,
							ih_item_len(ih) -
							new_item_len);

					RFALSE(ih_item_len(ih) <= 0,
					       "PAP-12080: there is nothing to insert into L[0]: ih_item_len=%d",
					       ih_item_len(ih));

					/*                           */
					buffer_info_init_left(tb, &bi);
					leaf_insert_into_buf(&bi,
							     n + item_pos -
							     ret_val, ih, body,
							     zeros_num >
							     ih_item_len(ih) ?
							     ih_item_len(ih) :
							     zeros_num);

					version = ih_version(ih);

					/*                                                                   */
					set_le_ih_k_offset(ih,
							   le_ih_k_offset(ih) +
							   (tb->
							    lbytes <<
							    (is_indirect_le_ih
							     (ih) ? tb->tb_sb->
							     s_blocksize_bits -
							     UNFM_P_SHIFT :
							     0)));

					put_ih_item_len(ih, new_item_len);
					if (tb->lbytes > zeros_num) {
						body +=
						    (tb->lbytes - zeros_num);
						zeros_num = 0;
					} else
						zeros_num -= tb->lbytes;

					RFALSE(ih_item_len(ih) <= 0,
					       "PAP-12085: there is nothing to insert into S[0]: ih_item_len=%d",
					       ih_item_len(ih));
				} else {
					/*                                   */
					/*                               */
					ret_val =
					    leaf_shift_left(tb, tb->lnum[0] - 1,
							    tb->lbytes);
					/*                           */
					buffer_info_init_left(tb, &bi);
					leaf_insert_into_buf(&bi,
							     n + item_pos -
							     ret_val, ih, body,
							     zeros_num);
					tb->insert_size[0] = 0;
					zeros_num = 0;
				}
				break;

			case M_PASTE:	/*                     */

				if (item_pos == tb->lnum[0] - 1
				    && tb->lbytes != -1) {
					/*                                             */
					if (is_direntry_le_ih
					    (B_N_PITEM_HEAD(tbS0, item_pos))) {

						RFALSE(zeros_num,
						       "PAP-12090: invalid parameter in case of a directory");
						/*                */
						if (tb->lbytes > pos_in_item) {
							/*                                     */
							struct item_head
							    *pasted;
							int l_pos_in_item =
							    pos_in_item;

							/*                                                                                      */
							ret_val =
							    leaf_shift_left(tb,
									    tb->
									    lnum
									    [0],
									    tb->
									    lbytes
									    -
									    1);
							if (ret_val
							    && !item_pos) {
								pasted =
								    B_N_PITEM_HEAD
								    (tb->L[0],
								     B_NR_ITEMS
								     (tb->
								      L[0]) -
								     1);
								l_pos_in_item +=
								    I_ENTRY_COUNT
								    (pasted) -
								    (tb->
								     lbytes -
								     1);
							}

							/*                                                */
							buffer_info_init_left(tb, &bi);
							leaf_paste_in_buffer
							    (&bi,
							     n + item_pos -
							     ret_val,
							     l_pos_in_item,
							     tb->insert_size[0],
							     body, zeros_num);

							/*                                                                                          */

							/*                                                                     */

							/*                                              */
							leaf_paste_entries(&bi,
									   n +
									   item_pos
									   -
									   ret_val,
									   l_pos_in_item,
									   1,
									   (struct
									    reiserfs_de_head
									    *)
									   body,
									   body
									   +
									   DEH_SIZE,
									   tb->
									   insert_size
									   [0]
							    );
							tb->insert_size[0] = 0;
						} else {
							/*                                           */
							/*                                                                                                   */
							leaf_shift_left(tb,
									tb->
									lnum[0],
									tb->
									lbytes);
						}
						/*                                               */
						pos_in_item -= tb->lbytes;
					} else {
						/*                */
						RFALSE(tb->lbytes <= 0,
						       "PAP-12095: there is nothing to shift to L[0]. lbytes=%d",
						       tb->lbytes);
						RFALSE(pos_in_item !=
						       ih_item_len
						       (B_N_PITEM_HEAD
							(tbS0, item_pos)),
						       "PAP-12100: incorrect position to paste: item_len=%d, pos_in_item=%d",
						       ih_item_len
						       (B_N_PITEM_HEAD
							(tbS0, item_pos)),
						       pos_in_item);

						if (tb->lbytes >= pos_in_item) {
							/*                                        */
							int l_n;

							/*                                                             */
							l_n =
							    tb->lbytes -
							    pos_in_item;

							/*                              */
							tb->insert_size[0] -=
							    l_n;

							RFALSE(tb->
							       insert_size[0] <=
							       0,
							       "PAP-12105: there is nothing to paste into L[0]. insert_size=%d",
							       tb->
							       insert_size[0]);
							ret_val =
							    leaf_shift_left(tb,
									    tb->
									    lnum
									    [0],
									    ih_item_len
									    (B_N_PITEM_HEAD
									     (tbS0,
									      item_pos)));
							/*                                */
							buffer_info_init_left(tb, &bi);
							leaf_paste_in_buffer
							    (&bi,
							     n + item_pos -
							     ret_val,
							     ih_item_len
							     (B_N_PITEM_HEAD
							      (tb->L[0],
							       n + item_pos -
							       ret_val)), l_n,
							     body,
							     zeros_num >
							     l_n ? l_n :
							     zeros_num);
							/*                                                          */
							{
								int version;
								int temp_l =
								    l_n;

								RFALSE
								    (ih_item_len
								     (B_N_PITEM_HEAD
								      (tbS0,
								       0)),
								     "PAP-12106: item length must be 0");
								RFALSE
								    (comp_short_le_keys
								     (B_N_PKEY
								      (tbS0, 0),
								      B_N_PKEY
								      (tb->L[0],
								       n +
								       item_pos
								       -
								       ret_val)),
								     "PAP-12107: items must be of the same file");
								if (is_indirect_le_ih(B_N_PITEM_HEAD(tb->L[0], n + item_pos - ret_val))) {
									temp_l =
									    l_n
									    <<
									    (tb->
									     tb_sb->
									     s_blocksize_bits
									     -
									     UNFM_P_SHIFT);
								}
								/*                                */
								version =
								    ih_version
								    (B_N_PITEM_HEAD
								     (tbS0, 0));
								set_le_key_k_offset
								    (version,
								     B_N_PKEY
								     (tbS0, 0),
								     le_key_k_offset
								     (version,
								      B_N_PKEY
								      (tbS0,
								       0)) +
								     temp_l);
								/*                            */
								set_le_key_k_offset
								    (version,
								     B_N_PDELIM_KEY
								     (tb->
								      CFL[0],
								      tb->
								      lkey[0]),
								     le_key_k_offset
								     (version,
								      B_N_PDELIM_KEY
								      (tb->
								       CFL[0],
								       tb->
								       lkey[0]))
								     + temp_l);
							}

							/*                                                         */
							if (l_n > zeros_num) {
								body +=
								    (l_n -
								     zeros_num);
								zeros_num = 0;
							} else
								zeros_num -=
								    l_n;
							pos_in_item = 0;

							RFALSE
							    (comp_short_le_keys
							     (B_N_PKEY(tbS0, 0),
							      B_N_PKEY(tb->L[0],
								       B_NR_ITEMS
								       (tb->
									L[0]) -
								       1))
							     ||
							     !op_is_left_mergeable
							     (B_N_PKEY(tbS0, 0),
							      tbS0->b_size)
							     ||
							     !op_is_left_mergeable
							     (B_N_PDELIM_KEY
							      (tb->CFL[0],
							       tb->lkey[0]),
							      tbS0->b_size),
							     "PAP-12120: item must be merge-able with left neighboring item");
						} else {	/*                                                */

							/*                                               */
							pos_in_item -=
							    tb->lbytes;

							RFALSE(pos_in_item <= 0,
							       "PAP-12125: no place for paste. pos_in_item=%d",
							       pos_in_item);

							/*                                                                                  */
							leaf_shift_left(tb,
									tb->
									lnum[0],
									tb->
									lbytes);
						}
					}
				} else {	/*                                        */

					struct item_head *pasted;

					if (!item_pos && op_is_left_mergeable(B_N_PKEY(tbS0, 0), tbS0->b_size)) {	/*                                                             */
						/*                                                                 */
						pasted =
						    B_N_PITEM_HEAD(tb->L[0],
								   n - 1);
						if (is_direntry_le_ih(pasted))
							pos_in_item +=
							    ih_entry_count
							    (pasted);
						else
							pos_in_item +=
							    ih_item_len(pasted);
					}

					/*                                                                                  */
					ret_val =
					    leaf_shift_left(tb, tb->lnum[0],
							    tb->lbytes);
					/*                                */
					buffer_info_init_left(tb, &bi);
					leaf_paste_in_buffer(&bi,
							     n + item_pos -
							     ret_val,
							     pos_in_item,
							     tb->insert_size[0],
							     body, zeros_num);

					/*                                            */
					pasted =
					    B_N_PITEM_HEAD(tb->L[0],
							   n + item_pos -
							   ret_val);
					if (is_direntry_le_ih(pasted))
						leaf_paste_entries(&bi,
								   n +
								   item_pos -
								   ret_val,
								   pos_in_item,
								   1,
								   (struct
								    reiserfs_de_head
								    *)body,
								   body +
								   DEH_SIZE,
								   tb->
								   insert_size
								   [0]
						    );
					/*                                                                      */
					if (is_indirect_le_ih(pasted))
						set_ih_free_space(pasted, 0);
					tb->insert_size[0] = 0;
					zeros_num = 0;
				}
				break;
			default:	/*               */
				reiserfs_panic(tb->tb_sb, "PAP-12130",
					       "lnum > 0: unexpected mode: "
					       " %s(%d)",
					       (flag ==
						M_DELETE) ? "DELETE" : ((flag ==
									 M_CUT)
									? "CUT"
									:
									"UNKNOWN"),
					       flag);
			}
		} else {
			/*                                 */
			leaf_shift_left(tb, tb->lnum[0], tb->lbytes);
		}
	}

	/*                 */
	/*                             */
	item_pos -= (tb->lnum[0] - ((tb->lbytes != -1) ? 1 : 0));

	if (tb->rnum[0] > 0) {
		/*                                                          */
		n = B_NR_ITEMS(tbS0);
		switch (flag) {

		case M_INSERT:	/*             */
			if (n - tb->rnum[0] < item_pos) {	/*                                    */
				if (item_pos == n - tb->rnum[0] + 1 && tb->rbytes != -1) {	/*                                  */
					loff_t old_key_comp, old_len,
					    r_zeros_number;
					const char *r_body;
					int version;
					loff_t offset;

					leaf_shift_right(tb, tb->rnum[0] - 1,
							 -1);

					version = ih_version(ih);
					/*                                        */
					old_key_comp = le_ih_k_offset(ih);
					old_len = ih_item_len(ih);

					/*                                                             */
					offset =
					    le_ih_k_offset(ih) +
					    ((old_len -
					      tb->
					      rbytes) << (is_indirect_le_ih(ih)
							  ? tb->tb_sb->
							  s_blocksize_bits -
							  UNFM_P_SHIFT : 0));
					set_le_ih_k_offset(ih, offset);
					put_ih_item_len(ih, tb->rbytes);
					/*                                   */
					buffer_info_init_right(tb, &bi);
					if ((old_len - tb->rbytes) > zeros_num) {
						r_zeros_number = 0;
						r_body =
						    body + (old_len -
							    tb->rbytes) -
						    zeros_num;
					} else {
						r_body = body;
						r_zeros_number =
						    zeros_num - (old_len -
								 tb->rbytes);
						zeros_num -= r_zeros_number;
					}

					leaf_insert_into_buf(&bi, 0, ih, r_body,
							     r_zeros_number);

					/*                                                   */
					replace_key(tb, tb->CFR[0], tb->rkey[0],
						    tb->R[0], 0);

					/*                                                             */
					set_le_ih_k_offset(ih, old_key_comp);
					put_ih_item_len(ih,
							old_len - tb->rbytes);

					tb->insert_size[0] -= tb->rbytes;

				} else {	/*                                */

					/*                               */
					ret_val =
					    leaf_shift_right(tb,
							     tb->rnum[0] - 1,
							     tb->rbytes);
					/*                           */
					buffer_info_init_right(tb, &bi);
					leaf_insert_into_buf(&bi,
							     item_pos - n +
							     tb->rnum[0] - 1,
							     ih, body,
							     zeros_num);

					if (item_pos - n + tb->rnum[0] - 1 == 0) {
						replace_key(tb, tb->CFR[0],
							    tb->rkey[0],
							    tb->R[0], 0);

					}
					zeros_num = tb->insert_size[0] = 0;
				}
			} else {	/*                                               */

				leaf_shift_right(tb, tb->rnum[0], tb->rbytes);
			}
			break;

		case M_PASTE:	/*             */

			if (n - tb->rnum[0] <= item_pos) {	/*                                         */
				if (item_pos == n - tb->rnum[0] && tb->rbytes != -1) {	/*                                             */
					if (is_direntry_le_ih(B_N_PITEM_HEAD(tbS0, item_pos))) {	/*                             */
						int entry_count;

						RFALSE(zeros_num,
						       "PAP-12145: invalid parameter in case of a directory");
						entry_count =
						    I_ENTRY_COUNT(B_N_PITEM_HEAD
								  (tbS0,
								   item_pos));
						if (entry_count - tb->rbytes <
						    pos_in_item)
							/*                                     */
						{
							int paste_entry_position;

							RFALSE(tb->rbytes - 1 >=
							       entry_count
							       || !tb->
							       insert_size[0],
							       "PAP-12150: no enough of entries to shift to R[0]: rbytes=%d, entry_count=%d",
							       tb->rbytes,
							       entry_count);
							/*                                                                                                     */
							leaf_shift_right(tb,
									 tb->
									 rnum
									 [0],
									 tb->
									 rbytes
									 - 1);
							/*                                               */
							paste_entry_position =
							    pos_in_item -
							    entry_count +
							    tb->rbytes - 1;
							buffer_info_init_right(tb, &bi);
							leaf_paste_in_buffer
							    (&bi, 0,
							     paste_entry_position,
							     tb->insert_size[0],
							     body, zeros_num);
							/*             */
							leaf_paste_entries(&bi,
									   0,
									   paste_entry_position,
									   1,
									   (struct
									    reiserfs_de_head
									    *)
									   body,
									   body
									   +
									   DEH_SIZE,
									   tb->
									   insert_size
									   [0]
							    );

							if (paste_entry_position
							    == 0) {
								/*                        */
								replace_key(tb,
									    tb->
									    CFR
									    [0],
									    tb->
									    rkey
									    [0],
									    tb->
									    R
									    [0],
									    0);
							}

							tb->insert_size[0] = 0;
							pos_in_item++;
						} else {	/*                                            */

							leaf_shift_right(tb,
									 tb->
									 rnum
									 [0],
									 tb->
									 rbytes);
						}
					} else {	/*                */

						int n_shift, n_rem,
						    r_zeros_number;
						const char *r_body;

						/*                                                                    */
						if ((n_shift =
						     tb->rbytes -
						     tb->insert_size[0]) < 0)
							n_shift = 0;

						RFALSE(pos_in_item !=
						       ih_item_len
						       (B_N_PITEM_HEAD
							(tbS0, item_pos)),
						       "PAP-12155: invalid position to paste. ih_item_len=%d, pos_in_item=%d",
						       pos_in_item,
						       ih_item_len
						       (B_N_PITEM_HEAD
							(tbS0, item_pos)));

						leaf_shift_right(tb,
								 tb->rnum[0],
								 n_shift);
						/*                                                                             */
						if ((n_rem =
						     tb->insert_size[0] -
						     tb->rbytes) < 0)
							n_rem = 0;

						{
							int version;
							unsigned long temp_rem =
							    n_rem;

							version =
							    ih_version
							    (B_N_PITEM_HEAD
							     (tb->R[0], 0));
							if (is_indirect_le_key
							    (version,
							     B_N_PKEY(tb->R[0],
								      0))) {
								temp_rem =
								    n_rem <<
								    (tb->tb_sb->
								     s_blocksize_bits
								     -
								     UNFM_P_SHIFT);
							}
							set_le_key_k_offset
							    (version,
							     B_N_PKEY(tb->R[0],
								      0),
							     le_key_k_offset
							     (version,
							      B_N_PKEY(tb->R[0],
								       0)) +
							     temp_rem);
							set_le_key_k_offset
							    (version,
							     B_N_PDELIM_KEY(tb->
									    CFR
									    [0],
									    tb->
									    rkey
									    [0]),
							     le_key_k_offset
							     (version,
							      B_N_PDELIM_KEY
							      (tb->CFR[0],
							       tb->rkey[0])) +
							     temp_rem);
						}
/*                                             
                                                               */
						do_balance_mark_internal_dirty
						    (tb, tb->CFR[0], 0);

						/*                               */
						buffer_info_init_right(tb, &bi);
						if (n_rem > zeros_num) {
							r_zeros_number = 0;
							r_body =
							    body + n_rem -
							    zeros_num;
						} else {
							r_body = body;
							r_zeros_number =
							    zeros_num - n_rem;
							zeros_num -=
							    r_zeros_number;
						}

						leaf_paste_in_buffer(&bi, 0,
								     n_shift,
								     tb->
								     insert_size
								     [0] -
								     n_rem,
								     r_body,
								     r_zeros_number);

						if (is_indirect_le_ih
						    (B_N_PITEM_HEAD
						     (tb->R[0], 0))) {
#if 0
							RFALSE(n_rem,
							       "PAP-12160: paste more than one unformatted node pointer");
#endif
							set_ih_free_space
							    (B_N_PITEM_HEAD
							     (tb->R[0], 0), 0);
						}
						tb->insert_size[0] = n_rem;
						if (!n_rem)
							pos_in_item++;
					}
				} else {	/*                                      */

					struct item_head *pasted;

					ret_val =
					    leaf_shift_right(tb, tb->rnum[0],
							     tb->rbytes);
					/*                     */
					if (pos_in_item >= 0) {
						buffer_info_init_right(tb, &bi);
						leaf_paste_in_buffer(&bi,
								     item_pos -
								     n +
								     tb->
								     rnum[0],
								     pos_in_item,
								     tb->
								     insert_size
								     [0], body,
								     zeros_num);
					}

					/*                                            */
					pasted =
					    B_N_PITEM_HEAD(tb->R[0],
							   item_pos - n +
							   tb->rnum[0]);
					if (is_direntry_le_ih(pasted)
					    && pos_in_item >= 0) {
						leaf_paste_entries(&bi,
								   item_pos -
								   n +
								   tb->rnum[0],
								   pos_in_item,
								   1,
								   (struct
								    reiserfs_de_head
								    *)body,
								   body +
								   DEH_SIZE,
								   tb->
								   insert_size
								   [0]
						    );
						if (!pos_in_item) {

							RFALSE(item_pos - n +
							       tb->rnum[0],
							       "PAP-12165: directory item must be first item of node when pasting is in 0th position");

							/*                        */
							replace_key(tb,
								    tb->CFR[0],
								    tb->rkey[0],
								    tb->R[0],
								    0);
						}
					}

					if (is_indirect_le_ih(pasted))
						set_ih_free_space(pasted, 0);
					zeros_num = tb->insert_size[0] = 0;
				}
			} else {	/*                                 */

				leaf_shift_right(tb, tb->rnum[0], tb->rbytes);
			}
			break;
		default:	/*               */
			reiserfs_panic(tb->tb_sb, "PAP-12175",
				       "rnum > 0: unexpected mode: %s(%d)",
				       (flag ==
					M_DELETE) ? "DELETE" : ((flag ==
								 M_CUT) ? "CUT"
								: "UNKNOWN"),
				       flag);
		}

	}

	/*                 */
	RFALSE(tb->blknum[0] > 3,
	       "PAP-12180: blknum can not be %d. It must be <= 3",
	       tb->blknum[0]);
	RFALSE(tb->blknum[0] < 0,
	       "PAP-12185: blknum can not be %d. It must be >= 0",
	       tb->blknum[0]);

	/*                                                                   
                                                                     
                                                             */
	if (tb->blknum[0] == 0) {	/*                        */

		RFALSE(!tb->lnum[0] || !tb->rnum[0],
		       "PAP-12190: lnum and rnum must not be zero");
		/*                                                          
                                                                 
                       */
		if (tb->CFL[0]) {
			if (!tb->CFR[0])
				reiserfs_panic(tb->tb_sb, "vs-12195",
					       "CFR not initialized");
			copy_key(B_N_PDELIM_KEY(tb->CFL[0], tb->lkey[0]),
				 B_N_PDELIM_KEY(tb->CFR[0], tb->rkey[0]));
			do_balance_mark_internal_dirty(tb, tb->CFL[0], 0);
		}

		reiserfs_invalidate_buffer(tb, tbS0);
		return 0;
	}

	/*                                             */

	/*                                                                  
                            */
	snum[0] = tb->s1num, snum[1] = tb->s2num;
	sbytes[0] = tb->s1bytes;
	sbytes[1] = tb->s2bytes;
	for (i = tb->blknum[0] - 2; i >= 0; i--) {

		RFALSE(!snum[i], "PAP-12200: snum[%d] == %d. Must be > 0", i,
		       snum[i]);

		/*                                     */

		S_new[i] = get_FEB(tb);

		/*                                       */
		set_blkh_level(B_BLK_HEAD(S_new[i]), DISK_LEAF_NODE_LEVEL);

		n = B_NR_ITEMS(tbS0);

		switch (flag) {
		case M_INSERT:	/*             */

			if (n - snum[i] < item_pos) {	/*                                                        */
				if (item_pos == n - snum[i] + 1 && sbytes[i] != -1) {	/*                                      */
					int old_key_comp, old_len,
					    r_zeros_number;
					const char *r_body;
					int version;

					/*                                            */
					leaf_move_items(LEAF_FROM_S_TO_SNEW, tb,
							snum[i] - 1, -1,
							S_new[i]);
					/*                                        */
					version = ih_version(ih);
					old_key_comp = le_ih_k_offset(ih);
					old_len = ih_item_len(ih);

					/*                                                                 */
					set_le_ih_k_offset(ih,
							   le_ih_k_offset(ih) +
							   ((old_len -
							     sbytes[i]) <<
							    (is_indirect_le_ih
							     (ih) ? tb->tb_sb->
							     s_blocksize_bits -
							     UNFM_P_SHIFT :
							     0)));

					put_ih_item_len(ih, sbytes[i]);

					/*                                                        */
					buffer_info_init_bh(tb, &bi, S_new[i]);

					if ((old_len - sbytes[i]) > zeros_num) {
						r_zeros_number = 0;
						r_body =
						    body + (old_len -
							    sbytes[i]) -
						    zeros_num;
					} else {
						r_body = body;
						r_zeros_number =
						    zeros_num - (old_len -
								 sbytes[i]);
						zeros_num -= r_zeros_number;
					}

					leaf_insert_into_buf(&bi, 0, ih, r_body,
							     r_zeros_number);

					/*                                                             */
					set_le_ih_k_offset(ih, old_key_comp);
					put_ih_item_len(ih,
							old_len - sbytes[i]);
					tb->insert_size[0] -= sbytes[i];
				} else {	/*                                    */

					/*                                                               */
					leaf_move_items(LEAF_FROM_S_TO_SNEW, tb,
							snum[i] - 1, sbytes[i],
							S_new[i]);

					/*                               */
					buffer_info_init_bh(tb, &bi, S_new[i]);
					leaf_insert_into_buf(&bi,
							     item_pos - n +
							     snum[i] - 1, ih,
							     body, zeros_num);

					zeros_num = tb->insert_size[0] = 0;
				}
			}

			else {	/*                                               */

				leaf_move_items(LEAF_FROM_S_TO_SNEW, tb,
						snum[i], sbytes[i], S_new[i]);
			}
			break;

		case M_PASTE:	/*             */

			if (n - snum[i] <= item_pos) {	/*                                             */
				if (item_pos == n - snum[i] && sbytes[i] != -1) {	/*                                         */
					struct item_head *aux_ih;

					RFALSE(ih, "PAP-12210: ih must be 0");

					aux_ih = B_N_PITEM_HEAD(tbS0, item_pos);
					if (is_direntry_le_ih(aux_ih)) {
						/*                             */

						int entry_count;

						entry_count =
						    ih_entry_count(aux_ih);

						if (entry_count - sbytes[i] <
						    pos_in_item
						    && pos_in_item <=
						    entry_count) {
							/*                                         */

							RFALSE(!tb->
							       insert_size[0],
							       "PAP-12215: insert_size is already 0");
							RFALSE(sbytes[i] - 1 >=
							       entry_count,
							       "PAP-12220: there are no so much entries (%d), only %d",
							       sbytes[i] - 1,
							       entry_count);

							/*                                                                                                      */
							leaf_move_items
							    (LEAF_FROM_S_TO_SNEW,
							     tb, snum[i],
							     sbytes[i] - 1,
							     S_new[i]);
							/*                                               */
							buffer_info_init_bh(tb, &bi, S_new[i]);
							leaf_paste_in_buffer
							    (&bi, 0,
							     pos_in_item -
							     entry_count +
							     sbytes[i] - 1,
							     tb->insert_size[0],
							     body, zeros_num);
							/*                           */
							leaf_paste_entries(&bi,
									   0,
									   pos_in_item
									   -
									   entry_count
									   +
									   sbytes
									   [i] -
									   1, 1,
									   (struct
									    reiserfs_de_head
									    *)
									   body,
									   body
									   +
									   DEH_SIZE,
									   tb->
									   insert_size
									   [0]
							    );
							tb->insert_size[0] = 0;
							pos_in_item++;
						} else {	/*                                                */
							leaf_move_items
							    (LEAF_FROM_S_TO_SNEW,
							     tb, snum[i],
							     sbytes[i],
							     S_new[i]);
						}
					} else {	/*                */

						int n_shift, n_rem,
						    r_zeros_number;
						const char *r_body;

						RFALSE(pos_in_item !=
						       ih_item_len
						       (B_N_PITEM_HEAD
							(tbS0, item_pos))
						       || tb->insert_size[0] <=
						       0,
						       "PAP-12225: item too short or insert_size <= 0");

						/*                                                                    */
						n_shift =
						    sbytes[i] -
						    tb->insert_size[0];
						if (n_shift < 0)
							n_shift = 0;
						leaf_move_items
						    (LEAF_FROM_S_TO_SNEW, tb,
						     snum[i], n_shift,
						     S_new[i]);

						/*                                                                              */
						n_rem =
						    tb->insert_size[0] -
						    sbytes[i];
						if (n_rem < 0)
							n_rem = 0;
						/*                                   */
						buffer_info_init_bh(tb, &bi, S_new[i]);
						if (n_rem > zeros_num) {
							r_zeros_number = 0;
							r_body =
							    body + n_rem -
							    zeros_num;
						} else {
							r_body = body;
							r_zeros_number =
							    zeros_num - n_rem;
							zeros_num -=
							    r_zeros_number;
						}

						leaf_paste_in_buffer(&bi, 0,
								     n_shift,
								     tb->
								     insert_size
								     [0] -
								     n_rem,
								     r_body,
								     r_zeros_number);
						{
							struct item_head *tmp;

							tmp =
							    B_N_PITEM_HEAD(S_new
									   [i],
									   0);
							if (is_indirect_le_ih
							    (tmp)) {
								set_ih_free_space
								    (tmp, 0);
								set_le_ih_k_offset
								    (tmp,
								     le_ih_k_offset
								     (tmp) +
								     (n_rem <<
								      (tb->
								       tb_sb->
								       s_blocksize_bits
								       -
								       UNFM_P_SHIFT)));
							} else {
								set_le_ih_k_offset
								    (tmp,
								     le_ih_k_offset
								     (tmp) +
								     n_rem);
							}
						}

						tb->insert_size[0] = n_rem;
						if (!n_rem)
							pos_in_item++;
					}
				} else
					/*                                 */
				{
					int leaf_mi;
					struct item_head *pasted;

#ifdef CONFIG_REISERFS_CHECK
					struct item_head *ih_check =
					    B_N_PITEM_HEAD(tbS0, item_pos);

					if (!is_direntry_le_ih(ih_check)
					    && (pos_in_item != ih_item_len(ih_check)
						|| tb->insert_size[0] <= 0))
						reiserfs_panic(tb->tb_sb,
							     "PAP-12235",
							     "pos_in_item "
							     "must be equal "
							     "to ih_item_len");
#endif				/*                       */

					leaf_mi =
					    leaf_move_items(LEAF_FROM_S_TO_SNEW,
							    tb, snum[i],
							    sbytes[i],
							    S_new[i]);

					RFALSE(leaf_mi,
					       "PAP-12240: unexpected value returned by leaf_move_items (%d)",
					       leaf_mi);

					/*                 */
					buffer_info_init_bh(tb, &bi, S_new[i]);
					leaf_paste_in_buffer(&bi,
							     item_pos - n +
							     snum[i],
							     pos_in_item,
							     tb->insert_size[0],
							     body, zeros_num);

					pasted =
					    B_N_PITEM_HEAD(S_new[i],
							   item_pos - n +
							   snum[i]);
					if (is_direntry_le_ih(pasted)) {
						leaf_paste_entries(&bi,
								   item_pos -
								   n + snum[i],
								   pos_in_item,
								   1,
								   (struct
								    reiserfs_de_head
								    *)body,
								   body +
								   DEH_SIZE,
								   tb->
								   insert_size
								   [0]
						    );
					}

					/*                                                   */
					if (is_indirect_le_ih(pasted))
						set_ih_free_space(pasted, 0);
					zeros_num = tb->insert_size[0] = 0;
				}
			}

			else {	/*                                        */

				leaf_move_items(LEAF_FROM_S_TO_SNEW, tb,
						snum[i], sbytes[i], S_new[i]);
			}
			break;
		default:	/*               */
			reiserfs_panic(tb->tb_sb, "PAP-12245",
				       "blknum > 2: unexpected mode: %s(%d)",
				       (flag ==
					M_DELETE) ? "DELETE" : ((flag ==
								 M_CUT) ? "CUT"
								: "UNKNOWN"),
				       flag);
		}

		memcpy(insert_key + i, B_N_PKEY(S_new[i], 0), KEY_SIZE);
		insert_ptr[i] = S_new[i];

		RFALSE(!buffer_journaled(S_new[i])
		       || buffer_journal_dirty(S_new[i])
		       || buffer_dirty(S_new[i]), "PAP-12247: S_new[%d] : (%b)",
		       i, S_new[i]);
	}

	/*                                                                                                                  
                                     */
	if (0 <= item_pos && item_pos < tb->s0num) {	/*                                              */

		switch (flag) {
		case M_INSERT:	/*                       */
			buffer_info_init_tbS0(tb, &bi);
			leaf_insert_into_buf(&bi, item_pos, ih, body,
					     zeros_num);

			/*                                                      */
			if (item_pos == 0) {
				if (tb->CFL[0])	/*                        */
					replace_key(tb, tb->CFL[0], tb->lkey[0],
						    tbS0, 0);

			}
			break;

		case M_PASTE:{	/*                     */
				struct item_head *pasted;

				pasted = B_N_PITEM_HEAD(tbS0, item_pos);
				/*                                                 */
				if (is_direntry_le_ih(pasted)) {
					if (pos_in_item >= 0 &&
					    pos_in_item <=
					    ih_entry_count(pasted)) {

						RFALSE(!tb->insert_size[0],
						       "PAP-12260: insert_size is 0 already");

						/*               */
						buffer_info_init_tbS0(tb, &bi);
						leaf_paste_in_buffer(&bi,
								     item_pos,
								     pos_in_item,
								     tb->
								     insert_size
								     [0], body,
								     zeros_num);

						/*             */
						leaf_paste_entries(&bi,
								   item_pos,
								   pos_in_item,
								   1,
								   (struct
								    reiserfs_de_head
								    *)body,
								   body +
								   DEH_SIZE,
								   tb->
								   insert_size
								   [0]
						    );
						if (!item_pos && !pos_in_item) {
							RFALSE(!tb->CFL[0]
							       || !tb->L[0],
							       "PAP-12270: CFL[0]/L[0] must be specified");
							if (tb->CFL[0]) {
								replace_key(tb,
									    tb->
									    CFL
									    [0],
									    tb->
									    lkey
									    [0],
									    tbS0,
									    0);

							}
						}
						tb->insert_size[0] = 0;
					}
				} else {	/*                */
					if (pos_in_item == ih_item_len(pasted)) {

						RFALSE(tb->insert_size[0] <= 0,
						       "PAP-12275: insert size must not be %d",
						       tb->insert_size[0]);
						buffer_info_init_tbS0(tb, &bi);
						leaf_paste_in_buffer(&bi,
								     item_pos,
								     pos_in_item,
								     tb->
								     insert_size
								     [0], body,
								     zeros_num);

						if (is_indirect_le_ih(pasted)) {
#if 0
							RFALSE(tb->
							       insert_size[0] !=
							       UNFM_P_SIZE,
							       "PAP-12280: insert_size for indirect item must be %d, not %d",
							       UNFM_P_SIZE,
							       tb->
							       insert_size[0]);
#endif
							set_ih_free_space
							    (pasted, 0);
						}
						tb->insert_size[0] = 0;
					}
#ifdef CONFIG_REISERFS_CHECK
					else {
						if (tb->insert_size[0]) {
							print_cur_tb("12285");
							reiserfs_panic(tb->
								       tb_sb,
							    "PAP-12285",
							    "insert_size "
							    "must be 0 "
							    "(%d)",
							    tb->insert_size[0]);
						}
					}
#endif				/*                       */

				}
			}	/*               */
		}
	}
#ifdef CONFIG_REISERFS_CHECK
	if (flag == M_PASTE && tb->insert_size[0]) {
		print_cur_tb("12290");
		reiserfs_panic(tb->tb_sb,
			       "PAP-12290", "insert_size is still not 0 (%d)",
			       tb->insert_size[0]);
	}
#endif				/*                       */
	return 0;
}				/*                                                          */

/*                 */
void make_empty_node(struct buffer_info *bi)
{
	struct block_head *blkh;

	RFALSE(bi->bi_bh == NULL, "PAP-12295: pointer to the buffer is NULL");

	blkh = B_BLK_HEAD(bi->bi_bh);
	set_blkh_nr_item(blkh, 0);
	set_blkh_free_space(blkh, MAX_CHILD_SIZE(bi->bi_bh));

	if (bi->bi_parent)
		B_N_CHILD(bi->bi_parent, bi->bi_position)->dc_size = 0;	/*                  */
}

/*                        */
struct buffer_head *get_FEB(struct tree_balance *tb)
{
	int i;
	struct buffer_info bi;

	for (i = 0; i < MAX_FEB_SIZE; i++)
		if (tb->FEB[i] != NULL)
			break;

	if (i == MAX_FEB_SIZE)
		reiserfs_panic(tb->tb_sb, "vs-12300", "FEB list is empty");

	buffer_info_init_bh(tb, &bi, tb->FEB[i]);
	make_empty_node(&bi);
	set_buffer_uptodate(tb->FEB[i]);
	tb->used[i] = tb->FEB[i];
	tb->FEB[i] = NULL;

	return tb->used[i];
}

/*                                                               
            
*/
static void store_thrown(struct tree_balance *tb, struct buffer_head *bh)
{
	int i;

	if (buffer_dirty(bh))
		reiserfs_warning(tb->tb_sb, "reiserfs-12320",
				 "called with dirty buffer");
	for (i = 0; i < ARRAY_SIZE(tb->thrown); i++)
		if (!tb->thrown[i]) {
			tb->thrown[i] = bh;
			get_bh(bh);	/*                       */
			return;
		}
	reiserfs_warning(tb->tb_sb, "reiserfs-12321",
			 "too many thrown buffers");
}

static void free_thrown(struct tree_balance *tb)
{
	int i;
	b_blocknr_t blocknr;
	for (i = 0; i < ARRAY_SIZE(tb->thrown); i++) {
		if (tb->thrown[i]) {
			blocknr = tb->thrown[i]->b_blocknr;
			if (buffer_dirty(tb->thrown[i]))
				reiserfs_warning(tb->tb_sb, "reiserfs-12322",
						 "called with dirty buffer %d",
						 blocknr);
			brelse(tb->thrown[i]);	/*                             */
			reiserfs_free_block(tb->transaction_handle, NULL,
					    blocknr, 0);
		}
	}
}

void reiserfs_invalidate_buffer(struct tree_balance *tb, struct buffer_head *bh)
{
	struct block_head *blkh;
	blkh = B_BLK_HEAD(bh);
	set_blkh_level(blkh, FREE_LEVEL);
	set_blkh_nr_item(blkh, 0);

	clear_buffer_dirty(bh);
	store_thrown(tb, bh);
}

/*                                                                    */
void replace_key(struct tree_balance *tb, struct buffer_head *dest, int n_dest,
		 struct buffer_head *src, int n_src)
{

	RFALSE(dest == NULL || src == NULL,
	       "vs-12305: source or destination buffer is 0 (src=%p, dest=%p)",
	       src, dest);
	RFALSE(!B_IS_KEYS_LEVEL(dest),
	       "vs-12310: invalid level (%z) for destination buffer. dest must be leaf",
	       dest);
	RFALSE(n_dest < 0 || n_src < 0,
	       "vs-12315: src(%d) or dest(%d) key number < 0", n_src, n_dest);
	RFALSE(n_dest >= B_NR_ITEMS(dest) || n_src >= B_NR_ITEMS(src),
	       "vs-12320: src(%d(%d)) or dest(%d(%d)) key number is too big",
	       n_src, B_NR_ITEMS(src), n_dest, B_NR_ITEMS(dest));

	if (B_IS_ITEMS_LEVEL(src))
		/*                                  */
		memcpy(B_N_PDELIM_KEY(dest, n_dest), B_N_PITEM_HEAD(src, n_src),
		       KEY_SIZE);
	else
		memcpy(B_N_PDELIM_KEY(dest, n_dest), B_N_PDELIM_KEY(src, n_src),
		       KEY_SIZE);

	do_balance_mark_internal_dirty(tb, dest, 0);
}

int get_left_neighbor_position(struct tree_balance *tb, int h)
{
	int Sh_position = PATH_H_POSITION(tb->tb_path, h + 1);

	RFALSE(PATH_H_PPARENT(tb->tb_path, h) == NULL || tb->FL[h] == NULL,
	       "vs-12325: FL[%d](%p) or F[%d](%p) does not exist",
	       h, tb->FL[h], h, PATH_H_PPARENT(tb->tb_path, h));

	if (Sh_position == 0)
		return B_NR_ITEMS(tb->FL[h]);
	else
		return Sh_position - 1;
}

int get_right_neighbor_position(struct tree_balance *tb, int h)
{
	int Sh_position = PATH_H_POSITION(tb->tb_path, h + 1);

	RFALSE(PATH_H_PPARENT(tb->tb_path, h) == NULL || tb->FR[h] == NULL,
	       "vs-12330: F[%d](%p) or FR[%d](%p) does not exist",
	       h, PATH_H_PPARENT(tb->tb_path, h), h, tb->FR[h]);

	if (Sh_position == B_NR_ITEMS(PATH_H_PPARENT(tb->tb_path, h)))
		return 0;
	else
		return Sh_position + 1;
}

#ifdef CONFIG_REISERFS_CHECK

int is_reusable(struct super_block *s, b_blocknr_t block, int bit_value);
static void check_internal_node(struct super_block *s, struct buffer_head *bh,
				char *mes)
{
	struct disk_child *dc;
	int i;

	RFALSE(!bh, "PAP-12336: bh == 0");

	if (!bh || !B_IS_IN_TREE(bh))
		return;

	RFALSE(!buffer_dirty(bh) &&
	       !(buffer_journaled(bh) || buffer_journal_dirty(bh)),
	       "PAP-12337: buffer (%b) must be dirty", bh);
	dc = B_N_CHILD(bh, 0);

	for (i = 0; i <= B_NR_ITEMS(bh); i++, dc++) {
		if (!is_reusable(s, dc_block_number(dc), 1)) {
			print_cur_tb(mes);
			reiserfs_panic(s, "PAP-12338",
				       "invalid child pointer %y in %b",
				       dc, bh);
		}
	}
}

static int locked_or_not_in_tree(struct tree_balance *tb,
				  struct buffer_head *bh, char *which)
{
	if ((!buffer_journal_prepared(bh) && buffer_locked(bh)) ||
	    !B_IS_IN_TREE(bh)) {
		reiserfs_warning(tb->tb_sb, "vs-12339", "%s (%b)", which, bh);
		return 1;
	}
	return 0;
}

static int check_before_balancing(struct tree_balance *tb)
{
	int retval = 0;

	if (REISERFS_SB(tb->tb_sb)->cur_tb) {
		reiserfs_panic(tb->tb_sb, "vs-12335", "suspect that schedule "
			       "occurred based on cur_tb not being null at "
			       "this point in code. do_balance cannot properly "
			       "handle concurrent tree accesses on a same "
			       "mount point.");
	}

	/*                                                                                           
                                  */
	if (tb->lnum[0]) {
		retval |= locked_or_not_in_tree(tb, tb->L[0], "L[0]");
		retval |= locked_or_not_in_tree(tb, tb->FL[0], "FL[0]");
		retval |= locked_or_not_in_tree(tb, tb->CFL[0], "CFL[0]");
		check_leaf(tb->L[0]);
	}
	if (tb->rnum[0]) {
		retval |= locked_or_not_in_tree(tb, tb->R[0], "R[0]");
		retval |= locked_or_not_in_tree(tb, tb->FR[0], "FR[0]");
		retval |= locked_or_not_in_tree(tb, tb->CFR[0], "CFR[0]");
		check_leaf(tb->R[0]);
	}
	retval |= locked_or_not_in_tree(tb, PATH_PLAST_BUFFER(tb->tb_path),
					"S[0]");
	check_leaf(PATH_PLAST_BUFFER(tb->tb_path));

	return retval;
}

static void check_after_balance_leaf(struct tree_balance *tb)
{
	if (tb->lnum[0]) {
		if (B_FREE_SPACE(tb->L[0]) !=
		    MAX_CHILD_SIZE(tb->L[0]) -
		    dc_size(B_N_CHILD
			    (tb->FL[0], get_left_neighbor_position(tb, 0)))) {
			print_cur_tb("12221");
			reiserfs_panic(tb->tb_sb, "PAP-12355",
				       "shift to left was incorrect");
		}
	}
	if (tb->rnum[0]) {
		if (B_FREE_SPACE(tb->R[0]) !=
		    MAX_CHILD_SIZE(tb->R[0]) -
		    dc_size(B_N_CHILD
			    (tb->FR[0], get_right_neighbor_position(tb, 0)))) {
			print_cur_tb("12222");
			reiserfs_panic(tb->tb_sb, "PAP-12360",
				       "shift to right was incorrect");
		}
	}
	if (PATH_H_PBUFFER(tb->tb_path, 1) &&
	    (B_FREE_SPACE(PATH_H_PBUFFER(tb->tb_path, 0)) !=
	     (MAX_CHILD_SIZE(PATH_H_PBUFFER(tb->tb_path, 0)) -
	      dc_size(B_N_CHILD(PATH_H_PBUFFER(tb->tb_path, 1),
				PATH_H_POSITION(tb->tb_path, 1)))))) {
		int left = B_FREE_SPACE(PATH_H_PBUFFER(tb->tb_path, 0));
		int right = (MAX_CHILD_SIZE(PATH_H_PBUFFER(tb->tb_path, 0)) -
			     dc_size(B_N_CHILD(PATH_H_PBUFFER(tb->tb_path, 1),
					       PATH_H_POSITION(tb->tb_path,
							       1))));
		print_cur_tb("12223");
		reiserfs_warning(tb->tb_sb, "reiserfs-12363",
				 "B_FREE_SPACE (PATH_H_PBUFFER(tb->tb_path,0)) = %d; "
				 "MAX_CHILD_SIZE (%d) - dc_size( %y, %d ) [%d] = %d",
				 left,
				 MAX_CHILD_SIZE(PATH_H_PBUFFER(tb->tb_path, 0)),
				 PATH_H_PBUFFER(tb->tb_path, 1),
				 PATH_H_POSITION(tb->tb_path, 1),
				 dc_size(B_N_CHILD
					 (PATH_H_PBUFFER(tb->tb_path, 1),
					  PATH_H_POSITION(tb->tb_path, 1))),
				 right);
		reiserfs_panic(tb->tb_sb, "PAP-12365", "S is incorrect");
	}
}

static void check_leaf_level(struct tree_balance *tb)
{
	check_leaf(tb->L[0]);
	check_leaf(tb->R[0]);
	check_leaf(PATH_PLAST_BUFFER(tb->tb_path));
}

static void check_internal_levels(struct tree_balance *tb)
{
	int h;

	/*                          */
	for (h = 1; tb->insert_size[h]; h++) {
		check_internal_node(tb->tb_sb, PATH_H_PBUFFER(tb->tb_path, h),
				    "BAD BUFFER ON PATH");
		if (tb->lnum[h])
			check_internal_node(tb->tb_sb, tb->L[h], "BAD L");
		if (tb->rnum[h])
			check_internal_node(tb->tb_sb, tb->R[h], "BAD R");
	}

}

#endif

/*                                                                 
                                                                      
                                                                     
                                                                  
                                                                      
                                           */

/*                                     

                                                                     
                                                                   
               

                                                                   
                       

                                                                
                                                                  
                                                                      
                                                                      
                                  

                                                               
                                                                   
               

                                                                      
                                                                
                                                                   
                                              

*/

static inline void do_balance_starts(struct tree_balance *tb)
{
	/*                                                  
                 */

	/*                      */

	/*                                    */
/*                                                                                 
               */
	RFALSE(check_before_balancing(tb), "PAP-12340: locked buffers in TB");
#ifdef CONFIG_REISERFS_CHECK
	REISERFS_SB(tb->tb_sb)->cur_tb = tb;
#endif
}

static inline void do_balance_completed(struct tree_balance *tb)
{

#ifdef CONFIG_REISERFS_CHECK
	check_leaf_level(tb);
	check_internal_levels(tb);
	REISERFS_SB(tb->tb_sb)->cur_tb = NULL;
#endif

	/*                                                                
                                                                        
                           
  */

	REISERFS_SB(tb->tb_sb)->s_do_balance++;

	/*                                                 */
	unfix_nodes(tb);

	free_thrown(tb);
}

void do_balance(struct tree_balance *tb,	/*                        */
		struct item_head *ih,	/*                              */
		const char *body,	/*                                          */
		int flag)
{				/*                       
                         

                                       
                                         
                  

                                      

                                           
            

                                             
                                             
               */
	int child_pos,		/*                                        */
	 h;			/*                                   */
	struct item_head insert_key[2];	/*                               
                                   
                                      
                                     
                                     
                               
                 */
	struct buffer_head *insert_ptr[2];	/*                                
               */

	tb->tb_mode = flag;
	tb->need_balance_dirty = 0;

	if (FILESYSTEM_CHANGED_TB(tb)) {
		reiserfs_panic(tb->tb_sb, "clm-6000", "fs generation has "
			       "changed");
	}
	/*                                */
	if (!tb->insert_size[0]) {
		reiserfs_warning(tb->tb_sb, "PAP-12350",
				 "insert_size == 0, mode == %c", flag);
		unfix_nodes(tb);
		return;
	}

	atomic_inc(&(fs_generation(tb->tb_sb)));
	do_balance_starts(tb);

	/*                                                          
                                                             
                  */
	child_pos = PATH_H_B_ITEM_ORDER(tb->tb_path, 0) +
	    balance_leaf(tb, ih, body, flag, insert_key, insert_ptr);

#ifdef CONFIG_REISERFS_CHECK
	check_after_balance_leaf(tb);
#endif

	/*                                     */
	for (h = 1; h < MAX_HEIGHT && tb->insert_size[h]; h++)
		child_pos =
		    balance_internal(tb, h, child_pos, insert_key, insert_ptr);

	do_balance_completed(tb);

}
