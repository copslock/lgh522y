#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/cryptohash.h>
#include <linux/module.h>
#include <linux/cache.h>
#include <linux/random.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/string.h>

#include <net/secure_seq.h>

#if IS_ENABLED(CONFIG_IPV6) || IS_ENABLED(CONFIG_INET)
#define NET_SECRET_SIZE (MD5_MESSAGE_BYTES / 4)

static u32 net_secret[NET_SECRET_SIZE] ____cacheline_aligned;

static void net_secret_init(void)
{
	u32 tmp;
	int i;

	if (likely(net_secret[0]))
		return;

	for (i = NET_SECRET_SIZE; i > 0;) {
		do {
			get_random_bytes(&tmp, sizeof(tmp));
		} while (!tmp);
		cmpxchg(&net_secret[--i], 0, tmp);
	}
}
#endif

#ifdef CONFIG_INET
static u32 seq_scale(u32 seq)
{
	/*
                                          
                                   
                                                       
                                                       
                                                         
                                                            
                                                    
                                                             
  */
	return seq + (ktime_to_ns(ktime_get_real()) >> 6);
}
#endif

#if IS_ENABLED(CONFIG_IPV6)
__u32 secure_tcpv6_sequence_number(const __be32 *saddr, const __be32 *daddr,
				   __be16 sport, __be16 dport)
{
	u32 secret[MD5_MESSAGE_BYTES / 4];
	u32 hash[MD5_DIGEST_WORDS];
	u32 i;

	net_secret_init();
	memcpy(hash, saddr, 16);
	for (i = 0; i < 4; i++)
		secret[i] = net_secret[i] + (__force u32)daddr[i];
	secret[4] = net_secret[4] +
		(((__force u16)sport << 16) + (__force u16)dport);
	for (i = 5; i < MD5_MESSAGE_BYTES / 4; i++)
		secret[i] = net_secret[i];

	md5_transform(hash, secret);

	return seq_scale(hash[0]);
}
EXPORT_SYMBOL(secure_tcpv6_sequence_number);

u32 secure_ipv6_port_ephemeral(const __be32 *saddr, const __be32 *daddr,
			       __be16 dport)
{
	u32 secret[MD5_MESSAGE_BYTES / 4];
	u32 hash[MD5_DIGEST_WORDS];
	u32 i;

	net_secret_init();
	memcpy(hash, saddr, 16);
	for (i = 0; i < 4; i++)
		secret[i] = net_secret[i] + (__force u32) daddr[i];
	secret[4] = net_secret[4] + (__force u32)dport;
	for (i = 5; i < MD5_MESSAGE_BYTES / 4; i++)
		secret[i] = net_secret[i];

	md5_transform(hash, secret);

	return hash[0];
}
EXPORT_SYMBOL(secure_ipv6_port_ephemeral);
#endif

#ifdef CONFIG_INET

__u32 secure_tcp_sequence_number(__be32 saddr, __be32 daddr,
				 __be16 sport, __be16 dport)
{
	u32 hash[MD5_DIGEST_WORDS];

	net_secret_init();
	hash[0] = (__force u32)saddr;
	hash[1] = (__force u32)daddr;
	hash[2] = ((__force u16)sport << 16) + (__force u16)dport;
	hash[3] = net_secret[15];

	md5_transform(hash, net_secret);

	return seq_scale(hash[0]);
}

u32 secure_ipv4_port_ephemeral(__be32 saddr, __be32 daddr, __be16 dport)
{
	u32 hash[MD5_DIGEST_WORDS];

	net_secret_init();
	hash[0] = (__force u32)saddr;
	hash[1] = (__force u32)daddr;
	hash[2] = (__force u32)dport ^ net_secret[14];
	hash[3] = net_secret[15];

	md5_transform(hash, net_secret);

	return hash[0];
}
EXPORT_SYMBOL_GPL(secure_ipv4_port_ephemeral);
#endif

#if IS_ENABLED(CONFIG_IP_DCCP)
u64 secure_dccp_sequence_number(__be32 saddr, __be32 daddr,
				__be16 sport, __be16 dport)
{
	u32 hash[MD5_DIGEST_WORDS];
	u64 seq;

	net_secret_init();
	hash[0] = (__force u32)saddr;
	hash[1] = (__force u32)daddr;
	hash[2] = ((__force u16)sport << 16) + (__force u16)dport;
	hash[3] = net_secret[15];

	md5_transform(hash, net_secret);

	seq = hash[0] | (((u64)hash[1]) << 32);
	seq += ktime_to_ns(ktime_get_real());
	seq &= (1ull << 48) - 1;

	return seq;
}
EXPORT_SYMBOL(secure_dccp_sequence_number);

#if IS_ENABLED(CONFIG_IPV6)
u64 secure_dccpv6_sequence_number(__be32 *saddr, __be32 *daddr,
				  __be16 sport, __be16 dport)
{
	u32 secret[MD5_MESSAGE_BYTES / 4];
	u32 hash[MD5_DIGEST_WORDS];
	u64 seq;
	u32 i;

	net_secret_init();
	memcpy(hash, saddr, 16);
	for (i = 0; i < 4; i++)
		secret[i] = net_secret[i] + daddr[i];
	secret[4] = net_secret[4] +
		(((__force u16)sport << 16) + (__force u16)dport);
	for (i = 5; i < MD5_MESSAGE_BYTES / 4; i++)
		secret[i] = net_secret[i];

	md5_transform(hash, secret);

	seq = hash[0] | (((u64)hash[1]) << 32);
	seq += ktime_to_ns(ktime_get_real());
	seq &= (1ull << 48) - 1;

	return seq;
}
EXPORT_SYMBOL(secure_dccpv6_sequence_number);
#endif
#endif
