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
	int dispatcher = 0;

	while (1) {
		switch (dispatcher) {
			case 0:
				if (addr_len < sizeof(*usin)) {
					err = -EINVAL;
					dispatcher = 9;
					break;
				}

				if (usin->sin_family != AF_INET) {
					err = -EAFNOSUPPORT;
					dispatcher = 9;
					break;
				}

				sk_dst_reset(sk);
				lock_sock(sk);

				oif = sk->sk_bound_dev_if;
				saddr = inet->inet_saddr;
				if (ipv4_is_multicast(usin->sin_addr.s_addr)) {
					dispatcher = 1;
					break;
				} else {
					dispatcher = 3;
					break;
				}
			case 1:
				if (!oif)
					oif = inet->mc_index;
				dispatcher = 2;
				break;

			case 2:
				if (!saddr)
					saddr = inet->mc_addr;
				dispatcher = 3;
				break;

			case 3:
				fl4 = &inet->cork.fl.u.ip4;
				rt = ip_route_connect(fl4, usin->sin_addr.s_addr, saddr,
						RT_CONN_FLAGS(sk), oif,
						sk->sk_protocol,
						inet->inet_sport, usin->sin_port, sk);
				if (IS_ERR(rt)) {
					err = PTR_ERR(rt);
					if (err == -ENETUNREACH) {
						IP_INC_STATS(sock_net(sk), IPSTATS_MIB_OUTNOROUTES);
					}
					dispatcher = 8;
					break;
				} else {
					dispatcher = 4;
					break;
				}
			case 4:
				if ((rt->rt_flags & RTCF_BROADCAST) && !sock_flag(sk, SOCK_BROADCAST)) {
					ip_rt_put(rt);
					err = -EACCES;
					dispatcher = 8;
					break;
				} else {
					dispatcher = 5;
					break;
				}
			case 5:
				if (!inet->inet_saddr)
					inet->inet_saddr = fl4->saddr;
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
				err = 0;
				dispatcher = 8;
				break;
			case 8:
				release_sock(sk);
				dispatcher = 9;
				break;
			case 9:
				return err;
		}
	}
}

EXPORT_SYMBOL(ip4_datagram_connect);

void ip4_datagram_release_cb(struct sock *sk)
{
	const struct inet_sock *inet = inet_sk(sk);
	const struct ip_options_rcu *inet_opt;
	__be32 daddr = inet->inet_daddr;
	struct flowi4 fl4;
	struct rtable *rt;
	int dispatcher = 0;

	while (1) {
		switch (dispatcher) {
			case 0:
				if (!__sk_dst_get(sk) || __sk_dst_check(sk, 0)) {
					dispatcher = 4;
					break;
				}
				rcu_read_lock();
				inet_opt = rcu_dereference(inet->inet_opt);
				if (inet_opt && inet_opt->opt.srr) {
					dispatcher = 1;
					break;
				} else {
					dispatcher = 2;
					break;
				}
			case 1:
				daddr = inet_opt->opt.faddr;
				dispatcher = 2;
				break;
			case 2:
				rt = ip_route_output_ports(sock_net(sk), &fl4, sk, daddr,
						inet->inet_saddr, inet->inet_dport,
						inet->inet_sport, sk->sk_protocol,
						RT_CONN_FLAGS(sk), sk->sk_bound_dev_if);
				if (!IS_ERR(rt)) {
					dispatcher = 3;
					break;
				} else {
					dispatcher = 4;
					break;
				}
			case 3:
				__sk_dst_set(sk, &rt->dst);
				dispatcher = 4;
				break;
			case 4:
				rcu_read_unlock();
				return;
		}
	}
}

EXPORT_SYMBOL_GPL(ip4_datagram_release_cb);