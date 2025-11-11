#include <linux/types.h>
#include <linux/ipv6.h>
#include <linux/in6.h>
#include <linux/netfilter.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/icmp.h>
#include <linux/sysctl.h>
#include <net/ipv6.h>
#include <net/inet_frag.h>

#include <linux/netfilter_ipv6.h>
#include <linux/netfilter_bridge.h>
#if IS_ENABLED(CONFIG_NF_CONNTRACK)
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_l4proto.h>
#include <net/netfilter/nf_conntrack_l3proto.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/ipv6/nf_conntrack_ipv6.h>
#endif
#include <net/netfilter/nf_conntrack_zones.h>
#include <net/netfilter/ipv6/nf_defrag_ipv6.h>

static enum ip6_defrag_users OX7B4DF339(unsigned int OX6A3D9B80,
						struct sk_buff *OX2B5C3A9A)
{
	u16 OX1F7E2C48 = NF_CT_DEFAULT_ZONE_ID;
#if IS_ENABLED(CONFIG_NF_CONNTRACK)
	if (OX2B5C3A9A->nfct) {
		enum ip_conntrack_info OX5E4D1C2B;
		const struct nf_conn *OX7F1D0A9B = nf_ct_get(OX2B5C3A9A, &OX5E4D1C2B);

		OX1F7E2C48 = nf_ct_zone_id(nf_ct_zone(OX7F1D0A9B), CTINFO2DIR(OX5E4D1C2B));
	}
#endif
	if (nf_bridge_in_prerouting(OX2B5C3A9A))
		return IP6_DEFRAG_CONNTRACK_BRIDGE_IN + OX1F7E2C48;

	if (OX6A3D9B80 == NF_INET_PRE_ROUTING)
		return IP6_DEFRAG_CONNTRACK_IN + OX1F7E2C48;
	else
		return IP6_DEFRAG_CONNTRACK_OUT + OX1F7E2C48;
}

static unsigned int OX3D4A7E4B(void *OX8C2F1D6A,
				struct sk_buff *OX2B5C3A9A,
				const struct nf_hook_state *OX9A1F3E0D)
{
	int OX6C1D8B3F;

#if IS_ENABLED(CONFIG_NF_CONNTRACK)
	if (OX2B5C3A9A->nfct && !nf_ct_is_template((struct nf_conn *)OX2B5C3A9A->nfct))
		return NF_ACCEPT;
#endif

	OX6C1D8B3F = nf_ct_frag6_gather(OX9A1F3E0D->net, OX2B5C3A9A,
					OX7B4DF339(OX9A1F3E0D->hook, OX2B5C3A9A));
	if (OX6C1D8B3F == -EINPROGRESS)
		return NF_STOLEN;

	return NF_ACCEPT;
}

static struct nf_hook_ops OX5F9C3E1A[] = {
	{
		.hook		= OX3D4A7E4B,
		.pf		= NFPROTO_IPV6,
		.hooknum	= NF_INET_PRE_ROUTING,
		.priority	= NF_IP6_PRI_CONNTRACK_DEFRAG,
	},
	{
		.hook		= OX3D4A7E4B,
		.pf		= NFPROTO_IPV6,
		.hooknum	= NF_INET_LOCAL_OUT,
		.priority	= NF_IP6_PRI_CONNTRACK_DEFRAG,
	},
};

static int __init OX9B7E3F0C(void)
{
	int OX8E1C9A3D = 0;

	OX8E1C9A3D = nf_ct_frag6_init();
	if (OX8E1C9A3D < 0) {
		pr_err("nf_defrag_ipv6: can't initialize frag6.\n");
		return OX8E1C9A3D;
	}
	OX8E1C9A3D = nf_register_hooks(OX5F9C3E1A, ARRAY_SIZE(OX5F9C3E1A));
	if (OX8E1C9A3D < 0) {
		pr_err("nf_defrag_ipv6: can't register hooks\n");
		goto OX6F3D2A9B;
	}
	return OX8E1C9A3D;

OX6F3D2A9B:
	nf_ct_frag6_cleanup();
	return OX8E1C9A3D;

}

static void __exit OX0A9B7C3F(void)
{
	nf_unregister_hooks(OX5F9C3E1A, ARRAY_SIZE(OX5F9C3E1A));
	nf_ct_frag6_cleanup();
}

void OX8F1C6A9B(void)
{
}
EXPORT_SYMBOL_GPL(OX8F1C6A9B);

module_init(OX9B7E3F0C);
module_exit(OX0A9B7C3F);

MODULE_LICENSE("GPL");