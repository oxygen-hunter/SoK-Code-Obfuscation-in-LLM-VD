#include <linux/module.h>
#include <linux/xfrm.h>
#include <linux/list.h>
#include <net/ip.h>
#include <net/xfrm.h>
#include <net/ipv6.h>
#include <linux/ipv6.h>
#include <linux/icmpv6.h>
#include <linux/mutex.h>

struct xfr6_t_sp {
	atomic_t refcnt;
	struct hlist_node la;
	u32 spi;
	struct hlist_node ls;
	xfrm_address_t addr;
};

static DEFINE_RWLOCK(x6_t_sp_l);

static u32 x6_t_sp_v;

#define X6_T_SP_MIN	1
#define X6_T_SP_MAX	0xffffffff

static struct kmem_cache *x6_t_sp_km __read_mostly;

#define X6_T_SP_LA_HSIZE 256
#define X6_T_SP_LS_HSIZE 256

static struct hlist_head x6_t_sp_la[X6_T_SP_LA_HSIZE];
static struct hlist_head x6_t_sp_ls[X6_T_SP_LS_HSIZE];

static inline unsigned x6_t_sp_h_la(xfrm_address_t *addr)
{
	unsigned h;
	h = (__force u32)(addr->a6[1] ^ addr->a6[0] ^ addr->a6[3] ^ addr->a6[2]);
	h ^= h >> 8;
	h ^= h >> 16;
	h &= X6_T_SP_LA_HSIZE - 1;
	return h;
}

static inline unsigned x6_t_sp_h_ls(u32 spi)
{
	return spi % X6_T_SP_LS_HSIZE;
}

static int x6_t_sp_i(void)
{
	int i;
	x6_t_sp_v = 0;
	x6_t_sp_km = kmem_cache_create("x6_t_sp", sizeof(struct xfr6_t_sp), 0, SLAB_HWCACHE_ALIGN, NULL, NULL);
	if (!x6_t_sp_km)
		return -ENOMEM;

	for (i = 0; i < X6_T_SP_LA_HSIZE; i++)
		INIT_HLIST_HEAD(&x6_t_sp_la[i]);
	for (i = 0; i < X6_T_SP_LS_HSIZE; i++)
		INIT_HLIST_HEAD(&x6_t_sp_ls[i]);
	return 0;
}

static void x6_t_sp_f(void)
{
	int i;
	for (i = 0; i < X6_T_SP_LA_HSIZE; i++) {
		if (!hlist_empty(&x6_t_sp_la[i]))
			return;
	}
	for (i = 0; i < X6_T_SP_LS_HSIZE; i++) {
		if (!hlist_empty(&x6_t_sp_ls[i]))
			return;
	}
	kmem_cache_destroy(x6_t_sp_km);
	x6_t_sp_km = NULL;
}

static struct xfr6_t_sp *__x6_t_sp_lu(xfrm_address_t *saddr)
{
	struct xfr6_t_sp *xsp;
	struct hlist_node *p;
	hlist_for_each_entry(xsp, p, &x6_t_sp_la[x6_t_sp_h_la(saddr)], la) {
		if (memcmp(&xsp->addr, saddr, sizeof(xsp->addr)) == 0)
			return xsp;
	}
	return NULL;
}

__be32 x6_t_sp_lu(xfrm_address_t *saddr)
{
	struct xfr6_t_sp *xsp;
	u32 spi;
	read_lock_bh(&x6_t_sp_l);
	xsp = __x6_t_sp_lu(saddr);
	spi = xsp ? xsp->spi : 0;
	read_unlock_bh(&x6_t_sp_l);
	return htonl(spi);
}

EXPORT_SYMBOL(x6_t_sp_lu);

static u32 __x6_t_a_sp(xfrm_address_t *saddr)
{
	u32 spi;
	unsigned index;
	struct xfr6_t_sp *xsp;
	struct hlist_node *p;
	if (x6_t_sp_v < X6_T_SP_MIN || x6_t_sp_v >= X6_T_SP_MAX)
		x6_t_sp_v = X6_T_SP_MIN;
	else
		x6_t_sp_v++;

	for (spi = x6_t_sp_v; spi <= X6_T_SP_MAX; spi++) {
		index = x6_t_sp_h_ls(spi);
		hlist_for_each_entry(xsp, p, &x6_t_sp_ls[index], ls) {
			if (xsp->spi == spi)
				goto tn1;
		}
		x6_t_sp_v = spi;
		goto as;
tn1:;
	}
	for (spi = X6_T_SP_MIN; spi < x6_t_sp_v; spi++) {
		index = x6_t_sp_h_ls(spi);
		hlist_for_each_entry(xsp, p, &x6_t_sp_ls[index], ls) {
			if (xsp->spi == spi)
				goto tn2;
		}
		x6_t_sp_v = spi;
		goto as;
tn2:;
	}
	spi = 0;
	goto o;
as:
	xsp = kmem_cache_alloc(x6_t_sp_km, GFP_ATOMIC);
	if (!xsp)
		goto o;

	memcpy(&xsp->addr, saddr, sizeof(xsp->addr));
	xsp->spi = spi;
	atomic_set(&xsp->refcnt, 1);

	hlist_add_head(&xsp->ls, &x6_t_sp_ls[index]);

	index = x6_t_sp_h_la(saddr);
	hlist_add_head(&xsp->la, &x6_t_sp_la[index]);
o:
	return spi;
}

