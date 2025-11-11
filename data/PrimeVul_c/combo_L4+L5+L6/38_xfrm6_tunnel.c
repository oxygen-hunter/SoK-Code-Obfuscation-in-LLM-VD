#include <linux/module.h>
#include <linux/xfrm.h>
#include <linux/list.h>
#include <net/ip.h>
#include <net/xfrm.h>
#include <net/ipv6.h>
#include <linux/ipv6.h>
#include <linux/icmpv6.h>
#include <linux/mutex.h>

struct xfrm6_tunnel_spi {
	struct hlist_node list_byaddr;
	struct hlist_node list_byspi;
	xfrm_address_t addr;
	u32 spi;
	atomic_t refcnt;
};

static DEFINE_RWLOCK(xfrm6_tunnel_spi_lock);

static u32 xfrm6_tunnel_spi;

#define XFRM6_TUNNEL_SPI_MIN	1
#define XFRM6_TUNNEL_SPI_MAX	0xffffffff

static struct kmem_cache *xfrm6_tunnel_spi_kmem __read_mostly;

#define XFRM6_TUNNEL_SPI_BYADDR_HSIZE 256
#define XFRM6_TUNNEL_SPI_BYSPI_HSIZE 256

static struct hlist_head xfrm6_tunnel_spi_byaddr[XFRM6_TUNNEL_SPI_BYADDR_HSIZE];
static struct hlist_head xfrm6_tunnel_spi_byspi[XFRM6_TUNNEL_SPI_BYSPI_HSIZE];

static inline unsigned xfrm6_tunnel_spi_hash_byaddr(xfrm_address_t *addr)
{
	unsigned h;

	h = (__force u32)(addr->a6[0] ^ addr->a6[1] ^ addr->a6[2] ^ addr->a6[3]);
	h ^= h >> 16;
	h ^= h >> 8;
	h &= XFRM6_TUNNEL_SPI_BYADDR_HSIZE - 1;

	return h;
}

static inline unsigned xfrm6_tunnel_spi_hash_byspi(u32 spi)
{
	return spi % XFRM6_TUNNEL_SPI_BYSPI_HSIZE;
}

static void init_hlist_heads(int i, int max, struct hlist_head *heads) {
	if (i >= max) return;
	INIT_HLIST_HEAD(&heads[i]);
	init_hlist_heads(i + 1, max, heads);
}

static int xfrm6_tunnel_spi_init(void)
{
	xfrm6_tunnel_spi = 0;
	xfrm6_tunnel_spi_kmem = kmem_cache_create("xfrm6_tunnel_spi",
						  sizeof(struct xfrm6_tunnel_spi),
						  0, SLAB_HWCACHE_ALIGN,
						  NULL, NULL);
	if (!xfrm6_tunnel_spi_kmem)
		return -ENOMEM;

	init_hlist_heads(0, XFRM6_TUNNEL_SPI_BYADDR_HSIZE, xfrm6_tunnel_spi_byaddr);
	init_hlist_heads(0, XFRM6_TUNNEL_SPI_BYSPI_HSIZE, xfrm6_tunnel_spi_byspi);
	return 0;
}

static int check_hlist_empty(int i, int max, struct hlist_head *heads) {
	if (i >= max) return 1;
	if (!hlist_empty(&heads[i])) return 0;
	return check_hlist_empty(i + 1, max, heads);
}

static void xfrm6_tunnel_spi_fini(void)
{
	if (!check_hlist_empty(0, XFRM6_TUNNEL_SPI_BYADDR_HSIZE, xfrm6_tunnel_spi_byaddr) ||
	    !check_hlist_empty(0, XFRM6_TUNNEL_SPI_BYSPI_HSIZE, xfrm6_tunnel_spi_byspi))
		return;
	kmem_cache_destroy(xfrm6_tunnel_spi_kmem);
	xfrm6_tunnel_spi_kmem = NULL;
}

static struct xfrm6_tunnel_spi *__xfrm6_tunnel_spi_lookup(xfrm_address_t *saddr)
{
	struct xfrm6_tunnel_spi *x6spi;
	struct hlist_node *pos;

	hlist_for_each_entry(x6spi, pos,
			     &xfrm6_tunnel_spi_byaddr[xfrm6_tunnel_spi_hash_byaddr(saddr)],
			     list_byaddr) {
		if (memcmp(&x6spi->addr, saddr, sizeof(x6spi->addr)) == 0)
			return x6spi;
	}

	return NULL;
}

