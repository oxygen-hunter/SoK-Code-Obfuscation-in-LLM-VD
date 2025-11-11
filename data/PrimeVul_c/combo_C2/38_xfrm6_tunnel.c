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

static int xfrm6_tunnel_spi_init(void)
{
	int i, state = 0;
	while (1) {
		switch (state) {
			case 0:
				xfrm6_tunnel_spi = 0;
				xfrm6_tunnel_spi_kmem = kmem_cache_create("xfrm6_tunnel_spi",
									  sizeof(struct xfrm6_tunnel_spi),
									  0, SLAB_HWCACHE_ALIGN,
									  NULL, NULL);
				if (!xfrm6_tunnel_spi_kmem)
					return -ENOMEM;
				i = 0;
				state = 1;
				break;
			case 1:
				if (i < XFRM6_TUNNEL_SPI_BYADDR_HSIZE) {
					INIT_HLIST_HEAD(&xfrm6_tunnel_spi_byaddr[i]);
					i++;
				} else {
					i = 0;
					state = 2;
				}
				break;
			case 2:
				if (i < XFRM6_TUNNEL_SPI_BYSPI_HSIZE) {
					INIT_HLIST_HEAD(&xfrm6_tunnel_spi_byspi[i]);
					i++;
				} else {
					state = 3;
				}
				break;
			case 3:
				return 0;
		}
	}
}

static void xfrm6_tunnel_spi_fini(void)
{
	int i, state = 0;
	while (1) {
		switch (state) {
			case 0:
				i = 0;
				state = 1;
				break;
			case 1:
				if (i < XFRM6_TUNNEL_SPI_BYADDR_HSIZE) {
					if (!hlist_empty(&xfrm6_tunnel_spi_byaddr[i]))
						return;
					i++;
				} else {
					i = 0;
					state = 2;
				}
				break;
			case 2:
				if (i < XFRM6_TUNNEL_SPI_BYSPI_HSIZE) {
					if (!hlist_empty(&xfrm6_tunnel_spi_byspi[i]))
						return;
					i++;
				} else {
					state = 3;
				}
				break;
			case 3:
				kmem_cache_destroy(xfrm6_tunnel_spi_kmem);
				xfrm6_tunnel_spi_kmem = NULL;
				return;
		}
	}
}

static struct xfrm6_tunnel_spi *__xfrm6_tunnel_spi_lookup(xfrm_address_t *saddr)
{
	struct xfrm6_tunnel_spi *x6spi;
	struct hlist_node *pos;
	int state = 0;
	while (1) {
		switch (state) {
			case 0:
				hlist_for_each_entry(x6spi, pos,
							 &xfrm6_tunnel_spi_byaddr[xfrm6_tunnel_spi_hash_byaddr(saddr)],
							 list_byaddr) {
					if (memcmp(&x6spi->addr, saddr, sizeof(x6spi->addr)) == 0)
						return x6spi;
				}
				state = 1;
				break;
			case 1:
				return NULL;
		}
	}
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
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				if (xfrm6_tunnel_spi < XFRM6_TUNNEL_SPI_MIN ||
					xfrm6_tunnel_spi >= XFRM6_TUNNEL_SPI_MAX)
					xfrm6_tunnel_spi = XFRM6_TUNNEL_SPI_MIN;
				else
					xfrm6_tunnel_spi++;
				spi = xfrm6_tunnel_spi;
				state = 1;
				break;
			case 1:
				if (spi <= XFRM6_TUNNEL_SPI_MAX) {
					index = xfrm6_tunnel_spi_hash_byspi(spi);
					hlist_for_each_entry(x6spi, pos,
										 &xfrm6_tunnel_spi_byspi[index],
										 list_byspi) {
						if (x6spi->spi == spi)
							goto try_next_1;
					}
					xfrm6_tunnel_spi = spi;
					state = 2;
				} else {
					spi = XFRM6_TUNNEL_SPI_MIN;
					state = 3;
				}
				break;
			case 2:
				x6spi = kmem_cache_alloc(xfrm6_tunnel_spi_kmem, GFP_ATOMIC);
				if (!x6spi)
					goto out;
				memcpy(&x6spi->addr, saddr, sizeof(x6spi->addr));
				x6spi->spi = spi;
				atomic_set(&x6spi->refcnt, 1);
				hlist_add_head(&x6spi->list_byspi, &xfrm6_tunnel_spi_byspi[index]);
				index = xfrm6_tunnel_spi_hash_byaddr(saddr);
				hlist_add_head(&x6spi->list_byaddr, &xfrm6_tunnel_spi_byaddr[index]);
				goto out;
			case 3:
				if (spi < xfrm6_tunnel_spi) {
					index = xfrm6_tunnel_spi_hash_byspi(spi);
					hlist_for_each_entry(x6spi, pos,
										 &xfrm6_tunnel_spi_byspi[index],
										 list_byspi) {
						if (x6spi->spi == spi)
							goto try_next_2;
					}
					xfrm6_tunnel_spi = spi;
					state = 2;
				} else {
					spi = 0;
					goto out;
				}
				break;
			try_next_1:
				spi++;
				state = 1;
				break;
			try_next_2:
				spi++;
				state = 3;
				break;
			out:
				return spi;
		}
	}
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
	int state = 0;

	write_lock_bh(&xfrm6_tunnel_spi_lock);
	while (1) {
		switch (state) {
			case 0:
				hlist_for_each_entry_safe(x6spi, pos, n,
								  &xfrm6_tunnel_spi_byaddr[xfrm6_tunnel_spi_hash_byaddr(saddr)],
								  list_byaddr) {
					if (memcmp(&x6spi->addr, saddr, sizeof(x6spi->addr)) == 0) {
						if (atomic_dec_and_test(&x6spi->refcnt)) {
							hlist_del(&x6spi->list_byaddr);
							hlist_del(&x6spi->list_byspi);
							kmem_cache_free(xfrm6_tunnel_spi_kmem, x6spi);
							state = 1;
						}
					}
				}
				state = 1;
				break;
			case 1:
				write_unlock_bh(&xfrm6_tunnel_spi_lock);
				return;
		}
	}
}

