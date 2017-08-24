
/*
 * 68360 Communication Processor Module.
 * Copyright (c) 2000 Michael Leslie <mleslie@lineo.com> (mc68360) after:
 * Copyright (c) 1997 Dan Malek <dmalek@jlc.net> (mpc8xx)
 *
 * This file contains structures and information for the communication
 * processor channels.  Some CPM control and status is available
 * through the 68360 internal memory map.  See include/asm/360_immap.h for details.
 * This file is not a complete map of all of the 360 QUICC's capabilities
 *
 * On the MBX board, EPPC-Bug loads CPM microcode into the first 512
 * bytes of the DP RAM and relocates the I2C parameter area to the
 * IDMA1 space.  The remaining DP RAM is available for buffer descriptors
 * or other use.
 */
#ifndef __CPM_360__
#define __CPM_360__


/*                             */
#define CPM_CR_RST	((ushort)0x8000)
#define CPM_CR_OPCODE	((ushort)0x0f00)
#define CPM_CR_CHAN	((ushort)0x00f0)
#define CPM_CR_FLG	((ushort)0x0001)

/*                            */
#define CPM_CR_INIT_TRX		((ushort)0x0000)
#define CPM_CR_INIT_RX		((ushort)0x0001)
#define CPM_CR_INIT_TX		((ushort)0x0002)
#define CPM_CR_HUNT_MODE	((ushort)0x0003)
#define CPM_CR_STOP_TX		((ushort)0x0004)
#define CPM_CR_GRSTOP_TX	((ushort)0x0005)
#define CPM_CR_RESTART_TX	((ushort)0x0006)
#define CPM_CR_CLOSE_RXBD	((ushort)0x0007)
#define CPM_CR_SET_GADDR	((ushort)0x0008)
#define CPM_CR_GCI_TIMEOUT	((ushort)0x0009)
#define CPM_CR_GCI_ABORT	((ushort)0x000a)
#define CPM_CR_RESET_BCS	((ushort)0x000a)

/*                      */
#define CPM_CR_CH_SCC1	((ushort)0x0000)
#define CPM_CR_CH_SCC2	((ushort)0x0004)
#define CPM_CR_CH_SPI	((ushort)0x0005)	/*              */
#define CPM_CR_CH_TMR	((ushort)0x0005)
#define CPM_CR_CH_SCC3	((ushort)0x0008)
#define CPM_CR_CH_SMC1	((ushort)0x0009)	/*              */
#define CPM_CR_CH_IDMA1	((ushort)0x0009)
#define CPM_CR_CH_SCC4	((ushort)0x000c)
#define CPM_CR_CH_SMC2	((ushort)0x000d)	/*              */
#define CPM_CR_CH_IDMA2	((ushort)0x000d)


#define mk_cr_cmd(CH, CMD)	((CMD << 8) | (CH << 4))

#if 1 /*                                                        
                                                               */

/*                                                                     
                                                                     
                                                                     
                                                                     
 */
/*                                                                 */
#define CPM_DATAONLY_BASE	((uint)0x0000)
#define CPM_DATAONLY_SIZE	((uint)0x0800)
#define CPM_DP_NOSPACE		((uint)0x7fffffff)

#endif


/*                                                                 
                      */
/*                        */		/*                           */
extern QUICC *pquicc;
uint         m360_cpm_dpalloc(uint size);
/*                                              */
void	      m360_cpm_setbrg(uint brg, uint rate);

#if 0 /*                                                      */
/*                                                       */
typedef struct cpm_buf_desc {
	ushort	cbd_sc;		/*                    */
	ushort	cbd_datlen;	/*                       */
	uint	cbd_bufaddr;	/*                               */
} cbd_t;
#endif


/*                           */
#define BD_SC_EMPTY	((ushort)0x8000)	/*                  */
#define BD_SC_WRAP	((ushort)0x2000)	/*                                 */
#define BD_SC_INTRPT	((ushort)0x1000)	/*                     */
#define BD_SC_LAST	((ushort)0x0800)	/*                                      */

#define BD_SC_FIRST	((ushort)0x0400)	/*                             */
#define BD_SC_ADDR	((ushort)0x0400)	/*                                 */

#define BD_SC_CM	((ushort)0x0200)	/*                 */
#define BD_SC_ID	((ushort)0x0100)	/*                         */

