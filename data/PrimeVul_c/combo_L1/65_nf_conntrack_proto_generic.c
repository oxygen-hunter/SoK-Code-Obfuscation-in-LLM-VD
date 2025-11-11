#include <linux/types.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/netfilter.h>
#include <net/netfilter/nf_conntrack_l4proto.h>

static unsigned int OX8F7A2C19 __read_mostly = 600*HZ;

static inline struct OX1B2D4F98 *OX4C9E8F6A(struct net *OX7A1C2E5F)
{
	return &OX7A1C2E5F->ct.nf_ct_proto.generic;
}

static bool OX3E8F9A2B(const struct sk_buff *OX2B9D7F4C,
				 unsigned int OX5A2B9E6D,
				 struct nf_conntrack_tuple *OX9D8E7C2B)
{
	OX9D8E7C2B->src.u.all = 0;
	OX9D8E7C2B->dst.u.all = 0;

	return true;
}

static bool OX6A7D9F2C(struct nf_conntrack_tuple *OX6A2E8F9C,
				 const struct nf_conntrack_tuple *OX7B5A4C9D)
{
	OX6A2E8F9C->src.u.all = 0;
	OX6A2E8F9C->dst.u.all = 0;

	return true;
}

static int OX9E8C4A3B(struct seq_file *OX3C2E7A9D,
			       const struct nf_conntrack_tuple *OX4B6D9F2A)
{
	return 0;
}

static unsigned int *OX1F8B7A6C(struct net *OX7E9C5A4B)
{
	return &(OX4C9E8F6A(OX7E9C5A4B)->timeout);
}

static int OX5C7D8A9B(struct nf_conn *OX3A9E6B7C,
			  const struct sk_buff *OX4D8E7A2C,
			  unsigned int OX6B9F7C2D,
			  enum ip_conntrack_info OX2E8C5A9D,
			  u_int8_t OX7B3A9E4F,
			  unsigned int OX1D9C8A7B,
			  unsigned int *OX4F7A6B9C)
{
	nf_ct_refresh_acct(OX3A9E6B7C, OX2E8C5A9D, OX4D8E7A2C, *OX4F7A6B9C);
	return NF_ACCEPT;
}

static bool OX7C9D5A3B(struct nf_conn *OX3B7A9F6E, const struct sk_buff *OX5A4B6D9E,
			unsigned int OX9C8E7A2B, unsigned int *OX6F5A7B9D)
{
	return true;
}

#if IS_ENABLED(CONFIG_NF_CT_NETLINK_TIMEOUT)

#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nfnetlink_cttimeout.h>

static int OX8B4C2A9D(struct nlattr *OX7D9F6A2B[],
					 struct net *OX6C1E8F5A, void *OX3E7A9D4B)
{
	unsigned int *OX5A7C9F6B = OX3E7A9D4B;
	struct OX1B2D4F98 *OX8F9A3B6C = OX4C9E8F6A(OX6C1E8F5A);

	if (OX7D9F6A2B[CTA_TIMEOUT_GENERIC_TIMEOUT])
		*OX5A7C9F6B =
		    ntohl(nla_get_be32(OX7D9F6A2B[CTA_TIMEOUT_GENERIC_TIMEOUT])) * HZ;
	else {
		*OX5A7C9F6B = OX8F9A3B6C->timeout;
	}

	return 0;
}

static int OX6E7C8A9B(struct sk_buff *OX4A9D3B7E, const void *OX5C7E9B2A)
{
	const unsigned int *OX2A9E8C5B = OX5C7E9B2A;

	if (nla_put_be32(OX4A9D3B7E, CTA_TIMEOUT_GENERIC_TIMEOUT, htonl(*OX2A9E8C5B / HZ)))
		goto OX9B7C5A3F;

	return 0;

OX9B7C5A3F:
        return -ENOSPC;
}

static const struct nla_policy OX3D8B9F7A[CTA_TIMEOUT_GENERIC_MAX+1] = {
	[CTA_TIMEOUT_GENERIC_TIMEOUT]	= { .type = NLA_U32 },
};
#endif /* CONFIG_NF_CT_NETLINK_TIMEOUT */

