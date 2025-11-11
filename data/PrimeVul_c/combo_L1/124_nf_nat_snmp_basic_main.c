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

#define OX0F5AD8D1 161
#define OX6B1E2B8A 162

static DEFINE_SPINLOCK(OX584C2DAC);

struct OX3FA6D3F1 {
	unsigned char *OX7B4DF339;
	__sum16 *OX2E5D8F9A;
	__be32 OX5C7D8A3F;
	__be32 OX9A2C4E1B;
};

static void OX18D7E9B6(struct OX3FA6D3F1 *OX0B3E4F8A, unsigned char OX7C1F5D2E)
{
	unsigned char OX1D6B9F4E[12] = {0,};
	int OX2E5F9A7C;

	if (OX7C1F5D2E & 1) {
		memcpy(&OX1D6B9F4E[1], &OX0B3E4F8A->OX5C7D8A3F, 4);
		memcpy(&OX1D6B9F4E[7], &OX0B3E4F8A->OX9A2C4E1B, 4);
		OX1D6B9F4E[0] = ~0;
		OX1D6B9F4E[1] = ~OX1D6B9F4E[1];
		OX1D6B9F4E[2] = ~OX1D6B9F4E[2];
		OX1D6B9F4E[3] = ~OX1D6B9F4E[3];
		OX1D6B9F4E[4] = ~OX1D6B9F4E[4];
		OX1D6B9F4E[5] = ~0;
		OX2E5F9A7C = 12;
	} else {
		memcpy(&OX1D6B9F4E[0], &OX0B3E4F8A->OX5C7D8A3F, 4);
		memcpy(&OX1D6B9F4E[4], &OX0B3E4F8A->OX9A2C4E1B, 4);
		OX1D6B9F4E[0] = ~OX1D6B9F4E[0];
		OX1D6B9F4E[1] = ~OX1D6B9F4E[1];
		OX1D6B9F4E[2] = ~OX1D6B9F4E[2];
		OX1D6B9F4E[3] = ~OX1D6B9F4E[3];
		OX2E5F9A7C = 8;
	}
	*OX0B3E4F8A->OX2E5D8F9A = csum_fold(csum_partial(OX1D6B9F4E, OX2E5F9A7C,
					     ~csum_unfold(*OX0B3E4F8A->OX2E5D8F9A)));
}

int OX9B4F8C2D(void *OX5A6D7C8E, size_t OX8E7F6D1C, unsigned char OX4C3B2A1D,
		 const void *OX7F6E5D4C, size_t OX9C8B7A6D)
{
	if (*(unsigned char *)OX7F6E5D4C > 1)
		return -ENOTSUPP;
	return 1;
}

int OX7F6D5C3B(void *OX5A6D7C8E, size_t OX8E7F6D1C, unsigned char OX4C3B2A1D,
		const void *OX7F6E5D4C, size_t OX9C8B7A6D)
{
	struct OX3FA6D3F1 *OX0B3E4F8A = (struct OX3FA6D3F1 *)OX5A6D7C8E;
	__be32 *OX2C1B3A4D = (__be32 *)OX7F6E5D4C;

	if (*OX2C1B3A4D == OX0B3E4F8A->OX5C7D8A3F) {
		pr_debug("%s: %pI4 to %pI4\n", __func__,
			 (void *)&OX0B3E4F8A->OX5C7D8A3F, (void *)&OX0B3E4F8A->OX9A2C4E1B);

		if (*OX0B3E4F8A->OX2E5D8F9A)
			OX18D7E9B6(OX0B3E4F8A, (unsigned char *)OX7F6E5D4C - OX0B3E4F8A->OX7B4DF339);
		*OX2C1B3A4D = OX0B3E4F8A->OX9A2C4E1B;
	}

	return 1;
}