#define BD_SC_AM	((ushort)0x0080)	/*                         */
#define BD_SC_DE	((ushort)0x0080)	/*                   */

#define BD_SC_BR	((ushort)0x0020)	/*                */
#define BD_SC_LG	((ushort)0x0020)	/*                               */

#define BD_SC_FR	((ushort)0x0010)	/*               */
#define BD_SC_NO	((ushort)0x0010)	/*                               */

#define BD_SC_PR	((ushort)0x0008)	/*              */
#define BD_SC_AB	((ushort)0x0008)	/*                                */

#define BD_SC_OV	((ushort)0x0002)	/*         */
#define BD_SC_CD	((ushort)0x0001)	/*                     */

/*                                                  */
#define BD_SC_READY	((ushort)0x8000)	/*                   */
#define BD_SC_TC	((ushort)0x0400)	/*              */
#define BD_SC_P		((ushort)0x0100)	/*              */
#define BD_SC_UN	((ushort)0x0002)	/*          */




/*                        */



/*                                                                
                                                                  
                                                 */

#define PRSLOT_SCC1	0
#define PRSLOT_SCC2	1
#define PRSLOT_SCC3	2
#define PRSLOT_SMC1	2
#define PRSLOT_SCC4	3
#define PRSLOT_SMC2	3


/*                                   */
/*                                   */
/*                                   */
/*                                   */
/*                                   */
/*                                   */


/*                                                               
                                             
 */
typedef struct smc_uart {
	ushort	smc_rbase;	/*                                   */
	ushort	smc_tbase;	/*                                   */
	u_char	smc_rfcr;	/*                  */
	u_char	smc_tfcr;	/*                  */
	ushort	smc_mrblr;	/*                           */
	uint	smc_rstate;	/*          */
	uint	smc_idp;	/*          */
	ushort	smc_rbptr;	/*          */
	ushort	smc_ibc;	/*          */
	uint	smc_rxtmp;	/*          */
	uint	smc_tstate;	/*          */
	uint	smc_tdp;	/*          */
	ushort	smc_tbptr;	/*          */
	ushort	smc_tbc;	/*          */
	uint	smc_txtmp;	/*          */
	ushort	smc_maxidl;	/*                         */
	ushort	smc_tmpidl;	/*                        */
	ushort	smc_brklen;	/*                            */
	ushort	smc_brkec;	/*                               */
	ushort	smc_brkcr;	/*                          */
	ushort	smc_rmask;	/*                    */
} smc_uart_t;

/*                    
*/
#define SMC_EB	((u_char)0x10)	/*                           */

/*                        
*/
#define	SMCMR_REN	((ushort)0x0001)
#define SMCMR_TEN	((ushort)0x0002)
#define SMCMR_DM	((ushort)0x000c)
#define SMCMR_SM_GCI	((ushort)0x0000)
#define SMCMR_SM_UART	((ushort)0x0020)
#define SMCMR_SM_TRANS	((ushort)0x0030)
#define SMCMR_SM_MASK	((ushort)0x0030)
#define SMCMR_PM_EVEN	((ushort)0x0100)	/*                       */
#define SMCMR_REVD	SMCMR_PM_EVEN
#define SMCMR_PEN	((ushort)0x0200)	/*               */
#define SMCMR_BS	SMCMR_PEN
#define SMCMR_SL	((ushort)0x0400)	/*                     */
#define SMCR_CLEN_MASK	((ushort)0x7800)	/*                  */
#define smcr_mk_clen(C)	(((C) << 11) & SMCR_CLEN_MASK)

/*                                                                 
                                                                 
                                                                  
                                 
 */
typedef struct smc_centronics {
	ushort	scent_rbase;
	ushort	scent_tbase;
	u_char	scent_cfcr;
	u_char	scent_smask;
	ushort	scent_mrblr;
	uint	scent_rstate;
	uint	scent_r_ptr;
	ushort	scent_rbptr;
	ushort	scent_r_cnt;
	uint	scent_rtemp;
	uint	scent_tstate;
	uint	scent_t_ptr;
	ushort	scent_tbptr;
	ushort	scent_t_cnt;
	uint	scent_ttemp;
	ushort	scent_max_sl;
	ushort	scent_sl_cnt;
	ushort	scent_character1;
	ushort	scent_character2;
	ushort	scent_character3;
	ushort	scent_character4;
	ushort	scent_character5;
	ushort	scent_character6;
	ushort	scent_character7;
	ushort	scent_character8;
	ushort	scent_rccm;
	ushort	scent_rccr;
} smc_cent_t;

