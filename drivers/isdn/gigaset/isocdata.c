/*
 * Common data handling layer for bas_gigaset
 *
 * Copyright (c) 2005 by Tilman Schmidt <tilman@imap.cc>,
 *                       Hansjoerg Lipp <hjlipp@web.de>.
 *
 * =====================================================================
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as
 *	published by the Free Software Foundation; either version 2 of
 *	the License, or (at your option) any later version.
 * =====================================================================
 */

#include "gigaset.h"
#include <linux/crc-ccitt.h>
#include <linux/bitrev.h>

/*                              */
/*                              */

/*                            
 */
void gigaset_isowbuf_init(struct isowbuf_t *iwb, unsigned char idle)
{
	iwb->read = 0;
	iwb->nextread = 0;
	iwb->write = 0;
	atomic_set(&iwb->writesem, 1);
	iwb->wbits = 0;
	iwb->idle = idle;
	memset(iwb->data + BAS_OUTBUFSIZE, idle, BAS_OUTBUFPAD);
}

/*                                                        
                                                                 
 */
static inline int isowbuf_freebytes(struct isowbuf_t *iwb)
{
	int read, write, freebytes;

	read = iwb->read;
	write = iwb->write;
	freebytes = read - write;
	if (freebytes > 0) {
		/*                                                       */
		return freebytes - BAS_OUTBUFPAD;
	} else if (read < BAS_OUTBUFPAD) {
		/*                                                     */
		return BAS_OUTBUFSIZE - write;
	} else {
		/*                                            */
		return freebytes + BAS_OUTBUFSIZE - BAS_OUTBUFPAD;
	}
}

/*              
                              
                                   
 */
static inline int isowbuf_startwrite(struct isowbuf_t *iwb)
{
	if (!atomic_dec_and_test(&iwb->writesem)) {
		atomic_inc(&iwb->writesem);
		gig_dbg(DEBUG_ISO, "%s: couldn't acquire iso write semaphore",
			__func__);
		return -EBUSY;
	}
	gig_dbg(DEBUG_ISO,
		"%s: acquired iso write semaphore, data[write]=%02x, nbits=%d",
		__func__, iwb->data[iwb->write], iwb->wbits);
	return 0;
}

/*               
                              
                                     
 */
static inline int isowbuf_donewrite(struct isowbuf_t *iwb)
{
	int write = iwb->write;
	atomic_inc(&iwb->writesem);
	return write;
}

/*                                         
                                                  
                                              
                                               
                                                                              
                                                                
 */
static inline void isowbuf_putbits(struct isowbuf_t *iwb, u32 data, int nbits)
{
	int write = iwb->write;
	data <<= iwb->wbits;
	data |= iwb->data[write];
	nbits += iwb->wbits;
	while (nbits >= 8) {
		iwb->data[write++] = data & 0xff;
		write %= BAS_OUTBUFSIZE;
		data >>= 8;
		nbits -= 8;
	}
	iwb->wbits = nbits;
	iwb->data[write] = data & 0xff;
	iwb->write = write;
}

/*                                 
                                                                           
                                               
 */
static inline void isowbuf_putflag(struct isowbuf_t *iwb)
{
	int write;

	/*                                                */
	isowbuf_putbits(iwb, 0x7e7e, 8);
	/*                            */
	write = iwb->write;
	iwb->idle = iwb->data[write];
	gig_dbg(DEBUG_ISO, "idle fill byte %02x", iwb->idle);
	/*                                */
	iwb->data[write] &= (1 << iwb->wbits) - 1;
}

/*                                      
                                                                   
                                                                     
                       
                                                
 */
int gigaset_isowbuf_getbytes(struct isowbuf_t *iwb, int size)
{
	int read, write, limit, src, dst;
	unsigned char pbyte;

	read = iwb->nextread;
	write = iwb->write;
	if (likely(read == write)) {
		/*                   */
		return read < BAS_OUTBUFPAD ?
			BAS_OUTBUFSIZE : read - BAS_OUTBUFPAD;
	}

	limit = read + size;
	gig_dbg(DEBUG_STREAM, "%s: read=%d write=%d limit=%d",
		__func__, read, write, limit);
#ifdef CONFIG_GIGASET_DEBUG
	if (unlikely(size < 0 || size > BAS_OUTBUFPAD)) {
		pr_err("invalid size %d\n", size);
		return -EINVAL;
	}
#endif

	if (read < write) {
		/*                             */
		if (limit >= write) {
			/*                   */
			if (isowbuf_startwrite(iwb) < 0)
				return -EBUSY;
			/*                                   */
			write = iwb->write;
			if (limit >= write) {
				pbyte = iwb->data[write]; /*     
                         */
				limit = write + BAS_OUTBUFPAD;
				gig_dbg(DEBUG_STREAM,
					"%s: filling %d->%d with %02x",
					__func__, write, limit, iwb->idle);
				if (write + BAS_OUTBUFPAD < BAS_OUTBUFSIZE)
					memset(iwb->data + write, iwb->idle,
					       BAS_OUTBUFPAD);
				else {
					/*                                  */
					memset(iwb->data + write, iwb->idle,
					       BAS_OUTBUFSIZE + BAS_OUTBUFPAD
					       - write);
					limit = 0;
				}
				gig_dbg(DEBUG_STREAM,
					"%s: restoring %02x at %d",
					__func__, pbyte, limit);
				iwb->data[limit] = pbyte; /*        
                         */
				iwb->write = limit;
			}
			isowbuf_donewrite(iwb);
		}
	} else {
		/*                       */
		if (limit >= BAS_OUTBUFSIZE) {
			/*                                 */
			src = 0;
			dst = BAS_OUTBUFSIZE;
			while (dst < limit && src < write)
				iwb->data[dst++] = iwb->data[src++];
			if (dst <= limit) {
				/*                              */
				memset(iwb->data + dst, iwb->idle,
				       BAS_OUTBUFSIZE + BAS_OUTBUFPAD - dst);
			}
			limit = src;
		}
	}
	iwb->nextread = limit;
	return read;
}

