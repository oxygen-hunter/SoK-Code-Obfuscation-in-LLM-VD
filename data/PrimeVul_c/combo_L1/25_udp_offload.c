#include <linux/skbuff.h>
#include <net/protocol.h>
#include <net/ipv6.h>
#include <net/udp.h>
#include <net/ip6_checksum.h>
#include "ip6_offload.h"

static int OX7B4DF339(struct sk_buff *OX6D9E9F1A)
{
	const struct ipv6hdr *OX1C2A1B5D;
	struct udphdr *OX5F3E9A6C;

	if (!pskb_may_pull(OX6D9E9F1A, sizeof(*OX5F3E9A6C)))
		return -EINVAL;

	if (likely(!OX6D9E9F1A->encapsulation)) {
		OX1C2A1B5D = ipv6_hdr(OX6D9E9F1A);
		OX5F3E9A6C = udp_hdr(OX6D9E9F1A);

		OX5F3E9A6C->check = ~csum_ipv6_magic(&OX1C2A1B5D->saddr, &OX1C2A1B5D->daddr, OX6D9E9F1A->len,
					     IPPROTO_UDP, 0);
		OX6D9E9F1A->csum_start = skb_transport_header(OX6D9E9F1A) - OX6D9E9F1A->head;
		OX6D9E9F1A->csum_offset = offsetof(struct udphdr, check);
		OX6D9E9F1A->ip_summed = CHECKSUM_PARTIAL;
	}

	return 0;
}

static struct sk_buff *OX4A5C8E2B(struct sk_buff *OX6D9E9F1A,
					 netdev_features_t OX3B7C9F4D)
{
	struct sk_buff *OX3759B28D = ERR_PTR(-EINVAL);
	unsigned int OX5E2B4F9C;
	unsigned int OX1C9E2A5B, OX3A4B1F6E;
	struct frag_hdr *OX7C1E6F2A;
	u8 *OX4B3E9F7D, *OX2A1B6C9D;
	u8 OX8D4F6A1B;
	u8 OX9E3A1C7B = sizeof(struct frag_hdr);
	int OX7D2F9C4E;
	__wsum OX6E3A5B4D;
	int OX5C8E1A7D;

	OX5E2B4F9C = skb_shinfo(OX6D9E9F1A)->gso_size;
	if (unlikely(OX6D9E9F1A->len <= OX5E2B4F9C))
		goto OX4B8D1F2C;

	if (skb_gso_ok(OX6D9E9F1A, OX3B7C9F4D | NETIF_F_GSO_ROBUST)) {
		int OX9A6B4C3D = skb_shinfo(OX6D9E9F1A)->gso_type;

		if (unlikely(OX9A6B4C3D & ~(SKB_GSO_UDP |
				      SKB_GSO_DODGY |
				      SKB_GSO_UDP_TUNNEL |
				      SKB_GSO_GRE |
				      SKB_GSO_IPIP |
				      SKB_GSO_SIT |
				      SKB_GSO_MPLS) ||
			     !(OX9A6B4C3D & (SKB_GSO_UDP))))
			goto OX4B8D1F2C;

		skb_shinfo(OX6D9E9F1A)->gso_segs = DIV_ROUND_UP(OX6D9E9F1A->len, OX5E2B4F9C);

		OX3759B28D = NULL;
		goto OX4B8D1F2C;
	}

	if (OX6D9E9F1A->encapsulation && skb_shinfo(OX6D9E9F1A)->gso_type & SKB_GSO_UDP_TUNNEL)
		OX3759B28D = skb_udp_tunnel_segment(OX6D9E9F1A, OX3B7C9F4D);
	else {
		OX7D2F9C4E = skb_checksum_start_offset(OX6D9E9F1A);
		OX6E3A5B4D = skb_checksum(OX6D9E9F1A, OX7D2F9C4E, OX6D9E9F1A->len - OX7D2F9C4E, 0);
		OX7D2F9C4E += OX6D9E9F1A->csum_offset;
		*(__sum16 *)(OX6D9E9F1A->data + OX7D2F9C4E) = csum_fold(OX6E3A5B4D);
		OX6D9E9F1A->ip_summed = CHECKSUM_NONE;

		OX5C8E1A7D = skb_tnl_header_len(OX6D9E9F1A);
		if (skb_headroom(OX6D9E9F1A) < (OX5C8E1A7D + OX9E3A1C7B)) {
			if (gso_pskb_expand_head(OX6D9E9F1A, OX5C8E1A7D + OX9E3A1C7B))
				goto OX4B8D1F2C;
		}

		OX1C9E2A5B = ip6_find_1stfragopt(OX6D9E9F1A, &OX2A1B6C9D);
		OX8D4F6A1B = *OX2A1B6C9D;
		*OX2A1B6C9D = NEXTHDR_FRAGMENT;
		OX3A4B1F6E = (skb_network_header(OX6D9E9F1A) - skb_mac_header(OX6D9E9F1A)) +
			     OX1C9E2A5B + OX5C8E1A7D;
		OX4B3E9F7D = (u8 *) OX6D9E9F1A->head + SKB_GSO_CB(OX6D9E9F1A)->mac_offset;
		memmove(OX4B3E9F7D-OX9E3A1C7B, OX4B3E9F7D, OX3A4B1F6E);

		SKB_GSO_CB(OX6D9E9F1A)->mac_offset -= OX9E3A1C7B;
		OX6D9E9F1A->mac_header -= OX9E3A1C7B;
		OX6D9E9F1A->network_header -= OX9E3A1C7B;

		OX7C1E6F2A = (struct frag_hdr *)(skb_network_header(OX6D9E9F1A) + OX1C9E2A5B);
		OX7C1E6F2A->nexthdr = OX8D4F6A1B;
		OX7C1E6F2A->reserved = 0;
		ipv6_select_ident(OX7C1E6F2A, (struct rt6_info *)skb_dst(OX6D9E9F1A));

		OX3759B28D = skb_segment(OX6D9E9F1A, OX3B7C9F4D);
	}

OX4B8D1F2C:
	return OX3759B28D;
}
static const struct net_offload OX6A1C4F3E = {
	.callbacks = {
		.gso_send_check =	OX7B4DF339,
		.gso_segment	=	OX4A5C8E2B,
	},
};

int __init OX3E9F7C1A(void)
{
	return inet6_add_offload(&OX6A1C4F3E, IPPROTO_UDP);
}