/*                                 
*/
#define SMC_CENT_F	((u_char)0x08)
#define SMC_CENT_PE	((u_char)0x04)
#define SMC_CENT_S	((u_char)0x02)

/*                             
*/
#define	SMCM_BRKE	((unsigned char)0x40)	/*                   */
#define	SMCM_BRK	((unsigned char)0x10)	/*                   */
#define	SMCM_TXE	((unsigned char)0x10)	/*                          */
#define	SMCM_BSY	((unsigned char)0x04)
#define	SMCM_TX		((unsigned char)0x02)
#define	SMCM_RX		((unsigned char)0x01)

/*                      
*/
#define CPM_BRG_RST		((uint)0x00020000)
#define CPM_BRG_EN		((uint)0x00010000)
#define CPM_BRG_EXTC_INT	((uint)0x00000000)
#define CPM_BRG_EXTC_CLK2	((uint)0x00004000)
#define CPM_BRG_EXTC_CLK6	((uint)0x00008000)
#define CPM_BRG_ATB		((uint)0x00002000)
#define CPM_BRG_CD_MASK		((uint)0x00001ffe)
#define CPM_BRG_DIV16		((uint)0x00000001)

/*      
*/
#define SCC_GSMRH_IRP		((uint)0x00040000)
#define SCC_GSMRH_GDE		((uint)0x00010000)
#define SCC_GSMRH_TCRC_CCITT	((uint)0x00008000)
#define SCC_GSMRH_TCRC_BISYNC	((uint)0x00004000)
#define SCC_GSMRH_TCRC_HDLC	((uint)0x00000000)
#define SCC_GSMRH_REVD		((uint)0x00002000)
#define SCC_GSMRH_TRX		((uint)0x00001000)
#define SCC_GSMRH_TTX		((uint)0x00000800)
#define SCC_GSMRH_CDP		((uint)0x00000400)
#define SCC_GSMRH_CTSP		((uint)0x00000200)
#define SCC_GSMRH_CDS		((uint)0x00000100)
#define SCC_GSMRH_CTSS		((uint)0x00000080)
#define SCC_GSMRH_TFL		((uint)0x00000040)
#define SCC_GSMRH_RFW		((uint)0x00000020)
#define SCC_GSMRH_TXSY		((uint)0x00000010)
#define SCC_GSMRH_SYNL16	((uint)0x0000000c)
#define SCC_GSMRH_SYNL8		((uint)0x00000008)
#define SCC_GSMRH_SYNL4		((uint)0x00000004)
#define SCC_GSMRH_RTSM		((uint)0x00000002)
#define SCC_GSMRH_RSYN		((uint)0x00000001)