/*           
                                          
 */
static inline void dump_bytes(enum debuglevel level, const char *tag,
			      unsigned char *bytes, int count)
{
#ifdef CONFIG_GIGASET_DEBUG
	unsigned char c;
	static char dbgline[3 * 32 + 1];
	int i = 0;

	if (!(gigaset_debuglevel & level))
		return;

	while (count-- > 0) {
		if (i > sizeof(dbgline) - 4) {
			dbgline[i] = '\0';
			gig_dbg(level, "%s:%s", tag, dbgline);
			i = 0;
		}
		c = *bytes++;
		dbgline[i] = (i && !(i % 12)) ? '-' : ' ';
		i++;
		dbgline[i++] = hex_asc_hi(c);
		dbgline[i++] = hex_asc_lo(c);
	}
	dbgline[i] = '\0';
	gig_dbg(level, "%s:%s", tag, dbgline);
#endif
}

/*                                                                            */

/*                                           
                                                                    
                                                                   
                                  
                                                            
                                                       
 */
static const u16 stufftab[5 * 256] = {
/*                  */
	0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
	0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x201f,
	0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
	0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x203e, 0x205f,
	0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
	0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x209f,
	0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
	0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x207c, 0x207d, 0x20be, 0x20df,
	0x0480, 0x0481, 0x0482, 0x0483, 0x0484, 0x0485, 0x0486, 0x0487, 0x0488, 0x0489, 0x048a, 0x048b, 0x048c, 0x048d, 0x048e, 0x048f,
	0x0490, 0x0491, 0x0492, 0x0493, 0x0494, 0x0495, 0x0496, 0x0497, 0x0498, 0x0499, 0x049a, 0x049b, 0x049c, 0x049d, 0x049e, 0x251f,
	0x04a0, 0x04a1, 0x04a2, 0x04a3, 0x04a4, 0x04a5, 0x04a6, 0x04a7, 0x04a8, 0x04a9, 0x04aa, 0x04ab, 0x04ac, 0x04ad, 0x04ae, 0x04af,
	0x04b0, 0x04b1, 0x04b2, 0x04b3, 0x04b4, 0x04b5, 0x04b6, 0x04b7, 0x04b8, 0x04b9, 0x04ba, 0x04bb, 0x04bc, 0x04bd, 0x253e, 0x255f,
	0x08c0, 0x08c1, 0x08c2, 0x08c3, 0x08c4, 0x08c5, 0x08c6, 0x08c7, 0x08c8, 0x08c9, 0x08ca, 0x08cb, 0x08cc, 0x08cd, 0x08ce, 0x08cf,
	0x08d0, 0x08d1, 0x08d2, 0x08d3, 0x08d4, 0x08d5, 0x08d6, 0x08d7, 0x08d8, 0x08d9, 0x08da, 0x08db, 0x08dc, 0x08dd, 0x08de, 0x299f,
	0x0ce0, 0x0ce1, 0x0ce2, 0x0ce3, 0x0ce4, 0x0ce5, 0x0ce6, 0x0ce7, 0x0ce8, 0x0ce9, 0x0cea, 0x0ceb, 0x0cec, 0x0ced, 0x0cee, 0x0cef,
	0x10f0, 0x10f1, 0x10f2, 0x10f3, 0x10f4, 0x10f5, 0x10f6, 0x10f7, 0x20f8, 0x20f9, 0x20fa, 0x20fb, 0x257c, 0x257d, 0x29be, 0x2ddf,

/*                  */
	0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x200f,
	0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x202f,
	0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x204f,
	0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x203e, 0x206f,
	0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x208f,
	0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x20af,
	0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x20cf,
	0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007a, 0x007b, 0x207c, 0x207d, 0x20be, 0x20ef,
	0x0480, 0x0481, 0x0482, 0x0483, 0x0484, 0x0485, 0x0486, 0x0487, 0x0488, 0x0489, 0x048a, 0x048b, 0x048c, 0x048d, 0x048e, 0x250f,
	0x0490, 0x0491, 0x0492, 0x0493, 0x0494, 0x0495, 0x0496, 0x0497, 0x0498, 0x0499, 0x049a, 0x049b, 0x049c, 0x049d, 0x049e, 0x252f,
	0x04a0, 0x04a1, 0x04a2, 0x04a3, 0x04a4, 0x04a5, 0x04a6, 0x04a7, 0x04a8, 0x04a9, 0x04aa, 0x04ab, 0x04ac, 0x04ad, 0x04ae, 0x254f,
	0x04b0, 0x04b1, 0x04b2, 0x04b3, 0x04b4, 0x04b5, 0x04b6, 0x04b7, 0x04b8, 0x04b9, 0x04ba, 0x04bb, 0x04bc, 0x04bd, 0x253e, 0x256f,
	0x08c0, 0x08c1, 0x08c2, 0x08c3, 0x08c4, 0x08c5, 0x08c6, 0x08c7, 0x08c8, 0x08c9, 0x08ca, 0x08cb, 0x08cc, 0x08cd, 0x08ce, 0x298f,
	0x08d0, 0x08d1, 0x08d2, 0x08d3, 0x08d4, 0x08d5, 0x08d6, 0x08d7, 0x08d8, 0x08d9, 0x08da, 0x08db, 0x08dc, 0x08dd, 0x08de, 0x29af,
	0x0ce0, 0x0ce1, 0x0ce2, 0x0ce3, 0x0ce4, 0x0ce5, 0x0ce6, 0x0ce7, 0x0ce8, 0x0ce9, 0x0cea, 0x0ceb, 0x0cec, 0x0ced, 0x0cee, 0x2dcf,
	0x10f0, 0x10f1, 0x10f2, 0x10f3, 0x10f4, 0x10f5, 0x10f6, 0x10f7, 0x20f8, 0x20f9, 0x20fa, 0x20fb, 0x257c, 0x257d, 0x29be, 0x31ef,

/*                  */
	0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x2007, 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x2017,
	0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x2027, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x2037,
	0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x2047, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x2057,
	0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x2067, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x203e, 0x2077,
	0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x2087, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x2097,
	0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x20a7, 0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x20b7,
	0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x20c7, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x20d7,
	0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x20e7, 0x0078, 0x0079, 0x007a, 0x007b, 0x207c, 0x207d, 0x20be, 0x20f7,
	0x0480, 0x0481, 0x0482, 0x0483, 0x0484, 0x0485, 0x0486, 0x2507, 0x0488, 0x0489, 0x048a, 0x048b, 0x048c, 0x048d, 0x048e, 0x2517,
	0x0490, 0x0491, 0x0492, 0x0493, 0x0494, 0x0495, 0x0496, 0x2527, 0x0498, 0x0499, 0x049a, 0x049b, 0x049c, 0x049d, 0x049e, 0x2537,
	0x04a0, 0x04a1, 0x04a2, 0x04a3, 0x04a4, 0x04a5, 0x04a6, 0x2547, 0x04a8, 0x04a9, 0x04aa, 0x04ab, 0x04ac, 0x04ad, 0x04ae, 0x2557,
	0x04b0, 0x04b1, 0x04b2, 0x04b3, 0x04b4, 0x04b5, 0x04b6, 0x2567, 0x04b8, 0x04b9, 0x04ba, 0x04bb, 0x04bc, 0x04bd, 0x253e, 0x2577,
	0x08c0, 0x08c1, 0x08c2, 0x08c3, 0x08c4, 0x08c5, 0x08c6, 0x2987, 0x08c8, 0x08c9, 0x08ca, 0x08cb, 0x08cc, 0x08cd, 0x08ce, 0x2997,
	0x08d0, 0x08d1, 0x08d2, 0x08d3, 0x08d4, 0x08d5, 0x08d6, 0x29a7, 0x08d8, 0x08d9, 0x08da, 0x08db, 0x08dc, 0x08dd, 0x08de, 0x29b7,
	0x0ce0, 0x0ce1, 0x0ce2, 0x0ce3, 0x0ce4, 0x0ce5, 0x0ce6, 0x2dc7, 0x0ce8, 0x0ce9, 0x0cea, 0x0ceb, 0x0cec, 0x0ced, 0x0cee, 0x2dd7,
	0x10f0, 0x10f1, 0x10f2, 0x10f3, 0x10f4, 0x10f5, 0x10f6, 0x31e7, 0x20f8, 0x20f9, 0x20fa, 0x20fb, 0x257c, 0x257d, 0x29be, 0x41f7,

/*                  */
	0x0000, 0x0001, 0x0002, 0x2003, 0x0004, 0x0005, 0x0006, 0x200b, 0x0008, 0x0009, 0x000a, 0x2013, 0x000c, 0x000d, 0x000e, 0x201b,
	0x0010, 0x0011, 0x0012, 0x2023, 0x0014, 0x0015, 0x0016, 0x202b, 0x0018, 0x0019, 0x001a, 0x2033, 0x001c, 0x001d, 0x001e, 0x203b,
	0x0020, 0x0021, 0x0022, 0x2043, 0x0024, 0x0025, 0x0026, 0x204b, 0x0028, 0x0029, 0x002a, 0x2053, 0x002c, 0x002d, 0x002e, 0x205b,
	0x0030, 0x0031, 0x0032, 0x2063, 0x0034, 0x0035, 0x0036, 0x206b, 0x0038, 0x0039, 0x003a, 0x2073, 0x003c, 0x003d, 0x203e, 0x207b,
	0x0040, 0x0041, 0x0042, 0x2083, 0x0044, 0x0045, 0x0046, 0x208b, 0x0048, 0x0049, 0x004a, 0x2093, 0x004c, 0x004d, 0x004e, 0x209b,
	0x0050, 0x0051, 0x0052, 0x20a3, 0x0054, 0x0055, 0x0056, 0x20ab, 0x0058, 0x0059, 0x005a, 0x20b3, 0x005c, 0x005d, 0x005e, 0x20bb,
	0x0060, 0x0061, 0x0062, 0x20c3, 0x0064, 0x0065, 0x0066, 0x20cb, 0x0068, 0x0069, 0x006a, 0x20d3, 0x006c, 0x006d, 0x006e, 0x20db,
	0x0070, 0x0071, 0x0072, 0x20e3, 0x0074, 0x0075, 0x0076, 0x20eb, 0x0078, 0x0079, 0x007a, 0x20f3, 0x207c, 0x207d, 0x20be, 0x40fb,
	0x0480, 0x0481, 0x0482, 0x2503, 0x0484, 0x0485, 0x0486, 0x250b, 0x0488, 0x0489, 0x048a, 0x2513, 0x048c, 0x048d, 0x048e, 0x251b,
	0x0490, 0x0491, 0x0492, 0x2523, 0x0494, 0x0495, 0x0496, 0x252b, 0x0498, 0x0499, 0x049a, 0x2533, 0x049c, 0x049d, 0x049e, 0x253b,
	0x04a0, 0x04a1, 0x04a2, 0x2543, 0x04a4, 0x04a5, 0x04a6, 0x254b, 0x04a8, 0x04a9, 0x04aa, 0x2553, 0x04ac, 0x04ad, 0x04ae, 0x255b,
	0x04b0, 0x04b1, 0x04b2, 0x2563, 0x04b4, 0x04b5, 0x04b6, 0x256b, 0x04b8, 0x04b9, 0x04ba, 0x2573, 0x04bc, 0x04bd, 0x253e, 0x257b,
	0x08c0, 0x08c1, 0x08c2, 0x2983, 0x08c4, 0x08c5, 0x08c6, 0x298b, 0x08c8, 0x08c9, 0x08ca, 0x2993, 0x08cc, 0x08cd, 0x08ce, 0x299b,
	0x08d0, 0x08d1, 0x08d2, 0x29a3, 0x08d4, 0x08d5, 0x08d6, 0x29ab, 0x08d8, 0x08d9, 0x08da, 0x29b3, 0x08dc, 0x08dd, 0x08de, 0x29bb,
	0x0ce0, 0x0ce1, 0x0ce2, 0x2dc3, 0x0ce4, 0x0ce5, 0x0ce6, 0x2dcb, 0x0ce8, 0x0ce9, 0x0cea, 0x2dd3, 0x0cec, 0x0ced, 0x0cee, 0x2ddb,
	0x10f0, 0x10f1, 0x10f2, 0x31e3, 0x10f4, 0x10f5, 0x10f6, 0x31eb, 0x20f8, 0x20f9, 0x20fa, 0x41f3, 0x257c, 0x257d, 0x29be, 0x46fb,

/*                  */
	0x0000, 0x2001, 0x0002, 0x2005, 0x0004, 0x2009, 0x0006, 0x200d, 0x0008, 0x2011, 0x000a, 0x2015, 0x000c, 0x2019, 0x000e, 0x201d,
	0x0010, 0x2021, 0x0012, 0x2025, 0x0014, 0x2029, 0x0016, 0x202d, 0x0018, 0x2031, 0x001a, 0x2035, 0x001c, 0x2039, 0x001e, 0x203d,
	0x0020, 0x2041, 0x0022, 0x2045, 0x0024, 0x2049, 0x0026, 0x204d, 0x0028, 0x2051, 0x002a, 0x2055, 0x002c, 0x2059, 0x002e, 0x205d,
	0x0030, 0x2061, 0x0032, 0x2065, 0x0034, 0x2069, 0x0036, 0x206d, 0x0038, 0x2071, 0x003a, 0x2075, 0x003c, 0x2079, 0x203e, 0x407d,
	0x0040, 0x2081, 0x0042, 0x2085, 0x0044, 0x2089, 0x0046, 0x208d, 0x0048, 0x2091, 0x004a, 0x2095, 0x004c, 0x2099, 0x004e, 0x209d,
	0x0050, 0x20a1, 0x0052, 0x20a5, 0x0054, 0x20a9, 0x0056, 0x20ad, 0x0058, 0x20b1, 0x005a, 0x20b5, 0x005c, 0x20b9, 0x005e, 0x20bd,
	0x0060, 0x20c1, 0x0062, 0x20c5, 0x0064, 0x20c9, 0x0066, 0x20cd, 0x0068, 0x20d1, 0x006a, 0x20d5, 0x006c, 0x20d9, 0x006e, 0x20dd,
	0x0070, 0x20e1, 0x0072, 0x20e5, 0x0074, 0x20e9, 0x0076, 0x20ed, 0x0078, 0x20f1, 0x007a, 0x20f5, 0x207c, 0x40f9, 0x20be, 0x417d,
	0x0480, 0x2501, 0x0482, 0x2505, 0x0484, 0x2509, 0x0486, 0x250d, 0x0488, 0x2511, 0x048a, 0x2515, 0x048c, 0x2519, 0x048e, 0x251d,
	0x0490, 0x2521, 0x0492, 0x2525, 0x0494, 0x2529, 0x0496, 0x252d, 0x0498, 0x2531, 0x049a, 0x2535, 0x049c, 0x2539, 0x049e, 0x253d,
	0x04a0, 0x2541, 0x04a2, 0x2545, 0x04a4, 0x2549, 0x04a6, 0x254d, 0x04a8, 0x2551, 0x04aa, 0x2555, 0x04ac, 0x2559, 0x04ae, 0x255d,
	0x04b0, 0x2561, 0x04b2, 0x2565, 0x04b4, 0x2569, 0x04b6, 0x256d, 0x04b8, 0x2571, 0x04ba, 0x2575, 0x04bc, 0x2579, 0x253e, 0x467d,
	0x08c0, 0x2981, 0x08c2, 0x2985, 0x08c4, 0x2989, 0x08c6, 0x298d, 0x08c8, 0x2991, 0x08ca, 0x2995, 0x08cc, 0x2999, 0x08ce, 0x299d,
	0x08d0, 0x29a1, 0x08d2, 0x29a5, 0x08d4, 0x29a9, 0x08d6, 0x29ad, 0x08d8, 0x29b1, 0x08da, 0x29b5, 0x08dc, 0x29b9, 0x08de, 0x29bd,
	0x0ce0, 0x2dc1, 0x0ce2, 0x2dc5, 0x0ce4, 0x2dc9, 0x0ce6, 0x2dcd, 0x0ce8, 0x2dd1, 0x0cea, 0x2dd5, 0x0cec, 0x2dd9, 0x0cee, 0x2ddd,
	0x10f0, 0x31e1, 0x10f2, 0x31e5, 0x10f4, 0x31e9, 0x10f6, 0x31ed, 0x20f8, 0x41f1, 0x20fa, 0x41f5, 0x257c, 0x46f9, 0x29be, 0x4b7d
};

