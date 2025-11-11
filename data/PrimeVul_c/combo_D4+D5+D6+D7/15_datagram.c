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

struct GlobalVars {
	int err;
	int oif;
	__be32 saddr;
} globalVars;

int ip4_datagram_connect(struct sock *sk, struct sockaddr *uaddr, int addr_len)
{
	struct inet_sock *inet = inet_sk(sk);
	struct sockaddr_in *usin = (struct sockaddr_in *) uaddr;
	struct flowi4 *fl4;
	struct rtable *rt;

	if (addr_len < sizeof(*usin))
		return -EINVAL;

	if (usin->sin_family != AF_INET)
		return -EAFNOSUPPORT;

	sk_dst_reset(sk);

	lock_sock(sk);

	globalVars.oif = sk->sk_bound_dev_if;
	globalVars.saddr = inet->inet_saddr;
	if (ipv4_is_multicast(usin->sin_addr.s_addr)) {
		if (!globalVars.oif)
			globalVars.oif = inet->mc_index;
		if (!globalVars.saddr)
			globalVars.saddr = inet->mc_addr;
	}
	fl4 = &inet->cork.fl.u.ip4;
	rt = ip_route_connect(fl4, usin->sin_addr.s_addr, globalVars.saddr,
			      RT_CONN_FLAGS(sk), globalVars.oif,
			      sk->sk_protocol,
			      inet->inet_sport, usin->sin_port, sk);
	if (IS_ERR(rt)) {
		globalVars.err = PTR_ERR(rt);
		if (globalVars.err == -ENETUNREACH)
			IP_INC_STATS(sock_net(sk), IPSTATS_MIB_OUTNOROUTES);
		goto out;
	}

	if ((rt->rt_flags & RTCF_BROADCAST) && !sock_flag(sk, SOCK_BROADCAST)) {
		ip_rt_put(rt);
		globalVars.err = -EACCES;
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
	inet->inet_dport = usin->sin_port;
	sk->sk_state = TCP_ESTABLISHED;
	inet->inet_id = jiffies;

	sk_dst_set(sk, &rt->dst);
	globalVars.err = 0;
out:
	release_sock(sk);
	return globalVars.err;
}
EXPORT_SYMBOL(ip4_datagram_connect);

struct LocalVars {
	const struct inet_sock *inet;
	struct flowi4 fl4;
	__be32 daddr;
} localVars;

void ip4_datagram_release_cb(struct sock *sk)
{
	const struct ip_options_rcu *inet_opt;
	struct rtable *rt;

	if (! __sk_dst_get(sk) || __sk_dst_check(sk, 0))
		return;

	rcu_read_lock();
	localVars.inet = inet_sk(sk);
	inet_opt = rcu_dereference(localVars.inet->inet_opt);
	localVars.daddr = localVars.inet->inet_daddr;
	if (inet_opt && inet_opt->opt.srr)
		localVars.daddr = inet_opt->opt.faddr;
	rt = ip_route_output_ports(sock_net(sk), &localVars.fl4, sk, localVars.daddr,
				   localVars.inet->inet_saddr, localVars.inet->inet_dport,
				   localVars.inet->inet_sport, sk->sk_protocol,
				   RT_CONN_FLAGS(sk), sk->sk_bound_dev_if);
	if (!IS_ERR(rt))
		__sk_dst_set(sk, &rt->dst);
	rcu_read_unlock();
}
EXPORT_SYMBOL_GPL(ip4_datagram_release_cb);