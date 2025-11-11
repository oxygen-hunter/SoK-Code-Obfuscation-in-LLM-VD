#include <linux/module.h>
#include <linux/xfrm.h>
#include <linux/list.h>
#include <net/ip.h>
#include <net/xfrm.h>
#include <net/ipv6.h>
#include <linux/ipv6.h>
#include <linux/icmpv6.h>
#include <linux/mutex.h>

struct OX5A7C4E6E {
	struct hlist_node OX7E3D1A29;
	struct hlist_node OX4F8A9D3C;
	xfrm_address_t OX3C9F1B7D;
	u32 OX2B4C8D1E;
	atomic_t OX6C7E3A5B;
};

static DEFINE_RWLOCK(OX1B2E3C4D);

static u32 OX6A7B8C9D;

#define OX9E8F7D6C	1
#define OX4F3E2D1C	0xffffffff

static struct kmem_cache *OX3A1B2C3D __read_mostly;

#define OX8D7C6B5A 256
#define OX7E6F5D4C 256

static struct hlist_head OX9A8B7C6D[OX8D7C6B5A];
static struct hlist_head OX6B5A4E3D[OX7E6F5D4C];

static inline unsigned OX1F2E3D4C(xfrm_address_t *OX2A3B4C5D)
{
	unsigned OX3C4D5E6F;

	OX3C4D5E6F = (__force u32)(OX2A3B4C5D->a6[0] ^ OX2A3B4C5D->a6[1] ^ OX2A3B4C5D->a6[2] ^ OX2A3B4C5D->a6[3]);
	OX3C4D5E6F ^= OX3C4D5E6F >> 16;
	OX3C4D5E6F ^= OX3C4D5E6F >> 8;
	OX3C4D5E6F &= OX8D7C6B5A - 1;

	return OX3C4D5E6F;
}

static inline unsigned OX5D4C3B2A(u32 OX6E7F8D9C)
{
	return OX6E7F8D9C % OX7E6F5D4C;
}

static int OX4D5E6F7A(void)
{
	int OX1A2B3C4D;

	OX6A7B8C9D = 0;
	OX3A1B2C3D = kmem_cache_create("xfrm6_tunnel_spi",
						  sizeof(struct OX5A7C4E6E),
						  0, SLAB_HWCACHE_ALIGN,
						  NULL, NULL);
	if (!OX3A1B2C3D)
		return -ENOMEM;

	for (OX1A2B3C4D = 0; OX1A2B3C4D < OX8D7C6B5A; OX1A2B3C4D++)
		INIT_HLIST_HEAD(&OX9A8B7C6D[OX1A2B3C4D]);
	for (OX1A2B3C4D = 0; OX1A2B3C4D < OX7E6F5D4C; OX1A2B3C4D++)
		INIT_HLIST_HEAD(&OX6B5A4E3D[OX1A2B3C4D]);
	return 0;
}

static void OX9C8B7A6D(void)
{
	int OX7E6D5C4B;

	for (OX7E6D5C4B = 0; OX7E6D5C4B < OX8D7C6B5A; OX7E6D5C4B++) {
		if (!hlist_empty(&OX9A8B7C6D[OX7E6D5C4B]))
			return;
	}
	for (OX7E6D5C4B = 0; OX7E6D5C4B < OX7E6F5D4C; OX7E6D5C4B++) {
		if (!hlist_empty(&OX6B5A4E3D[OX7E6D5C4B]))
			return;
	}
	kmem_cache_destroy(OX3A1B2C3D);
	OX3A1B2C3D = NULL;
}

static struct OX5A7C4E6E *OX8B7A6C5D(xfrm_address_t *OX9F8E7D6C)
{
	struct OX5A7C4E6E *OX4E3D2C1B;
	struct hlist_node *OX5C4B3A2D;

	hlist_for_each_entry(OX4E3D2C1B, OX5C4B3A2D,
			     &OX9A8B7C6D[OX1F2E3D4C(OX9F8E7D6C)],
			     OX7E3D1A29) {
		if (memcmp(&OX4E3D2C1B->OX3C9F1B7D, OX9F8E7D6C, sizeof(OX4E3D2C1B->OX3C9F1B7D)) == 0)
			return OX4E3D2C1B;
	}

	return NULL;
}

__be32 OX7B6A5C4D(xfrm_address_t *OX8D7E6F5C)
{
	struct OX5A7C4E6E *OX1B2C3D4A;
	u32 OX2C3B4A5D;

	read_lock_bh(&OX1B2E3C4D);
	OX1B2C3D4A = OX8B7A6C5D(OX8D7E6F5C);
	OX2C3B4A5D = OX1B2C3D4A ? OX1B2C3D4A->OX2B4C8D1E : 0;
	read_unlock_bh(&OX1B2E3C4D);
	return htonl(OX2C3B4A5D);
}

EXPORT_SYMBOL(OX7B6A5C4D);

