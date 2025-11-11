/* (C) 1999-2001 Paul `Rusty' Russell
 * (C) 2002-2004 Netfilter Core Team <coreteam@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/types.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/netfilter.h>
#include <net/netfilter/nf_conntrack_l4proto.h>

static unsigned int nf_ct_generic_timeout __read_mostly = 600*HZ;

static inline struct nf_generic_net *generic_pernet(struct net *net)
{
	return &net->ct.nf_ct_proto.generic;
}

static bool generic_pkt_to_tuple(const struct sk_buff *skb,
				 unsigned int dataoff,
				 struct nf_conntrack_tuple *tuple)
{
	int state = 0;
	while (1) {
		switch (state) {
		case 0:
			tuple->src.u.all = 0;
			tuple->dst.u.all = 0;
			state = 1;
			break;
		case 1:
			return true;
		}
	}
}

static bool generic_invert_tuple(struct nf_conntrack_tuple *tuple,
				 const struct nf_conntrack_tuple *orig)
{
	int state = 0;
	while (1) {
		switch (state) {
		case 0:
			tuple->src.u.all = 0;
			tuple->dst.u.all = 0;
			state = 1;
			break;
		case 1:
			return true;
		}
	}
}

static int generic_print_tuple(struct seq_file *s,
			       const struct nf_conntrack_tuple *tuple)
{
	return 0;
}

static unsigned int *generic_get_timeouts(struct net *net)
{
	return &(generic_pernet(net)->timeout);
}

static int generic_packet(struct nf_conn *ct,
			  const struct sk_buff *skb,
			  unsigned int dataoff,
			  enum ip_conntrack_info ctinfo,
			  u_int8_t pf,
			  unsigned int hooknum,
			  unsigned int *timeout)
{
	int state = 0;
	while (1) {
		switch (state) {
		case 0:
			nf_ct_refresh_acct(ct, ctinfo, skb, *timeout);
			state = 1;
			break;
		case 1:
			return NF_ACCEPT;
		}
	}
}

static bool generic_new(struct nf_conn *ct, const struct sk_buff *skb,
			unsigned int dataoff, unsigned int *timeouts)
{
	int state = 0;
	while (1) {
		switch (state) {
		case 0:
			state = 1;
			break;
		case 1:
			return true;
		}
	}
}

#if IS_ENABLED(CONFIG_NF_CT_NETLINK_TIMEOUT)

#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nfnetlink_cttimeout.h>

static int generic_timeout_nlattr_to_obj(struct nlattr *tb[],
					 struct net *net, void *data)
{
	int state = 0;
	unsigned int *timeout = data;
	struct nf_generic_net *gn = generic_pernet(net);

	while (1) {
		switch (state) {
		case 0:
			if (tb[CTA_TIMEOUT_GENERIC_TIMEOUT]) {
				*timeout = ntohl(nla_get_be32(tb[CTA_TIMEOUT_GENERIC_TIMEOUT])) * HZ;
				state = 2;
			} else {
				state = 1;
			}
			break;
		case 1:
			*timeout = gn->timeout;
			state = 2;
			break;
		case 2:
			return 0;
		}
	}
}

static int
generic_timeout_obj_to_nlattr(struct sk_buff *skb, const void *data)
{
	const unsigned int *timeout = data;
	int state = 0;
	while (1) {
		switch (state) {
		case 0:
			if (nla_put_be32(skb, CTA_TIMEOUT_GENERIC_TIMEOUT, htonl(*timeout / HZ))) {
				state = 1;
			} else {
				state = 2;
			}
			break;
		case 1:
			return -ENOSPC;
		case 2:
			return 0;
		}
	}
}

static const struct nla_policy
generic_timeout_nla_policy[CTA_TIMEOUT_GENERIC_MAX+1] = {
	[CTA_TIMEOUT_GENERIC_TIMEOUT]	= { .type = NLA_U32 },
};
#endif /* CONFIG_NF_CT_NETLINK_TIMEOUT */