/*                   
                                                                  
              
                 
                                                              
                                         
                                  
                
                                                        
 */

static inline int hdlc_bitstuff_byte(struct isowbuf_t *iwb, unsigned char cin,
				     int ones)
{
	u16 stuff;
	int shiftinc, newones;

	/*                                        
                                   
                                                             
                                                        
  */
	stuff = stufftab[256 * ones + cin];
	shiftinc = (stuff >> 13) & 3;
	newones = (stuff >> 10) & 7;
	stuff &= 0x3ff;

	/*                                      */
	isowbuf_putbits(iwb, stuff, 8 + shiftinc);
	return newones;
}

/*                
                                                         
                                                                              
                                                                             
                                                                     
                                                                             
                                  
                                                                             
                                                                              
                                                
                                                                           
                                                            
              
                  
                                        
                                         
                                  
                
                                                         
                                                 
 */

static inline int hdlc_buildframe(struct isowbuf_t *iwb,
				  unsigned char *in, int count)
{
	int ones;
	u16 fcs;
	int end;
	unsigned char c;

	if (isowbuf_freebytes(iwb) < count + count / 5 + 6 ||
	    isowbuf_startwrite(iwb) < 0) {
		gig_dbg(DEBUG_ISO, "%s: %d bytes free -> -EAGAIN",
			__func__, isowbuf_freebytes(iwb));
		return -EAGAIN;
	}

	dump_bytes(DEBUG_STREAM_DUMP, "snd data", in, count);

	/*                                  */
	fcs = PPP_INITFCS;
	ones = 0;
	while (count-- > 0) {
		c = *in++;
		ones = hdlc_bitstuff_byte(iwb, c, ones);
		fcs = crc_ccitt_byte(fcs, c);
	}

	/*                        
                                                 */
	fcs ^= 0xffff;
	ones = hdlc_bitstuff_byte(iwb, fcs & 0x00ff, ones);
	ones = hdlc_bitstuff_byte(iwb, (fcs >> 8) & 0x00ff, ones);

	/*                                                */
	isowbuf_putflag(iwb);
	end = isowbuf_donewrite(iwb);
	return end;
}