#define SCC_GSMRL_SIR		((uint)0x80000000)	/*           */
#define SCC_GSMRL_EDGE_NONE	((uint)0x60000000)
#define SCC_GSMRL_EDGE_NEG	((uint)0x40000000)
#define SCC_GSMRL_EDGE_POS	((uint)0x20000000)
#define SCC_GSMRL_EDGE_BOTH	((uint)0x00000000)
#define SCC_GSMRL_TCI		((uint)0x10000000)
#define SCC_GSMRL_TSNC_3	((uint)0x0c000000)
#define SCC_GSMRL_TSNC_4	((uint)0x08000000)
#define SCC_GSMRL_TSNC_14	((uint)0x04000000)
#define SCC_GSMRL_TSNC_INF	((uint)0x00000000)
#define SCC_GSMRL_RINV		((uint)0x02000000)
#define SCC_GSMRL_TINV		((uint)0x01000000)
#define SCC_GSMRL_TPL_128	((uint)0x00c00000)
#define SCC_GSMRL_TPL_64	((uint)0x00a00000)
#define SCC_GSMRL_TPL_48	((uint)0x00800000)
#define SCC_GSMRL_TPL_32	((uint)0x00600000)
#define SCC_GSMRL_TPL_16	((uint)0x00400000)
#define SCC_GSMRL_TPL_8		((uint)0x00200000)
#define SCC_GSMRL_TPL_NONE	((uint)0x00000000)
#define SCC_GSMRL_TPP_ALL1	((uint)0x00180000)
#define SCC_GSMRL_TPP_01	((uint)0x00100000)
#define SCC_GSMRL_TPP_10	((uint)0x00080000)
#define SCC_GSMRL_TPP_ZEROS	((uint)0x00000000)
#define SCC_GSMRL_TEND		((uint)0x00040000)
#define SCC_GSMRL_TDCR_32	((uint)0x00030000)
#define SCC_GSMRL_TDCR_16	((uint)0x00020000)
#define SCC_GSMRL_TDCR_8	((uint)0x00010000)
#define SCC_GSMRL_TDCR_1	((uint)0x00000000)
#define SCC_GSMRL_RDCR_32	((uint)0x0000c000)
#define SCC_GSMRL_RDCR_16	((uint)0x00008000)
#define SCC_GSMRL_RDCR_8	((uint)0x00004000)
#define SCC_GSMRL_RDCR_1	((uint)0x00000000)
#define SCC_GSMRL_RENC_DFMAN	((uint)0x00003000)
#define SCC_GSMRL_RENC_MANCH	((uint)0x00002000)
#define SCC_GSMRL_RENC_FM0	((uint)0x00001000)
#define SCC_GSMRL_RENC_NRZI	((uint)0x00000800)
#define SCC_GSMRL_RENC_NRZ	((uint)0x00000000)
#define SCC_GSMRL_TENC_DFMAN	((uint)0x00000600)
#define SCC_GSMRL_TENC_MANCH	((uint)0x00000400)
#define SCC_GSMRL_TENC_FM0	((uint)0x00000200)
#define SCC_GSMRL_TENC_NRZI	((uint)0x00000100)
#define SCC_GSMRL_TENC_NRZ	((uint)0x00000000)
#define SCC_GSMRL_DIAG_LE	((uint)0x000000c0)	/*               */
#define SCC_GSMRL_DIAG_ECHO	((uint)0x00000080)
#define SCC_GSMRL_DIAG_LOOP	((uint)0x00000040)
#define SCC_GSMRL_DIAG_NORM	((uint)0x00000000)
#define SCC_GSMRL_ENR		((uint)0x00000020)
#define SCC_GSMRL_ENT		((uint)0x00000010)
#define SCC_GSMRL_MODE_ENET	((uint)0x0000000c)
#define SCC_GSMRL_MODE_DDCMP	((uint)0x00000009)
#define SCC_GSMRL_MODE_BISYNC	((uint)0x00000008)
#define SCC_GSMRL_MODE_V14	((uint)0x00000007)
#define SCC_GSMRL_MODE_AHDLC	((uint)0x00000006)
#define SCC_GSMRL_MODE_PROFIBUS	((uint)0x00000005)
#define SCC_GSMRL_MODE_UART	((uint)0x00000004)
#define SCC_GSMRL_MODE_SS7	((uint)0x00000003)
#define SCC_GSMRL_MODE_ATALK	((uint)0x00000002)
#define SCC_GSMRL_MODE_HDLC	((uint)0x00000000)

#define SCC_TODR_TOD		((ushort)0x8000)

/*                             
*/
#define	SCCM_TXE	((unsigned char)0x10)
#define	SCCM_BSY	((unsigned char)0x04)
#define	SCCM_TX		((unsigned char)0x02)
#define	SCCM_RX		((unsigned char)0x01)

typedef struct scc_param {
	ushort	scc_rbase;	/*                                   */
	ushort	scc_tbase;	/*                                   */
	u_char	scc_rfcr;	/*                  */
	u_char	scc_tfcr;	/*                  */
	ushort	scc_mrblr;	/*                           */
	uint	scc_rstate;	/*          */
	uint	scc_idp;	/*          */
	ushort	scc_rbptr;	/*          */
	ushort	scc_ibc;	/*          */
	uint	scc_rxtmp;	/*          */
	uint	scc_tstate;	/*          */
	uint	scc_tdp;	/*          */
	ushort	scc_tbptr;	/*          */
	ushort	scc_tbc;	/*          */
	uint	scc_txtmp;	/*          */
	uint	scc_rcrc;	/*          */
	uint	scc_tcrc;	/*          */
} sccp_t;