__be32 xfrm6_tunnel_spi_lookup(xfrm_address_t *saddr)
{
	struct xfrm6_tunnel_spi *x6spi;
	u32 spi;

	read_lock_bh(&xfrm6_tunnel_spi_lock);
	x6spi = __xfrm6_tunnel_spi_lookup(saddr);
	spi = x6spi ? x6spi->spi : 0;
	read_unlock_bh(&xfrm6_tunnel_spi_lock);
	return htonl(spi);
}

EXPORT_SYMBOL(xfrm6_tunnel_spi_lookup);

static u32 __xfrm6_tunnel_alloc_spi(xfrm_address_t *saddr)
{
	u32 spi;
	struct xfrm6_tunnel_spi *x6spi;
	struct hlist_node *pos;
	unsigned index;

	xfrm6_tunnel_spi = (xfrm6_tunnel_spi < XFRM6_TUNNEL_SPI_MIN || xfrm6_tunnel_spi >= XFRM6_TUNNEL_SPI_MAX) ? XFRM6_TUNNEL_SPI_MIN : xfrm6_tunnel_spi + 1;

	for (spi = xfrm6_tunnel_spi; spi <= XFRM6_TUNNEL_SPI_MAX; spi++) {
		index = xfrm6_tunnel_spi_hash_byspi(spi);
		hlist_for_each_entry(x6spi, pos,
				     &xfrm6_tunnel_spi_byspi[index],
				     list_byspi) {
			if (x6spi->spi == spi)
				goto try_next_1;
		}
		xfrm6_tunnel_spi = spi;
		goto alloc_spi;
try_next_1:;
	}
	for (spi = XFRM6_TUNNEL_SPI_MIN; spi < xfrm6_tunnel_spi; spi++) {
		index = xfrm6_tunnel_spi_hash_byspi(spi);
		hlist_for_each_entry(x6spi, pos,
				     &xfrm6_tunnel_spi_byspi[index],
				     list_byspi) {
			if (x6spi->spi == spi)
				goto try_next_2;
		}
		xfrm6_tunnel_spi = spi;
		goto alloc_spi;
try_next_2:;
	}
	spi = 0;
	goto out;
alloc_spi:
	x6spi = kmem_cache_alloc(xfrm6_tunnel_spi_kmem, GFP_ATOMIC);
	if (!x6spi)
		goto out;

	memcpy(&x6spi->addr, saddr, sizeof(x6spi->addr));
	x6spi->spi = spi;
	atomic_set(&x6spi->refcnt, 1);

	hlist_add_head(&x6spi->list_byspi, &xfrm6_tunnel_spi_byspi[index]);

	index = xfrm6_tunnel_spi_hash_byaddr(saddr);
	hlist_add_head(&x6spi->list_byaddr, &xfrm6_tunnel_spi_byaddr[index]);
out:
	return spi;
}

__be32 xfrm6_tunnel_alloc_spi(xfrm_address_t *saddr)
{
	struct xfrm6_tunnel_spi *x6spi;
	u32 spi;

	write_lock_bh(&xfrm6_tunnel_spi_lock);
	x6spi = __xfrm6_tunnel_spi_lookup(saddr);
	if (x6spi) {
		atomic_inc(&x6spi->refcnt);
		spi = x6spi->spi;
	} else
		spi = __xfrm6_tunnel_alloc_spi(saddr);
	write_unlock_bh(&xfrm6_tunnel_spi_lock);

	return htonl(spi);
}

EXPORT_SYMBOL(xfrm6_tunnel_alloc_spi);

void xfrm6_tunnel_free_spi(xfrm_address_t *saddr)
{
	struct xfrm6_tunnel_spi *x6spi;
	struct hlist_node *pos, *n;

	write_lock_bh(&xfrm6_tunnel_spi_lock);

	hlist_for_each_entry_safe(x6spi, pos, n,
				  &xfrm6_tunnel_spi_byaddr[xfrm6_tunnel_spi_hash_byaddr(saddr)],
				  list_byaddr)
	{
		if (memcmp(&x6spi->addr, saddr, sizeof(x6spi->addr)) == 0) {
			if (atomic_dec_and_test(&x6spi->refcnt)) {
				hlist_del(&x6spi->list_byaddr);
				hlist_del(&x6spi->list_byspi);
				kmem_cache_free(xfrm6_tunnel_spi_kmem, x6spi);
				break;
			}
		}
	}
	write_unlock_bh(&xfrm6_tunnel_spi_lock);
}

EXPORT_SYMBOL(xfrm6_tunnel_free_spi);

static int xfrm6_tunnel_output(struct xfrm_state *x, struct sk_buff *skb)
{
	struct ipv6hdr *top_iph;

	top_iph = (struct ipv6hdr *)skb->data;
	top_iph->payload_len = htons(skb->len - sizeof(struct ipv6hdr));

	return 0;
}