EXPORT_SYMBOL(xfrm6_tunnel_free_spi);

static int xfrm6_tunnel_output(struct xfrm_state *x, struct sk_buff *skb)
{
	struct ipv6hdr *top_iph;
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				top_iph = (struct ipv6hdr *)skb->data;
				top_iph->payload_len = htons(skb->len - sizeof(struct ipv6hdr));
				state = 1;
				break;
			case 1:
				return 0;
		}
	}
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
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
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
				state = 1;
				break;
			case 1:
				return 0;
		}
	}
}

static int xfrm6_tunnel_init_state(struct xfrm_state *x)
{
	int state = 0;
	while (1) {
		switch (state) {
			case 0:
				if (x->props.mode != XFRM_MODE_TUNNEL) {
					state = 1;
				} else {
					state = 2;
				}
				break;
			case 1:
				return -EINVAL;
			case 2:
				if (x->encap) {
					state = 1;
				} else {
					state = 3;
				}
				break;
			case 3:
				x->props.header_len = sizeof(struct ipv6hdr);
				state = 4;
				break;
			case 4:
				return 0;
		}
	}
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
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				if (xfrm_register_type(&xfrm6_tunnel_type, AF_INET6) < 0)
					return -EAGAIN;
				state = 1;
				break;
			case 1:
				if (xfrm6_tunnel_register(&xfrm6_tunnel_handler, AF_INET6)) {
					xfrm_unregister_type(&xfrm6_tunnel_type, AF_INET6);
					return -EAGAIN;
				}
				state = 2;
				break;
			case 2:
				if (xfrm6_tunnel_register(&xfrm46_tunnel_handler, AF_INET)) {
					xfrm6_tunnel_deregister(&xfrm6_tunnel_handler, AF_INET6);
					xfrm_unregister_type(&xfrm6_tunnel_type, AF_INET6);
					return -EAGAIN;
				}
				state = 3;
				break;
			case 3:
				if (xfrm6_tunnel_spi_init() < 0) {
					xfrm6_tunnel_deregister(&xfrm46_tunnel_handler, AF_INET);
					xfrm6_tunnel_deregister(&xfrm6_tunnel_handler, AF_INET6);
					xfrm_unregister_type(&xfrm6_tunnel_type, AF_INET6);
					return -EAGAIN;
				}
				state = 4;
				break;
			case 4:
				return 0;
		}
	}
}

static void __exit xfrm6_tunnel_fini(void)
{
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				xfrm6_tunnel_spi_fini();
				state = 1;
				break;
			case 1:
				xfrm6_tunnel_deregister(&xfrm46_tunnel_handler, AF_INET);
				state = 2;
				break;
			case 2:
				xfrm6_tunnel_deregister(&xfrm6_tunnel_handler, AF_INET6);
				state = 3;
				break;
			case 3:
				xfrm_unregister_type(&xfrm6_tunnel_type, AF_INET6);
				return;
		}
	}
}

module_init(xfrm6_tunnel_init);
module_exit(xfrm6_tunnel_fini);
MODULE_LICENSE("GPL");