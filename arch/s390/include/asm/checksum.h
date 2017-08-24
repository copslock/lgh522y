/*
 *    S390 fast network checksum routines
 *
 *  S390 version
 *    Copyright IBM Corp. 1999
 *    Author(s): Ulrich Hild        (first version)
 *               Martin Schwidefsky (heavily optimized CKSM version)
 *               D.J. Barrow        (third attempt) 
 */

#ifndef _S390_CHECKSUM_H
#define _S390_CHECKSUM_H

#include <asm/uaccess.h>

/*
                                                               
                             
  
                                                           
                       
  
                                                         
                                          
  
                                                      
 */
static inline __wsum
csum_partial(const void *buff, int len, __wsum sum)
{
	register unsigned long reg2 asm("2") = (unsigned long) buff;
	register unsigned long reg3 asm("3") = (unsigned long) len;

	asm volatile(
		"0:	cksm	%0,%1\n"	/*                      */
		"	jo	0b\n"
		: "+d" (sum), "+d" (reg2), "+d" (reg3) : : "cc", "memory");
	return sum;
}

/*
                                                             
  
                                                                     
                          
  
                                                                      
                                    
 */
static inline __wsum
csum_partial_copy_from_user(const void __user *src, void *dst,
                                          int len, __wsum sum,
                                          int *err_ptr)
{
	int missing;

	missing = copy_from_user(dst, src, len);
	if (missing) {
		memset(dst + len - missing, 0, missing);
		*err_ptr = -EFAULT;
	}
		
	return csum_partial(dst, len, sum);
}


static inline __wsum
csum_partial_copy_nocheck (const void *src, void *dst, int len, __wsum sum)
{
        memcpy(dst,src,len);
	return csum_partial(dst, len, sum);
}

/*
                                                             
 */
static inline __sum16 csum_fold(__wsum sum)
{
	u32 csum = (__force u32) sum;

	csum += (csum >> 16) + (csum << 16);
	csum >>= 16;
	return (__force __sum16) ~csum;
}

/*
                                                                   
                                               
  
 */
static inline __sum16 ip_fast_csum(const void *iph, unsigned int ihl)
{
	return csum_fold(csum_partial(iph, ihl*4, 0));
}

/*
                                                     
                            
 */
static inline __wsum
csum_tcpudp_nofold(__be32 saddr, __be32 daddr,
                   unsigned short len, unsigned short proto,
                   __wsum sum)
{
	__u32 csum = (__force __u32)sum;

	csum += (__force __u32)saddr;
	if (csum < (__force __u32)saddr)
		csum++;

	csum += (__force __u32)daddr;
	if (csum < (__force __u32)daddr)
		csum++;

	csum += len + proto;
	if (csum < len + proto)
		csum++;

	return (__force __wsum)csum;
}

/*
                                                     
                                                  
 */

static inline __sum16
csum_tcpudp_magic(__be32 saddr, __be32 daddr,
                  unsigned short len, unsigned short proto,
                  __wsum sum)
{
	return csum_fold(csum_tcpudp_nofold(saddr,daddr,len,proto,sum));
}

/*
                                                                   
            
 */

static inline __sum16 ip_compute_csum(const void *buff, int len)
{
	return csum_fold(csum_partial(buff, len, 0));
}

#endif /*                  */