/*                    
 */
#define SCC_EB	((u_char)0x10)	/*                           */
#define SCC_FC_DMA ((u_char)0x08) /*          */

/*                           
 */
typedef struct scc_enet {
	sccp_t	sen_genscc;
	uint	sen_cpres;	/*            */
	uint	sen_cmask;	/*                       */
	uint	sen_crcec;	/*                   */
	uint	sen_alec;	/*                         */
	uint	sen_disfc;	/*                       */
	ushort	sen_pads;	/*                              */
	ushort	sen_retlim;	/*                       */
	ushort	sen_retcnt;	/*                     */
	ushort	sen_maxflr;	/*                               */
	ushort	sen_minflr;	/*                               */
	ushort	sen_maxd1;	/*                     */
	ushort	sen_maxd2;	/*                     */
	ushort	sen_maxd;	/*            */
	ushort	sen_dmacnt;	/*                */
	ushort	sen_maxb;	/*                   */
	ushort	sen_gaddr1;	/*                      */
	ushort	sen_gaddr2;
	ushort	sen_gaddr3;
	ushort	sen_gaddr4;
	uint	sen_tbuf0data0;	/*                             */
	uint	sen_tbuf0data1;	/*                             */
	uint	sen_tbuf0rba;	/*          */
	uint	sen_tbuf0crc;	/*          */
	ushort	sen_tbuf0bcnt;	/*          */
	ushort	sen_paddrh;	/*                        */
	ushort	sen_paddrm;
	ushort	sen_paddrl;	/*                        */
	ushort	sen_pper;	/*             */
	ushort	sen_rfbdptr;	/*                     */
	ushort	sen_tfbdptr;	/*                     */
	ushort	sen_tlbdptr;	/*                    */
	uint	sen_tbuf1data0;	/*                             */
	uint	sen_tbuf1data1;	/*                             */
	uint	sen_tbuf1rba;	/*          */
	uint	sen_tbuf1crc;	/*          */
	ushort	sen_tbuf1bcnt;	/*          */
	ushort	sen_txlen;	/*                         */
	ushort	sen_iaddr1;	/*                           */
	ushort	sen_iaddr2;
	ushort	sen_iaddr3;
	ushort	sen_iaddr4;
	ushort	sen_boffcnt;	/*                 */

	/*                                                           
                                                       
  */
	ushort	sen_taddrh;	/*                    */
	ushort	sen_taddrm;
	ushort	sen_taddrl;	/*                    */
} scc_enet_t;



#if defined (CONFIG_UCQUICC)
/*                                                         
                     
                  
                  
                   
                   
                   
                  
                 
 */
#define PA_ENET_RXD	PA_RXD1
#define PA_ENET_TXD	PA_TXD1
#define PA_ENET_TCLK	PA_CLK1
#define PA_ENET_RCLK	PA_CLK2
#define PC_ENET_TENA	PC_RTS1
#define PC_ENET_CLSN	PC_CTS1
#define PC_ENET_RENA	PC_CD1

/*                                                                 
        
 */
#define SICR_ENET_MASK	((uint)0x000000ff)
#define SICR_ENET_CLKRT	((uint)0x0000002c)

#endif /*                */


#ifdef MBX
/*                                                                
                                                                     
                                                                      
                                                                      
              
 */
#define PA_ENET_RXD	((ushort)0x0001)
#define PA_ENET_TXD	((ushort)0x0002)
#define PA_ENET_TCLK	((ushort)0x0200)
#define PA_ENET_RCLK	((ushort)0x0800)
#define PC_ENET_TENA	((ushort)0x0001)
#define PC_ENET_CLSN	((ushort)0x0010)
#define PC_ENET_RENA	((ushort)0x0020)

/*                                                                 
                                                                 
 */
#define SICR_ENET_MASK	((uint)0x000000ff)
#define SICR_ENET_CLKRT	((uint)0x0000003d)
#endif

#ifdef CONFIG_BSEIP
/*                                                         
                                             
 */
#define PA_ENET_RXD	((ushort)0x0004)
#define PA_ENET_TXD	((ushort)0x0008)
#define PA_ENET_TCLK	((ushort)0x0100)
#define PA_ENET_RCLK	((ushort)0x0200)
#define PB_ENET_TENA	((uint)0x00002000)
#define PC_ENET_CLSN	((ushort)0x0040)
#define PC_ENET_RENA	((ushort)0x0080)

