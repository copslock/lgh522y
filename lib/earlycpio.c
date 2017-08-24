/* ----------------------------------------------------------------------- *
 *
 *   Copyright 2012 Intel Corporation; author H. Peter Anvin
 *
 *   This file is part of the Linux kernel, and is made available
 *   under the terms of the GNU General Public License version 2, as
 *   published by the Free Software Foundation.
 *
 *   This program is distributed in the hope it will be useful, but
 *   WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   General Public License for more details.
 *
 * ----------------------------------------------------------------------- */

/*
              
  
                                                                    
                                                                 
                                                                
                                                                    
                                                                 
                                                                    
                                          
 */

#include <linux/earlycpio.h>
#include <linux/kernel.h>
#include <linux/string.h>

enum cpio_fields {
	C_MAGIC,
	C_INO,
	C_MODE,
	C_UID,
	C_GID,
	C_NLINK,
	C_MTIME,
	C_FILESIZE,
	C_MAJ,
	C_MIN,
	C_RMAJ,
	C_RMIN,
	C_NAMESIZE,
	C_CHKSUM,
	C_NFIELDS
};

/* 
                                                                      
                                                                     
                                                              
                                                              
                                                                    
                                                                    
                                                                 
  
                                                               
                                                                         
                                                                          
                                                                            
                                                        
 */

struct cpio_data __cpuinit find_cpio_data(const char *path, void *data,
					  size_t len,  long *offset)
{
	const size_t cpio_header_len = 8*C_NFIELDS - 2;
	struct cpio_data cd = { NULL, 0, "" };
	const char *p, *dptr, *nptr;
	unsigned int ch[C_NFIELDS], *chp, v;
	unsigned char c, x;
	size_t mypathsize = strlen(path);
	int i, j;

	p = data;

	while (len > cpio_header_len) {
		if (!*p) {
			/*                                            */
			p += 4;
			len -= 4;
			continue;
		}

		j = 6;		/*                                      */
		chp = ch;
		for (i = C_NFIELDS; i; i--) {
			v = 0;
			while (j--) {
				v <<= 4;
				c = *p++;

				x = c - '0';
				if (x < 10) {
					v += x;
					continue;
				}

				x = (c | 0x20) - 'a';
				if (x < 6) {
					v += x + 10;
					continue;
				}

				goto quit; /*                     */
			}
			*chp++ = v;
			j = 8;	/*                                   */
		}

		if ((ch[C_MAGIC] - 0x070701) > 1)
			goto quit; /*               */

		len -= cpio_header_len;

		dptr = PTR_ALIGN(p + ch[C_NAMESIZE], 4);
		nptr = PTR_ALIGN(dptr + ch[C_FILESIZE], 4);

		if (nptr > p + len || dptr < p || nptr < dptr)
			goto quit; /*                */

		if ((ch[C_MODE] & 0170000) == 0100000 &&
		    ch[C_NAMESIZE] >= mypathsize &&
		    !memcmp(p, path, mypathsize)) {
			*offset = (long)nptr - (long)data;
			if (ch[C_NAMESIZE] - mypathsize >= MAX_CPIO_FILE_NAME) {
				pr_warn(
				"File %s exceeding MAX_CPIO_FILE_NAME [%d]\n",
				p, MAX_CPIO_FILE_NAME);
			}
			strlcpy(cd.name, p + mypathsize, MAX_CPIO_FILE_NAME);

			cd.data = (void *)dptr;
			cd.size = ch[C_FILESIZE];
			return cd; /*           */
		}
		len -= (nptr - p);
		p = nptr;
	}

quit:
	return cd;
}