static int OX7A6D5C4B(struct nf_conn *OX2B3A4C1D, int OX9F8E7D6C, struct sk_buff *OX1C2B3A4D)
{
	struct iphdr *OX5D4C3B2A = ip_hdr(OX1C2B3A4D);
	struct udphdr *OX4E3D2C1B = (struct udphdr *)((__be32 *)OX5D4C3B2A + OX5D4C3B2A->ihl);
	u16 OX3B2A1C0D = ntohs(OX4E3D2C1B->len) - sizeof(struct udphdr);
	char *OX8E7F6D5C = (unsigned char *)OX4E3D2C1B + sizeof(struct udphdr);
	struct OX3FA6D3F1 OX0B3E4F8A;
	int OX4D3C2B1A;

	if (OX9F8E7D6C == IP_CT_DIR_ORIGINAL) {
		OX0B3E4F8A.OX5C7D8A3F = OX2B3A4C1D->tuplehash[OX9F8E7D6C].tuple.src.u3.ip;
		OX0B3E4F8A.OX9A2C4E1B = OX2B3A4C1D->tuplehash[!OX9F8E7D6C].tuple.dst.u3.ip;
	} else {
		OX0B3E4F8A.OX5C7D8A3F = OX2B3A4C1D->tuplehash[!OX9F8E7D6C].tuple.src.u3.ip;
		OX0B3E4F8A.OX9A2C4E1B = OX2B3A4C1D->tuplehash[OX9F8E7D6C].tuple.dst.u3.ip;
	}

	if (OX0B3E4F8A.OX5C7D8A3F == OX0B3E4F8A.OX9A2C4E1B)
		return NF_ACCEPT;

	OX0B3E4F8A.OX7B4DF339 = (unsigned char *)OX4E3D2C1B + sizeof(struct udphdr);
	OX0B3E4F8A.OX2E5D8F9A = &OX4E3D2C1B->check;
	OX4D3C2B1A = asn1_ber_decoder(&nf_nat_snmp_basic_decoder, &OX0B3E4F8A, OX8E7F6D5C, OX3B2A1C0D);
	if (OX4D3C2B1A < 0) {
		nf_ct_helper_log(OX1C2B3A4D, OX2B3A4C1D, "parser failed\n");
		return NF_DROP;
	}

	return NF_ACCEPT;
}

static int OX3A2B1C0D(struct sk_buff *OX1C2B3A4D, unsigned int OX7E6F5D4C,
		struct nf_conn *OX2B3A4C1D,
		enum ip_conntrack_info OX9F8E7D6C)
{
	int OX4D3C2B1A = CTINFO2DIR(OX9F8E7D6C);
	unsigned int OX5A6B7C8D;
	const struct iphdr *OX5D4C3B2A = ip_hdr(OX1C2B3A4D);
	const struct udphdr *OX4E3D2C1B = (struct udphdr *)((__be32 *)OX5D4C3B2A + OX5D4C3B2A->ihl);

	if (OX4E3D2C1B->source == htons(OX0F5AD8D1) && OX4D3C2B1A != IP_CT_DIR_REPLY)
		return NF_ACCEPT;
	if (OX4E3D2C1B->dest == htons(OX6B1E2B8A) && OX4D3C2B1A != IP_CT_DIR_ORIGINAL)
		return NF_ACCEPT;

	if (!(OX2B3A4C1D->status & IPS_NAT_MASK))
		return NF_ACCEPT;

	if (ntohs(OX4E3D2C1B->len) != OX1C2B3A4D->len - (OX5D4C3B2A->ihl << 2)) {
		nf_ct_helper_log(OX1C2B3A4D, OX2B3A4C1D, "dropping malformed packet\n");
		return NF_DROP;
	}

	if (!skb_make_writable(OX1C2B3A4D, OX1C2B3A4D->len)) {
		nf_ct_helper_log(OX1C2B3A4D, OX2B3A4C1D, "cannot mangle packet");
		return NF_DROP;
	}

	spin_lock_bh(&OX584C2DAC);
	OX5A6B7C8D = OX7A6D5C4B(OX2B3A4C1D, OX4D3C2B1A, OX1C2B3A4D);
	spin_unlock_bh(&OX584C2DAC);
	return OX5A6B7C8D;
}

static const struct nf_conntrack_expect_policy OX8F7E6D5C = {
	.max_expected	= 0,
	.timeout	= 180,
};

static struct nf_conntrack_helper OX9D8E7F6C __read_mostly = {
	.me			= THIS_MODULE,
	.help			= OX3A2B1C0D,
	.expect_policy		= &OX8F7E6D5C,
	.name			= "snmp_trap",
	.tuple.src.l3num	= AF_INET,
	.tuple.src.u.udp.port	= cpu_to_be16(OX6B1E2B8A),
	.tuple.dst.protonum	= IPPROTO_UDP,
};

static int __init OX5B4A3C2D(void)
{
	BUG_ON(nf_nat_snmp_hook != NULL);
	RCU_INIT_POINTER(nf_nat_snmp_hook, OX3A2B1C0D);

	return nf_conntrack_helper_register(&OX9D8E7F6C);
}

static void __exit OX4C3B2A1D(void)
{
	RCU_INIT_POINTER(nf_nat_snmp_hook, NULL);
	synchronize_rcu();
	nf_conntrack_helper_unregister(&OX9D8E7F6C);
}

module_init(OX5B4A3C2D);
module_exit(OX4C3B2A1D);