/*                                                 
*/
#define PB_BSE_POWERUP	((uint)0x00000004)
#define PB_BSE_FDXDIS	((uint)0x00008000)
#define PC_BSE_LOOPBACK	((ushort)0x0800)

#define SICR_ENET_MASK	((uint)0x0000ff00)
#define SICR_ENET_CLKRT	((uint)0x00002c00)
#endif

/*                                        
*/
#define SCCE_ENET_GRA	((ushort)0x0080)	/*                        */
#define SCCE_ENET_TXE	((ushort)0x0010)	/*                */
#define SCCE_ENET_RXF	((ushort)0x0008)	/*                     */
#define SCCE_ENET_BSY	((ushort)0x0004)	/*                           */
#define SCCE_ENET_TXB	((ushort)0x0002)	/*                          */
#define SCCE_ENET_RXB	((ushort)0x0001)	/*                       */

/*                                              
*/
#define SCC_PMSR_HBC	((ushort)0x8000)	/*                  */
#define SCC_PMSR_FC	((ushort)0x4000)	/*                 */
#define SCC_PMSR_RSH	((ushort)0x2000)	/*                      */
#define SCC_PMSR_IAM	((ushort)0x1000)	/*                       */
#define SCC_PMSR_ENCRC	((ushort)0x0800)	/*                   */
#define SCC_PMSR_PRO	((ushort)0x0200)	/*                  */
#define SCC_PMSR_BRO	((ushort)0x0100)	/*                      */
#define SCC_PMSR_SBT	((ushort)0x0080)	/*                       */
#define SCC_PMSR_LPB	((ushort)0x0040)	/*                   */
#define SCC_PMSR_SIP	((ushort)0x0020)	/*                   */
#define SCC_PMSR_LCW	((ushort)0x0010)	/*                       */
#define SCC_PMSR_NIB22	((ushort)0x000a)	/*                    */
#define SCC_PMSR_FDE	((ushort)0x0001)	/*                    */

/*                                                           
*/
#define BD_ENET_RX_EMPTY	((ushort)0x8000)
#define BD_ENET_RX_WRAP		((ushort)0x2000)
#define BD_ENET_RX_INTR		((ushort)0x1000)
#define BD_ENET_RX_LAST		((ushort)0x0800)
#define BD_ENET_RX_FIRST	((ushort)0x0400)
#define BD_ENET_RX_MISS		((ushort)0x0100)
#define BD_ENET_RX_LG		((ushort)0x0020)
#define BD_ENET_RX_NO		((ushort)0x0010)
#define BD_ENET_RX_SH		((ushort)0x0008)
#define BD_ENET_RX_CR		((ushort)0x0004)
#define BD_ENET_RX_OV		((ushort)0x0002)
#define BD_ENET_RX_CL		((ushort)0x0001)
#define BD_ENET_RX_STATS	((ushort)0x013f)	/*                 */

/*                                                            
*/
#define BD_ENET_TX_READY	((ushort)0x8000)
#define BD_ENET_TX_PAD		((ushort)0x4000)
#define BD_ENET_TX_WRAP		((ushort)0x2000)
#define BD_ENET_TX_INTR		((ushort)0x1000)
#define BD_ENET_TX_LAST		((ushort)0x0800)
#define BD_ENET_TX_TC		((ushort)0x0400)
#define BD_ENET_TX_DEF		((ushort)0x0200)
#define BD_ENET_TX_HB		((ushort)0x0100)
#define BD_ENET_TX_LC		((ushort)0x0080)
#define BD_ENET_TX_RL		((ushort)0x0040)
#define BD_ENET_TX_RCMASK	((ushort)0x003c)
#define BD_ENET_TX_UN		((ushort)0x0002)
#define BD_ENET_TX_CSL		((ushort)0x0001)
#define BD_ENET_TX_STATS	((ushort)0x03ff)	/*                 */

