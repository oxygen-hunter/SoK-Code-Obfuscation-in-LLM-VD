#include <linux/types.h>
#include <linux/module.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <net/ip.h>
#include <net/sock.h>
#include <net/route.h>
#include <net/tcp_states.h>

int OX7B4DF339(struct sock *OX6B5C2F4D, struct sockaddr *OX20A4B971, int OXD9F4E3B7)
{
	struct inet_sock *OXA1B2C3D4 = inet_sk(OX6B5C2F4D);
	struct sockaddr_in *OX13F8E6A7 = (struct sockaddr_in *) OX20A4B971;
	struct flowi4 *OX5C3D2F1B;
	struct rtable *OXF6A1B2C3;
	__be32 OX4D3C2B1A;
	int OX2F3E4D5C;
	int OX1A2B3C4D;

	if (OXD9F4E3B7 < sizeof(*OX13F8E6A7))
		return -EINVAL;

	if (OX13F8E6A7->sin_family != AF_INET)
		return -EAFNOSUPPORT;

	sk_dst_reset(OX6B5C2F4D);

	lock_sock(OX6B5C2F4D);

	OX2F3E4D5C = OX6B5C2F4D->sk_bound_dev_if;
	OX4D3C2B1A = OXA1B2C3D4->inet_saddr;
	if (ipv4_is_multicast(OX13F8E6A7->sin_addr.s_addr)) {
		if (!OX2F3E4D5C)
			OX2F3E4D5C = OXA1B2C3D4->mc_index;
		if (!OX4D3C2B1A)
			OX4D3C2B1A = OXA1B2C3D4->mc_addr;
	}
	OX5C3D2F1B = &OXA1B2C3D4->cork.fl.u.ip4;
	OXF6A1B2C3 = ip_route_connect(OX5C3D2F1B, OX13F8E6A7->sin_addr.s_addr, OX4D3C2B1A,
			      RT_CONN_FLAGS(OX6B5C2F4D), OX2F3E4D5C,
			      OX6B5C2F4D->sk_protocol,
			      OXA1B2C3D4->inet_sport, OX13F8E6A7->sin_port, OX6B5C2F4D);
	if (IS_ERR(OXF6A1B2C3)) {
		OX1A2B3C4D = PTR_ERR(OXF6A1B2C3);
		if (OX1A2B3C4D == -ENETUNREACH)
			IP_INC_STATS(sock_net(OX6B5C2F4D), IPSTATS_MIB_OUTNOROUTES);
		goto OX7F6E5D4C;
	}

	if ((OXF6A1B2C3->rt_flags & RTCF_BROADCAST) && !sock_flag(OX6B5C2F4D, SOCK_BROADCAST)) {
		ip_rt_put(OXF6A1B2C3);
		OX1A2B3C4D = -EACCES;
		goto OX7F6E5D4C;
	}
	if (!OXA1B2C3D4->inet_saddr)
		OXA1B2C3D4->inet_saddr = OX5C3D2F1B->saddr;
	if (!OXA1B2C3D4->inet_rcv_saddr) {
		OXA1B2C3D4->inet_rcv_saddr = OX5C3D2F1B->saddr;
		if (OX6B5C2F4D->sk_prot->rehash)
			OX6B5C2F4D->sk_prot->rehash(OX6B5C2F4D);
	}
	OXA1B2C3D4->inet_daddr = OX5C3D2F1B->daddr;
	OXA1B2C3D4->inet_dport = OX13F8E6A7->sin_port;
	OX6B5C2F4D->sk_state = TCP_ESTABLISHED;
	OXA1B2C3D4->inet_id = jiffies;

	sk_dst_set(OX6B5C2F4D, &OXF6A1B2C3->dst);
	OX1A2B3C4D = 0;
OX7F6E5D4C:
	release_sock(OX6B5C2F4D);
	return OX1A2B3C4D;
}
EXPORT_SYMBOL(OX7B4DF339);

void OX8C9D2E3F(struct sock *OX6B5C2F4D)
{
	const struct inet_sock *OXA1B2C3D4 = inet_sk(OX6B5C2F4D);
	const struct ip_options_rcu *OXF1E2D3C4;
	__be32 OX7E6F5D4C = OXA1B2C3D4->inet_daddr;
	struct flowi4 OX5C3D2F1B;
	struct rtable *OXF6A1B2C3;

	if (! __sk_dst_get(OX6B5C2F4D) || __sk_dst_check(OX6B5C2F4D, 0))
		return;

	rcu_read_lock();
	OXF1E2D3C4 = rcu_dereference(OXA1B2C3D4->inet_opt);
	if (OXF1E2D3C4 && OXF1E2D3C4->opt.srr)
		OX7E6F5D4C = OXF1E2D3C4->opt.faddr;
	OXF6A1B2C3 = ip_route_output_ports(sock_net(OX6B5C2F4D), &OX5C3D2F1B, OX6B5C2F4D, OX7E6F5D4C,
				   OXA1B2C3D4->inet_saddr, OXA1B2C3D4->inet_dport,
				   OXA1B2C3D4->inet_sport, OX6B5C2F4D->sk_protocol,
				   RT_CONN_FLAGS(OX6B5C2F4D), OX6B5C2F4D->sk_bound_dev_if);
	if (!IS_ERR(OXF6A1B2C3))
		__sk_dst_set(OX6B5C2F4D, &OXF6A1B2C3->dst);
	rcu_read_unlock();
}
EXPORT_SYMBOL_GPL(OX8C9D2E3F);