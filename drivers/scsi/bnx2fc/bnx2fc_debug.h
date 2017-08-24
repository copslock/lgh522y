#ifndef __BNX2FC_DEBUG__
#define __BNX2FC_DEBUG__

/*                    */
#define LOG_IO		0x01	/*                         */
#define LOG_TGT		0x02	/*                              */
#define LOG_HBA		0x04	/*                               */
#define LOG_ELS		0x08	/*          */
#define LOG_MISC	0x10	/*                           */
#define LOG_ALL		0xff	/*                  */

extern unsigned int bnx2fc_debug_level;

#define BNX2FC_ELS_DBG(fmt, ...)				\
do {								\
	if (unlikely(bnx2fc_debug_level & LOG_ELS))		\
		pr_info(fmt, ##__VA_ARGS__);			\
} while (0)

#define BNX2FC_MISC_DBG(fmt, ...)				\
do {								\
	if (unlikely(bnx2fc_debug_level & LOG_MISC))		\
		pr_info(fmt, ##__VA_ARGS__);			\
} while (0)

__printf(2, 3)
void BNX2FC_IO_DBG(const struct bnx2fc_cmd *io_req, const char *fmt, ...);
__printf(2, 3)
void BNX2FC_TGT_DBG(const struct bnx2fc_rport *tgt, const char *fmt, ...);
__printf(2, 3)
void BNX2FC_HBA_DBG(const struct fc_lport *lport, const char *fmt, ...);

#endif