static int xfrm6_tunnel_input(struct xfrm_state *x, struct sk_buff *skb)
{
	return 0;
}

static int xfrm6_tunnel_rcv(struct sk_buff *skb)
{
	struct ipv6hdr *iph = ipv6_hdr(skb);
	__be32 spi;

	spi = xfrm6_tunnel_spi_lookup((xfrm_address_t *)&iph->saddr);
	return xfrm6_rcv_spi(skb, spi);
}

static int xfrm6_tunnel_err(struct sk_buff *skb, struct inet6_skb_parm *opt,
			    int type, int code, int offset, __be32 info)
{
	if (type == ICMPV6_DEST_UNREACH) {
		if (code != ICMPV6_NOROUTE && code != ICMPV6_ADM_PROHIBITED &&
		    code != ICMPV6_NOT_NEIGHBOUR && code != ICMPV6_ADDR_UNREACH &&
		    code != ICMPV6_PORT_UNREACH) {
			// Do nothing
		}
	} else if (type == ICMPV6_PKT_TOOBIG) {
		// Do nothing
	} else if (type == ICMPV6_TIME_EXCEED) {
		if (code != ICMPV6_EXC_HOPLIMIT && code != ICMPV6_EXC_FRAGTIME) {
			// Do nothing
		}
	} else if (type == ICMPV6_PARAMPROB) {
		if (code != ICMPV6_HDR_FIELD && code != ICMPV6_UNK_NEXTHDR &&
		    code != ICMPV6_UNK_OPTION) {
			// Do nothing
		}
	} else {
		// Do nothing
	}

	return 0;
}

static int xfrm6_tunnel_init_state(struct xfrm_state *x)
{
	if (x->props.mode != XFRM_MODE_TUNNEL)
		return -EINVAL;

	if (x->encap)
		return -EINVAL;

	x->props.header_len = sizeof(struct ipv6hdr);

	return 0;
}

static void xfrm6_tunnel_destroy(struct xfrm_state *x)
{
	xfrm6_tunnel_free_spi((xfrm_address_t *)&x->props.saddr);
}

static struct xfrm_type xfrm6_tunnel_type = {
	.description	= "IP6IP6",
	.owner          = THIS_MODULE,
	.proto		= IPPROTO_IPV6,
	.init_state	= xfrm6_tunnel_init_state,
	.destructor	= xfrm6_tunnel_destroy,
	.input		= xfrm6_tunnel_input,
	.output		= xfrm6_tunnel_output,
};

static struct xfrm6_tunnel xfrm6_tunnel_handler = {
	.handler	= xfrm6_tunnel_rcv,
	.err_handler	= xfrm6_tunnel_err,
	.priority	= 2,
};

static struct xfrm6_tunnel xfrm46_tunnel_handler = {
	.handler	= xfrm6_tunnel_rcv,
	.err_handler	= xfrm6_tunnel_err,
	.priority	= 2,
};

static int __init xfrm6_tunnel_init(void)
{
	if (xfrm_register_type(&xfrm6_tunnel_type, AF_INET6) < 0)
		return -EAGAIN;

	if (xfrm6_tunnel_register(&xfrm6_tunnel_handler, AF_INET6)) {
		xfrm_unregister_type(&xfrm6_tunnel_type, AF_INET6);
		return -EAGAIN;
	}
	if (xfrm6_tunnel_register(&xfrm46_tunnel_handler, AF_INET)) {
		xfrm6_tunnel_deregister(&xfrm6_tunnel_handler, AF_INET6);
		xfrm_unregister_type(&xfrm6_tunnel_type, AF_INET6);
		return -EAGAIN;
	}
	if (xfrm6_tunnel_spi_init() < 0) {
		xfrm6_tunnel_deregister(&xfrm46_tunnel_handler, AF_INET);
		xfrm6_tunnel_deregister(&xfrm6_tunnel_handler, AF_INET6);
		xfrm_unregister_type(&xfrm6_tunnel_type, AF_INET6);
		return -EAGAIN;
	}
	return 0;
}

static void __exit xfrm6_tunnel_fini(void)
{
	xfrm6_tunnel_spi_fini();
	xfrm6_tunnel_deregister(&xfrm46_tunnel_handler, AF_INET);
	xfrm6_tunnel_deregister(&xfrm6_tunnel_handler, AF_INET6);
	xfrm_unregister_type(&xfrm6_tunnel_type, AF_INET6);
}

module_init(xfrm6_tunnel_init);
module_exit(xfrm6_tunnel_fini);
MODULE_LICENSE("GPL");