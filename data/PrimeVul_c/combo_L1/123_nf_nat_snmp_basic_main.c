#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <net/checksum.h>
#include <net/udp.h>

#include <net/netfilter/nf_nat.h>
#include <net/netfilter/nf_conntrack_expect.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <linux/netfilter/nf_conntrack_snmp.h>
#include "nf_nat_snmp_basic.asn1.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("James Morris <jmorris@intercode.com.au>");
MODULE_DESCRIPTION("Basic SNMP Application Layer Gateway");
MODULE_ALIAS("ip_nat_snmp_basic");
MODULE_ALIAS_NFCT_HELPER("snmp_trap");

#define OX715FF 161
#define OX8C5A8 162

static DEFINE_SPINLOCK(OXBE6C69);

struct OX1C9C5D {
	unsigned char *OX5727D4;
	__sum16 *OXB4A1B6;
	__be32 OX99EF10;
	__be32 OX5F73D8;
};

static void OX6A1F5F(struct OX1C9C5D *OX7E6D5B, unsigned char OXBEA8F7)
{
	unsigned char OX5BFD84[12] = {0,};
	int OXA5B2C8;

	if (OXBEA8F7 & 1) {
		memcpy(&OX5BFD84[1], &OX7E6D5B->OX99EF10, 4);
		memcpy(&OX5BFD84[7], &OX7E6D5B->OX5F73D8, 4);
		OX5BFD84[0] = ~0;
		OX5BFD84[1] = ~OX5BFD84[1];
		OX5BFD84[2] = ~OX5BFD84[2];
		OX5BFD84[3] = ~OX5BFD84[3];
		OX5BFD84[4] = ~OX5BFD84[4];
		OX5BFD84[5] = ~0;
		OXA5B2C8 = 12;
	} else {
		memcpy(&OX5BFD84[0], &OX7E6D5B->OX99EF10, 4);
		memcpy(&OX5BFD84[4], &OX7E6D5B->OX5F73D8, 4);
		OX5BFD84[0] = ~OX5BFD84[0];
		OX5BFD84[1] = ~OX5BFD84[1];
		OX5BFD84[2] = ~OX5BFD84[2];
		OX5BFD84[3] = ~OX5BFD84[3];
		OXA5B2C8 = 8;
	}
	*OX7E6D5B->OXB4A1B6 = csum_fold(csum_partial(OX5BFD84, OXA5B2C8,
					     ~csum_unfold(*OX7E6D5B->OXB4A1B6)));
}

int OX5A1B3C(void *OXB3A2F1, size_t OX6C7F5B, unsigned char OXCAF3D5,
		 const void *OXF8B7CA, size_t OX5D4C8A)
{
	if (*(unsigned char *)OXF8B7CA > 1)
		return -ENOTSUPP;
	return 1;
}

int OX8B7F6A(void *OXB3A2F1, size_t OX6C7F5B, unsigned char OXCAF3D5,
		const void *OXF8B7CA, size_t OX5D4C8A)
{
	struct OX1C9C5D *OX7E6D5B = (struct OX1C9C5D *)OXB3A2F1;
	__be32 *OXA1B3C5 = (__be32 *)OXF8B7CA;

	if (*OXA1B3C5 == OX7E6D5B->OX99EF10) {
		pr_debug("%s: %pI4 to %pI4\n", __func__,
			 (void *)&OX7E6D5B->OX99EF10, (void *)&OX7E6D5B->OX5F73D8);

		if (*OX7E6D5B->OXB4A1B6)
			OX6A1F5F(OX7E6D5B, (unsigned char *)OXF8B7CA - OX7E6D5B->OX5727D4);
		*OXA1B3C5 = OX7E6D5B->OX5F73D8;
	}

	return 1;
}