/*            
*/
typedef struct scc_uart {
	sccp_t	scc_genscc;
	uint	scc_res1;	/*          */
	uint	scc_res2;	/*          */
	ushort	scc_maxidl;	/*                    */
	ushort	scc_idlc;	/*                   */
	ushort	scc_brkcr;	/*                      */
	ushort	scc_parec;	/*                              */
	ushort	scc_frmec;	/*                               */
	ushort	scc_nosec;	/*                       */
	ushort	scc_brkec;	/*                                 */
	ushort	scc_brkln;	/*                            */
	ushort	scc_uaddr1;	/*                          */
	ushort	scc_uaddr2;	/*                          */
	ushort	scc_rtemp;	/*              */
	ushort	scc_toseq;	/*                               */
	ushort	scc_char1;	/*                     */
	ushort	scc_char2;	/*                     */
	ushort	scc_char3;	/*                     */
	ushort	scc_char4;	/*                     */
	ushort	scc_char5;	/*                     */
	ushort	scc_char6;	/*                     */
	ushort	scc_char7;	/*                     */
	ushort	scc_char8;	/*                     */
	ushort	scc_rccm;	/*                                */
	ushort	scc_rccr;	/*                                    */
	ushort	scc_rlbc;	/*                              */
} scc_uart_t;

/*                                                        
*/
#define UART_SCCM_GLR		((ushort)0x1000)
#define UART_SCCM_GLT		((ushort)0x0800)
#define UART_SCCM_AB		((ushort)0x0200)
#define UART_SCCM_IDL		((ushort)0x0100)
#define UART_SCCM_GRA		((ushort)0x0080)
#define UART_SCCM_BRKE		((ushort)0x0040)
#define UART_SCCM_BRKS		((ushort)0x0020)
#define UART_SCCM_CCR		((ushort)0x0008)
#define UART_SCCM_BSY		((ushort)0x0004)
#define UART_SCCM_TX		((ushort)0x0002)
#define UART_SCCM_RX		((ushort)0x0001)

/*                                  
*/
#define SCU_PMSR_FLC		((ushort)0x8000)
#define SCU_PMSR_SL		((ushort)0x4000)
#define SCU_PMSR_CL		((ushort)0x3000)
#define SCU_PMSR_UM		((ushort)0x0c00)
#define SCU_PMSR_FRZ		((ushort)0x0200)
#define SCU_PMSR_RZS		((ushort)0x0100)
#define SCU_PMSR_SYN		((ushort)0x0080)
#define SCU_PMSR_DRT		((ushort)0x0040)
#define SCU_PMSR_PEN		((ushort)0x0010)
#define SCU_PMSR_RPM		((ushort)0x000c)
#define SCU_PMSR_REVP		((ushort)0x0008)
#define SCU_PMSR_TPM		((ushort)0x0003)
#define SCU_PMSR_TEVP		((ushort)0x0003)

/*                          
 */
typedef struct scc_trans {
	sccp_t	st_genscc;
	uint	st_cpres;	/*            */
	uint	st_cmask;	/*                       */
} scc_trans_t;

#define BD_SCC_TX_LAST		((ushort)0x0800)



/*                                                                 
                                                                   
                                                                   
                                                                  
                                                                
                                      
 */
/*                       */
/*                                         */
/*                                     */
/*                                     */
/*                                     */
/*                                     */
/*                                         */
/*                                       */
/*                                         */
/*                                         */
/*                                           */
/*                                      */
/*                                      */
/*                                       */
/*                                         */
/*                                    */
/*                                         */
/*                                         */
/*                                       */
/*                                        */
/*                                        */
/*                                        */
/*                                       */
/*                                        */
/*                                    */
/*                                     */
/*                                     */
/*                                        */
/*                                        */
/*                                      */

extern void cpm_install_handler(int vec, void (*handler)(void *), void *dev_id);

/*                                    
*/
#define	CICR_SCD_SCC4		((uint)0x00c00000)	/*             */
#define	CICR_SCC_SCC3		((uint)0x00200000)	/*             */
#define	CICR_SCB_SCC2		((uint)0x00040000)	/*             */
#define	CICR_SCA_SCC1		((uint)0x00000000)	/*             */
#define CICR_IRL_MASK		((uint)0x0000e000)	/*                */
#define CICR_HP_MASK		((uint)0x00001f00)	/*             */
#define CICR_IEN		((uint)0x00000080)	/*             */
#define CICR_SPS		((uint)0x00000001)	/*            */
#endif /*             */