/*                 
                                                             
                       
                                                             
                                                            
              
                  
                                        
                                         
                                  
                
                                                         
                                                 
 */

static inline int trans_buildframe(struct isowbuf_t *iwb,
				   unsigned char *in, int count)
{
	int write;
	unsigned char c;

	if (unlikely(count <= 0))
		return iwb->write;

	if (isowbuf_freebytes(iwb) < count ||
	    isowbuf_startwrite(iwb) < 0) {
		gig_dbg(DEBUG_ISO, "can't put %d bytes", count);
		return -EAGAIN;
	}

	gig_dbg(DEBUG_STREAM, "put %d bytes", count);
	dump_bytes(DEBUG_STREAM_DUMP, "snd data", in, count);

	write = iwb->write;
	do {
		c = bitrev8(*in++);
		iwb->data[write++] = c;
		write %= BAS_OUTBUFSIZE;
	} while (--count > 0);
	iwb->write = write;
	iwb->idle = c;

	return isowbuf_donewrite(iwb);
}

int gigaset_isoc_buildframe(struct bc_state *bcs, unsigned char *in, int len)
{
	int result;

	switch (bcs->proto2) {
	case L2_HDLC:
		result = hdlc_buildframe(bcs->hw.bas->isooutbuf, in, len);
		gig_dbg(DEBUG_ISO, "%s: %d bytes HDLC -> %d",
			__func__, len, result);
		break;
	default:			/*                    */
		result = trans_buildframe(bcs->hw.bas->isooutbuf, in, len);
		gig_dbg(DEBUG_ISO, "%s: %d bytes trans -> %d",
			__func__, len, result);
	}
	return result;
}