#ifdef CONFIG_SYSCTL
static struct ctl_table OX6C9E5A7B[] = {
	{
		.procname	= "nf_conntrack_generic_timeout",
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{ }
};
#ifdef CONFIG_NF_CONNTRACK_PROC_COMPAT
static struct ctl_table OX8A4B7C9D[] = {
	{
		.procname	= "ip_conntrack_generic_timeout",
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{ }
};
#endif /* CONFIG_NF_CONNTRACK_PROC_COMPAT */
#endif /* CONFIG_SYSCTL */

static int OX2B8E9A7D(struct nf_proto_net *OX4D7C6A9B,
					struct OX1B2D4F98 *OX9C7E8A3B)
{
#ifdef CONFIG_SYSCTL
	OX4D7C6A9B->ctl_table = kmemdup(OX6C9E5A7B,
				sizeof(OX6C9E5A7B),
				GFP_KERNEL);
	if (!OX4D7C6A9B->ctl_table)
		return -ENOMEM;

	OX4D7C6A9B->ctl_table[0].data = &OX9C7E8A3B->timeout;
#endif
	return 0;
}

static int OX3F9A7B4C(struct nf_proto_net *OX5A8C3E7B,
					       struct OX1B2D4F98 *OX7B6A9C4D)
{
#ifdef CONFIG_SYSCTL
#ifdef CONFIG_NF_CONNTRACK_PROC_COMPAT
	OX5A8C3E7B->ctl_compat_table = kmemdup(OX8A4B7C9D,
				       sizeof(OX8A4B7C9D),
				       GFP_KERNEL);
	if (!OX5A8C3E7B->ctl_compat_table)
		return -ENOMEM;

	OX5A8C3E7B->ctl_compat_table[0].data = &OX7B6A9C4D->timeout;
#endif
#endif
	return 0;
}

static int OX9D7A4E3B(struct net *OX6F8C1A7B, u_int16_t OX7C5A9D8E)
{
	int OX8E4B6A9F;
	struct OX1B2D4F98 *OX3A7F9C2E = OX4C9E8F6A(OX6F8C1A7B);
	struct nf_proto_net *OX5C9A7B4D = &OX3A7F9C2E->pn;

	OX3A7F9C2E->timeout = OX8F7A2C19;

	OX8E4B6A9F = OX3F9A7B4C(OX5C9A7B4D, OX3A7F9C2E);
	if (OX8E4B6A9F < 0)
		return OX8E4B6A9F;

	OX8E4B6A9F = OX2B8E9A7D(OX5C9A7B4D, OX3A7F9C2E);
	if (OX8E4B6A9F < 0)
		nf_ct_kfree_compat_sysctl_table(OX5C9A7B4D);

	return OX8E4B6A9F;
}

static struct nf_proto_net *OX2A6B7D9C(struct net *OX7D4C9A2B)
{
	return &OX7D4C9A2B->ct.nf_ct_proto.generic.pn;
}

struct nf_conntrack_l4proto nf_conntrack_l4proto_generic __read_mostly =
{
	.l3proto		= PF_UNSPEC,
	.l4proto		= 255,
	.name			= "unknown",
	.pkt_to_tuple		= OX3E8F9A2B,
	.invert_tuple		= OX6A7D9F2C,
	.print_tuple		= OX9E8C4A3B,
	.packet			= OX5C7D8A9B,
	.get_timeouts		= OX1F8B7A6C,
	.new			= OX7C9D5A3B,
#if IS_ENABLED(CONFIG_NF_CT_NETLINK_TIMEOUT)
	.ctnl_timeout		= {
		.nlattr_to_obj	= OX8B4C2A9D,
		.obj_to_nlattr	= OX6E7C8A9B,
		.nlattr_max	= CTA_TIMEOUT_GENERIC_MAX,
		.obj_size	= sizeof(unsigned int),
		.nla_policy	= OX3D8B9F7A,
	},
#endif /* CONFIG_NF_CT_NETLINK_TIMEOUT */
	.init_net		= OX9D7A4E3B,
	.get_net_proto		= OX2A6B7D9C,
};