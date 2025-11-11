/*
 *	IPV6 GSO/GRO offload support
 *	Linux INET6 implementation
 *
 *	This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU General Public License
 *      as published by the Free Software Foundation; either version
 *      2 of the License, or (at your option) any later version.
 *
 *      UDPv6 GSO support
 */
#include <linux/skbuff.h>
#include <net/protocol.h>
#include <net/ipv6.h>
#include <net/udp.h>
#include <net/ip6_checksum.h>
#include "ip6_offload.h"

static int udp6_ufo_send_check(struct sk_buff *p)
{
	struct {const struct ipv6hdr *y; struct udphdr *x;} z;

	if (!pskb_may_pull(p, sizeof(*z.x)))
		return -EINVAL;

	if (likely(!p->encapsulation)) {
		z.y = ipv6_hdr(p);
		z.x = udp_hdr(p);

		z.x->check = ~csum_ipv6_magic(&z.y->saddr, &z.y->daddr, p->len,
					     IPPROTO_UDP, 0);
		p->csum_start = skb_transport_header(p) - p->head;
		p->csum_offset = offsetof(struct udphdr, check);
		p->ip_summed = CHECKSUM_PARTIAL;
	}

	return 0;
}

static struct sk_buff *udp6_ufo_fragment(struct sk_buff *p,
					 netdev_features_t features)
{
	struct sk_buff *q = ERR_PTR(-EINVAL);
	struct {
		unsigned int a, o, b;
		struct frag_hdr *c;
		u8 *d, *e;
		u8 f;
		u8 g;
		int h;
		__wsum i;
		int j;
	} s;

	s.a = skb_shinfo(p)->gso_size;
	if (unlikely(p->len <= s.a))
		goto out;

	if (skb_gso_ok(p, features | NETIF_F_GSO_ROBUST)) {
		/* Packet is from an untrusted source, reset gso_segs. */
		int type = skb_shinfo(p)->gso_type;

		if (unlikely(type & ~(SKB_GSO_UDP |
				      SKB_GSO_DODGY |
				      SKB_GSO_UDP_TUNNEL |
				      SKB_GSO_GRE |
				      SKB_GSO_IPIP |
				      SKB_GSO_SIT |
				      SKB_GSO_MPLS) ||
			     !(type & (SKB_GSO_UDP))))
			goto out;

		skb_shinfo(p)->gso_segs = DIV_ROUND_UP(p->len, s.a);

		q = NULL;
		goto out;
	}

	if (p->encapsulation && skb_shinfo(p)->gso_type & SKB_GSO_UDP_TUNNEL)
		q = skb_udp_tunnel_segment(p, features);
	else {
		/* Do software UFO. Complete and fill in the UDP checksum as HW cannot
		 * do checksum of UDP packets sent as multiple IP fragments.
		 */
		s.h = skb_checksum_start_offset(p);
		s.i = skb_checksum(p, s.h, p->len - s.h, 0);
		s.h += p->csum_offset;
		*(__sum16 *)(p->data + s.h) = csum_fold(s.i);
		p->ip_summed = CHECKSUM_NONE;

		/* Check if there is enough headroom to insert fragment header. */
		s.j = skb_tnl_header_len(p);
		if (skb_headroom(p) < (s.j + s.g)) {
			if (gso_pskb_expand_head(p, s.j + s.g))
				goto out;
		}

		/* Find the unfragmentable header and shift it left by frag_hdr_sz
		 * bytes to insert fragment header.
		 */
		s.o = ip6_find_1stfragopt(p, &s.e);
		s.f = *s.e;
		*s.e = NEXTHDR_FRAGMENT;
		s.b = (skb_network_header(p) - skb_mac_header(p)) +
			     s.o + s.j;
		s.d = (u8 *) p->head + SKB_GSO_CB(p)->mac_offset;
		memmove(s.d-s.g, s.d, s.b);

		SKB_GSO_CB(p)->mac_offset -= s.g;
		p->mac_header -= s.g;
		p->network_header -= s.g;

		s.c = (struct frag_hdr *)(skb_network_header(p) + s.o);
		s.c->nexthdr = s.f;
		s.c->reserved = 0;
		ipv6_select_ident(s.c, (struct rt6_info *)skb_dst(p));

		/* Fragment the skb. ipv6 header and the remaining fields of the
		 * fragment header are updated in ipv6_gso_segment()
		 */
		q = skb_segment(p, features);
	}

out:
	return q;
}
static const struct net_offload udpv6_offload = {
	.callbacks = {
		.gso_send_check =	udp6_ufo_send_check,
		.gso_segment	=	udp6_ufo_fragment,
	},
};

int __init udp_offload_init(void)
{
	return inet6_add_offload(&udpv6_offload, IPPROTO_UDP);
}