/*             
                                                                         
 */
static inline void hdlc_putbyte(unsigned char c, struct bc_state *bcs)
{
	bcs->rx_fcs = crc_ccitt_byte(bcs->rx_fcs, c);
	if (bcs->rx_skb == NULL)
		/*          */
		return;
	if (bcs->rx_skb->len >= bcs->rx_bufsize) {
		dev_warn(bcs->cs->dev, "received oversized packet discarded\n");
		bcs->hw.bas->giants++;
		dev_kfree_skb_any(bcs->rx_skb);
		bcs->rx_skb = NULL;
		return;
	}
	*__skb_put(bcs->rx_skb, 1) = c;
}

/*           
                                
 */
static inline void hdlc_flush(struct bc_state *bcs)
{
	/*                                           */
	if (bcs->rx_skb != NULL)
		skb_trim(bcs->rx_skb, 0);
	else
		gigaset_new_rx_skb(bcs);

	/*                    */
	bcs->rx_fcs = PPP_INITFCS;
}

/*          
                                     
 */
static inline void hdlc_done(struct bc_state *bcs)
{
	struct cardstate *cs = bcs->cs;
	struct sk_buff *procskb;
	unsigned int len;

	if (unlikely(bcs->ignore)) {
		bcs->ignore--;
		hdlc_flush(bcs);
		return;
	}
	procskb = bcs->rx_skb;
	if (procskb == NULL) {
		/*                */
		gig_dbg(DEBUG_ISO, "%s: skb=NULL", __func__);
		gigaset_isdn_rcv_err(bcs);
	} else if (procskb->len < 2) {
		dev_notice(cs->dev, "received short frame (%d octets)\n",
			   procskb->len);
		bcs->hw.bas->runts++;
		dev_kfree_skb_any(procskb);
		gigaset_isdn_rcv_err(bcs);
	} else if (bcs->rx_fcs != PPP_GOODFCS) {
		dev_notice(cs->dev, "frame check error\n");
		bcs->hw.bas->fcserrs++;
		dev_kfree_skb_any(procskb);
		gigaset_isdn_rcv_err(bcs);
	} else {
		len = procskb->len;
		__skb_trim(procskb, len -= 2);	/*              */
		gig_dbg(DEBUG_ISO, "%s: good frame (%d octets)", __func__, len);
		dump_bytes(DEBUG_STREAM_DUMP,
			   "rcv data", procskb->data, len);
		bcs->hw.bas->goodbytes += len;
		gigaset_skb_rcvd(bcs, procskb);
	}
	gigaset_new_rx_skb(bcs);
	bcs->rx_fcs = PPP_INITFCS;
}