static int OX5F1C2D(struct nf_conn *OXB2A3F4, int OX3F5B7D, struct sk_buff *OX9B8C7D)
{
	struct iphdr *OX5A1B2C = ip_hdr(OX9B8C7D);
	struct udphdr *OX7C8D9E = (struct udphdr *)((__be32 *)OX5A1B2C + OX5A1B2C->ihl);
	u16 OX6E5C3A = ntohs(OX7C8D9E->len) - sizeof(struct udphdr);
	char *OX7E9F8C = (unsigned char *)OX7C8D9E + sizeof(struct udphdr);
	struct OX1C9C5D OX7E6D5B;
	int OX6F8D2B;

	if (OX3F5B7D == IP_CT_DIR_ORIGINAL) {
		OX7E6D5B.OX99EF10 = OXB2A3F4->tuplehash[OX3F5B7D].tuple.src.u3.ip;
		OX7E6D5B.OX5F73D8 = OXB2A3F4->tuplehash[!OX3F5B7D].tuple.dst.u3.ip;
	} else {
		OX7E6D5B.OX99EF10 = OXB2A3F4->tuplehash[!OX3F5B7D].tuple.src.u3.ip;
		OX7E6D5B.OX5F73D8 = OXB2A3F4->tuplehash[OX3F5B7D].tuple.dst.u3.ip;
	}

	if (OX7E6D5B.OX99EF10 == OX7E6D5B.OX5F73D8)
		return NF_ACCEPT;

	OX7E6D5B.OX5727D4 = (unsigned char *)OX7C8D9E + sizeof(struct udphdr);
	OX7E6D5B.OXB4A1B6 = &OX7C8D9E->check;
	OX6F8D2B = asn1_ber_decoder(&nf_nat_snmp_basic_decoder, &OX7E6D5B, OX7E9F8C, OX6E5C3A);
	if (OX6F8D2B < 0) {
		nf_ct_helper_log(OX9B8C7D, OXB2A3F4, "parser failed\n");
		return NF_DROP;
	}

	return NF_ACCEPT;
}

static int OX9C7E8B(struct sk_buff *OX9B8C7D, unsigned int OX8A5D4C,
		struct nf_conn *OXB2A3F4,
		enum ip_conntrack_info OX5D4A6B)
{
	int OX3F5B7D = CTINFO2DIR(OX5D4A6B);
	unsigned int OX6F8D2B;
	const struct iphdr *OX5A1B2C = ip_hdr(OX9B8C7D);
	const struct udphdr *OX7C8D9E = (struct udphdr *)((__be32 *)OX5A1B2C + OX5A1B2C->ihl);

	if (OX7C8D9E->source == htons(OX715FF) && OX3F5B7D != IP_CT_DIR_REPLY)
		return NF_ACCEPT;
	if (OX7C8D9E->dest == htons(OX8C5A8) && OX3F5B7D != IP_CT_DIR_ORIGINAL)
		return NF_ACCEPT;

	if (!(OXB2A3F4->status & IPS_NAT_MASK))
		return NF_ACCEPT;

	if (ntohs(OX7C8D9E->len) != OX9B8C7D->len - (OX5A1B2C->ihl << 2)) {
		nf_ct_helper_log(OX9B8C7D, OXB2A3F4, "dropping malformed packet\n");
		return NF_DROP;
	}

	if (!skb_make_writable(OX9B8C7D, OX9B8C7D->len)) {
		nf_ct_helper_log(OX9B8C7D, OXB2A3F4, "cannot mangle packet");
		return NF_DROP;
	}

	spin_lock_bh(&OXBE6C69);
	OX6F8D2B = OX5F1C2D(OXB2A3F4, OX3F5B7D, OX9B8C7D);
	spin_unlock_bh(&OXBE6C69);
	return OX6F8D2B;
}

static const struct nf_conntrack_expect_policy OX6C7B2F = {
	.max_expected	= 0,
	.timeout	= 180,
};

static struct nf_conntrack_helper OX3E8F4A __read_mostly = {
	.me			= THIS_MODULE,
	.help			= OX9C7E8B,
	.expect_policy		= &OX6C7B2F,
	.name			= "snmp_trap",
	.tuple.src.l3num	= AF_INET,
	.tuple.src.u.udp.port	= cpu_to_be16(OX8C5A8),
	.tuple.dst.protonum	= IPPROTO_UDP,
};

static int __init OX8D1B5A(void)
{
	BUG_ON(nf_nat_snmp_hook != NULL);
	RCU_INIT_POINTER(nf_nat_snmp_hook, OX9C7E8B);

	return nf_conntrack_helper_register(&OX3E8F4A);
}

static void __exit OX2C7D5F(void)
{
	RCU_INIT_POINTER(nf_nat_snmp_hook, NULL);
	synchronize_rcu();
	nf_conntrack_helper_unregister(&OX3E8F4A);
}

module_init(OX8D1B5A);
module_exit(OX2C7D5F);