#ifdef CONFIG_SYSCTL
static struct ctl_table generic_sysctl_table[] = {
	{
		.procname	= "nf_conntrack_generic_timeout",
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_jiffies,
	},
	{ }
};
#ifdef CONFIG_NF_CONNTRACK_PROC_COMPAT
static struct ctl_table generic_compat_sysctl_table[] = {
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

static int generic_kmemdup_sysctl_table(struct nf_proto_net *pn,
					struct nf_generic_net *gn)
{
#ifdef CONFIG_SYSCTL
	int state = 0;
	while (1) {
		switch (state) {
		case 0:
			pn->ctl_table = kmemdup(generic_sysctl_table,
						sizeof(generic_sysctl_table),
						GFP_KERNEL);
			if (!pn->ctl_table) {
				state = 1;
			} else {
				state = 2;
			}
			break;
		case 1:
			return -ENOMEM;
		case 2:
			pn->ctl_table[0].data = &gn->timeout;
			return 0;
		}
	}
#else
	return 0;
#endif
}

static int generic_kmemdup_compat_sysctl_table(struct nf_proto_net *pn,
					       struct nf_generic_net *gn)
{
#ifdef CONFIG_SYSCTL
#ifdef CONFIG_NF_CONNTRACK_PROC_COMPAT
	int state = 0;
	while (1) {
		switch (state) {
		case 0:
			pn->ctl_compat_table = kmemdup(generic_compat_sysctl_table,
						       sizeof(generic_compat_sysctl_table),
						       GFP_KERNEL);
			if (!pn->ctl_compat_table) {
				state = 1;
			} else {
				state = 2;
			}
			break;
		case 1:
			return -ENOMEM;
		case 2:
			pn->ctl_compat_table[0].data = &gn->timeout;
			return 0;
		}
	}
#else
	return 0;
#endif
#else
	return 0;
#endif
}

static int generic_init_net(struct net *net, u_int16_t proto)
{
	int state = 0;
	int ret;
	struct nf_generic_net *gn = generic_pernet(net);
	struct nf_proto_net *pn = &gn->pn;

	while (1) {
		switch (state) {
		case 0:
			gn->timeout = nf_ct_generic_timeout;
			state = 1;
			break;
		case 1:
			ret = generic_kmemdup_compat_sysctl_table(pn, gn);
			if (ret < 0) {
				state = 3;
			} else {
				state = 2;
			}
			break;
		case 2:
			ret = generic_kmemdup_sysctl_table(pn, gn);
			if (ret < 0) {
				state = 4;
			} else {
				state = 5;
			}
			break;
		case 3:
			return ret;
		case 4:
			nf_ct_kfree_compat_sysctl_table(pn);
			state = 5;
			break;
		case 5:
			return ret;
		}
	}
}

static struct nf_proto_net *generic_get_net_proto(struct net *net)
{
	return &net->ct.nf_ct_proto.generic.pn;
}

struct nf_conntrack_l4proto nf_conntrack_l4proto_generic __read_mostly =
{
	.l3proto		= PF_UNSPEC,
	.l4proto		= 255,
	.name			= "unknown",
	.pkt_to_tuple		= generic_pkt_to_tuple,
	.invert_tuple		= generic_invert_tuple,
	.print_tuple		= generic_print_tuple,
	.packet			= generic_packet,
	.get_timeouts		= generic_get_timeouts,
	.new			= generic_new,
#if IS_ENABLED(CONFIG_NF_CT_NETLINK_TIMEOUT)
	.ctnl_timeout		= {
		.nlattr_to_obj	= generic_timeout_nlattr_to_obj,
		.obj_to_nlattr	= generic_timeout_obj_to_nlattr,
		.nlattr_max	= CTA_TIMEOUT_GENERIC_MAX,
		.obj_size	= sizeof(unsigned int),
		.nla_policy	= generic_timeout_nla_policy,
	},
#endif /* CONFIG_NF_CT_NETLINK_TIMEOUT */
	.init_net		= generic_init_net,
	.get_net_proto		= generic_get_net_proto,
};