/*          
                                                    
 */
static inline void hdlc_frag(struct bc_state *bcs, unsigned inbits)
{
	if (unlikely(bcs->ignore)) {
		bcs->ignore--;
		hdlc_flush(bcs);
		return;
	}

	dev_notice(bcs->cs->dev, "received partial byte (%d bits)\n", inbits);
	bcs->hw.bas->alignerrs++;
	gigaset_isdn_rcv_err(bcs);
	__skb_trim(bcs->rx_skb, 0);
	bcs->rx_fcs = PPP_INITFCS;
}

/*                                                
                    
                                                                     
                                                                     
                                                                    
                                                                          
 */
static const unsigned char bitcounts[256] = {
	0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x04,
	0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x05,
	0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x04,
	0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x80, 0x06,
	0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x04,
	0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x05,
	0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x04,
	0x00, 0x01, 0x00, 0x02, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x02, 0x80, 0x81, 0x80, 0x07,
	0x10, 0x11, 0x10, 0x12, 0x10, 0x11, 0x10, 0x13, 0x10, 0x11, 0x10, 0x12, 0x10, 0x11, 0x10, 0x14,
	0x10, 0x11, 0x10, 0x12, 0x10, 0x11, 0x10, 0x13, 0x10, 0x11, 0x10, 0x12, 0x10, 0x11, 0x10, 0x15,
	0x10, 0x11, 0x10, 0x12, 0x10, 0x11, 0x10, 0x13, 0x10, 0x11, 0x10, 0x12, 0x10, 0x11, 0x10, 0x14,
	0x10, 0x11, 0x10, 0x12, 0x10, 0x11, 0x10, 0x13, 0x10, 0x11, 0x10, 0x12, 0x10, 0x11, 0x90, 0x16,
	0x20, 0x21, 0x20, 0x22, 0x20, 0x21, 0x20, 0x23, 0x20, 0x21, 0x20, 0x22, 0x20, 0x21, 0x20, 0x24,
	0x20, 0x21, 0x20, 0x22, 0x20, 0x21, 0x20, 0x23, 0x20, 0x21, 0x20, 0x22, 0x20, 0x21, 0x20, 0x25,
	0x30, 0x31, 0x30, 0x32, 0x30, 0x31, 0x30, 0x33, 0x30, 0x31, 0x30, 0x32, 0x30, 0x31, 0x30, 0x34,
	0x40, 0x41, 0x40, 0x42, 0x40, 0x41, 0x40, 0x43, 0x50, 0x51, 0x50, 0x52, 0x60, 0x61, 0x70, 0x78
};