static u32 OX4A3B2C1D(xfrm_address_t *OX5D4C3E2F)
{
	u32 OX6B5A4D3C;
	struct OX5A7C4E6E *OX2E3F4A5B;
	struct hlist_node *OX1D2C3B4A;
	unsigned OX4B3C2D1A;

	if (OX6A7B8C9D < OX9E8F7D6C ||
	    OX6A7B8C9D >= OX4F3E2D1C)
		OX6A7B8C9D = OX9E8F7D6C;
	else
		OX6A7B8C9D++;

	for (OX6B5A4D3C = OX6A7B8C9D; OX6B5A4D3C <= OX4F3E2D1C; OX6B5A4D3C++) {
		OX4B3C2D1A = OX5D4C3B2A(OX6B5A4D3C);
		hlist_for_each_entry(OX2E3F4A5B, OX1D2C3B4A,
				     &OX6B5A4E3D[OX4B3C2D1A],
				     OX4F8A9D3C) {
			if (OX2E3F4A5B->OX2B4C8D1E == OX6B5A4D3C)
				goto OX5B4A3C2D;
		}
		OX6A7B8C9D = OX6B5A4D3C;
		goto OX6C5B4A3D;
OX5B4A3C2D:;
	}
	for (OX6B5A4D3C = OX9E8F7D6C; OX6B5A4D3C < OX6A7B8C9D; OX6B5A4D3C++) {
		OX4B3C2D1A = OX5D4C3B2A(OX6B5A4D3C);
		hlist_for_each_entry(OX2E3F4A5B, OX1D2C3B4A,
				     &OX6B5A4E3D[OX4B3C2D1A],
				     OX4F8A9D3C) {
			if (OX2E3F4A5B->OX2B4C8D1E == OX6B5A4D3C)
				goto OX3D2C1B4A;
		}
		OX6A7B8C9D = OX6B5A4D3C;
		goto OX6C5B4A3D;
OX3D2C1B4A:;
	}
	OX6B5A4D3C = 0;
	goto OX9A8B7C6D;
OX6C5B4A3D:
	OX2E3F4A5B = kmem_cache_alloc(OX3A1B2C3D, GFP_ATOMIC);
	if (!OX2E3F4A5B)
		goto OX9A8B7C6D;

	memcpy(&OX2E3F4A5B->OX3C9F1B7D, OX5D4C3E2F, sizeof(OX2E3F4A5B->OX3C9F1B7D));
	OX2E3F4A5B->OX2B4C8D1E = OX6B5A4D3C;
	atomic_set(&OX2E3F4A5B->OX6C7E3A5B, 1);

	hlist_add_head(&OX2E3F4A5B->OX4F8A9D3C, &OX6B5A4E3D[OX4B3C2D1A]);

	OX4B3C2D1A = OX1F2E3D4C(OX5D4C3E2F);
	hlist_add_head(&OX2E3F4A5B->OX7E3D1A29, &OX9A8B7C6D[OX4B3C2D1A]);
OX9A8B7C6D:
	return OX6B5A4D3C;
}

__be32 OX3D4C5B2A(xfrm_address_t *OX1F2E3D4B)
{
	struct OX5A7C4E6E *OX3B2C1A4D;
	u32 OX5E4D3C2B;

	write_lock_bh(&OX1B2E3C4D);
	OX3B2C1A4D = OX8B7A6C5D(OX1F2E3D4B);
	if (OX3B2C1A4D) {
		atomic_inc(&OX3B2C1A4D->OX6C7E3A5B);
		OX5E4D3C2B = OX3B2C1A4D->OX2B4C8D1E;
	} else
		OX5E4D3C2B = OX4A3B2C1D(OX1F2E3D4B);
	write_unlock_bh(&OX1B2E3C4D);

	return htonl(OX5E4D3C2B);
}

EXPORT_SYMBOL(OX3D4C5B2A);

void OX7A6B8C9D(xfrm_address_t *OX2D3C4B5A)
{
	struct OX5A7C4E6E *OX5F4E3D2C;
	struct hlist_node *OX9B8A7D6C, *OX1F2E3D4A;

	write_lock_bh(&OX1B2E3C4D);

	hlist_for_each_entry_safe(OX5F4E3D2C, OX9B8A7D6C, OX1F2E3D4A,
				  &OX9A8B7C6D[OX1F2E3D4C(OX2D3C4B5A)],
				  OX7E3D1A29)
	{
		if (memcmp(&OX5F4E3D2C->OX3C9F1B7D, OX2D3C4B5A, sizeof(OX5F4E3D2C->OX3C9F1B7D)) == 0) {
			if (atomic_dec_and_test(&OX5F4E3D2C->OX6C7E3A5B)) {
				hlist_del(&OX5F4E3D2C->OX7E3D1A29);
				hlist_del(&OX5F4E3D2C->OX4F8A9D3C);
				kmem_cache_free(OX3A1B2C3D, OX5F4E3D2C);
				break;
			}
		}
	}
	write_unlock_bh(&OX1B2E3C4D);
}

EXPORT_SYMBOL(OX7A6B8C9D);