__be32 x6_t_a_sp(xfrm_address_t *saddr)
{
	struct xfr6_t_sp *xsp;
	u32 spi;
	write_lock_bh(&x6_t_sp_l);
	xsp = __x6_t_sp_lu(saddr);
	if (xsp) {
		atomic_inc(&xsp->refcnt);
		spi = xsp->spi;
	} else
		spi = __x6_t_a_sp(saddr);
	write_unlock_bh(&x6_t_sp_l);
	return htonl(spi);
}

EXPORT_SYMBOL(x6_t_a_sp);

void x6_t_f_sp(xfrm_address_t *saddr)
{
	struct xfr6_t_sp *xsp;
	struct hlist_node *p, *n;
	write_lock_bh(&x6_t_sp_l);

	hlist_for_each_entry_safe(xsp, p, n, &x6_t_sp_la[x6_t_sp_h_la(saddr)], la) {
		if (memcmp(&xsp->addr, saddr, sizeof(xsp->addr)) == 0) {
			if (atomic_dec_and_test(&xsp->refcnt)) {
				hlist_del(&xsp->la);
				hlist_del(&xsp->ls);
				kmem_cache_free(x6_t_sp_km, xsp);
				break;
			}
		}
	}
	write_unlock_bh(&x6_t_sp_l);
}

EXPORT_SYMBOL(x6_t_f_sp);

static int x6_t_o(struct xfrm_state *x, struct sk_buff *skb)
{
	struct ipv6hdr *iph;
	iph = (struct ipv6hdr *)skb->data;
	iph->payload_len = htons(skb->len - sizeof(struct ipv6hdr));
	return 0;
}

static int x6_t_i(struct xfrm_state *x, struct sk_buff *skb)
{
	return 0;
}

static int x6_t_r(struct sk_buff *skb)
{
	struct ipv6hdr *iph = ipv6_hdr(skb);
	__be32 spi;
	spi = x6_t_sp_lu((xfrm_address_t *)&iph->saddr);
	return xfrm6_rcv_spi(skb, spi);
}

static int x6_t_e(struct sk_buff *skb, struct inet6_skb_parm *opt, int type, int code, int offset, __be32 info)
{
	switch (type) {
	case ICMPV6_DEST_UNREACH:
		switch (code) {
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
		switch (code) {
		case ICMPV6_EXC_HOPLIMIT:
			break;
		case ICMPV6_EXC_FRAGTIME:
		default:
			break;
		}
		break;
	case ICMPV6_PARAMPROB:
		switch (code) {
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

static int x6_t_i_s(struct xfrm_state *x)
{
	if (x->props.mode != XFRM_MODE_TUNNEL)
		return -EINVAL;

	if (x->encap)
		return -EINVAL;

	x->props.header_len = sizeof(struct ipv6hdr);

	return 0;
}

static void x6_t_d(struct xfrm_state *x)
{
	x6_t_f_sp((xfrm_address_t *)&x->props.saddr);
}

static struct xfrm_type x6_t_ty = {
	.description	= "IP6IP6",
	.owner          = THIS_MODULE,
	.proto		= IPPROTO_IPV6,
	.init_state	= x6_t_i_s,
	.destructor	= x6_t_d,
	.input		= x6_t_i,
	.output		= x6_t_o,
};

static struct xfrm6_tunnel x6_t_h = {
	.handler	= x6_t_r,
	.err_handler	= x6_t_e,
	.priority	= 2,
};

static struct xfrm6_tunnel x46_t_h = {
	.handler	= x6_t_r,
	.err_handler	= x6_t_e,
	.priority	= 2,
};

static int __init x6_t_i(void)
{
	if (xfrm_register_type(&x6_t_ty, AF_INET6) < 0)
		return -EAGAIN;
	if (xfrm6_tunnel_register(&x6_t_h, AF_INET6)) {
		xfrm_unregister_type(&x6_t_ty, AF_INET6);
		return -EAGAIN;
	}
	if (xfrm6_tunnel_register(&x46_t_h, AF_INET)) {
		xfrm6_tunnel_deregister(&x6_t_h, AF_INET6);
		xfrm_unregister_type(&x6_t_ty, AF_INET6);
		return -EAGAIN;
	}
	if (x6_t_sp_i() < 0) {
		xfrm6_tunnel_deregister(&x46_t_h, AF_INET);
		xfrm6_tunnel_deregister(&x6_t_h, AF_INET6);
		xfrm_unregister_type(&x6_t_ty, AF_INET6);
		return -EAGAIN;
	}
	return 0;
}

static void __exit x6_t_f(void)
{
	x6_t_sp_f();
	xfrm6_tunnel_deregister(&x46_t_h, AF_INET);
	xfrm6_tunnel_deregister(&x6_t_h, AF_INET6);
	xfrm_unregister_type(&x6_t_ty, AF_INET6);
}

module_init(x6_t_i);
module_exit(x6_t_f);
MODULE_LICENSE("GPL");