/*            
                                                                     
                                                                             
                                                                              
                                            
                                                     
              
                    
                                 
                                    
 */
static inline void hdlc_unpack(unsigned char *src, unsigned count,
			       struct bc_state *bcs)
{
	struct bas_bc_state *ubc = bcs->hw.bas;
	int inputstate;
	unsigned seqlen, inbyte, inbits;

	/*                     
                                  
                                                                       
                                        
                                                                        
                         
                                                                       
                   
                                                              
                                                                   
  */
	inputstate = bcs->inputstate;
	seqlen = ubc->seqlen;
	inbyte = ubc->inbyte;
	inbits = ubc->inbits;

	/*                             
                                                                        
                                                                    
                                  
  */
	while (count--) {
		unsigned char c = *src++;
		unsigned char tabentry = bitcounts[c];
		unsigned lead1 = tabentry & 0x0f;
		unsigned trail1 = (tabentry >> 4) & 0x0f;

		seqlen += lead1;

		if (unlikely(inputstate & INS_flag_hunt)) {
			if (c == PPP_FLAG) {
				/*             */
				inputstate &= ~(INS_flag_hunt | INS_have_data);
				inbyte = 0;
				inbits = 0;
			} else if (seqlen == 6 && trail1 != 7) {
				/*                                        */
				inputstate &= ~(INS_flag_hunt | INS_have_data);
				inbyte = c >> (lead1 + 1);
				inbits = 7 - lead1;
				if (trail1 >= 8) {
					/*                   
                                            
                                       
                           */
					inbits--;
					switch (c) {
					case 0xbe:
						inbyte = 0x3f;
						break;
					}
				}
			}
			/*                             */
		} else if (likely(seqlen < 5 && trail1 < 7)) {
			/*                                            */
			inbyte |= c << inbits;
			hdlc_putbyte(inbyte & 0xff, bcs);
			inputstate |= INS_have_data;
			inbyte >>= 8;
			/*                  */
		} else if (likely(seqlen == 6 && inbits == 7 - lead1 &&
				  trail1 + 1 == inbits &&
				  !(inputstate & INS_have_data))) {
			/*                                               */
		} else if (unlikely(seqlen > 6)) {
			/*                */
			ubc->aborts++;
			hdlc_flush(bcs);
			inputstate |= INS_flag_hunt;
		} else if (seqlen == 6) {
			/*                                         
                              */
			if (inbits > 7 - lead1) {
				hdlc_frag(bcs, inbits + lead1 - 7);
				inputstate &= ~INS_have_data;
			} else {
				if (inbits < 7 - lead1)
					ubc->stolen0s++;
				if (inputstate & INS_have_data) {
					hdlc_done(bcs);
					inputstate &= ~INS_have_data;
				}
			}

			if (c == PPP_FLAG) {
				/*                                            */
				ubc->shared0s++;
				inbits = 0;
				inbyte = 0;
			} else if (trail1 != 7) {
				/*                */
				inbyte = c >> (lead1 + 1);
				inbits = 7 - lead1;
				if (trail1 >= 8) {
					/*                   
                                            
                                       
                           */
					inbits--;
					switch (c) {
					case 0xbe:
						inbyte = 0x3f;
						break;
					}
				}
			} else {
				/*                        
                                */
				ubc->aborts++;
				inputstate |= INS_flag_hunt;
			}
		} else { /*                                              */

			if (c == PPP_FLAG) {
				/*               */
				if (seqlen == 5)
					ubc->stolen0s++;
				if (inbits) {
					hdlc_frag(bcs, inbits);
					inbits = 0;
					inbyte = 0;
				} else if (inputstate & INS_have_data)
					hdlc_done(bcs);
				inputstate &= ~INS_have_data;
			} else if (trail1 == 7) {
				/*                */
				ubc->aborts++;
				hdlc_flush(bcs);
				inputstate |= INS_flag_hunt;
			} else {
				/*              */
				if (trail1 < 7) { /*                */
					/*                             
                             */
					unsigned char mask = (1 << lead1) - 1;
					c = (c & mask) | ((c & ~mask) >> 1);
					inbyte |= c << inbits;
					inbits += 7;
				} else if (seqlen < 5) { /*             */
					/*                   
                                            
                                       
                           */
					switch (c) {
					case 0xbe:
						c = 0x7e;
						break;
					}
					inbyte |= c << inbits;
					inbits += 7;
				} else { /*                            */

					/*                                  
                                         */
					switch (c) {
					case 0x7d:
						c = 0x3f;
						break;
					case 0xbe:
						c = 0x3f;
						break;
					case 0x3e:
						c = 0x1f;
						break;
					case 0x7c:
						c = 0x3e;
						break;
					}
					inbyte |= c << inbits;
					inbits += 6;
				}
				if (inbits >= 8) {
					inbits -= 8;
					hdlc_putbyte(inbyte & 0xff, bcs);
					inputstate |= INS_have_data;
					inbyte >>= 8;
				}
			}
		}
		seqlen = trail1 & 7;
	}

