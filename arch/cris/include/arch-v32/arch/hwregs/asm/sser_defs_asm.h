#ifndef __sser_defs_asm_h
#define __sser_defs_asm_h

/*
                                  
                                                       
                                                                       
                                             
  
                                                                                                                   
                                                                        
                                 
  
                              
 */

#ifndef REG_FIELD
#define REG_FIELD( scope, reg, field, value ) \
  REG_FIELD_X_( value, reg_##scope##_##reg##___##field##___lsb )
#define REG_FIELD_X_( value, shift ) ((value) << shift)
#endif

#ifndef REG_STATE
#define REG_STATE( scope, reg, field, symbolic_value ) \
  REG_STATE_X_( regk_##scope##_##symbolic_value, reg_##scope##_##reg##___##field##___lsb )
#define REG_STATE_X_( k, shift ) (k << shift)
#endif

#ifndef REG_MASK
#define REG_MASK( scope, reg, field ) \
  REG_MASK_X_( reg_##scope##_##reg##___##field##___width, reg_##scope##_##reg##___##field##___lsb )
#define REG_MASK_X_( width, lsb ) (((1 << width)-1) << lsb)
#endif

#ifndef REG_LSB
#define REG_LSB( scope, reg, field ) reg_##scope##_##reg##___##field##___lsb
#endif

#ifndef REG_BIT
#define REG_BIT( scope, reg, field ) reg_##scope##_##reg##___##field##___bit
#endif

#ifndef REG_ADDR
#define REG_ADDR( scope, inst, reg ) REG_ADDR_X_(inst, reg_##scope##_##reg##_offset)
#define REG_ADDR_X_( inst, offs ) ((inst) + offs)
#endif

#ifndef REG_ADDR_VECT
#define REG_ADDR_VECT( scope, inst, reg, index ) \
         REG_ADDR_VECT_X_(inst, reg_##scope##_##reg##_offset, index, \
			 STRIDE_##scope##_##reg )
#define REG_ADDR_VECT_X_( inst, offs, index, stride ) \
                          ((inst) + offs + (index) * stride)
#endif

/*                                      */
#define reg_sser_rw_cfg___clk_div___lsb 0
#define reg_sser_rw_cfg___clk_div___width 16
#define reg_sser_rw_cfg___base_freq___lsb 16
#define reg_sser_rw_cfg___base_freq___width 3
#define reg_sser_rw_cfg___gate_clk___lsb 19
#define reg_sser_rw_cfg___gate_clk___width 1
#define reg_sser_rw_cfg___gate_clk___bit 19
#define reg_sser_rw_cfg___clkgate_ctrl___lsb 20
#define reg_sser_rw_cfg___clkgate_ctrl___width 1
#define reg_sser_rw_cfg___clkgate_ctrl___bit 20
#define reg_sser_rw_cfg___clkgate_in___lsb 21
#define reg_sser_rw_cfg___clkgate_in___width 1
#define reg_sser_rw_cfg___clkgate_in___bit 21
#define reg_sser_rw_cfg___clk_dir___lsb 22
#define reg_sser_rw_cfg___clk_dir___width 1
#define reg_sser_rw_cfg___clk_dir___bit 22
#define reg_sser_rw_cfg___clk_od_mode___lsb 23
#define reg_sser_rw_cfg___clk_od_mode___width 1
#define reg_sser_rw_cfg___clk_od_mode___bit 23
#define reg_sser_rw_cfg___out_clk_pol___lsb 24
#define reg_sser_rw_cfg___out_clk_pol___width 1
#define reg_sser_rw_cfg___out_clk_pol___bit 24
#define reg_sser_rw_cfg___out_clk_src___lsb 25
#define reg_sser_rw_cfg___out_clk_src___width 2
#define reg_sser_rw_cfg___clk_in_sel___lsb 27
#define reg_sser_rw_cfg___clk_in_sel___width 1
#define reg_sser_rw_cfg___clk_in_sel___bit 27
#define reg_sser_rw_cfg___hold_pol___lsb 28
#define reg_sser_rw_cfg___hold_pol___width 1
#define reg_sser_rw_cfg___hold_pol___bit 28
#define reg_sser_rw_cfg___prepare___lsb 29
#define reg_sser_rw_cfg___prepare___width 1
#define reg_sser_rw_cfg___prepare___bit 29
#define reg_sser_rw_cfg___en___lsb 30
#define reg_sser_rw_cfg___en___width 1
#define reg_sser_rw_cfg___en___bit 30
#define reg_sser_rw_cfg_offset 0

/*                                          */
#define reg_sser_rw_frm_cfg___wordrate___lsb 0
#define reg_sser_rw_frm_cfg___wordrate___width 10
#define reg_sser_rw_frm_cfg___rec_delay___lsb 10
#define reg_sser_rw_frm_cfg___rec_delay___width 3
#define reg_sser_rw_frm_cfg___tr_delay___lsb 13
#define reg_sser_rw_frm_cfg___tr_delay___width 3
#define reg_sser_rw_frm_cfg___early_wend___lsb 16
#define reg_sser_rw_frm_cfg___early_wend___width 1
#define reg_sser_rw_frm_cfg___early_wend___bit 16
#define reg_sser_rw_frm_cfg___level___lsb 17
#define reg_sser_rw_frm_cfg___level___width 2
#define reg_sser_rw_frm_cfg___type___lsb 19
#define reg_sser_rw_frm_cfg___type___width 1
#define reg_sser_rw_frm_cfg___type___bit 19
#define reg_sser_rw_frm_cfg___clk_pol___lsb 20
#define reg_sser_rw_frm_cfg___clk_pol___width 1
#define reg_sser_rw_frm_cfg___clk_pol___bit 20
#define reg_sser_rw_frm_cfg___fr_in_rxclk___lsb 21
#define reg_sser_rw_frm_cfg___fr_in_rxclk___width 1
#define reg_sser_rw_frm_cfg___fr_in_rxclk___bit 21
#define reg_sser_rw_frm_cfg___clk_src___lsb 22
#define reg_sser_rw_frm_cfg___clk_src___width 1
#define reg_sser_rw_frm_cfg___clk_src___bit 22
#define reg_sser_rw_frm_cfg___out_off___lsb 23
#define reg_sser_rw_frm_cfg___out_off___width 1
#define reg_sser_rw_frm_cfg___out_off___bit 23
#define reg_sser_rw_frm_cfg___out_on___lsb 24
#define reg_sser_rw_frm_cfg___out_on___width 1
#define reg_sser_rw_frm_cfg___out_on___bit 24
#define reg_sser_rw_frm_cfg___frame_pin_dir___lsb 25
#define reg_sser_rw_frm_cfg___frame_pin_dir___width 1
#define reg_sser_rw_frm_cfg___frame_pin_dir___bit 25
#define reg_sser_rw_frm_cfg___frame_pin_use___lsb 26
#define reg_sser_rw_frm_cfg___frame_pin_use___width 2
#define reg_sser_rw_frm_cfg___status_pin_dir___lsb 28
#define reg_sser_rw_frm_cfg___status_pin_dir___width 1
#define reg_sser_rw_frm_cfg___status_pin_dir___bit 28
#define reg_sser_rw_frm_cfg___status_pin_use___lsb 29
#define reg_sser_rw_frm_cfg___status_pin_use___width 2
#define reg_sser_rw_frm_cfg_offset 4

/*                                         */
#define reg_sser_rw_tr_cfg___tr_en___lsb 0
#define reg_sser_rw_tr_cfg___tr_en___width 1
#define reg_sser_rw_tr_cfg___tr_en___bit 0
#define reg_sser_rw_tr_cfg___stop___lsb 1
#define reg_sser_rw_tr_cfg___stop___width 1
#define reg_sser_rw_tr_cfg___stop___bit 1
#define reg_sser_rw_tr_cfg___urun_stop___lsb 2
#define reg_sser_rw_tr_cfg___urun_stop___width 1
#define reg_sser_rw_tr_cfg___urun_stop___bit 2
#define reg_sser_rw_tr_cfg___eop_stop___lsb 3
#define reg_sser_rw_tr_cfg___eop_stop___width 1
#define reg_sser_rw_tr_cfg___eop_stop___bit 3
#define reg_sser_rw_tr_cfg___sample_size___lsb 4
#define reg_sser_rw_tr_cfg___sample_size___width 6
#define reg_sser_rw_tr_cfg___sh_dir___lsb 10
#define reg_sser_rw_tr_cfg___sh_dir___width 1
#define reg_sser_rw_tr_cfg___sh_dir___bit 10
#define reg_sser_rw_tr_cfg___clk_pol___lsb 11
#define reg_sser_rw_tr_cfg___clk_pol___width 1
#define reg_sser_rw_tr_cfg___clk_pol___bit 11
#define reg_sser_rw_tr_cfg___clk_src___lsb 12
#define reg_sser_rw_tr_cfg___clk_src___width 1
#define reg_sser_rw_tr_cfg___clk_src___bit 12
#define reg_sser_rw_tr_cfg___use_dma___lsb 13
#define reg_sser_rw_tr_cfg___use_dma___width 1
#define reg_sser_rw_tr_cfg___use_dma___bit 13
#define reg_sser_rw_tr_cfg___mode___lsb 14
#define reg_sser_rw_tr_cfg___mode___width 2
#define reg_sser_rw_tr_cfg___frm_src___lsb 16
#define reg_sser_rw_tr_cfg___frm_src___width 1
#define reg_sser_rw_tr_cfg___frm_src___bit 16
#define reg_sser_rw_tr_cfg___use60958___lsb 17
#define reg_sser_rw_tr_cfg___use60958___width 1
#define reg_sser_rw_tr_cfg___use60958___bit 17
#define reg_sser_rw_tr_cfg___iec60958_ckdiv___lsb 18
#define reg_sser_rw_tr_cfg___iec60958_ckdiv___width 2
#define reg_sser_rw_tr_cfg___rate_ctrl___lsb 20
#define reg_sser_rw_tr_cfg___rate_ctrl___width 1
#define reg_sser_rw_tr_cfg___rate_ctrl___bit 20
#define reg_sser_rw_tr_cfg___use_md___lsb 21
#define reg_sser_rw_tr_cfg___use_md___width 1
#define reg_sser_rw_tr_cfg___use_md___bit 21
#define reg_sser_rw_tr_cfg___dual_i2s___lsb 22
#define reg_sser_rw_tr_cfg___dual_i2s___width 1
#define reg_sser_rw_tr_cfg___dual_i2s___bit 22
#define reg_sser_rw_tr_cfg___data_pin_use___lsb 23
#define reg_sser_rw_tr_cfg___data_pin_use___width 2
#define reg_sser_rw_tr_cfg___od_mode___lsb 25
#define reg_sser_rw_tr_cfg___od_mode___width 1
#define reg_sser_rw_tr_cfg___od_mode___bit 25
#define reg_sser_rw_tr_cfg___bulk_wspace___lsb 26
#define reg_sser_rw_tr_cfg___bulk_wspace___width 2
#define reg_sser_rw_tr_cfg_offset 8

/*                                          */
#define reg_sser_rw_rec_cfg___rec_en___lsb 0
#define reg_sser_rw_rec_cfg___rec_en___width 1
#define reg_sser_rw_rec_cfg___rec_en___bit 0
#define reg_sser_rw_rec_cfg___force_eop___lsb 1
#define reg_sser_rw_rec_cfg___force_eop___width 1
#define reg_sser_rw_rec_cfg___force_eop___bit 1
#define reg_sser_rw_rec_cfg___stop___lsb 2
#define reg_sser_rw_rec_cfg___stop___width 1
#define reg_sser_rw_rec_cfg___stop___bit 2
#define reg_sser_rw_rec_cfg___orun_stop___lsb 3
#define reg_sser_rw_rec_cfg___orun_stop___width 1
#define reg_sser_rw_rec_cfg___orun_stop___bit 3
#define reg_sser_rw_rec_cfg___eop_stop___lsb 4
#define reg_sser_rw_rec_cfg___eop_stop___width 1
#define reg_sser_rw_rec_cfg___eop_stop___bit 4
#define reg_sser_rw_rec_cfg___sample_size___lsb 5
#define reg_sser_rw_rec_cfg___sample_size___width 6
#define reg_sser_rw_rec_cfg___sh_dir___lsb 11
#define reg_sser_rw_rec_cfg___sh_dir___width 1
#define reg_sser_rw_rec_cfg___sh_dir___bit 11
#define reg_sser_rw_rec_cfg___clk_pol___lsb 12
#define reg_sser_rw_rec_cfg___clk_pol___width 1
#define reg_sser_rw_rec_cfg___clk_pol___bit 12
#define reg_sser_rw_rec_cfg___clk_src___lsb 13
#define reg_sser_rw_rec_cfg___clk_src___width 1
#define reg_sser_rw_rec_cfg___clk_src___bit 13
#define reg_sser_rw_rec_cfg___use_dma___lsb 14
#define reg_sser_rw_rec_cfg___use_dma___width 1
#define reg_sser_rw_rec_cfg___use_dma___bit 14
#define reg_sser_rw_rec_cfg___mode___lsb 15
#define reg_sser_rw_rec_cfg___mode___width 2
#define reg_sser_rw_rec_cfg___frm_src___lsb 17
#define reg_sser_rw_rec_cfg___frm_src___width 2
#define reg_sser_rw_rec_cfg___use60958___lsb 19
#define reg_sser_rw_rec_cfg___use60958___width 1
#define reg_sser_rw_rec_cfg___use60958___bit 19
#define reg_sser_rw_rec_cfg___iec60958_ui_len___lsb 20
#define reg_sser_rw_rec_cfg___iec60958_ui_len___width 5
#define reg_sser_rw_rec_cfg___slave2_en___lsb 25
#define reg_sser_rw_rec_cfg___slave2_en___width 1
#define reg_sser_rw_rec_cfg___slave2_en___bit 25
#define reg_sser_rw_rec_cfg___slave3_en___lsb 26
#define reg_sser_rw_rec_cfg___slave3_en___width 1
#define reg_sser_rw_rec_cfg___slave3_en___bit 26
#define reg_sser_rw_rec_cfg___fifo_thr___lsb 27
#define reg_sser_rw_rec_cfg___fifo_thr___width 2
#define reg_sser_rw_rec_cfg_offset 12

/*                                          */
#define reg_sser_rw_tr_data___data___lsb 0
#define reg_sser_rw_tr_data___data___width 16
#define reg_sser_rw_tr_data___md___lsb 16
#define reg_sser_rw_tr_data___md___width 1
#define reg_sser_rw_tr_data___md___bit 16
#define reg_sser_rw_tr_data_offset 16

/*                                         */
#define reg_sser_r_rec_data___data___lsb 0
#define reg_sser_r_rec_data___data___width 16
#define reg_sser_r_rec_data___md___lsb 16
#define reg_sser_r_rec_data___md___width 1
#define reg_sser_r_rec_data___md___bit 16
#define reg_sser_r_rec_data___ext_clk___lsb 17
#define reg_sser_r_rec_data___ext_clk___width 1
#define reg_sser_r_rec_data___ext_clk___bit 17
#define reg_sser_r_rec_data___status_in___lsb 18
#define reg_sser_r_rec_data___status_in___width 1
#define reg_sser_r_rec_data___status_in___bit 18
#define reg_sser_r_rec_data___frame_in___lsb 19
#define reg_sser_r_rec_data___frame_in___width 1
#define reg_sser_r_rec_data___frame_in___bit 19
#define reg_sser_r_rec_data___din___lsb 20
#define reg_sser_r_rec_data___din___width 1
#define reg_sser_r_rec_data___din___bit 20
#define reg_sser_r_rec_data___data_in___lsb 21
#define reg_sser_r_rec_data___data_in___width 1
#define reg_sser_r_rec_data___data_in___bit 21
#define reg_sser_r_rec_data___clk_in___lsb 22
#define reg_sser_r_rec_data___clk_in___width 1
#define reg_sser_r_rec_data___clk_in___bit 22
#define reg_sser_r_rec_data_offset 20

/*                                        */
#define reg_sser_rw_extra___clkoff_cycles___lsb 0
#define reg_sser_rw_extra___clkoff_cycles___width 20
#define reg_sser_rw_extra___clkoff_en___lsb 20
#define reg_sser_rw_extra___clkoff_en___width 1
#define reg_sser_rw_extra___clkoff_en___bit 20
#define reg_sser_rw_extra___clkon_en___lsb 21
#define reg_sser_rw_extra___clkon_en___width 1
#define reg_sser_rw_extra___clkon_en___bit 21
#define reg_sser_rw_extra___dout_delay___lsb 22
#define reg_sser_rw_extra___dout_delay___width 5
#define reg_sser_rw_extra_offset 24

/*                                            */
#define reg_sser_rw_intr_mask___trdy___lsb 0
#define reg_sser_rw_intr_mask___trdy___width 1
#define reg_sser_rw_intr_mask___trdy___bit 0
#define reg_sser_rw_intr_mask___rdav___lsb 1
#define reg_sser_rw_intr_mask___rdav___width 1
#define reg_sser_rw_intr_mask___rdav___bit 1
#define reg_sser_rw_intr_mask___tidle___lsb 2
#define reg_sser_rw_intr_mask___tidle___width 1
#define reg_sser_rw_intr_mask___tidle___bit 2
#define reg_sser_rw_intr_mask___rstop___lsb 3
#define reg_sser_rw_intr_mask___rstop___width 1
#define reg_sser_rw_intr_mask___rstop___bit 3
#define reg_sser_rw_intr_mask___urun___lsb 4
#define reg_sser_rw_intr_mask___urun___width 1
#define reg_sser_rw_intr_mask___urun___bit 4
#define reg_sser_rw_intr_mask___orun___lsb 5
#define reg_sser_rw_intr_mask___orun___width 1
#define reg_sser_rw_intr_mask___orun___bit 5
#define reg_sser_rw_intr_mask___md_rec___lsb 6
#define reg_sser_rw_intr_mask___md_rec___width 1
#define reg_sser_rw_intr_mask___md_rec___bit 6
#define reg_sser_rw_intr_mask___md_sent___lsb 7
#define reg_sser_rw_intr_mask___md_sent___width 1
#define reg_sser_rw_intr_mask___md_sent___bit 7
#define reg_sser_rw_intr_mask___r958err___lsb 8
#define reg_sser_rw_intr_mask___r958err___width 1
#define reg_sser_rw_intr_mask___r958err___bit 8
#define reg_sser_rw_intr_mask_offset 28

/*                                           */
#define reg_sser_rw_ack_intr___trdy___lsb 0
#define reg_sser_rw_ack_intr___trdy___width 1
#define reg_sser_rw_ack_intr___trdy___bit 0
#define reg_sser_rw_ack_intr___rdav___lsb 1
#define reg_sser_rw_ack_intr___rdav___width 1
#define reg_sser_rw_ack_intr___rdav___bit 1
#define reg_sser_rw_ack_intr___tidle___lsb 2
#define reg_sser_rw_ack_intr___tidle___width 1
#define reg_sser_rw_ack_intr___tidle___bit 2
#define reg_sser_rw_ack_intr___rstop___lsb 3
#define reg_sser_rw_ack_intr___rstop___width 1
#define reg_sser_rw_ack_intr___rstop___bit 3
#define reg_sser_rw_ack_intr___urun___lsb 4
#define reg_sser_rw_ack_intr___urun___width 1
#define reg_sser_rw_ack_intr___urun___bit 4
#define reg_sser_rw_ack_intr___orun___lsb 5
#define reg_sser_rw_ack_intr___orun___width 1
#define reg_sser_rw_ack_intr___orun___bit 5
#define reg_sser_rw_ack_intr___md_rec___lsb 6
#define reg_sser_rw_ack_intr___md_rec___width 1
#define reg_sser_rw_ack_intr___md_rec___bit 6
#define reg_sser_rw_ack_intr___md_sent___lsb 7
#define reg_sser_rw_ack_intr___md_sent___width 1
#define reg_sser_rw_ack_intr___md_sent___bit 7
#define reg_sser_rw_ack_intr___r958err___lsb 8
#define reg_sser_rw_ack_intr___r958err___width 1
#define reg_sser_rw_ack_intr___r958err___bit 8
#define reg_sser_rw_ack_intr_offset 32

/*                                     */
#define reg_sser_r_intr___trdy___lsb 0
#define reg_sser_r_intr___trdy___width 1
#define reg_sser_r_intr___trdy___bit 0
#define reg_sser_r_intr___rdav___lsb 1
#define reg_sser_r_intr___rdav___width 1
#define reg_sser_r_intr___rdav___bit 1
#define reg_sser_r_intr___tidle___lsb 2
#define reg_sser_r_intr___tidle___width 1
#define reg_sser_r_intr___tidle___bit 2
#define reg_sser_r_intr___rstop___lsb 3
#define reg_sser_r_intr___rstop___width 1
#define reg_sser_r_intr___rstop___bit 3
#define reg_sser_r_intr___urun___lsb 4
#define reg_sser_r_intr___urun___width 1
#define reg_sser_r_intr___urun___bit 4
#define reg_sser_r_intr___orun___lsb 5
#define reg_sser_r_intr___orun___width 1
#define reg_sser_r_intr___orun___bit 5
#define reg_sser_r_intr___md_rec___lsb 6
#define reg_sser_r_intr___md_rec___width 1
#define reg_sser_r_intr___md_rec___bit 6
#define reg_sser_r_intr___md_sent___lsb 7
#define reg_sser_r_intr___md_sent___width 1
#define reg_sser_r_intr___md_sent___bit 7
#define reg_sser_r_intr___r958err___lsb 8
#define reg_sser_r_intr___r958err___width 1
#define reg_sser_r_intr___r958err___bit 8
#define reg_sser_r_intr_offset 36

/*                                            */
#define reg_sser_r_masked_intr___trdy___lsb 0
#define reg_sser_r_masked_intr___trdy___width 1
#define reg_sser_r_masked_intr___trdy___bit 0
#define reg_sser_r_masked_intr___rdav___lsb 1
#define reg_sser_r_masked_intr___rdav___width 1
#define reg_sser_r_masked_intr___rdav___bit 1
#define reg_sser_r_masked_intr___tidle___lsb 2
#define reg_sser_r_masked_intr___tidle___width 1
#define reg_sser_r_masked_intr___tidle___bit 2
#define reg_sser_r_masked_intr___rstop___lsb 3
#define reg_sser_r_masked_intr___rstop___width 1
#define reg_sser_r_masked_intr___rstop___bit 3
#define reg_sser_r_masked_intr___urun___lsb 4
#define reg_sser_r_masked_intr___urun___width 1
#define reg_sser_r_masked_intr___urun___bit 4
#define reg_sser_r_masked_intr___orun___lsb 5
#define reg_sser_r_masked_intr___orun___width 1
#define reg_sser_r_masked_intr___orun___bit 5
#define reg_sser_r_masked_intr___md_rec___lsb 6
#define reg_sser_r_masked_intr___md_rec___width 1
#define reg_sser_r_masked_intr___md_rec___bit 6
#define reg_sser_r_masked_intr___md_sent___lsb 7
#define reg_sser_r_masked_intr___md_sent___width 1
#define reg_sser_r_masked_intr___md_sent___bit 7
#define reg_sser_r_masked_intr___r958err___lsb 8
#define reg_sser_r_masked_intr___r958err___width 1
#define reg_sser_r_masked_intr___r958err___bit 8
#define reg_sser_r_masked_intr_offset 40


/*           */
#define regk_sser_both                            0x00000002
#define regk_sser_bulk                            0x00000001
#define regk_sser_clk100                          0x00000000
#define regk_sser_clk_in                          0x00000000
#define regk_sser_const0                          0x00000003
#define regk_sser_dout                            0x00000002
#define regk_sser_edge                            0x00000000
#define regk_sser_ext                             0x00000001
#define regk_sser_ext_clk                         0x00000001
#define regk_sser_f100                            0x00000000
#define regk_sser_f29_493                         0x00000004
#define regk_sser_f32                             0x00000005
#define regk_sser_f32_768                         0x00000006
#define regk_sser_frm                             0x00000003
#define regk_sser_gio0                            0x00000000
#define regk_sser_gio1                            0x00000001
#define regk_sser_hispeed                         0x00000001
#define regk_sser_hold                            0x00000002
#define regk_sser_in                              0x00000000
#define regk_sser_inf                             0x00000003
#define regk_sser_intern                          0x00000000
#define regk_sser_intern_clk                      0x00000001
#define regk_sser_intern_tb                       0x00000000
#define regk_sser_iso                             0x00000000
#define regk_sser_level                           0x00000001
#define regk_sser_lospeed                         0x00000000
#define regk_sser_lsbfirst                        0x00000000
#define regk_sser_msbfirst                        0x00000001
#define regk_sser_neg                             0x00000001
#define regk_sser_neg_lo                          0x00000000
#define regk_sser_no                              0x00000000
#define regk_sser_no_clk                          0x00000007
#define regk_sser_nojitter                        0x00000002
#define regk_sser_out                             0x00000001
#define regk_sser_pos                             0x00000000
#define regk_sser_pos_hi                          0x00000001
#define regk_sser_rec                             0x00000000
#define regk_sser_rw_cfg_default                  0x00000000
#define regk_sser_rw_extra_default                0x00000000
#define regk_sser_rw_frm_cfg_default              0x00000000
#define regk_sser_rw_intr_mask_default            0x00000000
#define regk_sser_rw_rec_cfg_default              0x00000000
#define regk_sser_rw_tr_cfg_default               0x01800000
#define regk_sser_rw_tr_data_default              0x00000000
#define regk_sser_thr16                           0x00000001
#define regk_sser_thr32                           0x00000002
#define regk_sser_thr8                            0x00000000
#define regk_sser_tr                              0x00000001
#define regk_sser_ts_out                          0x00000003
#define regk_sser_tx_bulk                         0x00000002
#define regk_sser_wiresave                        0x00000002
#define regk_sser_yes                             0x00000001
#endif /*                   */
