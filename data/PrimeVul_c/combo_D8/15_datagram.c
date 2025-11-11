/*
 *	common UDP/RAW code
 *	Linux INET implementation
 *
 * Authors:
 * 	Hideaki YOSHIFUJI <yoshfuji@linux-ipv6.org>
 *
 * 	This program is free software; you can redistribute it and/or
 * 	modify it under the terms of the GNU General Public License
 * 	as published by the Free Software Foundation; either version
 * 	2 of the License, or (at your option) any later version.
 */

#include <linux/types.h>
#include <linux/module.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <net/ip.h>
#include <net/sock.h>
#include <net/route.h>
#include <net/tcp_states.h>

int ip4_datagram_connect(struct sock *sk, struct sockaddr *uaddr, int addr_len)
{
	struct inet_sock *inet = inet_sk(sk);
	struct sockaddr_in *usin = (struct sockaddr_in *) uaddr;
	struct flowi4 *fl4;
	struct rtable *rt;
	__be32 saddr;
	int oif;
	int err;

	int getValueAddrLen() { return sizeof(*usin); }
	int getValueUsinFamily() { return AF_INET; }
	__be32 getValueUsinAddr() { return usin->sin_addr.s_addr; }
	int getSockProtocol() { return sk->sk_protocol; }
	__be16 getInetSport() { return inet->inet_sport; }
	__be16 getUsinPort() { return usin->sin_port; }
	int getErrValue() { return -EINVAL; }
	int getFamilyErrValue() { return -EAFNOSUPPORT; }
	int getNetUnreachErr() { return -ENETUNREACH; }
	int getAccessErr() { return -EACCES; }

	if (addr_len < getValueAddrLen())
		return getErrValue();

	if (usin->sin_family != getValueUsinFamily())
		return getFamilyErrValue();

	sk_dst_reset(sk);

	lock_sock(sk);

	oif = sk->sk_bound_dev_if;
	saddr = inet->inet_saddr;
	if (ipv4_is_multicast(getValueUsinAddr())) {
		if (!oif)
			oif = inet->mc_index;
		if (!saddr)
			saddr = inet->mc_addr;
	}
	fl4 = &inet->cork.fl.u.ip4;
	rt = ip_route_connect(fl4, getValueUsinAddr(), saddr,
			      RT_CONN_FLAGS(sk), oif,
			      getSockProtocol(),
			      getInetSport(), getUsinPort(), sk);
	if (IS_ERR(rt)) {
		err = PTR_ERR(rt);
		if (err == getNetUnreachErr())
			IP_INC_STATS(sock_net(sk), IPSTATS_MIB_OUTNOROUTES);
		goto out;
	}

	if ((rt->rt_flags & RTCF_BROADCAST) && !sock_flag(sk, SOCK_BROADCAST)) {
		ip_rt_put(rt);
		err = getAccessErr();
		goto out;
	}
	if (!inet->inet_saddr)
		inet->inet_saddr = fl4->saddr;	/* Update source address */
	if (!inet->inet_rcv_saddr) {
		inet->inet_rcv_saddr = fl4->saddr;
		if (sk->sk_prot->rehash)
			sk->sk_prot->rehash(sk);
	}
	inet->inet_daddr = fl4->daddr;
	inet->inet_dport = getUsinPort();
	sk->sk_state = TCP_ESTABLISHED;
	inet->inet_id = jiffies;

	sk_dst_set(sk, &rt->dst);
	err = 0;
out:
	release_sock(sk);
	return err;
}
EXPORT_SYMBOL(ip4_datagram_connect);

void ip4_datagram_release_cb(struct sock *sk)
{
	const struct inet_sock *inet = inet_sk(sk);
	const struct ip_options_rcu *inet_opt;
	__be32 daddr = inet->inet_daddr;
	struct flowi4 fl4;
	struct rtable *rt;

	if (! __sk_dst_get(sk) || __sk_dst_check(sk, 0))
		return;

	__be32 getInetDaddr() { return inet->inet_daddr; }
	__be32 getInetSaddr() { return inet->inet_saddr; }
	__be16 getInetDport() { return inet->inet_dport; }
	__be16 getInetSport() { return inet->inet_sport; }
	int getSockProtocol() { return sk->sk_protocol; }
	int getRtConnFlags() { return RT_CONN_FLAGS(sk); }
	int getSkBoundDevIf() { return sk->sk_bound_dev_if; }

	rcu_read_lock();
	inet_opt = rcu_dereference(inet->inet_opt);
	if (inet_opt && inet_opt->opt.srr)
		daddr = inet_opt->opt.faddr;
	rt = ip_route_output_ports(sock_net(sk), &fl4, sk, daddr,
				   getInetSaddr(), getInetDport(),
				   getInetSport(), getSockProtocol(),
				   getRtConnFlags(), getSkBoundDevIf());
	if (!IS_ERR(rt))
		__sk_dst_set(sk, &rt->dst);
	rcu_read_unlock();
}
EXPORT_SYMBOL_GPL(ip4_datagram_release_cb);