static int OX8A7B9C6D(struct xfrm_state *OX2E3D4C1A, struct sk_buff *OX3B2A5D4C)
{
	struct ipv6hdr *OX5A4E3D1C;

	OX5A4E3D1C = (struct ipv6hdr *)OX3B2A5D4C->data;
	OX5A4E3D1C->payload_len = htons(OX3B2A5D4C->len - sizeof(struct ipv6hdr));

	return 0;
}

static int OX2C3B4A5E(struct xfrm_state *OX6D7C4B2A, struct sk_buff *OX5E4D3C2B)
{
	return 0;
}

static int OX1B2C3D4F(struct sk_buff *OX5F4E3D2C)
{
	struct ipv6hdr *OX7D6C5B4A = ipv6_hdr(OX5F4E3D2C);
	__be32 OX3C4B5A2D;

	OX3C4B5A2D = OX7B6A5C4D((xfrm_address_t *)&OX7D6C5B4A->saddr);
	return xfrm6_rcv_spi(OX5F4E3D2C, OX3C4B5A2D);
}

static int OX6C5B4A3F(struct sk_buff *OX4D3C5B2A, struct inet6_skb_parm *OX2E1F3D4C,
			    int OX1A2B3C5D, int OX5E4D3C2B, int OX6F5A4D3C, __be32 OX9B8A7D6C)
{
	switch (OX1A2B3C5D) {
	case ICMPV6_DEST_UNREACH:
		switch (OX5E4D3C2B) {
		case ICMPV6_NOROUTE:
		case ICMPV6_ADM_PROHIBITED:
		case ICMPV6_NOT_NEIGHBOUR:
		case ICMPV6_ADDR_UNREACH:
		case ICMPV6_PORT_UNREACH:
		default:
			break;
		}
		break;
	case ICMPV6_PKT_TOOBIG:
		break;
	case ICMPV6_TIME_EXCEED:
		switch (OX5E4D3C2B) {
		case ICMPV6_EXC_HOPLIMIT:
			break;
		case ICMPV6_EXC_FRAGTIME:
		default:
			break;
		}
		break;
	case ICMPV6_PARAMPROB:
		switch (OX5E4D3C2B) {
		case ICMPV6_HDR_FIELD: break;
		case ICMPV6_UNK_NEXTHDR: break;
		case ICMPV6_UNK_OPTION: break;
		}
		break;
	default:
		break;
	}

	return 0;
}

static int OX2D3C4B5A(struct xfrm_state *OX5A4D3C2B)
{
	if (OX5A4D3C2B->props.mode != XFRM_MODE_TUNNEL)
		return -EINVAL;

	if (OX5A4D3C2B->encap)
		return -EINVAL;

	OX5A4D3C2B->props.header_len = sizeof(struct ipv6hdr);

	return 0;
}

static void OX3B2C1A4F(struct xfrm_state *OX4D3C5E2B)
{
	OX7A6B8C9D((xfrm_address_t *)&OX4D3C5E2B->props.saddr);
}

static struct xfrm_type OX6B5A4D3C = {
	.description	= "IP6IP6",
	.owner          = THIS_MODULE,
	.proto		= IPPROTO_IPV6,
	.init_state	= OX2D3C4B5A,
	.destructor	= OX3B2C1A4F,
	.input		= OX2C3B4A5E,
	.output		= OX8A7B9C6D,
};

static struct xfrm6_tunnel OX5C4B3A2E = {
	.handler	= OX1B2C3D4F,
	.err_handler	= OX6C5B4A3F,
	.priority	= 2,
};

static struct xfrm6_tunnel OX9D8C7B6A = {
	.handler	= OX1B2C3D4F,
	.err_handler	= OX6C5B4A3F,
	.priority	= 2,
};

static int __init OX4B3C2D1A(void)
{
	if (xfrm_register_type(&OX6B5A4D3C, AF_INET6) < 0)
		return -EAGAIN;

	if (xfrm6_tunnel_register(&OX5C4B3A2E, AF_INET6)) {
		xfrm_unregister_type(&OX6B5A4D3C, AF_INET6);
		return -EAGAIN;
	}
	if (xfrm6_tunnel_register(&OX9D8C7B6A, AF_INET)) {
		xfrm6_tunnel_deregister(&OX5C4B3A2E, AF_INET6);
		xfrm_unregister_type(&OX6B5A4D3C, AF_INET6);
		return -EAGAIN;
	}
	if (OX4D5E6F7A() < 0) {
		xfrm6_tunnel_deregister(&OX9D8C7B6A, AF_INET);
		xfrm6_tunnel_deregister(&OX5C4B3A2E, AF_INET6);
		xfrm_unregister_type(&OX6B5A4D3C, AF_INET6);
		return -EAGAIN;
	}
	return 0;
}

static void __exit OX2A3B4C5D(void)
{
	OX9C8B7A6D();
	xfrm6_tunnel_deregister(&OX9D8C7B6A, AF_INET);
	xfrm6_tunnel_deregister(&OX5C4B3A2E, AF_INET6);
	xfrm_unregister_type(&OX6B5A4D3C, AF_INET6);
}

module_init(OX4B3C2D1A);
module_exit(OX2A3B4C5D);
MODULE_LICENSE("GPL");