	/*                */
	bcs->inputstate = inputstate;
	ubc->seqlen = seqlen;
	ubc->inbyte = inbyte;
	ubc->inbits = inbits;
}

/*              
                                                                       
               
                                                     
              
                    
                                 
                                    
 */
static inline void trans_receive(unsigned char *src, unsigned count,
				 struct bc_state *bcs)
{
	struct sk_buff *skb;
	int dobytes;
	unsigned char *dst;

	if (unlikely(bcs->ignore)) {
		bcs->ignore--;
		return;
	}
	skb = bcs->rx_skb;
	if (skb == NULL) {
		skb = gigaset_new_rx_skb(bcs);
		if (skb == NULL)
			return;
	}
	dobytes = bcs->rx_bufsize - skb->len;
	while (count > 0) {
		dst = skb_put(skb, count < dobytes ? count : dobytes);
		while (count > 0 && dobytes > 0) {
			*dst++ = bitrev8(*src++);
			count--;
			dobytes--;
		}
		if (dobytes == 0) {
			dump_bytes(DEBUG_STREAM_DUMP,
				   "rcv data", skb->data, skb->len);
			bcs->hw.bas->goodbytes += skb->len;
			gigaset_skb_rcvd(bcs, skb);
			skb = gigaset_new_rx_skb(bcs);
			if (skb == NULL)
				return;
			dobytes = bcs->rx_bufsize;
		}
	}
}

void gigaset_isoc_receive(unsigned char *src, unsigned count,
			  struct bc_state *bcs)
{
	switch (bcs->proto2) {
	case L2_HDLC:
		hdlc_unpack(src, count, bcs);
		break;
	default:		/*                    */
		trans_receive(src, count, bcs);
	}
}

/*                                                                           */

/*                                                                        
                                                                        
                                        
                                                                            
                                                           
 */
static void cmd_loop(unsigned char *src, int numbytes, struct inbuf_t *inbuf)
{
	struct cardstate *cs = inbuf->cs;
	unsigned cbytes      = cs->cbytes;
	unsigned char c;

	while (numbytes--) {
		c = *src++;
		switch (c) {
		case '\n':
			if (cbytes == 0 && cs->respdata[0] == '\r') {
				/*                               */
				cs->respdata[0] = 0;
				break;
			}
			/*                          */
		case '\r':
			/*                                               */
			if (cbytes >= MAX_RESP_SIZE) {
				dev_warn(cs->dev, "response too large (%d)\n",
					 cbytes);
				cbytes = MAX_RESP_SIZE;
			}
			cs->cbytes = cbytes;
			gigaset_dbg_buffer(DEBUG_TRANSCMD, "received response",
					   cbytes, cs->respdata);
			gigaset_handle_modem_response(cs);
			cbytes = 0;

			/*                                    */
			cs->respdata[0] = c;
			break;
		default:
			/*                                   */
			if (cbytes < MAX_RESP_SIZE)
				cs->respdata[cbytes] = c;
			cbytes++;
		}
	}

	/*            */
	cs->cbytes = cbytes;
}


/*                                                             
 */
void gigaset_isoc_input(struct inbuf_t *inbuf)
{
	struct cardstate *cs = inbuf->cs;
	unsigned tail, head, numbytes;
	unsigned char *src;

	head = inbuf->head;
	while (head != (tail = inbuf->tail)) {
		gig_dbg(DEBUG_INTR, "buffer state: %u -> %u", head, tail);
		if (head > tail)
			tail = RBUFSIZE;
		src = inbuf->data + head;
		numbytes = tail - head;
		gig_dbg(DEBUG_INTR, "processing %u bytes", numbytes);

		if (cs->mstate == MS_LOCKED) {
			gigaset_dbg_buffer(DEBUG_LOCKCMD, "received response",
					   numbytes, src);
			gigaset_if_receive(inbuf->cs, src, numbytes);
		} else {
			cmd_loop(src, numbytes, inbuf);
		}

		head += numbytes;
		if (head == RBUFSIZE)
			head = 0;
		gig_dbg(DEBUG_INTR, "setting head to %u", head);
		inbuf->head = head;
	}
}


/*                                                                           */

/* 
                                                     
                                        
                      
  
                                                                      
             
                                                                            
                                                             
  
                
                                                         
                                        
 */
int gigaset_isoc_send_skb(struct bc_state *bcs, struct sk_buff *skb)
{
	int len = skb->len;
	unsigned long flags;

	spin_lock_irqsave(&bcs->cs->lock, flags);
	if (!bcs->cs->connected) {
		spin_unlock_irqrestore(&bcs->cs->lock, flags);
		return -ENODEV;
	}

	skb_queue_tail(&bcs->squeue, skb);
	gig_dbg(DEBUG_ISO, "%s: skb queued, qlen=%d",
		__func__, skb_queue_len(&bcs->squeue));

	/*                                  */
	tasklet_schedule(&bcs->hw.bas->sent_tasklet);
	spin_unlock_irqrestore(&bcs->cs->lock, flags);

	return len;	/*           */
}
